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
// BCGPMultiMiniFrameWnd.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPMultiMiniFrameWnd.h"
#include "BCGPGlobalUtils.h"
#include "BCGPDockManager.h"
#include "BCGPDockingControlBar.h"
#include "BCGPSlider.h"

#include "BCGPBaseTabbedBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CBCGPMultiMiniFrameWnd,CBCGPMiniFrameWnd,VERSIONABLE_SCHEMA | 2)

/////////////////////////////////////////////////////////////////////////////
// CBCGPMultiMiniFrameWnd

CBCGPMultiMiniFrameWnd::CBCGPMultiMiniFrameWnd()
{
	m_hWndLastFocused = NULL;	
	m_bHostsToolbar = FALSE;
}

CBCGPMultiMiniFrameWnd::~CBCGPMultiMiniFrameWnd()
{
}


BEGIN_MESSAGE_MAP(CBCGPMultiMiniFrameWnd, CBCGPMiniFrameWnd)
	//{{AFX_MSG_MAP(CBCGPMultiMiniFrameWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_GETMINMAXINFO()
	ON_WM_LBUTTONDBLCLK()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(BCGPM_CHECKEMPTYMINIFRAME, OnCheckEmptyState)
	ON_REGISTERED_MESSAGE(BCGM_CHANGEVISUALMANAGER, OnChangeVisualManager)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPMultiMiniFrameWnd message handlers

int CBCGPMultiMiniFrameWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPMiniFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_barContainerManager.Create (this, NULL);
	
	return 0;
}

//********************************************************************************
// Should return TRUE if no docking occures!!!
//********************************************************************************
BOOL CBCGPMultiMiniFrameWnd::DockFrame (CBCGPMiniFrameWnd* pDockedFrame, 
										BCGP_DOCK_METHOD dockMethod)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDockedFrame);

	CBCGPMultiMiniFrameWnd* pMultiDockedFrame = 
		DYNAMIC_DOWNCAST (CBCGPMultiMiniFrameWnd, pDockedFrame);

	if (pMultiDockedFrame == NULL )
	{
		// we can dock only multi mini frame windows! 
		//(their dock bars have CBRS_FLOAT_MULTI style)
		return TRUE;
	}

	CBCGPBaseControlBar* pFirstBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar,
		pMultiDockedFrame->GetFirstVisibleBar ());
	if (pFirstBar == NULL || pFirstBar->GetEnabledAlignment () == 0)
	{
		return TRUE;
	}

	CBCGPDockingControlBar* pTargetControlBar = NULL;
	BOOL bTabArea = FALSE;
	BOOL bCaption = FALSE;
	DWORD dwAlignment = CBRS_ALIGN_LEFT;

	CPoint pt;
	GetCursorPos (&pt);

	if (dockMethod == BCGP_DM_MOUSE || dockMethod == BCGP_DM_STANDARD)
	{
		CBCGPGlobalUtils globalUtils;
		if (!globalUtils.GetCBAndAlignFromPoint (m_barContainerManager, pt, 
									&pTargetControlBar, dwAlignment, bTabArea, bCaption))
		{
			return TRUE;
		}
	}

	if (pTargetControlBar == NULL || dwAlignment == 0)
	{
		return TRUE;
	}

	CBCGPBarContainerManager& barManager = pMultiDockedFrame->GetContainerManager ();
	CWnd* pFirstDockedBar = barManager.GetFirstVisibleBar ();

	if ((bTabArea || bCaption) && pTargetControlBar != NULL)  
		
	{
		// if the first bar is a tabbed bar it will be destroyed when adding to tab wnd
		// we need to take one of its tabs
		CBCGPBaseTabbedBar* pTabbedBar = 
			DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pFirstDockedBar);

		if (pTabbedBar != NULL)
		{
			int iTabNum = -1; 
			pFirstDockedBar = pTabbedBar->GetFirstVisibleTab (iTabNum);
		}

		if (!m_barContainerManager.AddContainerManagerToTabWnd (pTargetControlBar, barManager))
		{
			return TRUE;
		}
	}
	else
	{
		if (!m_barContainerManager.AddContainerManager (pTargetControlBar, dwAlignment, 
														barManager, TRUE))
		{	
			return TRUE;
		}
	}

	HWND hDockedFrame = pDockedFrame->m_hWnd;
	pMultiDockedFrame->SendMessage (BCGPM_CHECKEMPTYMINIFRAME);
	if (IsWindow (hDockedFrame))
	{
		pMultiDockedFrame->MoveWindow (pMultiDockedFrame->GetRecentFloatingRect ());
	}

	OnBarRecalcLayout ();

	if (dockMethod == BCGP_DM_MOUSE && pFirstDockedBar != NULL)
	{
		pFirstDockedBar->ScreenToClient (&pt);
		if (pFirstDockedBar->IsKindOf (RUNTIME_CLASS (CBCGPControlBar)))
		{
			((CBCGPControlBar*) pFirstDockedBar)->EnterDragMode (TRUE);
		}
		else
		{
			pFirstDockedBar->SendMessage (WM_LBUTTONDOWN, 0, MAKELPARAM (pt.x, pt.y));
		}
		
	}

	OnSetRollUpTimer ();

	return FALSE;
}
//********************************************************************************
BOOL CBCGPMultiMiniFrameWnd::DockBar (CBCGPDockingControlBar* pDockedBar)
{
	CPoint pt;
	GetCursorPos (&pt);

	BOOL bTabArea = FALSE;
	BOOL bCaption = FALSE;
	CBCGPDockingControlBar* pTargetControlBar = NULL;
	DWORD dwAlignment = 0;

	CBCGPGlobalUtils globalUtils;
	if (!globalUtils.GetCBAndAlignFromPoint (m_barContainerManager, pt, 
									&pTargetControlBar, dwAlignment, bTabArea, bCaption))
	{
		return TRUE;
	}

	if (pTargetControlBar == NULL || dwAlignment == 0)
	{
		return TRUE;
	}

	pDockedBar->UnDockControlBar (FALSE);

	pDockedBar->SetParent (this);
	BOOL bResult = m_barContainerManager.InsertControlBar (pDockedBar, 
		pTargetControlBar, dwAlignment);

	if (!bResult)
	{
		ASSERT (FALSE);
	}

	if (bResult)
	{
		AddRemoveBarFromGlobalList (pDockedBar, TRUE);
		CheckGripperVisibility ();
		OnBarRecalcLayout ();
		SendMessage (WM_NCPAINT);
	}

	OnSetRollUpTimer ();

	if (pDockedBar->CanFocus ())
	{
		pDockedBar->SetFocus ();
	}

	OnBarRecalcLayout ();

	return !bResult;
}
//********************************************************************************
CBCGPDockingControlBar* CBCGPMultiMiniFrameWnd::DockControlBarStandard (BOOL& bWasDocked)
{
	if (!OnBeforeDock ())
	{
		return NULL;
	}

	CObList lstBars;
	m_barContainerManager.AddControlBarsToList (&lstBars, NULL);
	CList<HWND, HWND> lstBarsHwnd;

	for (POSITION pos = lstBars.GetHeadPosition (); pos != NULL;)
	{
		CWnd* pWnd = DYNAMIC_DOWNCAST (CWnd, lstBars.GetNext (pos));
		if (pWnd != NULL)
		{
			lstBarsHwnd.AddTail (pWnd->GetSafeHwnd ());
		}
	}

	CBCGPBaseControlBar* pTargetControlBar = m_dragFrameImpl.m_pFinalTargetBar;
	BCGP_PREDOCK_STATE state = m_dragFrameImpl.m_bDockToTab ? BCGP_PDS_DOCK_TO_TAB : BCGP_PDS_DOCK_REGULAR;
	
	CBCGPMiniFrameWnd* pParentMiniFrame = NULL; 

	if (pTargetControlBar != NULL)
	{
		pParentMiniFrame = pTargetControlBar->GetParentMiniFrame ();
	}

	CWnd* pFocusWnd = GetFocus ();

	if (pParentMiniFrame == NULL)
	{
		bWasDocked = !SetPreDockState (state, pTargetControlBar, BCGP_DM_STANDARD);
	}
	else
	{
		CBCGPMultiMiniFrameWnd* pParentMultiMiniFrame = 
			DYNAMIC_DOWNCAST (CBCGPMultiMiniFrameWnd, pParentMiniFrame);
		if (pParentMultiMiniFrame != NULL && 
			pParentMultiMiniFrame != this)
		{
			bWasDocked = !pParentMultiMiniFrame->DockFrame (this, BCGP_DM_STANDARD);
		}
	}

	if (pFocusWnd != NULL && ::IsWindow (pFocusWnd->GetSafeHwnd ()))
	{
		pFocusWnd->SetFocus ();
	}

	if (bWasDocked)
	{
		for (POSITION pos = lstBarsHwnd.GetHeadPosition (); pos != NULL;)
		{
			HWND hwnd = lstBarsHwnd.GetNext (pos);
			CBCGPDockingControlBar* pNextBar = 
				DYNAMIC_DOWNCAST (CBCGPDockingControlBar, CWnd::FromHandle (hwnd));
			if (pNextBar != NULL)
			{
				pNextBar->OnAfterDockFromMiniFrame ();
			}
		}
	}

	return NULL;
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::OnSetRollUpTimer ()
{
	CObList lstControlBars; 
	m_barContainerManager.AddControlBarsToList (&lstControlBars, NULL);	

	for (POSITION pos = lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pNextBar = 
			DYNAMIC_DOWNCAST (CBCGPBaseControlBar, lstControlBars.GetNext (pos));
		ASSERT_VALID (pNextBar);

		if (pNextBar != NULL && pNextBar->GetBCGStyle () & CBRS_BCGP_AUTO_ROLLUP)
		{
			SetRollUpTimer ();	
			break;
		}
	}
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::OnKillRollUpTimer ()
{
	CObList lstControlBars; 
	m_barContainerManager.AddControlBarsToList (&lstControlBars, NULL);

	BOOL bThereIsRollupState = FALSE;
	for (POSITION pos = lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pNextBar = 
			DYNAMIC_DOWNCAST (CBCGPBaseControlBar, lstControlBars.GetNext (pos));
		ASSERT_VALID (pNextBar);

		if (pNextBar != NULL && pNextBar->GetBCGStyle () & CBRS_BCGP_AUTO_ROLLUP)
		{
			bThereIsRollupState = TRUE;
			break;
		}
	}

	if (!bThereIsRollupState)
	{
		KillRollupTimer ();
	}
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::AdjustBarFrames ()
{
	CObList& lstControlBars = m_barContainerManager.m_lstControlBars;
	UINT uiSWPFlags =	SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | 
						SWP_NOACTIVATE | SWP_FRAMECHANGED;
	for (POSITION pos = lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pNextBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstControlBars.GetNext (pos));
		ASSERT_VALID (pNextBar);

		pNextBar->SetWindowPos (NULL, -1, -1, -1, -1, uiSWPFlags);
	}	
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::AddControlBar (CBCGPBaseControlBar* pWnd)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pWnd);
	ASSERT_KINDOF (CBCGPDockingControlBar, pWnd);

	CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pWnd);

	if (m_barContainerManager.IsEmpty ())
	{
		m_barContainerManager.AddControlBar (pBar);
		CBCGPMiniFrameWnd::AddControlBar (pWnd);
	}

	OnSetRollUpTimer ();
}
//********************************************************************************
BOOL CBCGPMultiMiniFrameWnd::AddRecentControlBar (CBCGPDockingControlBar* pBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);
	
	CBCGPBarContainer* pRecentContainer = pBar->m_recentDockInfo.GetRecentContainer (FALSE);
	CBCGPBarContainer* pRecentTabContainer = pBar->m_recentDockInfo.GetRecentTabContainer (FALSE);
	if (pRecentContainer != NULL)
	{
		pBar->SetParent (this);
		AddRemoveBarFromGlobalList (pBar, TRUE);
		CBCGPDockingControlBar* pAddedBar = m_barContainerManager.AddControlBarToRecentContainer (pBar, pRecentContainer);

		CWnd* pEmbeddedWnd = CWnd::FromHandlePermanent (m_hEmbeddedBar);

		if (pAddedBar != NULL && pEmbeddedWnd == NULL)
		{
			m_hEmbeddedBar = pAddedBar->GetSafeHwnd ();
		}
		if (m_barContainerManager.GetVisibleBarCount () == 1 && 
			pBar == pAddedBar)
		{
			MoveWindow (m_rectRecentFloatingRect);
		}
		if (pAddedBar != NULL)
		{
			OnShowControlBar (pAddedBar, TRUE);
		}
	}
	else if (pRecentTabContainer != NULL)
	{
		pBar->SetParent (this);
		AddRemoveBarFromGlobalList (pBar, TRUE);
		BOOL bRecentLeftBar = pBar->m_recentDockInfo.IsRecentLeftBar (FALSE);
		CBCGPDockingControlBar* pTabbedBar = (CBCGPDockingControlBar*) (bRecentLeftBar ? 
			pRecentTabContainer->GetLeftBar () : pRecentTabContainer->GetRightBar ());
		if (pTabbedBar != NULL)
		{
			CBCGPDockingControlBar* pCreatedTabbedBar = NULL;
			pBar->AttachToTabWnd (pTabbedBar, BCGP_DM_DBL_CLICK, TRUE, &pCreatedTabbedBar);

			pTabbedBar->ShowControlBar (TRUE, FALSE, TRUE);
			OnBarRecalcLayout ();		
		}
		else
		{
			CBCGPDockingControlBar* pAddedBar = m_barContainerManager.AddControlBarToRecentContainer (pBar, pRecentTabContainer);
			OnShowControlBar (pAddedBar, TRUE);
		}
	}
	else
	{
		ASSERT (FALSE);
		return FALSE;
	}
	OnSetRollUpTimer ();
	return TRUE;
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	OnBarRecalcLayout ();

	ArrangeCaptionButtons ();
	SendMessage (WM_NCPAINT);
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::OnSizing(UINT fwSide, LPRECT pRect) 
{
	CWnd::OnSizing(fwSide, pRect);
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::OnBarRecalcLayout ()
{
	ASSERT_VALID (this);

	CRect rectClient;
	GetClientRect (rectClient);
	HDWP hdwp = ::BeginDeferWindowPos (20);
	m_barContainerManager.ResizeBarContainers (rectClient, hdwp);
	EndDeferWindowPos (hdwp);	

	if (CBCGPControlBar::m_bHandleMinSize)
	{
		CRect rectContainer; 
		m_barContainerManager.GetWindowRect (rectContainer);
		
		CRect rectWnd;
		GetWindowRect (rectWnd);

		int nDeltaWidth = rectContainer.Width () - rectClient.Width ();
		int nDeltaHeight = rectContainer.Height () - rectClient.Height ();

		if (nDeltaWidth < 0)
		{
			nDeltaWidth = 0;
		}

		if (nDeltaHeight < 0)
		{
			nDeltaHeight = 0;
		}

		if (nDeltaWidth != 0 || nDeltaHeight != 0)
		{
			SetWindowPos (NULL, -1, -1, rectWnd.Width () + nDeltaWidth, rectWnd.Height () + nDeltaHeight, 
				SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}
	}
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::RemoveControlBar (CBCGPBaseControlBar* pBar, BOOL bDestroy,
											   BOOL /*bNoDelayedDestroy*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)))
	{
		m_barContainerManager.RemoveControlBarFromContainer ((CBCGPDockingControlBar*) pBar);
		if (!m_barContainerManager.IsEmpty ())
		{
			CBCGPMiniFrameWnd::ReplaceControlBar (pBar, m_barContainerManager.GetFirstBar ());
		}
		else
		{
			// do not destroy the miniframe in the base class
			CBCGPMiniFrameWnd::RemoveControlBar (pBar, FALSE);

			// if embedded bar has became NULL set it to the first bar in the container
			CWnd* pEmbeddedWnd = CWnd::FromHandlePermanent (m_hEmbeddedBar);
			if (pEmbeddedWnd == NULL)
			{
				m_hEmbeddedBar = m_barContainerManager.GetFirstBar ()->GetSafeHwnd ();
			}
		}
	}
	
	if (bDestroy && GetControlBarCount () == 0)
	{
		PostMessage (BCGPM_CHECKEMPTYMINIFRAME);
	}
	else 
	{
		pBar->EnableGripper (TRUE);
		OnBarRecalcLayout ();
		SendMessage (WM_NCPAINT);
	}

	OnKillRollUpTimer ();
}
//********************************************************************************
BOOL CBCGPMultiMiniFrameWnd::SetPreDockState (BCGP_PREDOCK_STATE preDockState, 
											  CBCGPBaseControlBar* pBarToDock, 
											  BCGP_DOCK_METHOD dockMethod)
{
	ASSERT_VALID (this);

	if (preDockState == BCGP_PDS_NOTHING || 
		preDockState == BCGP_PDS_DOCK_TO_TAB && 
		pBarToDock != NULL && 
		!pBarToDock->CanBeAttached ())
	{
		return TRUE;
	}


	CBCGPDockingControlBar* pTargetDockBar = 
		DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pBarToDock);


	CBCGPDockManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : globalUtils.GetDockManager (GetParent ());
	if (pDockManager == NULL)
	{
		ASSERT (FALSE);
		return TRUE;
	}

	CWnd* pFirstDockedBar = m_barContainerManager.GetFirstVisibleBar ();

	// determine dock alignment and edge
	CPoint pt;
	GetCursorPos (&pt);

	DWORD dwAlignment = 0;

	CObList lstControlBars; 
	m_barContainerManager.AddControlBarsToList (&lstControlBars, NULL);

	CList<HWND, HWND> lstHandles;

	POSITION pos = NULL;

	for (pos = lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pNextBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstControlBars.GetNext (pos));
		ASSERT_VALID (pNextBar);
		
		pNextBar->StoreRecentDockInfo ();
		lstHandles.AddTail (pNextBar->GetSafeHwnd ());
	}
	
	if (pTargetDockBar != NULL)
	{
		CBCGPBaseControlBar* pHeadBar = m_barContainerManager.GetFirstBar ();
		
		if (pHeadBar == NULL || !pTargetDockBar->CanAcceptBar (pHeadBar) || 
			!pHeadBar->CanAcceptBar (pTargetDockBar))
		{
			return TRUE;
		}

		if (!pHeadBar->IsBarVisible () && m_barContainerManager.GetBarCount () == 1 && 
			(pHeadBar->GetDockMode () & BCGP_DT_STANDARD) != 0)
		{
			// the head bar is unvisible and there is only one bar in container manager
			// means that this bar was torn off from the tab window and its parent miniframe
			// is hidden
			pHeadBar->ModifyStyle (0, WS_VISIBLE);
		}


		BOOL bOuterEdge = FALSE;
		if (preDockState == BCGP_PDS_DOCK_REGULAR && 
			!globalUtils.CheckAlignment (pt, pTargetDockBar, 
										 CBCGPDockManager::m_nDockSencitivity, NULL,
										 bOuterEdge, dwAlignment))
		{
			// unable for some reason to determine alignment
			return TRUE;
		}

		if (preDockState == BCGP_PDS_DOCK_REGULAR)
		{
			if (!pTargetDockBar->DockContainer (m_barContainerManager, dwAlignment, dockMethod))
			{
				return TRUE;
			}
		}
		else if (preDockState == BCGP_PDS_DOCK_TO_TAB)
		{
			for (pos = lstControlBars.GetHeadPosition (); pos != NULL;)
			{
				CBCGPDockingControlBar* pNextBar = 
					DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstControlBars.GetNext (pos));
				ASSERT_VALID (pNextBar);
				
				AddRemoveBarFromGlobalList (pNextBar, FALSE);
				pNextBar->AttachToTabWnd (pTargetDockBar, dockMethod);
			}

			ShowWindow (SW_HIDE);
			MoveWindow (GetRecentFloatingRect ());
			CBCGPMiniFrameWnd::OnCancelMode(); 
			SendMessage (BCGPM_CHECKEMPTYMINIFRAME);
			return TRUE;
		}
		else
		{
			return TRUE;
		}
	}
	else // dock to frame window - need to create a new default slider
	{
		BOOL bOuterEdge = FALSE;
		if (!pDockManager->IsPointNearDockBar (pt, dwAlignment, bOuterEdge))
		{
			return TRUE;
		}

		CBCGPSlider* pSlider = 
			CBCGPDockingControlBar::CreateDefaultSlider (dwAlignment, GetParent ());

		if (pSlider == NULL)
		{
			return TRUE;
		}

		pSlider->SetBarAlignment (dwAlignment);

		CRect rectContainer;
		m_barContainerManager.GetWindowRect (rectContainer);
		pDockManager->AdjustRectToClientArea (rectContainer, dwAlignment);
		HDWP hdwp = NULL;
		m_barContainerManager.ResizeBarContainers (rectContainer, hdwp);

		if (bOuterEdge)
		{
			// register slider with the dock manager
			pDockManager->AddControlBar (pSlider, !bOuterEdge, FALSE, bOuterEdge);
			pSlider->AddContainer (m_barContainerManager, bOuterEdge);
		}
		else
		{
			pSlider->AddContainer (m_barContainerManager, FALSE);
			pDockManager->AddControlBar (pSlider);
		}
	}

	// FINALLY destroy the frame - 
	// all its bars should have been docked (and therefore removed)
	HWND hwndSave = m_hWnd;
	SendMessage (BCGPM_CHECKEMPTYMINIFRAME);

	if (IsWindow (hwndSave))
	{
		MoveWindow (m_rectRecentFloatingRect);
	}
	else
	{
		return FALSE;
	}

	if (pFirstDockedBar != NULL && dockMethod == BCGP_DM_MOUSE)
	{
		pFirstDockedBar->ScreenToClient (&pt);
		if (pFirstDockedBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)))
		{
			((CBCGPDockingControlBar*)pFirstDockedBar)->EnterDragMode (FALSE);
		}
		else
		{
			pFirstDockedBar->SendMessage (WM_LBUTTONDOWN, 0, MAKELPARAM (pt.x, pt.y));
		}
	}

	// adjust the docking layout
	if (pTargetDockBar != NULL)
	{
		pTargetDockBar->RecalcLayout ();
	}
	else if (pFirstDockedBar != NULL)
	{
		CBCGPDockingControlBar* pDockingBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pFirstDockedBar);
		if (pDockingBar != NULL)
		{
			pDockingBar->AdjustDockingLayout ();
		}
	}

	OnSetRollUpTimer ();

	for (pos = lstHandles.GetHeadPosition (); pos != NULL;)
	{
		HWND hwndNext = lstHandles.GetNext (pos);
		if (::IsWindow (hwndNext))
		{
			CBCGPDockingControlBar* pNextBar = 
				DYNAMIC_DOWNCAST (CBCGPDockingControlBar, CWnd::FromHandle (hwndNext));
			if (pNextBar != NULL)
			{
				pNextBar->OnAfterDockFromMiniFrame ();
			}
		}
	}

	return FALSE;
}
//*************************************************************************************
BOOL CBCGPMultiMiniFrameWnd::SaveState (LPCTSTR lpszProfileName, UINT uiID)
{
	ASSERT_VALID (this);
	CObList& lstControlBars = m_barContainerManager.m_lstControlBars;
	for (POSITION pos = lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pNextBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstControlBars.GetNext (pos));
		ASSERT_VALID (pNextBar);

		pNextBar->SaveState (lpszProfileName, uiID);
	}	
	return TRUE;
}
//*************************************************************************************
BOOL CBCGPMultiMiniFrameWnd::LoadState (LPCTSTR lpszProfileName, UINT uiID)
{
	ASSERT_VALID (this);
	CObList& lstControlBars = m_barContainerManager.m_lstControlBars;
	for (POSITION pos = lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pNextBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstControlBars.GetNext (pos));
		ASSERT_VALID (pNextBar);

		pNextBar->LoadState (lpszProfileName, uiID);
	}	
	return TRUE;
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::Serialize (CArchive& ar)
{
	ASSERT_VALID (this);
	CBCGPMiniFrameWnd::Serialize (ar);
	m_barContainerManager.Serialize (ar);
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::SetDockState (CBCGPDockManager* pDockManager)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDockManager);

	CObList lstBarsToRemove;
	if (!m_barContainerManager.IsEmpty ())
	{
		// float each control bar, reparent it and set its window position
		CObList& lstControlBars = m_barContainerManager.m_lstControlBars;
		POSITION pos = NULL;

		for (pos = lstControlBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPDockingControlBar* pNextBar = 
				DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstControlBars.GetNext (pos));
			ASSERT_VALID (pNextBar);

			if (pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPBaseTabbedBar)))
			{
				BOOL bLeftBar = FALSE;
				CBCGPBarContainer* pContainer = 
					m_barContainerManager.FindContainer (pNextBar, bLeftBar);

				ASSERT (pContainer != NULL);

				CList<UINT, UINT>* pListBarIDs = 
					pContainer->GetAssociatedSiblingBarIDs (pNextBar);

				if (pListBarIDs != NULL)
				{
					for (POSITION pos = pListBarIDs->GetHeadPosition (); pos != NULL;)
					{
						UINT nIDNext = pListBarIDs->GetNext (pos);
						CBCGPBaseControlBar* pBarToAttach = pDockManager->FindBarByID (nIDNext, TRUE);

						if (pBarToAttach == NULL)
						{
							continue;
						}

						if (pBarToAttach->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)) && 
							((CBCGPDockingControlBar*)pBarToAttach)->IsAutoHideMode ())
						{
							((CBCGPDockingControlBar*)pBarToAttach)->SetAutoHideMode (FALSE, CBRS_ALIGN_ANY);
						}

						if (pBarToAttach->IsTabbed ())
						{
							CBCGPBaseTabWnd* pTabWnd = 
									(CBCGPBaseTabWnd*) pBarToAttach->GetParent ();
							CBCGPBaseTabbedBar* pTabBar = (CBCGPBaseTabbedBar*)
									pTabWnd->GetParent ();	
							ASSERT_VALID (pTabBar);

							pBarToAttach->SetParent (this);
							pTabBar->RemoveControlBar (pBarToAttach);
						}
						else
						{
							// float this bar in case if was docked somewhere else
							pBarToAttach->FloatControlBar (CRect (0, 0, 10, 10), BCGP_DM_SHOW, false);
						}
						CBCGPMiniFrameWnd* pParentMiniFrame = pBarToAttach->GetParentMiniFrame ();

						if (pParentMiniFrame != NULL && pParentMiniFrame != this)
						{
							pParentMiniFrame->RemoveControlBar (pBarToAttach);	
						}

						((CBCGPDockingControlBar*) pBarToAttach)->
							AttachToTabWnd (pNextBar, BCGP_DM_UNKNOWN, FALSE);

						pParentMiniFrame->PostMessage (BCGPM_CHECKEMPTYMINIFRAME);
					}
				}

				if (((CBCGPBaseTabbedBar*)pNextBar)->GetTabsNum () == 0)
				{
					lstBarsToRemove.AddTail (pNextBar);
				}
				else
				{
					((CBCGPBaseTabbedBar*)pNextBar)->ApplyRestoredTabInfo ();
					pNextBar->RecalcLayout ();
				}
				continue;
			}

			if (pNextBar->IsTabbed ())
			{
				CBCGPBaseTabWnd* pTabWnd = (CBCGPBaseTabWnd*) pNextBar->GetParent ();
				CBCGPBaseTabbedBar* pTabBar = (CBCGPBaseTabbedBar*) pTabWnd->GetParent ();	
				ASSERT_VALID (pTabBar);
				// set belong to any parent
				pNextBar->SetParent (GetParent ());
				pTabBar->RemoveControlBar (pNextBar);
				if (pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)))
				{
					((CBCGPDockingControlBar*) pNextBar)->EnableGripper (TRUE);
				}

				pNextBar->ShowWindow (SW_SHOW);
			}

			if (pNextBar->IsAutoHideMode ())
			{
				pNextBar->SetAutoHideMode (FALSE, CBRS_ALIGN_ANY);
			}
			
			CRect rectDummy;
			pNextBar->GetWindowRect (rectDummy);
			pNextBar->FloatControlBar (rectDummy, BCGP_DM_SHOW, false);

			CBCGPMiniFrameWnd* pParentMiniFrame = pNextBar->GetParentMiniFrame ();
			if (pParentMiniFrame != NULL)
			{
				pNextBar->SetParent (this);
				pParentMiniFrame->RemoveControlBar (pNextBar);

				CRect rect = pNextBar->m_rectSavedDockedRect;
			
				pNextBar->SetWindowPos (NULL, rect.left, rect.top, rect.Width (), rect.Height (),
											SWP_NOZORDER | SWP_FRAMECHANGED  | SWP_NOACTIVATE);
			}
		}

		for (pos = lstBarsToRemove.GetHeadPosition (); pos != NULL;)
		{
			CBCGPDockingControlBar* pNextBar = 
				DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstBarsToRemove.GetNext (pos));
			RemoveControlBar (pNextBar);
			pNextBar->DestroyWindow ();
		}

		// retake the list
		CObList& lstModifiedControlBars = m_barContainerManager.m_lstControlBars;

		if (lstModifiedControlBars.IsEmpty ())
		{
			SendMessage (BCGPM_CHECKEMPTYMINIFRAME);
			return;
		}

		for (pos = lstModifiedControlBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPDockingControlBar* pNextBar = 
				DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstModifiedControlBars.GetNext (pos));
			ASSERT_VALID (pNextBar);

			pNextBar->ShowControlBar (pNextBar->GetRecentVisibleState (), FALSE, FALSE);
			AddRemoveBarFromGlobalList (pNextBar, TRUE);
		}

		// set embedded bar to the first bar in the container
		CBCGPBaseControlBar* pEmbeddedBar = 
			DYNAMIC_DOWNCAST (CBCGPBaseControlBar, lstControlBars.GetHead ());
		if (pEmbeddedBar != NULL)
		{
			if (lstControlBars.GetCount () > 1)
			{
				m_hEmbeddedBar = pEmbeddedBar->GetSafeHwnd ();	
			}
			else
			{
				CString strText;
				pEmbeddedBar->GetWindowText (strText);
				SetWindowText (strText);

				SetIcon (pEmbeddedBar->GetIcon (FALSE), FALSE);
				SetIcon (pEmbeddedBar->GetIcon (TRUE), TRUE);
			}
		}
		OnSetRollUpTimer ();
		SetCaptionButtons (m_dwCaptionButtons);
		OnBarRecalcLayout ();
		return;
	}

	// if we're here the miniframe is empty and should be destroyed
	PostMessage (BCGPM_CHECKEMPTYMINIFRAME);
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	CSize sizeBase;

	m_barContainerManager.GetMinSize (sizeBase);
	CalcMinSize (sizeBase, lpMMI);

	CWnd::OnGetMinMaxInfo(lpMMI);
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::OnShowControlBar (CBCGPDockingControlBar* pBar, BOOL bShow)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	m_barContainerManager.OnShowControlBar (pBar, bShow);

	if (bShow)
	{
		ShowWindow (SW_SHOWNOACTIVATE);
		OnSetRollUpTimer ();
	}
	else if (!m_barContainerManager.IsRootContainerVisible ())
	{
		ShowWindow (SW_HIDE);
		OnKillRollUpTimer ();
	}
	
	CheckGripperVisibility ();

	OnBarRecalcLayout ();

	// redraw caption to reflect the number of visible bars and set the recent pos
	SetWindowPos (NULL, 0, 0, 0, 0, 
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::CloseMiniFrame ()
{
	if (OnCloseMiniFrame ())
	{
		ShowWindow (SW_HIDE);
		m_barContainerManager.HideAll ();
	}
}

//********************************************************************************
void CBCGPMultiMiniFrameWnd::CheckGripperVisibility ()
{
	if (IsWindowVisible ())
	{
		int nVisibleCount = m_barContainerManager.GetVisibleBarCount ();

		if (nVisibleCount == 1) // take off caption from this bar
		{
			CBCGPDockingControlBar* pVisibleBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
								m_barContainerManager.GetFirstVisibleBar ());

			if (pVisibleBar != NULL)
			{
				pVisibleBar->EnableGripper (FALSE);
			}
		}
		else
		{
			m_barContainerManager.EnableGrippers (TRUE);
		}
	}
}
//********************************************************************************
CString CBCGPMultiMiniFrameWnd::GetCaptionText ()
{
	CString strCaptionText;
	if (m_barContainerManager.GetVisibleBarCount () == 1)
	{
		CWnd* pVisibleBar = DYNAMIC_DOWNCAST (CWnd, m_barContainerManager.GetFirstVisibleBar ());

		if (pVisibleBar != NULL)
		{
			pVisibleBar->GetWindowText (strCaptionText);
		}
	}
	else if (AfxGetApp() != NULL && AfxGetApp()->m_pszAppName != NULL)
	{
		strCaptionText = AfxGetApp()->m_pszAppName;
	}

	return strCaptionText;
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::OnLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/)
{
	CBCGPDockManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : globalUtils.GetDockManager (GetParent ());
	if (!m_bHostsToolbar && pDockManager != NULL && pDockManager->AreFloatingBarsCanBeMaximized() &&
		pDockManager->MaximizeFloatingBarsByDblClick())
	{
		if (!m_rectRestore.IsRectEmpty())
		{
			RestoreFromMaximized();
		}
		else
		{
			SetMaximized();
		}
	}
	else
	{
		OnDockToRecentPos ();
	}
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::OnDockToRecentPos ()
{
	RestoreFromMaximized();

	CBCGPDockManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : globalUtils.GetDockManager (this);	

	CObList lstControlBars; 
	m_barContainerManager.AddControlBarsToList (&lstControlBars, NULL);

	POSITION pos = NULL;

	for (pos = lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pNextBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstControlBars.GetNext (pos));
		ASSERT_VALID (pNextBar);
		
		pNextBar->StoreRecentDockInfo ();
	}

	for (pos = lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pNextBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstControlBars.GetNext (pos));
		ASSERT_VALID (pNextBar);
		
		AddRemoveBarFromGlobalList (pNextBar, FALSE);
		pNextBar->DockControlBar (pNextBar, NULL, BCGP_DM_DBL_CLICK);
	}
	
	globalUtils.ForceAdjustLayout (pDockManager);
	SendMessage (BCGPM_CHECKEMPTYMINIFRAME);
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::SaveRecentFloatingState ()
{
	if (m_rectRestore.IsRectEmpty())
	{
		GetWindowRect (m_rectRecentFloatingRect);
	}
	else
	{
		m_rectRecentFloatingRect = m_rectRestore;
	}

	CObList lstControlBars; 
	m_barContainerManager.AddControlBarsToList (&lstControlBars, NULL);
	for (POSITION pos = lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pNextBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstControlBars.GetNext (pos));
		ASSERT_VALID (pNextBar);
		
		pNextBar->m_recentDockInfo.m_rectRecentFloatingRect = m_rectRecentFloatingRect;
	}
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::StoreRecentDockInfo (CBCGPControlBar* pBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);
	ASSERT_KINDOF (CBCGPDockingControlBar, pBar);

	CBCGPDockingControlBar* pDockingBar = 
		DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pBar);

	if (pDockingBar == NULL)
	{
		return;
	}

	BOOL bLeftBar = TRUE;
	CBCGPBarContainer* pRecentContainer = 
		m_barContainerManager.FindContainer (pDockingBar, bLeftBar);

	pDockingBar->m_recentDockInfo.StoreDockInfo (pRecentContainer);
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::StoreRecentTabRelatedInfo (CBCGPDockingControlBar* pDockingBar, 
													CBCGPDockingControlBar* pTabbedBar)
{
	BOOL bLeftBar = TRUE;
	CBCGPBarContainer* pRecentContainer = 
		m_barContainerManager.FindContainer (pTabbedBar, bLeftBar);

	pDockingBar->m_recentDockInfo.StoreDockInfo (pRecentContainer, pTabbedBar);
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::DockRecentControlBarToMainFrame (CBCGPDockingControlBar* pBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);
	AddRemoveBarFromGlobalList (pBar, FALSE);
	pBar->DockControlBar (pBar, NULL, BCGP_DM_DBL_CLICK);	
}
//********************************************************************************
LRESULT CBCGPMultiMiniFrameWnd::OnCheckEmptyState (WPARAM, LPARAM)
{
	if (m_barContainerManager.m_pRootContainer != NULL)
	{
		m_barContainerManager.m_pRootContainer->ReleaseEmptyContainer ();
	}
	if (m_barContainerManager.GetNodeCount () == 0 ||
		m_barContainerManager.GetNodeCount () == 1 && 
		m_barContainerManager.m_pRootContainer != NULL &&
		m_barContainerManager.m_pRootContainer->GetRefCount () == 0 && 
		m_barContainerManager.m_pRootContainer->IsContainerEmpty ())
	{
		CBCGPMiniFrameWnd::OnCancelMode(); 
		DestroyWindow ();
	}
	else if (m_barContainerManager.GetVisibleBarCount () == 0)
	{
		ShowWindow (SW_HIDE);
		CBCGPMiniFrameWnd::OnCancelMode(); 
	}
	return 0;
}
//********************************************************************************
BOOL CBCGPMultiMiniFrameWnd::InsertControlBar (CBCGPBaseControlBar* pControlBar, 
												CBCGPBaseControlBar* /*pTarget*/, BOOL /*bAfter*/)
{
	AddRemoveBarFromGlobalList (pControlBar, TRUE);
	return TRUE;
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::CalcExpectedDockedRect (CWnd* pWndToDock, CPoint ptMouse,
													 CRect& rectResult, BOOL& bDrawTab, 
													 CBCGPDockingControlBar** ppTargetBar)
{
	CBCGPGlobalUtils globalUtils;	
	if (m_bRolledUp)
	{
		// can't dock on rolled up miniframe
		bDrawTab = FALSE;
		rectResult.SetRectEmpty ();
		return;
	}
	globalUtils.CalcExpectedDockedRect (m_barContainerManager, pWndToDock, 
										ptMouse, rectResult, bDrawTab, ppTargetBar);
}
//********************************************************************************
CBCGPBaseControlBar* CBCGPMultiMiniFrameWnd::ControlBarFromPoint (CPoint point, int nSencitivity,
																BOOL bCheckVisibility)
{
	if (bCheckVisibility && !IsWindowVisible ())
	{
		return NULL;
	}
	BOOL bTabArea = FALSE; 
	BOOL bCaption = FALSE;
	return m_barContainerManager.ControlBarFromPoint (point, nSencitivity, TRUE, bTabArea, bCaption);
}
//********************************************************************************
BOOL CBCGPMultiMiniFrameWnd::CanBeDockedToBar (const CBCGPDockingControlBar* pDockingBar) const
{
	for (POSITION pos = m_barContainerManager.m_lstControlBars.GetHeadPosition (); 
			pos != NULL;)
	{
		CBCGPDockingControlBar* pNextBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
								m_barContainerManager.m_lstControlBars.GetNext (pos));
		ASSERT_VALID (pNextBar);
		
		if (pDockingBar->CanAcceptBar (pNextBar) && 
			pNextBar->CanAcceptBar (pDockingBar))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//********************************************************************************
LRESULT CBCGPMultiMiniFrameWnd::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
	CBCGPMiniFrameWnd::OnIdleUpdateCmdUI (wParam, 0);
	return 0L;
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::ReplaceControlBar (CBCGPBaseControlBar* pBarOrg, 
												CBCGPBaseControlBar* pBarReplaceWith)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBarOrg);
	ASSERT_VALID (pBarReplaceWith);
	ASSERT_KINDOF (CBCGPDockingControlBar, pBarReplaceWith);
	
	m_barContainerManager.ReplaceControlBar ((CBCGPDockingControlBar*) pBarOrg, 
											 (CBCGPDockingControlBar*) pBarReplaceWith);
	OnSetRollUpTimer ();
}
//********************************************************************************
CWnd* CBCGPMultiMiniFrameWnd::GetControlBar () const
{
	CWnd* pWnd = CBCGPMiniFrameWnd::GetControlBar ();
	if (pWnd == NULL)
	{
		pWnd = GetFirstVisibleBar ();
		if (pWnd == NULL)
		{
			pWnd = m_barContainerManager.GetFirstBar ();
		}
	}

	return pWnd;
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::RemoveNonValidBars ()
{
	m_barContainerManager.RemoveNonValidBars ();
}
//********************************************************************************
void CBCGPMultiMiniFrameWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	CBCGPBaseControlBar* pFirstBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar, GetFirstVisibleBar ());
	if (m_hWndLastFocused == NULL)
	{
		if (pFirstBar != NULL && ::IsWindow (pFirstBar->GetSafeHwnd ()) && pFirstBar->CanFocus ())
		{
			pFirstBar->SetFocus ();
		}
	}
	else
	{
		CBCGPDockingControlBar* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
								CWnd::FromHandlePermanent (m_hWndLastFocused));
		CBCGPBarContainer* pContainer = NULL;
		if (pWnd != NULL)
		{
			BOOL bLeftBar;
			pContainer = m_barContainerManager.FindContainer (pWnd, bLeftBar);
		}

		if (pContainer != NULL && ::IsWindow (pWnd->GetSafeHwnd ()))
		{
			pWnd->SetFocus ();
		}
		else if (pFirstBar != NULL && ::IsWindow (pFirstBar->GetSafeHwnd ())) 
		{
			pFirstBar->SetFocus ();
		}
	}

	if (GetParentFrame () != NULL)
	{
		GetParentFrame ()->SetWindowPos (&wndTop, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}	
}
void CBCGPMultiMiniFrameWnd::ConvertToTabbedDocument ()
{
	CObList lstControlBars; 
	m_barContainerManager.AddControlBarsToList (&lstControlBars, NULL);
	for (POSITION pos = lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pNextBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstControlBars.GetNext (pos));
		ASSERT_VALID (pNextBar);
		pNextBar->ConvertToTabbedDocument (FALSE);
	}

	PostMessage (BCGPM_CHECKEMPTYMINIFRAME);
}

LRESULT CBCGPMultiMiniFrameWnd::OnChangeVisualManager(WPARAM wp, LPARAM lp)
{
	CBCGPMiniFrameWnd::OnChangeVisualManager(wp, lp);

	CObList& lstControlBars = m_barContainerManager.m_lstControlBars;

	for (POSITION pos = lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pNextBar = DYNAMIC_DOWNCAST(CBCGPDockingControlBar, lstControlBars.GetNext(pos));
		ASSERT_VALID (pNextBar);

		pNextBar->SendMessageToDescendants(BCGM_CHANGEVISUALMANAGER, wp, lp, TRUE, FALSE);
	}

	return 0;
}