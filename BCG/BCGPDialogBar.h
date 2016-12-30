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

#if !defined(AFX_BCGPDIALOGBAR_H__D80D5302_D01B_4806_B6DE_BD4505A1183D__INCLUDED_)
#define AFX_BCGPDIALOGBAR_H__D80D5302_D01B_4806_B6DE_BD4505A1183D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPDialogBar.h : header file
//

#include "BCGCBPro.h"

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
#include "BCGPDockingControlBar.h"
#endif

#include "BCGPDlgImpl.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPDialogBar window

class BCGCBPRODLLEXPORT CBCGPDialogBar : public CBCGPDockingControlBar
{
	DECLARE_SERIAL(CBCGPDialogBar)

// Construction
public:
	CBCGPDialogBar();

	BOOL Create(LPCTSTR lpszWindowName, CWnd* pParentWnd, BOOL bHasGripper,
			LPCTSTR lpszTemplateName, UINT nStyle, UINT nID,
#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
			DWORD dwTabbedStyle = CBRS_BCGP_REGULAR_TABS, 
			DWORD dwBCGStyle = dwDefaultBCGDockingBarStyle);
#else
			DWORD dwTabbedStyle = AFX_CBRS_REGULAR_TABS, 
			DWORD dwBCGStyle = AFX_DEFAULT_DOCKING_PANE_STYLE);
#endif
	BOOL Create(LPCTSTR lpszWindowName, CWnd* pParentWnd, BOOL bHasGripper,
			UINT nIDTemplate, UINT nStyle, UINT nID);

	BOOL Create(CWnd* pParentWnd, LPCTSTR lpszTemplateName,
			UINT nStyle, UINT nID)
	{
		return Create (_T(""), pParentWnd, TRUE /* bHasGripper */,
			lpszTemplateName, nStyle, nID);
	}

	BOOL Create(CWnd* pParentWnd, UINT nIDTemplate,
			UINT nStyle, UINT nID)
	{
		return Create (_T(""), pParentWnd, TRUE /* bHasGripper */,
			nIDTemplate, nStyle, nID);
	}

// Attributes
public:
	void EnableVisualManagerStyle (BOOL bEnable = TRUE, const CList<UINT,UINT>* plstNonSubclassedItems = NULL);
	BOOL IsVisualManagerStyle () const
	{
		return m_Impl.m_bVisualManagerStyle;
	}

	// Layout:
	void EnableLayout(BOOL bEnable = TRUE, CRuntimeClass* pRTC = NULL);
	BOOL IsLayoutEnabled() const
	{
		return m_Impl.IsLayoutEnabled();
	}

	CBCGPControlsLayout* GetLayout()
	{
		return m_Impl.m_pLayout;
	}

	virtual void AdjustControlsLayout()
	{
		m_Impl.AdjustControlsLayout();
	}

	void EnableScrolling(BOOL bEnable = TRUE);
	BOOL IsScrollingEnabled() const
	{
		return m_bIsScrollingEnabled;
	}

protected:
	CSize			m_sizeDefault;
	CBCGPDlgImpl	m_Impl;

	BOOL			m_bIsScrollingEnabled;
	CPoint			m_scrollPos;
	CSize			m_scrollRange;
	CSize			m_scrollSize;
	CSize			m_scrollLine;
	BOOL			m_bUpdateScroll;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPDialogBar)
	//}}AFX_VIRTUAL

	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual BOOL ClipPaint () const	{	return FALSE;	}
	virtual void OnEraseNCBackground (CDC* pDC, CRect rectBar);

	virtual void UpdateScrollBars();
	virtual BOOL ScrollHorzAvailable(BOOL bLeft);
	virtual BOOL ScrollVertAvailable(BOOL bTop);
	virtual void OnScrollClient(UINT uiScrollCode);
	virtual CPoint GetScrollPos() const {	return m_scrollPos;	}

	virtual BOOL OnGestureEventPan(const CPoint& ptFrom, const CPoint& ptTo, CSize& sizeOverPan);

// Implementation
#ifndef _AFX_NO_OCC_SUPPORT
	// data and functions necessary for OLE control containment
	_AFX_OCC_DIALOG_INFO* m_pOccDialogInfo;
	virtual BOOL SetOccDialogInfo(_AFX_OCC_DIALOG_INFO* pOccDialogInfo);
#endif

	//{{AFX_MSG(CBCGPDialogBar)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);
	afx_msg LRESULT OnChangeVisualManager (WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

protected:
	virtual void ScrollClient(const CSize& sizeScroll);

public:
	virtual ~CBCGPDialogBar();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPDIALOGBAR_H__D80D5302_D01B_4806_B6DE_BD4505A1183D__INCLUDED_)
