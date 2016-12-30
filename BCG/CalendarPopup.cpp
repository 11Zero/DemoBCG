// CalendarPopup.cpp : implementation file
//
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#include "stdafx.h"

#include "BCGCBPro.h"
#include "BCGPCalendarMenuButton.h"

#ifndef _BCGSUITE_
#include "BCGPControlBar.h"
#endif

#include "CalendarPopup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCalendarPopup

IMPLEMENT_DYNAMIC(CCalendarPopup, CBCGPPopupMenu)

BEGIN_MESSAGE_MAP(CCalendarPopup, CBCGPPopupMenu)
	//{{AFX_MSG_MAP(CCalendarPopup)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CCalendarPopup::~CCalendarPopup()
{
}

BOOL CCalendarPopup::Create (CWnd* pWndParent, int x, int y, HMENU hMenu, BOOL /*bLocked*/, BOOL bOwnMessage)
{
	return CBCGPPopupMenu::Create (pWndParent, x, y, hMenu, TRUE, bOwnMessage);
}

/////////////////////////////////////////////////////////////////////////////
// CCalendarPopup message handlers

int CCalendarPopup::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPToolBar::IsCustomizeMode () && !m_bEnabledInCustomizeMode)
	{
		// Don't show calendar popup in cistomization mode
		return -1;
	}

	if (CMiniFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	DWORD toolbarStyle = dwDefaultToolbarStyle;
	if (GetAnimationType () != NO_ANIMATION && !CBCGPToolBar::IsCustomizeMode ())
	{
		toolbarStyle &= ~WS_VISIBLE;
	}

	m_wndCalendarBar.SetState (CBCGPCalendarBar::CBR_ENABLED | CBCGPCalendarBar::CBR_NAVIGATION_BUTTONS, 0xFFFFFFFF);

	if (!m_wndCalendarBar.Create (this, toolbarStyle | CBRS_TOOLTIPS | CBRS_FLYBY, 1, false))
	{
		TRACE(_T ("Can't create popup menu bar\n"));
		return -1;
	}


	CWnd* pWndParent = GetParent ();
	ASSERT_VALID (pWndParent);

	m_wndCalendarBar.SetOwner (pWndParent);
	m_wndCalendarBar.SetBarStyle(m_wndCalendarBar.GetBarStyle() | CBRS_TOOLTIPS);

	ActivatePopupMenu (BCGCBProGetTopLevelFrame (pWndParent), this);
	RecalcLayout ();

	return 0;
}
//*************************************************************************************
CBCGPControlBar* CCalendarPopup::CreateTearOffBar (CFrameWnd* pWndMain, UINT uiID, LPCTSTR lpszName)
{
	ASSERT_VALID (pWndMain);
	ASSERT (lpszName != NULL);
	ASSERT (uiID != 0);

	CBCGPCalendarMenuButton* pCalendarMenuButton = 
		DYNAMIC_DOWNCAST (CBCGPCalendarMenuButton, GetParentButton ());
	if (pCalendarMenuButton == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	CBCGPCalendarBar* pCalendarBar = new CBCGPCalendarBar (m_wndCalendarBar, pCalendarMenuButton->m_nID, false);

	if (!pCalendarBar->Create (pWndMain,
		dwDefaultToolbarStyle,
		uiID, false))
	{
		TRACE0 ("Failed to create a new toolbar!\n");
		delete pCalendarBar;
		return NULL;
	}

	pCalendarBar->SetWindowText (lpszName);
	pCalendarBar->SetBarStyle (pCalendarBar->GetBarStyle () |
		CBRS_TOOLTIPS | CBRS_FLYBY);
	pCalendarBar->EnableDocking (CBRS_ALIGN_ANY);

	return pCalendarBar;
}

