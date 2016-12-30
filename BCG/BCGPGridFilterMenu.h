//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a sample for BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPGridFilterMenu.h: interface for the CBCGPGridFilterMenuButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPGRIDFILTERMENU_H__10CABCAB_AAB9_4933_906E_4097D230362A__INCLUDED_)
#define AFX_BCGPGRIDFILTERMENU_H__10CABCAB_AAB9_4933_906E_4097D230362A__INCLUDED_

#include "BCGCBPro.h"

#include "BCGPGridFilter.h"
#include "BCGPDialog.h"
#include "BCGPEdit.h"
#include "BCGPListBox.h"
#include "BCGPButton.h"
#include "bcgprores.h"

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
#include "BCGPPopupMenu.h"
#include "BCGPPopupMenuBar.h"
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridFilterListDlg dialog

class CBCGPBaseFilterPopupMenu;

class CBCGPGridFilterListDlg : public CBCGPDialog
{
	// Construction
public:
	CBCGPGridFilterListDlg(UINT uiFilterCmd, 
		const CStringList& lstValues,
		CBCGPBaseFilterPopupMenu* pParent = NULL);

	// Dialog Data
	//{{AFX_DATA(CBCGPGridFilterListDlg)
	enum { IDD = IDD_BCGBARRES_GRID_FILTER_DLG };
	CBCGPButton	m_btnOK;
	CBCGPButton	m_btnCancel;
	CBCGPEdit	m_wndEdit;
	CBCGPCheckListBox	m_wndFilterList;
	CString	m_strSearch;
	//}}AFX_DATA
	
	UINT			m_uiFilterCmd;
	CStringList		m_lstValues;
	BOOL			m_bIsEmptyMenu;
	
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPGridFilterListDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
	// Implementation
protected:
	void FillList ();
	
	// Generated message map functions
	//{{AFX_MSG(CBCGPGridFilterListDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnChangeFilterSearch();
	//}}AFX_MSG
	afx_msg void OnCheckchangeFilterList();
	DECLARE_MESSAGE_MAP()

	CString	m_strSelectAll;
};

////////////////////////////////////////////////////////////////////////
// CBCGPGridFilterPopupMenuBar

class CBCGPGridFilterPopupMenuBar : public CBCGPPopupMenuBar
{
	friend class CBCGPBaseFilterPopupMenu;

	CBCGPGridFilterPopupMenuBar()
	{
		m_nListWidth = 0;
	}

	virtual CSize CalcSize(BOOL bVertDock)
	{
		CSize size = CBCGPPopupMenuBar::CalcSize(bVertDock);
		size.cx = max(size.cx, m_nListWidth);

		return size;
	}

	virtual CSize GetEmptyMenuSize() const
	{
		return CSize(0, 0);
	}

	int m_nListWidth;
};

////////////////////////////////////////////////////////////////////////
// CBCGPBaseFilterPopupMenu

class BCGCBPRODLLEXPORT CBCGPBaseFilterPopupMenu : public CBCGPPopupMenu
{
	DECLARE_DYNAMIC(CBCGPBaseFilterPopupMenu)

public:
	CBCGPBaseFilterPopupMenu(
		UINT uiFilterCmd,
		const CStringList& lstCheckItems);

	virtual ~CBCGPBaseFilterPopupMenu();

	virtual BOOL IsAll() const = 0;
	virtual void SetAll(BOOL bSet) = 0;

	virtual BOOL IsChecked(LPCTSTR lpszItem) const = 0;
	virtual void AddChecked(LPCTSTR lpszItem) = 0;

	virtual void SendFilterCommand (CWnd* pDestWnd, UINT uiCmd);

protected:
	// Generated message map functions
	//{{AFX_MSG(CBCGPBaseFilterPopupMenu)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual void RecalcLayout(BOOL bNotify = TRUE);
	virtual CBCGPPopupMenuBar* GetMenuBar() { return &m_wndMenuBar; }

	CBCGPGridFilterPopupMenuBar	m_wndMenuBar;
	CBCGPGridFilterListDlg		m_wndList;
};

////////////////////////////////////////////////////////////////////////
// CBCGPGridFilterPopupMenu

class CBCGPGridCtrl;

class BCGCBPRODLLEXPORT CBCGPGridFilterPopupMenu : public CBCGPBaseFilterPopupMenu
{
	DECLARE_DYNAMIC(CBCGPGridFilterPopupMenu)
		
public:
	CBCGPGridFilterPopupMenu(
		UINT uiFilterCmd,
		const CStringList& lstCheckItems,
		BCGP_FILTER_COLUMN_INFO& filterColumnInfo,
		CBCGPGridCtrl* pOwnerGrid);
	
	virtual ~CBCGPGridFilterPopupMenu();
	
	virtual BOOL IsAll() const;
	virtual void SetAll(BOOL bSet);
	
	virtual BOOL IsChecked(LPCTSTR lpszItem) const;
	virtual void AddChecked(LPCTSTR lpszItem);

	virtual void SendFilterCommand (CWnd* pDestWnd, UINT uiCmd);
	
protected:
	// Generated message map functions
	//{{AFX_MSG(CBCGPGridFilterPopupMenu)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
	BCGP_FILTER_COLUMN_INFO&	m_filterColumnInfo;
	CBCGPGridCtrl*				m_pOwnerGrid;
};

#endif // !defined(AFX_BCGPGRIDFILTERMENU_H__10CABCAB_AAB9_4933_906E_4097D230362A__INCLUDED_)
