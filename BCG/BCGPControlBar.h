#if !defined(AFX_BCGPCONTROLBAR_H__7227A114_C3AC_47D9_B989_821621680BFE__INCLUDED_)
#define AFX_BCGPCONTROLBAR_H__7227A114_C3AC_47D9_B989_821621680BFE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPControlBar.h : header file
//

#include "BCGCBPro.h"
#include "BCGPBaseControlBar.h"
#include "BCGPRecentDockInfo.h"
#include "BCGPDragFrameImpl.h"

#pragma warning( disable : 4100 34 )

/////////////////////////////////////////////////////////////////////////////
// CBCGPControlBar window

class CBCGPDockBar;
class CBCGPFrameWnd;
class CBCGPDockBarRow;
class CBCGPMiniFrameWnd;
class CBCGPDockManager;

// CS - change state status
typedef enum BCGP_CS_STATUS
{
	BCGP_CS_NOTHING,
	BCGP_CS_DOCK_IMMEDIATELY,
	BCGP_CS_DELAY_DOCK,
	BCGP_CS_DELAY_DOCK_TO_TAB
};

static const DWORD dwDefaultCBStyle = CBRS_BCGP_FLOAT | CBRS_BCGP_CLOSE;
static const int dwDefaultToolbarStyle = (WS_CHILD | WS_VISIBLE | CBRS_TOP | 
										  CBRS_GRIPPER | CBRS_HIDE_INPLACE);


/////////////////////////////////////////////////////////////////////////////
// CBCGPControlBar window

class BCGCBPRODLLEXPORT CBCGPControlBar : public CBCGPBaseControlBar
{
	friend class CBCGPMiniFrameWnd;
	friend class CBCGPMultiMiniFrameWnd;

	DECLARE_DYNCREATE(CBCGPControlBar)

// Construction
protected:
	CBCGPControlBar();

// Attributes
public:

	virtual BOOL CanBeDocked (CBCGPBaseControlBar* pDockBar) const;
	virtual BOOL IsResizable () const
	{
		return FALSE;
	}

	virtual BOOL IsDragMode () const
	{
		return m_bDragMode;
	}

	virtual void GetMinSize (CSize& size) const
	{
		size = m_sizeMin;
	}

	static BOOL m_bHandleMinSize;

	void SetMinSize (const CSize& size)
	{
		m_sizeMin = size;
	}

	virtual int GetMinLength () const
	{
		return IsHorizontal () ? 10 : 10;
	}

	virtual int GetResizeStep () const 
	{
		return -1;
	}

	static BOOL m_bHideDisabledControlBarMenuItems;

	CPoint  GetHotSpot () const {return m_dragFrameImpl.m_ptHot;} 
	CPoint  GetClientHotSpot () const {return m_ptClientHotSpot;}
	void	SetClientHotSpot (const CPoint& ptNew) {m_ptClientHotSpot = ptNew;}

	virtual BOOL IsFloatingMulti () const {return FALSE;}

	virtual int GetAvailableExpandSize () const;
	virtual int GetAvailableStretchSize () const;
	
	virtual BOOL DoesAllowSiblingBars () const {return !m_bExclusiveRow;}

	bool IsLeftOf (CRect rect, bool bWindowRect = true) const;
	bool IsLastBarOnLastRow () const;

	virtual BOOL IsTabbed () const; 
	virtual void CopyState (CBCGPControlBar* pOrgBar);

	virtual BOOL AllowShowOnControlMenu () const	{	return TRUE;	}
	virtual void GetBarName (CString& strName) const;

	CBCGPDockBarRow* GetDockBarRow () const {return m_pDockBarRow;}
	void			 SetDockBarRow (CBCGPDockBarRow* pRow) {m_pDockBarRow = pRow;}

	void			 GetVirtualRect (CRect& rectVirtual) const;
	void			 SetVirtualRect (const CRect& rect, BOOL bMapToParent = TRUE); 
	void			 UpdateVirtualRect ();
	void			 UpdateVirtualRect (CPoint ptOffset);
	void			 UpdateVirtualRect (CSize sizeNew);
	
	bool			 m_bDisableMove;

	BOOL			 m_bFirstInGroup;
	BOOL			 m_bLastInGroup;
	BOOL			 m_bActiveInGroup;

	virtual void SetActiveInGroup (BOOL bActive);

	virtual void SetExclusiveRowMode (BOOL bExclusive = TRUE) {m_bExclusiveRow = bExclusive;}
	virtual BOOL GetExclusiveRowMode () const {return m_bExclusiveRow;}

// Operations
public:
	virtual void PrepareToDock		(CBCGPDockBar* pDockBar, BCGP_DOCK_METHOD dockMethod);
	virtual BOOL DockControlBar		(CBCGPBaseControlBar* pDockBar, LPCRECT lpRect, 
									 BCGP_DOCK_METHOD dockMethod);
	virtual BOOL DockByMouse		(CBCGPBaseControlBar* pDockBar);
	virtual BOOL DockToFrameWindow  (DWORD dwAlignment, LPCRECT lpRect = NULL, 
									 DWORD dwDockFlags = BCGP_DT_DOCK_LAST, 
									 CBCGPBaseControlBar* pRelativeBar = NULL, 
									 int nRelativeIndex = -1,
									 BOOL bOuterEdge = FALSE) 
	{
		return FALSE;
	}

	virtual BOOL FloatControlBar	(CRect rectFloat, BCGP_DOCK_METHOD dockMethod = BCGP_DM_UNKNOWN, 
									 bool bShow = true);
	virtual CSize CalcSize			(BOOL bVertDock) {return CSize (0, 0);}
	
	BOOL MoveByAlignment			(DWORD dwAlignment, int nOffset);
	virtual CSize MoveControlBar	(CRect rectNew, BOOL bForceMove, HDWP& hdwp);
	virtual int  StretchBar			(int nStretchSize, HDWP& hdwp);

	virtual CBCGPControlBar* DockControlBarStandard (BOOL& bWasDocked) 
	{
		bWasDocked = FALSE;
		return this;
	}
	virtual BCGP_CS_STATUS IsChangeState (int nOffset, CBCGPBaseControlBar** ppTargetBar) const;

	virtual CBCGPMiniFrameWnd* CreateDefaultMiniframe	 (CRect rectInitial);

	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);

	virtual void SetDockState (CBCGPDockManager* pDockManager);
	virtual void OnProcessDblClk ();

	virtual BOOL OnBeforeFloat (CRect& rectFloat, BCGP_DOCK_METHOD dockMethod);
	virtual void OnAfterFloat  (); 

	virtual void UnDockControlBar (BOOL bDelay = FALSE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPControlBar)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwBCGStyle = dwDefaultCBStyle, CCreateContext* pContext = NULL);
	virtual BOOL CreateEx(DWORD dwStyleEx, LPCTSTR lpszClassName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwBCGStyle = dwDefaultCBStyle, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

	virtual void OnBeforeChangeParent (CWnd* pWndNewParent, BOOL bDelay = FALSE);
	virtual void OnAfterChangeParent  (CWnd* pWndOldParent);

	virtual void RecalcLayout ();
	virtual CSize CalcAvailableSize (CRect rectRequired);

	void	SetMiniFrameRTC (CRuntimeClass* pClass)	{m_pMiniFrameRTC = pClass;}

	virtual void AdjustSizeImmediate (BOOL bRecalcLayout = TRUE);

	virtual void ToggleAutoHide ()	{}
	virtual void OnPressCloseButton ();

	virtual BOOL CanBeTabbedDocument () const;
	virtual void ConvertToTabbedDocument (BOOL bActiveTabOnly = TRUE);

public:
	virtual BOOL OnShowControlBarMenu (CPoint point);
	
protected:
	virtual BOOL OnBeforeShowControlBarMenu (CMenu& /*menu*/)	{	return TRUE;	}
	virtual BOOL OnAfterShowControlBarMenu (int nMenuResult)	{	return TRUE;	}

	virtual void OnGetControlBarMenuItemText(UINT nResID, CString& strItem);

	virtual void OnContinueMoving ();
	virtual void OnDrawDragRect (LPCRECT /*lprectNew*/, LPCRECT /*lprectOld*/) {}
	
	virtual BOOL OnBeforeDock (CBCGPBaseControlBar** ppDockBar, LPCRECT lpRect, BCGP_DOCK_METHOD dockMethod);
	virtual void OnAfterDock  (CBCGPBaseControlBar* /*pBar*/, LPCRECT /*lpRect*/, BCGP_DOCK_METHOD /*dockMethod*/) {}

	virtual void OnBeforeStretch (int& /*nStretchSize*/) {}
	virtual void OnAfterStretch (int /*nStretchSize*/) {}
	
	virtual void RemoveFromMiniframe (CWnd* pNewParent, BCGP_DOCK_METHOD dockMethod);

	virtual void SetDragMode (BOOL bOnOff);

	virtual BOOL Dock (CBCGPBaseControlBar* pDockBar, LPCRECT lpRect, 
						BCGP_DOCK_METHOD dockMethod);

	virtual void EnterDragMode (BOOL bChangeHotPoint);
	
	virtual void OnRTLChanged (BOOL bIsRTL);

	void DisableControlBarMenuItem(CMenu& menu, UINT nItem);

	// Implementation
public:
	virtual ~CBCGPControlBar();
	
	void SetBorders  (int cxLeft = 0, int cyTop = 0, int cxRight = 0, int cyBottom = 0, BOOL bIsTransparent = FALSE);
	void SetBorders  (LPCRECT lpRect, BOOL bIsTransparent = FALSE);
	CRect GetBorders () const;

	// array of elements  
	int m_nCount;
	void* m_pData;        // m_nCount elements - type depends on derived class
	// info about bar (for status bar and toolbar)
	int m_cxLeftBorder, m_cxRightBorder;
	int m_cyTopBorder, m_cyBottomBorder;
	BOOL  m_bIsTransparentBorder;

	int m_cxDefaultGap;         // default gap value
	int m_nMRUWidth;

	void CalcInsideRect(CRect& rect, BOOL bHorz) const; // adjusts borders etc
	BOOL AllocElements(int nElements, int cbElement);

//----------------------------------------------------------------------------------------------
	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPControlBar)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnNcDestroy ();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CRect				m_rectVirtual;
	CRect				m_rectDragImmediate;
	bool				m_bCaptured;
	UINT				m_nID;

	CPoint				m_ptClientHotSpot;
	bool				m_bDisableChangeHot;

	bool				m_bDblClick;
	BOOL				m_bDragMode;

	CRuntimeClass*		m_pMiniFrameRTC;

	CBCGPDragFrameImpl	m_dragFrameImpl;

	HWND				m_hwndMiniFrameToBeClosed;
	BOOL				m_bExclusiveRow;

	BOOL				m_bPinState;

	CSize				m_sizeMin;

//-----------------------------------------------
//  Recent dock/float state
//-----------------------------------------------
public:
	CBCGPRecentDockInfo		m_recentDockInfo;


	CRect					m_rectSavedDockedRect; // in client coordinates
	
	BOOL					m_bRecentFloatingState;
	BOOL					m_bWasFloatingBeforeMove;

	void SetRecentTabWindowInfo (DWORD dwFrameAlignment, 
								 const CList<UINT, UINT&>& lstSiblingBars);

	void CalcRecentDockedRect ();


protected:
	virtual void StoreRecentDockInfo ();
//-----------------------------------------------
};

_inline BOOL CBCGPControlBar::CanBeDocked (CBCGPBaseControlBar* pDockBar) const 
{
	ASSERT_VALID (this);
	
	return ((GetEnabledAlignment () & pDockBar->GetCurrentAlignment ()) != 0);
}

#pragma warning( default : 4100 34 )

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPCONTROLBAR_H__7227A114_C3AC_47D9_B989_821621680BFE__INCLUDED_)
