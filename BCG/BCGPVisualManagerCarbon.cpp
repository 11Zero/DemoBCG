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
// BCGPVisualManagerCarbon.cpp: implementation of the CBCGPVisualManagerCarbon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPGlobalUtils.h"
#include "BCGPVisualManagerCarbon.h"
#include "BCGPToolBar.h"
#include "BCGPDrawManager.h"
#include "BCGPPopupMenuBar.h"
#include "BCGPMenuBar.h"
#include "bcgglobals.h"
#include "BCGPToolbarMenuButton.h"
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
#include "BCGPTagManager.h"
#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPDockingControlBar.h"
#include "BCGPOutlookWnd.h"
#include "BCGPGridCtrl.h"
#include "BCGPToolbarComboBoxButton.h"
#include "BCGPToolbarEditBoxButton.h"
#include "BCGCBProVer.h"
#include "BCGPGroup.h"
#include "BCGPDialog.h"
#include "BCGPPropertySheet.h"
#include "BCGPOutlookButton.h"
#include "BCGPShowAllButton.h"

#ifndef BCGP_EXCLUDE_RIBBON
#include "BCGPRibbonBar.h"
#include "BCGPRibbonPanel.h"
#include "BCGPRibbonCategory.h"
#include "BCGPRibbonButton.h"
#include "BCGPRibbonQuickAccessToolbar.h"
#include "BCGPRibbonComboBox.h"
#include "BCGPRibbonMainPanel.h"
#include "BCGPRibbonPanelMenu.h"
#include "BCGPRibbonLabel.h"
#include "BCGPRibbonPaletteButton.h"
#include "BCGPRibbonStatusBar.h"
#include "BCGPRibbonStatusBarPane.h"
#include "BCGPRibbonProgressBar.h"
#include "BCGPRibbonHyperlink.h"
#include "BCGPRibbonSlider.h"
#endif

#ifndef BCGP_EXCLUDE_PLANNER
#include "BCGPPlannerViewDay.h"
#include "BCGPPlannerViewMonth.h"
#endif

#include "BCGPGanttChart.h"
#include "BCGPPropList.h"
#include "BCGPURLLinkButton.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define UPDATE_COLOR(clr, dblHue, dblSaturation) \
{ \
	double H, L, S; \
	CBCGPDrawManager::RGBtoHSL (clr, &H, &S, &L); \
	H = H * 360.0 + dblHue; \
	H = H - ((int)(H / 360)) * 360; \
	S = max (0.0, min (S + dblSaturation, 1.0)); \
	clr = CBCGPDrawManager::HLStoRGB_TWO (H, L, S); \
} \


IMPLEMENT_DYNCREATE(CBCGPVisualManagerCarbon, CBCGPVisualManager2007)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CString CBCGPVisualManagerCarbon::GetStyleResourceID ()
{
	CString strResID (_T("IDX_BCG_STYLE"));

#if !defined _AFXDLL || defined _BCGCBPRO_STATIC_

	strResID = _T("CARBON_") + strResID;

#endif

	return strResID;
};

BOOL CBCGPVisualManagerCarbon::SetStyle (LPCTSTR lpszPath)
{
#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_
	CString strVer;
	strVer.Format (_T("%d%d"), _BCGCBPRO_VERSION_MAJOR, _BCGCBPRO_VERSION_MINOR);

	CString strStyleDLLName = _T("BCGPStyleCarbon") + strVer + _T(".dll");

	CString strStyleDLLPath;

	if (lpszPath != NULL && _tcslen (lpszPath) > 0)
	{
		strStyleDLLPath = lpszPath;

		if (strStyleDLLPath [strStyleDLLPath.GetLength () - 1] != _T('\\'))
		{
			strStyleDLLPath += _T('\\');
		}

		strStyleDLLPath += strStyleDLLName;
	}
	else
	{
		strStyleDLLPath = strStyleDLLName;
	}

	HINSTANCE hinstRes = LoadLibrary (strStyleDLLPath);

	if (hinstRes <= (HINSTANCE) 32)
	{
		TRACE(_T("Cannot load Style DLL: %s\r\n"), strStyleDLLPath);
		ASSERT (FALSE);
		return FALSE;
	}

	SetResourceHandle (hinstRes);
	m_bAutoFreeRes = TRUE;

#else

	UNREFERENCED_PARAMETER (lpszPath);

	CString strStyle (GetStyleResourceID ());
	HINSTANCE hinstRes = AfxFindResourceHandle (strStyle, RT_BCG_STYLE_XML);

	if (::FindResource(hinstRes, strStyle, RT_BCG_STYLE_XML) == NULL)
	{
		TRACE(_T("Cannot load Style: %s\r\n"), strStyle);
		ASSERT (FALSE);
		return FALSE;
	}

	SetResourceHandle (hinstRes);

#endif

	return TRUE;
}

COLORREF CBCGPVisualManagerCarbon::GetColor(COLORREF clr, double h, double s)
{
	COLORREF clrOut = clr;
	UPDATE_COLOR(clrOut, h, s)

	return clrOut;
}

CBCGPVisualManagerCarbon::CBCGPVisualManagerCarbon()
{
	m_bUpdateColors	= FALSE;

	m_dblHue = 0.0;
	m_dblSaturation = 0.0;

	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseMainBorderCaption.GetImages (), &m_ctrlMainBorderCaption.GetImages ()));
	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseMainBorderL.GetImages (), &m_ctrlMainBorderL.GetImages ()));
	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseMainBorderR.GetImages (), &m_ctrlMainBorderR.GetImages ()));
	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseMainBorderTB.GetImages (), &m_ctrlMainBorderTB.GetImages ()));
	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseMainBorder.GetImages (), &m_ctrlMainBorder.GetImages ()));
	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseMDIChildBorder.GetImages (), &m_ctrlMDIChildBorder.GetImages ()));
	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseMiniBorderCaption.GetImages (), &m_ctrlMiniBorderCaption.GetImages ()));
	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseMiniBorderTB.GetImages (), &m_ctrlMiniBorderTB.GetImages ()));
	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseMiniBorder.GetImages (), &m_ctrlMiniBorder.GetImages ()));
	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseDialogBorder.GetImages (), &m_ctrlDialogBorder.GetImages ()));
	for (int i = 0; i < 2; i++)
	{
		m_arLinkImages.Add (XLinkImages (&m_ctrlBaseSysBtnBack[i].GetImages (), &m_SysBtnBack[i].GetImages ()));
		m_arLinkImages.Add (XLinkImages (&m_ctrlBaseSysBtnBackC[i].GetImages (), &m_SysBtnBackC[i].GetImages ()));
	}
	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseStatusBarBack.GetImages (), &m_ctrlStatusBarBack.GetImages ()));
	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseStatusBarBack_Ext.GetImages (), &m_ctrlStatusBarBack_Ext.GetImages ()));
	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseMenuBarBtn.GetImages (), &m_ctrlMenuBarBtn.GetImages ()));
	m_arLinkImages.Add (XLinkImages (&m_ctrlBasePopupBorder.GetImages (), &m_ctrlPopupBorder.GetImages ()));
	m_arLinkImages.Add (XLinkImages (&m_BaseToolBarGripper, &m_ToolBarGripper));
	m_arLinkImages.Add (XLinkImages (&m_BaseToolBarTear, &m_ToolBarTear));
	m_arLinkImages.Add (XLinkImages (&m_BaseStatusBarPaneBorder, &m_StatusBarPaneBorder));
	m_arLinkImages.Add (XLinkImages (&m_BaseStatusBarSizeBox, &m_StatusBarSizeBox));
	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseToolBarBtn.GetImages (), &m_ctrlToolBarBtn.GetImages ()));
	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseMenuItemBack.GetImages (), &m_ctrlMenuItemBack.GetImages ()));
	m_arLinkImages.Add (XLinkImages (&m_BaseMenuItemMarkerC, &m_MenuItemMarkerC));
	m_arLinkImages.Add (XLinkImages (&m_BaseMenuItemMarkerR, &m_MenuItemMarkerR));
	m_arLinkImages.Add (XLinkImages (&m_ctrlBaseRibbonBtnPush.GetImages (), &m_ctrlRibbonBtnPush.GetImages ()));

	m_arLinkColors.Add (XLinkColors (&m_clrBaseBarFace, &globalData.clrBarFace));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseActiveCaption, &globalData.clrActiveCaption));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseInactiveCaption, &globalData.clrInactiveCaption));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseBarShadow, &globalData.clrBarShadow));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseBarDkShadow, &globalData.clrBarDkShadow));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseBarLight, &globalData.clrBarLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseFloatToolBarBorder, &m_clrFloatToolBarBorder));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseHighlightGradientDark, &m_clrHighlightGradientDark));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseHighlightGradientLight, &m_clrHighlightGradientLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseHighlightDnGradientDark, &m_clrHighlightDnGradientDark));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseHighlightDnGradientLight, &m_clrHighlightDnGradientLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseHighlightCheckedGradientDark, &m_clrHighlightCheckedGradientDark));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseHighlightCheckedGradientLight, &m_clrHighlightCheckedGradientLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBasePressedButtonBorder, &m_clrPressedButtonBorder));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseDlgBackground, &m_clrDlgBackground));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseDlgButtonsArea, &m_clrDlgButtonsArea));
	m_arLinkColors.Add (XLinkColors (&m_BaseToolTipParams.m_clrFill, &m_ToolTipParams.m_clrFill));
	m_arLinkColors.Add (XLinkColors (&m_BaseToolTipParams.m_clrFillGradient, &m_ToolTipParams.m_clrFillGradient));
	m_arLinkColors.Add (XLinkColors (&m_BaseToolTipParams.m_clrBorder, &m_ToolTipParams.m_clrBorder));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseMainClientArea, &m_clrMainClientArea));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseMenuLight, &m_clrMenuLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseMenuBorder, &m_clrMenuBorder));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseMenuRarelyUsed, &m_clrMenuRarelyUsed));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseMenuItemBorder, &m_clrMenuItemBorder));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseMenuGutterLight, &m_clrMenuGutterLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseMenuGutterDark, &m_clrMenuGutterDark));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseSeparator1, &m_clrSeparator1));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseSeparator2, &m_clrSeparator2));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseSeparatorLight, &m_clrSeparatorLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseSeparatorDark, &m_clrSeparatorDark));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseBarBkgnd, &m_clrBarBkgnd));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseBarGradientLight, &m_clrBarGradientLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseBarGradientDark, &m_clrBarGradientDark));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseToolBarGradientLight, &m_clrToolBarGradientLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseToolBarGradientDark, &m_clrToolBarGradientDark));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseToolbarDisabled, &m_clrToolbarDisabled));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseToolBarGradientVertLight, &m_clrToolBarGradientVertLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseToolBarGradientVertDark, &m_clrToolBarGradientVertDark));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseCustomizeButtonGradientLight, &m_clrCustomizeButtonGradientLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseCustomizeButtonGradientDark, &m_clrCustomizeButtonGradientDark));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseToolBarBottomLine, &m_clrToolBarBottomLine));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseSeparatorLight, &m_clrSeparatorLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseSeparatorDark, &m_clrSeparatorDark));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseMenuBarGradientLight, &m_clrMenuBarGradientLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseMenuBarGradientDark, &m_clrMenuBarGradientDark));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseMenuBarGradientVertLight, &m_clrMenuBarGradientVertLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseMenuBarGradientVertDark, &m_clrMenuBarGradientVertDark));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseCaptionBarGradientLight, &m_clrCaptionBarGradientLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseCaptionBarGradientDark, &m_clrCaptionBarGradientDark));

	m_arLinkColors.Add (XLinkColors (&m_clrBaseTab3DFace, &m_clrTab3DFace));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseTab3DBlack, &m_clrTab3DBlack));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseTab3DDark, &m_clrTab3DDark));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseTab3DDarkShadow, &m_clrTab3DDarkShadow));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseTab3DLight, &m_clrTab3DLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseTab3DHighlight, &m_clrTab3DHighlight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseTabFlatFace, &m_clrTabFlatFace));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseTabFlatBlack, &m_clrTabFlatBlack));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseTabFlatDark, &m_clrTabFlatDark));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseTabFlatDarkShadow, &m_clrTabFlatDarkShadow));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseTabFlatLight, &m_clrTabFlatLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBaseTabFlatHighlight, &m_clrTabFlatHighlight));

	m_arLinkColors.Add (XLinkColors (&m_clrBasePopupGradientLight, &m_clrPopupGradientLight));
	m_arLinkColors.Add (XLinkColors (&m_clrBasePopupGradientDark, &m_clrPopupGradientDark));
}

CBCGPVisualManagerCarbon::~CBCGPVisualManagerCarbon()
{
}

void CBCGPVisualManagerCarbon::CleanUp ()
{
	if (m_bUpdateColors)
	{
		return;
	}

	CBCGPVisualManager2007::CleanUp ();

	int i = 0;

	m_ctrlBaseMainBorderCaption.CleanUp ();
	m_ctrlBaseMainBorderL.CleanUp ();
	m_ctrlBaseMainBorderR.CleanUp ();
	m_ctrlBaseMainBorderTB.CleanUp ();
	m_ctrlBaseMainBorder.CleanUp ();
	m_ctrlBaseMDIChildBorder.CleanUp ();
	m_ctrlBaseMiniBorderCaption.CleanUp ();
	m_ctrlBaseMiniBorderTB.CleanUp ();
	m_ctrlBaseMiniBorder.CleanUp ();
	m_ctrlBaseDialogBorder.CleanUp ();
	for (i = 0; i < 2; i++)
	{
		m_ctrlBaseSysBtnBack[i].CleanUp ();
		m_ctrlBaseSysBtnBackC[i].CleanUp ();
	}

	m_ctrlBaseStatusBarBack.CleanUp ();
	m_ctrlBaseStatusBarBack_Ext.CleanUp ();
	m_ctrlBaseMenuBarBtn.CleanUp ();
	m_ctrlBasePopupBorder.CleanUp ();
	m_BaseToolBarGripper.Clear ();
	m_BaseToolBarTear.Clear ();
	m_BaseStatusBarPaneBorder.Clear ();
	m_BaseStatusBarSizeBox.Clear ();
	m_ctrlBaseToolBarBtn.CleanUp ();
	m_ctrlBaseMenuItemBack.CleanUp ();
	m_BaseMenuItemMarkerC.Clear ();
	m_BaseMenuItemMarkerR.Clear ();

	m_clrBaseBarFace = (COLORREF)-1;
	m_clrBaseActiveCaption = (COLORREF)-1;
	m_clrBaseInactiveCaption = (COLORREF)-1;
	m_clrBaseFloatToolBarBorder = (COLORREF)-1;
	m_clrBaseDlgBackground = (COLORREF)-1;
	m_clrBaseDlgButtonsArea = (COLORREF)-1;
	m_clrBaseMainClientArea = (COLORREF)-1;
	m_clrBaseBarBkgnd = (COLORREF)-1;
	m_clrBaseBarGradientLight = (COLORREF)-1;
	m_clrBaseBarGradientDark = (COLORREF)-1;
	m_clrBaseToolBarGradientLight = (COLORREF)-1;
	m_clrBaseToolBarGradientDark = (COLORREF)-1;
	m_clrBaseToolbarDisabled = (COLORREF)-1;
	m_clrBaseToolBarGradientVertLight = (COLORREF)-1;
	m_clrBaseToolBarGradientVertDark = (COLORREF)-1;
	m_clrBaseCustomizeButtonGradientLight = (COLORREF)-1;
	m_clrBaseCustomizeButtonGradientDark = (COLORREF)-1;
	m_clrBaseToolBarBottomLine = (COLORREF)-1;
	m_clrBaseSeparatorLight = (COLORREF)-1;
	m_clrBaseSeparatorDark = (COLORREF)-1;
	m_clrBaseMenuBarGradientLight = (COLORREF)-1;
	m_clrBaseMenuBarGradientDark = (COLORREF)-1;
	m_clrBaseMenuBarGradientVertLight = (COLORREF)-1;
	m_clrBaseMenuBarGradientVertDark = (COLORREF)-1;

	m_clrBaseTab3DFace = CLR_DEFAULT;
	m_clrBaseTab3DBlack = CLR_DEFAULT;
	m_clrBaseTab3DDark = CLR_DEFAULT;
	m_clrBaseTab3DDarkShadow = CLR_DEFAULT;
	m_clrBaseTab3DLight = CLR_DEFAULT;
	m_clrBaseTab3DHighlight = CLR_DEFAULT;
	m_clrBaseTabFlatFace = CLR_DEFAULT;
	m_clrBaseTabFlatBlack = CLR_DEFAULT;
	m_clrBaseTabFlatDark = CLR_DEFAULT;
	m_clrBaseTabFlatDarkShadow = CLR_DEFAULT;
	m_clrBaseTabFlatLight = CLR_DEFAULT;
	m_clrBaseTabFlatHighlight = CLR_DEFAULT;

	m_clrLinkText = CLR_DEFAULT;
	m_clrLinkHotText = CLR_DEFAULT;
	m_clrActionText = CLR_DEFAULT;

	CBCGPToolTipParams dummy;
	m_BaseToolTipParams = dummy;

	for (i = 0; i < 2; i++)
	{
		m_SysBtnBackC[i].CleanUp ();
		m_SysBtnBackH[i].CleanUp ();
		m_SysBtnBackCH[i].CleanUp ();
	}

	m_ctrlMiniSysBtn.CleanUp ();

	m_clrBaseMenuGutterLight = CLR_DEFAULT;
	m_clrBaseMenuGutterDark = CLR_DEFAULT;

	m_bLoaded = FALSE;
}

void CBCGPVisualManagerCarbon::UpdateColors (double h, double s)
{
	if (m_dblHue != h || m_dblSaturation != s)
	{
		m_dblHue = h;
		m_dblSaturation = s;

		UpdateLinked ();
	}
}

void CBCGPVisualManagerCarbon::UpdateLinked ()
{
	if (m_arLinkImages.GetSize () == 0 && 
		m_arLinkColors.GetSize () == 0)
	{
		return;
	}

	int i = 0;
	for (i = 0; i < m_arLinkImages.GetSize (); i++)
	{
		HBITMAP bmpSrc = m_arLinkImages[i].bmpSrc->GetImageWell ();
		HBITMAP bmpDst = m_arLinkImages[i].bmpDst->GetImageWell ();

		if (bmpSrc == NULL)
		{
			continue;
		}

		if (bmpDst == NULL)
		{
			m_arLinkImages[i].bmpSrc->CopyTo (*m_arLinkImages[i].bmpDst);
			bmpDst = m_arLinkImages[i].bmpDst->GetImageWell ();
			if (bmpDst == NULL)
			{
				continue;
			}
		}

		DIBSECTION dibSrc;
		ZeroMemory (&dibSrc, sizeof (DIBSECTION));
		if (::GetObject (bmpSrc, sizeof (DIBSECTION), &dibSrc) != sizeof (DIBSECTION) ||
			dibSrc.dsBm.bmBits == NULL)
		{
			continue;
		}

		DIBSECTION dibDst;
		ZeroMemory (&dibDst, sizeof (DIBSECTION));
		if (::GetObject (bmpDst, sizeof (DIBSECTION), &dibDst) != sizeof (DIBSECTION) ||
			dibDst.dsBm.bmBits == NULL)
		{
			continue;
		}

		if (dibSrc.dsBm.bmWidth != dibDst.dsBm.bmWidth ||
			dibSrc.dsBm.bmHeight != dibDst.dsBm.bmHeight ||
			dibSrc.dsBm.bmBitsPixel != dibDst.dsBm.bmBitsPixel)
		{
			continue;
		}

		const int width  = dibSrc.dsBm.bmWidth;
		const int height = dibSrc.dsBm.bmHeight;
		const int pitch  = dibSrc.dsBm.bmWidthBytes;
		LPBYTE pBitsSrc  = (LPBYTE)dibSrc.dsBm.bmBits;
		LPBYTE pBitsDst  = (LPBYTE)dibDst.dsBm.bmBits;

		for (int y = 0; y < height; y++)
		{
			LPBYTE pRowSrc = pBitsSrc;
			LPBYTE pRowDst = pBitsDst;

			for (int x = 0; x < width; x++)
			{
				if (pRowSrc[2] != pRowSrc[1] || pRowSrc[1] != pRowSrc[0])
				{
					COLORREF clr = RGB (pRowSrc[2], pRowSrc[1], pRowSrc[0]);
					UPDATE_COLOR (clr, m_dblHue, m_dblSaturation);

					*pRowDst++ = GetBValue(clr);
					*pRowDst++ = GetGValue(clr);
					*pRowDst++ = GetRValue(clr);
					pRowDst++;
				}
				else
				{
					memcpy (pRowDst, pRowSrc, 4);
					pRowDst += 4;
				}

				pRowSrc += 4;
			}

			pBitsSrc += pitch;
			pBitsDst += pitch;
		}
	}

	for (i = 0; i < m_arLinkColors.GetSize (); i++)
	{
		*m_arLinkColors[i].clrDst = *m_arLinkColors[i].clrSrc;
		if (*m_arLinkColors[i].clrSrc != (COLORREF)-1)
		{
			UPDATE_COLOR (*m_arLinkColors[i].clrDst, m_dblHue, m_dblSaturation);
		}
	}

	if (globalData.clrBarFace != (COLORREF)-1)
	{
		globalData.brBarFace.DeleteObject ();
		globalData.brBarFace.CreateSolidBrush (globalData.clrBarFace);
	}
	if (globalData.clrActiveCaption != (COLORREF)-1)
	{
		globalData.brActiveCaption.DeleteObject ();
		globalData.brActiveCaption.CreateSolidBrush (globalData.clrActiveCaption);
	}
	if (globalData.clrInactiveCaption != (COLORREF)-1)
	{
		globalData.brInactiveCaption.DeleteObject ();
		globalData.brInactiveCaption.CreateSolidBrush (globalData.clrInactiveCaption);
	}
	if (m_clrFloatToolBarBorder != (COLORREF)-1)
	{
		m_brFloatToolBarBorder.DeleteObject ();
		m_brFloatToolBarBorder.CreateSolidBrush (m_clrFloatToolBarBorder);
	}
	if (m_clrDlgBackground != (COLORREF)-1)
	{
		m_brDlgBackground.DeleteObject ();
		m_brDlgBackground.CreateSolidBrush (m_clrDlgBackground);
	}
	if (m_clrDlgButtonsArea != (COLORREF)-1)
	{
		m_brDlgButtonsArea.DeleteObject ();
		m_brDlgButtonsArea.CreateSolidBrush (m_clrDlgButtonsArea);
	}

	if (m_clrMainClientArea != (COLORREF)-1)
	{
		m_brMainClientArea.DeleteObject ();
		m_brMainClientArea.CreateSolidBrush (m_clrMainClientArea);
	}

	if (m_clrMenuLight != (COLORREF)-1)
	{
		m_brMenuLight.DeleteObject ();
		m_brMenuLight.CreateSolidBrush (m_clrMenuLight);
	}
	if (m_clrMenuRarelyUsed != (COLORREF)-1)
	{
		m_brMenuRarelyUsed.DeleteObject ();
		m_brMenuRarelyUsed.CreateSolidBrush (m_clrMenuRarelyUsed);
	}
	if (m_clrSeparator1 != (COLORREF)-1)
	{
		m_penSeparator.DeleteObject ();
		m_penSeparator.CreatePen (PS_SOLID, 1, m_clrSeparator1);
	}
	if (m_clrSeparator2 != (COLORREF)-1)
	{
		m_penSeparator2.DeleteObject ();
		m_penSeparator2.CreatePen (PS_SOLID, 1, m_clrSeparator2);
	}
	if (m_clrMenuItemBorder != (COLORREF)-1)
	{
		m_penMenuItemBorder.DeleteObject ();
		m_penMenuItemBorder.CreatePen (PS_SOLID, 1, m_clrMenuItemBorder);
	}
	if (m_clrBarBkgnd != (COLORREF)-1)
	{
		m_brBarBkgnd.DeleteObject ();
		m_brBarBkgnd.CreateSolidBrush  (m_clrBarBkgnd);

		m_brMenuConnect.DeleteObject();
		m_brMenuConnect.CreateSolidBrush(m_clrBarBkgnd);
	}

	if (m_clrToolBarBottomLine != (COLORREF)-1)
	{
		m_penBottomLine.DeleteObject ();
		m_penBottomLine.CreatePen (PS_SOLID, 1, m_clrToolBarBottomLine);
	}
	if (m_clrSeparatorDark != (COLORREF)-1)
	{
		m_penSeparatorDark.DeleteObject ();
		m_penSeparatorDark.CreatePen (PS_SOLID, 1, m_clrSeparatorDark);
	}
	if (m_clrSeparatorLight != (COLORREF)-1)
	{
		m_penSeparatorLight.DeleteObject ();
		m_penSeparatorLight.CreatePen (PS_SOLID, 1, m_clrSeparatorLight);
	}

	m_clrEditBorder            = m_clrPressedButtonBorder;
	m_clrEditBorderDisabled    = globalData.clrBtnShadow;
	m_clrEditBorderHighlighted = m_clrEditBorder;
	m_clrEditSelection         = globalData.clrHilite;

	m_clrComboBorder               = m_clrEditBorder;
	m_clrComboBorderDisabled       = m_clrEditBorderDisabled;
	m_clrComboBorderHighlighted    = m_clrEditBorderHighlighted;
	m_clrComboBorderPressed        = m_clrComboBorderHighlighted;
	m_clrComboBtnBorder            = m_clrComboBorder;
	m_clrComboBtnBorderDisabled    = globalData.clrBtnHilite;
	m_clrComboBtnBorderHighlighted = m_clrComboBorderHighlighted;
	m_clrComboBtnBorderPressed     = m_clrComboBorderPressed;
	m_clrComboSelection            = m_clrEditSelection;
	m_clrComboBtnStart             = m_clrHighlightGradientDark;
	m_clrComboBtnFinish            = m_clrHighlightGradientLight;
	m_clrComboBtnDisabledStart     = globalData.clrBtnFace;
	m_clrComboBtnDisabledFinish    = m_clrComboBtnDisabledStart;
	m_clrComboBtnHighlightedStart  = m_clrHighlightCheckedGradientDark;
	m_clrComboBtnHighlightedFinish = m_clrHighlightCheckedGradientLight;
	m_clrComboBtnPressedStart      = m_clrHighlightDnGradientDark;
	m_clrComboBtnPressedFinish     = m_clrHighlightDnGradientLight;

	m_clrTaskPaneGroupBorder = m_clrToolBarBottomLine;
}

void CBCGPVisualManagerCarbon::OnUpdateSystemColors ()
{
	if (m_bUpdateColors || CBCGPVisualManager::GetInstance () != this)
	{
		return;
	}

	if (globalData.bIsWindows9x)
	{
		CBCGPVisualManager2007::OnUpdateSystemColors ();
		return;
	}

	if (!globalData.bIsOSAlphaBlendingSupport ||
		globalData.IsHighContastMode () ||
		globalData.m_nBitsPerPixel <= 8)
	{
		CBCGPVisualManager2007::OnUpdateSystemColors ();
		return;
	}

	m_bUpdateColors = TRUE;

	CBCGPVisualManager2007::Style style = CBCGPVisualManager2007::m_Style;
	HINSTANCE	hinstRes       = CBCGPVisualManager2007::m_hinstRes;
	CString		strStylePrefix = CBCGPVisualManager2007::m_strStylePrefix;
	BOOL		bAutoFreeRes   = CBCGPVisualManager2007::m_bAutoFreeRes;
	m_bAutoFreeRes = FALSE;

	if (!CBCGPVisualManager2007::SetStyle (CBCGPVisualManager2007::VS2007_ObsidianBlack))
	{
		return;
	}

	CBCGPVisualManager2007::OnUpdateSystemColors ();

	HINSTANCE hinstResOld = NULL;

	if (!SetStyle ())
	{
		return;
	}

	if (m_hinstRes != NULL)
	{
		hinstResOld = AfxGetResourceHandle ();
		AfxSetResourceHandle (m_hinstRes);
	}

	CBCGPTagManager tm;
	CBCGPTagManager::SetBaseColor ((COLORREF)-1, (COLORREF)-1);

	if (!tm.LoadFromResource (GetStyleResourceID (), RT_BCG_STYLE_XML))
	{
#if !defined _AFXDLL || defined _BCGCBPRO_STATIC_
		TRACE(_T("\r\nImportant: to enable Carbon look in static link, you need:\r\n"));
		TRACE(_T("1. Open \"Resource Includes\" dialog and add resource files:\r\n"));
		TRACE(_T("<BCGCBPro-Path>\\styles\\BCGPStyleCarbon.rc\r\n"));
		TRACE(_T("2. Add path to this folder to \"Additional Resource Include Directories\"\r\n"));
		TRACE(_T("<BCGCBPro-Path>\\styles\r\n\r\n"));
		ASSERT (FALSE);
#endif
		if (hinstResOld != NULL)
		{
			AfxSetResourceHandle (hinstResOld);
		}

		return;
	}

	{
		CString strStyle;
		tm.ExcludeTag (_T("STYLE"), strStyle);
		tm.SetBuffer (strStyle);
	}

	CString strItem;
	
	m_nType = 20;
	
	if (!tm.IsEmpty ())
	{
		int nVersion = 0;

		if (tm.ExcludeTag (_T("VERSION"), strItem))
		{
			CBCGPTagManager tmItem (strItem);

			tmItem.ReadInt (_T("NUMBER"), nVersion);

			if (nVersion == 2007)
			{
				tmItem.ReadInt (_T("TYPE"), m_nType);

				if (m_nType < 10)
				{
					m_nType *= 10;
				}

				m_bLoaded = TRUE;
			}

			if (m_bLoaded)
			{
				if (tmItem.ExcludeTag (_T("ID_PREFIX"), strItem))
				{
					strItem.TrimLeft ();
					strItem.TrimRight ();
					m_strStylePrefix = strItem;
				}
			}
		}
	}

	if (!m_bLoaded)
	{
		if (hinstResOld != NULL)
		{
			::AfxSetResourceHandle (hinstResOld);
		}

		CBCGPTagManager::SetBaseColor ((COLORREF)-1, (COLORREF)-1);
		return;
	}

	m_szNcBtnSize[0] = CSize (::GetSystemMetrics (SM_CXSIZE),
							  ::GetSystemMetrics (SM_CYSIZE));
	m_szNcBtnSize[1] = CSize (::GetSystemMetrics (SM_CXSMSIZE),
							  ::GetSystemMetrics (SM_CYSMSIZE));

	// globals
	if (tm.ExcludeTag (_T("GLOBALS"), strItem))
	{
		CBCGPTagManager tmItem (strItem);

		tmItem.ReadColor (_T("BarText"), globalData.clrBarText);

		tmItem.ReadColor (_T("BarFace"), m_clrBaseBarFace);
		tmItem.ReadColor (_T("ActiveCaption"), m_clrBaseActiveCaption);
		m_clrBaseInactiveCaption     = m_clrBaseActiveCaption;

		if (tmItem.ReadColor (_T("CaptionText"), globalData.clrCaptionText))
		{
			globalData.clrInactiveCaptionText = globalData.clrCaptionText;
		}

		tmItem.ReadColor (_T("InactiveCaption"), m_clrBaseInactiveCaption);
		tmItem.ReadColor (_T("InactiveCaptionText"), globalData.clrInactiveCaptionText);

		tmItem.ReadColor (_T("BarShadow"), m_clrBaseBarShadow);
		tmItem.ReadColor (_T("BarDkShadow"), m_clrBaseBarDkShadow);
		tmItem.ReadColor (_T("BarLight"), m_clrBaseBarLight);

		tmItem.ReadColor (_T("FloatToolBarBorder"), m_clrBaseFloatToolBarBorder);

		tmItem.ReadColor (_T("HighlightGradientDark"), m_clrBaseHighlightGradientDark);
		tmItem.ReadColor (_T("HighlightGradientLight"), m_clrBaseHighlightGradientLight);

		m_clrBaseHighlightDnGradientDark = m_clrBaseHighlightGradientLight;
		m_clrBaseHighlightDnGradientLight = m_clrBaseHighlightGradientDark;
		tmItem.ReadColor (_T("HighlightDnGradientDark"), m_clrBaseHighlightDnGradientDark);
		tmItem.ReadColor (_T("HighlightDnGradientLight"), m_clrBaseHighlightDnGradientLight);

		m_clrBaseHighlightCheckedGradientDark = m_clrBaseHighlightDnGradientLight;
		m_clrBaseHighlightCheckedGradientLight = m_clrBaseHighlightDnGradientDark;
		tmItem.ReadColor (_T("HighlightCheckedGradientDark"), m_clrBaseHighlightCheckedGradientDark);
		tmItem.ReadColor (_T("HighlightCheckedGradientLight"), m_clrBaseHighlightCheckedGradientLight);

		tmItem.ReadColor (_T("PressedButtonBorder"), m_clrBasePressedButtonBorder);

		COLORREF clrHB = globalData.clrHilite;
		COLORREF clrHT = globalData.clrTextHilite;
		if (tmItem.ReadColor (_T("Highlight"), clrHB) &&
			tmItem.ReadColor (_T("HighlightText"), clrHT))
		{
			globalData.clrHilite = clrHB;

			globalData.brHilite.DeleteObject ();
			globalData.brHilite.CreateSolidBrush (clrHB);

			globalData.clrTextHilite = clrHT;
		}

		tmItem.ReadColor (_T("LinkText"), m_clrLinkText);
		tmItem.ReadColor (_T("LinkHotText"), m_clrLinkHotText);
		tmItem.ReadColor (_T("ActionText"), m_clrActionText);

		tmItem.ReadColor (_T("MenuShadowColor"), m_clrMenuShadowBase);

		// dialog background
		m_clrBaseDlgBackground = m_clrBaseBarLight;
		tmItem.ReadColor (_T("DlgBackColor"), m_clrBaseDlgBackground);

		m_clrBaseDlgButtonsArea = m_clrBaseBarShadow;
		tmItem.ReadColor (_T("DlgButtonsAreaColor"), m_clrBaseDlgButtonsArea);

		// ToolTipParams
		m_bToolTipParams = tmItem.ReadToolTipParams (_T("TOOLTIP"), m_BaseToolTipParams);
		m_ToolTipParams = m_BaseToolTipParams;
	}

	// mainwnd
	if (tm.ExcludeTag (_T("MAINWND"), strItem))
	{
		CBCGPTagManager tmItem (strItem);

		// caption
		CString strCaption;
		if (tmItem.ExcludeTag (_T("CAPTION"), strCaption))
		{
			CBCGPTagManager tmCaption (strCaption);

			NONCLIENTMETRICS ncm;
			if (globalData.GetNonClientMetrics  (ncm))
			{
				m_AppCaptionFont.DeleteObject ();
				m_AppCaptionFont.CreateFontIndirect (&ncm.lfCaptionFont);
			}

			tmCaption.ReadColor (_T("ActiveText"), m_clrAppCaptionActiveText);
			tmCaption.ReadColor (_T("InactiveText"), m_clrAppCaptionInactiveText);
			tmCaption.ReadColor (_T("ActiveTitleText"), m_clrAppCaptionActiveTitleText);
			tmCaption.ReadColor (_T("InactiveTitleText"), m_clrAppCaptionInactiveTitleText);

			tmCaption.ReadBool (_T("TextCenter"), m_bNcTextCenter);

			tmCaption.ReadControlRenderer (_T("BORDER"), m_ctrlBaseMainBorderCaption, MakeResourceID(_T("MAINBRD_CAPTION")));
			m_ctrlBaseMainBorderCaption.CopyTo (m_ctrlMainBorderCaption);

			// buttons
			CString strButtons;
			if (tmCaption.ExcludeTag (_T("BUTTONS"), strButtons))
			{
				CBCGPTagManager tmButtons (strButtons);

				LPCTSTR szTags [4] = 
				{
					_T("NORMAL"),
					_T("NORMAL_SMALL"),
					_T("CLOSE"),
					_T("CLOSE_SMALL")
				};

				for (int i = 0; i < 2; i++)
				{
					CString str;
					CString suffix;
					if (i == 1)
					{
						suffix = _T("_S");
					}

					if (tmButtons.ExcludeTag (szTags[i], str))
					{
						CBCGPTagManager tmBtn (str);

						CSize sizeIcon (0, 0);
						if (tmBtn.ReadSize (_T("IconSize"), sizeIcon))
						{
							m_SysBtnRestore[i].Clear ();
							m_SysBtnRestore[i].SetPreMultiplyAutoCheck (TRUE);
							m_SysBtnRestore[i].SetImageSize (sizeIcon);
							m_SysBtnRestore[i].LoadStr (MakeResourceID(_T("SYS_BTN_RESTORE") + suffix));

							m_SysBtnMaximize[i].Clear ();
							m_SysBtnMaximize[i].SetPreMultiplyAutoCheck (TRUE);
							m_SysBtnMaximize[i].SetImageSize (sizeIcon);
							m_SysBtnMaximize[i].LoadStr (MakeResourceID(_T("SYS_BTN_MAXIMIZE") + suffix));

							m_SysBtnMinimize[i].Clear ();
							m_SysBtnMinimize[i].SetPreMultiplyAutoCheck (TRUE);
							m_SysBtnMinimize[i].SetImageSize (sizeIcon);
							m_SysBtnMinimize[i].LoadStr (MakeResourceID(_T("SYS_BTN_MINIMIZE") + suffix));

							m_SysBtnHelp[i].Clear ();
							m_SysBtnHelp[i].SetPreMultiplyAutoCheck (TRUE);
							m_SysBtnHelp[i].SetImageSize (sizeIcon);
							m_SysBtnHelp[i].LoadStr (MakeResourceID(_T("SYS_BTN_HELP") + suffix));
						}

						CBCGPTagManager::ParseControlRenderer (tmBtn.GetBuffer (), 
							m_ctrlBaseSysBtnBack[i], MakeResourceID(_T("SYS_BTN_BACK")));
						m_ctrlBaseSysBtnBack[i].CopyTo (m_SysBtnBack[i]);
						CBCGPTagManager::ParseControlRenderer (tmBtn.GetBuffer (), 
							m_SysBtnBackH[i], MakeResourceID(_T("SYS_BTN_BACK_H")));
					}

					if (tmButtons.ExcludeTag (szTags[i + 2], str))
					{
						CBCGPTagManager tmBtn (str);

						CSize sizeIcon (0, 0);
						if (tmBtn.ReadSize (_T("IconSize"), sizeIcon))
						{
							m_SysBtnClose[i].Clear ();
							m_SysBtnClose[i].SetPreMultiplyAutoCheck (TRUE);
							m_SysBtnClose[i].SetImageSize (sizeIcon);
							m_SysBtnClose[i].LoadStr (MakeResourceID(_T("SYS_BTN_CLOSE") + suffix));
						}

						CBCGPTagManager::ParseControlRenderer (tmBtn.GetBuffer (), 
							m_ctrlBaseSysBtnBackC[i], MakeResourceID(_T("SYS_BTN_BACK_C") + suffix));
						m_ctrlBaseSysBtnBackC[i].CopyTo (m_SysBtnBackC[i]);
						CBCGPTagManager::ParseControlRenderer (tmBtn.GetBuffer (), 
							m_SysBtnBackCH[i], MakeResourceID(_T("SYS_BTN_BACK_C_H") + suffix));
					}
				}
			}
		}

		// border
		tmItem.ReadControlRenderer (_T("BORDER"), m_ctrlBaseMainBorder, MakeResourceID(_T("MAINBRD")));
		m_ctrlBaseMainBorder.CopyTo (m_ctrlMainBorder);
		tmItem.ReadControlRenderer (_T("BORDER_L"), m_ctrlBaseMainBorderL, MakeResourceID(_T("MAINBRD_L")));
		m_ctrlBaseMainBorderL.CopyTo (m_ctrlMainBorderL);
		tmItem.ReadControlRenderer (_T("BORDER_R"), m_ctrlBaseMainBorderR, MakeResourceID(_T("MAINBRD_R")));
		m_ctrlBaseMainBorderR.CopyTo (m_ctrlMainBorderR);
		tmItem.ReadControlRenderer (_T("BORDER_TB"), m_ctrlBaseMainBorderTB, MakeResourceID(_T("MAINBRD_TB")));
		m_ctrlBaseMainBorderTB.CopyTo (m_ctrlMainBorderTB);
		tmItem.ReadControlRenderer (_T("BORDER_MDICHILD"), m_ctrlBaseMDIChildBorder, MakeResourceID(_T("MDICHILDBRD")));
		m_ctrlBaseMDIChildBorder.CopyTo (m_ctrlMDIChildBorder);

		if (tmItem.ReadColor (_T("MainClientArea"), m_clrBaseMainClientArea))
		{
			m_clrMainClientArea = m_clrBaseMainClientArea;
			m_brMainClientArea.DeleteObject ();
			m_brMainClientArea.CreateSolidBrush (m_clrMainClientArea);
		}
	}

	// miniwnd
	if (tm.ExcludeTag (_T("MINIWND"), strItem))
	{
		CBCGPTagManager tmItem (strItem);

		// caption
		CString strCaption;
		if (tmItem.ExcludeTag (_T("CAPTION"), strCaption))
		{
			CBCGPTagManager tmCaption (strCaption);

			tmCaption.ReadControlRenderer (_T("BORDER"), m_ctrlBaseMiniBorderCaption, MakeResourceID(_T("MINIBRD_CAPTION")));
			m_ctrlBaseMiniBorderCaption.CopyTo (m_ctrlMiniBorderCaption);
			tmCaption.ReadControlRenderer (_T("BUTTONS"), m_ctrlMiniSysBtn, MakeResourceID(_T("MINI_SYS_BTN")));
		}

		// border
		tmItem.ReadControlRenderer (_T("BORDER"), m_ctrlBaseMiniBorder, MakeResourceID(_T("MINIBRD")));
		m_ctrlBaseMiniBorder.CopyTo (m_ctrlMiniBorder);
		tmItem.ReadControlRenderer (_T("BORDER_TB"), m_ctrlBaseMiniBorderTB, MakeResourceID(_T("MINIBRD_TB")));
		m_ctrlBaseMiniBorderTB.CopyTo (m_ctrlMiniBorderTB);
	}

	// menu
	if (tm.ExcludeTag (_T("MENU"), strItem))
	{
		CBCGPTagManager tmItem (strItem);

		tmItem.ReadColor (_T("Light"), m_clrBaseMenuLight);
		tmItem.ReadColor (_T("GutterLight"), m_clrBaseMenuGutterLight);
		tmItem.ReadColor (_T("GutterDark"), m_clrBaseMenuGutterDark);

		m_clrMenuRarelyUsed = CLR_DEFAULT;
		tmItem.ReadColor (_T("Rarely"), m_clrBaseMenuRarelyUsed);
		tmItem.ReadColor (_T("Border"), m_clrBaseMenuBorder);
		tmItem.ReadColor (_T("Separator1"), m_clrBaseSeparator1);
		tmItem.ReadColor (_T("Separator2"), m_clrBaseSeparator2);

		COLORREF clrGroupBack = (COLORREF)-1;
		if (tmItem.ReadColor (_T("GroupBackground"), clrGroupBack))
		{
			m_brGroupBackground.DeleteObject ();
			m_brGroupBackground.CreateSolidBrush (clrGroupBack);
		}

		tmItem.ReadColor (_T("GroupText"), m_clrGroupText);

		tmItem.ReadColor (_T("ItemBorder"), m_clrBaseMenuItemBorder);

		tmItem.ReadControlRenderer (_T("ItemBack"), m_ctrlBaseMenuItemBack, MakeResourceID(_T("MENU_ITEM_BACK")));
		m_ctrlBaseMenuItemBack.CopyTo (m_ctrlMenuItemBack);
		tmItem.ReadToolBarImages (_T("ItemCheck"), m_BaseMenuItemMarkerC, MakeResourceID(_T("MENU_ITEM_MARKER_C")));
		m_BaseMenuItemMarkerC.CopyTo (m_MenuItemMarkerC);
		tmItem.ReadToolBarImages (_T("ItemRadio"), m_BaseMenuItemMarkerR, MakeResourceID(_T("MENU_ITEM_MARKER_R")));
		m_BaseMenuItemMarkerR.CopyTo (m_MenuItemMarkerR);

		tmItem.ReadColor (_T("TextNormal"), m_clrMenuText);
		tmItem.ReadColor (_T("TextHighlighted"), m_clrMenuTextHighlighted);
		tmItem.ReadColor (_T("TextDisabled"), m_clrMenuTextDisabled);

		COLORREF clrImages = m_clrMenuText;

		CString strColors;
		if (tmItem.ExcludeTag (_T("COLORS"), strColors))
		{
			CBCGPTagManager tmColors (strColors);

			tmColors.ReadColor (_T("Black"), clrImages);
			CBCGPMenuImages::SetColor (CBCGPMenuImages::ImageBlack, clrImages);

			tmColors.ReadColor (_T("Black2"), clrImages);
			CBCGPMenuImages::SetColor (CBCGPMenuImages::ImageBlack2, clrImages);

			struct XColors
			{
				CBCGPMenuImages::IMAGE_STATE state;
				LPCTSTR name;
			};
			XColors colors[4] =
				{
					{CBCGPMenuImages::ImageGray, _T("Gray")},
					{CBCGPMenuImages::ImageLtGray, _T("LtGray")},
					{CBCGPMenuImages::ImageWhite, _T("White")},
					{CBCGPMenuImages::ImageDkGray, _T("DkGray")}
				};

			for (int ic = 0; ic < 4; ic++)
			{
				if (tmColors.ReadColor (colors[ic].name, clrImages))
				{
					CBCGPMenuImages::SetColor (colors[ic].state, clrImages);
				}
			}
		}
		else
		{
			tmItem.ReadColor (_T("ImagesColor"), clrImages);
			CBCGPMenuImages::SetColor (CBCGPMenuImages::ImageBlack, clrImages);
			CBCGPMenuImages::SetColor (CBCGPMenuImages::ImageBlack2, clrImages);
		}

		// TODO:
		//CBCGPMenuImages::SetColor (CBCGPMenuImages::ImageBlack2, RGB (21, 66, 139));
	}

	// bars
	if (tm.ExcludeTag (_T("BARS"), strItem))
	{
		CBCGPTagManager tmItem (strItem);

		CString strBar;
		if (tmItem.ExcludeTag (_T("DEFAULT"), strBar))
		{
			CBCGPTagManager tmBar (strBar);

			tmBar.ReadColor (_T("Bkgnd"), m_clrBaseBarBkgnd);
			tmBar.ReadColor (_T("GradientLight"), m_clrBaseBarGradientLight);
			m_clrBaseBarGradientDark = m_clrBaseBarGradientLight;
			tmBar.ReadColor (_T("GradientDark"), m_clrBaseBarGradientDark);
		}

		if (tmItem.ExcludeTag (_T("TOOLBAR"), strBar))
		{
			CBCGPTagManager tmBar (strBar);

			m_clrBaseToolBarGradientLight = m_clrBaseBarGradientLight;
			m_clrBaseToolBarGradientDark  = m_clrBaseBarGradientDark;

			m_clrBaseToolbarDisabled = CBCGPDrawManager::SmartMixColors (
				m_clrBaseToolBarGradientDark, m_clrBaseToolBarGradientLight);

			tmBar.ReadColor (_T("GradientLight"), m_clrBaseToolBarGradientLight);
			tmBar.ReadColor (_T("GradientDark"), m_clrBaseToolBarGradientDark);

			m_clrBaseToolBarGradientVertLight = m_clrBaseToolBarGradientLight;
			m_clrBaseToolBarGradientVertDark  = m_clrBaseToolBarGradientDark;

			tmBar.ReadColor (_T("GradientVertLight"), m_clrBaseToolBarGradientVertLight);
			tmBar.ReadColor (_T("GradientVertDark"), m_clrBaseToolBarGradientVertDark);

			tmBar.ReadColor (_T("CustomizeButtonGradientLight"), m_clrBaseCustomizeButtonGradientLight);
			tmBar.ReadColor (_T("CustomizeButtonGradientDark"), m_clrBaseCustomizeButtonGradientDark);

			tmBar.ReadToolBarImages (_T("GRIPPER"), m_BaseToolBarGripper, MakeResourceID(_T("GRIPPER")));
			m_BaseToolBarGripper.CopyTo (m_ToolBarGripper);
			tmBar.ReadToolBarImages (_T("TEAR"), m_BaseToolBarTear, MakeResourceID(_T("TEAR")));
			m_BaseToolBarTear.CopyTo (m_ToolBarTear);
			tmBar.ReadControlRenderer (_T("BUTTON"), m_ctrlBaseToolBarBtn, MakeResourceID(_T("TB_BTN")));
			m_ctrlBaseToolBarBtn.CopyTo (m_ctrlToolBarBtn);

			m_clrToolBarBtnText            = globalData.clrBarText;
			m_clrToolBarBtnTextHighlighted = m_clrToolBarBtnText;
			tmBar.ReadColor (_T("TextNormal"), m_clrToolBarBtnText);
			tmBar.ReadColor (_T("TextHighlighted"), m_clrToolBarBtnTextHighlighted);
			tmBar.ReadColor (_T("TextDisabled"), m_clrToolBarBtnTextDisabled);

			tmBar.ReadColor (_T("BottomLineColor"), m_clrBaseToolBarBottomLine);
			if (!tmBar.ReadColor (_T("SeparatorDark"), m_clrBaseSeparatorDark))
			{
				m_clrBaseSeparatorDark = 
					CBCGPDrawManager::PixelAlpha (m_clrBaseToolBarBottomLine, RGB (255, 255, 255), 95);
			}
			if (!tmBar.ReadColor (_T("SeparatorLight"), m_clrBaseSeparatorLight))
			{
				m_clrBaseSeparatorLight	= RGB (255, 255, 255);
			}
		}

		if (tmItem.ExcludeTag (_T("MENUBAR"), strBar))
		{
			CBCGPTagManager tmBar (strBar);

			m_clrBaseMenuBarGradientLight = m_clrBaseToolBarGradientLight;
			m_clrBaseMenuBarGradientDark  = m_clrBaseToolBarGradientDark;

			tmBar.ReadColor (_T("GradientLight"), m_clrBaseMenuBarGradientLight);
			tmBar.ReadColor (_T("GradientDark"), m_clrBaseMenuBarGradientDark);

			m_clrBaseMenuBarGradientVertLight = m_clrBaseMenuBarGradientLight;
			m_clrBaseMenuBarGradientVertDark  = m_clrBaseMenuBarGradientDark;

			tmBar.ReadColor (_T("GradientVertLight"), m_clrBaseMenuBarGradientVertLight);
			tmBar.ReadColor (_T("GradientVertDark"), m_clrBaseMenuBarGradientVertDark);

			m_clrMenuBarBtnText            = m_clrToolBarBtnText;
			m_clrMenuBarBtnTextHighlighted = m_clrToolBarBtnTextHighlighted;
			m_clrMenuBarBtnTextDisabled    = m_clrToolBarBtnTextDisabled;
			tmBar.ReadColor (_T("TextNormal"), m_clrMenuBarBtnText);
			tmBar.ReadColor (_T("TextHighlighted"), m_clrMenuBarBtnTextHighlighted);
			tmBar.ReadColor (_T("TextDisabled"), m_clrMenuBarBtnTextDisabled);

			tmBar.ReadControlRenderer (_T("BUTTON"), m_ctrlBaseMenuBarBtn, MakeResourceID(_T("MB_BTN")));
			m_ctrlBaseMenuBarBtn.CopyTo (m_ctrlMenuBarBtn);
		}

		if (tmItem.ExcludeTag (_T("POPUPBAR"), strBar))
		{
			CBCGPTagManager tmBar (strBar);
			tmBar.ReadControlRenderer (_T("BORDER"), m_ctrlBasePopupBorder, MakeResourceID(_T("POPMENU_BRD")));
			m_ctrlBasePopupBorder.CopyTo (m_ctrlPopupBorder);
		}

		if (tmItem.ExcludeTag (_T("STATUSBAR"), strBar))
		{
			CBCGPTagManager tmBar (strBar);

			tmBar.ReadControlRenderer (_T("BACK"), m_ctrlBaseStatusBarBack, MakeResourceID(_T("SB_BACK")));
			m_ctrlBaseStatusBarBack.CopyTo (m_ctrlStatusBarBack);
			tmBar.ReadControlRenderer (_T("BACK_EXT"), m_ctrlBaseStatusBarBack_Ext, MakeResourceID(_T("SB_BACK_EXT")));
			m_ctrlBaseStatusBarBack_Ext.CopyTo (m_ctrlStatusBarBack_Ext);

			tmBar.ReadToolBarImages (_T("PANEBORDER"), m_BaseStatusBarPaneBorder, MakeResourceID(_T("SB_PANEBRD")));
			m_BaseStatusBarPaneBorder.CopyTo (m_StatusBarPaneBorder);
			tmBar.ReadToolBarImages (_T("SIZEBOX"), m_BaseStatusBarSizeBox, MakeResourceID(_T("SB_SIZEBOX")));
			m_BaseStatusBarSizeBox.CopyTo (m_StatusBarSizeBox);

			m_clrStatusBarText         = m_clrMenuBarBtnText;
			m_clrStatusBarTextDisabled = m_clrMenuBarBtnTextDisabled;
			m_clrExtenedStatusBarTextDisabled = m_clrMenuBarBtnTextDisabled;

			tmBar.ReadColor (_T("TextNormal"), m_clrStatusBarText);
			tmBar.ReadColor (_T("TextDisabled"), m_clrStatusBarTextDisabled);
			tmBar.ReadColor (_T("TextExtendedDisabled"), m_clrExtenedStatusBarTextDisabled);
		}

		m_clrBaseCaptionBarGradientLight = m_clrBaseToolBarGradientLight;
		m_clrBaseCaptionBarGradientDark = m_clrBaseToolBarGradientDark;

		if (tmItem.ExcludeTag (_T("CAPTIONBAR"), strBar))
		{
			CBCGPTagManager tmBar (strBar);

			tmBar.ReadColor (_T("GradientLight"), m_clrBaseCaptionBarGradientLight);
			tmBar.ReadColor (_T("GradientDark"), m_clrBaseCaptionBarGradientDark);
			tmBar.ReadColor (_T("TextNormal"), m_clrCaptionBarText);
			m_clrCaptionBarTextMessage = m_clrCaptionBarText;
			tmBar.ReadColor (_T("TextMessage"), m_clrCaptionBarTextMessage);
		}
	}

	if (tm.ExcludeTag (_T("TABS"), strItem))
	{
		CBCGPTagManager tmItem (strItem);

		tmItem.ReadColor (_T("TextColorActive"), m_clrTabTextActive);
		tmItem.ReadColor (_T("TextColorInactive"), m_clrTabTextInactive);

		CString strTab;
		if (tmItem.ExcludeTag (_T("3D"), strTab))
		{
			CBCGPTagManager tmTab (strTab);

			tmTab.ReadColor (_T("Face"), m_clrBaseTab3DFace);
			tmTab.ReadColor (_T("Black"), m_clrBaseTab3DBlack);
			tmTab.ReadColor (_T("Dark"), m_clrBaseTab3DDark);
			tmTab.ReadColor (_T("DarkShadow"), m_clrBaseTab3DDarkShadow);
			tmTab.ReadColor (_T("Light"), m_clrBaseTab3DLight);
			tmTab.ReadColor (_T("Highlight"), m_clrBaseTab3DHighlight);
			tmTab.ReadColor (_T("TextActive"), m_clrTab3DTextActive);
			tmTab.ReadColor (_T("TextInactive"), m_clrTab3DTextInactive);
		}

		if (tmItem.ExcludeTag (_T("FLAT"), strTab))
		{
			CBCGPTagManager tmTab (strTab);

			tmTab.ReadColor (_T("Face"), m_clrBaseTabFlatFace);
			tmTab.ReadColor (_T("Black"), m_clrBaseTabFlatBlack);
			tmTab.ReadColor (_T("Dark"), m_clrBaseTabFlatDark);
			tmTab.ReadColor (_T("DarkShadow"), m_clrBaseTabFlatDarkShadow);
			tmTab.ReadColor (_T("Light"), m_clrBaseTabFlatLight);
			tmTab.ReadColor (_T("Highlight"), m_clrBaseTabFlatHighlight);
			tmTab.ReadColor (_T("TextActive"), m_clrTabFlatTextActive);
			tmTab.ReadColor (_T("TextInactive"), m_clrTabFlatTextInactive);
		}
	}

	if (tm.ExcludeTag (_T("RIBBON"), strItem))
	{
		CBCGPTagManager tmItem (strItem);

		CString str;
		if (tmItem.ExcludeTag (_T("PANEL"), str))
		{
			CBCGPTagManager tmPanel (str);

			CString strButtons;
			if (tmPanel.ExcludeTag (_T("BUTTONS"), strButtons))
			{
				CBCGPTagManager tmButtons (strButtons);

				tmButtons.ReadControlRenderer (_T("BUTTON_PUSH"), m_ctrlBaseRibbonBtnPush, MakeResourceID(_T("RB_BTN_PUSH")));
				tmButtons.ReadControlRenderer (_T("BUTTON_GROUP"), m_ctrlRibbonBtnGroup, MakeResourceID(_T("RB_BTN_GRP")));
				m_ctrlBaseRibbonBtnPush.CopyTo (m_ctrlRibbonBtnPush);
			}
		}
	}

	m_clrOutlookCaptionTextNormal   = m_clrCaptionBarText;
	m_clrOutlookPageTextNormal      = m_clrOutlookCaptionTextNormal;
	m_clrOutlookPageTextHighlighted = m_clrOutlookPageTextNormal;
	m_clrOutlookPageTextPressed     = m_clrOutlookPageTextNormal;

	m_ctrlOutlookWndBar.CleanUp ();
	m_ctrlOutlookWndPageBtn.CleanUp ();

	if (tm.ExcludeTag (_T("OUTLOOK"), strItem))
	{
		CBCGPTagManager tmItem (strItem);

		CString str;
		if (tmItem.ExcludeTag (_T("CAPTION"), str))
		{
			CBCGPTagManager tmCaption (str);

			tmCaption.ReadColor (_T("TextNormal"), m_clrOutlookCaptionTextNormal);
		}
		
		if (tmItem.ExcludeTag (_T("PAGEBUTTON"), str))
		{
			CBCGPTagManager tmPage (str);

			tmPage.ReadColor (_T("TextNormal"), m_clrOutlookPageTextNormal);
			tmPage.ReadColor (_T("TextHighlighted"), m_clrOutlookPageTextHighlighted);
			tmPage.ReadColor (_T("TextPressed"), m_clrOutlookPageTextPressed);
		}
	}

	// Popup Window:
	m_clrBasePopupGradientLight = m_clrBaseBarGradientLight;
	m_clrBasePopupGradientDark = m_clrBaseBarGradientDark;

	if (tm.ExcludeTag (_T("POPUP"), strItem))
	{
		CBCGPTagManager tmItem (strItem);

		tmItem.ReadColor (_T("GradientFillLight"), m_clrBasePopupGradientLight);
		tmItem.ReadColor (_T("GradientFillDark"), m_clrBasePopupGradientDark);
	}

	if (m_clrBaseMenuGutterLight == CLR_DEFAULT)
	{
		m_clrBaseMenuGutterLight = m_clrBaseToolBarGradientLight;
	}
	if (m_clrBaseMenuGutterDark == CLR_DEFAULT)
	{
		m_clrBaseMenuGutterDark = m_clrBaseToolBarGradientLight;
	}

	if (hinstResOld != NULL)
	{
		AfxSetResourceHandle (hinstResOld);
	}

	m_brCtrlBackground.DeleteObject();
	m_brCtrlBackground.CreateSolidBrush(m_clrBaseToolBarGradientLight);

	CleanStyle ();

	CBCGPVisualManager2007::m_hinstRes       = hinstRes;
	CBCGPVisualManager2007::m_strStylePrefix = strStylePrefix;
	CBCGPVisualManager2007::m_bAutoFreeRes   = bAutoFreeRes;

	CBCGPVisualManager2007::SetStyle (style);

	m_brTabBack.DeleteObject ();
	m_brTabBack.CreateSolidBrush (globalData.clrBarFace);
	m_clrInactiveTabText = m_clrTab3DTextInactive;

	CBCGPTagManager::SetBaseColor ((COLORREF)-1, (COLORREF)-1);

	m_brRibbonMainPanelBkgnd.DeleteObject ();
	m_clrRibbonMainPanelBkgnd = m_clrMainClientArea;
	m_brRibbonMainPanelBkgnd.CreateSolidBrush (m_clrRibbonMainPanelBkgnd);

	UpdateLinked ();

	m_bUpdateColors = FALSE;
}

BOOL CBCGPVisualManagerCarbon::OnSetWindowRegion (CWnd* pWnd, CSize sizeWindow)
{
	ASSERT_VALID (pWnd);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManager2007::OnSetWindowRegion(pWnd, sizeWindow);
	}
	
	if (pWnd->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	if (!CanDrawImage ())
	{
		return FALSE;
	}

	if (globalData.DwmIsCompositionEnabled () && IsDWMCaptionSupported())
	{
		return FALSE;
	}

    CSize sz (0, 0);

	BOOL bMainWnd = FALSE;

#ifndef BCGP_EXCLUDE_RIBBON
	if (DYNAMIC_DOWNCAST (CBCGPRibbonBar, pWnd) != NULL)
	{
		return FALSE;
	}
	else
#endif
	{
		if (pWnd->IsZoomed ())
		{
			pWnd->SetWindowRgn (NULL, TRUE);
			return TRUE;
		}

		sz  = CSize (9, 9);

		bMainWnd = TRUE;
	}

	if (sz != CSize (0, 0))
	{
        CRgn rgn;
		BOOL bCreated = FALSE;

		bCreated = rgn.CreateRoundRectRgn (0, 0, sizeWindow.cx + 1, sizeWindow.cy + 1, sz.cx, sz.cy);

		if (bCreated)
		{
			if (pWnd->IsKindOf (RUNTIME_CLASS (CMDIChildWnd)) ||
				pWnd->IsKindOf (RUNTIME_CLASS (CBCGPDialog)) ||
				pWnd->IsKindOf (RUNTIME_CLASS (CBCGPPropertySheet)))
			{
				CRgn rgnWinodw;
				rgnWinodw.CreateRectRgn (0, sz.cy, sizeWindow.cx, sizeWindow.cy);

				rgn.CombineRgn (&rgn, &rgnWinodw, RGN_OR);
			}

			pWnd->SetWindowRgn ((HRGN)rgn.Detach (), TRUE);
			return TRUE;
		}
	}

	return FALSE;
}

void CBCGPVisualManagerCarbon::DrawNcBtn (CDC* pDC, const CRect& rect, UINT nButton, 
										BCGBUTTON_STATE state, BOOL /*bSmall*/, 
										BOOL bActive, BOOL /*bMDI = FALSE*/, BOOL bEnabled)
{
	ASSERT_VALID (pDC);

	CBCGPControlRenderer* pBack = NULL;
	CBCGPControlRenderer* pBackH = NULL;
	CBCGPToolBarImages* pImage = NULL;

	const double dScale = globalData.GetRibbonImageScale ();
	int nIndex = 0;

	if (nButton == SC_CLOSE)
	{
		pBack = &m_SysBtnBackC[nIndex];
		pBackH = &m_SysBtnBackCH[nIndex];
		pImage = &m_SysBtnClose[nIndex];
	}
	else
	{
		pBack = &m_SysBtnBack[nIndex];
		pBackH = &m_SysBtnBackH[nIndex];

		if (nButton == SC_MINIMIZE)
		{
			pImage = &m_SysBtnMinimize[nIndex];
		}
		else if (nButton == SC_MAXIMIZE)
		{
			pImage = &m_SysBtnMaximize[nIndex];
		}
		else if (nButton == SC_RESTORE)
		{
			pImage = &m_SysBtnRestore[nIndex];
		}
		else if (nButton == SC_CONTEXTHELP)
		{
			pImage = &m_SysBtnHelp[nIndex];
		}
	}

	BYTE alphaSrc = 255;

	pBack->Draw (pDC, rect, 0);
	if (state != ButtonsIsRegular)
	{
		pBackH->Draw (pDC, rect, state == ButtonsIsHighlighted ? 0 : 1);
	}
	else if (!bActive || !bEnabled)
	{
		alphaSrc = 189;
	}

	if (pImage != NULL)
	{
		CRect rectImage (rect);

		CBCGPToolBarImages::ImageAlignHorz horz = CBCGPToolBarImages::ImageAlignHorzCenter;
		CBCGPToolBarImages::ImageAlignVert vert = CBCGPToolBarImages::ImageAlignVertCenter;

		if (dScale != 1.)
		{
			horz = CBCGPToolBarImages::ImageAlignHorzStretch;
			vert = CBCGPToolBarImages::ImageAlignVertStretch;

			CSize sizeImage(pImage->GetImageSize ());
			sizeImage.cx = (int)(sizeImage.cx * dScale);
			sizeImage.cy = (int)(sizeImage.cy * dScale);

			if (sizeImage.cx > rect.Width () ||
				sizeImage.cy > rect.Height ())
			{
				double value = min(rect.Width() / (double)sizeImage.cx, rect.Height() / (double)sizeImage.cy);
				sizeImage.cx = (int)(sizeImage.cx * value);
				sizeImage.cy = (int)(sizeImage.cy * value);
			}

			rectImage.left   = (rect.left * 2 + rect.Width () - sizeImage.cx) / 2;
			rectImage.top    = (rect.top * 2 + rect.Height () - sizeImage.cy) / 2;
			rectImage.right  = rectImage.left + sizeImage.cx;
			rectImage.bottom = rectImage.top + sizeImage.cy;
		}

		pImage->DrawEx (pDC, rectImage, 0, horz, vert, CRect (0, 0, 0, 0), alphaSrc);
	}
}

void CBCGPVisualManagerCarbon::DrawNcCaption (CDC* pDC, CRect rectCaption, 
											   DWORD dwStyle, DWORD dwStyleEx,
											   const CString& strTitle, const CString& strDocument,
											   HICON hIcon, BOOL bPrefix, BOOL bActive, 
											   BOOL bTextCenter,
											   const CObList& lstSysButtons)
{
	const BOOL bIsRTL           = (dwStyleEx & WS_EX_LAYOUTRTL) == WS_EX_LAYOUTRTL;
	const BOOL bIsSmallCaption	= (dwStyleEx & WS_EX_TOOLWINDOW) != 0;
	const int nSysCaptionHeight = bIsSmallCaption ? ::GetSystemMetrics (SM_CYSMCAPTION) : ::GetSystemMetrics (SM_CYCAPTION);
	CSize szSysBorder (globalUtils.GetSystemBorders (dwStyle));

    CDC memDC;
    memDC.CreateCompatibleDC (pDC);
    CBitmap memBmp;
    memBmp.CreateCompatibleBitmap (pDC, rectCaption.Width (), rectCaption.Height ());
    CBitmap* pBmpOld = memDC.SelectObject (&memBmp);
	memDC.BitBlt (0, 0, rectCaption.Width (), rectCaption.Height (), pDC, 0, 0, SRCCOPY);

	BOOL bMaximized = (dwStyle & WS_MAXIMIZE) == WS_MAXIMIZE;

	BOOL bBorderOnly = (rectCaption.Height() - szSysBorder.cy) < nSysCaptionHeight;
	int indexBorder = (m_ctrlMainBorderCaption.GetImageCount() > 1 && bBorderOnly) ? 1 : 0;

	CRect rectBorderCaption (rectCaption);
	if (bMaximized)
	{
		rectBorderCaption.OffsetRect (-rectBorderCaption.TopLeft ());
		rectBorderCaption.bottom -= szSysBorder.cy;
	}
	m_ctrlMainBorderCaption.Draw (&memDC, rectBorderCaption, indexBorder);

	if ((dwStyle & WS_MINIMIZE) != WS_MINIMIZE)
	{
		CRect rectCaptionB (rectCaption);
		rectCaptionB.DeflateRect ((bMaximized ? 0 : szSysBorder.cx) - 2, 0);
		rectCaptionB.top = rectCaptionB.bottom - m_ctrlMainBorderTB.GetParams ().m_rectImage.Height ();
		m_ctrlMainBorderTB.Draw (&memDC, rectCaptionB, 0);
	}

	if (!bBorderOnly)
	{
		CRect rect (rectCaption);
		rect.DeflateRect (szSysBorder.cx, szSysBorder.cy, szSysBorder.cx, 0);
		rect.top = rect.bottom - nSysCaptionHeight - 1;

		// Draw icon:
		if (hIcon != NULL && !bIsSmallCaption)
		{
			CSize szIcon (::GetSystemMetrics (SM_CXSMICON), ::GetSystemMetrics (SM_CYSMICON));

			long x = rect.left + (bMaximized ? szSysBorder.cx : 0) + 2;
			long y = rect.top + max (0, (nSysCaptionHeight - szIcon.cy) / 2);

			::DrawIconEx (memDC.GetSafeHdc (), x, y, hIcon, szIcon.cx, szIcon.cy,
				0, NULL, DI_NORMAL);

			rect.left = x + szIcon.cx + (bMaximized ? szSysBorder.cx : 4);
		}
		else
		{
			int xOffset = (bMaximized ? szSysBorder.cx : 0) + 2;
			rect.left += xOffset;
		}

		// Draw system buttons:
		int xButtonsRight = rect.right;

		for (POSITION pos = lstSysButtons.GetHeadPosition (); pos != NULL;)
		{
			CBCGPFrameCaptionButton* pButton = (CBCGPFrameCaptionButton*)
				lstSysButtons.GetNext (pos);
			ASSERT_VALID (pButton);

			BCGBUTTON_STATE state = ButtonsIsRegular;

			if (pButton->m_bPushed && pButton->m_bFocused)
			{
				state = ButtonsIsPressed;
			}
			else if (pButton->m_bFocused)
			{
				state = ButtonsIsHighlighted;
			}

			UINT uiHit = pButton->GetHit ();
			UINT nButton = 0;

			switch (uiHit)
			{
			case HTCLOSE_BCG:
				nButton = SC_CLOSE;
				break;

			case HTMAXBUTTON_BCG:
				nButton = 
					(dwStyle & WS_MAXIMIZE) == WS_MAXIMIZE ? SC_RESTORE : SC_MAXIMIZE;
				break;

			case HTMINBUTTON_BCG:
				nButton = 
					(dwStyle & WS_MINIMIZE) == WS_MINIMIZE ? SC_RESTORE : SC_MINIMIZE;
				break;

			case HTHELPBUTTON_BCG:
				nButton = SC_CONTEXTHELP;
				break;
			}

			CRect rectBtn (pButton->GetRect ());
			if (bMaximized)
			{
				rectBtn.OffsetRect (szSysBorder.cx, szSysBorder.cy);
			}

			DrawNcBtn (&memDC, rectBtn, nButton, state, FALSE, bActive, FALSE, pButton->m_bEnabled);

			xButtonsRight = min (xButtonsRight, pButton->GetRect ().left);
		}

		// Draw text:
		if ((!strTitle.IsEmpty () || !strDocument.IsEmpty ()) && 
			rect.left < rect.right)
		{
			CFont* pOldFont = (CFont*)memDC.SelectObject (&GetNcCaptionTextFont());

			CRect rectText = rect;
			rectText.right = xButtonsRight - 1;

			DrawNcText (&memDC, rectText, strTitle, strDocument, bPrefix, bActive, bIsRTL, bTextCenter);

			memDC.SelectObject (pOldFont);
		}
	}

    pDC->BitBlt (rectCaption.left, rectCaption.top, rectCaption.Width (), rectCaption.Height (),
        &memDC, 0, 0, SRCCOPY);

    memDC.SelectObject (pBmpOld);
}

BOOL CBCGPVisualManagerCarbon::OnNcPaint (CWnd* pWnd, const CObList& lstSysButtons, CRect rectRedraw)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnNcPaint (pWnd, lstSysButtons, rectRedraw);
	}

	ASSERT_VALID (pWnd);

	if (pWnd->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	CWindowDC dc (pWnd);

	if (dc.GetSafeHdc () != NULL)
	{
		CRgn rgn;
		if (!rectRedraw.IsRectEmpty ())
		{
			rgn.CreateRectRgnIndirect (rectRedraw);
			dc.SelectClipRgn (&rgn);
		}

#ifndef BCGP_EXCLUDE_RIBBON
		CBCGPRibbonBar* pBar = GetRibbonBar (pWnd);
		BOOL bRibbonCaption  = pBar != NULL && 
							   pBar->IsWindowVisible () &&
							   pBar->IsReplaceFrameCaption ();
#else
		BOOL bRibbonCaption = FALSE;
#endif

		CRect rtWindow;
		pWnd->GetWindowRect (rtWindow);
		pWnd->ScreenToClient (rtWindow);

		CRect rtClient;
		pWnd->GetClientRect (rtClient);

		rtClient.OffsetRect (-rtWindow.TopLeft ());
		dc.ExcludeClipRect (rtClient);

		rtWindow.OffsetRect (-rtWindow.TopLeft ());

        BOOL bActive = IsWindowActive (pWnd);

		const DWORD dwStyle = pWnd->GetStyle ();
		CRect rectCaption (rtWindow);
		CSize szSysBorder (globalUtils.GetSystemBorders (dwStyle));

		BOOL bDialog = pWnd->IsKindOf (RUNTIME_CLASS (CBCGPDialog)) || pWnd->IsKindOf (RUNTIME_CLASS (CBCGPPropertySheet));

		if (!pWnd->IsIconic ())
		{
			rectCaption.bottom = rectCaption.top + szSysBorder.cy;
		}

		BOOL bMaximized = (dwStyle & WS_MAXIMIZE) == WS_MAXIMIZE;

		if (!bRibbonCaption)
		{
			const DWORD dwStyleEx = pWnd->GetExStyle ();
			const BOOL bIsSmallCaption = (dwStyleEx & WS_EX_TOOLWINDOW) != 0;
			const BOOL bIsIconic = pWnd->IsIconic();

			if (!bIsIconic)
			{
				const int nSysCaptionHeight = bIsSmallCaption ? ::GetSystemMetrics (SM_CYSMCAPTION) : ::GetSystemMetrics (SM_CYCAPTION);
				rectCaption.bottom += nSysCaptionHeight;
			}
			else
			{
				rectCaption.bottom -= 1;
			}

			BOOL bDestroyIcon = FALSE;
			HICON hIcon = globalUtils.GetWndIcon (pWnd, &bDestroyIcon, FALSE);

			CString strText;
			pWnd->GetWindowText (strText);

			CString strTitle (strText);
			CString strDocument;

			BOOL bPrefix = FALSE;
			if ((dwStyle & FWS_ADDTOTITLE) == FWS_ADDTOTITLE)
			{
				bPrefix = (dwStyle & FWS_PREFIXTITLE) == FWS_PREFIXTITLE;
				CFrameWnd* pFrameWnd = DYNAMIC_DOWNCAST(CFrameWnd, pWnd);

				if (pFrameWnd != NULL)
				{
					strTitle = pFrameWnd->GetTitle();

					if (!strTitle.IsEmpty ())
					{
						if (strText.GetLength () >= strTitle.GetLength ())
						{
							if (bPrefix)
							{
								int pos = strText.Find (strTitle, strText.GetLength () - strTitle.GetLength ());
								if (pos != -1)
								{
									strTitle = strText.Right (strTitle.GetLength () + 3);
									strDocument = strText.Left (strText.GetLength () - strTitle.GetLength ());
								}
							}
							else
							{
								int pos = strText.Find (strTitle);
								if (pos != -1)
								{
									strTitle = strText.Left (strTitle.GetLength () + 3);
									strDocument = strText.Right (strText.GetLength () - strTitle.GetLength ());
								}	
							}
						}
					}
					else
					{
						strDocument = strText;
					}
				}
			}

			if (!bIsIconic && rtClient.top <= szSysBorder.cy)
			{
				rectCaption.bottom = rectCaption.top + szSysBorder.cy;
			}

			if (bMaximized)
			{
				rectCaption.InflateRect (szSysBorder.cx, szSysBorder.cy, szSysBorder.cx, 0);
			}
			
			DrawNcCaption (&dc, rectCaption, dwStyle, dwStyleEx, 
							strTitle, strDocument, hIcon, bPrefix, bActive, m_bNcTextCenter,
							lstSysButtons);

			if (bDestroyIcon)
			{
				::DestroyIcon (hIcon);
			}


			if (bMaximized)
			{
				return TRUE;
			}
		}
#ifndef BCGP_EXCLUDE_RIBBON
		else
		{
			if (bMaximized)
			{
				return TRUE;
			}

			rectCaption.bottom += pBar->GetCaptionHeight ();

			m_ctrlMainBorderCaption.Draw (&dc, rectCaption, bActive ? 0 : 1);
		}
#endif // BCGP_EXCLUDE_RIBBON

		rtWindow.top = rectCaption.bottom;

		dc.ExcludeClipRect (rectCaption);

		int indexBorder = 0;

		//-------------------------------
		// Find status bar extended area:
		//-------------------------------
		CRect rectExt (0, 0, 0, 0);
		BOOL bExtended    = FALSE;
		BOOL bIsStatusBar = FALSE;

		CWnd* pStatusBar = pWnd->GetDescendantWindow (AFX_IDW_STATUS_BAR, TRUE);

		if (pStatusBar->GetSafeHwnd () != NULL && pStatusBar->IsWindowVisible ())
		{
			CBCGPStatusBar* pClassicStatusBar = DYNAMIC_DOWNCAST (
				CBCGPStatusBar, pStatusBar);
			if (pClassicStatusBar != NULL)
			{
				bExtended = pClassicStatusBar->GetExtendedArea (rectExt);
				bIsStatusBar = TRUE;
			}
		}

		CRect rectStatus (0, 0, 0, szSysBorder.cy);
		if (bIsStatusBar)
		{
			pStatusBar->GetClientRect (rectStatus);

			int nHeight = rectStatus.Height () + szSysBorder.cy;
			rectStatus.bottom = rtWindow.bottom;
			rectStatus.top    = rectStatus.bottom - nHeight;
			rectStatus.left   = rtWindow.left;
			rectStatus.right  = rtWindow.right;

			if (bExtended)
			{
				rectExt.left   = rectStatus.right - rectExt.Width () - szSysBorder.cx;
				rectExt.top    = rectStatus.top;
				rectExt.bottom = rectStatus.bottom;
				rectExt.right  = rtWindow.right;
			}

			m_ctrlStatusBarBack.Draw (&dc, rectStatus, indexBorder);

			if (bExtended)
			{
				rectExt.left -= m_ctrlStatusBarBack_Ext.GetParams ().m_rectCorners.left;

				m_ctrlStatusBarBack_Ext.Draw (&dc, rectExt, indexBorder);
			}
		}

		CRect rectPart (rtWindow);
		rectPart.bottom -= rectStatus.Height ();
		rectPart.right = rectPart.left + szSysBorder.cx;
		m_ctrlMainBorderL.Draw (&dc, rectPart, indexBorder);
		rectPart.right = rtWindow.right;
		rectPart.left  = rectPart.right - szSysBorder.cx;
		m_ctrlMainBorderR.Draw (&dc, rectPart, indexBorder);

		BOOL bBottomLine = TRUE;
		if (!bIsStatusBar)
		{
			CRect rectBottom (rtWindow);
			rectBottom.top = rectBottom.bottom - szSysBorder.cy;

			if (pWnd->IsKindOf (RUNTIME_CLASS (CMDIChildWnd)) ||
				(bDialog && !m_ctrlDialogBorder.IsValid ()))
			{
				if (bDialog)
				{
					CRect rtDialog (rtWindow);
					rtDialog.DeflateRect (1, 0, 1, 1);
					dc.FillRect (rtDialog, &GetDlgBackBrush (pWnd));

					dc.ExcludeClipRect (rtDialog);
					bBottomLine = FALSE;
				}

				m_ctrlMDIChildBorder.Draw (&dc, rectBottom, indexBorder);
			}
			else if (bDialog)
			{
				m_ctrlDialogBorder.Draw (&dc, rectBottom, indexBorder);
			}
			else
			{
				m_ctrlMainBorder.Draw (&dc, rectBottom, indexBorder);
			}
		}

		if (bBottomLine && !pWnd->IsIconic ())
		{
			CRect rectBottom (rtWindow);
			rectBottom.top = rectBottom.bottom - rectStatus.Height ();

			rectBottom.DeflateRect ((bMaximized ? 0 : szSysBorder.cx) - 2, 0);
			rectBottom.bottom = rectBottom.top + m_ctrlMainBorderTB.GetParams ().m_rectImage.Height ();
			m_ctrlMainBorderTB.Draw (&dc, rectBottom, indexBorder + m_ctrlMainBorderTB.GetImageCount () / 2);
		}

		dc.SelectClipRgn (NULL);

		return TRUE;
	}

	return CBCGPVisualManager2003::OnNcPaint (pWnd, lstSysButtons, rectRedraw);
}

BOOL CBCGPVisualManagerCarbon::OnUpdateNcButtons (CWnd* pWnd, const CObList& lstSysButtons, CRect rectCaption)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2007::OnUpdateNcButtons (pWnd, lstSysButtons, rectCaption);
	}

	ASSERT_VALID (pWnd);

	if (pWnd->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	CSize szSysBorder (globalUtils.GetSystemBorders (pWnd));

	int x = rectCaption.right + 2;

	if (!pWnd->IsIconic () && !pWnd->IsZoomed())
	{
		rectCaption.top -= szSysBorder.cy;
	}
	else
	{
		if (pWnd->IsZoomed())
		{
			rectCaption.top -= 2;
		}
		else
		{
			rectCaption.top = 0;
		}
	}

	int index = 0; // normal, small

	for (POSITION pos = lstSysButtons.GetHeadPosition (); pos != NULL;)
	{
		CSize sizeButton (0, 0);
		CBCGPFrameCaptionButton* pButton = (CBCGPFrameCaptionButton*)
			lstSysButtons.GetNext (pos);
		ASSERT_VALID (pButton);

		if (pButton->GetHit () == HTCLOSE_BCG)
		{
			sizeButton = m_SysBtnBackC[index].GetParams ().m_rectImage.Size ();
		}
		else
		{
			sizeButton = m_SysBtnBack[index].GetParams ().m_rectImage.Size ();
		}

		x -= sizeButton.cx;
		pButton->SetRect (CRect (CPoint (x, rectCaption.top), sizeButton));
	}

    return TRUE;
}

void CBCGPVisualManagerCarbon::OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
						CRect rectClient, CRect rectClip,
						BOOL bNCArea/* = FALSE*/)
{
	ASSERT_VALID (pBar);

	if (pBar->IsOnGlass ())
	{
		pDC->FillSolidRect (rectClient, RGB (0, 0, 0));
		return;
	}

    CRuntimeClass* pBarClass = pBar->GetRuntimeClass ();

	if (!CanDrawImage () ||
		pBar->IsDialogControl () ||
		pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPColorBar)) ||
		pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPCalculator)) ||
		pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPCalendarBar)))
	{
		CBCGPVisualManager2007::OnFillBarBackground (pDC, pBar, rectClient, rectClip, bNCArea);
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

			CBCGPDrawManager dm (*pDC);
            dm.FillGradient (rectImages, m_clrMenuGutterLight, m_clrMenuGutterDark, FALSE,
				35);

			if (!pMenuBar->HasGutterLogo())
			{
				rectImages.left = rectImages.right;
				rectImages.right += 2;
			}
			else
			{
				rectImages.left = rectImages.right - 2;
			}

            DrawSeparator (pDC, rectImages, FALSE);
		}

        return;
    }
    else if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPStatusBar)))
    {
		CSize szSysBorder (globalUtils.GetSystemBorders (pBar->GetParent()));
		if (szSysBorder.cx == 0 && szSysBorder.cy == 0)
		{
			szSysBorder = globalUtils.GetSystemBorders (BCGCBProGetTopLevelFrame(pBar));
		}		

		CRect rect (rectClient);
		CRect rectExt (0, 0, 0, 0);
		BOOL bExtended = ((CBCGPStatusBar*)pBar)->GetExtendedArea (rectExt);

		if (bExtended)
		{
			rect.right = rectExt.left;
		}

		rect.InflateRect (szSysBorder.cx, 0, szSysBorder.cx, szSysBorder.cy);
		m_ctrlStatusBarBack.Draw (pDC, rect, 0);

		if (bExtended)
		{
			rectExt.InflateRect (0, 0, szSysBorder.cx, szSysBorder.cy);
			rectExt.left -= m_ctrlStatusBarBack_Ext.GetParams ().m_rectCorners.left;
			m_ctrlStatusBarBack_Ext.Draw (pDC, rectExt, 0);
		}

		rect = rectClient;
		rect.InflateRect (2, 0);
		rect.bottom = rect.top + m_ctrlMainBorderTB.GetParams ().m_rectImage.Height ();
		m_ctrlMainBorderTB.Draw (pDC, rect, 1);

        return;
    }

	CBCGPVisualManager2007::OnFillBarBackground (pDC, pBar, rectClient, rectClip, bNCArea);
}

COLORREF CBCGPVisualManagerCarbon::OnFillMiniFrameCaption (CDC* pDC, 
								CRect rectCaption, 
								CBCGPMiniFrameWnd* pFrameWnd, BOOL bActive)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnFillMiniFrameCaption (pDC, 
								rectCaption, pFrameWnd, bActive);
	}

	ASSERT_VALID (pDC);

	CRect rectCaptionB (rectCaption);

	CRect rectBorderSize (0, 0, 0, 0);
	if (pFrameWnd != NULL)
	{
		pFrameWnd->CalcBorderSize (rectBorderSize);
	}
	else
	{
		rectBorderSize.left   = ::GetSystemMetrics (SM_CYSIZEFRAME);
		rectBorderSize.right  = rectBorderSize.left;
		rectBorderSize.top    = ::GetSystemMetrics (SM_CXSIZEFRAME);
		rectBorderSize.bottom = rectBorderSize.top;
	}

	rectCaption.InflateRect (rectBorderSize.left, rectBorderSize.top, rectBorderSize.right, 0);

	m_ctrlMiniBorderCaption.Draw (pDC, rectCaption);

	rectCaptionB.InflateRect (2, 0);
	rectCaptionB.top = rectCaptionB.bottom - m_ctrlMiniBorderTB.GetParams ().m_rectImage.Height ();
	m_ctrlMiniBorderTB.Draw (pDC, rectCaptionB, 0);

	//rectCaption

    // get the text color
	return bActive 
			? m_clrAppCaptionActiveTitleText
			: m_clrAppCaptionInactiveTitleText;
}

void CBCGPVisualManagerCarbon::OnDrawMiniFrameBorder (
										CDC* pDC, CBCGPMiniFrameWnd* pFrameWnd,
										CRect rectBorder, CRect rectBorderSize)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawMiniFrameBorder (pDC, 
								pFrameWnd, rectBorder, rectBorderSize);
		return;
	}

	ASSERT_VALID (pDC);

	rectBorder.top += rectBorderSize.top + ::GetSystemMetrics (SM_CYSMCAPTION);

	CRect rtPart (rectBorder);
	rtPart.right  = rtPart.left + rectBorderSize.left;
	rtPart.bottom -= rectBorderSize.bottom;
	m_ctrlMainBorderL.Draw (pDC, rtPart);

	rtPart = rectBorder;
	rtPart.left   = rtPart.right - rectBorderSize.right;
	rtPart.bottom -= rectBorderSize.bottom;
	m_ctrlMainBorderR.Draw (pDC, rtPart);

	rectBorder.top = rtPart.bottom;
	m_ctrlMiniBorder.Draw (pDC, rectBorder);

	rectBorder.DeflateRect (rectBorderSize.left, 0, rectBorderSize.right, 0);
	rectBorder.InflateRect (2, 0);
	rectBorder.bottom = rectBorder.top + m_ctrlMiniBorderTB.GetParams ().m_rectImage.Height ();
	m_ctrlMiniBorderTB.Draw (pDC, rectBorder, 1);
}

void CBCGPVisualManagerCarbon::OnDrawFloatingToolbarBorder (
												CDC* pDC, CBCGPBaseToolBar* pToolBar, 
												CRect rectBorder, CRect rectBorderSize)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawFloatingToolbarBorder (pDC, 
								pToolBar, rectBorder, rectBorderSize);
		return;
	}

	OnDrawMiniFrameBorder (pDC, NULL, rectBorder, rectBorderSize);
}

void CBCGPVisualManagerCarbon::OnDrawMenuBorder (CDC* pDC, CBCGPPopupMenu* pMenu, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawMenuBorder (pDC, pMenu, rect);
		return;
	}

#ifndef BCGP_EXCLUDE_RIBBON
	if (pMenu != NULL)
	{
		CBCGPRibbonPanelMenuBar* pRibbonMenuBar = 
			DYNAMIC_DOWNCAST (CBCGPRibbonPanelMenuBar, pMenu->GetMenuBar ());

		if (pRibbonMenuBar != NULL)
		{
			CBCGPVisualManager2007::OnDrawMenuBorder (pDC, pMenu, rect);
			return;
		}
	}
#endif

	m_ctrlPopupBorder.DrawFrame (pDC, rect);
}

void CBCGPVisualManagerCarbon::OnDrawCaptionButton (
						CDC* pDC, CBCGPCaptionButton* pButton, BOOL bActive,
						BOOL bHorz, BOOL bMaximized, BOOL bDisabled, 
						int nImageID/* = -1*/)
{
	if (!CanDrawImage () || bDisabled)
	{
		CBCGPVisualManager2007::OnDrawCaptionButton (pDC, pButton, 
			bActive, bHorz, bMaximized, bDisabled, nImageID);
		return;
	}

	CBCGPMenuImages::IMAGES_IDS id = (CBCGPMenuImages::IMAGES_IDS)-1;
	
	if (nImageID != -1)
	{
		id = (CBCGPMenuImages::IMAGES_IDS)nImageID;
	}
	else if (pButton != NULL)
	{
		id = pButton->GetIconID (bHorz, bMaximized);
	}

	ASSERT_VALID (pDC);
    CRect rc = pButton->GetRect ();
	rc.DeflateRect (0, 1);

	BOOL bPushed = pButton->m_bPushed;
	BOOL bFocused = pButton->m_bFocused || pButton->m_bDroppedDown;

	int nIndex = 0;
	if (bPushed && bFocused)
	{
		nIndex = 2;
	}
	else if (bFocused)
	{
		nIndex = 1;
	}

	if (id == CBCGPMenuImages::IdClose)
	{
		nIndex += 3;
	}

	m_ctrlMiniSysBtn.Draw (pDC, rc, nIndex);

	if (id != -1)
	{
		CBCGPMenuImages::IMAGE_STATE imageState = CBCGPMenuImages::ImageWhite;
		if (!bActive && !bPushed && !bFocused)
		{
			imageState = CBCGPMenuImages::ImageGray;
		}
		CBCGPMenuImages::Draw (pDC, id, rc, imageState);
	}
}

void CBCGPVisualManagerCarbon::GetTabFrameColors (const CBCGPBaseTabWnd* pTabWnd,
				   COLORREF& clrDark,
				   COLORREF& clrBlack,
				   COLORREF& clrHighlight,
				   COLORREF& clrFace,
				   COLORREF& clrDarkShadow,
				   COLORREF& clrLight,
				   CBrush*& pbrFace,
				   CBrush*& pbrBlack)
{
	CBCGPVisualManager2007::GetTabFrameColors (pTabWnd,
			   clrDark, clrBlack,
			   clrHighlight, clrFace,
			   clrDarkShadow, clrLight,
			   pbrFace, pbrBlack);

	pbrFace  = &m_brTabBack;
	pbrBlack = &m_brTabBack;
}

void CBCGPVisualManagerCarbon::OnEraseTabsArea (CDC* pDC, CRect rect, 
										 const CBCGPBaseTabWnd* pTabWnd)
{
	CBCGPVisualManager2003::OnEraseTabsArea (pDC, rect, pTabWnd);
}
//*****************************************************************************
void CBCGPVisualManagerCarbon::OnDrawTab (CDC* pDC, CRect rectTab,
						int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd)
{
	CBCGPVisualManager2003::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);
}
//*****************************************************************************
void CBCGPVisualManagerCarbon::OnFillTab (CDC* pDC, CRect rectFill, CBrush* pbrFill,
									 int iTab, BOOL bIsActive, 
									 const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pTabWnd);

	CBCGPVisualManager2003::OnFillTab (pDC, rectFill, pbrFill,
								 iTab, bIsActive, pTabWnd);
}
//*****************************************************************************
COLORREF CBCGPVisualManagerCarbon::GetTabTextColor (const CBCGPBaseTabWnd* pTabWnd, int iTab, BOOL bIsActive)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetTabTextColor (pTabWnd, iTab, bIsActive);
	}

	ASSERT_VALID (pTabWnd);

	if (pTabWnd->GetTabTextColor(iTab) != (COLORREF)-1)
	{
		return pTabWnd->GetTabTextColor(iTab);
	}

	if (pTabWnd->IsOneNoteStyle () && globalData.m_nBitsPerPixel > 8 &&
		!globalData.IsHighContastMode () && iTab == pTabWnd->GetHighlightedTab () &&
		pTabWnd->GetTabBkColor (iTab) == (COLORREF)-1)
	{
		return m_clrTabTextActive;
	}

	if (!bIsActive && (pTabWnd->IsOneNoteStyle () || pTabWnd->IsVS2005Style () || pTabWnd->IsLeftRightRounded () || pTabWnd->IsPointerStyle()) && 
		pTabWnd->GetTabBkColor (iTab) == (COLORREF)-1)
	{
		return pTabWnd->IsPointerStyle() ? m_clrSeparator : m_clrTabTextInactive;
	}

	if (pTabWnd->IsPointerStyle())
	{
		return (bIsActive || iTab == pTabWnd->GetHighlightedTab ()) ? m_clrTabTextInactive : m_clrSeparator;
	}
	
	return CBCGPVisualManager2003::GetTabTextColor (pTabWnd, iTab, bIsActive);
}
//*****************************************************************************
int CBCGPVisualManagerCarbon::GetTabHorzMargin (const CBCGPBaseTabWnd* pTabWnd)
{
	return CBCGPVisualManager2003::GetTabHorzMargin (pTabWnd);
}
//*****************************************************************************
BOOL CBCGPVisualManagerCarbon::OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd)
{	
	return CBCGPVisualManager2003::OnEraseTabsFrame (pDC, rect, pTabWnd);
}
//*****************************************************************************
void CBCGPVisualManagerCarbon::OnEraseTabsButton (CDC* pDC, CRect rect,
											  CBCGPButton* pButton,
											  CBCGPBaseTabWnd* pBaseTab)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);
	ASSERT_VALID (pBaseTab);

	if (pBaseTab->IsPointerStyle() && !pButton->IsHighlighted() && !pButton->IsPressed())
	{
		OnFillDialog (pDC, pBaseTab->GetParent (), rect);
		return;
	}

	CBCGPVisualManager2003::OnEraseTabsButton (pDC, rect, pButton, pBaseTab);
}
//*****************************************************************************
void CBCGPVisualManagerCarbon::OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
												 CBCGPButton* pButton, UINT uiState,
												 CBCGPBaseTabWnd* pWndTab)
{
	CBCGPVisualManager2003::OnDrawTabsButtonBorder(pDC, rect, 
											 pButton, uiState,
											 pWndTab);
}

void CBCGPVisualManagerCarbon::OnDrawSlider (CDC* pDC, CBCGPSlider* pSlider, CRect rect, BOOL bAutoHideMode)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawSlider (pDC, pSlider, rect, bAutoHideMode);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pSlider);

	pDC->FillRect (rect, &globalData.brBarFace);
}
//*******************************************************************************
void CBCGPVisualManagerCarbon::OnDrawSliderTic(CDC* pDC, CBCGPSliderCtrl* pSlider, CRect rectTic, BOOL bVert, BOOL bLeftTop, BOOL bDrawOnGlass)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawSliderTic(pDC, pSlider, rectTic, bVert, bLeftTop, bDrawOnGlass);
		return;
	}

	ASSERT_VALID (pDC);
	
	BOOL bIsDisabled = pSlider->GetSafeHwnd() != NULL && !pSlider->IsWindowEnabled();
	
	COLORREF clrLine = bIsDisabled ? globalData.clrBarShadow : globalData.clrBarHilite;
	
	CBCGPDrawManager dm (*pDC);
	dm.DrawRect (rectTic, (COLORREF)-1, clrLine);
}

void CBCGPVisualManagerCarbon::OnHighlightMenuItem (CDC *pDC, CBCGPToolbarMenuButton* pButton,
		CRect rect, COLORREF& clrText)
{
	CBCGPVisualManager2003::OnHighlightMenuItem (pDC, pButton, rect, clrText);
}

void CBCGPVisualManagerCarbon::OnDrawOutlookPageButtonBorder (
	CDC* pDC, CRect& rectBtn, BOOL bIsHighlighted, BOOL bIsPressed)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawOutlookPageButtonBorder (pDC, rectBtn, bIsHighlighted, bIsPressed);
		return;
	}

	if (bIsHighlighted || (bIsPressed && bIsHighlighted))
	{
		pDC->Draw3dRect (rectBtn, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
	else
	{
		pDC->Draw3dRect (rectBtn, m_clrToolBarGradientLight, m_clrToolBarBottomLine);
	}
}

#ifndef BCGP_EXCLUDE_PROP_LIST

COLORREF CBCGPVisualManagerCarbon::OnFillPropList(CDC* pDC, CBCGPPropList* pPropList, const CRect& rectClient, COLORREF& clrFill)
{
	ASSERT_VALID (pPropList);

	if (!CanDrawImage () || !pPropList->DrawControlBarColors ())
	{
		return CBCGPVisualManager2003::OnFillPropList(pDC, pPropList, rectClient, clrFill);
	}

	pDC->FillRect(rectClient, &GetDlgBackBrush(pPropList));
	clrFill = m_clrDlgBackground;

	return globalData.clrBarText;
}

COLORREF CBCGPVisualManagerCarbon::OnFillPropListDescriptionArea(CDC* pDC, CBCGPPropList* pList, const CRect& rect)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pList);

	if (!pList->DrawControlBarColors () || !CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnFillPropListDescriptionArea(pDC, pList, rect);
	}

	pDC->FillRect(rect, &GetDlgBackBrush(pList));
	return m_clrSeparatorLight;
}

COLORREF CBCGPVisualManagerCarbon::OnFillPropListCommandsArea(CDC* pDC, CBCGPPropList* pList, const CRect& rect)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pList);

	if (!pList->DrawControlBarColors () || !CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnFillPropListCommandsArea(pDC, pList, rect);
	}

	pDC->FillRect(rect, &GetDlgBackBrush(pList));
	return m_clrSeparatorLight;
}

COLORREF CBCGPVisualManagerCarbon::GetPropListGroupColor (CBCGPPropList* pPropList)
{
	ASSERT_VALID (pPropList);

	if (!CanDrawImage () || !pPropList->DrawControlBarColors ())
	{
		return CBCGPVisualManager2003::GetPropListGroupColor (pPropList);
	}

	return m_clrSeparatorLight;
}

COLORREF CBCGPVisualManagerCarbon::GetPropListGroupTextColor (CBCGPPropList* pPropList)
{
	ASSERT_VALID (pPropList);

	if (!CanDrawImage () || !pPropList->DrawControlBarColors ())
	{
		return CBCGPVisualManager2003::GetPropListGroupTextColor (pPropList);
	}

	return globalData.clrBarText;
}

COLORREF CBCGPVisualManagerCarbon::GetPropListDisabledTextColor(CBCGPPropList* pPropList)
{
	if (!CanDrawImage () || !pPropList->DrawControlBarColors ())
	{
		return CBCGPVisualManager2003::GetPropListDisabledTextColor (pPropList);
	}

	return m_clrMenuTextDisabled;
}

COLORREF CBCGPVisualManagerCarbon::GetPropListDesciptionTextColor (CBCGPPropList* pPropList)
{
	ASSERT_VALID (pPropList);

	if (!CanDrawImage () || !pPropList->DrawControlBarColors ())
	{
		return CBCGPVisualManager2003::GetPropListDesciptionTextColor (pPropList);
	}

	return globalData.clrBarText;
}

COLORREF CBCGPVisualManagerCarbon::GetPropListCommandTextColor (CBCGPPropList* pPropList)
{	
	ASSERT_VALID (pPropList);

	if (!CanDrawImage () || !pPropList->DrawControlBarColors ())
	{
		return CBCGPVisualManager2003::GetPropListCommandTextColor (pPropList);
	}

	return m_clrActionText == CLR_DEFAULT ? globalData.clrHotText : m_clrActionText;
}

#endif // BCGP_EXCLUDE_PROP_LIST

#ifndef BCGP_EXCLUDE_TOOLBOX

COLORREF CBCGPVisualManagerCarbon::GetToolBoxButtonTextColor (CBCGPToolBoxButton* pButton)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetToolBoxButtonTextColor (pButton);
	}

	return pButton->m_bIsHighlighted || pButton->m_bIsChecked ? m_clrToolBarBtnTextHighlighted : m_clrToolBarBtnText;
}

#endif // BCGP_EXCLUDE_TOOLBOX

COLORREF CBCGPVisualManagerCarbon::GetURLLinkColor (CBCGPURLLinkButton* pButton, BOOL bHover)
{
	ASSERT_VALID (pButton);

	if (!CanDrawImage () || !pButton->m_bVisualManagerStyle || pButton->m_bOnGlass)
	{
		return CBCGPVisualManager2003::GetURLLinkColor (pButton, bHover);
	}

	if (pButton->GetSafeHwnd() != NULL && !pButton->IsWindowEnabled())
	{
		return m_clrToolBarBtnTextDisabled;
	}
	
	return bHover
			? (m_clrLinkHotText == (COLORREF)-1 ? globalData.clrHotLinkText : m_clrLinkHotText)
			: (m_clrLinkText == (COLORREF)-1 ? globalData.clrHotLinkText : m_clrLinkText);
}

//**************************************************************************************
COLORREF CBCGPVisualManagerCarbon::GetToolbarButtonTextColor (CBCGPToolbarButton* pButton,
												  CBCGPVisualManager::BCGBUTTON_STATE state)
{
	if (CanDrawImage ())
	{
		BOOL bDisabled = (CBCGPToolBar::IsCustomizeMode () && !pButton->IsEditable ()) ||
			(!CBCGPToolBar::IsCustomizeMode () && (pButton->m_nStyle & TBBS_DISABLED));

		if (pButton->IsKindOf (RUNTIME_CLASS (CBCGPOutlookButton)) != NULL ||
			pButton->IsKindOf (RUNTIME_CLASS (CBCGPToolbarComboBoxButton)) != NULL ||
			pButton->IsKindOf (RUNTIME_CLASS (CBCGPToolbarEditBoxButton)) != NULL)
		{
			return bDisabled ? m_clrToolBarBtnTextDisabled : m_clrToolBarBtnText;
		}
	}

	return CBCGPVisualManager2007::GetToolbarButtonTextColor (pButton, state);
}
//**************************************************************************************
void CBCGPVisualManagerCarbon::OnDrawEditCtrlResizeBox (CDC* pDC, CBCGPEditCtrl* /*pEdit*/, CRect rect)
{
	pDC->FillRect (rect, &globalData.brBtnFace);
}

#ifndef BCGP_EXCLUDE_GRID_CTRL

COLORREF CBCGPVisualManagerCarbon::OnFillGridItem (CBCGPGridCtrl* pCtrl, 
											CDC* pDC, CRect rectFill,
											BOOL bSelected, BOOL bActiveItem, BOOL bSortedColumn)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnFillGridItem (pCtrl, 
											pDC, rectFill, bSelected, bActiveItem, bSortedColumn);
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pCtrl);

	// Fill area:
	if (bSelected && !bActiveItem)
	{
		if (!pCtrl->IsFocused ())
		{
			pDC->FillRect (rectFill, &globalData.brBtnFace);
			return globalData.clrBtnText;
		}
		else
		{
			pDC->FillRect (rectFill, &globalData.brHilite);
			return globalData.clrTextHilite;
		}
	}
	else
	{
		if (bActiveItem)
		{
			pDC->FillRect (rectFill, &globalData.brWindow);
		}
		else if (bSortedColumn)
		{
			CBrush br (pCtrl->GetSortedColor ());
			pDC->FillRect (rectFill, &br);
		}
		else
		{
			// no painting
		}
	}

	return (COLORREF)-1;
}

COLORREF CBCGPVisualManagerCarbon::OnFillGridGroupByBoxBackground (CDC* pDC, CRect rect)
{
	COLORREF clr = CBCGPVisualManager2007::OnFillGridGroupByBoxBackground (pDC, rect);

	if (CanDrawImage ())
	{
		clr = globalData.clrWindowText;
	}

	return clr;
}

COLORREF CBCGPVisualManagerCarbon::OnFillReportCtrlRowBackground (CBCGPGridCtrl* pCtrl, 
												  CDC* pDC, CRect rectFill,
												  BOOL bSelected, BOOL bGroup)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnFillReportCtrlRowBackground (pCtrl, 
											pDC, rectFill, bSelected, bGroup);
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pCtrl);

	// Fill area:
	COLORREF clrText = (COLORREF)-1;

	if (bSelected)
	{
		if (!pCtrl->IsFocused ())
		{
			pDC->FillRect (rectFill, &globalData.brBtnFace);
			clrText = m_clrReportGroupText;
		}
		else
		{
			pDC->FillRect (rectFill, &globalData.brHilite);
			clrText = globalData.clrTextHilite;
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

BOOL CBCGPVisualManagerCarbon::OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme)
{
	BOOL bRes = CBCGPVisualManager2007::OnSetGridColorTheme (pCtrl, theme);

	if (!CanDrawImage ())
	{
		return bRes;
	}

	theme.m_clrBackground = globalData.clrBarFace;
	theme.m_clrText = globalData.clrBarText;
	theme.m_clrHorzLine = theme.m_clrVertLine = globalData.clrBarShadow;

	theme.m_EvenColors.m_clrBackground = m_clrToolBarGradientDark;
	theme.m_EvenColors.m_clrGradient = (COLORREF)-1;
	theme.m_EvenColors.m_clrText = m_clrToolBarBtnText;

	theme.m_OddColors.m_clrBackground = globalData.clrBarFace;
	theme.m_OddColors.m_clrGradient = (COLORREF)-1;
	theme.m_OddColors.m_clrText = globalData.clrBarText;

	theme.m_SelColors.m_clrBackground = m_clrComboBtnFinish;
	theme.m_SelColors.m_clrGradient = m_clrComboBtnStart;
	theme.m_SelColors.m_clrBorder = m_clrComboBtnPressedStart;
	theme.m_SelColors.m_clrText = m_clrComboBtnBorder;

	theme.m_GroupSelColors.m_clrBackground = m_clrComboBtnFinish;
	theme.m_GroupSelColors.m_clrGradient = m_clrComboBtnStart;
	theme.m_GroupSelColors.m_clrBorder = m_clrComboBtnPressedStart;
	theme.m_GroupSelColors.m_clrText = m_clrComboBtnBorder;

	theme.m_HeaderColors.m_clrBackground = m_clrToolBarGradientDark;
	theme.m_HeaderColors.m_clrBorder = m_clrToolBarGradientLight;
	theme.m_HeaderColors.m_clrGradient = (COLORREF)-1;
	theme.m_HeaderColors.m_clrText = globalData.clrBarText;

	theme.m_HeaderSelColors.m_clrBackground = m_clrComboBtnFinish;
	theme.m_HeaderSelColors.m_clrGradient =	m_clrComboBtnStart;
	theme.m_HeaderSelColors.m_clrBorder = m_clrComboBtnBorder;
	theme.m_HeaderSelColors.m_clrText = m_clrComboBtnBorder;

	theme.m_GroupColors.m_clrBackground = m_clrToolBarGradientDark;
	theme.m_GroupColors.m_clrGradient = m_clrToolBarGradientLight;
	theme.m_GroupColors.m_clrText = m_clrToolBarBtnText;

	theme.m_LeftOffsetColors.m_clrBackground = m_clrToolBarGradientDark;
	theme.m_LeftOffsetColors.m_clrBorder = theme.m_clrVertLine;

	return TRUE;
}

void CBCGPVisualManagerCarbon::OnDrawGridDataBar(CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawGridDataBar(pCtrl, pDC, rect);
		return;
	}

	ASSERT_VALID(pDC);

	COLORREF clrLight = RGB (60, 160, 95);
	COLORREF clrDark = RGB (44, 116, 69);

	CBCGPDrawManager dm(*pDC);

	dm.FillGradient(rect, clrDark, clrLight, FALSE);
	pDC->Draw3dRect(rect, clrDark, clrDark);
}

void CBCGPVisualManagerCarbon::GetGridColorScaleBaseColors(CBCGPGridCtrl* pCtrl, COLORREF& clrLow, COLORREF& clrMid, COLORREF& clrHigh)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::GetGridColorScaleBaseColors(pCtrl, clrLow, clrMid, clrHigh);
		return;
	}

	clrLow = RGB (50, 0, 0);
	clrMid = RGB (174, 112, 0);
	clrHigh = RGB (0, 100, 0);
}

#endif // BCGP_EXCLUDE_GRID_CTRL

void CBCGPVisualManagerCarbon::OnFillSpinButton (CDC* pDC, CBCGPSpinButtonCtrl* pSpinCtrl, CRect rect, BOOL bDisabled)
{
	if (CBCGPToolBarImages::m_bIsDrawOnGlass || !CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillSpinButton (pDC, pSpinCtrl, rect, bDisabled);
	}
	else
	{
		CBrush br (bDisabled ? globalData.clrBtnFace : m_clrComboBtnStart);

		pDC->FillRect (rect, &br);
		pDC->Draw3dRect (rect, globalData.clrBarHilite, globalData.clrBarHilite);
	}
}

COLORREF CBCGPVisualManagerCarbon::OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnFillCommandsListBackground (pDC, rect, bIsSelected);
	}

	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	rect.left = 0;

	if (bIsSelected)
	{
		COLORREF clr1 = m_clrHighlightGradientDark;
		COLORREF clr2 = m_clrHighlightGradientLight;
		COLORREF clrBorder = m_clrPressedButtonBorder;

		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rect, clr1, clr2, TRUE);

		pDC->Draw3dRect (rect, clrBorder, clrBorder);

		CBCGPToolbarMenuButton dummy;
		return GetHighlightedMenuItemTextColor (&dummy);
	}
	else
	{
		pDC->FillRect (rect, &globalData.brWindow);

		int iImageWidth = CBCGPToolBar::GetMenuImageSize ().cx + GetMenuImageMargin ();

		CRect rectImages = rect;
		rectImages.right = rectImages.left + iImageWidth + MENU_IMAGE_MARGIN;

		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rectImages, m_clrToolBarGradientLight, m_clrToolBarGradientDark, FALSE);

		return globalData.clrWindowText;
	}
}
//*****************************************************************************
int CBCGPVisualManagerCarbon::GetMenuDownArrowState (CBCGPToolbarMenuButton* pButton, BOOL bHightlight, BOOL bPressed, BOOL bDisabled)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetMenuDownArrowState (pButton, bHightlight, bPressed, bDisabled);
	}

	CBCGPWnd* pWnd = DYNAMIC_DOWNCAST(CBCGPWnd, pButton->GetParentWnd());

	if (pWnd != NULL && pWnd->IsOnGlass())
	{
		return CBCGPVisualManager2003::GetMenuDownArrowState (pButton, bHightlight, bPressed, bDisabled);
	}

	if (bDisabled)
	{
		return (int)CBCGPMenuImages::ImageGray;
	}

	return (int) ((bHightlight || bPressed) ? CBCGPMenuImages::ImageBlack : CBCGPMenuImages::ImageWhite);
}

int CBCGPVisualManagerCarbon::GetTabButtonState (CBCGPTabWnd* pTab, CBCGTabButton* pButton)
{
	if (!CanDrawImage () || pButton->IsHighlighted () || pButton->IsPressed ())
	{
		return CBCGPVisualManager2003::GetTabButtonState (pTab, pButton);
	}

	return (int) CBCGPMenuImages::ImageWhite;
}

void CBCGPVisualManagerCarbon::OnDrawShowAllMenuItems (CDC* pDC, CRect rect, 
												 CBCGPVisualManager::BCGBUTTON_STATE state)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawShowAllMenuItems (pDC, rect, state);
		return;
	}

	ASSERT_VALID (pDC);

	if (globalData.m_nBitsPerPixel > 8 && !globalData.IsHighContastMode ())
	{
		rect.left = rect.CenterPoint().x - rect.Height() / 2;
		rect.right = rect.left + rect.Height();
		rect.DeflateRect(1, 1);
		
		CBCGPDrawManager dm (*pDC);
		dm.DrawEllipse(rect, m_clrHighlightGradientLight, m_clrHighlightDnGradientLight);
	}

	CBCGPVisualManager::OnDrawShowAllMenuItems (pDC, rect, state);
}

void CBCGPVisualManagerCarbon::OnFillButtonInterior (CDC* pDC,
	CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state)
{
	if (CanDrawImage ())
	{
		CBCGPControlRenderer* pRenderer = NULL;
		int index = 0;

		BOOL bDisabled = (pButton->m_nStyle & TBBS_DISABLED) == TBBS_DISABLED;
		BOOL bPressed  = (pButton->m_nStyle & TBBS_PRESSED ) == TBBS_PRESSED;
		BOOL bChecked  = (pButton->m_nStyle & TBBS_CHECKED ) == TBBS_CHECKED;

		CBCGPBaseControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar, pButton->GetParentWnd ());

		CBCGPToolbarMenuButton* pMenuButton = 
			DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);
		if (pMenuButton != NULL && pBar != NULL)
		{
			if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar)) &&
				DYNAMIC_DOWNCAST(CBCGPShowAllButton, pButton) != NULL)
			{
				if (state == ButtonsIsPressed || state == ButtonsIsHighlighted)
				{
					COLORREF clrText;
					CBCGPVisualManager2003::OnHighlightMenuItem (pDC, pMenuButton, rect, clrText);
				}

				return;
			}
		}
		else if (pBar != NULL && pBar->IsKindOf (RUNTIME_CLASS (CBCGPColorBar)))
		{
			index = -1;

			if (bChecked)
			{
				if (bDisabled)
				{
					index = 0;
				}
				else if (state == ButtonsIsPressed || state == ButtonsIsHighlighted)
				{
					index = 3;
				}
			}

			if (!bDisabled)
			{
				if (bPressed)
				{
					index = 2;
				}
				else if (state == ButtonsIsHighlighted)
				{
					if (index == -1)
					{
						index = 0;
					}

					index++;
				}
			}

			if (index == -1)
			{
				return;
			}

			pRenderer = &m_ctrlToolBarBtn;
		}

		if (pRenderer != NULL)
		{
			if (pRenderer != NULL)
			{
				pRenderer->Draw (pDC, rect, index);
			}

			return;
		}
	}

	CBCGPVisualManager2007::OnFillButtonInterior (pDC, pButton, rect, state);
}

void CBCGPVisualManagerCarbon::OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawTearOffCaption (pDC, rect, bIsActive);
		return;
	}

	COLORREF clrDark  = m_clrToolBarGradientDark;
	COLORREF clrLight = m_clrToolBarGradientLight;

	if (bIsActive)
	{
		clrDark  = m_clrHighlightGradientDark;
		clrLight = m_clrHighlightGradientLight;
	}

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect,
					clrDark,
					clrLight,
					TRUE);

    if (bIsActive)
    {
		pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
    }

    m_ToolBarTear.DrawEx (pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
}

void CBCGPVisualManagerCarbon::OnDrawMenuScrollButton (CDC* pDC, CRect rect, BOOL bIsScrollDown, 
												 BOOL bIsHighlited, BOOL bIsPressed,
												 BOOL bIsDisabled)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawMenuScrollButton (pDC, rect, bIsScrollDown, bIsHighlited, bIsPressed, bIsDisabled);
		return;
	}

	ASSERT_VALID (pDC);

	COLORREF clrDark  = m_clrHighlightGradientDark;
	COLORREF clrLight = m_clrHighlightGradientLight;

	if (bIsHighlited)
	{
		clrDark  = m_clrHighlightCheckedGradientDark;
		clrLight = m_clrHighlightCheckedGradientLight;
	}

	CBCGPDrawManager dm (*pDC);

	if (!bIsScrollDown)
	{
		rect.top--;
		dm.FillGradient (rect,
						clrDark,
						clrLight,
						TRUE);
	}
	else
	{
		dm.FillGradient (rect,
						clrLight,
						clrDark,
						TRUE);
	}

	if (bIsHighlited)
	{
		pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}

	CBCGPMenuImages::Draw (pDC, bIsScrollDown ? CBCGPMenuImages::IdArowDown : CBCGPMenuImages::IdArowUp, rect);
}

void CBCGPVisualManagerCarbon::OnDrawMenuSystemButton (CDC* pDC, CRect rect, 
												UINT uiSystemCommand, 
												UINT nStyle, BOOL bHighlight)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawMenuSystemButton (pDC, rect, 
												uiSystemCommand, nStyle, bHighlight);
		return;
	}

	CBCGPMenuImages::IMAGES_IDS imageID = (CBCGPMenuImages::IMAGES_IDS)-1;

	switch (uiSystemCommand)
	{
	case SC_CLOSE:
		imageID = CBCGPMenuImages::IdClose;
		break;
	case SC_MINIMIZE:
		imageID = CBCGPMenuImages::IdMinimize;
		break;
	case SC_RESTORE:
		imageID = CBCGPMenuImages::IdRestore;
		break;
	}

	if (imageID == -1)
	{
		return;
	}

	ASSERT_VALID (pDC);

	BOOL bDisabled = (nStyle & TBBS_DISABLED);
	BOOL bPressed = (nStyle & TBBS_PRESSED);

	if (!bDisabled)
	{
		int nIndex = 0;
		if (bPressed && bHighlight)
		{
			nIndex = 2;
		}
		else if (bHighlight)
		{
			nIndex = 1;
		}

		if (imageID == CBCGPMenuImages::IdClose)
		{
			nIndex += 3;
		}

		m_ctrlMiniSysBtn.Draw (pDC, rect, nIndex);
	}

	CBCGPMenuImages::IMAGE_STATE imageState = CBCGPMenuImages::ImageWhite;
	if (bDisabled)
	{
		imageState = CBCGPMenuImages::ImageDkGray;
	}
	else if (!bPressed && !bHighlight)
	{
		imageState = CBCGPMenuImages::ImageGray;
	}

	CBCGPMenuImages::Draw (pDC, imageID, rect, imageState);
}

//**************************************************************************************
BOOL CBCGPVisualManagerCarbon::OnDrawPushButton (CDC* pDC, CRect rect, CBCGPButton* pButton, COLORREF& clrText)
{
	BOOL bRes = CBCGPVisualManager2007::OnDrawPushButton (pDC, rect, pButton, clrText);

	if (CanDrawImage ())
	{	
		clrText = m_clrRibbonBarBtnText;

		if (pButton->GetSafeHwnd() != NULL)
		{
			BOOL bDisabled    = !pButton->IsWindowEnabled ();
			BOOL bFocused     = pButton->GetSafeHwnd () == ::GetFocus ();
			BOOL bHighlighted = pButton->IsHighlighted ();

			bHighlighted |= bFocused;

			if (bDisabled)
			{
				clrText = m_clrRibbonBarBtnTextDisabled;
			}
			else if (bHighlighted)
			{
				clrText = m_clrRibbonBarBtnTextHighlighted;
			}
		}
	}

	return bRes;
}

#ifndef BCGP_EXCLUDE_TASK_PANE

void CBCGPVisualManagerCarbon::OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawScrollButtons(pDC, rect, nBorderSize, iImage, bHilited);
		return;
	}

	CRect rt (rect);
	rt.top--;

	COLORREF clrDark  = m_clrHighlightGradientDark;
	COLORREF clrLight = m_clrHighlightGradientLight;

	if (bHilited)
	{
		clrDark  = m_clrHighlightCheckedGradientDark;
		clrLight = m_clrHighlightCheckedGradientLight;
	}

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rt,
					clrDark,
					clrLight,
					TRUE);

	if (bHilited)
	{
		pDC->Draw3dRect (rt, m_clrPressedButtonBorder, m_clrPressedButtonBorder);
	}

	CBCGPMenuImages::Draw (pDC, (CBCGPMenuImages::IMAGES_IDS) iImage, rect);
}

#endif

#ifndef BCGP_EXCLUDE_RIBBON

COLORREF CBCGPVisualManagerCarbon::OnDrawRibbonPanel (CDC* pDC, CBCGPRibbonPanel* pPanel, CRect rectPanel, CRect rectCaption)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2007::OnDrawRibbonPanel (pDC, pPanel, rectPanel, rectCaption);
	}

	ASSERT_VALID(pPanel);

	if (pPanel->IsBackstageView())
	{
		return (COLORREF)-1;
	}

	COLORREF clrText = CBCGPVisualManager2007::OnDrawRibbonPanel (pDC, pPanel, rectPanel, rectCaption);
	
	if (CanDrawImage () && pPanel->IsMainPanel ())
	{
		clrText = m_clrToolBarBtnText;
	}

	return clrText;
}

COLORREF CBCGPVisualManagerCarbon::OnFillRibbonButton (CDC* pDC, CBCGPRibbonButton* pButton)
{
	ASSERT_VALID(pButton);

	COLORREF clrText = CBCGPVisualManager2007::OnFillRibbonButton (pDC, pButton);
	
	if (CanDrawImage ())
	{
		if ((pButton->IsHighlighted () && !pButton->IsDisabled ()) || pButton->IsFocused ())
		{
			clrText = m_clrToolBarBtnTextHighlighted;
		}
		else if (pButton->IsStatusBarMode())
		{
			clrText = m_clrStatusBarText;
		}

	}

	return clrText;
}

COLORREF CBCGPVisualManagerCarbon::OnFillRibbonMainPanelButton (CDC* pDC, CBCGPRibbonButton* pButton)
{
	COLORREF clrText = CBCGPVisualManager2007::OnFillRibbonMainPanelButton (pDC, pButton);

	if (CanDrawImage () && !pButton->IsHighlighted () && !pButton->IsDisabled ())
	{
		clrText = globalData.clrBtnText;
	}

	return clrText;
}

#endif

void CBCGPVisualManagerCarbon::OnFillPropSheetHeaderArea(CDC* pDC, CBCGPPropertySheet* pPropSheet, CRect rect, BOOL& bDrawBottomLine)
{
	ASSERT_VALID (pDC);

	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnFillPropSheetHeaderArea(pDC, pPropSheet, rect, bDrawBottomLine);
		return;
	}

	COLORREF clr1 = globalData.clrBarLight;
	COLORREF clr2 = globalData.clrBarFace;

	CBCGPDrawManager dm(*pDC);
	dm.Fill4ColorsGradient(rect, clr1, clr2, clr2, clr1, TRUE);

	bDrawBottomLine = FALSE;
}

void CBCGPVisualManagerCarbon::OnDrawCaptionBarInfoArea (CDC* pDC, CBCGPCaptionBar* pBar, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawCaptionBarInfoArea (pDC, pBar, rect);
		return;
	}

	ASSERT_VALID (pDC);

	COLORREF clrFill = pBar->m_clrBarBackground != -1 ? pBar->m_clrBarBackground : m_ToolTipParams.m_clrFillGradient;
	COLORREF clrGradient = pBar->m_clrBarBackground != -1 ? CBCGPDrawManager::PixelAlpha (pBar->m_clrBarBackground, 150) : m_ToolTipParams.m_clrFill;
	COLORREF clrBorder = pBar->m_clrBarBackground != -1 ? CBCGPDrawManager::PixelAlpha (pBar->m_clrBarBackground, 80) : m_ToolTipParams.m_clrBorder;

	CBCGPDrawManager dm (*pDC);

	dm.FillGradient (rect, clrFill, clrGradient, TRUE);
	pDC->Draw3dRect(rect, clrBorder, clrBorder);
}

BOOL CBCGPVisualManagerCarbon::OnDrawBrowseButton (CDC* pDC, CRect rect, CBCGPEdit* pEdit, CBCGPVisualManager::BCGBUTTON_STATE state, COLORREF& clrText)
{
	ASSERT_VALID(pEdit);

	if (!CanDrawImage () || (pEdit != NULL && !pEdit->m_bVisualManagerStyle))
	{
		return CBCGPVisualManager2003::OnDrawBrowseButton(pDC, rect, pEdit, state, clrText);
	}

	CBCGPVisualManager2007::OnDrawBrowseButton(pDC, rect, pEdit, state, clrText);

	if (pEdit->IsWindowEnabled ())
	{
		clrText = globalData.clrBarDkShadow;
	}

	return TRUE;
}

CBrush& CBCGPVisualManagerCarbon::GetEditCtrlBackgroundBrush(CBCGPEdit* pEdit)
{
	if (CanDrawImage () && pEdit->GetSafeHwnd() != NULL && !pEdit->IsWindowEnabled())
	{
		return globalData.brLight;
	}
	
	return CBCGPVisualManager2007::GetEditCtrlBackgroundBrush(pEdit);
}

COLORREF CBCGPVisualManagerCarbon::OnDrawMenuLabel (CDC* pDC, CRect rect)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnDrawMenuLabel (pDC, rect);
	}
	
	ASSERT_VALID (pDC);
	
	pDC->FillRect (rect, m_brGroupBackground.GetSafeHandle () != NULL ? &m_brGroupBackground : &m_brBarBkgnd);
	return m_clrGroupText != (COLORREF)-1 ? m_clrGroupText : globalData.clrBarText;
}

void CBCGPVisualManagerCarbon::OnDrawControlBorder (CDC* pDC, CRect rect, CWnd* pWndCtrl, BOOL bDrawOnGlass)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawControlBorder (pDC, rect, pWndCtrl, bDrawOnGlass);
		return;
	}
	
	COLORREF clrBorder = m_clrSeparatorLight;
	
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

COLORREF CBCGPVisualManagerCarbon::GetListControlMarkedColor(CBCGPListCtrl* pListCtrl)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetListControlMarkedColor(pListCtrl);
	}
	
	return m_clrSeparatorLight;
}

#ifndef BCGP_EXCLUDE_POPUP_WINDOW

COLORREF CBCGPVisualManagerCarbon::GetPopupWindowLinkTextColor(CBCGPPopupWindow* pPopupWnd, BOOL bIsHover)
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManager2003::GetPopupWindowLinkTextColor(pPopupWnd, bIsHover);
	}

	return bIsHover
		? (m_clrLinkHotText == (COLORREF)-1 ? globalData.clrHotLinkText : m_clrLinkHotText)
		: (m_clrLinkText == (COLORREF)-1 ? globalData.clrHotLinkText : m_clrLinkText);
}

#endif

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

void CBCGPVisualManagerCarbon::DrawGanttHeaderCell (const CBCGPGanttChart* pChart, CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, BOOL bHilite)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::DrawGanttHeaderCell (pChart, dc, cellInfo, bHilite);
		return;
	}

	dc.FillRect (cellInfo.rectCell, &globalData.brBarFace);
	dc.Draw3dRect (cellInfo.rectCell, globalData.clrBarShadow, globalData.clrBarShadow);
}
//********************************************************************************
COLORREF CBCGPVisualManagerCarbon::GetGanttHeaderTextColor (BOOL bHilite) const
{
    if (!CanDrawImage ())
    {
        return CBCGPVisualManager2003::GetGanttHeaderTextColor (bHilite);
    }

	return globalData.clrBarText;
}
//********************************************************************************
void CBCGPVisualManagerCarbon::GetGanttColors (const CBCGPGanttChart* pChart, BCGP_GANTT_CHART_COLORS& colors, COLORREF clrBack) const
{
	CBCGPVisualManager2007::GetGanttColors(pChart, colors, clrBack);

	if (!CanDrawImage ())
	{
		return;
	}

	if (pChart->GetSafeHwnd() != NULL && !pChart->IsVisualManagerStyle())
	{
		return;
	}

	if (clrBack == CLR_DEFAULT)
	{
		clrBack = globalData.clrBarLight;
	}	

    colors.clrBackground      = globalData.clrBarFace;
	colors.clrShadows         = m_clrMenuShadowBase;

	colors.clrRowBackground   = colors.clrBackground;
    colors.clrGridLine0       = globalData.clrBarShadow;
    colors.clrGridLine1       = globalData.clrBarShadow;
    colors.clrSelection       = globalData.clrHilite;
	colors.clrSelectionBorder = globalData.clrHilite;

	colors.clrRowBackground  = globalData.clrBarFace;
	colors.clrRowDayOff      = CBCGPDrawManager::ColorMakeDarker(globalData.clrBarFace);
	colors.clrConnectorLines = globalData.clrBarLight;
}

#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)
