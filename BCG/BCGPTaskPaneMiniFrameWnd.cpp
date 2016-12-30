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
// BCGPTaskPaneMiniFrameWnd.cpp: implementation of the CBCGPTaskPaneMiniFrameWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"

#ifndef BCGP_EXCLUDE_TASK_PANE

#include "BCGPGlobalUtils.h"
#include "BCGPDockBar.h"
#include "BCGPDockManager.h"
#include "BCGPCaptionMenuButton.h"
#include "BCGPTasksPane.h"
#include "BCGPTaskPaneMiniFrameWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CBCGPTaskPaneMiniFrameWnd,CBCGPMiniFrameWnd,VERSIONABLE_SCHEMA | 2)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPTaskPaneMiniFrameWnd::CBCGPTaskPaneMiniFrameWnd()
{
	m_bMenuBtnPressed = FALSE;
}

CBCGPTaskPaneMiniFrameWnd::~CBCGPTaskPaneMiniFrameWnd()
{

}

BEGIN_MESSAGE_MAP(CBCGPTaskPaneMiniFrameWnd, CBCGPMiniFrameWnd)
	//{{AFX_MSG_MAP(CBCGPTaskPaneMiniFrameWnd)
	ON_WM_NCPAINT()
	//}}AFX_MSG_MAP
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnNeedTipText)
END_MESSAGE_MAP()

//--------------------------------------------------------------------------------------//
void CBCGPTaskPaneMiniFrameWnd::SetCaptionButtons (DWORD dwButtons)
{
	ASSERT_VALID (this);

	RemoveAllCaptionButtons ();

	CBCGPDockManager* pDockManager = m_pDockManager != NULL ? m_pDockManager : globalUtils.GetDockManager (GetParent ());
	if (!m_bHostsToolbar && pDockManager != NULL && pDockManager->AreFloatingBarsCanBeMaximized())
	{
		dwButtons |= BCGP_CAPTION_BTN_MAXIMIZE;
	}
	else
	{
		dwButtons &= ~BCGP_CAPTION_BTN_MAXIMIZE;
	}

	if (dwButtons & BCGP_CAPTION_BTN_CLOSE)
	{
		CBCGPBaseControlBar* pBar = DYNAMIC_DOWNCAST (CBCGPBaseControlBar, GetControlBar ());
		if (pBar != NULL && pBar->CanBeClosed ())
		{
			AddButton (HTCLOSE);
		}
	}

	if (dwButtons & BCGP_CAPTION_BTN_MAXIMIZE)
	{
		AddButton(HTMAXBUTTON_BCG); 
	}
	
	if (dwButtons & BCGP_CAPTION_BTN_PIN)
	{
		AddButton (HTMAXBUTTON);
	}

	if (dwButtons & BCGP_CAPTION_BTN_MENU)
	{
		AddButton (HTMINBUTTON);
	}

	AddButton (HTLEFTBUTTON_BCG);
	AddButton (HTRIGHTBUTTON_BCG);
	AddButton (HTMENU_BCG);

	m_dwCaptionButtons = dwButtons | BCGP_CAPTION_BTN_LEFT | 
			BCGP_CAPTION_BTN_RIGHT | BCGP_CAPTION_BTN_TPMENU;
	SetCaptionButtonsToolTips ();

	ArrangeCaptionButtons ();
	SendMessage (WM_NCPAINT);
}
//--------------------------------------------------------------------------------------//
void CBCGPTaskPaneMiniFrameWnd::AddButton (UINT nHit)
{
	ASSERT_VALID (this);

	CBCGPCaptionButton* pBtn = FindButton (nHit);

	if (pBtn == NULL)
	{
		switch (nHit) 
		{
		case HTLEFTBUTTON_BCG:
			m_lstCaptionButtons.AddHead (new CBCGPCaptionButton (HTLEFTBUTTON_BCG, TRUE));
			break;
		case HTRIGHTBUTTON_BCG:
			m_lstCaptionButtons.AddHead (new CBCGPCaptionButton (HTRIGHTBUTTON_BCG, TRUE));
			break;

		case HTMENU_BCG:
			{
				CBCGPCaptionMenuButton *pMenuBtn = new CBCGPCaptionMenuButton;
				pMenuBtn->m_bOSMenu = FALSE;
				pMenuBtn->m_nHit = HTMENU_BCG;
				m_lstCaptionButtons.AddHead (pMenuBtn);
			}
			break;

		default:
			CBCGPMiniFrameWnd::AddButton (nHit);
		}
	}
}
//--------------------------------------------------------------------------------------//
void CBCGPTaskPaneMiniFrameWnd::SetTaskPaneCaptionButtons ()
{
	ASSERT_VALID (this);

	if (TRUE)
	{
		SetCaptionButtons (m_dwCaptionButtons | BCGP_CAPTION_BTN_LEFT | 
												BCGP_CAPTION_BTN_RIGHT | 
												BCGP_CAPTION_BTN_TPMENU);
	}	
}
//--------------------------------------------------------------------------------------//
void CBCGPTaskPaneMiniFrameWnd::OnNcPaint()
{
	// Enable or disable Taskpane specific caption buttons:
	CBCGPTasksPane* pTaskPane = DYNAMIC_DOWNCAST (CBCGPTasksPane, GetControlBar ());
	BOOL bMultiPages = (pTaskPane != NULL && pTaskPane->GetPagesCount () > 1);
	BOOL bUseNavigationToolbar = (pTaskPane != NULL && pTaskPane->IsNavigationToolbarEnabled ());

	for (POSITION pos = m_lstCaptionButtons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPCaptionButton* pBtn = (CBCGPCaptionButton*) m_lstCaptionButtons.GetNext (pos);
		ASSERT_VALID (pBtn);

		switch (pBtn->GetHit ())
		{
		case HTLEFTBUTTON_BCG:
		case HTRIGHTBUTTON_BCG:
		case HTMENU_BCG:
			pBtn->m_bHidden = !bMultiPages || bUseNavigationToolbar;
		}
		if (pBtn->GetHit () == HTLEFTBUTTON_BCG)
		{
			pBtn->m_bEnabled = (pTaskPane != NULL && pTaskPane->IsBackButtonEnabled ());
		}
		if (pBtn->GetHit () == HTRIGHTBUTTON_BCG)
		{
			pBtn->m_bEnabled = (pTaskPane != NULL && pTaskPane->IsForwardButtonEnabled ());
		}
	}

	UpdateTooltips ();
	
	CBCGPMiniFrameWnd::OnNcPaint ();
}
//--------------------------------------------------------------------------------------//
void CBCGPTaskPaneMiniFrameWnd::OnDrawBorder (CDC* pDC)
{
	CBCGPMiniFrameWnd::OnDrawBorder (pDC);
}
//--------------------------------------------------------------------------------------//
void CBCGPTaskPaneMiniFrameWnd::OnDrawCaptionButtons (CDC* pDC)
{
	ASSERT_VALID (pDC);

	// Paint caption buttons:
	for (POSITION pos = m_lstCaptionButtons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPCaptionButton* pBtn = (CBCGPCaptionButton*) m_lstCaptionButtons.GetNext (pos);
		ASSERT_VALID (pBtn);

		BOOL bMaximized = TRUE;
		if (pBtn->GetHit () == HTMAXBUTTON && m_bPinned)
		{
			bMaximized = FALSE;
		}
		pBtn->OnDraw (pDC, FALSE, TRUE, bMaximized);
	}
}
//--------------------------------------------------------------------------------------//
void CBCGPTaskPaneMiniFrameWnd::OnPressButtons (UINT nHit)
{
	CBCGPTasksPane* pTaskPane = DYNAMIC_DOWNCAST (CBCGPTasksPane, GetControlBar ());
	if (pTaskPane != NULL)
	{
		ASSERT_VALID (pTaskPane);

		switch (nHit)
		{
		case HTLEFTBUTTON_BCG:
			// Handle Back caption button
			pTaskPane->OnPressBackButton ();
			break;

		case HTRIGHTBUTTON_BCG:
			// Handle Forward caption button
			pTaskPane->OnPressForwardButton ();
			break;

		case HTMENU_BCG:
			// Handle Other caption button
			{
				CBCGPCaptionMenuButton* pbtn = (CBCGPCaptionMenuButton*)FindButton (HTMENU_BCG);
				if (pbtn != NULL)
				{
					m_bMenuBtnPressed = TRUE;
					pTaskPane->OnPressOtherButton (pbtn, this);
					m_bMenuBtnPressed = FALSE;
				}
			}
			break;
		}
	}

	CBCGPMiniFrameWnd::OnPressButtons (nHit);
}
//--------------------------------------------------------------------------------------//
void CBCGPTaskPaneMiniFrameWnd::CalcBorderSize (CRect& rectBorderSize) const
{
		rectBorderSize.SetRect (g_nToolbarBorderSize, g_nToolbarBorderSize, 
								g_nToolbarBorderSize, g_nToolbarBorderSize);
}
//--------------------------------------------------------------------------------------//
void CBCGPTaskPaneMiniFrameWnd::OnTrackCaptionButtons (CPoint point)
{
	if (!m_bMenuBtnPressed)
	{
		CBCGPMiniFrameWnd::OnTrackCaptionButtons (point);
	}
}
//--------------------------------------------------------------------------------------//
void CBCGPTaskPaneMiniFrameWnd::StopCaptionButtonsTracking ()
{
	if (!m_bMenuBtnPressed)
	{
		CBCGPMiniFrameWnd::StopCaptionButtonsTracking ();
	}
}
//--------------------------------------------------------------------------------------//
BOOL CBCGPTaskPaneMiniFrameWnd::OnNeedTipText(UINT id, NMHDR* pNMH, LRESULT* pResult)
{
	static CString strTipText;

	if (m_pToolTip->GetSafeHwnd () == NULL || 
		pNMH->hwndFrom != m_pToolTip->GetSafeHwnd ())
	{
		return FALSE;
	}

	LPNMTTDISPINFO	pTTDispInfo	= (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	if (pNMH->idFrom > 0 && (int)pNMH->idFrom <= m_lstCaptionButtons.GetCount())
	{
		POSITION pos = m_lstCaptionButtons.FindIndex (pNMH->idFrom - 1);
		if (pos != NULL)
		{
			CBCGPCaptionButton* pBtn = (CBCGPCaptionButton*)m_lstCaptionButtons.GetAt (pos);
			ASSERT_VALID (pBtn);

			switch (pBtn->GetHit ())
			{
			case HTLEFTBUTTON_BCG:
				//CBCGPLocalResource locaRes;
				//strTipText.LoadString (IDS_BCGBARRES_BACK);
				strTipText = _T("Back");
				pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) strTipText);
				return TRUE;
			case HTRIGHTBUTTON_BCG:
				//CBCGPLocalResource locaRes;
				//strTipText.LoadString (IDS_BCGBARRES_FORWARD);
				strTipText = _T("Forward");
				pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) strTipText);
				return TRUE;
			case HTMENU_BCG:
				//CBCGPLocalResource locaRes;
				//strTipText.LoadString (IDS_BCGBARRES_OTHER_TASKS_PANE);
				strTipText = _T("Other Tasks Pane");
				pTTDispInfo->lpszText = const_cast<LPTSTR> ((LPCTSTR) strTipText);
				return TRUE;
			}
	}
	}

	return CBCGPMiniFrameWnd::OnNeedTipText (id, pNMH, pResult);
}

#endif // BCGP_EXCLUDE_TASK_PANE
