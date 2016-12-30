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

#if !defined(AFX_BCGFONTCOMBOBOX_H__36B2AFC4_0E1E_47DD_9C7B_CF90FB9CFF83__INCLUDED_)
#define AFX_BCGFONTCOMBOBOX_H__36B2AFC4_0E1E_47DD_9C7B_CF90FB9CFF83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGFontComboBox.h : header file
//

#include "BCGCBPro.h"
#include "BCGPComboBox.h"
#include "BCGPToolBarImages.h"

class CBCGPFontDesc;

/////////////////////////////////////////////////////////////////////////////
// CBCGPFontComboBox window

class BCGCBPRODLLEXPORT CBCGPFontComboBox : public CBCGPComboBox
{
	DECLARE_DYNAMIC(CBCGPFontComboBox)

// Construction
public:
	CBCGPFontComboBox();

// Attributes
public:
	static BOOL	m_bDrawUsingFont;

protected:
	CBCGPToolBarImages	m_Images;
	BOOL				m_bToolBarMode;

// Operations
public:
	BOOL Setup (int nFontType = DEVICE_FONTTYPE | RASTER_FONTTYPE | TRUETYPE_FONTTYPE,
				BYTE nCharSet = DEFAULT_CHARSET,
				BYTE nPitchAndFamily = DEFAULT_PITCH);
	BOOL SelectFont (CBCGPFontDesc* pDesc);
	BOOL SelectFont (LPCTSTR lpszName, BYTE nCharSet = DEFAULT_CHARSET);
	
	CBCGPFontDesc* GetSelFont () const;

protected:
	void Init ();
	void CleanUp ();

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPFontComboBox)
	public:
	virtual int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPFontComboBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPFontComboBox)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGFONTCOMBOBOX_H__36B2AFC4_0E1E_47DD_9C7B_CF90FB9CFF83__INCLUDED_)
