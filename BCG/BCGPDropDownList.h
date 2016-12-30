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
// BCGPDropDownList.h : header file
//

#if !defined(AFX_BCGPDROPDOWNLIST_H__9C438DB1_7F91_4158_BEAB_9F5C216C49C2__INCLUDED_)
#define AFX_BCGPDROPDOWNLIST_H__9C438DB1_7F91_4158_BEAB_9F5C216C49C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcbpro.h"
#include "BCGPPopupMenu.h"

class CBCGPRibbonComboBox;

/////////////////////////////////////////////////////////////////////////////
// CBCGPDropDownList window

class BCGCBPRODLLEXPORT CBCGPDropDownList : public CBCGPPopupMenu
{
	friend class CBCGPPopupMenuBar;

	DECLARE_DYNAMIC(CBCGPDropDownList)

// Construction
public:
	CBCGPDropDownList();
	CBCGPDropDownList(CWnd* pEditCtrl);
	CBCGPDropDownList(CBCGPRibbonComboBox* pRibbonCombo);

protected:
	void CommonInit ();

// Attributes
public:
	void SetMaxHeight (int nMaxHeight)
	{
		m_nMaxHeight = nMaxHeight;
	}

	void SetMinWidth (int nMinWidth)
	{
		m_nMinWidth = nMinWidth;
	}

	int GetCount() const;

	int GetCurSel();
	int SetCurSel(int nSelect);

	// for listbox items
	void GetText(int nIndex, CString& rString) const;

	BOOL IsEditFocused () const
	{
		return m_pEditCtrl->GetSafeHwnd () == ::GetFocus ();
	}

// Operations
public:
	// manipulating listbox items
	void AddString(LPCTSTR lpszItem);
	void ResetContent();

	void Track (CPoint point, CWnd *pWndOwner = NULL);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPDropDownList)
	//}}AFX_VIRTUAL

	virtual void OnDrawItem (CDC* pDC, CBCGPToolbarMenuButton* pItem, BOOL bHighlight);
	virtual CSize OnGetItemSize (CDC* pDC, CBCGPToolbarMenuButton* pItem, CSize sizeDefault);
	virtual void OnChooseItem (UINT uidCmdID);
	virtual void OnChangeHot (int nHot);

	virtual BOOL DefaultMouseClickOnClose () const
	{
		return m_pRibbonCombo == NULL;
	}

// Implementation
public:
	virtual ~CBCGPDropDownList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPDropDownList)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void HighlightItem (int nIndex);
	CBCGPToolbarButton* GetItem (int nIndex) const;

	CMenu					m_Menu;
	int						m_nCurSel;
	CWnd*					m_pEditCtrl;
	CBCGPRibbonComboBox*	m_pRibbonCombo;
	int						m_nMinWidth;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPDROPDOWNLIST_H__9C438DB1_7F91_4158_BEAB_9F5C216C49C2__INCLUDED_)
