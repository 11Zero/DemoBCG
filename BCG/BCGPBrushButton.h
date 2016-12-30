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
// BCGPBrushButton.h : header file
//

#if !defined(AFX_BCGPBRUSHBUTTON_H__C35A95E8_ECF9_4E15_BEA6_40E1149A9B17__INCLUDED_)
#define AFX_BCGPBRUSHBUTTON_H__C35A95E8_ECF9_4E15_BEA6_40E1149A9B17__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPButton.h"
#include "BCGPGraphicsManager.h"
#include "bcgprores.h"
#include "BCGPEditBrushDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPBrushButton window

class BCGCBPRODLLEXPORT CBCGPBrushButton : public CBCGPButton
{
	DECLARE_DYNAMIC(CBCGPBrushButton)

// Construction
public:
	CBCGPBrushButton();

// Attributes
public:
	const CBCGPBrush& GetBrush() const
	{
		return m_Brush;
	}

	void SetBrush(const CBCGPBrush& brush);

	CBCGPEditBrushOptions	m_Options;

// Operations
protected:
	void UpdateBitmap(BOOL bRedraw);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPBrushButton)
	//}}AFX_VIRTUAL
	virtual void OnDrawFocusRect (CDC* pDC, const CRect& rectClient);
	virtual void OnDraw (CDC* pDC, const CRect& rect, UINT uiState);

// Implementation
public:
	virtual ~CBCGPBrushButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPBrushButton)
	afx_msg BOOL OnClicked();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBCGPBrush	m_Brush;
	BOOL		m_bInCommand;
	BOOL		m_bIsFirstDraw;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPBRUSHBUTTON_H__C35A95E8_ECF9_4E15_BEA6_40E1149A9B17__INCLUDED_)
