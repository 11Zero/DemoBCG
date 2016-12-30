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
// BCGPRibbonTreeCtrl.h : header file
//

#if !defined(AFX_BCGPRIBBONTREECTRL_H__C48070ED_1185_47B5_9CBF_786C88E122A6__INCLUDED_)
#define AFX_BCGPRIBBONTREECTRL_H__C48070ED_1185_47B5_9CBF_786C88E122A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPGridCtrl.h"

class CBCGPRibbonBar;
class CBCGPRibbonCategory;
class CBCGPRibbonCustomizationData;
class CBCGPBaseRibbonElement;

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonTreeCtrl window

#ifndef BCGP_EXCLUDE_RIBBON
#ifndef BCGP_EXCLUDE_GRID_CTRL

class BCGCBPRODLLEXPORT CBCGPRibbonTreeCtrl : public CBCGPGridCtrl
{
// Construction
public:
	CBCGPRibbonTreeCtrl(CBCGPRibbonCustomizationData* pCustomizationData, CBCGPRibbonBar* pRibbonBar, BOOL bShowHiddenCategories);

// Attributes
protected:
	CBCGPRibbonBar*					m_pRibbonBar;
	CBCGPRibbonCustomizationData*	m_pCustomizationData;

// Operations
public:
	void RebuildItems(int nRibbonTabsDest, LPCTSTR lpszMainTabsCaption);
	void RebuildItems(const CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& arElements);

	BOOL MoveSelection(BOOL bMoveNext);
	BOOL IsSelFirstInGroup() const;
	BOOL IsSelLastInGroup() const;

	UINT GetContextID(CBCGPGridRow* pRow, int nOffset = 0) const;

	CBCGPBaseRibbonElement* FindElementInPanel(CBCGPRibbonPanel* pPanel, UINT nID) const;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonTreeCtrl)
	//}}AFX_VIRTUAL

	virtual BOOL OnDrawItem (CDC* pDC, CBCGPGridRow* pItem);
	virtual int GetLeftMarginWidth() const { return m_bListMode ? 0 : CBCGPGridCtrl::GetLeftMarginWidth(); }

// Implementation
public:
	virtual ~CBCGPRibbonTreeCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPRibbonTreeCtrl)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void AddCategoty(CBCGPRibbonCategory* pCategory);
	void SetVisibility(HTREEITEM hItem);

	UINT GetCategoryContextID(CBCGPRibbonCategory* pCategory) const;

	BOOL	m_bListMode;
	BOOL	m_bShowHiddenCategories;
};

#else

class BCGCBPRODLLEXPORT CBCGPRibbonTreeCtrl : public CStatic
{
	// Construction
public:
	CBCGPRibbonTreeCtrl(CBCGPRibbonCustomizationData* /*pCustomizationData*/, CBCGPRibbonBar* /*pRibbonBar*/, BOOL /*bShowHiddenCategories*/)
	{
	}

	void RebuildItems(int /*nRibbonTabsDest*/, LPCTSTR /*lpszMainTabsCaption*/) {}
	void RebuildItems(const CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*>& /*arElements*/) {}
	
	BOOL MoveSelection(BOOL /*bMoveNext*/) { return FALSE; }
};

#endif
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRIBBONTREECTRL_H__C48070ED_1185_47B5_9CBF_786C88E122A6__INCLUDED_)
