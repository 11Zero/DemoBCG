#if !defined(AFX_BCGHEADERCTRL_H__86C38AD2_4BAE_4C60_9FB6_B3F4D7103A47__INCLUDED_)
#define AFX_BCGHEADERCTRL_H__86C38AD2_4BAE_4C60_9FB6_B3F4D7103A47__INCLUDED_

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
// BCGHeaderCtrl.h : header file
//

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPHeaderCtrl window

class BCGCBPRODLLEXPORT CBCGPHeaderCtrl : public CHeaderCtrl
{
	DECLARE_DYNAMIC (CBCGPHeaderCtrl)

// Construction
public:
	CBCGPHeaderCtrl();

// Attributes
public:
	int GetSortColumn () const;
	BOOL IsAscending () const;

	int GetColumnState (int iColumn) const;
		// Returns: 0 - not not sorted, -1 - descending, 1 - ascending

	BOOL IsMultipleSort () const
	{
		return m_bMultipleSort;
	}

	BOOL IsDialogControl () const
	{
		return m_bIsDlgControl;
	}

	BOOL	m_bVisualManagerStyle;

protected:
	CMap<int,int,int,int>	m_mapColumnsStatus;	// -1, 1, 0

	BOOL	m_bIsMousePressed;
	BOOL	m_bDefaultDrawing;
	BOOL	m_bMultipleSort;
	BOOL	m_bAscending;

	int		m_nHighlightedItem;
	BOOL	m_bTracked;

	BOOL	m_bIsDlgControl;

	HFONT	m_hFont;

// Operations
public:
	void SetSortColumn (int iColumn, BOOL bAscending = TRUE, BOOL bAdd = FALSE);
	void RemoveSortColumn (int iColumn);
	void EnableMultipleSort (BOOL bEnable = TRUE);

protected:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPHeaderCtrl)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	virtual void OnDrawItem (CDC* pDC, int iItem, CRect rect, BOOL bIsPressed,
							BOOL bIsHighlighted);
	virtual void OnFillBackground (CDC* pDC);
	virtual void OnDrawSortArrow (CDC* pDC, CRect rectArrow);
	
	virtual void OnDraw(CDC* pDC);

// Implementation
public:
	virtual ~CBCGPHeaderCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPHeaderCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
	afx_msg LRESULT OnSetFont(WPARAM, LPARAM);
	afx_msg LRESULT OnBCGSetControlVMMode (WPARAM, LPARAM);
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()

	void CommonInit ();
	CFont* SelectFont (CDC *pDC);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGHEADERCTRL_H__86C38AD2_4BAE_4C60_9FB6_B3F4D7103A47__INCLUDED_)
