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

#if !defined(AFX_BCGPTOOLSPAGE_H__80D80813_B943_11D3_A713_009027900694__INCLUDED_)
#define AFX_BCGPTOOLSPAGE_H__80D80813_B943_11D3_A713_009027900694__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPToolsPage.h : header file
//

#include "BCGPEditListBox.h"
#include "BCGPMenuButton.h"

class CBCGPToolbarCustomize;
class CBCGPToolsPage;
class CBCGPUserTool;

class CToolsList : public CBCGPEditListBox
{
public:
	CToolsList(CBCGPToolsPage* pParent) :
		m_pParent (pParent) {}
		
	virtual void OnSelectionChanged ();
	virtual BOOL OnBeforeRemoveItem (int iItem);
	virtual void OnAfterAddItem (int iItem);
	virtual void OnAfterRenameItem (int iItem);
	virtual void OnAfterMoveItemUp (int iItem);
	virtual void OnAfterMoveItemDown (int iItem);

	CBCGPToolsPage* m_pParent;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolsPage dialog

class CBCGPToolsPage : public CPropertyPage
{
	friend class CToolsList;

// Construction
public:
	CBCGPToolsPage();
	~CBCGPToolsPage();

// Dialog Data
	//{{AFX_DATA(CBCGPToolsPage)
	enum { IDD = IDD_BCGBARRES_PROPPAGE7 };
	CBCGPMenuButton	m_wndInitialDirBtn;
	CBCGPMenuButton	m_wndArgumentsBtn;
	CEdit	m_wndArgumentsEdit;
	CEdit	m_wndInitialDirEdit;
	CEdit	m_wndCommandEdit;
	CButton	m_wndBrowseBtn;
	CToolsList	m_wndToolsList;
	CString	m_strCommand;
	CString	m_strArguments;
	CString	m_strInitialDirectory;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CBCGPToolsPage)
	public:
	virtual void OnOK();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBCGPToolsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnBcgbarresBrowseCommand();
	afx_msg void OnUpdateTool();
	afx_msg void OnArgumentsOptions();
	afx_msg void OnInitialDirectoryOptions();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBCGPUserTool* CreateNewTool ();
	void EnableControls ();

	CBCGPUserTool*			m_pSelTool;
	CBCGPToolbarCustomize*	m_pParentSheet;
	//UPDATE
	CMenu                   m_menuArguments;
	CMenu                   m_menuInitialDir;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPTOOLSPAGE_H__80D80813_B943_11D3_A713_009027900694__INCLUDED_)
