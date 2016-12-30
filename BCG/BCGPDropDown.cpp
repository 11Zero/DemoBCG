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

// BCGPDropDown.cpp : implementation file
//

#include "stdafx.h"
#include "multimon.h"

#include <afxpriv.h>
#include "mmsystem.h"
#include "BCGPDropDown.h"
#include "BCGGlobals.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPFrameWnd.h"
#include "BCGPMenuBar.h"
#include "bcgpsound.h"
#include "BCGPToolbarMenuButton.h"
#include "TrackMouse.h"
#include "BCGPVisualManager.h"
#include "BCGPDrawManager.h"
#include "bcgprores.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const UINT uiShowBarTimerId = 1;
static const int nArrowSize = 7;

UINT CBCGPDropDownToolbarButton::m_uiShowBarDelay = 500;	// ms

IMPLEMENT_SERIAL(CBCGPDropDownToolBar, CBCGPToolBar, 1)

extern CObList	gAllToolbars;

BOOL CBCGPDropDownToolBar::OnSendCommand(const CBCGPToolbarButton* pButton)
{
	ASSERT_VALID (pButton);
	
	if ((pButton->m_nStyle & TBBS_DISABLED) != 0 ||
		pButton->m_nID == 0 || pButton->m_nID == (UINT)-1)
	{
		return FALSE;
	}
	
	CBCGPDropDownFrame* pParent = (CBCGPDropDownFrame*)GetParent();
	ASSERT_KINDOF(CBCGPDropDownFrame, pParent);
	
	pParent->m_pParentBtn->SetDefaultCommand (pButton->m_nID);
	
	//----------------------------------
	// Send command to the parent frame:
	//----------------------------------
	CFrameWnd* pParentFrame = GetParentFrame ();
	ASSERT_VALID (pParentFrame);
	
	GetOwner()->PostMessage(WM_COMMAND, pButton->m_nID);
	pParentFrame->DestroyWindow ();
	return TRUE;
}
//*************************************************************************************
void CBCGPDropDownToolBar::OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
{
	CBCGPToolBar::OnUpdateCmdUI ((CFrameWnd*)GetCommandTarget(), bDisableIfNoHndler);
}

BEGIN_MESSAGE_MAP(CBCGPDropDownToolBar, CBCGPToolBar)
//{{AFX_MSG_MAP(CBCGPDropDownToolBar)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CBCGPDropDownToolBar::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	if (m_ptLastMouse != CPoint (-1, -1) &&
		abs (m_ptLastMouse.x - point.x) < 1 &&
		abs (m_ptLastMouse.y - point.y) < 1)
	{
		m_ptLastMouse = point;
		return;
	}
	
	m_ptLastMouse = point;
	
	int iPrevHighlighted = m_iHighlighted;
	m_iHighlighted = HitTest (point);
	
	CBCGPToolbarButton* pButton = m_iHighlighted == -1 ?
		NULL : GetButton (m_iHighlighted);
	if (pButton != NULL &&
		(pButton->m_nStyle & TBBS_SEPARATOR || 
		(pButton->m_nStyle & TBBS_DISABLED && !AllowSelectDisabled ())))
	{
		m_iHighlighted = -1;
	}
	
	if (!m_bTracked)
	{
		m_bTracked = TRUE;
		
		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		BCGPTrackMouse (&trackmouseevent);	
	}
	
	if (iPrevHighlighted != m_iHighlighted)
	{
		BOOL bNeedUpdate = FALSE;
		
		m_iButtonCapture = m_iHighlighted;
		if (iPrevHighlighted != -1)
		{
			CBCGPToolbarButton* pTBBCapt = GetButton (iPrevHighlighted);
			ASSERT (pTBBCapt != NULL);
			ASSERT (!(pTBBCapt->m_nStyle & TBBS_SEPARATOR));
			
			UINT nNewStyle = (pTBBCapt->m_nStyle & ~TBBS_PRESSED);
			
			if (nNewStyle != pTBBCapt->m_nStyle)
			{
				SetButtonStyle (iPrevHighlighted, nNewStyle);
			}
			
		}
		
		if (m_iButtonCapture != -1)
		{
			CBCGPToolbarButton* pTBBCapt = GetButton (m_iButtonCapture);
			ASSERT (pTBBCapt != NULL);
			ASSERT (!(pTBBCapt->m_nStyle & TBBS_SEPARATOR));
			
			UINT nNewStyle = (pTBBCapt->m_nStyle & ~TBBS_PRESSED);
			if (m_iHighlighted == m_iButtonCapture)
			{
				nNewStyle |= TBBS_PRESSED;
			}
			
			if (nNewStyle != pTBBCapt->m_nStyle)
			{
				SetButtonStyle (m_iButtonCapture, nNewStyle);
				bNeedUpdate = TRUE;
			}
		}
		
		if ((m_iButtonCapture == -1 || 
			iPrevHighlighted == m_iButtonCapture) &&
			iPrevHighlighted != -1)
		{
			InvalidateButton (iPrevHighlighted);
			bNeedUpdate = TRUE;
		}
		
		if ((m_iButtonCapture == -1 || 
			m_iHighlighted == m_iButtonCapture) &&
			m_iHighlighted != -1)
		{
			InvalidateButton (m_iHighlighted);
			bNeedUpdate = TRUE;
		}
		
		if (bNeedUpdate)
		{
			UpdateWindow ();
		}
		
		if (m_iHighlighted != -1 && 
			(m_iHighlighted == m_iButtonCapture || m_iButtonCapture == -1))
		{
			ASSERT (pButton != NULL);
			ShowCommandMessageString (pButton->m_nID);
		}
		else if (m_iButtonCapture == -1 && m_hookMouseHelp == NULL)
		{
			GetOwner()->SendMessage (WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
		}
		
		OnChangeHot (m_iHighlighted);
	}
}
//****************************************************************************************
void CBCGPDropDownToolBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CRect rectClient;
	GetClientRect (&rectClient);
	
	if (!m_bCustomizeMode && 
		!rectClient.PtInRect (point))
	{
		CFrameWnd* pParentFrame = GetParentFrame ();
		ASSERT_VALID (pParentFrame);
		
		pParentFrame->DestroyWindow ();
		return;
	}
	
	if (!m_bCustomizeMode && m_iHighlighted >= 0)
	{
		m_iButtonCapture = m_iHighlighted;

		CBCGPToolbarButton* pButton = GetButton (m_iHighlighted);
		ASSERT_VALID (pButton);

		pButton->m_nStyle &= ~TBBS_PRESSED;
	}
	
	CBCGPToolBar::OnLButtonUp (nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPDropDownFrame

static const int iBorderSize = 2;

CString	CBCGPDropDownFrame::m_strClassName;

IMPLEMENT_SERIAL(CBCGPDropDownFrame, CMiniFrameWnd, VERSIONABLE_SCHEMA | 1)

CBCGPDropDownFrame::CBCGPDropDownFrame()
{
	m_x = m_y = 0;
	m_pParentBtn = NULL;
	m_bAutoDestroyParent = TRUE;
	m_bAutoDestroy = TRUE;
	m_pWndOriginToolbar = NULL;
}
//****************************************************************************************
CBCGPDropDownFrame::~CBCGPDropDownFrame()
{
	m_wndToolBar.m_Buttons.RemoveAll ();	// toolbar has references to original buttons!

	if (m_bAutoDestroy)
	{
		m_wndToolBar.DestroyWindow();
	}
}

BEGIN_MESSAGE_MAP(CBCGPDropDownFrame, CMiniFrameWnd)
//{{AFX_MSG_MAP(CBCGPDropDownFrame)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_MOUSEACTIVATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
//}}AFX_MSG_MAP
	ON_WM_ACTIVATEAPP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPDropDownFrame message handlers

BOOL CBCGPDropDownFrame::Create (CWnd* pWndParent, int x, int y, CBCGPDropDownToolBar* pWndOriginToolbar)
{
	ASSERT_VALID (pWndOriginToolbar);
	ASSERT (pWndParent != NULL);
	
	BCGPlaySystemSound (BCGSOUND_MENU_POPUP);
	
	if (m_strClassName.IsEmpty ())
	{
		m_strClassName = ::AfxRegisterWndClass (
			CS_SAVEBITS,
			::LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)(COLOR_BTNFACE + 1), NULL);
	}

	m_pWndOriginToolbar = pWndOriginToolbar;

	if (x == -1 && y == -1)	// Undefined position
	{
		if (pWndParent->GetSafeHwnd() != NULL)
		{
			CRect rectParent;
			pWndParent->GetClientRect (&rectParent);
			pWndParent->ClientToScreen (&rectParent);
			
			m_x = rectParent.left + 5;
			m_y = rectParent.top + 5;
		}
		else
		{
			m_x = 0;
			m_y = 0;
		}
	}
	else
	{
		m_x = x;
		m_y = y;
	}
	
	DWORD dwStyle = WS_POPUP;
	DWORD dwStyleEx = 0;

	if (pWndParent->GetSafeHwnd() != NULL && (pWndParent->GetExStyle () & WS_EX_LAYOUTRTL))
	{
		dwStyleEx = WS_EX_LAYOUTRTL;
	}
	
	CRect rect (x, y, x, y);
	BOOL bCreated = CMiniFrameWnd::CreateEx (
		dwStyleEx,
		m_strClassName, m_strCaption,
		dwStyle, rect,
		pWndParent->GetOwner () == NULL ? 
			pWndParent : pWndParent->GetOwner ());
	if (!bCreated)
	{
		return FALSE;
	}
	
	ShowWindow (SW_SHOWNOACTIVATE);
	return TRUE;
}
//****************************************************************************************
int CBCGPDropDownFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	ASSERT_VALID (m_pWndOriginToolbar);
	ASSERT (m_pWndOriginToolbar->m_bLocked);

	if (CMiniFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CBCGPToolBar* pParentBar = m_pParentBtn == NULL ? NULL :
		DYNAMIC_DOWNCAST (CBCGPToolBar, m_pParentBtn->m_pWndParent);
	
	BOOL bHorz = pParentBar == NULL ? TRUE : pParentBar->IsHorizontal ();
	DWORD style = bHorz? CBRS_ORIENT_VERT : CBRS_ORIENT_HORZ;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | style, 
		CRect(1, 1, 1, 1), AFX_IDW_TOOLBAR + 39))
	{
		TRACE(_T ("Can't create toolbar bar\n"));
		return -1;
	}

	m_wndToolBar.m_bLocked = TRUE;

	//------------------------------
	// "Clone" the original toolbar:
	//------------------------------
	m_pWndOriginToolbar->m_ImagesLocked.CopyTemp (m_wndToolBar.m_ImagesLocked);
	m_pWndOriginToolbar->m_ColdImagesLocked.CopyTemp (m_wndToolBar.m_ColdImagesLocked);
	m_pWndOriginToolbar->m_DisabledImagesLocked.CopyTemp (m_wndToolBar.m_DisabledImagesLocked);
	m_pWndOriginToolbar->m_LargeImagesLocked.CopyTemp(m_wndToolBar.m_LargeImagesLocked);
	m_pWndOriginToolbar->m_LargeColdImagesLocked.CopyTemp(m_wndToolBar.m_LargeColdImagesLocked);
	m_pWndOriginToolbar->m_LargeDisabledImagesLocked.CopyTemp(m_wndToolBar.m_LargeDisabledImagesLocked);

	m_wndToolBar.m_sizeButtonLocked = m_pWndOriginToolbar->m_sizeButtonLocked;
	m_wndToolBar.m_sizeImageLocked = m_pWndOriginToolbar->m_sizeImageLocked;
	m_wndToolBar.m_sizeCurButtonLocked = m_pWndOriginToolbar->m_sizeCurButtonLocked;
	m_wndToolBar.m_sizeCurImageLocked = m_pWndOriginToolbar->m_sizeCurImageLocked;

	m_wndToolBar.m_dwStyle &= ~CBRS_GRIPPER;

	m_wndToolBar.SetOwner (m_pWndOriginToolbar->GetOwner());
	m_wndToolBar.SetRouteCommandsViaFrame (m_pWndOriginToolbar->GetRouteCommandsViaFrame ());

	m_wndToolBar.m_Buttons.AddTail (&m_pWndOriginToolbar->m_Buttons);
		
	RecalcLayout ();
	::ReleaseCapture();
	m_wndToolBar.SetCapture ();

	return 0;
}
//****************************************************************************************
void CBCGPDropDownFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMiniFrameWnd::OnSize(nType, cx, cy);
	
	if (m_wndToolBar.GetSafeHwnd () != NULL)
	{
		m_wndToolBar.SetWindowPos (NULL, iBorderSize, iBorderSize, 
			cx - iBorderSize * 2, cy - iBorderSize * 2, 
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
}
//****************************************************************************************
void CBCGPDropDownFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rectClient;	// Client area rectangle
	GetClientRect (&rectClient);

	dc.FillRect(rectClient, &globalData.brBarFace);
	
	CRect rectBorderSize(iBorderSize, iBorderSize, iBorderSize, iBorderSize);
	CBCGPVisualManager::GetInstance()->OnDrawFloatingToolbarBorder(&dc, &m_wndToolBar, rectClient, rectBorderSize);
}
//****************************************************************************************
int CBCGPDropDownFrame::OnMouseActivate(CWnd* /*pDesktopWnd*/, UINT /*nHitTest*/, UINT /*message*/) 
{
	return MA_NOACTIVATE;
}
//****************************************************************************************
void CBCGPDropDownFrame::RecalcLayout (BOOL /*bNotify*/) 
{
#ifdef _DEBUG
	if (m_pParentBtn != NULL)
	{
		ASSERT_VALID (m_pParentBtn);
		ASSERT (m_pParentBtn->m_pPopupMenu == this);
	}
#endif // _DEBUG
	
	if (!::IsWindow (m_hWnd) ||
		!::IsWindow (m_wndToolBar.m_hWnd))
	{
		return;
	}
	
	CBCGPToolBar* pParentBar = m_pParentBtn == NULL ? NULL :
	DYNAMIC_DOWNCAST (CBCGPToolBar, m_pParentBtn->m_pWndParent);
	
	BOOL bHorz = pParentBar->IsHorizontal ();
	
	CSize size = m_wndToolBar.CalcSize(bHorz);
	size.cx += iBorderSize * 3;
	size.cy += iBorderSize * 4;
	
	//---------------------------------------------
	// Adjust the menu position by the screen size:
	//---------------------------------------------
	CRect rectScreen;

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);
	if (GetMonitorInfo (MonitorFromPoint (CPoint (m_x, m_y), MONITOR_DEFAULTTONEAREST),
		&mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	if (m_x + size.cx > rectScreen.right)
	{
		//-----------------------------------------------------
		// Menu can't be overlapped with the parent popup menu!
		//-----------------------------------------------------
		CBCGPToolBar* pParentBar = m_pParentBtn == NULL ? NULL :
			DYNAMIC_DOWNCAST (CBCGPToolBar, m_pParentBtn->m_pWndParent);
		
		if (pParentBar != NULL && 
			(pParentBar->IsHorizontal ()) == 0)
		{
			//------------------------------------------------
			// Parent menu bar is docked vertical, place menu 
			// in the left or right side of the parent frame:
			//------------------------------------------------
			CRect rectParent;
			pParentBar->GetWindowRect (rectParent);
			
			m_x = rectParent.left - size.cx;
		}
		else
		{
			m_x = rectScreen.Width () - size.cx - 1;
		}
	}
	
	if (m_y + size.cy > rectScreen.bottom)
	{
		m_y -= size.cy;
		
		if (m_pParentBtn != NULL)
		{
			m_y -= m_pParentBtn->m_rect.Height () + 4;
		}
		else if (m_y < 0)
		{
			m_y = 0;
		}
	}
	
	SetWindowPos (NULL, m_x, m_y, size.cx, size.cy,
		SWP_NOZORDER | SWP_NOACTIVATE);
}
//****************************************************************************************
void CBCGPDropDownFrame::OnDestroy() 
{
	if (m_pParentBtn != NULL)
	{
		ASSERT (m_pParentBtn->m_pPopupMenu == this);
		
		m_pParentBtn->m_pPopupMenu = NULL;
		m_pParentBtn->m_nStyle = m_pParentBtn->m_nStyle & ~TBBS_PRESSED;
		
		CBCGPToolBar* pparentBar = DYNAMIC_DOWNCAST(CBCGPToolBar, m_pParentBtn->m_pWndParent);
		if (pparentBar)
		{
			CPoint point;
			::GetCursorPos(&point);
			
			pparentBar->ScreenToClient(&point);
			pparentBar->SendMessage(WM_LBUTTONUP, NULL, MAKELONG(point.x, point.y));
		}
	}
	
	CMiniFrameWnd::OnDestroy();
}
//****************************************************************************************
void CBCGPDropDownFrame::PostNcDestroy() 
{
	if (m_pParentBtn != NULL)
	{
		m_pParentBtn->OnCancelMode ();
	}
	
	CMiniFrameWnd::PostNcDestroy();
}
//****************************************************************************************
CBCGPDropDownFrame* CBCGPDropDownFrame::GetParentPopupMenu () const
{
	if (m_pParentBtn == NULL)
	{
		return NULL;
	}
	
	CBCGPPopupMenuBar* pParentBar = 
		DYNAMIC_DOWNCAST (CBCGPPopupMenuBar, m_pParentBtn->m_pWndParent);
	if (pParentBar != NULL)
	{
		CBCGPDropDownFrame* pParentMenu =
			DYNAMIC_DOWNCAST (CBCGPDropDownFrame, pParentBar->GetParentFrame ());
		ASSERT_VALID (pParentMenu);
		
		return pParentMenu;
	}
	else
	{
		return NULL;
	}
}
//****************************************************************************************
CBCGPMenuBar* CBCGPDropDownFrame::GetParentMenuBar () const
{
	if (m_pParentBtn == NULL)
	{
		return NULL;
	}
	
	CBCGPMenuBar* pParentBar = 
		DYNAMIC_DOWNCAST (CBCGPMenuBar, m_pParentBtn->m_pWndParent);
	return pParentBar;
}
//****************************************************************************************
BOOL CBCGPDropDownFrame::OnEraseBkgnd(CDC* pDC) 
{
	CRect rectClient;	// Client area rectangle
	GetClientRect (&rectClient);
	
	pDC->FillSolidRect (rectClient, globalData.clrBarFace);
	return TRUE;
}
//************************************************************************************
#if _MSC_VER >= 1300
void CBCGPDropDownFrame::OnActivateApp(BOOL bActive, DWORD /*dwThreadID*/)
#else
void CBCGPDropDownFrame::OnActivateApp(BOOL bActive, HTASK /*hTask*/) 
#endif
{
	if (!bActive && !CBCGPToolBar::IsCustomizeMode ())
	{
		SendMessage (WM_CLOSE);
	}
}

IMPLEMENT_SERIAL(CBCGPDropDownToolbarButton, CBCGPToolbarButton, VERSIONABLE_SCHEMA | 1)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPDropDownToolbarButton::CBCGPDropDownToolbarButton()
{
	m_pToolBar = NULL;
	m_pPopupMenu = NULL;
	m_pWndParent = NULL;
	m_uiTimer = 0;
	m_bLocked = TRUE;
	m_iSelectedImage = 0;
	m_bInternalDraw = FALSE;
	m_bLocalUserButton = FALSE;
}
//*****************************************************************************************
CBCGPDropDownToolbarButton::CBCGPDropDownToolbarButton (LPCTSTR lpszName, CBCGPDropDownToolBar* pToolBar)
{
	ASSERT (lpszName != NULL);
	m_strName = lpszName;

	m_uiTimer = 0;

	m_pPopupMenu = NULL;
	m_pWndParent = NULL;

	ASSERT_VALID (pToolBar);
	m_pToolBar = pToolBar;

	CBCGPToolbarButton* pbutton = pToolBar->GetButton (0);
	if (pbutton == NULL)	// Toolbar is empty!
	{
		ASSERT (FALSE);
	}
	else
	{
		CBCGPToolbarButton::CopyFrom (*pbutton);
	}

	m_iSelectedImage = 0;

	m_bLocalUserButton = FALSE;
}
//*****************************************************************************************
CBCGPDropDownToolbarButton::~CBCGPDropDownToolbarButton()
{
}
//****************************************************************************************
void CBCGPDropDownToolbarButton::SetDefaultCommand (UINT uiCmd)
{
	ASSERT_VALID (m_pToolBar);

	m_nID = uiCmd;

	//------------------
	// Find image index:
	//------------------
	int iImage = 0;
	m_iSelectedImage = -1;

	for (int i = 0; i < m_pToolBar->GetCount (); i ++)
	{
		CBCGPToolbarButton* pButton = m_pToolBar->GetButton (i);
		ASSERT_VALID (pButton);

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			continue;
		}

		if (pButton->m_nID == uiCmd)
		{
 			m_bLocalUserButton = pButton->m_bUserButton;
			m_strSelectedText = pButton->m_strText;

 			if (m_bLocalUserButton)
			{
 				m_iSelectedImage = pButton->GetImage();
			}
 			else
			{
				m_iSelectedImage = iImage;
			}
			break;
		}

		iImage ++;
	}

	if (m_iSelectedImage == -1)
	{
		ASSERT (FALSE);
		m_iSelectedImage = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// Overrides:

void CBCGPDropDownToolbarButton::CopyFrom (const CBCGPToolbarButton& s)
{
	CBCGPToolbarButton::CopyFrom (s);

	const CBCGPDropDownToolbarButton& src = (const CBCGPDropDownToolbarButton&) s;

	m_pToolBar = src.m_pToolBar;
	m_strName = src.m_strName;
	m_iSelectedImage = src.m_iSelectedImage;

	m_bDragFromCollection = FALSE;
}
//*****************************************************************************************
void CBCGPDropDownToolbarButton::Serialize (CArchive& ar)
{
	CBCGPToolbarButton::Serialize (ar);
	
	UINT uiToolbarResID = 0;

	if (ar.IsLoading ())
	{
		m_pToolBar = NULL;

		ar >> uiToolbarResID;
		ar >> m_strName;
		ar >> m_iSelectedImage;

		// Find toolbar with required resource ID:
		for (POSITION pos = gAllToolbars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPDropDownToolBar* pToolBar = DYNAMIC_DOWNCAST (CBCGPDropDownToolBar,
				gAllToolbars.GetNext (pos));

			if (pToolBar != NULL &&
				CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID (pToolBar);
				if (pToolBar->m_uiOriginalResID == uiToolbarResID)
				{
					m_pToolBar = pToolBar;
					break;
				}
			}
		}

		SetDefaultCommand (m_nID);
	}
	else
	{
		if (m_pToolBar == NULL)
		{
			ASSERT (FALSE);
		}
		else
		{
			ASSERT_VALID (m_pToolBar);
			uiToolbarResID = m_pToolBar->m_uiOriginalResID;
		}

		ar << uiToolbarResID;
		ar << m_strName;
		ar << m_iSelectedImage;
	}
}
//*****************************************************************************************
void CBCGPDropDownToolbarButton::OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* /*pImages*/,
										BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight, BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	ASSERT_VALID (pDC);
	
	//----------------------
	// Fill button interior:
	//----------------------
	FillInterior (pDC, rect, bHighlight);

	int nCurrArrowSize = nArrowSize;
	if (globalData.GetRibbonImageScale () != 1.)
	{
		nCurrArrowSize = (int) (globalData.GetRibbonImageScale () * nCurrArrowSize);
	}
	
	int nActualArrowSize = 
		CBCGPToolBar::IsLargeIcons () ? nCurrArrowSize * 2 : nCurrArrowSize;
	int nHalfArrowSize = 
		CBCGPToolBar::IsLargeIcons () ? nCurrArrowSize : nCurrArrowSize / 2 + 1;

	CRect rectParent = rect;
	rectParent.right -= nActualArrowSize / 2 + 1;

	if (m_pToolBar != NULL)
	{
		CBCGPDrawState ds(CBCGPVisualManager::GetInstance()->IsAutoGrayscaleImages());

		BOOL bImage = m_bImage;
		m_bInternalDraw = TRUE;

		CSize sizeDest = m_pToolBar->GetImageSize ();
		if (globalData.GetRibbonImageScale () != 1.)
		{
			double dblImageScale = globalData.GetRibbonImageScale ();
			sizeDest = CSize ((int)(.5 + sizeDest.cx * dblImageScale), (int)(.5 + sizeDest.cy * dblImageScale));
		}

		CBCGPToolBarImages& images = (m_pToolBar->m_bLargeIcons && m_pToolBar->m_LargeImagesLocked.GetCount () > 0) ?
				m_pToolBar->m_LargeImagesLocked : m_pToolBar->m_ImagesLocked;

		BOOL bImageWasPrepared = FALSE;

		if (!m_bLocalUserButton)
		{
			images.SetTransparentColor (globalData.clrBtnFace);
			bImageWasPrepared = images.PrepareDrawImage (ds, sizeDest);
		}
		else
		{
			m_pToolBar->m_pUserImages->SetTransparentColor (globalData.clrBtnFace);
			bImageWasPrepared = m_pToolBar->m_pUserImages->PrepareDrawImage (ds, sizeDest);
		}
	
		m_iImage	 = m_iSelectedImage;
		m_iUserImage = m_iSelectedImage;
		m_bImage = TRUE;

		CString strTextSaved;

		if (m_bTextBelow && bHorz)
		{
			strTextSaved = m_strText;

			if (m_strSelectedText.IsEmpty ())
			{
				CString strMessage;
				int iOffset;
				
				if (strMessage.LoadString (m_nID) && (iOffset = strMessage.Find (_T('\n'))) != -1)
				{
					m_strSelectedText = strMessage.Mid (iOffset + 1);
				}
			}

			m_strText = m_strSelectedText;
		}

		BOOL bDisableFill = m_bDisableFill;
		m_bDisableFill = TRUE;

		if (m_bLocalUserButton)
		{
			m_bUserButton = m_bLocalUserButton;
			CBCGPToolbarButton::OnDraw(pDC, rect, m_pToolBar->m_pUserImages, bHorz,  bCustomizeMode, bHighlight,  FALSE, bGrayDisabledButtons);
			m_bUserButton = FALSE;
		}
		else
		{
			CBCGPToolbarButton::OnDraw (pDC, rectParent, 
									&images, bHorz, bCustomizeMode, bHighlight, 
									FALSE, bGrayDisabledButtons);
		}

		if (m_bTextBelow && bHorz)
		{
			m_strText = strTextSaved;
		}

		m_bDisableFill = bDisableFill;
		m_iImage = -1;
		m_iUserImage = -1;
		m_bImage = bImage;

		if (bImageWasPrepared)
		{
			if (!m_bLocalUserButton)
			{
				images.EndDrawImage (ds);
			}
			else
			{
				m_pToolBar->m_pUserImages->EndDrawImage (ds);
			}
		}

		m_bInternalDraw = FALSE;
	}

	int offset = (m_nStyle & TBBS_PRESSED) ? 1 : 0;
	
	CPoint triang [] = 
	{
		CPoint( rect.right - nActualArrowSize + offset - 1, rect.bottom - nHalfArrowSize + offset + 1),
		CPoint (rect.right - nHalfArrowSize + offset + 1, rect.bottom - nHalfArrowSize + offset + 1),
		CPoint (rect.right - nHalfArrowSize + offset + 1, rect.bottom - nActualArrowSize + offset - 1)
	};
	
	CPen* pOldPen = (CPen*) pDC->SelectStockObject (NULL_PEN);
	ASSERT (pOldPen != NULL);

	CBrush brFill((m_nStyle & TBBS_DISABLED) ? globalData.clrBarShadow : globalData.clrBarText);

	CBrush* pOldBrush = (CBrush*) pDC->SelectObject (&brFill);
	ASSERT (pOldBrush != NULL);

	pDC->Polygon (triang, 3);

	if (!bCustomizeMode && HaveHotBorder () && bDrawBorder)
	{
		if (m_pPopupMenu != NULL ||
			(m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)))
		{
			//-----------------------
			// Pressed in or checked:
			//-----------------------
			if (m_nID != 0 && m_nID != (UINT) -1)
			{
				CBCGPVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
					this, rect, CBCGPVisualManager::ButtonsIsPressed);
			}
			else
			{
				CBCGPVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
					this, rect, CBCGPVisualManager::ButtonsIsHighlighted);
			}
		}
		else if (bHighlight && !(m_nStyle & TBBS_DISABLED) &&
			!(m_nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			if (m_nStyle & TBBS_PRESSED)
			{
				CBCGPVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
					this, rect, CBCGPVisualManager::ButtonsIsPressed);
			}
			else
			{
				CBCGPVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
					this, rect, CBCGPVisualManager::ButtonsIsHighlighted);
				
			}
		}
	}
	
	pDC->SelectObject (pOldPen);
	pDC->SelectObject (pOldBrush);
}

static CBCGPDropDownToolbarButton* g_pButtonDown = NULL;

static void CALLBACK EXPORT TimerProc(HWND hWnd, UINT, UINT_PTR, DWORD)
{
	CWnd* pwnd = CWnd::FromHandle (hWnd);
	if (g_pButtonDown != NULL)
	{
		g_pButtonDown->OnClick (pwnd, FALSE);
	}
}
//*****************************************************************************************
BOOL CBCGPDropDownToolbarButton::OnClick (CWnd* pWnd, BOOL bDelay)
{
	ASSERT_VALID (pWnd);
	if (m_uiTimer == 0)
	{
		if (m_pWndParent != NULL)
		{
			m_uiTimer = (UINT) m_pWndParent->SetTimer (uiShowBarTimerId, m_uiShowBarDelay, TimerProc);
		}

		g_pButtonDown = this;
		return CBCGPToolbarButton::OnClick (pWnd, bDelay);
	}
	
	if (m_pWndParent != NULL)
	{
		m_pWndParent->KillTimer (m_uiTimer);
	}

	m_uiTimer = 0;
	g_pButtonDown = NULL;
	
	CBCGPMenuBar* pMenuBar = DYNAMIC_DOWNCAST (CBCGPMenuBar, m_pWndParent);
	
	if (m_pPopupMenu != NULL)
	{
		//-----------------------------------------------------
		// Second click to the popup menu item closes the menu:
		//-----------------------------------------------------		
		ASSERT_VALID(m_pPopupMenu);
		
		m_pPopupMenu->m_bAutoDestroyParent = FALSE;
		m_pPopupMenu->DestroyWindow ();
		m_pPopupMenu = NULL;
		
		if (pMenuBar != NULL)
		{
			pMenuBar->SetHot (NULL);
		}
	}
	else
	{
		CBCGPPopupMenuBar* pParentMenu =
			DYNAMIC_DOWNCAST (CBCGPPopupMenuBar, m_pWndParent);
		if (bDelay && pParentMenu != NULL && !CBCGPToolBar::IsCustomizeMode ())
		{
		}
		else
		{
			DropDownToolbar(pWnd);
		}
		
		if (pMenuBar != NULL)
		{
			pMenuBar->SetHot (this);
		}
	}
	
	if (m_pWndParent != NULL)
	{
		m_pWndParent->InvalidateRect (m_rect);
	}
	
	return FALSE;
}
//****************************************************************************************
BOOL CBCGPDropDownToolbarButton::OnClickUp()
{
	CBCGPMenuBar* pMenuBar = DYNAMIC_DOWNCAST (CBCGPMenuBar, m_pWndParent);
	
	if (m_uiTimer)
	{
		if (m_pWndParent != NULL)
		{
			m_pWndParent->KillTimer (m_uiTimer);
		}

		m_uiTimer = 0;
		g_pButtonDown = NULL;
		return FALSE;
	}
	
	if (m_pPopupMenu != NULL)
	{
		//-----------------------------------------------------
		// Second click to the popup menu item closes the menu:
		//-----------------------------------------------------		
		ASSERT_VALID(m_pPopupMenu);
		
		m_pPopupMenu->m_bAutoDestroyParent = FALSE;
		m_pPopupMenu->DestroyWindow ();
		m_pPopupMenu = NULL;
		
		if (pMenuBar != NULL)
		{
			pMenuBar->SetHot (NULL);
		}
	}
	
	return TRUE;
}
//****************************************************************************************
void CBCGPDropDownToolbarButton::OnChangeParentWnd (CWnd* pWndParent)
{
	CBCGPToolbarButton::OnChangeParentWnd (pWndParent);

	m_bText = FALSE;
	m_strText.Empty ();
	m_bUserButton = FALSE;
}
//****************************************************************************************
void CBCGPDropDownToolbarButton::OnCancelMode ()
{
	if (m_pWndParent != NULL && ::IsWindow (m_pWndParent->m_hWnd))
	{
		m_pWndParent->InvalidateRect (m_rect);
		m_pWndParent->UpdateWindow ();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPDropDownToolbarButton diagnostics

#ifdef _DEBUG
void CBCGPDropDownToolbarButton::AssertValid() const
{
	CObject::AssertValid();
}

//******************************************************************************************
void CBCGPDropDownToolbarButton::Dump(CDumpContext& dc) const
{
	CObject::Dump (dc);
}

#endif

//****************************************************************************************
BOOL CBCGPDropDownToolbarButton::DropDownToolbar (CWnd* pWnd)
{
	if (m_pToolBar == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (m_pPopupMenu != NULL)
	{
		return FALSE;
	}
	
	if (pWnd == NULL)
	{
		pWnd = m_pWndParent;
		if (m_pWndParent == NULL)
		{
			return FALSE;
		}
	}
	
	//---------------------------------------------------------------
	// Define a new menu position. Place the menu in the right side
	// of the current menu in the poup menu case or under the current 
	// item by default:
	//---------------------------------------------------------------
	CPoint point;

	BOOL bIsRTL = (m_pWndParent->GetSafeHwnd() != NULL && (m_pWndParent->GetExStyle () & WS_EX_LAYOUTRTL));
	
	CBCGPToolBar* pParentBar = DYNAMIC_DOWNCAST (CBCGPToolBar, m_pWndParent);
	
	if (pParentBar != NULL && !pParentBar->IsHorizontal ())
	{
		//------------------------------------------------
		// Parent menu bar is docked vertical, place menu 
		// in the left or right side of the parent frame:
		//------------------------------------------------
		point = CPoint (bIsRTL ? (m_rect.left - 1) : (m_rect.right + 1), m_rect.top);
		pWnd->ClientToScreen (&point);
	}
	else
	{
		point = CPoint (bIsRTL ? (m_rect.right + 1) : (m_rect.left - 1), m_rect.bottom);
		pWnd->ClientToScreen (&point);
	}
	
	m_pPopupMenu = new CBCGPDropDownFrame;
	m_pPopupMenu->m_pParentBtn = this;
	
	return m_pPopupMenu->Create (pWnd, point.x, point.y, m_pToolBar);
}
//*********************************************************************************
SIZE CBCGPDropDownToolbarButton::OnCalculateSize (CDC* pDC, const CSize& sizeDefault, BOOL bHorz)
{
	if (m_nID == 0 && m_pToolBar != NULL)
	{
		ASSERT_VALID (m_pToolBar);

		CBCGPToolbarButton* pButton = m_pToolBar->GetButton (0);
		if (pButton == NULL)	// Toolbar is empty!
		{
			ASSERT (FALSE);
		}
		else
		{
			SetDefaultCommand (pButton->m_nID);
		}
	}

	BOOL bImage = m_bImage;

	m_iImage = m_iSelectedImage;
	m_bImage = TRUE;

	CSize sizeBtn = CBCGPToolbarButton::OnCalculateSize (pDC, sizeDefault, bHorz);

	m_iImage = -1;
	m_bImage = bImage;

	int nCurrArrowSize = nArrowSize;
	if (globalData.GetRibbonImageScale () != 1.)
	{
		nCurrArrowSize = (int) (globalData.GetRibbonImageScale () * nCurrArrowSize);
	}

	int nArrowWidth = CBCGPToolBar::IsLargeIcons () ? nCurrArrowSize + 2 : nCurrArrowSize / 2 + 1;
	sizeBtn.cx += nArrowWidth;

	return sizeBtn;
}
//*************************************************************************************
BOOL CBCGPDropDownToolbarButton::ExportToMenuButton (CBCGPToolbarMenuButton& menuButton) const
{
	if (m_pToolBar == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (!CBCGPToolbarButton::ExportToMenuButton (menuButton))
	{
		return FALSE;
	}

	//------------------------------------
	// Create a popup menu with all items:
	//------------------------------------
	CMenu menu;
	menu.CreatePopupMenu ();

	for (POSITION pos = m_pToolBar->m_Buttons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) m_pToolBar->m_Buttons.GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->m_nStyle & TBBS_SEPARATOR)
		{
			menu.AppendMenu (MF_SEPARATOR);
		}
		else if (pButton->m_nID != 0 && pButton->m_nID != (UINT) -1)// Ignore sub-menus
		{
			CString strItem = pButton->m_strText;
			if (strItem.IsEmpty ())
			{
				CString strMessage;
				int iOffset;

				if (strMessage.LoadString (pButton->m_nID) &&
					(iOffset = strMessage.Find (_T('\n'))) != -1)
				{
					strItem = strMessage.Mid (iOffset + 1);
				}
			}

			menu.AppendMenu (MF_STRING, pButton->m_nID, strItem);
		}
	}

	menuButton.m_nID = 0;
	menuButton.m_strText = m_strName;
	menuButton.SetImage (-1);
	menuButton.m_bImage = FALSE;
	menuButton.CreateFromMenu (menu);

	menu.DestroyMenu ();
	return TRUE;
}
//*************************************************************************************
int CBCGPDropDownToolbarButton::OnDrawOnCustomizeList (CDC* pDC, const CRect& rect, 
										BOOL bSelected)
{
	CString strText = m_strText;
	m_strText = m_strName;

	int iResult = CBCGPToolbarButton::OnDrawOnCustomizeList (
					pDC, rect, bSelected);

	m_strText = strText;
	return iResult;
}
//*********************************************************************************
BOOL CBCGPDropDownToolbarButton::OnCustomizeMenu (CMenu* pPopup)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pPopup);

	pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE, MF_GRAYED | MF_BYCOMMAND);
	pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_TEXT, MF_GRAYED | MF_BYCOMMAND);
	pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_IMAGE_AND_TEXT, MF_GRAYED | MF_BYCOMMAND);
	pPopup->EnableMenuItem (ID_BCGBARRES_TOOLBAR_APPEARANCE, MF_GRAYED | MF_BYCOMMAND);
	pPopup->EnableMenuItem (ID_BCGBARRES_COPY_IMAGE, MF_GRAYED | MF_BYCOMMAND);

	return TRUE;
}

