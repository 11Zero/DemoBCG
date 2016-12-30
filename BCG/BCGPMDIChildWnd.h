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

#if !defined(AFX_BCGPMDICHILDWND_H__3BE44BE6_C83D_11D3_A723_009027900694__INCLUDED_)
#define AFX_BCGPMDICHILDWND_H__3BE44BE6_C83D_11D3_A723_009027900694__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGMDIChildWnd.h : header file
//

#include "BCGCBPro.h"
#include "BCGPFrameImpl.h"

class CBCGPMDIFrameWnd;
class CBCGPMDIChildWnd;

/////////////////////////////////////////////////////////////////////////////
// CBCGPTaskBarPreviewParameters

struct BCGCBPRODLLEXPORT CBCGPTaskBarPreviewParameters
{
	BOOL	m_bIsThumbnail;
	CRect	m_rectDraw;
	BOOL	m_bDontStretch;
	CSize	m_sizeDest;

	CBCGPTaskBarPreviewParameters()
	{
		m_bIsThumbnail = FALSE;
		m_rectDraw.SetRectEmpty();
		m_bDontStretch = FALSE;
		m_sizeDest = CSize(0, 0);
	}
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPMDITabProxyWnd

class CBCGPMDITabProxyWnd : public CWnd
{
	DECLARE_DYNCREATE(CBCGPMDITabProxyWnd)

public:
	CBCGPMDITabProxyWnd();
	virtual ~CBCGPMDITabProxyWnd();

	void SetRelatedMDIChildFrame(CBCGPMDIChildWnd* pRelatedMDIFrame);
	HBITMAP GetClientBitmap(int nWidth, int nHeight, BOOL bIsThumbnail);

	static BCGCBPRODLLEXPORT BOOL OnPrepareScrollViewPreview(CDC* pDC, CScrollView* pScrollView);

protected:
	afx_msg LRESULT OnSendIconicThumbnail(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSendIconicLivePreviewBitmap(WPARAM wParam, LPARAM lParam);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()

	BOOL IsMDIChildActive();

	CBCGPMDIChildWnd* m_pRelatedMDIChildFrame;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPMDIChildWnd frame

class BCGCBPRODLLEXPORT CBCGPMDIChildWnd : public CMDIChildWnd
{
	friend class CBCGPMainClientAreaWnd;
	friend class CBCGPGlobalUtils;
	friend class CBCGPMDIFrameWnd;
	friend class CBCGPMDITabProxyWnd;

	DECLARE_DYNCREATE(CBCGPMDIChildWnd)
protected:
	CBCGPMDIChildWnd();           // protected constructor used by dynamic creation

// Attributes
public:
	static BOOL			m_bEnableFloatingBars;
	static DWORD		m_dwExcludeStyle;

	BOOL IsTabbedControlBar () const
	{
		return m_pTabbedControlBar != NULL;
	}

	CBCGPDockingControlBar* GetTabbedControlBar () const
	{
		return m_pTabbedControlBar;
	}

	BOOL IsPrintPreview ()
	{
		return m_dockManager.IsPrintPreviewValid ();
	}

protected:
	// ---- MDITabGroup+
	CBCGPTabWnd*		m_pRelatedTabGroup;
	// ---- MDITabGroup-

	CBCGPFrameImpl		m_Impl;
	CBCGPMDIFrameWnd*	m_pMDIFrame;
	BOOL				m_bToBeDestroyed;
	CBCGPDockManager	m_dockManager;

	BOOL				m_bWasMaximized;
	BOOL				m_bIsMinimized;

	CRect				m_rectOriginal;

	// set during OnMDIActivate to prevent unnecessary 
	// RecalcLayout in CMDIFrameWnd in OLE InPlace mode
	BOOL					m_bActivating;

	CBCGPDockingControlBar*	m_pTabbedControlBar;

	CBCGPMDITabProxyWnd		m_tabProxyWnd;
	BOOL					m_bTabRegistered;
	BOOL					m_bEnableTaskbarThumbnailClip;

// Operations
public:
		// ---- MDITabGroup+
	CBCGPTabWnd*		GetRelatedTabGroup () {return m_pRelatedTabGroup;}
	void				SetRelatedTabGroup (CBCGPTabWnd* p)  {m_pRelatedTabGroup = p;}
	// ---- MDITabGroup-

	void AddDockBar			();
	BOOL AddControlBar		(CBCGPBaseControlBar* pControlBar, BOOL bTail = TRUE);
	BOOL InsertControlBar	(CBCGPBaseControlBar* pControlBar, 
							 CBCGPBaseControlBar* pTarget, BOOL bAfter = TRUE);
	void RemoveControlBarFromDockManager (CBCGPBaseControlBar* pControlBar, BOOL bDestroy,
							 BOOL bAdjustLayout, BOOL bAutoHide, CBCGPBaseControlBar* pBarReplacement);
	void DockControlBar		(CBCGPBaseControlBar* pBar, UINT nDockBarID = 0, 
							 LPCRECT lpRect = NULL);

	void AddTabbedControlBar (CBCGPDockingControlBar* pControlBar);

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

	virtual BOOL GetToolbarButtonToolTipText (CBCGPToolbarButton* /*pButton*/, CString& /*strTTText*/)
	{
		return FALSE;
	}


	BOOL DockControlBarLeftOf(CBCGPControlBar* pBar, CBCGPControlBar* pLeftOf);

	void DisableThemeCaption()
	{
		ASSERT(GetSafeHwnd() == NULL);
		m_Impl.m_bDisableThemeCaption = TRUE;
	}

// Overrides

	// Next methods used by MDI tabs:
	virtual CString GetFrameText () const;
	virtual HICON GetFrameIcon () const;

	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

	virtual BOOL CanShowOnMDITabs ()	{	return TRUE;	}
	virtual BOOL CanShowOnWindowsList (){	return TRUE;	}
	virtual BOOL IsReadOnly ()			{	return FALSE;	}

	CBCGPDockManager* GetDockManager () {	return &m_dockManager;	}

	virtual LPCTSTR GetDocumentName (CObject** pObj);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPMDIChildWnd)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void ActivateFrame(int nCmdShow = -1);
	virtual void OnSetPreviewMode(BOOL bPreview, CPrintPreviewState* pState);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	virtual void AdjustClientArea ();

	// Windows 7 Taskbar support:
public:
	virtual void RegisterTaskbarTab(CBCGPMDIChildWnd* pWndBefore = NULL);
	virtual void UnregisterTaskbarTab(BOOL bCheckRegisteredMDIChildCount = TRUE);

	BOOL IsRegisteredWithTaskbarTabs();
	BOOL InvalidateIconicBitmaps();
	BOOL IsTaskbarTabsSupportEnabled();

	void SetTaskbarTabOrder(CBCGPMDIChildWnd* pWndBefore = NULL);
	void SetTaskbarTabProperties(UINT uiFlags, BOOL bInvalidateIconicBitmaps = FALSE);
	void SetTaskbarTabActive();

protected:
	virtual HBITMAP OnGetIconicThumbnail(int /*nWidth*/, int /*nHeight*/) { return NULL; }
	virtual HBITMAP OnGetIconicLivePreviewBitmap(BOOL /*bIsMDIChildActive*/, CPoint& /*ptLocation*/) { return NULL; }

	virtual void ActivateTopLevelFrame();
	virtual void OnTaskbarTabThumbnailActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	virtual int OnTaskbarTabThumbnailMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	virtual void OnPressTaskbarThmbnailCloseButton();
	virtual CWnd* GetTaskbarPreviewWnd();
	virtual BOOL SetTaskbarThumbnailClipRect(CRect rect);

	virtual void UpdateTaskbarTabIcon(HICON hIcon);
	virtual CRect GetTaskbarThumbnailClipRect() const;

	CBCGPMDITabProxyWnd* GetTabProxyWnd() { return &m_tabProxyWnd;}

	void EnableTaskbarThumbnailClipRect(BOOL bEnable = TRUE);

	virtual BOOL CanShowOnTaskBarTabs() { return TRUE; }

// Implementation
protected:
	virtual ~CBCGPMDIChildWnd();

	// Generated message map functions
	//{{AFX_MSG(CBCGPMDIChildWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnDestroy();
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg BCGNcHitTestType OnNcHitTest(CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnNcRButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	//}}AFX_MSG
	afx_msg LRESULT OnSetText(WPARAM,LPARAM);
	afx_msg LRESULT OnSetIcon(WPARAM,LPARAM);
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam = 0, LPARAM lParam = 0);
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	afx_msg LRESULT OnChangeVisualManager (WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	
};

extern BCGCBPRODLLEXPORT UINT BCGM_ON_PREPARE_TASKBAR_PREVIEW;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_AFTER_TASKBAR_ACTIVATE;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPMDICHILDWND_H__3BE44BE6_C83D_11D3_A723_009027900694__INCLUDED_)
