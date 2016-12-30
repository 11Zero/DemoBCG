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

#if !defined(AFX_BCGPFRAMEWND_H__829B77B4_FE0E_11D1_8BCB_00A0C9B05590__INCLUDED_)
#define AFX_BCGPFRAMEWND_H__829B77B4_FE0E_11D1_8BCB_00A0C9B05590__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BCGFrameWnd.h : header file
//

#include "BCGPFrameImpl.h"
#include "BCGCBPro.h"
#include "BCGPPopupMenu.h"

#include "BCGPDockManager.h"

class CBCGPMenuBar;
class CBCGPToolbarMenuButton;
class CBCGPToolbarButton;

/////////////////////////////////////////////////////////////////////////////
// CBCGPFrameWnd frame

class BCGCBPRODLLEXPORT CBCGPFrameWnd : public CFrameWnd
{
	friend class CBCGPToolBar;
	friend class CBCGPMenuBar;
	friend class CBCGPToolbarsPage;
	friend class CBCGPFrameImpl;
	friend class CBCGPWorkspace;
	friend class CBCGPPopupMenu;
	friend class CBCGPMiniFrameWnd;
	friend class CBCGPGlobalUtils;
	friend class CBCGPShadowManager;

	DECLARE_DYNCREATE(CBCGPFrameWnd)

protected:
	CBCGPFrameWnd();           // protected constructor used by dynamic creation

// Attributes
public:
	BOOL IsMenuBarAvailable () const
	{
		return m_Impl.GetMenuBar () != NULL;
	}

	const CBCGPMenuBar* GetMenuBar () const
	{
		return m_Impl.GetMenuBar ();
	}

	UINT GetDefaultResId () const
	{
		return m_Impl.m_nIDDefaultResource;
	}

	CBCGPPopupMenu*	GetActivePopup () const
	{
		return CBCGPPopupMenu::m_pActivePopupMenu;
	}

	const CObList& GetTearOffBars () const
	{
		return m_Impl.m_listTearOffToolbars;
	}

	CBCGPRibbonBar* GetRibbonBar ()
	{
		return m_Impl.GetRibbonBar ();
	}

	BOOL IsClosing () const
	{
		return m_bClosing;
	}

	UINT GetControlbarsMenuEntryID() const
	{
		return m_Impl.m_uiControlbarsMenuEntryID;
	}

protected:
	BOOL				m_bIsTlbCustMode;
	HMENU				m_hmenuWindow;
	CBCGPFrameImpl		m_Impl;
	CBCGPDockManager	m_dockManager; // THE DockManager
	BOOL				m_bContextHelp;
	BOOL				m_bWasMaximized;
	BOOL				m_bIsMinimized;
	BOOL				m_bClosing;

	// workaround for Pront Preview bug in VS 7.1
	CFrameWnd*				m_pPrintPreviewFrame;

// Operations
public:
	// Input mode:
	void SetInputMode(BCGP_INPUT_MODE mode) { m_Impl.SetInputMode(mode); }
	BCGP_INPUT_MODE GetInputMode() const { return m_Impl.m_InputMode; }

	//------------------ dockmanager layer ------------
	void AddDockBar			();
	BOOL AddControlBar		(CBCGPBaseControlBar* pControlBar, BOOL bTail = TRUE);
	BOOL InsertControlBar	(CBCGPBaseControlBar* pControlBar, 
							 CBCGPBaseControlBar* pTarget, BOOL bAfter = TRUE);
	void RemoveControlBarFromDockManager (CBCGPBaseControlBar* pControlBar, BOOL bDestroy, 
							 BOOL bAdjustLayout, BOOL bAutoHide, CBCGPBaseControlBar* pBarReplacement);
	void DockControlBar		(CBCGPBaseControlBar* pBar, UINT nDockBarID = 0, 
							 LPCRECT lpRect = NULL);

	CBCGPBaseControlBar* ControlBarFromPoint (CPoint point, int nSensitivity, 
											  bool bExactBar, 
											   CRuntimeClass* pRTCBarType) const;
	CBCGPBaseControlBar* ControlBarFromPoint (CPoint point, int nSensitivity, 
											  DWORD& dwAlignment, 
											  CRuntimeClass* pRTCBarType) const;
	BOOL IsPointNearDockBar (CPoint point, DWORD& dwBarAlignment, BOOL& bOuterEdge) const;
	virtual void AdjustDockingLayout (HDWP hdwp = NULL);
	BOOL EnableDocking (DWORD dwDockStyle);	
	BOOL EnableAutoHideBars (DWORD dwDockStyle, BOOL bActivateOnMouseClick = FALSE);

	void EnableMaximizeFloatingBars(BOOL bEnable = TRUE, BOOL bMaximizeByDblClick = FALSE);
	BOOL AreFloatingBarsCanBeMaximized() const;

	CBCGPBaseControlBar* GetControlBar (UINT nID);
	void ShowControlBar (CBCGPBaseControlBar* pBar, BOOL bShow, BOOL bDelay, BOOL bActivate);
	
	virtual BOOL OnMoveMiniFrame	(CWnd* pFrame);
	virtual void RecalcLayout (BOOL bNotify = TRUE);

	BOOL DockControlBarLeftOf (CBCGPControlBar* pBar, CBCGPControlBar* pLeftOf);

	void SetupToolbarMenu (	CMenu& menu,
							const UINT uiViewUserToolbarCmdFirst,
							const UINT uiViewUserToolbarCmdLast)
	{
		m_Impl.SetupToolbarMenu (menu, uiViewUserToolbarCmdFirst, uiViewUserToolbarCmdLast);
	}

	void SetDockState(const CDockState& state)
	{
		m_Impl.SetDockState (state);
	}

	BOOL IsFullScreen() const
	{
		return m_Impl.m_FullScreenMgr.IsFullScreen();
	}

	void ShowFullScreen()
	{
		m_Impl.m_FullScreenMgr.ShowFullScreen();
	}

	void EnableFullScreenMainMenu(BOOL bEnableMenu)
	{
		m_Impl.m_FullScreenMgr.EnableMainMenu(bEnableMenu);
	}

	void EnableFullScreenAutoHideTaskbar (BOOL bEnable = TRUE)
	{
		m_Impl.m_FullScreenMgr.EnableAutoHideTaskbar (bEnable);
	}

	void EnableFullScreenMode(UINT uiFullScreenCmd)
	{
		m_Impl.m_FullScreenMgr.SetFullScreenID(uiFullScreenCmd);
	}

	CBCGPToolBar* GetFullScreenToolBar () const
	{
		return m_Impl.m_FullScreenMgr.GetFullScreenBar ();
	}

	void EnableLoadDockState(BOOL bEnable = TRUE)
	{
		m_Impl.m_bLoadDockState = bEnable;
	}

	CBCGPDockManager* GetDockManager ()
	{
		return &m_dockManager;
	}

	void SetPrintPreviewFrame (CFrameWnd* pWnd)
	{
		if (pWnd != NULL)
		{
			ASSERT_VALID (pWnd);
		}

		m_pPrintPreviewFrame = pWnd;
	}

	void EnableControlBarMenu (
		BOOL bEnable, UINT uiCustomizeCmd, const CString& strCustomizeLabel,
		UINT uiViewToolbarsMenuEntryID, 
		BOOL bContextMenuShowsToolbarsOnly = FALSE,
		BOOL bViewMenuShowsToolbarsOnly = FALSE,
		BOOL bIncludeRebarPanes = FALSE)
	{
		m_dockManager.EnableControlBarContextMenu (bEnable, uiCustomizeCmd, strCustomizeLabel, bContextMenuShowsToolbarsOnly, bIncludeRebarPanes);
		m_Impl.SetControlbarsMenuId (uiViewToolbarsMenuEntryID, bViewMenuShowsToolbarsOnly);
	}

	void UpdateCaption ()
	{
		m_Impl.UpdateCaption ();
	}

	BOOL IsPrintPreview ()
	{
		return m_Impl.IsPrintPreview ();
	}

	void ClosePrintPreview();

	void DisableThemeCaption()
	{
		ASSERT(GetSafeHwnd() == NULL);
		m_Impl.m_bDisableThemeCaption = TRUE;
	}

	// Win 7 taskbar interaction:
	BOOL SetTaskBarProgressValue(int nCompleted, int nTotal);
	BOOL SetTaskBarProgressState(BCGP_TBPFLAG tbpFlags);
	BOOL SetTaskBarOverlayIcon(UINT nIDResource, LPCTSTR lpcszDescr = NULL);
	BOOL SetTaskBarOverlayIcon(HICON hIcon, LPCTSTR lpcszDescr = NULL);
	void ClearTaskBarOverlayIcon();

protected:

// Implementation:
	void InitUserToolbars (LPCTSTR lpszRegEntry, UINT uiUserToolbarFirst, UINT uiUserToolbarLast)
	{
		m_Impl.InitUserToolbars (lpszRegEntry, uiUserToolbarFirst, uiUserToolbarLast);
	}

	void LoadUserToolbars ()
	{
		m_Impl.LoadUserToolbars ();
	}

	void SaveUserToolbars ()
	{
		m_Impl.SaveUserToolbars ();
	}

	void LoadTearOffMenus ()
	{
		m_Impl.LoadTearOffMenus ();
	}

	void SaveTearOffMenus ()
	{
		m_Impl.SaveTearOffMenus ();
	}

	BOOL IsDockStateValid (const CDockState& state)
	{
		return m_Impl.IsDockStateValid (state);
	}

	CBCGPToolBar* GetUserBarByIndex (int iIndex) const
	{
		return m_Impl.GetUserBarByIndex (iIndex);
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPFrameWnd)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	virtual void WinHelp(DWORD_PTR dwData, UINT nCmd = HELP_CONTEXT);
	virtual void OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

#if _MSC_VER >= 1300
	virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd = 0x000F);
#endif

// Implementation
protected:
	virtual ~CBCGPFrameWnd();

	BOOL OnSetMenu (HMENU hmenu);
	BOOL ShowPopupMenu (CBCGPPopupMenu* pMenuPopup);

	virtual void AdjustClientArea ();
	virtual COleClientItem*	GetInPlaceActiveItem ();

public:
	virtual BOOL OnShowPopupMenu (CBCGPPopupMenu* /*pMenuPopup*/);

	virtual BOOL OnShowCustomizePane(CBCGPPopupMenu* pMenuPane, UINT uiToolbarID)
	{
		m_Impl.AddDefaultButtonsToCustomizePane(pMenuPane, uiToolbarID);

		return TRUE;
	}

	virtual void OnClosePopupMenu (CBCGPPopupMenu* pMenuPopup);
	virtual BOOL OnDrawMenuImage (	CDC* pDC, 
									const CBCGPToolbarMenuButton* pMenuButton, 
									const CRect& rectImage);

	virtual BOOL OnMenuButtonToolHitTest (CBCGPToolbarButton* /*pButton*/, TOOLINFO* /*pTI*/)
	{
		return FALSE;
	}

	virtual BOOL GetToolbarButtonToolTipText (CBCGPToolbarButton* /*pButton*/, CString& /*strTTText*/)
	{
		return FALSE;
	}

	virtual void OnDrawMenuLogo (CDC* /*pDC*/, CBCGPPopupMenu* /*pMenu*/, const CRect& /*rectLogo*/)	{}

	virtual BOOL OnTearOffMenu (CBCGPPopupMenu* /*pMenuPopup*/, CBCGPControlBar* /*pBar*/)
	{
		return TRUE;
	}

	virtual void OnUpdateFrameMenu (HMENU hMenuAlt);

	virtual BOOL OnShowControlBars (BOOL bShow);
	virtual BOOL NegotiateBorderSpace( UINT nBorderCmd, LPRECT lpRectBorder );

	void ShowFloatingBars(BOOL bShow)
	{
		m_dockManager.ShowFloatingBars(bShow);
	}

	virtual void DelayUpdateFrameMenu(HMENU hMenuAlt);
	virtual BOOL OnCloseMiniFrame (CBCGPMiniFrameWnd* /*pWnd*/) 
	{
		return TRUE;
	}
	virtual BOOL OnCloseDockingBar (CBCGPDockingControlBar* /*pWnd*/)
	{
		return TRUE;
	}

	void ActiveItemRecalcLayout ();
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

	void SetRibbonBackstageView (CBCGPRibbonBackstageView* pView)
	{
		m_Impl.SetRibbonBackstageView(pView);
	}

	BOOL CloseRibbonBackstageView();

	BOOL IsRibbonBackstageView() const
	{
		return m_Impl.m_pBackstageView != NULL;
	}

	// Generated message map functions
	//{{AFX_MSG(CBCGPFrameWnd)
	afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg BCGNcHitTestType OnNcHitTest(CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	#if _MSC_VER >= 1300
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	#else
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	#endif

	afx_msg LRESULT OnSetMenu (WPARAM wp, LPARAM);
	afx_msg LRESULT OnToolbarCreateNew(WPARAM,LPARAM);
	afx_msg LRESULT OnToolbarDelete(WPARAM,LPARAM);
	afx_msg void OnContextHelp ();
	afx_msg void OnUpdateControlBarMenu(CCmdUI* pCmdUI);
	afx_msg BOOL OnBarCheck(UINT nID);
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam = 0, LPARAM lParam = 0);
	LRESULT OnExitSizeMove (WPARAM, LPARAM);
	afx_msg LRESULT OnToolbarContextMenu(WPARAM,LPARAM);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChangeVisualManager (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPostPreviewFrame (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDWMCompositionChanged(WPARAM,LPARAM);
	afx_msg LRESULT OnPowerBroadcast(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
};

#ifndef InitUserToobars
#define InitUserToobars	InitUserToolbars
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPFRAMEWND_H__829B77B4_FE0E_11D1_8BCB_00A0C9B05590__INCLUDED_)
