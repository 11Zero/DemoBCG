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
// BCGPReBar.h: interface for the CBCGPReBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPREBAR_H__BA7EEED4_B3E5_4319_9E83_0BC9DB01FA2D__INCLUDED_)
#define AFX_BCGPREBAR_H__BA7EEED4_B3E5_4319_9E83_0BC9DB01FA2D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPControlBar.h"
#include "BCGPControlBarImpl.h"

class CReBarCtrl;

class BCGCBPRODLLEXPORT CBCGPReBar : public CBCGPControlBar
{
	friend class CBCGPRebarState;

	DECLARE_DYNAMIC(CBCGPReBar)

// Construction
public:
	CBCGPReBar();
	BOOL Create(CWnd* pParentWnd, DWORD dwCtrlStyle = RBS_BANDBORDERS,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_TOP,
		UINT nID = AFX_IDW_REBAR);

// Attributes
public:
	// for direct access to the underlying common control
	CReBarCtrl& GetReBarCtrl() const;
	virtual BOOL CanFloat () const {return FALSE;}
// Operations
public:
	BOOL AddBar(CWnd* pBar, LPCTSTR pszText = NULL, CBitmap* pbmp = NULL,
		DWORD dwStyle = RBBS_GRIPPERALWAYS | RBBS_FIXEDBMP);
	BOOL AddBar(CWnd* pBar, COLORREF clrFore, COLORREF clrBack,
		LPCTSTR pszText = NULL, DWORD dwStyle = RBBS_GRIPPERALWAYS);

	void LockBars (BOOL bLock = TRUE);
	BOOL IsLocked () const
	{
		return m_bLocked;
	}

	virtual void SetBarAlignment (DWORD dwAlignment);

// Implementation
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
	virtual BOOL OnShowControlBarMenu (CPoint /*point*/)	{	return FALSE;	}
	virtual BOOL IsAccessibilityCompatible () { return FALSE; }

	void GetControlBarList (CObList& lst, CRuntimeClass* pRTCFilter);

#ifdef _DEBUG
	void EnableDocking(DWORD dwDockStyle);
#endif

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	BOOL _AddBar(CWnd* pBar, REBARBANDINFO* pRBBI);

	//{{AFX_MSG(CBCGPReBar)
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT);
	afx_msg void OnPaint();
	afx_msg void OnHeightChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL, NCCALCSIZE_PARAMS*);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnShowBand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRecalcParent();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBCGPControlBarImpl	m_Impl;
	BOOL				m_bLocked;
};

#endif // !defined(AFX_BCGPREBAR_H__BA7EEED4_B3E5_4319_9E83_0BC9DB01FA2D__INCLUDED_)
