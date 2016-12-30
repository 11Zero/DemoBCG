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

#if !defined(AFX_BCGPEXCHECKLIST_H__A2154114_301B_11D2_8BE6_00A0C9B05590__INCLUDED_)
#define AFX_BCGPEXCHECKLIST_H__A2154114_301B_11D2_8BE6_00A0C9B05590__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BCGExCheckList.h : header file
//
#include "BCGCBPro.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPExCheckList window

class BCGCBPRODLLEXPORT CBCGPExCheckList : public CCheckListBox
{
// Construction
public:
	CBCGPExCheckList();

// Attributes
protected:
	CArray<BOOL, BOOL>	m_arCheckData;

// Operations
public:
	void EnableCheck (int nIndex, BOOL bEnable = TRUE);
	BOOL IsCheckEnabled (int nIndex) const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPExCheckList)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPExCheckList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPExCheckList)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	LRESULT OnLBAddString(WPARAM wParam, LPARAM lParam);
	LRESULT OnLBInsertString(WPARAM wParam, LPARAM lParam);
	LRESULT OnLBResetContent(WPARAM wParam, LPARAM lParam);
	LRESULT OnLBDeleteString(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

	void OnNewString (int iIndex);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPEXCHECKLIST_H__A2154114_301B_11D2_8BE6_00A0C9B05590__INCLUDED_)
