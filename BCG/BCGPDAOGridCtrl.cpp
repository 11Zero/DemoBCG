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
// BCGPDAOGridCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPDAOGridCtrl.h"

#ifndef BCGP_EXCLUDE_GRID_CTRL

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable : 4995)

#ifndef _WIN64

/////////////////////////////////////////////////////////////////////////////
// CBCGPDAOGridCtrl

IMPLEMENT_DYNCREATE(CBCGPDAOGridCtrl, CBCGPDBGridCtrl)

CBCGPDAOGridCtrl::CBCGPDAOGridCtrl()
{
	m_pDataBase = NULL;
	m_pRecordSet = NULL;
}
//***************************************************************************
CBCGPDAOGridCtrl::~CBCGPDAOGridCtrl()
{
	Close ();
}

BEGIN_MESSAGE_MAP(CBCGPDAOGridCtrl, CBCGPDBGridCtrl)
	//{{AFX_MSG_MAP(CBCGPDAOGridCtrl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPDAOGridCtrl message handlers

BOOL CBCGPDAOGridCtrl::OpenDB (LPCTSTR /*lpszConnectionStr*/,
		LPCTSTR /*lpszUserName*/, LPCTSTR /*lpszPassword*/, LPARAM /*lOptions */)
{
	ASSERT (FALSE);

	// You should not use direct connections to ODBC data sources;
	// use an attached table instead
	
	return FALSE;
}
//***************************************************************************	
BOOL CBCGPDAOGridCtrl::OpenMSAccessFile (LPCTSTR lpszFileName,
		LPCTSTR /*lpszUserName*/, LPCTSTR /*lpszPassword*/, LPARAM /*lOptions TODO */)
{
	ASSERT(lpszFileName != NULL);

	try
	{
		Close ();
		
		ASSERT (m_pDataBase == NULL);
		m_pDataBase = new CDaoDatabase;

		m_pDataBase->Open (lpszFileName);
		return m_pDataBase->IsOpen ();
	}
	catch (CDaoException* pEx)
	{
		OnDaoException (pEx);
		pEx->Delete ();
	}

	return FALSE;
}
//***************************************************************************
BOOL CBCGPDAOGridCtrl::Close ()
{
	RemoveAll ();
	DeleteAllColumns ();

	try
	{
		if (m_pRecordSet != NULL)
		{
			ASSERT_VALID(m_pRecordSet);

			if (m_pRecordSet->IsOpen ())
			{
				m_pRecordSet->Close ();
			}

			delete m_pRecordSet;
			m_pRecordSet = NULL;
		}

		if (m_pDataBase != NULL)
		{
			ASSERT_VALID (m_pDataBase);

			if (m_pDataBase->IsOpen ())
			{
				m_pDataBase->Close ();
			}

			delete m_pDataBase;
			m_pDataBase = NULL;
		}
	}
	catch (CDaoException* pEx)
	{
		OnDaoException (pEx);
		pEx->Delete ();

		return FALSE;
	}

	return TRUE;
}
//***************************************************************************
BOOL CBCGPDAOGridCtrl::GetTableList (CStringList& lstTable)
{
	lstTable.RemoveAll ();

	try
	{
		if (m_pDataBase == NULL)
		{
			ASSERT (FALSE);
			return FALSE;
		}

		ASSERT_VALID (m_pDataBase);

		if (!m_pDataBase->IsOpen ())
		{
			ASSERT (FALSE);
			return FALSE;
		}

		const int nTableDefCount = m_pDataBase->GetTableDefCount ();

		for (int i = 0; i < nTableDefCount; i++)
		{
			CDaoTableDefInfo tabInfo;
			m_pDataBase->GetTableDefInfo (i, tabInfo, AFX_DAO_ALL_INFO);
				
			if (!(tabInfo.m_lAttributes & dbSystemObject))
			{
				lstTable.AddTail (tabInfo.m_strName);
			}
		}
	}
	catch (CDaoException* pEx)
	{
		OnDaoException (pEx);
		pEx->Delete ();

		return FALSE;
	}

	return TRUE;
}
//***************************************************************************
BOOL CBCGPDAOGridCtrl::GetFieldList (LPCTSTR lpszTable, CStringList& lstField)
{
	ASSERT (lpszTable != NULL);

	lstField.RemoveAll ();

	try
	{
		if (m_pDataBase == NULL)
		{
			ASSERT (FALSE);
			return FALSE;
		}

		ASSERT_VALID (m_pDataBase);

		if (!m_pDataBase->IsOpen ())
		{
			ASSERT (FALSE);
			return FALSE;
		}

		CDaoRecordset* pRecordSet = new CDaoRecordset (m_pDataBase);

		CString strSQL = _T("SELECT * FROM ");
		strSQL += lpszTable;

		pRecordSet->Open (AFX_DAO_USE_DEFAULT_TYPE, strSQL);

		if (!pRecordSet->IsOpen ())
		{
			delete pRecordSet;
			return FALSE;
		}

		const short nColumns = pRecordSet->GetFieldCount ();

		for (short nColumn = 0; nColumn < nColumns; nColumn++)
		{
			CDaoFieldInfo info;
			pRecordSet->GetFieldInfo (nColumn, info);

			lstField.AddTail (info.m_strName);
		}

		pRecordSet->Close ();
		delete pRecordSet;
	}
	catch (CDaoException* pEx)
	{
		OnDaoException (pEx);
		pEx->Delete ();

		return FALSE;
	}

	return TRUE;
}
//***************************************************************************
BOOL CBCGPDAOGridCtrl::GetFieldList (CStringList& lstField)
{
	lstField.RemoveAll ();

	try
	{
		ASSERT_VALID (m_pRecordSet);

		const short nColumns = m_pRecordSet->GetFieldCount ();

		for (short nColumn = 0; nColumn < nColumns; nColumn++)
		{
			CDaoFieldInfo info;
			m_pRecordSet->GetFieldInfo (nColumn, info);

			lstField.AddTail (info.m_strName);
		}
	}
	catch (CDaoException* pEx)
	{
		OnDaoException (pEx);
		pEx->Delete ();

		return FALSE;
	}

	return TRUE;
}
//***************************************************************************
BOOL CBCGPDAOGridCtrl::OpenSQL (LPCTSTR lpszSQL)
{
	try
	{
		ASSERT (lpszSQL != NULL);
		m_strSQL.Empty ();

		RemoveAll ();

		if (!m_bIsSorting)
		{
			DeleteAllColumns ();
		}
		
		if (m_pDataBase == NULL)
		{
			ASSERT (FALSE);
			return FALSE;
		}

		ASSERT_VALID (m_pDataBase);

		if (!m_pDataBase->IsOpen ())
		{
			ASSERT (FALSE);
			return FALSE;
		}

		if (m_pRecordSet != NULL)
		{
			ASSERT_VALID (m_pRecordSet);

			if (m_pRecordSet->IsOpen ())
			{
				m_pRecordSet->Close ();
			}

			delete m_pRecordSet;
			m_pRecordSet = NULL;
		}

		//---------------------------------------------------------
		// Create a new record set and open it using SQL statement:
		//---------------------------------------------------------
		m_pRecordSet = new CDaoRecordset (m_pDataBase);
		m_pRecordSet->Open (dbOpenDynaset, lpszSQL);

		if (!m_pRecordSet->IsOpen ())
		{
			return FALSE;
		}

		int nColumns = 0;

		if (!m_bIsSorting)
		{
			//-------------
			// Add columns:
			//-------------
			CStringList lstField;
			if (!GetFieldList (lstField))
			{
				return FALSE;
			}

			int nColumn = 0;
			for (POSITION pos = lstField.GetHeadPosition (); pos != NULL; nColumn++)
			{
				InsertColumn (nColumn, lstField.GetNext (pos), 50);
			}

			nColumns = (int) lstField.GetCount ();
		}
		else
		{
			nColumns = GetColumnCount (); 
		}

		if (nColumns == 0)
		{
			// No columns
			AdjustLayout ();
			return TRUE;
		}

		//-------------
		// Add records:
		//-------------
		if (m_pRecordSet->IsEOF () && m_pRecordSet->IsBOF ())
		{
			// The table is empty
			AdjustLayout ();
			return TRUE;
		}

		if (m_bVirtualMode)
		{
			while (!m_pRecordSet->IsEOF ())
			{
				m_pRecordSet->MoveNext ();
			}

			SetVirtualRows (max (0, m_pRecordSet->GetRecordCount ()));
		}		
		else
		{
			for (int nRow = 0; !m_pRecordSet->IsEOF (); 
				m_pRecordSet->MoveNext (), nRow++)
			{
				CBCGPGridRow* pRow = CreateRow (nColumns);
				ASSERT_VALID (pRow);

				for (int nColumn = 0; nColumn < nColumns; nColumn++)
				{
					OnAddData (pRow, nColumn, nRow);
				}

				if (OnBeforeAddRow (pRow, nRow))
				{
					AddRow (pRow, FALSE);
				}
				else
				{
					delete pRow;
				}
			}
		}

		if (!m_pRecordSet->CanUpdate ())
		{
			SetReadOnly ();
		}
	}
	catch (CDaoException* pEx)
	{
		OnDaoException (pEx);
		pEx->Delete ();

		return FALSE;
	}

	m_strSQL = lpszSQL;
	AdjustLayout ();
	return TRUE;
}
//***************************************************************************
BOOL CBCGPDAOGridCtrl::OpenTable (LPCTSTR lpszTable)
{
	ASSERT (lpszTable != NULL);

	CString strSQL = _T("SELECT * FROM ");
	strSQL += lpszTable;

	return OpenSQL (strSQL);
}
//***************************************************************************
BOOL CBCGPDAOGridCtrl::OnAddData (CBCGPGridRow* pRow, 
								   int nColumn, int /*nRow*/)
{
	try
	{
		ASSERT_VALID (pRow);
		ASSERT_VALID (m_pRecordSet);
		ASSERT (!m_pRecordSet->IsEOF ());
		ASSERT (!m_pRecordSet->IsBOF ());

		COleVariant varValue;
		m_pRecordSet->GetFieldValue ((short) nColumn, varValue);

		if (varValue.vt == VT_BOOL)
		{
			pRow->ReplaceItem (nColumn, new CBCGPGridCheckItem (varValue.bVal != 0), FALSE, TRUE);
		}
#ifndef _BCGPGRID_STANDALONE
		else if (varValue.vt == VT_DATE)
		{
			pRow->ReplaceItem (nColumn, new CBCGPGridDateTimeItem (varValue.date), FALSE, TRUE);
		}
#endif
		else if (varValue.vt == VT_BSTR)
		{
			CString str = (LPCTSTR) V_BSTRT(&varValue);

			CString strTextToDisplay;
			CString strURL;

			if (ParseURLString (str, strTextToDisplay, strURL))
			{
				pRow->ReplaceItem (nColumn, new CBCGPGridURLItem (strTextToDisplay, strURL), FALSE, TRUE);
			}
			else
			{
				pRow->GetItem (nColumn)->SetValue ((LPCTSTR) str, FALSE);
			}
		}
		else if (varValue.vt == VT_NULL || varValue.vt == VT_EMPTY)
		{
			//--------------------------------------------------------------
			// Check the field type: if it's a string, pass an empty string:
			//--------------------------------------------------------------
			CDaoFieldInfo info;
			m_pRecordSet->GetFieldInfo (nColumn, info);

			switch (info.m_nType)
			{
			case dbText:
			case dbMemo:
				pRow->GetItem (nColumn)->SetValue (_T(""), FALSE);
			}
		}
		else
		{
			pRow->GetItem (nColumn)->SetValue (varValue, FALSE);
		}
	}
	catch (CDaoException* pEx)
	{
		OnDaoException (pEx);
		pEx->Delete ();
	}

	return TRUE;
}
//***************************************************************************
void CBCGPDAOGridCtrl::OnItemChanged (CBCGPGridItem* pItem, int nRow, int nColumn)
{
	try
	{
		ASSERT_VALID (this);

		if (m_pRecordSet == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		ASSERT_VALID (pItem);
		ASSERT_VALID (m_pRecordSet);

		CBCGPGridRow* pRow = pItem->GetParentRow ();
		if (pRow == NULL)
		{
			return;
		}

		ASSERT_VALID (pRow);

		m_pRecordSet->MoveFirst ();
		m_pRecordSet->Move (nRow);

		m_pRecordSet->Edit ();

		COleVariant varCurr;
		m_pRecordSet->GetFieldValue ((short) nColumn, varCurr);

		COleVariant varOut;
		_variant_t varItemData = pItem->GetValue ();

		if (varItemData.vt == VT_BSTR)
		{
			CString str = (LPCTSTR)(_bstr_t) varItemData;
			BuildURL (pItem, str);

			varOut = COleVariant (str, VT_BSTRT);
		}
		else
		{
			varOut = varItemData;
		}

		m_pRecordSet->SetFieldValue ((short) nColumn, varOut);
		m_pRecordSet->Update ();
	}
	catch (CDaoException* pEx)
	{
		OnDaoException (pEx);
		pEx->Delete ();

		OnItemUpdateFailed ();
	}
}
//*****************************************************************************
BOOL CBCGPDAOGridCtrl::CanSortByColumn (int nColumn)
{
	try
	{
		CDaoFieldInfo info;
		m_pRecordSet->GetFieldInfo (nColumn, info);

		switch (info.m_nType)
		{
		case dbBoolean:
		case dbByte:
		case dbInteger:
		case dbLong:
		case dbCurrency:
		case dbSingle:
		case dbDouble:
		case dbDate:
		case dbText:
		case dbMemo:
			return TRUE;
		}
	}
	catch (CDaoException* pEx)
	{
		OnDaoException (pEx);
		pEx->Delete ();
	}

	return FALSE;
}
//*****************************************************************************
CBCGPGridRow* CBCGPDAOGridCtrl::CreateVirtualRow (int nRow)
{
	CBCGPGridRow* pRow = CBCGPGridCtrl::CreateVirtualRow (nRow);

	if (pRow == NULL)
	{
		return NULL;
	}

	if (m_pRecordSet == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	try
	{
		m_pRecordSet->MoveFirst ();
		m_pRecordSet->Move (nRow);

		for (int nCol = 0; nCol < GetColumnCount (); nCol++)
		{
			OnAddData (pRow, nCol, nRow);
		}
	}
	catch (CDaoException* pEx)
	{
		OnDaoException (pEx);
		pEx->Delete ();
	}

	return pRow;
}

#pragma warning (default : 4995)

#endif // BCGP_EXCLUDE_GRID_CTRL
#endif // !_WIN64
