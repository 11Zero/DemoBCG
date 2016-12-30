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
// BCGPTreeCtrl.h : header file
//

#if !defined(AFX_BCGPTREECTRL_H__2341CBB2_8676_40E1_BAD8_F4D8F893656F__INCLUDED_)
#define AFX_BCGPTREECTRL_H__2341CBB2_8676_40E1_BAD8_F4D8F893656F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPTreeCtrl window

#define _NEW_MEMBERS

#ifndef TVM_SETLINECOLOR
#define TVM_SETLINECOLOR            (TV_FIRST + 40)
#endif
#ifndef TreeView_SetLineColor
#define TreeView_SetLineColor(hwnd, clr) \
    (COLORREF)SNDMSG((hwnd), TVM_SETLINECOLOR, 0, (LPARAM)(clr))
#endif

#ifndef TVM_GETLINECOLOR
#define TVM_GETLINECOLOR            (TV_FIRST + 41)
#endif

#ifndef TreeView_GetLineColor
#define TreeView_GetLineColor(hwnd) \
    (COLORREF)SNDMSG((hwnd), TVM_GETLINECOLOR, 0, 0)
#endif

class BCGCBPRODLLEXPORT CBCGPTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CBCGPTreeCtrl)

// Construction
public:
	CBCGPTreeCtrl();

// Attributes
public:
	BOOL m_bVisualManagerStyle;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPTreeCtrl)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPTreeCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPTreeCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcPaint();
	//}}AFX_MSG
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnBCGSetControlVMMode (WPARAM, LPARAM);
	afx_msg LRESULT OnChangeVisualManager (WPARAM, LPARAM);
	afx_msg LRESULT OnPrint(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPTREECTRL_H__2341CBB2_8676_40E1_BAD8_F4D8F893656F__INCLUDED_)
