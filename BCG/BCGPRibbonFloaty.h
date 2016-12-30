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
// BCGPRibbonFloaty.h : header file
//

#if !defined(AFX_BCGPRIBBONFLOATY_H__629F5B11_333C_45E6_BD8F_748650A59139__INCLUDED_)
#define AFX_BCGPRIBBONFLOATY_H__629F5B11_333C_45E6_BD8F_748650A59139__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef BCGP_EXCLUDE_RIBBON

#include "BCGPRibbonPanelMenu.h"
#include "BCGPRibbonPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonFloaty window

class BCGCBPRODLLEXPORT CBCGPRibbonFloaty : public CBCGPRibbonPanelMenu
{
	DECLARE_DYNCREATE(CBCGPRibbonFloaty)

	friend class CBCGPPopupMenu;
	friend class CBCGPRibbonPanelMenuBar;
	friend class CBCGPRibbonBar;

// Construction
public:
	CBCGPRibbonFloaty();

// Attributes
public:
	BOOL IsContextMenuMode () const
	{
		return m_bContextMenuMode;
	}

	void SetOneRow (BOOL bSet = TRUE)
	{
		m_bIsOneRow = bSet;
	}

	BOOL IsOneRow () const
	{
		return m_bIsOneRow;
	}

protected:
	static CBCGPRibbonFloaty*	m_pCurrent;
	BOOL						m_bContextMenuMode;
	BYTE						m_nTransparency;
	BOOL						m_bWasHovered;
	BOOL						m_bIsOneRow;
	BOOL						m_bWasDroppedDown;

// Operations
public:
	void SetCommands (	CBCGPRibbonBar* pRibbonBar,
						const CList<UINT,UINT>& lstCommands);

	BOOL Show (int x, int y);
	BOOL ShowWithContextMenu (int x, int y, UINT uiMenuResID, CWnd* pWndOwner);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRibbonFloaty)
	//}}AFX_VIRTUAL

	virtual BOOL IsFloaty () const
	{
		return TRUE;
	}

// Implementation
public:
	virtual ~CBCGPRibbonFloaty();
	
	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName);

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPRibbonFloaty)
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL UpdateTransparency ();
	void CancelContextMenuMode ();
};

#endif // BCGP_EXCLUDE_RIBBON

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRIBBONFLOATY_H__629F5B11_333C_45E6_BD8F_748650A59139__INCLUDED_)
