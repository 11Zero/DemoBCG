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
// BCGPSDPlaceMarkerWnd.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPSDPlaceMarkerWnd.h"
#include "BCGPSmartDockingManager.h"
#include "BCGPDrawManager.h"
#include "bcgglobals.h"
#include "BCGPTabbedControlBar.h"
#include "BCGPDockManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPSDPlaceMarkerWnd

CBCGPSDPlaceMarkerWnd::CBCGPSDPlaceMarkerWnd () :
    m_bTabbed (FALSE),
    m_bShown (FALSE),
	m_pWndOwner (NULL),
	m_bUseThemeColorInShading (FALSE)
{
	m_rectLast.SetRectEmpty ();
	m_rectTab.SetRectEmpty ();
}

CBCGPSDPlaceMarkerWnd::~CBCGPSDPlaceMarkerWnd()
{
}

void CBCGPSDPlaceMarkerWnd::Create (CWnd* pwndOwner)
{
	ASSERT_VALID (pwndOwner);

	m_pWndOwner = pwndOwner;

    CRect rect;
	rect.SetRectEmpty ();

    DWORD dwExStyle = (globalData.IsWindowsLayerSupportAvailable () && globalData.m_nBitsPerPixel > 8) ? 
		WS_EX_LAYERED : 0;

	CreateEx (dwExStyle, GetSDWndClassName<0>(),
		_T(""), WS_POPUP, rect, pwndOwner, NULL);

	if (dwExStyle == WS_EX_LAYERED)
	{
		globalData.SetLayeredAttrib (GetSafeHwnd(), 0, 100, LWA_ALPHA);
	}

	CBCGPSmartDockingParams& params = CBCGPDockManager::GetSmartDockingParams ();
	m_bUseThemeColorInShading = params.m_bUseThemeColorInShading;
}

void CBCGPSDPlaceMarkerWnd::ShowAt (CRect rect)
{
    if (m_bTabbed || m_rectLast != rect)
    {
        Hide ();

        if (m_bTabbed)
        {
			SetWindowRgn (NULL, FALSE);
            m_bTabbed = FALSE;
        }

        SetWindowPos (&CWnd::wndTop, rect.left, rect.top, rect.Width(), rect.Height(),
            SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOREDRAW);

        m_rectLast = rect;

		m_bShown = TRUE;
        RedrawWindow ();
    }
}

void CBCGPSDPlaceMarkerWnd::Hide ()
{
    if (m_bShown)
    {
        ShowWindow (SW_HIDE);
        m_bShown = FALSE;

		if (m_pWndOwner != NULL)
		{
			m_pWndOwner->UpdateWindow ();
		}

		if (m_pDockingWnd != NULL)
		{
			m_pDockingWnd->UpdateWindow ();
		}

		m_rectLast.SetRectEmpty ();
		m_rectTab.SetRectEmpty ();
    }
}

void CBCGPSDPlaceMarkerWnd::ShowTabbedAt (CRect rect, CRect rectTab)
{
    if (!m_bTabbed || m_rectLast != rect || m_rectTab != rectTab)
    {
        Hide ();

        CRgn rgnMain;
        CBCGPTabbedControlBar::m_bTabsAlwaysTop ? 
			rgnMain.CreateRectRgn (0, rectTab.Height (), rect.Width(), rect.Height() + rectTab.Height ()) :
			rgnMain.CreateRectRgn (0, 0, rect.Width(), rect.Height());

        CRgn rgnTab;
        if (CBCGPTabbedControlBar::m_bTabsAlwaysTop)
		{
			rgnTab.CreateRectRgn (rectTab.left, 0, rectTab.Width (), rectTab.Height ());
		}
		else
		{
			rgnTab.CreateRectRgnIndirect (rectTab);
		}

        rgnMain.CombineRgn (&rgnMain, &rgnTab, RGN_OR);
        SetWindowRgn (rgnMain, FALSE);

        m_bTabbed = TRUE;

        m_rectLast = rect;
		m_rectTab = rectTab;

		if (CBCGPTabbedControlBar::m_bTabsAlwaysTop)
		{
			SetWindowPos (&CWnd::wndTop, rect.left, rectTab.top, 
				rect.Width(), rect.Height() + m_rectTab.Height (),
				SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOREDRAW);
		}
		else
		{
			SetWindowPos (&CWnd::wndTop, rect.left, rect.top, 
				rect.Width(), rect.Height() + m_rectTab.Height (),
				SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOREDRAW);
		}
		m_bShown = TRUE;
        RedrawWindow ();
    }
}

BEGIN_MESSAGE_MAP(CBCGPSDPlaceMarkerWnd, CWnd)
	//{{AFX_MSG_MAP(CBCGPSDPlaceMarkerWnd)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPSDPlaceMarkerWnd message handlers

void CBCGPSDPlaceMarkerWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if (!m_bShown)
	{
		return;
	}

    CRect rect;
    GetClientRect (rect);

	COLORREF colorFill = m_bUseThemeColorInShading ?
		globalData.clrActiveCaption : RGB (47, 103, 190);

    if (globalData.IsWindowsLayerSupportAvailable () && globalData.m_nBitsPerPixel > 8)
    {
        CBrush brFill (CBCGPDrawManager::PixelAlpha (colorFill, 105));
		dc.FillRect (rect, &brFill);
    }
    else
    {
		CBrush brFill (CBCGPDrawManager::PixelAlpha (RGB (
			255 - GetRValue (colorFill), 
			255 - GetGValue (colorFill), 
			255 - GetBValue (colorFill)), 
			50));

		CBrush* pBrushOld = dc.SelectObject (&brFill);
		dc.PatBlt (0, 0, rect.Width (), rect.Height (), PATINVERT);
		dc.SelectObject (pBrushOld);
    }
}

void CBCGPSDPlaceMarkerWnd::OnClose() 
{
    // do nothing
}

BOOL CBCGPSDPlaceMarkerWnd::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
