#if !defined(AFX_BCGPODBCGRIDCTRL_H__C1682894_119A_4E19_B8CF_54A43B685FA0__INCLUDED_)
#define AFX_BCGPODBCGRIDCTRL_H__C1682894_119A_4E19_B8CF_54A43B685FA0__INCLUDED_

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
// BCGPODBCGridCtrl.h : header file
//

#include <afxdb.h>			// MFC ODBC database classes
#include "BCGPDBGridCtrl.h"

#ifndef BCGP_EXCLUDE_GRID_CTRL

/////////////////////////////////////////////////////////////////////////////
// CBCGPODBCGridCtrl window

class BCGCBPRODLLEXPORT CBCGPODBCGridCtrl : public CBCGPDBGridCtrl
{
	DECLARE_DYNCREATE(CBCGPODBCGridCtrl)

// Construction
public:
	CBCGPODBCGridCtrl();

// Attributes
public:
	const CRecordset* GetRecordSet () const
	{
		return m_pRecordSet;
	}

	const CDatabase* GetDataBase () const
	{
		return m_pDataBase;
	}
	
protected:
	CRecordset* m_pRecordSet;
	CDatabase*  m_pDataBase;

// Overrides
public:
	virtual BOOL OpenDB (	LPCTSTR lpszConnectionStr,
							LPCTSTR lpszUserName = _T(""),
							LPCTSTR lpszPassword = _T(""),
							LPARAM  lOptions = 0);
		// 'lOptions' should be as 'dwOptions' in CDatabase::OpenEx
	
	virtual BOOL OpenMSAccessFile (	LPCTSTR lpszFileName, 
									LPCTSTR lpszUserName = _T(""),
									LPCTSTR lpszPassword = _T(""),
									LPARAM  lOptions = 0);
		// 'lOptions' should be as 'dwOptions' in CDatabase::OpenEx

	virtual BOOL Close ();

	virtual BOOL GetTableList (CStringList& lstTable);

	virtual BOOL GetFieldList (LPCTSTR lpszTable, CStringList& lstField);
	virtual BOOL GetFieldList (CStringList& lstField);

	virtual BOOL OpenTable (LPCTSTR lpszTable);
	virtual BOOL OpenSQL (LPCTSTR lpszSQL);

	virtual BOOL OnAddData (CBCGPGridRow* pRow, int nColumn, int nRow);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPODBCGridCtrl)
	//}}AFX_VIRTUAL

protected:
	virtual BOOL SetFieldData (CDBVariant& varValue, CBCGPGridRow* pRow, 
								   int nColumn, int nRow);

	virtual void OnItemChanged (CBCGPGridItem* pItem, int nRow, int nColumn);

	virtual BOOL CanSortByColumn (int nColumn);

	virtual void OnODBCException (CDBException* pEx)
	{
		pEx->ReportError ();
	}

	virtual CBCGPGridRow* CreateVirtualRow (int nRowIndex);

// Implementation
public:
	virtual ~CBCGPODBCGridCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPODBCGridCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // BCGP_EXCLUDE_GRID_CTRL

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPODBCGRIDCTRL_H__C1682894_119A_4E19_B8CF_54A43B685FA0__INCLUDED_)
