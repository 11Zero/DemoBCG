#if !defined(AFX_PROPERTYSHEETCTRL_H__FFE5BC0C_9181_4E1A_AF3D_24389930E42C__INCLUDED_)
#define AFX_PROPERTYSHEETCTRL_H__FFE5BC0C_9181_4E1A_AF3D_24389930E42C__INCLUDED_

// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertySheetCtrl.h : header file
//

#include "bcgcbpro.h"
#include "BCGPPropertySheet.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropertySheetCtrl

class BCGCBPRODLLEXPORT CBCGPPropertySheetCtrl : public CBCGPPropertySheet
{
	DECLARE_DYNAMIC(CBCGPPropertySheetCtrl)

// Construction
public:
	CBCGPPropertySheetCtrl(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CBCGPPropertySheetCtrl(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CBCGPPropertySheetCtrl();

// Attributes
public:
	BOOL m_bIsAutoDestroy;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPPropertySheetCtrl)
	public:
	virtual BOOL OnInitDialog();
	protected:
	virtual void PostNcDestroy();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPPropertySheetCtrl();

	void    LoadAcceleratorTable(UINT nAccelTableID = 0);

protected:
	HACCEL               m_hAccel;

	void ResizeControl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPPropertySheetCtrl)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTYSHEETCTRL_H__FFE5BC0C_9181_4E1A_AF3D_24389930E42C__INCLUDED_)
