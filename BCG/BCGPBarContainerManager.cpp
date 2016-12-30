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
// BCGBarContainerManager.cpp: implementation of the CBCGPBarContainerManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPControlBar.h"
#include "BCGPDockingControlBar.h"
#include "BCGPBaseTabbedBar.h"
#include "BCGPSlider.h"
#include "BCGPBarContainerManager.h"
#include "BCGPBarContainer.h"
#include "BCGPDockManager.h"
#include "BCGPGlobalUtils.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPMultiMiniFrameWnd.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static int g_nSliderSpacingForMove = 4;

static int g_nSliderID = 1;

IMPLEMENT_DYNCREATE (CBCGPBarContainerManager, CObject)
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPBarContainerManager::CBCGPBarContainerManager() : m_pRootContainer (NULL), 
													 m_pDockSite (NULL), 
													 m_pContainerRTC (NULL), 
													 m_pDefaultSlider (NULL),
													 m_bDestroyRootContainer (TRUE)
{
	
}

CBCGPBarContainerManager::~CBCGPBarContainerManager()
{
	// should not be destroyed when the rrot container is "exported" to another 
	// manager (when one multi miniframe is docked to another multi miniframe)
	if (m_bDestroyRootContainer)
	{
		for (POSITION pos = m_lstSliders.GetHeadPosition (); pos != NULL;)
		{
			CBCGPSlider* pSlider = 
				DYNAMIC_DOWNCAST (CBCGPSlider, m_lstSliders.GetNext (pos));
			if (pSlider != NULL)
			{
				pSlider->SetContainerManager (NULL);
			}
		}
		if (m_pRootContainer != NULL)
		{
			delete m_pRootContainer;
		}
	}
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::Create (CWnd* pParentWnd, 
									   CBCGPSlider* pDefaultSlider,
									  CRuntimeClass* pContainerRTC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pParentWnd);
	m_pDockSite = pParentWnd;
	m_pContainerRTC = pContainerRTC;


	ASSERT (m_pRootContainer == NULL);

	if (m_pContainerRTC != NULL)
	{
		m_pRootContainer = (CBCGPBarContainer*) m_pContainerRTC->CreateObject ();
		m_pRootContainer->SetContainerManager (this);
	}
	else
	{
		m_pRootContainer = new CBCGPBarContainer (this);
	}

	m_pDefaultSlider = pDefaultSlider;
	return TRUE;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::AddControlBar (CBCGPDockingControlBar* pControlBarToAdd)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pControlBarToAdd); 
	ASSERT_KINDOF (CBCGPDockingControlBar, pControlBarToAdd);

	m_pRootContainer->SetBar (pControlBarToAdd, TRUE);
	m_lstControlBars.AddTail (pControlBarToAdd);
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::AddContainerManager (CBCGPBarContainerManager& srcManager, 
													BOOL bOuterEdge)
{
	ASSERT_VALID (this);
	ASSERT (m_pRootContainer != NULL);

	if (!m_pRootContainer->IsContainerEmpty ())
	{
		return FALSE;
	}

	CBCGPDockManager* pDockManager = globalUtils.GetDockManager (m_pDockSite);
	if (pDockManager == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	m_lstControlBars.RemoveAll ();
	m_lstSliders.RemoveAll ();

	srcManager.AddControlBarsToList (&m_lstControlBars, &m_lstSliders);
	srcManager.RemoveAllControlBarsAndSliders ();

	// we must copy containers before SetParent, because Copy sets recent dock info
	// internally and we need the "old" parent for that
	CBCGPBarContainer* pNewContainer = srcManager.m_pRootContainer->Copy (m_pRootContainer);
	m_pRootContainer->SetContainer (pNewContainer, TRUE);
	pNewContainer->SetContainerManager (this, TRUE);

	globalUtils.SetNewParent (m_lstControlBars, m_pDockSite);
	globalUtils.SetNewParent (m_lstSliders, m_pDockSite);

	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar,  m_lstControlBars.GetNext (pos));
		ASSERT_VALID (pBar);
		// move them out of screen
		CRect rect;
		pBar->GetWindowRect (rect);
		pBar->GetParent ()->ScreenToClient (rect);
	}

	// set new container manager for each slider
	for (pos = m_lstSliders.GetHeadPosition (); pos != NULL;)
	{
		CBCGPSlider* pSlider = (CBCGPSlider*) m_lstSliders.GetNext (pos);
		ASSERT_VALID (pSlider);

		pSlider->SetContainerManager (this);
	}

	// finally, enable caption for each control bar in the container manager
	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar,  m_lstControlBars.GetNext (pos));
		ASSERT_VALID (pBar);

		pBar->SetDefaultSlider (m_pDefaultSlider->m_hWnd);	
		pBar->SetBarAlignment (m_pDefaultSlider->GetCurrentAlignment ());
		pDockManager->AddControlBar (pBar, !bOuterEdge, FALSE, bOuterEdge);

		pBar->EnableGripper (TRUE);
	}

	m_pRootContainer->CheckSliderVisibility ();
	m_pRootContainer->CalculateRecentSize ();

	return TRUE;
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::AddContainerManager (CBCGPDockingControlBar* pTargetControlBar, 
													DWORD dwAlignment, 
													CBCGPBarContainerManager& srcManager, 
													BOOL bCopy)
{
	CObList lstControlBars;
	CObList lstSliders;

	srcManager.AddControlBarsToList (&lstControlBars, &lstSliders);

	BOOL bLeftBar = FALSE;
	CBCGPBarContainer* pContainer = FindContainer (pTargetControlBar, bLeftBar);

	if (pContainer == NULL)
	{
		return FALSE;
	}

	POSITION posTargetBar = m_lstControlBars.Find (pTargetControlBar);

	if (posTargetBar == NULL)
	{
		return FALSE;
	}

	CBCGPBarContainer* pNewContainer = NULL;
	
	if (!bCopy)
	{
		pNewContainer = srcManager.m_pRootContainer;
	}
	else
	{
		// we must copy containers before SetParent, because Copy sets recent dock info
		// internally and we need the "old" parent for that
		pNewContainer = srcManager.m_pRootContainer->Copy (m_pRootContainer);
		pNewContainer->SetContainerManager (this, TRUE);
		srcManager.RemoveAllControlBarsAndSliders ();
	}

	CWnd* pOldParent = srcManager.GetDockSite ();

	globalUtils.SetNewParent (lstControlBars, m_pDockSite);
	globalUtils.SetNewParent (lstSliders, m_pDockSite);
	
	if (!AddControlBarAndContainer (pTargetControlBar, pNewContainer, dwAlignment))
	{
		// reparent back
		globalUtils.SetNewParent (lstControlBars, pOldParent);
		globalUtils.SetNewParent (lstSliders, pOldParent);
		return FALSE;
	}

	BOOL bInsertBefore = (dwAlignment & CBRS_ALIGN_TOP) || (dwAlignment & CBRS_ALIGN_LEFT);

	// add/insert control bars and sliders from the manager is being added
	if (bInsertBefore)
	{
		for (POSITION pos = lstControlBars.GetHeadPosition (); pos != NULL;)
		{
			CWnd* pWnd = (CWnd*) lstControlBars.GetNext (pos);
			m_lstControlBars.InsertBefore (posTargetBar, pWnd);
		}
	}
	else
	{
		for (POSITION pos = lstControlBars.GetTailPosition (); pos != NULL;)
		{
			CWnd* pWnd = (CWnd*) lstControlBars.GetPrev (pos);
			m_lstControlBars.InsertAfter (posTargetBar, pWnd);
		}
	}

	m_lstSliders.AddTail (&lstSliders);

	POSITION pos = NULL;

	// set new container manager for each slider
	for (pos = lstSliders.GetHeadPosition (); pos != NULL;)
	{
		CBCGPSlider* pSlider = (CBCGPSlider*) lstSliders.GetNext (pos);
		ASSERT_VALID (pSlider);

		pSlider->SetContainerManager (this);
	}
	
	if (!bCopy)
	{
		srcManager.m_bDestroyRootContainer = FALSE;
		srcManager.m_pRootContainer->SetContainerManager (this, TRUE);
	}

	// finally, enable caption for each control bar in the container manager
	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar,  m_lstControlBars.GetNext (pos));
		ASSERT_VALID (pBar);
		
		pBar->EnableGripper (TRUE);
		pBar->RedrawWindow ();
	}

	m_pRootContainer->CheckSliderVisibility ();
	m_pRootContainer->CalculateRecentSize ();
	
	return TRUE;
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::AddContainerManagerToTabWnd (CBCGPDockingControlBar* pTargetControlBar, 
															CBCGPBarContainerManager& srcManager)
{
	CObList lstControlBars;

	srcManager.AddControlBarsToList (&lstControlBars, NULL);

	BOOL bLeftBar = FALSE;
	CBCGPBarContainer* pContainer = FindContainer (pTargetControlBar, bLeftBar);

	if (pContainer == NULL)
	{
		return FALSE;
	}

	POSITION posTargetBar = m_lstControlBars.Find (pTargetControlBar);

	if (posTargetBar == NULL)
	{
		return FALSE;
	}

	CBCGPBaseTabbedBar* pTabbedBar = DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pTargetControlBar);
	for (POSITION pos = lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, lstControlBars.GetNext (pos));
		if (pBar != NULL)
		{
			if (pTabbedBar == NULL)
			{
				pBar->AttachToTabWnd (pTargetControlBar, BCGP_DM_MOUSE, TRUE, 
										(CBCGPDockingControlBar**) &pTabbedBar);
			}
			else
			{
				pBar->AttachToTabWnd (pTabbedBar, BCGP_DM_MOUSE, TRUE, 
										(CBCGPDockingControlBar**) &pTabbedBar);
			}
		}
	}

	return TRUE;
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::InsertControlBar (CBCGPDockingControlBar* pControlBarToInsert,
												 CBCGPDockingControlBar* pTargetControlBar,
												 DWORD dwAlignment,
												 LPCRECT /*lpRect*/,
												 BCGP_DOCK_METHOD /*dockMethod*/)
{
	ASSERT_VALID (this);
	ASSERT (m_pRootContainer != NULL);
	ASSERT_VALID (pControlBarToInsert); 
	ASSERT_KINDOF (CBCGPDockingControlBar, pControlBarToInsert);

	BOOL bResult = FALSE;
	if (pTargetControlBar != NULL)
	{
		POSITION pos = m_lstControlBars.Find (pTargetControlBar);
		if (pos != NULL)
		{
			bResult = AddControlBarAndSlider (pTargetControlBar, pControlBarToInsert, 
												pos, dwAlignment);
		}
		else
		{
			TRACE0 ("TargetControlBar does not belong to the container. Docking failed\n");
			ASSERT (FALSE);
		}
	}
	return bResult;
}
//-----------------------------------------------------------------------------------//
CBCGPDockingControlBar* CBCGPBarContainerManager::AddControlBarToRecentContainer 
													(CBCGPDockingControlBar* pBarToAdd, 
													 CBCGPBarContainer* pRecentContainer)
{
	ASSERT_VALID (this);
	ASSERT_KINDOF (CBCGPDockingControlBar, pBarToAdd);
	ASSERT_VALID (pRecentContainer);

	CBCGPBarContainer::BC_FIND_CRITERIA searchType = 
							CBCGPBarContainer::BC_FIND_BY_CONTAINER;

	CBCGPBarContainer* pContainer = 
		m_pRootContainer->FindSubContainer (pRecentContainer, searchType);

	if (pContainer == NULL)
	{
		return NULL;
	}
	
	if (!pContainer->IsContainerEmpty () && pContainer->GetSlider () == NULL)
	{
		CBCGPSlider* pSlider = CreateSlider (pContainer->GetRecentSliderRect (), 
											  pContainer->GetRecentSliderStyle ());
		pContainer->SetSlider (pSlider);
	}
	
	if (pContainer->IsContainerEmpty ())
	{
		// this container becomes non-empty
		// we need to ckeck parent containers in order to ensure their 
		// slider existance
		CBCGPBarContainer* pParentContainer = pContainer->GetParentContainer ();
		while (pParentContainer != m_pRootContainer &&
				pParentContainer != NULL)
		{
			if (pParentContainer->GetSlider () == NULL && 
				pParentContainer->GetRecentSliderStyle () != 0)
			{
				CBCGPSlider* pSlider = CreateSlider (pParentContainer->GetRecentSliderRect (), 
													 pParentContainer->GetRecentSliderStyle ());
				pParentContainer->SetSlider (pSlider);
			}
			pParentContainer = pParentContainer->GetParentContainer ();
		}
	}

	BOOL bDockSiteIsMiniFrame = m_pDockSite->
			IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd));
	CObList lstRecentListOfBars; 
	lstRecentListOfBars.AddTail (&pBarToAdd->m_recentDockInfo.GetRecentListOfBars (!bDockSiteIsMiniFrame));
	// scan recent list of bars and look for the bar to insert after
	POSITION posRecent = lstRecentListOfBars.Find (pBarToAdd);

	// it may be different from pBarToAdd in case of tabbed window
	CBCGPDockingControlBar* pAddedBar = pContainer->AddControlBar (pBarToAdd);
	if (pAddedBar == pBarToAdd)
	{
		m_pRootContainer->CheckSliderVisibility ();

		while (posRecent != NULL)
		{
			CBCGPDockingControlBar* p = 
				(CBCGPDockingControlBar*) lstRecentListOfBars.GetPrev (posRecent);

			ASSERT_VALID (p);

			POSITION posCurrent = m_lstControlBars.Find (p);
			if (posCurrent != NULL)
			{
				m_lstControlBars.InsertAfter (posCurrent, pAddedBar);
				return pAddedBar;
			}
		}

		m_lstControlBars.AddHead (pAddedBar);

		return pAddedBar;
	}
	

	return pAddedBar;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::StoreRecentDockInfo (CBCGPDockingControlBar* pBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);
	
	BOOL bLeftBar = TRUE;
	CBCGPBarContainer* pContainer = FindContainer (pBar, bLeftBar);

	if (pContainer != NULL)
	{
		pContainer->StoreRecentDockInfo (pBar);
	}
}
//-----------------------------------------------------------------------------------//
CBCGPBaseControlBar* CBCGPBarContainerManager::GetFirstBar () const
{
	ASSERT_VALID (this);
	if (!m_lstControlBars.IsEmpty ())
	{
		return (CBCGPBaseControlBar*) m_lstControlBars.GetHead ();
	}

	return NULL;
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::RemoveControlBarFromContainer (CBCGPDockingControlBar* pControlBar)
{
	ASSERT_VALID (this);
	if (m_pRootContainer == NULL)
	{
		return FALSE;
	}

	
	BOOL bLeftBar = FALSE;
	CBCGPBarContainer* pContainer = FindContainer (pControlBar, bLeftBar);

	if (pContainer == NULL)
	{
		return FALSE;
	}
	
	pContainer->DeleteControlBar (pControlBar, 
		bLeftBar ?  CBCGPBarContainer::BC_FIND_BY_LEFT_BAR : 
					CBCGPBarContainer::BC_FIND_BY_RIGHT_BAR);

	m_pRootContainer->CheckSliderVisibility ();

	CBCGPSlider* pSlider = (CBCGPSlider*) pContainer->GetSlider ();

	if (pSlider != NULL)
	{
		ASSERT (m_lstSliders.Find (pSlider) != NULL);
		pSlider->ShowWindow (SW_HIDE);
	}
	
	POSITION pos = m_lstControlBars.Find (pControlBar);
	
	if (pos != NULL)
	{
		CList<HWND,HWND> lstRecentBarHandles;
		for (POSITION posBar = m_lstControlBars.GetHeadPosition (); posBar != NULL;)
		{
			CWnd* pWnd = DYNAMIC_DOWNCAST (CWnd, m_lstControlBars.GetNext (posBar));
			ASSERT_VALID (pWnd);

			lstRecentBarHandles.AddTail (pWnd->GetSafeHwnd ());
		}

		BOOL bDockSiteIsMiniFrame = 
			m_pDockSite->IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd));
		pControlBar->m_recentDockInfo.SaveListOfRecentBars (lstRecentBarHandles, !bDockSiteIsMiniFrame);
		m_lstControlBars.RemoveAt (pos);
	}

	return TRUE;
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::OnShowControlBar (CBCGPDockingControlBar* /*pBar*/, BOOL /*bShow*/)
{
	if (m_pRootContainer != NULL)
	{
		m_pRootContainer->CheckSliderVisibility ();
	}

	return IsRootContainerVisible ();
}
//-----------------------------------------------------------------------------------//
int CBCGPBarContainerManager::OnSliderMove (CBCGPSlider* pSlider, UINT /*uFlags*/, 
											int nOffset, HDWP& hdwp)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pSlider);

	CSize sizeMinContainer;
	CRect rectContainer;
	m_pRootContainer->GetWindowRect (rectContainer);
	m_pRootContainer->GetMinSize (sizeMinContainer);

	// check whether it is the default slider 
	if (pSlider == m_pDefaultSlider)
	{
		DWORD dwSliderAlignment = pSlider->GetCurrentAlignment ();
		m_pDockSite->ScreenToClient (rectContainer);
		BOOL bIsRTL = m_pDockSite->GetExStyle () & WS_EX_LAYOUTRTL;  
		switch (dwSliderAlignment)
		{
		case CBRS_ALIGN_LEFT:
			if (bIsRTL)
			{
				rectContainer.left += nOffset;
			}
			else
			{
				rectContainer.right += nOffset;
			}
			if (rectContainer.Width () < sizeMinContainer.cx)
			{
				rectContainer.right = rectContainer.left + sizeMinContainer.cx;
			}
			break;
		case CBRS_ALIGN_RIGHT:
			if (bIsRTL)
			{
				rectContainer.right += nOffset;
			}
			else
			{
				rectContainer.left += nOffset;
			}
			if (rectContainer.Width () < sizeMinContainer.cx)
			{
				rectContainer.left = rectContainer.right - sizeMinContainer.cx;
			}
			break;
		case CBRS_ALIGN_TOP:
			rectContainer.bottom += nOffset;
			if (rectContainer.Height () < sizeMinContainer.cy)
			{
				rectContainer.bottom = rectContainer.top + sizeMinContainer.cy;
			}
			break;
		case CBRS_ALIGN_BOTTOM:
			rectContainer.top += nOffset;
			if (rectContainer.Height () < sizeMinContainer.cy)
			{
				rectContainer.top = rectContainer.bottom - sizeMinContainer.cy;
			}
			break;
		}

		ResizeBarContainers (rectContainer, hdwp);

		return 0;
	}
	
	CRect rectSlider;
	pSlider->GetWindowRect (&rectSlider);
	CBCGPBarContainer* pContainer = 
		m_pRootContainer->FindSubContainer (pSlider, CBCGPBarContainer::BC_FIND_BY_SLIDER);
	if (pContainer != NULL)
	{
		return pContainer->OnMoveInternalSlider (nOffset, hdwp);
	}
	
	return 0;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::GetMinMaxOffset (CBCGPSlider* pSlider, int& nMinOffset, 
												int& nMaxOffset, int& nStep)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pSlider);
	ASSERT_VALID (m_pRootContainer);

	nMinOffset = nMaxOffset = 0;  
	nStep = -1;
	
	CRect rectContainer;
	CRect rectSlider;

	pSlider->GetWindowRect (rectSlider);

	if (pSlider->IsDefault ())
	{
		CRect rectMainClientArea;
		ASSERT_VALID (pSlider->GetDockSite ());

		CBCGPDockManager* pDockManager = 
			globalUtils.GetDockManager (pSlider->GetDockSite ());

		ASSERT_VALID (pDockManager);

		m_pRootContainer->GetWindowRect (rectContainer);

		CSize sizeMin;
		m_pRootContainer->GetMinSize (sizeMin);
		rectContainer.DeflateRect  (sizeMin);

		rectMainClientArea = pDockManager->GetClientAreaBounds ();
		pSlider->GetDockSite ()->ClientToScreen (rectMainClientArea);
		rectMainClientArea.DeflateRect (g_nSliderSpacingForMove, g_nSliderSpacingForMove);
	
		DWORD dwAlignment = pSlider->GetCurrentAlignment ();
		if (dwAlignment & CBRS_ALIGN_LEFT)
		{
			nMinOffset = rectContainer.left - rectSlider.left + 1;
			nMaxOffset = rectMainClientArea.right - rectSlider.right - 1;
		}
		else if (dwAlignment & CBRS_ALIGN_TOP)
		{
			nMinOffset = rectContainer.top - rectSlider.top + 1;
			nMaxOffset = rectMainClientArea.bottom - rectSlider.bottom - 1;
		}
		else if (dwAlignment & CBRS_ALIGN_RIGHT)
		{
			nMinOffset = rectMainClientArea.left - rectSlider.left + 1;
			nMaxOffset = rectContainer.right - rectSlider.right - 1;
		}
		else if (dwAlignment & CBRS_ALIGN_BOTTOM)
		{
			nMinOffset = rectMainClientArea.top - rectSlider.top + 1;
			nMaxOffset = rectContainer.bottom - rectSlider.bottom - 1;
		}
		
		nStep = m_pRootContainer->GetResizeStep ();
	}	
	else
	{
		CBCGPBarContainer* pContainer = 
			m_pRootContainer->FindSubContainer (pSlider, CBCGPBarContainer::BC_FIND_BY_SLIDER);

		if (pContainer == NULL)
		{
			return;
		}

		pContainer->GetWindowRect (rectContainer);

		CSize sizeMinLeft;
		CSize sizeMinRight;
		pContainer->GetMinSizeLeft (sizeMinLeft);
		pContainer->GetMinSizeRight (sizeMinRight);

		if (pSlider->IsHorizontal ())
		{
			nMinOffset = rectContainer.top - rectSlider.top + sizeMinLeft.cy + 1;
			nMaxOffset = rectContainer.bottom - rectSlider.bottom - sizeMinRight.cy - 1;
		}
		else
		{
			nMinOffset = rectContainer.left - rectSlider.left + sizeMinLeft.cx;
			nMaxOffset = rectContainer.right - rectSlider.right - sizeMinRight.cx - 1;
		}			
		
		nStep = pContainer->GetResizeStep ();
	}
	
}
//-----------------------------------------------------------------------------------//
CBCGPSlider* CBCGPBarContainerManager::CreateSlider (CRect rectSlider, 
													 DWORD dwSliderStyle, 
													 int nSliderID)
{
	ASSERT_VALID (this);
	
	CBCGPSlider* pSlider = DYNAMIC_DOWNCAST (CBCGPSlider, CBCGPSlider::m_pSliderRTC->CreateObject ());
	ASSERT_VALID (pSlider);

	pSlider->Init ();

	if (nSliderID == -1)
	{
		nSliderID = g_nSliderID;
		g_nSliderID ++;
	}
	
	if (nSliderID >= g_nSliderID)
	{
		g_nSliderID = nSliderID;
		g_nSliderID++;
	}

	for (POSITION pos = m_lstSliders.GetHeadPosition (); pos != NULL;)
	{
		CBCGPSlider* pNextSlider = (CBCGPSlider*) m_lstSliders.GetNext (pos);
		if (pNextSlider->GetDlgCtrlID () == nSliderID)
		{
			nSliderID = g_nSliderID;
			g_nSliderID++;
		}
	}

	if (!pSlider->CreateEx (0, dwSliderStyle, rectSlider, m_pDockSite, nSliderID, NULL))
	{
		TRACE0 ("CBCGPDockSiteResizableRow: Failed to create slider");
		delete pSlider;
		return NULL;
	}
	pSlider->ShowWindow (SW_SHOW);
	pSlider->SetContainerManager (this);

	m_lstSliders.AddTail (pSlider);	
	return pSlider;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::RemoveSlider (CBCGPSlider* pSlider)
{
	POSITION pos = m_lstSliders.Find (pSlider);
	if (pos != NULL)
	{
		m_lstSliders.RemoveAt (pos);
		pSlider->SetContainerManager (NULL);
	}

	if (m_pRootContainer != NULL)
	{
		CBCGPBarContainer* pContainer = 
			m_pRootContainer->FindSubContainer (pSlider, 
												CBCGPBarContainer::BC_FIND_BY_SLIDER);

		if (pContainer != NULL)
		{
			pContainer->SetSlider (NULL);
		}
	}
}
//-----------------------------------------------------------------------------------//
UINT CBCGPBarContainerManager::FindBar (CPoint /*pt*/, CBCGPControlBar** /*ppBar*/, POSITION& /*posRet*/)
{
	return (UINT) HTERROR;
}
//-----------------------------------------------------------------------------------//
UINT CBCGPBarContainerManager::FindBar (CRect /*rect*/, CBCGPControlBar** /*ppBar*/, POSITION& /*posRet*/)
{
	return (UINT) HTERROR;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::GetWindowRect (CRect& rect) const
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pRootContainer);

	m_pRootContainer->GetWindowRect (rect);
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::GetAvailableSpace (CRect& rect) const
{
	ASSERT_VALID (this);
	CRect rectUnited;
	rectUnited.SetRectEmpty ();
	CRect rectBar;
	rectBar.SetRectEmpty ();

	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CWnd* pWnd = (CWnd*) m_lstControlBars.GetNext (pos);
		pWnd->GetWindowRect (rectBar);
		rectUnited.UnionRect (&rectUnited, &rectBar);
	}

	for (pos = m_lstSliders.GetHeadPosition (); pos != NULL;)
	{
		CWnd* pWnd = (CWnd*) m_lstSliders.GetNext (pos);
		pWnd->GetWindowRect (rectBar);
		rectUnited.UnionRect (&rectUnited, &rectBar);
	}

	GetWindowRect (rect);
	rect.SubtractRect (&rect, &rectUnited);
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::CalcRects (CRect& rectOriginal, 
										  CRect& rectInserted,
										  CRect& rectSlider,
										  DWORD& dwSliderStyle,
										  DWORD dwAlignment, 
										  CSize /*sizeMinOriginal*/,
										  CSize sizeMinInserted) 
										  
{
	if (rectInserted.Width () < sizeMinInserted.cx)
	{
		rectInserted.right = rectInserted.left + sizeMinInserted.cx;
	}

	if (rectInserted.Height () < sizeMinInserted.cy)
	{
		rectInserted.bottom = rectInserted.top + sizeMinInserted.cy;
	}

	// calculate the width/height (size) of both rectangles, slider's boundaries and orientation
	int nNewSize = 0;

	if (dwAlignment & CBRS_ORIENT_HORZ)
	{
		// align the rectangle of the bar to insert by the width of the sell
		rectSlider.left = rectInserted.left = rectOriginal.left;
		rectSlider.right = rectInserted.right = rectOriginal.right;
		
		if (rectInserted.Height () > rectOriginal.Height () / 2)
		{
			nNewSize = rectOriginal.Height () / 2;
		}
		else 
		{
			nNewSize = rectInserted.Height ();
		}
		dwSliderStyle = CBCGPSlider::SS_HORZ;
	}
	else
	{
		// align the rectangle of the bar to insert by the height of the sell
		rectSlider.top = rectInserted.top = rectOriginal.top;
		rectSlider.bottom = rectInserted.bottom = rectOriginal.bottom;

		if (rectInserted.Width () > rectOriginal.Width () / 2)
		{
			nNewSize = rectOriginal.Width () / 2;
		}
		else 
		{
			nNewSize = rectInserted.Width ();
		}
		dwSliderStyle = CBCGPSlider::SS_VERT;

	}


	// set rects for both rectangles and slider
	switch (dwAlignment & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_TOP:
		rectInserted.top = rectOriginal.top;
		rectInserted.bottom = rectInserted.top + nNewSize;
		rectOriginal.top = rectInserted.bottom + CBCGPSlider::GetDefaultWidth ();
		rectSlider.top = rectInserted.bottom; 
		rectSlider.bottom = rectOriginal.top; 
		break;
	case CBRS_ALIGN_BOTTOM:
		rectInserted.top = rectOriginal.bottom - nNewSize;
		rectInserted.bottom = rectOriginal.bottom;
		rectOriginal.bottom = rectInserted.top - CBCGPSlider::GetDefaultWidth ();
		rectSlider.top = rectOriginal.bottom;
		rectSlider.bottom = rectInserted.top;
		break;
	case CBRS_ALIGN_LEFT:
		rectInserted.left = rectOriginal.left;
		rectInserted.right = rectInserted.left + nNewSize;
		rectOriginal.left = rectInserted.right + CBCGPSlider::GetDefaultWidth ();
		rectSlider.left = rectInserted.right;
		rectSlider.right = rectOriginal.left;
		break;
	case CBRS_ALIGN_RIGHT:
		rectInserted.right = rectOriginal.right;
		rectInserted.left = rectInserted.right - nNewSize;
		rectOriginal.right = rectInserted.left - CBCGPSlider::GetDefaultWidth ();
		rectSlider.left = rectOriginal.right;
		rectSlider.right = rectInserted.left;
		break;
	}
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::AddControlBarAndContainer (CBCGPDockingControlBar* pBarOriginal, 
														  CBCGPBarContainer* pContainerToInsert, 
														  DWORD dwAlignment)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBarOriginal); 
	ASSERT_VALID (pContainerToInsert);
	ASSERT (dwAlignment & CBRS_ALIGN_ANY);

	if (m_pRootContainer == NULL)
	{
		TRACE0 ("The root container must be created first (call Create) \r\n");
		return FALSE;
	}

	CRect rectBarOriginal;
	CRect rectContainerToInsert;
	CRect rectSlider; rectSlider.SetRectEmpty ();

	CSize sizeMinOriginal;
	pBarOriginal->GetMinSize (sizeMinOriginal);

	CSize sizeMinToInsert;
	pContainerToInsert->GetMinSize (sizeMinToInsert);

	pBarOriginal->GetWindowRect (rectBarOriginal);
	pContainerToInsert->GetWindowRect (rectContainerToInsert);
	
	DWORD dwSliderStyle = CBCGPSlider::SS_HORZ;

	m_pDockSite->ScreenToClient (rectBarOriginal);
	m_pDockSite->ScreenToClient (rectContainerToInsert);
	m_pDockSite->ScreenToClient (rectSlider);

	BOOL bIsRTL = m_pDockSite->GetExStyle () & WS_EX_LAYOUTRTL;

	CalcRects (rectBarOriginal, rectContainerToInsert, rectSlider, dwSliderStyle, 
				dwAlignment, sizeMinOriginal, sizeMinToInsert);

	pBarOriginal->MoveWindow (rectBarOriginal);
	
	HDWP hdwp = NULL;
	pContainerToInsert->ResizeContainer (rectContainerToInsert, hdwp);
	pContainerToInsert->Move (rectContainerToInsert.TopLeft ());
	
	// it's not a default slider
	CBCGPSlider* pSlider = CreateSlider (rectSlider, dwSliderStyle);
	if (pSlider == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPBarContainer* pContainer = NULL;
	if (m_pContainerRTC == NULL) 
	{
		pContainer = new CBCGPBarContainer ();
	}
	else
	{
		pContainer = (CBCGPBarContainer*) m_pContainerRTC->CreateObject ();
	}

	pContainer->SetContainerManager (this);
	pContainer->SetSlider (pSlider);

	BOOL bRightNode = (dwAlignment & CBRS_ALIGN_BOTTOM) || (dwAlignment & CBRS_ALIGN_RIGHT);	

	if (bIsRTL)
	{
		bRightNode = dwAlignment & CBRS_ALIGN_LEFT;
	}
	
	pContainer->SetBar (pBarOriginal, bRightNode);
	pContainer->SetContainer (pContainerToInsert, !bRightNode);

	pSlider->BringWindowToTop ();

	return m_pRootContainer->AddSubContainer (pContainer, bRightNode);
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::AddControlBarAndSlider (CBCGPDockingControlBar* pBarOriginal, 
													  CBCGPDockingControlBar* pBarToInsert, 
													  POSITION posNearestBar, 
													  DWORD dwAlignment)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBarOriginal); 
	ASSERT_VALID (pBarToInsert);
	ASSERT_VALID (m_pRootContainer);
	ASSERT_KINDOF (CBCGPDockingControlBar, pBarOriginal);
	ASSERT_KINDOF (CBCGPDockingControlBar, pBarToInsert);
	ASSERT (dwAlignment & CBRS_ALIGN_ANY);

	if (m_pRootContainer == NULL)
	{
		TRACE0 ("The root container must be created first (call Create) \r\n");
		return FALSE;
	}

	// insert the new bar into the list of control bars accordig to its 
	// hit test (side) relatively to the nearest bar
	switch (dwAlignment & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_TOP:
	case CBRS_ALIGN_LEFT:
		m_lstControlBars.InsertBefore (posNearestBar, pBarToInsert);
		break;
	case CBRS_ALIGN_BOTTOM:
	case CBRS_ALIGN_RIGHT:
		m_lstControlBars.InsertAfter (posNearestBar, pBarToInsert);
		break;
	default:
		ASSERT (FALSE);
		return FALSE;
	}

	CRect rectBarOriginal;
	CRect rectBarToInsert;
	CRect rectSlider;

	CSize sizeMinOriginal;
	pBarOriginal->GetMinSize (sizeMinOriginal);

	CSize sizeMinToInsert;
	pBarToInsert->GetMinSize (sizeMinToInsert);

	pBarOriginal->GetWindowRect (rectBarOriginal);
	pBarToInsert->GetWindowRect (rectBarToInsert);

	if (rectBarToInsert.Width () < sizeMinToInsert.cx)
	{
		rectBarToInsert.right = rectBarToInsert.left + sizeMinToInsert.cx;
	}

	if (rectBarToInsert.Height () < sizeMinToInsert.cy)
	{
		rectBarToInsert.bottom = rectBarToInsert.top + sizeMinToInsert.cy;
	}

	// calculate the width/height (size) of both rectangles, slider's boundaries and orientation
	DWORD dwSliderStyle = 0;
	int nNewSize = 0;

	if (dwAlignment & CBRS_ORIENT_HORZ)
	{
		// align the rectangle of the bar to insert by the width of the sell
		rectSlider.left = rectBarToInsert.left = rectBarOriginal.left;
		rectSlider.right = rectBarToInsert.right = rectBarOriginal.right;
		
		if (rectBarToInsert.Height () > rectBarOriginal.Height () / 2) //- sizeMinOriginal.cy * 2- CBCGPSlider::GetDefaultWidth ())
		{
			nNewSize = rectBarOriginal.Height () / 2; //  - sizeMinOriginal.cy * 4 - CBCGPSlider::GetDefaultWidth ();
		}
		else 
		{
			nNewSize = rectBarToInsert.Height ();
		}
		dwSliderStyle = CBCGPSlider::SS_HORZ;
	}
	else
	{
		// align the rectangle of the bar to insert by the height of the sell
		rectSlider.top = rectBarToInsert.top = rectBarOriginal.top;
		rectSlider.bottom = rectBarToInsert.bottom = rectBarOriginal.bottom;

		if (rectBarToInsert.Width () > rectBarOriginal.Width () / 2) //- sizeMinOriginal.cx * 2 - CBCGPSlider::GetDefaultWidth ())
		{
			nNewSize = rectBarOriginal.Width () / 2; //- sizeMinOriginal.cx * 4;
		}
		else 
		{
			nNewSize = rectBarToInsert.Width ();
		}
		dwSliderStyle = CBCGPSlider::SS_VERT;

	}

	BOOL bRightNode = FALSE;
	CBCGPDockingControlBar* pLeftBar = NULL;
	CBCGPDockingControlBar* pRightBar = NULL;	

	m_pDockSite->ScreenToClient (rectBarOriginal);
	m_pDockSite->ScreenToClient (rectBarToInsert);
	m_pDockSite->ScreenToClient (rectSlider);

	BOOL bIsRTL = m_pDockSite->GetExStyle () & WS_EX_LAYOUTRTL;

	// set rects for both rectangles and slider
	switch (dwAlignment & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_TOP:
		rectBarToInsert.top = rectBarOriginal.top;
		rectBarToInsert.bottom = rectBarToInsert.top + nNewSize;
		rectBarOriginal.top = rectBarToInsert.bottom + CBCGPSlider::GetDefaultWidth ();
		rectSlider.top = rectBarToInsert.bottom; 
		rectSlider.bottom = rectBarOriginal.top; 
		pLeftBar = pBarToInsert;
		pRightBar = pBarOriginal;
		break;
	case CBRS_ALIGN_BOTTOM:
		rectBarToInsert.top = rectBarOriginal.bottom - nNewSize;
		rectBarToInsert.bottom = rectBarOriginal.bottom;
		rectBarOriginal.bottom = rectBarToInsert.top - CBCGPSlider::GetDefaultWidth ();
		rectSlider.top = rectBarOriginal.bottom;
		rectSlider.bottom = rectBarToInsert.top;
		dwSliderStyle = CBCGPSlider::SS_HORZ;
		pLeftBar = pBarOriginal;
		pRightBar = pBarToInsert;
		bRightNode = TRUE;
		break;
	case CBRS_ALIGN_LEFT:
		if (bIsRTL)
		{
			rectBarToInsert.right = rectBarOriginal.right;
			rectBarToInsert.left = rectBarToInsert.right - nNewSize;
			rectBarOriginal.right = rectBarToInsert.left - CBCGPSlider::GetDefaultWidth ();
			rectSlider.left = rectBarOriginal.right;
			rectSlider.right = rectBarToInsert.left;
			pLeftBar = pBarOriginal;
			pRightBar = pBarToInsert;
			bRightNode = TRUE;
		}
		else
		{
			rectBarToInsert.left = rectBarOriginal.left;
			rectBarToInsert.right = rectBarToInsert.left + nNewSize;
			rectBarOriginal.left = rectBarToInsert.right + CBCGPSlider::GetDefaultWidth ();
			rectSlider.left = rectBarToInsert.right; 
			rectSlider.right = rectBarOriginal.left; 
			pLeftBar = pBarToInsert;
			pRightBar = pBarOriginal;
		}	
		
		break;
	case CBRS_ALIGN_RIGHT:
		if (bIsRTL)
		{
			rectBarToInsert.left = rectBarOriginal.left;
			rectBarToInsert.right = rectBarToInsert.left + nNewSize;
			rectBarOriginal.left = rectBarToInsert.right + CBCGPSlider::GetDefaultWidth ();
			rectSlider.left = rectBarToInsert.right; 
			rectSlider.right = rectBarOriginal.left; 
			pLeftBar = pBarToInsert;
			pRightBar = pBarOriginal;

		}
		else
		{
			rectBarToInsert.right = rectBarOriginal.right;
			rectBarToInsert.left = rectBarToInsert.right - nNewSize;
			rectBarOriginal.right = rectBarToInsert.left - CBCGPSlider::GetDefaultWidth ();
			rectSlider.left = rectBarOriginal.right;
			rectSlider.right = rectBarToInsert.left;
			pLeftBar = pBarOriginal;
			pRightBar = pBarToInsert;
			bRightNode = TRUE;
		}
		break;
	}
	
	pBarOriginal->SetWindowPos (NULL, rectBarOriginal.left, rectBarOriginal.top, 
									  rectBarOriginal.Width (), rectBarOriginal.Height (), 
									  SWP_NOZORDER | SWP_NOACTIVATE);

	pBarToInsert->SetWindowPos (NULL, rectBarToInsert.left, rectBarToInsert.top, 
									  rectBarToInsert.Width (), rectBarToInsert.Height (), 
									  SWP_NOZORDER | SWP_NOACTIVATE); 
	// it's not a default slider

	CBCGPSlider* pSlider = CreateSlider (rectSlider, dwSliderStyle);
	if (pSlider == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPBarContainer* pContainer = NULL;
	if (m_pContainerRTC == NULL) 
	{
		pContainer = new CBCGPBarContainer (this, pLeftBar, pRightBar, pSlider);
	}
	else
	{
		pContainer = (CBCGPBarContainer*) m_pContainerRTC->CreateObject ();
		pContainer->SetContainerManager (this);
		pContainer->SetBar (pLeftBar, TRUE);
		pContainer->SetBar (pRightBar, FALSE);
		pContainer->SetSlider (pSlider);
	}
	
	return m_pRootContainer->AddSubContainer (pContainer, bRightNode);
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::ResizeBarContainers (UINT nSide, BOOL bExpand, int nOffset, 
													HDWP& hdwp)
{
	ASSERT_VALID (this);
	if (m_pRootContainer != NULL)
	{
		bool bStretchHorz = (nSide == WMSZ_RIGHT || nSide  == WMSZ_LEFT);
		bool bLeftBar = true;
		nOffset *= bExpand ? 1 : (-1);
		
		m_pRootContainer->StretchContainer (nOffset, bStretchHorz, bLeftBar, TRUE, hdwp);
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::ResizeBarContainers (CRect rect, HDWP& hdwp)
{
	ASSERT_VALID (this);
	if (m_pRootContainer != NULL)
	{
		m_pRootContainer->ResizeContainer (rect, hdwp);
	}
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::ReplaceControlBar (CBCGPDockingControlBar* pBarOld, 
												  CBCGPDockingControlBar* pBarNew)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBarOld);
	ASSERT_VALID (pBarNew);
	ASSERT_KINDOF (CBCGPDockingControlBar, pBarOld);
	ASSERT_KINDOF (CBCGPDockingControlBar, pBarNew);

	POSITION pos = m_lstControlBars.Find (pBarOld);

	if (pos != NULL)
	{
		BOOL bLeftBar = FALSE;
		CBCGPBarContainer* pContainer = FindContainer (pBarOld, bLeftBar);

		if (pContainer != NULL)
		{
			pContainer->SetBar (pBarNew, bLeftBar);

			m_lstControlBars.InsertAfter (pos, pBarNew);
			m_lstControlBars.RemoveAt (pos);
		}
	}
	else
	{
		m_lstControlBars.AddTail (pBarNew);
	}

	return TRUE;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::SetResizeMode (BOOL bResize)
{
	ASSERT_VALID (this);
	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pBar = 
			(CBCGPDockingControlBar*) m_lstControlBars.GetNext (pos);
		ASSERT_VALID (pBar);
		pBar->SetResizeMode (bResize);
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::Serialize (CArchive& ar)
{
	ASSERT_VALID (this);
	CObject::Serialize (ar);

	if (ar.IsStoring ())
	{
		int nSliderCount = (int) m_lstSliders.GetCount ();
		m_pRootContainer->ReleaseEmptyContainer ();

		nSliderCount = (int) m_lstSliders.GetCount ();

		m_pRootContainer->Serialize (ar);

		ar << (int) m_lstControlBars.GetCount (); 
		for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
		{
			CWnd* pNextBar = (CWnd*) m_lstControlBars.GetNext (pos);
			ASSERT_VALID (pNextBar);

			int nBarID = pNextBar->GetDlgCtrlID ();
			if (nBarID != -1)
			{
				ar << nBarID;
			}
			else 
			{
				// this is tab control bar - identify it by its first tabbed bar
				CBCGPBaseTabbedBar* pTabBar = DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pNextBar);
				ASSERT_VALID (pTabBar);
				CWnd* pWnd  = pTabBar->FindBarByTabNumber (0);

				if (pWnd != NULL)
				{
					int nTabbedBarID = pWnd->GetDlgCtrlID ();

					ASSERT (nTabbedBarID != -1);
					ar << nBarID;
					ar << nTabbedBarID;
				}
			}
		}
	}
	else
	{
		m_pRootContainer->Serialize (ar);

		// rewrite to conform with miniframe (m_pDefaultSlider is null there) !!!
		CBCGPDockManager* pDockManager = NULL;

		if (m_pDefaultSlider != NULL)
		{
			pDockManager = globalUtils.GetDockManager (m_pDefaultSlider->GetDockSite ());
		}
		else if (m_pDockSite->IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd)))
		{
			pDockManager = globalUtils.GetDockManager (m_pDockSite->GetParent ());
		}

		if (pDockManager == NULL)
		{
			TRACE0 ("Unexpected NULL pointer to dock manager. Serialization failed.\n");
			throw new CArchiveException;
			return;
		}

		int nCount = 0;

		// load control bar id's
		ar >> nCount;

		int nControlBarID = 0;
		for (int i = 0; i < nCount; i++)
		{
			ar >> nControlBarID;

			// -1 means tabbed control bar, these bars are stored and loaded by containers
			if (nControlBarID != -1)
			{
				CBCGPDockingControlBar* pBar = 
					DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
										pDockManager->FindBarByID (nControlBarID, TRUE));
				if (pBar != NULL)
				{
					ASSERT_VALID (pBar);

					m_lstControlBars.AddTail (pBar);

					m_pRootContainer->SetUpByID (nControlBarID, pBar);
				}
			}
			else
			{
				// load the first tabbed bar id
				ar >> nControlBarID;

				CBCGPDockingControlBar* pBar = 
								m_pRootContainer->FindTabbedBar ((UINT) nControlBarID);

				if (pBar != NULL)
				{
					m_lstControlBars.AddTail (pBar);
				}
			}
		}
	}
}
//-----------------------------------------------------------------------------------//
CBCGPDockingControlBar* CBCGPBarContainerManager::FindTabbedBar (UINT nID)
{
	ASSERT_VALID (this);
	if (m_pRootContainer != NULL)
	{
		return m_pRootContainer->FindTabbedBar (nID);
	}
	return NULL;
}
//-----------------------------------------------------------------------------------//
CBCGPBarContainer* CBCGPBarContainerManager::FindContainer (CBCGPDockingControlBar* pBar, 
															BOOL& bLeftBar)
{
	ASSERT_VALID (this);
	if (m_pRootContainer != NULL)
	{
		bLeftBar = TRUE;
		CBCGPBarContainer::BC_FIND_CRITERIA barType = CBCGPBarContainer::BC_FIND_BY_LEFT_BAR;
		CBCGPBarContainer* pContainer = m_pRootContainer->FindSubContainer (pBar, barType);
		if (pContainer == NULL)
		{
			barType = CBCGPBarContainer::BC_FIND_BY_RIGHT_BAR;
			pContainer = m_pRootContainer->FindSubContainer (pBar, barType);
			bLeftBar = FALSE;
		}
		return pContainer;
	}
	return NULL;
}
//-----------------------------------------------------------------------------------//
// Look for control bar that contains point according to secitivity: if we're looking
// for the exact bar, the point must be in the area between bars' bounds and deflated bars'
// bounds; otherwise the point must be inside inflated bars' window rectangle
//-----------------------------------------------------------------------------------//
CBCGPDockingControlBar* CBCGPBarContainerManager::ControlBarFromPoint (CPoint point, 
																	   int nSensitivity, 
																	   BOOL bExactBar,
																	   BOOL& bIsTabArea,
																	   BOOL& bCaption)
{
	ASSERT_VALID (this);
	
	bIsTabArea = FALSE;


	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, m_lstControlBars.GetNext (pos));

		CRect rectWnd;
		pBar->GetWindowRect (rectWnd);

		CRect rectTabAreaTop;
		CRect rectTabAreaBottom;
		pBar->GetTabArea (rectTabAreaTop, rectTabAreaBottom);

		if (rectTabAreaTop.PtInRect (point) ||
			rectTabAreaBottom.PtInRect (point))
		{
			bIsTabArea = TRUE;
			return pBar;
		}

		if (pBar->HitTest (point, TRUE) == HTCAPTION)
		{
			bCaption = TRUE;
			return pBar;
		}

		int nCaptionHeight = pBar->GetCaptionHeight ();
		rectWnd.top += nCaptionHeight;
		rectWnd.bottom -= rectTabAreaBottom.Height ();


		if (rectWnd.PtInRect (point))
		{
            CWnd* pWnd = pBar->GetParent ();
			ASSERT_VALID (pWnd);

            CBCGPDockManager* pDockManager = NULL;
            CBCGPSmartDockingManager* pSDManager = NULL;

            if ((pDockManager = globalUtils.GetDockManager (pWnd)) != NULL
                && (pSDManager = pDockManager->GetSDManagerPermanent ()) != NULL
                && pSDManager->IsStarted())
            {
                CBCGPSmartDockingMarker::SDMarkerPlace m_nHiliteSideNo = pSDManager->GetHilitedMarkerNo ();
                if (m_nHiliteSideNo >= CBCGPSmartDockingMarker::sdCLEFT
                    && m_nHiliteSideNo <= CBCGPSmartDockingMarker::sdCMIDDLE) // if central group is selected
                {
                    bCaption = (m_nHiliteSideNo == CBCGPSmartDockingMarker::sdCMIDDLE);
                }

                return pBar;
            }

			rectWnd.InflateRect (-nSensitivity, -nSensitivity);
			if (!rectWnd.PtInRect (point) || nSensitivity == 0)
			{
				return pBar;
			}
		}
	}

	if (!bExactBar)
	{
		for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPDockingControlBar* pBar = 
				DYNAMIC_DOWNCAST (CBCGPDockingControlBar, m_lstControlBars.GetNext (pos));

			CRect rectWnd;
			pBar->GetWindowRect (rectWnd);

			rectWnd.InflateRect (nSensitivity, nSensitivity);
			if (rectWnd.PtInRect (point))
			{
				return pBar;
			}
		}
	}
	return NULL;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::GetMinSize (CSize& size)
{
	ASSERT_VALID (this);
	size.cx = size.cy = 0;

	if (m_pRootContainer != NULL)
	{
		m_pRootContainer->GetMinSize (size);
	}
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::IsRootContainerVisible () const
{
	ASSERT_VALID (this);
	if (m_pRootContainer != NULL)
	{
		return m_pRootContainer->IsContainerVisible ();
	}

	return FALSE;
}
//-----------------------------------------------------------------------------------//
int CBCGPBarContainerManager::GetVisibleBarCount () const
{
	ASSERT_VALID (this);
	int nCount = 0;

	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPBaseControlBar, m_lstControlBars.GetNext (pos));
		ASSERT_VALID (pWnd);

		if (pWnd->IsBarVisible ())
		{
			nCount++;
		}
	}

	return nCount;
}
//-----------------------------------------------------------------------------------//
CWnd* CBCGPBarContainerManager::GetFirstVisibleBar () const
{
	ASSERT_VALID (this);
	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPBaseControlBar, m_lstControlBars.GetNext (pos));
		ASSERT_VALID (pWnd);

		if (pWnd->IsBarVisible ())
		{
			return pWnd;
		}
	}	
	return NULL;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::EnableGrippers (BOOL bEnable)
{
	ASSERT_VALID (this);
	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, m_lstControlBars.GetNext (pos));

		if (pWnd != NULL)
		{
			pWnd->EnableGripper (bEnable);
		}
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::HideAll ()
{
	ASSERT_VALID (this);

	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CWnd* pWnd = DYNAMIC_DOWNCAST (CWnd, m_lstControlBars.GetNext (pos));

		if (pWnd != NULL)
		{
			pWnd->ShowWindow (SW_HIDE);
		}
	}

	for (pos = m_lstSliders.GetHeadPosition (); pos != NULL;)
	{
		CWnd* pWnd = DYNAMIC_DOWNCAST (CWnd, m_lstSliders.GetNext (pos));

		if (pWnd != NULL)
		{
			pWnd->ShowWindow (SW_HIDE);
		}
	}
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::DoesContainFloatingBar ()
{
	ASSERT_VALID (this);
	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPBaseControlBar, m_lstControlBars.GetNext (pos));
		if (pWnd->CanFloat ())
		{
			return TRUE;
		}
	}

	return FALSE;
}
//-----------------------------------------------------------------------------------//
int	CBCGPBarContainerManager::GetNodeCount () const
{
	ASSERT_VALID (this);

	if (m_pRootContainer == NULL)
	{
		return 0;
	}

	return m_pRootContainer->GetNodeCount ();
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::IsEmpty () const 
{
	return m_lstControlBars.IsEmpty ();
}
//-----------------------------------------------------------------------------------//
int CBCGPBarContainerManager::GetTotalRefCount () const
{
	if (m_pRootContainer == NULL)
	{
		return 0;
	}
	return m_pRootContainer->GetTotalReferenceCount ();
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::SetDefaultSliderForControlBars (CBCGPSlider* /*pSlider*/)
{
	ASSERT_VALID (this);
	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, m_lstControlBars.GetNext (pos));
		if (pWnd != NULL)
		{	
			pWnd->SetDefaultSlider (NULL);
		}
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::AddControlBarsToList (CObList* plstControlBars, 
													 CObList* plstSliders)
{
	ASSERT_VALID (this);
	if (plstControlBars != NULL)
	{
		for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
		{
			CWnd* pWnd = 
				DYNAMIC_DOWNCAST (CWnd, m_lstControlBars.GetNext (pos));
			ASSERT_VALID (pWnd);
	
			if (pWnd->GetStyle () & WS_VISIBLE) 
			{	
				plstControlBars->AddTail (pWnd);
			}
		}
	}
	
	if (plstSliders != NULL)
	{
		for (POSITION pos = m_lstSliders.GetHeadPosition (); pos != NULL;)
		{
			CWnd* pWnd = 
				DYNAMIC_DOWNCAST (CWnd, m_lstSliders.GetNext (pos));
			ASSERT_VALID (pWnd);
			
			if (pWnd->GetStyle () & WS_VISIBLE) 
			{	
				plstSliders->AddTail (pWnd);
			}
		}
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::RemoveAllControlBarsAndSliders ()
{
	ASSERT_VALID (this);
	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;
		CBCGPBaseControlBar* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPBaseControlBar, m_lstControlBars.GetNext (pos));
		ASSERT_VALID (pWnd);

		if (pWnd->IsBarVisible ())
		{
			m_lstControlBars.RemoveAt (posSave);
		}
	}

	for (pos = m_lstSliders.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;
		CBCGPBaseControlBar* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPBaseControlBar, m_lstControlBars.GetNext (pos));
		ASSERT_VALID (pWnd);

		if (pWnd->IsBarVisible ())
		{
			m_lstSliders.RemoveAt (posSave);
		}
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::AddControlBarToList (CBCGPDockingControlBar* pControlBarToAdd) 
{
	ASSERT_VALID (this);
	ASSERT_VALID (pControlBarToAdd);

	m_lstControlBars.AddTail (pControlBarToAdd);
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::DoesAllowDynInsertBefore () const
{
	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPBaseControlBar, m_lstControlBars.GetNext (pos));
		ASSERT_VALID (pWnd);

		if (pWnd->DoesAllowDynInsertBefore ())
		{
			return TRUE;
		}
	}

	return FALSE;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::NotifySlider ()
{
	if (m_pDefaultSlider != NULL)
	{
		m_pDefaultSlider->NotifyAboutRelease ();
	}
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::CheckForMiniFrameAndCaption (CPoint point, 
									CBCGPDockingControlBar** ppTargetControlBar)
{
	CBCGPMultiMiniFrameWnd* pMiniFrameWnd = 
		DYNAMIC_DOWNCAST (CBCGPMultiMiniFrameWnd, m_pDockSite);

	*ppTargetControlBar = NULL;

	if (pMiniFrameWnd == NULL)
	{
		return FALSE;
	}

	if (GetVisibleBarCount () > 1)
	{
		return FALSE;
	}

	CRect rectCaption;
	pMiniFrameWnd->GetCaptionRect (rectCaption);
	pMiniFrameWnd->ScreenToClient (&point);


	CRect rectBorderSize;
	pMiniFrameWnd->CalcBorderSize (rectBorderSize);

	point.Offset (rectBorderSize.left, 
				  rectBorderSize.top + pMiniFrameWnd->GetCaptionHeight ());

	if (rectCaption.PtInRect (point))
	{
		*ppTargetControlBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, GetFirstVisibleBar ());
	}

	return (*ppTargetControlBar != NULL);
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::RemoveNonValidBars ()
{
	if (m_pRootContainer != NULL)
	{
		m_pRootContainer->RemoveNonValidBars ();
	}
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::CheckAndRemoveNonValidBar (CWnd* pWnd)
{
	if (pWnd != NULL)
	{
		UINT nControlID = pWnd->GetDlgCtrlID ();
		if (IsWindow (pWnd->GetSafeHwnd ()) && nControlID != -1)
		{
			return TRUE;
		}

		CBCGPBaseTabbedBar* pBaseTabbedBar = 
				DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pWnd);

		if (pBaseTabbedBar != NULL && pBaseTabbedBar->GetTabsNum () > 0)
		{
			return TRUE;
		}
	}

	POSITION pos = m_lstControlBars.Find (pWnd);
	if (pos != NULL)
	{
		m_lstControlBars.RemoveAt (pos);
	}

	return FALSE;
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainerManager::CanBeAttached () const
{
	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPBaseControlBar, m_lstControlBars.GetNext (pos));
		ASSERT_VALID (pWnd);

		if (!pWnd->CanBeAttached ())
		{
			return FALSE;
		}
	}

	return TRUE;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainerManager::ReleaseEmptyContainers ()
{
	if (m_pRootContainer != NULL)
	{
		m_pRootContainer->ReleaseEmptyContainer ();
	}
}
