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
// BCGPRibbonDialogBar.h : header file
//

#if !defined(AFX_BCGPRIBBONDIALOGBAR_H__8E4A4D88_1757_4740_BB82_4538CB6E8036__INCLUDED_)
#define AFX_BCGPRIBBONDIALOGBAR_H__8E4A4D88_1757_4740_BB82_4538CB6E8036__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPDockingControlBar.h"
#include "BCGPRibbonBar.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonDialogBar window

class BCGCBPRODLLEXPORT CBCGPRibbonDialogBar : public CBCGPDockingControlBar
{
	DECLARE_DYNCREATE(CBCGPRibbonDialogBar)

// Construction
public:
	CBCGPRibbonDialogBar();
	virtual ~CBCGPRibbonDialogBar();

	BOOL Create(LPCTSTR lpszCaption, CWnd* pParentWnd, const RECT& rect, 
									UINT nImagesSmall,
									UINT nImagesLarge,
									BOOL bHasGripper, UINT nID, DWORD dwStyle, 
									DWORD dwTabbedStyle = CBRS_BCGP_REGULAR_TABS, 
									DWORD dwBCGStyle = dwDefaultBCGDockingBarStyle, 
									CCreateContext* pContext = NULL);

// Attributes
public:
	CBCGPRibbonBar& GetRibbon ()
	{
		return m_wndRibbon;
	}

// Operations
public:
	CBCGPRibbonPanel* AddPanel (LPCTSTR lpszPanelName, HICON hIcon = 0, int nRows = 3, CRuntimeClass* pRTI = NULL, BOOL bAutoDestroyIcon = TRUE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonDialogBar)
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPRibbonDialogBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


protected:
	CBCGPRibbonBar			m_wndRibbon;
	CBCGPRibbonCategory*	m_pCategory;
	UINT					m_nImagesSmall;
	UINT					m_nImagesLarge;
};

#endif // BCGP_EXCLUDE_RIBBON

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRIBBONDIALOGBAR_H__8E4A4D88_1757_4740_BB82_4538CB6E8036__INCLUDED_)
