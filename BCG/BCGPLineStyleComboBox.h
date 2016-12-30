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
// BCGPLineStyleComboBox.h : header file
//

#if !defined(AFX_BCGPLINESTYLECOMBOBOX_H__BE667EB6_145D_46B1_B936_451FC718364D__INCLUDED_)
#define AFX_BCGPLINESTYLECOMBOBOX_H__BE667EB6_145D_46B1_B936_451FC718364D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPComboBox.h"

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
#include "BCGPToolBarImages.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPLineStyleComboBox window

class BCGCBPRODLLEXPORT CBCGPLineStyleComboBox : public CBCGPComboBox
{
	DECLARE_DYNAMIC(CBCGPLineStyleComboBox)

// Construction
public:
	CBCGPLineStyleComboBox();

// Attributes
public:
	BOOL m_bAutoFillItems;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPLineStyleComboBox)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPLineStyleComboBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPLineStyleComboBox)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSysColorChange();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	void Init();
	void PrepareImages();

	CBCGPToolBarImages	m_imageListStyles;
	CBCGPToolBarImages	m_imageListStylesSelected;
	CBCGPToolBarImages	m_imageListStylesDisabled;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPLINESTYLECOMBOBOX_H__BE667EB6_145D_46B1_B936_451FC718364D__INCLUDED_)
