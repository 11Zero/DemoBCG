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

#if !defined(AFX_BCGPMENUBAR_H__644FE23B_D1DB_11D1_A649_00A0C93A70EC__INCLUDED_)
#define AFX_BCGPMENUBAR_H__644FE23B_D1DB_11D1_A649_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BCGPMenuBar.h : header file
//

#include "BCGPToolBar.h"
#include "BCGCBPro.h"

class CBCGPToolbarMenuButton;
class CBCGPToolbarSystemMenuButton;
class CBCGPToolbarMenuButtonsButton;
class CBCGPToolbarComboBoxButton;
class CBCGPMenuPage;

#define BCG_IDW_MENUBAR               0xE806  // CBCGPMenuBar

/////////////////////////////////////////////////////////////////////////////
// CBCGPMenuBar window

class BCGCBPRODLLEXPORT CBCGPMenuBar : public CBCGPToolBar
{
	friend class CBCGPOptionsPage;
	friend class CBCGPToolBar;
	friend class CBCGPPopupMenuBar;
	friend class CBCGPMenuPage;
	friend class CBCGPFrameImpl;

	DECLARE_SERIAL(CBCGPMenuBar)

// Construction
public:
	CBCGPMenuBar();

// Attributes
public:
	// In popup mode, the menu bar will appear upon "Alt" key only
	void EnablePopupMode(BOOL bPopupMode = TRUE);

	virtual BOOL IsPopupMode() const
	{
		return m_bIsPopupMode;
	}

	BOOL IsVisibleInPopupMode() const
	{
		return m_bVisibleInPopupMode;
	}

	static BOOL IsShowAllCommands ()
	{
		return	m_bShowAllCommands || !m_bRecentlyUsedMenus ||
				!m_UsageCount.HasEnouthInformation ();
	}

	static BOOL IsRecentlyUsedMenus ()
	{
		return m_bRecentlyUsedMenus;
	}

	static BOOL IsShowAllCommandsDelay ()
	{
		return m_bShowAllMenusDelay;
	}

	static void SetShowAllCommands (BOOL bShowAllCommands = TRUE);
	static void SetRecentlyUsedMenus (BOOL bOn = TRUE);

	static BOOL SetMenuFont (LPLOGFONT lpLogFont, BOOL bHorz = TRUE);
	static const CFont& GetMenuFont (BOOL bHorz = TRUE);

	static void EnableMenuShadows (BOOL bEnable = TRUE)
	{
		m_bMenuShadows = bEnable;
	}

	static BOOL IsMenuShadows ()
	{
		return m_bMenuShadows;
	}

	static void HighlightDisabledItems (BOOL bHighlight = TRUE)
	{
		m_bHighlightDisabledItems = bHighlight;
	}

	static BOOL IsHighlightDisabledItems ()
	{
		return m_bHighlightDisabledItems ;
	}

	BOOL	m_bAutoDocMenus;	// Automatically load/save  document menus
	BOOL	m_bClearHashOnClose;// Clear temporary files on close

    BOOL GetForceDownArrows() { return m_bForceDownArrows; }
    void SetForceDownArrows(BOOL bValue) { m_bForceDownArrows = bValue; }

	// Support for MS Office-like help combobox:
	void EnableHelpCombobox (UINT uiID /* 0 - disable */, 
		LPCTSTR lpszPrompt = NULL,
		int nComboBoxWidth = 150);
	CBCGPToolbarComboBoxButton* GetHelpCombobox ();

	void SetMenuButtonRTC (CRuntimeClass* pMenuButtonRTC);

protected:
	CWnd*			m_pParentWnd;
	BOOL			m_bMaximizeMode;
	BOOL			m_bIsPopupMode;
	BOOL			m_bIsPopupModeSaved;	// For customization mode
	BOOL			m_bVisibleInPopupMode;
	HMENU			m_hMenu;
	HMENU			m_hDefaultMenu;
	HMENU			m_hSysMenu;
	HICON			m_hSysIcon;
	UINT			m_uiDefMenuResId;
	int				m_nSystemButtonsNum;
	int				m_nSystemButtonsNumSaved;
	BOOL			m_bHaveButtons;
	CSize			m_szSystemButton;	// System button size
	CBCGPMenuPage*	m_pMenuPage;
    BOOL			m_bForceDownArrows;
	UINT			m_nHelpComboID;
	int				m_nHelpComboWidth;
	CString			m_strHelpComboPrompt;
	CRuntimeClass*	m_pMenuButtonRTC;

	static BOOL	m_bShowAllCommands;
	static BOOL	m_bRecentlyUsedMenus;
	static BOOL	m_bShowAllMenusDelay;

	static BOOL m_bMenuShadows;
	static BOOL m_bHighlightDisabledItems;

	IAccessible*	m_pAccessible;

// Operations
public:
	virtual void CreateFromMenu (HMENU hMenu, BOOL bDefaultMenu = FALSE,
						BOOL bForceUpdate = FALSE);
	void SetMaximizeMode (BOOL bMax, CWnd* pWnd = NULL, BOOL bRecalcLayout = TRUE);

	void SetDefaultMenuResId (UINT uiResId);
	UINT GetDefaultMenuResId () const
	{
		return m_uiDefMenuResId;
	}

	HMENU GetHMenu () const
	{
		return m_hMenu;
	}

	HMENU GetDefaultMenu () const
	{
		return m_hDefaultMenu;
	}

	CBCGPToolbarButton* GetMenuItem (int iItem) const;
	CBCGPToolbarSystemMenuButton* GetSystemMenu () const;
	CBCGPToolbarMenuButtonsButton* GetSystemButton (UINT uiBtn, BOOL bByCommand = TRUE) const;
	int GetSystemButtonsCount () const
	{
		return m_nSystemButtonsNum;
	}

	void DeactivateInPopupMode();

	int GetFloatPopupDirection (CBCGPToolbarMenuButton* pButton);

	void AccNotifyObjectFocusEvent ();

protected:
	void RestoreMaximizeMode (BOOL bRecalcLayout = TRUE);

	static BOOL FindMenuItemText (HMENU hMenu, const UINT nID, CString& strText);

	void CalcSysButtonSize ();
	
	virtual BOOL BuildOrigItems (UINT uiMenuResID); // Required for the SmartResourceUpdate
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPMenuBar)
	//}}AFX_VIRTUAL

public:
	virtual BOOL Create(CWnd* pParentWnd,
			DWORD dwStyle = dwDefaultToolbarStyle,
			UINT nID = BCG_IDW_MENUBAR);
	virtual BOOL CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle = TBSTYLE_FLAT,
			DWORD dwStyle = dwDefaultToolbarStyle,
			CRect rcBorders = CRect(1, 1, 1, 1),
			UINT nID = BCG_IDW_MENUBAR);

	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual CSize CalcLayout(DWORD dwMode, int nLength = -1);

	virtual int CalcMaxButtonHeight ();

	virtual void AdjustLocations ();
	virtual BOOL OnSendCommand (const CBCGPToolbarButton* pButton);

	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);

	virtual BOOL CanBeRestored () const
	{
		return TRUE;
	}

	virtual BOOL CanBeClosed () const
	{
		return FALSE;
	}

	virtual BOOL AllowChangeTextLabels () const
	{
		return FALSE;
	}

	virtual BOOL IsButtonExtraSizeAvailable () const
	{
		return FALSE;
	}

	virtual BOOL AllowShowOnControlMenu () const
	{	
		return FALSE;	
	}

	virtual void OnDefaultMenuLoaded (HMENU) {};
	virtual BOOL RestoreOriginalstate ();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	virtual int GetRowHeight () const
	{
		return max (globalData.GetTextHeight (GetCurrentAlignment () & CBRS_ORIENT_HORZ),
			m_bHaveButtons ?
			GetButtonSize ().cy :
				m_sizeMenuButton.cy <= 0 ? 
					m_sizeButton.cy - 2 : m_sizeMenuButton.cy - 2);
	}

	virtual int GetColumnWidth () const
	{
		return m_bHaveButtons ?
			GetButtonSize ().cx :
				m_sizeMenuButton.cx <= 0 ? 
					m_sizeButton.cx - 2 : m_sizeMenuButton.cx - 2;
	}

	virtual BOOL OnSetDefaultButtonText (CBCGPToolbarButton* pButton);
	virtual void OnChangeHot (int iHot);
	
	virtual int GetAvailableExpandSize () const 
	{
		return m_bExclusiveRow ? 0xFFFF /*unlimited size*/ : 
								 CBCGPToolBar::GetAvailableExpandSize ();
	}

	virtual void OnCustomizeMode (BOOL bSet);

protected:
	virtual int FindDropIndex (const CPoint point, CRect& rectDrag) const;
	virtual void ResetImages ();

	virtual BOOL IsPureMenuButton (CBCGPToolbarButton* pButton) const;

	static void UpdateMDIChildrenMenus (CMultiDocTemplate* pTemplate);

	// Implementation
public:
	virtual ~CBCGPMenuBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPMenuBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnCancelMode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPMENUBAR_H__644FE23B_D1DB_11D1_A649_00A0C93A70EC__INCLUDED_)
