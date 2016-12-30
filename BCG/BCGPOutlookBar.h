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

#if !defined(AFX_BCGPOUTLOOKBAR_H__D92D40D4_BD73_11D1_A63F_00A0C93A70EC__INCLUDED_)
#define AFX_BCGPOUTLOOKBAR_H__D92D40D4_BD73_11D1_A63F_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// bcgoutlookbar.h : header file
//

#include "BCGCBPro.h"
#include "BCGPBaseTabbedBar.h"

class CBCGPOutlookBarPane;

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookBar window

class BCGCBPRODLLEXPORT CBCGPOutlookBar : public CBCGPBaseTabbedBar
{
	DECLARE_SERIAL(CBCGPOutlookBar)

// Construction
public:
	CBCGPOutlookBar();
	virtual ~CBCGPOutlookBar();

// Attributes

	virtual BOOL CanAcceptBar (const CBCGPBaseControlBar* pBar) const;
	virtual BOOL AllowDestroyEmptyTabbedBar () const 
	{
		// allow to destroy only when the outlook can be floating (dynamic)
		return CanFloat ();
	}

	virtual void OnScroll (BOOL /*bDown*/) {}

	void SetMode2003 (BOOL bMode2003 = TRUE);
	BOOL IsMode2003 () const
	{
		return m_bMode2003;
	}
	
	virtual BOOL CanSetCaptionTextToTabName () const
	{
		return m_bMode2003 ? TRUE : CBCGPBaseTabbedBar::CanSetCaptionTextToTabName ();
	}

	CFont* GetButtonsFont () const
	{
		return m_pFontButtons;
	}

	void SetButtonsFont (CFont* pFont, BOOL bRedraw = TRUE);

	virtual BOOL IsRestoreTabsState() const
	{
		return !m_bMode2003;
	}

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPOutlookBar)
	virtual BOOL Create(LPCTSTR lpszCaption, CWnd* pParentWnd, const RECT& rect, UINT nID, DWORD dwStyle, DWORD dwBCGStyle = CBRS_BCGP_RESIZE, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL
	virtual void GetTabArea (CRect& rectTabAreaTop, CRect& rectTabAreaBottom) const;
	virtual BCGP_CS_STATUS GetDockStatus (CPoint pt, int nSencitivity);
// Implementation

	CBCGPOutlookBarPane* CreateCustomPage (	LPCTSTR lpszPageName,
											BOOL bActivatePage = TRUE,
											DWORD dwEnabledDocking = CBRS_ALIGN_ANY, 
											BOOL bEnableTextLabels = TRUE);

	BOOL RemoveCustomPage (UINT uiPage, CBCGPOutlookWnd* pTargetWnd);

	virtual BOOL LoadState (LPCTSTR lpszProfileName, int nIndex, UINT uiID);
	virtual BOOL SaveState (LPCTSTR lpszProfileName, int nIndex, UINT uiID);

	virtual BOOL OnBeforeAnimation (int /*nPage*/)	{	return TRUE;	}
	virtual void OnAfterAnimation (int /*nPage*/)	{}

	virtual BOOL FloatTab (CWnd* pBar, int nTabID, BCGP_DOCK_METHOD dockMethod, 
									  BOOL bHide);

	virtual void GetBarName (CString& strName) const
	{
		strName = m_strBarName;
	}

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPOutlookBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Attributes
public:
	virtual BOOL DoesAllowDynInsertBefore () const {return FALSE;}

protected:
	UINT FindAvailablePageID ();

protected:
	CObList	m_lstCustomPages;
	BOOL	m_bMode2003;
	CFont*	m_pFontButtons;

	CString	m_strBarName;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPOUTLOOKBAR_H__D92D40D4_BD73_11D1_A63F_00A0C93A70EC__INCLUDED_)
