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
// BCGPComboBox.h : header file
//

#if !defined(AFX_BCGPCOMBOBOX_H__B809A10B_3085_419E_8ADA_6AA9A852CA73__INCLUDED_)
#define AFX_BCGPCOMBOBOX_H__B809A10B_3085_419E_8ADA_6AA9A852CA73__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPComboBox window

class BCGCBPRODLLEXPORT CBCGPComboBox : public CComboBox
{
	DECLARE_DYNAMIC(CBCGPComboBox)

// Construction
public:
	CBCGPComboBox();

// Attributes
public:
	BOOL		m_bOnGlass;
	BOOL		m_bVisualManagerStyle;

protected:
	CBCGPEdit	m_wndEdit;
	BOOL		m_bIsDroppedDown;
	CRect		m_rectBtn;
	BOOL		m_bIsButtonHighlighted;
	BOOL		m_bTracked;
	CString		m_strPrompt;
	COLORREF	m_clrPrompt;
	CString		m_strErrorMessage;
	COLORREF	m_clrErrorText;
	BOOL		m_bDefaultPrintClient;

// Operations
public:
	void SetPrompt(LPCTSTR lpszPrompt, COLORREF clrText = (COLORREF)-1, BOOL bRedraw = TRUE);
	CString GetPrompt() const
	{
		return m_strPrompt;
	};

	void SetErrorMessage(LPCTSTR lpszMessage, COLORREF clrText = (COLORREF)-1, BOOL bRedraw = TRUE);
	CString GetErrorMessage() const
	{
		return m_strErrorMessage;
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPComboBox)
	protected:
	virtual void PreSubclassWindow();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	virtual void OnDraw(CDC* pDC, BOOL bDrawPrompt);

// Implementation
public:
	virtual ~CBCGPComboBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPComboBox)
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	afx_msg BOOL OnEditupdate();
	afx_msg BOOL OnSelchange();
	afx_msg BOOL OnCloseup();
	afx_msg BOOL OnDropdown();
	afx_msg LRESULT OnBCGSetControlVMMode (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGSetControlAero (WPARAM, LPARAM);
	afx_msg LRESULT OnSetText (WPARAM, LPARAM);
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()

	void SubclassEditBox();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPCOMBOBOX_H__B809A10B_3085_419E_8ADA_6AA9A852CA73__INCLUDED_)
