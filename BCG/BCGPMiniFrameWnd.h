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

#if !defined(AFX_BCGPMINIFRAMEWND_H__0197A4DC_E9A9_40C3_AF02_1DAC9FC8DB7E__INCLUDED_)
#define AFX_BCGPMINIFRAMEWND_H__0197A4DC_E9A9_40C3_AF02_1DAC9FC8DB7E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPMiniFrameWnd.h : header file
//

#include "BCGCBPro.h"
#include "BCGPToolBar.h"
#include "BCGPCaptionButton.h"
#include "BCGPDragFrameImpl.h"
#include "BCGPShadowManager.h"

#pragma warning( disable : 4100 34 )

/////////////////////////////////////////////////////////////////////////////
// CBCGPMiniFrameWnd window
class CBCGPControlBar;
class CBCGPBaseControlBar;
class CBCGPDockingControlBar;
class CBCGPPopupMenu;

#define BCGP_CAPTION_BTN_PIN		0x1
#define BCGP_CAPTION_BTN_CLOSE		0x2
#define BCGP_CAPTION_BTN_MENU		0x4
#define BCGP_CAPTION_BTN_MAXIMIZE	0x8
#define BCGP_CAPTION_BTN_CUSTOMIZE  0x10

typedef enum BCGP_PREDOCK_STATE
{
	BCGP_PDS_NOTHING,
	BCGP_PDS_DOCK_REGULAR,
	BCGP_PDS_DOCK_TO_TAB
};

static const UINT BCGP_DOCK_EVENT = 4;
static const UINT BCGP_CHECK_ROLL_STATE = 5;

static int g_nCaptionButtonMargin = 0;
static int g_nCaptionButtonSpacing = 4;
static const int g_nToolbarBorderSize = 3;

extern UINT BCGPM_CHECKEMPTYMINIFRAME;

class BCGCBPRODLLEXPORT CBCGPMiniFrameWnd : public CWnd
{
	DECLARE_SERIAL(CBCGPMiniFrameWnd)
	
	friend class CBCGPControlBar;
	friend class CBCGPDockManager;
	friend class CBCGPBaseTabWnd; 
	friend class CBCGPPopupMenu;
	friend class CBCGPToolBar;

// Construction
public:
	CBCGPMiniFrameWnd();

// Attributes
public:
	CPoint	GetHotPoint () const {return m_dragFrameImpl.m_ptHot;}
	void	SetHotPoint (CPoint& ptNew) {m_dragFrameImpl.m_ptHot = ptNew;}
	BOOL	IsCaptured () const {return m_bCaptured;}

	virtual BOOL CanBeDockedToBar (const CBCGPDockingControlBar* pDockingBar) const;
	virtual BOOL IsRollDown () const;
	virtual BOOL IsRollUp () const;

	virtual int	GetCaptionHeight () const {return m_nCaptionHeight;}

	virtual BOOL CanBeAttached () const 
	{
		CBCGPControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPControlBar, GetControlBar ());
		if (pBar != NULL)
		{
			return pBar->CanBeAttached ();
		}
		return FALSE;
	}

	bool	m_bBlockMove;

	static UINT m_nRollTimeOut;
	static BOOL m_bUseSaveBits;

	void SetDockManager (CBCGPDockManager* pManager)
	{
		m_pDockManager = pManager;
	}

	CBCGPDockManager* GetDockManager () const
	{
		return m_pDockManager;
	}

	CWnd* GetParent () 
	{
		return CWnd::FromHandlePermanent (m_hParentWnd);
	}

	BOOL IsActive() const { return m_bActive; }

// Operations
public:
	static void BringAllMiniFramesToWorkArea (int nVisibleWidth = -1);

	virtual void AddControlBar     (CBCGPBaseControlBar* pWnd);
	virtual void RemoveControlBar  (CBCGPBaseControlBar* pWnd, BOOL bDestroy = FALSE,
									BOOL bNoDelayedDestroy = FALSE);
	virtual void ReplaceControlBar (CBCGPBaseControlBar* pBarOrg, 
									CBCGPBaseControlBar* pBarReplaceWith);
	static BOOL AddRemoveBarFromGlobalList (CBCGPBaseControlBar* pWnd, BOOL bAdd);
	static void RedrawAll ();

	virtual CBCGPDockingControlBar* DockControlBar (BOOL& bWasDocked);
	virtual void CheckGripperVisibility () {}

	virtual void SetCaptionButtons (DWORD dwButtons);

	virtual BOOL SetPreDockState (BCGP_PREDOCK_STATE preDockState, 
								   CBCGPBaseControlBar* pBarToDock = NULL,
								   BCGP_DOCK_METHOD dockMethod = BCGP_DM_MOUSE);

	BCGP_PREDOCK_STATE GetPreDockState () { return m_preDockStateCurr; }

	virtual void OnMoveControlBar (CBCGPControlBar* pBar, CPoint ptOffset);

	virtual void OnDockToRecentPos ();

	virtual BOOL OnBeforeDock ();

	virtual int GetControlBarCount () const {return (CWnd::FromHandlePermanent (m_hEmbeddedBar) != NULL) ? 1 : 0;}
	virtual int GetVisibleBarCount () const 
	{
		if (IsWindow (m_hEmbeddedBar) && (::GetWindowLong (m_hEmbeddedBar, GWL_STYLE) & WS_VISIBLE))
		{
			return 1;
		}
		return 0;
	}

	virtual CWnd* GetControlBar () const;
	virtual CWnd* GetFirstVisibleBar () const;

    virtual BCGP_DOCK_TYPE GetDockMode () const;
	virtual void SizeToContent ();

	virtual CString GetCaptionText ();
	virtual void CalcBorderSize (CRect& rectBorderSize) const;
	virtual void CalcExpectedDockedRect (CWnd* pWndToDock, CPoint ptMouse, 
											CRect& rectResult, BOOL& bDrawTab, 
											CBCGPDockingControlBar** ppTargetBar)
	{
		bDrawTab = FALSE;
		rectResult.SetRectEmpty ();
	}

	virtual CBCGPBaseControlBar* ControlBarFromPoint (CPoint point, int nSencitivity,
													  BOOL bCheckVisibility);

	virtual void GetCaptionRect (CRect& rectCaption) const;

	virtual void OnBarRecalcLayout ();
	virtual void AdjustLayout ();

	void SetDockTimer (UINT nTimeOut);
	void KillDockTimer ();

	static CBCGPBaseControlBar* FindFloatingBarByID (UINT nID);
	static CBCGPMiniFrameWnd* FrameFromPoint (CPoint pt, int nSencitivity, 
											  CBCGPMiniFrameWnd* pFrameToExclude = NULL, 
											  BOOL bFloatMultiOnly = FALSE);

	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL, UINT uiID = (UINT) -1);
	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, UINT uiID = (UINT) -1);

	virtual void Serialize (CArchive& ar);

	virtual void SetDockState (CBCGPDockManager* pDockManager);
	virtual void StoreRecentDockInfo (CBCGPControlBar* pBar);
	virtual void StoreRecentTabRelatedInfo (CBCGPDockingControlBar* pDockingBar, 
									CBCGPDockingControlBar* pTabbedBar);

	CRect GetRecentFloatingRect () const {return m_rectRecentFloatingRect;}

	// does nothing - derived class should adjust container (if any)
	virtual void OnShowControlBar (CBCGPDockingControlBar* pBar, BOOL bShow) {}

	virtual BCGNcHitTestType HitTest (CPoint point, BOOL bDetectCaption);

	virtual void RemoveNonValidBars ();

	void Pin (BOOL bPin = TRUE)
	{
		m_bPinned = bPin;
	}

	BOOL GetPinState () const {return m_bPinned;} 

	BOOL StartTearOff (CBCGPPopupMenu* pMenu);

	virtual void OnSetRollUpTimer ();
	virtual void OnKillRollUpTimer ();

	virtual void AdjustBarFrames () {}
	virtual void ConvertToTabbedDocument ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPMiniFrameWnd)
	public:
	virtual BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, CCreateContext* pContext = NULL);
	virtual BOOL CreateEx(DWORD dwStyleEx, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, CCreateContext* pContext = NULL);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	virtual void OnCapture (BOOL bOnOff);
	virtual void EnterDragMode (HWND hWndToDestroyOnRelease = NULL); 	

	static void GetControlBarList (CObList& lstBars, CRuntimeClass* pRTCFilter = NULL,
								   BOOL bIncludeTabs = FALSE);

// Implementation
public:
	virtual ~CBCGPMiniFrameWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPMiniFrameWnd)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcDestroy();
	afx_msg void OnCancelMode();
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	afx_msg LRESULT OnCheckEmptyState (WPARAM, LPARAM);
	afx_msg LRESULT OnFloatStatus(WPARAM wParam, LPARAM);
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
	afx_msg LRESULT OnExitSizeMove (WPARAM, LPARAM);
	afx_msg BOOL OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	afx_msg BCGNcHitTestType OnNcHitTest(CPoint point);
	afx_msg LRESULT OnBCGUpdateToolTips(WPARAM, LPARAM);
	afx_msg LRESULT OnChangeVisualManager(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

protected:
	void MoveDragFrame ();
	BOOL MoveMiniFrame ();
	virtual void OnDrawBorder (CDC* pDC);
	virtual void OnDrawCaptionButtons (CDC* pDC);

	virtual void SaveRecentFloatingState ();

	virtual BOOL OnCloseMiniFrame ();
	virtual void CloseMiniFrame ();
	virtual void OnPressButtons (UINT nHit);

	virtual CBCGPDockingControlBar* DockControlBarStandard (BOOL& bWasDocked);

	virtual void AddButton (UINT nHit);
	void RemoveButton (UINT nHit);
	void ShowButton (UINT nHit, BOOL bShow = TRUE);
	CBCGPCaptionButton* FindButton (CPoint point) const;
	CBCGPCaptionButton* FindButton (UINT uiHit) const;

	void ReplaceButton (UINT nHit, UINT nHitNew);
	void RemoveAllCaptionButtons ();
	void ArrangeCaptionButtons ();
	void SetCaptionButtonsToolTips (BOOL bForceRebuild = FALSE);

	void RedrawCaptionButton (CBCGPCaptionButton* pBtn);
	virtual void OnTrackCaptionButtons (CPoint point);
	virtual void StopCaptionButtonsTracking ();

	CSize CalcAlignedRect (CRect& rectWnd, CBCGPDockBar* pDockBar = NULL);

	BOOL IsCustModeAndNotFloatingToolbar () const;
	int  GetCaptionButtonTotalWidth () const;

	virtual void CalcMinSize (CSize& sizeBase, MINMAXINFO FAR* lpMMI);
	virtual void OnCheckRollState ();

	void SetRollUpTimer ();
	void KillRollupTimer ();

	virtual void RecalcCaptionHeight ();
	void UpdateTooltips ();

	void ForceAdjustLayout (CBCGPDockManager* pDockManager);

	void SetMaximized();
	void RestoreFromMaximized();
	BOOL IsMaximized () const;

	BOOL CreateShadow(BOOL bRepos);

protected:
	BOOL					m_bActive;
	UINT					m_nCaptionHeight;

	CObList					m_lstCaptionButtons;
	DWORD					m_dwCaptionButtons;

	CToolTipCtrl*			m_pToolTip;

	CBCGPDockingControlBar*	m_pPreDockBar;

	HWND					m_hEmbeddedBar;

	bool					m_bCaptured;

	BOOL					m_bIsMoving;
	CPoint					m_ptHot; 
	

	BOOL					m_bEnablePreDockState;

	BCGP_PREDOCK_STATE		m_preDockStateCurr;
	bool					m_bTabDragRectDisplayed;

	CSize					m_sizeMinSize;

	UINT					m_nHit;	// Clicked button
	UINT					m_nHot;	// "Hot" button

	CRect					m_rectRedraw;
	CRect					m_rectRestore;

	UINT					m_nDockTimerID;
	UINT					m_nRollTimerID;
	BOOL					m_bRolledUp;
	int						m_nHeightBeforeRollUp;
	BOOL					m_bPinned;

	CBCGPDragFrameImpl		m_dragFrameImpl;	

	BOOL					m_bNoDelayedDestroy;	
	HWND					m_hParentWnd;
	HWND					m_hWndToDestroyOnRelease;
	HWND					m_hLastFocusWnd;

	BOOL					m_bHostsToolbar;

	CBCGPDockManager*		m_pDockManager; // docking manager related to embedded control bar's frame. Should be set on creation of miniframe

	static CMap<UINT,UINT,HWND,HWND>	m_mapFloatingBars;
	static CList<HWND, HWND>			CBCGPMiniFrameWnd::m_lstFrames;

	static CFrameWnd*	m_pParentWndForSerialize; // should be set by caller before starting serialization

	// loaded from archive for setting dock state
	UINT					m_nRestoredEmbeddedBarID;

	CRect m_rectRecentFloatingRect;

	void AdjustShadow();
	CBCGPShadowManager*	m_pShadow;
};

#ifndef InitUserToobars
#define InitUserToobars	InitUserToolbars
#endif

#pragma warning( default : 4100 34 )

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPMINIFRAMEWND_H__0197A4DC_E9A9_40C3_AF02_1DAC9FC8DB7E__INCLUDED_)
