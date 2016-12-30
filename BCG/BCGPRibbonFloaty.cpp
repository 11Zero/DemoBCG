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
// BCGPRibbonFloaty.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPRibbonFloaty.h"
#include "BCGPRibbonBar.h"
#include "bcgglobals.h"
#include "BCGPContextMenuManager.h"

#ifndef BCGP_EXCLUDE_RIBBON

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_VISIBILITY_TIMER	1

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonFloaty

IMPLEMENT_DYNCREATE(CBCGPRibbonFloaty, CBCGPRibbonPanelMenu)

CBCGPRibbonFloaty* CBCGPRibbonFloaty::m_pCurrent = NULL;

CBCGPRibbonFloaty::CBCGPRibbonFloaty()
{
	if (m_pCurrent != NULL)
	{
		m_pCurrent->SendMessage (WM_CLOSE);
		m_pCurrent = NULL;
	}

	m_wndRibbonBar.m_bIsFloaty = TRUE;
	m_bContextMenuMode = FALSE;
	m_nTransparency = 0;
	m_bWasHovered = FALSE;
	m_bDisableAnimation = TRUE;
	m_bIsOneRow = FALSE;
	m_bWasDroppedDown = FALSE;
}
//*******************************************************************************
CBCGPRibbonFloaty::~CBCGPRibbonFloaty()
{
	ASSERT (m_pCurrent == this);
	m_pCurrent = NULL;

	if (m_bContextMenuMode)
	{
		g_pContextMenuManager->SetDontCloseActiveMenu (FALSE);
	}
}

BEGIN_MESSAGE_MAP(CBCGPRibbonFloaty, CBCGPRibbonPanelMenu)
	//{{AFX_MSG_MAP(CBCGPRibbonFloaty)
	ON_WM_TIMER()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CBCGPRibbonFloaty::SetCommands (
	CBCGPRibbonBar* pRibbonBar,
	const CList<UINT,UINT>& lstCommands)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pRibbonBar);

	CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arButtons;

	for (POSITION pos = lstCommands.GetHeadPosition (); pos != NULL;)
	{
		UINT uiCmd = lstCommands.GetNext (pos);
		
		if (uiCmd == 0)
		{
			// TODO: add separator
			continue;
		}

		CBCGPBaseRibbonElement* pSrcElem = pRibbonBar->FindByID (uiCmd, FALSE);
		if (pSrcElem == NULL)
		{
			continue;
		}

		arButtons.Add (pSrcElem);
	}

	m_wndRibbonBar.AddButtons (pRibbonBar, arButtons, TRUE);
}
//*******************************************************************************
BOOL CBCGPRibbonFloaty::Show (int x, int y)
{
	ASSERT_VALID (this);

	m_wndRibbonBar.m_bIsOneRowFloaty = m_bIsOneRow;
	CSize size = m_wndRibbonBar.CalcSize (FALSE);

	if (!Create (m_wndRibbonBar.m_pRibbonBar, 
		x, y - size.cy - ::GetSystemMetrics (SM_CYCURSOR) / 2, (HMENU) NULL))
	{
		return FALSE;
	}

	m_pCurrent = this;

	ModifyStyleEx (0, WS_EX_LAYERED);

	UpdateTransparency ();

	globalData.SetLayeredAttrib (GetSafeHwnd (), 0, 
		m_nTransparency, LWA_ALPHA);

	UpdateShadowTransparency (m_nTransparency);

	return TRUE;
}
//*******************************************************************************
BOOL CBCGPRibbonFloaty::ShowWithContextMenu (int x, int y, UINT uiMenuResID, CWnd* pWndOwner)
{
	ASSERT_VALID (this);

	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return FALSE;
	}

	if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0)	// Left mouse button is pressed
	{
		return FALSE;
	}

	if (g_pContextMenuManager == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (x == -1 || y == -1)
	{
		return g_pContextMenuManager->ShowPopupMenu (uiMenuResID, x, y, pWndOwner);
	}

	CSize size = m_wndRibbonBar.CalcSize (FALSE);

	const int yOffset = 15;

	m_bContextMenuMode = TRUE;

	if (!Create (m_wndRibbonBar.m_pRibbonBar, x, y - size.cy - yOffset, (HMENU) NULL))
	{
		return FALSE;
	}

	m_pCurrent = this;

	ASSERT_VALID (g_pContextMenuManager);

	g_pContextMenuManager->SetDontCloseActiveMenu ();

	m_nMinWidth = size.cx;

	g_pContextMenuManager->ShowPopupMenu (uiMenuResID, x, y, pWndOwner);

	m_nMinWidth = 0;

	CBCGPPopupMenu* pPopup = CBCGPPopupMenu::GetActiveMenu ();

	if (pPopup != NULL)
	{
		ASSERT_VALID (pPopup);
		pPopup->m_hwndConnectedFloaty = GetSafeHwnd ();

		CRect rectMenu;
		pPopup->GetWindowRect (&rectMenu);

		if (rectMenu.top < y)
		{
			SetWindowPos (NULL, rectMenu.left, rectMenu.top - size.cy - yOffset, -1, -1,
				SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
		}

		CRect rectFloaty;
		GetWindowRect (rectFloaty);

		if (rectMenu.top < rectFloaty.bottom)
		{
			SetWindowPos (NULL, rectMenu.left, rectMenu.bottom + yOffset, -1, -1,
				SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
		}
	}

	return TRUE;
}
//************************************************************************************
void CBCGPRibbonFloaty::OnTimer(UINT_PTR nIDEvent) 
{
	CBCGPRibbonPanelMenu::OnTimer(nIDEvent);

	if (nIDEvent != ID_VISIBILITY_TIMER)
	{
		return;
	}

	if (m_bContextMenuMode)
	{
		KillTimer (ID_VISIBILITY_TIMER);
		return;
	}

	if (m_wndRibbonBar.GetPanel () != NULL)
	{
		if (m_wndRibbonBar.GetPanel ()->GetDroppedDown () != NULL)
		{
			m_bWasDroppedDown = TRUE;
			return;
		}
	}

	if (UpdateTransparency ())
	{
		globalData.SetLayeredAttrib (GetSafeHwnd (), 0, 
			m_nTransparency, LWA_ALPHA);

		UpdateShadowTransparency (m_nTransparency);
	}
}
//************************************************************************************
int CBCGPRibbonFloaty::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPRibbonPanelMenu::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_bContextMenuMode)
	{
		SetTimer (ID_VISIBILITY_TIMER, 100, NULL);
	}

	return 0;
}
//************************************************************************************
BOOL CBCGPRibbonFloaty::UpdateTransparency ()
{
	BYTE nTransparency = 0;

	CRect rect;
	GetWindowRect (rect);

	CPoint ptCursor;
	::GetCursorPos (&ptCursor);

	if (m_wndRibbonBar.GetPanel ()->GetDroppedDown () != NULL ||
		m_wndRibbonBar.GetPanel ()->GetHighlighted () != NULL ||
		m_wndRibbonBar.GetPanel ()->GetPressed () != NULL)
	{
		nTransparency = 255;

		if (m_bWasDroppedDown && rect.PtInRect (ptCursor))
		{
			m_bWasDroppedDown = FALSE;
		}
	}
	else
	{
		if (CBCGPPopupMenu::GetActiveMenu() != this)
		{
			PostMessage (WM_CLOSE);
			return FALSE;
		}

		if (rect.PtInRect (ptCursor))
		{
			m_bWasHovered = TRUE;
			nTransparency = 255;
			m_bWasDroppedDown = FALSE;
		}
		else if (m_bWasDroppedDown)
		{
			nTransparency = 255;
		}
		else
		{
			const int x = ptCursor.x;
			const int y = ptCursor.y;

			int dx = 0;
			int dy = 0;

			if (x < rect.left)
			{
				dx = rect.left - x;
			}
			else if (x > rect.right)
			{
				dx = x - rect.right;
			}

			if (y < rect.top)
			{
				dy = rect.top - y;
			}
			else if (y > rect.bottom)
			{
				dy = y - rect.bottom;
			}

			const int nDistance = max (dx, dy);
			const int nRange = m_bWasHovered ? 22 : 11;
			const int nDimissDistance = m_bWasHovered ? 176 : 44;

			if (nDistance > nDimissDistance)
			{
				PostMessage (WM_CLOSE);
				return FALSE;
			}

			if (nDistance < nRange)
			{
				nTransparency = 200;
			}
			else if (nDistance < 2 * nRange)
			{
				nTransparency = 100;
			}
			else if (nDistance < 3 * nRange)
			{
				nTransparency = 50;
			}
		}
	}

	if (m_nTransparency == nTransparency)
	{
		return FALSE;
	}

	m_nTransparency = nTransparency;
	return TRUE;
}
//************************************************************************************
void CBCGPRibbonFloaty::CancelContextMenuMode ()
{
	ASSERT_VALID (this);

	if (!m_bContextMenuMode)
	{
		return;
	}

	m_bContextMenuMode = FALSE;
	SetTimer (ID_VISIBILITY_TIMER, 100, NULL);

	ModifyStyleEx (0, WS_EX_LAYERED);

	UpdateTransparency ();

	globalData.SetLayeredAttrib (GetSafeHwnd (), 0, 
		m_nTransparency, LWA_ALPHA);

	UpdateShadowTransparency (m_nTransparency);
}
//************************************************************************************
HRESULT CBCGPRibbonFloaty::get_accName(VARIANT varChild, BSTR *pszName)
{
	if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
	{
		CString str = _T("RibbonMiniToolbar");
		*pszName = str.AllocSysString ();
		return S_OK;
	}

	return CBCGPRibbonPanelMenu::get_accName(varChild, pszName);
}
/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonFloaty message handlers

#endif // BCGP_EXCLUDE_RIBBON
