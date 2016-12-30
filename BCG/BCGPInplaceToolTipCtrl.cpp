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

// BCGPInplaceToolTipCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "multimon.h"
#include "bcgcbpro.h"
#include "BCGGlobals.h"
#include "BCGPInplaceToolTipCtrl.h"

#ifndef _BCGSUITE_
	#include "BCGPToolTipCtrl.h"
	#define visualManagerMFC	CBCGPVisualManager::GetInstance ()
#else
	#define visualManagerMFC	CMFCVisualManager::GetInstance ()
#endif

#include "BCGPVisualManager.h"
#include "BCGPDrawManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPInplaceToolTipCtrl

CString CBCGPInplaceToolTipCtrl::m_strClassName;

IMPLEMENT_DYNAMIC(CBCGPInplaceToolTipCtrl, CWnd)

CBCGPInplaceToolTipCtrl::CBCGPInplaceToolTipCtrl()
{
	m_rectLast.SetRectEmpty ();
	m_nTextMargin = 10;
	m_pFont	= NULL;
	m_pWndParent = NULL;
	m_bMultiline = FALSE;
}

CBCGPInplaceToolTipCtrl::~CBCGPInplaceToolTipCtrl()
{
}


BEGIN_MESSAGE_MAP(CBCGPInplaceToolTipCtrl, CWnd)
	//{{AFX_MSG_MAP(CBCGPInplaceToolTipCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETFONT, OnSetFont)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPInplaceToolTipCtrl message handlers

BOOL CBCGPInplaceToolTipCtrl::Create (CWnd* pWndParent) 
{
	ASSERT_VALID (pWndParent);
	m_pWndParent = pWndParent;

	if (m_strClassName.IsEmpty ())
	{
		m_strClassName = ::AfxRegisterWndClass (
			CS_SAVEBITS,
			::LoadCursor(NULL, IDC_ARROW),
			(HBRUSH)(COLOR_BTNFACE + 1));
	}  

	return CreateEx (0,
					m_strClassName, _T (""), WS_POPUP,
					0, 0, 0, 0,
					pWndParent->GetSafeHwnd (), (HMENU) NULL);
}

BOOL CBCGPInplaceToolTipCtrl::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

void CBCGPInplaceToolTipCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rect;
	GetClientRect (rect);

	CBCGPToolTipParams params;
	visualManagerMFC->GetToolTipParams (params);

	COLORREF clrLine = params.m_clrBorder == (COLORREF)-1 ?
		::GetSysColor (COLOR_INFOTEXT) : params.m_clrBorder;

	COLORREF clrText = params.m_clrText == (COLORREF)-1 ?
		::GetSysColor (COLOR_INFOTEXT) : params.m_clrText;

	if (params.m_clrFill == (COLORREF)-1)
	{
#ifndef _BCGSUITE_
		CRect rectFill = rect;
		rectFill.InflateRect(1, 1);

		CBCGPToolTipCtrl dummy;
		visualManagerMFC->OnFillToolTip (&dc, &dummy, rectFill, clrText, clrLine);
#else
		::FillRect (dc.GetSafeHdc (), rect, ::GetSysColorBrush (COLOR_INFOBK));
#endif
	}
	else
	{
		if (params.m_clrFillGradient == (COLORREF)-1)
		{
			CBrush br (params.m_clrFill);
			dc.FillRect (rect, &br);
		}
		else
		{
			CBCGPDrawManager dm (dc);

			dm.FillGradient2 (rect, 
				params.m_clrFillGradient, params.m_clrFill,
				params.m_nGradientAngle == -1 ? 90 : params.m_nGradientAngle);
		}
	}

	dc.Draw3dRect (rect, clrLine, clrLine);

	CFont* pPrevFont = m_pFont == NULL ?
		(CFont*) dc.SelectStockObject (DEFAULT_GUI_FONT) :
		dc.SelectObject (m_pFont);

	dc.SetBkMode (TRANSPARENT);
	dc.SetTextColor(clrText);

	if (m_strText.FindOneOf (_T("\n")) != -1 || m_bMultiline)	// multi-line tooltip
	{
		rect.DeflateRect (m_nTextMargin, m_nTextMargin);
		if (rect.Height () < m_rectLast.Height () && !m_bMultiline)
		{
			// center tooltip vertically
			rect.top += (m_rectLast.Height () - rect.Height ()) / 2;
		}

		dc.DrawText (m_strText, rect, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);
	}
	else // single line tooltip
	{
		rect.DeflateRect (m_nTextMargin, 0);
		dc.DrawText (m_strText, rect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
	}

	if (pPrevFont != NULL)
	{
		dc.SelectObject (pPrevFont);
	}
}
//*******************************************************************************************
void CBCGPInplaceToolTipCtrl::Track (CRect rect, const CString& strText)
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	if (m_rectLast == rect && m_strText == strText)
	{
		return;
	}

	ASSERT_VALID (m_pWndParent);

	m_rectLast = rect;
	m_strText = strText;

	CClientDC dc (this);

    ASSERT_VALID(m_pFont);
	CFont* pPrevFont = m_pFont == NULL ?
		(CFont*) dc.SelectStockObject (DEFAULT_GUI_FONT) :
		dc.SelectObject (m_pFont);
	ASSERT (pPrevFont != NULL);

	int nTextHeight = rect.Height ();
	int nTextWidth = rect.Width ();
	if (m_strText.FindOneOf (_T("\n")) != -1 || m_bMultiline)	// multi-line tooltip
	{
		const int nDefaultHeight = globalData.GetTextHeight ();
		const int nDefaultWidth = 200;
		CRect rectText (0, 0, nDefaultWidth, nDefaultHeight);
 
		nTextHeight = dc.DrawText (m_strText, rectText, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
		nTextWidth = rectText.Width ();
		nTextHeight += 2 * m_nTextMargin;
		nTextWidth += 2 * m_nTextMargin;
	}
	else
	{
		nTextWidth = dc.GetTextExtent (m_strText).cx + 2 * m_nTextMargin;
	}

	dc.SelectObject (pPrevFont);

	if (m_bMultiline)
	{
		TEXTMETRIC tm;
		dc.GetTextMetrics (&tm);
		int nLines = max (1, m_rectLast.Height () / (int)tm.tmHeight);

		nTextWidth = nTextWidth / nLines * 3 / 2;

		if (nTextHeight > m_rectLast.Height ())
		{
			nTextWidth = max (m_rectLast.Width (), nTextWidth);
		}
	}

	if (m_pWndParent->GetExStyle () & WS_EX_LAYOUTRTL)
	{
		rect.left = rect.right - nTextWidth;
	}
	else
	{
		rect.right = rect.left + nTextWidth;
	}
	rect.bottom = rect.top + nTextHeight;
	if (rect.Height () < m_rectLast.Height ())
	{
		rect.top = m_rectLast.top;
		rect.bottom = m_rectLast.bottom;
	}

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);

	CRect rectScreen;

	if (GetMonitorInfo (MonitorFromPoint (rect.TopLeft (), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	if (rect.Width () > rectScreen.Width ())
	{
		rect.left = rectScreen.left;
		rect.right = rectScreen.right;
	}
	else if (rect.right > rectScreen.right)
	{
		rect.right = rectScreen.right;
		rect.left = rect.right - nTextWidth;
	}
	else if (rect.left < rectScreen.left)
	{
		rect.left = rectScreen.left;
		rect.right = rect.left + nTextWidth;
	}

	if (rect.Height () > rectScreen.Height ())
	{
		rect.top = rectScreen.top;
		rect.bottom = rectScreen.bottom;
	}
	else if (rect.bottom > rectScreen.bottom)
	{
		rect.bottom = rectScreen.bottom;
		rect.top = rect.bottom - nTextHeight;
	}
	else if (rect.top < rectScreen.top)
	{
		rect.top = rectScreen.top;
		rect.bottom = rect.bottom + nTextHeight;
	}

	SetWindowPos (&wndTop, rect.left, rect.top, 
		rect.Width (), rect.Height (), SWP_NOACTIVATE | SWP_NOOWNERZORDER);
  
    ShowWindow (SW_SHOWNOACTIVATE);
	Invalidate ();
	UpdateWindow ();

	SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_ARROW));
}
//*******************************************************************************************
void CBCGPInplaceToolTipCtrl::Hide ()
{
	if (GetSafeHwnd () != NULL)
	{
		ShowWindow (SW_HIDE);
	}
}
//*******************************************************************************************
void CBCGPInplaceToolTipCtrl::Deactivate ()
{
	m_strText.Empty ();
	m_rectLast.SetRectEmpty ();

	Hide ();
}
//*****************************************************************************
LRESULT CBCGPInplaceToolTipCtrl::OnSetFont (WPARAM wParam, LPARAM lParam)
{
	BOOL bRedraw = (BOOL) LOWORD (lParam);

    m_pFont = CFont::FromHandle((HFONT) wParam);

	if (bRedraw)
	{
		Invalidate ();
		UpdateWindow ();
	}

	return 0;
}
//***************************************************************************
BOOL CBCGPInplaceToolTipCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message >= WM_MOUSEFIRST &&
		pMsg->message <= WM_MOUSELAST)
	{
		if (pMsg->message != WM_MOUSEMOVE)
		{
			Hide ();
		}

		ASSERT_VALID (m_pWndParent);

		// the parent should receive the mouse message in its client coordinates
		CPoint pt(BCG_GET_X_LPARAM(pMsg->lParam), BCG_GET_Y_LPARAM(pMsg->lParam));
		MapWindowPoints (m_pWndParent, &pt, 1);
		LPARAM lParam = MAKELPARAM (pt.x, pt.y);

		m_pWndParent->SendMessage (pMsg->message, pMsg->wParam, lParam);
		return TRUE;
	}
	
	return CWnd::PreTranslateMessage(pMsg);
}

BOOL CBCGPInplaceToolTipCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if (m_pWndParent->GetSafeHwnd() != NULL)
	{
		return (BOOL)m_pWndParent->SendMessage(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
	}
	
	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}
