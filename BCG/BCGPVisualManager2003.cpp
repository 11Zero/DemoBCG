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

// BCGPVisualManager2003.cpp: implementation of the CBCGPVisualManager2003 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPVisualManager2003.h"
#include "BCGPDrawManager.h"
#include "BCGPPopupMenuBar.h"
#include "BCGPMenuBar.h"
#include "bcgglobals.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPToolbarComboBoxButton.h"
#include "CustomizeButton.h"
#include "MenuImages.h"
#include "BCGPCaptionBar.h"
#include "BCGPBaseTabWnd.h"
#include "BCGPColorBar.h"
#include "BCGPCalculator.h"
#include "BCGPCalendarBar.h"
#include "BCGPTabWnd.h"
#include "BCGPTasksPane.h"
#include "BCGPStatusBar.h"
#include "BCGPAutoHideButton.h"
#include "BCGPHeaderCtrl.h"
#include "BCGPReBar.h"
#include "BCGPToolBox.h"
#include "BCGPPopupWindow.h"
#include "BCGPCalendarBar.h"
#include "BCGPDropDown.h"
#include "BCGPRibbonCategory.h"
#include "BCGPRibbonQuickAccessToolbar.h"
#include "BCGPRibbonBar.h"
#include "BCGPRibbonStatusBar.h"
#include "BCGPRibbonStatusBarPane.h"
#include "BCGPRibbonPanel.h"
#include "BCGPToolTipCtrl.h"
#include "BCGPDockBar.h"
#include "bcgpstyle.h"
#include "BCGPGanttChart.h"
#include "BCGPGridCtrl.h"
#include "BCGPPropList.h"

#ifndef BCGP_EXCLUDE_RIBBON
#include "BCGPRibbonProgressBar.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef CP_READONLY
#define CP_READONLY	5
#endif

IMPLEMENT_DYNCREATE(CBCGPVisualManager2003, CBCGPVisualManagerXP)

BOOL CBCGPVisualManager2003::m_bUseGlobalTheme = TRUE;
BOOL CBCGPVisualManager2003::m_bStatusBarOfficeXPLook = TRUE;
BOOL CBCGPVisualManager2003::m_bDefaultWinXPColors = TRUE;

static COLORREF CalculateHourLineColor (COLORREF clrBase, BOOL /*bWorkingHours*/, BOOL bHour)
{
	int nAlpha = bHour ? 85 : 95;
	return CBCGPDrawManager::PixelAlpha (clrBase, nAlpha);
}

static COLORREF CalculateWorkingColor (COLORREF clrBase)
{
	return clrBase;
}

static COLORREF CalculateNonWorkingColor (COLORREF clrBase)
{
	if (clrBase == RGB (255, 255, 255))
	{
		return globalData.clrBtnFace;
	}

	double H;
	double S;
	double L;

	CBCGPDrawManager::RGBtoHSL (clrBase, &H, &S, &L);
	
	return CBCGPDrawManager::HLStoRGB_ONE (
		S == 1 ? H * .84 : H,
		S == 1 ? L * .95 : L * .84,
		S);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPVisualManager2003::CBCGPVisualManager2003()
{
	m_WinXPTheme = WinXpTheme_None;

	m_bShadowHighlightedImage = FALSE;
	m_bFadeInactiveImage = FALSE;
	m_nMenuShadowDepth = 3;

	m_nVertMargin = 8;
	m_nHorzMargin = 8;
	m_nGroupVertOffset = 8;
	m_nGroupCaptionHeight = 18;
	m_nGroupCaptionHorzOffset = 3;
	m_nGroupCaptionVertOffset = 3;
	m_nTasksHorzOffset = 8;
	m_nTasksIconHorzOffset = 5;
	m_nTasksIconVertOffset = 4;
	m_bActiveCaptions = TRUE;
	m_bEmbossGripper = TRUE;
	m_bTabFillGradient = TRUE;
	
	OnUpdateSystemColors ();
}
//****************************************************************************************
CBCGPVisualManager2003::~CBCGPVisualManager2003()
{
}
//****************************************************************************************
void CBCGPVisualManager2003::DrawCustomizeButton (CDC* pDC, CRect rect, BOOL bIsHorz,
						  CBCGPVisualManager::BCGBUTTON_STATE state,
						  BOOL bIsCustomize, BOOL bIsMoreButtons)
{
	ASSERT_VALID (pDC);

    COLORREF clrDark = state == ButtonsIsRegular ?
		m_clrCustomizeButtonGradientDark : m_clrHighlightGradientDark;

	COLORREF clrLight = state == ButtonsIsRegular ?
		m_clrCustomizeButtonGradientLight : m_clrHighlightGradientLight;

	#define PTS_NUM 6
	POINT pts [PTS_NUM];

	if (bIsHorz)
	{
		pts [0] = CPoint (rect.left, rect.top);
		pts [1] = CPoint (rect.left + 2, rect.top + 1);
		pts [2] = CPoint (rect.left + 3, rect.bottom - 3);
		pts [3] = CPoint (rect.left, rect.bottom);
		pts [4] = CPoint (rect.right, rect.bottom);
		pts [5] = CPoint (rect.right, rect.top);
	}
	else
	{
		pts [0] = CPoint (rect.left, rect.top);
		pts [1] = CPoint (rect.left + 3, rect.top + 2);
		pts [2] = CPoint (rect.right - 3, rect.top + 3);
		pts [3] = CPoint (rect.right, rect.top);
		pts [4] = CPoint (rect.right, rect.bottom);
		pts [5] = CPoint (rect.left, rect.bottom);
	}

	CRgn rgnClip;
	rgnClip.CreatePolygonRgn (pts, PTS_NUM, WINDING);

	pDC->SelectClipRgn (&rgnClip);

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, clrDark, clrLight, bIsHorz);

	//---------------------
	// Draw button content:
	//---------------------
	const int nEllipse = 2;

	if (bIsHorz)
	{
		rect.DeflateRect (0, nEllipse);
		rect.left += nEllipse;
	}
	else
	{
		rect.DeflateRect (nEllipse, 0);
		rect.top += nEllipse;
	}

	const int nMargin = GetToolBarCustomizeButtonMargin ();

	CSize sizeImage = CBCGPMenuImages::Size ();
	if (CBCGPToolBar::IsLargeIcons ())
	{
		sizeImage.cx *= 2;
		sizeImage.cy *= 2;
	}

	if (bIsCustomize)
	{
		//-----------------
		// Draw menu image:
		//-----------------
		CRect rectMenu = rect;
		if (bIsHorz)
		{
			rectMenu.top = rectMenu.bottom - sizeImage.cy - 2 * nMargin;
		}
		else
		{
			rectMenu.top++;
			rectMenu.left = rectMenu.right - sizeImage.cx - 2 * nMargin;
		}

		rectMenu.DeflateRect (
			(rectMenu.Width () - sizeImage.cx) / 2,
			(rectMenu.Height () - sizeImage.cy) / 2);

		rectMenu.OffsetRect (1, 1);

		CBCGPMenuImages::IMAGES_IDS id = bIsHorz ? 
			CBCGPMenuImages::IdCustomizeArowDown : CBCGPMenuImages::IdCustomizeArowLeft;

		CBCGPMenuImages::Draw (	pDC, id, rectMenu.TopLeft (),
			CBCGPMenuImages::ImageWhite, sizeImage);

		rectMenu.OffsetRect (-1, -1);

		CBCGPMenuImages::Draw (	pDC, id, rectMenu.TopLeft (), CBCGPMenuImages::ImageBlack, sizeImage);
	}

	if (bIsMoreButtons)
	{
		//-------------------
		// Draw "more" image:
		//-------------------
		CRect rectMore = rect;
		if (bIsHorz)
		{
			rectMore.bottom = rectMore.top + sizeImage.cy + 2 * nMargin;
		}
		else
		{
			rectMore.right = rectMore.left + sizeImage.cx + 2 * nMargin;
			rectMore.top++;
		}

		rectMore.DeflateRect (
			(rectMore.Width () - sizeImage.cx) / 2,
			(rectMore.Height () - sizeImage.cy) / 2);

		CBCGPMenuImages::IMAGES_IDS id = 
			bIsHorz ? 
				CBCGPMenuImages::IdCustomizeMoreButtonsHorz : 
				CBCGPMenuImages::IdCustomizeMoreButtonsVert;

		rectMore.OffsetRect (1, 1);
		CBCGPMenuImages::Draw (pDC, id, rectMore.TopLeft (), CBCGPMenuImages::ImageWhite, sizeImage);

		rectMore.OffsetRect (-1, -1);
		CBCGPMenuImages::Draw (pDC, id, rectMore.TopLeft (), CBCGPMenuImages::ImageBlack, sizeImage);
	}

	pDC->SelectClipRgn (NULL);
}
//***********************************************************************************
BOOL CBCGPVisualManager2003::IsToolbarRoundShape (CBCGPToolBar* pToolBar)
{
	ASSERT_VALID (pToolBar);
	return !pToolBar->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar));
}
//***********************************************************************************
void CBCGPVisualManager2003::OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
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

	CRuntimeClass* pBarClass = pBar->GetRuntimeClass ();

	if (globalData.m_nBitsPerPixel <= 8 ||
		globalData.IsHighContastMode () ||
		pBar->IsDialogControl () ||
		pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPColorBar)) ||
		pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPCalculator)) ||
		pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPCalendarBar)))
	{
		CBCGPVisualManagerXP::OnFillBarBackground (pDC, pBar, rectClient, rectClip);
		return;
	}

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPStatusBar)))
	{
		if (m_bStatusBarOfficeXPLook && m_hThemeStatusBar != NULL)
		{
			(*m_pfDrawThemeBackground) (m_hThemeStatusBar, 
				pDC->GetSafeHdc (),
				0, 0, &rectClient, 0);
			return;
		}
	}

#ifndef BCGP_EXCLUDE_RIBBON
	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPRibbonStatusBar)))
	{
		if (m_hThemeStatusBar != NULL)
		{
			(*m_pfDrawThemeBackground) (m_hThemeStatusBar, 
				pDC->GetSafeHdc (),
				0, 0, &rectClient, 0);
			return;
		}
	}
#endif	// BCGP_EXCLUDE_RIBBON

	if (rectClip.IsRectEmpty ())
	{
		rectClip = rectClient;
	}

	CBCGPDrawManager dm (*pDC);

    if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPCaptionBar)))
	{
		CBCGPCaptionBar* pCaptionBar = (CBCGPCaptionBar*) pBar;

		if (pCaptionBar->IsMessageBarMode ())
		{
			dm.FillGradient (rectClient, 
				m_clrBarGradientDark, m_clrBarGradientLight, FALSE);
		}
		else
		{
			if (pCaptionBar->m_clrBarBackground == -1)
			{
				dm.FillGradient (rectClient, 
					m_clrCaptionBarGradientDark, m_clrCaptionBarGradientLight, TRUE);
			}
			else
			{
				COLORREF clrLight = CBCGPDrawManager::PixelAlpha(pCaptionBar->m_clrBarBackground, 150);

				dm.FillGradient (rectClient, 
					pCaptionBar->m_clrBarBackground, clrLight, TRUE);
			}
		}
		return;
	}

    if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar)))
	{
		pDC->FillRect (rectClip, &m_brMenuLight);

		CBCGPPopupMenuBar* pMenuBar = DYNAMIC_DOWNCAST (CBCGPPopupMenuBar, pBar);
		if (!pMenuBar->m_bDisableSideBarInXPMode)
		{
			CRect rectImages = rectClient;

			rectImages.right = rectImages.left + pMenuBar->GetGutterWidth ();

			if (!pMenuBar->HasGutterLogo())
			{
				rectImages.DeflateRect (0, 1);
			}

			dm.FillGradient (rectImages, m_clrToolBarGradientLight, m_clrToolBarGradientDark, FALSE,
				35);
		}

		return;
	}

	BOOL bIsHorz = (pBar->GetBarStyle () & CBRS_ORIENT_HORZ);
	BOOL bIsToolBar = pBar->IsKindOf (RUNTIME_CLASS (CBCGPToolBar)) &&
						!pBar->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar));

#if !defined(BCGP_EXCLUDE_TOOLBOX) && !defined(BCGP_EXCLUDE_TASK_PANE)
	BOOL bIsToolBox =	pBar->IsKindOf (RUNTIME_CLASS (CBCGPToolBoxPage)) ||
						pBar->IsKindOf (RUNTIME_CLASS (CBCGPToolBox)) ||
						pBar->IsKindOf (RUNTIME_CLASS (CBCGPToolBoxEx));
#else
	BOOL bIsToolBox = FALSE;
#endif

	COLORREF clr1 = bIsHorz ? m_clrToolBarGradientDark : m_clrToolBarGradientVertLight;
	COLORREF clr2 = bIsHorz ? m_clrToolBarGradientLight : m_clrToolBarGradientVertDark;

	if (bIsToolBox)
	{
		clr1 = m_clrBarGradientLight;
		clr2 = m_clrBarGradientDark;

		bIsHorz = FALSE;
	}

	if (!bIsToolBar && !bIsToolBox)
	{
		bIsHorz = FALSE;

		clr1 = m_clrBarGradientDark;
		clr2 = m_clrBarGradientLight;

		rectClient.right = rectClient.left + globalData.m_rectVirtual.Width () + 10;
	}

	const int nStartFlatPercentage = bIsToolBar ? 25 : 0;

	BOOL bRoundedCorners = FALSE;

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPDropDownToolBar)))
	{
		bNCArea = FALSE;
	}

	CBCGPToolBar* pToolBar = DYNAMIC_DOWNCAST (CBCGPToolBar, pBar);
	if (bNCArea && pToolBar != NULL && pToolBar->IsDocked () &&
		pToolBar->GetParentDockBar () != NULL &&
		!pToolBar->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar)))
	{
		bRoundedCorners = TRUE;

		CBCGPBaseControlBar* pParentBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar,
			pBar->GetParent ());

		if (pParentBar != NULL)
		{
			CPoint pt (0, 0);
			pBar->MapWindowPoints (pParentBar, &pt, 1);
			pt = pDC->OffsetWindowOrg (pt.x, pt.y);

			CRect rectParent;
			pParentBar->GetClientRect (rectParent);

			OnFillBarBackground (pDC, pParentBar, rectParent, rectParent);

			pDC->SetWindowOrg(pt.x, pt.y);
		}

		CRect rectFill = rectClient;
		rectFill.DeflateRect (1, 0);

		dm.FillGradient (rectFill, clr1, clr2, bIsHorz, nStartFlatPercentage);

		CRect rectLeft = rectClient;
		rectLeft.top ++;
		rectLeft.right = rectLeft.left + 1;

		dm.FillGradient (rectLeft, clr1, clr2, bIsHorz);

		CRect rectRight = rectClient;
		rectLeft.top ++;
		rectRight.left = rectRight.right - 1;

		dm.FillGradient (rectRight, clr1, clr2, bIsHorz);
	}
	else
	{
		CRect rectFill = rectClient;

		if (bIsToolBar && pBar->IsDocked () && pToolBar->GetParentDockBar () != NULL)
		{
			ASSERT_VALID (pToolBar);

			rectFill.left -= pToolBar->m_cxLeftBorder;
			rectFill.right += pToolBar->m_cxRightBorder;
			rectFill.top -= pToolBar->m_cyTopBorder;
			rectFill.bottom += pToolBar->m_cyBottomBorder;
		}

		if (pBar->GetCurrentAlignment () == CBRS_ALIGN_RIGHT &&
			clr1 != clr2)
		{
			if (DYNAMIC_DOWNCAST(CBCGPDockBar, pBar) != NULL ||
				DYNAMIC_DOWNCAST(CBCGPAutoHideToolBar, pBar) != NULL)
			{
				CFrameWnd* pMainFrame = BCGCBProGetTopLevelFrame (pBar);
				if (pMainFrame->GetSafeHwnd () != NULL)
				{
					CRect rectMain;
					pMainFrame->GetClientRect (rectMain);
					pMainFrame->MapWindowPoints (pBar, &rectMain);

					rectFill.top = rectMain.top;
					rectFill.left = rectMain.left;
					rectFill.right = rectFill.left + globalData.m_rectVirtual.Width () + 10;
				}
			}
		}

		dm.FillGradient (rectFill, clr1, clr2, bIsHorz, nStartFlatPercentage);
	}

	if (bNCArea)
	{
		CCustomizeButton* pCustomizeButton = NULL;
		
		CRect rectCustomizeBtn;
		rectCustomizeBtn.SetRectEmpty ();

		if (pToolBar != NULL && pToolBar->GetCount () > 0)
		{
			pCustomizeButton = DYNAMIC_DOWNCAST (CCustomizeButton, 
				pToolBar->GetButton (pToolBar->GetCount () - 1));

			if (pCustomizeButton != NULL)
			{
				rectCustomizeBtn = pCustomizeButton->Rect ();
			}
		}

		if (bRoundedCorners)
		{
			//------------------------
			// Draw bottom/right edge:
			//------------------------
			CPen* pOldPen = pDC->SelectObject (&m_penBottomLine);
			ASSERT (pOldPen != NULL);

			if (bIsHorz)
			{
				pDC->MoveTo (rectClient.left + 2, rectClient.bottom - 1);
				pDC->LineTo (rectClient.right - rectCustomizeBtn.Width (), rectClient.bottom - 1);
			}
			else
			{
				pDC->MoveTo (rectClient.right - 1, rectClient.top + 2);
				pDC->LineTo (rectClient.right - 1, rectClient.bottom - 2 - rectCustomizeBtn.Height ());
			}

			pDC->SelectObject (pOldPen);
		}

		if (pToolBar != NULL && pToolBar->GetCount () > 0)
		{
			if (pCustomizeButton != NULL && !rectCustomizeBtn.IsRectEmpty () &&
				pCustomizeButton->IsPipeStyle ())
			{
				BOOL bIsRTL = pBar->GetExStyle() & WS_EX_LAYOUTRTL;

				//----------------------------------------
				// Special drawing for "Customize" button:
				//----------------------------------------
				CRect rectWindow;
				pBar->GetWindowRect (rectWindow);

				pBar->ClientToScreen (&rectCustomizeBtn);

				CRect rectButton = rectClient;

				if (pToolBar->IsHorizontal ())
				{
					if (bIsRTL)
					{
						int nButtonWidth = rectCustomizeBtn.Width ();

						nButtonWidth -= 
							rectWindow.left - rectCustomizeBtn.left;
						rectButton.left = rectButton.right - nButtonWidth;
					}
					else
					{
						rectButton.left = rectButton.right - rectCustomizeBtn.Width () - 
							rectWindow.right + rectCustomizeBtn.right;
					}

					pCustomizeButton->SetExtraSize (
						0,
						rectWindow.bottom - rectCustomizeBtn.bottom);
				}
				else
				{
					rectButton.top = rectButton.bottom - rectCustomizeBtn.Height () - 
						rectWindow.bottom + rectCustomizeBtn.bottom;
					pCustomizeButton->SetExtraSize (
						rectWindow.right - rectCustomizeBtn.right,
						0);
				}

				BCGBUTTON_STATE state = ButtonsIsRegular;

				if (pToolBar->IsButtonHighlighted (pToolBar->GetCount () - 1) ||
					pCustomizeButton->IsDroppedDown ())
				{
					state = ButtonsIsHighlighted;
				}
				else if (pCustomizeButton->m_nStyle & (TBBS_PRESSED | TBBS_CHECKED))
				{
					//-----------------------
					// Pressed in or checked:
					//-----------------------
					state = ButtonsIsPressed;
				}

				DrawCustomizeButton (pDC, rectButton,
					pToolBar->IsHorizontal (), state,
					(int) pCustomizeButton->GetCustomizeCmdId () > 0,
					!pCustomizeButton->GetInvisibleButtons ().IsEmpty ());
			}
		}
	}
}
//****************************************************************************************
void CBCGPVisualManager2003::OnDrawBarBorder (CDC* pDC, CBCGPBaseControlBar* pBar, CRect& rect)
{
	ASSERT_VALID (pBar);

	if (pBar->IsDialogControl () || globalData.m_nBitsPerPixel <= 8 || 
		globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnDrawBarBorder (pDC, pBar, rect);
	}
}
//***************************************************************************************
void CBCGPVisualManager2003::OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz,
									   CBCGPBaseControlBar* pBar)
{
	ASSERT_VALID (pDC);

	if (pBar != NULL && pBar->IsDialogControl () ||
		globalData.m_nBitsPerPixel <= 8)
	{
		CBCGPVisualManagerXP::OnDrawBarGripper (pDC, rectGripper, bHorz, pBar);
		return;
	}

	const int nBoxSize = 4;

	if (bHorz)
	{
		rectGripper.left = rectGripper.right - nBoxSize;
	}
	else
	{
		rectGripper.top = rectGripper.bottom - nBoxSize;
	}

	CBCGPToolBar* pToolBar = DYNAMIC_DOWNCAST (CBCGPToolBar, pBar);
	if (pToolBar != NULL)
	{
		if (bHorz)
		{
			const int nHeight = CBCGPToolBar::IsLargeIcons () ? 
				pToolBar->GetRowHeight () : pToolBar->GetButtonSize ().cy;

			const int nDelta = max (0, (nHeight - pToolBar->GetImageSize ().cy) / 2);
			rectGripper.DeflateRect (0, nDelta);
		}
		else
		{
			const int nWidth = CBCGPToolBar::IsLargeIcons () ? 
				pToolBar->GetColumnWidth () : pToolBar->GetButtonSize ().cx;

			const int nDelta = max (0, (nWidth - pToolBar->GetImageSize ().cx) / 2);
			rectGripper.DeflateRect (nDelta, 0);
		}
	}

	const int nBoxesNumber = bHorz ?
		(rectGripper.Height () - nBoxSize) / nBoxSize : 
		(rectGripper.Width () - nBoxSize) / nBoxSize;

	int nOffset = bHorz ? 
		(rectGripper.Height () - nBoxesNumber * nBoxSize) / 2 :
		(rectGripper.Width () - nBoxesNumber * nBoxSize) / 2;

	for (int nBox = 0; nBox < nBoxesNumber; nBox++)
	{
		int x = bHorz ? 
			rectGripper.left : 
			rectGripper.left + nOffset;

		int y = bHorz ? 
			rectGripper.top + nOffset : 
			rectGripper.top;

		if (m_bEmbossGripper)
		{
			pDC->FillSolidRect (x + 1, y + 1, nBoxSize / 2, nBoxSize / 2, 
				globalData.clrBtnHilite);
		}

		pDC->FillSolidRect (x, y, nBoxSize / 2, nBoxSize / 2, 
			m_clrGripper);

		nOffset += nBoxSize;
	}
}
//**************************************************************************************
void CBCGPVisualManager2003::OnDrawComboBorder (CDC* pDC, CRect rect,
												BOOL bDisabled,
												BOOL bIsDropped,
												BOOL bIsHighlighted,
												CBCGPToolbarComboBoxButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnDrawComboBorder (pDC, rect,
												bDisabled,
												bIsDropped,
												bIsHighlighted,
												pButton);
		return;
	}

	if (bIsHighlighted || bIsDropped || bDisabled)
	{
		rect.DeflateRect (1, 1);

		COLORREF colorBorder = bDisabled ? globalData.clrBtnShadow : m_clrMenuItemBorder;

		if (CBCGPToolBarImages::m_bIsDrawOnGlass)
		{
			CBCGPDrawManager dm (*pDC);
			dm.DrawRect (rect, (COLORREF)-1, colorBorder);
		}
		else
		{
			pDC->Draw3dRect (&rect, colorBorder, colorBorder);
		}
	}
}
//*********************************************************************************
void CBCGPVisualManager2003::OnFillOutlookPageButton (CDC* pDC, const CRect& rect,
										BOOL bIsHighlighted, BOOL bIsPressed,
										COLORREF& clrText)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnFillOutlookPageButton (pDC, rect,
										bIsHighlighted, bIsPressed,
										clrText);
		return;
	}

	ASSERT_VALID (pDC);

	COLORREF clr1 = m_clrBarGradientDark;
	COLORREF clr2 = m_clrBarGradientLight;

	if (bIsPressed)
	{
		if (bIsHighlighted)
		{
			clr1 = m_clrHighlightDnGradientDark;
			clr2 = m_clrHighlightDnGradientLight;
		}
		else
		{
			clr1 = m_clrHighlightDnGradientLight;
			clr2 = m_clrHighlightDnGradientDark;
		}
	}
	else if (bIsHighlighted)
	{
		clr1 = m_clrHighlightGradientDark;
		clr2 = m_clrHighlightGradientLight;
	}

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, clr1, clr2, TRUE);

	clrText = globalData.clrBtnText;
}
//****************************************************************************************
void CBCGPVisualManager2003::OnDrawOutlookPageButtonBorder (CDC* pDC, 
							CRect& rectBtn, BOOL bIsHighlighted, BOOL bIsPressed)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnDrawOutlookPageButtonBorder (pDC, 
							rectBtn, bIsHighlighted, bIsPressed);
		return;
	}

	ASSERT_VALID (pDC);

	pDC->Draw3dRect (rectBtn, globalData.clrBtnHilite, m_clrGripper);
}
//**********************************************************************************
void CBCGPVisualManager2003::OnFillButtonInterior (CDC* pDC,
	CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state)
{

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	CCustomizeButton* pCustButton = DYNAMIC_DOWNCAST (CCustomizeButton, pButton);
	if (pCustButton == NULL || !pCustButton->IsPipeStyle () || globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnFillButtonInterior (pDC, pButton, rect, state);
		return;
	}

	CBCGPToolBar* pToolBar = pCustButton->GetParentToolbar ();
	if (pToolBar != NULL)
	{
		ASSERT_VALID (pToolBar);

		CRect rectToolbar;
		pToolBar->GetWindowRect (rectToolbar);
		pToolBar->ScreenToClient (rectToolbar);

		if (pToolBar->IsHorizontal ())
		{
			rect.right = rectToolbar.right;
		}
		else
		{
			rect.bottom = rectToolbar.bottom;
		}
	}

	CSize sizeExtra = pCustButton->GetExtraSize ();

	rect.InflateRect (sizeExtra);
	DrawCustomizeButton (pDC, rect, pToolBar->IsHorizontal (), state,
		(int) pCustButton->GetCustomizeCmdId () > 0,
		!pCustButton->GetInvisibleButtons ().IsEmpty ());

	pCustButton->SetDefaultDraw (FALSE);
}
//**************************************************************************************
void CBCGPVisualManager2003::OnDrawButtonBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, BCGBUTTON_STATE state)
{
	CCustomizeButton* pCustButton = DYNAMIC_DOWNCAST (CCustomizeButton, pButton);
	if (pCustButton == NULL || !pCustButton->IsPipeStyle () || globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnDrawButtonBorder (pDC, pButton, rect, state);
	}

	// Do nothing - the border is already painted in OnFillButtonInterior
}
//**************************************************************************************
void CBCGPVisualManager2003::OnDrawSeparator (CDC* pDC, CBCGPBaseControlBar* pBar,
										 CRect rect, BOOL bHorz)
{
	ASSERT_VALID (pBar);

	if (pBar->IsDialogControl () ||
		pBar->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar)) ||
		globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnDrawSeparator (pDC, pBar, rect, bHorz);
		return;
	}

#ifndef BCGP_EXCLUDE_RIBBON
	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPRibbonStatusBar)))
	{
		if (m_hThemeStatusBar == NULL)
		{
			CBCGPVisualManagerXP::OnDrawSeparator (pDC, pBar, rect, bHorz);
			return;
		}

		rect.InflateRect (1, 5);

		(*m_pfDrawThemeBackground) (m_hThemeStatusBar, pDC->GetSafeHdc(), 1 /*SP_PANE*/,
				0, &rect, 0);
		return;
	}

#endif
	CBCGPToolBar* pToolBar = DYNAMIC_DOWNCAST (CBCGPToolBar, pBar);
	if (pToolBar == NULL)
	{
		CBCGPVisualManagerXP::OnDrawSeparator (pDC, pBar, rect, bHorz);
		return;
	}

	CPen* pOldPen = pDC->SelectObject (&m_penSeparator);
	ASSERT (pOldPen != NULL);

	if (bHorz)
	{
		const int nDelta = max (0, (pToolBar->GetButtonSize ().cy - pToolBar->GetImageSize ().cy) / 2);
		rect.DeflateRect (0, nDelta);

		int x = rect.left += rect.Width () / 2 - 1;

		if (CBCGPToolBarImages::m_bIsDrawOnGlass)
		{
			CBCGPDrawManager dm (*pDC);

			dm.DrawLine (x, rect.top, x, rect.bottom - 1, m_clrSeparator);
			dm.DrawLine (x + 1, rect.top + 1, x + 1, rect.bottom, globalData.clrBarHilite);
		}
		else
		{
			pDC->MoveTo (x, rect.top);
			pDC->LineTo (x, rect.bottom - 1);

			pDC->SelectObject (&m_penSeparatorLight);

			pDC->MoveTo (x + 1, rect.top + 1);
			pDC->LineTo (x + 1, rect.bottom);
		}
	}
	else
	{
		const int nDelta = max (0, (pToolBar->GetButtonSize ().cx - pToolBar->GetImageSize ().cx) / 2);
		rect.DeflateRect (nDelta, 0);

		int y = rect.top += rect.Height () / 2 - 1;

		if (CBCGPToolBarImages::m_bIsDrawOnGlass)
		{
			CBCGPDrawManager dm (*pDC);

			dm.DrawLine (rect.left, y, rect.right - 1, y, m_clrSeparator);
			dm.DrawLine (rect.left + 1, y + 1, rect.right, y + 1, globalData.clrBarHilite);
		}
		else
		{
			pDC->MoveTo (rect.left, y);
			pDC->LineTo (rect.right - 1, y);

			pDC->SelectObject (&m_penSeparatorLight);

			pDC->MoveTo (rect.left + 1, y + 1);
			pDC->LineTo (rect.right, y + 1);
		}
	}

	pDC->SelectObject (pOldPen);
}
//***********************************************************************************
void CBCGPVisualManager2003::OnUpdateSystemColors ()
{
	CBCGPWinXPThemeManager::UpdateSystemColors ();

	BOOL bIsAppThemed = m_bUseGlobalTheme || (m_pfGetWindowTheme != NULL && 
		(*m_pfGetWindowTheme) (AfxGetMainWnd ()->GetSafeHwnd ()) != NULL);

	m_WinXPTheme = bIsAppThemed ? GetStandardWinXPTheme () : WinXpTheme_None;

	if (!m_bDefaultWinXPColors && m_WinXPTheme != WinXpTheme_None)
	{
		m_WinXPTheme = WinXpTheme_NonStandard;
	}

	m_bIsStandardWinXPTheme = 
		m_WinXPTheme == WinXpTheme_Blue ||
		m_WinXPTheme == WinXpTheme_Olive ||
		m_WinXPTheme == WinXpTheme_Silver;

	//----------------------
	// Modify global colors:
	//----------------------
	ModifyGlobalColors ();

	CBCGPVisualManagerXP::OnUpdateSystemColors ();

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		m_clrTaskPaneGradientDark  = globalData.clrWindow;
		m_clrTaskPaneGradientLight  = globalData.clrWindow;
		m_clrTaskPaneGroupCaptionDark  = globalData.clrBarFace;
		m_clrTaskPaneGroupCaptionLight  = globalData.clrBarFace;
		m_clrTaskPaneGroupCaptionSpecDark  = globalData.clrBarFace;
		m_clrTaskPaneGroupCaptionSpecLight  = globalData.clrBarFace;
		m_clrTaskPaneGroupAreaLight  = globalData.clrWindow;
		m_clrTaskPaneGroupAreaDark  = globalData.clrWindow;
		m_clrTaskPaneGroupAreaSpecLight  = globalData.clrWindow;
		m_clrTaskPaneGroupAreaSpecDark  = globalData.clrWindow;
		m_clrTaskPaneGroupBorder  = globalData.clrBtnShadow;

		m_clrBarGradientLight = m_clrToolBarGradientLight = globalData.clrBarLight;

		m_penTaskPaneGroupBorder.DeleteObject ();
		m_penTaskPaneGroupBorder.CreatePen (PS_SOLID, 1, m_clrTaskPaneGroupBorder);

		m_penGridExpandBoxLight.DeleteObject ();
		m_penGridExpandBoxLight.CreatePen (PS_SOLID, 1, globalData.clrBtnLight);

		m_penGridExpandBoxDark.DeleteObject ();
		m_penGridExpandBoxDark.CreatePen (PS_SOLID, 1, globalData.clrBtnShadow);

		m_clrToolbarDisabled = globalData.clrBtnHilite;
		return;
	}

	//--------------------------------------------------
	// Calculate control bars bakground gradient colors:
	//--------------------------------------------------
	COLORREF clrBase = GetBaseThemeColor ();

	if (m_WinXPTheme == WinXpTheme_Olive)
	{
		m_clrToolBarGradientDark = CBCGPDrawManager::PixelAlpha (
			clrBase, 120);

		m_clrBarGradientDark = CBCGPDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_3DFACE),
			.87, 1, 3);

		m_clrToolBarGradientLight = CBCGPDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
			1., 2, 1);

		m_clrBarGradientLight = CBCGPDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
			1.03);
	}
	else if (m_WinXPTheme == WinXpTheme_Silver)
	{
		m_clrToolBarGradientDark = CBCGPDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_3DFACE),
			0.75, 2);

		m_clrBarGradientDark = CBCGPDrawManager::PixelAlpha (
			clrBase, 120);

		m_clrToolBarGradientLight = CBCGPDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_3DHIGHLIGHT),
			.98);

		m_clrBarGradientLight = CBCGPDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
			1.03);
	}
	else if (m_WinXPTheme == WinXpTheme_Blue)
	{
		m_clrToolBarGradientDark = CBCGPDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_3DFACE),
			0.93, 2);


		m_clrBarGradientDark = CBCGPDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_3DLIGHT),
			.99, 2, 1);

		m_clrToolBarGradientLight = CBCGPDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
			1.03);

		m_clrBarGradientLight = m_clrToolBarGradientLight;
	}
	else
	{
		m_clrToolBarGradientDark = CBCGPDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_3DFACE),
			0.93, 2);

		m_clrBarGradientDark = CBCGPDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_3DLIGHT),
			.99, 2, 1);

		m_clrToolBarGradientLight = CBCGPDrawManager::SmartMixColors (
			clrBase, 
			GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
			1., 1, 4);

		m_clrBarGradientLight = m_clrToolBarGradientLight;
	}

	m_clrToolBarGradientVertLight = m_clrToolBarGradientLight;

	m_clrToolBarGradientVertDark = CBCGPDrawManager::PixelAlpha (
			m_clrToolBarGradientDark, 98);

	COLORREF clrSeparatorDark;
	COLORREF clrPlannerTodayLine;

	//-------------------------------------
	// Calculate highlight gradient colors:
	//-------------------------------------
	if (m_bIsStandardWinXPTheme)
	{
		ASSERT (m_pfGetThemeColor != NULL);

		COLORREF clr1, clr2, clr3;

		if (m_WinXPTheme == WinXpTheme_Blue && globalData.bIsWindowsVista)
		{
			clr1 = RGB (250, 196, 88);
			clr2 = RGB (250, 196, 88);
			clr3 = RGB (228, 93, 61);
		}
		else
		{
			(*m_pfGetThemeColor) (m_hThemeButton, BP_PUSHBUTTON, 0, TMT_ACCENTCOLORHINT, &clr1);
			(*m_pfGetThemeColor) (m_hThemeButton, BP_RADIOBUTTON, 0, TMT_ACCENTCOLORHINT, &clr2);
			(*m_pfGetThemeColor) (m_hThemeWindow, WP_CLOSEBUTTON, 0, TMT_FILLCOLORHINT, &clr3);
		}

		m_clrHighlightMenuItem = CBCGPDrawManager::SmartMixColors (
			clr1, 
			clr2,
			1.3, 1, 1);

		m_clrHighlightGradientLight = CBCGPDrawManager::SmartMixColors (
			clr1, 
			clr3,
			1.55, 2, 1);

		m_clrHighlightGradientDark = CBCGPDrawManager::SmartMixColors (
			clr1, 
			clr2,
			1.03, 2, 1);

		m_clrHighlightDnGradientLight = CBCGPDrawManager::SmartMixColors (
			clr1, 
			clr3,
			1.03, 1, 2);

		m_brFloatToolBarBorder.DeleteObject ();

		COLORREF clrCustom;
		(*m_pfGetThemeColor) (m_hThemeButton, BP_RADIOBUTTON, 0, TMT_BORDERCOLORHINT, &clrCustom);

		COLORREF clrToolbarBorder = CBCGPDrawManager::SmartMixColors (
			clrCustom, 
			clrBase,
			0.84, 1, 4);
		m_brFloatToolBarBorder.CreateSolidBrush (clrToolbarBorder);

		if (m_WinXPTheme == WinXpTheme_Blue || m_WinXPTheme == WinXpTheme_Silver)
		{
			m_clrCustomizeButtonGradientDark = globalData.bIsWindowsVista ? RGB (7, 57, 142) :
				CBCGPDrawManager::PixelAlpha (globalData.clrActiveCaption, 60);

			const double k = (m_WinXPTheme == WinXpTheme_Blue) ? 1.61 : 1;

			m_clrCustomizeButtonGradientLight = CBCGPDrawManager::SmartMixColors (
				m_clrCustomizeButtonGradientDark,
				clrBase, k, 3, 1);

			(*m_pfGetThemeColor) (m_hThemeButton, BP_PUSHBUTTON, PBS_DEFAULTED, TMT_ACCENTCOLORHINT, &clrCustom);
		}
		else
		{
			m_clrCustomizeButtonGradientDark = CBCGPDrawManager::SmartMixColors (
				clrCustom, 
				clrBase,
				0.63, 1, 3);

			(*m_pfGetThemeColor) (m_hThemeButton, BP_PUSHBUTTON, PBS_DEFAULTED, TMT_ACCENTCOLORHINT, &clrCustom);

			m_clrCustomizeButtonGradientLight = CBCGPDrawManager::SmartMixColors (
				clrCustom,
				clrBase,
				1.2, 1, 3);
		}

		globalData.clrBarDkShadow = m_clrCustomizeButtonGradientDark;

		if (m_WinXPTheme != WinXpTheme_Silver)
		{
			globalData.clrBarShadow = CBCGPDrawManager::SmartMixColors (
				clrCustom,
				clrBase,
				1.4, 1, 3);
		}

		m_clrToolBarBottomLine = m_WinXPTheme == WinXpTheme_Silver ?
			CBCGPDrawManager::PixelAlpha (m_clrToolBarGradientDark, 80) :
			CBCGPDrawManager::PixelAlpha (m_clrToolBarGradientDark, 50);

		m_colorToolBarCornerTop = CBCGPDrawManager::PixelAlpha (m_clrToolBarGradientLight, 92);
		m_colorToolBarCornerBottom = CBCGPDrawManager::PixelAlpha (m_clrToolBarGradientDark, 97);

		m_clrGripper = 
			CBCGPDrawManager::PixelAlpha (m_clrToolBarGradientVertDark, 40);

		clrSeparatorDark = 
			CBCGPDrawManager::PixelAlpha (m_clrToolBarGradientVertDark, 81);

		m_clrMenuItemBorder = m_clrGripper;
		
		m_clrMenuBorder = 
			CBCGPDrawManager::PixelAlpha (clrBase, 80);

		m_clrCaptionBarGradientDark = m_clrCustomizeButtonGradientDark;
		m_clrCaptionBarGradientLight = m_clrCustomizeButtonGradientLight;

		m_clrPlannerTodayFill = RGB (249, 203, 95);
		clrPlannerTodayLine = RGB (187, 85, 3);

		m_clrMenuLight = CBCGPDrawManager::PixelAlpha (
			globalData.clrWindow, .96, .96, .96);

		m_brMenuLight.DeleteObject ();
		m_brMenuLight.CreateSolidBrush (m_clrMenuLight);
	}
	else
	{
		m_clrHighlightMenuItem = (COLORREF)-1;

		m_clrHighlightGradientLight = m_clrHighlight;
		m_clrHighlightGradientDark = m_clrHighlightDn;
		m_clrHighlightDnGradientLight = 
			CBCGPDrawManager::PixelAlpha (m_clrHighlightGradientLight, 120);

		m_clrCustomizeButtonGradientDark = globalData.clrBarShadow;
		m_clrCustomizeButtonGradientLight = CBCGPDrawManager::SmartMixColors (
			m_clrCustomizeButtonGradientDark,
			globalData.clrBarFace, 1, 1, 1);

		m_clrToolBarBottomLine = CBCGPDrawManager::PixelAlpha (m_clrToolBarGradientDark, 75);
		m_colorToolBarCornerTop = globalData.clrBarLight;
		m_colorToolBarCornerBottom = m_clrToolBarGradientDark;

		m_clrGripper = globalData.clrBarShadow;
		clrSeparatorDark = globalData.clrBarShadow;

		m_clrCaptionBarGradientLight = globalData.clrBarShadow;
		m_clrCaptionBarGradientDark = globalData.clrBarDkShadow;

		m_clrPlannerTodayFill = m_clrHighlight;
		clrPlannerTodayLine = m_clrMenuItemBorder;
	}

	m_clrHighlightDnGradientDark = m_clrHighlightGradientDark;

	m_clrHighlightCheckedGradientLight = m_clrHighlightDnGradientDark;

	m_clrHighlightCheckedGradientDark = 
		CBCGPDrawManager::PixelAlpha (m_clrHighlightDnGradientLight, 120);

	m_brTabBack.DeleteObject ();
	m_brTabBack.CreateSolidBrush (m_clrToolBarGradientLight);

	m_penSeparatorLight.DeleteObject ();
	m_penSeparatorLight.CreatePen (PS_SOLID, 1, globalData.clrBarHilite);

	m_brTearOffCaption.DeleteObject ();
	m_brTearOffCaption.CreateSolidBrush (globalData.clrBarFace);

	m_brFace.DeleteObject ();
	m_brFace.CreateSolidBrush (m_clrToolBarGradientLight);

	m_penSeparator.DeleteObject ();
	m_penSeparator.CreatePen (PS_SOLID, 1, clrSeparatorDark);

	m_clrMenuShadowBase = globalData.clrBarFace;

	m_clrToolbarDisabled = CBCGPDrawManager::SmartMixColors (
		m_clrToolBarGradientDark, m_clrToolBarGradientLight, .92);

	m_penBottomLine.DeleteObject ();
	m_penBottomLine.CreatePen (PS_SOLID, 1, m_clrToolBarBottomLine);

	// --------------------------
	// Calculate TaskPane colors:
	// --------------------------
	if (m_bIsStandardWinXPTheme && m_hThemeExplorerBar != NULL)
	{
		ASSERT (m_pfGetThemeColor != NULL);

		(*m_pfGetThemeColor) (m_hThemeExplorerBar, 0, 0, TMT_GRADIENTCOLOR1, &m_clrTaskPaneGradientLight);
		(*m_pfGetThemeColor) (m_hThemeExplorerBar, 0, 0, TMT_GRADIENTCOLOR2, &m_clrTaskPaneGradientDark);

		(*m_pfGetThemeColor) (m_hThemeExplorerBar, EBP_NORMALGROUPBACKGROUND, 0, TMT_FILLCOLOR, &m_clrTaskPaneGroupCaptionDark);
		(*m_pfGetThemeColor) (m_hThemeExplorerBar, EBP_SPECIALGROUPHEAD, 0, TMT_FILLCOLOR, &m_clrTaskPaneGroupCaptionSpecDark);
		m_clrTaskPaneGroupCaptionSpecLight = m_clrTaskPaneGroupCaptionDark;

		(*m_pfGetThemeColor) (m_hThemeExplorerBar, EBP_NORMALGROUPBACKGROUND, 0, TMT_FILLCOLOR, &m_clrTaskPaneGroupAreaLight);
		m_clrTaskPaneGroupAreaDark = m_clrTaskPaneGroupAreaLight;
		(*m_pfGetThemeColor) (m_hThemeExplorerBar, EBP_SPECIALGROUPBACKGROUND, 0, TMT_FILLCOLORHINT, &m_clrTaskPaneGroupAreaSpecLight);
		m_clrTaskPaneGroupAreaSpecDark = m_clrTaskPaneGroupAreaSpecLight;
		(*m_pfGetThemeColor) (m_hThemeExplorerBar, EBP_NORMALGROUPBACKGROUND, 0, TMT_BORDERCOLOR, &m_clrTaskPaneGroupBorder);
		m_clrTaskPaneGroupCaptionLight = m_clrTaskPaneGroupBorder;

		if (m_WinXPTheme == WinXpTheme_Blue && globalData.bIsWindowsVista)
		{
			m_clrTaskPaneGradientDark = m_clrBarGradientDark;
			m_clrTaskPaneGradientLight = m_clrBarGradientLight;
			m_clrTaskPaneGroupCaptionSpecDark = m_clrToolBarGradientDark;
			m_clrTaskPaneGroupCaptionSpecLight = m_clrToolBarGradientLight;
		}
	}
	else
	{
		m_clrTaskPaneGradientDark  = m_clrBarGradientDark;
		m_clrTaskPaneGradientLight  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupCaptionDark  = m_clrBarGradientDark;
		m_clrTaskPaneGroupCaptionLight  = m_clrToolBarGradientLight;

		COLORREF clrLight = 
			CBCGPDrawManager::PixelAlpha (globalData.clrBarShadow, 125);

		m_clrTaskPaneGroupCaptionSpecDark = CBCGPDrawManager::SmartMixColors (
			m_clrCustomizeButtonGradientDark, 
			clrLight);

		m_clrTaskPaneGroupCaptionSpecLight  = m_clrCustomizeButtonGradientLight;
		m_clrTaskPaneGroupAreaLight  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupAreaDark  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupAreaSpecLight  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupAreaSpecDark  = m_clrToolBarGradientLight;
		m_clrTaskPaneGroupBorder  = m_clrToolBarGradientLight;
	}

	m_penTaskPaneGroupBorder.DeleteObject ();
	m_penTaskPaneGroupBorder.CreatePen (PS_SOLID, 1, m_clrTaskPaneGroupBorder);

	m_clrTaskPaneHotText = globalData.clrHilite;

	//--------------------------------------
	// Calculate grid/report control colors:
	//--------------------------------------
	m_penGridExpandBoxLight.DeleteObject ();
	m_penGridExpandBoxLight.CreatePen (PS_SOLID, 1, 
		CBCGPDrawManager::PixelAlpha (m_clrToolBarBottomLine, 210));

	m_penGridExpandBoxDark.DeleteObject ();
	m_penGridExpandBoxDark.CreatePen (PS_SOLID, 1, 
		CBCGPDrawManager::PixelAlpha (m_clrToolBarBottomLine, 75));

	//--------------------------
	// Calculate planner colors:
	//--------------------------
	m_clrPlannerWork = RGB (255, 255, 213);

	m_clrPlannerTodayLine = clrPlannerTodayLine;
	m_penPlannerTodayLine.DeleteObject ();
	m_penPlannerTodayLine.CreatePen (PS_SOLID, 1, m_clrPlannerTodayLine);

	m_clrDockingBarScrollButton = m_clrBarGradientDark;

	m_clrGridSelectAllActive = m_clrHighlightDn;
}
//***********************************************************************************
void CBCGPVisualManager2003::OnFillHighlightedArea (CDC* pDC, CRect rect, 
							CBrush* pBrush, CBCGPToolbarButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnFillHighlightedArea (pDC, rect, pBrush, pButton);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pBrush);

	BOOL bIsHorz = TRUE;
	BOOL bIsPopupMenu = FALSE;

	COLORREF clr1 = (COLORREF)-1;
	COLORREF clr2 = (COLORREF)-1;

	if (pButton != NULL)
	{
		ASSERT_VALID (pButton);

		bIsHorz = pButton->IsHorizontal ();

		CBCGPToolbarMenuButton* pMenuButton = 
			DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);

		bIsPopupMenu = pMenuButton != NULL &&
			pMenuButton->GetParentWnd () != NULL &&
			pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar));

		if (bIsPopupMenu && pBrush == &m_brHighlight)
		{
			if (m_clrHighlightMenuItem != (COLORREF)-1)
			{
				CBrush br (m_clrHighlightMenuItem);
				pDC->FillRect (&rect, &br);
				return;
			}
		}

		if (pMenuButton != NULL &&
			!bIsPopupMenu &&
			pMenuButton->IsDroppedDown ())
		{
			clr1 = CBCGPDrawManager::PixelAlpha (
				m_clrToolBarGradientDark, 
				m_bIsStandardWinXPTheme ? 101 : 120);

			clr2 = CBCGPDrawManager::PixelAlpha (
				m_clrToolBarGradientLight, 110);
		}
	}

	if (pBrush == &m_brHighlight && m_bIsStandardWinXPTheme)
	{
		clr1 = m_clrHighlightGradientDark;
		clr2 = bIsPopupMenu ? clr1 : m_clrHighlightGradientLight;
	}
	else if (pBrush == &m_brHighlightDn && m_bIsStandardWinXPTheme)
	{
		clr1 = bIsPopupMenu ? m_clrHighlightDnGradientLight : m_clrHighlightDnGradientDark;
		clr2 = m_clrHighlightDnGradientLight;
	}
	else if (pBrush == &m_brHighlightChecked && m_bIsStandardWinXPTheme)
	{
		clr1 = bIsPopupMenu ? m_clrHighlightCheckedGradientLight : m_clrHighlightCheckedGradientDark;
		clr2 = m_clrHighlightCheckedGradientLight;
	}

	if (clr1 == (COLORREF)-1 || clr2 == (COLORREF)-1)
	{
		CBCGPVisualManagerXP::OnFillHighlightedArea (pDC, rect, pBrush, pButton);
		return;
	}

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, clr1, clr2, bIsHorz);
}
//***********************************************************************************
void CBCGPVisualManager2003::OnDrawShowAllMenuItems (CDC* pDC, CRect rect, 
												 CBCGPVisualManager::BCGBUTTON_STATE state)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel > 8 && !globalData.IsHighContastMode ())
	{
		rect.left = rect.CenterPoint().x - rect.Height() / 2;
		rect.right = rect.left + rect.Height();
		rect.DeflateRect(1, 1);

		CBCGPDrawManager dm (*pDC);
		dm.DrawEllipse(rect, m_clrMenuLight, m_clrToolBarGradientDark);
	}

	CBCGPVisualManager::OnDrawShowAllMenuItems (pDC, rect, state);
}
//************************************************************************************
int CBCGPVisualManager2003::GetShowAllMenuItemsHeight (CDC* pDC, const CSize& sizeDefault)
{
	int nHeight = CBCGPVisualManager::GetShowAllMenuItemsHeight (pDC, sizeDefault);
	return nHeight + 4;
}
//***********************************************************************************
void CBCGPVisualManager2003::OnDrawCaptionBarBorder (CDC* pDC, 
	CBCGPCaptionBar* pBar, CRect rect, COLORREF clrBarBorder, BOOL bFlatBorder)
{
	ASSERT_VALID (pDC);

	if (clrBarBorder == (COLORREF) -1)
	{
		pDC->FillRect (rect, 
			(pBar != NULL && pBar->IsDialogControl ()) ? &globalData.brBtnFace : &globalData.brBarFace);
	}
	else
	{
		CBrush brBorder (clrBarBorder);
		pDC->FillRect (rect, &brBorder);
	}

	if (!bFlatBorder)
	{
		pDC->Draw3dRect (rect, m_clrBarGradientLight, m_clrToolBarBottomLine);
	}
}
//**************************************************************************************
void CBCGPVisualManager2003::OnDrawComboDropButton (CDC* pDC, CRect rect,
												BOOL bDisabled,
												BOOL bIsDropped,
												BOOL bIsHighlighted,
												CBCGPToolbarComboBoxButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnDrawComboDropButton (pDC, rect,
												bDisabled, bIsDropped,
												bIsHighlighted, pButton);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	const BOOL bIsCtrl = pButton != NULL && pButton->IsCtrlButton ();

	if (!bDisabled)
	{
		if (bIsDropped || bIsHighlighted)
		{
			OnFillHighlightedArea (pDC, rect, 
				bIsDropped ? &m_brHighlightDn : &m_brHighlight,
				NULL);

			if (!bIsCtrl)
			{
				if (CBCGPToolBarImages::m_bIsDrawOnGlass)
				{
					CBCGPDrawManager dm (*pDC);
					dm.DrawLine (rect.left, rect.top, rect.left, rect.bottom, m_clrMenuItemBorder);
				}
				else
				{
					CPen pen (PS_SOLID, 1, m_clrMenuItemBorder);
					CPen* pOldPen = pDC->SelectObject (&pen);
					ASSERT (pOldPen != NULL);

					pDC->MoveTo (rect.left, rect.top);
					pDC->LineTo (rect.left, rect.bottom);

					pDC->SelectObject (pOldPen);
				}
			}
		}
		else
		{
			CBCGPDrawManager dm (*pDC);
			dm.FillGradient (rect, m_clrToolBarGradientDark, m_clrToolBarGradientLight, TRUE);

			if (!bIsCtrl)
			{
				if (CBCGPToolBarImages::m_bIsDrawOnGlass)
				{
					dm.DrawRect (rect, (COLORREF)-1, globalData.clrWindow);
				}
				else
				{
					pDC->Draw3dRect (rect, globalData.clrWindow, globalData.clrWindow);
				}
			}
		}
	}
	else
	{
		pDC->FillRect(rect, &globalData.brBtnFace);
	}

	CBCGPMenuImages::Draw (pDC, CBCGPMenuImages::IdArowDown, rect,
		bDisabled ? CBCGPMenuImages::ImageGray : CBCGPMenuImages::ImageBlack);
}
//***********************************************************************************
void CBCGPVisualManager2003::OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnDrawTearOffCaption (pDC, rect, bIsActive);
		return;
	}

	const int iBorderSize = 1;
	ASSERT_VALID (pDC);

	pDC->FillRect (rect, &m_brMenuLight);

	rect.DeflateRect (iBorderSize, 1);

	if (bIsActive)
	{
		OnFillHighlightedArea (pDC, rect, bIsActive ? &m_brHighlight : &m_brBarBkgnd,
			NULL);
	}
	else
	{
		pDC->FillRect (rect, &m_brTearOffCaption);
	}
	
	// Draw gripper:
	OnDrawBarGripper (pDC, rect, FALSE, NULL);

	if (bIsActive)
	{
		pDC->Draw3dRect (rect, m_clrMenuBorder, m_clrMenuBorder);
	}
}
//***********************************************************************************
void CBCGPVisualManager2003::OnDrawMenuBorder (CDC* pDC, 
		CBCGPPopupMenu* pMenu, CRect rect)
{
	BOOL bConnectMenuToParent = m_bConnectMenuToParent;

	if (DYNAMIC_DOWNCAST (CCustomizeButton, pMenu->GetParentButton ()) != NULL)
	{
		m_bConnectMenuToParent = FALSE;
	}

	CBCGPVisualManagerXP::OnDrawMenuBorder (pDC, pMenu, rect);
	m_bConnectMenuToParent = bConnectMenuToParent;
}
//***********************************************************************************
COLORREF CBCGPVisualManager2003::GetThemeColor (HTHEME hTheme, int nIndex) const
{
	if (hTheme != NULL && m_pfGetThemeSysColor != NULL)
	{
		return (*m_pfGetThemeSysColor) (hTheme, nIndex);
	}
	
	return ::GetSysColor (nIndex);
}
//***********************************************************************************
void CBCGPVisualManager2003::OnEraseTabsArea (CDC* pDC, CRect rect, 
										 const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (pTabWnd->IsDialogControl () || pTabWnd->IsPointerStyle())
	{
		if (pTabWnd->IsVisualManagerStyle ())
		{
			OnFillDialog (pDC, pTabWnd->GetParent (), rect);
		}
		else if (pTabWnd->IsPointerStyle() && pTabWnd->IsMDITab())
		{
			pDC->FillRect(rect, &globalData.brBarFace);
		}
		else
		{
			pDC->FillRect (rect, &globalData.brBtnFace);
		}

		return;
	}

	if (pTabWnd->IsFlatTab () || globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnEraseTabsArea (pDC, rect, pTabWnd);
		return;
	}

	CBCGPDrawManager dm (*pDC);

	COLORREF clr1 = m_clrToolBarGradientDark;
	COLORREF clr2 = m_clrToolBarGradientLight;

	if (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
	{
		dm.FillGradient (rect, clr1, clr2, TRUE);
	}
	else
	{
		dm.FillGradient (rect, clr2, clr1, TRUE);
	}
}
//*************************************************************************************
void CBCGPVisualManager2003::OnDrawTab (CDC* pDC, CRect rectTab,
						int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pTabWnd);
	ASSERT_VALID (pDC);

	if (!pTabWnd->IsOneNoteStyle () || globalData.m_nBitsPerPixel <= 8 || 
		globalData.IsHighContastMode () || pTabWnd->IsLeftRightRounded () || pTabWnd->IsPointerStyle())
	{
		CBCGPVisualManagerXP::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);
		return;
	}

	CRect rectClip;
	pTabWnd->GetTabsRect (rectClip);

	const int nExtra = (pTabWnd->IsFirstTab(iTab) || bIsActive) ? 0 : rectTab.Height ();

	if (rectTab.left + nExtra + 10 > rectClip.right ||
		rectTab.right - 10 <= rectClip.left)
	{
		return;
	}

	const BOOL bIsHighlight = iTab == pTabWnd->GetHighlightedTab ();

	COLORREF clrTab = pTabWnd->GetTabBkColor (iTab);
	if (clrTab == (COLORREF)-1 && bIsActive)
	{
		clrTab = GetActiveTabBackColor(pTabWnd);
		if (clrTab == (COLORREF)-1)
		{
			clrTab = globalData.clrWindow;
		}
	}

	if (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
	{
		rectTab.OffsetRect (0, -1);
	}

	CRect rectFill = rectTab;

	#define POINTS_NUM	5
	POINT pts [POINTS_NUM];

	const int nHeight = rectFill.Height ();

	pts [0].x = rectFill.left;
	pts [0].y = rectFill.bottom;

	pts [1].x = rectFill.left + nHeight;
	pts [1].y = rectFill.top;
	
	pts [2].x = rectFill.right - 2;
	pts [2].y = rectFill.top;
	
	pts [3].x = rectFill.right;
	pts [3].y = rectFill.top + 2;

	pts [4].x = rectFill.right;
	pts [4].y = rectFill.bottom;

	BOOL bIsCutted = FALSE;

	for (int i = 0; i < POINTS_NUM; i++)
	{
		if (pts [i].x > rectClip.right)
		{
			pts [i].x = rectClip.right;
			bIsCutted = TRUE;
		}

		if (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
		{
			pts [i].y = rectFill.bottom - pts [i].y + rectFill.top;
		}
	}

	CRgn rgn;
	rgn.CreatePolygonRgn (pts, POINTS_NUM, WINDING);

	pDC->SelectClipRgn (&rgn);

	CRect rectLeft;
	pTabWnd->GetClientRect (rectLeft);
	rectLeft.right = rectClip.left - 1;

	pDC->ExcludeClipRect (rectLeft);

	CBCGPDrawManager dm (*pDC);

	COLORREF clrFill = bIsHighlight ? m_clrHighlightMenuItem : clrTab;
	COLORREF clr2 = clrFill;

	if (clrFill != (COLORREF)-1)
	{
		if (m_bTabFillGradient)
		{
			clr2 = CBCGPDrawManager::PixelAlpha (clrFill, 1.22, 1.22, 1.22);
		}
		else
		{
			clrFill = CBCGPDrawManager::PixelAlpha (clrFill, 1.1, 1.1, 1.1);
		}
	}
	else
	{
		clrFill = m_clrToolBarGradientDark;
		clr2 = m_clrToolBarGradientLight;
	}

	if (!m_bTabFillGradient)
	{
		clr2 = clrFill;
	}

	if (pTabWnd->GetLocation () == CBCGPTabWnd::LOCATION_BOTTOM)
	{
		rectFill.top++;
	}

	CRect rectTop = rectFill;
	rectTop.bottom = rectTop.CenterPoint ().y - 1;

	CBrush brTop (clr2);
	pDC->FillRect (rectTop, &brTop);

	CRect rectMiddle = rectFill;
	rectMiddle.top = rectTop.bottom;
	rectMiddle.bottom = rectMiddle.top + 3;

	dm.FillGradient (rectMiddle, clrFill, clr2);

	CRect rectBottom = rectFill;
	rectBottom.top = rectMiddle.bottom;

	CBrush brBottom (clrFill);
	pDC->FillRect (rectBottom, &brBottom);

	pDC->SelectClipRgn (NULL);

	pDC->ExcludeClipRect (rectLeft);

	if (!pTabWnd->IsFirstTab(iTab) && !bIsActive && iTab != pTabWnd->GetFirstVisibleTabNum ())
	{
		CRect rectLeftTab = rectClip;
		rectLeftTab.right = rectFill.left + rectFill.Height () - 10;

		if (pTabWnd->GetLocation () == CBCGPTabWnd::LOCATION_BOTTOM)
		{
			rectLeftTab.top -= 2;
		}
		else
		{
			rectLeftTab.bottom++;
		}

		pDC->ExcludeClipRect (rectLeftTab);
	}

	CPen penGray (PS_SOLID, 1, IsDarkTheme() ? globalData.clrBarHilite : globalData.clrBarDkShadow);
	CPen penShadow (PS_SOLID, 1, IsDarkTheme() ? globalData.clrBarLight : globalData.clrBarShadow);

	CPen* pOldPen = pDC->SelectObject (&penGray);
	CBrush* pOldBrush = (CBrush*) pDC->SelectStockObject (NULL_BRUSH);

	pDC->Polyline (pts, POINTS_NUM);

	if (bIsCutted)
	{
		pDC->MoveTo (rectClip.right, rectTab.top);
		pDC->LineTo (rectClip.right, rectTab.bottom);
	}

	CRect rectRight = rectClip;
	rectRight.left = rectFill.right;

	pDC->ExcludeClipRect (rectRight);

	if (m_bTabFillGradient)
	{
		CPen penLight (PS_SOLID, 1, IsDarkTheme() ? globalData.clrBarDkShadow : globalData.clrBarHilite);

		pDC->SelectObject (&penLight);

		if (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
		{
		}
		else
		{
			pDC->MoveTo (rectFill.left + 1, rectFill.bottom);
			pDC->LineTo (rectFill.left + nHeight, rectFill.top + 1);
			pDC->LineTo (rectFill.right - 1, rectFill.top + 1);
		}
	}

	pDC->SelectObject (pOldPen);
	pDC->SelectObject (pOldBrush);

	if (bIsActive)
	{
		const int iBarHeight = 1;
		const int y = (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM) ? 
			(rectTab.top - iBarHeight) : (rectTab.bottom);

		CRect rectFill (CPoint (rectTab.left + 2, y), 
						CSize (rectTab.Width () - 1, iBarHeight));
		
		if (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
		{
			rectFill.OffsetRect (-1, 1);
		}

		rectFill.right = min (rectFill.right, rectClip.right);

		CBrush br (clrTab);
		pDC->FillRect (rectFill, &br);
	}

	if (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
	{
		rectTab.left += rectTab.Height () + CBCGPBaseTabWnd::TAB_IMAGE_MARGIN;
	}
	else
	{
		rectTab.left += rectTab.Height ();
		rectTab.right -= CBCGPBaseTabWnd::TAB_IMAGE_MARGIN;
	}

	COLORREF clrText = pTabWnd->GetTabTextColor (iTab);
	
	COLORREF cltTextOld = (COLORREF)-1;
	if (clrText != (COLORREF)-1)
	{
		cltTextOld = pDC->SetTextColor (clrText);
	}

	rectTab.right = min (rectTab.right, rectClip.right - 2);

	OnDrawTabContent (pDC, rectTab, iTab, bIsActive, pTabWnd, (COLORREF)-1);

	if (cltTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor (cltTextOld);
	}

	pDC->SelectClipRgn (NULL);
}
//*********************************************************************************
void CBCGPVisualManager2003::OnFillTab (CDC* pDC, CRect rectFill, CBrush* pbrFill,
									 int iTab, BOOL bIsActive, 
									 const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pTabWnd);

	if (pTabWnd->IsFlatTab () || globalData.m_nBitsPerPixel <= 8 || 
		globalData.IsHighContastMode () || pTabWnd->IsDialogControl () || pTabWnd->IsPointerStyle())
	{
		CBCGPVisualManagerXP::OnFillTab (pDC, rectFill, pbrFill,
									 iTab, bIsActive, pTabWnd);
		return;
	}

	ASSERT_VALID (pDC);

	COLORREF clr1 = CBCGPDrawManager::PixelAlpha (m_clrBarGradientDark, 105);
	
	if (pTabWnd->GetTabBkColor (iTab) != (COLORREF)-1)
	{
		clr1 = pTabWnd->GetTabBkColor (iTab);

		if (clr1 == globalData.clrWindow && bIsActive)
		{
			pDC->FillRect (rectFill, &globalData.brWindow);
			return;
		}
	}
	else 
	{
		if (m_bAlwaysFillTab)
		{
			if (bIsActive)
			{
				pDC->FillRect (rectFill, &globalData.brWindow);
				return;
			}
		}
		else
		{
			if (pTabWnd->IsVS2005Style () || pTabWnd->IsLeftRightRounded ())
			{
				if (bIsActive)
				{
					pDC->FillRect (rectFill, &globalData.brWindow);
					return;
				}
			}
			else if (!bIsActive)
			{
				return;
			}
		}
	}

	if (!m_bTabFillGradient)
	{
		CBrush br(CBCGPDrawManager::PixelAlpha (clr1, 110));
		pDC->FillRect(rectFill, &br);
		return;
	}

	COLORREF clr2 = CBCGPDrawManager::PixelAlpha (clr1, 120);

	CBCGPDrawManager dm (*pDC);

	if (pTabWnd->GetLocation () == CBCGPTabWnd::LOCATION_TOP)
	{
		dm.FillGradient (rectFill, clr1, clr2, TRUE);
	}
	else
	{
		dm.FillGradient (rectFill, clr2, clr1, TRUE);
	}
}
//***********************************************************************************
BOOL CBCGPVisualManager2003::OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd)
{	
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (pTabWnd->IsFlatTab () || globalData.m_nBitsPerPixel <= 8 || 
		globalData.IsHighContastMode () || pTabWnd->IsDialogControl ())
	{
		return CBCGPVisualManagerXP::OnEraseTabsFrame (pDC, rect, pTabWnd);
	}

	COLORREF clrActiveTab = pTabWnd->GetTabBkColor (pTabWnd->GetActiveTab ());
	if (clrActiveTab == (COLORREF)-1 && 
		(pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style ()))
	{
		pDC->FillRect (rect, &globalData.brWindow);
		return TRUE;
	}

	CBCGPDrawManager dm (*pDC);

	COLORREF clr1 = m_clrBarGradientDark;

	if (clrActiveTab != (COLORREF)-1)
	{
		clr1 = clrActiveTab;
	}

	COLORREF clr2 = CBCGPDrawManager::PixelAlpha (clr1, 130);

	if (pTabWnd->GetLocation () == CBCGPTabWnd::LOCATION_BOTTOM)
	{
		COLORREF clr = clr1;
		clr1 = clr2;
		clr2 = clr;
	}

	dm.FillGradient2 (rect, clr1, clr2, 45);
	return TRUE;
}
//*********************************************************************************
void CBCGPVisualManager2003::OnEraseTabsButton (CDC* pDC, CRect rect,
											  CBCGPButton* pButton,
											  CBCGPBaseTabWnd* pBaseTab)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);
	ASSERT_VALID (pBaseTab);

	CBCGPTabWnd* pWndTab = DYNAMIC_DOWNCAST (CBCGPTabWnd, pBaseTab);

	if (pWndTab == NULL || pBaseTab->IsFlatTab () || 
		globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		pBaseTab->IsDialogControl ())
	{
		if (pBaseTab->IsDialogControl () && pBaseTab->IsVisualManagerStyle ())
		{
			OnFillDialog (pDC, pBaseTab->GetParent (), rect);
		}
		else
		{
			CBCGPVisualManagerXP::OnEraseTabsButton (pDC, rect, pButton, pBaseTab);
		}

		return;
	}

	if ((pBaseTab->IsOneNoteStyle () || pBaseTab->IsVS2005Style ()) && 
		(pButton->IsPressed () || pButton->IsHighlighted ()))
	{
		CBCGPDrawManager dm (*pDC);

		if (pButton->IsPressed ())
		{
			dm.FillGradient (rect, m_clrHighlightDnGradientDark, m_clrHighlightDnGradientLight);
		}
		else
		{
			dm.FillGradient (rect, m_clrHighlightGradientDark, m_clrHighlightGradientLight);
		}

		return;
	}

	CRgn rgn;
	rgn.CreateRectRgnIndirect (rect);

	pDC->SelectClipRgn (&rgn);

	CRect rectTabs;
	pWndTab->GetClientRect (&rectTabs);

	CRect rectTabArea;
	pWndTab->GetTabsRect (rectTabArea);

	if (pWndTab->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
	{
		rectTabs.top = rectTabArea.top;
	}
	else
	{
		rectTabs.bottom = rectTabArea.bottom;
	}

	pWndTab->MapWindowPoints (pButton, rectTabs);
	OnEraseTabsArea (pDC, rectTabs, pWndTab);

	pDC->SelectClipRgn (NULL);
}
//************************************************************************************
void CBCGPVisualManager2003::OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
												 CBCGPButton* pButton, UINT /*uiState*/,
												 CBCGPBaseTabWnd* /*pWndTab*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pButton->IsPushed () || pButton->IsHighlighted ())
	{
		pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
}
//************************************************************************************
void CBCGPVisualManager2003::ModifyGlobalColors ()
{
	if (globalData.m_nBitsPerPixel <= 8 || !m_bIsStandardWinXPTheme || globalData.IsHighContastMode ())
	{
		//----------------------------------------------
		// Theme color may differ from the system color:
		//----------------------------------------------
		globalData.clrBarFace = GetThemeColor (m_hThemeButton, COLOR_3DFACE);
		globalData.clrBarShadow = GetThemeColor (m_hThemeButton, COLOR_3DSHADOW);
		globalData.clrBarHilite = GetThemeColor (m_hThemeButton, COLOR_3DHIGHLIGHT);
		globalData.clrBarDkShadow = GetThemeColor (m_hThemeButton, COLOR_3DDKSHADOW);
		globalData.clrBarLight = GetThemeColor (m_hThemeButton, COLOR_3DLIGHT);
	}
	else
	{
		COLORREF clrBase = GetBaseThemeColor ();

		if (m_WinXPTheme == WinXpTheme_Olive)
		{
			COLORREF clrToolBarGradientDark = CBCGPDrawManager::PixelAlpha (
				clrBase, 120);

			COLORREF clrToolBarGradientLight = CBCGPDrawManager::SmartMixColors (
				clrBase, 
				GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
				1., 2, 1);

			globalData.clrBarFace = CBCGPDrawManager::SmartMixColors (
				clrToolBarGradientDark,
				clrToolBarGradientLight, 1., 2, 1);
		}
		else if (m_WinXPTheme == WinXpTheme_Silver)
		{
			COLORREF clrToolBarGradientDark = CBCGPDrawManager::SmartMixColors (
				clrBase, 
				GetThemeColor (m_hThemeWindow, COLOR_3DFACE),
				0.75, 2);

			COLORREF clrToolBarGradientLight = CBCGPDrawManager::SmartMixColors (
				clrBase, 
				GetThemeColor (m_hThemeWindow, COLOR_WINDOW),
				1.03);

			globalData.clrBarFace = CBCGPDrawManager::PixelAlpha (CBCGPDrawManager::SmartMixColors (
				clrToolBarGradientDark,
				clrToolBarGradientLight), 95);
		}
		else
		{
			globalData.clrBarFace = CBCGPDrawManager::SmartMixColors (
				GetThemeColor (m_hThemeWindow, /*COLOR_HIGHLIGHT*/29),
				GetThemeColor (m_hThemeWindow, COLOR_WINDOW));
		}

		globalData.clrBarShadow = CBCGPDrawManager::PixelAlpha (
			globalData.clrBarFace, 70);
		globalData.clrBarHilite = CBCGPDrawManager::PixelAlpha (
			globalData.clrBarFace, 130);
		globalData.clrBarDkShadow = CBCGPDrawManager::PixelAlpha (
			globalData.clrBarFace, 50);
		globalData.clrBarLight = CBCGPDrawManager::PixelAlpha (
			globalData.clrBarFace, 110);
	}

	globalData.brBarFace.DeleteObject ();
	globalData.brBarFace.CreateSolidBrush (globalData.clrBarFace);
}
//************************************************************************************
void CBCGPVisualManager2003::SetUseGlobalTheme (BOOL bUseGlobalTheme/* = TRUE*/)
{
	m_bUseGlobalTheme = bUseGlobalTheme;

	CBCGPVisualManager::GetInstance ()->OnUpdateSystemColors ();
	CBCGPVisualManager::GetInstance ()->RedrawAll ();
}
//************************************************************************************
void CBCGPVisualManager2003::SetStatusBarOfficeXPLook (BOOL bStatusBarOfficeXPLook/* = TRUE*/)
{
	m_bStatusBarOfficeXPLook = bStatusBarOfficeXPLook;

	CBCGPVisualManager::GetInstance ()->RedrawAll ();
}
//***********************************************************************************
void CBCGPVisualManager2003::SetDefaultWinXPColors (BOOL bDefaultWinXPColors/* = TRUE*/)
{
	m_bDefaultWinXPColors = bDefaultWinXPColors;

	CBCGPVisualManager::GetInstance ()->OnUpdateSystemColors ();
	CBCGPVisualManager::GetInstance ()->RedrawAll ();
}
//************************************************************************************
void CBCGPVisualManager2003::GetTabFrameColors (const CBCGPBaseTabWnd* pTabWnd,
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
	
	CBCGPVisualManagerXP::GetTabFrameColors (pTabWnd,
			   clrDark, clrBlack,
			   clrHighlight, clrFace,
			   clrDarkShadow, clrLight,
			   pbrFace, pbrBlack);

	if (pTabWnd->IsOneNoteStyle () || globalData.m_nBitsPerPixel <= 8 || 
		globalData.IsHighContastMode () || pTabWnd->IsDialogControl () ||
		!m_bIsStandardWinXPTheme)
	{
		return;
	}

	COLORREF clrActiveTab = pTabWnd->GetTabBkColor (pTabWnd->GetActiveTab ());

	if (clrActiveTab == (COLORREF)-1)
	{
		clrFace = globalData.clrWindow;
	}

	clrDark = globalData.clrBarShadow;
	clrBlack = globalData.clrBarDkShadow;
	clrHighlight = pTabWnd->IsVS2005Style () ? globalData.clrBarShadow : globalData.clrBarLight;
	clrDarkShadow = globalData.clrBarShadow;
	clrLight = globalData.clrBarFace;
}

#ifndef BCGP_EXCLUDE_TASK_PANE

void CBCGPVisualManager2003::OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnFillTasksPaneBackground (pDC, rectWorkArea);
		return;
	}

	ASSERT_VALID (pDC);

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rectWorkArea, m_clrTaskPaneGradientDark, m_clrTaskPaneGradientLight, TRUE);
}
//************************************************************************************
void CBCGPVisualManager2003::OnDrawTasksGroupCaption(
										CDC* pDC, CBCGPTasksGroup* pGroup, 
										BOOL bIsHighlighted /*= FALSE*/, BOOL bIsSelected /*= FALSE*/, 
										BOOL bCanCollapse /*= FALSE*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID (pGroup);
	ASSERT_VALID (pGroup->m_pPage);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnDrawTasksGroupCaption(
										pDC, pGroup, 
										bIsHighlighted, bIsSelected, bCanCollapse);
		return;
	}

#ifndef BCGP_EXCLUDE_TOOLBOX
	BOOL bIsToolBox = pGroup->m_pPage->m_pTaskPane != NULL &&
		(pGroup->m_pPage->m_pTaskPane->IsKindOf (RUNTIME_CLASS (CBCGPToolBoxEx)));
#else
	BOOL bIsToolBox = FALSE;
#endif

	CRect rectGroup = pGroup->m_rect;

	// -----------------------
	// Draw caption background
	// -----------------------
	if (bIsToolBox)
	{
		CRect rectFill = rectGroup;
		rectFill.DeflateRect (1, 1);
		rectFill.bottom--;

		COLORREF clrGrdaient1 = CBCGPDrawManager::PixelAlpha (
			m_clrToolBarGradientDark, 105);
		COLORREF clrGrdaient2 = CBCGPDrawManager::PixelAlpha (
			m_clrToolBarGradientDark, 120);

		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rectFill, clrGrdaient1, clrGrdaient2, TRUE);

		CBrush brFillBottom (CBCGPDrawManager::PixelAlpha (m_clrToolBarGradientDark, 120));
		
		CRect rectFillBottom = rectGroup;
		rectFillBottom.DeflateRect (1, 0);
		rectFillBottom.top = rectFillBottom.bottom - 1;

		pDC->FillRect (rectFillBottom, &brFillBottom);

		if (bCanCollapse)
		{
			//--------------------
			// Draw expanding box:
			//--------------------
			int nBoxSize = 9;
			int nBoxOffset = 6;

			if (globalData.GetRibbonImageScale () != 1.)
			{
				nBoxSize = (int)(.5 + nBoxSize * globalData.GetRibbonImageScale ());
			}


			CRect rectButton = rectFill;
			
			rectButton.left += nBoxOffset;
			rectButton.right = rectButton.left + nBoxSize;
			rectButton.top = rectButton.CenterPoint ().y - nBoxSize / 2;
			rectButton.bottom = rectButton.top + nBoxSize;

			OnDrawExpandingBox (pDC, rectButton, !pGroup->m_bIsCollapsed, 
				globalData.clrBarText);

			rectGroup.left = rectButton.right + nBoxOffset;
			bCanCollapse = FALSE;
		}
	}
	else
	{
		POINT pts [7];

		const int nLeft = pGroup->m_rect.left;
		const int nTop = pGroup->m_rect.top;

		pts [0].x = nLeft;
		pts [0].y = pGroup->m_rect.bottom;

		pts [1].x = nLeft;
		pts [1].y = nTop + 4;

		pts [2].x = nLeft + 1;
		pts [2].y = nTop + 2;
		
		pts [3].x = nLeft + 2;
		pts [3].y = nTop + 1;
		
		pts [4].x = nLeft + 4;
		pts [4].y = nTop;

		pts [5].x = pGroup->m_rect.right;
		pts [5].y = nTop;

		pts [6].x = pGroup->m_rect.right;
		pts [6].y = pGroup->m_rect.bottom;

		CRgn rgn;
		rgn.CreatePolygonRgn (pts, 7, WINDING);

		pDC->SelectClipRgn (&rgn);

		CBCGPDrawManager dm (*pDC);

		if (pGroup->m_bIsSpecial)
		{
			dm.FillGradient (pGroup->m_rect, m_clrTaskPaneGroupCaptionSpecDark, 
				m_clrTaskPaneGroupCaptionSpecLight, FALSE);
		}
		else
		{
			dm.FillGradient (pGroup->m_rect, m_clrTaskPaneGroupCaptionLight, 
				m_clrTaskPaneGroupCaptionDark, FALSE);
		}

		pDC->SelectClipRgn (NULL);
	}

	// ---------------------------
	// Draw an icon if it presents
	// ---------------------------
	BOOL bShowIcon = (pGroup->m_hIcon != NULL 
		&& pGroup->m_sizeIcon.cx < rectGroup.Width () - rectGroup.Height());
	if (bShowIcon)
	{
		OnDrawTasksGroupIcon(pDC, pGroup, 5, bIsHighlighted, bIsSelected, bCanCollapse);
	}

	// -----------------------
	// Draw group caption text
	// -----------------------
	CFont* pFontOld = pDC->SelectObject (&globalData.fontBold);
	COLORREF clrTextOld = pDC->GetTextColor();

	if (bIsToolBox)
	{
		pDC->SetTextColor (globalData.clrBarText);
	}
	else
	{
		if (bCanCollapse && bIsHighlighted)
		{
			pDC->SetTextColor (pGroup->m_clrTextHot == (COLORREF)-1 ?
				(pGroup->m_bIsSpecial ? m_clrTaskPaneGroupBorder : m_clrTaskPaneHotText) :
				pGroup->m_clrTextHot);
		}
		else
		{
			pDC->SetTextColor (pGroup->m_clrText == (COLORREF)-1 ?
				(pGroup->m_bIsSpecial ? m_clrTaskPaneGroupBorder : m_clrTaskPaneHotText) :
				pGroup->m_clrText);
		}
	}

	int nBkModeOld = pDC->SetBkMode(TRANSPARENT);
	
	int nTaskPaneHOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionHorzOffset();
	int nTaskPaneVOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionVertOffset();
	int nCaptionHOffset = (nTaskPaneHOffset != -1 ? nTaskPaneHOffset : m_nGroupCaptionHorzOffset);

	CRect rectText = rectGroup;
	rectText.left += (bShowIcon ? pGroup->m_sizeIcon.cx	+ 5: nCaptionHOffset);
	rectText.top += (nTaskPaneVOffset != -1 ? nTaskPaneVOffset : m_nGroupCaptionVertOffset);
	rectText.right = max(rectText.left, 
						rectText.right - (bCanCollapse ? rectGroup.Height() : nCaptionHOffset));

	pDC->DrawText (pGroup->m_strName, rectText, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

	pDC->SetBkMode(nBkModeOld);
	pDC->SelectObject (pFontOld);
	pDC->SetTextColor (clrTextOld);

	// -------------------------
	// Draw group caption button
	// -------------------------
	if (bCanCollapse && !pGroup->m_strName.IsEmpty())
	{
		CSize sizeButton = CBCGPMenuImages::Size();
		CRect rectButton = rectGroup;
		rectButton.left = max(rectButton.left, 
			rectButton.right - (rectButton.Height()+1)/2 - (sizeButton.cx+1)/2);
		rectButton.top = max(rectButton.top, 
			rectButton.bottom - (rectButton.Height()+1)/2 - (sizeButton.cy+1)/2);
		rectButton.right = rectButton.left + sizeButton.cx;
		rectButton.bottom = rectButton.top + sizeButton.cy;

		if (rectButton.right <= rectGroup.right && rectButton.bottom <= rectGroup.bottom)
		{
			if (bIsHighlighted)
			{
				// Draw button frame
				CBrush* pBrushOld = (CBrush*) pDC->SelectObject (&globalData.brBarFace);
				COLORREF clrBckOld = pDC->GetBkColor ();

				pDC->Draw3dRect(&rectButton, globalData.clrWindow, globalData.clrBarShadow);

				pDC->SetBkColor (clrBckOld);
				pDC->SelectObject (pBrushOld);
			}

			if (!pGroup->m_bIsCollapsed)
			{
				CBCGPMenuImages::Draw(pDC, CBCGPMenuImages::IdArowUp, rectButton.TopLeft());
			}
			else
			{
				CBCGPMenuImages::Draw(pDC, CBCGPMenuImages::IdArowDown, rectButton.TopLeft());
			}
		}
	}
}
//************************************************************************************
void CBCGPVisualManager2003::OnFillTasksGroupInterior(
								CDC* pDC, CRect rect, BOOL bSpecial /*= FALSE*/)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnFillTasksGroupInterior(pDC, rect, bSpecial);
		return;
	}

	ASSERT_VALID (pDC);

	CBCGPDrawManager dm (*pDC);
	if (bSpecial)
	{
		dm.FillGradient (rect, m_clrTaskPaneGroupCaptionSpecDark, 
			m_clrTaskPaneGroupCaptionSpecLight, TRUE);
	}
	else
	{
		dm.FillGradient (rect, m_clrTaskPaneGroupAreaDark, 
			m_clrTaskPaneGroupAreaLight, TRUE);
	}
}
//************************************************************************************
void CBCGPVisualManager2003::OnDrawTasksGroupAreaBorder(
											CDC* pDC, CRect rect, BOOL /*bSpecial = FALSE*/,
											BOOL /*bNoTitle = FALSE*/)
{
	ASSERT_VALID(pDC);

	// Draw underline
	CPen* pPenOld = (CPen*) pDC->SelectObject (&m_penTaskPaneGroupBorder);

	rect.right -= 1;
	rect.bottom -= 1;
	pDC->MoveTo (rect.left, rect.top);
	pDC->LineTo (rect.right, rect.top);
	pDC->LineTo (rect.right, rect.bottom);
	pDC->LineTo (rect.left, rect.bottom);
	pDC->LineTo (rect.left, rect.top);

	pDC->SelectObject (pPenOld);
}
//************************************************************************************
void CBCGPVisualManager2003::OnDrawTask(CDC* pDC, CBCGPTask* pTask, CImageList* pIcons, 
							BOOL bIsHighlighted /*= FALSE*/, BOOL bIsSelected /*= FALSE*/)
{
	ASSERT_VALID (pTask);
	ASSERT_VALID (pDC);

	if (pTask->m_bIsSeparator)
	{
		CRect rectText = pTask->m_rect;

		CPen* pPenOld = (CPen*) pDC->SelectObject (&m_penSeparator);

		pDC->MoveTo (rectText.left, rectText.CenterPoint ().y);
		pDC->LineTo (rectText.right, rectText.CenterPoint ().y);

		pDC->SelectObject (pPenOld);
		return;
	}

	CBCGPVisualManagerXP::OnDrawTask(pDC, pTask, pIcons, bIsHighlighted, bIsSelected);
}
//**********************************************************************************
void CBCGPVisualManager2003::OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited)
{
	ASSERT_VALID (pDC);

	CRect rectImage (CPoint (0, 0), CBCGPMenuImages::Size ());

	CRect rectFill = rect;
	rectFill.top -= nBorderSize;

	pDC->FillRect (rectFill, &globalData.brBarFace);

	if (bHilited)
	{
		CBrush br (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ?
			globalData.clrWindow : m_clrHighlightMenuItem == (COLORREF)-1 ? 
			m_clrHighlight : m_clrHighlightMenuItem);

		pDC->FillRect (rect, &br);
		pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
	else
	{
		pDC->Draw3dRect (rect, globalData.clrBarShadow, globalData.clrBarShadow);
	}

	CBCGPMenuImages::Draw (pDC, (CBCGPMenuImages::IMAGES_IDS) iImage, rect);
}

#endif // BCGP_EXCLUDE_TASK_PANE

COLORREF CBCGPVisualManager2003::OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnFillCommandsListBackground (pDC, rect, bIsSelected);
	}

	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	COLORREF clrText = globalData.clrBarText;

	int iImageWidth = CBCGPToolBar::GetMenuImageSize ().cx + GetMenuImageMargin ();

	if (bIsSelected)
	{
		rect.left = 0;

		COLORREF color = m_clrHighlightMenuItem == (COLORREF)-1 ?
			m_clrHighlight : m_clrHighlightMenuItem;
		
		CBrush br (color);
		pDC->FillRect (&rect, &br);

		pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);

		// Now, we should define a menu text color...
		if (GetRValue (color) > 100 &&
			GetGValue (color) > 100 &&
			GetBValue (color) > 100)
		{
			clrText = RGB (0, 0, 0);
		}
		else
		{
			clrText = RGB (255, 255, 255);
		}
	}
	else
	{
		pDC->FillRect (rect, &m_brMenuLight);

		CRect rectImages = rect;
		rectImages.right = rectImages.left + iImageWidth + MENU_IMAGE_MARGIN;

		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rectImages, m_clrToolBarGradientLight, m_clrToolBarGradientDark, FALSE);

		clrText = globalData.clrBarText;
	}

	return clrText;
}
//**************************************************************************************
void CBCGPVisualManager2003::OnDrawStatusBarProgress (CDC* pDC, CBCGPStatusBar* pStatusBar,
			CRect rectProgress, int nProgressTotal, int nProgressCurr,
			COLORREF clrBar, COLORREF clrProgressBarDest, COLORREF clrProgressText,
			BOOL bProgressText)
{
	if (!DrawStatusBarProgress (pDC, pStatusBar,
			rectProgress, nProgressTotal, nProgressCurr,
			clrBar, clrProgressBarDest, clrProgressText, bProgressText))
	{
		CBCGPVisualManagerXP::OnDrawStatusBarProgress (pDC, pStatusBar,
			rectProgress, nProgressTotal, nProgressCurr,
			clrBar, clrProgressBarDest, clrProgressText, bProgressText);
	}
}
//****************************************************************************************
void CBCGPVisualManager2003::OnDrawStatusBarPaneBorder (CDC* pDC, CBCGPStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle)
{
	if (!m_bStatusBarOfficeXPLook || m_hThemeStatusBar == NULL)
	{
		CBCGPVisualManagerXP::OnDrawStatusBarPaneBorder (pDC, pBar,
						rectPane, uiID, nStyle);
	}

	if (m_hThemeStatusBar != NULL &&
		!(nStyle & SBPS_NOBORDERS))
	{
		(*m_pfDrawThemeBackground) (m_hThemeStatusBar, pDC->GetSafeHdc(), 1 /*SP_PANE*/,
			0, &rectPane, 0);
	}
}
//****************************************************************************************
COLORREF CBCGPVisualManager2003::OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
			BOOL bActive, CRect rectCaption, CRect rectButtons)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnDrawControlBarCaption (pDC, pBar, 
			bActive, rectCaption, rectButtons);
	}

	CBCGPDrawManager dm (*pDC);

	if (!bActive)
	{
		dm.FillGradient (rectCaption, 
						m_clrToolBarGradientDark, 
						m_clrToolBarGradientLight, TRUE);
	}
	else
	{
		dm.FillGradient (rectCaption,	
								m_clrHighlightGradientDark,
								m_clrHighlightGradientLight,
								TRUE);
	}

	return globalData.clrBarText;
}
//*********************************************************************************
void CBCGPVisualManager2003::OnFillAutoHideButtonBackground (CDC* pDC, CRect rect, CBCGPAutoHideButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnFillAutoHideButtonBackground (pDC, rect, pButton);
		return;
	}

	CBCGPDrawManager dm (*pDC);

	if (pButton->IsActive ())
	{
		dm.FillGradient (rect, 
			m_clrHighlightGradientLight, m_clrHighlightGradientDark,
			pButton->IsHorizontal ());
	}
	else
	{
		dm.FillGradient (rect, 
			m_clrBarGradientLight, m_clrBarGradientDark,
			pButton->IsHorizontal ());
	}
}
//*********************************************************************************
void CBCGPVisualManager2003::OnDrawAutoHideButtonBorder (CDC* pDC, CRect rectBounds, CRect rectBorderSize, CBCGPAutoHideButton* pButton)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnDrawAutoHideButtonBorder (pDC, rectBounds, rectBorderSize, pButton);
		return;
	}

	COLORREF clr = GetAutoHideButtonBorderColor(pButton);
	COLORREF clrText = pDC->GetTextColor ();

	if (rectBorderSize.left > 0)
	{
		pDC->FillSolidRect (rectBounds.left, rectBounds.top, 
							rectBounds.left + rectBorderSize.left, 
							rectBounds.bottom, clr);
	}
	if (rectBorderSize.top > 0)
	{
		pDC->FillSolidRect (rectBounds.left, rectBounds.top, 
							rectBounds.right, 
							rectBounds.top + rectBorderSize.top, clr);
	}
	if (rectBorderSize.right > 0)
	{
		pDC->FillSolidRect (rectBounds.right - rectBorderSize.right, rectBounds.top, 
							rectBounds.right, 
							rectBounds.bottom, clr);
	}
	if (rectBorderSize.bottom > 0)
	{
		pDC->FillSolidRect (rectBounds.left, rectBounds.bottom - rectBorderSize.bottom,  
							rectBounds.right, 
							rectBounds.bottom, clr);
	}

	pDC->SetTextColor (clrText);
}
//*******************************************************************************
void CBCGPVisualManager2003::OnDrawOutlookBarSplitter (CDC* pDC, CRect rectSplitter)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnDrawOutlookBarSplitter (pDC, rectSplitter);
		return;
	}

	CBCGPDrawManager dm (*pDC);

	dm.FillGradient (rectSplitter,
					m_clrCaptionBarGradientDark,
					m_clrCaptionBarGradientLight,
					TRUE);

	const int nBoxesNumber = 10;
	const int nBoxSize = rectSplitter.Height () - 3;

	int x = rectSplitter.CenterPoint ().x - nBoxSize * nBoxesNumber / 2;
	int y = rectSplitter.top + 2;

	for (int nBox = 0; nBox < nBoxesNumber; nBox++)
	{
		pDC->FillSolidRect (x + 1, y + 1, nBoxSize / 2, nBoxSize / 2, 
			globalData.clrBtnHilite);
		pDC->FillSolidRect (x, y, nBoxSize / 2, nBoxSize / 2, 
			m_clrGripper);

		x += nBoxSize;
	}
}
//*******************************************************************************
void CBCGPVisualManager2003::OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, COLORREF& clrText)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnFillOutlookBarCaption (pDC, rectCaption, clrText);
		return;
	}

	CBCGPDrawManager dm (*pDC);

	dm.FillGradient (rectCaption,
		m_clrCaptionBarGradientDark, m_clrCaptionBarGradientLight, TRUE);
	clrText = globalData.clrBarHilite;
}
//********************************************************************************
BOOL CBCGPVisualManager2003::OnDrawCalculatorButton (CDC* pDC, 
	CRect rect, CBCGPToolbarButton* pButton, 
	CBCGPVisualManager::BCGBUTTON_STATE state, 
	int cmd,
	CBCGPCalculator* pCalculator)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnDrawCalculatorButton (pDC, rect, pButton, state, cmd, pCalculator);
	}

	ASSERT_VALID (pButton);
	ASSERT_VALID (pDC);
	ASSERT_VALID (pCalculator);

	switch (state)
	{
	case ButtonsIsPressed:
		OnFillHighlightedArea (pDC, rect, &m_brHighlightDn, pButton);
		break;

	case ButtonsIsHighlighted:
		OnFillHighlightedArea (pDC, rect, &m_brHighlight, pButton);
		break;

	default:
		{
			CBCGPDrawManager dm (*pDC);

			if (pCalculator->IsDialogControl ())
			{
				dm.FillGradient (rect, globalData.clrBtnFace, globalData.clrBtnHilite);
				pDC->Draw3dRect (&rect, globalData.clrBtnShadow, globalData.clrBtnShadow);
				return TRUE;
			}
			else
			{
				dm.FillGradient (rect, m_clrToolBarGradientDark, m_clrToolBarGradientLight);
			}
		}
		break;
	}

	pDC->Draw3dRect (&rect, m_clrToolBarGradientDark, m_clrToolBarGradientDark);

	return TRUE;
}
//********************************************************************************
BOOL CBCGPVisualManager2003::OnDrawCalculatorDisplay (CDC* pDC, CRect rect, 
												  const CString& strText, BOOL bMem,
												  CBCGPCalculator* pCalculator)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnDrawCalculatorDisplay(pDC, rect, strText, bMem, pCalculator);
	}

	pDC->FillRect (rect, &globalData.brWindow);
	pDC->Draw3dRect (&rect, m_clrToolBarGradientDark, m_clrToolBarGradientDark);

	return TRUE;
}
//********************************************************************************
BOOL CBCGPVisualManager2003::OnDrawBrowseButton (CDC* pDC, CRect rect, 
	CBCGPEdit* pEdit, CBCGPVisualManager::BCGBUTTON_STATE state, COLORREF& clrText)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnDrawBrowseButton(pDC, rect, pEdit, state, clrText);
	}

	if (pEdit != NULL && !pEdit->m_bVisualManagerStyle && m_hThemeButton != NULL)
	{
		int nState = PBS_NORMAL;

		if (!pEdit->IsWindowEnabled ())
		{
			nState = PBS_DISABLED;
		}
		else if (state == ButtonsIsHighlighted)
		{
			nState = PBS_HOT;
		}
		else if (state == ButtonsIsPressed)
		{
			nState = PBS_PRESSED;
		}

		pDC->FillRect(rect, &globalData.brWindow);
		(*m_pfDrawThemeBackground) (m_hThemeButton, pDC->GetSafeHdc(), BP_PUSHBUTTON, nState, &rect, 0);
		return TRUE;
	}

	ASSERT_VALID (pDC);

	CRect rectFrame = rect;
	rectFrame.InflateRect (0, 1, 1, 1);

	CBCGPDrawManager dm (*pDC);

	if (pEdit != NULL && pEdit->m_bOnGlass)
	{
		COLORREF clrFrame = globalData.clrBarHilite;
		COLORREF clrFill = m_clrToolBarGradientDark;

		CBCGPDrawManager dm(*pDC);

		switch (state)
		{
		case ButtonsIsPressed:
			clrFrame = m_clrToolBarGradientDark;
			clrFill = m_clrHighlightDnGradientDark;
			break;

		case ButtonsIsHighlighted:
			clrFrame = m_clrToolBarGradientDark;
			clrFill = m_clrHighlightGradientDark;
			break;
		}

		dm.DrawRect(rectFrame, clrFill, clrFrame);
	}
	else
	{
		switch (state)
		{
		case ButtonsIsPressed:
			OnFillHighlightedArea (pDC, rect, &m_brHighlightDn, NULL);
			pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
			break;

		case ButtonsIsHighlighted:
			OnFillHighlightedArea (pDC, rect, &m_brHighlight, NULL);
			pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
			break;

		default:
			dm.FillGradient (rect, m_clrToolBarGradientDark, m_clrToolBarGradientLight);
			break;
		}
	}

	return TRUE;
}
//*********************************************************************************
COLORREF CBCGPVisualManager2003::GetWindowColor () const
{
	return GetThemeColor (m_hThemeWindow, COLOR_WINDOW);
}
//************************************************************************************
void CBCGPVisualManager2003::OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed)
{
	ASSERT_VALID (pDC);

	rectRarelyUsed.left --;
	rectRarelyUsed.right = rectRarelyUsed.left + CBCGPToolBar::GetMenuImageSize ().cx + 
		2 * GetMenuImageMargin () + 2;

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnHighlightRarelyUsedMenuItems (pDC, rectRarelyUsed);
		return;
	}

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rectRarelyUsed, m_clrMenuRarelyUsed, m_clrToolBarGradientDark, FALSE);
}
//*************************************************************************************
void CBCGPVisualManager2003::OnDrawControlBorder (CDC* pDC, CRect rect, CWnd* pWndCtrl, BOOL bDrawOnGlass)
{
	if (m_hThemeComboBox == NULL)
	{
		CBCGPVisualManagerXP::OnDrawControlBorder (pDC, rect, pWndCtrl, bDrawOnGlass);
		return;
	}

	ASSERT_VALID (pDC);

	COLORREF clrBorder = (COLORREF)-1;

	if ((*m_pfGetThemeColor) (m_hThemeComboBox, CP_READONLY, 0, TMT_BORDERCOLOR, &clrBorder) != S_OK)
	{
		CBCGPVisualManagerXP::OnDrawControlBorder (pDC, rect, pWndCtrl, bDrawOnGlass);
		return;
	}

	if (bDrawOnGlass)
	{
		CBCGPDrawManager dm (*pDC);
		dm.DrawRect (rect, (COLORREF)-1, clrBorder);

		rect.DeflateRect (1, 1);

		dm.DrawRect (rect, (COLORREF)-1, globalData.clrWindow);
	}
	else
	{
		pDC->Draw3dRect (&rect, clrBorder, clrBorder);

		rect.DeflateRect (1, 1);
		pDC->Draw3dRect (rect, globalData.clrWindow, globalData.clrWindow);
	}
}
//************************************************************************************
void CBCGPVisualManager2003::OnDrawExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox)
{
	ASSERT_VALID(pDC);

	if (m_hThemeTree == NULL)
	{
		CBCGPVisualManagerXP::OnDrawExpandingBox (pDC, rect, bIsOpened, colorBox);
		return;
	}

	(*m_pfDrawThemeBackground) (m_hThemeTree, pDC->GetSafeHdc(), TVP_GLYPH,
		bIsOpened ? GLPS_OPENED : GLPS_CLOSED, &rect, 0);
}
//******************************************************************************
void CBCGPVisualManager2003::GetSmartDockingBaseMarkerColors (
		COLORREF& clrBaseGroupBackground,
		COLORREF& clrBaseGroupBorder)
{
	if (globalData.m_nBitsPerPixel > 8 && !globalData.IsHighContastMode ())
	{
		clrBaseGroupBackground = RGB (228, 228, 228);
		clrBaseGroupBorder = RGB (181, 181, 181);
	}
	else
	{
		clrBaseGroupBackground = globalData.clrBarFace;
		clrBaseGroupBorder = globalData.clrBarShadow;
	}
}
//******************************************************************************
COLORREF CBCGPVisualManager2003::GetSmartDockingMarkerToneColor ()
{
	if (globalData.m_nBitsPerPixel > 8 && !globalData.IsHighContastMode ())
	{
		WinXpTheme theme = GetStandardWinXPTheme ();

		switch (theme)
		{
		case WinXpTheme_Blue:
			return RGB (61, 123, 241);

		case WinXpTheme_Olive:
			return RGB (190, 146, 109);

		case WinXpTheme_Silver:
			return RGB (134, 130, 169);
		}
	}

	return CBCGPVisualManagerXP::GetSmartDockingMarkerToneColor ();
}
//*****************************************************************************************
void CBCGPVisualManager2003::OnDrawStatusBarSizeBox (CDC* pDC, CBCGPStatusBar* pStatBar,
			CRect rectSizeBox)
{
	if (m_hThemeScrollBar == NULL)
	{
		CBCGPVisualManagerXP::OnDrawStatusBarSizeBox (pDC, pStatBar, rectSizeBox);
		return;
	}

	(*m_pfDrawThemeBackground) (m_hThemeScrollBar, pDC->GetSafeHdc(), SBP_SIZEBOX,
		globalData.m_bIsRTL ? SZB_LEFTALIGN : SZB_RIGHTALIGN, &rectSizeBox, 0);
}
//****************************************************************************************
COLORREF CBCGPVisualManager2003::GetBaseThemeColor ()
{
	return m_bIsStandardWinXPTheme && m_hThemeWindow != NULL ?
		GetThemeColor (m_hThemeWindow, 29) :
		globalData.clrBarFace;
}

#ifndef BCGP_EXCLUDE_TOOLBOX

BOOL CBCGPVisualManager2003::OnEraseToolBoxButton (CDC* pDC, CRect rect,
											CBCGPToolBoxButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pButton->IsHighlighted () || pButton->GetCheck ())
	{
		OnFillHighlightedArea (pDC, rect, 
			pButton->GetCheck () ? &m_brHighlightChecked : &m_brHighlight, NULL);
	}

	return TRUE;
}
//**********************************************************************************
BOOL CBCGPVisualManager2003::OnDrawToolBoxButtonBorder (CDC* pDC, CRect& rect, 
												 CBCGPToolBoxButton* pButton, UINT /*uiState*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pButton->IsHighlighted () || pButton->GetCheck ())
	{
		pDC->Draw3dRect (&rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}

	return TRUE;
}

#endif	// BCGP_EXCLUDE_TOOLBOX

void CBCGPVisualManager2003::OnHighlightQuickCustomizeMenuButton (CDC* pDC, 
	CBCGPToolbarMenuButton* /*pButton*/, CRect rect)
{
	ASSERT_VALID (pDC);

	if (globalData.IsHighContastMode ())
	{
		pDC->FillRect (rect, &m_brBarBkgnd);
	}
	else
	{
		CBrush br (m_clrToolBarGradientLight);
		pDC->FillRect (rect, &br);
	}

	pDC->Draw3dRect (rect, m_clrMenuBorder, m_clrMenuBorder);
}
//****************************************************************************************
void CBCGPVisualManager2003::OnDrawHeaderCtrlBorder (CBCGPHeaderCtrl* pCtrl, CDC* pDC,
		CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted)
{
	if (m_hThemeHeader != NULL)
	{
		int nState = HIS_NORMAL;

		if (bIsPressed)
		{
			nState = HIS_PRESSED;
		}
		else if (bIsHighlighted)
		{
			nState = HIS_HOT;
		}

		(*m_pfDrawThemeBackground) (m_hThemeHeader, pDC->GetSafeHdc(), 
									HP_HEADERITEM, nState, &rect, 0);
		return;
	}

	BOOL bIsVMStyle = (pCtrl == NULL || pCtrl->m_bVisualManagerStyle);
	if (!bIsVMStyle)
	{
		CBCGPVisualManagerXP::OnDrawHeaderCtrlBorder(pCtrl, pDC, rect, bIsPressed, bIsHighlighted);
		return;
	}

	CRect rectButton = rect;

	if (bIsHighlighted)
	{
		OnFillHighlightedArea (pDC, rectButton, bIsPressed ? &m_brHighlightDn : &m_brHighlight, NULL);
		pDC->Draw3dRect (rectButton, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
	else
	{
		pDC->FillRect (rectButton, &m_brBarBkgnd);
		pDC->Draw3dRect (rect, globalData.clrBarShadow, globalData.clrBarShadow);
	}
}
//*****************************************************************************************
void CBCGPVisualManager2003::OnDrawHeaderCtrlSortArrow (CBCGPHeaderCtrl* pCtrl,
												   CDC* pDC,
												   CRect& rect, BOOL bIsUp)
{
	if (m_hThemeHeader == NULL || !globalData.bIsWindows7)
	{
		CBCGPVisualManagerXP::OnDrawHeaderCtrlSortArrow (pCtrl, pDC, rect, bIsUp);
		return;
	}


	int nState = bIsUp ? HSAS_SORTEDUP : HSAS_SORTEDDOWN;

	if ((*m_pfDrawThemeBackground) (m_hThemeHeader, pDC->GetSafeHdc(), 
								HP_HEADERSORTARROW, nState, &rect, 0) != S_OK)
	{
		CBCGPVisualManagerXP::OnDrawHeaderCtrlSortArrow (pCtrl, pDC, rect, bIsUp);
	}
}

#ifndef BCGP_EXCLUDE_POPUP_WINDOW

void CBCGPVisualManager2003::OnFillPopupWindowBackground (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnFillPopupWindowBackground (pDC, rect);
		return;
	}

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, m_clrBarGradientDark, m_clrBarGradientLight);
}
//**********************************************************************************
void CBCGPVisualManager2003::OnDrawPopupWindowBorder (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
}
//**********************************************************************************
void CBCGPVisualManager2003::OnDrawPopupWindowRoundedBorder (CDC* pDC, CRect rect, CBCGPPopupWindow* /*pPopupWnd*/, int nCornerRadius)
{
	ASSERT_VALID (pDC);
	
	CPen pen (PS_SOLID, 1, m_clrMenuItemBorder);
	CPen* pOldPen = pDC->SelectObject (&pen);
	CBrush* pOldBrush = (CBrush*) pDC->SelectStockObject (NULL_BRUSH);
	
	pDC->RoundRect (rect.left, rect.top, rect.right, rect.bottom, nCornerRadius + 1, nCornerRadius + 1);
	
	pDC->SelectObject (pOldPen);
	pDC->SelectObject (pOldBrush);
}
//**********************************************************************************
COLORREF CBCGPVisualManager2003::GetPopupWindowCaptionTextColor(CBCGPPopupWindow* pPopupWnd, BOOL bButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::GetPopupWindowCaptionTextColor(pPopupWnd, bButton);
	}
	
	return globalData.clrBarText;
}
//**********************************************************************************
COLORREF CBCGPVisualManager2003::OnDrawPopupWindowCaption (CDC* pDC, CRect rectCaption, CBCGPPopupWindow* pPopupWnd)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnDrawPopupWindowCaption (pDC, rectCaption, pPopupWnd);
	}

	CBCGPDrawManager dm (*pDC);

	if (pPopupWnd->HasSmallCaption ())
	{
		if (pPopupWnd->IsSmallCaptionGripper())
		{
			dm.FillGradient (rectCaption, m_clrCaptionBarGradientDark, m_clrCaptionBarGradientLight, TRUE);

			CRect rectGripper = rectCaption;

			int xCenter = rectGripper.CenterPoint ().x;
			int yCenter = rectGripper.CenterPoint ().y;

			rectGripper.left = xCenter - 20;
			rectGripper.right = xCenter + 20;

			rectGripper.top = yCenter - 4;
			rectGripper.bottom = yCenter + 2;

			OnDrawBarGripper (pDC, rectGripper, FALSE, NULL);
		}
		else
		{
			pDC->FillSolidRect(rectCaption, m_clrBarGradientLight);
		}
	}
	else
	{
		dm.FillGradient (rectCaption, m_clrToolBarGradientDark, m_clrToolBarGradientLight, TRUE);
	}

	return globalData.clrBarText;
}
//**********************************************************************************
void CBCGPVisualManager2003::OnErasePopupWindowButton (CDC* pDC, CRect rc, CBCGPPopupWndButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnErasePopupWindowButton (pDC, rc, pButton);
		return;
	}

	if (pButton->IsPressed ())
	{
		COLORREF color = m_clrHighlightDnGradientLight == (COLORREF)-1 ?
			m_clrHighlightDn : m_clrHighlightDnGradientLight;
		
		CBrush br (color);
		pDC->FillRect (&rc, &br);
	}
	else if (pButton->IsHighlighted () || pButton->IsPushed ())
	{
		COLORREF color = m_clrHighlightMenuItem == (COLORREF)-1 ?
			m_clrHighlight : m_clrHighlightMenuItem;
		
		CBrush br (color);
		pDC->FillRect (&rc, &br);
	}
	else
	{
		globalData.DrawParentBackground(pButton, pDC);
	}
}
//**********************************************************************************
void CBCGPVisualManager2003::OnDrawPopupWindowButtonBorder (CDC* pDC, CRect rc, CBCGPPopupWndButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pButton->IsHighlighted () || pButton->IsPushed ())
	{
		pDC->Draw3dRect (rc, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
}

#endif	// BCGP_EXCLUDE_POPUP_WINDOW

#ifndef BCGP_EXCLUDE_PLANNER

COLORREF CBCGPVisualManager2003::OnFillPlannerCaption (CDC* pDC, CBCGPPlannerView* pView,
		CRect rect, BOOL bIsToday, BOOL bIsSelected, BOOL bNoBorder/* = FALSE*/, BOOL bHorz /*= TRUE*/)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		!bIsToday)
	{
		return CBCGPVisualManagerXP::OnFillPlannerCaption (pDC, pView, 
			rect, bIsToday, bIsSelected, bNoBorder, bHorz);
	}

	ASSERT_VALID (pDC);

	CBCGPDrawManager dm (*pDC);

	rect.DeflateRect (1, 1);

	dm.FillGradient (rect,
		m_clrPlannerTodayFill, globalData.clrBtnFace, bHorz);

	if (bIsToday)
	{
		CPen* pOldPen = pDC->SelectObject (&m_penPlannerTodayLine);

		pDC->MoveTo (rect.left, rect.bottom);
		pDC->LineTo (rect.right, rect.bottom);

		pDC->SelectObject (pOldPen);
	}

	return globalData.clrBtnText;
}
//*******************************************************************************
COLORREF CBCGPVisualManager2003::GetPlannerViewBackgroundColor (CBCGPPlannerView* pView)
{
	ASSERT_VALID (pView);

	COLORREF colorFill = pView->GetPlanner ()->GetBackgroundColor ();
	
	if (colorFill == CLR_DEFAULT)
	{
		colorFill = m_clrPlannerWork;	// Use default color
	}

	return colorFill;
}
//*******************************************************************************
COLORREF CBCGPVisualManager2003::GetPlannerHourLineColor (CBCGPPlannerView* pView,
		BOOL bWorkingHours, BOOL bHour)
{
	ASSERT_VALID (pView);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::GetPlannerHourLineColor (pView, 
			bWorkingHours, bHour);
	}

	COLORREF colorFill = CLR_DEFAULT;
		
	if (bWorkingHours)
	{
		colorFill = GetPlannerViewNonWorkingColor (pView);
	}
	else
	{
		colorFill = GetPlannerViewWorkingColor (pView);
	}

	if (colorFill == CLR_DEFAULT)
	{
		colorFill = GetPlannerViewBackgroundColor (pView);
	}

	return CalculateHourLineColor (colorFill, bWorkingHours, bHour);
}
//*******************************************************************************
COLORREF CBCGPVisualManager2003::GetPlannerNonWorkingColor (COLORREF clrWorking)
{
	return CalculateNonWorkingColor (clrWorking);
}
//*******************************************************************************
COLORREF CBCGPVisualManager2003::GetPlannerViewWorkingColor (CBCGPPlannerView* pView)
{
	return CalculateWorkingColor (GetPlannerViewBackgroundColor (pView));
}
//*******************************************************************************
COLORREF CBCGPVisualManager2003::GetPlannerViewNonWorkingColor (CBCGPPlannerView* pView)
{
	return GetPlannerNonWorkingColor (GetPlannerViewWorkingColor (pView));
}
//*******************************************************************************
void CBCGPVisualManager2003::OnDrawPlannerTimeLine (CDC* pDC, CBCGPPlannerView* pView, CRect rect)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnDrawPlannerTimeLine (pDC, pView, rect);
		return;
	}

	ASSERT_VALID (pDC);

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, m_clrPlannerTodayFill, globalData.clrBtnFace, TRUE);

	CPen* pOldPen = pDC->SelectObject (&m_penPlannerTodayLine);

	pDC->MoveTo (rect.left, rect.bottom);
	pDC->LineTo (rect.right, rect.bottom);

	pDC->SelectObject (pOldPen);
}
//*******************************************************************************
void CBCGPVisualManager2003::OnFillPlanner (CDC* pDC, CBCGPPlannerView* pView, 
		CRect rect, BOOL bWorkingArea)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pView);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnFillPlanner (pDC, pView, rect, bWorkingArea);
		return;
	}

	COLORREF colorFill = CLR_DEFAULT;
		
	if (bWorkingArea)
	{
		colorFill = GetPlannerViewWorkingColor (pView);
	}
	else
	{
		colorFill = GetPlannerViewNonWorkingColor (pView);
	}

	if (colorFill == CLR_DEFAULT)
	{
		colorFill = GetPlannerViewBackgroundColor (pView);
	}

	CBrush br (colorFill);
	pDC->FillRect (rect, &br);
}

#endif // BCGP_EXCLUDE_PLANNER

#ifndef BCGP_EXCLUDE_GRID_CTRL

COLORREF CBCGPVisualManager2003::GetReportCtrlGroupBackgoundColor ()
{
	return m_clrHighlightMenuItem;
}

void CBCGPVisualManager2003::OnDrawGridSelectionBorder (CBCGPGridCtrl* /*pCtrl*/, CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);
	
	pDC->Draw3dRect (rect, m_clrHighlightGradientDark, m_clrHighlightGradientDark);
	rect.DeflateRect (1, 1);
	pDC->Draw3dRect (rect, m_clrHighlightGradientDark, m_clrHighlightGradientDark);
}

void CBCGPVisualManager2003::OnDrawGridExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnDrawGridExpandingBox (pDC, rect, bIsOpened, colorBox);
		return;
	}

	ASSERT_VALID(pDC);

	if ((rect.Width () % 2) != 0)
	{
		rect.right++;
	}

	if ((rect.Height () % 2) != 0)
	{
		rect.bottom++;
	}

	rect.DeflateRect (1, 1, 0, 0);
	CBCGPDrawManager dm (*pDC);
	dm.FillGradient2 (rect, RGB (255, 255, 255), m_clrToolBarGradientDark, 45);
	rect.InflateRect (1, 1, 0, 0);

	CPen* pOldPen = pDC->SelectObject (&m_penGridExpandBoxLight);
	ASSERT_VALID (pOldPen);

	pDC->MoveTo (rect.left + 1, rect.top);
	pDC->LineTo (rect.right, rect.top);

	pDC->MoveTo (rect.left, rect.top + 1);
	pDC->LineTo (rect.left, rect.bottom);

	pDC->SelectObject (&m_penGridExpandBoxDark);

	pDC->MoveTo (rect.left + 1, rect.bottom);
	pDC->LineTo (rect.right, rect.bottom);

	pDC->MoveTo (rect.right, rect.top + 1);
	pDC->LineTo (rect.right, rect.bottom);

	const int dx = rect.Width () / 2 - 3;
	const int dy = rect.Height () / 2 - 3;

	CPoint ptCenter = rect.CenterPoint ();

	pDC->MoveTo (ptCenter.x - dx, ptCenter.y);
	pDC->LineTo (ptCenter.x + dx + 1, ptCenter.y);

	if (!bIsOpened)
	{
		pDC->MoveTo (ptCenter.x, ptCenter.y - dy);
		pDC->LineTo (ptCenter.x, ptCenter.y + dy + 1);
	}

	pDC->SelectObject (pOldPen);
}
//********************************************************************************
void CBCGPVisualManager2003::OnFillGridHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		m_hThemeHeader == NULL)
	{
		CBCGPVisualManagerXP::OnFillGridHeaderBackground (pCtrl, pDC, rect);
		return;
	}

	ASSERT_VALID(pDC);
	pDC->FillRect (rect, &globalData.brBtnFace);
}
//******************************************************************************
BOOL CBCGPVisualManager2003::OnDrawGridHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		m_hThemeHeader == NULL)
	{
		return CBCGPVisualManagerXP::OnDrawGridHeaderItemBorder (pCtrl, pDC, rect, bPressed);
	}

	ASSERT_VALID (pDC);
	(*m_pfDrawThemeBackground) (m_hThemeHeader, pDC->GetSafeHdc(), 
								HP_HEADERITEM, HIS_NORMAL, &rect, 0);
	return TRUE;
}
//******************************************************************************
void CBCGPVisualManager2003::OnFillGridRowHeaderBackground (CBCGPGridCtrl* /*pCtrl*/, CDC* pDC, CRect rect)
{
	ASSERT_VALID(pDC);
	pDC->FillRect (rect, &globalData.brBtnFace);
}
//******************************************************************************
BOOL CBCGPVisualManager2003::OnDrawGridRowHeaderItemBorder (CBCGPGridCtrl* /*pCtrl*/, CDC* pDC, CRect rect, BOOL /*bPressed*/)
{
 	ASSERT_VALID (pDC);

	CPen* pOldPen = pDC->SelectObject (&globalData.penBarShadow);

	pDC->MoveTo (rect.left, rect.top);
	pDC->LineTo (rect.left, rect.bottom);

	pDC->MoveTo (rect.right - 1, rect.top);
	pDC->LineTo (rect.right - 1, rect.bottom);

	pDC->MoveTo (rect.left, rect.top);
	pDC->LineTo (rect.right - 1, rect.top);

	pDC->SelectObject (pOldPen);

	return TRUE;
}
//******************************************************************************
void CBCGPVisualManager2003::OnFillGridSelectAllAreaBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL /*bPressed*/)
{
	OnFillGridHeaderBackground (pCtrl, pDC, rect);
}
//********************************************************************************
void CBCGPVisualManager2003::OnDrawGridSelectAllMarker(CBCGPGridCtrl* /*pCtrl*/, CDC* pDC, CRect rect, int /*nPadding*/, BOOL bPressed)
{
	ASSERT_VALID(pDC);
	
	POINT ptRgn [] =
	{
		{rect.right, rect.top},
		{rect.right, rect.bottom},
		{rect.left, rect.bottom}
	};
	
	CRgn rgn;
	rgn.CreatePolygonRgn (ptRgn, 3, WINDING);
	
	pDC->SelectClipRgn (&rgn, RGN_COPY);

	if (bPressed)
	{
		pDC->FillSolidRect(rect, m_clrGridSelectAllActive);
	}
	else
	{
		pDC->FillSolidRect(rect, m_clrSeparator);
	}
	
	pDC->SelectClipRgn (NULL, RGN_COPY);
}
//******************************************************************************
BOOL CBCGPVisualManager2003::OnDrawGridSelectAllAreaBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed)
{
	return OnDrawGridHeaderItemBorder (pCtrl, pDC, rect, bPressed);
}
//******************************************************************************
BOOL CBCGPVisualManager2003::OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		m_hThemeHeader == NULL)
	{
		return CBCGPVisualManagerXP::OnSetGridColorTheme (pCtrl, theme);
	}

	theme.m_clrHorzLine                     = m_clrToolBarGradientDark;
	theme.m_clrVertLine                     = m_clrToolBarBottomLine;

	theme.m_EvenColors.m_clrBackground      = m_clrToolBarGradientLight;
	theme.m_EvenColors.m_clrText            = globalData.clrBarText;

	theme.m_OddColors.m_clrBackground       = m_clrToolBarGradientDark;
	theme.m_OddColors.m_clrText             = globalData.clrBarText;

	theme.m_SelColors.m_clrBackground       = m_clrHighlightMenuItem;

	if (GetRValue (m_clrHighlightMenuItem) > 100 &&
		GetGValue (m_clrHighlightMenuItem) > 100 &&
		GetBValue (m_clrHighlightMenuItem) > 100)
	{
		theme.m_SelColors.m_clrText = RGB (0, 0, 0);
	}
	else
	{
		theme.m_SelColors.m_clrText = RGB (255, 255, 255);
	}

	theme.m_GroupColors.m_clrBackground     = m_clrToolBarGradientDark;
	theme.m_GroupColors.m_clrGradient       = m_clrToolBarGradientLight;
	theme.m_GroupColors.m_clrText           = globalData.clrBarText;

	theme.m_GroupSelColors.m_clrBackground  = m_clrHighlightGradientDark;
	theme.m_GroupSelColors.m_clrGradient    = m_clrHighlightGradientLight;
	theme.m_GroupSelColors.m_clrText		= globalData.clrBarText;

	theme.m_LeftOffsetColors.m_clrBackground= m_clrToolBarGradientDark;
	theme.m_LeftOffsetColors.m_clrBorder	   = theme.m_clrHorzLine;

	return TRUE;
}

#endif // BCGP_EXCLUDE_GRID_CTRL

//********************************************************************************

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

//********************************************************************************
void CBCGPVisualManager2003::GetGanttColors (const CBCGPGanttChart* pChart, BCGP_GANTT_CHART_COLORS& colors, COLORREF clrBack) const
{
	CBCGPVisualManagerXP::GetGanttColors (pChart, colors, clrBack);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return;
	}

	colors.clrGridLine0 = CalculateHourLineColor (colors.clrRowDayOff, TRUE, TRUE);
	colors.clrGridLine1 = CalculateHourLineColor (colors.clrRowDayOff, TRUE, FALSE);
	colors.clrSelection = globalData.clrHilite;
	colors.clrSelectionBorder = globalData.clrHilite;
	colors.clrConnectorLines = globalData.clrWindowFrame;
}
//********************************************************************************
void CBCGPVisualManager2003::DrawGanttHeaderCell (const CBCGPGanttChart* pChart, CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, BOOL bHilite)
{
    if (m_hThemeHeader == NULL)
    {
        CBCGPVisualManagerXP::DrawGanttHeaderCell (pChart, dc, cellInfo, bHilite);
        return;
    }

    (*m_pfDrawThemeBackground) (m_hThemeHeader, dc.GetSafeHdc (), HP_HEADERITEM, HIS_NORMAL, cellInfo.rectCell, 0);
}
//********************************************************************************
void CBCGPVisualManager2003::FillGanttBar (const CBCGPGanttItem* pItem, CDC& dc, const CRect& rectFill, COLORREF color, double dGlowLine)
{
    if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
    {
        CBCGPVisualManagerXP::FillGanttBar (pItem, dc, rectFill, color, dGlowLine);
        return;
    }

    dGlowLine = min (1.0, dGlowLine);
    dGlowLine = max (0.0, dGlowLine);
    CRect rectPart = rectFill;
    int h = rectFill.Height ();
    rectPart.bottom = (LONG)(rectFill.top + dGlowLine * h);
    CBCGPDrawManager dm(dc);
    dm.FillGradient (rectPart, CBCGPDrawManager::MixColors (color, RGB(255, 255, 255), 0.3f), CBCGPDrawManager::MixColors (color, RGB(255, 255, 255), 0.15f), TRUE);
    rectPart.top = rectPart.bottom;
    rectPart.bottom = rectFill.bottom;
    dm.FillGradient (rectPart, CBCGPDrawManager::MixColors (color, RGB(0, 0, 0), 0.2f), color, TRUE);
}
//********************************************************************************

#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

//**********************************************************************************
void CBCGPVisualManager2003::OnDrawCheckBoxEx (CDC *pDC, CRect rect, 
										 int nState,
										 BOOL bHighlighted, 
										 BOOL bPressed,
										 BOOL bEnabled)
{
	if (!DrawCheckBox (pDC, rect, bHighlighted, nState, bEnabled, bPressed))
	{
		CBCGPVisualManagerXP::OnDrawCheckBoxEx (pDC, rect, nState, bHighlighted, bPressed, bEnabled);
	}
}
//*************************************************************************************
void CBCGPVisualManager2003::OnDrawRadioButton (CDC *pDC, CRect rect, 
											BOOL bOn,
											BOOL bHighlighted, 
											BOOL bPressed,
											BOOL bEnabled)
{
	if (!DrawRadioButton (pDC, rect, bHighlighted, bOn, bEnabled, bPressed))
	{
		CBCGPVisualManagerXP::OnDrawRadioButton (pDC, rect, bOn, bHighlighted, bPressed, bEnabled);
	}
}
//**********************************************************************************
void CBCGPVisualManager2003::GetCalendarColors (const CBCGPCalendar* pCalendar,
				   CBCGPCalendarColors& colors)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::GetCalendarColors (pCalendar, colors);
		return;
	}

	colors.clrCaption = m_clrBarGradientDark;
	colors.clrCaptionText = globalData.clrBarText;

	if (m_bIsStandardWinXPTheme)
	{
		colors.clrSelected = RGB (251, 230, 148);
	}
	else
	{
		if (m_clrHighlightMenuItem != (COLORREF)-1)
		{
			colors.clrSelected = m_clrHighlightMenuItem;
		}
		else
		{
			colors.clrSelected = m_clrHighlight;
		}
	}

	colors.clrSelectedText = globalData.clrBarText;
	colors.clrTodayBorder = RGB (187, 85, 3);

	if (pCalendar != NULL && pCalendar->IsVisualManagerStyle())
	{
		colors.clrBackground = globalData.clrBarFace;
		colors.clrText = globalData.clrBarText;
		colors.clrLine = m_clrSeparator;
	}

	if (pCalendar->GetSafeHwnd() != NULL && !pCalendar->IsWindowEnabled())
	{
		colors.clrCaptionText = globalData.clrGrayedText;
		colors.clrSelectedText = globalData.clrGrayedText;
		colors.clrText = globalData.clrGrayedText;
		colors.clrLine = globalData.clrBtnLight;
	}
}

#ifndef BCGP_EXCLUDE_PROP_LIST

COLORREF CBCGPVisualManager2003::GetPropListGroupColor (CBCGPPropList* pPropList)
{
	return CBCGPVisualManager::GetPropListGroupColor (pPropList);
}
//********************************************************************************
COLORREF CBCGPVisualManager2003::GetPropListGroupTextColor (CBCGPPropList* pPropList)
{
	return CBCGPVisualManager::GetPropListGroupTextColor (pPropList);
}

#endif

#ifndef BCGP_EXCLUDE_RIBBON

COLORREF CBCGPVisualManager2003::OnDrawRibbonCategoryTab (
					CDC* pDC, 
					CBCGPRibbonTab* pTab,
					BOOL bIsActive)
{
	if (globalData.m_nBitsPerPixel <= 8 || 
		globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnDrawRibbonCategoryTab (
					pDC, pTab, bIsActive);
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pTab);

	CBCGPRibbonCategory* pCategory = pTab->GetParentCategory ();
	ASSERT_VALID (pCategory);
	CBCGPRibbonBar* pBar = pCategory->GetParentRibbonBar ();
	ASSERT_VALID (pBar);

	bIsActive = bIsActive && 
		((pBar->GetHideFlags () & BCGPRIBBONBAR_HIDE_ELEMENTS) == 0 || pTab->GetDroppedDown () != NULL);

	const BOOL bIsFocused	= pTab->IsFocused () && (pBar->GetHideFlags () & BCGPRIBBONBAR_HIDE_ELEMENTS);
	const BOOL bIsHighlighted = (pTab->IsHighlighted () || bIsFocused) && !pTab->IsDroppedDown ();

	CRect rectTab = pTab->GetRect ();
	rectTab.top += 3;

	const int nTrancateRatio = pBar->GetTabTrancateRatio ();

	if (nTrancateRatio > 0)
	{
		CRect rectRight = rectTab;
		rectRight.left = rectRight.right - 1;

		const int nPercent = max (10, 100 - nTrancateRatio / 2);

		COLORREF color1 = CBCGPDrawManager::PixelAlpha (
			globalData.clrBarShadow, nPercent);

		COLORREF color2 = CBCGPDrawManager::PixelAlpha (
			color1, 120);

		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rectRight, color1, color2, TRUE);
	}

	if (!bIsActive && !bIsHighlighted)
	{
		return globalData.clrBarText;
	}

	rectTab.right -= 2;

	CPen pen (PS_SOLID, 1, globalData.clrBarShadow);
	CPen* pOldPen = pDC->SelectObject (&pen);
	ASSERT (pOldPen != NULL);

	#define RIBBONTAB_POINTS_NUM	8
	POINT pts [RIBBONTAB_POINTS_NUM];

	pts [0] = CPoint (rectTab.left, rectTab.bottom);
	pts [1] = CPoint (rectTab.left + 1, rectTab.bottom - 1);
	pts [2] = CPoint (rectTab.left + 1, rectTab.top + 2);
	pts [3] = CPoint (rectTab.left + 3, rectTab.top);
	pts [4] = CPoint (rectTab.right - 3, rectTab.top);
	pts [5] = CPoint (rectTab.right - 1, rectTab.top + 2);
	pts [6] = CPoint (rectTab.right - 1, rectTab.bottom - 1);
	pts [7] = CPoint (rectTab.right, rectTab.bottom);

	CRgn rgnClip;
	rgnClip.CreatePolygonRgn (pts, RIBBONTAB_POINTS_NUM, WINDING);

	pDC->SelectClipRgn (&rgnClip);

	CBCGPDrawManager dm (*pDC);

	const BOOL bIsSelected = pTab->IsSelected ();

	COLORREF clrFill = bIsSelected ? m_clrHighlightGradientDark : RibbonCategoryColorToRGB (pCategory->GetTabColor ());

	COLORREF clr1 = globalData.clrBarFace;
	COLORREF clr2 = (clrFill == (COLORREF)-1) ? 
		CBCGPDrawManager::PixelAlpha (clr1, 120) : clrFill;

	if (bIsHighlighted)
	{
		if (bIsActive)
		{
			clr2 = m_clrHighlightGradientLight;
		}
		else
		{
			if (clrFill == (COLORREF)-1)
			{
				clr1 = m_clrHighlightGradientDark;
				clr2 = m_clrHighlightGradientLight;
			}
			else
			{
				clr1 = clrFill;
				clr2 = CBCGPDrawManager::PixelAlpha (clr1, 120);
			}
		}
	}

	dm.FillGradient (rectTab, clr1, clr2, TRUE);

	pDC->SelectClipRgn (NULL);

	pDC->Polyline (pts, RIBBONTAB_POINTS_NUM);

	if (bIsHighlighted && bIsActive && !bIsSelected)
	{
		//---------------------
		// Draw internal frame:
		//---------------------
		const CPoint ptCenter = rectTab.CenterPoint ();

		for (int i = 0; i < RIBBONTAB_POINTS_NUM; i++)
		{
			if (pts [i].x < ptCenter.x)
			{
				pts [i].x++;
			}
			else
			{
				pts [i].x--;
			}

			if (pts [i].y < ptCenter.y)
			{
				pts [i].y++;
			}
			else
			{
				pts [i].y--;
			}
		}

		CPen penInternal (PS_SOLID, 1, m_clrHighlightGradientDark);
		pDC->SelectObject (&penInternal);

		pDC->Polyline (pts, RIBBONTAB_POINTS_NUM);
		pDC->SelectObject (pOldPen);
	}

	pDC->SelectObject (pOldPen);

	return globalData.clrBarText;
}
//*******************************************************************************
COLORREF CBCGPVisualManager2003::OnDrawRibbonButtonsGroup (
					CDC* pDC, CBCGPRibbonButtonsGroup* pGroup,
					CRect rect)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pGroup);

	if (DYNAMIC_DOWNCAST (CBCGPRibbonQuickAccessToolbar, pGroup) != NULL ||
		pGroup->GetCount () == 0)
	{
		return (COLORREF)-1;
	}

	CBCGPBaseRibbonElement* pButton = pGroup->GetButton (0);
	ASSERT_VALID (pButton);

	if (!pButton->IsShowGroupBorder ())
	{
		return (COLORREF)-1;
	}

	const int dx = 2;
	const int dy = 2;

	CPen pen (PS_SOLID, 1, m_clrToolBarGradientDark);
	CPen* pOldPen = pDC->SelectObject (&pen);
	ASSERT (pOldPen != NULL);

	CBrush* pOldBrush = (CBrush*) pDC->SelectStockObject (NULL_BRUSH);
	ASSERT (pOldBrush != NULL);

	rect.DeflateRect (1, 1);
	pDC->RoundRect (rect, CPoint (dx, dy));

	pDC->SelectObject (pOldPen);
	pDC->SelectObject (pOldBrush);

	return (COLORREF)-1;
}
//***********************************************************************************
COLORREF CBCGPVisualManager2003::OnDrawRibbonCategoryCaption (
					CDC* pDC, 
					CBCGPRibbonContextCaption* pContextCaption)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pContextCaption);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnDrawRibbonCategoryCaption (
					pDC, pContextCaption);
	}

	COLORREF clrFill = RibbonCategoryColorToRGB (pContextCaption->GetColor ());
	CRect rect = pContextCaption->GetRect ();
	
	if (clrFill != (COLORREF)-1)
	{
		if (CBCGPToolBarImages::m_bIsDrawOnGlass)
		{
			rect.DeflateRect (0, 1);
		}
		
		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rect, clrFill, globalData.clrBarFace, TRUE);
	}

	return globalData.clrBarText;
}
//***********************************************************************************
void CBCGPVisualManager2003::GetRibbonSliderColors (CBCGPRibbonSlider* pSlider, 
					BOOL bIsHighlighted, 
					BOOL bIsPressed,
					BOOL bIsDisabled,
					COLORREF& clrLine,
					COLORREF& clrFill)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::GetRibbonSliderColors(pSlider, bIsHighlighted, bIsPressed, bIsDisabled, clrLine, clrFill);
		return;
	}

	clrLine = (bIsPressed || bIsHighlighted) ? globalData.clrBarDkShadow : globalData.clrBtnDkShadow;

	if (bIsPressed || bIsHighlighted)
	{
		clrFill = bIsPressed ? m_clrHighlightDnGradientLight : m_clrHighlightDnGradientDark;
	}
	else
	{
		clrFill = globalData.clrBarFace;
	}
}
//***********************************************************************************
COLORREF CBCGPVisualManager2003::OnDrawRibbonStatusBarPane (CDC* pDC, CBCGPRibbonStatusBar* pBar,
					CBCGPRibbonStatusBarPane* pPane)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pPane);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		m_hThemeStatusBar == NULL)
	{
		return CBCGPVisualManagerXP::OnDrawRibbonStatusBarPane (
					pDC, pBar, pPane);
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

	return (COLORREF)-1;
}
//**************************************************************************************
void CBCGPVisualManager2003::OnDrawRibbonProgressBar (CDC* pDC, 
												  CBCGPRibbonProgressBar* pProgress, 
												  CRect rectProgress, CRect rectChunk,
												  BOOL bInfiniteMode)
{
	if (pProgress == NULL || !DrawProgressBar(pDC, rectProgress, rectChunk, bInfiniteMode, pProgress->IsVertical(),
		pProgress->GetPos (), pProgress->GetRangeMin (), pProgress->GetRangeMax ()))
	{
		CBCGPVisualManagerXP::OnDrawRibbonProgressBar (pDC, pProgress, rectProgress, rectChunk, bInfiniteMode);
	}
}
//********************************************************************************
void CBCGPVisualManager2003::OnDrawRibbonQATSeparator (CDC* pDC, 
												   CBCGPRibbonSeparator* /*pSeparator*/, CRect rect)
{
	ASSERT_VALID (pDC);

	int x = rect.CenterPoint ().x;

	if (CBCGPToolBarImages::m_bIsDrawOnGlass)
	{
		CBCGPDrawManager dm (*pDC);
		dm.DrawLine (x, rect.top, x, rect.bottom - 1, globalData.clrBarDkShadow);
		dm.DrawLine (x + 1, rect.top + 1, x + 1, rect.bottom, globalData.clrBarLight);
	}
	else
	{
		CPen* pOldPen = pDC->SelectObject (&m_penSeparator);
		ASSERT (pOldPen != NULL);

		pDC->MoveTo (x, rect.top);
		pDC->LineTo (x, rect.bottom - 1);

		pDC->SelectObject (&m_penSeparatorLight);

		pDC->MoveTo (x + 1, rect.top + 1);
		pDC->LineTo (x + 1, rect.bottom);

		pDC->SelectObject (pOldPen);
	}
}
//********************************************************************************
COLORREF CBCGPVisualManager2003::OnFillRibbonPanelCaption (CDC* pDC, CBCGPRibbonPanel* pPanel, CRect rectCaption)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pPanel);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnFillRibbonPanelCaption (pDC, pPanel, rectCaption);
	}

	CBrush br (!pPanel->IsHighlighted () ? 
		m_clrBarGradientDark : 
		CBCGPDrawManager::PixelAlpha (m_clrBarGradientDark, 108));

	pDC->FillRect (rectCaption, &br);
	return globalData.clrBarText;
}

#endif // BCGP_EXCLUDE_RIBBON

BOOL CBCGPVisualManager2003::GetToolTipParams (CBCGPToolTipParams& params, 
											   UINT /*nType*/ /*= (UINT)(-1)*/)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::GetToolTipParams (params);
	}

	params.m_bBoldLabel = TRUE;
	params.m_bDrawDescription = TRUE;
	params.m_bDrawIcon = TRUE;
	params.m_bRoundedCorners = TRUE;
	params.m_bDrawSeparator = FALSE;

	params.m_clrFill = globalData.clrBarHilite;
	params.m_clrFillGradient = globalData.clrBarFace;
	params.m_clrText = globalData.clrBarText;
	params.m_clrBorder = globalData.clrBarShadow;

	return TRUE;
}
//********************************************************************************
COLORREF CBCGPVisualManager2003::GetHighlightedColor(UINT nType)
{
	return nType == 0 ? m_clrHighlightGradientDark : m_clrHighlightDnGradientDark;
}

#ifndef BCGP_EXCLUDE_PROP_LIST

COLORREF CBCGPVisualManager2003::OnFillPropertyListSelectedItem(CDC* pDC, CBCGPProp* pProp, CBCGPPropList* pWndList, const CRect& rectFill, BOOL bFocused)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID(pProp);

	if (!pWndList->DrawControlBarColors() || globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnFillPropertyListSelectedItem(pDC, pProp, pWndList, rectFill, bFocused);
	}

	if (!bFocused)
	{
		pDC->FillRect (rectFill, &globalData.brBarFace);
		return pProp->IsEnabled() ? globalData.clrBarText : globalData.clrGrayedText;
	}

	OnFillHighlightedArea (pDC, rectFill, &m_brHighlight, NULL);

	if (!pProp->IsEnabled())
	{
		return globalData.clrGrayedText;
	}

	CBCGPToolbarMenuButton dummy;
	return GetHighlightedMenuItemTextColor (&dummy);
}

#endif
