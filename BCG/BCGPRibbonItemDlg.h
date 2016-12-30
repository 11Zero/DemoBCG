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
// BCGPRibbonItemDlg.h : header file
//

#if !defined(AFX_BCGPRIBBONITEMDLG_H__81F75AEA_C319_4077_9916_0444D25BB34E__INCLUDED_)
#define AFX_BCGPRIBBONITEMDLG_H__81F75AEA_C319_4077_9916_0444D25BB34E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "bcgprores.h"
#include "BCGPDialog.h"
#include "ButtonsList.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonItemDlg dialog

class BCGCBPRODLLEXPORT CBCGPRibbonItemDlg : public CBCGPDialog
{
// Construction
public:
	CBCGPRibbonItemDlg(CBCGPToolBarImages& images, CWnd* pParent = NULL);   // standard constructor
	virtual ~CBCGPRibbonItemDlg();

// Dialog Data
	//{{AFX_DATA(CBCGPRibbonItemDlg)
	enum { IDD = IDD_BCGBARRES_RIBBON_ITEM_DLG };
	CButtonsList	m_wndImageList;
	CString	m_strName;
	//}}AFX_DATA

	int	m_iSelImage;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonItemDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGPRibbonItemDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnImageList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CObList				m_Buttons;
	CBCGPToolBarImages& m_images;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRIBBONITEMDLG_H__81F75AEA_C319_4077_9916_0444D25BB34E__INCLUDED_)
