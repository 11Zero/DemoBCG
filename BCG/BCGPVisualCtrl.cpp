//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPVisualCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPVisualCtrl.h"
#include "bcgglobals.h"
#include "trackmouse.h"
#include "BCGPDrawManager.h"
#include "BCGPMath.h"
#include "BCGPGraphicsManagerGDI.h"

#if (!defined _BCGSUITE_) && (!defined _BCGPCHART_STANDALONE)
	#include "BCGPPopupMenu.h"
	#include "BCGPTooltipManager.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseVisualCtrl

IMPLEMENT_DYNAMIC(CBCGPBaseVisualCtrl, CStatic)
IMPLEMENT_DYNAMIC(CBCGPVisualCtrl, CBCGPBaseVisualCtrl)
IMPLEMENT_SERIAL(CBCGPVisualContainerCtrl, CBCGPBaseVisualCtrl, 1)

HHOOK CBCGPBaseVisualCtrl::m_hookMouse = NULL;
HWND CBCGPBaseVisualCtrl::m_hwndHookedPopup = NULL;
BOOL CBCGPBaseVisualCtrl::m_bInPaint = FALSE;

CBCGPBaseVisualCtrl::CBCGPBaseVisualCtrl()
{
	m_pGM = NULL;

	m_dblScale = 0.0;
	
	m_bIsCaptured = FALSE;
	m_bIsTracked = FALSE;

	m_bIsTooltip = FALSE;
	m_bToolTipCleared = FALSE;
	m_bTooltipTrackingMode = FALSE;
	m_pToolTip = NULL;

	m_bIsFocused = FALSE;
	m_nDlgCode = 0;

	m_bIsPopup = FALSE;
	m_nPopupAlpha = 255;
}

CBCGPBaseVisualCtrl::~CBCGPBaseVisualCtrl()
{
	if (m_pGM != NULL)
	{
		delete m_pGM;
	}

#if _MSC_VER >= 1300
	if (m_pProxy != NULL)
	{
		//force disconnect accessibility clients
		::CoDisconnectObject ((IAccessible*)m_pProxy, NULL);
	}
#endif
}

BEGIN_MESSAGE_MAP(CBCGPBaseVisualCtrl, CStatic)
	//{{AFX_MSG_MAP(CBCGPBaseVisualCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_SETCURSOR()
	ON_WM_ENABLE()
	ON_WM_DESTROY()
	ON_WM_GETDLGCODE()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_NCDESTROY()
	ON_WM_NCCREATE()
	//}}AFX_MSG_MAP
#if (!defined _BCGSUITE_) && (!defined _BCGPCHART_STANDALONE)
	ON_REGISTERED_MESSAGE(BCGM_UPDATETOOLTIPS, OnBCGUpdateToolTips)
	ON_REGISTERED_MESSAGE(BCGM_POSTREDRAW, OnBCGPostRedraw)
#endif
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnTTNeedTipText)
	ON_MESSAGE(WM_FLOATSTATUS, OnFloatStatus)
	ON_MESSAGE(WM_GESTURE, OnGestureEvent)
	ON_MESSAGE(WM_TABLET_QUERYSYSTEMGESTURESTATUS, OnTabletQuerySystemGestureStatus)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_MESSAGE(WM_GETOBJECT, OnGetObject)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseVisualCtrl message handlers

BOOL CBCGPBaseVisualCtrl::Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwStyle, CCreateContext* pContext)
{
	CString strClassName = globalData.RegisterWindowClass (_T("BCGPVisualCtrl"));

	return CWnd::Create (strClassName, _T (""),
							dwStyle, 
							rect,
							pParentWnd,
							nID,
							pContext);
}
//***********************************************************************************************************
BOOL CBCGPBaseVisualCtrl::CreatePopup(const CRect& rect, BYTE nTransparency, CWnd* pWndOwner)
{
#if !defined _BCGPCHART_STANDALONE
	CString strClassName = globalData.RegisterWindowClass (_T("BCGPVisualCtrl"));

	m_bIsPopup = TRUE;
	m_nPopupAlpha = nTransparency;

#if _MSC_VER < 1700 || !defined(_BCGSUITE_)
    DWORD dwExStyle = (globalData.IsWindowsLayerSupportAvailable () && globalData.m_nBitsPerPixel > 8) ? 
		WS_EX_LAYERED : 0;
#else
    DWORD dwExStyle = globalData.m_nBitsPerPixel > 8 ? WS_EX_LAYERED : 0;
#endif

	if (!CWnd::CreateEx (dwExStyle | WS_EX_TOOLWINDOW, 
								strClassName, 
								_T (""),
								WS_POPUP | MFS_SYNCACTIVE, 
								rect,
								pWndOwner == NULL ? AfxGetMainWnd() : pWndOwner,
								0))
	{
		return FALSE;
	}

	if (pWndOwner != NULL)
	{
		SetOwner(pWndOwner);
	}

	if (CBCGPPopupMenu::GetActiveMenu() != NULL && ::IsWindow (CBCGPPopupMenu::GetActiveMenu()->m_hWnd))
	{
		CBCGPPopupMenu::GetActiveMenu()->SendMessage (WM_CLOSE);
	}

	ShowWindow(SW_SHOWNOACTIVATE);

	if (TRUE /* TODO - flag close on mouse click */)
	{
		if (m_hwndHookedPopup != NULL)
		{
			::SendMessage(m_hwndHookedPopup, WM_CLOSE, 0, 0);
		}

		ASSERT(m_hookMouse == NULL);
		m_hookMouse = ::SetWindowsHookEx(WH_MOUSE, MouseProc, 0, GetCurrentThreadId());

		m_hwndHookedPopup = GetSafeHwnd();
	}

	if (dwExStyle & WS_EX_LAYERED)
	{
		OnDrawLayeredPopup();
	}

	SetFocus();

	return TRUE;
#else
	return FALSE;
#endif
}
//***********************************************************************************************************
BOOL CBCGPBaseVisualCtrl::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//***********************************************************************************************************
void CBCGPBaseVisualCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if (m_pGM == NULL)
	{
		m_pGM = CBCGPGraphicsManager::CreateInstance();
	}

	if (m_pGM == NULL)
	{
		return;
	}

	double dblMemDCScale = m_pGM->IsSupported(BCGP_GRAPHICS_MANAGER_SCALING) ? 1.0 : m_dblScale;
	
	CBCGPMemDC memDC (dc, this, 0, dblMemDCScale);
	CDC* pDC = &memDC.GetDC ();

	CRect rectClip(0, 0, 0, 0);
	if (!IsDirty())
	{
		dc.GetClipBox(rectClip);
	}

	DoPaint(pDC, rectClip);
}
//***********************************************************************************************************
void CBCGPBaseVisualCtrl::DoPaint(CDC* pDC, CRect rectClip)
{
	CRect rectClient;
	GetClientRect(rectClient);

	m_bInPaint = TRUE;

	pDC->FillRect(rectClient, &globalData.brWindow);

	globalData.DrawParentBackground (this, pDC);

	CBCGPRect rect = GetRect();

	if (m_pGM == NULL)
	{
		m_pGM = CBCGPGraphicsManager::CreateInstance();
	}

	if (m_pGM == NULL)
	{
		m_bInPaint = FALSE;
		return;
	}

	if (rect.IsRectEmpty())
	{
		CRect rectClient;
		GetClientRect(rectClient);

		SetRect(rectClient);
		rect = GetRect();
	}

	m_pGM->BindDC(pDC, rect);

	if (m_dblScale != 0.0 && m_pGM->IsSupported(BCGP_GRAPHICS_MANAGER_SCALING))
	{
		m_pGM->SetScale(m_dblScale);
	}

	if (!m_pGM->BeginDraw())
	{
		return;
	}

	BOOL bSetClipRect = FALSE;

	if (rectClip.IsRectEmpty())
	{
		rectClip = rect;
	}
	else
	{
		bSetClipRect = TRUE;
		m_pGM->SetClipRect(rectClip);
	}

	OnDraw(m_pGM, rectClip);

	if (bSetClipRect)
	{
		m_pGM->ReleaseClipArea();
	}

	SetDirty(FALSE);

	m_pGM->EndDraw();

	m_bInPaint = FALSE;
}
//***********************************************************************************************************
void CBCGPBaseVisualCtrl::OnSize(UINT nType, int cx, int cy)
{
	CStatic::OnSize(nType, cx, cy);

	SetRect(CBCGPRect(0, 0, cx, cy), TRUE);

	CRect rect (0, 0, cx, cy);
	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		m_pToolTip->SetToolRect (this, 1, rect);
	}

	if (GetExStyle() & WS_EX_LAYERED)
	{
		OnDrawLayeredPopup();
	}
}
//***********************************************************************************************************
int CBCGPBaseVisualCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CStatic::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (m_bIsPopup)
	{
		SendMessage(WM_IDLEUPDATECMDUI, (WPARAM)TRUE);
	}
	
	InitCtrl();
	InitTooltip();

	if (GetRect().IsRectEmpty())
	{
		CRect rect;
		GetClientRect(rect);

		SetRect(rect);
	}

	CBCGPGestureConfig gestureConfig;
	if (GetGestureConfig(gestureConfig))
	{
		bcgpGestureManager.SetGestureConfig(GetSafeHwnd(), gestureConfig);
	}

	return 0;
}
//***********************************************************************************************************
void CBCGPBaseVisualCtrl::PreSubclassWindow() 
{
	CStatic::PreSubclassWindow();

	if (!m_bIsPopup)
	{
		InitCtrl();
		InitTooltip();
	}

	ModifyStyle(0, SS_NOTIFY);

	if (GetRect().IsRectEmpty())
	{
		CRect rect;
		GetClientRect(rect);

		SetRect(rect);
	}

	CBCGPGestureConfig gestureConfig;
	if (GetGestureConfig(gestureConfig))
	{
		bcgpGestureManager.SetGestureConfig(GetSafeHwnd(), gestureConfig);
	}
}
//***********************************************************************************************************
void CBCGPBaseVisualCtrl::InitTooltip()
{
	if (!m_bIsTooltip)
	{
		return;
	}

#if !defined _BCGPCHART_STANDALONE
	CBCGPTooltipManager::CreateToolTip (m_pToolTip, this, BCGP_TOOLTIP_TYPE_BUTTON);

	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		CRect rectClient;
		GetClientRect (&rectClient);

		m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, &rectClient, 1);
	}
#endif
}
//***********************************************************************************************************
void CBCGPBaseVisualCtrl::SetGraphicsManager(CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER manager, CBCGPGraphicsManagerParams* pParams)
{
	if (m_pGM != NULL)
	{
		delete m_pGM;
	}

	m_pGM = CBCGPGraphicsManager::CreateInstance(manager, TRUE, pParams);
}
//***********************************************************************************************************
void CBCGPBaseVisualCtrl::SetGraphicsManager(CRuntimeClass* pRTI)
{
	if (m_pGM != NULL)
	{
		delete m_pGM;
	}

	m_pGM = DYNAMIC_DOWNCAST(CBCGPGraphicsManager, pRTI->CreateObject());
	ASSERT_VALID(m_pGM);
}
//***********************************************************************************************************
void CBCGPBaseVisualCtrl::EnableTooltip(BOOL bEnable, BOOL bTooltipTrackingMode)
{
#if !defined _BCGPCHART_STANDALONE
	m_bIsTooltip = bEnable;
	m_bTooltipTrackingMode = bTooltipTrackingMode;

	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	if (m_bIsTooltip)
	{
		if (m_pToolTip->GetSafeHwnd() == NULL)
		{
			InitTooltip();

			if (m_pToolTip->GetSafeHwnd() != NULL)
			{
				m_pToolTip->Activate(TRUE);
			}
		}
	}
	else
	{
		CBCGPTooltipManager::DeleteToolTip(m_pToolTip);
	}
#endif
}
//***********************************************************************************************************
BOOL CBCGPBaseVisualCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint ptCursor;

	::GetCursorPos(&ptCursor);
	ScreenToClient(&ptCursor);

	if (OnSetMouseCursor(ptCursor))
	{
		return TRUE;
	}
	
	return CStatic::OnSetCursor(pWnd, nHitTest, message);
}
//***********************************************************************************************************
BOOL CBCGPBaseVisualCtrl::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
	case WM_MOUSEMOVE:
		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->RelayEvent(pMsg);
		}
	}

	switch (pMsg->message)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		if (pMsg->hwnd == GetSafeHwnd())
		{
			if (!m_bIsPopup)
			{
				SetFocus();
			}

			CPoint pt (BCG_GET_X_LPARAM(pMsg->lParam), BCG_GET_Y_LPARAM(pMsg->lParam));
			if (OnMouseDown(pMsg->message == WM_LBUTTONDOWN ? 0 : pMsg->message == WM_RBUTTONDOWN ? 1 : 2, pt))
			{
#if !defined _BCGPCHART_STANDALONE
				if (CBCGPPopupMenu::GetActiveMenu() != NULL && ::IsWindow (CBCGPPopupMenu::GetActiveMenu()->m_hWnd))
				{
					CBCGPPopupMenu::GetActiveMenu()->SendMessage (WM_CLOSE);
				}
#endif
				SetCapture();
				m_bIsCaptured = TRUE;

				StartClickAndHold();
				return TRUE;
			}
		}
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		if (pMsg->hwnd == GetSafeHwnd())
		{
			CPoint pt (BCG_GET_X_LPARAM(pMsg->lParam), BCG_GET_Y_LPARAM(pMsg->lParam));
			OnMouseUp(pMsg->message == WM_LBUTTONUP ? 0 : pMsg->message == WM_RBUTTONUP ? 1 : 2, pt);

			if (m_bIsCaptured)
			{
				ReleaseCapture();
				m_bIsCaptured = FALSE;
			}

			StopClickAndHold();
		}
		break;

	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
		if (pMsg->hwnd == GetSafeHwnd())
		{
			CPoint pt (BCG_GET_X_LPARAM(pMsg->lParam), BCG_GET_Y_LPARAM(pMsg->lParam));
			OnMouseDblClick(pMsg->message == WM_LBUTTONDBLCLK ? 0 : pMsg->message == WM_RBUTTONDBLCLK ? 1 : 2, pt);
		}
		break;
	
	case WM_MOUSEWHEEL:
		{
			CPoint pt (BCG_GET_X_LPARAM(pMsg->lParam), BCG_GET_Y_LPARAM(pMsg->lParam));

			CWnd* pWnd = CWnd::FromHandle (pMsg->hwnd);
			if (pWnd != NULL)
			{
				pWnd->ScreenToClient(&pt);
			}

			if (OnMouseWheel(pt, HIWORD(pMsg->wParam)))
			{
				if (m_pToolTip->GetSafeHwnd () != NULL)
				{
					m_pToolTip->Pop();
				}

				return TRUE;
			}
		}
		break;

	case WM_MOUSEMOVE:
		{
			CPoint pt (BCG_GET_X_LPARAM(pMsg->lParam), BCG_GET_Y_LPARAM(pMsg->lParam));

			OnMouseMove(pt);

			if (!m_bIsTracked)
			{
				m_bIsTracked = TRUE;
				
				TRACKMOUSEEVENT trackmouseevent;
				trackmouseevent.cbSize = sizeof(trackmouseevent);
				trackmouseevent.dwFlags = TME_LEAVE;
				trackmouseevent.hwndTrack = GetSafeHwnd();
				trackmouseevent.dwHoverTime = HOVER_DEFAULT;
				::BCGPTrackMouse (&trackmouseevent);	
			}

			if (m_pToolTip->GetSafeHwnd () != NULL)
			{
				CString strToolTip;
				CString strDescr;

				OnGetToolTip(pt, strToolTip, strDescr);

				if (strToolTip != m_strLastDisplayedToolTip || strDescr != m_strLastDisplayedToolTipDescr)
				{
					if (m_bTooltipTrackingMode)
					{
						m_pToolTip->Update();
					}
					else
					{
						m_pToolTip->Pop ();
					}

					if (!strToolTip.IsEmpty() && !strDescr.IsEmpty())
					{
						m_bToolTipCleared = TRUE;
					}
				}
			}
		}
		break;

	case WM_CANCELMODE:
		if (m_bIsCaptured)
		{
			ReleaseCapture();
			m_bIsCaptured = FALSE;

			OnCancelMode();
		}

		StopClickAndHold();
		break;

	case WM_KEYDOWN:
		if (pMsg->wParam == VK_ESCAPE)
		{
			if (OnKeyboardDown((UINT)pMsg->wParam, (pMsg->lParam & 0xFFFF), 0))
			{
				return TRUE;
			}
		}
		break;

	case WM_TIMER:
		if (pMsg->hwnd == GetSafeHwnd() && pMsg->wParam == GetClickAndHoldID())
		{
			CPoint point;
			
			::GetCursorPos (&point);
			ScreenToClient (&point);

			if (GetClickAndHoldRect().PtInRect(point))
			{
				OnClickAndHoldEvent(GetClickAndHoldID(), point);
				return TRUE;
			}
		}
		break;
	}

	return CStatic::PreTranslateMessage(pMsg);
}
//********************************************************************************
LRESULT CALLBACK CBCGPBaseVisualCtrl::MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION && m_hwndHookedPopup != NULL)
	{
		MOUSEHOOKSTRUCT* lpMS = (MOUSEHOOKSTRUCT*)lParam;
		ASSERT (lpMS != NULL);

		switch (wParam)
		{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_NCLBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
		case WM_NCMBUTTONDOWN:
			{
				CBCGPBaseVisualCtrl* pWnd = DYNAMIC_DOWNCAST(CBCGPBaseVisualCtrl, CWnd::FromHandle(m_hwndHookedPopup));
				if (pWnd != NULL)
				{
					ASSERT_VALID(pWnd);
					
					if (pWnd->IsCloseOnClick(lpMS->pt))
					{
						pWnd->PostMessage(WM_CLOSE);
					}
				}
			}
			break;
		}
	}

	return CallNextHookEx(m_hookMouse, nCode, wParam, lParam);
}
//********************************************************************************
BOOL CBCGPBaseVisualCtrl::IsCloseOnClick(CPoint ptScreen)
{
	CRect rectWindow;
	GetWindowRect(rectWindow);

	return !rectWindow.PtInRect(ptScreen);
}
//********************************************************************************
void CBCGPBaseVisualCtrl::StartClickAndHold()
{
	if (!GetClickAndHoldRect().IsRectEmpty())
	{
		SetTimer(GetClickAndHoldID(), 200, NULL);
	}
}
//********************************************************************************
void CBCGPBaseVisualCtrl::StopClickAndHold()
{
	if (!GetClickAndHoldRect().IsRectEmpty())
	{
		KillTimer(GetClickAndHoldID());
	}
}
//********************************************************************************
void CBCGPBaseVisualCtrl::OnEnable(BOOL /*bEnable*/) 
{
	RedrawWindow ();
}
//***********************************************************************************************************
LRESULT CBCGPBaseVisualCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_MOUSELEAVE)
	{
		m_bIsTracked = FALSE;
		OnMouseLeave();
	}
	else if (message == WM_ACTIVATEAPP && m_bIsPopup && wParam == 0)
	{
		PostMessage(WM_CLOSE);
	}
	
	return CStatic::WindowProc(message, wParam, lParam);
}
//***********************************************************************************************************
BOOL CBCGPBaseVisualCtrl::OnTTNeedTipText (UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	static CString strTipText;

	CPoint point;
	
	::GetCursorPos (&point);
	ScreenToClient (&point);

	CString strDescr;
	if (!OnGetToolTip(point, strTipText, strDescr))
	{
		return FALSE;
	}

	LPNMTTDISPINFO	pTTDispInfo	= (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	if (m_bToolTipCleared && strTipText == m_strLastDisplayedToolTip && strDescr == m_strLastDisplayedToolTipDescr)
	{
		return FALSE;
	}

#if !defined _BCGPCHART_STANDALONE
	if (!strDescr.IsEmpty())
	{
		CBCGPToolTipCtrl* pToolTip = DYNAMIC_DOWNCAST (
			CBCGPToolTipCtrl, m_pToolTip);

		if (pToolTip != NULL)
		{
			ASSERT_VALID (pToolTip);
			pToolTip->SetDescription(strDescr);
		}
	}
#endif

	pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) strTipText);
	m_bToolTipCleared = FALSE;
	m_strLastDisplayedToolTip = strTipText;
	m_strLastDisplayedToolTipDescr = strDescr;

	return TRUE;
}
//***********************************************************************************************************
LRESULT CBCGPBaseVisualCtrl::OnBCGUpdateToolTips (WPARAM wp, LPARAM)
{
#if !defined _BCGPCHART_STANDALONE
	UINT nTypes = (UINT) wp;

	if (nTypes & BCGP_TOOLTIP_TYPE_DEFAULT)
	{
		CBCGPTooltipManager::CreateToolTip (m_pToolTip, this, BCGP_TOOLTIP_TYPE_BUTTON);

		if (m_pToolTip != NULL)
		{
			CRect rectClient;
			GetClientRect (&rectClient);

			m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, &rectClient, 1);
		}
	}
#endif
	return 0;
}
//*************************************************************************************
void CBCGPBaseVisualCtrl::OnDestroy() 
{
	if (m_hwndHookedPopup == GetSafeHwnd() && m_hookMouse != NULL)
	{
		::UnhookWindowsHookEx(m_hookMouse);

		m_hookMouse = NULL;
		m_hwndHookedPopup = NULL;
	}

#if !defined _BCGPCHART_STANDALONE
	CBCGPTooltipManager::DeleteToolTip (m_pToolTip);
#endif

	CStatic::OnDestroy();
}
//***********************************************************************************************************
UINT CBCGPBaseVisualCtrl::OnGetDlgCode() 
{
	return m_nDlgCode == 0 ? CStatic::OnGetDlgCode() : m_nDlgCode;
}
//***********************************************************************************************************
void CBCGPBaseVisualCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CStatic::OnKillFocus(pNewWnd);
	
	m_bIsFocused = FALSE;
	RedrawWindow ();
}
//***********************************************************************************************************
void CBCGPBaseVisualCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CStatic::OnSetFocus(pOldWnd);
	
	m_bIsFocused = TRUE;
	RedrawWindow ();
}
//***********************************************************************************************************
void CBCGPBaseVisualCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (!OnKeyboardDown(nChar, nRepCnt, nFlags))
	{
		CStatic::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}
//***********************************************************************************************************
void CBCGPBaseVisualCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (!OnKeyboardUp(nChar, nRepCnt, nFlags))
	{
		CStatic::OnKeyUp(nChar, nRepCnt, nFlags);
	}
}
//***********************************************************************************************************
void CBCGPBaseVisualCtrl::OnNcDestroy() 
{
	CStatic::OnNcDestroy();

	if (m_bIsPopup)
	{
		delete this;
	}
}
//***********************************************************************************************************
BOOL CBCGPBaseVisualCtrl::OnNcCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (!CStatic::OnNcCreate(lpCreateStruct))
		return FALSE;
	
	if (m_bIsPopup && (GetStyle() & MFS_SYNCACTIVE))
	{
		// syncronize activation state with top level parent
		CWnd* pParentWnd = GetTopLevelParent();
		ASSERT(pParentWnd != NULL);
		CWnd* pActiveWnd = GetForegroundWindow();
		BOOL bActive = (pParentWnd == pActiveWnd) ||
			(pParentWnd->GetLastActivePopup() == pActiveWnd &&
			 pActiveWnd->SendMessage(WM_FLOATSTATUS, FS_SYNCACTIVE) != 0);

		// the WM_FLOATSTATUS does the actual work
		SendMessage(WM_FLOATSTATUS, bActive ? FS_ACTIVATE : FS_DEACTIVATE);
	}
	
	return TRUE;
}
//***********************************************************************************************************
LRESULT CBCGPBaseVisualCtrl::OnFloatStatus(WPARAM wParam, LPARAM)
{
	if (!m_bIsPopup)
	{
		return Default();
	}

	// these asserts make sure no conflicting actions are requested
	ASSERT(!((wParam & FS_SHOW) && (wParam & FS_HIDE)));
	ASSERT(!((wParam & FS_ENABLE) && (wParam & FS_DISABLE)));
	ASSERT(!((wParam & FS_ACTIVATE) && (wParam & FS_DEACTIVATE)));

	// FS_SYNCACTIVE is used to detect MFS_SYNCACTIVE windows
	LRESULT lResult = 0;

	if ((GetStyle() & MFS_SYNCACTIVE) && (wParam & FS_SYNCACTIVE))
	{
		lResult = 1;
	}

	if (wParam & (FS_SHOW|FS_HIDE))
	{
		SetWindowPos(NULL, 0, 0, 0, 0,
			((wParam & FS_SHOW) ? SWP_SHOWWINDOW : SWP_HIDEWINDOW) | SWP_NOZORDER |
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
	}

	if (wParam & (FS_ENABLE|FS_DISABLE))
	{
		EnableWindow((wParam & FS_ENABLE) != 0);
	}

	if ((wParam & (FS_ACTIVATE|FS_DEACTIVATE)) && GetStyle() & MFS_SYNCACTIVE)
	{
		ModifyStyle(MFS_SYNCACTIVE, 0);
		SendMessage(WM_NCACTIVATE, (wParam & FS_ACTIVATE) != 0);
		ModifyStyle(0, MFS_SYNCACTIVE);
	}

	return lResult;
}
//***********************************************************************************************************
LRESULT CBCGPBaseVisualCtrl::OnBCGPostRedraw(WPARAM wp, LPARAM)
{
	BOOL bSetDirty = (BOOL)wp;

	if (bSetDirty)
	{
		SetDirty(bSetDirty);
	}

	RedrawWindow();
	return 0;
}
//***********************************************************************************************************
void CBCGPBaseVisualCtrl::OnDrawLayeredPopup()
{
	CRect rect;
	GetClientRect (rect);

	if (rect.Width () == 0 || rect.Height () == 0)
	{
		return;
	}

	CPoint point (0, 0);
	CSize size (rect.Size ());

	LPBYTE pBits = NULL;
	HBITMAP hBitmap = CBCGPDrawManager::CreateBitmap_32 (size, (void**)&pBits);
	if (hBitmap == NULL)
	{
		return;
	}

	CBitmap bitmap;
	bitmap.Attach (hBitmap);

	CClientDC clientDC(this);
	CDC dc;
	dc.CreateCompatibleDC (&clientDC);

	CBitmap* pBitmapOld = (CBitmap*)dc.SelectObject (&bitmap);

	if (m_pGM == NULL)
	{
		CBCGPGraphicsManagerParams params;
		params.bAlphaModePremultiplied = TRUE;

		m_pGM = CBCGPGraphicsManager::CreateInstance(CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_DEFAULT, TRUE, &params);
	
		if (m_pGM == NULL)
		{
			return;
		}

		m_pGM->EnableTransparentGradient();
	}

	SetRect(rect);

	m_pGM->BindDC(&dc, rect);

	if (!m_pGM->BeginDraw())
	{
		return;
	}

	OnDraw(m_pGM, rect);

	SetDirty(FALSE);

	m_pGM->EndDraw();

	m_pGM->BindDC(NULL);

	BLENDFUNCTION bf;
	bf.BlendOp             = AC_SRC_OVER;
	bf.BlendFlags          = 0;
	bf.SourceConstantAlpha = (BYTE)bcg_clamp(m_nPopupAlpha, 0, 255);
	bf.AlphaFormat         = LWA_COLORKEY;

#ifndef _BCGSUITE_
	globalData.UpdateLayeredWindow (GetSafeHwnd (), NULL, 0, &size, dc.GetSafeHdc (), 
		&point, 0, &bf, 0x02);
#else
	UpdateLayeredWindow (NULL, 0, &size, &dc, &point, 0, &bf, 0x02);
#endif

	dc.SelectObject (pBitmapOld);
}
//***********************************************************************************************************
void CBCGPBaseVisualCtrl::SetScale(double dblScale)
{
	m_dblScale = dblScale;
	SetDirty(TRUE);
}
//***********************************************************************************************************
BOOL CBCGPBaseVisualCtrl::DoPrint(CDC* pDCIn, CPrintInfo* pInfo)
{
	CObject* pBaseVisual = GetVisualBaseObject();
	if (pBaseVisual == NULL)
	{
		return FALSE;
	}

	CBCGPBaseVisualObject* pVisualObject = DYNAMIC_DOWNCAST(CBCGPBaseVisualObject, pBaseVisual);
	CBCGPVisualContainer* pVisualContainer = DYNAMIC_DOWNCAST(CBCGPVisualContainer, pBaseVisual);

	if (pVisualObject == NULL && pVisualContainer == NULL)
	{
		return FALSE;
	}

	CBCGPGraphicsManager* pGM = GetGraphicsManager();
	if (pGM == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pGM);

	CDC* pDC = pDCIn;
	CPrintDialog* pPrintDlg = NULL;

	if (pDC == NULL)
	{
		pPrintDlg = new CPrintDialog(FALSE);
		
		if (pPrintDlg->DoModal() == IDCANCEL)
		{
			delete pPrintDlg;
			return FALSE;
		}

		pDC = CDC::FromHandle(pPrintDlg->GetPrinterDC());
		pDC->m_bPrinting = TRUE;
		
		CString strTitle;
		strTitle.LoadString(AFX_IDS_APP_TITLE);
		
		DOCINFO di;
		ZeroMemory(&di, sizeof (DOCINFO));
		di.cbSize = sizeof (DOCINFO);
		di.lpszDocName = strTitle;
		
		if (!pDC->StartDoc(&di))
		{	
			pDC->AbortDoc();
			delete pPrintDlg;
			return FALSE;
		}

		pInfo = new CPrintInfo();
		pInfo->m_rectDraw.SetRect(0,0, pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES));
	}

	CSize size(pInfo->m_rectDraw.Size());
	CBCGPRect rect(CBCGPPoint(), size);
	CBCGPRect rectControl(pVisualObject != NULL ? pVisualObject->GetRect() : pVisualContainer->GetRect());

	CBCGPSize sizeScaleRatio((double)size.cx / rectControl.Width(), (double)size.cy / rectControl.Height());

	double dAspect = min(sizeScaleRatio.cx, sizeScaleRatio.cy);
	sizeScaleRatio.cx = sizeScaleRatio.cy = dAspect;

	size.cx = bcg_clamp((int)(rectControl.Width() * dAspect), 0, size.cx);
	size.cy = bcg_clamp((int)(rectControl.Height() * dAspect), 0, size.cy);
	
	rect.SetSize(size);

	CDC	dcPrint;
	CBitmap	bmpPrint;
	CBitmap* pBmpPrintOld = NULL;
	CDC* pDCOut = NULL;

	CBrush brWhite(RGB(255, 255, 255));

	if (DYNAMIC_DOWNCAST(CBCGPGraphicsManagerGDI, pGM) != NULL)
	{
		dcPrint.CreateCompatibleDC(NULL);
		bmpPrint.Attach(CBCGPDrawManager::CreateBitmap_32(size, NULL));
		
		pBmpPrintOld = dcPrint.SelectObject (&bmpPrint);
		ASSERT(pBmpPrintOld != NULL);

		pDCOut = &dcPrint;
	}
	else
	{
		pGM->SetPrintInfo(pInfo);
		pDCOut = pDC;
	}

	pDCOut->FillRect((CRect)rect, &brWhite);
	pGM->BindDC(pDCOut);

	if (pGM->IsBindDCFailed())
	{
		pDCOut->FillRect((CRect)rect, &brWhite);
		pDCOut->TextOut(0, 0, _T("Out of memory"));
	}
	else
	{
		pGM->BeginDraw();

		CBCGPSize sizeScaleRatioOld;
		
		if (pVisualObject != NULL)
		{
			sizeScaleRatioOld = pVisualObject->GetScaleRatio();
			pVisualObject->SetScaleRatio(sizeScaleRatio);
			pVisualObject->SetRect(rect);
		}
		else
		{
			sizeScaleRatioOld = pVisualContainer->GetScaleRatio();
			pVisualContainer->SetScaleRatio(sizeScaleRatio);
			pVisualContainer->SetRect(rect);
		}

		pGM->Clear();

		if (pVisualObject != NULL)
		{
			pVisualObject->OnDraw (pGM, rect);
		}
		else
		{
			pVisualContainer->OnDraw (pGM, rect);
		}

		pGM->EndDraw();

		pGM->BindDC(NULL);

		if (pVisualObject != NULL)
		{
			pVisualObject->SetScaleRatio(sizeScaleRatioOld);
		}
		else
		{
			pVisualContainer->SetScaleRatio(sizeScaleRatioOld);
		}

		if (dcPrint.GetSafeHdc() != NULL)
		{
			CBCGPDrawManager::FillAlpha (rect, (HBITMAP)bmpPrint.GetSafeHandle (), 255);
		}
	}

	if (pVisualObject != NULL)
	{
		pVisualObject->SetRect(rectControl);
		pVisualObject->SetDirty ();
	}
	else
	{
		pVisualContainer->SetRect(rectControl);
		pVisualContainer->SetDirty ();
	}

	if (dcPrint.GetSafeHdc() != NULL)
	{
		BITMAP bmp = {0};
		bmpPrint.GetBitmap (&bmp);

		CRect rectOut(pInfo->m_rectDraw.TopLeft (), CSize(bmp.bmWidth, bmp.bmHeight));

		int mode = pDC->SetStretchBltMode(HALFTONE);

		pDC->BitBlt (rectOut.left, rectOut.top, rectOut.Width(), rectOut.Height(), &dcPrint, 0, 0, SRCCOPY);
		pDC->SetStretchBltMode(mode);

		dcPrint.SelectObject(pBmpPrintOld);
	}
	else
	{
		pGM->SetPrintInfo(NULL);
	}

	if (pPrintDlg != NULL)
	{
		pDC->EndDoc();
		delete pPrintDlg;
		delete pInfo;
	}

	return TRUE;
}
//*******************************************************************************************************************
LRESULT CBCGPBaseVisualCtrl::OnGestureEvent(WPARAM wp, LPARAM lp)
{
	if (ProcessGestureEvent(this, wp, lp))
	{
		return Default();
	}
	
	return 0;
}
//******************************************************************************************************************
LRESULT CBCGPBaseVisualCtrl::OnTabletQuerySystemGestureStatus(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return (LRESULT)TABLET_DISABLE_PRESSANDHOLD;
}
//******************************************************************************************************************
LRESULT CBCGPBaseVisualCtrl::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if (m_bInPaint)
	{
		return Default();
	}

	if ((lp & PRF_CLIENT) == PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		DoPaint(pDC, CRect(0, 0, 0, 0));
		return 0;
	}

	return Default();
}
//***********************************************************************************************************
LRESULT CBCGPBaseVisualCtrl::OnGetObject(WPARAM wParam, LPARAM lParam)
{
#ifndef _BCGPCHART_STANDALONE
	if (globalData.IsAccessibilitySupport ())
	{
		
        if ((DWORD)lParam == OBJID_CLIENT)
		{
			CBCGPBaseAccessibleObject* pVisualObject = DYNAMIC_DOWNCAST(CBCGPBaseAccessibleObject, GetVisualBaseObject());
			if (pVisualObject == NULL || !pVisualObject->IsAccessibilityCompatible())
			{
				return 0;
			}
			

			LPUNKNOWN pUnknown = pVisualObject->GetInterface(&IID_IAccessible);
			if (pUnknown == NULL)
			{
				return 0;
			}
			
			pVisualObject->OnSetIAccessible((IAccessible*)pUnknown);
			
			return LresultFromObject(IID_IAccessible, wParam, pUnknown);
		}
	}
#endif	
	return (LRESULT)0L;
}	
