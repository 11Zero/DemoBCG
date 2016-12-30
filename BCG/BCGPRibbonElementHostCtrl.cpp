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
//
// BCGPRibbonElementHostCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPRibbonElementHostCtrl.h"
#include "BCGPRibbonCategory.h"
#include "BCGPVisualManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef BCGP_EXCLUDE_RIBBON

static const int nScrollBarID = 1;

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonGalleryCtrl

IMPLEMENT_DYNAMIC(CBCGPRibbonGalleryCtrl, CBCGPRibbonPanelMenuBar)

CBCGPRibbonGalleryCtrl::CBCGPRibbonGalleryCtrl()
{
	ASSERT_VALID(m_pPanel);

	m_bIsCtrlMode = TRUE;
	m_pPanel->m_bMenuMode = TRUE;

	m_pPanel->m_pPaletteButton = &m_PaletteButton;
	m_pPanel->m_pParentMenuBar = this;
	m_PaletteButton.m_pParentControl = this;

	m_bIsMouseClicked = FALSE;
}

CBCGPRibbonGalleryCtrl::~CBCGPRibbonGalleryCtrl()
{
	m_pPanel->m_arElements.RemoveAll ();
}

BEGIN_MESSAGE_MAP(CBCGPRibbonGalleryCtrl, CBCGPRibbonPanelMenuBar)
	//{{AFX_MSG_MAP(CBCGPRibbonGalleryCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_MOUSEWHEEL()
	ON_WM_DESTROY()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonGalleryCtrl message handlers

void CBCGPRibbonGalleryCtrl::InitCtrl()
{
	ASSERT_VALID(m_pPanel);

	if (m_wndScrollBarVert.GetSafeHwnd() == NULL)
	{
		m_wndScrollBarVert.Create (WS_CHILD | WS_VISIBLE | SBS_VERT,
			CRect (0, 0, 0, 0), this, nScrollBarID);
		m_pPanel->m_pScrollBar = &m_wndScrollBarVert;
	}

	ModifyStyle(0, WS_CLIPCHILDREN);

	if (m_pPanel->IsEmpty())
	{
		RecalcLayout();
	}
}
//***********************************************************************************************	
void CBCGPRibbonGalleryCtrl::PreSubclassWindow() 
{
	CBCGPRibbonPanelMenuBar::PreSubclassWindow();
}
//***********************************************************************************************	
BOOL CBCGPRibbonGalleryCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_RETURN:
			GetParent()->SendMessage(WM_COMMAND, (WPARAM)GetDlgCtrlID(), (LPARAM)GetSafeHwnd());
			return TRUE;

		case VK_ESCAPE:
			return FALSE;
		}
	}

	return CBCGPRibbonPanelMenuBar::PreTranslateMessage(pMsg);
}
//***********************************************************************************************	
BOOL CBCGPRibbonGalleryCtrl::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//***********************************************************************************************	
int CBCGPRibbonGalleryCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPRibbonPanelMenuBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	InitCtrl();
	return 0;
}
//***********************************************************************************************	
BOOL CBCGPRibbonGalleryCtrl::Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwStyle) 
{
	return CWnd::Create (
		globalData.RegisterWindowClass (_T("BCGPRibbonGallery")), _T(""), dwStyle, rect, pParentWnd, nID, 0);
}
//***********************************************************************************************	
void CBCGPRibbonGalleryCtrl::RemoveAll(BOOL bRecalcLayout)
{
	ASSERT_VALID(m_pPanel);

	const int nScrollButtons = m_PaletteButton.m_bIsComboMode ? 0 : 3;

	for (int i = 0; i < nScrollButtons; i++)
	{
		delete m_PaletteButton.m_arIcons[m_PaletteButton.m_arIcons.GetSize() - i - 1];
	}

	m_pPanel->RemoveAll();
	m_PaletteButton.m_arIcons.RemoveAll ();

	if (bRecalcLayout)
	{
		AdjustLocations();
	}
}
//***********************************************************************************************	
void CBCGPRibbonGalleryCtrl::RecalcLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pPanel);

	m_pPanel->m_arElements.RemoveAll ();
	m_PaletteButton.RemoveAll();

	m_PaletteButton.CreateIcons();

	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arButtons;
	m_PaletteButton.GetMenuItems (arButtons);

	for (int i = 0; i < arButtons.GetSize (); i++)
	{
		CBCGPBaseRibbonElement* pButton = arButtons [i];
		ASSERT_VALID (pButton);

		m_pPanel->Add (pButton);
	}

	AdjustLocations();
}
//***********************************************************************************************	
BOOL CBCGPRibbonGalleryCtrl::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/) 
{
	ASSERT_VALID (this);

	const int nSteps = abs(zDelta) / WHEEL_DELTA;

	for (int i = 0; i < nSteps; i++)
	{
		if (m_iOffset > 0 || m_pPanel->m_bScrollDnAvailable)
		{
			int iOffset = GetOffset ();

			if (zDelta > 0)
			{
				if (m_iOffset > 0)
				{
					SetOffset (iOffset - 1);
				}
			}
			else
			{
				if (m_pPanel->m_bScrollDnAvailable)
				{
					SetOffset (iOffset + 1);
				}
			}
		}
		else
		{
			OnVScroll (zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0, 
				&m_wndScrollBarVert);
		}
	}

	return TRUE;
}
//****************************************************************************************
void CBCGPRibbonGalleryCtrl::OnDestroy() 
{
	m_PaletteButton.Clear();
	m_pPanel->m_arElements.RemoveAll ();

	CBCGPRibbonPanelMenuBar::OnDestroy();
}
//****************************************************************************************
void CBCGPRibbonGalleryCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	int nOldSelected = GetSelectedItem();
	
	CBCGPRibbonPanelMenuBar::OnLButtonUp(nFlags, point);

	if (::IsWindow(GetSafeHwnd()) && nOldSelected != GetSelectedItem())
	{
		m_bIsMouseClicked = TRUE;

		CBCGPRibbonPaletteIcon* pIcon = DYNAMIC_DOWNCAST(CBCGPRibbonPaletteIcon, m_PaletteButton.GetHighlighted());
		if (pIcon != NULL)
		{
			GetParent()->SendMessage (BCGM_ON_HIGHLIGHT_RIBBON_LIST_ITEM, (WPARAM) pIcon->GetIndex(), (LPARAM) pIcon);
		}

		m_bIsMouseClicked = FALSE;
	}
}
//****************************************************************************************
BOOL CBCGPRibbonGalleryCtrl::OnKey (UINT nChar)
{
	CBCGPBaseRibbonElement* pOldHighlighted = m_PaletteButton.GetHighlighted();
	
	BOOL bRes = CBCGPRibbonPanelMenuBar::OnKey(nChar);

	if (::IsWindow(GetSafeHwnd()))
	{
		if (pOldHighlighted != m_PaletteButton.GetHighlighted())
		{
			CBCGPRibbonPaletteIcon* pIcon = DYNAMIC_DOWNCAST(CBCGPRibbonPaletteIcon, m_PaletteButton.GetHighlighted());
			if (pIcon != NULL)
			{
				SelectItem(pIcon->GetIndex());
				RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				GetParent()->SendMessage (BCGM_ON_HIGHLIGHT_RIBBON_LIST_ITEM, (WPARAM) pIcon->GetIndex(), (LPARAM) pIcon);
			}
		}
	}

	return bRes;
}
//****************************************************************************************
void CBCGPRibbonGalleryCtrl::OnLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/) 
{
	if (::IsWindow(GetSafeHwnd()))
	{
		m_bIsMouseClicked = TRUE;

		GetParent()->SendMessage(WM_COMMAND, (WPARAM)GetDlgCtrlID(), (LPARAM)GetSafeHwnd());

		m_bIsMouseClicked = FALSE;
	}
}

#endif // BCGP_EXCLUDE_RIBBON

