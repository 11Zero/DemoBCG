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

// BCGContextMenuManager.h: interface for the CBCGPContextMenuManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPCONTEXTMENUMANAGER_H__64F13A54_E5A9_11D1_A08F_00A0C9B05590__INCLUDED_)
#define AFX_BCGPCONTEXTMENUMANAGER_H__64F13A54_E5A9_11D1_A08F_00A0C9B05590__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"

class CBCGPPopupMenu;

class BCGCBPRODLLEXPORT CBCGPContextMenuManager : public CObject
{
	friend class CBCGPPopupMenuBar;
	friend class CBCGPRibbonPanelMenuBar;

public:
	CBCGPContextMenuManager();
	virtual ~CBCGPContextMenuManager();

// Opreations:
public:
	BOOL AddMenu (UINT uiMenuNameResId, UINT uiMenuResId);
	BOOL AddMenu (LPCTSTR lpszName, UINT uiMenuResId);

	virtual BOOL ShowPopupMenu (UINT uiMenuResId, int x, int y, CWnd* pWndOwner, 
		BOOL bOwnMessage = FALSE, BOOL bRightAlign = FALSE);
	virtual CBCGPPopupMenu* ShowPopupMenu (HMENU hmenuPopup, 
								int x, int y, 
								CWnd* pWndOwner, BOOL bOwnMessage = FALSE,
								BOOL bAutoDestroy = TRUE, BOOL bRightAlign = FALSE);
	virtual UINT TrackPopupMenu (HMENU hmenuPopup, int x, int y, CWnd* pWndOwner, BOOL bRightAlign = FALSE);

	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL);
	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL);

// Customization operations:
	void GetMenuNames (CStringList& listOfNames) const;
	HMENU GetMenuByName (LPCTSTR lpszName, UINT* puiOrigResID = NULL) const;
	HMENU GetMenuById (UINT nMenuResId) const;

	virtual BOOL ResetState ();

// Attributes:
public:
	void SetDontCloseActiveMenu (BOOL bSet = TRUE)
	{
		m_bDontCloseActiveMenu = bSet;
	}

protected:
	CMap<UINT, UINT, HMENU, HMENU>			m_Menus;
	CMap<CString, LPCTSTR, HMENU, HMENU> 	m_MenuNames;
	CMap<UINT, UINT, CObList*, CObList*>	m_MenuOriginalItems;
	UINT									m_nLastCommandID;
	BOOL									m_bTrackMode;
	BOOL									m_bDontCloseActiveMenu;

	void CopyOriginalMenuItemsToMenu (UINT uiResId, CBCGPPopupMenuBar& menuBar);
	void CopyOriginalMenuItemsFromMenu (UINT uiResId, CBCGPPopupMenuBar& menuBar);
};

extern BCGCBPRODLLEXPORT CBCGPContextMenuManager*	g_pContextMenuManager;

#endif // !defined(AFX_BCGPCONTEXTMENUMANAGER_H__64F13A54_E5A9_11D1_A08F_00A0C9B05590__INCLUDED_)
