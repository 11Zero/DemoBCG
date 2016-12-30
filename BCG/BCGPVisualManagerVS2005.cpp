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
// BCGPVisualManagerVS2005.cpp: implementation of the CBCGPVisualManagerVS2005 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "multimon.h"
#include "bcgcbpro.h"
#include "bcgglobals.h"
#include "BCGPDrawManager.h"
#include "BCGPCaptionButton.h"
#include "BCGPTabWnd.h"
#include "BCGPVisualManagerVS2005.h"
#include "BCGPAutoHideButton.h"
#include "BCGPToolBar.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPStatusBar.h"
#include "BCGPDockManager.h"
#include "BCGPTabbedControlBar.h"
#include "BCGPPropList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BOOL CBCGPVisualManagerVS2005::m_bRoundedAutohideButtons = FALSE;

IMPLEMENT_DYNCREATE(CBCGPVisualManagerVS2005, CBCGPVisualManager2003)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPVisualManagerVS2005::CBCGPVisualManagerVS2005()
{
	m_bAlwaysFillTab = TRUE;
	m_b3DTabWideBorder = FALSE;
	m_bShdowDroppedDownMenuButton = TRUE;
	m_bDrawLastTabLine = FALSE;
	m_colorActiveTabBorder = (COLORREF)-1;
	m_bFrameMenuCheckedItems = TRUE;

	CBCGPDockManager::EnableDockBarMenu ();
	CBCGPDockManager::SetDockMode (BCGP_DT_SMART);
	CBCGPAutoHideButton::m_bOverlappingTabs = FALSE;
}

CBCGPVisualManagerVS2005::~CBCGPVisualManagerVS2005()
{

}

void CBCGPVisualManagerVS2005::OnUpdateSystemColors ()
{
	BOOL bDefaultWinXPColors = m_bDefaultWinXPColors;

	m_clrPressedButtonBorder = (COLORREF)-1;

	m_CurrAppTheme = GetStandardWinXPTheme ();

	if (m_CurrAppTheme != WinXpTheme_Silver)
	{
		m_bDefaultWinXPColors = FALSE;
	}

	CBCGPVisualManager2003::OnUpdateSystemColors ();

	if (!bDefaultWinXPColors)
	{
		return;
	}

	COLORREF clrMenuButtonDroppedDown = m_clrBarBkgnd;
	COLORREF clrMenuItemCheckedHighlight = m_clrHighlightDn;

	if (m_hThemeComboBox == NULL ||
		m_pfGetThemeColor == NULL ||
		(*m_pfGetThemeColor) (m_hThemeComboBox, 5, 0, 3801, &m_colorActiveTabBorder) != S_OK)
	{
		m_colorActiveTabBorder = (COLORREF)-1;
	}

	if (globalData.m_nBitsPerPixel > 8 && !globalData.IsHighContastMode ())
	{
		m_clrCustomizeButtonGradientLight = CBCGPDrawManager::SmartMixColors (
			m_clrCustomizeButtonGradientDark,
			globalData.clrBarFace, 1.5, 1, 1);

		if (m_CurrAppTheme == WinXpTheme_Blue ||
			m_CurrAppTheme == WinXpTheme_Olive)
		{
			m_clrToolBarGradientDark = CBCGPDrawManager::PixelAlpha (
				m_clrToolBarGradientDark, 83);

			m_clrToolBarGradientLight = CBCGPDrawManager::SmartMixColors (
				GetBaseThemeColor (), 
				GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
				1., 3, 2);
		}
		else if (!m_bIsStandardWinXPTheme)
		{
			m_clrToolBarGradientLight = CBCGPDrawManager::SmartMixColors (
				m_clrToolBarGradientLight, 
				globalData.clrBarHilite,
				1.05, 1, 1);
		}

		if (m_CurrAppTheme == WinXpTheme_Blue)
		{
			m_clrCustomizeButtonGradientDark = CBCGPDrawManager::PixelAlpha (
				m_clrCustomizeButtonGradientDark, 90);

			m_clrCustomizeButtonGradientLight = CBCGPDrawManager::PixelAlpha (
				m_clrCustomizeButtonGradientLight, 115);

			m_clrToolBarBottomLine = CBCGPDrawManager::PixelAlpha (
				m_clrToolBarBottomLine, 85);
		}
		else if (m_CurrAppTheme == WinXpTheme_Olive)
		{
			m_clrToolBarBottomLine = CBCGPDrawManager::PixelAlpha (
				m_clrToolBarBottomLine, 110);

			m_clrCustomizeButtonGradientDark = m_clrToolBarBottomLine;

			m_clrCustomizeButtonGradientLight = CBCGPDrawManager::PixelAlpha (
				m_clrCustomizeButtonGradientLight, 120);

			m_clrHighlightDn = globalData.clrHilite;

			m_clrHighlight = CBCGPDrawManager::PixelAlpha (
				m_clrHighlightDn, 124);

			m_clrHighlightChecked = CBCGPDrawManager::PixelAlpha (
				GetThemeColor (m_hThemeWindow, 27 /*COLOR_GRADIENTACTIVECAPTION*/), 98);

			m_brHighlight.DeleteObject ();
			m_brHighlightDn.DeleteObject ();

			m_brHighlight.CreateSolidBrush (m_clrHighlight);
			m_brHighlightDn.CreateSolidBrush (m_clrHighlightDn);

			m_brHighlightChecked.DeleteObject ();
			m_brHighlightChecked.CreateSolidBrush (m_clrHighlightChecked);

			m_clrHighlightGradientDark = m_clrHighlightChecked;
			m_clrHighlightGradientLight = CBCGPDrawManager::PixelAlpha (
				m_clrHighlightGradientDark, 120);
		}
		else if (m_CurrAppTheme != WinXpTheme_Silver)
		{
			m_clrToolBarBottomLine = m_clrToolBarGradientDark;
		}

		clrMenuButtonDroppedDown = CBCGPDrawManager::PixelAlpha (
			m_clrBarBkgnd, 107);

		clrMenuItemCheckedHighlight = GetThemeColor (m_hThemeWindow, COLOR_HIGHLIGHT);

		if (m_CurrAppTheme == WinXpTheme_Blue ||
			m_CurrAppTheme == WinXpTheme_Olive)
		{
			m_clrBarGradientLight = CBCGPDrawManager::PixelAlpha (
					m_clrToolBarGradientLight, 95);

			m_clrBarGradientDark = CBCGPDrawManager::PixelAlpha (
				m_clrBarGradientDark, 97);
		}

		m_clrToolbarDisabled = CBCGPDrawManager::SmartMixColors (
			m_clrToolBarGradientDark, m_clrToolBarGradientLight, 
			.92, 1, 2);

		m_clrPressedButtonBorder = CBCGPDrawManager::SmartMixColors (
				m_clrMenuItemBorder, 
				globalData.clrBarDkShadow,
				.8, 1, 2);
	}

	m_brMenuButtonDroppedDown.DeleteObject ();
	m_brMenuButtonDroppedDown.CreateSolidBrush (clrMenuButtonDroppedDown);

	m_brMenuItemCheckedHighlight.DeleteObject ();
	m_brMenuItemCheckedHighlight.CreateSolidBrush (clrMenuItemCheckedHighlight);

	m_penActiveTabBorder.DeleteObject ();

	if (m_colorActiveTabBorder != (COLORREF)-1)
	{
		m_penActiveTabBorder.CreatePen (PS_SOLID, 1, m_colorActiveTabBorder);
	}

	m_bDefaultWinXPColors = bDefaultWinXPColors;

	m_clrInactiveTabText = globalData.clrBtnDkShadow;

	if (globalData.m_nBitsPerPixel > 8 && !globalData.IsHighContastMode ())
	{
		m_penSeparator.DeleteObject ();

		COLORREF clrSeparator = CBCGPDrawManager::PixelAlpha (
			globalData.clrBarFace, 84);

		m_penSeparator.CreatePen (PS_SOLID, 1, clrSeparator);
	}
}
//**************************************************************************************
COLORREF CBCGPVisualManagerVS2005::OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
			BOOL bActive, CRect rectCaption, CRect rectButtons)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnDrawControlBarCaption (pDC, pBar, 
			bActive, rectCaption, rectButtons);
	}

	rectCaption.bottom++;

	COLORREF clrFill;

	if (!bActive)
	{
		if (m_CurrAppTheme == WinXpTheme_Blue ||
			m_CurrAppTheme == WinXpTheme_Olive ||
			m_CurrAppTheme == WinXpTheme_Silver)
		{
			clrFill = CBCGPDrawManager::PixelAlpha (m_clrBarGradientDark, 87);

			CBrush brFill (clrFill);
			pDC->FillRect (rectCaption, &brFill);

			pDC->Draw3dRect (rectCaption, globalData.clrBarShadow, globalData.clrBarShadow);
		}
		else
		{
			CBrush brFill (globalData.clrInactiveCaption);
			pDC->FillRect (rectCaption, &brFill);
			return globalData.clrInactiveCaptionText;
		}
	}
	else
	{
		if (m_CurrAppTheme == WinXpTheme_Blue ||
			m_CurrAppTheme == WinXpTheme_Olive ||
			m_CurrAppTheme == WinXpTheme_Silver)
		{
			COLORREF clrLight = 
				CBCGPDrawManager::PixelAlpha (globalData.clrHilite, 130);

			CBCGPDrawManager dm (*pDC);
			dm.FillGradient (rectCaption, globalData.clrHilite, clrLight, TRUE);

			return globalData.clrTextHilite;
		}
		else
		{
			pDC->FillRect (rectCaption, &globalData.brActiveCaption);
			return globalData.clrCaptionText;
		}
	}

	if (GetRValue (clrFill) <= 192 &&
		GetGValue (clrFill) <= 192 &&
		GetBValue (clrFill) <= 192)
	{
		return RGB (255, 255, 255);
	}
	else
	{
		return RGB (0, 0, 0);
	}
}
//**************************************************************************************
void CBCGPVisualManagerVS2005::OnDrawCaptionButton (CDC* pDC, CBCGPCaptionButton* pButton, 
								BOOL bActive, BOOL bHorz, BOOL bMaximized, BOOL bDisabled,
								int nImageID /*= -1*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (bActive || pButton->IsMiniFrameButton () || globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnDrawCaptionButton (pDC, pButton, bActive, bHorz, bMaximized, bDisabled, nImageID);
		return;
	}

    CRect rc = pButton->GetRect ();

	const BOOL bHighlight = 
		(pButton->m_bPushed || pButton->m_bFocused || pButton->m_bDroppedDown) && !bDisabled;

	if (bHighlight)
	{
		pDC->FillRect (rc, &globalData.brBarFace);
	}

	CBCGPMenuImages::IMAGES_IDS id = (CBCGPMenuImages::IMAGES_IDS)-1;
	
	if (nImageID != -1)
	{
		id = (CBCGPMenuImages::IMAGES_IDS)nImageID;
	}
	else
	{
		id = pButton->GetIconID (bHorz, bMaximized);
	}

	if (id != (CBCGPMenuImages::IMAGES_IDS)-1)
	{
		CSize sizeImage = CBCGPMenuImages::Size ();
		CPoint ptImage (rc.left + (rc.Width () - sizeImage.cx) / 2,
						rc.top + (rc.Height () - sizeImage.cy) / 2);

		OnDrawCaptionButtonIcon (pDC, pButton, id, bActive, bDisabled, ptImage);
	}

	if (bHighlight)
	{
		pDC->Draw3dRect (rc, globalData.clrBarDkShadow, globalData.clrBarDkShadow);
	}
}
//***********************************************************************************
void CBCGPVisualManagerVS2005::OnEraseTabsArea (CDC* pDC, CRect rect, 
										 const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (pTabWnd->IsFlatTab () || globalData.m_nBitsPerPixel <= 8 || 
		globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnEraseTabsArea (pDC, rect, pTabWnd);
		return;
	}

	if (pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style ())
	{
		if (pTabWnd->IsDialogControl ())
		{
			if (pTabWnd->IsVisualManagerStyle ())
			{
				OnFillDialog (pDC, pTabWnd->GetParent (), rect);
			}
			else
			{
				pDC->FillRect (rect, &globalData.brBtnFace);
			}
		}
		else
		{
			pDC->FillRect (rect, &globalData.brBarFace);
		}
	}
	else
	{
		CBCGPBaseControlBar* pParentBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar,
			pTabWnd->GetParent ());
		if (pParentBar == NULL)
		{
			pDC->FillRect (rect, &globalData.brBtnFace);
		}
		else
		{
			CRect rectScreen = globalData.m_rectVirtual;
			pTabWnd->ScreenToClient (&rectScreen);

			CRect rectFill = rect;
			rectFill.left = min (rectFill.left, rectScreen.left);

			OnFillBarBackground (pDC, pParentBar, rectFill, rect);
		}
	}
}
//*************************************************************************************
void CBCGPVisualManagerVS2005::OnDrawTab (CDC* pDC, CRect rectTab,
						int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pTabWnd);
	ASSERT_VALID (pDC);

	if (pTabWnd->IsFlatTab () || pTabWnd->IsOneNoteStyle () ||
		pTabWnd->IsVS2005Style ())
	{
		CPen* pOldPen = NULL;

		if (bIsActive && pTabWnd->IsVS2005Style () &&
			m_penActiveTabBorder.GetSafeHandle () != NULL)
		{
			pOldPen = pDC->SelectObject (&m_penActiveTabBorder);
		}

		CBCGPVisualManager2003::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);

		if (pOldPen != NULL)
		{
			pDC->SelectObject (pOldPen);
		}

		return;
	}

	COLORREF clrTab = pTabWnd->GetTabBkColor (iTab);
	COLORREF clrTextOld = (COLORREF)-1;

	if (bIsActive && clrTab == (COLORREF)-1 && (!pTabWnd->IsPointerStyle() || globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ()))
	{
		clrTextOld = pDC->SetTextColor (globalData.clrWindowText);
		((CBCGPBaseTabWnd*)pTabWnd)->SetTabBkColor (iTab, globalData.clrWindow);
	}

	CBCGPVisualManagerXP::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);

	((CBCGPBaseTabWnd*)pTabWnd)->SetTabBkColor (iTab, clrTab);

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor (clrTextOld);
	}
}
//*********************************************************************************
int CBCGPVisualManagerVS2005::CreateAutoHideButtonRegion (CRect rect, 
								DWORD dwAlignment, LPPOINT& points)
{
	switch (dwAlignment & CBRS_ALIGN_ANY)
	{
	case CBRS_ALIGN_LEFT:
		rect.right--;
		break;

	case CBRS_ALIGN_TOP:
		rect.bottom--;
		break;
	}

	CRect rectOrign = rect;
	DWORD dwAlignmentOrign = dwAlignment;

	if ((dwAlignment & CBRS_ALIGN_ANY) == CBRS_ALIGN_LEFT || 
		(dwAlignment & CBRS_ALIGN_ANY) == CBRS_ALIGN_RIGHT)
	{
		rect = CRect (0, 0, rectOrign.Height (), rectOrign.Width ());
		dwAlignment = (dwAlignment == CBRS_ALIGN_LEFT) ? CBRS_ALIGN_TOP : CBRS_ALIGN_BOTTOM;
	}

	CList<POINT, POINT> pts;

	if (!m_bRoundedAutohideButtons)
	{
		rect.right--;

		pts.AddHead (CPoint (rect.left, rect.top));
		pts.AddHead (CPoint (rect.left, rect.bottom - 2));
		pts.AddHead (CPoint (rect.left + 2, rect.bottom));
		pts.AddHead (CPoint (rect.right - 2, rect.bottom));
		pts.AddHead (CPoint (rect.right, rect.bottom - 2));
		pts.AddHead (CPoint (rect.right, rect.top));
	}
	else
	{
		POSITION posLeft = pts.AddHead (CPoint (rect.left, rect.top));
		posLeft = pts.InsertAfter (posLeft, CPoint (rect.left, rect.top + 2));

		POSITION posRight = pts.AddTail (CPoint (rect.right, rect.top));
		posRight = pts.InsertBefore (posRight, CPoint (rect.right, rect.top + 2));

		int xLeft = rect.left + 1;
		int xRight = rect.right - 1;

		int y = 0;

		BOOL bIsHorz =
			(dwAlignmentOrign & CBRS_ALIGN_ANY) == CBRS_ALIGN_LEFT || 
			(dwAlignmentOrign & CBRS_ALIGN_ANY) == CBRS_ALIGN_RIGHT;

		for (y = rect.top + 2; y < rect.bottom - 4; y += 2)
		{
			posLeft = pts.InsertAfter (posLeft, CPoint (xLeft, y));
			posLeft = pts.InsertAfter (posLeft, CPoint (xLeft, y + 2));

			posRight = pts.InsertBefore (posRight, CPoint (xRight, y));
			posRight = pts.InsertBefore (posRight, CPoint (xRight, y + 2));

			xLeft++;
			xRight--;
		}

		if ((dwAlignmentOrign & CBRS_ALIGN_ANY) == CBRS_ALIGN_BOTTOM && !bIsHorz)
		{
			xLeft--;
			xRight++;
		}

		if (bIsHorz)
		{
			xRight++;
		}
	
		for (;y < rect.bottom - 1; y++)
		{
			posLeft = pts.InsertAfter (posLeft, CPoint (xLeft, y));
			posLeft = pts.InsertAfter (posLeft, CPoint (xLeft + 1, y + 1));

			posRight = pts.InsertBefore (posRight, CPoint (xRight, y));
			posRight = pts.InsertBefore (posRight, CPoint (xRight - 1, y + 1));

			if (y == rect.bottom - 2)
			{
				posLeft = pts.InsertAfter (posLeft, CPoint (xLeft + 1, y + 1));
				posLeft = pts.InsertAfter (posLeft, CPoint (xLeft + 3, y + 1));

				posRight = pts.InsertBefore (posRight, CPoint (xRight, y + 1));
				posRight = pts.InsertBefore (posRight, CPoint (xRight - 2, y + 1));
			}

			xLeft++;
			xRight--;
		}

		posLeft = pts.InsertAfter (posLeft, CPoint (xLeft + 2, rect.bottom));
		posRight = pts.InsertBefore (posRight, CPoint (xRight - 2, rect.bottom));
	}

	points = new POINT [pts.GetCount ()];

	int i = 0;

	for (POSITION pos = pts.GetHeadPosition (); pos != NULL; i++)
	{
		points [i] = pts.GetNext (pos);

		switch (dwAlignmentOrign & CBRS_ALIGN_ANY)
		{
		case CBRS_ALIGN_BOTTOM:
			points [i].y = rect.bottom - (points [i].y - rect.top);
			break;

		case CBRS_ALIGN_RIGHT:
			{
				int x = rectOrign.right - points [i].y;
				int y = rectOrign.top + points [i].x;

				points [i] = CPoint (x, y);
			}
			break;

		case CBRS_ALIGN_LEFT:
			{
				int x = rectOrign.left + points [i].y;
				int y = rectOrign.top + points [i].x;

				points [i] = CPoint (x, y);
			}
			break;
		}
	}

	return (int) pts.GetCount ();
}
//*********************************************************************************
void CBCGPVisualManagerVS2005::OnFillAutoHideButtonBackground (CDC* pDC, CRect rect, CBCGPAutoHideButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (!m_bRoundedAutohideButtons)
	{
		return;
	}

	LPPOINT points;
	int nPoints = CreateAutoHideButtonRegion (rect, pButton->GetAlignment (), points);

	CRgn rgnClip;
	rgnClip.CreatePolygonRgn (points, nPoints, WINDING);

	pDC->SelectClipRgn (&rgnClip);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManager2003::OnFillAutoHideButtonBackground (pDC, rect, pButton);
	}
	else
	{
		BOOL bIsHorz = 
			((pButton->GetAlignment () & CBRS_ALIGN_ANY) == CBRS_ALIGN_LEFT || 
			(pButton->GetAlignment () & CBRS_ALIGN_ANY) == CBRS_ALIGN_RIGHT);

		CBCGPDrawManager dm (*pDC);

		dm.FillGradient (rect,
			m_clrBarGradientDark, m_clrBarGradientLight, !bIsHorz);
	}

	pDC->SelectClipRgn (NULL);
	delete [] points;
}
//*********************************************************************************
void CBCGPVisualManagerVS2005::OnDrawAutoHideButtonBorder (CDC* pDC, CRect rect, CRect /*rectBorderSize*/, CBCGPAutoHideButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	CPen pen (PS_SOLID, 1, GetAutoHideButtonBorderColor(pButton));

	CPen* pOldPen = pDC->SelectObject (&pen);
	ASSERT (pOldPen != NULL);

	LPPOINT points;
	int nPoints = CreateAutoHideButtonRegion (rect, pButton->GetAlignment (), points);

	if (!m_bRoundedAutohideButtons)
	{
		pDC->Polyline (points, nPoints);
	}
	else
	{
		BOOL bIsHorz
			((pButton->GetAlignment () & CBRS_ALIGN_ANY) == CBRS_ALIGN_LEFT || 
			(pButton->GetAlignment () & CBRS_ALIGN_ANY) == CBRS_ALIGN_RIGHT);

		for (int i = 0; i < nPoints; i++)
		{
			if ((i % 2) != 0)
			{
				int x1 = points [i - 1].x;
				int y1 = points [i - 1].y;

				int x2 = points [i].x;
				int y2 = points [i].y;

				if (bIsHorz)
				{
					if (y1 > rect.CenterPoint ().y && y2 > rect.CenterPoint ().y)
					{
						y1--;
						y2--;
					}
				}
				else
				{
					if (x1 > rect.CenterPoint ().x && x2 > rect.CenterPoint ().x)
					{
						x1--;
						x2--;
					}
				}

				if (y2 >= y1)
				{
					pDC->MoveTo (x1, y1);
					pDC->LineTo (x2, y2);
				}
				else
				{
					pDC->MoveTo (x2, y2);
					pDC->LineTo (x1, y1);
				}
			}
		}
	}

	pDC->SelectObject (pOldPen);
	delete [] points;
}
//************************************************************************************
void CBCGPVisualManagerVS2005::GetTabFrameColors (const CBCGPBaseTabWnd* pTabWnd,
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
	
	CBCGPVisualManager2003::GetTabFrameColors (pTabWnd,
			   clrDark, clrBlack,
			   clrHighlight, clrFace,
			   clrDarkShadow, clrLight,
			   pbrFace, pbrBlack);

	if (pTabWnd->IsVS2005Style () && m_colorActiveTabBorder != (COLORREF)-1)
	{
		clrHighlight = m_colorActiveTabBorder;
	}

	clrBlack = clrDarkShadow;
}
//************************************************************************************
void CBCGPVisualManagerVS2005::OnDrawSeparator (CDC* pDC, CBCGPBaseControlBar* pBar,
										 CRect rect, BOOL bHorz)
{
	CBCGPToolBar* pToolBar = DYNAMIC_DOWNCAST (CBCGPToolBar, pBar);
	if (pToolBar != NULL)
	{
		ASSERT_VALID (pToolBar);

		if (bHorz)
		{
			const int nDelta = max (0, (pToolBar->GetButtonSize ().cy - pToolBar->GetImageSize ().cy) / 2);
			rect.top += nDelta;
		}
		else
		{
			const int nDelta = max (0, (pToolBar->GetButtonSize ().cx - pToolBar->GetImageSize ().cx) / 2);
			rect.left += nDelta;
		}
	}

	CBCGPVisualManagerXP::OnDrawSeparator (pDC, pBar, rect, bHorz);
}
//***********************************************************************************
void CBCGPVisualManagerVS2005::OnFillHighlightedArea (CDC* pDC, CRect rect, 
							CBrush* pBrush, CBCGPToolbarButton* pButton)
{
	if (pButton != NULL && 
		(m_CurrAppTheme == WinXpTheme_Blue || m_CurrAppTheme == WinXpTheme_Olive))
	{
		ASSERT_VALID (pButton);

		CBCGPToolbarMenuButton* pMenuButton = 
			DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);

		BOOL bIsPopupMenu = pMenuButton != NULL &&
			pMenuButton->GetParentWnd () != NULL &&
			pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar));

		if (bIsPopupMenu &&
			(pButton->m_nStyle & TBBS_CHECKED) &&
			pBrush == &m_brHighlightDn)
		{
			if (CBCGPToolBarImages::m_bIsDrawOnGlass)
			{
				CBCGPDrawManager dm (*pDC);
				dm.DrawRect (rect, m_clrBarBkgnd, (COLORREF)-1);
			}
			else
			{
				pDC->FillRect (rect, &m_brMenuItemCheckedHighlight);
			}
			return;
		}

		if (pMenuButton != NULL && !bIsPopupMenu && pMenuButton->IsDroppedDown ())
		{
			if (CBCGPToolBarImages::m_bIsDrawOnGlass)
			{
				CBCGPDrawManager dm (*pDC);
				dm.DrawRect (rect, m_clrBarBkgnd, (COLORREF)-1);
			}
			else
			{
				pDC->FillRect (rect, &m_brMenuButtonDroppedDown);
			}
			return;
		}
	}

	CBCGPVisualManager2003::OnFillHighlightedArea (pDC, rect, pBrush, pButton);
}
//***********************************************************************************
int CBCGPVisualManagerVS2005::GetDockingTabsBordersSize ()
{	
	return 
		CBCGPTabbedControlBar::m_StyleTabWnd == CBCGPTabWnd::STYLE_3D_ROUNDED ? 
			0 : 3;
}

#ifndef BCGP_EXCLUDE_PROP_LIST

COLORREF CBCGPVisualManagerVS2005::GetPropListGroupColor (CBCGPPropList* pPropList)
{
	ASSERT_VALID (pPropList);

	if (m_bDefaultWinXPColors)
	{
		return CBCGPVisualManager2003::GetPropListGroupColor (pPropList);
	}

	return pPropList->DrawControlBarColors () ?
		globalData.clrBarLight : globalData.clrBtnLight;
}

#endif // BCGP_EXCLUDE_PROP_LIST

COLORREF CBCGPVisualManagerVS2005::OnFillMiniFrameCaption (CDC* pDC, 
								CRect rectCaption, 
								CBCGPMiniFrameWnd* pFrameWnd,
								BOOL bActive)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pFrameWnd);

	if (DYNAMIC_DOWNCAST (CBCGPBaseToolBar, pFrameWnd->GetControlBar ()) == NULL)
	{
		return CBCGPVisualManager2003::OnFillMiniFrameCaption (pDC, 
								rectCaption, pFrameWnd, bActive);
	}

	if (DYNAMIC_DOWNCAST (CBCGPBaseToolBar, pFrameWnd->GetControlBar ()) != NULL)
	{
		pDC->FillRect (rectCaption, &m_brFloatToolBarBorder);
		return globalData.clrBarHilite;
	}

	::FillRect (pDC->GetSafeHdc (), rectCaption, ::GetSysColorBrush (COLOR_3DSHADOW));
	return globalData.clrCaptionText;
}

#ifndef BCGP_EXCLUDE_TASK_PANE

void CBCGPVisualManagerVS2005::OnDrawToolBoxFrame (CDC* pDC, const CRect& rect)
{
	ASSERT_VALID (pDC);
	pDC->Draw3dRect (rect, globalData.clrBarShadow, globalData.clrBarShadow);
}

#endif // BCGP_EXCLUDE_TASK_PANE
