// XTPReportHeader.cpp : implementation of the CXTPReportHeader class.
//
// This file is a part of the XTREME REPORTCONTROL MFC class library.
// (c)1998-2011 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME TOOLKIT PRO LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"
#include "Common/Resource.h"

#include "Common/XTPDrawHelpers.h"
#include "Common/XTPPropExchange.h"
#include "Common/XTPToolTipContext.h"
#include "Common/XTPResourceManager.h"

#include "XTPReportColumn.h"
#include "XTPReportColumns.h"
#include "XTPReportRecordItem.h"
#include "XTPReportControl.h"
#include "XTPReportPaintManager.h"
#include "XTPReportSubListControl.h"
#include "XTPReportFilterEditControl.h"
#include "XTPReportDragDrop.h"
#include "XTPReportInplaceControls.h"
#include "XTPReportHeader.h"
#include "XTPReportRecord.h"
#include "XTPReportRecords.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXTPReportHeader
int CXTPReportHeader::s_nMinAutoScrollStep = 3;
BOOL CXTPReportHeader::s_bShowItemsInGroupsPXDefault = TRUE;
BOOL CXTPReportHeader::s_bSendContextMenuForWholeHeaderArea = FALSE;

CXTPReportHeader::CXTPReportHeader(CXTPReportControl* pControl, CXTPReportColumns* pColumns)
	: m_pColumns(pColumns), m_pControl(pControl)
{
	m_nResizeCaptureRange = 3;
	m_pSubList = NULL;
	m_pFilterEdit = NULL;
	m_dragMode = dragNothing;
	m_nDropIndex = -1;
	m_nVisIndex = -1;
	m_rcHeader.SetRect(0, 0, 0, 0);
	m_pDropWnd = NULL;
	m_pDragWnd = NULL;
	m_pHotTrackingColumn = NULL;
	m_bAllowColumnResize = TRUE;
	m_bAllowColumnRemove = TRUE;
	m_bAllowColumnReorder = TRUE;
	m_bAllowColumnSort = TRUE;
	m_bHideColumnAfterGroupBoxDrop = TRUE;
	m_nForceLayoutAdjustment = 0;

	m_bLastColumnExpand = FALSE;
	m_bLastColumnExpandKeep = FALSE;

	// Release 13.1
	m_pDragColumn = NULL;
	m_pResizeColumn = NULL;
	m_hResizeCursor = XTPResourceManager()->LoadCursor(XTP_IDC_VRESIZE);
	m_hDontDropCursor = XTPResourceManager()->LoadCursor(XTP_IDC_REPORT_NODROP);
	m_bShowItemsInGroups = FALSE;
	m_bDragHeader = m_bDragGroupBox = FALSE;
	m_nIndentLevel = 0;
	m_bAutoColumnSizing = TRUE;
	m_nHeaderWidth = 0;

	m_nMaxColumnWidth = 0;

	EnableAutomation();
}

CXTPReportHeader::~CXTPReportHeader()
{
	DestroyDropWnd();

	if (m_pDragWnd)
		m_pDragWnd->DestroyWindow();
}

CXTPReportPaintManager* CXTPReportHeader::GetPaintManager() const
{
	return m_pControl->GetPaintManager();
}

void CXTPReportHeader::Draw(CDC* pDC, CRect rcHeader, int nLeftOffset)
{
	int PageWidth = GetPaintManager()->GetPrintPageWidth();
	int HeaderWidth = rcHeader.Width();
	int HeaderHeight = rcHeader.Height();
	int nFreezeCols = GetControl()->GetFreezeColumnsCount();

	rcHeader.OffsetRect(-nLeftOffset, 0);
	m_rcHeader = rcHeader;

	// draw background
	CXTPClientRect rcHeaderClientArea(m_pControl);
	rcHeaderClientArea.top = rcHeader.top;
	rcHeaderClientArea.bottom = rcHeader.bottom;

	if (!m_pControl->IsHeaderVisible() && m_pControl->IsGroupByVisible())
		rcHeaderClientArea.top--; //fix for Office2003 - Office2007 Column Style

	GetPaintManager()->FillHeaderControl(pDC, rcHeaderClientArea);

	int x = rcHeader.left;

	// draw items
	int nColumnsCount = m_pColumns->GetCount(), nColumn;
	for (nColumn = 0; nColumn < nColumnsCount; nColumn++)
	{
		CXTPReportColumn* pColumn = m_pColumns->GetAt(nColumn);

		if (pColumn && pColumn->IsVisible())
		{
			CRect rcItem(x, rcHeader.top, x + pColumn->GetWidth(), rcHeader.bottom);
			pColumn->m_rcColumn = rcItem;

			if (CRect().IntersectRect(rcHeaderClientArea, rcItem))
				GetPaintManager()->DrawColumn(pDC, pColumn, this, rcItem);

			x += rcItem.Width();
			pColumn->m_nMaxItemWidth = 0;
		}
	}

	int xFreeze = 0;

	// draw freeze items
	for (nColumn = 0; nFreezeCols > 0 && nColumn < nColumnsCount; nColumn++)
	{
		CXTPReportColumn* pColumn = m_pColumns->GetAt(nColumn);

		if (pColumn && pColumn->IsVisible())
		{
			CRect rcItem(xFreeze, rcHeader.top, xFreeze + pColumn->GetWidth(), rcHeader.bottom);
			pColumn->m_rcColumn = rcItem;

			if (CRect().IntersectRect(rcHeaderClientArea, rcItem))
			{
				GetPaintManager()->FillHeaderControl(pDC, pColumn->m_rcColumn);
				GetPaintManager()->DrawColumn(pDC, pColumn, this, rcItem);

				if (nFreezeCols == 1 &&
					(GetPaintManager()->GetFreezeColsDividerStyle() & xtpReportFreezeColsDividerHeader))
					GetPaintManager()->DrawFreezeColsDivider(pDC, rcItem, GetControl());
			}

			xFreeze += rcItem.Width();

			nFreezeCols--;
		}
	}

	// draw empty column to fill free header area
	if (x < rcHeaderClientArea.right)
	{
		CRect rcEmptyColumn = rcHeaderClientArea;
		rcEmptyColumn.left = x;
		rcEmptyColumn.right += 10;

		CXTPReportColumn colEmpty(0, _T(""), rcEmptyColumn.Width(), FALSE,
			XTP_REPORT_NOICON, FALSE, TRUE);
		ASSERT(m_pColumns);
		colEmpty.m_pColumns = m_pColumns;

		GetPaintManager()->DrawColumn(pDC, &colEmpty, this, rcEmptyColumn);
	}

	if (GetPaintManager()->IsColumnWidthWYSIWYG()
		&& !pDC->IsPrinting()
		&& GetPaintManager()->IsShowWYSIWYGMarkers()
		&& PageWidth > 0 && PageWidth <= HeaderWidth
		&& HeaderWidth > 1)
	{
		for (int st = 1; st <= HeaderWidth / PageWidth; st++)
		{
			CRect rc;
			if (m_bShowItemsInGroups || GetControl()->IsGroupByVisible())
				rc = CRect(PageWidth * st, HeaderHeight, PageWidth * st + 2, 3 * HeaderHeight);//group row mode
			else
				rc = CRect(PageWidth * st, 0, PageWidth * st + 2, HeaderHeight); //non-group row mode
			pDC->Draw3dRect(rc, RGB(255,0,0), RGB(255,0,0));
		}
	}
}

void CXTPReportHeader::CancelMouseMode()
{
	if (m_pControl->GetMouseMode() != xtpReportMouseNothing)
	{
		m_pControl->SetMouseMode(xtpReportMouseNothing);

		m_pDragColumn = NULL;
		m_pResizeColumn = NULL;

		SetHotDivider(-1);

		// Drag wnd cleanup
		if (m_pDragWnd != NULL)
		{
			m_pDragWnd->DestroyWindow();
			m_pDragWnd = NULL;
		}

		if (CWnd::GetCapture() == m_pControl)
			ReleaseCapture();

		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

		m_bDragGroupBox = FALSE;
		m_bDragHeader = FALSE;

		m_pControl->RedrawControl();
	}
}

void CXTPReportHeader::SetAutoColumnSizing(BOOL bAutoColumnSizing)
{
	m_pControl->m_nLeftOffset = 0;
	m_bAutoColumnSizing = bAutoColumnSizing;

	if (bAutoColumnSizing && m_pControl->m_hWnd)
		m_pControl->EnableScrollBarCtrl(SB_HORZ, FALSE);

	AdjustColumnsWidth(m_pControl->m_rcHeaderArea.Width());
	m_pControl->RedrawControl();
}

void CXTPReportHeader::OnColumnsChanged(int nReason, CXTPReportColumn* pColumn)
{
	// Release 13.1
	if (m_pControl != NULL
		&& m_pControl->m_bFreezeColumnsAbs
		&& m_pControl->m_nFreezeColumnsCount > 0
		&& pColumn != NULL)
	{
		int indViz = pColumn->GetVisibleIndex();
		int ind = pColumn->GetIndex();

		BOOL bOrd = (nReason & xtpReportColumnOrderChanged) > 0;
		BOOL bShow = (nReason & xtpReportColumnShown) > 0;
		BOOL bHide = (nReason & xtpReportColumnHidden) > 0;
		BOOL bGrAdd = (nReason & xtpReportColumnAddedToGroupby) > 0;
		BOOL bGrRem = (nReason & xtpReportColumnRemovedFromGroupby) > 0;
		BOOL bGrMode = (m_dragMode == dragInGroupBox);
		//TRUE if column is going to GroupBox
//TRACE(_T("Ord=%d Show=%d Hide=%d GrAdd=%d GrRem=%d m_nVisInd=%d VisInd=%d Freeze=%d GrMode = %d\n"), bOrd, bShow, bHide, bGrAdd, bGrRem, m_nVisIndex, indViz, m_pControl->m_nFreezeColumnsCount, bGrMode);

		if (bGrMode && bOrd && bHide)
		{
			if (m_pControl->m_nFreezeColumnsCount == pColumn->m_nOldVisibleIndex + 1)// remove freezed column
			{
				if (m_pControl->m_bMovePivot)
					m_pControl->m_nFreezeColumnsCount--;
				else
					m_pControl->m_nFreezeColumnsCount = 0; // no freeze columns
//TRACE(_T("bGrMode && bOrd && bHide OldVisibleIndex 0\n"));
			}
			else if (m_pControl->m_nFreezeColumnsCount > pColumn->m_nOldVisibleIndex + 1)
			{
				m_pControl->m_nFreezeColumnsCount--;
//TRACE(_T("bGrMode && bOrd && bHide OldVisibleIndex\n"));
			}
		}
		else if (bGrMode)
		{
			if (m_nVisIndex < m_pControl->m_nFreezeColumnsCount)
				m_pControl->m_nFreezeColumnsCount--;
			else if (m_pControl->m_nFreezeColumnsCount == m_nVisIndex)// remove freezed column
			{
				if (m_pControl->m_bMovePivot)
					m_pControl->m_nFreezeColumnsCount--;
				else
					m_pControl->m_nFreezeColumnsCount = 0; // no freeze columns
			}
		}
		else if (bOrd && bShow)
		{
//TRACE(_T("bOrd && bShow\n"));
			if (indViz < m_pControl->m_nFreezeColumnsCount)
			{
				m_pControl->m_nFreezeColumnsCount++;
//TRACE(_T("bOrd && bShow ++\n"));
			}
		}
		else if (bOrd && bHide)
		{
//TRACE(_T("bOrd && bHide m_nVisInd=%d VisInd=%d Ind=%d OldVisInd=%d\n"), m_nVisIndex, indViz, ind, pColumn->m_nOldVisibleIndex);
			if (m_pControl->m_nFreezeColumnsCount == pColumn->m_nOldVisibleIndex + 1)
			// this a case of menu command - no drag mode!
			// remove freezed column with direct command Hide
			{
				if (m_pControl->m_bMovePivot)
					m_pControl->m_nFreezeColumnsCount--;
				else
					m_pControl->m_nFreezeColumnsCount = 0;
//TRACE(_T("bOrd && bHide 0\n"));
			}
			else if (m_nVisIndex == -1 && indViz == -1)
			{
				if (pColumn->m_nOldVisibleIndex < m_pControl->m_nFreezeColumnsCount)
				{
					m_pControl->m_nFreezeColumnsCount--;
//TRACE(_T("bOrd && bHide OldVisibleIndex\n"));
				}
				else if (indViz > -1 && indViz < m_pControl->m_nFreezeColumnsCount)
				{
					m_pControl->m_nFreezeColumnsCount--;
//TRACE(_T("bOrd && bHide indViz\n"));
				}
			}
			else if (ind < m_pControl->m_nFreezeColumnsCount)
			{
				m_pControl->m_nFreezeColumnsCount--;
//TRACE(_T("bOrd && bHide ind\n"));
			}
			else if (indViz > -1 && indViz < m_pControl->m_nFreezeColumnsCount)
			{
				m_pControl->m_nFreezeColumnsCount--;
//TRACE(_T("bOrd && bHide indViz\n"));
			}
			else if (m_nVisIndex > -1 && m_nVisIndex < m_pControl->m_nFreezeColumnsCount)
			{
				m_pControl->m_nFreezeColumnsCount--;
//TRACE(_T("bOrd && bHide m_nVisIndex\n"));
			}
		}
		else if (bOrd && bGrAdd)
		{
//TRACE(_T("bOrd && bGrAdd\n"));
			if (m_nVisIndex < m_pControl->m_nFreezeColumnsCount)
			{
				m_pControl->m_nFreezeColumnsCount--;
//TRACE(_T("bOrd && bGrAdd --\n"));
			}
			else if (m_pControl->m_nFreezeColumnsCount == m_nVisIndex) // remove freezed column
			{
				if (m_pControl->m_bMovePivot)
					m_pControl->m_nFreezeColumnsCount--;
				else
					m_pControl->m_nFreezeColumnsCount = 0; // no freeze columns
			}
		}
		else if (bOrd && bGrRem)
		{
//TRACE(_T("bOrd && bGrRem\n"));
			if (indViz < m_pControl->m_nFreezeColumnsCount)
			{
				m_pControl->m_nFreezeColumnsCount++;
//TRACE(_T("bOrd && bGrRem ++\n"));
			}
			else if (m_pControl->m_nFreezeColumnsCount == m_nVisIndex + 1)// remove freezed column
			{
				if (m_pControl->m_bMovePivot)
					m_pControl->m_nFreezeColumnsCount--;
				else
					m_pControl->m_nFreezeColumnsCount = 0; // no freeze columns
//TRACE(_T("bOrd && bGrRem 0\n"));
			}
		}
		else if (bOrd) // pure reorder case
		{
			if (m_pControl->m_nFreezeColumnsCount == m_nVisIndex + 1) //move freezed column
			{
				m_pControl->m_nFreezeColumnsCount += indViz - m_nVisIndex;
//TRACE(_T("bOrd indViz - m_nVisIndex\n"));
			}
			else //move non-freezed column
			{
				if (m_nVisIndex < m_pControl->m_nFreezeColumnsCount)
				{
					m_pControl->m_nFreezeColumnsCount--;
//TRACE(_T("bOrd --\n"));
				}
				else
				{
					m_pControl->m_nFreezeColumnsCount++;
//TRACE(_T("bOrd ++\n"));
				}
			}
		}
	}

	if (m_pControl->IsFullColumnScrolling()
		&& pColumn
		&& m_pControl->GetScrollPos(SB_HORZ) > (pColumn->IsVisible() ? pColumn->GetVisibleIndex() : pColumn->GetOldVisibleIndex()))
	{
		if (nReason & xtpReportColumnShown)
			m_rcHeader.left -= pColumn->GetWidth();
		else if (nReason & xtpReportColumnHidden)
			m_rcHeader.left += pColumn->GetWidth();
		AdjustColumnsWidth(m_pControl->m_rcHeaderArea.Width());
		m_pControl->OnHScroll(SB_THUMBTRACK, m_pControl->GetScrollPos(SB_HORZ), NULL);
	}
	else
	{
		AdjustColumnsWidth(m_pControl->m_rcHeaderArea.Width());
		//if (!GetPaintManager()->IsFixedRowHeight())
		//  m_pControl->AdjustScrollBars();
	}
	m_pControl->UpdateSubList();
	m_pControl->RedrawControl();

	XTP_NM_REPORTCOLUMNORDERCHANGED nmData;
	nmData.pColumn = pColumn;
	nmData.nReason = nReason;

	if (nReason & xtpReportColumnGroupOrderChanged)
		m_pControl->SendNotifyMessage(XTP_NM_REPORT_GROUPORDERCHANGED, (NMHDR*) &nmData);
	m_pControl->SendNotifyMessage(XTP_NM_REPORT_COLUMNORDERCHANGED, (NMHDR*) &nmData);
}

void CXTPReportHeader::TrackColumn(CXTPReportColumn* pColumn, CPoint point)
{
	CXTPClientRect rcControl(m_pControl);
	m_pControl->ClientToScreen(&point);

	CRect rcColumn(pColumn->GetRect());
	rcColumn.bottom = rcControl.bottom;
	m_pControl->ClientToScreen(&rcColumn);

	int nBottomMax = rcColumn.bottom;
	int nLeftMin = 0;

	CDC* pDC = m_pControl->GetDC();
	if (pDC)
	{
		CRect rcVisible;
		UINT uRes = pDC->GetBoundsRect(&rcVisible, 0);

		m_pControl->ReleaseDC(pDC);
		pDC = NULL;

		if ((uRes & DCB_SET) == DCB_SET)
		{
			m_pControl->ClientToScreen(&rcVisible);
			nBottomMax = min(rcVisible.bottom, nBottomMax);
			nLeftMin = rcVisible.left;
		}
	}

	int nMaxAvailWidth = GetMaxAvailWidth(pColumn);

	if (m_nMaxColumnWidth > 0)
	{
		if (pColumn != m_pColumns->GetLastVisibleColumn())
			nMaxAvailWidth = min(nMaxAvailWidth, m_nMaxColumnWidth);
	}

	// uncomment this line in condition if you need set
	// maximum width constraint dependence on column
	// autosize option, then client code must be following:
	//
	// m_wndReportCtrl.GetColumns()->GetAt(0)->SetAutoSize(FALSE); // first need cancel column autosize
	// m_wndReportCtrl.GetColumns()->GetAt(0)->SetMaxWidth(100);   // and then set maximum width

	if (pColumn->GetMaxWidth()/* > 0 && !pColumn->IsAutoSize()*/)
		nMaxAvailWidth = pColumn->GetMaxWidth();

	int nMinWidth = pColumn->GetMinWidth();

	CRect rcAvail(rcColumn.left + nMinWidth, rcColumn.top,
		rcColumn.left + nMaxAvailWidth, nBottomMax);

	CRect rcTracker(rcColumn.right, rcColumn.top, rcColumn.right + 1, nBottomMax);
	CRect rcBound(rcColumn.left - 1, rcColumn.top, rcColumn.left, nBottomMax);

	BOOL bLayoutRTL = m_pControl->GetExStyle() & WS_EX_LAYOUTRTL;

	if (bLayoutRTL)
	{
		rcAvail.SetRect(rcColumn.left - nMaxAvailWidth, rcColumn.top,
			rcColumn.right - nMinWidth, nBottomMax);
	}

	CXTPSplitterTracker tracker(TRUE, m_pControl->m_bDesktopTrackerMode);

	CRect rcBoundX = bLayoutRTL ? rcTracker : rcBound;

	if (!bLayoutRTL && rcBoundX.left > nLeftMin ||
		bLayoutRTL && rcBoundX.left < nLeftMin)
	{
		tracker.SetBoundRect(rcBoundX);
	}

	if (tracker.Track(m_pControl, rcAvail, bLayoutRTL ? rcBound : rcTracker, point, TRUE))
	{
		ResizeColumn(pColumn, rcTracker.left - rcBound.right);

		m_pControl->RedrawControl();
	}
}

void CXTPReportHeader::OnLButtonDblClk(CPoint ptClick)
{
	if (m_pControl->GetMouseMode() == xtpReportMouseOverColumnDivide)
	{
		CPoint ptLeftColumn(ptClick);
		ptLeftColumn.x -= m_nResizeCaptureRange + 1;
		CXTPReportColumn* pColumn = HitTest(ptLeftColumn);
		if (pColumn != NULL)
		{
			BestFit(pColumn);
		}
	}
}

void CXTPReportHeader::OnLButtonDown(CPoint ptClick)
{
	m_pControl->SetCapture();

	if (m_pControl->GetMouseMode() == xtpReportMouseOverColumnDivide)
	{
		CXTPReportColumn* pColumn = MouseOverColumnResizeArea(ptClick);

		if (pColumn != NULL
			&& pColumn->IsResizable()
			&& m_bAllowColumnResize)
		{
			if (!m_bAutoColumnSizing
				|| (!IsLastResizebleColumn(pColumn) && !IsLastVisibleColumn(pColumn)))
			{
				m_pControl->SetMouseMode(xtpReportMouseNothing);
				TrackColumn(pColumn, ptClick);
				return;
			}
		}
	}

	if (m_pControl->GetMouseMode() == xtpReportMouseNothing)
	{
		BOOL bDragHeader = FALSE;
		CXTPReportColumn* pColumn = NULL;
		if (m_rcHeader.PtInRect(ptClick))
		{
			int nColumn = HitTestHeaderColumnIndex(ptClick);
			pColumn = m_pColumns->GetAt(nColumn);

			bDragHeader = TRUE;
		}
		else if (m_rcGroupBy.PtInRect(ptClick))
		{
			int nColumn = FindGroupByColumn(ptClick, TRUE);
			pColumn = m_pColumns->GetGroupsOrder()->GetAt(nColumn);

		}
		if (pColumn != NULL)
		{
			StartDragging(pColumn, bDragHeader);

			m_ptMouse = ptClick;
			m_pControl->SetMouseMode(xtpReportMousePrepareDragColumn);
			m_pControl->RedrawControl();
		}
	}
}

void CXTPReportHeader::AdjustColumnsWidth(int nTotalWidth, int nFirstIndex)
{
	if (nTotalWidth == 0)
		return;

	if (m_bAutoColumnSizing)
	{
		int nColumnsWidth = 0;
		int nTotalColumnsWidth = 0;
		CXTPReportColumn* pLastAutoColumn = NULL;
		int nColumn;

		for (nColumn = nFirstIndex; nColumn < m_pColumns->GetCount(); nColumn++)
		{
			CXTPReportColumn* pColumn = m_pColumns->GetAt(nColumn);
			if (!pColumn->IsVisible())
				continue;

			if (pColumn->m_bAutoSize)
			{
				pLastAutoColumn = pColumn;
				nColumnsWidth += pColumn->m_nColumnAutoWidth;
			}
			else
			{
				nTotalWidth -= pColumn->GetWidth();
				nTotalColumnsWidth += pColumn->GetWidth();
			}
		}

		if (pLastAutoColumn && nTotalWidth > 0)
		{
			m_nForceLayoutAdjustment = 0;

			for (nColumn = nFirstIndex; nColumn < m_pColumns->GetCount(); nColumn++)
			{
				CXTPReportColumn* pColumn = m_pColumns->GetAt(nColumn);
				if (!pColumn->IsVisible())
					continue;

				if (pColumn->m_bAutoSize)
				{
					if (pColumn == pLastAutoColumn)
					{
						pColumn->m_nColumnStaticWidth = max(nTotalWidth, pColumn->GetMinWidth());
						if (pColumn->m_nColumnStaticWidth != pColumn->m_nColumnAutoWidth)
						{
							XTP_NM_REPORTCOLUMNRESIZE nmData;
							nmData.pColumn = pColumn;
							nmData.nPrevWidth = pColumn->m_nColumnAutoWidth;
							nmData.nNewWidth = pColumn->m_nColumnStaticWidth;
							m_pControl->SendNotifyMessage(XTP_NM_REPORT_COLUMNWIDTHCHANGED, (NMHDR*)&nmData);
						}
					}
					else
					{
						nColumnsWidth = max(1, nColumnsWidth);

						pColumn->m_nColumnStaticWidth =
							max(int(pColumn->m_nColumnAutoWidth * nTotalWidth / nColumnsWidth), pColumn->GetMinWidth());
						if (pColumn->m_nColumnStaticWidth != pColumn->m_nColumnAutoWidth)
						{
							XTP_NM_REPORTCOLUMNRESIZE nmData;
							nmData.pColumn = pColumn;
							nmData.nPrevWidth = pColumn->m_nColumnAutoWidth;
							nmData.nNewWidth = pColumn->m_nColumnStaticWidth;
							m_pControl->SendNotifyMessage(XTP_NM_REPORT_COLUMNWIDTHCHANGED, (NMHDR*)&nmData);
						}

						nTotalWidth -= pColumn->m_nColumnStaticWidth;
						nColumnsWidth -= pColumn->m_nColumnAutoWidth;
					}
				}
			}
		}
		m_nHeaderWidth = m_pControl->m_rcHeaderArea.Width();

		CXTPReportColumn* p_Column = NULL;
		int TotalMinWidth = 0;
		for (int nCol = nFirstIndex; nCol < m_pColumns->GetCount(); nCol++)
		{
			CXTPReportColumn* pColumn = m_pColumns->GetAt(nCol);
			if (pColumn)
			{
				if (!pColumn->IsVisible())
					continue;
				p_Column = pColumn;

				if (!m_pControl->m_bIconColumnIndexNotValid)
					TotalMinWidth += pColumn->GetWidth();
			}
		}
		m_nForceLayoutAdjustment = max(TotalMinWidth - m_nHeaderWidth, 0);

		if (TotalMinWidth > m_nHeaderWidth)
			m_nHeaderWidth = TotalMinWidth;
	}
	else
	{
		m_nHeaderWidth = 0;

		for (int nColumn = 0; nColumn < m_pColumns->GetCount(); nColumn++)
		{
			CXTPReportColumn* pColumn = m_pColumns->GetAt(nColumn);

			if (pColumn && pColumn->IsVisible())
				m_nHeaderWidth += pColumn->GetWidth();
		}

		if (m_nHeaderWidth == 0)
			m_nHeaderWidth = nTotalWidth;
	}
	m_pControl->AdjustScrollBars();
}

void CXTPReportHeader::ResizeColumn(CXTPReportColumn* pColumnResize, int nWidth)
{
	int nResizeIndex = 0;
	int nColumn;
	int nTotalWidth = 0;
	m_pResizeColumn = pColumnResize;

	ASSERT(pColumnResize->IsResizable());

	for (nColumn = 0; nColumn < m_pColumns->GetCount(); nColumn++)
	{
		CXTPReportColumn* pColumn = m_pColumns->GetAt(nColumn);
		if (!pColumn->IsVisible())
			continue;

		if (nResizeIndex > 0)
			nTotalWidth += pColumn->m_nColumnStaticWidth;

		if (pColumnResize == pColumn)
		{
			int nDelta = pColumn->GetWidth() - nWidth;

			nResizeIndex = nColumn + 1;
			nTotalWidth = - nWidth + pColumn->GetWidth();
			pColumn->m_nColumnStaticWidth = nWidth;

			if (m_bAutoColumnSizing)
			{
				// if next column is "last resizeble" but not "auto size" column - resize it too
				// if special flag m_pControl->GetPaintManager()->IsLastColumnWidthWYSIWYG() not used.
				CXTPReportColumn* pColNext = CXTPReportHeader::GetNextVisibleColumn(nColumn, 1);

				if (pColNext && !pColNext->IsAutoSize()
					&& !m_pControl->GetPaintManager()->IsLastColumnWidthWYSIWYG()
					&& pColNext->IsResizable() && IsLastResizebleColumn(pColNext))
				{
					pColNext->m_nColumnStaticWidth += nDelta;
					if (pColNext->m_nColumnStaticWidth != pColNext->m_nColumnAutoWidth)
					{
						XTP_NM_REPORTCOLUMNRESIZE nmData;
						nmData.pColumn = pColNext;
						nmData.nPrevWidth = pColNext->m_nColumnAutoWidth;
						nmData.nNewWidth = pColNext->m_nColumnStaticWidth;
						m_pControl->SendNotifyMessage(XTP_NM_REPORT_COLUMNWIDTHCHANGED, (NMHDR*)&nmData);
					}
				}
			}
		}
		if (pColumn->m_nColumnStaticWidth != pColumn->m_nColumnAutoWidth)
		{
			XTP_NM_REPORTCOLUMNRESIZE nmData;
			nmData.pColumn = pColumn;
			nmData.nPrevWidth = pColumn->m_nColumnAutoWidth;
			nmData.nNewWidth = pColumn->m_nColumnStaticWidth;
			m_pControl->SendNotifyMessage(XTP_NM_REPORT_COLUMNWIDTHCHANGED, (NMHDR*)&nmData);
		}
		pColumn->m_nColumnAutoWidth = pColumn->m_nColumnStaticWidth;
	}
	AdjustColumnsWidth(nTotalWidth, nResizeIndex);

	for (nColumn = 0; nColumn < m_pColumns->GetCount(); nColumn++)
	{
		CXTPReportColumn* pColumn = m_pColumns->GetAt(nColumn);
		pColumn->m_nColumnAutoWidth = pColumn->m_nColumnStaticWidth;
	}
	AdjustColumnsWidth(m_rcHeader.Width());

	if (!GetPaintManager()->IsFixedRowHeight())
		m_pControl->AdjustScrollBars();
	m_pResizeColumn = NULL;
}

void CXTPReportHeader::OnLButtonUp(UINT nFlags, CPoint ptClick)
{
	XTPReportMouseMode mouseMode = m_pControl->GetMouseMode();
	m_pControl->SetMouseMode(xtpReportMouseNothing);

	ReleaseCapture();

	if (mouseMode == xtpReportMouseDraggingColumn)
	{
		// End dragging column
		CXTPReportColumn* pDragColumn = m_pDragColumn;
		ASSERT(pDragColumn);
		if (!pDragColumn)
			return;
		BOOL bGroupOrderChanged = FALSE;

		// Drop column - change position
		if (m_dragMode == dragOutTarget || m_dragMode == dragFieldChooser)
		{
			if (m_bAllowColumnRemove && pDragColumn->m_bAllowRemove)
			{
				if (m_bDragHeader)
				{
					XTP_TRACE(_T("Removing column\n"));
					pDragColumn->SetVisible(FALSE);
				}
				else if (m_bDragGroupBox)
				{
					m_pColumns->GetGroupsOrder()->Remove(pDragColumn);
					bGroupOrderChanged = TRUE;
					m_pControl->Populate();
				}
			}
		}
		else if (((m_dragMode & dragInTarget) != 0)
			&& m_nDropIndex >= 0)
		{
			XTP_TRACE(_T("Changing column position to %d\n"), m_nDropIndex);

			CXTPClientRect rcClient(m_pControl);
			CRect rcHeaderFull = m_rcHeader;
			rcHeaderFull.right = rcClient.right;
			if (rcHeaderFull.PtInRect(ptClick))
			{
				int nDragIndex = m_pColumns->IndexOf(pDragColumn);

				pDragColumn->SetVisible();
				m_pColumns->ChangeColumnOrder(m_nDropIndex, nDragIndex);

				if (m_bDragGroupBox && ((nFlags & MK_CONTROL) == 0))
				{
					m_pColumns->GetGroupsOrder()->Remove(pDragColumn);
					bGroupOrderChanged = TRUE;
					m_pControl->Populate();
				}
			}
			else if (m_rcGroupBy.PtInRect(ptClick))
			{
				// set grouping by
				BOOL bFCS = m_pControl->IsFullColumnScrolling(); //<<>>
				//if (pDragColumn->GetIndex() > 0)
				if (pDragColumn->GetVisibleIndex() > 0)
					m_pControl->SetFullColumnScrolling(FALSE);

				m_pColumns->GetGroupsOrder()->InsertAt(m_nDropIndex, pDragColumn);
				bGroupOrderChanged = TRUE;

				if ((m_bHideColumnAfterGroupBoxDrop && m_bDragHeader && ((nFlags & MK_CONTROL) == 0)) ||
					(!m_bHideColumnAfterGroupBoxDrop && m_bDragHeader && ((nFlags & MK_CONTROL) == MK_CONTROL)))
				{
					pDragColumn->SetVisible(FALSE);
				}
				m_pControl->Populate();

				m_pControl->SetFullColumnScrolling(bFCS); //<<>>
			}
		}
		int nReason = bGroupOrderChanged ? xtpReportColumnGroupOrderChanged : xtpReportColumnOrderChanged;
		if (m_dragMode == dragFieldChooser)
			nReason |= xtpReportColumnAddedToFieldChooser;
		else if (m_dragMode == dragInHeader && !m_bDragHeader)
			nReason |= xtpReportColumnRemovedFromFieldChooser;
		if (bGroupOrderChanged && m_bDragGroupBox)
			nReason |= xtpReportColumnRemovedFromGroupby;
		else if (bGroupOrderChanged && m_dragMode == dragInGroupBox)
			nReason |= xtpReportColumnAddedToGroupby;
		m_dragMode = dragNothing;
		OnColumnsChanged(nReason, pDragColumn);

		SetHotDivider(-1);
		m_pDragColumn = NULL;

		// Drag wnd cleanup
		if (m_pDragWnd != NULL)
		{
			m_pDragWnd->DestroyWindow();
			m_pDragWnd = NULL;
		}

		m_bDragHeader = m_bDragGroupBox = FALSE;

		return; // Stop message processing
	}

	if (m_pDragColumn && m_pDragColumn == HitTest(ptClick))
	{
		if (m_pDragColumn->IsPlusMinus())
		{
			CXTPReportColumns* pColumns = m_pControl->GetColumns();
			CRect rcPlusMinus = m_pDragColumn->GetRect();
			BOOL bAction = (rcPlusMinus.right - ptClick.x <= 15); //click on Plus-Minus glyph only [13 + 2 pixels]
			if (bAction && pColumns != NULL)
			{
				BOOL bExp = m_pDragColumn->IsExpanded();
				m_pDragColumn->SetExpanded(!bExp);

				m_bDragHeader = FALSE;
				m_dragMode = dragNothing;

				m_pControl->SendMessageToParent(NULL, NULL, m_pDragColumn, XTP_NM_REPORT_PLUSMINUSCLICK, &ptClick);
				return;
			}
		}
		else
		{
			m_pControl->SendMessageToParent(NULL, NULL, m_pDragColumn, NM_CLICK, &ptClick);
		}
	}

	// End holding left mouse button - change sort order for the column
	if (mouseMode == xtpReportMousePrepareDragColumn)
	{
		if (!m_pDragColumn)
			return;

		CXTPReportColumn* pColumn = m_pDragColumn;

		m_pDragColumn = NULL;

		// change sort order
		if (pColumn->IsSortable() && m_bAllowColumnSort)
		{
			BOOL bGroupOrderChanged = FALSE;
			// do not reset sort order if clicked on the column header from the Group By area
			if (m_bDragHeader)
			{
				CXTPReportColumnOrder* pColumnOrder = m_bShowItemsInGroups
					&& pColumn->IsGroupable() ?
						m_pColumns->GetGroupsOrder() :
						m_pColumns->GetSortOrder();

				BOOL bColumnFound = pColumnOrder->IndexOf(pColumn) >= 0;

				if (GetKeyState(VK_SHIFT) >= 0)
				{
					if (m_bShowItemsInGroups && !IsAllowColumnRemove())
					{
						for (int i = 0; i < pColumnOrder->GetCount(); i++)
						{
							pColumnOrder->GetAt(i)->SetVisible();
						}
					}
					pColumnOrder->Clear();
					pColumnOrder->Add(pColumn);
					bGroupOrderChanged = m_bShowItemsInGroups;
				}
				else if (!bColumnFound)
				{
					pColumnOrder->Add(pColumn);
					bGroupOrderChanged = m_bShowItemsInGroups;
				}

				if (bColumnFound)
				{
					pColumn->m_bSortIncreasing = !pColumn->m_bSortIncreasing;
				}
			}
			else
			{
				pColumn->m_bSortIncreasing = !pColumn->m_bSortIncreasing;
			}

			CUIntArray m_arPreSel;
			if (m_pControl->m_bKeepSelectionAfterSort)
			{
				CXTPReportSelectedRows* pSelectedRows = m_pControl->GetSelectedRows();
				if (pSelectedRows)
				{
					int n = pSelectedRows->GetCount();
					for (int i = 0; i < n; i++)
					{
						CXTPReportRow* pRow = pSelectedRows->GetAt(i);
						if (pRow)
						{
							CXTPReportRecord* pRec = pRow->GetRecord();
							if (pRec)
								m_arPreSel.Add(pRec->GetIndex());
						}
					}
				}
			}

			m_pControl->SendNotifyMessage(XTP_NM_REPORT_PRESORTORDERCHANGED);

			if (bGroupOrderChanged)
				m_pControl->Populate();
			else
				m_pControl->ReSortRows();

			if (m_pControl->m_bKeepSelectionAfterSort)
			{
				CXTPReportRows* pRows = m_pControl->GetRows();
				CXTPReportRecords* pRecords = m_pControl->GetRecords();
				if (m_arPreSel.GetSize() > 0)
				{
					for (int i = 0; i < m_arPreSel.GetSize(); i++)
					{
						int j = m_arPreSel.GetAt(i);
						CXTPReportRecord* pRec = pRecords->GetAt(j);
						if (pRec)
						{
							CXTPReportRow* pRow = pRows->FindInTree(pRec);
							if (pRow)
								pRow->SetSelected(TRUE);
						}
					}
				}
			}

			m_pControl->SendNotifyMessage(XTP_NM_REPORT_SORTORDERCHANGED);

			if (bGroupOrderChanged)
			{
				OnColumnsChanged(xtpReportColumnGroupOrderChanged | xtpReportColumnAddedToGroupby | xtpReportColumnRemovedFromGroupby, pColumn);
			}
		}
		else
		{
			m_pControl->RedrawControl();
		}

		m_bDragHeader = m_bDragGroupBox = FALSE;
	}
}

CXTPReportColumn* CXTPReportHeader::HitTest(CPoint ptPoint) const
{
	int nIndex = HitTestHeaderColumnIndex(ptPoint);
	if (nIndex < 0 || m_pColumns == NULL)
		return NULL;

	return m_pColumns->GetAt(nIndex);
}

int CXTPReportHeader::HitTestHeaderColumnIndex(CPoint ptPoint) const
{
	if (!m_pControl->m_rcHeaderArea.PtInRect(ptPoint))
		return -1;

	int x = m_rcHeader.left;

	int nFreezeCols = GetControl()->GetFreezeColumnsCount();

	// enumerate items
	int nColumnCount = m_pColumns->GetCount();
	for (int nColumn = 0; nColumn < nColumnCount; nColumn++)
	{
		CXTPReportColumn* pColumn = m_pColumns->GetAt(nColumn);
		if (pColumn && pColumn->IsVisible())
		{
			x += pColumn->GetWidth();

			if (ptPoint.x < x - (nFreezeCols > 0 ? m_rcHeader.left : 0))
				return nColumn;

			nFreezeCols--;
		}
	}
	return -1;
}

//NEED TO REVIEW THIS CODE
int CXTPReportHeader::GetFulColScrollInfo(
	CXTPReportColumn*& rpPrev,
	CXTPReportColumn*& rpCurr,
	CXTPReportColumn*& rpNext,
	int& rnScrollPos,
	int& rnScrollMax) const
{
	int x = m_rcHeader.left;
	int nLBorderX = 0;

	rnScrollPos = -1;
	rnScrollMax = 0;
	rpPrev = rpCurr = rpNext = NULL;
	BOOL bFind = FALSE;
	int nFreezeCols = GetControl()->GetFreezeColumnsCount();

	// enumerate items
	int nColumnCount = m_pColumns->GetCount();
	for (int nColumn = 0; nColumn < nColumnCount; nColumn++)
	{
		CXTPReportColumn* pColumn = m_pColumns->GetAt(nColumn);
		if (pColumn && pColumn->IsVisible())
		{
			if (nFreezeCols <= 0)
				rnScrollMax++;

			if (!bFind)
			{
				if (nFreezeCols <= 0)
					rnScrollPos++;

				rpPrev = rpCurr;
				rpCurr = pColumn;

				x += pColumn->GetWidth();

				if (nFreezeCols > 0)
				{
					nLBorderX += pColumn->GetWidth();
					nFreezeCols--;
				}
				else
				{
					if (nLBorderX + 1 < x)
					{
						bFind = TRUE;
					}
				}
			}
			else if (!rpNext)
			{
				rpNext = pColumn;
			}
		}
	}
	return nLBorderX;
}

CXTPReportColumn* CXTPReportHeader::MouseOverColumnResizeArea(CPoint ptPoint)
{
	if (ptPoint.y >= m_rcHeader.bottom ||
		ptPoint.y <= m_rcHeader.top)
	{
		return NULL;
	}

	// enumerate columns
	int nVisColCount = m_pColumns->GetVisibleColumnsCount() - (m_bAutoColumnSizing ? 1 : 0);
	int nFreezeCols = min(GetControl()->GetFreezeColumnsCount(), nVisColCount);
	int xMaxFreezeR = 0;

	for (int nColumn = 0; nColumn < nVisColCount; nColumn++)
	{
		BOOL bFreezeCol = nColumn < nFreezeCols;
		int nColIdx = bFreezeCol ? nColumn : nVisColCount + nFreezeCols - 1 - nColumn;

		CXTPReportColumn* pColumn = m_pColumns->GetVisibleAt(nColIdx);
		ASSERT(pColumn->IsVisible());

		if (!pColumn->IsResizable())
			continue;

		int xBorderR = pColumn->GetRect().right;

		if (bFreezeCol)
		{
			xMaxFreezeR = max(xMaxFreezeR, xBorderR);
		}
		else if (xBorderR <= xMaxFreezeR)
		{
			return NULL;
		}

		if (abs(ptPoint.x - xBorderR) <= m_nResizeCaptureRange)
		{
			if (!m_bAutoColumnSizing
				|| (!IsLastResizebleColumn(pColumn) && !IsLastVisibleColumn(pColumn)))
				return pColumn;
		}
	}

	return NULL;
}

int CXTPReportHeader::FindHeaderColumn(CPoint ptPoint) const
{
	// Find column in header

	CXTPClientRect rcClient(m_pControl);
	CRect rcLastEmptyCol = m_rcHeader;
	rcLastEmptyCol.left = m_rcHeader.right;
	rcLastEmptyCol.right = rcClient.right;

	int nCount = m_pColumns->GetCount();
	int nVisColCount = m_pColumns->GetVisibleColumnsCount();

	if (rcLastEmptyCol.PtInRect(ptPoint))
	{
		return nCount;
	}

	if (!m_pControl->m_rcHeaderArea.PtInRect(ptPoint))
		return -1;

	int nFreezeCols = min(GetControl()->GetFreezeColumnsCount(), nVisColCount);
	int xMaxFreezeR = 0;

	for (int nColumn = 0; nColumn < nVisColCount; nColumn++)
	{
		BOOL bFreezeCol = nColumn < nFreezeCols;
		int nColIdx = bFreezeCol ? nColumn : nVisColCount + nFreezeCols - 1 - nColumn;
		BOOL bLastFreezeCol = nColumn + 1 == nFreezeCols;

		CXTPReportColumn* pColumn = m_pColumns->GetVisibleAt(nColIdx);
		ASSERT(pColumn->IsVisible());

		int xBorderR = pColumn->GetRect().right;

		if (bFreezeCol)
		{
			xMaxFreezeR = max(xMaxFreezeR, xBorderR);
		}
		else if (xBorderR <= xMaxFreezeR)
		{
			return -1;
		}

		CRect rcTest0 = pColumn->GetRect();
		if (!bFreezeCol)
		{
			rcTest0.left = max(xMaxFreezeR, rcTest0.left);
		}
		CRect rcTest1 = rcTest0;

		if (rcTest0.left != pColumn->GetRect().left)
		{
			rcTest0.SetRect(0, 0, 0, 0);
		}
		else if (rcTest0.right > rcClient.right)
		{
			rcTest1.SetRect(0, 0, 0, 0);
		}
		else
		{
			rcTest0.right -= rcTest0.Width()/2;
			rcTest1.left += rcTest0.Width()/2;
		}

		if (rcTest0.PtInRect(ptPoint))
		{
			return pColumn->GetIndex();
		}
		if (rcTest1.PtInRect(ptPoint))
		{
			if (bLastFreezeCol && GetControl()->m_nLeftOffset != 0)
			{
				return pColumn->GetIndex();
			}
			else
			{
				return pColumn->GetIndex() + 1;
			}
		}
	}
	return 0;
}

int CXTPReportHeader::FindGroupByColumn(CPoint ptPoint, BOOL bExactSearch) const
{
	// Find column in Group By area
	if (m_rcGroupBy.PtInRect(ptPoint))
	{
		CXTPReportPaintManager* pPaintManager = GetPaintManager();
		int nHeaderHeight = pPaintManager->GetHeaderHeight();

		if (bExactSearch)
		{
			int x = m_rcGroupBy.left + 9;
			int y = m_rcGroupBy.top + 7;
			//int nTargetColumn = -1;
			CXTPReportColumn* pColumn = NULL;

			// draw items
			int nColumnsCount = m_pColumns->GetGroupsOrder()->GetCount();
			for (int nColumn = 0; nColumn < nColumnsCount; nColumn++)
			{
				pColumn = m_pColumns->GetGroupsOrder()->GetAt(nColumn);
				if (pColumn)
				{
					CRect rcItem(x, y, x + pColumn->m_rcGroupBy.Width(), y + nHeaderHeight - 3);

					if (rcItem.PtInRect(ptPoint))
						return nColumn;

					// next column
					x = rcItem.right + 5;
					y = rcItem.top + rcItem.Height() / 2;
				}
			}
			return nColumnsCount;
		}
		else
		{
			int xLeft = m_rcGroupBy.left + 9;
			int xRight;
			//int nTargetColumn = -1;
			CXTPReportColumn* pColumn = NULL;

			// draw items
			int nColumnsCount = m_pColumns->GetGroupsOrder()->GetCount();
			for (int nColumn = 0; nColumn < nColumnsCount; nColumn++)
			{
				pColumn = m_pColumns->GetGroupsOrder()->GetAt(nColumn);

				if (pColumn)
				{
					xRight = xLeft + pColumn->m_rcGroupBy.Width();

					if (ptPoint.x <= xRight)
					{
						if (ptPoint.x <= (xLeft + xRight) / 2)
							return nColumn;
						return nColumn + 1;
					}
					// next column
					xLeft = xRight + 5;
				}
			}
			return nColumnsCount;
		}
	}
	return -1;
}

void CXTPReportHeader::DestroyDropWnd()
{
	if (m_pDropWnd != NULL)
	{
		m_pDropWnd->DestroyWindow();
		m_pDropWnd = NULL;
	}
}

int CXTPReportHeader::SetHotDivider(int nIndex, BOOL bHeader)
{
	m_nDropIndex = nIndex;

	if (nIndex < 0)
	{
		DestroyDropWnd();
		return nIndex;
	}

	CXTPReportPaintManager* pPaintManager = GetPaintManager();
	int nHeaderHeight = pPaintManager->GetHeaderHeight();

	// compare hot divider left and right columns with dragging column
	//  and do not show hot divider near dragging column
	CXTPReportColumn* pDraggingColumn = GetDraggingColumn();

	CPoint pt;
	if (bHeader)
	{
		if (pDraggingColumn)
		{
			// find column following hot divider column
			CXTPReportColumn* pLeftColumn = NULL;
			CXTPReportColumn* pRightColumn = NULL;
			int nTotalColumnCount = m_pColumns->GetCount();
			// find left visible column
			for (int nLeftColumnIndex = nIndex - 1; nLeftColumnIndex >= 0; nLeftColumnIndex--)
			{
				if (nLeftColumnIndex < nTotalColumnCount)
				{
					pLeftColumn = m_pColumns->GetAt(nLeftColumnIndex);
					if (pLeftColumn->IsVisible())
						break;

					pLeftColumn = NULL;
				}
			}
			// find right visible column
			for (int nRightColumnIndex = nIndex; nRightColumnIndex < nTotalColumnCount; nRightColumnIndex++)
			{
				pRightColumn = m_pColumns->GetAt(nRightColumnIndex);
				if (pRightColumn->IsVisible())
					break;

				pRightColumn = NULL;
			}
			// compare
			if ((pDraggingColumn == pLeftColumn) || (pDraggingColumn == pRightColumn))
			{
				DestroyDropWnd();
				return -1;
			}
		}

		pt.y = m_rcHeader.CenterPoint().y;
		pt.x = m_rcHeader.left;

		if (nIndex < m_pColumns->GetCount())
		{
			CXTPReportColumn* pColumnL = m_pColumns->GetAt(nIndex);

			if (pColumnL->IsVisible())
			{
				pt.x = pColumnL->GetRect().left;
			}
			else
			{
				if (nIndex > 0 && m_pColumns->GetAt(nIndex-1))
				{
					CXTPReportColumn* pColumnR = m_pColumns->GetAt(nIndex-1);
					ASSERT(pColumnR->IsVisible());
					pt.x = pColumnR->GetRect().right;
				}
			}
		}
		else
		{
			pt.x = m_rcHeader.right;
		}

		if (pt.x < 0)
		{
			DestroyDropWnd();
			return -1;
		}
	}
	else
	{
		int nDraggingIndex = m_pColumns->GetGroupsOrder()->IndexOf(pDraggingColumn);
		if (nDraggingIndex != -1 && (nDraggingIndex == nIndex || nDraggingIndex == nIndex - 1))
		{
			DestroyDropWnd();
			return -1;
		}

		pt.x = m_rcGroupBy.left + 9;
		pt.y = m_rcGroupBy.top + 6 + nHeaderHeight / 2;

		int nColumnCount = m_pColumns->GetGroupsOrder()->GetCount();
		int nLastColumn = min(nColumnCount, nIndex);
		for (int nColumn = 0; nColumn < nLastColumn; nColumn++)
		{
			CXTPReportColumn* pColumn = m_pColumns->GetGroupsOrder()->GetAt(nColumn);
			if (pColumn)
			{
				pt.x += pColumn->m_rcGroupBy.Width();
				if (nColumn < nColumnCount - 1)
				{
					pt.x += 5;
					pt.y += (nHeaderHeight - 3) / 2 ;
				}
			}
		}
	}

	if (m_pDropWnd == NULL)
	{
		m_pDropWnd = new CXTPReportHeaderDropWnd(pPaintManager->m_clrHotDivider);
		if (m_pDropWnd)
			m_pDropWnd->Create(m_pControl, nHeaderHeight);
	}

	if (m_pDropWnd)
	{
		m_pControl->ClientToScreen(&pt);
		m_pDropWnd->SetWindowPos(pt.x, pt.y);
	}

	return nIndex;
}

int CXTPReportHeader::GetMaxAvailWidth(CXTPReportColumn* pColumnCheck)
{
	int nTotalWidth = m_pControl->m_rcHeaderArea.Width();

	if (!m_bAutoColumnSizing)
		return 32000;

	int nWidth = 0;

	for (int i = 0; i < m_pColumns->GetCount(); i++)
	{
		CXTPReportColumn* pColumn = m_pColumns->GetAt(i);
		if (!pColumn->IsVisible())
			continue;

		if (nWidth > 0)
		{
			nWidth -= pColumn->GetMinWidth();
		}

		if (pColumn == pColumnCheck)
		{
			nWidth = nTotalWidth - pColumnCheck->GetRect().left;
		}
	}

	return nWidth;
}

CXTPReportColumn* CXTPReportHeader::GetHotTrackingColumn() const
{
	return m_pDragColumn && IsDragHeader() ? m_pDragColumn : m_pHotTrackingColumn;
}

void CXTPReportHeader::SetHotTrackingColumn(CXTPReportColumn* pColumn)
{
	if (!GetPaintManager()->IsColumHotTrackingEnabled())
		pColumn = NULL;

	if (m_pHotTrackingColumn != pColumn)
	{
		m_pHotTrackingColumn = pColumn;
		m_pControl->RedrawControl();

		if (!CWnd::GetCapture())
		{
			TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_pControl->GetSafeHwnd(), 0};
			_TrackMouseEvent (&tme);
		}
	}
}

void CXTPReportHeader::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	XTPReportMouseMode mouseMode = m_pControl->GetMouseMode();

	CXTPReportColumn* pCol = HitTest(point);

	SetHotTrackingColumn(pCol);

	if (pCol != NULL && pCol->IsPlusMinus())
	{
		CRect rcPlusMinus = pCol->GetRect();
		if (rcPlusMinus.right - point.x <= 20 && rcPlusMinus.right - point.x > 15)
		{
			CXTPToolTipContext* pTipContext = m_pControl->GetToolTipContext();
			if (pTipContext)
				pTipContext->CancelToolTips();
		}
	}

	if (m_bAllowColumnResize
		&& (mouseMode == xtpReportMouseNothing || mouseMode == xtpReportMouseOverColumnDivide)
		&& MouseOverColumnResizeArea(point))
	{
		if (mouseMode == xtpReportMouseNothing)
			SetCursor(m_hResizeCursor);

		m_pControl->SetMouseMode(xtpReportMouseOverColumnDivide);
		return;
	}

	if (mouseMode == xtpReportMousePrepareDragColumn)
	{
		CXTPReportColumn* pColumn = GetDraggingColumn();
		if (!pColumn)
			return;

		if (!m_bAllowColumnReorder
			&& (!m_bAllowColumnRemove || !pColumn->m_bAllowRemove)
			&& !m_pControl->IsGroupByVisible())
			return;

		if (abs(point.x - m_ptMouse.x) + abs(point.y - m_ptMouse.y) < 4)
			return;

		if (!(pColumn->IsAllowDragging()
			&& pColumn->GetIndex() >= m_pControl->GetDisableReorderColumnsCount()))
		{
			m_pDragColumn = NULL;
			m_pControl->SetMouseMode(xtpReportMouseNothing);
			m_pControl->RedrawControl();
		}
		else
		{
			m_pControl->SetMouseMode(xtpReportMouseDraggingColumn);
			m_dragMode = m_bDragHeader ? dragInHeader : dragInGroupBox;
			// set dragging cursor
			ASSERT(m_pDragWnd == NULL);
			m_pDragWnd = new CXTPReportHeaderDragWnd();

			CRect rcItem(pColumn->GetRect());
			if (!m_bDragHeader)
			{
				// set sizes as drawing external
				rcItem.right = rcItem.left + pColumn->m_rcGroupBy.Width();
			}
			m_pControl->ClientToScreen(&rcItem);
			m_pDragWnd->Create(rcItem, this, GetPaintManager(), pColumn);
			m_nVisIndex = pColumn->GetVisibleIndex();
			// Release 13.1
//TRACE(_T("OnMouseMove m_nVisIndex=%d\n"), m_nVisIndex);
			return;
		}
	}

	if (mouseMode == xtpReportMouseDraggingColumn)
	{
		if (!m_pDragColumn)
			return;

		CPoint ptScreen = point;
		m_pControl->ClientToScreen(&ptScreen);

		if (m_pDragWnd)
		{
			CRect rcWnd;
			m_pDragWnd->GetWindowRect(&rcWnd);

			CPoint pt(ptScreen);
			pt.Offset(-(rcWnd.Width() >> 1), -(rcWnd.Height() >> 1));

			m_pDragWnd->SetWindowPos(&CWnd::wndTop,
				pt.x, pt.y, 0, 0,
				SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
		}

		if (m_pSubList
			&& m_pSubList->GetSafeHwnd()
			&& m_pSubList->IsWindowVisible()
			&& CXTPWindowRect(m_pSubList).PtInRect(ptScreen))
		{
			if (m_dragMode == dragFieldChooser)
				return;

			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

			SetHotDivider(-1);
			m_dragMode = dragFieldChooser;
			return;
		}

		CXTPClientRect rcClient(m_pControl);
		CRect rcDropTarget(m_rcHeader);
		if (m_pDragColumn->IsGroupable())
			rcDropTarget.UnionRect(m_rcHeader, m_rcGroupBy);
		rcDropTarget.right = rcClient.right;

		if (rcDropTarget.PtInRect(point))
		{
			CRect rcHeaderFull = m_rcHeader;
			rcHeaderFull.right = rcClient.right;
			BOOL bHeaderPoint = rcHeaderFull.PtInRect(point);
			// change dropping place
			int nDropPos = bHeaderPoint ? FindHeaderColumn(point) : FindGroupByColumn(point);

			if (bHeaderPoint
				&& !(m_bAllowColumnReorder && nDropPos > (m_pControl->GetDisableReorderColumnsCount() - 1))/* && m_bDragHeader*/)
				nDropPos = -1;

			XTP_TRACE(_T("nDropPos = %i, m_nDropIndex = %i\n"), nDropPos, m_nDropIndex);

			ReportDraggingMode dragMode = bHeaderPoint ? dragInHeader : dragInGroupBox;

			// dropping will change order
			if ((m_dragMode & dragInTarget) == 0)
			{
				// Change drag mode
				XTP_TRACE(_T("Switch IN point = %d %d\t"), point.x, point.y);

				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
			}

			// redraw control with new arrows if dropping place changed
			//if (nDropPos != m_nDropIndex || m_dragMode != dragMode)
			//{
				SetHotDivider(nDropPos, bHeaderPoint);
			//}
			m_dragMode = dragMode;
		}
		else
		{
			// dropping will remove the column
			if (m_dragMode != dragOutTarget)
			{
				// change drag mode
				XTP_TRACE(_T("Switch OUT point = %d %d\t"), point.x, point.y);

				if (m_bAllowColumnRemove && m_pDragColumn->m_bAllowRemove)
					SetCursor(m_hDontDropCursor);

				SetHotDivider(-1);

				m_dragMode = dragOutTarget;
			}
		}
		return;
	}

	if (mouseMode != xtpReportMouseNothing)
	{
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
		m_pControl->SetMouseMode(xtpReportMouseNothing);

		return;
	}
}

void CXTPReportHeader::StartDragging(CXTPReportColumn* pColumn, BOOL bHeader)
{
	ASSERT(pColumn);
	m_pDragColumn = pColumn;
	m_bDragHeader = bHeader;
	m_bDragGroupBox = !bHeader;
	m_nDropIndex = -1;
}

CXTPReportColumn* CXTPReportHeader::GetDraggingColumn() const
{
	return m_pDragColumn;
}

CXTPReportColumn* CXTPReportHeader::GetResizingColumn() const
{
	return m_pResizeColumn;
}

BOOL CXTPReportHeader::SetSubListCtrl(CXTPReportSubListControl* pSubList)
{
	m_pSubList = pSubList;

	if (!pSubList)
		return FALSE;

	m_pSubList->SetReportCtrl(m_pControl);

	return TRUE;
}

BOOL CXTPReportHeader::SetFilterEditCtrl(CXTPReportFilterEditControl* pFilterEdit)
{
	if (!pFilterEdit)
		return FALSE;

	m_pFilterEdit = pFilterEdit;
	m_pFilterEdit->SetReportCtrl(m_pControl);

	return TRUE;
}

void CXTPReportHeader::OnContextMenu(CPoint ptClick)
{
	CPoint ptClient(ptClick);
	if (!m_pControl || !m_pControl->m_rcHeaderArea.PtInRect(ptClient))
		return;

	if (m_pControl->GetMouseMode() == xtpReportMouseNothing)
	{
		// mark the column as clicked at by the dragging
		CXTPReportColumn* pColumn = HitTest(ptClient);
		if (pColumn || s_bSendContextMenuForWholeHeaderArea)
		{
			m_pControl->ClientToScreen(&ptClick);
			// send process notification to the user and wait for the reaction
			m_pControl->SendMessageToParent(NULL, NULL, pColumn, XTP_NM_REPORT_HEADER_RCLICK, &ptClick);

			m_pControl->RedrawControl();
		}
	}
}

void CXTPReportHeader::DrawFooter(CDC* pDC, CRect& rcFooter, int nLeftOffset)
{
	UNREFERENCED_PARAMETER(nLeftOffset);
	// draw background
	GetPaintManager()->FillFooter(pDC, rcFooter);

	// draw items
	int nVisColCount = m_pColumns->GetVisibleColumnsCount();
	int nFreezeCols = min(GetControl()->GetFreezeColumnsCount(), nVisColCount);

	for (int nColumn = nVisColCount-1; nColumn >= 0; nColumn--)
	{
		BOOL bFreezeCol = nColumn < nFreezeCols;
		int nColIdx = bFreezeCol ? nFreezeCols - 1 - nColumn : nColumn;

		CXTPReportColumn* pColumn = m_pColumns->GetVisibleAt(nColIdx);
		ASSERT(pColumn && pColumn->IsVisible());
		if (pColumn)
		{
			CRect rcItem = pColumn->GetRect();
			rcItem.top = rcFooter.top;
			rcItem.bottom = rcFooter.bottom;

			if (rcItem.Height() > 0)
			{
				if (bFreezeCol)
				{
					GetPaintManager()->FillFooter(pDC, rcItem);
				}
				GetPaintManager()->DrawColumnFooter(pDC, pColumn, this, rcItem);
			}
		}
	}
}

void CXTPReportHeader::DrawGroupByControl(CDC* pDC, CRect& rcGroupBy)
{
	m_rcGroupBy = rcGroupBy;

	if (rcGroupBy.Height() <= 0)
		return;

	CXTPReportPaintManager* pPM = GetPaintManager();
	if (pPM == NULL) return;

	pPM->FillGroupByControl(pDC, rcGroupBy);

	CXTPFontDC font(pDC, &pPM->m_fontCaption);

	int x = m_rcGroupBy.left + 9;
	int y = m_rcGroupBy.top + 7;

	int nHeaderHeight = pPM->GetHeaderHeight();

	// draw items
	int nColumnsCount = m_pColumns->GetGroupsOrder()->GetCount();
	for (int nColumn = 0; nColumn < nColumnsCount; nColumn++)
	{
		CXTPReportColumn* pColumn = m_pColumns->GetGroupsOrder()->GetAt(nColumn);

		if (pColumn /*&& pColumn->IsVisible()*/)
		{
			CRect rcItem(x, y, x + pColumn->GetCaptionWidth(pDC) + 50, y + nHeaderHeight - 3);
			pColumn->m_rcGroupBy = rcItem;

			// draw background
			pPM->FillHeaderControl(pDC, rcItem);
			// draw column
			pPM->DrawColumn(pDC, pColumn, this, rcItem, TRUE);
			// next column
			x = rcItem.right + 5;
			y = rcItem.top + rcItem.Height() / 2;
			// draw connector
			if (nColumn < nColumnsCount - 1)
				pPM->DrawConnector(pDC, CPoint(rcItem.right - 5, rcItem.bottom), CPoint(x, rcItem.bottom + 4));
		}
	}

// draw default dragging text if no items there
	if (nColumnsCount == 0)
	{
		CRect rcItem(x, y, x, y + nHeaderHeight - 4);
		pPM->DrawNoGroupByText(pDC, rcItem);
	}
}

int CXTPReportHeader::GetGroupByHeight() const
{
	CXTPReportPaintManager* pPaintManager = GetPaintManager();

	int nColumnsCount = m_pColumns->GetGroupsOrder()->GetCount();
	int nHeaderHeight = pPaintManager->GetHeaderHeight();
	int nHeight = 15 + ((nHeaderHeight - 3) / 2) * (nColumnsCount + 1);
	if (nColumnsCount == 0)
		nHeight += (nHeaderHeight - 3) / 2;
	return nHeight;
}

void CXTPReportHeader::BestFit(CXTPReportColumn* pColumn)
{
	if (pColumn->IsResizable() && m_bAllowColumnResize)
	{
		if (!m_bAutoColumnSizing
			|| (!GetControl()->m_bStrictBestFit && !IsLastResizebleColumn(pColumn) && !IsLastVisibleColumn(pColumn))
			)
		{
			int nMaxAvailWidth = GetMaxAvailWidth(pColumn);
			int nMinWidth = pColumn->GetMinWidth();

			int nMaxItemWidth = min(pColumn->GetBestFitWidth(), nMaxAvailWidth);

			if (nMaxItemWidth > nMinWidth)
			{
				ResizeColumn(pColumn, nMaxItemWidth);
				m_pControl->RedrawControl();
			}
		}
	}
}

BOOL CXTPReportHeader::IsLastVisibleColumn(CXTPReportColumn* pColumn) const
{
	int nColumnCount = m_pColumns->GetCount();

	for (int nColumn = m_pColumns->IndexOf(pColumn) + 1; nColumn < nColumnCount; nColumn++)
	{
		CXTPReportColumn* pCol = m_pColumns->GetAt(nColumn);
		if (pCol && pCol->IsVisible())
			return FALSE;
	}

	return TRUE;
}

BOOL CXTPReportHeader::IsLastResizebleColumn(CXTPReportColumn* pColumn) const
{
	int nColumnCount = m_pColumns->GetCount();

	for (int nColumn = m_pColumns->IndexOf(pColumn) + 1; nColumn < nColumnCount; nColumn++)
	{
		CXTPReportColumn* pCol = m_pColumns->GetAt(nColumn);

		if (pCol && pCol->IsVisible() && pCol->IsResizable())
			return FALSE;
	}

	return TRUE;
}

void CXTPReportHeader::ShowItemsInGroups(BOOL bShowInGroups)
{
	if (m_bShowItemsInGroups == bShowInGroups)
		return;

	m_bShowItemsInGroups = bShowInGroups;

	if (m_bShowItemsInGroups)
	{
		int nColumnCount = m_pColumns->GetGroupsOrder()->GetCount();

		for (int nColumn = 0; nColumn < nColumnCount; nColumn++)
		{
			CXTPReportColumn* pCol = m_pColumns->GetGroupsOrder()->GetAt(nColumn);

			if (pCol && !pCol->IsVisible())
				pCol->SetVisible(TRUE);
		}

		m_pColumns->GetGroupsOrder()->Clear();
		if (m_pColumns->GetSortOrder()->GetCount() > 0)
		{
		  m_pColumns->GetGroupsOrder()->Add(m_pColumns->GetSortOrder()->GetAt(0));
		  m_pColumns->GetSortOrder()->Clear();
		}
		m_pControl->Populate();
		OnColumnsChanged(xtpReportColumnGroupOrderChanged, NULL);
	}
}

CXTPReportColumn* CXTPReportHeader::GetNextVisibleColumn(int nIndex, int nDirection)
{
	if (nDirection == 1)
	{
		int nColumnCount = m_pColumns->GetCount();

		for (int nColumn = nIndex + 1; nColumn < nColumnCount; nColumn++)
		{
			CXTPReportColumn* pCol = m_pColumns->GetAt(nColumn);

			if (pCol && pCol->IsVisible())
				return pCol;
		}
	}
	if (nDirection == -1)
	{
		for (int nColumn = nIndex - 1; nColumn >= 0; nColumn--)
		{
			CXTPReportColumn* pCol = m_pColumns->GetAt(nColumn);

			if (pCol && pCol->IsVisible())
				return pCol;
		}
	}

	return NULL;
}

INT_PTR CXTPReportHeader::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	CXTPReportColumn* pColumn = HitTest(point);
	if (!pColumn)
		return -1;

	INT_PTR nHit = (INT_PTR) pColumn;

	CString strTip = pColumn->GetTooltip();
	CXTPReportPaintManager* pPaintManager = GetPaintManager();

	if (strTip.IsEmpty())
	{
		strTip = pColumn->GetCaption();

		if (pColumn->IsSortable() && m_bAllowColumnSort && pPaintManager != NULL)
		{
			if (m_pControl->m_iIconViewColumn > -1
				&& pColumn->GetColumns()->Find(m_pControl->m_iIconViewColumn) == pColumn)
			{
				if (pPaintManager->m_bRecOrRowNum)// flag for Record (TRUE) or Row (FALSE) number
					strTip = _T("Rec") + strTip;
				else
					strTip = _T("Row") + strTip;
			}
			else
			{
				if (pColumn->HasSortTriangle())
				{
					strTip = pPaintManager->m_strSortBy + strTip;
					if (pColumn->IsSortedDecreasing())
						strTip += pPaintManager->m_strDecreasing;
					else
						strTip += pPaintManager->m_strIncreasing;
				}
			}
		}
		if (pColumn->GetMarkupUIElement())
			return -1;
	}
	BOOL bAction(FALSE);
	if (pColumn->IsPlusMinus())
	{
		CRect rcPlusMinus = pColumn->GetRect();
		bAction = (rcPlusMinus.right - point.x <= 15); //click on Plus-Minus glyph only [13 + 2 pixels]
	}
	if (bAction && pPaintManager != NULL)
	{
		if (pColumn->IsExpanded())
			strTip.Format(_T("%s"), pPaintManager->m_strExpand);
		else
			strTip.Format(_T("%s"), pPaintManager->m_strCollapse);
	}

	if (strTip.GetLength() == 0)
		return -1;

	if (pPaintManager != NULL && pPaintManager->m_bCleanTooltip)
	{
		//Clean markup
		CString s(strTip), u, v;
		int j = s.Find(_T("<"));
		int k = s.Find(_T(">"));
		while (j > -1 && k > -1)
		{
			u = s.Left(j);
			v = s.Mid(k + 1);
			s = u + v;
			strTip = s;
			j = s.Find(_T("<"));
			k = s.Find(_T(">"));
		}
	}
	CXTPToolTipContext::FillInToolInfo(pTI, m_pControl->m_hWnd, pColumn->GetRect(), nHit, strTip);
	return nHit;
}

void CXTPReportHeader::SetLastColumnExpand(BOOL bLastColumnExpand, BOOL bLastColumnExpandKeep)
{
	if (m_pColumns != NULL &&
		m_pColumns->GetLastVisibleColumn() != NULL)
	{
		SetAutoColumnSizing(FALSE);

		m_bLastColumnExpand = bLastColumnExpand;
		m_bLastColumnExpandKeep = bLastColumnExpandKeep;
		if (bLastColumnExpand)
		{
			int nColumnsCount = m_pColumns->GetCount(), nColumn;
			for (nColumn = 0; nColumn < nColumnsCount; nColumn++)
			{
				CXTPReportColumn* pColumn = m_pColumns->GetAt(nColumn);
				if (pColumn && pColumn->IsVisible())
				{
					pColumn->SetAutoSize(FALSE);
				}
			}
			m_pColumns->GetLastVisibleColumn()->SetAutoSize(TRUE);
		}

		SetAutoColumnSizing(TRUE);
	}
}

void CXTPReportHeader::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Bool(pPX, _T("AllowColumnRemove"), m_bAllowColumnRemove, TRUE);
	PX_Bool(pPX, _T("AllowColumnResize"), m_bAllowColumnResize, TRUE);
	PX_Bool(pPX, _T("AllowColumnReorder"), m_bAllowColumnReorder, TRUE);
	PX_Bool(pPX, _T("AllowColumnSort"), m_bAllowColumnSort, TRUE);
	PX_Bool(pPX, _T("AutoColumnSizing"), m_bAutoColumnSizing, TRUE);
	PX_Bool(pPX, _T("ShowItemsInGroups"), m_bShowItemsInGroups, s_bShowItemsInGroupsPXDefault);
}

BEGIN_INTERFACE_MAP(CXTPReportHeader, CCmdTarget)
	INTERFACE_PART(CXTPReportHeader, IID_IAccessible, ExternalAccessible)
END_INTERFACE_MAP()

CCmdTarget* CXTPReportHeader::GetAccessible()
{
	return this;
}

HRESULT CXTPReportHeader::GetAccessibleParent(IDispatch* FAR* ppdispParent)
{
	SAFE_MANAGE_STATE(m_pModuleState);

	*ppdispParent = m_pControl->GetIDispatch(TRUE);
	return S_OK;
}

HRESULT CXTPReportHeader::GetAccessibleChildCount(long FAR* pChildCount)
{
	if (pChildCount == 0)
	{
		return E_INVALIDARG;
	}

	*pChildCount = m_pColumns->GetCount();
	return S_OK;
}

HRESULT CXTPReportHeader::GetAccessibleChild(VARIANT varChild, IDispatch* FAR* ppdispChild)
{
	*ppdispChild = NULL;
	int nChild = GetChildIndex(&varChild);

	if (nChild <= 0)
	{
		return E_INVALIDARG;
	}

	*ppdispChild = NULL;
	return S_FALSE;
}

HRESULT CXTPReportHeader::GetAccessibleName(VARIANT varChild, BSTR* pszName)
{
	int nIndex = GetChildIndex(&varChild);

	if (nIndex == CHILDID_SELF)
	{
		CString strCaption = _T("Report Header");
		*pszName = strCaption.AllocSysString();
		return S_OK;
	}

	CXTPReportColumn* pColumn = m_pColumns->GetAt(nIndex - 1);
	if (!pColumn)
		return E_INVALIDARG;

	CString strCaption = pColumn->GetCaption();
	*pszName = strCaption.AllocSysString();
	return S_OK;
}

HRESULT CXTPReportHeader::GetAccessibleRole(VARIANT /*varChild*/, VARIANT* pvarRole)
{
	pvarRole->vt = VT_I4;
	pvarRole->lVal = ROLE_SYSTEM_COLUMNHEADER;
	return S_OK;
}

HRESULT CXTPReportHeader::GetAccessibleState(VARIANT varChild, VARIANT* pvarState)
{
	pvarState->vt = VT_I4;
	pvarState->lVal = 0;
	int nChild = GetChildIndex(&varChild);

	if (nChild == CHILDID_SELF)
	{
		pvarState->lVal = 0;
	}
	else
	{
		CXTPReportColumn* pColumn = m_pColumns->GetAt(nChild - 1);
		if (pColumn)
		{
			if (!pColumn->IsVisible())
				pvarState->lVal |= STATE_SYSTEM_INVISIBLE;

		}

	}

	return S_OK;
}

HRESULT CXTPReportHeader::AccessibleLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild)
{
	*pxLeft = *pyTop = *pcxWidth = *pcyHeight = 0;

	CRect rc(0, 0, 0, 0);

	int nChild = GetChildIndex(&varChild);

	if (nChild == CHILDID_SELF)
	{
		rc = m_pControl->m_rcHeaderArea;
		m_pControl->ClientToScreen(&rc);
	}
	else
	{
		CXTPReportColumn* pColumn = m_pColumns->GetAt(nChild - 1);
		if (pColumn)
		{
			rc = pColumn->GetRect();
			m_pControl->ClientToScreen(&rc);
		}
	}

	*pxLeft = rc.left;
	*pyTop = rc.top;
	*pcxWidth = rc.Width();
	*pcyHeight = rc.Height();

	return S_OK;
}

HRESULT CXTPReportHeader::AccessibleHitTest(long xLeft, long yTop, VARIANT* pvarID)
{
	if (pvarID == NULL)
		return E_INVALIDARG;

	pvarID->vt = VT_I4;
	pvarID->lVal = CHILDID_SELF;

	CPoint pt(xLeft, yTop);
	m_pControl->ScreenToClient(&pt);

	if (!m_pControl->m_rcHeaderArea.PtInRect(pt))
		return S_FALSE;


	CXTPReportColumn* pColumn = HitTest(pt);
	if (pColumn)
	{
		pvarID->lVal = pColumn->GetIndex() + 1;
		return S_OK;
	}

	return S_OK;
}
