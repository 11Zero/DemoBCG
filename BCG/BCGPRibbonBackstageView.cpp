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
// BCGPRibbonBackstageView.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPRibbonBackstageView.h"
#include "BCGPRibbonBar.h"
#include "BCGPRibbonMainPanel.h"
#include "BCGPRibbonStatusBar.h"
#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef BCGP_EXCLUDE_RIBBON

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonBackstageView

IMPLEMENT_DYNAMIC(CBCGPRibbonBackstageView, CBCGPRibbonPanelMenuBar)

CBCGPRibbonBackstageView::CBCGPRibbonBackstageView(CBCGPRibbonBar* pRibbonBar, CBCGPRibbonMainPanel* pPanel) :
	CBCGPRibbonPanelMenuBar(pPanel)
{
	ASSERT_VALID(pRibbonBar);
	m_pRibbonBar = pRibbonBar;
}

CBCGPRibbonBackstageView::~CBCGPRibbonBackstageView()
{
}


BEGIN_MESSAGE_MAP(CBCGPRibbonBackstageView, CBCGPRibbonPanelMenuBar)
	//{{AFX_MSG_MAP(CBCGPRibbonBackstageView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_NCDESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonBackstageView message handlers

BOOL CBCGPRibbonBackstageView::Create() 
{
	ASSERT_VALID(m_pRibbonBar);
	
	CFrameWnd* pParentFrame = m_pRibbonBar->GetParentFrame ();
	ASSERT_VALID (pParentFrame);

	CRect rectView;
	GetRect(rectView);

	CString strClassName = ::AfxRegisterWndClass (
			CS_SAVEBITS,
			::LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)(COLOR_BTNFACE + 1), NULL);

	return CWnd::CreateEx(0, strClassName, _T(""), WS_CHILD | WS_VISIBLE, rectView, pParentFrame, (UINT)-1, NULL);
}

void CBCGPRibbonBackstageView::GetRect(CRect& rect)
{
	CFrameWnd* pParentFrame = m_pRibbonBar->GetParentFrame ();
	ASSERT_VALID (pParentFrame);

	pParentFrame->GetClientRect(rect);

	CRect rectRibbon;
	m_pRibbonBar->GetWindowRect(rectRibbon);
	pParentFrame->ScreenToClient(rectRibbon);

	rect.top = rectRibbon.top + m_pRibbonBar->m_nBackstageViewTop;
}

int CBCGPRibbonBackstageView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPRibbonPanelMenuBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	SetOnOffMode(TRUE);

	ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	SetFocus();
	return 0;
}

void CBCGPRibbonBackstageView::OnDestroy() 
{
	SetOnOffMode(FALSE);
	CBCGPRibbonPanelMenuBar::OnDestroy();
}

void CBCGPRibbonBackstageView::OnNcDestroy() 
{
	CBCGPRibbonPanelMenuBar::OnNcDestroy();
	delete this;
}

void CBCGPRibbonBackstageView::SetOnOffMode(BOOL bOn)
{
	ASSERT_VALID(m_pRibbonBar);
	
	CFrameWnd* pParentFrame = m_pRibbonBar->GetParentFrame ();
	if (pParentFrame == NULL)
	{
		return;
	}

	ASSERT_VALID (pParentFrame);

	if (bOn)
	{
		m_lstHiddenWindows.RemoveAll();

		CWnd* pWndChild = pParentFrame->GetWindow (GW_CHILD);
		while (pWndChild != NULL)
		{
			ASSERT_VALID (pWndChild);

			if (pWndChild->IsWindowVisible() && pWndChild->GetSafeHwnd() != m_pRibbonBar->GetSafeHwnd() &&
				pWndChild->GetSafeHwnd() != GetSafeHwnd())
			{
				m_lstHiddenWindows.AddHead(pWndChild->GetSafeHwnd());

				CBCGPRibbonStatusBar* pStatusBar = DYNAMIC_DOWNCAST(CBCGPRibbonStatusBar, pWndChild);
				if (pStatusBar != NULL)
				{
					ASSERT_VALID(pStatusBar);
					pStatusBar->m_bTemporaryHidden = TRUE;

				}

				pWndChild->ShowWindow(SW_HIDE);

				if (pStatusBar != NULL)
				{
					ASSERT_VALID(pStatusBar);
					pStatusBar->m_bTemporaryHidden = FALSE;
					
				}
			}

			pWndChild = pWndChild->GetNextWindow ();
		}
	}
	else
	{
		for (POSITION pos = m_lstHiddenWindows.GetHeadPosition(); pos != NULL;)
		{
			::ShowWindow(m_lstHiddenWindows.GetNext(pos), SW_SHOWNOACTIVATE);
		}

		m_lstHiddenWindows.RemoveAll();
	}

	pParentFrame->RecalcLayout();

	if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPFrameWnd)))
	{
		((CBCGPFrameWnd*) pParentFrame)->SetRibbonBackstageView (bOn ? this : NULL);
		((CBCGPFrameWnd*) pParentFrame)->ShowFloatingBars(!bOn);
	}
	else if (pParentFrame->IsKindOf (RUNTIME_CLASS (CBCGPMDIFrameWnd)))
	{
		((CBCGPMDIFrameWnd*) pParentFrame)->SetRibbonBackstageView (bOn ? this : NULL);
		((CBCGPMDIFrameWnd*) pParentFrame)->ShowFloatingBars(!bOn);

		if (((CBCGPMDIFrameWnd*) pParentFrame)->GetMDITabs().GetSafeHwnd() != NULL)
		{
			SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
	}
	
	m_pRibbonBar->SetBackstageViewActve(bOn);
	
	if (!bOn && m_pRibbonBar->m_pMainButton != NULL)
	{
		m_pRibbonBar->m_pMainButton->m_bIsDroppedDown = FALSE;
	}

	pParentFrame->SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);

	m_pRibbonBar->ForceRecalcLayout();
	m_pRibbonBar->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
}

BOOL CBCGPRibbonBackstageView::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (pMsg->wParam == VK_ESCAPE)
		{
			SendMessage (WM_CLOSE);
			return TRUE;
		}

		if (OnKey ((UINT)pMsg->wParam))
		{
			return TRUE;
		}
		break;
	}

	return CBCGPRibbonPanelMenuBar::PreTranslateMessage(pMsg);
}

BOOL CBCGPRibbonBackstageView::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

BOOL CBCGPRibbonBackstageView::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/) 
{
	if (!m_wndScrollBarVert.IsWindowVisible())
	{
		return FALSE;
	}

	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return TRUE;
	}

	const int nSteps = abs(zDelta) / WHEEL_DELTA;

	for (int i = 0; i < nSteps; i++)
	{
		OnVScroll (zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0,  &m_wndScrollBarVert);
	}

	return TRUE;
}

void CBCGPRibbonBackstageView::Deactivate ()
{
	m_pRibbonBar->m_bDontSetKeyTips = TRUE;
	CBCGPRibbonPanelMenuBar::Deactivate();
}

#endif // BCGP_EXCLUDE_RIBBON

