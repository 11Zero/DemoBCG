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
// BCGPVisualManagerVS2008.cpp: implementation of the CBCGPVisualManagerVS2008 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPVISUALMANAGERVS2008_H__F2994064_A72E_4AF2_A8DD_3799EF9D4199__INCLUDED_)
#define AFX_BCGPVISUALMANAGERVS2008_H__F2994064_A72E_4AF2_A8DD_3799EF9D4199__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgpvisualmanagervs2005.h"

class BCGCBPRODLLEXPORT CBCGPVisualManagerVS2008 : public CBCGPVisualManagerVS2005
{
	DECLARE_DYNCREATE(CBCGPVisualManagerVS2008)

public:
	CBCGPVisualManagerVS2008();
	virtual ~CBCGPVisualManagerVS2008();

	virtual void OnUpdateSystemColors ();

	// Menubar support:
	virtual void OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);

	virtual void OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed);

	virtual void OnHighlightMenuItem (CDC*pDC, CBCGPToolbarMenuButton* pButton,
		CRect rect, COLORREF& clrText);

	virtual COLORREF OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
		BOOL bActive, CRect rectCaption, CRect rectButtons);

	virtual CRect GetMenuImageFrameOffset () const
	{
		return CRect (4, 2, 0, 2);
	}

	virtual void OnDrawButtonBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	// Tabs support:
	virtual void OnDrawTab (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd);

	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, 
								const CBCGPBaseTabWnd* pTabWnd);

	virtual void OnEraseTabsButton (CDC* pDC, CRect rect, CBCGPButton* pButton,
									CBCGPBaseTabWnd* pWndTab);

	virtual BOOL OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd);

	virtual int GetMDITabsBordersSize ()		{	return 4;		}
	virtual BOOL IsMDITabsTopEdge ()			{	return FALSE;	}
	virtual BOOL AlwaysHighlight3DTabs () const	{	return TRUE;	}

	virtual void GetTabFrameColors (const CBCGPBaseTabWnd* pTabWnd,
				   COLORREF& clrDark,
				   COLORREF& clrBlack,
				   COLORREF& clrHighlight,
				   COLORREF& clrFace,
				   COLORREF& clrDarkShadow,
				   COLORREF& clrLight,
				   CBrush*& pbrFace,
				   CBrush*& pbrBlack);

	virtual void OnDrawTabResizeBar (CDC* pDC, CBCGPBaseTabWnd* pWndTab, 
									BOOL bIsVert, CRect rect,
									CBrush* pbrFace, CPen* pPen);

	// Auto-hide buttons:
	virtual void OnDrawAutoHideButtonBorder (CDC* pDC, CRect rectBounds, CRect rectBorderSize, CBCGPAutoHideButton* pButton);

	virtual COLORREF OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected = FALSE);

	virtual BCGP_SMARTDOCK_THEME GetSmartDockingTheme ();

	// Grid control:
#ifndef BCGP_EXCLUDE_GRID_CTRL
	virtual BOOL OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme);
#endif

protected:
	BOOL	m_bOSColors;
};

#endif // !defined(AFX_BCGPVISUALMANAGERVS2008_H__F2994064_A72E_4AF2_A8DD_3799EF9D4199__INCLUDED_)
