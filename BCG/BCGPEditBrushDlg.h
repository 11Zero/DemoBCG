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

#if !defined(AFX_BCGPEDITBRUSHDLG_H__C7A61B10_5A8A_45A8_B37A_45DEEE693204__INCLUDED_)
#define AFX_BCGPEDITBRUSHDLG_H__C7A61B10_5A8A_45A8_B37A_45DEEE693204__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPEditBrushDlg.h : header file
//

#include "BCGCBPro.h"
#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
#include "BCGPColorButton.h"
#endif
#include "BCGPDialog.h"
#include "BCGPButton.h"
#include "BCGPSliderCtrl.h"
#include "BCGPGraphicsManager.h"
#include "BCGPStatic.h"

class BCGCBPRODLLEXPORT CBCGBrushPreviewCtrl : public CBCGPStatic
{
	friend class CBCGPEditBrushDlg;

public:
	CBCGBrushPreviewCtrl()
	{
		m_pGM = NULL;
		m_pBrush = NULL;
		m_brBack.CreateHatchBrush(HS_DIAGCROSS, RGB (192, 192, 192));
	}

	virtual ~CBCGBrushPreviewCtrl()
	{
		if (m_pGM != NULL)
		{
			delete m_pGM;
		}
	}

protected:
	// Generated message map functions
	//{{AFX_MSG(CBCGBrushPreviewCtrl)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBCGPGraphicsManager*	m_pGM;
	CBCGPBrush*				m_pBrush;
	CBrush					m_brBack;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPEditBrushOptions

struct BCGCBPRODLLEXPORT CBCGPEditBrushOptions
{
public:
	CBCGPEditBrushOptions();

	CString		m_strLabel;
	CString		m_strEmptyColorLabel;
	CString		m_strOtherColorLabel;
	COLORREF	m_clrDefault;
	COLORREF	m_clrGradientDefault;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPEditBrushDlg dialog

class BCGCBPRODLLEXPORT CBCGPEditBrushDlg : public CBCGPDialog
{
// Construction
public:
	CBCGPEditBrushDlg(CBCGPBrush& brush, 
		CWnd* pParent = NULL, CBCGPEditBrushOptions* pOptions = NULL);

// Dialog Data
	//{{AFX_DATA(CBCGPEditBrushDlg)
	enum { IDD = IDD_BCGBARRES_EDIT_BRUSH };
	CBCGPButton	m_wndButtonSwap;
	CBCGPColorButton		m_wndColor1;
	CBCGPColorButton		m_wndColor2;
	CBCGPSliderCtrl			m_wndOpacity;
	CBCGBrushPreviewCtrl	m_wndImage;
	int		m_nType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPEditBrushDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGPEditBrushDlg)
	afx_msg void OnUpdateBrush();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSwapColors();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBCGPBrush				m_brush;
	CBCGPBrush&				m_brushRes;
	BOOL					m_bOriginalIsEmpty;
	CBCGPEditBrushOptions	m_Options;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPEDITBRUSHDLG_H__C7A61B10_5A8A_45A8_B37A_45DEEE693204__INCLUDED_)
