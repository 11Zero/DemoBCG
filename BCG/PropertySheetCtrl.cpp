// PropertySheetCtrl.cpp : implementation file
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
#include "PropertySheetCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropertySheetCtrl

IMPLEMENT_DYNAMIC(CBCGPPropertySheetCtrl, CBCGPPropertySheet)

CBCGPPropertySheetCtrl::CBCGPPropertySheetCtrl(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	: CBCGPPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	m_hAccel = NULL;
	m_bIsAutoDestroy = TRUE;
}

CBCGPPropertySheetCtrl::CBCGPPropertySheetCtrl(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	: CBCGPPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	m_hAccel = NULL;
	m_bIsAutoDestroy = TRUE;
}

CBCGPPropertySheetCtrl::CBCGPPropertySheetCtrl()
	: CBCGPPropertySheet(_T(""), NULL, 0)
{
	m_hAccel = NULL;
	m_bIsAutoDestroy = TRUE;
}

CBCGPPropertySheetCtrl::~CBCGPPropertySheetCtrl()
{
}

BEGIN_MESSAGE_MAP(CBCGPPropertySheetCtrl, CBCGPPropertySheet)
	//{{AFX_MSG_MAP(CBCGPPropertySheetCtrl)
	ON_WM_SIZE()
	ON_WM_NCLBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropertySheetCtrl message handlers

void CBCGPPropertySheetCtrl::PostNcDestroy()
{
	// Call the base class routine first
	CBCGPPropertySheet::PostNcDestroy();
	
	if (m_bModeless && m_bIsAutoDestroy)
	{
		delete this;
	}
}

BOOL CBCGPPropertySheetCtrl::OnInitDialog()
{
	ASSERT_VALID(this);
	
	BOOL bRes = CBCGPPropertySheet::OnInitDialog();
	
	ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	m_bDrawPageFrame = FALSE;

	ResizeControl();

	return bRes;
}

BOOL CBCGPPropertySheetCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR* pNMHDR = (NMHDR*) lParam;
	ASSERT(pNMHDR != NULL);

	if (pNMHDR->code == TCN_SELCHANGE)
	{
		ResizeControl();
	}
	
	return CBCGPPropertySheet::OnNotify(wParam, lParam, pResult);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
void CBCGPPropertySheetCtrl::LoadAcceleratorTable(UINT nAccelTableID /*=0*/)
{
	if (nAccelTableID)
	{
		m_hAccel = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(nAccelTableID));
		ASSERT(m_hAccel);
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
BOOL CBCGPPropertySheetCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (m_hAccel != NULL && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
	{
		return TRUE;
	}

	return CBCGPPropertySheet::PreTranslateMessage(pMsg);
}

void CBCGPPropertySheetCtrl::OnSize(UINT nType, int cx, int cy) 
{
	SetRedraw(FALSE);

	CBCGPPropertySheet::OnSize(nType, cx, cy);

	ResizeControl();

	if (m_wndTab.GetSafeHwnd() != NULL)
	{
		SetActivePage(GetActivePage());
	}

	SetRedraw(TRUE);
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
}

void CBCGPPropertySheetCtrl::ResizeControl()
{
	CWnd* pTabCtrl = GetTabControl();
	if (m_wndTab.GetSafeHwnd() != NULL)
	{
		pTabCtrl = &m_wndTab;
		m_wndTab.SetButtonsVisible(FALSE);
	}

	if (pTabCtrl->GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	pTabCtrl->SetWindowPos(NULL, 
		0, 0, rectClient.Width(), rectClient.Height(), 
		SWP_NOZORDER | SWP_NOACTIVATE);

	int nPageCount = CBCGPPropertySheet::GetPageCount();

	int nXBorder = ::GetSystemMetrics(SM_CXEDGE);
	int nYBorder = ::GetSystemMetrics(SM_CYEDGE);
	
	for (int nPage = 0; nPage <= nPageCount - 1; nPage++)
	{
		CPropertyPage* pPage = GetPage(nPage);
		if (pPage->GetSafeHwnd() != NULL)
		{
			CRect rectPage;

			pPage->GetWindowRect(&rectPage);
			pTabCtrl->ScreenToClient(rectPage);

			pPage->SetWindowPos(NULL, 
				rectPage.left, rectPage.top, 
				rectClient.Width() - nXBorder * 3, 
				rectClient.Height() - rectPage.top - nYBorder,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	RedrawWindow();
}

void CBCGPPropertySheetCtrl::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	if (IsDragClientAreaEnabled() && nHitTest == HTCAPTION)
	{
		CWnd* pParent = GetParent();
		if (pParent->GetSafeHwnd() != NULL)
		{
			pParent->SendMessage(WM_NCLBUTTONDOWN, (WPARAM) HTCAPTION, MAKELPARAM(point.x, point.y));
		}

		return;
	}

	CBCGPPropertySheet::OnNcLButtonDown(nHitTest, point);
}
