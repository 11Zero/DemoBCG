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
// BCGPRibbonKeyTip.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "multimon.h"

#ifndef BCGP_EXCLUDE_RIBBON

#include "bcgglobals.h"
#include "BCGPRibbonKeyTip.h"
#include "BCGPBaseRibbonElement.h"
#include "BCGPVisualManager.h"
#include "BCGPPopupMenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonKeyTip

CString CBCGPRibbonKeyTip::m_strClassName;

CBCGPRibbonKeyTip::CBCGPRibbonKeyTip(CBCGPBaseRibbonElement* pElement, BOOL bIsMenu)
{
	ASSERT_VALID (pElement);
	m_pElement = pElement;

	m_bIsMenu = bIsMenu;

	m_rectScreen.SetRectEmpty ();
}

CBCGPRibbonKeyTip::~CBCGPRibbonKeyTip()
{
}


BEGIN_MESSAGE_MAP(CBCGPRibbonKeyTip, CWnd)
	//{{AFX_MSG_MAP(CBCGPRibbonKeyTip)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_MOUSEACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonKeyTip message handlers

void CBCGPRibbonKeyTip::OnPaint() 
{
	ASSERT_VALID (m_pElement);

	CPaintDC dc(this); // device context for painting

	CBCGPMemDC memDC (dc, this);
	CDC* pDC = &memDC.GetDC ();

	CFont* pOldFont = pDC->SelectObject (&globalData.fontRegular);
	ASSERT (pOldFont != NULL);

	pDC->SetBkMode (TRANSPARENT);

	CRect rect;
	GetClientRect (rect);

	m_pElement->OnDrawKeyTip (pDC, rect, m_bIsMenu);

	pDC->SelectObject (pOldFont);
}
//***************************************************************************
BOOL CBCGPRibbonKeyTip::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//***************************************************************************
BOOL CBCGPRibbonKeyTip::Show (BOOL bRepos)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pElement);

	if (GetSafeHwnd () != NULL && !bRepos)
	{
		ShowWindow (SW_SHOWNOACTIVATE);
		return TRUE;
	}

	CWnd* pWndParent = m_pElement->GetParentWnd ();

	if (pWndParent->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	CClientDC dc (NULL);

	CFont* pOldFont = dc.SelectObject (&globalData.fontRegular);
	ASSERT (pOldFont != NULL);

	CRect rect = m_pElement->GetKeyTipRect (&dc, m_bIsMenu);

	dc.SelectObject (pOldFont);

	if (rect.IsRectEmpty ())
	{
		return FALSE;
	}

	pWndParent->ClientToScreen (&rect);

	//-------------------------
	// Normalize inside screen:
	//-------------------------
	CRect rectScreen;

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);
	if (GetMonitorInfo (MonitorFromPoint (rect.TopLeft (), MONITOR_DEFAULTTONEAREST),
		&mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	if (rect.right > rectScreen.right)
	{
		rect.OffsetRect (rectScreen.right - rect.right, 0);
	}
	else if (rect.left < rectScreen.left)
	{
		rect.OffsetRect (rectScreen.left - rect.left, 0);
	}

	if (rect.bottom > rectScreen.bottom)
	{
		rect.OffsetRect (0, rectScreen.bottom - rect.bottom);
	}
	else if (rect.top < rectScreen.top)
	{
		rect.OffsetRect (rectScreen.top - rect.top, 0);
	}

	if (m_pElement->IsScrolledOut ())
	{
		if (GetSafeHwnd () != NULL)
		{
			ShowWindow (SW_HIDE);
			UpdateMenuShadow ();
		}

		return TRUE;
	}

	if (GetSafeHwnd () == NULL)
	{
		if (m_strClassName.IsEmpty ())
		{
			m_strClassName = ::AfxRegisterWndClass (
				CS_SAVEBITS,
				::LoadCursor(NULL, IDC_ARROW),
				(HBRUSH)(COLOR_BTNFACE + 1), NULL);
		}

		DWORD dwStyleEx = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;

		if (m_pElement->IsDisabled () &&
			CBCGPVisualManager::GetInstance ()->IsLayeredRibbonKeyTip ())
		{
			dwStyleEx |= WS_EX_LAYERED;
		}

		if (!CreateEx (dwStyleEx, m_strClassName, _T(""), WS_POPUP, rect, NULL, 0))
		{
			return FALSE;
		}

		m_rectScreen = rect;

		if (dwStyleEx & WS_EX_LAYERED)
		{
			globalData.SetLayeredAttrib (GetSafeHwnd (), 0, 128, LWA_ALPHA);
		}
	}
	else
	{
		SetWindowPos (NULL, rect.left, rect.top, -1, -1, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	ShowWindow (SW_SHOWNOACTIVATE);
	return TRUE;
}
//***************************************************************************
void CBCGPRibbonKeyTip::Hide ()
{
	ASSERT_VALID (this);

	if (GetSafeHwnd () != NULL && IsWindowVisible ())
	{
		ShowWindow (SW_HIDE);
		UpdateMenuShadow ();
	}
}
//***************************************************************************
void CBCGPRibbonKeyTip::UpdateMenuShadow ()
{
	CWnd* pMenu = CBCGPPopupMenu::GetActiveMenu ();

	if (pMenu != NULL && CWnd::FromHandlePermanent (pMenu->GetSafeHwnd ()) != NULL &&
		!m_rectScreen.IsRectEmpty ())
	{
		CBCGPPopupMenu::UpdateAllShadows (m_rectScreen);
	}
}
//***************************************************************************
void CBCGPRibbonKeyTip::OnDestroy() 
{
	if (IsWindowVisible ())
	{
		ShowWindow (SW_HIDE);
		UpdateMenuShadow ();
	}

	CWnd::OnDestroy();
}
//***************************************************************************
int CBCGPRibbonKeyTip::OnMouseActivate(CWnd* /*pDesktopWnd*/, UINT /*nHitTest*/, UINT /*message*/) 
{
	return MA_NOACTIVATE;
}

#endif // BCGP_EXCLUDE_RIBBON
