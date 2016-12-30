#if !defined(AFX_BCGPSPINBUTTONCTRL_H__07135C19_9492_47DD_B0C1_AF8C36D4EBAC__INCLUDED_)
#define AFX_BCGPSPINBUTTONCTRL_H__07135C19_9492_47DD_B0C1_AF8C36D4EBAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPSpinButtonCtrl.h : header file
//

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
/////////////////////////////////////////////////////////////////////////////
// CBCGPSpinButtonCtrl window

#include "BCGCBPro.h"

class BCGCBPRODLLEXPORT CBCGPSpinButtonCtrl : public CSpinButtonCtrl
{
	DECLARE_DYNAMIC(CBCGPSpinButtonCtrl)

// Construction
public:
	CBCGPSpinButtonCtrl();

// Attributes
public:
	BOOL	m_bOnGlass;
	BOOL	m_bVisualManagerStyle;
	BOOL	m_bIsDateTimeControl;

protected:
	BOOL	m_bIsButtonPressedUp;
	BOOL	m_bIsButtonPressedDown;

	BOOL	m_bIsButtonHighligtedUp;
	BOOL	m_bIsButtonHighligtedDown;

	BOOL	m_bTracked;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPSpinButtonCtrl)
	//}}AFX_VIRTUAL
	virtual void OnDraw (CDC* pDC);
	virtual BOOL IsRibbonControl () const
	{
		return FALSE;
	}

// Implementation
public:
	virtual ~CBCGPSpinButtonCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPSpinButtonCtrl)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	afx_msg LRESULT OnBCGSetControlVMMode (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGSetControlAero (WPARAM, LPARAM);
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPSPINBUTTONCTRL_H__07135C19_9492_47DD_B0C1_AF8C36D4EBAC__INCLUDED_)
