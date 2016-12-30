#if !defined(AFX_BCGPDAOGRIDCTRL_H__64742711_1718_4369_BEF8_5F1BDA411AE1__INCLUDED_)
#define AFX_BCGPDAOGRIDCTRL_H__64742711_1718_4369_BEF8_5F1BDA411AE1__INCLUDED_

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
// BCGPDAOGridCtrl.h : header file
//
#ifndef _WIN64

#include <afxdao.h>			// MFC DAO database classes
#include "BCGPDBGridCtrl.h"

#ifndef BCGP_EXCLUDE_GRID_CTRL

#pragma warning (disable : 4995)

/////////////////////////////////////////////////////////////////////////////
// CBCGPDAOGridCtrl window

class BCGCBPRODLLEXPORT CBCGPDAOGridCtrl : public CBCGPDBGridCtrl
{
	DECLARE_DYNCREATE(CBCGPDAOGridCtrl)

// Construction
public:
	CBCGPDAOGridCtrl();

// Attributes
public:
	const CDaoRecordset* GetRecordSet () const
	{
		return m_pRecordSet;
	}

	const CDaoDatabase* GetDataBase () const
	{
		return m_pDataBase;
	}
	
protected:
	CDaoRecordset* m_pRecordSet;
	CDaoDatabase*  m_pDataBase;

// Overrides
public:
	virtual BOOL OpenDB (	LPCTSTR lpszConnectionStr,
							LPCTSTR lpszUserName = _T(""),
							LPCTSTR lpszPassword = _T(""),
							LPARAM  lOptions = 0);
		// 'lOptions' should be as 'dwOptions' in CDaoDatabase::OpenEx
	
	virtual BOOL OpenMSAccessFile (	LPCTSTR lpszFileName, 
									LPCTSTR lpszUserName = _T(""),
									LPCTSTR lpszPassword = _T(""),
									LPARAM  lOptions = 0);
		// 'lOptions' should be as 'dwOptions' in CDaoDatabase::OpenEx

	virtual BOOL Close ();

	virtual BOOL GetTableList (CStringList& lstTable);
	virtual BOOL GetFieldList (LPCTSTR lpszTable, CStringList& lstField);
	virtual BOOL GetFieldList (CStringList& lstField);

	virtual BOOL OpenTable (LPCTSTR lpszTable);
	virtual BOOL OpenSQL (LPCTSTR lpszSQL);

	virtual BOOL OnAddData (CBCGPGridRow* pRow, int nColumn, int nRow);

	virtual void OnItemChanged (CBCGPGridItem* pItem, int nRow, int nColumn);

	virtual void OnDaoException (CDaoException* pEx)
	{
		pEx->ReportError ();
	}

	virtual BOOL CanSortByColumn (int nColumn);
	virtual CBCGPGridRow* CreateVirtualRow (int nRow);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPDAOGridCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPDAOGridCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPDAOGridCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // BCGP_EXCLUDE_GRID_CTRL

#endif // !_WIN64

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#pragma warning (default : 4995)

#endif // !defined(AFX_BCGPDAOGRIDCTRL_H__64742711_1718_4369_BEF8_5F1BDA411AE1__INCLUDED_)
