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
// BCGPExplorerToolBar.h : header file
//

#if !defined(AFX_BCGPEXPLORERTOOLBAR_H__EA00C1DC_E077_43FA_90C0_B93731450796__INCLUDED_)
#define AFX_BCGPEXPLORERTOOLBAR_H__EA00C1DC_E077_43FA_90C0_B93731450796__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
#include "BCGPToolBar.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPExplorerToolBar window

class BCGCBPRODLLEXPORT CBCGPExplorerToolBar : public CBCGPToolBar
{
	friend class CBCGPExplorerNavigationHistoryButton;
	friend class CBCGPExplorerNavigationButton;

	DECLARE_DYNCREATE(CBCGPExplorerToolBar)

// Construction
public:
	CBCGPExplorerToolBar();

// Attributes
public:
	void EnableNavigationButtons (UINT nBackID, UINT nForwardID, 
		UINT nHistoryID, UINT nHistoryItemID /* Should be replaced by actual list */, 
		int xRightMargin = 15);

	void EnableAddressBar(UINT nID, HWND hwndAddressBar, BOOL bStretch = TRUE, int nAddressBarMinWidth = 100);
	HWND GetAddressBar() const
	{
		return m_hWndAddressBar;
	}

	void SetStretchID (UINT nID);
	UINT GetStretchID () const
	{
		return m_nStretchID;
	}

	CSize GetNavImageSize () const
	{
		return m_bLargeIcons ? m_NavImagesLarge.GetImageSize () : m_NavImages.GetImageSize ();
	}

	CSize GetNavFrameSize () const
	{
		return m_bLargeIcons ? m_NavFramesLarge.GetImageSize () : m_NavFrames.GetImageSize ();
	}

// Operations
public:
	//-----------------
	// Toolbar context:
	//-----------------
	virtual BOOL LoadBitmap (UINT uiResID, UINT uiColdResID = 0, UINT uiDisabledResID = 0,
					UINT uiLargeColdResID = 0, UINT uiLargeDisabledResID = 0, UINT uiLargeHotResID = 0);
	virtual BOOL LoadToolBar (UINT uiResID, 
					UINT uiColdResID = 0, UINT uiDisabledResID = 0, UINT uiHotResID = 0,
					UINT uiLargeColdResID = 0, UINT uiLargeDisabledResID = 0, UINT uiLargeHotResID = 0);

// Overrides
	virtual void OnLoadNavImages ();
	virtual void OnDrawNavButton (CDC* pDC, CBCGPToolbarButton* pButton, BOOL bIsForward, CRect rect, BOOL bHighlight);
	virtual void OnDrawNavButtonsFrame (CDC* pDC);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPExplorerToolBar)
	//}}AFX_VIRTUAL

	BOOL Create(CWnd* pParentWnd,
			DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,
			UINT nID = AFX_IDW_TOOLBAR);
	BOOL CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle = 0,
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,
		UINT nID = AFX_IDW_TOOLBAR);

	virtual BOOL LoadState (LPCTSTR /*lpszProfileName*/ = NULL, int /*nIndex*/ = -1, UINT /*uiID*/ = (UINT) -1)	{	return TRUE;	}
	virtual BOOL SaveState (LPCTSTR /*lpszProfileName*/ = NULL, int /*nIndex*/ = -1, UINT /*uiID*/ = (UINT) -1)	{	return TRUE;	}

	virtual void AdjustLocations ();
	virtual void OnFillBackground (CDC* pDC);
	virtual void AdjustLayout ();
	virtual int CalcMaxButtonHeight ();
	virtual CSize CalcLayout (DWORD dwMode, int nLength = -1);
	virtual void DWMCompositionChanged ();

// Implementation
public:
	virtual ~CBCGPExplorerToolBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPExplorerToolBar)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg LRESULT OnChangeVisualManager(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

protected:
	UINT				m_nBackID;
	UINT				m_nForwardID;
	UINT				m_nHistoryID;
	UINT				m_nHistoryItemID;
	CBCGPToolBarImages	m_NavImages;
	CBCGPToolBarImages	m_NavFrames;
	CBCGPToolBarImages	m_NavImagesLarge;
	CBCGPToolBarImages	m_NavFramesLarge;
	CBCGPToolBarImages	m_NavImages16;
	CBCGPToolBarImages	m_NavFrames16;
	int					m_xRightMargin;
	CRect				m_rectNavButtons;
	CRect				m_rectHistoryButton;
	BOOL				m_bIsHistoryDroppedDown;
	BOOL				m_bIsAeroEnabled;
	UINT				m_nStretchID;
	UINT				m_nAddressBarID;
	BOOL				m_bIsHidden;
	HWND				m_hWndAddressBar;
	int					m_nAddressBarMinWidth;

#if (defined _BCGSUITE_) || (defined _BCGSUITE_INC_)
	BOOL				m_bOnGlass;
#endif
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPEXPLORERTOOLBAR_H__EA00C1DC_E077_43FA_90C0_B93731450796__INCLUDED_)
