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
// BCGPBaseTabWnd.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPBaseTabWnd.h"
#include "BCGPDockingControlBar.h"
#include "BCGPDockingCBWrapper.h"
#include "BCGPBaseTabbedBar.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPDockManager.h"
#include "BCGPGlobalUtils.h"
#include "BCGPMainClientAreaWnd.h"
#include "BCGPTooltipManager.h"
#include "BCGPTabbedControlBar.h"
#include "BCGPLocalResource.h"
#include "BCGProRes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPTabDropTarget message handlers
//***************************************************************************************
BOOL CBCGPTabDropTarget::Register (CBCGPBaseTabWnd *pOwner)
{
	m_pOwner = pOwner;
	return COleDropTarget::Register (pOwner);
}
//***************************************************************************************
DROPEFFECT CBCGPTabDropTarget::OnDragEnter(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	ASSERT (m_pOwner != NULL);

	if (!CBCGPToolBar::IsCustomizeMode () ||
		!pDataObject->IsDataAvailable (CBCGPToolbarButton::m_cFormat))
	{
		return DROPEFFECT_NONE;
	}

	return m_pOwner->OnDragEnter(pDataObject, dwKeyState, point);
}
//***************************************************************************************
void CBCGPTabDropTarget::OnDragLeave(CWnd* /*pWnd*/) 
{
	ASSERT (m_pOwner != NULL);
	m_pOwner->OnDragLeave ();
}
//***************************************************************************************
DROPEFFECT CBCGPTabDropTarget::OnDragOver(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) 
{
	ASSERT (m_pOwner != NULL);

	if (!CBCGPToolBar::IsCustomizeMode () || 
		!pDataObject->IsDataAvailable (CBCGPToolbarButton::m_cFormat)) 
	{
		if (m_pOwner != NULL)
		{
			int nTabIndex = m_pOwner->GetTabFromPoint(point);

			if (nTabIndex != -1 && nTabIndex != m_pOwner->GetActiveTab())
			{
				m_pOwner->SetActiveTab (nTabIndex);
			}
		}

		return(DROPEFFECT_NONE);
	}


	return m_pOwner->OnDragOver(pDataObject, dwKeyState, point);
}
//***************************************************************************************
DROPEFFECT CBCGPTabDropTarget::OnDropEx(CWnd* /*pWnd*/, 
							COleDataObject* pDataObject, 
							DROPEFFECT dropEffect, 
							DROPEFFECT /*dropList*/, CPoint point) 
{
	ASSERT (m_pOwner != NULL);

	if (!CBCGPToolBar::IsCustomizeMode () ||
		!pDataObject->IsDataAvailable (CBCGPToolbarButton::m_cFormat))
	{
		return DROPEFFECT_NONE;
	}

	return m_pOwner->OnDrop(pDataObject, dropEffect, point) ?
			dropEffect : DROPEFFECT_NONE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseTabWnd

int CBCGPBaseTabWnd::TAB_TEXT_MARGIN = 4;
int CBCGPBaseTabWnd::TAB_IMAGE_MARGIN = 4;

#define DEFAULT_TAB_BORDER_SIZE	2

UINT BCGM_ON_RENAME_TAB			= ::RegisterWindowMessage (_T("BCGM_ON_RENAME_TAB"));
UINT BCGM_CHANGE_ACTIVE_TAB		= ::RegisterWindowMessage (_T("BCGM_ONCHANGE_ACTIVE_TAB"));
UINT BCGM_ON_MOVE_TAB			= ::RegisterWindowMessage (_T("BCGM_ON_MOVE_TAB"));
UINT BCGM_CHANGING_ACTIVE_TAB	= ::RegisterWindowMessage (_T("BCGM_ON_CHANGING_ACTIVE_TAB"));
UINT BCGM_ON_GET_TAB_TOOLTIP	= ::RegisterWindowMessage (_T("BCGM_ON_GET_TAB_TOOLTIP"));
UINT BCGM_NEW_TAB				= ::RegisterWindowMessage (_T("BCGM_NEW_TAB"));

IMPLEMENT_DYNAMIC(CBCGPBaseTabWnd, CBCGPWnd)

CBCGPBaseTabWnd::CBCGPBaseTabWnd()
{
	m_bAutoDestoyWindow = FALSE; 
	m_iActiveTab = -1;
	m_iTabBeforeDrag = -1;
	m_iTabsNum = 0;

	m_sizeImage = CSize (0, 0);
	m_hImageList = NULL;

	m_iCurTab = -1;
	m_nNextTabID = 1;

	m_bCustomToolTips = FALSE;

	m_bHideInactiveWnd = TRUE;

	m_location = LOCATION_BOTTOM;
	m_bReadyToDetach = FALSE;
	m_ptHot = CPoint (0, 0);
	m_nOffsetFromTabLeft = 0;

	m_iHighlighted = -1;
	m_iPressed = -1;

	m_bActivateOnBtnUp = FALSE;

	m_bEnableTabSwap = TRUE;
	m_bDontActivateView = FALSE;
	m_nRestoredActiveTabID = 0;	

	m_pDockingBarWrapperRTC = NULL;

	m_bEnableWrapping = FALSE;

	m_clrActiveTabBk = (COLORREF) -1;
	m_clrActiveTabFg = (COLORREF) -1;

	m_nTabBorderSize = DEFAULT_TAB_BORDER_SIZE;

	m_nTabsHeight = 0;

	// in-place editing
	m_pInPlaceEdit = NULL;
	m_iEditedTab = -1;

	m_bIsInPlaceEdit = FALSE;
	m_bHideSingleTab = FALSE;
	m_bSingleTabIsStatic = FALSE;

	m_bLabelNoPrefix = FALSE;
	m_bDrawNameInUpperCase = FALSE;
	m_bActivateLastVisibleTab = FALSE;
	m_bHighLightTabs = FALSE;

	m_bIsAutoColor = FALSE;
	m_bIsDefaultAutoColor = TRUE;

	m_bIsDlgControl = FALSE;

	m_iLastActiveTab = -1;
	m_bActivateLastActiveTab = FALSE;

	m_bIsMDITab = FALSE;
	m_bIsPropertySheetTab = FALSE;
	m_bIsParentFocused = FALSE;
	m_bSetActiveTabFired = FALSE;
	m_bUserSelectedTab = FALSE;

	m_bSetActiveTabByMouseClick = FALSE;
	m_bActivateTabOnRightClick = FALSE;

	m_bWindowPosChanged = FALSE;

	m_pToolTip = NULL;
	m_pToolTipClose = NULL;

	m_hFontCustom = NULL;
	m_bCaptionFont = FALSE;
}
//***************************************************************************************
CBCGPBaseTabWnd::~CBCGPBaseTabWnd()
{
}

BEGIN_MESSAGE_MAP(CBCGPBaseTabWnd, CBCGPWnd)
	//{{AFX_MSG_MAP(CBCGPBaseTabWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_MBUTTONUP()
	//}}AFX_MSG_MAP
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnNeedTipText)
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseTabWnd message handlers
//***************************************************************************************
void CBCGPBaseTabWnd::CleanUp ()
{
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if (pTab->m_bIsNewTab)
		{
			delete pTab;
			continue;
		}

		// we need to delete tab info only in the case the contained window is not
		// derived from CBCGPControlBar, because CBCGPControlBar detects that it's 
		// tabbed while processing OnDestroy and removes itself from the parent tabbed
		// window.
 		if (pTab->m_pWnd->m_hWnd == NULL || ::IsWindow (pTab->m_pWnd->m_hWnd))
		{
			BOOL bDeleteTabInfo = !pTab->m_pWnd->IsKindOf (RUNTIME_CLASS (CBCGPControlBar));

			if (m_bAutoDestoyWindow)
			{
				pTab->m_pWnd->DestroyWindow ();
			}

			if (bDeleteTabInfo || !m_bAutoDestoyWindow)
			{
				delete pTab;
			}
		}
	}

	CBCGPTooltipManager::DeleteToolTip (m_pToolTip);
	CBCGPTooltipManager::DeleteToolTip (m_pToolTipClose);

	m_arTabs.RemoveAll ();
	m_arTabIndexs.RemoveAll();

	m_iTabsNum = 0;
	m_iActiveTab = -1;
}
//***************************************************************************************
void CBCGPBaseTabWnd::AddTab (CWnd* pNewWnd, LPCTSTR lpszName, UINT uiImageId, BOOL bDetachable)
{
	if (pNewWnd->GetSafeHwnd () != NULL && pNewWnd->GetDlgCtrlID () == -1)
	{
		ASSERT (FALSE);
		TRACE0 ("Unable to add a new tab with control bar ID -1. \n");
		return;
	}

	CWnd* pWndToAdd = CreateWrapper (pNewWnd, lpszName, bDetachable);
	ASSERT_VALID (pWndToAdd);

	InsertTab (pWndToAdd, lpszName, -1, uiImageId, bDetachable);
}
//***************************************************************************************
void CBCGPBaseTabWnd::AddTab (CWnd* pTabWnd, UINT uiResTabLabel, UINT uiImageId, BOOL bDetachable)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pTabWnd);

	if (pTabWnd->GetDlgCtrlID () == -1)
	{
		ASSERT (FALSE);
		TRACE0 ("Unable to add a new tab with control bar ID -1. \n");
		return;
	}

	CString strLabel;
	strLabel.LoadString (uiResTabLabel);

	CWnd* pWndToAdd = CreateWrapper (pTabWnd, strLabel, bDetachable);
	ASSERT_VALID (pWndToAdd);

	AddTab (pWndToAdd, strLabel, uiImageId, bDetachable);
}
//***************************************************************************************
void CBCGPBaseTabWnd::InsertTab(CWnd* pNewWnd, LPCTSTR lpszTabLabel, int nInsertAt, UINT uiImageId, BOOL bDetachable)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pNewWnd);
	ASSERT (lpszTabLabel != NULL);

	CWnd* pWndToAdd = CreateWrapper (pNewWnd, lpszTabLabel, bDetachable);
	ASSERT_VALID (pWndToAdd);

	if (!IsWindowVisible ())
	{
		ShowWindow (SW_SHOW);
	}

	if (nInsertAt < 0 || nInsertAt > m_iTabsNum)
	{
		nInsertAt = m_iTabsNum;
	}

	if (IsNewTabEnabled() && m_iTabsNum > 0 && nInsertAt == m_iTabsNum)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [m_iTabsNum - 1];
		ASSERT_VALID (pTab);

		if (pTab->m_bIsNewTab)
		{
			nInsertAt--;
		}
	}

	CWnd* pActiveWnd = GetActiveWnd();

	// can't detach non-docking control bar derived objects
	if (!pWndToAdd->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)))
	{
		bDetachable = FALSE;
	}

	m_arTabs.InsertAt (nInsertAt, new CBCGPTabInfo (
		lpszTabLabel, uiImageId, pWndToAdd, m_nNextTabID, bDetachable));

	m_iTabsNum++;

	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		CRect rectEmpty (0, 0, 0, 0);
		m_pToolTip->AddTool (this, 
			m_bCustomToolTips ? LPSTR_TEXTCALLBACK : lpszTabLabel,
			&rectEmpty, m_nNextTabID);
	}

	if (m_pToolTipClose->GetSafeHwnd () != NULL)
	{
		CRect rectEmpty (0, 0, 0, 0);
		m_pToolTipClose->AddTool (this, LPSTR_TEXTCALLBACK, &rectEmpty, m_nNextTabID);
	}

	m_nNextTabID ++;

	m_arTabIndexs.RemoveAll();

	OnChangeTabs ();
	RecalcLayout ();

	if (m_iTabsNum == 1)
	{
		if (!m_bIsMDITab)
		{
			//----------------------------------------
			// First tab automatically becames active:
			//----------------------------------------
			SetActiveTab (0);
		}
	}
	else
	{
		m_iLastActiveTab = m_iActiveTab;

		if (m_iActiveTab == nInsertAt)
		{
			m_iLastActiveTab++;

			if (m_bHideInactiveWnd && pActiveWnd != NULL)
			{
				pActiveWnd->ShowWindow(SW_HIDE);
			}

			pWndToAdd->ShowWindow (SW_SHOWNORMAL);
		}
		else if (m_bHideInactiveWnd)
		{
			if (pWndToAdd->GetSafeHwnd () != NULL)
			{
				pWndToAdd->ShowWindow(SW_HIDE);
			}
		}
	}

	if (!m_bHideInactiveWnd && pActiveWnd->GetSafeHwnd() != NULL && !m_bIsMDITab)
	{
		pActiveWnd->BringWindowToTop ();
	}
}
//***************************************************************************************
void CBCGPBaseTabWnd::InsertTab(CWnd* pNewWnd, UINT uiResTabLabel, int nInsertAt, 
							UINT uiImageId, BOOL bDetachable)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pNewWnd);

	if (pNewWnd->GetDlgCtrlID () == -1)
	{
		ASSERT (FALSE);
		TRACE0 ("Unable to add a new tab with control bar ID -1. \n");
		return;
	}

	CString strLabel;
	strLabel.LoadString (uiResTabLabel);

	CWnd* pWndToAdd = CreateWrapper (pNewWnd, strLabel, bDetachable);
	ASSERT_VALID (pWndToAdd);

	InsertTab (pWndToAdd, strLabel, nInsertAt, uiImageId, bDetachable);
}
//***************************************************************************************
BOOL CBCGPBaseTabWnd::RemoveTab (int iTab, BOOL bRecalcLayout)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("RemoveTab: illegal tab number %d\n"), iTab);
		return FALSE;
	}

	if (m_iTabsNum == 1)
	{
		RemoveAllTabs ();
		return TRUE;
	}

	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		m_pToolTip->DelTool (this, pTab->m_iTabID);
	}

	if (m_pToolTipClose->GetSafeHwnd () != NULL)
	{
		m_pToolTipClose->DelTool (this, pTab->m_iTabID);
	}

	//----------------------------
	// Detach tab from collection:
	//----------------------------
	m_arTabs.RemoveAt (iTab);
	m_iTabsNum --;

	m_arTabIndexs.RemoveAll();

	//-----------------------------------
	// Destroy tab window and delete tab:
	//-----------------------------------
	if (m_bAutoDestoyWindow)
	{
		ASSERT_VALID (pTab->m_pWnd);
		pTab->m_pWnd->DestroyWindow ();
	}

	delete pTab;

	int iActiveTab = m_iActiveTab;
	if (m_iActiveTab >= iTab)
	{
		if (m_bActivateLastVisibleTab)
		{
			GetLastVisibleTab (iActiveTab);
		}
		else
		{
			// Find the next best tab to be activated
			for (int i = m_iTabsNum - 1; i >= 0; --i)
			{
				CBCGPTabInfo* pNextActiveTab = (CBCGPTabInfo*) m_arTabs [i];
				ASSERT_VALID (pNextActiveTab);

				if (i < iTab && iActiveTab >= 0 && iActiveTab < m_iTabsNum)
				{
					break;
				}

				if (pNextActiveTab->m_bVisible)
				{
					iActiveTab	= i;
				}
			}
		}
		
		m_iActiveTab = -1;
	}

	OnChangeTabs ();

	if (bRecalcLayout)
	{
		RecalcLayout ();
		if (iActiveTab != -1)
		{
			if (m_bActivateLastActiveTab && (m_iLastActiveTab != -1))
			{
				int iLastActiveTab = m_iLastActiveTab;
				if (iTab < m_iLastActiveTab)
				{
					iLastActiveTab = m_iLastActiveTab -1;
				}

				int iTabToActivate = -1; 
				GetFirstVisibleTab (iLastActiveTab, iTabToActivate);

				SetActiveTab (iTabToActivate);
			}
			else
			{
				int iTabToActivate = -1; 
				GetFirstVisibleTab (iActiveTab, iTabToActivate);
				SetActiveTab (iTabToActivate);
			}

			FireChangeActiveTab (m_iActiveTab);
		}
	}

	return TRUE;
}
//***************************************************************************************
void CBCGPBaseTabWnd::RemoveAllTabs ()
{
	m_iActiveTab = -1;
	m_nNextTabID = 1;

	while (m_iTabsNum > 0)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [m_iTabsNum - 1];
		ASSERT_VALID (pTab);

		m_iTabsNum--;

		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			m_pToolTip->DelTool (this, pTab->m_iTabID);
		}

		if (m_pToolTipClose->GetSafeHwnd () != NULL)
		{
			m_pToolTipClose->DelTool (this, pTab->m_iTabID);
		}

		if (m_bAutoDestoyWindow)
		{
			pTab->m_pWnd->DestroyWindow ();
		}

		delete pTab;
	}

	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		ASSERT (m_pToolTip->GetToolCount () == 0);
	}

	m_arTabs.RemoveAll ();
	m_arTabIndexs.RemoveAll();

	OnChangeTabs ();

	RecalcLayout ();
	FireChangeActiveTab ((UINT)-1);
}
//***************************************************************************************
int CBCGPBaseTabWnd::GetVisibleTabsNum () const
{
	int	nCount	= 0;

	for (int i = 0; i < m_iTabsNum; i++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);
		
		if (pTab->m_bVisible)
		{
			nCount++;
		}
	}

	return nCount;
}
//***************************************************************************************
BOOL CBCGPBaseTabWnd::ShowTab(int iTab, BOOL bShow /*= TRUE*/, BOOL bRecalcLayout /*= TRUE*/, 
							  BOOL bActivate)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("ShowTab: illegal tab number %d\n"), iTab);
		return FALSE;
	}

	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	if (pTab->m_bVisible == bShow)
	{
		return TRUE;
	}

	int	nVisibleCount = GetVisibleTabsNum ();
	pTab->m_bVisible = bShow;

	int iActiveTab = (bShow ? m_iActiveTab : -1);

	if (!bShow)
	{
		//----------
		// Hide tab:
		//----------
		if (m_bHideInactiveWnd)
		{
			ASSERT_VALID (pTab->m_pWnd);
			pTab->m_pWnd->ShowWindow(SW_HIDE);
		}

		if (iTab == m_iActiveTab)
		{
			// Find the next best tab to be activated
			for (int i = m_iTabsNum - 1; i >= 0; --i)
			{
				CBCGPTabInfo* pNextActiveTab = (CBCGPTabInfo*) m_arTabs [i];
				ASSERT_VALID (pNextActiveTab);

				if (i < iTab && iActiveTab >= 0)
				{
					break;
				}

				if (pNextActiveTab->m_bVisible)
				{
					iActiveTab	= i;
				}
			}

			m_iActiveTab = -1;
		}
	}

	// If there was no tab visible, activate this first one
	if (bShow && nVisibleCount == 0)
	{
		iActiveTab	= iTab;
	}

	if (bRecalcLayout)
	{
		RecalcLayout ();
	}

	if (iActiveTab >= 0 && !bShow && m_iActiveTab == -1 || bActivate || nVisibleCount == 0)
	{
		SetActiveTab (iActiveTab);
		FireChangeActiveTab (m_iActiveTab);
	}
	
	return TRUE;
}
//****************************************************************************************
CWnd* CBCGPBaseTabWnd::GetTabWnd (int iTab) const
{
	if (iTab >= 0 && iTab < m_iTabsNum)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
		ASSERT_VALID (pTab);

		return pTab->m_pWnd;
	}
	else
	{
		return NULL;
	}
}
//*********************************************************************************
CWnd* CBCGPBaseTabWnd::GetTabWndNoWrapper (int iTab) const
{
	if (iTab >= 0 && iTab < m_iTabsNum)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
		ASSERT_VALID (pTab);

		CBCGPDockingCBWrapper* pWrapper = 
					DYNAMIC_DOWNCAST (CBCGPDockingCBWrapper, pTab->m_pWnd);
		if (pWrapper != NULL)
		{
			return pWrapper->GetWrappedWnd ();
		}

		return pTab->m_pWnd;
	}
	else
	{
		return NULL;
	}
}
//*********************************************************************************
BOOL CBCGPBaseTabWnd::GetTabRect (int iTab, CRect& rect) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}
	
	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);
	
	if (!pTab->m_bVisible)
	{
		rect.SetRectEmpty ();
		return FALSE;
	}
	
	rect = pTab->m_rect;
	return TRUE;
}
//*********************************************************************************
BOOL CBCGPBaseTabWnd::GetTabCloseButtonRect (int iTab, CRect& rect, BOOL* pbIsHighlighted, BOOL* pbIsPressed) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}
	
	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);
	
	if (!pTab->m_bVisible)
	{
		rect.SetRectEmpty ();
		return FALSE;
	}
	
	rect = pTab->m_rectClose;

	if (pbIsHighlighted != NULL)
	{
		*pbIsHighlighted = pTab->m_bIsCloseHighlighted;
	}

	if (pbIsPressed != NULL)
	{
		*pbIsPressed = pTab->m_bIsClosePressed;
	}

	return TRUE;
}
//********************************************************************************
int CBCGPBaseTabWnd::GetTabID (int iTab) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return -1;
	}
	
	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	return pTab->m_iTabID;
}
//********************************************************************************
BOOL CBCGPBaseTabWnd::IsTabNew (int iTab) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	return pTab->m_bIsNewTab;
}
//*********************************************************************************
BOOL CBCGPBaseTabWnd::IsFirstTab(int iTab) const
{
	if (m_arTabIndexs.GetSize() != m_arTabs.GetSize())
	{
		return iTab == 0;
	}

	return m_arTabIndexs[0] == iTab;
}
//*********************************************************************************
BOOL CBCGPBaseTabWnd::GetTabLabel (int iTab, CString& strLabel) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}
	
	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);
	
	strLabel = pTab->m_bIconOnly ? _T("") : pTab->m_strText;

	return TRUE;
}
//*********************************************************************************
BOOL CBCGPBaseTabWnd::SetTabLabel (int iTab, const CString& strLabel)
{
	if (iTab < 0 || iTab >= m_iTabsNum || strLabel.IsEmpty ())
	{
		return FALSE;
	}
	
	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);
	
	pTab->m_strText = strLabel;
	
	if (m_pToolTip->GetSafeHwnd () != NULL)
	{
		if (m_bCustomToolTips)
		{
			CToolInfo info;

			m_pToolTip->GetToolInfo (info, this, pTab->m_iTabID);
			m_pToolTip->DelTool (this, pTab->m_iTabID);

			m_pToolTip->AddTool (this,
				LPSTR_TEXTCALLBACK, &info.rect, pTab->m_iTabID);
		}
		else
		{
			m_pToolTip->UpdateTipText (strLabel, this, pTab->m_iTabID);
		}
	}

	if (pTab->m_pWnd->GetSafeHwnd () != NULL)
	{
		pTab->m_pWnd->SetWindowText (strLabel);
	}

	RecalcLayout();

	if (iTab == m_iActiveTab)
	{
		//--------------------------------------------------
		// Set text to the parent frame/docking control bar:
		//--------------------------------------------------
		CBCGPBaseTabbedBar* pTabControlBar = 
			DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, GetParent ());
		if (pTabControlBar != NULL) // tabbed dock bar - redraw caption only in this case
		{
			// miniframe will take the text from the tab control bar
			if (pTabControlBar->CanSetCaptionTextToTabName ())
			{
				pTabControlBar->SetWindowText (strLabel);
			}

			CWnd* pWndToUpdate = pTabControlBar;
			if (!pTabControlBar->IsDocked ())
			{
				pWndToUpdate = pTabControlBar->GetParent ();
			}

			if (pWndToUpdate != NULL)
			{
				pWndToUpdate->RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
			}
		}
	}

	return TRUE;
}//********************************************************************************
UINT CBCGPBaseTabWnd::GetTabIcon (int iTab) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return (UINT) -1;
	}

	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	return pTab->m_uiIcon;
}
//*********************************************************************************
BOOL CBCGPBaseTabWnd::SetTabIcon (int iTab, UINT uiIcon)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	if (pTab->m_hIcon != NULL)
	{
		::DestroyIcon (pTab->m_hIcon);
	}

	if (pTab->m_hGrayIcon != NULL)
	{
		::DestroyIcon (pTab->m_hGrayIcon);
	}

	pTab->m_uiIcon = uiIcon;
	pTab->m_hIcon = NULL;
	pTab->m_hGrayIcon = NULL;

	return TRUE;
}
//********************************************************************************
HICON CBCGPBaseTabWnd::GetTabHicon (int iTab) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return NULL;
	}

	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	if (!CBCGPVisualManager::GetInstance()->IsAutoGrayscaleImages())
	{
		return pTab->m_hIcon;
	}

	if (pTab->m_hGrayIcon == NULL)
	{
		pTab->m_hGrayIcon = globalUtils.GrayIcon(pTab->m_hIcon);
	}

	return pTab->m_hGrayIcon == NULL ? pTab->m_hIcon : pTab->m_hGrayIcon;
}
//*********************************************************************************
BOOL CBCGPBaseTabWnd::SetTabHicon (int iTab, HICON hIcon)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	if (pTab->m_hIcon != NULL)
	{
		::DestroyIcon (pTab->m_hIcon);
	}

	if (pTab->m_hGrayIcon != NULL)
	{
		::DestroyIcon (pTab->m_hGrayIcon);
		pTab->m_hGrayIcon = NULL;
	}

	if (hIcon != NULL)
	{
		pTab->m_hIcon = ::CopyIcon (hIcon);
	}
	else
	{
		pTab->m_hIcon = NULL;
	}

	pTab->m_uiIcon = (UINT)-1;

	m_sizeImage.cx = max (m_sizeImage.cx, globalData.m_sizeSmallIcon.cx);
	m_sizeImage.cy = max (m_sizeImage.cy, globalData.m_sizeSmallIcon.cy);

	SetTabsHeight ();

	return TRUE;
}
//******************************************************************************************
BOOL CBCGPBaseTabWnd::IsTabIconOnly (int iTab) const
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}
	
	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);
	
	return pTab->m_bIconOnly;
};
//*********************************************************************************
BOOL CBCGPBaseTabWnd::SetTabIconOnly (int iTab, BOOL bIconOnly, BOOL bAlwaysShowToolTip)
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}
	
	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);
	
	pTab->m_bIconOnly = bIconOnly;
	pTab->m_bAlwaysShowToolTip = bAlwaysShowToolTip;
	
	RecalcLayout();
	return TRUE;
}
//*********************************************************************************
BOOL CBCGPBaseTabWnd::IsTabDetachable (int iTab) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	return pTab->m_bIsDetachable;
}
//******************************************************************************************
BOOL CBCGPBaseTabWnd::EnableTabDetach (int iTab, BOOL bEnable)
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	pTab->m_bIsDetachable = bEnable;
	return TRUE;
}
//******************************************************************************************
CWnd* CBCGPBaseTabWnd::GetActiveWnd () const
{
	return m_iActiveTab == -1 ? 
		NULL : 
		((CBCGPTabInfo*) m_arTabs [m_iActiveTab])->m_pWnd;
}
//***************************************************************************************
int CBCGPBaseTabWnd::GetTabFromPoint (CPoint& pt) const
{
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if (pTab->m_bVisible && pTab->m_rect.PtInRect (pt))
		{
			return i;
		}
	}

	return -1;
}
//***************************************************************************************
int CBCGPBaseTabWnd::GetTabFromHwnd (HWND hwnd) const
{
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if (pTab->m_pWnd == NULL)
		{
			continue;
		}

		if (pTab->m_pWnd->GetSafeHwnd () == hwnd)
		{
			return i;
		}

		CBCGPDockingCBWrapper* pWrapper = DYNAMIC_DOWNCAST (CBCGPDockingCBWrapper, 
																pTab->m_pWnd);
		if (pWrapper != NULL)
		{
			CWnd* pWrappedWnd = pWrapper->GetWrappedWnd ();
			if (pWrappedWnd != NULL && pWrappedWnd->GetSafeHwnd () == hwnd)
			{
				return i;
			}
		}
	}

	return -1;
}
//***************************************************************************************
int CBCGPBaseTabWnd::GetTabByID (int id) const
{
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if (pTab->m_iTabID == id)
		{
			return i;
		}
	}

	return -1;
}
//********************************************************************************
BOOL CBCGPBaseTabWnd::IsTabVisible(int iTab) const
{
	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		TRACE(_T("IsTabVisible: illegal tab number %d\n"), iTab);
		return FALSE;
	}
	
	const CBCGPTabInfo* pTab = (const CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);
	
	return pTab->m_bVisible;
}
//***************************************************************************************
int CBCGPBaseTabWnd::GetTabNumberToDetach (int nTabNum) const
{
	return (nTabNum == -1 ? m_iActiveTab : nTabNum);
}
//***************************************************************************************
BOOL CBCGPBaseTabWnd::DetachTab (BCGP_DOCK_METHOD dockMethod, int nTabNum, BOOL bHide)
{
	int nTabToDetach = GetTabNumberToDetach (nTabNum);
	if (nTabToDetach < 0)
	{
		return FALSE;
	}

	ASSERT (nTabToDetach >= 0 && nTabToDetach < GetTabsNum ());

	if (!IsTabDetachable (nTabToDetach))
	{
		return FALSE;
	}
	
	CBCGPControlBar* pDockingBar = 
		DYNAMIC_DOWNCAST (CBCGPControlBar, GetTabWnd (nTabToDetach));
	if (pDockingBar == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pDockingBar);

	if (!pDockingBar->CanFloat ())
	{
		return FALSE;
	}

	CRect rectFloat; rectFloat.SetRectEmpty ();

	if (!pDockingBar->OnBeforeFloat (rectFloat, dockMethod))
	{
		return FALSE;
	}

	if (dockMethod == BCGP_DM_MOUSE)
	{
		CPoint ptMouse;
		GetCursorPos (&ptMouse);
		CPoint ptHotDelta = m_ptHot - ptMouse;
		CSize szSencitivity = CBCGPDockingControlBar::GetDragSencitivity ();

		if (abs (ptHotDelta.x) < szSencitivity.cx && abs (ptHotDelta.y) < szSencitivity.cy)
		{
			return FALSE;
		}
	}

	if (pDockingBar != NULL && pDockingBar->CanBeAttached ())
	{
		BCGP_DOCK_TYPE dockType = pDockingBar->GetDockMode ();

		CBCGPBaseTabbedBar* pParent = DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, GetParent ());
		CBCGPMiniFrameWnd* pMiniFrame = pParent == NULL ? NULL : pParent->GetParentMiniFrame ();
		
		if (pParent != NULL)
		{
			if (!pParent->CanFloat () && GetVisibleTabsNum () == 1)
			{
				return FALSE;
			}

			if (pDockingBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)))
			{
				((CBCGPDockingControlBar*) pDockingBar)->EnableGripper (TRUE);
			}

			if (!bHide && (dockType & BCGP_DT_STANDARD) != 0 && dockMethod == BCGP_DM_MOUSE)
			{
				bHide = TRUE;
			}

			if (/*!bHide && */nTabToDetach != m_iActiveTab)
			{
				pDockingBar->ShowWindow (SW_SHOW);
			}
				
			pParent->FloatTab (pDockingBar, nTabToDetach, dockMethod, bHide);

			if (GetTabsNum () == 0)
			{
				if (pMiniFrame != NULL)
				{
					pMiniFrame->RemoveControlBar (pParent, TRUE);
				}
				else 
				{
					if (pParent->AllowDestroyEmptyTabbedBar ())
					{
						pParent->DestroyWindow ();
					}
					else
					{
						ShowWindow (SW_HIDE);
					}
				}
			}
			else if (GetVisibleTabsNum () == 0)
			{
				pParent->ShowControlBar (FALSE, FALSE, FALSE);
			}
			RecalcLayout ();
		}

		CBCGPMiniFrameWnd* pParentFrame = pDockingBar->GetParentMiniFrame ();
		if ((dockType & BCGP_DT_STANDARD) != 0 && dockMethod == BCGP_DM_MOUSE)
		{
			if (pParentFrame != NULL)
			{
				ReleaseCapture ();
				pParentFrame->SetFocus ();
				GetParent ()->SendMessage (WM_IDLEUPDATECMDUI);
			}
		}

		m_bReadyToDetach = FALSE;

		HWND hWndToDestroy = NULL;
		if (GetTabsNum () == 1 && m_bHideSingleTab)
		{
			CRect rectWnd;
			pParent->GetWindowRect (rectWnd);

			CBCGPDockingControlBar* pDockingBar = 
				DYNAMIC_DOWNCAST (CBCGPDockingControlBar, GetTabWnd (0));	

			pParent->InsertControlBar (pDockingBar, pParent);
			pParent->ReplaceControlBar (pDockingBar, dockMethod);
			pDockingBar->SetBarAlignment (pParent->GetCurrentAlignment ());
			RemoveTab (0);
			CWnd* pNewParent = (pMiniFrame == NULL) ? pParent->GetDockSite () 
													: pMiniFrame;

			pDockingBar->EnableGripper (TRUE);
			pNewParent->ScreenToClient (rectWnd);

			pDockingBar->SetParent (pNewParent);
			pDockingBar->SetWindowPos (NULL, rectWnd.left, rectWnd.top, 
										rectWnd.Width (), rectWnd.Height (),
										SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
		
			pDockingBar->ShowControlBar (TRUE, FALSE, FALSE);
			pParent->ShowWindow (SW_HIDE);
			pDockingBar->GetParent ()->InvalidateRect (NULL);
			pDockingBar->GetParent ()->UpdateWindow ();
			hWndToDestroy = pParent->GetSafeHwnd ();
		}

		if (dockMethod == BCGP_DM_MOUSE && pParentFrame != NULL)
		{
			ASSERT_VALID (pParentFrame);

			if ((dockType & BCGP_DT_STANDARD) != 0)
			{
				pParentFrame->EnterDragMode (hWndToDestroy);
				pParentFrame->MoveDragFrame ();
				hWndToDestroy = NULL;
			}
			else if ((dockType & BCGP_DT_IMMEDIATE) != 0)
			{
				pParentFrame->SetFocus ();
			}
		}

		if (CBCGPTabbedControlBar::m_bDestroyUnused && hWndToDestroy != NULL && ::IsWindow(hWndToDestroy))
		{
			::DestroyWindow(hWndToDestroy);
		}

		return TRUE;
	}

	return FALSE;
}
//***************************************************************************************
void CBCGPBaseTabWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (GetTabsNum() == 1 && IsSingleTabStatic())
	{
		return;
	}

	m_bWindowPosChanged = FALSE;

	CBCGPWnd::OnLButtonDown(nFlags, point);

	int nTab = GetTabFromPoint(point);
	if (nTab >= 0)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*)m_arTabs[nTab];
		ASSERT_VALID(pTab);
	
		if (pTab->m_rectClose.PtInRect(point))
		{
			pTab->m_bIsClosePressed = TRUE;
			RedrawWindow(pTab->m_rectClose);
			return;
		}
	}

	m_iHighlighted = nTab;
	int iHighlighted = m_iHighlighted;

	if (iHighlighted >= 0)
	{
		BOOL bTabWasMoved = FALSE;

		if (!ActivateOnBtnUp ())
		{
			nTab = GetTabFromPoint (point);

			if (nTab >= 0 && nTab != m_iActiveTab)
			{
				if (IsTabNew(nTab))
				{
					FireNewTab();
					return;
				}

				m_iLastActiveTab = m_iActiveTab;

				m_bSetActiveTabFired = FALSE;
				m_bSetActiveTabByMouseClick = TRUE;
				m_bUserSelectedTab = TRUE;

				if (!SetActiveTab (nTab))
				{
					m_bSetActiveTabFired = TRUE;
					m_bSetActiveTabByMouseClick = FALSE;
					m_bUserSelectedTab = FALSE;
					m_bWindowPosChanged = FALSE;
					return;
				}

				m_bSetActiveTabByMouseClick = FALSE;
				m_bUserSelectedTab = FALSE;

				if (!m_bSetActiveTabFired)
				{
					FireChangeActiveTab (m_iActiveTab);
				}

				m_bSetActiveTabFired = FALSE;
			}
			else if (nTab == m_iActiveTab)
			{
				//-----------------------------
				// Set focus to the tab window:
				//-----------------------------
				CWnd* pWnd = GetTabWnd (nTab);
				if (pWnd->GetSafeHwnd () != NULL)
				{
					pWnd->SetFocus ();
				}
			}

			bTabWasMoved = (nTab != GetTabFromPoint (point));

			m_iHighlighted = -1;
			ReleaseCapture ();
		}
		else
		{
			m_iPressed = m_iHighlighted;
		}

		if (iHighlighted != m_iActiveTab)
		{
			InvalidateTab (iHighlighted);
		}

		if (!bTabWasMoved && !m_bWindowPosChanged)
		{
			EnterDragMode ();
		}
	}

	m_bWindowPosChanged = FALSE;
}
//***************************************************************************************
void CBCGPBaseTabWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
	CBCGPWnd::OnRButtonDown (nFlags, point);

	if (m_bActivateTabOnRightClick)
	{
		int nTab = GetTabFromPoint (point);
		if (nTab >= 0 && nTab != m_iActiveTab)
		{
			m_bSetActiveTabFired = FALSE;
			m_bSetActiveTabByMouseClick = TRUE;
			m_bUserSelectedTab = FALSE;

			if (!SetActiveTab (nTab))
			{
				m_bSetActiveTabByMouseClick = FALSE;
				m_bUserSelectedTab = FALSE;
				return;
			}

			m_bSetActiveTabByMouseClick = FALSE;
			m_bUserSelectedTab = FALSE;

			if (!m_bSetActiveTabFired)
			{
				FireChangeActiveTab (m_iActiveTab);
			}

			m_bSetActiveTabFired = FALSE;
		}
		else if (nTab == m_iActiveTab)
		{
			//-----------------------------
			// Set focus to the tab window:
			//-----------------------------
			CWnd* pWnd = GetTabWnd (nTab);
			if (pWnd->GetSafeHwnd () != NULL)
			{
				pWnd->SetFocus ();
			}
		}
	}
}
//***************************************************************************************
void CBCGPBaseTabWnd::EnterDragMode ()
{
	m_bReadyToDetach = TRUE;
	SetCapture ();
	GetCursorPos (&m_ptHot);

	CRect rectTab;
	GetTabRect (m_iActiveTab, rectTab);
	m_iTabBeforeDrag = m_iActiveTab;
	ClientToScreen (rectTab);
	m_nOffsetFromTabLeft = m_ptHot.x - rectTab.left;
}
//***************************************************************************************
void CBCGPBaseTabWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CBCGPWnd::OnLButtonDblClk(nFlags, point);

	if (m_bIsInPlaceEdit)
	{
		int iTab = GetTabFromPoint (point); 
		if (iTab  == GetActiveTab())
		{
			if (StartRenameTab (iTab))
				return;
		}
	}
	else
	{
		CWnd* pWndTarget = FindTargetWnd (point);
		if (pWndTarget == NULL)
		{
			DetachTab (BCGP_DM_DBL_CLICK);
		}
	}	
}
//***************************************************************************************
void CBCGPBaseTabWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_iHighlighted >= 0 && m_iHighlighted < m_arTabs.GetSize())
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*)m_arTabs[m_iHighlighted];
		ASSERT_VALID(pTab);

		if (pTab->m_bIsClosePressed)
		{
			pTab->m_bIsClosePressed = FALSE;
			pTab->m_bIsCloseHighlighted = FALSE;

			RedrawWindow(pTab->m_rectClose);
	
			if (pTab->m_rectClose.PtInRect(point))
			{
				CWnd* pWndHighlighted = pTab->m_pWnd;
				if (pWndHighlighted != NULL)
				{
					OnClickCloseButton(pWndHighlighted);
				}
			}
			return;
		}
	}

	if (m_iTabBeforeDrag != m_iActiveTab)
	{
		CWnd* pWndParent = GetParent ();
		ASSERT_VALID (pWndParent);

		pWndParent->SendMessage (BCGM_ON_MOVE_TAB, m_iTabBeforeDrag, m_iActiveTab);
		if (pWndParent->IsKindOf (RUNTIME_CLASS (CBCGPBaseTabbedBar)) ||
			pWndParent->IsKindOf (RUNTIME_CLASS (CBCGPMainClientAreaWnd)))
		{
			pWndParent = BCGPGetParentFrame (pWndParent);
			if (pWndParent != NULL)
			{
				pWndParent->SendMessage (BCGM_ON_MOVE_TAB, m_iTabBeforeDrag, m_iActiveTab);
			}
		}
	}

	if (m_bReadyToDetach)
	{
		m_bReadyToDetach = FALSE;
		ReleaseCapture ();

		if (!ActivateOnBtnUp ())
		{
			m_iPressed = -1;
			m_iHighlighted = -1;
		}
	}

	if (ActivateOnBtnUp ())
	{
		bool bNewActiveTab = m_iActiveTab != m_iHighlighted;

		if (m_iHighlighted == m_iPressed &&
			m_iHighlighted >= 0 && m_iHighlighted != m_iActiveTab)
		{
			m_iLastActiveTab = m_iActiveTab;
			m_bSetActiveTabByMouseClick = TRUE;
			m_bUserSelectedTab = FALSE;

			if (!SetActiveTab (m_iHighlighted))
			{
				m_bSetActiveTabByMouseClick = FALSE;
				m_bUserSelectedTab = FALSE;
				m_iPressed = -1;

				if (!IsOneNoteStyle ())
				{
					m_iHighlighted = -1;
				}

				ReleaseCapture ();
				return;
			}

			FireChangeActiveTab (m_iActiveTab);
			m_bSetActiveTabByMouseClick = FALSE;
			m_bUserSelectedTab = FALSE;
		}

		int iHighlighted = m_iHighlighted;
		int iPressed = m_iPressed;

		m_iPressed = -1;

		if (!IsOneNoteStyle ())
		{
			m_iHighlighted = -1;
		}

		ReleaseCapture ();

		if (bNewActiveTab)
		{
			InvalidateTab (iHighlighted);

			if (iPressed != iHighlighted)
			{
				InvalidateTab (iPressed);
			}
		}
	}

	if (IsOneNoteStyle ())
	{
		CRect rectTabAreaTop;
		CRect rectTabAreaBottom;

		GetTabArea (rectTabAreaTop, rectTabAreaBottom);

		if (!rectTabAreaTop.IsRectEmpty ())
		{
			InvalidateRect (rectTabAreaTop, FALSE);
		}

		if (!rectTabAreaBottom.IsRectEmpty ())
		{
			InvalidateRect (rectTabAreaBottom, FALSE);
		}

		UpdateWindow ();
	}

	CBCGPWnd::OnLButtonUp(nFlags, point);
}
//***************************************************************************************
void CBCGPBaseTabWnd::OnClickCloseButton (CWnd* pWndActive)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pWndActive);

	pWndActive->SendMessage (WM_CLOSE);
}
//***************************************************************************************
void CBCGPBaseTabWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	CBCGPWnd::OnMouseMove(nFlags, point);

	if (GetTabsNum() == 1 && IsSingleTabStatic())
	{
		return;
	}

	int iPrevHighlighted = m_iHighlighted;
	
	if (IsTabCloseButton() && m_iHighlighted >= 0 && m_iHighlighted < m_iTabsNum)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*)m_arTabs[m_iHighlighted];
		ASSERT_VALID(pTab);
	
		BOOL bCloseHighlighted = pTab->m_bIsCloseHighlighted;
		pTab->m_bIsCloseHighlighted = pTab->m_rectClose.PtInRect(point);

		if (bCloseHighlighted != pTab->m_bIsCloseHighlighted)
		{
			RedrawWindow(pTab->m_rectClose);
		}
	}

	if (m_iHighlighted >= 0 && m_iPressed < 0 && !m_bReadyToDetach)
	{
		CPoint pt = point;
		ClientToScreen  (&pt);
		CWnd* pWnd = CWnd::WindowFromPoint (pt);
		if (pWnd != NULL && pWnd->GetSafeHwnd () != GetSafeHwnd ())
		{
			ReleaseCapture ();
			m_iHighlighted = -1;
			InvalidateTab (iPrevHighlighted);
			OnChangeHighlightedTab(iPrevHighlighted);
			return;
		}
	}

	m_iHighlighted = GetTabFromPoint (point);

	if (m_iPressed >= 0 && m_iHighlighted != m_iPressed)
	{
		m_iHighlighted = -1;
	}

	if (m_iHighlighted != iPrevHighlighted && (m_bHighLightTabs || IsTabCloseButton()))
	{
		if (iPrevHighlighted < 0)
		{
			if (m_iHighlighted >= 0)
			{
				SetCapture ();
			}
		}
		else
		{
			if (m_iHighlighted < 0 && m_iPressed < 0)
			{
				if (!m_bReadyToDetach)
				{
					ReleaseCapture ();
				}
			}
		}

		OnChangeHighlightedTab(iPrevHighlighted);

		InvalidateTab (m_iHighlighted);
		InvalidateTab (iPrevHighlighted);
	}

	if (m_bReadyToDetach)
	{
		int nNumTabs = m_iTabsNum; // how many tabs before detch
		
		// try to rearrange tabs if their number > 1
		if (IsPtInTabArea (point) && nNumTabs > 1 && m_bEnableTabSwap)
		{
			CRect rectTab;
			int nTabNum = GetTabFromPoint (point);

			if (nTabNum != m_iActiveTab && nTabNum != -1)
			{
				int nSecondTab = m_iActiveTab;
				SwapTabs (nTabNum, nSecondTab);
				RecalcLayout ();
				SetActiveTab (nTabNum);
				int nCurrTabNum = GetTabFromPoint (point);
				if (nCurrTabNum != nTabNum)
				{
					GetTabRect (nTabNum, rectTab);
					CPoint ptCursorNewPos = point;

					ptCursorNewPos.x = rectTab.left + m_nOffsetFromTabLeft;

					ClientToScreen (&ptCursorNewPos);
					SetCursorPos (ptCursorNewPos.x, ptCursorNewPos.y);
				}
			}
			return;
		}

		if (IsPtInTabArea (point))
		{
			return;
		}
		
		BOOL bDetachSucceeded = DetachTab (BCGP_DM_MOUSE);

		if (bDetachSucceeded && nNumTabs <= 2)
		{
			// last tab was detached successfully - run out, because the control 
			// jas been destroyed
			return;
		}	

		if (bDetachSucceeded)
		{
			m_bReadyToDetach = FALSE;
		}

		return;
	}
}
//************************************************************************************
void CBCGPBaseTabWnd::OnCancelMode() 
{
	CBCGPWnd::OnCancelMode();
	
	if (m_iHighlighted >= 0)
	{
		int iTab = m_iHighlighted;

		ReleaseCapture ();
		m_iHighlighted = -1;
		m_iPressed = -1;

		OnChangeHighlightedTab(iTab);
		InvalidateTab (iTab);
	}

	if (m_pInPlaceEdit != NULL)
	{
		m_pInPlaceEdit->DestroyWindow ();
		delete m_pInPlaceEdit;
		m_pInPlaceEdit = NULL;
		m_iEditedTab = -1;
		ReleaseCapture ();
	}

	m_bReadyToDetach = FALSE;
}
//************************************************************************************
void CBCGPBaseTabWnd::InvalidateTab (int iTab)
{
	ASSERT_VALID (this);

	if (iTab < 0)
	{
		return;
	}

	CRect rectTab;

	if (GetTabRect (iTab, rectTab))
	{
		InvalidateRect (rectTab);
		UpdateWindow ();
	}
}
//*************************************************************************************
void CBCGPBaseTabWnd::OnDestroy() 
{
	CleanUp ();
	CBCGPWnd::OnDestroy();
}
//*************************************************************************************
int CBCGPBaseTabWnd::FindTabInfo (int nBarID, CBCGPTabInfo** ppTabInfo)
{
	*ppTabInfo = NULL;
	for (int i = 0; i < m_arTabs.GetSize (); i++)
	{
		CBCGPTabInfo* pTabInfo = (CBCGPTabInfo*) m_arTabs.GetAt (i);
		ASSERT_VALID (pTabInfo);

		if (pTabInfo->m_pWnd != NULL && pTabInfo->m_pWnd->GetDlgCtrlID () == nBarID)
		{
			*ppTabInfo = pTabInfo;
			return i;
		}
	}

	return NULL;
}
//*************************************************************************************
void CBCGPBaseTabWnd::ApplyRestoredTabInfo (BOOL bUseTabIndexes)
{
	CBCGPDockManager* pDockManager = globalUtils.GetDockManager (BCGPGetParentFrame (this));
	ASSERT_VALID (pDockManager);

	POSITION pos = NULL;

	for (pos = m_lstRestoredTabInfo.GetHeadPosition (); pos != NULL;)
	{
		CBCGPRestoredTabInfo tabInfo = m_lstRestoredTabInfo.GetNext (pos);

		CBCGPTabInfo* pCurrTabInfo = NULL;
		FindTabInfo (tabInfo.m_nControlBarID, &pCurrTabInfo);
		if (pCurrTabInfo != NULL)
		{
			continue;
		}

		CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar,
				pDockManager->FindBarByID (tabInfo.m_nControlBarID, TRUE));

		if (pBar != NULL)
		{
			if (pBar->IsTabbed ())
			{
				CBCGPBaseTabWnd* pTabWnd = (CBCGPBaseTabWnd*) pBar->GetParent ();
				CBCGPBaseTabbedBar* pTabBar = (CBCGPBaseTabbedBar*) pTabWnd->GetParent ();	
				ASSERT_VALID (pTabBar);
				pBar->SetParent (GetParent ());
				pTabBar->RemoveControlBar (pBar);
				if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)))
				{
					((CBCGPDockingControlBar*) pBar)->EnableGripper (TRUE);
				}

				pBar->ShowWindow (SW_SHOW);
			}
			if (pBar->IsAutoHideMode ())
			{
				pBar->SetAutoHideMode (FALSE, CBRS_ALIGN_ANY);
			}

			CBCGPMiniFrameWnd* pMiniFrame = pBar->GetParentMiniFrame ();
			if (pMiniFrame != NULL)
			{
				pMiniFrame->RemoveControlBar (pBar);
			}

			pBar->SetParent (GetParent ());
			
			CBCGPBaseTabbedBar* pTabbedBar = 
					DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, GetParent ());
			pBar->AttachToTabWnd (pTabbedBar, BCGP_DM_SHOW, FALSE);
		}
	}

	int nTabIdx = 0;
	int nVisibleTabNumber = 0;

	for (pos = m_lstRestoredTabInfo.GetHeadPosition (); pos != NULL; nTabIdx++)
	{
		CBCGPRestoredTabInfo tabInfo = m_lstRestoredTabInfo.GetNext (pos);
		CBCGPTabInfo* pCurrTabInfo = NULL;
		int nCurIdx = FindTabInfo (tabInfo.m_nControlBarID, &pCurrTabInfo);

		if (pCurrTabInfo == NULL)
		{
			continue;
		}

		if (IsRestoreTabAttributes())
		{
			pCurrTabInfo->m_strText  = tabInfo.m_strText;
			pCurrTabInfo->m_clrText  = tabInfo.m_clrText;
			pCurrTabInfo->m_clrBack  = tabInfo.m_clrBack;
			pCurrTabInfo->m_bIsDetachable = tabInfo.m_bDetachable;
		}

		ShowTab (nCurIdx, tabInfo.m_bVisible, FALSE);

		if (tabInfo.m_bVisible)
		{
			nVisibleTabNumber++;
		}

		if (bUseTabIndexes && nCurIdx != nTabIdx)
		{
			SwapTabs (nCurIdx, nTabIdx);
			if (pCurrTabInfo->m_pWnd != NULL && nTabIdx == m_nRestoredActiveTabID)
			{
				pCurrTabInfo->m_pWnd->ShowWindow (SW_SHOW);
			}
		}
	}
	
	if (nVisibleTabNumber > 0 && !SetActiveTab (m_nRestoredActiveTabID))
	{
		SetActiveTab (0);
	}

	if (nVisibleTabNumber == 0)
	{
		CBCGPBaseTabbedBar* pTabBar = DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, GetParent ());	
		if (pTabBar != NULL)
		{
			pTabBar->ShowControlBar (FALSE, FALSE, FALSE);
		}
	}
		
	RecalcLayout ();
}
//*************************************************************************************
void CBCGPBaseTabWnd::Serialize (CArchive& ar)
{
	int nTabCount = 0;
	if (ar.IsLoading ())
	{
		m_lstRestoredTabInfo.RemoveAll ();

		ar >> nTabCount;

		for (int i = 0; i < nTabCount; i++)
		{
			CBCGPRestoredTabInfo tabInfo;

			ar >> tabInfo.m_strText;
			ar >> tabInfo.m_bVisible;
			ar >> tabInfo.m_nControlBarID;
			ar >> tabInfo.m_bDetachable;
			ar >> tabInfo.m_clrText;
			ar >> tabInfo.m_clrBack;

			m_lstRestoredTabInfo.AddTail (tabInfo);
		}
		ar >> m_nRestoredActiveTabID;

		int nLoc = 0;
		ar >> nLoc; 
		m_locationRestored = (Location) nLoc;
	}
	else
	{
		nTabCount = (int) m_arTabs.GetSize ();
		ar << nTabCount;

		for (int i = 0; i < nTabCount; i++)
		{
			CBCGPTabInfo* pTabInfo = (CBCGPTabInfo*) m_arTabs.GetAt (i);
			ASSERT_VALID (pTabInfo);

			ar << pTabInfo->m_strText;
			ar << pTabInfo->m_bVisible;

			ASSERT_VALID (pTabInfo->m_pWnd);
			ar << pTabInfo->m_pWnd->GetDlgCtrlID ();

			ar << pTabInfo->m_bIsDetachable;
			ar << pTabInfo->m_clrText;
			ar << pTabInfo->m_clrBack;
		}

		ar << m_iActiveTab;
		ar << m_location;
	}
}
//***************************************************************************************
void CBCGPBaseTabWnd::SwapTabs (int nFisrtTabID, int nSecondTabID)
{
	if (m_arTabs.GetSize () <= nFisrtTabID || 
		m_arTabs.GetSize () <= nSecondTabID)
	{
		return;
	}

	if (IsNewTabEnabled() && m_iTabsNum > 0)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [m_iTabsNum - 1];
		ASSERT_VALID (pTab);

		if (pTab->m_bIsNewTab)
		{
			if (nFisrtTabID == m_iTabsNum - 1 || nSecondTabID == m_iTabsNum - 1)
			{
				return;
			}
		}
	}

	CBCGPTabInfo* pTabInfoFirst  = (CBCGPTabInfo*) m_arTabs.GetAt (nFisrtTabID);
	CBCGPTabInfo* pTabInfoSecond = (CBCGPTabInfo*) m_arTabs.GetAt (nSecondTabID);
	m_arTabs.SetAt (nFisrtTabID, pTabInfoSecond);
	m_arTabs.SetAt (nSecondTabID, pTabInfoFirst);
}
//***************************************************************************************
void CBCGPBaseTabWnd::MoveTab(int nSource, int nDest)
{
	ASSERT_VALID (this);

	if (nSource == nDest && !IsMDITab())
	{
		return;
	}

	CBCGPTabInfo* pSource = (CBCGPTabInfo*) m_arTabs [nSource];
	CBCGPTabInfo* pActive = (CBCGPTabInfo*) m_arTabs [m_iActiveTab];

	ASSERT (nDest < m_arTabs.GetSize ());

	if (IsNewTabEnabled() && m_iTabsNum > 0 && nDest == m_iTabsNum - 1)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [m_iTabsNum - 1];
		ASSERT_VALID (pTab);

		if (pTab->m_bIsNewTab)
		{
			nDest--;
		}
	}

	if (IsMDITab())
	{
		// Don't change order in MDI Tabs - use indexes instead:
		if (m_arTabIndexs.GetSize() != m_arTabs.GetSize())
		{
			m_arTabIndexs.RemoveAll();

			for (int i = 0; i < (int)m_arTabs.GetSize(); i++)
			{
				m_arTabIndexs.Add(i);
			}
		}

		if (nDest == -1)
		{
			nDest = (int)m_arTabs.GetSize() - 1;
		}

		int nSourceIndex = -1;

		for (int i = 0; i < (int)m_arTabIndexs.GetSize(); i++)
		{
			if (m_arTabIndexs[i] == nSource)
			{
				nSourceIndex = i;
				break;
			}
		}

		if (nSourceIndex != -1)
		{
			m_arTabIndexs.RemoveAt(nSourceIndex);
			m_arTabIndexs.InsertAt(nDest, nSource);
		}
		else
		{
			ASSERT(FALSE);
		}
	}
	else
	{
		if (nDest == -1)
		{
			m_arTabs.Add (pSource);
			m_arTabs.RemoveAt (nSource);
		}
		else
		{
			m_arTabs.RemoveAt (nSource);
			m_arTabs.InsertAt (nDest, pSource);
		}

		for (int iTab = 0; iTab < m_arTabs.GetSize(); iTab++)
		{
			if (pActive == (CBCGPTabInfo*) m_arTabs [iTab])
			{
				if (iTab != m_iActiveTab)
				{
					SetActiveTab (iTab);
					FireChangeActiveTab (m_iActiveTab);
				}
				break;
			}
		}
	}

	RecalcLayout ();
}
//***************************************************************************************
BOOL CBCGPBaseTabWnd::SetTabsOrder (const CArray<int,int>& arOrder)
{
	ASSERT_VALID (this);

	if (arOrder.GetSize () != m_iTabsNum)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CPtrArray arTabsSaved;
	int i = 0;

	for (i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		arTabsSaved.Add (pTab);
	}

	m_arTabs.RemoveAll ();

	BOOL bActiveTabIsChamged = FALSE;

	for (i = 0; i < m_iTabsNum; i ++)
	{
		int nIndex = arOrder [i];
		ASSERT (nIndex >= 0);
		ASSERT (nIndex < m_iTabsNum);

		CBCGPTabInfo* pTab = (CBCGPTabInfo*) arTabsSaved [nIndex];
		ASSERT_VALID (pTab);

		m_arTabs.Add (pTab);

		if (nIndex == m_iActiveTab && !bActiveTabIsChamged)
		{
			m_iActiveTab = i;
			bActiveTabIsChamged = TRUE;
		}
	}

	return TRUE;
}
//*********************************************************************************
CWnd* CBCGPBaseTabWnd::CreateWrapper (CWnd* pWndToWrap, LPCTSTR lpszTabLabel, BOOL bDetachable)
{
	ASSERT_VALID (pWndToWrap);

	if (pWndToWrap->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)) ||
		!bDetachable || !m_bEnableWrapping)
	{
		return pWndToWrap;
	}


	CBCGPDockingCBWrapper* pWrapper = NULL; 
	
	if (m_pDockingBarWrapperRTC != NULL)
	{
		pWrapper = DYNAMIC_DOWNCAST (CBCGPDockingCBWrapper, 
										m_pDockingBarWrapperRTC->CreateObject ());
	}
	else
	{
		pWrapper = new CBCGPDockingCBWrapper;
	}

	ASSERT_VALID (pWrapper);

	CRect rectInit (pWrapper->m_rectInitial);  
	
	// create will alter the runtime class, but here we have to save it
	// and set back after create
	CRuntimeClass* pSaveRTC = pWrapper->GetTabbedControlBarRTC ();


	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	DWORD dwBCGStyle = dwDefaultBCGDockingBarStyle;
	if (pWndToWrap->IsKindOf (RUNTIME_CLASS (CBCGPBaseControlBar)))
	{
		 dwStyle |= ((CBCGPBaseControlBar*) pWndToWrap)->GetBarStyle ();
		 dwBCGStyle = ((CBCGPBaseControlBar*) pWndToWrap)->GetBCGStyle ();
	}
		

	if (!pWrapper->Create (lpszTabLabel, this, rectInit, TRUE, (UINT) pWndToWrap->GetDlgCtrlID (), 
						dwStyle, CBRS_BCGP_REGULAR_TABS, dwBCGStyle))
	{
		delete pWrapper;
		TRACE0 ("Unable to create CBCGPDockingCBWrapper. \n");
		return pWndToWrap;
	}
	pWrapper->EnableGripper (FALSE);
	pWrapper->SetWrappedWnd (pWndToWrap);

	pWrapper->m_recentDockInfo.m_rectRecentFloatingRect = rectInit;

	if (pSaveRTC != NULL)
	{
		pWrapper->SetTabbedControlBarRTC (pSaveRTC);
	}

	return pWrapper;
}
//*************************************************************************************
BOOL CBCGPBaseTabWnd::StartRenameTab (int iTab)
{
	ASSERT_VALID (this);
	ASSERT (GetSafeHwnd () != NULL);

	if (!m_bIsInPlaceEdit)
	{
		return FALSE;
	}

	if (iTab < 0 || iTab >= m_arTabs.GetSize ())
	{
		return FALSE;
	}

	CBCGPTabInfo* pTabInfo = (CBCGPTabInfo*) m_arTabs.GetAt (iTab);
	if (pTabInfo->m_rect.IsRectEmpty ())
	{
		return FALSE;
	}

	ASSERT (m_pInPlaceEdit == NULL);

	m_pInPlaceEdit = new CEdit;
	ASSERT_VALID (m_pInPlaceEdit);

	CRect rectEdit = pTabInfo->m_rect;
	CalcRectEdit (rectEdit);
	
	if (!m_pInPlaceEdit->Create (WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, rectEdit, 
		this, 1))
	{
		delete m_pInPlaceEdit;
		m_pInPlaceEdit = NULL;

 		return FALSE;
	}

	m_pInPlaceEdit->SetWindowText (pTabInfo->m_strText);
	m_pInPlaceEdit->SetFont(GetTabFont());
	m_pInPlaceEdit->SetSel (0, -1, TRUE);

	m_pInPlaceEdit->SetFocus ();
	m_iEditedTab = iTab;

	SetCapture ();
	return TRUE;
}
//**************************************************************************************
BOOL CBCGPBaseTabWnd::PreTranslateMessage(MSG* pMsg) 
{
	if (m_pInPlaceEdit != NULL)
	{
		if (pMsg->message >= WM_KEYFIRST &&
			pMsg->message <= WM_KEYLAST)
		{
			switch (pMsg->wParam)
			{
			case VK_RETURN:
				if (!RenameTab ())
				{
					MessageBeep ((UINT)-1);
					return TRUE;
				}
				// Slide down!

			case VK_ESCAPE:
				m_pInPlaceEdit->DestroyWindow ();
				delete m_pInPlaceEdit;
				m_pInPlaceEdit = NULL;
				m_iEditedTab = -1;
				ReleaseCapture ();
				break;

			default:
				return FALSE;
			}

			return TRUE;
		}
		else if (pMsg->message >= WM_MOUSEFIRST &&
				 pMsg->message <= WM_MOUSELAST)
		{
			CRect rectEdit;
			m_pInPlaceEdit->GetClientRect (rectEdit);
			m_pInPlaceEdit->MapWindowPoints (this, rectEdit);

			CPoint ptCursor;
			::GetCursorPos (&ptCursor);
			ScreenToClient (&ptCursor);

			if (rectEdit.PtInRect (ptCursor))
			{
				m_pInPlaceEdit->SendMessage (pMsg->message, pMsg->wParam, pMsg->lParam);
			}
			else if (pMsg->message != WM_MOUSEMOVE)
			{
				m_pInPlaceEdit->DestroyWindow ();
				delete m_pInPlaceEdit;
				m_pInPlaceEdit = NULL;
				m_iEditedTab = -1;
				ReleaseCapture ();
			}

			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	return CBCGPWnd::PreTranslateMessage(pMsg);
}
//*********************************************************************************
BOOL CBCGPBaseTabWnd::RenameTab ()
{
	ASSERT_VALID (this);
	ASSERT (m_bIsInPlaceEdit);

	if (m_pInPlaceEdit == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CString strName;
	m_pInPlaceEdit->GetWindowText (strName);

	if (!strName.IsEmpty () &&
		OnRenameTab (m_iEditedTab, strName) &&
		GetParent ()->SendMessage (BCGM_ON_RENAME_TAB, m_iEditedTab,
									(LPARAM)(LPCTSTR) strName) == 0)
	{
		return SetTabLabel (m_iEditedTab, strName);
	}

	return FALSE;
}
//***********************************************************************************
void CBCGPBaseTabWnd::SetActiveTabTextColor (COLORREF clr)
{
	m_clrActiveTabFg = clr;
}
//***********************************************************************************
void CBCGPBaseTabWnd::SetActiveTabColor (COLORREF clr)
{
	m_clrActiveTabBk = clr;

	if (m_brActiveTab.GetSafeHandle () != NULL)
	{
		m_brActiveTab.DeleteObject ();
	}

	m_brActiveTab.CreateSolidBrush (GetActiveTabColor ());
}
//*********************************************************************************
COLORREF CBCGPBaseTabWnd::GetTabBkColor (int iTab) const
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return (COLORREF)-1;
	}

	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	COLORREF color = pTab->m_clrBack;

	if (color == (COLORREF)-1 && m_bIsAutoColor)
	{
		color = m_arAutoColors [iTab % m_arAutoColors.GetSize ()];
		pTab->m_clrBack = color;
	}

	return color;
}
//*********************************************************************************
BOOL CBCGPBaseTabWnd::SetTabBkColor (int iTab, COLORREF color)
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	pTab->m_clrBack = color;
	return TRUE;
}
//*********************************************************************************
COLORREF CBCGPBaseTabWnd::GetTabTextColor (int iTab) const
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return (COLORREF)-1;
	}

	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	return pTab->m_clrText;
}
//*********************************************************************************
BOOL CBCGPBaseTabWnd::SetTabTextColor (int iTab, COLORREF color)
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	pTab->m_clrText = color;
	return TRUE;
}
//***************************************************************************************
int CBCGPBaseTabWnd::GetTabFullWidth (int iTab) const
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return 0;
	}

	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID(pTab);

	return pTab->m_nFullWidth;
}
//***************************************************************************************
BOOL CBCGPBaseTabWnd::SetImageList (UINT uiID, int cx, COLORREF clrTransp)
{
	CBitmap bmp;
	if (!bmp.LoadBitmap (uiID))
	{
		TRACE(_T("CBCGPTabWnd::SetImageList Can't load bitmap: %x\n"), uiID);
		return FALSE;
	}

	if (m_Images.GetSafeHandle () != NULL)
	{
		m_Images.DeleteImageList ();
	}

	if (m_ImagesGray.GetSafeHandle() != NULL)
	{
		m_ImagesGray.DeleteImageList ();
	}

	BITMAP bmpObj;
	bmp.GetBitmap (&bmpObj);

	UINT nFlags = (clrTransp == (COLORREF) -1) ? 0 : ILC_MASK;

	switch (bmpObj.bmBitsPixel)
	{
	case 4:
	default:
		nFlags |= ILC_COLOR4;
		break;

	case 8:
		nFlags |= ILC_COLOR8;
		break;

	case 16:
		nFlags |= ILC_COLOR16;
		break;

	case 24:
		nFlags |= ILC_COLOR24;
		break;

	case 32:
		nFlags |= ILC_COLOR32;
		break;
	}

	m_Images.Create (cx, bmpObj.bmHeight, nFlags, 0, 0);
	m_Images.Add (&bmp, clrTransp);

	m_sizeImage = CSize (cx, bmpObj.bmHeight);

	SetTabsHeight ();
	return TRUE;
}
//***************************************************************************************
BOOL CBCGPBaseTabWnd::SetImageList (HIMAGELIST hImageList)
{
	ASSERT (hImageList != NULL);

	if (m_Images.GetSafeHandle () != NULL)
	{
		m_Images.DeleteImageList ();
	}

	if (m_ImagesGray.GetSafeHandle() != NULL)
	{
		m_ImagesGray.DeleteImageList ();
	}

	CImageList* pImageList = CImageList::FromHandle (hImageList);
	if (pImageList == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	IMAGEINFO info;
	pImageList->GetImageInfo (0, &info);

	CRect rectImage = info.rcImage;
	m_sizeImage = rectImage.Size ();

	m_hImageList = hImageList;

	SetTabsHeight ();
	return TRUE;
}
//************************************************************************************
const CImageList* CBCGPBaseTabWnd::GetImageList () const
{
	if (m_Images.GetSafeHandle () != NULL)
	{
		if (!CBCGPVisualManager::GetInstance()->IsAutoGrayscaleImages())
		{
			return &m_Images;
		}

		if (m_ImagesGray.GetSafeHandle () == NULL)
		{
			CBCGPToolBarImages images;
			if (images.CreateFromImageList(m_Images))
			{
				images.ConvertToGrayScale();
				images.ExportToImageList(((CBCGPBaseTabWnd*)this)->m_ImagesGray);
			}
		}

		return m_ImagesGray.GetSafeHandle () == NULL ? &m_Images : &m_ImagesGray;
	}

	return m_hImageList != NULL ? CImageList::FromHandle (m_hImageList) :  NULL;
}
//************************************************************************************
void CBCGPBaseTabWnd::ClearImageList ()
{
	if (m_Images.GetSafeHandle () != NULL)
	{
		m_Images.DeleteImageList ();
	}

	if (m_ImagesGray.GetSafeHandle() != NULL)
	{
		m_ImagesGray.DeleteImageList ();
	}

	m_sizeImage = CSize (0, 0);
	SetTabsHeight ();
}
//************************************************************************************
void CBCGPBaseTabWnd::ResetImageList ()
{
	ClearImageList ();
	m_hImageList = NULL;
	m_mapAddedIcons.RemoveAll ();
}
//************************************************************************************
void CBCGPBaseTabWnd::SetTabsHeight ()
{
	const int nImageHeight = m_sizeImage.cy <= 0 ? 0 : m_sizeImage.cy + 7;
	
	int nTextMargin = 0;
	int nTextHeight = GetTextHeight(nTextMargin);

	m_nTabsHeight = (max (nImageHeight, nTextHeight - nTextMargin + 7));
}
//*********************************************************************************
void CBCGPBaseTabWnd::HideSingleTab (BOOL bHide)
{
	if (m_bHideSingleTab == bHide)
	{
		return;
	}

	m_bHideSingleTab = bHide;

	if (GetSafeHwnd () != NULL)
	{
		RecalcLayout ();
	}
}
//*********************************************************************************
void CBCGPBaseTabWnd::SetSingleTabStatic(BOOL bSet)
{
	m_bSingleTabIsStatic = bSet;
}
//*********************************************************************************
CWnd* CBCGPBaseTabWnd::GetFirstVisibleTab (int& iTabNum)
{
	return GetFirstVisibleTab (0, iTabNum);
}
//********************************************************************************
CWnd* CBCGPBaseTabWnd::GetFirstVisibleTab (int iStartFrom, int& iTabNum)
{
	for (int i = iStartFrom; i < m_iTabsNum; i++)
	{
		CBCGPTabInfo* pTabInfo = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTabInfo);

		if (pTabInfo->m_bVisible)
		{
			iTabNum = i;
			return pTabInfo->m_pWnd;
		}
	}
	if (iStartFrom > 0)
	{
		return GetFirstVisibleTab (0, iTabNum);
	}
	iTabNum = -1;
	return NULL;
}
//********************************************************************************
CWnd* CBCGPBaseTabWnd::GetLastVisibleTab (int& iTabNum)
{
	for (int i = m_iTabsNum - 1; i >= 0; i--)
	{
		CBCGPTabInfo* pTabInfo = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTabInfo);

		if (pTabInfo->m_bVisible)
		{
			iTabNum = i;
			return pTabInfo->m_pWnd;
		}
	}
	iTabNum = -1;
	return NULL;
}
//********************************************************************************
void CBCGPBaseTabWnd::SetDrawNoPrefix (BOOL bNoPrefix, BOOL bRedraw)
{
	ASSERT_VALID (this);
	m_bLabelNoPrefix = bNoPrefix;

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//*******************************************************************************
void CBCGPBaseTabWnd::SetDrawNameInUpperCase(BOOL bSet)
{
	m_bDrawNameInUpperCase = bSet;
}
//*******************************************************************************
int CBCGPBaseTabWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (BCGPGetParentFrame (this) == NULL)
	{
		m_bEnableWrapping = FALSE;
	}
	
	_AFX_THREAD_STATE* pState = AfxGetThreadState();
	if (pState->m_bNeedTerm)	// AfxOleInit was called
	{
		m_DropTarget.Register (this);
	}
	
	return 0;
}
//*******************************************************************************
CSize CBCGPBaseTabWnd::GetMaxWindowSize () const
{
	CSize sizeMax (0, 0);
	int nTabCount = (int) m_arTabs.GetSize ();

	for (int i = 0; i < nTabCount; i++)
	{
		CBCGPTabInfo* pTabInfo = (CBCGPTabInfo*) m_arTabs.GetAt (i);
		ASSERT_VALID (pTabInfo);

		if (pTabInfo->m_pWnd != NULL)
		{
			CSize sizeCurr (0, 0);
			if (pTabInfo->m_pWnd->IsKindOf (RUNTIME_CLASS (CBCGPBaseControlBar)))
			{
				CBCGPBaseControlBar* pBar = 
					DYNAMIC_DOWNCAST (CBCGPBaseControlBar, pTabInfo->m_pWnd);
				if (pBar != NULL)
				{
					sizeCurr = pBar->CalcFixedLayout (FALSE, TRUE);
				}
			}
			else
			{
				CRect rectWnd;
				pTabInfo->m_pWnd->GetWindowRect (rectWnd);
				sizeCurr = rectWnd.Size ();
			}

			sizeMax.cx = max (sizeCurr.cx, sizeMax.cx);
			sizeMax.cy = max (sizeCurr.cy, sizeMax.cy);
		}
	}

	return sizeMax;
}
//**********************************************************************************
void CBCGPBaseTabWnd::EnableAutoColor (BOOL bEnable/* = TRUE*/)
{
	if (m_bIsAutoColor && !bEnable)
	{
		for (int i = 0; i < m_iTabsNum; i ++)
		{
			CBCGPTabInfo* pTabInfo = (CBCGPTabInfo*) m_arTabs.GetAt (i);
			ASSERT_VALID (pTabInfo);

			pTabInfo->m_clrBack = (COLORREF)-1;
		}
	}

	m_bIsAutoColor = bEnable;
	InitAutoColors ();

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//***********************************************************************************
void CBCGPBaseTabWnd::InitAutoColors ()
{
	if (!m_bIsDefaultAutoColor)
	{
		return;
	}

	m_arAutoColors.RemoveAll ();

	if (globalData.m_nBitsPerPixel > 8)
	{
		m_arAutoColors.Add (RGB (197, 212, 242));
		m_arAutoColors.Add (RGB (255, 220, 120));
		m_arAutoColors.Add (RGB (190, 206, 161));
		m_arAutoColors.Add (RGB (240, 160, 161));
		m_arAutoColors.Add (RGB (188, 168, 225));
		m_arAutoColors.Add (RGB (156, 193, 182));
		m_arAutoColors.Add (RGB (247, 184, 134));
		m_arAutoColors.Add (RGB (217, 173, 194));
		m_arAutoColors.Add (RGB (165, 194, 215));
		m_arAutoColors.Add (RGB (179, 166, 190));
		m_arAutoColors.Add (RGB (234, 214, 163));
		m_arAutoColors.Add (RGB (246, 250, 125));
		m_arAutoColors.Add (RGB (181, 233, 157));
		m_arAutoColors.Add (RGB (95, 195, 207));
		m_arAutoColors.Add (RGB (193, 131, 131));
		m_arAutoColors.Add (RGB (202, 202, 213));
	}
	else
	{
		m_arAutoColors.Add (RGB (0, 255, 0));
		m_arAutoColors.Add (RGB (0, 255, 255));
		m_arAutoColors.Add (RGB (255, 0, 255));
		m_arAutoColors.Add (RGB (192, 192, 192));
		m_arAutoColors.Add (RGB (255, 255, 0));
	}
}
//*************************************************************************************
void CBCGPBaseTabWnd::OnSysColorChange() 
{
	CBCGPWnd::OnSysColorChange();

	InitAutoColors ();
}
//************************************************************************************
void CBCGPBaseTabWnd::SetAutoColors (const CArray<COLORREF, COLORREF>& arColors)
{
	int i = 0;

	for (i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTabInfo = (CBCGPTabInfo*) m_arTabs.GetAt (i);
		ASSERT_VALID (pTabInfo);

		pTabInfo->m_clrBack = (COLORREF)-1;
	}

	m_arAutoColors.RemoveAll ();

	if (arColors.GetSize () == 0)
	{
		m_bIsDefaultAutoColor = TRUE;
		InitAutoColors ();
	}
	else
	{
		m_bIsDefaultAutoColor = FALSE;

		for (i = 0; i < arColors.GetSize (); i++)
		{
			m_arAutoColors.Add (arColors [i]);
		}
	}

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//*********************************************************************************
void CBCGPBaseTabWnd::SetLocation (Location location)
{
	ASSERT_VALID (this);

	m_location = location;
	RecalcLayout ();

	if (GetSafeHwnd () != NULL)
	{
		GetParent ()->RedrawWindow (NULL, NULL, 
			RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}
}
//*********************************************************************************
BOOL CBCGPBaseTabWnd::IsColored () const
{
	for (int iTab = 0; iTab < m_iTabsNum; iTab++)
	{
		if (GetTabBkColor (iTab) != (COLORREF)-1)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*********************************************************************************
void CBCGPBaseTabWnd::FireChangeActiveTab (int nNewTab)
{
	CFrameWnd* pParentFrame = BCGCBProGetTopLevelFrame (this);
	CWnd* pParent = GetParent ();
	ASSERT_VALID (pParent);
	
	pParent->SendMessage (BCGM_CHANGE_ACTIVE_TAB, nNewTab, (LPARAM)this);
	if (pParent != pParentFrame && pParentFrame != NULL)
	{
		ASSERT_VALID (pParentFrame);
		pParentFrame->SendMessage (BCGM_CHANGE_ACTIVE_TAB, nNewTab, (LPARAM)this);
	}

	m_bSetActiveTabFired = TRUE;

	if (globalData.IsAccessibilitySupport() && nNewTab >= 0 && nNewTab < (int)m_arTabs.GetSize())
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*)m_arTabs[nNewTab];
		ASSERT_VALID(pTab);
	
		SetACCData(pTab, m_AccData, nNewTab == m_iActiveTab);
		globalData.NotifyWinEvent(EVENT_OBJECT_SELECTION, GetSafeHwnd(), OBJID_CLIENT , nNewTab + 1);
	}
}
//*********************************************************************************
BOOL CBCGPBaseTabWnd::FireChangingActiveTab (int nNewTab)
{
	CFrameWnd* pParentFrame = BCGCBProGetTopLevelFrame (this);
	CWnd* pParent = GetParent ();
	ASSERT_VALID (pParent);
	
	BOOL bRes = (BOOL) pParent->SendMessage (BCGM_CHANGING_ACTIVE_TAB, nNewTab, (LPARAM)this);
	if (bRes)
	{
		return TRUE;
	}

	if (pParent != pParentFrame && pParentFrame != NULL)
	{
		ASSERT_VALID (pParentFrame);
		bRes = (BOOL) pParentFrame->SendMessage (BCGM_CHANGING_ACTIVE_TAB, nNewTab, (LPARAM)this);
	}

	return bRes;
}
//*********************************************************************************
BOOL CBCGPBaseTabWnd::FireNewTab ()
{
	CFrameWnd* pParentFrame = BCGCBProGetTopLevelFrame (this);
	CWnd* pParent = GetParent ();
	ASSERT_VALID (pParent);
	
	BOOL bRes = (BOOL) pParent->SendMessage (BCGM_NEW_TAB, 0, (LPARAM)this);
	if (bRes)
	{
		return TRUE;
	}

	if (pParent != pParentFrame && pParentFrame != NULL)
	{
		ASSERT_VALID (pParentFrame);
		bRes = (BOOL) pParentFrame->SendMessage (BCGM_NEW_TAB, 0, (LPARAM)this);
	}

	return bRes;
}
//*******************************************************************************
BOOL CBCGPBaseTabWnd::HasImage (int iTab) const
{
	ASSERT_VALID (this);

	if (iTab < 0 || iTab >= m_iTabsNum)
	{
		return FALSE;
	}

	CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [iTab];
	ASSERT_VALID (pTab);

	if (pTab->m_hIcon != NULL)
	{
		return TRUE;
	}

	return GetImageList () != NULL && pTab->m_uiIcon != (UINT)-1;
}
//*****************************************************************************
void CBCGPBaseTabWnd::SetTabBorderSize (int nTabBorderSize, BOOL bRepaint/* = TRUE*/)
{
	if (nTabBorderSize == -1)
	{
		nTabBorderSize = DEFAULT_TAB_BORDER_SIZE;
	}

	if (m_nTabBorderSize != nTabBorderSize)
	{
		m_nTabBorderSize = nTabBorderSize;

		RecalcLayout ();

		if (bRepaint && GetSafeHwnd () != NULL)
		{
			Invalidate ();
			UpdateWindow ();
		}
	}
}
//*****************************************************************************
BOOL CBCGPBaseTabWnd::EnableCustomToolTips (BOOL bEnable/* = TRUE*/)
{
	if (m_bCustomToolTips == bEnable)
	{
		return TRUE;
	}

	m_bCustomToolTips = bEnable;

	if (m_pToolTip->GetSafeHwnd () == NULL)
	{
		return TRUE;
	}

	for (int i = 0; i < m_iTabsNum; i++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);
		
		if (m_bCustomToolTips)
		{
			CToolInfo info;

			m_pToolTip->GetToolInfo (info, this, pTab->m_iTabID);
			m_pToolTip->DelTool (this, pTab->m_iTabID);

			m_pToolTip->AddTool (this,
				LPSTR_TEXTCALLBACK, &info.rect, pTab->m_iTabID);
		}
		else
		{
			m_pToolTip->UpdateTipText (pTab->m_strText, this, pTab->m_iTabID);
		}
	}

	RecalcLayout ();
	return TRUE;
}
//*****************************************************************************************
BOOL CBCGPBaseTabWnd::OnNeedTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	static CBCGPTabToolTipInfo info;

	LPNMTTDISPINFO	pTTDispInfo	= (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	if (pNMH->hwndFrom == m_pToolTipClose->GetSafeHwnd ())
	{
		CBCGPLocalResource locaRes;
		CString str;

		info.m_strText.LoadString (IDS_BCGBARRES_CLOSEBAR);

		pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) info.m_strText);
		return TRUE;
	}

	if (m_pToolTip->GetSafeHwnd () == NULL || 
		pNMH->hwndFrom != m_pToolTip->GetSafeHwnd ())
	{
		return FALSE;
	}

	CFrameWnd* pParentFrame = BCGCBProGetTopLevelFrame (this);
	CWnd* pParent = GetParent ();
	ASSERT_VALID (pParent);

	info.m_pTabWnd = this;

	CPoint ptCursor;
	::GetCursorPos (&ptCursor);
	ScreenToClient (&ptCursor);

	CRect rectCloseButton;
	if (GetTabCloseButtonRect(m_iHighlighted, rectCloseButton) && rectCloseButton.PtInRect (ptCursor))
	{
		return FALSE;
	}

	info.m_nTabIndex = GetTabFromPoint (ptCursor);
	if (info.m_nTabIndex < 0)
	{
		return FALSE;
	}

	info.m_strText.Empty ();
	
	pParent->SendMessage (BCGM_ON_GET_TAB_TOOLTIP, 0, (LPARAM) &info);
	if (pParent != pParentFrame && pParentFrame != NULL)
	{
		ASSERT_VALID (pParentFrame);
		pParentFrame->SendMessage (BCGM_ON_GET_TAB_TOOLTIP, 0, (LPARAM) &info);
	}

	if (info.m_strText.IsEmpty ())
	{
		return FALSE;
	}

	pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) info.m_strText);
	return TRUE;
}
//*****************************************************************************************
void CBCGPBaseTabWnd::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CBCGPWnd::OnWindowPosChanged(lpwndpos);
	m_bWindowPosChanged = TRUE;
}
//*****************************************************************************************
void CBCGPBaseTabWnd::OnMButtonUp(UINT nFlags, CPoint point) 
{
    if (m_bIsMDITab)
    {
		int nTab = GetTabFromPoint(point);
		if (nTab >= 0)
		{
			CWnd* pWnd = GetTabWnd(nTab);
			if (pWnd->GetSafeHwnd () != NULL)
			{
				pWnd->SendMessage (WM_CLOSE);
				return;
			}
		}
    }

    CBCGPWnd::OnMButtonUp(nFlags, point);
}
//****************************************************************************
BOOL CBCGPBaseTabWnd::OnSetAccData (long lVal)
{
	ASSERT_VALID (this);

	m_AccData.Clear ();

	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if ((i + 1 == lVal) && pTab->m_bVisible && !pTab->m_rect.IsRectEmpty ())
		{
			SetACCData (pTab, m_AccData, i == m_iActiveTab);
			return TRUE;
		}
	}
	
	return FALSE;
}
//****************************************************************************
HRESULT CBCGPBaseTabWnd::accHitTest(long xLeft, long yTop, VARIANT *pvarChild)
{
	if( !pvarChild )
    {
        return E_INVALIDARG;
    }

	pvarChild->vt = VT_I4;
	pvarChild->lVal = CHILDID_SELF;

	CPoint pt (xLeft, yTop);
	ScreenToClient (&pt);

	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if (pTab->m_rect.PtInRect (pt))
		{
			pvarChild->lVal = i + 1;
			SetACCData (pTab, m_AccData, i == m_iActiveTab);
			break;
		}
	}

	return S_OK;
}
//****************************************************************************
HRESULT CBCGPBaseTabWnd::get_accChildCount(long *pcountChildren)
{
	if( !pcountChildren )
    {
        return E_INVALIDARG;
    }

	int count = 0;
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if (pTab->m_bVisible && !pTab->m_rect.IsRectEmpty ())
		{
			count++;
		}
	}

	*pcountChildren = count;
	return S_OK;
}
//****************************************************************************
HRESULT CBCGPBaseTabWnd::get_accChild(VARIANT /*varChild*/, IDispatch **ppdispChild)
{
	if( !ppdispChild )
    {
        return E_INVALIDARG;
    }

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPBaseTabWnd::accNavigate(long navDir, VARIANT varStart, VARIANT* pvarEndUpAt)
{
    pvarEndUpAt->vt = VT_EMPTY;

    if (varStart.vt != VT_I4)
    {
        return E_INVALIDARG;
    }

	int count = 0;
	for (int i = 0; i < m_iTabsNum; i ++)
	{
		CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
		ASSERT_VALID (pTab);

		if (pTab->m_bVisible && !pTab->m_rect.IsRectEmpty ())
		{
			count++;
		}
	}

	switch (navDir)
    {
	case NAVDIR_FIRSTCHILD:
		if (varStart.lVal == CHILDID_SELF)
		{
			pvarEndUpAt->vt = VT_I4;
			pvarEndUpAt->lVal = 1;
			return S_OK;	
		}
		break;

	case NAVDIR_LASTCHILD:
		if (varStart.lVal == CHILDID_SELF)
		{
			pvarEndUpAt->vt = VT_I4;
			pvarEndUpAt->lVal = count;
			return S_OK;
		}
		break;


	case NAVDIR_NEXT:   
	case NAVDIR_RIGHT:
		if (varStart.lVal != CHILDID_SELF)
		{
			pvarEndUpAt->vt = VT_I4;
			pvarEndUpAt->lVal = varStart.lVal + 1;
			if (pvarEndUpAt->lVal > count)
			{
				pvarEndUpAt->vt = VT_EMPTY;
				return S_FALSE;
			}
			return S_OK;
		}
		break;

	case NAVDIR_PREVIOUS: 
	case NAVDIR_LEFT:
		if (varStart.lVal != CHILDID_SELF)
		{
			pvarEndUpAt->vt = VT_I4;
			pvarEndUpAt->lVal = varStart.lVal - 1;
			if (pvarEndUpAt->lVal <= 0)
			{
				pvarEndUpAt->vt = VT_EMPTY;
				return S_FALSE;
			}
			return S_OK;
		}
		break;
	}

	return S_FALSE;
}
//******************************************************************************
HRESULT CBCGPBaseTabWnd::accDoDefaultAction(VARIANT varChild)
{
    if (varChild.vt != VT_I4)
    {
        return E_INVALIDARG;
    }

	if (varChild.lVal != CHILDID_SELF)
    {
		int count = 0;
		for (int i = 0; i < m_iTabsNum; i ++)
		{
			CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [i];
			ASSERT_VALID (pTab);

			CRect rectButton = pTab->m_rect;
			if (pTab->m_bVisible && !rectButton.IsRectEmpty ())
			{
				if (count == varChild.lVal)
				{
					SetActiveTab (i);
					FireChangeActiveTab(m_iActiveTab);
					break;
				}

				count++;
			}
		}
    }

    return S_OK;
}
//*******************************************************************************
BOOL CBCGPBaseTabWnd::SetACCData (CBCGPTabInfo* pTab, CBCGPAccessibilityData& data, BOOL bIsActive)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pTab);

	data.Clear ();

	data.m_strAccName = pTab->m_strText;
	data.m_strAccDefAction = _T("Switch");
	
	data.m_nAccHit = 1;
	data.m_nAccRole = ROLE_SYSTEM_PAGETAB;

	if (bIsActive)
	{
		data.m_bAccState |= STATE_SYSTEM_SELECTED;
	}
	
	data.m_rectAccLocation = pTab->m_rect;
	ClientToScreen (&data.m_rectAccLocation);

	return TRUE;
}
//****************************************************************************
HRESULT CBCGPBaseTabWnd::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
	if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
	{
		pvarRole->vt = VT_I4;
		pvarRole->lVal = ROLE_SYSTEM_PAGETABLIST;
		return S_OK;
	}

	if ( !pvarRole || (( varChild.vt != VT_I4 ) && ( varChild.lVal != CHILDID_SELF )) )
    {
        return E_INVALIDARG;
    }

	if (varChild.vt == VT_I4 && varChild.lVal > 0)
	{
		pvarRole->vt = VT_I4;
		OnSetAccData (varChild.lVal);
		pvarRole->lVal = m_AccData.m_nAccRole;
		return S_OK;
	}
			
    return S_OK;
}
//****************************************************************************
HRESULT CBCGPBaseTabWnd::get_accValue(VARIANT varChild, BSTR *pszValue)
{
	if (varChild.vt == VT_I4 && varChild.lVal > 0)
	{
		OnSetAccData (varChild.lVal);
		if (m_AccData.m_strAccValue.IsEmpty ())
		{
			return S_FALSE;
		}
		*pszValue = m_AccData.m_strAccValue.AllocSysString ();
	}
	else
	{
		if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF &&
			(m_iActiveTab != -1))
		{
			CBCGPTabInfo* pTab = (CBCGPTabInfo*) m_arTabs [m_iActiveTab];
			ASSERT_VALID (pTab);
			if (pTab->m_strText.IsEmpty ())
			{
				return S_FALSE;
			}

			*pszValue = pTab->m_strText.AllocSysString ();
			return S_OK;
		}

		return S_FALSE;
	}

	return S_OK;
}
//****************************************************************************
HRESULT CBCGPBaseTabWnd::get_accState(VARIANT varChild, VARIANT *pvarState)
{
	if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
	{
		pvarState->vt = VT_I4;
	    pvarState->lVal = STATE_SYSTEM_NORMAL;
		return S_OK;
	}

	if ( !pvarState || (( varChild.vt != VT_I4 ) && ( varChild.lVal != CHILDID_SELF )) )
    {
        return E_INVALIDARG;
    }

	if (varChild.vt == VT_I4 && varChild.lVal > 0)
	{
		OnSetAccData (varChild.lVal);
		pvarState->vt = VT_I4;
		pvarState->lVal = m_AccData.m_bAccState;
		return S_OK; 
	}

	return E_INVALIDARG;
}
//*****************************************************************************
void CBCGPBaseTabWnd::SetCaptionFont(BOOL bSet)
{
	m_bCaptionFont = bSet;

	if (GetSafeHwnd() != NULL)
	{
		SetTabsHeight();
		RecalcLayout();
		RedrawWindow();
	}
}
//*****************************************************************************
LRESULT CBCGPBaseTabWnd::OnSetFont (WPARAM wParam, LPARAM lParam)
{
	BOOL bRedraw = (BOOL)LOWORD(lParam);

	m_hFontCustom = (HFONT)wParam;

	if (bRedraw)
	{
		SetTabsHeight();
		RecalcLayout();
		RedrawWindow();
	}

	OnCustomFontChanged();

	return 0;
}
//*****************************************************************************
LRESULT CBCGPBaseTabWnd::OnGetFont (WPARAM, LPARAM)
{
	return (LRESULT)m_hFontCustom;
}
//*****************************************************************************
CFont* CBCGPBaseTabWnd::GetTabFont()
{
	ASSERT_VALID(this);

	if (m_hFontCustom != NULL && ::GetObjectType(m_hFontCustom) != OBJ_FONT)
	{
		m_hFontCustom = NULL;
	}

	return m_hFontCustom == NULL ? (IsPointerStyle() ? &globalData.fontVertCaption : &globalData.fontRegular) : CFont::FromHandle(m_hFontCustom);
}
//*****************************************************************************
int CBCGPBaseTabWnd::GetTextHeight(int& nTextMargin)
{
	if (m_hFontCustom != NULL && ::GetObjectType(m_hFontCustom) != OBJ_FONT)
	{
		m_hFontCustom = NULL;
	}

	if (m_hFontCustom == NULL)
	{
		if (IsCaptionFont())
		{
			const int nHeight = globalData.GetCaptionTextHeight();
			nTextMargin = nHeight / 5;
			return nHeight;
		}

		nTextMargin = globalData.GetTextMargins();
		return globalData.GetTextHeight();
	}

	CWindowDC dc (NULL);
	
	CFont* pOldFont = dc.SelectObject(GetTabFont());
	ASSERT (pOldFont != NULL);
	
	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);
	
	nTextMargin = tm.tmHeight < 15 ? 2 : 5;
	int nTextHeight = tm.tmHeight + nTextMargin;
	
	dc.SelectObject (pOldFont);

	return nTextHeight;
}
