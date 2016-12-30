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
//
// BCGPVisualManager2013.cpp: implementation of the CBCGPVisualManager2013 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPVisualManager2013.h"
#include "BCGGlobals.h"
#include "BCGPToolbar.h"
#include "BCGPToolbarButton.h"
#include "MenuImages.h"
#include "BCGPDrawManager.h"
#include "BCGPCaptionButton.h"
#include "BCGPMiniFrameWnd.h"
#include "BCGPSlider.h"
#include "BCGPPopupMenuBar.h"
#include "BCGPTabWnd.h"
#include "BCGPGridCtrl.h"
#include "BCGPToolTipCtrl.h"
#include "BCGPToolBox.h"
#include "BCGPAutoHideButton.h"
#include "BCGPReBar.h"
#include "BCGPRibbonPaletteButton.h"

#ifndef BCGP_EXCLUDE_PLANNER
#include "BCGPPlannerViewDay.h"
#include "BCGPPlannerViewMonth.h"
#include "BCGPPlannerViewMulti.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CBCGPVisualManager2013, CBCGPVisualManagerVS2012)

CBCGPVisualManager2013::Style CBCGPVisualManager2013::m_Style = CBCGPVisualManager2013::Office2013_White;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPVisualManager2013::CBCGPVisualManager2013()
{
	CBCGPVisualManagerVS2012::m_Style = VS2012_Light;

	m_bEmbossGripper = FALSE;
	m_bCheckedRibbonButtonFrame = FALSE;
}
//*********************************************************************************************************
CBCGPVisualManager2013::~CBCGPVisualManager2013()
{
	CBCGPMenuImages::SetColor (CBCGPMenuImages::ImageBlack, (COLORREF)-1);
	CBCGPMenuImages::SetColor (CBCGPMenuImages::ImageBlack2, (COLORREF)-1);
}
//**************************************************************************************************************
void CBCGPVisualManager2013::SetStyle(Style style)
{
	m_Style = style;

	CBCGPVisualManager2013* pThis = DYNAMIC_DOWNCAST(CBCGPVisualManager2013, CBCGPVisualManager::GetInstance ());
	if (pThis != NULL)
	{
		pThis->OnUpdateSystemColors();
	}
}
//*********************************************************************************************************
void CBCGPVisualManager2013::ModifyGlobalColors ()
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::ModifyGlobalColors();
		return;
	}

	if (m_Style == Office2013_White)
	{
		globalData.clrBarFace = RGB (255, 255, 255);
		globalData.clrBarText = RGB (59, 59, 59);
		globalData.clrBarShadow = RGB (238, 238, 238);
		globalData.clrBarHilite = RGB (255, 255, 255);
		globalData.clrBarDkShadow = RGB(162, 162, 162);
		globalData.clrBarLight = RGB (255, 255, 255);
	}
	else if (m_Style == Office2013_Gray)
	{
		globalData.clrBarFace = RGB (241, 241, 241);
		globalData.clrBarText = RGB (30, 30, 30);
		globalData.clrBarShadow = RGB (230, 230, 230);
		globalData.clrBarHilite = RGB (255, 255, 255);
		globalData.clrBarDkShadow = RGB(198, 198, 198);
		globalData.clrBarLight = RGB (250, 250, 250);
	}
	else  // Office2013_DarkGray
	{
		globalData.clrBarFace = RGB (229, 229, 229);
		globalData.clrBarText = RGB (30, 30, 30);
		globalData.clrBarShadow = RGB (224, 224, 224);
		globalData.clrBarHilite = RGB (255, 255, 255);
		globalData.clrBarDkShadow = RGB(171, 171, 171);
		globalData.clrBarLight = RGB (243, 243, 243);
	}

	globalData.brBarFace.DeleteObject ();
	globalData.brBarFace.CreateSolidBrush (globalData.clrBarFace);
}
//************************************************************************************
void CBCGPVisualManager2013::SetupColors()
{
	switch (m_curAccentColor)
	{
	case VS2012_Blue:
		m_clrAccent = RGB(43, 87, 154);	// Word
		break;
		
	case VS2012_Brown:
		m_clrAccent = RGB(164, 55, 58);	// Access
		break;
		
	case VS2012_Green:
		m_clrAccent = RGB(33, 115, 70);	// Excel
		break;
		
	case VS2012_Lime:
		m_clrAccent = RGB(137, 164, 48);
		break;
		
	case VS2012_Magenta:
		m_clrAccent = RGB(216,0,115);
		break;
		
	case VS2012_Orange:
		m_clrAccent = RGB(210, 71, 38);	// Power point
		break;
		
	case VS2012_Pink:
		m_clrAccent = RGB(230,113,184);
		break;
		
	case VS2012_Purple:
		m_clrAccent = RGB(128, 57, 123); // OneNote
		break;
		
	case VS2012_Red:
		m_clrAccent = RGB(229,20,0);
		break;
		
	case VS2012_Teal:
		m_clrAccent = RGB(7, 117, 104);	// Publisher
		break;
	}

//	m_clrAccent = RGB(0, 114, 198);	// Outlook ????

	CBCGPVisualManagerVS2012::SetupColors();

	m_clrMenuLight = RGB(255, 255, 255);

	m_clrAccentLight = CBCGPDrawManager::ColorMakePale(m_clrAccent, .9);

	m_clrHighlight = m_clrAccentLight;
	m_clrHighlightMenuItem = m_clrAccentLight;
	m_clrHighlightDn = CBCGPDrawManager::ColorMakeDarker(m_clrAccentLight, .14);
	m_clrHighlightChecked = CBCGPDrawManager::ColorMakeDarker(m_clrAccentLight, .05);
	
	m_clrHighlightGradientLight = m_clrAccentLight;
	m_clrHighlightGradientDark = m_clrAccentLight;
	
	m_clrHighlightDnGradientDark = m_clrHighlightDn;
	m_clrHighlightDnGradientLight =  m_clrHighlightDn;

	m_clrEditBoxBorder = RGB(171, 171, 171);

	switch (m_Style)
	{
	case Office2013_White:
		m_clrSeparator = m_clrMenuSeparator = RGB(212, 212, 212);
		break;

	case Office2013_Gray:
		m_clrSeparator = m_clrMenuSeparator = RGB(198, 198, 198);
		break;

	case Office2013_DarkGray:
		m_clrSeparator = m_clrMenuSeparator = RGB(171, 171, 171);
		break;
	}

	CBCGPMenuImages::SetColor (CBCGPMenuImages::ImageBlack, RGB(119, 119, 119));
	CBCGPMenuImages::SetColor (CBCGPMenuImages::ImageBlack2, m_clrAccentText);

	m_clrRibbonTabs = RGB(102, 102, 102);

	m_clrMenuBorder = CBCGPDrawManager::ColorMakeLighter(globalData.clrBarDkShadow);

	m_clrEditCtrlSelectionBkActive = CBCGPDrawManager::ColorMakePale(m_clrAccent, .85);
	m_clrEditCtrlSelectionBkInactive = CBCGPDrawManager::ColorMakePale(m_clrAccent, .95);

	m_clrTabsBackground = globalData.clrBarFace;
	m_clrGripper = globalData.clrBarDkShadow;

	m_clrReportGroupText = globalData.clrBarText;
	m_clrHighlighDownText = globalData.clrBarText;

	m_clrRibbonCategoryFill = globalData.clrBarLight;
	m_clrCombo = globalData.clrBarHilite;
	m_clrControl = globalData.clrBarHilite;
	m_clrDlgBackground = globalData.clrBarLight;

	m_clrPalennerLine     = m_clrSeparator;
	m_clrPlannerTodayLine = m_clrPlannerTodayFill;

	m_clrNcTextActive   = RGB(68, 68, 68);
	m_clrNcTextInactive = RGB(177, 177, 177);
	m_clrNcBorder       = RGB(131, 131, 131);

	if (m_Style == Office2013_DarkGray)
	{
		m_clrNcTextInactive = RGB(128, 128, 128);
	}

	m_clrButtonsArea = CBCGPDrawManager::ColorMakeDarker(m_clrDlgBackground, .02);

	m_clrFace = m_clrButtonsArea;
	m_clrHighlightNC = m_clrHighlight;
}
//************************************************************************************
COLORREF CBCGPVisualManager2013::OnDrawMenuLabel (CDC* pDC, CRect rect)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::OnDrawMenuLabel(pDC, rect);
	}

	ASSERT_VALID(pDC);

	CBrush brFill(globalData.clrBarShadow);
	pDC->FillRect(rect, &brFill);

	return globalData.clrBarText;
}
//**************************************************************************************
void CBCGPVisualManager2013::OnDrawMenuImageRectBorder (CDC* pDC, CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnDrawMenuImageRectBorder(pDC, pButton, rect, state);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if ((pButton->m_nStyle & TBBS_CHECKED) && (pButton->GetImage() >= 0 || pButton->IsRibbonImage()))
	{
		if (!CBCGPToolBar::IsCustomizeMode () && (pButton->m_nStyle & TBBS_DISABLED))
		{
			pDC->Draw3dRect(rect, globalData.clrBarShadow, globalData.clrBarShadow);
		}
		else
		{
			pDC->Draw3dRect(rect, m_clrHighlightDn, m_clrHighlightDn);
		}
	}
}
//****************************************************************************************
void CBCGPVisualManager2013::OnFillMenuImageRect (CDC* pDC, CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnFillMenuImageRect(pDC, pButton, rect, state);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if ((pButton->m_nStyle & TBBS_CHECKED) && (pButton->GetImage() >= 0 || pButton->IsRibbonImage()))
	{
		pDC->FillRect(rect, &m_brHighlight);
	}
}
//*************************************************************************************
void CBCGPVisualManager2013::OnScrollBarDrawThumb (CDC* pDC, CBCGPScrollBar* /*pScrollBar*/, CRect rect, 
		BOOL /*bHorz*/, BOOL bHighlighted, BOOL bPressed, BOOL bDisabled)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return;
	}

	ASSERT_VALID(pDC);

	CBCGPDrawManager dm (*pDC);
	dm.DrawRect (rect, 
		bPressed || bDisabled ? globalData.clrBarShadow : globalData.clrBarLight, 
		bHighlighted ? globalData.clrBarDkShadow : (m_Style == Office2013_White ? m_clrSeparator : m_clrMenuBorder));
}
//*************************************************************************************
void CBCGPVisualManager2013::OnScrollBarDrawButton (CDC* pDC, CBCGPScrollBar* pScrollBar, CRect rect, 
		BOOL bHorz, BOOL bHighlighted, BOOL bPressed, BOOL bFirst, BOOL bDisabled)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return;
	}

	COLORREF clrFill = bPressed || bDisabled ? globalData.clrBarShadow : globalData.clrBarLight;

	CBCGPDrawManager dm (*pDC);
	dm.DrawRect (rect, 
		clrFill, 
		bDisabled ? (COLORREF)-1 : bHighlighted ? globalData.clrBarDkShadow : (m_Style == Office2013_White ? m_clrSeparator : m_clrMenuBorder));

	CBCGPMenuImages::IMAGES_IDS ids;
	if (bHorz)
	{
		if (pScrollBar->GetSafeHwnd() != NULL && (pScrollBar->GetExStyle() & WS_EX_LAYOUTRTL))
		{
			bFirst = !bFirst;
		}
		
		ids = bFirst ? CBCGPMenuImages::IdArowLeftTab3d : CBCGPMenuImages::IdArowRightTab3d;
	}
	else
	{
		ids = bFirst ? CBCGPMenuImages::IdArowUpLarge : CBCGPMenuImages::IdArowDownLarge;
	}

	CBCGPMenuImages::IMAGE_STATE state = bDisabled ? CBCGPMenuImages::ImageLtGray : CBCGPMenuImages::GetStateByColor(clrFill);

	CBCGPMenuImages::Draw (pDC, ids, rect, state);
}
//*************************************************************************************
void CBCGPVisualManager2013::OnScrollBarFillBackground (CDC* pDC, CBCGPScrollBar* /*pScrollBar*/, CRect rect, 
		BOOL /*bHorz*/, BOOL /*bHighlighted*/, BOOL /*bPressed*/, BOOL /*bFirst*/, BOOL /*bDisabled*/)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return;
	}

	CBCGPDrawManager dm (*pDC);
	dm.DrawRect (rect, globalData.clrBarShadow, (COLORREF)-1);
}
//*********************************************************************************************************
COLORREF CBCGPVisualManager2013::OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
			BOOL bActive, CRect rectCaption, CRect rectButtons)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::OnDrawControlBarCaption (pDC, pBar, bActive, rectCaption, rectButtons);
	}

	ASSERT_VALID (pDC);

	rectCaption.bottom++;

	pDC->FillRect(rectCaption, &globalData.brBarFace);

	return bActive ? m_clrAccentText : globalData.clrBarText;
}
//****************************************************************************************
void CBCGPVisualManager2013::OnDrawControlBarCaptionText (CDC* pDC, CBCGPDockingControlBar* pBar, BOOL bActive, const CString& strTitle, CRect& rectCaption)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnDrawControlBarCaptionText(pDC, pBar, bActive, strTitle, rectCaption);
		return;
	}

	ASSERT_VALID (pDC);

	CRect rectText = rectCaption;
	pDC->DrawText (strTitle, rectText, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
}
//*********************************************************************************************************
void CBCGPVisualManager2013::OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CBCGPBaseControlBar* pBar)
{
//	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2010::OnDrawBarGripper(pDC, rectGripper, bHorz, pBar);
	}
}
//**************************************************************************************
void CBCGPVisualManager2013::OnDrawCaptionButton (CDC* pDC, CBCGPCaptionButton* pButton, 
								BOOL bActive, BOOL bHorz, BOOL bMaximized, BOOL bDisabled,
								int nImageID /*= -1*/)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnDrawCaptionButton (pDC, pButton, bActive, bHorz, bMaximized, bDisabled, nImageID);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

    CRect rc = pButton->GetRect ();

	const BOOL bHighlight = (pButton->m_bFocused || pButton->m_bDroppedDown) && !bDisabled;
	
	if (pButton->m_bPushed && bHighlight)
	{
		pDC->FillRect (rc, &m_brHighlightDn);
	}
	else if (bHighlight || pButton->m_bPushed)
	{
		pDC->FillRect (rc, &m_brHighlight);
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
}
//**********************************************************************************
void CBCGPVisualManager2013::OnDrawCaptionButtonIcon (CDC* pDC, 
													CBCGPCaptionButton* pButton,
													CBCGPMenuImages::IMAGES_IDS id,
													BOOL bActive, BOOL bDisabled,
													CPoint ptImage)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnDrawCaptionButtonIcon (pDC, pButton, id, bActive, bDisabled, ptImage);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	CBCGPMenuImages::IMAGE_STATE imageState = CBCGPMenuImages::ImageBlack;
	
	if (bDisabled)
	{
		imageState = CBCGPMenuImages::ImageGray;
	}
	else if (pButton->m_bFocused || pButton->m_bPushed || pButton->m_bDroppedDown)
	{
		imageState = CBCGPMenuImages::ImageBlack2;
	}

	CBCGPMenuImages::Draw (pDC, id, ptImage, imageState);
}
//*****************************************************************************
COLORREF CBCGPVisualManager2013::GetHighlightedMenuItemTextColor (CBCGPToolbarMenuButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManager2003::GetHighlightedMenuItemTextColor (pButton);
	}

	return globalData.clrBarText;
}
//**************************************************************************************
COLORREF CBCGPVisualManager2013::GetToolbarButtonTextColor (CBCGPToolbarButton* pButton, CBCGPVisualManager::BCGBUTTON_STATE state)
{
	ASSERT_VALID (pButton);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::GetToolbarButtonTextColor (pButton, state);
	}

	if (pButton->GetParentWnd() != NULL && state == ButtonsIsRegular)
	{
		if (DYNAMIC_DOWNCAST (CBCGPReBar, pButton->GetParentWnd()) != NULL || DYNAMIC_DOWNCAST (CBCGPReBar, pButton->GetParentWnd()->GetParent ()))
		{
			return CBCGPVisualManagerVS2010::GetToolbarButtonTextColor (pButton, state);
		}
	}

	BOOL bDisabled = (CBCGPToolBar::IsCustomizeMode () && !pButton->IsEditable ()) ||
			(!CBCGPToolBar::IsCustomizeMode () && (pButton->m_nStyle & TBBS_DISABLED));

	CBCGPControlBar* pWnd = DYNAMIC_DOWNCAST(CBCGPControlBar, pButton->GetParentWnd());
	if (pWnd != NULL && pWnd->IsDialogControl())
	{
		return bDisabled ? globalData.clrGrayedText : globalData.clrBtnText;
	}

	return bDisabled ? m_clrTextDisabled : globalData.clrBarText;
}
//*****************************************************************************
int CBCGPVisualManager2013::GetMenuDownArrowState (CBCGPToolbarMenuButton* pButton, BOOL bHightlight, BOOL bPressed, BOOL bDisabled)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::GetMenuDownArrowState (pButton, bHightlight, bPressed, bDisabled);
	}

	CBCGPWnd* pWnd = DYNAMIC_DOWNCAST(CBCGPWnd, pButton->GetParentWnd());

	if (pWnd != NULL && pWnd->IsOnGlass())
	{
		return CBCGPVisualManagerVS2010::GetMenuDownArrowState (pButton, bHightlight, bPressed, bDisabled);
	}

	if (bDisabled)
	{
		return (int)CBCGPMenuImages::ImageLtGray;
	}

	return (int)CBCGPMenuImages::ImageBlack;
}
//*********************************************************************************************************
void CBCGPVisualManager2013::OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2010::OnHighlightRarelyUsedMenuItems (pDC, rectRarelyUsed);
		return;
	}

	rectRarelyUsed.left --;
	rectRarelyUsed.right = rectRarelyUsed.left + CBCGPToolBar::GetMenuImageSize ().cx + 
		2 * GetMenuImageMargin () + 2;

	pDC->FillSolidRect(rectRarelyUsed, globalData.clrBarShadow);
}
//************************************************************************************
COLORREF CBCGPVisualManager2013::OnFillMiniFrameCaption (CDC* pDC, 
								CRect rectCaption, 
								CBCGPMiniFrameWnd* pFrameWnd,
								BOOL bActive)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pFrameWnd);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::OnFillMiniFrameCaption (pDC, rectCaption, pFrameWnd, bActive);
	}

	if (DYNAMIC_DOWNCAST (CBCGPBaseToolBar, pFrameWnd->GetControlBar ()) != NULL)
	{
		bActive = FALSE;
	}

	pDC->FillRect(rectCaption, &globalData.brBarFace);

	return bActive ? m_clrAccentText : globalData.clrBarText;
}
//************************************************************************************
void CBCGPVisualManager2013::OnDrawMiniFrameBorder (
										CDC* pDC, CBCGPMiniFrameWnd* pFrameWnd,
										CRect rectBorder, CRect rectBorderSize)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnDrawMiniFrameBorder (pDC, pFrameWnd, rectBorder, rectBorderSize);
		return;
	}

	ASSERT_VALID (pDC);

	CRect rectClip = rectBorder;
	rectClip.DeflateRect(rectClip);

	pDC->ExcludeClipRect(rectClip);

	pDC->FillRect(rectBorder, &globalData.brBarFace);

	if (pFrameWnd->GetSafeHwnd() != NULL && pFrameWnd->IsActive())
	{
		pDC->Draw3dRect(rectBorder, m_clrAccent, m_clrAccent);
	}
	else
	{
		pDC->Draw3dRect(rectBorder, m_clrMenuBorder, m_clrMenuBorder);
	}

	pDC->SelectClipRgn (NULL);
}
//*******************************************************************************
COLORREF CBCGPVisualManager2013::OnFillListBox(CDC* pDC, CBCGPListBox* pListBox, CRect rectClient)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::OnFillListBox(pDC, pListBox, rectClient);
	}

	ASSERT_VALID (pDC);

	pDC->FillRect(rectClient, &m_brControl);
	return globalData.clrBarText;
}
//*******************************************************************************
COLORREF CBCGPVisualManager2013::OnFillListBoxItem (CDC* pDC, CBCGPListBox* pListBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::OnFillListBoxItem (pDC, pListBox, nItem, rect, bIsHighlihted, bIsSelected);
	}

	ASSERT_VALID (pDC);

	COLORREF clrText = (COLORREF)-1;

	if (bIsHighlihted)
	{
		pDC->FillRect(rect, bIsSelected ? &m_brHighlightChecked : &m_brHighlight);
	}
	else if (bIsSelected)
	{
		pDC->FillRect(rect, &m_brHighlightDn);
	}

	return clrText;
}
//*******************************************************************************
COLORREF CBCGPVisualManager2013::OnFillComboBoxItem (CDC* pDC, CBCGPComboBox* pComboBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::OnFillComboBoxItem (pDC, pComboBox, nItem, rect, bIsHighlihted, bIsSelected);
	}

	ASSERT_VALID (pDC);

	if (!bIsHighlihted && !bIsSelected)
	{
		pDC->FillRect(rect, &m_brControl);
		return globalData.clrBarText;
	}

	COLORREF clrText = (COLORREF)-1;

	if (bIsHighlihted)
	{
		pDC->FillRect(rect, &m_brHighlight);
	}
	else if (bIsSelected)
	{
		pDC->FillRect(rect, &m_brHighlightDn);
	}

	return clrText;
}
//*************************************************************************************
void CBCGPVisualManager2013::OnDrawComboBorder (CDC* pDC, CRect rect,
												BOOL bDisabled,
												BOOL bIsDropped,
												BOOL bIsHighlighted,
												CBCGPToolbarComboBoxButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnDrawComboBorder (pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}
	
	rect.DeflateRect (1, 1);
	
	COLORREF clrBorder = bDisabled ? m_clrEditBoxBorderDisabled : m_clrEditBoxBorder;
	
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
//*****************************************************************************
void CBCGPVisualManager2013::OnDrawComboDropButton (CDC* pDC, CRect rect,
												BOOL bDisabled,
												BOOL bIsDropped,
												BOOL bIsHighlighted,
												CBCGPToolbarComboBoxButton* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID (this);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || CBCGPToolBarImages::m_bIsDrawOnGlass || bDisabled)
	{
		CBCGPVisualManagerVS2012::OnDrawComboDropButton (pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}

	COLORREF clrFill = m_clrCombo;

	if (bIsDropped)
	{
		pDC->FillRect(rect, &m_brHighlightDn);
		clrFill = m_clrHighlightDn;
	}
	else if (bIsHighlighted)
	{
		pDC->FillRect (rect, &m_brAccentLight);
		clrFill = m_clrAccentLight;
	}
	else
	{
		pDC->FillSolidRect (rect, m_clrCombo);
		clrFill = m_clrCombo;
	}

	CBCGPMenuImages::DrawByColor(pDC, CBCGPMenuImages::IdArowDown, rect, clrFill);
}
//*************************************************************************************
void CBCGPVisualManager2013::OnDrawEditBorder (CDC* pDC, CRect rect,
												BOOL bDisabled,
												BOOL bIsHighlighted,
												CBCGPToolbarEditBoxButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnDrawEditBorder (pDC, rect, bDisabled, bIsHighlighted, pButton);
		return;
	}
	
	COLORREF clrBorder = bDisabled ? m_clrEditBoxBorderDisabled : m_clrEditBoxBorder;
	
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
void CBCGPVisualManager2013::OnDrawButtonBorder (CDC* pDC, CBCGPToolbarButton* pButton, CRect rect, BCGBUTTON_STATE state)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnDrawButtonBorder (pDC, pButton, rect, state);
		return;
	}

	CBCGPControlBar* pWnd = DYNAMIC_DOWNCAST(CBCGPControlBar, pButton->GetParentWnd());
	if (pWnd != NULL && pWnd->IsDialogControl())
	{
		CBCGPVisualManagerVS2012::OnDrawButtonBorder (pDC, pButton, rect, state);
		return;
	}

	if (pButton->m_nStyle & TBBS_CHECKED)
	{
		COLORREF clrBorder = (!CBCGPToolBar::IsCustomizeMode () && (pButton->m_nStyle & TBBS_DISABLED)) ? globalData.clrBarShadow : m_clrHighlightChecked;
		rect.DeflateRect(1, 1);
		
		if (CBCGPToolBarImages::m_bIsDrawOnGlass)
		{
			CBCGPDrawManager dm (*pDC);
			
			dm.DrawRect (rect, (COLORREF)-1, clrBorder);
		}
		else
		{
			pDC->Draw3dRect(rect, clrBorder, clrBorder);
		}
	}
}
//***********************************************************************************
void CBCGPVisualManager2013::OnFillHighlightedArea (CDC* pDC, CRect rect, CBrush* pBrush, CBCGPToolbarButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || pButton == NULL)
	{
		CBCGPVisualManagerVS2012::OnFillHighlightedArea (pDC, rect, pBrush, pButton);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pBrush);
	ASSERT_VALID (pButton);

	BOOL bIsPopupMenu = pButton->GetParentWnd () != NULL &&
		pButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar));

	if (bIsPopupMenu)
	{
		CBCGPVisualManagerVS2012::OnFillHighlightedArea (pDC, rect, pBrush, pButton);
		return;
	}

	if ((pButton->m_nStyle & TBBS_CHECKED))
	{
		rect.DeflateRect(1, 1);
	}

	CBCGPDrawManager dm (*pDC);

	if (pBrush == &m_brHighlight)
	{
		dm.DrawRect(rect, m_clrHighlight, (COLORREF)-1);
		return;
	}
	else if (pBrush == &m_brHighlightDn)
	{
		dm.DrawRect(rect, m_clrHighlightDn, (COLORREF)-1);
		return;
	}
	else if (pBrush == &m_brHighlightChecked)
	{
		dm.DrawRect (rect, m_clrHighlightChecked, (COLORREF)-1);
		return;
	}
}
//*********************************************************************************************************
void CBCGPVisualManager2013::OnDrawTab (CDC* pDC, CRect rectTab,
						int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || !pTabWnd->Is3DStyle())
	{
		CBCGPVisualManagerVS2012::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);
		return;
	}

	const BOOL bIsHighlight = iTab == pTabWnd->GetHighlightedTab ();

	if (bIsActive || bIsHighlight)
	{
		CRect rectFill = rectTab;

		if (bIsActive)
		{
			if (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
			{
				rectFill.top--;
				rectTab.top--;
			}
			else
			{
				rectFill.bottom++;
				rectTab.bottom++;
			}
		}
		else
		{
			rectFill.DeflateRect(0, 1);
		}

		if (bIsActive)
		{
			pDC->FillSolidRect(rectFill, globalData.clrBarHilite);
		}
		else
		{
			pDC->FillRect (rectFill, &m_brHighlight);
		}
	}

	CRect rectTabFrame = rectTab;

	COLORREF clrBkTab = pTabWnd->GetTabBkColor(iTab);
	if (clrBkTab != (COLORREF)-1)
	{
		CRect rectColor = rectTab;
		const int nColorBarHeight = max(3, rectTab.Height() / 6);

		if (pTabWnd->GetLocation() == CBCGPBaseTabWnd::LOCATION_BOTTOM)
		{
			rectColor.top = rectColor.bottom - nColorBarHeight;
			rectTab.bottom -= nColorBarHeight;
		}
		else
		{
			rectColor.bottom = rectColor.top + nColorBarHeight;
			rectTab.top += nColorBarHeight;
		}

		rectColor.DeflateRect(bIsActive ? 1 : 2, 0);

		CBrush br(clrBkTab);
		pDC->FillRect(rectColor, &br);
	}

	if (bIsActive)
	{
		CBCGPPenSelector ps(*pDC, globalData.clrBarDkShadow);
		
		if (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
		{
			pDC->MoveTo (rectTabFrame.left, rectTabFrame.top);
			pDC->LineTo (rectTabFrame.left, rectTabFrame.bottom);
			pDC->LineTo (rectTabFrame.right - 1, rectTabFrame.bottom);
			pDC->LineTo (rectTabFrame.right - 1, rectTabFrame.top);
		}
		else
		{
			pDC->MoveTo (rectTabFrame.left, rectTabFrame.bottom - 1);
			pDC->LineTo (rectTabFrame.left, rectTabFrame.top);
			pDC->LineTo (rectTabFrame.right - 1, rectTabFrame.top);
			pDC->LineTo (rectTabFrame.right - 1, rectTabFrame.bottom);
		}
	}

	COLORREF clrTabText = pTabWnd->GetTabTextColor(iTab);
	if (clrTabText == (COLORREF)-1)
	{
		clrTabText = (bIsActive || bIsHighlight) ? m_clrAccentText : globalData.clrBarText;
	}

	OnDrawTabContent(pDC, rectTab, iTab, bIsActive, pTabWnd, clrTabText);
}
//*********************************************************************************************************
void CBCGPVisualManager2013::GetTabFrameColors (const CBCGPBaseTabWnd* pTabWnd,
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
	
	CBCGPVisualManagerVS2012::GetTabFrameColors (pTabWnd,
			   clrDark, clrBlack,
			   clrHighlight, clrFace,
			   clrDarkShadow, clrLight,
			   pbrFace, pbrBlack);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || 
		(pTabWnd->IsDialogControl() && !pTabWnd->IsPropertySheetTab()))
	{
		return;
	}

	clrDarkShadow = clrHighlight = clrLight = clrBlack = clrDark = globalData.clrBarDkShadow;
}
//**********************************************************************************
COLORREF CBCGPVisualManager2013::GetActiveTabBackColor(const CBCGPBaseTabWnd* pTabWnd) const
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || 
		(pTabWnd->IsDialogControl() && !pTabWnd->IsPropertySheetTab()))
	{
		return CBCGPVisualManagerVS2012::GetActiveTabBackColor(pTabWnd);
	}

	return globalData.clrBarFace;
}
//**********************************************************************************
void CBCGPVisualManager2013::OnDrawTabContent (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd,
							COLORREF clrText)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		(pTabWnd->IsDialogControl () && !pTabWnd->IsPropertySheetTab()) ||
		(pTabWnd->GetTabBkColor(iTab) != (COLORREF)-1 && !pTabWnd->Is3DStyle()) ||
		pTabWnd->GetTabTextColor(iTab) != (COLORREF)-1)
	{
		if (pTabWnd->IsPointerStyle() && (bIsActive || pTabWnd->GetHighlightedTab() == iTab))
		{
			clrText = m_clrAccentText;
		}
	}
	else
	{
		clrText = bIsActive || (pTabWnd->IsPointerStyle() && pTabWnd->GetHighlightedTab() == iTab) ? m_clrAccentText : globalData.clrBarText;
	}

	CBCGPVisualManagerXP::OnDrawTabContent (pDC, rectTab, iTab, bIsActive, pTabWnd, clrText);
}
//*********************************************************************************************************
void CBCGPVisualManager2013::OnDrawTabBorder(CDC* pDC, CBCGPTabWnd* pTabWnd, CRect rectBorder, COLORREF clrBorder,
										 CPen& penLine)
{
	CBCGPVisualManagerXP::OnDrawTabBorder(pDC, pTabWnd, rectBorder, clrBorder, penLine);
}
//**********************************************************************************
BOOL CBCGPVisualManager2013::OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || (!pTabWnd->Is3DStyle() && !pTabWnd->IsVS2005Style()))
	{
		return CBCGPVisualManagerVS2012::OnEraseTabsFrame (pDC, rect, pTabWnd);
	}

	COLORREF clrActiveTab = pTabWnd->GetTabBkColor (pTabWnd->GetActiveTab ());
	CBrush br(clrActiveTab == (COLORREF)-1 ? globalData.clrBarHilite : clrActiveTab);
	pDC->FillRect(rect, &br);

	return TRUE;
}
//**********************************************************************************
void CBCGPVisualManager2013::OnDrawTabButton (CDC* pDC, CRect rect, 
											   const CBCGPBaseTabWnd* pTabWnd,
											   int nID,
											   BOOL bIsHighlighted,
											   BOOL bIsPressed,
											   BOOL bIsDisabled)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || pTabWnd->IsDialogControl ())
	{
		CBCGPVisualManagerXP::OnDrawTabButton (pDC, rect, pTabWnd, nID, bIsHighlighted, bIsPressed, bIsDisabled);
		return;
	}

	rect.DeflateRect(1, 1);

	if (bIsHighlighted && bIsPressed)
	{
		if (pTabWnd->GetHighlightedTab() == m_nCurrDrawedTab && pTabWnd->GetActiveTab() != m_nCurrDrawedTab)
		{
			pDC->FillRect (rect, &m_brHighlightChecked);
		}
		else
		{
			pDC->FillRect (rect, &m_brHighlightDn);
		}
	}
	else if (bIsHighlighted)
	{
		if (pTabWnd->GetHighlightedTab() == m_nCurrDrawedTab && pTabWnd->GetActiveTab() != m_nCurrDrawedTab)
		{
			pDC->FillRect (rect, &m_brHighlightDn);
		}
		else
		{
			pDC->FillRect (rect, &m_brHighlight);
		}
	}

	CBCGPMenuImages::IMAGE_STATE state = (bIsHighlighted || bIsPressed) ? CBCGPMenuImages::ImageBlack2 : CBCGPMenuImages::ImageBlack;
	CBCGPMenuImages::Draw (pDC, (CBCGPMenuImages::IMAGES_IDS)nID, rect, state);
}
//**********************************************************************************
BOOL CBCGPVisualManager2013::UseLargeCaptionFontInDockingCaptions() 
{ 
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::UseLargeCaptionFontInDockingCaptions();
	}

	return TRUE; 
}
//**********************************************************************************
#ifndef BCGP_EXCLUDE_GRID_CTRL

void CBCGPVisualManager2013::OnFillGridHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnFillGridHeaderBackground (pCtrl, pDC, rect);
		return;
	}

	pDC->FillRect(rect, &globalData.brBarFace);
}
//********************************************************************************
BOOL CBCGPVisualManager2013::OnDrawGridHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::OnDrawGridHeaderItemBorder (pCtrl, pDC, rect, bPressed);
	}

	if (bPressed)
	{
		pDC->FillRect(rect, &m_brHighlightDn);
	}
	else if (pCtrl != NULL && pCtrl->IsGridHeaderItemHovered())
	{
		pDC->FillRect(rect, &m_brHighlight);
	}

	BOOL bIsVertLineReady = FALSE;

	if (pCtrl->GetSafeHwnd() != NULL)
	{
		CRect rectGrid;
		pCtrl->GetClientRect(rectGrid);

		if (rect.top - rectGrid.top > rect.Height() / 2)
		{
			CBCGPPenSelector pen(*pDC, &m_penSeparator);

			pDC->MoveTo(rect.right - 1, rect.top);
			pDC->LineTo(rect.right - 1, rect.bottom);

			bIsVertLineReady = TRUE;
		}

		if (pCtrl->IsGroupByBox())
		{
			CBCGPPenSelector pen(*pDC, &m_penSeparator);

			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.right - 1, rect.top);
		}
	}

	if (!bIsVertLineReady)
	{
		CRect rectVertLine = rect;
		rectVertLine.left = rectVertLine.right - 1;
		rectVertLine.bottom -= 2;
	
		CBCGPDrawManager dm(*pDC);
		dm.FillGradient(rectVertLine, m_clrSeparator, globalData.clrBarFace);
	}

	CBCGPPenSelector pen(*pDC, RGB(171, 171, 171));

	pDC->MoveTo(rect.left, rect.bottom - 1);
	pDC->LineTo(rect.right, rect.bottom - 1);

	return FALSE;
}
//********************************************************************************
void CBCGPVisualManager2013::OnFillGridRowHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnFillGridRowHeaderBackground (pCtrl, pDC, rect);
		return;
	}

	pDC->FillRect(rect, &globalData.brBarFace);
}
//********************************************************************************
void CBCGPVisualManager2013::OnFillGridSelectAllAreaBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pCtrl);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnFillGridSelectAllAreaBackground (pCtrl, pDC, rect, bPressed);
		return;
	}

	pDC->FillRect(rect, &globalData.brBarFace);

	rect.DeflateRect (5, 5);
	int nMin = min (rect.Width (), rect.Height ());
	nMin = min (nMin, pCtrl->GetButtonWidth () - 5);
	rect.left = rect.right  - nMin;
	rect.top  = rect.bottom - nMin;

	POINT ptRgn [] =
	{
		{rect.right, rect.top},
		{rect.right, rect.bottom},
		{rect.left, rect.bottom}
	};

	CRgn rgn;
	rgn.CreatePolygonRgn (ptRgn, 3, WINDING);

	pDC->SelectClipRgn (&rgn, RGN_COPY);

	CBrush brFill (m_clrSeparator);
	pDC->FillRect (rect, &brFill);

	pDC->SelectClipRgn (NULL, RGN_COPY);
}
//********************************************************************************
COLORREF CBCGPVisualManager2013::OnFillGridGroupByBoxBackground (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnFillGridGroupByBoxBackground (pDC, rect);
	}

	pDC->FillRect(rect, &globalData.brBarFace);
	return globalData.clrBarText;
}
//********************************************************************************
COLORREF CBCGPVisualManager2013::OnFillGridGroupByBoxTitleBackground (CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnFillGridGroupByBoxTitleBackground (pDC, rect);
	}

	pDC->FillRect(rect, &globalData.brBarFace);
	return globalData.clrBarText;
}
//********************************************************************************
COLORREF CBCGPVisualManager2013::GetGridGroupByBoxLineColor () const
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::GetGridGroupByBoxLineColor ();
	}

	return globalData.clrBarDkShadow;
}
//********************************************************************************
void CBCGPVisualManager2013::OnDrawGridGroupByBoxItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnDrawGridGroupByBoxItemBorder (pCtrl, pDC, rect);
		return;
	}

	pDC->FillRect(rect, &globalData.brBarFace);
	pDC->Draw3dRect (rect, globalData.clrBarDkShadow, globalData.clrBarDkShadow);
}
//********************************************************************************
COLORREF CBCGPVisualManager2013::GetGridLeftOffsetColor (CBCGPGridCtrl* pCtrl)
{
	return CBCGPVisualManagerXP::GetGridLeftOffsetColor (pCtrl);
}
//********************************************************************************
void CBCGPVisualManager2013::OnFillGridGroupBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pCtrl);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnFillGridGroupBackground (pCtrl, pDC, rectFill);
		return;
	}

	if (rectFill.Height () > pCtrl->GetBaseHeight ())
	{
		pDC->FillRect(rectFill, &globalData.brBarFace);
		rectFill.top ++;
	}

	CBrush brFill(m_clrSeparator);
	pDC->FillRect(rectFill, &brFill);
}
//********************************************************************************
BOOL CBCGPVisualManager2013::IsGridGroupUnderline () const
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::IsGridGroupUnderline ();
	}

	return FALSE;
}
//********************************************************************************
void CBCGPVisualManager2013::OnDrawGridGroupUnderline (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill)
{
	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerXP::OnDrawGridGroupUnderline (pCtrl, pDC, rectFill);
		return;
	}

	pDC->FillRect(rectFill, &globalData.brBarFace);
}
//********************************************************************************
COLORREF CBCGPVisualManager2013::OnFillGridRowBackground (CBCGPGridCtrl* pCtrl, 
												  CDC* pDC, CRect rectFill, BOOL bSelected)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pCtrl);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnFillGridRowBackground (pCtrl, pDC, rectFill, bSelected);
	}

	// Fill area:
	if (bSelected)
	{
		if (!pCtrl->IsFocused ())
		{
			// no painting
		}
		else
		{
			pDC->FillRect (rectFill, &m_brAccentLight);
		}
	}
	else
	{
		CBrush brFill(globalData.clrBarShadow);
		pDC->FillRect (rectFill, &brFill);
	}

	// Return text color:
	if (!pCtrl->IsHighlightGroups () && bSelected)
	{
		if (!pCtrl->IsFocused ())
		{
			return m_clrAccentText;
		}
		else
		{
			COLORREF clrText;

			if (GetRValue (m_clrAccentLight) <= 128 ||
				GetGValue (m_clrAccentLight) <= 128 ||
				GetBValue (m_clrAccentLight) <= 128)
			{
				clrText = RGB(255, 255, 255);
			}
			else
			{
				clrText = RGB(0, 0, 0);
			}

			return clrText;
		}
	}

	return pCtrl->IsHighlightGroups () ? 
		(pCtrl->IsControlBarColors () ? globalData.clrBarShadow : globalData.clrBtnShadow) :
		globalData.clrWindowText;
}
//********************************************************************************
COLORREF CBCGPVisualManager2013::OnFillGridItem (CBCGPGridCtrl* pCtrl, 
											CDC* pDC, CRect rectFill,
											BOOL bSelected, BOOL bActiveItem, BOOL bSortedColumn)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pCtrl);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnFillGridItem (pCtrl, pDC, rectFill, bSelected, bActiveItem, bSortedColumn);
	}

	if (bSelected && !bActiveItem)
	{
		if (!pCtrl->IsFocused ())
		{
			if (pCtrl->IsControlBarColors ())
			{
				pDC->FillRect (rectFill, &globalData.brBarFace);
			}
			else
			{
				pDC->FillRect (rectFill, &globalData.brBtnFace);
			}

			return m_clrAccentText;
		}
		else
		{
			pDC->FillRect (rectFill, &m_brAccentLight);

			COLORREF clrText;

			if (GetRValue (m_clrAccentLight) <= 128 ||
				GetGValue (m_clrAccentLight) <= 128 ||
				GetBValue (m_clrAccentLight) <= 128)
			{
				clrText = RGB(255, 255, 255);
			}
			else
			{
				clrText = RGB(0, 0, 0);
			}

			return clrText;
		}
	}

	if (bSelected && !pCtrl->IsFocused () && bActiveItem)
	{
		CBrush brFill(globalData.clrBarShadow);
		pDC->FillRect (rectFill, &brFill);
		return m_clrAccentText;
	}

	return CBCGPVisualManagerXP::OnFillGridItem(pCtrl, pDC, rectFill, bSelected, bActiveItem, bSortedColumn);
}
//********************************************************************************
BOOL CBCGPVisualManager2013::OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme)
{
	BOOL bRes = CBCGPVisualManagerVS2012::OnSetGridColorTheme (pCtrl, theme);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return bRes;
	}

	if (m_Style != Office2013_White)
	{
		theme.m_OddColors.m_clrBackground = globalData.clrBarLight;
	}

	theme.m_HeaderSelColors.m_clrBorder = theme.m_LeftOffsetColors.m_clrBorder = theme.m_HeaderColors.m_clrBorder = RGB(171, 171, 171);
	theme.m_clrVertLine = theme.m_clrHorzLine = m_clrSeparator;
	theme.m_HeaderColors.m_clrBackground = (COLORREF)-1;

	return bRes;
}
//********************************************************************************
COLORREF CBCGPVisualManager2013::GetReportCtrlGroupBackgoundColor ()
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::GetReportCtrlGroupBackgoundColor ();
	}

	return globalData.clrBarShadow;
}
//********************************************************************************
COLORREF CBCGPVisualManager2013::OnFillReportCtrlRowBackground (CBCGPGridCtrl* pCtrl, CDC* pDC,
		CRect rectFill, BOOL bSelected, BOOL bGroup)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pCtrl);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerXP::OnFillReportCtrlRowBackground (pCtrl, pDC, rectFill, bSelected, bGroup);
	}

	// Fill area:
	COLORREF clrText = (COLORREF)-1;

	clrText = RGB(255,0,0);

	if (bSelected)
	{
		if (!pCtrl->IsFocused ())
		{
			// no painting
			clrText = m_clrAccentText;
		}
		else
		{
			pDC->FillRect (rectFill, &m_brAccentLight);
			clrText = m_clrReportGroupText;
		}
	}
	else
	{
		if (bGroup)
		{
			// no painting
			clrText = m_clrReportGroupText;
		}
	}

	// Return text color:
	return clrText;
}
#endif // BCGP_EXCLUDE_GRID_CTRL

BOOL CBCGPVisualManager2013::GetToolTipParams (CBCGPToolTipParams& params, 
											   UINT /*nType*/ /*= (UINT)(-1)*/)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::GetToolTipParams (params);
	}
	
	params.m_bBoldLabel = TRUE;
	params.m_bDrawDescription = TRUE;
	params.m_bDrawIcon = TRUE;
	params.m_bRoundedCorners = FALSE;
	params.m_bDrawSeparator = FALSE;
	params.m_nPaddingX = 4;
	params.m_nPaddingY = 4;
	
	params.m_clrFill = globalData.clrBarHilite;
	params.m_clrFillGradient = (COLORREF)-1;
	params.m_clrText = m_clrRibbonTabs;
	params.m_clrBorder = globalData.clrBarShadow;
	
	return TRUE;
}

#ifndef BCGP_EXCLUDE_TOOLBOX

BOOL CBCGPVisualManager2013::OnEraseToolBoxButton (CDC* pDC, CRect rect,
											CBCGPToolBoxButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::OnEraseToolBoxButton(pDC, rect, pButton);
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	CBCGPDrawManager dm (*pDC);

	if (pButton->GetCheck ())
	{
		dm.DrawRect (rect, m_clrHighlightChecked, (COLORREF)-1);
	}
	else if (pButton->IsHighlighted ())
	{
		dm.DrawRect(rect, m_clrHighlight, (COLORREF)-1);
	}

	return TRUE;
}
//**********************************************************************************
BOOL CBCGPVisualManager2013::OnDrawToolBoxButtonBorder (CDC* pDC, CRect& rect, 
												 CBCGPToolBoxButton* pButton, UINT uiState)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::OnDrawToolBoxButtonBorder(pDC, rect, pButton, uiState);
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pButton->GetCheck ())
	{
		CBCGPDrawManager dm (*pDC);
		dm.DrawRect(rect, (COLORREF)-1, m_clrHighlightDn);
	}

	return TRUE;
}
//**********************************************************************************
COLORREF CBCGPVisualManager2013::GetToolBoxButtonTextColor (CBCGPToolBoxButton* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::GetToolBoxButtonTextColor(pButton);
	}

	return globalData.clrBarText;
}

#endif	// BCGP_EXCLUDE_TOOLBOX

//*********************************************************************************************************
void CBCGPVisualManager2013::OnDrawAutohideBar(CDC* pDC, CRect rectBar, CBCGPAutoHideButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	COLORREF clrFill = pButton->IsHighlighted() ? m_clrHighlightDn : m_clrSeparator;
	pDC->FillSolidRect(rectBar, clrFill);
}
//****************************************************************************************
void CBCGPVisualManager2013::OnFillOutlookPageButton (CDC* pDC, const CRect& rect,
										BOOL bIsHighlighted, BOOL bIsPressed,
										COLORREF& clrText)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnFillOutlookPageButton (pDC, rect,
										bIsHighlighted, bIsPressed,
										clrText);
		return;
	}

	ASSERT_VALID (pDC);

	clrText = globalData.clrBarText;

	CBCGPDrawManager dm (*pDC);

	if (bIsHighlighted)
	{
		dm.DrawRect(rect, m_clrHighlight, (COLORREF)-1);
	}
	else if (bIsPressed)
	{
		dm.DrawRect (rect, m_clrHighlightChecked, (COLORREF)-1);
	}
	else if (bIsHighlighted && bIsPressed)
	{
		dm.DrawRect(rect, m_clrHighlightDn, (COLORREF)-1);
	}
	else
	{
		dm.DrawRect (rect, globalData.clrBarLight, (COLORREF)-1);
	}
}
//****************************************************************************************
void CBCGPVisualManager2013::OnDrawOutlookPageButtonBorder (CDC* pDC, 
							CRect& rectBtn, BOOL bIsHighlighted, BOOL bIsPressed)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnDrawOutlookPageButtonBorder (pDC, 
							rectBtn, bIsHighlighted, bIsPressed);
		return;
	}
}
//****************************************************************************************
void CBCGPVisualManager2013::OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, COLORREF& clrText)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnFillOutlookBarCaption (pDC, rectCaption, clrText);
		return;
	}

	pDC->FillRect(rectCaption, &globalData.brBarFace);
	clrText = globalData.clrBarText;
}


#ifndef BCGP_EXCLUDE_PLANNER
//****************************************************************************************
void CBCGPVisualManager2013::OnFillPlanner (CDC* pDC, CBCGPPlannerView* pView, 
		CRect rect, BOOL bWorkingArea)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pView);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnFillPlanner (pDC, pView, rect, bWorkingArea);
		return;
	}

	if (m_bPlannerBackItemSelected)
	{
		pDC->FillRect (rect, &m_brAccent);
	}
	else
	{
		CBCGPVisualManagerVS2010::OnFillPlanner (pDC, pView, rect, bWorkingArea);
	}
}
//****************************************************************************************
COLORREF CBCGPVisualManager2013::OnFillPlannerCaption (CDC* pDC,
		CBCGPPlannerView* pView, CRect rect, BOOL bIsToday, BOOL bIsSelected,
		BOOL bNoBorder/* = FALSE*/, BOOL bHorz /*= TRUE*/)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ()/* || m_Style == VS2012_LightBlue*/)
	{
		return CBCGPVisualManagerVS2012::OnFillPlannerCaption (pDC,
			pView, rect, bIsToday, bIsSelected, bNoBorder, bHorz);
	}

	const BOOL bMonth = DYNAMIC_DOWNCAST(CBCGPPlannerViewMonth, pView) != NULL;

	ASSERT_VALID (pDC);

	BOOL bDay = FALSE;

	if (!bMonth)
	{
		bDay = pView->IsKindOf (RUNTIME_CLASS (CBCGPPlannerViewDay));

		if (bDay)
		{
			if (!bIsToday)
			{
				if (pView->IsKindOf (RUNTIME_CLASS (CBCGPPlannerViewMulti)))
				{
					rect.top++;
					rect.left++;
				}
			}
		}
	}
	else
	{
		if (!bIsToday)
		{
			rect.bottom--;
		}
	}

	COLORREF clrText = globalData.clrBarText;

	if ((bMonth && (m_bPlannerCaptionBackItemHeader | bIsToday | bIsSelected)) || !bMonth)
	{
		bIsSelected = bIsSelected & bMonth;

		CBrush br(bIsToday ? m_clrPlannerTodayFill : bIsSelected ? GetPlannerSelectionColor(pView) : m_clrAccentLight);
		pDC->FillRect(rect, &br);

		if (bIsToday || bIsSelected)
		{
			clrText = globalData.clrBarHilite;
		}
	}

	return clrText;
}
//****************************************************************************************
void CBCGPVisualManager2013::OnDrawPlannerHeader (CDC* pDC, 
	CBCGPPlannerView* pView, CRect rect)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnDrawPlannerHeader (pDC, pView, rect);
		return;
	}

	ASSERT_VALID (pDC);

	COLORREF clr = GetPlannerSeparatorColor (pView);

	if (DYNAMIC_DOWNCAST(CBCGPPlannerViewMonth, pView) != NULL)
	{
		clr = m_clrAccentLight;
	}

	CBrush br (clr);
	pDC->FillRect (rect, &br);

	if (pView->IsKindOf (RUNTIME_CLASS (CBCGPPlannerViewDay)))
	{
		if (rect.left == pView->GetAppointmentsRect().left)
		{
			CRect rect1 (rect);
			rect1.right = rect1.left + 1;

			if (pView->IsKindOf (RUNTIME_CLASS (CBCGPPlannerViewMulti)))
			{
				rect1.top++;
			}

			CBrush br(m_clrAccentLight);
			pDC->FillRect (rect1, &br);
		}
	}
}
//****************************************************************************************
void CBCGPVisualManager2013::OnDrawPlannerHeaderPane (CDC* pDC, 
	CBCGPPlannerView* pView, CRect rect)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnDrawPlannerHeaderPane (pDC, pView, rect);
		return;
	}
}
//****************************************************************************************
COLORREF CBCGPVisualManager2013::GetPlannerViewWorkingColor (CBCGPPlannerView* pView)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		GetPlannerViewBackgroundColor (pView) != m_clrPlannerWork)
	{
		return CBCGPVisualManagerVS2012::GetPlannerViewWorkingColor (pView);
	}

	return globalData.clrBarHilite;
}
//****************************************************************************************
COLORREF CBCGPVisualManager2013::GetPlannerViewNonWorkingColor (CBCGPPlannerView* pView)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		GetPlannerViewBackgroundColor (pView) != m_clrPlannerWork)
	{
		return CBCGPVisualManagerVS2012::GetPlannerViewNonWorkingColor (pView);
	}

	return RGB(240, 240, 240);
}
//****************************************************************************************
COLORREF CBCGPVisualManager2013::OnFillPlannerTimeBar (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect, COLORREF& clrLine)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::OnFillPlannerTimeBar (pDC, pView, rect, clrLine);
	}

	ASSERT_VALID (pDC);

	CBrush br(globalData.clrBarHilite);
	pDC->FillRect (rect, &br);
	
	clrLine = m_clrPalennerLine;

	return globalData.clrBarText;
}
//****************************************************************************************
void CBCGPVisualManager2013::OnFillPlannerWeekBar (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnFillPlannerWeekBar (pDC, pView, rect);
		return;
	}

	ASSERT_VALID (pDC);

	CBrush br(globalData.clrBarHilite);
	pDC->FillRect (rect, &br);
}
//****************************************************************************************
void CBCGPVisualManager2013::OnFillPlannerHeaderAllDay (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnFillPlannerHeaderAllDay (pDC, pView, rect);
		return;
	}

	ASSERT_VALID (pDC);

	rect.bottom--;

	CBrush br(globalData.clrBarHilite);
	pDC->FillRect (rect, &br);

	CPen pen(PS_SOLID, 0, m_clrPalennerLine);
	CPen* pOldPen = (CPen*)pDC->SelectObject(&pen);

	pDC->MoveTo(rect.left, rect.bottom);
	pDC->LineTo(rect.right, rect.bottom);

	pDC->SelectObject(pOldPen);
}
//****************************************************************************************
void CBCGPVisualManager2013::OnDrawPlannerHeaderAllDayItem (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect, BOOL bIsToday, BOOL bIsSelected)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnDrawPlannerHeaderAllDayItem (pDC, pView, rect, 
			bIsToday, bIsSelected);
		return;
	}

	if (!bIsSelected)
	{
		return;
	}

	if (pView->IsKindOf (RUNTIME_CLASS (CBCGPPlannerViewDay)) && rect.left != pView->GetAppointmentsRect().left)
	{
		rect.left++;
	}

	CBrush br (GetPlannerSelectionColor (pView));
	pDC->FillRect (rect, &br);
}

#endif

COLORREF CBCGPVisualManager2013::GetTreeControlFillColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected, BOOL bIsFocused, BOOL bIsDisabled)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::GetTreeControlFillColor(pTreeCtrl, bIsSelected, bIsFocused, bIsDisabled);
	}

	if (!bIsSelected)
	{
		return m_clrControl;
	}

	if (bIsDisabled)
	{
		return globalData.clrBarLight;
	}

	return bIsFocused ? m_clrAccentLight : globalData.clrBarShadow;
}
//*******************************************************************************
COLORREF CBCGPVisualManager2013::GetTreeControlTextColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected, BOOL bIsFocused, BOOL bIsDisabled)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManagerVS2012::GetTreeControlTextColor(pTreeCtrl, bIsSelected, bIsFocused, bIsDisabled);
	}

	if (!bIsSelected)
	{
		return globalData.clrBarText;
	}
	
	COLORREF clrText = (COLORREF)-1;
	
	if (bIsFocused)
	{
		if (GetRValue (m_clrAccentLight) <= 128 ||
			GetGValue (m_clrAccentLight) <= 128 ||
			GetBValue (m_clrAccentLight) <= 128)
		{
			clrText = RGB(255, 255, 255);
		}
		else
		{
			clrText = RGB(0, 0, 0);
		}
	}
	else
	{
		clrText = globalData.clrBarText;
	}

	return clrText;
}
//*******************************************************************************
void CBCGPVisualManager2013::OnDrawMenuResizeBar (CDC* pDC, CRect rect, int nResizeFlags)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnDrawMenuResizeBar(pDC, rect, nResizeFlags);
		return;
	}

	pDC->FillSolidRect(rect, globalData.clrBarShadow);
	OnDrawMenuResizeGipper(pDC, rect, nResizeFlags, m_clrAccent);
}

#ifndef BCGP_EXCLUDE_RIBBON

void CBCGPVisualManager2013::OnDrawRibbonPaletteButtonIcon (
					CDC* pDC, 
					CBCGPRibbonPaletteIcon* pButton,
					int nID,
					CRect rectImage)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnDrawRibbonPaletteButtonIcon(pDC, pButton, nID, rectImage);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	CBCGPMenuImages::IMAGES_IDS id = (CBCGPMenuImages::IMAGES_IDS)nID;

	CBCGPMenuImages::Draw (pDC, id, rectImage,
		pButton->IsDisabled() ? CBCGPMenuImages::ImageLtGray : CBCGPMenuImages::ImageBlack);
}
//***********************************************************************************
void CBCGPVisualManager2013::OnDrawRibbonPaletteButton (
					CDC* pDC, 
					CBCGPRibbonPaletteIcon* pButton)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPVisualManagerVS2012::OnDrawRibbonPaletteButton(pDC, pButton);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	OnFillRibbonButton (pDC, pButton);

	COLORREF clrBorder = globalData.clrBarShadow;

	if (!pButton->IsDisabled() && (pButton->IsHighlighted () || pButton->IsDroppedDown () || pButton->IsFocused()))
	{
		clrBorder = m_clrAccent;
	}

	pDC->Draw3dRect(pButton->GetRect(), clrBorder, clrBorder);
}

#endif
