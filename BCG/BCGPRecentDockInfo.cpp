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
// BCGPRecentDockInfo.cpp: implementation of the CBCGPRecentDockInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPRecentDockInfo.h"

#include "BCGPBarContainerManager.h"
#include "BCGPBarContainer.h"
#include "BCGPDockBar.h"
#include "BCGPDockBarRow.h"
#include "BCGPSlider.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPDockingControlBar.h"
#include "BCGPBaseTabWnd.h"
#include "BCGPBaseTabbedBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CBCGPRecentContainerInfo::CBCGPRecentContainerInfo ()
{
	Init ();
}
//*****************************************************************************
CBCGPRecentContainerInfo::~CBCGPRecentContainerInfo ()
{

}
//*****************************************************************************
void CBCGPRecentContainerInfo::Init ()
{
	m_pRecentBarContainer = NULL;
	m_rectDockedRect.SetRect (0, 0, 30, 30);
	m_nRecentPercent = 50;
	m_bIsRecentLeftBar = TRUE;
	m_pRecentContanerOfTabWnd = NULL;
}
//*****************************************************************************
void CBCGPRecentContainerInfo::StoreDockInfo (CBCGPBarContainer* pRecentContainer, 
										 CBCGPDockingControlBar* pBar, 
										 CBCGPDockingControlBar* pTabbedBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pRecentContainer);

	try
	{
		if (pRecentContainer == m_pRecentBarContainer &&
			m_pRecentBarContainer->m_dwRefCount == 0 ||
			pRecentContainer->m_dwRefCount < 0)
		{
			//ASSERT (FALSE);
		}

		if (pRecentContainer != NULL)
		{
			pRecentContainer->AddRef ();
			m_bIsRecentLeftBar = (pTabbedBar != NULL) ? 
				pRecentContainer->IsLeftBar (pTabbedBar) : pRecentContainer->IsLeftBar (pBar);
		}

		if (m_pRecentBarContainer != NULL && !m_pRecentBarContainer->IsDisposed ())
		{
			CBCGPBarContainerManager* pManager = m_pRecentBarContainer->m_pContainerManager;
			m_pRecentBarContainer->m_dwRefCount--;
			
			//m_pRecentBarContainer->Release ();
			if (m_pRecentBarContainer->m_dwRefCount <= 0)
			{
				pManager->m_pRootContainer->ReleaseEmptyContainer ();
			}
			m_pRecentBarContainer = NULL;
		}

		if (m_pRecentContanerOfTabWnd != NULL && !m_pRecentContanerOfTabWnd->IsDisposed ())
		{
			CBCGPBarContainerManager* pManager = m_pRecentContanerOfTabWnd->m_pContainerManager;
			m_pRecentContanerOfTabWnd->m_dwRefCount--;
			if (m_pRecentContanerOfTabWnd->m_dwRefCount <= 0)
			{
				pManager->m_pRootContainer->ReleaseEmptyContainer ();
			}
			
			m_pRecentContanerOfTabWnd = NULL;
		}

		pBar->GetWindowRect (m_rectDockedRect);
		if (pTabbedBar == NULL)
		{
			m_pRecentBarContainer = pRecentContainer;
		}
		else
		{
			m_pRecentContanerOfTabWnd = pRecentContainer;
		}

		m_nRecentPercent = (pTabbedBar != NULL) ? 
			pTabbedBar->GetLastPercentInContainer () : pBar->GetLastPercentInContainer ();

		if (m_pRecentBarContainer != NULL && m_pRecentBarContainer->GetRefCount () == 0 ||
			m_pRecentContanerOfTabWnd != NULL && m_pRecentContanerOfTabWnd->GetRefCount () == 0)
		{
			//ASSERT (FALSE);
		}
	}
	catch (...)
	{

	}
}
//*****************************************************************************
void CBCGPRecentContainerInfo::SetInfo (CBCGPRecentContainerInfo& srcInfo)
{
	if (srcInfo.m_pRecentBarContainer != NULL)
	{
		srcInfo.m_pRecentBarContainer->AddRef ();
	}

	if (m_pRecentBarContainer != NULL)
	{
		m_pRecentBarContainer->Release ();
	}

	m_pRecentBarContainer = srcInfo.m_pRecentBarContainer;
	m_rectDockedRect = srcInfo.m_rectDockedRect;
	m_nRecentPercent = srcInfo.m_nRecentPercent;

	if (srcInfo.m_pRecentContanerOfTabWnd != NULL)
	{
		srcInfo.m_pRecentContanerOfTabWnd->AddRef ();
	}

	if (m_pRecentContanerOfTabWnd != NULL)
	{
		m_pRecentContanerOfTabWnd->Release ();
	}
	m_pRecentContanerOfTabWnd = srcInfo.m_pRecentContanerOfTabWnd;

	m_lstRecentListOfBars.RemoveAll ();
	m_lstRecentListOfBars.AddTail (&srcInfo.m_lstRecentListOfBars);
}
//-----------------------------------------------------------------------------//
// CBCGPRecentDockInfo implementation
//-----------------------------------------------------------------------------//
CBCGPRecentDockInfo::CBCGPRecentDockInfo(CBCGPControlBar* pBar)
{
	m_pBar = pBar;
	Init ();	
}
//*****************************************************************************
CBCGPRecentDockInfo::~CBCGPRecentDockInfo()
{

}
//*****************************************************************************
void CBCGPRecentDockInfo::Init ()
{
	m_rectRecentFloatingRect.SetRect (10, 10, 110, 110);
		
	m_nRecentRowIndex = 0;
	m_pRecentDockBar = NULL;
	m_pRecentDockBarRow = NULL;
	m_nRecentTabNumber = -1;
	m_hRecentDefaultSlider = NULL;
	m_hRecentMiniFrame = NULL;
	m_dwRecentAlignmentToFrame = CBRS_ALIGN_LEFT;
}
//*****************************************************************************
void CBCGPRecentDockInfo::CleanUp ()
{
	Init ();
	m_recentSliderInfo.Init ();
	m_recentMiniFrameInfo.Init ();
}
//*****************************************************************************
void CBCGPRecentDockInfo::StoreDockInfo (CBCGPBarContainer* pRecentContainer, 
										 CBCGPDockingControlBar* pTabbedBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pBar);
	ASSERT_KINDOF (CBCGPDockingControlBar, m_pBar); // get here only for docking bars

	CBCGPDockingControlBar* pBar = 	
		DYNAMIC_DOWNCAST (CBCGPDockingControlBar, m_pBar);

	CBCGPSlider* pDefaultSlider = (pTabbedBar != NULL) ? 
		pTabbedBar->GetDefaultSlider () : pBar->GetDefaultSlider ();
	CBCGPMiniFrameWnd* pMiniFrame = pBar->GetParentMiniFrame ();

	if (pMiniFrame != NULL)
	{
		CBCGPMiniFrameWnd* pRecentMiniFrame = 
			DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, 
				CWnd::FromHandlePermanent (m_hRecentMiniFrame));
		 
		m_hRecentMiniFrame = pMiniFrame->GetSafeHwnd ();
		m_recentMiniFrameInfo.StoreDockInfo (pRecentContainer, pBar, pTabbedBar);
		pMiniFrame->ScreenToClient (m_recentMiniFrameInfo.m_rectDockedRect);
		pMiniFrame->GetWindowRect (m_rectRecentFloatingRect);

		if (pRecentMiniFrame != NULL)
		{
			pRecentMiniFrame->PostMessage (BCGPM_CHECKEMPTYMINIFRAME);
		}
	}
	else if (pDefaultSlider != NULL)
	{
		m_recentSliderInfo.StoreDockInfo (pRecentContainer, pBar, pTabbedBar);
		pBar->GetDockSite ()->ScreenToClient (m_recentSliderInfo.m_rectDockedRect);

		m_hRecentDefaultSlider = pDefaultSlider->GetSafeHwnd ();
		m_dwRecentAlignmentToFrame = pDefaultSlider->GetCurrentAlignment ();
	}
	else
	{
		m_hRecentMiniFrame = NULL;
		m_recentMiniFrameInfo.StoreDockInfo (NULL, pBar);
	}
}
//*****************************************************************************
CBCGPBarContainer* CBCGPRecentDockInfo::GetRecentContainer (BOOL bForSlider)
{
	return bForSlider ? m_recentSliderInfo.m_pRecentBarContainer : 
						m_recentMiniFrameInfo.m_pRecentBarContainer;
}			
//*****************************************************************************
CBCGPBarContainer* CBCGPRecentDockInfo::GetRecentTabContainer (BOOL bForSlider)
{
	return bForSlider ? m_recentSliderInfo.m_pRecentContanerOfTabWnd: 
						m_recentMiniFrameInfo.m_pRecentContanerOfTabWnd;
}
	
//*****************************************************************************
CRect& CBCGPRecentDockInfo::GetRecentDockedRect (BOOL bForSlider)
{
	return bForSlider ? m_recentSliderInfo.m_rectDockedRect : 
						m_recentMiniFrameInfo.m_rectDockedRect;
}
//*****************************************************************************
int CBCGPRecentDockInfo::GetRecentDockedPercent (BOOL bForSlider)
{
	return bForSlider ? m_recentSliderInfo.m_nRecentPercent : 
						m_recentMiniFrameInfo.m_nRecentPercent;
}
//*****************************************************************************
BOOL CBCGPRecentDockInfo::IsRecentLeftBar (BOOL bForSlider)
{
	return bForSlider ? m_recentSliderInfo.m_bIsRecentLeftBar : 
						m_recentMiniFrameInfo.m_bIsRecentLeftBar;
}
//*****************************************************************************
void CBCGPRecentDockInfo::SaveListOfRecentBars (CList<HWND, HWND>& lstOrg, BOOL bForSlider) 
{
	if (bForSlider)
	{
		m_recentSliderInfo.m_lstRecentListOfBars.RemoveAll ();
		m_recentSliderInfo.m_lstRecentListOfBars.AddTail (&lstOrg);
	}
	else
	{
		m_recentMiniFrameInfo.m_lstRecentListOfBars.RemoveAll ();
		m_recentMiniFrameInfo.m_lstRecentListOfBars.AddTail (&lstOrg);
	}
}
//*****************************************************************************
CList<HWND,HWND>& CBCGPRecentDockInfo::GetRecentListOfBars (BOOL bForSlider)
{
	return  bForSlider ? m_recentSliderInfo.m_lstRecentListOfBars : 
					m_recentMiniFrameInfo.m_lstRecentListOfBars;
}
//*****************************************************************************
CBCGPSlider* CBCGPRecentDockInfo::GetRecentDefaultSlider ()
{
	return DYNAMIC_DOWNCAST (CBCGPSlider, 
				CWnd::FromHandlePermanent (m_hRecentDefaultSlider));
}
//*****************************************************************************
void CBCGPRecentDockInfo::SetInfo (BOOL bForSlider, CBCGPRecentDockInfo& srcInfo)
{
	if (bForSlider)
	{
		m_dwRecentAlignmentToFrame = srcInfo.m_dwRecentAlignmentToFrame;
		m_hRecentDefaultSlider = srcInfo.m_hRecentDefaultSlider;
		m_recentSliderInfo.SetInfo (srcInfo.m_recentSliderInfo);
	}
	else
	{
		m_rectRecentFloatingRect = srcInfo.m_rectRecentFloatingRect;
		m_hRecentMiniFrame = srcInfo.m_hRecentMiniFrame;
		m_recentMiniFrameInfo.SetInfo (srcInfo.m_recentMiniFrameInfo);
	}
}
//*****************************************************************************
CBCGPRecentDockInfo& CBCGPRecentDockInfo::operator= (CBCGPRecentDockInfo& src)
{
	m_rectRecentFloatingRect	= src.m_rectRecentFloatingRect;
	m_dwRecentAlignmentToFrame	= src.m_dwRecentAlignmentToFrame;
	m_nRecentRowIndex			= src.m_nRecentRowIndex;
	m_pRecentDockBar			= src.m_pRecentDockBar;
	m_pRecentDockBarRow			= src.m_pRecentDockBarRow;
	m_nRecentTabNumber			= src.m_nRecentTabNumber;
	m_hRecentDefaultSlider		= src.m_hRecentDefaultSlider;
	m_hRecentMiniFrame			= src.m_hRecentMiniFrame;
	m_recentSliderInfo			= src.m_recentSliderInfo;
	m_recentMiniFrameInfo		= src.m_recentMiniFrameInfo;

	return *this;
}
//*****************************************************************************
CBCGPRecentContainerInfo& CBCGPRecentContainerInfo::operator= (CBCGPRecentContainerInfo& src)
{
	m_pRecentBarContainer		= src.m_pRecentBarContainer;
	m_rectDockedRect			= src.m_rectDockedRect;
	m_nRecentPercent			= src.m_nRecentPercent;
	m_bIsRecentLeftBar			= src.m_bIsRecentLeftBar;
	m_pRecentContanerOfTabWnd	= src.m_pRecentContanerOfTabWnd;

	m_lstRecentListOfBars.RemoveAll ();	
	m_lstRecentListOfBars.AddTail (&src.m_lstRecentListOfBars);

	return *this;
}