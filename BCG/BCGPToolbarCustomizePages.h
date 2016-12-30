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

// BCGPToolbarCustomizePages.h : header file
//

#ifndef __BCGPTOOLBARCUSTOMIZEPAGES_H__
#define __BCGPTOOLBARCUSTOMIZEPAGES_H__

#include "ButtonsList.h"
#include "ButtonsTextList.h"
#include "bcgprores.h"

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "BCGPExCheckList.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPCustomizePage dialog

class CBCGPToolbarButton;
class CBCGPToolBarImages;
class CBCGPToolBar;

class CBCGPCustomizePage : public CPropertyPage
{
	DECLARE_DYNCREATE(CBCGPCustomizePage)

// Construction
public:
	CBCGPCustomizePage();
	~CBCGPCustomizePage();

// Operations:
	void SetUserCategory (LPCTSTR lpszCategory);
	void SetAllCategory (LPCTSTR lpszCategory);
	void OnChangeSelButton (CBCGPToolbarButton* pButton);

protected:

// Dialog Data
	//{{AFX_DATA(CBCGPCustomizePage)
	enum { IDD = IDD_BCGBARRES_PROPPAGE1 };
	CListBox	m_wndCategory;
	CButtonsTextList	m_wndTools;
	CString	m_strButtonDescription;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CBCGPCustomizePage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBCGPCustomizePage)
	afx_msg void OnSelchangeUserTools();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCategory();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Attributes:
protected:

	CBCGPToolbarButton*	m_pSelButton;
	CString				m_strUserCategory;
	CString				m_strAllCategory;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolbarsPage dialog

class CBCGPToolbarsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CBCGPToolbarsPage)

// Construction
public:
	CBCGPToolbarsPage(CFrameWnd* pParentFrame = NULL);
	~CBCGPToolbarsPage();

// Dialog Data
	//{{AFX_DATA(CBCGPToolbarsPage)
	enum { IDD = IDD_BCGBARRES_PROPPAGE2 };
	CButton	m_wndTextLabels;
	CButton	m_bntRenameToolbar;
	CButton	m_btnNewToolbar;
	CButton	m_btnDelete;
	CButton	m_btnReset;
	CBCGPExCheckList	m_wndToolbarList;
	BOOL	m_bTextLabels;
	//}}AFX_DATA

// Operations:
public:
	void ShowToolBar (CBCGPToolBar* pToolBar, BOOL bShow);
	void EnableUserDefinedToolbars (BOOL bEnable)
	{
		m_bUserDefinedToolbars = bEnable;
	}

protected:
// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CBCGPToolbarsPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBCGPToolbarsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeToolbarList();
	afx_msg void OnDblclkToolbarList();
	afx_msg void OnResetToolbar();
	afx_msg void OnResetAllToolbars();
	afx_msg void OnDeleteToolbar();
	afx_msg void OnNewToolbar();
	afx_msg void OnRenameToolbar();
	afx_msg void OnBcgbarresTextLabels();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBCGPToolBar*	m_pSelectedToolbar;
	BOOL			m_bUserDefinedToolbars;
	CFrameWnd*		m_pParentFrame;
};

#endif // __BCGPTOOLBARCUSTOMIZEPAGES_H__
