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
// BCGPInplaceEdit.h : header file
//

#if !defined(AFX_BCGPINPLACEEDIT_H__1CBAAE00_773F_4451_84C2_E5EDAEC34782__INCLUDED_)
#define AFX_BCGPINPLACEEDIT_H__1CBAAE00_773F_4451_84C2_E5EDAEC34782__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// CBCGPInplaceEdit window

class CBCGPInplaceEdit : public CEdit
{
public:
	enum VertAlign { VA_Top, VA_Center, VA_Bottom };

// Construction
public:
	CBCGPInplaceEdit();

// Attributes
public:
	BOOL	m_bResizeEditor;// if TRUE, editor changes its height and width on content changing
	BOOL	m_bWrapText;	// if FALSE editor enlarges its width to fit text, if TRUE - the width is fixed

	CRect	m_rectInital;	// Initial bounding rect of the editor
	int		m_nTopMargin;	// Top margin for vertical alignment
	CRect	m_rectEditMax;	// Maximal size of the editor

	int		m_nVStep;
	int		m_nHLeftStep;
	int		m_nHRightStep;
	VertAlign m_VertAlign;	// VA_Top, VA_Center, VA_Bottom

	//CBCGPGridCtrl* m_pGridCtrl; // Owner grid control

// Operations
public:
	void ResizeEditor ();
	BOOL CanEnlargeHeight () const { return m_bResizeEditor;}
	BOOL CanEnlargeWidth () const { return m_bResizeEditor && !m_bWrapText;}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPInplaceEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPInplaceEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPInplaceEdit)
	afx_msg void OnUpdateR();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	afx_msg LRESULT OnSetText (WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPINPLACEEDIT_H__1CBAAE00_773F_4451_84C2_E5EDAEC34782__INCLUDED_)
