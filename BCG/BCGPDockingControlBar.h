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

#if !defined(AFX_BCGPDOCKINGCONTROLBAR_H__988A2ECD_5517_4CCA_BFBA_0F6E297A2A12__INCLUDED_)
#define AFX_BCGPDOCKINGCONTROLBAR_H__988A2ECD_5517_4CCA_BFBA_0F6E297A2A12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPDockingControlBar.h : header file
//

#include "BCGCBPro.h"
#include "BCGPControlBar.h"
#include "MenuImages.h"
#include "BCGPBarContainer.h"
#include "BCGPCaptionButton.h"
#include "BCGPMultiMiniFrameWnd.h"

#define CBCGSIZINGCONTROLBAR_BUTTONS_NUM	3	// Hide + Expand + Menu

// auto hide timer events
static const UINT BCGP_AUTO_HIDE_SLIDE_IN_EVENT = 1;
static const UINT BCGP_AUTO_HIDE_SLIDE_OUT_EVENT = 2;
static const UINT ID_CHECK_AUTO_HIDE_CONDITION = 3;

// scroll timer event
static const UINT BCGP_SCROLL_TIMER_ID = 6;

class CBCGPDockingControlBar;
class CBCGPTabbedControlBar;
class CBCGPBarContainerManager;


/////////////////////////////////////////////////////////////////////////
// CBCGPDockingControlBar control bar styles

#define SCBS_EDGELEFT       0x00000001
#define SCBS_EDGERIGHT      0x00000002
#define SCBS_EDGETOP        0x00000004
#define SCBS_EDGEBOTTOM     0x00000008
#define SCBS_EDGEALL        0x0000000F
#define SCBS_SHOWEDGES      0x00000010
#define SCBS_GRIPPER        0x00000020

static const DWORD dwDefaultBCGDockingBarStyle = 
	CBRS_BCGP_FLOAT | CBRS_BCGP_CLOSE | CBRS_BCGP_RESIZE | CBRS_BCGP_AUTOHIDE;

class CBCGPTabbedControlBar;
class CBCGPSlider;
class CBCGPAutoHideToolBar;
class CBCGPAutoHideButton;

extern BCGCBPRODLLEXPORT UINT BCGM_ON_PRESS_CLOSE_BUTTON;

class BCGCBPRODLLEXPORT CBCGPDockingControlBar : public CBCGPControlBar  
{
	friend class CBCGPDockManager;
	friend class CBCGPMiniFrameWnd;
	friend class CBCGPMultiMiniFrameWnd;
	friend class CBCGPBaseTabbedBar;
	friend class CBCGPTabbedControlBar;
	friend class CBCGPMainClientAreaWnd;

	DECLARE_SERIAL(CBCGPDockingControlBar);
public:
	CBCGPDockingControlBar();
	virtual ~CBCGPDockingControlBar();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPDockingControlBar)
	public:
	virtual BOOL Create(LPCTSTR lpszCaption, CWnd* pParentWnd, const RECT& rect, BOOL bHasGripper, UINT nID, DWORD dwStyle, DWORD dwTabbedStyle = CBRS_BCGP_REGULAR_TABS, DWORD dwBCGStyle = dwDefaultBCGDockingBarStyle, CCreateContext* pContext = NULL);
	virtual BOOL CreateEx(DWORD dwStyleEx, LPCTSTR lpszCaption, CWnd* pParentWnd, const RECT& rect, BOOL bHasGripper, UINT nID, DWORD dwStyle, DWORD dwTabbedStyle = CBRS_BCGP_REGULAR_TABS, DWORD dwBCGStyle = dwDefaultBCGDockingBarStyle, CCreateContext* pContext = NULL);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL
public:	
    virtual BOOL Create(LPCTSTR lpszWindowName, CWnd* pParentWnd,
        CSize sizeDefault, BOOL bHasGripper, UINT nID,
        DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_HIDE_INPLACE, 
		DWORD dwTabbedStyle = CBRS_BCGP_REGULAR_TABS,
		DWORD dwBCGStyle = dwDefaultBCGDockingBarStyle);

	virtual void OnUpdateCmdUI(class CFrameWnd *pTarget, int bDisableIfNoHndler);

	virtual BOOL IsDocked () const;

	virtual int GetCaptionHeight () const;

	virtual void GetTabArea (CRect& rectTabAreaTop, 
							 CRect& rectTabAreaBottom) const
	{
		rectTabAreaTop.SetRectEmpty ();
		rectTabAreaBottom.SetRectEmpty ();
	}

	virtual BOOL IsTabLocationBottom () const
	{
		return TRUE;
	}

	virtual void OnBeforeChangeParent (CWnd* pWndNewParent, BOOL bDelay = FALSE);
	virtual void OnAfterChangeParent  (CWnd* pWndOldParent);

	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

	virtual int HitTest (CPoint point, BOOL bDetectCaption = FALSE);

	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);

	virtual BOOL IsResizable () const
	{
		return TRUE;
	}
	virtual BOOL IsFloatingMulti () const;
	virtual BOOL CanAcceptBar (const CBCGPBaseControlBar* pBar) const
	{
		if (pBar == NULL)
		{
			return FALSE;
		}

		return (pBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar)) && 
			    (IsDocked () || IsFloatingMulti ())) ;
				
	}

	virtual BOOL CanAcceptMiniFrame (CBCGPMiniFrameWnd* pMiniFrame) const;

	virtual BOOL CanBeAttached () const 
	{
		return TRUE;
	}
	virtual BOOL IsAutoHideMode () const
	{
		return m_bPinState;
	}
	virtual BOOL HasAutoHideMode () const
	{
		return TRUE;
	}

	virtual BOOL IsAutohideAllEnabled () const
	{
		return m_bEnableAutoHideAll;
	}

	void EnableAutohideAll (BOOL bEnable = TRUE)
	{
		m_bEnableAutoHideAll = bEnable;
	}

	virtual BOOL IsVisible () const;
	virtual void Slide (BOOL bSlideOut, BOOL bUseTimer = TRUE);

	void SetAutoHideParents (CBCGPAutoHideToolBar* pToolBar, 
							 CBCGPAutoHideButton* pBtn);
	void SetResizeMode (BOOL bResize);

	virtual void ToggleAutoHide ();

	static const CSize& GetDragSencitivity ()
	{
		return m_sizeDragSencitivity;
	}

	virtual CBCGPControlBar* DockControlBarStandard (BOOL& bWasDocked);

	virtual BCGP_CS_STATUS IsChangeState (int nOffset, CBCGPBaseControlBar** ppTargetBar) const;

	virtual BCGP_CS_STATUS GetDockStatus (CPoint pt, int nSencitivity);

	virtual CBCGPDockingControlBar* AttachToTabWnd (CBCGPDockingControlBar* pTabControlBarAttachTo, 
													BCGP_DOCK_METHOD dockMethod,
													BOOL bSetActive = TRUE, 
													CBCGPDockingControlBar** ppTabbedControlBar = NULL);

	virtual CBCGPTabbedControlBar* CreateTabbedControlBar ();

	void SetDefaultSlider (HWND hSliderWnd);
	
	CBCGPSlider* GetDefaultSlider () const; 

	virtual BOOL DockToFrameWindow (DWORD dwAlignment, LPCRECT lpRect = NULL, 
									DWORD dwDockFlags = BCGP_DT_DOCK_LAST, 
									CBCGPBaseControlBar* pRelativeBar = NULL, 
									int nRelativeIndex = -1, 
									BOOL bOuterEdge = FALSE);
	virtual BOOL DockToWindow (CBCGPDockingControlBar* pTargetWindow, DWORD dwAlignment, 
							   LPCRECT lpRect = NULL);
	virtual BOOL DockContainer (CBCGPBarContainerManager& barContainerManager, 
								DWORD dwAlignment, BCGP_DOCK_METHOD dockMethod);

	virtual void UnDockControlBar (BOOL bDelay = FALSE);

	virtual void OnAfterDockFromMiniFrame  ()  
	{
		OnAfterDock (this, NULL, BCGP_DM_UNKNOWN);
	}

	virtual void UnSetAutoHideMode (CBCGPDockingControlBar* pFirstBarInGroup);
	BOOL ReplaceControlBar (CBCGPDockingControlBar* pBarToReplaceWith, BCGP_DOCK_METHOD dockMethod, 
							BOOL bRegisterWithFrame = FALSE);

	void RemoveFromDefaultSlider ();

	virtual void ShowControlBar (BOOL bShow, BOOL bDelay, BOOL bActivate/* = TRUE*/); 
	virtual BOOL DockToResentPos ()
	{
		return Dock (NULL, NULL, BCGP_DM_SHOW);
	}

	virtual void EnableGripper (BOOL bEnable);

	virtual CBCGPAutoHideToolBar* SetAutoHideMode (BOOL bMode, DWORD dwAlignment, 
													CBCGPAutoHideToolBar* pCurrAutoHideBar = NULL, 
													BOOL bUseTimer = TRUE);

	static void SetCaptionStyle (BOOL bDrawText, BOOL bForceGradient = FALSE,
								 BOOL bHideDisabledButtons = FALSE);
	
	static BOOL IsDrawCaption ()
	{
		return m_bCaptionText;
	}

	static BOOL IsHideDisabledButtons ()
	{
		return m_bHideDisabledButtons;
	}

	static CBCGPSlider* CreateDefaultSlider (DWORD dwAlignment, CWnd* pParent, 
											 CRuntimeClass* pSliderRTC = NULL);

	static BOOL	m_bHideInAutoHideMode;
	static BOOL	m_bDisableAnimation;

	virtual BOOL IsHideInAutoHideMode () const 
	{
		return CBCGPDockingControlBar::m_bHideInAutoHideMode;
	}

	virtual BOOL CanAutoHide () const;

	BOOL IsTracked () const
	{
		return m_bPrepareToFloat;
	}

	void SetLastPercentInContainer (int n) {m_nLastPercent = n;}
	int  GetLastPercentInContainer () const {return m_nLastPercent;}
	void GetRecentSiblingBarInfo (CList<UINT, UINT&>& lstBarIDs);

	CRect GetAHRestoredRect () const {return m_rectRestored;}
	virtual BOOL CanAdjustLayout () const 
	{
		return !m_bIsSliding || !m_bIsHiding;
	}

	virtual UINT GetAHSlideMode () const
	{
		return m_ahSlideMode;
	}

	CBCGPAutoHideToolBar* GetAutoHideToolBar () const {return m_pAutoHideBar;}
	CBCGPAutoHideButton* GetAutoHideButton () {return m_pAutoHideButton;}

	virtual void Serialize (CArchive& ar);
	virtual void CopyState (CBCGPDockingControlBar* pOrgBar);

	virtual BOOL OnBeforeFloat (CRect& rectFloat, BCGP_DOCK_METHOD dockMethod);

	void SetTabbedControlBarRTC (CRuntimeClass* pRTC)
	{
		ASSERT (pRTC != NULL);
		m_pTabbedControlBarRTC = pRTC;
	}

	void SetRestoredDefaultSlider (HWND hRestoredSlider)
	{
		m_hRestoredDefaultSlider = hRestoredSlider;
	}

	void RestoreDefaultSlider ()
	{
		SetDefaultSlider (m_hRestoredDefaultSlider);
	}

	CRuntimeClass* GetTabbedControlBarRTC () const
	{
		return m_pTabbedControlBarRTC;
	}

	virtual void ConvertToTabbedDocument (BOOL bActiveTabOnly = TRUE);

	virtual BOOL IsAccessibilityCompatible () { return FALSE; }

	static int	m_nTimeOutBeforeAutoHide;
	static int	m_nSlideSteps;
	//---------------------------------
protected:
	//{{AFX_MSG(CBCGPDockingControlBar)
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	afx_msg BCGNcHitTestType OnNcHitTest(CPoint point);
	afx_msg LRESULT OnBCGUpdateToolTips(WPARAM, LPARAM);
	afx_msg LRESULT OnRedrawFrame(WPARAM, LPARAM);
	afx_msg LRESULT OnChangeVisualManager(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

protected:
	virtual BOOL Dock (CBCGPBaseControlBar* pTargetBar, LPCRECT lpRect, BCGP_DOCK_METHOD dockMethod);
	virtual void OnContinueMoving () {} // do nothing - resizable control bar doesn't move after docking
	virtual void OnDrawDragRect (LPCRECT lprectNew, LPCRECT lprectOld);
	virtual void OnAfterDock  (CBCGPBaseControlBar* /*pBar*/, LPCRECT /*lpRect*/, BCGP_DOCK_METHOD /*dockMethod*/); 
	virtual void OnChangeActiveState() {}
	
	virtual void EnterDragMode (BOOL bChangeHotPoint);
	virtual void StoreRecentDockInfo ();
	virtual void StoreRecentTabRelatedInfo ();

	virtual void OnPressCloseButton ();
	virtual void OnPressButtons (UINT /*nHit*/) {}

	virtual void DoNcPaint(CDC* pDC);

	virtual void OnEraseNCBackground (CDC* pDC, CRect rectBar);
	virtual void DrawCaption (CDC* pDC, CRect rectCaption);
	virtual void AdjustBarWindowToContainer (CBCGPSlider* pSlider);

	virtual void UpdateScrollBars() {}
	virtual BOOL ScrollHorzAvailable(BOOL /*bLeft*/) {	return FALSE;	}
	virtual BOOL ScrollVertAvailable(BOOL /*bTop*/) {	return FALSE;	}
	virtual void OnScrollClient(UINT /*uiScrollCode*/) {}
	virtual CPoint GetScrollPos() const {	return CPoint(0, 0);	}

	void RedrawButton (const CBCGPCaptionButton* pBtn);
	CBCGPCaptionButton* FindButton (CPoint point) const;
	CBCGPCaptionButton* FindButtonByHit (UINT nHit) const;
	void EnableButton (UINT nHit, BOOL bEnable = TRUE);
	BOOL IsButtonEnabled (UINT nHit) const;

	virtual void OnTrackCaptionButtons (CPoint point);
	virtual void StopCaptionButtonsTracking ();

	virtual void SetCaptionButtons ();
	void RemoveCaptionButtons ();

	void CalcScrollButtons();
	int GetScrollButtonSize() const;
	
	void UpdateTooltips ();

	CRect	m_rectCloseButton;

	/// how far the mouse is moved after NcLBtnDown before the bar becomes floating
	static CSize m_sizeDragSencitivity; 

	bool					m_bPrepareToFloat;
	bool					m_bReadyToFloat;
	int						m_cyGripper;
	BOOL					m_bHasGripper; // indicates that the bar has caption (it may not be shown when floating)
	int						m_nBorderSize;
	BOOL					m_bActive;
	DWORD					m_dwSCBStyle;
	CRect					m_rectRedraw;

	int						m_nLastPercent;

	BOOL					m_bEnableAutoHideAll;

	//---------- auto hide support ----------
	BOOL					m_bPinState;
	UINT_PTR				m_nAutoHideConditionTimerID;
	UINT_PTR				m_nSlideTimer;
	int						m_nSlideStep;
	int						m_nSlideDelta;
	static int				m_nSlideDefaultTimeOut;
	CRect					m_rectRestored;
	
	BOOL					m_bIsSliding;	
	BOOL					m_bIsResizing;
	BOOL					m_bIsHiding;	
	
	UINT					m_ahSlideMode;

	CBCGPAutoHideButton*	m_pAutoHideButton;
	CBCGPAutoHideToolBar*	m_pAutoHideBar;

	virtual BOOL CheckAutoHideCondition ();
	virtual BOOL CheckStopSlideCondition (BOOL bDirection);
	virtual void OnSlide (BOOL bSlideOut);

	void CalcRectForSliding (CRect& rect, CRect& rectSlider, BOOL bSlideDirection);
	void OffsetRectForSliding (CRect& rect, BOOL bSlideDirection, BOOL bIsRTL);
	CPoint CalcCorrectOffset (CRect rect, BOOL bIsRTL);

	//----------------------------------------

	CTypedPtrArray<CObArray, CBCGPCaptionButton*>	m_arrButtons;

	//---------------- scroll support ----------------

	CArray<CBCGPDockingBarScrollButton*, CBCGPDockingBarScrollButton*>	m_arScrollButtons;
	static int m_nScrollTimeOut;

	static BOOL		m_bCaptionText;
	static BOOL		m_bHideDisabledButtons;

	CRuntimeClass*	m_pTabbedControlBarRTC;
	
	HWND			m_hDefaultSlider;
	HWND			m_hRestoredDefaultSlider;

	UINT			m_nHot;
	UINT			m_nHit;
	BOOL			m_bCaptionButtonsCaptured;

	CToolTipCtrl*			m_pToolTip;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPDOCKINGCONTROLBAR_H__988A2ECD_5517_4CCA_BFBA_0F6E297A2A12__INCLUDED_)
