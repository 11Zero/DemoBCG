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
// BCGPRibbonCustomizePage.h : header file
//

#if !defined(AFX_BCGPRIBBONCUSTOMIZEPAGE_H__F0EB6A7D_D21D_47B5_99AF_D55DBC42F199__INCLUDED_)
#define AFX_BCGPRIBBONCUSTOMIZEPAGE_H__F0EB6A7D_D21D_47B5_99AF_D55DBC42F199__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPPropertySheet.h"
#include "BCGPPropertyPage.h"
#include "BCGPButton.h"
#include "BCGPRibbonTreeCtrl.h"
#include "BCGPRibbonCustomizationData.h"
#include "bcgprores.h"

class CBCGPRibbonBar;
class CBCGPRibbonCustomGroup;
class CBCGPRibbonCustomizeQATPage;
class CBCGPRibbonToolsPage;
class CBCGPLocalResource;

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCustomizeRibbonPage dialog

class BCGCBPRODLLEXPORT CBCGPRibbonCustomizeRibbonPage : public CBCGPPropertyPage
{
	friend class CBCGPRibbonCustomize;

	DECLARE_DYNCREATE(CBCGPRibbonCustomizeRibbonPage)

// Construction
public:
	CBCGPRibbonCustomizeRibbonPage(CBCGPRibbonBar* pRibbonBar = NULL, BOOL bShowHiddenCategories = FALSE);
	virtual ~CBCGPRibbonCustomizeRibbonPage();

	void AddCustomCategory (LPCTSTR lpszName, const CList<UINT, UINT>& lstIDS);
	void EnableKeyboradCustomization (BOOL bEnable = TRUE);

#ifndef BCGP_EXCLUDE_GRID_CTRL
	virtual void OnShowTreeContextMenu(CBCGPRibbonTreeCtrl* pTreeCtrl, CPoint point);
#endif

// Dialog Data
	//{{AFX_DATA(CBCGPRibbonCustomizeRibbonPage)
	enum { IDD = IDD_BCGBARRES_PROPPAGE9 };
	CButton	m_wndNewTab;
	CComboBox	m_wndRibbonTabsCombo;
	CStatic	m_wndRibbonSrcTreePlaceholder;
	CButton	m_wndRename;
	CButton	m_wndNewGroup;
	CStatic	m_wndRibbonTreePlaceholder;
	CButton	m_wndKbdCustomize;
	CButton	m_wndAdd;
	CButton	m_wndRemove;
	CComboBox	m_wndCategoryCombo;
	CBCGPButton	m_wndUp;
	CBCGPButton	m_wndDown;
	int		m_nCategory;
	int		m_nRibbonTabsDest;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonCustomizeRibbonPage)
	public:
	virtual void OnOK();
	virtual void OnCancel();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CBCGPRibbonCustomizeRibbonPage)
	afx_msg void OnSelendokCategoryCombo();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnUp();
	afx_msg void OnDown();
	afx_msg void OnToolbarReset();
	virtual BOOL OnInitDialog();
	afx_msg void OnCustomizeKeyboard();
	afx_msg void OnNewGroup();
	afx_msg void OnNewTab();
	afx_msg void OnRename();
	afx_msg void OnSelendokRibbonTabsDest();
	//}}AFX_MSG
	afx_msg LRESULT OnSelChangeRibbonTree(WPARAM, LPARAM);
	afx_msg LRESULT OnChangeRibbonTree(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

	void CreateRibbonTree(CStatic& wndPlaceHolder, CBCGPRibbonTreeCtrl& wndTree);

#ifndef BCGP_EXCLUDE_GRID_CTRL
	void RenameCategory(CBCGPGridRow* pRow);
	void RenamePanel(CBCGPGridRow* pRow);
	void RenameElement(CBCGPGridRow* pRow);
#endif

	void MoveItem (BOOL bMoveUp);
	void RebuildDestTree(DWORD_PTR dwNewSel = 0, BOOL bExpandSel = FALSE);

	int GetNewTabIndex();
	UINT GetNewTabContextID();
	int GetNewPanelIndex();

	CList<CBCGPRibbonCustomGroup*,CBCGPRibbonCustomGroup*>	m_lstCustomCategories;
	CBCGPRibbonBar*											m_pRibbonBar;

	CBCGPRibbonTreeCtrl										m_wndRibbonTreeSrc;
	CBCGPRibbonTreeCtrl										m_wndRibbonTreeDest;

	BOOL													m_bIsCustomizeKeyboard;
	BOOL													m_bNoContextCategories;
	BOOL													m_bDontProcessSelChanged;
	CBCGPRibbonCustomizationData							m_CustomizationData;

	CBCGPRibbonCategory*									m_pSelCategorySrc;
	CBCGPRibbonCategory*									m_pSelCategoryDest;
	CBCGPRibbonPanel*										m_pSelPanelSrc;
	CBCGPRibbonPanel*										m_pSelPanelDest;
	CBCGPBaseRibbonElement*									m_pSelElemSrc;
	CBCGPBaseRibbonElement*									m_pSelElemDest;

	CString													m_strMainTabs;
};

class BCGCBPRODLLEXPORT CBCGPRibbonCustomize : public CBCGPPropertySheet
{
	DECLARE_DYNAMIC(CBCGPRibbonCustomize)

public:
	CBCGPRibbonCustomize (CWnd* pWndParent, CBCGPRibbonBar* pRibbon, BOOL bCustomizeQAT = TRUE);
	virtual ~CBCGPRibbonCustomize ();

	void EnableKeyboradCustomization (BOOL bEnable = TRUE);

protected:
	CBCGPRibbonCustomizeRibbonPage*	m_pPage;
	CBCGPRibbonCustomizeQATPage*	m_pQATPage;
	CBCGPRibbonToolsPage*			m_pToolsPage;
	BOOL							m_bCustomizeQAT;
	CString							m_strCaption;
};

#endif // BCGP_EXCLUDE_RIBBON

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRIBBONCUSTOMIZEPAGE_H__F0EB6A7D_D21D_47B5_99AF_D55DBC42F199__INCLUDED_)
