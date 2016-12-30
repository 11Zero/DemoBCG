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
// BCGPDockManager.cpp: implementation of the CBCGPDockManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "BCGCBProVer.h"
#include "BCGPWorkspace.h"
#include "BCGPGlobalUtils.h"
#include "BCGPDockBar.h"
#include "BCGPRebar.h"
#include "BCGPDockingControlBar.h"
#include "BCGPSlider.h"

#include "BCGPAutoHideDockBar.h"
#include "BCGPAutoHideToolBar.h"
#include "BCGPAutoHideButton.h"

#include "BCGPDockManager.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPMultiMiniFrameWnd.h"
#include "BCGPTabbedControlBar.h"
#include "BCGPMDIChildWnd.h"

#include "BCGPCaptionBar.h"

#include "RegPath.h"
#include "BCGPRegistry.h"

#include "BCGPDockBarRow.h"

#include "BCGPReBar.h"
#include "BCGPPopupMenu.h"
#include "BCGPOutlookBar.h"

#include "BCGPMDIFrameWnd.h"

#pragma warning (disable : 4706)

#include "multimon.h"

#pragma warning (default : 4706)

extern CBCGPWorkspace* g_pWorkspace;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define REG_SECTION_FMT						_T("%sBCGPDockManager-%d")
#define REG_ENTRY_DOCKING_CB_AND_SLIDERS	_T ("DockingCBAndSliders")

static const CString strDockManagerProfile	= _T("BCGPDockManagers");

const DWORD dwDockBarMap[4][2] =
{
	{ AFX_IDW_DOCKBAR_TOP,      CBRS_TOP    },
	{ AFX_IDW_DOCKBAR_BOTTOM,   CBRS_BOTTOM },
	{ AFX_IDW_DOCKBAR_LEFT,     CBRS_LEFT   },
	{ AFX_IDW_DOCKBAR_RIGHT,    CBRS_RIGHT  },
};

UINT CBCGPDockManager::g_nTimeOutBeforeToolBarDock = 200;
UINT CBCGPDockManager::g_nTimeOutBeforeDockingBarDock = 220;

BCGP_DOCK_TYPE	CBCGPDockManager::m_dockModeGlobal = BCGP_DT_STANDARD;
UINT			CBCGPDockManager::m_ahSlideModeGlobal = BCGP_AHSM_MOVE;
int				CBCGPDockManager::m_nDockSencitivity = 15;
BOOL			CBCGPDockManager::m_bRestoringDockState = FALSE;
BOOL			CBCGPDockManager::m_bHideDockingBarsInContainerMode = TRUE;
BOOL			CBCGPDockManager::m_bDisableRecalcLayout = FALSE;
BOOL			CBCGPDockManager::m_bFullScreenMode = FALSE;

BOOL			CBCGPDockManager::m_bSavingState = FALSE;

CBCGPSmartDockingParams	CBCGPDockManager::m_SDParams;
BOOL			CBCGPDockManager::m_bSDParamsModified = FALSE;
BCGP_SMARTDOCK_THEME	CBCGPDockManager::m_SDTheme = BCGP_SDT_DEFAULT;

BOOL			CBCGPDockManager::m_bDockBarMenu = FALSE;
BOOL			CBCGPDockManager::m_bIgnoreEnabledAlignment = FALSE;

CRuntimeClass*	CBCGPDockManager::m_pAutoHideToolbarRTC = RUNTIME_CLASS (CBCGPAutoHideToolBar);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CBCGPDockManager::CBCGPDockManager() : m_pParentWnd (NULL),
    m_pSDManager (NULL)
{
	m_dwEnabledDockBars = 0;
	m_dwEnabledSlideBars = 0;
	m_bActivateAutoHideBarOnMouseClick = FALSE;
	m_bMaximizeFloatingBars = FALSE;
	m_bMaximizeFloatingBarsByDblClick = FALSE;
	m_pActiveSlidingWnd = NULL;

	m_pLastTargetBar = NULL;
	m_pLastMultiMiniFrame = NULL;
	m_clkLastTime = 0;
	m_statusLast = BCGP_CS_NOTHING;

	m_rectDockBarBounds.SetRectEmpty ();
	m_rectClientAreaBounds.SetRectEmpty ();
	m_rectOuterEdgeBounds.SetRectEmpty ();

	m_rectInPlace.SetRectEmpty ();

	m_bIsPrintPreviewMode = FALSE;
	m_bEnableAdjustLayout = TRUE;

	m_bLockUpdate = FALSE;
	m_bAdjustingBarLayout = FALSE;
	m_bRecalcLayout = FALSE;
	m_bSizeFrame = FALSE;

	m_bDisableSetDockState = FALSE;

	m_bDisableRestoreDockState = FALSE;

	m_bControlBarsMenuIsShown = FALSE;

	m_bControlBarsContextMenu = FALSE;
	m_bControlBarsContextMenuToolbarsOnly = FALSE;
	m_bControlBarContextMenuRebarPanes = FALSE;
	m_uiCustomizeCmd = 0;

	m_bHiddenForOLE = FALSE;
}
//------------------------------------------------------------------------//
CBCGPDockManager::~CBCGPDockManager()
{
    if (m_pSDManager != NULL)
    {
        delete m_pSDManager;
        m_pSDManager = NULL;
    }
}
//------------------------------------------------------------------------//
BOOL CBCGPDockManager::Create (CFrameWnd* pParentWnd)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pParentWnd);
	m_pParentWnd = pParentWnd;

	return TRUE;
}
//------------------------------------------------------------------------//
BOOL CBCGPDockManager::EnableDocking (DWORD dwStyle)
{
	BCGP_DOCKBAR_INFO info;
	if (dwStyle & CBRS_ALIGN_TOP && (m_dwEnabledDockBars & CBRS_ALIGN_TOP) == 0)
	{
		info.m_dwBarAlignment = CBRS_ALIGN_TOP;
		info.pDockBarRTC = RUNTIME_CLASS (CBCGPDockBar);
		if (!AddDockBar (info))
		{
			return FALSE;
		}
		m_dwEnabledDockBars |= CBRS_ALIGN_TOP;
	}

	if (dwStyle & CBRS_ALIGN_BOTTOM && (m_dwEnabledDockBars & CBRS_ALIGN_BOTTOM) == 0)
	{
		info.m_dwBarAlignment = CBRS_ALIGN_BOTTOM;
		info.pDockBarRTC = RUNTIME_CLASS (CBCGPDockBar);
		if (!AddDockBar (info))
		{
			return FALSE;
		}
		m_dwEnabledDockBars |= CBRS_ALIGN_BOTTOM;
	}

	if (dwStyle & CBRS_ALIGN_LEFT && (m_dwEnabledDockBars & CBRS_ALIGN_LEFT) == 0)
	{
		info.m_dwBarAlignment = CBRS_ALIGN_LEFT;
		info.pDockBarRTC = RUNTIME_CLASS (CBCGPDockBar);
		if (!AddDockBar (info))
		{
			return FALSE;
		}
		m_dwEnabledDockBars |= CBRS_ALIGN_LEFT;
	}

	if (dwStyle & CBRS_ALIGN_RIGHT && (m_dwEnabledDockBars & CBRS_ALIGN_RIGHT) == 0)
	{
		info.m_dwBarAlignment = CBRS_ALIGN_RIGHT;
		info.pDockBarRTC = RUNTIME_CLASS (CBCGPDockBar);
		if (!AddDockBar (info))
		{
			return FALSE;
		}
		m_dwEnabledDockBars |= CBRS_ALIGN_RIGHT;
	}
	AdjustDockingLayout ();

	return TRUE;
}
//------------------------------------------------------------------------//
BOOL CBCGPDockManager::EnableAutoHideBars (DWORD dwStyle, BOOL bActivateOnMouseClick)
{
	BCGP_DOCKBAR_INFO info;

	m_bActivateAutoHideBarOnMouseClick = bActivateOnMouseClick;

	if (dwStyle & CBRS_ALIGN_TOP && (m_dwEnabledSlideBars & CBRS_ALIGN_TOP) == 0)
	{
		if ((m_dwEnabledDockBars & CBRS_ALIGN_TOP) == 0)
		{
			EnableDocking (CBRS_ALIGN_TOP);
		}
		info.m_dwBarAlignment = CBRS_ALIGN_TOP;
		info.pDockBarRTC = RUNTIME_CLASS (CBCGPAutoHideDockBar);
		if (!AddDockBar (info))
		{
			return FALSE;
		}
		m_dwEnabledSlideBars |= CBRS_ALIGN_TOP;
	}

	if (dwStyle & CBRS_ALIGN_BOTTOM  && (m_dwEnabledSlideBars & CBRS_ALIGN_BOTTOM) == 0)
	{
		if ((m_dwEnabledDockBars & CBRS_ALIGN_BOTTOM) == 0)
		{
			EnableDocking (CBRS_ALIGN_BOTTOM);
		}

		info.m_dwBarAlignment = CBRS_ALIGN_BOTTOM;
		info.pDockBarRTC = RUNTIME_CLASS (CBCGPAutoHideDockBar);
		if (!AddDockBar (info))
		{
			return FALSE;
		}
		m_dwEnabledSlideBars |= CBRS_ALIGN_BOTTOM;
	}

	if (dwStyle & CBRS_ALIGN_LEFT && (m_dwEnabledSlideBars & CBRS_ALIGN_LEFT) == 0)
	{
		if ((m_dwEnabledDockBars & CBRS_ALIGN_LEFT) == 0)
		{
			EnableDocking (CBRS_ALIGN_LEFT);
		}

		info.m_dwBarAlignment = CBRS_ALIGN_LEFT;
		info.pDockBarRTC = RUNTIME_CLASS (CBCGPAutoHideDockBar);
		if (!AddDockBar (info))
		{
			return FALSE;
		}
		m_dwEnabledSlideBars |= CBRS_ALIGN_LEFT;
	}

	if (dwStyle & CBRS_ALIGN_RIGHT && (m_dwEnabledSlideBars & CBRS_ALIGN_RIGHT) == 0)
	{
		if ((m_dwEnabledDockBars & CBRS_ALIGN_RIGHT) == 0)
		{
			EnableDocking (CBRS_ALIGN_RIGHT);
		}

		info.m_dwBarAlignment = CBRS_ALIGN_RIGHT;
		info.pDockBarRTC = RUNTIME_CLASS (CBCGPAutoHideDockBar);
		if (!AddDockBar (info))
		{
			return FALSE;
		}
		m_dwEnabledSlideBars |= CBRS_ALIGN_RIGHT;
	}
	
	AdjustDockingLayout ();
	return TRUE;
}
//------------------------------------------------------------------------//
void CBCGPDockManager::EnableMaximizeFloatingBars(BOOL bEnable, BOOL bMaximizeByDblClick)
{
	ASSERT_VALID(this);

	bEnable = bEnable ? TRUE : FALSE;
	BOOL bRebuildCaptionButtons = bEnable != m_bMaximizeFloatingBars;
	
	m_bMaximizeFloatingBars = bEnable;
	m_bMaximizeFloatingBarsByDblClick = bMaximizeByDblClick;

	if (bRebuildCaptionButtons)
	{
		for (POSITION pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
		{
			CBCGPMiniFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, m_lstMiniFrames.GetNext (pos));
			if (pMiniFrame != NULL)
			{
				ASSERT_VALID(pMiniFrame);

				pMiniFrame->SetCaptionButtons(pMiniFrame->m_dwCaptionButtons);
				pMiniFrame->SetCaptionButtonsToolTips (TRUE);
			}
		}
	}
}
//------------------------------------------------------------------------//
BOOL CBCGPDockManager::AddDockBar (const BCGP_DOCKBAR_INFO& info, 
										 CBCGPDockBar** ppDockBar)
{
	ASSERT_VALID (this);

	if (ppDockBar != NULL)
	{
		*ppDockBar = NULL;
	}

	CBCGPDockBar* pDockBar = (CBCGPDockBar*) info.pDockBarRTC->CreateObject ();
	ASSERT_VALID (pDockBar);
	if (pDockBar->Create (info.m_dwBarAlignment, CRect (0, 0, 0, 0), m_pParentWnd, 0))
	{
		m_lstControlBars.AddTail (pDockBar);
	}
	else
	{
		TRACE0 ("Failed to create DockBar");
		delete pDockBar;
		return FALSE;
	}

	if (ppDockBar != NULL)
	{
		*ppDockBar = pDockBar;
	}

	return TRUE;
}
//------------------------------------------------------------------------//
BOOL CBCGPDockManager::InsertDockBar (const BCGP_DOCKBAR_INFO& info, 
										 DWORD dwAlignToInsertAfter, 
										 CBCGPDockBar** ppDockBar)
{
	ASSERT_VALID (this);

	if (ppDockBar != NULL)
	{
		*ppDockBar = NULL;
	}

	CBCGPDockBar* pDockBar = (CBCGPDockBar*) info.pDockBarRTC->CreateObject ();
	ASSERT_VALID (pDockBar);
	if (pDockBar->Create (info.m_dwBarAlignment, CRect (0, 0, 0, 0), m_pParentWnd, 0))
	{
		BOOL bInserted = FALSE;
		for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPBaseControlBar* pNextBar = (CBCGPBaseControlBar*) m_lstControlBars.GetNext (pos);
			ASSERT_VALID (pNextBar);
			
			if (pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPDockBar)) &&
				pNextBar->GetCurrentAlignment () == (dwAlignToInsertAfter & CBRS_ALIGN_ANY) && 
				pos != NULL)
			{
				m_lstControlBars.InsertBefore (pos, pDockBar);
				bInserted = TRUE;
				break;
			}
		}

		if (!bInserted)
		{
			m_lstControlBars.AddTail (pDockBar);
		}				
	}
	else
	{
		TRACE0 ("Failed to create DockBar");
		delete pDockBar;
		return FALSE;
	}

	if (ppDockBar != NULL)
	{
		*ppDockBar = pDockBar;
	}

	return TRUE;
}
//------------------------------------------------------------------------//
BOOL CBCGPDockManager::AddControlBar (CBCGPBaseControlBar* pWnd, BOOL bTail, BOOL bAutoHide, 
									  BOOL bInsertForOuterEdge)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pWnd);

	CObList& lstBars = bAutoHide ? m_lstAutoHideBars : m_lstControlBars;

	if (lstBars.Find (pWnd))
	{
		TRACE0 ("Control bar already added!!!\n");
		ASSERT (FALSE);
		return FALSE;
	}

	if (bTail)
	{
		lstBars.AddTail (pWnd);
	}
	else if (bInsertForOuterEdge)
	{
		// find first control bar with the same alignment and insert before it
		for (POSITION pos = lstBars.GetHeadPosition (); pos != NULL;)
		{
			POSITION posSave = pos;
			CBCGPBaseControlBar* pNextBar = 
				DYNAMIC_DOWNCAST (CBCGPBaseControlBar, m_lstControlBars.GetNext (pos));
			ASSERT_VALID (pNextBar);

			if (pNextBar->DoesAllowDynInsertBefore ())
			{
				lstBars.InsertBefore (posSave, pWnd);
				return TRUE;
			}
		}

		lstBars.AddTail (pWnd);
	}
	else
	{
		lstBars.AddHead (pWnd);
	}

	pWnd->m_pDockSite = m_pParentWnd;

	return TRUE;
}
//------------------------------------------------------------------------//
BOOL CBCGPDockManager::InsertControlBar (CBCGPBaseControlBar* pControlBar, 
										     CBCGPBaseControlBar* pTarget, BOOL bAfter)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pControlBar);	

	if (m_lstControlBars.Find (pControlBar))
	{
		TRACE0 ("Control bar already added!!!\n");
		ASSERT (FALSE);
		return FALSE;
	}

	POSITION pos = m_lstControlBars.Find (pTarget);
	if (pos == NULL)
	{
		TRACE0 ("Control bar does not exist in the control container!!!\n");
		ASSERT (FALSE);
		return FALSE;
	}

	if (bAfter)
	{
		m_lstControlBars.InsertAfter (pos, pControlBar);
	}
	else
	{
		m_lstControlBars.InsertBefore (pos, pControlBar);
	}
	return TRUE;
}
//------------------------------------------------------------------------//
void CBCGPDockManager::RemoveControlBarFromDockManager (CBCGPBaseControlBar* pWnd, BOOL bDestroy,
										 BOOL bAdjustLayout, BOOL bAutoHide, CBCGPBaseControlBar* pBarReplacement)
{
	CObList& lstBars = bAutoHide ? m_lstAutoHideBars : m_lstControlBars;

	POSITION pos = lstBars.Find (pWnd);
	if (pBarReplacement != NULL)
	{
		pos != NULL ? lstBars.InsertAfter (pos, pBarReplacement) : lstBars.AddTail (pBarReplacement);
	}
	if (pos != NULL)
	{
		lstBars.RemoveAt (pos);
		if (bDestroy)
		{
			pWnd->DestroyWindow ();
		}

		if (bAdjustLayout)
		{
			AdjustDockingLayout ();
		}
	}
}
//------------------------------------------------------------------------//
BOOL CBCGPDockManager::AddMiniFrame (CBCGPMiniFrameWnd* pWnd)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pWnd);

	POSITION pos = m_lstMiniFrames.Find (pWnd);

	if (pos != NULL)
	{
		return FALSE;
	}

	m_lstMiniFrames.AddTail (pWnd);
	return TRUE;
}
//------------------------------------------------------------------------//
BOOL CBCGPDockManager::RemoveMiniFrame (CBCGPMiniFrameWnd* pWnd)
{
	ASSERT_VALID (this);

	POSITION pos = m_lstMiniFrames.Find (pWnd);
	if (pos != NULL)
	{
		m_lstMiniFrames.RemoveAt (pos);
		return TRUE;
	}

	return FALSE;
}
//------------------------------------------------------------------------//
CBCGPBaseControlBar* CBCGPDockManager::ControlBarFromPoint (CPoint point, int nSensitivity, 
															   bool bExactBar, 
															   CRuntimeClass* pRTCBarType, 
															   BOOL bCheckVisibility, 
															   const CBCGPBaseControlBar* pBarToIgnore) const
{
	ASSERT_VALID (this);
	
    if (m_pSDManager != NULL)
    {
        CBCGPSmartDockingMarker::SDMarkerPlace nHilitedSide = m_pSDManager->GetHilitedMarkerNo ();
        if (nHilitedSide >= CBCGPSmartDockingMarker::sdLEFT && nHilitedSide <= CBCGPSmartDockingMarker::sdBOTTOM)
        {
            return NULL;
        }
    }

    CBCGPMiniFrameWnd* pMiniFrameToIgnore = NULL;
    if (pBarToIgnore != NULL)
    {
        pMiniFrameToIgnore = pBarToIgnore->GetParentMiniFrame (TRUE);
    }

	CBCGPMiniFrameWnd* pFrame = FrameFromPoint (point, pMiniFrameToIgnore, FALSE);
	if (pFrame != NULL)
	{
		CBCGPBaseControlBar* pBar = 
			pFrame->ControlBarFromPoint (point, nSensitivity, bCheckVisibility);
		if (pBar != NULL && pBar != pBarToIgnore && 
			(pRTCBarType == NULL || pBar->IsKindOf (pRTCBarType)))
		{
			return pBar;
		}
	}

	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pNextBar = (CBCGPBaseControlBar*) m_lstControlBars.GetNext (pos);
		ASSERT_VALID (pNextBar);

		if ((pRTCBarType == NULL || pNextBar->IsKindOf (pRTCBarType)))  
		{
			if (bCheckVisibility && !pNextBar->IsBarVisible () ||
				pNextBar == pBarToIgnore)
			{
				continue;
			}
			
			CRect rectWnd;
			pNextBar->GetWindowRect (rectWnd);
			if (!bExactBar)
			{
				rectWnd.InflateRect (nSensitivity, nSensitivity);
			}
			
			if (rectWnd.PtInRect (point))
			{
				return pNextBar;	
			}
		}
	}

	return NULL;
}
//------------------------------------------------------------------------//
CBCGPBaseControlBar* CBCGPDockManager::ControlBarFromPoint (CPoint point, 
								   int nSensitivity, DWORD& dwAlignment, 
								   CRuntimeClass* pRTCBarType, 
								   const CBCGPBaseControlBar* pBarToIgnore) const
{
	ASSERT_VALID (this);
	dwAlignment = 0;
	CBCGPBaseControlBar* pBar = ControlBarFromPoint (point, nSensitivity, true, 
													 NULL, FALSE, pBarToIgnore);

	if (pBar != NULL) 
	{
		if ((pRTCBarType == NULL || pBar->IsKindOf (pRTCBarType)))
		{
			if (!globalUtils.CheckAlignment (point, pBar, nSensitivity, 
				this, FALSE, dwAlignment))
			{
				return NULL;
			}
		}
		else
		{
			pBar = NULL;
		}
	}

	return pBar;
}
//------------------------------------------------------------------------//
BCGP_CS_STATUS CBCGPDockManager::DetermineControlBarAndStatus (CPoint pt, 
									int nSencitivity, 
									DWORD dwEnabledAlignment, 
									CBCGPBaseControlBar** ppTargetBar, 
									const CBCGPBaseControlBar* pBarToIgnore, 
									const CBCGPBaseControlBar* pBarToDock)
{
	ASSERT_VALID (pBarToDock);

	// find the exact control bar first. 
	*ppTargetBar = ControlBarFromPoint (pt, nSencitivity, true, 
									  RUNTIME_CLASS (CBCGPDockingControlBar), TRUE, 
									  pBarToIgnore);

	if (*ppTargetBar == NULL)
	{
		// find a miniframe from point and check it for a single bar
		CBCGPMiniFrameWnd* pMiniFrame = FrameFromPoint (pt, NULL, TRUE);
		if (pMiniFrame != NULL && pBarToDock->GetParentMiniFrame () != pMiniFrame)
		{
			// detect caption
			BCGNcHitTestType uiHitTest = pMiniFrame->HitTest (pt, TRUE);
			if (uiHitTest == HTCAPTION && pMiniFrame->GetVisibleBarCount () == 1)
			{
				*ppTargetBar = DYNAMIC_DOWNCAST 
					(CBCGPBaseControlBar, pMiniFrame->GetFirstVisibleBar ());
				return BCGP_CS_DELAY_DOCK_TO_TAB;
			}
		}
	}
	// check this bar for caption and tab area
	if (*ppTargetBar != NULL)
	{
		if ((*ppTargetBar)->GetParentMiniFrame () != NULL && 
			(pBarToDock->GetBarStyle () & CBRS_FLOAT_MULTI) &&
			((*ppTargetBar)->GetBarStyle () & CBRS_FLOAT_MULTI) ||
			(*ppTargetBar)->GetParentMiniFrame () == NULL)
		{
		
			CBCGPDockingControlBar* pDockingBar = 
				DYNAMIC_DOWNCAST (CBCGPDockingControlBar, *ppTargetBar);

			if (pDockingBar != NULL && !pDockingBar->IsFloating () && 
				(pDockingBar->GetCurrentAlignment () & dwEnabledAlignment) == 0)
			{
				return BCGP_CS_NOTHING;
			}

			if (pDockingBar != NULL)
			{
				return pDockingBar->GetDockStatus (pt, nSencitivity);
			}
		}
	}
	
	*ppTargetBar = NULL;

	// check whether the mouse cursor is at the outer edge of the dock bar
	// or at the inner edge of the most inner control bar (on client area) and the 
	// bar is allowed to be docked at this side
	BOOL bOuterEdge = FALSE;
	DWORD dwAlignment = 0;

	if (IsPointNearDockBar (pt, dwAlignment, bOuterEdge) && 
								(dwAlignment & dwEnabledAlignment))
	{
		return BCGP_CS_DELAY_DOCK;
	}
	

	return BCGP_CS_NOTHING;
}
//------------------------------------------------------------------------//
CBCGPBaseControlBar* CBCGPDockManager::FindBarByID (UINT uBarID, BOOL bSearchMiniFrames)
{
	ASSERT_VALID (this);

	POSITION pos = NULL;

	for (pos = m_lstAutoHideBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pBar = (CBCGPBaseControlBar*) 
										m_lstAutoHideBars.GetNext (pos);

		ASSERT_VALID (pBar);
		if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPSlider)))
		{
			CBCGPSlider* pSlider = DYNAMIC_DOWNCAST (CBCGPSlider, pBar);
			// SLIDER CONTAINS ONLY ONE BAR IN AUTOHIDE MODE
			pBar = (CBCGPBaseControlBar*) pSlider->GetFirstBar ();
		}

		if (pBar == NULL)
		{
			continue;
		}

		UINT uID = pBar->GetDlgCtrlID ();

		if (uID == uBarID)
		{
			return pBar;
		}
	}

	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pBar = (CBCGPBaseControlBar*) 
										m_lstControlBars.GetNext (pos);

		UINT uID = pBar->GetDlgCtrlID ();

		if (uID == uBarID)
		{
			return pBar;
		}

		if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPBaseTabbedBar)))
		{
			CBCGPBaseTabbedBar* pTabbedBar = 
				DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pBar);
			ASSERT_VALID (pTabbedBar);

			CBCGPBaseControlBar* pControlBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar, 
												pTabbedBar->FindBarByID (uBarID));
			if (pControlBar != NULL)
			{
				return pControlBar;
			}
		}
		else if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPDockBar)))
		{
			CBCGPDockBar* pDockBar = (CBCGPDockBar*) pBar;
			ASSERT_VALID (pDockBar);

			CBCGPControlBar* pBar = pDockBar->FindBarByID (uBarID);
			if (pBar != NULL)
			{
				return DYNAMIC_DOWNCAST (CBCGPBaseControlBar, pBar);
			}
		}
		else if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPReBar)))
		{
			CBCGPReBar* pReBar = (CBCGPReBar*) pBar;
			ASSERT_VALID (pReBar);

			CBCGPBaseControlBar* pBar = DYNAMIC_DOWNCAST(CBCGPBaseControlBar,
					pReBar->GetDlgItem (uBarID));
			if (pBar != NULL)
			{
				return pBar;
			}
		}
	}
	

	if (bSearchMiniFrames)
	{
		for (pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
		{
			CBCGPMiniFrameWnd* pMiniFrame = 
				DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, m_lstMiniFrames.GetNext (pos));
			if (pMiniFrame != NULL)
			{
				CBCGPBaseControlBar* pWnd = 
					DYNAMIC_DOWNCAST (CBCGPBaseControlBar, pMiniFrame->GetControlBar ());
				if (pWnd != NULL && pWnd->GetDlgCtrlID () == (int) uBarID)
				{
					return pWnd;
				}
			}
		}

		return CBCGPMiniFrameWnd::FindFloatingBarByID (uBarID);
	}

	return NULL;
}
//------------------------------------------------------------------------//
CBCGPDockBar* CBCGPDockManager::FindDockBar (DWORD dwAlignment, BOOL bOuter)
{
	for (POSITION pos = bOuter ? m_lstControlBars.GetHeadPosition () : 
								 m_lstControlBars.GetTailPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pBar = (CBCGPBaseControlBar*) 
										(bOuter ? m_lstControlBars.GetNext (pos) : 
										m_lstControlBars.GetPrev (pos));
		ASSERT_VALID (pBar);

		if (!pBar->IsKindOf (RUNTIME_CLASS (CBCGPDockBar)))
		{
			continue;
		}

		if (pBar->GetCurrentAlignment () == (dwAlignment & CBRS_ALIGN_ANY))
		{
			return DYNAMIC_DOWNCAST (CBCGPDockBar, pBar);
		}
	}

	return NULL;
}
//------------------------------------------------------------------------//
void CBCGPDockManager::FixupVirtualRects ()
{
	ASSERT_VALID (this);
	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockBar, 
												m_lstControlBars.GetNext (pos));
		if (pBar != NULL)
		{
			pBar->FixupVirtualRects ();
		}
	}
	AdjustDockingLayout ();
}
//------------------------------------------------------------------------//
BOOL CBCGPDockManager::IsPointNearDockBar (CPoint point, DWORD& dwBarAlignment, 
											BOOL& bOuterEdge) const
{
	ASSERT_VALID (this);
	dwBarAlignment = 0;
	// check the "outer" edge first - non resizable dock bars
	
	CRect rectBounds = m_rectOuterEdgeBounds;
	m_pParentWnd->ClientToScreen (rectBounds);

	bOuterEdge = TRUE;
	if (globalUtils.CheckAlignment (point, NULL, CBCGPDockManager::m_nDockSencitivity, 
									this, bOuterEdge,
									dwBarAlignment, m_dwEnabledDockBars, rectBounds))
	{
		return TRUE;
	}

	// check the innre edges - edges of the client area
	rectBounds = m_rectClientAreaBounds;
	m_pParentWnd->ClientToScreen (rectBounds);

	bOuterEdge = FALSE;	
	return globalUtils.CheckAlignment (point, NULL, CBCGPDockManager::m_nDockSencitivity, 
										this, bOuterEdge, dwBarAlignment, m_dwEnabledDockBars, rectBounds);
}
//------------------------------------------------------------------------//
BOOL CBCGPDockManager::DockControlBarLeftOf (CBCGPControlBar* pBarToDock, 
											 CBCGPControlBar* pTargetBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBarToDock);
	ASSERT_VALID (pTargetBar);

	if (pTargetBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)) && 
		pBarToDock->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)))
	{
	}
	else if (pTargetBar->IsKindOf (RUNTIME_CLASS (CBCGPToolBar)) && 
			 pBarToDock->IsKindOf (RUNTIME_CLASS (CBCGPToolBar)))
	{
		CBCGPDockBar* pDockBar = FindDockBarByControlBar (pTargetBar);
		if (pDockBar != NULL)
		{
			pBarToDock->UnDockControlBar (TRUE);
			BOOL bResult = pDockBar->DockControlBarLeftOf (pBarToDock, pTargetBar);
			return bResult;
		}
	}

	return FALSE;
}
//------------------------------------------------------------------------//
CBCGPDockBar* CBCGPDockManager::FindDockBarByControlBar (CBCGPControlBar* pTargetBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pTargetBar);

	UINT uID = pTargetBar->GetDlgCtrlID ();

	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockBar, 
												m_lstControlBars.GetNext (pos));
		if (pBar != NULL)
		{
			if (pBar->FindBarByID (uID) == pTargetBar)
			{
				return pBar;
			}
		}
	}
	return NULL;
}
//------------------------------------------------------------------------//
// Should be used for toolbars or (resizable) control bars that can be docked
// on a resizable DockBar
//------------------------------------------------------------------------//
void CBCGPDockManager::DockControlBar (CBCGPBaseControlBar* pBar, UINT nDockBarID, 
											LPCRECT lpRect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	if (m_lstControlBars.IsEmpty ())
	{
		return;
	}

	// if the bar can be
	pBar->UnDockControlBar (TRUE);

	if (!pBar->CanBeResized () && !pBar->CanFloat ())
	{
		AddControlBar (pBar);
		return;
	}
	

	DWORD dwBarDockStyle = pBar->GetEnabledAlignment ();

	if (pBar->IsResizable ())
	{
		// resazable control bars are docked to frame window (their dock site)
		// directly
		if (nDockBarID == 0)
		{
			pBar->DockToFrameWindow (pBar->GetCurrentAlignment (), lpRect);
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				DWORD dwDockBarID = dwDockBarMap [i][0];
				DWORD dwDockAlign = dwDockBarMap [i][1];

				if ((nDockBarID == 0 || nDockBarID == dwDockBarID) && 
					(dwDockAlign & dwBarDockStyle))
				{
					pBar->DockToFrameWindow (dwDockAlign, lpRect);
					break;
				}
			}
		}
	}
	else
	{
		for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPBaseControlBar* pNextBar = (CBCGPBaseControlBar*) 
												m_lstControlBars.GetNext (pos);
			ASSERT_VALID (pNextBar);

			if (pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPDockBar)))
			{
				CBCGPDockBar* pNextDockBar = (CBCGPDockBar*) pNextBar;

				if ((nDockBarID == 0 || pNextDockBar->GetDockBarID () == nDockBarID) && 
					pBar->CanBeDocked (pNextDockBar) && pNextDockBar->CanAcceptBar (pBar))
				{
					if (pBar->DockControlBar (pNextDockBar, lpRect, BCGP_DM_RECT))
					{
						pBar->InvalidateRect (NULL);
						break;
					}
				}
			}
		}
	}
}
//------------------------------------------------------------------------//
CBCGPAutoHideToolBar* CBCGPDockManager::AutoHideBar (CBCGPDockingControlBar* pBar, 
											CBCGPAutoHideToolBar* pCurrAutoHideToolBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	DWORD dwAlignment = pBar->GetCurrentAlignment ();

	// create autohide toolbar and button - it's always inner dockbar
	CBCGPAutoHideDockBar* pAutoHideDockBar = 
		DYNAMIC_DOWNCAST (CBCGPAutoHideDockBar, FindDockBar (dwAlignment, FALSE));

	if (pAutoHideDockBar == NULL)
	{
		// no autohide allowed at this side
		return NULL;
	}

	CBCGPAutoHideToolBar* pAutoHideToolBar = pCurrAutoHideToolBar; 
	
	if (pAutoHideToolBar == NULL)	
	{
		pAutoHideToolBar = DYNAMIC_DOWNCAST (CBCGPAutoHideToolBar, m_pAutoHideToolbarRTC->CreateObject ());

		ASSERT_VALID (pAutoHideToolBar);

		DWORD dwBCGStyle = 0; // can't float...
		if (!pAutoHideToolBar->Create (NULL, WS_VISIBLE | WS_CHILD, CRect (0, 0, 0, 0), 
										m_pParentWnd, 1, dwBCGStyle))
		{
			TRACE0 ("Failde to create autohide toolbar");
			ASSERT (FALSE);
			delete pAutoHideToolBar;
			return NULL;
		}
	}
	pAutoHideToolBar->EnableDocking (CBRS_ALIGN_ANY);
	
	CBCGPSlider* pDefaultSlider = pBar->GetDefaultSlider ();
	ASSERT_VALID (pDefaultSlider);

	CBCGPAutoHideButton* pBtn = pAutoHideToolBar->AddAutoHideWindow (pBar, dwAlignment);
	if (pBtn == NULL)
	{
		ASSERT(FALSE);
	}

	ASSERT_VALID (pBtn);

	// NULL indicates that there was a new toolbar created here
	if (pCurrAutoHideToolBar == NULL)
	{
		if (!pAutoHideDockBar->IsBarVisible ())
		{
			pAutoHideDockBar->ShowWindow (SW_SHOW);
		}
		pAutoHideToolBar->DockControlBar (pAutoHideDockBar, NULL, BCGP_DM_RECT);
	}

	// recalc. layout according to the newly added bar
	AdjustDockingLayout ();

	// register the slider with the manager
	AddControlBar (pDefaultSlider, TRUE, TRUE);

	AlignAutoHideBar (pDefaultSlider);

	pBar->BringWindowToTop ();
	pDefaultSlider->BringWindowToTop ();

	return pAutoHideToolBar;
}
//------------------------------------------------------------------------//
void CBCGPDockManager::HideAutoHideBars (CBCGPDockingControlBar* pBarToExclude, BOOL bImmediately)
{
	for (POSITION pos = m_lstAutoHideBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPSlider* pSlider = (CBCGPSlider*) m_lstAutoHideBars.GetNext (pos);
		ASSERT_VALID (pSlider);

		CBCGPDockingControlBar* pControlBar = (CBCGPDockingControlBar*) 
													pSlider->GetFirstBar ();
		ASSERT_VALID (pControlBar);

		if (pControlBar == pBarToExclude)
		{
			continue;
		}

		if (pControlBar->IsBarVisible ())
		{
			pControlBar->Slide (FALSE, !bImmediately);
		}
	}
}
//------------------------------------------------------------------------//
void CBCGPDockManager::AlignAutoHideBar (CBCGPSlider* pDefaultSlider, BOOL bIsVisible)
{
	CRect rectSlider;

	pDefaultSlider->GetWindowRect (rectSlider);
	BOOL bHorz = pDefaultSlider->IsHorizontal ();

	DWORD dwAlignment = pDefaultSlider->GetCurrentAlignment ();
	BOOL bIsRTL = m_pParentWnd->GetExStyle () & WS_EX_LAYOUTRTL;

	if (bIsVisible)
	{
		CSize sizeRequered = pDefaultSlider->CalcFixedLayout (FALSE, bHorz);
		
		if (bHorz)
		{
			dwAlignment & CBRS_ALIGN_TOP ? rectSlider.bottom = rectSlider.top + sizeRequered.cy : 
										   rectSlider.top = rectSlider.bottom - sizeRequered.cy;
		}
		else
		{
			dwAlignment & CBRS_ALIGN_LEFT ? rectSlider.right = rectSlider.left + sizeRequered.cx : 
											rectSlider.left = rectSlider.right - sizeRequered.cx;
		}

		// m_rectOuterEdgeBounds - the area surrounded by dock bars
		CRect rectBoundsScreen = m_rectOuterEdgeBounds;
		m_pParentWnd->ClientToScreen (rectBoundsScreen);
		AlignByRect (rectBoundsScreen, rectSlider, dwAlignment, bHorz, TRUE);	

		HDWP hdwp = NULL;
		pDefaultSlider->RepositionBars (rectSlider, hdwp);			
	}
	else
	{
		// it can be nonvisible only when moved out of screen - adjust  only width/height
		CBCGPBaseControlBar* pControlBar = (CBCGPBaseControlBar*) 
														pDefaultSlider->GetFirstBar ();
		CRect rectControlBar;
		pControlBar->GetWindowRect (rectControlBar);

		pDefaultSlider->GetParent ()->ScreenToClient (rectSlider);
		pDefaultSlider->GetParent ()->ScreenToClient (rectControlBar);

		if (bHorz)
		{
			rectSlider.left = rectControlBar.left = m_rectOuterEdgeBounds.left;
			rectSlider.right = rectControlBar.right = m_rectOuterEdgeBounds.right;
		}
		else
		{
			rectSlider.top = rectControlBar.top = m_rectOuterEdgeBounds.top;
			rectSlider.bottom = rectControlBar.bottom = m_rectOuterEdgeBounds.bottom;			
		}

		CPoint ptOffset (0, 0);

		// slider is not hidden completely - it is aligned by m_rectOuterEdgeBounds 
		switch (dwAlignment)
		{
		case CBRS_ALIGN_LEFT:
			if (bIsRTL)
			{
				if (rectSlider.right != m_rectOuterEdgeBounds.right)
				{
					ptOffset.x = m_rectOuterEdgeBounds.right - rectSlider.right;
				}
			}
			else
			{
				if (rectSlider.left != m_rectOuterEdgeBounds.left)
				{
					ptOffset.x = m_rectOuterEdgeBounds.left - rectSlider.left;
				}
			}
			break;
		case CBRS_ALIGN_RIGHT:
			if (bIsRTL)
			{
				if (rectSlider.left != m_rectOuterEdgeBounds.left)
				{
					ptOffset.x = m_rectOuterEdgeBounds.left - rectSlider.left;
				}
			}
			else
			{
				if (rectSlider.right != m_rectOuterEdgeBounds.right)
				{
					ptOffset.x = m_rectOuterEdgeBounds.right - rectSlider.right;
				}
			}
			break;
		case CBRS_ALIGN_TOP:
			if (rectSlider.top != m_rectOuterEdgeBounds.top)
			{
				ptOffset.y = m_rectOuterEdgeBounds.top - rectSlider.top;
			}
			break;
		case CBRS_ALIGN_BOTTOM:
			if (rectSlider.bottom != m_rectOuterEdgeBounds.bottom)
			{
				ptOffset.y = m_rectOuterEdgeBounds.bottom - rectSlider.bottom;
			}
			break;
		}

		rectSlider.OffsetRect (ptOffset);
		rectControlBar.OffsetRect (ptOffset);

		pDefaultSlider->SetWindowPos (NULL, rectSlider.left, rectSlider.top, 
											rectSlider.Width (), rectSlider.Height (), 
											SWP_NOZORDER | SWP_NOACTIVATE);
		pControlBar->SetWindowPos (NULL, rectControlBar.left, rectControlBar.top, 
											rectControlBar.Width (), rectControlBar.Height (), 
											SWP_NOZORDER | SWP_NOACTIVATE);
		pControlBar->RecalcLayout ();

	}
}
//------------------------------------------------------------------------//
void CBCGPDockManager::CalcExpectedDockedRect (CWnd* pWnd, CPoint ptMouse, 
											   CRect& rectResult, BOOL& bDrawTab, 
											   CBCGPDockingControlBar** ppTargetBar)
{
	ASSERT_VALID (this);
	
	rectResult.SetRectEmpty ();

	if (GetKeyState (VK_CONTROL) < 0)
	{
		return;
	}


	BOOL bOuterEdge = FALSE;
	DWORD dwAlignment = 0;

    CBCGPMiniFrameWnd* pOtherMiniFrame = FrameFromPoint (ptMouse, DYNAMIC_DOWNCAST(CBCGPMiniFrameWnd, pWnd), TRUE);

	if (pOtherMiniFrame != NULL)
	{
		pOtherMiniFrame->CalcExpectedDockedRect (pWnd, ptMouse, rectResult, 
													bDrawTab, ppTargetBar);
	}
	
	if (pOtherMiniFrame == NULL || rectResult.IsRectEmpty ())
	{
        CBCGPBaseControlBar* pThisControlBar =
            (m_pSDManager != NULL && m_pSDManager->IsStarted()) ?
            DYNAMIC_DOWNCAST (CBCGPBaseControlBar, (DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, pWnd))->GetControlBar()) :
            NULL;

		CBCGPDockingControlBar* pBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
							ControlBarFromPoint (ptMouse, CBCGPDockManager::m_nDockSencitivity, 
														true, NULL, TRUE,
                                                        pThisControlBar));

		if (pBar != NULL && pBar->GetDefaultSlider () != NULL)
		{
			if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd)))
			{
				CBCGPMiniFrameWnd* pMiniFrame = 
					DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, pWnd);
				ASSERT_VALID (pMiniFrame);
				if (!pBar->CanAcceptMiniFrame (pMiniFrame))
				{
					return;
				}
			}

			CBCGPSlider* pDefaultSlider = pBar->GetDefaultSlider ();
			ASSERT_VALID (pDefaultSlider);

			pDefaultSlider->CalcExpectedDockedRect (pWnd, ptMouse, rectResult, bDrawTab, 
													ppTargetBar);
		}
		else
		if (IsPointNearDockBar (ptMouse, dwAlignment, bOuterEdge))
		{
			*ppTargetBar = NULL;

			if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPMiniFrameWnd)))
			{
				CBCGPMiniFrameWnd* pMiniFrame = 
					DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, pWnd);
				ASSERT_VALID (pMiniFrame);
				CBCGPControlBar* pBar = 
					DYNAMIC_DOWNCAST (CBCGPControlBar, pMiniFrame->GetControlBar ());
				if (pBar != NULL && (pBar->GetEnabledAlignment () & dwAlignment) == 0)
				{
					return;
				}
			}
			else if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)))
			{
				CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pWnd);
				if ((pBar->GetEnabledAlignment () & dwAlignment) == 0)
				{
					return;
				}
			}

			CRect rectWnd;
			pWnd->GetWindowRect (rectWnd);

			rectResult = bOuterEdge ? m_rectOuterEdgeBounds : m_rectClientAreaBounds;
		
			BOOL bIsRTL = m_pParentWnd->GetExStyle () & WS_EX_LAYOUTRTL;

			switch (dwAlignment)
			{
			case CBRS_ALIGN_LEFT:
				if (bIsRTL)
				{
					rectResult.left = rectResult.right - rectWnd.Width ();
				}
				else
				{
					rectResult.right = rectResult.left + rectWnd.Width ();
				}
				break;
			case CBRS_ALIGN_RIGHT:
				if (bIsRTL)
				{
					rectResult.right = rectResult.left + rectWnd.Width ();
				}
				else
				{
					rectResult.left = rectResult.right - rectWnd.Width ();
				}
				break;
			case CBRS_ALIGN_TOP:
				rectResult.bottom = rectResult.top + rectWnd.Height ();
				break;
			case CBRS_ALIGN_BOTTOM:
				rectResult.top = rectResult.bottom - rectWnd.Height ();
				break;
			}
			
			AdjustRectToClientArea (rectResult, dwAlignment);
			m_pParentWnd->ClientToScreen (rectResult);
			
		}
		else
		{
			*ppTargetBar = NULL;
		}
	}
}
//-----------------------------------------------------------------------//
BOOL CBCGPDockManager::AdjustRectToClientArea (CRect& rectResult, DWORD dwAlignment)
{
	BOOL bAdjusted = FALSE;

	int nAllowedHeight = (int) (m_rectClientAreaBounds.Height () * 
						 globalData.m_nCoveredMainWndClientAreaPercent / 100);
	int nAllowedWidth = (int) (m_rectClientAreaBounds.Width () * 
								 globalData.m_nCoveredMainWndClientAreaPercent / 100);

	if (dwAlignment & CBRS_ORIENT_HORZ && rectResult.Height () >= nAllowedHeight)
	{
		
		if (dwAlignment & CBRS_ALIGN_TOP)
		{
			rectResult.bottom = rectResult.top + nAllowedHeight;
			bAdjusted = TRUE;
		}
		else if (dwAlignment & CBRS_ALIGN_BOTTOM)
		{
			rectResult.top = rectResult.bottom - nAllowedHeight;
			bAdjusted = TRUE;
		}
	}
	else if (dwAlignment & CBRS_ORIENT_VERT && 
				rectResult.Width () >= nAllowedWidth)
	{
		BOOL bIsRTL = m_pParentWnd->GetExStyle () & WS_EX_LAYOUTRTL;
		
		if (dwAlignment & CBRS_ALIGN_LEFT)
		{
			if (bIsRTL)
			{
				rectResult.left = rectResult.right - nAllowedWidth;
			}
			else
			{
				rectResult.right = rectResult.left + nAllowedWidth;
			}
			bAdjusted = TRUE;
		}
		else if (dwAlignment & CBRS_ALIGN_RIGHT)
		{
			if (bIsRTL)
			{
				rectResult.right = rectResult.left + nAllowedWidth;
			}
			else
			{
				rectResult.left = rectResult.right - nAllowedWidth;
			}
			bAdjusted = TRUE;
		}
	}

	return bAdjusted;
}
//------------------------------------------------------------------------//
BOOL CBCGPDockManager::OnMoveMiniFrame	(CWnd* pFrame)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pFrame);

	if (GetKeyState (VK_CONTROL) < 0)
	{
		return TRUE;
	}

	BOOL bResult = TRUE;

	CBCGPMiniFrameWnd* pBCGMiniFrame = DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, pFrame);

	BOOL bSDockingIsOn = m_pSDManager != NULL && m_pSDManager->IsStarted();

	if (pBCGMiniFrame != NULL)
	{
		CRect rect;
		pFrame->GetWindowRect (rect);
		int captionHeight = pBCGMiniFrame->GetCaptionHeight ();
		CRect rectDelta (captionHeight, captionHeight, captionHeight, captionHeight);
		globalUtils.AdjustRectToWorkArea (rect, &rectDelta);
		
		pBCGMiniFrame->SetWindowPos (NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	
	if (pBCGMiniFrame != NULL)
	{
		CPoint ptMouse;
		GetCursorPos (&ptMouse);

		CBCGPControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPControlBar, 
												pBCGMiniFrame->GetControlBar ());

		// first, check if there is any other miniframe around and whether 
		// it is possible to dock at this miniframe - but only if the 
		// current bar is docking control bar has style cbrs_float_multi

		if ((pBar == NULL) ||
			(!bSDockingIsOn && (pBar->GetBarStyle () & CBRS_FLOAT_MULTI) && 
			 pBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar))))
		{
			CBCGPMiniFrameWnd* pOtherMiniFrame = 
				FrameFromPoint (ptMouse, pBCGMiniFrame, TRUE);

			// dock only bars from miniframes that have the same parent main frame,
			// otherwise it will create problems for dockmanagers 
			if (pOtherMiniFrame != NULL && 
				pOtherMiniFrame->GetParent () == pBCGMiniFrame->GetParent ())
			{
				CBCGPMultiMiniFrameWnd* pMultiMiniFrame = 
					DYNAMIC_DOWNCAST (CBCGPMultiMiniFrameWnd, pOtherMiniFrame);

				if (pMultiMiniFrame != NULL && m_pLastMultiMiniFrame == NULL)
				{
					m_clkLastTime = clock ();
					m_pLastMultiMiniFrame = pMultiMiniFrame;
				}
				
				if (pMultiMiniFrame != NULL && 
					m_pLastMultiMiniFrame == pMultiMiniFrame && 
					clock () - m_clkLastTime > (int) g_nTimeOutBeforeDockingBarDock)
				{
					bResult = pMultiMiniFrame->DockFrame (pBCGMiniFrame, 
										BCGP_DM_MOUSE);
					m_clkLastTime = clock ();
					m_pLastMultiMiniFrame = NULL;
					return bResult;
				}
				
				return TRUE;
			}
		}
		
		m_pLastMultiMiniFrame = NULL;

		if (pBar != NULL)
		{
			if ((pBar->GetEnabledAlignment () & CBRS_ALIGN_ANY) == 0)
			{
				// docking was not enabled for this control bar
				return TRUE;
			}

			// target control bar or dock bar
			CBCGPBaseControlBar* pTargetBar = NULL;
			BCGP_CS_STATUS status = pBar->
				IsChangeState (CBCGPDockManager::m_nDockSencitivity, &pTargetBar);

			if (pBar == pTargetBar)
			{
				status = BCGP_CS_NOTHING;
			}

			if ((pTargetBar != NULL || status == BCGP_CS_DELAY_DOCK) && !bSDockingIsOn) 
			{
				BOOL bDockBar =  pTargetBar != NULL ? 
					pTargetBar->IsKindOf (RUNTIME_CLASS (CBCGPDockBar)) : FALSE;

				BOOL bDockingBar = pTargetBar != NULL ? 
					pTargetBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)) : TRUE;

				if (bDockBar || bDockingBar)
				{
					UINT uTimeOut = bDockBar ? g_nTimeOutBeforeToolBarDock : 
												g_nTimeOutBeforeDockingBarDock;
					if (m_pLastTargetBar != pTargetBar || status != m_statusLast)
					{
						m_clkLastTime = clock ();
						m_pLastTargetBar = pTargetBar;
						m_statusLast = status;
						
						pBCGMiniFrame->SetDockTimer (uTimeOut);
						
					}

					if (clock () - m_clkLastTime < (int) uTimeOut)
					{
						return TRUE;
					}
				}
			}
			
			m_pLastTargetBar = NULL;
			m_clkLastTime = clock ();
			m_statusLast = BCGP_CS_NOTHING;
			pBCGMiniFrame->KillDockTimer ();

			if (status == BCGP_CS_DOCK_IMMEDIATELY && pTargetBar != NULL)		
			{
				// in the case docking was delayed we need always turn off predock state
				// (usually it happens only for resizable control bars)
				pBCGMiniFrame->SetPreDockState (BCGP_PDS_NOTHING);
				if (pBar->DockByMouse (pTargetBar))
				{
					return FALSE;
				}
			}
			
			if (status == BCGP_CS_DELAY_DOCK && !bSDockingIsOn) // status returned by resizable control bar
			{
				bResult = pBCGMiniFrame->SetPreDockState (BCGP_PDS_DOCK_REGULAR, pTargetBar);
			}
			else if (status == BCGP_CS_DELAY_DOCK_TO_TAB && !bSDockingIsOn)
			{
				bResult = pBCGMiniFrame->SetPreDockState (BCGP_PDS_DOCK_TO_TAB, pTargetBar);
				AdjustDockingLayout ();
			}
			else
			{
				bResult = pBCGMiniFrame->SetPreDockState (BCGP_PDS_NOTHING, pTargetBar);
			}
		}
	}
	return bResult;
}
//------------------------------------------------------------------------//
// used for autohide to prevent wrong z-order when auto hide window is sliding
// in (collapsed)
//------------------------------------------------------------------------//
void CBCGPDockManager::BringBarsToTop (DWORD dwAlignment, BOOL bExcludeDockedBars)
{
	dwAlignment &= CBRS_ALIGN_ANY;

	for (POSITION pos = m_lstControlBars.GetTailPosition (); pos != NULL;)	
	{
		CBCGPBaseControlBar* pBar = 
			(CBCGPBaseControlBar*) m_lstControlBars.GetPrev (pos);
		ASSERT_VALID (pBar);

		if (bExcludeDockedBars && 
			(pBar->IsKindOf (RUNTIME_CLASS (CBCGPControlBar)) ||
			 pBar->IsKindOf (RUNTIME_CLASS (CBCGPSlider))))
		{
			continue;
		}

		// starting from first dockbar do not exclude anything (so, th stattus bar 
		// and so on will be on top)
		bExcludeDockedBars = FALSE;	

		DWORD dwCurrAlignment = pBar->GetCurrentAlignment ();

		if (dwCurrAlignment == dwAlignment || dwAlignment == 0)
		{
			pBar->BringWindowToTop ();
		}
	}
}
//------------------------------------------------------------------------//
void CBCGPDockManager::SetAutohideZOrder (CBCGPDockingControlBar* pAHDockingBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pAHDockingBar);


	DWORD dwAlignment = pAHDockingBar->GetCurrentAlignment ();
	CBCGPSlider* pAHSlider = pAHDockingBar->GetDefaultSlider ();

	for (POSITION pos = m_lstControlBars.GetTailPosition (); pos != NULL;)	
	{
		CBCGPBaseControlBar* pBar = 
			(CBCGPBaseControlBar*) m_lstControlBars.GetPrev (pos);
		ASSERT_VALID (pBar);
		
		if (pBar == pAHSlider || pBar == pAHDockingBar)
		{
			continue;
		}

		if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPControlBar)) && 
			(pBar->GetCurrentAlignment () == dwAlignment)) 
			
		{
			pBar->SetWindowPos (pAHDockingBar, 0, 0, 0, 0, 
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);						
		}
		else if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPSlider)))
		{
			pBar->SetWindowPos (&CWnd::wndBottom, 0, 0, 0, 0, 
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);						
		}
	}

	pAHDockingBar->SetWindowPos (pAHSlider, 0, 0, 0, 0, 
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

}
//------------------------------------------------------------------------//
void CBCGPDockManager::RecalcLayout (BOOL /*bNotify*/)
{
	if (m_bDisableRecalcLayout)
	{
		return;
	}

	if (m_bRecalcLayout || m_bSizeFrame)
	{
		return;
	}

	if (!m_bEnableAdjustLayout)
	{
		return;
	}

	m_bRecalcLayout = TRUE;

	if (!IsOLEContainerMode ())
	{
		POSITION pos = NULL;

		for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPBaseControlBar* pNextControlBar = 
				(CBCGPBaseControlBar*) m_lstControlBars.GetNext (pos);
			ASSERT_VALID (pNextControlBar);
			pNextControlBar->AdjustLayout ();
		}

		for (pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
		{
			CBCGPMiniFrameWnd* pNextMiniFrame = 
				(CBCGPMiniFrameWnd*) m_lstMiniFrames.GetNext (pos);
			ASSERT_VALID (pNextMiniFrame);
			pNextMiniFrame->AdjustLayout ();
		}
	}
	else
	{
		for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPBaseControlBar* pNextControlBar = 
				(CBCGPBaseControlBar*) m_lstControlBars.GetNext (pos);
			ASSERT_VALID (pNextControlBar);
			if (pNextControlBar->IsBarVisible ())
			{
				pNextControlBar->AdjustLayout ();
			}
		}
	}

	AdjustDockingLayout ();
	m_bRecalcLayout = FALSE;
}
//------------------------------------------------------------------------//
void CBCGPDockManager::AdjustBarFrames ()
{
	ASSERT_VALID (this);

	UINT uiSWPFlags =	SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | 
						SWP_NOACTIVATE | SWP_FRAMECHANGED;

	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pNextControlBar = 
			(CBCGPBaseControlBar*) m_lstControlBars.GetNext (pos);
		ASSERT_VALID (pNextControlBar);

		pNextControlBar->SetWindowPos (NULL, -1, -1, -1, -1, uiSWPFlags);
	}

	for (pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
	{
		CBCGPMiniFrameWnd* pNextMiniFrame = 
			(CBCGPMiniFrameWnd*) m_lstMiniFrames.GetNext (pos);
		ASSERT_VALID (pNextMiniFrame);

		pNextMiniFrame->SetWindowPos (NULL, -1, -1, -1, -1, uiSWPFlags);
		pNextMiniFrame->AdjustBarFrames ();
	}
}
//------------------------------------------------------------------------//
void CBCGPDockManager::AdjustDockingLayout (HDWP hdwp)
{
	ASSERT_VALID (this);

	if (m_bDisableRecalcLayout)
	{
		return;
	}

	if (m_bAdjustingBarLayout)
	{
		return;
	}

	if (m_pParentWnd == NULL)
	{
		return;
	}

	m_pParentWnd->GetClientRect (m_rectClientAreaBounds);

	if (!m_rectInPlace.IsRectEmpty ())
	{
		m_rectClientAreaBounds = m_rectInPlace;
	}

	if (!m_bEnableAdjustLayout)
	{
		return;
	}

	if (m_lstControlBars.IsEmpty ())
	{
		return;
	}

	if (BCGCBProGetTopLevelFrame (m_pParentWnd) != NULL && 
		BCGCBProGetTopLevelFrame (m_pParentWnd)->IsIconic ())
	
	{
		return;
	}

	m_bAdjustingBarLayout = TRUE;

	CRect rectSaveOuterEdgeBounds = m_rectOuterEdgeBounds;

	BOOL bDeferWindowPosHere = FALSE;
	if (hdwp == NULL && !m_bIsPrintPreviewMode)
	{
		hdwp = BeginDeferWindowPos ((int) m_lstControlBars.GetCount ());
		bDeferWindowPosHere = TRUE;
	}

	CRect rectCurrBounds = m_rectDockBarBounds;

	// temporary - may want to enable feature of independent bounds
	m_pParentWnd->GetClientRect (rectCurrBounds);
	if (!m_rectInPlace.IsRectEmpty ())
	{
		rectCurrBounds = m_rectInPlace;
	}
	m_pParentWnd->ClientToScreen (rectCurrBounds);

	CRect rectControlBar;
	POSITION posLastDockBar = NULL;
	
	// find position of the last dock bar in the list (actually, it will be position
	// of the next control bar right after the last dock bar in the list)

	for (posLastDockBar = m_lstControlBars.GetTailPosition (); 
		 posLastDockBar != NULL;)
	{
		CBCGPBaseControlBar* pDockBar = 
			(CBCGPBaseControlBar*) m_lstControlBars.GetPrev (posLastDockBar);
		if (posLastDockBar == NULL)
		{
			break;
		}

		if (pDockBar->IsKindOf (RUNTIME_CLASS (CBCGPDockBar)) ||
			pDockBar->IsKindOf (RUNTIME_CLASS (CBCGPAutoHideDockBar)))
		{
			m_lstControlBars.GetNext (posLastDockBar);
			if (posLastDockBar != NULL)
			{
				m_lstControlBars.GetNext (posLastDockBar);
			}
			break;
		}
	}

	POSITION posBar = NULL;
	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		posBar = pos;

		CBCGPBaseControlBar* pNextControlBar = 
			(CBCGPBaseControlBar*) m_lstControlBars.GetNext (pos);
		ASSERT_VALID (pNextControlBar);


		if (!pNextControlBar->IsBarVisible() && 
			(pNextControlBar->IsKindOf (RUNTIME_CLASS (CBCGPControlBar)) ||
			pNextControlBar->IsKindOf (RUNTIME_CLASS (CBCGPSlider)) ||
			pNextControlBar->IsKindOf (RUNTIME_CLASS (CBCGPDockBar)) && 
			(m_bIsPrintPreviewMode || IsOLEContainerMode () || m_bHiddenForOLE)))
		{
			continue;
		}
		
		// let's see whether this control bar has enough space to be displayed,
		// has to be aligned differntly and so on. 

		pNextControlBar->GetWindowRect (rectControlBar);
		CRect rectSave = rectControlBar;
		
		DWORD dwAlignment = pNextControlBar->GetCurrentAlignment ();
		BOOL  bHorizontal = pNextControlBar->IsHorizontal ();
		BOOL  bResizable  = pNextControlBar->IsResizable ();

		if (pNextControlBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)))
		{
			CBCGPDockingControlBar* pDockingControlBar = 
				DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pNextControlBar);
			if (pDockingControlBar->GetDefaultSlider () != NULL)
			{
				// resizable control bars with sliders will be aligned by slider itself!!!
				continue;
			}
		}

		CSize sizeRequered = pNextControlBar->CalcFixedLayout (FALSE, bHorizontal);

		if (bHorizontal)
		{
			dwAlignment & CBRS_ALIGN_TOP ? rectControlBar.bottom = rectControlBar.top + sizeRequered.cy : 
										   rectControlBar.top = rectControlBar.bottom - sizeRequered.cy;
		}
		else
		{
			dwAlignment & CBRS_ALIGN_LEFT ? rectControlBar.right = rectControlBar.left + sizeRequered.cx : 
											rectControlBar.left = rectControlBar.right - sizeRequered.cx;
		}

		
		AlignByRect (rectCurrBounds, rectControlBar, dwAlignment, bHorizontal, bResizable);	
		
	
		CRect rectControlBarScreen = rectControlBar;

		ASSERT_VALID (pNextControlBar->GetParent ());
		if (pNextControlBar->IsKindOf (RUNTIME_CLASS (CBCGPDockBar)))
		{
			pNextControlBar->ScreenToClient (rectControlBar);
			if (pNextControlBar->IsHorizontal () && rectControlBar.Width () > 0 ||
				!pNextControlBar->IsHorizontal () && rectControlBar.Height () > 0)
			{
				((CBCGPDockBar*) pNextControlBar)->RepositionBars (rectControlBar);
			}

			rectControlBar = rectControlBarScreen;
		}

		if (pNextControlBar->IsKindOf (RUNTIME_CLASS (CBCGPSlider)))
		{
			// the slider will change its position, as well as position of
			// its resizable control bars (container)
			((CBCGPSlider*) pNextControlBar)->RepositionBars (rectControlBar, hdwp);			
		}
		else
		{
			pNextControlBar->GetParent ()->ScreenToClient (rectControlBar);
			hdwp = pNextControlBar->SetWindowPos (NULL, rectControlBar.left, 
						rectControlBar.top, rectControlBar.Width (), rectControlBar.Height (), 
						SWP_NOZORDER | SWP_NOACTIVATE, hdwp);
		}

		if (dwAlignment & CBRS_ALIGN_TOP)
		{
			rectCurrBounds.top += rectControlBarScreen.Height ();
		}
		else if (dwAlignment & CBRS_ALIGN_BOTTOM)
		{
			rectCurrBounds.bottom -= rectControlBarScreen.Height ();
		}
		else if (dwAlignment & CBRS_ALIGN_LEFT)
		{
			rectCurrBounds.left += rectControlBarScreen.Width ();
		}
		else 
		{
			rectCurrBounds.right -= rectControlBarScreen.Width ();
		}

		if (posLastDockBar == pos)
		{
			m_rectOuterEdgeBounds = rectCurrBounds;
		}

	}

	m_rectClientAreaBounds = rectCurrBounds;

	if (m_rectOuterEdgeBounds.IsRectEmpty () || IsOLEContainerMode ())
	{
		m_rectOuterEdgeBounds = rectCurrBounds;
	}

	

	m_pParentWnd->ScreenToClient (m_rectClientAreaBounds);
	m_pParentWnd->ScreenToClient (m_rectOuterEdgeBounds);

	if (m_rectOuterEdgeBounds != rectSaveOuterEdgeBounds)
	{
		HideAutoHideBars (NULL, TRUE);
	}

	// special processing for autohide dock bars
	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pBar = (CBCGPBaseControlBar*) m_lstControlBars.GetNext (pos);
		ASSERT_VALID (pBar);
		if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPAutoHideDockBar)))
		{
			CBCGPAutoHideDockBar* pSlidingBar = (CBCGPAutoHideDockBar*) pBar;
			pSlidingBar->SetOffsetLeft (0);
			pSlidingBar->SetOffsetRight (0);
			CalcBarOffset ((CBCGPAutoHideDockBar*) pBar);
		}
	}

	// redraw all right-side dock bars:
	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pBar = (CBCGPBaseControlBar*) m_lstControlBars.GetNext (pos);
		ASSERT_VALID (pBar);

		if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPDockBar)) &&
			pBar->GetCurrentAlignment () == CBRS_ALIGN_RIGHT)
		{
			pBar->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
	}

	if (bDeferWindowPosHere)
	{
		EndDeferWindowPos (hdwp);
	}

	if (m_pParentWnd->m_pNotifyHook != NULL)
	{
		m_pParentWnd->RecalcLayout ();
	}

	m_bAdjustingBarLayout = FALSE;
}
//----------------------------------------------------------------------------------//
void CBCGPDockManager::CalcBarOffset (CBCGPAutoHideDockBar* pBar)
{
	ASSERT_VALID (pBar);
	DWORD dwBarAlignOrg = pBar->GetCurrentAlignment ();
	CRect rectBarOrg;
	pBar->GetWindowRect (rectBarOrg);	
	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pBarNext = (CBCGPBaseControlBar*) m_lstControlBars.GetNext (pos);
		ASSERT_VALID (pBarNext);
		if (pBarNext->IsKindOf (RUNTIME_CLASS (CBCGPAutoHideDockBar)))
		{
			CBCGPAutoHideDockBar* pNextSlidingDockBar = 
				DYNAMIC_DOWNCAST (CBCGPAutoHideDockBar, pBarNext);

			if (pNextSlidingDockBar == pBar ||
				pNextSlidingDockBar->IsHorizontal () && pBar->IsHorizontal () ||
				!pNextSlidingDockBar->IsHorizontal () && !pBar->IsHorizontal ())
			{
				continue;
			}

			CRect rectBarNext;
			pNextSlidingDockBar->GetWindowRect (rectBarNext);
			if (rectBarNext.IsRectEmpty ())
			{
				continue;
			}

			DWORD dwBarAlignNext = pNextSlidingDockBar->GetCurrentAlignment ();
			
			if (dwBarAlignOrg & CBRS_ALIGN_LEFT && 
				dwBarAlignNext & CBRS_ALIGN_TOP)
			{
				if (rectBarOrg.top == rectBarNext.bottom)
				{
					pNextSlidingDockBar->SetOffsetLeft (rectBarOrg.Width ());
				}

				if (rectBarOrg.right == rectBarNext.left)
				{
					pBar->SetOffsetLeft (rectBarNext.Height ());
				}
			}
			else if (dwBarAlignOrg & CBRS_ALIGN_TOP && 
					 dwBarAlignNext & CBRS_ALIGN_RIGHT)
			{
				if (rectBarOrg.right == rectBarNext.left)
				{
					pNextSlidingDockBar->SetOffsetLeft (rectBarOrg.Height ());
				}

				if (rectBarOrg.bottom == rectBarNext.top)
				{
					pBar->SetOffsetRight (rectBarNext.Width ());
				}
			}
			else if (dwBarAlignOrg & CBRS_ALIGN_RIGHT && 
					 dwBarAlignNext & CBRS_ALIGN_BOTTOM)
			{
				if (rectBarOrg.bottom == rectBarNext.top)
				{
					pNextSlidingDockBar->SetOffsetRight (rectBarOrg.Width ());
				}

				if (rectBarOrg.left == rectBarNext.right)
				{
					pBar->SetOffsetRight (rectBarOrg.Width ());
				}
			}
			else if (dwBarAlignOrg & CBRS_ALIGN_LEFT && 
					 dwBarAlignNext & CBRS_ALIGN_BOTTOM)
			{
				if (rectBarOrg.bottom == rectBarNext.top)
				{
					pNextSlidingDockBar->SetOffsetLeft (rectBarOrg.Width ());
				}

				if (rectBarOrg.right == rectBarNext.left)
				{
					pBar->SetOffsetRight (rectBarNext.Height ());
				}
			}
		}
	}
} 
//----------------------------------------------------------------------------------//
void CBCGPDockManager::AlignByRect (const CRect& rectToAlignBy, 
										CRect& rectResult, DWORD dwAlignment, 
										BOOL bHorizontal, BOOL bResizable)
{
	ASSERT_VALID (this);

	int nCurrWidth = rectResult.Width ();
	int nCurrHeight = rectResult.Height ();


	DWORD dwCurrAlignment = dwAlignment & CBRS_ALIGN_ANY;
	switch (dwCurrAlignment)
	{
	case CBRS_ALIGN_LEFT:
		rectResult.TopLeft () = rectToAlignBy.TopLeft ();
		rectResult.bottom = rectResult.top + rectToAlignBy.Height ();
		rectResult.right = rectResult.left + nCurrWidth;
		break;

	case CBRS_ALIGN_TOP:
		rectResult.TopLeft () = rectToAlignBy.TopLeft ();
		rectResult.right = rectResult.left + rectToAlignBy.Width ();
		rectResult.bottom = rectResult.top + nCurrHeight;
		break;

	case CBRS_ALIGN_RIGHT:
		rectResult.BottomRight () = rectToAlignBy.BottomRight ();
		rectResult.top = rectResult.bottom - rectToAlignBy.Height ();
		rectResult.left = rectResult.right - nCurrWidth;
		break;
	case CBRS_ALIGN_BOTTOM:
		rectResult.BottomRight () = rectToAlignBy.BottomRight ();
		rectResult.left = rectResult.right - rectToAlignBy.Width ();
		rectResult.top = rectResult.bottom - nCurrHeight;	
		break;
	}

	
	if (bHorizontal)
	{		
		int nDelta = rectResult.Width () - rectToAlignBy.Width ();
		if (nDelta != 0)
		{
			rectResult.right += nDelta;
		}

		nDelta = rectResult.Height () - rectToAlignBy.Height ();
		if (nDelta > 0 && bResizable)
		{
			if (dwCurrAlignment & CBRS_ALIGN_TOP)
			{
				rectResult.bottom -= nDelta;
			}
			else if (dwCurrAlignment & CBRS_ALIGN_BOTTOM)
			{
				rectResult.top += nDelta;
			}
		}
	}
	else
	{
		int nDelta = rectResult.Height () - rectToAlignBy.Height ();
		if (nDelta != 0)
		{
			rectResult.bottom += nDelta;
		}

		nDelta = rectResult.Width () - rectToAlignBy.Width ();
		if (rectResult.Width () > rectToAlignBy.Width () && bResizable)
		{
			if (dwCurrAlignment & CBRS_ALIGN_LEFT)
			{
				rectResult.right -= nDelta;
			}
			else if (dwCurrAlignment & CBRS_ALIGN_RIGHT)
			{
				rectResult.left += nDelta;
			}
		}
	}
}
//----------------------------------------------------------------------------------//
BOOL CBCGPDockManager::SaveState (LPCTSTR lpszProfileName, UINT uiID)
{
	ASSERT_VALID (this);

	m_bSavingState = TRUE;

	CString strProfileName = ::BCGPGetRegPath (strDockManagerProfile, lpszProfileName);

	BOOL bResult = FALSE;


	CString strSection;
	strSection.Format (REG_SECTION_FMT, strProfileName, uiID);

	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pBarNext = (CBCGPBaseControlBar*) m_lstControlBars.GetNext (pos);
		ASSERT_VALID (pBarNext);

		if (pBarNext->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)) ||
			pBarNext->IsKindOf (RUNTIME_CLASS (CBCGPControlBar)) && 
			!pBarNext->IsKindOf (RUNTIME_CLASS (CBCGPToolBar)))
		{
			pBarNext->SaveState (lpszProfileName);
		}
	}

	for (pos = m_lstAutoHideBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPSlider* pSlider = DYNAMIC_DOWNCAST (CBCGPSlider, m_lstControlBars.GetNext (pos));

		if (pSlider != NULL && pSlider->IsDefault ())
		{
			CObList lstBars;
			CBCGPDockingControlBar* pNextBar = (CBCGPDockingControlBar*) pSlider->GetFirstBar ();
			if (pNextBar != NULL)
			{
				pNextBar->SaveState (lpszProfileName);
			}
		}
	}

	for (pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
	{
		CBCGPMiniFrameWnd* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, m_lstMiniFrames.GetNext (pos));

		ASSERT_VALID (pWnd);

		pWnd->SaveState (lpszProfileName);
	}

	try
	{
		CMemFile file;
		{
			CArchive ar (&file, CArchive::store);

			Serialize (ar);
			ar.Flush ();
		}

		UINT uiDataSize = (UINT) file.GetLength ();
		LPBYTE lpbData = file.Detach ();

		if (lpbData != NULL)
		{
			CBCGPRegistrySP regSP;
			CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

			if (reg.CreateKey (strSection))
			{
				bResult = reg.Write (REG_ENTRY_DOCKING_CB_AND_SLIDERS, lpbData, uiDataSize);
			}

			free (lpbData);
		}
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T ("Memory exception in CBCGPDockManager::SaveState ()!\n"));
	}

	m_bSavingState = FALSE;
	return bResult;
}
//----------------------------------------------------------------------------------//
BOOL CBCGPDockManager::LoadState (LPCTSTR lpszProfileName, UINT uiID)
{
	ASSERT_VALID (this);

	CString strProfileName = ::BCGPGetRegPath (strDockManagerProfile, lpszProfileName);

	BOOL bResult = FALSE;

	CString strSection;
	strSection.Format (REG_SECTION_FMT, strProfileName, uiID);


	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pBarNext = (CBCGPBaseControlBar*) m_lstControlBars.GetNext (pos);
		ASSERT_VALID (pBarNext);

		if (pBarNext->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)) ||
			pBarNext->IsKindOf (RUNTIME_CLASS (CBCGPControlBar)) && 
			!pBarNext->IsKindOf (RUNTIME_CLASS (CBCGPToolBar)))
		{
			pBarNext->LoadState (lpszProfileName);
		}
	}

	for (pos = m_lstAutoHideBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pBar = (CBCGPBaseControlBar*) 
										m_lstAutoHideBars.GetNext (pos);

		ASSERT_VALID (pBar);
		if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPSlider)))
		{
			CBCGPSlider* pSlider = DYNAMIC_DOWNCAST (CBCGPSlider, pBar);
			// SLIDER CONTAINS ONLY ONE BAR IN AUTOHIDE MODE
			pBar = (CBCGPBaseControlBar*) pSlider->GetFirstBar ();
			if (pBar != NULL && pBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)))
			{
				pBar->LoadState (lpszProfileName);
			}
		}
	}
	

	for (pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
	{
		CBCGPMiniFrameWnd* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, m_lstMiniFrames.GetNext (pos));

		ASSERT_VALID (pWnd);

		pWnd->LoadState (lpszProfileName);
	}

	LPBYTE	lpbData = NULL;
	UINT	uiDataSize;

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	if (!reg.Read (REG_ENTRY_DOCKING_CB_AND_SLIDERS, &lpbData, &uiDataSize))
	{
		return FALSE;
	}

	try
	{
		CMemFile file (lpbData, uiDataSize);
		CArchive ar (&file, CArchive::load);

		Serialize (ar);
		bResult = TRUE;
		m_bDisableSetDockState = FALSE;
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T ("Memory exception in CBCGPDockManager::LoadState!\n"));
		m_bDisableSetDockState = TRUE;
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T ("CArchiveException exception in CBCGPDockManager::LoadState ()!\n"));

		// destroy loaded sliders (docking control bars are loaded by ID's and have
		// been already created by application
		
		for (POSITION pos = m_lstLoadedBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPBaseControlBar* pNextBar = 
				(CBCGPBaseControlBar*) m_lstLoadedBars.GetNext (pos);
			if (pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPSlider)))
			{
				pNextBar->DestroyWindow ();
			}
			else
			{
				pNextBar->SetRestoredFromRegistry (FALSE);
			}
		}
		
		m_lstLoadedBars.RemoveAll ();
		m_bDisableSetDockState = TRUE;
	}
	catch (...)
	{
		// destroy loaded sliders (docking control bars are loaded by ID's and have
		// been already created by application
		
		for (POSITION pos = m_lstLoadedBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPBaseControlBar* pNextBar = 
				(CBCGPBaseControlBar*) m_lstLoadedBars.GetNext (pos);
			if (pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPSlider)))
			{
				pNextBar->DestroyWindow ();
			}
			else
			{
				pNextBar->SetRestoredFromRegistry (FALSE);
			}
		}
		
		m_lstLoadedBars.RemoveAll ();
		m_bDisableSetDockState = TRUE;
	}

	if (lpbData != NULL)
	{
		delete [] lpbData;
	}

	return bResult;

}
//----------------------------------------------------------------------------------//
void CBCGPDockManager::Serialize (CArchive& ar)
{
	// calculate or load the number of docking control bars and sliders
	int nCBCount = 0;
	int nNonFloatingBarCount = 0;

	if (ar.IsStoring ())
	{
		POSITION pos = NULL;

		// get rid on non-valid empty tabbed bars
		for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPBaseTabbedBar* pNextBar = 
				DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, m_lstControlBars.GetAt (pos));

			if (pNextBar != NULL && pNextBar->GetTabsNum () == 0 && pNextBar->CanFloat ())
			{
				m_lstControlBars.GetPrev (pos);
				pNextBar->UnDockControlBar (TRUE);
				if (pos == NULL)
				{
					pos = m_lstControlBars.GetHeadPosition ();
				}
			}
			if (pos != NULL)
			{
				m_lstControlBars.GetNext (pos);
			}
		}

		for (pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
		{
			CBCGPMiniFrameWnd* pNextMiniFrame = 
				DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, m_lstMiniFrames.GetNext (pos));

			if (pNextMiniFrame != NULL)
			{
				pNextMiniFrame->RemoveNonValidBars ();
			}
		}

		for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPBaseControlBar* pNextBar = 
				(CBCGPBaseControlBar*) m_lstControlBars.GetNext (pos);

			if (pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)) && 
				pNextBar->CanFloat () ||
				pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPSlider)) && 
				((CBCGPSlider*) pNextBar)->DoesContainFloatingBar ())
			{
				nCBCount++;
			}
			else
			{
				// static bar that may contain detachable/floating tabs
				if (pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPBaseTabbedBar)))
				{
					nNonFloatingBarCount++;
				}
			}
		}

		ar << nNonFloatingBarCount;

		for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPBaseControlBar* pNextBar = 
				(CBCGPBaseControlBar*) m_lstControlBars.GetNext (pos);
			if (!pNextBar->CanFloat () && 
				pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPBaseTabbedBar)))
			{
				CBCGPBaseTabbedBar* pTabbedBar = 
					DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pNextBar);
				if (pTabbedBar != NULL)
				{
					ASSERT_VALID (pTabbedBar->GetUnderlinedWindow ());
					ar << pTabbedBar->GetDlgCtrlID ();
					pTabbedBar->GetUnderlinedWindow ()->Serialize (ar);
				}
			}
		}

		ar << nCBCount;

		// START from the tail, so the sliders and embedded containers
		// will be stored bedore their control bars

		for (pos = m_lstControlBars.GetTailPosition (); pos != NULL;)
		{
			CBCGPBaseControlBar* pNextBar = 
				(CBCGPBaseControlBar*) m_lstControlBars.GetPrev (pos);
			ASSERT_VALID (pNextBar);

			if (pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)) && 
				pNextBar->CanFloat ())
			{
				int nBarID = pNextBar->GetDlgCtrlID ();
				// write docking control bar tag and ID
				

				if (nBarID != -1)
				{
					ar << TRUE;
					ar << nBarID;
				}
				else 
				{
					// this is tab control bar - write its tabbed bar ids
					CBCGPBaseTabbedBar* pTabBar = 
						DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pNextBar);
					ASSERT_VALID (pTabBar);

					// use first child bar as identifier of the tab control bar
					CWnd* pWnd  = pTabBar->FindBarByTabNumber (0);
				
					// if pWnd is NULL - write nothing! because we do not allow empty tabbed 
					// bars
					if (pWnd != NULL)
					{
						int nTabbedBarID = pWnd->GetDlgCtrlID ();
						ASSERT (nTabbedBarID != -1);
						
						ar << TRUE;
						ar << nBarID; 
						ar << nTabbedBarID;
					}
				}
				continue;
			}
			else if (pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPSlider))
					&& ((CBCGPSlider*) pNextBar)->DoesContainFloatingBar ())
			{
				// write slider tag and serialize the slider
				ar << FALSE;

				CBCGPSlider* pSlider = DYNAMIC_DOWNCAST (CBCGPSlider, pNextBar);
				ASSERT_VALID (pSlider);

				pSlider->Serialize (ar);
			}
		}

		int nCountMiniFrames = 0;

		for (pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
		{
			CBCGPMiniFrameWnd* pWnd = 
				DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, m_lstMiniFrames.GetNext (pos));
			ASSERT_VALID (pWnd);

			if (pWnd->GetControlBarCount () > 0)
			{
				nCountMiniFrames++;
			}
		}

		ar << nCountMiniFrames;

		for (pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
		{
			CBCGPMiniFrameWnd* pWnd = 
				DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, m_lstMiniFrames.GetNext (pos));
			ASSERT_VALID (pWnd);
			if (pWnd->GetControlBarCount () > 0)
			{
				ar << pWnd;
			}
		}

		// serialize autohide bars
	
		ar << (int) m_lstAutoHideBars.GetCount ();

		for (pos = m_lstAutoHideBars.GetHeadPosition (); pos != NULL;)
		{
			BCGP_AUTOHIDEBAR_SAVE_INFO ahSaveInfo;

			CBCGPSlider* pSlider = 
				DYNAMIC_DOWNCAST (CBCGPSlider, m_lstAutoHideBars.GetNext (pos));

			if (pSlider == NULL)
			{
				ASSERT (FALSE);
				continue;
			}

			ahSaveInfo.m_pSavedBar = 
				DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pSlider->GetFirstBar ());

			if (ahSaveInfo.m_pSavedBar != NULL)
			{
				ahSaveInfo.Serilaize (ar);
			}
		}

		// serialize MDI Tabbed Bars
		ar << (int) m_lstHiddenMDITabbedBars.GetCount ();
		
		for (pos = m_lstHiddenMDITabbedBars.GetHeadPosition (); pos != NULL;)
		{
			HWND hwnd = m_lstHiddenMDITabbedBars.GetNext (pos);
			CBCGPDockingControlBar* pNextBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
				CWnd::FromHandlePermanent (hwnd));
			if (pNextBar != NULL)
			{
				ar << (int) pNextBar->GetDlgCtrlID ();
			}
			else
			{
				ar << (int) -1;
			}
		}
	}
	else
	{
		m_lstLoadedBars.RemoveAll ();
		m_lstNonFloatingBars.RemoveAll ();
		m_lstLoadedAutoHideBarIDs.RemoveAll ();
		m_lstLoadedMiniFrames.RemoveAll ();

		UINT nBarID = (UINT) -1;

		CList<UINT, UINT&> lstNotFoundBars;

		ar >> nNonFloatingBarCount;

		int i = 0;

		for (i = 0; i < nNonFloatingBarCount; i++)
		{
			ar >> nBarID;
			CBCGPBaseTabbedBar* pBar = DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, FindBarByID (nBarID, TRUE));
			if (pBar != NULL)
			{
				pBar->GetUnderlinedWindow ()->Serialize (ar);
				m_lstNonFloatingBars.AddTail (pBar);
			}
		}

		ar >> nCBCount;
		
		BOOL bIsDockingControlBar = FALSE;
		

		CBCGPSlider* pCurrentDefaultSlider = NULL;

		// the list was stored from the tail (to store sliders first)
		// therefore we need to add head
		for (i = 0; i < nCBCount; i++)
		{
			ar >> bIsDockingControlBar;

			if (bIsDockingControlBar)
			{
				ar >> nBarID;
				
				CBCGPDockingControlBar* pBar = NULL;
				if (nBarID != -1)
				{
					pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, FindBarByID (nBarID, TRUE));
				}
				else
				{
					// tab docking bar - load first child bar
					ar >> nBarID; 

					if (pCurrentDefaultSlider != NULL)
					{
						pBar = pCurrentDefaultSlider->FindTabbedBar (nBarID);
					}
				}

				if (pBar != NULL)
				{
					ASSERT_VALID (pBar);

					if (pBar->IsAutoHideMode ())
					{
						pBar->SetAutoHideMode (FALSE, CBRS_ALIGN_ANY);
					}

					if (pCurrentDefaultSlider != NULL)
					{
						ASSERT_VALID(pCurrentDefaultSlider);

						pBar->SetRestoredDefaultSlider (pCurrentDefaultSlider->m_hWnd);
						pBar->SetBarAlignment (pCurrentDefaultSlider->GetCurrentAlignment ());
					}

					m_lstLoadedBars.AddHead (pBar);
				}
				else
				{
					lstNotFoundBars.AddTail (nBarID);
				}
			}
			else
			{
				pCurrentDefaultSlider = DYNAMIC_DOWNCAST (CBCGPSlider, CBCGPSlider::m_pSliderRTC->CreateObject ());
				ASSERT_VALID (pCurrentDefaultSlider);

				pCurrentDefaultSlider->Init (TRUE, m_pParentWnd);
				pCurrentDefaultSlider->Serialize (ar);
				
				m_lstLoadedBars.AddHead (pCurrentDefaultSlider);

				POSITION posSave = NULL;
				for (POSITION posNotFound = lstNotFoundBars.GetHeadPosition (); 
					 posNotFound != NULL;)
				{
					posSave = posNotFound;
					UINT nNextBarID = lstNotFoundBars.GetNext (posNotFound);
					CBCGPDockingControlBar* pNextBar = 
						pCurrentDefaultSlider->FindTabbedBar (nNextBarID);
					if (pNextBar != NULL)
					{
						ASSERT_VALID (pNextBar);

						if (pNextBar->IsAutoHideMode ())
						{
							pNextBar->SetAutoHideMode (FALSE, CBRS_ALIGN_ANY);
						}

						pNextBar->SetRestoredDefaultSlider (pCurrentDefaultSlider->m_hWnd);
						pNextBar->SetBarAlignment (pCurrentDefaultSlider->GetCurrentAlignment ());
						m_lstLoadedBars.AddHead (pNextBar);
						lstNotFoundBars.RemoveAt (posSave);
					}
				}
			}
		}

		int nMiniFrameCount = 0;

		ar >> nMiniFrameCount;

		for (i = 0; i < nMiniFrameCount; i++)
		{
			CBCGPMiniFrameWnd* pWnd = NULL;
			CBCGPMiniFrameWnd::m_pParentWndForSerialize = m_pParentWnd;
			ar >> pWnd;
			m_lstLoadedMiniFrames.AddTail (pWnd);
		}

		int nAHBarCount = 0;
		ar >> nAHBarCount;

		for (i = 0; i < nAHBarCount; i++)
		{
			BCGP_AUTOHIDEBAR_SAVE_INFO info;
			info.Serilaize (ar);
			m_lstLoadedAutoHideBarIDs.AddTail (info);
		}

		if (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x90200)
		{
			int nMdiTabbedBarsCount = 0;
			ar >> nMdiTabbedBarsCount;
			for (int i = 0; i < nMdiTabbedBarsCount; i++)
			{
				int nMDITabbedBarID = -1;
				ar >> nMDITabbedBarID;
				if (nMDITabbedBarID != -1)
				{
					CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
							FindBarByID (nMDITabbedBarID, TRUE));
					if (pBar != NULL)
					{
						if (pBar->IsFloating ())
						{
							CBCGPMiniFrameWnd* pMiniFrame = pBar->GetParentMiniFrame ();
							if (pMiniFrame != NULL)
							{
								pMiniFrame->RemoveControlBar (pBar);
							}

							pBar->SetParent (m_pParentWnd);
						}
						else if (pBar->IsAutoHideMode ())
						{
							pBar->SetAutoHideMode (FALSE, CBRS_ALIGN_ANY);
						}
						else if (pBar->IsTabbed ())
						{
							CBCGPBaseTabbedBar* pTabBar = pBar->GetParentTabbedBar ();	
							ASSERT_VALID (pTabBar);
							pBar->SetParent (m_pParentWnd);
							pTabBar->RemoveControlBar (pBar);
						}
						else
						{
							pBar->UnDockControlBar ();
						}
						pBar->ShowWindow (FALSE);
						AddHiddenMDITabbedBar (pBar);
						pBar->SetMDITabbed (TRUE);
					}
				}
			}
		}
	}
}
//----------------------------------------------------------------------------------//
void CBCGPDockManager::SetDockState ()
{
	if (m_bDisableSetDockState || m_bDisableRestoreDockState)
	{
		return;
	}

	if (m_lstLoadedBars.IsEmpty () && m_lstLoadedMiniFrames.IsEmpty () && 
		m_lstNonFloatingBars.IsEmpty () && m_lstLoadedAutoHideBarIDs.IsEmpty()  &&
		m_lstControlBars.IsEmpty ())
	{
		return;
	}

	m_bRestoringDockState = TRUE;

	m_bDisableRecalcLayout = TRUE;

	POSITION pos = NULL;
	CObList lstAutoHideBars;

	// set all autohide bars to the regular mode
	for (pos = m_lstAutoHideBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPSlider* pSlider = DYNAMIC_DOWNCAST (CBCGPSlider, m_lstAutoHideBars.GetNext (pos));
		if (pSlider != NULL)
		{
			CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
													pSlider->GetFirstBar ());
			if (pBar != NULL && pBar->GetAutoHideToolBar () != NULL && 
				pBar->GetAutoHideToolBar ()->m_bFirstInGroup)
			{
				lstAutoHideBars.AddTail (pBar);
			}
		}
	}

	for (pos = lstAutoHideBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
																lstAutoHideBars.GetNext (pos));
		pBar->SetAutoHideMode (FALSE, CBRS_ALIGN_ANY);
	}
	

	for (pos = m_lstLoadedBars.GetHeadPosition  (); pos != NULL;)
	{
		CBCGPDockingControlBar* pNextBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, m_lstLoadedBars.GetNext (pos));
		if (pNextBar != NULL)
		{
			pNextBar->RestoreDefaultSlider ();
		}
	}

	// set up miniframes - the original list may be modified by SetDockState
	CObList lstMiniFrames;
	lstMiniFrames.AddTail (&m_lstLoadedMiniFrames);

	for (pos = lstMiniFrames.GetHeadPosition (); pos != NULL;)
	{
		CBCGPMiniFrameWnd* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, lstMiniFrames.GetNext (pos));
		ASSERT_VALID (pWnd);

		pWnd->SetWindowPos (NULL, 0, 0, 0, 0, 
			SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		pWnd->SetDockState (this);
	}

	CObList lstAllBars;

	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pBarNext = (CBCGPBaseControlBar*) m_lstControlBars.GetNext (pos);
		ASSERT_VALID (pBarNext);

		if (pBarNext->IsKindOf (RUNTIME_CLASS (CBCGPDockBar)))
		{
			CBCGPDockBar* pDockBar = (CBCGPDockBar*)
				DYNAMIC_DOWNCAST (CBCGPDockBar, pBarNext);

			ASSERT_VALID (pDockBar);
			lstAllBars.AddTail ((CObList*)&pDockBar->GetControlBarList ());
		}
		else if (pBarNext->IsKindOf (RUNTIME_CLASS (CBCGPControlBar))) 
		{
			if (pBarNext->CanFloat ())
			{
				lstAllBars.AddTail (pBarNext);
			}
			else if (pBarNext->IsRestoredFromRegistry ())
			{
				// set the size of non-floating bar right now
				CRect rect = ((CBCGPControlBar*) pBarNext)->m_rectSavedDockedRect;
				pBarNext->SetWindowPos (NULL, rect.left, rect.top, rect.Width (), rect.Height (),
											SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
				pBarNext->ShowControlBar (pBarNext->GetRecentVisibleState (), TRUE, FALSE);
			}
		}
	}
	// take toolbars from all originally created miniframes
	for (pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
	{
		CBCGPMiniFrameWnd* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, m_lstMiniFrames.GetNext (pos));

		if (pWnd != NULL)
		{
			CBCGPBaseToolBar* pToolbar = 
				DYNAMIC_DOWNCAST (CBCGPBaseToolBar, pWnd->GetControlBar ());
			if (pToolbar != NULL)
			{
				lstAllBars.AddTail (pToolbar);
			}
		}
	}

	// we must float all bars first
	for (pos = lstAllBars.GetHeadPosition  (); pos != NULL;)
	{
		CBCGPControlBar* pNextBar = (CBCGPControlBar*) lstAllBars.GetNext (pos);
		ASSERT_VALID (pNextBar);

		if (pNextBar->IsRestoredFromRegistry ())
		{
			if (pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPBaseTabbedBar)))
			{
				// contained bars will be redocked later
				// the bar itself should be destroyed
			}
			else
			{
				pNextBar->FloatControlBar (pNextBar->m_recentDockInfo.m_rectRecentFloatingRect, BCGP_DM_SHOW, false);
			}
		}
	}

	// redock at recent rows regular control bars (toolbars and so on)
	for (pos = lstAllBars.GetHeadPosition  (); pos != NULL;)
	{
		CBCGPControlBar* pNextBar = (CBCGPControlBar*) lstAllBars.GetNext (pos);
		ASSERT_VALID (pNextBar);
	
		if (pNextBar->IsRestoredFromRegistry () && 
			!pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)))
		{
			pNextBar->SetDockState (this);	
			pNextBar->UpdateVirtualRect ();
		}
	}

	// add docking control bars and sliders (remove from miniframe first)
	for (pos = m_lstLoadedBars.GetHeadPosition  (); pos != NULL;)
	{
		CBCGPBaseControlBar* pNextBar = 
			(CBCGPBaseControlBar*) m_lstLoadedBars.GetNext (pos);
		ASSERT_VALID (this);

		if (pNextBar->IsTabbed ())
		{
			CBCGPBaseTabWnd* pTabWnd = (CBCGPBaseTabWnd*) pNextBar->GetParent ();
			CBCGPBaseTabbedBar* pTabBar = (CBCGPBaseTabbedBar*) pTabWnd->GetParent ();	
			ASSERT_VALID (pTabBar);
			pNextBar->SetParent (m_pParentWnd);
			pTabBar->RemoveControlBar (pNextBar);
			if (pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)))
			{
				((CBCGPDockingControlBar*) pNextBar)->EnableGripper (TRUE);
			}

			pNextBar->ShowWindow (SW_SHOW);
		}

		CBCGPMiniFrameWnd* pMiniFrame = pNextBar->GetParentMiniFrame ();
		if (pMiniFrame != NULL)
		{
			pMiniFrame->RemoveControlBar (pNextBar);
		}

		pNextBar->SetParent (m_pParentWnd);

		if (pNextBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)))
		{
			CBCGPDockingControlBar* pDockingBar = 
				DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pNextBar);
			CRect rect = pDockingBar->m_rectSavedDockedRect;

			pDockingBar->SetWindowPos (NULL, rect.left, rect.top, rect.Width (), rect.Height (),
										SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);

			BOOL bShow = pDockingBar->GetRecentVisibleState ();
			pDockingBar->ShowWindow (bShow ? SW_SHOWNORMAL : SW_HIDE);

			CBCGPSlider* pDefaultSlider = pDockingBar->GetDefaultSlider ();

			if (pDefaultSlider != NULL)
			{
				pDockingBar->SetBarAlignment (pDefaultSlider->GetCurrentAlignment ());
				pDefaultSlider->OnShowControlBar (pDockingBar, bShow);
			}

			if (pDockingBar->IsKindOf (RUNTIME_CLASS (CBCGPBaseTabbedBar)))
			{
				BOOL bLeftBar = FALSE;

				if (pDefaultSlider != NULL)
				{
					CBCGPBarContainer* pContainer = 
						pDefaultSlider->FindContainer (pDockingBar, bLeftBar);

					ASSERT (pContainer != NULL);

					if (pContainer == NULL)
					{
						continue;
					}

					CList<UINT, UINT>* pListBarIDs = 
						pContainer->GetAssociatedSiblingBarIDs (pDockingBar);

					
					if (pListBarIDs != NULL)
					{
						for (POSITION pos = pListBarIDs->GetHeadPosition (); pos != NULL;)
						{
							UINT nIDNext = pListBarIDs->GetNext (pos);
							CBCGPDockingControlBar* pBarToAttach = 
								DYNAMIC_DOWNCAST (CBCGPDockingControlBar, FindBarByID (nIDNext, TRUE));

							if (pBarToAttach != NULL)
							{
								if (pBarToAttach->IsTabbed ())
								{
									CBCGPBaseTabWnd* pTabWnd = 
											(CBCGPBaseTabWnd*) pBarToAttach->GetParent ();
									CBCGPBaseTabbedBar* pTabBar = (CBCGPBaseTabbedBar*)
											pTabWnd->GetParent ();	
									ASSERT_VALID (pTabBar);

									pBarToAttach->SetParent (m_pParentWnd);
									pTabBar->RemoveControlBar (pBarToAttach);
								}
								else if (pBarToAttach->IsAutoHideMode ())
								{
									pBarToAttach->SetAutoHideMode (FALSE, CBRS_ALIGN_ANY);
								}
								((CBCGPDockingControlBar*) pBarToAttach)->
									AttachToTabWnd (pDockingBar, BCGP_DM_UNKNOWN, FALSE);
							}
						}
					}
					if (((CBCGPBaseTabbedBar*)pDockingBar)->GetTabsNum () == 0)
					{
						continue;
					}
		
					((CBCGPBaseTabbedBar*)pDockingBar)->ApplyRestoredTabInfo ();
					pDockingBar->RecalcLayout ();
				}
			}
		}
	}

	m_lstControlBars.AddTail (&m_lstLoadedBars);

	m_bDisableRecalcLayout = FALSE; 
	AdjustDockingLayout ();

	for (pos = m_lstLoadedAutoHideBarIDs.GetHeadPosition (); pos != NULL;)
	{
		BCGP_AUTOHIDEBAR_SAVE_INFO& info = m_lstLoadedAutoHideBarIDs.GetNext (pos);
		CBCGPDockingControlBar* pBar = 
			DYNAMIC_DOWNCAST (CBCGPDockingControlBar, FindBarByID (info.m_nBarID, TRUE));
		if (pBar != NULL)
		{
			if (pBar->IsFloating ())
			{
				pBar->DockToFrameWindow (info.m_dwBarAlignment);
			}
			else if (pBar->IsAutoHideMode ())
			{
				pBar->SetAutoHideMode (FALSE, CBRS_ALIGN_ANY);
			}
			else if (pBar->IsTabbed ())
			{
				CBCGPBaseTabWnd* pTabWnd = 
						(CBCGPBaseTabWnd*) pBar->GetParent ();
				CBCGPBaseTabbedBar* pTabBar = (CBCGPBaseTabbedBar*)
						pTabWnd->GetParent ();	
				ASSERT_VALID (pTabBar);

				pBar->SetParent (m_pParentWnd);
				pTabBar->RemoveControlBar (pBar);
				pBar->EnableGripper (TRUE);
				pBar->DockToFrameWindow (info.m_dwBarAlignment);
			}
			else
			{
				pBar->DockToFrameWindow (info.m_dwBarAlignment);
			}
			pBar->SetWindowPos (NULL, info.m_rectBar.left, info.m_rectBar.top, 
									info.m_rectBar.Width (), info.m_rectBar.Height (),
									SWP_NOZORDER | SWP_NOACTIVATE);
			CBCGPAutoHideToolBar* pNewToolbar = 
				pBar->SetAutoHideMode (TRUE, info.m_dwBarAlignment, NULL, FALSE);

			if (pNewToolbar != NULL)
			{
				pNewToolbar->m_bActiveInGroup	= info.m_bActiveInGroup;
				pNewToolbar->m_bFirstInGroup	= info.m_bFirstInGroup;
				pNewToolbar->m_bLastInGroup		= info.m_bLastInGroup;


				info.m_pSavedBar = pBar;
				info.m_pSavedBar->m_pAutoHideBar->SetRecentVisibleState (info.m_bIsVisible); // used by dockbar when the frame is loaded
			}
		}
	}

	for (pos = m_lstLoadedAutoHideBarIDs.GetHeadPosition (); pos != NULL;)
	{
		BCGP_AUTOHIDEBAR_SAVE_INFO& info = m_lstLoadedAutoHideBarIDs.GetNext (pos);
		
		if (info.m_pSavedBar != NULL && info.m_pSavedBar->IsHideInAutoHideMode ())
		{
			info.m_pSavedBar->ShowControlBar (info.m_bIsVisible, FALSE, FALSE);
		}
	}

	for (pos = m_lstNonFloatingBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseTabbedBar* pBar = 
			DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, m_lstNonFloatingBars.GetNext (pos));
		if (pBar != NULL)
		{
			pBar->ApplyRestoredTabInfo (TRUE);		
		}
	}

	CObList lstCopy;
	lstCopy.AddTail (&m_lstControlBars);

	// check for empty sliders and tabbed bars
	for (pos = lstCopy.GetTailPosition (); pos != NULL;)
	{
		CObject* pBar = lstCopy.GetPrev (pos);
		CBCGPSlider* pSlider = DYNAMIC_DOWNCAST (CBCGPSlider, pBar);

		if (pSlider != NULL)
		{
			pSlider->NotifyAboutRelease ();
			continue;
		}

		CBCGPBaseTabbedBar* pTabbedBar = DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pBar);
		if (pTabbedBar != NULL && pTabbedBar->GetTabsNum () == 0 && pTabbedBar->CanFloat ())
		{
			pTabbedBar->UnDockControlBar (TRUE);
			pTabbedBar->DestroyWindow ();
		}
	}

	RecalcLayout ();

	m_lstLoadedBars.RemoveAll ();
	m_lstLoadedMiniFrames.RemoveAll ();
	m_lstNonFloatingBars.RemoveAll ();
	

	m_bRestoringDockState = FALSE;
}
//----------------------------------------------------------------------------------//
void BCGP_AUTOHIDEBAR_SAVE_INFO::Serilaize (CArchive& ar)
{
	if (ar.IsLoading ())
	{
		ar >> m_nBarID;
		ar >> m_dwBarAlignment;
		ar >> m_bIsVisible;

		int nSiblingCount = 0;
		ar >> nSiblingCount;

		for (int i = 0; i < nSiblingCount; i++)
		{
			UINT nSiblingBarID = (UINT)-1;
			ar >> nSiblingBarID;
			if (nSiblingBarID != -1) // can't be tabbed or so on
			{
				m_lstSiblingBars.AddHead (nSiblingBarID);
			}
		}
		ar >> m_rectBar;

		ar >> m_bFirstInGroup;
		ar >> m_bLastInGroup;
		ar >> m_bActiveInGroup;
	}
	else
	{
		ASSERT (m_pSavedBar != NULL);

		ar << m_pSavedBar->GetDlgCtrlID ();
		ar << m_pSavedBar->GetCurrentAlignment ();
		ar << (m_pSavedBar->IsHideInAutoHideMode () ? m_pSavedBar->IsVisible () : TRUE);

		CList<UINT, UINT&> lstSiblings;
		m_pSavedBar->GetRecentSiblingBarInfo (lstSiblings);

		int nSiblingCount = (int) lstSiblings.GetCount ();
		ar << nSiblingCount;

		for (POSITION pos = lstSiblings.GetHeadPosition (); pos != NULL;)
		{
			UINT nSiblingBarID = lstSiblings.GetNext (pos);
			ar << nSiblingBarID;
		}
		m_pSavedBar->GetWindowRect (&m_rectBar);
		if (m_rectBar.IsRectEmpty ())
		{
			ar << m_pSavedBar->GetAHRestoredRect ();
		}
		else
		{
			ar << m_rectBar;
		}

		ar << m_pSavedBar->GetAutoHideToolBar ()->m_bFirstInGroup;
		ar << m_pSavedBar->GetAutoHideToolBar ()->m_bLastInGroup;
		ar << m_pSavedBar->GetAutoHideToolBar ()->m_bActiveInGroup;
	}
}
//----------------------------------------------------------------------------------------
void CBCGPDockManager::HideForPrintPreview (const CObList& lstBars)
{
	for (POSITION pos = lstBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pBar = (CBCGPBaseControlBar*) lstBars.GetNext (pos);
		ASSERT_VALID (pBar);

		ASSERT_VALID (pBar);

		if (m_bHideDockingBarsInContainerMode || !IsOLEContainerMode ())
		{
			if (pBar->IsVisible () && pBar->HideInPrintPreviewMode ())
			{
				pBar->ShowControlBar (FALSE, TRUE, FALSE);
				m_lstBarsHiddenInPreview.AddTail (pBar);
			}
			for (POSITION pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
			{
				CWnd* pWnd = (CWnd*) m_lstMiniFrames.GetNext (pos);
				ASSERT_VALID (pWnd);
				if (pWnd->IsWindowVisible ())
				{
					pWnd->ShowWindow (SW_HIDE);
					m_lstBarsHiddenInPreview.AddTail (pWnd);
				}
			}
		}
		else
		{
			if (pBar->IsVisible () && pBar->HideInPrintPreviewMode () &&
				!pBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)) && 
				!pBar->IsKindOf (RUNTIME_CLASS (CBCGPAutoHideDockBar)) &&
				!pBar->IsKindOf (RUNTIME_CLASS (CBCGPSlider)))
			{
				pBar->ShowControlBar (FALSE, TRUE, FALSE);
				m_lstBarsHiddenInPreview.AddTail (pBar);
			}
		}
		
	}
}
//----------------------------------------------------------------------------------------
void CBCGPDockManager::SetPrintPreviewMode (BOOL bPreview, CPrintPreviewState* /*pState*/)
{
	ASSERT_VALID (this);

	if (bPreview)
	{
		if (m_bIsPrintPreviewMode || IsOLEContainerMode ())
		{
			m_bIsPrintPreviewMode = TRUE;
			return;
		}
		m_lstBarsHiddenInPreview.RemoveAll ();

		// Set visibility of standard ControlBars
		if (m_bHideDockingBarsInContainerMode || !IsOLEContainerMode ())
		{
			HideForPrintPreview (m_lstAutoHideBars);
		}
		HideForPrintPreview (m_lstControlBars);
	}
	else
	{
		if (!m_bIsPrintPreviewMode || IsOLEContainerMode ())
		{
			m_bIsPrintPreviewMode = FALSE;
			return;
		}
		for (POSITION pos = m_lstBarsHiddenInPreview.GetHeadPosition (); pos != NULL;)
		{
			CWnd* pWnd = DYNAMIC_DOWNCAST (CWnd, m_lstBarsHiddenInPreview.GetNext (pos));
			if (pWnd == NULL)
			{
				ASSERT (FALSE);
				continue;
			}
			ASSERT_VALID (pWnd);

			if (pWnd->IsKindOf (RUNTIME_CLASS (CBCGPBaseControlBar)))
			{
				CBCGPBaseControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar, pWnd);
				ASSERT_VALID (pBar);
				pBar->ShowControlBar (TRUE, TRUE, FALSE);
			}
			else
			{
				pWnd->ShowWindow (SW_SHOWNOACTIVATE);
			}
			
		}
	}

	m_bIsPrintPreviewMode = bPreview;
}
//----------------------------------------------------------------------------------------
CBCGPMiniFrameWnd* CBCGPDockManager::FrameFromPoint (CPoint pt,  
							CBCGPMiniFrameWnd* pFrameToExclude, BOOL bFloatMultiOnly) const
{
	for (POSITION pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
	{
		CBCGPMiniFrameWnd* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, m_lstMiniFrames.GetNext (pos));	
		ASSERT_VALID (pWnd);
		
		if (!pWnd->IsWindowVisible () || pWnd == pFrameToExclude)
		{
			continue;
		}

		if (!pWnd->IsKindOf (RUNTIME_CLASS (CBCGPMultiMiniFrameWnd)) && bFloatMultiOnly)
		{
			continue;
		}

		CRect rect;
		pWnd->GetWindowRect (rect);
		
		if (rect.PtInRect (pt))
		{
			return pWnd;
		}
	}

	return NULL;
}
//----------------------------------------------------------------------------------------
BOOL CBCGPDockManager::SendMessageToMiniFrames (UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	for (POSITION pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
	{
		CBCGPMiniFrameWnd* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, m_lstMiniFrames.GetNext (pos));

		ASSERT_VALID (pWnd);
		pWnd->SendMessage (uMessage, wParam, lParam);
	}

	return TRUE;
}
//----------------------------------------------------------------------------------------
BOOL CBCGPDockManager::SendMessageToControlBars(UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pBar = DYNAMIC_DOWNCAST(CBCGPBaseControlBar, m_lstControlBars.GetNext(pos));
		
		if (pBar->GetSafeHwnd() != NULL)
		{
			ASSERT_VALID(pBar);

			pBar->SendMessage(uMessage, wParam);
			pBar->SendMessageToDescendants(uMessage, wParam, lParam, TRUE, FALSE);
		}
	}

	for (pos = m_lstAutoHideBars.GetHeadPosition (); pos != NULL;)	
	{
		CBCGPSlider* pSlider = DYNAMIC_DOWNCAST(CBCGPSlider, m_lstAutoHideBars.GetNext (pos));
		if (pSlider->GetSafeHwnd() != NULL)
		{
			// SLIDER CONTAINS ONLY ONE BAR IN AUTOHIDE MODE
			CBCGPBaseControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar, pSlider->GetFirstBar());
			if (pBar->GetSafeHwnd() != NULL)
			{
				ASSERT_VALID (pBar);
		
				pBar->SendMessage(uMessage, wParam);
				pBar->SendMessageToDescendants(uMessage, wParam, lParam, TRUE, FALSE);
			}
		}
	}
		
	return TRUE;
}
//----------------------------------------------------------------------------------------
void CBCGPDockManager::RedrawAllMiniFrames()
{
	for (POSITION pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
	{
		CBCGPMiniFrameWnd* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, m_lstMiniFrames.GetNext (pos));

		if (pWnd->GetSafeHwnd() != NULL && pWnd->IsWindowVisible())
		{
			ASSERT_VALID (pWnd);
			pWnd->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
		}
	}
}
//----------------------------------------------------------------------------------------
void CBCGPDockManager::LockUpdate (BOOL bLock)
{
    if (bLock && m_pSDManager != NULL && m_pSDManager->IsStarted())
    {
        return;
    }

	if (bLock)
	{
		if (m_bLockUpdate)
		{
			return;
		}

		m_bLockUpdate = m_pParentWnd->LockWindowUpdate ();
	}
	else
	{
		if (!m_bLockUpdate)
		{
			return;
		}

		m_pParentWnd->UnlockWindowUpdate ();
		m_bLockUpdate = FALSE;
	}

	POSITION pos = NULL;

	for (pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
	{
		CBCGPMiniFrameWnd* pWnd = 
			DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, m_lstMiniFrames.GetNext (pos));

		pWnd->ValidateRect (NULL);
		pWnd->UpdateWindow ();

		ASSERT_VALID (pWnd);
		bLock ? pWnd->LockWindowUpdate () : 
				pWnd->UnlockWindowUpdate ();
	}

	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CWnd* pWnd = (CWnd*) m_lstControlBars.GetNext (pos);
		ASSERT_VALID (pWnd);

		pWnd->ValidateRect (NULL);
		pWnd->UpdateWindow ();

		bLock ? pWnd->LockWindowUpdate () : 
				pWnd->UnlockWindowUpdate ();
	}
}
//----------------------------------------------------------------------------------------
void CBCGPDockManager::GetControlBarList (CObList& lstBars, 
										  BOOL bIncludeAutohide,
										  CRuntimeClass* pRTCFilter, 
										  BOOL bIncludeTabs)
{
	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)	
	{
		CBCGPBaseControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar,
														m_lstControlBars.GetNext (pos));
		ASSERT_VALID (pBar);

		if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPDockBar)))
		{
			CBCGPDockBar* pDockBar = DYNAMIC_DOWNCAST (CBCGPDockBar, pBar);
			ASSERT_VALID (pDockBar);

			const CObList& lstDockedBars = pDockBar->GetControlBarList ();

			for (POSITION pos = lstDockedBars.GetHeadPosition (); pos != NULL;)
			{
				CBCGPBaseControlBar* pDockedBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar,
																		lstDockedBars.GetNext (pos));
				ASSERT_VALID (pDockedBar);
				if (pRTCFilter == NULL || pDockedBar->GetRuntimeClass () == pRTCFilter)
				{
					lstBars.AddTail (pDockedBar);
				}
			}
		}
		else if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPTabbedControlBar)) && bIncludeTabs)
		{
			CBCGPTabbedControlBar* pTabbedBar = DYNAMIC_DOWNCAST (CBCGPTabbedControlBar, pBar);
			ASSERT_VALID (pTabbedBar);
			pTabbedBar->GetControlBarList (lstBars, pRTCFilter);
			
		}
		else if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPReBar)) && m_bControlBarContextMenuRebarPanes)
		{
			CBCGPReBar* pRebar = DYNAMIC_DOWNCAST (CBCGPReBar, pBar);
			ASSERT_VALID (pRebar);

			pRebar->GetControlBarList (lstBars, pRTCFilter);
		}

		if (pRTCFilter == NULL || pBar->GetRuntimeClass () == pRTCFilter)
		{
			lstBars.AddTail (pBar);
		}
	}

	if (bIncludeAutohide)
	{
		for (POSITION pos = m_lstAutoHideBars.GetHeadPosition (); pos != NULL;)	
		{
			CBCGPBaseControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar, 
															m_lstAutoHideBars.GetNext (pos));	
			ASSERT_VALID (pBar);

			if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPSlider)))
			{
				CBCGPSlider* pSlider = DYNAMIC_DOWNCAST (CBCGPSlider, pBar);
				// SLIDER CONTAINS ONLY ONE BAR IN AUTOHIDE MODE
				if (pSlider != NULL)
				{
					pBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar, 
												pSlider->GetFirstBar ());
					if (pBar != NULL && 
						(pRTCFilter == NULL || pBar->GetRuntimeClass () == pRTCFilter))
					{
						lstBars.AddTail (pBar);
					}
				}
			}
		}
	}

	CBCGPMiniFrameWnd::GetControlBarList (lstBars, pRTCFilter, bIncludeTabs);
}
//----------------------------------------------------------------------------------------
BOOL CBCGPDockManager::ShowControlBars (BOOL bShow)
{
	if (!bShow)
	{
		if (m_bHiddenForOLE)
		{
			return FALSE;
		}
		
		m_lstBarsHiddenForOLE.RemoveAll ();

		CObList lstBars;
		GetControlBarList (lstBars, TRUE, NULL, TRUE);

		BOOL bHideInAutoHideMode = CBCGPDockingControlBar::m_bHideInAutoHideMode;
		CBCGPDockingControlBar::m_bHideInAutoHideMode = TRUE;

		m_bDisableRecalcLayout = TRUE;
		for (POSITION pos = lstBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPBaseControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar, lstBars.GetNext (pos));

			if (pBar != NULL)
			{
				DWORD dwStyle = pBar->GetBarStyle ();
				if ((dwStyle & CBRS_HIDE_INPLACE) != 0 && 
					 (pBar->IsVisible () || pBar->IsAutoHideMode ()))
				{
					pBar->ShowControlBar (FALSE, TRUE, FALSE);
					HWND hwndNext = pBar->GetSafeHwnd ();
					m_lstBarsHiddenForOLE.AddTail (hwndNext);
				}
			}
		}
		m_bDisableRecalcLayout = FALSE;

		CBCGPDockingControlBar::m_bHideInAutoHideMode = bHideInAutoHideMode;


		m_bHiddenForOLE = TRUE;
	}
	else
	{
		if (!m_bHiddenForOLE)
		{
			return FALSE;
		}

		BOOL bHideInAutoHideMode = CBCGPDockingControlBar::m_bHideInAutoHideMode;
		CBCGPDockingControlBar::m_bHideInAutoHideMode = TRUE;

		m_bDisableRecalcLayout = TRUE;
		for (POSITION pos = m_lstBarsHiddenForOLE.GetHeadPosition (); pos != NULL;)
		{
			CBCGPBaseControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar, 
				CWnd::FromHandlePermanent (m_lstBarsHiddenForOLE.GetNext (pos)));
			if (pBar != NULL)
			{
				pBar->ShowControlBar (TRUE, TRUE, FALSE);
			}
		}
		m_bDisableRecalcLayout = FALSE;
		CBCGPDockingControlBar::m_bHideInAutoHideMode = bHideInAutoHideMode;

		m_bHiddenForOLE = FALSE;
	}

	AdjustDockingLayout ();

	// significantly reduces flickering. If we return TRUE, MFC will perform 
	// additional recalc layout
	return FALSE;
}
//----------------------------------------------------------------------------------------
void CBCGPDockManager::OnActivateFrame (BOOL bActivate)
{
	if (m_pParentWnd == NULL)
	{
		return;
	}

	BOOL bCheckFoToolBarsOnly = !m_pParentWnd->IsKindOf (RUNTIME_CLASS (CBCGPMDIChildWnd));

	if (bActivate)
	{
		for (POSITION pos = m_lstBarsHiddenOnDeactivate.GetHeadPosition (); pos != NULL;)
		{
			HWND hWndNext = m_lstBarsHiddenOnDeactivate.GetNext (pos);
			if (IsWindow (hWndNext))
			{
				CWnd* pWndNext = CWnd::FromHandle (hWndNext);
				CBCGPMiniFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, pWndNext);
				if (pMiniFrame != NULL && pMiniFrame->GetControlBarCount () > 0)
				{
					ShowWindow (hWndNext, SW_SHOWNOACTIVATE);
				}
			}
		}

		m_lstBarsHiddenOnDeactivate.RemoveAll ();
	}
	else
	{
		for (POSITION pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
		{
			CWnd* pWndNext = (CWnd*) m_lstMiniFrames.GetNext (pos);

			HWND hWndNext = pWndNext->GetSafeHwnd ();
			if (::IsWindow (hWndNext) && IsWindowVisible (hWndNext))
			{
				if (bCheckFoToolBarsOnly)
				{
					CBCGPMiniFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, pWndNext);
					ASSERT_VALID (pMiniFrame);
					CBCGPBaseToolBar* pToolbar = 
						DYNAMIC_DOWNCAST (CBCGPBaseToolBar, pMiniFrame->GetControlBar ());

					if (pToolbar == NULL)
					{
						continue;
					}
				}

				ShowWindow (hWndNext, SW_HIDE);
				if (m_lstBarsHiddenOnDeactivate.Find (hWndNext) == NULL)
				{
					m_lstBarsHiddenOnDeactivate.AddTail (hWndNext);
				}
			}
		}
	}
}

void CBCGPDockManager::ShowFloatingBars(BOOL bShow)
{
	if (m_pParentWnd == NULL)
	{
		return;
	}

	if (bShow)
	{
		for (POSITION pos = m_lstHiddenFloatingBars.GetHeadPosition (); pos != NULL;)
		{
			HWND hWndNext = m_lstHiddenFloatingBars.GetNext (pos);
			if (IsWindow (hWndNext))
			{
				CWnd* pWndNext = CWnd::FromHandle (hWndNext);
				CBCGPMiniFrameWnd* pMiniFrame = DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, pWndNext);
				if (pMiniFrame != NULL && pMiniFrame->GetControlBarCount () > 0)
				{
					ShowWindow (hWndNext, SW_SHOWNOACTIVATE);
				}
			}
		}

		m_lstHiddenFloatingBars.RemoveAll ();
	}
	else
	{
		for (POSITION pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
		{
			CWnd* pWndNext = (CWnd*) m_lstMiniFrames.GetNext (pos);

			HWND hWndNext = pWndNext->GetSafeHwnd ();
			if (::IsWindow (hWndNext) && IsWindowVisible (hWndNext))
			{
				ShowWindow (hWndNext, SW_HIDE);
				if (m_lstHiddenFloatingBars.Find (hWndNext) == NULL)
				{
					m_lstHiddenFloatingBars.AddTail (hWndNext);
				}
			}
		}
	}
}

void CBCGPDockManager::ResortMiniFramesForZOrder ()
{
	int nCount = (int) m_lstMiniFrames.GetCount ();

	if (nCount == 0)
	{
		return;
	}

	CWnd* pFirst = DYNAMIC_DOWNCAST(CWnd, m_lstMiniFrames.GetHead());

	if (pFirst == NULL)
	{
		return;
	}

	CWnd* pParent = pFirst->GetParent ();

	if (pParent == NULL)
	{
		return;
	}

	CObList lstNewMiniFrames;

	CWnd* pNext = NULL;
	for (pNext = pParent->GetWindow (GW_HWNDFIRST); pNext != NULL; pNext = pNext->GetNextWindow() )
	{
		if (m_lstMiniFrames.Find (pNext) != NULL)
		{
			lstNewMiniFrames.AddTail (pNext);
		}
	}

	m_lstMiniFrames.RemoveAll ();
	m_lstMiniFrames.AddTail (&lstNewMiniFrames);
}

void CBCGPDockManager::SetDockMode (BCGP_DOCK_TYPE dockMode, BCGP_SMARTDOCK_THEME theme/* = BCGP_SDT_DEFAULT*/)
{
	m_dockModeGlobal = dockMode;

	if (m_dockModeGlobal == BCGP_DT_SMART)
	{
		// BCGP_DT_SMART should only be used along with BCGP_DT_IMMEDIATE,
		m_dockModeGlobal = BCGP_DOCK_TYPE (BCGP_DT_SMART | BCGP_DT_IMMEDIATE);
		m_SDTheme = theme;
	}
}

void CBCGPDockManager::SetSmartDockingParams (CBCGPSmartDockingParams& params)
{
	int nCount = 0;

	for (int i = 0; i < BCGP_SD_MARKERS_NUM; i++)
	{
		if (params.m_uiMarkerBmpResID [i] != 0)
		{
			nCount++;
		}
	}

	if (nCount != 0 && nCount != BCGP_SD_MARKERS_NUM)
	{
		// Unable to set part of bitmap markers!
		ASSERT (FALSE);
		return;
	}

	params.CopyTo (m_SDParams);
	m_bSDParamsModified = TRUE;
}
//-----------------------------------------------------------------------//
BOOL CBCGPDockManager::ReplaceControlBar (CBCGPDockingControlBar* pOriginalBar, 
										  CBCGPDockingControlBar* pNewBar)
{
	if (pOriginalBar == NULL || pNewBar == NULL)
	{
		return FALSE;
	}
	ASSERT_VALID (pNewBar);
	ASSERT_VALID (pOriginalBar);

	CRect rectOrgWnd;
	pOriginalBar->GetWindowRect (rectOrgWnd);

	CWnd* pOrgParentWnd = pOriginalBar->GetParent ();
	ASSERT_VALID (pOrgParentWnd);

	pOrgParentWnd->ScreenToClient (rectOrgWnd);
	pOriginalBar->StoreRecentDockInfo ();
	pNewBar->CopyState (pOriginalBar);

	if (pOriginalBar->IsAutoHideMode ())
	{
		// hide the original bar
		pOriginalBar->Slide (FALSE, FALSE);

		// set the same window pos for the new bar
		pNewBar->SetWindowPos (NULL, rectOrgWnd.left, rectOrgWnd.top, 
									 rectOrgWnd.Width (), rectOrgWnd.Height (),
									 SWP_NOZORDER);

		pNewBar->ShowWindow (SW_HIDE);

		// replace bar in default slider and in the button
		CBCGPAutoHideButton* pButton = pOriginalBar->GetAutoHideButton ();
		ASSERT_VALID (pButton);

		CBCGPSlider* pSlider = pOriginalBar->GetDefaultSlider ();
		if (pSlider != NULL)
		{
			ASSERT_VALID (pSlider);
			pSlider->ReplaceControlBar (pOriginalBar, pNewBar);
		}

		if (pButton != NULL)
		{
			pButton->ReplaceControlBar (pNewBar);
		}

		// reparent
		pNewBar->SetParent (pOrgParentWnd);

		// tell the new bar that it's in autohide mode
		pNewBar->m_bPinState = TRUE;
		pNewBar->m_nAutoHideConditionTimerID = pNewBar->SetTimer (ID_CHECK_AUTO_HIDE_CONDITION, 
													pNewBar->m_nTimeOutBeforeAutoHide, NULL);	
		AlignAutoHideBar (pSlider);

		// need to update caption buttons
		pNewBar->SetWindowPos (NULL, -1, -1, -1, -1,
							SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);

		return TRUE;
	}


	if (pOriginalBar->IsTabbed ())
	{
		HWND hwndTab = NULL;
		CBCGPBaseTabWnd* pTabWnd = pOriginalBar->GetParentTabWnd (hwndTab);

		if (pTabWnd != NULL)
		{
			ASSERT_VALID (pTabWnd);

			int nTabNum = pTabWnd->GetTabFromHwnd (pOriginalBar->GetSafeHwnd ());

			CString strText;
			pOriginalBar->GetWindowText (strText);

			pTabWnd->InsertTab (pNewBar, strText, nTabNum + 1);
			pTabWnd->SetTabHicon (nTabNum + 1, pNewBar->GetIcon (FALSE));
			pTabWnd->RemoveTab (nTabNum);

			pNewBar->EnableGripper (FALSE);
			pNewBar->SetParent (pTabWnd);

			return TRUE;
		}
	}
	
	
	BOOL bResult = pOriginalBar->ReplaceControlBar (pNewBar, BCGP_DM_UNKNOWN, TRUE);
	if (bResult)
	{
		pNewBar->SetParent (pOrgParentWnd);
		pNewBar->SetWindowPos (NULL, rectOrgWnd.left, rectOrgWnd.top, 
									 rectOrgWnd.Width (), rectOrgWnd.Height (),
									 SWP_NOZORDER);
		AdjustDockingLayout ();

	}
	
	return bResult;
}
//-----------------------------------------------------------------------//
void CBCGPDockManager::ReleaseEmptyContainers ()
{
	POSITION pos = NULL;

	for (pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPSlider* pSlider = DYNAMIC_DOWNCAST (CBCGPSlider,
													m_lstControlBars.GetNext (pos));
		if (pSlider != NULL && pSlider->IsDefault ())
		{
			pSlider->ReleaseEmptyContainers ();
		}
	}

	for (pos = m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
	{
		CBCGPMultiMiniFrameWnd* pWndNext = DYNAMIC_DOWNCAST (CBCGPMultiMiniFrameWnd, 
												m_lstMiniFrames.GetNext (pos));
		if (pWndNext != NULL)
		{
			CBCGPBarContainerManager& manager = pWndNext->GetContainerManager ();
			manager.ReleaseEmptyContainers ();
		}
	}
}

void CBCGPDockManager::BuildControlBarsMenu (CMenu& menu, BOOL bToolbarsOnly)
{
	m_mapControlBarsInMenu.RemoveAll ();

	CObList lstBars;
	GetControlBarList (lstBars, TRUE);

	for (int nStep = 0; nStep < 2; nStep++)	// 2 steps: 1-st: show toolbars, 2-nd other control bars
	{
		if (nStep == 1 && bToolbarsOnly)
		{
			break;
		}

		BOOL bIsFirst = TRUE;

		for (POSITION pos = lstBars.GetHeadPosition (); pos != NULL;)
		{
			CBCGPControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPControlBar,
				lstBars.GetNext (pos));

			if (pBar == NULL || !::IsWindow (pBar->m_hWnd))
			{
				continue;
			}

			ASSERT_VALID (pBar);

			if (!pBar->AllowShowOnControlMenu () || !pBar->CanBeClosed ())
			{
				continue;
			}
			
			const BOOL bIsToolbar = pBar->IsKindOf (RUNTIME_CLASS (CBCGPToolBar));

			if ((bIsToolbar && nStep == 1) || (!bIsToolbar && nStep == 0))
			{
				continue;
			}

			CString strBarName;
			pBar->GetBarName (strBarName);

			if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPBaseTabbedBar)) &&
				!pBar->IsKindOf (RUNTIME_CLASS (CBCGPOutlookBar)))
			{
				CBCGPBaseTabWnd* pTabWnd = ((CBCGPBaseTabbedBar*)pBar)->GetUnderlinedWindow ();
				if (pTabWnd != NULL)
				{
					for (int iTab = 0; iTab < pTabWnd->GetTabsNum (); iTab++)
					{
						CBCGPControlBar* pTabbedBar = DYNAMIC_DOWNCAST (CBCGPControlBar,
							pTabWnd->GetTabWnd (iTab));
						if (pTabbedBar != NULL && pTabbedBar->AllowShowOnControlMenu ())
						{
							CBCGPControlBar* pBarInMenu = NULL;

							if (!m_mapControlBarsInMenu.Lookup (pTabbedBar->GetDlgCtrlID (), pBarInMenu))
							{
								pTabbedBar->GetBarName (strBarName);

								if (bIsFirst && nStep == 1 && menu.GetMenuItemCount () > 0)
								{
									menu.AppendMenu (MF_SEPARATOR);
								}

								menu.AppendMenu (MF_STRING, pTabbedBar->GetDlgCtrlID (), strBarName);

								bIsFirst = FALSE;

								m_mapControlBarsInMenu.SetAt (pTabbedBar->GetDlgCtrlID (), pTabbedBar);
							}
						}
					}
				}
			}
			else if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPReBar)))
			{
				CBCGPReBar* pRebar = DYNAMIC_DOWNCAST (CBCGPReBar, pBar);
				ASSERT_VALID (pBar);

				CReBarCtrl& wndReBar = pRebar->GetReBarCtrl ();
				UINT uiReBarsCount = wndReBar.GetBandCount ();

				REBARBANDINFO bandInfo;
				bandInfo.cbSize = globalData.GetRebarBandInfoSize ();
				bandInfo.fMask = (RBBIM_CHILDSIZE | RBBIM_CHILD | RBBIM_IDEALSIZE);

				for (UINT uiBand = 0; uiBand < uiReBarsCount; uiBand++)
				{
					wndReBar.GetBandInfo (uiBand, &bandInfo);

					CBCGPControlBar* pRebarBand = DYNAMIC_DOWNCAST (CBCGPControlBar,
						CWnd::FromHandlePermanent (bandInfo.hwndChild));

					if (pRebarBand != NULL && pRebarBand->AllowShowOnControlMenu ())
					{
						pRebarBand->GetBarName (strBarName);

						if (bIsFirst && nStep == 1 && menu.GetMenuItemCount () > 0)
						{
							menu.AppendMenu (MF_SEPARATOR);
						}

						menu.AppendMenu (MF_STRING, pRebarBand->GetDlgCtrlID (), strBarName);

						bIsFirst = FALSE;

						m_mapControlBarsInMenu.SetAt (pRebarBand->GetDlgCtrlID (), pRebarBand);
					}
				}
			}
			else
			{
				CBCGPControlBar* pBarInMenu = NULL;

				if (!m_mapControlBarsInMenu.Lookup (pBar->GetDlgCtrlID (), pBarInMenu))
				{
					if (bIsFirst && nStep == 1 && menu.GetMenuItemCount () > 0)
					{
						menu.AppendMenu (MF_SEPARATOR);
					}

					menu.AppendMenu (MF_STRING, pBar->GetDlgCtrlID (), strBarName);
					bIsFirst = FALSE;
					m_mapControlBarsInMenu.SetAt (pBar->GetDlgCtrlID (), pBar);
				}
			}
		}
	}

	//------------------------------
	// Add MDI tabbed bars (if any):
	//------------------------------
	CBCGPMDIFrameWnd* pMDIFrame = DYNAMIC_DOWNCAST (
		CBCGPMDIFrameWnd, m_pParentWnd);

	if (pMDIFrame != NULL && !bToolbarsOnly)
	{
		HWND hwndMDIChild = ::GetWindow (pMDIFrame->m_hWndMDIClient, GW_CHILD);

		while (hwndMDIChild != NULL)
		{
			CBCGPMDIChildWnd* pMDIChildFrame = DYNAMIC_DOWNCAST (CBCGPMDIChildWnd, 
				CWnd::FromHandle (hwndMDIChild));

			if (pMDIChildFrame != NULL && pMDIChildFrame->IsTabbedControlBar ())
			{
				CBCGPDockingControlBar* pBar = pMDIChildFrame->GetTabbedControlBar ();
				ASSERT_VALID (pBar);

				CString strBarName;
				pBar->GetBarName (strBarName);

				menu.AppendMenu (MF_STRING, pBar->GetDlgCtrlID (), strBarName);
				m_mapControlBarsInMenu.SetAt (pBar->GetDlgCtrlID (), pBar);
			}

			hwndMDIChild = ::GetWindow (hwndMDIChild, GW_HWNDNEXT);
		}

		for (POSITION pos = m_lstHiddenMDITabbedBars.GetHeadPosition (); pos != NULL;)
		{
			HWND hwnd = m_lstHiddenMDITabbedBars.GetNext (pos);
			CBCGPDockingControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPDockingControlBar, 
				CWnd::FromHandlePermanent (hwnd));

			if (pBar != NULL)
			{
				ASSERT_VALID (pBar);

				CString strBarName;
				pBar->GetBarName (strBarName);

				menu.AppendMenu (MF_STRING, pBar->GetDlgCtrlID (), strBarName);
				m_mapControlBarsInMenu.SetAt (pBar->GetDlgCtrlID (), pBar);
			}
		}
	}

	if (m_uiCustomizeCmd != 0)
	{
		if (menu.GetMenuItemCount () > 0)
		{
			menu.AppendMenu (MF_SEPARATOR);
		}

		menu.AppendMenu (MF_STRING, m_uiCustomizeCmd, m_strCustomizeText);
	}
}

void CBCGPDockManager::OnControlBarContextMenu (CPoint point)
{
	if (!m_bControlBarsContextMenu)
	{
		return;
	}

	CFrameWnd* pFrame = BCGCBProGetTopLevelFrame (m_pParentWnd);
	if (pFrame == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	CMenu menu;
	menu.CreatePopupMenu ();

	BuildControlBarsMenu (menu, m_bControlBarsContextMenuToolbarsOnly);

	CBCGPPopupMenu* pPopupMenu = new CBCGPPopupMenu;
	pPopupMenu->SetAutoDestroy (FALSE);

	m_bControlBarsMenuIsShown = TRUE;

	pPopupMenu->Create (pFrame, point.x, point.y, (HMENU) menu);
}

BOOL CBCGPDockManager::ProcessControlBarContextMenuCommand (UINT nID, int nCode, 
	void* pExtra, AFX_CMDHANDLERINFO* /*pHandlerInfo*/)
{
	if (!m_bControlBarsContextMenu || m_mapControlBarsInMenu.IsEmpty ())
	{
		return FALSE;
	}

	if (nCode == CN_UPDATE_COMMAND_UI && !m_bControlBarsMenuIsShown)
	{
		return FALSE;
	}

	CBCGPControlBar* pBar = NULL;
	if (!m_mapControlBarsInMenu.Lookup (nID, pBar) || pBar == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pBar);

	if (nCode == CN_UPDATE_COMMAND_UI)
	{
		CCmdUI* pCmdUI = (CCmdUI*) pExtra;
		if (pCmdUI == NULL)
		{
			return FALSE;
		}

		pCmdUI->SetCheck (pBar->IsVisible ());
		return TRUE;
	}

	UINT nMsg = HIWORD (nCode);
	nCode = LOWORD (nCode);

	if ((nMsg != WM_COMMAND && nMsg != 0) || pExtra != NULL)
	{
		return TRUE;
	}

	pBar->ShowControlBar (!pBar->IsVisible (), FALSE, TRUE);

	CFrameWnd* pFrame = BCGCBProGetTopLevelFrame (pBar);
	if (pFrame == NULL)
	{
		RecalcLayout ();
	}
	else
	{
		pFrame->RecalcLayout ();
	}

	m_mapControlBarsInMenu.RemoveAll ();
	return TRUE;
}
//-----------------------------------------------------------------------//
void CBCGPDockManager::OnClosePopupMenu ()
{
	m_bControlBarsMenuIsShown = FALSE;
}
//-----------------------------------------------------------------------//
void CBCGPDockManager::EnableControlBarContextMenu (
		BOOL bEnable, UINT uiCustomizeCmd, const CString& strCustomizeText,
		BOOL bToolbarsOnly, BOOL bIncludeRebarPanes)
{
	m_bControlBarsContextMenu = bEnable;
	m_bControlBarsContextMenuToolbarsOnly = bToolbarsOnly;
	m_uiCustomizeCmd = uiCustomizeCmd;
	m_strCustomizeText = strCustomizeText;
	m_bControlBarContextMenuRebarPanes = bIncludeRebarPanes;
}
//-----------------------------------------------------------------------//
void CBCGPDockManager::AddHiddenMDITabbedBar (CBCGPDockingControlBar* pBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	HWND hwnd = pBar->GetSafeHwnd ();
	m_lstHiddenMDITabbedBars.AddTail (hwnd);
}
//-----------------------------------------------------------------------//
void CBCGPDockManager::RemoveHiddenMDITabbedBar (CBCGPDockingControlBar* pBar)
{
	ASSERT_VALID (this);

	HWND hwnd = pBar->GetSafeHwnd ();
	for (POSITION pos = m_lstHiddenMDITabbedBars.GetHeadPosition (); pos != NULL; m_lstHiddenMDITabbedBars.GetNext (pos))
	{
		HWND hNext = m_lstHiddenMDITabbedBars.GetAt (pos);
		if (hNext == hwnd)
		{
			m_lstHiddenMDITabbedBars.RemoveAt (pos);
			return;
		}
	}
}
//-----------------------------------------------------------------------//
CBCGPSmartDockingParams::CBCGPSmartDockingParams()
{
	m_sizeTotal = CSize (93, 93);
	m_nCentralGroupOffset = 5;
	m_clrTransparent = RGB (255, 0, 255);
	m_clrToneSrc = (COLORREF)-1;
	m_clrToneDest = (COLORREF)-1;

	for (int i = 0; i < BCGP_SD_MARKERS_NUM; i++)
	{
		m_uiMarkerBmpResID [i] = 0;
		m_uiMarkerLightBmpResID [i] = 0;
	}

	m_uiBaseBmpResID = 0;

	m_clrBaseBackground = (COLORREF)-1;
	m_clrBaseBorder = (COLORREF)-1;
	m_bUseThemeColorInShading = FALSE;
	m_bIsAlphaMarkers = FALSE;
}

void CBCGPSmartDockingParams::CopyTo (CBCGPSmartDockingParams& params)
{
	params.m_sizeTotal = m_sizeTotal;
	params.m_nCentralGroupOffset = m_nCentralGroupOffset;
	params.m_clrTransparent = m_clrTransparent;
	params.m_clrToneSrc = m_clrToneSrc;
	params.m_clrToneDest = m_clrToneDest;

	for (int i = 0; i < BCGP_SD_MARKERS_NUM; i++)
	{
		params.m_uiMarkerBmpResID [i] = m_uiMarkerBmpResID [i];
		params.m_uiMarkerLightBmpResID [i] = m_uiMarkerLightBmpResID [i];
	}

	params.m_uiBaseBmpResID = m_uiBaseBmpResID;

	params.m_clrBaseBackground = m_clrBaseBackground;
	params.m_clrBaseBorder = m_clrBaseBorder;
	params.m_bUseThemeColorInShading = m_bUseThemeColorInShading;
	params.m_bIsAlphaMarkers = m_bIsAlphaMarkers;
}
