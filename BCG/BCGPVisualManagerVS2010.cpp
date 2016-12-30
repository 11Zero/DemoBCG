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
// BCGPVisualManagerVS2010.cpp: implementation of the CBCGPVisualManagerVS2010 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPVisualManagerVS2010.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPDrawManager.h"
#include "BCGPTabWnd.h"
#include "BCGPAutoHideButton.h"
#include "BCGPColorBar.h"
#include "BCGPCalculator.h"
#include "BCGPCalendarBar.h"
#include "bcgpstyle.h"
#include "BCGPAutoHideDockBar.h"
#include "BCGPSlider.h"
#include "BCGPStatusBar.h"
#include "BCGPToolbarEditBoxButton.h"
#include "BCGPToolBox.h"
#include "BCGPPropList.h"
#include "BCGPOutlookButton.h"
#include "CustomizeButton.h"
#include "BCGPCustomizeMenuButton.h"
#include "BCGPToolbarComboBoxButton.h"
#include "BCGPGroup.h"
#include "BCGPReBar.h"
#include "BCGPGridCtrl.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPCaptionBar.h"
#include "BCGPGanttItem.h"
#include "BCGPGanttChart.h"

#ifndef BCGP_EXCLUDE_RIBBON
#include "BCGPRibbonCategory.h"
#include "BCGPRibbonStatusBar.h"
#include "BCGPRibbonStatusBarPane.h"
#include "BCGPRibbonHyperlink.h"
#include "BCGPRibbonSlider.h"
#endif

IMPLEMENT_DYNCREATE(CBCGPVisualManagerVS2010, CBCGPVisualManagerVS2008)

CBCGPVisualManagerVS2010::CBCGPVisualManagerVS2010()
{
	m_bEmbossGripper = FALSE;
	m_bConnectMenuToParent = TRUE;
	m_bIsTransparentExpandingBox = TRUE;

	OnUpdateSystemColors ();
}

CBCGPVisualManagerVS2010::~CBCGPVisualManagerVS2010()
{
	CBCGPMenuImages::SetColor (CBCGPMenuImages::ImageWhite, (COLORREF)-1);
}

void CBCGPVisualManagerVS2010::OnFillBackground(CDC* pDC, CRect rect, CWnd* pWnd)
{
	ASSERT_VALID(pDC);

	CRect rectOriginal = rect;

	CRgn rgn;
	rgn.CreateRectRgnIndirect (rect);

	pDC->SelectClipRgn (&rgn);

	CFrameWnd* pMainFrame = pWnd == NULL ? NULL : BCGCBProGetTopLevelFrame (pWnd);
	if (pMainFrame->GetSafeHwnd () != NULL)
	{
		CRect rectMain;

		pMainFrame->GetClientRect (rectMain);
		pMainFrame->MapWindowPoints (pWnd, &rectMain);

		rect.top = rectMain.top;
		rect.bottom = rect.top + globalData.m_rectVirtual.Height () + 10;
	}

    int nShift = 6;
    int nSteps = 1 << nShift;

	COLORREF colorStart = RGB (73, 100, 143);
	COLORREF colorFinish = RGB (32, 44, 65);

    for (int i = 0; i < nSteps; i++)
    {
        CRect r2 = rect;

		r2.bottom = rect.bottom - ((i * rect.Height()) >> nShift);
        r2.top = rect.bottom - (((i + 1) * rect.Height()) >> nShift);

        if (r2.Height() <= 0 || r2.bottom < rectOriginal.top || r2.top > rectOriginal.bottom)
		{
			continue;
		}

        // do a little alpha blending
        BYTE bR = (BYTE) ((GetRValue(colorStart) * (nSteps - i) +
                   GetRValue(colorFinish) * i) >> nShift);
        BYTE bG = (BYTE) ((GetGValue(colorStart) * (nSteps - i) +
                   GetGValue(colorFinish) * i) >> nShift);
        BYTE bB = (BYTE) ((GetBValue(colorStart) * (nSteps - i) +
                   GetBValue(colorFinish) * i) >> nShift);

		pDC->SetBkColor(RGB(bR, bG, bB));
		pDC->SetTextColor (RGB (54, 73, 105));

		pDC->FillRect(r2, &m_brTabs);
    }

	pDC->SelectClipRgn (NULL);
}
//**************************************************************************************************************
void CBCGPVisualManagerVS2010::OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
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

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		pBar->IsKindOf (RUNTIME_CLASS (CBCGPColorBar)) ||
		pBar->IsKindOf (RUNTIME_CLASS (CBCGPCalculator)) ||
		pBar->IsKindOf (RUNTIME_CLASS (CBCGPCalendarBar)))
	{
		CBCGPVisualManagerVS2008::OnFillBarBackground (pDC, pBar, rectClient, rectClip, bNCArea);
		return;
	}

	if (rectClip.IsRectEmpty ())
	{
		rectClip = rectClient;
	}

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar)))
	{
		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rectClient, m_clrMenuBarGradient1, m_clrMenuBarGradient2);
		return;
	}

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPAutoHideDockBar)) || pBar->IsKindOf (RUNTIME_CLASS (CBCGPAutoHideToolBar)))
	{
		OnFillBackground(pDC, rectClient, pBar);
		return;
	}

 	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPStatusBar)))
	{
		pDC->FillRect(rectClip, &m_brStatusBar);
		return;
	}

#ifndef BCGP_EXCLUDE_RIBBON
 	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPRibbonStatusBar)))
	{
		pDC->FillRect(rectClip, &m_brStatusBar);
		return;
	}
#endif

#if !defined(BCGP_EXCLUDE_TOOLBOX) && !defined(BCGP_EXCLUDE_TASK_PANE)
 	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPToolBoxPage)) || pBar->IsKindOf (RUNTIME_CLASS (CBCGPToolBox)) || pBar->IsKindOf (RUNTIME_CLASS (CBCGPToolBoxEx)))
	{
		pDC->FillRect(rectClip, &m_brWhite);
		return;
	}
#endif

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar)))
	{
		CBCGPDrawManager dm (*pDC);
		dm.FillGradient(rectClient, m_clrMenuLight, m_clrMenuLightGradient);

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

			pDC->FillRect (rectImages, &m_brMenuGutter);
		}

		return;
	}

	CBCGPVisualManagerVS2008::OnFillBarBackground (pDC, pBar, rectClient, rectClip, bNCArea);
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2010::OnHighlightMenuItem (CDC* pDC, CBCGPToolbarMenuButton* pButton,
											CRect rect, COLORREF& clrText)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::OnHighlightMenuItem (pDC, pButton, rect, clrText);
		return;
	}

	const int nRoundSize = 3;

	if (DYNAMIC_DOWNCAST(CBCGPCustomizeMenuButton, pButton) == NULL)
	{
		rect.DeflateRect (2, 0);
	}

	CRgn rgn;
	rgn.CreateRoundRectRgn (rect.left, rect.top, rect.right, rect.bottom, nRoundSize, nRoundSize);

	pDC->SelectClipRgn (&rgn);

	CBCGPDrawManager dm (*pDC);
	dm.Fill4ColorsGradient(rect, m_clrMenuItemGradient1, m_clrMenuItemGradient2, m_clrMenuItemGradientDark, m_clrMenuItemGradientDark);

	pDC->SelectClipRgn (NULL);

	CPen pen (PS_SOLID, 1, m_clrMenuItemBorder);
	CPen* pOldPen = pDC->SelectObject (&pen);
	CBrush* pOldBrush = (CBrush*) pDC->SelectStockObject (NULL_BRUSH);

	pDC->RoundRect (rect.left, rect.top, rect.right, rect.bottom, nRoundSize + 2, nRoundSize + 2);

	pDC->SelectObject (pOldPen);
	pDC->SelectObject (pOldBrush);

	clrText = GetHighlightedMenuItemTextColor (pButton);
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2010::OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || pTabWnd->IsFlatTab ())
	{
		CBCGPVisualManagerVS2008::OnEraseTabsArea (pDC, rect, pTabWnd);
		return;
	}

	if (pTabWnd->IsDialogControl () || pTabWnd->IsPointerStyle())
	{
		if (pTabWnd->IsVisualManagerStyle ())
		{
			OnFillDialog (pDC, pTabWnd->GetParent (), rect);
		}
		else
		{
			CBCGPVisualManagerVS2008::OnEraseTabsArea (pDC, rect, pTabWnd);
		}

		return;
	}

	OnFillBackground(pDC, rect, (CWnd*)pTabWnd);
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2010::OnEraseTabsButton (CDC* pDC, CRect rect,
											  CBCGPButton* pButton,
											  CBCGPBaseTabWnd* pBaseTab)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);
	ASSERT_VALID (pBaseTab);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || 
		pBaseTab->IsFlatTab () || (pBaseTab->IsPointerStyle() && !pButton->IsPressed () && !pButton->IsHighlighted ()))
	{
		CBCGPVisualManagerVS2008::OnEraseTabsButton (pDC, rect, pButton, pBaseTab);
		return;
	}

	CBCGPDrawManager dm (*pDC);

	if (pButton->IsPressed ())
	{
		dm.FillGradient (rect, m_clrHighlightDnGradientDark, m_clrHighlightDnGradientLight);
	}
	else if (pButton->IsHighlighted ())
	{
		dm.FillGradient (rect, m_clrHighlightGradientDark, m_clrHighlightGradientLight);
	}
	else
	{
		if (!pBaseTab->IsDialogControl ())
		{
			OnFillBackground(pDC, rect, pButton);
		}
		else
		{
			OnFillDialog (pDC, pBaseTab->GetParent (), rect);
		}
	}
}
//*********************************************************************************************************
BOOL CBCGPVisualManagerVS2010::OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		(pTabWnd->IsDialogControl () && !pTabWnd->IsVisualManagerStyle ()) || !pTabWnd->IsMDITab() ||
		pTabWnd->IsFlatTab () || pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style ())
	{
		return CBCGPVisualManagerVS2008::OnEraseTabsFrame (pDC, rect, pTabWnd);
	}

	OnFillBackground(pDC, rect, (CWnd*)pTabWnd);
	return TRUE;
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawTabBorder(CDC* pDC, CBCGPTabWnd* pTabWnd, CRect rectBorder, COLORREF clrBorder,
										 CPen& penLine)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		pTabWnd->IsDialogControl () || !pTabWnd->IsMDITab() ||
		pTabWnd->IsFlatTab () || pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style ())
	{
		CBCGPVisualManagerVS2008::OnDrawTabBorder(pDC, pTabWnd, rectBorder, clrBorder, penLine);
		return;
	}

	COLORREF clrActiveTab = pTabWnd->GetTabBkColor (pTabWnd->GetActiveTab ());
	if (clrActiveTab == (COLORREF)-1)
	{
		clrActiveTab =  pTabWnd->IsMDIFocused() && pTabWnd->IsActiveInMDITabGroup() ? 
			(m_bTabFillGradient ? m_clrHighlightDn : m_clrHighlight) : m_clrMenuLight;
	}

	CRect rectTop = rectBorder;
	rectTop.DeflateRect(pTabWnd->GetTabBorderSize() + 1, 0);
	rectTop.bottom = rectTop.top + pTabWnd->GetTabBorderSize() + 1;
	rectTop.right++;

	LPPOINT pointsTop;
	int nPointsTop = CreateAutoHideButtonRegion (rectTop, CBRS_ALIGN_BOTTOM, pointsTop);

	CRgn rgnClipTop;
	rgnClipTop.CreatePolygonRgn (pointsTop, nPointsTop, WINDING);

	pDC->SelectClipRgn (&rgnClipTop);

	pDC->FillSolidRect(rectTop, clrActiveTab);

	CRect rectBottom = rectBorder;
	rectBottom.DeflateRect(pTabWnd->GetTabBorderSize() + 1, 0);
	rectBottom.top = rectBottom.bottom - pTabWnd->GetTabBorderSize() - 1;
	rectBottom.right++;

	LPPOINT pointsBottom;
	int nPointsBottom = CreateAutoHideButtonRegion (rectBottom, CBRS_ALIGN_TOP, pointsBottom);

	CRgn rgnClipBottom;
	rgnClipBottom.CreatePolygonRgn (pointsBottom, nPointsBottom, WINDING);

	pDC->SelectClipRgn (&rgnClipBottom);
	pDC->FillSolidRect(rectBottom, clrActiveTab);

	pDC->SelectClipRgn (NULL);

	delete [] pointsTop;
	delete [] pointsBottom;
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawTab (CDC* pDC, CRect rectTab,
						int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pTabWnd);
	ASSERT_VALID (pDC);

	const BOOL bIsHighlight = (iTab == pTabWnd->GetHighlightedTab ());

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		pTabWnd->IsDialogControl () ||
		pTabWnd->IsFlatTab () || pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style () || pTabWnd->IsLeftRightRounded() || pTabWnd->IsPointerStyle())
	{
		CBCGPVisualManagerVS2008::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);
		return;
	}

	COLORREF clrTab = pTabWnd->GetTabBkColor(iTab);
	BOOL bDefaultTabColor = (clrTab == (COLORREF)-1);

	if (bIsActive || bIsHighlight || !bDefaultTabColor)
	{
		CRect rectFill = rectTab;

		CPen pen (PS_SOLID, 1, globalData.clrBarShadow);
		CPen* pOldPen = pDC->SelectObject (&pen);

		LPPOINT points;
		int nPoints = CreateAutoHideButtonRegion (rectTab, 
			pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_TOP ? CBRS_ALIGN_BOTTOM : CBRS_ALIGN_TOP, 
			points);

		CRgn rgnClip;
		rgnClip.CreatePolygonRgn (points, nPoints, WINDING);

		int nIndexDC = pDC->SaveDC ();

		pDC->SelectClipRgn (&rgnClip, RGN_AND);

		CBCGPDrawManager dm (*pDC);

		if (clrTab == (COLORREF)-1)
		{
			if (pTabWnd->IsMDITab())
			{
				clrTab =  pTabWnd->IsMDIFocused() && pTabWnd->IsActiveInMDITabGroup() ? 
					(m_bTabFillGradient ? m_clrHighlightDn : m_clrHighlight) : m_clrMenuLight;
			}
			else
			{
				clrTab = RGB(255, 255, 255);
			}
		}

		if (bIsActive)
		{
			if (pTabWnd->IsMDITab())
			{
				rectFill.bottom++;
			}

			if (!m_bTabFillGradient)
			{
				dm.DrawRect(rectFill, clrTab, (COLORREF)-1);
			}
			else
			{
				if (pTabWnd->IsMDITab())
				{
					COLORREF clrTabLight = CBCGPDrawManager::PixelAlpha(clrTab, 115);
					dm.Fill4ColorsGradient(rectFill, clrTabLight, RGB(255, 255, 255), clrTab, clrTab);
				}
				else
				{
					dm.FillGradient (rectFill, RGB(255, 255, 255), clrTab);
				}
			}
		}
		else
		{
			COLORREF clr1 = m_clrHighlightedTabGradient1;
			COLORREF clr2 = pTabWnd->GetTabBkColor(iTab);

			if (clr2 == (COLORREF)-1)
			{
				clr2 = m_clrHighlightedTabGradient2;
			}
			else
			{
				clr1 = CBCGPDrawManager::PixelAlpha(clr2, 125);
			}

			if (m_bTabFillGradient)
			{
				if (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
				{
					dm.FillGradient (rectFill, clr1, clr2);
				}
				else
				{
					dm.FillGradient (rectFill, clr2, clr1);
				}
			}
			else
			{
				dm.DrawRect(rectFill, clr2, (COLORREF)-1);
			}
		}

		pDC->SelectClipRgn (NULL);

		if (bIsActive && pTabWnd->IsMDITab())
		{
			const int nRectConnectHeight = 3;

			CRect rectConnect = rectTab;
			rectConnect.right--;

			if (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
			{
				rectConnect.bottom = rectConnect.top;
				rectConnect.top -= nRectConnectHeight;

				pDC->FillSolidRect(rectConnect, RGB(255, 255, 255));
			}
			else
			{
				rectConnect.top = rectConnect.bottom;
				rectConnect.bottom += nRectConnectHeight;

				pDC->FillSolidRect(rectConnect, clrTab);
			}
		}
		else
		{
			if (m_bTabFillGradient)
			{
				pDC->Polyline (points, nPoints);
			}
		}

		delete [] points;

		pDC->SelectObject (pOldPen);
		pDC->RestoreDC (nIndexDC);
	}

	COLORREF clrTabText = pTabWnd->GetTabTextColor(iTab);
	if (clrTabText == (COLORREF)-1)
	{
		clrTabText = bIsActive ? RGB(0, 0, 0) : (bDefaultTabColor ? RGB(255, 255, 255) : bIsHighlight ? RGB(50, 50, 50) : RGB(128, 128, 128));
	}

	OnDrawTabContent(pDC, rectTab, iTab, bIsActive, pTabWnd, clrTabText);
}
//**********************************************************************************
void CBCGPVisualManagerVS2010::OnDrawTabButton (CDC* pDC, CRect rect, 
											   const CBCGPBaseTabWnd* pTabWnd,
											   int nID,
											   BOOL bIsHighlighted,
											   BOOL bIsPressed,
											   BOOL bIsDisabled)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		pTabWnd->IsDialogControl () || pTabWnd->IsPointerStyle() ||
		pTabWnd->IsFlatTab () || pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style () || pTabWnd->IsLeftRightRounded())
	{
		CBCGPVisualManagerVS2008::OnDrawTabButton (pDC, rect, pTabWnd, nID, bIsHighlighted, bIsPressed, bIsDisabled);
		return;
	}

	rect.DeflateRect(1, 0);

	if (bIsHighlighted)
	{
		pDC->FillRect(rect, bIsPressed ? &m_brHighlight : &m_brMenuItemCheckedHighlight);
	}

	BOOL bIsInactiveTab = m_nCurrDrawedTab != pTabWnd->GetActiveTab();

	CBCGPMenuImages::Draw (pDC, (CBCGPMenuImages::IMAGES_IDS)nID, rect, 
		bIsInactiveTab && !bIsHighlighted ? CBCGPMenuImages::ImageLtGray : CBCGPMenuImages::ImageBlack);

	if (bIsHighlighted)
	{
		pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawAutoHideButtonBorder (CDC* pDC, CRect rect, CRect rectBorderSize, CBCGPAutoHideButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::OnDrawAutoHideButtonBorder (pDC, rect, rectBorderSize, pButton);
		return;
	}

	DWORD dwAlign = (pButton->GetAlignment () & CBRS_ALIGN_ANY);
	BOOL bVert = FALSE;

	switch (dwAlign)
	{
	case CBRS_ALIGN_LEFT:
	case CBRS_ALIGN_RIGHT:
		rect.bottom--;
		break;

	case CBRS_ALIGN_TOP:
	case CBRS_ALIGN_BOTTOM:
		rect.right--;
		bVert = TRUE;
		break;
	}

	if (pButton->IsHighlighted ())
	{
		BOOL bInverse = dwAlign == CBRS_ALIGN_LEFT || dwAlign == CBRS_ALIGN_BOTTOM;

		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rect, 
			bInverse ? m_clrHighlightedTabGradient2 : m_clrHighlightedTabGradient1, 
			bInverse ? m_clrHighlightedTabGradient1 : m_clrHighlightedTabGradient2, 
			bVert);
	}
	else
	{
		CRect rectFill = rect;
		rectFill.DeflateRect(1, 1);
		pDC->FillRect(rectFill, &m_brAutohideButton);
	}

	CBCGPVisualManagerVS2005::OnDrawAutoHideButtonBorder (pDC, rect, rectBorderSize, pButton);
}
//*********************************************************************************************************
COLORREF CBCGPVisualManagerVS2010::GetAutoHideButtonTextColor (CBCGPAutoHideButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2008::GetAutoHideButtonTextColor (pButton);
	}

	return RGB(255, 255, 255);
}
//*********************************************************************************************************
COLORREF CBCGPVisualManagerVS2010::GetAutoHideButtonBorderColor (CBCGPAutoHideButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2008::GetAutoHideButtonBorderColor (pButton);
	}

	return pButton->IsHighlighted() ? RGB (209, 209, 209) : RGB (54, 78, 111);
}
//*********************************************************************************************************
COLORREF CBCGPVisualManagerVS2010::OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
			BOOL bActive, CRect rectCaption, CRect rectButtons)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2008::OnDrawControlBarCaption (pDC, pBar, bActive, rectCaption, rectButtons);
	}

	rectCaption.bottom++;

	CBCGPDrawManager dm (*pDC);
	if (bActive)
	{
		dm.Fill4ColorsGradient(rectCaption, m_clrMenuItemGradient1, m_clrMenuItemGradient2, m_clrMenuItemGradientDark, m_clrMenuItemGradientDark);
	}
	else
	{
		dm.FillGradient(rectCaption, m_clrInactiveCaptionGradient1, m_clrInactiveCaptionGradient2);
	}

	return bActive ? globalData.clrBarText : RGB(255, 255, 255);
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2010::OnUpdateSystemColors ()
{
	CBCGPVisualManagerVS2008::OnUpdateSystemColors ();

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return;
	}

	WORD HatchBits [8] = { 0xDD, 0xDD, 0x77, 0x77, 0xDD, 0xDD, 0x77, 0x77 };

	CBitmap bmp;
	bmp.CreateBitmap (8, 8, 1, 1, HatchBits);

	m_brTabs.DeleteObject();
	m_brTabs.CreatePatternBrush(&bmp);

	m_clrHighlight = RGB (255, 239, 187);
	m_brHighlight.DeleteObject();
	m_brHighlight.CreateSolidBrush(m_clrHighlight);

	m_clrHighlightDn = RGB (255, 232, 166);
	m_brHighlightDn.DeleteObject();
	m_brHighlightDn.CreateSolidBrush(m_clrHighlightDn);

	m_clrHighlightChecked = RGB (255, 232, 166);
	m_brHighlightChecked.DeleteObject();
	m_brHighlightChecked.CreateSolidBrush(m_clrHighlightChecked);

	m_clrHighlightMenuItem = RGB (255, 232, 166);
	m_clrHighlightGradientLight = RGB (255, 250, 235);
	m_clrHighlightGradientDark = RGB (255, 236, 181);

	m_clrHighlightDnGradientDark = m_clrHighlightGradientLight;
	m_clrHighlightDnGradientLight =  m_clrHighlightGradientLight;
	m_clrHighlightDn = m_clrHighlightMenuItem;

	m_clrMenuLight = RGB (208, 215, 236);

	m_clrPressedButtonBorder = m_clrMenuItemBorder = RGB (229, 195, 101);

	m_penMenuItemBorder.DeleteObject ();
	m_penMenuItemBorder.CreatePen (PS_SOLID, 1, m_clrMenuItemBorder);

	m_brMenuLight.DeleteObject ();
	m_brMenuLight.CreateSolidBrush (m_clrMenuLight);

	m_clrToolBarGradientDark = m_clrBarGradientDark = m_clrToolBarGradientLight = RGB (188, 199, 216);
	m_clrBarGradientDark = m_clrBarGradientLight = RGB (156, 170, 193);

	m_clrToolBarGradientVertDark = m_clrToolBarGradientVertLight = m_clrToolBarGradientLight;

	m_clrToolBarBottomLine = m_clrCustomizeButtonGradientLight = m_clrCustomizeButtonGradientDark = RGB (213, 220, 232);

	m_colorToolBarCornerBottom = m_clrToolBarGradientDark;

	m_brTabBack.DeleteObject ();
	m_brTabBack.CreateSolidBrush (m_clrToolBarGradientLight);

	m_brFace.DeleteObject ();
	m_brFace.CreateSolidBrush (m_clrToolBarGradientLight);

	m_clrToolbarDisabled = CBCGPDrawManager::SmartMixColors (
		m_clrToolBarGradientDark, m_clrToolBarGradientLight, .92);

	m_penBottomLine.DeleteObject ();
	m_penBottomLine.CreatePen (PS_SOLID, 1, m_clrToolBarBottomLine);

	m_brMenuButtonDroppedDown.DeleteObject ();
	m_brMenuButtonDroppedDown.CreateSolidBrush (m_clrBarBkgnd);

	m_brMenuItemCheckedHighlight.DeleteObject ();
	m_brMenuItemCheckedHighlight.CreateSolidBrush (RGB(255, 252, 244));

	m_penSeparator.DeleteObject ();
	m_clrSeparator = RGB(133, 145, 162);
	m_penSeparator.CreatePen (PS_SOLID, 1, m_clrSeparator);

	m_clrMenuBorder = RGB(149, 149, 149);

	m_clrMenuItemGradient1 = RGB(255, 243, 207);
	m_clrMenuItemGradient2 = RGB(255, 251, 240);
	m_clrMenuItemGradientDark = m_clrHighlightGradientDark;

	m_clrMenuLightGradient = m_clrMenuGutter = RGB (233, 236, 238);
	m_brMenuGutter.DeleteObject();
	m_brMenuGutter.CreateSolidBrush(m_clrMenuGutter);

	COLORREF clrStatusBar = RGB (41, 57, 85);
	m_brStatusBar.DeleteObject();
	m_brStatusBar.CreateSolidBrush(clrStatusBar);

	m_clrMenuBarGradient1 = RGB (174, 185, 205);
	m_clrMenuBarGradient2 = RGB (202, 211, 226);

	m_clrHighlightedTabGradient1 = RGB(76, 92, 116);
	m_clrHighlightedTabGradient2 = RGB(107, 117, 121);

	m_clrGripper = RGB(96, 114, 140);

	m_clrMenuSeparator = RGB(190, 195, 203);

	m_brMenuConnect.DeleteObject();
	m_brMenuConnect.CreateSolidBrush(m_clrMenuGutter);

	m_clrEditBoxBorder = m_clrSeparator;
	m_clrEditBoxBorderDisabled = RGB(164, 173, 186);

	m_clrCombo = RGB(241, 243, 248);
	m_clrComboDisabled = RGB(213, 220, 232);

	m_brComboDisabled.DeleteObject();
	m_brComboDisabled.CreateSolidBrush(m_clrComboDisabled);

	m_brWhite.DeleteObject();
	m_brWhite.CreateSolidBrush(RGB(255, 255, 255));

	m_clrInactiveCaptionGradient1 = RGB (62, 83, 120);
	m_clrInactiveCaptionGradient2 = RGB (77, 96, 130);

	m_clrCaptionBarGradientDark = m_clrInactiveCaptionGradient1;
	m_clrCaptionBarGradientLight = m_clrInactiveCaptionGradient2;

	m_brDlgBackground.DeleteObject();
	m_brDlgBackground.CreateSolidBrush(m_clrToolBarGradientLight);

	CBCGPMenuImages::SetColor (CBCGPMenuImages::ImageWhite, RGB(206, 212, 221));

	m_clrEditCtrlSelectionBkActive = RGB(173, 195, 232);
	m_clrGridSelectionTextActive = m_clrEditCtrlSelectionTextActive = RGB(0, 0, 0);

	m_clrEditCtrlSelectionBkInactive = RGB(202, 213, 242);
	m_clrGridSelectionTextInactive = m_clrEditCtrlSelectionTextInactive = RGB(0, 0, 0);

	m_brGridSelectionBkActive.DeleteObject();
	m_brGridSelectionBkActive.CreateSolidBrush(m_clrEditCtrlSelectionBkActive);
	m_brGridSelectionBkInactive.DeleteObject();
	m_brGridSelectionBkInactive.CreateSolidBrush(m_clrEditCtrlSelectionBkInactive);

	m_clrReportGroupText = m_clrInactiveCaptionGradient1;

	m_brAutohideButton.DeleteObject();
	m_brAutohideButton.CreateSolidBrush(RGB (75, 94, 129));

	m_brDlgButtonsArea.DeleteObject ();
	m_brDlgButtonsArea.CreateSolidBrush (RGB (54, 73, 105));

	m_clrMainButton = m_clrInactiveCaptionGradient2;
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2010::GetTabFrameColors (const CBCGPBaseTabWnd* pTabWnd,
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
	
	CBCGPVisualManagerVS2008::GetTabFrameColors (pTabWnd,
			   clrDark, clrBlack,
			   clrHighlight, clrFace,
			   clrDarkShadow, clrLight,
			   pbrFace, pbrBlack);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || pTabWnd->IsDialogControl () ||
		pTabWnd->IsFlatTab ())
	{
		return;
	}

	clrFace = clrBlack;
	pbrFace  = &m_brTabs;
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawTabResizeBar (CDC* pDC, CBCGPBaseTabWnd* pWndTab, 
									BOOL bIsVert, CRect rect,
									CBrush* pbrFace, CPen* pPen)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pbrFace);
	ASSERT_VALID (pPen);
	ASSERT_VALID (pWndTab);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || pWndTab->IsFlatTab ())
	{
		CBCGPVisualManagerVS2008::OnDrawTabResizeBar (pDC, pWndTab, bIsVert, rect, pbrFace, pPen);
		return;
	}

	OnFillBackground(pDC, rect, pWndTab);
}
//*********************************************************************************************************
BCGP_SMARTDOCK_THEME CBCGPVisualManagerVS2010::GetSmartDockingTheme ()
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || !globalData.IsWindowsLayerSupportAvailable ())
	{
		return CBCGPVisualManagerVS2008::GetSmartDockingTheme ();
	}

	return BCGP_SDT_VS2010;
}
//*********************************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawSlider (CDC* pDC, CBCGPSlider* pSlider, CRect rect, BOOL bAutoHideMode)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::OnDrawSlider (pDC, pSlider, rect, bAutoHideMode);
		return;
	}

	ASSERT_VALID (pDC);

	OnFillBackground(pDC, rect, bAutoHideMode ? NULL : pSlider);
}
//*********************************************************************************************************
int CBCGPVisualManagerVS2010::GetTabButtonState (CBCGPTabWnd* pTab, CBCGTabButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || pButton->IsHighlighted () || pButton->IsPressed () || pTab == NULL || pTab->IsDialogControl() || pTab->IsPointerStyle())
	{
		return CBCGPVisualManagerVS2008::GetTabButtonState (pTab, pButton);
	}

	return (int) CBCGPMenuImages::ImageWhite;
}
//*****************************************************************************
COLORREF CBCGPVisualManagerVS2010::GetStatusBarPaneTextColor (CBCGPStatusBar* pStatusBar, CBCGStatusBarPaneInfo* pPane)
{
	ASSERT (pPane != NULL);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || pPane->clrText != (COLORREF)-1)
	{
		return CBCGPVisualManagerVS2008::GetStatusBarPaneTextColor (pStatusBar, pPane);
	}

	return (pPane->nStyle & SBPS_DISABLED) ? globalData.clrGrayedText : RGB(255, 255, 255);
}
//*****************************************************************************
void CBCGPVisualManagerVS2010::OnDrawComboDropButton (CDC* pDC, CRect rect,
												BOOL bDisabled,
												BOOL bIsDropped,
												BOOL bIsHighlighted,
												CBCGPToolbarComboBoxButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID (this);

	const BOOL bIsCtrl = pButton != NULL && pButton->IsCtrlButton ();

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || bIsDropped || bIsHighlighted || bIsCtrl)
	{
		CBCGPVisualManagerVS2008::OnDrawComboDropButton (pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}

	CBCGPMenuImages::Draw (pDC, CBCGPMenuImages::IdArowDown, rect,
		bDisabled ? CBCGPMenuImages::ImageGray : CBCGPMenuImages::ImageBlack);
}
//*************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawComboBorder (CDC* pDC, CRect rect,
												BOOL bDisabled,
												BOOL bIsDropped,
												BOOL bIsHighlighted,
												CBCGPToolbarComboBoxButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::OnDrawComboBorder (pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}

	rect.DeflateRect (1, 1);

	COLORREF clrBorder = bDisabled ? m_clrEditBoxBorderDisabled : (bIsHighlighted || bIsDropped) ? m_clrMenuItemBorder : m_clrEditBoxBorder;

	if (CBCGPToolBarImages::m_bIsDrawOnGlass)
	{
		CBCGPDrawManager dm (*pDC);
		dm.DrawRect (rect, (COLORREF)-1, clrBorder);
	}
	else
	{
		pDC->Draw3dRect (&rect,  clrBorder, clrBorder);
	}
}
//*************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawEditBorder (CDC* pDC, CRect rect,
												BOOL bDisabled,
												BOOL bIsHighlighted,
												CBCGPToolbarEditBoxButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || !CBCGPToolbarEditBoxButton::IsFlatMode ())
	{
		CBCGPVisualManagerVS2008::OnDrawEditBorder (pDC, rect, bDisabled, bIsHighlighted, pButton);
		return;
	}

	COLORREF clrBorder = bDisabled ? m_clrEditBoxBorderDisabled : bIsHighlighted ? m_clrMenuItemBorder : m_clrEditBoxBorder;

	if (CBCGPToolBarImages::m_bIsDrawOnGlass)
	{
		CBCGPDrawManager dm (*pDC);
		dm.DrawRect (rect, (COLORREF)-1, clrBorder);
	}
	else
	{
		pDC->Draw3dRect (&rect,  clrBorder, clrBorder);
	}
}
//*************************************************************************************
void CBCGPVisualManagerVS2010::OnFillButtonInterior (CDC* pDC,
	CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	CCustomizeButton* pCustButton = DYNAMIC_DOWNCAST (CCustomizeButton, pButton);

	CBCGPToolbarMenuButton* pMenuButton = 
		DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);

	BOOL bIsMenuBarButton = pMenuButton != NULL &&
		pMenuButton->GetParentWnd () != NULL &&
		pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar));

	BOOL bIsPopupMenuButton = pMenuButton != NULL &&
		pMenuButton->GetParentWnd () != NULL &&
		pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar));

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || pMenuButton == NULL || bIsPopupMenuButton || pCustButton != NULL)
	{
		CBCGPVisualManagerVS2008::OnFillButtonInterior (pDC, pButton, rect, state);
		return;
	}

	if (state != ButtonsIsPressed && state != ButtonsIsHighlighted)
	{
		return;
	}

	if (bIsMenuBarButton)
	{
		const int nRoundSize = 3;
		const int nOverlappSize = nRoundSize - 1;

		if (m_bConnectMenuToParent && pMenuButton->IsDroppedDown () && pMenuButton->GetPopupMenu() != NULL)
		{
			switch (pMenuButton->GetPopupMenu()->GetDropDirection ())
			{
			case CBCGPPopupMenu::DROP_DIRECTION_BOTTOM:
				rect.DeflateRect (1, 1, 0, -nOverlappSize);
				break;
				
			case CBCGPPopupMenu::DROP_DIRECTION_TOP:
				rect.DeflateRect (1, -nOverlappSize, 0, 1);
				break;
				
			case CBCGPPopupMenu::DROP_DIRECTION_RIGHT:
				rect.DeflateRect (1, 1, -nOverlappSize, 1);
				break;
				
			case CBCGPPopupMenu::DROP_DIRECTION_LEFT:
				rect.DeflateRect (-nOverlappSize, 1, 1, 1);
				break;
			}
		}
		else
		{
			rect.DeflateRect (1, 1);
		}

		CRgn rgn;
		rgn.CreateRoundRectRgn (rect.left, rect.top, rect.right, rect.bottom, nRoundSize, nRoundSize);

		pDC->SelectClipRgn (&rgn);

		if (pMenuButton->IsDroppedDown ())
		{
			if (CBCGPToolBarImages::m_bIsDrawOnGlass)
			{
				CBCGPDrawManager dm (*pDC);
				dm.DrawRect (rect, m_clrMenuGutter, (COLORREF)-1);
			}
			else
			{
				pDC->FillRect(rect, &m_brMenuGutter);
			}
		}
		else
		{
			CBCGPDrawManager dm (*pDC);
			dm.Fill4ColorsGradient(rect, m_clrHighlightGradientLight, m_clrHighlightGradientLight, m_clrHighlightGradientDark, m_clrHighlightGradientDark);
		}

		pDC->SelectClipRgn (NULL);

		rect.left--;
		rect.top--;

		CPen pen (PS_SOLID, 1, pMenuButton->IsDroppedDown () ? m_clrMenuBorder : m_clrMenuItemBorder);
		CPen* pOldPen = pDC->SelectObject (&pen);
		CBrush* pOldBrush = (CBrush*) pDC->SelectStockObject (NULL_BRUSH);

		pDC->RoundRect (rect.left, rect.top, rect.right, rect.bottom, nRoundSize + 2, nRoundSize + 2);

		pDC->SelectObject (pOldPen);
		pDC->SelectObject (pOldBrush);
	}
	else
	{
		if (pMenuButton->IsDroppedDown ())
		{
			ExtendMenuButton (pMenuButton, rect);
			OnFillHighlightedArea (pDC, rect, &m_brHighlightDn, pButton);
		}
		else
		{
			CBCGPVisualManagerVS2008::OnFillButtonInterior (pDC, pButton, rect, state);
		}
	}
}
//***********************************************************************************
void CBCGPVisualManagerVS2010::OnFillHighlightedArea (CDC* pDC, CRect rect, CBrush* pBrush, CBCGPToolbarButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::OnFillHighlightedArea (pDC, rect, pBrush, pButton);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pBrush);

	CBCGPToolbarMenuButton* pMenuButton = 
		DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);

	BOOL bIsPopupMenu = pMenuButton != NULL &&
		pMenuButton->GetParentWnd () != NULL &&
		pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar));

	if (pButton != NULL && !bIsPopupMenu)
	{
		if (pBrush == &m_brHighlight)
		{
			CBCGPDrawManager dm (*pDC);
			dm.Fill4ColorsGradient(rect, m_clrHighlightGradientLight, m_clrHighlightGradientLight, m_clrHighlightGradientDark, m_clrHighlightGradientDark);
			return;
		}
		else if (pBrush == &m_brHighlightDn)
		{
			if (CBCGPToolBarImages::m_bIsDrawOnGlass)
			{
				CBCGPDrawManager dm (*pDC);
				dm.DrawRect (rect, m_clrHighlightDn, (COLORREF)-1);
			}
			else
			{
				pDC->FillRect(rect, &m_brHighlightDn);
			}
			return;
		}
		else if (pBrush == &m_brHighlightChecked)
		{
			if (CBCGPToolBarImages::m_bIsDrawOnGlass)
			{
				CBCGPDrawManager dm (*pDC);
				dm.DrawRect (rect, m_clrHighlightChecked, (COLORREF)-1);
			}
			else
			{
				pDC->FillRect(rect, &m_brHighlight);
			}
			return;
		}
	}

	CBCGPVisualManagerVS2008::OnFillHighlightedArea (pDC, rect, pBrush, pButton);
}
//*************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawButtonBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, BCGBUTTON_STATE state)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	CBCGPToolbarMenuButton* pMenuButton = 
		DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);

	BOOL bIsMenuBarButton = pMenuButton != NULL &&
		pMenuButton->GetParentWnd () != NULL &&
		pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar));

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || !bIsMenuBarButton)
	{
		CBCGPVisualManagerVS2008::OnDrawButtonBorder (pDC, pButton, rect, state);
	}
}
//*************************************************************************************
int CBCGPVisualManagerVS2010::GetTabExtraHeight(const CBCGPTabWnd* pTabWnd)
{
	ASSERT_VALID(pTabWnd);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		pTabWnd->IsDialogControl () || !pTabWnd->IsMDITab() ||
		pTabWnd->IsFlatTab () || pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style ())
	{
		return CBCGPVisualManagerVS2008::GetTabExtraHeight(pTabWnd);
	}

	return 2 * globalData.GetTextMargins () - 2;
}
//*************************************************************************************
int CBCGPVisualManagerVS2010::GetTabsMargin(const CBCGPTabWnd* pTabWnd)
{
	ASSERT_VALID(pTabWnd);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		pTabWnd->IsDialogControl () || !pTabWnd->IsMDITab() ||
		pTabWnd->IsFlatTab () || pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style ())
	{
		return CBCGPVisualManagerVS2008::GetTabExtraHeight(pTabWnd);
	}

	return pTabWnd->GetTabBorderSize() + 1;
}
//*************************************************************************************
BOOL CBCGPVisualManagerVS2010::OnEraseMDIClientArea (CDC* pDC, CRect rectClient)
{
	if (globalData.m_nBitsPerPixel > 8 && !globalData.IsHighContastMode ())
	{
		CMDIFrameWnd* pMDIFrameWnd = DYNAMIC_DOWNCAST (CMDIFrameWnd, AfxGetMainWnd ());
		if (pMDIFrameWnd != NULL)
		{
			OnFillBackground(pDC, rectClient, CWnd::FromHandle(pMDIFrameWnd->m_hWndMDIClient));
			return TRUE;
		}
	}

	return CBCGPVisualManagerVS2008::OnEraseMDIClientArea (pDC, rectClient);
}
//*************************************************************************************
HBRUSH CBCGPVisualManagerVS2010::GetToolbarEditColors(CBCGPToolbarButton* pButton, COLORREF& clrText, COLORREF& clrBk)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2008::GetToolbarEditColors(pButton, clrText, clrBk);
	}

	if (pButton != NULL && !CBCGPToolBar::IsCustomizeMode () && (pButton->m_nStyle & TBBS_DISABLED))
	{
		clrText = globalData.clrBtnDkShadow;
		clrBk = m_clrComboDisabled;

		return (HBRUSH)m_brComboDisabled;
	}

	clrText = RGB(0, 0, 0);
	clrBk = RGB(255, 255, 255);

	return (HBRUSH) m_brWhite.GetSafeHandle ();
}
//*************************************************************************************
void CBCGPVisualManagerVS2010::OnFillCombo (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted,
										CBCGPToolbarComboBoxButton* pButton)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::OnFillCombo(pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}

	if (CBCGPToolBarImages::m_bIsDrawOnGlass)
	{
		CBCGPDrawManager dm (*pDC);

		dm.DrawRect (rect, bDisabled ? m_clrComboDisabled : m_clrCombo, (COLORREF)-1);
	}
	else
	{
		pDC->FillSolidRect (rect, bDisabled ? m_clrComboDisabled : m_clrCombo);
	}
}
//************************************************************************************
void CBCGPVisualManagerVS2010::ModifyGlobalColors ()
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::ModifyGlobalColors();
		return;
	}

	globalData.clrBarFace = RGB(240, 240, 240);
	globalData.clrBarShadow = RGB(160, 160, 160);
	globalData.clrBarHilite = RGB(255, 255, 255);
	globalData.clrBarDkShadow = RGB(120, 120, 120);;
	globalData.clrBarLight = RGB(245, 245, 245);;

	globalData.brBarFace.DeleteObject ();
	globalData.brBarFace.CreateSolidBrush (globalData.clrBarFace);
}
//************************************************************************************
COLORREF CBCGPVisualManagerVS2010::OnFillMiniFrameCaption (CDC* pDC, 
								CRect rectCaption, 
								CBCGPMiniFrameWnd* pFrameWnd,
								BOOL bActive)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pFrameWnd);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2008::OnFillMiniFrameCaption (pDC, rectCaption, pFrameWnd, bActive);
	}

	if (DYNAMIC_DOWNCAST (CBCGPBaseToolBar, pFrameWnd->GetControlBar ()) != NULL)
	{
		bActive = FALSE;
	}

	CBCGPDrawManager dm (*pDC);
	if (bActive)
	{
		dm.Fill4ColorsGradient(rectCaption, m_clrMenuItemGradient1, m_clrMenuItemGradient2, m_clrMenuItemGradientDark, m_clrMenuItemGradientDark);
	}
	else
	{
		dm.FillGradient(rectCaption, m_clrInactiveCaptionGradient1, m_clrInactiveCaptionGradient2);
	}

	return bActive ? globalData.clrBarText : RGB(255, 255, 255);
}
//************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawMiniFrameBorder (
										CDC* pDC, CBCGPMiniFrameWnd* pFrameWnd,
										CRect rectBorder, CRect rectBorderSize)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::OnDrawMiniFrameBorder (pDC, pFrameWnd, rectBorder, rectBorderSize);
		return;
	}

	ASSERT_VALID (pDC);

	CRect rectClip = rectBorder;
	rectClip.DeflateRect(rectClip);

	pDC->ExcludeClipRect(rectClip);

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient(rectBorder, m_clrInactiveCaptionGradient1, m_clrInactiveCaptionGradient2);

	pDC->SelectClipRgn (NULL);
}
//************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawFloatingToolbarBorder (
												CDC* pDC, CBCGPBaseToolBar* pToolBar, 
												CRect rectBorder, CRect rectBorderSize)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::OnDrawFloatingToolbarBorder (pDC, pToolBar, rectBorder, rectBorderSize);
		return;
	}

	OnDrawMiniFrameBorder (pDC, NULL, rectBorder, rectBorderSize);
}
//**************************************************************************************
COLORREF CBCGPVisualManagerVS2010::GetToolbarButtonTextColor (CBCGPToolbarButton* pButton, CBCGPVisualManager::BCGBUTTON_STATE state)
{
	ASSERT_VALID (pButton);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || pButton->IsKindOf (RUNTIME_CLASS (CBCGPOutlookButton)))
	{
		return CBCGPVisualManagerVS2008::GetToolbarButtonTextColor (pButton, state);
	}

	BOOL bDisabled = (CBCGPToolBar::IsCustomizeMode () && !pButton->IsEditable ()) ||
			(!CBCGPToolBar::IsCustomizeMode () && (pButton->m_nStyle & TBBS_DISABLED));

	return bDisabled ? globalData.clrGrayedText : globalData.clrBtnText;
}
//**************************************************************************************
BOOL CBCGPVisualManagerVS2010::OnFillDialog (CDC* pDC, CWnd* pDlg, CRect rect)
{
	if (m_brDlgBackground.GetSafeHandle () == NULL)
	{
		return CBCGPVisualManagerVS2008::OnFillDialog (pDC, pDlg, rect);
	}

	ASSERT_VALID (pDC);
	pDC->FillRect (rect, &GetDlgBackBrush (pDlg));

	return TRUE;
}
//**************************************************************************************
BOOL CBCGPVisualManagerVS2010::IsOwnerDrawDlgSeparator(CBCGPStatic* pCtrl)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2008::IsOwnerDrawDlgSeparator(pCtrl);
	}

	return TRUE;
}
//**************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawDlgSeparator(CDC* pDC, CBCGPStatic* pCtrl, CRect rect, BOOL bIsHorz)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || pCtrl == NULL)
	{
		CBCGPVisualManagerVS2008::OnDrawDlgSeparator(pDC, pCtrl, rect, bIsHorz);
		return;
	}

	if (pCtrl->m_bBackstageMode && IsRibbonBackstageWhiteBackground())
	{
		::FillRect (pDC->GetSafeHdc (), rect, (HBRUSH)::GetStockObject (WHITE_BRUSH));
	}
	else
	{
		pDC->FillRect(rect, &m_brDlgBackground);
	}

	int x1 = bIsHorz ? rect.left : rect.CenterPoint().x;
	int x2 = bIsHorz ? rect.right : rect.CenterPoint().x;

	int y1 = bIsHorz ? rect.CenterPoint().y : rect.top;
	int y2 = bIsHorz ? rect.CenterPoint().y : rect.bottom;

	CBCGPDrawManager dm (*pDC);
	dm.DrawLine (x1, y1, x2, y2, m_clrSeparator);
}
//**************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawGroup (CDC* pDC, CBCGPGroup* pGroup, CRect rect, const CString& strName)
{
	ASSERT_VALID (pGroup);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || pGroup->m_bOnGlass)
	{
		CBCGPVisualManagerVS2008::OnDrawGroup (pDC, pGroup, rect, strName);
		return;
	}

	ASSERT_VALID (pDC);

	CSize sizeText = pDC->GetTextExtent (strName);

	CRect rectFrame = rect;
	rectFrame.top += sizeText.cy / 2;

	if (sizeText == CSize (0, 0))
	{
		rectFrame.top += pDC->GetTextExtent (_T("A")).cy / 2;
	}

	int xMargin = sizeText.cy / 2;

	CRect rectText = rect;
	rectText.left += xMargin;
	rectText.right = rectText.left + sizeText.cx + xMargin;
	rectText.bottom = rectText.top + sizeText.cy;

	if (!strName.IsEmpty ())
	{
		pDC->ExcludeClipRect (rectText);
	}

	CPen* pOldPen = (CPen*)pDC->SelectObject(&m_penSeparator);
	CBrush* pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

	pDC->RoundRect(rectFrame, CPoint(6, 6));

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);

	pDC->SelectClipRgn (NULL);

	if (!strName.IsEmpty ())
	{
		CString strCaption = strName;
		pDC->DrawText (strCaption, rectText, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOCLIP);
	}
}
//*************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawControlBorder (CDC* pDC, CRect rect, CWnd* pWndCtrl, BOOL bDrawOnGlass)
{
	if (m_hThemeComboBox != NULL || globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::OnDrawControlBorder (pDC, rect, pWndCtrl, bDrawOnGlass);
		return;
	}

	COLORREF clrBorder = m_clrSeparator;

	if (bDrawOnGlass)
	{
		CBCGPDrawManager dm (*pDC);
		dm.DrawRect (rect, (COLORREF)-1, clrBorder);
	}
	else
	{
		pDC->Draw3dRect (&rect, clrBorder, clrBorder);
		rect.DeflateRect(1, 1);
		pDC->Draw3dRect (&rect, globalData.clrWindow, globalData.clrWindow);
	}
}
//**************************************************************************************
BOOL CBCGPVisualManagerVS2010::OnDrawPushButton (CDC* pDC, CRect rect, CBCGPButton* pButton, COLORREF& clrText)
{
	ASSERT_VALID(pDC);

	BOOL bOnGlass = pButton->GetSafeHwnd() != NULL && pButton->m_bOnGlass;

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || bOnGlass ||
		m_brDlgBackground.GetSafeHandle () == NULL)
	{
		return CBCGPVisualManagerVS2008::OnDrawPushButton (pDC, rect, pButton, clrText);
	}

	ASSERT_VALID (pDC);

	globalData.DrawParentBackground (pButton, pDC);

	CPen pen (PS_SOLID, 1, globalData.clrBarShadow);
	CPen* pOldPen = pDC->SelectObject (&pen);

	CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&globalData.brBarFace);

	if (pButton->GetSafeHwnd() != NULL && pButton->IsWindowEnabled ())
	{
		if (pButton->IsPressed () || pButton->GetCheck ())
		{
			pDC->SelectObject(&m_brHighlightDn);
			pDC->SelectObject(&m_penMenuItemBorder);
		}
		else if (pButton->IsHighlighted () || CWnd::GetFocus () == pButton)
		{
			pDC->SelectObject(&m_brHighlight);
			pDC->SelectObject(&m_penMenuItemBorder);
		}
	}

	pDC->RoundRect(rect, CPoint(5, 5));

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);

	clrText = globalData.clrBarText;

	return TRUE;
}
//**************************************************************************************
COLORREF CBCGPVisualManagerVS2010::GetToolbarDisabledTextColor ()
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2008::GetToolbarDisabledTextColor ();
	}

	return RGB(140, 140, 140);
}

#ifndef BCGP_EXCLUDE_GRID_CTRL

void CBCGPVisualManagerVS2010::OnDrawGridExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::OnDrawGridExpandingBox(pDC, rect, bIsOpened, colorBox);
		return;
	}

	OnDrawExpandingBox(pDC, rect, bIsOpened, colorBox);
}
//**************************************************************************************
void CBCGPVisualManagerVS2010::OnFillGridHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::OnFillGridHeaderBackground (pCtrl, pDC, rect);
		return;
	}
	
	ASSERT_VALID(pDC);
	CBCGPDrawManager dm (*pDC);
	dm.FillGradient(rect, m_clrMenuLight, m_clrMenuGutter);
}
//**************************************************************************************
BOOL CBCGPVisualManagerVS2010::OnDrawGridHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2008::OnDrawGridHeaderItemBorder (pCtrl, pDC, rect, bPressed);
	}

	ASSERT_VALID(pDC);

 	COLORREF clrStart  = bPressed ? m_clrHighlightGradientDark : m_clrMenuLight;
 	COLORREF clrFinish = bPressed ? m_clrHighlightGradientLight : m_clrMenuGutter;

	{
		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rect, clrStart, clrFinish);
	}

	CPen* pOldPen = pDC->SelectObject (&m_penSeparator);
	
	pDC->MoveTo (rect.right - 1, rect.top);
	pDC->LineTo (rect.right - 1, rect.bottom);
	
	pDC->MoveTo (rect.left, rect.top);
	pDC->LineTo (rect.right, rect.top);
	
	pDC->MoveTo (rect.left, rect.bottom - 1);
	pDC->LineTo (rect.right, rect.bottom - 1);
	
	pDC->SelectObject (pOldPen);
	
	return FALSE;
}
//**************************************************************************************
void CBCGPVisualManagerVS2010::OnFillGridRowHeaderBackground (CBCGPGridCtrl* /*pCtrl*/, CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);
	pDC->FillRect (rect, &m_brMenuLight);
}
//**************************************************************************************
BOOL CBCGPVisualManagerVS2010::OnDrawGridRowHeaderItemBorder (CBCGPGridCtrl* /*pCtrl*/, CDC* pDC, CRect rect, BOOL /*bPressed*/)
{
	ASSERT_VALID (pDC);
	
	CPen* pOldPen = pDC->SelectObject (&m_penSeparator);
	
	pDC->MoveTo (rect.left, rect.top);
	pDC->LineTo (rect.left, rect.bottom);
	
	pDC->MoveTo (rect.right - 1, rect.top);
	pDC->LineTo (rect.right - 1, rect.bottom);
	
	pDC->MoveTo (rect.left, rect.top);
	pDC->LineTo (rect.right - 1, rect.top);
	
	pDC->SelectObject (pOldPen);
	
	return TRUE;
}
//**************************************************************************************
void CBCGPVisualManagerVS2010::OnFillGridSelectAllAreaBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::OnFillGridSelectAllAreaBackground (pCtrl, pDC, rect, bPressed);
		return;
	}

	ASSERT_VALID(pDC);
	
	CBCGPDrawManager dm(*pDC);
	dm.FillGradient(rect, m_clrMenuLight, m_clrMenuGutter);
}
//**************************************************************************************
BOOL CBCGPVisualManagerVS2010::OnDrawGridSelectAllAreaBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2008::OnDrawGridSelectAllAreaBorder (pCtrl, pDC, rect, bPressed);
	}
	
	ASSERT_VALID(pDC);
	pDC->Draw3dRect (rect, m_clrSeparator, m_clrSeparator);
	
	return FALSE;
}
//**************************************************************************************
COLORREF CBCGPVisualManagerVS2010::OnFillGridGroupByBoxBackground (CDC* pDC, CRect rect)
{
	pDC->FillRect(rect, &m_brDlgBackground);
	return globalData.clrBarText;
}
//**************************************************************************************
COLORREF CBCGPVisualManagerVS2010::OnFillGridGroupByBoxTitleBackground (CDC* pDC, CRect rect)
{
	pDC->FillRect(rect, &m_brDlgBackground);
	return globalData.clrBarText;
}
//**************************************************************************************
COLORREF CBCGPVisualManagerVS2010::GetGridGroupByBoxLineColor () const
{
	return globalData.clrBarDkShadow;
}
//**************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawGridGroupByBoxItemBorder (CBCGPGridCtrl* /*pCtrl*/, CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient(rect, m_clrMenuLight, m_clrMenuGutter);

	pDC->Draw3dRect (rect, globalData.clrBarWindow, globalData.clrBtnShadow);
}
//**************************************************************************************
COLORREF CBCGPVisualManagerVS2010::GetGridLeftOffsetColor (CBCGPGridCtrl* /*pCtrl*/)
{
	return m_clrMenuGutter;
}
//**************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawGridGroupUnderline (CBCGPGridCtrl* /*pCtrl*/, CDC* pDC, CRect rectFill)
{
	ASSERT_VALID (pDC);
	
	COLORREF clrOld = pDC->GetBkColor ();
	pDC->FillSolidRect (rectFill, m_clrReportGroupText);
	pDC->SetBkColor (clrOld);
}
//**************************************************************************************
COLORREF CBCGPVisualManagerVS2010::OnFillGridRowBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, 
															CRect rectFill, BOOL bSelected)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pCtrl);

	if (m_brGridSelectionBkActive.GetSafeHandle () == NULL ||
		m_brGridSelectionBkInactive.GetSafeHandle () == NULL)
	{
		return CBCGPVisualManagerVS2008::OnFillGridRowBackground (pCtrl, pDC, rectFill, bSelected);
	}

	// Fill area:
	if (!pCtrl->IsFocused ())
	{
		pDC->FillRect (rectFill, 
			pCtrl->IsControlBarColors () ? &globalData.brBarFace : &globalData.brBtnFace);
	}
	else
	{
		pDC->FillRect (rectFill, &m_brGridSelectionBkActive);
	}
	
	// Return text color:
	if (!pCtrl->IsHighlightGroups () && bSelected)
	{
		return (!pCtrl->IsFocused ()) ? m_clrGridSelectionTextInactive : m_clrGridSelectionTextActive;
	}
 	
	return pCtrl->IsHighlightGroups () ? 
		(pCtrl->IsControlBarColors () ? globalData.clrBarShadow : globalData.clrBtnShadow) :
		globalData.clrWindowText;
}
//**************************************************************************************
COLORREF CBCGPVisualManagerVS2010::OnFillGridItem (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill,
								 BOOL bSelected, BOOL bActiveItem, BOOL bSortedColumn)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pCtrl);

	if (m_brGridSelectionBkActive.GetSafeHandle () == NULL ||
		m_brGridSelectionBkInactive.GetSafeHandle () == NULL)
	{
		return CBCGPVisualManagerVS2008::OnFillGridItem (pCtrl, pDC, rectFill, bSelected, bActiveItem, bSortedColumn);
	}

	// Fill area:
	if (bSelected && !bActiveItem)
	{
		if (!pCtrl->IsFocused ())
		{
			pDC->FillRect (rectFill, &m_brGridSelectionBkInactive);
			return m_clrGridSelectionTextInactive;
		}
		else
		{
			pDC->FillRect (rectFill, &m_brGridSelectionBkActive);
			return m_clrGridSelectionTextActive;
		}
	}

	return CBCGPVisualManagerVS2008::OnFillGridItem (pCtrl, pDC, rectFill, bSelected, bActiveItem, bSortedColumn);
}
//**************************************************************************************
void CBCGPVisualManagerVS2010::OnDrawGridSelectionBorder (CBCGPGridCtrl* /*pCtrl*/, CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);
	
	pDC->Draw3dRect (rect, m_clrSeparator, m_clrSeparator);
	rect.DeflateRect (1, 1);
	pDC->Draw3dRect (rect, m_clrSeparator, m_clrSeparator);
}
//**************************************************************************************
BOOL CBCGPVisualManagerVS2010::OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme)
{
	CBCGPVisualManagerVS2008::OnSetGridColorTheme (pCtrl, theme);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return TRUE;
	}

	theme.m_HeaderSelColors.m_clrBackground = theme.m_SelColors.m_clrBackground;
	theme.m_HeaderSelColors.m_clrGradient = theme.m_SelColors.m_clrGradient;
	theme.m_HeaderSelColors.m_clrBorder = theme.m_SelColors.m_clrBorder;
	theme.m_HeaderSelColors.m_clrText = theme.m_SelColors.m_clrText;

	theme.m_OddColors.m_clrBackground       = globalData.clrBarLight;
	theme.m_OddColors.m_clrText             = globalData.clrBarText;

	return TRUE;
}
//**************************************************************************************
COLORREF CBCGPVisualManagerVS2010::OnFillReportCtrlRowBackground (CBCGPGridCtrl* pCtrl, CDC* pDC,
												CRect rectFill, BOOL bSelected, BOOL bGroup)
{
	if (bSelected)
	{
		if (!pCtrl->IsFocused ())
		{
			pDC->FillRect (rectFill, &m_brGridSelectionBkInactive);
		}
		else
		{
			pDC->FillRect (rectFill, &m_brGridSelectionBkActive);
		}

		return m_clrReportGroupText;
	}

	return CBCGPVisualManagerVS2008::OnFillReportCtrlRowBackground (pCtrl, pDC, rectFill, bSelected, bGroup);
}
//**************************************************************************************
CBrush& CBCGPVisualManagerVS2010::GetGridCaptionBrush(CBCGPGridCtrl* pCtrl)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2008::GetGridCaptionBrush(pCtrl);
	}

	if (pCtrl != NULL && !pCtrl->IsVisualManagerStyle())
	{
		return globalData.brBtnFace;
	}

	return CBCGPVisualManagerVS2008::GetGridCaptionBrush(pCtrl);
}
//**************************************************************************************
COLORREF CBCGPVisualManagerVS2010::OnFillGridCaptionRow (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill)
{
	COLORREF clrText = CBCGPVisualManagerVS2008::OnFillGridCaptionRow(pCtrl, pDC, rectFill);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return clrText;
	}

	if (pCtrl != NULL && !pCtrl->IsVisualManagerStyle())
	{
		clrText = globalData.clrBtnText;
	}

	return clrText;
}

#endif // BCGP_EXCLUDE_GRID_CTRL

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

void CBCGPVisualManagerVS2010::DrawGanttHeaderCell (const CBCGPGanttChart* pChart, CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, BOOL bHilite)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::DrawGanttHeaderCell(pChart, dc, cellInfo, bHilite);
		return;
	}

	CBCGPDrawManager dm(dc);

	dm.FillGradient(cellInfo.rectCell, m_clrMenuLight, m_clrMenuGutter);

	CPen* pOldPen = dc.SelectObject (&m_penSeparator);
	
	dc.MoveTo (cellInfo.rectCell.right - 1, cellInfo.rectCell.top);
	dc.LineTo (cellInfo.rectCell.right - 1, cellInfo.rectCell.bottom);
	
	dc.MoveTo (cellInfo.rectCell.left, cellInfo.rectCell.bottom - 1);
	dc.LineTo (cellInfo.rectCell.right, cellInfo.rectCell.bottom - 1);
	
	dc.SelectObject (pOldPen);
}

#endif

#ifndef BCGP_EXCLUDE_PROP_LIST

void CBCGPVisualManagerVS2010::OnFillPropListToolbarArea(CDC* pDC, CBCGPPropList* pList, const CRect& rectToolBar)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || (pList != NULL && !pList->DrawControlBarColors()))
	{
		CBCGPVisualManagerVS2008::OnFillPropListToolbarArea(pDC, pList, rectToolBar);
		return;
	}

	pDC->FillRect(rectToolBar, &m_brFace);
}

#endif

#ifndef BCGP_EXCLUDE_RIBBON

void CBCGPVisualManagerVS2010::OnDrawRibbonCategory (
		CDC* pDC, 
		CBCGPRibbonCategory* pCategory,
		CRect rectCategory)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::OnDrawRibbonCategory (pDC, pCategory, rectCategory);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pCategory);

	const int nShadowSize = 2;

	rectCategory.right -= nShadowSize;
	rectCategory.bottom -= nShadowSize;

	pDC->FillRect (rectCategory, &m_brMenuLight);

	CRect rectActiveTab = pCategory->GetTabRect ();

	CPen pen (PS_SOLID, 1, globalData.clrBarShadow);
	CPen* pOldPen = pDC->SelectObject (&pen);
	ASSERT (pOldPen != NULL);

	pDC->MoveTo (rectCategory.left, rectCategory.top);
	pDC->LineTo (rectActiveTab.left + 1, rectCategory.top);

	pDC->MoveTo (rectActiveTab.right - 2, rectCategory.top);
	pDC->LineTo (rectCategory.right, rectCategory.top);
	pDC->LineTo (rectCategory.right, rectCategory.bottom);
	pDC->LineTo (rectCategory.left, rectCategory.bottom);
	pDC->LineTo (rectCategory.left, rectCategory.top);

	pDC->SelectObject (pOldPen);

	CBCGPDrawManager dm (*pDC);
	dm.DrawShadow (rectCategory, nShadowSize, 100, 75, NULL, NULL,
		m_clrMenuShadowBase);
}
//*****************************************************************************
void CBCGPVisualManagerVS2010::GetRibbonSliderColors (CBCGPRibbonSlider* pSlider, 
					BOOL bIsHighlighted, 
					BOOL bIsPressed,
					BOOL bIsDisabled,
					COLORREF& clrLine,
					COLORREF& clrFill)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::GetRibbonSliderColors(pSlider, bIsHighlighted, bIsPressed, bIsDisabled, clrLine, clrFill);
		return;
	}

	if (bIsDisabled)
	{
		clrLine = globalData.clrBarShadow;
		return;
	}

	if (bIsPressed && bIsHighlighted)
	{
		clrFill = m_clrHighlightDn;
		clrLine = m_clrMenuBorder;
	}
	else if (bIsHighlighted)
	{
		clrFill = m_clrHighlight;
		clrLine = m_clrMenuBorder;
	}
	else
	{
		clrFill = globalData.clrBarFace;

		if (DYNAMIC_DOWNCAST(CBCGPRibbonStatusBar, pSlider->GetParentRibbonBar()) != NULL)
		{
			clrLine = RGB(255, 255, 255);
		}
		else
		{
			clrLine = globalData.clrBarText;
		}
	}
}
//**************************************************************************************
COLORREF CBCGPVisualManagerVS2010::GetRibbonStatusBarTextColor (CBCGPRibbonStatusBar* pStatusBar)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2008::GetRibbonStatusBarTextColor(pStatusBar);
	}

	return RGB(255, 255, 255);
}
//********************************************************************************
COLORREF CBCGPVisualManagerVS2010::GetRibbonHyperlinkTextColor (CBCGPRibbonHyperlink* pHyperLink)
{
	ASSERT_VALID (pHyperLink);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || pHyperLink->IsDisabled ())
	{
		return CBCGPVisualManagerVS2008::GetRibbonHyperlinkTextColor (pHyperLink);
	}

	if (DYNAMIC_DOWNCAST(CBCGPRibbonStatusBar, pHyperLink->GetParentRibbonBar()) != NULL)
	{
		return pHyperLink->IsHighlighted () ? RGB (174, 222, 241) :RGB(135, 206, 235);
	}

	return CBCGPVisualManagerVS2008::GetRibbonHyperlinkTextColor (pHyperLink);
}
//**************************************************************************************
COLORREF CBCGPVisualManagerVS2010::OnDrawRibbonStatusBarPane(CDC* pDC, CBCGPRibbonStatusBar* pBar, CBCGPRibbonStatusBarPane* pPane)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || pPane->IsDisabled())
	{
		return CBCGPVisualManagerVS2008::OnDrawRibbonStatusBarPane(pDC, pBar, pPane);
	}

	CRect rect = pPane->GetRect ();

	if (pPane->IsHighlighted ())
	{
		CRect rectButton = rect;
		rectButton.DeflateRect (1, 1);

		OnFillHighlightedArea (pDC, rectButton, 
			pPane->IsPressed () ? 
			&m_brHighlightDn : &m_brHighlight, NULL);

		pDC->Draw3dRect (rectButton, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}

	return pPane->IsHighlighted () ? (COLORREF)-1 : RGB(255, 255, 255);
}
//*****************************************************************************
COLORREF CBCGPVisualManagerVS2010::OnDrawRibbonTabsFrame (
					CDC* pDC, 
					CBCGPRibbonBar* pWndRibbonBar, 
					CRect rectTab)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2008::OnDrawRibbonTabsFrame (pDC, pWndRibbonBar, rectTab);
	}

	return (COLORREF)-1;
}

#endif

void CBCGPVisualManagerVS2010::OnDrawExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2008::OnDrawExpandingBox (pDC, rect, bIsOpened, colorBox);
		return;
	}

	if (colorBox == globalData.clrBarText)
	{
		colorBox = globalData.clrBarDkShadow;
	}
	else if (colorBox == globalData.clrBtnText)
	{
		colorBox = globalData.clrBtnDkShadow;
	}

	int nSize = min(rect.Width(), rect.Height());

	rect = CRect(CPoint(rect.CenterPoint().x - nSize / 2, rect.CenterPoint().y - nSize / 2), CSize(nSize, nSize));

	CBCGPDrawManager dm (*pDC);

	rect.DeflateRect(1, 1);

	if (bIsOpened)
	{
		#define POINTS_NUM	3
		POINT pts[POINTS_NUM] = {
			{ rect.left, rect.bottom},
			{ rect.right, rect.bottom},
			{ rect.right, rect.top},
		};

		CRgn rgnClip;
		rgnClip.CreatePolygonRgn (pts, POINTS_NUM, WINDING);

		pDC->SelectClipRgn (&rgnClip);

		dm.DrawRect(rect, colorBox, (COLORREF)-1);
		pDC->SelectClipRgn (NULL);
	}
	else
	{
		if ((rect.Height() % 2) == 0)
		{
			rect.bottom--;
		}

		rect.right = rect.left + rect.Height() * 5 / 7;

		if ((rect.Width() % 2) == 0)
		{
			if (globalData.m_bIsRTL)
			{
				rect.left++;
			}
			else
			{
				rect.right--;
			}
		}

		if (globalData.m_bIsRTL)
		{
			dm.DrawLineA(rect.right, rect.top, rect.left, rect.CenterPoint().y, colorBox);
			dm.DrawLineA(rect.left, rect.CenterPoint().y, rect.right, rect.bottom, colorBox);
		}
		else
		{
			dm.DrawLineA(rect.left, rect.top, rect.right, rect.CenterPoint().y, colorBox);
			dm.DrawLineA(rect.right, rect.CenterPoint().y, rect.left, rect.bottom, colorBox);
		}

		dm.DrawLineA(rect.left, rect.bottom, rect.left, rect.top, colorBox);
	}
}
//*******************************************************************************
COLORREF CBCGPVisualManagerVS2010::GetTreeControlFillColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected, BOOL bIsFocused, BOOL bIsDisabled)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2008::GetTreeControlFillColor(pTreeCtrl, bIsSelected, bIsFocused, bIsDisabled);
	}

	if (bIsSelected)
	{
		return bIsFocused ? m_clrHighlightGradientDark : globalData.clrBarShadow;
	}

	return (COLORREF)-1;
}
//*******************************************************************************
COLORREF CBCGPVisualManagerVS2010::GetTreeControlTextColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected, BOOL bIsFocused, BOOL bIsDisabled)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2008::GetTreeControlTextColor(pTreeCtrl, bIsSelected, bIsFocused, bIsDisabled);
	}

	if (bIsSelected)
	{
		return globalData.clrBarText;
	}

	return (COLORREF)-1;
}
//***********************************************************************************
void CBCGPVisualManagerVS2010::OnDrawCaptionBarBorder (CDC* pDC, 
	CBCGPCaptionBar* pBar, CRect rect, COLORREF clrBarBorder, BOOL bFlatBorder)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || 
		clrBarBorder != (COLORREF) -1 || (pBar != NULL && pBar->m_clrBarBackground != (COLORREF)-1) ||
		(pBar != NULL && pBar->IsDialogControl ()))
	{
		CBCGPVisualManagerVS2008::OnDrawCaptionBarBorder(pDC, pBar, rect, clrBarBorder, bFlatBorder);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT(clrBarBorder == (COLORREF) -1);

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, m_clrCaptionBarGradientDark, m_clrCaptionBarGradientLight, TRUE);

	if (!bFlatBorder)
	{
		pDC->Draw3dRect (rect, m_clrBarGradientLight, m_clrToolBarBottomLine);
	}
}
