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
// BCGPPlannerPrintDay.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPPlannerPrintDay.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerManagerCtrl.h"
#include "BCGPPlannerViewDay.h"

#include "BCGPMath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerPrintDay

IMPLEMENT_DYNCREATE(CBCGPPlannerPrintDay, CBCGPPlannerPrint)

CBCGPPlannerPrintDay::CBCGPPlannerPrintDay()
	: CBCGPPlannerPrint ()
	, m_TimeDelta       (CBCGPPlannerView::BCGP_PLANNER_TIME_DELTA_60)
{
}

CBCGPPlannerPrintDay::~CBCGPPlannerPrintDay()
{
}

#ifdef _DEBUG
void CBCGPPlannerPrintDay::AssertValid() const
{
	CObject::AssertValid();
}

void CBCGPPlannerPrintDay::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif

void CBCGPPlannerPrintDay::DrawAppointment (CDC* pDC, CBCGPAppointment* pApp, CBCGPAppointmentDrawStruct* pDS)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pApp);

	if (pDC == NULL || pDC->GetSafeHdc () == NULL || pApp == NULL || pDS == NULL)
	{
		return;
	}

	CRect rect (pDS->GetRect ());
	CRect rectOriginal (rect);

	CRect rectClip(m_ViewRects[0]);
	if (!pApp->IsAllDay () && !pApp->IsMultiDay ())
	{
		if (rect.top < rectClip.top)
		{
			rect.top = rectClip.top;
		}
		if (rectClip.bottom < rect.bottom)
		{
			rect.bottom = rectClip.bottom;
		}
	}

	COleDateTime dtStart  (pApp->GetStart ());
	COleDateTime dtFinish (pApp->GetFinish ());

	CString      strStart  (pApp->m_strStart);
	CString      strFinish (pApp->m_strFinish);

	BOOL bAlternative = pApp->IsAllDay () || pApp->IsMultiDay ();
	BOOL bEmpty       = (dtStart == dtFinish) && !bAlternative;

	const int nMinuts  = CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ());

	BOOL bDrawTime = !bAlternative;

	if (!bAlternative)
	{
		CRect rt (rect);
		rt.right = rt.left + CBCGPPlannerViewDay::BCGP_PLANNER_DURATION_BAR_WIDTH * m_OnePoint.cx;

		{
			CBrush br (RGB (255, 255, 255));
			pDC->FillRect (rt, &br);
		}

		{
			BOOL bNonSingleDay = !CBCGPPlannerView::IsOneDay (dtStart, dtFinish);

			if (bNonSingleDay)
			{
				if (pDS->GetBorder () == CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START)
				{
					dtFinish  = COleDateTime (dtFinish.GetYear (), dtFinish.GetMonth (), dtFinish.GetDay (),
						0, 0, 0);
				}
				else if (pDS->GetBorder () == CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH)
				{
					dtStart  = COleDateTime (dtFinish.GetYear (), dtFinish.GetMonth (), dtFinish.GetDay (),
						0, 0, 0);
				}
			}

			double dStart = 
				(dtStart.GetHour () * 60 + dtStart.GetMinute ()) / (double)nMinuts;
			double dEnd = CBCGPPlannerView::IsOneDay (dtStart, dtFinish)
				? (dtFinish.GetHour () * 60 + dtFinish.GetMinute ()) / (double)nMinuts
				: 24 * 60 / (double)nMinuts;

			int nStart = (int)dStart;
			int nEnd   = (int)ceil(dEnd);

			CBrush br (pApp->GetDurationColor () == CLR_DEFAULT ? m_clrPen : pApp->GetDurationColor ());

			if (!bEmpty)
			{
				double dDelta = (double)rt.Height () / (double)(nEnd - nStart);

				CRect rt1 (rt);
				rt1.top    = rectOriginal.top + long((dStart - nStart) * dDelta);
				rt1.bottom = rectOriginal.bottom - long((nEnd - dEnd) * dDelta);
				if (rt1.top < rectClip.top)
				{
					rt1.top = rectClip.top;
				}
				if (rectClip.bottom < rt1.bottom)
				{
					rt1.bottom = rectClip.bottom;
				}

				pDC->FillRect (rt1, &br);
			}
			else
			{
				POINT points[3];
				points[0].x = rt.left;
				points[0].y = rt.top;
				points[1].x = rt.right;
				points[1].y = rt.top;
				points[2].x = rt.right;
				points[2].y = rt.top + rt.right - rt.left;

				CRgn rgn;
				rgn.CreatePolygonRgn (points, 3, WINDING);

				pDC->FillRgn (&rgn, &br);
			}

			if (bNonSingleDay)
			{
				dtStart  = pApp->GetStart ();
				dtFinish = pApp->GetFinish ();

				dStart = 
					(dtStart.GetHour () * 60 + dtStart.GetMinute ()) / (double)nMinuts;
				dEnd = CBCGPPlannerView::IsOneDay (dtStart, dtFinish)
					? (dtFinish.GetHour () * 60 + dtFinish.GetMinute ()) / (double)nMinuts
					: 24 * 60 / (double)nMinuts;

				nStart = (int)dStart;
				nEnd   = (int)ceil(dEnd);
			}

			bDrawTime = dStart != nStart || dEnd != nEnd;
		}

		CPen penBlack (PS_SOLID, 0, m_clrPen);
		CPen* pOldPen = pDC->SelectObject (&penBlack);

		pDC->MoveTo (rt.right, rt.top);
		pDC->LineTo (rt.right, rt.bottom);

		pDC->SelectObject (pOldPen);

		rect.left = rt.right + m_OnePoint.cx;
	}
	else
	{
		rect.DeflateRect (4 * m_OnePoint.cx, 0);
	}

	{
		CBrush br (pApp->GetBackgroundColor () == CLR_DEFAULT ? RGB (255, 255, 255) : pApp->GetBackgroundColor ());
		pDC->FillRect (rect, &br);
	}

	COLORREF clrOld = pDC->SetTextColor (pApp->GetForegroundColor () != CLR_DEFAULT 
		? pApp->GetForegroundColor ()
		: m_clrText);

	if (bDrawTime)
	{
		CSize szSpace (pDC->GetTextExtent (_T(" ")));

		CString str;

		if (!strStart.IsEmpty ())
		{
			str = strStart;

			if (!strFinish.IsEmpty () && !bEmpty)
			{
				str += _T("-") + strFinish;
			}
		}

		if (!str.IsEmpty ())
		{
			CSize sz (pDC->GetTextExtent (str));

			CRect rectText (rect);
			rectText.DeflateRect (4 * m_OnePoint.cx, m_OnePoint.cy, m_OnePoint.cx, 0);

			pDC->DrawText (str, rectText, DT_NOPREFIX);	

			rect.left += sz.cx + szSpace.cx;
		}
	}

	BOOL bDrawText = !pApp->m_strDescription.IsEmpty ();

	CSize szText (0, 0);
	if (bDrawText)
	{
		szText = pDC->GetTextExtent (pApp->m_strDescription);
	}

	if (bDrawText)
	{
		CRect rectText (rect);

		if (bAlternative && szText.cx < rectText.Width ())
		{
			rectText.left += (rectText.Width () - szText.cx) / 2;
		}
		else
		{
			// if time drawed and description not completely in view, then
			// move description down (if possible)
			if (bDrawTime)
			{
				if ((szText.cx + 4 * m_OnePoint.cx) > rect.Width () && 
					rect.Height () > szText.cy * 2)
				{
					rectText.left = pDS->GetRect ().left + 
						(CBCGPPlannerViewDay::BCGP_PLANNER_DURATION_BAR_WIDTH + 1) * m_OnePoint.cx;
					rectText.OffsetRect (0, szText.cy);
				}
			}
		}

		rectText.DeflateRect (4 * m_OnePoint.cx, m_OnePoint.cy, m_OnePoint.cx, 0);

		pDC->DrawText (pApp->m_strDescription, rectText, DT_NOPREFIX);
	}

	if (!bAlternative)
	{
		rect.left = pDS->GetRect ().left;
	}

	CPen pen (PS_SOLID, 0, m_clrPen);

	HBRUSH hOldBrush = (HBRUSH)::SelectObject (pDC->GetSafeHdc (), ::GetStockObject (NULL_BRUSH));
	CPen* pOldPen = (CPen*)pDC->SelectObject (&pen);

	if (!bAlternative || 
		(bAlternative && pDS->GetBorder () == CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_ALL))
	{
		pDC->Rectangle (rect);
	}
	else if (bAlternative)
	{
		pDC->MoveTo (rect.left , rect.top);
		pDC->LineTo (rect.right, rect.top);

		pDC->MoveTo (rect.left , rect.bottom - m_OnePoint.cy);
		pDC->LineTo (rect.right, rect.bottom - m_OnePoint.cy);

		if (pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START)
		{
			pDC->MoveTo (rect.left, rect.top);
			pDC->LineTo (rect.left, rect.bottom);
		}

		if (pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH)
		{
			pDC->MoveTo (rect.right - m_OnePoint.cx, rect.top);
			pDC->LineTo (rect.right - m_OnePoint.cx, rect.bottom);
		}
	}

	::SelectObject (pDC->GetSafeHdc (), hOldBrush);
	pDC->SelectObject (pOldPen);

	pDC->SetTextColor (clrOld);
}

void CBCGPPlannerPrintDay::OnDrawAppointmentsDuration (CDC* pDC)
{
	XBCGPAppointmentArray& arApps = GetQueryedAppointments ();

	if (arApps.GetSize () == 0)
	{
		return;
	}

	COleDateTime dtS = GetDateStart ();
	COleDateTime dtE = GetDateEnd ();

	const int nMinuts  = CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ());
	const int nCount = GetViewHours() * 60 / nMinuts;
	const int yOffset = GetViewHourOffset () * m_nRowHeight;

	for (int i = 0; i < arApps.GetSize (); i++)
	{
		CBCGPAppointment* pApp = arApps [i];
		if (pApp == NULL || !(pApp->IsAllDay () || pApp->IsMultiDay ()) || 
			pApp->GetDurationColor () == CLR_DEFAULT)
		{
			continue;
		}

		COleDateTime dtStart  = pApp->GetStart ();
		COleDateTime dtFinish = pApp->GetFinish ();

		dtStart.SetDate (dtStart.GetYear (), dtStart.GetMonth (), dtStart.GetDay ());
		dtFinish.SetDate (dtFinish.GetYear (), dtFinish.GetMonth (), dtFinish.GetDay ());

		BOOL bStart = FALSE;
		BOOL bEnd   = FALSE;

		if (dtStart < dtS)
		{
			dtStart = dtS;
			bStart  = TRUE;
		}

		if (dtE < dtFinish)
		{
			dtFinish = dtE;
			bEnd     = TRUE;
		}

		COleDateTimeSpan span (dtFinish - dtStart);

		int nStart = (dtStart - dtS).GetDays ();
		int nEnd   = min(nStart + span.GetDays () + 1, GetViewDuration ());

		CBrush br (pApp->GetDurationColor () == CLR_DEFAULT
			? m_clrPen
			: pApp->GetDurationColor ());

		for(int i = nStart; i < nEnd; i++)
		{
			CRect rt (m_ViewRects[i]);

			rt.right  = rt.left + 
				(CBCGPPlannerViewDay::BCGP_PLANNER_DURATION_BAR_WIDTH + 1) * m_OnePoint.cx;
			rt.left  -= (i == 0) ? m_OnePoint.cx : 0;
			rt.top   -= m_OnePoint.cy;
			rt.bottom = rt.top + nCount * m_nRowHeight;
			rt.DeflateRect (m_OnePoint.cx, 0);

			if (!pApp->IsAllDay ())
			{
				rt.OffsetRect (0, -yOffset);

				if (i == (nEnd - 1) && !bEnd)
				{
					dtFinish = pApp->GetFinish ();
					const double dDelta = (dtFinish.GetHour () * 60 + dtFinish.GetMinute ()) / (double)nMinuts;

					rt.bottom = rt.top + CBCGPPlannerView::round(dDelta * m_nRowHeight);
				}

				if (i == nStart && !bStart)
				{
					dtStart  = pApp->GetStart ();
					const double dDelta = 
						(dtStart.GetHour () * 60 + dtStart.GetMinute ()) / (double)nMinuts;

					rt.top += CBCGPPlannerView::round(dDelta * m_nRowHeight);
				}

				rt.IntersectRect (rt, m_ViewRects[i]);
			}

			pDC->FillRect (rt, &br);
		}
	}
}

void CBCGPPlannerPrintDay::GetCaptionFormatStrings (CStringArray& sa)
{
	sa.RemoveAll ();

	BOOL bCompact = (GetDrawFlags () & BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_COMPACT) ==
			BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_COMPACT;

	if (!bCompact)
	{
		CString strSep (CBCGPPlannerView::GetDateSeparator ());

		if (CBCGPPlannerView::IsDateBeforeMonth ())
		{
			sa.Add (_T("dddd d MMMM"));
			sa.Add (_T("dddd d MMM"));
			sa.Add (_T("ddd d MMM"));
			sa.Add (_T("d") + strSep + _T("M"));
		}
		else
		{
			sa.Add (_T("dddd MMMM d"));
			sa.Add (_T("dddd MMM d"));
			sa.Add (_T("ddd MMM d"));
			sa.Add (_T("M") + strSep + _T("d"));
		}
	}
	else
	{
		sa.Add (_T("d\ndddd"));
		sa.Add (_T("d\nddd"));
		sa.Add (_T("d"));
	}
}

void CBCGPPlannerPrintDay::AdjustLayout (CDC* /*pDC*/, const CRect& rectClient)
{
	m_nHeaderHeight       = GetViewDuration () > 1 ? 1 : 0;
	m_nHeaderAllDayHeight = 1;

	int nCount = GetViewHours() * 60 / CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ());

	m_rectTimeBar = rectClient;
	m_rectTimeBar.right = m_rectTimeBar.left + m_nRowHeight * 3 + 5 * m_OnePoint.cx;
	
	m_rectApps.left = m_rectTimeBar.right;

	{
		// finding allday or multiday events

		CDWordArray arDays;
		arDays.SetSize (GetViewDuration ());

		COleDateTime dtS = GetDateStart ();

		XBCGPAppointmentArray& arApps = GetQueryedAppointments ();

		int i = 0;
		for (i = 0; i < arApps.GetSize (); i++)
		{
			const CBCGPAppointment* pApp = arApps[i];
			if (pApp == NULL)
			{
				continue;
			}

			ASSERT_VALID (pApp);

			if (pApp->IsAllDay () || pApp->IsMultiDay ())
			{
				if (arDays.GetSize () > 1)
				{
					COleDateTime dtStart  = pApp->GetStart ();
					COleDateTime dtFinish = pApp->GetFinish ();

					dtStart = COleDateTime (dtStart.GetYear (), dtStart.GetMonth (), dtStart.GetDay (),
						0, 0, 0);

					if (pApp->IsAllDay ())
					{
						dtFinish += COleDateTimeSpan (1, 0, 0, 0);
					}
					else if (pApp->GetFinish ().GetHour () != 0 ||
							 pApp->GetFinish ().GetMinute () != 0)
					{
						dtFinish = COleDateTime (dtFinish.GetYear (), dtFinish.GetMonth (), dtFinish.GetDay (),
							0, 0, 0);
						dtFinish += COleDateTimeSpan (1, 0, 0, 0);
					}

					if (dtStart < dtS)
					{
						dtStart = dtS;
					}

					COleDateTimeSpan span (dtFinish - dtStart);

					int nStart = (dtStart - dtS).GetDays ();
					int nEnd   = nStart + span.GetDays ();

					if (nEnd > (int) arDays.GetSize ())
					{
						nEnd = (int) arDays.GetSize ();
					}

					for (int j = nStart; j < nEnd; j++)
					{
						arDays[j] = arDays[j] + 1;
					}
				}
				else
				{
					arDays[0] = arDays[0] + 1;
				}
			}
		}

		DWORD maxCount = 0;
		for (i = 0; i < arDays.GetSize (); i++)
		{
			if (maxCount < arDays[i])
			{
				maxCount = arDays[i];
			}
		}

		if (maxCount > 0)
		{
			m_nHeaderAllDayHeight = maxCount;
		}
	}

	int nRow = rectClient.Height () / 
		(nCount + m_nHeaderHeight + m_nHeaderAllDayHeight);

	int nOldRowHeight = m_nRowHeight;

	if (nRow > m_nRowHeight)
	{
		m_nRowHeight = nRow;
	}

	m_nHeaderHeight       *= m_nRowHeight;
	m_nHeaderAllDayHeight *= (m_nRowHeight + 2 * m_OnePoint.cy);

	m_rectApps.top += m_nHeaderHeight;

	nRow = (m_rectApps.Height () - m_nHeaderAllDayHeight) / nCount;

	if (nRow > nOldRowHeight)
	{
		m_nRowHeight = nRow;
	}

	int delta = m_rectApps.Height () - m_nHeaderAllDayHeight;

	if (delta < 0)
	{
		m_nHeaderAllDayHeight = m_rectApps.Height ();
	}
	else
	{
		int nc = (int)(delta / m_nRowHeight);

		if (nc >= nCount)
		{
			m_nHeaderAllDayHeight = m_rectApps.Height () - nCount * m_nRowHeight;
		}
		else
		{
			m_nHeaderAllDayHeight += delta - nc * m_nRowHeight;
		}
	}

	m_rectApps.top += m_nHeaderAllDayHeight;
}

void CBCGPPlannerPrintDay::AdjustRects ()
{
	const int nDays = GetViewDuration ();
	const int dxColumn = CBCGPPlannerView::round (m_rectApps.Width () / (double)nDays);

	CRect rect (m_rectApps);
	rect.right = rect.left + dxColumn;

	for (int nDay = 0; nDay < nDays; nDay++)
	{
		m_ViewRects.Add (rect);

		rect.OffsetRect (dxColumn, 0);

		if (nDay == (nDays - 2))
		{
			rect.right = m_rectApps.right;
		}
	}
}

void CBCGPPlannerPrintDay::AdjustAppointments ()
{
	XBCGPAppointmentArray& arQueryApps = GetQueryedAppointments ();

	const int nDays = GetViewDuration ();

	if (arQueryApps.GetSize () == 0 || m_ViewRects.GetSize () != nDays)
	{
		return;
	}

	COleDateTime date (m_DateStart);

	for (int i = 0; i < arQueryApps.GetSize (); i++)
	{
		CBCGPAppointment* pApp = arQueryApps[i];
		ASSERT_VALID (pApp);

		pApp->ResetPrint ();
	}

	const int nTimeDelta = CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ());
	const int yOffset = GetViewHourOffset () * m_nRowHeight;

	COleDateTimeSpan spanDay (1, 0, 0, 0);

	XBCGPAppointmentArray arAllDays;

	int nDay = 0;
	const int c_Space = CBCGPPlannerViewDay::BCGP_PLANNER_APPOINTMENT_SPACE * m_OnePoint.cx;

	for (nDay = 0; nDay < nDays; nDay ++)
	{	
		XBCGPAppointmentArray arByDate;

		int i;

		CRect rectFill (m_ViewRects [nDay]);
		rectFill.top -= yOffset;

		for (i = 0; i < arQueryApps.GetSize (); i++)
		{
			CBCGPAppointment* pApp = arQueryApps[i];
			ASSERT_VALID (pApp);

			if (CBCGPPlannerView::IsAppointmentInDate (*pApp, date))
			{
				CRect rtApp (rectFill);

				// trimming top and bottom edge, starting from TimeDelta and Appointment interval
				rtApp.top = rectFill.top + m_nRowHeight * 
					int((pApp->GetStart ().GetHour () * 60 + 
						 pApp->GetStart ().GetMinute ()) / nTimeDelta) - m_OnePoint.cy;

				if (pApp->IsAllDay () || pApp->IsMultiDay ())
				{
					BOOL bAdd = TRUE;

					for (int i = 0; i < arAllDays.GetSize (); i++)
					{
						if (arAllDays[i] == pApp)
						{
							bAdd = FALSE;
							break;
						}
					}

					if (bAdd)
					{
						arAllDays.Add (pApp);
					}
				}
				else
				{
					rtApp.right -= 2 * c_Space;

					if (pApp->GetStart () != pApp->GetFinish ())
					{
						COleDateTime dtAF (pApp->GetFinish ());
						int minutes = dtAF.GetHour () * 60 + dtAF.GetMinute ();

						if (!CBCGPPlannerView::IsOneDay (pApp->GetStart (), dtAF))
						{
							BOOL bStartDay   = CBCGPPlannerView::IsOneDay (date, pApp->GetStart ());
							BOOL bFinishNULL = dtAF.GetHour () == 0 && 
											   dtAF.GetMinute () == 0;

							if (bStartDay)
							{
								minutes = 24 * 60;
							}

							if (bFinishNULL)
							{
								if (!bStartDay)
								{
									if (pApp->GetDSPrint ().GetCount () == 1)
									{
										pApp->GetDSPrint ()[0].m_date2 = date;
										continue;
									}
								}
							}
							else
							{
								if (!bStartDay)
								{
									rtApp.top = rectFill.top - m_OnePoint.cy;
								}
							}
						}

						rtApp.bottom = rectFill.top + m_nRowHeight * 
							(long)ceil(minutes / (double)nTimeDelta) - m_OnePoint.cy;
					}
					else
					{
						rtApp.bottom = rtApp.top + m_nRowHeight;
					}

					pApp->SetRectPrint (rtApp, date);
					arByDate.Add (pApp);
				}
			}
		}

		// resort appointments in the view, if count of collection is great than 1
		if (arByDate.GetSize () > 1)
		{
			XBCGPAppointmentArray* ar = new XBCGPAppointmentArray;

			// array, that contains columns
			CArray<XBCGPAppointmentArray*, XBCGPAppointmentArray*> arColumns;
			arColumns.Add (ar);

			// initialize first column
			ar->Copy (arByDate);

			while (ar != NULL)
			{
				CBCGPAppointment* pApp1 = ar->GetAt (0);

				XBCGPAppointmentArray* arNew = NULL;

				i = 1;

				// remove appointmets, that have collisions in rects, from previous column 
				// to the next column
				while (i < ar->GetSize ())
				{
					CBCGPAppointment* pApp2 = ar->GetAt (i);

					CRect rtInter;
					if (rtInter.IntersectRect (pApp1->GetRectPrint (date), pApp2->GetRectPrint (date)))
					{
						if (arNew == NULL)
						{
							// add a new column
							arNew = new XBCGPAppointmentArray;
							arColumns.Add (arNew);
						}

						arNew->Add (pApp2);
						ar->RemoveAt (i);
					}
					else
					{
						pApp1 = pApp2;
						i++;
					}
				}

				ar = arNew;
			}

			int nCount = (int) arColumns.GetSize ();

			// reinitialize drawing rects, if found great than 1 columns
			if (nCount > 1)
			{
				int nWidth = rectFill.Width () / nCount;
				int nL = rectFill.left;

				// left border of appointments, based on column order
				for (i = 0; i < nCount; i++)
				{
					ar = arColumns[i];

					for (int j = 0; j < ar->GetSize (); j++)
					{
						CBCGPAppointment* pApp = ar->GetAt (j);

						CRect rtApp (pApp->GetRectPrint (date));
						rtApp.left = nL;

						pApp->SetRectPrint (rtApp, date);
					}

					nL += nWidth;
				}

				// correcting right border of appointments
				for (i = 0; i < nCount; i++)
				{
					ar = arColumns[i];

					for (int j = 0; j < ar->GetSize (); j++)
					{
						CBCGPAppointment* pApp = ar->GetAt (j);
						CRect rtApp (pApp->GetRectPrint (date));

						for (int k = i + 1; k < nCount; k++)
						{
							XBCGPAppointmentArray* arNext = arColumns[k];

							for (int m = 0; m < arNext->GetSize (); m++)
							{
								CBCGPAppointment* pAppNext = arNext->GetAt (m);

								CRect rtInter;
								if (rtInter.IntersectRect (rtApp, pAppNext->GetRectPrint (date)))
								{
									rtApp.right = rectFill.left + nWidth * k - c_Space;
									pApp->SetRectPrint (rtApp, date);
									break;
								}
							}
						}
					}
				}
			}

			// clean up columns array
			for (i = 0; i < nCount; i++)
			{
				delete arColumns[i];
			}

			arColumns.RemoveAll ();
		}
		
		date += spanDay;
	}

	// adjust "all day" or "multi day" appointments
	if (arAllDays.GetSize () > 0)
	{
		const int c_Count = (int) arAllDays.GetSize ();

		CRect rectFill (m_ViewRects [0]);
		rectFill.top    = m_rectApps.top - 
				(m_nHeaderHeight + m_nHeaderAllDayHeight) + m_OnePoint.cy;
		rectFill.bottom = rectFill.top + m_nRowHeight;

		int i = 0;

		for (i = 0; i < c_Count; i++)
		{
			CBCGPAppointment* pApp = arAllDays[i];

			date = m_DateStart;

			for (nDay = 0; nDay < nDays; nDay ++)
			{
				rectFill.left  = m_ViewRects [nDay].left;
				rectFill.right = m_ViewRects [nDay].right;

				if (CBCGPPlannerView::IsAppointmentInDate (*pApp, date))
				{
					pApp->SetRectPrint (rectFill, date);
				}

				date += spanDay;
			}
		}

		for (i = 1; i < c_Count; i++)
		{
			CBCGPAppointment* pApp1 = arAllDays[i];

			CRect rtApp1;
			if (pApp1->GetDSPrint ().IsEmpty ())
			{
				rtApp1 = pApp1->GetRectPrint ();
			}
			else
			{
				rtApp1 = pApp1->GetDSPrint ().GetByIndex (0)->GetRect ();
			}

			for (int j = 0; j < i; j++)
			{
				CBCGPAppointment* pApp2 = arAllDays[j];

				CRect rtApp2;
				if (pApp2->GetDSPrint ().IsEmpty ())
				{
					rtApp2 = pApp2->GetRectPrint ();
				}
				else
				{
					rtApp2 = pApp2->GetDSPrint ().GetByIndex (0)->GetRect ();
				}

				CRect rtInter;
				if (rtInter.IntersectRect (rtApp1, rtApp2))
				{
					rtApp1.top    = rtApp2.top;
					rtApp1.bottom = rtApp2.bottom;
					rtApp1.OffsetRect (0, m_nRowHeight + 2 * m_OnePoint.cy);
					
					if (pApp1->GetDSPrint ().IsEmpty ())
					{
						pApp1->SetRectPrint (rtApp1);
					}
					else
					{
						pApp1->GetDSPrint ().GetByIndex (0)->SetRect (rtApp1);
					}

					j = 0;
				}
			}
		}
	}

	date = m_DateStart;

	for (nDay = 0; nDay < nDays; nDay ++)
	{
		CheckVisibleAppointments (date, m_rectApps, FALSE);

		date += spanDay;
	}
}

void CBCGPPlannerPrintDay::CalculateDates (const COleDateTime& date)
{
	m_DateStart.SetDate (date.GetYear (), date.GetMonth (), date.GetDay ());
	m_DateEnd   = m_DateStart + COleDateTimeSpan (0, 23, 59, 59);
}

void CBCGPPlannerPrintDay::OnPaint (CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID (pDC);
	ASSERT (pInfo != NULL);

	CBCGPPlannerPrint::OnPaint (pDC, pInfo);

	int nOldBk = pDC->SetBkMode (TRANSPARENT);
	CFont* pOldFont = pDC->SelectObject (&m_Font);

	CRect rectFrame (m_rectApps);

	OnDrawClient (pDC);

	OnDrawAppointmentsDuration (pDC);

	if (m_nHeaderAllDayHeight != 0)
	{
		CRect rtHeader (m_rectApps);
		rtHeader.top    -= m_nHeaderAllDayHeight;
		rtHeader.bottom  = rtHeader.top + m_nHeaderAllDayHeight;

		OnDrawHeaderAllDay (pDC, rtHeader);

		rectFrame.top = rtHeader.top;
	}

	OnDrawAppointments (pDC, m_rectApps);

	if (!m_rectTimeBar.IsRectEmpty ())
	{
		OnDrawTimeBar (pDC, m_rectTimeBar);

		rectFrame.left = m_rectTimeBar.left;
	}

	if (m_nHeaderHeight != 0)
	{
		CRect rtHeader (m_rectApps);
		rtHeader.top    -= (m_nHeaderHeight + m_nHeaderAllDayHeight);
		rtHeader.bottom = rtHeader.top + m_nHeaderHeight;

		OnDrawHeader (pDC, rtHeader);

		rectFrame.top = rtHeader.top;
	}

	pDC->SelectObject (pOldFont);
	pDC->SetBkMode (nOldBk);

	{
		rectFrame.InflateRect (m_OnePoint.cx, m_OnePoint.cy);

		HBRUSH hOldBrush = (HBRUSH)::SelectObject (pDC->GetSafeHdc (), ::GetStockObject (NULL_BRUSH));
		CPen* pOldPen = (CPen*)pDC->SelectObject (&m_penBlack);

		pDC->Rectangle (rectFrame);

		::SelectObject (pDC->GetSafeHdc (), hOldBrush);
		pDC->SelectObject (pOldPen);
	}
}

void CBCGPPlannerPrintDay::OnDrawClient (CDC* pDC)
{
	ASSERT_VALID (pDC);

	CRect rectFill (m_rectApps);

	const int yOffset = 0;//m_nScrollOffset * m_nRowHeight;

	const int nDays = GetViewDuration ();

	rectFill.OffsetRect (0, -yOffset);

	const int nMinuts = CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ());
	const int nCount = 60 / nMinuts;

	CPen* pOldPen = pDC->SelectObject (&m_penBlack);

	const int iStart = GetViewHourOffset();
	const int iEnd   = min (iStart + m_rectApps.Height () / m_nRowHeight, nCount * 24);//min (m_nScrollOffset + rect.Height () / m_nRowHeight, nCount * 24);

	COleDateTime dtStart (GetDateStart ());
	int nDay = 0;

	for (nDay = 0; nDay < nDays; nDay++)
	{
		rectFill = m_ViewRects [nDay];

		rectFill.left   += (CBCGPPlannerViewDay::BCGP_PLANNER_DURATION_BAR_WIDTH + 1) * m_OnePoint.cx;
		rectFill.bottom = rectFill.top + m_nRowHeight - m_OnePoint.cy;

		for (int iStep = iStart; iStep < iEnd; iStep++)
		{
			pDC->MoveTo (rectFill.left, rectFill.bottom);
			pDC->LineTo (rectFill.right, rectFill.bottom);

			rectFill.OffsetRect (0, m_nRowHeight);
		}

		dtStart += COleDateTimeSpan (1, 0, 0, 0);
	}

	rectFill = m_rectApps;

	for (nDay = 0; nDay < nDays; nDay++)
	{
		CRect rectDurBar (m_ViewRects [nDay]);
		rectDurBar.right = rectDurBar.left + CBCGPPlannerViewDay::BCGP_PLANNER_DURATION_BAR_WIDTH * m_OnePoint.cx;

		// Draw duration bar (at left):
		pDC->FillRect (rectDurBar, &globalData.brWindow);

		if (nDay > 0)
		{
			pDC->MoveTo (rectDurBar.left, rectDurBar.top);
			pDC->LineTo (rectDurBar.left, rectDurBar.bottom);
		}

		pDC->MoveTo (rectDurBar.right, rectDurBar.top);
		pDC->LineTo (rectDurBar.right, rectDurBar.bottom);
	}

	pDC->SelectObject (pOldPen);
}

void CBCGPPlannerPrintDay::OnDrawHeader (CDC* pDC, const CRect& rectHeader)
{
	ASSERT_VALID (pDC);

	const int nDays = GetViewDuration ();

	DrawHeader (pDC, rectHeader, m_ViewRects [0].Width ());

	CRect rectDayCaption (rectHeader);

	COleDateTime day (GetDateStart ());

	for (int nDay = 0; nDay < nDays; nDay++)
	{
		rectDayCaption.left = m_ViewRects [nDay].left;
		rectDayCaption.right = m_ViewRects [nDay].right;

		DrawCaption (pDC, rectDayCaption, day, TRUE, TRUE, m_brGray);

		day += COleDateTimeSpan (1, 0, 0, 0);
	}
}

void CBCGPPlannerPrintDay::OnDrawHeaderAllDay (CDC* pDC, const CRect& rectHeader)
{
	ASSERT_VALID (pDC);

	const int nDays = GetViewDuration ();

	pDC->FillRect (rectHeader, &m_brGray);

	CPen* pOldPen = pDC->SelectObject (&m_penBlack);

	if (nDays > 1)
	{
		for (int nDay = 1; nDay < nDays; nDay++)
		{
			pDC->MoveTo (m_ViewRects [nDay].left, rectHeader.top);
			pDC->LineTo (m_ViewRects [nDay].left, rectHeader.bottom - m_OnePoint.cy);
		}
	}

	pDC->MoveTo (rectHeader.left, rectHeader.bottom - m_OnePoint.cy);
	pDC->LineTo (rectHeader.right, rectHeader.bottom - m_OnePoint.cy);

	pDC->SelectObject (pOldPen);
}

void CBCGPPlannerPrintDay::OnDrawTimeBar (CDC* pDC, const CRect& rectBar)
{
	ASSERT_VALID (pDC);

	BOOL b24Hours = CBCGPPlannerView::Is24Hours ();

	CString strAM;
	CString strPM;

	if (!b24Hours)
	{
		strAM = CBCGPPlannerView::GetTimeDesignator (TRUE);
		strAM.MakeLower ();
		strPM = CBCGPPlannerView::GetTimeDesignator (FALSE);
		strPM.MakeLower ();
	}

	pDC->FillRect (rectBar, &m_brGray);

	const int nHeaderHeight = m_nHeaderHeight + m_nHeaderAllDayHeight;

	int y = rectBar.top + nHeaderHeight - m_OnePoint.cy;

	CPen* pOldPen = pDC->SelectObject (&m_penBlack);

	const long nCount = 60 / CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ());

	UINT nFormat = DT_SINGLELINE | DT_RIGHT;

	if (nCount == 1)
	{
		nFormat |= DT_VCENTER;
	}


	LOGFONT lf;
	m_Font.GetLogFont (&lf);

	lf.lfHeight *= 2;

	CFont fontBold;
	fontBold.CreateFontIndirect (&lf);

	int bDrawFirstAM_PM = 0;

	int nStartHour = GetFirstViewHour();
	int nEndHour = GetLastViewHour();

	for (int i = nStartHour; i <= nEndHour; i++)
	{
		CRect rectHour  = rectBar;
		rectHour.top    = y;
		rectHour.bottom = y + m_nRowHeight * nCount;
		rectHour.left   += 5 * m_OnePoint.cx;
		rectHour.right  -= 5 * m_OnePoint.cy;

		if(rectHour.bottom < nHeaderHeight)
		{
			y = rectHour.bottom;
			continue;
		}

		if (nCount > 2)
		{
			long nd = y + m_nRowHeight;

			for (int j = 0; j < nCount - 1; j++)
			{
				if (nd >= nHeaderHeight)
				{
					pDC->MoveTo (rectHour.right - 18 * m_OnePoint.cx, nd);
					pDC->LineTo (rectHour.right, nd);
				}

				nd += m_nRowHeight;
			}
		}

		if (rectHour.bottom >= nHeaderHeight)
		{
			y += m_nRowHeight * nCount;

			pDC->MoveTo (rectHour.left , y);
			pDC->LineTo (rectHour.right, y);
		}

		if (rectHour.top >= nHeaderHeight || rectHour.bottom > nHeaderHeight)
		{
			if (rectHour.top >= nHeaderHeight - m_OnePoint.cy)
			{
				bDrawFirstAM_PM++;
			}

			CString str (_T("00"));

			int nHour = i;

			BOOL bAM = nHour < 12; 

			if (!b24Hours)
			{
				if (nHour == 0 || nHour == 12)
				{
					nHour = 12;
				}
				else if (nHour > 12)
				{
					nHour -= 12;
				}
			}

			if (nCount == 1)
			{
				str.Format (_T("%2d:00"), nHour);

				if (!b24Hours)
				{
					if (nHour == 12)
					{
						str.Format (_T("12 %s"), bAM ? strAM : strPM);
					}
				}

				pDC->DrawText (str, rectHour, nFormat);
			}
			else
			{
				y = rectHour.bottom;

				rectHour.bottom = rectHour.top + m_nRowHeight;

				if (!b24Hours)
				{
					if (nHour == 12 || bDrawFirstAM_PM == 1)
					{
						str = bAM ? strAM : strPM;
					}
				}

				CRect rectMin (rectHour);
				rectMin.left   = rectMin.right - 18 * m_OnePoint.cx;

				pDC->DrawText (str, rectMin, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

				pDC->SelectObject (&fontBold);

				rectHour.bottom = y;
				rectHour.right -= 18 * m_OnePoint.cx;

				str.Format (_T("%d"), nHour);
				pDC->DrawText (str, rectHour, nFormat);

				pDC->SelectObject (&m_Font);
			}
		}

		y = rectHour.bottom;
	}

	CRect rt (rectBar);
	rt.bottom = rt.top + nHeaderHeight;

	pDC->FillRect (rt, &m_brGray);

	pDC->MoveTo (rectBar.left, rt.bottom - m_OnePoint.cy);
	pDC->LineTo (rectBar.right - 5 * m_OnePoint.cx, rt.bottom - m_OnePoint.cy);

	pDC->MoveTo (rectBar.right - m_OnePoint.cx, rectBar.top);
	pDC->LineTo (rectBar.right - m_OnePoint.cx, rectBar.bottom);

	pDC->SelectObject (pOldPen);
}

void CBCGPPlannerPrintDay::OnDrawPageHeader (CDC* pDC)
{
	if (m_rectPageHeader.IsRectEmpty ())
	{
		return;
	}

	{
		CBrush* pOldBrush = (CBrush*)pDC->SelectObject (&m_brGray);
		CPen* pOldPen = (CPen*)pDC->SelectObject (&m_penBlack);

		pDC->Rectangle (m_rectPageHeader);

		pDC->SelectObject (pOldBrush);
		pDC->SelectObject (pOldPen);
	}


	SYSTEMTIME st;
	m_Date.GetAsSystemTime (st);

	CString strFmtDate;
	if (CBCGPPlannerView::IsDateBeforeMonth ())
	{
		strFmtDate = _T("d MMMM yyyy");
	}
	else
	{
		strFmtDate = _T("MMMM d yyyy");
	}

	TCHAR sz[100];
	::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st, strFmtDate, sz, 100);

	CString str1 (sz);

	::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st, _T("dddd"), sz, 100);

	CString str2 (sz);

	int nOldBk = pDC->SetBkMode (TRANSPARENT);
	CFont* pOldFont = pDC->SelectObject (&m_FontHeader);

	CRect rt (m_rectPageHeader);
	rt.DeflateRect (m_OnePoint.cx * 4, m_OnePoint.cy);

	pDC->DrawText (str1, rt, DT_LEFT);

	TEXTMETRIC tm;
	pDC->GetTextMetrics (&tm);

	rt.top += tm.tmHeight;

	LOGFONT lf;
	m_FontHeader.GetLogFont (&lf);

	lf.lfHeight *= 2;
	lf.lfHeight /= 3;
	lf.lfWeight = FW_NORMAL;

	CFont font;
	font.CreateFontIndirect (&lf);

	pDC->SelectObject (&font);

	pDC->DrawText (str2, rt, DT_LEFT);

	pDC->SelectObject (pOldFont);
	pDC->SetBkMode (nOldBk);
}

void CBCGPPlannerPrintDay::CheckVisibleAppointments(const COleDateTime& date, const CRect& rect, 
	BOOL bFullVisible)
{
	XBCGPAppointmentArray& arApps = GetQueryedAppointments ();

	if (arApps.GetSize () == 0)
	{
		return;
	}

	BOOL bSelect = date != COleDateTime ();

	for (int i = 0; i < arApps.GetSize (); i++)
	{
		CBCGPAppointment* pApp = arApps [i];

		if (bSelect)
		{
			if (!CBCGPPlannerView::IsAppointmentInDate (*pApp, date))
			{
				continue;
			}
		}

		CRect rt (rect);
		if (pApp->IsAllDay () || pApp->IsMultiDay ())
		{
			rt.top    -= m_nHeaderAllDayHeight;
			rt.bottom = rt.top + m_nHeaderAllDayHeight;
		}

		CRect rtDraw (pApp->GetRectPrint (date));

		CRect rtInter;
		rtInter.IntersectRect (rtDraw, rt);

		pApp->SetVisiblePrint ((!bFullVisible && rtInter.Height () >= 2) || 
			(bFullVisible && rtInter.Height () == rtDraw.Height () && 
			 rtInter.bottom < rt.bottom), date);
	}
}

int CBCGPPlannerPrintDay::GetViewHours () const
{
	return GetLastViewHour () - GetFirstViewHour () + 1;
}

int CBCGPPlannerPrintDay::GetViewHourOffset () const
{
	return GetFirstViewHour () * 60 / CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ());
}

#endif // BCGP_EXCLUDE_PLANNER
