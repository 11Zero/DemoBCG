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
// BCGPVisualManager2010.cpp: implementation of the CBCGPVisualManager2010 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPVisualManager2010.h"
#include "bcgcbpro.h"
#include "bcgglobals.h"
#include "BCGCBProVer.h"
#include "BCGPDrawManager.h"
#include "BCGPTagManager.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPGlobalUtils.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPFrameWnd.h"
#include "CustomizeButton.h"
#include "BCGPToolbarComboBoxButton.h"
#include "BCGPCaptionBar.h"
#include "bcgpstyle.h"
#include "BCGPCaptionBar.h"
#include "BCGPStatic.h"
#include "BCGPListBox.h"
#include "BCGPCalculator.h"
#include "BCGPCalendarBar.h"
#include "BCGPColorBar.h"
#include "BCGPOutlookWnd.h"

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
#include "BCGPRibbonBackstageView.h"
#endif

#ifndef BCGP_EXCLUDE_PLANNER
#include "BCGPPlannerViewDay.h"
#include "BCGPPlannerViewMulti.h"
#include "BCGPPlannerViewMonth.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const CBCGPVisualManager2010::Style c_StyleDefault = 
	CBCGPVisualManager2010::VS2010_Silver;

CBCGPVisualManager2010::Style CBCGPVisualManager2010::m_Style = c_StyleDefault;

IMPLEMENT_DYNCREATE(CBCGPVisualManager2010, CBCGPVisualManager2007)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPVisualManager2010::CBCGPVisualManager2010()
{
	m_bUseScenicColors = FALSE;
}

CBCGPVisualManager2010::~CBCGPVisualManager2010()
{
	CBCGPVisualManager2007::Style style2007 = CBCGPVisualManager2007::m_Style;
	CBCGPVisualManager2010::Style style2010 = CBCGPVisualManager2010::m_Style;
	CBCGPVisualManager2010::CleanStyle ();
	CBCGPVisualManager2010::m_Style = style2010;
	CBCGPVisualManager2007::m_Style = style2007;
}
//*****************************************************************************
CString CBCGPVisualManager2010::GetStyleResourceID(CBCGPVisualManager2010::Style style)
{
	CString strResID (_T("IDX_BCG_OFFICE2010_STYLE"));

#if !defined _AFXDLL || defined _BCGCBPRO_STATIC_

	CString strStylePrefix;

	switch (style)
	{
	case VS2010_Blue:
		strStylePrefix = _T("BLUE_");
		break;

	case VS2010_Black:
		strStylePrefix = _T("BLACK_");
		break;

	case VS2010_Silver:
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
}
//*****************************************************************************
BOOL CBCGPVisualManager2010::SetStyle (CBCGPVisualManager2010::Style style, LPCTSTR lpszPath)
{
	if (CBCGPVisualManager::GetInstance () == NULL ||
		!CBCGPVisualManager::GetInstance ()->IsKindOf (RUNTIME_CLASS(CBCGPVisualManager2010)))
	{
		CBCGPVisualManager2010::m_Style = style;
		return TRUE;
	}

	if (CBCGPVisualManager2010::m_Style == style && m_hinstRes > (HINSTANCE) 32)
	{
		return TRUE;
	}

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_

	CString strTheme;

	switch (style)
	{
	case VS2010_Blue:
		strTheme = _T("Blue");
		break;

	case VS2010_Black:
		strTheme = _T("Black");
		break;

	case VS2010_Silver:
		strTheme = _T("White");
		break;

	default:
		ASSERT (FALSE);
		return FALSE;
	}

	CString strVer;
	strVer.Format (_T("%d%d"), _BCGCBPRO_VERSION_MAJOR, _BCGCBPRO_VERSION_MINOR);

	CString strStyleDLLName = _T("BCGPStyle2010") + strTheme + strVer + _T(".dll");

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

	CBCGPVisualManager2010::Style oldStyle = m_Style;

	CBCGPVisualManager2010::CleanStyle ();

	HINSTANCE hinstRes = LoadLibrary (strStyleDLLPath);

	if (hinstRes <= (HINSTANCE) 32)
	{
		m_Style = oldStyle;

		TRACE(_T("Cannot load Style DLL: %s\r\n"), strStyleDLLPath);
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPVisualManager2010::m_Style = style;

	CBCGPVisualManager2010::SetResourceHandle (hinstRes);
	m_bAutoFreeRes = TRUE;

#else

	UNREFERENCED_PARAMETER (lpszPath);

	CString strStyle (CBCGPVisualManager2010::GetStyleResourceID (style));
	HINSTANCE hinstRes = AfxFindResourceHandle (strStyle, RT_BCG_STYLE_XML);

	if (::FindResource(hinstRes, strStyle, RT_BCG_STYLE_XML) == NULL)
	{
		TRACE(_T("Cannot load Style: %s\r\n"), strStyle);
		ASSERT (FALSE);
		return FALSE;
	}

	CBCGPVisualManager2010::CleanStyle ();

	CBCGPVisualManager2010::m_Style = style;

	CBCGPVisualManager2010::SetResourceHandle (hinstRes);

#endif

	return TRUE;
}
//*****************************************************************************
CBCGPVisualManager2010::Style CBCGPVisualManager2010::GetStyle ()
{
	return CBCGPVisualManager2010::m_Style;
}
//*****************************************************************************
void CBCGPVisualManager2010::CleanStyle ()
{
	CBCGPVisualManager2007::CleanStyle ();

	CBCGPVisualManager2010::m_Style = c_StyleDefault;
}
//*****************************************************************************
void CBCGPVisualManager2010::SetResourceHandle (HINSTANCE hinstRes)
{
	m_bAutoFreeRes = FALSE;

	if (m_hinstRes != hinstRes)
	{
		m_hinstRes = hinstRes;

		if (CBCGPVisualManager::GetInstance ()->IsKindOf (
			RUNTIME_CLASS (CBCGPVisualManager2010)))
		{
			CBCGPVisualManager::GetInstance ()->OnUpdateSystemColors ();
		}
	}
}
//*****************************************************************************
void CBCGPVisualManager2010::CleanUp ()
{
	m_ctrlRibbonCategoryTabFrame.CleanUp ();
	m_ctrlRibbonPanelBack.CleanUp ();

	m_RibbonBtnMinimizeIcon.Clear ();
	m_ctrlRibbonBtnGroupSeparator.CleanUp ();

	m_ctrlRibbonBsPanelBack.CleanUp ();
	m_ctrlRibbonBtnBsNormal.CleanUp ();
	m_ctrlRibbonBtnBsView.CleanUp ();
	m_RibbonBtnBsViewIcon.Clear ();
	m_RibbonBtnBsListIcon.Clear ();
	m_RibbonBsSeparatorHorz.Clear ();
	m_RibbonBsSeparatorVert.Clear ();
	m_ctrlRibbonBsBtnPush.CleanUp ();

	m_ctrlOutlookSplitter.CleanUp ();

	for (int i = 0; i < BCGPRibbonCategoryColorCount; i++)
	{
		m_ctrlRibbonContextCategory2[i].CleanUp ();
	}

	m_cacheRibbonPanelBack.Clear ();

	m_ctrlRibbonBtnMainColorized.CleanUp();
	m_ctrlRibbonBtnBsNormalColorized.CleanUp();
	m_ctrlRibbonBtnBsViewColorized.CleanUp();

	CBCGPVisualManager2007::CleanUp ();
}
//*****************************************************************************
void CBCGPVisualManager2010::OnUpdateSystemColors ()
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
		CBCGPVisualManager2010::SetStyle (CBCGPVisualManager2010::m_Style);
	}

	if (m_hinstRes != NULL)
	{
		hinstResOld = AfxGetResourceHandle ();
		AfxSetResourceHandle (m_hinstRes);
	}

	CBCGPTagManager tm;
	CBCGPTagManager::SetBaseColor (m_clrBase, m_clrTarget);

	if (!tm.LoadFromResource (CBCGPVisualManager2010::GetStyleResourceID (CBCGPVisualManager2010::m_Style), RT_BCG_STYLE_XML))
	{
#if !defined _AFXDLL || defined _BCGCBPRO_STATIC_
		TRACE(_T("\r\nImportant: to enable Office 2010 look in static link, you need:\r\n"));
		TRACE(_T("1. Open \"Resource Includes\" dialog and add resource files:\r\n"));
		TRACE(_T("<BCGCBPro-Path>\\styles\\BCGPStyle2010Blue.rc\r\n"));
		TRACE(_T("<BCGCBPro-Path>\\styles\\BCGPStyle2010White.rc\r\n"));
		TRACE(_T("<BCGCBPro-Path>\\styles\\BCGPStyle2010Black.rc\r\n"));
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

	m_nType = 30;

	CString strStyle;
	if (tm.ExcludeTag (_T("STYLE"), strStyle))
	{
		tm.SetBuffer (strStyle);
		m_bLoaded = ParseStyleXML (tm);
	}

	m_brCtrlBackground.DeleteObject();
	m_brCtrlBackground.CreateSolidBrush(m_clrToolBarGradientLight);

	if (hinstResOld != NULL)
	{
		AfxSetResourceHandle (hinstResOld);
	}

	CBCGPTagManager::SetBaseColor ((COLORREF)-1, (COLORREF)-1);
}
//*****************************************************************************
BOOL CBCGPVisualManager2010::ParseStyleXMLVersion(const CString& strItem)
{
	if (strItem.IsEmpty ())
	{
		return FALSE;
	}

	BOOL bLoaded = FALSE;
	int nVersion = 0;

	CBCGPTagManager tmItem (strItem);

	tmItem.ReadInt (_T("NUMBER"), nVersion);

	if (nVersion == 2010)
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
BOOL CBCGPVisualManager2010::ParseStyleXMLRibbon(const CString& strItem)
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
		tmCategory.ReadControlRenderer (_T("TAB_FRAME"), m_ctrlRibbonCategoryTabFrame, MakeResourceID(_T("RB_CAT_TAB_FRAME")));

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

				tmBack.ReadControlRenderer (_T("FULL"), m_ctrlRibbonPanelBack, MakeResourceID(_T("RB_PNL_BACK")));
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

				tmButtons.ReadToolBarImages (_T("BUTTON_MINIMIZE_ICON"), m_RibbonBtnMinimizeIcon, MakeResourceID(_T("RB_BTN_MINIMIZE_ICON")));
				tmButtons.ReadControlRenderer (_T("BUTTON_GROUP_SEPARATOR"), m_ctrlRibbonBtnGroupSeparator, MakeResourceID(_T("RB_BTN_GRP_SEPARATOR")));
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
			CBCGPControlRendererParams prSeparator;
			CBCGPControlRendererParams prPanelBack;
			CBCGPControlRendererParams prTabGlow;
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
				tmTab.ReadControlRendererParams(_T("GLOW"), prTabGlow);
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
				XRibbonContextCategory& cat   = m_ctrlRibbonContextCategory[i];
				XRibbonContextCategory2& cat2 = m_ctrlRibbonContextCategory2[i];

				prDefault.m_strBmpResID   = strID[i] + _T("BTN_DEF");
				prTab.m_strBmpResID       = strID[i] + _T("CAT_TAB");
				prTabGlow.m_strBmpResID   = strID[i] + _T("CAT_TAB_GLOW");
				prCaption.m_strBmpResID   = strID[i] + _T("CAT_CAPTION");
				prBack.m_strBmpResID      = strID[i] + _T("CAT_BACK");
				prSeparator.m_strBmpResID = strID[i] + _T("CAT_SEPARATOR");
				prPanelBack.m_strBmpResID = strID[i] + _T("CAT_PANEL");

				cat.m_ctrlBtnDefault.Create (prDefault);
				cat.m_ctrlCaption.Create (prCaption);
				cat.m_ctrlTab.Create (prTab);
				cat.m_ctrlBack.Create (prBack);
				cat2.m_ctrlSeparator.Create (prSeparator);
				cat2.m_ctrlPanelBack.Create (prPanelBack);
				cat2.m_ctrlTabGlow.Create (prTabGlow);
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

	m_clrRibbonBsTextNormal = m_clrToolBarBtnText;
	m_clrRibbonBsTextHighlighted = m_clrToolBarBtnTextHighlighted;
	m_clrRibbonBsTextActive = m_clrRibbonBsTextHighlighted;
	m_clrRibbonBsTextDisabled = m_clrToolBarBtnTextDisabled;

	if (tmItem.ExcludeTag (_T("BACKSTAGE"), str))
	{
		CBCGPTagManager tmBs (str);

		tmBs.ReadControlRenderer (_T("PANEL_BACK"), m_ctrlRibbonBsPanelBack, MakeResourceID(_T("RB_BS_PNL_BACK")));
		tmBs.ReadControlRenderer (_T("BUTTON_NORMAL"), m_ctrlRibbonBtnBsNormal, MakeResourceID(_T("RB_BS_BTN_NORMAL")));
		tmBs.ReadControlRenderer (_T("BUTTON_VIEW"), m_ctrlRibbonBtnBsView, MakeResourceID(_T("RB_BS_BTN_VIEW")));
		tmBs.ReadToolBarImages (_T("BUTTON_VIEW_ICON"), m_RibbonBtnBsViewIcon, MakeResourceID(_T("RB_BS_BTN_VIEW_ICON")));
		tmBs.ReadToolBarImages (_T("BUTTON_LIST_ICON"), m_RibbonBtnBsListIcon, MakeResourceID(_T("RB_BS_BTN_LIST_ICON")));
		tmBs.ReadToolBarImages (_T("SEPARATOR_HORZ"), m_RibbonBsSeparatorHorz, MakeResourceID(_T("RB_BS_SEP_H")));
		tmBs.ReadToolBarImages (_T("SEPARATOR_VERT"), m_RibbonBsSeparatorVert, MakeResourceID(_T("RB_BS_SEP_V")));
		tmBs.ReadControlRenderer (_T("BUTTON_PUSH"), m_ctrlRibbonBsBtnPush, MakeResourceID(_T("RB_BS_BTN_PUSH")));

		if (tmBs.ReadColor (_T("TextNormal"), m_clrRibbonBsTextNormal))
		{
			m_clrRibbonBsTextHighlighted = m_clrRibbonBsTextNormal;
		}
		tmBs.ReadColor (_T("TextHighlighted"), m_clrRibbonBsTextHighlighted);
		tmBs.ReadColor (_T("TextActive"), m_clrRibbonBsTextActive);
		tmBs.ReadColor (_T("TextDisabled"), m_clrRibbonBsTextDisabled);
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
void CBCGPVisualManager2010::DrawSeparator2 (CDC* pDC, const CRect& rect, BOOL bHorz)
{
	if (!CanDrawImage () || !bHorz)
	{
		CBCGPVisualManager2007::DrawSeparator (pDC, rect, bHorz);
		return;
	}

    CRect rect1 (rect);

    rect1.top += rect.Height () / 2 - 1;
    rect1.bottom = rect1.top + 1;

	LOGPEN logpen;
	m_penSeparator2.GetLogPen (&logpen);

	if (CBCGPToolBarImages::m_bIsDrawOnGlass)
	{
		CBCGPDrawManager dm (*pDC);
		dm.DrawRect (rect, logpen.lopnColor, (COLORREF)-1);
	}
	else
	{
		CBrush br;
		br.CreateSolidBrush (logpen.lopnColor);

		pDC->FillRect (rect, &br);
	}

	int nWidth = m_RibbonBsSeparatorHorz.GetImageSize ().cx;
	int nCount = rect.Width () / nWidth;
	CRect rectPart (rect1);
	rectPart.right = rectPart.left + nWidth;
	for (int i = 0; i < nCount; i++)
	{
		m_RibbonBsSeparatorHorz.DrawEx (pDC, rectPart, 0, CBCGPToolBarImages::ImageAlignHorzLeft, CBCGPToolBarImages::ImageAlignVertCenter);
		rectPart.OffsetRect (nWidth, 0);
	}

	if ((nCount * nWidth) < rect.Width ())
	{
		rectPart.right = rect.right;
		rectPart.left = rect.left + nCount * nWidth;
		m_RibbonBsSeparatorHorz.DrawEx (pDC, rectPart, 0, CBCGPToolBarImages::ImageAlignHorzLeft, CBCGPToolBarImages::ImageAlignVertCenter);
	}
}

#ifndef BCGP_EXCLUDE_RIBBON

COLORREF CBCGPVisualManager2010::GetRibbonEditBackgroundColor (
					CBCGPRibbonEditCtrl* pEdit,
					BOOL bIsHighlighted,
					BOOL bIsPaneHighlighted,
					BOOL bIsDisabled)
{
	if (CanDrawImage () && pEdit != NULL && pEdit->GetOwnerRibbonEdit ().IsFloatyMode())
	{
		return bIsDisabled ? globalData.clrBtnFace : globalData.clrWindow;
	}

	return CBCGPVisualManager2007::GetRibbonEditBackgroundColor ( 
			pEdit, bIsHighlighted, bIsPaneHighlighted, bIsDisabled);
}
//********************************************************************************
int CBCGPVisualManager2010::GetRibbonPanelMargin(CBCGPRibbonCategory* pCategory)
{
	if (!CanDrawImage())
	{
		return CBCGPVisualManager2007::GetRibbonPanelMargin(pCategory);
	}

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
				return 2;
			}
		}
	}

	return 0;
}
//********************************************************************************
int CBCGPVisualManager2010::GetRibbonPanelMarginTop() const
{
	return CBCGPVisualManager2007::GetRibbonPanelMarginTop();
}

#endif

void CBCGPVisualManager2010::OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnFillBarBackground (pDC, pBar, rectClient, rectClip, bNCArea);
		return;
	}

	if (pBar->IsKindOf (RUNTIME_CLASS (CBCGPCaptionBar)))
	{
		CBCGPCaptionBar* pCaptionBar = (CBCGPCaptionBar*) pBar;

		if (pCaptionBar->m_clrBarBackground != -1)
		{
			COLORREF clrGradient = CBCGPDrawManager::PixelAlpha (pCaptionBar->m_clrBarBackground, 150);
			COLORREF clrBorder = CBCGPDrawManager::PixelAlpha (pCaptionBar->m_clrBarBackground, 80);

			CBCGPDrawManager dm (*pDC);
			dm.FillGradient (rectClient, pCaptionBar->m_clrBarBackground, clrGradient, TRUE);

			pDC->Draw3dRect(rectClient, clrBorder, clrBorder);
			return;
		}
	}

	CBCGPVisualManager2007::OnFillBarBackground (pDC, pBar, rectClient, rectClip, bNCArea);
}
//*****************************************************************************
void CBCGPVisualManager2010::OnFillButtonInterior (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state)
{
	if (!CanDrawImage () || pButton == NULL || DYNAMIC_DOWNCAST(CBCGPOutlookBarToolBar, pButton->GetParentWnd()) == NULL ||
		!m_ctrlOutlookWndPageBtn.IsValid ())
	{
		CBCGPVisualManager2007::OnFillButtonInterior (pDC, pButton, rect, state);
		return;
	}

	BOOL bChecked = (pButton->m_nStyle & TBBS_CHECKED ) == TBBS_CHECKED;
	BOOL bPressed  = (pButton->m_nStyle & TBBS_PRESSED ) == TBBS_PRESSED;
	BOOL bHighlighted  = (state == ButtonsIsHighlighted);

	if (bHighlighted || bChecked)
	{
		int index = bChecked ? 3 : bPressed ? 4 : 1;

		rect.DeflateRect(0, 2);
		m_ctrlOutlookWndPageBtn.Draw (pDC, rect, index);
	}
}

#ifndef BCGP_EXCLUDE_RIBBON

//*****************************************************************************
void CBCGPVisualManager2010::OnDrawRibbonMainButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawRibbonMainButton(pDC, pButton);
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
			m_RibbonBtnMain.CopyTo(m_ctrlRibbonBtnMainColorized);
			m_ctrlRibbonBtnMainColorized.GetImages().AddaptColors(RGB(0, 0, 192), m_clrMainButton);
		}

		m_ctrlRibbonBtnMainColorized.Draw (pDC, pButton->GetRect (), index);
	}
	else
	{
		m_RibbonBtnMain.Draw (pDC, pButton->GetRect (), index);
	}
}
//*****************************************************************************
void CBCGPVisualManager2010::SetMainButtonColor(COLORREF clr)
{
	CBCGPVisualManager2007::SetMainButtonColor(clr);

	m_ctrlRibbonBtnMainColorized.CleanUp();
	m_ctrlRibbonBtnBsNormalColorized.CleanUp();
	m_ctrlRibbonBtnBsViewColorized.CleanUp();
}
//*****************************************************************************
void CBCGPVisualManager2010::OnDrawRibbonCaption (CDC* pDC, CBCGPRibbonBar* pBar,
											  CRect rectCaption, CRect rectText)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pBar);

	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawRibbonCaption(pDC, pBar, rectCaption, rectText);
		return;
	}

	CWnd* pWnd = pBar->GetParent ();
	ASSERT_VALID (pWnd);

	const DWORD dwStyle   = pWnd->GetStyle ();
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

			m_ctrlMainBorderCaption.Draw (pDC, rectCaption1, bActive ? 0 : 1);
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
			rectQAFrame.InflateRect (1, 1, 1, 1);

			const CBCGPControlRendererParams& params = m_ctrlRibbonCaptionQA.GetParams ();

			if (rectQAFrame.Height () < params.m_rectImage.Height ())
			{
				rectQAFrame.top = rectQAFrame.bottom - params.m_rectImage.Height ();
			}

			if (bGlass && pWnd->IsZoomed ())
			{
				m_ctrlRibbonCaptionQA_Glass.Draw (pDC, rectQAFrame, bActive ? 0 : 1);
			}

			rectQAFrame.right = pBar->GetQATCommandsLocation ().right + 1;
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
		bIsRTL, m_bNcTextCenter, bGlass, (pWnd->IsZoomed () && !globalData.bIsWindows7) ? 0 : 10, 
		(pWnd->IsZoomed () && !globalData.bIsWindows7) ? RGB (255, 255, 255) : (COLORREF)-1);

	pDC->SelectObject (pOldFont);
}
//*****************************************************************************
COLORREF CBCGPVisualManager2010::OnDrawRibbonCategoryCaption (
				CDC* pDC, 
				CBCGPRibbonContextCaption* pContextCaption)
{
	COLORREF clr = CBCGPVisualManager2007::OnDrawRibbonCategoryCaption (pDC, pContextCaption);

	if (CanDrawImage () && pContextCaption->GetColor () != BCGPCategoryColor_None)
	{
		XRibbonContextCategory2& context2 = 
			m_ctrlRibbonContextCategory2[pContextCaption->GetColor () - 1];

		CRect rect (pContextCaption->GetRect ());
		int xTabRight = pContextCaption->GetRightTabX ();

		if (xTabRight > 0 && pContextCaption->GetParentRibbonBar() != NULL && pContextCaption->GetParentRibbonBar ()->GetActiveCategory () != NULL)
		{
			CRect rectTab (pContextCaption->GetParentRibbonBar ()->GetActiveCategory ()->GetTabRect ());
			rect.top = rectTab.top;
			rect.bottom = rectTab.bottom;
			rect.right = xTabRight;

			context2.m_ctrlSeparator.Draw (pDC, rect);
		}
	}

	return clr;
}
//***********************************************************************************
void CBCGPVisualManager2010::OnDrawRibbonCategoryCaptionText (
					CDC* pDC, CBCGPRibbonContextCaption* pContextCaption, 
					CString& strText, CRect rectText, BOOL bIsOnGlass, BOOL bIsZoomed)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawRibbonCategoryCaptionText(pDC, pContextCaption, strText, rectText, bIsOnGlass, bIsZoomed);
		return;
	}

	ASSERT_VALID (pDC);

	const UINT uiDTFlags = DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX;

	rectText.OffsetRect(1, 1);

	for (int iStep = 0; iStep < 2; iStep++)
	{
		const COLORREF clr = iStep == 0 ? RGB(100, 100, 100) : RGB(255, 255, 255);

		if (bIsOnGlass)
		{
			DrawTextOnGlass(pDC, strText, rectText, uiDTFlags, 0, clr);
		}
		else
		{
			COLORREF clrOld = pDC->SetTextColor(clr);
			pDC->DrawText(strText, rectText, uiDTFlags);
			pDC->SetTextColor(clrOld);
		}
	
		rectText.OffsetRect(-1, -1);
	}
}
//*****************************************************************************
void CBCGPVisualManager2010::OnDrawRibbonCategory (
		CDC* pDC, 
		CBCGPRibbonCategory* pCategory,
		CRect rectCategory)
{
	if (CanDrawImage ())
	{
		if (pCategory->GetParentMenuBar () != NULL &&
			pCategory->GetParentMenuBar ()->IsKindOf (RUNTIME_CLASS(CBCGPRibbonBackstageView)))
		{
			return;
		}
	}

	CBCGPVisualManager2007::OnDrawRibbonCategory (pDC, pCategory, rectCategory);
}
//*****************************************************************************
COLORREF CBCGPVisualManager2010::OnFillRibbonButton(CDC* pDC, CBCGPRibbonButton* pButton)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	if (!CanDrawImage () || pButton->IsKindOf (RUNTIME_CLASS(CBCGPRibbonEdit)))
	{
		return CBCGPVisualManager2007::OnFillRibbonButton(pDC, pButton);
	}

	CRect rect (pButton->GetRect ());
	const BOOL bIsMenuMode = pButton->IsMenuMode ();

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

	BOOL bDefaultPanelButton = pButton->IsDefaultPanelButton () && !pButton->IsQATMode () && !pButton->IsSearchResultMode ();

	if (pButton->IsBackstageViewMode () && !pButton->IsQATMode())
	{
		BOOL bDrawIcon = FALSE;
		CBCGPControlRenderer* pRenderer = &m_ctrlRibbonBtnBsNormal;
		if (m_clrMainButton != (COLORREF)-1)
		{
			if (!m_ctrlRibbonBtnBsNormalColorized.IsValid())
			{
				m_ctrlRibbonBtnBsNormal.CopyTo(m_ctrlRibbonBtnBsNormalColorized);
				m_ctrlRibbonBtnBsNormalColorized.GetImages().AddaptColors(RGB(0, 0, 192), m_clrMainButton);
			}

			pRenderer = &m_ctrlRibbonBtnBsNormalColorized;
		}

		int index = -1;
		COLORREF clr = m_clrRibbonBsTextNormal;

		if (bDisabled)
		{
			if (bFocused)
			{
				index = 2;
			}

			clr = m_clrRibbonBsTextDisabled;
		}
		else
		{
			if (bPressed)
			{
				index = 1;
				clr = m_clrRibbonBsTextActive;
			}
			else if (bFocused || bHighlighted)
			{
				index = 0;
				clr = m_clrRibbonBsTextHighlighted;
			}
		}

		if (pButton->GetBackstageAttachedView() != NULL)
		{
			pRenderer = &m_ctrlRibbonBtnBsView;

			if (m_clrMainButton != (COLORREF)-1)
			{
				if (!m_ctrlRibbonBtnBsViewColorized.IsValid())
				{
					m_ctrlRibbonBtnBsView.CopyTo(m_ctrlRibbonBtnBsViewColorized);
					m_ctrlRibbonBtnBsViewColorized.GetImages().AddaptColors(RGB(0, 0, 192), m_clrMainButton);
				}

				pRenderer = &m_ctrlRibbonBtnBsViewColorized;
			}

			if (!bDisabled && bChecked)
			{
				index = 1;
				bDrawIcon = TRUE;
				clr = m_clrRibbonBsTextActive;
			}
		}

		if (index != -1)
		{
			pRenderer->Draw (pDC, rect, index);
			if (bDrawIcon)
			{
				m_RibbonBtnBsViewIcon.DrawEx (pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzRight, CBCGPToolBarImages::ImageAlignVertCenter);
			}
		}

		return clr;
	}

	if (bFocused)
	{
		bDisabled = FALSE;
	}

	CBCGPBaseRibbonElement::RibbonElementLocation location = 
		pButton->GetLocationInGroup ();

	if (location != CBCGPBaseRibbonElement::RibbonElementNotInGroup &&
		pButton->IsShowGroupBorder ())
	{
		if (!pButton->GetMenuRect().IsRectEmpty ())
		{
			CRect rectC = pButton->GetCommandRect();
			CRect rectM = pButton->GetMenuRect();

			CBCGPControlRenderer* pRendererC = &m_ctrlRibbonBtnGroupMenu_F[0];
			CBCGPControlRenderer* pRendererM = &m_ctrlRibbonBtnGroupMenu_L[1];

			CBCGPBitmapCache* pCacheC = &m_cacheRibbonBtnGroupMenu_F[0];
			CBCGPBitmapCache* pCacheM = &m_cacheRibbonBtnGroupMenu_L[1];

			int indexC = 0;
			int indexM = 0;

			BOOL bHighlightedC = pButton->IsCommandAreaHighlighted ();
			BOOL bHighlightedM = pButton->IsMenuAreaHighlighted ();

			if (bChecked)
			{
				indexC = 3;
				indexM = 3;
			}

			if (bDisabled)
			{
				if (bChecked)
				{
					indexC = 5;
					indexM = 5;
				}
			}
			else
			{
				if (bDroppedDown && !bIsMenuMode)
				{
					indexC = 2;
					indexM = 3;
				}
				else
				{
					if (bFocused)
					{
						indexC = 6;
						indexM = 6;
					}

					if (bHighlightedC || bHighlightedM)
					{
						if (bChecked)
						{
							indexC = bHighlightedC ? 4 : 3;
							indexM = bHighlightedM ? 4 : 3;
						}
						else
						{
							indexC = bHighlightedC ? 1 : 6;
							indexM = bHighlightedM ? 1 : 6;
						}
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
			if (bDroppedDown && !bIsMenuMode)
			{
				bChecked     = TRUE;
				bPressed     = FALSE;
				bHighlighted = FALSE;
			}

			pRenderer = &m_ctrlRibbonBtnGroup_S;
			pCache    = &m_cacheRibbonBtnGroup_S;

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
	else if (((!bDisabled && (bPressed || bChecked || bHighlighted || bDroppedDown)) || 
		      (bDisabled && bFocused)) && !bDefaultPanelButton)
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
					indexC = 1;
					indexM = 2;
				}
				else
				{
					if (bFocused)
					{
						indexC = 5;
						indexM = 5;
					}

					if (bChecked)
					{
						indexC = 2;
						indexM = 2;
					}

					if (bHighlightedC || bHighlightedM)
					{
						indexM = bChecked ? 2 : 5;

						if (bPressed)
						{
							if (bHighlightedC)
							{
								indexC = 1;
							}
							else if (bHighlightedM)
							{
								indexC = bChecked ? 2 : 5;
							}
						}
						else
						{
							if (bHighlightedC)
							{
								indexC = bChecked ? 3 : 0;
							}
							else if (bHighlightedM)
							{
								indexC = bChecked ? 2 : 5;
								indexM = bChecked ? 3 : 0;
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
	}

	if (pRenderer != NULL)
	{
		COLORREF clrText = bWasDisabled
			? m_clrRibbonBarBtnTextDisabled
			: m_clrRibbonBarBtnText;
		
		if (!bWasDisabled && pButton->GetParentGroup () != NULL)
		{
			clrText = (COLORREF)-1;
		}

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

		return clrText;
	}

	return CBCGPVisualManager2007::OnFillRibbonButton(pDC, pButton);
}
//*****************************************************************************
COLORREF CBCGPVisualManager2010::OnDrawRibbonCategoryTab (
					CDC* pDC, 
					CBCGPRibbonTab* pTab,
					BOOL bIsActive)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pTab);

	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2007::OnDrawRibbonCategoryTab (pDC, 
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
	CBCGPControlRenderer* pRendererGlow = NULL;
	COLORREF clrText = m_clrRibbonCategoryText;
	COLORREF clrTextHighlighted = m_clrRibbonCategoryTextHighlighted;
	COLORREF clrTextActive = m_clrRibbonCategoryTextActive;

	if (pCategory->GetTabColor () != BCGPCategoryColor_None)
	{
		XRibbonContextCategory& context = 
				m_ctrlRibbonContextCategory [pCategory->GetTabColor () - 1];

		pRenderer = &context.m_ctrlTab;
		clrText   = context.m_clrText;
		clrTextHighlighted = context.m_clrTextHighlighted;
		clrTextActive = context.m_clrTextActive;

		if (bIsActive && (pBar->GetHideFlags () & BCGPRIBBONBAR_HIDE_ELEMENTS) == 0)
		{
			XRibbonContextCategory2& context2 = 
				m_ctrlRibbonContextCategory2[pCategory->GetTabColor () - 1];
			if (context2.m_ctrlTabGlow.IsValid ())
			{
				pRendererGlow = &context2.m_ctrlTabGlow;
			}
		}
	}

	if (bIsActive || bPressed || bIsHighlight)
	{
		if (pRendererGlow != NULL)
		{
			CRect rectImage (pRendererGlow->GetParams ().m_rectImage);
			CRect rectActive (rectTab);
			if (rectActive.Width () > rectImage.Width ())
			{
				rectActive.left += (rectActive.Width () - rectImage.Width ()) / 2;
				rectActive.right = rectActive.left + rectImage.Width ();
			}

			rectActive.OffsetRect (0, -rectImage.Height () / 2);
			rectActive.bottom = rectActive.top + rectImage.Height ();
			pRendererGlow->Draw (pDC, rectActive);
		}

		int nImage = 0;

		if (bIsActive)
		{
			nImage = 1;

			if (pBar->GetKeyboardNavigationLevel () >= 0)
			{
				nImage = 2;
			}
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
//*****************************************************************************
COLORREF CBCGPVisualManager2010::OnDrawRibbonPanel (
		CDC* pDC,
		CBCGPRibbonPanel* pPanel, 
		CRect rectPanel,
		CRect rectCaption)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2007::OnDrawRibbonPanel (pDC, pPanel, rectPanel, rectCaption);
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
			m_ctrlRibbonBsPanelBack.Draw (pDC, rectPanel);
		}
	}
	else
	{
		BOOL bHighlighted = pPanel->IsHighlighted ();

		if (bHighlighted)
		{
			clrText = m_clrRibbonPanelTextHighlighted;
		}

		CBCGPRibbonPanelMenuBar* pMenuBar = pPanel->GetParentMenuBar ();
		if (!pPanel->IsMenuMode () && !pPanel->IsCollapsed () && (pMenuBar == NULL || pMenuBar->GetCategory () != NULL))
		{
			CBCGPControlRenderer* pRenderer = &m_ctrlRibbonPanelBack;
			CBCGPBitmapCache* pCache = &m_cacheRibbonPanelBack;

			CBCGPRibbonCategory* pCategory = pPanel->GetParentCategory ();
			ASSERT_VALID (pCategory);

			CBCGPBaseRibbonElement* pParentButton = pPanel->GetParentButton ();

			if (pCategory->GetTabColor () != BCGPCategoryColor_None &&
				(pParentButton == NULL || !pParentButton->IsQATMode ()))
			{
				XRibbonContextCategory2& context2 = 
					m_ctrlRibbonContextCategory2[pCategory->GetTabColor () - 1];

				if (context2.m_ctrlPanelBack.IsValid ())
				{
					pRenderer = &context2.m_ctrlPanelBack;
					pCache    = &context2.m_cachePanelBack;
				}

				clrText = bHighlighted 
							? m_clrRibbonContextPanelTextHighlighted
							: m_clrRibbonContextPanelText;
			}

			const CBCGPControlRendererParams& params = pRenderer->GetParams ();

			int nCacheIndex = -1;
			CSize size (params.m_rectImage.Width (), rectPanel.Height ());
			nCacheIndex = pCache->FindIndex (size);
			if (nCacheIndex == -1)
			{
				nCacheIndex = pCache->CacheY (size.cy, *pRenderer);
			}

			if (pPanel->IsOnDialogBar())
			{
				pRenderer->FillInterior (pDC, rectPanel, bHighlighted ? 1 : 0);

				CRect rectSeparator = rectPanel;
				rectSeparator.top = rectSeparator.bottom - 2;

				DrawSeparator (pDC, rectSeparator, m_penSeparatorDark, m_penSeparator2, TRUE);
			}
			else
			{
				if (nCacheIndex != -1)
				{
					pCache->Get(nCacheIndex)->DrawY (pDC, rectPanel, 
						CSize (params.m_rectInter.left, params.m_rectImage.right - params.m_rectInter.right), bHighlighted ? 1 : 0);
				}
				else
				{
					pRenderer->Draw (pDC, rectPanel, bHighlighted ? 1 : 0);
				}
			}
		}
	}

	return clrText;
}
//*****************************************************************************
COLORREF CBCGPVisualManager2010::OnFillRibbonPanelCaption (
					CDC* /*pDC*/,
					CBCGPRibbonPanel* /*pPanel*/, 
					CRect /*rectCaption*/)
{
	if (!CanDrawImage ())
	{
		return globalData.clrCaptionText;
	}

	return m_clrRibbonPanelCaptionText;
}
//*****************************************************************************
void CBCGPVisualManager2010::OnPreDrawRibbon (CDC* pDC, CBCGPRibbonBar* pRibbonBar, CRect rectTabs)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pRibbonBar);

	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnPreDrawRibbon (pDC, pRibbonBar, rectTabs);
		return;
	}

	if (pRibbonBar->AreTransparentTabs())
	{
		rectTabs.top = rectTabs.bottom - m_ctrlRibbonCategoryTabFrame.GetParams ().m_rectImage.Height ();
		m_ctrlRibbonCategoryTabFrame.Draw (pDC, rectTabs);
	}
	else
	{
		if (pRibbonBar->GetParent()->GetSafeHwnd() != NULL && !IsWindowActive(pRibbonBar->GetParent()))
		{
			m_ctrlMainBorderCaption.FillInterior(pDC, rectTabs, 1);
		}
	}
}
//*********************************************************************************
void CBCGPVisualManager2010::OnDrawRibbonMinimizeButtonImage(CDC* pDC, CBCGPRibbonMinimizeButton* pButton, BOOL bRibbonIsMinimized)
{
	if (!CanDrawImage () || !m_RibbonBtnMinimizeIcon.IsValid ())
	{
		CBCGPVisualManager2007::OnDrawRibbonMinimizeButtonImage(pDC, pButton, bRibbonIsMinimized);
		return;
	}

	ASSERT_VALID (pDC);
	ASSERT_VALID (pButton);

	m_RibbonBtnMinimizeIcon.DrawEx(pDC, pButton->GetRect(), bRibbonIsMinimized ? 1 : 0, 
		CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertCenter,
		CRect (0, 0, 0, 0), (BYTE)(pButton->IsDisabled() ? 128 : 255));
}
//*********************************************************************************
CSize CBCGPVisualManager2010::GetRibbonMinimizeButtonImageSize()
{
	if (!CanDrawImage () || !m_RibbonBtnMinimizeIcon.IsValid ())
	{
		return CBCGPVisualManager2007::GetRibbonMinimizeButtonImageSize();
	}

	CSize size = m_RibbonBtnMinimizeIcon.GetImageSize ();
	size.cx += 4;
	return size;
}
//*********************************************************************************
void CBCGPVisualManager2010::OnDrawRibbonGroupSeparator (CDC* pDC, CRect rectSeparator)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawRibbonGroupSeparator(pDC, rectSeparator);
		return;
	}

	ASSERT_VALID (pDC);
	m_ctrlRibbonBtnGroupSeparator.Draw(pDC, rectSeparator);
}
//*********************************************************************************
void CBCGPVisualManager2010::OnDrawRibbonBackstageTopLine(CDC* pDC, CRect rectLine)
{
	ASSERT_VALID (pDC);

	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawRibbonBackstageTopLine(pDC, rectLine);
		return;
	}

	CBCGPDrawManager dm (*pDC);

	dm.FillGradient (rectLine, m_clrMainButton == (COLORREF)-1 ? RGB(0, 0, 192) : m_clrMainButton, m_clrMenuLight, FALSE);
	dm.DrawLine(rectLine.left, rectLine.top, rectLine.right, rectLine.top, globalData.clrBarDkShadow);
}
//*********************************************************************************
CFont* CBCGPVisualManager2010::GetBackstageViewEntryFont()
{	
	return &globalData.fontGroup;
}
//*********************************************************************************
void CBCGPVisualManager2010::OnDrawRibbonPaletteButtonIcon (
															CDC* pDC, 
															CBCGPRibbonPaletteIcon* pButton,
															int nID,
															CRect rectImage)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawRibbonPaletteButtonIcon(pDC, pButton, nID, rectImage);
		return;
	}

	ASSERT_VALID(pDC);
	ASSERT_VALID(pButton);
	
	CBCGPMenuImages::IMAGES_IDS id = (CBCGPMenuImages::IMAGES_IDS)nID;
	
	CBCGPMenuImages::Draw (pDC, id, rectImage,
		pButton->IsDisabled() ? (CBCGPVisualManager2010::m_Style == CBCGPVisualManager2010::VS2010_Black ? CBCGPMenuImages::ImageGray : CBCGPMenuImages::ImageLtGray) : 
		CBCGPMenuImages::ImageBlack);
}

#endif

//**************************************************************************************
void CBCGPVisualManager2010::OnDrawCaptionBarButtonBorder (CDC* pDC, CBCGPCaptionBar* pBar,
											CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
											BOOL bIsDisabled, BOOL bHasDropDownArrow,
											BOOL bIsSysButton)
{
	ASSERT_VALID (pBar);

	if (!pBar->IsMessageBarMode () || !CanDrawImage () || pBar->m_clrBarBackground == -1)
	{
		CBCGPVisualManager2007::OnDrawCaptionBarButtonBorder (pDC, pBar,
											rect, bIsPressed, bIsHighlighted, 
											bIsDisabled, bHasDropDownArrow, bIsSysButton);
	}
}
//*****************************************************************************
void CBCGPVisualManager2010::OnDrawSeparator (CDC* pDC, CBCGPBaseControlBar* pBar, CRect rect, BOOL bHorz)
{
	ASSERT_VALID (pDC);

	if (!CanDrawImage () || pBar == NULL || pBar->IsDialogControl ())
	{
		CBCGPVisualManager2007::OnDrawSeparator (pDC, pBar, rect, bHorz);
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
		if (DYNAMIC_DOWNCAST(CDialog, pBar->GetParent()) == NULL)
		{
			DrawSeparator2 (pDC, rectSeparator, !bHorz);
		}
		else
		{
			DrawSeparator (pDC, rectSeparator, m_penSeparatorDark, m_penSeparator2, !bHorz);
		}
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
//***************************************************************************************
COLORREF CBCGPVisualManager2010::OnDrawMenuLabel (CDC* pDC, CRect rect)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2007::OnDrawMenuLabel (pDC, rect);
	}

	ASSERT_VALID (pDC);

	pDC->FillRect (rect, 
		m_brGroupBackground.GetSafeHandle () != NULL ? &m_brGroupBackground : &m_brBarBkgnd);

	return m_clrGroupText != (COLORREF)-1 ? m_clrGroupText : m_clrMenuText;
}
//**************************************************************************************
COLORREF CBCGPVisualManager2010::OnFillCaptionBarButton (CDC* pDC, CBCGPCaptionBar* pBar,
											CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
											BOOL bIsDisabled, BOOL bHasDropDownArrow,
											BOOL bIsSysButton)
{
	ASSERT_VALID (pBar);

	if (!pBar->IsMessageBarMode () || !CanDrawImage () || pBar->m_clrBarBackground == -1)
	{
		return CBCGPVisualManager2007::OnFillCaptionBarButton (pDC, pBar,
											rect, bIsPressed, bIsHighlighted, 
											bIsDisabled, bHasDropDownArrow, bIsSysButton);
	}

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

	if (bIsPressed || bIsHighlighted)
	{
		COLORREF clrGradient1 = CBCGPDrawManager::PixelAlpha (pBar->m_clrBarBackground, bIsPressed && bIsHighlighted ? 95 : 105);
		COLORREF clrGradient2 = CBCGPDrawManager::PixelAlpha (pBar->m_clrBarBackground, bIsPressed && bIsHighlighted ? 100 : 170);

		dm.FillGradient (rect, clrGradient1, clrGradient2, TRUE);
	}

	pDC->SelectClipRgn (NULL);

	CPen pen (PS_SOLID, 1, clrBorder);
	CPen* pOldPen = pDC->SelectObject (&pen);
	CBrush* pOldBrush = (CBrush*) pDC->SelectStockObject (NULL_BRUSH);

	pDC->RoundRect (rect.left, rect.top, rect.right, rect.bottom, nRoundSize + 2, nRoundSize + 2);

	pDC->SelectObject (pOldPen);
	pDC->SelectObject (pOldBrush);

	return bIsSysButton ? RGB(0, 0, 0) : bIsDisabled ? pBar->m_clrBarText : (COLORREF)-1;
}
//**************************************************************************************
COLORREF CBCGPVisualManager2010::GetCaptionBarTextColor (CBCGPCaptionBar* pBar)
{
	ASSERT_VALID(pBar);

	if (!CanDrawImage () || pBar->m_clrBarBackground == -1)
	{
		return CBCGPVisualManager2007::GetCaptionBarTextColor (pBar);
	}

	return globalData.clrBarText;
}
//*****************************************************************************
void CBCGPVisualManager2010::OnDrawCaptionBarInfoArea (CDC* pDC, CBCGPCaptionBar* pBar, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawCaptionBarInfoArea (pDC, pBar, rect);
	}
}
//**********************************************************************************
CBrush& CBCGPVisualManager2010::GetDlgButtonsAreaBrush(CWnd* pDlg, COLORREF* pclrLine)
{
	if (!CanDrawImage())
	{
		return CBCGPVisualManager2007::GetDlgButtonsAreaBrush (pDlg, pclrLine);
	}

	if (pclrLine != NULL)
	{
		*pclrLine = globalData.clrBarShadow;
	}

	return m_brDlgButtonsArea;
}
//**************************************************************************************
BOOL CBCGPVisualManager2010::OnDrawPushButton (CDC* pDC, CRect rect, CBCGPButton* pButton, COLORREF& clrText)
{
	ASSERT_VALID (pDC);

	BOOL bIsBackstageMode = pButton != NULL && pButton->m_bBackstageMode;

	if (!CanDrawImage () || !m_ctrlRibbonBsBtnPush.IsValid () || !bIsBackstageMode)
	{
		return CBCGPVisualManager2007::OnDrawPushButton (pDC, rect, pButton, clrText);
	}

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
	m_ctrlRibbonBsBtnPush.Draw (pDC, rect, index);

	if (bDisabled)
	{
		clrText = m_clrToolBarBtnTextDisabled;
	}
	else if (bHighlighted)
	{
		clrText = m_clrToolBarBtnTextHighlighted;
	}

	return TRUE;
}
//*******************************************************************************
void CBCGPVisualManager2010::OnDrawDlgSeparator(CDC* pDC, CBCGPStatic* pCtrl, CRect rect, BOOL bIsHorz)
{
	if (!CanDrawImage () || !pCtrl->m_bBackstageMode)
	{
		CBCGPVisualManager2007::OnDrawDlgSeparator(pDC, pCtrl, rect, bIsHorz);
		return;
	}

	if (pCtrl->GetSafeHwnd () != NULL)
	{
		::FillRect (pDC->GetSafeHdc (), rect, (HBRUSH)::GetStockObject (WHITE_BRUSH));
	}

	if (bIsHorz)
	{
		if (rect.Height () > 1)
		{
			rect.top += rect.Height () / 2;
			rect.bottom = rect.top + 1;
		}

		int nWidth = m_RibbonBsSeparatorHorz.GetImageSize ().cx;
		int nCount = rect.Width () / nWidth;
		CRect rectPart (rect);
		rectPart.right = rectPart.left + nWidth;
		for (int i = 0; i < nCount; i++)
		{
			m_RibbonBsSeparatorHorz.DrawEx (pDC, rectPart, 0, CBCGPToolBarImages::ImageAlignHorzLeft, CBCGPToolBarImages::ImageAlignVertCenter);
			rectPart.OffsetRect (nWidth, 0);
		}

		if ((nCount * nWidth) < rect.Width ())
		{
			rectPart.right = rect.right;
			rectPart.left = rect.left + nCount * nWidth;
			m_RibbonBsSeparatorHorz.DrawEx (pDC, rectPart, 0, CBCGPToolBarImages::ImageAlignHorzLeft, CBCGPToolBarImages::ImageAlignVertCenter);
		}
	}
	else
	{
		if (rect.Width () > 1)
		{
			rect.left += rect.Width () / 2;
			rect.right = rect.left + 1;
		}

		m_RibbonBsSeparatorVert.DrawEx (pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzCenter, CBCGPToolBarImages::ImageAlignVertStretch);
	}
}
//***************************************************************************************
void CBCGPVisualManager2010::OnFillOutlookPageButton (CDC* pDC, 
												  const CRect& rect,
												  BOOL bIsHighlighted, BOOL bIsPressed,
												  COLORREF& clrText)
{
	CRect rt (rect);

	if (CanDrawImage () && m_ctrlOutlookWndPageBtn.IsValid ())
	{
		rt.right--;
	}

	CBCGPVisualManager2007::OnFillOutlookPageButton (pDC, rt, bIsHighlighted, bIsPressed, clrText);
}
//****************************************************************************************
void CBCGPVisualManager2010::OnDrawOutlookPageButtonBorder (
	CDC* pDC, CRect& rectBtn, BOOL bIsHighlighted, BOOL bIsPressed)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawOutlookPageButtonBorder (pDC, rectBtn, bIsHighlighted, bIsPressed);
	}
}
//****************************************************************************************
void CBCGPVisualManager2010::OnDrawOutlookBarFrame(CDC* pDC, CRect rectFrame)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawOutlookBarFrame(pDC, rectFrame);
		return;
	}

	pDC->Draw3dRect(rectFrame, globalData.clrBarFace, globalData.clrBarFace);
}
//****************************************************************************************
COLORREF CBCGPVisualManager2010::OnFillListBoxItem (CDC* pDC, CBCGPListBox* pListBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected)
{
	COLORREF clr = CBCGPVisualManager2007::OnFillListBoxItem (pDC, pListBox, nItem, rect, bIsHighlihted, bIsSelected);

	if (CanDrawImage ())
	{
		if (pListBox != NULL && bIsSelected &&
			pListBox->IsBackstageMode () && pListBox->IsPropertySheetNavigator ())
		{
			rect.DeflateRect (0, 1);
			m_RibbonBtnBsListIcon.DrawEx (pDC, rect, 0, CBCGPToolBarImages::ImageAlignHorzRight, CBCGPToolBarImages::ImageAlignVertCenter);
		}
	}

	return clr;
}
//*************************************************************************************
void CBCGPVisualManager2010::GetCalendarColors (const CBCGPCalendar* pCalendar,
				   CBCGPCalendarColors& colors)
{
	CBCGPVisualManager2007::GetCalendarColors (pCalendar, colors);

	if (!CanDrawImage ())
	{
		return;
	}

	colors.clrSelectedText = m_clrMenuBarBtnTextHighlighted;
}

#ifndef BCGP_EXCLUDE_PLANNER

//*************************************************************************************
COLORREF CBCGPVisualManager2010::OnFillPlannerCaption (CDC* pDC,
		CBCGPPlannerView* pView, CRect rect, BOOL bIsToday, BOOL bIsSelected,
		BOOL bNoBorder/* = FALSE*/, BOOL bHorz /*= TRUE*/)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2007::OnFillPlannerCaption (pDC,
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
				if (pView->IsKindOf (RUNTIME_CLASS (CBCGPPlannerViewMulti)))
				{
					rect.top++;
					rect.left++;
				}
				else
				{
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
		CBrush br (GetPlannerHourLineColor (pView, FALSE, TRUE));
		pDC->FillRect (rect, &br);
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
void CBCGPVisualManager2010::OnDrawPlannerHeader (CDC* pDC, 
	CBCGPPlannerView* pView, CRect rect)
{
	if (!CanDrawImage ())
	{
		CBCGPVisualManager2007::OnDrawPlannerHeader (pDC, pView, rect);
		return;
	}

	ASSERT_VALID (pDC);

	COLORREF clr = GetPlannerSeparatorColor (pView);

	if (DYNAMIC_DOWNCAST(CBCGPPlannerViewMonth, pView) != NULL)
	{
		clr = m_clrPlannerNcArea;
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

			CBrush br1 (GetPlannerHourLineColor (pView, FALSE, TRUE));
			pDC->FillRect (rect1, &br1);
		}
	}
}

#endif

#ifndef BCGP_EXCLUDE_GRID_CTRL

COLORREF CBCGPVisualManager2010::OnFillReportCtrlRowBackground (CBCGPGridCtrl* pCtrl, 
												  CDC* pDC, CRect rectFill,
												  BOOL bSelected, BOOL bGroup)
{
	COLORREF clrText = CBCGPVisualManager2007::OnFillReportCtrlRowBackground (pCtrl, 
											pDC, rectFill, bSelected, bGroup);
	ASSERT_VALID (pCtrl);
	if (CanDrawImage () && bSelected && !pCtrl->IsFocused ())
	{
		clrText = pCtrl->IsControlBarColors () ? m_clrMenuBarBtnText : globalData.clrBtnText;
	}

	return clrText;
}
//********************************************************************************
COLORREF CBCGPVisualManager2010::OnFillGridItem (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill, BOOL bSelected, BOOL bActiveItem, BOOL bSortedColumn)
{
	COLORREF clrText = CBCGPVisualManager2007::OnFillGridItem (pCtrl, pDC, rectFill, bSelected, bActiveItem, bSortedColumn);

	if (CanDrawImage () && bSelected && !bActiveItem && !pCtrl->IsFocused ())
	{
		clrText = pCtrl->IsControlBarColors () ? m_clrMenuBarBtnText : globalData.clrBtnText;
	}

	return clrText;
}
//********************************************************************************
BOOL CBCGPVisualManager2010::OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme)
{
	CBCGPVisualManager2007::OnSetGridColorTheme (pCtrl, theme);

	theme.m_SelColors.m_clrBackground = theme.m_GroupSelColors.m_clrBackground;
	theme.m_SelColors.m_clrGradient = theme.m_GroupSelColors.m_clrGradient;
	theme.m_SelColors.m_clrBorder = theme.m_GroupSelColors.m_clrBorder;
	theme.m_SelColors.m_clrText = theme.m_GroupSelColors.m_clrText;

	return TRUE;
}

#endif

COLORREF CBCGPVisualManager2010::GetTreeControlFillColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected, BOOL bIsFocused, BOOL bIsDisabled)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2007::GetTreeControlFillColor(pTreeCtrl, bIsSelected, bIsFocused, bIsDisabled);
	}

	if (bIsSelected)
	{
		return bIsFocused ? m_clrRibbonComboBtnHighlightedStart : globalData.clrBarFace;
	}

	return m_clrToolBarGradientLight;
}
//*******************************************************************************
COLORREF CBCGPVisualManager2010::GetTreeControlTextColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected, BOOL bIsFocused, BOOL bIsDisabled)
{
	if (!CanDrawImage ())
	{
		return CBCGPVisualManager2007::GetTreeControlTextColor(pTreeCtrl, bIsSelected, bIsFocused, bIsDisabled);
	}

	if (bIsSelected)
	{
		return bIsFocused ? m_clrToolBarBtnTextHighlighted : globalData.clrBarText;
	}

	return globalData.clrBarText;
}
