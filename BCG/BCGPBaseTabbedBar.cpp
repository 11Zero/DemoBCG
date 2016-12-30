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
// BCGPBaseTabbedBar.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPBaseTabWnd.h"
#include "BCGPBaseTabbedBar.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPMultiMiniFrameWnd.h"
#include "BCGPDockingCBWrapper.h"
#include "BCGPAutoHideToolBar.h"
#include "BCGPDockBar.h"
#include "BCGPGlobalUtils.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPVisualManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CBCGPBaseTabbedBar, CBCGPDockingControlBar)

/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseTabbedBar

CBCGPBaseTabbedBar::CBCGPBaseTabbedBar (BOOL bAutoDestroy)
{
	m_bAutoDestroy = bAutoDestroy;
	m_pTabWnd = NULL;
	m_bEnableIDChecking = FALSE;
	m_bSetCaptionTextToTabName = TRUE;

	EnableDocking (CBRS_ALIGN_ANY);
}
//***********************************************************************************
CBCGPBaseTabbedBar::~CBCGPBaseTabbedBar()
{
}

BEGIN_MESSAGE_MAP(CBCGPBaseTabbedBar, CBCGPDockingControlBar)
	//{{AFX_MSG_MAP(CBCGPBaseTabbedBar)
	ON_WM_SIZE()
	ON_WM_NCDESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(BCGM_CHANGE_ACTIVE_TAB, OnChangeActiveTab)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseTabbedBar message handlers

void CBCGPBaseTabbedBar::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPDockingControlBar::OnSize(nType, cx, cy);
	
	CWnd* pUnderlinedWnd = GetUnderlinedWindow ();

	if (pUnderlinedWnd != NULL && IsWindow (pUnderlinedWnd->GetSafeHwnd ()))
	{
		CRect rectClient;
		GetClientRect (rectClient);
		
		pUnderlinedWnd->SetWindowPos (NULL, 0, 0, rectClient.Width (), rectClient.Height (), 
								SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
	}
}
//***********************************************************************************
void CBCGPBaseTabbedBar::OnNcDestroy() 
{
	if (m_pTabWnd != NULL)
	{
		delete m_pTabWnd;
		m_pTabWnd = NULL;
	}

	CBCGPDockingControlBar::OnNcDestroy();
	
	if (m_bAutoDestroy)
	{
		delete this;
	}
}
//***********************************************************************************
BOOL CBCGPBaseTabbedBar::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE; 
}
//***********************************************************************************
BOOL CBCGPBaseTabbedBar::AddTab (CWnd* pNewBar, BOOL bVisible, BOOL bSetActive, 
								 BOOL bDetachable)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pTabWnd);
	ASSERT_VALID (pNewBar);

	if (pNewBar->IsKindOf (RUNTIME_CLASS (CBCGPBaseTabbedBar)))
	{
		CBCGPBaseTabbedBar* pTabbedControlBar = 
			DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pNewBar);

		// it's false when the tabbed bar is added from miniframe to docksite
		BOOL bSetInfoForSlider = (GetParentMiniFrame () != NULL);

		ASSERT_VALID (pTabbedControlBar);

		CBCGPBaseTabWnd* pWndTab = pTabbedControlBar->GetUnderlinedWindow ();
		
		ASSERT_VALID (pWndTab);

		int nTabsNum = pWndTab->GetTabsNum ();
		ASSERT (nTabsNum > 0);

		for (int i = 0; i < nTabsNum; i++)
		{
			CBCGPBaseControlBar* pWnd = 
				DYNAMIC_DOWNCAST (CBCGPBaseControlBar, pWndTab->GetTabWnd (i));
			ASSERT_VALID (pWnd);

			BOOL bVisible = pWndTab->IsTabVisible (i);
			BOOL bDetachable = pWndTab->IsTabDetachable (i);

			pWnd->EnableGripper (FALSE);

			if (!AddTab (pWnd, bVisible, bVisible, bDetachable))
			{
				ASSERT (FALSE);
			}

			CBCGPDockingControlBar* pDockingBar = 
				DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pWnd);
			if (pDockingBar != NULL)
			{
				pDockingBar->m_recentDockInfo.SetInfo (bSetInfoForSlider, 
														pTabbedControlBar->m_recentDockInfo);
			}
		}

		pWndTab->RemoveAllTabs ();
		pNewBar->DestroyWindow ();

		// stop processing - this function will be called 
		// from AttachToTabWnd

		return FALSE;
	}
	else
	{
		if (pNewBar->IsKindOf (RUNTIME_CLASS (CBCGPControlBar)))
		{
			CBCGPControlBar* pNewControlBar = 
				DYNAMIC_DOWNCAST (CBCGPControlBar, pNewBar);
			ASSERT_VALID (pNewControlBar);

			CWnd* pOldParent = pNewControlBar->GetParent ();
			pNewControlBar->OnBeforeChangeParent (m_pTabWnd, TRUE);
			pNewControlBar->SetParent (m_pTabWnd);
			pNewControlBar->OnAfterChangeParent (pOldParent);

			if (pNewControlBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)))
			{
				((CBCGPDockingControlBar*) pNewControlBar)->EnableGripper (FALSE);
			}
		}

		CString strText; 
		pNewBar->GetWindowText (strText);

		m_pTabWnd->AddTab (pNewBar, strText, bSetActive, bDetachable);

		int iTab = m_pTabWnd->GetTabsNum () - 1;
		m_pTabWnd->SetTabHicon (iTab, pNewBar->GetIcon (FALSE));
		m_pTabWnd->EnableTabDetach (iTab, bDetachable);

		if (bVisible)
		{
			if (bSetActive)
			{
				m_pTabWnd->SetActiveTab (iTab);
			}
		}
		else
		{
			ASSERT (!bSetActive);
			m_pTabWnd->ShowTab (iTab, FALSE);
		}
	}
	return TRUE;
}

//**************************************************************************************
CWnd* CBCGPBaseTabbedBar::FindBarByID (UINT uBarID)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pTabWnd);

	for (int i = 0; i < m_pTabWnd->GetTabsNum (); i++)
	{
		CWnd* pBar = m_pTabWnd->GetTabWnd (i);
		ASSERT_VALID (pBar);

		if ((UINT) pBar->GetDlgCtrlID () == uBarID)
		{
			return pBar;
		}
	}

	return NULL;
}
//**************************************************************************************
CWnd* CBCGPBaseTabbedBar::FindBarByTabNumber (int nTabNum, BOOL bGetWrappedBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pTabWnd);

	if (nTabNum < 0 || nTabNum >= m_pTabWnd->GetTabsNum ())
	{
		return NULL;
	}

	CWnd* pWnd = m_pTabWnd->GetTabWnd (nTabNum);
	ASSERT_VALID (pWnd);

	if (bGetWrappedBar && pWnd->IsKindOf (RUNTIME_CLASS (CBCGPDockingCBWrapper)))
	{
		CBCGPDockingCBWrapper* pWrapper = 
			DYNAMIC_DOWNCAST (CBCGPDockingCBWrapper, pWnd);
		pWnd = pWrapper->GetWrappedWnd ();
		ASSERT_VALID (pWnd);
	}

	return pWnd;
}
//*******************************************************************************
BOOL CBCGPBaseTabbedBar::DetachControlBar (CWnd* pBar, BOOL bHide)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);
	ASSERT_VALID (m_pTabWnd);

	int nTabNumber = m_pTabWnd->GetTabFromHwnd (pBar->GetSafeHwnd ());

	if (nTabNumber < 0)
	{
		return FALSE;
	}

	m_pTabWnd->DetachTab (BCGP_DM_UNKNOWN, nTabNumber, bHide);
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPBaseTabbedBar::RemoveControlBar (CWnd* pBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);
	ASSERT_VALID (m_pTabWnd);

	int nTabNumber = m_pTabWnd->GetTabFromHwnd (pBar->GetSafeHwnd ());

	if (nTabNumber < 0 || nTabNumber >= m_pTabWnd->GetTabsNum ())
	{
		return FALSE;
	}

	m_pTabWnd->RemoveTab (nTabNumber);

	if (m_pTabWnd->GetTabsNum () == 0)
	{
		if (AllowDestroyEmptyTabbedBar ())
		{
			if (IsDocked ())
			{
				UnDockControlBar ();
			}
			else
			{
				CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame ();
				pMiniFrame->RemoveControlBar (this);
			}

			DestroyWindow ();
			return FALSE;
		}
		else 
		{
			m_pTabWnd->ShowWindow (SW_HIDE);
		}
	}

	return TRUE;
}
//*******************************************************************************
BOOL CBCGPBaseTabbedBar::ShowTab (CWnd* pBar, BOOL bShow, BOOL bDelay, BOOL bActivate)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);
	ASSERT_VALID (m_pTabWnd);
	
	int nTabNum = m_pTabWnd->GetTabFromHwnd (pBar->GetSafeHwnd ());

	BOOL bResult = m_pTabWnd->ShowTab (nTabNum, bShow, !bDelay, bActivate);

	BOOL bNowVisible = m_pTabWnd->GetVisibleTabsNum () > 0;

	if (bNowVisible && !(m_pTabWnd->GetStyle () & WS_VISIBLE))
	{
		m_pTabWnd->ShowWindow (SW_SHOW);
	}

	CBCGPDockingControlBar::ShowControlBar (bNowVisible, bDelay, bActivate);
	return bResult;
}
//*******************************************************************************
BOOL CBCGPBaseTabbedBar::FloatTab (CWnd* pBar, int nTabID, 
									  BCGP_DOCK_METHOD dockMethod, 
									  BOOL bHide)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);
	ASSERT_VALID (m_pTabWnd);

	CString strWndText;
	pBar->GetWindowText (strWndText);

	if (strWndText.IsEmpty ())
	{
		if (m_pTabWnd->GetTabLabel (nTabID, strWndText))
		{
			pBar->SetWindowText (strWndText);
		}
	}

	m_pTabWnd->RemoveTab (nTabID);
	
	if (dockMethod == BCGP_DM_MOUSE)
	{
		m_pTabWnd->SendMessage (WM_LBUTTONUP, 0, 0);
	}

	CBCGPDockingControlBar* pDockingBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pBar);

	if (pDockingBar != NULL)
	{
		pDockingBar->StoreRecentTabRelatedInfo ();
	}

	if (dockMethod == BCGP_DM_DBL_CLICK && pDockingBar != NULL)
	{
		CBCGPMultiMiniFrameWnd* pParentMiniFrame = 
				DYNAMIC_DOWNCAST (CBCGPMultiMiniFrameWnd, GetParentMiniFrame ());

		if (pParentMiniFrame != NULL)
		{
			pParentMiniFrame->DockRecentControlBarToMainFrame (pDockingBar);
			return TRUE;
		}
		else if (m_hDefaultSlider != NULL && IsWindow (m_hDefaultSlider))
		{
			CBCGPMultiMiniFrameWnd* pRecentMiniFrame = 
				DYNAMIC_DOWNCAST (CBCGPMultiMiniFrameWnd,
					CWnd::FromHandlePermanent (pDockingBar->m_recentDockInfo.m_hRecentMiniFrame));
			if (pRecentMiniFrame != NULL && 
				pRecentMiniFrame->AddRecentControlBar (pDockingBar))
			{
				return TRUE;
			}
		}
	}

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPControlBar)))
	{
		CBCGPControlBar* pControlBar = 
			DYNAMIC_DOWNCAST (CBCGPControlBar, pBar);
		ASSERT_VALID (pControlBar);
		pControlBar->FloatControlBar (pControlBar->m_recentDockInfo.m_rectRecentFloatingRect, 
									  dockMethod, !bHide);
		return TRUE;
	}
	return FALSE;
}
//**************************************************************************************
void CBCGPBaseTabbedBar::StoreRecentDockInfo ()
{
	int nTabsNum = m_pTabWnd->GetTabsNum ();
	for (int i = 0; i < nTabsNum; i++)
	{
		CBCGPDockingControlBar* pBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, m_pTabWnd->GetTabWnd (i));
		if (pBar != NULL)
		{
			pBar->StoreRecentTabRelatedInfo ();
		}
	}

	CBCGPDockingControlBar::StoreRecentDockInfo ();
}
//**************************************************************************************
BOOL CBCGPBaseTabbedBar::FloatControlBar (CRect rectFloat, 
											 BCGP_DOCK_METHOD dockMethod,
											 bool bShow)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pTabWnd);

	if (!CBCGPDockingControlBar::FloatControlBar (rectFloat, dockMethod, bShow))
	{
		return FALSE;
	}

	CBCGPMiniFrameWnd* pParentFrame = GetParentMiniFrame ();
	if (pParentFrame != NULL)
	{
		pParentFrame->SetIcon (m_pTabWnd->GetTabHicon (m_pTabWnd->GetActiveTab ()), FALSE);
	}

	return TRUE;
}
//**************************************************************************************
void CBCGPBaseTabbedBar::Serialize (CArchive& ar)
{
	CBCGPDockingControlBar::Serialize (ar);
	if (ar.IsLoading ())
	{
		ar >> m_bAutoDestroy;
	}
	else
	{
		ar << m_bAutoDestroy;
	}
}
//**************************************************************************************
void CBCGPBaseTabbedBar::SerializeTabWindow (CArchive& ar)
{
	if (m_pTabWnd != NULL)
	{
		m_pTabWnd->Serialize (ar);
	}
}
//**************************************************************************************
void CBCGPBaseTabbedBar::LoadSiblingBarIDs (CArchive& ar, CList<UINT, UINT>& lstBarIDs)
{
	ASSERT (ar.IsLoading ());
	if (ar.IsLoading ())
	{
		int nTabsNum = 0;
		ar >> nTabsNum;
		for (int i = 0; i < nTabsNum; i++)
		{
			int nBarID = -1;
			ar >> nBarID;
			ASSERT (nBarID != -1);
			lstBarIDs.AddTail (nBarID);
		}
	}
}
//**************************************************************************************
void CBCGPBaseTabbedBar::SaveSiblingBarIDs (CArchive& ar)
{
	ASSERT_VALID (this);
	ASSERT (ar.IsStoring ());
	ASSERT_VALID (m_pTabWnd);

	if (ar.IsStoring () && m_pTabWnd != NULL)
	{
		int nTabsNum = m_pTabWnd->GetTabsNum ();
		// DO NOT SAVE empty tabbed bars
		if (nTabsNum > 0)
		{
			ar << nTabsNum;
			for (int i = 0; i < nTabsNum; i++)
			{
				CBCGPBaseControlBar* pWnd = 
					DYNAMIC_DOWNCAST (CBCGPBaseControlBar, m_pTabWnd->GetTabWnd (i));
				ASSERT_VALID (pWnd);

				ar << pWnd->GetDlgCtrlID ();
			}
		}
	}
}
//**************************************************************************************
BOOL CBCGPBaseTabbedBar::LoadState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pTabWnd);

	FillDefaultTabsOrderArray ();

	// if initially tabbed bars were detached by user and exist only as regular
	// docking control bars we need to give them a chance to load their state 
	// from the registry

	CBCGPDockingControlBar::LoadState (lpszProfileName, nIndex, uiID);

	int nTabsNum = m_pTabWnd->GetTabsNum ();
	for (int i = 0; i < nTabsNum; i++)
	{
		CBCGPBaseControlBar* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPBaseControlBar, m_pTabWnd->GetTabWnd (i));

		if (pWnd != NULL && IsRestoreTabsState())
		{
			ASSERT_VALID (pWnd);
			pWnd->LoadState (lpszProfileName, nIndex, uiID);
		}
	}

	return TRUE;
}
//**************************************************************************************
BOOL CBCGPBaseTabbedBar::SaveState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pTabWnd);

	CBCGPDockingControlBar::SaveState (lpszProfileName, nIndex, uiID);

	int nTabsNum = m_pTabWnd->GetTabsNum ();
	for (int i = 0; i < nTabsNum; i++)
	{
		CBCGPBaseControlBar* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPBaseControlBar, m_pTabWnd->GetTabWnd (i));

		if (pWnd != NULL)
		{
			ASSERT_VALID (pWnd);
			if (!pWnd->SaveState (lpszProfileName, nIndex, uiID))
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

//**************************************************************************************
void CBCGPBaseTabbedBar::ApplyRestoredTabInfo (BOOL bUseTabIndexes)
{
	ASSERT_VALID (this);

	if (m_pTabWnd != NULL)
	{
		m_pTabWnd->ApplyRestoredTabInfo (bUseTabIndexes);
	}
}
//**************************************************************************************
void CBCGPBaseTabbedBar::RecalcLayout ()
{
	ASSERT_VALID (this);

	CBCGPDockingControlBar::RecalcLayout ();

	if (m_pTabWnd != NULL)
	{
		m_pTabWnd->RecalcLayout ();
	}
}
//**************************************************************************************
BOOL CBCGPBaseTabbedBar::CanFloat () const
{
	ASSERT_VALID (this);

	return CBCGPDockingControlBar::CanFloat ();
}
//**************************************************************************************
void CBCGPBaseTabbedBar::OnSetFocus(CWnd* pOldWnd) 
{
	CBCGPDockingControlBar::OnSetFocus(pOldWnd);

	// Pass the focus to the tab window
	CWnd*	pWndChild	= GetUnderlinedWindow();
	if (pWndChild != NULL)
		pWndChild->SetFocus();
}
//**************************************************************************************
CBCGPAutoHideToolBar* CBCGPBaseTabbedBar::SetAutoHideMode (BOOL bMode, DWORD dwAlignment, 
												CBCGPAutoHideToolBar* pCurrAutoHideBar, 
												BOOL bUseTimer)
{
	BOOL bHandleMinSize = CBCGPControlBar::m_bHandleMinSize;
    if (bHandleMinSize)
    {
         CBCGPControlBar::m_bHandleMinSize = FALSE;
    }

	CBCGPAutoHideToolBar* pAutoHideBar = pCurrAutoHideBar;
	CBCGPDockingControlBar* pActiveBar = NULL;

	int nActiveTab = m_pTabWnd->GetActiveTab ();
	int nTabsNum = m_pTabWnd->GetTabsNum ();
	
	CObList lstTmp;

	ShowControlBar (FALSE, TRUE, FALSE);

	int nNonDetachedCount = 0;
	for (int nNextTab = nTabsNum - 1; nNextTab >= 0; nNextTab--)
	{
		CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
													m_pTabWnd->GetTabWnd (nNextTab));
		ASSERT_VALID (pBar);

		BOOL bIsVisible = m_pTabWnd->IsTabVisible (nNextTab);
		BOOL bDetachable = m_pTabWnd->IsTabDetachable (nNextTab);
	
		if (pBar != NULL && bIsVisible && bDetachable)
		{
			m_pTabWnd->RemoveTab (nNextTab, FALSE);
			pBar->EnableGripper (TRUE);

			pBar->StoreRecentTabRelatedInfo ();

			CWnd* pOldParent = pBar->GetParent ();
			pBar->OnBeforeChangeParent (m_pDockSite);
			pBar->SetParent (m_pDockSite);
			pBar->SetOwner (m_pDockSite);
			pBar->OnAfterChangeParent (pOldParent);

			lstTmp.AddHead (pBar);
			
			if (nNextTab == nActiveTab)
			{
				pActiveBar = pBar;
			}
		}
		else
		{
			nNonDetachedCount++;
		}
	}

	BOOL bActiveSet = FALSE;
	CBCGPControlBar* pNewAHBar = NULL;

	for (POSITION pos = lstTmp.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
													lstTmp.GetNext (pos));
		BOOL bUseTimerForActiveBar = (pBar == pActiveBar) && bUseTimer;
		pNewAHBar = pBar->SetAutoHideMode (TRUE, dwAlignment, NULL, bUseTimerForActiveBar);

		if (pNewAHBar != NULL)
		{
			pNewAHBar->m_bFirstInGroup = (lstTmp.GetHead () == pBar);
			pNewAHBar->m_bLastInGroup = (lstTmp.GetTail () == pBar);
			pNewAHBar->m_bActiveInGroup = (pBar == pActiveBar);

			if (!bActiveSet && pNewAHBar->m_bActiveInGroup)
			{
				bActiveSet = TRUE;
			}
		}
	}

	if (pNewAHBar != NULL)
	{
		if (!bActiveSet)
		{
			pNewAHBar->m_bActiveInGroup = TRUE;
		}
		CRect rect (0, 0, 0, 0);
		pNewAHBar->GetParentDockBar ()->RepositionBars (rect);
	}

	if (nNonDetachedCount > 0)
	{
		if (m_pTabWnd->GetVisibleTabsNum () == 0)
		{
			ShowControlBar (FALSE, TRUE, FALSE);
		}
		else
		{
			if (m_pTabWnd->GetActiveTab () == -1)
			{	
				int nVisibleTab = -1;
				GetFirstVisibleTab (nVisibleTab);
				m_pTabWnd->SetActiveTab (nVisibleTab);
			}
			m_pTabWnd->RecalcLayout ();
			ShowControlBar (TRUE, TRUE, FALSE);
			pAutoHideBar = CBCGPDockingControlBar::SetAutoHideMode (bMode, dwAlignment, pCurrAutoHideBar, bUseTimer);
		}
	}
	
	if (pAutoHideBar != NULL)
	{
		pAutoHideBar->UpdateVisibleState();
	}

	CBCGPControlBar::m_bHandleMinSize = bHandleMinSize;

	return pAutoHideBar;
}
//**************************************************************************************
CWnd* CBCGPBaseTabbedBar::GetFirstVisibleTab (int& iTabNum)
{
	iTabNum = -1;
	if (m_pTabWnd == NULL)
	{
		return NULL;
	}

	return m_pTabWnd->GetFirstVisibleTab (iTabNum);
}
//**************************************************************************************
HICON CBCGPBaseTabbedBar::GetBarIcon (BOOL bBigIcon)
{
	HICON hIcon = GetIcon (bBigIcon);

	if (hIcon == NULL && m_pTabWnd != NULL)
	{
		CWnd* pWnd = m_pTabWnd->GetActiveWnd ();
		if (pWnd != NULL)
		{
			hIcon = pWnd->GetIcon (bBigIcon);
		}
	}

	return hIcon;
}
//**************************************************************************************
LRESULT CBCGPBaseTabbedBar::OnChangeActiveTab (WPARAM wp, LPARAM)
{
	int iTabNum = (int) wp;

	CString strLabel;
	if (m_pTabWnd != NULL && m_pTabWnd->GetTabLabel (iTabNum, strLabel) &&
		m_bSetCaptionTextToTabName)
	{
		SetWindowText (strLabel);
	}

	OnActivateTab (iTabNum);
	if (CBCGPControlBar::m_bHandleMinSize)
	{
		CBCGPMiniFrameWnd* pWnd = GetParentMiniFrame ();
		if (pWnd != NULL)
		{
			pWnd->OnBarRecalcLayout ();
		}
		else
		{
			globalUtils.ForceAdjustLayout (globalUtils.GetDockManager (GetDockSite ()));
		}
	}
	return 0;
}
//**************************************************************************************
BOOL CBCGPBaseTabbedBar::Dock (CBCGPBaseControlBar* pTargetBar, LPCRECT lpRect, 
							   BCGP_DOCK_METHOD dockMethod)
{
	BOOL bFloating = (GetParentMiniFrame () != NULL);
	int nTabsNum = m_pTabWnd->GetTabsNum ();
	BOOL bTabsHaveRecentInfo = TRUE;

	if (bFloating)
	{
		for (int i = 0; i < nTabsNum; i++)
		{
			if (m_pTabWnd->IsTabDetachable (i))
			{
				CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
															m_pTabWnd->GetTabWnd (i));
				if (pBar != NULL)
				{
					ASSERT_VALID (pBar);
					if (pBar->m_recentDockInfo.GetRecentContainer (TRUE) == NULL &&
						pBar->m_recentDockInfo.GetRecentTabContainer (TRUE) == NULL)
					{
						bTabsHaveRecentInfo = FALSE;
						break;
					}
				}
			}
		}	
	}

	if (dockMethod != BCGP_DM_DBL_CLICK || !bTabsHaveRecentInfo)
	{
		return CBCGPDockingControlBar::Dock (pTargetBar, lpRect, dockMethod);
	}
	
	
	if (bFloating && m_recentDockInfo.GetRecentContainer (TRUE) != NULL ||
		!bFloating && m_recentDockInfo.GetRecentContainer (FALSE) != NULL)
	{
		return CBCGPDockingControlBar::Dock (pTargetBar, lpRect, dockMethod);
	}
	

	ShowControlBar (FALSE, TRUE, FALSE);

	int nNonDetachedCount = 0;
	for (int nNextTab = nTabsNum - 1; nNextTab >= 0; nNextTab--)
	{
		CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
													m_pTabWnd->GetTabWnd (nNextTab));
		ASSERT_VALID (pBar);

		BOOL bIsVisible = m_pTabWnd->IsTabVisible (nNextTab);
		BOOL bDetachable = m_pTabWnd->IsTabDetachable (nNextTab);
	
		if (pBar != NULL && bIsVisible && bDetachable)
		{
			m_pTabWnd->RemoveTab (nNextTab, FALSE);
			pBar->EnableGripper (TRUE);

			pBar->StoreRecentTabRelatedInfo ();

			pBar->DockControlBar (pBar, lpRect, dockMethod);			
		}
		else
		{
			nNonDetachedCount++;
		}
	}

	if (nNonDetachedCount > 0)
	{
		if (m_pTabWnd->GetVisibleTabsNum () == 0)
		{
			ShowControlBar (FALSE, TRUE, FALSE);
		}
		else
		{
			if (m_pTabWnd->GetActiveTab () == -1)
			{	
				int nVisibleTab = -1;
				GetFirstVisibleTab (nVisibleTab);
				m_pTabWnd->SetActiveTab (nVisibleTab);
			}
			m_pTabWnd->RecalcLayout ();
			ShowControlBar (TRUE, TRUE, FALSE);
			return CBCGPDockingControlBar::Dock (pTargetBar, lpRect, dockMethod);
			
		}
	}
	else
	{
		DestroyWindow ();
		return FALSE;
	}

	return TRUE;
}
//**************************************************************************************
void CBCGPBaseTabbedBar::FillDefaultTabsOrderArray ()
{
	ASSERT_VALID (m_pTabWnd);
	m_arDefaultTabsOrder.RemoveAll ();

	const int nTabsNum = m_pTabWnd->GetTabsNum ();

	for (int i = 0; i < nTabsNum; i++)
	{
		int nID = m_pTabWnd->GetTabID (i);
		m_arDefaultTabsOrder.Add (nID);
	}
}
//**************************************************************************************
void CBCGPBaseTabbedBar::GetMinSize (CSize& size) const
{
	if (CBCGPControlBar::m_bHandleMinSize)
	{
		CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
								m_pTabWnd->GetActiveWnd ());
		if (pBar != NULL)
		{
			pBar->GetMinSize (size);
			return;

		}
	}
	CBCGPDockingControlBar::GetMinSize (size);
}
//**************************************************************************************
void CBCGPBaseTabbedBar::GetControlBarList (CObList& lst, CRuntimeClass* pRTCFilter)
{
	CBCGPBaseTabWnd* pTabWnd = GetUnderlinedWindow ();
	for (int i = 0; i < pTabWnd->GetTabsNum (); i++)
	{
		CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pTabWnd->GetTabWnd (i));
		if (pBar != NULL)
		{
			ASSERT_VALID (pBar);
			if (pRTCFilter == NULL || pBar->GetRuntimeClass () == pRTCFilter)
			{
				lst.AddTail (pBar);
			}
		}
	}
}
//*******************************************************************************
void CBCGPBaseTabbedBar::ConvertToTabbedDocument (BOOL bActiveTabOnly)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pTabWnd);

	CBCGPMDIFrameWnd* pMDIFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, GetDockSite ());
	if (pMDIFrame == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	ASSERT_VALID (pMDIFrame);

	HWND hwnd = GetSafeHwnd ();

	if (bActiveTabOnly)
	{
		CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
			m_pTabWnd->GetActiveWnd ());
		if (pBar == NULL)
		{
			return;
		}

		pBar->StoreRecentTabRelatedInfo ();
		pMDIFrame->ControlBarToTabbedDocument (pBar);

		

		RemoveControlBar (pBar);
	}
	else
	{
		CObList lst;
		CBCGPBaseTabWnd* pTabWnd = GetUnderlinedWindow ();

		for (int i = 0; i < pTabWnd->GetTabsNum (); i++)
		{
			if (pTabWnd->IsTabVisible (i))
			{
				CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pTabWnd->GetTabWnd (i));
				if (pBar != NULL)
				{
					pBar->StoreRecentTabRelatedInfo ();
					lst.AddTail (pBar);
				}
			}
		}

		for (POSITION pos = lst.GetHeadPosition (); pos != NULL;)
		{
			CBCGPDockingControlBar* pBar = (CBCGPDockingControlBar*) lst.GetNext (pos);
			pMDIFrame->ControlBarToTabbedDocument (pBar);
			RemoveControlBar (pBar);
		}
	}

	if (IsWindow (hwnd) && GetVisibleTabsNum () == 0 && GetTabsNum () > 0)
	{
		ShowControlBar (FALSE, FALSE, FALSE);
	}
}
//*******************************************************************************
void CBCGPBaseTabbedBar::OnChangeActiveState()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pTabWnd);

	m_pTabWnd->SetParentFocused(m_bActive);

	if (CBCGPVisualManager::GetInstance ()->IsFocusedTabSeparateLook())
	{
		m_pTabWnd->RedrawWindow();
	}
}
