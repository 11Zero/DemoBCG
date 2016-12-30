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

#include "stdafx.h"
#include "BCGPVisualManagerVS2008.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPDrawManager.h"
#include "BCGPTabWnd.h"
#include "BCGPAutoHideButton.h"
#include "BCGPColorBar.h"
#include "BCGPCalculator.h"
#include "BCGPCalendarBar.h"
#include "BCGPCustomizeMenuButton.h"
#include "BCGPReBar.h"
#include "bcgpstyle.h"
#include "BCGPGridCtrl.h"

#define WINXPBLUE_GRADIENT_LIGHT	RGB(239, 243, 250)
#define WINXPBLUE_GRADIENT_DARK		RGB(193, 210, 238)
#define WINXPBLUE_MENUITEM_BORDER	RGB(152, 181, 226)
#define WINXPBLUE_MENU_GUTTER		RGB(241, 241, 241)
#define WINXPBLUE_MENU_GUTTER_DARK	RGB(225, 225, 225)

IMPLEMENT_DYNCREATE(CBCGPVisualManagerVS2008, CBCGPVisualManagerVS2005)

CBCGPVisualManagerVS2008::CBCGPVisualManagerVS2008()
{
	m_bConnectMenuToParent = FALSE;
	m_bShdowDroppedDownMenuButton = FALSE;
	m_bOSColors = FALSE;

	OnUpdateSystemColors ();
}

CBCGPVisualManagerVS2008::~CBCGPVisualManagerVS2008()
{
}

void CBCGPVisualManagerVS2008::OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
								CRect rectClient, CRect rectClip,
								BOOL bNCArea)
{
	ASSERT_VALID(pBar);
	ASSERT_VALID(pDC);

	if (pBar->IsOnGlass ())
	{
		pDC->FillSolidRect (rectClient, RGB (0, 0, 0));
		return;
	}

	if (DYNAMIC_DOWNCAST (CBCGPReBar, pBar) != NULL ||
		DYNAMIC_DOWNCAST (CBCGPReBar, pBar->GetParent ()))
	{
		FillRebarPane (pDC, pBar, rectClient);
		return;
	}

	if (globalData.m_nBitsPerPixel <= 8 ||
		globalData.IsHighContastMode () ||
		!pBar->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar)) ||
		pBar->IsKindOf (RUNTIME_CLASS (CBCGPColorBar)) ||
		pBar->IsKindOf (RUNTIME_CLASS (CBCGPCalculator)) ||
		pBar->IsKindOf (RUNTIME_CLASS (CBCGPCalendarBar)) ||
		GetStandardWinXPTheme () != WinXpTheme_Blue ||
		m_bOSColors)
	{
		CBCGPVisualManagerVS2005::OnFillBarBackground (pDC, pBar, rectClient, rectClip, bNCArea);
		return;
	}

	if (rectClip.IsRectEmpty ())
	{
		rectClip = rectClient;
	}

	pDC->FillRect (rectClip, &m_brMenuLight);

	CBCGPPopupMenuBar* pMenuBar = DYNAMIC_DOWNCAST (CBCGPPopupMenuBar, pBar);
	ASSERT_VALID (pMenuBar);

	if (!pMenuBar->m_bDisableSideBarInXPMode)
	{
		CRect rectImages = rectClient;

		rectImages.right = rectImages.left + pMenuBar->GetGutterWidth ();

		if (!pMenuBar->HasGutterLogo())
		{
			rectImages.DeflateRect (0, 1);
		}

		CBrush br (WINXPBLUE_MENU_GUTTER);
		pDC->FillRect (rectImages, &br);

		CPen* pOldPen = pDC->SelectObject (&m_penSeparator);
		ASSERT (pOldPen != NULL);

		pDC->MoveTo (rectImages.right, rectImages.top);
		pDC->LineTo (rectImages.right, rectImages.bottom);

		pDC->SelectObject (pOldPen);
	}
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2008::OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		GetStandardWinXPTheme () != WinXpTheme_Blue)
	{
		CBCGPVisualManagerVS2005::OnHighlightRarelyUsedMenuItems (pDC, rectRarelyUsed);
		return;
	}

	rectRarelyUsed.left --;
	rectRarelyUsed.right = rectRarelyUsed.left + CBCGPToolBar::GetMenuImageSize ().cx + 
		2 * GetMenuImageMargin () + 2;

	CBrush br (WINXPBLUE_MENU_GUTTER_DARK);
	pDC->FillRect (rectRarelyUsed, &br);
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2008::OnDrawButtonBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, BCGBUTTON_STATE state)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	CBCGPToolbarMenuButton* pMenuButton = 
		DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);

	BOOL bIsMenuBarButton = pMenuButton != NULL &&
		pMenuButton->GetParentWnd () != NULL &&
		pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar));

	if (bIsMenuBarButton)
	{
		rect.bottom -= 2;
	}

	if (!bIsMenuBarButton || !pMenuButton->IsDroppedDown ())
	{
		CBCGPVisualManagerVS2005::OnDrawButtonBorder (pDC, pButton, rect, state);
		return;
	}

	pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2008::OnFillButtonInterior (CDC* pDC,
	CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	CBCGPToolbarMenuButton* pMenuButton = 
		DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);

	BOOL bIsMenuBarButton = pMenuButton != NULL &&
		pMenuButton->GetParentWnd () != NULL &&
		pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar));

	if (bIsMenuBarButton)
	{
		rect.bottom -= 2;
	}

	if (!bIsMenuBarButton || !pMenuButton->IsDroppedDown ())
	{
		CBCGPVisualManagerVS2005::OnFillButtonInterior (pDC, pButton, rect, state);
		return;
	}

	if (!m_bOSColors)
	{
		OnFillHighlightedArea (pDC, rect, &m_brBarBkgnd, pButton);
		return;
	}

	COLORREF clr1 = CBCGPDrawManager::PixelAlpha (m_clrHighlight, 85);
	COLORREF clr2 = RGB (255, 255, 255);

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, clr1, clr2, TRUE);
}
//*************************************************************************************************
void CBCGPVisualManagerVS2008::OnHighlightMenuItem (CDC* pDC, CBCGPToolbarMenuButton* pButton,
											CRect rect, COLORREF& clrText)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2005::OnHighlightMenuItem (pDC, pButton, rect, clrText);
		return;
	}

	const int nRoundSize = 3;
	const BOOL bIsWinXPBlue = (GetStandardWinXPTheme () == WinXpTheme_Blue);

	COLORREF clr1 = bIsWinXPBlue ? WINXPBLUE_GRADIENT_DARK : m_clrHighlightGradientDark;
	COLORREF clr2 = bIsWinXPBlue ? WINXPBLUE_GRADIENT_LIGHT : m_clrHighlightGradientLight;
	COLORREF clrBorder = bIsWinXPBlue ? WINXPBLUE_MENUITEM_BORDER : m_clrHighlightGradientDark;

	if (m_bOSColors)
	{
		clr1 = m_clrHighlight;
		clr2 = RGB (255, 255, 255);
		clrBorder = m_clrHighlightDn;
	}

	if (DYNAMIC_DOWNCAST(CBCGPCustomizeMenuButton, pButton) == NULL)
	{
		rect.DeflateRect (2, 0);
	}

	CRgn rgn;
	rgn.CreateRoundRectRgn (rect.left, rect.top, rect.right, rect.bottom, nRoundSize, nRoundSize);

	pDC->SelectClipRgn (&rgn);

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, clr1, clr2, TRUE);

	pDC->SelectClipRgn (NULL);

	CPen pen (PS_SOLID, 1, clrBorder);
	CPen* pOldPen = pDC->SelectObject (&pen);
	CBrush* pOldBrush = (CBrush*) pDC->SelectStockObject (NULL_BRUSH);

	pDC->RoundRect (rect.left, rect.top, rect.right, rect.bottom, nRoundSize + 2, nRoundSize + 2);

	pDC->SelectObject (pOldPen);
	pDC->SelectObject (pOldBrush);

	clrText = GetHighlightedMenuItemTextColor (pButton);
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2008::OnEraseTabsArea (CDC* pDC, CRect rect, 
										 const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		!pTabWnd->IsVS2005Style () ||
		pTabWnd->IsDialogControl ())
	{
		CBCGPVisualManagerVS2005::OnEraseTabsArea (pDC, rect, pTabWnd);
		return;
	}

	CFrameWnd* pMainFrame = BCGCBProGetTopLevelFrame (pTabWnd);
	if (pMainFrame->GetSafeHwnd () != NULL)
	{
		CRect rectMain;
		pMainFrame->GetClientRect (rectMain);
		pMainFrame->MapWindowPoints ((CBCGPTabWnd*)pTabWnd, &rectMain);

		rect.top = rectMain.top;
		rect.left = rectMain.left;
		rect.right = rect.left + globalData.m_rectVirtual.Width () + 10;
	}

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, m_clrBarGradientDark, m_clrBarGradientLight, FALSE, 0);
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2008::OnEraseTabsButton (CDC* pDC, CRect rect,
											  CBCGPButton* pButton,
											  CBCGPBaseTabWnd* pBaseTab)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);
	ASSERT_VALID (pBaseTab);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		!pBaseTab->IsVS2005Style () ||
		pBaseTab->IsDialogControl () ||
		pButton->IsPressed () || pButton->IsHighlighted ())
	{
		CBCGPVisualManagerVS2005::OnEraseTabsButton (pDC, rect, pButton, pBaseTab);
		return;
	}

	CRgn rgn;
	rgn.CreateRectRgnIndirect (rect);

	pDC->SelectClipRgn (&rgn);

	CFrameWnd* pMainFrame = BCGCBProGetTopLevelFrame (pButton);
	if (pMainFrame->GetSafeHwnd () != NULL)
	{
		CRect rectMain;
		pMainFrame->GetClientRect (rectMain);
		pMainFrame->MapWindowPoints (pButton, &rectMain);

		rect.top = rectMain.top;
		rect.left = rectMain.left;
		rect.right = rect.left + globalData.m_rectVirtual.Width () + 10;
	}

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, m_clrBarGradientDark, m_clrBarGradientLight, FALSE, 0);

	pDC->SelectClipRgn (NULL);
}
//*********************************************************************************************************
BOOL CBCGPVisualManagerVS2008::OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (!pTabWnd->IsVS2005Style () || globalData.m_nBitsPerPixel <= 8 || 
		globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2005::OnEraseTabsFrame (pDC, rect, pTabWnd);
	}

	COLORREF clrActiveTab = pTabWnd->GetTabBkColor (pTabWnd->GetActiveTab ());
	if (clrActiveTab == (COLORREF)-1)
	{
		clrActiveTab = m_clrHighlight;
	}

	CBrush brFill (clrActiveTab);
	pDC->FillRect (rect, &brFill);

	return TRUE;
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2008::OnDrawTab (CDC* pDC, CRect rectTab,
						int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pTabWnd);
	ASSERT_VALID (pDC);

	const COLORREF clrTab = pTabWnd->GetTabBkColor (iTab);
	const BOOL bIsHighlight = (iTab == pTabWnd->GetHighlightedTab ());

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		pTabWnd->IsDialogControl () ||
		pTabWnd->IsFlatTab () || pTabWnd->IsPointerStyle() ||
		clrTab != (COLORREF)-1)
	{
		CBCGPVisualManagerVS2005::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);
		return;
	}

	if ((bIsActive || bIsHighlight || m_bOSColors) && pTabWnd->IsVS2005Style ())
	{
		((CBCGPBaseTabWnd*)pTabWnd)->SetTabBkColor (iTab, 
			bIsActive ? m_clrHighlight : 
			bIsHighlight ? m_clrHighlightDnGradientDark : GetThemeColor (m_hThemeButton, 2));

		CBCGPVisualManagerVS2005::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);

		((CBCGPBaseTabWnd*)pTabWnd)->SetTabBkColor (iTab, clrTab);
		return;
	}

	if (m_hThemeTab == NULL ||
		pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style () ||
		pTabWnd->IsLeftRightRounded ())
	{
		CRect rectClip = rectTab;
		rectClip.bottom -= 2;

		CRgn rgn;
		rgn.CreateRectRgnIndirect (&rectClip);

		pDC->SelectClipRgn (&rgn);

		CBCGPVisualManagerVS2005::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);
		
		pDC->SelectClipRgn (NULL);
		return;
	}

	int nState = TIS_NORMAL;
	if (bIsActive)
	{
		nState = TIS_SELECTED;
	}
	else if (iTab == pTabWnd->GetHighlightedTab ())
	{
		nState = TIS_HOT;
	}

	rectTab.right += 2;

	if (!bIsActive)
	{
		rectTab.bottom--;
	}
	else if (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
	{
		rectTab.top--;
		rectTab.bottom++;
	}

	if (rectTab.Width () > 25)	// DrawThemeBackground will draw < 25 width tab bad
	{
		CRect rectDraw = rectTab;

		if (bIsActive && pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
		{
			rectDraw.bottom++;
		}

		(*m_pfDrawThemeBackground) (m_hThemeTab, pDC->GetSafeHdc(), TABP_TABITEM, nState, &rectDraw, 0);

		if (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
		{
			CBCGPDrawManager dm (*pDC);
			dm.MirrorRect (rectTab, FALSE);
		}
	}

	COLORREF clrTabText = globalData.clrWindowText;

	if (pTabWnd->GetTabTextColor(iTab) != (COLORREF)-1)
	{
		clrTabText = pTabWnd->GetTabTextColor(iTab);
	}
	else
	{
		if (!bIsActive)
		{
			clrTabText = globalData.clrBtnDkShadow;
		}
		else if (m_pfGetThemeColor != NULL)
		{
			(*m_pfGetThemeColor) (m_hThemeTab, TABP_TABITEM, nState, TMT_TEXTCOLOR, &clrTabText);
		}
	}

	COLORREF cltTextOld = pDC->SetTextColor (clrTabText);

	rectTab.right -= 2;

	OnDrawTabContent (pDC, rectTab, iTab, bIsActive, pTabWnd, (COLORREF)-1);

	pDC->SetTextColor (cltTextOld);
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2008::OnDrawAutoHideButtonBorder (CDC* pDC, CRect rect, CRect rectBorderSize, CBCGPAutoHideButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		m_hThemeTab == NULL)
	{
		CBCGPVisualManagerVS2005::OnDrawAutoHideButtonBorder (pDC, rect, rectBorderSize, pButton);
		return;
	}

	const int nState = pButton->IsHighlighted () ? TIS_HOT : TIS_NORMAL;
	const DWORD dwAlign = (pButton->GetAlignment ()) & CBRS_ALIGN_ANY;

	CBCGPDrawManager dm (*pDC);

	switch (dwAlign)
	{
	case CBRS_ALIGN_LEFT:
	case CBRS_ALIGN_RIGHT:
		{
			CRect rectTab (0, 0, rect.Height (), rect.Width ());

			CDC dcMem;
			dcMem.CreateCompatibleDC (pDC);

			CBitmap bmpMem;
			bmpMem.CreateCompatibleBitmap (pDC, rectTab.Width (), rectTab.Height ());

			CBitmap* pBmpOld = (CBitmap*) dcMem.SelectObject (&bmpMem);

			(*m_pfDrawThemeBackground) (m_hThemeTab, dcMem.GetSafeHdc(), TABP_TABITEM, nState, &rectTab, 0);

			dm.DrawRotated (rect, dcMem, dwAlign == CBRS_ALIGN_LEFT);
			dcMem.SelectObject (pBmpOld);
		}
		break;

	case CBRS_ALIGN_TOP:
	case CBRS_ALIGN_BOTTOM:
		(*m_pfDrawThemeBackground) (m_hThemeTab, pDC->GetSafeHdc(), TABP_TABITEM, nState, &rect, 0);

		if (dwAlign == CBRS_ALIGN_TOP)
		{
			dm.MirrorRect (rect, FALSE);
			break;
		}
	}
}
//*********************************************************************************************************
COLORREF CBCGPVisualManagerVS2008::OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
			BOOL bActive, CRect rectCaption, CRect rectButtons)
{
	ASSERT_VALID (pDC);

	if (!m_bOSColors)
	{
		return CBCGPVisualManagerVS2005::OnDrawControlBarCaption (pDC, pBar, 
			bActive, rectCaption, rectButtons);
	}

	rectCaption.bottom++;

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rectCaption, 
		bActive ? globalData.clrActiveCaptionGradient : globalData.clrInactiveCaptionGradient, 
		bActive ? globalData.clrActiveCaption : globalData.clrInactiveCaption, 
		TRUE);

	return bActive ? globalData.clrCaptionText : globalData.clrInactiveCaptionText;
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2008::OnUpdateSystemColors ()
{
	m_bOSColors = globalData.bIsWindowsVista &&
		m_hThemeExplorerBar != NULL &&
		globalData.m_nBitsPerPixel > 8 && !globalData.IsHighContastMode ();

	CBCGPVisualManagerVS2005::OnUpdateSystemColors ();

	if (!m_bOSColors || m_pfGetThemeColor == NULL)
	{
		return;
	}

	m_clrMenuLight = ::GetSysColor(COLOR_MENU);
	m_brMenuLight.DeleteObject();
	m_brMenuLight.CreateSolidBrush (m_clrMenuLight);

	(*m_pfGetThemeColor) (m_hThemeExplorerBar, 0, 0, TMT_EDGEHIGHLIGHTCOLOR, &m_clrToolBarGradientLight);
	(*m_pfGetThemeColor) (m_hThemeExplorerBar, 0, 0, TMT_GRADIENTCOLOR2, &m_clrToolBarGradientDark);

	m_clrBarGradientDark = CBCGPDrawManager::SmartMixColors (m_clrToolBarGradientDark, m_clrToolBarGradientLight,
		1., 2, 1);

	m_clrBarGradientLight = m_clrToolBarGradientLight;

	m_clrToolBarGradientVertLight = m_clrToolBarGradientLight;

	m_clrToolBarGradientVertDark = CBCGPDrawManager::PixelAlpha (
			m_clrToolBarGradientDark, 98);

	//-------------------------------------
	// Calculate highlight gradient colors:
	//-------------------------------------
	m_clrCustomizeButtonGradientLight = m_clrToolBarGradientDark;
	m_clrCustomizeButtonGradientDark = m_clrBarGradientDark;

	m_clrToolBarBottomLine = CBCGPDrawManager::PixelAlpha (m_clrToolBarGradientDark, 85);
	m_colorToolBarCornerBottom = m_clrToolBarGradientDark;

	m_brTabBack.DeleteObject ();
	m_brTabBack.CreateSolidBrush (m_clrToolBarGradientLight);

	m_brFace.DeleteObject ();
	m_brFace.CreateSolidBrush (m_clrToolBarGradientLight);

	m_clrToolbarDisabled = CBCGPDrawManager::SmartMixColors (
		m_clrToolBarGradientDark, m_clrToolBarGradientLight, .92);

	m_penBottomLine.DeleteObject ();
	m_penBottomLine.CreatePen (PS_SOLID, 1, m_clrToolBarBottomLine);

	//--------------------------------------
	// Calculate grid/report control colors:
	//--------------------------------------
	m_penGridExpandBoxLight.DeleteObject ();
	m_penGridExpandBoxLight.CreatePen (PS_SOLID, 1, 
		CBCGPDrawManager::PixelAlpha (m_clrToolBarBottomLine, 210));

	m_penGridExpandBoxDark.DeleteObject ();
	m_penGridExpandBoxDark.CreatePen (PS_SOLID, 1, 
		CBCGPDrawManager::PixelAlpha (m_clrToolBarBottomLine, 75));
}
//*********************************************************************************************************
COLORREF CBCGPVisualManagerVS2008::OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2005::OnFillCommandsListBackground (pDC, rect, bIsSelected);
	}

	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	rect.left = 0;

	const BOOL bIsWinXPBlue = (GetStandardWinXPTheme () == WinXpTheme_Blue) || m_bOSColors;

	if (bIsSelected)
	{
		COLORREF clr1 = bIsWinXPBlue ? WINXPBLUE_GRADIENT_DARK : m_clrHighlightGradientDark;
		COLORREF clr2 = bIsWinXPBlue ? WINXPBLUE_GRADIENT_LIGHT : m_clrHighlightGradientLight;
		COLORREF clrBorder = bIsWinXPBlue ? WINXPBLUE_MENUITEM_BORDER : m_clrHighlightGradientDark;

		if (m_bOSColors)
		{
			clr1 = m_clrHighlight;
			clr2 = RGB (255, 255, 255);
			clrBorder = m_clrHighlightDn;
		}

		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rect, clr1, clr2, TRUE);

		pDC->Draw3dRect (rect, clrBorder, clrBorder);

		CBCGPToolbarMenuButton dummy;
		return GetHighlightedMenuItemTextColor (&dummy);
	}
	else
	{
		pDC->FillRect (rect, &m_brMenuLight);

		int iImageWidth = CBCGPToolBar::GetMenuImageSize ().cx + GetMenuImageMargin ();

		CRect rectImages = rect;
		rectImages.right = rectImages.left + iImageWidth + MENU_IMAGE_MARGIN;

		if (bIsWinXPBlue)
		{
			CBrush br (WINXPBLUE_MENU_GUTTER);
			pDC->FillRect (rectImages, &br);

			CPen* pOldPen = pDC->SelectObject (&m_penSeparator);
			ASSERT (pOldPen != NULL);

			pDC->MoveTo (rectImages.right, rectImages.top);
			pDC->LineTo (rectImages.right, rectImages.bottom);

			pDC->SelectObject (pOldPen);
		}
		else
		{
			CBCGPDrawManager dm (*pDC);
			dm.FillGradient (rectImages, m_clrToolBarGradientLight, m_clrToolBarGradientDark, FALSE);
		}

		return globalData.clrBarText;
	}
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2008::GetTabFrameColors (const CBCGPBaseTabWnd* pTabWnd,
				   COLORREF& clrDark,
				   COLORREF& clrBlack,
				   COLORREF& clrHighlight,
				   COLORREF& clrFace,
				   COLORREF& clrDarkShadow,
				   COLORREF& clrLight,
				   CBrush*& pbrFace,
				   CBrush*& pbrBlack)
{
	ASSERT_VALID (pTabWnd);
	
	CBCGPVisualManagerVS2005::GetTabFrameColors (pTabWnd,
			   clrDark, clrBlack,
			   clrHighlight, clrFace,
			   clrDarkShadow, clrLight,
			   pbrFace, pbrBlack);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		pTabWnd->IsFlatTab ())
	{
		return;
	}

	clrBlack = globalData.clrBarShadow;
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2008::OnDrawTabResizeBar (CDC* pDC, CBCGPBaseTabWnd* pWndTab, 
									BOOL bIsVert, CRect rect,
									CBrush* pbrFace, CPen* pPen)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pbrFace);
	ASSERT_VALID (pPen);
	ASSERT_VALID (pWndTab);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || pWndTab->IsFlatTab ())
	{
		CBCGPVisualManagerVS2005::OnDrawTabResizeBar (pDC, pWndTab, bIsVert, rect, pbrFace, pPen);
		return;
	}

	CRgn rgn;
	rgn.CreateRectRgnIndirect (rect);

	pDC->SelectClipRgn (&rgn);

	CFrameWnd* pMainFrame = BCGCBProGetTopLevelFrame (pWndTab);
	if (pMainFrame->GetSafeHwnd () != NULL)
	{
		CRect rectMain;
		pMainFrame->GetClientRect (rectMain);
		pMainFrame->MapWindowPoints (pWndTab, &rectMain);

		rect.top = rectMain.top;
		rect.left = rectMain.left;
		rect.right = rect.left + globalData.m_rectVirtual.Width () + 10;
	}

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, m_clrBarGradientDark, m_clrBarGradientLight, FALSE, 0);

	pDC->SelectClipRgn (NULL);
}
//*********************************************************************************************************
BCGP_SMARTDOCK_THEME CBCGPVisualManagerVS2008::GetSmartDockingTheme ()
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || !globalData.IsWindowsLayerSupportAvailable () ||
		!globalData.bIsWindowsVista)
	{
		return CBCGPVisualManagerVS2005::GetSmartDockingTheme ();
	}

	return BCGP_SDT_VS2008;
}
//*********************************************************************************************************
#ifndef BCGP_EXCLUDE_GRID_CTRL

BOOL CBCGPVisualManagerVS2008::OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2005::OnSetGridColorTheme (pCtrl, theme);
	}

 	theme.m_EvenColors.m_clrBackground      = globalData.clrWindow;
 	theme.m_EvenColors.m_clrText            = globalData.clrWindowText;

	theme.m_OddColors.m_clrBackground       = m_clrMenuLight;
	theme.m_OddColors.m_clrText             = globalData.clrBarText;

	theme.m_SelColors.m_clrBackground       = m_clrHighlight;
	theme.m_SelColors.m_clrBorder			= m_clrMenuItemBorder;

	if (GetRValue (m_clrHighlight) > 100 &&
		GetGValue (m_clrHighlight) > 100 &&
		GetBValue (m_clrHighlight) > 100)
	{
		theme.m_SelColors.m_clrText = RGB (0, 0, 0);
	}
	else
	{
		theme.m_SelColors.m_clrText = RGB (255, 255, 255);
	}

	theme.m_GroupColors.m_clrBackground     = GetGridLeftOffsetColor (pCtrl);
	theme.m_GroupColors.m_clrText           = globalData.clrBarText;

	theme.m_clrHorzLine = theme.m_clrVertLine = theme.m_GroupColors.m_clrBackground;

	return TRUE;
}

#endif
