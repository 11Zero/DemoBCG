// ColorPopup.cpp : implementation file
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
#include "BCGPColorMenuButton.h"
#include "BCGPColorButton.h"
#include "BCGPPropList.h"
#include "BCGPControlBar.h"
#include "ColorPopup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorPopup

IMPLEMENT_DYNAMIC(CColorPopup, CBCGPPopupMenu)

BEGIN_MESSAGE_MAP(CColorPopup, CBCGPPopupMenu)
	//{{AFX_MSG_MAP(CColorPopup)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CColorPopup::~CColorPopup()
{
}

/////////////////////////////////////////////////////////////////////////////
// CColorPopup message handlers

int CColorPopup::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPToolBar::IsCustomizeMode () && !m_bEnabledInCustomizeMode)
	{
		// Don't show color popup in cistomization mode
		return -1;
	}

	if (CMiniFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	DWORD toolbarStyle = dwDefaultToolbarStyle;
	if (GetAnimationType () != NO_ANIMATION && !CBCGPToolBar::IsCustomizeMode ())
	{
		toolbarStyle &= ~WS_VISIBLE;
	}

	if (!m_wndColorBar.Create (this, toolbarStyle | CBRS_TOOLTIPS | CBRS_FLYBY, 1))
	{
		TRACE(_T ("Can't create popup menu bar\n"));
		return -1;
	}

	CWnd* pWndParent = GetParent ();
	ASSERT_VALID (pWndParent);

	m_wndColorBar.SetOwner (pWndParent);
	m_wndColorBar.SetBarStyle(m_wndColorBar.GetBarStyle() | CBRS_TOOLTIPS);

	ActivatePopupMenu (BCGCBProGetTopLevelFrame (pWndParent), this);
	RecalcLayout ();
	return 0;
}
//*************************************************************************************
CBCGPControlBar* CColorPopup::CreateTearOffBar (CFrameWnd* pWndMain, UINT uiID, LPCTSTR lpszName)
{
	ASSERT_VALID (pWndMain);
	ASSERT (lpszName != NULL);
	ASSERT (uiID != 0);

	CBCGPColorMenuButton* pColorMenuButton = 
		DYNAMIC_DOWNCAST (CBCGPColorMenuButton, GetParentButton ());
	if (pColorMenuButton == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	CBCGPColorBar* pColorBar = new CBCGPColorBar (m_wndColorBar, pColorMenuButton->m_nID);

	if (!pColorBar->Create (pWndMain,
		dwDefaultToolbarStyle,
		uiID))
	{
		TRACE0 ("Failed to create a new toolbar!\n");
		delete pColorBar;
		return NULL;
	}

	pColorBar->SetWindowText (lpszName);
	pColorBar->SetBarStyle (pColorBar->GetBarStyle () |
		CBRS_TOOLTIPS | CBRS_FLYBY);
	pColorBar->EnableDocking (CBRS_ALIGN_ANY);

	return pColorBar;
}
//**************************************************************************
CWnd* CColorPopup::GetParentArea (CRect& rectParentBtn)
{
	ASSERT_VALID (this);

	CBCGPColorButton* pParentButton = m_wndColorBar.m_pParentBtn;
	if (pParentButton->GetSafeHwnd() != NULL)
	{
		pParentButton->GetClientRect(rectParentBtn);
		return pParentButton;
	}

#ifndef BCGP_EXCLUDE_PROP_LIST
	if (m_wndColorBar.m_pWndPropList->GetSafeHwnd() != NULL)
	{
		CBCGPProp* pSelProp = m_wndColorBar.m_pWndPropList->GetCurSel();
		if (pSelProp != NULL)
		{
			rectParentBtn = pSelProp->GetRect();
			return m_wndColorBar.m_pWndPropList;
		}
	}
#endif

	return CBCGPPopupMenu::GetParentArea (rectParentBtn);
}
