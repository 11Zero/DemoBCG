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
// BCGPDockBar.cpp : implementation file
//

#include "stdafx.h"

#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPOleIPFrameWnd.h"
#include "BCGPOleDocIPFrameWnd.h"
#include "BCGPMDIChildWnd.h"
#include "BCGPOleCntrFrameWnd.h"

#include "BCGPControlBar.h"
#include "BCGPDockBarRow.h"
#include "BCGPReBar.h"

#include "BCGPGlobalUtils.h"

#include "BCGPDockBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CBCGPDockBar, CBCGPBaseControlBar)

/////////////////////////////////////////////////////////////////////////////
// CBCGPDockBar

CBCGPDockBar::CBCGPDockBar() : m_nDockBarID (0)
{
}

CBCGPDockBar::~CBCGPDockBar()
{
	while (!m_lstDockBarRows.IsEmpty ())
	{
		delete m_lstDockBarRows.RemoveHead ();
	}
}


BEGIN_MESSAGE_MAP(CBCGPDockBar, CBCGPBaseControlBar)
	//{{AFX_MSG_MAP(CBCGPDockBar)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CONTEXTMENU()
	ON_WM_NCDESTROY()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPDockBar message handlers

BOOL CBCGPDockBar::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, DWORD dwBCGStyle, CCreateContext* pContext) 
{
	ASSERT_VALID (this);
	return CBCGPDockBar::CreateEx (0, dwStyle, rect, pParentWnd, dwBCGStyle, pContext);
}
//----------------------------------------------------------------------------------//
BOOL CBCGPDockBar::CreateEx(DWORD dwStyleEx, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, DWORD dwBCGStyle, CCreateContext* pContext) 
{
	ASSERT_VALID (this);

	DWORD dwEnableAlignment = GetEnabledAlignment ();
	EnableDocking (dwEnableAlignment | dwStyle);

	SetBarAlignment (dwStyle);

	dwStyle |= WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPED;
	dwStyleEx = WS_EX_LEFT;

	// Align the bar along borders; initially, create the dock bar with zero height/width
	CRect rectDockBar = rect;

	CRect rectParent;
	pParentWnd->GetClientRect (&rectParent);

	rectDockBar = rectParent;

	switch (GetCurrentAlignment ())
	{
	case CBRS_ALIGN_LEFT:
		rectDockBar.right = 0;
		m_nDockBarID = AFX_IDW_DOCKBAR_LEFT;
		break;

	case CBRS_ALIGN_RIGHT:
		rectDockBar.left = rectParent.right;
		m_nDockBarID = AFX_IDW_DOCKBAR_RIGHT;
		break;

	case CBRS_ALIGN_TOP:
		rectDockBar.bottom = rectParent.top;
		m_nDockBarID = AFX_IDW_DOCKBAR_TOP;
		break;

	case CBRS_ALIGN_BOTTOM:
		rectDockBar.top  = rectParent.bottom;
		m_nDockBarID = AFX_IDW_DOCKBAR_BOTTOM;
		break;
	}

	m_dwBCGStyle = dwBCGStyle;
	m_pDockSite = pParentWnd;

	return CWnd::CreateEx (dwStyleEx, 
		globalData.RegisterWindowClass (_T("BCGPDockBar")), NULL, dwStyle, rectDockBar, pParentWnd, m_nDockBarID, pContext);
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::AlignDockBar (const CRect& rectToAlignBy, CRect& rectResult, 
								  BOOL bMoveImmediately)
{
	ASSERT_VALID (this);
	if (rectResult.IsRectEmpty ())
	{
		GetWindowRect (rectResult);
	}

	CRect rectOld;
	GetWindowRect (rectOld);

	int nCurrWidth = rectResult.Width ();
	int nCurrHeight = rectResult.Height ();

	switch (GetCurrentAlignment ())
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

	if (rectResult != rectOld && bMoveImmediately)
	{
		CRect rectNew = rectResult;
		ASSERT_VALID (GetParent ());
		GetParent ()->ScreenToClient (rectNew);

		OnSetWindowPos (&wndBottom, rectNew, SWP_NOACTIVATE | SWP_NOZORDER);
	}
}
//----------------------------------------------------------------------------------//
// Moves control bar within row; floats the bar or moves it to an adjustent row 
// if the bar' virtual rectangle is being moved out of row beyond a limit
//----------------------------------------------------------------------------------//
BOOL CBCGPDockBar::MoveControlBar (CBCGPControlBar* pControlBar, UINT /*nFlags*/, 
									CPoint ptOffset)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pControlBar);

	CBCGPDockBarRow* pRow = pControlBar->GetDockBarRow ();
	ASSERT_VALID (pRow);

	CRect rectVirtual;
	pControlBar->GetVirtualRect (rectVirtual);

	// where the virtual rectangle will be if it's moved according to ptOffset
	rectVirtual.OffsetRect (ptOffset);
	
	CPoint ptMouse;
	GetCursorPos (&ptMouse);

	CRect rectRow;
	pRow->GetWindowRect (rectRow);

	CPoint ptDelta (0, 0);

	// check whether the control bar should change its state from docked to floated
	CBCGPBaseControlBar* pDockBar = NULL;
	
	if (pControlBar->IsChangeState (15, &pDockBar))
	{
		pControlBar->UpdateVirtualRect (ptOffset);
		pControlBar->GetVirtualRect (rectVirtual);
		pControlBar->FloatControlBar (rectVirtual, BCGP_DM_MOUSE);
		return TRUE; // indicates that the bar was floated and shouldn't be moved anymore within the dock bar
	}

	bool bOuterRow = false;
	CBCGPDockBarRow* pNextRow = RowFromPoint (rectVirtual.CenterPoint (), bOuterRow);
	
	int nBaseLineOffset = 0;
	int nOffsetLimit = 0;

	if (IsHorizontal ())
	{
		nBaseLineOffset = min (rectRow.bottom - rectVirtual.bottom, rectRow.top - rectVirtual.top);
		nOffsetLimit = rectVirtual.Height () * 2 / 3; // / 2;
	}
	else
	{
		nBaseLineOffset = min (rectRow.right - rectVirtual.right, rectRow.left - rectVirtual.left);
		nOffsetLimit = rectVirtual.Width () * 2 /3 ; // / 2;
	}

	if (abs (nBaseLineOffset) > nOffsetLimit)
	{
		if (pRow->GetBarCount () > 1  && nBaseLineOffset < pRow->GetRowHeight ())
		{
			// the bar should be put on the separate row, find a position to insert the row
			POSITION pos = m_lstDockBarRows.Find (pRow);
			ASSERT (pos != NULL);

			if (nBaseLineOffset < 0) // moving down - find the next visible row
			{
				// the new row should be inserted before next visible row
				FindNextVisibleRow (pos);
			}
			// otherwise the new row will be inserted before the current row 
			// (that's visible for sure) by AddRow (it inserts a row before spec. pos).

			pRow->RemoveControlBar (pControlBar);	
			CBCGPDockBarRow* pNewRow = 
					AddRow (pos, IsHorizontal () ? rectVirtual.Height () : rectVirtual.Width ());
			pNewRow->AddControlBarFromRow (pControlBar, BCGP_DM_MOUSE);	

			return FALSE;
		}
		else if (pRow != pNextRow && pNextRow != NULL)
		{
			ASSERT_VALID (pNextRow);
			//the bar is moved from the separate row to adjustent row (if exist)

			SetRedraw (FALSE);

			if (pRow->IsExclusiveRow ())
			{
				SwapRows (pNextRow, pRow);
			}
			else
			{
				if (pNextRow->IsExclusiveRow ())
				{
					SwapRows (pRow, pNextRow);
				}
				else
				{
					pRow->RemoveControlBar (pControlBar);	
					pNextRow->AddControlBarFromRow (pControlBar, BCGP_DM_MOUSE);
				}
			}
			
			pControlBar->m_bDisableMove = true;
			
			SetRedraw (TRUE);
			RedrawWindow (NULL, NULL, 
				RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
			return FALSE;
		}
		
	}
	// just move the bar within the row
	if (abs (nBaseLineOffset) < rectRow.Height ())
	{
		HDWP hdwp = BeginDeferWindowPos (pRow->GetBarCount ());
		pRow->MoveControlBar (pControlBar, ptOffset, TRUE, hdwp);
		EndDeferWindowPos (hdwp);
		return FALSE;
	}
	return FALSE;		
}
//----------------------------------------------------------------------------------//
CBCGPDockBarRow* CBCGPDockBar::FindNextVisibleRow (POSITION& pos, BOOL bForward)
{
	if (m_lstDockBarRows.IsEmpty ())
	{
		pos = NULL;
		return NULL;
	}

	if (pos == NULL)
	{
		pos = bForward  ? m_lstDockBarRows.GetHeadPosition () 
						: m_lstDockBarRows.GetTailPosition ();
	}
	else
	{
		// we need to skip to the next / prev row from the current position
		bForward ? m_lstDockBarRows.GetNext (pos) : m_lstDockBarRows.GetPrev (pos);
	}

	while (pos != NULL)
	{
		POSITION posSave = pos;	
		CBCGPDockBarRow* pRow = (CBCGPDockBarRow*) 
									(bForward ? m_lstDockBarRows.GetNext (pos) 
											  : m_lstDockBarRows.GetPrev (pos));
		ASSERT_VALID (pRow);

		if (pRow->IsVisible ())
		{
			pos = posSave;
			return pRow;
		}
	}

	return NULL;
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType) 
{
	ASSERT_VALID (this);
	
	CWnd::CalcWindowRect(lpClientRect, nAdjustType);
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::DockControlBar (CBCGPControlBar* pControlBar, BCGP_DOCK_METHOD dockMethod, 
								    LPCRECT lpRect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pControlBar);

	CRect rectDockArea; rectDockArea.SetRectEmpty ();
	if (lpRect != NULL)
	{
		rectDockArea = lpRect;
	}

	BOOL bVertDock = !IsHorizontal ();
	CSize szBarSize = pControlBar->CalcFixedLayout (FALSE, !bVertDock);

	if (!m_lstControlBars.Find (pControlBar))
	{
		CBCGPDockBarRow* pRowToDock = NULL;
		bool bOuterRow = false;

		if (dockMethod == BCGP_DM_MOUSE)
		{
			// calculate from which side the control bar is coming, using mouse cursor position.
			// the default bar width (for side bars) and height (for top/bottom bars)
			// is 30 for this example

			CPoint ptMouse;
			GetCursorPos (&ptMouse);

			CRect rectDockBar;
			GetWindowRect (&rectDockBar);

			// get pointer to the row on which the bar should be placed
			pRowToDock = RowFromPoint (ptMouse, bOuterRow);
		}
		else if (dockMethod == BCGP_DM_DBL_CLICK || dockMethod == BCGP_DM_RECT)
		{
			if (dockMethod == BCGP_DM_DBL_CLICK && 
				m_lstDockBarRows.Find (pControlBar->m_recentDockInfo.m_pRecentDockBarRow) != NULL)
			{
				pRowToDock = pControlBar->m_recentDockInfo.m_pRecentDockBarRow;
			}
			else
			{
				int nRowCount = (int) m_lstDockBarRows.GetCount ();

				if (CBCGPDockManager::m_bRestoringDockState)
				{
					if (pControlBar->m_recentDockInfo.m_nRecentRowIndex > nRowCount - 1)
					{
						for (int i = 0;  
							 i < pControlBar->m_recentDockInfo.m_nRecentRowIndex - nRowCount + 1; i++)
						{
							AddRow (NULL, bVertDock ? szBarSize.cx : szBarSize.cy);
						}
					}

					POSITION posRow = m_lstDockBarRows.FindIndex (pControlBar->m_recentDockInfo.m_nRecentRowIndex);
					pRowToDock = (CBCGPDockBarRow*) m_lstDockBarRows.GetAt (posRow);
				}
				else
				{
					if (pControlBar->m_recentDockInfo.m_nRecentRowIndex < nRowCount && 
						dockMethod == BCGP_DM_DBL_CLICK)
					{
						POSITION pos = m_lstDockBarRows.FindIndex (pControlBar->m_recentDockInfo.m_nRecentRowIndex);
						pRowToDock = (CBCGPDockBarRow*) m_lstDockBarRows.GetAt (pos);
						bOuterRow = true;
					}
					else if (dockMethod == BCGP_DM_DBL_CLICK && 
							 !pControlBar->m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect.IsRectEmpty ())
					{
						pRowToDock = FindRowByRect (pControlBar->m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect);
					}
					else if (dockMethod == BCGP_DM_RECT && lpRect != NULL)
					{
						pRowToDock = FindRowByRect (lpRect);
					}
				}

				if (pRowToDock == NULL)
				{
					AddRow (NULL, bVertDock ? szBarSize.cx : szBarSize.cy);
					pRowToDock = (CBCGPDockBarRow*) m_lstDockBarRows.GetTail ();
				}
			}

			ASSERT_VALID (pRowToDock);
			rectDockArea = &pControlBar->m_recentDockInfo.m_recentSliderInfo.m_rectDockedRect;
			ClientToScreen (rectDockArea);
		}

		// if the bar is being placed on the exclusive row 
		// (with menu bar, for example) we should create a new row, put the 
		// bar on new row and put this row after/before the exclusive row
		POSITION posSwapRow = NULL;
		if (pRowToDock != NULL && pRowToDock->IsExclusiveRow () ||
			pRowToDock != NULL && !pControlBar->DoesAllowSiblingBars () && 
			!pRowToDock->IsEmpty ())
		{
			posSwapRow = m_lstDockBarRows.Find (pRowToDock);
			ASSERT (posSwapRow != NULL);
			pRowToDock = NULL;
		}
		

		if (pRowToDock == NULL)
		{
			POSITION posNewBar = NULL; 

			if (posSwapRow != NULL)
			{
				// the bar is inserted before the specified position in AddRow
				posNewBar = posSwapRow;
				if (!bOuterRow) 
				{
					m_lstDockBarRows.GetNext (posNewBar);
				}	
			}
			else
			{
				posNewBar = bOuterRow ? m_lstDockBarRows.GetHeadPosition () : NULL;
			}

			pRowToDock = AddRow (posNewBar, bVertDock ? szBarSize.cx : szBarSize.cy);
		}

		ASSERT_VALID (pRowToDock);

		// the bar should be placed on the existing row or new row
		pRowToDock->AddControlBar (pControlBar, dockMethod, rectDockArea);
		// if the bar suudently changed its size we need to resize the row again
		CSize sizeBarNew = pControlBar->CalcFixedLayout (FALSE, !bVertDock);
		if (sizeBarNew != szBarSize)
		{
			ResizeRow (pRowToDock, bVertDock ? sizeBarNew.cx : sizeBarNew.cy);
		}

		m_lstControlBars.AddTail (pControlBar);
		AdjustDockingLayout ();
		ShowWindow (SW_SHOW);
	}
}
//----------------------------------------------------------------------------------//
CBCGPDockBarRow* CBCGPDockBar::FindRowByRect (CRect rectRow)
{
	bool b;
	CPoint pt = rectRow.TopLeft ();
	ClientToScreen (&pt);
	return RowFromPoint (pt, b);
}
//----------------------------------------------------------------------------------//
BOOL CBCGPDockBar::DockControlBarLeftOf (CBCGPControlBar* pBarToDock, 
										 CBCGPControlBar* pTargetBar)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBarToDock);
	ASSERT_VALID (pTargetBar);
	
	CBCGPDockBarRow* pTargetRow = RowFromControlBar (pTargetBar);

	if (pTargetRow == NULL)
	{
		return FALSE;
	}

	CRect rectTargetBar;
	pTargetBar->GetWindowRect (rectTargetBar);
	ScreenToClient (rectTargetBar);

	BOOL bVertDock = !IsHorizontal ();
	CSize szBarSize = pBarToDock->CalcFixedLayout (FALSE, !bVertDock);

	CRect rectFinal;

	if (IsHorizontal ())
	{
		rectFinal.SetRect (rectTargetBar.left - szBarSize.cx - 10, 
							rectTargetBar.top, rectTargetBar.left - 10, rectTargetBar.bottom);
	}
	else
	{
		rectFinal.SetRect (rectTargetBar.left,  
							rectTargetBar.top - szBarSize.cy - 10, 
							rectTargetBar.right, rectTargetBar.top - 10);
	}

	pBarToDock->PrepareToDock (this, BCGP_DM_RECT);
	ClientToScreen (rectFinal);
	pTargetRow->m_bIgnoreBarVisibility = TRUE;
	pTargetRow->AddControlBar (pBarToDock, BCGP_DM_RECT, &rectFinal);

	POSITION pos = m_lstControlBars.Find (pTargetBar);
	ASSERT (pos != NULL);

	m_lstControlBars.InsertBefore (pos, pBarToDock);

	AdjustDockingLayout ();
	FixupVirtualRects ();
	pTargetRow->m_bIgnoreBarVisibility = FALSE;
	

	return TRUE;
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::RemoveControlBar	(CBCGPControlBar* pControlBar, BCGP_DOCK_METHOD /*dockMethod*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pControlBar);

	if (!m_lstControlBars.IsEmpty ())
	{
		POSITION pos = m_lstControlBars.Find (pControlBar);
		if (pos != NULL)
		{
			m_lstControlBars.RemoveAt (pos);
			// we need to reposition bars according to the new situation
			// 1. expand bars that were stretched due to presence of this bar
			// 2. remove empty rows
			
			CBCGPDockBarRow* pRow = pControlBar->GetDockBarRow ();
			if (pRow != NULL)
			{
				pRow->RemoveControlBar (pControlBar);
			}
		}
	}
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::FixupVirtualRects ()
{
	ASSERT_VALID (this);

	for (POSITION pos = m_lstDockBarRows.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockBarRow* pNextRow = (CBCGPDockBarRow*) m_lstDockBarRows.GetNext (pos);
		ASSERT_VALID (pNextRow);

		pNextRow->FixupVirtualRects (false);
	}
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::RepositionBars (CRect& rectNewClientArea)
{
	ASSERT_VALID (this);

	CRect rectOldArea;
	GetClientRect (rectOldArea);
	CSize sizeNew = rectNewClientArea.Size ();
	CSize sizeOld = rectOldArea.Size (); 
	if (sizeNew != sizeOld)
	{
		int nHorzOffset = sizeNew.cx - sizeOld.cx;
		int nVertOffset = sizeNew.cy - sizeOld.cy;

		for (POSITION pos = m_lstDockBarRows.GetHeadPosition (); pos != NULL;)
		{
			CBCGPDockBarRow* pNextRow = (CBCGPDockBarRow*) m_lstDockBarRows.GetNext (pos);
			ASSERT_VALID (pNextRow);
			if (nHorzOffset != 0)
			{
				pNextRow->RepositionBars (rectNewClientArea, WMSZ_RIGHT, nHorzOffset > 0, abs (nHorzOffset));
			}

			if (nVertOffset != 0)
			{
				pNextRow->RepositionBars (rectNewClientArea, WMSZ_BOTTOM, nVertOffset > 0, abs (nVertOffset));
			}
		}
	}
	else
	{
		// sanity check
		for (POSITION pos = m_lstDockBarRows.GetHeadPosition (); pos != NULL;)
		{
			CBCGPDockBarRow* pNextRow = (CBCGPDockBarRow*) m_lstDockBarRows.GetNext (pos);
			ASSERT_VALID (pNextRow);
			
			pNextRow->ExpandStretchedBarsRect ();
		}
	}
}
//----------------------------------------------------------------------------------//
CBCGPDockBarRow* CBCGPDockBar::CreateRow (CBCGPDockBar* /*pParentDocBar*/, int nOffset, int nRowHeight)
{
	ASSERT_VALID (this);
	CBCGPDockBarRow* pRow = new CBCGPDockBarRow (this, nOffset, nRowHeight);
	if (!pRow->Create ())
	{
		delete pRow;
		return NULL;
	}
	return pRow;
}
//----------------------------------------------------------------------------------//
CBCGPDockBarRow* CBCGPDockBar::AddRow (POSITION posRowBefore, int nRowHeight)
{
	ASSERT_VALID (this);
	// claculate the row offset 
	int nOffset = 0;

	for (POSITION pos = m_lstDockBarRows.GetHeadPosition (); pos != posRowBefore;)
	{
		CBCGPDockBarRow* pNextRow = (CBCGPDockBarRow*) m_lstDockBarRows.GetNext (pos);
		ASSERT_VALID (pNextRow);
		if (pNextRow->IsVisible ())
		{
			nOffset += pNextRow->GetRowHeight ();
		}
	}

	ResizeDockBarByOffset (nRowHeight);

	CBCGPDockBarRow* pNewRow = CreateRow (this, nOffset, nRowHeight);

	if (pNewRow == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	if (posRowBefore != NULL)
	{
		POSITION pos = m_lstDockBarRows.InsertBefore (posRowBefore, pNewRow);
		OnInsertRow (pos);
	}
	else
	{
		m_lstDockBarRows.AddTail (pNewRow);		
	}

	return pNewRow;
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::RemoveRow (CBCGPDockBarRow* pRow)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pRow);
	ASSERT (!m_lstDockBarRows.IsEmpty ());

	int nRowHeight = pRow->GetRowHeight ();
	if (pRow->IsVisible ())
	{
		ResizeDockBarByOffset (-nRowHeight);
	}

	POSITION pos = m_lstDockBarRows.Find (pRow);
	if (pos != NULL)
	{
		OnRemoveRow (pos);
		m_lstDockBarRows.RemoveAt (pos);
		delete pRow;
	}
}
//----------------------------------------------------------------------------------//
int CBCGPDockBar::ResizeRow  (CBCGPDockBarRow* pRow, int nNewSize, BOOL bAdjustLayout)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pRow);
	
	int nOffset = nNewSize - pRow->GetRowHeight ();
	if (nOffset < 0 && !pRow->IsEmpty ())
	{
		CSize size = pRow->CalcFixedLayout (TRUE, IsHorizontal ());
		if (IsHorizontal () && nNewSize - size.cy < 0 ||
			!IsHorizontal () && nNewSize - size.cx < 0)
		{
			return 0;
		}
	}
	int nActualOffset = OnResizeRow (pRow, nOffset);
	ResizeDockBarByOffset (nActualOffset, bAdjustLayout);
	
	return nActualOffset; 
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::ShowRow (CBCGPDockBarRow* pRow, BOOL bShow, BOOL bAdjustLayout)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pRow);
	ASSERT (!m_lstDockBarRows.IsEmpty ());

	POSITION pos = m_lstDockBarRows.Find (pRow);
	OnShowRow (pos, bShow);

	int nRowHeight = pRow->GetRowHeight ();
	ResizeDockBarByOffset (bShow ? nRowHeight : -nRowHeight, bAdjustLayout);
	
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::OnInsertRow (POSITION pos)	
{
	ASSERT_VALID (this);
	ASSERT (pos != NULL);

	CBCGPDockBarRow* pNewRow = (CBCGPDockBarRow*) m_lstDockBarRows.GetNext (pos);
	ASSERT_VALID (pNewRow);

	int nRowSize = pNewRow->GetRowHeight ();

	// when the row is inserted, all control bars that belongs to the rows after new,
	// should be moved down 
	while (pos != NULL)
	{
		CBCGPDockBarRow* pNextRow = (CBCGPDockBarRow*) m_lstDockBarRows.GetNext (pos);
		ASSERT_VALID (pNextRow);
		pNextRow->Move (nRowSize);
	}
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::OnRemoveRow (POSITION pos, BOOL bByShow)	
{
	ASSERT_VALID (this);
	ASSERT (pos != NULL);

	CBCGPDockBarRow* pRowToRemove = (CBCGPDockBarRow*) m_lstDockBarRows.GetNext (pos);
	ASSERT_VALID (pRowToRemove);

	if (!pRowToRemove->IsVisible () && !bByShow)
	{
		return;
	}

	int nRowSize = pRowToRemove->GetRowHeight ();
	
	while (pos != NULL)
	{
		CBCGPDockBarRow* pNextRow = (CBCGPDockBarRow*) m_lstDockBarRows.GetNext (pos);
		ASSERT_VALID (pNextRow);
		pNextRow->Move (-nRowSize);
	}
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::OnShowRow (POSITION pos, BOOL bShow)
{
	ASSERT_VALID (this);
	ASSERT (pos != NULL);

	bShow ? OnInsertRow (pos) : OnRemoveRow (pos, TRUE);
}
//----------------------------------------------------------------------------------//
int CBCGPDockBar::OnResizeRow (CBCGPDockBarRow* pRowToResize, int nOffset)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pRowToResize);

	int nActualOffset = pRowToResize->Resize (nOffset);
	if (!pRowToResize->IsVisible ())
	{
		return 0;
	}

	POSITION pos = m_lstDockBarRows.Find (pRowToResize);
	m_lstDockBarRows.GetNext (pos);
	// skip to next row
	while (pos != NULL)
	{
		CBCGPDockBarRow* pNextRow = (CBCGPDockBarRow*) m_lstDockBarRows.GetNext (pos);
		ASSERT_VALID (pNextRow);
		pNextRow->Move (nActualOffset);
	}

	return nActualOffset;
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::SwapRows (CBCGPDockBarRow* pFirstRow, CBCGPDockBarRow* pSecondRow)
{
	POSITION posFirstRow = m_lstDockBarRows.Find (pFirstRow);
	POSITION posSecondRow = m_lstDockBarRows.Find (pSecondRow);

	ASSERT (posFirstRow != NULL);
	ASSERT (posSecondRow != NULL);

	POSITION posTmp = posFirstRow;

	FindNextVisibleRow (posTmp);

	bool bSwapDown = (posTmp == posSecondRow);
	
	if (!bSwapDown)
	{
		posTmp = posFirstRow; 
		FindNextVisibleRow (posTmp, FALSE);
		if (posTmp != posSecondRow)
		{
			return;
		}
	}

	m_lstDockBarRows.InsertAfter (posFirstRow, pSecondRow);
	m_lstDockBarRows.InsertAfter (posSecondRow, pFirstRow);
	m_lstDockBarRows.RemoveAt (posFirstRow);
	m_lstDockBarRows.RemoveAt (posSecondRow);

	int nRowHeight = pFirstRow->GetRowHeight ();
	pSecondRow->Move (bSwapDown ? -nRowHeight : nRowHeight); 
	nRowHeight = pSecondRow->GetRowHeight ();
	pFirstRow->Move (bSwapDown ? nRowHeight : -nRowHeight); 
	FixupVirtualRects ();
}
//----------------------------------------------------------------------------------//
CBCGPDockBarRow* CBCGPDockBar::RowFromPoint (CPoint pt, bool& bOuterRow) const
{
	ASSERT_VALID (this);

	bOuterRow = false;
	CRect rectRow;
	for (POSITION pos = m_lstDockBarRows.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockBarRow* pRow = (CBCGPDockBarRow*) m_lstDockBarRows.GetNext (pos);
		ASSERT_VALID (pRow);

		if (!pRow->IsVisible ())
		{
			continue;
		}

		pRow->GetWindowRect (rectRow);
		if (rectRow.PtInRect (pt))
		{
			return pRow;
		}
	}

	CRect rectWnd;
	GetWindowRect (&rectWnd);

	if (IsHorizontal () && pt.y < rectWnd.top ||
		!IsHorizontal () && pt.x < rectWnd.left)
	{
		bOuterRow = true;
	}
	
	return NULL;
}
//----------------------------------------------------------------------------------//
CBCGPDockBarRow* CBCGPDockBar::RowFromControlBar (CBCGPBaseControlBar* pBar) const
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	for (POSITION pos = m_lstDockBarRows.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockBarRow* pRow = (CBCGPDockBarRow*) m_lstDockBarRows.GetNext (pos);
		ASSERT_VALID (pRow);

		if (pRow->HasControlBar (pBar) != NULL)
		{
			return pRow;
		}
	}

	return NULL;
}
//----------------------------------------------------------------------------------//
BOOL CBCGPDockBar::ShowControlBar (CBCGPBaseControlBar* pBar, BOOL bShow, BOOL bDelay, BOOL /*bActivate*/)
{
	CBCGPDockBarRow* pRow = RowFromControlBar (pBar);

	if (pRow != NULL)
	{
		CBCGPControlBar* pBarToShow = DYNAMIC_DOWNCAST (CBCGPControlBar, pBar);
		// allows to show/hide only CBCGPControlBar-derived bars (other bars
		// has no docking abilitty)
		if (pBarToShow != NULL)
		{
			return pRow->ShowControlBar (pBarToShow, bShow, bDelay);
		}
	}
	return FALSE;
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::ResizeDockBarByOffset (int nOffset, BOOL bAdjustLayout)
{
	ASSERT_VALID (this);

	CRect rect;
	GetWindowRect (&rect);
	GetParent ()->ScreenToClient (&rect);

	switch (GetCurrentAlignment ())
	{
	case CBRS_ALIGN_LEFT:
		rect.right += nOffset;
		break;

	case CBRS_ALIGN_RIGHT:
		rect.left -= nOffset;
		break;

	case CBRS_ALIGN_TOP:
		rect.bottom += nOffset;
		break;

	case CBRS_ALIGN_BOTTOM:
		rect.top  -= nOffset;
		break;
	}

	MoveWindow (rect);
	if (bAdjustLayout)
	{
		AdjustDockingLayout ();
	}
}
//----------------------------------------------------------------------------------//
bool CBCGPDockBar::IsLastRow (CBCGPDockBarRow* pRow) const
{
	ASSERT_VALID (this);
	return (!m_lstDockBarRows.IsEmpty () && 
			(pRow == m_lstDockBarRows.GetHead () ||
			 pRow == m_lstDockBarRows.GetTail ()));
}
//----------------------------------------------------------------------------------//
CSize CBCGPDockBar::CalcFixedLayout (BOOL bStretch, BOOL bHorz)
{
	ASSERT_VALID (this);

	int nTotalHeightRequired = 0;

	BOOL bHorzBar = IsHorizontal ();

	for (POSITION pos = m_lstDockBarRows.GetHeadPosition (); pos != NULL;)
	{
		CBCGPDockBarRow* pNextRow = (CBCGPDockBarRow*) m_lstDockBarRows.GetNext (pos);
		ASSERT_VALID (pNextRow);

		if (!pNextRow->IsVisible ())
		{
			continue;
		}

		int nCurrHeight = pNextRow->GetRowHeight ();
		CSize sizeRowRequired = pNextRow->CalcFixedLayout (bStretch, bHorz); 

		int nHeightRequired =  bHorzBar ? sizeRowRequired.cy : sizeRowRequired.cx;

		if (nHeightRequired != nCurrHeight && nHeightRequired > 0)
		{
			ResizeRow (pNextRow, nHeightRequired, FALSE);
		}

		nTotalHeightRequired += nHeightRequired;
	}

	CRect rectWnd;
	GetWindowRect (rectWnd);

	return rectWnd.Size ();
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::ResizeDockBar (int nNewWidth, int nNewHeight) // not called from anywhere !!!
{
	ASSERT_VALID (this);
	CWnd* pParentWnd = GetParent ();
	ASSERT_VALID (pParentWnd);

	CRect rectDockBar;
	GetClientRect (&rectDockBar);
	MapWindowPoints (pParentWnd, &rectDockBar);

	switch (GetCurrentAlignment ())
	{
	case CBRS_ALIGN_LEFT:
		if (nNewHeight != -1)
		{
			rectDockBar.bottom = rectDockBar.top + nNewHeight;
		}
		break;

	case CBRS_ALIGN_RIGHT:
		if (nNewHeight != -1)
		{
			rectDockBar.bottom = rectDockBar.top + nNewHeight;
		}
		break;

	case CBRS_ALIGN_TOP:
		if (nNewWidth != -1)
		{
			rectDockBar.right = rectDockBar.left + nNewWidth;
		}
		break;

	case CBRS_ALIGN_BOTTOM:
		if (nNewWidth != -1)
		{
			rectDockBar.right = rectDockBar.left + nNewWidth;
		}
		break;
	}

	OnSetWindowPos (&wndBottom, rectDockBar, SWP_NOACTIVATE | SWP_NOZORDER);
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
}
//----------------------------------------------------------------------------------//
BOOL CBCGPDockBar::IsRectWithinDockBar (CRect rect, CPoint& ptDelta)
{
	ASSERT_VALID (this);
	CRect rectWnd;
	GetWindowRect (&rectWnd);

	ptDelta.x = ptDelta.y = 0;

	if (IsHorizontal ())
	{
		if (rect.left < rectWnd.left)
		{
			ptDelta.x = rectWnd.left - rect.left;
			return FALSE;
		}
		if (rect.right >  rectWnd.right)
		{
			ptDelta.x = rectWnd.right - rect.right;
			return FALSE;
		}
	}
	else
	{
		if (rect.top < rectWnd.top)
		{
			ptDelta.y = rectWnd.top - rect.top;
			return FALSE;
		}
		if (rect.bottom > rectWnd.bottom)
		{
			ptDelta.y = rect.bottom - rectWnd.bottom;
			return FALSE;
		}
	}

	return TRUE;
}
//----------------------------------------------------------------------------------//
BOOL CBCGPDockBar::CanAcceptBar (const CBCGPBaseControlBar* pBar) const
{
	ASSERT_VALID (this);
	if (pBar == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	return !IsResizable ();
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::OnSize(UINT nType, int cx, int cy) 
{
	ASSERT_VALID (this);

	CWnd::OnSize(nType, cx, cy);
}
//----------------------------------------------------------------------------------//
CBCGPControlBar* CBCGPDockBar::ControlBarFromPoint (CPoint pt)
{
	ASSERT_VALID (this);

	CRect rectBar;
	CBCGPControlBar* pBar = NULL;
	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		pBar = (CBCGPControlBar*) m_lstControlBars.GetNext (pos);
		ASSERT_VALID (pBar);

		pBar->GetWindowRect (rectBar);
		if (rectBar.PtInRect (pt))
		{
			return pBar;
		}
	}

	return NULL;
}
//----------------------------------------------------------------------------------//
int CBCGPDockBar::RectSideFromPoint (const CRect& rect, const CPoint& point)
{
	int nDeltaLeft = point.x - rect.left;
	int nDeltaTop = point.y - rect.top;
	int nDeltaRight = rect.right - point.x;
	int nDeltaBottom = rect.bottom - point.y;

	// use hit test definition to describe the side
	UINT nHitTestLR = (nDeltaLeft <= nDeltaRight) ? HTLEFT : HTRIGHT;
	UINT nHitTetsTB = (nDeltaTop <= nDeltaBottom) ? HTTOP : HTBOTTOM;
	
	int nHitTest = HTERROR;
	if (nHitTestLR == HTLEFT && nHitTetsTB == HTTOP)
	{
		nHitTest = (nDeltaLeft <= nDeltaTop) ? HTLEFT : HTTOP;
	}
	else if (nHitTestLR == HTRIGHT && nHitTetsTB == HTTOP)
	{
		nHitTest = (nDeltaRight <= nDeltaTop) ? HTRIGHT : HTTOP;
	}
	else if (nHitTestLR == HTLEFT && nHitTetsTB == HTBOTTOM)
	{
		nHitTest = (nDeltaLeft <= nDeltaBottom) ? HTLEFT : HTBOTTOM;
	}
	else if (nHitTestLR == HTRIGHT && nHitTetsTB == HTBOTTOM)
	{
		nHitTest = (nDeltaRight <= nDeltaBottom) ? HTRIGHT : HTBOTTOM;
	}
	else 
	{
		return HTERROR;
	}
	return nHitTest;
}
//----------------------------------------------------------------------------------//
BOOL CBCGPDockBar::ReplaceControlBar (CBCGPControlBar* pOldBar, CBCGPControlBar* pNewBar)
{
	ASSERT_VALID (this);
	POSITION pos = m_lstControlBars.Find (pOldBar);
	
	if (pos != NULL)
	{
		m_lstControlBars.InsertAfter (pos, pNewBar);
		m_lstControlBars.RemoveAt (pos);
		return TRUE;
	}

	return FALSE;
}
//----------------------------------------------------------------------------------//
BOOL CBCGPDockBar::OnSetWindowPos (const CWnd* pWndInsertAfter, 
								   const CRect& rectWnd, UINT nFlags)
{
	ASSERT_VALID (this);
	return (BOOL)(SetWindowPos (pWndInsertAfter, 
						 rectWnd.left, rectWnd.top, rectWnd.Width (), rectWnd.Height (),
						 nFlags | SWP_NOACTIVATE) != 0);
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::OnNcDestroy() 
{
	CWnd::OnNcDestroy();
	delete this;
}
//----------------------------------------------------------------------------------//
BOOL CBCGPDockBar::OnEraseBkgnd(CDC* pDC) 
{
	CRect rect;
	GetClientRect (rect);

	CBCGPVisualManager::GetInstance ()->OnFillBarBackground (pDC, this, rect, rect, FALSE);
	return TRUE;
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::AdjustLayout ()
{
	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPBaseControlBar* pBar = (CBCGPBaseControlBar*) m_lstControlBars.GetNext (pos);
		ASSERT_VALID (pBar);
		pBar->AdjustLayout ();
	}
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::AdjustDockingLayout ()
{
	ASSERT_VALID (this);

	CWnd* pParent = GetParent ();
	ASSERT_VALID (pParent);

	if (pParent->IsKindOf (RUNTIME_CLASS (CBCGPFrameWnd)))
	{
		((CBCGPFrameWnd*) pParent)->AdjustDockingLayout ();
	}
	else if (pParent->IsKindOf (RUNTIME_CLASS (CBCGPMDIFrameWnd)))
	{
		((CBCGPMDIFrameWnd*) pParent)->AdjustDockingLayout (NULL);
	}
	else if (pParent->IsKindOf (RUNTIME_CLASS (CBCGPOleIPFrameWnd)))
	{
		((CBCGPOleIPFrameWnd*) pParent)->AdjustDockingLayout ();
	}
	else if (pParent->IsKindOf (RUNTIME_CLASS (CBCGPOleDocIPFrameWnd)))
	{
		((CBCGPOleDocIPFrameWnd*) pParent)->AdjustDockingLayout ();
	}
	else if (pParent->IsKindOf (RUNTIME_CLASS (CBCGPOleCntrFrameWnd)))
	{
		((CBCGPOleCntrFrameWnd*) pParent)->AdjustDockingLayout ();
	}
	else if (pParent->IsKindOf (RUNTIME_CLASS (CBCGPMDIChildWnd)))
	{
		((CBCGPMDIChildWnd*) pParent)->AdjustDockingLayout ();
	}
	else if (pParent->IsKindOf (RUNTIME_CLASS (CDialog)))
	{
		if (pParent->GetSafeHwnd() == AfxGetMainWnd()->GetSafeHwnd())
		{
			globalUtils.m_bDialogApp = TRUE;
		}
	}
}
//----------------------------------------------------------------------------------//
int CBCGPDockBar::FindRowIndex (CBCGPDockBarRow* pRow)
{
	ASSERT_VALID (this);

	if (pRow == NULL)
	{
		return 0;
	}

	int nIndex = 0;
	for (POSITION pos = m_lstDockBarRows.GetHeadPosition (); pos != NULL; nIndex++)
	{
		CBCGPDockBarRow* pNextRow = (CBCGPDockBarRow*) m_lstDockBarRows.GetNext (pos);
		if (pNextRow == pRow)
		{
			return nIndex;
		}
	}

	return 0;
}
//----------------------------------------------------------------------------------//
void CBCGPDockBar::OnContextMenu(CWnd* /*pWnd*/, CPoint point) 
{
	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return;
	}

	if (!CBCGPToolBar::IsCustomizeMode () && !IsDragMode ())
	{
		CFrameWnd* pParentFrame = BCGCBProGetTopLevelFrame (this);
		if (pParentFrame == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		OnControlBarContextMenu (pParentFrame, point);
	}
}
//----------------------------------------------------------------------------------//
BOOL CBCGPDockBar::IsDragMode () const
{
	ASSERT_VALID (this);
	
	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPControlBar* pBar = DYNAMIC_DOWNCAST 
			(CBCGPControlBar, m_lstControlBars.GetNext (pos));
		if (pBar == NULL)
		{
			continue;
		}

		if (pBar->IsDragMode ())
		{
			return TRUE;
		}
	}

	return FALSE;
}
//----------------------------------------------------------------------------------//
CBCGPControlBar* CBCGPDockBar::FindBarByID (UINT nID)
{
	ASSERT_VALID (this);

	for (POSITION pos = m_lstControlBars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPControlBar* pBar = (CBCGPControlBar*) m_lstControlBars.GetNext (pos);
		ASSERT_VALID (pBar);

		if (pBar->GetDlgCtrlID () == (int) nID)
		{
			return pBar;
		}

		//-----------------
		// Check for rebar:
		//-----------------
		CBCGPReBar* pRebar = DYNAMIC_DOWNCAST (CBCGPReBar, pBar);
		if (pRebar != NULL)
		{
			ASSERT_VALID(pRebar);

			CBCGPControlBar* pBarPane = DYNAMIC_DOWNCAST(CBCGPControlBar,
					pRebar->GetDlgItem (nID));
			if (pBarPane != NULL)
			{
				return pBarPane;
			}
		}
	}

	return NULL;
}

void CBCGPDockBar::OnDestroy() 
{
	RemoveControlBarFromDockManager (this, FALSE);
	CBCGPBaseControlBar::OnDestroy();
}
