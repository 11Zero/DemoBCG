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
// BCGPGlobalUtils.cpp: implementation of the CBCGPGlobalUtils class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPGlobalUtils.h"

#ifndef _BCGSUITE_

#include "BCGPDockManager.h"
#include "BCGPBarContainerManager.h"
#include "BCGPDockingControlBar.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPMultiMiniFrameWnd.h"
#include "BCGPBaseTabbedBar.h"

#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPOleIPFrameWnd.h"
#include "BCGPOleDocIPFrameWnd.h"
#include "BCGPMDIChildWnd.h"
#include "BCGPOleCntrFrameWnd.h"
#include "BCGPURLLinkButton.h"
#include "BCGCBProVer.h"
#else
#include "BCGSuiteVer.h"
#endif

#include "BCGPDialog.h"
#include "BCGPCalendarBar.h"
#include "BCGPImageProcessing.h"
#include "BCGPLocalResource.h"
#include "bcgprores.h"

#pragma warning (disable : 4706)

#include "multimon.h"

#pragma warning (default : 4706)

#ifndef SM_CXPADDEDBORDER
	#define SM_CXPADDEDBORDER 92
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CBCGPGlobalUtils globalUtils;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPGlobalUtils::CBCGPGlobalUtils()
{
	m_bDialogApp = FALSE;
	m_bIsDragging = FALSE;
	m_bUseMiniFrameParent = FALSE;
	m_bIsAdjustLayout = FALSE;
}

CBCGPGlobalUtils::~CBCGPGlobalUtils()
{

}

#ifndef _BCGSUITE_

//------------------------------------------------------------------------//
BOOL CBCGPGlobalUtils::CheckAlignment (CPoint point, CBCGPBaseControlBar* pBar, int nSencitivity, 
                                       const CBCGPDockManager* pDockManager,
									   BOOL bOuterEdge, DWORD& dwAlignment, 
									   DWORD dwEnabledDockBars, LPCRECT lpRectBounds) const
{
    BOOL bSmartDocking = FALSE;
    CBCGPSmartDockingMarker::SDMarkerPlace nHilitedSide = CBCGPSmartDockingMarker::sdNONE;
	
	if (pDockManager == NULL && pBar != NULL)
    {
        pDockManager = globalUtils.GetDockManager (pBar->GetParent());
    }
	
	if (pDockManager != NULL)
	{
        CBCGPSmartDockingManager* pSDManager = pDockManager->GetSDManagerPermanent ();
        if (pSDManager != NULL && pSDManager->IsStarted ())
        {
            bSmartDocking = TRUE;
            nHilitedSide = pSDManager->GetHilitedMarkerNo ();
        }
	}
	
	CRect rectBounds;
	if (pBar != NULL)
	{
		pBar->GetWindowRect (rectBounds);
		
	}
	else if (lpRectBounds != NULL)
	{
		rectBounds = *lpRectBounds;
	}
	else
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	int nCaptionHeight = 0;
	int nTabAreaTopHeight = 0; 
	int nTabAreaBottomHeight = 0;
	
	CBCGPDockingControlBar* pDockingBar = 
		DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pBar);
	
	if (pDockingBar != NULL)
	{
		nCaptionHeight = pDockingBar->GetCaptionHeight ();
		
		CRect rectTabAreaTop;
		CRect rectTabAreaBottom;
		pDockingBar->GetTabArea (rectTabAreaTop, rectTabAreaBottom);
		nTabAreaTopHeight = rectTabAreaTop.Height ();
		nTabAreaBottomHeight = rectTabAreaBottom.Height ();
	}
	
	// build rect for top area
	if (bOuterEdge)
	{
        if (bSmartDocking)
        {
            switch (nHilitedSide)
            {
            case CBCGPSmartDockingMarker::sdLEFT:
				dwAlignment = CBRS_ALIGN_LEFT;
				return TRUE;
            case CBCGPSmartDockingMarker::sdRIGHT:
				dwAlignment = CBRS_ALIGN_RIGHT;
				return TRUE;
            case CBCGPSmartDockingMarker::sdTOP:
				dwAlignment = CBRS_ALIGN_TOP;
				return TRUE;
            case CBCGPSmartDockingMarker::sdBOTTOM:
				dwAlignment = CBRS_ALIGN_BOTTOM;
				return TRUE;
            }
        }
		else
        {
			CRect rectToCheck (rectBounds.left - nSencitivity, rectBounds.top - nSencitivity, 
				rectBounds.right + nSencitivity, rectBounds.top);
			if (rectToCheck.PtInRect (point) && dwEnabledDockBars & CBRS_ALIGN_TOP)
			{
				dwAlignment = CBRS_ALIGN_TOP;
				return TRUE;
			}
			
			// build rect for left area
			rectToCheck.right = rectBounds.left;
			rectToCheck.bottom = rectBounds.bottom + nSencitivity;
			
			if (rectToCheck.PtInRect (point) && dwEnabledDockBars & CBRS_ALIGN_LEFT)
			{
				dwAlignment = CBRS_ALIGN_LEFT;
				return TRUE;
			}
			
			// build rect for bottom area
			rectToCheck.left = rectBounds.left - nSencitivity;
			rectToCheck.top = rectBounds.bottom;
			rectToCheck.right = rectBounds.right + nSencitivity;
			rectToCheck.bottom = rectBounds.bottom + nSencitivity;
			
			if (rectToCheck.PtInRect (point) && dwEnabledDockBars & CBRS_ALIGN_BOTTOM)
			{
				dwAlignment = CBRS_ALIGN_BOTTOM;
				return TRUE;
			}
			
			// build rect for right area
			rectToCheck.left = rectBounds.right;
			rectToCheck.top = rectBounds.top - nSencitivity;
			
			if (rectToCheck.PtInRect (point) && dwEnabledDockBars & CBRS_ALIGN_RIGHT)
			{
				dwAlignment = CBRS_ALIGN_RIGHT;
				return TRUE;
			}
        }
	}
	else
	{
        if (bSmartDocking)
        {
            switch (nHilitedSide)
            {
            case CBCGPSmartDockingMarker::sdCLEFT:
				dwAlignment = CBRS_ALIGN_LEFT;
				return TRUE;
            case CBCGPSmartDockingMarker::sdCRIGHT:
				dwAlignment = CBRS_ALIGN_RIGHT;
				return TRUE;
            case CBCGPSmartDockingMarker::sdCTOP:
				dwAlignment = CBRS_ALIGN_TOP;
				return TRUE;
            case CBCGPSmartDockingMarker::sdCBOTTOM:
				dwAlignment = CBRS_ALIGN_BOTTOM;
				return TRUE;
            }
        }
		else
        {
#ifdef __BOUNDS_FIX__
			CRect rectToCheck (rectBounds.left, rectBounds.top, 
				rectBounds.right, 
				rectBounds.top + nSencitivity + nCaptionHeight);
			if (rectToCheck.PtInRect (point) && dwEnabledDockBars & CBRS_ALIGN_TOP)
			{
				dwAlignment = CBRS_ALIGN_TOP;
				return TRUE;
			}
			
			
			// build rect for left area
			rectToCheck.right = rectBounds.left + nSencitivity;
			rectToCheck.bottom = rectBounds.bottom + nSencitivity;
			
			if (rectToCheck.PtInRect (point) && dwEnabledDockBars & CBRS_ALIGN_LEFT)
			{
				dwAlignment = CBRS_ALIGN_LEFT;
				return TRUE;
			}
			
			// build rect for bottom area
			rectToCheck.left = rectBounds.left;
			rectToCheck.top = rectBounds.bottom - nSencitivity - nTabAreaBottomHeight;
			rectToCheck.right = rectBounds.right;
			rectToCheck.bottom = rectBounds.bottom;
			
			if (rectToCheck.PtInRect (point) && dwEnabledDockBars & CBRS_ALIGN_BOTTOM)
			{
				dwAlignment = CBRS_ALIGN_BOTTOM;
				return TRUE;
			}
			
			// build rect for right area
			rectToCheck.left = rectBounds.right - nSencitivity;
			rectToCheck.top = rectBounds.top - nSencitivity;
			
			if (rectToCheck.PtInRect (point) && dwEnabledDockBars & CBRS_ALIGN_RIGHT)
			{
				dwAlignment = CBRS_ALIGN_RIGHT;
				return TRUE;
			}
#else
			
			// build rect for top area
			CRect rectToCheck (rectBounds.left - nSencitivity, rectBounds.top - nSencitivity, 
				rectBounds.right + nSencitivity, 
				rectBounds.top + nSencitivity + nCaptionHeight);
			if (rectToCheck.PtInRect (point) && dwEnabledDockBars & CBRS_ALIGN_TOP)
			{
				dwAlignment = CBRS_ALIGN_TOP;
				return TRUE;
			}
			
			
			// build rect for left area
			rectToCheck.right = rectBounds.left + nSencitivity;
			rectToCheck.bottom = rectBounds.bottom + nSencitivity;
			
			if (rectToCheck.PtInRect (point) && dwEnabledDockBars & CBRS_ALIGN_LEFT)
			{
				dwAlignment = CBRS_ALIGN_LEFT;
				return TRUE;
			}
			
			// build rect for bottom area
			rectToCheck.left = rectBounds.left - nSencitivity;
			rectToCheck.top = rectBounds.bottom - nSencitivity - nTabAreaBottomHeight;
			rectToCheck.right = rectBounds.right + nSencitivity;
			rectToCheck.bottom = rectBounds.bottom + nSencitivity;
			
			if (rectToCheck.PtInRect (point) && dwEnabledDockBars & CBRS_ALIGN_BOTTOM)
			{
				dwAlignment = CBRS_ALIGN_BOTTOM;
				return TRUE;
			}
			
			// build rect for right area
			rectToCheck.left = rectBounds.right - nSencitivity;
			rectToCheck.top = rectBounds.top - nSencitivity;
			
			if (rectToCheck.PtInRect (point) && dwEnabledDockBars & CBRS_ALIGN_RIGHT)
			{
				dwAlignment = CBRS_ALIGN_RIGHT;
				return TRUE;
			}
#endif		
        }
	}
	
	return FALSE;
}
//------------------------------------------------------------------------//
CBCGPDockManager* CBCGPGlobalUtils::GetDockManager (CWnd* pWnd)
{
	if (pWnd == NULL)
	{
		return NULL;
	}

	ASSERT_VALID (pWnd);

	if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPFrameWnd)))
	{
		return ((CBCGPFrameWnd*) pWnd)->GetDockManager ();
	}
	else if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPMDIFrameWnd)))
	{
		return ((CBCGPMDIFrameWnd*) pWnd)->GetDockManager ();
	}
	else if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPOleIPFrameWnd)))
	{
		return ((CBCGPOleIPFrameWnd*) pWnd)->GetDockManager ();
	}
	else if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPOleDocIPFrameWnd)))
	{
		return ((CBCGPOleDocIPFrameWnd*) pWnd)->GetDockManager ();
	}
	else if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPMDIChildWnd)))
	{
		return ((CBCGPMDIChildWnd*) pWnd)->GetDockManager ();
	}
	else if (pWnd->IsKindOf (RUNTIME_CLASS (CDialog)) ||
		pWnd->IsKindOf (RUNTIME_CLASS (CPropertySheet)))
	{
		if (pWnd->GetSafeHwnd() == AfxGetMainWnd()->GetSafeHwnd())
		{
			m_bDialogApp = TRUE;
		}
	}
	else if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPOleCntrFrameWnd)))
	{
		return ((CBCGPOleCntrFrameWnd*) pWnd)->GetDockManager ();
	}
    else if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd)))
    {
		CBCGPMiniFrameWnd* pMiniFrameWnd = DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, pWnd);
		ASSERT_VALID (pMiniFrameWnd);

		CBCGPDockManager* pManager = pMiniFrameWnd->GetDockManager ();
        return pManager != NULL ? pManager : GetDockManager (m_bUseMiniFrameParent ? pMiniFrameWnd->GetParent() : pWnd->GetParent ());
    }

	return NULL;
}
//------------------------------------------------------------------------//
void CBCGPGlobalUtils::FlipRect (CRect& rect, int nDegrees)
{
	CRect rectTmp = rect;
	switch (nDegrees)
	{
	case 90:
		rect.top = rectTmp.left;
		rect.right = rectTmp.top;
		rect.bottom = rectTmp.right;
		rect.left = rectTmp.bottom;
		break;
	case 180:
		rect.top = rectTmp.bottom;
		rect.bottom = rectTmp.top;
		break;
	case 275:
	case -90:
		rect.left = rectTmp.top;
		rect.top = rectTmp.right;
		rect.right = rectTmp.bottom;
		rect.bottom = rectTmp.left;
		break;
	}
}
//------------------------------------------------------------------------//
DWORD CBCGPGlobalUtils::GetOppositeAlignment (DWORD dwAlign)
{
	switch (dwAlign & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_LEFT:
		return CBRS_ALIGN_RIGHT;
	case CBRS_ALIGN_RIGHT:
		return CBRS_ALIGN_LEFT;
	case CBRS_ALIGN_TOP:
		return CBRS_ALIGN_BOTTOM;
	case CBRS_ALIGN_BOTTOM:
		return CBRS_ALIGN_TOP;
	}
	return 0;
}
//------------------------------------------------------------------------//
void CBCGPGlobalUtils::SetNewParent (CObList& lstControlBars, CWnd* pNewParent,
									 BOOL bCheckVisibility)
{
	ASSERT_VALID (pNewParent);
	for (POSITION pos = lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pBar = (CBCGPBaseControlBar*) lstControlBars.GetNext (pos);

		if (bCheckVisibility && !pBar->IsBarVisible ())
		{
			continue;
		}
		if (!pBar->IsKindOf (RUNTIME_CLASS (CBCGPSlider)))
		{
			pBar->ShowWindow (SW_HIDE);
			pBar->SetParent (pNewParent);
			CRect rectWnd;
			pBar->GetWindowRect (rectWnd);
			pNewParent->ScreenToClient (rectWnd);

			pBar->SetWindowPos (NULL, -rectWnd.Width (), -rectWnd.Height (), 
									  100, 100, SWP_NOZORDER | SWP_NOSIZE  | SWP_NOACTIVATE);
			pBar->ShowWindow (SW_SHOW);
		}
		else
		{
			pBar->SetParent (pNewParent);
		}
	}
}
//------------------------------------------------------------------------//
void CBCGPGlobalUtils::CalcExpectedDockedRect (CBCGPBarContainerManager& barContainerManager, 
												CWnd* pWndToDock, CPoint ptMouse, 
												CRect& rectResult, BOOL& bDrawTab, 
												CBCGPDockingControlBar** ppTargetBar)
{
	ASSERT (ppTargetBar != NULL);

	DWORD dwAlignment = CBRS_ALIGN_ANY;
	BOOL bTabArea = FALSE;
	BOOL bCaption = FALSE;
	bDrawTab = FALSE;
	*ppTargetBar = NULL;

	rectResult.SetRectEmpty ();

	if (GetKeyState (VK_CONTROL) < 0)
	{
		return;
	}

	if (!GetCBAndAlignFromPoint (barContainerManager, ptMouse, 
								 ppTargetBar, dwAlignment, bTabArea, bCaption) || 
		*ppTargetBar == NULL)
	{
		return;
	}

	CBCGPControlBar* pBar = NULL;
	
	if (pWndToDock->IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd)))
	{
		CBCGPMiniFrameWnd* pMiniFrame = 
			DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, pWndToDock);
		ASSERT_VALID (pMiniFrame);
		pBar = DYNAMIC_DOWNCAST (CBCGPControlBar, pMiniFrame->GetFirstVisibleBar ());
	}
	else
	{
		pBar = DYNAMIC_DOWNCAST (CBCGPControlBar, pWndToDock);
	}

	if (*ppTargetBar != NULL)
	{
		DWORD dwTargetEnabledAlign = (*ppTargetBar)->GetEnabledAlignment ();
		DWORD dwTargetCurrentAlign = (*ppTargetBar)->GetCurrentAlignment ();
		BOOL bTargetBarIsFloating = ((*ppTargetBar)->GetParentMiniFrame () != NULL);

		if (pBar != NULL)
		{
			if (pBar->GetEnabledAlignment () != dwTargetEnabledAlign && bTargetBarIsFloating ||
				(pBar->GetEnabledAlignment () & dwTargetCurrentAlign) == 0 && !bTargetBarIsFloating)
			{
				return;
			}
		}
	}

	if (bTabArea || bCaption)
	{
		// can't make tab on miniframe
		bDrawTab = ((*ppTargetBar) != NULL);

		if (bDrawTab)
		{
			bDrawTab = (*ppTargetBar)->CanBeAttached () && CanBeAttached (pWndToDock) && 
				pBar != NULL && ((*ppTargetBar)->GetEnabledAlignment () == pBar->GetEnabledAlignment ());
		}

		if (!bDrawTab)
		{
			return;
		}

	}

	if ((*ppTargetBar) != NULL && (*ppTargetBar)->GetParentMiniFrame () != NULL && 
		!IsWndCanFloatMulti (pWndToDock))
	{
		bDrawTab = FALSE;
		return;
	}

	if ((*ppTargetBar) != NULL && 
		pWndToDock->IsKindOf (RUNTIME_CLASS (CBCGPBaseControlBar)) && 
		!(*ppTargetBar)->CanAcceptBar ((CBCGPBaseControlBar*) pWndToDock))
	{
		bDrawTab = FALSE;
		return;
	}

	CRect rectOriginal; 
	(*ppTargetBar)->GetWindowRect (rectOriginal);

	if ((*ppTargetBar) == pWndToDock ||
		pWndToDock->IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd)) && 
		(*ppTargetBar)->GetParentMiniFrame () == pWndToDock)
	{
		bDrawTab = FALSE;
		return;
	}
	
	CRect rectInserted;
	CRect rectSlider;
	DWORD dwSliderStyle;
	CSize sizeMinOriginal (0, 0);
	CSize sizeMinInserted (0, 0);

	
	pWndToDock->GetWindowRect (rectInserted);

	if ((dwAlignment & pBar->GetEnabledAlignment ()) != 0 ||
		CBCGPDockManager::m_bIgnoreEnabledAlignment)
	{
		barContainerManager.CalcRects (rectOriginal, rectInserted, rectSlider, dwSliderStyle, 
										 dwAlignment, sizeMinOriginal, sizeMinInserted);
		rectResult = rectInserted;
	}
	
}
//------------------------------------------------------------------------//
BOOL CBCGPGlobalUtils::CanBeAttached (CWnd* pWnd) const
{
	ASSERT_VALID (pWnd);

	if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd)))
	{
		return ((CBCGPMiniFrameWnd*) pWnd)->CanBeAttached ();
	}

	if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPControlBar)))
	{
		return ((CBCGPControlBar*) pWnd)->CanBeAttached ();
	}

	return FALSE;

}
//------------------------------------------------------------------------//
BOOL CBCGPGlobalUtils::IsWndCanFloatMulti (CWnd* pWnd) const
{
	CBCGPControlBar* pBar = NULL;

	CBCGPMiniFrameWnd* pMiniFrame = 
		DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, pWnd);
	
	if (pMiniFrame != NULL)
	{
		pBar = DYNAMIC_DOWNCAST (CBCGPControlBar, pMiniFrame->GetControlBar ());
	}
	else
	{
		pBar = DYNAMIC_DOWNCAST (CBCGPControlBar, pWnd);

	}

	if (pBar == NULL)
	{
		return FALSE;
	}

	if (pBar->IsTabbed ())
	{
		CWnd* pParentMiniFrame = pBar->GetParentMiniFrame ();
		// tabbed bar that is floating in multi miniframe
		if (pParentMiniFrame != NULL && pParentMiniFrame->IsKindOf (RUNTIME_CLASS (CBCGPMultiMiniFrameWnd)))
		{
			return TRUE;
		}
	}

	
	return ((pBar->GetBarStyle () & CBRS_FLOAT_MULTI) != 0);
}
//------------------------------------------------------------------------//
BOOL CBCGPGlobalUtils::GetCBAndAlignFromPoint (CBCGPBarContainerManager& barContainerManager, 
													 CPoint pt, 
												     CBCGPDockingControlBar** ppTargetControlBar, 
												     DWORD& dwAlignment, 
													 BOOL& bTabArea, 
													 BOOL& bCaption)
{
	ASSERT (ppTargetControlBar != NULL);
	*ppTargetControlBar = NULL;

	BOOL bOuterEdge = FALSE;

	// if the mouse is over a miniframe's caption and this miniframe has only one
	// visible docking control bar, we need to draw a tab
	bCaption = barContainerManager.CheckForMiniFrameAndCaption (pt, ppTargetControlBar);
	if (bCaption)
	{
		return TRUE;
	}


	*ppTargetControlBar = 
		barContainerManager.ControlBarFromPoint (pt, CBCGPDockManager::m_nDockSencitivity, 
													TRUE, bTabArea, bCaption);

	if ((bCaption || bTabArea) && *ppTargetControlBar != NULL) 
	{
		return TRUE;
	}

	if (*ppTargetControlBar == NULL)
	{
		barContainerManager.ControlBarFromPoint (pt, CBCGPDockManager::m_nDockSencitivity, 
														FALSE, bTabArea, bCaption);
		// the exact bar was not found - it means the docked frame at the outer edge
		bOuterEdge = TRUE;
		return TRUE;
	}

	if (*ppTargetControlBar != NULL)
	{
		if (!globalUtils.CheckAlignment (pt, *ppTargetControlBar,
										CBCGPDockManager::m_nDockSencitivity, NULL,
										bOuterEdge, dwAlignment))
		{
			// unable for some reason to determine alignment
			*ppTargetControlBar = NULL;
		}
	}

	return TRUE;
}
//------------------------------------------------------------------------//
void CBCGPGlobalUtils::AdjustRectToWorkArea (CRect& rect, CRect* pRectDelta)
{
	CPoint ptStart;

	if (m_bIsDragging)
	{
		::GetCursorPos (&ptStart);
	}
	else
	{
		ptStart = rect.TopLeft ();
	}

	CRect rectScreen;
	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);
	if (GetMonitorInfo (
		MonitorFromPoint (ptStart, MONITOR_DEFAULTTONEAREST),
		&mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	int nDelta = pRectDelta != NULL ? pRectDelta->left : 0;

	if (rect.right <= rectScreen.left + nDelta)
	{
		rect.OffsetRect (rectScreen.left - rect.right + nDelta, 0);
	}

	nDelta = pRectDelta != NULL ? pRectDelta->right : 0;
	if (rect.left >= rectScreen.right - nDelta)
	{
		rect.OffsetRect (rectScreen.right - rect.left - nDelta, 0);
	}

	nDelta = pRectDelta != NULL ? pRectDelta->bottom : 0;
	if (rect.top >= rectScreen.bottom - nDelta)
	{
		rect.OffsetRect (0, rectScreen.bottom - rect.top - nDelta);
	}

	nDelta = pRectDelta != NULL ? pRectDelta->top : 0;
	if (rect.bottom < rectScreen.top + nDelta)
	{
		rect.OffsetRect (0, rectScreen.top - rect.bottom + nDelta);
	}
}
//------------------------------------------------------------------------//
void CBCGPGlobalUtils::ForceAdjustLayout (CBCGPDockManager* pDockManager, BOOL bForce,
										  BOOL bForceInvisible, BOOL bForceNcArea)
{
	if (pDockManager != NULL && 
		(CBCGPControlBar::m_bHandleMinSize || bForce))
	{
		CWnd* pDockSite = pDockManager->GetDockSite ();

		if (pDockSite == NULL)
		{
			return;
		}
			
		if (!pDockSite->IsWindowVisible () && !bForceInvisible)
		{
			return;
		}

		if (pDockSite->IsKindOf(RUNTIME_CLASS(CBCGPFrameWnd)) || pDockSite->IsKindOf(RUNTIME_CLASS(CBCGPMDIFrameWnd)))
		{
			m_bIsAdjustLayout = (pDockSite->GetStyle() & WS_MAXIMIZE) == WS_MAXIMIZE;
		}

		CRect rectWnd;
		pDockSite->SetRedraw (FALSE);
		pDockSite->GetWindowRect (rectWnd);
		pDockSite->SetWindowPos (NULL, -1, -1, 
			rectWnd.Width () + 1, rectWnd.Height () + 1, 
			SWP_NOZORDER |  SWP_NOMOVE | SWP_NOACTIVATE);
		pDockSite->SetWindowPos (NULL, -1, -1, 
			rectWnd.Width (), rectWnd.Height (), 
			SWP_NOZORDER |  SWP_NOMOVE  | SWP_NOACTIVATE);
		pDockSite->SetRedraw (TRUE);

		if (bForceNcArea)
		{
			pDockSite->SetWindowPos(NULL, 0, 0, 0, 0, 
				SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}

		m_bIsAdjustLayout = FALSE;

		pDockSite->RedrawWindow (NULL, NULL, 
			RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}
}

#endif

//------------------------------------------------------------------------//
BOOL CBCGPGlobalUtils::StringFromCy(CString& str, CY& cy)
{
	VARIANTARG varCy;
	VARIANTARG varBstr;
	AfxVariantInit(&varCy);
	AfxVariantInit(&varBstr);
	V_VT(&varCy) = VT_CY;
	V_CY(&varCy) = cy;
	if (FAILED(VariantChangeType(&varBstr, &varCy, 0, VT_BSTR)))
	{
		VariantClear(&varCy);
		VariantClear(&varBstr);
		return FALSE;
	}
	str = V_BSTR(&varBstr);
	VariantClear(&varCy);
	VariantClear(&varBstr);
	return TRUE;
}
//------------------------------------------------------------------------//
BOOL CBCGPGlobalUtils::CyFromString(CY& cy, LPCTSTR psz)
{
	USES_CONVERSION;

	if (psz == NULL || _tcslen (psz) == 0)
	{
		psz = _T("0");
	}

	VARIANTARG varBstr;
	VARIANTARG varCy;
	AfxVariantInit(&varBstr);
	AfxVariantInit(&varCy);
	V_VT(&varBstr) = VT_BSTR;
	V_BSTR(&varBstr) = SysAllocString(T2COLE(psz));
	if (FAILED(VariantChangeType(&varCy, &varBstr, 0, VT_CY)))
	{
		VariantClear(&varBstr);
		VariantClear(&varCy);
		return FALSE;
	}
	cy = V_CY(&varCy);
	VariantClear(&varBstr);
	VariantClear(&varCy);
	return TRUE;
}
//------------------------------------------------------------------------//
BOOL CBCGPGlobalUtils::StringFromDecimal(CString& str, DECIMAL& decimal)
{
	VARIANTARG varDecimal;
	VARIANTARG varBstr;
	AfxVariantInit(&varDecimal);
	AfxVariantInit(&varBstr);
	V_VT(&varDecimal) = VT_DECIMAL;
	V_DECIMAL(&varDecimal) = decimal;
	if (FAILED(VariantChangeType(&varBstr, &varDecimal, 0, VT_BSTR)))
	{
		VariantClear(&varDecimal);
		VariantClear(&varBstr);
		return FALSE;
	}
	str = V_BSTR(&varBstr);
	VariantClear(&varDecimal);
	VariantClear(&varBstr);
	return TRUE;
}
//------------------------------------------------------------------------//
BOOL CBCGPGlobalUtils::DecimalFromString(DECIMAL& decimal, LPCTSTR psz)
{
	USES_CONVERSION;

	if (psz == NULL || _tcslen (psz) == 0)
	{
		psz = _T("0");
	}

	VARIANTARG varBstr;
	VARIANTARG varDecimal;
	AfxVariantInit(&varBstr);
	AfxVariantInit(&varDecimal);
	V_VT(&varBstr) = VT_BSTR;
	V_BSTR(&varBstr) = SysAllocString(T2COLE(psz));
	if (FAILED(VariantChangeType(&varDecimal, &varBstr, 0, VT_DECIMAL)))
	{
		VariantClear(&varBstr);
		VariantClear(&varDecimal);
		return FALSE;
	}
	decimal = V_DECIMAL(&varDecimal);
	VariantClear(&varBstr);
	VariantClear(&varDecimal);
	return TRUE;
}
//------------------------------------------------------------------------//

void CBCGPGlobalUtils::StringAddItemQuoted (CString& strOutput, LPCTSTR pszItem, BOOL bInsertBefore, TCHAR charDelimiter, TCHAR charQuote)
{
	CString strItem(pszItem);

	TCHAR special[] = {charDelimiter, charQuote, 0};
	if (strItem.IsEmpty () || strItem.FindOneOf (special) >= 0)
	{
		TCHAR singleQuote[] = {charQuote, 0};
		TCHAR doubleQuote[] = {charQuote, charQuote, 0};
		strItem.Replace (singleQuote, doubleQuote);

		CString strTemp;
		strTemp.Format (_T("%c%s%c"), charQuote, strItem, charQuote);
		strItem = strTemp;
	}

	CString strTemp;
	if (strOutput.IsEmpty ())
	{
		if (bInsertBefore)
		{
			strTemp.Format(_T("%s%s"), strItem, strOutput);
		}
		else
		{
			strTemp.Format(_T("%s%s"), strOutput, strItem);
		}
	}
	else
	{
		if (bInsertBefore)
		{
			strTemp.Format(_T("%s%c%s"), strItem, charDelimiter, strOutput);
		}
		else
		{
			strTemp.Format(_T("%s%c%s"), strOutput, charDelimiter, strItem);
		}
	}

	strOutput = strTemp;
}

CString CBCGPGlobalUtils::StringExtractItemQuoted (const CString& strSource, LPCTSTR pszDelimiters, int& iStart, TCHAR charQuote)
{
	ASSERT (iStart >= 0);
	int iLen = strSource.GetLength ();
	if (iStart >= iLen)
	{
		return CString();
	}
		
	if (pszDelimiters == NULL || *pszDelimiters == 0)
	{
		return strSource.Mid (iStart);
	}

	int iPos = iStart;

	bool bQuoted = (strSource[iStart] == charQuote);
	if (bQuoted)
	{
		iPos ++;
	}

	CString strTokens(pszDelimiters);
	for (; iPos < iLen; ++ iPos)
	{
		if (bQuoted)
		{
			if (strSource[iPos] == charQuote)
			{
				iPos ++;
				if (iPos == iLen) break; // end quote found
				if (strTokens.Find (strSource[iPos]) >= 0) break; // closing quote found
			}
		}
		else
		{
			if (strTokens.Find (strSource[iPos]) >= 0)
			{
				break;
			}
		}
	}

	bool bEndsWithQuote = bQuoted && (strSource[iPos] == charQuote);
	CString strItem = strSource.Mid (iStart, (bEndsWithQuote ? iPos - 1 : iPos) - iStart);
	if (bQuoted)
	{
		TCHAR singleQuote[] = {charQuote, 0};
		TCHAR doubleQuote[] = {charQuote, charQuote, 0};
		strItem.Replace (doubleQuote, singleQuote);
	}

	iStart = min (iPos + 1, iLen);
	return strItem;
}
//------------------------------------------------------------------------//
HICON CBCGPGlobalUtils::GetWndIcon (CWnd* pWnd, BOOL* bDestroyIcon, BOOL bNoDefault)
{
#ifdef _BCGSUITE_
	UNREFERENCED_PARAMETER(bNoDefault);
#endif
	ASSERT_VALID (pWnd);
	
	if (pWnd->GetSafeHwnd () == NULL)
	{
		return NULL;
	}

	if (bDestroyIcon != NULL)
	{
		*bDestroyIcon = FALSE;
	}

	HICON hIcon = pWnd->GetIcon (FALSE);

	if (hIcon == NULL)
	{
		hIcon = pWnd->GetIcon (TRUE);

		if (hIcon != NULL)
		{
			CImageList il;
			il.Create (16, 16, ILC_COLOR32 | ILC_MASK, 0, 1);
			il.Add (hIcon);

			if (il.GetImageCount () == 1)
			{
				hIcon = il.ExtractIcon (0);
				if (bDestroyIcon != NULL)
				{
					*bDestroyIcon = hIcon != NULL;
				}
			}
		}
	}

	if (hIcon == NULL)
	{
		hIcon = (HICON)(LONG_PTR)::GetClassLongPtr(pWnd->GetSafeHwnd (), GCLP_HICONSM);
	}

	if (hIcon == NULL)
	{
		hIcon = (HICON)(LONG_PTR)::GetClassLongPtr(pWnd->GetSafeHwnd (), GCLP_HICON);
	}

#ifndef _BCGSUITE_
	if (hIcon == NULL && !bNoDefault && 
		!pWnd->IsKindOf (RUNTIME_CLASS(CDialog)) && !pWnd->IsKindOf (RUNTIME_CLASS(CPropertySheet)))
	{
		hIcon = globalData.m_hiconApp;
		if (bDestroyIcon != NULL)
		{
			*bDestroyIcon = FALSE;
		}
	}
#endif
	return hIcon;
}

HICON CBCGPGlobalUtils::GrayIcon(HICON hIcon)
{
	if (hIcon == NULL)
	{
		return NULL;
	}

#ifdef _BCGSUITE_
	return ::CopyIcon(hIcon);
#else
	HBITMAP hBmp = BCGPIconToBitmap32 (hIcon);

	BITMAP bmp;
	::GetObject (hBmp, sizeof (BITMAP), (LPVOID) &bmp);

	CBCGPToolBarImages images;
	images.SetImageSize (CSize (bmp.bmWidth, bmp.bmHeight));

	images.AddImage(hBmp, TRUE);
	images.ConvertToGrayScale();
	return images.ExtractIcon(0);

#endif
}

void CBCGPGlobalUtils::EnableWindowShadow(CWnd* pWnd, BOOL bEnable)
{
#ifdef _BCGSUITE_
	UNREFERENCED_PARAMETER(pWnd);
	UNREFERENCED_PARAMETER(bEnable);
#else

	if (!globalData.bIsWindowsVista)
	{
		return;
	}

	if (pWnd->GetSafeHwnd() == NULL)
	{
		return;
	}

	DWORD dwClassStyle = ::GetClassLong(pWnd->GetSafeHwnd(), GCL_STYLE);
	BOOL bHasShadow = (dwClassStyle & CS_DROPSHADOW) != 0;
	BOOL bWasChanged = FALSE;

	if (bEnable)
	{
		if (!bHasShadow)
		{
			::SetClassLong(pWnd->GetSafeHwnd(), GCL_STYLE, dwClassStyle | CS_DROPSHADOW);
			bWasChanged = TRUE;
		}
	}
	else
	{
		if (bHasShadow)
		{
			::SetClassLong(pWnd->GetSafeHwnd(), GCL_STYLE, dwClassStyle & (~CS_DROPSHADOW));
			bWasChanged = TRUE;
		}
	}

	if (bWasChanged && pWnd->IsWindowVisible())
	{
		BOOL bIsZoomed = pWnd->IsZoomed();
		BOOL bIsIconic = pWnd->IsIconic();

		pWnd->ShowWindow(SW_HIDE);
		pWnd->ShowWindow(bIsZoomed ? SW_SHOWMAXIMIZED : bIsIconic ? SW_SHOWMINIMIZED : SW_SHOW);
		pWnd->SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		if (AfxGetMainWnd()->GetSafeHwnd() == pWnd->GetSafeHwnd())
		{
			pWnd->BringWindowToTop();
		}
	}
#endif
}

BOOL CBCGPGlobalUtils::EnableEditCtrlAutoComplete (HWND hwndEdit, BOOL bDirsOnly)
{
#ifndef _BCGSUITE_
	if (globalData.bIsWindows9x || globalData.bIsWindowsNT4)
	{
		return FALSE;
	}
#endif

	typedef HRESULT (CALLBACK* LPFNAUTOCOMPLETE) (HWND ,DWORD);
	enum
	{
		shacf_usetab = 0x00000008,
		shacf_filesys_only = 0x00000010,
		shacf_filesys_dirs = 0x00000020,
		shacf_autosuggest_force_on = 0x10000000
	};

	BOOL bRes = FALSE;

	HINSTANCE hInst = ::LoadLibrary (_T("shlwapi.dll"));
	if(hInst != NULL)
	{
		LPFNAUTOCOMPLETE lpfnAutoComplete = (LPFNAUTOCOMPLETE)::GetProcAddress (hInst, "SHAutoComplete");
		if (lpfnAutoComplete != NULL)
		{
			DWORD dwFlags = shacf_usetab | shacf_autosuggest_force_on;
			dwFlags |= (bDirsOnly) ? shacf_filesys_dirs : shacf_filesys_only;
			
			if (SUCCEEDED(lpfnAutoComplete (hwndEdit, dwFlags)))
			{
				bRes = TRUE;
			}
		}

		::FreeLibrary (hInst);
	}

	return bRes;
}

CSize CBCGPGlobalUtils::GetSystemBorders(CWnd *pWnd)
{
	CSize size(0, 0);

	if (::IsWindow(pWnd->GetSafeHwnd()))
	{
		size = GetSystemBorders(pWnd->GetStyle());
	}

	return size;
}

CSize CBCGPGlobalUtils::GetSystemBorders(DWORD dwStyle)
{
	CSize size(0, 0);

	BOOL bCaption = (dwStyle & (WS_CAPTION | WS_DLGFRAME)) != 0;

	if ((dwStyle & WS_THICKFRAME) == WS_THICKFRAME)
	{
		if ((dwStyle & (WS_CHILD | WS_MINIMIZE)) != (WS_CHILD | WS_MINIMIZE))
		{
			size.cx = ::GetSystemMetrics(SM_CXSIZEFRAME);
			size.cy = ::GetSystemMetrics(SM_CYSIZEFRAME);

			if (!bCaption)
			{
				size.cx -= ::GetSystemMetrics(SM_CXBORDER);
				size.cy -= ::GetSystemMetrics(SM_CYBORDER);
			}

			if (size.cx != 0 && size.cy != 0)
			{
				size.cx += ::GetSystemMetrics(SM_CXPADDEDBORDER);
				size.cy += ::GetSystemMetrics(SM_CXPADDEDBORDER);
			}
		}
		else
		{
			size.cx = ::GetSystemMetrics(SM_CXFIXEDFRAME);
			size.cy = ::GetSystemMetrics(SM_CYFIXEDFRAME);
		}
	}
	else if (bCaption || (dwStyle & (WS_BORDER | DS_MODALFRAME)) != 0)
	{
		size.cx = ::GetSystemMetrics(SM_CXFIXEDFRAME);
		size.cy = ::GetSystemMetrics(SM_CYFIXEDFRAME);
	}

	return size;
}

CRuntimeClass* CBCGPGlobalUtils::RuntimeClassFromName(LPCSTR lpszClassName)
{
#if (_MSC_VER <= 1200)

	CRuntimeClass* pClass = NULL;

	// search app specific classes
	AFX_MODULE_STATE* pModuleState = AfxGetModuleState();
	AfxLockGlobals(CRIT_RUNTIMECLASSLIST);
	for (pClass = pModuleState->m_classList; pClass != NULL;
		pClass = pClass->m_pNextClass)
	{
		if (lstrcmpA(lpszClassName, pClass->m_lpszClassName) == 0)
		{
			AfxUnlockGlobals(CRIT_RUNTIMECLASSLIST);
			return pClass;
		}
	}
	AfxUnlockGlobals(CRIT_RUNTIMECLASSLIST);
#ifdef _AFXDLL
	// search classes in shared DLLs
	AfxLockGlobals(CRIT_DYNLINKLIST);
	for (CDynLinkLibrary* pDLL = pModuleState->m_libraryList; pDLL != NULL;
		pDLL = pDLL->m_pNextDLL)
	{
		for (pClass = pDLL->m_classList; pClass != NULL;
			pClass = pClass->m_pNextClass)
		{
			if (lstrcmpA(lpszClassName, pClass->m_lpszClassName) == 0)
			{
				AfxUnlockGlobals(CRIT_DYNLINKLIST);
				return pClass;
			}
		}
	}
	AfxUnlockGlobals(CRIT_DYNLINKLIST);
#endif

	return NULL;

#else

	return CRuntimeClass::FromName(lpszClassName);

#endif
}

CRuntimeClass* CBCGPGlobalUtils::RuntimeClassFromName(LPCWSTR lpszClassName)
{
#if (_MSC_VER <= 1200)

	if(lpszClassName == NULL)
	{
		return NULL;
	}

	int length = (int)wcslen(lpszClassName);
	if (length == 0)
	{
		return NULL;
	}

	LPSTR lpszClassNameA = new char[length + 1];
	AfxW2AHelper(lpszClassNameA, lpszClassName, length);

	CRuntimeClass* pClass = RuntimeClassFromName(lpszClassNameA);

	if (lpszClassNameA != NULL)
	{
		delete [] lpszClassNameA;
	}

	return pClass;

#else

	return CRuntimeClass::FromName(lpszClassName);

#endif
}

BOOL CBCGPGlobalUtils::ProcessMouseWheel(WPARAM wParam, LPARAM lParam)
{
	CPoint ptCursor;
	GetCursorPos(&ptCursor);
	
	HWND hwndCurr = ::WindowFromPoint(ptCursor);
	if (hwndCurr == NULL || !::IsWindow(hwndCurr))
	{
		return FALSE;
	}

	CWnd* pWndCurr = CWnd::FromHandlePermanent(hwndCurr);
	if (pWndCurr->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	CWnd* pWndFocus = CWnd::GetFocus();
	if (pWndFocus->GetSafeHwnd() != NULL)
	{
		TCHAR szClass [256];
		::GetClassName(pWndFocus->GetSafeHwnd (), szClass, 255);
		
		CString strClass = szClass;
		
		if (strClass.CompareNoCase (_T("COMBOBOX")) == 0 || 
			strClass.CompareNoCase (WC_COMBOBOXEX) == 0)
		{
			if ((BOOL)pWndFocus->SendMessage(CB_GETDROPPEDSTATE))
			{
				// The focused combo box is dropped-down: pass the mose wheel event to this control:
				pWndFocus->SendMessage(WM_MOUSEWHEEL, wParam, lParam);
				return TRUE;
			}
		}
		else
		{
			CBCGPCalendar* pWndCalendar = DYNAMIC_DOWNCAST(CBCGPCalendar, pWndFocus);
			if (pWndCalendar->GetSafeHwnd() != NULL && pWndCalendar->IsPopup())
			{
				pWndFocus->SendMessage(WM_MOUSEWHEEL, wParam, lParam);
				return TRUE;
			}
		}
	}
	
	pWndCurr->SendMessage(WM_MOUSEWHEEL, wParam, lParam);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPAboutDlg dialog

class CBCGPAboutDlg : public CBCGPDialog
{
// Construction
public:
	CBCGPAboutDlg(LPCTSTR lpszAppName, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBCGPAboutDlg)
	enum { IDD = IDD_BCGBARRES_ABOUT_DLG };
	CBCGPURLLinkButton	m_wndURL;
	CButton	m_wndPurchaseBtn;
	CStatic	m_wndAppName;
	CBCGPURLLinkButton	m_wndEMail;
	CString	m_strAppName;
	CString	m_strVersion;
	CString	m_strYear;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGPAboutDlg)
	afx_msg void OnBcgbarresPurchase();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CFont				m_fontBold;
	CBCGPToolBarImages	m_Logo;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPAboutDlg dialog

CBCGPAboutDlg::CBCGPAboutDlg(LPCTSTR lpszAppName, CWnd* pParent /*=NULL*/)
	: CBCGPDialog(CBCGPAboutDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBCGPAboutDlg)
	m_strAppName = _T("");
	m_strVersion = _T("");
	m_strYear = _T("");
	//}}AFX_DATA_INIT

	m_strVersion.Format (_T("%d.%d"), _BCGCBPRO_VERSION_MAJOR, _BCGCBPRO_VERSION_MINOR);

	CString strCurrDate = _T(__DATE__);
	m_strYear.Format (_T("1998-%s"), (LPCTSTR)strCurrDate.Right (4));

	m_strAppName = lpszAppName;

#ifndef _BCGSUITE_
	EnableVisualManagerStyle (globalData.m_bUseVisualManagerInBuiltInDialogs, TRUE);
#endif
}

void CBCGPAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPAboutDlg)
	DDX_Control(pDX, IDC_BCGBARRES_URL, m_wndURL);
	DDX_Control(pDX, IDC_BCGBARRES_PURCHASE, m_wndPurchaseBtn);
	DDX_Control(pDX, IDC_BCGBARRES_NAME, m_wndAppName);
	DDX_Control(pDX, IDC_BCGBARRES_MAIL, m_wndEMail);
	DDX_Text(pDX, IDC_BCGBARRES_NAME, m_strAppName);
	DDX_Text(pDX, IDC_BCGBARRES_VERSION, m_strVersion);
	DDX_Text(pDX, IDC_BCGBARRES_YEAR, m_strYear);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGPAboutDlg, CBCGPDialog)
	//{{AFX_MSG_MAP(CBCGPAboutDlg)
	ON_BN_CLICKED(IDC_BCGBARRES_PURCHASE, OnBcgbarresPurchase)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPAboutDlg message handlers

void CBCGPAboutDlg::OnBcgbarresPurchase() 
{
#ifndef _BCGSUITE_
	CString strURL = _T("http://www.bcgsoft.com/register-bcgcbpe.htm");
#else
	CString strURL = _T("http://www.bcgsoft.com/register-bcgsuite.htm");
#endif

	::ShellExecute (NULL, _T("open"), strURL, NULL, NULL, SW_SHOWNORMAL);

	EndDialog (IDOK);
}

BOOL CBCGPAboutDlg::OnInitDialog() 
{
	CBCGPDialog::OnInitDialog();

	//------------------
	// Create bold font:
	//------------------
	CFont* pFont = m_wndAppName.GetFont ();
	ASSERT_VALID (pFont);

	LOGFONT lf;
	memset (&lf, 0, sizeof (LOGFONT));

	pFont->GetLogFont (&lf);

	lf.lfWeight = FW_BOLD;
	m_fontBold.CreateFontIndirect (&lf);

	m_wndAppName.SetFont (&m_fontBold);

	//-----------------------------
	// Setup URL and e-mail fields:
	//-----------------------------
	m_wndEMail.SetURLPrefix (_T("mailto:"));
	m_wndEMail.SetURL (_T("info@bcgsoft.com"));
	m_wndEMail.SizeToContent ();
	m_wndEMail.SetTooltip (_T("Send mail to us"));
	m_wndEMail.m_bDrawFocus = FALSE;

	m_wndURL.m_bDrawFocus = FALSE;
	m_wndURL.SizeToContent ();

	//--------------------
	// Set dialog caption:
	//--------------------
	CString strCaption;
	strCaption.Format (_T("About %s"), m_strAppName);

	SetWindowText (strCaption);

	//----------------------------
	// Hide Logo in High DPI mode:
	//----------------------------
#ifndef _BCGSUITE_
	if (globalData.GetRibbonImageScale () > 1.)
	{
		CWnd* pWndLogo = GetDlgItem (IDC_BCGBARRES_DRAW_AREA);
		if (pWndLogo->GetSafeHwnd () != NULL)
		{
			m_Logo.Load (IDB_BCGBARRES_LOGO);
			m_Logo.SetSingleImage ();
			m_Logo.SmoothResize (globalData.GetRibbonImageScale ());

			pWndLogo->ShowWindow (SW_HIDE);
		}
	}
#endif

	//------------------------------------------
	// Hide "Purchase" button in retail version:
	//------------------------------------------
#ifndef _BCGCBPRO_EVAL_
	m_wndPurchaseBtn.EnableWindow (FALSE);
	m_wndPurchaseBtn.ShowWindow (SW_HIDE);
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void BCGPShowAboutDlg (LPCTSTR lpszAppName)
{
	CBCGPLocalResource locaRes;

	CBCGPAboutDlg dlg (lpszAppName);
	dlg.DoModal ();
}

void CBCGPAboutDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	if (!m_Logo.IsValid ())
	{
		return;
	}

	CRect rectClient;
	GetClientRect (&rectClient);

	m_Logo.DrawEx (&dc, rectClient, 0);
}

void BCGPShowAboutDlg (UINT uiAppNameResID)
{
	CString strAppName;
	strAppName.LoadString (uiAppNameResID);

	BCGPShowAboutDlg (strAppName);
}

