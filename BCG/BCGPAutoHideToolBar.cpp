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
// BCGPAutoHideToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPAutoHideToolBar.h"
#include "BCGPAutoHideButton.h"
#include "BCGPGlobalUtils.h"
#include "BCGPDockManager.h"
#include "BCGPDockBar.h"
#include "BCGPDockingControlBar.h"
#include "BCGPDockBarRow.h"
#include "trackmouse.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BCGP_DISPLAY_AHWND_EVENT	1

int CBCGPAutoHideToolBar::m_nShowAHWndDelay = 400;

CRuntimeClass* CBCGPAutoHideToolBar::m_pAutoHideButtonRTS = RUNTIME_CLASS (CBCGPAutoHideButton);

IMPLEMENT_DYNCREATE(CBCGPAutoHideToolBar, CBCGPControlBar)
/////////////////////////////////////////////////////////////////////////////
// CBCGPAutoHideToolBar

CBCGPAutoHideToolBar::CBCGPAutoHideToolBar()
{
	m_pLastActiveButton = NULL;	
	m_bReadyToDisplayAHWnd = FALSE;
	m_nDisplayAHWndTimerID = 0;
	m_pHighlightedButton = NULL;
	m_bTracked = FALSE;
	m_bResetSizes = FALSE;
}

CBCGPAutoHideToolBar::~CBCGPAutoHideToolBar()
{
	if (m_nDisplayAHWndTimerID != 0)
	{
		KillTimer (m_nDisplayAHWndTimerID);
	}
	CleanUpAutoHideButtons ();
}


BEGIN_MESSAGE_MAP(CBCGPAutoHideToolBar, CBCGPControlBar)
	//{{AFX_MSG_MAP(CBCGPAutoHideToolBar)
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_NCDESTROY()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_REGISTERED_MESSAGE(BCGM_CHANGEVISUALMANAGER, OnChangeVisualManager)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPAutoHideToolBar message handlers

int CBCGPAutoHideToolBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
		
	return 0;
}
//-------------------------------------------------------------------------------//
CBCGPAutoHideButton* CBCGPAutoHideToolBar::AddAutoHideWindow (CBCGPDockingControlBar* pAutoHideWnd, 
															  DWORD dwAlignment)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pAutoHideWnd);
	ASSERT_KINDOF (CBCGPDockingControlBar, pAutoHideWnd);

	CBCGPDockingControlBar* pAutoHideBar = 
		DYNAMIC_DOWNCAST (CBCGPDockingControlBar, pAutoHideWnd);

	if (pAutoHideBar == NULL)
	{
		ASSERT (FALSE);
		TRACE0 ("Only CBCGPDockingControlBar-derived class may have autohide state!\n");
		return NULL;
	}

	ASSERT (m_pAutoHideButtonRTS != NULL);

	CBCGPAutoHideButton* pNewAutoHideButton = 
		DYNAMIC_DOWNCAST (CBCGPAutoHideButton, m_pAutoHideButtonRTS->CreateObject ());

	if (pNewAutoHideButton == NULL)
	{
		ASSERT (FALSE);
		TRACE0 ("Wrong runtime class was specified for the autohide button class.\n");
		return NULL;
	}

	if (!pNewAutoHideButton->Create (this, pAutoHideBar, dwAlignment))
	{
		delete pNewAutoHideButton;
		TRACE0 ("Failed to create new AutoHide button.\n");
		return NULL;
	}
	
	CRect rect;
	GetWindowRect (rect);

	CSize sizeBtn = pNewAutoHideButton->GetSize ();

	if (m_lstAutoHideButtons.IsEmpty ())
	{
		rect.right += sizeBtn.cx;
		rect.bottom += sizeBtn.cy;
	}

	SetWindowPos (NULL, 0, 0, rect.Width (), rect.Height (), 
					SWP_NOZORDER | SWP_NOMOVE);

	m_lstAutoHideButtons.AddTail (pNewAutoHideButton);

	return pNewAutoHideButton;
}
//-------------------------------------------------------------------------------//
BOOL CBCGPAutoHideToolBar::RemoveAutoHideWindow (CBCGPDockingControlBar* pAutoHideWnd)
{
	if (m_nDisplayAHWndTimerID != 0)
	{
		KillTimer (m_nDisplayAHWndTimerID);
		m_nDisplayAHWndTimerID = 0;
	}

	POSITION posSave = NULL;
	for (POSITION pos = m_lstAutoHideButtons.GetHeadPosition (); pos != NULL;)
	{
		posSave = pos;
		CBCGPAutoHideButton* pBtn = (CBCGPAutoHideButton*) m_lstAutoHideButtons.GetNext (pos);
		ASSERT_VALID (pBtn);

		if (pBtn->GetAutoHideWindow () == pAutoHideWnd)
		{
			if (m_pLastActiveButton == pBtn)
			{
				m_pLastActiveButton = NULL;
			}

			m_lstAutoHideButtons.RemoveAt (posSave);
			delete pBtn;

			if (m_lstAutoHideButtons.IsEmpty ())
			{
				ASSERT_VALID (m_pParentDockBar);
				m_pParentDockBar->RemoveControlBar (this, BCGP_DM_UNKNOWN);
				CRect rectClient;
				m_pParentDockBar->GetClientRect (rectClient);
				m_pParentDockBar->RepositionBars (rectClient);
			}
			DestroyWindow ();
			return TRUE;
		}
	}
	
	return FALSE;
}
//-------------------------------------------------------------------------------//
BOOL CBCGPAutoHideToolBar::ShowAutoHideWindow (CBCGPDockingControlBar* pAutoHideWnd, BOOL bShow, 
											 BOOL /*bDelay*/)
{
	ASSERT_VALID (this);

	CBCGPAutoHideButton* pBtn = ButtonFromAutoHideWindow (pAutoHideWnd);
	if (pBtn == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pBtn);

	pBtn->ShowButton (bShow);
	
	UpdateVisibleState ();
	return TRUE;
}
//-------------------------------------------------------------------------------//
void CBCGPAutoHideToolBar::UpdateVisibleState ()
{
	BOOL bVisible = IsWindowVisible ();
	int nCount = GetVisibleCount ();

	ASSERT (m_pParentDockBar != NULL);

	if (/*bVisible &&*/ nCount == 0)
	{
		m_pParentDockBar->ShowControlBar (this, FALSE, FALSE, FALSE);	
	}
	else if (!bVisible && nCount > 0)
	{
		m_pParentDockBar->ShowControlBar (this, TRUE, FALSE, TRUE);	
	}
}
//-------------------------------------------------------------------------------//
void CBCGPAutoHideToolBar::UnSetAutoHideMode (CBCGPDockingControlBar* pFirstBarInGroup)
{
	for (POSITION pos = m_lstAutoHideButtons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPAutoHideButton* pBtn = (CBCGPAutoHideButton*) m_lstAutoHideButtons.GetNext (pos);
		ASSERT_VALID (pBtn);

		pBtn->UnSetAutoHideMode (pFirstBarInGroup);
	}
}
//-------------------------------------------------------------------------------//
int CBCGPAutoHideToolBar::GetVisibleCount ()
{
	int nCount = 0;
	for (POSITION pos = m_lstAutoHideButtons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPAutoHideButton* pBtn = (CBCGPAutoHideButton*) m_lstAutoHideButtons.GetNext (pos);
		ASSERT_VALID (pBtn);
		if (pBtn->IsVisible ())
		{
			nCount++;
		}
	}
	return nCount;
}
//-------------------------------------------------------------------------------//
CBCGPAutoHideButton* CBCGPAutoHideToolBar::ButtonFromAutoHideWindow (CBCGPDockingControlBar* pAutoHideWnd)
{
	ASSERT_VALID (this);
	for (POSITION pos = m_lstAutoHideButtons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPAutoHideButton* pBtn = (CBCGPAutoHideButton*) m_lstAutoHideButtons.GetNext (pos);
		ASSERT_VALID (pBtn);

		if (pBtn->GetAutoHideWindow () == pAutoHideWnd)
		{
			return pBtn;
		}
	}

	return NULL;
}
//-------------------------------------------------------------------------------//
void CBCGPAutoHideToolBar::DoPaint(CDC* pDC)
{
	ASSERT_VALID (pDC);

	CBCGPMemDC memDC (*pDC, this);
	
	CBCGPControlBar::DoPaint (&memDC.GetDC ());

	CBCGPAutoHideButton* pBtnTop = NULL;
	
	for (POSITION pos = m_lstAutoHideButtons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPAutoHideButton* pBtn = (CBCGPAutoHideButton*) m_lstAutoHideButtons.GetNext (pos);
		ASSERT_VALID (pBtn);
		if (pBtn->IsTop ())
		{
			pBtnTop = pBtn;
		}
		else if (pBtn->IsVisible ())
		{
			pBtn->OnDraw (&memDC.GetDC ());
		}
	}

	if (pBtnTop != NULL && pBtnTop->IsVisible ())
	{
		pBtnTop->OnDraw (&memDC.GetDC ());
	}
}
//-------------------------------------------------------------------------------//
void CBCGPAutoHideToolBar::OnMouseMove(UINT /*nFlags*/, CPoint /*point*/) 
{
	CPoint pt;
	GetCursorPos (&pt);
	ScreenToClient (&pt);
	CBCGPAutoHideButton* pBtn = ButtonFromPoint (pt);

	if (!m_bTracked)
	{
		m_bTracked = TRUE;
		
		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		::BCGPTrackMouse (&trackmouseevent);	
	}

	if (pBtn != NULL && !m_bReadyToDisplayAHWnd)
	{
		CBCGPDockingControlBar* pAttachedBar = pBtn->GetAutoHideWindow ();
		ASSERT_VALID (pAttachedBar);

		if (!pAttachedBar->IsWindowVisible ())
		{
			if (m_nDisplayAHWndTimerID != 0)
			{
				KillTimer (m_nDisplayAHWndTimerID);
				m_nDisplayAHWndTimerID = 0;
			}

			BOOL bStartAutoHide = TRUE;

			CBCGPDockManager* pDockManager = globalUtils.GetDockManager (GetDockSite ());
			if (pDockManager != NULL)
			{
				ASSERT_VALID (pDockManager);
				bStartAutoHide = !pDockManager->IsActivateAutoHideBarOnMouseClick();
			}

			if (bStartAutoHide)
			{
				m_bReadyToDisplayAHWnd = TRUE;
				m_nDisplayAHWndTimerID = SetTimer(BCGP_DISPLAY_AHWND_EVENT, m_nShowAHWndDelay, NULL);
			}
		}

		if (m_pHighlightedButton != NULL && m_pHighlightedButton != pBtn)
		{
			ASSERT_VALID (m_pHighlightedButton);

			m_pHighlightedButton->HighlightButton (FALSE);
		}

		if (pBtn != m_pHighlightedButton)
		{
			pBtn->HighlightButton (TRUE);
			m_pHighlightedButton = pBtn;

			UpdateWindow ();
			Invalidate ();
		}
	}
}
//-------------------------------------------------------------------------------//
afx_msg LRESULT CBCGPAutoHideToolBar::OnMouseLeave (WPARAM, LPARAM)
{
	if (m_pHighlightedButton != NULL)
	{
		ASSERT_VALID (m_pHighlightedButton);

		m_pHighlightedButton->HighlightButton (FALSE);
		m_pHighlightedButton = NULL;

		UpdateWindow ();
		Invalidate ();
	}

	m_bTracked = FALSE;
	return 0;
}
//-------------------------------------------------------------------------------//
CBCGPAutoHideButton* CBCGPAutoHideToolBar::ButtonFromPoint (CPoint pt)
{
	POSITION pos = NULL;

	for (pos = m_lstAutoHideButtons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPAutoHideButton* pBtn = (CBCGPAutoHideButton*) m_lstAutoHideButtons.GetNext (pos);
		ASSERT_VALID (pBtn);

		if (!pBtn->IsTop ())
		{
			continue;
		}

		if (pBtn->IsVisible())
		{
			CRect rect = pBtn->GetRect ();
			if (rect.PtInRect (pt))
			{
				return pBtn;
			}
		}
	}

	for (pos = m_lstAutoHideButtons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPAutoHideButton* pBtn = (CBCGPAutoHideButton*) m_lstAutoHideButtons.GetNext (pos);
		ASSERT_VALID (pBtn);

		if (pBtn->IsVisible())
		{
			CRect rect = pBtn->GetRect ();
			if (rect.PtInRect (pt))
			{
				return pBtn;
			}
		}
	}

	return NULL;
}
//-------------------------------------------------------------------------------//
CSize CBCGPAutoHideToolBar::CalcFixedLayout(BOOL /*bStretch*/, BOOL /*bHorz*/)
{
	CRect rect;
	GetWindowRect (&rect);
	return rect.Size ();
}
//-------------------------------------------------------------------------------//
void CBCGPAutoHideToolBar::CleanUpAutoHideButtons ()
{
	for (POSITION pos = m_lstAutoHideButtons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPAutoHideButton* pBtn = (CBCGPAutoHideButton*) m_lstAutoHideButtons.GetNext (pos);
		ASSERT_VALID (pBtn);

		delete pBtn;

	}

	m_lstAutoHideButtons.RemoveAll ();
}
//-------------------------------------------------------------------------------//
void CBCGPAutoHideToolBar::OnNcDestroy() 
{
	if (m_nDisplayAHWndTimerID != 0)
	{
		KillTimer (m_nDisplayAHWndTimerID);
		m_nDisplayAHWndTimerID = 0;
	}

	CWnd::OnNcDestroy();
	delete this;
}
//-------------------------------------------------------------------------------//
void CBCGPAutoHideToolBar::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == BCGP_DISPLAY_AHWND_EVENT)	
	{
		CPoint pt;
		GetCursorPos (&pt);
		ScreenToClient (&pt);
		CBCGPAutoHideButton* pBtn = ButtonFromPoint (pt);

		KillTimer (m_nDisplayAHWndTimerID);
		m_nDisplayAHWndTimerID = 0;

		if (pBtn != NULL && m_bReadyToDisplayAHWnd)
		{
			m_bReadyToDisplayAHWnd = FALSE;
			
			CBCGPDockingControlBar* pAttachedBar = pBtn->GetAutoHideWindow ();
			ASSERT_VALID (pAttachedBar);

			if (!pAttachedBar->IsWindowVisible ())
			{
				pBtn->ShowAttachedWindow (TRUE);
			}
		}
		else
		{
			m_bReadyToDisplayAHWnd = FALSE;
		}
	}
	
	CBCGPControlBar::OnTimer(nIDEvent);
}
//-------------------------------------------------------------------------------//
CSize CBCGPAutoHideToolBar::StretchControlBar (int /*nLength*/, BOOL /*bVert*/)
{
	CRect rect;
	GetWindowRect (rect);
	CSize size (0, 0);

	for (POSITION pos = m_lstAutoHideButtons.GetHeadPosition (); pos != NULL;)
	{
		CBCGPAutoHideButton* pBtn = (CBCGPAutoHideButton*) m_lstAutoHideButtons.GetNext (pos);
		ASSERT_VALID (pBtn);

		size = pBtn->GetSize (m_bResetSizes);
	}

	SetWindowPos (NULL, 0, 0, size.cx, size.cy, SWP_NOMOVE | SWP_NOZORDER); 

	m_bResetSizes = FALSE;
	return size;
}
//---------------------------------------------------------------------------------------
void CBCGPAutoHideToolBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CBCGPAutoHideButton* pBtn = CBCGPAutoHideToolBar::ButtonFromPoint (point);

	if (pBtn != NULL)
	{
		CBCGPDockingControlBar* pAttachedBar = pBtn->GetAutoHideWindow ();
		if (pAttachedBar != NULL)
		{
			BOOL bActivateOnClick = FALSE;
			
			CBCGPDockManager* pDockManager = globalUtils.GetDockManager (GetDockSite ());
			if (pDockManager != NULL)
			{
				ASSERT_VALID (pDockManager);
				bActivateOnClick = pDockManager->IsActivateAutoHideBarOnMouseClick();
			}

			if (bActivateOnClick)
			{
				pBtn->ShowAttachedWindow(!pAttachedBar->IsWindowVisible());
			}

			pAttachedBar->SetFocus ();
		}
	}
		
	CBCGPControlBar::OnLButtonDown(nFlags, point);
}
//---------------------------------------------------------------------------------------
void CBCGPAutoHideToolBar::SetActiveInGroup (BOOL bActive)
{
	CBCGPControlBar::SetActiveInGroup (bActive);
	if (bActive)
	{
		
		CObList lst;
		m_pDockBarRow->GetGroupFromBar (this, lst);

		for (POSITION pos = lst.GetHeadPosition (); pos != NULL;)
		{
			CBCGPControlBar* pBar = 
				DYNAMIC_DOWNCAST (CBCGPControlBar, lst.GetNext (pos));
			ASSERT_VALID (pBar);

			if (pBar != this)
			{
				pBar->SetActiveInGroup (FALSE);
			}
		}

		CRect rect; rect.SetRectEmpty ();
		m_pParentDockBar->RepositionBars (rect);
		
	}
}
//---------------------------------------------------------------------------------------
CBCGPDockingControlBar* CBCGPAutoHideToolBar::GetFirstAHWindow ()
{
	if (m_lstAutoHideButtons.IsEmpty ())
	{
		return NULL;
	}

	CBCGPAutoHideButton* pBtn = (CBCGPAutoHideButton*) m_lstAutoHideButtons.GetHead ();
	if (pBtn != NULL)
	{
		return pBtn->GetAutoHideWindow ();
	}
	return NULL;
}
//---------------------------------------------------------------------------------------
LRESULT CBCGPAutoHideToolBar::OnChangeVisualManager (WPARAM, LPARAM)
{
	m_bResetSizes = TRUE;

	return 0;
}
