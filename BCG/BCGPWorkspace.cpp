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

#include "stdafx.h"
#include "BCGGlobals.h"
#include "BCGCBPro.h"
#include "BCGPToolBar.h"
#include "BCGPWorkspace.h"

#include "BCGPFrameImpl.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPFrameWnd.h"
#include "BCGPOleIPFrameWnd.h"

#include "BCGPMouseManager.h"
#include "BCGPContextMenuManager.h"
#include "BCGPKeyboardManager.h"
#include "BCGPUserToolsManager.h"
#include "BCGPTearOffManager.h"
#include "BCGPSkinManager.h"
#include "BCGPShellManager.h"
#include "BCGPTooltipManager.h"
#include "BCGPRibbonBar.h"

#include "BCGPRegistry.h"
#include "RegPath.h"
#include "BCGPRebarState.h"
#include "BCGCBProVer.h"	// Library version info.

#include "BCGPVisualManager.h"
#include "BCGPVisualManagerXP.h"
#include "BCGPWinXPVisualManager.h"
#include "BCGPVisualManager2003.h"
#include "BCGPVisualManagerVS2005.h"
#include "BCGPVisualManager2007.h"
#include "BCGPVisualManagerVS2008.h"
#include "BCGPVisualManagerVS2010.h"
#include "BCGPVisualManagerVS2012.h"
#include "BCGPVisualManagerCarbon.h"
#include "BCGPVisualManagerScenic.h"
#include "BCGPVisualManager2010.h"
#include "BCGPVisualManager2013.h"

#include "BCGPToolbarCustomize.h"
#include "BCGPTabbedControlBar.h"

#include "BCGPDrawManager.h"
#include "BCGPGlobalUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////////
// CBCGPWinApp

IMPLEMENT_DYNAMIC(CBCGPWinApp, CWinApp)

CBCGPWinApp::CBCGPWinApp() :
	CBCGPWorkspace(TRUE),
	m_bDPIAware(TRUE),
	m_bMSAASupport(TRUE),
	m_bKeyboardManager(TRUE),
	m_bContextMenuManager(TRUE),
	m_bShellManager(TRUE),
	m_bThemedTooltips(TRUE),
	m_bReloadRecentTheme(TRUE),
	m_strRecentThemeRegistry(_T("RecentVisualTheme")),
	m_pMainWndTemp(NULL)
{
	m_bAutoMenuBar = TRUE;
	m_bMouseWheelInInactiveWindow = TRUE;

	SetVisualTheme(BCGP_VISUAL_THEME_DEFAULT);
}
//*************************************************************************************
BOOL CBCGPWinApp::InitInstance()
{
	if (!CWinApp::InitInstance())
	{
		return FALSE;
	}

	if (m_bDPIAware)
	{
		globalData.SetDPIAware ();
	}

	if (m_bMSAASupport)
	{
		globalData.EnableAccessibilitySupport();
	}

	if (m_bContextMenuManager)
	{
		InitContextMenuManager();
	}

	if (m_bKeyboardManager)
	{
		InitKeyboardManager();
	}

	if (m_bShellManager)
	{
		InitShellManager();
	}

	if (m_bThemedTooltips)
	{
		InitTooltipManager();

		if (g_pTooltipManager != NULL)
		{
			CBCGPToolTipParams params;
			params.m_bVislManagerTheme = TRUE;
		
			g_pTooltipManager->SetTooltipParams(BCGP_TOOLTIP_TYPE_ALL, RUNTIME_CLASS (CBCGPToolTipCtrl), &params);
		}
	}

	return TRUE;
}
//*************************************************************************************
int CBCGPWinApp::ExitInstance()
{
	if (m_bReloadRecentTheme && m_mapVisualThemeCmds.GetCount() > 1)
	{
		WriteInt(m_strRecentThemeRegistry, (int)m_ActiveTheme);
	}

	m_mapVisualThemeCmds.RemoveAll();

	BCGCBProCleanUp ();
	return CWinApp::ExitInstance();
}
//*************************************************************************************
void CBCGPWinApp::SetVisualTheme(BCGP_VISUAL_THEME theme)
{
	CWaitCursor* pWaitCursor = NULL;
	if (afxData.hcurWait != NULL)
	{
		pWaitCursor = new CWaitCursor;
	}

	m_ActiveTheme = theme;

	m_AppOptions.Reset();

	BCGP_DOCK_TYPE dockMode = BCGP_DT_SMART;

	CWnd* pWndMain = m_pMainWndTemp->GetSafeHwnd() == NULL ? GetMainWnd() : m_pMainWndTemp;
	if (pWndMain->GetSafeHwnd() != NULL)
	{
		pWndMain->LockWindowUpdate ();
	}

	BOOL bWereSmallBorders = CBCGPVisualManager::GetInstance()->IsSmallSystemBorders();

	switch (theme)
	{
	case BCGP_VISUAL_THEME_DEFAULT:
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPWinXPVisualManager));
		CBCGPWinXPVisualManager::m_b3DTabsXPTheme = TRUE;
		break;

	case BCGP_VISUAL_THEME_OFFICE_2000:
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManager));
		dockMode = BCGP_DT_STANDARD;
		break;
		
	case BCGP_VISUAL_THEME_OFFICE_XP:
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManagerXP));
		dockMode = BCGP_DT_STANDARD;
		break;
		
	case BCGP_VISUAL_THEME_OFFICE_2003:
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManager2003));

		m_AppOptions.m_MDITabsStyle = CBCGPTabWnd::STYLE_3D_ONENOTE;
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_ACTIVE;
		m_AppOptions.m_bMDITabsAutoColor = TRUE;
		dockMode = BCGP_DT_STANDARD;
		break;
		
	case BCGP_VISUAL_THEME_VS_2005:
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManagerVS2005));

		m_AppOptions.m_MDITabsStyle = CBCGPTabWnd::STYLE_3D_VS2005;
		break;
		
	case BCGP_VISUAL_THEME_OFFICE_2007_BLUE:
		CBCGPVisualManager2007::SetStyle (CBCGPVisualManager2007::VS2007_LunaBlue);
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManager2007));
		
		m_AppOptions.m_bMDITabsAutoColor = TRUE;
		m_AppOptions.m_MDITabsStyle = CBCGPTabWnd::STYLE_3D_ONENOTE;
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_ACTIVE;
		break;
		
	case BCGP_VISUAL_THEME_OFFICE_2007_BLACK:
		CBCGPVisualManager2007::SetStyle (CBCGPVisualManager2007::VS2007_ObsidianBlack);
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManager2007));
		
		m_AppOptions.m_bMDITabsAutoColor = TRUE;
		m_AppOptions.m_MDITabsStyle = CBCGPTabWnd::STYLE_3D_ONENOTE;
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_ACTIVE;
		break;
		
	case BCGP_VISUAL_THEME_OFFICE_2007_SILVER:
		CBCGPVisualManager2007::SetStyle (CBCGPVisualManager2007::VS2007_Silver);
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManager2007));
		
		m_AppOptions.m_bMDITabsAutoColor = TRUE;
		m_AppOptions.m_MDITabsStyle = CBCGPTabWnd::STYLE_3D_ONENOTE;
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_ACTIVE;
		break;
		
	case BCGP_VISUAL_THEME_OFFICE_2007_AQUA:
		CBCGPVisualManager2007::SetStyle (CBCGPVisualManager2007::VS2007_Aqua);
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManager2007));
		
		m_AppOptions.m_bMDITabsAutoColor = TRUE;
		m_AppOptions.m_MDITabsStyle = CBCGPTabWnd::STYLE_3D_ONENOTE;
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_ACTIVE;
		break;
		
	case BCGP_VISUAL_THEME_CARBON:
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManagerCarbon));
		break;
		
	case BCGP_VISUAL_THEME_VS_2008:
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManagerVS2008));

		m_AppOptions.m_MDITabsStyle = CBCGPTabWnd::STYLE_3D_VS2005;
		break;
		
	case BCGP_VISUAL_THEME_VS_2010:
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManagerVS2010));
		
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_HIGHLIGHTED;
		m_AppOptions.m_bMDIActiveTabBold = FALSE;
		break;
		
	case BCGP_VISUAL_THEME_SCENIC:
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManagerScenic));

		m_AppOptions.m_MDITabsStyle = CBCGPTabWnd::STYLE_3D_ONENOTE;
		m_AppOptions.m_bMDITabsAutoColor = TRUE;
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_ACTIVE;
		
		if (globalData.bIsWindows8)
		{
			m_AppOptions.m_strScenicRibbonLabel = _T("File");
			m_AppOptions.m_bRibbonMinimizeButton = TRUE;
		}

		m_AppOptions.m_bScenicRibbon = TRUE;
		break;
		
	case BCGP_VISUAL_THEME_OFFICE_2010_BLUE:
		CBCGPVisualManager2010::SetStyle (CBCGPVisualManager2010::VS2010_Blue);
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManager2010));
		
		m_AppOptions.m_bMDITabsAutoColor = TRUE;
		m_AppOptions.m_MDITabsStyle = CBCGPTabWnd::STYLE_3D_ONENOTE;
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_ACTIVE;
		
		m_AppOptions.m_strScenicRibbonLabel = _T("File");
		m_AppOptions.m_bScenicRibbon = TRUE;
		m_AppOptions.m_bRibbonMinimizeButton = TRUE;
		break;
		
	case BCGP_VISUAL_THEME_OFFICE_2010_BLACK:
		CBCGPVisualManager2010::SetStyle (CBCGPVisualManager2010::VS2010_Black);
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManager2010));
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_ACTIVE;
		
		m_AppOptions.m_strScenicRibbonLabel = _T("File");
		m_AppOptions.m_bScenicRibbon = TRUE;
		m_AppOptions.m_bRibbonMinimizeButton = TRUE;
		break;
		
	case BCGP_VISUAL_THEME_OFFICE_2010_SILVER:
		CBCGPVisualManager2010::SetStyle (CBCGPVisualManager2010::VS2010_Silver);
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManager2010));
		
		m_AppOptions.m_bMDITabsAutoColor = TRUE;
		m_AppOptions.m_MDITabsStyle = CBCGPTabWnd::STYLE_3D_ONENOTE;
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_ACTIVE;
		
		m_AppOptions.m_strScenicRibbonLabel = _T("File");
		m_AppOptions.m_bScenicRibbon = TRUE;
		m_AppOptions.m_bRibbonMinimizeButton = TRUE;
		break;
		
	case BCGP_VISUAL_THEME_VS_2012_LIGHT:
	case BCGP_VISUAL_THEME_VS_2013_LIGHT:
		CBCGPVisualManagerVS2012::SetStyle(CBCGPVisualManagerVS2012::VS2012_Light);
		
		CBCGPVisualManager::SetDefaultManager(theme == BCGP_VISUAL_THEME_VS_2012_LIGHT ? 
			RUNTIME_CLASS(CBCGPVisualManagerVS2012) :
			RUNTIME_CLASS(CBCGPVisualManagerVS2013));
		
		m_AppOptions.m_bMDIActiveTabBold = FALSE;
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_HIGHLIGHTED;
		
		m_AppOptions.m_strScenicRibbonLabel = _T("File");
		m_AppOptions.m_bScenicRibbon = TRUE;
		m_AppOptions.m_bRibbonMinimizeButton = TRUE;
		break;
		
	case BCGP_VISUAL_THEME_VS_2012_DARK:
	case BCGP_VISUAL_THEME_VS_2013_DARK:
		CBCGPVisualManagerVS2012::SetStyle(CBCGPVisualManagerVS2012::VS2012_Dark);

		CBCGPVisualManager::SetDefaultManager(theme == BCGP_VISUAL_THEME_VS_2012_DARK ? 
			RUNTIME_CLASS(CBCGPVisualManagerVS2012) :
			RUNTIME_CLASS(CBCGPVisualManagerVS2013));
		
		m_AppOptions.m_bMDIActiveTabBold = FALSE;
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_HIGHLIGHTED;
		
		m_AppOptions.m_strScenicRibbonLabel = _T("File");
		m_AppOptions.m_bScenicRibbon = TRUE;
		m_AppOptions.m_bRibbonMinimizeButton = TRUE;
		break;
		
	case BCGP_VISUAL_THEME_VS_2012_BLUE:
	case BCGP_VISUAL_THEME_VS_2013_BLUE:
		CBCGPVisualManagerVS2012::SetStyle(CBCGPVisualManagerVS2012::VS2012_LightBlue);

		CBCGPVisualManager::SetDefaultManager(theme == BCGP_VISUAL_THEME_VS_2012_BLUE ? 
			RUNTIME_CLASS(CBCGPVisualManagerVS2012) :
			RUNTIME_CLASS(CBCGPVisualManagerVS2013));
		
		m_AppOptions.m_bMDIActiveTabBold = FALSE;
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_HIGHLIGHTED;
		
		m_AppOptions.m_strScenicRibbonLabel = _T("File");
		m_AppOptions.m_bScenicRibbon = TRUE;
		m_AppOptions.m_bRibbonMinimizeButton = TRUE;
		break;
		
	case BCGP_VISUAL_THEME_OFFICE_2013_WHITE:
		CBCGPVisualManager2013::SetStyle(CBCGPVisualManager2013::Office2013_White);
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManager2013));

		m_AppOptions.m_bMDIActiveTabBold = FALSE;
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_ACTIVE;
		m_AppOptions.m_bMDITabsLargeFont = TRUE;

		m_AppOptions.m_strScenicRibbonLabel = _T("File");
		m_AppOptions.m_bScenicRibbon = TRUE;
		m_AppOptions.m_bRibbonMinimizeButton = TRUE;
		break;
		
	case BCGP_VISUAL_THEME_OFFICE_2013_GRAY:
		CBCGPVisualManager2013::SetStyle(CBCGPVisualManager2013::Office2013_Gray);
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManager2013));

		m_AppOptions.m_bMDIActiveTabBold = FALSE;
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_ACTIVE;
		m_AppOptions.m_bMDITabsLargeFont = TRUE;
		
		m_AppOptions.m_strScenicRibbonLabel = _T("File");
		m_AppOptions.m_bScenicRibbon = TRUE;
		m_AppOptions.m_bRibbonMinimizeButton = TRUE;
		break;
		
	case BCGP_VISUAL_THEME_OFFICE_2013_DARK_GRAY:
		CBCGPVisualManager2013::SetStyle(CBCGPVisualManager2013::Office2013_DarkGray);
		CBCGPVisualManager::SetDefaultManager(RUNTIME_CLASS(CBCGPVisualManager2013));
		
		m_AppOptions.m_bMDIActiveTabBold = FALSE;
		m_AppOptions.m_MDITabsCloseButtonMode = CBCGPTabWnd::TAB_CLOSE_BUTTON_ACTIVE;
		m_AppOptions.m_bMDITabsLargeFont = TRUE;
		
		m_AppOptions.m_strScenicRibbonLabel = _T("File");
		m_AppOptions.m_bScenicRibbon = TRUE;
		m_AppOptions.m_bRibbonMinimizeButton = TRUE;
		break;

	case BCGP_VISUAL_THEME_CUSTOM:
		break;

	default:
		ASSERT(FALSE);
	}

	if (!globalData.m_bShowFrameLayeredShadows && pWndMain->GetSafeHwnd() != NULL && pWndMain->IsZoomed() && CBCGPVisualManager::GetInstance()->IsSmallSystemBorders() && !bWereSmallBorders)
	{
		globalUtils.EnableWindowShadow(pWndMain, FALSE);
		globalUtils.EnableWindowShadow(pWndMain, TRUE);
	}

	OnBeforeChangeVisualTheme(m_AppOptions, pWndMain);

	CBCGPRibbonBar* pRibbonBar = NULL;
	CBCGPDockManager* pDockManager = NULL;
	CFrameWnd* pWndFrame = NULL;

	CBCGPMDIFrameWnd* pMDIMainFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, pWndMain);
	if (pMDIMainFrame->GetSafeHwnd() != NULL)
	{
		ASSERT_VALID(pMDIMainFrame);

		pWndFrame = pMDIMainFrame;

		CBCGPMDITabParams mdiTabParams;

		if (pMDIMainFrame->IsMDITabbedGroup())
		{
			mdiTabParams = pMDIMainFrame->GetMDITabbedGroupsParams();
			
			mdiTabParams.m_bAutoColor = m_AppOptions.m_bMDITabsAutoColor;
			mdiTabParams.m_style = m_AppOptions.m_MDITabsStyle;
			mdiTabParams.m_closeButtonMode = m_AppOptions.m_MDITabsCloseButtonMode;
			mdiTabParams.m_bTabsCaptionFont = m_AppOptions.m_bMDITabsLargeFont;
			mdiTabParams.m_bActiveTabBoldFont = m_AppOptions.m_bMDIActiveTabBold;
			mdiTabParams.m_bDocumentMenu = TRUE;

			pMDIMainFrame->EnableMDITabbedGroups (TRUE, mdiTabParams);
		}
		else if (pMDIMainFrame->AreMDITabs())
		{
			pMDIMainFrame->EnableMDITabs(TRUE /* Enable */, TRUE /* With icons */,
				CBCGPTabWnd::LOCATION_TOP, TRUE /* Close button */,
				m_AppOptions.m_MDITabsStyle);
			pMDIMainFrame->GetMDITabs().EnableAutoColor(m_AppOptions.m_bMDITabsAutoColor);
			pMDIMainFrame->GetMDITabs().SetActiveTabBoldFont(m_AppOptions.m_bMDIActiveTabBold);
			pMDIMainFrame->GetMDITabs().SetTabCloseButtonMode(m_AppOptions.m_MDITabsCloseButtonMode);
			pMDIMainFrame->GetMDITabs().SetCaptionFont(m_AppOptions.m_bMDITabsLargeFont);
		}

		pRibbonBar = pMDIMainFrame->GetRibbonBar();
		pDockManager = pMDIMainFrame->GetDockManager();
	}
	else
	{
		CBCGPFrameWnd* pSDIMainFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, pWndMain);
		if (pSDIMainFrame->GetSafeHwnd() != NULL)
		{
			pWndFrame = pSDIMainFrame;

			pRibbonBar = pSDIMainFrame->GetRibbonBar();
			pDockManager = pSDIMainFrame->GetDockManager();
		}
	}

	if (pRibbonBar->GetSafeHwnd() != NULL)
	{
		pRibbonBar->SetScenicLook(m_AppOptions.m_bScenicRibbon, FALSE);
		pRibbonBar->EnableMinimizeButton(m_AppOptions.m_bRibbonMinimizeButton, FALSE);
		
		if (pRibbonBar->GetMainButton() != NULL)
		{
			pRibbonBar->GetMainButton()->SetScenicText(m_AppOptions.m_strScenicRibbonLabel);
		}

		pRibbonBar->RecalcLayout();
	}

	if (pDockManager != NULL)
	{
		ASSERT_VALID (pDockManager);
		pDockManager->AdjustBarFrames();
	}
	
	CBCGPTabbedControlBar::ResetTabs ();
	CBCGPDockManager::SetDockMode(dockMode);

	OnAfterChangeVisualTheme(pWndMain);

	if (pWndMain->GetSafeHwnd() != NULL)
	{
		pWndMain->UnlockWindowUpdate();
		
		if (pWndFrame->GetSafeHwnd() != NULL)
		{
			pWndFrame->RecalcLayout();
		}

		pWndMain->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN | RDW_FRAME);
	}
	
	if (pWaitCursor != NULL)
	{
		delete pWaitCursor;
	}
}
//*************************************************************************************
UINT CBCGPWinApp::GetVisualThemeCommandID(BCGP_VISUAL_THEME theme) const
{
	for (POSITION pos = m_mapVisualThemeCmds.GetStartPosition(); pos != NULL;)
	{
		BCGP_VISUAL_THEME nNextTheme = (BCGP_VISUAL_THEME)-1;
		UINT nCommandID = (UINT)-1;

		m_mapVisualThemeCmds.GetNextAssoc(pos, nCommandID, nNextTheme);

		if (nNextTheme == theme)
		{
			return nCommandID;
		}
	}

	return (UINT)-1;
}
//*************************************************************************************
void CBCGPWinApp::AddVisualTheme(BCGP_VISUAL_THEME theme, UINT nID, BOOL bActive)
{
	m_mapVisualThemeCmds.SetAt(nID, theme);

	if (bActive)
	{
		SetVisualTheme(theme); 
	}
}
//*************************************************************************************
BOOL CBCGPWinApp::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	BCGP_VISUAL_THEME theme = (BCGP_VISUAL_THEME)-1;

	if (nID != 0 && (m_mapVisualThemeCmds.Lookup(nID, theme) || nID == m_ToolbarOptions.m_nCustomizeCommandID))
	{
		if (nCode == CN_UPDATE_COMMAND_UI && pExtra != NULL)
		{
			CCmdUI* pCmdUI = (CCmdUI*)pExtra;
			
			pCmdUI->Enable();
			pCmdUI->SetRadio(theme == m_ActiveTheme);

			return TRUE;
		}
		else if (nCode == CN_COMMAND)
		{
			if (nID == m_ToolbarOptions.m_nCustomizeCommandID)
			{
				return OnCustomizeToolBars();
			}
			else if (theme != (BCGP_VISUAL_THEME)-1)
			{
				SetVisualTheme(theme);
				return TRUE;
			}
		}
	}

	return CWinApp::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
//*************************************************************************************
BOOL CBCGPWinApp::OnCustomizeToolBars()
{
	CFrameWnd* pMainFrame = DYNAMIC_DOWNCAST(CFrameWnd, GetMainWnd());
	if (pMainFrame == NULL)
	{
		return FALSE;
	}

	CBCGPToolbarCustomize* pDlgCust = new CBCGPToolbarCustomize(pMainFrame,
		TRUE /* Automatic menus scaning */, m_ToolbarOptions.m_nToolbarCustomizeFlags);
	
	if (m_ToolbarOptions.m_nUserToolbarCommandIDFirst != 0 && m_ToolbarOptions.m_nUserToolbarCommandIDLast != 0)
	{
		pDlgCust->EnableUserDefinedToolbars ();
	}

	OnBeforeCreateCustomizationDlg(pDlgCust);
	
	pDlgCust->Create();
	return TRUE;
}
//*************************************************************************************
void CBCGPWinApp::PreSaveState()
{
	CBCGPWorkspace::PreSaveState();

	if (m_bReloadRecentTheme && m_mapVisualThemeCmds.GetCount() > 1)
	{
		WriteInt(m_strRecentThemeRegistry, (int)m_ActiveTheme);
	}
}
//*************************************************************************************
BOOL CBCGPWinApp::LoadState(LPCTSTR lpszSectionName, CBCGPFrameImpl* pFrameImpl)
{
	if (pFrameImpl != NULL && m_ToolbarOptions.m_nUserToolbarCommandIDFirst != 0 && m_ToolbarOptions.m_nUserToolbarCommandIDLast != 0)
	{
		pFrameImpl->InitUserToolbars(NULL, m_ToolbarOptions.m_nUserToolbarCommandIDFirst, m_ToolbarOptions.m_nUserToolbarCommandIDLast);
	}

	if (!m_ToolbarOptions.m_strToolbarCustomIconsPath.IsEmpty() && m_ToolbarCustomIcons.Load(m_ToolbarOptions.m_strToolbarCustomIconsPath))
	{
		CBCGPToolBar::SetUserImages(&m_ToolbarCustomIcons, TRUE);
	}

	if (pFrameImpl != NULL)
	{
		m_pMainWndTemp = pFrameImpl->m_pFrame;
	}

	if (m_bReloadRecentTheme && m_mapVisualThemeCmds.GetCount() > 1)
	{
		SetVisualTheme((BCGP_VISUAL_THEME)GetInt(m_strRecentThemeRegistry, m_ActiveTheme));
	}
	else
	{
		// Maybe, some visual manager is already instatiated?
		if (CBCGPVisualManager::m_pRTIDefault == RUNTIME_CLASS(CBCGPWinXPVisualManager) && m_ActiveTheme == BCGP_VISUAL_THEME_DEFAULT)
		{
			SetVisualTheme(m_ActiveTheme);
		}
	}

	m_pMainWndTemp = NULL;

	if (!CBCGPWorkspace::LoadState(lpszSectionName, pFrameImpl))
	{
		return FALSE;
	}

	if (pFrameImpl != NULL && m_ToolbarOptions.m_bInitialized)
	{
		pFrameImpl->SetupToolbars(m_ToolbarOptions);
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// CBCGPWorkspace

CBCGPWorkspace* g_pWorkspace = NULL;
BOOL           g_bWorkspaceAutocreated = FALSE;

CBCGPWorkspace* GetWorkspace()
{ 
	//---------------------------------------------------------------------
	// You must either:
	// ----------------
	// a) construct a CBCGPWorkspace object
	// b) mix a CBCGPWorkspace class somewhere in (e.g. your CWinApp object)
	// c) call CBCGPWorkspace::UseWorkspaceManager() to automatically
	//    initialize an object for you
	//---------------------------------------------------------------------
	ASSERT (g_pWorkspace != NULL);
	return g_pWorkspace; 
}

//-----------------------
// clean up if necessary:
//-----------------------
struct _WORKSPACE_TERM
{
	~_WORKSPACE_TERM()
	{
		if (g_pWorkspace != NULL && g_bWorkspaceAutocreated)
		{
			delete g_pWorkspace;
			g_pWorkspace = NULL;
			g_bWorkspaceAutocreated = FALSE;
		}
	}
};
static const _WORKSPACE_TERM workspaceTerm;

//*************************************************************************************

static const CString strRegEntryNameControlBars		= _T("\\ControlBars");
static const CString strWindowPlacementRegSection	= _T("WindowPlacement");
static const CString strRectMainKey					= _T("MainWindowRect");
static const CString strFlagsKey					= _T("Flags");
static const CString strShowCmdKey					= _T("ShowCmd");
static const CString strRegEntryNameSizingBars		= _T("\\SizingBars");
static const CString strRegEntryVersion				= _T("BCGControlBarVersion");
static const CString strVersionMajorKey				= _T("Major");
static const CString strVersionMinorKey				= _T("Minor");
static const CString strRegEntryPinned				= _T("Pinned");
static const CString strPinnedFilesKey				= _T("PinnedFiles");
static const CString strPinnedFoldersKey			= _T("PinnedFolders");
static const CString strInputMode					= _T("InputMode");

extern CObList	gAllToolbars;

//*************************************************************************************
BOOL CBCGPWorkspace::UseWorkspaceManager(LPCTSTR lpszSectionName /*=NULL*/, BOOL bResourceSmartUpdate)
{
	if(g_pWorkspace != NULL)
	{
		return FALSE;	// already exists
	}

	g_pWorkspace = new CBCGPWorkspace (bResourceSmartUpdate);
	g_bWorkspaceAutocreated = TRUE;	// Cleanup

	if(lpszSectionName != NULL)
	{
		g_pWorkspace->m_strRegSection = lpszSectionName;
	}
	
	return TRUE;
}
//*************************************************************************************
LPCTSTR CBCGPWorkspace::SetRegistryBase(LPCTSTR lpszSectionName /*= NULL*/)
{
	m_strRegSection = (lpszSectionName != NULL) ? 
			lpszSectionName : 
			_T("");

	return m_strRegSection;
}
//*************************************************************************************
CBCGPWorkspace::CBCGPWorkspace (BOOL bResourceSmartUpdate /*= FALSE*/) :
							m_bResourceSmartUpdate (bResourceSmartUpdate),
							m_bAutoMenuBar(FALSE)
{
	// ONLY ONE ALLOWED
	ASSERT(g_pWorkspace == NULL);
	g_pWorkspace = this;

	m_bKeyboardManagerAutocreated = FALSE;
	m_bContextMenuManagerAutocreated = FALSE;
	m_bMouseManagerAutocreated = FALSE;
	m_bUserToolsManagerAutoCreated = FALSE;
	m_bTearOffManagerAutoCreated = FALSE;
	m_bSkinManagerAutocreated = FALSE;
	m_bShellManagerAutocreated = FALSE;
	m_bTooltipManagerAutocreated = FALSE;

	const CString strRegEntryNameWorkspace = _T("BCGWorkspace");
	m_strRegSection = strRegEntryNameWorkspace;

	m_iSavedVersionMajor = -1;
	m_iSavedVersionMinor = -1;

	m_bForceDockStateLoad = FALSE;
	m_bLoadSaveFrameBarsOnly = FALSE;

	m_bSaveState = TRUE;
	m_bAfxStoreDockSate = TRUE;
	m_bForceImageReset = FALSE;

	m_bLoadUserToolbars	= TRUE;

	m_bLoadWindowPlacement = TRUE;

	m_bTaskBarInteraction = TRUE;
	m_bMouseWheelInInactiveWindow = FALSE;
}
//*************************************************************************************
CBCGPWorkspace::~CBCGPWorkspace()
{
	// NO OTHER !!
	ASSERT(g_pWorkspace == this);
	g_pWorkspace = NULL;

	// Delete autocreated managers
	if(m_bKeyboardManagerAutocreated && g_pKeyboardManager != NULL)
	{
		delete g_pKeyboardManager;
		g_pKeyboardManager = NULL;
	}

	if (m_bContextMenuManagerAutocreated && g_pContextMenuManager != NULL)
	{
		delete g_pContextMenuManager;
		g_pContextMenuManager = NULL;
	}

	if (m_bMouseManagerAutocreated && g_pMouseManager != NULL)
	{
		delete g_pMouseManager;
		g_pMouseManager = NULL;
	}

	if (m_bUserToolsManagerAutoCreated && g_pUserToolsManager != NULL)
	{
		delete g_pUserToolsManager;
		g_pUserToolsManager = NULL;
	}

	if (m_bTearOffManagerAutoCreated && g_pBCGPTearOffMenuManager != NULL)
	{
		delete g_pBCGPTearOffMenuManager;
		g_pBCGPTearOffMenuManager = NULL;
	}

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version
	if (m_bSkinManagerAutocreated && g_pSkinManager != NULL)
	{
		delete g_pSkinManager;
		g_pSkinManager = NULL;
	}
#endif

#ifndef BCGP_EXCLUDE_SHELL
	if (m_bShellManagerAutocreated && g_pShellManager != NULL)
	{
		delete g_pShellManager;
		g_pShellManager = NULL;
	}
#endif

	if (m_bTooltipManagerAutocreated && g_pTooltipManager != NULL)
	{
		delete g_pTooltipManager;
		g_pTooltipManager = NULL;
	}
}
//*************************************************************************************
BOOL CBCGPWorkspace::InitShellManager()
{
#ifdef BCGP_EXCLUDE_SHELL

	ASSERT (FALSE);
	return FALSE;

#else
	if (g_pShellManager != NULL)
	{
		return FALSE;
	}

	g_pShellManager = new CBCGPShellManager;
	m_bShellManagerAutocreated = TRUE;
	return TRUE;
#endif
}
//*************************************************************************************
BOOL CBCGPWorkspace::InitTooltipManager()
{
	if (g_pTooltipManager != NULL)
	{
		return FALSE;
	}

	g_pTooltipManager = new CBCGPTooltipManager;
	m_bTooltipManagerAutocreated = TRUE;
	return TRUE;
}
//*************************************************************************************
BOOL CBCGPWorkspace::InitMouseManager()
{
	if (g_pMouseManager != NULL)
	{
		return FALSE;
	}

	g_pMouseManager = new CBCGPMouseManager;
	m_bMouseManagerAutocreated = TRUE;
	return TRUE;
}
//*************************************************************************************
BOOL CBCGPWorkspace::InitContextMenuManager()
{
	if (g_pContextMenuManager != NULL)
	{
		return FALSE;
	}

	g_pContextMenuManager = new CBCGPContextMenuManager;
	m_bContextMenuManagerAutocreated = TRUE;
	
	return TRUE;
}
//*************************************************************************************
BOOL CBCGPWorkspace::InitKeyboardManager()
{
	if (g_pKeyboardManager != NULL)
	{
		return FALSE;
	}

	g_pKeyboardManager = new CBCGPKeyboardManager;
	m_bKeyboardManagerAutocreated = TRUE;

	return TRUE;
}

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version

BOOL CBCGPWorkspace::InitSkinManager(LPCTSTR lpszSkinsDirectory/* = BCG_DEFAULT_SKINS_DIR*/)
{
	if (g_pSkinManager != NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	g_pSkinManager = new CBCGPSkinManager (lpszSkinsDirectory);
	m_bSkinManagerAutocreated = TRUE;

	return TRUE;
}

#endif

BOOL CBCGPWorkspace::EnableUserTools (const UINT uiCmdToolsDummy,
									 const UINT uiCmdFirst, const UINT uiCmdLast,
									CRuntimeClass* pToolRTC,
									UINT uArgMenuID, UINT uInitDirMenuID)

{
	if (g_pUserToolsManager != NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	g_pUserToolsManager = new 
		CBCGPUserToolsManager (	uiCmdToolsDummy, uiCmdFirst, uiCmdLast, pToolRTC,
								uArgMenuID, uInitDirMenuID);
	m_bUserToolsManagerAutoCreated = TRUE;

	return TRUE;
}
//*************************************************************************************
BOOL CBCGPWorkspace::EnableTearOffMenus (LPCTSTR lpszRegEntry,
							const UINT uiCmdFirst, const UINT uiCmdLast)
{
	if (g_pBCGPTearOffMenuManager != NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	g_pBCGPTearOffMenuManager = new CBCGPTearOffManager;
	m_bTearOffManagerAutoCreated = TRUE;

	return g_pBCGPTearOffMenuManager->Initialize (lpszRegEntry, uiCmdFirst, uiCmdLast);
}
//**************************************************************************************
CBCGPMouseManager* CBCGPWorkspace::GetMouseManager()
{
	if (g_pMouseManager == NULL)
	{
		InitMouseManager ();
	}

	ASSERT_VALID (g_pMouseManager);
	return g_pMouseManager;
}
//*************************************************************************************
CBCGPShellManager* CBCGPWorkspace::GetShellManager()
{
#ifdef BCGP_EXCLUDE_SHELL

	ASSERT (FALSE);
	return NULL;

#else
	if (g_pShellManager == NULL)
	{
		InitShellManager ();
	}

	ASSERT_VALID (g_pShellManager);
	return g_pShellManager;
#endif
}
//*************************************************************************************
CBCGPTooltipManager* CBCGPWorkspace::GetTooltipManager()
{
	if (g_pTooltipManager == NULL)
	{
		InitTooltipManager ();
	}

	ASSERT_VALID (g_pTooltipManager);
	return g_pTooltipManager;
}
//**************************************************************************************
CBCGPContextMenuManager* CBCGPWorkspace::GetContextMenuManager()
{
	if (g_pContextMenuManager == NULL)
	{
		InitContextMenuManager();
	}

	ASSERT_VALID (g_pContextMenuManager);
	return g_pContextMenuManager;
}
//*************************************************************************************
CBCGPKeyboardManager* CBCGPWorkspace::GetKeyboardManager()
{
	if (g_pKeyboardManager == NULL)
	{
		InitKeyboardManager ();
	}

	ASSERT_VALID (g_pKeyboardManager);
	return g_pKeyboardManager;
}

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version

CBCGPSkinManager* CBCGPWorkspace::GetSkinManager()
{
	if (g_pSkinManager == NULL)
	{
		InitSkinManager ();
	}

	ASSERT_VALID (g_pSkinManager);
	return g_pSkinManager;
}

#endif

CBCGPUserToolsManager* CBCGPWorkspace::GetUserToolsManager()
{
	return g_pUserToolsManager;
}
//*************************************************************************************
CString	CBCGPWorkspace::GetRegSectionPath(LPCTSTR szSectionAdd /*=NULL*/)
{
	CString strSectionPath = ::BCGPGetRegPath (m_strRegSection);
	if (szSectionAdd != NULL && _tcslen (szSectionAdd) != 0)
	{
		strSectionPath += szSectionAdd;
		strSectionPath += _T("\\");
	}

	return strSectionPath;
}
//*************************************************************************************
BOOL CBCGPWorkspace::LoadState (LPCTSTR lpszSectionName /*=NULL*/, CBCGPFrameImpl* pFrameImpl /*= NULL*/)
{
	if (lpszSectionName != NULL)
	{
		m_strRegSection = lpszSectionName;
	}

	CString strSection = GetRegSectionPath ();

	//-----------------------------
	// Other things to do before ?:
	//-----------------------------
	PreLoadState();

	//------------------------
	// Loaded library version:
	//------------------------
	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (reg.Open (GetRegSectionPath (strRegEntryVersion)))
	{
		reg.Read (strVersionMajorKey, m_iSavedVersionMajor);
		reg.Read (strVersionMinorKey, m_iSavedVersionMinor);
	}

	//--------------------------------
	// Loaded "pinned" files/folders:
	//--------------------------------
	if (reg.Open (GetRegSectionPath (strRegEntryPinned)))
	{
		reg.Read (strPinnedFilesKey, m_arPinnedFiles);
		reg.Read (strPinnedFoldersKey, m_arPinnedFolders);
	}

	//--------------------------------------
	// Save general toolbar/menu parameters:
	//--------------------------------------
	CBCGPToolBar::LoadParameters (strSection);
	BCGPCMD_MGR.LoadState (strSection);

	BOOL bResetImages = FALSE;	// Reset images to default 

	if (m_bResourceSmartUpdate)
	{
		CBCGPToolbarButton::m_bUpdateImages = FALSE;
	}

	if (pFrameImpl != NULL) 
	{
		ASSERT_VALID(pFrameImpl->m_pFrame);

		if (m_bAutoMenuBar)
		{
			pFrameImpl->SetupAutoMenuBar();
		}

		//-----------------
		// Load input mode:
		//-----------------
		pFrameImpl->SetInputMode((BCGP_INPUT_MODE)GetInt(strInputMode, 0));

		//-------------------
		// Load rebars state:
		//-------------------
		CBCGPRebarState::LoadState (strSection, pFrameImpl->m_pFrame);

		BOOL	bPrevDisableRecalcLayout	= CBCGPDockManager::m_bDisableRecalcLayout;
		CBCGPDockManager::m_bDisableRecalcLayout = TRUE;

		//-----------------------------------------------------
		// Load all toolbars, menubar and docking control bars:
		//-----------------------------------------------------
		for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
		{
			CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (posTlb);
			ASSERT (pToolBar != NULL);

			if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID(pToolBar);

				if (!m_bLoadSaveFrameBarsOnly ||
					pToolBar->GetTopLevelFrame () == pFrameImpl->m_pFrame)
				{
					if (!pToolBar->IsFloating ())
					{
						pToolBar->LoadState (strSection);
						if (pToolBar->IsResourceChanged ())
						{
							bResetImages = TRUE;
						}
					}
				}
			}
		}

		//----------------------------
		// Load user defined toolbars:
		//----------------------------
		if (m_bLoadUserToolbars)
		{
			pFrameImpl->LoadUserToolbars ();
		}

		//------------------------
		// Load tear-off toolbars:
		//------------------------
		pFrameImpl->LoadTearOffMenus ();

		CBCGPDockManager::m_bDisableRecalcLayout = bPrevDisableRecalcLayout;

		CDockState dockState;

		if (m_bAfxStoreDockSate)
		{
			dockState.LoadState(m_strRegSection + strRegEntryNameControlBars);
		}

		if (m_bForceDockStateLoad || pFrameImpl->IsDockStateValid (dockState))
		{
			if ((GetDataVersionMajor() != -1) && (GetDataVersionMinor() != -1))
			{
				pFrameImpl->LoadDockState (strSection);
				pFrameImpl->SetDockState (dockState);
			}
		}

		if (m_bLoadWindowPlacement)
		{
			//--------------------------------------------------------
			// Set frame default (restored) size:
			//--------------------------------------------------------
			ReloadWindowPlacement (pFrameImpl->m_pFrame);
		}
	}

	//--------------------------------------
	// Load mouse/keyboard/menu managers:
	//--------------------------------------
	if (g_pMouseManager != NULL)
	{
		g_pMouseManager->LoadState (strSection);
	}

	if (g_pContextMenuManager != NULL)
	{
		g_pContextMenuManager->LoadState(strSection);
	}

	if (g_pKeyboardManager != NULL)
	{
		g_pKeyboardManager->LoadState (strSection,
			pFrameImpl == NULL ? NULL : pFrameImpl->m_pFrame);
	}

	if (g_pUserToolsManager != NULL)
	{
		g_pUserToolsManager->LoadState (strSection);

#ifndef BCGP_EXCLUDE_RIBBON
		if (pFrameImpl != NULL && pFrameImpl->GetRibbonBar() != NULL)
		{
			pFrameImpl->GetRibbonBar()->ForceRecalcLayout();	
		}
#endif
	}

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version
	if (g_pSkinManager != NULL)
	{
		g_pSkinManager->LoadState (strSection);
	}
#endif

	if (m_bResourceSmartUpdate)
	{
		CBCGPToolbarButton::m_bUpdateImages = TRUE;
	}

	if (m_bForceImageReset || (m_bResourceSmartUpdate && bResetImages))
	{
		for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
		{
			CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (posTlb);
			ASSERT (pToolBar != NULL);

			if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID (pToolBar);

				pToolBar->ResetImages ();
			}
		}

		if (pFrameImpl != NULL)
		{
			ASSERT_VALID (pFrameImpl->m_pFrame);
			pFrameImpl->m_pFrame->RecalcLayout ();
		}
	}

	//----------
	// Call Hook
	//----------
	LoadCustomState();

	//----------------------------------------------------------------------
	// To not confuse internal serialization, set version number to current:
	//----------------------------------------------------------------------
	m_iSavedVersionMajor = _BCGCBPRO_VERSION_MAJOR;
	m_iSavedVersionMinor = _BCGCBPRO_VERSION_MINOR;

	if (pFrameImpl != NULL)
	{
		ASSERT_VALID (pFrameImpl->m_pFrame);

		if (pFrameImpl->m_pFrame->IsZoomed ())
		{
			pFrameImpl->m_pFrame->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
		}
	}

	return TRUE;
}
//*************************************************************************************
BOOL CBCGPWorkspace::LoadState (CBCGPMDIFrameWnd* pFrame, LPCTSTR lpszSectionName /*=NULL*/)
{ 
	ASSERT_VALID (pFrame);
	return LoadState (lpszSectionName, &pFrame->m_Impl); 
}
//*************************************************************************************
BOOL CBCGPWorkspace::LoadState (CBCGPFrameWnd* pFrame, LPCTSTR lpszSectionName /*=NULL*/)
{ 
	ASSERT_VALID (pFrame);
	return LoadState (lpszSectionName, &pFrame->m_Impl);
}
//***********************************************************************************
BOOL CBCGPWorkspace::LoadState (CBCGPOleIPFrameWnd* pFrame, LPCTSTR lpszSectionName /*=NULL*/)
{ 
	ASSERT_VALID (pFrame);
	return LoadState (lpszSectionName, &pFrame->m_Impl);
}
//*************************************************************************************
BOOL CBCGPWorkspace::CleanState (LPCTSTR lpszSectionName /*=NULL*/)
{
	if (lpszSectionName != NULL)
	{
		m_strRegSection = lpszSectionName;
	}

	CString strSection = GetRegSectionPath ();

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	return reg.DeleteKey(strSection);
}
//*************************************************************************************
BOOL CBCGPWorkspace::SaveState (LPCTSTR lpszSectionName  /*=NULL*/, CBCGPFrameImpl* pFrameImpl /*= NULL*/)
{
	if (!m_bSaveState)
	{
		return FALSE;
	}

	if (lpszSectionName != NULL)
	{
		m_strRegSection = lpszSectionName;
	}

	CString strSection = GetRegSectionPath ();

	//-----------------------------
	// Other things to do before ?:
	//-----------------------------
	PreSaveState();

	//----------------------
	// Save library version:
	//----------------------
	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (GetRegSectionPath (strRegEntryVersion)))
	{
		reg.Write (strVersionMajorKey, _BCGCBPRO_VERSION_MAJOR);
		reg.Write (strVersionMinorKey, _BCGCBPRO_VERSION_MINOR);
	}

	if (reg.CreateKey(GetRegSectionPath (strRegEntryPinned)))
	{
		reg.Write (strPinnedFilesKey, m_arPinnedFiles);
		reg.Write (strPinnedFoldersKey, m_arPinnedFolders);
	}

	//--------------------------------------
	// Save general toolbar/menu parameters:
	//--------------------------------------
	CBCGPToolBar::SaveParameters (strSection);
	BCGPCMD_MGR.SaveState (strSection);

	if (pFrameImpl != NULL) 
	{
		if (m_bAfxStoreDockSate)
		{
			CDockState dockState;
			
			pFrameImpl->m_pFrame->GetDockState (dockState);
			dockState.SaveState (m_strRegSection + strRegEntryNameControlBars);
		}

		//-----------------
		// Save input mode:
		//-----------------
		WriteInt(strInputMode, pFrameImpl->m_InputMode);

		pFrameImpl->SaveDockState (strSection);

		//-----------------------------------------------------
		// Save all toolbars, menubar and docking control bars:
		//-----------------------------------------------------
		for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
		{
			CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (posTlb);
			ASSERT (pToolBar != NULL);

			if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
			{
				ASSERT_VALID(pToolBar);

				if (!m_bLoadSaveFrameBarsOnly ||
					pToolBar->GetTopLevelFrame () == pFrameImpl->m_pFrame)
				{
					pToolBar->SaveState (strSection);
				}
			}
		}

		//----------------------------
		// Save user defined toolbars:
		//----------------------------
		pFrameImpl->SaveUserToolbars (m_bLoadSaveFrameBarsOnly);

		//------------------------
		// Save tear-off toolbars:
		//------------------------
		pFrameImpl->SaveTearOffMenus (m_bLoadSaveFrameBarsOnly);

		//-------------------
		// Save rebars state:
		//-------------------
		CBCGPRebarState::SaveState (strSection, pFrameImpl->m_pFrame);

		//--------------------------
		// Store window placement
		//--------------------------
		pFrameImpl->StoreWindowPlacement ();
	}

	//------------------
	// Save user images:
	//------------------
	if (CBCGPToolBar::m_pUserImages != NULL)
	{
		ASSERT_VALID (CBCGPToolBar::m_pUserImages);
		CBCGPToolBar::m_pUserImages->Save ();
	}

	//--------------------------------------
	// Save mouse/keyboard/menu managers:
	//--------------------------------------
	if (g_pMouseManager != NULL)
	{
		g_pMouseManager->SaveState (strSection);
	}

	if (g_pContextMenuManager != NULL)
	{
		g_pContextMenuManager->SaveState (strSection);
	}

	if (g_pKeyboardManager != NULL)
	{
		g_pKeyboardManager->SaveState (strSection,
			pFrameImpl == NULL ? NULL : pFrameImpl->m_pFrame);
	}

	if (g_pUserToolsManager != NULL)
	{
		g_pUserToolsManager->SaveState (strSection);
	}

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version
	if (g_pSkinManager != NULL)
	{
		g_pSkinManager->SaveState (strSection);
	}
#endif

	SaveCustomState();
	return TRUE;
}

//*************************************************************************************
// Overidables for customization

void CBCGPWorkspace::OnClosingMainFrame (CBCGPFrameImpl* pFrame)
{
	// Defaults to automatically saving state.
	SaveState(0, pFrame);
}

//--------------------------------------------------------
// the next one have to be called explicitly in your code:
//--------------------------------------------------------
BOOL CBCGPWorkspace::OnViewDoubleClick (CWnd* pWnd, int iViewId)
{
	if (g_pMouseManager == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT_VALID (g_pMouseManager);

	UINT uiCmd = g_pMouseManager->GetViewDblClickCommand (iViewId);
	if (uiCmd > 0 && uiCmd != (UINT) -1)
	{
		if (g_pUserToolsManager != NULL &&
			g_pUserToolsManager->InvokeTool (uiCmd))
		{
			return TRUE;
		}

		CWnd* pTargetWnd = (pWnd == NULL) ? 
			AfxGetMainWnd () : 
			BCGCBProGetTopLevelFrame (pWnd);
		ASSERT_VALID (pTargetWnd);

		pTargetWnd->SendMessage (WM_COMMAND, uiCmd);
		return TRUE;
	}

	MessageBeep ((UINT) -1);
	return FALSE;
}
//***********************************************************************************
BOOL CBCGPWorkspace::ShowPopupMenu (UINT uiMenuResId, const CPoint& point, CWnd* pWnd)
{
	if (g_pContextMenuManager == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT_VALID (g_pContextMenuManager);
	return g_pContextMenuManager->ShowPopupMenu (uiMenuResId,
				point.x, point.y, pWnd);
}
//***********************************************************************************
BOOL CBCGPWorkspace::ReloadWindowPlacement (CFrameWnd* pFrameWnd)
{
	ASSERT_VALID (pFrameWnd);

	CCommandLineInfo cmdInfo;
	AfxGetApp ()->ParseCommandLine (cmdInfo);
	if (cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated)
	{
		//Don't show the main window if Application 
		//was run with /Embedding or /Automation.  
		return FALSE;
	}

	CRect rectNormal;
	int nFlags = 0;
	int nShowCmd = SW_SHOWNORMAL;

	if (LoadWindowPlacement (rectNormal, nFlags, nShowCmd))
	{
		WINDOWPLACEMENT wp;
		wp.length = sizeof (WINDOWPLACEMENT);

		if (pFrameWnd->GetWindowPlacement (&wp))
		{
			wp.rcNormalPosition = rectNormal;
			wp.showCmd = nShowCmd;

			RECT rectDesktop;
			SystemParametersInfo(SPI_GETWORKAREA,0,(PVOID)&rectDesktop,0);
			OffsetRect(&wp.rcNormalPosition, -rectDesktop.left, -rectDesktop.top);

			pFrameWnd->SetWindowPlacement (&wp);

			return TRUE;
		}
	}

	return FALSE;
}
//***********************************************************************************
BOOL CBCGPWorkspace::LoadWindowPlacement (
					CRect& rectNormalPosition, int& nFlags, int& nShowCmd)
{
	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (!reg.Open (GetRegSectionPath (strWindowPlacementRegSection)))
	{
		return FALSE;
	}

	return	reg.Read (strRectMainKey, rectNormalPosition) &&
			reg.Read (strFlagsKey, nFlags) &&
			reg.Read (strShowCmdKey, nShowCmd);
}
//***********************************************************************************
BOOL CBCGPWorkspace::StoreWindowPlacement (
					const CRect& rectNormalPosition, int nFlags, int nShowCmd)
{
	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (!reg.CreateKey (GetRegSectionPath (strWindowPlacementRegSection)))
	{
		return FALSE;
	}

	return	reg.Write (strRectMainKey, rectNormalPosition) &&
			reg.Write (strFlagsKey, nFlags) &&
			reg.Write (strShowCmdKey, nShowCmd);
}
//*************************************************************************************
//*************************************************************************************
// These functions load and store values from the "Custom" subkey
// To use subkeys of the "Custom" subkey use GetSectionInt() etc.
// instead
int CBCGPWorkspace::GetInt(LPCTSTR lpszEntry, int nDefault /*= 0*/)
{
	return GetSectionInt(_T(""), lpszEntry, nDefault);
}
//*************************************************************************************
double CBCGPWorkspace::GetDouble(LPCTSTR lpszEntry, double dblDefault /*= 0.0*/)
{
	return GetSectionDouble(_T(""), lpszEntry, dblDefault);
}
//*************************************************************************************
CString	CBCGPWorkspace::GetString(LPCTSTR lpszEntry, LPCTSTR lpszDefault /*= ""*/)
{
	return GetSectionString(_T(""), lpszEntry, lpszDefault);
}
//*************************************************************************************
BOOL CBCGPWorkspace::GetBinary(LPCTSTR lpszEntry, LPBYTE* ppData, UINT* pBytes)
{
	return GetSectionBinary(_T(""), lpszEntry, ppData, pBytes);
}
//*************************************************************************************
BOOL CBCGPWorkspace::GetObject(LPCTSTR lpszEntry, CObject& obj)
{
	return GetSectionObject(_T(""), lpszEntry, obj);
}
//*************************************************************************************
BOOL CBCGPWorkspace::WriteInt(LPCTSTR lpszEntry, int nValue )
{
	return WriteSectionInt(_T(""), lpszEntry, nValue);
}
//*************************************************************************************
BOOL CBCGPWorkspace::WriteDouble(LPCTSTR lpszEntry, double dblValue )
{
	return WriteSectionDouble(_T(""), lpszEntry, dblValue);
}
//*************************************************************************************
BOOL CBCGPWorkspace::WriteString(LPCTSTR lpszEntry, LPCTSTR lpszValue )
{
	return WriteSectionString(_T(""), lpszEntry, lpszValue);
}
//*************************************************************************************
BOOL CBCGPWorkspace::WriteBinary(LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes)
{
	return WriteSectionBinary(_T(""), lpszEntry, pData, nBytes);
}
//*************************************************************************************
BOOL CBCGPWorkspace::WriteObject(LPCTSTR lpszEntry, CObject& obj)
{
	return WriteSectionObject(_T(""), lpszEntry, obj);
}
//*************************************************************************************
//*************************************************************************************
// These functions load and store values from a given subkey
// of the "Custom" subkey. For simpler access you may use
// GetInt() etc.
int CBCGPWorkspace::GetSectionInt( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, int nDefault /*= 0*/)
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	
	int nRet = nDefault;

	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (reg.Open (strSection))
	{
		reg.Read (lpszEntry, nRet);
	}
	return nRet;
}
//*************************************************************************************
double CBCGPWorkspace::GetSectionDouble( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, double dblDefault /*= 0.0*/)
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	
	double dblRet = dblDefault;

	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (reg.Open (strSection))
	{
		reg.Read (lpszEntry, dblRet);
	}
	return dblRet;
}
//*************************************************************************************
CString CBCGPWorkspace::GetSectionString( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault /*= ""*/)
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	ASSERT(lpszDefault);
	
	CString strRet = lpszDefault;

	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (reg.Open (strSection))
	{
		if (!reg.Read (lpszEntry, strRet))
		{
			strRet = lpszDefault;
		}
	}

	return strRet;
}
//*************************************************************************************
BOOL CBCGPWorkspace::GetSectionBinary(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPBYTE* ppData, UINT* pBytes)
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	ASSERT(ppData);
	
	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (reg.Open (strSection) 
		&& reg.Read (lpszEntry, ppData, pBytes) ) 
	{
		return TRUE;
	}
	return FALSE;
}
//*************************************************************************************
BOOL CBCGPWorkspace::GetSectionObject(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, CObject& obj)
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	ASSERT_VALID(&obj);
	
	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	if (reg.Open (strSection) && reg.Read (lpszEntry, obj)) 
	{
		return TRUE;
	}
	return FALSE;
}
//*************************************************************************************
BOOL CBCGPWorkspace::WriteSectionInt( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, int nValue )
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	
	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		return reg.Write (lpszEntry, nValue);
	}
	return FALSE;
}
//*************************************************************************************
BOOL CBCGPWorkspace::WriteSectionDouble( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, double dblValue )
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	
	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		return reg.Write (lpszEntry, dblValue);
	}
	return FALSE;
}
//*************************************************************************************
BOOL CBCGPWorkspace::WriteSectionString( LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPCTSTR lpszValue )
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	ASSERT(lpszValue);

	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		return reg.Write (lpszEntry, lpszValue);
	}
	return FALSE;
}
//*************************************************************************************
BOOL CBCGPWorkspace::WriteSectionBinary(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes)
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	ASSERT(pData);

	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		return reg.Write (lpszEntry, pData, nBytes);
	}
	return FALSE;
}
//*************************************************************************************
BOOL CBCGPWorkspace::WriteSectionObject(LPCTSTR lpszSubSection, LPCTSTR lpszEntry, CObject& obj)
{
	ASSERT(lpszSubSection);
	ASSERT(lpszEntry);
	ASSERT_VALID(&obj);

	CString strSection = GetRegSectionPath(lpszSubSection);

	CBCGPRegistrySP regSP;
	CBCGPRegistry& reg = regSP.Create (FALSE, FALSE);

	if (reg.CreateKey (strSection))
	{
		return reg.Write (lpszEntry, obj);
	}

	return FALSE;
}
//**********************************************************************************
void CBCGPWorkspace::OnAppContextHelp (CWnd* pWndControl, const DWORD dwHelpIDArray [])
{
	::WinHelp (pWndControl->GetSafeHwnd (),
				AfxGetApp()->m_pszHelpFilePath, 
				HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID) dwHelpIDArray);
}
//*************************************************************************************
BOOL CBCGPWorkspace::SaveState (CBCGPMDIFrameWnd* pFrame, LPCTSTR
							   lpszSectionName /*=NULL*/)
{ 
	ASSERT_VALID (pFrame);
	return SaveState (lpszSectionName, &pFrame->m_Impl); 
}
//*************************************************************************************
BOOL CBCGPWorkspace::SaveState (CBCGPFrameWnd* pFrame, LPCTSTR lpszSectionName
							   /*=NULL*/)
{ 
	ASSERT_VALID (pFrame);
	return SaveState (lpszSectionName, &pFrame->m_Impl);
}
//***********************************************************************************
BOOL CBCGPWorkspace::SaveState (CBCGPOleIPFrameWnd* pFrame, LPCTSTR
							   lpszSectionName /*=NULL*/)
{ 
	ASSERT_VALID (pFrame);
	return SaveState (lpszSectionName, &pFrame->m_Impl);
}
//***********************************************************************************
BOOL CBCGPWorkspace::IsStateExists(LPCTSTR lpszSectionName /*=NULL*/)
{
	 if (lpszSectionName != NULL)
	 {
		m_strRegSection = lpszSectionName;
	 }

	 CString strSection = GetRegSectionPath ();

	//------------------------
	// Loaded library version:
	//------------------------
	 CBCGPRegistrySP regSP;
	 CBCGPRegistry& reg = regSP.Create (FALSE, TRUE);

	 return reg.Open (GetRegSectionPath (strRegEntryVersion));
}
//***********************************************************************************
int CBCGPWorkspace::GetDataVersion () const
{
	if (m_iSavedVersionMajor == -1 || m_iSavedVersionMinor == -1)
	{
		return 0xFFFFFFFF;
	}

	int nVersionMinor = m_iSavedVersionMinor / 10;
	int nVersionDigit = m_iSavedVersionMinor % 10;
	
	nVersionMinor *= 0x100;
	nVersionDigit *= 0x10;

	if (nVersionMinor < 10)
	{
		nVersionDigit *=0x10;
	}

	return m_iSavedVersionMajor * 0x10000 + nVersionMinor + nVersionDigit;
}

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version

void CBCGPWorkspace::OnSelectSkin ()
{
	CFrameWnd* pMainWnd = DYNAMIC_DOWNCAST (CFrameWnd, AfxGetMainWnd ());
	if (pMainWnd != NULL)
	{
		pMainWnd->RecalcLayout ();
	}

	for (POSITION posTlb = gAllToolbars.GetHeadPosition (); posTlb != NULL;)
	{
		CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (posTlb);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);
			pToolBar->AdjustLayout ();
		}
	}

	CBCGPVisualManager::GetInstance ()->RedrawAll ();
}

#endif

void CBCGPWorkspace::EnableTaskBarInteraction(BOOL bEnable)
{
	m_bTaskBarInteraction = bEnable;

	CBCGPMDIFrameWnd* pMDIMainFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, AfxGetMainWnd ());
	if (pMDIMainFrame != NULL)
	{
		ASSERT_VALID(pMDIMainFrame);
		pMDIMainFrame->RegisterAllMDIChildrenWithTaskbar(bEnable);
	}
}
//***********************************************************************************
const CStringArray& CBCGPWorkspace::GetPinnedPaths(BOOL bFile)
{
	return bFile ? m_arPinnedFiles : m_arPinnedFolders;
}
//***********************************************************************************
BOOL CBCGPWorkspace::IsPinned(BOOL bFile, LPCTSTR lpszPath)
{
	ASSERT(lpszPath != NULL);

	CString strPath = lpszPath;
	strPath.MakeUpper();

	CStringArray& ar = bFile ? m_arPinnedFiles : m_arPinnedFolders;

	for (int i = 0; i < (int)ar.GetSize(); i++)
	{
		CString str = ar[i];
		str.MakeUpper();

		if (str == strPath)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//***********************************************************************************
void CBCGPWorkspace::PinPath(BOOL bFile, LPCTSTR lpszPath, BOOL bPin)
{
	ASSERT(lpszPath != NULL);

	CString strPath = lpszPath;
	if (strPath.IsEmpty())
	{
		return;
	}

	CStringArray& ar = bFile ? m_arPinnedFiles : m_arPinnedFolders;

	if (bPin)
	{
		if (!IsPinned(bFile, lpszPath))
		{
			ar.Add(lpszPath);
		}
	}
	else
	{
		strPath.MakeUpper();

		for (int i = 0; i < (int)ar.GetSize(); i++)
		{
			CString str = ar[i];
			str.MakeUpper();

			if (str == strPath)
			{
				ar.RemoveAt(i);
				return;
			}
		}
	}
}
//***********************************************************************************
BOOL CBCGPWorkspace::CreateScreenshot(CBitmap& bmpScreenshot, CWnd* pWnd)
{
	if (bmpScreenshot.GetSafeHandle() != NULL)
	{
		bmpScreenshot.DeleteObject();
	}

	if (pWnd == NULL)
	{
		pWnd = AfxGetMainWnd();
	}

	if (pWnd->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}
	
	CRect rectWnd;
	pWnd->GetWindowRect(rectWnd);
	rectWnd.OffsetRect(-rectWnd.left, -rectWnd.top);

	BOOL bRes = FALSE;
	
	CDC dc;
	if (dc.CreateCompatibleDC(NULL))
	{
		LPBYTE pBitsWnd = NULL;
		HBITMAP hBitmap = CBCGPDrawManager::CreateBitmap_32 (rectWnd.Size(), (void**)&pBitsWnd);

		if (hBitmap != NULL)
		{
			HBITMAP hBitmapOld = (HBITMAP)dc.SelectObject(hBitmap);
			if (hBitmapOld != NULL)
			{
				dc.FillRect(rectWnd, &globalData.brWindow);
				pWnd->SendMessage(WM_PRINT, (WPARAM)dc.GetSafeHdc(), (LPARAM)(PRF_CLIENT | PRF_CHILDREN | PRF_NONCLIENT | PRF_ERASEBKGND));
	
				for (int i = 0; i < rectWnd.Width() * rectWnd.Height(); i++)
				{
					pBitsWnd[3] = 255;
					pBitsWnd += 4;
				}
	
				dc.SelectObject (hBitmapOld);

				bRes = TRUE;
			}
	
			bmpScreenshot.Attach(hBitmap);
		}
	}

	return bRes;
}
