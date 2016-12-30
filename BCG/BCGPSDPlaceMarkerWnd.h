#if !defined(AFX_BCGPSDPLACEMARKERWND_H__FA84F558_73E5_40AA_9C70_4D69E9FF496C__INCLUDED_)
#define AFX_BCGPSDPLACEMARKERWND_H__FA84F558_73E5_40AA_9C70_4D69E9FF496C__INCLUDED_

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
// BCGPSDPlaceMarkerWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBCGPSDPlaceMarkerWnd window

class CBCGPSDPlaceMarkerWnd : public CWnd
{
	friend class CBCGPSmartDockingManager;

// Construction
public:
	CBCGPSDPlaceMarkerWnd ();

    void Create (CWnd* pwndOwner);

	void SetDockingWnd (CWnd* pDockingWnd)
	{
		m_pDockingWnd = pDockingWnd;
	}

    void ShowAt (CRect rect);
    void ShowTabbedAt (CRect rect, CRect rectTab);
    void Hide ();

// Attributes
protected:
	CWnd*	m_pWndOwner;
	CWnd*	m_pDockingWnd;
	CRect	m_rectLast;
	CRect	m_rectTab;
    BOOL    m_bTabbed;
    BOOL    m_bShown;
	BOOL	m_bUseThemeColorInShading;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPSDPlaceMarkerWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPSDPlaceMarkerWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPSDPlaceMarkerWnd)
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPSDPLACEMARKERWND_H__FA84F558_73E5_40AA_9C70_4D69E9FF496C__INCLUDED_)
