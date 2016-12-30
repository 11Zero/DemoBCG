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
// BCGPTabbedControlBar.cpp : implementation file
//

#include "stdafx.h"

#include "BCGPSlider.h"
#include "BCGPTabbedControlBar.h"
#include "BCGPMiniFrameWnd.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CBCGPTabWnd::Style CBCGPTabbedControlBar::m_StyleTabWnd = CBCGPTabWnd::STYLE_3D;
CArray<COLORREF, COLORREF> CBCGPTabbedControlBar::m_arTabsAutoColors;
BOOL CBCGPTabbedControlBar::m_bIsTabsAutoColor = FALSE;
CList<HWND,HWND> CBCGPTabbedControlBar::m_lstTabbedControlBars;

BOOL			CBCGPTabbedControlBar::m_bTabsAlwaysTop = FALSE;
BOOL			CBCGPTabbedControlBar::m_bDestroyUnused = TRUE;
CRuntimeClass*	CBCGPTabbedControlBar::m_pTabWndRTC = RUNTIME_CLASS (CBCGPTabWnd);

IMPLEMENT_SERIAL(CBCGPTabbedControlBar, CBCGPBaseTabbedBar, VERSIONABLE_SCHEMA | 2)

/////////////////////////////////////////////////////////////////////////////
// CBCGPTabbedControlBar

CBCGPTabbedControlBar::CBCGPTabbedControlBar (BOOL bAutoDestroy) : CBCGPBaseTabbedBar (bAutoDestroy)
{
}

CBCGPTabbedControlBar::~CBCGPTabbedControlBar()
{
}

BEGIN_MESSAGE_MAP(CBCGPTabbedControlBar, CBCGPBaseTabbedBar)
	//{{AFX_MSG_MAP(CBCGPTabbedControlBar)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPTabbedControlBar message handlers
//*******************************************************************************
int CBCGPTabbedControlBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	ASSERT_VALID (this);
	if (CBCGPBaseTabbedBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rectClient (0, 0, lpCreateStruct->cx, lpCreateStruct->cy);

	ASSERT (m_pTabWnd == NULL);
	ASSERT (m_pTabWndRTC != NULL);

	m_pTabWnd = DYNAMIC_DOWNCAST (CBCGPTabWnd, m_pTabWndRTC->CreateObject ());

	if (m_pTabWnd == NULL)
	{
		TRACE0("Failed to dynamically inatantiate a tab window object\n");
		return -1;      // fail to create
	}

	CBCGPTabWnd* pTabWnd = (CBCGPTabWnd*) m_pTabWnd;

	// Create tabs window:
	if (!pTabWnd->Create (m_StyleTabWnd, rectClient, this, 101, 
		CBCGPTabbedControlBar::m_bTabsAlwaysTop ? CBCGPTabWnd::LOCATION_TOP : CBCGPTabWnd::LOCATION_BOTTOM))
	{
		TRACE0("Failed to create tab window\n");
		delete m_pTabWnd;
		m_pTabWnd = NULL;
		return -1;      // fail to create
	}

	m_pTabWnd->m_bActivateTabOnRightClick = TRUE;

	if (m_bIsTabsAutoColor)
	{
		pTabWnd->EnableAutoColor ();
		pTabWnd->SetAutoColors (m_arTabsAutoColors);
	}

	pTabWnd->AutoDestroyWindow (FALSE);
	pTabWnd->HideSingleTab ();

	pTabWnd->SetTabBorderSize (CBCGPVisualManager::GetInstance ()->GetDockingTabsBordersSize ());
	pTabWnd->SetDrawNameInUpperCase(CBCGPVisualManager::GetInstance ()->IsDockingTabUpperCase());

	pTabWnd->m_bEnableWrapping = TRUE;

	m_lstTabbedControlBars.AddTail (GetSafeHwnd ());
	return 0;
}
//*******************************************************************************
BOOL CBCGPTabbedControlBar::FloatTab (CWnd* pBar, int nTabID, 
									  BCGP_DOCK_METHOD dockMethod, 
									  BOOL bHide)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	if (GetTabWnd ()->GetTabsNum () > 1)
	{
		return CBCGPBaseTabbedBar::FloatTab (pBar, nTabID, dockMethod, bHide);
	}

	return FALSE;
}
//*******************************************************************************
BOOL CBCGPTabbedControlBar::DetachControlBar (CWnd* pBar, BOOL bHide)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pBar);

	if (GetTabWnd ()->GetTabsNum () > 0)
	{
		return CBCGPBaseTabbedBar::DetachControlBar (pBar, bHide);
	}
	
	return FALSE;
}

//**************************************************************************************
BOOL CBCGPTabbedControlBar::CheckTabbedBarAlignment ()
{
	return TRUE;
}
//**************************************************************************************
void CBCGPTabbedControlBar::GetTabArea (CRect& rectTabAreaTop, 
										CRect& rectTabAreaBottom) const
{
	rectTabAreaTop.SetRectEmpty ();
	rectTabAreaBottom.SetRectEmpty ();

	if (IsTabLocationBottom ())
	{
		GetTabWnd ()->GetTabsRect (rectTabAreaBottom);
		GetTabWnd ()->ClientToScreen (rectTabAreaBottom);
	}
	else
	{
		GetTabWnd ()->GetTabsRect (rectTabAreaTop);
		GetTabWnd ()->ClientToScreen (rectTabAreaTop);
	}
}
//**************************************************************************************
BOOL CBCGPTabbedControlBar::IsTabLocationBottom () const
{
	return (GetTabWnd ()->GetLocation () == CBCGPTabWnd::LOCATION_BOTTOM);
}
//**************************************************************************************
void CBCGPTabbedControlBar::OnPressCloseButton ()
{
	if (m_pTabWnd == NULL)
	{
		return;
	}

	CWnd* pWnd = m_pTabWnd->GetActiveWnd ();

	CFrameWnd* pParentFrame = DYNAMIC_DOWNCAST (CFrameWnd, BCGPGetParentFrame (this));
	ASSERT_VALID (pParentFrame);

	if (pParentFrame != NULL)
	{
		if (pParentFrame->SendMessage (BCGM_ON_PRESS_CLOSE_BUTTON, NULL, (LPARAM) (LPVOID) pWnd))
		{
			return;
		}
	}

	int nVisibleTabNum = m_pTabWnd->GetVisibleTabsNum ();

	if (nVisibleTabNum == 1)
	{
		CBCGPDockingControlBar::OnPressCloseButton ();
	}
	
	int nActiveTab = m_pTabWnd->GetActiveTab ();
	m_pTabWnd->ShowTab (nActiveTab, FALSE);
}
//**********************************************************************************
void CBCGPTabbedControlBar::EnableTabAutoColor (BOOL bEnable/* = TRUE*/)
{
	m_bIsTabsAutoColor = bEnable;
	ResetTabs ();
}
//*********************************************************************************
void CBCGPTabbedControlBar::SetTabAutoColors (const CArray<COLORREF, COLORREF>& arColors)
{
	m_arTabsAutoColors.RemoveAll ();

	for (int i = 0; i < arColors.GetSize (); i++)
	{
		m_arTabsAutoColors.Add (arColors [i]);
	}

	ResetTabs ();
}
//***********************************************************************************
void CBCGPTabbedControlBar::OnDestroy() 
{
	POSITION pos = m_lstTabbedControlBars.Find (GetSafeHwnd ());
	if (pos == NULL)
	{
		ASSERT (FALSE);
	}
	else
	{
		m_lstTabbedControlBars.RemoveAt (pos);
	}

	CBCGPBaseTabbedBar::OnDestroy();
}
//*********************************************************************************
void CBCGPTabbedControlBar::ResetTabs ()
{
	for (POSITION pos = m_lstTabbedControlBars.GetHeadPosition (); pos != NULL;)
	{
		HWND hWnd = m_lstTabbedControlBars.GetNext (pos);
		if (!::IsWindow (hWnd))
		{
			continue;
		}

		CBCGPTabbedControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPTabbedControlBar,
			CWnd::FromHandlePermanent (hWnd));
		if (pBar == NULL)
		{
			continue;
		}

		ASSERT_VALID (pBar);

		CBCGPTabWnd* pTabWnd = pBar->GetTabWnd ();
		ASSERT_VALID (pTabWnd);

		pTabWnd->SetTabBorderSize (CBCGPVisualManager::GetInstance ()->GetDockingTabsBordersSize ());
		pTabWnd->SetDrawFrame (CBCGPVisualManager::GetInstance ()->IsDockingTabHasBorder ());
		pTabWnd->SetDrawNameInUpperCase(CBCGPVisualManager::GetInstance ()->IsDockingTabUpperCase());

		pTabWnd->ModifyTabStyle (m_StyleTabWnd);
		pTabWnd->RecalcLayout ();
		
		if (m_bIsTabsAutoColor)
		{
			pTabWnd->EnableAutoColor ();
			pTabWnd->SetAutoColors (m_arTabsAutoColors);
		}
		else
		{
			pTabWnd->EnableAutoColor (FALSE);

			CArray<COLORREF, COLORREF> arTabsAutoColors;
			pTabWnd->SetAutoColors (arTabsAutoColors);
		}
	}
}
