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

#if !defined(AFX_BCGPINPLACETOOLTIPCTRL_H__124C1BA5_A2BA_4419_ADDE_35845BBA31D9__INCLUDED_)
#define AFX_BCGPINPLACETOOLTIPCTRL_H__124C1BA5_A2BA_4419_ADDE_35845BBA31D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPInplaceToolTipCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBCGPInplaceToolTipCtrl window

class BCGCBPRODLLEXPORT CBCGPInplaceToolTipCtrl : public CWnd
{
	DECLARE_DYNAMIC(CBCGPInplaceToolTipCtrl)

// Construction
public:
	CBCGPInplaceToolTipCtrl();

// Attributes
public:
	void SetTextMargin (int nTextMargin)
	{
		m_nTextMargin = nTextMargin;
	}

	void GetLastRect (CRect& rect) const
	{
		rect = m_rectLast;
	}

	void SetMultiline (BOOL bMultiline)
	{
		m_bMultiline = bMultiline;
	}

	BOOL IsMultiline () const
	{
		return m_bMultiline;
	}

protected:
	static CString	m_strClassName;
	CString			m_strText;
	CRect			m_rectLast;
	int				m_nTextMargin;
	CFont*			m_pFont;
	CWnd*			m_pWndParent;
	BOOL			m_bMultiline;

// Operations
public:
	void Track (CRect rect, const CString& strText);
	void Hide ();
	void Deactivate ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPInplaceToolTipCtrl)
	public:
	virtual BOOL Create(CWnd* pWndParent = NULL);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPInplaceToolTipCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPInplaceToolTipCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	afx_msg LRESULT OnSetFont (WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPINPLACETOOLTIPCTRL_H__124C1BA5_A2BA_4419_ADDE_35845BBA31D9__INCLUDED_)
