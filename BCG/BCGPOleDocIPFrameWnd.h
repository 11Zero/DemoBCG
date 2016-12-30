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

#if !defined(AFX_BCGPOLEDOCIPFRAMEWND_H__D6152532_1ACC_48E8_9A46_F9BBFB227B87__INCLUDED_)
#define AFX_BCGPOLEDOCIPFRAMEWND_H__D6152532_1ACC_48E8_9A46_F9BBFB227B87__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGOleDocIPFrameWnd.h : header file
//

#ifndef __AFXDOCOB_H__
#include <afxdocob.h>
#endif

#include "BCGCBPro.h"
#include "BCGPFrameImpl.h"
#include "BCGPPopupMenu.h"
#include "BCGPDockManager.h"

class CBCGPMenuBar;
class CBCGPToolbarMenuButton;
class CBCGPToolbarButton;

/////////////////////////////////////////////////////////////////////////////
// CBCGPOleDocIPFrameWnd frame

class BCGCBPRODLLEXPORT CBCGPOleDocIPFrameWnd : public COleDocIPFrameWnd
{
	friend class CBCGPMenuBar;
	friend class CBCGPToolBar;
	friend class CToolbarsPage;
	friend class CBCGPWorkspace;
	friend class CBCGPPopupMenu;
	friend class CBCGPMiniFrameWnd;
	friend class CBCGPGlobalUtils;
	friend class CBCGPOleCntrFrameWnd;

	DECLARE_DYNCREATE(CBCGPOleDocIPFrameWnd)

// Construction
protected:
	CBCGPOleDocIPFrameWnd();           // protected constructor used by dynamic creation

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

protected:
	HMENU				m_hmenuWindow;
	CBCGPFrameImpl		m_Impl;
	CBCGPDockManager	m_dockManager;
	BOOL				m_bContextHelp;
	HWND				m_hwndLastTopLevelFrame;

// Operations
public:

	// Input mode:
	void SetInputMode(BCGP_INPUT_MODE mode) { m_Impl.SetInputMode(mode); }
	BCGP_INPUT_MODE GetInputMode() const { return m_Impl.m_InputMode; }

	virtual BOOL OnShowControlBars (BOOL bShow);

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

	CBCGPBaseControlBar* GetControlBar (UINT nID);
	void ShowControlBar (CBCGPBaseControlBar* pBar, BOOL bShow, BOOL bDelay, BOOL bActivate);

	virtual BOOL OnMoveMiniFrame	(CWnd* pFrame);
	virtual void RecalcLayout (BOOL bNotify = TRUE);
	BOOL DockControlBarLeftOf (CBCGPControlBar* pBar, CBCGPControlBar* pLeftOf);
	//-------------------------------------------------
	
	
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

	CBCGPDockManager* GetDockManager () {return &m_dockManager;}

	CBCGPOleCntrFrameWnd* GetContainerFrameWindow();

	void EnableControlBarMenu (
		BOOL bEnable, UINT uiCustomizeCmd, const CString& strCustomizeLabel,
		UINT uiViewToolbarsMenuEntryID, 
		BOOL bContextMenuShowsToolbarsOnly = FALSE,
		BOOL bViewMenuShowsToolbarsOnly = FALSE)
	{
		m_dockManager.EnableControlBarContextMenu (bEnable, uiCustomizeCmd, strCustomizeLabel, bContextMenuShowsToolbarsOnly);
		m_Impl.SetControlbarsMenuId (uiViewToolbarsMenuEntryID, bViewMenuShowsToolbarsOnly);
	}

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
	//{{AFX_VIRTUAL(CBCGPOleDocIPFrameWnd)
	public:
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
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
	virtual BOOL OnCreateControlBars(CFrameWnd* pWndFrame, CFrameWnd* pWndDoc);

// Implementation
protected:
	virtual ~CBCGPOleDocIPFrameWnd();

	BOOL OnSetMenu (HMENU hmenu);
	BOOL ShowPopupMenu (CBCGPPopupMenu* pMenuPopup);

public:
	virtual BOOL OnShowPopupMenu (CBCGPPopupMenu* /*pMenuPopup*/)
	{
		return TRUE;
	}

	virtual BOOL OnShowCustomizePane(CBCGPPopupMenu* pMenuPane, UINT uiToolbarID)
	{
		m_Impl.AddDefaultButtonsToCustomizePane(pMenuPane, uiToolbarID);

		return TRUE;
	}

	virtual void OnClosePopupMenu (CBCGPPopupMenu* pMenuPopup);
	virtual BOOL OnDrawMenuImage (	CDC* /*pDC*/, 
									const CBCGPToolbarMenuButton* /*pMenuButton*/, 
									const CRect& /*rectImage*/)
	{
		return FALSE;
	}

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
	virtual BOOL OnCloseMiniFrame (CBCGPMiniFrameWnd* /*pWnd*/) 
	{
		return TRUE;
	}
	virtual BOOL OnCloseDockingBar (CBCGPDockingControlBar* /*pWnd*/)
	{
		return TRUE;
	}


	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPOleDocIPFrameWnd)
	afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LRESULT OnSetMenu (WPARAM wp, LPARAM);
	afx_msg LRESULT OnToolbarCreateNew(WPARAM,LPARAM);
	afx_msg LRESULT OnToolbarDelete(WPARAM,LPARAM);
	afx_msg void OnContextHelp ();
	afx_msg void OnUpdateControlBarMenu(CCmdUI* pCmdUI);
	afx_msg BOOL OnBarCheck(UINT nID);
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
	afx_msg LRESULT OnToolbarContextMenu(WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()
};

#ifndef InitUserToobars
#define InitUserToobars	InitUserToolbars
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPOLEDOCIPFRAMEWND_H__D6152532_1ACC_48E8_9A46_F9BBFB227B87__INCLUDED_)
