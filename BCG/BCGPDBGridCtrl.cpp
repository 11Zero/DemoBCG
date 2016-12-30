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
// BCGPDBGridCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPDBGridCtrl.h"

#ifndef BCGP_EXCLUDE_GRID_CTRL

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPDBGridCtrl

IMPLEMENT_DYNAMIC(CBCGPDBGridCtrl, CBCGPGridCtrl)

CBCGPDBGridCtrl::CBCGPDBGridCtrl()
{
	m_bDbSort = FALSE;
	m_bIsSorting = FALSE;

	m_lstURLPrefixes.AddTail (_T("http://"));
	m_lstURLPrefixes.AddTail (_T("https://"));
	m_lstURLPrefixes.AddTail (_T("ftp://"));
	m_lstURLPrefixes.AddTail (_T("mailto:"));
}

CBCGPDBGridCtrl::~CBCGPDBGridCtrl()
{
}


BEGIN_MESSAGE_MAP(CBCGPDBGridCtrl, CBCGPGridCtrl)
	//{{AFX_MSG_MAP(CBCGPDBGridCtrl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPDBGridCtrl message handlers

BOOL CBCGPDBGridCtrl::ParseURLString (CString str, 
								CString& strTextToDisplay,
								CString& strURL)
{
	const int nLen = str.GetLength ();

	if (nLen < 3)
	{
		return FALSE;
	}

	int nStart = str.Find (_T('#'));
	if (nStart < 0 || nStart == nLen - 1)
	{
		return FALSE;
	}

	if (str.GetAt (nLen - 1) != _T('#'))
	{
		return FALSE;
	}

	for (POSITION pos = m_lstURLPrefixes.GetHeadPosition (); pos != NULL; )
	{
		CString strURLPrefix = m_lstURLPrefixes.GetNext (pos);

		if (str.Find (strURLPrefix, nStart + 1) == nStart + 1)
		{
			strURL = str.Mid (nStart + 1);
			strURL = strURL.Left (strURL.GetLength () - 1);

			if (nStart > 0)
			{
				strTextToDisplay = str.Left (nStart);
			}
			else
			{
				strTextToDisplay = strURL.Mid (strURLPrefix.GetLength ());
			}

			return TRUE;
		}
	}

	return FALSE;
}
//*****************************************************************************
BOOL CBCGPDBGridCtrl::BuildURL (CBCGPGridItem* pItem, CString& strResult)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pItem);

	CBCGPGridURLItem* pURLItem = DYNAMIC_DOWNCAST (CBCGPGridURLItem, pItem);
	if (pURLItem == NULL)
	{
		return FALSE;
	}

	CString strURL = pURLItem->GetURL ();
	CString strVal = (LPCTSTR) (_bstr_t) pURLItem->GetValue ();

	for (POSITION pos = m_lstURLPrefixes.GetHeadPosition (); pos != NULL; )
	{
		CString strURLPrefix = m_lstURLPrefixes.GetNext (pos);

		if (strURL.Find (strURLPrefix) == 0)
		{
			strURL = strURLPrefix + strVal;
			pURLItem->SetURL (strURL);

			strResult = strVal;
			strResult += _T('#');
			strResult += strURL;
			strResult +=  _T('#');
			return TRUE;
		}
	}

	return FALSE;
}
//*****************************************************************************
void CBCGPDBGridCtrl::Sort (int nColumn, BOOL bAscending, BOOL bAdd)
{
	if (m_bIsSorting || !CanSortByColumn (nColumn))
	{
		return;
	}

	if (!m_bDbSort || m_strSQL.IsEmpty ())
	{
		CBCGPGridCtrl::Sort (nColumn, bAscending, bAdd);
		return;
	}

	SetCurSel (NULL);

	m_CachedItems.CleanUpCache ();

	CString strSQLOrign = m_strSQL;

	CString strColumn = GetColumnName (nColumn);
	if (strColumn.Find (_T(' ')) >= 0)
	{
		strColumn = _T('\'') + strColumn + _T('\'');
	}

	CString strSQL;
	strSQL.Format (_T("%s ORDER BY %s %s"),
		m_strSQL, strColumn,
		bAscending ? _T(" ASC") : _T(" DESC"));

	if (bAdd)
	{
		for (POSITION pos = m_Columns.m_mapSortColumn.GetStartPosition (); pos != NULL; )
		{
			int nListColumn, nState;

			m_Columns.m_mapSortColumn.GetNextAssoc (pos, nListColumn, nState);

			if (nState != 0 && nListColumn != nColumn)
			{
				CString strListColumn = GetColumnName (nListColumn);
				if (strListColumn.Find (_T(' ')) >= 0)
				{
					strListColumn = _T('\'') + strListColumn + _T('\'');
				}

				CString strOrder;
				strOrder.Format (_T(", %s %s"),
					strListColumn,
					nState > 0 ? _T(" ASC") : _T(" DESC"));

				strSQL += strOrder;
			}
		}
	}

	CWaitCursor wait;

	m_bRebuildTerminalItems = TRUE;

	m_bIsSorting = TRUE;

	if (OpenSQL (strSQL))
	{
		m_Columns.SetSortColumn (nColumn, bAscending, bAdd);
	}

	RedrawWindow (m_rectHeader);

	m_bIsSorting = FALSE;
	m_strSQL = strSQLOrign;
}
//********************************************************************************
BOOL CALLBACK CBCGPDBGridCtrl::GridCallback (BCGPGRID_DISPINFO* /*pdi*/, LPARAM /*lp*/)
{
	return TRUE;
}
//********************************************************************************
void CBCGPDBGridCtrl::EnableVirtualMode ()
{
	CBCGPGridCtrl::EnableVirtualMode (GridCallback);
}

#endif // BCGP_EXCLUDE_GRID_CTRL
