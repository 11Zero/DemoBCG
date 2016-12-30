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

#if !defined(AFX_BCGPRIBBONTOOLSPAGE_H__C263D518_7328_4D23_B296_A8B40CFB397D__INCLUDED_)
#define AFX_BCGPRIBBONTOOLSPAGE_H__C263D518_7328_4D23_B296_A8B40CFB397D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPRibbonToolsPage.h : header file
//
#include "BCGCBPro.h"
#include "BCGPEditListBox.h"
#include "BCGPMenuButton.h"
#include "BCGPPropertyPage.h"

#ifndef BCGP_EXCLUDE_RIBBON

class CBCGPRibbonToolsPage;

class CRibbonToolsList : public CBCGPEditListBox
{
public:
	CRibbonToolsList(CBCGPRibbonToolsPage* pParent) :
	  m_pParent (pParent) {}
	  
	  virtual void OnSelectionChanged ();
	  virtual BOOL OnBeforeRemoveItem (int iItem);
	  virtual void OnAfterAddItem (int iItem);
	  virtual void OnAfterRenameItem (int iItem);
	  virtual void OnAfterMoveItemUp (int iItem);
	  virtual void OnAfterMoveItemDown (int iItem);
	  
	  CBCGPRibbonToolsPage* m_pParent;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonToolsPage window

class BCGCBPRODLLEXPORT CBCGPRibbonToolsPage : public CBCGPPropertyPage
{
	friend class CRibbonToolsList;
	
	// Construction
public:
	CBCGPRibbonToolsPage(CBCGPRibbonBar* pParentRibbon);
	~CBCGPRibbonToolsPage();
	
	// Dialog Data
	//{{AFX_DATA(CBCGPRibbonToolsPage)
	enum { IDD = IDD_BCGBARRES_PROPPAGE7 };
	CBCGPMenuButton	m_wndInitialDirBtn;
	CBCGPMenuButton	m_wndArgumentsBtn;
	CEdit	m_wndArgumentsEdit;
	CEdit	m_wndInitialDirEdit;
	CEdit	m_wndCommandEdit;
	CButton	m_wndBrowseBtn;
	CRibbonToolsList	m_wndToolsList;
	CString	m_strCommand;
	CString	m_strArguments;
	CString	m_strInitialDirectory;
	//}}AFX_DATA
	
	
	// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonToolsPage)
	public:
	virtual void OnOK();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBCGPRibbonToolsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnBcgbarresBrowseCommand();
	afx_msg void OnUpdateTool();
	afx_msg void OnArgumentsOptions();
	afx_msg void OnInitialDirectoryOptions();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
	CBCGPUserTool* CreateNewTool ();
	void EnableControls ();

	CBCGPRibbonBar*			m_pParentRibbon;
	CBCGPUserToolsManager	m_ToolsManager;
	CBCGPUserTool*			m_pSelTool;
	CMenu                   m_menuArguments;
	CMenu                   m_menuInitialDir;
};

#endif // BCGP_EXCLUDE_RIBBON

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRIBBONTOOLSPAGE_H__C263D518_7328_4D23_B296_A8B40CFB397D__INCLUDED_)
