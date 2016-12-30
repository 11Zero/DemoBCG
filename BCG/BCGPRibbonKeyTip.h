#if !defined(AFX_BCGPRIBBONKEYTIP_H__23B0A2A7_18D2_40BE_B479_6DEECF3BDFB1__INCLUDED_)
#define AFX_BCGPRIBBONKEYTIP_H__23B0A2A7_18D2_40BE_B479_6DEECF3BDFB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// BCGPRibbonKeyTip.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonKeyTip window

#ifndef BCGP_EXCLUDE_RIBBON

class CBCGPBaseRibbonElement;

class CBCGPRibbonKeyTip : public CWnd
{
// Construction
public:
	CBCGPRibbonKeyTip(CBCGPBaseRibbonElement* pElement, BOOL bIsMenu = FALSE);

// Attributes
public:
	CBCGPBaseRibbonElement* GetElement () const
	{
		return m_pElement;
	}

	BOOL IsMenuKey () const
	{
		return m_bIsMenu;
	}

protected:
	CBCGPBaseRibbonElement*	m_pElement;
	CRect					m_rectScreen;
	BOOL					m_bIsMenu;

	static CString			m_strClassName;

// Operations
public:
	BOOL Show (BOOL bRepos = FALSE);
	void Hide ();

protected:
	void UpdateMenuShadow ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonKeyTip)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPRibbonKeyTip();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPRibbonKeyTip)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // BCGP_EXCLUDE_RIBBON

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRIBBONKEYTIP_H__23B0A2A7_18D2_40BE_B479_6DEECF3BDFB1__INCLUDED_)
