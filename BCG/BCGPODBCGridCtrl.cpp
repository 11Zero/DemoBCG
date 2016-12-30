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
// BCGPODBCGridCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPODBCGridCtrl.h"

#ifndef BCGP_EXCLUDE_GRID_CTRL

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPODBCGridCtrl

IMPLEMENT_DYNCREATE(CBCGPODBCGridCtrl, CBCGPDBGridCtrl)

CBCGPODBCGridCtrl::CBCGPODBCGridCtrl()
{
	m_pDataBase = NULL;
	m_pRecordSet = NULL;
}

CBCGPODBCGridCtrl::~CBCGPODBCGridCtrl()
{
	Close ();
}

BEGIN_MESSAGE_MAP(CBCGPODBCGridCtrl, CBCGPDBGridCtrl)
	//{{AFX_MSG_MAP(CBCGPODBCGridCtrl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPODBCGridCtrl message handlers

BOOL CBCGPODBCGridCtrl::OpenDB (LPCTSTR lpszConnectionStr,
		LPCTSTR lpszUserName, LPCTSTR lpszPassword, LPARAM lOptions)
{
	ASSERT (lpszConnectionStr != NULL);
	ASSERT (lpszUserName != NULL);
	ASSERT (lpszPassword != NULL);

	try
	{
		Close ();
		
		ASSERT (m_pDataBase == NULL);
		m_pDataBase = new CDatabase;

		CString strConnect;
		strConnect.Format (_T("%s;UID=%s;PWD=%s"), 
			lpszConnectionStr, lpszUserName, lpszPassword);

		return m_pDataBase->OpenEx (strConnect, (DWORD) lOptions);
	}
	catch (CDBException* pEx)
	{
		OnODBCException (pEx);
		pEx->Delete ();
	}

	return FALSE;
}
	
BOOL CBCGPODBCGridCtrl::OpenMSAccessFile (LPCTSTR lpszFileName,
		LPCTSTR lpszUserName, LPCTSTR lpszPassword, LPARAM lOptions)
{
	ASSERT(lpszFileName != NULL);

	CString strConnect;
	strConnect.Format (_T("DSN=MS Access Database;DBQ=%s"), 
		lpszFileName);

	return OpenDB (strConnect, lpszUserName, lpszPassword, lOptions);
}

BOOL CBCGPODBCGridCtrl::Close ()
{
	RemoveAll ();
	DeleteAllColumns ();

	try
	{
		if (m_pRecordSet != NULL)
		{
			ASSERT_VALID(m_pRecordSet);

			if(m_pRecordSet->IsOpen ())
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
	catch (CDBException* pEx)
	{
		OnODBCException (pEx);
		pEx->Delete ();
		return FALSE;
	}

	return TRUE;
}

BOOL CBCGPODBCGridCtrl::GetTableList (CStringList& lstTable)
{
	lstTable.RemoveAll ();

	if (m_pDataBase == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT_VALID (m_pDataBase);

	try
	{
		if (!m_pDataBase->IsOpen ())
		{
			ASSERT (FALSE);
			return FALSE;
		}

		HSTMT hStmt;
		SQLRETURN rc = ::SQLAllocStmt(m_pDataBase->m_hdbc, &hStmt);

		if (!m_pDataBase->Check (rc))
		{
			return FALSE;
		}

	#if _MSC_VER >= 1300
		#ifdef 	UNICODE
			#define __BCGP_SQLCHAR	SQLWCHAR
		#else
			#define __BCGP_SQLCHAR	SQLCHAR
		#endif
	#else
		#define __BCGP_SQLCHAR	SQLCHAR
	#endif

		rc = ::SQLTables (hStmt,
						NULL,SQL_NTS,
						NULL,SQL_NTS,
						NULL,SQL_NTS,
						(__BCGP_SQLCHAR*) _T("'TABLE'"), SQL_NTS);
		
		if (!m_pDataBase->Check (rc))
		{
			AfxThrowDBException (rc, m_pDataBase, hStmt);
		}
		
		while ((rc = ::SQLFetch (hStmt)) != SQL_NO_DATA_FOUND)
		{
			if (!m_pDataBase->Check (rc))
			{
				AfxThrowDBException (rc, m_pDataBase, hStmt);
			}

			UCHAR	szName [256];
#if _MSC_VER < 1300
			SDWORD	cbName;
#else
			SQLLEN	cbName;
#endif

			rc = ::SQLGetData (hStmt, 3, SQL_C_CHAR, szName, 256, &cbName);

			if (!m_pDataBase->Check (rc))
			{
				AfxThrowDBException (rc, m_pDataBase, hStmt);
			}

#ifdef 	_UNICODE
			lstTable.AddTail(CString(szName));
#else
			lstTable.AddTail ((LPCTSTR)szName);
#endif
		}
		
		::SQLFreeStmt (hStmt, SQL_CLOSE);
	}
	catch (CDBException* pEx)
	{
		OnODBCException (pEx);
		pEx->Delete ();

		return FALSE;
	}

	return TRUE;
}

BOOL CBCGPODBCGridCtrl::GetFieldList (LPCTSTR lpszTable, CStringList& lstField)
{
	ASSERT (lpszTable != NULL);

	lstField.RemoveAll ();

	if (m_pDataBase == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	try
	{
		ASSERT_VALID (m_pDataBase);

		if (!m_pDataBase->IsOpen ())
		{
			ASSERT (FALSE);
			return FALSE;
		}

		CRecordset* pRecordSet = new CRecordset (m_pDataBase);

		CString strSQL = _T("SELECT * FROM ");
		strSQL += lpszTable;

		if (!pRecordSet->Open (CRecordset::forwardOnly, strSQL))
		{
			delete pRecordSet;
			return FALSE;
		}

		const short nColumns = pRecordSet->GetODBCFieldCount ();

		for (short nColumn = 0; nColumn < nColumns; nColumn++)
		{
			CODBCFieldInfo fieldinfo;
			pRecordSet->GetODBCFieldInfo (nColumn, fieldinfo);

			lstField.AddTail (fieldinfo.m_strName);
		}

		pRecordSet->Close ();
		delete pRecordSet;
	}
	catch (CDBException* pEx)
	{
		OnODBCException (pEx);
		pEx->Delete ();

		return FALSE;
	}

	return TRUE;
}

BOOL CBCGPODBCGridCtrl::GetFieldList (CStringList& lstField)
{
	lstField.RemoveAll ();

	if (m_pRecordSet == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	try
	{
		ASSERT_VALID (m_pRecordSet);

		const short nColumns = m_pRecordSet->GetODBCFieldCount ();

		for (short nColumn = 0; nColumn < nColumns; nColumn++)
		{
			CODBCFieldInfo fieldinfo;
			m_pRecordSet->GetODBCFieldInfo (nColumn, fieldinfo);

			lstField.AddTail (fieldinfo.m_strName);
		}
	}
	catch (CDBException* pEx)
	{
		OnODBCException (pEx);
		pEx->Delete ();

		return FALSE;
	}

	return TRUE;
}

BOOL CBCGPODBCGridCtrl::OpenSQL (LPCTSTR lpszSQL)
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

	try
	{
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
		m_pRecordSet = new CRecordset (m_pDataBase);
		if (!m_pRecordSet->Open (CRecordset::dynaset, lpszSQL))
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

		m_strSQL = lpszSQL;
		AdjustLayout ();

		if (!m_pRecordSet->CanUpdate ())
		{
			SetReadOnly ();
		}
	}
	catch (CDBException* pEx)
	{
		OnODBCException (pEx);
		pEx->Delete ();

		return FALSE;
	}

	return TRUE;
}

BOOL CBCGPODBCGridCtrl::OpenTable (LPCTSTR lpszTable)
{
	ASSERT (lpszTable != NULL);

	CString strSQL = _T("SELECT * FROM ");
	strSQL += lpszTable;

	return OpenSQL (strSQL);
}

BOOL CBCGPODBCGridCtrl::OnAddData (CBCGPGridRow* pRow, 
								   int nColumn, int nRow)
{
	ASSERT_VALID (pRow);
	ASSERT_VALID (m_pRecordSet);
	ASSERT (!m_pRecordSet->IsEOF ());
	ASSERT (!m_pRecordSet->IsBOF ());

	CDBVariant varValue;
	m_pRecordSet->GetFieldValue ((short) nColumn, varValue);

	return SetFieldData (varValue, pRow, nColumn, nRow);
}

BOOL CBCGPODBCGridCtrl::SetFieldData (	CDBVariant& varValue,
										CBCGPGridRow* pRow, 
										int nColumn, int /*nRow*/)
{
	ASSERT_VALID (pRow);

	CString strDBString;
	BOOL bIsText = FALSE;

	switch (varValue.m_dwType)
	{
	case DBVT_BOOL:
		pRow->ReplaceItem (nColumn, new CBCGPGridCheckItem ((varValue.m_boolVal & 1) != 0), FALSE, TRUE);
		break;

	case DBVT_UCHAR:
		pRow->GetItem (nColumn)->SetValue (varValue.m_chVal, FALSE);
		break;

	case DBVT_SHORT:
		pRow->GetItem (nColumn)->SetValue (varValue.m_iVal, FALSE);
		break;

	case DBVT_SINGLE:
		pRow->GetItem (nColumn)->SetValue (varValue.m_fltVal, FALSE);
		break;

	case DBVT_DOUBLE:
		pRow->GetItem (nColumn)->SetValue (varValue.m_dblVal, FALSE);
		break;

#ifndef _BCGPGRID_STANDALONE
	case DBVT_DATE:
		{
			TIMESTAMP_STRUCT* pTS = varValue.m_pdate;
			ASSERT (pTS != NULL);

			COleDateTime date (pTS->year, pTS->month, pTS->day,
				pTS->hour, pTS->minute, pTS->second);
			pRow->ReplaceItem (nColumn, new CBCGPGridDateTimeItem (date), FALSE, TRUE);
		}
		break;
#endif
	case DBVT_NULL:
		{
			//--------------------------------------------------------------
			// Check the field type: if it's a string, pass an empty string:
			//--------------------------------------------------------------
			CODBCFieldInfo fieldinfo;
			m_pRecordSet->GetODBCFieldInfo ((short) nColumn, fieldinfo);

			switch (fieldinfo.m_nSQLType)
			{
			case SQL_VARCHAR:
			case SQL_LONGVARCHAR:
			case SQL_WCHAR:
			case SQL_WVARCHAR:
			case SQL_WLONGVARCHAR:
				pRow->GetItem (nColumn)->SetValue (_T(""), FALSE);
				break;
			}
		}
		break;

#if _MSC_VER >= 1300
	case DBVT_ASTRING:
		strDBString = *varValue.m_pstringA;
		bIsText = TRUE;
		break;

	case DBVT_WSTRING:
		strDBString = *varValue.m_pstringW;
		bIsText = TRUE;
		break;
#endif

	case DBVT_STRING:
		strDBString = *varValue.m_pstring;
		bIsText = TRUE;
		break;

	case DBVT_LONG:
		pRow->GetItem (nColumn)->SetValue (varValue.m_lVal, FALSE);
		break;					
	}

	if (bIsText)
	{
		CString strTextToDisplay;
		CString strURL;

		if (ParseURLString (strDBString, strTextToDisplay, strURL))
		{
			pRow->ReplaceItem (nColumn, new CBCGPGridURLItem (strTextToDisplay, strURL), FALSE, TRUE);
		}
		else
		{
			pRow->GetItem (nColumn)->SetValue ((LPCTSTR) strDBString, FALSE);
		}
	}

	return TRUE;
}

void CBCGPODBCGridCtrl::OnItemChanged (CBCGPGridItem* pItem, int nRow, int nColumn)
{
	ASSERT_VALID (this);

	if (m_pRecordSet == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	try
	{
		ASSERT_VALID (pItem);
		ASSERT_VALID (m_pRecordSet);

		CBCGPGridRow* pRow = pItem->GetParentRow ();
		if (pRow == NULL)
		{
			return;
		}

		ASSERT_VALID (pRow);

		CString strTable = m_pRecordSet->GetTableName ();

		RETCODE nRetCode;
		SQLUSMALLINT   RowStatusArray[10];
		SQLHSTMT  hstmtU;

		AFX_SQL_SYNC (::SQLAllocStmt (m_pDataBase->m_hdbc, &hstmtU));
		if (!m_pDataBase->Check (nRetCode))
		{
			AfxThrowDBException (nRetCode, m_pDataBase, hstmtU);
		}

		AFX_SQL_SYNC (::SQLSetStmtAttr (hstmtU, SQL_ATTR_CONCURRENCY, (SQLPOINTER)SQL_CONCUR_LOCK, 0));
		if (!m_pDataBase->Check (nRetCode))
		{
			AfxThrowDBException (nRetCode, m_pDataBase, hstmtU);
		}

		AFX_SQL_SYNC (::SQLSetStmtAttr (hstmtU, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, 0));
		if (!m_pDataBase->Check (nRetCode))
		{
			AfxThrowDBException (nRetCode, m_pDataBase, hstmtU);
		}
		AFX_SQL_SYNC (::SQLSetStmtAttr (hstmtU, SQL_ATTR_ROW_BIND_TYPE, SQL_BIND_BY_COLUMN, 0));
		if (!m_pDataBase->Check (nRetCode))
		{
			AfxThrowDBException (nRetCode, m_pDataBase, hstmtU);
		}
		AFX_SQL_SYNC (::SQLSetStmtAttr (hstmtU, SQL_ATTR_ROW_STATUS_PTR, RowStatusArray, 0));
		if (!m_pDataBase->Check (nRetCode))
		{
			AfxThrowDBException (nRetCode, m_pDataBase, hstmtU);
		}

		_variant_t vtItemValue = pItem->GetValue ();

	    TCHAR				szFieldBuffer[5000];
		SQLSMALLINT			cbBoolVal;
		tagTIMESTAMP_STRUCT datetime;
		SQLSMALLINT			dataFormatShort;
		SQLINTEGER			dataFormatLong;
		TCHAR				dataFormatChar;
		SQLUSMALLINT		dataFormatUShort;
		SQLUINTEGER			dataFormatULong;
		SQLREAL				dataFormatFloat;
		SQLDOUBLE			dataFormatDouble;

#ifdef _WIN64
		SQLLEN				nFieldLenOrInd;
		SQLLEN				lenColDataTypeInd;
#else
		SQLINTEGER			nFieldLenOrInd;
		SQLINTEGER			lenColDataTypeInd;
#endif

		switch (vtItemValue.vt)
		{
		case VT_BSTR:
			  {
				CString strVal = (LPCTSTR) (_bstr_t) pItem->GetValue ();
				BuildURL (pItem, strVal);

				AFX_SQL_SYNC (::SQLBindCol (hstmtU, (unsigned short)(nColumn+1), 
					SQL_C_CHAR, szFieldBuffer, strVal.GetLength (), &nFieldLenOrInd)); 
			  }
			break;
		case VT_BOOL:
			{
				AFX_SQL_SYNC (::SQLBindCol (hstmtU, (unsigned short)(nColumn+1),
					SQL_C_BIT, &cbBoolVal, 0, 0));
			}
			break;

		case VT_DATE:
			{
				AFX_SQL_SYNC (::SQLBindCol (hstmtU, (unsigned short)(nColumn+1),
					SQL_C_TIMESTAMP, &datetime, sizeof (datetime), NULL));
			}
			break;
		case VT_I2:
			{
				AFX_SQL_SYNC (::SQLBindCol (hstmtU, (unsigned short)(nColumn+1), 
					SQL_C_SSHORT, &dataFormatShort, 0, &lenColDataTypeInd ));
			}
			break;
		case VT_I4:
		case VT_INT:
			{
				AFX_SQL_SYNC (::SQLBindCol (hstmtU, (unsigned short)(nColumn+1), 
					SQL_C_SLONG, &dataFormatLong, 0, &lenColDataTypeInd ));
			}
			break;
		case VT_UI1:
			{
				AFX_SQL_SYNC (::SQLBindCol (hstmtU, (unsigned short)(nColumn+1),
					SQL_C_BIT, &dataFormatChar, 0, &lenColDataTypeInd ));
			}
			break;
		case VT_UI2:
			{
				AFX_SQL_SYNC (::SQLBindCol (hstmtU, (unsigned short)(nColumn+1), 
					SQL_C_USHORT, &dataFormatUShort, 0, &lenColDataTypeInd ));
			}
			break;
		case VT_UINT:
		case VT_UI4:
			{
				AFX_SQL_SYNC (::SQLBindCol (hstmtU, (unsigned short)(nColumn+1),
					SQL_C_ULONG, &dataFormatULong, 0, &lenColDataTypeInd ));
			}
			break;
		case VT_R4:
			{
				AFX_SQL_SYNC (::SQLBindCol (hstmtU, (unsigned short)(nColumn+1),
					SQL_C_FLOAT, &dataFormatFloat, 0, &lenColDataTypeInd ));
			}
			break;
		case VT_R8:
			{
				AFX_SQL_SYNC (::SQLBindCol (hstmtU, (unsigned short)(nColumn+1),
					SQL_C_DOUBLE, &dataFormatDouble, 0, &lenColDataTypeInd ));
			}
			break;
		default:
			return;

		};

		if (!m_pDataBase->Check (nRetCode))
		{
			AfxThrowDBException (nRetCode, m_pDataBase, hstmtU);
		}

		CString strSQL = m_pRecordSet->GetSQL ();

#ifdef _UNICODE
		#if _MSC_VER >= 1300
			AFX_SQL_SYNC (::SQLExecDirect (hstmtU, (SQLWCHAR*)(LPCTSTR)strSQL, SQL_NTS));
		#else
			AFX_SQL_SYNC (::SQLExecDirect (hstmtU, (UCHAR*)(LPCTSTR)strSQL, SQL_NTS));
		#endif
#else
		AFX_SQL_SYNC (::SQLExecDirect (hstmtU, (SQLCHAR*)(LPCTSTR)strSQL, SQL_NTS));
#endif

		if (!m_pDataBase->Check (nRetCode))
		{
			AfxThrowDBException (nRetCode, m_pDataBase, hstmtU);
		}

		AFX_SQL_SYNC (::SQLFetchScroll (hstmtU, SQL_FETCH_ABSOLUTE, nRow+1));
		if (!m_pDataBase->Check (nRetCode))
		{
			AfxThrowDBException (nRetCode, m_pDataBase, hstmtU);
		}
		
		switch (vtItemValue.vt)
		{
		case VT_BSTR:
			{
				CString strVal = (LPCTSTR) (_bstr_t) pItem->GetValue ();
				BuildURL (pItem, strVal);

				nFieldLenOrInd = strVal.GetLength ();
				lstrcpy (szFieldBuffer, (LPCTSTR)strVal);;
			}
			break;
		case VT_DATE:
			{
				COleDateTime oleDate = (DATE) vtItemValue;
				datetime.day = (unsigned short)oleDate.GetDay ();
				datetime.month = (unsigned short)oleDate.GetMonth ();
				datetime.year = (unsigned short)oleDate.GetYear ();
				datetime.hour = (unsigned short)oleDate.GetHour ();
				datetime.minute = (unsigned short)oleDate.GetMinute ();
				datetime.second = (unsigned short)oleDate.GetSecond (); 
			}
			break;
		case VT_BOOL:
			cbBoolVal = (bool) vtItemValue;
			break;
		case VT_I2:
			dataFormatShort = (short) vtItemValue;
			break;
		case VT_I4:
		case VT_INT:
			dataFormatLong = (long) vtItemValue;
			break;
		case VT_UI1:
			dataFormatChar = (TCHAR)(BYTE)vtItemValue;
			break;
		case VT_UI2:
			dataFormatUShort = vtItemValue.uiVal;
			break;
		case VT_UINT:
		case VT_UI4:
			dataFormatULong = vtItemValue.ulVal;
			break;
		case VT_R4:
			dataFormatFloat = (float) vtItemValue;
			break;
		case VT_R8:
			dataFormatDouble = (double) vtItemValue;
			break;
		default:
			return;

		};

		AFX_SQL_SYNC (::SQLSetPos (hstmtU, 1, SQL_UPDATE, SQL_LOCK_NO_CHANGE));
		if (!m_pDataBase->Check (nRetCode))
		{
			AfxThrowDBException (nRetCode, m_pDataBase, hstmtU);
		}

		AFX_SQL_SYNC(::SQLCloseCursor(hstmtU));
		if (!m_pDataBase->Check (nRetCode))
		{
			AfxThrowDBException (nRetCode, m_pDataBase, hstmtU);
		}
	}
	catch (CDBException* pEx)
	{
		OnODBCException (pEx);
		pEx->Delete ();

		OnItemUpdateFailed ();
	}
}
//*****************************************************************************
BOOL CBCGPODBCGridCtrl::CanSortByColumn (int nColumn)
{
	try
	{
		CODBCFieldInfo fieldinfo;
		m_pRecordSet->GetODBCFieldInfo ((short) nColumn, fieldinfo);

		switch (fieldinfo.m_nSQLType)
		{
		case SQL_CHAR:
		case SQL_VARCHAR:
		case SQL_LONGVARCHAR:
		case SQL_WCHAR:
		case SQL_WVARCHAR:
		case SQL_WLONGVARCHAR:
		case SQL_DECIMAL:
		case SQL_NUMERIC:
		case SQL_SMALLINT:
		case SQL_INTEGER:
		case SQL_REAL:
		case SQL_FLOAT:
		case SQL_DOUBLE:
		case SQL_BIT:
		case SQL_TINYINT:
		case SQL_BIGINT:
		case SQL_TYPE_DATE:
		case SQL_TYPE_TIME:
			return TRUE;
		}
	}
	catch (CDBException* pEx)
	{
		OnODBCException (pEx);
		pEx->Delete ();
	}

	return FALSE;
}
//*****************************************************************************
CBCGPGridRow* CBCGPODBCGridCtrl::CreateVirtualRow (int nRow)
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

		for (short nCol = 0; nCol < (short) GetColumnCount (); nCol++)
		{
			if (!OnAddData (pRow, nCol, nRow))
			{
				ASSERT (FALSE);
				break;
			}
		}
	}
	catch (CDBException* pEx)
	{
		OnODBCException (pEx);
		pEx->Delete ();
	}

	return pRow;
}

#endif // BCGP_EXCLUDE_GRID_CTRL
