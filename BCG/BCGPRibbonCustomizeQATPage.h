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
// BCGPRibbonCustomizeQATPage.h : header file
//

#if !defined(AFX_BCGPRIBBONCUSTOMIZEQATPAGE_H__F952A91F_ECE3_4414_BDAC_1CA77AB984CA__INCLUDED_)
#define AFX_BCGPRIBBONCUSTOMIZEQATPAGE_H__F952A91F_ECE3_4414_BDAC_1CA77AB984CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPPropertySheet.h"
#include "BCGPPropertyPage.h"
#include "BCGPButton.h"
#include "BCGPRibbonCommandsListBox.h"
#include "bcgprores.h"

class CBCGPRibbonBar;
class CBCGPLocalResource;
class CBCGPRibbonCustomGroup;

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCustomizeQATPage dialog

class BCGCBPRODLLEXPORT CBCGPRibbonCustomizeQATPage : public CBCGPPropertyPage
{
	DECLARE_DYNCREATE(CBCGPRibbonCustomizeQATPage)

// Construction
public:
	CBCGPRibbonCustomizeQATPage(CBCGPRibbonBar* pRibbonBar = NULL);
	virtual ~CBCGPRibbonCustomizeQATPage();

	void AddCustomCategory (LPCTSTR lpszName, const CList<UINT, UINT>& lstIDS);
	void EnableKeyboradCustomization (BOOL bEnable = TRUE);

// Dialog Data
	//{{AFX_DATA(CBCGPRibbonCustomizeQATPage)
	enum { IDD = IDD_BCGBARRES_PROPPAGE8 };
	CButton	m_wndKbdCustomize;
	CButton	m_wndAdd;
	CButton	m_wndRemove;
	CBCGPRibbonCommandsListBox	m_wndCommandsList;
	CComboBox	m_wndCategoryCombo;
	CBCGPRibbonCommandsListBox	m_wndQATList;
	CBCGPButton	m_wndUp;
	CBCGPButton	m_wndDown;
	int		m_nCategory;
	BOOL	m_bQAToolbarOnBottom;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonCustomizeQATPage)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBCGPRibbonCustomizeQATPage)
	afx_msg void OnSelendokCategoryCombo();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnUp();
	afx_msg void OnDown();
	afx_msg void OnToolbarReset();
	afx_msg void OnSelchangeQATCommands();
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomizeKeyboard();
	afx_msg void OnSelchangeCommandsList();
	//}}AFX_MSG
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void MoveItem (BOOL bMoveUp);

	CBCGPRibbonBar*											m_pRibbonBar;
	CList<CBCGPRibbonCustomGroup*,CBCGPRibbonCustomGroup*>	m_lstCustomCategories;
	BOOL													m_bIsCustomizeKeyboard;
};

// For the backward compatibility:
#define CBCGPRibbonCustomizePage CBCGPRibbonCustomizeQATPage

class CBCGPRibbonCustomGroup : public CObject
{
	DECLARE_DYNAMIC(CBCGPRibbonCustomGroup)

	friend class CBCGPRibbonCustomizeRibbonPage;

	CString				m_strName;
	CList<UINT,UINT>	m_lstIDs;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONCUSTOMIZEQATPAGE_H__F952A91F_ECE3_4414_BDAC_1CA77AB984CA__INCLUDED_)
