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

#if !defined(AFX_BCGPPOPUPMENU_H__A6E1AD75_D511_11D1_A64D_00A0C93A70EC__INCLUDED_)
#define AFX_BCGPPOPUPMENU_H__A6E1AD75_D511_11D1_A64D_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BCGPPopupMenu.h : header file
//

#include "BCGCBPro.h"
#include "BCGPPopupMenuBar.h"
#include "BCGPScrollBar.h"
#include "BCGPControlRenderer.h"

class CBCGPToolbarMenuButton;
class CBCGPMenuBar;
class CBCGPMenuPage;
class CBCGPBaseRibbonElement;
class CBCGPShadowWnd;

/////////////////////////////////////////////////////////////////////////////
// CBCGPPopupMenu frame

class BCGCBPRODLLEXPORT CBCGPPopupMenu : public CMiniFrameWnd
{
	friend class CCustomizeButton;
	friend class CBCGPToolbarMenuButton;
	friend class CBCGPMenuPage;
	friend class CBCGPFrameWnd;
	friend class CBCGPMDIFrameWnd;
	friend class CBCGPOleIPFrameWnd;
	friend class CBCGPOleDocIPFrameWnd;
	friend class CBCGPFrameImpl;
	friend class CBCGPDlgImpl;
	friend class CBCGPContextMenuManager;
	friend class CBCGPBaseRibbonElement;
	friend class CBCGPPopupMenuBar;
	friend class CBCGPRibbonBar;
	friend class CBCGPRibbonFloaty;
	friend class CBCGPRibbonPanelMenuBar;

	DECLARE_SERIAL(CBCGPPopupMenu)

public:
	CBCGPPopupMenu();
	virtual ~CBCGPPopupMenu();

protected:
	CBCGPPopupMenu(CBCGPMenuPage* pCustPage, LPCTSTR lpszTitle);	// For customization only

// Attributes
public:
	enum ANIMATION_TYPE
	{
		NO_ANIMATION,
		UNFOLD,
		SLIDE,
		FADE,
		SYSTEM_DEFAULT_ANIMATION = 999,
	};

	enum MENUAREA_TYPE
	{
		MENU,
		SHADOW_RIGHT,
		SHADOW_BOTTOM,
		LOGO,
		OUTSIDE,
		TEAROFF_CAPTION
	};

	enum DROP_DIRECTION
	{
		DROP_DIRECTION_NONE,
		DROP_DIRECTION_BOTTOM,
		DROP_DIRECTION_TOP,
		DROP_DIRECTION_RIGHT,
		DROP_DIRECTION_LEFT,
	};

	enum LOGO_LOCATION
	{
		MENU_LOGO_LEFT,
		MENU_LOGO_RIGHT,
		MENU_LOGO_TOP,
		MENU_LOGO_BOTTOM,
	};

	enum QUICK_CUSTOMIZE_TYPE
	{
		QUICK_CUSTOMIZE_PANE,
		QUICK_CUSTOMIZE_ADDREMOVE,
		QUICK_CUSTOMIZE_NONE,
	};

	enum MENU_RESIZE_TYPE
	{
		MENU_RESIZE_BOTTOM,
		MENU_RESIZE_BOTTOM_RIGHT,
		MENU_RESIZE_TOP,
		MENU_RESIZE_TOP_RIGHT,
	};

	BOOL m_bAutoDestroyParent;
	HMENU GetHMenu () const
	{
		return m_hMenu;
	}

	virtual CBCGPPopupMenuBar* GetMenuBar ()
	{
		return &m_wndMenuBar;
	}

	void SetAutoDestroy (BOOL bAutoDestroy = TRUE)
	{
		m_bAutoDestroy = bAutoDestroy;
	}

	CBCGPToolbarMenuButton* GetParentButton () const
	{
		return m_pParentBtn;
	}

	CBCGPBaseRibbonElement*	GetParentRibbonElement () const
	{
		return m_pParentRibbonElement;
	}

    static void SetForceShadow(BOOL bValue)
    {
        m_bForceShadow = bValue;
    }

    static BOOL GetForceShadow()
    {
        return m_bForceShadow;
    }

    static void SetForceMenuFocus (BOOL bValue)
    {
        m_bForceMenuFocus = bValue;
    }

    static BOOL GetForceMenuFocus ()
    {
        return m_bForceMenuFocus;
    }

	static CBCGPPopupMenu::ANIMATION_TYPE GetAnimationType (BOOL bNoSystem = FALSE)
	{
		if (m_AnimationType == SYSTEM_DEFAULT_ANIMATION && !bNoSystem)
		{
			if (globalData.m_bMenuAnimation)
			{
				return globalData.m_bMenuFadeEffect ? FADE : SLIDE; 
			}
			else
			{
				return NO_ANIMATION;
			}
		}

		return m_AnimationType;
	}

	static void SetAnimationType (CBCGPPopupMenu::ANIMATION_TYPE type)
	{
		m_AnimationType = type;
	}

	static void SetAnimationSpeed (UINT nElapse);

	static UINT GetAnimationSpeed ()
	{
		return m_AnimationSpeed;
	}

	static void EnableMenuSound (BOOL bEnable = TRUE)
	{
		m_bMenuSound = bEnable;
	}

	static UINT IsMenuSound ()
	{
		return m_bMenuSound;
	}

	static void AlwaysShowEmptyToolsEntry (BOOL bShow = TRUE)
	{
		m_bAlwaysShowEmptyToolsEntry = bShow;
	}

	static BOOL IsAlwaysShowEmptyToolsEntry ()
	{
		return m_bAlwaysShowEmptyToolsEntry;
	}

	static void SetSendMenuSelectMsg (BOOL bSet = TRUE)
	{
		m_bSendMenuSelectMsg = bSet;
	}

	static BOOL IsSendMenuSelectMsg ()
	{
		return m_bSendMenuSelectMsg;
	}

	void EnableResize (CSize sizeMinResize);
	void EnableVertResize (int nMinResize);
	BOOL IsResizeble () const
	{
		return m_bIsResizable;
	}

	BOOL IsScrollable () const
	{
		return m_bScrollable;
	}

	void EnableScrolling (BOOL /*bEnable*/ = TRUE)
	{
		// This method is obsolete;
	}

	void EnableMenuLogo (int iLogoSize, LOGO_LOCATION nLogoLocation = MENU_LOGO_LEFT, 
		BOOL bDrawGutter = FALSE /* Valid for MENU_LOGO_TOP or MENU_LOGO_BOTTOM only */);

	BOOL IsDrawGutterAlongWithLogo() const
	{
		return m_bGutterLogo;
	}

	void SetMaxWidth (int iMaxWidth);

	CWnd* GetMessageWnd () const
	{
		return m_pMessageWnd; 
	}

	void SetMessageWnd (CWnd* pMsgWnd)
	{
		m_pMessageWnd = pMsgWnd;
	}

	BOOL PostCommand (UINT uiCommandID);

   void MoveTo (const CPoint& pt)
   {
	   m_ptLocation = pt;
	   RecalcLayout ();
   }

   virtual BOOL InCommand ()
   {
	   CBCGPPopupMenuBar* pBar = GetMenuBar ();
	   return pBar == NULL ? FALSE : pBar->m_bInCommand;
   }

	static CBCGPPopupMenu* GetActiveMenu ()
	{
		return m_pActivePopupMenu;
	}

	static CBCGPPopupMenu* GetSafeActivePopupMenu();

	DROP_DIRECTION GetDropDirection () const
	{
		return m_DropDirection;
	}

	void SetRightAlign (BOOL bRightAlign = TRUE)
	{
		m_bRightAlign = bRightAlign;
	}

	BOOL IsRightAlign () const
	{
		return m_bRightAlign;
	}

	BOOL IsQuickCustomize()
	{
		return m_bQuickCusomize;
	}

	void SetQuickCustomizeType(QUICK_CUSTOMIZE_TYPE Type)
	{
		m_QuickType = Type;
	}

	QUICK_CUSTOMIZE_TYPE GetQuickCustomizeType() const
	{
		return m_QuickType;
	}

	BOOL IsCustomizePane()
	{
		return m_QuickType == QUICK_CUSTOMIZE_PANE;
	}

	BOOL IsShown() const
	{
		return m_bShown;
	}

	virtual BOOL IsIdle () const
	{
		return m_bAnimationIsDone && m_iScrollMode == 0;
	}

	virtual CWnd* GetParentArea (CRect& rectParentBtn);
	void SetParentRibbonElement (CBCGPBaseRibbonElement* pElem);

	virtual BOOL IsAlwaysClose () const	{	return FALSE;	}

	BOOL HasBeenResized () const
	{
		return m_bHasBeenResized;
	}

	virtual BOOL DefaultMouseClickOnClose () const
	{
		return TRUE;
	}

protected:
	static CBCGPPopupMenu*	m_pActivePopupMenu;
	static BOOL				m_bForceMenuFocus;	// Menu takes a focus when activated
	static BOOL				m_bForceRTL;

	CWnd*					m_pMessageWnd;

	CBCGPToolbarMenuButton*	m_pParentBtn;
	DROP_DIRECTION			m_DropDirection;
	CBCGPPopupMenuBar		m_wndMenuBar;
	HMENU					m_hMenu;
	CPoint					m_ptLocation;
	CPoint					m_ptLocationInitial;
	CSize					m_FinalSize;
	int						m_nMenuBarHeight;
	BOOL					m_bAutoDestroy;
	BOOL					m_bTrackMode;
	BOOL					m_bHasBeenResized;
	BOOL					m_bRightAlign;

	BOOL					m_bShown;
	BOOL					m_bTobeDstroyed;

	int						m_iMaxWidth;
	static int				m_nMinWidth;
	int						m_iLogoWidth;
	LOGO_LOCATION			m_nLogoLocation;
	BOOL					m_bGutterLogo;

	CBCGPMenuPage* const	m_pMenuCustomizationPage;
	CString					m_strCaption;

	static BOOL				m_bMenuSound;
	static BOOL				m_bAlwaysShowEmptyToolsEntry;
	static BOOL				m_bSendMenuSelectMsg;

	int						m_nLastCommandID;

	//----------------------
	// Animation attributes:
	//----------------------
	static ANIMATION_TYPE	m_AnimationType;
	static UINT				m_AnimationSpeed;
	CSize					m_AnimSize;
	BOOL					m_bIsAnimRight;
	BOOL					m_bIsAnimDown;
	BOOL					m_bAnimationIsDone;
	BOOL					m_bDisableAnimation;

	//----------------------
	// Scrolling attributes:
	//----------------------
	BOOL					m_bScrollable;
	CRect					m_rectScrollUp;
	CRect					m_rectScrollDn;
	int						m_iScrollMode;	// -1 - Up, 0 - None, 1 - Down
	int						m_iScrollBtnHeight;
	BOOL					m_bShowScrollBar;
	int						m_nMaxHeight;
	CBCGPScrollBar			m_wndScrollBarVert;

	CBitmap					m_bmpScreenSrc;
	CBitmap					m_bmpScreenDst;
	CBitmap					m_bmpScreenTmp;

	int						m_iFadePercent;
	
	COLORREF*				m_cFadeSrcBits;
	COLORREF*				m_cFadeDstBits;
	COLORREF*				m_cFadeTmpBits;

	//-------------------
	// Shadow attributes:
	//-------------------
	int						m_iShadowSize;
    static BOOL             m_bForceShadow;     // when TRUE paints shadows even outside main-frame
	CBitmap					m_bmpShadowRight;	// Saved shadow butmaps
	CBitmap					m_bmpShadowBottom;
	CBCGPShadowWnd*			m_pWndShadow;		// Layered shadow window

	//---------------------
	// Tear-off attributes:
	//---------------------
	BOOL					m_bTearOffTracking;
	BOOL					m_bIsTearOffCaptionActive;
	CRect					m_rectTearOffCaption;
	CToolTipCtrl			m_wndToolTip;

	//-------------------
	// Resize attributes:
	//-------------------
	BOOL					m_bIsResizable;
	CSize					m_sizeMinResize;
	CRect					m_rectResize;
	BOOL					m_bResizeTracking;
	BOOL					m_bWasResized;
	CSize					m_sizeCurrent;
	BOOL					m_bIsResizeBarOnTop;

	//------------------------
	// Quick Cusomization flags
	//------------------------
	BOOL					m_bQuickCusomize;
	QUICK_CUSTOMIZE_TYPE    m_QuickType;

	BOOL					m_bEscClose;

	//-------------------
	// Ribbon attributes:
	//-------------------
	CBCGPBaseRibbonElement*	m_pParentRibbonElement;
	HWND					m_hwndConnectedFloaty;

// Operations
public:
	virtual void SaveState ();

	CBCGPPopupMenu* GetParentPopupMenu () const;
	CBCGPToolBar* GetParentToolBar () const;

	CBCGPToolbarMenuButton* GetSelItem ();

	int InsertItem (const CBCGPToolbarMenuButton& button, int iInsertAt = -1);
	int InsertSeparator (int iInsertAt = -1);

	int GetMenuItemCount () const;
	CBCGPToolbarMenuButton* GetMenuItem (int iIndex) const;
	CBCGPToolbarMenuButton* FindSubItemByCommand (UINT uiCmd) const;
	
	BOOL RemoveItem (int iIndex);
	void RemoveAllItems ();

	void SetDefaultItem (UINT uiCmd)
	{
		GetMenuBar ()->m_uiDefaultMenuCmdId = uiCmd;
	}

	static BOOL ActivatePopupMenu (CFrameWnd* pTopFrame, CBCGPPopupMenu* pPopupMenu);
	void CloseMenu (BOOL bSetFocusToBar = FALSE);
	void ShowAllCommands ();

	BOOL HideRarelyUsedCommands () const;
	BOOL AreAllCommandsShown () const
	{
		return ((CBCGPPopupMenu*) this)->GetMenuBar ()->m_bAreAllCommandsShown;
	}

	MENUAREA_TYPE CheckArea (const CPoint& ptScreen) const;

	void UpdateShadow (LPRECT lprectScreen = NULL);
	static void UpdateAllShadows (LPRECT lprectScreen = NULL); // Repaint all opened menu shadows
	void UpdateShadowTransparency (BYTE nTransparency);

	void SetQuickMode();

	BOOL IsEscClose () { return m_bEscClose;}

	void TriggerResize ();

	virtual BOOL IsFloaty () const
	{
		return FALSE;
	}

protected:
	void Initialize ();

	virtual BOOL IsScrollUpAvailable ();
	virtual BOOL IsScrollDnAvailable ();

	void CollapseSubmenus ();
	void UpdateBottomWindows (BOOL bCheckOnly = FALSE);

	virtual void DoPaint (CDC* pPaintDC);
	virtual void DrawFade (CDC* pPaintDC);
	virtual BOOL OnDrawLogo(CDC* pDC, const CRect& rectLogo);

	BOOL AdjustScroll (BOOL bForceMenuBarResize = FALSE);
	void SetScrollBar ();

	virtual BOOL InitMenuBar ();
	virtual BOOL TearOff (CPoint point);
	virtual CBCGPControlBar* CreateTearOffBar (CFrameWnd* pWndMain, UINT uiID, LPCTSTR lpszName);
	virtual void OnChooseItem (UINT /*uidCmdID*/)	{}
	virtual void OnChangeHot (int /*nHot*/) {}

	BOOL NotifyParentDlg (BOOL bActivate);

	BOOL StartResize ();

	virtual int GetBorderSize () const;

	static CBCGPPopupMenu* FindMenuWithConnectedFloaty ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPPopupMenu)
	public:
	virtual BOOL Create(CWnd* pWndParent, int x, int y, HMENU hMenu, BOOL bLocked = FALSE, BOOL bOwnMessage = FALSE);
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void PostNcDestroy();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGPPopupMenu)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BCGNcHitTestType OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
#if _MSC_VER >= 1300
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
#else
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
#endif
	DECLARE_MESSAGE_MAP()

	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName);
	virtual HRESULT get_accParent(IDispatch **ppdispParent);
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT get_accChild(VARIANT varChild, IDispatch **ppdispChild);
	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole);
	virtual HRESULT get_accState(VARIANT varChild, VARIANT *pvarState);};

class BCGCBPRODLLEXPORT CBCGPDisableMenuAnimation
{
public:
	CBCGPDisableMenuAnimation ()
	{
		m_animType = CBCGPPopupMenu::GetAnimationType ();
		CBCGPPopupMenu::SetAnimationType (CBCGPPopupMenu::NO_ANIMATION);
	}

	virtual ~CBCGPDisableMenuAnimation ()
	{
		Restore ();
	}

	void Restore ()
	{
		CBCGPPopupMenu::SetAnimationType (m_animType);
	}

protected:
	CBCGPPopupMenu::ANIMATION_TYPE m_animType;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPShadowWnd

class CBCGPShadowWnd : public CMiniFrameWnd
{
public:
	CBCGPShadowWnd(CWnd* pOwner, int nOffset, int nDepth = 0, const CSize& szCorners = CSize(0, 0))
	{
		m_pOwner  = pOwner;
		m_nOffset = abs(nOffset);
		m_nDepth  = nDepth == 0 ? m_nOffset : abs(nDepth);
		m_szCorners = szCorners;
		m_bIsRTL  = FALSE;
	}

	~CBCGPShadowWnd()
	{
	}
	
	void Repos();
	void UpdateTransparency (BYTE nTransparency);
	
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPShadowWnd)
public:
	virtual BOOL Create();
	//}}AFX_VIRTUAL
	
	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPShadowWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
	void DrawShadow ();
	
	CWnd*				m_pOwner;
	int					m_nOffset;
	int					m_nDepth;
	CSize				m_szCorners;
	CBCGPShadowRenderer m_Shadow;
	BOOL				m_bIsRTL;
	BYTE				m_nTransparency;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPOPUPMENU_H__A6E1AD75_D511_11D1_A64D_00A0C93A70EC__INCLUDED_)
