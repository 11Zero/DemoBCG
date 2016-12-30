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

#if !defined(AFX_BCGPOLECNTRFRAMEWND_H__F7386ABF_BAB4_44F2_ABE1_D73DDE92EAED__INCLUDED_)
#define AFX_BCGPOLECNTRFRAMEWND_H__F7386ABF_BAB4_44F2_ABE1_D73DDE92EAED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPOleCntrFrameWnd.h : header file
//

#include "afxpriv.h"
#include "BCGCBPro.h"
#include "BCGPDockManager.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPOleCntrFrameWnd frame

class BCGCBPRODLLEXPORT CBCGPOleCntrFrameWnd : public COleCntrFrameWnd
{
	friend class CBCGPOleIPFrameWnd;
	friend class CBCGPOleDocIPFrameWnd;

	DECLARE_DYNAMIC(CBCGPOleCntrFrameWnd)

public:
	CBCGPOleCntrFrameWnd(COleIPFrameWnd* pInPlaceFrame);

// Attributes
protected:
	CBCGPDockManager	m_dockManager;

// Operations
public:
	virtual BOOL OnShowControlBars (BOOL bShow);

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

	CBCGPBaseControlBar* GetControlBar (UINT nID);
	void ShowControlBar (CBCGPBaseControlBar* pBar, BOOL bShow, BOOL bDelay, BOOL bActivate);

	virtual BOOL OnMoveMiniFrame	(CWnd* pFrame);
	virtual void RecalcLayout (BOOL bNotify = TRUE);

	BOOL DockControlBarLeftOf(CBCGPControlBar* pBar, CBCGPControlBar* pLeftOf);

	void CreateDockManager ()
	{
		m_dockManager.Create (this);
	}

	void AdjustClientArea();

// Overrides
	CBCGPDockManager* GetDockManager () {return &m_dockManager;}

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPOleCntrFrameWnd)
	public:
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CBCGPOleCntrFrameWnd();

	// Generated message map functions
	//{{AFX_MSG(CBCGPOleCntrFrameWnd)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg void OnIdleUpdateCmdUI();
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPOLECNTRFRAMEWND_H__F7386ABF_BAB4_44F2_ABE1_D73DDE92EAED__INCLUDED_)
