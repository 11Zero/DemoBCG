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

#if !defined(AFX_BCGPOUTLOOKWND_H__FFAA1381_D08D_11D5_A305_444553540000__INCLUDED_)
#define AFX_BCGPOUTLOOKWND_H__FFAA1381_D08D_11D5_A305_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPOutlookWnd.h : header file
//

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"
#include "bcgprores.h"
#include "BCGPBaseTabWnd.h"
#include "BCGPButton.h"
#include "BCGPToolbar.h"
#include "BCGPExCheckList.h"

class CBCGPOutlookWnd;

class CBCGPOutlookSrcrollButton : public CBCGPButton
{
	virtual void OnFillBackground (CDC* pDC, const CRect& rectClient);
	virtual void OnDrawBorder (CDC* pDC, CRect& rectClient, UINT uiState);
};

class BCGCBPRODLLEXPORT CBCGPOutlookBarToolBar : public CBCGPToolBar
{
	friend class CBCGPOutlookWnd;

	DECLARE_DYNAMIC(CBCGPOutlookBarToolBar)

	CBCGPOutlookBarToolBar (CBCGPOutlookWnd* pParentBar);

	virtual BOOL LoadState (LPCTSTR /*lpszProfileName*/ = NULL, int /*nIndex*/ = -1, UINT /*uiID*/ = (UINT) -1)	{	return TRUE;	}
	virtual BOOL SaveState (LPCTSTR /*lpszProfileName*/ = NULL, int /*nIndex*/ = -1, UINT /*uiID*/ = (UINT) -1)	{	return TRUE;	}

	virtual BOOL OnSendCommand (const CBCGPToolbarButton* pButton);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

	virtual BOOL OnUserToolTip (CBCGPToolbarButton* pButton, CString& strTTText) const;

	virtual BOOL IsTabbed () const
	{
		return FALSE;
	}

	virtual BOOL AllowShowOnControlMenu () const{	return FALSE;	}
	virtual BOOL AllowShowOnList () const		{	return FALSE;	}
	virtual void AdjustLocations ();
	virtual void OnCustomizeMode (BOOL bSet);
	virtual void OnChangeVisualManager ();

	CMap<int, int, int, int>	m_TabButtons;
	CBCGPOutlookWnd*			m_pParentBar;

	//{{AFX_MSG(CBCGPOutlookBarToolBar)
	//}}AFX_MSG
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookWnd window

class BCGCBPRODLLEXPORT CBCGPOutlookWnd : public CBCGPBaseTabWnd
{
	friend class CBCGPOutlookOptionsDlg;
	friend class CBCGPOutlookBarToolBar;

	DECLARE_DYNCREATE(CBCGPOutlookWnd)

// Construction
public:
	CBCGPOutlookWnd();

// Attributes
public:
	virtual BOOL IsPtInTabArea (CPoint point) const;
	virtual void GetTabArea (CRect& rectTabAreaTop, CRect& rectTabAreaBottom) const;

	int GetBorderSize () const
	{
		return m_nBorderSize;
	}

	void SetBorderSize (int nBorderSize);

	static void EnableAnimation (BOOL bEnable = TRUE);
	static BOOL IsAnimation ()
	{
		return m_bEnableAnimation;
	}

	void SetPageButtonTextAlign (UINT uiAlign, BOOL bRedraw = TRUE);
								// TA_LEFT, TA_CENTER or TA_RIGHT

	BOOL IsMode2003 () const;

	int GetVisiblePageButtons () const
	{
		return m_nVisiblePageButtons;
	}

	void SetVisiblePageButtons (int nVisiblePageButtons)
	{
		m_nVisiblePageButtons = nVisiblePageButtons;
	}

	void EnableCaption(BOOL bEnable = TRUE)	 // Mode 2003 only!
	{
		m_bHasCaption = bEnable;
	}

	BOOL HasCaption() const
	{
		return m_bHasCaption;
	}

	void EnableFrame(BOOL bEnable = TRUE)
	{
		m_bDrawFrame = bEnable;
	}
	
	BOOL HasFrame() const
	{
		return m_bDrawFrame;
	}

	void EnableBottomLine(BOOL bEnable = TRUE)	// If splitter is not defined only!
	{
		m_bDrawBottomLine = bEnable;
	}
	
	BOOL HasBottomLine() const
	{
		return m_bDrawBottomLine;
	}

	virtual void OnShowMorePageButtons ();
	virtual void OnShowFewerPageButtons ();
	virtual BOOL CanShowMorePageButtons () const;
	virtual BOOL CanShowFewerPageButtons () const;
	virtual void OnShowOptions ();

	virtual void OnChangeTabs ();

	BOOL SetToolbarImageList (UINT uiID, int cx, COLORREF clrTransp = RGB (255, 0, 255));

	void UseAlphaBlendIcons (BOOL bAlphaBlend = TRUE, BOOL bRebuildIcons = FALSE);
// Operations
public:

	void AddControl (CWnd* pWndCtrl, LPCTSTR lpszName, int nImageID = -1,
		BOOL bDetachable = TRUE, 
		DWORD dwBCGStyle = CBRS_BCGP_FLOAT | CBRS_BCGP_CLOSE | CBRS_BCGP_RESIZE | CBRS_BCGP_AUTOHIDE);

	virtual void RecalcLayout ();
	virtual void EnableInPlaceEdit (BOOL bEnable) 
	{
		ASSERT_VALID (this);
		m_bIsInPlaceEdit = bEnable;
	}

	virtual void CalcRectEdit (CRect& rectEdit)
	{
		ASSERT_VALID(this);
		rectEdit.DeflateRect (1, 1);
	}

	void EnableScrollButtons (BOOL bEnable = TRUE,
		BOOL bIsUp = TRUE, BOOL bIsDown = TRUE);

// Overrides
	virtual BOOL IsTabDetachable (int iTab) const;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPOutlookWnd)
	public:
	virtual BOOL Create(const CRect& rect, CWnd* pParentWnd, UINT nID);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);

// Implementation
public:
	virtual ~CBCGPOutlookWnd();

	virtual BOOL SetActiveTab (int iTab);
	virtual CWnd* FindTargetWnd (const CPoint& pt);

protected:
	virtual int GetTabNumberToDetach (int nTabNum = -1) const;

	void DrawTabButton (CDC& dc, int iButtonIdx, BOOL bDrawPressedButton = TRUE);
	void RebuildToolBar ();

	void OnDraw(CDC* pDC);

	virtual void OnScrollUp () {}
	virtual void OnScrollDown () {}

	virtual BOOL IsRestoreTabAttributes() const {	return !IsMode2003();	}

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPOutlookWnd)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	//}}AFX_MSG
	afx_msg void OnToolbarCommand (UINT id);
	afx_msg void OnUpdateToolbarCommand (CCmdUI* pCmdUI);
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()

	CRect						m_rectWndArea;
	CRect						m_rectCaption;
	BOOL						m_bHasCaption;
	BOOL						m_bDrawFrame;
	BOOL						m_bDrawBottomLine;
	int							m_nBorderSize;
	CRect						m_rectSplitter;
	BOOL						m_bIsTracking;
	CBCGPOutlookBarToolBar		m_wndToolBar;
	int							m_nVisiblePageButtons;
	int							m_nMaxVisiblePageButtons;
	BOOL						m_bDontAdjustLayout;

	BOOL						m_bScrollButtons;
	CBCGPOutlookSrcrollButton	m_btnUp;
	CBCGPOutlookSrcrollButton	m_btnDown;
	UINT						m_nPageButtonTextAlign;
	CImageList					m_imagesToolbar;
	CSize						m_sizeToolbarImage;
	BOOL						m_bAlphaBlendIcons;
	BOOL						m_bIsPrintingClient;

	static BOOL	m_bEnableAnimation;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPOUTLOOKWND_H__FFAA1381_D08D_11D5_A305_444553540000__INCLUDED_)
