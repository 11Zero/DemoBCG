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
// BCGPOleCntrFrameWnd.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPPrintPreviewView.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPOleCntrFrameWnd.h"
#include "BCGPOleDocIPFrameWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPOleCntrFrameWnd

IMPLEMENT_DYNAMIC(CBCGPOleCntrFrameWnd, CFrameWnd)

CBCGPOleCntrFrameWnd::CBCGPOleCntrFrameWnd(COleIPFrameWnd* pInPlaceFrame) :
	COleCntrFrameWnd (pInPlaceFrame)
{
}

CBCGPOleCntrFrameWnd::~CBCGPOleCntrFrameWnd()
{
	POSITION pos = NULL;

	for (pos = m_dockManager.m_lstMiniFrames.GetHeadPosition (); pos != NULL;)
	{
		CBCGPMiniFrameWnd* pNextFrame = DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd,
					m_dockManager.m_lstMiniFrames.GetNext (pos));
		if (pNextFrame != NULL)
		{
			pNextFrame->DestroyWindow ();
		}
	}

	CList<HWND, HWND> lstChildren;
	CWnd* pNextWnd = GetTopWindow ();
	while (pNextWnd != NULL)
	{
		lstChildren.AddTail (pNextWnd->m_hWnd);
		pNextWnd = pNextWnd->GetNextWindow ();
	}

	for (pos = lstChildren.GetHeadPosition (); pos != NULL;)
	{
		HWND hwndNext = lstChildren.GetNext (pos);
		if (IsWindow (hwndNext) && ::GetParent (hwndNext) == m_hWnd)
		{
			::DestroyWindow (hwndNext);
		}
	}

	const CObList& list = CBCGPToolBar::GetAllToolbars ();
	CObList& gAllToolbars = const_cast<CObList&>(list);

	for (pos = gAllToolbars.GetHeadPosition (); pos != NULL;)
	{
		POSITION posSave = pos;

		CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (pos);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) == NULL)
		{
			gAllToolbars.RemoveAt (posSave);
		}
	}

}

BEGIN_MESSAGE_MAP(CBCGPOleCntrFrameWnd, COleCntrFrameWnd)
	//{{AFX_MSG_MAP(CBCGPOleCntrFrameWnd)
	ON_WM_SIZE()
	ON_WM_SIZING()
	//}}AFX_MSG_MAP
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPOleCntrFrameWnd message handlers

BOOL CBCGPOleCntrFrameWnd::DockControlBarLeftOf(CBCGPControlBar* pBar, CBCGPControlBar* pLeftOf)
{
	m_dockManager.DockControlBarLeftOf (pBar, pLeftOf);
	return TRUE;
}
//*************************************************************************************
void CBCGPOleCntrFrameWnd::OnSize(UINT nType, int cx, int cy) 
{
	COleCntrFrameWnd::OnSize(nType, cx, cy);
	
	if (nType != SIZE_MINIMIZED)
	{
		AdjustDockingLayout ();
	}
}
//*****************************************************************************
BOOL CBCGPOleCntrFrameWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
	m_dockManager.Create (this);
	
	return COleCntrFrameWnd::PreCreateWindow(cs);
}

//*****************************************************************************
//******************* dockmanager layer ***************************************
//*****************************************************************************

void CBCGPOleCntrFrameWnd::AddDockBar ()
{
	ASSERT_VALID (this);
}
//*****************************************************************************
BOOL CBCGPOleCntrFrameWnd::AddControlBar (CBCGPBaseControlBar* pControlBar, BOOL bTail)
{
	ASSERT_VALID (this);
	return m_dockManager.AddControlBar (pControlBar, bTail);
}
//*****************************************************************************
BOOL CBCGPOleCntrFrameWnd::InsertControlBar (CBCGPBaseControlBar* pControlBar, 
									  CBCGPBaseControlBar* pTarget, BOOL bAfter)
{
	ASSERT_VALID (this);
	return m_dockManager.InsertControlBar (pControlBar, pTarget, bAfter);
}
//*****************************************************************************
void CBCGPOleCntrFrameWnd::RemoveControlBarFromDockManager (CBCGPBaseControlBar* pControlBar, BOOL bDestroy,
										 BOOL bAdjustLayout, BOOL bAutoHide, CBCGPBaseControlBar* pBarReplacement)
{
	ASSERT_VALID (this);
	m_dockManager.RemoveControlBarFromDockManager (pControlBar, bDestroy, bAdjustLayout, bAutoHide, pBarReplacement);
	AdjustDockingLayout ();
}
//*****************************************************************************
void CBCGPOleCntrFrameWnd::DockControlBar (CBCGPBaseControlBar* pBar, UINT nDockBarID, 
									LPCRECT lpRect)
{
	ASSERT_VALID (this);
	m_dockManager.DockControlBar (pBar, nDockBarID, lpRect);
	AdjustDockingLayout ();
}
//*****************************************************************************
CBCGPBaseControlBar* CBCGPOleCntrFrameWnd::GetControlBar (UINT nID)
{
	ASSERT_VALID (this);
	
	return m_dockManager.FindBarByID (nID, TRUE);
}
//*****************************************************************************
void CBCGPOleCntrFrameWnd::ShowControlBar (CBCGPBaseControlBar* pBar, BOOL bShow, 
									   BOOL bDelay, BOOL bActivate)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	pBar->ShowControlBar (bShow, bDelay, bActivate);
}
//*****************************************************************************
CBCGPBaseControlBar* CBCGPOleCntrFrameWnd::ControlBarFromPoint (CPoint point, 
							int nSensitivity, bool bExactBar, 
							CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID (this);
	return m_dockManager.ControlBarFromPoint (point, nSensitivity, bExactBar, 
												pRTCBarType);
}
//*****************************************************************************
CBCGPBaseControlBar* CBCGPOleCntrFrameWnd::ControlBarFromPoint (CPoint point, 
								int nSensitivity, DWORD& dwAlignment, 
								CRuntimeClass* pRTCBarType) const
{
	ASSERT_VALID (this);
	return m_dockManager.ControlBarFromPoint (point, nSensitivity, dwAlignment, 
												pRTCBarType);
}
//*****************************************************************************
BOOL CBCGPOleCntrFrameWnd::IsPointNearDockBar (CPoint point, DWORD& dwBarAlignment, 
										   BOOL& bOuterEdge) const
{
	ASSERT_VALID (this);
	return m_dockManager.IsPointNearDockBar (point, dwBarAlignment, bOuterEdge);
}
//*****************************************************************************
void CBCGPOleCntrFrameWnd::AdjustDockingLayout (HDWP /*hdwp*/)
{
	ASSERT_VALID (this);
	AdjustClientArea();
}
//*****************************************************************************
BOOL CBCGPOleCntrFrameWnd::OnMoveMiniFrame	(CWnd* pFrame)
{
	ASSERT_VALID (this);
	return m_dockManager.OnMoveMiniFrame (pFrame);
}
//****************************************************************************************
BOOL CBCGPOleCntrFrameWnd::EnableDocking (DWORD dwDockStyle)
{
	return m_dockManager.EnableDocking (dwDockStyle);
}
//****************************************************************************************
BOOL CBCGPOleCntrFrameWnd::EnableAutoHideBars (DWORD dwDockStyle, BOOL bActivateOnMouseClick)
{
	return m_dockManager.EnableAutoHideBars (dwDockStyle, bActivateOnMouseClick);
}
//*************************************************************************************
void CBCGPOleCntrFrameWnd::RecalcLayout (BOOL bNotify)
{
	AdjustClientArea();
	m_dockManager.AdjustDockingLayout ();
	m_dockManager.RecalcLayout (bNotify);

	CView* pView = GetActiveView ();
	if (pView != NULL && pView->IsKindOf (RUNTIME_CLASS (CBCGPPrintPreviewView)) && 
		m_dockManager.IsPrintPreviewValid ())
	{
		CRect rectClient = m_dockManager.GetClientAreaBounds ();
		pView->SetWindowPos (NULL, rectClient.left, rectClient.top, 
								rectClient.Width (), rectClient.Height (),
								SWP_NOZORDER  | SWP_NOACTIVATE);
	}

	m_pInPlaceFrame->RecalcLayout (bNotify);
}
//*************************************************************************************
void CBCGPOleCntrFrameWnd::OnSizing(UINT fwSide, LPRECT pRect) 
{
	COleCntrFrameWnd::OnSizing(fwSide, pRect);

	CRect rect;
	GetWindowRect (rect);

	if (rect.Size () != CRect (pRect).Size ())
	{
		AdjustDockingLayout ();	
	}
}
//*************************************************************************************
void CBCGPOleCntrFrameWnd::OnIdleUpdateCmdUI()
{
	COleCntrFrameWnd::OnIdleUpdateCmdUI ();

	// update control bars
	m_dockManager.SendMessageToMiniFrames (WM_IDLEUPDATECMDUI);

	POSITION pos = m_dockManager.m_lstControlBars.GetHeadPosition();
	while (pos != NULL)
	{
		CBCGPBaseControlBar* pBar = (CBCGPBaseControlBar*)m_dockManager.m_lstControlBars.GetNext(pos);
		ASSERT(pBar != NULL);
		ASSERT_VALID(pBar);

		pBar->SendMessageToDescendants (WM_IDLEUPDATECMDUI, (WPARAM) TRUE);
	}
}
//*************************************************************************************
BOOL CBCGPOleCntrFrameWnd::OnShowControlBars (BOOL bShow)
{
	ASSERT_VALID (this);
	BOOL bResult = m_dockManager.ShowControlBars (bShow);
	AdjustDockingLayout ();
	
	return bResult;
}

void CBCGPOleCntrFrameWnd::AdjustClientArea()
{

	    COleServerDoc* pDoc = (COleServerDoc*)m_pInPlaceFrame->GetActiveDocument();

		if (pDoc != NULL ) 
		{
			ASSERT_VALID(pDoc);
			ASSERT_KINDOF(COleServerDoc, pDoc);
			CBCGPOleDocIPFrameWnd* pFrame = (CBCGPOleDocIPFrameWnd*)m_pInPlaceFrame;

			pDoc->OnResizeBorder(NULL, pFrame->m_lpFrame, TRUE);
		}

	
}