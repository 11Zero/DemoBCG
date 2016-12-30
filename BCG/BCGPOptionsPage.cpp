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

// OptionsPage.cpp : implementation file
//

#include "stdafx.h"

#include "bcgprores.h"
#include "BCGCBPro.h"
#include "BCGPOptionsPage.h"
#include "BCGPToolBar.h"
#include "BCGPMenuBar.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPFrameWnd.h"
#include "BCGPLocalResource.h"
#include "BCGPToolbarCustomize.h"
#include "BCGPVisualManager.h"
#include "BCGPSkinManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPOptionsPage property page

IMPLEMENT_DYNCREATE(CBCGPOptionsPage, CPropertyPage)

CBCGPOptionsPage::CBCGPOptionsPage(BOOL bIsMenuBarExist) : 
	CPropertyPage(CBCGPOptionsPage::IDD),
	m_bIsMenuBarExist (bIsMenuBarExist)
{
	//{{AFX_DATA_INIT(CBCGPOptionsPage)
	m_bShowTooltips = CBCGPToolBar::m_bShowTooltips;
	m_bShowShortcutKeys = CBCGPToolBar::m_bShowShortcutKeys;
	m_bRecentlyUsedMenus = CBCGPMenuBar::m_bRecentlyUsedMenus;
	m_bShowAllMenusDelay = CBCGPMenuBar::m_bShowAllMenusDelay;
	m_bLargeIcons = CBCGPToolBar::m_bLargeIcons;
	m_bLook2000 = CBCGPVisualManager::GetInstance ()->IsLook2000 ();
	//}}AFX_DATA_INIT

}

CBCGPOptionsPage::~CBCGPOptionsPage()
{
}

void CBCGPOptionsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPOptionsPage)
	DDX_Control(pDX, IDC_BCGBARRES_SKINS, m_wndSkinsBtn);
	DDX_Control(pDX, IDC_BCGBARRES_LOOK2000, m_wndLook2000);
	DDX_Control(pDX, IDC_BCGBARRES_LARGE_ICONS, m_wndLargeIcons);
	DDX_Control(pDX, IDC_BCGBARRES_SHOW_RECENTLY_USED_MENUS, m_wndRUMenus);
	DDX_Control(pDX, IDC_BCGBARRES_RESET_USAGE_DATA, m_wndResetUsageBtn);
	DDX_Control(pDX, IDC_RU_MENUS_TITLE, m_wndRuMenusLine);
	DDX_Control(pDX, IDC_RU_MENUS_LINE, m_wndRuMenusTitle);
	DDX_Control(pDX, IDC_BCGBARRES_SHOW_MENUS_DELAY, m_wndShowAllMenusDelay);
	DDX_Control(pDX, IDC_BCGBARRES_SHOW_TOOLTIPS_WITH_KEYS, m_wndShowShortcutKeys);
	DDX_Check(pDX, IDC_BCGBARRES_SHOW_TOOLTIPS, m_bShowTooltips);
	DDX_Check(pDX, IDC_BCGBARRES_SHOW_TOOLTIPS_WITH_KEYS, m_bShowShortcutKeys);
	DDX_Check(pDX, IDC_BCGBARRES_SHOW_RECENTLY_USED_MENUS, m_bRecentlyUsedMenus);
	DDX_Check(pDX, IDC_BCGBARRES_SHOW_MENUS_DELAY, m_bShowAllMenusDelay);
	DDX_Check(pDX, IDC_BCGBARRES_LARGE_ICONS, m_bLargeIcons);
	DDX_Check(pDX, IDC_BCGBARRES_LOOK2000, m_bLook2000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGPOptionsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CBCGPOptionsPage)
	ON_BN_CLICKED(IDC_BCGBARRES_SHOW_TOOLTIPS_WITH_KEYS, OShowTooltipsWithKeys)
	ON_BN_CLICKED(IDC_BCGBARRES_SHOW_TOOLTIPS, OnShowTooltips)
	ON_BN_CLICKED(IDC_BCGBARRES_RESET_USAGE_DATA, OnResetUsageData)
	ON_BN_CLICKED(IDC_BCGBARRES_SHOW_RECENTLY_USED_MENUS, OnShowRecentlyUsedMenus)
	ON_BN_CLICKED(IDC_BCGBARRES_SHOW_MENUS_DELAY, OnShowMenusDelay)
	ON_BN_CLICKED(IDC_BCGBARRES_LARGE_ICONS, OnLargeIcons)
	ON_BN_CLICKED(IDC_BCGBARRES_LOOK2000, OnBcgbarresLook2000)
	ON_BN_CLICKED(IDC_BCGBARRES_SKINS, OnBcgbarresSkins)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPOptionsPage message handlers

BOOL CBCGPOptionsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_wndShowShortcutKeys.EnableWindow (m_bShowTooltips);
	m_wndShowAllMenusDelay.EnableWindow (m_bRecentlyUsedMenus);

	if (CBCGPToolBar::m_lstBasicCommands.IsEmpty () || !m_bIsMenuBarExist)
	{
		m_wndRUMenus.ShowWindow (SW_HIDE);
		m_wndRUMenus.EnableWindow (FALSE);

		m_wndResetUsageBtn.ShowWindow (SW_HIDE);
		m_wndResetUsageBtn.EnableWindow (FALSE);

		m_wndRuMenusLine.ShowWindow (SW_HIDE);
		m_wndRuMenusLine.EnableWindow (FALSE);

		m_wndRuMenusTitle.ShowWindow (SW_HIDE);
		m_wndRuMenusTitle.EnableWindow (FALSE);

		m_wndShowAllMenusDelay.ShowWindow (SW_HIDE);
		m_wndShowAllMenusDelay.EnableWindow (FALSE);
	}

	CBCGPToolbarCustomize* pWndParent = DYNAMIC_DOWNCAST (CBCGPToolbarCustomize, GetParent ());
	ASSERT (pWndParent != NULL);

	if ((pWndParent->GetFlags () & BCGCUSTOMIZE_LOOK_2000) == 0)
	{
		m_wndLook2000.ShowWindow (SW_HIDE);
		m_wndLook2000.EnableWindow (FALSE);
	}

	if (pWndParent->GetFlags () & BCGCUSTOMIZE_NO_LARGE_ICONS)
	{
		m_wndLargeIcons.ShowWindow (SW_HIDE);
		m_wndLargeIcons.EnableWindow (FALSE);
		m_bLargeIcons = FALSE;
	}

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version
	if ((pWndParent->GetFlags () & BCGCUSTOMIZE_SELECT_SKINS) == 0)
	{
		m_wndSkinsBtn.ShowWindow (SW_HIDE);
		m_wndSkinsBtn.EnableWindow (FALSE);
	}
	else
	{
		ASSERT (g_pSkinManager != NULL);
	}
#else
	m_wndSkinsBtn.ShowWindow (SW_HIDE);
	m_wndSkinsBtn.EnableWindow (FALSE);
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//*******************************************************************************
void CBCGPOptionsPage::OShowTooltipsWithKeys() 
{
	UpdateData ();
	CBCGPToolBar::m_bShowShortcutKeys = m_bShowShortcutKeys;
}
//*******************************************************************************
void CBCGPOptionsPage::OnShowTooltips() 
{
	UpdateData ();

	CBCGPToolBar::m_bShowTooltips = m_bShowTooltips;
	m_wndShowShortcutKeys.EnableWindow (m_bShowTooltips);
}
//******************************************************************************
void CBCGPOptionsPage::OnResetUsageData() 
{
	CBCGPLocalResource locaRes;

	CString strMsg;
	strMsg.LoadString (IDS_BCGBARRES_RESET_USAGE_WARNING);

	if (MessageBox (strMsg, NULL, MB_YESNO) == IDYES)
	{
		CBCGPToolBar::m_UsageCount.Reset ();
	}
}
//*******************************************************************************
void CBCGPOptionsPage::OnShowRecentlyUsedMenus() 
{
	UpdateData ();
	m_wndShowAllMenusDelay.EnableWindow (m_bRecentlyUsedMenus);

	CBCGPMenuBar::m_bRecentlyUsedMenus = m_bRecentlyUsedMenus;
}
//*******************************************************************************
void CBCGPOptionsPage::OnShowMenusDelay() 
{
	UpdateData ();
	CBCGPMenuBar::m_bShowAllMenusDelay = m_bShowAllMenusDelay;
}
//*******************************************************************************
void CBCGPOptionsPage::OnLargeIcons() 
{
	UpdateData ();
	CBCGPToolBar::SetLargeIcons (m_bLargeIcons);
}
//*******************************************************************************
void CBCGPOptionsPage::OnBcgbarresLook2000() 
{
	UpdateData ();
	CBCGPVisualManager::GetInstance ()->SetLook2000 (m_bLook2000);
	AfxGetMainWnd()->Invalidate();
}
//********************************************************************************
void CBCGPOptionsPage::OnBcgbarresSkins() 
{
#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version
	ASSERT (g_pSkinManager != NULL);
	g_pSkinManager->ShowSelectSkinDlg ();
#else
	ASSERT (FALSE);
#endif
}
