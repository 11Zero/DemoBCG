#if !defined(AFX_BCGPGROUP_H__917CCAFD_2ED2_44EC_8297_0C3A5C62F28F__INCLUDED_)
#define AFX_BCGPGROUP_H__917CCAFD_2ED2_44EC_8297_0C3A5C62F28F__INCLUDED_

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
// BCGPGroup.h : header file
//

#include "BCGCBPro.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPGroup window

class BCGCBPRODLLEXPORT CBCGPGroup : public CButton
{
	DECLARE_DYNAMIC(CBCGPGroup)

// Construction
public:
	CBCGPGroup();

// Attributes
public:
	BOOL	m_bOnGlass;
	BOOL	m_bVisualManagerStyle;
	HFONT	m_hFont;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPGroup)
	//}}AFX_VIRTUAL

	virtual void OnDraw(CDC* pDC);

// Implementation
public:
	virtual ~CBCGPGroup();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPGroup)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnEnable(BOOL bEnable);
	//}}AFX_MSG
	afx_msg LRESULT OnBCGSetControlVMMode (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGSetControlAero (WPARAM, LPARAM);
	afx_msg LRESULT OnUpdateUIState (WPARAM, LPARAM);
	afx_msg LRESULT OnSetText (WPARAM, LPARAM lp);
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnSetFont (WPARAM wParam, LPARAM lp);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPGROUP_H__917CCAFD_2ED2_44EC_8297_0C3A5C62F28F__INCLUDED_)
