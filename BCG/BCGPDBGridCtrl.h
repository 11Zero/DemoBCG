#if !defined(AFX_BCGPDBGRIDCTRL_H__8A85BD85_5438_4E86_9A32_24A9B2B74272__INCLUDED_)
#define AFX_BCGPDBGRIDCTRL_H__8A85BD85_5438_4E86_9A32_24A9B2B74272__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// BCGPDBGridCtrl.h : header file
//

#include "BCGPGridCtrl.h"

#ifndef BCGP_EXCLUDE_GRID_CTRL

/////////////////////////////////////////////////////////////////////////////
// CBCGPDBGridCtrl window

class BCGCBPRODLLEXPORT CBCGPDBGridCtrl : public CBCGPGridCtrl
{
	DECLARE_DYNAMIC(CBCGPDBGridCtrl)

// Construction
public:
	CBCGPDBGridCtrl();

// Operations
	void EnableVirtualMode ();	// Cannot be disabled

// Attributes:
public:
	BOOL IsDbSort () const
	{
		return m_bDbSort;
	}

	void SetDbSort (BOOL bSet = TRUE)
	{
		m_bDbSort = bSet;
	}

protected:
	CString	m_strSQL;		// Need to be set in derived classes in OpenSQL
	BOOL	m_bDbSort;		// Use 'ORDER BY' for sorting
	BOOL	m_bIsSorting;	// Inside sorting mode

	CStringList m_lstURLPrefixes;	// http:, ...

// Overrides
public:
	virtual BOOL OpenDB (	LPCTSTR lpszConnectionStr,
							LPCTSTR lpszUserName = _T(""),
							LPCTSTR lpszPassword = _T(""),
							LPARAM  lOptions = 0) = 0;
	
	virtual BOOL OpenMSAccessFile (	LPCTSTR lpszFileName, 
									LPCTSTR lpszUserName = _T(""),
									LPCTSTR lpszPassword = _T(""),
									LPARAM  lOptions = 0) = 0;

	virtual BOOL Close () = 0;
	virtual BOOL GetTableList (CStringList& lstTable) = 0;
	virtual BOOL GetFieldList (LPCTSTR lpszTable, CStringList& lstField) = 0;
	virtual BOOL GetFieldList (CStringList& lstField) = 0;
	virtual BOOL OpenTable (LPCTSTR lpszTable) = 0;
	virtual BOOL OpenSQL (LPCTSTR lpszSQL) = 0;
	virtual BOOL OnAddData (CBCGPGridRow* pRow, int nColumn, int nRow) = 0;

	virtual BOOL ParseURLString (	CString str, 
							CString& strTextToDisplay,
							CString& strURL);
	virtual BOOL BuildURL (CBCGPGridItem* pItem, CString& strResult);

	virtual void Sort (int nColumn, BOOL bAscending = TRUE, BOOL bAdd = FALSE);
	virtual BOOL IsSortingMode () const
	{
		return !m_bDbSort;
	}

	virtual BOOL CanSortByColumn (int nColumn) = 0;

	virtual BOOL OnBeforeAddRow (CBCGPGridRow* /*pRow*/, int /*nRow*/)	{	return TRUE;	}

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPDBGridCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPDBGridCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPDBGridCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	static BOOL CALLBACK GridCallback (BCGPGRID_DISPINFO* pdi, LPARAM lp);
};

/////////////////////////////////////////////////////////////////////////////

#endif // BCGP_EXCLUDE_GRID_CTRL

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPDBGRIDCTRL_H__8A85BD85_5438_4E86_9A32_24A9B2B74272__INCLUDED_)
