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

// BCGPMDIChildWnd.cpp : implementation file
//

#include "stdafx.h"
#include <afxhtml.h>
#include <afxrich.h>
#include "BCGCBPro.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPMDIChildWnd.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPPrintPreviewView.h"
#include "BCGPMainClientAreaWnd.h"
#include "BCGPGlobalUtils.h"
#include "BCGPDockingControlBar.h"
#include "BCGPDrawManager.h"
#include "BCGPToolbarImages.h"
#include "BCGPWorkspace.h"
#include "BCGPRibbonBar.h"

extern CBCGPWorkspace*	g_pWorkspace;

#if _MSC_VER >= 1300
	#include <..\atlmfc\src\mfc\oleimpl2.h>
#else
	#include <..\src\oleimpl2.h>
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef WS_EX_NOACTIVATE
#define WS_EX_NOACTIVATE        0x08000000L
#endif

BOOL CBCGPMDIChildWnd::m_bEnableFloatingBars	= FALSE;
DWORD CBCGPMDIChildWnd::m_dwExcludeStyle		= WS_CAPTION | WS_BORDER | WS_THICKFRAME;

UINT BCGM_ON_PREPARE_TASKBAR_PREVIEW = ::RegisterWindowMessage (_T("BCGM_ON_PREPARE_TASKBAR_PREVIEW"));
UINT BCGM_ON_AFTER_TASKBAR_ACTIVATE = ::RegisterWindowMessage (_T("BCGM_ON_AFTER_TASKBAR_ACTIVATE"));

/////////////////////////////////////////////////////////////////////////////
// CBCGPMDIChildWnd

IMPLEMENT_DYNCREATE(CBCGPMDIChildWnd, CMDIChildWnd)

#pragma warning (disable : 4355)

CBCGPMDIChildWnd::CBCGPMDIChildWnd() :
	m_Impl (this)
{
	m_pMDIFrame = NULL;
	m_bToBeDestroyed = FALSE;
	m_bWasMaximized = FALSE;
	m_bIsMinimized = FALSE;
	m_rectOriginal.SetRectEmpty ();
	m_bActivating = FALSE;

	// ---- MDITabGroup+
	m_pRelatedTabGroup = NULL;
	// ---- MDITabGroup-

	m_pTabbedControlBar = NULL;

	m_bTabRegistered = FALSE;
	m_bEnableTaskbarThumbnailClip = TRUE;
}

#pragma warning (default : 4355)

CBCGPMDIChildWnd::~CBCGPMDIChildWnd()
{
}


BEGIN_MESSAGE_MAP(CBCGPMDIChildWnd, CMDIChildWnd)
	//{{AFX_MSG_MAP(CBCGPMDIChildWnd)
	ON_WM_CREATE()
	ON_WM_MDIACTIVATE()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_DESTROY()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_GETMINMAXINFO()
	ON_WM_NCPAINT()
	ON_WM_NCACTIVATE()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCHITTEST()
	ON_WM_NCCALCSIZE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_SHOWWINDOW()
	ON_WM_NCRBUTTONUP()
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETTEXT,OnSetText)
	ON_MESSAGE(WM_SETICON,OnSetIcon)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_WM_STYLECHANGED()
	ON_REGISTERED_MESSAGE(BCGM_CHANGEVISUALMANAGER, OnChangeVisualManager)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPMDIChildWnd message handlers

BOOL CBCGPMDIChildWnd::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST &&
		m_pMDIFrame != NULL &&
		m_pMDIFrame->GetActivePopup () != NULL)
	{
		// Don't process accelerators if popup window is active
		return FALSE;
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE && 
		m_pRelatedTabGroup != NULL && GetCapture () == m_pRelatedTabGroup)
	{
		m_pRelatedTabGroup->PostMessage (WM_CANCELMODE);
		return CMDIChildWnd::PreTranslateMessage(pMsg);
	}

	return CMDIChildWnd::PreTranslateMessage(pMsg);
}
//*******************************************************************************
int CBCGPMDIChildWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	m_pMDIFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, GetMDIFrame ());
	ASSERT_VALID (m_pMDIFrame);

	m_Impl.m_bHasBorder = (GetStyle () & WS_BORDER) != 0;
	m_Impl.m_bHasCaption = (GetStyle() & WS_CAPTION) != 0;

	if ((GetStyle () & WS_SYSMENU) == 0)
	{
		GetParent ()->SetRedraw (FALSE);	
		
		m_rectOriginal = CRect (CPoint (lpCreateStruct->x, lpCreateStruct->y),
			CSize (lpCreateStruct->cx, lpCreateStruct->cy));

		if (m_pMDIFrame != NULL && !m_pMDIFrame->IsMDITabbedGroup ())
		{
			CRect rect;
			m_pMDIFrame->m_wndClientArea.GetClientRect (rect);

			CRect rectClient;
			GetClientRect (rectClient);
			ClientToScreen (rectClient);

			CRect rectScreen;
			GetWindowRect (rectScreen);

			rect.left -= rectClient.left - rectScreen.left;
			rect.top -= rectClient.top - rectScreen.top;
			rect.right += rectScreen.right - rectClient.right;
			rect.bottom += rectScreen.bottom - rectClient.bottom;

			SetWindowPos (NULL, rect.left, rect.top, rect.Width (), rect.Height (), 
							SWP_NOACTIVATE | SWP_NOZORDER);
		}

		GetParent ()->SetRedraw (TRUE);
	}

	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	if (m_pMDIFrame->IsPrintPreview ())
	{
		m_pMDIFrame->SendMessage (WM_CLOSE);
	}

	CBCGPFrameImpl::AddFrame (this);
	RegisterTaskbarTab();
	PostMessage (BCGM_CHANGEVISUALMANAGER);

	if (IsRegisteredWithTaskbarTabs())
	{
		InvalidateIconicBitmaps();
	}

	return 0;
}
//*************************************************************************************
void CBCGPMDIChildWnd::RegisterTaskbarTab(CBCGPMDIChildWnd* pWndBefore)
{
	ASSERT_VALID(this);

	if (!IsTaskbarTabsSupportEnabled() || m_tabProxyWnd.GetSafeHwnd() != NULL)
	{
		return;
	}

	m_tabProxyWnd.SetRelatedMDIChildFrame(this);
	CRect rect(CPoint(-32000, -32000), CSize(10, 10));

	CString strClassName = globalData.RegisterWindowClass(_T("BCGP_TASKBAR_TAB"));

	CString strWindowText;
	GetWindowText(strWindowText);

	if (!m_tabProxyWnd.CreateEx(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, strClassName, strWindowText, 
		WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, rect, NULL, 0, NULL))
	{
		TRACE1("Creation of tab proxy window failed, error code: %d", GetLastError());
		return;
	}

	CBCGPMDIFrameWnd* pTopLevel = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, GetTopLevelFrame());
	ASSERT_VALID(pTopLevel);

	if (!globalData.TaskBar_RegisterTab(m_tabProxyWnd.GetSafeHwnd(), pTopLevel->GetSafeHwnd()))
	{
		return;
	}

	if (pWndBefore == NULL && m_pMDIFrame != NULL)
	{
		// attempt to find right place automatically
		pWndBefore = m_pMDIFrame->m_wndClientArea.FindNextRegisteredWithTaskbarMDIChild(this);
	}

	CBCGPMDITabProxyWnd* pProxyWnd = pWndBefore != NULL ? pWndBefore->GetTabProxyWnd() : NULL;

	if (!globalData.TaskBar_SetTabOrder(m_tabProxyWnd.GetSafeHwnd(), pProxyWnd->GetSafeHwnd()))
	{
		return;
	}

	// Set the appropriate DWM properties on the MDI child window
	BOOL bHasIconicBitmap = TRUE;

	globalData.DwmSetWindowAttribute(m_tabProxyWnd.GetSafeHwnd(), 10 /*DWMWA_HAS_ICONIC_BITMAP*/, &bHasIconicBitmap, sizeof(BOOL));
	globalData.DwmSetWindowAttribute(m_tabProxyWnd.GetSafeHwnd(), 7 /*DWMWA_FORCE_ICONIC_REPRESENTATION*/, &bHasIconicBitmap, sizeof(BOOL));

	SetTaskbarTabProperties(0x2/*STPF_USEAPPTHUMBNAILWHENACTIVE*/ | 0x8/*STPF_USEAPPPEEKWHENACTIVE*/, FALSE);

	m_bTabRegistered = TRUE;
}
//*************************************************************************************
BOOL CBCGPMDIChildWnd::IsRegisteredWithTaskbarTabs()
{
	return m_tabProxyWnd.GetSafeHwnd() != NULL;
}
//*************************************************************************************
BOOL CBCGPMDIChildWnd::IsTaskbarTabsSupportEnabled()
{
	const BOOL bIsEnabledInWorkspace = g_pWorkspace == NULL || g_pWorkspace->IsTaskBarInteractionEnabled();

	return CanShowOnTaskBarTabs() && globalData.bIsWindows7 && (GetStyle () & WS_SYSMENU) == 0 && bIsEnabledInWorkspace;
}
//*************************************************************************************
BOOL CBCGPMDIChildWnd::InvalidateIconicBitmaps()
{
	ASSERT_VALID(this);

	if (!IsTaskbarTabsSupportEnabled() || !IsRegisteredWithTaskbarTabs()) 
	{
		return FALSE;
	}

	CRect rectThumbnailClip(0, 0, 0, 0);
	if (m_bEnableTaskbarThumbnailClip)
	{
		rectThumbnailClip = GetTaskbarThumbnailClipRect();
	}

	SetTaskbarThumbnailClipRect(rectThumbnailClip);

	return globalData.DwmInvalidateIconicBitmaps(m_tabProxyWnd.GetSafeHwnd());
}
//*************************************************************************************
void CBCGPMDIChildWnd::UpdateTaskbarTabIcon(HICON hIcon)
{
	if (m_tabProxyWnd.GetSafeHwnd() != NULL)
	{
		m_tabProxyWnd.SetIcon(hIcon, FALSE);
	}
}
//*************************************************************************************
void CBCGPMDIChildWnd::SetTaskbarTabOrder(CBCGPMDIChildWnd* pWndBefore)
{
	ASSERT_VALID(this);

	if (IsTaskbarTabsSupportEnabled() && IsRegisteredWithTaskbarTabs() && m_tabProxyWnd.GetSafeHwnd() != NULL)
	{
		HWND hWndBefore = pWndBefore != NULL ? pWndBefore->GetTabProxyWnd()->GetSafeHwnd() : NULL;
		globalData.TaskBar_SetTabOrder(m_tabProxyWnd.GetSafeHwnd(), hWndBefore);
	}
}
//*************************************************************************************
void CBCGPMDIChildWnd::SetTaskbarTabProperties(UINT uiFlags, BOOL bInvalidateIconicBitmaps)
{
	ASSERT_VALID(this);

	if (IsTaskbarTabsSupportEnabled() && IsRegisteredWithTaskbarTabs() && m_tabProxyWnd.GetSafeHwnd() != NULL)
	{
		globalData.TaskBar_SetTabProperties(m_tabProxyWnd.GetSafeHwnd(), uiFlags);

		if (bInvalidateIconicBitmaps)
		{
			InvalidateIconicBitmaps();
		}
	}
}
//*************************************************************************************
BOOL CBCGPMDIChildWnd::DockControlBarLeftOf(CBCGPControlBar* pBar, CBCGPControlBar* pLeftOf)
{
	m_dockManager.DockControlBarLeftOf (pBar, pLeftOf);
	return TRUE;
}
//*************************************************************************************
void CBCGPMDIChildWnd::SetTaskbarTabActive()
{
	ASSERT_VALID(this);

	if (!IsTaskbarTabsSupportEnabled())
	{
		return;
	}

	CBCGPMDIFrameWnd* pParentFrame = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, GetTopLevelFrame());
	ASSERT_VALID(pParentFrame);

	globalData.TaskBar_SetTabActive(m_tabProxyWnd.GetSafeHwnd(), pParentFrame->GetSafeHwnd(), 0);
}
//*************************************************************************************
void CBCGPMDIChildWnd::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd) 
{
	static BOOL bActivating = FALSE;

	m_dockManager.OnActivateFrame (bActivate);

	m_bActivating = bActivate;

	if (!bActivating)
	{
		bActivating = TRUE;

		CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

		if (bActivate && m_pMDIFrame != NULL)
		{
			ASSERT_VALID (m_pMDIFrame);
			m_pMDIFrame->m_wndClientArea.SetActiveTab (pActivateWnd->GetSafeHwnd ());
		}

		// If in MDI Tabbed or MDI Tabbed Group, mode, and if the application wants
		// the behavior, set the MDI child as the active tab in the task bar tab list.
		if (IsTaskbarTabsSupportEnabled() && IsRegisteredWithTaskbarTabs())
		{
			InvalidateIconicBitmaps();
			CBCGPMDIChildWnd* pChild = DYNAMIC_DOWNCAST(CBCGPMDIChildWnd, pDeactivateWnd);
			if (pChild != NULL)
			{
				pChild->InvalidateIconicBitmaps();
			}

			SetTaskbarTabActive();
		}

		bActivating = FALSE;

		if (bActivate && m_pMDIFrame != NULL)
		{
			CBCGPDockManager* parentDockManager = m_pMDIFrame->GetDockManager ();
			ASSERT_VALID (parentDockManager);

			if (parentDockManager != NULL && 
				parentDockManager->IsOLEContainerMode () ||
				m_dockManager.IsOLEContainerMode ())
			{
				globalUtils.ForceAdjustLayout (parentDockManager, TRUE, FALSE, TRUE);
				m_pMDIFrame->m_Impl.OnChangeVisualManager ();
			}
		}
	}

	if (bActivate && !IsTaskbarTabsSupportEnabled() || !IsRegisteredWithTaskbarTabs())
	{
		SetTaskbarThumbnailClipRect(CRect(0, 0, 0, 0));
	}

#if _MSC_VER >= 1300
	if (!bActivate && IsPrintPreview())
	{
		PostMessage(WM_COMMAND, AFX_ID_PREVIEW_CLOSE);
	}
#endif
}
//*************************************************************************************
void CBCGPMDIChildWnd::ActivateFrame(int nCmdShow) 
{
	CWnd* pWndParent = GetParent ();
	ASSERT_VALID (pWndParent);

	if (CBCGPMDIFrameWnd::m_bDisableSetRedraw ||
		pWndParent->GetSafeHwnd () == NULL)
	{
		if ((GetStyle () & WS_SYSMENU) == 0)
		{
			nCmdShow = SW_SHOWMAXIMIZED;
		}

		if (m_pMDIFrame != 0 && m_pMDIFrame->IsMDITabbedGroup ())
		{
			nCmdShow = SW_SHOWNORMAL;
		}


		CMDIChildWnd::ActivateFrame(nCmdShow);	
		return;
	}
	
	pWndParent->SetRedraw (FALSE);

	CMDIChildWnd::ActivateFrame(nCmdShow);

	pWndParent->SetRedraw (TRUE);
	pWndParent->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	
}
//*************************************************************************************
LRESULT CBCGPMDIChildWnd::OnSetText(WPARAM, LPARAM lParam)
{
	LRESULT lRes = Default();

	if (m_pMDIFrame != NULL)
	{
		ASSERT_VALID (m_pMDIFrame);
		m_pMDIFrame->m_wndClientArea.UpdateTabs (TRUE);
	}

	m_Impl.OnSetText ((LPCTSTR)lParam);

	if (IsTaskbarTabsSupportEnabled() && IsRegisteredWithTaskbarTabs())
	{
		CBCGPMDIFrameWnd* pWnd = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, GetTopLevelFrame());
		if (pWnd == NULL)
		{
			return lRes;
		}

		DWORD dwStyle = pWnd->GetStyle();

		if ((dwStyle & FWS_ADDTOTITLE) == FWS_ADDTOTITLE)
		{
			CString strFrameTitle = pWnd->GetTitle();
			CString strWndName;
			CString strChildTitle((LPCTSTR)lParam);
			if ((dwStyle & FWS_PREFIXTITLE) == FWS_PREFIXTITLE)
			{
				strWndName = strChildTitle + _T(" - ") + strFrameTitle;
			}
			else
			{
				strWndName = strFrameTitle + _T(" - ") + strChildTitle;
			}
			m_tabProxyWnd.SetWindowText((LPCTSTR) strWndName);
		}
		else
		{
			m_tabProxyWnd.SetWindowText((LPCTSTR) lParam);
		}
	}

	return lRes;
}
//*************************************************************************************
LRESULT CBCGPMDIChildWnd::OnSetIcon(WPARAM,LPARAM)
{
	LRESULT lRes = Default();

	if (m_pMDIFrame != NULL)
	{
		ASSERT_VALID (m_pMDIFrame);
		m_pMDIFrame->m_wndClientArea.UpdateTabs ();
	}

	return lRes;
}
//*************************************************************************************
CString CBCGPMDIChildWnd::GetFrameText () const
{
	ASSERT_VALID (this);

	CString strText;
	GetWindowText (strText);

	return strText;
}
//*************************************************************************************
HICON CBCGPMDIChildWnd::GetFrameIcon () const
{
	ASSERT_VALID (this);

	HICON hIcon = GetIcon (FALSE);
	if (hIcon == NULL)
	{
		hIcon = (HICON)(LONG_PTR) GetClassLongPtr (GetSafeHwnd (), GCLP_HICONSM);
	}

	return hIcon;
}
//*************************************************************************************
void CBCGPMDIChildWnd::OnUpdateFrameTitle (BOOL bAddToTitle)
{
	BOOL bRedraw = m_Impl.IsOwnerDrawCaption () && 
		IsWindowVisible () && (GetStyle () & WS_MAXIMIZE) == 0;

	CString strTitle1;

	if (bRedraw)
	{
		GetWindowText (strTitle1);
	}	

	CMDIChildWnd::OnUpdateFrameTitle (bAddToTitle);

	if (bRedraw)
	{
		CString strTitle2;
		GetWindowText (strTitle2);

		if (strTitle1 != strTitle2)
		{
			SendMessage (WM_NCPAINT, 0, 0);
		}
	}

	if (m_pMDIFrame != NULL)
	{
		ASSERT_VALID (m_pMDIFrame);
		m_pMDIFrame->m_wndClientArea.UpdateTabs ();
	}
}
//*****************************************************************************
void CBCGPMDIChildWnd::OnSize(UINT nType, int cx, int cy) 
{
	if (m_bToBeDestroyed)
	{
		// prevents main menu flickering when the last dockument is being closed
		return;
	}
	
	InvalidateIconicBitmaps();

	m_bIsMinimized = (nType == SIZE_MINIMIZED);

	if (CBCGPVisualManager::GetInstance ()->IsOwnerDrawCaption ())
	{
		if (m_pMDIFrame != NULL && !m_pMDIFrame->IsMDITabbedGroup ())
		{
			CRect rectWindow;
			GetWindowRect (rectWindow);

			WINDOWPOS wndpos;
			wndpos.flags = SWP_FRAMECHANGED;
			wndpos.x     = rectWindow.left;
			wndpos.y     = rectWindow.top;
			wndpos.cx    = rectWindow.Width ();
			wndpos.cy    = rectWindow.Height ();

			m_Impl.OnWindowPosChanging (&wndpos);
		}
		else if (m_Impl.m_bIsWindowRgn)
		{
			m_Impl.m_bIsWindowRgn = FALSE;
			SetWindowRgn (NULL, TRUE);
		}
	}

	if (!m_bIsMinimized && nType != SIZE_MAXIMIZED && !m_bWasMaximized)
	{
		m_dockManager.m_bSizeFrame = TRUE;
		CMDIChildWnd::OnSize(nType, cx, cy);
		AdjustDockingLayout ();			
		m_dockManager.m_bSizeFrame = FALSE;

		m_Impl.UpdateCaption ();
		return;
	}

	CMDIChildWnd::OnSize(nType, cx, cy);

	if ((nType == SIZE_MAXIMIZED || (nType == SIZE_RESTORED && m_bWasMaximized)))
	{
		RecalcLayout ();

		if (m_pNotifyHook != NULL && nType == SIZE_RESTORED)
		{
			CBCGPMDIFrameWnd* pTopLevelFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, GetParentFrame ());
			if (pTopLevelFrame == NULL || !pTopLevelFrame->AreMDITabs ())
			{
				ModifyStyle(0, WS_OVERLAPPEDWINDOW);
			}
		}
	}

	m_bWasMaximized = (nType == SIZE_MAXIMIZED);
	m_Impl.UpdateCaption ();
}
//*****************************************************************************
BOOL CBCGPMDIChildWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	m_dockManager.Create (this);
	return CMDIChildWnd::PreCreateWindow(cs);
}

//*****************************************************************************
//******************* dockmanager layer ***************************************
//*****************************************************************************
void CBCGPMDIChildWnd::AddDockBar ()
{
	ASSERT_VALID (this);
}
//*****************************************************************************
BOOL CBCGPMDIChildWnd::AddControlBar (CBCGPBaseControlBar* pControlBar, BOOL bTail)
{
	ASSERT_VALID (this);
	return m_dockManager.AddControlBar (pControlBar, bTail);
}
//*****************************************************************************
BOOL CBCGPMDIChildWnd::InsertControlBar (CBCGPBaseControlBar* pControlBar, 
									  CBCGPBaseControlBar* pTarget, BOOL bAfter)
{
	ASSERT_VALID (this);
	return m_dockManager.InsertControlBar (pControlBar, pTarget, bAfter);
}
//*****************************************************************************
void CBCGPMDIChildWnd::RemoveControlBarFromDockManager (CBCGPBaseControlBar* pControlBar, BOOL bDestroy,
										 BOOL bAdjustLayout, BOOL bAutoHide, CBCGPBaseControlBar* pBarReplacement)
{
	ASSERT_VALID (this);
	m_dockManager.RemoveControlBarFromDockManager (pControlBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
}
//*****************************************************************************
void CBCGPMDIChildWnd::DockControlBar (CBCGPBaseControlBar* pBar, UINT nDockBarID, 
									LPCRECT /*lpRect*/)
{
	ASSERT_VALID (this);

	if (pBar->CanFloat () && !CBCGPMDIChildWnd::m_bEnableFloatingBars)
	{
		// bar can't be floating
		pBar->m_dwBCGStyle &= ~CBRS_BCGP_FLOAT;
	}


	if (pBar->CanBeResized () || pBar->CanFloat ())
	{
		pBar->EnableDocking (CBRS_ALIGN_ANY);
		m_dockManager.DockControlBar (pBar, nDockBarID);
	}
	else
	{
		AddControlBar (pBar, TRUE);
	}
}
//*****************************************************************************
void CBCGPMDIChildWnd::OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState)
{
	ASSERT_VALID (this);

	CBCGPMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, AfxGetMainWnd ());
	if (pMainFrame != NULL)
	{
		pMainFrame->SetPrintPreviewFrame (bPreview ? this : NULL);
	}

	m_dockManager.SetPrintPreviewMode (bPreview, pState);
	DWORD dwSavedState = pState->dwStates;
	CMDIChildWnd::OnSetPreviewMode (bPreview, pState);
	pState->dwStates = dwSavedState;

	AdjustDockingLayout ();
	RecalcLayout ();
}
//*****************************************************************************
CBCGPBaseControlBar* CBCGPMDIChildWnd::GetControlBar (UINT nID)
{
	ASSERT_VALID (this);
	
	CBCGPBaseControlBar* pBar = m_dockManager.FindBarByID (nID, TRUE);
	return pBar;
}
//*****************************************************************************
void CBCGPMDIChildWnd::ShowControlBar (CBCGPBaseControlBar* pBar, BOOL bShow, 
									   BOOL bDelay, BOOL bActivate)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	pBar->ShowControlBar (bShow, bDelay, bActivate);
}
//*****************************************************************************
CBCGPBaseControlBar* CBCGPMDIChildWnd::ControlBarFromPoint (CPoint point, 
							int nSensitivity, bool bExactBar, 
							CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID (this);
	return m_dockManager.ControlBarFromPoint (point, nSensitivity, bExactBar, 
												pRTCBarType);
}
//*****************************************************************************
CBCGPBaseControlBar* CBCGPMDIChildWnd::ControlBarFromPoint (CPoint point, 
								int nSensitivity, DWORD& dwAlignment, 
								CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID (this);
	return m_dockManager.ControlBarFromPoint (point, nSensitivity, dwAlignment, 
												pRTCBarType);
}
//*****************************************************************************
BOOL CBCGPMDIChildWnd::IsPointNearDockBar (CPoint point, DWORD& dwBarAlignment, 
										   BOOL& bOuterEdge) const
{
	ASSERT_VALID (this);
	return m_dockManager.IsPointNearDockBar (point, dwBarAlignment, bOuterEdge);
}
//*****************************************************************************
void CBCGPMDIChildWnd::AdjustDockingLayout (HDWP hdwp)
{
	ASSERT_VALID (this);
	
	if (m_dockManager.IsInAdjustLayout ())
	{	
		return;
	}

	m_dockManager.AdjustDockingLayout (hdwp);
	AdjustClientArea ();
}
//*****************************************************************************
void CBCGPMDIChildWnd::AdjustClientArea ()
{
	CWnd* pChildWnd = (m_pTabbedControlBar != NULL && m_pTabbedControlBar->IsMDITabbed () && 
						m_pTabbedControlBar->GetParent () == this) ? m_pTabbedControlBar : GetDlgItem (AFX_IDW_PANE_FIRST);
	if (pChildWnd != NULL)
	{
		if (!pChildWnd->IsKindOf (RUNTIME_CLASS (CSplitterWnd)) &&
			!pChildWnd->IsKindOf (RUNTIME_CLASS (CFormView)))
		{
			pChildWnd->ModifyStyle (0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
		}
		else
		{
			pChildWnd->ModifyStyle (0, WS_CLIPSIBLINGS);
		}

		if (!CBCGPDockManager::m_bFullScreenMode)
		{
			CRect rectClientAreaBounds = m_dockManager.GetClientAreaBounds ();

			rectClientAreaBounds.left += m_rectBorder.left;
			rectClientAreaBounds.top  += m_rectBorder.top;
			rectClientAreaBounds.right -= m_rectBorder.right;
			rectClientAreaBounds.bottom -= m_rectBorder.bottom;
			
			pChildWnd->SetWindowPos (&wndBottom, rectClientAreaBounds.left, 
											rectClientAreaBounds.top, 
											rectClientAreaBounds.Width (), 
											rectClientAreaBounds.Height (),
											SWP_NOACTIVATE);
		}
	}
}
//*****************************************************************************
BOOL CBCGPMDIChildWnd::OnMoveMiniFrame	(CWnd* pFrame)
{
	ASSERT_VALID (this);
	return m_dockManager.OnMoveMiniFrame (pFrame);
}
//****************************************************************************************
BOOL CBCGPMDIChildWnd::EnableDocking (DWORD dwDockStyle)
{
	return m_dockManager.EnableDocking (dwDockStyle);
}
//****************************************************************************************
BOOL CBCGPMDIChildWnd::EnableAutoHideBars (DWORD dwDockStyle, BOOL bActivateOnMouseClick)
{
	return m_dockManager.EnableAutoHideBars (dwDockStyle, bActivateOnMouseClick);
}
//****************************************************************************************
void CBCGPMDIChildWnd::EnableMaximizeFloatingBars(BOOL bEnable, BOOL bMaximizeByDblClick)
{
	m_dockManager.EnableMaximizeFloatingBars(bEnable, bMaximizeByDblClick);
}
//****************************************************************************************
BOOL CBCGPMDIChildWnd::AreFloatingBarsCanBeMaximized() const
{
	return m_dockManager.AreFloatingBarsCanBeMaximized();
}
//*************************************************************************************
void CBCGPMDIChildWnd::RecalcLayout (BOOL bNotify)
{
	if (m_bInRecalcLayout)
		return;

	m_bInRecalcLayout = TRUE;

	if (!m_bIsMinimized)
	{
		CView* pView = GetActiveView ();

		if (m_dockManager.IsPrintPreviewValid () ||
			m_dockManager.IsOLEContainerMode ())
		{
			if (pView != NULL && pView->IsKindOf (RUNTIME_CLASS (CBCGPPrintPreviewView)))
			{
	
				m_dockManager.RecalcLayout (bNotify);
				CRect rectClient = m_dockManager.GetClientAreaBounds ();
				pView->SetWindowPos (NULL, rectClient.left, rectClient.top, 
										rectClient.Width (), rectClient.Height (),
										SWP_NOZORDER  | SWP_NOACTIVATE);
			}
			else
			{
				COleClientItem*	pActiveItem = NULL;
				CView* pView = GetActiveView ();
				
				if (pView != NULL && GetParentFrame ()->GetActiveFrame () == this && 
					m_bActivating)
				{
					ASSERT_VALID (pView);

					COleDocument* pDoc = DYNAMIC_DOWNCAST (COleDocument, pView->GetDocument ());
					if (pDoc != NULL)
					{
						ASSERT_VALID (pDoc);
						pActiveItem = pDoc->GetInPlaceActiveItem (pView);

						if (bNotify && pActiveItem != NULL && pActiveItem->m_pInPlaceFrame != NULL)
						{
							pActiveItem->m_pInPlaceFrame->OnRecalcLayout ();
						}
					}

					CRect rectClient;
					CFrameWnd* pFrame = pView->GetParentFrame ();

					if (pFrame->GetSafeHwnd() != NULL)
					{
						pFrame->GetClientRect (rectClient);

						CWnd* pChildWnd = m_dockManager.IsPrintPreviewValid () ? GetDlgItem (AFX_IDW_PANE_SAVE) : 
																				GetDlgItem (AFX_IDW_PANE_FIRST);

						if (pChildWnd->GetSafeHwnd() != NULL && pChildWnd->IsKindOf (RUNTIME_CLASS (CSplitterWnd)))
						{
							pChildWnd->SetWindowPos (NULL, 0, 0, rectClient.Width (), rectClient.Height (),
													SWP_NOZORDER  | SWP_NOACTIVATE);	
						}
						else
						{
							pView->SetWindowPos (NULL, 0, 0, rectClient.Width (), rectClient.Height (),
													SWP_NOZORDER  | SWP_NOACTIVATE);
						}
					}
				}
				else
				{
					AdjustClientArea ();	
				}
			}
		}
		else
		{
			m_dockManager.RecalcLayout (bNotify);
			AdjustClientArea ();
		}
	}

	m_bInRecalcLayout = FALSE;
}
//*************************************************************************************
void CBCGPMDIChildWnd::OnSizing(UINT fwSide, LPRECT pRect) 
{
	CMDIChildWnd::OnSizing(fwSide, pRect);

	CRect rect;
	GetWindowRect (rect);

	if (rect.Size () != CRect (pRect).Size ())
	{
		AdjustDockingLayout ();	
	}
	
}
//*************************************************************************************
void CBCGPMDIChildWnd::UnregisterTaskbarTab(BOOL bCheckRegisteredMDIChildCount)
{
	if (m_tabProxyWnd.GetSafeHwnd() == NULL)
	{
		return;
	}

	globalData.TaskBar_UnregisterTab(m_tabProxyWnd.GetSafeHwnd());
	m_tabProxyWnd.DestroyWindow();

	if (bCheckRegisteredMDIChildCount)
	{
		// if no registered children - reset clip rect to full app window
		CBCGPMDIFrameWnd* pTopLevel = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, GetTopLevelFrame());
		if (pTopLevel != NULL && pTopLevel->GetRegisteredWithTaskBarMDIChildCount() == 0)
		{
			globalData.TaskBar_SetThumbnailClip(pTopLevel->GetSafeHwnd(), NULL);
		}
	}
}
//*************************************************************************************
void CBCGPMDIChildWnd::OnDestroy() 
{
	UnregisterTaskbarTab();

	if (m_pMDIFrame != NULL && m_pMDIFrame->IsPrintPreview ())
	{
		m_pMDIFrame->SendMessage (WM_CLOSE);
	}

	if (m_pTabbedControlBar != NULL && CWnd::FromHandlePermanent (m_pTabbedControlBar->GetSafeHwnd ()) != NULL)
	{
		CWnd* pParent = m_pTabbedControlBar->GetParent ();
		
		if (pParent == this && m_pMDIFrame != NULL && !m_pMDIFrame->m_bClosing)
		{
			// tabbed MDI is being closed. We need to reassign parent of embedded control bar
			m_pTabbedControlBar->ShowWindow (SW_HIDE);
			m_pTabbedControlBar->SetParent (m_pTabbedControlBar->GetDockSite ());
			m_pMDIFrame->GetDockManager ()->AddHiddenMDITabbedBar (m_pTabbedControlBar);
		}
		m_pTabbedControlBar = NULL;
	}

	CBCGPFrameImpl::RemoveFrame (this);

	POSITION pos = NULL;

	for (pos = m_dockManager.m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
	{
		CBCGPMiniFrameWnd* pNextFrame = DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd,
					m_dockManager.m_lstMiniFrames.GetNext (pos));
		if (pNextFrame != NULL)
		{
			pNextFrame->DestroyWindow ();
		}
	}

	CList<HWND, HWND> lstChildren;
	CWnd* pNextWnd = GetTopWindow ();
	while (pNextWnd != NULL)
	{
		lstChildren.AddTail (pNextWnd->m_hWnd);
		pNextWnd = pNextWnd->GetNextWindow ();
	}

	for (pos = lstChildren.GetHeadPosition (); pos != NULL;)
	{
		HWND hwndNext = lstChildren.GetNext (pos);
		if (IsWindow (hwndNext) && ::GetParent (hwndNext) == m_hWnd)
		{
			::DestroyWindow (hwndNext);
		}
	}

	// CBCGPMainClientArea::OnMDIDestroy will take care about removing from the tabs.
	m_pRelatedTabGroup = NULL;

	CMDIChildWnd::OnDestroy();
}
//*************************************************************************************
void CBCGPMDIChildWnd::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	if (m_pMDIFrame != NULL && m_pMDIFrame->IsFullScreen() && 
		!m_pMDIFrame->AreMDITabs())
	{
		m_pMDIFrame->m_Impl.GetFullScreenMinMaxInfo(lpMMI);
	}
	else
	{
		CMDIChildWnd::OnGetMinMaxInfo(lpMMI);
	}
}
//*********************************************************************************
void CBCGPMDIChildWnd::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	CMDIChildWnd::OnStyleChanged (nStyleType, lpStyleStruct);

	BOOL bWasSysMenu = (lpStyleStruct->styleOld & WS_SYSMENU);
	BOOL bIsSysMenu = (lpStyleStruct->styleNew & WS_SYSMENU);

	if (IsTaskbarTabsSupportEnabled())
	{
		RegisterTaskbarTab();
	}
	else
	{
		UnregisterTaskbarTab();
	}

	if (bWasSysMenu == bIsSysMenu)
	{
		return;
	}

	BOOL bIsInMDITabbedGroup = m_pMDIFrame != NULL && m_pMDIFrame->IsMDITabbedGroup ();

	if (bWasSysMenu)
	{
		if ((lpStyleStruct->styleOld & WS_MAXIMIZE) == 0 &&
			(lpStyleStruct->styleOld & WS_MINIMIZE) == 0)
		{
			CRect rectWindow;
			GetWindowRect (rectWindow);

			GetParent()->ScreenToClient (&rectWindow);

			m_rectOriginal = rectWindow;
		}

		if (m_pMDIFrame != NULL && (m_pMDIFrame->m_wndClientArea.GetExStyle () & WS_EX_CLIENTEDGE) != 0)
		{
			m_pMDIFrame->m_wndClientArea.ModifyStyleEx (WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED);
		}

		if (!bIsInMDITabbedGroup)
		{
			if (!IsZoomed () && bIsSysMenu)
			{
				if (CBCGPVisualManager::GetInstance ()->IsOwnerDrawCaption ())
				{
					m_Impl.m_bIsWindowRgn = CBCGPVisualManager::GetInstance ()->OnSetWindowRegion (
						this, m_rectOriginal.Size ());
				}
				else
				{
					m_Impl.m_bIsWindowRgn = FALSE;
					SetWindowRgn (NULL, TRUE);
				}
			}

			CRect rect;
			m_pMDIFrame->m_wndClientArea.GetClientRect (rect);

			CRect rectClient;
			GetClientRect (rectClient);
			ClientToScreen (rectClient);

			CRect rectScreen;
			GetWindowRect (rectScreen);

			rect.left -= rectClient.left - rectScreen.left;
			rect.top -= rectClient.top - rectScreen.top;
			rect.right += rectScreen.right - rectClient.right;
			rect.bottom += rectScreen.bottom - rectClient.bottom;

			if (!rect.IsRectNull ())
			{
				SetWindowPos (NULL, rect.left, rect.top, rect.Width (), rect.Height (), 
								SWP_NOACTIVATE | SWP_NOZORDER);
			}
		}
	}
	else if (!bIsInMDITabbedGroup)
	{
		if (m_pMDIFrame != NULL && (m_pMDIFrame->m_wndClientArea.GetExStyle () & WS_EX_CLIENTEDGE) == 0)
		{
			m_pMDIFrame->m_wndClientArea.ModifyStyleEx (0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
		}

		if (!IsZoomed () && bIsSysMenu)
		{
			if (CBCGPVisualManager::GetInstance ()->IsOwnerDrawCaption ())
			{
				CRect rectWindow (m_rectOriginal);

				if (rectWindow.IsRectNull ())
				{
					GetWindowRect (rectWindow);
					GetParent()->ScreenToClient (&rectWindow);
				}

				if (!rectWindow.IsRectNull ())
				{
					m_Impl.m_bIsWindowRgn = CBCGPVisualManager::GetInstance ()->OnSetWindowRegion (
						this, rectWindow.Size ());
				}
			}
			else
			{
				m_Impl.m_bIsWindowRgn = FALSE;
				SetWindowRgn (NULL, TRUE);
			}
		}

		if (!m_rectOriginal.IsRectNull ())
		{
			SetWindowPos (NULL, m_rectOriginal.left, m_rectOriginal.top, 
				m_rectOriginal.Width (), m_rectOriginal.Height (),
				SWP_NOACTIVATE | SWP_NOZORDER);
		}
	}
}
//*********************************************************************************
LRESULT CBCGPMDIChildWnd::OnIdleUpdateCmdUI(WPARAM, LPARAM)
{
	m_dockManager.SendMessageToMiniFrames (WM_IDLEUPDATECMDUI);
	return 0L;
}
//*********************************************************************************
LPCTSTR CBCGPMDIChildWnd::GetDocumentName (CObject** /*pObj*/)
{
	CDocument* pDoc = GetActiveDocument ();
	if (pDoc != NULL)
	{
		return pDoc->GetPathName ();
	}
	return NULL;
}
//**************************************************************************
void CBCGPMDIChildWnd::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	if ((lpwndpos->flags & SWP_FRAMECHANGED) == SWP_FRAMECHANGED)
	{
		if (m_pMDIFrame != NULL && !m_pMDIFrame->IsMDITabbedGroup ())
		{
			m_Impl.OnWindowPosChanging (lpwndpos);
		}
		else if (m_Impl.m_bIsWindowRgn)
		{
			m_Impl.m_bIsWindowRgn = FALSE;
			SetWindowRgn (NULL, TRUE);
		}
	}

	CMDIChildWnd::OnWindowPosChanged(lpwndpos);
}
//**************************************************************************************
void CBCGPMDIChildWnd::OnNcPaint() 
{
	BOOL bIsInMDITabbedGroup = m_pMDIFrame != NULL && m_pMDIFrame->IsMDITabbedGroup ();

	if (bIsInMDITabbedGroup || IsZoomed () ||
		!CBCGPVisualManager::GetInstance ()->IsOwnerDrawCaption ())
	{
		Default ();
		return;
	}
	
	if (!m_Impl.OnNcPaint ())
	{
		Default ();
	}
}
//*****************************************************************************************
BOOL CBCGPMDIChildWnd::OnNcActivate(BOOL bActive) 
{
	BOOL bIsOwnerDraw = m_Impl.OnNcActivate (bActive);

	if (bIsOwnerDraw)
	{
		SetRedraw (FALSE);
	}

	BOOL bRes = CMDIChildWnd::OnNcActivate(bActive);

	if (bIsOwnerDraw)
	{
		SetRedraw (TRUE);
		RedrawWindow (NULL, NULL, 
			RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}

	InvalidateIconicBitmaps();
	return bRes;
}
//**************************************************************************
void CBCGPMDIChildWnd::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	m_Impl.OnNcMouseMove (nHitTest, point);
	CMDIChildWnd::OnNcMouseMove(nHitTest, point);
}
//**************************************************************************
BCGNcHitTestType CBCGPMDIChildWnd::OnNcHitTest(CPoint point) 
{
	UINT nHit = m_Impl.OnNcHitTest (point);
	if (nHit != HTNOWHERE)
	{
		return nHit;
	}

	return CMDIChildWnd::OnNcHitTest(point);
}
//**************************************************************************
LRESULT CBCGPMDIChildWnd::OnChangeVisualManager (WPARAM wp, LPARAM lp)
{
	if (m_pMDIFrame != NULL && !m_pMDIFrame->IsMDITabbedGroup ())
	{
		m_Impl.OnChangeVisualManager ();

		if (m_Impl.IsOwnerDrawCaption () && !CBCGPVisualManager::GetInstance()->IsDWMCaptionSupported())
		{
			CBCGPVisualManager::GetInstance ()->OnNcActivate (this, m_pMDIFrame->MDIGetActive () == this);
		}
	}

	CView* pViewActive = GetActiveView();
	if (pViewActive->GetSafeHwnd() != NULL)
	{
		pViewActive->SendMessage(BCGM_CHANGEVISUALMANAGER, wp, lp);
	}

	return 0;
}
//***************************************************************************
void CBCGPMDIChildWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	BOOL bIsInMDITabbedGroup = m_pMDIFrame != NULL && m_pMDIFrame->IsMDITabbedGroup ();

	if (!bIsInMDITabbedGroup && !IsZoomed () &&
		CBCGPVisualManager::GetInstance ()->IsOwnerDrawCaption () &&
		(GetStyle () & WS_BORDER) == 0)
	{
		lpncsp->rgrc[0].top += ::GetSystemMetrics (SM_CYCAPTION);
	}
	
	CMDIChildWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
}
//***************************************************************************
void CBCGPMDIChildWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_Impl.OnLButtonUp (point);
	CMDIChildWnd::OnLButtonUp(nFlags, point);
}
//***************************************************************************
void CBCGPMDIChildWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	m_Impl.OnMouseMove (point);
	CMDIChildWnd::OnMouseMove(nFlags, point);
}
//***************************************************************************
void CBCGPMDIChildWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_Impl.OnLButtonDown (point);
	CMDIChildWnd::OnLButtonDown(nFlags, point);
}
//***************************************************************************
void CBCGPMDIChildWnd::AddTabbedControlBar (CBCGPDockingControlBar* pControlBar)
{
	ASSERT_VALID (pControlBar);
	m_pTabbedControlBar = pControlBar;
	
	m_pTabbedControlBar->OnBeforeChangeParent (this);
	m_pTabbedControlBar->EnableGripper (FALSE);
	m_pTabbedControlBar->SetParent (this);
	m_pTabbedControlBar->ShowWindow (SW_SHOW);
	m_pTabbedControlBar->SetMDITabbed (TRUE);

	m_pMDIFrame->GetDockManager ()->RemoveHiddenMDITabbedBar (m_pTabbedControlBar);
	
	AdjustClientArea (); 
}
//***************************************************************************
BOOL CBCGPMDIChildWnd::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//***************************************************************************
void CBCGPMDIChildWnd::OnNcRButtonUp(UINT nHitTest, CPoint point)
{
	if (m_pTabbedControlBar != NULL && nHitTest == HTCAPTION && !IsZoomed ())	
	{
		ASSERT_VALID (m_pTabbedControlBar);
		m_pTabbedControlBar->OnShowControlBarMenu (point);
		return;
	}

	CMDIChildWnd::OnNcLButtonUp(nHitTest, point);
}
//***************************************************************************
void CBCGPMDIChildWnd::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CMDIChildWnd::OnShowWindow(bShow, nStatus);
	
	BOOL bIsInMDITabbedGroup = m_pMDIFrame != NULL && m_pMDIFrame->IsMDITabbedGroup ();

	if (bShow && !bIsInMDITabbedGroup && !IsZoomed () &&
		CBCGPVisualManager::GetInstance ()->IsOwnerDrawCaption () &&
		(GetStyle () & WS_BORDER) == 0 &&
		GetActiveDocument () == NULL)
	{
        CRect rectClient;
        GetWindowRect(rectClient);

        // Trigger resize:
        SetWindowPos (NULL, -1, -1, rectClient.Width () + 1, rectClient.Height (),
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
        SetWindowPos (NULL, -1, -1, rectClient.Width (), rectClient.Height (),
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
    }
}
//***************************************************************************
CWnd* CBCGPMDIChildWnd::GetTaskbarPreviewWnd()
{
	ASSERT_VALID(this);
	CWnd* pWnd = GetDescendantWindow(AFX_IDW_PANE_FIRST);

	if (pWnd->GetSafeHwnd() != NULL)
	{
		ASSERT_VALID(pWnd);

		CWnd* pParent = pWnd->GetParent();

		if (pParent != this && pParent->GetSafeHwnd() != NULL && pParent->IsKindOf(RUNTIME_CLASS(CSplitterWnd)))
		{
			pWnd = pParent;
		}
	}
	else
	{
		// Get first child window:
		pWnd = GetWindow(GW_CHILD);
	}

	return pWnd;
}
//***************************************************************************
void CBCGPMDIChildWnd::OnPressTaskbarThmbnailCloseButton()
{
	CBCGPMDIFrameWnd* pTopLevelFrame = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, GetTopLevelFrame());
	ASSERT_VALID(pTopLevelFrame);

	if (pTopLevelFrame == NULL || !pTopLevelFrame->IsWindowEnabled())
	{
		return;
	}

	CDocument* pDoc = GetActiveDocument();
	if (pDoc != NULL && pDoc->IsModified())
	{
		ActivateTopLevelFrame();
	}

	PostMessage(WM_CLOSE);
}
//***************************************************************************
void CBCGPMDIChildWnd::OnTaskbarTabThumbnailActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	UNREFERENCED_PARAMETER(pWndOther);
	UNREFERENCED_PARAMETER(bMinimized);

	if (nState != WA_CLICKACTIVE && nState != WA_INACTIVE)
	{
#ifndef BCGP_EXCLUDE_RIBBON
		CBCGPMDIFrameWnd* pTopLevelFrame = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, GetTopLevelFrame());
		ASSERT_VALID(pTopLevelFrame);

		if (pTopLevelFrame->GetRibbonBar() != NULL && pTopLevelFrame->GetRibbonBar()->IsBackstageViewActive())
		{
			pTopLevelFrame->CloseRibbonBackstageView();
		}
#endif
		ActivateTopLevelFrame();
	}
}
//***************************************************************************
int CBCGPMDIChildWnd::OnTaskbarTabThumbnailMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	UNREFERENCED_PARAMETER(pDesktopWnd);
	UNREFERENCED_PARAMETER(nHitTest);

	if (message == WM_LBUTTONUP)
	{
		ActivateTopLevelFrame();
	}

	return 1;
}
//***************************************************************************
void CBCGPMDIChildWnd::ActivateTopLevelFrame()
{
	CBCGPMDIFrameWnd* pTopLevel = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, GetTopLevelFrame());
	if (pTopLevel == NULL)
	{
		return;
	}

	ActivateFrame(-1);

	pTopLevel->SetForegroundWindow();

	BOOL bIsMinimized = pTopLevel->IsIconic();

	pTopLevel->ShowWindow(bIsMinimized ? SW_RESTORE : SW_SHOW);
	pTopLevel->PostMessage(BCGM_ON_AFTER_TASKBAR_ACTIVATE, (WPARAM)bIsMinimized);

	SetFocus();
}
//***************************************************************************
void CBCGPMDIChildWnd::EnableTaskbarThumbnailClipRect(BOOL bEnable)
{
	m_bEnableTaskbarThumbnailClip = bEnable;

	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rect(0, 0, 0, 0); 
	if (bEnable) 
	{
		rect = GetTaskbarThumbnailClipRect();
	}

	SetTaskbarThumbnailClipRect(rect);	
}
//***************************************************************************
CRect CBCGPMDIChildWnd::GetTaskbarThumbnailClipRect() const
{
	ASSERT_VALID(this);

	CRect rect(0, 0, 0, 0);
	GetWindowRect(rect);
	
	return rect;
}
//***************************************************************************
BOOL CBCGPMDIChildWnd::SetTaskbarThumbnailClipRect(CRect rect)
{
	if (!globalData.bIsWindows7)
	{
		return FALSE;
	}
	
	CBCGPMDIFrameWnd* pTopLevel = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, GetTopLevelFrame());
	if (pTopLevel == NULL || pTopLevel->MDIGetActive() != this)
	{
		return FALSE;
	}

	if (!rect.IsRectNull())
	{
		pTopLevel->ScreenToClient(rect);
	}

	return globalData.TaskBar_SetThumbnailClip(pTopLevel->GetSafeHwnd(), rect.IsRectNull() || rect.IsRectEmpty() ? NULL : &rect);
}
//***************************************************************************
void CBCGPMDIChildWnd::OnSysCommand(UINT nID, LPARAM lParam) 
{
	UINT nItemID = (nID & 0xFFF0);

	if (nItemID == SC_KEYMENU && (GetSystemMenu(FALSE) == NULL || ((GetStyle() & WS_SYSMENU) == 0)))
	{
		return;
	}
	
	CMDIChildWnd::OnSysCommand(nID, lParam);
}

//////////////////////////////////////////////////
/// CBCGPMDITabProxyWnd

IMPLEMENT_DYNCREATE(CBCGPMDITabProxyWnd, CWnd)

BEGIN_MESSAGE_MAP(CBCGPMDITabProxyWnd, CWnd)
	ON_MESSAGE(WM_DWMSENDICONICTHUMBNAIL, OnSendIconicThumbnail)
	ON_MESSAGE(WM_DWMSENDICONICLIVEPREVIEWBITMAP, OnSendIconicLivePreviewBitmap)
	ON_WM_ACTIVATE()
	ON_WM_MOUSEACTIVATE()
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

CBCGPMDITabProxyWnd::CBCGPMDITabProxyWnd() : m_pRelatedMDIChildFrame(NULL)
{
}
//***************************************************************************
CBCGPMDITabProxyWnd::~CBCGPMDITabProxyWnd()
{
}
//***************************************************************************
void CBCGPMDITabProxyWnd::SetRelatedMDIChildFrame(CBCGPMDIChildWnd* pRelatedMDIFrame)
{
	ASSERT_KINDOF(CBCGPMDIChildWnd, pRelatedMDIFrame);
	m_pRelatedMDIChildFrame = pRelatedMDIFrame;
}
//***************************************************************************
BOOL CBCGPMDITabProxyWnd::IsMDIChildActive()
{
	ASSERT_VALID(m_pRelatedMDIChildFrame);

	CBCGPMDIFrameWnd* pTopLevel = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, m_pRelatedMDIChildFrame->GetTopLevelFrame());
	ASSERT_VALID(pTopLevel);

	if (pTopLevel->IsIconic())
	{
		return FALSE;
	}

	return pTopLevel->MDIGetActive() == m_pRelatedMDIChildFrame;
}
//***************************************************************************
static double CorrectZoomSize(const CSize& sizeSrc, CSize& sizeDst)
{
	double dblZoom = min((double)sizeDst.cx / sizeSrc.cx, (double)sizeDst.cy / sizeSrc.cy);

	sizeDst.cx = (long)(sizeSrc.cx * dblZoom);
	sizeDst.cy = (long)(sizeSrc.cy * dblZoom);

	return dblZoom;
}
//***************************************************************************

class CBCGPView : public CView
{
	friend class CBCGPMDITabProxyWnd;
};

BOOL CBCGPMDITabProxyWnd::OnPrepareScrollViewPreview(CDC* pDC, CScrollView* pScrollView)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pScrollView);

	if (DYNAMIC_DOWNCAST(CFormView, pScrollView) != NULL)
	{
		return FALSE;
	}

	CRect rect;
	pScrollView->GetClientRect(rect);

	CHtmlView* pHTMLView = DYNAMIC_DOWNCAST(CHtmlView, pScrollView);
	if (pHTMLView != NULL)
	{
		OleDraw(pHTMLView->GetHtmlDocument(), DVASPECT_CONTENT, pDC->GetSafeHdc(), rect);
		return TRUE;
	}

	CDC dcScrollView;
	dcScrollView.CreateCompatibleDC(pDC);

	HBITMAP hBitmapScrollView = CBCGPDrawManager::CreateBitmap_32 (rect.Size(), NULL);

	HBITMAP hBmpOld = (HBITMAP)dcScrollView.SelectObject(hBitmapScrollView);

	dcScrollView.BitBlt(0, 0, rect.Width(), rect.Height(), pDC, 0, 0, SRCCOPY);

	dcScrollView.SetViewportOrg(0, 0);
	dcScrollView.SetWindowOrg(0, 0);
	dcScrollView.SetMapMode(MM_TEXT);

	pScrollView->OnPrepareDC(&dcScrollView, NULL);
	((CBCGPView*)pScrollView)->OnDraw(&dcScrollView);

	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dcScrollView, 0, 0, SRCCOPY);

	dcScrollView.SelectObject(hBmpOld);
	::DeleteObject(hBitmapScrollView);

	return TRUE;
}
//***************************************************************************
HBITMAP CBCGPMDITabProxyWnd::GetClientBitmap (int nWidth, int nHeight, BOOL bIsThumbnail)
{
	if (m_pRelatedMDIChildFrame == NULL)
	{
		return NULL;
	}

	const int nMinSize = 5;

	nHeight = max(nMinSize, nHeight);
	nWidth = max(nMinSize, nWidth);

	CWnd* pWndPreview = m_pRelatedMDIChildFrame->GetTaskbarPreviewWnd();
	if (pWndPreview == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(pWndPreview);

	CRect rectWnd;
	pWndPreview->GetWindowRect(rectWnd);

	rectWnd.OffsetRect(-rectWnd.left, -rectWnd.top);

	if (rectWnd.Height() <= nMinSize)
	{
		rectWnd.bottom = rectWnd.top + nMinSize;
	}

	if (rectWnd.Width() <= nMinSize)
	{
		rectWnd.right = rectWnd.left + nMinSize;
	}

	CSize szDst(nWidth, nHeight);
	CorrectZoomSize(rectWnd.Size(), szDst);
	CRect rectDst (CPoint(0, 0), szDst);

	HBITMAP hBitmap = CBCGPDrawManager::CreateBitmap_32 (szDst, NULL);

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	HBITMAP hBitmapOld = (HBITMAP)dc.SelectObject (hBitmap);

	CDC dcView;
	dcView.CreateCompatibleDC(&dc);

	LPBYTE pBitsView = NULL;
	HBITMAP hBitmapView = CBCGPDrawManager::CreateBitmap_32 (rectWnd.Size(), (void**)&pBitsView);

	HBITMAP hBitmapViewOld = (HBITMAP)dcView.SelectObject (hBitmapView);

	dcView.FillRect(rectWnd, &globalData.brWindow);

	CBCGPTaskBarPreviewParameters params;

	params.m_bIsThumbnail = bIsThumbnail;
	params.m_rectDraw = rectWnd;
	params.m_sizeDest = szDst;

	if (pWndPreview->SendMessage(BCGM_ON_PREPARE_TASKBAR_PREVIEW, (WPARAM)dcView.GetSafeHdc(), (LPARAM)&params) == 0)
	{
		params.m_bDontStretch = FALSE;

		BOOL bIsReady = FALSE;

		CView* pView = DYNAMIC_DOWNCAST(CView, pWndPreview);
		if (pView != NULL)
		{
			CScrollView* pScrollView = DYNAMIC_DOWNCAST(CScrollView, pWndPreview);
			if (pScrollView != NULL)
			{
				bIsReady = OnPrepareScrollViewPreview(&dcView, pScrollView);
			}
			else if (pView->IsKindOf(RUNTIME_CLASS(CRichEditView)))
			{
				CRichEditView* pRichEditView = (CRichEditView*)pWndPreview;

				CRect rectEdit;
				pRichEditView->GetRichEditCtrl().GetRect(rectEdit);

				long lPageWidth = ::MulDiv(rectEdit.Width(), 1440, dcView.GetDeviceCaps(LOGPIXELSX));
				long lPageHeight = ::MulDiv(rectEdit.Height(), 1440, dcView.GetDeviceCaps(LOGPIXELSY));

				CRect rcPage(rectEdit.left, rectEdit.top, lPageWidth, lPageHeight);

				FORMATRANGE fr;

				fr.hdcTarget = dcView.m_hDC;
				fr.hdc = dcView.m_hDC;
				fr.rcPage = rcPage;
				fr.rc = rcPage;

				int nLine = pRichEditView->GetRichEditCtrl().GetFirstVisibleLine();

				fr.chrg.cpMin = pRichEditView->GetRichEditCtrl().LineIndex(nLine);
				fr.chrg.cpMax = -1;

				pRichEditView->GetRichEditCtrl().FormatRange(&fr,TRUE);
				bIsReady = TRUE;
			}
			else if (!pView->IsKindOf(RUNTIME_CLASS(CCtrlView)) && pView->GetWindow(GW_CHILD) == NULL)
			{
				pView->OnPrepareDC(&dcView, NULL);
				((CBCGPView*)pView)->OnDraw(&dcView);

				bIsReady = TRUE;
			}
		}

		if (!bIsReady)
		{
			pWndPreview->SendMessage(WM_PRINT, (WPARAM)dcView.GetSafeHdc(), (LPARAM)(PRF_CLIENT | PRF_CHILDREN | PRF_NONCLIENT | PRF_ERASEBKGND));
		}
	}

	for (int i = 0; i < rectWnd.Width() * rectWnd.Height(); i++)
	{
		pBitsView[3] = 255;
		pBitsView += 4;
	}

	if (params.m_bDontStretch)
	{
		dc.BitBlt(0, 0, szDst.cx, szDst.cy, &dcView, rectWnd.left, rectWnd.top, SRCCOPY);
	}
	else
	{
		if (szDst.cx < rectWnd.Width() && szDst.cy < rectWnd.Height())
		{
			dc.SetStretchBltMode(HALFTONE);
		}
		
		dc.StretchBlt(0, 0, szDst.cx, szDst.cy, &dcView, rectWnd.left, rectWnd.top,
					rectWnd.Width(), rectWnd.Height(), SRCCOPY);
	}

	dcView.SelectObject (hBitmapViewOld);
	::DeleteObject(hBitmapView);

	dc.SelectObject (hBitmapOld);

	return hBitmap;
}
//***************************************************************************
LRESULT CBCGPMDITabProxyWnd::OnSendIconicThumbnail(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);

	if (m_pRelatedMDIChildFrame == NULL)
	{
		return Default();
	}

	int nWidth = HIWORD(lParam); 
	int nHeight = LOWORD(lParam);

	HBITMAP hBitmap = m_pRelatedMDIChildFrame->OnGetIconicThumbnail(nWidth, nHeight); 
	if (hBitmap == NULL)
	{
		hBitmap = GetClientBitmap(nWidth, nHeight, TRUE);
	}
	
	if (hBitmap != NULL)
	{
		globalData.DwmSetIconicThumbnail(GetSafeHwnd(), hBitmap, 0);

		DeleteObject(hBitmap);
	}

	return Default();
}
//***************************************************************************
LRESULT CBCGPMDITabProxyWnd::OnSendIconicLivePreviewBitmap(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	if (m_pRelatedMDIChildFrame == NULL)
	{
		return Default();
	}

	ASSERT_VALID(m_pRelatedMDIChildFrame);

	BOOL bActive = IsMDIChildActive();
	CPoint ptClient(0,0);

	HBITMAP hBitmap = m_pRelatedMDIChildFrame->OnGetIconicLivePreviewBitmap(bActive, ptClient);
	if (hBitmap == NULL)
	{
		CBCGPMDIFrameWnd* pTopLevelFrame = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, m_pRelatedMDIChildFrame->GetTopLevelFrame());
		ASSERT_VALID(pTopLevelFrame);

		CWnd* pPrintWnd = m_pRelatedMDIChildFrame->GetTaskbarPreviewWnd();
		if (pPrintWnd != NULL)
		{
			ASSERT_VALID(pPrintWnd);

			CRect rectWnd;
			pPrintWnd->GetWindowRect(rectWnd);
			pTopLevelFrame->ScreenToClient(rectWnd);

			ptClient = rectWnd.TopLeft();

			if (pTopLevelFrame->GetMenu() != NULL)
			{
				ptClient.y += ::GetSystemMetrics(SM_CYMENU);
			}

			if (!CBCGPVisualManager::GetInstance()->IsDWMCaptionSupported())
			{
				CSize szSystemBorder (globalUtils.GetSystemBorders(pTopLevelFrame));
				if (CBCGPVisualManager::GetInstance()->IsSmallSystemBorders())
				{
					szSystemBorder = CSize (3, 3);
				}

				ptClient.x += szSystemBorder.cx;
				ptClient.y += szSystemBorder.cy;

				if (pTopLevelFrame->GetRibbonBar() == NULL)
				{
					ptClient.y += ::GetSystemMetrics (SM_CYCAPTION);
				}
			}

			hBitmap = GetClientBitmap(rectWnd.Width(), rectWnd.Height(), FALSE);
		}
	}

	if (hBitmap != NULL)
	{
		globalData.DwmSetIconicLivePreviewBitmap(GetSafeHwnd(), hBitmap, &ptClient, 0);
		DeleteObject(hBitmap);

		CBCGPPopupMenu* pActivePopupMenu = CBCGPPopupMenu::GetSafeActivePopupMenu();
		if (pActivePopupMenu->GetSafeHwnd() != NULL)
		{
			pActivePopupMenu->SendMessage(WM_CLOSE);
		}

		return 0;
	}

	return 1;
}
//***************************************************************************
void CBCGPMDITabProxyWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (m_pRelatedMDIChildFrame == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pRelatedMDIChildFrame);

	CBCGPMDIFrameWnd* pTopLevelFrame = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, m_pRelatedMDIChildFrame->GetTopLevelFrame());
	ASSERT_VALID(pTopLevelFrame);

	if (nID != SC_CLOSE)
	{
		if (nID != SC_MINIMIZE)
		{
			m_pRelatedMDIChildFrame->ActivateTopLevelFrame();
		}

		pTopLevelFrame->SendMessage(WM_SYSCOMMAND, nID, lParam);	
		return;
	}

	Default();
}
//***************************************************************************
void CBCGPMDITabProxyWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	if (m_pRelatedMDIChildFrame != NULL)
	{
		ASSERT_VALID(m_pRelatedMDIChildFrame);
		m_pRelatedMDIChildFrame->OnTaskbarTabThumbnailActivate(nState, pWndOther, bMinimized);
	}
}
//***************************************************************************
int CBCGPMDITabProxyWnd::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	if (m_pRelatedMDIChildFrame == NULL)
	{
		return 0;
	}

	ASSERT_VALID(m_pRelatedMDIChildFrame);
	return m_pRelatedMDIChildFrame->OnTaskbarTabThumbnailMouseActivate(pDesktopWnd, nHitTest, message);
}
//***************************************************************************
void CBCGPMDITabProxyWnd::OnClose()
{
	if (m_pRelatedMDIChildFrame != NULL)
	{
		ASSERT_VALID(m_pRelatedMDIChildFrame);
		m_pRelatedMDIChildFrame->OnPressTaskbarThmbnailCloseButton();
	}
}
