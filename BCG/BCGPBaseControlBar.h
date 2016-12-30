#if !defined(AFX_BCGPBASECONTROLBAR_H__F0BA3C67_2E4B_43EA_8B4E_AE7DF09B5580__INCLUDED_)
#define AFX_BCGPBASECONTROLBAR_H__F0BA3C67_2E4B_43EA_8B4E_AE7DF09B5580__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// BCGPBaseControlBar.h : header file
//

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#ifndef __AFXMT_H__
	#include <afxmt.h>
#endif

#include "BCGCBPro.h"
#include "BCGPWnd.h"
#include "BCGGlobals.h"

#pragma warning( disable : 4100 34 )

class CBCGPSlider;
class CBCGPMiniFrameWnd;
class CBCGPDockBar;
class CBCGPDockBarRow;
class CBCGPTabbedControlBar;
class CBCGPBaseTabWnd;
class CBCGPBaseTabbedBar;

// DM - dock method
typedef enum BCGP_DOCK_METHOD
{
	BCGP_DM_UNKNOWN,
	BCGP_DM_MOUSE,
	BCGP_DM_DBL_CLICK,
	BCGP_DM_SHOW,
	BCGP_DM_RECT,
	BCGP_DM_STANDARD
};

// DT - dock type
typedef enum BCGP_DOCK_TYPES
{
	BCGP_DT_DOCK_FIRST,
	BCGP_DT_DOCK_LAST,
	BCGP_DT_DOCK_BEFORE,
	BCGP_DT_DOCK_AFTER,
	BCGP_DT_DOCK_BY_INDEX
};

static const DWORD CBRS_BCGP_FLOAT			= 0x1;
static const DWORD CBRS_BCGP_AUTOHIDE		= 0x2;
static const DWORD CBRS_BCGP_RESIZE			= 0x4;
static const DWORD CBRS_BCGP_CLOSE			= 0x8;
static const DWORD CBRS_BCGP_AUTO_ROLLUP	= 0x10;

static const DWORD CBRS_BCGP_REGULAR_TABS	= 0x20;
static const DWORD CBRS_BCGP_OUTLOOK_TABS	= 0x40;

/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseControlBar window

class BCGCBPRODLLEXPORT CBCGPBaseControlBar : public CBCGPWnd
{
	friend class CBCGPDockManager;
	friend class CBCGPMDIChildWnd;
	friend class CBCGPTabWnd;
	friend class CBCGPReBar;
	friend class CBCGPRebarState;

DECLARE_DYNAMIC(CBCGPBaseControlBar)

// Construction
public:
	CBCGPBaseControlBar();

// Attributes
public:
	virtual BOOL IsHorizontal () const;
	virtual BOOL IsDocked () const;
	virtual BOOL IsTabbed () const;
	virtual BOOL IsFloating () const {return !IsDocked ();}
	virtual BOOL IsMDITabbed () const;
	virtual BOOL IsResizable () const {return TRUE;}
	virtual BOOL IsVisible () const;
	virtual BOOL DoesAllowDynInsertBefore () const {return CanFloat ();}
	virtual BOOL CanAcceptBar (const CBCGPBaseControlBar* pBar) const;
	virtual BOOL CanBeDocked (CBCGPBaseControlBar* /*pDockBar*/) const {return FALSE;}
	virtual BOOL CanBeAttached () const {return FALSE;}
	virtual BOOL CanBeTabbedDocument () const {return FALSE;};
	virtual DWORD GetCurrentAlignment () const;
	virtual DWORD GetEnabledAlignment () const {return m_dwEnabledAlignment;}
	virtual CWnd* GetDockSite () const;
	virtual CBCGPDockBar* GetParentDockBar () const {return m_pParentDockBar;}
	virtual int GetCaptionHeight () const {return 0;}
	virtual BOOL IsFloatingMulti () const {return FALSE;}
	
	virtual BOOL GetRecentVisibleState () const {return m_bRecentVisibleState;}
	virtual BOOL IsRestoredFromRegistry () const {return m_bIsRestoredFromRegistry;}
	// use when loading from registry was partial (due to exception) to clean the state
	virtual void SetRestoredFromRegistry (BOOL bState) {m_bIsRestoredFromRegistry = bState;}

	virtual BCGP_DOCK_TYPE GetDockMode () const; 

	virtual HICON GetBarIcon (BOOL bBigIcon);

	BOOL IsBarVisible () const
	{
		return ((GetStyle () & WS_VISIBLE) != 0);
	}

	void SetDockMode (BCGP_DOCK_TYPE dockModeNew)
	{
		m_dockMode = dockModeNew;
	}
	void SetMDITabbed (BOOL bValue)
	{
		m_bIsMDITabbed = bValue;
	}

	virtual DWORD GetBarStyle () const {return m_dwStyle;}	
	virtual DWORD GetBCGStyle () const {return m_dwBCGStyle;}	
	virtual BOOL CanBeClosed () const { return m_dwBCGStyle & CBRS_BCGP_CLOSE; }
	virtual BOOL CanFloat () const;
	virtual BOOL CanBeResized () const { return m_dwBCGStyle & CBRS_BCGP_RESIZE; }
	virtual BOOL CanAutoHide () const {return m_dwBCGStyle & CBRS_BCGP_AUTOHIDE; }
	virtual BOOL CanFocus () const {return TRUE;}

	virtual BOOL IsAutoHideMode () const
	{
		return FALSE;
	}


	CBCGPDockBarRow* GetDockRow () {return m_pDockBarRow;}

	static BOOL m_bSetTooltipTopmost;

	static BOOL IsTooltipTopmost ()
	{
		return m_bSetTooltipTopmost && !globalData.bIsWindows9x && !globalData.bIsWindowsNT4;
	}

	static BOOL m_bMultiThreaded;

	BOOL IsRebarPane() const
	{
		return m_bIsRebarPane;
	}

// Operations
public:
	virtual void SetBarAlignment (DWORD dwAlignment) 
	{
		m_dwStyle &= ~(CBRS_ALIGN_ANY);
		m_dwStyle |= dwAlignment;
	}
	virtual void SetBarStyle (DWORD dwNewStyle) 
	{
		m_dwStyle = dwNewStyle;
	}
	virtual void SetBCGStyle (DWORD dwNewStyle)
	{
		m_dwBCGStyle = dwNewStyle;
	}
	virtual void EnableDocking (DWORD dwAlignment) 
	{
		m_dwEnabledAlignment = dwAlignment;
	}
	virtual void EnableGripper (BOOL bEnable) {}
	virtual void OnDrawCaption () {}
	virtual BOOL DockControlBar	(CBCGPBaseControlBar* pDockBar, LPCRECT lpRect, 
								 BCGP_DOCK_METHOD dockMethod) 
	{
		return FALSE;
	}
	virtual BOOL FloatControlBar	(CRect rectFloat, 
									 BCGP_DOCK_METHOD dockMethod = BCGP_DM_UNKNOWN, 
									 bool bShow = true) 
	{
		return FALSE;
	}

	virtual BOOL DockToFrameWindow  (DWORD dwAlignment, LPCRECT lpRect = NULL, 
									 DWORD dwDockFlags = BCGP_DT_DOCK_LAST, 
									 CBCGPBaseControlBar* pRelativeBar = NULL, 
									 int nRelativeIndex = -1, 
									 BOOL bOuterEdge = FALSE) 
	{
		return FALSE;
	}
	
	virtual void OnMoveSlider (CBCGPSlider* /*pSlider*/) {}
	virtual CSize StretchControlBar (int nLength, BOOL bVert) 
	{
		return CSize (0, 0);
	}

	void DockControlBarMap	 (BOOL bUseDockSite);

	virtual void AdjustLayout () {}
	virtual void RecalcLayout () {}

	virtual CBCGPMiniFrameWnd* CreateDefaultMiniframe (CRect rectInitial) {return NULL;}
	virtual void OnRemoveFromMiniFrame (CBCGPMiniFrameWnd* pMiniFrame) {}

	virtual void OnBeforeChangeParent (CWnd* pWndNewParent, BOOL bDelay = FALSE) {} 
	virtual void OnAfterChangeParent  (CWnd* pWndOldParent) {}

	virtual void ShowControlBar (BOOL bShow, BOOL bDelay, BOOL bActivate/* = TRUE*/); 

	virtual CBCGPMiniFrameWnd* GetParentMiniFrame (BOOL bNoAssert = FALSE) const;

	virtual void Serialize (CArchive& ar);
	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);

	virtual HDWP MoveWindow (CRect& rect, BOOL bRepaint = TRUE, HDWP hdwp = NULL);
	virtual HDWP SetWindowPos (const CWnd* pWndInsertAfter, int x, int y, int cx, int cy, 
								UINT nFlags, HDWP hdwp = NULL);

	virtual void CopyState (CBCGPBaseControlBar* pOrgBar);

	CBCGPBaseTabWnd* GetParentTabWnd (HWND& hWndTab) const;
	CBCGPBaseTabbedBar* GetParentTabbedBar () const;
	virtual void UnDockControlBar (BOOL bDelay = FALSE) {}

	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL /*bDisableIfNoHndler*/) {}

	const CRect& GetBarRect () const {return m_rectBar;}
	void SetBarRect (const CRect& rect) {m_rectBar = rect;}
	void FillWindowRect () {GetWindowRect (m_rectBar);}

	virtual void OnControlBarContextMenu (CWnd* pParentFrame, CPoint point);

	virtual BOOL HideInPrintPreviewMode () const	{	return TRUE;	}
	virtual BOOL HideInFullScreenMode () const	{	return TRUE;	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPBaseControlBar)
	public:
	virtual BOOL CreateEx(DWORD dwStyleEx, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwBCGStyle = 0, CCreateContext* pContext = NULL);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	
	
// Implementation
public:
	virtual ~CBCGPBaseControlBar();

	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz)
	{
		CSize size;
		size.cx = (bStretch && bHorz ? 32767 : 0);
		size.cy = (bStretch && !bHorz ? 32767 : 0);
		return size;
	}

	// Generated message map functions
protected:
	virtual void DoPaint(CDC* pDC);
	virtual void DoNcPaint(CDC* /*pDC*/) {}

	//{{AFX_MSG(CBCGPBaseControlBar)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	//}}AFX_MSG
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
	afx_msg LRESULT OnHelpHitTest(WPARAM, LPARAM lParam);
	afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnSetIcon (WPARAM,LPARAM);
	afx_msg LRESULT OnPrint(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()

	BOOL		m_bVisible; // use this status instead of IsWindowVisible
	BOOL		m_bIsRebarPane;

	DWORD		m_dwEnabledAlignment;
	DWORD		m_dwStyle;
	DWORD		m_dwBCGStyle;

	BOOL		m_bRecentVisibleState;
	BOOL		m_bIsRestoredFromRegistry;

	CBCGPDockBar*		m_pParentDockBar;
	CBCGPDockBarRow*	m_pDockBarRow;

	CWnd*				m_pDockSite;

	BCGP_DOCK_TYPE		m_dockMode;
	BOOL				m_bEnableIDChecking;

	CRect				m_rectBar;

	BOOL				m_bIsDlgControl;
	BOOL				m_bIsMDITabbed;

	HICON				m_hiconGraySmall;
	HICON				m_hiconGrayLarge;

	// Dialog bar-specific:
	LPTSTR				m_lpszBarTemplateName;
	CSize				m_sizeDialog;

	static CCriticalSection		g_cs;

public:
	////------------ for frame mapping ---------------------------
	void AddControlBar (CBCGPBaseControlBar* pBar);
	BOOL InsertControlBar	(CBCGPBaseControlBar* pControlBar, 
							 CBCGPBaseControlBar* pTarget, BOOL bAfter = TRUE);
	void RemoveControlBarFromDockManager (CBCGPBaseControlBar* pBar, BOOL bDestroy = TRUE,
							BOOL bAdjustLayout = FALSE, BOOL bAutoHide = FALSE, CBCGPBaseControlBar* pBarReplacement = NULL);
	BOOL IsPointNearDockBar (CPoint point, DWORD& dwBarAlignment, BOOL& bOuterEdge) const;
	CBCGPBaseControlBar* ControlBarFromPoint (CPoint point, int nSensitivity, 
											 bool bExactBar = false, 
											 CRuntimeClass* pRTCBarType = NULL) const;
	virtual void AdjustDockingLayout (HDWP hdwp = NULL);

	BOOL IsDialogControl () const
	{
		return m_bIsDlgControl && !m_bVisualManagerStyle;
	}

	virtual CFont* SelectDefaultFont (CDC* pDC)
	{
		if (m_bIsDlgControl)
		{
			return (CFont*) pDC->SelectStockObject (DEFAULT_GUI_FONT);
		}
		else
		{
			return pDC->SelectObject (&globalData.fontRegular);
		}
	}
};

_inline BOOL CBCGPBaseControlBar::IsHorizontal () const
{
	ASSERT_VALID (this);
	return (GetCurrentAlignment () & CBRS_ORIENT_HORZ);
}

_inline BOOL CBCGPBaseControlBar::CanAcceptBar (const CBCGPBaseControlBar* /* pBar */) const
{
	return FALSE;
}

#pragma warning( default : 4100 34 )

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPBASECONTROLBAR_H__F0BA3C67_2E4B_43EA_8B4E_AE7DF09B5580__INCLUDED_)
