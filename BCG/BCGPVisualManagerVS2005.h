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
// BCGPVisualManagerVS2005.h: interface for the CBCGPVisualManagerVS2005 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPVISUALMANAGERVS2005_H__02A9932B_48CE_4154_9D11_1A6A23DA154F__INCLUDED_)
#define AFX_BCGPVISUALMANAGERVS2005_H__02A9932B_48CE_4154_9D11_1A6A23DA154F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPVisualManager2003.h"

class BCGCBPRODLLEXPORT CBCGPVisualManagerVS2005 : public CBCGPVisualManager2003  
{
	DECLARE_DYNCREATE(CBCGPVisualManagerVS2005)

public:
	CBCGPVisualManagerVS2005();
	virtual ~CBCGPVisualManagerVS2005();

	virtual void OnUpdateSystemColors ();
	virtual COLORREF OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
		BOOL bActive, CRect rectCaption, CRect rectButtons);
	virtual void OnDrawCaptionButton (CDC* pDC, CBCGPCaptionButton* pButton, BOOL bActive, BOOL bHorz,
										BOOL bMaximized, BOOL bDisabled, int nImageID = -1);

	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd);
	virtual void OnDrawTab (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd);

	virtual BOOL HasOverlappedAutoHideButtons () const
	{
		return m_bRoundedAutohideButtons;
	}

	virtual void OnFillAutoHideButtonBackground (CDC* pDC, CRect rect, CBCGPAutoHideButton* pButton);
	virtual void OnDrawAutoHideButtonBorder (CDC* pDC, CRect rectBounds, CRect rectBorderSize, CBCGPAutoHideButton* pButton);

	virtual void GetTabFrameColors (const CBCGPBaseTabWnd* pTabWnd,
				   COLORREF& clrDark,
				   COLORREF& clrBlack,
				   COLORREF& clrHighlight,
				   COLORREF& clrFace,
				   COLORREF& clrDarkShadow,
				   COLORREF& clrLight,
				   CBrush*& pbrFace,
				   CBrush*& pbrBlack);
	virtual void OnDrawSeparator (CDC* pDC, CBCGPBaseControlBar* pBar, CRect rect, BOOL bIsHoriz);
	virtual void OnFillHighlightedArea (CDC* pDC, CRect rect, CBrush* pBrush,
		CBCGPToolbarButton* pButton);

	virtual int GetMDITabsBordersSize ()		{	return 1;	}
	virtual int GetDockingTabsBordersSize ();

	virtual int GetDockingBarCaptionExtraHeight () const
	{
		return -2;
	}

#ifndef BCGP_EXCLUDE_PROP_LIST
	virtual COLORREF GetPropListGroupColor (CBCGPPropList* pPropList);

#endif

#ifndef BCGP_EXCLUDE_TASK_PANE
	virtual void OnDrawToolBoxFrame (CDC* pDC, const CRect& rect);

#endif // BCGP_EXCLUDE_TASK_PANE

	virtual COLORREF OnFillMiniFrameCaption (CDC* pDC, CRect rectCaption, 
											CBCGPMiniFrameWnd* pFrameWnd, 
											BOOL bActive);
	static BOOL m_bRoundedAutohideButtons;

#ifndef BCGP_EXCLUDE_GRID_CTRL
	// Grid control:
	virtual BOOL OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme)
	{
		return CBCGPVisualManagerXP::OnSetGridColorTheme (pCtrl, theme);
	}
#endif

protected:
	virtual int CreateAutoHideButtonRegion (CRect rect, DWORD dwAlignment, LPPOINT& points);

	COLORREF	m_colorActiveTabBorder;
	CPen		m_penActiveTabBorder;
	CBrush		m_brMenuButtonDroppedDown;
	CBrush		m_brMenuItemCheckedHighlight;
	WinXpTheme	m_CurrAppTheme;
};

#endif // !defined(AFX_BCGPVISUALMANAGERVS2005_H__02A9932B_48CE_4154_9D11_1A6A23DA154F__INCLUDED_)
