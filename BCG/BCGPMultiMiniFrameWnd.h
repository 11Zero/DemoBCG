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

#if !defined(AFX_BCGPMULTIMINIFRAMEWND_H__AB451BF2_C06B_4FBC_8AD2_2519274D1510__INCLUDED_)
#define AFX_BCGPMULTIMINIFRAMEWND_H__AB451BF2_C06B_4FBC_8AD2_2519274D1510__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPMultiMiniFrameWnd.h : header file
//

#include "BCGCBPro.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPBarContainerManager.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPMultiMiniFrameWnd window

class BCGCBPRODLLEXPORT CBCGPMultiMiniFrameWnd : public CBCGPMiniFrameWnd
{
	DECLARE_SERIAL(CBCGPMultiMiniFrameWnd)
// Construction
public:
	CBCGPMultiMiniFrameWnd();

// Attributes
public:
	CBCGPBarContainerManager& GetContainerManager () {return m_barContainerManager;}
	virtual BOOL CanBeDockedToBar (const CBCGPDockingControlBar* pDockingBar) const;
// Operations
public:

	virtual void AddControlBar (CBCGPBaseControlBar* pWnd);
	virtual void RemoveControlBar (CBCGPBaseControlBar* pBar, BOOL bDestroy = FALSE, BOOL bNoDelayedDestroy = TRUE);
	virtual void ReplaceControlBar (CBCGPBaseControlBar* pBarOrg, 
									CBCGPBaseControlBar* pBarReplaceWith);
	virtual BOOL DockFrame (CBCGPMiniFrameWnd* pDockedFrame, BCGP_DOCK_METHOD dockMethod);
	virtual BOOL DockBar (CBCGPDockingControlBar* pDockedBar);
	virtual int GetControlBarCount () const {return m_barContainerManager.GetBarCount ();}
	virtual int GetVisibleBarCount () const
	{
		return m_barContainerManager.GetVisibleBarCount ();
	}
	virtual CWnd* GetFirstVisibleBar () const
	{
		return m_barContainerManager.GetFirstVisibleBar ();
	}
	virtual CWnd* GetControlBar () const;

	virtual BOOL CanBeAttached () const 
	{
		return m_barContainerManager.CanBeAttached ();
	}

	virtual void CloseMiniFrame ();

	virtual BOOL SetPreDockState (BCGP_PREDOCK_STATE preDockState, 
								   CBCGPBaseControlBar* pBarToDock = NULL,
								   BCGP_DOCK_METHOD dockMethod = BCGP_DM_MOUSE);

	virtual BOOL SaveState (LPCTSTR lpszProfileName = NULL, UINT uiID = (UINT) -1);
	virtual BOOL LoadState (LPCTSTR lpszProfileName = NULL, UINT uiID = (UINT) -1);

	virtual void Serialize (CArchive& ar);
	virtual void SetDockState (CBCGPDockManager* pDockManager);
	virtual void StoreRecentDockInfo (CBCGPControlBar* pBar);
	virtual void StoreRecentTabRelatedInfo (CBCGPDockingControlBar* pDockingBar, 
											CBCGPDockingControlBar* pTabbedBar);

	virtual BOOL AddRecentControlBar (CBCGPDockingControlBar* pBar);
	virtual void DockRecentControlBarToMainFrame (CBCGPDockingControlBar* pBar);

	virtual void OnShowControlBar (CBCGPDockingControlBar* pBar, BOOL bShow);

	virtual BOOL InsertControlBar (CBCGPBaseControlBar* pControlBar, 
									CBCGPBaseControlBar* pTarget, BOOL bAfter);

	virtual void CalcExpectedDockedRect (CWnd* pWndToDock, CPoint ptMouse, 
											CRect& rectResult, BOOL& bDrawTab, 
											CBCGPDockingControlBar** ppTargetBar);
	virtual CBCGPBaseControlBar* ControlBarFromPoint (CPoint point, int nSencitivity, 
															BOOL bCheckVisibility);	

	virtual void RemoveNonValidBars ();

	virtual void OnSetRollUpTimer ();
	virtual void OnKillRollUpTimer ();

	virtual void AdjustBarFrames (); 

	virtual void OnDockToRecentPos ();
	virtual void ConvertToTabbedDocument ();

	void SetLastFocusedBar (HWND hwnd) {m_hWndLastFocused = hwnd;}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPMultiMiniFrameWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPMultiMiniFrameWnd();

	virtual void CheckGripperVisibility ();
	virtual void OnBarRecalcLayout ();
	virtual void AdjustLayout () {}
	virtual CString GetCaptionText ();
	
	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPMultiMiniFrameWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
	afx_msg LRESULT OnCheckEmptyState (WPARAM, LPARAM);
	afx_msg LRESULT OnChangeVisualManager(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

protected:
	virtual void SaveRecentFloatingState ();
	virtual CBCGPDockingControlBar* DockControlBarStandard (BOOL& bWasDocked);

protected:
	CBCGPBarContainerManager	m_barContainerManager;
	HWND						m_hWndLastFocused;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPMULTIMINIFRAMEWND_H__AB451BF2_C06B_4FBC_8AD2_2519274D1510__INCLUDED_)
