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
//
// BCGPRibbonKeyboardCustomizeDlg.h : header file
//

#if !defined(AFX_BCGPRIBBONKEYBOARDCUSTOMIZEDLG_H__A35EF3C4_F289_4EF4_A752_0A3934A37CED__INCLUDED_)
#define AFX_BCGPRIBBONKEYBOARDCUSTOMIZEDLG_H__A35EF3C4_F289_4EF4_A752_0A3934A37CED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPDialog.h"
#include "bcgprores.h"
#include "KeyAssign.h"
#include "BCGPRibbonCommandsListBox.h"

class CBCGPBaseRibbonElement;
class CBCGPRibbonBar;

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonKeyboardCustomizeDlg dialog

class BCGCBPRODLLEXPORT CBCGPRibbonKeyboardCustomizeDlg : public CBCGPDialog
{
// Construction
public:
	CBCGPRibbonKeyboardCustomizeDlg(CBCGPRibbonBar* pRibbonBar, CWnd* pParent = NULL);   // standard constructor
	virtual ~CBCGPRibbonKeyboardCustomizeDlg();

// Dialog Data
	//{{AFX_DATA(CBCGPRibbonKeyboardCustomizeDlg)
	enum { IDD = IDD_BCGBARRES_RIBBON_KEYBOARD_DLG };
	CStatic	m_wndAssignedToTitle;
	CKeyAssign	m_wndNewKey;
	CComboBox	m_wndViewTypeList;
	CStatic	m_wndViewIcon;
	CButton	m_wndRemoveButton;
	CListBox	m_wndCurrentKeysList;
	CBCGPRibbonCommandsListBox	m_wndCommandsList;
	CListBox	m_wndCategoryList;
	CButton	m_wndAssignButton;
	CString	m_strDesrcription;
	CString	m_strAssignedTo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonKeyboardCustomizeDlg)
	public:
	virtual INT_PTR DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGPRibbonKeyboardCustomizeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAssign();
	afx_msg void OnSelchangeCategory();
	afx_msg void OnSelchangeCommandsList();
	afx_msg void OnSelchangeCurrentKeysList();
	afx_msg void OnRemove();
	afx_msg void OnResetAll();
	afx_msg void OnSelchangeViewType();
	afx_msg void OnUpdateNewShortcutKey();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Operations:
protected:
	void AddKeyEntry (LPACCEL pEntry);

// Attributes:
public:
	static BOOL				m_bUseDocumentTemplates;

protected:
	HACCEL					m_hAccelTable;
	LPACCEL					m_lpAccel;
	int						m_nAccelSize;
	CMultiDocTemplate*		m_pSelTemplate;
	CBCGPBaseRibbonElement*	m_pSelButton;
	LPACCEL					m_pSelEntry;
	CFrameWnd*				m_pParentFrame;
	BOOL					m_bAutoSet;
	CBCGPRibbonBar*			m_pRibbonBar;
	CBCGPLocalResource*		m_pLocaRes;
	BOOL					m_bIsAlreadyDefined;
};

#endif	// BCGP_EXCLUDE_RIBBON

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRIBBONKEYBOARDCUSTOMIZEDLG_H__A35EF3C4_F289_4EF4_A752_0A3934A37CED__INCLUDED_)
