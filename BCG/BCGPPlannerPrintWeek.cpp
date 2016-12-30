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
// BCGPPlannerPrintWeek.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPPlannerPrintWeek.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerManagerCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerPrintWeek

IMPLEMENT_DYNCREATE(CBCGPPlannerPrintWeek, CBCGPPlannerPrint)

CBCGPPlannerPrintWeek::CBCGPPlannerPrintWeek()
	: CBCGPPlannerPrint  ()
	, m_bDrawTimeFinish  (TRUE)
{
}

CBCGPPlannerPrintWeek::~CBCGPPlannerPrintWeek()
{
}

#ifdef _DEBUG
void CBCGPPlannerPrintWeek::AssertValid() const
{
	CObject::AssertValid();
}

void CBCGPPlannerPrintWeek::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif

CString CBCGPPlannerPrintWeek::GetPageHeaderText () const
{
	CString strText;

	SYSTEMTIME st;
	CString str;

	COleDateTime dtStart (GetDateStart ());
	COleDateTime dtEnd (GetDateEnd ());

	CString strFormat (_T("d MMMM yyyy"));
	if (!CBCGPPlannerView::IsDateBeforeMonth ())
	{
		strFormat = _T("MMMM d yyyy");
	}

	dtStart.GetAsSystemTime (st);
	::GetDateFormat
		(
			LOCALE_USER_DEFAULT,
			0,
			&st,
			strFormat,
			str.GetBuffer (100),
			100
		);
	str.ReleaseBuffer ();

	strText = str + _T(" -\r");

	dtEnd.GetAsSystemTime (st);
	::GetDateFormat
		(
			LOCALE_USER_DEFAULT,
			0,
			&st,
			strFormat,
			str.GetBuffer (100),
			100
		);
	str.ReleaseBuffer ();

	strText += str;

	return strText;
}

void CBCGPPlannerPrintWeek::GetCaptionFormatStrings (CStringArray& sa)
{
	sa.RemoveAll ();

	BOOL bCompact = (GetDrawFlags () & BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_COMPACT) ==
			BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_COMPACT;

	if (!bCompact)
	{
		if (CBCGPPlannerView::IsDateBeforeMonth ())
		{
			sa.Add (_T("dddd d MMMM"));
			sa.Add (_T("dddd d MMM"));
			sa.Add (_T("ddd d MMM"));
			sa.Add (_T("ddd d"));
		}
		else
		{
			sa.Add (_T("dddd MMMM d"));
			sa.Add (_T("dddd MMM d"));
			sa.Add (_T("ddd MMM d"));
			sa.Add (_T("ddd d"));
		}
	}
	else
	{
		sa.Add (_T("d\ndddd"));
		sa.Add (_T("d\nddd"));
		sa.Add (_T("d"));
	}
}

void CBCGPPlannerPrintWeek::AdjustLayout (CDC* /*pDC*/, const CRect& /*rectClient*/)
{
}

void CBCGPPlannerPrintWeek::AdjustRects ()
{
	const int nWidth2    = m_rectApps.Width () / 2;
	const int nHeight3   = m_rectApps.Height () / 3;

	CRect rect (m_rectApps);
	rect.right  = rect.left + nWidth2;
	rect.bottom = rect.top + nHeight3;

	const int nDayStart = CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () == 0
		? 1
		: CBCGPPlannerManagerCtrl::GetFirstDayOfWeek ();

	for (int nDay = 0; nDay < 6; nDay++)
	{
		int nWeekDay = nDayStart + nDay - 7;

		if (nDay == 2)
		{
			rect.bottom = m_rectApps.bottom;
		}
		else if (nDay == 3)
		{
			rect = m_rectApps;

			rect.left  += nWidth2 + m_OnePoint.cx;
			rect.bottom = rect.top + nHeight3;
		}

		if (nWeekDay == -1)
		{
			CRect rt (rect);

			rt.bottom = rect.top + rect.Height () / 2;
			m_ViewRects.Add (rt);

			rt.top = rt.bottom + m_OnePoint.cy;
			rt.bottom = rect.bottom;
			m_ViewRects.Add (rt);
		}
		else
		{
			m_ViewRects.Add (rect);
		}

		rect.OffsetRect (0, nHeight3 + m_OnePoint.cy);
	}
}

void CBCGPPlannerPrintWeek::AdjustAppointments ()
{
	XBCGPAppointmentArray& arQueryApps = GetQueryedAppointments ();

	const int nDays = GetViewDuration ();

	if (arQueryApps.GetSize () == 0 || m_ViewRects.GetSize () != nDays)
	{
		return;
	}

	for (int i = 0; i < arQueryApps.GetSize (); i++)
	{
		CBCGPAppointment* pApp = arQueryApps[i];
		ASSERT_VALID (pApp);

		pApp->ResetPrint ();
	}

	COleDateTime date (m_DateStart);

	for (int nDay = 0; nDay < nDays; nDay ++)
	{
		CRect rect (m_ViewRects [nDay]);
		rect.top += m_nRowHeight + 2 * m_OnePoint.cy;
		rect.DeflateRect (m_OnePoint.cx, 0);

		int nItem = 0;

		for (int i = 0; i < arQueryApps.GetSize (); i++)
		{
			CBCGPAppointment* pApp = arQueryApps[i];
			ASSERT_VALID (pApp);

			if (CBCGPPlannerView::IsAppointmentInDate (*pApp, date))
			{
				CRect rtApp (rect);

				if (nItem > 0)
				{
					rtApp.top += nItem * (m_nRowHeight + 2 * m_OnePoint.cy);
				}

				nItem++;

				rtApp.bottom = rtApp.top + m_nRowHeight;

				pApp->SetRectPrint (rtApp, date);
			}
		}

		CheckVisibleAppointments(date, rect, TRUE);

		date += COleDateTimeSpan (1, 0, 0, 0);
	}
}

void CBCGPPlannerPrintWeek::OnPaint (CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID (pDC);
	ASSERT (pInfo != NULL);

	CBCGPPlannerPrint::OnPaint (pDC, pInfo);

	int nOldBk = pDC->SetBkMode (TRANSPARENT);
	CFont* pOldFont = pDC->SelectObject (&m_Font);

	OnDrawClient (pDC);

	OnDrawAppointments (pDC);

	pDC->SelectObject (pOldFont);
	pDC->SetBkMode (nOldBk);

	{
		CRect rect (m_rectApps);
		rect.InflateRect (m_OnePoint.cx, m_OnePoint.cy);

		HBRUSH hOldBrush = (HBRUSH)::SelectObject (pDC->GetSafeHdc (), ::GetStockObject (NULL_BRUSH));
		CPen* pOldPen = (CPen*)pDC->SelectObject (&m_penBlack);

		pDC->Rectangle (rect);

		::SelectObject (pDC->GetSafeHdc (), hOldBrush);
		pDC->SelectObject (pOldPen);
	}
}

void CBCGPPlannerPrintWeek::OnDrawClient (CDC* pDC)
{
	ASSERT_VALID (pDC);

	const int nDays = GetViewDuration ();

	if (m_ViewRects.GetSize () != nDays)
	{
		return;
	}

	CRect rectFill (m_rectApps);

	DWORD dwFlags = GetDrawFlags ();
	BOOL bBold = (dwFlags & BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_BOLD) ==
			BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_BOLD;

	CFont* pOldFont = NULL;
	if (bBold)
	{
		pOldFont = pDC->SelectObject (&m_FontBold);
	}

	COleDateTime day (GetDateStart ());

	int nDay = 0;
	for (nDay = 0; nDay < nDays; nDay++)
	{
		CRect rectDayCaption (m_ViewRects [nDay]);
		rectDayCaption.bottom = rectDayCaption.top + m_nRowHeight;

		DrawCaption (pDC, rectDayCaption, day, FALSE, TRUE, m_brGray);

		day += COleDateTimeSpan (1, 0, 0, 0);
	}

	if (bBold && pOldFont != NULL)
	{
		pDC->SelectObject (pOldFont);
	}

	CPen* pOldPen = pDC->SelectObject (&m_penBlack);

	pDC->MoveTo (m_ViewRects[0].right, m_rectApps.top);
	pDC->LineTo (m_ViewRects[0].right, m_rectApps.bottom);

	for (nDay = 1; nDay < 7; nDay++)
	{
		if (m_ViewRects[nDay].top != m_rectApps.top)
		{
			pDC->MoveTo (m_ViewRects[nDay].left, m_ViewRects[nDay].top - m_OnePoint.cy);
			pDC->LineTo (m_ViewRects[nDay].right, m_ViewRects[nDay].top - m_OnePoint.cy);
		}
	}

	pDC->SelectObject (pOldPen);
}

void CBCGPPlannerPrintWeek::CalculateDates (const COleDateTime& date)
{
	m_DateStart = GetFirstWeekDay2 (date, 
		CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () + 1);
	m_DateEnd   = m_DateStart + COleDateTimeSpan (6, 23, 59, 59);
}

void CBCGPPlannerPrintWeek::SetDrawTimeFinish (BOOL bDraw)
{
	m_bDrawTimeFinish = bDraw;
}

BOOL CBCGPPlannerPrintWeek::IsDrawTimeFinish () const
{
	return m_bDrawTimeFinish;
}

COleDateTime CBCGPPlannerPrintWeek::GetFirstWeekDay2 (const COleDateTime& date, int nWeekStart) const
{
	return CBCGPPlannerView::GetFirstWeekDay (date, nWeekStart == 1 ? 2 : nWeekStart);
}

#endif // BCGP_EXCLUDE_PLANNER
