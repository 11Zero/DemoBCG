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
// BCGPDurationCtrl.cpp: implementation of the CBCGPDurationCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "bcgglobals.h"
#include "BCGPDurationCtrl.h"
#include "BCGPDlgImpl.h"
#include "BCGPCalendarBar.h"
#include "BCGPEdit.h"

#ifndef _BCGPCALENDAR_STANDALONE
	#include "BCGPVisualManager.h"
	#define visualManager	CBCGPVisualManager::GetInstance ()
#else
	#include "BCGPCalendarVisualManager.h"
	#define visualManager	CBCGPCalendarVisualManager::GetInstance ()
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


static const int iSpinWidth = 15;
static const int iSpinID = 1;

const UINT CBCGPDurationCtrl::DRTN_DAYS =  1u;
const UINT CBCGPDurationCtrl::DRTN_HOURS_MINS = 2u;
const UINT CBCGPDurationCtrl::DRTN_SECONDS = 4u;
const UINT CBCGPDurationCtrl::DRTN_SPIN =  8u;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPDurationCtrl::CBCGPDurationCtrl()
{
	m_spinButton = TRUE;
	m_lDays = 0;
	m_lHours = 0;
	m_lMinutes = 0;
	m_lSeconds = 0;

	m_strDaysLabel = _T("Day(s) ");
	m_strHoursLabel = _T(":");
	m_strMinutesLabel = _T("");
	m_strSecondsLabel = _T("");

	m_bShowDays = TRUE;
	m_bShowHoursMinutes = TRUE;
	m_bShowSeconds = FALSE;

	m_iSelectedPart = UNDEFINED_PART;

	m_iPrevDigit = -1;
	m_bIsInitialized = FALSE;

	m_bAutoResize = TRUE;

	m_iControlWidth = 0;
	m_iControlHeight = 0;

	m_colorText = (COLORREF)-1;
	m_colorBackground = (COLORREF)-1;

	m_hFont	= NULL;
	m_bVisualManagerStyle = FALSE;
}

CBCGPDurationCtrl::~CBCGPDurationCtrl()
{

}

BEGIN_MESSAGE_MAP(CBCGPDurationCtrl, CButton)
	//{{AFX_MSG_MAP(CBCGPDurationCtrl)
	ON_WM_GETDLGCODE()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONUP()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_ENABLE()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_PRINT, OnPrint)
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLVMMODE, OnBCGSetControlVMMode)
END_MESSAGE_MAP()

LRESULT CBCGPDurationCtrl::OnBCGSetControlVMMode (WPARAM wp, LPARAM)
{
	m_bVisualManagerStyle = (BOOL) wp;
	RedrawWindow();
	
	return 0L;
}
//*****************************************************************************************
UINT CBCGPDurationCtrl::OnGetDlgCode()
{
	return DLGC_WANTARROWS | DLGC_WANTCHARS;
}
//**************************************************************************************
void CBCGPDurationCtrl::SizeToContent (BOOL bRedraw/* = TRUE */)
{
	if (GetSafeHwnd () != NULL)
	{
		CRect rectClient;
		GetClientRect (rectClient);

		AdjustControl (rectClient);

		if (bRedraw)
		{
			RedrawWindow ();
		}
	}
}
//**************************************************************************************
void CBCGPDurationCtrl::AdjustControl (CRect rectClient, BOOL bRedraw/* = TRUE)*/)
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	int nSpinWidthCurr = iSpinWidth;
	
	if (globalData.GetRibbonImageScale() != 1.0)
	{
		nSpinWidthCurr = (int)(.5 + globalData.GetRibbonImageScale() * nSpinWidthCurr);
	}

	CClientDC dc (this);

	CFont* pPrevFont = m_hFont == NULL ?
		(CFont*) dc.SelectStockObject (DEFAULT_GUI_FONT) :
		dc.SelectObject (CFont::FromHandle (m_hFont));
	ASSERT (pPrevFont != NULL);

	if (m_bAutoResize)
	{
		TEXTMETRIC tm;
		dc.GetTextMetrics (&tm);

		m_iControlHeight = tm.tmHeight + 6;
	}
	else
	{
		m_iControlHeight = rectClient.Height ();
	}

	dc.SelectObject (pPrevFont);

	int x = rectClient.left;
	for (int i = 0; i < PARTS_NUM; i ++)
	{
		CSize size (0, 0);
		CString strText;

		switch (i)
		{
		case DAYS:
			if (m_bShowDays)
			{
				strText = _T("000");
			}
			break;

		case DAYS_LABEL:
			if (m_bShowDays)
			{
				strText = m_strDaysLabel;
			}
			break;

		case HOURS:
		case MINUTES:
			if (m_bShowHoursMinutes)
			{
				strText = _T("00");
			}
			break;

		case HOURS_LABEL:
			if (m_bShowHoursMinutes)
			{
				strText = m_strHoursLabel;
			}
			break;

		case MINUTES_LABEL:
			if (m_bShowHoursMinutes)
			{
				strText = m_strMinutesLabel;
			}
			break;

		case SECONDS_LABEL:
			if (m_bShowSeconds)
			{
				strText = m_strSecondsLabel;
			}
			break;

		case SECONDS:
			if (m_bShowSeconds)
			{
				strText = _T("00");
			}
			break;

		default:
			ASSERT (FALSE);
		}

		GetPartSize (strText, size);

		m_rectParts [i] = CRect (x, rectClient.top,
								x + size.cx, rectClient.top + m_iControlHeight);
		m_rectParts [i].DeflateRect (0, 1);

		if (size.cx > 0)
		{
			x += size.cx + 2;
		}
	}

	int nRightPart = m_bShowSeconds ? SECONDS_LABEL : MINUTES_LABEL;

	m_iControlWidth = m_rectParts [nRightPart].right + 2;

	if (m_spinButton)
	{
		m_iControlWidth += nSpinWidthCurr;
	}

	if (!m_bAutoResize)
	{
		m_iControlWidth = rectClient.Width ();
		m_iControlHeight = rectClient.Height ();
	}

	if (m_bAutoResize)
	{
		SetWindowPos (NULL, -1, -1, m_iControlWidth + 2, m_iControlHeight + 2,
			SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	// Adjust spin button:
	if (m_spinButton)
	{
		if (m_wndSpin.GetSafeHwnd () == NULL)
		{
			CRect rectSpin (0, 0, 0, 0);
			m_wndSpin.Create (WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_AUTOBUDDY,
								rectSpin, this, iSpinID);

			m_wndSpin.m_bIsDateTimeControl = TRUE;
		}

		m_wndSpin.SetWindowPos (NULL, 
				rectClient.left + m_iControlWidth - nSpinWidthCurr - 2, 
				rectClient.top,
				nSpinWidthCurr, m_iControlHeight - 2,
				SWP_NOZORDER | SWP_NOACTIVATE);
		m_wndSpin.ShowWindow (SW_SHOW);

		m_wndSpin.EnableWindow (IsWindowEnabled ());
	}
	else
	{
		if (m_wndSpin.GetSafeHwnd () != NULL)
		{
			m_wndSpin.SetWindowPos (NULL, 
					0, 0,
					0, 0,
					SWP_NOZORDER | SWP_NOACTIVATE);
			m_wndSpin.ShowWindow (SW_HIDE);
		}
	}

	if (bRedraw)
	{
		RedrawWindow ();
	}
}
//***************************************************************************************
void CBCGPDurationCtrl::GetPartSize (const CString& strText, CSize& size)
{
	if (strText.IsEmpty ())
	{
		size = CSize (0, 0);
		return;
	}

	CClientDC dc (this);

	CRect rectClient;
	GetClientRect (&rectClient);

	CFont* pPrevFont = m_hFont == NULL ?
		(CFont*) dc.SelectStockObject (DEFAULT_GUI_FONT) :
		dc.SelectObject (CFont::FromHandle (m_hFont));
	ASSERT (pPrevFont != NULL);

	dc.DrawText (strText, rectClient, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);

	dc.SelectObject (pPrevFont);
	size = rectClient.Size ();
}
//**************************************************************************************
void CBCGPDurationCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CButton::OnLButtonDown(nFlags, point);

	SetFocus ();

	int iNewSel = UNDEFINED_PART;

	for (int i = 0; i < PARTS_NUM; i ++)
	{
		if (m_rectParts [i].PtInRect (point))
		{
			iNewSel = i;
			break;
		}
	}

	SelectPart (iNewSel);

	if (m_wndSpin.GetSafeHwnd () != NULL)
	{
		m_wndSpin.RedrawWindow ();
	}
}
//**************************************************************************************
void CBCGPDurationCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (isdigit (nChar))
	{
		PushDigit (nChar - '0');
		return;
	}

	if (nChar >= VK_NUMPAD0 && nChar <= VK_NUMPAD9)
	{
		PushDigit (nChar - VK_NUMPAD0);
		return;
	}

	m_iPrevDigit = -1;

	switch (nChar)
	{
	case VK_DOWN:
		ScrollCurrPart (-1);
		break;

	case VK_UP:
		ScrollCurrPart (1);
		break;

	case VK_RIGHT:
		SelectNext ();
		break;

	case VK_LEFT:
		SelectPrev ();
		break;

	case VK_HOME:
		SelectPart (m_bShowDays ? DAYS : HOURS);
		break;

	case VK_END:
		if (m_bShowSeconds)
		{
			SelectPart (SECONDS);
		}
		else if (m_bShowHoursMinutes)
		{
			SelectPart (MINUTES);
		}
		break;
	}
	
	CButton::OnKeyDown(nChar, nRepCnt, nFlags);
}
//*****************************************************************************************
void CBCGPDurationCtrl::SelectNext ()
{
	int iNewSel = m_iSelectedPart;

	switch (m_iSelectedPart)
	{
	case DAYS:
		if (m_bShowHoursMinutes)
		{
			iNewSel = HOURS;
		}
		break;

	case HOURS:
		iNewSel = MINUTES;
		break;

	case MINUTES:
		iNewSel = m_bShowSeconds ? SECONDS : m_bShowDays ? DAYS : HOURS;
		break;

	case SECONDS:
		iNewSel = m_bShowDays ? DAYS : HOURS;
	}

	SelectPart (iNewSel);
}
//*****************************************************************************************
void CBCGPDurationCtrl::SelectPrev ()
{
	int iNewSel = m_iSelectedPart;

	switch (m_iSelectedPart)
	{
	case DAYS:
		if (m_bShowSeconds)
		{
			iNewSel = SECONDS;
		}
		else if (m_bShowHoursMinutes)
		{
			iNewSel = MINUTES;
		}
		break;

	case HOURS:
		iNewSel = m_bShowDays ? DAYS : m_bShowSeconds ? SECONDS : MINUTES;
		break;

	case MINUTES:
		iNewSel = HOURS;
		break;

	case SECONDS:
		iNewSel = MINUTES;
	}

	SelectPart (iNewSel);
}
//**************************************************************************************
void CBCGPDurationCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CButton::OnSetFocus(pOldWnd);
	
	if (m_iSelectedPart == UNDEFINED_PART)
	{
		SelectPart (m_bShowDays ? DAYS : HOURS);
	}
	else
	{
		if (m_iSelectedPart != UNDEFINED_PART)
		{
			RedrawWindow (m_rectParts [m_iSelectedPart]);
		}
	}
}
//**************************************************************************************
void CBCGPDurationCtrl::SelectPart (int iNewSel)
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	int iOldSel = m_iSelectedPart;

	switch (iNewSel)
	{
	case DAYS:
	case HOURS:
	case MINUTES:
	case SECONDS:
		m_iSelectedPart = iNewSel;
		break;

	case DAYS_LABEL:
		m_iSelectedPart = DAYS;
		break;

	case HOURS_LABEL:
		m_iSelectedPart = HOURS;
		break;

	case MINUTES_LABEL:
		m_iSelectedPart = MINUTES;
		break;
	}

	if (m_iSelectedPart == iOldSel)
	{
		return;
	}

	if (iOldSel != UNDEFINED_PART)
	{
		RedrawWindow (m_rectParts [iOldSel]);
	}

	if (m_iSelectedPart != UNDEFINED_PART)
	{
		RedrawWindow (m_rectParts [m_iSelectedPart]);
	}
}
//***************************************************************************************
void CBCGPDurationCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	CButton::OnKillFocus(pNewWnd);
	
	if (m_iSelectedPart != UNDEFINED_PART)
	{
		RedrawWindow (m_rectParts [m_iSelectedPart]);
	}
}
//*****************************************************************************************
void CBCGPDurationCtrl::PushDigit (int iDigit)
{
	int iNumber;
	if (m_iPrevDigit == -1)
	{
		iNumber = iDigit;
	}
	else
	{
		iNumber = m_iPrevDigit * 10 + iDigit;
	}

	switch (m_iSelectedPart)
	{
	case DAYS:
		if (m_iPrevDigit == -1)
		{
			m_lDays = iDigit;
		}
		else
		{
			m_lDays = m_lDays * 10 + iDigit;
		}
		break;

	case HOURS:
		if (iNumber >= 24)
		{
			return;
		}

		m_lHours = iNumber;
		break;

	case MINUTES:
		if (iNumber >= 60)
		{
			return;
		}

		m_lMinutes = iNumber;
		break;

	case SECONDS:
		if (iNumber >= 60)
		{
			return;
		}

		m_lSeconds = iNumber;
		break;

	default:
		return;
	}

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow (m_rectParts [m_iSelectedPart]);
	}

	if (m_iPrevDigit == -1)	// First push
	{
		m_iPrevDigit = iDigit;
	}
	else
	{
		if (m_iSelectedPart != DAYS || m_lDays >= 100)
		{
			m_iPrevDigit = -1;
		}

		int nLastPart = m_bShowSeconds ? SECONDS : 
			m_bShowHoursMinutes ? MINUTES : DAYS;

		if (m_iSelectedPart != nLastPart &&
			(m_iSelectedPart != DAYS || m_lDays >= 100))
		{
			SelectNext ();
		}
	}

	OnDurationChanged ();
}
//***************************************************************************************
COleDateTimeSpan CBCGPDurationCtrl::GetDuration() const
{
	return COleDateTimeSpan(m_lDays, m_lHours, m_lMinutes, m_lSeconds);
}
//****************************************************************************************
void CBCGPDurationCtrl::SetDuration(const COleDateTimeSpan& timeSpan)
{
	m_lDays = timeSpan.GetDays ();
	m_lHours = timeSpan.GetHours ();
	m_lMinutes = timeSpan.GetMinutes ();
	m_lSeconds = timeSpan.GetSeconds ();

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//***************************************************************************************
void CBCGPDurationCtrl::ScrollCurrPart (int iDelta)
{
	switch (m_iSelectedPart)
	{
	case DAYS:
		if (m_lDays + iDelta < 0 || m_lDays + iDelta >= 1000)
		{
			return;
		}

		m_lDays += iDelta;
		break;

	case HOURS:
		m_lHours += iDelta;
		if (m_lHours < 0)
		{
			m_lHours = 23;
		}
		else if (m_lHours > 23)
		{
			m_lHours = 0;
		}
		break;

	case MINUTES:
		m_lMinutes += iDelta;
		if (m_lMinutes < 0)
		{
			m_lMinutes = 59;
		}
		else if (m_lMinutes > 59)
		{
			m_lMinutes = 0;
		}
		break;

	case SECONDS:
		m_lSeconds += iDelta;
		if (m_lSeconds < 0)
		{
			m_lSeconds = 59;
		}
		else if (m_lSeconds > 59)
		{
			m_lSeconds = 0;
		}
		break;

	default:
		return;
	}

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow (m_rectParts [m_iSelectedPart]);
	}

	OnDurationChanged ();
}
//**************************************************************************************
BOOL CBCGPDurationCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	if (wParam == iSpinID)
	{
		NM_UPDOWN* pNM = (NM_UPDOWN*) lParam;
		ASSERT (pNM != NULL);

		if (pNM->hdr.code == UDN_DELTAPOS)
		{
			ScrollCurrPart (pNM->iDelta < 0 ? 1 : -1);
		}

		SetFocus ();
	}
	
	return CButton::OnNotify(wParam, lParam, pResult);
}
//***************************************************************************************
void CBCGPDurationCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CButton::OnLButtonUp(nFlags, point);

	if (m_wndSpin.GetSafeHwnd () != NULL)
	{
		m_wndSpin.RedrawWindow ();
	}
}
//***************************************************************************************
BOOL CBCGPDurationCtrl::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= BS_OWNERDRAW;
	cs.style &= ~BS_DEFPUSHBUTTON;

	m_bIsInitialized = TRUE;

	return CButton::PreCreateWindow(cs);
}

void CBCGPDurationCtrl::PreSubclassWindow() 
{
	if (!m_bIsInitialized)
	{
		ModifyStyle (BS_DEFPUSHBUTTON, BS_OWNERDRAW | WS_BORDER);
	}
	else
	{
		ModifyStyle (BS_DEFPUSHBUTTON, BS_OWNERDRAW);
	}

	CButton::PreSubclassWindow();

	CRect rectClient;
	GetClientRect (rectClient);
	
	if (rectClient.Width() <= 0 || rectClient.Height() <= 0)
	{
		return;
	}

	AdjustControl(rectClient);
}

void CBCGPDurationCtrl::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	if (GetStyle() & WS_BORDER) 
	{
		lpncsp->rgrc[0].left++; 
		lpncsp->rgrc[0].top++ ;
		lpncsp->rgrc[0].right--;
		lpncsp->rgrc[0].bottom--;
	}

	CButton::OnNcCalcSize(bCalcValidRects, lpncsp);
}

void CBCGPDurationCtrl::OnNcPaint() 
{
	if (GetStyle () & WS_BORDER)
	{
		visualManager->OnDrawControlBorder (this);
	}
}

BOOL CBCGPDurationCtrl::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

void CBCGPDurationCtrl::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	ASSERT (lpDIS != NULL);
	ASSERT (lpDIS->CtlType == ODT_BUTTON);

	CDC* pDC = CDC::FromHandle (lpDIS->hDC);
	ASSERT_VALID (pDC);

	CRect rectClient = lpDIS->rcItem;
	COLORREF clrTextDefault = globalData.clrWindowText;

	if (m_colorBackground != (COLORREF)-1)
	{
		pDC->FillSolidRect (&rectClient, m_colorBackground);
	}
	else if (m_bVisualManagerStyle)
	{
		CBCGPEdit dummy;
		
		pDC->FillRect(rectClient, &CBCGPVisualManager::GetInstance ()->GetEditCtrlBackgroundBrush(&dummy));
		clrTextDefault = CBCGPVisualManager::GetInstance ()->GetEditCtrlTextColor(&dummy);
	}
	else
	{
		pDC->FillRect(rectClient, IsWindowEnabled() ? &globalData.brWindow : &globalData.brBtnFace);
	}

	pDC->SetBkMode (TRANSPARENT);

	CFont* pPrevFont = m_hFont == NULL ?
		(CFont*) pDC->SelectStockObject (DEFAULT_GUI_FONT) :
		pDC->SelectObject (CFont::FromHandle (m_hFont));
	ASSERT_VALID (pPrevFont);

	for (int i = 0; i < PARTS_NUM; i ++)
	{
		if (m_rectParts [i].Width () == 0)
		{
			continue;
		}

		CString strText;

		switch (i)
		{
		case DAYS:
			strText.Format (_T("%d"), m_lDays);
			break;

		case DAYS_LABEL:
			strText = m_strDaysLabel;
			break;

		case HOURS:
			strText.Format (_T("%02d"), m_lHours);
			break;

		case MINUTES:
			strText.Format (_T("%02d"), m_lMinutes);
			break;

		case SECONDS:
			strText.Format (_T("%02d"), m_lSeconds);
			break;

		case HOURS_LABEL:
			strText = m_strHoursLabel;
			break;

		case MINUTES_LABEL:
			strText = m_strMinutesLabel;
			break;

		case SECONDS_LABEL:
			strText = m_strSecondsLabel;
			break;

		default:
			ASSERT (FALSE);
		}

		CRect rect = m_rectParts [i];

		if (IsWindowEnabled () && GetFocus () == this && i == m_iSelectedPart)	// Selected part
		{
			CRect rectFill = rect;
			rectFill.bottom = rectClient.bottom - 1;

			if (m_bVisualManagerStyle)
			{
				CBCGPCalendar dummy;
				CBCGPCalendarColors colors;
				
				CBCGPVisualManager::GetInstance()->GetCalendarColors(&dummy, colors);
				
				pDC->FillSolidRect (rectFill, colors.clrSelected);
				pDC->SetTextColor (colors.clrSelectedText);
			}
			else
			{
				pDC->FillSolidRect (rectFill, globalData.clrHilite);
				pDC->SetTextColor (globalData.clrTextHilite);
			}
		}
		else
		{
			pDC->SetTextColor (IsWindowEnabled () ? 
					(m_colorText == (COLORREF)-1 ? clrTextDefault : m_colorText) : 
					globalData.clrGrayedText);
		}

		pDC->DrawText (strText, rect, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);
	}

	if (::IsWindow (m_wndSpin.GetSafeHwnd ()))
	{
		m_wndSpin.RedrawWindow ();
	}

	pDC->SelectObject (pPrevFont);
}
//*************************************************************************************
void CBCGPDurationCtrl::OnDurationChanged ()
{
	CWnd* pParent = GetParent ();
	if (pParent != NULL)
	{
		pParent->SendMessage (	WM_COMMAND,
								MAKEWPARAM (GetDlgCtrlID (), BN_CLICKED),
								(LPARAM) m_hWnd);
	}
}
//*****************************************************************************************
UINT CBCGPDurationCtrl::GetState () const
{
	UINT flags = 0;

	if (m_bShowDays)
	{
		flags |= DRTN_DAYS;
	}

	if (m_bShowHoursMinutes)
	{
		flags |= DRTN_HOURS_MINS;
	}

	if (m_bShowSeconds)
	{
		flags |= DRTN_SECONDS;
	}

	if (m_spinButton)
	{
		flags |= DRTN_SPIN;
	}

	return flags;
}
//*****************************************************************************************
void CBCGPDurationCtrl::SetState (UINT flags, UINT mask)
{
	if(! ( flags & (~DRTN_SPIN) ) )
	{
		flags |= DRTN_DAYS | DRTN_HOURS_MINS | DRTN_SECONDS;
		mask |= DRTN_DAYS | DRTN_HOURS_MINS | DRTN_SECONDS;
	}

	m_bShowDays = (flags & DRTN_DAYS);
	m_bShowHoursMinutes = (flags & DRTN_HOURS_MINS);
	m_bShowSeconds = (flags & DRTN_SECONDS);
	m_spinButton = (flags & DRTN_SPIN);

	m_strMinutesLabel = m_bShowSeconds ? _T(":") : _T("");

    m_iSelectedPart = UNDEFINED_PART;

	if (GetSafeHwnd () != NULL)
	{
		CRect rectClient;
		GetClientRect (rectClient);

		AdjustControl (rectClient);
		RedrawWindow ();
	}
}

void CBCGPDurationCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CButton::OnSize(nType, cx, cy);
	
	CRect rectClient;
	GetClientRect (rectClient);

	AdjustControl (rectClient);
	RedrawWindow ();
}

BOOL CBCGPDurationCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint ptCursor;
	::GetCursorPos (&ptCursor);
	ScreenToClient (&ptCursor);

	for (int i = 0; i < PARTS_NUM; i ++)
	{
		if ((i == DAYS || i == HOURS || i == MINUTES || i == SECONDS) && // Not label &&
			m_rectParts [i].PtInRect (ptCursor))
		{
			::SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_IBEAM));
			return TRUE;
		}
	}
	
	return CButton::OnSetCursor(pWnd, nHitTest, message);
}
//*******************************************************************************
void CBCGPDurationCtrl::SetTextColor (COLORREF color, BOOL bRedraw)
{
	m_colorText = color;

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//*******************************************************************************
void CBCGPDurationCtrl::SetBackgroundColor (COLORREF color, BOOL bRedraw)
{
	m_colorBackground = color;

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//*****************************************************************************
LRESULT CBCGPDurationCtrl::OnSetFont (WPARAM wParam, LPARAM lParam)
{
	BOOL bRedraw = (BOOL) LOWORD (lParam);

	m_hFont = (HFONT) wParam;

	if (bRedraw)
	{
		Invalidate ();
		UpdateWindow ();
	}

	return 0;
}
//*****************************************************************************
LRESULT CBCGPDurationCtrl::OnGetFont (WPARAM, LPARAM)
{
	return (LRESULT) m_hFont;
}
//*****************************************************************************
void CBCGPDurationCtrl::SetDaysLabel (CString strLabel, BOOL bRedraw/* = TRUE*/)
{
	m_strDaysLabel = strLabel;
	SizeToContent (FALSE);

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//*****************************************************************************
void CBCGPDurationCtrl::SetHoursLabel (CString strLabel, BOOL bRedraw/* = TRUE*/)
{
	m_strHoursLabel = strLabel;
	SizeToContent (FALSE);

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//*****************************************************************************
void CBCGPDurationCtrl::SetMinutesLabel (CString strLabel, BOOL bRedraw/* = TRUE*/)
{
	m_strMinutesLabel = strLabel;
	SizeToContent (FALSE);

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//*****************************************************************************
void CBCGPDurationCtrl::SetSecondsLabel (CString strLabel, BOOL bRedraw/* = TRUE*/)
{
	m_strSecondsLabel = strLabel;
	SizeToContent (FALSE);

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//*******************************************************************************
void CBCGPDurationCtrl::EnableVisualManagerStyle(BOOL bEnable)
{
	m_bVisualManagerStyle = bEnable;

	if (GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}
//*******************************************************************************
void CBCGPDurationCtrl::OnEnable(BOOL bEnable) 
{
	CButton::OnEnable(bEnable);

	if (::IsWindow (m_wndSpin.GetSafeHwnd ()))
	{
		m_wndSpin.EnableWindow (bEnable);
		m_wndSpin.RedrawWindow ();
	}
}
//****************************************************************************
LRESULT CBCGPDurationCtrl::OnPrint(WPARAM wp, LPARAM lp)
{
	DWORD dwFlags = (DWORD)lp;

	if ((dwFlags & PRF_NONCLIENT) == PRF_NONCLIENT && (GetStyle () & WS_BORDER))
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		CRect rect;
		GetWindowRect(rect);
		
		rect.bottom -= rect.top;
		rect.right -= rect.left;
		rect.left = rect.top = 0;
		
		visualManager->OnDrawControlBorder(pDC, rect, this, FALSE);
	}

	return Default();
}
//****************************************************************************
BOOL CBCGPDurationCtrl::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
	return DoScroll(zDelta / WHEEL_DELTA);
}
//*********************************************************************************
BOOL CBCGPDurationCtrl::DoScroll(int nScrollSteps)
{
	if (nScrollSteps == 0)
	{
		return FALSE;
	}

	if (m_iSelectedPart == UNDEFINED_PART)
	{
		SelectPart (m_bShowDays ? DAYS : HOURS);
	}

	for (int i = 0; i < abs(nScrollSteps); i++)
	{
		ScrollCurrPart (nScrollSteps < 0 ? 1 : -1);
	}

	return TRUE;
}
