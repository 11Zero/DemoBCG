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
// BCGPSmartDockingManager.cpp: implementation of the CBCGPSmartDockingManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "bcgcbpro.h"
#include "BCGPSmartDockingManager.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPDockingControlBar.h"
#include "bcgglobals.h"
#include "BCGPTabbedControlBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPSmartDockingManager::CBCGPSmartDockingManager () :
	m_bStarted (FALSE),
	m_bCreated (FALSE),
    m_bShown (FALSE),
    m_bCentralGroupShown (FALSE),
    m_nHiliteSideNo (CBCGPSmartDockingMarker::sdNONE),
	m_pwndOwner (NULL),
	m_pDockingWnd (NULL)
{
	ZeroMemory(m_arMarkers, sizeof(m_arMarkers));
}

CBCGPSmartDockingManager::~CBCGPSmartDockingManager ()
{
	Destroy ();
}

void CBCGPSmartDockingManager::Create (CWnd * pwndOwner,
		CRuntimeClass* prtMarker,
		CRuntimeClass* prtCentralGroup)
{
	ASSERT_VALID (pwndOwner);

	if (prtMarker == NULL)
	{
		prtMarker = RUNTIME_CLASS(CBCGPSmartDockingMarker);
	}

	if (prtCentralGroup == NULL)
	{
		prtCentralGroup = RUNTIME_CLASS(CBCGPSDCentralGroup);
	}

	ASSERT(prtMarker != NULL && prtMarker->IsDerivedFrom (RUNTIME_CLASS (CBCGPSmartDockingMarker)));
	ASSERT(prtCentralGroup != NULL && prtCentralGroup->IsDerivedFrom (RUNTIME_CLASS (CBCGPSDCentralGroup)));

	Destroy ();

	m_pCentralGroup = (CBCGPSDCentralGroup*)(prtCentralGroup->m_pfnCreateObject());
	ASSERT_VALID (m_pCentralGroup);

	CBCGPSmartDockingMarker::SDMarkerPlace i;
	for (i = CBCGPSmartDockingMarker::sdLEFT; i <= CBCGPSmartDockingMarker::sdBOTTOM; ++reinterpret_cast<int&>(i))
	{
		m_arMarkers[i] = (CBCGPSmartDockingMarker*)(prtMarker->m_pfnCreateObject());
		ASSERT_VALID (m_arMarkers [i]);

		m_arMarkers[i]->Create (i, pwndOwner);
	}

	m_pCentralGroup->Create (pwndOwner);

	for (i = CBCGPSmartDockingMarker::sdCLEFT; i <= CBCGPSmartDockingMarker::sdCMIDDLE; ++reinterpret_cast<int&>(i))
	{
		m_arMarkers[i] = m_pCentralGroup->GetMarker (i);
	}

	m_pwndOwner = pwndOwner;

    m_wndPlaceMarker.Create (m_pwndOwner);

	m_bCreated = TRUE;
}

void CBCGPSmartDockingManager::Destroy ()
{
	if (!m_bCreated)
	{
		return;
	}

	Stop ();

	CBCGPSmartDockingMarker::SDMarkerPlace i;
	for (i = CBCGPSmartDockingMarker::sdLEFT; i <= CBCGPSmartDockingMarker::sdBOTTOM; ++reinterpret_cast<int&>(i))
    {
		delete m_arMarkers[i];
		m_arMarkers[i] = NULL;
	}

	m_pCentralGroup->Destroy ();
	delete m_pCentralGroup;
	m_pCentralGroup = NULL;

	m_bCreated = FALSE;
}

void CBCGPSmartDockingManager::Start (CWnd* pDockingWnd)
{
	if (!m_bCreated)
	{
		return;
	}
	
	if (m_bStarted)
	{
		return;
	}

	ASSERT_VALID (pDockingWnd);

	m_pDockingWnd = pDockingWnd;

	m_wndPlaceMarker.SetDockingWnd (m_pDockingWnd);

    m_nHiliteSideNo = CBCGPSmartDockingMarker::sdNONE;

	m_dwEnabledAlignment = CBRS_ALIGN_ANY;
	if (m_pDockingWnd != NULL)
	{
		CBCGPMiniFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, m_pDockingWnd);
		if (pMiniFrame != NULL)
		{
			CBCGPDockingControlBar* pFisrtBar = 
				DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pMiniFrame->GetFirstVisibleBar ());
			if (pFisrtBar != NULL)
			{
				m_dwEnabledAlignment = pFisrtBar->GetEnabledAlignment ();
			}
		}
	}


	CBCGPSmartDockingMarker::SDMarkerPlace i;
	for (i = CBCGPSmartDockingMarker::sdLEFT; i <= CBCGPSmartDockingMarker::sdBOTTOM; ++reinterpret_cast<int&>(i))
    {
		m_arMarkers[i]->AdjustPos (&m_rcOuter);
		if (((m_dwEnabledAlignment & CBRS_ALIGN_LEFT) != 0) && i == CBCGPSmartDockingMarker::sdLEFT ||
			((m_dwEnabledAlignment & CBRS_ALIGN_RIGHT) != 0) && i == CBCGPSmartDockingMarker::sdRIGHT ||
			((m_dwEnabledAlignment & CBRS_ALIGN_TOP) != 0) && i == CBCGPSmartDockingMarker::sdTOP ||
			((m_dwEnabledAlignment & CBRS_ALIGN_BOTTOM) != 0) && i == CBCGPSmartDockingMarker::sdBOTTOM)
		{
			m_arMarkers[i]->Show (TRUE);
		}
	}

    m_bShown = TRUE;
    m_bCentralGroupShown = FALSE;

	m_bStarted = TRUE;
}

void CBCGPSmartDockingManager::Stop ()
{
	if (!m_bStarted)
	{
		return;
	}

    m_nHiliteSideNo = CBCGPSmartDockingMarker::sdNONE;

	m_wndPlaceMarker.Hide ();

	CBCGPSmartDockingMarker::SDMarkerPlace i;
	for (i = CBCGPSmartDockingMarker::sdLEFT; i <= CBCGPSmartDockingMarker::sdBOTTOM; ++reinterpret_cast<int&>(i))
    {
		m_arMarkers[i]->Show (FALSE);
	}

	m_pCentralGroup->Show (FALSE);

	m_bStarted = FALSE;
}

void CBCGPSmartDockingManager::Show (BOOL bShow)
{
	if (m_bStarted && m_bShown != bShow)
	{
        m_bShown = bShow;

        if (m_bCentralGroupShown)
        {
            m_pCentralGroup->Show (bShow);
        }
	    CBCGPSmartDockingMarker::SDMarkerPlace i;
	    for (i = CBCGPSmartDockingMarker::sdLEFT; i <= CBCGPSmartDockingMarker::sdBOTTOM; ++reinterpret_cast<int&>(i))
        {
			if (((m_dwEnabledAlignment & CBRS_ALIGN_LEFT) != 0) && i == CBCGPSmartDockingMarker::sdLEFT ||
				((m_dwEnabledAlignment & CBRS_ALIGN_RIGHT) != 0) && i == CBCGPSmartDockingMarker::sdRIGHT ||
				((m_dwEnabledAlignment & CBRS_ALIGN_TOP) != 0) && i == CBCGPSmartDockingMarker::sdTOP ||
				((m_dwEnabledAlignment & CBRS_ALIGN_BOTTOM) != 0) && i == CBCGPSmartDockingMarker::sdBOTTOM)
			{
				m_arMarkers[i]->Show (bShow);
			}
	    }

		if (!bShow && !m_wndPlaceMarker.m_bTabbed)
        {
            m_wndPlaceMarker.Hide ();
        }
    }
}

void CBCGPSmartDockingManager::OnMouseMove (CPoint point)
{
	if (m_bStarted)
	{
        m_nHiliteSideNo = CBCGPSmartDockingMarker::sdNONE;

		BOOL bFound = FALSE;
        CBCGPSmartDockingMarker::SDMarkerPlace i;
        CBCGPSmartDockingMarker::SDMarkerPlace first =
            m_pCentralGroup->m_bMiddleIsOn ?
            CBCGPSmartDockingMarker::sdCMIDDLE :
            CBCGPSmartDockingMarker::sdCBOTTOM;

		for (i = first; i >= CBCGPSmartDockingMarker::sdLEFT; --reinterpret_cast<int&>(i))	// from top z-position
		{
			if (!bFound && (m_arMarkers[i] != NULL) && (m_arMarkers[i]->IsPtIn(point)))
			{
				bFound = TRUE;
				m_arMarkers[i]->Highlight ();
                m_nHiliteSideNo = i;
			}
			else
			{
				if (m_arMarkers[i] != NULL)
				{
					m_arMarkers[i]->Highlight (FALSE);
				}
			}
		}
	}
}

void CBCGPSmartDockingManager::OnPosChange ()
{
	if (m_bStarted)
	{
		RECT rcOwner;

		m_pwndOwner->GetClientRect (&rcOwner);
		m_pwndOwner->ClientToScreen (&rcOwner);
		
	    CBCGPSmartDockingMarker::SDMarkerPlace i;
	    for (i = CBCGPSmartDockingMarker::sdLEFT; i <= CBCGPSmartDockingMarker::sdBOTTOM; ++reinterpret_cast<int&>(i))
		{
			m_arMarkers[i]->AdjustPos (&rcOwner);
		}

		m_pCentralGroup->AdjustPos (&rcOwner, -1);
	}
}

void CBCGPSmartDockingManager::SetOuterRect (CRect rcOuter)
{
    m_rcOuter = rcOuter;

    m_pwndOwner->ClientToScreen (&m_rcOuter);

    if (m_bStarted)
    {
	    CBCGPSmartDockingMarker::SDMarkerPlace i;
	    for (i = CBCGPSmartDockingMarker::sdLEFT; i <= CBCGPSmartDockingMarker::sdBOTTOM; ++reinterpret_cast<int&>(i))
        {
		    m_arMarkers[i]->AdjustPos (&m_rcOuter);
		    m_arMarkers[i]->Show (TRUE);
	    }

	    m_pCentralGroup->AdjustPos (&m_rcOuter, -1);
    }
}

void CBCGPSmartDockingManager::ShowPlaceAt (CRect rect)
{
    if (!m_bStarted || !m_bShown)
    {
        return;
    }

    if (m_nHiliteSideNo != CBCGPSmartDockingMarker::sdNONE)
    {
		m_wndPlaceMarker.ShowAt (rect);
	}
}

void CBCGPSmartDockingManager::HidePlace ()
{
    if (m_bStarted)
    {
		m_wndPlaceMarker.Hide ();
	}
}

void CBCGPSmartDockingManager::ShowTabbedPlaceAt (CRect rect, int nTabXOffset,
                            int nTabWidth, int nTabHeight)
{
    if (m_bStarted)
    {
		CRect rectTab;
		if (CBCGPTabbedControlBar::m_bTabsAlwaysTop)
		{
			rectTab.SetRect (CPoint (nTabXOffset, rect.top - nTabHeight),
							 CPoint  (nTabXOffset + nTabWidth, rect.top));

		}
		else
		{
			rectTab.SetRect (CPoint (nTabXOffset, rect.Height ()),
							 CPoint  (nTabXOffset + nTabWidth, rect.Height () + nTabHeight));
		}	

		m_wndPlaceMarker.ShowTabbedAt (rect, rectTab);
	}
}

void CBCGPSmartDockingManager::MoveCentralGroup (CRect rect, int nMiddleIsOn, 
												 DWORD dwEnabledAlignment)
{
	if (m_bStarted && m_pCentralGroup != NULL)
	{
		CRect rectGroup;
		m_pCentralGroup->GetWindowRect (rectGroup);
		if (rectGroup == rect)
		{
			return;
		}
		
		m_pCentralGroup->ShowMarker (CBCGPSmartDockingMarker::sdCLEFT, 
								 (dwEnabledAlignment & CBRS_ALIGN_LEFT) != 0,
								 TRUE);
		m_pCentralGroup->ShowMarker (CBCGPSmartDockingMarker::sdCTOP, 
									 (dwEnabledAlignment & CBRS_ALIGN_TOP) != 0,
									 TRUE);
		m_pCentralGroup->ShowMarker (CBCGPSmartDockingMarker::sdCRIGHT, 
									 (dwEnabledAlignment & CBRS_ALIGN_RIGHT) != 0,
									 TRUE);
		m_pCentralGroup->ShowMarker (CBCGPSmartDockingMarker::sdCBOTTOM, 
									 (dwEnabledAlignment & CBRS_ALIGN_BOTTOM) != 0,
									 TRUE);
		
		
        if (m_pCentralGroup->AdjustPos (rect, nMiddleIsOn))
        {
            m_nHiliteSideNo = CBCGPSmartDockingMarker::sdNONE;
        }
    }
}

void CBCGPSmartDockingManager::ShowCentralGroup (BOOL bShow, DWORD dwEnabledAlignment)
{
    if (m_bStarted && m_pCentralGroup != NULL && m_bShown && m_bCentralGroupShown != bShow)
    {
		m_pCentralGroup->ShowMarker (CBCGPSmartDockingMarker::sdCLEFT, 
								 (dwEnabledAlignment & CBRS_ALIGN_LEFT) != 0,
								 TRUE);
		m_pCentralGroup->ShowMarker (CBCGPSmartDockingMarker::sdCTOP, 
									 (dwEnabledAlignment & CBRS_ALIGN_TOP) != 0,
									 TRUE);
		m_pCentralGroup->ShowMarker (CBCGPSmartDockingMarker::sdCRIGHT, 
									 (dwEnabledAlignment & CBRS_ALIGN_RIGHT) != 0,
									 TRUE);
		m_pCentralGroup->ShowMarker (CBCGPSmartDockingMarker::sdCBOTTOM, 
									 (dwEnabledAlignment & CBRS_ALIGN_BOTTOM) != 0,
									 TRUE);
				
        m_pCentralGroup->Show (bShow);
    }
    m_bCentralGroupShown = bShow;
}

void CBCGPSmartDockingManager::CauseCancelMode ()
{
    if (!m_bStarted)
    {
        return;
    }

	ASSERT_VALID (m_pDockingWnd);

	m_pDockingWnd->SendMessage (WM_CANCELMODE);
}
