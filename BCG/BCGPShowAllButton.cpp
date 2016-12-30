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

// BCGPShowAllButton.cpp: implementation of the CBCGPShowAllButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPShowAllButton.h"
#include "BCGPMenuBar.h"
#include "BCGPPopupMenuBar.h"
#include "BCGPPopupMenu.h"
#include "BCGGlobals.h"
#include "BCGPKeyHelper.h"
#include "bcgprores.h"
#include "BCGPLocalResource.h"
#include "BCGPVisualManager.h"
#include "BCGPDrawManager.h"

IMPLEMENT_DYNCREATE(CBCGPShowAllButton, CBCGPToolbarMenuButton)

const int nMinMenuWidth = 50;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPShowAllButton::CBCGPShowAllButton()
{
}
//***************************************************************************************
CBCGPShowAllButton::~CBCGPShowAllButton()
{
}
//***************************************************************************************
void CBCGPShowAllButton::OnDraw (CDC* pDC, const CRect& rect, CBCGPToolBarImages* /*pImages*/,
								BOOL /*bHorz*/, BOOL /*bCustomizeMode*/, BOOL bHighlight,
								BOOL /*bDrawBorder*/, BOOL /*bGrayDisabledButtons*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	CRect rectBar = rect;
	rectBar.DeflateRect (1, 1);

	//----------------------
	// Fill button interior:
	//----------------------
	FillInterior (pDC, rect, bHighlight);

	//-----------------------
	// Draw "show all" image:
	//-----------------------
	CBCGPVisualManager::BCGBUTTON_STATE state = CBCGPVisualManager::ButtonsIsRegular;

	if (bHighlight)
	{
		state = CBCGPVisualManager::ButtonsIsHighlighted;
	}
	else if (m_nStyle & (TBBS_PRESSED | TBBS_CHECKED))
	{
		//-----------------------
		// Pressed in or checked:
		//-----------------------
		state = CBCGPVisualManager::ButtonsIsPressed;
	}

	CBCGPVisualManager::GetInstance ()->OnDrawShowAllMenuItems (pDC,
		rectBar, state);

	//--------------------
	// Draw button border:
	//--------------------
	if (m_nStyle & (TBBS_PRESSED | TBBS_CHECKED))
	{
		//-----------------------
		// Pressed in or checked:
		//-----------------------
		CBCGPVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
			this, rectBar, CBCGPVisualManager::ButtonsIsPressed);
	}
	else if (bHighlight)
	{
		CBCGPVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
			this, rectBar, CBCGPVisualManager::ButtonsIsHighlighted);
	}
}
//***********************************************************************************
SIZE CBCGPShowAllButton::OnCalculateSize (
								CDC* pDC,
								const CSize& sizeDefault,
								BOOL /*bHorz*/)
{
	return CSize (nMinMenuWidth, 
		CBCGPVisualManager::GetInstance ()->GetShowAllMenuItemsHeight (pDC, sizeDefault));
}
//************************************************************************************
BOOL CBCGPShowAllButton::OnClick (CWnd* /*pWnd*/, BOOL bDelay)
{
	CBCGPPopupMenuBar* pParentMenuBar = DYNAMIC_DOWNCAST (CBCGPPopupMenuBar, m_pWndParent);
	if (pParentMenuBar == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (bDelay)
	{
		if (CBCGPMenuBar::IsShowAllCommandsDelay ())
		{
			pParentMenuBar->StartPopupMenuTimer (this, 2);
		}

		return TRUE;
	}

	CBCGPPopupMenu* pParentMenu = 
		DYNAMIC_DOWNCAST (CBCGPPopupMenu, pParentMenuBar->GetParent ());
	if (pParentMenu == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	pParentMenu->ShowAllCommands ();
	return TRUE;
}
//************************************************************************************
BOOL CBCGPShowAllButton::OpenPopupMenu (CWnd* pWnd)
{
	return OnClick (pWnd, FALSE);
}
//************************************************************************************
BOOL CBCGPShowAllButton::OnToolHitTest (const CWnd* /*pWnd*/, TOOLINFO* pTI)
{
	if (pTI == NULL)
	{
		return FALSE;
	}

	CString strText;
	CString strKey;

	ACCEL accel;
	accel.fVirt = FVIRTKEY | FCONTROL;
	accel.key = VK_DOWN;

	CBCGPKeyHelper helper (&accel);
	helper.Format (strKey);

	CBCGPLocalResource locaRes;
	strText.Format (IDS_BCGBARRES_EXPAND_FMT, strKey);

	pTI->lpszText = (LPTSTR) ::calloc ((strText.GetLength () + 1), sizeof (TCHAR));
	lstrcpy (pTI->lpszText, strText);

	pTI->uId = 0;
	return TRUE;
}
