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
// BCGPVisualManagerScenic.cpp: implementation of the CBCGPVisualManagerScenic class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "bcgglobals.h"
#include "BCGCBProVer.h"
#include "BCGPVisualManagerScenic.h"
#include "BCGPDrawManager.h"
#include "BCGPTagManager.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPGlobalUtils.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPFrameWnd.h"
#include "CustomizeButton.h"
#include "BCGPToolbarComboBoxButton.h"
#include "BCGPCaptionBar.h"
#include "BCGPCalculator.h"
#include "BCGPCalendarBar.h"
#include "BCGPColorBar.h"
#include "bcgpstyle.h"
#include "BCGPPropList.h"

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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
	extern BOOL g_bToolBarImagesTrace;
#endif

IMPLEMENT_DYNCREATE (CBCGPVisualManagerScenic, CBCGPWinXPVisualManager)

CBCGPVisualManagerScenic::Style	CBCGPVisualManagerScenic::m_Style = CBCGPVisualManagerScenic::VSScenic_Default;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPVisualManagerScenic::CBCGPVisualManagerScenic()
	: m_bLoaded       (FALSE)
	, m_hinstRes      (NULL)
	, m_bAutoFreeRes  (FALSE)
	, m_bNcTextCenter (FALSE)
	, m_bToolTipParams(FALSE)
{
	m_bIsRectangulareRibbonTab = TRUE;
}
//*****************************************************************************
CBCGPVisualManagerScenic::~CBCGPVisualManagerScenic()
{
	CBCGPMenuImages::SetColor (CBCGPMenuImages::ImageBlack, (COLORREF)-1);
}
//*****************************************************************************
CString CBCGPVisualManagerScenic::MakeResourceID(LPCTSTR lpszID) const
{
	CString strResID(lpszID);
	ASSERT(!strResID.IsEmpty());

	if (!m_strStylePrefix.IsEmpty())
	{
		strResID = m_strStylePrefix + strResID;
	}

	return strResID;
}
//*****************************************************************************
CString CBCGPVisualManagerScenic::GetStyleDllName() const
{
	CString strVer;
	strVer.Format (_T("%d%d"), _BCGCBPRO_VERSION_MAJOR, _BCGCBPRO_VERSION_MINOR);

	return _T("BCGPStyleScenic") + strVer + _T(".dll");
}
//*****************************************************************************
CString CBCGPVisualManagerScenic::GetStyleResourceID() const
{
	CString strResID (_T("IDX_BCG_STYLE"));

#if !defined _AFXDLL || defined _BCGCBPRO_STATIC_

	strResID = _T("SCENIC_") + strResID;

#endif

	return strResID;
}
//*****************************************************************************
void CBCGPVisualManagerScenic::SetResourceHandle(HINSTANCE hinstRes)
{
	m_bAutoFreeRes = FALSE;

	if (m_hinstRes != hinstRes)
	{
		m_hinstRes = hinstRes;

		if (CBCGPVisualManager::GetInstance()->IsKindOf(RUNTIME_CLASS(CBCGPVisualManagerScenic)))
		{
			CBCGPVisualManager::GetInstance()->OnUpdateSystemColors();
		}
	}
}
//*****************************************************************************
BOOL CBCGPVisualManagerScenic::SetStyle (Style style, LPCTSTR lpszPath)
{
	m_Style = style;

	if (GetCurrStyle() == VSScenic_Win7)
	{
		return SetStyle(lpszPath);
	}

	CleanStyle ();
	CleanUp();

	CBCGPWinXPVisualManager::OnUpdateSystemColors ();
	return TRUE;
}
//*****************************************************************************
CBCGPVisualManagerScenic::Style CBCGPVisualManagerScenic::GetStyle ()
{
	return m_Style;
}
//*****************************************************************************
CBCGPVisualManagerScenic::Style CBCGPVisualManagerScenic::GetCurrStyle ()
{
	if (m_Style != VSScenic_Default)
	{
		return m_Style;
	}

	return globalData.bIsWindows8 ? VSScenic_Win8 : VSScenic_Win7;
}
//*****************************************************************************
BOOL CBCGPVisualManagerScenic::SetStyle(LPCTSTR lpszPath)
{
	if (m_hinstRes > (HINSTANCE) 32)
	{
		return TRUE;
	}

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_

	CString strStyleDLLName (GetStyleDllName());

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

	CleanStyle ();

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

	CleanStyle ();

	SetResourceHandle (hinstRes);

#endif

	return TRUE;
}
//*****************************************************************************
void CBCGPVisualManagerScenic::CleanStyle()
{
	if (m_bAutoFreeRes && m_hinstRes >(HINSTANCE) 32)
	{
		::FreeLibrary(m_hinstRes);
	}

	m_hinstRes = NULL;
	m_strStylePrefix.Empty();
}
//*****************************************************************************
void CBCGPVisualManagerScenic::DrawSeparator (CDC* pDC, const CRect& rect, BOOL bHorz)
{
    DrawSeparator (pDC, rect, m_penSeparator, m_penSeparatorLight, bHorz);
}
//*****************************************************************************
void CBCGPVisualManagerScenic::DrawSeparator (CDC* pDC, const CRect& rect, CPen& pen1, CPen& pen2, BOOL bHorz)
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
//*****************************************************************************
CFont& CBCGPVisualManagerScenic::GetNcCaptionTextFont()
{
	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::GetNcCaptionTextFont();
	}

	return m_AppCaptionFont;
}
//*****************************************************************************
COLORREF CBCGPVisualManagerScenic::GetNcCaptionTextColor(BOOL bActive, BOOL /*bTitle*/) const
{
	return bActive ? globalData.clrCaptionText : globalData.clrInactiveCaptionText;
}
//*****************************************************************************
void CBCGPVisualManagerScenic::DrawNcBtn(CDC* pDC, const CRect& rect, UINT nButton, 
										BCGBUTTON_STATE state, BOOL bSmall, 
										BOOL bActive, BOOL bMDI/* = FALSE*/, BOOL bEnabled)
{
	ASSERT_VALID(pDC);

	if (m_hThemeWindow == NULL)
	{
		UINT nState = 0;

		switch (nButton)
		{
		case SC_CLOSE:
			nState = DFCS_CAPTIONCLOSE;
			break;

		case SC_MINIMIZE:
			nState = DFCS_CAPTIONMIN;
			break;

		case SC_MAXIMIZE:
			nState = DFCS_CAPTIONMAX;
			break;

		case SC_RESTORE:
			nState = DFCS_CAPTIONRESTORE;
			break;

		case SC_CONTEXTHELP:
			nState = DFCS_CAPTIONHELP;
			break;

		default:
			return;
		}

		if (!bActive || !bEnabled)
		{
			nState |= DFCS_INACTIVE;
		}

		if (state != ButtonsIsRegular)
		{
			nState |= state == ButtonsIsHighlighted ? 0x1000/*DFCS_HOT*/ : DFCS_PUSHED;
		}

		CRect rt (rect);
		pDC->DrawFrameControl (rt, DFC_CAPTION, nState);

		return;
	}

    int nPart = 0;
    int nState = 0;
	if (nButton == SC_CLOSE)
	{
		if (bMDI)
		{
            nPart = WP_MDICLOSEBUTTON;
		}
		else
		{
			nPart = bSmall ? WP_SMALLCLOSEBUTTON : WP_CLOSEBUTTON;
		}

		nState = !bEnabled ? CBS_DISABLED : bActive ? CBS_NORMAL : 5;
		if (state != ButtonsIsRegular)
		{
			nState = state == ButtonsIsHighlighted ? CBS_HOT : CBS_PUSHED;
		}
	}
	else if (nButton == SC_MINIMIZE)
	{
		if (bMDI)
		{
            nPart = WP_MDIMINBUTTON;
		}
		else if (!bSmall)
		{
			nPart = WP_MINBUTTON;
		}

        nState = !bEnabled ? MINBS_DISABLED : bActive ? MINBS_NORMAL : 5;
		if (state != ButtonsIsRegular)
		{
			nState = state == ButtonsIsHighlighted ? MINBS_HOT : MINBS_PUSHED;
		}
	}
	else if (nButton == SC_MAXIMIZE)
	{
		if (!bMDI && !bSmall)
		{
            nPart = WP_MAXBUTTON;
		}

        nState = !bEnabled ? MAXBS_DISABLED : bActive ? MAXBS_NORMAL : 5;
		if (state != ButtonsIsRegular)
		{
			nState = state == ButtonsIsHighlighted ? MAXBS_HOT : MAXBS_PUSHED;
		}
	}
	else if (nButton == SC_RESTORE)
	{
		if (bMDI)
		{
            nPart = WP_MDIRESTOREBUTTON;
		}
		else
		{
			nPart = WP_RESTOREBUTTON;
		}

        nState = !bEnabled ? RBS_DISABLED : bActive ? RBS_NORMAL : 5;
		if (state != ButtonsIsRegular)
		{
			nState = state == ButtonsIsHighlighted ? RBS_HOT : RBS_PUSHED;
		}
	}
	else if (nButton == SC_CONTEXTHELP)
	{
		if (bMDI)
		{
            nPart = WP_MDIHELPBUTTON;
		}
		else if (!bSmall)
		{
			nPart = WP_HELPBUTTON;
		}

        nState = bActive ? HBS_NORMAL : HBS_DISABLED;
		if (state != ButtonsIsRegular)
		{
			nState = state == ButtonsIsHighlighted ? HBS_HOT: HBS_PUSHED;
		}
	}

    if (nPart == 0)
	{
		return;
	}

	(*m_pfDrawThemeBackground)(m_hThemeWindow, pDC->GetSafeHdc(), nPart, nState, &rect, 0);
}
//*****************************************************************************
void CBCGPVisualManagerScenic::DrawNcText (CDC* pDC, CRect& rect, 
										 const CString& strTitle, 
										 BOOL bActive, BOOL bIsRTL, 
										 BOOL bTextCenter,
										 BOOL bGlass/* = FALSE*/, int nGlassGlowSize/* = 0*/, 
										 COLORREF clrGlassText/* = (COLORREF)-1*/)
{
	if (strTitle.IsEmpty () || rect.right <= rect.left)
	{
		return;
	}

	ASSERT_VALID (pDC);

	int nOldMode = pDC->SetBkMode (TRANSPARENT);
	COLORREF clrOldText = pDC->GetTextColor ();

	DWORD dwTextStyle = DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX |
		(bIsRTL ? DT_RTLREADING : 0);

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

	pDC->SetBkMode    (nOldMode);
	pDC->SetTextColor (clrOldText);
}
//*****************************************************************************
BOOL CBCGPVisualManagerScenic::IsOwnerDrawMenuCheck ()
{
	return CanDrawImage () ? FALSE : CBCGPWinXPVisualManager::IsOwnerDrawMenuCheck ();
}
//*****************************************************************************
BOOL CBCGPVisualManagerScenic::IsHighlightWholeMenuItem ()
{
	return CanDrawImage () ? TRUE : CBCGPWinXPVisualManager::IsHighlightWholeMenuItem ();
}
//*****************************************************************************
void CBCGPVisualManagerScenic::CleanUp ()
{
	m_AppCaptionFont.DeleteObject ();
	m_penSeparatorDark.DeleteObject ();
	m_penSeparatorLight.DeleteObject ();

	m_ctrlPopupBorder.CleanUp ();
	m_ctrlPopupResizeBar.CleanUp ();
	m_PopupResizeBar_HV.Clear ();
	m_PopupResizeBar_HVT.Clear ();
	m_PopupResizeBar_V.Clear ();

	m_ctrlMenuItemBack.CleanUp();
    m_MenuItemMarkerC.Clear ();
    m_MenuItemMarkerR.Clear ();
	m_ctrlMenuHighlighted[0].CleanUp();
	m_ctrlMenuHighlighted[1].CleanUp();

	m_ctrlRibbonComboBoxBtn.CleanUp ();

	m_ctrlRibbonCaptionQA.CleanUp ();
	m_ctrlRibbonCategoryBack.CleanUp ();
	m_ctrlRibbonCategoryTab.CleanUp ();
	m_ctrlRibbonCategoryTabSep.CleanUp ();
	m_ctrlRibbonCategoryTabFrame.CleanUp ();
	m_ctrlRibbonCategoryBtnPage[0].CleanUp ();
	m_ctrlRibbonCategoryBtnPage[1].CleanUp ();
	m_ctrlRibbonPanelBack.CleanUp ();
	m_ctrlRibbonPanelBackSep.CleanUp ();
	m_RibbonPanelSeparator.Clear ();
	m_ctrlRibbonPanelQAT.CleanUp ();
	m_ctrlRibbonMainPanel.CleanUp ();
	m_ctrlRibbonBtnMainPanel.CleanUp();
	m_ctrlRibbonBtnGroup_S.CleanUp();
	m_ctrlRibbonBtnGroup_F.CleanUp();
	m_ctrlRibbonBtnGroup_M.CleanUp();
	m_ctrlRibbonBtnGroup_L.CleanUp();
	m_ctrlRibbonBtnGroupMenu_F[0].CleanUp();
	m_ctrlRibbonBtnGroupMenu_F[1].CleanUp();
	m_ctrlRibbonBtnGroupMenu_M[0].CleanUp();
	m_ctrlRibbonBtnGroupMenu_M[1].CleanUp();
	m_ctrlRibbonBtnGroupMenu_L[0].CleanUp();
	m_ctrlRibbonBtnGroupMenu_L[1].CleanUp();
	m_ctrlRibbonBtn[0].CleanUp();
	m_ctrlRibbonBtn[1].CleanUp();
	m_ctrlRibbonBtnMenuH[0].CleanUp();
	m_ctrlRibbonBtnMenuH[1].CleanUp();
	m_ctrlRibbonBtnMenuV[0].CleanUp();
	m_ctrlRibbonBtnMenuV[1].CleanUp();
	m_ctrlRibbonBtnLaunch.CleanUp();
	m_RibbonBtnLaunchIcon.Clear();
	m_ctrlRibbonBtnMain.CleanUp();
	m_ctrlRibbonBtnMainColorized.CleanUp();
	m_ctrlRibbonSliderBtnPlus.CleanUp();
	m_ctrlRibbonSliderBtnMinus.CleanUp();
	m_RibbonBtnDefaultImage.Clear ();
	m_ctrlRibbonBtnDefault.CleanUp();
	m_ctrlRibbonBtnDefaultIcon.CleanUp();
	m_ctrlRibbonBtnDefaultQAT.CleanUp();
	m_ctrlRibbonBtnCheck.CleanUp ();
	m_ctrlRibbonBtnRadio.CleanUp ();
	m_ctrlRibbonBtnStatusPane.CleanUp();
	m_ctrlRibbonBtnPalette[0].CleanUp();
	m_ctrlRibbonBtnPalette[1].CleanUp();
	m_ctrlRibbonBtnPalette[2].CleanUp();

	m_ctrlRibbonBorder_QAT.CleanUp ();
	m_ctrlRibbonBorder_Floaty.CleanUp ();
	m_ctrlRibbonBorder_Panel.CleanUp ();

	m_ctrlRibbonKeyTip.CleanUp ();
	m_clrRibbonKeyTipTextNormal   = (COLORREF)(-1);
	m_clrRibbonKeyTipTextDisabled = (COLORREF)(-1);

	m_ctrlRibbonContextSeparator.CleanUp ();
	for (int i = 0; i < BCGPRibbonCategoryColorCount; i++)
	{
		m_ctrlRibbonContextCategory[i].CleanUp ();
	}

	m_cacheRibbonCategoryBack.Clear ();
	m_cacheRibbonPanelBack.Clear ();
	m_cacheRibbonBtnGroup_S.Clear();
	m_cacheRibbonBtnGroup_F.Clear();
	m_cacheRibbonBtnGroup_M.Clear();
	m_cacheRibbonBtnGroup_L.Clear();
	m_cacheRibbonBtnGroupMenu_F[0].Clear();
	m_cacheRibbonBtnGroupMenu_M[0].Clear();
	m_cacheRibbonBtnGroupMenu_L[0].Clear();
	m_cacheRibbonBtnGroupMenu_F[1].Clear();
	m_cacheRibbonBtnGroupMenu_M[1].Clear();
	m_cacheRibbonBtnGroupMenu_L[1].Clear();
	m_cacheRibbonBtnDefault.Clear();

	m_bToolTipParams = FALSE;
	CBCGPToolTipParams dummy;
	m_ToolTipParams = dummy;

	m_ActivateFlag.RemoveAll();

	m_bLoaded = FALSE;
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnUpdateSystemColors ()
{
	if (globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ())
	{
		CBCGPWinXPVisualManager::OnUpdateSystemColors ();
		return;
	}

	HINSTANCE hinstResOld = NULL;

	if (m_hinstRes == NULL)
	{
		SetStyle(m_Style);
		return;
	}

	if (m_hinstRes != NULL)
	{
		hinstResOld = AfxGetResourceHandle();
		AfxSetResourceHandle(m_hinstRes);
	}

	CleanUp ();

	CBCGPWinXPVisualManager::OnUpdateSystemColors ();

	if (globalData.bIsWindows9x)
	{
		return;
	}

	m_bShadowHighlightedImage = FALSE;
	m_bFadeInactiveImage = FALSE;

	CBCGPTagManager tm;

	if (!tm.LoadFromResource(GetStyleResourceID(), RT_BCG_STYLE_XML))
	{
#if !defined _AFXDLL
		TRACE(_T("\r\nImportant: to enable the Scenic look in static link,\r\n"));
		TRACE(_T("include afxribbon.rc from the RC file in your project.\r\n\r\n"));
		ASSERT(FALSE);
#endif
		if (hinstResOld != NULL)
		{
			AfxSetResourceHandle(hinstResOld);
		}

		return;
	}

	{
		CString strStyle;
		tm.ExcludeTag(_T("STYLE"), strStyle);
		tm.SetBuffer(strStyle);
	}

	CString strItem;

	if (!tm.IsEmpty())
	{
		int nVersion = 0;

		if (tm.ExcludeTag(_T("VERSION"), strItem))
		{
			CBCGPTagManager tmItem(strItem);

			tmItem.ReadInt(_T("NUMBER"), nVersion);

			int nType = 20;
			if (nVersion == 2007)
			{
				tmItem.ReadInt(_T("TYPE"), nType);

				m_bLoaded = TRUE;
			}

			if (m_bLoaded)
			{
				if (tmItem.ExcludeTag(_T("ID_PREFIX"), strItem))
				{
					strItem.TrimLeft();
					strItem.TrimRight();
					m_strStylePrefix = strItem;
				}
			}
		}
	}

	if (!m_bLoaded)
	{
		if (hinstResOld != NULL)
		{
			::AfxSetResourceHandle(hinstResOld);
		}

		return;
	}

#ifdef _DEBUG
	BOOL bToolBarImagesTrace = g_bToolBarImagesTrace;
	g_bToolBarImagesTrace = FALSE;
#endif

	// globals
	if (tm.ExcludeTag (_T("GLOBALS"), strItem))
	{
		CBCGPTagManager tmItem(strItem);

		tmItem.ReadColor (_T("BarText"), globalData.clrBarText);

		// ToolTipParams
		m_bToolTipParams = tmItem.ReadToolTipParams (_T("TOOLTIP"), m_ToolTipParams);

		if (m_hThemeWindow == NULL)
		{
			tmItem.ReadColor (_T("GrayedText"), globalData.clrGrayedText);

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

			if (tmItem.ReadColor (_T("HighlightNormal"), m_clrHighlight))
			{
				m_brHighlight.DeleteObject ();
				m_brHighlight.CreateSolidBrush (m_clrHighlight);
			}
			if (tmItem.ReadColor (_T("HighlightDown"), m_clrHighlightDn))
			{
				m_brHighlightDn.DeleteObject ();
				m_brHighlightDn.CreateSolidBrush (m_clrHighlightDn);
			}
			if (tmItem.ReadColor (_T("HighlightChecked"), m_clrHighlightChecked))
			{
				m_brHighlightChecked.DeleteObject ();
				m_brHighlightChecked.CreateSolidBrush (m_clrHighlightChecked);
			}

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

			m_brTabBack.DeleteObject();
			m_brTabBack.CreateSolidBrush(globalData.clrBarFace);

			m_brTabFace.DeleteObject();
			m_brTabFace.CreateSolidBrush(globalData.clrBarFace);

			m_clrInactiveTabText = globalData.clrBtnDkShadow;

			m_brGripperHorz.DeleteObject();
			m_brGripperHorz.CreateSolidBrush(globalData.clrBarShadow);

			m_brGripperVert.DeleteObject();
			m_brGripperVert.CreateSolidBrush(globalData.clrBarShadow);

			m_clrPaneBorder = globalData.clrBarDkShadow;

			m_clrDockingBarScrollButton = CBCGPDrawManager::PixelAlpha (globalData.clrBarFace, 91);
			m_clrDockingBarScrollButtonBorder = globalData.clrBarShadow;

			m_clrGripper = CBCGPDrawManager::PixelAlpha (globalData.clrBarShadow, 110);
		}
	}

	m_clrMenuText            = globalData.clrBarText;
	m_clrMenuTextHighlighted = m_clrMenuText;
	m_clrMenuTextDisabled    = globalData.clrGrayedText;

	// menu
	if (tm.ExcludeTag(_T("MENU"), strItem))
	{
		CBCGPTagManager tmItem(strItem);

		if (tmItem.ReadColor (_T("Light"), m_clrMenuLight))
		{
			m_brMenuLight.DeleteObject ();
			m_brMenuLight.CreateSolidBrush (m_clrMenuLight);
		}

		m_clrMenuRarelyUsed = CLR_DEFAULT;
		tmItem.ReadColor (_T("Rarely"), m_clrMenuRarelyUsed);

		tmItem.ReadColor (_T("Border"), m_clrMenuBorder);

		tmItem.ReadColor (_T("Separator1"), m_clrSeparator);
		m_clrMenuSeparator = m_clrSeparator;

		COLORREF clrSeparator;
		if (tmItem.ReadColor (_T("Separator2"), clrSeparator))
		{
			m_penSeparatorLight.DeleteObject ();
			m_penSeparatorLight.CreatePen (PS_SOLID, 1, clrSeparator);
		}

		if (tmItem.ReadColor (_T("ItemBorder"), m_clrMenuItemBorder))
		{
			m_penMenuItemBorder.DeleteObject ();
			m_penMenuItemBorder.CreatePen (PS_SOLID, 1, m_clrMenuItemBorder);
		}

		tmItem.ReadInt (_T("BorderSize"), m_nMenuBorderSize);

		tmItem.ReadControlRenderer (_T("ItemBack"), m_ctrlMenuItemBack, MakeResourceID(_T("MENU_ITEM_BACK")));
		tmItem.ReadToolBarImages (_T("ItemCheck"), m_MenuItemMarkerC, MakeResourceID(_T("MENU_ITEM_MARKER_C")));
		tmItem.ReadToolBarImages (_T("ItemRadio"), m_MenuItemMarkerR, MakeResourceID(_T("MENU_ITEM_MARKER_R")));
		tmItem.ReadControlRenderer (_T("Highlighted"), m_ctrlMenuHighlighted[0], MakeResourceID(_T("MENU_BTN")));
		tmItem.ReadControlRenderer (_T("HighlightedDisabled"), m_ctrlMenuHighlighted[1], MakeResourceID(_T("MENU_BTN_DISABLED")));

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
	}

	m_penSeparator.DeleteObject ();
	m_penSeparator.CreatePen (PS_SOLID, 1, m_clrSeparator);
	m_penSeparatorDark.DeleteObject ();
	m_penSeparatorDark.CreatePen (PS_SOLID, 1, m_clrSeparator);

	m_brMenuRarelyUsed.DeleteObject();
	m_brMenuRarelyUsed.CreateSolidBrush(m_clrMenuRarelyUsed);

	m_clrRibbonEdit            = globalData.clrWindow;
	m_clrRibbonEditHighlighted = globalData.clrWindow;
	m_clrRibbonEditPressed     = m_clrRibbonEditHighlighted;
	m_clrRibbonEditDisabled    = globalData.clrBtnFace;

	m_clrRibbonEditBorder            = globalData.clrWindow;
	m_clrRibbonEditBorderDisabled    = globalData.clrBtnShadow;
	m_clrRibbonEditBorderHighlighted = m_clrMenuItemBorder;
	m_clrRibbonEditBorderPressed     = m_clrRibbonEditBorderHighlighted;
	m_clrRibbonEditSelection         = globalData.clrHilite;

	m_clrRibbonPanelText            = globalData.clrBarText;
	m_clrRibbonPanelTextHighlighted = m_clrRibbonPanelText;

	m_clrRibbonBarBtnText            = m_clrRibbonPanelText;
	m_clrRibbonBarBtnTextHighlighted = m_clrRibbonBarBtnText;
	m_clrRibbonBarBtnTextDisabled    = globalData.clrGrayedText;

	// bars
	if (tm.ExcludeTag (_T("BARS"), strItem))
	{
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
	}

	m_clrRibbonCategoryText                = globalData.clrBarText;
	m_clrRibbonCategoryTextHighlighted     = globalData.clrBarText;
	m_clrRibbonCategoryTextDisabled		   = globalData.clrBarText;

	if (tm.ExcludeTag(_T("RIBBON"), strItem))
	{
		CBCGPTagManager tmItem(strItem);

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
				tmTab.ReadColor (_T("TextDisabled"), m_clrRibbonCategoryTextDisabled);
			}

			tmCategory.ReadControlRenderer (_T("TAB_SEPARATOR"), m_ctrlRibbonCategoryTabSep, MakeResourceID(_T("RB_CAT_TAB_SEP")));
			tmCategory.ReadControlRenderer (_T("TAB_FRAME"), m_ctrlRibbonCategoryTabFrame, MakeResourceID(_T("RB_CAT_TAB_FRAME")));

			tmCategory.ReadControlRenderer (_T("BUTTON_PAGE_L"), m_ctrlRibbonCategoryBtnPage[0], MakeResourceID(_T("RB_BTN_PAGE_L")));
			tmCategory.ReadControlRenderer (_T("BUTTON_PAGE_R"), m_ctrlRibbonCategoryBtnPage[1], MakeResourceID(_T("RB_BTN_PAGE_R")));
		}

		if (tmItem.ExcludeTag(_T("PANEL"), str))
		{
			CBCGPTagManager tmPanel(str);

			{
				CString strBack;
				if (tmPanel.ExcludeTag (_T("BACK"), strBack))
				{
					CBCGPTagManager tmBack (strBack);

					tmBack.ReadControlRenderer (_T("FULL"), m_ctrlRibbonPanelBack, MakeResourceID(_T("RB_PNL_BACK")));
					tmBack.ReadControlRenderer (_T("SEPARATOR"), m_ctrlRibbonPanelBackSep, MakeResourceID(_T("RB_PNL_BACK_SEP")));
				}
			}

			{
				CString strCaption;
				if (tmPanel.ExcludeTag(_T("CAPTION"), strCaption))
				{
					CBCGPTagManager tmCaption(strCaption);

					tmCaption.ReadControlRenderer(_T("LAUNCH_BTN"), m_ctrlRibbonBtnLaunch, MakeResourceID(_T("RB_BTN_LAUNCH")));
					tmCaption.ReadToolBarImages(_T("LAUNCH_ICON"), m_RibbonBtnLaunchIcon, MakeResourceID(_T("RB_BTN_LAUNCH_ICON")));
					tmCaption.ReadColor(_T("TextNormal"), m_clrRibbonPanelCaptionText);
					tmCaption.ReadColor(_T("TextHighlighted"), m_clrRibbonPanelCaptionTextHighlighted);
				}
			}

			tmPanel.ReadToolBarImages (_T("SEPARATOR"), m_RibbonPanelSeparator, MakeResourceID(_T("RB_PNL_SEPARATOR")));
			tmPanel.ReadControlRenderer (_T("QAT"), m_ctrlRibbonPanelQAT, MakeResourceID(_T("RB_PNL_QAT")));

			{
				CString strButtons;
				if (tmPanel.ExcludeTag(_T("BUTTONS"), strButtons))
				{
					CBCGPTagManager tmButtons(strButtons);

					tmButtons.ReadControlRenderer(_T("BUTTON_GROUP_F"), m_ctrlRibbonBtnGroup_F, MakeResourceID(_T("RB_BTN_GRP_F")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUP_M"), m_ctrlRibbonBtnGroup_M, MakeResourceID(_T("RB_BTN_GRP_M")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUP_L"), m_ctrlRibbonBtnGroup_L, MakeResourceID(_T("RB_BTN_GRP_L")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUP_S"), m_ctrlRibbonBtnGroup_S, MakeResourceID(_T("RB_BTN_GRP_S")));

					tmButtons.ReadControlRenderer(_T("BUTTON_GROUPMENU_F_C"), m_ctrlRibbonBtnGroupMenu_F[0], MakeResourceID(_T("RB_BTN_GRPMENU_F_C")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUPMENU_F_M"), m_ctrlRibbonBtnGroupMenu_F[1], MakeResourceID(_T("RB_BTN_GRPMENU_F_M")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUPMENU_M_C"), m_ctrlRibbonBtnGroupMenu_M[0], MakeResourceID(_T("RB_BTN_GRPMENU_M_C")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUPMENU_M_M"), m_ctrlRibbonBtnGroupMenu_M[1], MakeResourceID(_T("RB_BTN_GRPMENU_M_M")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUPMENU_L_C"), m_ctrlRibbonBtnGroupMenu_L[0], MakeResourceID(_T("RB_BTN_GRPMENU_L_C")));
					tmButtons.ReadControlRenderer(_T("BUTTON_GROUPMENU_L_M"), m_ctrlRibbonBtnGroupMenu_L[1], MakeResourceID(_T("RB_BTN_GRPMENU_L_M")));

					tmButtons.ReadControlRenderer(_T("BUTTON_NORMAL_S"), m_ctrlRibbonBtn[0], MakeResourceID(_T("RB_BTN_NORMAL_S")));
					tmButtons.ReadControlRenderer(_T("BUTTON_NORMAL_B"), m_ctrlRibbonBtn[1], MakeResourceID(_T("RB_BTN_NORMAL_B")));

					tmButtons.ReadControlRenderer(_T("BUTTON_DEFAULT"), m_ctrlRibbonBtnDefault, MakeResourceID(_T("RB_BTN_DEF")));
					tmButtons.ReadControlRenderer(_T("BUTTON_DEFAULT_ICON"), m_ctrlRibbonBtnDefaultIcon, MakeResourceID(_T("RB_BTN_DEF_ICON")));
					tmButtons.ReadToolBarImages(_T("BUTTON_DEFAULT_IMAGE"), m_RibbonBtnDefaultImage, MakeResourceID(_T("RB_BTN_DEF_IMAGE")));
					tmButtons.ReadControlRenderer(_T("BUTTON_DEFAULT_QAT"), m_ctrlRibbonBtnDefaultQAT, MakeResourceID(_T("RB_BTN_DEF_QAT")));

					tmButtons.ReadControlRenderer(_T("BUTTON_MENU_H_C"), m_ctrlRibbonBtnMenuH[0], MakeResourceID(_T("RB_BTN_MENU_H_C")));
					tmButtons.ReadControlRenderer(_T("BUTTON_MENU_H_M"), m_ctrlRibbonBtnMenuH[1], MakeResourceID(_T("RB_BTN_MENU_H_M")));
					tmButtons.ReadControlRenderer(_T("BUTTON_MENU_V_C"), m_ctrlRibbonBtnMenuV[0], MakeResourceID(_T("RB_BTN_MENU_V_C")));
					tmButtons.ReadControlRenderer(_T("BUTTON_MENU_V_M"), m_ctrlRibbonBtnMenuV[1], MakeResourceID(_T("RB_BTN_MENU_V_M")));
					tmButtons.ReadControlRenderer(_T("BUTTON_CHECK"), m_ctrlRibbonBtnCheck, MakeResourceID(_T("RB_BTN_CHECK")));
					tmButtons.ReadControlRenderer(_T("BUTTON_RADIO"), m_ctrlRibbonBtnRadio, MakeResourceID(_T("RB_BTN_RADIO")));

					m_ctrlRibbonBtnCheck.SmoothResize(globalData.GetRibbonImageScale());
					m_ctrlRibbonBtnRadio.SmoothResize(globalData.GetRibbonImageScale());

					tmButtons.ReadControlRenderer(_T("BUTTON_PNL_T"), m_ctrlRibbonBtnPalette[0], MakeResourceID(_T("RB_BTN_PALETTE_T")));
					tmButtons.ReadControlRenderer(_T("BUTTON_PNL_M"), m_ctrlRibbonBtnPalette[1], MakeResourceID(_T("RB_BTN_PALETTE_M")));
					tmButtons.ReadControlRenderer(_T("BUTTON_PNL_B"), m_ctrlRibbonBtnPalette[2], MakeResourceID(_T("RB_BTN_PALETTE_B")));

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

						tmButton.ReadControlRenderer (_T("IMAGE"), m_ctrlRibbonComboBoxBtn, MakeResourceID(_T("COMBOBOX_BTN")));
					}
				}
			}

			tmPanel.ReadColor (_T("TextNormal"), m_clrRibbonPanelText);
			tmPanel.ReadColor (_T("TextHighlighted"), m_clrRibbonPanelTextHighlighted);
		}

		if (tmItem.ExcludeTag (_T("CONTEXT"), str))
		{
			CBCGPTagManager tmContext (str);

			CString strCategory;
			if (tmContext.ExcludeTag (_T("CATEGORY"), strCategory))
			{
				CBCGPTagManager tmCategory (strCategory);

				CBCGPControlRendererParams prDefault;
				CBCGPControlRendererParams prBack;
				CBCGPControlRendererParams prCaption;
				CBCGPControlRendererParams prTab;
				CBCGPControlRendererParams prSeparator;
				CBCGPControlRendererParams prPanelBack;
				COLORREF clrText = m_clrRibbonCategoryText;
				COLORREF clrTextHighlighted = m_clrRibbonCategoryTextHighlighted;
				COLORREF clrCaptionText = clrText;

				tmCategory.ReadControlRendererParams (_T("BACK"), prBack);

				CString strTab;
				if (tmCategory.ExcludeTag (_T("TAB"), strTab))
				{
					CBCGPTagManager tmTab (strTab);

					tmTab.ReadControlRendererParams(_T("BUTTON"), prTab);
					tmTab.ReadColor (_T("TextNormal"), clrText);
					tmTab.ReadColor (_T("TextHighlighted"), clrTextHighlighted);
				}

				CString strCaption;
				if (tmCategory.ExcludeTag (_T("CAPTION"), strCaption))
				{
					CBCGPTagManager tmCaption (strCaption);

					tmCaption.ReadControlRendererParams(_T("BACK"), prCaption);
					tmCaption.ReadColor (_T("TextNormal"), clrCaptionText);
				}

				tmCategory.ReadControlRendererParams(_T("BUTTON_DEFAULT"), prDefault);
				tmCategory.ReadControlRendererParams (_T("SEPARATOR"), prSeparator);
				tmCategory.ReadControlRendererParams (_T("PANEL"), prPanelBack);

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

					prDefault.m_strBmpResID   = strID[i] + _T("BTN_DEF");
					prTab.m_strBmpResID       = strID[i] + _T("CAT_TAB");
					prCaption.m_strBmpResID   = strID[i] + _T("CAT_CAPTION");
					prBack.m_strBmpResID      = strID[i] + _T("CAT_BACK");
					prSeparator.m_strBmpResID = strID[i] + _T("CAT_SEPARATOR");
					prPanelBack.m_strBmpResID = strID[i] + _T("CAT_PANEL");

					cat.m_ctrlBtnDefault.Create (prDefault);
					cat.m_ctrlCaption.Create (prCaption);
					cat.m_ctrlTab.Create (prTab);
					cat.m_ctrlBack.Create (prBack);
					cat.m_ctrlSeparator.Create (prSeparator);
					cat.m_ctrlPanelBack.Create (prPanelBack);
					cat.m_clrText            = clrText;
					cat.m_clrTextHighlighted = clrTextHighlighted;
					cat.m_clrCaptionText     = clrCaptionText;
				}
			}

			tmContext.ReadControlRenderer (_T("SEPARATOR"), m_ctrlRibbonContextSeparator, MakeResourceID(_T("RB_CTX_SEPARATOR")));
		}

		tmItem.ReadControlRenderer(_T("MAIN_BUTTON"), m_ctrlRibbonBtnMain, MakeResourceID(_T("RB_BTN_MAIN")));

		if (tmItem.ExcludeTag(_T("MAIN"), str))
		{
			CBCGPTagManager tmMain(str);

			tmMain.ReadControlRenderer (_T("BACK"), m_ctrlRibbonMainPanel, MakeResourceID(_T("RB_PNL_MAIN")));
			tmMain.ReadControlRenderer(_T("BUTTON"), m_ctrlRibbonBtnMainPanel, MakeResourceID(_T("RB_BTN_PNL_MAIN")));
		}

		if (tmItem.ExcludeTag (_T("CAPTION"), str))
		{
			CBCGPTagManager tmCaption (str);

			tmCaption.ReadControlRenderer(_T("QA"), m_ctrlRibbonCaptionQA, MakeResourceID(_T("RB_CAPTION_QA")));
		}

		if (tmItem.ExcludeTag(_T("STATUS"), str))
		{
			CBCGPTagManager tmStatus(str);
			tmStatus.ReadControlRenderer(_T("PANE_BUTTON"), m_ctrlRibbonBtnStatusPane, MakeResourceID(_T("RB_BTN_STATUS_PANE")));

			CString strSlider;
			if (tmStatus.ExcludeTag(_T("SLIDER"), strSlider))
			{
				CBCGPTagManager tmSlider(strSlider);

				tmSlider.ReadControlRenderer(_T("PLUS"), m_ctrlRibbonSliderBtnPlus, MakeResourceID(_T("RB_SLIDER_BTN_PLUS")));
				tmSlider.ReadControlRenderer(_T("MINUS"), m_ctrlRibbonSliderBtnMinus, MakeResourceID(_T("RB_SLIDER_BTN_MINUS")));
			}
		}

		if (tmItem.ExcludeTag (_T("BORDERS"), str))
		{
			CBCGPTagManager tmBorders (str);

			tmBorders.ReadControlRenderer (_T("QAT"), m_ctrlRibbonBorder_QAT, MakeResourceID(_T("RB_BRD_QAT")));
			tmBorders.ReadControlRenderer (_T("FLOATY"), m_ctrlRibbonBorder_Floaty, MakeResourceID(_T("RB_BRD_FLOATY")));
			tmBorders.ReadControlRenderer (_T("PANEL"), m_ctrlRibbonBorder_Panel, MakeResourceID(_T("RB_BRD_PANEL")));
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
	}

#ifdef _DEBUG
	g_bToolBarImagesTrace = bToolBarImagesTrace;
#endif

	if (hinstResOld != NULL)
	{
		AfxSetResourceHandle(hinstResOld);
	}

	NONCLIENTMETRICS ncm;
	globalData.GetNonClientMetrics (ncm);
	m_AppCaptionFont.CreateFontIndirect (&ncm.lfCaptionFont);
}
//*****************************************************************************
COLORREF CBCGPVisualManagerScenic::GetMenuItemTextColor (
	CBCGPToolbarMenuButton* pButton, BOOL bHighlighted, BOOL bDisabled)
{
	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::GetMenuItemTextColor (pButton, 
			bHighlighted, bDisabled);
	}

	return bDisabled ? m_clrMenuTextDisabled : m_clrMenuText;
}
//*****************************************************************************
COLORREF CBCGPVisualManagerScenic::GetHighlightedMenuItemTextColor (CBCGPToolbarMenuButton* pButton)
{
	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::GetHighlightedMenuItemTextColor (pButton);
	}

	return m_clrMenuTextHighlighted;
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawSeparator (CDC* pDC, CBCGPBaseControlBar* pBar, CRect rect, BOOL bHorz)
{
	ASSERT_VALID (pDC);

	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawSeparator (pDC, pBar, rect, bHorz);
		return;
	}

	ASSERT_VALID (pBar);

	CRect rectSeparator (rect);

#ifndef BCGP_EXCLUDE_RIBBON
	if (CanDrawImage () && pBar != NULL && !pBar->IsDialogControl ())
	{
		if ((pBar->IsKindOf (RUNTIME_CLASS (CBCGPRibbonBar)) ||
			 (bHorz && pBar->IsKindOf (RUNTIME_CLASS (CBCGPRibbonPanelMenuBar)))) &&
			 m_RibbonPanelSeparator.IsValid ())
		{
			if (rect.Width () < m_RibbonPanelSeparator.GetImageSize ().cx)
			{
				rect.left = rect.right - m_RibbonPanelSeparator.GetImageSize ().cx;
			}
			
			m_RibbonPanelSeparator.DrawEx (pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzCenter,
				CBCGPToolBarImages::ImageAlignVertCenter);
			return;
		}
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
void CBCGPVisualManagerScenic::OnDrawMenuBorder (CDC* pDC, CBCGPPopupMenu* pMenu, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawMenuBorder (pDC, pMenu, rect);
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
					return;
				}
				else if (pRibbonMenuBar->IsFloaty ())
				{
					m_ctrlRibbonBorder_Floaty.DrawFrame (pDC, rect);
					return;
				}
				else
				{
					if (pRibbonMenuBar->GetPanel () != NULL)
					{
						m_ctrlRibbonBorder_Panel.DrawFrame (pDC, rect);
						return;
					}
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
		CBCGPWinXPVisualManager::OnDrawMenuBorder (pDC, pMenu, rect);
	}
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawMenuCheck (CDC* pDC, CBCGPToolbarMenuButton* pButton, 
		CRect rect, BOOL bHighlight, BOOL bIsRadio)
{
	ASSERT_VALID (pButton);

    CBCGPToolBarImages& img = bIsRadio ? m_MenuItemMarkerR : m_MenuItemMarkerC;

	if (!CanDrawImage () || img.GetCount () == 0)
	{
		CBCGPWinXPVisualManager::OnDrawMenuCheck (pDC, pButton, rect, bHighlight, bIsRadio);
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
//*****************************************************************************
void CBCGPVisualManagerScenic::OnHighlightMenuItem (CDC *pDC, CBCGPToolbarMenuButton* pButton,
		CRect rect, COLORREF& clrText)
{
	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnHighlightMenuItem (pDC, pButton, rect, clrText);
		return;
	}

	clrText = globalData.clrMenuText;
	m_ctrlMenuHighlighted[(pButton->m_nStyle & TBBS_DISABLED) == TBBS_DISABLED ? 1 : 0].Draw (pDC, rect);
}
//********************************************************************************
CSize CBCGPVisualManagerScenic::GetCheckRadioDefaultSize ()
{
	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::GetCheckRadioDefaultSize ();
	}

	return m_ctrlRibbonBtnCheck.GetParams ().m_rectImage.Size () + CSize (2, 2);
}
//********************************************************************************
BOOL CBCGPVisualManagerScenic::DrawCheckBox (CDC *pDC, CRect rect, 
										 BOOL bHighlighted, 
										 int nState,
										 BOOL bEnabled,
										 BOOL bPressed)
{
	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::DrawCheckBox (pDC, rect, bHighlighted, nState, bEnabled, bPressed);
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

BOOL CBCGPVisualManagerScenic::DrawRadioButton (CDC *pDC, CRect rect, 
										 BOOL bHighlighted, 
										 BOOL bChecked,								 
										 BOOL bEnabled,
										 BOOL bPressed)
{
	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::DrawRadioButton (pDC, rect, bHighlighted, bChecked, bEnabled, bPressed);
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
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawRibbonCaption (CDC* pDC, CBCGPRibbonBar* pBar,
											  CRect rectCaption, CRect rectText)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pBar);

	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawRibbonCaption(pDC, pBar, rectCaption, rectText);
		return;
	}

	CWnd* pWnd = pBar->GetParent ();
	ASSERT_VALID (pWnd);

	const DWORD dwStyleEx = pWnd->GetExStyle ();

	const BOOL bIsRTL     = (dwStyleEx & WS_EX_LAYOUTRTL) == WS_EX_LAYOUTRTL;
	const BOOL bActive    = IsWindowActive (pWnd);
	const BOOL bGlass	  = pBar->IsTransparentCaption ();

    {
		CSize szSysBorder(globalUtils.GetSystemBorders (pBar->GetParent()));
		if (szSysBorder.cx == 0 && szSysBorder.cy == 0)
		{
			szSysBorder = globalUtils.GetSystemBorders (BCGCBProGetTopLevelFrame(pBar));
		}		

		if (!bGlass)
		{
			CRect rectCaption1(rectCaption);
			rectCaption1.InflateRect(szSysBorder.cx, szSysBorder.cy, szSysBorder.cx, 0);

			if (m_hThemeWindow != NULL)
			{
				(*m_pfDrawThemeBackground) (m_hThemeWindow, pDC->GetSafeHdc(),
						/*WP_CAPTION*/1, bActive ? /*CS_ACTIVE*/1 : /*CS_INACTIVE*/2, &rectCaption1, 0);
			}
			else
			{
				CBCGPDrawManager dm (*pDC);
				dm.FillGradient (rectCaption1, 
					bActive ? globalData.clrActiveCaption : globalData.clrInactiveCaption, 
					bActive ? globalData.clrActiveCaptionGradient : globalData.clrInactiveCaptionGradient, FALSE);
			}
		}

		CRect rectQAT = pBar->GetQuickAccessToolbarLocation ();

		if (rectQAT.left > rectQAT.right)
		{
			rectText.left = rectQAT.left + 1;
		}

		BOOL bHide  = (pBar->GetHideFlags () & BCGPRIBBONBAR_HIDE_ALL) != 0;
		BOOL bExtra = !bHide && pBar->IsQuickAccessToolbarOnTop () &&
					  rectQAT.left < rectQAT.right && !pBar->IsQATEmpty();

		BOOL bDrawIcon = (bHide && !bExtra) || pBar->IsScenicLook ();

		if (bExtra)
		{
			CRect rectQAFrame (rectQAT);
			rectQAFrame.right = rectText.left - 6;
			rectQAFrame.InflateRect (1, 1, 1, 1);

			const CBCGPControlRendererParams& params = m_ctrlRibbonCaptionQA.GetParams ();

			if (rectQAFrame.Height () < params.m_rectImage.Height ())
			{
				rectQAFrame.top = rectQAFrame.bottom - params.m_rectImage.Height ();
			}

			m_ctrlRibbonCaptionQA.Draw (pDC, rectQAFrame, bActive ? 0 : 1);
		}
		
		if (bDrawIcon)
		{
			BOOL bDestroyIcon = FALSE;
			HICON hIcon = globalUtils.GetWndIcon (pWnd, &bDestroyIcon, FALSE);

			if (hIcon != NULL)
			{
				CSize szIcon (::GetSystemMetrics (SM_CXSMICON), ::GetSystemMetrics (SM_CYSMICON));

				long x = rectCaption.left + 2 + pBar->GetControlsSpacing().cx / 2;
				long y = rectCaption.top + max (0, (rectCaption.Height () - szIcon.cy) / 2);
				
				if (globalData.DwmIsCompositionEnabled () && IsDWMCaptionSupported())
				{
					y += 2;
				}

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

	DrawNcText (pDC, rectText, strText, bActive, 
		bIsRTL, m_bNcTextCenter, bGlass, (pWnd->IsZoomed () && !globalData.bIsWindows7) ? 0 : 10, 
		(pWnd->IsZoomed() && !globalData.bIsWindows7) ? RGB (255, 255, 255) : (COLORREF)-1);

	pDC->SelectObject (pOldFont);
}
//*****************************************************************************
COLORREF CBCGPVisualManagerScenic::OnDrawRibbonTabsFrame (
					CDC* pDC, 
					CBCGPRibbonBar* pWndRibbonBar, 
					CRect rectTab)
{
	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::OnDrawRibbonTabsFrame(pDC, pWndRibbonBar, rectTab);
	}

	return globalData.m_bIsBlackHighContrast ? globalData.clrBtnText : (COLORREF)-1;
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawRibbonCaptionButton(CDC* pDC, CBCGPRibbonCaptionButton* pButton)
{
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

	CRect rect(pButton->GetRect());

	if (bMDI)
	{
		rect.DeflateRect(0, 2);
	}
	else
	{
		rect.DeflateRect(2, 2);
	}

	DrawNcBtn(pDC, rect, pButton->GetID(), state, FALSE, bActive, bMDI, !pButton->IsDisabled());
}
//*******************************************************************************
COLORREF CBCGPVisualManagerScenic::OnDrawRibbonButtonsGroup (
					CDC* pDC, CBCGPRibbonButtonsGroup* pGroup,
					CRect rectGroup)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pGroup);

	if (CanDrawImage () && m_ctrlRibbonPanelQAT.IsValid () && 
		pGroup->IsKindOf (RUNTIME_CLASS(CBCGPRibbonQuickAccessToolbar)))
	{
		CBCGPRibbonBar* pBar = pGroup->GetParentRibbonBar ();

		if (pBar != NULL &&
			(pBar->GetHideFlags () & BCGPRIBBONBAR_HIDE_ALL) == 0 &&
			!pBar->IsQuickAccessToolbarOnTop ())
		{
			m_ctrlRibbonPanelQAT.Draw (pDC, rectGroup);
		}

		return (COLORREF)-1;
	}

	return CBCGPWinXPVisualManager::OnDrawRibbonButtonsGroup (pDC, pGroup, rectGroup);
}
//*****************************************************************************
COLORREF CBCGPVisualManagerScenic::OnDrawRibbonPanel (
		CDC* pDC,
		CBCGPRibbonPanel* pPanel, 
		CRect rectPanel,
		CRect rectCaption)
{
	if (GetCurrStyle() == VSScenic_Win8)
	{
		CPen* pOldPen = pDC->SelectObject(&m_penSeparator);

		pDC->MoveTo(rectPanel.right - 1, rectPanel.top + 1);
		pDC->LineTo(rectPanel.right - 1, rectPanel.bottom - 1);

		pDC->SelectObject(pOldPen);
		return globalData.clrBarText;
	}

	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::OnDrawRibbonPanel (pDC, pPanel, rectPanel, rectCaption);
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pPanel);

	COLORREF clrText = m_clrRibbonPanelText;

	if (pPanel->IsKindOf (RUNTIME_CLASS(CBCGPRibbonMainPanel)))
	{
		if (!pPanel->IsBackstageView())
		{
			const int nBorderSize = GetPopupMenuBorderSize ();
			rectPanel.InflateRect (nBorderSize, nBorderSize);
			
			m_ctrlRibbonMainPanel.Draw (pDC, rectPanel);
		}
		else
		{
			rectPanel = ((CBCGPRibbonMainPanel*)pPanel)->GetCommandsFrame ();
			{
				CBCGPDrawManager dm (*pDC);
				dm.FillGradient (rectPanel,	m_clrBarGradientDark,
											m_clrBarGradientLight,
											TRUE);
			}

			rectPanel.right--;
			rectPanel.left = rectPanel.right;

			CPen* pOldPen = pDC->SelectObject (&m_penSeparator);
			ASSERT (pOldPen != NULL);

			pDC->MoveTo (rectPanel.TopLeft ());
			pDC->LineTo (rectPanel.BottomRight ());

			pDC->SelectObject (pOldPen);
		}
	}
	else
	{
		if (!pPanel->IsMenuMode () && !pPanel->IsCollapsed ())
		{
			BOOL bHighlighted = pPanel->IsHighlighted ();

			if (bHighlighted)
			{
				clrText = m_clrRibbonPanelTextHighlighted;
			}

			CBCGPControlRenderer* pRenderer = &m_ctrlRibbonPanelBack;
			CBCGPBitmapCache* pCache = &m_cacheRibbonPanelBack;

			CBCGPRibbonCategory* pCategory = pPanel->GetParentCategory ();
			ASSERT_VALID (pCategory);

			CBCGPBaseRibbonElement* pParentButton = pPanel->GetParentButton ();

			if (pCategory->GetTabColor () != BCGPCategoryColor_None &&
				(pParentButton == NULL || !pParentButton->IsQATMode ()))
			{
				XRibbonContextCategory& context = 
					m_ctrlRibbonContextCategory[pCategory->GetTabColor () - 1];

				if (context.m_ctrlPanelBack.IsValid ())
				{
					pRenderer = &context.m_ctrlPanelBack;
					pCache    = &context.m_cachePanelBack;
				}
			}

			const CBCGPControlRendererParams& params = pRenderer->GetParams ();

			CRect rectBack (rectPanel);
			CBCGPRibbonPanelMenuBar* pMenuBar = pPanel->GetParentMenuBar ();
			if (m_ctrlRibbonPanelBackSep.IsValid () && (pMenuBar == NULL || pMenuBar->GetPanel () == NULL))
			{
				CRect rectSep (rectPanel);

				rectSep.left = rectSep.right - m_ctrlRibbonPanelBackSep.GetParams ().m_rectImage.Width ();
				m_ctrlRibbonPanelBackSep.Draw (pDC, rectSep);
				rectPanel.right = rectSep.left;
			}

			int nCacheIndex = -1;
			CSize size (params.m_rectImage.Width (), rectPanel.Height ());
			nCacheIndex = pCache->FindIndex (size);
			if (nCacheIndex == -1)
			{
				nCacheIndex = pCache->CacheY (size.cy, *pRenderer);
			}

			if (nCacheIndex != -1)
			{
				pCache->Get(nCacheIndex)->DrawY (pDC, rectBack, 
					CSize (params.m_rectInter.left, params.m_rectImage.right - params.m_rectInter.right), bHighlighted ? 1 : 0);
			}
			else
			{
				pRenderer->Draw (pDC, rectBack, bHighlighted ? 1 : 0);
			}
		}
	}

	return clrText;
}
//*****************************************************************************
COLORREF CBCGPVisualManagerScenic::OnFillRibbonPanelCaption (
					CDC* /*pDC*/,
					CBCGPRibbonPanel* /*pPanel*/, 
					CRect /*rectCaption*/)
{
	return m_clrRibbonPanelCaptionText;
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawRibbonCategory (
		CDC* pDC, 
		CBCGPRibbonCategory* pCategory,
		CRect rectCategory)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pCategory);

	if (!CanDrawImage () || pCategory->IsOnDialogBar ())
	{
		CBCGPWinXPVisualManager::OnDrawRibbonCategory (pDC, pCategory, rectCategory);
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
void CBCGPVisualManagerScenic::OnDrawRibbonCategoryScroll (
					CDC* pDC, 
					CBCGPRibbonCategoryScroll* pScroll)
{
	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawRibbonCategoryScroll (pDC, pScroll);
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
COLORREF CBCGPVisualManagerScenic::OnDrawRibbonCategoryTab (
					CDC* pDC, 
					CBCGPRibbonTab* pTab,
					BOOL bIsActive)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTab);

	if (GetCurrStyle() == VSScenic_Win8)
	{
	}

	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::OnDrawRibbonCategoryTab (pDC, 
											pTab, bIsActive);
	}

	CBCGPRibbonCategory* pCategory = pTab->GetParentCategory ();
	ASSERT_VALID (pCategory);
	CBCGPRibbonBar* pBar = pCategory->GetParentRibbonBar ();
	ASSERT_VALID (pBar);

	bIsActive = bIsActive && 
		((pBar->GetHideFlags () & BCGPRIBBONBAR_HIDE_ELEMENTS) == 0 || pTab->GetDroppedDown () != NULL);

	const BOOL bPressed     = pTab->IsPressed ();
	const BOOL bIsFocused	= pTab->IsFocused () && (pBar->GetHideFlags () & BCGPRIBBONBAR_HIDE_ELEMENTS);
	const BOOL bIsHighlight = (pTab->IsHighlighted () || bIsFocused) && !pTab->IsDroppedDown ();

	CRect rectTab (pTab->GetRect ());
	rectTab.bottom++;

	int ratio = 0;
	if (m_ctrlRibbonCategoryTabSep.IsValid ())
	{
		ratio = pBar->GetTabTrancateRatio ();
	}

	if (ratio > 0)
	{
		rectTab.left++;
	}

	CBCGPControlRenderer* pRenderer = &m_ctrlRibbonCategoryTab;
	COLORREF clrText = m_clrRibbonCategoryText;
	COLORREF clrTextHighlighted = m_clrRibbonCategoryTextHighlighted;

	if (pCategory->GetTabColor () != BCGPCategoryColor_None)
	{
		XRibbonContextCategory& context = 
				m_ctrlRibbonContextCategory [pCategory->GetTabColor () - 1];

		pRenderer = &context.m_ctrlTab;
		clrText   = context.m_clrText;
		clrTextHighlighted = context.m_clrTextHighlighted;
	}

	if (bIsActive || bPressed || bIsHighlight)
	{
		int nImage = 1;

		if (bIsHighlight && !bPressed)
		{
			nImage = bIsActive ? 2 : 0;
		}

		pRenderer->Draw (pDC, rectTab, nImage);
	}

	if (ratio > 0)
	{
		CRect rectSep (rectTab);
		rectSep.left = rectSep.right;
		rectSep.right += m_ctrlRibbonCategoryTabSep.GetParams ().m_rectImage.Width ();
		rectSep.bottom--;

		m_ctrlRibbonCategoryTabSep.Draw (pDC, rectSep, 0, (BYTE)min(ratio * 255 / 100, 255));
	}

	return bIsActive
			? clrTextHighlighted
			: clrText;
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnPreDrawRibbon (
					CDC* pDC, 
					CBCGPRibbonBar* pRibbonBar, 
					CRect rectTabs)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pRibbonBar);

	if (!CanDrawImage () || !pRibbonBar->AreTransparentTabs())
	{
		CBCGPWinXPVisualManager::OnPreDrawRibbon (pDC, pRibbonBar, rectTabs);
		return;
	}

	rectTabs.top -= pRibbonBar->GetCaptionHeight ();
	m_ctrlRibbonCategoryTabFrame.Draw (pDC, rectTabs);
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawRibbonMenuCheckFrame (
					CDC* pDC,
					CBCGPRibbonButton* pButton, 
					CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawRibbonMenuCheckFrame (pDC, pButton, rect);
		return;
	}

	ASSERT_VALID (pDC);

	m_ctrlMenuItemBack.Draw (pDC, rect);
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnFillRibbonQATPopup (
				CDC* pDC, CBCGPRibbonPanelMenuBar* pMenuBar, CRect rect)
{
	if (!CanDrawImage () || !m_ctrlRibbonBorder_QAT.IsValid ())
	{
		CBCGPWinXPVisualManager::OnFillRibbonQATPopup (pDC, pMenuBar, rect);
		return;
	}

	ASSERT_VALID (pDC);

	m_ctrlRibbonBorder_QAT.FillInterior (pDC, rect);
}
//*******************************************************************************
void CBCGPVisualManagerScenic::OnDrawRibbonKeyTip (CDC* pDC, 
												 CBCGPBaseRibbonElement* pElement, 
												 CRect rect, CString str)
{
	if (!CanDrawImage () || !m_ctrlRibbonKeyTip.IsValid ())
	{
		CBCGPWinXPVisualManager::OnDrawRibbonKeyTip (pDC, pElement, rect, str);
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
//*****************************************************************************
int CBCGPVisualManagerScenic::GetRibbonPopupBorderSize (const CBCGPRibbonPanelMenu* pPopup) const
{
	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::GetRibbonPopupBorderSize (pPopup);
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
				return m_ctrlRibbonMainPanel.GetParams ().m_rectSides.left;
			}

			if (!pRibbonMenuBar->IsMenuMode ())
			{
				if (pRibbonMenuBar->IsQATPopup ())
				{
					if (m_ctrlRibbonBorder_QAT.IsValid ())
					{
						return m_ctrlRibbonBorder_QAT.GetParams ().m_rectSides.left;
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
						if (m_ctrlRibbonBorder_Panel.IsValid ())
						{
							return m_ctrlRibbonBorder_Panel.GetParams ().m_rectSides.left;
						}
					}
				}
			}
		}
	}

	return (int)GetPopupMenuBorderSize ();
}
//*****************************************************************************
int CBCGPVisualManagerScenic::GetRibbonPanelMargin(CBCGPRibbonCategory* pCategory)
{
	if (pCategory != NULL)
	{
		CBCGPRibbonMainPanel* pPanel = pCategory->GetMainPanel();
		if (pPanel != NULL)
		{
			if (pPanel->IsBackstageView ())
			{
				return 0;
			}
			else if (!pPanel->IsScenicLook ())
			{
				return 4;
			}
			else
			{
				return 0;
			}
		}
	}

	return 2;
}

#endif
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawButtonBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state)
{
	CBCGPToolbarMenuButton* pMenuButton = DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);
	if (CanDrawImage () && pMenuButton != NULL &&
		pMenuButton->GetParentWnd () != NULL &&
		pMenuButton->GetParentWnd ()->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar)))
	{
		return;
	}

	CBCGPWinXPVisualManager::OnDrawButtonBorder (pDC, pButton, rect, state);
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pBar);

	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnFillBarBackground (pDC, pBar, rectClient, rectClip, bNCArea);
		return;
	}

	if (pBar->IsOnGlass ())
	{
		pDC->FillSolidRect (rectClient, RGB (0, 0, 0));
		return;
	}

#ifndef BCGP_EXCLUDE_RIBBON
	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPRibbonStatusBar)))
	{
		if (m_hThemeWindow != NULL)
		{
			(*m_pfDrawThemeBackground) (m_hThemeStatusBar, pDC->GetSafeHdc(),
				0, 0, &rectClient, 0);
			return;
		}
	}
	else if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPRibbonBar)))
	{
		CBCGPDrawManager dm (*pDC);
		dm.FillGradient (rectClient, m_clrBarGradientDark, m_clrBarGradientLight, TRUE);

		return;
	}
	else if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPCaptionBar)))
	{
		CBCGPCaptionBar* pCaptionBar = (CBCGPCaptionBar*) pBar;

		if (pCaptionBar->m_clrBarBackground != -1)
		{
			COLORREF clrGradient = CBCGPDrawManager::PixelAlpha (pCaptionBar->m_clrBarBackground, 150);
			COLORREF clrBorder = CBCGPDrawManager::PixelAlpha (pCaptionBar->m_clrBarBackground, 80);

			CBCGPDrawManager dm (*pDC);
			dm.FillGradient (rectClient, pCaptionBar->m_clrBarBackground, clrGradient, TRUE);

			pDC->Draw3dRect(rectClient, clrBorder, clrBorder);
		}
		else if (pCaptionBar->IsMessageBarMode ())
		{
			pDC->FillRect (rectClip, &globalData.brBarFace);
		}
		else
		{
			CBCGPDrawManager dm (*pDC);
			dm.FillGradient (rectClient, m_clrBarGradientDark, RGB(255, 255, 255), TRUE);
		}

		return;
	}
	else if (pBar->IsKindOf(RUNTIME_CLASS (CBCGPCalculator)))
	{
		pDC->FillRect (rectClient, &m_brBarBkgnd);
		return;
	}
	else if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar)))
	{
		OnFillPopupMenuBackground (pDC, rectClient);

		if (!pBar->IsKindOf (RUNTIME_CLASS (CBCGPColorBar)) &&
			!pBar->IsKindOf (RUNTIME_CLASS (CBCGPCalculator)) &&
			!pBar->IsKindOf (RUNTIME_CLASS (CBCGPCalendarBar)))
		{
			CBCGPPopupMenuBar* pMenuBar = DYNAMIC_DOWNCAST (CBCGPPopupMenuBar, pBar);
			if (!pMenuBar->m_bDisableSideBarInXPMode)
			{
				CRect rectGutter = rectClient;

				rectGutter.right = rectGutter.left + pMenuBar->GetGutterWidth () + 2;

				if (!pMenuBar->HasGutterLogo())
				{
					rectGutter.DeflateRect (0, 1);
				}

				pDC->FillRect(rectGutter, &m_brBarBkgnd);

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
		}
		return;
	}
#endif

	CBCGPWinXPVisualManager::OnFillBarBackground (pDC, pBar, rectClient, rectClip, bNCArea);
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawCaptionBarButtonBorder (CDC* pDC, CBCGPCaptionBar* pBar,
											CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
											BOOL bIsDisabled, BOOL bHasDropDownArrow,
											BOOL bIsSysButton)
{
	ASSERT_VALID (pBar);

	if (!pBar->IsMessageBarMode () || !CanDrawImage () || pBar->m_clrBarBackground == -1)
	{
		CBCGPWinXPVisualManager::OnDrawCaptionBarButtonBorder (pDC, pBar,
											rect, bIsPressed, bIsHighlighted, 
											bIsDisabled, bHasDropDownArrow, bIsSysButton);
	}
}
//**************************************************************************************
COLORREF CBCGPVisualManagerScenic::OnFillCaptionBarButton (CDC* pDC, CBCGPCaptionBar* pBar,
											CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
											BOOL bIsDisabled, BOOL bHasDropDownArrow,
											BOOL bIsSysButton)
{
	ASSERT_VALID (pBar);

	if (!pBar->IsMessageBarMode () || !CanDrawImage () || pBar->m_clrBarBackground == -1)
	{
		return CBCGPWinXPVisualManager::OnFillCaptionBarButton (pDC, pBar,
											rect, bIsPressed, bIsHighlighted, 
											bIsDisabled, bHasDropDownArrow, bIsSysButton);
	}

	COLORREF clrGradient1 = CBCGPDrawManager::PixelAlpha (pBar->m_clrBarBackground, bIsPressed ? 100 : bIsHighlighted ? 150 : 120);
	COLORREF clrGradient2 = CBCGPDrawManager::PixelAlpha (pBar->m_clrBarBackground, bIsPressed ? 95 : bIsHighlighted ? 200 : 180);
	COLORREF clrBorder = CBCGPDrawManager::PixelAlpha (pBar->m_clrBarBackground, 80);

	if (bIsSysButton && !bIsHighlighted)
	{
		return clrBorder;
	}

	const int nRoundSize = 3;

	CRgn rgn;
	rgn.CreateRoundRectRgn (rect.left, rect.top, rect.right, rect.bottom, nRoundSize, nRoundSize);

	pDC->SelectClipRgn (&rgn);

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, clrGradient1, clrGradient2, TRUE);

	pDC->SelectClipRgn (NULL);

	CPen pen (PS_SOLID, 1, clrBorder);
	CPen* pOldPen = pDC->SelectObject (&pen);
	CBrush* pOldBrush = (CBrush*) pDC->SelectStockObject (NULL_BRUSH);

	pDC->RoundRect (rect.left, rect.top, rect.right, rect.bottom, nRoundSize + 2, nRoundSize + 2);

	pDC->SelectObject (pOldPen);
	pDC->SelectObject (pOldBrush);

	return (bIsSysButton && !bIsPressed) ? RGB(0, 0, 0) : bIsDisabled ? pBar->m_clrBarText : (COLORREF)-1;
}
//**************************************************************************************
COLORREF CBCGPVisualManagerScenic::GetCaptionBarTextColor (CBCGPCaptionBar* pBar)
{
	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::GetCaptionBarTextColor (pBar);
	}

	return globalData.clrBarText;
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawCaptionBarInfoArea (CDC* pDC, CBCGPCaptionBar* pBar, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawCaptionBarInfoArea (pDC, pBar, rect);
	}
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, COLORREF& clrText)
{
	ASSERT_VALID(pDC);

	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnFillOutlookBarCaption (pDC, rectCaption, clrText);
		return;
	}

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rectCaption, m_clrBarGradientDark, RGB(255, 255, 255), TRUE);
	clrText = globalData.clrBarText;
}

#ifndef BCGP_EXCLUDE_RIBBON
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawRibbonPaletteButton (
					CDC* pDC, 
					CBCGPRibbonPaletteIcon* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawRibbonPaletteButton (pDC, pButton);
		return;
	}

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
//*****************************************************************************
COLORREF CBCGPVisualManagerScenic::OnDrawRibbonCategoryCaption (
				CDC* pDC, 
				CBCGPRibbonContextCaption* pContextCaption)
{
	if (!CanDrawImage () || pContextCaption->GetColor () == BCGPCategoryColor_None)
	{
		return CBCGPWinXPVisualManager::OnDrawRibbonCategoryCaption (pDC, pContextCaption);
	}

	XRibbonContextCategory& context = 
		m_ctrlRibbonContextCategory[pContextCaption->GetColor () - 1];

	CRect rect (pContextCaption->GetRect ());
	context.m_ctrlCaption.Draw (pDC, rect);

	int xTabRight = pContextCaption->GetRightTabX ();

	if (xTabRight > 0)
	{
		CRect rectTab (pContextCaption->GetParentRibbonBar ()->GetActiveCategory ()->GetTabRect ());
		rect.top = rectTab.top;
		rect.bottom = rectTab.bottom;
		rect.right = xTabRight;

		if (context.m_ctrlSeparator.IsValid ())
		{
			context.m_ctrlSeparator.Draw (pDC, rect);
		}
		else
		{
			m_ctrlRibbonContextSeparator.DrawFrame (pDC, rect);
		}
	}

	return context.m_clrCaptionText;
}
//*****************************************************************************
COLORREF CBCGPVisualManagerScenic::OnDrawRibbonStatusBarPane (CDC* pDC, CBCGPRibbonStatusBar* pBar,
					CBCGPRibbonStatusBarPane* pPane)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pBar);
	ASSERT_VALID (pPane);

	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::OnDrawRibbonStatusBarPane (pDC, pBar, pPane);
	}

	CRect rectPane = pPane->GetRect ();

	const BOOL bHighlighted = pPane->IsHighlighted () || pPane->IsFocused ();
	const BOOL bChecked     = pPane->IsChecked ();

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

	return (COLORREF)-1;
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawRibbonSliderZoomButton (
			CDC* pDC, CBCGPRibbonSlider* pSlider, 
			CRect rect, BOOL bIsZoomOut, 
			BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled)
{
	ASSERT_VALID (pDC);

	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawRibbonSliderZoomButton (pDC, pSlider, rect,
			bIsZoomOut, bIsHighlighted, bIsPressed, bIsDisabled);
		return;
	}

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
		globalData.GetRibbonImageScale () != 1. ? CBCGPToolBarImages::ImageAlignHorzStretch : CBCGPToolBarImages::ImageAlignHorzCenter, 
		globalData.GetRibbonImageScale () != 1. ? CBCGPToolBarImages::ImageAlignVertStretch : CBCGPToolBarImages::ImageAlignVertCenter,
		index);
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawDefaultRibbonImage (CDC* pDC, CRect rectImage,
					BOOL bIsDisabled/* = FALSE*/,
					BOOL bIsPressed/* = FALSE*/,
					BOOL bIsHighlighted/* = FALSE*/)
{
	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawDefaultRibbonImage(pDC, rectImage, bIsDisabled, 
			bIsPressed, bIsHighlighted);
		return;
	}

	m_RibbonBtnDefaultImage.DrawEx (pDC, rectImage, bIsDisabled ? 1 : 0,
		CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawRibbonMainButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawRibbonMainButton(pDC, pButton);
		return;
	}

	BOOL bIsHighlighted = pButton->IsHighlighted () || pButton->IsFocused ();
	BOOL bIsPressed = pButton->IsPressed () || pButton->IsDroppedDown ();

	if (pButton->IsDroppedDown ())
	{
		bIsPressed = TRUE;
		bIsHighlighted = TRUE;
	}

	CRect rect = pButton->GetRect ();

	int index = 0;
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

	if (m_clrMainButton != (COLORREF)-1)
	{
		if (!m_ctrlRibbonBtnMainColorized.IsValid())
		{
			m_ctrlRibbonBtnMain.CopyTo(m_ctrlRibbonBtnMainColorized);
			m_ctrlRibbonBtnMainColorized.GetImages().AddaptColors(RGB(0, 0, 192), m_clrMainButton);
		}

		m_ctrlRibbonBtnMainColorized.Draw (pDC, pButton->GetRect (), index);
	}
	else
	{
		m_ctrlRibbonBtnMain.Draw (pDC, pButton->GetRect (), index);
	}
}
//*****************************************************************************
void CBCGPVisualManagerScenic::SetMainButtonColor(COLORREF clr)
{
	CBCGPWinXPVisualManager::SetMainButtonColor(clr);

	m_ctrlRibbonBtnMainColorized.CleanUp();
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawCheckBoxEx (CDC *pDC, CRect rect, 
											int nState,
											BOOL bHighlighted, 
											BOOL bPressed,
											BOOL bEnabled)
{
	if (!CanDrawImage () && m_hThemeButton == NULL)
	{
		CBCGPVisualManager::OnDrawCheckBoxEx (pDC, rect, nState, bHighlighted, bPressed, bEnabled);
		return;
	}

	DrawCheckBox (pDC, rect, bHighlighted, nState, bEnabled, bPressed);
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawRadioButton (CDC *pDC, CRect rect, 
									 BOOL bOn,
									 BOOL bHighlighted, 
									 BOOL bPressed,
									 BOOL bEnabled)
{
	if (!CanDrawImage () && m_hThemeButton == NULL)
	{
		CBCGPVisualManager::OnDrawRadioButton (pDC, rect, bOn, bHighlighted, bPressed, bEnabled);
		return;
	}

	DrawRadioButton (pDC, rect, bHighlighted, bOn, bEnabled, bPressed);
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawRibbonButtonBorder (
					CDC* pDC, 
					CBCGPRibbonButton* pButton)
{
	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawRibbonButtonBorder (pDC, pButton);
		return;
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
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawRibbonLaunchButton (
					CDC* pDC,
					CBCGPRibbonLaunchButton* pButton,
					CBCGPRibbonPanel* pPanel)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);
	ASSERT_VALID (pPanel);

	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawRibbonLaunchButton(pDC, pButton, pPanel);
		return;
	}

	CRect rect (pButton->GetRect ());
	rect.right--;
	rect.bottom--;

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
		const double dblImageScale = globalData.GetRibbonImageScale ();

		if (dblImageScale == 1.)
		{
			m_RibbonBtnLaunchIcon.DrawEx (pDC, rect, index,
				CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter);
		}
		else
		{
			CSize sizeImage = m_RibbonBtnLaunchIcon.GetImageSize ();
			
			sizeImage.cx = (int)(.5 + dblImageScale * sizeImage.cx);
			sizeImage.cy = (int)(.5 + dblImageScale * sizeImage.cy);

			rect.left = rect.CenterPoint ().x - sizeImage.cx / 2;
			rect.right = rect.left + sizeImage.cx;

			rect.top = rect.CenterPoint ().y - sizeImage.cy / 2;
			rect.bottom = rect.top + sizeImage.cy;

			m_RibbonBtnLaunchIcon.DrawEx (pDC, rect, index, 
				CBCGPToolBarImages::ImageAlignHorzStretch, 
				CBCGPToolBarImages::ImageAlignVertStretch);
		}
	}
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawRibbonPinnedButtonBorder (
					CDC* pDC, 
					CBCGPRibbonButton* pButton)
{
	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawRibbonPinnedButtonBorder(pDC, pButton);
	}
}
//*****************************************************************************
COLORREF CBCGPVisualManagerScenic::OnFillRibbonButton(CDC* pDC, CBCGPRibbonButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::OnFillRibbonButton(pDC, pButton);
	}

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
		COLORREF color1 = m_clrRibbonEdit;
		if (bDisabled)
		{
			color1 = m_clrRibbonEditDisabled;
		}
		else if (bChecked || bHighlighted)
		{
			color1 = m_clrRibbonEditHighlighted;
		}

		COLORREF color2 = color1;

		rect.left = pButton->GetCommandRect ().left;

		{
			CBCGPDrawManager dm (*pDC);
			dm.FillGradient (rect, color1, color2, TRUE);
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

					if (context.m_ctrlBtnDefault.IsValid ())
					{
						pRenderer = &context.m_ctrlBtnDefault;
						pCache    = &context.m_cacheBtnDefault;
					}
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

				return globalData.clrBtnText;
			}
		}
	}
	else if ((!bDisabled && (bPressed || bChecked || bHighlighted)) || 
		    (bDisabled && bFocused))
	{
		if (!pButton->GetMenuRect().IsRectEmpty ()/* &&
			(pButton->IsHighlighted () || bChecked)*/)
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
		: m_clrRibbonBarBtnText;

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
//*****************************************************************************
COLORREF CBCGPVisualManagerScenic::OnFillRibbonPinnedButton(CDC* pDC, CBCGPRibbonButton* pButton, BOOL& bIsDarkPin)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::OnFillRibbonPinnedButton(pDC, pButton, bIsDarkPin);
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
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawRibbonDefaultPaneButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	BOOL bIsQATMode = pButton->IsQATMode ();

	if (GetCurrStyle() == VSScenic_Win8 && !bIsQATMode && !pButton->IsSearchResultMode ())
	{
		OnFillRibbonButton(pDC, pButton);
		OnDrawRibbonDefaultPaneButtonContext (pDC, pButton);
		return;
	}

	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawRibbonDefaultPaneButton (pDC, pButton);
		return;
	}

	OnFillRibbonButton (pDC, pButton);

	CRect rectFrame (pButton->GetRect ());

	if (!bIsQATMode && !pButton->IsSearchResultMode ())
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
	}

	OnDrawRibbonDefaultPaneButtonContext (pDC, pButton);
}
//***********************************************************************************
void CBCGPVisualManagerScenic::OnDrawRibbonMainPanelFrame (
					CDC* pDC, 
					CBCGPRibbonMainPanel* pPanel,
					CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawRibbonMainPanelFrame (pDC, 
					pPanel, rect);
		return;
	}

	ASSERT_VALID (pDC);

	LOGPEN pen;
	m_penSeparatorDark.GetLogPen (&pen);
 	pDC->Draw3dRect (rect, pen.lopnColor, pen.lopnColor);
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawRibbonRecentFilesFrame (
					CDC* pDC, 
					CBCGPRibbonMainPanel* pPanel,
					CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawRibbonRecentFilesFrame (pDC, 
					pPanel, rect);
		return;
	}

	ASSERT_VALID (pDC);

	rect.right += 2;
	pDC->FillRect (rect, &m_brBarBkgnd);

	CRect rectSeparator = rect;
	rectSeparator.right = rectSeparator.left + 2;

	DrawSeparator (pDC, rectSeparator, m_penSeparatorDark, m_penSeparatorLight, FALSE);
}

#endif
//*****************************************************************************
BOOL CBCGPVisualManagerScenic::OnNcPaint (CWnd* pWnd, const CObList& lstSysButtons, CRect rectRedraw)
{
	UNREFERENCED_PARAMETER(lstSysButtons);

#ifdef BCGP_EXCLUDE_RIBBON

	UNREFERENCED_PARAMETER(pWnd);
	UNREFERENCED_PARAMETER(rectRedraw);

#else

	if (globalData.DwmIsCompositionEnabled() && IsDWMCaptionSupported())
	{
		return FALSE;
	}

	if (pWnd->GetSafeHwnd() == NULL || m_hThemeWindow == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pWnd);

	CBCGPRibbonBar* pBar = GetRibbonBar(pWnd);
	BOOL bRibbonCaption  = pBar != NULL && pBar->IsWindowVisible() && pBar->IsReplaceFrameCaption();

	const DWORD dwStyle = pWnd->GetStyle();

	if (!bRibbonCaption)
	{
		return FALSE;
	}
	else
	{
		BOOL bMaximized = (dwStyle & WS_MAXIMIZE) == WS_MAXIMIZE;
        if (bMaximized)
		{
            return TRUE;
		}
	}

	CWindowDC dc(pWnd);

	if (dc.GetSafeHdc() != NULL)
	{
		CRgn rgn;
		if (!rectRedraw.IsRectEmpty())
		{
			rgn.CreateRectRgnIndirect(rectRedraw);
			dc.SelectClipRgn(&rgn);
		}

		CRect rtWindow;
		pWnd->GetWindowRect(rtWindow);
		pWnd->ScreenToClient(rtWindow);

		CRect rtClient;
		pWnd->GetClientRect(rtClient);

		rtClient.OffsetRect(-rtWindow.TopLeft());
		dc.ExcludeClipRect(rtClient);

		rtWindow.OffsetRect(-rtWindow.TopLeft());

		BOOL bActive = IsWindowActive(pWnd);

		// Modify bActive (if currently TRUE) for owner-drawn MDI child windows: draw child
		// frame active only if window is active MDI child and the MDI frame window is active.
		if (bActive && IsOwnerDrawCaption() && pWnd->IsKindOf(RUNTIME_CLASS(CMDIChildWnd)))
		{
			CMDIFrameWnd *pParent = ((CMDIChildWnd *)pWnd)->GetMDIFrame();
			if (pParent)
			{
				CMDIChildWnd *pActiveChild = pParent->MDIGetActive(NULL);
				if (pActiveChild)
				{
					bActive = ((pActiveChild->GetSafeHwnd() == pWnd->GetSafeHwnd()) && IsWindowActive(pParent));
				}
			}
		}

		CRect rectCaption(rtWindow);
		CSize szSysBorders(globalUtils.GetSystemBorders (dwStyle));

		rectCaption.bottom = rectCaption.top + szSysBorders.cy + pBar->GetCaptionHeight();

		(*m_pfDrawThemeBackground) (m_hThemeWindow, dc.GetSafeHdc(),
				/*WP_CAPTION*/1, bActive ? /*CS_ACTIVE*/1 : /*CS_INACTIVE*/2, &rectCaption, 0);

		rtWindow.top = rectCaption.bottom;
		dc.ExcludeClipRect(rectCaption);

		int framestate = bActive ? /*FS_ACTIVE*/1 : /*FS_INACTIVE*/2;

        CRect rectPart (rtWindow);
        rectPart.top = rectPart.bottom - szSysBorders.cy;
		(*m_pfDrawThemeBackground) (m_hThemeWindow, dc.GetSafeHdc(),
				/*WP_FRAMEBOTTOM*/9, framestate, &rectPart, 0);

        rectPart.bottom = rectPart.top;
        rectPart.top = rtWindow.top;
		rectPart.right = rectPart.left + szSysBorders.cx;
		(*m_pfDrawThemeBackground) (m_hThemeWindow, dc.GetSafeHdc(),
				/*WP_FRAMELEFT*/7, framestate, &rectPart, 0);

        rectPart.right = rtWindow.right;
        rectPart.left = rectPart.right - szSysBorders.cx;
		(*m_pfDrawThemeBackground) (m_hThemeWindow, dc.GetSafeHdc(),
				/*WP_FRAMERIGHT*/8, framestate, &rectPart, 0);

		dc.SelectClipRgn(NULL);

		return TRUE;
	}
#endif

	return FALSE;
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnActivateApp(CWnd* pWnd, BOOL bActive)
{
	ASSERT_VALID(pWnd);

	if (pWnd->GetSafeHwnd() == NULL)
	{
		return;
	}

	if (globalData.DwmIsCompositionEnabled() && IsDWMCaptionSupported())
	{
		return;
	}

#ifndef BCGP_EXCLUDE_RIBBON
	CBCGPRibbonBar* pBar = GetRibbonBar(pWnd);
	if (pBar->GetSafeHwnd() == NULL || !pBar->IsWindowVisible() || !pBar->IsReplaceFrameCaption())
#endif
	{
		return;
	}

	// but do not stay active if the window is disabled
	if (!pWnd->IsWindowEnabled())
	{
		bActive = FALSE;
	}

	BOOL bIsMDIFrame = FALSE;
	BOOL bWasActive = FALSE;

	// If the active state of an owner-draw MDI frame window changes, we need to
	// invalidate the MDI client area so the MDI child window captions are redrawn.
	if (IsOwnerDrawCaption())
	{
		bIsMDIFrame = pWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd));
		bWasActive = IsWindowActive(pWnd);
	}

	m_ActivateFlag[pWnd->GetSafeHwnd()] = bActive;
	pWnd->SendMessage(WM_NCPAINT, 0, 0);

	if (IsOwnerDrawCaption())
	{
		if (bIsMDIFrame && (bWasActive != bActive))
		{
			::RedrawWindow(((CMDIFrameWnd *)pWnd)->m_hWndMDIClient, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
		}
	}
}
//*****************************************************************************
BOOL CBCGPVisualManagerScenic::OnNcActivate(CWnd* pWnd, BOOL bActive)
{
	ASSERT_VALID(pWnd);

	if (pWnd->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}

	if (globalData.DwmIsCompositionEnabled() && IsDWMCaptionSupported())
	{
		return FALSE;
	}

#ifndef BCGP_EXCLUDE_RIBBON
	CBCGPRibbonBar* pBar = GetRibbonBar(pWnd);
	if (pBar->GetSafeHwnd() == NULL || !pBar->IsWindowVisible() || !pBar->IsReplaceFrameCaption())
#endif
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

	BOOL bIsMDIFrame = FALSE;
	BOOL bWasActive = FALSE;

	// If the active state of an owner-draw MDI frame window changes, we need to
	// invalidate the MDI client area so the MDI child window captions are redrawn.
	if (IsOwnerDrawCaption())
	{
		bIsMDIFrame = pWnd->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd));
		bWasActive = IsWindowActive(pWnd);
	}

	m_ActivateFlag[pWnd->GetSafeHwnd()] = bActive;
	pWnd->SendMessage(WM_NCPAINT, 0, 0);

	if (IsOwnerDrawCaption())
	{
		if (bIsMDIFrame && (bWasActive != bActive))
		{
			::RedrawWindow(((CMDIFrameWnd *)pWnd)->m_hWndMDIClient, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
		}
	}

	return TRUE;
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawComboDropButton (CDC* pDC, CRect rect,
											    BOOL bDisabled,
												BOOL bIsDropped,
												BOOL bIsHighlighted,
												CBCGPToolbarComboBoxButton* pButton)
{
	if (!CanDrawImage () || pButton == NULL || !pButton->IsRibbonButton ())
	{
		CBCGPWinXPVisualManager::OnDrawComboDropButton (pDC, rect, bDisabled, bIsDropped, bIsHighlighted, pButton);
		return;
	}

	rect.InflateRect (0, 1, 1, 1);

	int nIndex = 0;
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

	m_ctrlRibbonComboBoxBtn.Draw (pDC, rect, nIndex);

	rect.DeflateRect (0, 1, 1, 1);

	rect.bottom -= 2;

	CBCGPMenuImages::Draw (pDC, CBCGPMenuImages::IdArowDown, rect,
		bDisabled 
		? CBCGPMenuImages::ImageGray 
		: CBCGPMenuImages::ImageBlack);
}


#ifndef BCGP_EXCLUDE_RIBBON

COLORREF CBCGPVisualManagerScenic::GetRibbonEditBackgroundColor (
					CBCGPRibbonEditCtrl* pEdit,
					BOOL bIsHighlighted,
					BOOL bIsPaneHighlighted,
					BOOL bIsDisabled)
{
	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::GetRibbonEditBackgroundColor ( 
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
//*****************************************************************************
void CBCGPVisualManagerScenic::OnDrawRibbonPaletteBorder (
				CDC* pDC, 
				CBCGPRibbonPaletteButton* pButton, 
				CRect rectBorder)
{
	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawRibbonPaletteBorder (pDC, pButton, rectBorder);
		return;
	}

	rectBorder.right -= 5;

	ASSERT_VALID (pDC);

	CBCGPDrawManager dm (*pDC);
	dm.DrawRect (rectBorder, m_clrRibbonEdit, m_clrRibbonEditBorder);
}

#endif
//*****************************************************************************
COLORREF CBCGPVisualManagerScenic::OnDrawMenuLabel (CDC* pDC, CRect rect)
{
	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::OnDrawMenuLabel (pDC, rect);
	}

	ASSERT_VALID (pDC);

	pDC->FillRect (rect, &m_brBarBkgnd);

	CRect rectSeparator = rect;
	rectSeparator.top = rectSeparator.bottom - 2;

	DrawSeparator (pDC, rectSeparator, m_penSeparatorDark, m_penSeparatorLight, TRUE);

	return globalData.clrBarText;
}
//***********************************************************************************
void CBCGPVisualManagerScenic::OnDrawMenuResizeBar (CDC* pDC, CRect rect, int nResizeFlags)
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
		CBCGPWinXPVisualManager::OnDrawMenuResizeBar (pDC, rect, nResizeFlags);
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
//*************************************************************************************
void CBCGPVisualManagerScenic::OnFillPopupMenuBackground (CDC* pDC, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnFillPopupMenuBackground (pDC, rect);
		return;
	}

	pDC->FillRect (rect, &m_brMenuLight);
}
//*************************************************************************************
COLORREF CBCGPVisualManagerScenic::OnFillListBoxItem (CDC* pDC, CBCGPListBox* pListBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected)
{
	if (!CanDrawImage () || rect.Height() < m_ctrlRibbonBtn[0].GetImages().GetImageSize().cy)
	{
		return CBCGPWinXPVisualManager::OnFillListBoxItem (pDC, pListBox, nItem, rect, bIsHighlihted, bIsSelected);
	}

	rect.DeflateRect (0, 1);

	int nIndex = 0;

	if (bIsSelected)
	{
		nIndex = bIsHighlihted ? 1 : 2;
	}

	m_ctrlRibbonBtn [0].Draw (pDC, rect, nIndex);
	return globalData.clrBarText;
}
//********************************************************************************
void CBCGPVisualManagerScenic::OnDrawEditClearButton(CDC* pDC, CBCGPButton* pButton, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnDrawEditClearButton(pDC, pButton, rect);
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
//********************************************************************************
BOOL CBCGPVisualManagerScenic::OnDrawCalculatorButton (CDC* pDC, 
	CRect rect, CBCGPToolbarButton* pButton, CBCGPVisualManager::BCGBUTTON_STATE state, int cmd, CBCGPCalculator* pCalculator)
{
	if (!CanDrawImage () || pCalculator->IsDialogControl())
	{
		return CBCGPWinXPVisualManager::OnDrawCalculatorButton (pDC, rect, pButton, state, cmd, pCalculator);
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

	m_ctrlRibbonBtnGroup_S.Draw (pDC, rect, nIndex);
	return TRUE;
}
//********************************************************************************
BOOL CBCGPVisualManagerScenic::OnDrawCalculatorDisplay (CDC* pDC, CRect rect, 
												  const CString& strText, BOOL bMem,
												  CBCGPCalculator* pCalculator)
{
	if (!CanDrawImage () || pCalculator->IsDialogControl())
	{
		return CBCGPWinXPVisualManager::OnDrawCalculatorDisplay(pDC, rect, strText, bMem, pCalculator);
	}

	CBCGPDrawManager dm (*pDC);
	dm.FillGradient (rect, m_clrRibbonEditHighlighted, m_clrRibbonEdit, TRUE);

	pDC->Draw3dRect (&rect, m_clrRibbonEditBorder, m_clrRibbonEditBorder);

	return TRUE;
}
//***********************************************************************************
COLORREF CBCGPVisualManagerScenic::GetCalculatorButtonTextColor (BOOL bIsUserCommand, int nCmd /* CBCGPCalculator::CalculatorCommands */)
{
	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::GetCalculatorButtonTextColor(bIsUserCommand, nCmd);
	}

	return m_clrRibbonPanelCaptionText;
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnFillMenuImageRect (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state)
{
	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnFillMenuImageRect (pDC, pButton, rect, state);
		return;
	}

	OnFillButtonInterior (pDC, pButton, rect, state);
}
//*****************************************************************************
void CBCGPVisualManagerScenic::OnFillButtonInterior (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state)
{
	if (!CanDrawImage ())
	{
		CBCGPWinXPVisualManager::OnFillButtonInterior (pDC, pButton, rect, state);
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
		BOOL bChecked  = (pButton->m_nStyle & TBBS_CHECKED ) == TBBS_CHECKED;

		CBCGPBaseControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar, pButton->GetParentWnd ());

		CBCGPToolbarMenuButton* pMenuButton = 
			DYNAMIC_DOWNCAST (CBCGPToolbarMenuButton, pButton);
		if (pMenuButton != NULL && pBar != NULL)
		{
			if (!pBar->IsKindOf (RUNTIME_CLASS (CBCGPMenuBar)) &&
				pBar->IsKindOf (RUNTIME_CLASS (CBCGPPopupMenuBar)))
			{
				if (bChecked)
				{
					pRenderer = &m_ctrlMenuItemBack;

					if (bDisabled)
					{
						index = 1;
					}

					rect.InflateRect (0, 0, 0, 1);
				}
				else if (state == ButtonsIsPressed || state == ButtonsIsHighlighted)
				{
					pRenderer = &m_ctrlMenuHighlighted[bDisabled ? 1 : 0];
				}
				else
				{
					return;
				}
			}
		}

		if (pRenderer != NULL)
		{
			pRenderer->Draw (pDC, rect, index);
			return;
		}
	}

	CBCGPWinXPVisualManager::OnFillButtonInterior (pDC, pButton, rect, state);
}
//*******************************************************************************
BOOL CBCGPVisualManagerScenic::GetToolTipParams (CBCGPToolTipParams& params, 
											   UINT /*nType*/ /*= (UINT)(-1)*/)
{
	if (!CanDrawImage () ||
		!m_bToolTipParams)
	{
		return CBCGPWinXPVisualManager::GetToolTipParams (params);
	}

	params = m_ToolTipParams;
	return TRUE;
}
//*******************************************************************************
COLORREF CBCGPVisualManagerScenic::GetEditBackSidebarColor(CBCGPEditCtrl* pEdit)
{
	if (!CanDrawImage ())
	{
		return CBCGPWinXPVisualManager::GetEditBackSidebarColor(pEdit);
	}

	return globalData.clrBarLight;
}
//**********************************************************************************
void CBCGPVisualManagerScenic::GetTabFrameColors (const CBCGPBaseTabWnd* pTabWnd,
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

	CBCGPWinXPVisualManager::GetTabFrameColors (pTabWnd,
				   clrDark,
				   clrBlack,
				   clrHighlight,
				   clrFace,
				   clrDarkShadow,
				   clrLight,
				   pbrFace,
				   pbrBlack);

	if (!CanDrawImage () || pTabWnd->IsDialogControl ())
	{
		return;
	}

	if (m_brTabFace.GetSafeHandle() != NULL)
	{
		clrFace = globalData.clrBarFace;
		pbrFace = &m_brTabFace;
	}
}
//*********************************************************************************
void CBCGPVisualManagerScenic::OnFillTab (CDC* pDC, CRect rectFill, CBrush* pbrFill,
									 int iTab, BOOL bIsActive, 
									 const CBCGPBaseTabWnd* pTabWnd)
{
	ASSERT_VALID (pTabWnd);

	if (pTabWnd->IsFlatTab () || globalData.m_nBitsPerPixel <= 8 || 
		globalData.IsHighContastMode () || pTabWnd->IsDialogControl () ||
		pTabWnd->GetTabBkColor (iTab) == (COLORREF)-1 ||
		pTabWnd->IsPointerStyle())
	{
		CBCGPWinXPVisualManager::OnFillTab (pDC, rectFill, pbrFill,
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
//*********************************************************************************
void CBCGPVisualManagerScenic::OnDrawEditCtrlResizeBox (CDC* pDC, CBCGPEditCtrl* /*pEdit*/, CRect rect)
{
	pDC->FillRect (rect, &globalData.brBtnFace);
}

#ifndef BCGP_EXCLUDE_PROP_LIST

COLORREF CBCGPVisualManagerScenic::GetPropListGroupTextColor (CBCGPPropList* pPropList)
{
	ASSERT_VALID (pPropList);

	if (m_hThemeToolBar == NULL || globalData.m_nBitsPerPixel <= 8)
	{
		return pPropList->DrawControlBarColors () ? globalData.clrBarText : globalData.clrBtnShadow;
	}

	return CBCGPWinXPVisualManager::GetPropListGroupTextColor (pPropList);
}

#endif

void CBCGPVisualManagerScenic::FillRebarPane (CDC* pDC, CBCGPBaseControlBar* pBar, CRect rectClient)
{
	if (GetCurrStyle() == VSScenic_Win8 && !globalData.bIsWindows8)
	{
		CBrush br(RGB(246, 246, 246));
		pDC->FillRect(rectClient, &br);
		return;
	}

	CBCGPWinXPVisualManager::FillRebarPane(pDC, pBar, rectClient);
}
