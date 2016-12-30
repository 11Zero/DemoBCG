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

#if !defined(AFX_BCGPDOCKINGCBWRAPPER_H__D6D4A26B_CFCB_4977_84E8_A94C4E2E92C3__INCLUDED_)
#define AFX_BCGPDOCKINGCBWRAPPER_H__D6D4A26B_CFCB_4977_84E8_A94C4E2E92C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPDockingCBWrapper.h : header file
//

#include "BCGCBPro.h"
#include "BCGPDockingControlBar.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPDockingCBWrapper window

class BCGCBPRODLLEXPORT CBCGPDockingCBWrapper : public CBCGPDockingControlBar
{
	DECLARE_SERIAL(CBCGPDockingCBWrapper);
// Construction
public:
	CBCGPDockingCBWrapper();

// Attributes
public:
	CRect	m_rectInitial;
	DWORD	m_dwEnabledAlignmentInitial;

// Operations
public:
	virtual BOOL	SetWrappedWnd (CWnd* pWnd);
	virtual CWnd*	GetWrappedWnd () const {return m_pWnd;}

	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPDockingCBWrapper)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPDockingCBWrapper();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPDockingCBWrapper)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	CWnd*	m_pWnd;
	
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPDOCKINGCBWRAPPER_H__D6D4A26B_CFCB_4977_84E8_A94C4E2E92C3__INCLUDED_)
