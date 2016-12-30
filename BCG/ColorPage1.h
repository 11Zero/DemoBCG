#if !defined(AFX_COLORPAGE1_H__9C4A9AB9_351E_4D5A_9548_7FA57A113573__INCLUDED_)
#define AFX_COLORPAGE1_H__9C4A9AB9_351E_4D5A_9548_7FA57A113573__INCLUDED_

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
// ColorPage1.h : header file
//

#include "BCGPPowerColorPicker.h"
#include "BCGPPropertyPage.h"

class CBCGPColorDialog;

/////////////////////////////////////////////////////////////////////////////
// CBCGPColorPage1 dialog

class CBCGPColorPage1 : public CBCGPPropertyPage
{
	friend class CBCGPColorDialog;

	DECLARE_DYNCREATE(CBCGPColorPage1)

// Construction
public:
	CBCGPColorPage1();

	CBCGPColorDialog*	m_pDialog;

// Dialog Data
	//{{AFX_DATA(CBCGPColorPage1)
	enum { IDD = IDD_BCGBARRES_COLOR_PAGE_ONE };
	CBCGPColorPickerCtrl	m_hexpicker;
	CBCGPColorPickerCtrl	m_hexpicker_greyscale;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CBCGPColorPage1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	friend void hexfunc(COLORREF ref,DWORD ClientData);
	friend void hexfunc(COLORREF ref,void *ClientData);
	
	friend void hexgreyscalefunc(COLORREF ref,DWORD ClientData);
	friend void hexgreyscalefunc(COLORREF ref,void *ClientData);

	// Generated message map functions
	//{{AFX_MSG(CBCGPColorPage1)
	virtual BOOL OnInitDialog();
	afx_msg void OnGreyscale();
	afx_msg void OnHexColor();
	afx_msg void OnDoubleClickedColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORPAGE1_H__9C4A9AB9_351E_4D5A_9548_7FA57A113573__INCLUDED_)
