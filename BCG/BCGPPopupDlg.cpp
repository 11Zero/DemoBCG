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
// BCGPPopupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "multimon.h"
#include "BCGPPopupDlg.h"

#ifndef BCGP_EXCLUDE_POPUP_WINDOW

#include "BCGPPopupWindow.h"
#include "BCGProRes.h"
#include "BCGPLocalResource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_CLASS_NAME	255
#define STATIC_CLASS	_T("Static")
#define BUTTON_CLASS	_T("Button")

#define MAX_TEXT_LEN	512

IMPLEMENT_DYNCREATE (CBCGPPopupDlg, CBCGPDialog)

/////////////////////////////////////////////////////////////////////////////
// CBCGPPopupDlg

CBCGPPopupDlg::CBCGPPopupDlg()
{
	m_pParentPopup = NULL;
	m_bDefault = FALSE;
	m_sizeDlg = CSize (0, 0);
	m_bDontSetFocus = FALSE;
	m_bMenuIsActive = FALSE;
}

CBCGPPopupDlg::~CBCGPPopupDlg()
{
}


BEGIN_MESSAGE_MAP(CBCGPPopupDlg, CBCGPDialog)
	//{{AFX_MSG_MAP(CBCGPPopupDlg)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPPopupDlg message handlers

HBRUSH CBCGPPopupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkMode(TRANSPARENT);

		if (globalData.IsHighContastMode ())
		{
			pDC->SetTextColor (globalData.clrWindowText);
		}
		else
		{
			COLORREF clrText = globalData.clrBarText;

			if (m_pParentPopup != NULL)
			{
				ASSERT_VALID(m_pParentPopup);

				COLORREF clrTxtParent = m_pParentPopup->GetTextColor(this, FALSE, FALSE);
				if (clrTxtParent != (COLORREF)-1)
				{
					clrText = clrTxtParent;
				}
			}

			pDC->SetTextColor(clrText);
		}

		return (HBRUSH) ::GetStockObject (HOLLOW_BRUSH);
	}

	return CBCGPDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CBCGPPopupDlg::OnEraseBkgnd(CDC* pDC)
{
	if (!globalData.IsWinXPDrawParentBackground())
	{
		CRect rectClient;
		GetClientRect (&rectClient);
		
		CBCGPVisualManager::GetInstance ()->OnFillPopupWindowBackground(pDC, rectClient);
	}

	return TRUE;
}

void CBCGPPopupDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CBCGPMemDC memDC (dc, this);
	CDC* pDC = &memDC.GetDC ();

	OnDraw (pDC);
}

void CBCGPPopupDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CBCGPDialog::OnLButtonDown(nFlags, point);

	GetParent ()->SendMessage (WM_LBUTTONDOWN, 0, MAKELPARAM (point.x, point.y));
	SetFocus ();
}

BOOL CBCGPPopupDlg::HasFocus () const
{
	if (GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	if (m_bMenuIsActive)
	{
		return TRUE;
	}

	CWnd* pWndMain = AfxGetMainWnd ();
	if (pWndMain->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	if (pWndMain->IsIconic () ||
		!pWndMain->IsWindowVisible() ||
		pWndMain != GetForegroundWindow ())
	{
		return FALSE;
	}

    CWnd* pFocus = GetFocus();

    BOOL bActive = (pFocus->GetSafeHwnd () != NULL && 
		(IsChild (pFocus) || pFocus->GetSafeHwnd () == GetSafeHwnd ()));

	return bActive;
}

BOOL CBCGPPopupDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	ASSERT_VALID (m_pParentPopup);

	HWND hWndCtrl = (HWND)lParam;
	if (hWndCtrl == NULL && (wParam == IDCANCEL || wParam == IDOK)) // Enter or ESC were pressed
	{
		m_pParentPopup->SendMessage(WM_CLOSE);
		return TRUE;
	}

	if (m_pParentPopup->ProcessCommand ((HWND)lParam))
	{
		return TRUE;
	}

	if (m_btnURL.GetSafeHwnd () == hWndCtrl &&
		m_btnURL.GetDlgCtrlID () == LOWORD (wParam) &&
		m_pParentPopup->OnClickLinkButton (m_btnURL.GetDlgCtrlID ()))
	{
		return TRUE;
	}
	
	return CBCGPDialog::OnCommand(wParam, lParam);
}

int CBCGPPopupDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_pParentPopup = DYNAMIC_DOWNCAST (CBCGPPopupWindow, GetParent ());
	ASSERT_VALID (m_pParentPopup);
	
	return 0;
}

BOOL CBCGPPopupDlg::OnInitDialog() 
{
	CBCGPDialog::OnInitDialog();
	
	CWnd* pWndChild = GetWindow (GW_CHILD);
	while (pWndChild != NULL)
	{
		ASSERT_VALID (pWndChild);

		CBCGPButton* pButton = DYNAMIC_DOWNCAST(CBCGPButton, pWndChild);
		if (pButton != NULL)
		{
			pButton->m_bDrawFocus = FALSE;

			if (m_pParentPopup != NULL)
			{
				CBCGPURLLinkButton* pLink = DYNAMIC_DOWNCAST(CBCGPURLLinkButton, pWndChild);
				if (pLink != NULL)
				{
					pLink->SetCustomTextColors(m_pParentPopup->GetLinkTextColor(this, FALSE), m_pParentPopup->GetLinkTextColor(this, TRUE));
				}
			}
		}
		else
		{
			TCHAR lpszClassName [MAX_CLASS_NAME + 1];

			::GetClassName (pWndChild->GetSafeHwnd (), lpszClassName, MAX_CLASS_NAME);
			CString strClass = lpszClassName;

			if (strClass == STATIC_CLASS && (pWndChild->GetStyle () & SS_ICON) == SS_ICON)
			{
				pWndChild->ShowWindow (SW_HIDE);
			}
		}

		pWndChild = pWndChild->GetNextWindow ();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CBCGPPopupDlg::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if (lp & PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle ((HDC) wp);
		ASSERT_VALID (pDC);

		OnDraw (pDC);
	}

	return 0;
}

void CBCGPPopupDlg::OnDraw (CDC* pDC)
{
	ASSERT_VALID (pDC);

	CRect rectClient;
	GetClientRect (&rectClient);

	if (m_pParentPopup == NULL || !m_pParentPopup->OnFillBackground(pDC, this, rectClient))
	{
		CBCGPVisualManager::GetInstance ()->OnFillPopupWindowBackground(pDC, rectClient);
	}

	CWnd* pWndChild = GetWindow (GW_CHILD);

	while (pWndChild != NULL)
	{
		ASSERT_VALID (pWndChild);

		TCHAR lpszClassName [MAX_CLASS_NAME + 1];

		::GetClassName (pWndChild->GetSafeHwnd (), lpszClassName, MAX_CLASS_NAME);
		CString strClass = lpszClassName;

		if (strClass == STATIC_CLASS && (pWndChild->GetStyle () & SS_ICON) == SS_ICON)
		{
			CRect rectIcon;
			pWndChild->GetWindowRect (rectIcon);
			ScreenToClient (rectIcon);

			HICON hIcon = ((CStatic*) pWndChild)->GetIcon ();
			pDC->DrawIcon (rectIcon.TopLeft (), hIcon);
		}

		pWndChild = pWndChild->GetNextWindow ();
	}
}

CSize CBCGPPopupDlg::GetOptimalTextSize (CString str)
{
	if (str.IsEmpty ())
	{
		return CSize (0, 0);
	}

	CRect rectScreen;

	CRect rectDlg;
	GetWindowRect (rectDlg);

	MONITORINFO mi;
	mi.cbSize = sizeof (MONITORINFO);
	if (GetMonitorInfo (MonitorFromPoint (rectDlg.TopLeft (), MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		::SystemParametersInfo (SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	CClientDC dc (this);

	CFont* pOldFont = dc.SelectObject (&globalData.fontRegular);
	ASSERT_VALID (pOldFont);

	int nStepY = globalData.GetTextHeight ();
	int nStepX = nStepY * 3;

	CRect rectText (0, 0, nStepX, nStepY);

	for (;;)
	{
		CRect rectTextSaved = rectText;

		int nHeight = dc.DrawText (str, rectText, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX);
		int nWidth = rectText.Width ();

		rectText = rectTextSaved;

		if (nHeight <= rectText.Height () ||
			rectText.Width () > rectScreen.Width () ||
			rectText.Height () > rectScreen.Height ())
		{
			rectText.bottom = rectText.top + nHeight + 5;
			rectText.right = rectText.left + nWidth + 5;
			break;
		}

		rectText.right += nStepX;
		rectText.bottom += nStepY;
	}

	dc.SelectObject (pOldFont);
	return rectText.Size ();
}

BOOL CBCGPPopupDlg::CreateFromParams (CBCGPPopupWndParams& params, CBCGPPopupWindow* pParent)
{
	CBCGPLocalResource	lr;

	if (!Create (IDD_BCGBARRES_POPUP_DLG, pParent))
	{
		return FALSE;
	}

	m_Params = params;

	int xMargin = 10;
	int yMargin = 10;

	double dblScale = globalData.GetRibbonImageScale ();
	if (dblScale != 1.)
	{
		xMargin = (int) (.5 + dblScale * xMargin);
		yMargin = (int) (.5 + dblScale * yMargin);
	}

	int x = xMargin;
	int y = yMargin;

	int cxIcon = 0;
	int cyIcon = 0;

	CString strText = m_Params.m_strText;
	if (strText.GetLength () > MAX_TEXT_LEN)
	{
		strText = strText.Left (MAX_TEXT_LEN - 1);
		strText += _T("...");
	}

	CString strURL = m_Params.m_strURL;
	if (strURL.GetLength () > MAX_TEXT_LEN)
	{
		strURL = strURL.Left (MAX_TEXT_LEN - 1);
		strURL += _T("...");
	}

	CSize sizeText = GetOptimalTextSize (strText);
	CSize sizeURL = GetOptimalTextSize (strURL);

	int cx = max (sizeText.cx, sizeURL.cx);

	if (m_Params.m_hIcon != NULL)
	{
		ICONINFO iconInfo;
		::GetIconInfo (m_Params.m_hIcon, &iconInfo);

		BITMAP bitmap;
		::GetObject (iconInfo.hbmColor, sizeof (BITMAP), &bitmap);

		::DeleteObject (iconInfo.hbmColor);
		::DeleteObject (iconInfo.hbmMask);

		cxIcon = bitmap.bmWidth;
		cyIcon = bitmap.bmHeight;

		if (dblScale != 1.)
		{
			cxIcon = (int) (.5 + dblScale * cxIcon);
			cyIcon = (int) (.5 + dblScale * cyIcon);
		}

		CRect rectIcon = CRect (xMargin, yMargin, 
								cxIcon + xMargin, cyIcon + yMargin);

		m_wndIcon.Create (_T(""), WS_CHILD | SS_ICON | SS_NOPREFIX, rectIcon, this);
		m_wndIcon.SetIcon (m_Params.m_hIcon);

		cxIcon += xMargin;
		cyIcon += 2 * yMargin;

		x += cxIcon;
	}

	if (!strText.IsEmpty ())
	{
		CRect rectText (CPoint (x, y), CSize (cx, sizeText.cy));

		m_wndText.Create (strText, WS_CHILD | WS_VISIBLE, rectText, this);
		m_wndText.SetFont (&globalData.fontRegular);

		y = rectText.bottom + yMargin;
	}

	if (!strURL.IsEmpty ())
	{
		CRect rectURL (CPoint (x, y), CSize (cx, sizeURL.cy));

		m_btnURL.Create (strURL, WS_VISIBLE | WS_CHILD, rectURL, this, m_Params.m_nURLCmdID);

		m_btnURL.m_bVisualManagerStyle = TRUE;
		m_btnURL.m_bMultilineText = TRUE;
		m_btnURL.m_bAlwaysUnderlineText = FALSE;
		m_btnURL.m_bDefaultClickProcess = !params.m_bOpenURLOnClick;
		m_btnURL.m_bDrawFocus = FALSE;

		if (m_pParentPopup != NULL)
		{
			m_btnURL.SetCustomTextColors(m_pParentPopup->GetLinkTextColor(this, FALSE), m_pParentPopup->GetLinkTextColor(this, TRUE));
		}

		y = rectURL.bottom + yMargin;
	}

	m_sizeDlg = CSize (cxIcon + cx + 2 * xMargin, max(y, cyIcon));
	return TRUE;
}

CSize CBCGPPopupDlg::GetDlgSize ()
{
	if (!m_bDefault)
	{
		ASSERT (FALSE);
		return CSize (0, 0);
	}

	return m_sizeDlg;
}

void CBCGPPopupDlg::OnSetFocus(CWnd* pOldWnd) 
{
	if (m_bDontSetFocus && pOldWnd->GetSafeHwnd () != NULL)
	{
		pOldWnd->SetFocus ();
		return;
	}

	CBCGPDialog::OnSetFocus(pOldWnd);
}

BOOL CBCGPPopupDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_LBUTTONDOWN && m_pParentPopup->GetSafeHwnd () != NULL)
	{
		CWnd* pWnd = CWnd::FromHandle (pMsg->hwnd);
		if (pWnd != NULL)
		{
			TCHAR lpszClassName [MAX_CLASS_NAME + 1];

			::GetClassName (pWnd->GetSafeHwnd (), lpszClassName, MAX_CLASS_NAME);
			CString strClass = lpszClassName;

			if (strClass == STATIC_CLASS || pWnd->GetSafeHwnd () == GetSafeHwnd ())
			{
				m_pParentPopup->StartWindowMove ();
			}
		}
	}

	return CBCGPDialog::PreTranslateMessage(pMsg);
}

#endif	// BCGP_EXCLUDE_POPUP_WINDOW
