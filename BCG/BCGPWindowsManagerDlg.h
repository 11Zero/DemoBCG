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
// BCGPWindowsManagerDlg.h : header file
//

#if !defined(AFX_BCGPWINDOWSMANAGERDLG_H__014E9113_B3C3_11D3_A712_009027900694__INCLUDED_)
#define AFX_BCGPWINDOWSMANAGERDLG_H__014E9113_B3C3_11D3_A712_009027900694__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"
#include "bcgprores.h"
#include "BCGPDialog.h"
#include "BCGPListBox.h"

class CBCGPMDIFrameWnd;

BCGCBPRODLLEXPORT extern UINT WM_BCGWINDOW_HELP;

/////////////////////////////////////////////////////////////////////////////
// CBCGPWindowsManagerDlg dialog

class BCGCBPRODLLEXPORT CBCGPWindowsManagerDlg : public CBCGPDialog
{
// Construction
public:
	CBCGPWindowsManagerDlg(CBCGPMDIFrameWnd* pMDIFrame, BOOL bHelpButton = FALSE);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBCGPWindowsManagerDlg)
	enum { IDD = IDD_BCGBARRES_WINDOWS_DLG };
	CBCGPListBox	m_wndList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPWindowsManagerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void			MDIMessage(UINT uMsg,WPARAM flag);
	void			FillWindowList(void);
	void			SelActive(void);
	void			UpdateButtons(void);

	CBCGPMDIFrameWnd*	m_pMDIFrame;
	const BOOL			m_bHelpButton;

	BOOL				m_bMDIActions;
	CList<HWND,HWND>	m_lstCloseDisabled;
	CList<HWND,HWND>	m_lstSaveDisabled;

	CImageList			m_Icons;

	// Generated message map functions
	//{{AFX_MSG(CBCGPWindowsManagerDlg)
	afx_msg void OnActivate();
	afx_msg void OnSave();
	afx_msg void OnClose();
	afx_msg void OnCascade();
	afx_msg void OnTilehorz();
	afx_msg void OnTilevert();
	afx_msg void OnMinimize();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeBcgbarresList();
	afx_msg void OnBcgbarresWindowHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPWINDOWSMANAGERDLG_H__014E9113_B3C3_11D3_A712_009027900694__INCLUDED_)
