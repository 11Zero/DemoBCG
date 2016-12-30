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


#if !defined(AFX_BCGPPRINTPREVIEWVIEW_H__7AA67963_DFEF_11D3_AA95_000000000000__INCLUDED_)
#define AFX_BCGPPRINTPREVIEWVIEW_H__7AA67963_DFEF_11D3_AA95_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxpriv.h> //for CPreviewView
#include "BCGPToolBar.h"
#include "BCGCBPro.h"

class CBCGPStatusBar;
class CBCGPRibbonBar;
class CBCGPRibbonButton;

BCGCBPRODLLEXPORT void BCGPPrintPreview (CView* pView);

/////////////////////////////////////////////////////////////////////////////
// CBCGPPrintPreviewToolBar toolbar

class BCGCBPRODLLEXPORT CBCGPPrintPreviewToolBar : public CBCGPToolBar
{
	friend class CBCGPPrintPreviewView;

	DECLARE_DYNAMIC(CBCGPPrintPreviewToolBar)

protected:
	//{{AFX_MSG(CBCGPPrintPreviewToolBar)
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint pos);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	virtual BOOL AllowShowOnControlMenu () const			{	return FALSE;	}
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPPrintPreviewView window

class BCGCBPRODLLEXPORT CBCGPPrintPreviewView : public CPreviewView
{
protected:
	CBCGPPrintPreviewView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBCGPPrintPreviewView)

// Attributes
public:
	static void EnableScaleLargeImages (BOOL bScaleLargeImages = TRUE)
	{
		m_bScaleLargeImages = bScaleLargeImages;
	}

// Operations
protected:
	void SetToolbarSize ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPPrintPreviewView)
	//}}AFX_VIRTUAL
	void OnDisplayPageNumber(UINT nPage, UINT nPagesDisplayed);

// Implementation
protected:
	virtual ~CBCGPPrintPreviewView();

	CBCGPPrintPreviewToolBar	m_wndToolBar;

	int m_iOnePageImageIndex;
	int m_iTwoPageImageIndex;

	int m_iPagesBtnIndex;

	CBCGPStatusBar*	m_pWndStatusBar;
	int				m_nSimpleType;
	BOOL			m_bIsStatusBarSimple;
	CSize			m_recentToolbarSize;

	static BOOL		m_bScaleLargeImages;

	CBCGPRibbonBar*	m_pWndRibbonBar;
	CBCGPRibbonButton* m_pNumPageButton;

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPPrintPreviewView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdatePreviewNumPage(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPRINTPREVIEWVIEW_H__7AA67963_DFEF_11D3_AA95_000000000000__INCLUDED_)
