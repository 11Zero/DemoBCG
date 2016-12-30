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
// BCGPGanttView.cpp: implementation of the CBCGPGanttView class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPGanttView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

/////////////////////////////////////////////////////////////////////////////
// CBCGPGanttView

IMPLEMENT_DYNCREATE(CBCGPGanttView, CView)

CBCGPGanttView::CBCGPGanttView()
{
}

CBCGPGanttView::~CBCGPGanttView()
{
}


BEGIN_MESSAGE_MAP(CBCGPGanttView, CView)
	//{{AFX_MSG_MAP(CBCGPGanttView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(BCGM_GANTT_CONTROL_CREATE_CHART, OnNotifyCreateChart)
	ON_REGISTERED_MESSAGE(BCGM_GANTT_CONTROL_CREATE_GRID, OnNotifyCreateGrid)
	ON_REGISTERED_MESSAGE(BCGM_GANTT_CHART_CLICKCHART, OnNotifyChartClick)
	ON_REGISTERED_MESSAGE(BCGM_GANTT_CHART_CLICKHEADER, OnNotifyChartClickHeader)
	ON_REGISTERED_MESSAGE(BCGM_GANTT_CHART_CLICKITEM, OnNotifyChartClickItem)
    ON_REGISTERED_MESSAGE(BCGM_GANTT_CHART_DBLCLICKCHART, OnNotifyChartDblClick)
    ON_REGISTERED_MESSAGE(BCGM_GANTT_CHART_DBLCLICKHEADER, OnNotifyChartDblClickHeader)
    ON_REGISTERED_MESSAGE(BCGM_GANTT_CHART_DBLCLICKITEM, OnNotifyChartDblClickItem)
	ON_REGISTERED_MESSAGE(BCGM_GANTT_CHART_ITEM_MOVING, OnNotifyChartItemMoving)
	ON_REGISTERED_MESSAGE(BCGM_GANTT_CHART_SCALE_CHANGING, OnNotifyChartScaleChanging)
	ON_REGISTERED_MESSAGE(BCGM_GANTT_CHART_SCALE_CHANGED, OnNotifyChartScaleChanged)	
    ON_REGISTERED_MESSAGE(BCGM_GANTT_CONTROL_READ_ITEM_DATA_FROM_GRID, OnNotifyReadDataFromGrid)
    ON_REGISTERED_MESSAGE(BCGM_GANTT_CONTROL_WRITE_ITEM_DATA_TO_GRID, OnNotifyWriteDataToGrid)
	ON_REGISTERED_MESSAGE(BCGM_GANTT_STORAGE_CHANGED, OnNotifyStorageChanged)
    ON_REGISTERED_MESSAGE(BCGM_GANTT_CONNECTION_ADDED, OnNotifyStorageConnectionAdded)
    ON_REGISTERED_MESSAGE(BCGM_GANTT_CONNECTION_REMOVED, OnNotifyStorageConnectionRemoved)
	ON_REGISTERED_MESSAGE(BCGM_CHANGEVISUALMANAGER, OnChangeVisualManager)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPGanttView drawing

void CBCGPGanttView::OnDraw(CDC* /*pDC*/)
{
}

void CBCGPGanttView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_wndGanttControl.CreateControls ();
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGanttView diagnostics

#ifdef _DEBUG
void CBCGPGanttView::AssertValid() const
{
	CView::AssertValid();
}

void CBCGPGanttView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBCGPGanttView message handlers

int CBCGPGanttView::OnCreate (LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}	

    if (!m_wndGanttControl.Create (WS_VISIBLE | WS_CHILD, CRect(0, 0, 1, 1), this, 1))
    {
        TRACE0("CBCGPGanttControl::OnCreate: cannot create gantt control\n");
        return -1;
    }

    return 0;
}
//*******************************************************************************
BOOL CBCGPGanttView::OnEraseBkgnd (CDC* /*pDC*/) 
{
	return TRUE;
}
//*******************************************************************************
void CBCGPGanttView::OnSize (UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	if (m_wndGanttControl.GetSafeHwnd () != NULL)
	{
		m_wndGanttControl.SetWindowPos (this, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | 
			SWP_NOOWNERZORDER | SWP_NOACTIVATE);
	}	
}
//*******************************************************************************
void CBCGPGanttView::OnSetFocus (CWnd* pOldWnd) 
{
    CView::OnSetFocus(pOldWnd);

	if (m_wndGanttControl.GetSafeHwnd () != NULL)
	{
		m_wndGanttControl.SetFocus();
	}	
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyChartClick (WPARAM wParam, LPARAM lParam)
{
	CPoint point;
	POINTSTOPOINT (point, lParam);

	return (LRESULT)OnChartClick ((UINT)wParam, point);
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyChartClickItem (WPARAM wParam, LPARAM lParam)
{
	return (LRESULT)OnChartClickItem ((UINT)wParam, (CBCGPGanttItem*)lParam);
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyChartClickHeader (WPARAM wParam, LPARAM lParam)
{
	CPoint point;
	POINTSTOPOINT (point, lParam);

	return (LRESULT)OnChartClickHeader ((UINT)wParam, point);
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyChartDblClick (WPARAM wParam, LPARAM lParam)
{
    CPoint point;
    POINTSTOPOINT (point, lParam);

    return (LRESULT)OnChartDoubleClick ((UINT)wParam, point);
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyChartDblClickItem (WPARAM wParam, LPARAM lParam)
{
    return (LRESULT)OnChartDoubleClickItem ((UINT)wParam, (CBCGPGanttItem*)lParam);
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyChartDblClickHeader (WPARAM wParam, LPARAM lParam)
{
    CPoint point;
    POINTSTOPOINT (point, lParam);

    return (LRESULT)OnChartDoubleClickHeader ((UINT)wParam, point);
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyChartItemMoving (WPARAM, LPARAM lParam)
{
	return OnChartItemMoving ((BCGP_GANTT_ITEM_DRAGDROP*)lParam) ? 0L : 1L;
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyChartItemsChanged (WPARAM wParam, LPARAM lParam)
{
    OnChartItemChanged ((CBCGPGanttItem*)lParam, (DWORD)wParam);
	return 0L;
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyChartScaleChanging (WPARAM wParam, LPARAM)
{
	return OnChartScaleChanging (wParam == 1) ? 1L : 0L;
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyChartScaleChanged (WPARAM, LPARAM)
{
	OnChartScaleChanged ();
	return 0L;
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyStorageChanged (WPARAM, LPARAM lParam)
{
    BCGP_GANTT_STORAGE_UPDATE_INFO* pUpdate = (BCGP_GANTT_STORAGE_UPDATE_INFO*)lParam;
    if (pUpdate != NULL)
    {
        OnStorageChanged (*pUpdate);
    }
    return 0L;
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyStorageConnectionAdded (WPARAM, LPARAM lParam)
{
    CBCGPGanttConnection* pConnection = (CBCGPGanttConnection*)lParam;
    if (pConnection != NULL)
    {
        OnStorageConnectionAdded (*pConnection);
    }
    return 0L;
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyStorageConnectionRemoved (WPARAM, LPARAM lParam)
{
    CBCGPGanttConnection* pConnection = (CBCGPGanttConnection*)lParam;
    if (pConnection != NULL)
    {
        OnStorageConnectionRemoved (*pConnection);
    }
    return 0L;
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyReadDataFromGrid (WPARAM, LPARAM lParam)
{
    BCGP_GANTT_CONTROL_ROW* pRowInfo = (BCGP_GANTT_CONTROL_ROW*)lParam;
    if (pRowInfo != NULL)
    {
        return (LRESULT)OnGridRowReadData (*pRowInfo);
    }
    return 0;
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyWriteDataToGrid (WPARAM, LPARAM lParam)
{
    BCGP_GANTT_CONTROL_ROW* pRowInfo = (BCGP_GANTT_CONTROL_ROW*)lParam;
    if (pRowInfo != NULL)
    {
        return (LRESULT)OnGridRowWriteData (*pRowInfo);
    }
    return 0;
}
//*******************************************************************************
BOOL CBCGPGanttView::OnPreparePrinting(CPrintInfo* pInfo) 
{
	CBCGPGanttGrid* pWndGridCtrl = GetGrid ();
	if (pWndGridCtrl != NULL)
	{
		ASSERT_VALID (pWndGridCtrl);

		int nFirstItem = 0;										// By default print all grid items
		int nLastItem  = max(pWndGridCtrl->GetTotalItems () - 1, 0);	// from first row to the last
		pWndGridCtrl->OnPreparePrintPages (pInfo, nFirstItem, nLastItem);
	}

	return DoPreparePrinting(pInfo);
}
//*******************************************************************************
void CBCGPGanttView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	CBCGPGanttGrid* pWndGridCtrl = GetGrid ();
	if (pWndGridCtrl != NULL)
	{
		ASSERT_VALID (pWndGridCtrl);

		ASSERT_VALID (pDC);
		ASSERT (pInfo != NULL);

		int nFirstItem = 0;										// By default print all grid items
		int nLastItem  = max(pWndGridCtrl->GetTotalItems () - 1, 0);	// from first row to the last
		pWndGridCtrl->OnPreparePrintPages (pInfo, nFirstItem, nLastItem);

		ASSERT (pInfo == pWndGridCtrl->m_PrintParams.m_pPrintInfo);
		ASSERT (pInfo->m_lpUserData != NULL);

		pWndGridCtrl->m_bIsPrinting = TRUE;
		pWndGridCtrl->m_pPrintDC = pDC;

		pWndGridCtrl->OnBeginPrinting (pDC, pInfo);

		// The printable area has not been initialized. Initialize it.
		pInfo->m_rectDraw.SetRect (0, 0,
								pDC->GetDeviceCaps(HORZRES), 
								pDC->GetDeviceCaps(VERTRES));

		// Page margins:
		CRect rectMargins = pWndGridCtrl->OnGetPageMargins (pDC, pInfo);
		pInfo->m_rectDraw.DeflateRect (&rectMargins);

		// Specify pages count:
		int nPagesCount = pWndGridCtrl->OnCalcPrintPages (pDC, pInfo);
		pInfo->SetMaxPage (nPagesCount);
	}
}
//*******************************************************************************
void CBCGPGanttView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	CBCGPGanttGrid* pWndGridCtrl = GetGrid ();
	if (pWndGridCtrl != NULL)
	{
		ASSERT_VALID (pWndGridCtrl);
		pWndGridCtrl->OnEndPrinting (pDC, pInfo);
		
		pWndGridCtrl->m_bIsPrinting = FALSE;
		pWndGridCtrl->m_pPrintDC = NULL;

		pWndGridCtrl->AdjustLayout ();
	}
}
//*******************************************************************************
void CBCGPGanttView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	CBCGPGanttGrid* pWndGridCtrl = GetGrid ();
	if (pWndGridCtrl != NULL)
	{
		ASSERT_VALID (pDC);
		ASSERT (pInfo != NULL);
		ASSERT_VALID (pWndGridCtrl);

		// don't do anything if not fully initialized
		if (!pWndGridCtrl->m_bIsPrinting || pWndGridCtrl->m_pPrintDC == NULL)
		{
			return;
		}

		// Page margins:
		CRect rectMargins = pWndGridCtrl->OnGetPageMargins (pDC, pInfo);
		pInfo->m_rectDraw.DeflateRect (&rectMargins);

		pWndGridCtrl->OnPrint (pDC, pInfo);
	}
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyCreateChart (WPARAM, LPARAM)
{
	return (LRESULT)OnCreateChart ();
}
//*******************************************************************************
LRESULT CBCGPGanttView::OnNotifyCreateGrid (WPARAM, LPARAM)
{
	return (LRESULT)OnCreateGrid ();
}
//*******************************************************************************
CBCGPGanttChart* CBCGPGanttView::OnCreateChart ()
{
	return NULL; // use default chart implementation
}
//*******************************************************************************
CBCGPGanttGrid* CBCGPGanttView::OnCreateGrid ()
{
	return NULL; // use default grid implementation
}
//**************************************************************************
LRESULT CBCGPGanttView::OnChangeVisualManager (WPARAM wp, LPARAM lp)
{
	if (m_wndGanttControl.GetSafeHwnd() != NULL)
	{
		m_wndGanttControl.SendMessage(BCGM_CHANGEVISUALMANAGER, wp, lp);
	}

	return 0;
}

#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)
