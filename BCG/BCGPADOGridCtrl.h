#if !defined(AFX_BCGPADOGRIDCTRL_H__3C8A2ED9_8101_41B6_9EF6_67A42095F9AF__INCLUDED_)
#define AFX_BCGPADOGRIDCTRL_H__3C8A2ED9_8101_41B6_9EF6_67A42095F9AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// BCGPADOGridCtrl.h : header file
//

#include "BCGPDBGridCtrl.h"

#ifndef BCGP_EXCLUDE_GRID_CTRL

/////////////////////////////////////////////////////////////////////////////
// CBCGPADOGridCtrl window

#define BCGP_ADO_CHECK(x)				\
	{									\
		m_hresLastError = x;			\
		if (FAILED (m_hresLastError))	\
			return FALSE;				\
	}

#define BCGP_ADO_CHECK_UPDATE(x)		\
	{									\
		m_hresLastError = x;			\
		if (FAILED (m_hresLastError))	\
		{								\
			OnItemUpdateFailed();		\
			return;						\
		}								\
	}

class CBCGPADOGridCtrl : public CBCGPDBGridCtrl
{
// Construction
public:
	CBCGPADOGridCtrl()
	{
		m_hresLastError = S_OK;
	}

// Attributes
public:
	HRESULT	GetLastError () const
	{
		return m_hresLastError;
	}

protected:
	_ConnectionPtr	m_pConnection;
	_RecordsetPtr	m_pRecordSet;
	HRESULT			m_hresLastError;

// Overrides
public:
	virtual BOOL OpenDB (	LPCTSTR lpszConnectionStr,
							LPCTSTR lpszUserName = _T(""),
							LPCTSTR lpszPassword = _T(""),
							LPARAM  lOptions = 0)
		// lOptions should be ConnectModeEnum
	{
		m_hresLastError = S_OK;

		try
		{
			if (m_pConnection == NULL)
			{
				m_pConnection.CreateInstance (__uuidof (Connection));
			}

			if (m_pRecordSet == NULL)
			{
				m_pRecordSet.CreateInstance (__uuidof (Recordset));
			}

			Close ();

			BCGP_ADO_CHECK (m_pConnection->Open (	
					_bstr_t (lpszConnectionStr),
					_bstr_t (lpszUserName),
					_bstr_t (lpszPassword),
					lOptions == 0 ? adModeUnknown : (long) lOptions));
		}
		catch (_com_error& err)
		{
			m_hresLastError = err.Error ();

			OnADOException (err);
			return FALSE;
		}

		return TRUE;
	}
	
	virtual BOOL OpenMSAccessFile (	LPCTSTR lpszFileName, 
									LPCTSTR lpszUserName = _T(""),
									LPCTSTR lpszPassword = _T(""),
									LPARAM  lOptions = 0)
		// lOptions should be ConnectModeEnum
	{
		ASSERT (lpszFileName != NULL);

		CString strConnect;
		strConnect.Format (_T("Provider=Microsoft.Jet.OLEDB.4.0; Data Source = %s"),
			lpszFileName);

		return OpenDB (strConnect, lpszUserName, lpszPassword, lOptions);
	}

	virtual BOOL Close ()
	{
		m_hresLastError = S_OK;

		RemoveAll ();
		DeleteAllColumns ();

		try
		{
			if (m_pRecordSet != NULL && m_pRecordSet->GetState () == adStateOpen)
			{
				BCGP_ADO_CHECK (m_pRecordSet->Close ());
			}

			if (m_pConnection != NULL && m_pConnection->GetState () == adStateOpen)
			{
				BCGP_ADO_CHECK (m_pConnection->Close ());
			}
		}
		catch (_com_error& err)
		{
			m_hresLastError = err.Error ();

			OnADOException (err);
			return FALSE;
		}

		return TRUE;
	}

	virtual BOOL GetTableList (CStringList& lstTable)
	{
		m_hresLastError = S_OK;

		lstTable.RemoveAll ();

		try
		{
			if (m_pConnection == NULL || m_pConnection->GetState () != adStateOpen)
			{
				ASSERT (FALSE);
				return FALSE;
			}

			_RecordsetPtr rsTables = 
				m_pConnection->OpenSchema (adSchemaTables);
			
			while (!rsTables->ADOEOF)
			{
				CString strType = 
					(LPCTSTR) (_bstr_t) rsTables->Fields->GetItem (TEXT("TABLE_TYPE"))->Value;

				if (strType == _T("TABLE"))
				{
					_bstr_t bstrTableName = rsTables->Fields->GetItem (TEXT("TABLE_NAME"))->Value;
					lstTable.AddTail ((LPCTSTR) bstrTableName);
				}

				BCGP_ADO_CHECK (rsTables->MoveNext ());
			} 

			BCGP_ADO_CHECK (rsTables->Close ());
		}
		catch (_com_error& err)
		{
			m_hresLastError = err.Error ();

			OnADOException (err);
			return FALSE;
		}

		return TRUE;
	}

	virtual BOOL GetFieldList (LPCTSTR lpszTable, CStringList& lstField)
	{
		ASSERT (lpszTable != NULL);

		m_hresLastError = S_OK;

		CString strSQL = _T("SELECT * FROM ");
		strSQL += lpszTable;

		lstField.RemoveAll ();

		try
		{
			if (m_pConnection == NULL || m_pConnection->GetState () != adStateOpen)
			{
				ASSERT (FALSE);
				return FALSE;
			}

			_CommandPtr pCommand;
			pCommand.CreateInstance (__uuidof (Command));

			pCommand->ActiveConnection = m_pConnection;
			pCommand->CommandText = (LPCTSTR) strSQL;

			_RecordsetPtr pRecordSet (__uuidof (Recordset));

			pRecordSet->CursorLocation = adUseClient;

			BCGP_ADO_CHECK (pRecordSet->Open (
							(IDispatch*) pCommand, vtMissing,
							adOpenStatic, adLockUnspecified,
							adCmdUnknown));

			for (short i = 0; i < pRecordSet->Fields->Count; i++)
			{ 
				lstField.AddTail ((LPCTSTR) 
					pRecordSet->Fields->GetItem (_variant_t (i))->GetName ());
			}
		}
		catch (_com_error& err)
		{
			m_hresLastError = err.Error ();

			OnADOException (err);
			return FALSE;
		}

		return TRUE;
	}

	virtual BOOL GetFieldList (CStringList& lstField)
	{
		lstField.RemoveAll ();

		m_hresLastError = S_OK;

		try
		{
			if (m_pRecordSet == NULL)
			{
				ASSERT (FALSE);
				return FALSE;
			}

			for (short i = 0; i < m_pRecordSet->Fields->Count; i++)
			{ 
				lstField.AddTail ((LPCTSTR) 
					m_pRecordSet->Fields->GetItem (_variant_t (i))->GetName ());
			}
		}
		catch (_com_error& err)
		{
			m_hresLastError = err.Error ();

			OnADOException (err);
			return FALSE;
		}

		return TRUE;
	}

	virtual BOOL OpenTable (LPCTSTR lpszTable)
	{
		ASSERT (lpszTable != NULL);

		CString strSQL = _T("SELECT * FROM ");
		strSQL += lpszTable;

		return OpenSQL (strSQL);
	}

	virtual BOOL OpenSQL (LPCTSTR lpszSQL)
	{
		ASSERT (lpszSQL != NULL);

		m_hresLastError = S_OK;
		m_strSQL.Empty ();

		RemoveAll ();

		if (!m_bIsSorting)
		{
			DeleteAllColumns ();
		}

		try
		{
			if (m_pConnection == NULL || m_pRecordSet == NULL || 
				m_pConnection->GetState () != adStateOpen)
			{
				ASSERT (FALSE);
				return FALSE;
			}

			if (m_pRecordSet->GetState () == adStateOpen)
			{
				m_pRecordSet->Close ();
			}

			_CommandPtr pCommand;
			pCommand.CreateInstance (__uuidof (Command));

			pCommand->ActiveConnection = m_pConnection;
			pCommand->CommandText = lpszSQL;

			m_pRecordSet->CursorLocation = adUseClient;

			BCGP_ADO_CHECK (m_pRecordSet->Open ((
							IDispatch*) pCommand, vtMissing,
							adOpenStatic, adLockBatchOptimistic, 
							adCmdUnknown));

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
				SetVirtualRows (0);
				return TRUE;
			}

			//-------------
			// Add records:
			//-------------
			if (m_pRecordSet->ADOEOF)
			{
				// The table is empty
				SetVirtualRows (0);
				return TRUE;
			}

			if (m_bVirtualMode)
			{
				while (!m_pRecordSet->ADOEOF)
				{
					m_pRecordSet->MoveNext ();
				}

				SetVirtualRows (max (0, (int)m_pRecordSet->GetRecordCount ()));
			}
			else
			{
				for (int nRow = 0; !m_pRecordSet->ADOEOF;
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

			AdjustLayout ();
		}
		catch (_com_error& err)
		{
			m_hresLastError = err.Error ();

			OnADOException (err);
			return FALSE;
		}

		m_strSQL = lpszSQL;
		return TRUE;
	}

	virtual BOOL OnAddData (CBCGPGridRow* pRow, int nColumn, int /*nRow*/)
	{
		ASSERT_VALID (pRow);
		ASSERT (m_pRecordSet != NULL);
		ASSERT (m_pRecordSet->GetState () == adStateOpen);
		ASSERT (!m_pRecordSet->ADOEOF);

		m_hresLastError = S_OK;

		_variant_t varValue = 
			m_pRecordSet->Fields->GetItem (_variant_t ((short) nColumn))->GetValue ();

		switch (varValue.vt)
		{
		case VT_DATE:
			pRow->ReplaceItem (nColumn, new CBCGPGridDateTimeItem ((DATE) varValue), FALSE, TRUE);
			break;

		case VT_BOOL:
			pRow->ReplaceItem (nColumn, new CBCGPGridCheckItem ((bool) varValue), FALSE, TRUE);
			break;

		case VT_NULL:
		case VT_EMPTY:
			//--------------------------------------------------------------
			// Check the field type: if it's a string, pass an empty string:
			//--------------------------------------------------------------
			switch (m_pRecordSet->Fields->GetItem (
				_variant_t ((short) nColumn))->GetType ())
			{
			case adVarChar:
			case adLongVarChar:
			case adWChar:
			case adVarWChar:
			case adLongVarWChar:
				{
					pRow->GetItem (nColumn)->SetValue (_T(""), FALSE);
				}
			}
			break;

		case VT_BSTR:
			{
				CString str = (LPCTSTR)(_bstr_t) varValue;

				CString strTextToDisplay;
				CString strURL;

				if (ParseURLString (str, strTextToDisplay, strURL))
				{
					pRow->ReplaceItem (nColumn, new CBCGPGridURLItem (strTextToDisplay, strURL), FALSE, TRUE);
					break;
				}
			}

		default:
			pRow->GetItem (nColumn)->SetValue (varValue, FALSE);
		}

		return TRUE;
	}

	virtual void OnItemChanged (CBCGPGridItem* pItem, int nRow, int nColumn)
	{
		m_hresLastError = S_OK;

		try
		{
			ASSERT_VALID (this);

			if (m_pRecordSet == NULL)
			{
				ASSERT (FALSE);
				return;
			}

			ASSERT_VALID (pItem);

			CBCGPGridRow* pRow = pItem->GetParentRow ();
			if (pRow == NULL)
			{
				return;
			}

			ASSERT_VALID (pRow);

			BCGP_ADO_CHECK_UPDATE (m_pRecordSet->MoveFirst ());
			BCGP_ADO_CHECK_UPDATE (m_pRecordSet->Move (nRow));

			CString strURL;

			if (BuildURL (pItem, strURL))
			{
				m_pRecordSet->Fields->GetItem (_variant_t ((short) nColumn))->Value =
					(LPCTSTR) strURL;
			}
			else
			{
				m_pRecordSet->Fields->GetItem (_variant_t ((short) nColumn))->Value =
					pItem->GetValue ();
			}

			BCGP_ADO_CHECK_UPDATE (m_pRecordSet->Update ());
			BCGP_ADO_CHECK_UPDATE (m_pRecordSet->UpdateBatch (adAffectAll));
		}
		catch (_com_error& err)
		{
			m_hresLastError = err.Error ();
			OnItemUpdateFailed (err);
		}
	}

	virtual void OnADOException (_com_error& err)
	{
		AfxMessageBox (err.ErrorMessage ());
	}

	virtual void OnItemUpdateFailed ()
	{
		_com_error err (m_hresLastError);
		OnItemUpdateFailed (err);
	}

	virtual void OnItemUpdateFailed (_com_error& err)
	{
		AfxMessageBox (err.ErrorMessage ());
		CBCGPDBGridCtrl::OnItemUpdateFailed ();
	}

	virtual BOOL CanSortByColumn (int nColumn)
	{
		m_hresLastError = S_OK;

		try
		{
			if (m_pRecordSet == NULL)
			{
				ASSERT (FALSE);
				return FALSE;
			}

			switch (m_pRecordSet->Fields->GetItem (
				_variant_t ((short) nColumn))->GetType ())
			{
			case adTinyInt:
			case adSmallInt:
			case adInteger:
			case adBigInt:
			case adUnsignedTinyInt:
			case adUnsignedSmallInt:
			case adUnsignedInt:
			case adUnsignedBigInt:
			case adSingle:
			case adDouble:
			case adCurrency:
			case adDecimal:
			case adNumeric:
			case adBoolean:
			case adDate:
			case adDBDate:
			case adDBTime:
			case adDBTimeStamp:
			case adBSTR:
			case adChar:
			case adVarChar:
			case adLongVarChar:
			case adWChar:
			case adVarWChar:
			case adLongVarWChar:
				return TRUE;
			}
		}
		catch (_com_error& err)
		{
			m_hresLastError = err.Error ();

			OnADOException (err);
		}

		return FALSE;
	}

	virtual CBCGPGridRow* CreateVirtualRow (int nRow)
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
		catch (_com_error& err)
		{
			m_hresLastError = err.Error ();

			OnADOException (err);
		}

		return pRow;
	}

// Implementation
public:
	virtual ~CBCGPADOGridCtrl()
	{
		Close ();
	}
};

#endif	// BCGP_EXCLUDE_GRID_CTRL

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPADOGRIDCTRL_H__3C8A2ED9_8101_41B6_9EF6_67A42095F9AF__INCLUDED_)
