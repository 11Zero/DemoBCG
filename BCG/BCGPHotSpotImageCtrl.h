#if !defined(AFX_BCGPHOTSPOTIMAGECTRL_H__5ADD1DBB_71A3_4651_87DB_543122F7D4AF__INCLUDED_)
#define AFX_BCGPHOTSPOTIMAGECTRL_H__5ADD1DBB_71A3_4651_87DB_543122F7D4AF__INCLUDED_

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
//
// BCGPHotSpotImageCtrl.h : header file
//

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"
#include "BCGPScrollBar.h"

#ifndef BCGP_EXCLUDE_HOT_SPOT_IMAGE

/////////////////////////////////////////////////////////////////////////////
// CBCGPHotSpot class

class BCGCBPRODLLEXPORT CBCGPHotSpot : public CObject
{
	friend class CBCGPHotSpotImageCtrl;

	DECLARE_SERIAL(CBCGPHotSpot)

public:
	CBCGPHotSpot(UINT m_nID = 0);
	virtual ~CBCGPHotSpot();

	void CreateRect (CRect rect);
	virtual BOOL IsPointWithin (CPoint pt) const;

	virtual void Serialize(CArchive& ar);

protected:
	CRect			m_rect;
	const UINT		m_nID;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPHotSpotImageCtrl window

class BCGCBPRODLLEXPORT CBCGPHotSpotImageCtrl : public CButton
{
// Construction
public:
	CBCGPHotSpotImageCtrl();

// Attributes
public:
	COLORREF GetBackgroundColor () const
	{
		return m_clrBack;
	}

	void SetBackgroundColor (COLORREF color);

	UINT GetHotSpot () const
	{
		return m_nSelectedNum;
	}

	const CSize	GetImageSize () const
	{
		return m_sizeImage;
	}

	void SetScrollBarsStyle (CBCGPScrollBar::BCGPSB_STYLE style)
	{
		m_ScrollBarStyle = style;

		m_wndScrollVert.SetVisualStyle (style);
		m_wndScrollHorz.SetVisualStyle (style);
	}

	CBCGPScrollBar::BCGPSB_STYLE GetScrollBarsStyle () const
	{
		return m_ScrollBarStyle;
	}

protected:
	HBITMAP			m_hbmpImage;
	HBITMAP			m_hbmpHot;
	BOOL			m_bAutoDestroyImage;
	CSize			m_sizeImage;
	COLORREF		m_clrBack;

	CList<CBCGPHotSpot*, CBCGPHotSpot*>	m_lstHotSpots;

	CBCGPHotSpot*	m_pHot;
	CBCGPHotSpot*	m_pClicked;

	UINT			m_nSelectedNum;

	CSize			m_lineDev;
	CSize			m_pageDev;

	int				m_nHighlightPercentage;
	int				m_nHighlightTolerance;

	COLORREF		m_clrTransparentCurr;

	BOOL			m_bScrollBars;

	CBCGPScrollBar::BCGPSB_STYLE m_ScrollBarStyle;

	CBCGPScrollBar	m_wndScrollVert;		// Vertical scroll bar
	CBCGPScrollBar	m_wndScrollHorz;		// Horizontal scroll bar

	CRect			m_rectView;

// Operations
public:
	BOOL SetImage (UINT uiImageResID, UINT uiHotImageResID = 0);
	BOOL SetImage (HBITMAP hbmpImage, HBITMAP hbmpImageHot = NULL, BOOL bAutoDestroy = FALSE);

	BOOL AddHotSpot (UINT nID, CRect rect);

	void SetHighlightHotArea (int nPercentage, int nTolerance = 0);
	void EnableScrollBars (BOOL bEnable = TRUE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPHotSpotImageCtrl)
	public:
	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
	protected:
	virtual void PreSubclassWindow();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	virtual CBCGPHotSpot* HitTest (CPoint pt);
	virtual void OnDrawImage (CDC* pDC, CRect rectImage);
	virtual void OnDrawHotSpot (CDC* pDC, CRect rectImage, CBCGPHotSpot* pHotSpot);
	virtual void CleanUp ();

	virtual BOOL HasHotImage () const
	{
		return m_hbmpHot != NULL;
	}

	virtual CBCGPHotSpot* FindHotSpot (UINT uiID) const;

// Implementation
public:
	virtual ~CBCGPHotSpotImageCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPHotSpotImageCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	afx_msg LRESULT OnStyleChanging (WPARAM,LPARAM);
	DECLARE_MESSAGE_MAP()

	void UpdateScrollBars ();

	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll);
	virtual void OnPrepareDC(CDC* pDC);

	CPoint GetDeviceScrollPosition() const;
	void RedrawRect (CRect rect);
};

#endif // BCGP_EXCLUDE_HOT_SPOT_IMAGE

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPHOTSPOTIMAGECTRL_H__5ADD1DBB_71A3_4651_87DB_543122F7D4AF__INCLUDED_)

