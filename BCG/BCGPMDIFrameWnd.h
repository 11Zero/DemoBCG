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

#if !defined(AFX_BCGPMDIFRAMEWND_H__08B9EC04_DCE3_11D1_A64F_00A0C93A70EC__INCLUDED_)
#define AFX_BCGPMDIFRAMEWND_H__08B9EC04_DCE3_11D1_A64F_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// BCGPMDIFrameWnd.h : header file
//

#include "BCGCBPro.h"
#include "BCGPFrameImpl.h"
#include "BCGPMainClientAreaWnd.h"
#include "BCGPPopupMenu.h"
#include "BCGPDockManager.h"

class COleClientItem;
class CBCGPPopupMenu;
class CBCGPToolBar;
class CBCGPToolbarMenuButton;
class CBCGPToolbarButton;

static const DWORD BCGP_MDI_CREATE_VERT_GROUP	= 0x001;
static const DWORD BCGP_MDI_CREATE_HORZ_GROUP	= 0x002;
static const DWORD BCGP_MDI_CAN_MOVE_PREV		= 0x004;
static const DWORD BCGP_MDI_CAN_MOVE_NEXT		= 0x008;
static const DWORD BCGP_MDI_CAN_BE_DOCKED		= 0x010;

/////////////////////////////////////////////////////////////////////////////
// CBCGPMDIFrameWnd frame

class BCGCBPRODLLEXPORT CBCGPMDIFrameWnd : public CMDIFrameWnd
{
	friend class CBCGPMainClientAreaWnd;
	friend class CBCGPToolBar;
	friend class CBCGPMenuBar;
	friend class CBCGPToolbarsPage;
	friend class CBCGPWorkspace;
	friend class CBCGPPopupMenu;
	friend class CBCGPToolbarCustomize;
	friend class CBCGPMiniFrameWnd;
	friend class CBCGPMDIChildWnd;
	friend class CBCGPGlobalUtils;
	friend class CBCGPTabWnd;
	friend class CBCGPRibbonButton;
	friend class CBCGPWinApp;
	friend class CBCGPShadowManager;

	DECLARE_DYNCREATE(CBCGPMDIFrameWnd)

protected:
	CBCGPMDIFrameWnd();           // protected constructor used by dynamic creation

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

	void SetPrintPreviewFrame (CFrameWnd* pWnd)
	{
		if (pWnd != NULL)
		{
			ASSERT_VALID (pWnd);
		}
		m_pPrintPreviewFrame = pWnd;
	}

	BOOL IsPrintPreview ()
	{
		return m_Impl.IsPrintPreview ();
	}
	
	void ClosePrintPreview();

	void ActiveItemRecalcLayout ();

	static BOOL m_bDisableSetRedraw;

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
	BOOL					m_bIsTlbCustMode;
	CBCGPMainClientAreaWnd	m_wndClientArea;
	HMENU					m_hmenuWindow;
	CBCGPFrameImpl			m_Impl;
	CBCGPDockManager		m_dockManager; // THE DockManager
	BOOL					m_bContextHelp;
	BOOL					m_bDoSubclass;
	UINT					m_uiWindowsDlgMenuId;
	CString					m_strWindowsDlgMenuText;
	BOOL					m_bShowWindowsDlgAlways;
	BOOL					m_bShowWindowsDlgHelpButton;
	BOOL					m_bCanCovertControlBarToMDIChild;

	BOOL					m_bWasMaximized;
	BOOL					m_bIsMinimized;

	BOOL					m_bClosing;
	int						m_nFrameID; // need for uniqueness when saving state in MTI 

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
	BOOL IsPointNearDockBar (CPoint point, DWORD& dwBarAlignment, 
							BOOL& bOuterEdge) const;
	virtual void AdjustDockingLayout (HDWP hdwp = NULL);
	BOOL EnableDocking (DWORD dwDockStyle);	
	BOOL EnableAutoHideBars (DWORD dwDockStyle, BOOL bActivateOnMouseClick = FALSE);

	void EnableMaximizeFloatingBars(BOOL bEnable = TRUE, BOOL bMaximizeByDblClick = FALSE);
	BOOL AreFloatingBarsCanBeMaximized() const;

	CBCGPBaseControlBar* GetControlBar (UINT nID);
	void ShowControlBar (CBCGPBaseControlBar* pBar, BOOL bShow, BOOL bDelay, BOOL bActivate);

	virtual BOOL OnMoveMiniFrame	(CWnd* pFrame);
	virtual void RecalcLayout (BOOL bNotify = TRUE);
	
	//-------------------- MDI Tab Groups interface ---
	DWORD GetMDITabsContextMenuAllowedItems ()
	{
		return m_wndClientArea.GetMDITabsContextMenuAllowedItems ();
	}

	BOOL IsMemberOfMDITabGroup (CWnd* pWnd)
	{
		return m_wndClientArea.IsMemberOfMDITabGroup (pWnd);
	}

	void MDITabMoveToNextGroup (BOOL bNext = TRUE)
	{
		m_wndClientArea.MDITabMoveToNextGroup (bNext);
	}

	void MDITabNewGroup (BOOL bVert = TRUE)
	{
		m_wndClientArea.MDITabNewGroup (bVert);
	}

	const CObList& GetMDITabGroups () const 
	{
		return m_wndClientArea.GetMDITabGroups ();
	}
	
	virtual BOOL OnShowMDITabContextMenu (CPoint point, DWORD dwAllowedItems, BOOL bTabDrop);

	virtual CBCGPTabWnd* CreateMDITabGroup() const
	{
		return new CBCGPTabWnd;
	}

	//-------------------------------------------------

	BOOL DockControlBarLeftOf (CBCGPControlBar* pBar, CBCGPControlBar* pLeftOf);

	void SetupToolbarMenu (	CMenu& menu,
							const UINT uiViewUserToolbarCmdFirst,
							const UINT uiViewUserToolbarCmdLast)
	{
		m_Impl.SetupToolbarMenu (menu, uiViewUserToolbarCmdFirst, uiViewUserToolbarCmdLast);
	}

	void EnableWindowsDialog (UINT uiMenuId, 
		LPCTSTR lpszMenuText,
		BOOL bShowAllways = FALSE,
		BOOL bShowHelpButton = FALSE);
	void EnableWindowsDialog (UINT uiMenuId, 
		UINT uiMenuTextResId,
		BOOL bShowAllways = FALSE,
		BOOL bShowHelpButton = FALSE);
		// Enable built-in CBCGPWindowsManagerDlg dialog. When bShowAllways
		// is TRUE, show windows dialog everytime; otherwise only instead of
		// the statndard "Windows..." dialog (appear if more than 9 windows 
		// are open)
	void ShowWindowsDialog ();
	
	// Enable/Disable MDI child tabs:
	void EnableMDITabs (BOOL bEnable = TRUE, BOOL bIcons = TRUE,
						CBCGPTabWnd::Location tabLocation = CBCGPTabWnd::LOCATION_BOTTOM,
						BOOL bTabCloseButton = FALSE,
						CBCGPTabWnd::Style style = CBCGPTabWnd::STYLE_3D_SCROLLED,
						BOOL bTabCustomTooltips = FALSE,
						BOOL bActiveTabCloseButton = FALSE);

	void EnableMDITabbedGroups (BOOL bEnable = TRUE, const CBCGPMDITabParams& params = CBCGPMDITabParams());
	const CBCGPMDITabParams& GetMDITabbedGroupsParams() const			
	{
		return m_wndClientArea.m_mdiTabParams;
	}

	BOOL AreMDITabs (int* pnMDITabsType = NULL) const
	{
		if (pnMDITabsType != NULL)
		{
			*pnMDITabsType = 0;
			if (m_wndClientArea.m_bTabIsEnabled)
			{
				*pnMDITabsType = 1;
			}
			else if (m_wndClientArea.IsMDITabbedGroup ())
			{
				*pnMDITabsType = 2;
			}
		}
		return m_wndClientArea.m_bTabIsEnabled || m_wndClientArea.IsMDITabbedGroup ();
	}

	BOOL IsMDITabbedGroup () const
	{
		return m_wndClientArea.IsMDITabbedGroup ();
	}


	CBCGPTabWnd& GetMDITabs ()
	{
		return m_wndClientArea.GetMDITabs ();
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

	void EnableMDITabsLastActiveActivation(BOOL bLastActiveTab = TRUE)
	{
		m_wndClientArea.EnableMDITabsLastActiveActivation(bLastActiveTab);
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

	virtual CBCGPMDIChildWnd* ControlBarToTabbedDocument (CBCGPDockingControlBar* pBar);
	virtual BOOL TabbedDocumentToControlBar (CBCGPMDIChildWnd* pMDIChildWnd);

	void UpdateMDITabbedBarsIcons ();

	virtual BOOL CanCovertControlBarToMDIChild ()
	{
		return m_bCanCovertControlBarToMDIChild;
	}

	void SetRibbonBackstageView (CBCGPRibbonBackstageView* pView)
	{
		m_Impl.SetRibbonBackstageView(pView);
	}

	BOOL IsRibbonBackstageView() const
	{
		return m_Impl.m_pBackstageView != NULL;
	}

	BOOL CloseRibbonBackstageView();

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
	virtual BOOL OnSetMenu (HMENU hmenu);

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

	void SetDockState(const CDockState& state)
	{
		m_Impl.SetDockState (state);
	}

	CBCGPDockManager* GetDockManager () {return &m_dockManager;}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPMDIFrameWnd)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	virtual void WinHelp(DWORD_PTR dwData, UINT nCmd = HELP_CONTEXT);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	virtual HMENU GetWindowMenuPopup (HMENU hMenuBar);
#if _MSC_VER >= 1300
	virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd = 0x000F);
#endif

	BOOL ShowPopupMenu (CBCGPPopupMenu* pMenuPopup);

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

	virtual BOOL OnEraseMDIClientBackground (CDC* /*pDC*/)
	{
		return FALSE;
	}

	virtual void OnSizeMDIClient (const CRect& /*rectOld*/, const CRect& /*rectNew*/)	{}

	virtual void OnDrawMenuLogo (CDC* /*pDC*/, CBCGPPopupMenu* /*pMenu*/, const CRect& /*rectLogo*/)	{}
	virtual void OnUpdateFrameMenu (HMENU hMenuAlt);

	virtual BOOL OnTearOffMenu (CBCGPPopupMenu* /*pMenuPopup*/, CBCGPControlBar* /*pBar*/)
	{
		return TRUE;
	}

	virtual void OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState);
	virtual BOOL OnShowControlBars (BOOL bShow);
	virtual BOOL NegotiateBorderSpace( UINT nBorderCmd, LPRECT lpRectBorder );

	void ShowFloatingBars(BOOL bShow)
	{
		m_dockManager.ShowFloatingBars(bShow);
	}

	virtual void AdjustClientArea ();
	virtual BOOL OnCloseMiniFrame (CBCGPMiniFrameWnd* /*pWnd*/) 
	{
		return TRUE;
	}
	virtual BOOL OnCloseDockingBar (CBCGPDockingControlBar* /*pWnd*/)
	{
		return TRUE;
	}

	virtual BOOL LoadMDIState (LPCTSTR lpszProfileName);
	virtual BOOL SaveMDIState (LPCTSTR lpszProfileName);

	virtual CBCGPMDIChildWnd* CreateDocumentWindow (LPCTSTR lpcszDocName, CObject* pObj);
	virtual CBCGPMDIChildWnd* CreateNewWindow (LPCTSTR lpcszDocName, CObject* pObj);

	virtual void OnCancelWndCapture(CWnd* pWndCapture);

	// Taskbar interaction support:
	void RegisterAllMDIChildrenWithTaskbar(BOOL bRegister = TRUE);
	int GetRegisteredWithTaskBarMDIChildCount();

// Implementation
protected:
	virtual ~CBCGPMDIFrameWnd();
	virtual COleClientItem*	GetInPlaceActiveItem ();

	// Generated message map functions
	//{{AFX_MSG(CBCGPMDIFrameWnd)
	afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
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

	afx_msg LRESULT OnToolbarCreateNew(WPARAM,LPARAM);
	afx_msg LRESULT OnToolbarDelete(WPARAM,LPARAM);
	afx_msg void OnContextHelp ();
	afx_msg void OnUpdateControlBarMenu(CCmdUI* pCmdUI);
	afx_msg BOOL OnBarCheck(UINT nID);
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
	LRESULT OnExitSizeMove (WPARAM, LPARAM);
	afx_msg void OnWindowNew();
	afx_msg LRESULT OnToolbarContextMenu(WPARAM,LPARAM);
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChangeVisualManager (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPostPreviewFrame (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDWMCompositionChanged(WPARAM,LPARAM);
	afx_msg LRESULT OnPowerBroadcast(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnAfterTaskbarActivate(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

#ifndef InitUserToobars
#define InitUserToobars	InitUserToolbars
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPMDIFRAMEWND_H__08B9EC04_DCE3_11D1_A64F_00A0C93A70EC__INCLUDED_)
