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
// BCGPGanttControl.cpp: implementation of the CBCGPGanttControl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPGanttControl.h"
#include "BCGPGanttItemStorage.h"

#ifndef _BCGSUITE_
#include "BCGPRegistry.h"
#include "BCGPWorkspace.h"
extern CBCGPWorkspace*		g_pWorkspace;
#endif

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

const UINT BCGM_GANTT_CONTROL_READ_ITEM_DATA_FROM_GRID = ::RegisterWindowMessage (_T("BCGM_GANTT_CTRL_READITEMFROMGRID"));
const UINT BCGM_GANTT_CONTROL_WRITE_ITEM_DATA_TO_GRID = ::RegisterWindowMessage (_T("BCGM_GANTT_CTRL_WRITEITEMTOGRID"));
const UINT BCGM_GANTT_CONTROL_CREATE_CHART = ::RegisterWindowMessage (_T("BCGM_GANTT_CONTROL_CREATE_CHART"));
const UINT BCGM_GANTT_CONTROL_CREATE_GRID = ::RegisterWindowMessage (_T("BCGM_GANTT_CONTROL_CREATE_GRID"));

enum
{
	eStateNormal,
	eStateProcessingGridNotification,
	eStateProcessingChartNotification
};

IMPLEMENT_DYNCREATE (CBCGPGanttControl, CBCGPWnd)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPGanttControl::CBCGPGanttControl()
	: m_nState (eStateNormal)
	, m_nItemNameColumn (-1)
	, m_nItemStartDateColumn (-1)
	, m_nItemFinishDateColumn (-1)
	, m_nItemProgressColumn (-1)
	, m_pGrid (NULL)
	, m_pChart (NULL)
	, m_pParentWnd (NULL)
	, m_bLoadLastSavedTime(FALSE)
{
}
//*****************************************************************************
CBCGPGanttControl::~CBCGPGanttControl()
{
	if (m_pGrid != NULL)
	{
		delete m_pGrid;
	}

	if (m_pChart != NULL)
	{
		delete m_pChart;
	}
}
//*****************************************************************************
static CString GetSection (LPCTSTR lpszProfileName, int nIndex)
{
	static const CString s_strProfile = _T("BCGPGanttControls");

	CString strProfileName;

#ifndef _BCGSUITE_
	if (g_pWorkspace == NULL ||
		AfxGetApp () == NULL || AfxGetApp ()->m_pszRegistryKey == NULL)
	{
		return CString ();
	}

	strProfileName = g_pWorkspace->GetRegSectionPath (s_strProfile);
#else
	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp ());
	if (pApp == NULL || pApp->m_pszRegistryKey == NULL)
	{
		return FALSE;
	}

	strProfileName = pApp->GetRegSectionPath (s_strProfile);
#endif

	if (lpszProfileName != NULL)
	{
		strProfileName += lpszProfileName;
	}

	CString strSection;
	strSection.Format (_T("%sBCGPGantt-%d"), strProfileName, nIndex);

	return strSection;
}
//*****************************************************************************
BOOL CBCGPGanttControl::SaveState (LPCTSTR lpszProfileName, int nIndex)
{
	if (m_pChart == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(m_pChart);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (!reg.CreateKey (GetSection (lpszProfileName, nIndex)))
	{
		return FALSE;
	}

	// Splitter

	int iSplitterPos = 0, iUnused;
	m_wndSplitter.GetColumnInfo (0, iSplitterPos, iUnused);
	reg.Write (_T("SplitterPos"), iSplitterPos);

	// Chart
#ifndef _BCGSUITE_
	reg.Write (_T("ChartTimeDelta"), (double)m_pChart->m_dtPixelTimeDelta);
	reg.Write (_T("ChartLeftMostTime"), (double)m_pChart->m_dtLeftMostChartTime); 
#endif

	reg.Write (_T("ChartShowHeader0"), m_pChart->GetHeaderVisible (0));
	reg.Write (_T("ChartShowHeader1"), m_pChart->GetHeaderVisible (1));
	reg.Write (_T("ChartShowEmptyRows"), m_pChart->GetShowEmptyRows ());
	reg.Write (_T("ChartShowTooltip"), m_pChart->IsShowToolTip ());
	reg.Write (_T("ChartColorScheme"), (DWORD)m_pChart->GetColorScheme ());

	// Grid

	reg.Write (_T("GridNameColumnIndex"), m_nItemNameColumn);
	reg.Write (_T("GridStartColumnIndex"), m_nItemStartDateColumn);
	reg.Write (_T("GridFinishColumnIndex"), m_nItemFinishDateColumn);
	reg.Write (_T("GridProgressColumnIndex"), m_nItemProgressColumn);

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPGanttControl::LoadState (LPCTSTR lpszProfileName, int nIndex)
{
	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (GetSection (lpszProfileName, nIndex)))
	{
		return FALSE;
	}

	// Splitter

	int iSplitterPos = 0;
	if (reg.Read (_T("SplitterPos"), iSplitterPos))
	{
        if (iSplitterPos > 0)
        {
            m_wndSplitter.SetColumnInfo (0, iSplitterPos, 30);
            m_wndSplitter.RecalcLayout();
        }
	}

	// Chart
#ifndef _BCGSUITE_
	double dTimeDelta = 0.;     // zoom level
	if (reg.Read (_T("ChartTimeDelta"), dTimeDelta) && m_pChart != NULL)
	{
		m_pChart->SetTimeDelta ((COleDateTimeSpan)dTimeDelta);
	}

	if (m_bLoadLastSavedTime)
	{
		double dLeftMostDate;  // horizontal scroll position
		if (reg.Read (_T("ChartLeftMostTime"), dLeftMostDate) && m_pChart != NULL)
		{
			m_pChart->SetLeftmostDateTime ((COleDateTime)dLeftMostDate);
		}
	}
#endif

	BOOL bValue = FALSE;

	if (reg.Read (_T("ChartShowHeader0"), bValue) && m_pChart != NULL)
	{
		m_pChart->SetHeaderVisible (0, bValue);
	}

	if (reg.Read (_T("ChartShowHeader1"), bValue) && m_pChart != NULL)
	{
		m_pChart->SetHeaderVisible (1, bValue);
	}

	if (reg.Read (_T("ChartShowEmptyRows"), bValue) && m_pChart != NULL)
	{
		m_pChart->SetShowEmptyRows (bValue);
	}

	if (reg.Read (_T("ChartShowTooltip"), bValue) && m_pChart != NULL)
	{
		m_pChart->SetShowToolTip (bValue);
	}
	
	DWORD dwColor = 0;

	if (reg.Read (_T("ChartColorScheme"), dwColor) && m_pChart != NULL)
	{
		m_pChart->SetColorScheme ((COLORREF)dwColor);
	}

	// Grid

	reg.Read (_T("GridNameColumnIndex"), m_nItemNameColumn);
	reg.Read (_T("GridStartColumnIndex"), m_nItemStartDateColumn);
	reg.Read (_T("GridFinishColumnIndex"), m_nItemFinishDateColumn);
	reg.Read (_T("GridProgressColumnIndex"), m_nItemProgressColumn);

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPGanttControl::Create (DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	m_pParentWnd = pParentWnd;
	m_pGrid = CreateGrid ();
	m_pChart = CreateChart ();
	BOOL bResult = CWnd::Create (globalData.RegisterWindowClass (_T("BCGPGanttControl")), _T(""), dwStyle, rect, pParentWnd, nID);

	return bResult && m_pGrid != NULL && m_pChart != NULL;
}
//*****************************************************************************
const CBCGPSplitterWnd* CBCGPGanttControl::GetSplitter () const
{
	return &m_wndSplitter;
}
//*****************************************************************************
const CBCGPGanttGrid* CBCGPGanttControl::GetGrid () const
{
	return m_pGrid;
}
//*****************************************************************************
const CBCGPGanttChart* CBCGPGanttControl::GetChart () const
{
	return m_pChart;
}
//*****************************************************************************
CBCGPSplitterWnd* CBCGPGanttControl::GetSplitter ()
{
	return &m_wndSplitter;
}
//*****************************************************************************
CBCGPGanttGrid* CBCGPGanttControl::GetGrid ()
{
	return m_pGrid;
}
//*****************************************************************************
CBCGPGanttChart* CBCGPGanttControl::GetChart ()
{
	return m_pChart;
}
//*****************************************************************************
void CBCGPGanttControl::SetNameColumnIndex (int nColumn)
{
	if (nColumn == m_nItemNameColumn)
	{
		return;
	}

	m_nItemNameColumn = nColumn;

	if (m_nItemStartDateColumn == nColumn)
	{
		m_nItemStartDateColumn = -1;
	}

	if (m_nItemFinishDateColumn == nColumn)
	{
		m_nItemFinishDateColumn = -1;
	}

	if (m_nItemProgressColumn == nColumn)
	{
		m_nItemProgressColumn = -1;
	}

	if (nColumn >= 0)
	{
		m_pGrid->SetItemRTC (nColumn, RUNTIME_CLASS (CBCGPGridItem));
		UpdateGrid ();
	}
}
//*****************************************************************************
void CBCGPGanttControl::SetStartDateColumnIndex (int nColumn)
{
	if (nColumn == m_nItemStartDateColumn)
	{
		return;
	}

	m_nItemStartDateColumn = nColumn;

	if (m_nItemNameColumn == nColumn)
	{
		m_nItemNameColumn = -1;
	}

	if (m_nItemFinishDateColumn == nColumn)
	{
		m_nItemFinishDateColumn = -1;
	}

	if (m_nItemProgressColumn == nColumn)
	{
		m_nItemProgressColumn = -1;
	}

	if (nColumn >= 0)
	{
		m_pGrid->SetItemRTC (nColumn, RUNTIME_CLASS (CBCGPGridDateTimeItem));
		UpdateGrid ();
	}
}
//*****************************************************************************
void CBCGPGanttControl::SetFinishDateColumnIndex (int nColumn)
{
	if (nColumn == m_nItemFinishDateColumn)
	{
		return;
	}

	m_nItemFinishDateColumn = nColumn;

	if (m_nItemNameColumn == nColumn)
	{
		m_nItemNameColumn = -1;
	}

	if (m_nItemStartDateColumn == nColumn)
	{
		m_nItemStartDateColumn = -1;
	}

	if (m_nItemProgressColumn == nColumn)
	{
		m_nItemProgressColumn = -1;
	}

	if (nColumn >= 0)
	{
		m_pGrid->SetItemRTC (nColumn, RUNTIME_CLASS (CBCGPGridDateTimeItem));
		UpdateGrid ();
	}
}
//*****************************************************************************
void CBCGPGanttControl::SetProgressColumnIndex (int nColumn)
{
	if (nColumn == m_nItemProgressColumn)
	{
		return;
	}

	m_nItemProgressColumn = nColumn;

	if (m_nItemNameColumn == nColumn)
	{
		m_nItemNameColumn = -1;
	}

	if (m_nItemStartDateColumn == nColumn)
	{
		m_nItemStartDateColumn = -1;
	}

	if (m_nItemFinishDateColumn == nColumn)
	{
		m_nItemFinishDateColumn = -1;
	}

	if (nColumn >= 0)
	{
		m_pGrid->SetItemRTC (nColumn, RUNTIME_CLASS (CBCGPGridPercentItem));
		UpdateGrid ();
	}
}
//*****************************************************************************
CBCGPGanttItem* CBCGPGanttControl::ItemFromRow (int nRow) const
{
	if (nRow < 0)
	{
		return NULL;
	}

	return m_pChart->GetStorage ()->GetItem (nRow);
}
//*****************************************************************************
int CBCGPGanttControl::RowFromItem (CBCGPGanttItem* pItem) const
{
	return m_pChart->GetStorage ()->IndexOf (pItem);
}
//*****************************************************************************
BOOL CBCGPGanttControl::IsProcessingGridNotification () const
{
	return m_nState == eStateProcessingGridNotification;
}
//*****************************************************************************
BOOL CBCGPGanttControl::IsProcessingChartNotification () const
{
	return m_nState == eStateProcessingChartNotification;
}
//*****************************************************************************
BOOL CBCGPGanttControl::StartProcessNotification (CWnd* pSender)
{
	if (m_pGrid->GetSafeHwnd () == NULL || m_pChart->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	if (pSender == DYNAMIC_DOWNCAST(CWnd, GetChart ()))
	{
		if (m_nState != eStateProcessingGridNotification)
		{
			m_nState = eStateProcessingChartNotification;
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else if (pSender == DYNAMIC_DOWNCAST(CWnd, GetGrid ()))
	{
		if (m_nState != eStateProcessingChartNotification)
		{
			m_nState = eStateProcessingGridNotification;
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}
//*****************************************************************************
void CBCGPGanttControl::FinishProcessNotification ()
{
	m_nState = eStateNormal;
}
//*****************************************************************************
void CBCGPGanttControl::DoVerticalResize (CWnd* pSender)
{
	if (!StartProcessNotification (pSender))
	{
		return;
	}

	int nFooter = 0;

	if (m_nState == eStateProcessingChartNotification) // Chart -> Grid
	{
		CRect rectClient;
		m_pChart->GetClientRect (&rectClient);
		CRect rectChart = m_pChart->GetVisibleChartArea ();

		m_pGrid->SetVerticalSizes (rectChart.top - rectClient.top, m_pChart->GetDefaultItemHeight ());
	}
	if (m_nState == eStateProcessingGridNotification) // Grid -> Chart
	{
		UINT nHeader, nRows;
		m_pGrid->GetVerticalSizes (&nHeader, &nRows);

		m_pChart->SetDefaultItemHeight (nRows);

		UINT nChartHeader0_hgt, nChartHeader1_hgt;
		m_pChart->GetHeaderHeights (&nChartHeader0_hgt, &nChartHeader1_hgt);

		if (nHeader > 0 && nFooter > 0) // both header and footer
		{
			m_pChart->SetHeaderHeights (nHeader, nFooter);
			m_pChart->SetHeaderVisible (0, TRUE);
			m_pChart->SetHeaderVisible (1, TRUE);
			m_pChart->SetHeaderAboveChart (0, TRUE);
			m_pChart->SetHeaderAboveChart (1, FALSE);
		}
		else if (nHeader > 0 || nFooter > 0) // either header or footer
		{
			BOOL bH0Visible = m_pChart->GetHeaderVisible (0);
			BOOL bH1Visible = m_pChart->GetHeaderVisible (1);
			int h = (nHeader > 0) ? nHeader : nFooter;

			if (bH0Visible && bH1Visible)
			{
				m_pChart->SetHeaderHeights (h / 2, h - (h / 2));
				m_pChart->SetHeaderAboveChart (0, (nHeader > 0));
				m_pChart->SetHeaderAboveChart (1, (nHeader > 0));
			}
			else if (bH0Visible)
			{
				m_pChart->SetHeaderHeights (h, nChartHeader1_hgt);
				m_pChart->SetHeaderAboveChart (0, (nHeader > 0));
			}
			else if (bH1Visible)
			{
				m_pChart->SetHeaderHeights (nChartHeader0_hgt, h);
				m_pChart->SetHeaderAboveChart (1, (nHeader > 0));
			}
			else
			{
				m_pChart->SetHeaderHeights (h, nChartHeader1_hgt);
				m_pChart->SetHeaderAboveChart (0, (nHeader > 0));
			}
		}
		else // nor header neither footer
		{
			m_pChart->SetHeaderVisible (0, FALSE);
			m_pChart->SetHeaderVisible (1, FALSE);
		}
	}

	FinishProcessNotification ();
}
//*****************************************************************************
void CBCGPGanttControl::DoItemShow (CWnd* pSender, CBCGPGanttItem* pItem, BOOL bVisible)
{
	if (!StartProcessNotification (pSender))
	{
		return;
	}

	if (m_nState == eStateProcessingGridNotification) // Grid -> Chart
	{
		pItem->Show (bVisible);
	}

	FinishProcessNotification ();
}
//*****************************************************************************
void CBCGPGanttControl::ExpandGroup (CBCGPGridRow* pParentRow)
{
	ASSERT_VALID (pParentRow);

	if (pParentRow == NULL)
	{
		return;
	}

	CList<CBCGPGridRow*, CBCGPGridRow*> rows;
	pParentRow->GetSubItems (rows, FALSE);

	POSITION pos = rows.GetHeadPosition ();
	while (pos != NULL)
	{
		CBCGPGridRow* pRow = rows.GetNext (pos);
		if (pRow != NULL)
		{
			CBCGPGanttItem* pItem = ItemFromRow (pRow->GetRowId ());

			ASSERT (pItem != NULL);
			if (pItem != NULL)
			{
				pItem->Show (TRUE);
			}

			if (pRow->IsGroup () && pRow->IsExpanded ())
			{
				ExpandGroup (pRow); //recursive call
			}
		}
	}
}
//*****************************************************************************
void CBCGPGanttControl::CollapseGroup (CBCGPGridRow* pParentRow)
{
	CList<CBCGPGridRow*, CBCGPGridRow*> rows;
	pParentRow->GetSubItems (rows, TRUE); // recursive

	POSITION pos = rows.GetHeadPosition ();
	while (pos != NULL)
	{
		CBCGPGridRow* pRow = rows.GetNext (pos);
		if (pRow != NULL)
		{
			CBCGPGanttItem* pItem = ItemFromRow (pRow->GetRowId ());

			ASSERT (pItem != NULL);
			if (pItem != NULL)
			{
				pItem->Show (FALSE);
			}
		}
	}
}
//*****************************************************************************
void CBCGPGanttControl::DoExpandGroup (CWnd* pSender, CBCGPGridRow* pRow, BOOL bExpand)
{
	if (!StartProcessNotification (pSender))
	{
		return;
	}

	if (m_nState == eStateProcessingGridNotification) // Grid -> Chart only supported
	{
		if (!bExpand)
		{
			CollapseGroup (pRow);
		}
		else
		{
			ExpandGroup (pRow);
		}
	}

	FinishProcessNotification ();
}
//*****************************************************************************
void CBCGPGanttControl::DoVerticalScroll (CWnd* pSender, int /*nVerticalOffset*/)
{
	if (!StartProcessNotification (pSender))
	{
		return;
	}

	if (m_nState == eStateProcessingChartNotification) // Chart -> Grid
	{
		int nPos = m_pChart->GetVScrollPos ();
		m_pGrid->SetVScrollPos (nPos);
	}

	if (m_nState == eStateProcessingGridNotification) // Grid -> Chart
	{
		int nPos = m_pGrid->GetVScrollPos ();
		m_pChart->SetVScrollPos (nPos);
	}

	FinishProcessNotification ();
}
//*****************************************************************************
void CBCGPGanttControl::DoSelectionChanged (CWnd* pSender, CBCGPGanttItem* pItem, BOOL bSelected)
{
	if (!StartProcessNotification (pSender))
	{
		return;
	}

	if (m_nState == eStateProcessingGridNotification) // Grid -> Chart
	{
		pItem->Select (bSelected);
	}

	if (m_nState == eStateProcessingChartNotification) // Chart -> Grid
	{
	}

	FinishProcessNotification ();
}

//*****************************************************************************
// The grid control calls this virtual function when the user has changed data
void CBCGPGanttControl::OnGridItemChanged (CBCGPGridItem* pGridItem, int nRow, int nColumn)
{
	if (nRow < 0 || nColumn < 0 || pGridItem == NULL)
	{
		return;
	}

	CBCGPGanttItem* pItem = ItemFromRow (nRow);

	if (pItem != NULL)
	{
		ASSERT_VALID (pItem);

		_variant_t v = pGridItem->GetValue ();
		bool bIsDate = v.vt == VT_DATE;

		if (nColumn == m_nItemNameColumn)
		{
			CString sName = (LPCTSTR)(_bstr_t)pGridItem->GetValue ();
			pItem->SetName (sName);
		}
		else if (nColumn == m_nItemStartDateColumn && bIsDate)
		{
			COleDateTime dt = COleDateTime((double)v);
			pItem->SetInterval (dt, pItem->GetFinishTime ());
		}
		else if (nColumn == m_nItemFinishDateColumn && bIsDate)
		{
			COleDateTime dt = COleDateTime((double)v);
			pItem->SetInterval (pItem->GetStartTime (), dt);
		}
		else if (nColumn == m_nItemProgressColumn)
		{
			double dProgress = (double)v;
			pItem->SetProgress (dProgress);
		}

		CWnd* pParent = GetParent ();
		if (pParent->GetSafeHwnd () != NULL)
		{
			BCGP_GANTT_CONTROL_ROW rowInfo;
			rowInfo.pGridRow = pGridItem->GetParentRow ();
			rowInfo.pItem = pItem;
			pParent->SendMessage (BCGM_GANTT_CONTROL_READ_ITEM_DATA_FROM_GRID, 0, (LPARAM)&rowInfo);
		}
	}
}
//*****************************************************************************
void CBCGPGanttControl::UpdateGridItem (CBCGPGanttItem* pItem, CBCGPGridRow* pGridRow)
{
	ASSERT_VALID (pGridRow);

	int nColumns = m_pGrid->GetColumnCount ();

	if (m_nItemNameColumn >= 0 && m_nItemNameColumn < nColumns)
	{
		CBCGPGridItem* pGridItem = pGridRow->GetItem (m_nItemNameColumn);
		if (pGridItem != NULL)
		{
			pGridItem->SetValue ((LPCTSTR)pItem->GetName ());
		}
	}

	if (m_nItemStartDateColumn >= 0 && m_nItemStartDateColumn < nColumns)
	{
		CBCGPGridItem* pGridItem = pGridRow->GetItem (m_nItemStartDateColumn);
		if (pGridItem != NULL)
		{
			pGridItem->SetValue (_variant_t((double)pItem->GetStartTime (), VT_DATE));
		}
	}

	if (m_nItemFinishDateColumn >= 0 && m_nItemFinishDateColumn < nColumns)
	{
		CBCGPGridItem* pGridItem = pGridRow->GetItem (m_nItemFinishDateColumn);
		if (pGridItem != NULL)
		{
			pGridItem->SetValue (_variant_t((double)pItem->GetFinishTime (), VT_DATE));
		}
	}

	if (m_nItemStartDateColumn >= 0 && m_nItemStartDateColumn < nColumns)
	{
		CBCGPGridItem* pGridItem = pGridRow->GetItem (m_nItemProgressColumn);
		if (pGridItem != NULL)
		{
			pGridItem->SetValue (pItem->GetProgress ());
		}
	}

	CWnd* pParent = GetParent ();
	if (pParent->GetSafeHwnd () != NULL)
	{
		BCGP_GANTT_CONTROL_ROW rowInfo;
		rowInfo.pGridRow = pGridRow;
		rowInfo.pItem = pItem;
		pParent->SendMessage (BCGM_GANTT_CONTROL_WRITE_ITEM_DATA_TO_GRID, 0, (LPARAM)&rowInfo);
	}
}
//*****************************************************************************
struct GridGroupEntry
{
	int             iIndent;
	CBCGPGridRow*   pParentRow;
};

void CBCGPGanttControl::UpdateGrid ()
{
	m_pGrid->SetRedraw (FALSE);

	m_pGrid->RemoveAll ();

	CBCGPGanttItemStorageBase* pStorage = m_pChart->GetStorage ();

	if (pStorage == NULL)
	{
		return;
	}

	int nColumns = m_pGrid->GetColumnCount ();
	POSITION pos = pStorage->GetHeadPosition ();

	CList <GridGroupEntry, GridGroupEntry&> stack;
	int iLevel = 0;
	CBCGPGridRow* pParentRow = NULL;
	CBCGPGridRow* pPrevRow = NULL;
	CBCGPGanttItem* pPrevItem = NULL;

	while (pos != NULL)
	{
		CBCGPGanttItem* pItem = pStorage->GetNext (pos);
		ASSERT_VALID (pItem);

//		if (m_pChart->IsMulipleItemsInRowEnabled() && pItem->GetRow() >= 0)
		{
//			continue;
		}

		BOOL bBeginGroup = FALSE;

		int iItemLevel = pItem->GetHierarchyLevel ();

		if (iItemLevel > iLevel && pPrevItem != NULL && pPrevItem->IsGroupItem ()) // push
		{
			GridGroupEntry entry;
			entry.iIndent = iLevel;
			entry.pParentRow = pParentRow;
			stack.AddTail (entry);

			iLevel = iItemLevel;
			bBeginGroup = TRUE;
		}
		else if (iItemLevel < iLevel) // pop
		{
			while (iItemLevel < iLevel && !stack.IsEmpty ())
			{
				GridGroupEntry entry = stack.GetTail ();
				if (iItemLevel <= entry.iIndent)
				{
					stack.RemoveTail ();
					iLevel = entry.iIndent;
					pParentRow = entry.pParentRow;
				}
			}
		}

		pPrevItem = pItem;

		CBCGPGridRow* pRow = m_pGrid->CreateRow (nColumns);
		ASSERT_VALID (pRow);

		UpdateGridItem (pItem, pRow);

		if (bBeginGroup)
		{
			pParentRow = pPrevRow;
		}

		if (pParentRow == NULL)
		{
			m_pGrid->AddRow (pRow);       
		}
		else
		{
			pParentRow->AllowSubItems (TRUE);
			pParentRow->AddSubItem (pRow);
		}

		pPrevRow = pRow;
	}

	m_pGrid->SetRedraw (TRUE);
	m_pGrid->RedrawWindow ();
}
//*****************************************************************************
void CBCGPGanttControl::IndentItem (CBCGPGanttItem* pItem)
{
	const CBCGPGanttItemStorageBase* pStorage = m_pChart->GetStorage ();
	ASSERT_VALID (pStorage);

	POSITION pos = pStorage->GetItemPosition (pItem);
	if (pos != NULL)
	{
		CBCGPGanttItem* pPrev = pStorage->GetPrev (pos);
		if (pPrev == NULL || pPrev->GetHierarchyLevel () < pItem->GetHierarchyLevel ())
		{
			return;
		}

		pPrev->SetGroupItem (TRUE);
		pItem->SetHierarchyLevel (pPrev->GetHierarchyLevel () + 1);
	}
}
//*****************************************************************************
void CBCGPGanttControl::OutdentItem (CBCGPGanttItem* pItem)
{
	CBCGPGanttItemStorageBase* pStorage = m_pChart->GetStorage ();
	ASSERT_VALID (pStorage);

	CBCGPGanttItem* pParent = pStorage->GetParentGroupItem (pItem);
	if (pParent == NULL)
	{
		return;
	}
	
	CBCGPGanttItem* pLastInGroup = NULL;
	int nSubItems = pStorage->GetSubItemsCount (pParent, &pLastInGroup);

	if (pLastInGroup != pItem)
	{
		pStorage->Move (pStorage->IndexOf (pItem), pStorage->IndexOf (pLastInGroup) + 1);
	}

	pItem->SetHierarchyLevel (pParent->GetHierarchyLevel ());

	if (nSubItems == 1)
	{
		pParent->SetGroupItem (FALSE);
	}
}
//*****************************************************************************
BOOL CBCGPGanttControl::CanIndentItem (CBCGPGanttItem* pItem) const
{
	// Detect if the previous item has same hierarchy level

	const CBCGPGanttItemStorageBase* pStorage = m_pChart->GetStorage ();
	ASSERT_VALID (pStorage);
	
	POSITION pos = pStorage->GetItemPosition (pItem);
	if (pos != NULL)
	{
		CBCGPGanttItem* pPrev = pStorage->GetPrev (pos);
		if (pPrev && pPrev->GetHierarchyLevel () == pItem->GetHierarchyLevel ())
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*****************************************************************************
BOOL CBCGPGanttControl::CanOutdentItem (CBCGPGanttItem* pItem) const
{
	// Detect if an item has a parent

	return m_pChart->GetStorage ()->GetParentGroupItem (pItem) != NULL;
}
//*****************************************************************************

BEGIN_MESSAGE_MAP(CBCGPGanttControl, CBCGPWnd)
	//{{AFX_MSG_MAP(CBCGPGanttControl)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(BCGM_CHANGEVISUALMANAGER, OnChangeVisualManager)
	ON_REGISTERED_MESSAGE(BCGM_GANTT_STORAGE_CHANGED, OnNotifyStorageChanged)
END_MESSAGE_MAP()

LRESULT CBCGPGanttControl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// First let the MFC message map handle the message
	LRESULT lResult = CBCGPWnd::WindowProc (message, wParam, lParam);

	if (lResult == 0 && m_pChart != NULL && m_pChart->IsGanttChartNotificationMessage (message, wParam, lParam))
	{
		CWnd* pParent = GetParent ();
		if (pParent->GetSafeHwnd () != NULL)
		{
			return pParent->SendMessage (message, wParam, lParam);
		}
	}

	return lResult;
}
//*****************************************************************************
LRESULT CBCGPGanttControl::OnNotifyStorageChanged (WPARAM, LPARAM lParam)
{
	BCGP_GANTT_STORAGE_UPDATE_INFO* pUpdate = (BCGP_GANTT_STORAGE_UPDATE_INFO*)lParam;
	if (pUpdate != NULL)
	{
		DoItemsChange (GetChart (), *pUpdate);
	}
	return 0L;
}
//*****************************************************************************
void CBCGPGanttControl::DoItemsChange (CWnd* pSender, const BCGP_GANTT_STORAGE_UPDATE_INFO& updateInfo)
{
	if (!StartProcessNotification (pSender))
	{
		return;
	}

	if (m_nState == eStateProcessingChartNotification) // Chart -> Grid only supported now
	{
		// for single item updates only
		CBCGPGanttItem* pItem = updateInfo.pItem;
		CBCGPGridRow* pRow = NULL;
		int nRow = -1;

		if (updateInfo.iFirstItemIndex == updateInfo.iLastItemIndex)
		{
			nRow = updateInfo.iFirstItemIndex;
			if (nRow >= 0)
			{
				pRow = m_pGrid->GetRow (nRow);
			}
		}

		switch (updateInfo.uiAction)
		{
		case BCGP_GANTT_STORAGE_INSERT_ITEM:
			if (pItem != NULL) // inserting single item only supported
			{
//				if (!m_pChart->IsMulipleItemsInRowEnabled() || pItem->GetRow() < 0)
				{
					ASSERT_VALID (pItem);

					CBCGPGanttItemStorageBase* pStorage = m_pChart->GetStorage ();
					ASSERT_VALID (pStorage);

					CBCGPGanttItem* pParentItem = pStorage->GetParentGroupItem (pItem);
					int nParentRow = RowFromItem (pParentItem);

					pRow = m_pGrid->CreateRow (m_pGrid->GetColumnCount ());

					if (nParentRow >= 0)
					{
						CBCGPGridRow* pParentRow = m_pGrid->GetRow (nParentRow);
						ASSERT_VALID (pParentRow);
						m_pGrid->InsertRowAfter (pParentRow, nRow - nParentRow - 1, pRow);
					}
					else
					{
						m_pGrid->InsertRowAfter (NULL, nRow - 1, pRow);
					}

					UpdateGridItem (pItem, pRow);
				}
			}
			break;

		case BCGP_GANTT_STORAGE_UPDATE_ITEM:
			{
				bool bNeedUpdate = (updateInfo.dwFlags & ~BCGP_GANTT_ITEM_PROP_SELECTED) != BCGP_GANTT_ITEM_PROP_NONE;

				if (bNeedUpdate)
				{
					if (pItem != NULL)
					{ 
						if (pRow != NULL)
						{
							UpdateGridItem (pItem, pRow);
						}
					}
					else // multiple items
					{
						UpdateGrid ();
					}
				}
			}
			break;

		case BCGP_GANTT_STORAGE_REMOVED_ITEM:
			{
				int nRemovedCount = 0;
				if (pItem == NULL) // multiple items
				{
					int i = updateInfo.iLastItemIndex;
					while (i >= updateInfo.iFirstItemIndex)
					{
						nRemovedCount = m_pGrid->RemoveRow (i, FALSE);
						nRemovedCount = max(1, nRemovedCount);
						i -= nRemovedCount;
					}

					m_pGrid->AdjustLayout ();
				}
				else // single item
				{
					nRemovedCount = m_pGrid->RemoveRow (nRow, TRUE);
					ASSERT (nRemovedCount <= 1); // If it is not, the grid row had subitems that were also removed.
				}
			}
			break;
		}
	}

	FinishProcessNotification ();
}
//*****************************************************************************
int CBCGPGanttControl::CreateControls ()
{
	if (m_wndSplitter.GetSafeHwnd () != NULL)
	{
		return -1;
	}

	if (!m_wndSplitter.CreateStatic (this, 1, 2))
	{
		ASSERT(FALSE);
		return -1;
	}

	m_wndSplitter.ModifyStyle (WS_HSCROLL | WS_VSCROLL, 0);

	CRect rectDummy (0, 0, 1, 1);

	if (!m_pGrid->Create (WS_VISIBLE | WS_CHILD, rectDummy, &m_wndSplitter, m_wndSplitter.IdFromRowCol (0, 0)))
	{
		TRACE0("Failed to create grid control\n");
		ASSERT(FALSE);
		return -1;
	}

	if (!m_pChart->Create (WS_VISIBLE | WS_CHILD, rectDummy, &m_wndSplitter, m_wndSplitter.IdFromRowCol (0, 1)))
	{
		TRACE0("Failed to create chart control\n");
		ASSERT(FALSE);
		return -1;
	}

	CRect rect;
	GetClientRect (rect);

	m_wndSplitter.SetColumnInfo (0, rect.Width () / 3, 30);
	m_wndSplitter.SetWindowPos (NULL, 0, 0, rect.Width (), rect.Height (), SWP_NOZORDER | SWP_NOREDRAW);

	m_pGrid->SetFont (NULL);
	m_pGrid->SetRowMarker (FALSE);

	m_pChart->SetHeaderHeights (22, 20);

	UpdateGrid ();

	return 0;
}
//*****************************************************************************
CBCGPGanttGrid* CBCGPGanttControl::CreateGrid ()
{
	CBCGPGanttGrid* pNewGrid = NULL;

	if (m_pParentWnd->GetSafeHwnd () != NULL)
	{
		pNewGrid = (CBCGPGanttGrid*)m_pParentWnd->SendMessage (BCGM_GANTT_CONTROL_CREATE_GRID, 0, 0);
	}

	if (pNewGrid != NULL)
	{
		ASSERT_VALID(pNewGrid);
		return pNewGrid;
	}
	else
	{
		return new CBCGPGanttGrid;
	}
}
//*****************************************************************************
CBCGPGanttChart* CBCGPGanttControl::CreateChart ()
{
	CBCGPGanttChart* pNewChart = NULL;

	if (m_pParentWnd->GetSafeHwnd () != NULL)
	{
		pNewChart = (CBCGPGanttChart*)m_pParentWnd->SendMessage (BCGM_GANTT_CONTROL_CREATE_CHART, 0, 0);
	}

	if (pNewChart != NULL)
	{
		ASSERT_VALID(pNewChart);
		return pNewChart;
	}
	else
	{
		return new CBCGPGanttChart;
	}
}
//*****************************************************************************
void CBCGPGanttControl::SetReadOnly (BOOL bReadOnly)
{
	if (m_pGrid != NULL && m_pChart != NULL)
	{
		m_pGrid->SetReadOnly (bReadOnly);
		m_pChart->SetReadOnly (bReadOnly);
	}
}
//*****************************************************************************
BOOL CBCGPGanttControl::IsReadOnly () const
{
	return m_pGrid != NULL && m_pGrid->IsReadOnly ();
}
//*****************************************************************************
int CBCGPGanttControl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int iResult = CBCGPWnd::OnCreate (lpCreateStruct);
	if (iResult != 0)
	{
		return iResult;
	}

	CWnd* pParent = CWnd::FromHandle (lpCreateStruct->hwndParent);
	if (pParent != NULL && pParent->IsKindOf (RUNTIME_CLASS (CView)) == NULL)
	{
		return CreateControls ();
	}

	return 0;
}
//*****************************************************************************
void CBCGPGanttControl::SetScrollBarsStyle (CBCGPScrollBar::BCGPSB_STYLE style)
{
	ASSERT (CBCGPScrollBar::BCGP_SBSTYLE_FIRST <= style && style <= CBCGPScrollBar::BCGP_SBSTYLE_LAST);

	m_pGrid->SetScrollBarsStyle (style);
	m_pChart->SetScrollBarsStyle (style);
}
//*****************************************************************************
BOOL CBCGPGanttControl::OnEraseBkgnd (CDC*)
{
	return TRUE;
}
//*****************************************************************************
void CBCGPGanttControl::OnPaint ()
{
	CPaintDC dc (this);
}
//*****************************************************************************
void CBCGPGanttControl::OnSize (UINT nType, int cx, int cy)
{
	CBCGPWnd::OnSize (nType, cx, cy);

	if (m_wndSplitter.GetSafeHwnd () != NULL)
	{
		m_wndSplitter.SetWindowPos (this, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | 
			SWP_NOZORDER | SWP_NOREDRAW);
	}	
}
//*****************************************************************************
LRESULT CBCGPGanttControl::OnSetFont (WPARAM wParam, LPARAM lParam)
{
	m_pGrid->SendMessage (WM_SETFONT, wParam, lParam);
	m_pChart->SendMessage (WM_SETFONT, wParam, lParam);
	return 0;
}
//*****************************************************************************
LRESULT CBCGPGanttControl::OnGetFont (WPARAM, LPARAM)
{
	return m_pChart->SendMessage (WM_GETFONT);
}
//*****************************************************************************
void CBCGPGanttControl::OnSetFocus(CWnd* pOldWnd)
{
	CBCGPWnd::OnSetFocus (pOldWnd);

	if (m_pGrid->GetSafeHwnd () != NULL)
	{
		m_pGrid->SetFocus ();
	}	
}
//*****************************************************************************
void CBCGPGanttControl::OnDestroy ()
{
	if (m_wndSplitter.GetSafeHwnd () != NULL)
	{
		m_wndSplitter.DestroyWindow ();
	}
}
//*****************************************************************************
void CBCGPGanttControl::AdjustLayout ()
{
	m_pGrid->AdjustLayout ();
	m_pChart->AdjustLayout ();
}
//*****************************************************************************
void CBCGPGanttControl::SetVisualManagerStyle(BOOL bSet/* = TRUE*/)
{
	m_bVisualManagerStyle = bSet;

	if (m_pGrid != NULL)
	{
		m_pGrid->SetVisualManagerColorTheme(bSet);
	}	

	if (m_pChart != NULL)
	{
		m_pChart->m_bVisualManagerStyle = bSet;
	}
}
//*****************************************************************************
LRESULT CBCGPGanttControl::OnChangeVisualManager (WPARAM /*wp*/, LPARAM /*lp*/)
{
	if (m_pGrid != NULL)
	{
		m_pGrid->SetVisualManagerColorTheme(m_bVisualManagerStyle);
	}	

	return 0;
}

#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)
