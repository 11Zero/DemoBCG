// BCGPDateTimeCtrl.cpp : implementation file
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//

#include "stdafx.h"

#include "BCGCBPro.h"
#include "BCGPDateTimeCtrl.h"
#include "BCGPCalendarBar.h"
#include "BCGPDlgImpl.h"
#include "BCGPEdit.h"
#include "trackmouse.h"

#ifndef _BCGPCALENDAR_STANDALONE
	#include "BCGPVisualManager.h"
#ifndef _BCGSUITE_
	#define visualManagerMFC	CBCGPVisualManager::GetInstance ()
#else
	#define visualManagerMFC	CMFCVisualManager::GetInstance ()
#endif
	
	#define visualManager	CBCGPVisualManager::GetInstance ()
#else
	#include "BCGPCalendarVisualManager.h"

	#define visualManager	CBCGPCalendarVisualManager::GetInstance ()
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int iSpinWidth = 15;
static const int iDropButtonWidth = 17;
static const int iSpinID = 1;
static const COLORREF clrTransparent = RGB (255, 0, 255);

static const int iFirstAllowedYear = 100;	// COleDateTime limitation
static const int iLastAllowedYear = 9998;	// COleDateTime limitation

const UINT		CBCGPDateTimeCtrl::DTM_SPIN			= 0x1;
const UINT		CBCGPDateTimeCtrl::DTM_DATE			= 0x2;
const UINT		CBCGPDateTimeCtrl::DTM_TIME			= 0x4;
const UINT		CBCGPDateTimeCtrl::DTM_CHECKBOX		= 0x8;
const UINT		CBCGPDateTimeCtrl::DTM_DROPCALENDAR = 0x10;
const UINT		CBCGPDateTimeCtrl::DTM_TIME24H		= 0x20;
const UINT		CBCGPDateTimeCtrl::DTM_CHECKED		= 0x40;
const UINT		CBCGPDateTimeCtrl::DTM_TIME24HBYLOCALE = 0x80;
const UINT		CBCGPDateTimeCtrl::DTM_SECONDS = 0x100;

static int GetDaysInMonth (int iMonth, int iYear)
{
	static int nMonthLen [] = 
	{	
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 
	};

	int nRes = nMonthLen [iMonth - 1];
	if (iMonth == 2 && iYear % 4 == 0 && 
		(iYear % 100 != 0 || iYear % 400 == 0))
	{
		nRes = 29;
	}

	return nRes;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPDateTimeCtrl

IMPLEMENT_DYNAMIC(CBCGPDateTimeCtrl, CButton)

CBCGPDateTimeCtrl::CBCGPDateTimeCtrl()
{
	m_checkButton = TRUE;
	m_dropCalendar = TRUE;
	m_showDate = TRUE;
	m_showTime = TRUE;
	m_spinButton = TRUE;

	m_type2DigitsInYear = TRUE;
	m_maxYear2Digits = 2090;

	m_bCheckBoxIsAvailable = FALSE;
	m_iPartNum = 0;
	m_iPartsNumber = 0;
	m_bIsChecked = TRUE;
	m_Date = COleDateTime::GetCurrentTime ();
	m_iPrevDigit = -1;

	m_rectText.SetRectEmpty ();

	m_bShowSelection = FALSE;
	m_bDropButtonIsPressed = FALSE;

	m_bMouseOnDropButton = FALSE;
	m_bMouseOnCheckBox = FALSE;

	m_bTrackPart = FALSE;

	m_pPopup = NULL;

	m_bDropButtonIsPressed = FALSE;

	for (int i = 0; i < MAX_PARTS; i ++)
	{
		m_arPartRects [i].SetRectEmpty ();
	}

	m_rectText.SetRectEmpty ();
	m_bAutoResize = TRUE;

	m_iControlWidth = 0;
	m_iControlHeight = 0;

	m_iYearPos = 0;
	m_monthFormat = 0;

	m_MinDate = COleDateTime (iFirstAllowedYear, 1, 1, 0, 0, 0);
	m_MaxDate = COleDateTime (iLastAllowedYear, 12, 31, 23, 59, 59);

	m_weekStart = 1;
	m_b24HoursByLocale = TRUE;
	m_bSeconds = FALSE;

	m_hFont	= NULL;
	m_bIsInitialized = FALSE;

	m_bDrawDateTimeOnly = FALSE;

	m_colorText = (COLORREF)-1;
	m_colorBackground = (COLORREF)-1;

	m_bPropListMode = FALSE;
	m_bVisualManagerStyle = FALSE;

	m_bDontChangeLocale = FALSE;
}

CBCGPDateTimeCtrl::~CBCGPDateTimeCtrl()
{
}

BEGIN_MESSAGE_MAP(CBCGPDateTimeCtrl, CButton)
	//{{AFX_MSG_MAP(CBCGPDateTimeCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	ON_WM_GETDLGCODE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_ERASEBKGND()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_ENABLE()
	ON_WM_CANCELMODE()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_PRINT, OnPrint)
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLVMMODE, OnBCGSetControlVMMode)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPDateTimeCtrl message handlers

LRESULT CBCGPDateTimeCtrl::OnBCGSetControlVMMode (WPARAM wp, LPARAM)
{
	m_bVisualManagerStyle = (BOOL) wp;
	RedrawWindow();
	
	return 0L;
}
//*****************************************************************************************
void CBCGPDateTimeCtrl::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	ASSERT (lpDIS != NULL);
	ASSERT (lpDIS->CtlType == ODT_BUTTON);

	CDC* pDC = CDC::FromHandle (lpDIS->hDC);
	ASSERT_VALID (pDC);

	CBCGPDefaultLocale dl(m_bDontChangeLocale);

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
	ASSERT (pPrevFont != NULL);

	if (!m_bDrawDateTimeOnly)
	{
		// Draw check box:
		DrawCheckBox (pDC, lpDIS->itemState);
	}

	// Draw date/time parts:
	int iStart = m_bCheckBoxIsAvailable ? 1 : 0;

	for (int i = iStart; i < m_iPartsNumber; i ++)
	{
		CString str;
		
		if (m_arPartsOrder [i] == YEAR)
		{
			str.Format(_T("%04d"), m_Date.GetYear());
		}
		else
		{
			str = PartToString(m_Date, i);;
		}

		CRect rect = m_arPartRects [i];

		if (!m_rectDropButton.IsRectEmpty() && rect.right > m_rectDropButton.left)
		{
			rect.right = max(rect.left, m_rectDropButton.left);

			if (rect.right <= rect.left)
			{
				continue;
			}
		}

		if (m_bIsChecked && m_bShowSelection && i == m_iPartNum)	// Selected part
		{
			CRect rectFill = rect;
			pDC->DrawText (str, rectFill, 
				DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_CALCRECT);

			int iOffset = (m_rectText.Height () - rectFill.Height ()) / 2 - 1;

			rectFill.OffsetRect ((rect.Width () - rectFill.Width ()) / 2, iOffset);

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
			pDC->SetTextColor (m_bIsChecked && IsWindowEnabled () ? 
					(m_colorText == (COLORREF)-1 ? clrTextDefault : m_colorText) : 
					globalData.clrGrayedText);
		}

		pDC->DrawText (str, rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

		//-----------------
		// Draw separator:
		//-----------------
		if (i < m_iPartsNumber - 1)
		{
			CString strSeparator = _T(" ");

			if (IsDatePart (i) && IsDatePart (i + 1))
			{
				strSeparator = m_strDateSeparator;
			}

			if (IsTimePart (i) && IsTimePart (i + 1))
			{
				strSeparator = m_strTimeSeparator;
			}
		
			rect.left = m_arPartRects [i].right;
			rect.right = m_arPartRects [i + 1].left;

			pDC->SetTextColor (m_bIsChecked && IsWindowEnabled () ? 
					(m_colorText == (COLORREF)-1 ? clrTextDefault : m_colorText) : 
					globalData.clrGrayedText);
			pDC->DrawText (strSeparator, rect, DT_SINGLELINE | DT_VCENTER);
		}
	}

	if (!m_bDrawDateTimeOnly)
	{
		// Draw "Drop Date combo" button:
		DrawDateDropButton (pDC, lpDIS->itemState);
	}

	pDC->SelectObject (pPrevFont);

	if (::IsWindow (m_wndSpin.GetSafeHwnd ()))
	{
		m_wndSpin.RedrawWindow ();
	}
}
//*****************************************************************************************
void CBCGPDateTimeCtrl::OnShowCalendarPopup () 
{
	if (!m_dropCalendar)
		return;

	if (m_pPopup != NULL)
	{
		ClosePopupCalendar ();
		return;
	}

	m_bDropButtonIsPressed = TRUE;

	CRect rectRedraw = m_rectDropButton;
	rectRedraw.InflateRect (2, 2);

	RedrawWindow (rectRedraw);

	CRect rectWindow;
	GetWindowRect (rectWindow);

	m_pPopup = new CBCGPCalendar ();
	m_pPopup->SetDontChangeLocale(m_bDontChangeLocale);
	m_pPopup->SetDate (m_Date);
	m_pPopup->SetFirstDayOfWeek (m_weekStart - 1);
	m_pPopup->EnableTodayButton ();
	m_pPopup->EnableVisualManagerStyle(m_bVisualManagerStyle);

	m_pPopup->SetMinDate(m_MinDate);
	m_pPopup->SetMaxDate(m_MaxDate);

	CRect rectCalendar (CPoint (rectWindow.left, rectWindow.bottom),
		CSize (0, 0));

	BOOL bIsCreated = m_pPopup->Create (
		WS_POPUP | WS_VISIBLE, rectCalendar, this, 0);

	if (!bIsCreated)
	{
		ASSERT (FALSE);
		m_pPopup = NULL;

		TRACE(_T ("Calendar menu can't be used in the customization mode. You need to set CBCGPDateTimeCtrl::m_bEnabledInCustomizeMode\n"));
	}
}
//************************************************************************************
void CBCGPDateTimeCtrl::OnLButtonDown(UINT /*nFlags*/, CPoint point) 
{
	m_iPrevDigit = -1;

	int iPrevPart = m_iPartNum;

	SetFocus ();

	if (m_rectDropButton.PtInRect (point) && IsWindowEnabled())
	{
		if (m_dropCalendar && m_bIsChecked && m_showDate)
		{
			m_bDropButtonIsPressed = TRUE;

			SetCapture ();
			RedrawWindow (m_rectDropButton);

			OnShowCalendarPopup ();
		}

		return;
	}

	int iPartNum = GetPartFromPoint (point);
	if (iPartNum == -1)
	{
		m_CurrPartType = NO;

		if (m_rectText.PtInRect (point))
		{
			do
			{
				point.x++;

				if (point.x > m_rectText.right)
				{
					return;
				}

				iPartNum = GetPartFromPoint (point);
			}
			while (iPartNum == -1);
		}
		else
		{
			return;
		}
	}

	if (!m_bIsChecked &&
		m_arPartsOrder [iPartNum] != CHECK_BOX)
	{
		return;
	}

	m_iPartNum = iPartNum;
	m_CurrPartType = m_arPartsOrder [m_iPartNum];

	if (m_CurrPartType == CHECK_BOX)
	{
		ToggleCheck ();
	}

	if (iPrevPart != m_iPartNum)
	{
		CRect rectPrevPart = m_arPartRects [iPrevPart];
		rectPrevPart.InflateRect (1, 1);
		RedrawWindow (rectPrevPart);

		CRect rectPart = m_arPartRects [m_iPartNum];
		rectPart.InflateRect (1, 1);
		RedrawWindow (rectPart);
	}
}
//****************************************************************************************
void CBCGPDateTimeCtrl::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/) 
{
	if (m_bDropButtonIsPressed)
	{
		m_bDropButtonIsPressed = FALSE;

		CRect rectRedraw = m_rectDropButton;
		rectRedraw.InflateRect (2, 2);

		RedrawWindow (rectRedraw);

		if (GetCapture () == this)
		{
			ReleaseCapture ();
		}

		if (m_pPopup != NULL)
		{
			ASSERT (m_pPopup != NULL);
			m_pPopup->SetFocus ();
		}
	}
}
//****************************************************************************************
void CBCGPDateTimeCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	CButton::OnMouseMove(nFlags, point);

	if (!IsWindowEnabled())
	{
		return;
	}

	BOOL bMouseOnCheckBox = m_rectCheck.PtInRect(point);
	BOOL bMouseOnDropButton = m_rectDropButton.PtInRect(point);

	BOOL bNeedToTack = FALSE;

	if (bMouseOnCheckBox != m_bMouseOnCheckBox)
	{
		m_bMouseOnCheckBox = bMouseOnCheckBox;

		RedrawWindow(m_rectCheck);
		bNeedToTack = TRUE;
	}

	if (bMouseOnDropButton != m_bMouseOnDropButton)
	{
		m_bMouseOnDropButton = bMouseOnDropButton;

		RedrawWindow(m_rectDropButton);
		bNeedToTack = TRUE;
	}

	if (!m_bTrackPart && bNeedToTack)
	{
		m_bTrackPart = TRUE;
		
		TRACKMOUSEEVENT trackmouseevent;
		trackmouseevent.cbSize = sizeof(trackmouseevent);
		trackmouseevent.dwFlags = TME_LEAVE;
		trackmouseevent.hwndTrack = GetSafeHwnd();
		trackmouseevent.dwHoverTime = HOVER_DEFAULT;
		::BCGPTrackMouse (&trackmouseevent);	
	}
}
//*****************************************************************************************
LRESULT CBCGPDateTimeCtrl::OnMouseLeave(WPARAM,LPARAM)
{
	m_bTrackPart = FALSE;

	BOOL bNeedRedrawCheckBox = m_bMouseOnCheckBox;
	BOOL bNeedRedrawDropButton = m_bMouseOnDropButton;

	m_bMouseOnCheckBox = FALSE;
	m_bMouseOnDropButton = FALSE;

	if (bNeedRedrawCheckBox)
	{
		RedrawWindow(m_rectCheck);
	}

	if (bNeedRedrawDropButton)
	{
		RedrawWindow(m_rectDropButton);
	}

	return 0;
}
//****************************************************************************************
int CBCGPDateTimeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CButton::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectSpin (0, 0, 1, 1);

	if (!m_wndSpin.Create (WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_AUTOBUDDY,
						rectSpin, this, iSpinID))
	{
		TRACE (_T ("CBCGPDateTimeCtrl: Can't create spin button!\n"));
		return -1;
	}

	m_wndSpin.m_bIsDateTimeControl = TRUE;

	CRect rectClient;
	GetClientRect (rectClient);

	AdjustControl (rectClient);
	RedrawWindow ();

	return 0;
}
//****************************************************************************************
UINT CBCGPDateTimeCtrl::OnGetDlgCode() 
{
	return DLGC_WANTARROWS | DLGC_WANTCHARS;
}
//****************************************************************************************
void CBCGPDateTimeCtrl::OnRButtonDown(UINT /*nFlags*/, CPoint /*point*/) 
{
}
//****************************************************************************************
void CBCGPDateTimeCtrl::OnRButtonUp(UINT /*nFlags*/, CPoint /*point*/) 
{
}
//****************************************************************************************
void CBCGPDateTimeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (!IsWindowEnabled ())
	{
		return;
	}

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

	if (isalpha (nChar))
	{
		switch (m_CurrPartType)
		{
		case MONTH:
			if (m_monthFormat != 2)
			{
				ChangeMonth (nChar);
			}
			break;

		case AMPM:
			ChangeAmPm (nChar);
			break;
		}

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
		ScrollCurrPartToLimit (TRUE);
		break;

	case VK_END:
		ScrollCurrPartToLimit (FALSE);
		break;

	case VK_SPACE:
		if (m_CurrPartType == CHECK_BOX)
		{
			ToggleCheck ();
		}
		else if (m_CurrPartType == AMPM)
		{
			ScrollCurrPart (1);
		}
		break;
	}
	
	CButton::OnKeyDown(nChar, nRepCnt, nFlags);
}
//****************************************************************************************
DATE CBCGPDateTimeCtrl::GetDate() 
{
	if (m_bIsChecked)
	{
		COleDateTime today = COleDateTime::GetCurrentTime ();

		COleDateTime date (	m_showDate ? m_Date.GetYear () : today.GetYear (), 
							m_showDate ? m_Date.GetMonth () : today.GetMonth (), 
							m_showDate ? m_Date.GetDay () :  today.GetDay (),
							m_showTime ? m_Date.GetHour () : 0,
							m_showTime ? m_Date.GetMinute () : 0,
							(m_showTime && m_bSeconds) ? m_Date.GetSecond() : 0);
		return (DATE) date;
	}
	else
	{
		COleDateTime dateEmpty;
		return (DATE) dateEmpty;
	}
}
//****************************************************************************************
void CBCGPDateTimeCtrl::SetDate (DATE newValue) 
{
	m_Date = COleDateTime (newValue);

	if (m_bCheckBoxIsAvailable)
	{
		COleDateTime dateEmpty;
		m_bIsChecked = (m_Date != dateEmpty);

		if (m_spinButton)
		{
			m_wndSpin.EnableWindow (m_bIsChecked);
		}
	}

	if (GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//****************************************************************************************
void CBCGPDateTimeCtrl::SetMinDate(DATE newValue) 
{
	COleDateTime dateMin = newValue;
	COleDateTime dateEmpty;

	if (m_MaxDate > dateMin || m_MaxDate == dateEmpty)
	{
		m_MinDate = dateMin;

		if (m_Date < m_MinDate)
		{
			m_Date = m_MinDate;
			OnDateChanged ();
		}
	
		if (GetSafeHwnd()!=NULL)
		{
			RedrawWindow ();
		}
	}
}
//****************************************************************************************
DATE CBCGPDateTimeCtrl::GetMaxDate() 
{
	return (DATE) m_MaxDate;
}
//****************************************************************************************
DATE CBCGPDateTimeCtrl::GetMinDate() 
{
	return (DATE) m_MinDate;
}
//****************************************************************************************
void CBCGPDateTimeCtrl::SetMaxDate(DATE newValue) 
{
	COleDateTime dateMax = newValue;
	COleDateTime dateEmpty;

	if (m_MinDate < dateMax)
	{
		m_MaxDate = dateMax;

		if (m_MaxDate.GetHour () == 0 && m_MaxDate.GetMinute () == 0)
		{
			m_MaxDate -= COleDateTimeSpan (0, 0, 1, 0);	// 1 minute before
		}

		if (m_Date > m_MaxDate && m_MaxDate != dateEmpty)
		{
			m_Date = m_MaxDate;
			OnDateChanged ();
		}

		if (GetSafeHwnd() != NULL)
		{
			RedrawWindow ();
		}
	}
}
//************************************************************************************************
void CBCGPDateTimeCtrl::AdjustControl (CRect rectClient)
{
	if (GetSafeHwnd () == NULL)
	{
		return;
	}

	int nSpinWidthCurr = iSpinWidth;
	int nDropDownWidthCurr = iDropButtonWidth;

	if (globalData.GetRibbonImageScale() != 1.0)
	{
		nSpinWidthCurr = (int)(.5 + globalData.GetRibbonImageScale() * nSpinWidthCurr);
		nDropDownWidthCurr = (int)(.5 + globalData.GetRibbonImageScale() * nDropDownWidthCurr);
	}

	CClientDC dc (this);

	CFont* pPrevFont = m_hFont == NULL ?
		(CFont*) dc.SelectStockObject (DEFAULT_GUI_FONT) :
		dc.SelectObject (CFont::FromHandle (m_hFont));

	if (m_WidestDate == COleDateTime ())
	{
		BuidWidestDate (&dc);
	}

	SetPartsOrder ();
	
	m_iPartNum = m_bCheckBoxIsAvailable ? 1 : 0;
	m_CurrPartType = m_arPartsOrder [m_iPartNum];

	m_rectText = rectClient;
	m_rectText.InflateRect (0, -2);

	if (m_bAutoResize)
	{
		// Calculate text size:
		TEXTMETRIC tm;

		dc.GetTextMetrics (&tm);

		m_iControlHeight = tm.tmHeight + 6;
	}
	else
	{
		m_iControlHeight = rectClient.Height ();
	}

	if (m_bCheckBoxIsAvailable)
	{
		m_rectCheck = CRect (rectClient.left + 1, rectClient.top + 1, 
			rectClient.left + m_iControlHeight - 1, rectClient.bottom - 1);
		m_rectText.left = m_rectCheck.right + 2;
	}
	
	m_rectText.top = rectClient.top;
	m_rectText.bottom = rectClient.top + m_iControlHeight;

	// Calculate parts:
	CalcPartRects (&dc);

	// Adjust control size:
	m_rectText.right = m_arPartRects [m_iPartsNumber - 1].right + 2;
	m_iControlWidth = m_rectText.right;

	if (m_spinButton)
	{
		m_iControlWidth += nSpinWidthCurr;
	}

	if (!m_bAutoResize)
	{
		m_iControlWidth = rectClient.Width ();
		m_iControlHeight = rectClient.Height ();
	}

	if (m_dropCalendar && m_showDate)
	{
		if (m_bAutoResize)
		{
			m_iControlWidth += nDropDownWidthCurr;
		}

		int iDropStart = rectClient.left + m_iControlWidth - nDropDownWidthCurr - 1;
		if (m_spinButton)
		{
			iDropStart -= nSpinWidthCurr + 1;
		}

		m_rectDropButton = CRect (	iDropStart,
									rectClient.top,
									iDropStart + nDropDownWidthCurr,
									rectClient.top + m_iControlHeight - 2);
	}
	else
	{
		m_rectDropButton.SetRectEmpty();
	}

	if (m_bAutoResize)
	{
		SetWindowPos (NULL, -1, -1, m_iControlWidth + 2, m_iControlHeight + 2,
			SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
	}

	// Adjust spin button:
	if (m_spinButton)
	{
		if (m_wndSpin.GetSafeHwnd () == NULL)
		{
			m_wndSpin.Create (WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_AUTOBUDDY,
						CRect (0, 0, 0, 0), this, iSpinID);
			m_wndSpin.m_bIsDateTimeControl = TRUE;
		}

		m_wndSpin.SetWindowPos (NULL, 
				rectClient.left + m_iControlWidth - nSpinWidthCurr - 2, rectClient.top,
				nSpinWidthCurr, m_iControlHeight - 2,
				SWP_NOZORDER);
		m_wndSpin.ShowWindow (SW_SHOW);

		m_wndSpin.EnableWindow (IsWindowEnabled () && m_bIsChecked);
	}
	else if (m_wndSpin.GetSafeHwnd () != NULL)
	{
		m_wndSpin.SetWindowPos (NULL, 
				0, 0,
				0, 0,
				SWP_NOZORDER);
		m_wndSpin.ShowWindow (SW_HIDE);
	}

	if (pPrevFont != NULL)
	{
		dc.SelectObject (pPrevFont);
	}

	//-----------------------------
	// Adjust min./max date values:
	//-----------------------------
	COleDateTime minAllowedDate (iFirstAllowedYear, 1, 1, 0, 0, 0);
	COleDateTime maxAllowedDate (iLastAllowedYear, 12, 31, 23, 59, 59);
	COleDateTime emptyDate;

	m_MinDate = max (m_MinDate, minAllowedDate);

	if (m_MaxDate != emptyDate)
	{
		m_MaxDate = min (m_MaxDate, maxAllowedDate);
	}

	else
	{
		m_MaxDate = maxAllowedDate;
	}
}
//*****************************************************************************************
void CBCGPDateTimeCtrl::DrawCheckBox (CDC* pDC, UINT uiState)
{
	if (!m_bCheckBoxIsAvailable)
	{
		return;
	}

	ASSERT_VALID (pDC);

	visualManagerMFC->OnDrawCheckBox (pDC, m_rectCheck, 
		(m_bShowSelection && m_CurrPartType == CHECK_BOX) || m_bMouseOnCheckBox,
		m_bIsChecked, !(uiState & ODS_DISABLED));
}
//*****************************************************************************************
void CBCGPDateTimeCtrl::DrawDateDropButton (CDC* pDC, UINT uiState)
{
	if (!m_dropCalendar || !m_showDate)
	{
		return;
	}

	BOOL bDisabled = (uiState & ODS_DISABLED) || !m_bIsChecked;

#ifndef _BCGSUITE_
	visualManagerMFC->OnDrawDateTimeDropButton (pDC, m_rectDropButton,
										bDisabled,
										m_bDropButtonIsPressed && !bDisabled,
										m_bMouseOnDropButton && !bDisabled,
										this);
#else
	CBCGPToolbarComboBoxButton buttonDummy;
	visualManagerMFC->OnDrawComboDropButton (pDC, m_rectDropButton,
										bDisabled,
										m_bDropButtonIsPressed && !bDisabled,
										m_bMouseOnDropButton && !bDisabled,
										&buttonDummy);
#endif
}
//*****************************************************************************************
void CBCGPDateTimeCtrl::SelectNext ()
{
	if (m_iPartNum == -1 || !m_bIsChecked)
	{
		return;
	}

	int iPrevPart = m_iPartNum;

	if (m_iPartNum == m_iPartsNumber - 1)	// Last
	{
		m_iPartNum = 0;
	}
	else
	{
		m_iPartNum ++;
	}

	m_CurrPartType = m_arPartsOrder [m_iPartNum];

	CRect rectPrevPart = m_arPartRects [iPrevPart];
	rectPrevPart.InflateRect (1, 1);
	RedrawWindow (rectPrevPart);

	CRect rectPart = m_arPartRects [m_iPartNum];
	rectPart.InflateRect (1, 1);
	RedrawWindow (rectPart);
}
//*****************************************************************************************
void CBCGPDateTimeCtrl::SelectPrev ()
{
	if (m_iPartNum == -1 || !m_bIsChecked)
	{
		return;
	}

	int iPrevPart = m_iPartNum;

	if (m_iPartNum == 0)	// First
	{
		m_iPartNum = m_iPartsNumber - 1;
	}
	else
	{
		m_iPartNum --;
	}

	m_CurrPartType = m_arPartsOrder [m_iPartNum];

	CRect rectPrevPart = m_arPartRects [iPrevPart];
	rectPrevPart.InflateRect (1, 1);
	RedrawWindow (rectPrevPart);

	CRect rectPart = m_arPartRects [m_iPartNum];
	rectPart.InflateRect (1, 1);
	RedrawWindow (rectPart);
}
//*****************************************************************************************
void CBCGPDateTimeCtrl::SetPartsOrder ()
{
	int i = 0;

	if (m_checkButton)
	{
		m_arPartsOrder [i ++] = CHECK_BOX;
		m_bCheckBoxIsAvailable = TRUE;
	}
	else
	{
		m_bCheckBoxIsAvailable = FALSE;
	}

	TCHAR szLocaleData [100];
	
	GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SDATE, szLocaleData, 100);
	m_strDateSeparator = szLocaleData [0] == 0 ? _T ("") : szLocaleData;

	GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_STIME, szLocaleData, 100);
	m_strTimeSeparator = szLocaleData [0] == 0 ? _T ("") : szLocaleData;

	if (m_showDate)
	{
		GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_IDATE, szLocaleData, 100);
		switch (szLocaleData [0])
		{
		case '0':	// Month-Day-Year
		default:
			m_arPartsOrder [i ++] = MONTH;
			m_arPartsOrder [i ++] = DAY;
			m_arPartsOrder [i ++] = YEAR;
			break;

		case '1':	// Day-Month-Year
			m_arPartsOrder [i ++] = DAY;
			m_arPartsOrder [i ++] = MONTH;
			m_arPartsOrder [i ++] = YEAR;
			break;

		case '2':	// Year-Month-Day
			m_arPartsOrder [i ++] = YEAR;
			m_arPartsOrder [i ++] = MONTH;
			m_arPartsOrder [i ++] = DAY;
			break;
		}
	}

	if (m_showTime)
	{
		m_arPartsOrder [i ++] = HOUR;
		m_arPartsOrder [i ++] = MIN;

		if (m_bSeconds)
		{
			m_arPartsOrder [i ++] = SECONDS;
		}

		if (m_b24HoursByLocale)
		{
			GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_ITIME, szLocaleData, 100);
			switch (szLocaleData [0])
			{
			case '0':	// AM/PM
			default:
				m_b24HoursFormat = FALSE;
				break;
			case '1':
				m_b24HoursFormat = TRUE;
				break;
			}
		}

		if (!m_b24HoursFormat)
		{
			m_arPartsOrder [i ++] = AMPM;
		}
	}

	m_iPartsNumber = i;

	GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_S1159, szLocaleData, 100);
	m_strAM = szLocaleData;
	m_strAM.MakeUpper ();

	if (m_strAM.IsEmpty())
	{
		m_strAM = _T("AM");
	}

	GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_S2359, szLocaleData, 100);
	m_strPM = szLocaleData;
	m_strPM.MakeUpper ();

	if (m_strPM.IsEmpty())
	{
		m_strPM = _T("PM");
	}
}
//*****************************************************************************************
LPCTSTR CBCGPDateTimeCtrl::GetPartFormat (int iPart) const
{
	switch (m_arPartsOrder [iPart])
	{
	case NO:
	case CHECK_BOX:
		return _T ("");
	case DAY:
		return _T ("%d");
	case MONTH:
		switch (m_monthFormat)
		{
		case 0:	// Short:
			return _T ("%b");
		case 1:	// Long:
			return _T ("%B");
		case 2:	// Numeric:
			return _T ("%m");
		default:
			ASSERT (FALSE);
			return _T ("%b");
		}

	case YEAR:
		return _T ("%Y");

	case HOUR:
		return m_b24HoursFormat ? _T ("%H") : _T ("%I");

	case MIN:
		return _T ("%M");

	case AMPM:
		return _T ("%p");

	case SECONDS:
		return _T("%S");
	}

	return _T ("");
}
//*****************************************************************************************
CString CBCGPDateTimeCtrl::PartToString(const COleDateTime& date, int iPart) const
{
	CString str = date.Format(GetPartFormat(iPart));

	if (m_arPartsOrder[iPart] == AMPM && str.IsEmpty())
	{
		str = date.GetHour() >= 12 ? m_strPM : m_strAM;
	}

	return str;
}
//*****************************************************************************************
int CBCGPDateTimeCtrl::GetPartFromPoint (POINT point)
{
	for (int i = 0; i < m_iPartsNumber; i ++)
	{
		if (m_arPartRects [i].PtInRect (point))
		{
			return i;
		}
	}

	return -1;
}
//*****************************************************************************************
void CBCGPDateTimeCtrl::CalcPartRects (CDC* pDC)
{
	ASSERT_VALID (pDC);

	CBCGPDefaultLocale dl(m_bDontChangeLocale);

	int iStart = 0;
	if (m_arPartsOrder [0] == CHECK_BOX)
	{
		m_arPartRects [0] = m_rectCheck;
		iStart = 1;
	}

	int x = m_rectText.left;
	int i = 0;

	for (i = iStart; i < m_iPartsNumber; i ++)
	{
		CString strPart;
		if (m_arPartsOrder [i] == YEAR)
		{
			strPart.Format(_T("%04d"), m_Date.GetYear());
		}
		else
		{
			strPart = PartToString(m_WidestDate, i);
		}

		m_arPartRects [i] = m_rectText;
		m_arPartRects [i].left = x;
		m_arPartRects [i].right = x + pDC->GetTextExtent (strPart).cx;

		//-----------------------
		// Calc. separator width:
		//-----------------------
		CString strSeparator = _T(" ");
		if (i < m_iPartsNumber - 1)
		{
			if (IsDatePart (i) && IsDatePart (i + 1))
			{
				strSeparator = m_strDateSeparator;
			}

			if (IsTimePart (i) && IsTimePart (i + 1))
			{
				strSeparator = m_strTimeSeparator;
			}
		}

		x = m_arPartRects [i].right + pDC->GetTextExtent (strSeparator).cx;
	}

	// Clean rest:
	while (i < MAX_PARTS)
	{
		m_arPartRects [i ++].SetRectEmpty ();
	}
}
//*****************************************************************************************
void CBCGPDateTimeCtrl::ScrollCurrPart (int iDir)
{
	int iDay = m_Date.GetDay ();
	int iMonth = m_Date.GetMonth ();
	int iYear = m_Date.GetYear ();
	int iHour = m_Date.GetHour ();
	int iMin = m_Date.GetMinute ();
	int iSec = m_Date.GetSecond ();
	BOOL bLastDayInMonth = (iDay == GetDaysInMonth (iMonth, iYear));

	switch (m_CurrPartType)
	{
	case NO:
	case CHECK_BOX:
		return;

	case DAY:
		iDay += iDir;
		if (iDay <= 0)
		{
			iDay = GetDaysInMonth (iMonth, iYear);
		}

		if (iDay > GetDaysInMonth (iMonth, iYear))
		{
			iDay = 1;
		}
		break;

	case MONTH:
		iMonth += iDir;
		if (iMonth <= 0)
		{
			iMonth = 12;
		}

		if (iMonth > 12)
		{
			iMonth = 1;
		}
		
		if (bLastDayInMonth ||
			iDay > GetDaysInMonth (iMonth, iYear))
		{
			iDay = GetDaysInMonth (iMonth, iYear);
		}

		break;

	case YEAR:
		iYear += iDir;
		iYear = min(iLastAllowedYear, max(iFirstAllowedYear, iYear));

		if (iDay > GetDaysInMonth (iMonth, iYear))
		{
			iDay = GetDaysInMonth (iMonth, iYear);
		}

		break;

	case HOUR:
		iHour += iDir;
		if (iHour < 0)
		{
			iHour = 23;
		}

		if (iHour > 23)
		{
			iHour = 0;
		}
		break;

	case MIN:
		iMin += iDir;
		if (iMin < 0)
		{
			iMin = 59;
		}

		if (iMin > 59)
		{
			iMin = 0;
		}
		break;

	case SECONDS:
		iSec += iDir;
		if (iSec < 0)
		{
			iSec = 59;
		}
		
		if (iSec > 59)
		{
			iSec = 0;
		}
		break;

	case AMPM:
		if (iHour < 12)
		{
			iHour += 12;
		}
		else
		{
			iHour -= 12;
		}
	}

	COleDateTime date (iYear, iMonth, iDay, iHour, iMin, iSec);
	COleDateTime dateEmpty;

	if (iDir > 0 && date > m_MaxDate && m_MaxDate != dateEmpty)
	{
		date = m_MaxDate;
	}

	if (iDir < 0 && date < m_MinDate)
	{
		date = m_MinDate;
	}

	if (m_Date != date && IsDateValid (date))
	{
		m_Date = date;
		RedrawWindow (m_rectText);

		OnDateChanged ();
	}
}
//*****************************************************************************************
void CBCGPDateTimeCtrl::ScrollCurrPartToLimit (BOOL bTop)
{
	int iDay = m_Date.GetDay ();
	int iMonth = m_Date.GetMonth ();
	int iYear = m_Date.GetYear ();
	int iHour = m_Date.GetHour ();
	int iMin = m_Date.GetMinute ();
	int iSec = m_Date.GetSecond ();

	switch (m_CurrPartType)
	{
	case NO:
	case CHECK_BOX:
	case AMPM:
	case YEAR:
		return;

	case DAY:
		if (bTop)
		{
			iDay = 1;
		}
		else
		{
			iDay = GetDaysInMonth (iMonth, iYear);
		}
		break;

	case MONTH:
		if (bTop)
		{
			iMonth = 1;
		}
		else
		{
			iMonth = 12;
		}
		break;

	case HOUR:
		if (bTop)
		{
			iHour = 0;
		}
		else
		{
			iHour = 23;
		}
		break;

	case MIN:
		if (bTop)
		{
			iMin = 0;
		}
		else
		{
			iMin = 59;
		}
		break;

	case SECONDS:
		if (bTop)
		{
			iSec = 0;
		}
		else
		{
			iSec = 59;
		}
		break;
	}

	COleDateTime date (iYear, iMonth, iDay, iHour, iMin, iSec);
	if (IsDateValid (date) && m_Date != date)
	{
		m_Date = date;
		RedrawWindow (m_rectText);

		OnDateChanged ();
	}
}
//*****************************************************************************************
void CBCGPDateTimeCtrl::PushDigit (int iDigit)
{
	int iDay = m_Date.GetDay ();
	int iMonth = m_Date.GetMonth ();
	int iYear = m_Date.GetYear ();
	int iHour = m_Date.GetHour ();
	int iMin = m_Date.GetMinute ();
	int iSec = m_Date.GetSecond ();

	const int iYearDigits = m_type2DigitsInYear ? 2 : 4;

	int iNumber;
	if (m_iPrevDigit == -1)
	{
		iNumber = iDigit;
		m_iYearPos = 0;
	}
	else
	{
		iNumber = m_iPrevDigit * 10 + iDigit;
	}

	switch (m_CurrPartType)
	{
	case NO:
	case CHECK_BOX:
	case AMPM:
		return;

	case DAY:
		iDay = iNumber;
		break;

	case HOUR:
		if (!m_b24HoursFormat)
		{
			BOOL bPM = (iHour >= 12);
			iHour = iNumber;

			if (iHour > 12)
			{
				iHour = iDigit;
			}

			if (bPM)
			{
				iHour += 12;
			}

			if (iHour == 24)
			{
				iHour = 0;
			}

			if (iDigit != 1)	// Only 10, 11 or 12 are allowed!
			{
				iDigit = -1;
			}
		}
		else	// Normal format
		{
			iHour = iNumber;
		}
		break;

	case MIN:
		iMin = iNumber;
		break;

	case SECONDS:
		iSec = iNumber;
		break;

	case MONTH:
		if (m_monthFormat == 2 &&
			iNumber >= 1 && iNumber <= 12)
		{
			iMonth = iNumber;

			if (iDigit > 1 && m_iPrevDigit == -1)
			{
				m_iPrevDigit = 0;	// Stimulate junp to the next part
			}
		}
		break;

	case YEAR:
		if (m_type2DigitsInYear)
		{
			iYear = iYear / 100 * 100 + iNumber;

			if (iYear < m_maxYear2Digits - 99)
			{
				iYear = m_maxYear2Digits / 100 * 100 + iNumber;
			}

			if (iYear > m_maxYear2Digits)
			{
				iYear = (m_maxYear2Digits - 100) / 100 * 100 + iNumber;
			}
		}
		else
		{
			ASSERT (m_iYearPos >= 0);
			ASSERT (m_iYearPos < iYearDigits);

			//------------------------------------------------
			// Replace digit in position m_iYearPos to iDigit:
			//------------------------------------------------
			int iYearNew = 0;

			for (int iPos = 0; iPos < iYearDigits; iPos ++)
			{
				int iTens = 1;
				for (int i = 0; i < iYearDigits - iPos - 1; i ++)
				{
					iTens *= 10;
				}

				iYearNew *= 10;
				if (iPos == m_iYearPos)
				{
					iYearNew += iDigit;
				}
				else
				{
					iYearNew += iYear / iTens;
				}

				iYear %= iTens;
			}

			iYear = iYearNew;
		}

		m_iYearPos ++;
		break;
	}

	COleDateTime date (iYear, iMonth, iDay, iHour, iMin, iSec);
	BOOL bValidDate = IsDateValid (date);
	
	if (bValidDate)
	{
		m_Date = date;
		RedrawWindow (m_rectText);

		OnDateChanged ();
	}

	if (m_iPrevDigit == -1)	// First push
	{
		m_iPrevDigit = iDigit;
	}
	else
	{
		if (bValidDate && m_iPartNum < m_iPartsNumber - 1)
		{
			if (m_CurrPartType != YEAR || m_iYearPos == iYearDigits)
			{
				m_iPrevDigit = -1;
				SelectNext ();
			}
		}
	}
}
//*****************************************************************************************
void CBCGPDateTimeCtrl::ChangeMonth (UINT uiMonthLetter)
{
	ASSERT (m_monthFormat == 0 || m_monthFormat == 1);

	CBCGPDefaultLocale dl(m_bDontChangeLocale);

	int iDay = m_Date.GetDay ();
	int iMonth = m_Date.GetMonth ();
	int iYear = m_Date.GetYear ();
	int iHour = m_Date.GetHour ();
	int iMin = m_Date.GetMinute ();
	int iSec = m_Date.GetSecond();

	BOOL bLastDayInMonth = (iDay == GetDaysInMonth (iMonth, iYear));

	BOOL bFound = FALSE;
	for (int i = iMonth + 1; i != iMonth; i ++)
	{
		if (i > 12)
		{
			i = 1;
		}

		if (i == iMonth)
		{
			break;
		}

		//--------------------------------------------------
		// Compare manth 'i' first char with the typed char:
		//--------------------------------------------------
		CString strMonth = COleDateTime (iYear, i, 1, 0, 0, 0).
			Format (m_monthFormat == 0 ? _T ("%b") : _T ("%B"));

		if (strMonth.GetLength () > 1 &&
			strMonth.GetAt (0) == (char) uiMonthLetter)
		{
			iMonth = i;
			bFound = TRUE;
			break;
		}
	}

	if (bFound)
	{
		if (bLastDayInMonth ||
			iDay > GetDaysInMonth (iMonth, iYear))
		{
			iDay = GetDaysInMonth (iMonth, iYear);
		}

		COleDateTime date (iYear, iMonth, iDay, iHour, iMin, iSec);
		if (IsDateValid (date))
		{
			m_Date = date;
			RedrawWindow (m_rectText);

			OnDateChanged ();
		}
	}
}
//*****************************************************************************************
void CBCGPDateTimeCtrl::ChangeAmPm (UINT uiAmPm)
{
	int iDay = m_Date.GetDay ();
	int iMonth = m_Date.GetMonth ();
	int iYear = m_Date.GetYear ();
	int iHour = m_Date.GetHour ();
	int iMin = m_Date.GetMinute ();
	int iSec = m_Date.GetSecond ();

	CString str;
	str += (char) uiAmPm;
	str.MakeUpper ();

	if (str == m_strPM [0] && iHour < 12)
	{
		iHour += 12;
	}

	if (str == m_strAM [0] && iHour >= 12)
	{
		iHour -= 12;
	}

	COleDateTime date (iYear, iMonth, iDay, iHour, iMin, iSec);
	if (IsDateValid (date) && m_Date != date)
	{
		m_Date = date;
		RedrawWindow (m_rectText);

		OnDateChanged ();
	}
}
//*****************************************************************************************
void CBCGPDateTimeCtrl::ToggleCheck ()
{
	m_bIsChecked = !m_bIsChecked;

	if (m_spinButton)
	{
		m_wndSpin.EnableWindow (m_bIsChecked);
	}

	COleDateTime emptyDate;
	if (m_bIsChecked && m_Date == emptyDate)
	{
		m_Date = COleDateTime::GetCurrentTime ();
		OnDateChanged ();
	}

	RedrawWindow ();
}
//*****************************************************************************************
BOOL CBCGPDateTimeCtrl::IsDateValid (COleDateTime& date) const
{
	if (date.GetStatus () == COleDateTime::invalid)
	{
		return FALSE;
	}

	COleDateTime dateEmpty;

	if (m_MinDate != dateEmpty && date < m_MinDate)
	{
		return FALSE;
	}

	if (m_MaxDate != dateEmpty && date > m_MaxDate)
	{
		return FALSE;
	}

	return TRUE;
}
//*****************************************************************************************
BOOL CBCGPDateTimeCtrl::IsDatePart (int iPart) const
{
	PART_TYPE type = m_arPartsOrder [iPart];
	return type == DAY || type == MONTH || type == YEAR;
}
//*****************************************************************************************
BOOL CBCGPDateTimeCtrl::IsTimePart (int iPart) const
{
	PART_TYPE type = m_arPartsOrder [iPart];
	return type == HOUR || type == MIN || type == SECONDS;
}
//*****************************************************************************************
void CBCGPDateTimeCtrl::OnHideCalendarPopup ()
{
	m_bDropButtonIsPressed = FALSE;

	CRect rectRedraw = m_rectDropButton;
	rectRedraw.InflateRect (2, 2);

	RedrawWindow (rectRedraw);
}
//****************************************************************************************
void CBCGPDateTimeCtrl::OnSetFocus(CWnd* pOldWnd) 
{
	CButton::OnSetFocus(pOldWnd);

	if (m_iPartNum >= 0 && m_iPartNum < m_iPartsNumber)
	{
		m_CurrPartType = m_arPartsOrder [m_iPartNum];
	}

	m_bShowSelection = TRUE;
	RedrawWindow ();
}
//****************************************************************************************
BOOL CBCGPDateTimeCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
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
//****************************************************************************************
void CBCGPDateTimeCtrl::OnKillFocus(CWnd* pNewWnd) 
{
	m_bShowSelection = FALSE;
	m_iPrevDigit = -1;
	RedrawWindow ();

	CButton::OnKillFocus(pNewWnd);
}
//****************************************************************************************
void CBCGPDateTimeCtrl::BuidWidestDate (CDC* pDC)
{
	ASSERT_VALID (pDC);

	CBCGPDefaultLocale dl(m_bDontChangeLocale);

	//-----------------------
	// Find the widest month:
	//-----------------------
	int iMaxMonth = 1;

	if (m_monthFormat == 2)	// Numeric
	{
		iMaxMonth = 9;
	}
	else
	{
		int iMaxMonthWidth = 0;
		for (int iMonth = 1; iMonth <= 12; iMonth ++)
		{
			COleDateTime date (1998, iMonth, 1, 0, 0, 0);
			CString strMonth = date.Format (m_monthFormat == 0 ? _T ("%b") : _T("%B"));

			int iMonthWidth = pDC->GetTextExtent (strMonth).cx;
			if (iMonthWidth > iMaxMonthWidth)
			{
				iMaxMonthWidth = iMonthWidth;
				iMaxMonth = iMonth;
			}
		}
	}

	m_WidestDate = COleDateTime (2000, iMaxMonth, 20, 0, 0, 0);
}
//****************************************************************************************
void CBCGPDateTimeCtrl::SizeToContent ()
{
	if (GetSafeHwnd () != NULL)
	{
		m_WidestDate = COleDateTime();

		CRect rectClient;
		GetClientRect (rectClient);

		AdjustControl (rectClient);
	}
}
//*************************************************************************************
void CBCGPDateTimeCtrl::SetAutoResize(BOOL bSet)
{
	m_bAutoResize = bSet;

	if (GetSafeHwnd () != NULL)
	{
		SizeToContent ();
		RedrawWindow ();
	}
}
//*************************************************************************************
void CBCGPDateTimeCtrl::OnDateChanged ()
{
	CWnd* pParent = GetParent ();
	if (pParent != NULL)
	{
		pParent->SendMessage (	WM_COMMAND,
								MAKEWPARAM (GetDlgCtrlID (), BN_CLICKED),
								(LPARAM) m_hWnd);
	}
}
//*************************************************************************************
void CBCGPDateTimeCtrl::SetFirstDayOfWeek(int nDay)	// 0 - 6
{
	m_weekStart = nDay + 1;
}
//*************************************************************************************
UINT CBCGPDateTimeCtrl::GetState () const
{
	UINT stateFlags = 0;

	if (m_spinButton)
		stateFlags |= DTM_SPIN;
	if (m_dropCalendar)
		stateFlags |= DTM_DROPCALENDAR;
	if (m_showDate)
		stateFlags |= DTM_DATE;
	if (m_b24HoursFormat)
		stateFlags |= DTM_TIME24H;
	if (m_checkButton)
		stateFlags |= DTM_CHECKBOX;
	if (m_showTime)
		stateFlags |= DTM_TIME;
	if (m_bIsChecked)
		stateFlags |= DTM_CHECKED;
	if (m_b24HoursByLocale)
		stateFlags |= DTM_TIME24HBYLOCALE;
	if (m_bSeconds)
			stateFlags |= DTM_SECONDS;

	return stateFlags;
}
//*************************************************************************************
void CBCGPDateTimeCtrl::SetState (UINT stateFlags, UINT stateMask)
{
	if(!(stateFlags & DTM_DATE) && !(stateFlags & DTM_TIME))
		stateFlags |= (DTM_DATE | DTM_TIME);

	if(stateMask & DTM_SPIN)
		m_spinButton = 
			((stateFlags & DTM_SPIN) != 0);

	if(stateMask & DTM_DROPCALENDAR)
		m_dropCalendar = 
			((stateFlags & DTM_DROPCALENDAR) != 0);

	if(stateMask & DTM_DATE)
		m_showDate = 
			((stateFlags & DTM_DATE) != 0);

	if(stateMask & DTM_TIME24H)
		m_b24HoursFormat = 
			((stateFlags & DTM_TIME24H) != 0);

	if(stateMask & DTM_CHECKBOX)
		m_checkButton = 
			((stateFlags & DTM_CHECKBOX) != 0);

	if(stateMask & DTM_TIME)
		m_showTime = 
			((stateFlags & DTM_TIME) != 0);

	if(stateMask & DTM_CHECKED)
		m_bIsChecked = 
			((stateFlags & DTM_CHECKED) != 0);

	if(stateMask & DTM_TIME24HBYLOCALE)
		m_b24HoursByLocale = 
			((stateFlags & DTM_TIME24HBYLOCALE) != 0);

	if(stateMask & DTM_SECONDS)
		m_bSeconds = ((stateFlags & DTM_SECONDS) != 0);

	if(::IsWindow (m_hWnd))
	{
		SizeToContent();
	}
}
//*************************************************************************************
void CBCGPDateTimeCtrl::PreSubclassWindow() 
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

	rectClient.right -= 4;
	rectClient.bottom -= 4;

	AdjustControl (rectClient);
}
//*************************************************************************************
BOOL CBCGPDateTimeCtrl::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= BS_OWNERDRAW;
	cs.style &= ~BS_DEFPUSHBUTTON;

	m_bIsInitialized = TRUE;

	return CButton::PreCreateWindow(cs);
}
//*************************************************************************************
BOOL CBCGPDateTimeCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}
//*************************************************************************************
void CBCGPDateTimeCtrl::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
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
//*************************************************************************************
void CBCGPDateTimeCtrl::OnNcPaint() 
{
	if (GetStyle () & WS_BORDER)
	{
		visualManager->OnDrawControlBorder (this);
	}
}
//*************************************************************************************
BOOL CBCGPDateTimeCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (m_bIsChecked)
	{
		CPoint ptCursor;
		::GetCursorPos (&ptCursor);
		ScreenToClient (&ptCursor);

		if (m_rectText.PtInRect (ptCursor) && !m_rectDropButton.PtInRect (ptCursor))
		{
			::SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_IBEAM));
			return TRUE;
		}
	}

	return CButton::OnSetCursor(pWnd, nHitTest, message);
}
//*****************************************************************************
LRESULT CBCGPDateTimeCtrl::OnSetFont (WPARAM wParam, LPARAM lParam)
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
LRESULT CBCGPDateTimeCtrl::OnGetFont (WPARAM, LPARAM)
{
	return (LRESULT) m_hFont;
}
//******************************************************************************
void CBCGPDateTimeCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CButton::OnSize(nType, cx, cy);

	CRect rectClient;
	GetClientRect (rectClient);

	AdjustControl (rectClient);
	RedrawWindow ();
}
//*******************************************************************************
void CBCGPDateTimeCtrl::SetTextColor (COLORREF color, BOOL bRedraw)
{
	m_colorText = color;

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//*******************************************************************************
void CBCGPDateTimeCtrl::SetBackgroundColor (COLORREF color, BOOL bRedraw)
{
	m_colorBackground = color;

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow ();
	}
}
//*******************************************************************************
CString CBCGPDateTimeCtrl::ToString () const
{
	CString strResult;

	int iStart = m_bCheckBoxIsAvailable ? 1 : 0;

	for (int i = iStart; i < m_iPartsNumber; i ++)
	{
		if (m_arPartsOrder [i] == YEAR)
		{
			CString str;
			str.Format(_T("%04d"), m_Date.GetYear());

			strResult += str;
		}
		else
		{
			strResult += PartToString(m_Date, i);
		}

		//---------------
		// Add separator:
		//---------------
		if (i < m_iPartsNumber - 1)
		{
			CString strSeparator = _T(" ");

			if (IsDatePart (i) && IsDatePart (i + 1))
			{
				strSeparator = m_strDateSeparator;
			}

			if (IsTimePart (i) && IsTimePart (i + 1))
			{
				strSeparator = m_strTimeSeparator;
			}
		
			strResult += strSeparator;
		}
	}

	return strResult;
}
//*******************************************************************************
void CBCGPDateTimeCtrl::ClosePopupCalendar (COleDateTime date/* = COleDateTime ()*/)
{
	if (m_pPopup == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pPopup);
	ASSERT(::IsWindow (m_pPopup->GetSafeHwnd ()));

	BOOL bIsDateChanged = FALSE;

	if (date != COleDateTime ())
	{
		COleDateTime dateOld = GetDate ();

		if (dateOld.GetDay () != date.GetDay () ||
			dateOld.GetMonth () != date.GetMonth () ||
			dateOld.GetYear () != date.GetYear ())
		{
			bIsDateChanged = TRUE;
		}

		SetDate (date);
	}

	m_pPopup->SendMessage (WM_CLOSE);

	if (::IsWindow (GetSafeHwnd ()))
	{
		m_pPopup = NULL;
		SetFocus ();

		if (bIsDateChanged)
		{
			OnDateChanged ();
		}
	}
}
//*******************************************************************************
void CBCGPDateTimeCtrl::OnDestroy() 
{
	ClosePopupCalendar ();
	CButton::OnDestroy();
}
//*******************************************************************************
void CBCGPDateTimeCtrl::OnEnable(BOOL bEnable) 
{
	CButton::OnEnable(bEnable);

	if (::IsWindow (m_wndSpin.GetSafeHwnd ()))
	{
		if (bEnable && m_bCheckBoxIsAvailable && !m_bIsChecked)
		{
			bEnable = FALSE;
		}

		m_wndSpin.EnableWindow (bEnable);
		m_wndSpin.RedrawWindow ();
	}
}
//*******************************************************************************
void CBCGPDateTimeCtrl::OnCancelMode() 
{
	CButton::OnCancelMode();
	
	if (m_bTrackPart)
	{
		OnMouseLeave(0, 0);
	}
}
//*******************************************************************************
void CBCGPDateTimeCtrl::EnableVisualManagerStyle(BOOL bEnable)
{
	m_bVisualManagerStyle = bEnable;

	if (GetSafeHwnd() != NULL)
	{
		RedrawWindow();
	}
}
//****************************************************************************
LRESULT CBCGPDateTimeCtrl::OnPrint(WPARAM wp, LPARAM lp)
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
BOOL CBCGPDateTimeCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (m_pPopup != NULL &&	::IsWindow (m_pPopup->GetSafeHwnd ()))
	{
		return (BOOL)m_pPopup->SendMessage(WM_MOUSEWHEEL, MAKEWPARAM(nFlags, zDelta), MAKELPARAM(pt.x, pt.y));
	}

	return DoScroll(zDelta / WHEEL_DELTA);
}
//*********************************************************************************
BOOL CBCGPDateTimeCtrl::DoScroll(int nScrollSteps)
{
	if (nScrollSteps == 0)
	{
		return FALSE;
	}

	for (int i = 0; i < abs(nScrollSteps); i++)
	{
		ScrollCurrPart (nScrollSteps < 0 ? 1 : -1);
	}

	return TRUE;
}
