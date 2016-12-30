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
// BCGWinXPVisualManager.cpp: implementation of the CBCGPWinXPVisualManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "multimon.h"
#include "BCGPWinXPVisualManager.h"
#include "BCGPPopupMenuBar.h"
#include "BCGPButton.h"
#include "BCGPDrawManager.h"
#include "BCGPBaseTabWnd.h"
#include "BCGPDockingControlBar.h"
#include "BCGPTasksPane.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPOutlookButton.h"
#include "BCGPStatusBar.h"
#include "BCGPCaptionBar.h"
#include "BCGPToolBox.h"
#include "BCGPPopupWindow.h"
#include "BCGPCalendarBar.h"
#include "BCGPDockBar.h"
#include "BCGPRibbonStatusBar.h"
#include "BCGPRibbonPanelMenu.h"
#include "BCGPToolbarEditBoxButton.h"
#include "bcgpstyle.h"
#include "BCGPGanttChart.h"
#include "BCGPColorBar.h"
#include "BCGPSpinButtonCtrl.h"
#include "BCGPCalculator.h"
#include "BCGPOutlookWnd.h"
#include "BCGPPropList.h"
#include "BCGPGridCtrl.h"

#ifndef BCGP_EXCLUDE_RIBBON
#include "BCGPRibbonProgressBar.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CBCGPWinXPVisualManager, CBCGPVisualManagerXP)

BOOL CBCGPWinXPVisualManager::m_b3DTabsXPTheme = FALSE;

#ifndef MENU_POPUPGUTTER
#define MENU_POPUPGUTTER 13
#endif

#ifndef MENU_POPUPITEM
#define MENU_POPUPITEM	14
#endif

#ifndef MPI_HOT
#define MPI_HOT			2
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPWinXPVisualManager::CBCGPWinXPVisualManager(BOOL bIsTemporary) :
	CBCGPVisualManagerXP (bIsTemporary)
{
	m_bShadowHighlightedImage = FALSE;

	m_bOfficeStyleMenus = FALSE;

	m_nVertMargin = 12;
	m_nHorzMargin = 12;
	m_nGroupVertOffset = 15;
	m_nGroupCaptionHeight = 25;
	m_nGroupCaptionHorzOffset = 13;
	m_nGroupCaptionVertOffset = 7;
	m_nTasksHorzOffset = 12;
	m_nTasksIconHorzOffset = 5;
	m_nTasksIconVertOffset = 4;
	m_bActiveCaptions = TRUE;

	globalData.UpdateSysColors ();
	OnUpdateSystemColors ();
}

CBCGPWinXPVisualManager::~CBCGPWinXPVisualManager()
{
}

BOOL CBCGPWinXPVisualManager::IsWinXPThemeAvailible ()
{
	CBCGPWinXPVisualManager* pWinXPManager =
		DYNAMIC_DOWNCAST (CBCGPWinXPVisualManager, m_pVisManager);
	if (pWinXPManager != NULL)
	{
		return pWinXPManager->m_hThemeWindow != NULL;
	}

	// Create a temporary manager and check it:
	CBCGPWinXPVisualManager winXPManager (TRUE /* Temporary */);
	return winXPManager.m_hThemeWindow != NULL;
}

void CBCGPWinXPVisualManager::SetOfficeStyleMenus (BOOL bOn)
{
	m_bOfficeStyleMenus = bOn;
}

void CBCGPWinXPVisualManager::OnUpdateSystemColors ()
{
	CBCGPVisualManagerXP::OnUpdateSystemColors ();

	m_bShadowHighlightedImage = TRUE;

	CBCGPWinXPThemeManager::UpdateSystemColors ();

	if (m_hThemeWindow != NULL)
	{
		m_bShadowHighlightedImage = FALSE;
	}

	m_clrCalendarCaption = (COLORREF)-1;
	m_clrCalendarCaptionText = (COLORREF)-1;

	if (m_pfGetThemeColor != NULL)
	{
		if (m_hThemeToolBar != NULL)
		{
			(*m_pfGetThemeColor) (m_hThemeToolBar, TP_BUTTON, 0, TMT_TEXTCOLOR, &globalData.clrBarText);
			(*m_pfGetThemeColor) (m_hThemeToolBar, TP_BUTTON, 0, TMT_FILLCOLOR, &globalData.clrBarFace);
			(*m_pfGetThemeColor) (m_hThemeToolBar, TP_BUTTON, 0, TMT_LIGHT3D, &globalData.clrBarLight);
			(*m_pfGetThemeColor) (m_hThemeToolBar, TP_BUTTON, 0, TMT_EDGEHIGHLIGHTCOLOR, &globalData.clrBarHilite);
			(*m_pfGetThemeColor) (m_hThemeToolBar, TP_BUTTON, 0, TMT_EDGESHADOWCOLOR, &globalData.clrBarShadow);
			(*m_pfGetThemeColor) (m_hThemeToolBar, TP_BUTTON, 0, TMT_EDGEDKSHADOWCOLOR, &globalData.clrBarDkShadow);
		}

		if (m_hThemeButton != NULL)
		{
			(*m_pfGetThemeColor) (m_hThemeButton, BP_PUSHBUTTON, 0, TMT_TEXTCOLOR, &globalData.clrBtnText);
			(*m_pfGetThemeColor) (m_hThemeButton, BP_PUSHBUTTON, 0, TMT_FILLCOLOR, &globalData.clrBtnFace);
			(*m_pfGetThemeColor) (m_hThemeButton, BP_PUSHBUTTON, 0, TMT_EDGELIGHTCOLOR, &globalData.clrBtnLight);
			(*m_pfGetThemeColor) (m_hThemeButton, BP_PUSHBUTTON, 0, TMT_EDGEHIGHLIGHTCOLOR, &globalData.clrBtnHilite);
			(*m_pfGetThemeColor) (m_hThemeButton, BP_PUSHBUTTON, 0, TMT_EDGESHADOWCOLOR, &globalData.clrBtnShadow);
			(*m_pfGetThemeColor) (m_hThemeButton, BP_PUSHBUTTON, 0, TMT_EDGEDKSHADOWCOLOR, &globalData.clrBtnDkShadow);
		}

		if (m_hThemeExplorerBar != NULL)
		{
			(*m_pfGetThemeColor) (m_hThemeExplorerBar, EBP_NORMALGROUPBACKGROUND, 0, TMT_FILLCOLOR, &m_clrCalendarCaption);
			(*m_pfGetThemeColor) (m_hThemeExplorerBar, EBP_NORMALGROUPBACKGROUND, 0, TMT_TEXTCOLOR, &m_clrCalendarCaptionText);
		}
	}
}

void CBCGPWinXPVisualManager::OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz,
									   CBCGPBaseControlBar* pBar)
{
	ASSERT_VALID (pDC);

	if (m_hThemeRebar == NULL || pBar->GetSafeHwnd () == NULL)
	{
		CBCGPVisualManagerXP::OnDrawBarGripper (pDC, rectGripper, bHorz, pBar);
		return;
	}

	BOOL bSideBar = pBar != NULL && pBar->IsKindOf (RUNTIME_CLASS (CBCGPDockingControlBar));

	CRect rectFill = rectGripper;

	if (bSideBar)
	{
		bHorz = !bHorz;
	}

	COLORREF clrTextOld = pDC->SetTextColor (globalData.clrBtnShadow);
	COLORREF clrBkOld = pDC->SetBkColor (globalData.clrBtnFace);

	CRect rectGripperTheme = rectGripper;
	const int nGripperOffset = 2;

	if (bHorz)
	{
		rectGripperTheme.DeflateRect (0, nGripperOffset);
		rectGripperTheme.OffsetRect (nGripperOffset, 0);
		rectGripperTheme.right = rectGripperTheme.left + 3 * nGripperOffset;
	}
	else
	{
		rectGripperTheme.DeflateRect (nGripperOffset, 0);
		rectGripperTheme.OffsetRect (0, nGripperOffset);
		rectGripperTheme.bottom = rectGripperTheme.top + 3 * nGripperOffset;
	}

	(*m_pfDrawThemeBackground) (m_hThemeRebar, pDC->GetSafeHdc(),
		bHorz ? RP_GRIPPER : RP_GRIPPERVERT, 0, &rectGripperTheme, 0);

	if (bSideBar)
	{
		//------------------
		// Draw bar caption:
		//------------------
		int nOldBkMode = pDC->SetBkMode (OPAQUE);
		pDC->SetTextColor (globalData.clrBtnText);

		const CFont& font = CBCGPMenuBar::GetMenuFont (bHorz);

		CFont* pOldFont = pDC->SelectObject ((CFont*) &font);

		CString strCaption;
		pBar->GetWindowText (strCaption);
		strCaption = _T(" ") + strCaption + _T(" ");

		CRect rectText = rectGripper;
		UINT uiTextFormat = 0;

		TEXTMETRIC tm;
		pDC->GetTextMetrics (&tm);

		CPoint ptTextOffset (0, 0);
		if (bHorz)
		{
			ptTextOffset.y = (rectGripper.Height () - tm.tmHeight - 1) / 2;
		}
		else
		{
			ptTextOffset.x = (rectGripper.Width () - tm.tmHeight + 1) / 2;
		}

		if (bHorz)
		{
			rectText.top += ptTextOffset.y;
			pDC->DrawText (strCaption, &rectText, uiTextFormat);
		}
		else
		{
			rectText.left = rectText.right - ptTextOffset.x;
			rectText.top = rectGripper.top + ptTextOffset.y;
			rectText.bottom = rectGripper.top + 3 * ptTextOffset.y;

			uiTextFormat |= DT_NOCLIP;

			pDC->DrawText (strCaption, &rectText, uiTextFormat);
		}

		pDC->SelectObject(pOldFont);
		pDC->SetBkMode(nOldBkMode);
	}

	pDC->SetTextColor (clrTextOld);
	pDC->SetBkColor (clrBkOld);
}
//*************************************************************************************
void CBCGPWinXPVisualManager::OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea)
{
	ASSERT_VALID (pBar);
	ASSERT_VALID (pDC);

	if (pBar->IsOnGlass ())
	{
		pDC->FillSolidRect (rectClient, RGB (0, 0, 0));
		return;
	}

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPStatusBar)) &&
		m_hThemeStatusBar != NULL)
	{
		(*m_pfDrawThemeBackground) (m_hThemeStatusBar, 
			pDC->GetSafeHdc (),
			0, 0, &rectClient, 0);
		return;
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

	if (m_pfDrawThemeBackground == NULL || 
		m_hThemeRebar == NULL || 
		pBar->IsDialogControl () ||
		pBar->IsKindOf (RUNTIME_CLASS (CBCGPCaptionBar)) ||
		pBar->IsKindOf (RUNTIME_CLASS (CBCGPCalculator)) ||
		pBar->IsKindOf (RUNTIME_CLASS (CBCGPColorBar)))
	{
		CBCGPVisualManagerXP::OnFillBarBackground (pDC, pBar,rectClient, rectClip, bNCArea);
		return;
	}

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar)))
	{
		if (m_bOfficeStyleMenus)
		{
			CBCGPVisualManagerXP::OnFillBarBackground (pDC, pBar,rectClient, rectClip, bNCArea);
		}
		else
		{
			CBCGPPopupMenuBar* pMenuBar = DYNAMIC_DOWNCAST (CBCGPPopupMenuBar, pBar);
			if (pMenuBar->IsDropDownListMode())
			{
				pDC->FillRect (rectClient, &m_brMenuLight);
			}
			else
			{
				OnFillPopupMenuBackground (pDC, rectClient);
			}

			if (m_hThemeMenu != NULL && !pMenuBar->m_bDisableSideBarInXPMode)
			{
				CRect rectGutter = rectClient;
				rectGutter.right = rectGutter.left + pMenuBar->GetGutterWidth ();

				if (!pMenuBar->HasGutterLogo())
				{
					rectGutter.DeflateRect (0, 1);
					rectGutter.right += 2;
				}

				(*m_pfDrawThemeBackground) (m_hThemeMenu, pDC->GetSafeHdc(), 
					MENU_POPUPGUTTER, 0, &rectGutter, 0);
			}
		}

		return;
	}

	FillRebarPane (pDC, pBar, rectClient);
}
//*************************************************************************************
void CBCGPWinXPVisualManager::OnFillPopupMenuBackground (CDC* pDC, CRect rect)
{
	::FillRect (pDC->GetSafeHdc (), rect, ::GetSysColorBrush (COLOR_MENU));
}
//*************************************************************************************
void CBCGPWinXPVisualManager::OnDrawBarBorder (CDC* pDC, CBCGPBaseControlBar* pBar, CRect& rect)
{
	CBCGPVisualManager::OnDrawBarBorder (pDC, pBar, rect);
}
//*************************************************************************************
void CBCGPWinXPVisualManager::OnFillButtonInterior (CDC* pDC,
				CBCGPToolbarButton* pButton, CRect rect,
				CBCGPVisualManager::BCGBUTTON_STATE state)
{
	if (m_hThemeToolBar == NULL)
	{
		CBCGPVisualManagerXP::OnFillButtonInterior (pDC, pButton, rect, state);
		return;
	}

	BOOL bIsMenuBar = FALSE;
	BOOL bIsPopupMenu = FALSE;

	CBCGPToolbarMenuButton* pMenuButton = 
		DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);
	if (pMenuButton != NULL)
	{
		bIsMenuBar = pMenuButton->GetParentWnd () != NULL &&
			pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar));

		bIsPopupMenu = pMenuButton->GetParentWnd () != NULL &&
			pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar));
	}

	if (m_bOfficeStyleMenus && (bIsPopupMenu || bIsMenuBar))
	{
		CBCGPVisualManagerXP::OnFillButtonInterior (pDC, pButton, rect, state);
		return;
	}

	if (bIsPopupMenu && state != ButtonsIsHighlighted &&
		state != ButtonsIsPressed)
	{
		return;
	}

	if (pButton != NULL && DYNAMIC_DOWNCAST(CBCGPOutlookBarToolBar, pButton->GetParentWnd()) != NULL)
	{
		rect.DeflateRect(0, 2);
	}

	int nState = TS_NORMAL;
	
	if (pButton->m_nStyle & TBBS_DISABLED)
	{
		nState = TS_DISABLED;
	}
	else if ((pButton->m_nStyle & TBBS_PRESSED) && state == ButtonsIsHighlighted)
	{
		nState = TS_PRESSED;
	}
	else if (pButton->m_nStyle & TBBS_CHECKED)
	{
		nState = (state == ButtonsIsHighlighted) ? TS_HOTCHECKED : TS_CHECKED;
	}
	else if (state == ButtonsIsHighlighted)
	{
		nState = TS_HOT;

		if (pMenuButton != NULL && pMenuButton->IsDroppedDown ())
		{
			nState = TS_PRESSED;
			rect.bottom--;
		}
	}

	(*m_pfDrawThemeBackground) (m_hThemeToolBar, pDC->GetSafeHdc(), TP_BUTTON, nState, &rect, 0);
}
//**************************************************************************************
COLORREF CBCGPWinXPVisualManager::GetToolbarButtonTextColor (CBCGPToolbarButton* pButton, 
														CBCGPVisualManager::BCGBUTTON_STATE state)
{
	ASSERT_VALID (pButton);

	if (m_hThemeToolBar == NULL ||
		pButton->IsKindOf (RUNTIME_CLASS (CBCGPOutlookButton)))
	{
		return CBCGPVisualManagerXP::GetToolbarButtonTextColor (pButton, state);
	}

	return CBCGPVisualManager::GetToolbarButtonTextColor (pButton, state);
}
//************************************************************************************
void CBCGPWinXPVisualManager::OnHighlightMenuItem (CDC*pDC, CBCGPToolbarMenuButton* pButton,
											CRect rect, COLORREF& clrText)
{
	if (m_hThemeWindow == NULL || m_bOfficeStyleMenus)
	{
		CBCGPVisualManagerXP::OnHighlightMenuItem (pDC, pButton, rect, clrText);
		return;
	}

	if (m_hThemeMenu == NULL)
	{
		CBCGPVisualManager::OnHighlightMenuItem (pDC, pButton,	rect, clrText);
		return;
	}
	
	(*m_pfDrawThemeBackground) (m_hThemeMenu, pDC->GetSafeHdc(), 
		MENU_POPUPITEM, MPI_HOT, &rect, 0);

	(*m_pfGetThemeColor) (m_hThemeMenu, MENU_POPUPITEM, MPI_HOT, TMT_TEXTCOLOR, &clrText);
}
//************************************************************************************
COLORREF CBCGPWinXPVisualManager::GetHighlightedMenuItemTextColor (CBCGPToolbarMenuButton* pButton)
{
	if (m_hThemeMenu != NULL && !m_bOfficeStyleMenus)
	{
		COLORREF clrText = 0;
		(*m_pfGetThemeColor) (m_hThemeMenu, MENU_POPUPITEM, 0, TMT_TEXTCOLOR, &clrText);
		return clrText;
	}

	if (m_hThemeWindow == NULL || m_bOfficeStyleMenus)
	{
		return CBCGPVisualManagerXP::GetHighlightedMenuItemTextColor (pButton);
	}
	else
	{
		return CBCGPVisualManager::GetHighlightedMenuItemTextColor (pButton);
	}
}
//************************************************************************************
void CBCGPWinXPVisualManager::OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed)
{
	if (m_hThemeWindow == NULL || m_bOfficeStyleMenus)
	{
		CBCGPVisualManagerXP::OnHighlightRarelyUsedMenuItems  (pDC, rectRarelyUsed);
		return;
	}

	ASSERT_VALID (pDC);

	CBCGPDrawManager dm (*pDC);

	rectRarelyUsed.left --;
	rectRarelyUsed.right = rectRarelyUsed.left + CBCGPToolBar::GetMenuImageSize ().cx + 
		2 * GetMenuImageMargin () + 2;
	dm.HighlightRect (rectRarelyUsed, 94);
}
//************************************************************************************
void CBCGPWinXPVisualManager::OnFillMenuImageRect (CDC* pDC,
	CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state)
{
	if (m_hThemeMenu == NULL || m_bOfficeStyleMenus)
	{
		CBCGPVisualManagerXP::OnFillMenuImageRect (pDC, pButton, rect, state);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pButton->m_nStyle & TBBS_CHECKED)
	{
		(*m_pfDrawThemeBackground) (m_hThemeMenu, pDC->GetSafeHdc(), 
			/*MENU_POPUPCHECKBACKGROUND*/12, 
			2 /* MCB_NORMAL */, &rect, 0);
	}
}
//************************************************************************************
void CBCGPWinXPVisualManager::OnDrawButtonBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, BCGBUTTON_STATE state)
{
	if (m_hThemeToolBar == NULL)
	{
		CBCGPVisualManagerXP::OnDrawButtonBorder (pDC, pButton, rect, state);
		return;
	}

	if (m_bOfficeStyleMenus)
	{
		CBCGPToolbarMenuButton* pMenuButton = 
			DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);
		if (pMenuButton != NULL)
		{
			BOOL bIsMenuBar = pMenuButton->GetParentWnd () != NULL &&
				pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar));

			BOOL bIsPopupMenu = pMenuButton->GetParentWnd () != NULL &&
				pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar));

			if (bIsPopupMenu || bIsMenuBar)
			{
				CBCGPVisualManagerXP::OnDrawButtonBorder (pDC, pButton, rect, state);
				return;
			}
		}
	}
}
//*************************************************************************************
void CBCGPWinXPVisualManager::OnDrawButtonSeparator (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state,
		BOOL bHorz)
{
	if (m_hThemeToolBar == NULL)
	{
		CBCGPVisualManagerXP::OnDrawButtonSeparator (pDC, pButton, rect, state, bHorz);
		return;
	}

	rect.InflateRect (2, 2);

	(*m_pfDrawThemeBackground) (m_hThemeToolBar, pDC->GetSafeHdc(), 
		bHorz ? TP_SEPARATOR : TP_SEPARATORVERT,
		0, &rect, 0);
}
//*************************************************************************************
void CBCGPWinXPVisualManager::OnDrawSeparator (CDC* pDC, CBCGPBaseControlBar* pBar,
										 CRect rect, BOOL bHorz)
{
	ASSERT_VALID (pBar);

	if (m_hThemeToolBar == NULL ||
		pBar->IsDialogControl ())
	{
		CBCGPVisualManagerXP::OnDrawSeparator (pDC, pBar, rect, bHorz);
		return;
	}

	CBCGPPopupMenuBar* pMenuBar = DYNAMIC_DOWNCAST (CBCGPPopupMenuBar, pBar);
	if (pMenuBar != NULL)
	{
		if (m_bOfficeStyleMenus)
		{
			CBCGPVisualManagerXP::OnDrawSeparator (pDC, pBar, rect, bHorz);
			return;
		}

		if (m_hThemeMenu != NULL)
		{
#ifndef BCGP_EXCLUDE_RIBBON
			CBCGPRibbonPanelMenuBar* pRibbonMenuBar = DYNAMIC_DOWNCAST(CBCGPRibbonPanelMenuBar, pBar);
			BOOL bRibbonPanel = pRibbonMenuBar != NULL && !pRibbonMenuBar->IsMenuMode () && 
								!pRibbonMenuBar->IsMainPanel ();
			if (bRibbonPanel)
			{
				if (bHorz && rect.Width () < 2)
				{
					rect.right++;
				}

				(*m_pfDrawThemeBackground) (m_hThemeToolBar, pDC->GetSafeHdc(), 
					bHorz ? TP_SEPARATOR : TP_SEPARATORVERT, 
					0, &rect, 0);
				return;
			}

			if (pRibbonMenuBar == NULL || !bRibbonPanel)
#endif
			{
#ifndef BCGP_EXCLUDE_RIBBON
				if (pRibbonMenuBar == NULL || !pRibbonMenuBar->IsMainPanel ())
#endif
				{
					CRect rectClient;
					pBar->GetClientRect (rectClient);

					if (pMenuBar->IsDrawGutter())
					{
						rect.left = rectClient.left + pMenuBar->GetGutterWidth ();
					}

					rect.right = pMenuBar->IsRibbonPanel () ? rect.right : rectClient.right;
				}

				rect.top = rect.CenterPoint ().y - 3;
				rect.bottom = rect.CenterPoint ().y + 3;
			}

			(*m_pfDrawThemeBackground) (m_hThemeMenu, pDC->GetSafeHdc(), 
				/*MENU_POPUPSEPARATOR */15, 
				0, &rect, 0);
			return;
		}
	}

#ifndef BCGP_EXCLUDE_RIBBON
	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPRibbonStatusBar)))
	{
		CRect rectClient;
		pBar->GetClientRect (rectClient);

		rect.top = rectClient.top;
		rect.bottom = rectClient.bottom;
	}
#endif

	if (bHorz && rect.Width () < 2)
	{
		rect.right++;
	}

	(*m_pfDrawThemeBackground) (m_hThemeToolBar, pDC->GetSafeHdc(), 
		bHorz ? TP_SEPARATOR : TP_SEPARATORVERT, 
		0, &rect, 0);
}
//***************************************************************************************
void CBCGPWinXPVisualManager::OnDrawCaptionButton (CDC* pDC, CBCGPCaptionButton* pButton, 
								BOOL bActive, BOOL bHorz, BOOL bMaximized, BOOL bDisabled,
								int nImageID /*= -1*/)
{
	if (m_hThemeToolBar == NULL)
	{
		CBCGPVisualManagerXP::OnDrawCaptionButton (pDC, pButton, bActive, bHorz, bMaximized, bDisabled, nImageID);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT (pButton != NULL);

	BOOL bPushed = pButton->m_bPushed;
	BOOL bFocused = pButton->m_bFocused || pButton->m_bDroppedDown;

	int nState = TS_NORMAL;
	if (bDisabled)
	{
		nState = TS_DISABLED;
	}
	else if (bPushed && bFocused)
	{
		nState = TS_PRESSED;
	}
	else if (bFocused)
	{
		nState = TS_HOT;
	}

	CRect rect = pButton->GetRect ();

	if (pButton->IsMiniFrameButton ())
	{
		rect.bottom -= 3;
		rect.right -= 2;
	}

	int nPart = 0;
	HTHEME hTheme = NULL;

	if (nImageID == -1)
	{
		switch (pButton->m_nHit)
		{
		case HTCLOSE:
		case HTCLOSE_BCG:

			if (pButton->IsMiniFrameButton () || globalData.bIsWindowsVista)
			{
				nPart = WP_SMALLCLOSEBUTTON;
				hTheme = m_hThemeWindow;

				if (!pButton->IsMiniFrameButton ())
				{
					rect.DeflateRect (1, 2);
				}
			}
			else
			{
				nPart = EBP_HEADERCLOSE;
				hTheme = m_hThemeExplorerBar;
			}

			break;

		case HTMAXBUTTON:
			if (!globalData.bIsWindowsVista)
			{
				nPart = EBP_HEADERPIN;
				hTheme = m_hThemeExplorerBar;

				if (!bMaximized)
				{
					nState += 3;
				}
			}
			break;
		}
	}

	if (nPart == 0 || hTheme == NULL)
	{
		if (!pButton->IsMiniFrameButton () && !globalData.bIsWindowsVista)
		{
			rect.DeflateRect (1, 2);
		}

		(*m_pfDrawThemeBackground) (m_hThemeToolBar, pDC->GetSafeHdc(), TP_BUTTON, nState, &rect, 0);

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
			CPoint ptImage (rect.left + (rect.Width () - sizeImage.cx) / 2,
							rect.top + (rect.Height () - sizeImage.cy) / 2);

			OnDrawCaptionButtonIcon (pDC, pButton, id, bActive, bDisabled, ptImage);
		}
	}
	else
	{
		(*m_pfDrawThemeBackground) (hTheme, pDC->GetSafeHdc(), nPart, nState, &rect, 0);
	}
}
//**********************************************************************************
void CBCGPWinXPVisualManager::OnDrawCaptionButtonIcon (CDC* pDC, 
													CBCGPCaptionButton* pButton,
													CBCGPMenuImages::IMAGES_IDS id,
													BOOL bActive, BOOL bDisabled,
													CPoint ptImage)
{
	BOOL bFocused = pButton->m_bFocused || pButton->m_bDroppedDown;

	if (m_hThemeButton == NULL || !bFocused)
	{
		CBCGPVisualManagerXP::OnDrawCaptionButtonIcon (pDC, pButton, id, bActive, 
													bDisabled, ptImage);
		return;
	}

	ASSERT_VALID (pDC);
	CBCGPMenuImages::Draw (pDC, id, ptImage, CBCGPMenuImages::ImageBlack);
}
//***********************************************************************************
COLORREF CBCGPWinXPVisualManager::OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected)
{
	if (m_hThemeWindow == NULL || m_bOfficeStyleMenus)
	{
		return CBCGPVisualManagerXP::OnFillCommandsListBackground (pDC, rect, bIsSelected);
	}

	::FillRect (pDC->GetSafeHdc (), rect, ::GetSysColorBrush (COLOR_MENU));

	if (bIsSelected)
	{
		pDC->FillRect (rect, &globalData.brHilite);
		pDC->Draw3dRect (rect, globalData.clrMenuText, globalData.clrMenuText);

		return globalData.clrTextHilite;
	}

	return globalData.clrMenuText;
}
//**********************************************************************************
void CBCGPWinXPVisualManager::OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive)
{
	CBCGPVisualManagerXP::OnDrawTearOffCaption (pDC, rect, bIsActive);
}
//***********************************************************************************
void CBCGPWinXPVisualManager::OnDrawMenuSystemButton (CDC* pDC, CRect rect, 
												UINT uiSystemCommand, 
												UINT nStyle, BOOL bHighlight)
{
	if (m_hThemeWindow == NULL)
	{
		CBCGPVisualManagerXP::OnDrawMenuSystemButton (pDC, rect, uiSystemCommand, nStyle, bHighlight);
		return;
	}

	int nPart;
	switch (uiSystemCommand)
	{
	case SC_CLOSE:
		nPart = WP_MDICLOSEBUTTON;
		break;

	case SC_RESTORE:
		nPart = WP_MDIRESTOREBUTTON;
		break;

	case SC_MINIMIZE:
		nPart = WP_MDIMINBUTTON;
		break;

	default:
		return;
	}

	BOOL bIsDisabled = (nStyle & TBBS_DISABLED);
	BOOL bIsPressed = (nStyle & TBBS_PRESSED);

	int nState = CBS_NORMAL;
	if (bIsDisabled)
	{
		nState = CBS_DISABLED;
	}
	else if (bIsPressed && bHighlight)
	{
		nState = CBS_PUSHED;
	}
	else if (bHighlight)
	{
		nState = CBS_HOT;
	}

	(*m_pfDrawThemeBackground) (m_hThemeWindow, pDC->GetSafeHdc(), nPart,
		nState, &rect, 0);
}
//********************************************************************************
void CBCGPWinXPVisualManager::OnDrawStatusBarPaneBorder (CDC* pDC, CBCGPStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle)
{
	if (m_hThemeStatusBar == NULL)
	{
		CBCGPVisualManagerXP::OnDrawStatusBarPaneBorder (pDC, pBar, rectPane, uiID, nStyle);
		return;
	}

	if (!(nStyle & SBPS_NOBORDERS))
	{
		(*m_pfDrawThemeBackground) (m_hThemeStatusBar, pDC->GetSafeHdc(), SP_PANE,
			0, &rectPane, 0);
	}
}
//*****************************************************************************************
void CBCGPWinXPVisualManager::OnDrawStatusBarSizeBox (CDC* pDC, CBCGPStatusBar* pStatBar,
			CRect rectSizeBox)
{
	if (m_hThemeStatusBar == NULL && m_hThemeScrollBar == NULL)
	{
		CBCGPVisualManagerXP::OnDrawStatusBarSizeBox (pDC, pStatBar, rectSizeBox);
		return;
	}

	if (m_hThemeStatusBar != NULL)
	{
		(*m_pfDrawThemeBackground) (m_hThemeStatusBar, pDC->GetSafeHdc(),
			/*SP_GRIPPER*/3, 0, &rectSizeBox, 0);
	}
	else
	{
		(*m_pfDrawThemeBackground) (m_hThemeScrollBar, pDC->GetSafeHdc(), SBP_SIZEBOX,
			globalData.m_bIsRTL ? SZB_LEFTALIGN : SZB_RIGHTALIGN, &rectSizeBox, 0);
	}
}
//***********************************************************************************
void CBCGPWinXPVisualManager::OnDrawMenuResizeBar (CDC* pDC, CRect rect, int nResizeFlags)
{
	ASSERT_VALID (pDC);

	OnFillHighlightedArea (pDC, rect, &m_brBarBkgnd, NULL);
	OnDrawMenuResizeGipper(pDC, rect, nResizeFlags, globalData.clrBarDkShadow);

	pDC->Draw3dRect(rect, m_clrMenuLight, m_clrMenuLight);
}
//**************************************************************************************
void CBCGPWinXPVisualManager::OnDrawMenuBorder (CDC* pDC, CBCGPPopupMenu* pMenu, CRect rect)
{
	if (m_hThemeWindow == NULL || m_bOfficeStyleMenus)
	{
		ASSERT_VALID (pMenu);

		BOOL bConnectMenuToParent = m_bConnectMenuToParent;
		m_bConnectMenuToParent = FALSE;

		if (m_hThemeWindow == NULL)
		{
			m_bConnectMenuToParent = TRUE;
		}
		else if (!CBCGPToolBar::IsCustomizeMode ())
		{
			CBCGPToolbarMenuButton* pMenuButton = pMenu->GetParentButton ();

			if (pMenuButton != NULL)
			{
				BOOL bIsMenuBar = pMenuButton->GetParentWnd () != NULL &&
					pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar));
				if (bIsMenuBar)
				{
					m_bConnectMenuToParent = TRUE;
				}
			}
		}

		CBCGPVisualManagerXP::OnDrawMenuBorder (pDC, pMenu, rect);

		m_bConnectMenuToParent = bConnectMenuToParent;
	}
	else
	{
		CBCGPVisualManager::OnDrawMenuBorder (pDC, pMenu, rect);
	}
}
//****************************************************************************************
void CBCGPWinXPVisualManager::OnDrawComboDropButton (CDC* pDC, CRect rect,
											    BOOL bDisabled,
												BOOL bIsDropped,
												BOOL bIsHighlighted,
												CBCGPToolbarComboBoxButton* pButton)
{
	if (m_hThemeComboBox == NULL || globalData.IsHighContastMode())
	{
		CBCGPVisualManagerXP::OnDrawComboDropButton (pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}

	int nState = bDisabled ? CBXS_DISABLED : bIsDropped ? CBXS_PRESSED : bIsHighlighted ? CBXS_HOT : CBXS_NORMAL;

	(*m_pfDrawThemeBackground) (m_hThemeComboBox, pDC->GetSafeHdc(), CP_DROPDOWNBUTTON, 
		nState, &rect, 0);
}
//*************************************************************************************
void CBCGPWinXPVisualManager::OnDrawComboBorder (CDC* pDC, CRect rect,
												BOOL bDisabled,
												BOOL bIsDropped,
												BOOL bIsHighlighted,
												CBCGPToolbarComboBoxButton* pButton)
{
	if (m_hThemeWindow == NULL)
	{
		CBCGPVisualManagerXP::OnDrawComboBorder (pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}

	if (bIsHighlighted || bIsDropped || globalData.IsHighContastMode())
	{
		rect.DeflateRect (1, 1);

		if (CBCGPToolBarImages::m_bIsDrawOnGlass)
		{
			CBCGPDrawManager dm (*pDC);
			dm.DrawRect (rect, (COLORREF)-1, globalData.clrHilite);
		}
		else
		{
			pDC->Draw3dRect (&rect,  globalData.clrHilite, globalData.clrHilite);
		}
	}
}
//**************************************************************************************
void CBCGPWinXPVisualManager::OnEraseTabsButton (CDC* pDC, CRect rect,
											  CBCGPButton* pButton,
											  CBCGPBaseTabWnd* pWndTab)
{
	if (!m_b3DTabsXPTheme || m_hThemeTab == NULL || pWndTab->IsFlatTab () || 
		pWndTab->IsOneNoteStyle () || pWndTab->IsVS2005Style ())
	{
		CBCGPVisualManagerXP::OnEraseTabsButton (pDC, rect, pButton, pWndTab);
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
		rectTabs.bottom += 2;
	}
	else
	{
		rectTabs.bottom = rectTabArea.bottom;
	}

	pWndTab->MapWindowPoints (pButton, rectTabs);

	OnEraseTabsArea (pDC, rectTabs, pWndTab);

	pDC->SelectClipRgn (NULL);
}
//**************************************************************************************
void CBCGPWinXPVisualManager::OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
												 CBCGPButton* pButton, UINT uiState,
												 CBCGPBaseTabWnd* pWndTab)
{
	if (m_hThemeToolBar == NULL)
	{
		CBCGPVisualManagerXP::OnDrawTabsButtonBorder (pDC, rect, pButton, uiState, pWndTab);
		return;
	}

	int nState = TS_NORMAL;

	if (!pButton->IsWindowEnabled ())
	{
		nState = TS_DISABLED;
	}
	else if (pButton->IsPressed () || pButton->GetCheck ())
	{
		nState = TS_PRESSED;
	}
	else if (pButton->IsHighlighted ())
	{
		nState = TS_HOT;
	}

	globalData.DrawParentBackground (pButton, pDC, rect);

	(*m_pfDrawThemeBackground) (m_hThemeToolBar, pDC->GetSafeHdc(), TP_BUTTON, nState, &rect, 0);
}
//**************************************************************************************
COLORREF CBCGPWinXPVisualManager::OnFillMiniFrameCaption (
											CDC* pDC, CRect rectCaption, 
											CBCGPMiniFrameWnd* pFrameWnd, 
											BOOL bActive)
{
	if (m_hThemeWindow == NULL)
	{
		return CBCGPVisualManager::OnFillMiniFrameCaption (pDC, rectCaption, pFrameWnd, bActive);
	}

	return globalData.clrCaptionText;
}
//**************************************************************************************
void CBCGPWinXPVisualManager::OnDrawMiniFrameBorder (
										CDC* pDC, CBCGPMiniFrameWnd* pFrameWnd,
										CRect rectBorder, CRect rectBorderSize)
{
	if (m_hThemeWindow == NULL)
	{
		CBCGPVisualManager::OnDrawMiniFrameBorder (pDC, pFrameWnd, rectBorder, rectBorderSize);
		return;
	}

	(*m_pfDrawThemeBackground) (m_hThemeWindow, pDC->GetSafeHdc(), 
		WP_SMALLCAPTION,
		0, &rectBorder, 0);

	pDC->Draw3dRect (rectBorder, globalData.clrBarFace, globalData.clrBarDkShadow);
	rectBorder.DeflateRect (1, 1);
	pDC->Draw3dRect (rectBorder, globalData.clrBarHilite, globalData.clrBarShadow);
}
//**************************************************************************************
void CBCGPWinXPVisualManager::OnDrawFloatingToolbarBorder (
												CDC* pDC, CBCGPBaseToolBar* pToolBar, 
												CRect rectBorder, CRect rectBorderSize)
{
	if (m_hThemeWindow == NULL)
	{
		CBCGPVisualManager::OnDrawFloatingToolbarBorder (pDC, pToolBar, 
												rectBorder, rectBorderSize);
		return;
	}

	(*m_pfDrawThemeBackground) (m_hThemeWindow, pDC->GetSafeHdc(), 
		WP_SMALLCAPTION,
		0, &rectBorder, 0);

	pDC->Draw3dRect (rectBorder, globalData.clrBarFace, globalData.clrBarDkShadow);
	rectBorder.DeflateRect (1, 1);
	pDC->Draw3dRect (rectBorder, globalData.clrBarHilite, globalData.clrBarShadow);
}
//**************************************************************************************
void CBCGPWinXPVisualManager::OnFillOutlookPageButton (CDC* pDC, const CRect& rectClient,
										BOOL bIsHighlighted, BOOL bIsPressed,
										COLORREF& clrText)
{
	if (m_hThemeButton == NULL)
	{
		CBCGPVisualManagerXP::OnFillOutlookPageButton (pDC, rectClient, bIsHighlighted, bIsPressed, clrText);
		return;
	}

	int nState = PBS_NORMAL;
	if (bIsHighlighted)
	{
		nState = PBS_PRESSED;
	}
	else if (bIsPressed)
	{
		nState = PBS_HOT;
	}

	CRect rect = rectClient;
	rect.InflateRect (1, 1);

	(*m_pfDrawThemeBackground) (m_hThemeButton, pDC->GetSafeHdc(), BP_PUSHBUTTON, 
		nState, &rect, 0);
}
//****************************************************************************************
void CBCGPWinXPVisualManager::OnDrawOutlookPageButtonBorder (CDC* pDC, 
							CRect& rectBtn, BOOL bIsHighlighted, BOOL bIsPressed)
{
	if (m_hThemeButton == NULL)
	{
		CBCGPVisualManagerXP::OnDrawOutlookPageButtonBorder (pDC, rectBtn, bIsHighlighted, bIsPressed);
	}
}
//**************************************************************************************
void CBCGPWinXPVisualManager::OnDrawStatusBarProgress (CDC* pDC, CBCGPStatusBar* pStatusBar,
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
CSize CBCGPWinXPVisualManager::GetButtonExtraBorder () const
{
	if (m_hThemeWindow == NULL)
	{
		return CBCGPVisualManagerXP::GetButtonExtraBorder ();
	}

	return CSize (2, 2);
}
//****************************************************************************************
CSize CBCGPWinXPVisualManager::GetCaptionButtonExtraBorder () const
{
	if (m_hThemeWindow == NULL)
	{
		return CBCGPVisualManagerXP::GetCaptionButtonExtraBorder ();
	}

	if (globalData.bIsWindows8)
	{
		return CSize (5, 5);
	}

	return CSize (1, 1);
}
//****************************************************************************************
void CBCGPWinXPVisualManager::OnDrawHeaderCtrlBorder (CBCGPHeaderCtrl* pCtrl, CDC* pDC,
		CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted)
{
	if (m_hThemeHeader == NULL)
	{
		CBCGPVisualManagerXP::OnDrawHeaderCtrlBorder (pCtrl, pDC, rect, bIsPressed, bIsHighlighted);
		return;
	}

	int nState = HIS_NORMAL;

	if (bIsPressed)
	{
		nState = HIS_PRESSED;
	}
	else if (bIsHighlighted)
	{
		nState = HIS_HOT;
	}

	if ((*m_pfDrawThemeBackground) (m_hThemeHeader, pDC->GetSafeHdc(), 
								HP_HEADERITEM, nState, &rect, 0) != S_OK)
	{
		CBCGPVisualManagerXP::OnDrawHeaderCtrlBorder (pCtrl, pDC, rect, bIsPressed, bIsHighlighted);
	}
}
//*****************************************************************************************
void CBCGPWinXPVisualManager::OnDrawHeaderCtrlSortArrow (CBCGPHeaderCtrl* pCtrl,
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
//*************************************************************************************
void CBCGPWinXPVisualManager::OnDrawEditBorder (CDC* pDC, CRect rect,
												BOOL bDisabled,
												BOOL bIsHighlighted,
												CBCGPToolbarEditBoxButton* pButton)
{
	if (m_hThemeComboBox == NULL)
	{
		CBCGPVisualManagerXP::OnDrawEditBorder (pDC, rect, bDisabled, bIsHighlighted, pButton);
		return;
	}

	if (!pButton->IsFlatMode ())
	{
		COLORREF clrBorder = (COLORREF)-1;

		if ((*m_pfGetThemeColor) (m_hThemeComboBox, 5, 0, TMT_BORDERCOLOR, &clrBorder) != S_OK)
		{
			CBCGPVisualManagerXP::OnDrawEditBorder (pDC, rect, bDisabled, bIsHighlighted, pButton);
			return;
		}

		if (CBCGPToolBarImages::m_bIsDrawOnGlass)
		{
			CBCGPDrawManager dm (*pDC);
			dm.DrawRect (rect, (COLORREF)-1, clrBorder);
		}
		else
		{
			pDC->Draw3dRect (&rect,  clrBorder, clrBorder);
		}

		return;
	}

	if (bIsHighlighted)
	{
		if (CBCGPToolBarImages::m_bIsDrawOnGlass)
		{
			CBCGPDrawManager dm (*pDC);
			dm.DrawRect (rect, (COLORREF)-1, globalData.clrBtnHilite);
		}
		else
		{
			pDC->Draw3dRect (&rect,  globalData.clrBtnHilite, globalData.clrBtnHilite);
		}
	}
}

#ifndef BCGP_EXCLUDE_TASK_PANE

void CBCGPWinXPVisualManager::OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea)
{
	ASSERT_VALID (pDC);

	if (m_hThemeExplorerBar == NULL)
	{
		CBCGPVisualManagerXP::OnFillTasksPaneBackground (pDC, rectWorkArea);
		return;
	}

	(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_HEADERBACKGROUND,
		0, &rectWorkArea, 0);
}
//**********************************************************************************
void CBCGPWinXPVisualManager::OnDrawTasksGroupCaption(CDC* pDC, CBCGPTasksGroup* pGroup,
								BOOL bIsHighlighted, BOOL bIsSelected, BOOL bCanCollapse)
{
	ASSERT_VALID(pDC);
	ASSERT(pGroup != NULL);
	ASSERT_VALID (pGroup->m_pPage);

#ifndef BCGP_EXCLUDE_TOOLBOX
	BOOL bIsToolBox = pGroup->m_pPage->m_pTaskPane != NULL &&
		(pGroup->m_pPage->m_pTaskPane->IsKindOf (RUNTIME_CLASS (CBCGPToolBoxEx)));
#else
	BOOL bIsToolBox = FALSE;
#endif

	if (m_hThemeExplorerBar == NULL || bIsToolBox)
	{
		CBCGPVisualManagerXP::OnDrawTasksGroupCaption (pDC, pGroup, bIsHighlighted, bIsSelected, bCanCollapse);
		return;
	}

	if (pGroup->m_strName.IsEmpty())
	{
		return;
	}

	// -------------------------------
	// Draw group caption (Windows XP)
	// -------------------------------
	if (pGroup->m_bIsSpecial)
	{
		(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_SPECIALGROUPHEAD,
			0, &pGroup->m_rect, 0);
	}
	else
	{
		(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_NORMALGROUPHEAD,
			0, &pGroup->m_rect, 0);
	}

	// ---------------------------
	// Draw an icon if it presents
	// ---------------------------
	BOOL bShowIcon = (pGroup->m_hIcon != NULL 
		&& pGroup->m_sizeIcon.cx < pGroup->m_rect.Width () - pGroup->m_rect.Height());
	if (bShowIcon)
	{
		OnDrawTasksGroupIcon(pDC, pGroup, 5, bIsHighlighted, bIsSelected, bCanCollapse);
	}
	
	// -----------------------
	// Draw group caption text
	// -----------------------
	CFont* pFontOld = pDC->SelectObject (&globalData.fontBold);
	COLORREF clrTextOld = pDC->GetTextColor();
	if (bCanCollapse && bIsHighlighted)
	{
		if (pGroup->m_bIsSpecial)
		{
			pDC->SetTextColor (pGroup->m_clrTextHot == (COLORREF)-1 ? 
				globalData.clrWindow : pGroup->m_clrTextHot);
		}
		else
		{
			pDC->SetTextColor (pGroup->m_clrTextHot == (COLORREF)-1 ? 
				m_clrTaskPaneHotText : pGroup->m_clrTextHot);
		}
	}
	else
	{
		if (pGroup->m_bIsSpecial)
		{
			pDC->SetTextColor (pGroup->m_clrText == (COLORREF)-1 ? 
				globalData.clrWindow : pGroup->m_clrText);
		}
		else
		{
			pDC->SetTextColor (pGroup->m_clrText == (COLORREF)-1 ? 
				m_clrTaskPaneHotText : pGroup->m_clrText);
		}
	}
	int nBkModeOld = pDC->SetBkMode(TRANSPARENT);
	
	int nTaskPaneHOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionHorzOffset();
	int nTaskPaneVOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionVertOffset();
	int nCaptionHOffset = (nTaskPaneHOffset != -1 ? nTaskPaneHOffset : m_nGroupCaptionHorzOffset);
	
	CRect rectText = pGroup->m_rect;
	rectText.left += (bShowIcon ? pGroup->m_sizeIcon.cx	+ 5: nCaptionHOffset);
	rectText.top += (nTaskPaneVOffset != -1 ? nTaskPaneVOffset : m_nGroupCaptionVertOffset);
	rectText.right = max(rectText.left, 
						rectText.right - (bCanCollapse ? pGroup->m_rect.Height() : nCaptionHOffset));

	pDC->DrawText (pGroup->m_strName, rectText, DT_SINGLELINE | DT_END_ELLIPSIS);

	pDC->SetBkMode(nBkModeOld);
	pDC->SelectObject (pFontOld);
	pDC->SetTextColor (clrTextOld);

	// -------------------------
	// Draw group caption button
	// -------------------------
	if (bCanCollapse)
	{
		CRect rectButton = pGroup->m_rect;
		rectButton.left = max(rectButton.left, rectButton.right - rectButton.Height());
		
		if (pGroup->m_bIsSpecial)
		{
			if (!pGroup->m_bIsCollapsed)
			{
				if (bIsHighlighted)
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_SPECIALGROUPCOLLAPSE, EBSGC_HOT, &rectButton, 0);
				}
				else
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_SPECIALGROUPCOLLAPSE, EBSGC_NORMAL, &rectButton, 0);
				}
			}
			else
			{
				if (bIsHighlighted)
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_SPECIALGROUPEXPAND, EBSGE_HOT, &rectButton, 0);
				}
				else
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_SPECIALGROUPEXPAND, EBSGE_NORMAL, &rectButton, 0);
				}
			}
		}
		else
		{
			if (!pGroup->m_bIsCollapsed)
			{
				if (bIsHighlighted)
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_NORMALGROUPCOLLAPSE, EBNGC_HOT, &rectButton, 0);
				}
				else
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_NORMALGROUPCOLLAPSE, EBNGC_NORMAL, &rectButton, 0);
				}
			}
			else
			{
				if (bIsHighlighted)
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_NORMALGROUPEXPAND, EBNGE_HOT, &rectButton, 0);
				}
				else
				{
					(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
						EBP_NORMALGROUPEXPAND, EBNGE_NORMAL, &rectButton, 0);
				}
			}
		}
	}
}
//**********************************************************************************
void CBCGPWinXPVisualManager::OnFillTasksGroupInterior(CDC* pDC, CRect rect, BOOL bSpecial)
{
	ASSERT_VALID(pDC);

	if (m_hThemeExplorerBar == NULL)
	{
		CBCGPVisualManagerXP::OnFillTasksGroupInterior (pDC, rect);
		return;
	}

	if (!bSpecial)
	{
		(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
			EBP_NORMALGROUPBACKGROUND, 0, &rect, 0);
	}
	else
	{
		(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
			EBP_SPECIALGROUPBACKGROUND, 0, &rect, 0);
	}
}
//**********************************************************************************
void CBCGPWinXPVisualManager::OnDrawTasksGroupAreaBorder(CDC* pDC, CRect rect, BOOL bSpecial, 
														BOOL bNoTitle)
{
	if (m_hThemeExplorerBar == NULL)
	{
		CBCGPVisualManagerXP::OnDrawTasksGroupAreaBorder(pDC, rect, bSpecial, bNoTitle);
		return;
	}

	ASSERT_VALID(pDC);

	// Draw underline
	if (bNoTitle)
	{
		CRect rectDraw = rect;
		rectDraw.bottom = rectDraw.top + 1;

		if (bSpecial)
		{
			(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_SPECIALGROUPHEAD,
				0, &rectDraw, 0);
		}
		else
		{
			(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_NORMALGROUPHEAD,
				0, &rectDraw, 0);
		}
	}
	
	return;
}
//**********************************************************************************
void CBCGPWinXPVisualManager::OnDrawTask(CDC* pDC, CBCGPTask* pTask, CImageList* pIcons, 
										BOOL bIsHighlighted, BOOL bIsSelected)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pIcons);
	ASSERT(pTask != NULL);

	if (m_hThemeExplorerBar == NULL)
	{
		CBCGPVisualManagerXP::OnDrawTask (pDC, pTask, pIcons, bIsHighlighted, bIsSelected);
		return;
	}

	if (pTask->m_bIsSeparator)
	{
		// --------------
		// Draw separator
		// --------------
		CRect rectDraw = pTask->m_rect;
		rectDraw.top = pTask->m_rect.CenterPoint ().y;
		rectDraw.bottom = rectDraw.top + 1;

		// draw same as group caption
		(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), EBP_NORMALGROUPHEAD,
			0, &rectDraw, 0);
		return;
	}
	
	// ---------
	// Draw icon
	// ---------
	CRect rectText = pTask->m_rect;
	CSize sizeIcon(0, 0);
	::ImageList_GetIconSize (pIcons->m_hImageList, (int*) &sizeIcon.cx, (int*) &sizeIcon.cy);
	if (pTask->m_nIcon >= 0 && sizeIcon.cx > 0)
	{
		pIcons->Draw (pDC, pTask->m_nIcon, rectText.TopLeft (), ILD_TRANSPARENT);
	}
	int nTaskPaneOffset = pTask->m_pGroup->m_pPage->m_pTaskPane->GetTasksIconHorzOffset();
	rectText.left += sizeIcon.cx + (nTaskPaneOffset != -1 ? nTaskPaneOffset : m_nTasksIconHorzOffset);

	// ---------
	// Draw text
	// ---------
	BOOL bIsLabel = (pTask->m_uiCommandID == 0);

	CFont* pFontOld = NULL;
	COLORREF clrTextOld = pDC->GetTextColor();
	if (bIsLabel)
	{
		pFontOld = pDC->SelectObject (
			pTask->m_bIsBold ? &globalData.fontBold : &globalData.fontRegular);
		pDC->SetTextColor (pTask->m_clrText == (COLORREF)-1 ?
			globalData.clrWindowText : pTask->m_clrText);
	}
	else if (!pTask->m_bEnabled)
	{
		pDC->SetTextColor (globalData.clrGrayedText);
		pFontOld = pDC->SelectObject (&globalData.fontRegular);
	}
	else if (bIsHighlighted)
	{
		pFontOld = pDC->SelectObject (&globalData.fontUnderline);
		pDC->SetTextColor (pTask->m_clrTextHot == (COLORREF)-1 ?
			m_clrTaskPaneHotText : pTask->m_clrTextHot);
	}
	else
	{
		pFontOld = pDC->SelectObject (&globalData.fontRegular);
		pDC->SetTextColor (pTask->m_clrText == (COLORREF)-1 ?
			m_clrTaskPaneHotText : pTask->m_clrText);
	}

	int nBkModeOld = pDC->SetBkMode(TRANSPARENT);

	CBCGPTasksPane* pTaskPane = pTask->m_pGroup->m_pPage->m_pTaskPane;
	ASSERT_VALID (pTaskPane);

	BOOL bMultiline = bIsLabel ? 
		pTaskPane->IsWrapLabelsEnabled () : pTaskPane->IsWrapTasksEnabled ();

	if (bMultiline)
	{
		pDC->DrawText (pTask->m_strName, rectText, DT_WORDBREAK);
	}
	else
	{
		CString strText = pTask->m_strName;
		strText.Remove (_T('\n'));
		strText.Remove (_T('\r'));
		pDC->DrawText (strText, rectText, DT_SINGLELINE | DT_VCENTER);
	}

	pDC->SetBkMode(nBkModeOld);
	pDC->SelectObject (pFontOld);
	pDC->SetTextColor (clrTextOld);
}
//************************************************************************************
void CBCGPWinXPVisualManager::OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited)
{
	ASSERT_VALID (pDC);

	CRect rectImage (CPoint (0, 0), CBCGPMenuImages::Size ());

	CRect rectFill = rect;
	rectFill.top -= nBorderSize;

	pDC->FillRect (rectFill, &globalData.brBarFace);

	if (bHilited)
	{
		CBCGPDrawManager dm (*pDC);
		dm.HighlightRect (rect);

		pDC->Draw3dRect (rect,
			globalData.clrBarHilite,
			globalData.clrBarDkShadow);
	}

	CBCGPMenuImages::Draw (pDC, (CBCGPMenuImages::IMAGES_IDS) iImage, rect);
}

#endif // BCGP_EXCLUDE_TASK_PANE

void CBCGPWinXPVisualManager::OnDrawExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox)
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
//**********************************************************************************
void CBCGPWinXPVisualManager::OnDrawCheckBoxEx (CDC *pDC, CRect rect, 
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
//**********************************************************************************
void CBCGPWinXPVisualManager::OnDrawRadioButton (CDC *pDC, CRect rect, 
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
//*************************************************************************************
void CBCGPWinXPVisualManager::OnDrawControlBorder (CDC* pDC, CRect rect, CWnd* pWndCtrl, BOOL bDrawOnGlass)
{
	if (m_hThemeComboBox == NULL)
	{
		CBCGPVisualManagerXP::OnDrawControlBorder (pDC, rect, pWndCtrl, bDrawOnGlass);
		return;
	}

	ASSERT_VALID (pDC);

	COLORREF clrBorder = (COLORREF)-1;

	if ((*m_pfGetThemeColor) (m_hThemeComboBox, 5, 0, TMT_BORDERCOLOR, &clrBorder) != S_OK)
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
//********************************************************************************
BOOL CBCGPWinXPVisualManager::OnDrawCalculatorButton (CDC* pDC, 
	CRect rect, CBCGPToolbarButton* pButton, 
	CBCGPVisualManager::BCGBUTTON_STATE state, 
	int cmd /* CBCGPCalculator::CalculatorCommands */,
	CBCGPCalculator* pCalculator)
{
	if (m_hThemeButton == NULL)
	{
		return CBCGPVisualManagerXP::OnDrawCalculatorButton (pDC, rect, pButton, state, cmd, pCalculator);
	}

	ASSERT_VALID (pButton);
	ASSERT_VALID (pDC);

	int nState = PBS_NORMAL;

	switch (state)
	{
	case ButtonsIsPressed:
		nState = PBS_PRESSED;
		break;

	case ButtonsIsHighlighted:
		nState = PBS_HOT;
		break;
	}

	(*m_pfDrawThemeBackground) (m_hThemeButton, pDC->GetSafeHdc(), BP_PUSHBUTTON, 
		nState, &rect, 0);
	
	return TRUE;
}
//*********************************************************************************
BOOL CBCGPWinXPVisualManager::OnDrawBrowseButton (CDC* pDC, CRect rect, 
											 CBCGPEdit* pEdit, 
											 CBCGPVisualManager::BCGBUTTON_STATE state,
											 COLORREF& clrText)
{
	if (m_hThemeButton == NULL)
	{
		return CBCGPVisualManagerXP::OnDrawBrowseButton (pDC, rect, 
			pEdit, state, clrText);
	}

	ASSERT_VALID (pDC);

	rect.InflateRect(0, 1);

	if (pEdit != NULL && pEdit->m_bOnGlass)
	{
		CBCGPDrawManager dm(*pDC);
		dm.DrawRect(rect, globalData.clrWindow, (COLORREF)-1);
	}
	else
	{
		pDC->FillRect (rect, &globalData.brWindow);
	}

	int nState = PBS_NORMAL;

	switch (state)
	{
	case ButtonsIsPressed:
		nState = PBS_PRESSED;
		break;

	case ButtonsIsHighlighted:
		nState = PBS_HOT;
		break;
	}

	(*m_pfDrawThemeBackground) (m_hThemeButton, pDC->GetSafeHdc(), BP_PUSHBUTTON,
		nState, &rect, 0);
	
	return TRUE;
}
//**********************************************************************************
void CBCGPWinXPVisualManager::OnDrawSpinButtons (CDC* pDC, CRect rect, 
	int nState, BOOL bOrientation, CBCGPSpinButtonCtrl* pSpinCtrl)
{
	if (m_hThemeSpin == NULL || pSpinCtrl->IsRibbonControl ())
	{
		CBCGPVisualManagerXP::OnDrawSpinButtons (pDC, rect, nState, bOrientation, pSpinCtrl);
		return;
	}

	const int nMinSize = 17; // If part size is less then give, Windows Theme manager doesn't draw this part properly

	if ((bOrientation && rect.Width() < nMinSize) || (!bOrientation && rect.Height() < nMinSize))
	{
		CBCGPVisualManagerXP::OnDrawSpinButtons (pDC, rect,  nState, bOrientation, pSpinCtrl);
		return;
	}

	// Draw up part:
	CRect rectUp = rect;
	if (bOrientation)
	{
		rectUp.left = rect.CenterPoint ().x;
	}
	else
	{
		rectUp.bottom = rect.CenterPoint ().y;
	}

	int nDrawState = UPS_NORMAL;

	if (nState & SPIN_DISABLED)
	{
		nDrawState = UPS_DISABLED;
	}
	else if (nState & SPIN_PRESSEDUP)
	{
		nDrawState = UPS_PRESSED;
	}
	else if (nState & SPIN_HIGHLIGHTEDUP)
	{
		nDrawState = UPS_HOT;
	}

	(*m_pfDrawThemeBackground) (m_hThemeSpin, pDC->GetSafeHdc(), 
		bOrientation ? SPNP_UPHORZ : SPNP_UP, nDrawState, &rectUp, 0);

	// Draw down part:
	CRect rectDown = rect;
	if (bOrientation)
	{
		rectDown.right = rect.CenterPoint ().x;
	}
	else
	{
		rectDown.top = rect.CenterPoint ().y;
	}

	nDrawState = UPS_NORMAL;

	if (nState & SPIN_DISABLED)
	{
		nDrawState = UPS_DISABLED;
	}
	else if (nState & SPIN_PRESSEDDOWN)
	{
		nDrawState = UPS_PRESSED;
	}
	else if (nState & SPIN_HIGHLIGHTEDDOWN)
	{
		nDrawState = UPS_HOT;
	}

	(*m_pfDrawThemeBackground) (m_hThemeSpin, pDC->GetSafeHdc(), 
		bOrientation ? SPNP_DOWNHORZ : SPNP_DOWN, nDrawState, &rectDown, 0);
}
//**********************************************************************************
void CBCGPWinXPVisualManager::OnDrawDateTimeDropButton (CDC* pDC, CRect rect, 
	BOOL bDisabled, BOOL bPressed, BOOL bHighlighted, CBCGPDateTimeCtrl* pCtrl)
{
	if (m_hThemeComboBox == NULL)
	{
		CBCGPVisualManagerXP::OnDrawDateTimeDropButton (pDC, rect, bDisabled, bPressed, bHighlighted, pCtrl);
		return;
	}

	int nState = bDisabled ? CBXS_DISABLED : bPressed ? CBXS_PRESSED : bHighlighted ? CBXS_HOT : CBXS_NORMAL;

	if (pCtrl == NULL || !pCtrl->IsPropListMode())
	{
		rect.DeflateRect(1, 1, 0, 1);
	}
	else
	{
		rect.InflateRect(0, 1);
	}

	(*m_pfDrawThemeBackground) (m_hThemeComboBox, pDC->GetSafeHdc(), CP_DROPDOWNBUTTON, 
		nState, &rect, 0);
}
//*************************************************************************************
void CBCGPWinXPVisualManager::OnDrawTab (CDC* pDC, CRect rectTab,
						int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pTabWnd);
	ASSERT_VALID (pDC);

	if (!m_b3DTabsXPTheme || m_hThemeTab == NULL || pTabWnd->IsFlatTab () || 
		pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style () ||
		pTabWnd->IsLeftRightRounded () || pTabWnd->IsPointerStyle())
	{
		CBCGPVisualManagerXP::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);
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

	rectTab.right++;

	if (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_TOP && !bIsActive)
	{
		rectTab.bottom--;
	}

	(*m_pfDrawThemeBackground) (m_hThemeTab, pDC->GetSafeHdc(), TABP_TABITEM, nState, &rectTab, 0);

	if (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
	{
		CBCGPDrawManager dm (*pDC);
		dm.MirrorRect (rectTab, FALSE);
	}

	COLORREF clrTabText = pTabWnd->GetTabTextColor(iTab);
	if (clrTabText == (COLORREF)-1)
	{
		if (m_pfGetThemeColor != NULL)
		{
			(*m_pfGetThemeColor) (m_hThemeTab, TABP_TABITEM, nState, TMT_TEXTCOLOR, &clrTabText);
		}
		else
		{
			clrTabText = globalData.clrWindowText;
		}
	}
	
	COLORREF cltTextOld = pDC->SetTextColor (clrTabText);

	OnDrawTabContent (pDC, rectTab, iTab, bIsActive, pTabWnd, (COLORREF)-1);

	pDC->SetTextColor (cltTextOld);
}
//**********************************************************************************
void CBCGPWinXPVisualManager::OnDrawTabCloseButton (CDC* pDC, CRect rect, 
											   const CBCGPBaseTabWnd* pTabWnd,
											   BOOL bIsHighlighted,
											   BOOL bIsPressed,
											   BOOL bIsDisabled)
{
	if (m_hThemeWindow == NULL)
	{
		CBCGPVisualManagerXP::OnDrawTabCloseButton (pDC, rect, pTabWnd,
											   bIsHighlighted, bIsPressed, bIsDisabled);
		return;
	}

	ASSERT_VALID (pDC);

	int nState = TS_NORMAL;
	if (bIsDisabled)
	{
		nState = TS_DISABLED;
	}
	else if (bIsPressed && bIsHighlighted)
	{
		nState = TS_PRESSED;
	}
	else if (bIsHighlighted)
	{
		nState = TS_HOT;
	}

	(*m_pfDrawThemeBackground) (m_hThemeWindow, pDC->GetSafeHdc(), WP_SMALLCLOSEBUTTON, nState, &rect, 0);
}
//***********************************************************************************
void CBCGPWinXPVisualManager::OnEraseTabsArea (CDC* pDC, CRect rect, 
										 const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (!m_b3DTabsXPTheme || m_hThemeTab == NULL || pTabWnd->IsFlatTab () || 
		pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style () ||
		pTabWnd->IsDialogControl () || pTabWnd->IsPointerStyle())
	{
		CBCGPVisualManagerXP::OnEraseTabsArea (pDC, rect, pTabWnd);
		return;
	}

	rect.right += 10;

	if (pTabWnd->GetLocation () == CBCGPBaseTabWnd::LOCATION_BOTTOM)
	{
		rect.top -= 3;

		CBCGPMemDC memDC (*pDC, (CWnd*) pTabWnd);

		(*m_pfDrawThemeBackground) (m_hThemeTab, memDC.GetDC ().GetSafeHdc (), TABP_PANE, 0, &rect, 
			NULL);

		CBCGPDrawManager dm (memDC.GetDC ());
		dm.MirrorRect (rect, FALSE);
	}
	else
	{
		rect.bottom += 2;

		(*m_pfDrawThemeBackground) (m_hThemeTab, pDC->GetSafeHdc(), TABP_PANE, 0, &rect, 0);
	}
}
//***********************************************************************************
BOOL CBCGPWinXPVisualManager::OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (!m_b3DTabsXPTheme || m_hThemeTab == NULL || pTabWnd->IsFlatTab () || 
		pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style ())
	{
		return CBCGPVisualManagerXP::OnEraseTabsFrame (pDC, rect, pTabWnd);
	}

	return FALSE;
}

#ifndef BCGP_EXCLUDE_POPUP_WINDOW

void CBCGPWinXPVisualManager::OnErasePopupWindowButton (CDC* pDC, CRect rect, CBCGPPopupWndButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (m_hThemeButton == NULL || pButton->IsCaptionButton ())
	{
		CBCGPVisualManagerXP::OnErasePopupWindowButton (pDC, rect, pButton);
		return;
	}

	CRect rectParent;
	pButton->GetParent ()->GetClientRect (rectParent);

	pButton->GetParent ()->MapWindowPoints (pButton, rectParent);
	OnFillPopupWindowBackground (pDC, rectParent);
}
//**********************************************************************************
COLORREF CBCGPWinXPVisualManager::GetPopupWindowCaptionTextColor(CBCGPPopupWindow* pPopupWnd, BOOL bButton)
{
	if (m_hThemeButton == NULL || !bButton)
	{
		return CBCGPVisualManagerXP::GetPopupWindowCaptionTextColor(pPopupWnd, bButton);
	}
	
	return globalData.clrBarText;
}
//**********************************************************************************
void CBCGPWinXPVisualManager::OnDrawPopupWindowButtonBorder (CDC* pDC, CRect rect, CBCGPPopupWndButton* pButton)
{
	ASSERT_VALID (pButton);

	int nState = PBS_NORMAL;

	if (!pButton->IsWindowEnabled ())
	{
		nState = PBS_DISABLED;
	}
	else if (pButton->IsPressed () || pButton->GetCheck ())
	{
		nState = PBS_PRESSED;
	}
	else if (pButton->IsHighlighted ())
	{
		nState = PBS_HOT;
	}
	else
	{
		nState = PBS_NORMAL;
	}

	if (m_hThemeWindow != NULL && pButton->IsCloseButton () && pButton->IsCaptionButton ())
	{
		(*m_pfDrawThemeBackground) (m_hThemeWindow, pDC->GetSafeHdc(), 
			WP_SMALLCLOSEBUTTON, nState, &rect, 0);
		return;
	}

	if (m_hThemeButton == NULL)
	{
		CBCGPVisualManagerXP::OnDrawPopupWindowButtonBorder (pDC, rect, pButton);
		return;
	}

	globalData.DrawParentBackground (pButton, pDC, rect);
	(*m_pfDrawThemeBackground) (m_hThemeButton, pDC->GetSafeHdc(), BP_PUSHBUTTON, nState, &rect, 0);
}
//**********************************************************************************
BOOL CBCGPWinXPVisualManager::IsDefaultWinXPPopupButton (CBCGPPopupWndButton* pButton) const
{
	ASSERT_VALID (pButton);
	return m_hThemeWindow != NULL && pButton->IsCloseButton () && pButton->IsCaptionButton ();
}

#endif // BCGP_EXCLUDE_POPUP_WINDOW

#ifndef BCGP_EXCLUDE_GRID_CTRL

void CBCGPWinXPVisualManager::OnFillGridHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect)
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
//********************************************************************************
BOOL CBCGPWinXPVisualManager::OnDrawGridHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed)
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
//********************************************************************************
void CBCGPWinXPVisualManager::OnFillGridRowHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		m_hThemeHeader == NULL)
	{
		CBCGPVisualManagerXP::OnFillGridRowHeaderBackground (pCtrl, pDC, rect);
		return;
	}

	ASSERT_VALID(pDC);
	pDC->FillRect (rect, &globalData.brBtnFace);
}
//********************************************************************************
BOOL CBCGPWinXPVisualManager::OnDrawGridRowHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		m_hThemeHeader == NULL)
	{
		return CBCGPVisualManagerXP::OnDrawGridRowHeaderItemBorder (pCtrl, pDC, rect, bPressed);
	}

	ASSERT_VALID (pDC);
	(*m_pfDrawThemeBackground) (m_hThemeHeader, pDC->GetSafeHdc(), 
								HP_HEADERITEMLEFT, HIS_NORMAL, &rect, 0);
	return TRUE;
}
//********************************************************************************
void CBCGPWinXPVisualManager::OnFillGridSelectAllAreaBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL /*bPressed*/)
{
	OnFillGridHeaderBackground (pCtrl, pDC, rect);
}
//********************************************************************************
void CBCGPWinXPVisualManager::OnDrawGridSelectAllMarker(CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, int nPadding, BOOL bPressed)
{
	if (m_hThemeStatusBar == NULL)
	{
		CBCGPVisualManagerXP::OnDrawGridSelectAllMarker(pCtrl, pDC, rect, nPadding, bPressed);
		return;
	}

	rect.left -= nPadding;
	rect.top -= nPadding;

	(*m_pfDrawThemeBackground) (m_hThemeStatusBar, pDC->GetSafeHdc(), /*SP_GRIPPER*/3, 0, &rect, 0);
}
//********************************************************************************
BOOL CBCGPWinXPVisualManager::OnDrawGridSelectAllAreaBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed)
{
	return OnDrawGridHeaderItemBorder (pCtrl, pDC, rect, bPressed);
}
//********************************************************************************
void CBCGPWinXPVisualManager::OnDrawGridHeaderMenuButton (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, 
		BOOL bHighlighted, BOOL bPressed, BOOL bDisabled)
{
	ASSERT_VALID (pDC);

	if (m_hThemeToolBar == NULL)
	{
		CBCGPVisualManagerXP::OnDrawGridHeaderMenuButton (pCtrl, pDC, rect, bHighlighted, bPressed, bDisabled);
		return;
	}

	rect.DeflateRect (1, 1);

	if (bHighlighted)
	{
		(*m_pfDrawThemeBackground) (m_hThemeToolBar, pDC->GetSafeHdc(), TP_BUTTON, TS_HOT, &rect, 0);
	}
}
//********************************************************************************
BOOL CBCGPWinXPVisualManager::OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme)
{
	CBCGPVisualManagerXP::OnSetGridColorTheme (pCtrl, theme);

	if (m_hThemeMenu != NULL)
	{
		theme.m_SelColors.m_clrBackground = m_clrHighlight;
		theme.m_SelColors.m_clrGradient = m_clrMenuLight;
		theme.m_SelColors.m_clrBorder = m_clrHighlightDn;
		(*m_pfGetThemeColor) (m_hThemeMenu, MENU_POPUPITEM, MPI_HOT, TMT_TEXTCOLOR, &theme.m_SelColors.m_clrText);

		theme.m_GroupSelColors.m_clrBackground = theme.m_SelColors.m_clrBackground;
		theme.m_GroupSelColors.m_clrGradient = m_clrMenuLight;
		theme.m_GroupSelColors.m_clrBorder = theme.m_SelColors.m_clrBorder;
		theme.m_GroupSelColors.m_clrText = theme.m_SelColors.m_clrText;
	}

	return TRUE;
}

#endif // BCGP_EXCLUDE_GRID_CTRL

//********************************************************************************

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

//********************************************************************************
void CBCGPWinXPVisualManager::DrawGanttHeaderCell (const CBCGPGanttChart* pChart, CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, BOOL bHilite)
{
    if (m_hThemeHeader == NULL)
    {
        CBCGPVisualManagerXP::DrawGanttHeaderCell (pChart, dc, cellInfo, bHilite);
        return;
    }

    (*m_pfDrawThemeBackground) (m_hThemeHeader, dc.GetSafeHdc (), HP_HEADERITEM, HIS_NORMAL, cellInfo.rectCell, 0);
}
//********************************************************************************
void CBCGPWinXPVisualManager::FillGanttBar (const CBCGPGanttItem* pItem, CDC& dc, const CRect& rectFill, COLORREF color, double dGlowLine)
{
    if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
    {
        CBCGPVisualManager::FillGanttBar (pItem, dc, rectFill, color, dGlowLine);
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

//********************************************************************************


void CBCGPWinXPVisualManager::GetCalendarColors (const CBCGPCalendar* pCalendar,
				   CBCGPCalendarColors& colors)
{
	CBCGPVisualManagerXP::GetCalendarColors (pCalendar, colors);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () ||
		m_clrCalendarCaption == (COLORREF)-1 ||
		m_clrCalendarCaptionText == (COLORREF)-1)
	{
		return;
	}

	colors.clrCaption = m_clrCalendarCaption;

	if (pCalendar->GetSafeHwnd() != NULL && !pCalendar->IsWindowEnabled())
	{
		colors.clrCaptionText = globalData.clrGrayedText;
	}
	else
	{
		colors.clrCaptionText = m_clrCalendarCaptionText;
	}
}
//**************************************************************************************
COLORREF CBCGPWinXPVisualManager::OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
			BOOL bActive, CRect rectCaption, CRect rectButtons)
{
	if (m_hThemeExplorerBar == NULL || m_pfGetThemeColor == NULL)
	{
		return CBCGPVisualManagerXP::OnDrawControlBarCaption (pDC, pBar, 
			bActive, rectCaption, rectButtons);
	}

	if (globalData.bIsWindowsVista)
	{
		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rectCaption, 
			bActive ? globalData.clrActiveCaptionGradient : globalData.clrInactiveCaptionGradient, 
			bActive ? globalData.clrActiveCaption : globalData.clrInactiveCaption, 
			TRUE);

		return bActive ? globalData.clrCaptionText : globalData.clrInactiveCaptionText;
	}

	(*m_pfDrawThemeBackground) (m_hThemeExplorerBar, pDC->GetSafeHdc(), 
		EBP_HEADERBACKGROUND,
		0, &rectCaption, 0);

	COLORREF clrText;
	if ((*m_pfGetThemeColor) (m_hThemeExplorerBar, EBP_HEADERBACKGROUND, 0, TMT_TEXTCOLOR, &clrText) != S_OK)
	{
		clrText = globalData.clrInactiveCaptionText;
	}

	if (bActive)
	{
		CBCGPDrawManager dm (*pDC);
		int nPercentage = 110;

		if (GetRValue (clrText) > 100 &&
			GetGValue (clrText) > 100 &&
			GetBValue (clrText) > 100)
		{
			nPercentage = 80;
		}

		dm.HighlightRect (rectCaption, nPercentage);
	}

	return clrText;
}

#ifndef BCGP_EXCLUDE_TOOLBOX

BOOL CBCGPWinXPVisualManager::OnEraseToolBoxButton (CDC* pDC, CRect rect,
											CBCGPToolBoxButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (m_hThemeToolBar == NULL)
	{
		return CBCGPVisualManagerXP::OnEraseToolBoxButton (pDC, rect, pButton);
	}

	if (pButton->IsHighlighted () || pButton->GetCheck ())
	{
		int nState = TS_NORMAL;
		
		if (pButton->GetCheck ())
		{
			nState = (pButton->IsHighlighted ()) ? TS_HOTCHECKED : TS_CHECKED;
		}
		else if (pButton->IsHighlighted ())
		{
			nState = TS_HOT;
		}

		(*m_pfDrawThemeBackground) (m_hThemeToolBar, pDC->GetSafeHdc(), TP_BUTTON, nState, &rect, 0);
	}

	return TRUE;
}
//**********************************************************************************
BOOL CBCGPWinXPVisualManager::OnDrawToolBoxButtonBorder (CDC* pDC, CRect& rect, 
												 CBCGPToolBoxButton* pButton, UINT uiState)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (m_hThemeToolBar == NULL)
	{
		return CBCGPVisualManagerXP::OnDrawToolBoxButtonBorder (pDC, rect, pButton, uiState);
	}

	return TRUE;
}

#endif


BOOL CBCGPWinXPVisualManager::OnDrawPushButton (CDC* pDC, CRect rect, CBCGPButton* pButton, COLORREF& clrText)
{
	clrText = globalData.clrBtnText;
	return DrawPushButton (pDC, rect, pButton, 0);
}

#ifndef BCGP_EXCLUDE_RIBBON

void CBCGPWinXPVisualManager::OnDrawRibbonProgressBar (CDC* pDC, 
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
//***********************************************************************************
void CBCGPWinXPVisualManager::OnDrawRibbonPaletteButton (
					CDC* pDC, 
					CBCGPRibbonPaletteIcon* pButton)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	OnFillRibbonButton (pDC, pButton);

	CRect rectButton = pButton->GetRect();
	COLORREF clrBorder = globalData.clrBarShadow;

	if (!pButton->IsDisabled() && (pButton->IsHighlighted () || pButton->IsDroppedDown () || pButton->IsFocused()))
	{
		pDC->Draw3dRect(rectButton, clrBorder, clrBorder);
	}
	else
	{
		CBCGPPenSelector pen(*pDC, globalData.clrBarShadow);

		pDC->MoveTo(rectButton.left, rectButton.top);
		pDC->LineTo(rectButton.left, rectButton.bottom);
	}
}

#endif

#ifndef BCGP_EXCLUDE_PROP_LIST

void CBCGPWinXPVisualManager::OnFillPropListToolbarArea(CDC* pDC, CBCGPPropList* pList, const CRect& rectToolBar)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pList);

	if (m_hThemeRebar == NULL || (pList != NULL && !pList->DrawControlBarColors()))
	{
		CBCGPVisualManagerXP::OnFillPropListToolbarArea(pDC, pList, rectToolBar);
		return;
	}

	CWnd* pWndParent = BCGPGetParentFrame (pList);
	if (pWndParent->GetSafeHwnd () == NULL)
	{
		pWndParent = pList->GetParent ();
	}

	ASSERT_VALID (pWndParent);

	CRect rectParent;
	pWndParent->GetWindowRect (rectParent);

	pList->ScreenToClient (&rectParent);

	CRect rect = rectToolBar;

	rect.right = max (rect.right, rectParent.right);
	rect.bottom = max (rect.bottom, rectParent.bottom);
	
	CFrameWnd* pMainFrame = BCGCBProGetTopLevelFrame (pWndParent);
	if (pMainFrame->GetSafeHwnd () != NULL)
	{
		CRect rectMain;
		pMainFrame->GetClientRect (rectMain);
		pMainFrame->MapWindowPoints (pList, &rectMain);

		rect.top = rectMain.top;
	}
	else
	{
		rect.top -= 2;
	}

	CRgn rgn;
	rgn.CreateRectRgnIndirect (rectToolBar);

	pDC->SelectClipRgn (&rgn);

	(*m_pfDrawThemeBackground) (m_hThemeRebar, pDC->GetSafeHdc(),
		0, 0, &rect, 0);

	pDC->SelectClipRgn (NULL);
}

COLORREF CBCGPWinXPVisualManager::OnFillPropertyListSelectedItem(CDC* pDC, CBCGPProp* pProp, CBCGPPropList* pWndList, const CRect& rectFill, BOOL bFocused)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID(pProp);

	if (m_hThemeMenu == NULL || !bFocused)
	{
		return CBCGPVisualManagerXP::OnFillPropertyListSelectedItem(pDC, pProp, pWndList, rectFill, bFocused);
	}
	
	(*m_pfDrawThemeBackground) (m_hThemeMenu, pDC->GetSafeHdc(), MENU_POPUPITEM, MPI_HOT, &rectFill, 0);

	COLORREF clrText = 0;
	(*m_pfGetThemeColor) (m_hThemeMenu, MENU_POPUPITEM, MPI_HOT, TMT_TEXTCOLOR, &clrText);

	return pProp->IsEnabled() ? clrText : globalData.clrGrayedText;
}

COLORREF CBCGPWinXPVisualManager::GetPropListGroupColor (CBCGPPropList* pPropList)
{
	ASSERT_VALID (pPropList);

	if (m_hThemeToolBar == NULL || globalData.m_nBitsPerPixel <= 8)
	{
		return CBCGPVisualManagerXP::GetPropListGroupColor (pPropList);
	}

	COLORREF clrFill = 0;
	(*m_pfGetThemeColor) (m_hThemeToolBar, TP_BUTTON, 0, TMT_FILLCOLOR, &clrFill);

	return clrFill;
}

COLORREF CBCGPWinXPVisualManager::GetPropListGroupTextColor (CBCGPPropList* pPropList)
{
	ASSERT_VALID (pPropList);

	if (m_hThemeToolBar == NULL || globalData.m_nBitsPerPixel <= 8)
	{
		return CBCGPVisualManagerXP::GetPropListGroupTextColor (pPropList);
	}

	COLORREF clrText = 0;
	(*m_pfGetThemeColor) (m_hThemeToolBar, TP_BUTTON, 0, TMT_TEXTCOLOR, &clrText);

	return clrText;
}

#endif

COLORREF CBCGPWinXPVisualManager::OnFillListBoxItem (CDC* pDC, CBCGPListBox* pListBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected)
{
	if (m_hThemeMenu == NULL || globalData.m_nBitsPerPixel <= 8)
	{
		return CBCGPVisualManagerXP::OnFillListBoxItem (pDC, pListBox, nItem, rect, bIsHighlihted, bIsSelected);
	}

	COLORREF clrText = globalData.clrBarText;

	if (bIsSelected || bIsHighlihted)
	{
		(*m_pfDrawThemeBackground) (m_hThemeMenu, pDC->GetSafeHdc(), MENU_POPUPITEM, MPI_HOT, &rect, 0);
		(*m_pfGetThemeColor) (m_hThemeMenu, MENU_POPUPITEM, MPI_HOT, TMT_TEXTCOLOR, &clrText);
	}

	return clrText;
}

COLORREF CBCGPWinXPVisualManager::OnFillComboBoxItem (CDC* pDC, CBCGPComboBox* pComboBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected)
{
	if (m_hThemeMenu == NULL || globalData.m_nBitsPerPixel <= 8)
	{
		return CBCGPVisualManagerXP::OnFillComboBoxItem (pDC, pComboBox, nItem, rect, bIsHighlihted, bIsSelected);
	}

	COLORREF clrText = globalData.clrBarText;

	if (bIsSelected || bIsHighlihted)
	{
		(*m_pfDrawThemeBackground) (m_hThemeMenu, pDC->GetSafeHdc(), MENU_POPUPITEM, MPI_HOT, &rect, 0);
		(*m_pfGetThemeColor) (m_hThemeMenu, MENU_POPUPITEM, MPI_HOT, TMT_TEXTCOLOR, &clrText);
	}
	else
	{
		pDC->FillRect (rect, &globalData.brWindow);
		clrText = globalData.clrWindowText;
	}

	return clrText;
}

