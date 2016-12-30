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

#if !defined(AFX_BCGPTABBEDTOOLBAR_H__789773C7_26CC_4BB7_9929_07C5ADB66A6D__INCLUDED_)
#define AFX_BCGPTABBEDTOOLBAR_H__789773C7_26CC_4BB7_9929_07C5ADB66A6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPTabbedToolbar.h : header file
//

#include "BCGCBPro.h"
#include "BCGPToolBar.h"

class CBCGPTabWnd;

/////////////////////////////////////////////////////////////////////////////
// CBCGPTabbedToolbar window

class BCGCBPRODLLEXPORT CBCGPTabbedToolbar : public CBCGPToolBar
{
	DECLARE_SERIAL(CBCGPTabbedToolbar)
// Construction
public:
	CBCGPTabbedToolbar();

	virtual BOOL Create  (CWnd* pParentWnd, DWORD dwStyle = dwDefaultToolbarStyle,
						 UINT nID = AFX_IDW_TOOLBAR, DWORD dwBCGStyle = CBRS_BCGP_FLOAT);

// Attributes
public:

// Operations
public:
	virtual BOOL AddTab (CBCGPControlBar* pNewBar, BOOL bVisible = TRUE, 
					BOOL bSetActive = TRUE, BOOL bDetachable = TRUE);

	virtual CBCGPTabWnd* GetUnderlinedWindow () {return m_pTabWnd;}
	virtual void RecalcLayout ();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPTabbedToolbar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPTabbedToolbar();

	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPTabbedToolbar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcDestroy();
	//}}AFX_MSG
	afx_msg LRESULT OnChangeActiveTab(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()

	CBCGPTabWnd*	m_pTabWnd;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPTABBEDTOOLBAR_H__789773C7_26CC_4BB7_9929_07C5ADB66A6D__INCLUDED_)
