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

#if !defined(AFX_BCGPTOOLTIPCTRL_H__5A4EF6E8_E8B6_4161_B4C0_45B99DBCB725__INCLUDED_)
#define AFX_BCGPTOOLTIPCTRL_H__5A4EF6E8_E8B6_4161_B4C0_45B99DBCB725__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPToolTipCtrl.h : header file
//

#include "bcgcbpro.h"

class CBCGPToolBar;
class CBCGPToolbarButton;
class CBCGPToolBarImages;
class CBCGPRibbonButton;

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolTipParams

class BCGCBPRODLLEXPORT CBCGPToolTipParams
{
public:
	BOOL			m_bBallonTooltip;
	BOOL			m_bDrawIcon;
	BOOL			m_bDrawDescription;
	int				m_nMaxDescrWidth;
	BOOL			m_bRoundedCorners;	// Obsolete
	BOOL			m_bBoldLabel;
	COLORREF		m_clrFill;
	COLORREF		m_clrFillGradient;
	int				m_nGradientAngle;	// 0 - 360, -1 -default (vertical)
	COLORREF		m_clrText;
	COLORREF		m_clrBorder;
	BOOL			m_bDrawSeparator;
	BOOL			m_bVislManagerTheme;
	int				m_nPaddingX;
	int				m_nPaddingY;
	UINT			m_nDelayAutoPop;
	UINT			m_nDelayInitial;
	UINT			m_nDelayReshow;
	UINT			m_nDelayAutomatic;

	CBCGPToolTipParams()
	{
		m_bBallonTooltip	= FALSE;
		m_bDrawIcon			= TRUE;
		m_bDrawDescription	= TRUE;
		m_nMaxDescrWidth	= 200;
		m_bRoundedCorners	= FALSE;
		m_bBoldLabel		= TRUE;
		m_clrFill			= (COLORREF)-1;
		m_clrFillGradient	= (COLORREF)-1;
		m_nGradientAngle	= -1;
		m_clrText			= (COLORREF)-1;
		m_clrBorder			= (COLORREF)-1;
		m_bDrawSeparator	= FALSE;
		m_bVislManagerTheme	= FALSE;
		m_nPaddingX			= 0;
		m_nPaddingY			= 0;
		m_nDelayAutoPop		= 0;
		m_nDelayInitial		= 0;
		m_nDelayReshow		= 0;
		m_nDelayAutomatic	= 0;
	}

	CBCGPToolTipParams& operator= (CBCGPToolTipParams& src)
	{
		m_bBallonTooltip	= src.m_bBallonTooltip;
		m_bDrawIcon			= src.m_bDrawIcon;
		m_bDrawDescription	= src.m_bDrawDescription;
		m_nMaxDescrWidth	= src.m_nMaxDescrWidth;
		m_bRoundedCorners	= src.m_bRoundedCorners;
		m_bBoldLabel		= src.m_bBoldLabel;
		m_clrFill			= src.m_clrFill;
		m_clrFillGradient	= src.m_clrFillGradient;
		m_nGradientAngle	= src.m_nGradientAngle;
		m_clrText			= src.m_clrText;
		m_clrBorder			= src.m_clrBorder;
		m_bDrawSeparator	= src.m_bDrawSeparator;
		m_bVislManagerTheme	= src.m_bVislManagerTheme;
		m_nPaddingX			= src.m_nPaddingX;
		m_nPaddingY			= src.m_nPaddingY;
		m_nDelayAutoPop		= src.m_nDelayAutoPop;
		m_nDelayInitial		= src.m_nDelayInitial;
		m_nDelayReshow		= src.m_nDelayReshow;
		m_nDelayAutomatic	= src.m_nDelayAutomatic;

		return *this;
	}
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolTipCtrl window

class BCGCBPRODLLEXPORT CBCGPToolTipCtrl : public CToolTipCtrl
{
	DECLARE_DYNCREATE(CBCGPToolTipCtrl)

// Construction
public:
	CBCGPToolTipCtrl(CBCGPToolTipParams* pParams = NULL);

// Attributes
public:
	void SetFixedWidth (int nWidthRegular, int nWidthLargeImage)
	{
		m_nFixedWidthRegular = nWidthRegular;
		m_nFixedWidthWithImage = nWidthLargeImage;
	}

protected:
	CBCGPToolBar*		m_pToolBar;
	CBCGPToolBarImages* m_pToolBarImages;
	CBCGPToolbarButton* m_pHotButton;
	CBCGPRibbonButton*	m_pRibbonButton;
	int					m_nRibbonImageType;
	CString				m_strDescription;
	CSize				m_sizeImage;
	CSize				m_sizeCornerRadius;
	CPoint				m_ptMargin;
	CPoint				m_ptLocation;
	int					m_nFixedWidthRegular;
	int					m_nFixedWidthWithImage;
	BOOL				m_bResetDelayTime;

	CBCGPToolTipParams	m_Params;

// Operations
public:
	void SetParams (CBCGPToolTipParams* pParams);	// NULL - default params
	const CBCGPToolTipParams& GetParams () const
	{
		return m_Params;
	}

	void SetLocation (CPoint pt);
	void SetHotRibbonButton (CBCGPRibbonButton* pRibbonButton);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPToolTipCtrl)
	//}}AFX_VIRTUAL

	virtual void SetDescription (const CString strDescription);

	virtual CSize GetIconSize ();
	virtual void OnFillBackground (CDC* pDC, CRect rect, COLORREF& clrText, COLORREF& clrLine);

	virtual CString GetLabel ();

	virtual void OnDrawBorder (CDC* pDC, CRect rect, COLORREF clrLine);
	virtual BOOL OnDrawIcon (CDC* pDC, CRect rectImage);
	virtual CSize OnDrawLabel (CDC* pDC, CRect rect, BOOL bCalcOnly);
	virtual CSize OnDrawDescription (CDC* pDC, CRect rect, BOOL bCalcOnly);
	virtual void OnDrawSeparator (CDC* pDC, int x1, int x2, int y);

// Implementation
public:
	virtual ~CBCGPToolTipCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPToolTipCtrl)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	afx_msg void OnShow(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPop(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	void GetHotButton ();
	int GetFixedWidth ();

	void AdjustDelayTime();
};

#define m_strDesrciption m_strDescription

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPTOOLTIPCTRL_H__5A4EF6E8_E8B6_4161_B4C0_45B99DBCB725__INCLUDED_)
