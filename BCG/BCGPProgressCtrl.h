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
// BCGPProgressCtrl.h : header file
//

#if !defined(AFX_BCGPPROGRESSCTRL_H__23E0C43A_6498_403C_AB8F_7A395D9C2FA4__INCLUDED_)
#define AFX_BCGPPROGRESSCTRL_H__23E0C43A_6498_403C_AB8F_7A395D9C2FA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPProgressCtrl window

class BCGCBPRODLLEXPORT CBCGPProgressCtrl : public CProgressCtrl
{
	DECLARE_DYNAMIC(CBCGPProgressCtrl)

// Construction
public:
	CBCGPProgressCtrl();

// Attributes
public:
	BOOL	m_bOnGlass;
	BOOL	m_bVisualManagerStyle;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPProgressCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPProgressCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPProgressCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	//}}AFX_MSG
	afx_msg LRESULT OnBCGSetControlVMMode (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGSetControlAero (WPARAM, LPARAM);
	afx_msg LRESULT OnSetMarquee(WPARAM, LPARAM);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnPrintClient(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

	void DoPaint(CDC* pDC);

	int	m_nMarqueeStep;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPROGRESSCTRL_H__23E0C43A_6498_403C_AB8F_7A395D9C2FA4__INCLUDED_)
