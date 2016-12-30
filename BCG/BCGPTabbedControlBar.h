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

#if !defined(AFX_BCGPTABBEDCONTROLBAR_H__D03B71D9_935C_4BF0_8AF5_A11FFC87BA6A__INCLUDED_)
#define AFX_BCGPTABBEDCONTROLBAR_H__D03B71D9_935C_4BF0_8AF5_A11FFC87BA6A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPTabbedControlBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBCGPTabbedControlBar window

#include "BCGCBPro.h"
#include "BCGPBaseTabbedBar.h"
#include "BCGPTabWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPTabbedControlBar window

class BCGCBPRODLLEXPORT CBCGPTabbedControlBar : public CBCGPBaseTabbedBar
{
	DECLARE_SERIAL(CBCGPTabbedControlBar)
// Construction
public:
	CBCGPTabbedControlBar(BOOL bAutoDestroy = FALSE);

// Attributes
public:
	virtual void GetTabArea (CRect& rectTabAreaTop, CRect& rectTabAreaBottom) const;
	virtual BOOL IsTabLocationBottom () const;

	CBCGPTabWnd* GetTabWnd () const
	{
		ASSERT_KINDOF(CBCGPTabWnd, m_pTabWnd);
		return DYNAMIC_DOWNCAST (CBCGPTabWnd, m_pTabWnd);
	}

	virtual BOOL HasAutoHideMode () const
	{
		return TRUE;
	}

	static CBCGPTabWnd::Style	m_StyleTabWnd; 
	static BOOL					m_bTabsAlwaysTop;
	static CRuntimeClass*		m_pTabWndRTC;
	static BOOL					m_bDestroyUnused;

	static void EnableTabAutoColor (BOOL bEnable = TRUE);
	static void SetTabAutoColors (const CArray<COLORREF, COLORREF>& arColors);
	
	static void ResetTabs ();

// Operations
public:
	virtual BOOL FloatTab (CWnd* pBar, int nTabID, BCGP_DOCK_METHOD dockMethod, BOOL bHide = FALSE);
	virtual BOOL DetachControlBar (CWnd* pBar, BOOL bHide = FALSE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPTabbedControlBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPTabbedControlBar();

protected:
	virtual BOOL CheckTabbedBarAlignment ();
	virtual void OnPressCloseButton ();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPTabbedControlBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// Tab Auto-colors support
	static CArray<COLORREF, COLORREF>	m_arTabsAutoColors;
	static BOOL							m_bIsTabsAutoColor;
	static CList<HWND,HWND>				m_lstTabbedControlBars;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPTABBEDCONTROLBAR_H__D03B71D9_935C_4BF0_8AF5_A11FFC87BA6A__INCLUDED_)
