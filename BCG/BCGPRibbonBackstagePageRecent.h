//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
// BCGPRibbonBackstagePageRecent.h : header file
//
#if !defined(AFX_BCGPRIBBONBACKSTAGEPAGERECENT_H__C8A471B5_FA79_423C_B3CD_C1A9E6288095__INCLUDED_)
#define AFX_BCGPRIBBONBACKSTAGEPAGERECENT_H__C8A471B5_FA79_423C_B3CD_C1A9E6288095__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPDialog.h"
#include "BCGPRibbonBackstageViewPanel.h"

#ifndef BCGP_EXCLUDE_RIBBON

#define BCGP_SHOW_FOLDERS_LIST	0x0001
#define BCGP_SHOW_PINS			0x0002

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonBackstagePageRecent dialog

class BCGCBPRODLLEXPORT CBCGPRibbonBackstagePageRecent : public CBCGPDialog
{
	DECLARE_DYNCREATE(CBCGPRibbonBackstagePageRecent)

// Construction
public:
	CBCGPRibbonBackstagePageRecent(CWnd* pParent = NULL);   // standard constructor

// Attributes
public:
	void SetFlags(UINT nFlags);
	UINT GetFlags() const
	{
		return m_nFlags;
	}

// Dialog Data
	//{{AFX_DATA(CBCGPRibbonBackstagePageRecent)
	enum { IDD = IDD_BCGBARRES_RECENT_FORM };
	CBCGPRecentFilesListBox	m_wndRecentFiles;
	CBCGPRecentFilesListBox	m_wndRecentFolders;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonBackstagePageRecent)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	void OnDraw(CDC* pDC);

// Implementation
protected:

	void HideAndDisable(UINT nID);

	// Generated message map functions
	//{{AFX_MSG(CBCGPRibbonBackstagePageRecent)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	//}}AFX_MSG
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()

	UINT	m_nFlags;
};

#endif // BCGP_EXCLUDE_RIBBON

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRIBBONBACKSTAGEPAGERECENT_H__C8A471B5_FA79_423C_B3CD_C1A9E6288095__INCLUDED_)
