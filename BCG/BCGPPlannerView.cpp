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
// BCGPPlannerView.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPPlannerView.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerManagerCtrl.h"

#include "BCGPDrawManager.h"

#ifndef _BCGPCALENDAR_STANDALONE
#ifndef _BCGSUITE_
	#include "BCGPPopupMenu.h"
#endif
	#include "BCGPVisualManager.h"
	#include "bcgprores.h"

	#define visualManager	CBCGPVisualManager::GetInstance ()
#else
	#include "resource.h"
	#include "BCGPCalendarVisualManager.h"
	
	#define visualManager	CBCGPCalendarVisualManager::GetInstance ()
#endif

#include "BCGPAppointmentStorage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BCGP_PLANNER_TIMER_EDIT_EVENT 2005 

// static members
long CBCGPPlannerView::GetTimeDeltaInMinuts (BCGP_PLANNER_TIME_DELTA delta)
{
	ASSERT(BCGP_PLANNER_TIME_DELTA_FIRST <= delta);
	ASSERT(delta <= BCGP_PLANNER_TIME_DELTA_LAST);

	long nRes = 60;

	switch (delta)
	{
	case BCGP_PLANNER_TIME_DELTA_60:
		nRes = 60;
		break;
	case BCGP_PLANNER_TIME_DELTA_30:
		nRes = 30;
		break;
	case BCGP_PLANNER_TIME_DELTA_20:
		nRes = 20;
		break;
	case BCGP_PLANNER_TIME_DELTA_15:
		nRes = 15;
		break;
	case BCGP_PLANNER_TIME_DELTA_10:
		nRes = 10;
		break;
	case BCGP_PLANNER_TIME_DELTA_6:
		nRes = 6;
		break;
	case BCGP_PLANNER_TIME_DELTA_5:
		nRes = 5;
		break;
	case BCGP_PLANNER_TIME_DELTA_4:
		nRes = 4;
		break;
	case BCGP_PLANNER_TIME_DELTA_3:
		nRes = 3;
		break;
	case BCGP_PLANNER_TIME_DELTA_2:
		nRes = 2;
		break;
	case BCGP_PLANNER_TIME_DELTA_1:
		nRes = 1;
		break;
	default:
		ASSERT (FALSE);
	}

	return nRes;
}

COleDateTime CBCGPPlannerView::GetFirstWeekDay (const COleDateTime& day, int nWeekStart)
{
	return day - COleDateTimeSpan((day.GetDayOfWeek () - nWeekStart + 7) % 7, 0, 0, 0);
}

BOOL CBCGPPlannerView::Is24Hours ()
{
	TCHAR szLocaleData[2];
	::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_ITIME, szLocaleData, 2);

	return szLocaleData [0] == TCHAR('1');
}

CString CBCGPPlannerView::GetTimeDesignator (BOOL bAM)
{
	CString str;

	::GetLocaleInfo (LOCALE_USER_DEFAULT, bAM ? LOCALE_S1159 : LOCALE_S2359, 
		str.GetBuffer (10), 10);

	str.ReleaseBuffer ();

	return str;
}

CString CBCGPPlannerView::GetTimeSeparator ()
{
	CString str;

	::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_STIME, str.GetBuffer (10), 10);

	str.ReleaseBuffer ();

	return str;
}

BOOL CBCGPPlannerView::IsDateBeforeMonth ()
{
	TCHAR szLocaleData[2];
	::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_IDATE, szLocaleData, 2);

	return szLocaleData [0] == TCHAR('1');
}

CString CBCGPPlannerView::GetDateSeparator ()
{
	CString str;

	::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SDATE, str.GetBuffer (10), 10);

	str.ReleaseBuffer ();

	return str;
}

BOOL CBCGPPlannerView::IsDayLZero ()
{
	TCHAR szLocaleData[2];
	::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_IDAYLZERO, szLocaleData, 2);

	return szLocaleData [0] == TCHAR('1');
}

BOOL CBCGPPlannerView::IsMonthLZero ()
{
	TCHAR szLocaleData[2];
	::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_IMONLZERO, szLocaleData, 2);

	return szLocaleData [0] == TCHAR('1');
}

int CBCGPPlannerView::round(double val)
{
	const double c_Median = 0.5;
	return int((val - int(val - c_Median)) >= c_Median ? val + c_Median : val - c_Median);
}

BOOL CBCGPPlannerView::IsAppointmentInDate (const CBCGPAppointment& rApp, const COleDateTime& date)
{
	COleDateTime dt (date);
	dt.SetDate (dt.GetYear (), dt.GetMonth (), dt.GetDay ());

	COleDateTime dtS (rApp.GetStart ());
	dtS.SetDate (dtS.GetYear (), dtS.GetMonth (), dtS.GetDay ());

	COleDateTime dtF (rApp.GetFinish ());

	if (rApp.IsAllDay ())
	{
		dtF.SetDate (dtF.GetYear (), dtF.GetMonth (), dtF.GetDay ());

		return (dtS <= dt && dt <= dtF);
	}

	return (dtS <= dt && dt < dtF) || (dtS == dt && dt == dtF);
}


BOOL CBCGPPlannerView::IsOneDay (const COleDateTime& date1, const COleDateTime& date2)
{
	return date1.GetYear () == date2.GetYear () &&
		   date1.GetDayOfYear () == date2.GetDayOfYear ();
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerView

IMPLEMENT_DYNAMIC(CBCGPPlannerView, CObject)

CBCGPPlannerView::CBCGPPlannerView()
	: m_pPlanner           (NULL)
	, m_strCaptionFormat   ()
	, m_nHeaderScrollOffset(0)
	, m_nHeaderScrollTotal (0)
	, m_nHeaderScrollPage  (0)
	, m_nScrollOffset      (0)
	, m_nScrollTotal       (0)
	, m_nScrollPage        (0)
	, m_rectApps           (0, 0, 0, 0)
	, m_nRowHeight         (0)
	, m_Font               ()
	, m_FontBold           ()
	, m_FontVert           ()
	, m_bActive            (FALSE)
	, m_dtCaptureStart     ()
	, m_dtCaptureCurrent   ()
	, m_pDragAppointments  (NULL)
	, m_TimerEdit          (NULL)
	, m_AdjustAction       (BCGP_PLANNER_ADJUST_ACTION_NONE)
	, m_pCapturedAppointment        (NULL)
	, m_hitCapturedAppointment      (BCGP_PLANNER_HITTEST_NOWHERE)
	, m_dtCapturedAppointment       ()
	, m_bCapturedAppointmentChanged (FALSE)
	, m_htCaptureAreaStart          (BCGP_PLANNER_HITTEST_NOWHERE)
	, m_htCaptureAreaCurrent        (BCGP_PLANNER_HITTEST_NOWHERE)
	, m_htCaptureResourceStart      (CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID)
	, m_htCaptureResourceCurrent    (CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID)
	, m_bUpdateToolTipInfo (FALSE)
{
	m_hcurAppHorz = AfxGetApp ()->LoadStandardCursor (IDC_SIZEWE);
	m_hcurAppVert = AfxGetApp ()->LoadStandardCursor (IDC_SIZENS);
}

CBCGPPlannerView::~CBCGPPlannerView()
{
	if (m_pDragAppointments != NULL)
	{
		delete m_pDragAppointments;
	}

	if (m_hcurAppVert != NULL)
	{
		::DestroyCursor (m_hcurAppVert);
		m_hcurAppVert = NULL;
	}

	if (m_hcurAppHorz != NULL)
	{
		::DestroyCursor (m_hcurAppHorz);
		m_hcurAppHorz = NULL;
	}
}

#ifdef _DEBUG
void CBCGPPlannerView::AssertValid() const
{
	CObject::AssertValid();
}

void CBCGPPlannerView::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif

HFONT CBCGPPlannerView::SetCurrFont (CDC* pDC, BOOL bBold/* = FALSE*/)
{
	ASSERT_VALID (pDC);
	
	HFONT hFont = GetFont (bBold);

	if (hFont == NULL)
	{
		return NULL;
	}

	return (HFONT) ::SelectObject (pDC->GetSafeHdc (), hFont);
}

BOOL CBCGPPlannerView::IsDateInSelection (const COleDateTime& date) const
{
	const COleDateTime& pSel1 = m_Selection[0] < m_Selection [1] 
		? m_Selection [0]
		: m_Selection [1];
	const COleDateTime& pSel2 = m_Selection[0] < m_Selection [1] 
		? m_Selection [1]
		: m_Selection [0];

	if (pSel1 <= date && date <= pSel2)
	{
		return TRUE;
	}

	return FALSE;
}

COleDateTime CBCGPPlannerView::GetDateFromPoint (const CPoint& point) const
{
	COleDateTime date;

	int nDay = -1;

	const int nDuration = GetViewDuration ();

	for (int i = 0; i < nDuration; i++)
	{
		CRect rt (m_ViewRects[i]);
		rt.right++;
		rt.bottom++;

		if (rt.PtInRect (point))
		{
			nDay = i;
			break;
		}
	}

	if (nDay != -1)
	{
		date = GetDateStart () + COleDateTimeSpan (nDay, 0, 0, 0);
	}

	return date;
}

int CBCGPPlannerView::GetWeekFromPoint (const CPoint& /*point*/) const
{
	return -1;
}

CBCGPAppointment* CBCGPPlannerView::GetAppointmentFromPoint (const CPoint& point)
{
	CBCGPAppointment* pApp = NULL;

	XBCGPAppointmentArray& arApps = GetQueryedAppointments ();

	for (int i = 0; i < arApps.GetSize (); i++)
	{
		if (arApps[i]->PointInRectDraw (point))
		{
			pApp = arApps[i];
			break;
		}
	}

	return pApp;
}

CRect CBCGPPlannerView::GetRectFromDate(const COleDateTime& date) const
{
	CRect rect(0, 0, 0, 0);

	if (date < GetDateStart() || GetDateEnd() < date)
	{
		return rect;
	}

	const int nDays = GetViewDuration ();
	if (m_ViewRects.GetSize() != nDays)
	{
		return rect;
	}

	COleDateTime dt(date.GetYear(), date.GetMonth(), date.GetDay(), 0, 0, 0);
	COleDateTime dtCur(GetDateStart());

	int nIndex = (int)((dt - dtCur).GetTotalDays());
	if (0 <= nIndex && nIndex < nDays)
	{
		rect = m_ViewRects[nIndex];
	}

	return rect;
}

void CBCGPPlannerView::AdjustCaptionFormat (CDC* pDC)
{
	ASSERT_VALID (pDC);

	const int nDays = GetViewDuration ();

	CStringArray sa;
	GetCaptionFormatStrings (sa);

	BOOL bCompact = (GetPlanner ()->GetDrawFlags () & BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_COMPACT) ==
			BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_COMPACT;

	for (int n = 0; n < sa.GetSize (); n++)
	{
		m_strCaptionFormat = sa [n];

		if (n == sa.GetSize () - 1)
		{
			break;
		}

		COleDateTime day (GetDateStart ());

		BOOL bFlag = FALSE;

		for (int nDay = 0; nDay < nDays; nDay++)
		{
			CString strDate;
			ConstructCaptionText (day, strDate, m_strCaptionFormat);

			CSize sz (pDC->GetTextExtent (strDate));

			day += COleDateTimeSpan (1, 0, 0, 0);

			if (bCompact)
			{
				sz.cx *= 10;
				sz.cx /= 6;
			}

			if (m_ViewRects [nDay].Width () < sz.cx + 6)
			{
				bFlag = TRUE;
				break;
			}
		}

		if (!bFlag)
		{
			break;
		}
	}
}

void CBCGPPlannerView::AdjustScrollSizes ()
{
	ASSERT_VALID(m_pPlanner);

	SCROLLINFO si;

	ZeroMemory (&si, sizeof (SCROLLINFO));
	si.cbSize = sizeof (SCROLLINFO);
	si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	si.nMin  = 0;

	CScrollBar* pwndScroll = NULL;
	if (GetPlanner()->IsHeaderScrollingEnabled())
	{
		CScrollBar* pwndScroll = m_pPlanner->GetHeaderScrollBarCtrl (SB_VERT);
		if (pwndScroll->GetSafeHwnd () != NULL)
		{
			si.nMax  = m_nHeaderScrollTotal;
			si.nPage = m_nHeaderScrollPage;
			si.nPos  = m_nHeaderScrollOffset;

			pwndScroll->SetScrollInfo (&si, TRUE);
			pwndScroll->EnableScrollBar (ESB_ENABLE_BOTH);
			pwndScroll->EnableWindow ();
		}
	}

	pwndScroll = m_pPlanner->GetScrollBarCtrl (SB_VERT);
	if (pwndScroll->GetSafeHwnd () != NULL)
	{
		si.nMax  = m_nScrollTotal;
		si.nPage = m_nScrollPage;
		si.nPos  = m_nScrollOffset;

		pwndScroll->SetScrollInfo (&si, TRUE);
		pwndScroll->EnableScrollBar (ESB_ENABLE_BOTH);
		pwndScroll->EnableWindow ();
	}
}

void CBCGPPlannerView::AdjustLayout (BOOL bRedraw)
{
	if (!m_bActive)
	{
		return;
	}

	ASSERT_VALID(m_pPlanner);

	if (m_pPlanner->GetSafeHwnd () == NULL)
	{
		return;
	}

	StopEditAppointment ();

	CClientDC dc (m_pPlanner);
	HFONT hfontOld = SetCurrFont (&dc, TRUE);

	TEXTMETRIC tm;
	dc.GetTextMetrics (&tm);
	m_nRowHeight    = max
						(
							max
							(
								tm.tmHeight + 4 + visualManager->GetPlannerRowExtraHeight (),
								m_pPlanner->GetClockIconSize ().cy + 2
							), 
							CBCGPPlannerManagerCtrl::GetImageSize ().cy + 2
						);

	CRect rectClient;
	m_pPlanner->GetClientRect (rectClient);

	m_rectApps = rectClient;

	AdjustLayout (&dc, rectClient);

	const int cxScroll = ::GetSystemMetrics (SM_CXHSCROLL);

	CScrollBar* pHeaderScrollBar = m_pPlanner->GetHeaderScrollBarCtrl (SB_VERT);
	CScrollBar* pScrollBar = m_pPlanner->GetScrollBarCtrl (SB_VERT);
	if (m_pPlanner->IsScrollBarVisible ())
	{
		m_rectApps.right -= cxScroll;
		int nScrollTop    = rectClient.top;
		int nScrollHeight = rectClient.Height ();

		if (m_pPlanner->IsHeaderScrollingEnabled() && CanUseHeaderScrolling())
		{
			pHeaderScrollBar->ShowScrollBar (TRUE);
			pHeaderScrollBar->SetWindowPos (NULL, m_rectApps.right, 
				rectClient.top, cxScroll, rectClient.Height () - m_rectApps.Height (), SWP_NOZORDER | SWP_NOACTIVATE);

			nScrollTop    = m_rectApps.top;
			nScrollHeight = m_rectApps.Height ();
		}
		else
		{
			pHeaderScrollBar->ShowScrollBar (FALSE);
		}

		pScrollBar->ShowScrollBar (TRUE);
		pScrollBar->SetWindowPos (NULL, m_rectApps.right, 
			nScrollTop, cxScroll, nScrollHeight, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		pHeaderScrollBar->ShowScrollBar (FALSE);
		pScrollBar->ShowScrollBar (FALSE);
	}

	m_ViewRects.RemoveAll ();

	ClearVisibleUpDownIcons ();

	AdjustRects ();

	AdjustCaptionFormat (&dc);

	AdjustAppointments ();

	SetSelection(GetSelectionStart (), GetSelectionEnd (), FALSE);

	if (bRedraw)
	{
		m_pPlanner->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	::SelectObject (dc.GetSafeHdc (), hfontOld);
}

void CBCGPPlannerView::CheckVisibleAppointments(const COleDateTime& date, const CRect& rect, 
	BOOL bFullVisible)
{
	ASSERT (date != COleDateTime ());

	XBCGPAppointmentArray& arQueryApps = GetQueryedAppointments ();
	XBCGPAppointmentArray& arDragApps = GetDragedAppointments ();

	if (arQueryApps.GetSize () == 0 && arDragApps.GetSize () == 0)
	{
		return;
	}

	BOOL bDragDrop        = IsDragDrop ();
	DROPEFFECT dragEffect = GetDragEffect ();
	BOOL bDragMatch       = IsCaptureMatched ();	

	bDragDrop = !bDragDrop || 
		(bDragDrop && ((dragEffect & DROPEFFECT_COPY) == DROPEFFECT_COPY && bDragMatch) || 
		!bDragMatch);
	bDragDrop = bDragDrop && arDragApps.GetSize ();

	BOOL bSelect = date != COleDateTime ();

	for (int nApp = 0; nApp < 2; nApp++)
	{
		if (!bDragDrop && nApp == 0)
		{
			continue;
		}

		XBCGPAppointmentArray& arApps = nApp == 0 ? arDragApps : arQueryApps;

		for (int i = 0; i < arApps.GetSize (); i++)
		{
			CBCGPAppointment* pApp = arApps [i];

			if (bSelect)
			{
				if (!IsAppointmentInDate (*pApp, date))
				{
					continue;
				}
			}

			CRect rtDraw (pApp->GetRectDraw (date));

			CRect rtInter;
			rtInter.IntersectRect (rtDraw, rect);

			pApp->SetVisibleDraw ((!bFullVisible && rtInter.Height () >= 2) || 
				(bFullVisible && rtInter.Height () == rtDraw.Height () && 
				 rtInter.bottom < rect.bottom), date);
		}
	}
}

void CBCGPPlannerView::ClearVisibleUpDownIcons()
{
	m_UpRects.RemoveAll ();
	m_DownRects.RemoveAll ();
}

void CBCGPPlannerView::CheckVisibleUpDownIcons(BOOL bFullVisible)
{
	ClearVisibleUpDownIcons ();

	CSize sz (GetPlanner ()->GetUpDownIconSize ());

	if (sz == CSize (0, 0))
	{
		return;
	}

	XBCGPAppointmentArray& arQueryApps = GetQueryedAppointments ();
	XBCGPAppointmentArray& arDragApps  = GetDragedAppointments ();

	if (arQueryApps.GetSize () == 0 && arDragApps.GetSize () == 0)
	{
		return;
	}

	BOOL bDragDrop        = IsDragDrop ();
	DROPEFFECT dragEffect = GetDragEffect ();
	BOOL bDragMatch       = IsCaptureMatched ();

	bDragDrop = !bDragDrop || 
		(bDragDrop && ((dragEffect & DROPEFFECT_COPY) == DROPEFFECT_COPY && bDragMatch) || 
		!bDragMatch);
	bDragDrop = bDragDrop && arDragApps.GetSize ();

	const int nDays = GetViewDuration ();

	COleDateTime date (m_DateStart);

	for (int nDay = 0; nDay < nDays; nDay++)
	{
		BYTE res = 0;
		CRect rect (m_ViewRects [nDay]);

		for (int nApp = 0; nApp < 2; nApp++)
		{
			XBCGPAppointmentArray& arApps = nApp == 1 ? arDragApps : arQueryApps;

			if (nApp == 1)
			{
				bDragDrop = bDragDrop && arDragApps.GetSize ();
			}

			if (arApps.GetSize () == 0)
			{
				continue;
			}

			for (int i = 0; i < arApps.GetSize (); i++)
			{
				CBCGPAppointment* pApp = arApps [i];
				if (pApp == NULL)
				{
					continue;
				}

				if (!IsAppointmentInDate (*pApp, date))
				{
					continue;
				}

				if (!pApp->IsVisibleDraw (date))
				{
					BOOL bAdd = FALSE;

					if (bDragDrop && dragEffect != DROPEFFECT_NONE && pApp->IsSelected () &&
						nApp == 0)
					{
						if ((dragEffect & DROPEFFECT_COPY) == DROPEFFECT_COPY || bDragMatch)
						{
							bAdd = TRUE;
						}
					}
					else
					{
						bAdd = TRUE;
					}

					if (bAdd)
					{
						if (bFullVisible)
						{
							res |= 0x02;
						}
						else
						{
							if (!rect.IsRectNull ())
							{
								CRect rectDraw (pApp->GetRectDraw (date));

								if (rectDraw.bottom <= rect.top)
								{
									res |= 0x01;
								}
								else if (rect.bottom <= (rectDraw.top + 1))
								{
									res |= 0x02;
								}
							}
						}
					}

					if (res == 0x03)
					{
						break;
					}
				}
			}

			if (res == 0x03)
			{
				break;
			}
		}

		if (res != 0)
		{
			AddUpDownRect (res, rect);
		}

		date += COleDateTimeSpan (1, 0, 0, 0);
	}
}

BYTE CBCGPPlannerView::OnDrawAppointments (CDC* pDC, const CRect& rect, const COleDateTime& date)
{
	BYTE res = 0;

	XBCGPAppointmentArray& arQueryApps = GetQueryedAppointments ();
	XBCGPAppointmentArray& arDragApps  = GetDragedAppointments ();

	if (arQueryApps.GetSize () == 0 && arDragApps.GetSize () == 0)
	{
		return res;
	}

	BOOL bSelect = date != COleDateTime ();

	BOOL bDragDrop        = IsDragDrop ();
	DROPEFFECT dragEffect = GetDragEffect ();
	BOOL bDragMatch       = IsCaptureMatched ();

	for (int nApp = 0; nApp < 2; nApp++)
	{
		XBCGPAppointmentArray& arApps = nApp == 1 ? arDragApps : arQueryApps;

		if (nApp == 1)
		{
			bDragDrop = bDragDrop && arDragApps.GetSize ();
		}

		if (arApps.GetSize () == 0)
		{
			continue;
		}

		for (int i = 0; i < arApps.GetSize (); i++)
		{
			CBCGPAppointment* pApp = arApps [i];
			if (pApp == NULL)
			{
				continue;
			}

			if (bSelect)
			{
				if (!IsAppointmentInDate (*pApp, date))
				{
					continue;
				}
			}

			if (!bSelect || (bSelect && pApp->IsVisibleDraw (date)))
			{
				if (bDragDrop && dragEffect != DROPEFFECT_NONE && pApp->IsSelected () &&
					nApp == 0)
				{
					if ((dragEffect & DROPEFFECT_COPY) == DROPEFFECT_COPY || bDragMatch)
					{
						pApp->SetSelected (FALSE);
						pApp->OnDraw (pDC, this, date);
						pApp->SetSelected (TRUE);
					}
				}
				else
				{
					pApp->OnDraw (pDC, this, date);
				}
			}
			else
			{
				if (!rect.IsRectNull ())
				{
					if (pApp->GetRectDraw (date).bottom <= rect.top)
					{
						res |= 0x01;
					}
					else
					{
						res |= 0x02;
					}
				}
				else
				{
					res |= 0x01;
				}
			}
		}
	}

	return res;
}

void CBCGPPlannerView::OnDrawUpDownIcons (CDC* pDC)
{
	if ((GetPlanner ()->GetDrawFlags () & BCGP_PLANNER_DRAW_VIEW_NO_UPDOWN) == 
			BCGP_PLANNER_DRAW_VIEW_NO_UPDOWN)
	{
		return;
	}

	ASSERT_VALID (pDC);

	if (pDC->GetSafeHdc () == NULL)
	{
		return;
	}

	CSize sz (GetPlanner ()->GetUpDownIconSize ());

	if (m_rectApps.Height () < sz.cy)
	{
		return;
	}

	for (int i = 0; i < 2; i++)
	{
		HICON hIcon = GetPlanner ()->GetUpDownIcon (i);
		if (hIcon == NULL)
		{
			continue;
		}

		CArray<CRect, CRect&>& ar = i == 0 ? m_UpRects : m_DownRects;

		for (int j = 0; j < ar.GetSize (); j++)
		{
			::DrawIconEx (pDC->GetSafeHdc (), 
				ar[j].left, ar[j].top, hIcon, 
				sz.cx, sz.cy, 0, NULL, DI_NORMAL);
		}

		::DestroyIcon (hIcon);
	}
}

void CBCGPPlannerView::DrawHeader (CDC* pDC, const CRect& rect, int dxColumn)
{
	ASSERT_VALID (pDC);

	visualManager->OnDrawPlannerHeader (pDC, this, rect);

	CRect rt (rect);

	rt.right = rt.left + dxColumn;
	rt.DeflateRect (1, 0);

	while (rt.right < (rect.right - 4))
	{
		CRect rectPane (
			CPoint (rt.right, rt.top + 2), CSize (2, rt.Height () - 4));

		visualManager->OnDrawPlannerHeaderPane (pDC, this, rectPane);

		rt.OffsetRect (dxColumn, 0);
	}
}

void CBCGPPlannerView::DrawCaptionText (CDC* pDC, CRect rect, 
	const CString& strText, COLORREF clrText, int nAlign, BOOL bHighlight)
{
	ASSERT_VALID (pDC);

	visualManager->OnDrawPlannerCaptionText (pDC, this, rect, strText, clrText,
		nAlign, bHighlight);
}

void CBCGPPlannerView::DrawCaptionText (CDC* pDC, CRect rect,
	const COleDateTime& day, COLORREF clrText, int nAlign, BOOL bHighlight)
{
	ASSERT_VALID (pDC);

	CString strText;

	ConstructCaptionText (day, strText, m_strCaptionFormat);

	DrawCaptionText (pDC, rect, strText, clrText, nAlign, bHighlight);
}

void CBCGPPlannerView::ConstructCaptionText (const COleDateTime& day, CString& strText, const CString& strFormat)
{
	strText.Empty ();
	strText.GetBuffer (_MAX_PATH);

	SYSTEMTIME st;
	day.GetAsSystemTime (st);

	::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st, 
		strFormat, (LPTSTR)(LPCTSTR)strText, _MAX_PATH);

	strText.ReleaseBuffer ();
}

void CBCGPPlannerView::DrawAppointment (CDC* pDC, CBCGPAppointment* pApp, CBCGPAppointmentDrawStructEx* pDS)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pApp);

	if (pDC == NULL || pDC->GetSafeHdc () == NULL || pApp == NULL || pDS == NULL)
	{
		return;
	}

	CBCGPPlannerManagerCtrl* pPlanner = GetPlanner ();
	ASSERT_VALID (pPlanner);

	const DWORD dwDrawFlags = pPlanner->GetDrawFlags ();
	const BOOL bIsGradientFill      = dwDrawFlags & BCGP_PLANNER_DRAW_APP_GRADIENT_FILL;
	const BOOL bIsRoundedCorners    = dwDrawFlags & BCGP_PLANNER_DRAW_APP_ROUNDED_CORNERS;
	const BOOL bIsOverrideSelection = dwDrawFlags & BCGP_PLANNER_DRAW_APP_OVERRIDE_SELECTION;

	CRect rect (pDS->GetRect ());

	COleDateTime dtStart  (pApp->GetStart ());
	COleDateTime dtFinish (pApp->GetFinish ());

	CString      strStart  (pApp->m_strStart);
	CString      strFinish (pApp->m_strFinish);

	const BOOL bDrawShadow  = IsDrawAppsShadow () && !pApp->IsSelected ();
	const BOOL bAlternative = pApp->IsAllDay () || pApp->IsMultiDay ();
	const BOOL bEmpty       = (dtStart == dtFinish) && !bAlternative;
	const BOOL bSelected    = pApp->IsSelected ();

	BOOL bAllBoders   = TRUE;
	if (bAlternative)
	{
		bAllBoders = pDS->GetBorder () == CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_ALL;
	}

	if (!bAlternative && !IsOneDay (dtStart, dtFinish))
	{
		if (pDS->GetBorder () == CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START)
		{
			dtFinish  = COleDateTime (dtFinish.GetYear (), dtFinish.GetMonth (), dtFinish.GetDay (),
				0, 0, 0);
			strFinish = CBCGPAppointment::GetFormattedTimeString (dtFinish, TIME_NOSECONDS, FALSE);
			strFinish.MakeLower ();
		}
		else if (pDS->GetBorder () == CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH)
		{
			dtStart  = COleDateTime (dtFinish.GetYear (), dtFinish.GetMonth (), dtFinish.GetDay (),
				0, 0, 0);
			strStart = CBCGPAppointment::GetFormattedTimeString (dtStart, TIME_NOSECONDS, FALSE);
			strStart.MakeLower ();
		}
	}

	CRgn rgn;
	if (bIsRoundedCorners)
	{
		int left  = 3;
		int right = 3;

		if (bAlternative)
		{
			if (!bAllBoders)
			{
				if ((pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START) == 0)
				{
					left = 0;
				}

				if ((pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH) == 0)
				{
					right = 0;
				}
			}

			if (bSelected)
			{
				rect.InflateRect (0, 1);
			}
		}
		else
		{
			if (bSelected)
			{
				if (!bIsOverrideSelection)
				{
					left  = 0;
					right = 0;
				}
				else
				{
					rect.InflateRect (0, 1);
				}
			}
		}

		rect.DeflateRect (left, 0, right, 0);

		rgn.CreateRoundRectRgn (rect.left, rect.top, rect.right + 1, rect.bottom + 1, 5, 5);

		if (bAlternative)
		{
			if (!bAllBoders)
			{
				if ((pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START) == 0)
				{
					CRgn rgnTmp;
					rgnTmp.CreateRectRgn (rect.left - 1, rect.top, rect.left + 3, rect.bottom);
					rgn.CombineRgn (&rgn, &rgnTmp, RGN_OR);
				}

				if ((pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH) == 0)
				{
					CRgn rgnTmp;
					rgnTmp.CreateRectRgn (rect.right - 4, rect.top, rect.right + 1, rect.bottom);
					rgn.CombineRgn (&rgn, &rgnTmp, RGN_OR);
				}
			}
		}

		pDC->SelectClipRgn (&rgn, RGN_COPY);

		if (bSelected)
		{
			if (bAlternative || (!bAlternative && bIsOverrideSelection))
			{
				rect.DeflateRect (0, 1);
			}
		}
	}
	else
	{
		if (bAlternative)
		{
			rect.DeflateRect (3, 0);
		}
		else if (!bSelected || bIsOverrideSelection)
		{
			if (bIsOverrideSelection)
			{
				rect.DeflateRect (3, 0);
			}
			else if (!bSelected)
			{
				rect.DeflateRect (5, 0);
			}
		}
	}

	CBCGPDrawManager dm (*pDC);

	COLORREF clrBack1   = pApp->GetBackgroundColor ();
	COLORREF clrBack2   = clrBack1;
	COLORREF clrText    = pApp->GetForegroundColor ();
	COLORREF clrFrame1  = globalData.clrWindowFrame;
	COLORREF clrFrame2  = clrFrame1;
	COLORREF clrTextOld = CLR_DEFAULT;

	visualManager->GetPlannerAppointmentColors (this, bSelected, !bAlternative, 
		dwDrawFlags, clrBack1, clrBack2, clrFrame1, clrFrame2, clrText);

	if (bSelected && !bIsOverrideSelection)
	{
		CBrush br (clrBack1);
		pDC->FillRect (rect, &br);

		if (!bAlternative)
		{
			rect.DeflateRect (bIsRoundedCorners ? 3 : 5, 0);
		}
	}
	else
	{
		if (bIsGradientFill)
		{
			dm.FillGradient (rect, clrBack1, clrBack2);
		}
		else
		{
			if (pApp->m_clrBackgroung != CLR_DEFAULT || bAlternative)
			{
				CBrush br (clrBack1);
				pDC->FillRect (rect, &br);
			}
		}
	}

	if (bDrawShadow)
	{
		CRect rt (rect);
		rt.bottom++;

		dm.DrawShadow (rt, CBCGPAppointment::BCGP_PLANNER_SHADOW_DEPTH, 100, 75);
	}

	clrTextOld = pDC->SetTextColor (clrText);

	BOOL bCancelDraw = FALSE;
	BOOL bToolTipNeeded = FALSE;

	CSize szImage (CBCGPPlannerManagerCtrl::GetImageSize ());

	if (bIsRoundedCorners)
	{
		if (bAlternative || (bSelected && bIsOverrideSelection))
		{
			int nWidth = 2;

			if (!bSelected)
			{
				if (bAlternative)
				{
					nWidth = 1;
				}
			}

			CBrush br (clrFrame2);
			pDC->FrameRgn (&rgn, &br, nWidth, nWidth);
		}
	}
	else
	{
		if (bAlternative)
		{
			CPen pen (PS_SOLID, 0, clrFrame2);
			CPen* pOldPen = pDC->SelectObject (&pen);

			if (bAllBoders)
			{
				pDC->Draw3dRect (rect, clrFrame2, clrFrame2);
			}
			else
			{
				pDC->MoveTo (rect.left , rect.top);
				pDC->LineTo (rect.right, rect.top);

				pDC->MoveTo (rect.left , rect.bottom - 1);
				pDC->LineTo (rect.right, rect.bottom - 1);

				if (pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START)
				{
					pDC->MoveTo (rect.left, rect.top);
					pDC->LineTo (rect.left, rect.bottom);
				}

				if (pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH)
				{
					pDC->MoveTo (rect.right - 1, rect.top);
					pDC->LineTo (rect.right - 1, rect.bottom);
				}
			}

			if (bSelected)
			{
				CPoint pt (rect.left, rect.right);

				if ((pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START) == 0)
				{
					pt.x--;
				}

				if ((pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH) == 0)
				{
					pt.y++;
				}

				pDC->MoveTo (pt.x, rect.top - 1);
				pDC->LineTo (pt.y, rect.top - 1);

				pDC->MoveTo (pt.x, rect.bottom);
				pDC->LineTo (pt.y, rect.bottom);
			}

			pDC->SelectObject (pOldPen);
		}
		else
		{
			if (bSelected && bIsOverrideSelection)
			{
				CRect rt (rect);

				rt.DeflateRect (1, 0);
				pDC->Draw3dRect (rt, clrFrame2, clrFrame2);

				rt.InflateRect  (1, 1);
				pDC->Draw3dRect (rt, clrFrame2, clrFrame2);
			}
		}
	}

	if (bIsRoundedCorners || bIsOverrideSelection)
	{
		if (bIsRoundedCorners)
		{
			rect.DeflateRect (bAlternative ? 1 : 3, 0);
		}
		else
		{
			rect.DeflateRect (bAlternative ? 1 : 2, 0);
		}
	}

	CSize szClock (GetClockIconSize ());

	if (!pApp->IsAllDay ())
	{
		if ((IsDrawTimeAsIcons () || 
			(bAlternative && (dwDrawFlags & BCGP_PLANNER_DRAW_APP_NO_MULTIDAY_CLOCKS) == 0)) && 
			szClock != CSize (0, 0))
		{
			int top = (rect.Height () - szClock.cy) / 2;

			if (bAlternative)
			{
				if (pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START)
				{
					if (dtStart.GetHour () != 0 || dtStart.GetMinute () != 0 ||
						dtStart.GetSecond () != 0)
					{
						DrawClockIcon (pDC, CPoint (rect.left + 1, rect.top + top), 
							dtStart);

						rect.left += szClock.cx + 3;
					}
				}

				if (pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH)
				{
					if (dtFinish.GetHour () != 0 || dtFinish.GetMinute () != 0 ||
						dtFinish.GetSecond () != 0)
					{
						DrawClockIcon (pDC, CPoint (rect.right - szClock.cx - 1, rect.top + top), 
							dtFinish);

						rect.right -= (szClock.cx + 3);
					}
				}
			}
			else
			{
				if (rect.Width () >= szClock.cx * 3)
				{
					DrawClockIcon (pDC, CPoint (rect.left + 1, rect.top + top), 
						dtStart);

					rect.left += szClock.cx + 3;

					if (rect.Width () >= szClock.cx * 4 && IsDrawTimeFinish ())
					{
						DrawClockIcon (pDC, CPoint (rect.left + 1, rect.top + top), 
							dtFinish);

						rect.left += szClock.cx + 3;
					}
				}

				bToolTipNeeded = TRUE;
			}
		}
		else if (!strStart.IsEmpty ())
		{
			COLORREF clrTime = visualManager->GetPlannerAppointmentTimeColor(this, 
				bSelected, !bAlternative, dwDrawFlags);
			if (clrTime != CLR_DEFAULT)
			{
				pDC->SetTextColor (clrTime);
			}

			COleDateTime dt (2005, 1, 1, 23, 59, 0);

			CString strFormat (_T("%H"));
			strFormat += GetTimeSeparator () + _T("%M");
			if (!Is24Hours ())
			{
				strFormat += _T("%p");
			}
			CString strDT (dt.Format (strFormat));
			strDT.MakeLower ();

			CSize szSpace (pDC->GetTextExtent (_T(" ")));
			CSize szDT (pDC->GetTextExtent (strDT));

			if ((!bAlternative && rect.Width () >= (szDT.cx + szSpace.cx) * 2) ||
				( bAlternative && rect.Width () >= (szDT.cx + szSpace.cx) * 3))
			{
				if (bAlternative)
				{
					if (pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START)
					{
						if (dtStart.GetHour () != 0 || dtStart.GetMinute () != 0 ||
							dtStart.GetSecond () != 0)
						{
							CRect rectText (rect);
							rectText.DeflateRect (2, 0, 3, 1);
							rectText.right = rectText.left + szDT.cx;

							pDC->DrawText (strStart, rectText, 
								DT_NOPREFIX | DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
							rect.left = rectText.right + szSpace.cx;
						}
					}

					if (pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH)
					{
						if (dtFinish.GetHour () != 0 || dtFinish.GetMinute () != 0 ||
							dtFinish.GetSecond () != 0)
						{
							CRect rectText (rect);
							rectText.DeflateRect (2, 0, 3, 1);
							rectText.left = rectText.right - szDT.cx;

							pDC->DrawText (strFinish, rectText, 
								DT_NOPREFIX | DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
							rect.right = rectText.left - szSpace.cx;
						}
					}
				}
				else
				{
					CRect rectText (rect);
					rectText.DeflateRect (0, 0, 1, 1);
					rectText.right = rectText.left + szDT.cx;

					pDC->DrawText (strStart, rectText, 
						DT_NOPREFIX | DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
					rect.left = rectText.right + szSpace.cx;

					if (IsDrawTimeFinish ())
					{
						if (rect.Width () >= szDT.cx * 3)
						{
							rectText = rect;
							rectText.DeflateRect (0, 0, 1, 1);
							rectText.right = rectText.left + szDT.cx;

							if (!bEmpty)
							{
								pDC->DrawText (strFinish, rectText, 
									DT_NOPREFIX | DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
							}

							rect.left = rectText.right + szSpace.cx;
						}
						else
						{
							bToolTipNeeded = TRUE;
						}
					}
					else
					{
						bToolTipNeeded = TRUE;
					}
				}
			}
			else
			{
				bToolTipNeeded = TRUE;
			}

			if (clrTime != CLR_DEFAULT)
			{
				pDC->SetTextColor (clrText);
			}
		}
	}

	CRect rectEdit (rect);
	rectEdit.right = pDS->GetRect ().right;

	if (!bAlternative)
	{
		if (bIsOverrideSelection)
		{
			rectEdit.right -= 4;
		}
	}
	else
	{
		if (!bIsRoundedCorners)
		{
			rectEdit.right -= 3;
		}
	}

	pDS->SetRectEditHitTest (rectEdit);

	if (bAlternative)
	{
		if (bIsRoundedCorners)
		{
			rectEdit.left = pDS->GetRect ().left;

			if (pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START)
			{
				rectEdit.left += 4;
			}

			if (pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH)
			{
				rectEdit.right -= 4;
			}
		}
		else
		{
			rectEdit.left = pDS->GetRect ().left + 3;
			rectEdit.InflateRect (0, 1);

			if ((pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START) == 0)
			{
				rectEdit.left--;
			}

			if ((pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH) == 0)
			{
				rectEdit.right++;
			}
		}
	}

	rectEdit.DeflateRect (1, 1);
	pDS->SetRectEdit (rectEdit);

	rectEdit = pDS->GetRectEditHitTest ();
	rectEdit.DeflateRect (1, 1);
	pDS->SetRectEditHitTest (rectEdit);

	CDWordArray dwIcons;

	CRect rtIcons (rect);
	rtIcons.left++;

	if (szImage != CSize (0, 0))
	{
		if (pApp->m_RecurrenceClone &&
			(dwDrawFlags & BCGP_PLANNER_DRAW_APP_NO_RECURRENCE_IMAGE) == 0)
		{
			if (rtIcons.Width () > szImage.cx * 1.5)
			{
				dwIcons.Add (pApp->m_RecurrenceEcp ? 1 : 0);
				rtIcons.left += szImage.cx;
			}
		}

		if (!pApp->m_lstImages.IsEmpty () &&
			(dwDrawFlags & BCGP_PLANNER_DRAW_APP_NO_IMAGES) == 0)
		{
			for (POSITION pos = pApp->m_lstImages.GetHeadPosition (); pos != NULL;)
			{
				if (rtIcons.Width () <= szImage.cx)
				{
					bCancelDraw = TRUE;
					break;
				}

				rtIcons.left += szImage.cx;

				dwIcons.Add (pApp->m_lstImages.GetNext (pos));
			}
		}
	}

	BOOL bDrawText = !(pApp->m_strDescription.IsEmpty () || bCancelDraw);

	int nTextWidth = pDC->GetTextExtent (pApp->m_strDescription).cx;

	if (dwIcons.GetSize () > 0)
	{
		long nIconsWidth = rtIcons.left - rect.left;
		
		CPoint pt (rect.left + 1, rect.top + (rect.Height () - szImage.cy) / 2);

		if (bAlternative)
		{
			nIconsWidth += nTextWidth;

			if (nIconsWidth < rect.Width ())
			{
				pt.x += (rect.Width () - nIconsWidth) / 2;
			}
		}

		for (int i = 0; i < dwIcons.GetSize (); i++)
		{
			CBCGPPlannerManagerCtrl::DrawImageIcon (pDC, pt, dwIcons[i]);

			pt.x += szImage.cx;
		}

		rect.left = pt.x + 1;
	}

	if (bDrawText)
	{
		CRect rectText (rect);

		rectText.DeflateRect (4, 0, 1, 1);

		if (bAlternative && dwIcons.GetSize () == 0 && nTextWidth < rectText.Width ())
		{
			rectText.left += (rectText.Width () - nTextWidth) / 2;
		}

		pDC->DrawText (pApp->m_strDescription, rectText, 
			DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE);

		if (rectText.Width () < nTextWidth)
		{
			bToolTipNeeded = TRUE;
		}
	}
	else if (!pApp->m_strDescription.IsEmpty ())
	{
		bToolTipNeeded = TRUE;
	}

	pDC->SetTextColor (clrTextOld);

	if (bIsRoundedCorners)
	{
		pDC->SelectClipRgn (NULL, RGN_COPY);
	}

	pDS->SetToolTipNeeded (bToolTipNeeded);
}

HFONT CBCGPPlannerView::SetFont (HFONT hFont)
{
	HFONT hOldFont = NULL;

	if (m_Font.GetSafeHandle () != NULL)
	{
		hOldFont = (HFONT)m_Font.Detach ();
	}

	if (hFont != NULL)
	{
		m_Font.Attach (hFont);

		if (m_FontBold.GetSafeHandle () != NULL)
		{
			m_FontBold.DeleteObject ();
		}

		if (m_FontVert.GetSafeHandle () != NULL)
		{
			m_FontVert.DeleteObject ();
		}

		LOGFONT lf;
		m_Font.GetLogFont (&lf);

		LONG lfWeight = lf.lfWeight;

		lf.lfWeight = FW_BOLD;
		m_FontBold.CreateFontIndirect (&lf);

		lf.lfWeight      = lfWeight;
		lf.lfOrientation = 900;
		lf.lfEscapement  = lf.lfOrientation;
		m_FontVert.CreateFontIndirect (&lf);
	}

	if (hOldFont != hFont)
	{
		AdjustLayout ();
	}

	return hOldFont;
}

HFONT CBCGPPlannerView::GetFont (BOOL bBold/* = FALSE*/)
{
	if (bBold)
	{
		bBold = (m_pPlanner->GetDrawFlags () & BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_BOLD) ==
			BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_BOLD;
	}

	if (bBold)
	{
		return (HFONT) (m_FontBold.GetSafeHandle () != NULL 
							? m_FontBold.GetSafeHandle () 
							: globalData.fontBold);
	}

	return (HFONT) (m_Font.GetSafeHandle () != NULL 
						? m_Font.GetSafeHandle () 
						: globalData.fontRegular);
}

HFONT CBCGPPlannerView::GetFontVert ()
{
	return (HFONT) (m_FontVert.GetSafeHandle () != NULL 
						? m_FontVert.GetSafeHandle () 
						: globalData.fontVertCaption);
}

BOOL CBCGPPlannerView::OnLButtonDown(UINT nFlags, CPoint point)
{
	BCGP_PLANNER_HITTEST hit = HitTest (point);

	if (hit == BCGP_PLANNER_HITTEST_CLIENT ||
		hit == BCGP_PLANNER_HITTEST_DAY_CAPTION ||
		hit == BCGP_PLANNER_HITTEST_TIMEBAR ||
		hit >= BCGP_PLANNER_HITTEST_APPOINTMENT)
	{
		BOOL bRepaint = FALSE;

		BOOL bControl = (nFlags & MK_CONTROL) != 0 || (nFlags & MK_SHIFT) != 0;

		if (!bControl)
		{
			bRepaint = GetSelectedAppointments ().GetCount () > 0;

			ClearAppointmentSelection (FALSE);
		}

		if (hit >= BCGP_PLANNER_HITTEST_APPOINTMENT)
		{
			CBCGPAppointment* pApp = GetAppointmentFromPoint (point);

			SelectAppointment (pApp, !pApp->IsSelected (), FALSE);

			bRepaint = TRUE;
		}

		COleDateTime date (GetDateFromPoint (point));
		COleDateTime dt (date.GetYear(), date.GetMonth(), date.GetDay(), 0, 0, 0);

		UINT nResourceID = GetResourceFromPoint (point);

		if (dt != COleDateTime () && (dt != m_Date || 
			date != GetSelectionStart() || date != GetSelectionEnd() ||
			nResourceID != GetCurrentResourceID ()))
		{
			m_Date = dt;

			SetCurrentResourceID (nResourceID, FALSE, TRUE);

			if ((nFlags & MK_SHIFT) != 0 && hit < BCGP_PLANNER_HITTEST_APPOINTMENT)
			{
				SetSelection (m_Selection[0], date);
			}
			else
			{
				SetSelection (date, date);
			}
		}
		else
		{
			if (bRepaint)
			{
				GetPlanner()->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
		}
	}

	return TRUE;
}

BOOL CBCGPPlannerView::OnLButtonUp(UINT nFlags, CPoint point)
{
	BCGP_PLANNER_HITTEST hit = HitTest (point);

	if (hit >= BCGP_PLANNER_HITTEST_APPOINTMENT)
	{
		// only one left button must be pressed
		if (nFlags == 0 && !IsReadOnly ())
		{
			CBCGPAppointment* pApp = GetAppointmentFromPoint (point);

			if (pApp != NULL && 
				pApp->IsSelected () && 
				pApp->PointInRectEditHitTest (point))
			{
				pApp->SetupRectEdit (point);

				StartTimerEdit ();
			}
		}
	}

	return TRUE;
}

BOOL CBCGPPlannerView::OnLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/)
{
	StopTimerEdit ();

	return FALSE;
}

void CBCGPPlannerView::StartTimerEdit ()
{
	ASSERT_VALID (m_pPlanner);

	if (m_pPlanner == NULL || m_pPlanner->GetSafeHwnd () == NULL)
	{
		return;
	}

	if (m_TimerEdit == NULL)
	{
		m_TimerEdit = m_pPlanner->SetTimer (BCGP_PLANNER_TIMER_EDIT_EVENT, 
			::GetDoubleClickTime() / 3, NULL);
	}
}

void CBCGPPlannerView::StopTimerEdit ()
{
	ASSERT_VALID (m_pPlanner);

	if (m_pPlanner == NULL || m_pPlanner->GetSafeHwnd () == NULL)
	{
		return;
	}

	if (m_TimerEdit != NULL)
	{
		m_pPlanner->KillTimer (BCGP_PLANNER_TIMER_EDIT_EVENT);
		m_TimerEdit = NULL;
	}
}

BOOL CBCGPPlannerView::OnTimer(UINT_PTR nIDEvent)
{
	ASSERT_VALID (m_pPlanner);

	if (m_pPlanner == NULL || m_pPlanner->GetSafeHwnd () == NULL)
	{
		return FALSE;
	}

	if (nIDEvent == BCGP_PLANNER_TIMER_EDIT_EVENT && m_TimerEdit != NULL)
	{
		StopTimerEdit ();

		if (GetSelectedAppointments ().GetCount () != 1)
		{
			return TRUE;
		}

		CBCGPAppointment* pApp = GetSelectedAppointments ().GetHead ();

		if (pApp != NULL && pApp->IsSelected ())
		{
			StartEditAppointment (pApp);
		}

		return TRUE;
	}

	return FALSE;
}

BOOL CBCGPPlannerView::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	BCGP_PLANNER_HITTEST hit = HitTest (point);

	BOOL bRepaint = FALSE;

	if (hit >= BCGP_PLANNER_HITTEST_APPOINTMENT)
	{
		CBCGPAppointment* pApp = GetAppointmentFromPoint (point);
		ASSERT_VALID (pApp);

		if (!pApp->IsSelected ())
		{
			ClearAppointmentSelection (FALSE);

			CBCGPAppointment* pApp = GetAppointmentFromPoint (point);

			SelectAppointment (pApp, !pApp->IsSelected (), FALSE);

			bRepaint = TRUE;
		}

		hit = BCGP_PLANNER_HITTEST_CLIENT;
	}

	if (hit == BCGP_PLANNER_HITTEST_CLIENT || hit == BCGP_PLANNER_HITTEST_TIMEBAR)
	{
		COleDateTime date (GetDateFromPoint (point));
		COleDateTime dt (date.GetYear(), date.GetMonth(), date.GetDay(), 0, 0, 0);

		UINT nResourceID = GetResourceFromPoint (point);
		if (nResourceID == GetPlanner ()->GetCurrentResourceID ())
		{
			nResourceID = CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID;
		}

		if (dt != COleDateTime () && (dt != m_Date || 
			date != GetSelectionStart() || date != GetSelectionEnd() ||
			nResourceID != CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID))
		{
			m_Date = dt;

			if (nResourceID != CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID)
			{
				SetCurrentResourceID (nResourceID, FALSE, TRUE);
			}

			SetSelection (date, date, FALSE);

			bRepaint = TRUE;
		}
	}

	if (bRepaint)
	{
		GetPlanner()->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	return TRUE;
}

BOOL CBCGPPlannerView::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
	if (m_pCapturedAppointment != NULL)
	{
		return FALSE;
	}

#ifndef _BCGPCALENDAR_STANDALONE
	if (CBCGPPopupMenu::GetActiveMenu () != NULL)
	{
		return FALSE;
	}
#endif

	if (m_nScrollTotal > 0)
	{
		const int nSteps = abs(zDelta) / WHEEL_DELTA;

		for (int i = 0; i < nSteps; i++)
		{
			OnVScroll (zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0, NULL);
		}
	}

	return TRUE;
}

BOOL CBCGPPlannerView::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	if (IsCaptured ())
	{
		BCGP_PLANNER_HITTEST hit = HitTestArea (point);

		if (hit != BCGP_PLANNER_HITTEST_NOWHERE &&
			hit != BCGP_PLANNER_HITTEST_HEADER)
		{
			COleDateTime date = GetDateFromPoint (point);

			m_htCaptureAreaCurrent = hit;

			if (date != COleDateTime () && date != m_dtCaptureCurrent)
			{
				m_dtCaptureCurrent = date;

				if (!IsDragDrop () && 
					m_hitCapturedAppointment == BCGP_PLANNER_HITTEST_NOWHERE)
				{
					SetSelection (m_Selection [0], m_dtCaptureCurrent);
				}
				else if (m_pCapturedAppointment != NULL)
				{
					if (m_hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_LEFT ||
						m_hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_RIGHT)
					{
						date = COleDateTime (date.GetYear (), date.GetMonth (), date.GetDay (),
							0, 0, 0);
					}

					COleDateTime dtS (m_pCapturedAppointment->GetStart ());
					COleDateTime dtF (m_pCapturedAppointment->GetFinish ());

					if (m_hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_TOP ||
						m_hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_LEFT)
					{
						if (dtS != date)
						{
							dtS = COleDateTime (date.GetYear (), date.GetMonth (), date.GetDay (),
								date.GetHour (), date.GetMinute (), 0);

							if (dtF < dtS)
							{
								dtS = dtF;
							}
						}
					}
					else if (m_hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_BOTTOM ||
						     m_hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_RIGHT)
					{
						COleDateTime dt (date);
						if (m_hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_RIGHT &&
							!m_pCapturedAppointment->IsAllDay ())
						{
							dt += COleDateTimeSpan (1, 0, 0, 0);
						}
						else if (m_hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_BOTTOM)
						{
							dt += GetMinimumSpan ();
						}

						if (dtF != dt)
						{
							dtF = COleDateTime (dt.GetYear (), dt.GetMonth (), dt.GetDay (),
								dt.GetHour (), dt.GetMinute (), 0);

							if (dtF < dtS)
							{
								dtF = dtS;
							}
						}
					}

					if (m_pCapturedAppointment->GetStart () != dtS ||
						m_pCapturedAppointment->GetFinish () != dtF)
					{
						m_bCapturedAppointmentChanged = TRUE;

						BOOL bWasMulti = m_pCapturedAppointment->IsMultiDay ();
						
						m_pCapturedAppointment->SetInterval (dtS, dtF);

						{
							// need resort appointments
							XBCGPAppointmentArray& ar = GetQueryedAppointments ();

							SortAppointments (ar, (int) ar.GetSize ());
						}

						if (bWasMulti != m_pCapturedAppointment->IsMultiDay () || bWasMulti)
						{
							GetPlanner ()->AdjustLayout (FALSE);
							date = GetDateFromPoint (point);

							if (date != COleDateTime())
							{
								m_dtCaptureCurrent = date;
							}
						}
						else
						{
							AdjustAppointments ();
						}

						GetPlanner ()->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
					}
				}
			}
		}
	}

	return TRUE;
}

BOOL CBCGPPlannerView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	ASSERT_VALID(m_pPlanner);

	if (IsReadOnly ())
	{
		return FALSE;
	}

	CPoint pt;
	::GetCursorPos (&pt);
	ScreenToClient (&pt);

	BCGP_PLANNER_HITTEST hit = HitTest (pt);

	if (hit >= BCGP_PLANNER_HITTEST_APPOINTMENT)
	{
		if (hit == BCGP_PLANNER_HITTEST_APPOINTMENT_MOVE)
		{
			if (globalData.m_hcurSizeAll == NULL)
			{
				globalData.m_hcurSizeAll = AfxGetApp ()->LoadStandardCursor (IDC_SIZEALL);
			}

			::SetCursor (globalData.m_hcurSizeAll);
			
			return TRUE;
		}
		else if ((hit == BCGP_PLANNER_HITTEST_APPOINTMENT_TOP || 
			      hit == BCGP_PLANNER_HITTEST_APPOINTMENT_BOTTOM) &&
			m_hcurAppVert != NULL && CanCaptureAppointment (hit))
		{
			::SetCursor (m_hcurAppVert);
		
			return TRUE;
		}
		else if ((hit == BCGP_PLANNER_HITTEST_APPOINTMENT_LEFT || 
			      hit == BCGP_PLANNER_HITTEST_APPOINTMENT_RIGHT) &&
			m_hcurAppHorz != NULL && CanCaptureAppointment (hit))
		{
			::SetCursor (m_hcurAppHorz);
		
			return TRUE;
		}
	}

	return FALSE;
}

void CBCGPPlannerView::ScreenToClient (LPPOINT point) const
{
	ASSERT_VALID(m_pPlanner);

	m_pPlanner->ScreenToClient (point);
}

void CBCGPPlannerView::ScreenToClient (LPRECT rect) const
{
	ASSERT_VALID(m_pPlanner);

	m_pPlanner->ScreenToClient (rect);
}

void CBCGPPlannerView::ClientToScreen (LPPOINT point) const
{
	ASSERT_VALID(m_pPlanner);

	m_pPlanner->ClientToScreen (point);
}

void CBCGPPlannerView::ClientToScreen (LPRECT rect) const
{
	ASSERT_VALID(m_pPlanner);

	m_pPlanner->ClientToScreen (rect);
}

void CBCGPPlannerView::GetDragScrollRect (CRect& rect)
{
	rect = m_rectApps;
}

BOOL CBCGPPlannerView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
	if (HIBYTE(nScrollCode) == (BYTE)-1)
	{
		return FALSE;
	}

	if (!bDoScroll)
	{
		return m_nScrollTotal != 0;
	}
	else
	{
		OnVScroll (HIBYTE(nScrollCode), nPos, NULL);
	}

	return TRUE;
}

BOOL CBCGPPlannerView::OnVScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* pScrollBar)
{
	ASSERT_VALID (m_pPlanner);

	StopEditAppointment ();

	if (pScrollBar == m_pPlanner->GetHeaderScrollBarCtrl (SB_VERT))
	{
		pScrollBar->SetScrollPos (m_nHeaderScrollOffset, TRUE);
	}
	else
	{
		m_pPlanner->SetScrollPos (SB_VERT, m_nScrollOffset, TRUE);
	}

	if (m_arDragAppointments.GetSize () > 0)
	{
		CPoint pt;
		::GetCursorPos (&pt);
		ScreenToClient (&pt);

		COleDateTime dtCaptureStart (m_dtCaptureCurrent);
		m_dtCaptureCurrent = GetDateFromPoint (pt);

		BOOL bAdd =  m_dtCaptureCurrent > dtCaptureStart;

		COleDateTimeSpan spanTo;

		if (bAdd)
		{
			spanTo = m_dtCaptureCurrent - dtCaptureStart;
		}
		else
		{
			spanTo = dtCaptureStart - m_dtCaptureCurrent;
		}

		CBCGPPlannerManagerCtrl::MoveAppointments (m_arDragAppointments, spanTo, bAdd);
	}

	AdjustLayout ();

	return TRUE;
}

void CBCGPPlannerView::ClearDragedAppointments ()
{
	if (m_pDragAppointments)
	{
		delete m_pDragAppointments;
		m_pDragAppointments = NULL;
	}

	m_arDragAppointments.RemoveAll ();
}

DROPEFFECT CBCGPPlannerView::OnDragScroll(DWORD /*dwKeyState*/, CPoint /*point*/)
{
	return DROPEFFECT_NONE;
}

DROPEFFECT CBCGPPlannerView::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	ASSERT_VALID (m_pPlanner);

	ASSERT (pDataObject != NULL);
	ASSERT (pDataObject->IsDataAvailable (CBCGPPlannerManagerCtrl::GetClipboardFormat ()));

	DROPEFFECT dragEffect = HitTestDrag (dwKeyState, point);

	BOOL bDateChanged       = FALSE;
	BOOL bDragEffectChanged = GetDragEffect () != dragEffect;
	BOOL bDragMatchOld      = IsCaptureMatched ();

	m_AdjustAction = BCGP_PLANNER_ADJUST_ACTION_NONE;

	if (dragEffect != DROPEFFECT_NONE)
	{
		COleDateTime dtCur  = GetDateFromPoint (point);

		bDateChanged = dtCur != COleDateTime () && dtCur != m_dtCaptureCurrent;

		if (bDateChanged)
		{
			m_dtCaptureCurrent = dtCur;
		}
	}

	BOOL bChanged = bDragEffectChanged || bDateChanged;

	if (bChanged)
	{
		if (m_pDragAppointments != NULL)
		{
			m_pDragAppointments->RemoveAll ();
			m_arDragAppointments.RemoveAll ();
		}
		else
		{
			m_pDragAppointments = 
				(CBCGPAppointmentStorage*)RUNTIME_CLASS(CBCGPAppointmentStorage)->CreateObject ();
		}

		if (dragEffect == DROPEFFECT_NONE)
		{
			m_AdjustAction = BCGP_PLANNER_ADJUST_ACTION_APPOINTMENTS;
			return dragEffect;
		}

		CFile* pFile = pDataObject->GetFileData (CBCGPPlannerManagerCtrl::GetClipboardFormat ());
		if (pFile == NULL)
		{
			return FALSE;
		}

		XBCGPAppointmentArray ar;

		BOOL bRes = CBCGPPlannerManagerCtrl::SerializeFrom (*pFile, ar, 
			m_pPlanner->GetType (), 
			IsDragDrop () ? COleDateTime () : m_dtCaptureCurrent);

		delete pFile;

		if (bRes)
		{
			if (IsDragDrop ())
			{
				COleDateTimeSpan spanTo (0, 0, 0, 0);

				BOOL bAdd = m_dtCaptureCurrent > m_dtCaptureStart;

				if (bAdd)
				{
					spanTo = m_dtCaptureCurrent - m_dtCaptureStart;
				}
				else
				{
					spanTo = m_dtCaptureStart - m_dtCaptureCurrent;
				}

				CBCGPPlannerManagerCtrl::MoveAppointments (ar, spanTo, bAdd);
			}

			for (int i = 0; i < ar.GetSize (); i++)
			{
				ar[i]->SetSelected (TRUE);

				m_pDragAppointments->Add (ar[i]);
			}

			m_pDragAppointments->Query (m_arDragAppointments, m_DateStart, m_DateEnd);

			DROPEFFECT dragEffectNew = CanDropAppointmets(dwKeyState, point);
			if (dragEffectNew != dragEffect)
			{
				dragEffect = dragEffectNew;

				if (dragEffect == DROPEFFECT_NONE)
				{
					m_pDragAppointments->RemoveAll();
					m_arDragAppointments.RemoveAll();

					m_AdjustAction = BCGP_PLANNER_ADJUST_ACTION_APPOINTMENTS;
					return dragEffect;
				}
			}
		}
	}

	if (bChanged || bDragMatchOld != IsCaptureMatched ())
	{
		m_AdjustAction = BCGP_PLANNER_ADJUST_ACTION_APPOINTMENTS;
	}

	return dragEffect;
}

CBCGPPlannerView::BCGP_PLANNER_HITTEST
CBCGPPlannerView::HitTest (const CPoint& point) const
{
	BCGP_PLANNER_HITTEST hit = HitTestArea (point);

	if (hit == BCGP_PLANNER_HITTEST_CLIENT ||
		hit == BCGP_PLANNER_HITTEST_NOWHERE)
	{
		BCGP_PLANNER_HITTEST hitA = HitTestAppointment (point);

		if (hitA != BCGP_PLANNER_HITTEST_NOWHERE)
		{
			hit = hitA;
		}
	}

	return hit;
}

CBCGPPlannerView::BCGP_PLANNER_HITTEST
CBCGPPlannerView::HitTestArea (const CPoint& point) const
{
	BCGP_PLANNER_HITTEST hit = BCGP_PLANNER_HITTEST_NOWHERE;

	if (m_rectApps.PtInRect (point))
	{
		hit = BCGP_PLANNER_HITTEST_CLIENT;

		if ((GetPlanner()->GetDrawFlags() & BCGP_PLANNER_DRAW_VIEW_NO_UPDOWN) == 0)
		{
			for (int i = 0; i < m_UpRects.GetSize (); i++)
			{
				if (m_UpRects[i].PtInRect (point))
				{
					hit = BCGP_PLANNER_HITTEST_ICON_UP;
					break;
				}
			}

			if (hit != BCGP_PLANNER_HITTEST_ICON_UP)
			{
				for (int i = 0; i < m_DownRects.GetSize (); i++)
				{
					if (m_DownRects[i].PtInRect (point))
					{
						hit = BCGP_PLANNER_HITTEST_ICON_DOWN;
						break;
					}
				}
			}
		}
	}

	return hit;
}

CBCGPPlannerView::BCGP_PLANNER_HITTEST
CBCGPPlannerView::HitTestAppointment (const CPoint& point) const
{
	BCGP_PLANNER_HITTEST hit = BCGP_PLANNER_HITTEST_NOWHERE;

	CBCGPAppointment* pApp = ((CBCGPPlannerView*) this)->GetAppointmentFromPoint (point);

	if (pApp != NULL)
	{
		switch(pApp->HitTest (point))
		{
		case CBCGPAppointment::BCGP_APPOINTMENT_HITTEST_INSIDE:
			hit = BCGP_PLANNER_HITTEST_APPOINTMENT;
			break;
		case CBCGPAppointment::BCGP_APPOINTMENT_HITTEST_MOVE:
			hit = BCGP_PLANNER_HITTEST_APPOINTMENT_MOVE;
			break;
		case CBCGPAppointment::BCGP_APPOINTMENT_HITTEST_TOP:
			hit = BCGP_PLANNER_HITTEST_APPOINTMENT_TOP;
			break;
		case CBCGPAppointment::BCGP_APPOINTMENT_HITTEST_BOTTOM:
			hit = BCGP_PLANNER_HITTEST_APPOINTMENT_BOTTOM;
			break;
		case CBCGPAppointment::BCGP_APPOINTMENT_HITTEST_LEFT:
			hit = BCGP_PLANNER_HITTEST_APPOINTMENT_LEFT;
			break;
		case CBCGPAppointment::BCGP_APPOINTMENT_HITTEST_RIGHT:
			hit = BCGP_PLANNER_HITTEST_APPOINTMENT_RIGHT;
			break;
		default:
			ASSERT(FALSE);
		}
	}

	return hit;
}

DROPEFFECT CBCGPPlannerView::HitTestDrag (DWORD dwKeyState, const CPoint& point) const
{
	BCGP_PLANNER_HITTEST hit = HitTestArea (point);

	if (hit == BCGP_PLANNER_HITTEST_NOWHERE)
	{
		return DROPEFFECT_NONE;
	}

	return (dwKeyState & MK_CONTROL) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
}

DROPEFFECT CBCGPPlannerView::CanDropAppointmets (DWORD dwKeyState, const CPoint& /*point*/) const
{
	return (dwKeyState & MK_CONTROL) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
}

void CBCGPPlannerView::OnActivate(CBCGPPlannerManagerCtrl* pPlanner, const CBCGPPlannerView* /*pOldView*/)
{
	if (m_bActive)
	{
		return;
	}

	ASSERT_VALID(pPlanner);

	m_pPlanner = pPlanner;

	m_bActive = TRUE;

	m_nHeaderScrollPage   = 0;
	m_nHeaderScrollTotal  = 0;
	m_nHeaderScrollOffset = 0;

	m_nScrollPage   = 0;
	m_nScrollTotal  = 0;
	m_nScrollOffset = 0;

	QueryAppointments ();

	if (m_pPlanner->GetSafeHwnd ())
	{
		OnUpdateStorage ();
		AdjustLayout ();
	}
}

void CBCGPPlannerView::StartEditAppointment (CBCGPAppointment* pApp)
{
	if (pApp == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pPlanner);

	if (m_pPlanner == NULL || m_pPlanner->GetSafeHwnd () == NULL)
	{
		return;
	}

	ASSERT_VALID(pApp);

	if (m_rectApps.top <= pApp->GetRectEdit ().top)
	{
		pApp->OnEdit (GetPlanner ());
	}
}

void CBCGPPlannerView::StopEditAppointment ()
{
	if (m_pPlanner->GetSafeHwnd () == NULL ||
		m_pPlanner->GetDlgItem (CBCGPPlannerManagerCtrl::BCGP_PLANNER_ID_INPLACE) == NULL)
	{
		return;
	}

	XBCGPAppointmentArray& arApps = GetQueryedAppointments ();

	for (int i = 0; i < arApps.GetSize (); i++)
	{
		CBCGPAppointment* pApp = arApps[i];

		if (pApp != NULL && pApp->IsEditExists ())
		{
			pApp->OnEndEdit (FALSE);
		}
	}
}

void CBCGPPlannerView::OnDateChanged ()
{
	ASSERT_VALID(m_pPlanner);

	m_pPlanner->OnDateChanged ();
}

void CBCGPPlannerView::OnDeactivate(CBCGPPlannerManagerCtrl* pPlanner)
{
	if (m_bActive)
	{
		ASSERT_VALID(pPlanner);
		UNREFERENCED_PARAMETER(pPlanner);

		StopEditAppointment ();

		m_bActive = FALSE;

		m_pPlanner = NULL;
	}
}

CSize CBCGPPlannerView::GetClockIconSize () const
{
	ASSERT_VALID(m_pPlanner);

	return m_pPlanner->GetClockIconSize ();
}

void CBCGPPlannerView::DrawClockIcon (CDC* pDC, const CPoint& point, 
									  const COleDateTime& time) const
{
	ASSERT_VALID(m_pPlanner);

	m_pPlanner->DrawClockIcon (pDC, point, time);
}

void CBCGPPlannerView::QueryAppointments ()
{
	ASSERT_VALID(m_pPlanner);

	StopEditAppointment ();

	m_pPlanner->QueryAppointments ();
}

BOOL CBCGPPlannerView::IsReadOnly () const
{
	ASSERT_VALID(m_pPlanner);

	return m_pPlanner->IsReadOnly ();
}

XBCGPAppointmentArray& CBCGPPlannerView::GetQueryedAppointments ()
{
	ASSERT_VALID(m_pPlanner);

	return m_pPlanner->GetQueryedAppointments ();
}

const XBCGPAppointmentArray& CBCGPPlannerView::GetQueryedAppointments () const
{
	ASSERT_VALID(m_pPlanner);

	return m_pPlanner->GetQueryedAppointments ();
}

XBCGPAppointmentList& CBCGPPlannerView::GetSelectedAppointments ()
{
	ASSERT_VALID(m_pPlanner);

	return m_pPlanner->GetSelectedAppointments ();
}

const XBCGPAppointmentList& CBCGPPlannerView::GetSelectedAppointments () const
{
	ASSERT_VALID(m_pPlanner);

	return m_pPlanner->GetSelectedAppointments ();
}

void CBCGPPlannerView::ClearAppointmentSelection (BOOL bRedraw)
{
	ASSERT_VALID(m_pPlanner);

	m_pPlanner->ClearAppointmentSelection (bRedraw);
}

void CBCGPPlannerView::SelectAppointment (CBCGPAppointment* pApp, 
										  BOOL bSelect, BOOL bRedraw)
{
	ASSERT_VALID(m_pPlanner);

	m_pPlanner->SelectAppointment (pApp, bSelect, bRedraw);
}

BOOL CBCGPPlannerView::IsDragDrop () const
{
	ASSERT_VALID(m_pPlanner);

	return m_pPlanner->IsDragDrop ();
}

BOOL CBCGPPlannerView::CanStartDragDrop () const
{
	BCGP_PLANNER_HITTEST hitCapturedAppointment = m_hitCapturedAppointment;

	if (m_pCapturedAppointment == NULL)
	{
		CPoint point;
		::GetCursorPos (&point);
		ScreenToClient (&point);

		hitCapturedAppointment = HitTestAppointment (point);
	}

	return hitCapturedAppointment >= BCGP_PLANNER_HITTEST_APPOINTMENT && 
		!CanCaptureAppointment (hitCapturedAppointment);
}

BOOL CBCGPPlannerView::CanCaptureAppointment (BCGP_PLANNER_HITTEST hitCapturedAppointment) const
{
	if (IsReadOnly ())
	{
		return FALSE;
	}

	return  hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_TOP ||
		    hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_BOTTOM ||
		    hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_LEFT ||
		    hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_RIGHT;
}

COleDateTimeSpan CBCGPPlannerView::GetMinimumSpan () const
{
	return COleDateTimeSpan (1, 0, 0, 0);
}

BOOL CBCGPPlannerView::IsCaptured () const
{
	ASSERT_VALID(m_pPlanner);

	return m_pPlanner->IsCaptured ();
}

void CBCGPPlannerView::StartCapture ()
{
	ASSERT_VALID(m_pPlanner);

	CPoint pt;
	::GetCursorPos (&pt);
	ScreenToClient (&pt);

	m_dtCaptureStart   = GetDateFromPoint(pt);
	m_dtCaptureCurrent = m_dtCaptureStart;

	m_AdjustAction     = BCGP_PLANNER_ADJUST_ACTION_NONE;

	m_pCapturedAppointment        = NULL;
	BCGP_PLANNER_HITTEST hitCapturedAppointment = HitTest (pt);
	m_bCapturedAppointmentChanged = FALSE;

	if (CanCaptureAppointment (hitCapturedAppointment))
	{
		m_hitCapturedAppointment = hitCapturedAppointment;
		m_pCapturedAppointment = GetAppointmentFromPoint (pt);

		if (m_pCapturedAppointment != NULL)
		{
			m_CapturedProperties.RemoveAll ();
			m_CapturedProperties.Add 
				(
					BCGP_APPOINTMENT_PROPERTY_DATE_START, 
					new CBCGPAppointmentProperty (m_pCapturedAppointment->GetStart ())
				);
			m_CapturedProperties.Add
				(
					BCGP_APPOINTMENT_PROPERTY_DATE_FINISH, 
					new CBCGPAppointmentProperty (m_pCapturedAppointment->GetFinish ())
				);
		}

		m_dtCapturedAppointment = m_pCapturedAppointment->GetStart ();
	}

	m_htCaptureAreaStart   = HitTestArea (pt);
	m_htCaptureAreaCurrent = m_htCaptureAreaStart;

	m_htCaptureResourceStart = GetResourceFromPoint (pt);
	m_htCaptureResourceCurrent = m_htCaptureResourceStart;
}

void CBCGPPlannerView::StopCapture ()
{
	ASSERT_VALID(m_pPlanner);

	m_dtCaptureStart   = COleDateTime ();
	m_dtCaptureCurrent = m_dtCaptureStart;

	m_AdjustAction     = BCGP_PLANNER_ADJUST_ACTION_NONE;

	m_htCaptureAreaStart   = BCGP_PLANNER_HITTEST_NOWHERE;
	m_htCaptureAreaCurrent = m_htCaptureAreaStart;

	m_htCaptureResourceStart = CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID;
	m_htCaptureResourceCurrent = m_htCaptureResourceStart;

	if (m_pCapturedAppointment != NULL && m_bCapturedAppointmentChanged)
	{
		BOOL bCapturedAppointmentChanged = FALSE;

		{
			COleDateTime dtStart;
			COleDateTime dtFinish;

			if (m_CapturedProperties.PropertyExists (BCGP_APPOINTMENT_PROPERTY_DATE_START))
			{
				dtStart = *((CBCGPAppointmentProperty*)m_CapturedProperties.Get (BCGP_APPOINTMENT_PROPERTY_DATE_START));
			}

			if (m_CapturedProperties.PropertyExists (BCGP_APPOINTMENT_PROPERTY_DATE_FINISH))
			{
				dtFinish = *((CBCGPAppointmentProperty*)m_CapturedProperties.Get (BCGP_APPOINTMENT_PROPERTY_DATE_FINISH));
			}

			bCapturedAppointmentChanged = 
				m_pCapturedAppointment->GetStart () != dtStart ||
				m_pCapturedAppointment->GetFinish () != dtFinish;
		}

		if (bCapturedAppointmentChanged)
		{
			BOOL bECP = m_pCapturedAppointment->m_RecurrenceEcp;

			if (m_pCapturedAppointment->IsRecurrenceClone ())
			{
				m_pCapturedAppointment->m_RecurrenceEcp = TRUE;
			}

			if (!GetPlanner ()->UpdateAppointment (m_pCapturedAppointment, 
					m_dtCapturedAppointment, FALSE, FALSE))
			{
				if (m_pCapturedAppointment->IsRecurrenceClone ())
				{
					m_pCapturedAppointment->m_RecurrenceEcp = bECP;
				}

				RestoreCapturedAppointment ();
			}
		}

		m_CapturedProperties.RemoveAll ();

		if (m_pCapturedAppointment != NULL)
		{
			COleDateTime dtSel1 (m_pCapturedAppointment->GetStart ());
			COleDateTime dtSel2 (m_pCapturedAppointment->GetFinish ());	

			if (dtSel1 == dtSel2)
			{
				dtSel2 += GetMinimumSpan ();
			}

			dtSel2 -= COleDateTimeSpan (0, 0, 0, 1);

			SetSelection (dtSel1, dtSel2, FALSE);
		}

		AdjustAppointments ();

		//SelectAppointment (m_pCapturedAppointment, TRUE, FALSE);

		GetPlanner ()->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

		GetPlanner ()->UpdateCalendarsState ();
	}

	m_pCapturedAppointment        = NULL;
	m_hitCapturedAppointment      = BCGP_PLANNER_HITTEST_NOWHERE;
	m_dtCapturedAppointment       = COleDateTime ();
	m_bCapturedAppointmentChanged = FALSE;
}

void CBCGPPlannerView::RestoreCapturedAppointment ()
{
	if (m_pCapturedAppointment != NULL && m_bCapturedAppointmentChanged)
	{
		COleDateTime dtStart;
		COleDateTime dtFinish;

		if (m_CapturedProperties.PropertyExists (BCGP_APPOINTMENT_PROPERTY_DATE_START))
		{
			dtStart = *((CBCGPAppointmentProperty*)m_CapturedProperties.Get (BCGP_APPOINTMENT_PROPERTY_DATE_START));
		}

		if (m_CapturedProperties.PropertyExists (BCGP_APPOINTMENT_PROPERTY_DATE_FINISH))
		{
			dtFinish = *((CBCGPAppointmentProperty*)m_CapturedProperties.Get (BCGP_APPOINTMENT_PROPERTY_DATE_FINISH));
		}


		ASSERT (dtStart != COleDateTime ());
		ASSERT (dtFinish != COleDateTime ());

		m_pCapturedAppointment->SetInterval (dtStart, dtFinish);
	}
}

DROPEFFECT CBCGPPlannerView::GetDragEffect () const
{
	ASSERT_VALID(m_pPlanner);

	return m_pPlanner->GetDragEffect ();
}

BOOL CBCGPPlannerView::IsCaptureMatched () const
{
	return IsCaptureDatesMatched () && IsCaptureAreasMatched () && IsCaptureResourcesMatched ();
}

void CBCGPPlannerView::SetDate (const COleDateTime& date)
{
	COleDateTime dt (date.GetYear (), date.GetMonth (), date.GetDay (), 0, 0, 0);

	m_Date = dt;

	QueryAppointments ();

	SetSelection (m_Date, m_Date, FALSE);

	AdjustLayout ();
}

void CBCGPPlannerView::SetSelection (const COleDateTime& sel1, const COleDateTime& sel2, BOOL bRedraw)
{
	//ASSERT_VALID(m_pPlanner);

	COleDateTime s1 (sel1);
	COleDateTime s2 (sel2);

	if (s1 < m_DateStart)
	{
		s1 = m_DateStart;
	}
	else if (s1 > m_DateEnd)
	{
		s1 = m_DateEnd;
	}

	if (s2 < m_DateStart)
	{
		s2 = m_DateStart;
	}
	else if (s2 > m_DateEnd)
	{
		s2 = m_DateEnd;
	}
/*
	BOOL bChanged = m_Selection [0] != s1 || m_Selection [1] != s2;

	if (bChanged)
	{
		m_Selection [0] = s1;
		m_Selection [1] = s2;
	}

	bRedraw = bRedraw && bChanged;
*/
	if (m_Selection [0] != s1 || m_Selection [1] != s2)
	{
		m_Selection [0] = s1;
		m_Selection [1] = s2;
	}

	if (bRedraw && 
		m_pPlanner != NULL && m_pPlanner->GetSafeHwnd () != NULL && m_pPlanner->IsWindowVisible ())
	{
		m_pPlanner->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

COLORREF CBCGPPlannerView::GetHourLineColor (BOOL bWorkingHours, BOOL bHour)
{
	return visualManager->GetPlannerHourLineColor (this, bWorkingHours, bHour);
}

void CBCGPPlannerView::OnFillPlanner (CDC* pDC, CRect rect, BOOL bWorkingArea)
{
	visualManager->OnFillPlanner (pDC, this, rect, bWorkingArea);
}

COLORREF CBCGPPlannerView::OnFillPlannerCaption (CDC* pDC, 
		CRect rect, BOOL bIsToday, BOOL bIsSelected, BOOL bNoBorder/* = FALSE*/)
{
	return visualManager->
		OnFillPlannerCaption (pDC, this, rect, bIsToday, bIsSelected, bNoBorder);
}

void CBCGPPlannerView::InitToolTipInfo ()
{
	ASSERT_VALID (m_pPlanner);

	if (!m_bUpdateToolTipInfo)
	{
		return;
	}

	m_pPlanner->InitToolTipInfo ();

	m_bUpdateToolTipInfo = FALSE;
}

void CBCGPPlannerView::InitViewToolTipInfo ()
{

}

void CBCGPPlannerView::ClearToolTipInfo ()
{
	ASSERT_VALID(m_pPlanner);

	m_pPlanner->ClearToolTipInfo ();
}

void CBCGPPlannerView::AddToolTipInfo (const CRect& rect)
{
	ASSERT_VALID(m_pPlanner);

	m_pPlanner->AddToolTipInfo (rect);
}

CString CBCGPPlannerView::GetToolTipText (const CPoint& point)
{
	const CBCGPAppointment* pApp = GetAppointmentFromPoint (point);

	if (pApp == NULL)
	{
		return CString ();
	}

	return pApp->GetToolTipText ();
}

UINT CBCGPPlannerView::GetCurrentResourceID () const
{
	return CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID;
}

BOOL CBCGPPlannerView::SetCurrentResourceID (UINT /*nResourceID*/, 
											 BOOL /*bRedraw*//* = TRUE*/, BOOL /*bNotify*//* = FALSE*/)
{
	return FALSE;
}

UINT CBCGPPlannerView::GetResourceFromPoint (const CPoint& /*point*/) const
{
	return GetPlanner ()->GetCurrentResourceID ();
}

CBCGPPlannerView::BCGP_PLANNER_WORKING_STATUS
CBCGPPlannerView::GetWorkingPeriodParameters (int ResourceId, const COleDateTime& dtStart, const COleDateTime& dtEnd, XBCGPPlannerWorkingParameters& parameters) const
{
	parameters.m_clrWorking = CLR_DEFAULT;
	parameters.m_clrNonWorking = CLR_DEFAULT;

	return GetPlanner ()->GetWorkingPeriodParameters (ResourceId, dtStart, dtEnd, parameters);
}

CString CBCGPPlannerView::GetAccName() const
{
	return CString();
}

CString CBCGPPlannerView::GetAccIntervalFormattedString(const COleDateTime& date1, const COleDateTime& date2)
{
	CString strFmt;
	CString strDay;

	if (CBCGPPlannerView::IsDateBeforeMonth ())
	{
		if (CBCGPPlannerView::IsDayLZero ())
		{
			strFmt = _T("dd MMM");
		}
		else
		{
			strFmt = _T("d MMM");
		}
	}
	else
	{
		if (CBCGPPlannerView::IsDayLZero ())
		{
			strFmt = _T("MMM dd");
		}
		else
		{
			strFmt = _T("MMM d");
		}
	}

	strFmt += _T("   ");

	SYSTEMTIME st;
	date1.GetAsSystemTime (st);
	::GetDateFormat
		(
			LOCALE_USER_DEFAULT,
			0,
			&st,
			strFmt,
			strDay.GetBuffer (100),
			100
		);
	strDay.ReleaseBuffer ();

	CString strStart(strDay + CBCGPAppointment::GetFormattedTimeString (date1, TIME_NOSECONDS, FALSE));
	CString strEnd(CBCGPAppointment::GetFormattedTimeString (date2, TIME_NOSECONDS, FALSE));

	if ((date2 - date1).GetDays () != 0)
	{
		SYSTEMTIME st;
		date2.GetAsSystemTime (st);
		::GetDateFormat
			(
				LOCALE_USER_DEFAULT,
				0,
				&st,
				strFmt,
				strDay.GetBuffer (100),
				100
			);
		strDay.ReleaseBuffer ();

		strEnd = strDay + strEnd;
	}

	CString strValue(strStart + _T(" - ") + strEnd);

	return strValue;
}

CString CBCGPPlannerView::GetAccValue() const
{
	COleDateTime dtStart (GetSelectionStart ());
	COleDateTime dtEnd (GetSelectionEnd ());

	if (dtEnd < dtStart)
	{
		COleDateTime dtTemp(dtStart);
		dtStart = dtEnd;
		dtEnd = dtTemp;
	}

	dtEnd += COleDateTimeSpan(0, 0, 0, 1);

	return CBCGPPlannerView::GetAccIntervalFormattedString (dtStart, dtEnd);
}

CString CBCGPPlannerView::GetAccDescription() const
{
	return CString();
}

#endif // BCGP_EXCLUDE_PLANNER
