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
//
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_BCGPPRINTPREVIEWCTRL_H__CC886E57_0839_4E3C_8599_54D6BA9D588F__INCLUDED_)
#define AFX_BCGPPRINTPREVIEWCTRL_H__CC886E57_0839_4E3C_8599_54D6BA9D588F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPPrintPreviewCtrl.h : header file
//

#include "BCGCBPro.h"
#include "BCGPWnd.h"
#include "BCGPControlRenderer.h"

class CPreviewDC;

/////////////////////////////////////////////////////////////////////////////
// CBCGPPrintPreviewCtrl window

class BCGCBPRODLLEXPORT CBCGPPrintPreviewCtrl : public CBCGPWnd
{
	friend class CBCGPRibbonBackstagePagePrint;

	DECLARE_DYNCREATE(CBCGPPrintPreviewCtrl)

// Construction
public:
	CBCGPPrintPreviewCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPPrintPreviewCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPPrintPreviewCtrl();

	virtual BOOL Create (DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	virtual BOOL CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, LPVOID lpParam = NULL);

	virtual CPrintInfo* CreatePrintInfo() const;
	virtual BOOL SetPrintView(CView* pPrintView, CPrintInfo* pPrintInfo);

	CView* GetPrintView()
	{
		return m_pPrintView;
	}
	CPrintInfo* GetPrintInfo()
	{
		return m_pPreviewInfo;
	}

	UINT GetCurrentPage() const
	{
		return m_nCurrentPage;
	}
	void SetCurrentPage (UINT nPage, BOOL bRedraw = TRUE);

	UINT GetZoomType() const
	{
		return m_nZoomType;
	}
	void SetZoomType (int nZoomType, BOOL bRedraw = TRUE);

	double GetZoom() const
	{
		return m_dZoom;
	}

	void SetNotifyPage (UINT nCommand, BOOL bPostMessage = FALSE)
	{
		m_nNotifyPage = nCommand;
		m_bNotifyPagePost = bPostMessage;
	}
	void SetNotifyZoom (UINT nCommand, BOOL bPostMessage = FALSE)
	{
		m_nNotifyZoom = nCommand;
		m_bNotifyZoomPost = bPostMessage;
	}

	UINT GetStartPage () const;
	const CSize& GetGridSize () const
	{
		return m_sizeGrid;
	}

	BOOL ChangeSettings();

	BOOL DrawFocusEnabled() const
	{
		return m_bDrawFocus;
	}
	void EnableDrawFocus(BOOL bEnable = TRUE);

protected:
	virtual BOOL PrepareSettings();
	void PrepareBackDC();
	CSize GetPreviewMargins() const;
	CSize GetThumbnailMargins() const;
	CRect GetThumbnailFrameMargins() const;
	virtual void OnPreviewClose();
	void RecalcLayout(BOOL bUpdateScrollBars = TRUE);
	void UpdateScrollBars(BOOL bRedraw = TRUE);

	UINT GetPageFromPoint (const CPoint& point) const;

	virtual void DoDraw(CDC* pDC);
	virtual void DoDrawBackground(CDC* pDC, const CRect& rect);
	virtual BOOL DoDrawPage(CDC* pPageDC, const CRect& rectPage, UINT nPage);
	virtual void DoDrawThumbnail(CDC* pDC, const CRect& rectThumbnail, CDC* pPageDC, const CRect& rectPage);

	virtual void OnZoomChanged();
	virtual void OnPageChanged();

	virtual BOOL CalculateScale();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPPrintPreviewCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnBCGSetControlBackStageMode (WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

protected:
	CView*			m_pPrintView;
	CPrintInfo*		m_pPreviewInfo;
	BOOL			m_bPreviewInfo;

	UINT			m_nCurrentPage;
	CSize			m_sizeGrid;
	CSize			m_sizeGridMax;

	LONG			m_nOffsetPage;
	CPoint			m_ptOffsetPage;

	int             m_nZoomType;
	double			m_dZoom;

	UINT			m_nNotifyZoom;
	BOOL			m_bNotifyZoomPost;
	UINT			m_nNotifyPage;
	BOOL			m_bNotifyPagePost;

	CRect			m_rectThumbnail;
	CSize			m_sizePrinter;
	CSize			m_sizePrinterPPI;
	double			m_dScalePPIX;
	double			m_dScalePPIY;

	CSize			m_margPreview;
	CSize			m_margThumbnail;
	CRect			m_margThumbnailFrame;

	CPreviewDC*		m_pPreviewDC;
	CDC				m_dcPrint;

	CDC				m_pageDC;
	CBitmap			m_pageBmp;

	CDC				m_backDC;
	CBitmap			m_backBmp;
	BOOL			m_bInvalidate;

	BOOL			m_bFastPrint;

	BOOL			m_bScrollBars[2];
	BOOL			m_bScrollPage;
	BOOL			m_bInsideUpdate;
	BOOL			m_bDrawFocus;

	BOOL			m_bBackstageMode;

	CBCGPControlRenderer m_ctrlFrame;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPRINTPREVIEWCTRL_H__CC886E57_0839_4E3C_8599_54D6BA9D588F__INCLUDED_)
