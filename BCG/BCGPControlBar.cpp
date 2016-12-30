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
// BCGPControlBar.cpp : implementation file
//

#include "stdafx.h"

#include "BCGGlobals.h"
#include "BCGPDockBar.h"
#include "BCGPDockBarRow.h"
#include "BCGPMiniFrameWnd.h"

#include "BCGPControlBar.h"
#include "BCGPTabWnd.h"
#include "BCGPTabbedControlBar.h"
#include "BCGPTabbedToolbar.h"
#include "BCGPDockingCBWrapper.h"

#include "BCGPDockManager.h"
#include "BCGPGlobalUtils.h"

#include "BCGPOleCntrFrameWnd.h"

#include "RegPath.h"
#include "BCGPRegistry.h"
#include "BCGPReBar.h"

#include "BCGPContextMenuManager.h"

#include "BCGPLocalResource.h"
#include "BCGProRes.h"

#include "BCGPPopupMenu.h"

#include "BCGPMDIFrameWnd.h"
#include "BCGPMDIChildWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const CString strControlBarProfile = _T ("BCGControlBars");

int CBCGPControlBar::m_bHandleMinSize = FALSE;
BOOL CBCGPControlBar::m_bHideDisabledControlBarMenuItems = FALSE;

#define REG_SECTION_FMT					_T("%sBCGPControlBar-%d")
#define REG_SECTION_FMT_EX				_T("%sBCGPControlBar-%d%x")

IMPLEMENT_DYNCREATE(CBCGPControlBar, CBCGPBaseControlBar)

/////////////////////////////////////////////////////////////////////////////
// CBCGPControlBar

#pragma warning (disable : 4355)

CBCGPControlBar::CBCGPControlBar() :  m_bCaptured (false), 
									m_nID (0), 
									m_bDisableMove (false),
									m_recentDockInfo (this)
{
	m_cxLeftBorder = m_cxRightBorder = 6;
	m_cxDefaultGap = 2;
	m_cyTopBorder = m_cyBottomBorder = 2;
	m_bIsTransparentBorder = FALSE;

	m_pData = NULL; 
	m_nCount = 0;
	m_nMRUWidth = 32767;
	m_bDblClick = false;

	m_ptClientHotSpot.x = m_ptClientHotSpot.y = 0;
	
	m_rectSavedDockedRect.SetRectEmpty ();
	
	m_bDragMode = FALSE;
	m_bWasFloatingBeforeMove = FALSE;
	m_bRecentFloatingState = FALSE;

	m_pMiniFrameRTC = RUNTIME_CLASS (CBCGPMiniFrameWnd);
	m_rectDragImmediate.SetRectEmpty ();

	m_hwndMiniFrameToBeClosed = NULL;

	m_bFirstInGroup = TRUE;
	m_bLastInGroup = TRUE;
	m_bActiveInGroup = TRUE;
	m_bExclusiveRow = FALSE;

	m_bPinState = FALSE;

	m_sizeMin.cx = m_sizeMin.cy = 1;
}

#pragma warning (default : 4355)

CBCGPControlBar::~CBCGPControlBar()
{
	// free array
	if (m_pData != NULL)
	{
		ASSERT(m_nCount != 0);
		free(m_pData);
	}
}


BEGIN_MESSAGE_MAP(CBCGPControlBar, CBCGPBaseControlBar)
	//{{AFX_MSG_MAP(CBCGPControlBar)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_NCDESTROY()
	ON_WM_CONTEXTMENU()
	ON_WM_CANCELMODE()
	ON_WM_CHAR()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPControlBar message handlers

BOOL CBCGPControlBar::Create(LPCTSTR lpszClassName, DWORD dwStyle, const RECT& rect, 
							 CWnd* pParentWnd, UINT nID, DWORD dwBCGStyle,
							 CCreateContext* pContext) 
{
	return CBCGPControlBar::CreateEx (0, lpszClassName, dwStyle, rect, pParentWnd, nID, dwBCGStyle, pContext);
}
//***********************************************************************************//
BOOL CBCGPControlBar::CreateEx(DWORD dwStyleEx, LPCTSTR lpszClassName, DWORD dwStyle, 
							   const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwBCGStyle,
							   CCreateContext* pContext)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pParentWnd);
	
	CString strClassName;

	if (lpszClassName == NULL)
	{
		strClassName = globalData.RegisterWindowClass (_T("BCGPControlBar"));
	}
	else
	{
		strClassName = lpszClassName;
	}

	m_nID = nID;

	dwStyle |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	if (!CBCGPBaseControlBar::CreateEx (dwStyleEx, strClassName, NULL, dwStyle, rect, pParentWnd, nID, dwBCGStyle, pContext))
	{
		return FALSE;
	}

	CRect rectInit = rect;
	pParentWnd->ClientToScreen (rectInit);

	if (m_recentDockInfo.m_recentMiniFrameInfo.m_rectDockedRect.IsRectEmpty ())
	{
		m_recentDockInfo.m_recentMiniFrameInfo.m_rectDockedRect = rectInit;
	}

	if (m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect.IsRectEmpty ())
	{
		m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect = rectInit;
	}

	if (!rectInit.IsRectEmpty ())
	{
		m_recentDockInfo.m_rectRecentFloatingRect = rectInit;
	}

	SetOwner (pParentWnd);
	UpdateVirtualRect ();

	if (m_dwBCGStyle & CanFloat ())
	{
		m_dragFrameImpl.Init (this);
	}

	return TRUE;
}
//***********************************************************************************
void CBCGPControlBar::SetBorders (int cxLeft, int cyTop, int cxRight, int cyBottom, BOOL bIsTransparent)
{
	m_cxLeftBorder = cxLeft;
	m_cxRightBorder = cxRight;
	m_cyTopBorder = cyTop;
	m_cyBottomBorder = cyBottom;
	m_bIsTransparentBorder = bIsTransparent;
}
//**********************************************************************************
void CBCGPControlBar::SetBorders (LPCRECT lpRect, BOOL bIsTransparent)
{
	m_cxLeftBorder = lpRect->left;
	m_cxRightBorder = lpRect->right;
	m_cyTopBorder = lpRect->top;
	m_cyBottomBorder = lpRect->bottom;
	m_bIsTransparentBorder = bIsTransparent;
}
//***********************************************************************************
CRect CBCGPControlBar::GetBorders () const
{
	CRect rect (m_cxLeftBorder, m_cyTopBorder, m_cxRightBorder, m_cyBottomBorder);
	return rect;
}
//***********************************************************************************
void CBCGPControlBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	ASSERT_VALID (this);

	if (!m_bCaptured && CanFloat ())
	{
		CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame ();

		if ((GetDockMode () & BCGP_DT_IMMEDIATE) != 0 || pMiniFrame != NULL)
		{
			StoreRecentDockInfo ();
			if (pMiniFrame == NULL)
			{
				EnterDragMode (TRUE);
				m_bWasFloatingBeforeMove = FALSE;
			}
			else if (pMiniFrame != NULL)
			{
				ASSERT_VALID (pMiniFrame);
				// it's on the miniframe - reflect message to the miniframe if 
				// this bar is alone on the miniframe
				if (pMiniFrame->GetControlBarCount () == 1)
				{
					MapWindowPoints (pMiniFrame, &point, 1);
					pMiniFrame->SendMessage (WM_LBUTTONDOWN, nFlags, MAKELPARAM (point.x, point.y));
					m_bWasFloatingBeforeMove = TRUE;
				}
				else
				{
					EnterDragMode (TRUE);
					m_bWasFloatingBeforeMove = FALSE;
				}
				return;
			}
		}
		else if ((GetDockMode () & BCGP_DT_STANDARD) != 0)
		{
			EnterDragMode (TRUE);
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}
//***********************************************************************************
// EnterDragMode is called from OnLButtonDown and OnContinueMoving - when docked
// in the last case we dont need to save recent floating info and client hot spot
//***********************************************************************************
void CBCGPControlBar::EnterDragMode (BOOL bChangeHotPoint)
{
	ASSERT_VALID (this);

	CPoint ptCursorPos;
	GetCursorPos (&ptCursorPos);

	// fixup the current virtual rectangle when mouse btn is down - to be
	// prepared to move THIS control bar
	UpdateVirtualRect ();

	if (bChangeHotPoint)
	{
		m_ptClientHotSpot = ptCursorPos;
		ScreenToClient (&m_ptClientHotSpot);
	}

	if (!m_bCaptured && IsDocked ())
	{
		SetCapture ();
		
		m_bCaptured = true;
		m_dragFrameImpl.m_ptHot = ptCursorPos;

		SetDragMode (TRUE);

		GetWindowRect (m_rectDragImmediate);
	}
}
//***********************************************************************************
void CBCGPControlBar::StoreRecentDockInfo ()
{
	m_recentDockInfo.m_pRecentDockBarRow	= m_pDockBarRow;
	m_recentDockInfo.m_pRecentDockBar		= m_pParentDockBar;

	if (m_recentDockInfo.m_pRecentDockBar != NULL)
	{
		m_recentDockInfo.m_nRecentRowIndex = 
			m_recentDockInfo.m_pRecentDockBar->FindRowIndex (m_pDockBarRow);
	}
	
	CalcRecentDockedRect ();
}
//***********************************************************************************
void CBCGPControlBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ASSERT_VALID (this);

	CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame ();

	if (m_bCaptured)	
	{
		ReleaseCapture ();
 		m_bCaptured = false;

		if (nFlags != 0xFFFF)
		{
			if (m_hwndMiniFrameToBeClosed != NULL && ::IsWindow (m_hwndMiniFrameToBeClosed))
			{
				::DestroyWindow (m_hwndMiniFrameToBeClosed);
			}

			m_hwndMiniFrameToBeClosed = NULL;
		}

		SetDragMode (FALSE);

	    CBCGPDockManager* pDockManager = globalUtils.GetDockManager (GetParent ());
	    if (pDockManager != NULL)
	    {
            pDockManager->StopSDocking ();
        }

		if ((GetDockMode () & BCGP_DT_STANDARD) != 0 && 
			(m_dragFrameImpl.m_bDragStarted || m_dragFrameImpl.m_nInsertedTabID >= 0))
		{
			CRect rectFinal = m_dragFrameImpl.m_rectDrag;
			if (m_dragFrameImpl.m_bDragStarted && 
				(GetDockMode () & BCGP_DT_STANDARD) != 0)
			{
				m_dragFrameImpl.EndDrawDragFrame ();
			}
			BOOL bWasDocked = FALSE;
			StoreRecentDockInfo ();
			CBCGPControlBar* pTargetBar = DockControlBarStandard (bWasDocked);
		
			if (!bWasDocked)
			{
				if (!rectFinal.IsRectEmpty () && pTargetBar != this)
				{
					FloatControlBar (rectFinal, BCGP_DM_STANDARD);
				}	
			}

			return;
		}
	}
	else if (pMiniFrame != NULL && !m_bDblClick && 
			 pMiniFrame->IsWindowVisible ())
	{
		// it's attached to the miniframe - reflect message to the miniframe
		ASSERT_VALID (pMiniFrame);
		MapWindowPoints (pMiniFrame, &point, 1);
		pMiniFrame->SendMessage (WM_LBUTTONUP, nFlags, MAKELPARAM (point.x, point.y));
		return;
	}
	m_bDblClick = false;

	if (m_pDockBarRow != NULL)
	{
		m_pDockBarRow->FixupVirtualRects (false);
	}
	
	CWnd::OnLButtonUp(nFlags, point);
}
//***********************************************************************************
void CBCGPControlBar::OnMouseMove(UINT nFlags, CPoint point) 
{
	ASSERT_VALID (this);
	// the control bar is moved when it resides on the dock bar (docked)
	if (m_bCaptured)
	{
        BCGP_DOCK_TYPE docktype = GetDockMode ();

		if ((docktype & BCGP_DT_IMMEDIATE) != 0)
		{
			CPoint ptMouse;
			GetCursorPos (&ptMouse);

			CPoint ptOffset = ptMouse - m_dragFrameImpl.m_ptHot;	
			m_rectDragImmediate.OffsetRect (ptOffset);

			UpdateVirtualRect (ptOffset);

			if (m_pParentDockBar != NULL)
			{
				m_pParentDockBar->MoveControlBar (this, nFlags, ptOffset);
				RedrawWindow ();
			}

			m_dragFrameImpl.m_ptHot = ptMouse;		
		}
		else if ((docktype & BCGP_DT_STANDARD) != 0)
		{
			m_dragFrameImpl.MoveDragFrame ();
		}
	}
	else
	{
		// it should be moved (if captured) along with the mini fraeme
		CWnd::OnMouseMove(nFlags, point);
	}
}
//***********************************************************************************
void CBCGPControlBar::RecalcLayout ()
{
	ASSERT_VALID (this);
	
	if (m_pDockBarRow != NULL)
	{
		UpdateVirtualRect ();
	}
	else 
	{
		CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame ();
		CWnd* pParent = GetParent ();

		if (pMiniFrame != NULL && !pParent->IsKindOf (RUNTIME_CLASS (CBCGPTabWnd)))
		{
			pMiniFrame->OnBarRecalcLayout ();
		}
	}
}
//***********************************************************************************
BOOL CBCGPControlBar::DockByMouse (CBCGPBaseControlBar* pDockBar)
{
	ASSERT_VALID (this);

	if (!OnBeforeDock (&pDockBar, NULL, BCGP_DM_MOUSE))
	{
		return FALSE;
	}

	if (Dock (pDockBar, NULL, BCGP_DM_MOUSE))
	{
		OnAfterDock (pDockBar, NULL, BCGP_DM_MOUSE);
		return TRUE;
	}

	return FALSE;
}
//***********************************************************************************
BOOL CBCGPControlBar::DockControlBar (CBCGPBaseControlBar* pDockBar, LPCRECT lpRect, 
									  BCGP_DOCK_METHOD dockMethod)
{
	ASSERT_VALID (this);
	if (!OnBeforeDock (&pDockBar, lpRect, dockMethod))
	{
		return FALSE;
	}

	if (Dock (pDockBar, lpRect, dockMethod))
	{
		OnAfterDock (pDockBar, lpRect, dockMethod);
		return TRUE;
	}
	return FALSE;
}
//***********************************************************************************
BOOL CBCGPControlBar::Dock (CBCGPBaseControlBar* pDockBar, LPCRECT lpRect, 
							BCGP_DOCK_METHOD dockMethod)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDockBar);

	if (dockMethod == BCGP_DM_DBL_CLICK)
	{
		pDockBar = m_recentDockInfo.m_pRecentDockBar;

		if (pDockBar == NULL)
		{
			CBCGPDockManager* pDockManager = 
				globalUtils.GetDockManager (GetDockSite ());
			ASSERT_VALID (pDockManager);
			pDockManager->DockControlBar (this);
			return TRUE;
		}
	}

	ASSERT_KINDOF (CBCGPDockBar, pDockBar);

	// check whether the control bar can be docked at the given DockBar 
	if (!CanBeDocked (pDockBar) || !pDockBar->CanAcceptBar (this))
	{
		return FALSE;
	}

	// save the window rectandle of the control bar, because it will be adjusted in the
	// moment when the parent is changed
	CRect rect;
	rect.SetRectEmpty ();
	GetWindowRect (&rect);


	BOOL bWasCaptured = TRUE;
	CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame ();
	if (pMiniFrame != NULL)
	{
		bWasCaptured = pMiniFrame->IsCaptured ();
	}

	if (pDockBar != NULL)
	{
		pDockBar->SetRedraw (FALSE);
	}

	PrepareToDock ((CBCGPDockBar*)pDockBar, dockMethod);
	
	if (dockMethod == BCGP_DM_MOUSE)
	{
		m_pParentDockBar->DockControlBar (this, dockMethod, &rect);
		if (bWasCaptured)
		{
			OnContinueMoving ();
		}
	}
	else if (dockMethod == BCGP_DM_RECT || dockMethod == BCGP_DM_DBL_CLICK)
	{
		m_pParentDockBar->DockControlBar (this, dockMethod, lpRect);
	}

    if (pDockBar != NULL)
    {
		pDockBar->SetRedraw (TRUE);
    }

	return TRUE;
}
//***********************************************************************************
void CBCGPControlBar::PrepareToDock (CBCGPDockBar* pDockBar, BCGP_DOCK_METHOD dockMethod)
{
	if (pDockBar != NULL)
	{		
		m_pParentDockBar = DYNAMIC_DOWNCAST (CBCGPDockBar, pDockBar);	
		ASSERT_VALID (m_pParentDockBar);
		// remove the control bar from its miniframe
		RemoveFromMiniframe (pDockBar, dockMethod);

		// align correctly and turn on all borders
		DWORD dwStyle = GetBarStyle();
		dwStyle &= ~(CBRS_ALIGN_ANY);
		dwStyle |=  (m_dwStyle & CBRS_ALIGN_ANY) | CBRS_BORDER_ANY;

		dwStyle &= ~CBRS_FLOATING;
		SetBarStyle (dwStyle);

		SetBarAlignment (pDockBar->GetCurrentAlignment ());
	}
}
//***********************************************************************************
void CBCGPControlBar::RemoveFromMiniframe (CWnd* pNewParent, BCGP_DOCK_METHOD dockMethod)
{
	ASSERT_VALID (this);

	CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame ();

	CWnd* pOldParent = GetParent ();
	// reassign the parentship to the new parent
	OnBeforeChangeParent (pNewParent);

	// miniframe will be NULL if the bar has never been floating
	if (pMiniFrame != NULL)
	{
		// remove the control bar from its miniframe
		// DO NOT destroy miniframe meanwhile

		// delay destroying of miniframes only in case if it's the first miniframe after
		// the control bar has been captured

		// dockMethod == BCGP_DM_DBL_CLICK - would be required to prevent crash while canceling
		// drag from tab window to main frame edge - see support from 30/06/2004,
		// but currently can't be reproduced. Left here until testing has been finished
		BOOL bDelayDestroy = ((dockMethod == BCGP_DM_MOUSE /*|| dockMethod == BCGP_DM_DBL_CLICK*/) && 
							  m_hwndMiniFrameToBeClosed == NULL);

		pMiniFrame->RemoveControlBar (this, FALSE, bDelayDestroy);
		if ((dockMethod == BCGP_DM_MOUSE /*|| dockMethod == BCGP_DM_DBL_CLICK*/) && 
			 m_hwndMiniFrameToBeClosed == NULL /*&& 
			 pMiniFrame->GetControlBarCount () == 0*/)
		{
			m_hwndMiniFrameToBeClosed = pMiniFrame->GetSafeHwnd ();
		}
		if (dockMethod == BCGP_DM_MOUSE)
		{
			pMiniFrame->SendMessage (WM_LBUTTONUP, 0, 0);
		}
	}
	
	if (pNewParent != NULL)
	{
		SetParent (pNewParent);
	}
	OnAfterChangeParent (pOldParent);
}
//***********************************************************************************
BOOL CBCGPControlBar::OnBeforeDock (CBCGPBaseControlBar** /*ppDockBar*/, LPCRECT /*lpRect*/, BCGP_DOCK_METHOD /*dockMethod*/)
{
	ASSERT_VALID (this);
	CBCGPMiniFrameWnd* pParentMiniFrame = GetParentMiniFrame ();
	if (pParentMiniFrame != NULL)
	{
		m_bPinState = pParentMiniFrame->GetPinState ();
	}
	return TRUE;
}
//***********************************************************************************
BOOL CBCGPControlBar::OnBeforeFloat (CRect& /*rectFloat*/, BCGP_DOCK_METHOD /*dockMethod*/)
{
	ASSERT_VALID (this);
	return TRUE;
}
//***********************************************************************************
void CBCGPControlBar::OnAfterFloat  ()
{
	ASSERT_VALID (this);
	SetBarAlignment (CBRS_ALIGN_TOP);
	CBCGPMiniFrameWnd* pParentMiniFrame = GetParentMiniFrame ();
	if (pParentMiniFrame != NULL)
	{
		pParentMiniFrame->Pin (m_bPinState);
		pParentMiniFrame->SetWindowPos (NULL, -1, -1, -1, -1, 
				SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
			
	}
	if (CBCGPControlBar::m_bHandleMinSize)
	{
		SetWindowRgn (NULL, TRUE);
	}
}
//***********************************************************************************
void CBCGPControlBar::OnContinueMoving ()
{
	ASSERT_VALID (this);
	// continue moving
	EnterDragMode (FALSE);
}
//***********************************************************************************
BOOL CBCGPControlBar::FloatControlBar (CRect rectFloat, BCGP_DOCK_METHOD dockMethod, 
									   bool bShow)
{
	ASSERT_VALID (this);
	if (!IsDocked () && !IsTabbed ())
	{
		return TRUE;
	}

	if (!CanFloat () || (GetStyle() & WS_CHILD) == 0)
	{
		return TRUE;
	}

	CRect rectBeforeFloat;
	GetWindowRect (rectBeforeFloat);

	CWnd* pDockSite = GetDockSite ();
	ASSERT_VALID (pDockSite);

	pDockSite->ScreenToClient (rectBeforeFloat);

	CPoint ptMouseScreen; //mouse coords
	GetCursorPos (&ptMouseScreen);

	CPoint ptScreen = m_ptClientHotSpot;
	ClientToScreen (&ptScreen);

	if (!OnBeforeFloat (rectFloat, dockMethod))
	{
		return TRUE;
	}
	
	CRect rectDelta (16, 16, 16, 16);
	globalUtils.AdjustRectToWorkArea (rectFloat, &rectDelta);

	// create miniframe if it does not exist and move it if it does exist
	CBCGPMiniFrameWnd* pParentMiniFrame = CreateDefaultMiniframe (rectFloat);
	if (pParentMiniFrame == NULL)
	{
		return FALSE;
	}

	CBCGPDockManager* pDockManager = globalUtils.GetDockManager (GetDockSite ());
	if (dockMethod != BCGP_DM_MOUSE && 
		pDockManager != NULL && 
		!pDockManager->m_bRestoringDockState)
	{
		StoreRecentDockInfo ();
	}

	// this rectangle does not take into account miniframe caption height and borders
	pParentMiniFrame->m_rectRecentFloatingRect = 
		m_recentDockInfo.m_rectRecentFloatingRect;

	OnBeforeChangeParent (pParentMiniFrame);

	CPoint ptMouseClient = ptMouseScreen;
	ScreenToClient (&ptMouseClient);

	if (dockMethod == BCGP_DM_MOUSE)
	{
		SendMessage (WM_LBUTTONUP, 0xFFFF, MAKELPARAM (ptMouseClient.x, ptMouseClient.y));
		if (IsTabbed ())
		{
			CBCGPMiniFrameWnd* pWnd = GetParentMiniFrame ();
			if (pWnd != NULL)
			{
				pWnd->SendMessage (WM_LBUTTONUP, 0, MAKELPARAM (ptMouseClient.x, ptMouseClient.y));
			}
		}
	}	

	CWnd* pOldParent = GetParent ();
	SetParent (pParentMiniFrame);

	if (m_pParentDockBar != NULL)
	{
		OnAfterChangeParent (m_pParentDockBar);
		m_pParentDockBar = NULL;
	}
	else
	{
		OnAfterChangeParent (pOldParent);
	}

	pParentMiniFrame->AddControlBar (this);

	//move control bar to the top left corner of the miniframe
	pParentMiniFrame->CheckGripperVisibility ();
	SetWindowPos (&wndTop, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
	
	if (dockMethod == BCGP_DM_MOUSE && (GetDockMode () & BCGP_DT_IMMEDIATE) != 0)
	{
		pParentMiniFrame->EnterDragMode ();
		// block the first MouseMove to prevent immediate docking
		pParentMiniFrame->m_bBlockMove = true;
	}

	OnAfterFloat ();

	DWORD dwStyle = GetBarStyle ();
	dwStyle |= CBRS_FLOATING;
	SetBarStyle (dwStyle);
		
	RecalcLayout ();

	if (bShow)
	{
		GetParentMiniFrame ()->AdjustLayout ();
	}

	// move the default miniframe so the client hot spot will be on place
		// move the default miniframe so the client hot spot will be on place
	if (dockMethod == BCGP_DM_MOUSE)
	{
		CRect rectFinalMiniFrame;
		pParentMiniFrame->GetWindowRect (rectFinalMiniFrame);

		ptScreen = m_ptClientHotSpot;

		pParentMiniFrame->ClientToScreen (&ptScreen);

		if (ptScreen.x > rectFinalMiniFrame.left + rectFinalMiniFrame.Width () ||
			ptScreen.x < rectFinalMiniFrame.left)
		{
			ptScreen.x = rectFinalMiniFrame.left + rectFinalMiniFrame.Width () / 2;
		}

		if (ptScreen.y > rectFinalMiniFrame.top + rectFinalMiniFrame.Height () || 
			ptScreen.y < rectFinalMiniFrame.top)
		{
			ptScreen.y = rectFinalMiniFrame.top + pParentMiniFrame->GetCaptionHeight () / 2;
		}

		CPoint ptOffset = ptMouseScreen - ptScreen;

		rectFinalMiniFrame.OffsetRect (ptOffset);

		pParentMiniFrame->SetWindowPos (NULL, rectFinalMiniFrame.left, 
											  rectFinalMiniFrame.top,
											  rectFinalMiniFrame.Width (),
											  rectFinalMiniFrame.Height (),
											  SWP_NOZORDER | SWP_NOACTIVATE);

		pParentMiniFrame->SetHotPoint (ptMouseScreen);
	}

	if (bShow)
	{
		pParentMiniFrame->ShowWindow (SW_SHOWNA);
		GetDockSite ()->RedrawWindow (&rectBeforeFloat, NULL, RDW_FRAME | RDW_INVALIDATE | 
										RDW_UPDATENOW | RDW_ALLCHILDREN |
										RDW_NOINTERNALPAINT | RDW_NOERASE);
		if (GetDockSite ()->IsKindOf (RUNTIME_CLASS (CBCGPOleCntrFrameWnd)))
		{
			((CBCGPOleCntrFrameWnd*) GetDockSite ())->AdjustDockingLayout ();
		}	

		if (CanFocus ())
		{
			pParentMiniFrame->SetFocus ();
		}
	}

	return TRUE;
}
//***********************************************************************************
void CBCGPControlBar::OnBeforeChangeParent (CWnd* pWndNewParent, BOOL bDelay) 
{
	ASSERT_VALID (this);
	if (m_pParentDockBar != NULL)
	{
		m_pParentDockBar->RemoveControlBar (this, BCGP_DM_UNKNOWN);
	}

	CBCGPBaseControlBar::OnBeforeChangeParent (pWndNewParent, bDelay);
}
//***********************************************************************************
void CBCGPControlBar::OnAfterChangeParent  (CWnd* pWndOldParent) 
{
	ASSERT_VALID (this);
	UpdateVirtualRect ();

	CWnd* pParent = GetParent ();

	if (pParent->GetSafeHwnd() == NULL || !pParent->IsKindOf (RUNTIME_CLASS (CBCGPDockBar)))
	{
		m_pParentDockBar = NULL;
		m_pDockBarRow = NULL;
	}
	CBCGPBaseControlBar::OnAfterChangeParent (pWndOldParent);
}
//***********************************************************************************
BOOL CBCGPControlBar::MoveByAlignment (DWORD dwAlignment, int nOffset)
{
	ASSERT_VALID (this);

	CRect rect;
	GetWindowRect (rect);

	CWnd* pParentWnd = GetParent ();

	ASSERT_VALID (pParentWnd);
	pParentWnd->ScreenToClient (&rect);

	switch (dwAlignment & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_LEFT:
	case CBRS_ALIGN_RIGHT:
		rect.OffsetRect (nOffset, 0);
		UpdateVirtualRect (CPoint (nOffset, 0));
		break;
		
	case CBRS_ALIGN_TOP:
	case CBRS_ALIGN_BOTTOM:
		rect.OffsetRect (0, nOffset);
		UpdateVirtualRect (CPoint (0, nOffset));
		break;
	}

	return (BOOL) (SetWindowPos (&wndTop, rect.left, rect.top, rect.Width (), rect.Height (), 
							SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE) != 0);

}
//***********************************************************************************
CSize CBCGPControlBar::MoveControlBar (CRect rectNew, BOOL bForceMove, HDWP& /*hdwp*/)
{
	ASSERT_VALID (this);

	CSize sizeMin;
	CSize sizeNew = rectNew.Size ();	
	
	GetMinSize (sizeMin);

	CRect rectCurrent;
	GetWindowRect (rectCurrent);

	CSize sizeActual = rectNew.Size () - rectCurrent.Size ();

	if (!bForceMove && abs (sizeNew.cx) < sizeMin.cx)
	{
		if (rectCurrent.left == rectNew.left ||
			rectCurrent.left != rectNew.left && 
			rectCurrent.right != rectNew.right)
		{
			rectNew.right = rectCurrent.left + sizeMin.cx;
		}
		else if (rectCurrent.right == rectNew.right)
		{
			rectNew.left = rectCurrent.right - sizeMin.cx;			
		}
		sizeActual.cx = rectCurrent.Width () - rectNew.Width ();  
	}

	if (!bForceMove && abs (sizeNew.cy) < sizeMin.cy)
	{
		if (rectCurrent.top == rectNew.top || 
			rectCurrent.top != rectNew.top && 
			rectCurrent.bottom != rectNew.bottom)
		{
			rectNew.bottom = rectCurrent.top + sizeMin.cy;
		} 
		else if (rectCurrent.bottom == rectNew.bottom)
		{
			rectNew.top = rectCurrent.bottom - sizeMin.cy;
		}

		sizeActual.cy = rectCurrent.Height () - rectNew.Height ();  
	}

	ASSERT_VALID (GetParent ());
	GetParent ()->ScreenToClient (rectNew);
	MoveWindow (rectNew);
	return sizeActual;
}
//***********************************************************************************
int CBCGPControlBar::StretchBar (int nStretchSize, HDWP& /*hdwp*/)
{
	ASSERT_VALID (this);

	// the bar is stretched - calculate how far it can be expanded and do not
	// exceed its original size
	int nAvailExpandSize = GetAvailableExpandSize ();
	int nAvailStretchSize = GetAvailableStretchSize ();

	int nActualStretchSize = 0;
	if (nStretchSize > 0)
	{
		if (nAvailExpandSize == 0)
		{
			return 0;
		}
		// the bar is expanded 
		nActualStretchSize = nAvailExpandSize > nStretchSize ? nStretchSize : nAvailExpandSize;
	}
	else
	{
		nActualStretchSize = nAvailStretchSize < abs (nStretchSize) ? -nAvailStretchSize : nStretchSize;
	}


	CRect rect;
	GetWindowRect (rect);

	if (IsHorizontal ())
	{
		rect.right += nActualStretchSize; 
	}
	else
	{
		rect.bottom += nActualStretchSize; 
	}	

	OnBeforeStretch (nActualStretchSize);

	if (abs (nActualStretchSize) > 0)
	{
		ASSERT_VALID (GetParent ());
		GetParent ()->ScreenToClient (rect);
		MoveWindow (rect);
		OnAfterStretch (nActualStretchSize);
	}

	return nActualStretchSize;
}
//***********************************************************************************
int CBCGPControlBar::GetAvailableExpandSize () const
{
	ASSERT_VALID (this);

	CRect rect;
	GetWindowRect (rect);

	// can't expand beyond virtual rect
	if ((IsHorizontal () && rect.Width () >= m_rectVirtual.Width ()) || 
		(!IsHorizontal () && rect.Height () >= m_rectVirtual.Height ()))
	{
		return 0;
	}


	return IsHorizontal () ? m_rectVirtual.Width () - rect.Width () : 
							m_rectVirtual.Height () - rect.Height ();
}
//***********************************************************************************
int CBCGPControlBar::GetAvailableStretchSize () const
{
	ASSERT_VALID (this);

	CRect rect;
	GetWindowRect (rect);

	CSize sizeMin; 
	GetMinSize (sizeMin);
	
	return IsHorizontal () ? rect.Width () - sizeMin.cx: 
							 rect.Height () - sizeMin.cy;
}
//***********************************************************************************
CSize CBCGPControlBar::CalcAvailableSize (CRect rectRequired)
{
	ASSERT_VALID (this);

	CSize sizeMin;
	GetMinSize (sizeMin);

	CSize sizeAvailable (0, 0);	

	if (rectRequired.Width () < sizeMin.cx)
	{
		rectRequired.right = rectRequired.left + sizeMin.cx;
	}

	if (rectRequired.Height () < sizeMin.cy)
	{
		rectRequired.bottom = rectRequired.top + sizeMin.cy;
	}
	
	CRect rectCurrent;
	GetWindowRect (rectCurrent);

	// available space is negative when stretching
	sizeAvailable.cx = rectRequired.Width () - rectCurrent.Width ();
	sizeAvailable.cy = rectRequired.Height () - rectCurrent.Height ();
	
	return sizeAvailable;
}
//***********************************************************************************
bool CBCGPControlBar::IsLeftOf (CRect rect, bool bWindowRect) const
{
	ASSERT_VALID (this);
	if (m_pParentDockBar == NULL)
	{
		return true;
	}

	CRect rectBar;
	GetWindowRect (&rectBar);

	if (!bWindowRect) 
	{
		m_pParentDockBar->ScreenToClient (&rectBar);
	}

	if (m_pParentDockBar->IsHorizontal ())
	{
		return (rect.left < rectBar.left);
	}
	else
	{
		return (rect.top < rectBar.top);
	}
}
//***********************************************************************************
bool CBCGPControlBar::IsLastBarOnLastRow () const
{
	ASSERT_VALID (this);
	if (m_pParentDockBar->IsLastRow (m_pDockBarRow))
	{
		return (m_pDockBarRow->GetBarCount () == 1);
	}
	return false;
}
//***********************************************************************************
BCGP_CS_STATUS CBCGPControlBar::IsChangeState (int nOffset, 
											 CBCGPBaseControlBar** ppTargetBar) const
{
	ASSERT_VALID (this);
	ASSERT (ppTargetBar != NULL);
	

	CPoint	ptMousePos;
	CRect	rectBarWnd;
	CRect	rectDockBarWnd;
	CRect	rectIntersect;

	CRect	rectVirtual;

	CPoint  ptDelta;

	GetCursorPos (&ptMousePos);

	GetWindowRect  (&rectBarWnd);
	GetVirtualRect (rectVirtual);

	// check whether the mouse is around a dock bar
	CBCGPBaseControlBar* pBaseBar = ControlBarFromPoint (ptMousePos, nOffset, FALSE,
											RUNTIME_CLASS (CBCGPDockBar)); 

	*ppTargetBar = DYNAMIC_DOWNCAST (CBCGPDockBar, pBaseBar);

	if (m_pParentDockBar != NULL)
	{
		// the mouse is around the dock bar, check the virtual rect
		m_pParentDockBar->GetWindowRect (&rectDockBarWnd);
		if (!rectIntersect.IntersectRect (rectDockBarWnd, rectVirtual))
		{
			return BCGP_CS_DOCK_IMMEDIATELY;
		}

		// there is some intersection of the virtual rectangle an the dock bar. 
		// special processing when horizontal bar is about to float in horizontal direction
		bool bTreatMouse = false; 
		if (m_pParentDockBar->IsHorizontal ())
		{

			if (rectVirtual.left < rectDockBarWnd.left && 
				rectDockBarWnd.left - rectVirtual.left > nOffset * 2 ||
				rectVirtual.right > rectDockBarWnd.right && 
				rectVirtual.right - rectDockBarWnd.right > nOffset * 2)
			{
				bTreatMouse = true;	
			}
		}
		else
		{
			if (rectVirtual.top < rectDockBarWnd.top && 
				rectDockBarWnd.top - rectVirtual.top > nOffset * 2 ||
				rectVirtual.bottom > rectDockBarWnd.bottom && 
				rectVirtual.bottom - rectDockBarWnd.bottom > nOffset * 2)
			{
				bTreatMouse = true;	
			}
			
		}

		if (bTreatMouse && !rectDockBarWnd.PtInRect (ptMousePos))
		{
			return BCGP_CS_DOCK_IMMEDIATELY;
		}
	}
	else
	{
		if (*ppTargetBar == NULL)
		{
			// the mouse is out of dock bar in either direction - keep the bar floating
			return BCGP_CS_NOTHING;
		}

		if (!CanBeDocked (*ppTargetBar))
		{
			// bar's style does not allow to dock the bar to this dock bar
			return BCGP_CS_NOTHING;
		}
		// the mouse is getting closer to a dock bar
		(*ppTargetBar)->GetWindowRect (&rectDockBarWnd);

		if (rectDockBarWnd.PtInRect (ptMousePos))
		{
			// the mouse is over the dock bar, the bar must be docked
			return BCGP_CS_DOCK_IMMEDIATELY;
		}

		// check on which side the mouse is relatively to the dock bar
		bool bMouseLeft = ptMousePos.x < rectDockBarWnd.left;
		bool bMouseRight = ptMousePos.x > rectDockBarWnd.right;
		bool bMouseTop  = ptMousePos.y < rectDockBarWnd.top;
		bool bMouseBottom = ptMousePos.y > rectDockBarWnd.bottom;

		double	dPixelsOnDock = nOffset;
		int		nMouseOffset  = 0;
		if (bMouseLeft)
		{
			dPixelsOnDock = ((rectBarWnd.right - ptMousePos.x) * 100. / 
								rectBarWnd.Width ()) / 100. * nOffset;
			nMouseOffset = rectDockBarWnd.left - ptMousePos.x;
			
		}
		else if (bMouseRight)
		{
			dPixelsOnDock = ((ptMousePos.x - rectBarWnd.left) * 100. / 
								rectBarWnd.Width ()) / 100. * nOffset;
			nMouseOffset = ptMousePos.x - rectDockBarWnd.right;
		}
		else if (bMouseTop)
		{
			dPixelsOnDock = ((rectBarWnd.bottom - ptMousePos.y) * 100. / 
								rectBarWnd.Height ()) / 100. * nOffset;
			nMouseOffset = rectDockBarWnd.top - ptMousePos.y;
		}
		else if (bMouseBottom)
		{
			dPixelsOnDock = ((ptMousePos.y - rectBarWnd.top) * 100. / 
								rectBarWnd.Height ()) / 100. * nOffset;
			nMouseOffset = ptMousePos.y - rectDockBarWnd.bottom;
		}

		if (nMouseOffset <= dPixelsOnDock)
		{
			return BCGP_CS_DOCK_IMMEDIATELY;
		}
	}

	return BCGP_CS_NOTHING;
}
//***********************************************************************************
CBCGPMiniFrameWnd* CBCGPControlBar::CreateDefaultMiniframe (CRect rectInitial)
{
	ASSERT_VALID (this);

	CRect rectVirtual = rectInitial;

	CBCGPMiniFrameWnd* pMiniFrame =  
		(CBCGPMiniFrameWnd*) m_pMiniFrameRTC->CreateObject ();
	
	if (pMiniFrame != NULL)
	{
		// it must have valid BCGFrame window as parent
		CWnd* pParentFrame = BCGPGetParentFrame (this);
		ASSERT_VALID (pParentFrame);

		pMiniFrame->SetDockManager (globalUtils.GetDockManager (GetDockSite ()));
		
		if (pParentFrame == NULL || !pMiniFrame->Create (NULL, WS_POPUP, rectVirtual, pParentFrame))
		{
			TRACE0 ("Failed to create miniframe");
			delete pMiniFrame;
			return NULL;
		}
	}
	else
	{
		TRACE0 ("Failed to create miniframe using runtime class information \n");
		ASSERT (FALSE);
	}
	return pMiniFrame;
}
//***********************************************************************************
void CBCGPControlBar::UpdateVirtualRect ()
{
	ASSERT_VALID (this);

	GetWindowRect (m_rectVirtual);

	CSize size = CalcFixedLayout (FALSE, IsHorizontal ());
	
	m_rectVirtual.right = m_rectVirtual.left + size.cx;
	m_rectVirtual.bottom = m_rectVirtual.top + size.cy;

	if (GetParent () != NULL) 
	{
		GetParent ()->ScreenToClient (m_rectVirtual);
	}

}
//***********************************************************************************
void CBCGPControlBar::UpdateVirtualRect (CPoint ptOffset) 
{
	ASSERT_VALID (this);
	CWnd* pParent = GetParent();

	if (pParent->GetSafeHwnd() != NULL && (pParent->GetExStyle () & WS_EX_LAYOUTRTL) && IsHorizontal ())
	{
		ptOffset.x = -ptOffset.x;
		m_rectVirtual.OffsetRect (ptOffset);
	}
	else
	{
		m_rectVirtual.OffsetRect (ptOffset);
	}
}
//***********************************************************************************//
void CBCGPControlBar::UpdateVirtualRect (CSize sizeNew)
{
	ASSERT_VALID (this);

	GetWindowRect (m_rectVirtual);
	
	m_rectVirtual.right = m_rectVirtual.left + sizeNew.cx;
	m_rectVirtual.bottom = m_rectVirtual.top + sizeNew.cy;

	if (GetParent () != NULL) 
	{
		GetParent ()->ScreenToClient (m_rectVirtual);
	}
}
//***********************************************************************************
void CBCGPControlBar::GetVirtualRect (CRect& rectVirtual) const 
{
	ASSERT_VALID (this);
	rectVirtual = m_rectVirtual;
	ASSERT_VALID (GetParent ());
	GetParent ()->ClientToScreen (rectVirtual);
}
//***********************************************************************************
void CBCGPControlBar::SetVirtualRect (const CRect& rect, BOOL bMapToParent) 
{
	ASSERT_VALID (this);
	m_rectVirtual = rect;
	ASSERT_VALID (GetParent ());
	if (bMapToParent)
	{
		MapWindowPoints (GetParent (), m_rectVirtual);
	}
}
//***********************************************************************************
void CBCGPControlBar::OnDestroy() 
{
	if (IsTabbed ())
	{
		CWnd* pParent = GetParent ();
		ASSERT_VALID (pParent);

		if (pParent->IsKindOf (RUNTIME_CLASS (CBCGPBaseTabWnd)))
		{
			pParent = pParent->GetParent ();
			ASSERT_VALID (pParent);
		}
		
		if (pParent->IsKindOf (RUNTIME_CLASS (CBCGPBaseTabbedBar)))
		{
			CBCGPBaseTabbedBar* pTabbedBar = DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, pParent);
			ASSERT (pTabbedBar != NULL);

			HWND hwnd = m_hWnd;
			pTabbedBar->RemoveControlBar (this);
			if (!IsWindow (hwnd))
			{
				// the control bar has been destroyed by RemoveControlBar
				return;
			}
		}
	}

	CBCGPBaseControlBar::OnDestroy();
}
//***********************************************************************************
void CBCGPControlBar::OnNcDestroy() 
{
	ASSERT_VALID (this);
	CBCGPMiniFrameWnd::AddRemoveBarFromGlobalList (this, FALSE /* remove*/);
	ASSERT_VALID (this);

	CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame (TRUE);

	if (pMiniFrame != NULL)
		pMiniFrame->RemoveControlBar (this, FALSE);

	CBCGPBaseControlBar::OnNcDestroy();
}
//***********************************************************************************
// MFC's control bar compatibility 
//***********************************************************************************
BOOL CBCGPControlBar::AllocElements(int nElements, int cbElement)
{
	ASSERT_VALID(this);
	ASSERT(nElements >= 0 && cbElement >= 0);
	ASSERT(m_pData != NULL || m_nCount == 0);

	// allocate new data if necessary
	void* pData = NULL;
	if (nElements > 0)
	{
		ASSERT(cbElement > 0);
		if ((pData = calloc(nElements, cbElement)) == NULL)
			return FALSE;
	}

	free (m_pData);      // free old data

	// set new data and elements
	m_pData = pData;
	m_nCount = nElements;

	return TRUE;
}
//-------------------------------------------------------------------------------------//
void CBCGPControlBar::CalcInsideRect(CRect& rect, BOOL bHorz) const
{
	ASSERT_VALID(this);
	DWORD dwStyle = GetBarStyle (); 

	if (!IsFloating () && !IsTabbed ())
	{
		if (dwStyle & CBRS_BORDER_LEFT)
			rect.left += CX_BORDER;
		if (dwStyle & CBRS_BORDER_TOP)
			rect.top += CY_BORDER;
		if (dwStyle & CBRS_BORDER_RIGHT)
			rect.right -= CX_BORDER;
		if (dwStyle & CBRS_BORDER_BOTTOM)
			rect.bottom -= CY_BORDER;
	}

	// inset the top and bottom.
	if (bHorz)
	{
		rect.left += m_cxLeftBorder;
		rect.top += m_cyTopBorder;
		rect.right -= m_cxRightBorder;
		rect.bottom -= m_cyBottomBorder;

		if ((dwStyle & (CBRS_GRIPPER|CBRS_FLOATING)) == CBRS_GRIPPER)
		{
			if (GetExStyle() & WS_EX_LAYOUTRTL)
			{
				rect.right -= CX_BORDER_GRIPPER+CX_GRIPPER+CX_BORDER_GRIPPER;
			}
			else
			{
				rect.left += CX_BORDER_GRIPPER+CX_GRIPPER+CX_BORDER_GRIPPER;
			}
		}
	}
	else
	{
		rect.left += m_cyTopBorder;
		rect.top += m_cxLeftBorder;
		rect.right -= m_cyBottomBorder;
		rect.bottom -= m_cxRightBorder;

		if ((dwStyle & (CBRS_GRIPPER|CBRS_FLOATING)) == CBRS_GRIPPER)
		{
			rect.top += CY_BORDER_GRIPPER+CY_GRIPPER+CY_BORDER_GRIPPER;
		}
	}
}
//-------------------------------------------------------------------------------------//
void CBCGPControlBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	OnProcessDblClk ();

	if (CanFloat ())
	{
		FloatControlBar (m_recentDockInfo.m_rectRecentFloatingRect, BCGP_DM_DBL_CLICK);
		CBCGPBaseControlBar::OnLButtonDblClk(nFlags, point);
	}
}
//-------------------------------------------------------------------------------------//
void CBCGPControlBar::OnProcessDblClk ()
{
	m_bDblClick = true;

	StoreRecentDockInfo ();

	if (m_bCaptured)
	{
		ReleaseCapture ();

		m_bCaptured = false;
		SetDragMode (FALSE);

		if (m_hwndMiniFrameToBeClosed != NULL && ::IsWindow (m_hwndMiniFrameToBeClosed))
		{
			::DestroyWindow (m_hwndMiniFrameToBeClosed);
		}

		m_hwndMiniFrameToBeClosed = NULL;
	}
}
//-------------------------------------------------------------------------------------//
BOOL CBCGPControlBar::IsTabbed () const
{
	CWnd* pImmediateParent = GetParent ();
	if (pImmediateParent == NULL)
	{
		return FALSE;
	}

	CWnd* pNextParent = pImmediateParent->GetParent ();
	return (pNextParent != NULL) &&
		((pImmediateParent->IsKindOf (RUNTIME_CLASS (CBCGPBaseTabWnd)) &&
		 (pNextParent->IsKindOf (RUNTIME_CLASS (CBCGPBaseTabbedBar)) ||
		  pNextParent->IsKindOf (RUNTIME_CLASS (CBCGPTabbedToolbar)))) ||
		 (pImmediateParent->IsKindOf (RUNTIME_CLASS (CBCGPDockingCBWrapper)) &&
		  pNextParent->IsKindOf (RUNTIME_CLASS (CBCGPBaseTabWnd))));
}
//------------------------------------------------------------------------------------
void CBCGPControlBar::SetDragMode (BOOL bOnOff)
{
	m_bDragMode = bOnOff;
}
//------------------------------------------------------------------------------------
void CBCGPControlBar::OnContextMenu(CWnd* /*pWnd*/, CPoint point) 
{
	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return;
	}

	if (!CBCGPToolBar::IsCustomizeMode ())
	{
		if (OnShowControlBarMenu (point))
		{
			return;
		}

		CFrameWnd* pParentFrame = DYNAMIC_DOWNCAST (CFrameWnd, m_pDockSite);
		if (pParentFrame == NULL)
		{
			pParentFrame = BCGCBProGetTopLevelFrame (this);
		}

		if (pParentFrame != NULL)
		{
			ASSERT_VALID(pParentFrame);

			OnControlBarContextMenu (pParentFrame, point);
		}
	}
}
//------------------------------------------------------------------------------------
BOOL CBCGPControlBar::LoadState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::BCGPGetRegPath (strControlBarProfile, lpszProfileName);

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID ();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);
	}
	else
	{
		strSection.Format (REG_SECTION_FMT_EX, strProfileName, nIndex, uiID);
	}

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (strSection))
	{
		return FALSE;
	}

	reg.Read (_T ("ID"), (int&) m_nID);

	reg.Read (_T ("RectRecentFloat"), m_recentDockInfo.m_rectRecentFloatingRect);
	reg.Read (_T ("RectRecentDocked"), m_rectSavedDockedRect);

	// !!!!!! change to appropriate handling for slider/frame
	m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect = m_rectSavedDockedRect;

	reg.Read (_T ("RecentFrameAlignment"), m_recentDockInfo.m_dwRecentAlignmentToFrame);
	reg.Read (_T ("RecentRowIndex"), m_recentDockInfo.m_nRecentRowIndex);
	reg.Read (_T ("IsFloating"), m_bRecentFloatingState);
	reg.Read (_T ("MRUWidth"), m_nMRUWidth);
	reg.Read (_T ("PinState"), m_bPinState);

	return CBCGPBaseControlBar::LoadState (lpszProfileName, nIndex, uiID);	
}
//------------------------------------------------------------------------------------
BOOL CBCGPControlBar::SaveState (LPCTSTR lpszProfileName, int nIndex, UINT uiID)
{
	CString strProfileName = ::BCGPGetRegPath (strControlBarProfile, lpszProfileName);

	if (nIndex == -1)
	{
		nIndex = GetDlgCtrlID ();
	}

	CString strSection;
	if (uiID == (UINT) -1)
	{
		strSection.Format (REG_SECTION_FMT, strProfileName, nIndex);
	}
	else
	{
		strSection.Format (REG_SECTION_FMT_EX, strProfileName, nIndex, uiID);
	}

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		BOOL bFloating = IsFloating ();

		if (bFloating)
		{
			CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame ();
			if (pMiniFrame != NULL)
			{
				pMiniFrame->GetWindowRect (m_recentDockInfo.m_rectRecentFloatingRect);
			}
		}
		else
		{
			CalcRecentDockedRect ();
			if (m_pParentDockBar != NULL)
			{
				m_recentDockInfo.m_dwRecentAlignmentToFrame = m_pParentDockBar->GetCurrentAlignment ();
				m_recentDockInfo.m_nRecentRowIndex = m_pParentDockBar->FindRowIndex (m_pDockBarRow);
			}
		}

		reg.Write (_T ("ID"), (int&)m_nID);

		reg.Write (_T ("RectRecentFloat"), m_recentDockInfo.m_rectRecentFloatingRect);
		reg.Write (_T ("RectRecentDocked"), m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect);
		
		reg.Write (_T ("RecentFrameAlignment"), m_recentDockInfo.m_dwRecentAlignmentToFrame);
		reg.Write (_T ("RecentRowIndex"), m_recentDockInfo.m_nRecentRowIndex);
		reg.Write (_T ("IsFloating"), bFloating);
		reg.Write (_T ("MRUWidth"), m_nMRUWidth);
		reg.Write (_T ("PinState"), m_bPinState);
	}
	return CBCGPBaseControlBar::SaveState (lpszProfileName, nIndex, uiID);	
}
//------------------------------------------------------------------------------------
void CBCGPControlBar::CalcRecentDockedRect ()
{
	GetWindowRect (m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect);
			
	if (m_pParentDockBar != NULL)
	{
		m_pParentDockBar->ScreenToClient (m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect);
	}
	else if (GetDockSite () != NULL)
	{
		GetDockSite ()->ScreenToClient (m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect);
	}
}
//------------------------------------------------------------------------------------
void CBCGPControlBar::SetDockState (CBCGPDockManager* pDockManager)
{
	ASSERT_VALID (this);

	
	if (m_bRecentFloatingState)
	{
		ShowControlBar (GetRecentVisibleState (), TRUE, FALSE);
	}
	else 
	{
		CBCGPDockBar* pDockBar = 
			pDockManager->FindDockBar (m_recentDockInfo.m_dwRecentAlignmentToFrame, TRUE);
		
		if (pDockBar != NULL)
		{
			pDockManager->DockControlBar (this, pDockBar->GetDockBarID (), 
											m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect);
		}

		if (m_pParentDockBar != NULL)
		{
			m_pParentDockBar->ShowControlBar (this, GetRecentVisibleState (), TRUE, FALSE);
			if (m_pDockBarRow != NULL)
			{
				m_pDockBarRow->ExpandStretchedBars ();
			}
		}
	}
}
//------------------------------------------------------------------------------------
void CBCGPControlBar::OnCancelMode() 
{
	CBCGPBaseControlBar::OnCancelMode();
	if (m_bCaptured)
	{
		if ((GetDockMode () & BCGP_DT_STANDARD) != 0)
		{
			m_dragFrameImpl.EndDrawDragFrame ();	
		}

		ReleaseCapture ();
 		m_bCaptured = false;
		SetDragMode (FALSE);

		if (m_hwndMiniFrameToBeClosed != NULL && ::IsWindow (m_hwndMiniFrameToBeClosed))
		{
			::DestroyWindow (m_hwndMiniFrameToBeClosed);
		}

		m_hwndMiniFrameToBeClosed = NULL;
	}
}
//------------------------------------------------------------------------------------
void CBCGPControlBar::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_ESCAPE)
	{
		OnCancelMode ();
	}
	CBCGPBaseControlBar::OnChar(nChar, nRepCnt, nFlags);
}
//------------------------------------------------------------------------------------
void CBCGPControlBar::SetActiveInGroup (BOOL bActive)
{
	m_bActiveInGroup = bActive;
}
//------------------------------------------------------------------------------------
void CBCGPControlBar::UnDockControlBar (BOOL bDelay)
{
	ASSERT_VALID (this);
	if (m_pParentDockBar != NULL)
	{
		m_pParentDockBar->RemoveControlBar (this, BCGP_DM_UNKNOWN);
	}

	if (!bDelay)
	{
		AdjustDockingLayout ();
	}
}
//------------------------------------------------------------------------------------
void CBCGPControlBar::AdjustSizeImmediate (BOOL bRecalcLayout)
{
	CBCGPReBar* pBar = DYNAMIC_DOWNCAST (CBCGPReBar, GetParent ());
	if (pBar != NULL)
	{
		return;
	}

	CSize sizeCurr = CalcFixedLayout (FALSE, IsHorizontal ());
	CRect rect;
	GetWindowRect (rect);

	if (rect.Size () != sizeCurr)
	{
		SetWindowPos (NULL, 0, 0, sizeCurr.cx, sizeCurr.cy, SWP_NOMOVE  | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	if (m_pParentDockBar != NULL)
	{
		UpdateVirtualRect ();
		if (bRecalcLayout)
		{
			m_pDockBarRow->ArrangeBars(this);
			BCGPGetParentFrame (this)->RecalcLayout ();
		}
	}
}

//*****************************************************************************
void CBCGPControlBar::OnRTLChanged (BOOL bIsRTL)
{
	globalData.m_bIsRTL = bIsRTL;

	if (GetParentDockBar () != NULL && IsHorizontal ())
	{
		SetWindowPos (NULL, m_rectVirtual.left, m_rectVirtual.top, 
							m_rectVirtual.Width (), m_rectVirtual.Height (),
							SWP_NOZORDER); 			
	}
}
//*****************************************************************************
void CBCGPControlBar::DisableControlBarMenuItem(CMenu& menu, UINT nItem)
{
	if (m_bHideDisabledControlBarMenuItems)
	{
		menu.DeleteMenu(nItem, MF_BYCOMMAND);
	}
	else
	{
		menu.EnableMenuItem(nItem, MF_GRAYED);
	}
}
//*****************************************************************************
void CBCGPControlBar::OnGetControlBarMenuItemText(UINT nResID, CString& strItem)
{
	strItem.Empty();

	CBCGPBaseTabbedBar* pTabbedBar = DYNAMIC_DOWNCAST(CBCGPBaseTabbedBar, this);
	if (pTabbedBar != NULL)
	{
		ASSERT_VALID(pTabbedBar);

		CBCGPBaseTabWnd* pTabWnd = pTabbedBar->GetUnderlinedWindow(); 
		if (pTabWnd != NULL)
		{
			ASSERT_VALID(pTabWnd);

			CBCGPControlBar* pActivePane = DYNAMIC_DOWNCAST(CBCGPControlBar, pTabWnd->GetActiveWnd());
			if (pActivePane != NULL)
			{
				ASSERT_VALID(pActivePane);

				pActivePane->OnGetControlBarMenuItemText(nResID, strItem);
				return;
			}
		}
	}

	strItem.LoadString(nResID);
}
//*****************************************************************************
BOOL CBCGPControlBar::OnShowControlBarMenu (CPoint point)
{
	if (g_pContextMenuManager == NULL)
	{
		return FALSE;
	}

	if ((GetEnabledAlignment () & CBRS_ALIGN_ANY) == 0 && !CanFloat ())
	{
		if (!CanAutoHide () || GetParentMiniFrame () != NULL)
		{
			return FALSE;
		}
	}

	const UINT idFloating	= (UINT) -102;
	const UINT idDocking	= (UINT) -103;
	const UINT idAutoHide	= (UINT) -104;
	const UINT idHide		= (UINT) -105;
	const UINT idTabbed		= (UINT) -106;

	CMenu menu;
	menu.CreatePopupMenu ();

	{
		CBCGPLocalResource locaRes;

		CString strItem;

		OnGetControlBarMenuItemText(IDS_BCGBARRES_FLOATING, strItem);
		menu.AppendMenu (MF_STRING, idFloating, strItem);

		OnGetControlBarMenuItemText(IDS_BCGBARRES_DOCKING, strItem);
		menu.AppendMenu (MF_STRING, idDocking,	strItem);

		OnGetControlBarMenuItemText(IDS_BCGBARRES_TABBED, strItem);
		menu.AppendMenu (MF_STRING, idTabbed, strItem);

		OnGetControlBarMenuItemText(IDS_BCGBARRES_AUTOHIDE, strItem);
		menu.AppendMenu (MF_STRING, idAutoHide, strItem);

		OnGetControlBarMenuItemText(IDS_BCGBARRES_HIDE, strItem);
		menu.AppendMenu (MF_STRING, idHide,	strItem);
	}

	if (!CanFloat ())
	{
		DisableControlBarMenuItem(menu, idFloating);
	}

	if (!CanAutoHide () || GetParentMiniFrame () != NULL)
	{
		DisableControlBarMenuItem(menu, idAutoHide);
	}

	if (IsAutoHideMode ())
	{
		DisableControlBarMenuItem(menu, idFloating);
		DisableControlBarMenuItem(menu, idDocking);
		menu.CheckMenuItem (idAutoHide, MF_CHECKED);
		DisableControlBarMenuItem(menu, idHide);
	}

	CBCGPMDIFrameWnd* pFrame = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, GetDockSite ());
	if (!CanBeTabbedDocument () || pFrame != NULL && pFrame->IsFullScreen ())
	{
		DisableControlBarMenuItem(menu, idTabbed);
	}

	if (IsMDITabbed ())
	{
		DisableControlBarMenuItem(menu, idFloating);
		DisableControlBarMenuItem(menu, idDocking);
		menu.CheckMenuItem (idTabbed, MF_CHECKED);
	}

	if (IsFloating ())
	{
		menu.CheckMenuItem (idFloating, MF_CHECKED);
	}
	else if (!IsAutoHideMode () && !IsMDITabbed ())
	{
		menu.CheckMenuItem (idDocking, MF_CHECKED);
	}

	if ((GetEnabledAlignment () & CBRS_ALIGN_ANY) == 0)
	{
		DisableControlBarMenuItem(menu, idDocking);
	}

	if (!CanBeClosed ())
	{
		DisableControlBarMenuItem(menu, idHide);
	}

	if (!OnBeforeShowControlBarMenu (menu))
	{
		return FALSE;
	}

	HWND hwndThis = GetSafeHwnd ();

	int nMenuResult = g_pContextMenuManager->TrackPopupMenu (
			menu, point.x, point.y, this);

	if (!::IsWindow (hwndThis))
	{
		return TRUE;
	}

	if (!OnAfterShowControlBarMenu (nMenuResult))
	{
		return TRUE;
	}

	switch (nMenuResult)
	{
	case idDocking:
		if (IsFloating ())
		{
			CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame ();
			if (pMiniFrame != NULL)
			{
				pMiniFrame->OnDockToRecentPos ();
			}
		}
		break;

	case idFloating:
		{
			BOOL bWasFloated = FALSE;

			CBCGPBaseTabbedBar* pTabbedBar = 
				 DYNAMIC_DOWNCAST (CBCGPBaseTabbedBar, 
					IsTabbed () ? GetParentTabbedBar () : this);

			if (pTabbedBar != NULL)
			{
				ASSERT_VALID (pTabbedBar);

				CBCGPBaseTabWnd* pTabWnd = pTabbedBar->GetUnderlinedWindow (); 
				if (pTabWnd != NULL)
				{
					ASSERT_VALID (pTabWnd);

					const int nTabID = pTabWnd->GetActiveTab ();
					CWnd* pWnd = pTabWnd->GetTabWnd (nTabID);

					if (pWnd != NULL && pTabWnd->IsTabDetachable (nTabID))
					{
						HWND hwndTab = pTabWnd->GetSafeHwnd();

						bWasFloated = pTabbedBar->DetachControlBar (pWnd, FALSE);
						if (bWasFloated)
						{
							if (::IsWindow(hwndTab) && pTabWnd->GetTabsNum () > 0 &&
								pTabWnd->GetVisibleTabsNum () == 0)
							{
								pTabbedBar->ShowControlBar (FALSE, FALSE, FALSE); 
							}

						}
					}
				}
			}
			
			if (!bWasFloated)
			{
				FloatControlBar (m_recentDockInfo.m_rectRecentFloatingRect);
			}
		}
		break;

	case idAutoHide:
		ToggleAutoHide ();
		break;

	case idHide:
		OnPressCloseButton ();
		break;

	case idTabbed:
		if (IsMDITabbed ())
		{
			CBCGPMDIChildWnd* pMDIChild = DYNAMIC_DOWNCAST(CBCGPMDIChildWnd, GetParent ());
			if (pMDIChild == NULL)
			{
				ASSERT (FALSE);
				return FALSE;
			}

			CBCGPMDIFrameWnd* pFrame = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, GetDockSite ());
			if (pFrame == NULL)
			{
				ASSERT (FALSE);
				return FALSE;
			} 

			pFrame->TabbedDocumentToControlBar (pMDIChild);
		}
		else
		{
			ConvertToTabbedDocument ();
		}
	}

	return TRUE;
}
//*****************************************************************************
void CBCGPControlBar::OnPressCloseButton ()
{
	CBCGPMiniFrameWnd* pMiniFrame = GetParentMiniFrame ();
	if (pMiniFrame != NULL)
	{
		if (pMiniFrame->OnCloseMiniFrame ())
		{
			pMiniFrame->CloseMiniFrame ();
		}
	}
}
//*****************************************************************************
void CBCGPControlBar::CopyState (CBCGPControlBar* pOrgBar)
{
	ASSERT_VALID (pOrgBar);

	CBCGPBaseControlBar::CopyState (pOrgBar);

	m_bFirstInGroup			= pOrgBar->m_bFirstInGroup;
	m_bLastInGroup			= pOrgBar->m_bLastInGroup;
	m_bActiveInGroup		= pOrgBar->m_bActiveInGroup;

	pOrgBar->GetMinSize (m_sizeMin);

	m_recentDockInfo		= pOrgBar->m_recentDockInfo;
	m_rectSavedDockedRect	= pOrgBar->m_rectSavedDockedRect;
	m_bRecentFloatingState	= pOrgBar->m_bRecentFloatingState;

}
//*******************************************************************************
void CBCGPControlBar::GetBarName (CString& strName) const
{
	if (GetSafeHwnd () == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	GetWindowText (strName);
}
//*****************************************************************************
BOOL CBCGPControlBar::CanBeTabbedDocument () const
{
	ASSERT_VALID (this);

	if (IsAutoHideMode ())
	{
		return FALSE;
	}

	CBCGPMDIFrameWnd* pMDIFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, GetDockSite ());
	if (pMDIFrame == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pMDIFrame);

	return pMDIFrame->CanCovertControlBarToMDIChild ();
}
//*******************************************************************************
void CBCGPControlBar::ConvertToTabbedDocument (BOOL /*bActiveTabOnly*/)
{
	ASSERT(FALSE);
	TRACE0("You need to derive a class from CBCGPDockingControlBar\n");
	return;
}