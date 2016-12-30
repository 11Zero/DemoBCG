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
// BCGPDropDownList.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPDropDownList.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPDialog.h"
#include "BCGPPropertyPage.h"
#include "BCGPWorkspace.h"
#include "BCGPRibbonComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CBCGPWorkspace* g_pWorkspace;

const UINT idStart = (UINT) -200;

/////////////////////////////////////////////////////////////////////////////
// CBCGPDropDownList

IMPLEMENT_DYNAMIC(CBCGPDropDownList, CBCGPPopupMenu)

CBCGPDropDownList::CBCGPDropDownList()
{
	CommonInit ();
}
//***************************************************************************
CBCGPDropDownList::CBCGPDropDownList(CWnd* pEditCtrl)
{
	ASSERT_VALID (pEditCtrl);

	CommonInit ();
	m_pEditCtrl = pEditCtrl;
}

#ifndef BCGP_EXCLUDE_RIBBON

CBCGPDropDownList::CBCGPDropDownList(CBCGPRibbonComboBox* pRibbonCombo)
{
	ASSERT_VALID (pRibbonCombo);

	CommonInit ();

	m_pRibbonCombo = pRibbonCombo;
	m_pEditCtrl = pRibbonCombo->m_pWndEdit;
}

#endif

void CBCGPDropDownList::CommonInit ()
{
	m_pEditCtrl = NULL;
	m_pRibbonCombo = NULL;

	m_bShowScrollBar = TRUE;
	m_nMaxHeight = -1;

	m_Menu.CreatePopupMenu ();
	m_nCurSel = -1;

	m_nMinWidth = -1;
	m_bDisableAnimation = TRUE;

	SetAutoDestroy (FALSE);
}
//***************************************************************************
CBCGPDropDownList::~CBCGPDropDownList()
{
}

BEGIN_MESSAGE_MAP(CBCGPDropDownList, CBCGPPopupMenu)
	//{{AFX_MSG_MAP(CBCGPDropDownList)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CBCGPDropDownList::Track (CPoint point, CWnd *pWndOwner)
{
	if (!Create (pWndOwner, point.x, point.y, m_Menu, FALSE, TRUE))
	{
		return;
	}

	CBCGPPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	pMenuBar->m_iMinWidth = m_nMinWidth;
	pMenuBar->m_bDisableSideBarInXPMode = TRUE;

	HighlightItem (m_nCurSel);
	pMenuBar->RedrawWindow ();

	CRect rect;
	GetWindowRect (&rect);
	UpdateShadow (&rect);

	CBCGPDialog* pParentDlg = NULL;
	if (pWndOwner != NULL && pWndOwner->GetParent () != NULL)
	{
		pParentDlg = DYNAMIC_DOWNCAST (CBCGPDialog, pWndOwner->GetParent ());
		if (pParentDlg != NULL)
		{
			pParentDlg->SetActiveMenu (this);
		}
	}

	CBCGPPropertyPage* pParentPropPage = NULL;
	if (pWndOwner != NULL && pWndOwner->GetParent () != NULL)
	{
		pParentPropPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage, pWndOwner->GetParent ());
		if (pParentPropPage != NULL)
		{
			pParentPropPage->SetActiveMenu (this);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPDropDownList message handlers

int CBCGPDropDownList::GetCount() const
{
	ASSERT_VALID (this);
	return m_Menu.GetMenuItemCount ();
}
//***************************************************************************
int CBCGPDropDownList::GetCurSel()
{
	ASSERT_VALID (this);

	if (GetSafeHwnd () == NULL)
	{
		return m_nCurSel;
	}

	CBCGPPopupMenuBar* pMenuBar = ((CBCGPDropDownList*) this)->GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	CBCGPToolbarButton* pSel = pMenuBar->GetHighlightedButton ();
	if (pSel == NULL)
	{
		return -1;
	}

	int nIndex = 0;

	for (int i = 0; i < pMenuBar->GetCount (); i++)
	{
		CBCGPToolbarButton* pItem = pMenuBar->GetButton (i);
		ASSERT_VALID (pItem);

		if (!(pItem->m_nStyle & TBBS_SEPARATOR))
		{
			if (pSel == pItem)
			{
				m_nCurSel = nIndex;
				return nIndex;
			}

			nIndex++;
		}
	}

	return -1;
}
//***************************************************************************
int CBCGPDropDownList::SetCurSel(int nSelect)
{
	ASSERT_VALID (this);

	const int nSelOld = GetCurSel ();

	if (GetSafeHwnd () == NULL)
	{
		m_nCurSel = nSelect;
		return nSelOld;
	}

	CBCGPPopupMenuBar* pMenuBar = ((CBCGPDropDownList*) this)->GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	int nIndex = 0;

	for (int i = 0; i < pMenuBar->GetCount (); i++)
	{
		CBCGPToolbarButton* pItem = pMenuBar->GetButton (i);
		ASSERT_VALID (pItem);

		if (!(pItem->m_nStyle & TBBS_SEPARATOR))
		{
			if (nIndex == nSelect)
			{
				HighlightItem (i);
				return nSelOld;
			}

			nIndex++;
		}
	}

	return -1;
}
//***************************************************************************
void CBCGPDropDownList::GetText(int nIndex, CString& rString) const
{
	ASSERT_VALID (this);

	CBCGPToolbarButton* pItem = GetItem (nIndex);
	if (pItem == NULL)
	{
		return;
	}

	ASSERT_VALID (pItem);
	rString = pItem->m_strText;
}
//***************************************************************************
void CBCGPDropDownList::AddString (LPCTSTR lpszItem)
{
	ASSERT_VALID (this);
	ASSERT(lpszItem != NULL);
	ASSERT(GetSafeHwnd () == NULL);

	const UINT uiID = idStart - GetCount ();
	m_Menu.AppendMenu (MF_STRING, uiID, lpszItem);
}
//***************************************************************************
void CBCGPDropDownList::ResetContent()
{
	ASSERT_VALID (this);
	ASSERT(GetSafeHwnd () == NULL);

	m_Menu.DestroyMenu ();
	m_Menu.CreatePopupMenu ();
}
//***************************************************************************
void CBCGPDropDownList::HighlightItem (int nIndex)
{
	ASSERT_VALID (this);

	CBCGPPopupMenuBar* pMenuBar = GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	if (nIndex < 0)
	{
		return;
	}

	pMenuBar->m_iHighlighted = nIndex;

	SCROLLINFO scrollInfo;
	ZeroMemory(&scrollInfo, sizeof(SCROLLINFO));

    scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_ALL;

	m_wndScrollBarVert.GetScrollInfo (&scrollInfo);

	int iOffset = nIndex;
	int nMaxOffset = scrollInfo.nMax;

	iOffset = min (max (0, iOffset), nMaxOffset);

	if (iOffset != pMenuBar->GetOffset ())
	{
		pMenuBar->SetOffset (iOffset);

		m_wndScrollBarVert.SetScrollPos (iOffset);
		AdjustScroll ();
		
	}
}
//****************************************************************************
CBCGPToolbarButton* CBCGPDropDownList::GetItem (int nIndex) const
{
	ASSERT_VALID (this);

	CBCGPPopupMenuBar* pMenuBar = ((CBCGPDropDownList*) this)->GetMenuBar ();
	ASSERT_VALID (pMenuBar);

	int nCurrIndex = 0;

	for (int i = 0; i < pMenuBar->GetCount (); i++)
	{
		CBCGPToolbarButton* pItem = pMenuBar->GetButton (i);
		ASSERT_VALID (pItem);

		if (!(pItem->m_nStyle & TBBS_SEPARATOR))
		{
			if (nIndex == nCurrIndex)
			{
				return pItem;
			}

			nCurrIndex++;
		}
	}

	return NULL;
}
//****************************************************************************
void CBCGPDropDownList::OnDrawItem (CDC* pDC, CBCGPToolbarMenuButton* pItem, 
									BOOL bHighlight)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (pItem);

	CRect rectText = pItem->Rect ();
	rectText.DeflateRect (2 * TEXT_MARGIN, 0);

#ifndef BCGP_EXCLUDE_RIBBON
	if (m_pRibbonCombo != NULL)
	{
		ASSERT_VALID (m_pRibbonCombo);

		int nIndex = (int) idStart - pItem->m_nID;

		if (m_pRibbonCombo->OnDrawDropListItem (pDC, nIndex,
			pItem, bHighlight))
		{
			return;
		}
	}
#else
	UNREFERENCED_PARAMETER(bHighlight);
#endif

	pDC->DrawText (pItem->m_strText, &rectText, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
}
//****************************************************************************
CSize CBCGPDropDownList::OnGetItemSize (CDC* pDC, 
										CBCGPToolbarMenuButton* pItem, CSize sizeDefault)
{
	ASSERT_VALID (this);

#ifndef BCGP_EXCLUDE_RIBBON
	if (m_pRibbonCombo != NULL)
	{
		ASSERT_VALID (m_pRibbonCombo);

		int nIndex = (int) idStart - pItem->m_nID;

		CSize size = m_pRibbonCombo->OnGetDropListItemSize (
			pDC, nIndex, pItem, sizeDefault);

		if (size != CSize (0, 0))
		{
			return size;
		}
	}
#else
	UNREFERENCED_PARAMETER(pDC);
	UNREFERENCED_PARAMETER(pItem);
#endif
	return sizeDefault;
}
//****************************************************************************
void CBCGPDropDownList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	ASSERT_VALID (this);

	if (m_pEditCtrl->GetSafeHwnd () != NULL)
	{
		switch (nChar)
		{
		case VK_UP:
		case VK_DOWN:
		case VK_PRIOR:
		case VK_NEXT:
		case VK_ESCAPE:
		case VK_RETURN:
			break;

		default:
			m_pEditCtrl->SendMessage (WM_KEYDOWN, nChar, MAKELPARAM (nRepCnt, nFlags));
			return;
		}
	}
	
	CBCGPPopupMenu::OnKeyDown(nChar, nRepCnt, nFlags);
}
//****************************************************************************
void CBCGPDropDownList::OnChooseItem (UINT uiCommandID)
{
	ASSERT_VALID (this);

	CBCGPPopupMenu::OnChooseItem (uiCommandID);

#ifndef BCGP_EXCLUDE_RIBBON
	int nIndex = (int) idStart - uiCommandID;

	if (m_pRibbonCombo != NULL)
	{
		ASSERT_VALID (m_pRibbonCombo);
		m_pRibbonCombo->OnSelectItem (nIndex);
	}
#else
	UNREFERENCED_PARAMETER(uiCommandID);
#endif
}
//****************************************************************************
void CBCGPDropDownList::OnChangeHot (int nHot)
{
	ASSERT_VALID (this);

	CBCGPPopupMenu::OnChangeHot (nHot);

#ifndef BCGP_EXCLUDE_RIBBON
	if (m_pRibbonCombo != NULL)
	{
		ASSERT_VALID (m_pRibbonCombo);
		m_pRibbonCombo->NotifyHighlightListItem (nHot);
	}
#else
	UNREFERENCED_PARAMETER(nHot);
#endif
}
