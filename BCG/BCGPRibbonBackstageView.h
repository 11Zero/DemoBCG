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
// BCGPRibbonBackstageView.h : header file
//

#if !defined(AFX_BCGPRIBBONBACKSTAGEVIEW_H__B7F5D00C_3B29_4DA0_8DAA_3925B1FD1BFF__INCLUDED_)
#define AFX_BCGPRIBBONBACKSTAGEVIEW_H__B7F5D00C_3B29_4DA0_8DAA_3925B1FD1BFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPRibbonPanelMenu.h"

class CBCGPRibbonMainPanel;
class CBCGPRibbonBar;

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonBackstageView window

class BCGCBPRODLLEXPORT CBCGPRibbonBackstageView : public CBCGPRibbonPanelMenuBar
{
	DECLARE_DYNAMIC(CBCGPRibbonBackstageView)

// Construction
public:
	CBCGPRibbonBackstageView(CBCGPRibbonBar* pRibbonBar, CBCGPRibbonMainPanel* pPanel);

// Attributes
public:
	void GetRect(CRect& rect);

protected:
	CList<HWND, HWND> m_lstHiddenWindows;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonBackstageView)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
	virtual BOOL Create();

	virtual BOOL IsBackStageView() const
	{
		return TRUE;
	}

	virtual void Deactivate ();

// Implementation
public:
	virtual ~CBCGPRibbonBackstageView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPRibbonBackstageView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnNcDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void SetOnOffMode(BOOL bOn);
};

#endif // BCGP_EXCLUDE_RIBBON

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRIBBONBACKSTAGEVIEW_H__B7F5D00C_3B29_4DA0_8DAA_3925B1FD1BFF__INCLUDED_)
