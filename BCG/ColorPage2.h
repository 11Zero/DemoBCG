#if !defined(AFX_COLORPAGE2_H__CEF11377_7CBE_4D0A_BDF3_792C35B4AE5E__INCLUDED_)
#define AFX_COLORPAGE2_H__CEF11377_7CBE_4D0A_BDF3_792C35B4AE5E__INCLUDED_

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
// ColorPage2.h : header file
//

#include "BCGPPowerColorPicker.h"
#include "BCGPPropertyPage.h"

class CBCGPColorDialog;

/////////////////////////////////////////////////////////////////////////////
// CBCGPColorPage2 dialog

class CBCGPColorPage2 : public CBCGPPropertyPage
{
	DECLARE_DYNCREATE(CBCGPColorPage2)

// Construction
public:
	CBCGPColorPage2();
	CBCGPColorDialog*	m_pDialog;

	void Setup (BYTE R, BYTE G, BYTE B);

protected:
// Dialog Data
	//{{AFX_DATA(CBCGPColorPage2)
	enum { IDD = IDD_BCGBARRES_COLORPAGE_TWO };
	CBCGPColorPickerCtrl	m_wndLuminance;
	CBCGPColorPickerCtrl	m_wndColorPicker;
	int		m_r;
	int		m_b;
	int		m_g;
	UINT	m_l;
	UINT	m_h;
	UINT	m_s;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CBCGPColorPage2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBCGPColorPage2)
	virtual BOOL OnInitDialog();
	afx_msg void OnRGBChanged();
	afx_msg void OnHLSChanged();
	afx_msg void OnLuminance();
	afx_msg void OnColour();
	afx_msg void OnDoubleClickedColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL	m_bIsReady;
	BOOL	m_bInUpdate;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORPAGE2_H__CEF11377_7CBE_4D0A_BDF3_792C35B4AE5E__INCLUDED_)
