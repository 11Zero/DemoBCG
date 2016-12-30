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

#if !defined(AFX_BCGANIMCTRL_H__6891655B_66D7_11D3_A740_0090274409AC__INCLUDED_)
#define AFX_BCGANIMCTRL_H__6891655B_66D7_11D3_A740_0090274409AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPAnimCtrl.h : header file
//
#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_ANIM_CTRL

/////////////////////////////////////////////////////////////////////////////
// CBCGPAnimCtrl window

class BCGCBPRODLLEXPORT CBCGPAnimCtrl : public CStatic
{
	DECLARE_DYNAMIC(CBCGPAnimCtrl)

// Construction
public:
	CBCGPAnimCtrl();

// Attributes
public:
	BOOL IsRunning () const
	{
		return m_bIsRunning;
	}

protected:
	CSize		m_sizeFrame;
	COLORREF	m_clrBack;
	CImageList	m_imagesAnim;
	CImageList*	m_pImagesAnim;
	int			m_iCurrFrame;
	int			m_iFrameCount;
	BOOL		m_bIsRunning;
	UINT		m_uiFrameRate;

// Operations
public:
	BOOL SetBitmap (UINT uiBmpId, int nFrameWidth, 
		COLORREF clrTransparent = RGB (255, 0, 255), // Use (COLORREF) -1 for opaque
		BOOL bSizeToContent = TRUE);
	BOOL SetBitmap (CImageList* pImagesAnim, BOOL bSizeToContent = TRUE);

	BOOL Play (UINT uiFrameRate = 0);
	BOOL Stop ();

	void SetFrameRate (UINT uiFrameRate);

	UINT GetFrameRate () const
	{
		return m_uiFrameRate;
	}

	void SetBackColor (COLORREF clrBack)	// (-1) - Transparent
	{
		m_clrBack = clrBack;
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPAnimCtrl)
	protected:
	//}}AFX_VIRTUAL

	virtual void OnDraw(CDC* pDC);

// Implementation
public:
	virtual ~CBCGPAnimCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPAnimCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
	//}}AFX_MSG
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
};

#endif // BCGP_EXCLUDE_ANIM_CTRL

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGANIMCTRL_H__6891655B_66D7_11D3_A740_0090274409AC__INCLUDED_)
