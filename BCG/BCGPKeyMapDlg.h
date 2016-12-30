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

#if !defined(AFX_BCGPKEYMAPDLG_H__BD41B8C2_5025_11D4_9A06_E906E7563969__INCLUDED_)
#define AFX_BCGPKEYMAPDLG_H__BD41B8C2_5025_11D4_9A06_E906E7563969__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPKeyMapDlg.h : header file
//

#include "BCGCBPro.h"
#include "BCGPDialog.h"
#include "bcgprores.h"
#include "BCGPStatic.h"
#include "BCGPButton.h"
#include "BCGPListCtrl.h"
#include "BCGPComboBox.h"

class CBCGPToolbarCustomize;
class CBCGPToolbarButton;
class CBCGPRibbonBar;
class CBCGPBaseRibbonElement;

/////////////////////////////////////////////////////////////////////////////
// CBCGPKeyMapDlg dialog

class BCGCBPRODLLEXPORT CBCGPKeyMapDlg : public CBCGPDialog
{
// Construction
public:
	CBCGPKeyMapDlg(CFrameWnd* pWndParentFrame, BOOL bEnablePrint = FALSE, BOOL bShowCommandsWithKeyOnly = FALSE);
	~CBCGPKeyMapDlg();

// Dialog Data
	//{{AFX_DATA(CBCGPKeyMapDlg)
	enum { IDD = IDD_BCGBARRES_KEYMAP };
	CBCGPStatic	m_wndAccelLabel;
	CBCGPListCtrl	m_KeymapList;
	CBCGPComboBox	m_wndCategoryList;
	CStatic		m_wndViewIcon;
	CBCGPComboBox	m_wndViewTypeList;
	CBCGPButton	m_ButtonPrint;
	CBCGPButton	m_ButtonCopy;
	//}}AFX_DATA

	CFrameWnd*				m_pWndParentFrame;
	HACCEL					m_hAccelTable;
	LPACCEL					m_lpAccel;
	int						m_nAccelSize;
	CBCGPToolbarCustomize*	m_pDlgCust;
	int						m_nSortedCol;
	BOOL					m_bSortAscending;
	const BOOL				m_bEnablePrint;
	const BOOL				m_bShowCommandsWithKeyOnly;
	HINSTANCE				m_hInstDefault;
	BOOL					m_bItemWasAdded;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPKeyMapDlg)
	public:
	virtual INT_PTR DoModal();
	virtual BOOL Create(CWnd* pParentWnd);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

protected:
	virtual void CopyKeyMap ();
	virtual void PrintKeyMap ();

	virtual int OnPrintHeader (CDC& dc, int nPage, int cx) const;
	virtual int OnPrintItem (CDC& dc, int nItem, int y, int cx, BOOL bCalcHeight) const;

	virtual CString FormatItem (int nItem) const;

	virtual void OnSetColumns ();
	virtual void OnInsertItem (CBCGPToolbarButton* pButton, int nItem);
#ifndef BCGP_EXCLUDE_RIBBON
	virtual void OnInsertItem (CBCGPBaseRibbonElement* pRibbonElement, int nItem);
#endif
	virtual void SetColumnsWidth ();

	virtual CString GetCommandKeys (UINT uiCmdID) const;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGPKeyMapDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeViewType();
	afx_msg void OnSelchangeCategory();
	afx_msg void OnCopy();
	afx_msg void OnPrint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBCGPRibbonBar*	m_pWndRibbonBar;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPKEYMAPDLG_H__BD41B8C2_5025_11D4_9A06_E906E7563969__INCLUDED_)
