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
// BCGBarContainer.cpp: implementation of the CBCGPBarContainer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPBarContainer.h"

#include "BCGPDockingControlBar.h"
#include "BCGPBaseTabbedBar.h"
#include "BCGPSlider.h"
#include "BCGPMiniFrameWnd.h"

#include "BCGPBarContainerManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CBCGPBarContainer, CObject)

BOOL CBCGPBarContainer::m_bMaintainPercentage = FALSE;
BOOL CBCGPBarContainer::m_bRetainInternalSliderPosition = FALSE;

CBCGPBarContainerGC gc;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPBarContainer::CBCGPBarContainer(CBCGPBarContainerManager* pManager,
									 CBCGPDockingControlBar* pLeftBar, 
									 CBCGPDockingControlBar* pRightBar, 
									 CBCGPSlider* pSlider) : 
									m_pContainerManager (pManager),	
									m_pBarLeftTop (pLeftBar), 
									m_pBarRightBottom (pRightBar),
									m_pSlider (pSlider), 
									m_pLeftContainer (NULL), 
									m_pRightContainer (NULL),
									m_pParentContainer (NULL), 
									m_dwRefCount (0)
{
	m_nSavedLeftBarID			= (UINT)-1;
	m_nSavedRightBarID			= (UINT)-1;
	m_nSavedSliderID			= (UINT)-1;	
	m_bSavedSliderVisibility	= FALSE;
	m_rectSavedSliderRect.SetRectEmpty ();

	m_dwRecentSliderStyle = 0;
	m_rectRecentSlider.SetRectEmpty ();

	m_nRecentPercent = 50;
	m_bIsRecentSliderHorz = FALSE;
	m_bDisposed = FALSE;
}
//-----------------------------------------------------------------------------------//
CBCGPBarContainer::~CBCGPBarContainer()
{
	CleanUp ();
	m_bDisposed = TRUE;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::CleanUp ()
{
	if (m_pLeftContainer != NULL)
	{
		m_pLeftContainer->CleanUp ();
		delete m_pLeftContainer;
		m_pLeftContainer = NULL;
	}

	if (m_pRightContainer != NULL)
	{
		m_pRightContainer->CleanUp ();
		delete m_pRightContainer;
		m_pRightContainer = NULL;
	}

	if (m_pSlider != NULL && !m_pSlider->IsDefault () && 
		m_pSlider->GetSafeHwnd () != NULL)
	{
		m_pSlider->DestroyWindow ();
		m_pSlider = NULL;
	}
}
//-----------------------------------------------------------------------------------//
int CBCGPBarContainer::GetResizeStep () const
{
	ASSERT_VALID (this);

	int nStep = -1;
	
	if (m_pBarLeftTop != NULL)
	{
		nStep = m_pBarLeftTop->GetResizeStep ();
	}

	if (m_pBarRightBottom != NULL)
	{
		nStep = max (nStep, m_pBarRightBottom->GetResizeStep ());
	}

	if (m_pLeftContainer != NULL)
	{
		nStep = m_pLeftContainer->GetResizeStep ();
	}

	if (m_pRightContainer != NULL)
	{
		nStep = max (nStep, m_pRightContainer->GetResizeStep ());
	}
	
	return nStep;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::GetWindowRect (CRect& rect, BOOL bIgnoreVisibility) const
{
	ASSERT_VALID (this);
	CRect rectLeft;
	CRect rectRight;
	CRect rectContainer;

	rect.SetRectEmpty ();
	rectLeft.SetRectEmpty ();
	rectRight.SetRectEmpty ();

	// VCheck 
	BOOL bAutoHideMode = m_pContainerManager->IsAutoHideMode ();

	if (m_pBarLeftTop != NULL && (m_pBarLeftTop->IsBarVisible () || bIgnoreVisibility || bAutoHideMode))
	{
		m_pBarLeftTop->GetWindowRect (rectLeft);
		if (rectLeft.IsRectEmpty ())
		{
			CSize sz; 
			m_pBarLeftTop->GetMinSize (sz);

			if (rectLeft.Width () == 0)
			{
				rectLeft.InflateRect (0, 0, sz.cx, 0);
			}
			
			if (rectLeft.Height () == 0)
			{
				rectLeft.InflateRect (0, 0, 0, sz.cy);
			}

		}
	}

	if (m_pBarRightBottom != NULL && (m_pBarRightBottom->IsBarVisible () || bIgnoreVisibility || bAutoHideMode))
	{
		m_pBarRightBottom->GetWindowRect (rectRight);
		if (rectRight.IsRectEmpty ())
		{
			CSize sz; 
			m_pBarRightBottom->GetMinSize (sz);
			
			if (rectRight.Width () == 0)
			{
				rectRight.InflateRect (0, 0, sz.cx, 0);
			}
			
			if (rectRight.Height () == 0)
			{
				rectRight.InflateRect (0, 0, 0, sz.cy);
			}
		}
	}

	rect.UnionRect (rectLeft, rectRight);

	if (m_pLeftContainer != NULL && (m_pLeftContainer->IsContainerVisible () || bIgnoreVisibility || bAutoHideMode))
	{
		m_pLeftContainer->GetWindowRect (rectContainer);
		rect.UnionRect (rect, rectContainer);
	}

	if (m_pRightContainer != NULL && (m_pRightContainer->IsContainerVisible () || bIgnoreVisibility || bAutoHideMode))
	{
		m_pRightContainer->GetWindowRect (rectContainer);
		rect.UnionRect (rect, rectContainer);
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::GetMinSize (CSize& size) const
{
	ASSERT_VALID (this);
	ASSERT (m_pContainerManager != NULL);

	CSize minSizeLeft (0, 0);
	CSize minSizeRight (0, 0);
	size.cx = size.cy = 0;

	BOOL bAutoHideMode = m_pContainerManager->IsAutoHideMode ();

	// VCheck
	if (m_pBarLeftTop != NULL && (m_pBarLeftTop->IsBarVisible () || bAutoHideMode))
	{
		m_pBarLeftTop->GetMinSize (minSizeLeft);
	}
	
	if (m_pBarRightBottom != NULL && (m_pBarRightBottom->IsBarVisible () || bAutoHideMode))
	{
		m_pBarRightBottom->GetMinSize (minSizeRight);
	}

	CSize sizeLeftContainer (0, 0);
	if (m_pLeftContainer != NULL && (m_pLeftContainer->IsContainerVisible () || bAutoHideMode))
	{
		m_pLeftContainer->GetMinSize (sizeLeftContainer);
	}

	CSize sizeRightContainer (0, 0);
	if (m_pRightContainer != NULL && (m_pRightContainer->IsContainerVisible () || bAutoHideMode))
	{
		m_pRightContainer->GetMinSize (sizeRightContainer);
	}

	if (m_pSlider != NULL && (m_pSlider->IsBarVisible () || bAutoHideMode))
	{
		if (IsSliderHorz ())
		{
			size.cx = max (minSizeLeft.cx, minSizeRight.cx);
			size.cx = max (sizeLeftContainer.cx, size.cx);
			size.cx = max (sizeRightContainer.cx, size.cx);
			size.cy = minSizeLeft.cy + minSizeRight.cy + sizeLeftContainer.cy + 
						sizeRightContainer.cy + m_pSlider->GetWidth ();
		}
		else
		{
			size.cy = max (minSizeLeft.cy, minSizeRight.cy);
			size.cy = max (sizeLeftContainer.cy, size.cy);
			size.cy = max (sizeRightContainer.cy, size.cy);
			size.cx = minSizeLeft.cx + minSizeRight.cx + sizeLeftContainer.cx + 
						sizeRightContainer.cx + m_pSlider->GetWidth ();
		}
	}
	else
	{
		size.cx = max (minSizeLeft.cx, minSizeRight.cx);
		size.cy = max (minSizeLeft.cy, minSizeRight.cy);
		if (m_pLeftContainer != NULL && m_pLeftContainer->IsContainerVisible ())
		{
			size = sizeLeftContainer;
		}
		if (m_pRightContainer != NULL && m_pRightContainer->IsContainerVisible ())
		{
			size = sizeRightContainer;
		}
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::GetMinSizeLeft (CSize& size) const
{
	ASSERT_VALID (this);

	BOOL bAutoHideMode = m_pContainerManager->IsAutoHideMode ();

	// VCheck
	CSize minSizeLeft (0, 0);
	if (m_pBarLeftTop != NULL && (m_pBarLeftTop->IsBarVisible () || bAutoHideMode))
	{
		m_pBarLeftTop->GetMinSize (minSizeLeft);
	}

	CSize sizeLeftContainer (0, 0);
	if (m_pLeftContainer != NULL && (m_pLeftContainer->IsContainerVisible () || bAutoHideMode))
	{
		m_pLeftContainer->GetMinSize (sizeLeftContainer);
	}
	
	size.cx = max (minSizeLeft.cx, sizeLeftContainer.cx);
	size.cy = max (minSizeLeft.cy, sizeLeftContainer.cy);
		
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::GetMinSizeRight(CSize& size) const
{
	ASSERT_VALID (this);
	// VCheck
	BOOL bAutoHideMode = m_pContainerManager->IsAutoHideMode ();
	CSize minSizeRight (0, 0);
	if (m_pBarRightBottom != NULL && (m_pBarRightBottom->IsBarVisible () || bAutoHideMode))
	{
		m_pBarRightBottom->GetMinSize (minSizeRight);
	}

	CSize sizeRightContainer (0, 0);
	if (m_pRightContainer != NULL && (m_pRightContainer->IsContainerVisible () || bAutoHideMode))
	{
		m_pRightContainer->GetMinSize (sizeRightContainer);
	}
	
	size.cx = max (minSizeRight.cx, sizeRightContainer.cx);
	size.cy = max (minSizeRight.cy, sizeRightContainer.cy);
}
//-----------------------------------------------------------------------------------//
CBCGPDockingControlBar* CBCGPBarContainer::AddControlBar (CBCGPDockingControlBar* pBar)
{
	ASSERT_VALID  (this);
	ASSERT_KINDOF (CBCGPDockingControlBar, pBar);
	
	CWnd* pDockSite = m_pContainerManager->GetDockSite ();
	ASSERT_VALID (pDockSite);

	BOOL bAddToMiniFrame = pDockSite->
		IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd));

	CRect rectNew = pBar->m_recentDockInfo.GetRecentDockedRect (!bAddToMiniFrame);
	

	CRect rectContainer;
	rectContainer.SetRectEmpty ();
	GetWindowRect (rectContainer);


	pDockSite->ScreenToClient (rectContainer);

	// if the container was empty we'll need to expand its parent container
	BOOL bExpandParentContainer = IsContainerEmpty ();
	// find first non-empty parent container
	CBCGPBarContainer* pNextContainer = m_pParentContainer;
	while (pNextContainer != NULL)
	{
		if (!pNextContainer->IsContainerEmpty ())
		{
			break;
		}
		pNextContainer = pNextContainer->GetParentContainer ();
	}

	CRect rectParentContainer;
	rectParentContainer.SetRectEmpty ();

	if (pNextContainer != NULL)
	{
		pNextContainer->GetWindowRect (rectParentContainer);
		pDockSite->ScreenToClient (rectParentContainer);
	}

	int nNewWidth  = rectContainer.Width () > 0 ? rectContainer.Width () : rectParentContainer.Width ();  
	int nNewHeight = rectContainer.Height () > 0 ? rectContainer.Height () : rectParentContainer.Height (); 

	if (nNewWidth == 0) 
	{
		nNewWidth = rectNew.Width ();
	}

	if (nNewHeight == 0)
	{
		nNewHeight = rectNew.Height ();
	}

	if (!rectContainer.IsRectEmpty ())
	{
		rectNew.left = rectContainer.left;
		rectNew.top = rectContainer.top;
	}
	else if (!rectParentContainer.IsRectEmpty ())
	{
		rectNew.left = rectParentContainer.left;
		rectNew.top = rectParentContainer.top;
	}

	CSize sizeMin;
	pBar->GetMinSize (sizeMin);

	if (nNewWidth < sizeMin.cx)
	{
		nNewWidth = sizeMin.cx;
	}

	if (nNewHeight < sizeMin.cy)
	{
		nNewHeight = sizeMin.cy;
	}

	int nRecentPercent = pBar->m_recentDockInfo.GetRecentDockedPercent (!bAddToMiniFrame);

	if (nRecentPercent == 100 || nRecentPercent == 0)
    {
		nRecentPercent = 50;
    }


	if (!IsContainerEmpty () && m_pSlider != NULL)
	{
		if (IsSliderHorz ())
		{			
			if (pBar->m_recentDockInfo.IsRecentLeftBar (!bAddToMiniFrame))
			{
				nNewHeight = rectContainer.Height () * nRecentPercent / 100;
				rectNew.top = rectContainer.top;
			}
			else
			{
				nNewHeight = rectContainer.Height () - 
					(rectContainer.Height () * (100 - nRecentPercent) / 100)
					- m_pSlider->GetWidth ();

				rectNew.top = rectContainer.bottom - nNewHeight;
			}
		}
		else
		{
			
			if (pBar->m_recentDockInfo.IsRecentLeftBar (!bAddToMiniFrame))
			{
				nNewWidth = rectContainer.Width () * nRecentPercent / 100;
				rectNew.left = rectContainer.left;
			}
			else
			{
				nNewWidth = rectContainer.Width () - 
					(rectContainer.Width () * (100 - nRecentPercent) / 100)
					- m_pSlider->GetWidth ();

				rectNew.left = rectContainer.right - nNewWidth;
			}
		}
	}

	rectNew.bottom = rectNew.top + nNewHeight;
	rectNew.right = rectNew.left + nNewWidth;

	BOOL bShowSlider = FALSE;
	
	HDWP hdwp = BeginDeferWindowPos (10);

	hdwp = pBar->MoveWindow (rectNew, FALSE, hdwp);

	CRect rectSlider = rectNew;
	CRect rectSecondBar;

	BOOL bIsRecentLeftBar = pBar->m_recentDockInfo.IsRecentLeftBar (!bAddToMiniFrame);

	if (bIsRecentLeftBar && m_pLeftContainer != NULL)
	{
		return m_pLeftContainer->AddControlBar (pBar);
	}

	if (!bIsRecentLeftBar && m_pRightContainer != NULL)
	{
		return m_pRightContainer->AddControlBar (pBar);	
	}

	if (bIsRecentLeftBar)
	{
		ASSERT (m_pLeftContainer == NULL);

		if (m_pBarLeftTop != NULL)
		{
			CBCGPDockingControlBar* pTabbedControlBar = NULL;
			pBar->AttachToTabWnd (m_pBarLeftTop, BCGP_DM_DBL_CLICK, TRUE, &pTabbedControlBar);
			if (pTabbedControlBar != NULL && m_pBarLeftTop != NULL)
			{
				m_pContainerManager->ReplaceControlBar (m_pBarLeftTop, pTabbedControlBar);
			}
			else if (pTabbedControlBar != NULL)
			{
				m_pContainerManager->AddControlBarToList (pTabbedControlBar);
				m_pBarLeftTop = pTabbedControlBar;
			}
			return pTabbedControlBar;
		}
		//ASSERT (m_pBarLeftTop == NULL);
		m_pBarLeftTop = pBar;

		bShowSlider = (m_pBarRightBottom != NULL) || (m_pRightContainer != NULL);

		if (m_pBarRightBottom != NULL)
		{
			m_pBarRightBottom->GetWindowRect (rectSecondBar);
		}
		else if (m_pRightContainer != NULL)
		{
			m_pRightContainer->GetWindowRect (rectSecondBar);
		}

		pDockSite->ScreenToClient (rectSecondBar);

		if (m_pSlider != NULL)
		{
			if (IsSliderHorz ())
			{
				rectSlider.top = rectNew.bottom;
				rectSlider.bottom = rectSlider.top + m_pSlider->GetWidth ();
				rectSecondBar.top = rectSlider.bottom;
			}
			else
			{
				rectSlider.left     = rectNew.right;
				rectSlider.right    = rectSlider.left + m_pSlider->GetWidth ();
				rectSecondBar.left  = rectSlider.right;
			}
		}

		if (m_pBarRightBottom != NULL)
		{
			hdwp = m_pBarRightBottom->MoveWindow (rectSecondBar, FALSE, hdwp);
		} 
		else if (m_pRightContainer != NULL)
		{
			m_pRightContainer->ResizeContainer (rectSecondBar, hdwp);
		}
	}
	else
	{
		ASSERT (m_pRightContainer == NULL);
		if (m_pBarRightBottom != NULL)
		{
			CBCGPDockingControlBar* pTabbedControlBar = NULL;
			pBar->AttachToTabWnd (m_pBarRightBottom, BCGP_DM_DBL_CLICK, TRUE, &pTabbedControlBar);
			if (pTabbedControlBar != NULL && m_pBarRightBottom != NULL)
			{
				m_pContainerManager->ReplaceControlBar (m_pBarRightBottom, pTabbedControlBar);
			}
			else if (pTabbedControlBar != NULL)
			{
				m_pContainerManager->AddControlBarToList (pTabbedControlBar);
				m_pBarRightBottom = pTabbedControlBar;
			}
			return pTabbedControlBar;
		}
		
		m_pBarRightBottom = pBar;

		bShowSlider = (m_pBarLeftTop != NULL) || (m_pLeftContainer != NULL);

		if (m_pBarLeftTop != NULL)
		{
			m_pBarLeftTop->GetWindowRect (rectSecondBar);
		} 
		else if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->GetWindowRect (rectSecondBar);
		}

		pDockSite->ScreenToClient (rectSecondBar);

		if (m_pSlider != NULL)
		{
			if (IsSliderHorz ())
			{
				rectSlider.bottom = rectNew.top;
				rectSlider.top = rectSlider.bottom - m_pSlider->GetWidth ();
				rectSecondBar.bottom = rectSlider.top;
			}
			else
			{
				rectSlider.right = rectNew.left;
				rectSlider.left = rectSlider.right - m_pSlider->GetWidth ();
				rectSecondBar.right = rectSlider.left;
			}
		}

		if (m_pBarLeftTop != NULL)
		{
			hdwp = m_pBarLeftTop->MoveWindow (rectSecondBar, FALSE, hdwp);
		}
		else if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->ResizeContainer (rectSecondBar, hdwp);
		}
	}

	if (m_pSlider != NULL)
	{
		if (bShowSlider)
		{
			hdwp = m_pSlider->MoveWindow (rectSlider, FALSE, hdwp);
		}
		else
		{
			m_pSlider->ShowWindow (SW_HIDE);
		}
	}
	
	rectContainer.UnionRect (rectNew, rectSecondBar);
	pDockSite->ClientToScreen (rectContainer);

	if (bExpandParentContainer)
	{
		// find the first parent container that has non-empty rectangly and
		// whose left/right bar/containre should be expanded

		if (pNextContainer != NULL)
		{
			const CBCGPSlider* pParentSlider = pNextContainer->GetSlider ();

			if (pParentSlider != NULL)
			{
				ASSERT_VALID (pParentSlider);

				CBCGPBarContainer* pLeftContainer = (CBCGPBarContainer*) pNextContainer->GetLeftContainer ();
				CBCGPBarContainer* pRightContainer = (CBCGPBarContainer*) pNextContainer->GetRightContainer ();

				BOOL bIsLeftContainer = FALSE;

				if (pLeftContainer != NULL && 
					pLeftContainer-> FindSubContainer (this, BC_FIND_BY_CONTAINER))
				{
					bIsLeftContainer = TRUE;
				}
				else if (pRightContainer != NULL && 
						 pRightContainer-> FindSubContainer (this, BC_FIND_BY_CONTAINER))
				{
					bIsLeftContainer = FALSE;
				}
				else
				{
					return pBar;
				}
				
				pParentSlider->GetWindowRect (rectSlider);

				int nOffset = pParentSlider->GetWidth ();

				if (bIsLeftContainer)
				{
					if (pParentSlider->IsHorizontal ())
					{
						nOffset += nNewHeight;
						rectSlider.top = rectContainer.bottom;
						rectSlider.bottom = rectSlider.top + pParentSlider->GetWidth ();
					}
					else 
					{
						nOffset += nNewWidth;
						rectSlider.left = rectContainer.right;
						rectSlider.right = rectSlider.left + pParentSlider->GetWidth ();
					}
				}
				else
				{
					if (pParentSlider->IsHorizontal ())
					{
						nOffset = -(nNewHeight + pParentSlider->GetWidth ());
						rectSlider.bottom = rectContainer.top;
						rectSlider.top = rectSlider.bottom - pParentSlider->GetWidth ();
					}
					else 
					{
						nOffset = -(nNewWidth + pParentSlider->GetWidth ());;
						rectSlider.right = rectContainer.left;
						rectSlider.left = rectSlider.right - pParentSlider->GetWidth ();
					}
				}

				pDockSite->ScreenToClient (rectSlider);
				if (m_pSlider != NULL)
				{
					hdwp = m_pSlider->MoveWindow (rectSlider, FALSE, hdwp);
				}
				pNextContainer->ResizePartOfContainer (nOffset, !bIsLeftContainer, hdwp);
			}
		}
	}

	EndDeferWindowPos (hdwp);
	return pBar;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::ResizePartOfContainer (int nOffset, BOOL bLeftPart, 
											   HDWP& hdwp)
{
	ASSERT_VALID (this);

	if (m_pSlider == NULL)
	{
		return;
	}

	CRect rectPart; rectPart.SetRectEmpty ();
	CSize sizeMin (0, 0);

	if (bLeftPart && m_pLeftContainer != NULL)
	{
		m_pLeftContainer->GetWindowRect (rectPart);
		m_pLeftContainer->GetMinSize (sizeMin);
	}
	else if (bLeftPart && m_pBarLeftTop != NULL)
	{
		m_pBarLeftTop->GetWindowRect (rectPart);
		m_pBarLeftTop->GetMinSize (sizeMin);
	}
	else if (!bLeftPart && m_pRightContainer != NULL)
	{
		m_pRightContainer->GetWindowRect (rectPart);
		m_pRightContainer->GetMinSize (sizeMin);
	}
	else if (!bLeftPart && m_pBarRightBottom != NULL)
	{
		m_pBarRightBottom->GetWindowRect (rectPart);
		m_pBarRightBottom->GetMinSize (sizeMin);
	}
	else
	{
		return;
	}

	if (bLeftPart && IsSliderHorz ())
	{
		rectPart.bottom += nOffset;
		if (rectPart.Height () < sizeMin.cy)
		{
			rectPart.bottom = rectPart.top + sizeMin.cy;
		}
	}
	else if (bLeftPart && !IsSliderHorz ())
	{
		rectPart.right += nOffset;
		if (rectPart.Width () < sizeMin.cx)
		{
			rectPart.right = rectPart.left + sizeMin.cx;
		}
	}
	else if (!bLeftPart && IsSliderHorz ())
	{
		rectPart.top += nOffset;
		if (rectPart.Height () < sizeMin.cy)
		{
			rectPart.top = rectPart.bottom - sizeMin.cy;
		}
	}
	else
	{
		rectPart.left += nOffset;
		if (rectPart.Width () < sizeMin.cx)
		{
			rectPart.left = rectPart.right - sizeMin.cx;
		}
	}

	CWnd* pDockSite = m_pContainerManager->GetDockSite ();
	ASSERT_VALID (pDockSite);
	pDockSite->ScreenToClient (rectPart);

	if (bLeftPart && m_pLeftContainer != NULL)
	{
		m_pLeftContainer->ResizeContainer (rectPart, hdwp);
	}
	else if (bLeftPart && m_pBarLeftTop != NULL)
	{
		hdwp = m_pBarLeftTop->MoveWindow (rectPart, FALSE, hdwp);
	}
	else if (!bLeftPart && m_pRightContainer != NULL)
	{
		m_pRightContainer->ResizeContainer (rectPart, hdwp);
	}
	else if (!bLeftPart && m_pBarRightBottom != NULL)
	{
		hdwp = m_pBarRightBottom->MoveWindow (rectPart, FALSE, hdwp);
	}
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainer::AddSubContainer (CBCGPBarContainer* pContainer, BOOL bRightNodeNew)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pContainer);

	// slider must be unique
	ASSERT (m_pSlider != pContainer->GetSlider ());
	
	ASSERT (pContainer->GetLeftBar () != NULL || pContainer->GetRightBar () != NULL);

	CBCGPBarContainer* pExistingContainer = NULL;
	// one of the nodes (control bars) is always new, e.g is being docked.
	// find a container that contains a node with an exisisting control bar
	// the incoming control bar is being docked to.
	const CBCGPControlBar* pBarToFind = bRightNodeNew ? pContainer->GetLeftBar () : pContainer->GetRightBar ();
	ASSERT_VALID (pBarToFind);	

	pExistingContainer = FindSubContainer (pBarToFind, BC_FIND_BY_LEFT_BAR);
	
	if (pExistingContainer == NULL)
	{
		pExistingContainer = FindSubContainer (pBarToFind, BC_FIND_BY_RIGHT_BAR);
	}

	// a node with the left or right bar must exist in the tree
	if (pExistingContainer == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	pExistingContainer->AddNode (pContainer);
	return TRUE;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::AddNode (CBCGPBarContainer* pContainer)
{
	ASSERT_VALID (this);
	// onr of the bars must be the same
	ASSERT (m_pBarLeftTop == pContainer->GetLeftBar () ||
			m_pBarLeftTop == pContainer->GetRightBar () ||
			m_pBarRightBottom == pContainer->GetLeftBar () ||
			m_pBarRightBottom == pContainer->GetRightBar ());

	if (m_pBarLeftTop != NULL && 
		(m_pBarLeftTop == pContainer->GetLeftBar () ||
		 m_pBarLeftTop == pContainer->GetRightBar ()))
	{
		m_pBarLeftTop = NULL;
		m_pLeftContainer = pContainer;
	}
	else 
	{
		m_pBarRightBottom = NULL;
		m_pRightContainer = pContainer;
	}

	pContainer->SetParentContainer (this);
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::RemoveControlBar (CBCGPDockingControlBar* pBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	BC_FIND_CRITERIA barType = BC_FIND_BY_LEFT_BAR;
	CBCGPBarContainer* pContainer = FindSubContainer (pBar, barType);
	if (pContainer == NULL)
	{
		barType = BC_FIND_BY_RIGHT_BAR;
		pContainer = FindSubContainer (pBar, barType);
	}

	if (pContainer != NULL)
	{
		pContainer->DeleteControlBar (pBar, barType);
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::AddRef ()
{
	m_dwRefCount++;
}
//-----------------------------------------------------------------------------------//
DWORD CBCGPBarContainer::Release ()
{
	m_dwRefCount--;
	if (m_dwRefCount <= 0)
	{
		FreeReleasedContainer ();	
		return 0;
	}

	return m_dwRefCount;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::FreeReleasedContainer ()
{
	int nCountNode = 0;

	if (m_pBarLeftTop != NULL) 
	{
		nCountNode++;
	}
	if (m_pBarRightBottom != NULL)
	{
		nCountNode++;
	}
	if (m_pLeftContainer != NULL) 
	{
		nCountNode++;
	}
	if (m_pRightContainer != NULL)
	{
		nCountNode++;
	}

	if (nCountNode > 1)
	{
		return;
	}

	if (m_dwRefCount <= 0)
	{
		if ((m_pSlider != NULL && !m_pSlider->IsDefault () ||
			m_pSlider == NULL) && m_pParentContainer != NULL && 
			m_pParentContainer != m_pContainerManager->m_pRootContainer)
		{

			ASSERT (m_pParentContainer->GetLeftContainer () != NULL ||
					m_pParentContainer->GetRightContainer () != NULL);

			BOOL bLeft = (m_pParentContainer->GetLeftContainer () == this);

			m_pParentContainer->SetContainer (NULL, bLeft);

			if (m_pBarLeftTop != NULL)
			{
				m_pParentContainer->SetBar (m_pBarLeftTop, bLeft);
				m_pBarLeftTop = NULL;
			}
			else if (m_pBarRightBottom != NULL)
			{
				m_pParentContainer->SetBar (m_pBarRightBottom, bLeft);
				m_pBarRightBottom = NULL;
			}
			else if (m_pLeftContainer != NULL)
			{
				m_pParentContainer->SetContainer (m_pLeftContainer, bLeft);
				m_pLeftContainer = NULL;
			}
			else if (m_pRightContainer != NULL)
			{
				m_pParentContainer->SetContainer (m_pRightContainer, bLeft);
				m_pRightContainer = NULL;
			}

			if (m_pSlider != NULL)
			{
				m_pSlider->DestroyWindow ();
				m_pSlider = NULL;
			}

			//delete this;
			m_bDisposed = TRUE;
			gc.AddContainer (this);
		}
		else
		{
			m_pContainerManager->NotifySlider ();
		}
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::ReleaseEmptyContainer ()
{
	if (m_pLeftContainer != NULL)
	{
		m_pLeftContainer->ReleaseEmptyContainer ();
	}

	if (m_pRightContainer != NULL)
	{
		m_pRightContainer->ReleaseEmptyContainer ();
	}

	if (m_pParentContainer != m_pContainerManager->m_pRootContainer)
	{
		FreeReleasedContainer ();
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::DeleteControlBar (CBCGPDockingControlBar* pBar, 
										  BC_FIND_CRITERIA barType)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	//-------------- set recent dock info
	//StoreRecentDockInfo (pBar);
	//--------------

	CRect rectContainer;
	GetWindowRect (rectContainer);

	CRect rectBar;
	pBar->GetWindowRect (rectBar);

	// it's required to expand remaining container - take the width of slider first
	int nExpandOffset = 0;
	if (m_pSlider != NULL)
	{
		nExpandOffset = m_pSlider->GetWidth ();

		nExpandOffset += IsSliderHorz () ? rectBar.Height () : rectBar.Width ();
	}

	HDWP hdwp = BeginDeferWindowPos (10);

	BOOL bNeedToExpandParentContainer = FALSE;

	if (barType == BC_FIND_BY_LEFT_BAR && pBar == m_pBarLeftTop)
	{
		m_pBarLeftTop = NULL;

		if (m_pBarRightBottom != NULL)
		{
			m_pBarRightBottom->MoveControlBar (rectContainer, FALSE, hdwp);
		}
		else if (m_pRightContainer != NULL && !m_pRightContainer->IsContainerEmpty ())
		{
			// expanding right container - in the left direction
			m_pContainerManager->GetDockSite ()->ScreenToClient (rectContainer);			
			ResizeContainer (rectContainer, hdwp);
		}
		else if (m_pParentContainer != NULL)
		{
			bNeedToExpandParentContainer = TRUE;
		}
	}
	else if (barType == BC_FIND_BY_RIGHT_BAR && pBar == m_pBarRightBottom)
	{
		m_pBarRightBottom = NULL;
		if (m_pBarLeftTop)
		{
			m_pBarLeftTop->MoveControlBar (rectContainer, FALSE, hdwp);
		}
		else if (m_pLeftContainer != NULL && !m_pLeftContainer->IsContainerEmpty ())
		{
			// expanding left container - in the right direction
			m_pContainerManager->GetDockSite ()->ScreenToClient (rectContainer);
			ResizeContainer (rectContainer, hdwp);
		}
		else if (m_pParentContainer != NULL)
		{
			bNeedToExpandParentContainer = TRUE;
		}
		
	}
	else
	{
		ASSERT (FALSE);
	}

	if (bNeedToExpandParentContainer)
	{
		// find the first parent container that has non-empty rectangly and
		// whose left/right bar/containre should be expanded
		CBCGPBarContainer* pNextContainer = m_pParentContainer;
		while (pNextContainer != NULL)
		{
			if (!pNextContainer->IsContainerEmpty ())
			{
				break;
			}
			pNextContainer = pNextContainer->GetParentContainer ();
		}

		if (pNextContainer != NULL)
		{
			CBCGPSlider* pParentSlider = (CBCGPSlider*) pNextContainer->GetSlider ();

			if (pParentSlider != NULL)
			{
				int nExpandParentContainerOffset = pParentSlider->IsHorizontal () ? 
									rectBar.Height () : rectBar.Width ();
				nExpandParentContainerOffset *= 2;
				nExpandParentContainerOffset += pParentSlider->GetWidth () +2; 
				
				if (pNextContainer->IsLeftPartEmpty ())
				{
					pNextContainer->StretchContainer (-nExpandParentContainerOffset, 
													  !pParentSlider->IsHorizontal (), 
													  FALSE, TRUE, hdwp);
				}
				else if (pNextContainer->IsRightPartEmpty ())
				{
					pNextContainer->StretchContainer (nExpandParentContainerOffset, 
													  !pParentSlider->IsHorizontal (), 
													  TRUE, TRUE, hdwp);
				}
			}
		}
	}
	EndDeferWindowPos (hdwp);

	if (m_pSlider == NULL)
	{
		// it was last bar/container here
		m_pBarLeftTop = m_pBarRightBottom = NULL;
		m_pLeftContainer = m_pRightContainer = NULL;
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::StoreRecentDockInfo (CBCGPDockingControlBar* pBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	//-------------- set recent dock info
	CBCGPSlider* pSlider = pBar->GetDefaultSlider ();
	
	// default slider is NULL when the bar is float_multi on miniframe
	if (pSlider == NULL || pBar->GetParentMiniFrame () != NULL)
	{
		pBar->m_recentDockInfo.StoreDockInfo (this);
		return;
	}

	// DO NOT SAVE recent dock info during transition from autohide mode 
	// to the regular dock mode! (because it's transition from dock to dock state)
	if (!pSlider->IsAutoHideMode ())
	{
		pBar->m_recentDockInfo.StoreDockInfo (this);
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::SetBar (CBCGPDockingControlBar* pBar, BOOL bLeft)
{
	ASSERT_VALID (this);
	// pBar can be NULL
	if (bLeft)
	{
		m_pBarLeftTop = pBar;
	}
	else
	{
		m_pBarRightBottom = pBar;
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::SetContainer (CBCGPBarContainer* pContainer, BOOL bLeft)
{
	ASSERT_VALID (this);
	
	if (bLeft)
	{
		m_pLeftContainer = pContainer;
	}
	else
	{
		m_pRightContainer = pContainer;
	}

	if (pContainer != NULL)
	{
		pContainer->SetParentContainer (this);
	}
}
//-----------------------------------------------------------------------------------//
CBCGPBarContainer* CBCGPBarContainer::FindSubContainer (const CObject* pObject, 
													  BC_FIND_CRITERIA findCriteria)
{
	ASSERT_VALID (this);
	ASSERT (pObject != NULL);

	switch (findCriteria)
	{
	case BC_FIND_BY_LEFT_BAR:
		if (m_pBarLeftTop == pObject)
		{
			return this;
		}
		break;
	case BC_FIND_BY_RIGHT_BAR:
		if (m_pBarRightBottom == pObject)
		{
			return this;
		}
		break;
	case BC_FIND_BY_SLIDER:
		if (m_pSlider == pObject)
		{
			return this;
		}
		break;
	case BC_FIND_BY_CONTAINER:
		if (this == pObject)
		{
			return this;
		}
		break;
	}

	CBCGPBarContainer* pSubContainer = NULL;
	
	if (m_pLeftContainer != NULL)
	{
		pSubContainer = m_pLeftContainer->FindSubContainer (pObject, findCriteria);
	}

	if (pSubContainer == NULL && m_pRightContainer != NULL)
	{
		pSubContainer = m_pRightContainer->FindSubContainer (pObject, findCriteria);
	}

	return pSubContainer;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::CalculateRecentSize ()
{
	CRect rectContainer; rectContainer.SetRectEmpty ();

	BOOL bAutoHideMode = m_pContainerManager->IsAutoHideMode ();

	GetWindowRect (rectContainer);

	CRect rectLeft; rectLeft.SetRectEmpty ();
	CRect rectRight; rectRight.SetRectEmpty ();

	CSize sizeMinLeft;
	CSize sizeMinRight;

	double dLeftPercent = 0.;

	if (m_pBarLeftTop != NULL && (m_pBarLeftTop->IsBarVisible () || bAutoHideMode))
	{
		m_pBarLeftTop->GetWindowRect (rectLeft);
		m_pBarLeftTop->GetMinSize (sizeMinLeft);
	}

	if (m_pLeftContainer != NULL && (m_pLeftContainer->IsContainerVisible () || bAutoHideMode))
	{
		m_pLeftContainer->GetWindowRect (rectLeft);
		m_pLeftContainer->GetMinSize (sizeMinLeft);
	}

	if (m_pBarRightBottom != NULL && (m_pBarRightBottom->IsBarVisible () || bAutoHideMode))
	{
		m_pBarRightBottom->GetWindowRect (rectRight);	
		m_pBarRightBottom->GetMinSize (sizeMinRight);
	}

	if (m_pRightContainer != NULL && (m_pRightContainer->IsContainerVisible () || bAutoHideMode))
	{
		m_pRightContainer->GetWindowRect (rectRight);	
		m_pRightContainer->GetMinSize (sizeMinRight);
	}

	BOOL bCheckVisibility = !bAutoHideMode;

	if (!IsLeftPartEmpty (bCheckVisibility) && IsRightPartEmpty (bCheckVisibility))
	{
		if (m_pBarLeftTop != NULL)
		{
			m_pBarLeftTop->SetLastPercentInContainer (100);
		}

		if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->CalculateRecentSize ();
			m_pLeftContainer->SetRecentPercent (100);
		}
	}
	else if (IsLeftPartEmpty (bCheckVisibility) && !IsRightPartEmpty (bCheckVisibility))
	{

		if (m_pBarRightBottom != NULL)
		{
			m_pBarRightBottom->SetLastPercentInContainer (100);
		}

		if (m_pRightContainer != NULL)
		{
			m_pRightContainer->CalculateRecentSize ();
			m_pRightContainer->SetRecentPercent (100);
		}
	}
	else if (!IsLeftPartEmpty (bCheckVisibility) && !IsRightPartEmpty (bCheckVisibility))
	{
		ASSERT (m_pSlider != NULL);
		
		if (IsSliderHorz ())
		{
			int nPercent = -1;
			if ((rectLeft.Height () + rectRight.Height ()) > rectContainer.Height ())
			{
				nPercent = 50;
				if (rectLeft.Height () == rectContainer.Height ())
				{
					if (m_pBarRightBottom != NULL)
					{
						nPercent = m_pBarRightBottom->GetLastPercentInContainer ();
					}
					else if (m_pRightContainer != NULL)
					{
						nPercent = m_pRightContainer->GetRecentPercent ();
					}

					rectLeft.bottom = rectLeft.top + 
						rectContainer.Height () - 
						((rectContainer.Height () * nPercent) / 100);
					nPercent = 100 - nPercent;
				}
				else if (rectRight.Height () == rectContainer.Height ())
				{
					if (m_pBarLeftTop != NULL)
					{
						nPercent = m_pBarLeftTop->GetLastPercentInContainer ();
					}
					else if (m_pLeftContainer != NULL)
					{
						nPercent = m_pLeftContainer->GetRecentPercent ();
					}

					rectLeft.bottom = rectLeft.top + 
						((rectContainer.Height () * nPercent) / 100);
				}
			}
			
			dLeftPercent = ((double) rectLeft.Height ()) / 
									rectContainer.Height () * 100;

			if (nPercent != -1)
			{
				dLeftPercent = nPercent;
			}

		}
		else
		{
			int nPercent = -1;
			if ((rectLeft.Width () + rectRight.Width ()) > rectContainer.Width ())
			{
				if (rectLeft.Width () == rectContainer.Width ())
				{
					nPercent = 50;
					if (m_pBarRightBottom != NULL)
					{
						nPercent = m_pBarRightBottom->GetLastPercentInContainer ();
					}
					else if (m_pRightContainer != NULL)
					{
						nPercent = m_pRightContainer->GetRecentPercent ();
					}

					rectLeft.right = rectLeft.left + 
						rectContainer.Width () - 
						((rectContainer.Width () * nPercent) / 100);
					nPercent = 100 - nPercent;
				}
				else if (rectRight.Width () == rectContainer.Width ())
				{
					nPercent = 50;
					if (m_pBarLeftTop != NULL)
					{
						nPercent = m_pBarLeftTop->GetLastPercentInContainer ();
					}
					else if (m_pLeftContainer != NULL)
					{
						nPercent = m_pLeftContainer->GetRecentPercent ();
					}

					rectLeft.right = rectLeft.left + 
						((rectContainer.Width () * nPercent) / 100);
				}
			}

			dLeftPercent = ((double) rectLeft.Width ()) / 
									rectContainer.Width () * 100;


			if (nPercent != -1)
			{
				dLeftPercent = nPercent;
			}
		}

		if (m_pBarLeftTop != NULL)
		{
			m_pBarLeftTop->SetLastPercentInContainer ((int) dLeftPercent);
		}

		if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->CalculateRecentSize ();
			m_pLeftContainer->SetRecentPercent ((int) dLeftPercent);
		}

		if (m_pBarRightBottom != NULL)
		{
			m_pBarRightBottom->SetLastPercentInContainer (100 - (int) dLeftPercent);
		}

		if (m_pRightContainer != NULL)
		{
			m_pRightContainer->CalculateRecentSize ();
			m_pRightContainer->SetRecentPercent (100 - (int) dLeftPercent);
		}		
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::ResizeContainer (CRect rect, HDWP& hdwp, BOOL bRedraw)
{
	CRect rectContainer; rectContainer.SetRectEmpty ();
	CRect rectSlider; rectSlider.SetRectEmpty ();

	BOOL bAutoHideMode = m_pContainerManager->IsAutoHideMode ();
	// VCheck
	if (m_pSlider != NULL && (m_pSlider->IsBarVisible () || bAutoHideMode))
	{
		m_pSlider->GetWindowRect (rectSlider);
	}

	GetWindowRect (rectContainer);

	CRect rectLeft; rectLeft.SetRectEmpty ();
	CRect rectRight; rectRight.SetRectEmpty ();

	CSize sizeMinLeft;
	CSize sizeMinRight;

	double dLeftPercent = 0.;

	if (m_pBarLeftTop != NULL && (m_pBarLeftTop->IsBarVisible () || bAutoHideMode))
	{
		m_pBarLeftTop->GetWindowRect (rectLeft);
		m_pBarLeftTop->GetMinSize (sizeMinLeft);
	}

	if (m_pLeftContainer != NULL && (m_pLeftContainer->IsContainerVisible () || bAutoHideMode))
	{
		m_pLeftContainer->GetWindowRect (rectLeft);
		m_pLeftContainer->GetMinSize (sizeMinLeft);
	}

	if (m_pBarRightBottom != NULL && (m_pBarRightBottom->IsBarVisible () || bAutoHideMode))
	{
		m_pBarRightBottom->GetWindowRect (rectRight);	
		m_pBarRightBottom->GetMinSize (sizeMinRight);
	}

	if (m_pRightContainer != NULL && (m_pRightContainer->IsContainerVisible () || bAutoHideMode))
	{
		m_pRightContainer->GetWindowRect (rectRight);	
		m_pRightContainer->GetMinSize (sizeMinRight);
	}

	BOOL bCheckVisibility = !bAutoHideMode;

	if (!IsLeftPartEmpty (bCheckVisibility) && IsRightPartEmpty (bCheckVisibility))
	{
		if (m_pBarLeftTop != NULL)
		{
			if (rect.Width () < sizeMinLeft.cx && CBCGPControlBar::m_bHandleMinSize)
			{
				rect.right = rect.left + sizeMinLeft.cx;
			}
			if (rect.Height () < sizeMinLeft.cy && CBCGPControlBar::m_bHandleMinSize)
			{
				rect.bottom = rect.top + sizeMinLeft.cy;
			}
			hdwp = m_pBarLeftTop->MoveWindow (rect, bRedraw, hdwp);
		}

		if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->ResizeContainer (rect, hdwp, bRedraw);
		}
	}
	else if (IsLeftPartEmpty (bCheckVisibility) && !IsRightPartEmpty (bCheckVisibility))
	{
		if (m_pBarRightBottom != NULL)
		{
			if (rect.Width () < sizeMinRight.cx && CBCGPControlBar::m_bHandleMinSize)
			{
				rect.right = rect.left + sizeMinRight.cx;
			}
			if (rect.Height () < sizeMinRight.cy && CBCGPControlBar::m_bHandleMinSize)
			{
				rect.bottom = rect.top + sizeMinRight.cy;
			} 

			hdwp = m_pBarRightBottom->MoveWindow (rect, bRedraw, hdwp);
		}

		if (m_pRightContainer != NULL)
		{
			m_pRightContainer->ResizeContainer (rect, hdwp, bRedraw);
		}
	}
	else if (!IsLeftPartEmpty (bCheckVisibility) && !IsRightPartEmpty (bCheckVisibility))
	{
		CRect rectFinalLeft = rect;
		CRect rectFinalRight = rect;
		CRect rectFinalSlider = rect;

		ASSERT (m_pSlider != NULL);

		
		if (IsSliderHorz ())
		{
			int nPercent = -1;
			if ((rectLeft.Height () + rectRight.Height ()) > rectContainer.Height () ||
				 rectLeft.IsRectEmpty () || rectRight.IsRectEmpty ())
			{
				nPercent = 50;
				if (rectLeft.Height () == rectContainer.Height ())
				{
					if (m_pBarRightBottom != NULL)
					{
						nPercent = m_pBarRightBottom->GetLastPercentInContainer ();
					}
					else if (m_pRightContainer != NULL)
					{
						nPercent = m_pRightContainer->GetRecentPercent ();
					}

					if (nPercent == 100 || nPercent == 0)
					{
						nPercent = 50;
					}

					rectLeft.bottom = rectLeft.top + 
						rectContainer.Height () - 
						((rectContainer.Height () * nPercent) / 100);
					nPercent = 100 - nPercent;
				}
				else if (rectRight.Height () == rectContainer.Height ())
				{
					if (m_pBarLeftTop != NULL)
					{
						nPercent = m_pBarLeftTop->GetLastPercentInContainer ();
					}
					else if (m_pLeftContainer != NULL)
					{
						nPercent = m_pLeftContainer->GetRecentPercent ();
					}

					if (nPercent == 100 || nPercent == 0)
					{
						nPercent = 50;
					}

					rectLeft.bottom = rectLeft.top + 
						((rectContainer.Height () * nPercent) / 100);
				}
			}
			
			int nDelta = rect.Height () - rectContainer.Height ();

			dLeftPercent = ((double) rectLeft.Height ()) / 
									rectContainer.Height () * 100;

			if (dLeftPercent == 100 || dLeftPercent == 0)
			{
				dLeftPercent = 50;
			}
			if (m_bMaintainPercentage)
			{
				if (nDelta != 0)
				{
					rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height () + 
						(int)( (double) nDelta * (dLeftPercent) / 100.);
				}
				else
				{
					rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height ();
					if (nPercent != -1)
					{
						dLeftPercent = nPercent;
					}
				}
			}
			else
			{
                if (m_bRetainInternalSliderPosition)
                {
				    if (nDelta > 0)
				    {
					    rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height ();
				    }

				    else if (nDelta < 0)
				    {
					    rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height ();
				    }
				    else
				    {
					    rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height ();
					    if (nPercent != -1)
					    {
						    dLeftPercent = nPercent;
					    }
				    }
                    if (CWnd::GetCapture() != m_pSlider)
                    {
                        CRect rc(rectSlider);
                        m_pSlider->GetParent()->ScreenToClient(rc);
                        rectFinalLeft.bottom = rc.top;
                    }
                    dLeftPercent =   rectFinalLeft.Height() 
                                    / (static_cast<double>(rectContainer.Height()));
                }
                else
                {
				    if (nDelta > 0)
				    {
					    rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height () + 
						    (int)( (double) (nDelta * (100 - dLeftPercent)) / 100.) ;
				    }

				    else if (nDelta < 0)
				    {
					    rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height () + 
						    (int)( (double) nDelta * (dLeftPercent) / 100.);
				    }
				    else
				    {
					    rectFinalLeft.bottom = rectFinalLeft.top + rectLeft.Height ();
					    if (nPercent != -1)
					    {
						    dLeftPercent = nPercent;
					    }
				    }
                }
			}

			rectFinalSlider.top = rectFinalLeft.bottom;
			rectFinalSlider.bottom = rectFinalSlider.top + m_pSlider->GetWidth ();
			rectFinalRight.top = rectFinalSlider.bottom;

            if (CBCGPControlBar::m_bHandleMinSize)
			{
				int deltaLeft = sizeMinLeft.cy - rectFinalLeft.Height ();
				int deltaRight = sizeMinRight.cy - rectFinalRight.Height ();
				int nSliderWidth = m_pSlider->GetWidth ();

				if (deltaLeft <= 0 && deltaRight <= 0)
				{
				}
				else if (deltaLeft > 0 && deltaRight <= 0)
				{
					rectFinalLeft.bottom += deltaLeft;
					rectFinalRight.top = rectFinalLeft.bottom + nSliderWidth;
					if (rectFinalRight.Height () < sizeMinRight.cy)
					{
						rectFinalRight.bottom = rectFinalRight.top + sizeMinRight.cy;	
					}
				}
				else if (deltaLeft <= 0 && deltaRight > 0)
				{
					rectFinalLeft.bottom -= deltaRight;
					if (rectFinalLeft.Height () < sizeMinLeft.cy)
					{
						rectFinalLeft.bottom = rectFinalLeft.top + sizeMinLeft.cy;
					}
					
					rectFinalRight.top = rectFinalLeft.bottom + nSliderWidth;
					rectFinalRight.bottom = rectFinalRight.top + sizeMinRight.cy;	
				}
				else if (deltaLeft > 0 && deltaRight > 0)
				{
					rectFinalLeft.bottom = rectFinalLeft.top + sizeMinLeft.cy;
					rectFinalRight.top = rectFinalLeft.bottom + nSliderWidth;
					rectFinalRight.bottom = rectFinalRight.top + sizeMinRight.cy;	
				}

				rectFinalSlider.top = rectFinalLeft.bottom;
				rectFinalSlider.bottom = rectFinalSlider.top + nSliderWidth;	

				dLeftPercent = ((double) rectFinalLeft.Height ()) / 
										rectContainer.Height () * 100;

				if (rectFinalLeft.Width () < sizeMinLeft.cx)
				{
					rectFinalLeft.right = rectFinalLeft.left + sizeMinLeft.cx;
					rectFinalRight.right = rectFinalRight.left + sizeMinLeft.cx;
				}
			}			

		}
		else
		{
			int nPercent = -1;
			if ((rectLeft.Width () + rectRight.Width ()) > rectContainer.Width () ||
				rectLeft.IsRectEmpty () || rectRight.IsRectEmpty ())
			{
				if (rectLeft.Width () == rectContainer.Width ())
				{
					nPercent = 50;
					if (m_pBarRightBottom != NULL)
					{
						nPercent = m_pBarRightBottom->GetLastPercentInContainer ();
					}
					else if (m_pRightContainer != NULL)
					{
						nPercent = m_pRightContainer->GetRecentPercent ();
					}

					if (nPercent == 100 || nPercent == 0)
					{
						nPercent = 50;
					}

					rectLeft.right = rectLeft.left + 
						rectContainer.Width () - 
						((rectContainer.Width () * nPercent) / 100);
					nPercent = 100 - nPercent;
				}
				else if (rectRight.Width () == rectContainer.Width ())
				{
					nPercent = 50;
					if (m_pBarLeftTop != NULL)
					{
						nPercent = m_pBarLeftTop->GetLastPercentInContainer ();
					}
					else if (m_pLeftContainer != NULL)
					{
						nPercent = m_pLeftContainer->GetRecentPercent ();
					}

					if (nPercent == 100 || nPercent == 0)
					{
						nPercent = 50;
					}

					rectLeft.right = rectLeft.left + 
						((rectContainer.Width () * nPercent) / 100);
				}
			}

			int nDelta = rect.Width () - rectContainer.Width ();
			dLeftPercent = ((double) rectLeft.Width ()) / 
									rectContainer.Width () * 100;


			if (dLeftPercent == 100 || dLeftPercent == 0)
			{
				dLeftPercent = 50;
			}


			if (m_bMaintainPercentage)
			{
				if (nDelta != 0)
				{
					rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width () + 
									(int) (((double) nDelta * dLeftPercent) / 100.);
				}
				else
				{
					rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width ();
					if (nPercent != -1)
					{
						dLeftPercent = nPercent;
					}
				}
			}
			else
			{
                if (m_bRetainInternalSliderPosition)
                {
				    if (nDelta > 0)
				    {
					    rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width ();
				    }
				    else if (nDelta < 0)
				    {
					    rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width ();
				    }
				    else
				    {
					    rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width ();
					    if (nPercent != -1)
					    {
						    dLeftPercent = nPercent;
					    }
				    }
                    if (CWnd::GetCapture() != m_pSlider)
                    {
                        CRect rc(rectSlider);
                        m_pSlider->GetParent()->ScreenToClient(rc);
                        rectFinalLeft.right = rc.left;
                    }
                    dLeftPercent =   rectFinalLeft.Width() 
                                    / (static_cast<double>(rectContainer.Width()));
                }
                else
                {
				    if (nDelta > 0)
				    {
					    rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width () + 
									    (int) (((double) nDelta * (100 - dLeftPercent)) / 100.);
				    }
				    else if (nDelta < 0)
				    {
					    rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width () + 
									    (int) (((double) nDelta * dLeftPercent) / 100.);
				    }
				    else
				    {
					    rectFinalLeft.right = rectFinalLeft.left + rectLeft.Width ();
					    if (nPercent != -1)
					    {
						    dLeftPercent = nPercent;
					    }
				    }
                }
			}

			rectFinalSlider.left = rectFinalLeft.right;
			rectFinalSlider.right = rectFinalSlider.left + m_pSlider->GetWidth ();
			rectFinalRight.left = rectFinalSlider.right;
			
			if (CBCGPControlBar::m_bHandleMinSize)
			{
				int deltaLeft = sizeMinLeft.cx - rectFinalLeft.Width();
				int deltaRight = sizeMinRight.cx - rectFinalRight.Width ();
				int nSliderWidth = m_pSlider->GetWidth ();

				if (deltaLeft <= 0 && deltaRight <= 0)
				{
				}
				else if (deltaLeft > 0 && deltaRight <= 0)
				{
					rectFinalLeft.right += deltaLeft;
					rectFinalRight.left = rectFinalLeft.right + nSliderWidth;
					if (rectFinalRight.Width () < sizeMinRight.cx)
					{
						rectFinalRight.right = rectFinalRight.left + sizeMinRight.cx;	
					}
				}
				else if (deltaLeft <= 0 && deltaRight > 0)
				{
					rectFinalLeft.right -= deltaRight;
					if (rectFinalLeft.Width () < sizeMinLeft.cx)
					{
						rectFinalLeft.right = rectFinalLeft.left + sizeMinLeft.cx;
					}
					
					rectFinalRight.left = rectFinalLeft.right + nSliderWidth;
					rectFinalRight.right = rectFinalRight.left + sizeMinRight.cx;	
				}
				else if (deltaLeft > 0 && deltaRight > 0)
				{
					rectFinalLeft.right = rectFinalLeft.left + sizeMinLeft.cx;
					rectFinalRight.left = rectFinalLeft.right + nSliderWidth;
					rectFinalRight.right = rectFinalRight.left + sizeMinRight.cx;	
				}

				rectFinalSlider.left = rectFinalLeft.right;
				rectFinalSlider.right = rectFinalSlider.left + nSliderWidth;	
				
				dLeftPercent = ((double) rectFinalLeft.Width ()) / 
										rectContainer.Width () * 100;

				if (rectFinalLeft.Height () < sizeMinLeft.cy)
				{
					rectFinalLeft.bottom = rectFinalLeft.top + sizeMinLeft.cy;
					rectFinalRight.bottom = rectFinalRight.top + sizeMinLeft.cy;
				}
			}				

		}

		if (m_pBarLeftTop != NULL)
		{
			hdwp = m_pBarLeftTop->MoveWindow (rectFinalLeft, bRedraw, hdwp);
			m_pBarLeftTop->SetLastPercentInContainer ((int) dLeftPercent);
		}

		if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->ResizeContainer (rectFinalLeft, hdwp, bRedraw);
			m_pLeftContainer->SetRecentPercent ((int) dLeftPercent);
		}

		if (m_pBarRightBottom != NULL)
		{
			hdwp = m_pBarRightBottom->MoveWindow (rectFinalRight, bRedraw, hdwp);
			m_pBarRightBottom->SetLastPercentInContainer (100 - (int) dLeftPercent);
		}

		if (m_pRightContainer != NULL)
		{
			m_pRightContainer->ResizeContainer (rectFinalRight, hdwp, bRedraw);
			m_pRightContainer->SetRecentPercent (100 - (int) dLeftPercent);
		}		

		if (m_pSlider->IsBarVisible ())
		{
			hdwp = m_pSlider->MoveWindow (rectFinalSlider, bRedraw, hdwp);
		}

	}
}
//-----------------------------------------------------------------------------------//
int CBCGPBarContainer::StretchContainer (int nOffset, BOOL bStretchHorz, BOOL bLeftBar, 
										BOOL bMoveSlider, HDWP& hdwp)
{
	ASSERT_VALID (this);

	
	if ((AfxGetMainWnd ()->GetExStyle () & WS_EX_LAYOUTRTL) && bStretchHorz)
	{
		nOffset = -nOffset;
	}

	int nDirection = nOffset < 0 ? -1 : 1;


	CSize sizeStretch (0, 0);
	bStretchHorz ? sizeStretch.cx = nOffset : sizeStretch.cy = nOffset;

	int nAvailSpace = bStretchHorz ? CalcAvailableSpace (sizeStretch, bLeftBar).cx : 
									CalcAvailableSpace (sizeStretch, bLeftBar).cy;
	// set the sign here
	int nActualSize = nDirection * min (abs (nOffset), abs (nAvailSpace));

	if (abs (nActualSize) == 0)
	{
		return 0;
	}
	
	// check whether the container's native slider has the same 
	// orientation as stretch direction
	if (m_pSlider == NULL || (m_pSlider->IsHorizontal () && bStretchHorz || 
		!m_pSlider->IsHorizontal () && !bStretchHorz))
	{
		// just use minimum of the avail. and req to stretch both bars and the 
		// slider
		ResizeBar (nActualSize, m_pBarLeftTop, m_pLeftContainer, bStretchHorz, bLeftBar, hdwp);
		ResizeBar (nActualSize, m_pBarRightBottom, m_pRightContainer, bStretchHorz, bLeftBar, hdwp);
		// resize the slider
		if (bMoveSlider && m_pSlider != NULL)
		{
			CRect rectSlider;
			m_pSlider->GetWindowRect (rectSlider);
			if (m_pSlider->IsHorizontal ())
			{
				bLeftBar ? rectSlider.right += nActualSize : 
						   rectSlider.left += nActualSize;
			}
			else
			{
				bLeftBar ? rectSlider.bottom += nActualSize : 
							rectSlider.top += nActualSize;
			}
			if (m_pSlider->IsBarVisible ())
			{
				m_pSlider->GetParent ()->ScreenToClient (rectSlider);
				m_pSlider->MoveWindow (rectSlider, FALSE, hdwp);
			}
		}
	}
	else 
	{
		// treat bar's available space individually
		int nLeftAvailOffset  = CalcAvailableBarSpace (nOffset, m_pBarLeftTop, m_pLeftContainer, bLeftBar);
		int nRigthAvailOffset = CalcAvailableBarSpace (nOffset, m_pBarRightBottom, m_pRightContainer, bLeftBar);

		int nSliderOffset = 0;
		int nBarOffset = nActualSize; 
		if (abs (nLeftAvailOffset) == abs (nRigthAvailOffset))
		{
			nSliderOffset = (abs (nLeftAvailOffset) / 2 + 1) * nDirection;
		}
		else 
		{
			nSliderOffset = nActualSize;
		}

		CPoint pt (0, 0);
		bStretchHorz ? pt.x = nSliderOffset : pt.y = nSliderOffset;

		if (bMoveSlider)
		{
			m_pSlider->MoveSlider (pt);
		}

		if (bLeftBar)
		{
			ResizeBar (nBarOffset, m_pBarRightBottom, m_pRightContainer, bStretchHorz, bLeftBar, hdwp);
		}
		else
		{
			ResizeBar (nBarOffset, m_pBarLeftTop, m_pLeftContainer, bStretchHorz, bLeftBar, hdwp);
		}
	}

	return nActualSize;
}
//-----------------------------------------------------------------------------------//
int CBCGPBarContainer::OnMoveInternalSlider (int nOffset, HDWP& hdwp)
{

	ASSERT_VALID (this);
	ASSERT_VALID (m_pSlider);

	CRect rectLeft; rectLeft.SetRectEmpty ();
	CRect rectRight; rectRight.SetRectEmpty ();

	CSize sizeMinLeft;
	CSize sizeMinRight;

	if (m_pBarLeftTop != NULL)
	{
		m_pBarLeftTop->GetWindowRect (rectLeft);
		m_pBarLeftTop->GetMinSize (sizeMinLeft);
	}

	if (m_pLeftContainer != NULL)
	{
		m_pLeftContainer->GetWindowRect (rectLeft);
		m_pLeftContainer->GetMinSize (sizeMinLeft);
	}

	if (m_pBarRightBottom != NULL)
	{
		m_pBarRightBottom->GetWindowRect (rectRight);
		m_pBarRightBottom->GetMinSize (sizeMinRight);
	}

	if (m_pRightContainer != NULL)
	{
		m_pRightContainer->GetWindowRect (rectRight); 
		m_pRightContainer->GetMinSize (sizeMinRight);
	}

	m_pSlider->GetParent ()->ScreenToClient (rectLeft);
	m_pSlider->GetParent ()->ScreenToClient (rectRight);

	if (!rectLeft.IsRectEmpty ())
	{
		if (IsSliderHorz ())
		{
			rectLeft.bottom += nOffset;
			if (rectLeft.Height () < sizeMinLeft.cy)
			{
				rectLeft.bottom = rectLeft.top + sizeMinLeft.cy;
			}
		}
		else
		{
			rectLeft.right += nOffset;
			if (rectLeft.Width () < sizeMinLeft.cx)
			{
				rectLeft.right = rectLeft.left + sizeMinLeft.cx;
			}
		}
	}

	if (!rectRight.IsRectEmpty ())
	{
		if (IsSliderHorz ()) 
		{
			rectRight.top += nOffset;
			if (rectRight.Height () < sizeMinRight.cy)
			{
				rectRight.top = rectRight.bottom - sizeMinRight.cy;
			}
		}
		else
		{
			rectRight.left += nOffset;
			if (rectRight.Width () < sizeMinRight.cx)
			{
				rectRight.left = rectRight.right - sizeMinRight.cx;
			}
		}
	}


	if (m_pBarLeftTop != NULL)
	{
		hdwp = m_pBarLeftTop->MoveWindow (rectLeft, TRUE, hdwp);
	}

	if (m_pLeftContainer != NULL)
	{
		m_pLeftContainer->ResizeContainer (rectLeft, hdwp);
	}

	if (m_pBarRightBottom != NULL)
	{
		hdwp = m_pBarRightBottom->MoveWindow (rectRight, TRUE, hdwp);
	}

	if (m_pRightContainer != NULL)
	{
		m_pRightContainer->ResizeContainer (rectRight, hdwp); 
	}

	return nOffset;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::ResizeBar (int nOffset, CBCGPControlBar* pBar, 
								  CBCGPBarContainer* pContainer, 
								  BOOL bHorz, 
								  BOOL bLeftBar, HDWP& hdwp)
{
	ASSERT_VALID (this);
	if (pBar != NULL)
	{
		CRect rectBar;
		pBar->GetWindowRect (rectBar);
		if (bHorz)
		{
			bLeftBar ? rectBar.bottom += nOffset : rectBar.top -= nOffset;  
		}	
		else
		{
			bLeftBar ? rectBar.right += nOffset : rectBar.left += nOffset;
		}
		pBar->MoveControlBar (rectBar, FALSE, hdwp);
	}
	else if (pContainer != NULL)
	{
		// the container will be stretched by "foregn" slider, threfore
		// if the native bar's slider is horizontal, a container
		// will be stretched vertically
		pContainer->StretchContainer (nOffset, bHorz, bLeftBar, TRUE, hdwp);
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::Move (CPoint ptNewLeftTop)
{
	ASSERT_VALID (this);

	CRect rectLeft; rectLeft.SetRectEmpty ();
	CRect rectRight; rectRight.SetRectEmpty ();

	int nLeftOffset = 0;
	int nTopOffset = 0;

	if (m_pBarLeftTop != NULL)
	{
		m_pBarLeftTop->GetWindowRect (rectLeft);
		m_pBarLeftTop->SetWindowPos (NULL, ptNewLeftTop.x, ptNewLeftTop.y, 0, 0, 
									 SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	if (m_pLeftContainer != NULL)
	{
		m_pLeftContainer->GetWindowRect (rectLeft);
		m_pLeftContainer->Move (ptNewLeftTop);

	}

	nLeftOffset = rectLeft.Width ();
	nTopOffset = rectLeft.Height ();

	if (m_pSlider != NULL)
	{
		if (m_pSlider->IsHorizontal ())
		{
			m_pSlider->SetWindowPos (NULL, ptNewLeftTop.x, ptNewLeftTop.y + nTopOffset,
									 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			nTopOffset += m_pSlider->GetWidth ();
			nLeftOffset = 0;
		}
		else
		{
			m_pSlider->SetWindowPos (NULL, ptNewLeftTop.x + nLeftOffset, ptNewLeftTop.y,
									 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
			nLeftOffset += m_pSlider->GetWidth ();
			nTopOffset = 0;
		}
	}


	if (m_pBarRightBottom != NULL)
	{
		m_pBarRightBottom->SetWindowPos (NULL, ptNewLeftTop.x + nLeftOffset, 
										 ptNewLeftTop.y + nTopOffset,
										 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	if (m_pRightContainer != NULL)
	{
		CPoint pt (ptNewLeftTop.x + nLeftOffset, ptNewLeftTop.y + nTopOffset); 
		m_pRightContainer->Move (pt);
	}
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::MoveWnd (CWnd* pWnd, int nOffset, BOOL bHorz)
{
	ASSERT_VALID (this);

	
	if (pWnd != NULL)
	{
		CWnd* pParent = pWnd->GetParent ();
		ASSERT_VALID (pParent);

		CRect rectWnd;
		CRect rectParent;

		pParent->GetClientRect (rectParent);
		pWnd->GetWindowRect (rectWnd);
		pParent->ScreenToClient (rectWnd);

		int nActualOffset = bHorz ? rectWnd.left - rectParent.left : 
									rectWnd.top - rectParent.top;

		bHorz ? rectWnd.OffsetRect (CPoint (nOffset - nActualOffset, 0)) : rectWnd.OffsetRect (CPoint (0, nOffset - nActualOffset));
		
		pWnd->MoveWindow (rectWnd, TRUE);
	}
}
//-----------------------------------------------------------------------------------//
int CBCGPBarContainer::CalcAvailableBarSpace (int nRequiredOffset, 
											CBCGPControlBar* pBar,
											CBCGPBarContainer* pContainer,	
											BOOL bLeftBar)
{
	ASSERT_VALID (this);

	CRect rectBar;
	int nAvailableSpace = nRequiredOffset;
	
	if (pBar != NULL)
	{
		ASSERT_VALID (pBar);
		pBar->GetWindowRect (rectBar);
		if (IsSliderHorz ())
		{
			bLeftBar ? rectBar.bottom += nRequiredOffset :  
						rectBar.top += nRequiredOffset;  
			nAvailableSpace = pBar->CalcAvailableSize (rectBar).cy;
		}	
		else
		{
			bLeftBar ? rectBar.right += nRequiredOffset : 
						rectBar.left += nRequiredOffset;
			nAvailableSpace = pBar->CalcAvailableSize (rectBar).cx;
		}
	}
	else if (pContainer != NULL) 
	{
		ASSERT_VALID (pContainer);
		nAvailableSpace = IsSliderHorz () ? 
			pContainer->CalcAvailableSpace (CSize (0, nRequiredOffset), bLeftBar).cy : 
			pContainer->CalcAvailableSpace (CSize (nRequiredOffset, 0), bLeftBar).cx;
	}

	return nAvailableSpace;
}
//-----------------------------------------------------------------------------------//
CSize CBCGPBarContainer::CalcAvailableSpace (CSize sizeStretch, BOOL bLeftBar)
{
	ASSERT_VALID (this);

	CRect rectWndOrg;
	GetWindowRect (rectWndOrg);
	CRect rectWndNew = rectWndOrg;

	if (bLeftBar)
	{
		rectWndNew.right += sizeStretch.cx;
		rectWndNew.bottom += sizeStretch.cy;
	}
	else
	{
		rectWndNew.left += sizeStretch.cx;
		rectWndNew.top += sizeStretch.cy;
	}

	CSize sizeMin;
	GetMinSize (sizeMin);

	CSize sizeAvailable (sizeStretch.cx, sizeStretch.cy);

	if (rectWndNew.Width () < sizeMin.cx)
	{
		sizeAvailable.cx = rectWndOrg.Width () - sizeMin.cx;
		// if already less or eq. to minimum
		if (sizeAvailable.cx < 0)
		{
			sizeAvailable.cx = 0;
		}

		// preserve direction
		if (sizeStretch.cx < 0)
		{
			sizeAvailable.cx = -sizeAvailable.cx;
		}
	}

	if (rectWndNew.Height () < sizeMin.cy)
	{
		sizeAvailable.cy = rectWndNew.Height () - sizeMin.cy;
		if (sizeAvailable.cy < 0)
		{
			sizeAvailable.cy = 0;
		}

		// preserve direction
		if (sizeStretch.cy < 0)
		{
			sizeAvailable.cy = -sizeAvailable.cy;
		}
	}

	return sizeAvailable;
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainer::IsLeftContainer () const
{
	ASSERT_VALID (this);

	if (m_pParentContainer == NULL)
	{
		return TRUE;
	}

	if (m_pParentContainer->GetLeftContainer () == this)
	{
		return TRUE;
	}

	if (m_pParentContainer->GetRightContainer () == this)
	{
		return FALSE;
	}

	return FALSE;
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainer::IsLeftBar (CBCGPDockingControlBar* pBar) const
{
	if (pBar == m_pBarLeftTop)
	{
		return TRUE;
	}

	if (pBar == m_pBarRightBottom)
	{
		return FALSE;
	}

	return FALSE;
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainer::IsContainerEmpty () const
{
	ASSERT_VALID (this);

	return  (m_pBarLeftTop == NULL && m_pBarRightBottom == NULL && 
			 (m_pLeftContainer == NULL || m_pLeftContainer->IsContainerEmpty ()) &&
			 (m_pRightContainer == NULL || m_pRightContainer->IsContainerEmpty ()));
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainer::IsLeftPartEmpty (BOOL bCheckVisibility) const
{
	ASSERT_VALID (this);
	return  ((m_pBarLeftTop == NULL || 
			  bCheckVisibility && m_pBarLeftTop != NULL  && !m_pBarLeftTop->IsBarVisible ()) &&
			 (m_pLeftContainer == NULL || m_pLeftContainer->IsContainerEmpty () || 
			  bCheckVisibility && m_pLeftContainer != NULL && !m_pLeftContainer->IsContainerVisible ()));
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainer::IsRightPartEmpty (BOOL bCheckVisibility) const
{
	ASSERT_VALID (this);
	return  ((m_pBarRightBottom == NULL || 
			  bCheckVisibility && m_pBarRightBottom != NULL && !m_pBarRightBottom->IsBarVisible ()) && 
			 (m_pRightContainer == NULL || m_pRightContainer->IsContainerEmpty () ||
			  bCheckVisibility && m_pRightContainer != NULL && !m_pRightContainer->IsContainerVisible ()));
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainer::IsContainerVisible () const
{
	ASSERT_VALID (this);

	return (m_pBarLeftTop != NULL && m_pBarLeftTop->IsBarVisible () ||
			m_pBarRightBottom != NULL && m_pBarRightBottom->IsBarVisible () ||
			m_pLeftContainer != NULL && m_pLeftContainer->IsContainerVisible () ||
			m_pRightContainer != NULL && m_pRightContainer->IsContainerVisible ());
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::CheckSliderVisibility ()
{
	ASSERT_VALID (this);

	BOOL bLeftContainerVisible = FALSE;
	BOOL bRightContainerVisible = FALSE;
	BOOL bLeftBarVisible = m_pBarLeftTop != NULL && m_pBarLeftTop->IsBarVisible ();
	BOOL bRightBarVisible = m_pBarRightBottom != NULL && m_pBarRightBottom->IsBarVisible ();

	if (m_pLeftContainer != NULL)
	{
		m_pLeftContainer->CheckSliderVisibility ();
		bLeftContainerVisible = m_pLeftContainer->IsContainerVisible ();
	}

	if (m_pRightContainer != NULL)
	{
		m_pRightContainer->CheckSliderVisibility ();
		bRightContainerVisible = m_pRightContainer->IsContainerVisible ();
	}

	if (m_pSlider == NULL)
	{
		return;
	}

	BOOL bShow = FALSE;
	if (bLeftBarVisible && bRightBarVisible ||
		bLeftBarVisible && bRightContainerVisible ||
		bRightBarVisible && bLeftContainerVisible ||
		bLeftContainerVisible && bRightContainerVisible)
	{
		bShow = TRUE;
	}

	m_pSlider->ShowWindow (bShow ? SW_SHOW : SW_HIDE);

}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::Serialize (CArchive& ar)
{
	CObject::Serialize (ar);

	if (ar.IsStoring ())
	{
		if (m_pBarLeftTop != NULL)
		{
			int nBarID = m_pBarLeftTop->GetDlgCtrlID ();
			if (nBarID != -1)
			{
				ar << nBarID;
			}
			else 
			{
				SaveTabbedBar (ar, m_pBarLeftTop);
			}

		}
		else
		{
			ar << (int) 0;
		}

		if (m_pBarRightBottom != NULL)
		{
			int nBarID = m_pBarRightBottom->GetDlgCtrlID ();
			if (nBarID != -1)
			{
				ar << nBarID;
			}
			else
			{
				SaveTabbedBar (ar, m_pBarRightBottom);
				
			}
		}
		else
		{
			ar << (int) 0;
		}

		if (m_pSlider != NULL)
		{
			ar << m_pSlider->GetDlgCtrlID ();
			m_pSlider->Serialize (ar);
		}
		else
		{
			ar << (int) 0;
		}

		ar << (BOOL)(m_pLeftContainer != NULL);
		if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->Serialize (ar);
		}
		
		ar << (BOOL)(m_pRightContainer != NULL);
		if (m_pRightContainer != NULL)
		{
			m_pRightContainer->Serialize (ar);
		}
	}
	else
	{
		ar >> m_nSavedLeftBarID;

		if (m_nSavedLeftBarID == -1)
		{
			m_pBarLeftTop = LoadTabbedBar (ar, m_lstSavedSiblingBarIDsLeft);
		}

		ar >> m_nSavedRightBarID;

		if (m_nSavedRightBarID == -1)
		{
			m_pBarRightBottom = LoadTabbedBar (ar, m_lstSavedSiblingBarIDsRight);
		}

		ar >> m_nSavedSliderID;	
		
		if (m_nSavedSliderID != NULL)
		{
			m_pSlider = DYNAMIC_DOWNCAST (CBCGPSlider, CBCGPSlider::m_pSliderRTC->CreateObject ());
			ASSERT_VALID (m_pSlider);

			m_pSlider->Init (FALSE, m_pContainerManager->m_pDockSite);
			m_pSlider->Serialize (ar);
			m_pSlider->SetContainerManager (m_pContainerManager);
			m_pContainerManager->m_lstSliders.AddTail (m_pSlider);
		}

		BOOL bLeftContainerPresent = FALSE;
		ar >> bLeftContainerPresent;

		CRuntimeClass* pContainerRTC = m_pContainerManager->GetContainerRTC ();

		if (bLeftContainerPresent)
		{
			if (pContainerRTC == NULL)
			{
				m_pLeftContainer = new CBCGPBarContainer (m_pContainerManager);
			}	
			else
			{
				m_pLeftContainer = (CBCGPBarContainer*) pContainerRTC->CreateObject ();
				m_pLeftContainer->SetContainerManager (m_pContainerManager);
			}
			m_pLeftContainer->Serialize (ar);
			m_pLeftContainer->SetParentContainer (this);
		}

		BOOL bRightContainerPresent = FALSE;
		ar >> bRightContainerPresent;

		if (bRightContainerPresent)
		{
			if (pContainerRTC == NULL)
			{
				m_pRightContainer = new CBCGPBarContainer (m_pContainerManager);
			}	
			else
			{
				m_pRightContainer = (CBCGPBarContainer*) pContainerRTC->CreateObject ();
				m_pRightContainer->SetContainerManager (m_pContainerManager);
			}

			m_pRightContainer->Serialize (ar);
			m_pRightContainer->SetParentContainer (this);
		}
	}
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainer::SetUpByID (UINT nID, CBCGPDockingControlBar* pBar)
{
	ASSERT_KINDOF (CBCGPDockingControlBar, pBar);
	if (m_nSavedLeftBarID == nID)
	{
		m_pBarLeftTop = pBar;
		return TRUE;
	}

	if (m_nSavedRightBarID == nID)
	{
		m_pBarRightBottom = pBar;
		return TRUE;
	}

	if (m_pLeftContainer != NULL && 
		m_pLeftContainer->SetUpByID (nID, pBar))
	{
		return TRUE;
	}

	if (m_pRightContainer != NULL)
	{
		return m_pRightContainer->SetUpByID (nID, pBar);
	}

	return FALSE;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::SaveTabbedBar (CArchive& ar, CBCGPDockingControlBar* pBar)
{
	ASSERT_KINDOF (CBCGPBaseTabbedBar, pBar);
	CBCGPBaseTabbedBar* pTabbedBar = DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pBar);
	ASSERT (ar.IsStoring ());

	if (pTabbedBar->GetTabsNum () > 0)
	{
		ar << (int) -1;
		pTabbedBar->SaveSiblingBarIDs (ar);
		ar << pTabbedBar;
		ar << pTabbedBar->GetStyle ();

		pTabbedBar->SerializeTabWindow (ar);
	}
}
//-----------------------------------------------------------------------------------//
CBCGPDockingControlBar* CBCGPBarContainer::LoadTabbedBar (CArchive& ar, 
																CList<UINT, UINT>& lstBarIDs)
{
	ASSERT (ar.IsLoading ());

	CBCGPDockingControlBar* pBar = NULL;
	DWORD dwStyle = 0;

	CBCGPBaseTabbedBar::LoadSiblingBarIDs (ar, lstBarIDs);
	ar >> pBar;
	ar >> dwStyle;

	if (!pBar->Create (_T (""), m_pContainerManager->m_pDockSite, 
							 pBar->m_rectSavedDockedRect, TRUE, (UINT) -1, 
							 dwStyle, pBar->GetBCGStyle ()))
	{
		TRACE0 ("Failed to create tab docking bar");
		ASSERT (FALSE);
		lstBarIDs.RemoveAll ();
		delete pBar;
		return NULL;
	}

	ASSERT_KINDOF (CBCGPBaseTabbedBar, pBar);
	((CBCGPBaseTabbedBar*) pBar)->SerializeTabWindow (ar);

	return pBar;
}
//-----------------------------------------------------------------------------------//
CBCGPDockingControlBar* CBCGPBarContainer::FindTabbedBar (UINT nID)
{
	ASSERT_VALID (this);

	if (m_lstSavedSiblingBarIDsLeft.Find (nID) != NULL)
	{
		return m_pBarLeftTop;
	}

	if (m_lstSavedSiblingBarIDsRight.Find (nID) != NULL)
	{
		return m_pBarRightBottom;
	}

	if (m_pLeftContainer != NULL)
	{
		CBCGPDockingControlBar* pBar = m_pLeftContainer->FindTabbedBar (nID);
		if (pBar != NULL)
		{
			return pBar;
		}
	}

	if (m_pRightContainer != NULL)
	{
		return m_pRightContainer->FindTabbedBar (nID);
	}

	return NULL;
}
//-----------------------------------------------------------------------------------//
CList<UINT, UINT>* CBCGPBarContainer::GetAssociatedSiblingBarIDs (CBCGPDockingControlBar* pBar)
{
	if (pBar == m_pBarLeftTop)
	{
		return &m_lstSavedSiblingBarIDsLeft;
	}

	if (pBar == m_pBarRightBottom)
	{
		return &m_lstSavedSiblingBarIDsRight;
	}
	return NULL;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::SetContainerManager (CBCGPBarContainerManager* p, BOOL bDeep) 
{
	m_pContainerManager = p;

	if (bDeep)
	{
		if (m_pLeftContainer != NULL)
		{
			m_pLeftContainer->SetContainerManager (p, bDeep);
		}

		if (m_pRightContainer != NULL)
		{
			m_pRightContainer->SetContainerManager (p, bDeep);
		}
	}
}
//-----------------------------------------------------------------------------------//
int  CBCGPBarContainer::GetNodeCount () const
{
	int nCount = 1;

	if (m_pLeftContainer != NULL)
	{
		nCount += m_pLeftContainer->GetNodeCount ();
	}

	if (m_pRightContainer != NULL)
	{
		nCount += m_pRightContainer->GetNodeCount ();
	}

	return nCount;
}
//-----------------------------------------------------------------------------------//
CBCGPBarContainer* CBCGPBarContainer::Copy (CBCGPBarContainer* pParentContainer)
{
	// we should copy container and pointers to contained bars 
	// only if these bars are visible;
	// unvisible parts of the new container shold be cleared
	CRuntimeClass* pContainerRTC = m_pContainerManager->GetContainerRTC ();
	CBCGPBarContainer* pNewContainer = NULL; 

	if (pContainerRTC == NULL)
	{
		pNewContainer = new CBCGPBarContainer (m_pContainerManager, m_pBarLeftTop, m_pBarRightBottom, m_pSlider);
	}	
	else
	{
		pNewContainer = (CBCGPBarContainer*) pContainerRTC->CreateObject ();
		pNewContainer->SetContainerManager (m_pContainerManager);
		pNewContainer->SetBar (m_pBarLeftTop, TRUE);
		pNewContainer->SetBar (m_pBarRightBottom, FALSE);
		pNewContainer->SetSlider (m_pSlider);
	}

	if (m_pBarLeftTop != NULL)
	{
		if (m_pBarLeftTop->GetStyle () & WS_VISIBLE)
		{
			m_pBarLeftTop = NULL;
		}
		else
		{
			pNewContainer->SetBar (NULL, TRUE);
		}
	}
	if (m_pBarRightBottom != NULL)
	{
		if (m_pBarRightBottom->GetStyle () & WS_VISIBLE)
		{
			m_pBarRightBottom = NULL;
		}
		else
		{
			pNewContainer->SetBar (NULL, FALSE);
		}
	}

	pNewContainer->SetParentContainer (pParentContainer);

	if (m_pLeftContainer != NULL)
	{
		CBCGPBarContainer* pNewLeftContainer = m_pLeftContainer->Copy (pNewContainer);
		pNewContainer->SetContainer (pNewLeftContainer, TRUE);
	}

	if (m_pRightContainer != NULL)
	{

		CBCGPBarContainer* pNewRightContainer = m_pRightContainer->Copy (pNewContainer);
		pNewContainer->SetContainer (pNewRightContainer, FALSE);
	}

	if (m_pSlider != NULL)
	{
		if (m_pSlider->GetStyle () & WS_VISIBLE)
		{
			m_dwRecentSliderStyle = m_pSlider->GetSliderStyle  ();
			m_pSlider->GetClientRect (m_rectRecentSlider);
			m_bIsRecentSliderHorz = m_pSlider->IsHorizontal ();
			m_pSlider = NULL;
		}
		else
		{
			pNewContainer->SetSlider (NULL);
		}
	}

	return pNewContainer;
}
//-----------------------------------------------------------------------------------//
BOOL CBCGPBarContainer::IsSliderHorz () const
{
	return m_pSlider != NULL ? m_pSlider->IsHorizontal () : 
								m_bIsRecentSliderHorz;
}
//-----------------------------------------------------------------------------------//
int  CBCGPBarContainer::GetTotalReferenceCount () const
{
	int nRefCount = m_dwRefCount;

	if (m_pRightContainer != NULL)
	{
		nRefCount += m_pRightContainer->GetTotalReferenceCount ();
	}

	if (m_pLeftContainer != NULL)
	{
		nRefCount += m_pLeftContainer->GetTotalReferenceCount ();
	}

	return nRefCount;
}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::RemoveNonValidBars ()
{
	if (m_pContainerManager == NULL)
	{
		return;
	}

	if (m_pBarLeftTop != NULL)
	{
		if (!m_pContainerManager->CheckAndRemoveNonValidBar (m_pBarLeftTop))
		{
			m_pBarLeftTop = NULL;
		}
	}

	if (m_pLeftContainer != NULL)
	{
		m_pLeftContainer->RemoveNonValidBars ();
	}

	if (m_pBarRightBottom != NULL)
	{
		if (!m_pContainerManager->CheckAndRemoveNonValidBar (m_pBarRightBottom))
		{
			m_pBarRightBottom = NULL;
		}
	}

	if (m_pRightContainer != NULL)
	{
		m_pRightContainer->RemoveNonValidBars ();
	}

}
//-----------------------------------------------------------------------------------//
void CBCGPBarContainer::OnShowControlBar (CBCGPDockingControlBar* pBar, BOOL bShow)
{
	if (bShow)
	{
		return;
	}

	CWnd* pDockSite = m_pContainerManager->GetDockSite ();
	ASSERT_VALID (pDockSite);

	CRect rectContainer;
	GetWindowRect (rectContainer, TRUE);
	pDockSite->ScreenToClient (rectContainer);
	
	if (m_pBarLeftTop != NULL && m_pBarLeftTop != pBar)
	{
		m_pBarLeftTop->SetWindowPos (NULL, rectContainer.left, rectContainer.top,
			rectContainer.Width (), rectContainer.Height (), 
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else if (m_pBarRightBottom != NULL && m_pBarRightBottom != pBar)
	{
		m_pBarRightBottom->SetWindowPos (NULL, rectContainer.left, rectContainer.top,
			rectContainer.Width (), rectContainer.Height (), 
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else if (m_pLeftContainer != NULL)
	{
		HDWP hdwp = NULL;
		m_pLeftContainer->ResizeContainer (rectContainer, hdwp, TRUE);
	}
	else if (m_pRightContainer != NULL)
	{
		HDWP hdwp = NULL;
		m_pRightContainer->ResizeContainer (rectContainer, hdwp, TRUE);
	}
	else
	{
		// find parent container
		// to be implemented
	}
}
