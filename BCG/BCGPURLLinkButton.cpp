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

// BCGURLLinkButton.cpp : implementation file
//

#include "stdafx.h"
#include "BCGCBPro.h"
#include "BCGPURLLinkButton.h"
#include "BCGGlobals.h"
#include "BCGPLocalResource.h"
#include "bcgprores.h"
#include "BCGPVisualManager.h"
#include "BCGPDrawManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CBCGPURLLinkButton, CBCGPButton)

/////////////////////////////////////////////////////////////////////////////
// CBCGPURLLinkButton

CBCGPURLLinkButton::CBCGPURLLinkButton()
{
	m_nFlatStyle = BUTTONSTYLE_NOBORDERS;
	m_sizePushOffset = CSize (0, 0);
	m_bTransparent = TRUE;

	m_bMultilineText = FALSE;
	m_bAlwaysUnderlineText = TRUE;
	m_bDefaultClickProcess = FALSE;

	m_clrTextCustom = (COLORREF)-1;
	m_clrHoverTextCustom = (COLORREF)-1;

	SetMouseCursorHand ();
}

CBCGPURLLinkButton::~CBCGPURLLinkButton()
{
}

BEGIN_MESSAGE_MAP(CBCGPURLLinkButton, CBCGPButton)
	//{{AFX_MSG_MAP(CBCGPURLLinkButton)
	ON_CONTROL_REFLECT_EX(BN_CLICKED, OnClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPURLLinkButton message handlers

void CBCGPURLLinkButton::OnDraw (CDC* pDC, const CRect& rect, UINT /*uiState*/)
{
	ASSERT_VALID (pDC);

	// Set font:
	CFont* pOldFont = NULL;
		
	if (m_bAlwaysUnderlineText || m_bHover)
	{
		pOldFont = pDC->SelectObject (&globalData.fontDefaultGUIUnderline);
	}
	else
	{
		pOldFont = CBCGPButton::SelectFont (pDC);
	}

	ASSERT (pOldFont != NULL);

	// Set text parameters:
	COLORREF clrText = m_bHover ? m_clrHoverTextCustom : m_clrTextCustom;
	if (clrText == (COLORREF)-1)
	{
		clrText = CBCGPVisualManager::GetInstance ()->GetURLLinkColor (this, m_bHover);
	}

	pDC->SetTextColor(clrText);
	pDC->SetBkMode (TRANSPARENT);

	// Obtain label:
	CString strLabel;
	GetWindowText (strLabel);

	DWORD uiDTFlags = DT_WORDBREAK;
	if (!m_bMultilineText)
	{
		uiDTFlags = DT_SINGLELINE | (m_nAlignStyle == ALIGN_LEFT ? DT_LEFT : (m_nAlignStyle == ALIGN_RIGHT ? DT_RIGHT : DT_CENTER));
	}

	CRect rectText = rect;
	pDC->DrawText (strLabel, rectText, uiDTFlags);

	pDC->SelectObject (pOldFont);
}
//******************************************************************************************
BOOL CBCGPURLLinkButton::OnClicked() 
{
	ASSERT_VALID (this);

	if (!IsWindowEnabled ())
	{
		return TRUE;
	}

	if (m_bDefaultClickProcess)
	{
		m_bHover = FALSE;
		Invalidate ();
		UpdateWindow ();

		return FALSE;
	}

	CWaitCursor wait;

	CString strURL = m_strURL;
	if (strURL.IsEmpty ())
	{
		GetWindowText (strURL);
	}

	if (::ShellExecute (NULL, NULL, m_strPrefix + strURL, NULL, NULL, NULL) < (HINSTANCE) 32)
	{
		TRACE(_T("Can't open URL: %s\n"), strURL);
	}

	m_bHover = FALSE;
	Invalidate ();
	UpdateWindow ();

	return TRUE;
}
//*******************************************************************************************
void CBCGPURLLinkButton::SetURL (LPCTSTR lpszURL)
{
	if (lpszURL == NULL)
	{
		m_strURL.Empty ();
	}
	else
	{
		m_strURL = lpszURL;
	}
}
//*******************************************************************************************
void CBCGPURLLinkButton::SetURLPrefix (LPCTSTR lpszPrefix)
{
	ASSERT (lpszPrefix != NULL);
	m_strPrefix = lpszPrefix;
}
//*******************************************************************************************
CSize CBCGPURLLinkButton::SizeToContent (BOOL bVCenter, BOOL bHCenter)
{
	if (m_sizeImage != CSize (0, 0))
	{
		return CBCGPButton::SizeToContent ();
	}

	ASSERT_VALID (this);
	ASSERT (GetSafeHwnd () != NULL);

	CClientDC dc (this);

	// Set font:
	CFont* pOldFont = dc.SelectObject (&globalData.fontDefaultGUIUnderline);
	ASSERT (pOldFont != NULL);

	// Obtain label:
	CString strLabel;
	GetWindowText (strLabel);

	CRect rectClient;
	GetClientRect (rectClient);

	CRect rectText = rectClient;
	dc.DrawText (strLabel, rectText, DT_SINGLELINE | DT_CALCRECT);
	rectText.InflateRect (3, 3);

	if (bVCenter || bHCenter)
	{
		ASSERT (GetParent ()->GetSafeHwnd () != NULL);
		MapWindowPoints (GetParent (), rectClient);

		int dx = bHCenter ? (rectClient.Width () - rectText.Width ()) / 2 : 0;
		int dy = bVCenter ? (rectClient.Height () - rectText.Height ()) / 2 : 0;

		SetWindowPos (NULL, rectClient.left + dx, rectClient.top + dy, 
			rectText.Width (), rectText.Height (),
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		SetWindowPos (NULL, -1, -1, rectText.Width (), rectText.Height (),
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	dc.SelectObject (pOldFont);
	return rectText.Size ();
}
//*****************************************************************************
void CBCGPURLLinkButton::OnDrawFocusRect (CDC* pDC, const CRect& rectClient)
{
	ASSERT_VALID (pDC);

	CRect rectFocus = rectClient;

	if (m_bOnGlass)
	{
		CBCGPDrawManager dm (*pDC);
		dm.DrawFocusRect(rectFocus);
	}
	else
	{
		pDC->DrawFocusRect (rectFocus);
	}
}
//****************************************************************************************
BOOL CBCGPURLLinkButton::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (pMsg->wParam == VK_SPACE || pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
		break;

	case WM_KEYUP:
		if (pMsg->wParam == VK_SPACE)
		{
			return TRUE;
		}
		else if (pMsg->wParam == VK_RETURN)
		{
			OnClicked ();
			return TRUE;
		}
		break;
	}

	return CBCGPButton::PreTranslateMessage (pMsg);
}
//****************************************************************************************
void CBCGPURLLinkButton::SetCustomTextColors(COLORREF clrText, COLORREF clrHoverText)
{
	m_clrTextCustom = clrText;
	m_clrHoverTextCustom = clrHoverText;
}
