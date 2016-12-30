//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPGridFilter.cpp
//

#include "stdafx.h"
#include "BCGPGridCtrl.h"
#include "BCGPGridFilter.h"

/////////////////////////////////////////////////////////////////////////////
// BCGP_FILTER_COLUMN_INFO

void BCGP_FILTER_COLUMN_INFO::Clear ()
{
	nCol = -1;
	
	strFilter.Empty ();
	
	bAll = TRUE;
	lstValues.RemoveAll ();
}

BOOL BCGP_FILTER_COLUMN_INFO::IsEmpty () const
{
	if (nCol == -1)
	{
		return TRUE;
	}
	
	return strFilter.IsEmpty () && lstValues.IsEmpty ();
}

void BCGP_FILTER_COLUMN_INFO::Copy (const BCGP_FILTER_COLUMN_INFO& src)
{
	Clear ();
	
	nCol = src.nCol;
	strFilter = src.strFilter;
	bAll = src.bAll;
	
	lstValues.AddTail ((CStringList*) &src.lstValues);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridFilterParams

CBCGPGridFilterParams::CBCGPGridFilterParams ()
{
}

CBCGPGridFilterParams::~CBCGPGridFilterParams ()
{
	ClearAll ();
}
//****************************************************************************************
void CBCGPGridFilterParams::AddColumn (int nColumn, BCGP_FILTER_COLUMN_INFO* pParam)
{
	BCGP_FILTER_COLUMN_INFO* pOldValue = NULL;
	if (m_mapColumns.Lookup(nColumn, pOldValue))
	{
		ClearColumn (nColumn);
	}

	if (pParam == NULL)
	{
		pParam = new BCGP_FILTER_COLUMN_INFO;
		pParam->nCol = nColumn;
	}

	m_mapColumns.SetAt(nColumn, pParam);
}
//****************************************************************************************
void CBCGPGridFilterParams::ClearColumn (int nColumn)
{
	BCGP_FILTER_COLUMN_INFO* pParam = NULL;
	if (m_mapColumns.Lookup(nColumn, pParam) && pParam != NULL)
	{
		pParam->Clear ();
		delete pParam;
	}

	m_mapColumns.RemoveKey (nColumn);
}
//****************************************************************************************
void CBCGPGridFilterParams::ClearAll ()
{
	POSITION pos = m_mapColumns.GetStartPosition ();
	while (pos != NULL)
	{
		int nColumn = -1;
		BCGP_FILTER_COLUMN_INFO* pParam = NULL;

		m_mapColumns.GetNextAssoc (pos, nColumn, pParam);
		ASSERT (pParam != NULL);

		pParam->Clear ();
		delete pParam;
	}

	m_mapColumns.RemoveAll ();
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridFilter

CBCGPGridFilter::CBCGPGridFilter () : m_pOwnerGrid (NULL), m_bOneColumnOnly (FALSE)
{
}
//****************************************************************************************
void CBCGPGridFilter::SetFilter (CBCGPGridCtrl* pOwnerGrid)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pOwnerGrid);

	m_pOwnerGrid = pOwnerGrid;
	m_pOwnerGrid->EnableFilter (CBCGPGridFilter::pfnFilterCallback, (LPARAM)this);
}
//****************************************************************************************
BCGP_FILTER_COLUMN_INFO* CBCGPGridFilter::GetColumnInfo (int nColumn)
{
	ASSERT_VALID (this);

	BCGP_FILTER_COLUMN_INFO* pColumnInfo = m_params.Column (nColumn);
	
	if (pColumnInfo == NULL)
	{
		if (m_bOneColumnOnly)
		{
			m_params.ClearAll ();
		}

		m_params.AddColumn (nColumn);
		pColumnInfo = m_params.Column (nColumn);

		ASSERT(pColumnInfo != NULL);
	}

	return pColumnInfo;
}
//****************************************************************************************
// Override FilterFunc in the derived class. Return TRUE to hide row.
BOOL CBCGPGridFilter::FilterFunc (CBCGPGridRow* pRow, BCGP_FILTER_COLUMN_INFO* pInfo) 
{
	ASSERT_VALID (pRow);
	ASSERT (pInfo != NULL);

	//----------------------------------
	// The default filter implementation
	//----------------------------------
	if (pRow->IsGroup ())
	{
		return FALSE; // do not hide groups
	}

	CBCGPGridItem* pItem = pRow->GetItem (pInfo->nCol);
	if (pItem == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pItem);

	if (!pInfo->bAll)
	{
		const CString strItem = pItem->GetLabel ();
		if (pInfo->lstValues.Find (strItem) == NULL)
		{
			return TRUE; // hide
		}
	}
	
	return FALSE; // show all
}
//****************************************************************************************
// The predefined callback function
LRESULT CALLBACK CBCGPGridFilter::pfnFilterCallback (WPARAM wParam, LPARAM lParam)
{
	CBCGPGridRow* pRow = (CBCGPGridRow*) wParam;
	ASSERT_VALID (pRow);
	
	CBCGPGridFilter* pFilter = (CBCGPGridFilter*)lParam;
	ASSERT_VALID (pFilter);

	POSITION pos = pFilter->m_params.m_mapColumns.GetStartPosition ();
	while (pos != NULL)
	{
		int nColumn = -1;
		BCGP_FILTER_COLUMN_INFO* pParam = NULL;

		pFilter->m_params.m_mapColumns.GetNextAssoc (pos, nColumn, pParam);
		ASSERT (pParam != NULL);

		if (pFilter->FilterFunc(pRow, pParam))
		{
			return TRUE; // hide
		}
	}

	return FALSE; // do not hide
}
