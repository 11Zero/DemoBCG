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

// BCGPDockManager.h: interface for the CBCGPDockManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPDOCKMANAGER_H__FEF7A0D3_B9D4_4FF4_911F_E75B71B20FDF__INCLUDED_)
#define AFX_BCGPDOCKMANAGER_H__FEF7A0D3_B9D4_4FF4_911F_E75B71B20FDF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPControlBar.h"
#include "BCGPSmartDockingManager.h"

class CBCGPDockBar;
class CBCGPAutoHideDockBar;
class CBCGPAutoHideToolBar;
class CBCGPMiniFrameWnd;
class CBCGPMultiMiniFrameWnd;
class CBCGPTabbedControlBar;
class CBCGPSlider;
class CBCGPDockingControlBar;
class CBCGPOleCntrFrameWnd;

struct BCGP_DOCKBAR_INFO
{
	DWORD			m_dwBarAlignment;
	CRuntimeClass*	pDockBarRTC;
};

struct BCGP_AUTOHIDEBAR_SAVE_INFO
{
	BCGP_AUTOHIDEBAR_SAVE_INFO ()
	{
		m_pSavedBar = NULL;
		m_rectBar.SetRectEmpty ();
		m_nBarID = (UINT)-1;
		m_bIsVisible = FALSE;
		m_dwBarAlignment = 0;
		m_bFirstInGroup = TRUE;
		m_bLastInGroup = TRUE;
		m_bActiveInGroup = TRUE;
	}

	BCGP_AUTOHIDEBAR_SAVE_INFO (BCGP_AUTOHIDEBAR_SAVE_INFO& src)
	{
		Copy (src);	
	}

	UINT	m_nBarID;
	DWORD	m_dwBarAlignment;
	BOOL	m_bIsVisible;
	CRect	m_rectBar;
	CList<UINT, UINT&> m_lstSiblingBars;

	BOOL	m_bFirstInGroup;
	BOOL	m_bLastInGroup;
	BOOL	m_bActiveInGroup;

	void Serilaize (CArchive& ar);
	BCGP_AUTOHIDEBAR_SAVE_INFO& operator= (BCGP_AUTOHIDEBAR_SAVE_INFO& src)
	{
		return Copy (src);	
	}

	BCGP_AUTOHIDEBAR_SAVE_INFO& Copy (BCGP_AUTOHIDEBAR_SAVE_INFO& src)
	{
		m_lstSiblingBars.RemoveAll ();
		m_nBarID = src.m_nBarID;
		m_dwBarAlignment = src.m_dwBarAlignment;
		m_bIsVisible = src.m_bIsVisible;
		m_rectBar = src.m_rectBar;
		m_lstSiblingBars.AddTail (&src.m_lstSiblingBars);
		m_bFirstInGroup = src.m_bFirstInGroup;
		m_bLastInGroup = src.m_bLastInGroup;
		m_bActiveInGroup = src.m_bActiveInGroup;
		return *this;
	}

	CBCGPDockingControlBar* m_pSavedBar; // must be set before serialize;
};

#define BCGP_SD_MARKERS_NUM 5

class BCGCBPRODLLEXPORT CBCGPSmartDockingParams : public CObject
{
public:
	CBCGPSmartDockingParams();

	void CopyTo (CBCGPSmartDockingParams& params);

	CSize		m_sizeTotal;			// Smart docking group bounds dimension
	int			m_nCentralGroupOffset;
	COLORREF	m_clrTransparent;
	COLORREF	m_clrToneSrc;
	COLORREF	m_clrToneDest;
	UINT		m_uiMarkerBmpResID [BCGP_SD_MARKERS_NUM];
	UINT		m_uiMarkerLightBmpResID [BCGP_SD_MARKERS_NUM];
	UINT		m_uiBaseBmpResID;
	COLORREF	m_clrBaseBackground;
	COLORREF	m_clrBaseBorder;
	BOOL		m_bUseThemeColorInShading;
	BOOL		m_bIsAlphaMarkers;
};

class BCGCBPRODLLEXPORT CBCGPDockManager : public CObject
{
	friend class CBCGPPrintPreviewView;
	friend class CBCGPMDIChildWnd;
	friend class CBCGPOleCntrFrameWnd;
	friend struct BCGPGLOBAL_DATA;
	friend class CBCGPVisualManager;
	friend class CBCGPFrameImpl;

public:
	CBCGPDockManager();
	virtual ~CBCGPDockManager();

	DWORD GetEnabledAutoHideAlignment () const {return m_dwEnabledSlideBars;}
	BOOL IsActivateAutoHideBarOnMouseClick() const { return m_bActivateAutoHideBarOnMouseClick; }

	void DockControlBar (CBCGPBaseControlBar* pBar, UINT nDockBarID = 0, 
						 LPCRECT lpRect = NULL);
	BOOL DockControlBarLeftOf (CBCGPControlBar* pBarToDock, CBCGPControlBar* pTargetBar);

	BOOL IsPointNearDockBar (CPoint point, DWORD& dwBarAlignment, BOOL& bOuterEdge) const;

	BOOL Create (CFrameWnd* pParentWnd);
	BOOL EnableDocking (DWORD dwStyle);
	BOOL EnableAutoHideBars (DWORD dwStyle, BOOL bActivateOnMouseClick = FALSE);
	
	void EnableMaximizeFloatingBars(BOOL bEnable = TRUE, BOOL bMaximizeByDblClick = FALSE);
	BOOL AreFloatingBarsCanBeMaximized() const
	{
		return m_bMaximizeFloatingBars;
	}
	BOOL MaximizeFloatingBarsByDblClick() const
	{
		return m_bMaximizeFloatingBarsByDblClick;
	}

	BOOL AddDockBar (const BCGP_DOCKBAR_INFO& info, CBCGPDockBar** ppDockBar = NULL); 
	BOOL InsertDockBar (const BCGP_DOCKBAR_INFO& info, 
						 DWORD dwAlignToInsertAfter, 
						 CBCGPDockBar** ppDockBar = NULL);

	BOOL AddControlBar (CBCGPBaseControlBar* pWnd, BOOL bTail = TRUE, 
						BOOL bAutoHide = FALSE, BOOL bInsertForOuterEdge = FALSE); 
	BOOL InsertControlBar (CBCGPBaseControlBar* pControlBar, 
						   CBCGPBaseControlBar* pTarget, BOOL bAfter = TRUE);

	void CalcExpectedDockedRect (CWnd* pWnd, CPoint ptMouse, CRect& rectResult, 
									BOOL& bDrawTab, CBCGPDockingControlBar** ppTargetBar);

	
	void RemoveControlBarFromDockManager (CBCGPBaseControlBar* pWnd, BOOL bDestroy, BOOL bAdjustLayout, 
											BOOL bAutoHide = FALSE, CBCGPBaseControlBar* pBarReplacement = NULL);

	virtual CBCGPBaseControlBar* ControlBarFromPoint (CPoint point, int nSensitivity, 
													 bool bExactBar = false, 
													 CRuntimeClass* pRTCBarType = NULL,
													 BOOL bCheckVisibility = FALSE, 
													 const CBCGPBaseControlBar* pBarToIgnore = NULL) const;
	virtual CBCGPBaseControlBar* ControlBarFromPoint (CPoint point, int nSensitivity, 
													 DWORD& dwAlignment, 
													 CRuntimeClass* pRTCBarType = NULL, 
													 const CBCGPBaseControlBar* pBarToIgnore = NULL) const;
	virtual CBCGPMiniFrameWnd* FrameFromPoint (CPoint pt, CBCGPMiniFrameWnd* pFrameToExclude, 
												BOOL bFloatMultiOnly) const;

	virtual BCGP_CS_STATUS DetermineControlBarAndStatus (CPoint pt, int nSencitivity, 
														 DWORD dwEnabledAlignment,
														 CBCGPBaseControlBar** ppTargetBar, 
														 const CBCGPBaseControlBar* pBarToIgnore,
														 const CBCGPBaseControlBar* pBarToDock);

	virtual BOOL AddMiniFrame (CBCGPMiniFrameWnd* pWnd);
	virtual BOOL RemoveMiniFrame (CBCGPMiniFrameWnd* pWnd);

	virtual CBCGPBaseControlBar* FindBarByID (UINT uBarID, BOOL bSearchMiniFrames = FALSE);
	virtual CBCGPDockBar* FindDockBar (DWORD dwAlignment, BOOL bOuter);	

	virtual CBCGPDockBar* FindDockBarByControlBar (CBCGPControlBar* pTargetBar);

	virtual void FixupVirtualRects ();
	virtual void RecalcLayout (BOOL bNotify = TRUE);
	virtual void AdjustDockingLayout (HDWP hdwp = NULL);
	virtual BOOL OnMoveMiniFrame (CWnd* pFrame);

	virtual void AdjustBarFrames ();

	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL, UINT uiID = (UINT) -1);
	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, UINT uiID = (UINT) -1);
	virtual void SetDockState ();

	virtual BOOL ShowControlBars (BOOL bShow);
	virtual void OnActivateFrame (BOOL bActivate);

	void ShowFloatingBars(BOOL bShow);

	CRect GetClientAreaBounds () const {return m_rectClientAreaBounds;}
	CRect GetOuterEdgeBounds () const {return m_rectOuterEdgeBounds;}
	CFrameWnd* GetDockSite () const {return m_pParentWnd;}

	CBCGPAutoHideToolBar*  AutoHideBar (CBCGPDockingControlBar* pBar, 
										CBCGPAutoHideToolBar* pCurrAutoHideToolBar = NULL);
	void AlignAutoHideBar (CBCGPSlider* pDefaultSlider, BOOL bIsVisible = TRUE);
	void BringBarsToTop (DWORD dwAlignment = 0, BOOL bExcludeDockedBars = TRUE);
	void SetAutohideZOrder (CBCGPDockingControlBar* pAHDockingBar);
	void HideAutoHideBars (CBCGPDockingControlBar* pBarToExclude = NULL, BOOL bImmediately = FALSE);

	virtual BOOL AdjustRectToClientArea (CRect& rectResult, DWORD dwAlignment);

	void Serialize (CArchive& ar);

	static UINT g_nTimeOutBeforeToolBarDock;
	static UINT g_nTimeOutBeforeDockingBarDock;

	void SetPrintPreviewMode (BOOL bPreview, CPrintPreviewState* pState);
	BOOL IsPrintPreviewValid () const 
	{
		return m_bIsPrintPreviewMode;
	}

	BOOL IsOLEContainerMode () const 
	{
		if (m_pParentWnd == NULL || m_pParentWnd->GetSafeHwnd () == NULL)
		{
			return FALSE;
		}
		return m_pParentWnd->m_pNotifyHook != NULL;
	}

	BOOL SendMessageToMiniFrames(UINT uMessage, WPARAM wParam = 0, LPARAM lParam = 0);
	BOOL SendMessageToControlBars(UINT uMessage, WPARAM wParam = 0, LPARAM lParam = 0);

	BOOL ReplaceControlBar (CBCGPDockingControlBar* pOriginalBar, 
							CBCGPDockingControlBar* pNewBar);
	void ReleaseEmptyContainers ();

	void LockUpdate (BOOL bLock);

	void DisableRestoreDockState (BOOL bDisable = TRUE)
	{
		m_bDisableRestoreDockState = bDisable;
	}

	BOOL IsInAdjustLayout () const
	{
		return m_bAdjustingBarLayout;
	}

	void GetControlBarList (CObList& lstBars, BOOL bIncludeAutohide = FALSE, 
							CRuntimeClass* pRTCFilter = NULL, BOOL bIncludeTabs = FALSE);

    CBCGPSmartDockingManager* GetSDManagerPermanent () const
    {
        return m_pSDManager;
    }

    CBCGPSmartDockingManager* GetSDManager ()  // consider returning NULL
    {
		if (m_bSDParamsModified)
		{
			// Smart docking manager should be re-created
			if (m_pSDManager != NULL)
			{
				delete m_pSDManager;
				m_pSDManager = NULL;
			}

			m_bSDParamsModified = FALSE;
		}

        // m_pParentWnd must already exist
        if (m_pSDManager == NULL)
        {
            m_pSDManager = new CBCGPSmartDockingManager;
            m_pSDManager->Create (m_pParentWnd);
        }

        return m_pSDManager;
    }

    void StartSDocking (CWnd* pDockingWnd)
    {
        CBCGPSmartDockingManager* pSDManager = GetSDManager ();
        if (pSDManager != NULL)
        {
            pSDManager->SetOuterRect (m_rectOuterEdgeBounds);
            pSDManager->Start (pDockingWnd);
        }
    }

    void StopSDocking ()
    {
        if (m_pSDManager != NULL)
        {
            m_pSDManager->Stop ();
        }
    }

    void GetClientAreaBounds (CRect & rcClient)
    {
        rcClient = m_rectClientAreaBounds;
    }

	void ResortMiniFramesForZOrder ();

	const CObList& GetMiniFrames () const
	{
		return m_lstMiniFrames;
	}

	void RedrawAllMiniFrames();

	void OnControlBarContextMenu (CPoint point);
	BOOL ProcessControlBarContextMenuCommand (UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	void OnClosePopupMenu ();

	void AddHiddenMDITabbedBar (CBCGPDockingControlBar* pBar);
	void RemoveHiddenMDITabbedBar (CBCGPDockingControlBar* pBar);
	
public:
	BOOL					m_bEnableAdjustLayout;
	BOOL					m_bLockUpdate;
	BOOL					m_bSizeFrame;

	CRect					m_rectInPlace;

	static BCGP_DOCK_TYPE GetDockMode ()
	{
		return m_dockModeGlobal;
	}

	static void SetDockMode (BCGP_DOCK_TYPE dockMode, BCGP_SMARTDOCK_THEME theme = BCGP_SDT_DEFAULT);

	static void SetSmartDockingParams (CBCGPSmartDockingParams& params);
	static CBCGPSmartDockingParams& GetSmartDockingParams ()
	{
		return m_SDParams;
	}

	static BCGP_SMARTDOCK_THEME	GetSmartDockingTheme ()
	{
		return m_SDTheme;
	}

	static void EnableDockBarMenu (BOOL bEnable = TRUE)
	{
		m_bDockBarMenu = bEnable;
	}

	static BOOL IsDockBarMenu ()
	{
		return m_bDockBarMenu;
	}

	void EnableControlBarContextMenu (
		BOOL bEnable, UINT uiCustomizeCmd, const CString& strCustomizeText,
		BOOL bToolbarsOnly = FALSE, BOOL bIncludeRebarPanes = FALSE);

	void BuildControlBarsMenu (CMenu& menu, BOOL bToolbarsOnly);

	static UINT				m_ahSlideModeGlobal;
	static int				m_nDockSencitivity;
	static BOOL				m_bRestoringDockState; 
	static BOOL				m_bSavingState; 

	static BOOL				m_bHideDockingBarsInContainerMode;

	static BOOL				m_bDisableRecalcLayout;

	static BOOL				m_bFullScreenMode;
	static BOOL				m_bIgnoreEnabledAlignment;
	static CRuntimeClass*	m_pAutoHideToolbarRTC;

protected:
	static BCGP_DOCK_TYPE	m_dockModeGlobal;

	CObList		m_lstControlBars;
	CObList		m_lstAutoHideBars;

	CObList		m_lstLoadedBars; // list of control bars restored from the archive
	CObList		m_lstNonFloatingBars; // list of non-floating control bars restored from the archive
	CObList		m_lstLoadedMiniFrames;

	// list of ahoto hide bar ids loaded from the archive
	CList<BCGP_AUTOHIDEBAR_SAVE_INFO, BCGP_AUTOHIDEBAR_SAVE_INFO&> 
													m_lstLoadedAutoHideBarIDs;

	CObList		m_lstMiniFrames; // list miniframes; these miniframes contain control bars
								 // that belong to this dock manager (and parent frame as well)

	CFrameWnd*	m_pParentWnd;
	CRect		m_rectDockBarBounds;
	CRect		m_rectClientAreaBounds; // it contains bounds of the client area and 
										// calculated as m_rectDockBarBounds minus
										// rectangles of all dock bars
	CRect		m_rectOuterEdgeBounds;  // it contains rectangle of "outer edges" and
										// calculated as m_rectDockBarBounds minus 
										// rectangles of first nonresizable dock bars

	DWORD		m_dwEnabledDockBars;
	DWORD		m_dwEnabledSlideBars;
	BOOL		m_bActivateAutoHideBarOnMouseClick;

	BOOL		m_bMaximizeFloatingBars;
	BOOL		m_bMaximizeFloatingBarsByDblClick;

	CBCGPBaseControlBar*	m_pLastTargetBar;
	CBCGPMultiMiniFrameWnd* m_pLastMultiMiniFrame;
	clock_t					m_clkLastTime;
	BCGP_CS_STATUS			m_statusLast;

	// Print preview memebers:
	BOOL		m_bIsPrintPreviewMode;
	CObList		m_lstBarsHiddenInPreview;

	CBCGPMiniFrameWnd* m_pActiveSlidingWnd;

	BOOL		m_bAdjustingBarLayout;
	BOOL		m_bRecalcLayout;

	BOOL		m_bDisableSetDockState; // should be disabled if load from registry failed
	BOOL		m_bDisableRestoreDockState; // can be set by user to disable load dock state
	// Bars hidden on deactivation
	CList<HWND, HWND&>				m_lstBarsHiddenOnDeactivate;

	// maintains a list of control bars hidden during in-place activation
	CList<HWND, HWND&>				m_lstBarsHiddenForOLE;

	// maintains a list of control bars hidden during in-place activation
	CList<HWND, HWND&>				m_lstHiddenMDITabbedBars;

	// Hiden floating bars
	CList<HWND, HWND&>				m_lstHiddenFloatingBars;

    CBCGPSmartDockingManager*		m_pSDManager;
	static CBCGPSmartDockingParams	m_SDParams;
	static BOOL						m_bSDParamsModified;
	static BCGP_SMARTDOCK_THEME		m_SDTheme;

	static BOOL						m_bDockBarMenu;

	// Map of control bars that appear in context menu:
	CMap<UINT,UINT,CBCGPControlBar*,CBCGPControlBar*>	m_mapControlBarsInMenu;
	BOOL							m_bControlBarsMenuIsShown;

	// Controlbar context menu attributes:
	BOOL	m_bControlBarsContextMenu;
	BOOL	m_bControlBarsContextMenuToolbarsOnly;
	BOOL	m_bControlBarContextMenuRebarPanes;
	UINT	m_uiCustomizeCmd;
	CString	m_strCustomizeText;

	BOOL							m_bHiddenForOLE;

protected:
	void AlignByRect (const CRect& rectToAlignBy, CRect& rectResult, 
						DWORD dwAlignment, BOOL bHorizontal, BOOL bResizable);
	void AlignContainer (CRect& rectCurrBounds, POSITION& posNextBar, DWORD dwAlignment, 
						 BOOL bHorizontal);
	void CalcBarOffset (CBCGPAutoHideDockBar* pBar);

	void HideForPrintPreview (const CObList& lstBars);
};

#endif // !defined(AFX_BCGPDOCKMANAGER_H__FEF7A0D3_B9D4_4FF4_911F_E75B71B20FDF__INCLUDED_)
