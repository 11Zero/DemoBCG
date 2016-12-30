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
// BCGPVisualManager2007.cpp: implementation of the CBCGPVisualManager2007 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPGlobalUtils.h"
#include "BCGPVisualManager2007.h"
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
#include "BCGCBProVer.h"
#include "BCGPGroup.h"
#include "BCGPDialog.h"
#include "BCGPPropertySheet.h"
#include "BCGPMath.h"

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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
	extern BOOL g_bToolBarImagesTrace;
#endif

const CBCGPVisualManager2007::Style c_StyleDefault = 
	CBCGPVisualManager2007::VS2007_LunaBlue;

CBCGPVisualManager2007::Style CBCGPVisualManager2007::m_Style = c_StyleDefault;

CString	CBCGPVisualManager2007::m_strStylePrefix;

HINSTANCE CBCGPVisualManager2007::m_hinstRes = NULL;

BOOL CBCGPVisualManager2007::m_bAutoFreeRes = FALSE;

COLORREF CBCGPVisualManager2007::m_clrBase = (COLORREF)-1;
COLORREF CBCGPVisualManager2007::m_clrTarget = (COLORREF)-1;

static COLORREF CalculateHourLineColor (COLORREF clrBase, BOOL bWorkingHours, BOOL bHour)
{
	double H, S, V;
	CBCGPDrawManager::RGBtoHSV (clrBase, &H, &S, &V);

	if (bHour)
	{
		S = S * 0.77;
		V = min (V * 1.03, 1.0);
	}
	else
	{
		if (bWorkingHours)
		{
			S = S * 0.2;
			V = min (V * 1.14, 1.0);
		}
		else
		{
			S = S * 0.34;
			V = min (V * 1.12, 1.0);
		}
	}

	return CBCGPDrawManager::HSVtoRGB (H, S, V);
}

static COLORREF CalculateWorkingColor (COLORREF /*clrBase*/)
{
	return RGB (255, 255, 255);
}

static COLORREF CalculateNonWorkingColor (COLORREF clrBase, BOOL bLight)
{
	if (bLight)
	{
		return CalculateHourLineColor (clrBase, TRUE, FALSE);
	}

	return CalculateHourLineColor (clrBase, FALSE, TRUE);
}

static COLORREF CalculateSelectionColor (COLORREF clrBase)
{
	double H, S, V;
	CBCGPDrawManager::RGBtoHSV (clrBase, &H, &S, &V);

	return CBCGPDrawManager::HSVtoRGB (H, min (S * 1.88, 1.0), V * 0.56);
}

static COLORREF CalculateSeparatorColor (COLORREF clrBase)
{
	double H, S, V;
	CBCGPDrawManager::RGBtoHSV (clrBase, &H, &S, &V);

	return CBCGPDrawManager::HSVtoRGB (H, min (S * 1.6, 1.0), V * 0.85);
}

CBCGPBitmapCache::CBitmapCacheItem::CBitmapCacheItem()
{
	m_bMirror = FALSE;
}

CBCGPBitmapCache::CBitmapCacheItem::~CBitmapCacheItem()
{
}

void CBCGPBitmapCache::CBitmapCacheItem::AddImage (HBITMAP hBmp)
{
	m_Images.AddImage (hBmp, TRUE);
}

void CBCGPBitmapCache::CBitmapCacheItem::Cache (const CSize& size, CBCGPControlRenderer& renderer)
{
	m_Images.Clear ();

	const int nCount = renderer.GetImageCount ();

	if (nCount > 0)
	{
		m_Images.SetImageSize (size);
		m_Images.SetTransparentColor ((COLORREF)-1);

		for (int i = 0; i < nCount; i++)
		{
			BITMAPINFO bi;
			memset(&bi, 0, sizeof(BITMAPINFO));

			// Fill in the BITMAPINFOHEADER
			bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
			bi.bmiHeader.biWidth       = size.cx;
			bi.bmiHeader.biHeight      = size.cy;
			bi.bmiHeader.biPlanes      = 1;
			bi.bmiHeader.biBitCount    = 32;
			bi.bmiHeader.biCompression = BI_RGB;
			bi.bmiHeader.biSizeImage   = size.cy * size.cx * 4;

			LPBYTE pBits = NULL;
			HBITMAP hDib = ::CreateDIBSection (
				NULL, &bi, DIB_RGB_COLORS, (void **)&pBits,
				NULL, NULL);

			if (hDib == NULL || pBits == NULL)
			{
				ASSERT (FALSE);
				break;
			}

			CDC dc;
			dc.CreateCompatibleDC (NULL);

			HBITMAP hOldDib = (HBITMAP)::SelectObject (dc.GetSafeHdc (), hDib);

			m_bMirror = renderer.IsMirror ();
			if (m_bMirror)
			{
				renderer.Mirror ();
			}

			renderer.Draw (&dc, CRect (0, 0, size.cx, size.cy), i);

			if (m_bMirror)
			{
				renderer.Mirror ();
			}

			::SelectObject (dc.GetSafeHdc (), hOldDib);

			AddImage (hDib);
			
			::DeleteObject (hDib);
		}
	}
}

void CBCGPBitmapCache::CBitmapCacheItem::Draw (CDC* pDC, CRect rect, int iImageIndex/* = 0*/,
										   BYTE alphaSrc/* = 255*/)
{
	m_Images.DrawEx (pDC, CRect (rect.TopLeft (), m_Images.GetImageSize ()), 
		iImageIndex, CBCGPToolBarImages::ImageAlignHorzLeft, 
		CBCGPToolBarImages::ImageAlignVertTop, CRect (0, 0, 0, 0), alphaSrc);
}

void CBCGPBitmapCache::CBitmapCacheItem::DrawY (CDC* pDC, CRect rect, CSize sides, 
											int iImageIndex/* = 0*/, BYTE alphaSrc/* = 255*/)
{
	CRect rectImage (CPoint (0, 0), m_Images.GetImageSize ());

	ASSERT (rect.Height () == rectImage.Height ());

	if (sides.cx > 0)
	{
		CRect rt (rectImage);
		if (m_bMirror)
		{
			rt.left = rectImage.right - sides.cx;
			rectImage.right = rt.left;
		}
		else
		{
			rt.right = rt.left + sides.cx;
			rectImage.left = rt.right;
		}

		m_Images.DrawEx (pDC, rect, iImageIndex, CBCGPToolBarImages::ImageAlignHorzLeft,
			CBCGPToolBarImages::ImageAlignVertTop, rt, alphaSrc);
	}

	if (sides.cy > 0)
	{
		CRect rt (rectImage);
		if (m_bMirror)
		{
			rt.right = rectImage.left + sides.cy;
			rectImage.left = rt.right;
		}
		else
		{
			rt.left = rectImage.right - sides.cy;
			rectImage.right = rt.left;
		}

		m_Images.DrawEx (pDC, rect, iImageIndex, CBCGPToolBarImages::ImageAlignHorzRight,
			CBCGPToolBarImages::ImageAlignVertTop, rt, alphaSrc);
	}

	if (rectImage.Width () > 0)
	{
		rect.DeflateRect (sides.cx, 0, sides.cy, 0);
		m_Images.DrawEx (pDC, rect, iImageIndex, CBCGPToolBarImages::ImageAlignHorzStretch,
			CBCGPToolBarImages::ImageAlignVertTop, rectImage, alphaSrc);
	}
}

CBCGPBitmapCache::CBCGPBitmapCache()
{
}

CBCGPBitmapCache::~CBCGPBitmapCache()
{
	Clear ();
}

void CBCGPBitmapCache::Clear ()
{
	for (int i = 0; i < m_Cache.GetSize (); i++)
	{
		if (m_Cache[i] != NULL)
		{
			delete m_Cache[i];
		}
	}

	m_Cache.RemoveAll ();
	m_Sizes.RemoveAll ();
}

int CBCGPBitmapCache::Cache (const CSize& size, CBCGPControlRenderer& renderer)
{
	if (FindIndex (size) != -1)
	{
		ASSERT (FALSE);
		return -1;
	}

	CBitmapCacheItem* pItem = new CBitmapCacheItem;
	pItem->Cache (size, renderer);

	int nCache = (int) m_Cache.Add (pItem);

#ifdef _DEBUG
	int nSize = (int) m_Sizes.Add (size);
	ASSERT (nCache == nSize);
#else
	m_Sizes.Add (size);
#endif

	return nCache;
}

int CBCGPBitmapCache::CacheY (int height, CBCGPControlRenderer& renderer)
{
	CSize size (renderer.GetParams ().m_rectImage.Width (), height);

	return Cache (size, renderer);
}

BOOL CBCGPBitmapCache::IsCached (const CSize& size) const
{
	return FindIndex (size) != -1;
}

int CBCGPBitmapCache::FindIndex (const CSize& size) const
{
	int nRes = -1;
	for (int i = 0; i < m_Sizes.GetSize (); i++)
	{
		if (size == m_Sizes[i])
		{
			nRes = i;
			break;
		}
	}

	return nRes;
}

CBCGPBitmapCache::CBitmapCacheItem* CBCGPBitmapCache::Get (const CSize& size)
{
	int nIndex = FindIndex (size);

	if (nIndex != -1)
	{
		return m_Cache[nIndex];
	}

	return NULL;
}

CBCGPBitmapCache::CBitmapCacheItem* CBCGPBitmapCache::Get (int nIndex)
{
	if (0 <= nIndex && nIndex < m_Cache.GetSize ())
	{
		return m_Cache[nIndex];
	}

	return NULL;
}


IMPLEMENT_DYNCREATE(CBCGPVisualManager2007, CBCGPVisualManager2003)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPVisualManager2007::CBCGPVisualManager2007()
	: m_bNcTextCenter   (FALSE)
	, m_bLoaded         (FALSE)
	, m_bPlannerBlack   (FALSE)
	, m_bToolTipParams  (FALSE)
{
	m_szNcBtnSize[0]    = CSize (0, 0);
	m_szNcBtnSize[1]    = CSize (0, 0);

	m_ptRibbonMainImageOffset = CPoint (0, -1);

	m_clrEditBorder                = (COLORREF)(-1);
	m_clrEditBorderDisabled        = (COLORREF)(-1);
	m_clrEditBorderHighlighted     = (COLORREF)(-1);
	m_clrEditSelection             = (COLORREF)(-1);
	m_clrComboBorder               = (COLORREF)(-1);
	m_clrComboBorderDisabled       = (COLORREF)(-1);
    m_clrComboBorderPressed        = (COLORREF)(-1);
	m_clrComboBorderHighlighted    = (COLORREF)(-1);
	m_clrComboBtnStart             = (COLORREF)(-1);
	m_clrComboBtnFinish            = (COLORREF)(-1);
	m_clrComboBtnBorder            = (COLORREF)(-1);
	m_clrComboBtnDisabledStart     = (COLORREF)(-1);
	m_clrComboBtnDisabledFinish    = (COLORREF)(-1);
	m_clrComboBtnBorderDisabled    = (COLORREF)(-1);
	m_clrComboBtnPressedStart      = (COLORREF)(-1);
	m_clrComboBtnPressedFinish     = (COLORREF)(-1);
	m_clrComboBtnBorderPressed     = (COLORREF)(-1);
	m_clrComboBtnHighlightedStart  = (COLORREF)(-1);
	m_clrComboBtnHighlightedFinish = (COLORREF)(-1);
	m_clrComboBtnBorderHighlighted = (COLORREF)(-1);
	m_clrComboSelection            = (COLORREF)(-1);

	m_clrGroupText = (COLORREF)-1;

	m_clrHeaderNormalText      = (COLORREF)-1;
	m_clrHeaderHighlightedText = (COLORREF)-1;
	m_clrHeaderPressedText     = (COLORREF)-1;

	m_clrTabTextActive = (COLORREF)-1;
	m_clrTabTextInactive = (COLORREF)-1;
	m_clrTabTextHighlighted = (COLORREF)-1;
	m_clrTab3DTextActive = (COLORREF)-1;
	m_clrTab3DTextInactive = (COLORREF)-1;
	m_clrTab3DTextHighlighted = (COLORREF)-1;
	m_clrTabFlatTextActive = (COLORREF)-1;
	m_clrTabFlatTextInactive = (COLORREF)-1;
	m_clrTabFlatTextHighlighted = (COLORREF)-1;
	m_clrTab3DFace = CLR_DEFAULT;
	m_clrTab3DBlack = CLR_DEFAULT;
	m_clrTab3DDark = CLR_DEFAULT;
	m_clrTab3DDarkShadow = CLR_DEFAULT;
	m_clrTab3DLight = CLR_DEFAULT;
	m_clrTab3DHighlight = CLR_DEFAULT;
	m_clrTabFlatFace = CLR_DEFAULT;
	m_clrTabFlatBlack = CLR_DEFAULT;
	m_clrTabFlatDark = CLR_DEFAULT;
	m_clrTabFlatDarkShadow = CLR_DEFAULT;
	m_clrTabFlatLight = CLR_DEFAULT;
	m_clrTabFlatHighlight = CLR_DEFAULT;

	m_clrRibbonMainPanelBkgnd = CLR_DEFAULT;

	m_clrRibbonKeyTipTextNormal   = (COLORREF)(-1);
	m_clrRibbonKeyTipTextDisabled = (COLORREF)(-1);

	m_clrRibbonCategoryTextActive = (COLORREF)-1;

	m_clrRibbonHyperlinkInactive = (COLORREF)-1;
	m_clrRibbonHyperlinkActive = (COLORREF)-1;
	m_clrRibbonStatusbarHyperlinkInactive = (COLORREF)-1;
	m_clrRibbonStatusbarHyperlinkActive = (COLORREF)-1;

	m_clrDlgBackground  = (COLORREF)-1;
	m_clrDlgButtonsArea = (COLORREF)-1;

	m_clrAppCaptionActiveStart = (COLORREF)-1;
	m_clrAppCaptionActiveFinish = (COLORREF)-1;

	m_bUseScenicColors = TRUE;
}
//*****************************************************************************
CBCGPVisualManager2007::~CBCGPVisualManager2007()
{
	Style style = m_Style;
	CleanStyle ();
	m_Style = style;

	CBCGPMenuImages::SetColor (CBCGPMenuImages::ImageBlack, (COLORREF)-1);
}

CString CBCGPVisualManager2007::MakeResourceID (LPCTSTR lpszID)
{
	CString strResID (lpszID);
	ASSERT(!strResID.IsEmpty ());

	if (!m_strStylePrefix.IsEmpty ())
	{
		strResID = m_strStylePrefix + strResID;
	}

	return strResID;
}

CString CBCGPVisualManager2007::GetStyleResourceID (Style style)
{
	CString strResID (_T("IDX_BCG_OFFICE2007_STYLE"));

#if !defined _AFXDLL || defined _BCGCBPRO_STATIC_

	CString strStylePrefix;

	switch (style)
	{
	case VS2007_LunaBlue:
		strStylePrefix = _T("BLUE_");
		break;

	case VS2007_ObsidianBlack:
		strStylePrefix = _T("BLACK_");
		break;

	case VS2007_Aqua:
		strStylePrefix = _T("AQUA_");
		break;
		
	case VS2007_Silver:
		strStylePrefix = _T("SILVER_");
		break;

	default:
		ASSERT (FALSE);
	}

	strResID = strStylePrefix + strResID;

#else

	UNREFERENCED_PARAMETER (style);

#endif

	return strResID;
};

BOOL CBCGPVisualManager2007::SetStyle (Style style, LPCTSTR lpszPath)
{
	if (CBCGPVisualManager::GetInstance () == NULL ||
		!CBCGPVisualManager::GetInstance ()->IsKindOf (RUNTIME_CLASS(CBCGPVisualManager2007)))
	{
		m_Style = style;
		return TRUE;
	}

	if (m_Style == style && m_hinstRes > (HINSTANCE) 32)
	{
		return TRUE;
	}

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_

	CString strTheme;

	switch (style)
	{
	case VS2007_LunaBlue:
		strTheme = _T("Luna");
		break;

	case VS2007_ObsidianBlack:
		strTheme = _T("Obsidian");
		break;

	case VS2007_Aqua:
		strTheme = _T("Aqua");
		break;
		
	case VS2007_Silver:
		strTheme = _T("Silver");
		break;

	default:
		ASSERT (FALSE);
		return FALSE;
	}

	CString strVer;
	strVer.Format (_T("%d%d"), _BCGCBPRO_VERSION_MAJOR, _BCGCBPRO_VERSION_MINOR);

	CString strStyleDLLName = _T("BCGPStyle2007") + strTheme + strVer + _T(".dll");

	CString strStyleDLLPath;

	if (lpszPath != NULL && _tcslen (lpszPath) > 0)
	{
		strStyleDLLPath = lpszPath;

		if (strStyleDLLPath [strStyleDLLPath.GetLength () - 1] != TCHAR('\\'))
		{
			strStyleDLLPath += _T("\\");
		}

		strStyleDLLPath += strStyleDLLName;
	}
	else
	{
		strStyleDLLPath = strStyleDLLName;
	}

	Style oldStyle = m_Style;

	CleanStyle ();

	HINSTANCE hinstRes = LoadLibrary (strStyleDLLPath);

	if (hinstRes <= (HINSTANCE) 32)
	{
		m_Style = oldStyle;

		TRACE(_T("Cannot load Style DLL: %s\r\n"), strStyleDLLPath);
		ASSERT (FALSE);
		return FALSE;
	}

	m_Style = style;

	SetResourceHandle (hinstRes);
	m_bAutoFreeRes = TRUE;

#else

	UNREFERENCED_PARAMETER (lpszPath);

	CString strStyle (GetStyleResourceID (style));
	HINSTANCE hinstRes = AfxFindResourceHandle (strStyle, RT_BCG_STYLE_XML);

	if (::FindResource(hinstRes, strStyle, RT_BCG_STYLE_XML) == NULL)
	{
		TRACE(_T("Cannot load Style: %s\r\n"), strStyle);
		ASSERT (FALSE);
		return FALSE;
	}

	CleanStyle ();

	m_Style = style;

	SetResourceHandle (hinstRes);

#endif

	return TRUE;
}
//*****************************************************************************
CBCGPVisualManager2007::Style CBCGPVisualManager2007::GetStyle ()
{
	return m_Style;
}
//*****************************************************************************
void CBCGPVisualManager2007::SetResourceHandle (HINSTANCE hinstRes)
{
	m_bAutoFreeRes = FALSE;

	if (m_hinstRes != hinstRes)
	{
		m_hinstRes = hinstRes;

		if (CBCGPVisualManager::GetInstance ()->IsKindOf (
			RUNTIME_CLASS (CBCGPVisualManager2007)))
		{
			CBCGPVisualManager::GetInstance ()->OnUpdateSystemColors ();
		}
	}
}
//*****************************************************************************
void CBCGPVisualManager2007::CleanStyle ()
{
	if (m_bAutoFreeRes && m_hinstRes > (HINSTANCE) 32)
	{
		::FreeLibrary (m_hinstRes);
	}

	m_hinstRes = NULL;
	m_Style = c_StyleDefault;
	m_strStylePrefix.Empty ();
}
//*****************************************************************************
void CBCGPVisualManager2007::SetCustomColor (COLORREF clrTarget)
{
	if (clrTarget == (COLORREF)-1)
	{
		m_clrBase = (COLORREF)-1;
	}
	else
	{
		switch (m_Style)
		{
		case VS2007_LunaBlue:
			m_clrBase = RGB (206, 221, 238);	// Only 'LunaBlue' style is supported
			break;

		default:
			ASSERT(FALSE);
			clrTarget = m_clrBase = (COLORREF)-1;
		}
	}

	if (m_clrTarget == clrTarget)
	{
		return;
	}

	m_clrTarget = clrTarget;

	if (CBCGPVisualManager::GetInstance ()->IsKindOf (
		RUNTIME_CLASS (CBCGPVisualManager2007)))
	{
		CBCGPVisualManager::GetInstance ()->OnUpdateSystemColors ();
		CBCGPMenuImages::CleanUp ();
	}
}
//*****************************************************************************
void CBCGPVisualManager2007::CleanUp ()
{
	m_clrEditBorder                = (COLORREF)(-1);
	m_clrEditBorderDisabled        = (COLORREF)(-1);
	m_clrEditBorderHighlighted     = (COLORREF)(-1);
	m_clrEditSelection             = (COLORREF)(-1);
	m_clrComboBorder               = (COLORREF)(-1);
	m_clrComboBorderDisabled       = (COLORREF)(-1);
    m_clrComboBorderPressed        = (COLORREF)(-1);
	m_clrComboBorderHighlighted    = (COLORREF)(-1);
	m_clrComboBtnStart             = (COLORREF)(-1);
	m_clrComboBtnFinish            = (COLORREF)(-1);
	m_clrComboBtnBorder            = (COLORREF)(-1);
	m_clrComboBtnDisabledStart     = (COLORREF)(-1);
	m_clrComboBtnDisabledFinish    = (COLORREF)(-1);
	m_clrComboBtnBorderDisabled    = (COLORREF)(-1);
	m_clrComboBtnPressedStart      = (COLORREF)(-1);
	m_clrComboBtnPressedFinish     = (COLORREF)(-1);
	m_clrComboBtnBorderPressed     = (COLORREF)(-1);
	m_clrComboBtnHighlightedStart  = (COLORREF)(-1);
	m_clrComboBtnHighlightedFinish = (COLORREF)(-1);
	m_clrComboBtnBorderHighlighted = (COLORREF)(-1);
	m_clrComboSelection            = (COLORREF)(-1);
	m_ctrlComboBoxBtn.CleanUp ();

	m_ToolBarGripper.Clear ();
	m_ToolBarTear.Clear ();
	m_ctrlToolBarBorder.CleanUp ();

	m_ctrlStatusBarBack.CleanUp ();
	m_ctrlStatusBarBack_Ext.CleanUp ();
	m_StatusBarPaneBorder.Clear ();
	m_StatusBarSizeBox.Clear ();

	m_SysBtnBack[0].CleanUp ();
	m_SysBtnBack[1].CleanUp ();
	m_SysBtnBackC[0].CleanUp ();
	m_SysBtnBackC[1].CleanUp ();
	m_SysBtnClose[0].Clear ();
	m_SysBtnClose[1].Clear ();
	m_SysBtnRestore[0].Clear ();
	m_SysBtnRestore[1].Clear ();
	m_SysBtnMaximize[0].Clear ();
	m_SysBtnMaximize[1].Clear ();
	m_SysBtnMinimize[0].Clear ();
	m_SysBtnMinimize[1].Clear ();
	m_SysBtnHelp[0].Clear ();
	m_SysBtnHelp[1].Clear ();

	m_brMainClientArea.DeleteObject ();

	m_AppCaptionFont.DeleteObject ();
	m_penSeparator2.DeleteObject ();

	m_brGroupBackground.DeleteObject ();
	m_clrGroupText = (COLORREF)-1;

	m_penSeparatorDark.DeleteObject ();

	m_ctrlMainBorder.CleanUp ();
	m_ctrlMDIChildBorder.CleanUp ();
	m_ctrlDialogBorder.CleanUp ();
	m_ctrlMainBorderCaption.CleanUp ();
	m_ctrlAppBorderCaption.CleanUp();
	m_ctrlPopupBorder.CleanUp ();
	m_ctrlPopupResizeBar.CleanUp ();
	m_PopupResizeBar_HV.Clear ();
	m_PopupResizeBar_HVT.Clear ();
	m_PopupResizeBar_V.Clear ();

	m_ctrlMenuBarBtn.CleanUp ();

	m_ctrlMenuItemBack.CleanUp ();
    m_MenuItemMarkerC.Clear ();
    m_MenuItemMarkerR.Clear ();
	m_MenuItemPin.Clear ();
	m_ctrlMenuItemShowAll.CleanUp ();
	m_ctrlMenuHighlighted[0].CleanUp ();
	m_ctrlMenuHighlighted[1].CleanUp ();
	m_ctrlMenuButtonBorder.CleanUp ();
	m_ctrlMenuScrollBtn[0].CleanUp ();
	m_ctrlMenuScrollBtn[1].CleanUp ();

	m_ctrlToolBarBtn.CleanUp ();
	
#ifndef BCGP_EXCLUDE_TASK_PANE
	m_ctrlTaskScrollBtn.CleanUp ();
#endif

	m_clrHeaderNormalText      = (COLORREF)-1;
	m_clrHeaderHighlightedText = (COLORREF)-1;
	m_clrHeaderPressedText     = (COLORREF)-1;

	int i, j;
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2; j++)
		{
			m_ctrlScrollBar_Back[i][j].CleanUp ();
			m_ctrlScrollBar_Item[i][j].CleanUp ();
			m_ctrlScrollBar_ThumbBack[i][j].CleanUp ();
			m_ctrlScrollBar_ThumbIcon[i][j].CleanUp ();
		}
	}

	m_ctrlSliderThumb[0].CleanUp ();
	m_ctrlSliderThumb[1].CleanUp ();
	m_ctrlSliderThumb[2].CleanUp ();
	m_ctrlSliderThumb[3].CleanUp ();
	m_ctrlSliderThumb[4].CleanUp ();
	m_ctrlSliderThumb[5].CleanUp ();

	m_ctrlTab3D[0].CleanUp ();
	m_ctrlTab3D[1].CleanUp ();
	m_ctrlTabFlat[0].CleanUp ();
	m_ctrlTabFlat[1].CleanUp ();
	m_clrTabTextActive = (COLORREF)-1;
	m_clrTabTextInactive = (COLORREF)-1;
	m_clrTabTextHighlighted = (COLORREF)-1;
	m_clrTab3DTextActive = (COLORREF)-1;
	m_clrTab3DTextInactive = (COLORREF)-1;
	m_clrTab3DTextHighlighted = (COLORREF)-1;
	m_clrTabFlatTextActive = (COLORREF)-1;
	m_clrTabFlatTextInactive = (COLORREF)-1;
	m_clrTabFlatTextHighlighted = (COLORREF)-1;
	m_clrTab3DFace = CLR_DEFAULT;
	m_clrTab3DBlack = CLR_DEFAULT;
	m_clrTab3DDark = CLR_DEFAULT;
	m_clrTab3DDarkShadow = CLR_DEFAULT;
	m_clrTab3DLight = CLR_DEFAULT;
	m_clrTab3DHighlight = CLR_DEFAULT;
	m_clrTabFlatFace = CLR_DEFAULT;
	m_clrTabFlatBlack = CLR_DEFAULT;
	m_clrTabFlatDark = CLR_DEFAULT;
	m_clrTabFlatDarkShadow = CLR_DEFAULT;
	m_clrTabFlatLight = CLR_DEFAULT;
	m_clrTabFlatHighlight = CLR_DEFAULT;
	m_penTabFlatInner[0].DeleteObject ();
	m_penTabFlatInner[1].DeleteObject ();
	m_penTabFlatOuter[0].DeleteObject ();
	m_penTabFlatOuter[1].DeleteObject ();

	m_ctrlOutlookWndBar.CleanUp ();
	m_ctrlOutlookWndPageBtn.CleanUp ();
	m_ctrlOutlookSplitter.CleanUp ();

	m_ctrlRibbonCaptionQA.CleanUp ();
	m_ctrlRibbonCaptionQA_Glass.CleanUp ();
	m_ctrlRibbonCategoryBack.CleanUp ();
	m_ctrlRibbonCategoryTab.CleanUp ();
	m_ctrlRibbonCategoryTabSep.CleanUp ();
	m_ctrlRibbonCategoryBtnPage[0].CleanUp ();
	m_ctrlRibbonCategoryBtnPage[1].CleanUp ();
	m_ctrlRibbonPanelBack_T.CleanUp ();
	m_ctrlRibbonPanelBack_B.CleanUp ();
	m_RibbonPanelSeparator.Clear ();
	m_ctrlRibbonPanelQAT.CleanUp ();
	m_ctrlRibbonMainPanel.CleanUp ();
	m_ctrlRibbonMainPanelBorder.CleanUp ();
	m_ctrlRibbonBtnMainPanel.CleanUp ();

	m_clrRibbonMainPanelBkgnd = CLR_DEFAULT;
	m_brRibbonMainPanelBkgnd.DeleteObject ();

	m_ctrlRibbonBtnGroup_S.CleanUp ();
	m_ctrlRibbonBtnGroup_F.CleanUp ();
	m_ctrlRibbonBtnGroup_M.CleanUp ();
	m_ctrlRibbonBtnGroup_L.CleanUp ();
	m_ctrlRibbonBtnGroupMenu_F[0].CleanUp ();
	m_ctrlRibbonBtnGroupMenu_F[1].CleanUp ();
	m_ctrlRibbonBtnGroupMenu_M[0].CleanUp ();
	m_ctrlRibbonBtnGroupMenu_M[1].CleanUp ();
	m_ctrlRibbonBtnGroupMenu_L[0].CleanUp ();
	m_ctrlRibbonBtnGroupMenu_L[1].CleanUp ();
	m_ctrlRibbonBtn[0].CleanUp ();
	m_ctrlRibbonBtn[1].CleanUp ();
	m_ctrlRibbonBtnMenuH[0].CleanUp ();
	m_ctrlRibbonBtnMenuH[1].CleanUp ();
	m_ctrlRibbonBtnMenuV[0].CleanUp ();
	m_ctrlRibbonBtnMenuV[1].CleanUp ();
	m_ctrlRibbonBtnLaunch.CleanUp ();
	m_RibbonBtnLaunchIcon.Clear ();
	m_RibbonBtnMain.CleanUp ();
	m_ctrlRibbonBtnDefault.CleanUp ();
	m_ctrlRibbonBtnDefaultIcon.CleanUp ();
	m_RibbonBtnDefaultImage.Clear ();
	m_ctrlRibbonBtnDefaultQATIcon.CleanUp ();
	m_ctrlRibbonBtnDefaultQAT.CleanUp ();
	m_ctrlRibbonBtnCheck.CleanUp ();
	m_ctrlRibbonBtnRadio.CleanUp ();
	m_ctrlRibbonBtnPush.CleanUp ();
	m_ctrlRibbonBtnGroup.CleanUp ();
	m_ctrlRibbonBtnPalette[0].CleanUp ();
	m_ctrlRibbonBtnPalette[1].CleanUp ();
	m_ctrlRibbonBtnPalette[2].CleanUp ();
	m_ctrlRibbonBtnStatusPane.CleanUp ();
	m_ctrlRibbonSliderThumb.CleanUp ();
	m_ctrlRibbonSliderThumbA[0].CleanUp ();
	m_ctrlRibbonSliderThumbA[1].CleanUp ();
	m_ctrlRibbonSliderThumbA[2].CleanUp ();
	m_ctrlRibbonSliderThumbA[3].CleanUp ();
	m_ctrlRibbonSliderThumbA[4].CleanUp ();
	m_ctrlRibbonSliderBtnPlus.CleanUp ();
	m_ctrlRibbonSliderBtnMinus.CleanUp ();
	m_ctrlRibbonProgressBack.CleanUp ();
	m_ctrlRibbonProgressNormal.CleanUp ();
	m_ctrlRibbonProgressNormalExt.CleanUp ();
	m_ctrlRibbonProgressInfinity.CleanUp ();
	m_ctrlRibbonProgressBackV.CleanUp ();
	m_ctrlRibbonProgressNormalV.CleanUp ();
	m_ctrlRibbonProgressNormalExtV.CleanUp ();
	m_ctrlRibbonProgressInfinityV.CleanUp ();	
	m_ctrlRibbonBorder_QAT.CleanUp ();
	m_ctrlRibbonBorder_Floaty.CleanUp ();
	m_ctrlRibbonBorder_Panel.CleanUp ();

	m_ctrlRibbonKeyTip.CleanUp ();
	m_clrRibbonKeyTipTextNormal   = (COLORREF)(-1);
	m_clrRibbonKeyTipTextDisabled = (COLORREF)(-1);

	m_ctrlRibbonComboBoxBtn.CleanUp ();

	m_cacheRibbonCategoryBack.Clear ();
	m_cacheRibbonPanelBack_T.Clear ();
	m_cacheRibbonPanelBack_B.Clear ();
	m_cacheRibbonBtnDefault.Clear ();

	m_cacheRibbonBtnGroup_S.Clear ();
	m_cacheRibbonBtnGroup_F.Clear ();
	m_cacheRibbonBtnGroup_M.Clear ();
	m_cacheRibbonBtnGroup_L.Clear ();
	m_cacheRibbonBtnGroupMenu_F[0].Clear ();
	m_cacheRibbonBtnGroupMenu_M[0].Clear ();
	m_cacheRibbonBtnGroupMenu_L[0].Clear ();
	m_cacheRibbonBtnGroupMenu_F[1].Clear ();
	m_cacheRibbonBtnGroupMenu_M[1].Clear ();
	m_cacheRibbonBtnGroupMenu_L[1].Clear ();

	m_ctrlRibbonContextPanelBack_T.CleanUp ();
	m_ctrlRibbonContextPanelBack_B.CleanUp ();
	m_cacheRibbonContextPanelBack_T.Clear ();
	m_cacheRibbonContextPanelBack_B.Clear ();
	m_ctrlRibbonContextSeparator.CleanUp ();

	m_clrRibbonCategoryTextActive = (COLORREF)-1;

	for (i = 0; i < BCGPRibbonCategoryColorCount; i++)
	{
		m_ctrlRibbonContextCategory[i].CleanUp ();
	}

	m_clrPlannerTodayCaption[0] = RGB (247, 208, 112);
	m_clrPlannerTodayCaption[1] = RGB (251, 230, 148);
	m_clrPlannerTodayCaption[2] = RGB (239, 155,  30);
	m_clrPlannerTodayCaption[3] = RGB (250, 224, 139);
	m_clrPlannerTodayBorder     = RGB (238, 147,  17);
	m_clrPlannerNcArea          = globalData.clrBtnFace;
	m_clrPlannerNcLine          = globalData.clrBtnShadow;
	m_clrPlannerNcText          = globalData.clrBtnText;

	m_clrCaptionBarText         = globalData.clrWindow;
	m_clrCaptionBarTextMessage  = m_clrCaptionBarText;

	m_penGridSeparator.DeleteObject ();
	m_penGridSeparator.CreatePen (PS_SOLID, 1, globalData.clrBarShadow);

	m_clrGridLeftOffset = globalData.clrBarFace;

	m_bToolTipParams = FALSE;
	CBCGPToolTipParams dummy;
	m_ToolTipParams = dummy;

	m_ActivateFlag.RemoveAll ();

	m_bPlannerBlack = FALSE;

	m_clrRibbonHyperlinkInactive = (COLORREF)-1;
	m_clrRibbonHyperlinkActive = (COLORREF)-1;
	m_clrRibbonStatusbarHyperlinkInactive = (COLORREF)-1;
	m_clrRibbonStatusbarHyperlinkActive = (COLORREF)-1;

	m_clrDlgBackground = (COLORREF)-1;
	m_brDlgBackground.DeleteObject ();

	m_clrDlgButtonsArea = (COLORREF)-1;
	m_brDlgButtonsArea.DeleteObject ();

	m_ctrlCalculatorBtn.CleanUp ();
	m_clrCalculatorBtnText[0] = globalData.clrBtnFace;
	m_clrCalculatorBtnText[1] = m_clrCalculatorBtnText[0];
	m_clrCalculatorBtnText[2] = m_clrCalculatorBtnText[1];

#ifndef BCGP_EXCLUDE_GRID_CTRL
	CBCGPGridColors gridcolors;
	m_GridColors = gridcolors;
#endif

	m_clrAppCaptionActiveStart = (COLORREF)-1;
	m_clrAppCaptionActiveFinish = (COLORREF)-1;

	m_bLoaded = FALSE;
}
//*****************************************************************************
void CBCGPVisualManager2007::OnUpdateSystemColors ()
{
	CleanUp ();

	CBCGPVisualManager2003::OnUpdateSystemColors ();

	if (globalData.bIsWindows9x)
	{
		return;
	}

	if (!globalData.bIsOSAlphaBlendingSupport ||
		globalData.IsHighContastMode () ||
		globalData.m_nBitsPerPixel <= 8)
	{
		return;
	}

	m_nMenuBorderSize = 1;

	HINSTANCE hinstResOld = NULL;

	if (m_hinstRes == NULL)
	{
		SetStyle (m_Style);
	}

	if (m_hinstRes != NULL)
	{
		hinstResOld = AfxGetResourceHandle ();
		AfxSetResourceHandle (m_hinstRes);
	}

	CBCGPTagManager tm;
	CBCGPTagManager::SetBaseColor (m_clrBase, m_clrTarget);

	if (!tm.LoadFromResource (GetStyleResourceID (m_Style), RT_BCG_STYLE_XML))
	{
#if !defined _AFXDLL || defined _BCGCBPRO_STATIC_
		TRACE(_T("\r\nImportant: to enable Office 2007 look in static link, you need:\r\n"));
		TRACE(_T("1. Open \"Resource Includes\" dialog and add resource files:\r\n"));
		TRACE(_T("<BCGCBPro-Path>\\styles\\BCGPStyle2007Luna.rc\r\n"));
		TRACE(_T("<BCGCBPro-Path>\\styles\\BCGPStyle2007Obsidian.rc\r\n"));
		TRACE(_T("<BCGCBPro-Path>\\styles\\BCGPStyle2007Silver.rc\r\n"));
		TRACE(_T("<BCGCBPro-Path>\\styles\\BCGPStyle2007Aqua.rc\r\n"));
		TRACE(_T("2. Add path to this folder to \"Additional Resource Include Directories\"\r\n"));
		TRACE(_T("<BCGCBPro-Path>\\styles\r\n\r\n"));
		ASSERT (FALSE);
#endif
		if (hinstResOld != NULL)
		{
			AfxSetResourceHandle (hinstResOld);
		}

		CBCGPTagManager::SetBaseColor ((COLORREF)-1, (COLORREF)-1);
		return;
	}

	m_nType = 20;

	CString strStyle;
	if (tm.ExcludeTag (_T("STYLE"), strStyle))
	{
		tm.SetBuffer (strStyle);
		m_bLoaded = ParseStyleXML (tm);
	}

	if (hinstResOld != NULL)
	{
		AfxSetResourceHandle (hinstResOld);
	}
	
	CBCGPTagManager::SetBaseColor ((COLORREF)-1, (COLORREF)-1);
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXML(CBCGPTagManager& tm)
{
	CString strItem;
	if (tm.ExcludeTag (_T("VERSION"), strItem))
	{
		if (!ParseStyleXMLVersion (strItem))
		{
			return FALSE;
		}
	}

#ifdef _DEBUG
	BOOL bToolBarImagesTrace = g_bToolBarImagesTrace;
	g_bToolBarImagesTrace = FALSE;
#endif

	if (tm.ExcludeTag (_T("GLOBALS"), strItem))
	{
		ParseStyleXMLGlobals (strItem);
	}

	if (tm.ExcludeTag (_T("MAINWND"), strItem))
	{
		ParseStyleXMLMainWnd (strItem);
	}

	if (tm.ExcludeTag (_T("MENU"), strItem))
	{
		ParseStyleXMLMenu (strItem);
	}

	if (tm.ExcludeTag (_T("BARS"), strItem))
	{
		ParseStyleXMLBars (strItem);
	}

	if (m_clrMenuRarelyUsed == CLR_DEFAULT)
	{
		m_clrMenuRarelyUsed = m_clrBarBkgnd;
	}

	m_brMenuRarelyUsed.DeleteObject ();
	m_brMenuRarelyUsed.CreateSolidBrush (m_clrMenuRarelyUsed);

	m_clrEditBorder            = globalData.clrWindow;
	m_clrEditBorderDisabled    = globalData.clrBtnShadow;
	m_clrEditBorderHighlighted = m_clrMenuItemBorder;
	m_clrEditSelection         = globalData.clrHilite;

	if (tm.ExcludeTag (_T("EDIT"), strItem))
	{
		ParseStyleXMLEdit (strItem);
	}

	m_clrComboBorder               = globalData.clrWindow;
	m_clrComboBorderDisabled       = globalData.clrBtnShadow;
	m_clrComboBorderHighlighted    = m_clrMenuItemBorder;
	m_clrComboBorderPressed        = m_clrComboBorderHighlighted;
	m_clrComboBtnBorder            = m_clrComboBorder;
	m_clrComboBtnBorderHighlighted = m_clrComboBorderHighlighted;
	m_clrComboBtnBorderPressed     = m_clrComboBorderHighlighted;
	m_clrComboSelection            = globalData.clrHilite;
	m_clrComboBtnStart             = m_clrToolBarGradientDark;
	m_clrComboBtnFinish            = m_clrToolBarGradientLight;
	m_clrComboBtnDisabledStart     = globalData.clrBtnFace;
	m_clrComboBtnDisabledFinish    = m_clrComboBtnDisabledStart;
	m_clrComboBtnHighlightedStart  = m_clrHighlightGradientDark;
	m_clrComboBtnHighlightedFinish = m_clrHighlightGradientLight;
	m_clrComboBtnPressedStart      = m_clrHighlightDnGradientDark;
	m_clrComboBtnPressedFinish     = m_clrHighlightDnGradientLight;

	if (tm.ExcludeTag (_T("COMBO"), strItem))
	{
		ParseStyleXMLCombo (strItem);
	}

#ifndef BCGP_EXCLUDE_TASK_PANE

	m_clrTaskPaneGradientDark       = m_clrBarGradientLight;
	m_clrTaskPaneGradientLight      = m_clrTaskPaneGradientDark;

	if (tm.ExcludeTag (_T("TASK"), strItem))
	{
		ParseStyleXMLTaskPane (strItem);
	}

#endif

	if (tm.ExcludeTag (_T("TABS"), strItem))
	{
		ParseStyleXMLTabs (strItem);
	}

	if (m_clrTab3DTextInactive == (COLORREF)-1)
	{
		m_clrTab3DTextInactive = m_clrMenuBarBtnText;
	}
	if (m_clrTab3DTextActive == (COLORREF)-1)
	{
		m_clrTab3DTextActive = m_clrMenuBarBtnTextHighlighted;
	}
	if (m_clrTab3DTextHighlighted == (COLORREF)-1)
	{
		m_clrTab3DTextHighlighted = m_clrTab3DTextInactive;
	}

	if (m_clrTabFlatTextInactive == (COLORREF)-1)
	{
		m_clrTabFlatTextInactive = m_clrMenuBarBtnTextHighlighted;
	}
	if (m_clrTabFlatTextActive == (COLORREF)-1)
	{
		m_clrTabFlatTextActive = m_clrMenuBarBtnTextHighlighted;
	}
	if (m_clrTabFlatTextHighlighted == (COLORREF)-1)
	{
		m_clrTabFlatTextHighlighted = m_clrTabFlatTextActive;
	}

	if (tm.ExcludeTag (_T("HEADER"), strItem))
	{
		ParseStyleXMLHeader (strItem);
	}

	m_clrRibbonBarBtnText                  = m_clrToolBarBtnText;
	m_clrRibbonBarBtnTextHighlighted       = m_clrToolBarBtnTextHighlighted;
	m_clrRibbonBarBtnTextDisabled          = m_clrToolBarBtnTextDisabled;

	m_clrRibbonEdit                        = globalData.clrBarLight;
	m_clrRibbonEditHighlighted             = globalData.clrWindow;
	m_clrRibbonEditPressed                 = m_clrRibbonEditHighlighted;
	m_clrRibbonEditDisabled                = globalData.clrBtnFace;
	m_clrRibbonEditBorder                  = m_clrEditBorder;
	m_clrRibbonEditBorderDisabled          = m_clrEditBorderDisabled;
	m_clrRibbonEditBorderHighlighted       = m_clrEditBorderHighlighted;
	m_clrRibbonEditBorderPressed           = m_clrRibbonEditBorderHighlighted;
	m_clrRibbonEditSelection               = m_clrEditSelection;

	m_clrRibbonComboBtnBorder              = m_clrComboBtnBorder;
	m_clrRibbonComboBtnBorderHighlighted   = m_clrComboBtnBorderHighlighted;
	m_clrRibbonComboBtnBorderPressed       = m_clrComboBtnBorderPressed;
	m_clrRibbonComboBtnStart               = m_clrComboBtnStart;
	m_clrRibbonComboBtnFinish              = m_clrComboBtnFinish;
	m_clrRibbonComboBtnDisabledStart       = m_clrComboBtnDisabledStart;
	m_clrRibbonComboBtnDisabledFinish      = m_clrComboBtnDisabledFinish;
	m_clrRibbonComboBtnHighlightedStart    = m_clrComboBtnHighlightedStart;
	m_clrRibbonComboBtnHighlightedFinish   = m_clrComboBtnHighlightedFinish;
	m_clrRibbonComboBtnPressedStart        = m_clrComboBtnPressedStart;
	m_clrRibbonComboBtnPressedFinish       = m_clrComboBtnPressedFinish;

	m_clrRibbonCategoryText                = m_clrMenuBarBtnText;
	m_clrRibbonCategoryTextHighlighted     = m_clrMenuBarBtnTextHighlighted;
	m_clrRibbonCategoryTextDisabled        = m_clrMenuBarBtnTextDisabled;
	m_clrRibbonPanelText                   = m_clrToolBarBtnText;
	m_clrRibbonPanelTextHighlighted        = m_clrToolBarBtnTextHighlighted;
	m_clrRibbonPanelCaptionText            = m_clrRibbonPanelText;
	m_clrRibbonPanelCaptionTextHighlighted = m_clrRibbonPanelTextHighlighted;

	if (tm.ExcludeTag (_T("RIBBON"), strItem))
	{
		ParseStyleXMLRibbon (strItem);
	}

	if (m_clrRibbonMainPanelBkgnd == CLR_DEFAULT)
	{
		m_clrRibbonMainPanelBkgnd = m_clrBarBkgnd;
	}

	m_brRibbonMainPanelBkgnd.DeleteObject ();
	m_brRibbonMainPanelBkgnd.CreateSolidBrush (m_clrRibbonMainPanelBkgnd);

	if (tm.ExcludeTag (_T("PLANNER"), strItem))
	{
		ParseStyleXMLPlanner (strItem);
	}

	m_clrGridLeftOffset                       = globalData.clrBarFace;
	m_clrGridGroupLine			              = globalData.clrBarShadow;
	m_clrGridHeaderNormalStart                = m_clrHeaderNormalStart;
	m_clrGridHeaderNormalFinish               = m_clrHeaderNormalFinish;
	m_clrGridHeaderNormalBorder               = m_clrHeaderNormalBorder;
	m_clrGridHeaderPressedStart               = m_clrHeaderPressedStart;
	m_clrGridHeaderPressedFinish              = m_clrHeaderPressedFinish;
	m_clrGridHeaderPressedBorder              = m_clrHeaderPressedBorder;

	m_clrGridHeaderAllNormalBackStart         = CBCGPDrawManager::MixColors (m_clrGridHeaderNormalStart, m_clrGridHeaderNormalBorder, 0.50);
	m_clrGridHeaderAllNormalBackFinish        = m_clrGridHeaderAllNormalBackStart;
	m_clrGridHeaderAllPressedBackStart        = m_clrHeaderPressedBorder;
	m_clrGridHeaderAllPressedBackFinish       = m_clrGridHeaderAllPressedBackStart;
	m_clrGridHeaderAllNormalBorderHighlighted = m_clrGridHeaderNormalStart;
	m_clrGridHeaderAllNormalBorderShadow      = m_clrGridHeaderNormalFinish;
	m_clrGridHeaderAllPressedBorderHighlighted= m_clrGridHeaderAllNormalBorderHighlighted;
	m_clrGridHeaderAllPressedBorderShadow     = m_clrGridHeaderAllNormalBorderShadow;
	m_clrGridHeaderAllNormalSignStart         = m_clrGridHeaderNormalStart;
	m_clrGridHeaderAllNormalSignFinish        = m_clrGridHeaderNormalFinish;
	m_clrGridHeaderAllPressedSignStart        = m_clrGridHeaderAllNormalSignStart;
	m_clrGridHeaderAllPressedSignFinish       = m_clrGridHeaderAllNormalSignFinish;

	if (tm.ExcludeTag (_T("GRID"), strItem))
	{
		ParseStyleXMLGrid (strItem);
	}

	m_clrOutlookCaptionTextNormal   = m_clrCaptionBarText;
	m_clrOutlookPageTextNormal      = m_clrOutlookCaptionTextNormal;
	m_clrOutlookPageTextHighlighted = m_clrOutlookPageTextNormal;
	m_clrOutlookPageTextPressed     = m_clrOutlookPageTextNormal;

	if (tm.ExcludeTag (_T("OUTLOOK"), strItem))
	{
		ParseStyleXMLOutlook (strItem);
	}

	m_clrPopupGradientLight = m_clrBarGradientLight;
	m_clrPopupGradientDark  = m_clrBarGradientDark;

	if (tm.ExcludeTag (_T("POPUP"), strItem))
	{
		ParseStyleXMLPopup (strItem);
	}

	if (tm.ExcludeTag (_T("SLIDER"), strItem))
	{
		ParseStyleXMLSlider (strItem);
	}

	m_clrCalculatorBtnText[0] = m_clrRibbonPanelText;
	m_clrCalculatorBtnText[1] = m_clrCalculatorBtnText[0];
	m_clrCalculatorBtnText[2] = m_clrCalculatorBtnText[1];

	m_clrDockingBarScrollButton       = globalData.clrInactiveCaption;
	m_clrDockingBarScrollButtonBorder = m_clrToolBarBottomLine;

	if (tm.ExcludeTag (_T("CALCULATOR"), strItem))
	{
		ParseStyleXMLCalculator (strItem);
	}

#ifdef _DEBUG
	g_bToolBarImagesTrace = bToolBarImagesTrace;
#endif

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLVersion(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

	BOOL bLoaded = FALSE;
	int nVersion = 0;

	CBCGPTagManager tmItem (strItem);

	tmItem.ReadInt (_T("NUMBER"), nVersion);

	if (nVersion == 2007)
	{
		tmItem.ReadInt (_T("TYPE"), m_nType);

		if (m_nType < 10)
		{
			m_nType *= 10;
		}

		bLoaded = TRUE;
	}

	if (bLoaded)
	{
		CString strPrefix;
		if (tmItem.ExcludeTag (_T("ID_PREFIX"), strPrefix))
		{
			strPrefix.TrimLeft ();
			strPrefix.TrimRight ();
			m_strStylePrefix = strPrefix;
		}
	}

	return bLoaded;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLGlobals(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

	CBCGPTagManager tmItem (strItem);

	tmItem.ReadColor (_T("BarText"), globalData.clrBarText);

	if (tmItem.ReadColor (_T("BarFace"), globalData.clrBarFace))
	{
		globalData.brBarFace.DeleteObject ();
		globalData.brBarFace.CreateSolidBrush (globalData.clrBarFace);
		m_clrMenuShadowBase = globalData.clrBarFace;
	}
	if (tmItem.ReadColor (_T("ActiveCaption"), globalData.clrActiveCaption))
	{
		globalData.clrInactiveCaption     = globalData.clrActiveCaption;
		globalData.brActiveCaption.DeleteObject ();
		globalData.brActiveCaption.CreateSolidBrush (globalData.clrActiveCaption);
	}
	if (tmItem.ReadColor (_T("CaptionText"), globalData.clrCaptionText))
	{
		globalData.clrInactiveCaptionText = globalData.clrCaptionText;
	}

	tmItem.ReadColor (_T("InactiveCaption"), globalData.clrInactiveCaption);
	globalData.brInactiveCaption.DeleteObject ();
	globalData.brInactiveCaption.CreateSolidBrush (globalData.clrInactiveCaption);
	tmItem.ReadColor (_T("InactiveCaptionText"), globalData.clrInactiveCaptionText);

	tmItem.ReadColor (_T("BarShadow"), globalData.clrBarShadow);
	tmItem.ReadColor (_T("BarDkShadow"), globalData.clrBarDkShadow);
	tmItem.ReadColor (_T("BarLight"), globalData.clrBarLight);
	tmItem.ReadColor (_T("BarHilite"), globalData.clrBarHilite);
	
	COLORREF clrFloatToolBarBorder;
	tmItem.ReadColor (_T("FloatToolBarBorder"), clrFloatToolBarBorder);
	m_brFloatToolBarBorder.DeleteObject ();
	m_brFloatToolBarBorder.CreateSolidBrush (clrFloatToolBarBorder);

	tmItem.ReadColor (_T("HighlightGradientDark"), m_clrHighlightGradientDark);
	tmItem.ReadColor (_T("HighlightGradientLight"), m_clrHighlightGradientLight);

	m_clrHighlightDnGradientDark = m_clrHighlightGradientLight;
	m_clrHighlightDnGradientLight = m_clrHighlightGradientDark;
	tmItem.ReadColor (_T("HighlightDnGradientDark"), m_clrHighlightDnGradientDark);
	tmItem.ReadColor (_T("HighlightDnGradientLight"), m_clrHighlightDnGradientLight);

	m_clrHighlightCheckedGradientDark = m_clrHighlightDnGradientLight;
	m_clrHighlightCheckedGradientLight = m_clrHighlightDnGradientDark;
	tmItem.ReadColor (_T("HighlightCheckedGradientDark"), m_clrHighlightCheckedGradientDark);
	tmItem.ReadColor (_T("HighlightCheckedGradientLight"), m_clrHighlightCheckedGradientLight);

	tmItem.ReadColor (_T("PressedButtonBorder"), m_clrPressedButtonBorder);

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

	tmItem.ReadColor (_T("MenuShadowColor"), m_clrMenuShadowBase);

	// dialog background
	m_clrDlgBackground = globalData.clrBarLight;
	tmItem.ReadColor (_T("DlgBackColor"), m_clrDlgBackground);
	m_brDlgBackground.DeleteObject ();
	m_brDlgBackground.CreateSolidBrush (m_clrDlgBackground);

	m_clrDlgButtonsArea = globalData.clrBarShadow;
	tmItem.ReadColor (_T("DlgButtonsAreaColor"), m_clrDlgButtonsArea);
	m_brDlgButtonsArea.DeleteObject ();
	m_brDlgButtonsArea.CreateSolidBrush (m_clrDlgButtonsArea);

	// ToolTipParams
	m_bToolTipParams = tmItem.ReadToolTipParams (_T("TOOLTIP"), m_ToolTipParams);

	m_brCtrlBackground.DeleteObject();
	m_brCtrlBackground.CreateSolidBrush(globalData.clrBarLight);

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLMainWnd(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

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

		tmCaption.ReadColor (_T("ActiveStart"), m_clrAppCaptionActiveStart);
		tmCaption.ReadColor (_T("ActiveFinish"), m_clrAppCaptionActiveFinish);
		tmCaption.ReadColor (_T("InactiveStart"), m_clrAppCaptionInactiveStart);
		tmCaption.ReadColor (_T("InactiveFinish"), m_clrAppCaptionInactiveFinish);
		tmCaption.ReadColor (_T("ActiveText"), m_clrAppCaptionActiveText);
		tmCaption.ReadColor (_T("InactiveText"), m_clrAppCaptionInactiveText);
		tmCaption.ReadColor (_T("ActiveTitleText"), m_clrAppCaptionActiveTitleText);
		tmCaption.ReadColor (_T("InactiveTitleText"), m_clrAppCaptionInactiveTitleText);

		tmCaption.ReadBool (_T("TextCenter"), m_bNcTextCenter);

		tmCaption.ReadControlRenderer (_T("BORDER"), m_ctrlMainBorderCaption, MakeResourceID(_T("MAINBRD_CAPTION")));

		if (!tmCaption.ReadControlRenderer (_T("APP_BORDER"), m_ctrlAppBorderCaption, MakeResourceID(_T("MAINBRD_APPCAPTION"))))
		{
			m_ctrlMainBorderCaption.CopyTo(m_ctrlAppBorderCaption);
		}

		m_szNcBtnSize[0] = CSize (::GetSystemMetrics (SM_CXSIZE),
								  ::GetSystemMetrics (SM_CYSIZE));
		m_szNcBtnSize[1] = CSize (::GetSystemMetrics (SM_CXSMSIZE),
								  ::GetSystemMetrics (SM_CYSMSIZE));

		// buttons
		CString strButtons;
		if (tmCaption.ExcludeTag (_T("BUTTONS"), strButtons))
		{
			CBCGPTagManager tmButtons (strButtons);

			for (int i = 0; i < 2; i++)
			{
				CString str;
				CString suffix;
				if (i == 1)
				{
					suffix = _T("_S");
				}
				if (tmButtons.ExcludeTag (i == 0 ? _T("NORMAL") : _T("SMALL"), str))
				{
					CBCGPTagManager tmBtn (str);

					tmBtn.ReadSize (_T("ConstSize"), m_szNcBtnSize[i]);

					CSize sizeIcon (0, 0);
					if (tmBtn.ReadSize (_T("IconSize"), sizeIcon))
					{
						m_SysBtnClose[i].Clear ();
						m_SysBtnClose[i].SetPreMultiplyAutoCheck (TRUE);
						m_SysBtnClose[i].SetImageSize (sizeIcon);
						m_SysBtnClose[i].LoadStr (MakeResourceID(_T("SYS_BTN_CLOSE") + suffix));
						
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
						m_SysBtnBack[i], MakeResourceID(_T("SYS_BTN_BACK")));
					CBCGPTagManager::ParseControlRenderer (tmBtn.GetBuffer (), 
						m_SysBtnBackC[i], MakeResourceID(_T("SYS_BTN_CLOSE_BACK")));
				}
			}
		}
	}

	// border
	tmItem.ReadControlRenderer (_T("BORDER"), m_ctrlMainBorder, MakeResourceID(_T("MAINBRD")));
	tmItem.ReadControlRenderer (_T("BORDER_MDICHILD"), m_ctrlMDIChildBorder, MakeResourceID(_T("MDICHILDBRD")));
	tmItem.ReadControlRenderer (_T("BORDER_DIALOG"), m_ctrlDialogBorder, MakeResourceID(_T("DIALOGBRD")));

	if (tmItem.ReadColor (_T("MainClientArea"), m_clrMainClientArea))
	{
		m_brMainClientArea.DeleteObject ();
		m_brMainClientArea.CreateSolidBrush (m_clrMainClientArea);
	}

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLMenu(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

	CBCGPTagManager tmItem (strItem);

	if (tmItem.ReadColor (_T("Light"), m_clrMenuLight))
	{
		m_brMenuLight.DeleteObject ();
		m_brMenuLight.CreateSolidBrush (m_clrMenuLight);
	}

	m_clrMenuRarelyUsed = CLR_DEFAULT;
	tmItem.ReadColor (_T("Rarely"), m_clrMenuRarelyUsed);

	tmItem.ReadColor (_T("Border"), m_clrMenuBorder);

	if (tmItem.ReadColor (_T("Separator1"), m_clrSeparator1))
	{
		m_penSeparator.DeleteObject ();
		m_penSeparator.CreatePen (PS_SOLID, 1, m_clrSeparator1);
	}

	if (tmItem.ReadColor (_T("Separator2"), m_clrSeparator2))
	{
		m_penSeparator2.DeleteObject ();
		m_penSeparator2.CreatePen (PS_SOLID, 1, m_clrSeparator2);
	}

	COLORREF clrGroupBack = (COLORREF)-1;
	if (tmItem.ReadColor (_T("GroupBackground"), clrGroupBack))
	{
		m_brGroupBackground.DeleteObject ();
		m_brGroupBackground.CreateSolidBrush (clrGroupBack);
	}

	tmItem.ReadColor (_T("GroupText"), m_clrGroupText);

	if (tmItem.ReadColor (_T("ItemBorder"), m_clrMenuItemBorder))
	{
		m_penMenuItemBorder.DeleteObject ();
		m_penMenuItemBorder.CreatePen (PS_SOLID, 1, m_clrMenuItemBorder);
	}

	tmItem.ReadInt (_T("BorderSize"), m_nMenuBorderSize);

	tmItem.ReadControlRenderer (_T("ItemBack"), m_ctrlMenuItemBack, MakeResourceID(_T("MENU_ITEM_BACK")));
	tmItem.ReadToolBarImages (_T("ItemCheck"), m_MenuItemMarkerC, MakeResourceID(_T("MENU_ITEM_MARKER_C")));
	tmItem.ReadToolBarImages (_T("ItemRadio"), m_MenuItemMarkerR, MakeResourceID(_T("MENU_ITEM_MARKER_R")));
	tmItem.ReadToolBarImages (_T("ItemPin"), m_MenuItemPin, MakeResourceID(_T("MENU_ITEM_PIN")));
	tmItem.ReadControlRenderer (_T("ItemShowAll"), m_ctrlMenuItemShowAll, MakeResourceID(_T("MENU_ITEM_SHOWALL")));
	tmItem.ReadControlRenderer (_T("Highlighted"), m_ctrlMenuHighlighted[0], MakeResourceID(_T("MENU_BTN")));
	tmItem.ReadControlRenderer (_T("HighlightedDisabled"), m_ctrlMenuHighlighted[1], MakeResourceID(_T("MENU_BTN_DISABLED")));
	tmItem.ReadControlRenderer (_T("ButtonBorder"), m_ctrlMenuButtonBorder, MakeResourceID(_T("MENU_BTN_V_SEPARATOR")));
	tmItem.ReadControlRenderer (_T("ScrollBtn_T"), m_ctrlMenuScrollBtn[0], MakeResourceID(_T("MENU_BTN_SCROLL_T")));
	tmItem.ReadControlRenderer (_T("ScrollBtn_B"), m_ctrlMenuScrollBtn[1], MakeResourceID(_T("MENU_BTN_SCROLL_B")));

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

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLBars(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

	CBCGPTagManager tmItem (strItem);

	CString strBar;
	if (tmItem.ExcludeTag (_T("DEFAULT"), strBar))
	{
		CBCGPTagManager tmBar (strBar);

		if (tmBar.ReadColor (_T("Bkgnd"), m_clrBarBkgnd))
		{
			m_brBarBkgnd.DeleteObject ();
			m_brBarBkgnd.CreateSolidBrush  (m_clrBarBkgnd);

			m_brMenuConnect.DeleteObject();
			m_brMenuConnect.CreateSolidBrush(m_clrBarBkgnd);
		}

		tmBar.ReadColor (_T("GradientLight"), m_clrBarGradientLight);
		m_clrBarGradientDark = m_clrBarGradientLight;
		tmBar.ReadColor (_T("GradientDark"), m_clrBarGradientDark);
	}

	if (tmItem.ExcludeTag (_T("TOOLBAR"), strBar))
	{
		CBCGPTagManager tmBar (strBar);

		m_clrToolBarGradientLight = m_clrBarGradientLight;
		m_clrToolBarGradientDark  = m_clrBarGradientDark;

		m_clrToolbarDisabled = CBCGPDrawManager::SmartMixColors (
			m_clrToolBarGradientDark, m_clrToolBarGradientLight);

		tmBar.ReadColor (_T("GradientLight"), m_clrToolBarGradientLight);
		tmBar.ReadColor (_T("GradientDark"), m_clrToolBarGradientDark);

		m_clrToolBarGradientVertLight = m_clrToolBarGradientLight;
		m_clrToolBarGradientVertDark  = m_clrToolBarGradientDark;

		tmBar.ReadColor (_T("GradientVertLight"), m_clrToolBarGradientVertLight);
		tmBar.ReadColor (_T("GradientVertDark"), m_clrToolBarGradientVertDark);

		tmBar.ReadColor (_T("CustomizeButtonGradientLight"), m_clrCustomizeButtonGradientLight);
		tmBar.ReadColor (_T("CustomizeButtonGradientDark"), m_clrCustomizeButtonGradientDark);

		tmBar.ReadToolBarImages (_T("GRIPPER"), m_ToolBarGripper, MakeResourceID(_T("GRIPPER")));
		tmBar.ReadToolBarImages (_T("TEAR"), m_ToolBarTear, MakeResourceID(_T("TEAR")));

		tmBar.ReadControlRenderer (_T("BUTTON"), m_ctrlToolBarBtn, MakeResourceID(_T("TB_BTN")));
		//tmBar.ReadControlRenderer (_T("BORDER"), m_ctrlToolBarBorder, MakeResourceID(_T("TB_BRD")));

		m_clrToolBarBtnText            = globalData.clrBarText;
		m_clrToolBarBtnTextHighlighted = m_clrToolBarBtnText;
		tmBar.ReadColor (_T("TextNormal"), m_clrToolBarBtnText);
		tmBar.ReadColor (_T("TextHighlighted"), m_clrToolBarBtnTextHighlighted);
		tmBar.ReadColor (_T("TextDisabled"), m_clrToolBarBtnTextDisabled);

		if (tmBar.ReadColor (_T("BottomLineColor"), m_clrToolBarBottomLine))
		{
			m_penBottomLine.DeleteObject ();
			m_penBottomLine.CreatePen (PS_SOLID, 1, m_clrToolBarBottomLine);
		}

		m_penSeparatorDark.DeleteObject ();
		m_penSeparatorDark.CreatePen (PS_SOLID, 1, 
			CBCGPDrawManager::PixelAlpha (m_clrToolBarBottomLine, RGB (255, 255, 255), 95));

		m_penSeparatorLight.DeleteObject ();
		m_penSeparatorLight.CreatePen (PS_SOLID, 1, RGB (255, 255, 255));
	}

	if (tmItem.ExcludeTag (_T("MENUBAR"), strBar))
	{
		CBCGPTagManager tmBar (strBar);

		m_clrMenuBarGradientLight = m_clrToolBarGradientLight;
		m_clrMenuBarGradientDark  = m_clrToolBarGradientDark;

		tmBar.ReadColor (_T("GradientLight"), m_clrMenuBarGradientLight);
		tmBar.ReadColor (_T("GradientDark"), m_clrMenuBarGradientDark);

		m_clrMenuBarGradientVertLight = m_clrMenuBarGradientLight;
		m_clrMenuBarGradientVertDark  = m_clrMenuBarGradientDark;

		tmBar.ReadColor (_T("GradientVertLight"), m_clrMenuBarGradientVertLight);
		tmBar.ReadColor (_T("GradientVertDark"), m_clrMenuBarGradientVertDark);

		m_clrMenuBarBtnText            = m_clrToolBarBtnText;
		m_clrMenuBarBtnTextHighlighted = m_clrToolBarBtnTextHighlighted;
		m_clrMenuBarBtnTextDisabled    = m_clrToolBarBtnTextDisabled;
		tmBar.ReadColor (_T("TextNormal"), m_clrMenuBarBtnText);
		tmBar.ReadColor (_T("TextHighlighted"), m_clrMenuBarBtnTextHighlighted);
		tmBar.ReadColor (_T("TextDisabled"), m_clrMenuBarBtnTextDisabled);

		tmBar.ReadControlRenderer (_T("BUTTON"), m_ctrlMenuBarBtn, MakeResourceID(_T("MB_BTN")));
	}

	if (tmItem.ExcludeTag (_T("POPUPBAR"), strBar))
	{
		CBCGPTagManager tmBar (strBar);
		tmBar.ReadControlRenderer (_T("BORDER"), m_ctrlPopupBorder, MakeResourceID(_T("POPMENU_BRD")));

		CString strResize;
		if (tmBar.ExcludeTag (_T("RESIZEBAR"), strResize))
		{
			CBCGPTagManager tmResize (strResize);
			tmResize.ReadControlRenderer (_T("BACK"), m_ctrlPopupResizeBar, MakeResourceID(_T("POPMENU_RSB")));
			tmResize.ReadToolBarImages (_T("ICON_HV"), m_PopupResizeBar_HV, MakeResourceID(_T("POPMENU_RSB_ICON_HV")));
			tmResize.ReadToolBarImages (_T("ICON_HVT"), m_PopupResizeBar_HVT, MakeResourceID(_T("POPMENU_RSB_ICON_HVT")));
			tmResize.ReadToolBarImages (_T("ICON_V"), m_PopupResizeBar_V, MakeResourceID(_T("POPMENU_RSB_ICON_V")));
		}
	}

	if (tmItem.ExcludeTag (_T("STATUSBAR"), strBar))
	{
		CBCGPTagManager tmBar (strBar);

		tmBar.ReadControlRenderer (_T("BACK"), m_ctrlStatusBarBack, MakeResourceID(_T("SB_BACK")));
		tmBar.ReadControlRenderer (_T("BACK_EXT"), m_ctrlStatusBarBack_Ext, MakeResourceID(_T("SB_BACK_EXT")));

		tmBar.ReadToolBarImages (_T("PANEBORDER"), m_StatusBarPaneBorder, MakeResourceID(_T("SB_PANEBRD")));
		tmBar.ReadToolBarImages (_T("SIZEBOX"), m_StatusBarSizeBox, MakeResourceID(_T("SB_SIZEBOX")));

		m_clrStatusBarText         = m_clrMenuBarBtnText;
		m_clrStatusBarTextDisabled = m_clrMenuBarBtnTextDisabled;
		m_clrExtenedStatusBarTextDisabled = m_clrMenuBarBtnTextDisabled;

		tmBar.ReadColor (_T("TextNormal"), m_clrStatusBarText);
		tmBar.ReadColor (_T("TextDisabled"), m_clrStatusBarTextDisabled);
		tmBar.ReadColor (_T("TextExtendedDisabled"), m_clrExtenedStatusBarTextDisabled);
	}
	
	if (tmItem.ExcludeTag (_T("CAPTIONBAR"), strBar))
	{
		CBCGPTagManager tmBar (strBar);

		tmBar.ReadColor (_T("GradientLight"), m_clrCaptionBarGradientLight);
		tmBar.ReadColor (_T("GradientDark"), m_clrCaptionBarGradientDark);
		tmBar.ReadColor (_T("TextNormal"), m_clrCaptionBarText);
		m_clrCaptionBarTextMessage = m_clrCaptionBarText;
		tmBar.ReadColor (_T("TextMessage"), m_clrCaptionBarTextMessage);
	}

	if (tmItem.ExcludeTag (_T("SCROLLBAR"), strBar))
	{
		CBCGPTagManager tmBar (strBar);

		LPCTSTR szSBName [] = {_T("HORZ"), _T("VERT")};
		LPCTSTR szSBNameRes [] = {_T("H"), _T("V")};

		CString strSB;
		for (int i = 0; i < 2; i++)
		{
			if (tmBar.ExcludeTag (szSBName[i], strSB))
			{
				CBCGPTagManager tmSB (strSB);

				CString strName (_T("SB_"));
				strName += szSBNameRes[i];

				tmSB.ReadControlRenderer (_T("BACK_1"), m_ctrlScrollBar_Back[i][0], MakeResourceID(strName + _T("_BACK_1")));
				tmSB.ReadControlRenderer (_T("ITEM_1"), m_ctrlScrollBar_Item[i][0], MakeResourceID(strName + _T("_ITEM_1")));
				tmSB.ReadControlRenderer (_T("THUMB_BACK_1"), m_ctrlScrollBar_ThumbBack[i][0], MakeResourceID(strName + _T("_THUMB_BACK_1")));
				tmSB.ReadControlRenderer (_T("THUMB_ICON_1"), m_ctrlScrollBar_ThumbIcon[i][0], MakeResourceID(strName + _T("_THUMB_ICON_1")));

				tmSB.ReadControlRenderer (_T("BACK_2"), m_ctrlScrollBar_Back[i][1], MakeResourceID(strName + _T("_BACK_2")));
				tmSB.ReadControlRenderer (_T("ITEM_2"), m_ctrlScrollBar_Item[i][1], MakeResourceID(strName + _T("_ITEM_2")));
				tmSB.ReadControlRenderer (_T("THUMB_BACK_2"), m_ctrlScrollBar_ThumbBack[i][1], MakeResourceID(strName + _T("_THUMB_BACK_2")));
				tmSB.ReadControlRenderer (_T("THUMB_ICON_2"), m_ctrlScrollBar_ThumbIcon[i][1], MakeResourceID(strName + _T("_THUMB_ICON_2")));
			}
		}
	}

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLEdit(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

	CBCGPTagManager tmItem (strItem);

	tmItem.ReadColor (_T("BorderNormal"), m_clrEditBorder);
	tmItem.ReadColor (_T("BorderHighlighted"), m_clrEditBorderHighlighted);
	tmItem.ReadColor (_T("BorderDisabled"), m_clrEditBorderDisabled);
	tmItem.ReadColor (_T("Selection"), m_clrEditSelection);

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLCombo(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

	CBCGPTagManager tmItem (strItem);

	tmItem.ReadColor (_T("BorderNormal"), m_clrComboBorder);
	tmItem.ReadColor (_T("BorderHighlighted"), m_clrComboBorderHighlighted);
	tmItem.ReadColor (_T("BorderDisabled"), m_clrComboBorderDisabled);

	m_clrComboBorderPressed = m_clrComboBorderHighlighted;
	tmItem.ReadColor (_T("BorderPressed"), m_clrComboBorderPressed);

	tmItem.ReadColor (_T("Selection"), m_clrComboSelection);

	CString strButton;
	if (tmItem.ExcludeTag (_T("BUTTON"), strButton))
	{
		CBCGPTagManager tmButton (strButton);

		tmButton.ReadColor (_T("GradientStartNormal"), m_clrComboBtnStart);
		tmButton.ReadColor (_T("GradientFinishNormal"), m_clrComboBtnFinish);
		tmButton.ReadColor (_T("BtnBorderNormal"), m_clrComboBtnBorder);

		if (!tmButton.ReadControlRenderer (_T("IMAGE"), m_ctrlComboBoxBtn, MakeResourceID(_T("COMBOBOX_BTN"))))
		{
			tmButton.ReadColor (_T("GradientStartHighlighted"), m_clrComboBtnHighlightedStart);
			tmButton.ReadColor (_T("GradientFinishHighlighted"), m_clrComboBtnHighlightedFinish);
			tmButton.ReadColor (_T("GradientStartDisabled"), m_clrComboBtnDisabledStart);
			tmButton.ReadColor (_T("GradientFinishDisabled"), m_clrComboBtnDisabledFinish);
			tmButton.ReadColor (_T("GradientStartPressed"), m_clrComboBtnPressedStart);
			tmButton.ReadColor (_T("GradientFinishPressed"), m_clrComboBtnPressedFinish);

			tmButton.ReadColor (_T("BtnBorderHighlighted"), m_clrComboBtnBorderHighlighted);
			tmButton.ReadColor (_T("BtnBorderDisabled"), m_clrComboBtnBorderDisabled);

			m_clrComboBtnBorderPressed = m_clrComboBtnBorderHighlighted;
			tmButton.ReadColor (_T("BtnBorderPressed"), m_clrComboBtnBorderPressed);
		}
	}

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLTaskPane(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

#ifndef BCGP_EXCLUDE_TASK_PANE

	CBCGPTagManager tmItem (strItem);

	tmItem.ReadColor (_T("GradientDark"), m_clrTaskPaneGradientDark);
	tmItem.ReadColor (_T("GradientLight"), m_clrTaskPaneGradientLight);

	CString strGroup;
	if (tmItem.ExcludeTag (_T("GROUP"), strGroup))
	{
		CBCGPTagManager tmGroup (strGroup);

		CString strState;
		if (tmGroup.ExcludeTag (_T("NORMAL"), strState))
		{
			CBCGPTagManager tmState (strState);

			CString str;

			if (tmState.ExcludeTag (_T("CAPTION"), str))
			{
				CBCGPTagManager tmCaption (str);

				tmCaption.ReadColor (_T("DarkNormal"), m_clrTaskPaneGroupCaptionDark);
				tmCaption.ReadColor (_T("LightNormal"), m_clrTaskPaneGroupCaptionLight);
				tmCaption.ReadColor (_T("DarkHighlighted"), m_clrTaskPaneGroupCaptionHighDark);
				tmCaption.ReadColor (_T("LightHighlighted"), m_clrTaskPaneGroupCaptionHighLight);
				tmCaption.ReadColor (_T("TextNormal"), m_clrTaskPaneGroupCaptionText);
				tmCaption.ReadColor (_T("TextHighlighted"), m_clrTaskPaneGroupCaptionTextHigh);
			}

			if (tmState.ExcludeTag (_T("AREA"), str))
			{
				CBCGPTagManager tmArea (str);

				tmArea.ReadColor (_T("DarkNormal"), m_clrTaskPaneGroupAreaDark);
				tmArea.ReadColor (_T("LightNormal"), m_clrTaskPaneGroupAreaLight);
			}
		}

		if (tmGroup.ExcludeTag (_T("SPECIAL"), strState))
		{
			CBCGPTagManager tmState (strState);

			CString str;
			if (tmState.ExcludeTag (_T("CAPTION"), str))
			{
				CBCGPTagManager tmCaption (str);

				tmCaption.ReadColor (_T("DarkNormal"), m_clrTaskPaneGroupCaptionSpecDark);
				tmCaption.ReadColor (_T("LightNormal"), m_clrTaskPaneGroupCaptionSpecLight);
				tmCaption.ReadColor (_T("DarkHighlighted"), m_clrTaskPaneGroupCaptionHighSpecDark);
				tmCaption.ReadColor (_T("LightHighlighted"), m_clrTaskPaneGroupCaptionHighSpecLight);
				tmCaption.ReadColor (_T("TextNormal"), m_clrTaskPaneGroupCaptionTextSpec);
				tmCaption.ReadColor (_T("TextHighlighted"), m_clrTaskPaneGroupCaptionTextHighSpec);
			}

			if (tmState.ExcludeTag (_T("AREA"), str))
			{
				CBCGPTagManager tmArea (str);

				tmArea.ReadColor (_T("DarkNormal"), m_clrTaskPaneGroupAreaSpecDark);
				tmArea.ReadColor (_T("LightNormal"), m_clrTaskPaneGroupAreaSpecLight);
			}
		}

		if (tmGroup.ReadColor (_T("BORDER"), m_clrTaskPaneGroupBorder))
		{
			m_penTaskPaneGroupBorder.DeleteObject ();
			m_penTaskPaneGroupBorder.CreatePen (PS_SOLID, 1, m_clrTaskPaneGroupBorder);
		}
	}

	tmItem.ReadControlRenderer (_T("SCROLL_BUTTON"), m_ctrlTaskScrollBtn, MakeResourceID(_T("TASKPANE_SCROLL_BTN")));

#endif

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLTabs(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

	CBCGPTagManager tmItem (strItem);

	tmItem.ReadColor (_T("TextColorActive"), m_clrTabTextActive);
	tmItem.ReadColor (_T("TextColorInactive"), m_clrTabTextInactive);
	tmItem.ReadColor (_T("TextColorHighlighted"), m_clrTabTextHighlighted);

	CString strTab;
	if (tmItem.ExcludeTag (_T("3D"), strTab))
	{
		CBCGPTagManager tmTab (strTab);

		CString strBtn;
		if (tmTab.ExcludeTag (_T("BUTTON"), strBtn))
		{
			CBCGPControlRendererParams params (MakeResourceID(_T("TAB_3D")), CRect (0, 0, 0, 0), CRect (0, 0, 0, 0));
			if (CBCGPTagManager::ParseControlRendererParams (strBtn, params))
			{
				m_ctrlTab3D[0].Create (params);
				m_ctrlTab3D[1].Create (params, TRUE);
			}
		}

		tmTab.ReadColor (_T("Face"), m_clrTab3DFace);
		tmTab.ReadColor (_T("Black"), m_clrTab3DBlack);
		tmTab.ReadColor (_T("Dark"), m_clrTab3DDark);
		tmTab.ReadColor (_T("DarkShadow"), m_clrTab3DDarkShadow);
		tmTab.ReadColor (_T("Light"), m_clrTab3DLight);
		tmTab.ReadColor (_T("Highlight"), m_clrTab3DHighlight);
		tmTab.ReadColor (_T("TextActive"), m_clrTab3DTextActive);
		tmTab.ReadColor (_T("TextInactive"), m_clrTab3DTextInactive);
		tmTab.ReadColor (_T("TextHighlighted"), m_clrTab3DTextHighlighted);
	}

	if (tmItem.ExcludeTag (_T("FLAT"), strTab))
	{
		CBCGPTagManager tmTab (strTab);

		CString strBtn;
		if (tmTab.ExcludeTag (_T("BUTTON"), strBtn))
		{
			CBCGPControlRendererParams params (MakeResourceID(_T("TAB_FLAT")), CRect (0, 0, 0, 0), CRect (0, 0, 0, 0));
			if (CBCGPTagManager::ParseControlRendererParams (strBtn, params))
			{
				m_ctrlTabFlat[0].Create (params);
				m_ctrlTabFlat[1].Create (params, TRUE);
			}
		}

		tmTab.ReadColor (_T("Face"), m_clrTabFlatFace);
		tmTab.ReadColor (_T("Black"), m_clrTabFlatBlack);
		tmTab.ReadColor (_T("Dark"), m_clrTabFlatDark);
		tmTab.ReadColor (_T("DarkShadow"), m_clrTabFlatDarkShadow);
		tmTab.ReadColor (_T("Light"), m_clrTabFlatLight);
		tmTab.ReadColor (_T("Highlight"), m_clrTabFlatHighlight);

		COLORREF clr;
		if (tmTab.ReadColor (_T("BorderInnerNormal"), clr))
		{
			m_penTabFlatInner[0].DeleteObject ();
			m_penTabFlatInner[0].CreatePen (PS_SOLID, 1, clr);
		}
		if (tmTab.ReadColor (_T("BorderInnerActive"), clr))
		{
			m_penTabFlatInner[1].DeleteObject ();
			m_penTabFlatInner[1].CreatePen (PS_SOLID, 1, clr);
		}
		if (tmTab.ReadColor (_T("BorderOuterNormal"), clr))
		{
			m_penTabFlatOuter[0].DeleteObject ();
			m_penTabFlatOuter[0].CreatePen (PS_SOLID, 1, clr);
		}
		if (tmTab.ReadColor (_T("BorderOuterActive"), clr))
		{
			m_penTabFlatOuter[1].DeleteObject ();
			m_penTabFlatOuter[1].CreatePen (PS_SOLID, 1, clr);
		}

		tmTab.ReadColor (_T("TextActive"), m_clrTabFlatTextActive);
		tmTab.ReadColor (_T("TextInactive"), m_clrTabFlatTextInactive);
		tmTab.ReadColor (_T("TextHighlighted"), m_clrTabFlatTextHighlighted);
	}

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLHeader(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

	CBCGPTagManager tmItem (strItem);

	tmItem.ReadColor (_T("NormalStart"), m_clrHeaderNormalStart);
	tmItem.ReadColor (_T("NormalFinish"), m_clrHeaderNormalFinish);
	tmItem.ReadColor (_T("NormalBorder"), m_clrHeaderNormalBorder);
	tmItem.ReadColor (_T("TextNormal"), m_clrHeaderNormalText);
	tmItem.ReadColor (_T("HighlightedStart"), m_clrHeaderHighlightedStart);
	tmItem.ReadColor (_T("HighlightedFinish"), m_clrHeaderHighlightedFinish);
	tmItem.ReadColor (_T("HighlightedBorder"), m_clrHeaderHighlightedBorder);
	tmItem.ReadColor (_T("TextHighlighted"), m_clrHeaderHighlightedText);
	tmItem.ReadColor (_T("PressedStart"), m_clrHeaderPressedStart);
	tmItem.ReadColor (_T("PressedFinish"), m_clrHeaderPressedFinish);
	tmItem.ReadColor (_T("PressedBorder"), m_clrHeaderPressedBorder);
	tmItem.ReadColor (_T("TextPressed"), m_clrHeaderPressedText);

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLRibbon(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

	CBCGPTagManager tmItem (strItem);

	CString str;

	if (tmItem.ExcludeTag (_T("CATEGORY"), str))
	{
		CBCGPTagManager tmCategory (str);
		tmCategory.ReadControlRenderer(_T("BACK"), m_ctrlRibbonCategoryBack, MakeResourceID(_T("RB_CAT_BACK")));

		CString strTab;
		if (tmCategory.ExcludeTag (_T("TAB"), strTab))
		{
			CBCGPTagManager tmTab (strTab);
			tmTab.ReadControlRenderer(_T("BUTTON"), m_ctrlRibbonCategoryTab, MakeResourceID(_T("RB_CAT_TAB")));

			tmTab.ReadColor (_T("TextNormal"), m_clrRibbonCategoryText);
			tmTab.ReadColor (_T("TextHighlighted"), m_clrRibbonCategoryTextHighlighted);
			tmTab.ReadColor (_T("TextActive"), m_clrRibbonCategoryTextActive);
			tmTab.ReadColor (_T("TextDisabled"), m_clrRibbonCategoryTextDisabled);
		}

		tmCategory.ReadControlRenderer (_T("TAB_SEPARATOR"), m_ctrlRibbonCategoryTabSep, MakeResourceID(_T("RB_CAT_TAB_SEP")));

		tmCategory.ReadControlRenderer (_T("BUTTON_PAGE_L"), m_ctrlRibbonCategoryBtnPage[0], MakeResourceID(_T("RB_BTN_PAGE_L")));
		tmCategory.ReadControlRenderer (_T("BUTTON_PAGE_R"), m_ctrlRibbonCategoryBtnPage[1], MakeResourceID(_T("RB_BTN_PAGE_R")));
	}

	if (tmItem.ExcludeTag (_T("PANEL"), str))
	{
		CBCGPTagManager tmPanel (str);

		{
			CString strBack;
			if (tmPanel.ExcludeTag (_T("BACK"), strBack))
			{
				CBCGPTagManager tmBack (strBack);

				tmBack.ReadControlRenderer(_T("TOP"), m_ctrlRibbonPanelBack_T, MakeResourceID(_T("RB_PNL_BACK_T")));
				tmBack.ReadControlRenderer(_T("BOTTOM"), m_ctrlRibbonPanelBack_B, MakeResourceID(_T("RB_PNL_BACK_B")));
			}
		}

		{
			CString strCaption;
			if (tmPanel.ExcludeTag (_T("CAPTION"), strCaption))
			{
				CBCGPTagManager tmCaption (strCaption);

				tmCaption.ReadControlRenderer (_T("LAUNCH_BTN"), m_ctrlRibbonBtnLaunch, MakeResourceID(_T("RB_BTN_LAUNCH")));
				tmCaption.ReadToolBarImages (_T("LAUNCH_ICON"), m_RibbonBtnLaunchIcon, MakeResourceID(_T("RB_BTN_LAUNCH_ICON")));
				tmCaption.ReadColor (_T("TextNormal"), m_clrRibbonPanelCaptionText);
				tmCaption.ReadColor (_T("TextHighlighted"), m_clrRibbonPanelCaptionTextHighlighted);

				m_RibbonBtnLaunchIcon.SmoothResize (globalData.GetRibbonImageScale ());
			}
		}

		tmPanel.ReadToolBarImages (_T("SEPARATOR"), m_RibbonPanelSeparator, MakeResourceID(_T("RB_PNL_SEPARATOR")));

		tmPanel.ReadControlRenderer (_T("QAT"), m_ctrlRibbonPanelQAT, MakeResourceID(_T("RB_PNL_QAT")));

		{
			CString strButtons;
			if (tmPanel.ExcludeTag (_T("BUTTONS"), strButtons))
			{
				CBCGPTagManager tmButtons (strButtons);

				tmButtons.ReadControlRenderer (_T("BUTTON_GROUP_F"), m_ctrlRibbonBtnGroup_F, MakeResourceID(_T("RB_BTN_GRP_F")));
				tmButtons.ReadControlRenderer (_T("BUTTON_GROUP_M"), m_ctrlRibbonBtnGroup_M, MakeResourceID(_T("RB_BTN_GRP_M")));
				tmButtons.ReadControlRenderer (_T("BUTTON_GROUP_L"), m_ctrlRibbonBtnGroup_L, MakeResourceID(_T("RB_BTN_GRP_L")));
				tmButtons.ReadControlRenderer (_T("BUTTON_GROUP_S"), m_ctrlRibbonBtnGroup_S, MakeResourceID(_T("RB_BTN_GRP_S")));

				tmButtons.ReadControlRenderer (_T("BUTTON_GROUPMENU_F_C"), m_ctrlRibbonBtnGroupMenu_F[0], MakeResourceID(_T("RB_BTN_GRPMENU_F_C")));
				tmButtons.ReadControlRenderer (_T("BUTTON_GROUPMENU_F_M"), m_ctrlRibbonBtnGroupMenu_F[1], MakeResourceID(_T("RB_BTN_GRPMENU_F_M")));
				tmButtons.ReadControlRenderer (_T("BUTTON_GROUPMENU_M_C"), m_ctrlRibbonBtnGroupMenu_M[0], MakeResourceID(_T("RB_BTN_GRPMENU_M_C")));
				tmButtons.ReadControlRenderer (_T("BUTTON_GROUPMENU_M_M"), m_ctrlRibbonBtnGroupMenu_M[1], MakeResourceID(_T("RB_BTN_GRPMENU_M_M")));
				tmButtons.ReadControlRenderer (_T("BUTTON_GROUPMENU_L_C"), m_ctrlRibbonBtnGroupMenu_L[0], MakeResourceID(_T("RB_BTN_GRPMENU_L_C")));
				tmButtons.ReadControlRenderer (_T("BUTTON_GROUPMENU_L_M"), m_ctrlRibbonBtnGroupMenu_L[1], MakeResourceID(_T("RB_BTN_GRPMENU_L_M")));

				tmButtons.ReadControlRenderer (_T("BUTTON_NORMAL_S"), m_ctrlRibbonBtn[0], MakeResourceID(_T("RB_BTN_NORMAL_S")));
				tmButtons.ReadControlRenderer (_T("BUTTON_NORMAL_B"), m_ctrlRibbonBtn[1], MakeResourceID(_T("RB_BTN_NORMAL_B")));

				tmButtons.ReadControlRenderer (_T("BUTTON_DEFAULT"), m_ctrlRibbonBtnDefault, MakeResourceID(_T("RB_BTN_DEF")));
				tmButtons.ReadControlRenderer (_T("BUTTON_DEFAULT_ICON"), m_ctrlRibbonBtnDefaultIcon, MakeResourceID(_T("RB_BTN_DEF_ICON")));
				tmButtons.ReadToolBarImages (_T("BUTTON_DEFAULT_IMAGE"), m_RibbonBtnDefaultImage, MakeResourceID(_T("RB_BTN_DEF_IMAGE")));
				tmButtons.ReadControlRenderer (_T("BUTTON_DEFAULT_QAT"), m_ctrlRibbonBtnDefaultQAT, MakeResourceID(_T("RB_BTN_DEF_QAT")));

				if (!m_ctrlRibbonBtnDefaultQAT.IsValid ())
				{
					tmButtons.ReadControlRenderer (_T("BUTTON_DEFAULT_QAT_ICON"), m_ctrlRibbonBtnDefaultQATIcon, MakeResourceID(_T("RB_BTN_DEF_QAT_ICON")));
				}

				tmButtons.ReadControlRenderer (_T("BUTTON_MENU_H_C"), m_ctrlRibbonBtnMenuH[0], MakeResourceID(_T("RB_BTN_MENU_H_C")));
				tmButtons.ReadControlRenderer (_T("BUTTON_MENU_H_M"), m_ctrlRibbonBtnMenuH[1], MakeResourceID(_T("RB_BTN_MENU_H_M")));
				tmButtons.ReadControlRenderer (_T("BUTTON_MENU_V_C"), m_ctrlRibbonBtnMenuV[0], MakeResourceID(_T("RB_BTN_MENU_V_C")));
				tmButtons.ReadControlRenderer (_T("BUTTON_MENU_V_M"), m_ctrlRibbonBtnMenuV[1], MakeResourceID(_T("RB_BTN_MENU_V_M")));
				tmButtons.ReadControlRenderer (_T("BUTTON_CHECK"), m_ctrlRibbonBtnCheck, MakeResourceID(_T("RB_BTN_CHECK")));
				tmButtons.ReadControlRenderer (_T("BUTTON_RADIO"), m_ctrlRibbonBtnRadio, MakeResourceID(_T("RB_BTN_RADIO")));
				tmButtons.ReadControlRenderer (_T("BUTTON_PUSH"), m_ctrlRibbonBtnPush, MakeResourceID(_T("RB_BTN_PUSH")));
				tmButtons.ReadControlRenderer (_T("BUTTON_GROUP"), m_ctrlRibbonBtnGroup, MakeResourceID(_T("RB_BTN_GRP")));

				m_ctrlRibbonBtnCheck.SmoothResize (globalData.GetRibbonImageScale ());
				m_ctrlRibbonBtnRadio.SmoothResize (globalData.GetRibbonImageScale ());

				tmButtons.ReadControlRenderer (_T("BUTTON_PNL_T"), m_ctrlRibbonBtnPalette[0], MakeResourceID(_T("RB_BTN_PALETTE_T")));
				tmButtons.ReadControlRenderer (_T("BUTTON_PNL_M"), m_ctrlRibbonBtnPalette[1], MakeResourceID(_T("RB_BTN_PALETTE_M")));
				tmButtons.ReadControlRenderer (_T("BUTTON_PNL_B"), m_ctrlRibbonBtnPalette[2], MakeResourceID(_T("RB_BTN_PALETTE_B")));

				tmButtons.ReadColor (_T("TextNormal"), m_clrRibbonBarBtnText);
				tmButtons.ReadColor (_T("TextHighlighted"), m_clrRibbonBarBtnTextHighlighted);
				tmButtons.ReadColor (_T("TextDisabled"), m_clrRibbonBarBtnTextDisabled);
			}
		}

		{
			CString strEdit;
			if (tmPanel.ExcludeTag (_T("EDIT"), strEdit))
			{
				CBCGPTagManager tmEdit (strEdit);

				tmEdit.ReadColor (_T("Normal"), m_clrRibbonEdit);
				tmEdit.ReadColor (_T("Highlighted"), m_clrRibbonEditHighlighted);
				tmEdit.ReadColor (_T("Disabled"), m_clrRibbonEditDisabled);
				tmEdit.ReadColor (_T("Pressed"), m_clrRibbonEditPressed);

				tmEdit.ReadColor (_T("BorderNormal"), m_clrRibbonEditBorder);
				tmEdit.ReadColor (_T("BorderHighlighted"), m_clrRibbonEditBorderHighlighted);
				tmEdit.ReadColor (_T("BorderDisabled"), m_clrRibbonEditBorderDisabled);
				tmEdit.ReadColor (_T("BorderPressed"), m_clrRibbonEditBorderPressed);
				tmEdit.ReadColor (_T("Selection"), m_clrRibbonEditSelection);

				CString strButton;
				if (tmEdit.ExcludeTag (_T("BUTTON"), strButton))
				{
					CBCGPTagManager tmButton (strButton);

					tmButton.ReadColor (_T("GradientStartNormal"), m_clrRibbonComboBtnStart);
					tmButton.ReadColor (_T("GradientFinishNormal"), m_clrRibbonComboBtnFinish);
					tmButton.ReadColor (_T("BtnBorderNormal"), m_clrRibbonComboBtnBorder);

					if (!tmButton.ReadControlRenderer (_T("IMAGE"), m_ctrlRibbonComboBoxBtn, MakeResourceID(_T("COMBOBOX_BTN"))))
					{
						tmButton.ReadColor (_T("GradientStartHighlighted"), m_clrRibbonComboBtnHighlightedStart);
						tmButton.ReadColor (_T("GradientFinishHighlighted"), m_clrRibbonComboBtnHighlightedFinish);
						tmButton.ReadColor (_T("GradientStartDisabled"), m_clrRibbonComboBtnDisabledStart);
						tmButton.ReadColor (_T("GradientFinishDisabled"), m_clrRibbonComboBtnDisabledFinish);
						tmButton.ReadColor (_T("GradientStartPressed"), m_clrRibbonComboBtnPressedStart);
						tmButton.ReadColor (_T("GradientFinishPressed"), m_clrRibbonComboBtnPressedFinish);

						tmButton.ReadColor (_T("BtnBorderHighlighted"), m_clrRibbonComboBtnBorderHighlighted);
						tmButton.ReadColor (_T("BtnBorderDisabled"), m_clrRibbonComboBtnBorderDisabled);

						m_clrRibbonComboBtnBorderPressed = m_clrRibbonComboBtnBorderHighlighted;
						tmButton.ReadColor (_T("BtnBorderPressed"), m_clrRibbonComboBtnBorderPressed);
					}
				}
			}
		}

		tmPanel.ReadColor (_T("TextNormal"), m_clrRibbonPanelText);
		tmPanel.ReadColor (_T("TextHighlighted"), m_clrRibbonPanelTextHighlighted);
	}

	m_clrRibbonContextPanelText = m_clrRibbonPanelText;
	m_clrRibbonContextPanelTextHighlighted = m_clrRibbonPanelTextHighlighted;
	m_clrRibbonContextPanelCaptionText = m_clrRibbonPanelCaptionText;
	m_clrRibbonContextPanelCaptionTextHighlighted = m_clrRibbonPanelCaptionTextHighlighted;

	if (tmItem.ExcludeTag (_T("CONTEXT"), str))
	{
		CBCGPTagManager tmContext (str);

		CString strCategory;
		if (tmContext.ExcludeTag (_T("CATEGORY"), strCategory))
		{
			CBCGPTagManager tmCategory (strCategory);

			CBCGPControlRendererParams prBack;
			CBCGPControlRendererParams prCaption;
			CBCGPControlRendererParams prTab;
			CBCGPControlRendererParams prDefault;
			COLORREF clrText = m_clrRibbonCategoryText;
			COLORREF clrTextHighlighted = m_clrRibbonCategoryTextHighlighted;
			COLORREF clrTextActive = m_clrRibbonCategoryTextActive;
			COLORREF clrCaptionText = clrText;

			tmCategory.ReadControlRendererParams (_T("BACK"), prBack);

			CString strTab;
			if (tmCategory.ExcludeTag (_T("TAB"), strTab))
			{
				CBCGPTagManager tmTab (strTab);

				tmTab.ReadControlRendererParams(_T("BUTTON"), prTab);
				tmTab.ReadColor (_T("TextNormal"), clrText);
				tmTab.ReadColor (_T("TextHighlighted"), clrTextHighlighted);
				tmTab.ReadColor (_T("TextActive"), clrTextActive);
			}

			CString strCaption;
			if (tmCategory.ExcludeTag (_T("CAPTION"), strCaption))
			{
				CBCGPTagManager tmCaption (strCaption);

				tmCaption.ReadControlRendererParams(_T("BACK"), prCaption);
				tmCaption.ReadColor (_T("TextNormal"), clrCaptionText);
			}

			tmCategory.ReadControlRendererParams(_T("BUTTON_DEFAULT"), prDefault);

			CString strID[BCGPRibbonCategoryColorCount] = 
				{
					MakeResourceID(_T("RB_CTX_R_")),
					MakeResourceID(_T("RB_CTX_O_")),
					MakeResourceID(_T("RB_CTX_Y_")),
					MakeResourceID(_T("RB_CTX_G_")),
					MakeResourceID(_T("RB_CTX_B_")),
					MakeResourceID(_T("RB_CTX_I_")),
					MakeResourceID(_T("RB_CTX_V_"))
				};

			for (int i = 0; i < BCGPRibbonCategoryColorCount; i++)
			{
				XRibbonContextCategory& cat = m_ctrlRibbonContextCategory[i];

				prDefault.m_strBmpResID = strID[i] + _T("BTN_DEF");
				prTab.m_strBmpResID     = strID[i] + _T("CAT_TAB");
				prCaption.m_strBmpResID = strID[i] + _T("CAT_CAPTION");
				prBack.m_strBmpResID    = strID[i] + _T("CAT_BACK");

				cat.m_ctrlBtnDefault.Create (prDefault);
				cat.m_ctrlCaption.Create (prCaption);
				cat.m_ctrlTab.Create (prTab);
				cat.m_ctrlBack.Create (prBack);
				cat.m_clrText            = clrText;
				cat.m_clrTextHighlighted = clrTextHighlighted;
				cat.m_clrTextActive      = clrTextActive;
				cat.m_clrCaptionText     = clrCaptionText;
			}
		}

		CString strPanel;
		if (tmContext.ExcludeTag (_T("PANEL"), strPanel))
		{
			CBCGPTagManager tmPanel (strPanel);

			CString strBack;
			if (tmPanel.ExcludeTag (_T("BACK"), strBack))
			{
				CBCGPTagManager tmBack (strBack);

				tmBack.ReadControlRenderer(_T("TOP"), m_ctrlRibbonContextPanelBack_T, MakeResourceID(_T("RB_CTX_PNL_BACK_T")));
				tmBack.ReadControlRenderer(_T("BOTTOM"), m_ctrlRibbonContextPanelBack_B, MakeResourceID(_T("RB_CTX_PNL_BACK_B")));
			}

			CString strCaption;
			if (tmPanel.ExcludeTag (_T("CAPTION"), strCaption))
			{
				CBCGPTagManager tmCaption (strCaption);

				tmCaption.ReadColor (_T("TextNormal"), m_clrRibbonContextPanelCaptionText);
				tmCaption.ReadColor (_T("TextHighlighted"), m_clrRibbonContextPanelCaptionTextHighlighted);
			}

			tmPanel.ReadColor (_T("TextNormal"), m_clrRibbonContextPanelText);
			tmPanel.ReadColor (_T("TextHighlighted"), m_clrRibbonContextPanelTextHighlighted);
		}

		tmContext.ReadControlRenderer (_T("SEPARATOR"), m_ctrlRibbonContextSeparator, MakeResourceID(_T("RB_CTX_SEPARATOR")));
	}

	tmItem.ReadControlRenderer (_T("MAIN_BUTTON"), m_RibbonBtnMain, MakeResourceID(_T("RB_BTN_MAIN")));

	if (m_RibbonBtnMain.IsValid ())
	{
		m_RibbonBtnMain.SmoothResize (globalData.GetRibbonImageScale ());
	}

	if (tmItem.ExcludeTag (_T("MAIN"), str))
	{
		CBCGPTagManager tmMain (str);
		tmMain.ReadColor (_T("Bkgnd"), m_clrRibbonMainPanelBkgnd);
		tmMain.ReadControlRenderer (_T("BACK"), m_ctrlRibbonMainPanel, MakeResourceID(_T("RB_PNL_MAIN")));
		tmMain.ReadControlRenderer (_T("BORDER"), m_ctrlRibbonMainPanelBorder, MakeResourceID(_T("RB_PNL_MAIN_BRD")));
		tmMain.ReadControlRenderer (_T("BUTTON"), m_ctrlRibbonBtnMainPanel, MakeResourceID(_T("RB_BTN_PNL_MAIN")));
	}

	if (tmItem.ExcludeTag (_T("CAPTION"), str))
	{
		CBCGPTagManager tmCaption (str);
		tmCaption.ReadControlRenderer(_T("QA"), m_ctrlRibbonCaptionQA, MakeResourceID(_T("RB_CAPTION_QA")));
		tmCaption.ReadControlRenderer(_T("QA_GLASS"), m_ctrlRibbonCaptionQA_Glass, MakeResourceID(_T("RB_CAPTION_QA_GLASS")));
	}

	if (tmItem.ExcludeTag (_T("STATUS"), str))
	{
		CBCGPTagManager tmStatus (str);
		tmStatus.ReadControlRenderer(_T("PANE_BUTTON"), m_ctrlRibbonBtnStatusPane, MakeResourceID(_T("RB_BTN_STATUS_PANE")));

		CString strSlider;
		if (tmStatus.ExcludeTag (_T("SLIDER"), strSlider))
		{
			CBCGPTagManager tmSlider (strSlider);

			tmSlider.ReadControlRenderer(_T("THUMB"), m_ctrlRibbonSliderThumb, MakeResourceID(_T("RB_SLIDER_THUMB")));
			tmSlider.ReadControlRenderer(_T("THUMB_H"), m_ctrlRibbonSliderThumbA[0], MakeResourceID(_T("RB_SLIDER_THUMB_H")));
			tmSlider.ReadControlRenderer(_T("THUMB_T"), m_ctrlRibbonSliderThumbA[1], MakeResourceID(_T("RB_SLIDER_THUMB_T")));
			tmSlider.ReadControlRenderer(_T("THUMB_R"), m_ctrlRibbonSliderThumbA[2], MakeResourceID(_T("RB_SLIDER_THUMB_R")));
			tmSlider.ReadControlRenderer(_T("THUMB_V"), m_ctrlRibbonSliderThumbA[3], MakeResourceID(_T("RB_SLIDER_THUMB_V")));
			tmSlider.ReadControlRenderer(_T("THUMB_L"), m_ctrlRibbonSliderThumbA[4], MakeResourceID(_T("RB_SLIDER_THUMB_L")));

			tmSlider.ReadControlRenderer(_T("PLUS"), m_ctrlRibbonSliderBtnPlus, MakeResourceID(_T("RB_SLIDER_BTN_PLUS")));
			tmSlider.ReadControlRenderer(_T("MINUS"), m_ctrlRibbonSliderBtnMinus, MakeResourceID(_T("RB_SLIDER_BTN_MINUS")));

			m_ctrlRibbonSliderThumb.SmoothResize (globalData.GetRibbonImageScale ());
			m_ctrlRibbonSliderThumbA[0].SmoothResize (globalData.GetRibbonImageScale ());
			m_ctrlRibbonSliderThumbA[1].SmoothResize (globalData.GetRibbonImageScale ());
			m_ctrlRibbonSliderThumbA[2].SmoothResize (globalData.GetRibbonImageScale ());
			m_ctrlRibbonSliderThumbA[3].SmoothResize (globalData.GetRibbonImageScale ());
			m_ctrlRibbonSliderThumbA[4].SmoothResize (globalData.GetRibbonImageScale ());

			m_ctrlRibbonSliderBtnMinus.SmoothResize (globalData.GetRibbonImageScale ());
			m_ctrlRibbonSliderBtnPlus.SmoothResize (globalData.GetRibbonImageScale ());
		}

		CString strProgress;
		if (tmStatus.ExcludeTag (_T("PROGRESS"), strProgress))
		{
			CBCGPTagManager tmProgress (strProgress);

			tmProgress.ReadControlRenderer(_T("BACK"), m_ctrlRibbonProgressBack, MakeResourceID(_T("RB_PROGRESS_BACK")));
			tmProgress.ReadControlRenderer(_T("NORMAL"), m_ctrlRibbonProgressNormal, MakeResourceID(_T("RB_PROGRESS_NORMAL")));
			tmProgress.ReadControlRenderer(_T("NORMAL_EXT"), m_ctrlRibbonProgressNormalExt, MakeResourceID(_T("RB_PROGRESS_NORMAL_EXT")));
			tmProgress.ReadControlRenderer(_T("INFINITY"), m_ctrlRibbonProgressInfinity, MakeResourceID(_T("RB_PROGRESS_INFINITY")));

			tmProgress.ReadControlRenderer(_T("BACK_V"), m_ctrlRibbonProgressBackV, MakeResourceID(_T("RB_PROGRESS_BACK_V")));
			if (!m_ctrlRibbonProgressBackV.IsValid())
			{
				m_ctrlRibbonProgressBackV.Create(m_ctrlRibbonProgressBack.GetParams());
				m_ctrlRibbonProgressBackV.Rotate(FALSE);
			}
			tmProgress.ReadControlRenderer(_T("NORMAL_V"), m_ctrlRibbonProgressNormalV, MakeResourceID(_T("RB_PROGRESS_NORMAL_V")));
			if (!m_ctrlRibbonProgressNormalV.IsValid())
			{
				m_ctrlRibbonProgressNormalV.Create(m_ctrlRibbonProgressNormal.GetParams());
				m_ctrlRibbonProgressNormalV.Rotate(FALSE);
			}

			tmProgress.ReadControlRenderer(_T("NORMAL_EXT_V"), m_ctrlRibbonProgressNormalExtV, MakeResourceID(_T("RB_PROGRESS_NORMAL_EXT_V")));
			if (!m_ctrlRibbonProgressNormalExtV.IsValid())
			{
				m_ctrlRibbonProgressNormalExtV.Create(m_ctrlRibbonProgressNormalExt.GetParams());
				m_ctrlRibbonProgressNormalExtV.Rotate(FALSE);
			}

			tmProgress.ReadControlRenderer(_T("INFINITY_V"), m_ctrlRibbonProgressInfinityV, MakeResourceID(_T("RB_PROGRESS_INFINITY_V")));
			if (!m_ctrlRibbonProgressInfinityV.IsValid())
			{
				m_ctrlRibbonProgressInfinityV.Create(m_ctrlRibbonProgressInfinity.GetParams());
				m_ctrlRibbonProgressInfinityV.Rotate(FALSE);
			}
		}
	}

	if (tmItem.ExcludeTag (_T("BORDERS"), str))
	{
		CBCGPTagManager tmBorders (str);

		tmBorders.ReadControlRenderer (_T("QAT"), m_ctrlRibbonBorder_QAT, MakeResourceID(_T("RB_BRD_QAT")));
		tmBorders.ReadControlRenderer (_T("FLOATY"), m_ctrlRibbonBorder_Floaty, MakeResourceID(_T("RB_BRD_FLOATY")));
		tmBorders.ReadControlRenderer (_T("PANEL"), m_ctrlRibbonBorder_Panel, MakeResourceID(_T("RB_BRD_PNL")));
	}

	if (tmItem.ExcludeTag (_T("KEYTIP"), str))
	{
		CBCGPTagManager tmKeyTip (str);

		tmKeyTip.ReadControlRenderer(_T("BACK"), m_ctrlRibbonKeyTip, MakeResourceID(_T("RB_KEYTIP_BACK")));

		tmKeyTip.ReadColor (_T("TextNormal"), m_clrRibbonKeyTipTextNormal);

		BOOL bSystem = FALSE;
		if (m_clrRibbonKeyTipTextNormal == (COLORREF) (-1))
		{
			if (m_bToolTipParams && m_ToolTipParams.m_clrText != (COLORREF) (-1))
			{
				m_clrRibbonKeyTipTextNormal = m_ToolTipParams.m_clrText;
			}
			else
			{
				bSystem = TRUE;
				m_clrRibbonKeyTipTextNormal = ::GetSysColor (COLOR_INFOTEXT);
			}
		}

		tmKeyTip.ReadColor (_T("TextDisabled"), m_clrRibbonKeyTipTextDisabled);

		if (m_clrRibbonKeyTipTextDisabled == (COLORREF) (-1))
		{
			if (bSystem)
			{
				m_clrRibbonKeyTipTextDisabled = globalData.clrGrayedText;
			}
			else
			{
				m_clrRibbonKeyTipTextDisabled = CBCGPDrawManager::PixelAlpha (
					m_clrRibbonKeyTipTextNormal, globalData.clrWindow, 50);
			}
		}
	}

	if (tmItem.ExcludeTag (_T("HYPERLINK"), str))
	{
		CBCGPTagManager tmHyperlink (str);

		tmHyperlink.ReadColor (_T("Inactive"),			m_clrRibbonHyperlinkInactive);
		tmHyperlink.ReadColor (_T("Active"),			m_clrRibbonHyperlinkActive);
		tmHyperlink.ReadColor (_T("StatusbarInactive"),	m_clrRibbonStatusbarHyperlinkInactive);
		tmHyperlink.ReadColor (_T("StatusbarActive"),	m_clrRibbonStatusbarHyperlinkActive);
	}

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLPlanner(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

	CBCGPTagManager tmItem (strItem);

	if (tmItem.ReadColor (_T("WorkColor"), m_clrPlannerWork))
	{
		m_brPlanner.DeleteObject ();
		m_brPlanner.CreateSolidBrush (m_clrPlannerWork);
	}

	tmItem.ReadColor (_T("NcAreaColor"), m_clrPlannerNcArea);
	tmItem.ReadColor (_T("NcLineColor"), m_clrPlannerNcLine);
	tmItem.ReadColor (_T("NcTextColor"), m_clrPlannerNcText);

	CString str;
	if (tmItem.ExcludeTag (_T("TODAY"), str))
	{
		CBCGPTagManager tmToday (str);

		tmToday.ReadColor (_T("Caption1Start") , m_clrPlannerTodayCaption[0]);
		tmToday.ReadColor (_T("Caption1Finish"), m_clrPlannerTodayCaption[1]);
		tmToday.ReadColor (_T("Caption2Start") , m_clrPlannerTodayCaption[2]);
		tmToday.ReadColor (_T("Caption2Finish"), m_clrPlannerTodayCaption[3]);
		tmToday.ReadColor (_T("BorderColor")   , m_clrPlannerTodayBorder);
	}

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLGrid(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

	CBCGPTagManager tmItem (strItem);

	CString strHeader;
	if (tmItem.ExcludeTag (_T("HEADER"), strHeader))
	{
		CBCGPTagManager tmHeader (strHeader);

		tmHeader.ReadColor (_T("NormalStart"), m_clrGridHeaderNormalStart);
		tmHeader.ReadColor (_T("NormalFinish"), m_clrGridHeaderNormalFinish);
		tmHeader.ReadColor (_T("NormalBorder"), m_clrGridHeaderNormalBorder);

		m_clrGridHeaderPressedStart  = m_clrGridHeaderNormalFinish;
		m_clrGridHeaderPressedFinish = m_clrGridHeaderNormalStart;
		m_clrGridHeaderPressedBorder = m_clrGridHeaderNormalBorder;

		tmHeader.ReadColor (_T("PressedStart"), m_clrGridHeaderPressedStart);
		tmHeader.ReadColor (_T("PressedFinish"), m_clrGridHeaderPressedFinish);
		tmHeader.ReadColor (_T("PressedBorder"), m_clrGridHeaderPressedBorder);

		m_clrGridHeaderAllNormalBackStart         = CBCGPDrawManager::MixColors (m_clrGridHeaderNormalStart, m_clrGridHeaderNormalBorder, 0.50);
		m_clrGridHeaderAllNormalBackFinish        = m_clrGridHeaderAllNormalBackStart;
		m_clrGridHeaderAllPressedBackStart        = m_clrHeaderPressedBorder;
		m_clrGridHeaderAllPressedBackFinish       = m_clrGridHeaderAllPressedBackStart;
		m_clrGridHeaderAllNormalBorderHighlighted = m_clrGridHeaderNormalStart;
		m_clrGridHeaderAllNormalBorderShadow      = m_clrGridHeaderNormalFinish;
		m_clrGridHeaderAllPressedBorderHighlighted= m_clrGridHeaderAllNormalBorderHighlighted;
		m_clrGridHeaderAllPressedBorderShadow     = m_clrGridHeaderAllNormalBorderShadow;
		m_clrGridHeaderAllNormalSignStart         = m_clrGridHeaderNormalStart;
		m_clrGridHeaderAllNormalSignFinish        = m_clrGridHeaderNormalFinish;
		m_clrGridHeaderAllPressedSignStart        = m_clrGridHeaderAllNormalSignStart;
		m_clrGridHeaderAllPressedSignFinish       = m_clrGridHeaderAllNormalSignFinish;

		CString strAll;
		if (tmHeader.ExcludeTag (_T("ALL_AREA"), strAll))
		{
			CBCGPTagManager tmAll (strAll);

			tmAll.ReadColor (_T("NormalBackStart"), m_clrGridHeaderAllNormalBackStart);
			m_clrGridHeaderAllNormalBackFinish = m_clrGridHeaderAllNormalBackStart;
			tmAll.ReadColor (_T("NormalBackFinish"), m_clrGridHeaderAllNormalBackFinish);

			m_clrGridHeaderAllPressedBackStart = m_clrGridHeaderAllNormalBackStart;
			m_clrGridHeaderAllPressedBackFinish = m_clrGridHeaderAllNormalBackFinish;
			tmAll.ReadColor (_T("PressedBackStart"), m_clrGridHeaderAllPressedBackStart);
			m_clrGridHeaderAllPressedBackFinish = m_clrGridHeaderAllPressedBackStart;
			tmAll.ReadColor (_T("PressedBackFinish"), m_clrGridHeaderAllPressedBackFinish);

			tmAll.ReadColor (_T("NormalBorderHighlighted"), m_clrGridHeaderAllNormalBorderHighlighted);
			m_clrGridHeaderAllNormalBorderShadow = m_clrGridHeaderAllNormalBorderHighlighted;
			tmAll.ReadColor (_T("NormalBorderShadow"), m_clrGridHeaderAllNormalBorderShadow);

			m_clrGridHeaderAllPressedBorderHighlighted = m_clrGridHeaderAllNormalBorderHighlighted;
			m_clrGridHeaderAllPressedBorderShadow = m_clrGridHeaderAllNormalBorderShadow;
			tmAll.ReadColor (_T("PressedBorderHighlighted"), m_clrGridHeaderAllPressedBorderHighlighted);
			m_clrGridHeaderAllPressedBorderShadow = m_clrGridHeaderAllPressedBorderHighlighted;
			tmAll.ReadColor (_T("PressedBorderShadow"), m_clrGridHeaderAllPressedBorderShadow);

			if (tmAll.ReadColor (_T("NormalSignStart"), m_clrGridHeaderAllNormalSignStart))
			{
				m_clrGridHeaderAllNormalSignFinish = m_clrGridHeaderAllNormalSignStart;
			}
			tmAll.ReadColor (_T("NormalSignFinish"), m_clrGridHeaderAllNormalSignFinish);

			m_clrGridHeaderAllPressedSignStart = m_clrGridHeaderAllNormalSignStart;
			m_clrGridHeaderAllPressedSignFinish = m_clrGridHeaderAllNormalSignFinish;
			if (tmAll.ReadColor (_T("PressedSignStart"), m_clrGridHeaderAllPressedSignStart))
			{
				m_clrGridHeaderAllPressedSignFinish = m_clrGridHeaderAllPressedSignStart;
			}
			tmAll.ReadColor (_T("PressedSignFinish"), m_clrGridHeaderAllPressedSignFinish);
		}

		COLORREF clr;
		if (tmHeader.ReadColor (_T("Separator"), clr))
		{
			m_penGridSeparator.DeleteObject ();
			m_penGridSeparator.CreatePen (PS_SOLID, 1, clr);
		}
	}

	COLORREF clr = globalData.clrBtnLight;
	if (tmItem.ReadColor (_T("ExpandBoxLight"), clr))
	{
		m_penGridExpandBoxLight.DeleteObject ();
		m_penGridExpandBoxLight.CreatePen (PS_SOLID, 1, clr);
	}

	clr = globalData.clrBtnShadow;
	if (tmItem.ReadColor (_T("ExpandBoxDark"), clr))
	{
		m_penGridExpandBoxDark.DeleteObject ();
		m_penGridExpandBoxDark.CreatePen (PS_SOLID, 1, clr);
	}

	tmItem.ReadColor (_T("LeftOffset"), m_clrGridLeftOffset);

	tmItem.ReadColor (_T("GroupLine"), m_clrGridGroupLine);
	m_clrGridGroupSubLine = m_clrGridGroupLine;
	tmItem.ReadColor (_T("GroupSubLine"), m_clrGridGroupSubLine);

#ifndef BCGP_EXCLUDE_GRID_CTRL
	if (!tmItem.ReadGridColors (_T("COLORS"), m_GridColors))
	{
		m_GridColors.m_clrHorzLine                     = m_clrToolBarGradientDark;
		m_GridColors.m_clrVertLine                     = m_clrToolBarBottomLine;

		m_GridColors.m_EvenColors.m_clrBackground      = m_clrToolBarGradientLight;
		m_GridColors.m_EvenColors.m_clrText            = m_clrToolBarBtnText;

		m_GridColors.m_OddColors.m_clrBackground       = m_clrToolBarGradientDark;
		m_GridColors.m_OddColors.m_clrText             = m_clrToolBarBtnText;

		m_GridColors.m_SelColors.m_clrBackground       = m_clrRibbonComboBtnHighlightedFinish;

		m_GridColors.m_GroupColors.m_clrBackground     = m_clrToolBarGradientDark;
		m_GridColors.m_GroupColors.m_clrGradient       = m_clrToolBarGradientLight;
		m_GridColors.m_GroupColors.m_clrText           = m_clrToolBarBtnText;

		m_GridColors.m_GroupSelColors.m_clrBackground  = m_clrRibbonComboBtnHighlightedStart;
		m_GridColors.m_GroupSelColors.m_clrGradient    = m_clrRibbonComboBtnHighlightedFinish;

		m_GridColors.m_HeaderSelColors.m_clrBackground = m_clrRibbonComboBtnHighlightedStart;
		m_GridColors.m_HeaderSelColors.m_clrGradient   = m_clrRibbonComboBtnHighlightedFinish;
		m_GridColors.m_HeaderSelColors.m_clrBorder     = m_clrRibbonComboBtnBorderHighlighted;

		m_GridColors.m_LeftOffsetColors.m_clrBackground= m_clrToolBarGradientDark;
		m_GridColors.m_LeftOffsetColors.m_clrBorder	   = m_GridColors.m_clrHorzLine;
	}
#endif	

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLOutlook(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

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

		tmPage.ReadControlRenderer (_T("BACK"), m_ctrlOutlookWndPageBtn, MakeResourceID(_T("OUTLOOK_BTN_PAGE")));

		tmPage.ReadColor (_T("TextNormal"), m_clrOutlookPageTextNormal);
		tmPage.ReadColor (_T("TextHighlighted"), m_clrOutlookPageTextHighlighted);
		tmPage.ReadColor (_T("TextPressed"), m_clrOutlookPageTextPressed);
	}

	if (tmItem.ExcludeTag (_T("SPLITTER"), str))
	{
		CBCGPTagManager tmSplitter (str);

		tmSplitter.ReadControlRenderer (_T("BACK"), m_ctrlOutlookSplitter, MakeResourceID(_T("OUTLOOK_SPLITTER")));
	}

	if (tmItem.ExcludeTag (_T("BAR"), str))
	{
		CBCGPTagManager tmBar (str);

		tmBar.ReadControlRenderer (_T("BACK"), m_ctrlOutlookWndBar, MakeResourceID(_T("OUTLOOK_BAR_BACK")));
	}

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLPopup(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

	CBCGPTagManager tmItem (strItem);

	tmItem.ReadColor (_T("GradientFillLight"), m_clrPopupGradientLight);
	tmItem.ReadColor (_T("GradientFillDark"), m_clrPopupGradientDark);

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLSlider(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

	CBCGPTagManager tmItem (strItem);

	tmItem.ReadControlRenderer(_T("THUMB_B"), m_ctrlSliderThumb[0], MakeResourceID(_T("SLIDER_THUMB_B")));
	tmItem.ReadControlRenderer(_T("THUMB_H"), m_ctrlSliderThumb[1], MakeResourceID(_T("SLIDER_THUMB_H")));
	tmItem.ReadControlRenderer(_T("THUMB_T"), m_ctrlSliderThumb[2], MakeResourceID(_T("SLIDER_THUMB_T")));
	tmItem.ReadControlRenderer(_T("THUMB_R"), m_ctrlSliderThumb[3], MakeResourceID(_T("SLIDER_THUMB_R")));
	tmItem.ReadControlRenderer(_T("THUMB_V"), m_ctrlSliderThumb[4], MakeResourceID(_T("SLIDER_THUMB_V")));
	tmItem.ReadControlRenderer(_T("THUMB_L"), m_ctrlSliderThumb[5], MakeResourceID(_T("SLIDER_THUMB_L")));

	return TRUE;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::ParseStyleXMLCalculator(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

	CBCGPTagManager tmItem (strItem);

	tmItem.ReadControlRenderer(_T("BUTTON"), m_ctrlCalculatorBtn, MakeResourceID(_T("CALC_BTN")));
	if (tmItem.ReadColor (_T("TextNumber"), m_clrCalculatorBtnText[0]))
	{
		m_clrCalculatorBtnText[1] = m_clrCalculatorBtnText[0];
		m_clrCalculatorBtnText[2] = m_clrCalculatorBtnText[1];
	}
	if (tmItem.ReadColor (_T("TextExtended"), m_clrCalculatorBtnText[1]))
	{
		m_clrCalculatorBtnText[2] = m_clrCalculatorBtnText[1];
	}

	tmItem.ReadColor (_T("TextUser"), m_clrCalculatorBtnText[2]);

	return TRUE;
}
//*****************************************************************************
void CBCGPVisualManager2007::OnActivateApp(CWnd* pWnd, BOOL bActive)
{
	ASSERT_VALID (pWnd);
	
	if (pWnd->GetSafeHwnd () == NULL)
	{
		return;
	}
	
	if (globalData.DwmIsCompositionEnabled () && IsDWMCaptionSupported())
	{
		return;
	}
	
	// but do not stay active if the window is disabled
	if (!pWnd->IsWindowEnabled())
	{
		bActive = FALSE;
	}
	
	m_ActivateFlag[pWnd->GetSafeHwnd ()] = bActive;
	pWnd->SendMessage (WM_NCPAINT, 0, 0);
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::OnNcActivate (CWnd* pWnd, BOOL bActive)
{
	ASSERT_VALID (pWnd);

	if (pWnd->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	if (globalData.DwmIsCompositionEnabled () && IsDWMCaptionSupported())
	{
		return FALSE;
	}

	// stay active if WF_STAYACTIVE bit is on
	if (pWnd->m_nFlags & WF_STAYACTIVE)
	{
		bActive = TRUE;
	}

	// but do not stay active if the window is disabled
	if (!pWnd->IsWindowEnabled())
	{
		bActive = FALSE;
	}

	m_ActivateFlag[pWnd->GetSafeHwnd ()] = bActive;
	pWnd->SendMessage (WM_NCPAINT, 0, 0);

	return TRUE;
}
//*****************************************************************************
void CBCGPVisualManager2007::DrawNcBtn (CDC* pDC, const CRect& rect, UINT nButton, 
										BCGBUTTON_STATE state, BOOL bSmall, 
										BOOL bActive, BOOL bMDI/* = FALSE*/, BOOL bEnabled/* = TRUE */)
{
	ASSERT_VALID (pDC);

	CBCGPToolBarImages* pImage = NULL;

	const double dScale = globalData.GetRibbonImageScale ();
	int nIndex = bSmall ? 1 : 0;
	const CBCGPControlRendererParams& params = m_SysBtnBack[nIndex].GetParams();
	if (!bSmall && 
		((rect.Width() - params.m_rectCorners.left - params.m_rectCorners.right) < (int)(m_SysBtnRestore[nIndex].GetImageSize().cx * dScale) ||
		 (rect.Height() - params.m_rectCorners.top - params.m_rectCorners.bottom) < (int)(m_SysBtnRestore[nIndex].GetImageSize().cy * dScale)))
	{
		bSmall = TRUE;
		nIndex = 1;
	}

	if (nButton == SC_CLOSE)
	{
		pImage = &m_SysBtnClose[nIndex];
	}
	else if (nButton == SC_MINIMIZE)
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

	CSize sizeImage = pImage == NULL ? CSize (0, 0) : pImage->GetImageSize ();

	CRect rtBtnImage (CPoint (0, 0), sizeImage);

	if (state != ButtonsIsRegular)
	{
		if (!IsBeta () && bMDI)
		{
			m_ctrlRibbonBtn[0].Draw (pDC, rect, state == ButtonsIsHighlighted ? 0 : 1);
		}
		else
		{
			CBCGPControlRenderer* pBack = &m_SysBtnBack[nIndex];
			if (nButton == SC_CLOSE && m_SysBtnBackC[nIndex].IsValid ())
			{
				pBack = &m_SysBtnBackC[nIndex];
			}

			pBack->Draw (pDC, rect, state == ButtonsIsHighlighted ? 0 : 1);
		}

		rtBtnImage.OffsetRect (0, sizeImage.cy * 
			(state == ButtonsIsHighlighted ? 1 : 2));
	}
	else if (!bActive || !bEnabled)
	{
		rtBtnImage.OffsetRect (0, sizeImage.cy * 3);
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

		pImage->DrawEx (pDC, rectImage, 0, horz, vert, rtBtnImage);
	}
}
//*****************************************************************************
void CBCGPVisualManager2007::DrawNcText (CDC* pDC, CRect& rect, 
										 const CString& strTitle, 
										 const CString& strDocument, 
										 BOOL bPrefix, BOOL bActive, BOOL bIsRTL, 
										 BOOL bTextCenter,
										 BOOL bGlass/* = FALSE*/, int nGlassGlowSize/* = 0*/, 
										 COLORREF clrGlassText/* = (COLORREF)-1*/)
{
	if ((strTitle.IsEmpty () && strDocument.IsEmpty ()) || 
		rect.right <= rect.left)
	{
		return;
	}

	ASSERT_VALID (pDC);

	int nOldMode = pDC->SetBkMode (TRANSPARENT);
	COLORREF clrOldText = pDC->GetTextColor ();

	DWORD dwTextStyle = DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX |
		(bIsRTL ? DT_RTLREADING : 0);

	if (strDocument.IsEmpty ())
	{
		COLORREF clrText = GetNcCaptionTextColor(bActive, TRUE);

		int widthFull = rect.Width ();
		int width = pDC->GetTextExtent (strTitle).cx;

		if (bTextCenter && width < widthFull)
		{
			rect.left += (widthFull - width) / 2;
		}

		rect.right = min (rect.left + width, rect.right);

		if (rect.right > rect.left)
		{
			if (bGlass)
			{
				DrawTextOnGlass (pDC, strTitle, rect, dwTextStyle, nGlassGlowSize, clrGlassText);
			}
			else
			{
				pDC->SetTextColor (clrText);
				pDC->DrawText (strTitle, rect, dwTextStyle);
			}
		}
	}
	else
	{
		const CString& str1 = bPrefix ? strDocument : strTitle;
		const CString& str2 = bPrefix ? strTitle : strDocument;

		COLORREF clrText1 = GetNcCaptionTextColor(bActive, FALSE);
		COLORREF clrText2 = GetNcCaptionTextColor(bActive, TRUE);

		if (!bPrefix)
		{
			COLORREF clr = clrText1;
			clrText1 = clrText2;
			clrText2 = clr;
		}

		int widthFull = rect.Width ();
		CSize sz1 = pDC->GetTextExtent (str1);
		CSize sz2 = pDC->GetTextExtent (str2);
		int width = sz1.cx + sz2.cx;
		int left = rect.left;

		if (bTextCenter && width < widthFull)
		{
			rect.left += (widthFull - width) / 2;
		}

		rect.right = min (rect.left + width, rect.right);

		if (bIsRTL)
		{
			if (width <= rect.Width ())
			{
				rect.left += sz2.cx;
			}
			else
			{
				if (sz1.cx < rect.Width ())
				{
					rect.left += max (0, sz2.cx + (rect.Width () - width));
				}
			}
		}

		if (bGlass)
		{
			DrawTextOnGlass (pDC, str1, rect, dwTextStyle, nGlassGlowSize, clrGlassText);
		}
		else
		{
			pDC->SetTextColor (clrText1);
			pDC->DrawText (str1, rect, dwTextStyle);
		}

		if (bIsRTL)
		{
			if (width <= (rect.right - left))
			{
				rect.right = rect.left;
				rect.left  = rect.right - sz2.cx;
			}
			else
			{
				rect.left = left;
				rect.right -= sz1.cx;
			}
		}
		else
		{
			rect.left += sz1.cx;
		}

		if (rect.right > rect.left)
		{
			if (bGlass)
			{
				DrawTextOnGlass (pDC, str2, rect, dwTextStyle, nGlassGlowSize, clrGlassText);
			}
			else
			{
				pDC->SetTextColor (clrText2);
				pDC->DrawText (str2, rect, dwTextStyle);
			}
		}
	}

	pDC->SetBkMode    (nOldMode);
	pDC->SetTextColor (clrOldText);
}
//*****************************************************************************
void CBCGPVisualManager2007::DrawNcCaption (CDC* pDC, CRect rectCaption, 
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
	int indexBorder = (m_ctrlAppBorderCaption.GetImageCount() > 2 && bBorderOnly) ? 2 : 0;
	if (!bActive)
	{
		indexBorder++;
	}

    {
        if (IsBeta ())
        {
			COLORREF clr1  = bActive 
								? m_clrAppCaptionActiveStart 
								: m_clrAppCaptionInactiveStart;
			COLORREF clr2  = bActive 
								? m_clrAppCaptionActiveFinish 
								: m_clrAppCaptionInactiveFinish;

			CRect rectCaption1 (rectCaption);
			CRect rectBorder (m_ctrlAppBorderCaption.GetParams ().m_rectSides);
			
			rectCaption1.DeflateRect (rectBorder.left, rectBorder.top, 
				rectBorder.right, rectBorder.bottom);

			{
				CBCGPDrawManager dm (memDC);
				dm.Fill4ColorsGradient (rectCaption1, clr1, clr2, clr2, clr1, FALSE);
			}

			m_ctrlAppBorderCaption.DrawFrame (&memDC, rectCaption, indexBorder);
		}
		else
		{
			CRect rectBorderCaption (rectCaption);
			if (bMaximized)
			{
				rectBorderCaption.OffsetRect (-rectBorderCaption.TopLeft ());
				rectBorderCaption.bottom -= szSysBorder.cy;
			}

			m_ctrlAppBorderCaption.Draw (&memDC, rectBorderCaption, indexBorder);
		}
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
//*****************************************************************************
BOOL CBCGPVisualManager2007::OnNcPaint (CWnd* pWnd, const CObList& lstSysButtons, CRect rectRedraw)
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
				rectCaption.bottom += bIsSmallCaption ? ::GetSystemMetrics (SM_CYSMCAPTION) : ::GetSystemMetrics (SM_CYCAPTION);
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

			if (IsBeta ())
			{
				CRect rectBorder (m_ctrlMainBorderCaption.GetParams ().m_rectSides);

				COLORREF clr1  = bActive 
									? m_clrAppCaptionActiveStart 
									: m_clrAppCaptionInactiveStart;
				COLORREF clr2  = bActive 
									? m_clrAppCaptionActiveFinish 
									: m_clrAppCaptionInactiveFinish;

				CRect rectCaption2 (rectCaption);
				rectCaption2.DeflateRect (rectBorder.left, rectBorder.top, 
					rectBorder.right, rectBorder.bottom);

				{
					CBCGPDrawManager dm (dc);
					dm.Fill4ColorsGradient (rectCaption2, clr1, clr2, clr2, clr1, FALSE);
				}

				m_ctrlMainBorderCaption.DrawFrame (&dc, rectCaption, bActive ? 0 : 1);
			}
			else
			{
				m_ctrlMainBorderCaption.Draw (&dc, rectCaption, bActive ? 0 : 1);
			}
		}
#endif // BCGP_EXCLUDE_RIBBON

		rtWindow.top = rectCaption.bottom;

		dc.ExcludeClipRect (rectCaption);

		if (pWnd->IsKindOf (RUNTIME_CLASS (CMDIChildWnd)) ||
			(bDialog && !m_ctrlDialogBorder.IsValid ()))
		{
			if (bDialog)
			{
				CRect rtDialog (rtWindow);
				rtDialog.DeflateRect (1, 0, 1, 1);
				dc.FillRect (rtDialog, &GetDlgBackBrush (pWnd));

				dc.ExcludeClipRect (rtDialog);
			}

			m_ctrlMDIChildBorder.DrawFrame (&dc, rtWindow, bActive ? 0 : 1);
		}
		else if (bDialog)
		{
			m_ctrlDialogBorder.DrawFrame (&dc, rtWindow, bActive ? 0 : 1);
		}
		else
		{
			m_ctrlMainBorder.DrawFrame (&dc, rtWindow, bActive ? 0 : 1);
		}

		if (bDialog)
		{
			dc.SelectClipRgn (NULL);
			return TRUE;
		}

		//-------------------------------
		// Find status bar extended area:
		//-------------------------------
		CRect rectExt (0, 0, 0, 0);
		BOOL bExtended    = FALSE;
		BOOL bBottomFrame = FALSE;
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
#ifndef BCGP_EXCLUDE_RIBBON
			else
			{
				CBCGPRibbonStatusBar* pRibbonStatusBar = DYNAMIC_DOWNCAST (
					CBCGPRibbonStatusBar, pStatusBar);
				if (pRibbonStatusBar != NULL)
				{
					bExtended    = pRibbonStatusBar->GetExtendedArea (rectExt);
					bBottomFrame = pRibbonStatusBar->IsBottomFrame ();
					bIsStatusBar = TRUE;
				}
			}
#endif // BCGP_EXCLUDE_RIBBON
		}

		if (bIsStatusBar)
		{
			CRect rectStatus;
			pStatusBar->GetClientRect (rectStatus);

			int nHeight = rectStatus.Height ();
			rectStatus.bottom = rtWindow.bottom;
			rectStatus.top    = rectStatus.bottom - nHeight - (bBottomFrame ? 0 : szSysBorder.cy);
			rectStatus.left   = rtWindow.left;
			rectStatus.right  = rtWindow.right;

			if (bExtended)
			{
				rectExt.left   = rectStatus.right - rectExt.Width () - szSysBorder.cx;
				rectExt.top    = rectStatus.top;
				rectExt.bottom = rectStatus.bottom;
				rectExt.right  = rtWindow.right;
			}

			m_ctrlStatusBarBack.Draw (&dc, rectStatus, bActive ? 0 : 1);

			if (bExtended)
			{
				rectExt.left -= m_ctrlStatusBarBack_Ext.GetParams ().m_rectCorners.left;

				m_ctrlStatusBarBack_Ext.Draw (&dc, rectExt, bActive ? 0 : 1);
			}
		}

		dc.SelectClipRgn (NULL);

		return TRUE;
	}

	return CBCGPVisualManager2003::OnNcPaint (pWnd, lstSysButtons, rectRedraw);
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::OnSetWindowRegion (CWnd* pWnd, CSize sizeWindow)
{
	ASSERT_VALID (pWnd);

	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		return CBCGPVisualManager2003::OnSetWindowRegion(pWnd, sizeWindow);
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

	if (DYNAMIC_DOWNCAST (CBCGPPopupMenu, pWnd) != NULL)
	{
		sz  = CSize (3, 3);
	}
#ifndef BCGP_EXCLUDE_RIBBON
	else if (DYNAMIC_DOWNCAST (CBCGPRibbonBar, pWnd) != NULL)
	{
		return FALSE;
	}
#endif
	else
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
				CRgn rgnWindow;
				rgnWindow.CreateRectRgn (0, sz.cy, sizeWindow.cx, sizeWindow.cy);

				rgn.CombineRgn (&rgn, &rgnWindow, RGN_OR);
			}

			pWnd->SetWindowRgn ((HRGN)rgn.Detach (), TRUE);
			return TRUE;
		}
	}

	return FALSE;
}
//*****************************************************************************
CSize CBCGPVisualManager2007::GetNcBtnSize (BOOL bSmall) const
{
	return m_szNcBtnSize[bSmall ? 1 : 0];
}
//*****************************************************************************
void CBCGPVisualManager2007::DrawSeparator (CDC* pDC, const CRect& rect, BOOL bHorz)
{
    DrawSeparator (pDC, rect, m_penSeparator, m_penSeparator2, bHorz);
}
//*****************************************************************************
void CBCGPVisualManager2007::DrawSeparator (CDC* pDC, const CRect& rect, CPen& pen1, CPen& pen2, BOOL bHorz)
{
    CRect rect1 (rect);
    CRect rect2;

    if (bHorz)
    {
        rect1.top += rect.Height () / 2 - 1;
        rect1.bottom = rect1.top;
        rect2 = rect1;
        rect2.OffsetRect (0, 1);
    }
    else
    {
        rect1.left += rect.Width () / 2 - 1;
        rect1.right = rect1.left;
        rect2 = rect1;
        rect2.OffsetRect (1, 0);
    }

	if (CBCGPToolBarImages::m_bIsDrawOnGlass)
	{
		CBCGPDrawManager dm (*pDC);

		LOGPEN logpen;

		pen1.GetLogPen (&logpen);
		dm.DrawLine (rect1.left, rect1.top, rect1.right, rect1.bottom, logpen.lopnColor);

		pen2.GetLogPen (&logpen);
		dm.DrawLine (rect2.left, rect2.top, rect2.right, rect2.bottom, logpen.lopnColor);
	}
	else
	{
		CPen* pOldPen = pDC->SelectObject (&pen1);
		pDC->MoveTo (rect1.TopLeft ());
		pDC->LineTo (rect1.BottomRight ());

		pDC->SelectObject (&pen2);
		pDC->MoveTo (rect2.TopLeft ());
		pDC->LineTo (rect2.BottomRight ());

		pDC->SelectObject (pOldPen);
	}
}
//**********************************************************************************
CBrush& CBCGPVisualManager2007::GetDlgButtonsAreaBrush(CWnd* pDlg, COLORREF* pclrLine)
{
	if (!CanDrawImage())
	{
		return CBCGPVisualManager2003::GetDlgButtonsAreaBrush (pDlg, pclrLine);
	}

	if (pclrLine != NULL)
	{
		*pclrLine = globalData.clrBarDkShadow;
	}

	return m_brDlgButtonsArea;
}
//*****************************************************************************
CFont& CBCGPVisualManager2007::GetNcCaptionTextFont()
{
	if (!CanDrawImage())
	{
		return CBCGPVisualManager2003::GetNcCaptionTextFont ();
	}

	return m_AppCaptionFont;
}
//*****************************************************************************
COLORREF CBCGPVisualManager2007::GetNcCaptionTextColor(BOOL bActive, BOOL bTitle) const
{
	if (!CanDrawImage())
	{
		return CBCGPVisualManager2003::GetNcCaptionTextColor (bActive, bTitle);
	}

	if (bTitle)
	{
		return bActive ? m_clrAppCaptionActiveTitleText : m_clrAppCaptionInactiveTitleText;
	}

	return bActive ? m_clrAppCaptionActiveText : m_clrAppCaptionInactiveText;
}
//**********************************************************************************
COLORREF CBCGPVisualManager2007::GetCaptionBarTextColor (CBCGPCaptionBar* pBar)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetCaptionBarTextColor (pBar);
	}

	return pBar->IsMessageBarMode () ? m_clrCaptionBarTextMessage : m_clrCaptionBarText;
}
//**************************************************************************************
void CBCGPVisualManager2007::OnDrawCaptionBarInfoArea (CDC* pDC, CBCGPCaptionBar* pBar, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawCaptionBarInfoArea (pDC, pBar, rect);
		return;
	}

	ASSERT_VALID (pDC);

	COLORREF clrFill = pBar->m_clrBarBackground != -1 ? pBar->m_clrBarBackground : globalData.clrBarFace;
	COLORREF clrGradient = pBar->m_clrBarBackground != -1 ? CBCGPDrawManager::PixelAlpha (pBar->m_clrBarBackground, 150) : RGB (255, 255, 255);
	COLORREF clrBorder = pBar->m_clrBarBackground != -1 ? CBCGPDrawManager::PixelAlpha (pBar->m_clrBarBackground, 80) : globalData.clrBarDkShadow;

	CBCGPDrawManager dm (*pDC);

	dm.FillGradient (rect, clrFill, clrGradient, TRUE);
	pDC->Draw3dRect(rect, clrBorder, clrBorder);
}
//***************************************************************************************
void CBCGPVisualManager2007::OnFillOutlookPageButton (CDC* pDC, 
												  const CRect& rect,
												  BOOL bIsHighlighted, BOOL bIsPressed,
												  COLORREF& clrText)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillOutlookPageButton (pDC, rect, bIsHighlighted, bIsPressed, clrText);
		return;
	}

	ASSERT_VALID (pDC);

	CRect rt (rect);

	if (m_ctrlOutlookWndPageBtn.IsValid ())
	{
		int index = 0;

		if (bIsPressed)
		{
			index = 2;

			if (bIsHighlighted)
			{
				index = 3;
			}

			clrText = m_clrOutlookPageTextPressed;
		}
		else if (bIsHighlighted)
		{
			index = 1;

			clrText = m_clrOutlookPageTextHighlighted;
		}

		m_ctrlOutlookWndPageBtn.Draw (pDC, rt, index);
	}
	else
	{
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
				clr1 = m_clrHighlightCheckedGradientLight;
				clr2 = m_clrHighlightCheckedGradientDark;
			}
		}
		else if (bIsHighlighted)
		{
			clr1 = m_clrHighlightGradientDark;
			clr2 = m_clrHighlightGradientLight;
		}

		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rect, clr1, clr2, TRUE);
	}

	clrText = m_clrOutlookPageTextNormal;

	if (bIsPressed)
	{
		clrText = m_clrOutlookPageTextPressed;
	}
	else if (bIsHighlighted)
	{
		clrText = m_clrOutlookPageTextHighlighted;
	}
}
//****************************************************************************************
void CBCGPVisualManager2007::OnDrawOutlookPageButtonBorder (
	CDC* pDC, CRect& rectBtn, BOOL bIsHighlighted, BOOL bIsPressed)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawOutlookPageButtonBorder (pDC, rectBtn, bIsHighlighted, bIsPressed);
		return;
	}

	pDC->Draw3dRect (rectBtn, globalData.clrBtnHilite, m_clrToolBarBottomLine);
}
//*******************************************************************************
void CBCGPVisualManager2007::OnDrawOutlookBarSplitter (CDC* pDC, CRect rectSplitter)
{
	ASSERT_VALID (pDC);

	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawOutlookBarSplitter (pDC, rectSplitter);
		return;
	}

	if (!m_ctrlOutlookSplitter.IsValid ())
	{
		CBCGPDrawManager dm (*pDC);

		dm.FillGradient (rectSplitter,
						m_clrCaptionBarGradientDark,
						m_clrCaptionBarGradientLight,
						TRUE);

		rectSplitter.OffsetRect (0, 1);
		m_ToolBarTear.DrawEx (pDC, rectSplitter, 0, CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
		rectSplitter.OffsetRect (0, -1);

		CPen* pOldPen = pDC->SelectObject (&m_penBottomLine);

		pDC->MoveTo (rectSplitter.left, rectSplitter.top);
		pDC->LineTo (rectSplitter.right, rectSplitter.top);

		pDC->MoveTo (rectSplitter.left, rectSplitter.bottom - 1);
		pDC->LineTo (rectSplitter.right, rectSplitter.bottom - 1);

		pDC->SelectObject (pOldPen);
	}
	else
	{
		m_ctrlOutlookSplitter.Draw (pDC, rectSplitter);
	}
}
//*******************************************************************************
void CBCGPVisualManager2007::OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, COLORREF& clrText)
{
	CBCGPVisualManager2003::OnFillOutlookBarCaption (pDC, rectCaption, clrText);

	if (CanDrawImage ())
	{
		clrText = m_clrOutlookCaptionTextNormal;
	}
}
//*******************************************************************************
void CBCGPVisualManager2007::OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
						CRect rectClient, CRect rectClip,
						BOOL bNCArea/* = FALSE*/)
{
	ASSERT_VALID (pBar);

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

	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillBarBackground (pDC, pBar, rectClient, rectClip, bNCArea);
		return;
	}

    CRuntimeClass* pBarClass = pBar->GetRuntimeClass ();

	BOOL bIsCalculator = pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPCalculator));
	BOOL bIsColorBar = pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPColorBar));

	if (pBar->IsDialogControl () || pBarClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPCalendarBar)))
	{
		CBCGPVisualManager2003::OnFillBarBackground (pDC, pBar, rectClient, rectClip, bNCArea);
		return;
	}

	if (bIsCalculator || (bIsColorBar && DYNAMIC_DOWNCAST(CDialog, pBar->GetParent()) != NULL))
	{
		if (m_brDlgBackground.GetSafeHandle() != NULL)
		{
			pDC->FillRect(rectClient, &m_brDlgBackground);
		}
		else
		{
			CBCGPVisualManager2003::OnFillBarBackground (pDC, pBar, rectClient, rectClip, bNCArea);
		}
		return;
	}

    if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar)))
    {
	    BOOL bIsHorz = (pBar->GetBarStyle () & CBRS_ORIENT_HORZ);
	    COLORREF clr1 = bIsHorz ? m_clrMenuBarGradientDark : m_clrMenuBarGradientVertLight;
	    COLORREF clr2 = bIsHorz ? m_clrMenuBarGradientLight : m_clrMenuBarGradientVertDark;

        CBCGPDrawManager dm (*pDC);
        dm.Fill4ColorsGradient (rectClient, clr1, clr2, clr2, clr1, !bIsHorz);
        return;
    }
    else if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar)))
    {
		pDC->FillRect (rectClient, &m_brMenuLight);

		CBCGPPopupMenuBar* pMenuBar = DYNAMIC_DOWNCAST (CBCGPPopupMenuBar, pBar);
		if (!pMenuBar->m_bDisableSideBarInXPMode && !bIsColorBar)
		{
			CRect rectGutter = rectClient;

			rectGutter.right = rectGutter.left + pMenuBar->GetGutterWidth ();

			if (!pMenuBar->HasGutterLogo())
			{
				rectGutter.DeflateRect (0, 1);
			}

            pDC->FillRect (rectGutter, &m_brBarBkgnd);

			if (!pMenuBar->HasGutterLogo())
			{
				rectGutter.left = rectGutter.right;
				rectGutter.right += 2;
			}
			else
			{
				rectGutter.left = rectGutter.right - 2;
			}

            DrawSeparator (pDC, rectGutter, FALSE);
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

		CWnd* pWnd = ((CBCGPStatusBar*)pBar)->GetParent ();
		ASSERT_VALID (pWnd);

		BOOL bActive = IsWindowActive (pWnd);

		rect.InflateRect (szSysBorder.cx, 0, szSysBorder.cx, szSysBorder.cy);
		m_ctrlStatusBarBack.Draw (pDC, rect, bActive ? 0 : 1);

		if (bExtended)
		{
			rectExt.InflateRect (0, 0, szSysBorder.cx, szSysBorder.cy);
			rectExt.left -= m_ctrlStatusBarBack_Ext.GetParams ().m_rectCorners.left;
			m_ctrlStatusBarBack_Ext.Draw (pDC, rectExt, bActive ? 0 : 1);
		}

        return;
    }
#ifndef BCGP_EXCLUDE_RIBBON
    else if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPRibbonStatusBar)))
    {
		CBCGPRibbonStatusBar* pRibbonStatusBar = DYNAMIC_DOWNCAST (
			CBCGPRibbonStatusBar, pBar);

		CSize szSysBorder (globalUtils.GetSystemBorders (pBar->GetParent()));
		if (szSysBorder.cx == 0 && szSysBorder.cy == 0)
		{
			szSysBorder = globalUtils.GetSystemBorders (BCGCBProGetTopLevelFrame(pBar));
		}

		CRect rect (rectClient);
		CRect rectExt (0, 0, 0, 0);

		BOOL bExtended    = pRibbonStatusBar->GetExtendedArea (rectExt);
		BOOL bBottomFrame = pRibbonStatusBar->IsBottomFrame ();

		if (bExtended)
		{
			rect.right = rectExt.left;
		}

		CWnd* pWnd = pBar->GetParent ();
		ASSERT_VALID (pWnd);

		BOOL bActive = IsWindowActive (pWnd);

		rect.InflateRect (szSysBorder.cx, 0, szSysBorder.cx, bBottomFrame ? 0 : szSysBorder.cy);
		m_ctrlStatusBarBack.Draw (pDC, rect, bActive ? 0 : 1);

		if (bExtended)
		{
			rectExt.InflateRect (0, 0, szSysBorder.cx, bBottomFrame ? 0 : szSysBorder.cy);
			rectExt.left -= m_ctrlStatusBarBack_Ext.GetParams ().m_rectCorners.left;
			m_ctrlStatusBarBack_Ext.Draw (pDC, rectExt, bActive ? 0 : 1);
		}

        return;
    }
#endif
	else if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPOutlookBarToolBar)))
	{
		if (m_ctrlOutlookWndBar.IsValid ())
		{
			m_ctrlOutlookWndBar.Draw (pDC, rectClient);
		}
		else
		{
			CBCGPDrawManager dm (*pDC);
			dm.FillGradient (rectClient,	m_clrToolBarGradientDark,
											m_clrToolBarGradientLight,
											TRUE);
		}

		return;
	}

#if !defined(BCGP_EXCLUDE_TOOLBOX) && !defined(BCGP_EXCLUDE_TASK_PANE)
	BOOL bIsToolBox =	pBar->IsKindOf (RUNTIME_CLASS (CBCGPToolBoxPage)) ||
						pBar->IsKindOf (RUNTIME_CLASS (CBCGPToolBox)) ||
						pBar->IsKindOf (RUNTIME_CLASS (CBCGPToolBoxEx));

	if (bIsToolBox)
	{
			CBCGPDrawManager dm (*pDC);
			dm.FillGradient (rectClient,	m_clrToolBarGradientLight,
											m_clrToolBarGradientDark,
											FALSE);
		return;
	}

#endif

	CBCGPVisualManager2003::OnFillBarBackground (pDC, pBar, rectClient, rectClip, bNCArea);
}
//***********************************************************************************
void CBCGPVisualManager2007::OnFillSpinButton (CDC* pDC, CBCGPSpinButtonCtrl* pSpinCtrl, CRect rect, BOOL bDisabled)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillSpinButton(pDC, pSpinCtrl, rect, bDisabled);
		return;
	}

	COLORREF color1 = bDisabled ? m_clrComboBtnDisabledStart : m_clrComboBtnStart;
	COLORREF color2 = bDisabled ? m_clrComboBtnDisabledFinish : m_clrComboBtnFinish;
	COLORREF colorBorder = bDisabled ? m_clrComboBtnBorderDisabled : m_clrComboBtnBorder;

	if (colorBorder != (COLORREF)(-1))
	{
		if (CBCGPToolBarImages::m_bIsDrawOnGlass)
		{
			CBCGPDrawManager dm (*pDC);
			dm.DrawRect (rect, (COLORREF)-1, colorBorder);
		}
		else
		{
			pDC->Draw3dRect (rect, colorBorder, colorBorder);
		}

		rect.DeflateRect (1, 1, 1, 1);
	}

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, color1, color2, TRUE);
}
//*****************************************************************************
void CBCGPVisualManager2007::OnFillHighlightedArea (CDC* pDC, CRect rect, 
							CBrush* pBrush, CBCGPToolbarButton* pButton)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillHighlightedArea (pDC, rect, pBrush, pButton);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pBrush);

	BOOL bIsHorz = TRUE;

	COLORREF clr1 = (COLORREF)-1;
	COLORREF clr2 = (COLORREF)-1;

	if (pButton != NULL)
	{
		ASSERT_VALID (pButton);

		bIsHorz = pButton->IsHorizontal ();

		CBCGPToolbarMenuButton* pCustButton = 
			DYNAMIC_DOWNCAST (CCustomizeButton, pButton);

		if (pCustButton != NULL)
		{
			if (pButton->IsDroppedDown ())
			{
				clr1 = m_clrHighlightDnGradientDark;
				clr2 = m_clrHighlightDnGradientLight;
			}
		}
	}

	if (pBrush == &m_brHighlight)
	{
		clr1 = m_clrHighlightGradientDark;
		clr2 = m_clrHighlightGradientLight;//bIsPopupMenu ? clr1 : m_clrHighlightGradientLight;
	}
	else if (pBrush == &m_brHighlightDn)
	{
		clr1 = m_clrHighlightDnGradientDark;//bIsPopupMenu ? m_clrHighlightDnGradientLight : m_clrHighlightDnGradientDark;
		clr2 = m_clrHighlightDnGradientLight;
	}
	else if (pBrush == &m_brHighlightChecked)
	{
		clr1 = m_clrHighlightCheckedGradientDark;//bIsPopupMenu ? m_clrHighlightCheckedGradientLight : m_clrHighlightCheckedGradientDark;
		clr2 = m_clrHighlightCheckedGradientLight;
	}

	if (clr1 == (COLORREF)-1 || clr2 == (COLORREF)-1)
	{
		CBCGPVisualManager2003::OnFillHighlightedArea (pDC, rect, pBrush, pButton);
		return;
	}

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, clr1, clr2, bIsHorz);
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawMenuBorder (CDC* pDC, CBCGPPopupMenu* pMenu, CRect rect)
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
			ASSERT_VALID (pRibbonMenuBar);

			if (pRibbonMenuBar->IsMainPanel ())
			{
				if (m_ctrlRibbonMainPanel.IsValid ())
				{
					m_ctrlRibbonMainPanel.DrawFrame (pDC, rect);
				}
				else
				{
					m_ctrlPopupBorder.DrawFrame (pDC, rect);
				}

				return;
			}

			if (!pRibbonMenuBar->IsMenuMode ())
			{
				if (pRibbonMenuBar->IsQATPopup () &&
					m_ctrlRibbonBorder_QAT.IsValid ())
				{
					m_ctrlRibbonBorder_QAT.DrawFrame (pDC, rect);
					return;
				}
				else if (pRibbonMenuBar->IsCategoryPopup ())
				{
					if (IsBeta1 ())
					{
						m_ctrlRibbonCategoryBack.DrawFrame (pDC, rect);
					}

					return;
				}
				else if (pRibbonMenuBar->IsFloaty () &&
						 m_ctrlRibbonBorder_Floaty.IsValid ())
				{
					m_ctrlRibbonBorder_Floaty.DrawFrame (pDC, rect);
					return;
				}
				else
				{
					if (pRibbonMenuBar->GetPanel () != NULL)
					{
						if (IsBeta1 ())
						{
							m_ctrlRibbonCategoryBack.DrawFrame (pDC, rect);
						}
						else if (m_ctrlRibbonBorder_Panel.IsValid ())
						{
							m_ctrlRibbonBorder_Panel.DrawFrame (pDC, rect);
						}

						return;
					}

					// draw standard
				}
			}
		}
	}
#endif

	CBCGPBaseControlBar* pTopLevelBar = NULL;

	for (CBCGPPopupMenu* pParentMenu = pMenu; 
		pParentMenu != NULL; pParentMenu = pParentMenu->GetParentPopupMenu ())
	{
		CBCGPToolbarMenuButton* pParentButton = pParentMenu->GetParentButton ();
		if (pParentButton == NULL)
		{
			break;
		}
	
		pTopLevelBar = 
			DYNAMIC_DOWNCAST (CBCGPBaseControlBar, pParentButton->GetParentWnd ());
	}

	if (pTopLevelBar == NULL || pTopLevelBar->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar)))
	{
		m_ctrlPopupBorder.DrawFrame (pDC, rect);
	}
	else
	{
		CBCGPVisualManager2003::OnDrawMenuBorder (pDC, pMenu, rect);
	}
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CBCGPBaseControlBar* pBar)
{
	if (!CanDrawImage () ||
		(pBar != NULL && pBar->IsDialogControl ()) ||
        m_ToolBarGripper.GetCount () == 0)
	{
		CBCGPVisualManager2003::OnDrawBarGripper (pDC, rectGripper, bHorz, pBar);
		return;
	}

    CSize szBox (m_ToolBarGripper.GetImageSize ());

    if (szBox != CSize (0, 0))
    {
		if (bHorz)
		{
			rectGripper.left = rectGripper.right - szBox.cx;
		}
		else
		{
			rectGripper.top = rectGripper.bottom - szBox.cy;
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
			(rectGripper.Height () - szBox.cy) / szBox.cy : 
			(rectGripper.Width () - szBox.cx) / szBox.cx;

		int nOffset = bHorz ? 
			(rectGripper.Height () - nBoxesNumber * szBox.cy) / 2 :
			(rectGripper.Width () - nBoxesNumber * szBox.cx) / 2;

		for (int nBox = 0; nBox < nBoxesNumber; nBox++)
		{
			int x = bHorz ? 
				rectGripper.left : 
				rectGripper.left + nOffset;

			int y = bHorz ? 
				rectGripper.top + nOffset : 
				rectGripper.top;

            m_ToolBarGripper.DrawEx (pDC, CRect (CPoint (x, y), szBox), 0);

			nOffset += bHorz ? szBox.cy : szBox.cx;
		}
	}
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawSeparator (CDC* pDC, CBCGPBaseControlBar* pBar, CRect rect, BOOL bHorz)
{
	ASSERT_VALID (pDC);

	if (!CanDrawImage () || pBar == NULL || pBar->IsDialogControl ())
	{
		CBCGPVisualManager2003::OnDrawSeparator (pDC, pBar, rect, bHorz);
		return;
	}

	ASSERT_VALID (pBar);

	CRect rectSeparator (rect);

#ifndef BCGP_EXCLUDE_RIBBON
	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPRibbonStatusBar)))
	{
		CBCGPRibbonStatusBar* pRibbonStatusBar = DYNAMIC_DOWNCAST (
			CBCGPRibbonStatusBar, pBar);

		rect.InflateRect (1, 5, 1, pRibbonStatusBar->IsBottomFrame () ? 2 : 5);

		m_StatusBarPaneBorder.DrawEx (pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzRight, 
			CBCGPToolBarImages::ImageAlignVertStretch);
		return;
	}

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPRibbonBar)) ||
		(bHorz && pBar->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPanelMenuBar))))
	{
		if (rect.Width () < m_RibbonPanelSeparator.GetImageSize ().cx)
		{
			rect.left = rect.right - m_RibbonPanelSeparator.GetImageSize ().cx;
		}
		
		m_RibbonPanelSeparator.DrawEx (pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzCenter,
			CBCGPToolBarImages::ImageAlignVertCenter);
		return;
	}
#endif

	BOOL bPopupMenu = FALSE;

	if (!bHorz)
	{
		BOOL bIsRibbon = FALSE;

#ifndef BCGP_EXCLUDE_RIBBON
		bIsRibbon = pBar->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPanelMenuBar));

		if (bIsRibbon && ((CBCGPRibbonPanelMenuBar*) pBar)->IsDefaultMenuLook ())
		{
			bIsRibbon = FALSE;
		}
#endif
		bPopupMenu = pBar->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar));

		if (bPopupMenu &&
			!bIsRibbon &&
			!pBar->IsKindOf (RUNTIME_CLASS (CBCGPColorBar)) && 
			!pBar->IsKindOf (RUNTIME_CLASS (CBCGPCalculator)) && 
			!pBar->IsKindOf (RUNTIME_CLASS (CBCGPCalendarBar)))
		{
			rectSeparator.left = rect.left + CBCGPToolBar::GetMenuImageSize ().cx + 
						GetMenuImageMargin () + 1;

			CRect rectBar;
			pBar->GetClientRect (rectBar);

			if (rectBar.right - rectSeparator.right < 50) // Last item in row
			{
				rectSeparator.right = rectBar.right;
			}

			if (((CBCGPPopupMenuBar*) pBar)->m_bDisableSideBarInXPMode)
			{
				rectSeparator.left = 0;
			}

			//---------------------------------
			//	Maybe Quick Customize separator
			//---------------------------------
			if (bPopupMenu)
			{
				CWnd* pWnd = pBar->GetParent();
				if (pWnd != NULL && pWnd->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenu)))
				{
					CBCGPPopupMenu* pMenu = (CBCGPPopupMenu*)pWnd;
					if (pMenu->IsCustomizePane())
					{
						rectSeparator.left = rect.left + 2 * CBCGPToolBar::GetMenuImageSize ().cx + 
								3 * GetMenuImageMargin () + 2;
					}
				}
			}
		}
	}

	if (bPopupMenu)
	{
		DrawSeparator (pDC, rectSeparator, !bHorz);
	}
	else
	{
		if (bHorz)
		{
			int nHeight = rectSeparator.Height () / 5;
			rectSeparator.top    += nHeight;
			rectSeparator.bottom -= nHeight;
		}
		else
		{
			int nWidth = rectSeparator.Width () / 5;
			rectSeparator.left  += nWidth;
			rectSeparator.right -= nWidth;
		}

		DrawSeparator (pDC, rectSeparator, m_penSeparatorDark, m_penSeparatorLight, !bHorz);
	}
}
//*****************************************************************************
COLORREF CBCGPVisualManager2007::OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
		BOOL bActive, CRect rectCaption, CRect rectButtons)
{
	if (!CanDrawImage () || pBar == NULL || pBar->IsDialogControl ())
	{
		return CBCGPVisualManager2003::OnDrawControlBarCaption (pDC, pBar, bActive, rectCaption, rectButtons);
	}

	ASSERT_VALID (pDC);

	CPen pen (PS_SOLID, 1, globalData.clrBarFace);
	CPen* pOldPen = pDC->SelectObject (&pen);

	rectCaption.bottom += 2;

	pDC->MoveTo (rectCaption.left, rectCaption.bottom);
	pDC->LineTo (rectCaption.left, rectCaption.top);

	pDC->MoveTo (rectCaption.left  + 1, rectCaption.top);
	pDC->LineTo (rectCaption.right - 1, rectCaption.top);

	pDC->MoveTo (rectCaption.right - 1, rectCaption.top + 1);
	pDC->LineTo (rectCaption.right - 1, rectCaption.bottom);

	pDC->SelectObject (pOldPen);

	rectCaption.DeflateRect (1, 1, 1, 0);

	COLORREF clr = bActive ? globalData.clrActiveCaption : globalData.clrInactiveCaption;
	COLORREF clrGradient = CBCGPDrawManager::PixelAlpha(clr, bActive ? 95 : 115);

	CBCGPDrawManager dm (*pDC);

	if (bActive)
	{
		dm.FillGradient (rectCaption, clr, clrGradient, TRUE);
	}
	else
	{
		dm.FillGradient (rectCaption, clrGradient, clr, TRUE);
	}

	return bActive ? globalData.clrCaptionText : globalData.clrInactiveCaptionText;
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawStatusBarPaneBorder (CDC* pDC, CBCGPStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawStatusBarPaneBorder (pDC, pBar, rectPane, 
								uiID, nStyle);
		return;
	}	

	BOOL bExtended = pBar->GetDrawExtendedArea ();
	if (!bExtended || ((nStyle & SBPS_STRETCH) == 0 && bExtended))
	{
		rectPane.OffsetRect (1, 0);
		m_StatusBarPaneBorder.DrawEx (pDC, rectPane, 0, CBCGPToolBarImages::ImageAlignHorzRight, 
			CBCGPToolBarImages::ImageAlignVertStretch);
	}
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawStatusBarSizeBox (CDC* pDC, CBCGPStatusBar* pStatBar,
			CRect rectSizeBox)
{
	if (!CanDrawImage () || 
        m_StatusBarSizeBox.GetCount () == 0)
	{
		CBCGPVisualManager2003::OnDrawStatusBarSizeBox (pDC, pStatBar, rectSizeBox);
		return;
	}

    m_StatusBarSizeBox.DrawEx (pDC, rectSizeBox, 0, CBCGPToolBarImages::ImageAlignHorzRight, CBCGPToolBarImages::ImageAlignVertBottom);
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawComboDropButton (CDC* pDC, CRect rect,
							BOOL bDisabled,
							BOOL bIsDropped,
							BOOL bIsHighlighted,
							CBCGPToolbarComboBoxButton* pButton)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawComboDropButton (pDC, rect,
												bDisabled, bIsDropped,
												bIsHighlighted, pButton);
		return;
	}

	const BOOL bIsCtrl = pButton != NULL && pButton->IsCtrlButton ();

	if (bIsCtrl)
	{
		int index = 0;

		if (bDisabled)
		{
			index = 6;
		}
		else if (bIsDropped)
		{
			index = 2;
		}
		else if (bIsHighlighted)
		{
			index++;
		}

		pDC->FillRect(rect, bDisabled ? &globalData.brBtnFace : &globalData.brWindow);
		m_ctrlRibbonBtnPush.Draw (pDC, rect, index);
	}
	else
	{
		const BOOL bRibbon = pButton != NULL && pButton->IsRibbonButton ();

		const BOOL bActive = bIsHighlighted || bIsDropped;

		CBCGPControlRenderer* pRenderer = bRibbon 
			? &m_ctrlRibbonComboBoxBtn
			: &m_ctrlComboBoxBtn;

		if (!pRenderer->IsValid ())
		{
			COLORREF color1 = bRibbon 
								? m_clrRibbonComboBtnStart
								: m_clrComboBtnStart;
			COLORREF color2 = bRibbon 
								? m_clrRibbonComboBtnFinish
								: m_clrComboBtnFinish;
			COLORREF colorBorder = bRibbon 
								? m_clrRibbonComboBtnBorder
								: m_clrComboBtnBorder;
			if (bDisabled)
			{
				color1 = bRibbon 
								? m_clrRibbonComboBtnDisabledStart
								: m_clrComboBtnDisabledStart;
				color2 = bRibbon 
								? m_clrRibbonComboBtnDisabledFinish
								: m_clrComboBtnDisabledFinish;
				colorBorder = bRibbon 
								? m_clrRibbonComboBtnBorderDisabled
								: m_clrComboBtnBorderDisabled;
			}
			else if (bActive)
			{
				if (bIsDropped)
				{
					color1 = bRibbon 
									? m_clrRibbonComboBtnPressedStart
									: m_clrComboBtnPressedStart;
					color2 = bRibbon 
									? m_clrRibbonComboBtnPressedFinish
									: m_clrComboBtnPressedFinish;
					colorBorder = bRibbon 
									? m_clrRibbonComboBtnBorderPressed
									: m_clrComboBtnBorderPressed;
				}
				else
				{
					color1 = bRibbon 
									? m_clrRibbonComboBtnHighlightedStart
									: m_clrComboBtnHighlightedStart;
					color2 = bRibbon 
									? m_clrRibbonComboBtnHighlightedFinish
									: m_clrComboBtnHighlightedFinish;
					colorBorder = bRibbon 
									? m_clrRibbonComboBtnBorderHighlighted
									: m_clrComboBtnBorderHighlighted;
				}
			}

			if (bRibbon || 
				!bDisabled || 
				(bDisabled && colorBorder != (COLORREF)(-1)))
			{
				if (!bDisabled)
				{
					rect.InflateRect (0, 1, 1, 1);
				}

				if (CBCGPToolBarImages::m_bIsDrawOnGlass)
				{
					CBCGPDrawManager dm (*pDC);
					dm.DrawRect (rect, (COLORREF)-1, colorBorder);
				}
				else
				{
					pDC->Draw3dRect (rect, colorBorder, colorBorder);
				}

				if (!bDisabled)
				{
					rect.DeflateRect (0, 1, 1, 1);
				}
			}

			if (bDisabled)
			{
				rect.DeflateRect (0, 1, 1, 1);
			}
			else if (bActive)
			{
				rect.DeflateRect (1, 0, 0, 0);
			}

			CBCGPDrawManager dm (*pDC);
			dm.FillGradient (rect, color1, color2, TRUE);

			if (bDisabled)
			{
				rect.InflateRect (0, 1, 1, 1);
			}
			else if (bActive)
			{
				rect.InflateRect (1, 0, 0, 0);
			}
		}
		else
		{
			rect.InflateRect (0, 1, 1, 1);

			int nIndex = (bRibbon && pButton->IsRibbonFloaty ()) ? 5 : 0;
			if (bDisabled)
			{
				nIndex = 3;
			}
			else
			{
				if (bIsDropped)
				{
					nIndex = 2;
				}
				else if (bIsHighlighted)
				{
					nIndex = 1;
				}
			}

			pRenderer->Draw (pDC, rect, nIndex);

			rect.DeflateRect (0, 1, 1, 1);
		}

		rect.bottom -= 2;
	}

	CBCGPMenuImages::Draw (pDC, CBCGPMenuImages::IdArowDown, rect,
		bDisabled 
		? CBCGPMenuImages::ImageGray 
		: CBCGPMenuImages::ImageBlack);
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawComboBorder (CDC* pDC, CRect rect,
							BOOL bDisabled,
							BOOL bIsDropped,
							BOOL bIsHighlighted,
							CBCGPToolbarComboBoxButton* pButton)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawComboBorder (pDC, rect,
												bDisabled,
												bIsDropped,
												bIsHighlighted,
												pButton);
		return;
	}

	rect.DeflateRect (1, 1);

    COLORREF colorBorder = m_clrComboBorder;

	if (bDisabled)
	{
		colorBorder = m_clrComboBorderDisabled;
	}
	else if (bIsHighlighted || bIsDropped)
	{
		colorBorder = bIsDropped
			? m_clrComboBorderPressed
			: m_clrComboBorderHighlighted;
	}

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
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawEditBorder (CDC* pDC, CRect rect,
							BOOL bDisabled,
							BOOL bIsHighlighted,
							CBCGPToolbarEditBoxButton* pButton)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawEditBorder (pDC, rect,
												bDisabled,
												bIsHighlighted,
												pButton);
		return;
	}

	rect.DeflateRect (1, 1);

    COLORREF colorBorder = m_clrEditBorder;

	if (bDisabled)
	{
		colorBorder = m_clrEditBorderDisabled;
	}
	else if (bIsHighlighted)
	{
		colorBorder = m_clrEditBorderHighlighted;
	}

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
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive)
{
	if (!CanDrawImage () || 
        m_ToolBarTear.GetCount () == 0)
	{
		CBCGPVisualManager2003::OnDrawTearOffCaption (pDC, rect, bIsActive);
		return;
	}

    pDC->FillRect (rect, &m_brBarBkgnd);	
    if (bIsActive)
    {
		m_ctrlMenuHighlighted[0].Draw (pDC, rect);
    }

    m_ToolBarTear.DrawEx (pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
}
//***********************************************************************************
void CBCGPVisualManager2007::OnDrawMenuResizeBar (CDC* pDC, CRect rect, int nResizeFlags)
{
	CBCGPToolBarImages& images =
		(nResizeFlags == (int) CBCGPPopupMenu::MENU_RESIZE_BOTTOM_RIGHT) ?
	m_PopupResizeBar_HV :
		(nResizeFlags == (int) CBCGPPopupMenu::MENU_RESIZE_TOP_RIGHT) ?
	m_PopupResizeBar_HVT :
	m_PopupResizeBar_V;	// TODO - vertical resize

	if (!CanDrawImage () ||
		!m_ctrlPopupResizeBar.IsValid () ||
		!images.IsValid ())
	{
		CBCGPVisualManager2003::OnDrawMenuResizeBar (pDC, rect, nResizeFlags);
		return;
	}

	ASSERT_VALID (pDC);

	m_ctrlPopupResizeBar.Draw (pDC, rect);

	if (nResizeFlags == (int) CBCGPPopupMenu::MENU_RESIZE_BOTTOM_RIGHT ||
		nResizeFlags == (int) CBCGPPopupMenu::MENU_RESIZE_TOP_RIGHT)
	{
		images.DrawEx (pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzRight,
			nResizeFlags == (int) CBCGPPopupMenu::MENU_RESIZE_TOP_RIGHT ?
				CBCGPToolBarImages::ImageAlignVertTop :
				CBCGPToolBarImages::ImageAlignVertBottom);
	}
	else
	{
		images.DrawEx (pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzCenter,
			CBCGPToolBarImages::ImageAlignVertCenter);
	}
}
//***********************************************************************************
void CBCGPVisualManager2007::OnDrawMenuScrollButton (CDC* pDC, CRect rect, BOOL bIsScrollDown, 
												 BOOL bIsHighlited, BOOL bIsPressed,
												 BOOL bIsDisabled)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawMenuScrollButton (pDC, rect, bIsScrollDown, bIsHighlited, bIsPressed, bIsDisabled);
		return;
	}

	ASSERT_VALID (pDC);

	CBCGPControlRenderer* pRenderer = &m_ctrlMenuScrollBtn[0];

	if (bIsScrollDown && m_ctrlMenuScrollBtn[1].IsValid ())
	{
		pRenderer = &m_ctrlMenuScrollBtn[1];
	}

	rect.top --;

	pRenderer->Draw (pDC, rect, bIsHighlited ? 1 : 0);

	CBCGPMenuImages::Draw (pDC, bIsScrollDown ? CBCGPMenuImages::IdArowDown : CBCGPMenuImages::IdArowUp, rect);
}
//***********************************************************************************
void CBCGPVisualManager2007::OnDrawMenuSystemButton (CDC* pDC, CRect rect, 
												UINT uiSystemCommand, 
												UINT nStyle, BOOL bHighlight)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawMenuSystemButton (pDC, rect, 
												uiSystemCommand, nStyle, bHighlight);
		return;
	}

	ASSERT_VALID (pDC);

	CBCGPToolBarImages* pImage = NULL;

	int nIndex = (IsSmallMenuSystemButton () && m_SysBtnClose[1].IsValid ()) ? 1 : 0;

	switch (uiSystemCommand)
	{
	case SC_CLOSE:
		pImage = &m_SysBtnClose[nIndex];
		break;

	case SC_MINIMIZE:
		pImage = &m_SysBtnMinimize[nIndex];
		break;

	case SC_RESTORE:
		pImage = &m_SysBtnRestore[nIndex];
		break;

	default:
		return;
	}

	BOOL bDisabled = (nStyle & TBBS_DISABLED);
	BOOL bPressed = (nStyle & TBBS_PRESSED);

	CRect rtBtnImage (CPoint (0, 0), pImage->GetImageSize ());

	int nImage = 0;
	if (bDisabled)
	{
		nImage = 3;
	}
	else if (bPressed || bHighlight)
	{
		int index = -1;
		if (bPressed)
		{
			if (bHighlight)
			{
				index = 1;
			}
		}
		else if (bHighlight)
		{
			index = 0;
		}

		if (index != -1 && m_ctrlRibbonBtn[0].IsValid ())
		{
			m_ctrlRibbonBtn[0].Draw (pDC, rect, index);
		}	
	}

	rtBtnImage.OffsetRect (0, pImage->GetImageSize ().cy * nImage);
	pImage->DrawEx (pDC, rect, 0, 
		CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter, 
		rtBtnImage);
}
//*****************************************************************************
void CBCGPVisualManager2007::OnFillButtonInterior (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillButtonInterior (pDC, pButton, rect, state);
		return;
	}

	CCustomizeButton* pCustButton = DYNAMIC_DOWNCAST (CCustomizeButton, pButton);

	if (pCustButton == NULL)
	{
		if (CBCGPToolBar::IsCustomizeMode () && 
			!CBCGPToolBar::IsAltCustomizeMode () && !pButton->IsLocked ())
		{
			return;
		}

		CBCGPControlRenderer* pRenderer = NULL;
		int index = 0;

		BOOL bDisabled = (pButton->m_nStyle & TBBS_DISABLED) == TBBS_DISABLED;
		BOOL bPressed  = (pButton->m_nStyle & TBBS_PRESSED ) == TBBS_PRESSED;
		BOOL bChecked  = (pButton->m_nStyle & TBBS_CHECKED ) == TBBS_CHECKED;
		BOOL bHandled  = FALSE;

		CBCGPBaseControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar, pButton->GetParentWnd ());

		CBCGPToolbarMenuButton* pMenuButton = 
			DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);
		if (pMenuButton != NULL && pBar != NULL)
		{
			if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar)))
			{
				if (state == ButtonsIsPressed || state == ButtonsIsHighlighted)
				{
					if (pMenuButton->IsDroppedDown ())
					{
						ExtendMenuButton (pMenuButton, rect);
						index = 1;
					}

					pRenderer = &m_ctrlMenuBarBtn;

					bHandled = TRUE;
				}
				else
				{
					return;
				}

				bHandled = TRUE;
			}
			else if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar)))
			{
				if (bChecked)
				{
					pRenderer = &m_ctrlMenuItemBack;

					if (bDisabled)
					{
						index = 1;
					}

					rect.InflateRect (0, 0, 0, 1);
					bHandled = TRUE;
				}
				else if (state == ButtonsIsPressed || state == ButtonsIsHighlighted)
				{
					pRenderer = &m_ctrlMenuHighlighted[bDisabled ? 1 : 0];
					bHandled = TRUE;
				}
				else
				{
					return;
				}
			}
			else if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPToolBar)))
			{
				if (pMenuButton->IsDroppedDown ())
				{
					ExtendMenuButton (pMenuButton, rect);
				}
			}
		}
		else if (pBar != NULL && pBar->IsKindOf (RUNTIME_CLASS (CBCGPColorBar)))
		{
			if (bChecked)
			{
				pRenderer = &m_ctrlMenuItemBack;

				if (bDisabled)
				{
					index = 1;
				}
			}


			if (!bDisabled)
			{
				if (state == ButtonsIsHighlighted)
				{
					pRenderer = &m_ctrlMenuHighlighted[0];
					index = 0;
				}
			}

			bHandled = TRUE;
		}
		else if (pBar != NULL && pBar->IsKindOf (RUNTIME_CLASS (CBCGPOutlookBarToolBar)))
		{
			bHandled = TRUE;
		}

		if (!bHandled)
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
			pRenderer->Draw (pDC, rect, index);
			return;
		}
	}

	CBCGPVisualManager2003::OnFillButtonInterior (pDC, pButton, rect, state);
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawButtonBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawButtonBorder (pDC, pButton, rect, state);
		return;
	}

	//------------------------------------------------
	// Draw shadow under the dropped-down menu button:
	//------------------------------------------------
	if (state != ButtonsIsPressed && state != ButtonsIsHighlighted)
	{
		return;
	}

	if (!m_bShdowDroppedDownMenuButton ||
		!CBCGPMenuBar::IsMenuShadows () ||
		CBCGPToolBar::IsCustomizeMode ())
	{
		return;
	}

	CBCGPToolbarMenuButton* pMenuButton = DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);
	if (pMenuButton == NULL || !pMenuButton->IsDroppedDown ())
	{
		return;
	}

	BOOL bIsPopupMenu = 
		pMenuButton->GetParentWnd () != NULL &&
		pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar));

	if (bIsPopupMenu)
	{
		return;
	}

	CBCGPPopupMenu* pPopupMenu= pMenuButton->GetPopupMenu ();
	if (pPopupMenu != NULL && 
		(pPopupMenu->IsWindowVisible () || pPopupMenu->IsShown()) &&
		!pPopupMenu->IsRightAlign () &&
		!(pPopupMenu->GetExStyle() & WS_EX_LAYOUTRTL))
	{
		ExtendMenuButton (pMenuButton, rect);

		CBCGPDrawManager dm (*pDC);

		dm.DrawShadow (rect, m_nMenuShadowDepth, 100, 75, NULL, NULL,
			m_clrMenuShadowBase);
	}
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawButtonSeparator (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state,
		BOOL bHorz)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawButtonSeparator (pDC, pButton, rect, state, bHorz);
		return;
	}

	CPen* pPen = &m_penMenuItemBorder;

	CPen* pOldPen = pDC->SelectObject (pPen);
	ASSERT (pOldPen != NULL);

	if (bHorz)
	{
		pDC->MoveTo (rect.left, rect.top + 2);
		pDC->LineTo (rect.left, rect.bottom - 2);
	}
	else
	{
		pDC->MoveTo (rect.left  + 2, rect.top);
		pDC->LineTo (rect.right - 2, rect.top);
	}

	pDC->SelectObject (pOldPen);
}
//*****************************************************************************
void CBCGPVisualManager2007::OnHighlightMenuItem (CDC *pDC, CBCGPToolbarMenuButton* pButton,
		CRect rect, COLORREF& clrText)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnHighlightMenuItem (pDC, pButton, rect, clrText);
		return;
	}

	m_ctrlMenuHighlighted[(pButton->m_nStyle & TBBS_DISABLED) == TBBS_DISABLED ? 1 : 0].Draw (pDC, rect);
}
//*****************************************************************************
void CBCGPVisualManager2007::OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnHighlightRarelyUsedMenuItems (pDC, rectRarelyUsed);
	}

	rectRarelyUsed.left --;
	rectRarelyUsed.right = rectRarelyUsed.left + CBCGPToolBar::GetMenuImageSize ().cx + 
		2 * GetMenuImageMargin () + 2;

	pDC->FillRect (rectRarelyUsed, &m_brMenuRarelyUsed);
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawMenuCheck (CDC* pDC, CBCGPToolbarMenuButton* pButton, 
		CRect rect, BOOL bHighlight, BOOL bIsRadio)
{
	ASSERT_VALID (pButton);

    CBCGPToolBarImages& img = bIsRadio ? m_MenuItemMarkerR : m_MenuItemMarkerC;

	if (!CanDrawImage () || img.GetCount () == 0)
	{
		CBCGPVisualManager2003::OnDrawMenuCheck (pDC, pButton, rect, bHighlight, bIsRadio);
		return;
	}

    CSize size (img.GetImageSize ());
    CRect rectImage (0, 0, size.cx, size.cy);

    if ((pButton->m_nStyle & TBBS_DISABLED) == TBBS_DISABLED)
    {
        rectImage.OffsetRect (0, size.cy);
    }

	if (globalData.m_bIsRTL)
	{
		img.Mirror ();
	}

    img.DrawEx (pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter, rectImage);

	if (globalData.m_bIsRTL)
	{
		img.Mirror ();
	}
}
//************************************************************************************
void CBCGPVisualManager2007::OnDrawMenuItemButton (CDC* pDC, CBCGPToolbarMenuButton* pButton,
				CRect rectButton, BOOL bHighlight, BOOL bDisabled)
{
	if (!CanDrawImage () || !m_ctrlMenuButtonBorder.IsValid ())
	{
		CBCGPVisualManager2003::OnDrawMenuItemButton (pDC, pButton,
				rectButton, bHighlight, bDisabled);
		return;
	}

	ASSERT_VALID (pDC);

	CRect rect = rectButton;
	rect.right = rect.left + 1;
	rect.left--;
	rect.DeflateRect (0, 1);

	if (bHighlight)
	{
		m_ctrlMenuButtonBorder.Draw (pDC, rect);
	}
	else
	{
		CBrush br (globalData.clrBtnShadow);

		rect.DeflateRect (0, 3);
		rect.right--;
		pDC->FillRect (rect, &br);
	}
}
//*****************************************************************************
CSize CBCGPVisualManager2007::GetPinSize(BOOL bIsPinned)
{
	if (!CanDrawImage () || !m_MenuItemPin.IsValid ())
	{
		return CBCGPVisualManager2003::GetPinSize(bIsPinned);
	}

	if (globalData.GetRibbonImageScale() != 1. && !m_MenuItemPin.IsScaled())
	{
		m_MenuItemPin.SmoothResize(globalData.GetRibbonImageScale());
	}

	return m_MenuItemPin.GetImageSize();
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawPin(CDC* pDC, const CRect& rect, BOOL bIsPinned, BOOL bIsDark, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled)
{
	if (!CanDrawImage () || !m_MenuItemPin.IsValid ())
	{
		CBCGPVisualManager2003::OnDrawPin(pDC, rect, bIsPinned, bIsDark, bIsHighlighted, bIsPressed, bIsDisabled);
		return;
	}

	if (globalData.GetRibbonImageScale() != 1. && !m_MenuItemPin.IsScaled())
	{
		m_MenuItemPin.SmoothResize(globalData.GetRibbonImageScale());
	}

	m_MenuItemPin.DrawEx (pDC, rect, bIsPinned ? 1 : 0, 
		CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter, CRect(0, 0, 0, 0), (BYTE)(bIsDisabled ? 100 : 255));
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawShowAllMenuItems (CDC* pDC, CRect rect, 
												 CBCGPVisualManager::BCGBUTTON_STATE state)
{
	if (!CanDrawImage () || !m_ctrlMenuItemShowAll.IsValid ())
	{
		CBCGPVisualManager2003::OnDrawShowAllMenuItems (pDC, rect, state);
		return;
	}

	m_ctrlMenuItemShowAll.FillInterior (pDC, rect, CBCGPToolBarImages::ImageAlignHorzCenter,
		CBCGPToolBarImages::ImageAlignVertCenter, state == ButtonsIsHighlighted ? 1 : 0);
}
//*****************************************************************************
int CBCGPVisualManager2007::GetShowAllMenuItemsHeight (CDC* pDC, const CSize& sizeDefault)
{
	return (CanDrawImage () && m_ctrlMenuItemShowAll.IsValid ())
		? m_ctrlMenuItemShowAll.GetParams ().m_rectImage.Size ().cy + 2 * TEXT_MARGIN
		: CBCGPVisualManager2003::GetShowAllMenuItemsHeight (pDC, sizeDefault);
}
//*****************************************************************************
COLORREF CBCGPVisualManager2007::OnFillMiniFrameCaption (CDC* pDC, 
								CRect rectCaption, 
								CBCGPMiniFrameWnd* pFrameWnd, BOOL bActive)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnFillMiniFrameCaption (pDC, 
								rectCaption, pFrameWnd, bActive);
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pFrameWnd);

	if (DYNAMIC_DOWNCAST (CBCGPBaseToolBar, pFrameWnd->GetControlBar ()) != NULL)
	{
		pDC->FillRect(rectCaption, &globalData.brInactiveCaption);
		return globalData.clrInactiveCaptionText;
	}

	COLORREF clr = bActive ? globalData.clrActiveCaption : globalData.clrInactiveCaption;
	COLORREF clrGradient = CBCGPDrawManager::PixelAlpha(clr, bActive ? 95 : 115);
	
	CBCGPDrawManager dm (*pDC);
	
	if (bActive)
	{
		dm.FillGradient (rectCaption, clr, clrGradient, TRUE);
	}
	else
	{
		dm.FillGradient (rectCaption, clrGradient, clr, TRUE);
	}
	
    // get the text color
	return bActive ? globalData.clrCaptionText : globalData.clrInactiveCaptionText;
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawMiniFrameBorder (
										CDC* pDC, CBCGPMiniFrameWnd* pFrameWnd,
										CRect rectBorder, CRect rectBorderSize)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawMiniFrameBorder (
										pDC, pFrameWnd,
										rectBorder, rectBorderSize);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pFrameWnd);

#ifndef BCGP_EXCLUDE_TASK_PANE
	BOOL bIsTasksPane = pFrameWnd->IsKindOf( RUNTIME_CLASS( CBCGPTaskPaneMiniFrameWnd ) );
#else
	BOOL bIsTasksPane = FALSE;
#endif

	if (bIsTasksPane)
	{
		CBrush* pOldBrush = pDC->SelectObject (&m_brFloatToolBarBorder);
		ASSERT (pOldBrush != NULL);

		pDC->PatBlt (rectBorder.left, rectBorder.top, rectBorderSize.left, rectBorder.Height (), PATCOPY);
		pDC->PatBlt (rectBorder.left, rectBorder.top, rectBorder.Width (), rectBorderSize.top, PATCOPY);
		pDC->PatBlt (rectBorder.right - rectBorderSize.right, rectBorder.top, rectBorderSize.right, rectBorder.Height (), PATCOPY);
		pDC->PatBlt (rectBorder.left, rectBorder.bottom - rectBorderSize.bottom, rectBorder.Width (), rectBorderSize.bottom, PATCOPY);

		rectBorderSize.DeflateRect (2, 2);
		rectBorder.DeflateRect (2, 2);

		pDC->SelectObject (&globalData.brBarFace);

		pDC->PatBlt (rectBorder.left, rectBorder.top + 1, rectBorderSize.left, rectBorder.Height () - 1, PATCOPY);
		pDC->PatBlt (rectBorder.left + 1, rectBorder.top, rectBorder.Width () - 2, rectBorderSize.top, PATCOPY);
		pDC->PatBlt (rectBorder.right - rectBorderSize.right, rectBorder.top + 1, rectBorderSize.right, rectBorder.Height () - 1, PATCOPY);
		pDC->PatBlt (rectBorder.left + 1, rectBorder.bottom - rectBorderSize.bottom, rectBorder.Width () - 2, rectBorderSize.bottom, PATCOPY);

		pDC->SelectObject (pOldBrush);
	}
	else
	{
		CBCGPVisualManager2003::OnDrawMiniFrameBorder (pDC, pFrameWnd, rectBorder, rectBorderSize);
	}
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawFloatingToolbarBorder (
												CDC* pDC, CBCGPBaseToolBar* pToolBar, 
												CRect rectBorder, CRect rectBorderSize)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawFloatingToolbarBorder (pDC, pToolBar, 
												rectBorder, rectBorderSize);
		return;
	}

	ASSERT_VALID (pDC);

	CBrush* pOldBrush = pDC->SelectObject (&m_brFloatToolBarBorder);
	ASSERT (pOldBrush != NULL);

	pDC->PatBlt (rectBorder.left, rectBorder.top, rectBorderSize.left, rectBorder.Height (), PATCOPY);
	pDC->PatBlt (rectBorder.left, rectBorder.top, rectBorder.Width (), rectBorderSize.top, PATCOPY);
	pDC->PatBlt (rectBorder.right - rectBorderSize.right, rectBorder.top, rectBorderSize.right, rectBorder.Height (), PATCOPY);
	pDC->PatBlt (rectBorder.left, rectBorder.bottom - rectBorderSize.bottom, rectBorder.Width (), rectBorderSize.bottom, PATCOPY);

	rectBorderSize.DeflateRect (2, 2);
	rectBorder.DeflateRect (2, 2);

	pDC->SelectObject (&globalData.brBarFace);

	pDC->PatBlt (rectBorder.left, rectBorder.top + 1, rectBorderSize.left, rectBorder.Height () - 1, PATCOPY);
	pDC->PatBlt (rectBorder.left + 1, rectBorder.top, rectBorder.Width () - 2, rectBorderSize.top, PATCOPY);
	pDC->PatBlt (rectBorder.right - rectBorderSize.right, rectBorder.top + 1, rectBorderSize.right, rectBorder.Height () - 1, PATCOPY);
	pDC->PatBlt (rectBorder.left + 1, rectBorder.bottom - rectBorderSize.bottom, rectBorder.Width () - 2, rectBorderSize.bottom, PATCOPY);

	pDC->SelectObject (pOldBrush);
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::IsOwnerDrawMenuCheck ()
{
	return CanDrawImage () ? FALSE : CBCGPVisualManager2003::IsOwnerDrawMenuCheck ();
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::IsHighlightWholeMenuItem ()
{
	return CanDrawImage () ? TRUE : CBCGPVisualManager2003::IsHighlightWholeMenuItem ();
}
//*****************************************************************************
COLORREF CBCGPVisualManager2007::GetStatusBarPaneTextColor (CBCGPStatusBar* pStatusBar, 
									CBCGStatusBarPaneInfo* pPane)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetStatusBarPaneTextColor (pStatusBar, pPane);
	}

	ASSERT (pPane != NULL);

	return (pPane->nStyle & SBPS_DISABLED) ? m_clrStatusBarTextDisabled : 
			pPane->clrText == (COLORREF)-1 ? m_clrStatusBarText : pPane->clrText;
}
//*****************************************************************************
COLORREF CBCGPVisualManager2007::GetToolbarButtonTextColor (
	CBCGPToolbarButton* pButton, CBCGPVisualManager::BCGBUTTON_STATE state)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetToolbarButtonTextColor (pButton, state);
	}

	ASSERT_VALID (pButton);

	BOOL bDisabled = (CBCGPToolBar::IsCustomizeMode () && !pButton->IsEditable ()) ||
		(!CBCGPToolBar::IsCustomizeMode () && (pButton->m_nStyle & TBBS_DISABLED));

	if (pButton->GetParentWnd () != NULL && 
		pButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar)))
	{
		if (CBCGPToolBar::IsCustomizeMode ())
		{
			return m_clrMenuBarBtnText;
		}

		return bDisabled
					? m_clrMenuBarBtnTextDisabled
					: ((state == ButtonsIsHighlighted || state == ButtonsIsPressed ||
						pButton->IsDroppedDown ())
						? m_clrMenuBarBtnTextHighlighted
						: m_clrMenuBarBtnText);
	}

	return bDisabled
				? m_clrToolBarBtnTextDisabled
				: ((state == ButtonsIsHighlighted || state == ButtonsIsPressed || 
					pButton->IsDroppedDown ())
					? m_clrToolBarBtnTextHighlighted
					: m_clrToolBarBtnText);
}
//*****************************************************************************
COLORREF CBCGPVisualManager2007::GetMenuItemTextColor (
	CBCGPToolbarMenuButton* pButton, BOOL bHighlighted, BOOL bDisabled)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetMenuItemTextColor (pButton, 
			bHighlighted, bDisabled);
	}

	return bDisabled ? m_clrMenuTextDisabled : m_clrMenuText;
}
//*****************************************************************************
COLORREF CBCGPVisualManager2007::GetHighlightedMenuItemTextColor (CBCGPToolbarMenuButton* pButton)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetHighlightedMenuItemTextColor (pButton);
	}

	return m_clrMenuTextHighlighted;
}
//**********************************************************************************
void CBCGPVisualManager2007::GetTabFrameColors (const CBCGPBaseTabWnd* pTabWnd,
				   COLORREF& clrDark,
				   COLORREF& clrBlack,
				   COLORREF& clrHighlight,
				   COLORREF& clrFace,
				   COLORREF& clrDarkShadow,
				   COLORREF& clrLight,
				   CBrush*& pbrFace,
				   CBrush*& pbrBlack)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::GetTabFrameColors (pTabWnd,
				   clrDark, clrBlack, clrHighlight, clrFace,
				   clrDarkShadow, clrLight, pbrFace, pbrBlack);
		return;
	}

	ASSERT_VALID (pTabWnd);

	CBCGPVisualManager2003::GetTabFrameColors (pTabWnd,
				   clrDark,
				   clrBlack,
				   clrHighlight,
				   clrFace,
				   clrDarkShadow,
				   clrLight,
				   pbrFace,
				   pbrBlack);

	if (!pTabWnd->IsDialogControl ())
	{
		if (pTabWnd->IsFlatTab ())
		{
			if (m_clrTabFlatFace != CLR_DEFAULT)
			{
				clrFace = m_clrTabFlatFace;
			}
			if (m_clrTabFlatBlack != CLR_DEFAULT)
			{
				clrBlack = m_clrTabFlatBlack;
			}
			if (m_clrTabFlatDark != CLR_DEFAULT)
			{
				clrDark = m_clrTabFlatDark;
			}
			if (m_clrTabFlatDarkShadow != CLR_DEFAULT)
			{
				clrDarkShadow = m_clrTabFlatDarkShadow;
			}
			if (m_clrTabFlatLight != CLR_DEFAULT)
			{
				clrLight = m_clrTabFlatLight;
			}
			if (m_clrTabFlatHighlight != CLR_DEFAULT)
			{
				clrHighlight = m_clrTabFlatHighlight;
			}
		}
		else
		{
			if (m_clrTab3DFace != CLR_DEFAULT)
			{
				clrFace = m_clrTab3DFace;
			}
			if (m_clrTab3DBlack != CLR_DEFAULT)
			{
				clrBlack = m_clrTab3DBlack;
			}
			if (m_clrTab3DDark != CLR_DEFAULT)
			{
				clrDark = m_clrTab3DDark;
			}
			if (m_clrTab3DDarkShadow != CLR_DEFAULT)
			{
				clrDarkShadow = m_clrTab3DDarkShadow;
			}
			if (m_clrTab3DLight != CLR_DEFAULT)
			{
				clrLight = m_clrTab3DLight;
			}
			if (m_clrTab3DHighlight != CLR_DEFAULT)
			{
				clrHighlight = m_clrTab3DHighlight;
			}
		}
	}
}
//*****************************************************************************
void CBCGPVisualManager2007::OnEraseTabsArea (CDC* pDC, CRect rect, 
										 const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (!CanDrawImage () || pTabWnd->IsDialogControl ())
	{
		CBCGPVisualManager2003::OnEraseTabsArea (pDC, rect, pTabWnd);
		return;
	}

	if(pTabWnd->IsOneNoteStyle () ||
	   pTabWnd->IsColored () ||
	   pTabWnd->IsVS2005Style () ||
	   pTabWnd->IsLeftRightRounded () ||
	   pTabWnd->IsPointerStyle())
	{
		CBCGPVisualManager2003::OnEraseTabsArea (pDC, rect, pTabWnd);
		return;
	}

	const BOOL bBottom = pTabWnd->GetLocation () == CBCGPTabWnd::LOCATION_BOTTOM;

	if (pTabWnd->IsFlatTab ())
	{
		m_ctrlTabFlat[bBottom ? 1 : 0].Draw (pDC, rect);
	}
	else
	{
		CBCGPDrawManager dm (*pDC);

		COLORREF clr1 = m_clrBarGradientDark;
		COLORREF clr2 = m_clrBarGradientLight;

		if (bBottom)
		{
			dm.FillGradient (rect, clr1, clr2, TRUE);
		}
		else
		{
			dm.FillGradient (rect, clr2, clr1, TRUE);
		}
	}
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawTab (CDC* pDC, CRect rectTab,
						int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pTabWnd);
	ASSERT_VALID (pDC);

	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);
		return;
	}

	if(pTabWnd->IsOneNoteStyle () ||
	   pTabWnd->IsColored () ||
	   pTabWnd->IsVS2005Style () ||
	   pTabWnd->IsLeftRightRounded () ||
	   pTabWnd->IsPointerStyle())
	{
		CBCGPVisualManager2003::OnDrawTab (pDC, rectTab, iTab, bIsActive, pTabWnd);
		return;
	}

	const BOOL bBottom = pTabWnd->GetLocation () == CBCGPTabWnd::LOCATION_BOTTOM;
	const BOOL bIsHighlight = iTab == pTabWnd->GetHighlightedTab ();

	COLORREF clrText = pTabWnd->GetTabTextColor (iTab);
	
	if (pTabWnd->IsFlatTab ())
	{
		int nImage = (bIsActive || bIsHighlight) ? 2 : 1;

		CRgn rgn;

		POINT pts[4];

		if (bBottom)
		{
			rectTab.bottom++;

			pts[0].x = rectTab.left;
			pts[0].y = rectTab.bottom + 1;
			pts[1].x = rectTab.left;
			pts[1].y = rectTab.top;
			pts[2].x = rectTab.right + 1;
			pts[2].y = rectTab.top;
			pts[3].x = rectTab.right - rectTab.Height () + 1;
			pts[3].y = rectTab.bottom + 1;

			rectTab.top++;
		}
		else
		{
			pts[0].x = rectTab.left;
			pts[0].y = rectTab.bottom + 1;
			pts[1].x = rectTab.left;
			pts[1].y = rectTab.top;
			pts[2].x = rectTab.right - rectTab.Height () + 1;
			pts[2].y = rectTab.top;
			pts[3].x = rectTab.right + 1;
			pts[3].y = rectTab.bottom + 1;
		}

		rgn.CreatePolygonRgn (pts, 4, WINDING);

		int isave = pDC->SaveDC ();

		pDC->SelectClipRgn (&rgn, RGN_AND);

		m_ctrlTabFlat[bBottom ? 1 : 0].Draw (pDC, rectTab, nImage);

		CPen* pOldPen = pDC->SelectObject (&m_penTabFlatOuter[bIsActive ? 1 : 0]);

		if (bBottom)
		{
			pDC->MoveTo (pts[2].x, pts[2].y);
			pDC->LineTo (pts[3].x, pts[3].y - 1);
		}
		else
		{
			pDC->MoveTo (pts[2].x - 1, pts[2].y);
			pDC->LineTo (pts[3].x - 1, pts[3].y - 1);
		}

		pDC->SelectObject (&m_penTabFlatInner[bIsActive ? 1 : 0]);

		if (bBottom)
		{
			pDC->MoveTo (pts[2].x - 2, pts[2].y + 1);
			pDC->LineTo (pts[3].x, pts[3].y - 2);
		}
		else
		{
			pDC->MoveTo (pts[2].x - 1, pts[2].y + 1);
			pDC->LineTo (pts[3].x - 2, pts[3].y - 1);
		}

		pDC->SelectObject (pOldPen);

		pDC->SelectClipRgn (NULL);

		if (clrText == (COLORREF)-1)
		{
			clrText = bIsActive
						? m_clrTabFlatTextActive
						: bIsHighlight
							? m_clrTabFlatTextHighlighted
							: m_clrTabFlatTextInactive;
		}

		if(clrText == (COLORREF)-1)
		{
			clrText = globalData.clrBarText;
		}

		pDC->RestoreDC (isave);
	}
	else
	{
		if(clrText == (COLORREF)-1)
		{
			clrText = bIsActive
						? m_clrTab3DTextActive
						: bIsHighlight
							? m_clrTab3DTextHighlighted
							: m_clrTab3DTextInactive;
		}

		int nImage = bIsActive ? 3 : 0;
		if(bIsHighlight)
		{
			nImage += 1;
		}

		m_ctrlTab3D[bBottom ? 1 : 0].Draw (pDC, rectTab, nImage);

		if (pTabWnd->IsDialogControl ())
		{
			clrText = globalData.clrBtnText;
		}
	}

	COLORREF clrTextOld = (COLORREF)-1;
	if (pTabWnd->IsFlatTab ())
	{
		if (clrText != (COLORREF)-1)
		{
			clrTextOld = pDC->SetTextColor (clrText);
		}
	}

	if (!bBottom && pTabWnd->IsFlatTab ())
	{
		rectTab.right -= rectTab.Height() / 2;
	}

	OnDrawTabContent (pDC, rectTab, iTab, bIsActive, pTabWnd, clrText);

	if (pTabWnd->IsFlatTab ())
	{
		if (clrText != (COLORREF)-1)
		{
			pDC->SetTextColor (clrTextOld);
		}
	}
}
//*****************************************************************************
void CBCGPVisualManager2007::OnFillTab (CDC* pDC, CRect rectFill, CBrush* pbrFill,
									 int iTab, BOOL bIsActive, 
									 const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pTabWnd);

	if (!CanDrawImage () || pTabWnd->IsDialogControl ())
	{
		CBCGPVisualManager2003::OnFillTab (pDC, rectFill, pbrFill,
									 iTab, bIsActive, pTabWnd);
		return;
	}

	if(pTabWnd->IsFlatTab () ||
	   pTabWnd->IsOneNoteStyle () ||
	   pTabWnd->IsColored () ||
	   pTabWnd->IsVS2005Style () ||
	   pTabWnd->IsLeftRightRounded () ||
	   pTabWnd->IsPointerStyle())
	{
		CBCGPVisualManager2003::OnFillTab (pDC, rectFill, pbrFill,
									 iTab, bIsActive, pTabWnd);
		return;
	}

	ASSERT_VALID (pDC);

	const BOOL bBottom = pTabWnd->GetLocation () == CBCGPTabWnd::LOCATION_BOTTOM;
	const BOOL bIsHighlight = iTab == pTabWnd->GetHighlightedTab ();

	BOOL bIsBeta = IsBeta ();
	if (!bIsBeta || (bIsActive || bIsHighlight))
	{
		int nImage = bIsActive 
						? (bIsBeta ? 2 : 3)
						: (bIsBeta ? -1 : 0);
		if(bIsHighlight)
		{
			nImage += 1;
		}
		
		m_ctrlTab3D[bBottom ? 1 : 0].Draw (pDC, rectFill, nImage);
	}
}
//*****************************************************************************
COLORREF CBCGPVisualManager2007::GetTabTextColor (const CBCGPBaseTabWnd* pTabWnd, int iTab, BOOL bIsActive)
{
	if (!CanDrawImage () || pTabWnd->IsDialogControl () || pTabWnd->IsPointerStyle())
	{
		return CBCGPVisualManager2003::GetTabTextColor (pTabWnd, iTab, bIsActive);
	}

	ASSERT_VALID (pTabWnd);

	if (pTabWnd->GetTabTextColor(iTab) != (COLORREF)-1)
	{
		return pTabWnd->GetTabTextColor(iTab);
	}

	if (pTabWnd->IsOneNoteStyle () || pTabWnd->GetTabBkColor (iTab) != (COLORREF)-1)
	{
		return CBCGPVisualManager2003::GetTabTextColor (pTabWnd, iTab, bIsActive);
	}

	return bIsActive ? m_clrTabTextActive : m_clrTabTextInactive;
}
//*****************************************************************************
int CBCGPVisualManager2007::GetTabHorzMargin (const CBCGPBaseTabWnd* pTabWnd)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetTabHorzMargin (pTabWnd);
	}

	CBCGPControlRenderer* pRenderer = pTabWnd->IsFlatTab ()
										? &m_ctrlTabFlat[0]
										: &m_ctrlTab3D[0];

	if(pTabWnd->IsOneNoteStyle () ||
	   pTabWnd->IsColored () ||
	   pTabWnd->IsVS2005Style () ||
	   pTabWnd->IsLeftRightRounded () ||
	   !pRenderer->IsValid ())
	{
		return CBCGPVisualManager2003::GetTabHorzMargin (pTabWnd);
	}

	if (pTabWnd->IsFlatTab())
	{
		return pTabWnd->GetTabsHeight() / 2 - 2;
	}

	return pRenderer->GetParams ().m_rectSides.right / 2;
}
//*****************************************************************************
BOOL CBCGPVisualManager2007::OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd)
{	
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTabWnd);

	if (!CanDrawImage () || pTabWnd->IsDialogControl ())
	{
		return CBCGPVisualManager2003::OnEraseTabsFrame (pDC, rect, pTabWnd);
	}

	if(pTabWnd->IsOneNoteStyle () ||
	   pTabWnd->IsColored () ||
	   pTabWnd->IsVS2005Style () ||
	   pTabWnd->IsLeftRightRounded ())
	{
		return CBCGPVisualManager2003::OnEraseTabsFrame (pDC, rect, pTabWnd);
	}

	if (pTabWnd->IsFlatTab ())
	{
		pDC->FillRect (rect, &globalData.brWindow);

		if (pTabWnd->GetLocation () != CBCGPTabWnd::LOCATION_BOTTOM)
		{
			CPen pen (PS_SOLID, 1, m_clrTabFlatBlack);
			CPen* pOldPen = pDC->SelectObject (&pen);

			pDC->MoveTo (rect.left, rect.top + pTabWnd->GetTabsHeight () + 1);
			pDC->LineTo (rect.right, rect.top + pTabWnd->GetTabsHeight () + 1);

			pDC->SelectObject (pOldPen);
		}

		return TRUE;
	}

	return FALSE;
}
//*****************************************************************************
void CBCGPVisualManager2007::OnEraseTabsButton (CDC* pDC, CRect rect,
											  CBCGPButton* pButton,
											  CBCGPBaseTabWnd* pBaseTab)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);
	ASSERT_VALID (pBaseTab);

	CBCGPTabWnd* pWndTab = DYNAMIC_DOWNCAST (CBCGPTabWnd, pBaseTab);

	if (!CanDrawImage () || 
		pWndTab == NULL || 
		pBaseTab->IsDialogControl ())
	{
		CBCGPVisualManager2003::OnEraseTabsButton (pDC, rect, pButton, pBaseTab);
		return;
	}

	if(pBaseTab->IsFlatTab () || 
	   pBaseTab->IsOneNoteStyle () ||
	   pBaseTab->IsColored () ||
	   pBaseTab->IsVS2005Style () ||
	   pBaseTab->IsLeftRightRounded () ||
	   (!pButton->IsPressed () && !pButton->IsHighlighted ()))
	{
		CBCGPVisualManager2003::OnEraseTabsButton (pDC, rect, pButton, pBaseTab);
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

	int index = pButton->IsPressed () ? 2 : 1;
	m_ctrlToolBarBtn.Draw (pDC, rect, index);
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
												 CBCGPButton* pButton, UINT uiState,
												 CBCGPBaseTabWnd* pWndTab)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawTabsButtonBorder(pDC, rect, 
												 pButton, uiState,
												 pWndTab);
	}

	ASSERT_VALID(pButton);

	if (pButton->IsPushed () || pButton->IsHighlighted ())
	{
		COLORREF clrDark = globalData.clrBarDkShadow;
		pDC->Draw3dRect (rect, clrDark, clrDark);
	}
}
//**********************************************************************************
void CBCGPVisualManager2007::OnDrawTabButton (CDC* pDC, CRect rect, 
											   const CBCGPBaseTabWnd* pTabWnd,
											   int nID,
											   BOOL bIsHighlighted,
											   BOOL bIsPressed,
											   BOOL bIsDisabled)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawTabButton (pDC, rect, pTabWnd, nID, bIsHighlighted, bIsPressed, bIsDisabled);
		return;
	}


	CBCGPMenuImages::IMAGE_STATE state = CBCGPMenuImages::ImageBlack;

	if (bIsHighlighted)
	{
		OnFillHighlightedArea (pDC, rect, bIsPressed ? &m_brHighlightDn : &m_brHighlight, NULL);
	}
	else
	{
		if (pTabWnd->IsPointerStyle())
		{
			CBCGPMenuImages::DrawByColor(pDC, CBCGPMenuImages::IdClose, rect, globalData.clrBarFace);
			return;
		}

		BOOL bIsActive = m_nCurrDrawedTab == pTabWnd->GetActiveTab();
		COLORREF clrText = GetTabTextColor (pTabWnd, m_nCurrDrawedTab, bIsActive);

		if (pTabWnd->IsFlatTab ())
		{
			clrText = bIsActive ? m_clrTabFlatTextActive : m_clrTabFlatTextInactive;
		}

		if (clrText == (COLORREF)-1)
		{
			clrText = globalData.clrBarText;
		}

		if (GetRValue (clrText) > 100 &&
			GetGValue (clrText) > 100 &&
			GetBValue (clrText) > 100)
		{
			state = CBCGPMenuImages::ImageLtGray;
		}
	}

	CBCGPMenuImages::Draw (pDC, (CBCGPMenuImages::IMAGES_IDS)nID, rect, state);

	if (bIsHighlighted)
	{
		pDC->Draw3dRect (rect, m_clrMenuItemBorder, m_clrMenuItemBorder);
	}
}

#ifndef BCGP_EXCLUDE_TASK_PANE
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawTasksGroupCaption(
										CDC* pDC, CBCGPTasksGroup* pGroup, 
										BOOL bIsHighlighted /*= FALSE*/, BOOL bIsSelected /*= FALSE*/, 
										BOOL bCanCollapse /*= FALSE*/)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID (pGroup);
	ASSERT_VALID (pGroup->m_pPage);

	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawTasksGroupCaption(
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
		CBCGPDrawManager dm (*pDC);

		if (pGroup->m_bIsSpecial)
		{
			if (IsBeta ())
			{
				dm.FillGradient (pGroup->m_rect, 
					bIsHighlighted ? m_clrTaskPaneGroupCaptionHighSpecLight : m_clrTaskPaneGroupCaptionSpecLight,
					bIsHighlighted ? m_clrTaskPaneGroupCaptionHighSpecDark : m_clrTaskPaneGroupCaptionSpecDark,
					TRUE);
			}
			else
			{
				dm.Fill4ColorsGradient (pGroup->m_rect, 
					bIsHighlighted ? m_clrTaskPaneGroupCaptionHighSpecDark  : m_clrTaskPaneGroupCaptionSpecDark, 
					bIsHighlighted ? m_clrTaskPaneGroupCaptionHighSpecLight : m_clrTaskPaneGroupCaptionSpecLight, 
					bIsHighlighted ? m_clrTaskPaneGroupCaptionHighSpecLight : m_clrTaskPaneGroupCaptionSpecLight, 
					bIsHighlighted ? m_clrTaskPaneGroupCaptionHighSpecDark  : m_clrTaskPaneGroupCaptionSpecDark,
					FALSE);
			}
		}
		else
		{
			if (IsBeta ())
			{
				dm.FillGradient (pGroup->m_rect, 
					bIsHighlighted ? m_clrTaskPaneGroupCaptionHighLight : m_clrTaskPaneGroupCaptionLight, 
					bIsHighlighted ? m_clrTaskPaneGroupCaptionHighDark : m_clrTaskPaneGroupCaptionDark, 
					TRUE);
			}
			else
			{
				dm.Fill4ColorsGradient (pGroup->m_rect, 
					bIsHighlighted ? m_clrTaskPaneGroupCaptionHighDark  : m_clrTaskPaneGroupCaptionDark, 
					bIsHighlighted ? m_clrTaskPaneGroupCaptionHighLight : m_clrTaskPaneGroupCaptionLight, 
					bIsHighlighted ? m_clrTaskPaneGroupCaptionHighLight : m_clrTaskPaneGroupCaptionLight, 
					bIsHighlighted ? m_clrTaskPaneGroupCaptionHighDark  : m_clrTaskPaneGroupCaptionDark,
					FALSE);
			}
		}
	}

	//-------------
	// Draw border:
	//-------------
	CRect rectBorder = pGroup->m_rect;
	
	if (!bIsToolBox)
	{
		rectBorder.bottom++;
	}

	pDC->Draw3dRect (rectBorder, m_clrTaskPaneGroupBorder, m_clrTaskPaneGroupBorder);

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
                (pGroup->m_bIsSpecial ? m_clrTaskPaneGroupCaptionTextHighSpec : m_clrTaskPaneGroupCaptionTextHigh) :
				pGroup->m_clrTextHot);
		}
		else
		{
			pDC->SetTextColor (pGroup->m_clrText == (COLORREF)-1 ?
				(pGroup->m_bIsSpecial ? m_clrTaskPaneGroupCaptionTextSpec : m_clrTaskPaneGroupCaptionText) :
				pGroup->m_clrText);
		}
	}

	int nBkModeOld = pDC->SetBkMode(TRANSPARENT);
	
	int nTaskPaneHOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionHorzOffset();
	int nTaskPaneVOffset = pGroup->m_pPage->m_pTaskPane->GetGroupCaptionVertOffset();
	int nCaptionHOffset = (nTaskPaneHOffset != -1 ? nTaskPaneHOffset : m_nGroupCaptionHorzOffset);

	CRect rectText = rectGroup;
	rectText.left += (bShowIcon ? pGroup->m_sizeIcon.cx	+ 5: nCaptionHOffset) + 5;
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
			rectButton.right - (rectButton.Height() + 1) / 2 - (sizeButton.cx + 1) / 2);
		rectButton.top = max(rectButton.top, 
			rectButton.bottom - (rectButton.Height() + 1) / 2 - (sizeButton.cy + 1) / 2);
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

			CBCGPMenuImages::Draw(pDC, 
				pGroup->m_bIsCollapsed 
					? CBCGPMenuImages::IdArowDown
					: CBCGPMenuImages::IdArowUp, 
				rectButton.TopLeft(),
				CBCGPMenuImages::ImageBlack);
		}
	}
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawTask(CDC* pDC, CBCGPTask* pTask, CImageList* pIcons, 
							BOOL bIsHighlighted /*= FALSE*/, BOOL bIsSelected /*= FALSE*/)
{
	ASSERT_VALID (pTask);

	if (CanDrawImage () && pTask->m_bIsSeparator)
	{
		CRect rectFill = pTask->m_rect;
		rectFill.top = rectFill.CenterPoint ().y;
		rectFill.bottom = rectFill.top + 1;

		CBCGPDrawManager dm (*pDC);
		dm.Fill4ColorsGradient (rectFill, m_clrTaskPaneGroupAreaLight, 
			m_clrTaskPaneGroupBorder, m_clrTaskPaneGroupBorder, m_clrTaskPaneGroupAreaLight, FALSE);

        return;
	}

	CBCGPVisualManager2003::OnDrawTask(pDC, pTask, pIcons, bIsHighlighted, 
		bIsSelected);
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawScrollButtons(pDC, rect, nBorderSize, iImage, bHilited);
		return;
	}

	CRect rt (rect);
	rt.top--;
	m_ctrlTaskScrollBtn.Draw (pDC, rt, bHilited ? 1 : 0);

	CBCGPMenuImages::Draw (pDC, (CBCGPMenuImages::IMAGES_IDS) iImage, rect);
}

#endif	// BCGP_EXCLUDE_TASK_PANE

//****************************************************************************************
COLORREF CBCGPVisualManager2007::GetHeaderCtrlTextColor (CBCGPHeaderCtrl* pCtrl, BOOL bIsPressed, BOOL bIsHighlighted)
{
	BOOL bIsVMStyle = (pCtrl == NULL || pCtrl->m_bVisualManagerStyle);
	if (!CanDrawImage () || !bIsVMStyle)
	{
		return CBCGPVisualManager2003::GetHeaderCtrlTextColor (pCtrl, bIsPressed, bIsHighlighted);
	}

	if (bIsPressed)
	{
		return m_clrHeaderPressedText;
	}

	if (bIsHighlighted)
	{
		return m_clrHeaderHighlightedText;
	}

	return m_clrHeaderNormalText;
}
//****************************************************************************************
void CBCGPVisualManager2007::OnDrawHeaderCtrlBorder (CBCGPHeaderCtrl* pCtrl, CDC* pDC,
		CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted)
{
	BOOL bIsVMStyle = (pCtrl == NULL || pCtrl->m_bVisualManagerStyle);
	if (!CanDrawImage () || !bIsVMStyle)
	{
		CBCGPVisualManager2003::OnDrawHeaderCtrlBorder (pCtrl, pDC, rect, bIsPressed, bIsHighlighted);
		return;
	}

	COLORREF clrStart  = m_clrHeaderNormalStart;
	COLORREF clrFinish = m_clrHeaderNormalFinish;
	COLORREF clrBorder = m_clrHeaderNormalBorder;

	if (bIsPressed)
	{
		clrStart  = m_clrHeaderPressedStart;
		clrFinish = m_clrHeaderPressedFinish;
		clrBorder = m_clrHeaderPressedBorder;
	}
	else if (bIsHighlighted)
	{
		clrStart  = m_clrHeaderHighlightedStart;
		clrFinish = m_clrHeaderHighlightedFinish;
		clrBorder = m_clrHeaderHighlightedBorder;
	}

	{
		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rect, clrFinish, clrStart);
	}

	CPen pen (PS_SOLID, 0, clrBorder);
	CPen* pOldPen = pDC->SelectObject (&pen);
	
	if (bIsPressed || bIsHighlighted)
	{
		pDC->MoveTo (rect.right - 1, rect.top);
		pDC->LineTo (rect.right - 1, rect.bottom - 1);
		pDC->LineTo (rect.left, rect.bottom - 1);
		pDC->LineTo (rect.left, rect.top - 1);
	}
	else
	{
		pDC->MoveTo (rect.right - 1, rect.top);
		pDC->LineTo (rect.right - 1, rect.bottom - 1);
		pDC->LineTo (rect.left - 1, rect.bottom - 1);
	}

	pDC->SelectObject (pOldPen);
}

#ifndef BCGP_EXCLUDE_GRID_CTRL

void CBCGPVisualManager2007::OnFillGridHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillGridHeaderBackground (pCtrl, pDC, rect);
		return;
	}

	ASSERT_VALID(pDC);

	pDC->FillRect (rect, &globalData.brBarFace);
}
//********************************************************************************
BOOL CBCGPVisualManager2007::OnDrawGridHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnDrawGridHeaderItemBorder (pCtrl, pDC, rect, bPressed);
	}

	ASSERT_VALID(pDC);

	COLORREF clrStart  = bPressed ? m_clrGridHeaderPressedStart : m_clrGridHeaderNormalStart;
	COLORREF clrFinish = bPressed ? m_clrGridHeaderPressedFinish : m_clrGridHeaderNormalFinish;
	COLORREF clrBorder = bPressed ? m_clrGridHeaderPressedBorder : m_clrGridHeaderNormalBorder;

	{
		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rect, clrFinish, clrStart);
	}

	CPen* pOldPen = pDC->SelectObject (&m_penGridSeparator);

	int nHeight = rect.Height () / 5;
	if (pCtrl->GetHeaderFlags () & BCGP_GRID_HEADER_FORCESIMPLEBORDERS)
	{
		nHeight = 0;
	}

	pDC->MoveTo (rect.right - 1, rect.top + nHeight);
	pDC->LineTo (rect.right - 1, rect.bottom - nHeight);

	pDC->SelectObject (pOldPen);

	{
		CPen pen (PS_SOLID, 1, clrBorder);
		pOldPen = pDC->SelectObject (&pen);

		pDC->MoveTo (rect.left, rect.top);
		pDC->LineTo (rect.right, rect.top);

		pDC->MoveTo (rect.left, rect.bottom - 1);
		pDC->LineTo (rect.right, rect.bottom - 1);

		pDC->SelectObject (pOldPen);
	}

	return FALSE;
}
//********************************************************************************
void CBCGPVisualManager2007::OnFillGridRowHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillGridRowHeaderBackground (pCtrl, pDC, rect);
		return;
	}

	ASSERT_VALID(pDC);

	CRect rectFill = rect;
	rectFill.top += 1;
	pDC->FillRect (rectFill, &globalData.brBarFace);
}
//********************************************************************************
BOOL CBCGPVisualManager2007::OnDrawGridRowHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnDrawGridRowHeaderItemBorder (pCtrl, pDC, rect, bPressed);
	}

	ASSERT_VALID(pDC);

	CRect rectInnerBorders (1, 1, 1, 0);
	CRect rectOuterBorders (0, 0, 0, 1);

	COLORREF clrStart  = bPressed ? m_clrGridHeaderPressedFinish : m_clrGridHeaderNormalFinish;
	COLORREF clrFinish = bPressed ? m_clrGridHeaderPressedStart : m_clrGridHeaderNormalStart;
	COLORREF clrBorder = bPressed ? m_clrGridHeaderPressedBorder : m_clrGridHeaderNormalBorder;

	{
		CRect rectFill = rect;
		rectFill.top += rectInnerBorders.top;

		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rectFill, clrFinish, clrStart, FALSE);
	}

	CPen* pOldPen = pDC->SelectObject (&m_penGridSeparator);

	pDC->MoveTo (rect.left, rect.bottom - 1 + rectOuterBorders.bottom);
	pDC->LineTo (rect.right, rect.bottom - 1 + rectOuterBorders.bottom);

	pDC->SelectObject (pOldPen);

	{
		CPen pen (PS_SOLID, 1, clrBorder);
		pOldPen = pDC->SelectObject (&pen);

		pDC->MoveTo (rect.left, rect.top);
		pDC->LineTo (rect.left, rect.bottom + rectOuterBorders.bottom);

		pDC->MoveTo (rect.right - 1, rect.top);
		pDC->LineTo (rect.right - 1, rect.bottom + rectOuterBorders.bottom);

		pDC->SelectObject (pOldPen);
	}

	return FALSE;
}
//********************************************************************************
void CBCGPVisualManager2007::OnFillGridSelectAllAreaBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillGridSelectAllAreaBackground (pCtrl, pDC, rect, bPressed);
		return;
	}

	ASSERT_VALID(pDC);

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient 
		(
			rect, 
			bPressed ? m_clrGridHeaderAllPressedBackFinish : m_clrGridHeaderAllNormalBackFinish, 
			bPressed ? m_clrGridHeaderAllPressedBackStart : m_clrGridHeaderAllNormalBackStart
		);
}
//********************************************************************************
void CBCGPVisualManager2007::OnDrawGridSelectAllMarker(CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, int nPadding, BOOL bPressed)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawGridSelectAllMarker(pCtrl, pDC, rect, nPadding, bPressed);
		return;
	}

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

	CBCGPDrawManager dm(*pDC);

	dm.FillGradient(rect, 
		bPressed ? m_clrGridHeaderAllPressedSignFinish : m_clrGridHeaderAllNormalSignFinish, 
		bPressed ? m_clrGridHeaderAllPressedSignStart : m_clrGridHeaderAllNormalSignStart);

	pDC->SelectClipRgn (NULL, RGN_COPY);
}
//********************************************************************************
BOOL CBCGPVisualManager2007::OnDrawGridSelectAllAreaBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnDrawGridSelectAllAreaBorder (pCtrl, pDC, rect, bPressed);
	}

	ASSERT_VALID(pDC);

	COLORREF clrBorder = bPressed ? m_clrGridHeaderPressedBorder : m_clrGridHeaderNormalBorder;
	pDC->Draw3dRect (rect, clrBorder, clrBorder);

	rect.DeflateRect (1, 1);

	pDC->Draw3dRect 
		(
			rect, 
			bPressed ? m_clrGridHeaderAllPressedBorderHighlighted : m_clrGridHeaderAllNormalBorderHighlighted, 
			bPressed ? m_clrGridHeaderAllPressedBorderShadow : m_clrGridHeaderAllNormalBorderShadow
		);

	return FALSE;
}
//********************************************************************************
COLORREF CBCGPVisualManager2007::OnFillGridGroupByBoxBackground (CDC* pDC, CRect rect)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnFillGridGroupByBoxBackground (pDC, rect);
	}

	ASSERT_VALID (pDC);

	pDC->FillRect (rect, &globalData.brBarFace);

	return globalData.clrBarText;
}
//********************************************************************************
COLORREF CBCGPVisualManager2007::OnFillGridGroupByBoxTitleBackground (CDC* pDC, CRect rect)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnFillGridGroupByBoxTitleBackground (pDC, rect);
	}

	ASSERT_VALID (pDC);
	return globalData.clrBarText;
}
//********************************************************************************
void CBCGPVisualManager2007::OnDrawGridGroupByBoxItemBorder (CBCGPGridCtrl* pCtrl,
														 CDC* pDC, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawGridGroupByBoxItemBorder (pCtrl,
														 pDC, rect);
		return;
	}

	ASSERT_VALID (pDC);

	COLORREF clrStart  = m_clrGridHeaderNormalStart;
	COLORREF clrFinish = m_clrGridHeaderNormalFinish;
	COLORREF clrBorder = m_clrGridHeaderNormalBorder;

	{
		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rect, clrFinish, clrStart);
	}

	pDC->Draw3dRect (rect, clrBorder, clrBorder);
}
//*****************************************************************************
COLORREF CBCGPVisualManager2007::GetGridLeftOffsetColor (CBCGPGridCtrl* pCtrl)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetGridLeftOffsetColor (pCtrl);
	}

	return m_clrGridLeftOffset;
}
//********************************************************************************
COLORREF CBCGPVisualManager2007::OnFillGridRowBackground (CBCGPGridCtrl* pCtrl, 
												  CDC* pDC, CRect rectFill, BOOL bSelected)
{
	COLORREF clr = CBCGPVisualManager2003::OnFillGridRowBackground (pCtrl, 
											  pDC, rectFill, bSelected);

	if (CanDrawImage ())
	{
		clr = pCtrl->IsHighlightGroups ()
				? (pCtrl->IsControlBarColors ()
					? globalData.clrBarText
					: globalData.clrBtnShadow)
				: globalData.clrWindowText;
	}

	return clr;
}
//********************************************************************************
BOOL CBCGPVisualManager2007::OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnSetGridColorTheme (pCtrl, theme);
	}

	theme = m_GridColors;

	return TRUE;
}
//********************************************************************************
void CBCGPVisualManager2007::OnDrawGridDataBar(CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawGridDataBar(pCtrl, pDC, rect);
		return;
	}

	ASSERT_VALID(pDC);

	COLORREF clrLight = RGB(223, 243, 229);
	COLORREF clrDark = RGB (99, 195, 132);

	CBCGPDrawManager dm(*pDC);

	dm.FillGradient(rect, clrDark, clrLight, FALSE);
	pDC->Draw3dRect(rect, clrDark, clrDark);
}

#endif // BCGP_EXCLUDE_GRID_CTRL

//********************************************************************************

#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

//********************************************************************************
void CBCGPVisualManager2007::GetGanttColors (const CBCGPGanttChart* pChart, BCGP_GANTT_CHART_COLORS& colors, COLORREF clrBack) const
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::GetGanttColors (pChart, colors, clrBack);
		return;
	}

	BOOL bIsDefault = FALSE;

	if (clrBack == CLR_DEFAULT)
	{
		clrBack = m_clrPlannerWork;
		bIsDefault = TRUE;
	}	

	CBCGPGanttChart::PrepareColorScheme (clrBack, colors);

    colors.clrBackground = globalData.clrWindow;
	colors.clrShadows   = m_clrMenuShadowBase;

	colors.clrRowBackground = CalculateWorkingColor (clrBack);
	colors.clrRowDayOff = CalculateNonWorkingColor (clrBack, TRUE);
	colors.clrGridLine0 = CalculateSeparatorColor (clrBack);

	if (bIsDefault)
	{
		colors.clrGridLine1 = globalData.clrBarFace;
	}
	else
	{
		colors.clrGridLine1 = CalculateHourLineColor (clrBack, TRUE, TRUE);
	}

	colors.clrConnectorLines = CalculateSelectionColor (clrBack);
	colors.clrSelection = colors.clrGridLine0;
	colors.clrSelectionBorder = colors.clrConnectorLines;
	
    double H, S, L;
    CBCGPDrawManager::RGBtoHSL (RGB (0, 0, 255), &H, &S, &L);
    colors.clrBarFill         = CBCGPDrawManager::HLStoRGB_ONE (H, max (L, 0.85) - 0.30, S);

    CBCGPDrawManager::RGBtoHSL (RGB (0, 255, 0), &H, &S, &L);
    colors.clrBarComplete     = CBCGPDrawManager::HLStoRGB_ONE (H, max (L, 0.85) - 0.30, S);
}
//********************************************************************************
void CBCGPVisualManager2007::DrawGanttHeaderCell (const CBCGPGanttChart* pChart, CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, BOOL bHilite)
{
    if (!CanDrawImage ())
    {
        CBCGPVisualManager2003::DrawGanttHeaderCell (pChart, dc, cellInfo, bHilite);
        return;
    }

    // Use the same as in grid control

    BOOL bPressed = FALSE; // reserved
    COLORREF clrStart  = bPressed ? m_clrGridHeaderPressedStart : m_clrGridHeaderNormalStart;
    COLORREF clrFinish = bPressed ? m_clrGridHeaderPressedFinish : m_clrGridHeaderNormalFinish;
    COLORREF clrBorder = bPressed ? m_clrGridHeaderPressedBorder : m_clrGridHeaderNormalBorder;

    CRect rect = cellInfo.rectCell;

    // Fill the header
    {
        CBCGPDrawManager dm (dc);
        dm.FillGradient (rect, clrFinish, clrStart);
    }

    // Drawing header separator
    {
        CPen* pOldPen = dc.SelectObject (&m_penGridSeparator);

        int nHeight = rect.Height () / 5;

        dc.MoveTo (rect.right - 1, rect.top + nHeight);
        dc.LineTo (rect.right - 1, rect.bottom - nHeight);

        dc.SelectObject (pOldPen);
    }

    // Drawing header borders
    {
        CBCGPPenSelector pen (dc, clrBorder);

        if (cellInfo.pHeaderInfo != NULL && cellInfo.pHeaderInfo->bAboveChart)
        {
            dc.MoveTo (rect.left, rect.bottom - 1);
            dc.LineTo (rect.right, rect.bottom - 1);
        }
        else
        {
            dc.MoveTo (rect.left, rect.top);
            dc.LineTo (rect.right, rect.top);
        }

    }
}
//********************************************************************************
COLORREF CBCGPVisualManager2007::GetGanttHeaderTextColor (BOOL bHilite) const
{
    if (!CanDrawImage ())
    {
        return CBCGPVisualManager2003::GetGanttHeaderTextColor (bHilite);
    }

	return bHilite ? globalData.clrHotText : globalData.clrBtnText;
}

#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

//********************************************************************************
CSize CBCGPVisualManager2007::GetCheckRadioDefaultSize ()
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetCheckRadioDefaultSize ();
	}

	return m_ctrlRibbonBtnCheck.GetParams ().m_rectImage.Size () + CSize (2, 2);
}
//********************************************************************************
BOOL CBCGPVisualManager2007::DrawCheckBox (CDC *pDC, CRect rect, 
										 BOOL bHighlighted, 
										 int nState,
										 BOOL bEnabled,
										 BOOL bPressed)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::DrawCheckBox (pDC, rect, bHighlighted, nState, bEnabled, bPressed);
	}

	int index = nState * 4;

	if (!bEnabled)
	{
		index += 3;
	}
	else if (bPressed)
	{
		if (bHighlighted)
		{
			index += 2;
		}
	}
	else if (bHighlighted)
	{
		index += 1;
	}

	if (globalData.m_bIsRTL)
	{
		m_ctrlRibbonBtnCheck.Mirror ();
	}

	m_ctrlRibbonBtnCheck.FillInterior (pDC, rect, 
		CBCGPToolBarImages::ImageAlignHorzCenter, 
		CBCGPToolBarImages::ImageAlignVertCenter,
		index);

	if (globalData.m_bIsRTL)
	{
		m_ctrlRibbonBtnCheck.Mirror ();
	}

	return TRUE;
}

BOOL CBCGPVisualManager2007::DrawRadioButton (CDC *pDC, CRect rect, 
										 BOOL bHighlighted, 
										 BOOL bChecked,								 
										 BOOL bEnabled,
										 BOOL bPressed)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::DrawRadioButton (pDC, rect, bHighlighted, bChecked, bEnabled, bPressed);
	}

	int index = bChecked ? 4 : 0;

	if (!bEnabled)
	{
		index += 3;
	}
	else if (bPressed)
	{
		if (bHighlighted)
		{
			index += 2;
		}
	}
	else if (bHighlighted)
	{
		index += 1;
	}

	if (globalData.m_bIsRTL)
	{
		m_ctrlRibbonBtnRadio.Mirror ();
	}

	m_ctrlRibbonBtnRadio.FillInterior (pDC, rect, 
		CBCGPToolBarImages::ImageAlignHorzCenter, 
		CBCGPToolBarImages::ImageAlignVertCenter,
		index);

	if (globalData.m_bIsRTL)
	{
		m_ctrlRibbonBtnRadio.Mirror ();
	}

	return TRUE;
}

#ifndef BCGP_EXCLUDE_RIBBON

void CBCGPVisualManager2007::OnDrawRibbonCaption (CDC* pDC, CBCGPRibbonBar* pBar,
											  CRect rectCaption, CRect rectText)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonCaption (pDC, pBar, rectCaption, rectText);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pBar);

	CWnd* pWnd = pBar->GetParent ();
	ASSERT_VALID (pWnd);

	const DWORD dwStyle   = pWnd->GetStyle ();
	const DWORD dwStyleEx = pWnd->GetExStyle ();

	const BOOL bIsRTL     = (dwStyleEx & WS_EX_LAYOUTRTL) == WS_EX_LAYOUTRTL;
	const BOOL bActive    = IsWindowActive (pWnd);
	const BOOL bGlass	  = pBar->IsTransparentCaption ();

    {
		CSize szSysBorder (globalUtils.GetSystemBorders (pBar->GetParent()));
		if (szSysBorder.cx == 0 && szSysBorder.cy == 0)
		{
			szSysBorder = globalUtils.GetSystemBorders (BCGCBProGetTopLevelFrame(pBar));
		}

		CRect rectCaption1 (rectCaption);
		CRect rectBorder (m_ctrlMainBorderCaption.GetParams ().m_rectSides);
		CRect rectQAT = pBar->GetQuickAccessToolbarLocation ();

		if (rectQAT.left > rectQAT.right)
		{
			rectText.left = rectQAT.left + 1;
		}

		rectCaption1.InflateRect (szSysBorder.cx, szSysBorder.cy, szSysBorder.cx, 0);

		BOOL bHide  = (pBar->GetHideFlags () & BCGPRIBBONBAR_HIDE_ALL) != 0;
		BOOL bExtra = !bHide && pBar->IsQuickAccessToolbarOnTop () &&
					  rectQAT.left < rectQAT.right && 
					  (!pBar->IsQATEmpty() || IsBeta1 ());

		if (!bGlass)
		{
			if (IsBeta ())
			{
				COLORREF clr1  = bActive 
									? m_clrAppCaptionActiveStart 
									: m_clrAppCaptionInactiveStart;
				COLORREF clr2  = bActive 
									? m_clrAppCaptionActiveFinish 
									: m_clrAppCaptionInactiveFinish;

				CRect rectCaption2 (rectCaption1);
				rectCaption2.DeflateRect (rectBorder.left, rectBorder.top, 
					rectBorder.right, rectBorder.bottom);

				CBCGPDrawManager dm (*pDC);
				dm.Fill4ColorsGradient (rectCaption2, clr1, clr2, clr2, clr1, FALSE);

				m_ctrlMainBorderCaption.DrawFrame (pDC, rectCaption1, bActive ? 0 : 1);
			}
			else
			{
				m_ctrlMainBorderCaption.Draw (pDC, rectCaption1, bActive ? 0 : 1);
			}
		}

		BOOL bDrawIcon = (bHide && !bExtra) || pBar->IsScenicLook ();

		if (bExtra)
		{
			CBCGPControlRenderer* pCaptionQA = bGlass 
				? &m_ctrlRibbonCaptionQA_Glass
				: &m_ctrlRibbonCaptionQA;

			if (pCaptionQA->IsValid ())
			{
				CRect rectQAFrame (rectQAT);
				rectQAFrame.right = pBar->GetQATCommandsLocation ().right;
				rectQAFrame.InflateRect (0, 1, 1, 1);
				if (!pBar->IsScenicLook ())
				{
					const CBCGPControlRendererParams& params = pCaptionQA->GetParams ();

					rectQAFrame.left -= params.m_rectCorners.left - 2;
					rectQAFrame.right += GetRibbonQATRightMargin ();

					if (rectQAFrame.Height () < params.m_rectImage.Height ())
					{
						rectQAFrame.top = rectQAFrame.bottom - params.m_rectImage.Height ();
					}

					if (bGlass)
					{
						const int dxFrame = GetSystemMetrics (SM_CXSIZEFRAME) / 2;
						const int nTop = globalData.GetRibbonImageScale () != 1. ? -2 : 1;

						rectQAFrame.DeflateRect (1, nTop, dxFrame, 0);
					}

					pCaptionQA->Draw (pDC, rectQAFrame, bActive ? 0 : 1);
				}
				else
				{
					rectQAFrame.DeflateRect (0, 4, 0, 6);
					CRect rectSep (rectQAFrame);
					rectSep.left -= 2;
					rectSep.right = rectSep.left + 2;

					DrawSeparator (pDC, rectSep, m_penSeparatorDark, m_penSeparatorLight, FALSE);

					rectSep.left = rectText.left - 6;
					rectSep.right = rectSep.left + 2;

					DrawSeparator (pDC, rectSep, m_penSeparatorDark, m_penSeparatorLight, FALSE);
				}
			}
		}
		
		if (bDrawIcon)
		{
			BOOL bDestroyIcon = FALSE;
			HICON hIcon = globalUtils.GetWndIcon (pWnd, &bDestroyIcon, FALSE);

			if (hIcon != NULL)
			{
				CSize szIcon (::GetSystemMetrics (SM_CXSMICON), ::GetSystemMetrics (SM_CYSMICON));

				long x = rectCaption.left + 2 + pBar->GetControlsSpacing().cx / 2;
				long y = rectCaption.top  + max (0, (rectCaption.Height () - szIcon.cy) / 2);

				if (bGlass)
				{
					globalData.DrawIconOnGlass (m_hThemeWindow, pDC, hIcon, CRect (x, y, x + szIcon.cx, y + szIcon.cy));
				}
				else
				{
					::DrawIconEx (pDC->GetSafeHdc (), x, y, hIcon, szIcon.cx, szIcon.cy,
						0, NULL, DI_NORMAL);
				}

				if (rectText.left < (x + szIcon.cx + 4))
				{
					rectText.left = x + szIcon.cx + 4;
				}

				if (bDestroyIcon)
				{
					::DestroyIcon (hIcon);
				}
			}
		}
    }

	CString strText;
	pWnd->GetWindowText (strText);

	CFont* pOldFont = (CFont*)pDC->SelectObject (&GetNcCaptionTextFont());
	ASSERT (pOldFont != NULL);

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

	DrawNcText (pDC, rectText, strTitle, strDocument, bPrefix, bActive, 
		bIsRTL, m_bNcTextCenter && !pBar->IsScenicLook (), bGlass, pWnd->IsZoomed () && !globalData.bIsWindows7 ? 0 : 10, 
		pWnd->IsZoomed () && !globalData.bIsWindows7 ? RGB (255, 255, 255) : (COLORREF)-1);

	pDC->SelectObject (pOldFont);
}
//*****************************************************************************
int CBCGPVisualManager2007::GetRibbonQATRightMargin ()
{
	if (!CanDrawImage () ||
		!m_ctrlRibbonCaptionQA.IsValid ())
	{
		return CBCGPVisualManager2003::GetRibbonQATRightMargin ();
	}

	return m_ctrlRibbonCaptionQA.GetParams ().m_rectSides.right;
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonCaptionButton (
					CDC* pDC, CBCGPRibbonCaptionButton* pButton)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonCaptionButton (pDC, pButton);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	const BOOL bHighlighted = pButton->IsHighlighted () || pButton->IsFocused ();
	const BOOL bPressed = pButton->IsPressed ();

	BCGBUTTON_STATE state = ButtonsIsRegular;
	if (bPressed)
	{
		if (bHighlighted)
		{
			state = ButtonsIsPressed;
		}
	}
	else if (bHighlighted)
	{
		state = ButtonsIsHighlighted;
	}

	const BOOL bMDI = pButton->IsMDIChildButton ();
	BOOL bActive = TRUE;

	if (!bMDI)
	{
		CBCGPRibbonBar* pBar = pButton->GetParentRibbonBar ();
		if (pBar->GetSafeHwnd () != NULL)
		{
			CWnd* pWnd = pBar->GetParent ();
			ASSERT_VALID (pWnd);

			bActive = IsWindowActive (pWnd);
		}
	}

	DrawNcBtn (pDC, pButton->GetRect (), pButton->GetID (), state, FALSE, bActive, pButton->IsMDIChildButton ());
}
//*******************************************************************************
COLORREF CBCGPVisualManager2007::OnDrawRibbonButtonsGroup (
					CDC* pDC, CBCGPRibbonButtonsGroup* pGroup,
					CRect rectGroup)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnDrawRibbonButtonsGroup (pDC, pGroup, rectGroup);
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pGroup);

	if (pGroup->IsKindOf (RUNTIME_CLASS(CBCGPRibbonQuickAccessToolbar)) &&
		m_ctrlRibbonPanelQAT.IsValid ())
	{
		CBCGPRibbonBar* pBar = pGroup->GetParentRibbonBar ();

		if (pBar != NULL &&
			(pBar->GetHideFlags () & BCGPRIBBONBAR_HIDE_ALL) == 0 &&
			!pBar->IsQuickAccessToolbarOnTop ())
		{
			m_ctrlRibbonPanelQAT.Draw (pDC, rectGroup);
		}
	}

	return (COLORREF)-1;
}
//*******************************************************************************
void CBCGPVisualManager2007::OnDrawDefaultRibbonImage (CDC* pDC, CRect rectImage,
					BOOL bIsDisabled/* = FALSE*/,
					BOOL bIsPressed/* = FALSE*/,
					BOOL bIsHighlighted/* = FALSE*/)
{
	if (!CanDrawImage () || !m_RibbonBtnDefaultImage.IsValid ())
	{
		CBCGPVisualManager2003::OnDrawDefaultRibbonImage (pDC, rectImage, 
			bIsDisabled, bIsPressed, bIsHighlighted);
		return;
	}

	m_RibbonBtnDefaultImage.DrawEx (pDC, rectImage, bIsDisabled ? 1 : 0,
		CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
}

//*******************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonMainButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonMainButton (pDC, pButton);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	BOOL bHighlighted = pButton->IsHighlighted () || pButton->IsFocused ();
	BOOL bPressed = pButton->IsPressed ();

	if (pButton->IsDroppedDown ())
	{
		bPressed = TRUE;
		bHighlighted = TRUE;
	}

	CRect rect = pButton->GetRect ();

	int index = 0;
	if (bPressed)
	{
		if (bHighlighted)
		{
			index = 2;
		}
	}
	else if (bHighlighted)
	{
		index = 1;
	}

	if (!pButton->GetParentRibbonBar ()->IsScenicLook ())
	{
		CRect rectImage (m_RibbonBtnMain.GetParams ().m_rectImage);

		CBCGPToolBarImages::ImageAlignHorz horz = CBCGPToolBarImages::ImageAlignHorzStretch;
		CBCGPToolBarImages::ImageAlignVert vert = CBCGPToolBarImages::ImageAlignVertStretch;

		if (rect.Width () >= rectImage.Width () &&
			rect.Height () >= rectImage.Height () &&
			(globalData.GetRibbonImageScale () == 1. || m_RibbonBtnMain.IsScaled ()))
		{
			horz = CBCGPToolBarImages::ImageAlignHorzCenter;
			vert = CBCGPToolBarImages::ImageAlignVertCenter;
		}

		rect.OffsetRect (1, -1);
		m_RibbonBtnMain.FillInterior (pDC, rect, horz, vert, index);
	}
	else
	{
		m_ctrlRibbonBtnGroup_S.Draw (pDC, rect, index);
	}
}
//*****************************************************************************
COLORREF CBCGPVisualManager2007::OnDrawRibbonTabsFrame (
					CDC* pDC, 
					CBCGPRibbonBar* pWndRibbonBar, 
					CRect rectTab)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnDrawRibbonTabsFrame (pDC, 
											pWndRibbonBar, rectTab);
	}

	return m_clrRibbonCategoryText;
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonCategory (
		CDC* pDC, 
		CBCGPRibbonCategory* pCategory,
		CRect rectCategory)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonCategory (pDC, pCategory, rectCategory);
		return;
	}

	if (pCategory->IsOnDialogBar ())
	{
		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rectCategory, m_clrToolBarGradientDark, m_clrToolBarGradientLight, TRUE);

		return;
	}

	CBCGPControlRenderer* pRenderer = &m_ctrlRibbonCategoryBack;
	CBCGPBitmapCache* pCache = &m_cacheRibbonCategoryBack;

	CBCGPBaseRibbonElement* pParentButton = pCategory->GetParentButton ();

	if (pCategory->GetTabColor () != BCGPCategoryColor_None &&
		(pParentButton == NULL || !pParentButton->IsQATMode ()))
	{
		XRibbonContextCategory& context = 
			m_ctrlRibbonContextCategory[pCategory->GetTabColor () - 1];

		pRenderer = &context.m_ctrlBack;
		pCache    = &context.m_cacheBack;
	}

	const CBCGPControlRendererParams& params = pRenderer->GetParams ();

	CBCGPRibbonPanelMenuBar* pMenuBar = pCategory->GetParentMenuBar ();
	if (pMenuBar != NULL)
	{
		if (pMenuBar->GetCategory () != NULL)
		{
			if (rectCategory.left < 0 || rectCategory.top < 0)
			{
				CBCGPDrawManager dm (*pDC);
				dm.FillGradient (rectCategory, m_clrBarGradientDark, m_clrBarGradientLight, TRUE);

				return;
			}
		}
		else if (pMenuBar->GetPanel () != NULL)
		{
			if (IsBeta ())
			{
				pRenderer->FillInterior (pDC, rectCategory);
				return;
			}
			else if (m_ctrlRibbonBorder_Panel.IsValid ())
			{
				CRect rectSides (m_ctrlRibbonBorder_Panel.GetParams ().m_rectSides);
				rectCategory.InflateRect (rectSides.left, rectSides.top, rectSides.right, rectSides.bottom);
			}
		}
	}

	int nCacheIndex = -1;
	if (pCache != NULL)
	{
		CSize size (params.m_rectImage.Width (), rectCategory.Height ());
		nCacheIndex = pCache->FindIndex (size);
		if (nCacheIndex == -1)
		{
			nCacheIndex = pCache->CacheY (size.cy, *pRenderer);
		}
	}

	if (nCacheIndex != -1)
	{
		pCache->Get(nCacheIndex)->DrawY (pDC, rectCategory, 
			CSize (params.m_rectInter.left, params.m_rectImage.right - params.m_rectInter.right));
	}
	else
	{
		pRenderer->Draw (pDC, rectCategory);
	}
}
//*****************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonCategoryScroll (
					CDC* pDC, 
					CBCGPRibbonCategoryScroll* pScroll)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonCategoryScroll (pDC, pScroll);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pScroll);

	CRect rect = pScroll->GetRect ();

	CBCGPControlRenderer* pRenderer = 
		&m_ctrlRibbonCategoryBtnPage[pScroll->IsLeftScroll () ? 0 : 1];
	int index = 0;

	if (pScroll->IsPressed ())
	{
		index = 1;
		if (pScroll->IsHighlighted ())
		{
			index = 2;
		}
	}
	else if (pScroll->IsHighlighted ())
	{
		index = 1;
	}

	pRenderer->Draw (pDC, rect, index);
	
	BOOL bIsLeft = pScroll->IsLeftScroll ();
	if (globalData.m_bIsRTL)
	{
		bIsLeft = !bIsLeft;
	}

	CBCGPMenuImages::Draw (pDC,
		bIsLeft ? CBCGPMenuImages::IdArowLeftLarge : CBCGPMenuImages::IdArowRightLarge, 
		rect);
}
//*****************************************************************************
COLORREF CBCGPVisualManager2007::OnDrawRibbonCategoryTab (
					CDC* pDC, 
					CBCGPRibbonTab* pTab,
					BOOL bIsActive)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnDrawRibbonCategoryTab (pDC, 
											pTab, bIsActive);
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pTab);

	CBCGPRibbonCategory* pCategory = pTab->GetParentCategory ();
	ASSERT_VALID (pCategory);
	CBCGPRibbonBar* pBar = pCategory->GetParentRibbonBar ();
	ASSERT_VALID (pBar);

	bIsActive = bIsActive && 
		((pBar->GetHideFlags () & BCGPRIBBONBAR_HIDE_ELEMENTS) == 0 || pTab->GetDroppedDown () != NULL);

	const BOOL bPressed     = pTab->IsPressed ();
	const BOOL bIsFocused   = pTab->IsFocused () && (pBar->GetHideFlags () & BCGPRIBBONBAR_HIDE_ELEMENTS);
	const BOOL bIsHighlight = (pTab->IsHighlighted () || bIsFocused) && !pTab->IsDroppedDown ();

	CRect rectTab (pTab->GetRect ());
	rectTab.bottom++;

	int ratio = 0;
	if (!IsBeta () && m_ctrlRibbonCategoryTabSep.IsValid ())
	{
		ratio = pBar->GetTabTrancateRatio ();
	}

	if (ratio > 0)
	{
		rectTab.left++;
	}

	int nImage = bIsActive ? 3 : 0;

	if (bPressed)
	{
		if (bIsHighlight)
		{
			nImage = bIsActive ? 2 : 1;
		}
	}
	
	if(bIsHighlight)
	{
		nImage += 1;
	}

	CBCGPControlRenderer* pRenderer = &m_ctrlRibbonCategoryTab;
	COLORREF clrText = m_clrRibbonCategoryText;
	COLORREF clrTextHighlighted = m_clrRibbonCategoryTextHighlighted;
	COLORREF clrTextActive = m_clrRibbonCategoryTextActive;

	if (pCategory->GetTabColor () != BCGPCategoryColor_None || pTab->IsSelected ())
	{
		XRibbonContextCategory& context = 
				m_ctrlRibbonContextCategory [
				(pTab->IsSelected () || nImage == 4)
					? BCGPCategoryColor_Orange - 1
					: pCategory->GetTabColor () - 1];

		pRenderer = &context.m_ctrlTab;
		clrText   = context.m_clrText;
		clrTextHighlighted = context.m_clrTextHighlighted;
		clrTextActive = context.m_clrTextActive;
	}

	pRenderer->Draw (pDC, rectTab, nImage);

	if (ratio > 0)
	{
		CRect rectSep (rectTab);
		rectSep.left = rectSep.right;
		rectSep.right++;
		rectSep.bottom--;

		m_ctrlRibbonCategoryTabSep.Draw (pDC, rectSep, 0, (BYTE)min(ratio * 255 / 100, 255));
	}

	if (clrTextActive == (COLORREF)-1)
	{
		return bIsActive
				? clrTextHighlighted
				: clrText;
	}

	return bIsActive
			? clrTextActive
			: bIsHighlight
				? clrTextHighlighted
				: clrText;
}
//****************************************************************************
COLORREF CBCGPVisualManager2007::OnDrawRibbonPanel (
		CDC* pDC,
		CBCGPRibbonPanel* pPanel, 
		CRect rectPanel,
		CRect rectCaption)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnDrawRibbonPanel (pDC, 
											pPanel, rectPanel, rectCaption);
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pPanel);

	COLORREF clrText = m_clrRibbonPanelText;

	if (pPanel->IsKindOf (RUNTIME_CLASS(CBCGPRibbonMainPanel)))
	{
		if (!pPanel->IsBackstageView())
		{
			int nBorderSize = GetPopupMenuBorderSize ();
			if (pPanel->IsScenicLook ())
			{
				nBorderSize = m_ctrlRibbonMainPanel.GetParams ().m_rectSides.left;
			}

			rectPanel.InflateRect (nBorderSize, nBorderSize);
			
			m_ctrlRibbonMainPanel.Draw (pDC, rectPanel);
		}
		else
		{
			rectPanel = ((CBCGPRibbonMainPanel*)pPanel)->GetCommandsFrame ();
			{
				CBCGPDrawManager dm (*pDC);
				dm.FillGradient (rectPanel,	m_clrToolBarGradientDark,
											m_clrToolBarGradientLight,
											TRUE);
			}

			rectPanel.right--;
			rectPanel.left = rectPanel.right;

			CPen* pOldPen = pDC->SelectObject (&m_penBottomLine);
			ASSERT (pOldPen != NULL);

			pDC->MoveTo (rectPanel.TopLeft ());
			pDC->LineTo (rectPanel.BottomRight ());

			pDC->SelectObject (pOldPen);
		}
	}
	else
	{
		BOOL bHighlighted = pPanel->IsHighlighted ();

		if (bHighlighted)
		{
			clrText = m_clrRibbonPanelTextHighlighted;
		}

		if (!pPanel->IsScenicLook ())
		{
			CBCGPControlRenderer* pRendererB = &m_ctrlRibbonPanelBack_B;
			CBCGPControlRenderer* pRendererT = &m_ctrlRibbonPanelBack_T;
			CBCGPBitmapCache* pCacheB = &m_cacheRibbonPanelBack_B;
			CBCGPBitmapCache* pCacheT = &m_cacheRibbonPanelBack_T;

			CBCGPRibbonCategory* pCategory = pPanel->GetParentCategory ();
			ASSERT_VALID (pCategory);

			CBCGPBaseRibbonElement* pParentButton = pPanel->GetParentButton ();

			if (pCategory->GetTabColor () != BCGPCategoryColor_None &&
				(pParentButton == NULL || !pParentButton->IsQATMode ()))
			{
				pRendererB = &m_ctrlRibbonContextPanelBack_B;
				pRendererT = &m_ctrlRibbonContextPanelBack_T;
				pCacheB = &m_cacheRibbonContextPanelBack_B;
				pCacheT = &m_cacheRibbonContextPanelBack_T;

				clrText = bHighlighted 
							? m_clrRibbonContextPanelTextHighlighted
							: m_clrRibbonContextPanelText;
			}

			if (!pPanel->IsCollapsed ())
			{
				CRect rect (rectPanel);

				BOOL bDrawCaption = rectCaption.Height () > 0 && pRendererT->IsValid ();

				if (bDrawCaption)
				{
					BOOL bBottomEnabled = pRendererB->IsValid ();

					if (bBottomEnabled)
					{
						rect.bottom -= rectCaption.Height () == 0
								? pRendererB->GetParams ().m_rectImage.Height ()
								: rectCaption.Height ();
					}

					{
						const CBCGPControlRendererParams& params = pRendererT->GetParams ();

						int nCacheIndex = -1;
						if (pCacheT != NULL)
						{
							CSize size (params.m_rectImage.Width (), rect.Height ());
							nCacheIndex = pCacheT->FindIndex (size);
							if (nCacheIndex == -1)
							{
								nCacheIndex = pCacheT->CacheY (size.cy, *pRendererT);
							}
						}

						if (nCacheIndex != -1)
						{
							pCacheT->Get(nCacheIndex)->DrawY (pDC, rect, 
								CSize (params.m_rectInter.left, params.m_rectImage.right - params.m_rectInter.right),
								bHighlighted ? 1 : 0);
						}
						else
						{
							pRendererT->Draw (pDC, rect, bHighlighted ? 1 : 0);
						}
					}	

					if (bBottomEnabled)
					{
						rect.top = rect.bottom;
						rect.bottom = rectPanel.bottom;

						const CBCGPControlRendererParams& params = pRendererB->GetParams ();

						int nCacheIndex = -1;
						if (pCacheB != NULL)
						{
							CSize size (params.m_rectImage.Width (), rect.Height ());
							nCacheIndex = pCacheB->FindIndex (size);
							if (nCacheIndex == -1)
							{
								nCacheIndex = pCacheB->CacheY (size.cy, *pRendererB);
							}
						}

						if (nCacheIndex != -1)
						{
							pCacheB->Get(nCacheIndex)->DrawY (pDC, rect, 
								CSize (params.m_rectInter.left, params.m_rectImage.right - params.m_rectInter.right),
								bHighlighted ? 1 : 0);
						}
						else
						{
							pRendererB->Draw (pDC, rect, bHighlighted ? 1 : 0);
						}
					}
				}
			}
		}
		else if (!pPanel->IsMenuMode () && !pPanel->IsCollapsed ())
		{
			rectPanel.left = rectPanel.right;
			rectPanel.right += 2;
			rectPanel.DeflateRect (0, 4);
			DrawSeparator (pDC, rectPanel, m_penSeparatorDark, m_penSeparatorLight, FALSE);			
		}
	}

	return clrText;
}
//****************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonPanelCaption (
		CDC* pDC,
		CBCGPRibbonPanel* pPanel, 
		CRect rectCaption)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonPanelCaption (pDC, 
											pPanel, rectCaption);
		return;
	}

	if (pPanel->IsKindOf (RUNTIME_CLASS(CBCGPRibbonMainPanel)))
	{
		return;
	}

	CString str = pPanel->GetDisplayName ();

	if (!str.IsEmpty ())
	{
		if (pPanel->GetLaunchButton ().GetID () > 0)
		{
			rectCaption.right = pPanel->GetLaunchButton ().GetRect ().left;

			rectCaption.DeflateRect (1, 1);
			rectCaption.OffsetRect (-1, -1);
		}
		else
		{
			rectCaption.DeflateRect (1, 1);

			if ((rectCaption.Width () % 2) == 0)
			{
				rectCaption.right--;
			}

			rectCaption.OffsetRect (0, -1);
		}

		COLORREF clrRibbonPanelCaptionText = m_clrRibbonPanelCaptionText;
		COLORREF clrRibbonPanelCaptionTextHighlighted = m_clrRibbonPanelCaptionTextHighlighted;

		if (pPanel->IsScenicLook () && m_bUseScenicColors)
		{
			clrRibbonPanelCaptionText = m_clrRibbonPanelText;
			clrRibbonPanelCaptionTextHighlighted = m_clrRibbonPanelTextHighlighted;
		}

		COLORREF clrTextOld = pDC->SetTextColor (pPanel->IsHighlighted () ? 
			clrRibbonPanelCaptionTextHighlighted : clrRibbonPanelCaptionText);

		pDC->DrawText (	str, rectCaption, 
						DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

		pDC->SetTextColor (clrTextOld);
	}
}
//****************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonLaunchButton (
					CDC* pDC,
					CBCGPRibbonLaunchButton* pButton,
					CBCGPRibbonPanel* pPanel)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonLaunchButton (pDC, 
											pButton, pPanel);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);
	ASSERT_VALID (pPanel);

	CRect rect (pButton->GetRect ());

	if (!IsBeta ())
	{
		rect.right--;
		rect.bottom--;
	}

	BOOL bHighlighted = pButton->IsHighlighted () || pButton->IsFocused ();

	int index = 0;

	if (m_RibbonBtnLaunchIcon.GetCount () > 3)
	{
		if (pButton->IsDisabled ())
		{
			index = 3;
		}
		else if (pButton->IsPressed ())
		{
			if (bHighlighted)
			{
				index = 2;
			}
		}
		else if (bHighlighted)
		{
			index = 1;
		}
	}
	else
	{
		if (!pButton->IsDisabled ())
		{
			if (pButton->IsPressed ())
			{
				if (bHighlighted)
				{
					index = 2;
				}
			}
			else if (bHighlighted)
			{
				index = 1;
			}
		}
	}

	if (m_ctrlRibbonBtnLaunch.IsValid ())
	{
		m_ctrlRibbonBtnLaunch.Draw (pDC, rect, index);
	}

	if (m_RibbonBtnLaunchIcon.IsValid ())
	{
		m_RibbonBtnLaunchIcon.DrawEx (pDC, rect, index,
			CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
	}
}
//****************************************************************************
COLORREF CBCGPVisualManager2007::OnFillRibbonButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnFillRibbonButton (pDC, pButton);
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	BOOL bIsMenuMode = pButton->IsMenuMode ();

	CRect rect (pButton->GetRect ());

	CBCGPControlRenderer* pRenderer = NULL;
	CBCGPBitmapCache* pCache = NULL;
	int index = 0;

	BOOL bDisabled    = pButton->IsDisabled ();
	BOOL bWasDisabled = bDisabled;

	if (bDisabled && pButton->HasMenu ())
	{
		bDisabled = FALSE;
	}

	BOOL bFocused     = pButton->IsFocused ();
	BOOL bDroppedDown = pButton->IsDroppedDown ();
	BOOL bPressed     = pButton->IsPressed () && !bIsMenuMode;
	BOOL bChecked     = pButton->IsChecked ();
	BOOL bHighlighted = pButton->IsHighlighted () || bFocused;

	if (!bDisabled && pButton->IsBackstageViewMode () && !pButton->IsQATMode() &&
		pButton->GetBackstageAttachedView() != NULL && bChecked)
	{
		bIsMenuMode  = FALSE;
		bChecked     = TRUE;
		bHighlighted = FALSE;
	}

	BOOL bDefaultPanelButton = pButton->IsDefaultPanelButton () && !pButton->IsQATMode () && !pButton->IsSearchResultMode ();
	if (bFocused)
	{
		bDisabled = FALSE;
	}

	if (bDroppedDown && !bIsMenuMode)
	{
		bChecked     = TRUE;
		bPressed     = FALSE;
		bHighlighted = FALSE;
	}

	CBCGPBaseRibbonElement::RibbonElementLocation location = 
		pButton->GetLocationInGroup ();

	if (pButton->IsKindOf (RUNTIME_CLASS(CBCGPRibbonEdit)))
	{
		COLORREF color = GetRibbonEditBackgroundColor (
			const_cast<CBCGPRibbonEditCtrl*>(((CBCGPRibbonEdit*)pButton)->GetEditCtrl ()), bChecked || bHighlighted, FALSE, bDisabled);

		rect.left = pButton->GetCommandRect ().left;

		{
			CBCGPDrawManager dm (*pDC);
			dm.DrawRect (rect, color, (COLORREF)-1);
		}

		return (COLORREF)-1;
	}

	if (bChecked && bIsMenuMode && !pButton->IsPaletteIcon ())
	{
		bChecked = FALSE;
	}

	if (location != CBCGPBaseRibbonElement::RibbonElementNotInGroup &&
		pButton->IsShowGroupBorder ())
	{
		if (!pButton->GetMenuRect().IsRectEmpty ())
		{
			CRect rectC = pButton->GetCommandRect();
			CRect rectM = pButton->GetMenuRect();

			CBCGPControlRenderer* pRendererC = NULL;
			CBCGPControlRenderer* pRendererM = NULL;

			CBCGPBitmapCache* pCacheC = NULL;
			CBCGPBitmapCache* pCacheM = NULL;

			if (location == CBCGPBaseRibbonElement::RibbonElementSingleInGroup)
			{
				pRendererC = &m_ctrlRibbonBtnGroupMenu_F[0];
				pRendererM = &m_ctrlRibbonBtnGroupMenu_L[1];

				pCacheC = &m_cacheRibbonBtnGroupMenu_F[0];
				pCacheM = &m_cacheRibbonBtnGroupMenu_L[1];
			}
			else if (location == CBCGPBaseRibbonElement::RibbonElementFirstInGroup)
			{
				pRendererC = &m_ctrlRibbonBtnGroupMenu_F[0];
				pRendererM = &m_ctrlRibbonBtnGroupMenu_F[1];

				pCacheC = &m_cacheRibbonBtnGroupMenu_F[0];
				pCacheM = &m_cacheRibbonBtnGroupMenu_F[1];
			}
			else if (location == CBCGPBaseRibbonElement::RibbonElementLastInGroup)
			{
				pRendererC = &m_ctrlRibbonBtnGroupMenu_L[0];
				pRendererM = &m_ctrlRibbonBtnGroupMenu_L[1];

				pCacheC = &m_cacheRibbonBtnGroupMenu_L[0];
				pCacheM = &m_cacheRibbonBtnGroupMenu_L[1];
			}
			else
			{
				pRendererC = &m_ctrlRibbonBtnGroupMenu_M[0];
				pRendererM = &m_ctrlRibbonBtnGroupMenu_M[1];

				pCacheC = &m_cacheRibbonBtnGroupMenu_M[0];
				pCacheM = &m_cacheRibbonBtnGroupMenu_M[1];
			}

			int indexC = 0;
			int indexM = 0;

			BOOL bHighlightedC = pButton->IsCommandAreaHighlighted ();
			BOOL bHighlightedM = pButton->IsMenuAreaHighlighted ();

			if (IsBeta ())
			{
				if (bChecked)
				{
					indexC = 3;
				}

				if (bDisabled)
				{
					indexC = 0;
				}
				else
				{
					if (bDroppedDown && !bIsMenuMode)
					{
						indexC = pButton->IsChecked () ? 3 : 0;
						indexM = 3;
					}
					else
					{
						if (bPressed)
						{
							if (bHighlightedC)
							{
								indexM = 1;
								indexC = 2;
							}
						}
						else if (bHighlighted)
						{
							indexC++;
							indexM = 1;
						}
					}
				}
			}
			else
			{
				if (bChecked)
				{
					indexC = 3;

					if (bHighlighted)
					{
						indexM = 5;
					}
				}

				if (bDisabled)
				{
					if (bChecked)
					{
						indexC = 5;
						indexM = 4;
					}
				}
				else
				{
					if (bDroppedDown && !bIsMenuMode)
					{
						indexC = pButton->IsChecked () ? 3 : 6;
						indexM = 3;
					}
					else
					{
						if (bFocused)
						{
							indexC = 6;
							indexM = 5;
						}

						if (bHighlightedC || bHighlightedM)
						{
							if (bChecked)
							{
								indexC = bHighlightedC ? 4 : 3;
							}
							else
							{
								indexC = bHighlightedC ? 1 : 6;
							}

							indexM = bHighlightedM ? 1 : 5;
						}

						if (bPressed)
						{
							if (bHighlightedC)
							{
								indexC = 2;
							}
						}
					}
				}
			}

			if (indexC != -1 && indexM != -1)
			{
				int nCacheIndex = -1;
				if (pCacheC != NULL)
				{
					CSize size (rectC.Size ());
					nCacheIndex = pCacheC->FindIndex (size);
					if (nCacheIndex == -1)
					{
						nCacheIndex = pCacheC->Cache (size, *pRendererC);
					}
				}

				if (nCacheIndex != -1)
				{
					pCacheC->Get (nCacheIndex)->Draw (pDC, rectC, indexC);
				}
				else
				{
					pRendererC->Draw (pDC, rectC, indexC);
				}

				nCacheIndex = -1;
				if (pCacheM != NULL)
				{
					CSize size (rectM.Size ());
					nCacheIndex = pCacheM->FindIndex (size);
					if (nCacheIndex == -1)
					{
						nCacheIndex = pCacheM->Cache (size, *pRendererM);
					}
				}

				if (nCacheIndex != -1)
				{
					pCacheM->Get (nCacheIndex)->Draw (pDC, rectM, indexM);
				}
				else
				{
					pRendererM->Draw (pDC, rectM, indexM);
				}
			}

			return (COLORREF)-1;
		}
		else
		{
			if (location == CBCGPBaseRibbonElement::RibbonElementSingleInGroup)
			{
				pRenderer = &m_ctrlRibbonBtnGroup_S;
				pCache    = &m_cacheRibbonBtnGroup_S;
			}
			else if (location == CBCGPBaseRibbonElement::RibbonElementFirstInGroup)
			{
				pRenderer = &m_ctrlRibbonBtnGroup_F;
				pCache    = &m_cacheRibbonBtnGroup_F;
			}
			else if (location == CBCGPBaseRibbonElement::RibbonElementLastInGroup)
			{
				pRenderer = &m_ctrlRibbonBtnGroup_L;
				pCache    = &m_cacheRibbonBtnGroup_L;
			}
			else
			{
				pRenderer = &m_ctrlRibbonBtnGroup_M;
				pCache    = &m_cacheRibbonBtnGroup_M;
			}

			if (bChecked)
			{
				index = 3;
			}

			if (bDisabled && !bFocused)
			{
				index = 0;
			}
			else
			{
				if (bPressed)
				{
					if (bHighlighted)
					{
						index = 2;
					}
				}
				else if (bHighlighted)
				{
					index++;
				}
			}
		}
	}
	else if (bDefaultPanelButton)
	{
		if (bPressed)
		{
			if (bHighlighted)
			{
				index = 2;
			}
		}
		else if (bHighlighted)
		{
			index = 1;
		}
		else if (bChecked)
		{
			index = 2;
		}

		if (bFocused && !bDroppedDown && m_ctrlRibbonBtnDefault.GetImageCount () > 3)
		{
			index = 3;
		}

		if (index != -1)
		{
			pRenderer = &m_ctrlRibbonBtnDefault;
			CBCGPBitmapCache* pCache = &m_cacheRibbonBtnDefault;

			CBCGPRibbonCategory* pCategory = pButton->GetParentCategory ();
			if (pCategory != NULL)
			{
				ASSERT_VALID (pCategory);

				if (pCategory->GetTabColor () != BCGPCategoryColor_None)
				{
					XRibbonContextCategory& context = 
						m_ctrlRibbonContextCategory[pCategory->GetTabColor () - 1];

					pRenderer = &context.m_ctrlBtnDefault;
					pCache    = &context.m_cacheBtnDefault;
				}
			}

			const CBCGPControlRendererParams& params = pRenderer->GetParams ();

			int nCacheIndex = -1;
			if (pCache != NULL)
			{
				CSize size (params.m_rectImage.Width (), rect.Height ());
				nCacheIndex = pCache->FindIndex (size);
				if (nCacheIndex == -1)
				{
					nCacheIndex = pCache->CacheY (size.cy, *pRenderer);
				}
			}

			if (nCacheIndex != -1)
			{
				pCache->Get(nCacheIndex)->DrawY (pDC, rect, 
					CSize (params.m_rectInter.left, params.m_rectImage.right - params.m_rectInter.right),
					index);

				return m_clrToolBarBtnTextHighlighted;
			}
		}
	}
	else if ((!bDisabled && (bPressed || bChecked || bHighlighted || bDroppedDown)) || 
		    (bDisabled && bFocused))
	{
		if (!pButton->GetMenuRect().IsRectEmpty ())
		{
			CRect rectC = pButton->GetCommandRect();
			CRect rectM = pButton->GetMenuRect();

			CBCGPControlRenderer* pRendererC = pButton->IsMenuOnBottom () 
				? &m_ctrlRibbonBtnMenuV[0]
				: &m_ctrlRibbonBtnMenuH[0];
			CBCGPControlRenderer* pRendererM = pButton->IsMenuOnBottom () 
				? &m_ctrlRibbonBtnMenuV[1]
				: &m_ctrlRibbonBtnMenuH[1];

			int indexC = -1;
			int indexM = -1;

			BOOL bHighlightedC = pButton->IsCommandAreaHighlighted ();
			BOOL bHighlightedM = pButton->IsMenuAreaHighlighted ();

			if (IsBeta ())
			{
				if (bChecked)
				{
					indexC = 2;
					indexM = 2;
				}

				if (bDisabled)
				{
					if (bChecked)
					{
						// TODO
					}
				}
				else
				{
					if (bDroppedDown && !bIsMenuMode)
					{
						indexC = bChecked ? 2 : 4;
						indexM = 2;
					}
					else
					{
						if (bPressed)
						{
							if (bHighlighted)
							{
								if (bHighlightedC)
								{
									indexC = 1;
								}
								else
								{
									indexC = bChecked ? indexC : 0;
								}

								indexM = bChecked ? indexM : 0;
							}
						}
						else if (bHighlighted)
						{
							indexC++;
							indexM++;
						}
					}
				}
			}
			else
			{
				if (bDisabled)
				{
					if (bHighlightedC || bHighlightedM)
					{
						indexC = 4;
						indexM = 4;

						if (bHighlightedM)
						{
							indexM = 0;

							if (bDroppedDown && !bIsMenuMode)
							{
								indexC = 5;
								indexM = 2;
							}
							else if (bPressed)
							{
								indexM = 1;
							}
						}
					}
				}
				else
				{
					if (bDroppedDown && !bIsMenuMode)
					{
						indexC = 5;
						indexM = 2;
					}
					else
					{
						if (bFocused)
						{
							indexC = 5;
							indexM = 4;
						}

						if (bChecked)
						{
							indexC = 2;
							indexM = 2;
						}

						if (bHighlightedC || bHighlightedM)
						{
							indexM = 4;

							if (bPressed)
							{
								if (bHighlightedC)
								{
									indexC = 1;
								}
								else if (bHighlightedM)
								{
									indexC = bChecked ? 3 : 5;
								}
							}
							else
							{
								indexC = bChecked ? 3 : 0;

								if (bHighlightedM)
								{
									indexC = bChecked ? 3 : 5;
									indexM = 0;
								}
							}
						}
					}
				}
			}

			if (indexC != -1)
			{
				pRendererC->Draw (pDC, rectC, indexC);
			}

			if (indexM != -1)
			{
				pRendererM->Draw (pDC, rectM, indexM);
			}

			return (COLORREF)-1;
		}
		else
		{
			index = -1;

			pRenderer = &m_ctrlRibbonBtn[0];
			if (rect.Height () > pRenderer->GetParams ().m_rectImage.Height () * 1.5 &&
				m_ctrlRibbonBtn[1].IsValid ())
			{
				pRenderer = &m_ctrlRibbonBtn[1];
			}

			if (bDisabled && bFocused)
			{
				if (pRenderer->GetImageCount () > 4)
				{
					index = 4;
				}
				else
				{
					index = 0;
				}
			}
			
			if (!bDisabled)
			{
				if (bChecked)
				{
					index = 2;
				}

				if (bPressed)
				{
					if (bHighlighted)
					{
						index = 1;
					}
				}
				else if (bHighlighted)
				{
					index++;
				}
			}
		}
	}

	COLORREF clrText = bWasDisabled
		? m_clrRibbonBarBtnTextDisabled
		: pButton->IsStatusBarMode() 
		? m_clrStatusBarText : m_clrRibbonBarBtnText;

	if (!bWasDisabled && pButton->GetParentGroup () != NULL)
	{
		clrText = (COLORREF)-1;
	}

	if (pRenderer != NULL)
	{
		if (index != -1)
		{
			int nCacheIndex = -1;
			if (pCache != NULL)
			{
				CSize size (rect.Size ());
				nCacheIndex = pCache->FindIndex (size);
				if (nCacheIndex == -1)
				{
					nCacheIndex = pCache->Cache (size, *pRenderer);
				}
			}

			if (nCacheIndex != -1)
			{
				pCache->Get (nCacheIndex)->Draw (pDC, rect, index);
			}
			else
			{
				pRenderer->Draw (pDC, rect, index);
			}

			if (!bWasDisabled)
			{
				clrText = m_clrRibbonBarBtnTextHighlighted;
			}
		}
	}

	return clrText;
}
//****************************************************************************
COLORREF CBCGPVisualManager2007::OnFillRibbonPinnedButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton, BOOL& bIsDarkPin)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnFillRibbonPinnedButton (pDC, pButton, bIsDarkPin);
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	bIsDarkPin = TRUE;

	if (pButton->IsCommandAreaHighlighted() || pButton->IsFocused())
	{
		m_ctrlRibbonBtn[0].Draw(pDC, pButton->GetRect(), 0);
	}
	else if (pButton->IsMenuAreaHighlighted())
	{
		m_ctrlRibbonBtn[0].Draw(pDC, pButton->GetMenuRect(), 0);
	}

	return (COLORREF)-1;
}
//****************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonButtonBorder (
					CDC* pDC, 
					CBCGPRibbonButton* pButton)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonButtonBorder (pDC, pButton);
	}

	if (pButton->IsKindOf (RUNTIME_CLASS(CBCGPRibbonEdit)))
	{
		CRect rect (pButton->GetRect ());

		COLORREF colorBorder = m_clrRibbonEditBorder;

		if (pButton->IsDisabled ())
		{
			colorBorder = m_clrRibbonEditBorderDisabled;
		}
		else if (pButton->IsHighlighted () || pButton->IsDroppedDown () || pButton->IsFocused ())
		{
			colorBorder = pButton->IsDroppedDown ()
				? m_clrRibbonEditBorderPressed
				: m_clrRibbonEditBorderHighlighted;
		}

		rect.left = pButton->GetCommandRect ().left;

		if (CBCGPToolBarImages::m_bIsDrawOnGlass)
		{
			CBCGPDrawManager dm (*pDC);
			dm.DrawRect (rect, (COLORREF)-1, colorBorder);
		}
		else
		{
			pDC->Draw3dRect (rect, colorBorder, colorBorder);
		}
	}
}
//****************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonPinnedButtonBorder (
					CDC* pDC, 
					CBCGPRibbonButton* pButton)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonButtonBorder (pDC, pButton);
	}
}
//***********************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonMenuCheckFrame (
					CDC* pDC,
					CBCGPRibbonButton* pButton, 
					CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonMenuCheckFrame (pDC, pButton, rect);
		return;
	}

	ASSERT_VALID (pDC);

	m_ctrlMenuItemBack.Draw (pDC, rect);
}
//****************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonDefaultPaneButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonDefaultPaneButton (pDC, pButton);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (pButton->IsSearchResultMode ())
	{
		int index = pButton->IsHighlighted() ? 1 : pButton->IsPressed() ? 2 : 0;
		m_ctrlRibbonBtnPush.Draw (pDC, pButton->GetRect(), index);

		OnDrawRibbonDefaultPaneButtonContext (pDC, pButton);
		return;
	}

	OnFillRibbonButton (pDC, pButton);

	BOOL bIsQATMode = pButton->IsQATMode ();

	CRect rectFrame (pButton->GetRect ());

	if (!bIsQATMode)
	{
		if (m_ctrlRibbonBtnDefaultIcon.IsValid ())
		{
			const CSize sizeImage = pButton->GetImageSize (CBCGPRibbonButton::RibbonImageSmall);
			const int nMarginX = 11;
			const int nMarginY = 10;
			
			rectFrame.top += nMarginY / 2;
			rectFrame.bottom = rectFrame.top + sizeImage.cy + 2 * nMarginY;
			rectFrame.top -= 2;
			rectFrame.left = rectFrame.CenterPoint ().x - sizeImage.cx / 2 - nMarginX;
			rectFrame.right = rectFrame.left + sizeImage.cx + 2 * nMarginX;

			m_ctrlRibbonBtnDefaultIcon.Draw (pDC, rectFrame);
		}
	}
	else
	{
		if (m_ctrlRibbonBtnDefaultQAT.IsValid ())
		{
			int index = 0;
			if (pButton->IsDroppedDown ())
			{
				index = 2;
			}
			else if (pButton->IsPressed ())
			{
				if (pButton->IsHighlighted ())
				{
					index = 2;
				}
			}
			else if (pButton->IsHighlighted () || pButton->IsFocused ())
			{
				index = 1;
			}

			m_ctrlRibbonBtnDefaultQAT.Draw (pDC, rectFrame, index);
		}
		else if (m_ctrlRibbonBtnDefaultQATIcon.IsValid ())
		{
			int nHeight = m_ctrlRibbonBtnDefaultQATIcon.GetParams ().m_rectImage.Height ();

			if (rectFrame.Height () < nHeight)
			{
				nHeight = rectFrame.Height () / 2;
			}

			rectFrame.DeflateRect (1, 0);
			rectFrame.top = rectFrame.bottom - nHeight;

			m_ctrlRibbonBtnDefaultQATIcon.Draw (pDC, rectFrame);
		}
	}

	OnDrawRibbonDefaultPaneButtonContext (pDC, pButton);
}
//****************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonDefaultPaneButtonIndicator (
					CDC* pDC, 
					CBCGPRibbonButton* pButton,
					CRect rect, 
					BOOL bIsSelected, 
					BOOL bHighlighted)
{
	if (!CanDrawImage () || !m_ctrlRibbonBtnDefaultIcon.IsValid ())
	{
		CBCGPVisualManager2003::OnDrawRibbonDefaultPaneButtonIndicator (
					pDC, pButton, rect, bIsSelected, bHighlighted);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	rect.left = rect.right - m_ctrlRibbonBtnDefaultIcon.GetParams ().m_rectImage.Width ();
	m_ctrlRibbonBtnDefaultIcon.Draw (pDC, rect);

	CRect rectWhite = rect;
	rectWhite.OffsetRect (0, 1);

	CBCGPMenuImages::Draw (pDC, CBCGPMenuImages::IdArowDown, rectWhite,
		CBCGPMenuImages::ImageWhite);

	CBCGPMenuImages::Draw (pDC, CBCGPMenuImages::IdArowDown, rect,
		CBCGPMenuImages::ImageBlack);
}
//***********************************************************************************
COLORREF CBCGPVisualManager2007::GetRibbonEditBackgroundColor (
					CBCGPRibbonEditCtrl* pEdit,
					BOOL bIsHighlighted,
					BOOL bIsPaneHighlighted,
					BOOL bIsDisabled)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetRibbonEditBackgroundColor ( 
			pEdit, bIsHighlighted, bIsPaneHighlighted, bIsDisabled);
	}

	COLORREF color = m_clrRibbonEdit;

	if (bIsDisabled)
	{
		color = m_clrRibbonEditDisabled;
	}
	else
	{
		if (bIsHighlighted)
		{
			color = m_clrRibbonEditHighlighted;
		}
	}

	return color;
}
//***********************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonMainPanelFrame (
					CDC* pDC, 
					CBCGPRibbonMainPanel* pPanel,
					CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonMainPanelFrame (pDC, 
					pPanel, rect);
		return;
	}

	if (!IsBeta ())
	{
		ASSERT_VALID (pDC);

		rect.right += m_ctrlRibbonMainPanelBorder.GetParams ().m_rectSides.right; //TODO
		m_ctrlRibbonMainPanelBorder.DrawFrame (pDC, rect);
	}
}
//***********************************************************************************
void CBCGPVisualManager2007::OnFillRibbonMenuFrame (
					CDC* pDC, 
					CBCGPRibbonMainPanel* pPanel,
					CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillRibbonMenuFrame (pDC, 
					pPanel, rect);
		return;
	}

	ASSERT_VALID (pDC);

	pDC->FillRect (rect, &m_brMenuLight);
}
//***********************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonRecentFilesFrame (
					CDC* pDC, 
					CBCGPRibbonMainPanel* pPanel,
					CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonRecentFilesFrame (pDC, 
					pPanel, rect);
		return;
	}

	ASSERT_VALID (pDC);

	rect.right += 2; //TODO
	pDC->FillRect (rect, &m_brRibbonMainPanelBkgnd);

	CRect rectSeparator = rect;
	rectSeparator.right = rectSeparator.left + 2;

	DrawSeparator (pDC, rectSeparator, FALSE);
}
//***********************************************************************************
COLORREF CBCGPVisualManager2007::OnFillRibbonMainPanelButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnFillRibbonMainPanelButton (pDC, pButton);
	}

	BOOL bHighlighted = pButton->IsHighlighted ();

	COLORREF clrText = bHighlighted
						? m_clrMenuTextHighlighted
						: pButton->IsDisabled () 
							? m_clrMenuTextDisabled
							: m_clrMenuText;

	const int index = bHighlighted ? 1 : 0;
	m_ctrlRibbonBtnMainPanel.Draw (pDC, pButton->GetRect (), index);

	return clrText;
}
//****************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonMainPanelButtonBorder (
		CDC* pDC, CBCGPRibbonButton* pButton)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonMainPanelButtonBorder (pDC, pButton);
		return;
	}
}
//***********************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonPaletteButton (
					CDC* pDC, 
					CBCGPRibbonPaletteIcon* pButton)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonPaletteButton (pDC, pButton);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	int index = 0;
	if (pButton->IsDisabled ())
	{
		index = 3;
	}
	else
	{
		if (pButton->IsPressed ())
		{
			if (pButton->IsHighlighted ())
			{
				index = 2;
			}
		}
		else if (pButton->IsHighlighted () || pButton->IsFocused ())
		{
			index = 1;
		}
	}

	int nBtn = 1;
	if (pButton->IsLast ())
	{
		nBtn = 2;
	}
	else if (pButton->IsFirst ())
	{
		nBtn = 0;
	}

	m_ctrlRibbonBtnPalette[nBtn].Draw (pDC, pButton->GetRect (), index);
}
//***********************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonPaletteBorder (
				CDC* pDC, 
				CBCGPRibbonPaletteButton* pButton, 
				CRect rectBorder)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonPaletteBorder (pDC, pButton, rectBorder);
		return;
	}

	rectBorder.right -= 5;

	ASSERT_VALID (pDC);

	CBCGPDrawManager dm (*pDC);
	dm.DrawRect (rectBorder, m_clrRibbonEdit, m_clrRibbonEditBorder);
}
//*******************************************************************************
COLORREF CBCGPVisualManager2007::OnDrawRibbonCategoryCaption (
				CDC* pDC, 
				CBCGPRibbonContextCaption* pContextCaption)
{
	if (!CanDrawImage () || pContextCaption == NULL || pContextCaption->GetColor () == BCGPCategoryColor_None)
	{
		return CBCGPVisualManager2003::OnDrawRibbonCategoryCaption (pDC, pContextCaption);
	}

	XRibbonContextCategory& context = 
		m_ctrlRibbonContextCategory[pContextCaption->GetColor () - 1];

	CRect rect (pContextCaption->GetRect ());
	context.m_ctrlCaption.Draw (pDC, rect);

	int xTabRight = pContextCaption->GetRightTabX ();

	if (xTabRight > 0 && pContextCaption->GetParentRibbonBar () != NULL && 
		pContextCaption->GetParentRibbonBar ()->GetActiveCategory () != NULL)
	{
		CRect rectTab (pContextCaption->GetParentRibbonBar ()->GetActiveCategory ()->GetTabRect ());
		rect.top = rectTab.top;
		rect.bottom = rectTab.bottom;
		rect.right = xTabRight;

		m_ctrlRibbonContextSeparator.DrawFrame (pDC, rect);
	}

	return context.m_clrCaptionText;
}
//********************************************************************************
COLORREF CBCGPVisualManager2007::OnDrawRibbonStatusBarPane (CDC* pDC, CBCGPRibbonStatusBar* pBar,
					CBCGPRibbonStatusBarPane* pPane)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnDrawRibbonStatusBarPane (pDC, pBar, pPane);
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pBar);
	ASSERT_VALID (pPane);

	CRect rectPane = pPane->GetRect ();

	const BOOL bHighlighted = pPane->IsHighlighted () || pPane->IsFocused ();
	const BOOL bChecked     = pPane->IsChecked ();
	const BOOL bExtended	= pPane->IsExtended ();

	if (bHighlighted || bChecked)
	{
		CRect rectButton = rectPane;
		rectButton.DeflateRect (1, 1);

		int index = 0;
		if (pPane->IsPressed ())
		{
			if (bHighlighted)
			{
				index = 1;
			}
		}
		else if (bChecked)
		{
			if (bHighlighted)
			{
				index = 0;
			}
			else
			{
				index = 1;
			}
		}

		m_ctrlRibbonBtnStatusPane.Draw (pDC, rectButton, index);
	}

	if (pPane->IsDisabled ())
	{
		return bExtended ? m_clrExtenedStatusBarTextDisabled : m_clrStatusBarTextDisabled;
	}

	return bHighlighted ? 
		m_clrToolBarBtnTextHighlighted :
		m_clrStatusBarText;
}
//********************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonSliderZoomButton (
			CDC* pDC, CBCGPRibbonSlider* pSlider, 
			CRect rect, BOOL bIsZoomOut, 
			BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonSliderZoomButton (
			pDC, pSlider, rect, bIsZoomOut, bIsHighlighted, bIsPressed, bIsDisabled);
		return;
	}

	ASSERT_VALID (pDC);

	CBCGPControlRenderer* pRenderer = bIsZoomOut 
		? &m_ctrlRibbonSliderBtnMinus
		: &m_ctrlRibbonSliderBtnPlus;

	int index = 0;
	if (bIsDisabled)
	{
		index = 3;
	}
	else
	{
		if (bIsPressed)
		{
			if (bIsHighlighted)
			{
				index = 2;
			}
		}
		else if (bIsHighlighted)
		{
			index = 1;
		}
	}

	pRenderer->FillInterior (pDC, rect, 
		CBCGPToolBarImages::ImageAlignHorzCenter, 
		CBCGPToolBarImages::ImageAlignVertCenter,
		index);
}
//********************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonSliderChannel (
			CDC* pDC, CBCGPRibbonSlider* pSlider, 
			CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonSliderChannel (
			pDC, pSlider, rect);
		return;
	}

	ASSERT_VALID (pDC);

	DrawSeparator (pDC, rect, m_penSeparatorDark, m_penSeparator2, !pSlider->IsVert ());

	if (pSlider->IsVert ())
	{
		rect.top += rect.Height () / 2 - 1;
		rect.bottom = rect.top + 2;

		rect.InflateRect (2, 0);
	}
	else
	{
		rect.left += rect.Width () / 2 - 1;
		rect.right = rect.left + 2;

		rect.InflateRect (0, 2);
	}

	DrawSeparator (pDC, rect, m_penSeparatorDark, m_penSeparator2, pSlider->IsVert ());
}
//********************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonSliderThumb (
			CDC* pDC, CBCGPRibbonSlider* pSlider, 
			CRect rect, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonSliderThumb (
			pDC, pSlider, rect, bIsHighlighted, bIsPressed, bIsDisabled);
		return;
	}

	ASSERT_VALID (pDC);

	BOOL bIsVert = FALSE;
	BOOL bLeftTop = FALSE;
	BOOL bRightBottom = FALSE;

	if (pSlider != NULL)
	{
		ASSERT_VALID (pSlider);

		bIsVert = pSlider->IsVert ();
		bLeftTop = pSlider->IsThumbLeftTop ();
		bRightBottom = pSlider->IsThumbRightBottom ();
	}

	int indexRen = 0;

	if (bLeftTop && bRightBottom)
	{
		indexRen = 1;
	}
	else if (bLeftTop)
	{
		indexRen = 2;
	}

	if (bIsVert)
	{
		indexRen += 3;
	}

	CBCGPControlRenderer* pRenderer = NULL;
	
	if (indexRen == 0)
	{
		pRenderer = &m_ctrlRibbonSliderThumb;
	}
	else
	{
		pRenderer = &m_ctrlRibbonSliderThumbA[indexRen - 1];
	}

	if (pRenderer != NULL)
	{
		int index = 0;

		if (bIsDisabled)
		{
			index = 3;
		}
		else
		{
			if (bIsPressed)
			{
				index = 2;
			}
			else if (bIsHighlighted)
			{
				index = 1;
			}
		}

		pRenderer->FillInterior (pDC, rect, 
			CBCGPToolBarImages::ImageAlignHorzCenter, 
			CBCGPToolBarImages::ImageAlignVertCenter,
			index);
	}
}
//********************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonProgressBar (CDC* pDC, 
												  CBCGPRibbonProgressBar* pProgress, 
												  CRect rectProgress, CRect rectChunk,
												  BOOL bInfiniteMode)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawRibbonProgressBar (pDC, pProgress, 
												  rectProgress, rectChunk, bInfiniteMode);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pProgress);

	BOOL bIsVertical = pProgress->IsVertical();

	CRect rectBorders;

	if (bIsVertical)
	{
		m_ctrlRibbonProgressBackV.Draw (pDC, rectProgress);
		rectBorders = m_ctrlRibbonProgressBackV.GetParams ().m_rectCorners;
	}
	else
	{
		m_ctrlRibbonProgressBack.Draw (pDC, rectProgress);
		rectBorders = m_ctrlRibbonProgressBack.GetParams ().m_rectCorners;
	}

	CRect rectInternal (rectProgress);
	rectInternal.DeflateRect (rectBorders.left, rectBorders.top, rectBorders.right, rectBorders.bottom);

	if (!bInfiniteMode)
	{
		// normal
		rectChunk.IntersectRect (rectChunk, rectInternal);

		if (!rectChunk.IsRectEmpty () || pProgress->GetPos () != pProgress->GetRangeMin ())
		{
			int nDC = pDC->SaveDC();

			CRect rectClip(0, 0, 0, 0);
			pDC->GetClipBox(rectClip);

			if (!rectClip.IsRectEmpty())
			{
				rectClip.IntersectRect(rectClip, rectInternal);
			}

			CRgn rgn;
			rgn.CreateRectRgnIndirect (rectClip);

			pDC->SelectClipRgn (&rgn);

			if (!rectChunk.IsRectEmpty ())
			{
				if (bIsVertical)
				{
					rectChunk.bottom = rectChunk.top + rectInternal.Height();
					m_ctrlRibbonProgressNormalV.Draw (pDC, rectChunk);
				}
				else
				{
					rectChunk.left = rectChunk.right - rectInternal.Width ();
					m_ctrlRibbonProgressNormal.Draw (pDC, rectChunk);
				}
			}
			else
			{
				rectChunk = rectInternal;

				if (bIsVertical)
				{
					rectChunk.bottom = rectInternal.top;
				}
				else
				{
					rectChunk.right = rectInternal.left;
				}
			}

			if (bIsVertical)
			{
				if (rectChunk.top != rectInternal.top)
				{
					rectChunk.bottom = rectChunk.top;
					rectChunk.top -= m_ctrlRibbonProgressNormalExtV.GetParams ().m_rectImage.Height ();
					
					m_ctrlRibbonProgressNormalExtV.Draw (pDC, rectChunk);
				}
			}
			else
			{
				if (rectChunk.right != rectInternal.right)
				{
					rectChunk.left = rectChunk.right;
					rectChunk.right += m_ctrlRibbonProgressNormalExt.GetParams ().m_rectImage.Width ();

					m_ctrlRibbonProgressNormalExt.Draw (pDC, rectChunk);
				}
			}

			pDC->SelectClipRgn (NULL);
			pDC->RestoreDC(nDC);
		}
	}
	else if (pProgress->GetRangeMax () != pProgress->GetRangeMin ())
	{
		CBCGPControlRenderer* pRenderer = bIsVertical ? &m_ctrlRibbonProgressInfinityV : &m_ctrlRibbonProgressInfinity;

		int count = pRenderer->GetImageCount();
		double range = pProgress->GetRangeMax () - pProgress->GetRangeMin () + 1;
		range = (double)(pProgress->GetPos () - pProgress->GetRangeMin ()) / range;

		pRenderer->Draw (pDC, rectInternal, bcg_clamp((int)(range * count), 0, count - 1));
	}
}
//*******************************************************************************
void CBCGPVisualManager2007::OnFillRibbonQATPopup (
				CDC* pDC, CBCGPRibbonPanelMenuBar* pMenuBar, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillRibbonQATPopup (pDC, pMenuBar, rect);
		return;
	}

	ASSERT_VALID (pDC);

	if (m_ctrlRibbonBorder_QAT.IsValid ())
	{
		m_ctrlRibbonBorder_QAT.FillInterior (pDC, rect);
	}
	else
	{
		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rect, m_clrBarGradientDark, m_clrBarGradientLight, TRUE);
	}
}
//*******************************************************************************
int CBCGPVisualManager2007::GetRibbonPopupBorderSize (const CBCGPRibbonPanelMenu* pPopup) const
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetRibbonPopupBorderSize (pPopup);
	}

	if (pPopup != NULL)
	{
		ASSERT_VALID (pPopup);

		CBCGPRibbonPanelMenuBar* pRibbonMenuBar = 
			DYNAMIC_DOWNCAST (CBCGPRibbonPanelMenuBar, 
				(const_cast<CBCGPRibbonPanelMenu*>(pPopup))->GetMenuBar ());

		if (pRibbonMenuBar != NULL)
		{
			if (pRibbonMenuBar->IsMainPanel ())
			{
				CBCGPRibbonBar* pBar = pRibbonMenuBar->GetTopLevelRibbonBar ();
				if (pBar != NULL && pBar->IsScenicLook ())
				{
					return m_ctrlRibbonMainPanel.GetParams ().m_rectSides.left;
				}

				return (int)GetPopupMenuBorderSize ();
			}

			if (!pRibbonMenuBar->IsMenuMode ())
			{
				if (pRibbonMenuBar->IsQATPopup ())
				{
					if (m_ctrlRibbonBorder_QAT.IsValid ())
					{
						return m_ctrlRibbonBorder_QAT.GetParams ().m_rectSides.left;
					}
					else if (!IsBeta1())
					{
						return 0;
					}
				}
				else if (pRibbonMenuBar->IsCategoryPopup ())
				{
					return 0;
				}
				else if (pRibbonMenuBar->IsFloaty ())
				{
					if (m_ctrlRibbonBorder_Floaty.IsValid ())
					{
						return m_ctrlRibbonBorder_Floaty.GetParams ().m_rectSides.left;
					}
				}
				else
				{
					if (pRibbonMenuBar->GetPanel () != NULL)
					{
						if (!IsBeta1 ())
						{
							if (m_ctrlRibbonBorder_Panel.IsValid ())
							{
								return m_ctrlRibbonBorder_Panel.GetParams ().m_rectSides.left;
							}

							return 0;
						}
					}

					// standard size
				}
			}
		}
	}

	return (int)GetPopupMenuBorderSize ();
}
//*******************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonKeyTip (CDC* pDC, 
												 CBCGPBaseRibbonElement* pElement, 
												 CRect rect, CString str)
{
	if (!CanDrawImage () ||
		!m_ctrlRibbonKeyTip.IsValid ())
	{
		CBCGPVisualManager2003::OnDrawRibbonKeyTip (pDC, pElement, rect, str);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pElement);

	BOOL bDisabled = pElement->IsDisabled ();

	m_ctrlRibbonKeyTip.Draw (pDC, rect, 0);

	str.MakeUpper ();

	COLORREF clrTextOld = pDC->SetTextColor (
		bDisabled ? m_clrRibbonKeyTipTextDisabled : m_clrRibbonKeyTipTextNormal);
	
	pDC->DrawText (str, rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

	pDC->SetTextColor (clrTextOld);
}
//********************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonCheckBoxOnList (CDC* pDC, CBCGPRibbonButton* pButton, 
													 CRect rect, BOOL bIsSelected, BOOL bHighlighted)
{
	ASSERT_VALID (pDC);

    CBCGPToolBarImages& img = m_MenuItemMarkerC;

	if (!CanDrawImage () || img.GetCount () == 0)
	{
		CBCGPVisualManager2003::OnDrawRibbonCheckBoxOnList (pDC, pButton, rect, bIsSelected, bHighlighted);
		return;
	}

	if (globalData.GetRibbonImageScale () != 1.0)
	{
		rect.DeflateRect (5, 5);
		img.DrawEx (pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzStretch, CBCGPToolBarImages::ImageAlignVertStretch);
	}
	else
	{
		img.DrawEx (pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
	}
}
//********************************************************************************
void CBCGPVisualManager2007::OnDrawRibbonRadioButtonOnList (CDC* pDC, CBCGPRibbonButton* pButton,
													 CRect rect, BOOL bIsSelected, BOOL bHighlighted)
{
	ASSERT_VALID (pDC);

    CBCGPToolBarImages& img = m_MenuItemMarkerR;

	if (!CanDrawImage () || img.GetCount () == 0)
	{
		CBCGPVisualManager2003::OnDrawRibbonRadioButtonOnList (pDC, pButton, rect, bIsSelected, bHighlighted);
		return;
	}

	if (globalData.GetRibbonImageScale () != 1.0)
	{
		rect.DeflateRect (5, 5);
		img.DrawEx (pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzStretch, CBCGPToolBarImages::ImageAlignVertStretch);
	}
	else
	{
		img.DrawEx (pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
	}
}
//********************************************************************************
COLORREF CBCGPVisualManager2007::GetRibbonHyperlinkTextColor (CBCGPRibbonHyperlink* pHyperLink)
{
	ASSERT_VALID (pHyperLink);

	if (!CanDrawImage () || pHyperLink->IsDisabled ())
	{
		return CBCGPVisualManager2003::GetRibbonHyperlinkTextColor (pHyperLink);
	}

	COLORREF clrText = pHyperLink->IsHighlighted () ? 
			m_clrRibbonHyperlinkActive : m_clrRibbonHyperlinkInactive;

	if (m_clrRibbonStatusbarHyperlinkActive != (COLORREF)-1 &&
		m_clrRibbonStatusbarHyperlinkInactive != (COLORREF)-1)
	{
		CBCGPRibbonStatusBar* pParentStatusBar = DYNAMIC_DOWNCAST (
			CBCGPRibbonStatusBar, pHyperLink->GetParentRibbonBar ());

		if (pParentStatusBar != NULL)
		{
			ASSERT_VALID (pParentStatusBar);

			if (!pParentStatusBar->IsExtendedElement (pHyperLink))
			{
				clrText = pHyperLink->IsHighlighted () ? 
					m_clrRibbonStatusbarHyperlinkActive : m_clrRibbonStatusbarHyperlinkInactive;
			}
		}
	}

	if (clrText == (COLORREF)-1)
	{
		return CBCGPVisualManager2003::GetRibbonHyperlinkTextColor (pHyperLink);
	}

	return clrText;
}
//********************************************************************************
COLORREF CBCGPVisualManager2007::GetRibbonStatusBarTextColor (CBCGPRibbonStatusBar* pStatusBar)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetRibbonStatusBarTextColor (pStatusBar);
	}

	return m_clrStatusBarText;
}

#endif // BCGP_EXCLUDE_RIBBON

#ifndef BCGP_EXCLUDE_PLANNER
//*******************************************************************************
void CBCGPVisualManager2007::OnFillPlanner (CDC* pDC, CBCGPPlannerView* pView, 
		CRect rect, BOOL bWorkingArea)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pView);

	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillPlanner (pDC, pView, rect, bWorkingArea);
		return;
	}

	if (m_bPlannerBackItemSelected)
	{
		CBrush br (GetPlannerHourLineColor (pView, TRUE, FALSE));
		pDC->FillRect (rect, &br);
	}
	else
	{
		CBCGPVisualManager2003::OnFillPlanner (pDC, pView, rect, bWorkingArea);
	}

	if (m_bPlannerBackItemToday && DYNAMIC_DOWNCAST (CBCGPPlannerViewDay, pView) == NULL)
	{
		rect.right--;
		pDC->Draw3dRect (rect, m_clrPlannerTodayBorder, m_clrPlannerTodayBorder);

		rect.left--;
		rect.right++;
		rect.bottom++;
		pDC->Draw3dRect (rect, m_clrPlannerTodayBorder, m_clrPlannerTodayBorder);
	}
}
//*******************************************************************************
COLORREF CBCGPVisualManager2007::OnFillPlannerCaption (CDC* pDC,
		CBCGPPlannerView* pView, CRect rect, BOOL bIsToday, BOOL bIsSelected,
		BOOL bNoBorder/* = FALSE*/, BOOL bHorz /*= TRUE*/)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnFillPlannerCaption (pDC,
			pView, rect, bIsToday, bIsSelected, bNoBorder, bHorz);
	}

	const BOOL bMonth = DYNAMIC_DOWNCAST(CBCGPPlannerViewMonth, pView) != NULL;

	if (bMonth && m_bPlannerCaptionBackItemHeader)
	{
		return m_clrPlannerNcText;
	}

	ASSERT_VALID (pDC);

	BOOL bDay = FALSE;

	if (!bMonth)
	{
		bDay = pView->IsKindOf (RUNTIME_CLASS (CBCGPPlannerViewDay));

		if (bDay)
		{
			if (!bIsToday)
			{
				rect.DeflateRect (1, 1);
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

	COLORREF clrText   = RGB (0, 0, 0);
	COLORREF clrBorder = CLR_DEFAULT;

	if (bIsToday)
	{
		CRect rectHalf (rect);

		CBCGPDrawManager dm (*pDC);

		if (bHorz)
		{
			rectHalf.bottom = rectHalf.top + rectHalf.Height () / 2;

			dm.Fill4ColorsGradient (rectHalf,
				m_clrPlannerTodayCaption[0], m_clrPlannerTodayCaption[1],
				m_clrPlannerTodayCaption[1], m_clrPlannerTodayCaption[0], FALSE);

			rectHalf.top = rectHalf.bottom;
			rectHalf.bottom = rect.bottom;

			dm.Fill4ColorsGradient (rectHalf,
				m_clrPlannerTodayCaption[2], m_clrPlannerTodayCaption[3],
				m_clrPlannerTodayCaption[3], m_clrPlannerTodayCaption[2], FALSE);
		}
		else
		{
			rectHalf.right = rectHalf.left + rectHalf.Width () / 2;

			dm.Fill4ColorsGradient (rectHalf,
				m_clrPlannerTodayCaption[1], m_clrPlannerTodayCaption[0],
				m_clrPlannerTodayCaption[0], m_clrPlannerTodayCaption[1], TRUE);

			rectHalf.left = rectHalf.right;
			rectHalf.right = rect.right;

			dm.Fill4ColorsGradient (rectHalf,
				m_clrPlannerTodayCaption[3], m_clrPlannerTodayCaption[2],
				m_clrPlannerTodayCaption[2], m_clrPlannerTodayCaption[3], TRUE);
		}

		clrBorder = m_clrPlannerTodayBorder;
	}
	else
	{
		COLORREF clrBack = GetPlannerViewBackgroundColor (pView);

		double H, S, V;
		CBCGPDrawManager::RGBtoHSV (GetPlannerViewBackgroundColor (pView), &H, &S, &V);

		CBCGPDrawManager dm (*pDC);

		if (bHorz)
		{
			dm.Fill4ColorsGradient (rect,
				CBCGPDrawManager::HSVtoRGB (H, S * 0.40, min (V * 1.09, 1.0)),
				CBCGPDrawManager::HSVtoRGB (H, S * 0.20, min (V * 1.12, 1.0)),
				CBCGPDrawManager::HSVtoRGB (H, S * 0.37, min (V * 1.10, 1.0)), 
				CBCGPDrawManager::HSVtoRGB (H, S * 0.48, min (V * 1.08, 1.0)),
				TRUE);
		}
		else
		{
			dm.Fill4ColorsGradient (rect,
				CBCGPDrawManager::HSVtoRGB (H, S * 0.20, min (V * 1.12, 1.0)),
				CBCGPDrawManager::HSVtoRGB (H, S * 0.40, min (V * 1.09, 1.0)),
				CBCGPDrawManager::HSVtoRGB (H, S * 0.48, min (V * 1.08, 1.0)),
				CBCGPDrawManager::HSVtoRGB (H, S * 0.37, min (V * 1.10, 1.0)),
				FALSE);
		}

		if (!bDay)
		{
			clrBorder = clrBack;
		}
	}

	if (clrBorder != CLR_DEFAULT && !bNoBorder)
	{
		if (!bDay)
		{
			rect.InflateRect (1, 0);
		}

		pDC->Draw3dRect (rect, clrBorder, clrBorder);
	}

	return clrText;
}
//*******************************************************************************
void CBCGPVisualManager2007::OnDrawPlannerCaptionText (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect, const CString& strText, 
		COLORREF clrText, int nAlign, BOOL bHighlight)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawPlannerCaptionText (pDC,
			pView, rect, strText, clrText, nAlign, bHighlight);
		return;
	}

	const int nTextMargin = 2;

	rect.DeflateRect (nTextMargin, 0);

	COLORREF clrOld = pDC->SetTextColor (clrText);

	pDC->DrawText (strText, rect, DT_SINGLELINE | DT_VCENTER | nAlign);

	pDC->SetTextColor (clrOld);
}
//*******************************************************************************
COLORREF CBCGPVisualManager2007::GetPlannerAppointmentTimeColor (CBCGPPlannerView* pView,
		BOOL bSelected, BOOL bSimple, DWORD dwDrawFlags)
{
	if (!CanDrawImage () ||
		(bSelected && (dwDrawFlags & BCGP_PLANNER_DRAW_APP_OVERRIDE_SELECTION) == 0))
	{
		return CBCGPVisualManager2003::GetPlannerAppointmentTimeColor (pView,
			bSelected, bSimple, dwDrawFlags);
	}

	double H, S, V;
	CBCGPDrawManager::RGBtoHSV (GetPlannerViewBackgroundColor (pView), &H, &S, &V);

	return CBCGPDrawManager::HSVtoRGB (H, min (S * 1.88, 1.0), V * 0.5);
}
//*******************************************************************************
// in the future versions use base function
COLORREF CBCGPVisualManager2007::GetPlannerViewBackgroundColor (CBCGPPlannerView* pView)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetPlannerViewBackgroundColor (pView);
	}

	ASSERT_VALID (pView);

	CBCGPPlannerManagerCtrl* pCtrl = pView->GetPlanner ();
	ASSERT_VALID (pCtrl);

	return pCtrl->GetBackgroundColor () == CLR_DEFAULT
				? m_clrPlannerWork
				: pCtrl->GetBackgroundColor ();
}
//*******************************************************************************
COLORREF CBCGPVisualManager2007::GetPlannerHourLineColor (CBCGPPlannerView* pView,
		BOOL bWorkingHours, BOOL bHour)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetPlannerHourLineColor (pView, 
			bWorkingHours, bHour);
	}

	ASSERT_VALID (pView);
	return CalculateHourLineColor (GetPlannerViewBackgroundColor (pView), bWorkingHours, bHour);
}
//*******************************************************************************
COLORREF CBCGPVisualManager2007::GetPlannerViewWorkingColor (CBCGPPlannerView* pView)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetPlannerViewWorkingColor (pView);
	}

	ASSERT_VALID (pView);
	return CalculateWorkingColor (GetPlannerViewBackgroundColor (pView));
}
//*******************************************************************************
COLORREF CBCGPVisualManager2007::GetPlannerViewNonWorkingColor (CBCGPPlannerView* pView)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetPlannerViewNonWorkingColor (pView);
	}

	ASSERT_VALID (pView);
	return CalculateNonWorkingColor (GetPlannerViewBackgroundColor (pView),
		pView->IsKindOf (RUNTIME_CLASS (CBCGPPlannerViewDay)));

}
//*******************************************************************************
COLORREF CBCGPVisualManager2007::GetPlannerSelectionColor (CBCGPPlannerView* pView)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetPlannerSelectionColor (pView);
	}

	ASSERT_VALID (pView);
	return CalculateSelectionColor (GetPlannerViewBackgroundColor (pView));
}
//*******************************************************************************
COLORREF CBCGPVisualManager2007::GetPlannerSeparatorColor (CBCGPPlannerView* pView)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetPlannerSeparatorColor (pView);
	}

	ASSERT_VALID (pView);
	return CalculateSeparatorColor (GetPlannerViewBackgroundColor (pView));
}
//*******************************************************************************
COLORREF CBCGPVisualManager2007::OnFillPlannerTimeBar (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect, COLORREF& clrLine)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnFillPlannerTimeBar (pDC, pView, rect, clrLine);
	}

	ASSERT_VALID (pDC);

	CBrush br (m_clrPlannerNcArea);
	pDC->FillRect (rect, &br);
	
	clrLine = m_clrPlannerNcLine;

	return m_clrPlannerNcText;
}
//*******************************************************************************
void CBCGPVisualManager2007::OnFillPlannerWeekBar (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillPlannerWeekBar (pDC, pView, rect);
		return;
	}

	ASSERT_VALID (pDC);

	CBrush br (m_clrPlannerNcArea);
	pDC->FillRect (rect, &br);
}
//*******************************************************************************
void CBCGPVisualManager2007::OnDrawPlannerHeader (CDC* pDC, 
	CBCGPPlannerView* pView, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawPlannerHeader (pDC, pView, rect);
		return;
	}

	ASSERT_VALID (pDC);

	COLORREF clr = GetPlannerViewBackgroundColor (pView);

	if (DYNAMIC_DOWNCAST(CBCGPPlannerViewMonth, pView) != NULL)
	{
		clr = m_clrPlannerNcArea;
	}

	CBrush br (clr);
	pDC->FillRect (rect, &br);
}
//*******************************************************************************
void CBCGPVisualManager2007::OnDrawPlannerHeaderPane (CDC* pDC, 
	CBCGPPlannerView* pView, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawPlannerHeaderPane (pDC, pView, rect);
		return;
	}

	if (DYNAMIC_DOWNCAST(CBCGPPlannerViewMonth, pView) != NULL)
	{
		pDC->Draw3dRect (rect.right - 1, rect.top - 2, 1, rect.Height () + 4, 
			m_clrPlannerNcText, m_clrPlannerNcText);
	}
}
//*******************************************************************************
void CBCGPVisualManager2007::OnFillPlannerHeaderAllDay (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillPlannerHeaderAllDay (pDC, pView, rect);
		return;
	}

	ASSERT_VALID (pDC);
	
	CBrush br (GetPlannerHourLineColor (pView, FALSE, TRUE));
	pDC->FillRect (rect, &br);
}
//*******************************************************************************
void CBCGPVisualManager2007::OnDrawPlannerHeaderAllDayItem (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect, BOOL bIsToday, BOOL bIsSelected)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawPlannerHeaderAllDayItem (pDC, pView, rect, 
			bIsToday, bIsSelected);
		return;
	}

	ASSERT_VALID (pDC);

	rect.left++;
	if (bIsSelected)
	{
		CBrush br (GetPlannerSelectionColor (pView));
		pDC->FillRect (rect, &br);
	}

	if (bIsToday)
	{
		rect.top--;
		rect.right--;
		pDC->Draw3dRect (rect, m_clrPlannerTodayBorder, m_clrPlannerTodayBorder);

		rect.left--;
		rect.right++;
		rect.bottom++;
		pDC->Draw3dRect (rect, m_clrPlannerTodayBorder, m_clrPlannerTodayBorder);
	}
}
//*******************************************************************************
void CBCGPVisualManager2007::PreparePlannerBackItem (BOOL bIsToday, BOOL bIsSelected)
{
	m_bPlannerBackItemToday    = bIsToday;
	m_bPlannerBackItemSelected = bIsSelected;
}
//*******************************************************************************
DWORD CBCGPVisualManager2007::GetPlannerDrawFlags () const
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetPlannerDrawFlags ();
	}

	return BCGP_PLANNER_DRAW_APP_GRADIENT_FILL | 
		   BCGP_PLANNER_DRAW_APP_ROUNDED_CORNERS | 
		   BCGP_PLANNER_DRAW_APP_OVERRIDE_SELECTION |
		   BCGP_PLANNER_DRAW_APP_NO_MULTIDAY_CLOCKS |
		   BCGP_PLANNER_DRAW_APP_DURATION_SHAPE |
		   BCGP_PLANNER_DRAW_VIEW_NO_DURATION |
		   BCGP_PLANNER_DRAW_VIEW_WEEK_BAR |
		   BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_BOLD |
		   BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_COMPACT |
		   BCGP_PLANNER_DRAW_VIEW_DAYS_UPDOWN | 
		   BCGP_PLANNER_DRAW_VIEW_DAYS_UPDOWN_VCENTER;
}

#endif // BCGP_EXCLUDE_PLANNER

BOOL CBCGPVisualManager2007::OnEraseMDIClientArea (CDC* pDC, CRect rectClient)
{
	if (!CanDrawImage () ||
		m_brMainClientArea.GetSafeHandle () == NULL)
	{
		return CBCGPVisualManager2003::OnEraseMDIClientArea (pDC, rectClient);
	}

	pDC->FillRect (rectClient, &m_brMainClientArea);
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPVisualManager2007::GetToolTipParams (CBCGPToolTipParams& params, 
											   UINT /*nType*/ /*= (UINT)(-1)*/)
{
	if (!CanDrawImage () ||
		!m_bToolTipParams)
	{
		return CBCGPVisualManager2003::GetToolTipParams (params);
	}

	params = m_ToolTipParams;
	return TRUE;
}
//*************************************************************************************
void CBCGPVisualManager2007::OnScrollBarDrawThumb (CDC* pDC, CBCGPScrollBar* pScrollBar, CRect rect, 
		BOOL bHorz, BOOL bHighlighted, BOOL bPressed, BOOL bDisabled)
{
	if (!CanDrawImage ())
	{
		return;
	}

	int nScroll = bHorz ? 0 : 1;
	BOOL bIsFrame = pScrollBar->GetVisualStyle () == CBCGPScrollBar::BCGP_SBSTYLE_VISUAL_MANAGER_FRAME;
	int nIndex  = bIsFrame ? 0 : 1;

	m_ctrlScrollBar_Back[nScroll][nIndex].Draw (pDC, rect, 0);

	if (!bDisabled)
	{
		if (!IsBeta ())
		{
			if (bHorz)
			{
				rect.DeflateRect (0, 1);
			}
			else
			{
				rect.DeflateRect (1, 0);
			}
		}

		m_ctrlScrollBar_ThumbBack[nScroll][nIndex].Draw (pDC, rect, bPressed ? 2 : bHighlighted ? 1 : 0);

		if (rect.Width () - 4 > m_ctrlScrollBar_ThumbIcon[nScroll][nIndex].GetParams ().m_rectImage.Width () &&
			rect.Height () - 4 > m_ctrlScrollBar_ThumbIcon[nScroll][nIndex].GetParams ().m_rectImage.Height ())
		{
			rect.OffsetRect (1, 1);
			m_ctrlScrollBar_ThumbIcon[nScroll][nIndex].FillInterior (pDC, rect, 
				CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter,
				bPressed ? 2 : bHighlighted ? 1 : 0);
		}
	}
}
//*************************************************************************************
void CBCGPVisualManager2007::OnScrollBarDrawButton (CDC* pDC, CBCGPScrollBar* pScrollBar, CRect rect, 
		BOOL bHorz, BOOL bHighlighted, BOOL bPressed, BOOL bFirst, BOOL bDisabled)
{
	if (!CanDrawImage ())
	{
		return;
	}

	int nScroll = bHorz ? 0 : 1;
	BOOL bIsFrame = pScrollBar->GetVisualStyle () == CBCGPScrollBar::BCGP_SBSTYLE_VISUAL_MANAGER_FRAME;
	int nIndex  = bIsFrame ? 0 : 1;

	m_ctrlScrollBar_Back[nScroll][nIndex].Draw (pDC, rect, 0);

	if (!bDisabled)
	{
		if (!IsBeta ())
		{
			if (bHorz)
			{
				rect.DeflateRect (0, 1);
			}
			else
			{
				rect.DeflateRect (1, 0);
			}
		}

		m_ctrlScrollBar_Item[nScroll][nIndex].Draw (pDC, rect, bPressed ? 3 : bHighlighted ? 2 : pScrollBar->IsActive () ? 1 : 0);
	}

	CBCGPMenuImages::IMAGES_IDS ids;
	if (bHorz)
	{
		ids = bFirst ? CBCGPMenuImages::IdArowLeftTab3d : CBCGPMenuImages::IdArowRightTab3d;
	}
	else
	{
		ids = bFirst ? CBCGPMenuImages::IdArowUpLarge : CBCGPMenuImages::IdArowDownLarge;
	}

	CBCGPMenuImages::IMAGE_STATE state = bDisabled ? CBCGPMenuImages::ImageGray : CBCGPMenuImages::ImageBlack2;

	if (!pScrollBar->IsActive () && m_Style == VS2007_ObsidianBlack && bIsFrame)
	{
		state = CBCGPMenuImages::ImageLtGray;
	}

	CBCGPMenuImages::Draw (pDC, ids, rect, state);
}
//*************************************************************************************
void CBCGPVisualManager2007::OnScrollBarFillBackground (CDC* pDC, CBCGPScrollBar* pScrollBar, CRect rect, 
		BOOL bHorz, BOOL /*bHighlighted*/, BOOL bPressed, BOOL /*bFirst*/, BOOL /*bDisabled*/)
{
	if (!CanDrawImage ())
	{
		return;
	}

	int nScroll = bHorz ? 0 : 1;
	BOOL bIsFrame = pScrollBar->GetVisualStyle () == CBCGPScrollBar::BCGP_SBSTYLE_VISUAL_MANAGER_FRAME;
	int nIndex  = bIsFrame ? 0 : 1;

	m_ctrlScrollBar_Back[nScroll][nIndex].Draw (pDC, rect, bPressed ? 1 : 0);
}
//*************************************************************************************
void CBCGPVisualManager2007::GetCalendarColors (const CBCGPCalendar* pCalendar,
				   CBCGPCalendarColors& colors)
{
	CBCGPVisualManager2003::GetCalendarColors (pCalendar, colors);

	if (!CanDrawImage ())
	{
		return;
	}

	colors.clrCaption = m_clrCaptionBarGradientDark;
	colors.clrCaptionText = m_clrCaptionBarText;

	colors.clrSelected = m_clrHighlightGradientDark;
	colors.clrSelectedText = m_clrOutlookPageTextHighlighted;

	if (pCalendar != NULL && pCalendar->IsVisualManagerStyle())
	{
		CBrush& brFill = GetDlgBackBrush((CWnd*)pCalendar);
		
		LOGBRUSH lbr;
		brFill.GetLogBrush(&lbr);
		
		colors.clrBackground = lbr.lbColor;
		colors.clrInactiveText = m_clrToolBarBtnTextDisabled;
		colors.clrLine = m_clrToolBarBottomLine;
	}

	if (pCalendar->GetSafeHwnd() != NULL && !pCalendar->IsWindowEnabled())
	{
		colors.clrCaptionText = globalData.clrGrayedText;
		colors.clrSelectedText = globalData.clrGrayedText;
		colors.clrText = pCalendar->IsVisualManagerStyle() ? globalData.clrBarShadow : globalData.clrGrayedText;
	}
}

#ifndef BCGP_EXCLUDE_POPUP_WINDOW

void CBCGPVisualManager2007::OnFillPopupWindowBackground (CDC* pDC, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillPopupWindowBackground (pDC, rect);
		return;
	}

	ASSERT_VALID (pDC);

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, m_clrPopupGradientDark, m_clrPopupGradientLight);
}
//*****************************************************************************
COLORREF CBCGPVisualManager2007::OnDrawPopupWindowCaption (CDC* pDC, CRect rectCaption, CBCGPPopupWindow* pPopupWnd)
{
	ASSERT_VALID(pPopupWnd);

	if (CanDrawImage () && pPopupWnd->HasSmallCaption() && !pPopupWnd->IsSmallCaptionGripper())
	{
		pDC->FillSolidRect(rectCaption, m_clrPopupGradientLight);
		return m_clrOutlookCaptionTextNormal;
	}

	COLORREF clrText = CBCGPVisualManager2003::OnDrawPopupWindowCaption (pDC, rectCaption, pPopupWnd);

	if (CanDrawImage ())
	{
		clrText = m_clrOutlookCaptionTextNormal;
	}

	return clrText;
}
//*****************************************************************************
COLORREF CBCGPVisualManager2007::GetPopupWindowCaptionTextColor(CBCGPPopupWindow* pPopupWnd, BOOL bButton)
{
	COLORREF clrText = CBCGPVisualManager2003::GetPopupWindowCaptionTextColor(pPopupWnd, bButton);

	if (CanDrawImage ())
	{
		clrText = m_clrOutlookCaptionTextNormal;
	}

	return clrText;
}

#endif

COLORREF CBCGPVisualManager2007::OnFillListBoxItem (CDC* pDC, CBCGPListBox* pListBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected)
{
	if (!CanDrawImage () || !m_ctrlRibbonBtn[0].IsValid ())
	{
		return CBCGPVisualManager2003::OnFillListBoxItem (pDC, pListBox, nItem, rect, bIsHighlihted, bIsSelected);
	}

	rect.DeflateRect (0, 1);

	int nIndex = 0;

	if (bIsSelected)
	{
		nIndex = bIsHighlihted ? 1 : 2;
	}

	if (rect.Height() < m_ctrlRibbonBtn[0].GetImages().GetImageSize().cy)
	{
		CBCGPDrawManager dm (*pDC);

		if (bIsSelected)
		{
			dm.DrawRect (rect, m_clrRibbonComboBtnHighlightedStart, m_clrComboBtnBorderPressed);
		}
		else
		{
			dm.DrawRect (rect, m_clrRibbonComboBtnHighlightedFinish, m_clrRibbonComboBtnHighlightedStart);
		}

		return m_clrToolBarBtnTextHighlighted;
	}

	m_ctrlRibbonBtn [0].Draw (pDC, rect, nIndex);
	return m_clrToolBarBtnTextHighlighted;
}
//***************************************************************************************
COLORREF CBCGPVisualManager2007::OnFillComboBoxItem (CDC* pDC, CBCGPComboBox* pComboBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected)
{
	if (!CanDrawImage () || !m_ctrlRibbonBtn[0].IsValid ())
	{
		return CBCGPVisualManager2003::OnFillComboBoxItem (pDC, pComboBox, nItem, rect, bIsHighlihted, bIsSelected);
	}

	if (!bIsHighlihted && !bIsSelected)
	{
		pDC->FillRect(rect, &globalData.brWindow);
		return globalData.clrWindowText;
	}

	rect.DeflateRect (0, 1);

	int nIndex = 0;

	if (bIsSelected)
	{
		nIndex = bIsHighlihted ? 1 : 2;
	}

	if (rect.Height() < m_ctrlRibbonBtn[0].GetImages().GetImageSize().cy)
	{
		CBCGPDrawManager dm (*pDC);

		if (bIsSelected)
		{
			dm.DrawRect (rect, m_clrRibbonComboBtnHighlightedStart, m_clrComboBtnBorderPressed);
		}
		else
		{
			dm.DrawRect (rect, m_clrRibbonComboBtnHighlightedFinish, m_clrRibbonComboBtnHighlightedStart);
		}

		return m_clrToolBarBtnTextHighlighted;
	}

	m_ctrlRibbonBtn [0].Draw (pDC, rect, nIndex);
	return m_clrToolBarBtnTextHighlighted;
}
//***************************************************************************************
COLORREF CBCGPVisualManager2007::OnDrawMenuLabel (CDC* pDC, CRect rect)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnDrawMenuLabel (pDC, rect);
	}

	ASSERT_VALID (pDC);

	pDC->FillRect (rect, 
		m_brGroupBackground.GetSafeHandle () != NULL ? &m_brGroupBackground : &m_brBarBkgnd);

	CRect rectSeparator = rect;
	rectSeparator.top = rectSeparator.bottom - 2;

	DrawSeparator (pDC, rectSeparator, TRUE);

	return m_clrGroupText != (COLORREF)-1 ? m_clrGroupText : m_clrMenuText;
}
//**************************************************************************************
COLORREF CBCGPVisualManager2007::OnFillCaptionBarButton (CDC* pDC, CBCGPCaptionBar* pBar,
											CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
											BOOL bIsDisabled, BOOL bHasDropDownArrow,
											BOOL bIsSysButton)
{
	COLORREF clrText = CBCGPVisualManager2003::OnFillCaptionBarButton (pDC, pBar,
											rect, bIsPressed, bIsHighlighted, 
											bIsDisabled, bHasDropDownArrow, bIsSysButton);

	ASSERT_VALID (pBar);

	if (CanDrawImage () &&
		pBar->IsMessageBarMode () && !bIsHighlighted)
	{
		clrText = bIsSysButton ? m_clrMenuBarBtnText : m_clrMenuText;
	}

	return clrText;
}
//**************************************************************************************
BOOL CBCGPVisualManager2007::OnDrawPushButton (CDC* pDC, CRect rect, CBCGPButton* pButton, COLORREF& clrText)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::OnDrawPushButton (pDC, rect, pButton, clrText);
	}

	ASSERT_VALID (pDC);

	int index = 0;

	BOOL bDisabled    = pButton->GetSafeHwnd() != NULL && !pButton->IsWindowEnabled ();
	BOOL bFocused     = pButton->GetSafeHwnd() != NULL && pButton->GetSafeHwnd () == ::GetFocus ();
	BOOL bDefault     = pButton->GetSafeHwnd() != NULL && pButton->IsDefaultButton ();
	BOOL bPressed     = pButton->GetSafeHwnd() != NULL && pButton->IsPressed ();
	BOOL bChecked     = pButton->GetSafeHwnd() != NULL && pButton->IsChecked ();
	BOOL bHighlighted = pButton->GetSafeHwnd() != NULL && pButton->IsHighlighted ();

	bHighlighted |= bFocused;

	if (bDisabled)
	{
		index = 6;
	}
	else
	{
		if (bChecked)
		{
			index = 3;
		}
		else
		{
			if (bDefault && !bHighlighted)
			{
				index = 5;
			}
		}

		if (bPressed)
		{
			if (bHighlighted)
			{
				index = 2;
			}
		}
		else if (bHighlighted)
		{
			index++;
		}
	}

	globalData.DrawParentBackground (pButton, pDC);
	m_ctrlRibbonBtnPush.Draw (pDC, rect, index);

	clrText = globalData.clrBarText;

	if (bDisabled)
	{
		clrText = m_clrRibbonBarBtnTextDisabled;
	}
	else if (bHighlighted)
	{
		clrText = m_clrRibbonBarBtnTextHighlighted;
	}

	return TRUE;
}
//**************************************************************************************
void CBCGPVisualManager2007::OnDrawGroup (CDC* pDC, CBCGPGroup* pGroup, CRect rect, const CString& strName)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawGroup (pDC, pGroup, rect, strName);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pGroup);

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

	m_ctrlRibbonBtnGroup.DrawFrame (pDC, rectFrame);

	pDC->SelectClipRgn (NULL);

	if (strName.IsEmpty ())
	{
		return;
	}

	DWORD dwTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOCLIP;

	if (pGroup->m_bOnGlass)
	{
		DrawTextOnGlass (pDC, strName, rectText, dwTextStyle, 10, globalData.clrBarText);
	}
	else
	{
		CString strCaption = strName;
		pDC->DrawText (strCaption, rectText, dwTextStyle);
	}
}
//*******************************************************************************
void CBCGPVisualManager2007::OnDrawDlgSeparator(CDC* pDC, CBCGPStatic* pCtrl, CRect rect, BOOL bIsHorz)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawDlgSeparator(pDC, pCtrl, rect, bIsHorz);
		return;
	}

	CPen pen1(PS_SOLID, 0, globalData.clrBarShadow);
	CPen pen2(PS_SOLID, 0, globalData.clrBarLight);

	DrawSeparator (pDC, rect, pen1, pen2, bIsHorz);
}
//*******************************************************************************
BOOL CBCGPVisualManager2007::OnFillDialog (CDC* pDC, CWnd* pDlg, CRect rect)
{
	if (!CanDrawImage () || m_brDlgBackground.GetSafeHandle () == NULL)
	{
		return CBCGPVisualManager2003::OnFillDialog (pDC, pDlg, rect);
	}

	ASSERT_VALID (pDC);

	pDC->FillRect (rect, &GetDlgBackBrush (pDlg));

	return TRUE;
}
//*******************************************************************************
void CBCGPVisualManager2007::OnDrawDlgSizeBox (CDC* pDC, CWnd* pDlg, CRect rectSizeBox)
{
	CBCGPDialog* pDialog = DYNAMIC_DOWNCAST(CBCGPDialog, pDlg);
	if (!CanDrawImage () || pDialog == NULL || !pDialog->IsVisualManagerStyle ())
	{
		CBCGPVisualManager2003::OnDrawStatusBarSizeBox (pDC, NULL, rectSizeBox);
		return;
	}

	OnDrawStatusBarSizeBox (pDC, NULL, rectSizeBox);
}
//*******************************************************************************
void CBCGPVisualManager2007::OnDrawSliderChannel (CDC* pDC, CBCGPSliderCtrl* pSlider, BOOL bVert, CRect rect, BOOL bDrawOnGlass)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawSliderChannel (pDC, pSlider, bVert, rect, bDrawOnGlass);
		return;
	}

	ASSERT_VALID (pDC);
	DrawSeparator (pDC, rect, m_penSeparatorDark, m_penSeparator2, !bVert);
}
//********************************************************************************
void CBCGPVisualManager2007::OnDrawSliderThumb (CDC* pDC, CBCGPSliderCtrl* pSlider, 
			CRect rect, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled,
			BOOL bVert, BOOL bLeftTop, BOOL bRightBottom,
			BOOL bDrawOnGlass)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawSliderThumb (
			pDC, pSlider, rect, bIsHighlighted, bIsPressed, bIsDisabled,
			bVert, bLeftTop, bRightBottom, bDrawOnGlass);
		return;
	}

	ASSERT_VALID (pDC);

	int indexRen = 0;

	if (bLeftTop && bRightBottom)
	{
		indexRen = 1;
	}
	else if (bLeftTop)
	{
		indexRen = 2;
	}

	if (bVert)
	{
		indexRen += 3;
	}

	CBCGPControlRenderer* pRenderer = &m_ctrlSliderThumb[indexRen];

	if (pRenderer != NULL)
	{
		int index = 0;
		if (bIsDisabled)
		{
			index = 3;
		}
		else	
		{
			if (bIsPressed)
			{
				index = 2;
			}
			else if (bIsHighlighted)
			{
				index = 1;
			}
		}

		CRect rectSides = pRenderer->GetParams ().m_rectSides;

		if ((rectSides.left + rectSides.right) >= rect.Width () || rectSides.top + rectSides.bottom >= rect.Height ())
		{
			CBCGPVisualManager2003::OnDrawSliderThumb (
				pDC, pSlider, rect, bIsHighlighted, bIsPressed, bIsDisabled,
				bVert, bLeftTop, bRightBottom, bDrawOnGlass);
		}
		else
		{
			pRenderer->Draw (pDC, rect, index);
		}
	}
}
//********************************************************************************
BCGP_SMARTDOCK_THEME CBCGPVisualManager2007::GetSmartDockingTheme ()
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode () || !globalData.IsWindowsLayerSupportAvailable () ||
		!globalData.bIsWindowsVista)
	{
		return CBCGPVisualManager2003::GetSmartDockingTheme ();
	}

	return BCGP_SDT_VS2008;
}
//********************************************************************************
void CBCGPVisualManager2007::OnDrawEditClearButton(CDC* pDC, CBCGPButton* pButton, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawEditClearButton(pDC, pButton, rect);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);

	CBCGPDrawManager dm (*pDC);
	dm.DrawRect (rect, m_clrRibbonEdit, (COLORREF)-1);

	if (pButton->IsPressed())
	{
		if (m_ctrlRibbonComboBoxBtn.IsValid())
		{
			rect.InflateRect(0, 1);
			m_ctrlRibbonComboBoxBtn.Draw (pDC, rect, 2);
			return;
		}
	}
	else if (pButton->IsHighlighted())
	{
		if (m_ctrlRibbonComboBoxBtn.IsValid())
		{
			rect.InflateRect(0, 1);
			m_ctrlRibbonComboBoxBtn.Draw (pDC, rect, 1);
			return;
		}
	}
}
//*************************************************************************************
void CBCGPVisualManager2007::OnDrawControlBorder (CDC* pDC, CRect rect, CWnd* pWndCtrl, BOOL bDrawOnGlass)
{
	if (!CanDrawImage () || DYNAMIC_DOWNCAST(CBCGPPopupMenuBar, pWndCtrl) == NULL)
	{
		CBCGPVisualManager2003::OnDrawControlBorder (pDC, rect, pWndCtrl, bDrawOnGlass);
		return;
	}

	m_ctrlRibbonBtnGroup.DrawFrame (pDC, rect);
}
//********************************************************************************
BOOL CBCGPVisualManager2007::OnDrawBrowseButton (CDC* pDC, CRect rect, CBCGPEdit* pEdit, CBCGPVisualManager::BCGBUTTON_STATE state, COLORREF& clrText)
{
	if (!CanDrawImage () || (pEdit != NULL && !pEdit->m_bVisualManagerStyle))
	{
		return CBCGPVisualManager2003::OnDrawBrowseButton(pDC, rect, pEdit, state, clrText);
	}

	int index = 0;

	BOOL bDisabled = !pEdit->IsWindowEnabled ();

	if (bDisabled)
	{
		index = 6;
	}
	else if (state == ButtonsIsPressed)
	{
		index = 2;
	}
	else if (state == ButtonsIsHighlighted)
	{
		index++;
	}

	pDC->FillRect(rect, bDisabled ? &globalData.brBtnFace : &globalData.brWindow);
	m_ctrlRibbonBtnPush.Draw (pDC, rect, index);

	clrText = globalData.clrBarText;

	if (bDisabled)
	{
		clrText = m_clrRibbonBarBtnTextDisabled;
	}
	else if (state == ButtonsIsHighlighted)
	{
		clrText = m_clrRibbonBarBtnTextHighlighted;
	}

	return TRUE;
}
//********************************************************************************
BOOL CBCGPVisualManager2007::OnDrawCalculatorButton (CDC* pDC, 
	CRect rect, CBCGPToolbarButton* pButton, CBCGPVisualManager::BCGBUTTON_STATE state, int cmd, CBCGPCalculator* pCalculator)
{
	if (!CanDrawImage () || pCalculator->IsDialogControl())
	{
		return CBCGPVisualManager2003::OnDrawCalculatorButton (pDC, rect, pButton, state, cmd, pCalculator);
	}

	ASSERT_VALID (pButton);
	ASSERT_VALID (pDC);
	ASSERT_VALID (pCalculator);

	int nIndex = 0;

	switch (state)
	{
	case ButtonsIsPressed:
		nIndex = 2;
		break;

	case ButtonsIsHighlighted:
		nIndex = 1;
		break;
	}

	CBCGPControlRenderer* pRenderer = &m_ctrlRibbonBtnPush;

	if (m_ctrlCalculatorBtn.IsValid ())
	{
		pRenderer = &m_ctrlCalculatorBtn;

		if (nIndex == 0)
		{
			if (cmd >= CBCGPCalculator::idCommandUser)
			{
				nIndex = 2;
			}
			else if ((CBCGPCalculator::idCommandClear <= cmd && cmd <= CBCGPCalculator::idCommandBackspace) ||
				(CBCGPCalculator::idCommandAdvSin <= cmd && cmd <= CBCGPCalculator::idCommandAdvPi))
			{
				nIndex = 1;
			}
		}
		else
		{
			nIndex += 2;
		}
	}

	pRenderer->Draw (pDC, rect, nIndex);
	
	return TRUE;
}
//********************************************************************************
BOOL CBCGPVisualManager2007::OnDrawCalculatorDisplay (CDC* pDC, CRect rect, 
												  const CString& strText, BOOL bMem,
												  CBCGPCalculator* pCalculator)
{
	if (!CanDrawImage () || pCalculator->IsDialogControl())
	{
		return CBCGPVisualManager2003::OnDrawCalculatorDisplay(pDC, rect, strText, bMem, pCalculator);
	}

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, m_clrRibbonEditHighlighted, m_clrRibbonEdit, TRUE);

	pDC->Draw3dRect (&rect, m_clrRibbonEditBorder, m_clrRibbonEditBorder);

	return TRUE;
}
//***********************************************************************************
COLORREF CBCGPVisualManager2007::GetCalculatorButtonTextColor (BOOL bIsUserCommand, int nCmd /* CBCGPCalculator::CalculatorCommands */)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetCalculatorButtonTextColor(bIsUserCommand, nCmd);
	}

	if (bIsUserCommand)
	{
		return m_clrCalculatorBtnText[2];
	}

	return ((CBCGPCalculator::idCommandClear <= nCmd && nCmd <= CBCGPCalculator::idCommandBackspace) ||
			(CBCGPCalculator::idCommandAdvSin <= nCmd && nCmd <= CBCGPCalculator::idCommandAdvPi))
			? m_clrCalculatorBtnText[1] : m_clrCalculatorBtnText[0];
}
//***********************************************************************************
void CBCGPVisualManager2007::OnFillPropSheetHeaderArea(CDC* pDC, CBCGPPropertySheet* pPropSheet, CRect rect, BOOL& bDrawBottomLine)
{
	ASSERT_VALID (pDC);

	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnFillPropSheetHeaderArea(pDC, pPropSheet, rect, bDrawBottomLine);
		return;
	}

	COLORREF clr1 = (m_Style != VS2007_LunaBlue) ? globalData.clrBarHilite : m_clrAppCaptionActiveStart;
	COLORREF clr2 = (m_Style != VS2007_LunaBlue) ? globalData.clrBarLight : m_clrAppCaptionActiveFinish;

	CBCGPDrawManager dm(*pDC);
	dm.Fill4ColorsGradient(rect, clr1, clr2, clr2, clr1, TRUE);
}
//***********************************************************************************
void CBCGPVisualManager2007::OnDrawSpinButtons (CDC* pDC, CRect rectSpin, 
	int nState, BOOL bOrientation, CBCGPSpinButtonCtrl* pSpinCtrl)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawSpinButtons(pDC, rectSpin, nState, bOrientation, pSpinCtrl);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID (this);

	CRect rect [2];
	rect[0] = rect[1] = rectSpin;

	if (!bOrientation) 
	{
		rect[0].DeflateRect(0, 0, 0, rect[0].Height() / 2);
		rect[1].top = rect[0].bottom ;
	}
	else
	{
		rect[1].DeflateRect(0, 0, rect[0].Width() / 2, 0);
		rect[0].left = rect[1].right;
	}

	CBCGPMenuImages::IMAGES_IDS id[2][2] = {{CBCGPMenuImages::IdArowUp, CBCGPMenuImages::IdArowDown}, {CBCGPMenuImages::IdArowRight, CBCGPMenuImages::IdArowLeft}};

	int idxPressed = (nState & (SPIN_PRESSEDUP | SPIN_PRESSEDDOWN)) - 1;
	
	int idxHighlighted = -1;
	if (nState & SPIN_HIGHLIGHTEDUP)
	{
		idxHighlighted = 0;
	}
	else if (nState & SPIN_HIGHLIGHTEDDOWN)
	{
		idxHighlighted = 1;
	}

	BOOL bDisabled = nState & SPIN_DISABLED;

	COLORREF color1 = m_clrComboBtnDisabledStart;
	COLORREF color2 = m_clrComboBtnDisabledFinish;
	COLORREF colorBorder = m_clrComboBtnBorderDisabled;

	for (int i = 0; i < 2; i ++)
	{
		CRect rectBtn(rect[i]);

		if (bDisabled || (idxPressed != i && idxHighlighted != i))
		{
			OnFillSpinButton (pDC, pSpinCtrl, rectBtn, bDisabled);
		}
		else
		{
			if (idxPressed == i)
			{
				color1 = m_clrComboBtnPressedStart;
				color2 = m_clrComboBtnPressedFinish;
				colorBorder = m_clrComboBtnBorderPressed;
			}
			else if (idxHighlighted == i)
			{
				color1 = m_clrComboBtnHighlightedStart;
				color2 = m_clrComboBtnHighlightedFinish;
				colorBorder = m_clrComboBtnBorderHighlighted;
			}

			if (colorBorder != (COLORREF)(-1))
			{
				if (CBCGPToolBarImages::m_bIsDrawOnGlass)
				{
					CBCGPDrawManager dm (*pDC);
					dm.DrawRect (rectBtn, (COLORREF)-1, colorBorder);
				}
				else
				{
					pDC->Draw3dRect (rectBtn, colorBorder, colorBorder);
				}

				rectBtn.DeflateRect (1, 1, 1, 1);
			}

			CBCGPDrawManager dm (*pDC);
			dm.FillGradient (rectBtn, color1, color2, TRUE);
		}

		CBCGPMenuImages::Draw (pDC, id[bOrientation ? 1 : 0][i], rectBtn,
			bDisabled ? CBCGPMenuImages::ImageGray : CBCGPMenuImages::ImageBlack);
	}
}
//***********************************************************************************
void CBCGPVisualManager2007::OnDrawDateTimeDropButton (CDC* pDC, CRect rect, 
	BOOL bDisabled, BOOL bPressed, BOOL bHighlighted, CBCGPDateTimeCtrl* pCtrl)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2003::OnDrawDateTimeDropButton(pDC, rect, bDisabled, bPressed, bHighlighted, pCtrl);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID (this);

	COLORREF color1 = m_clrComboBtnStart;
	COLORREF color2 = m_clrComboBtnFinish;
	COLORREF colorBorder = m_clrComboBtnBorder;

	if (bDisabled)
	{
		color1 = m_clrComboBtnDisabledStart;
		color2 = m_clrComboBtnDisabledFinish;
		colorBorder = m_clrComboBtnBorderDisabled;
	}
	else if (bPressed)
	{
		color1 = m_clrComboBtnPressedStart;
		color2 = m_clrComboBtnPressedFinish;
		colorBorder = m_clrComboBtnBorderPressed;
	}
	else if (bHighlighted)
	{
		color1 = m_clrComboBtnHighlightedStart;
		color2 = m_clrComboBtnHighlightedFinish;
		colorBorder = m_clrComboBtnBorderHighlighted;
	}

	if (colorBorder != (COLORREF)(-1))
	{
		if (CBCGPToolBarImages::m_bIsDrawOnGlass)
		{
			CBCGPDrawManager dm (*pDC);
			dm.DrawRect (rect, (COLORREF)-1, colorBorder);
		}
		else
		{
			pDC->Draw3dRect (rect, colorBorder, colorBorder);
		}

		rect.DeflateRect (1, 1, 1, 1);
	}

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, color1, color2, TRUE);

	CBCGPMenuImages::Draw (pDC, CBCGPMenuImages::IdArowDown, rect,
		bDisabled ? CBCGPMenuImages::ImageGray : CBCGPMenuImages::ImageBlack);
}
//*******************************************************************************
COLORREF CBCGPVisualManager2007::GetTreeControlFillColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected, BOOL bIsFocused, BOOL bIsDisabled)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetTreeControlFillColor(pTreeCtrl, bIsSelected, bIsFocused, bIsDisabled);
	}

	if (bIsSelected)
	{
		return bIsFocused ? m_clrRibbonComboBtnHighlightedStart : globalData.clrBarShadow;
	}

	return globalData.clrBarLight;
}
//*******************************************************************************
COLORREF CBCGPVisualManager2007::GetTreeControlTextColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected, BOOL bIsFocused, BOOL bIsDisabled)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetTreeControlTextColor(pTreeCtrl, bIsSelected, bIsFocused, bIsDisabled);
	}

	if (bIsSelected)
	{
		return bIsFocused ? m_clrToolBarBtnTextHighlighted : globalData.clrBarText;
	}

	return globalData.clrBarText;
}
//*******************************************************************************
HBRUSH CBCGPVisualManager2007::GetListControlFillBrush(CBCGPListCtrl* pListCtrl)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetListControlFillBrush(pListCtrl);
	}

	return m_brCtrlBackground;
}
//*******************************************************************************
COLORREF CBCGPVisualManager2007::GetListControlTextColor(CBCGPListCtrl* pListCtrl)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2003::GetListControlTextColor(pListCtrl);
	}

	return globalData.clrBarText;
}
