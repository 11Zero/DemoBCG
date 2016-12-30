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

// BCGPFrameWnd.cpp : implementation file
//

#include "stdafx.h"
#include "afxpriv.h"
#include "BCGPFrameWnd.h"
#include "BCGPMenuBar.h"
#include "BCGPPopupMenu.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPUserToolsManager.h"
#include "BCGPPrintPreviewView.h"
#include "BCGPSlider.h"
#include "BCGPRibbonBar.h"
#include "BCGPRibbonStatusBar.h"
#include "BCGPVisualManager.h"
#include "BCGPGlobalUtils.h"
#include "BCGPSlider.h"
#include "BCGPRibbonBackstageView.h"

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

/////////////////////////////////////////////////////////////////////////////
// CBCGPFrameWnd

IMPLEMENT_DYNCREATE(CBCGPFrameWnd, CFrameWnd)

#pragma warning (disable : 4355)

CBCGPFrameWnd::CBCGPFrameWnd() :
	m_Impl (this),
	m_bContextHelp (FALSE),
	m_bWasMaximized (FALSE),
	m_bIsMinimized (FALSE),
	m_pPrintPreviewFrame (NULL),
	m_bClosing (FALSE)
{
}

#pragma warning (default : 4355)

CBCGPFrameWnd::~CBCGPFrameWnd()
{
}

BEGIN_MESSAGE_MAP(CBCGPFrameWnd, CFrameWnd)
	//{{AFX_MSG_MAP(CBCGPFrameWnd)
	ON_WM_MENUCHAR()
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_SIZING()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_NCPAINT()
	ON_WM_NCACTIVATE()
	ON_WM_CREATE()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCHITTEST()
	ON_WM_NCCALCSIZE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_WM_ACTIVATEAPP()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_REGISTERED_MESSAGE(BCGM_CREATETOOLBAR, OnToolbarCreateNew)
	ON_REGISTERED_MESSAGE(BCGM_DELETETOOLBAR, OnToolbarDelete)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STATUS_BAR, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_STATUS_BAR, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_TOOLBAR, OnBarCheck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_REBAR, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_REBAR, OnBarCheck)
	ON_REGISTERED_MESSAGE(BCGM_TOOLBARMENU, OnToolbarContextMenu)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	ON_MESSAGE(WM_EXITSIZEMOVE, OnExitSizeMove)
	ON_REGISTERED_MESSAGE(BCGM_CHANGEVISUALMANAGER, OnChangeVisualManager)
	ON_REGISTERED_MESSAGE(BCGM_POSTSETPREVIEWFRAME, OnPostPreviewFrame)
	ON_MESSAGE(WM_DWMCOMPOSITIONCHANGED, OnDWMCompositionChanged)
	ON_MESSAGE(WM_POWERBROADCAST, OnPowerBroadcast)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPFrameWnd message handlers

LRESULT CBCGPFrameWnd::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu) 
{
	if (m_Impl.OnMenuChar (nChar))
	{
		return MAKELPARAM (MNC_EXECUTE, -1);
	}
		
	return CFrameWnd::OnMenuChar(nChar, nFlags, pMenu);
}
//*******************************************************************************************
afx_msg LRESULT CBCGPFrameWnd::OnSetMenu (WPARAM wp, LPARAM lp)
{
	OnSetMenu ((HMENU) wp);
	return DefWindowProc (WM_MDISETMENU, NULL, lp);
}
//*******************************************************************************************
BOOL CBCGPFrameWnd::OnSetMenu (HMENU hmenu)
{
	if (m_Impl.m_pMenuBar != NULL)
	{
		m_Impl.m_pMenuBar->CreateFromMenu 
			(hmenu == NULL ? m_Impl.m_hDefaultMenu : hmenu);
		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPFrameWnd::PreTranslateMessage(MSG* pMsg) 
{
	BOOL bProcessAccel = TRUE;

	switch (pMsg->message)
	{
	case WM_SYSKEYDOWN:
#ifndef BCGP_EXCLUDE_RIBBON
		if (m_Impl.m_pRibbonBar != NULL && m_Impl.m_pRibbonBar->OnSysKeyDown (this, pMsg->wParam, pMsg->lParam))
		{
			return TRUE;
		}

		if (pMsg->wParam == VK_F4 && m_Impl.m_pRibbonBar != NULL && m_Impl.m_pRibbonBar->IsBackstageViewActive())
		{
			break;
		}
#endif
		if (CBCGPPopupMenu::GetSafeActivePopupMenu() == NULL)
		{
			m_Impl.CancelToolbarMode();
		}

	case WM_CONTEXTMENU:
		if (!globalData.m_bSysUnderlineKeyboardShortcuts && !globalData.m_bUnderlineKeyboardShortcuts)
		{
			globalData.m_bUnderlineKeyboardShortcuts = TRUE;
			CBCGPToolBar::RedrawUnderlines ();
		}

		if (CBCGPPopupMenu::GetSafeActivePopupMenu() != NULL && (pMsg->wParam == VK_MENU || pMsg->wParam == VK_F10))
		{
			CBCGPPopupMenu::m_pActivePopupMenu->SendMessage (WM_CLOSE);
			return TRUE;
		}
		else if (m_Impl.ProcessKeyboard ((int) pMsg->wParam))
		{
			return TRUE;
		}
		break;

	case WM_SYSKEYUP:
		if (m_Impl.ProcessSysKeyUp(pMsg->wParam, pMsg->lParam))
		{
			return TRUE;
		}
		break;

	case WM_KEYDOWN:
		//-----------------------------------------
		// Pass keyboard action to the active menu:
		//-----------------------------------------
		if (!CBCGPFrameImpl::IsHelpKey (pMsg) && 
			m_Impl.ProcessKeyboard ((int) pMsg->wParam, &bProcessAccel))
		{
			return TRUE;
		}

		if (pMsg->wParam == VK_ESCAPE)
		{
			if (IsFullScreen())
			{
				if (!IsPrintPreview())
				{
					m_Impl.InvokeFullScreenCommand();
					return TRUE;
				}
			}

			CBCGPSmartDockingManager* pSDManager = NULL;
			if ((pSDManager = m_dockManager.GetSDManagerPermanent()) != NULL &&
				pSDManager->IsStarted())
			{
				pSDManager->CauseCancelMode ();
			}

			CBCGPSlider* pSlider = DYNAMIC_DOWNCAST (CBCGPSlider, GetCapture ());
			if (pSlider != NULL)
			{
				pSlider->SendMessage (WM_CANCELMODE);
				return TRUE;
			}
		}

		if (!bProcessAccel)
		{
			return FALSE;
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		{
			CPoint pt (BCG_GET_X_LPARAM(pMsg->lParam), BCG_GET_Y_LPARAM(pMsg->lParam));
			CWnd* pWnd = CWnd::FromHandle (pMsg->hwnd);

			if (pWnd != NULL && ::IsWindow (pMsg->hwnd))
			{
				pWnd->ClientToScreen (&pt);
			}

			if (m_Impl.ProcessMouseClick (pMsg->message, pt, pMsg->hwnd))
			{
				return TRUE;
			}

			if (!::IsWindow (pMsg->hwnd))
			{
				return TRUE;
			}
		}
		break;

	case WM_NCLBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONDOWN:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONDOWN:
	case WM_NCMBUTTONUP:
		if (m_Impl.ProcessMouseClick (pMsg->message,
			CPoint (BCG_GET_X_LPARAM(pMsg->lParam), BCG_GET_Y_LPARAM(pMsg->lParam)),
			pMsg->hwnd))
		{
			return TRUE;
		}
		break;

	case WM_MOUSEWHEEL:
		if (m_Impl.ProcessMouseWheel (pMsg->wParam, pMsg->lParam))
		{
			return TRUE;
		}
		break;

	case WM_MOUSEMOVE:
		{
			CPoint pt (BCG_GET_X_LPARAM(pMsg->lParam), BCG_GET_Y_LPARAM(pMsg->lParam));
			CWnd* pWnd = CWnd::FromHandle (pMsg->hwnd);

			if (pWnd != NULL)
			{
				pWnd->ClientToScreen (&pt);
			}

			if (m_Impl.ProcessMouseMove (pt))
			{
				return TRUE;
			}
		}
	}

	return CFrameWnd::PreTranslateMessage(pMsg);
}
//*******************************************************************************************
BOOL CBCGPFrameWnd::ShowPopupMenu (CBCGPPopupMenu* pMenuPopup)
{
	if (!m_Impl.OnShowPopupMenu (pMenuPopup, this))
	{
		return FALSE;
	}

	if (pMenuPopup != NULL && pMenuPopup->m_bShown)
	{
		return TRUE;
	}

	BOOL bResult = OnShowPopupMenu (pMenuPopup);

	return bResult; 
}
//*******************************************************************************************
void CBCGPFrameWnd::OnClosePopupMenu (CBCGPPopupMenu* pMenuPopup)
{
	if (globalData.IsAccessibilitySupport () && pMenuPopup != NULL)
	{
		CBCGPPopupMenu* pPopupParent = pMenuPopup->GetParentPopupMenu ();
		CBCGPToolbarMenuButton* pParentButton  = pMenuPopup->GetParentButton ();

		if (pMenuPopup->IsEscClose () || pPopupParent != NULL || pParentButton == NULL)
		{
			globalData.NotifyWinEvent (EVENT_SYSTEM_MENUPOPUPEND,
				pMenuPopup->GetSafeHwnd (), OBJID_WINDOW, CHILDID_SELF);
		}
		else
		{
			globalData.NotifyWinEvent (EVENT_SYSTEM_MENUEND,
				pMenuPopup->GetSafeHwnd (), OBJID_WINDOW, CHILDID_SELF);
		}
				

		CBCGPMenuBar* pMenuBar = (CBCGPMenuBar*) GetMenuBar ();
		
		if (pMenuBar->GetSafeHwnd () != NULL && pMenuBar == GetFocus ())
		{
			pMenuBar->AccNotifyObjectFocusEvent ();
		}
	}

	if (CBCGPPopupMenu::m_pActivePopupMenu == pMenuPopup)
	{
		CBCGPPopupMenu::m_pActivePopupMenu = NULL;
	}

	m_dockManager.OnClosePopupMenu ();
}
//*******************************************************************************************
BOOL CBCGPFrameWnd::OnDrawMenuImage (CDC* pDC, 
									const CBCGPToolbarMenuButton* pMenuButton, 
									const CRect& rectImage)
{
	ASSERT_VALID (this);

#ifndef BCGP_EXCLUDE_RIBBON
	if (m_Impl.m_pRibbonBar != NULL)
	{
		ASSERT_VALID (m_Impl.m_pRibbonBar);
		return m_Impl.m_pRibbonBar->DrawMenuImage (pDC, pMenuButton, rectImage);
	}
#endif

	return FALSE;
}
//*******************************************************************************************
BOOL CBCGPFrameWnd::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD (wParam) == 1)
	{
		UINT uiCmd = LOWORD (wParam);

		CBCGPToolBar::AddCommandUsage (uiCmd);

		//---------------------------
		// Simmulate ESC keystroke...
		//---------------------------
		if (m_Impl.ProcessKeyboard (VK_ESCAPE))
		{
			return TRUE;
		}

		if (g_pUserToolsManager != NULL &&
			g_pUserToolsManager->InvokeTool (uiCmd))
		{
			return TRUE;
		}
	}

	if (!CBCGPToolBar::IsCustomizeMode ())
	{
		return CFrameWnd::OnCommand(wParam, lParam);
	}

	return FALSE;
}
//******************************************************************
BOOL CBCGPFrameWnd::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	m_Impl.m_nIDDefaultResource = nIDResource;
	m_Impl.LoadLargeIconsState ();

	if (!CFrameWnd::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	m_Impl.OnLoadFrame ();
	return TRUE;
}
//***************************************************************************************
void CBCGPFrameWnd::OnClose() 
{
	if (m_pPrintPreviewFrame != NULL)
	{
		m_pPrintPreviewFrame->SendMessage (WM_COMMAND, AFX_ID_PREVIEW_CLOSE);
		m_pPrintPreviewFrame = NULL;
		return;
	}

	if (!m_Impl.IsPrintPreview ())
	{
		m_bClosing = TRUE;
	}

	// Deactivate OLE container first:
	COleClientItem*	pActiveItem = GetInPlaceActiveItem ();
	if (pActiveItem != NULL)
	{
		pActiveItem->Deactivate ();
	}

	m_Impl.OnCloseFrame();	
	CFrameWnd::OnClose();
}
//***************************************************************************************
void CBCGPFrameWnd::ClosePrintPreview()
{
	if (IsPrintPreview())
	{
		SendMessage(WM_CLOSE);
	}
}
//***************************************************************************************
BOOL CBCGPFrameWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
	m_dockManager.Create (this);
	m_Impl.SetDockManager (&m_dockManager);

	m_Impl.RestorePosition(cs);
	return CFrameWnd::PreCreateWindow(cs);
}
//***************************************************************************************

#if _MSC_VER >= 1300
void CBCGPFrameWnd::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
{
	if (dwData > 0 || !m_bContextHelp)
	{
		CFrameWnd::HtmlHelp(dwData, nCmd);
	}
	else
	{
		OnContextHelp ();
	}
}
#endif

//***************************************************************************************
void CBCGPFrameWnd::WinHelp(DWORD_PTR dwData, UINT nCmd) 
{
	if (dwData > 0 || !m_bContextHelp)
	{
		CFrameWnd::WinHelp(dwData, nCmd);
	}
	else
	{
		OnContextHelp ();
	}
}
//***************************************************************************************
void CBCGPFrameWnd::OnContextHelp ()
{
	m_bContextHelp = TRUE;

	if (!m_bHelpMode && CanEnterHelpMode())
	{
		CBCGPToolBar::SetHelpMode ();
	}

	CFrameWnd::OnContextHelp ();

	if (!m_bHelpMode)
	{
		CBCGPToolBar::SetHelpMode (FALSE);
	}

	m_bContextHelp = FALSE;
}
//*******************************************************************************************
LRESULT CBCGPFrameWnd::OnToolbarCreateNew(WPARAM,LPARAM lp)
{
	ASSERT (lp != NULL);
	return (LRESULT) m_Impl.CreateNewToolBar ((LPCTSTR) lp);
}
//***************************************************************************************
LRESULT CBCGPFrameWnd::OnToolbarDelete(WPARAM,LPARAM lp)
{
	CBCGPToolBar* pToolbar = (CBCGPToolBar*) lp;
	ASSERT_VALID (pToolbar);

	return (LRESULT) m_Impl.DeleteToolBar (pToolbar);
}
//***************************************************************************************
BOOL CBCGPFrameWnd::DockControlBarLeftOf (CBCGPControlBar* pBar, CBCGPControlBar* pLeftOf)
{
	return m_dockManager.DockControlBarLeftOf (pBar, pLeftOf);
}
//************************************************************************************
void CBCGPFrameWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CFrameWnd::OnActivate(nState, pWndOther, bMinimized);

	switch (nState)
	{
	case WA_CLICKACTIVE:
		UpdateWindow ();
		break;

	case WA_INACTIVE:
		if (!CBCGPToolBar::IsCustomizeMode ())
		{
			m_Impl.DeactivateMenu ();
		}
		break;
	}

	m_Impl.AdjustShadow(nState != WA_INACTIVE && !bMinimized);

	if (nState != WA_INACTIVE)
	{
		m_Impl.ActivateRibbonBackstageView();
	}
}
//****************************************************************************
#if _MSC_VER >= 1300
void CBCGPFrameWnd::OnActivateApp(BOOL bActive, DWORD /*dwThreadID*/)
#else
void CBCGPFrameWnd::OnActivateApp(BOOL bActive, HTASK /*hTask*/) 
#endif
{
	m_dockManager.OnActivateFrame (bActive);
	m_Impl.OnActivateApp (bActive);
}
//****************************************************************************
void CBCGPFrameWnd::DelayUpdateFrameMenu(HMENU hMenuAlt)
{
	OnUpdateFrameMenu (hMenuAlt);
	CFrameWnd::DelayUpdateFrameMenu (hMenuAlt);
}
//****************************************************************************
COleClientItem*	CBCGPFrameWnd::GetInPlaceActiveItem ()
{
	CFrameWnd* pActiveFrame = GetActiveFrame ();
	if (pActiveFrame == NULL)
	{
		return NULL;
	}

	ASSERT_VALID (pActiveFrame);

	CView* pView = pActiveFrame->GetActiveView ();
	if (pView == NULL || pView->IsKindOf (RUNTIME_CLASS (CBCGPPrintPreviewView)))
	{
		return NULL;
	}

	ASSERT_VALID (pView);

	COleDocument* pDoc = DYNAMIC_DOWNCAST (COleDocument, pView->GetDocument ());
	if (pDoc == NULL)
	{
		return NULL;
	}

	ASSERT_VALID (pDoc);
	return pDoc->GetInPlaceActiveItem (pView);
}
//****************************************************************************
void CBCGPFrameWnd::OnUpdateFrameMenu (HMENU hMenuAlt)
{
	CFrameWnd::OnUpdateFrameMenu (hMenuAlt);

	BOOL bIsMenuBar = m_Impl.m_pMenuBar != NULL &&
		(m_Impl.m_pMenuBar->GetStyle () & WS_VISIBLE);

	BOOL bIsRibbon = FALSE;

#ifndef BCGP_EXCLUDE_RIBBON
	if (m_Impl.m_pRibbonBar != NULL &&
		(m_Impl.m_pRibbonBar->GetStyle () & WS_VISIBLE))
	{
		bIsRibbon = TRUE;
	}
#endif

	if (bIsMenuBar || bIsRibbon)
	{
		COleClientItem*	pActiveItem = GetInPlaceActiveItem ();
		if (pActiveItem == NULL ||
			pActiveItem->GetInPlaceWindow () == NULL)
		{
			SetMenu (NULL);
		}
		else
		{
			SetMenu (CMenu::FromHandle (hMenuAlt));
		}
	}
}
//*****************************************************************************
void CBCGPFrameWnd::OnDestroy() 
{
    if (m_hAccelTable != NULL)
    {
        ::DestroyAcceleratorTable (m_hAccelTable);
        m_hAccelTable = NULL;
    }

	m_dockManager.m_bEnableAdjustLayout = FALSE;

	CList<HWND, HWND> lstChildren;

	for (int i = 0; i < 2; i++)
	{
		CWnd* pNextWnd = GetTopWindow ();
		while (pNextWnd != NULL)
		{
			const BOOL bIsSlider = pNextWnd->IsKindOf(RUNTIME_CLASS(CBCGPSlider));

			if ((i == 0 && !bIsSlider) || (i == 1 && bIsSlider))
			{
				lstChildren.AddTail (pNextWnd->m_hWnd);
			}

			pNextWnd = pNextWnd->GetNextWindow ();
		}
	}

	for (POSITION pos = lstChildren.GetHeadPosition (); pos != NULL;)
	{
		HWND hwndNext = lstChildren.GetNext (pos);
		if (IsWindow (hwndNext) && ::GetParent (hwndNext) == m_hWnd)
		{
			::DestroyWindow (hwndNext);
		}
	}

	CBCGPFrameImpl::RemoveFrame (this);
	CFrameWnd::OnDestroy();
}

//*****************************************************************************
//******************* dockmanager layer ***************************************
//*****************************************************************************
void CBCGPFrameWnd::AddDockBar ()
{
	ASSERT_VALID (this);
}
//*****************************************************************************
BOOL CBCGPFrameWnd::AddControlBar (CBCGPBaseControlBar* pControlBar, BOOL bTail)
{
	ASSERT_VALID (this);

#ifndef BCGP_EXCLUDE_RIBBON
	CBCGPRibbonBar* pRibbonBar = DYNAMIC_DOWNCAST (CBCGPRibbonBar, pControlBar);
	if (pRibbonBar != NULL)
	{
		ASSERT_VALID (pRibbonBar);

		if (pRibbonBar->IsMainRibbonBar ())
		{
			m_Impl.m_pRibbonBar = pRibbonBar;
		}
	}

	CBCGPRibbonStatusBar* pRibbonStatusBar = DYNAMIC_DOWNCAST (CBCGPRibbonStatusBar, pControlBar);
	if (pRibbonStatusBar != NULL)
	{
		ASSERT_VALID (pRibbonStatusBar);
		m_Impl.m_pRibbonStatusBar = pRibbonStatusBar;
	}
#endif

	return m_dockManager.AddControlBar (pControlBar, bTail);
}
//*****************************************************************************
BOOL CBCGPFrameWnd::InsertControlBar (CBCGPBaseControlBar* pControlBar, 
									  CBCGPBaseControlBar* pTarget, BOOL bAfter)
{
	ASSERT_VALID (this);
	return m_dockManager.InsertControlBar (pControlBar, pTarget, bAfter);
}
//*****************************************************************************
void CBCGPFrameWnd::RemoveControlBarFromDockManager (CBCGPBaseControlBar* pControlBar, BOOL bDestroy,
									  BOOL bAdjustLayout, BOOL bAutoHide, CBCGPBaseControlBar* pBarReplacement)
{
	ASSERT_VALID (this);
	m_dockManager.RemoveControlBarFromDockManager (pControlBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
}
//*****************************************************************************
void CBCGPFrameWnd::DockControlBar (CBCGPBaseControlBar* pBar, UINT nDockBarID, 
									LPCRECT lpRect)
{
	ASSERT_VALID (this);
	m_dockManager.DockControlBar (pBar, nDockBarID, lpRect);
}
//*****************************************************************************
CBCGPBaseControlBar* CBCGPFrameWnd::ControlBarFromPoint (CPoint point, 
							int nSensitivity, bool bExactBar, 
							CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID (this);
	return m_dockManager.ControlBarFromPoint (point, nSensitivity, bExactBar, 
												pRTCBarType);
}
//*****************************************************************************
CBCGPBaseControlBar* CBCGPFrameWnd::ControlBarFromPoint (CPoint point, 
								int nSensitivity, DWORD& dwAlignment, 
								CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID (this);
	return m_dockManager.ControlBarFromPoint (point, nSensitivity, dwAlignment, 
												pRTCBarType);
}
//*****************************************************************************
BOOL CBCGPFrameWnd::IsPointNearDockBar (CPoint point, DWORD& dwBarAlignment, 
										BOOL& bOuterEdge) const
{
	ASSERT_VALID (this);
	return m_dockManager.IsPointNearDockBar (point, dwBarAlignment, bOuterEdge);
}
//*****************************************************************************
void CBCGPFrameWnd::AdjustDockingLayout (HDWP hdwp)
{
	ASSERT_VALID (this);
	
	if (m_dockManager.IsInAdjustLayout ())
	{
		return;
	}

	m_dockManager.AdjustDockingLayout (hdwp);

	AdjustClientArea ();
	if (m_dockManager.IsOLEContainerMode ())
	{
		RecalcLayout ();
	}
}
//*****************************************************************************
void CBCGPFrameWnd::AdjustClientArea ()
{
	CWnd* pChildWnd = GetDlgItem (AFX_IDW_PANE_FIRST);
	if (pChildWnd != NULL)
	{
		CRect rectClientAreaBounds = m_dockManager.GetClientAreaBounds ();

		rectClientAreaBounds.left += m_rectBorder.left;
		rectClientAreaBounds.top  += m_rectBorder.top;
		rectClientAreaBounds.right -= m_rectBorder.right;
		rectClientAreaBounds.bottom -= m_rectBorder.bottom;

		pChildWnd->CalcWindowRect (rectClientAreaBounds);

		if (!pChildWnd->IsKindOf (RUNTIME_CLASS (CSplitterWnd)) &&
			!pChildWnd->IsKindOf (RUNTIME_CLASS (CFormView)))
		{
			pChildWnd->ModifyStyle (0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
		}
		else
		{
			pChildWnd->ModifyStyle (0, WS_CLIPSIBLINGS);
		}

		
		pChildWnd->SetWindowPos (&wndBottom, rectClientAreaBounds.left, 
										rectClientAreaBounds.top, 
										rectClientAreaBounds.Width (), 
										rectClientAreaBounds.Height (),
										SWP_NOACTIVATE);
	}
}
//*****************************************************************************
BOOL CBCGPFrameWnd::OnMoveMiniFrame	(CWnd* pFrame)
{
	ASSERT_VALID (this);
	return m_dockManager.OnMoveMiniFrame (pFrame);
}
//*****************************************************************************
BOOL CBCGPFrameWnd::EnableDocking (DWORD dwDockStyle)
{
	return m_dockManager.EnableDocking (dwDockStyle);
}
//****************************************************************************************
BOOL CBCGPFrameWnd::EnableAutoHideBars (DWORD dwDockStyle, BOOL bActivateOnMouseClick)
{
	return m_dockManager.EnableAutoHideBars (dwDockStyle, bActivateOnMouseClick);
}
//****************************************************************************************
void CBCGPFrameWnd::EnableMaximizeFloatingBars(BOOL bEnable, BOOL bMaximizeByDblClick)
{
	m_dockManager.EnableMaximizeFloatingBars(bEnable, bMaximizeByDblClick);
}
//****************************************************************************************
BOOL CBCGPFrameWnd::AreFloatingBarsCanBeMaximized() const
{
	return m_dockManager.AreFloatingBarsCanBeMaximized();
}
//****************************************************************************************
CBCGPBaseControlBar* CBCGPFrameWnd::GetControlBar (UINT nID)
{
	ASSERT_VALID (this);

	CBCGPBaseControlBar* pBar = m_dockManager.FindBarByID (nID, TRUE);
	return pBar;
}
//****************************************************************************************
void CBCGPFrameWnd::ShowControlBar (CBCGPBaseControlBar* pBar, BOOL bShow, BOOL bDelay, BOOL bActivate)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	pBar->ShowControlBar (bShow, bDelay, bActivate);
}
//*************************************************************************************
void CBCGPFrameWnd::OnUpdateControlBarMenu(CCmdUI* pCmdUI)
{
	CBCGPBaseControlBar* pBar = GetControlBar(pCmdUI->m_nID);
	if (pBar != NULL)
	{
		pCmdUI->SetCheck((pBar->GetStyle() & WS_VISIBLE) != 0);
		return;
	}

	pCmdUI->ContinueRouting();
}
//*************************************************************************************
BOOL CBCGPFrameWnd::OnBarCheck(UINT nID)
{
	ASSERT_VALID (this);

	CBCGPBaseControlBar* pBar = GetControlBar(nID);
	if (pBar != NULL)
	{
		ShowControlBar(pBar, (pBar->GetStyle() & WS_VISIBLE) == 0, FALSE, FALSE);
		return TRUE;
	}

	return FALSE;
}
//*************************************************************************************
void CBCGPFrameWnd::OnSizing(UINT fwSide, LPRECT pRect) 
{
	CFrameWnd::OnSizing(fwSide, pRect);
	
	AdjustDockingLayout ();	
}
//*************************************************************************************
void CBCGPFrameWnd::RecalcLayout (BOOL bNotify)
{
	if (m_bInRecalcLayout)
		return;

	m_bInRecalcLayout = TRUE;
	
	BOOL bWasOleInPlaceActive = m_Impl.m_bIsOleInPlaceActive;
	m_Impl.m_bIsOleInPlaceActive = FALSE;

	COleClientItem*	pActiveItem = GetInPlaceActiveItem ();

	if (pActiveItem != NULL && pActiveItem->m_pInPlaceFrame != NULL &&
		pActiveItem->GetItemState () == COleClientItem::activeUIState)
	{
		m_Impl.m_bIsOleInPlaceActive = TRUE;
		m_Impl.m_bHadCaption = (GetStyle () & WS_CAPTION) != 0;
	}

	if (!m_bIsMinimized)
	{
		CView* pView = GetActiveView ();

		if (m_dockManager.IsPrintPreviewValid () ||
			m_pNotifyHook != NULL)
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
				if (bNotify && m_pNotifyHook != NULL)
				{
					ActiveItemRecalcLayout ();
				}
				else
				{
					m_bInRecalcLayout = FALSE;
					CFrameWnd::RecalcLayout (bNotify);

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

	if (bWasOleInPlaceActive != m_Impl.m_bIsOleInPlaceActive)
	{
		if (!m_Impl.m_bHadCaption)
		{
			if (m_Impl.m_bIsOleInPlaceActive)
			{
				ModifyStyle (0, WS_CAPTION);
			}
			else
			{
				ModifyStyle (WS_CAPTION, 0);
			}
		}

		PostMessage(BCGM_CHANGEVISUALMANAGER);
	}

	m_bInRecalcLayout = FALSE;
}
//****************************************************************************************
void CBCGPFrameWnd::ActiveItemRecalcLayout ()
{
	COleClientItem*	pActiveItem = GetInPlaceActiveItem ();

	if (pActiveItem != NULL && pActiveItem->m_pInPlaceFrame != NULL)
	{
		CRect rectBounds = m_dockManager.GetClientAreaBounds ();
		pActiveItem->m_pInPlaceFrame->OnRecalcLayout ();
	}

	AdjustClientArea ();
}
//****************************************************************************************
BOOL CBCGPFrameWnd::NegotiateBorderSpace( UINT nBorderCmd, LPRECT lpRectBorder )
{
	CRect border, request;

	switch (nBorderCmd)
	{
	case borderGet:
	{
		CFrameWnd::NegotiateBorderSpace (nBorderCmd, lpRectBorder);
		CRect rectBounds = m_dockManager.GetClientAreaBounds ();
		ASSERT(lpRectBorder != NULL);

		*lpRectBorder = rectBounds;
		break;
	}
	case borderRequest:
		return TRUE;

	case borderSet:
		return CFrameWnd::NegotiateBorderSpace (nBorderCmd, lpRectBorder);

	default:
		ASSERT(FALSE);  // invalid CFrameWnd::BorderCmd
	}

	return TRUE;
}
//*************************************************************************************
void CBCGPFrameWnd::OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState)
{
	ASSERT_VALID (this);

	CBCGPFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, BCGCBProGetTopLevelFrame (this));
	if (pMainFrame != NULL)
	{
		pMainFrame->SetPrintPreviewFrame (bPreview ? this : NULL);
	}

	m_dockManager.SetPrintPreviewMode (bPreview, pState);
	DWORD dwSavedState = pState->dwStates;
	CFrameWnd::OnSetPreviewMode (bPreview, pState);
	pState->dwStates = dwSavedState;

	AdjustDockingLayout ();
	RecalcLayout ();

#ifndef BCGP_EXCLUDE_RIBBON
	if (m_Impl.m_pRibbonBar != NULL &&
		m_Impl.m_pRibbonBar->IsReplaceFrameCaption ())
	{
		PostMessage (BCGM_POSTSETPREVIEWFRAME, bPreview);
	}
#endif
}
//*************************************************************************************
BOOL CBCGPFrameWnd::OnShowControlBars (BOOL bShow)
{
	ASSERT_VALID (this);

	BOOL bRibbonWasHidden = FALSE;

#ifndef BCGP_EXCLUDE_RIBBON
	if (bShow &&
		m_Impl.m_pRibbonBar != NULL &&
		(m_Impl.m_pRibbonBar->GetStyle () & WS_VISIBLE) == 0)
	{
		bRibbonWasHidden = TRUE;
	}
#endif

	BOOL bResult = m_dockManager.ShowControlBars (bShow);
	AdjustDockingLayout ();

#ifndef BCGP_EXCLUDE_RIBBON
	if (bShow && bRibbonWasHidden &&
		m_Impl.m_pRibbonBar != NULL &&
		(m_Impl.m_pRibbonBar->GetStyle () & WS_VISIBLE))
	{
		m_Impl.m_pRibbonBar->RedrawWindow(NULL, NULL, 
			RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}
#endif
	
	return bResult;
}

//********************************************************************************
LRESULT CBCGPFrameWnd::OnIdleUpdateCmdUI(WPARAM, LPARAM)
{
	m_dockManager.SendMessageToMiniFrames (WM_IDLEUPDATECMDUI);
	return 0L;
}
//****************************************************************************************
void CBCGPFrameWnd::OnSize(UINT nType, int cx, int cy) 
{
	m_bIsMinimized = (nType == SIZE_MINIMIZED);

	if (m_Impl.m_pRibbonBar || m_Impl.IsOwnerDrawCaption ())
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

	m_Impl.UpdateCaption ();
	m_dockManager.OnActivateFrame (!m_bIsMinimized);

	if (!m_bIsMinimized && nType != SIZE_MAXIMIZED && !m_bWasMaximized)
	{
		m_dockManager.m_bSizeFrame = TRUE;
		CFrameWnd::OnSize(nType, cx, cy);
		AdjustDockingLayout ();			
		m_dockManager.m_bSizeFrame = FALSE;
		return;
	}

	CFrameWnd::OnSize(nType, cx, cy);

	if (nType == SIZE_MAXIMIZED || (nType == SIZE_RESTORED && m_bWasMaximized))
	{
		RecalcLayout ();
	}

	m_bWasMaximized = (nType == SIZE_MAXIMIZED);
}
//****************************************************************************************
LRESULT CBCGPFrameWnd::OnExitSizeMove (WPARAM, LPARAM)
{
	RecalcLayout ();
	m_dockManager.FixupVirtualRects ();
	return 0;
}
//****************************************************************************************
void CBCGPFrameWnd::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	if (IsFullScreen ())
	{
		m_Impl.m_FullScreenMgr.OnGetMinMaxInfo (lpMMI);
	}
	else
	{
		m_Impl.OnGetMinMaxInfo (lpMMI);
		CFrameWnd::OnGetMinMaxInfo (lpMMI);
	}
}
//**************************************************************************************
BOOL CBCGPFrameWnd::OnShowPopupMenu (CBCGPPopupMenu* pMenuPopup)
{
	if (globalData.IsAccessibilitySupport () && pMenuPopup != NULL)
	{
		globalData.NotifyWinEvent(EVENT_SYSTEM_MENUPOPUPSTART,
			pMenuPopup->GetSafeHwnd (), OBJID_WINDOW , CHILDID_SELF);
	}

	return TRUE;
}
//**************************************************************************************
LRESULT CBCGPFrameWnd::OnToolbarContextMenu(WPARAM,LPARAM)
{
	return 1l;
}
//**************************************************************************************
BOOL CBCGPFrameWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if (CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
	{
		return TRUE;
	}

	return m_dockManager.ProcessControlBarContextMenuCommand (nID, nCode, pExtra, pHandlerInfo);
}
//**************************************************************************************
void CBCGPFrameWnd::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	if ((lpwndpos->flags & SWP_FRAMECHANGED) == SWP_FRAMECHANGED)
	{
		m_Impl.OnWindowPosChanging (lpwndpos);
	}
	else if (IsZoomed() && (m_Impl.m_pRibbonBar != NULL || m_Impl.IsOwnerDrawCaption()) &&
		globalData.GetShellAutohideBars () != 0)
	{
		if ((lpwndpos->flags & SWP_NOMOVE) != SWP_NOMOVE)
		{
			ShowWindow(SW_RESTORE);
		}
	}

	CFrameWnd::OnWindowPosChanged (lpwndpos);

	if (m_Impl.m_pShadow != NULL)
	{
		m_Impl.m_pShadow->Repos();
	}

	if ((lpwndpos->flags & SWP_NOSIZE) == 0)
	{
		m_Impl.AdjustRibbonBackstageViewRect();
	}
}
//*************************************************************************************
void CBCGPFrameWnd::OnNcPaint() 
{
	if (!m_Impl.OnNcPaint ())
	{
		Default ();
	}
}
//*****************************************************************************************
LRESULT CBCGPFrameWnd::OnSetText(WPARAM, LPARAM lParam) 
{
	LRESULT	lRes = Default();

	m_Impl.OnSetText ((LPCTSTR)lParam);
	return lRes;
}
//*****************************************************************************************
BOOL CBCGPFrameWnd::OnNcActivate(BOOL bActive) 
{
	if (m_Impl.OnNcActivate (bActive))
	{
		return TRUE;
	}

	return CFrameWnd::OnNcActivate (bActive);
}
//**************************************************************************
int CBCGPFrameWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_Impl.m_bHasBorder = (GetStyle () & WS_BORDER) != 0;
	m_Impl.m_bHasCaption = (GetStyle() & WS_CAPTION) != 0;
	
	CBCGPFrameImpl::AddFrame (this);

	OnChangeVisualManager (0, 0);
	return 0;
}
//**************************************************************************
LRESULT CBCGPFrameWnd::OnChangeVisualManager (WPARAM wp, LPARAM lp)
{
	m_dockManager.SendMessageToMiniFrames(BCGM_CHANGEVISUALMANAGER);
	m_dockManager.SendMessageToControlBars(BCGM_CHANGEVISUALMANAGER);
	
	m_Impl.OnChangeVisualManager ();

	CView* pViewActive = GetActiveView();
	if (pViewActive->GetSafeHwnd() != NULL)
	{
		pViewActive->SendMessage(BCGM_CHANGEVISUALMANAGER, wp, lp);
	}

	return 0;
}
//**************************************************************************
void CBCGPFrameWnd::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	m_Impl.OnNcMouseMove (nHitTest, point);

	if (nHitTest == HTCAPTION && (GetStyle () & WS_MAXIMIZE) == WS_MAXIMIZE)
	{
		BOOL bIsRibbonCaption = FALSE;

	#ifndef BCGP_EXCLUDE_RIBBON
		if (m_Impl.m_pRibbonBar != NULL &&
			m_Impl.m_pRibbonBar->IsWindowVisible () &&
			m_Impl.m_pRibbonBar->IsReplaceFrameCaption ())
		{
			bIsRibbonCaption = TRUE;
		}
	#endif
		if (!bIsRibbonCaption && CBCGPVisualManager::GetInstance ()->IsOwnerDrawCaption ())
		{
			return;
		}
	}
	
	CFrameWnd::OnNcMouseMove(nHitTest, point);
}
//**************************************************************************
BCGNcHitTestType CBCGPFrameWnd::OnNcHitTest(CPoint point) 
{
	UINT nHit = m_Impl.OnNcHitTest (point);
	if (nHit != HTNOWHERE)
	{
		return nHit;
	}

	return CFrameWnd::OnNcHitTest(point);
}
//**************************************************************************
void CBCGPFrameWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	if (!m_Impl.OnNcCalcSize (bCalcValidRects, lpncsp))
	{
		CFrameWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
	}
}
//***************************************************************************
void CBCGPFrameWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_Impl.OnLButtonUp (point);
	CFrameWnd::OnLButtonUp(nFlags, point);
}
//***************************************************************************
void CBCGPFrameWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	m_Impl.OnMouseMove (point);
	CFrameWnd::OnMouseMove(nFlags, point);
}
//***************************************************************************
void CBCGPFrameWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_Impl.OnLButtonDown (point);
	CFrameWnd::OnLButtonDown(nFlags, point);
}
//***************************************************************************
LRESULT CBCGPFrameWnd::OnPostPreviewFrame(WPARAM, LPARAM)
{
	return 0;
}
//***************************************************************************
LRESULT CBCGPFrameWnd::OnDWMCompositionChanged(WPARAM,LPARAM)
{
	m_Impl.OnDWMCompositionChanged ();
	return 0;
}
//***************************************************************************
void CBCGPFrameWnd::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	BOOL bIsRibbonCaption = FALSE;

#ifndef BCGP_EXCLUDE_RIBBON
	if (m_Impl.m_pRibbonBar != NULL &&
		(m_Impl.m_pRibbonBar->IsWindowVisible () || !IsWindowVisible ()) &&
		m_Impl.m_pRibbonBar->IsReplaceFrameCaption ())
	{
		bIsRibbonCaption = TRUE;
	}
#endif

	if (!m_Impl.IsOwnerDrawCaption () || !IsWindowVisible () || bIsRibbonCaption)
	{
		CFrameWnd::OnUpdateFrameTitle(bAddToTitle);
		return;
	}

	CString strTitle1;
	GetWindowText (strTitle1);

	CFrameWnd::OnUpdateFrameTitle(bAddToTitle);

	CString strTitle2;
	GetWindowText (strTitle2);

	if (strTitle1 != strTitle2)
	{
		SendMessage (WM_NCPAINT, 0, 0);
	}
}
//***************************************************************************
LRESULT CBCGPFrameWnd::OnPowerBroadcast(WPARAM wp, LPARAM)
{
	LRESULT lres = Default ();

	if (wp == PBT_APMRESUMESUSPEND)
	{
		globalData.Resume ();
	}

	return lres;
}
//************************************************************************************
BOOL CBCGPFrameWnd::SetTaskBarProgressValue(int nCompleted, int nTotal)
{
	return globalData.TaskBar_SetProgressValue(GetSafeHwnd(), nCompleted, nTotal);
}
//************************************************************************************
BOOL CBCGPFrameWnd::SetTaskBarProgressState(BCGP_TBPFLAG tbpFlags)
{
	return globalData.TaskBar_SetProgressState(GetSafeHwnd(), tbpFlags);
}
//************************************************************************************
BOOL CBCGPFrameWnd::SetTaskBarOverlayIcon(UINT nIDResource, LPCTSTR lpcszDescr)
{
	return globalData.TaskBar_SetOverlayIcon(GetSafeHwnd(), nIDResource, lpcszDescr);
}
//************************************************************************************
BOOL CBCGPFrameWnd::SetTaskBarOverlayIcon(HICON hIcon, LPCTSTR lpcszDescr)
{
	return globalData.TaskBar_SetOverlayIcon(GetSafeHwnd(), hIcon, lpcszDescr);
}
//************************************************************************************
void CBCGPFrameWnd::ClearTaskBarOverlayIcon()
{
	globalData.TaskBar_SetOverlayIcon(GetSafeHwnd(), (HICON)NULL, NULL);
}
//************************************************************************************
BOOL CBCGPFrameWnd::CloseRibbonBackstageView()
{
#ifndef BCGP_EXCLUDE_RIBBON
	if (m_Impl.m_pBackstageView->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	m_Impl.m_pBackstageView->PostMessage(WM_CLOSE);
#endif
	return TRUE;
}
