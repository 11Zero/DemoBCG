//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPFullScreenImpl.cpp: implementation of the BCGPFullScreenImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPFullScreenImpl.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPMDIChildWnd.h"
#include "BCGPFrameWnd.h"
#include "BCGPWorkspace.h"
#include "BCGPToolBar.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPDockingControlBar.h"
#include "BCGPDropDown.h"
#include "BCGPBaseTabbedBar.h"
#include "BCGPDockBar.h"
#include "bcgprores.h"
#include "BCGPLocalResource.h"
#include "BCGPStatusBar.h"
#include "BCGPReBar.h"
#include "multimon.h"
#include "BCGPRibbonBar.h"
#include "BCGPAutoHideDockBar.h"
#include "BCGPAutoHideToolBar.h"
#include "BCGPTabbedControlBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define TASKBAR_MARGIN  2

extern CBCGPWorkspace* g_pWorkspace;

class CBCGFullScreenToolbar : public CBCGPToolBar
{
	virtual BOOL CanBeClosed () const
	{
		return FALSE;
	}
};

BOOL CBCGPFullScreenImpl::m_bUseFullScreenRegPath = FALSE;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPFullScreenImpl::CBCGPFullScreenImpl(CBCGPFrameImpl* pFrameImpl)
{
	m_pImpl				= pFrameImpl;
	m_pwndFullScreenBar = NULL;
	m_bFullScreen		= FALSE;
	m_bShowMenu			= TRUE;
	m_bMenuBarWasVisible= FALSE;
	m_bTabsArea			= TRUE;
	m_uiFullScreenID	= (UINT)-1;
	m_strRegSection		= _T("");
	m_bAutoHideTaskbar  = FALSE;
}
//*************************************************************************************
CBCGPFullScreenImpl::~CBCGPFullScreenImpl()
{

}
static BOOL ContainsSplitter (CFrameWnd* pFrame)
{
	CWnd* pWndChild = pFrame->GetWindow (GW_CHILD);
	while (pWndChild != NULL)
	{
		ASSERT_VALID (pWndChild);
		CSplitterWnd* pSplitter = DYNAMIC_DOWNCAST (CSplitterWnd, pWndChild);
		if (pSplitter != NULL)
		{
			return TRUE;
		}
		pWndChild = pWndChild->GetNextWindow ();
	}
	return FALSE;
}
//*************************************************************************************
void CBCGPFullScreenImpl::ShowFullScreen(CFrameWnd* pFrame)
{
	ASSERT (m_uiFullScreenID != -1);

	if (m_uiFullScreenID == -1)
	{
		return;
	}

	m_bMenuBarWasVisible = FALSE;
	BOOL bSplitter = FALSE;

	CBCGPMDIFrameWnd* pbcgMDIFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, pFrame);
	if (pbcgMDIFrame != NULL)
	{
		if (pbcgMDIFrame->IsPrintPreview())
		{
			return;
		}

		CBCGPMDIChildWnd* pChildWnd = DYNAMIC_DOWNCAST (CBCGPMDIChildWnd, pbcgMDIFrame->MDIGetActive ());
		if (pChildWnd != NULL)
		{
			CWnd* pViewWnd = pChildWnd->GetActiveView ();
			if (pViewWnd == NULL && !pChildWnd->IsTabbedControlBar ())
			{
				return;
			}

			bSplitter = ContainsSplitter (pChildWnd);
		}
	}
	else
	{	
		CBCGPFrameWnd* pbcgFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, pFrame);
		if (pbcgFrame == NULL)
		{
			return;
		}

		if (pbcgFrame->IsPrintPreview())
		{
			return;
		}

		bSplitter = ContainsSplitter (pbcgFrame);
	}

	CRect rectFrame, rectView, rectChild, rcScreen;

	pFrame->GetWindowRect (&rectFrame);
	m_rectFramePrev = rectFrame;

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);
	if (GetMonitorInfo (MonitorFromRect (rectFrame, MONITOR_DEFAULTTONEAREST),
		&mi))
	{
		rcScreen = mi.rcMonitor;

	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rcScreen, 0);
	}

	m_strRegSection = g_pWorkspace->GetRegistryBase (); 

	if (pbcgMDIFrame != NULL)
	{
		g_pWorkspace->SaveState(pbcgMDIFrame, m_strRegSection);
		
		if (m_bUseFullScreenRegPath)
		{
			g_pWorkspace->CleanState (_T("FullScreeenState"));	 
		}
	}
	else
	{
		CBCGPFrameWnd* pbcgFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, pFrame);
		if (pbcgFrame != NULL)
		{
			g_pWorkspace->SaveState (pbcgFrame, m_strRegSection);
			
			if (m_bUseFullScreenRegPath)
			{
				g_pWorkspace->CleanState (_T("FullScreeenState"));
			}
		}
		else
		{
			return;
		}
	}

	CBCGPMenuBar* pMenuBar = NULL;
	if (pbcgMDIFrame != NULL)
	{
		pMenuBar = DYNAMIC_DOWNCAST (CBCGPMenuBar, pbcgMDIFrame->GetMenuBar ());
	}
	else
	{
		CBCGPFrameWnd* pbcgFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, pFrame);
		if (pbcgFrame != NULL)
		{
			pMenuBar = DYNAMIC_DOWNCAST (CBCGPMenuBar, pbcgFrame->GetMenuBar ());
		}
	}

	CSize sizeMenu = CSize (0,0);
	if (pMenuBar != NULL)
	{
		m_bMenuBarWasVisible = pMenuBar->IsWindowVisible();

		if (m_bMenuBarWasVisible)
		{
			sizeMenu  = pMenuBar->CalcFixedLayout (FALSE, TRUE);
		}
	}

	BOOL bNormalizeFrame = FALSE;
	BOOL bAdjustBasicMode = FALSE;

#ifndef BCGP_EXCLUDE_RIBBON
	if (m_pImpl != NULL &&
		m_pImpl->m_pRibbonBar->GetSafeHwnd () != NULL &&
		m_pImpl->m_pRibbonBar->IsWindowVisible () && 
		globalData.DwmIsCompositionEnabled () && 
		pFrame->IsZoomed())
	{
		bNormalizeFrame = TRUE;
	}

	if (m_pImpl != NULL &&
		m_pImpl->m_pRibbonBar->GetSafeHwnd () != NULL &&
		m_pImpl->m_pRibbonBar->IsWindowVisible () && 
		!globalData.DwmIsCompositionEnabled () && 
		!pFrame->IsZoomed())
	{
		bAdjustBasicMode = TRUE;
	}
#endif

	pFrame->SetRedraw (FALSE);
	UnDockAndHideControlBars (pFrame);

	CBCGPMDIChildWnd*	pChildWnd = NULL;
	if (pbcgMDIFrame != NULL)
	{
		pChildWnd = DYNAMIC_DOWNCAST (CBCGPMDIChildWnd, pbcgMDIFrame->MDIGetActive ());
	}

	if (pbcgMDIFrame != NULL && pChildWnd != NULL)
	{
		CWnd* pViewWnd = NULL;
		if (pChildWnd->IsTabbedControlBar ()) 
		{
			pViewWnd = pChildWnd->GetTabbedControlBar ();
		}
		else
		{
			pViewWnd = pChildWnd->GetActiveView ();
		}

		if (pViewWnd == NULL)
		{
			return;
		}

		pChildWnd->GetWindowRect (&rectView);
		pViewWnd->GetWindowRect (&rectChild);

		if (bSplitter)
		{
			 pChildWnd->GetClientRect (&rectView);
			 pChildWnd->ClientToScreen (&rectView);
		}

		CRect rectClient; 
		pbcgMDIFrame->GetClientRect (&rectClient);
		pbcgMDIFrame->ClientToScreen (&rectClient);

		rectFrame.InflateRect (
			 (rectClient.left - rcScreen.left),
			 (rectClient.top - rcScreen.top) ,
			 (rcScreen.right - rectClient.right),
			 rcScreen.bottom - rectClient.bottom);
	}
	else // Maybe SDI
	{
		CBCGPFrameWnd* pbcgFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, pFrame);

		if (pbcgFrame != NULL)
		{
			CWnd* pViewWnd = pFrame->GetActiveView ();
			if (pViewWnd == NULL)
			{
				CRect rectFrmClient;
				pbcgFrame->GetClientRect (&rectFrmClient);
				pbcgFrame->ClientToScreen (&rectFrmClient);

				rectFrame.InflateRect (
						rectFrmClient.left - rcScreen.left,
						rectFrmClient.top - rcScreen.top,
						rcScreen.right - rectFrmClient.right,
						rcScreen.bottom - rectFrmClient.bottom);
			}
			else
			{
				int nMargin = bAdjustBasicMode ? GetSystemMetrics (SM_CYSIZEFRAME) : 0;

				pViewWnd->GetWindowRect (&rectView);
				pbcgFrame->GetWindowRect (&rectFrame);
				rectFrame.InflateRect (
					rectView.left - rcScreen.left,
					rectView.top - rcScreen.top,
					rcScreen.right - rectView.right,
					rcScreen.bottom - rectView.bottom + nMargin);
			}
		}
	}

	if (pbcgMDIFrame != NULL && pChildWnd == NULL)
	{
		CRect rectFrmClient;
		pbcgMDIFrame->GetClientRect (&rectFrmClient);
		pbcgMDIFrame->ClientToScreen (&rectFrmClient);

		rectFrame.InflateRect (
				rectFrmClient.left - rcScreen.left,
				rectFrmClient.top - rcScreen.top,
				rcScreen.right - rectFrmClient.right,
				rcScreen.bottom - rectFrmClient.bottom);
	}

	if (m_bAutoHideTaskbar)
	{
		AdjustFrameToAutoHideTaskbar (rectFrame);
	}

	// Remember this for OnGetMinMaxInfo()
	m_rectFullScreenWindow = rectFrame;
		
	m_pwndFullScreenBar	= new CBCGFullScreenToolbar;
	
	if (!m_pwndFullScreenBar->Create (pFrame))
	{
		TRACE0("Failed to create toolbar\n");
		return;      // fail to create
	}
	
	CString strCaption;
	CString strLabel;
	{
		CBCGPLocalResource locaRes;
		strLabel.LoadString (IDS_BCGBARRES_FULLSCREEN_CLOSE);
		strCaption.LoadString (IDS_BCGBARRES_FULLSCREEN);
	}

	CBCGPToolbarButton	button (m_uiFullScreenID, -1, strLabel, FALSE, TRUE);
	m_pwndFullScreenBar->InsertButton (button);
	m_pwndFullScreenBar->EnableDocking (0);
	m_pwndFullScreenBar->SetWindowPos (0, 100, 100, 100, 100, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	m_pwndFullScreenBar->SetWindowText (strCaption);
	m_pwndFullScreenBar->FloatControlBar (CRect(rcScreen.left + 100, rcScreen.top + 100, 200, 200));
	m_bFullScreen = TRUE;

	if (bNormalizeFrame)
	{
		pFrame->ShowWindow(SW_NORMAL);	
	}

	pFrame->SetWindowPos (NULL, rectFrame.left, rectFrame.top,
		rectFrame.Width (), rectFrame.Height (), SWP_NOZORDER);

	if (m_bShowMenu)
	{
		if (pbcgMDIFrame != NULL)
		{
			const CBCGPMenuBar* pMenuBar = pbcgMDIFrame->GetMenuBar ();

			if (pMenuBar != NULL)
			{
				if (m_bTabsArea)
				{
					if (pMenuBar->CanFloat ())
					{
						pbcgMDIFrame->DockControlBar ((CBCGPBaseControlBar*)pMenuBar);
					}
					pbcgMDIFrame->ShowControlBar ((CBCGPBaseControlBar*)pMenuBar, TRUE,FALSE, FALSE);
				}
				else
				{
					if (pMenuBar->CanFloat ())
					{
						pbcgMDIFrame->DockControlBar ((CBCGPBaseControlBar*)pMenuBar);
						((CBCGPBaseControlBar*)pMenuBar)->FloatControlBar (CRect(300, 200, 500, 500));
					}
					pbcgMDIFrame->ShowControlBar ((CBCGPBaseControlBar*)pMenuBar, TRUE,FALSE, FALSE);
				}
			}	
		}
		else
		{
			CBCGPFrameWnd* pbcgFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, pFrame);
			if (pbcgFrame != NULL)
			{
				const CBCGPMenuBar* pMenuBar = pbcgFrame->GetMenuBar ();

				if (pMenuBar != NULL)
				{
					if (pMenuBar->CanFloat ())
					{
						pbcgFrame->DockControlBar ((CBCGPBaseControlBar*)pMenuBar);
					}
					pbcgFrame->ShowControlBar ((CBCGPBaseControlBar*)pMenuBar, TRUE,FALSE, FALSE);
				}	
			}
		}
	}

	pFrame->SetRedraw (TRUE);
	pFrame->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | 
							RDW_ERASE | RDW_ALLCHILDREN);

	if (pbcgMDIFrame != NULL && pbcgMDIFrame->IsMDITabbedGroup ())
	{
		pbcgMDIFrame->AdjustClientArea ();
	}
}
//*************************************************************************************
void CBCGPFullScreenImpl::RestoreState(CFrameWnd* pFrame) 
{
	ASSERT (m_uiFullScreenID != -1);

	if (m_uiFullScreenID == -1)
	{
		return;
	}

	// Destroy the toolbar
	CWnd*	pWnd	= m_pwndFullScreenBar->GetParentMiniFrame ();
	if (pWnd == NULL)
		pWnd	= m_pwndFullScreenBar;
	VERIFY (pWnd->DestroyWindow());

	delete m_pwndFullScreenBar; 
		
	m_pwndFullScreenBar = NULL;
	m_bFullScreen = FALSE;

	CBCGPMDIFrameWnd* pbcgMDIFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, pFrame);
		
	//Restore window pos
	pFrame->SetWindowPos (NULL, m_rectFramePrev.left, m_rectFramePrev.top,
		m_rectFramePrev.Width (), m_rectFramePrev.Height (), SWP_NOZORDER);

	//restore layout
	pFrame->SetRedraw (FALSE);

	g_pWorkspace->m_bLoadUserToolbars = FALSE;

	if (pbcgMDIFrame != NULL)
	{
		g_pWorkspace->LoadState (pbcgMDIFrame, m_strRegSection);
	}
	else
	{
		CBCGPFrameWnd* pbcgFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, pFrame);
		if (pbcgFrame != NULL)
		{
			g_pWorkspace->LoadState (pbcgFrame, m_strRegSection);
		}
	}

	pFrame->SetRedraw (TRUE);
	pFrame->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | 
								RDW_ERASE | RDW_ALLCHILDREN);

	if (!m_bShowMenu && m_bMenuBarWasVisible)
	{
		if (pbcgMDIFrame != NULL)
		{
			const CBCGPMenuBar* pMenuBar = pbcgMDIFrame->GetMenuBar ();

			if (pMenuBar != NULL)
			{
				pbcgMDIFrame->ShowControlBar ((CBCGPBaseControlBar*)pMenuBar, TRUE,FALSE, FALSE);
				if (pMenuBar->IsFloating ())
				{
					pbcgMDIFrame->SetFocus ();
				}
			}
		}
		else
		{
			CBCGPFrameWnd* pbcgFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, pFrame);
			if (pbcgFrame != NULL)
			{
				const CBCGPMenuBar* pMenuBar = pbcgFrame->GetMenuBar ();
				if (pMenuBar != NULL)
				{
					pbcgFrame->ShowControlBar ((CBCGPBaseControlBar*)pMenuBar, TRUE,FALSE, FALSE);
					if (pMenuBar->IsFloating ())
					{
						pbcgFrame->SetFocus ();
					}
				}
			}
		}	
	}

	if (m_pImpl != NULL)
	{
		CBCGPMenuBar* pMenuBar = m_pImpl->GetMenuBar();
		if (pMenuBar != NULL && pMenuBar->IsPopupMode())
		{
			pMenuBar->ShowControlBar (FALSE, FALSE, FALSE);
		}
	}

	if (pbcgMDIFrame != NULL && pbcgMDIFrame->IsMDITabbedGroup ())
	{
		pbcgMDIFrame->AdjustClientArea ();
	}

	BOOL bDontCallChangeVM = FALSE;

#ifndef BCGP_EXCLUDE_RIBBON
	if (m_pImpl != NULL && 
		m_pImpl->m_pRibbonBar->GetSafeHwnd () != NULL)
	{
		if (m_pImpl->m_pRibbonBar->IsWindowVisible () &&
			m_pImpl->m_pRibbonBar->IsReplaceFrameCaption ())
		{
			m_pImpl->OnChangeVisualManager ();
			m_pImpl->m_pRibbonBar->DWMCompositionChanged ();
			
			bDontCallChangeVM = TRUE;
		}

		if (!m_pImpl->m_pRibbonBar->IsWindowVisible () &&
			pFrame->IsZoomed())
		{
			pFrame->SetWindowPos(NULL, -1, -1, -1, -1, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
		}
	}
#endif

	if (m_pImpl != NULL && !bDontCallChangeVM)
	{
		m_pImpl->OnChangeVisualManager ();
	}
}
//*************************************************************************************
void CBCGPFullScreenImpl::ShowFullScreen()
{
	if (!m_bFullScreen)
	{
		ShowFullScreen (m_pImpl->m_pFrame);
	}
	else
	{
		RestoreState (m_pImpl->m_pFrame);
	}
}
//*************************************************************************************
void CBCGPFullScreenImpl::ProccessTabbedBar (CBCGPTabbedControlBar* pTabbedBar)
{
	int nTabsNum = pTabbedBar->GetTabsNum ();

	for (int i = 0; i < nTabsNum; i++)
	{
		CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST(CBCGPDockingControlBar, pTabbedBar->FindBarByTabNumber (i, TRUE));
		ASSERT_VALID (pBar);	
		if (pBar->HideInFullScreenMode ())
		{
			pBar->ShowControlBar (FALSE,FALSE, FALSE);
		}
	}
}
//*************************************************************************************
BOOL CBCGPFullScreenImpl::ShouldProccessTabbedBar (CBCGPTabbedControlBar* pTabbedBar)
{
	int nTabsNum = pTabbedBar->GetTabsNum ();

	for (int i = 0; i < nTabsNum; i++)
	{
		CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST(CBCGPDockingControlBar, pTabbedBar->FindBarByTabNumber (i, TRUE));
		ASSERT_VALID (pBar);	
		if (!pBar->HideInFullScreenMode ())
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*************************************************************************************
void CBCGPFullScreenImpl::UnDockAndHideControlBars(CFrameWnd* pFrame)
{
	CBCGPMDIFrameWnd* pbcgMDIFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, pFrame);
	CBCGPFrameWnd* pbcgFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, pFrame);

	CBCGPDockManager* pDockMgr = m_pImpl->m_pDockManager;

	if	(pDockMgr == NULL)
	{
		return;
	}

	CObList list;
	pDockMgr->GetControlBarList (list, TRUE);

	//------------------------------------
	// UnDock and hide DockingControlBars
	//-------------------------------------

	POSITION pos;
	for (pos = list.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pBarNext = (CBCGPBaseControlBar*) list.GetNext (pos);

		if (!::IsWindow(pBarNext->m_hWnd))
		{
			continue;
		}

		CBCGPAutoHideDockBar* pAutoDockBar = DYNAMIC_DOWNCAST(CBCGPAutoHideDockBar, pBarNext);
		if (pAutoDockBar != NULL)
		{
			continue;
		}

		CBCGPAutoHideToolBar* pAutoToolBar = DYNAMIC_DOWNCAST(CBCGPAutoHideToolBar, pBarNext);
		if (pAutoToolBar != NULL)
		{
			continue;
		}

		CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST(CBCGPDockingControlBar, pBarNext);
		if (pBar != NULL) 
		{
			if (pBar->IsMDITabbed ())
			{
				continue;
			}

			CBCGPTabbedControlBar* pTabbedBar = DYNAMIC_DOWNCAST(CBCGPTabbedControlBar, pBarNext);
			if (pTabbedBar != NULL)
			{
				if (ShouldProccessTabbedBar (pTabbedBar))
				{
					ProccessTabbedBar (pTabbedBar);
					continue;	
				}
			}

			if (!pBar->HideInFullScreenMode ())
			{
				continue;
			}

			if (pBar->IsAutoHideMode ())
			{
				pBar->SetAutoHideMode (FALSE, CBRS_ALIGN_ANY);
			}

			if (pBar->IsTabbed ())
			{
				CBCGPBaseTabWnd* pTabWnd = (CBCGPBaseTabWnd*) pBar->GetParent ();
				CBCGPBaseTabbedBar* pTabBar = (CBCGPBaseTabbedBar*) pTabWnd->GetParent ();
				pTabBar->DetachControlBar (pBar);
			}

			if(pBar->CanFloat())
			{
				pBar->FloatControlBar (CRect (300, 200, 500, 500), BCGP_DM_UNKNOWN, FALSE);
			}

			if (pbcgMDIFrame != NULL)
			{
				pbcgMDIFrame->ShowControlBar (pBar, FALSE,FALSE, FALSE);

			}
			else if (pbcgFrame != NULL)
			{
				pbcgFrame->ShowControlBar (pBar, FALSE,FALSE, FALSE);
			}

		}else
		{
			CBCGPControlBar* pControlBar =  DYNAMIC_DOWNCAST (CBCGPControlBar, pBarNext);

			if (pControlBar != NULL)
			{
				if (pbcgMDIFrame != NULL)
				{
					pbcgMDIFrame->ShowControlBar (pControlBar, FALSE,FALSE, FALSE);
				}
				else if (pbcgFrame != NULL)
				{
					pbcgFrame->ShowControlBar (pControlBar, FALSE,FALSE, FALSE);
				}

				continue;
			}
		}
	}

	//------------------------------------
	// UnDock and hide all Toolbars
	//-------------------------------------
	const CObList& gAllToolbars = CBCGPToolBar::GetAllToolbars ();
	
	for (pos = gAllToolbars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (pos);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			if (pToolBar->GetParentFrame()->GetSafeHwnd() != pFrame->GetSafeHwnd())
			{
				continue;
			}

			if (!pToolBar->HideInFullScreenMode ())
			{
				continue;
			}

			//------------------------------
			// Don't touch dropdown toolbars!
			//------------------------------
			if (!pToolBar->IsKindOf (RUNTIME_CLASS (CBCGPDropDownToolBar)))
			{
				if (pToolBar->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar)))
				{
					if(pToolBar->CanFloat ())
					{
						pToolBar->FloatControlBar (CRect (0, -1024, 0, -1024));

						if (pbcgMDIFrame != NULL)
						{
							pbcgMDIFrame->ShowControlBar (pToolBar, FALSE,FALSE, FALSE);
						}
						else if (pbcgFrame != NULL)
						{
							pbcgFrame->ShowControlBar (pToolBar, FALSE,FALSE, FALSE);
						}
					}
					continue;
				}

				//------------------------------------------------------
				// Don't touch toolbars resids on the DockingControlBars
				//------------------------------------------------------

				CWnd* pWnd = pToolBar->GetParent ();
				if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPDockBar)) ||
				   pWnd->IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd)))
				{
					if (pbcgMDIFrame != NULL)
					{
						if (pToolBar->CanFloat ())
						{
							pToolBar->FloatControlBar (CRect (300, 200, 500, 500));
						}

						pbcgMDIFrame->ShowControlBar (pToolBar, FALSE,FALSE, FALSE);
					}
					else if (pbcgFrame != NULL)
					{
					
						if (pToolBar->CanFloat ())
						{
							pToolBar->FloatControlBar (CRect (300, 200, 500, 500));
						}
						pbcgFrame->ShowControlBar (pToolBar, FALSE,FALSE, FALSE);
					}
				}
			}
		}
	}
}
//*************************************************************************************
void CBCGPFullScreenImpl::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	if (m_bFullScreen)
	{
		lpMMI->ptMaxSize.x		=
		lpMMI->ptMaxTrackSize.x	= m_rectFullScreenWindow.Width ();

		lpMMI->ptMaxSize.y		=
		lpMMI->ptMaxTrackSize.y	= m_rectFullScreenWindow.Height ();
	}
}
//**************************************************************************************
void CBCGPFullScreenImpl::AdjustFrameToAutoHideTaskbar (CRect& rect)
{
	if (!m_bAutoHideTaskbar)
	{
		return;
	}

	APPBARDATA abd;
	ZeroMemory (&abd, sizeof (APPBARDATA));
	abd.cbSize = sizeof (APPBARDATA);

	abd.uEdge = ABE_BOTTOM;
	if (SHAppBarMessage (ABM_GETAUTOHIDEBAR, &abd))
	{
		UINT uState = (UINT) SHAppBarMessage(ABM_GETSTATE, &abd);
		if (uState == ABS_AUTOHIDE)
		{
			rect.bottom -= TASKBAR_MARGIN;
		}
	}

	abd.uEdge = ABE_TOP;
	if (SHAppBarMessage (ABM_GETAUTOHIDEBAR, &abd))
	{
		UINT uState = (UINT) SHAppBarMessage(ABM_GETSTATE, &abd);
		if (uState == ABS_AUTOHIDE)
		{
			rect.top += TASKBAR_MARGIN;
		}
	}

	abd.uEdge = ABE_LEFT;
	if (SHAppBarMessage (ABM_GETAUTOHIDEBAR, &abd))
	{
		UINT uState = (UINT) SHAppBarMessage(ABM_GETSTATE, &abd);
		if (uState == ABS_AUTOHIDE)
		{
			rect.left += TASKBAR_MARGIN;
		}
	}

	abd.uEdge = ABE_RIGHT;
	if (SHAppBarMessage (ABM_GETAUTOHIDEBAR, &abd))
	{
		UINT uState = (UINT) SHAppBarMessage(ABM_GETSTATE, &abd);
		if (uState == ABS_AUTOHIDE)
		{
			rect.right -= TASKBAR_MARGIN;
		}
	}
}