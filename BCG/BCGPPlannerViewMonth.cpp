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
// BCGPPlannerViewMonth.cpp: implementation of the CBCGPPlannerViewMonth class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPPlannerViewMonth.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerManagerCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef _BCGPCALENDAR_STANDALONE
	#include "BCGPVisualManager.h"
	#define visualManager	CBCGPVisualManager::GetInstance ()
#else
	#include "BCGPCalendarVisualManager.h"
	#define visualManager	CBCGPCalendarVisualManager::GetInstance ()
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CBCGPPlannerViewMonth, CBCGPPlannerView)

CBCGPPlannerViewMonth::CBCGPPlannerViewMonth()
	: CBCGPPlannerView   ()
	, m_nHeaderHeight    (0)
	, m_nWeekBarWidth    (0)
	, m_bDrawTimeFinish  (TRUE)
	, m_bDrawTimeAsIcons (FALSE)
	, m_bCompressWeekend (TRUE)
	, m_nDuration        (35)
{
	m_DateEnd = m_DateStart + COleDateTimeSpan (m_nDuration - 1, 23, 59, 59);
}

CBCGPPlannerViewMonth::~CBCGPPlannerViewMonth()
{
}

#ifdef _DEBUG
void CBCGPPlannerViewMonth::AssertValid() const
{
	CBCGPPlannerView::AssertValid();
}

void CBCGPPlannerViewMonth::Dump(CDumpContext& dc) const
{
	CBCGPPlannerView::Dump(dc);
}
#endif

void CBCGPPlannerViewMonth::GetCaptionFormatStrings (CStringArray& sa)
{
	sa.RemoveAll ();

	sa.Add (_T("dddd"));
	sa.Add (_T("ddd"));
}

void CBCGPPlannerViewMonth::AdjustScrollSizes ()
{
	if (m_nScrollPage != 5 && m_nScrollTotal != 250)
	{
		m_nScrollPage   = 5;
		m_nScrollTotal  = 250;
		m_nScrollOffset = m_nScrollTotal / 2;
	}

	CBCGPPlannerView::AdjustScrollSizes ();
}

void CBCGPPlannerViewMonth::AdjustLayout (CDC* /*pDC*/, const CRect& /*rectClient*/)
{
	m_nHeaderHeight = m_nRowHeight;
	m_nWeekBarWidth = 0;

	if ((GetPlanner ()->GetDrawFlags () & BCGP_PLANNER_DRAW_VIEW_WEEK_BAR) ==
			BCGP_PLANNER_DRAW_VIEW_WEEK_BAR)
	{
		m_nWeekBarWidth = m_nHeaderHeight + 3;
	}

	m_rectApps.left += m_nWeekBarWidth;

	AdjustScrollSizes ();

	m_rectApps.top += m_nHeaderHeight;
}

void CBCGPPlannerViewMonth::AdjustRects ()
{
	const int nDays = GetViewDuration ();

	const int nRows    = nDays / 7;
	const int dxColumn = CBCGPPlannerView::round (m_rectApps.Width () / (m_bCompressWeekend ? 6.0 : 7.0)) - 1;
	const int dxRow    = CBCGPPlannerView::round (m_rectApps.Height () / (double)nRows) - 1;

	const int nDayStart = (IsCompressWeekend () && CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () == 0)
		? 1
		: CBCGPPlannerManagerCtrl::GetFirstDayOfWeek ();

	if (m_ViewRects.GetSize () != nDays)
	{
		m_ViewRects.SetSize (nDays);
	}

	int iRow = 0;
	for (iRow = 0; iRow < nRows; iRow++)
	{
		CRect rect (m_rectApps);
		rect.right   = rect.left + dxColumn;
		rect.top    += iRow * (dxRow + 1);
		rect.bottom  = iRow == (nRows - 1) ? rect.bottom : rect.top + dxRow;

		int nTop    = rect.top;
		int nBottom = rect.bottom;

		int nColumn = 0;

		for (int iDay = 0; iDay < 7; iDay++)
		{
			int nWeekDay = nDayStart + iDay - 7;

			if (m_bCompressWeekend)
			{
				if (nWeekDay == -1)
				{
					rect.bottom = rect.top + dxRow / 2;
				}
				else if (nWeekDay == 0)
				{
					rect.top = rect.bottom + 1;
					rect.bottom = nBottom;
				}
				else
				{
					rect.top = nTop;
				}
			}

			m_ViewRects[iRow * 7 + iDay] = rect;

			if ((m_bCompressWeekend && nWeekDay != -1) || !m_bCompressWeekend)
			{
				rect.OffsetRect (dxColumn + 1, 0);

				nColumn++;

				if ((m_bCompressWeekend && nColumn == 5) ||
					(!m_bCompressWeekend && nColumn == 6))
				{
					rect.right = m_rectApps.right;
				}
			}
		}
	}

	if (m_nWeekBarWidth > 0)
	{
		int nIndex = 0;
		if (IsCompressWeekend () && m_ViewRects[0].top != m_ViewRects[1].top)
		{
			nIndex = 2;
		}

		m_WeekRects.SetSize (nRows);

		for (iRow = 0; iRow < nRows; iRow++)
		{
			CRect rect (m_ViewRects[nIndex]);
			rect.right = m_rectApps.left;
			rect.left  = rect.right - m_nRowHeight;

			rect.top    += m_nRowHeight;
			rect.InflateRect (0, 1);

			m_WeekRects[iRow] = rect;

			nIndex += 7;
		}
	}
}

void CBCGPPlannerViewMonth::AdjustAppointments ()
{
	XBCGPAppointmentArray& arQueryApps = GetQueryedAppointments ();
	XBCGPAppointmentArray& arDragApps = GetDragedAppointments ();

	const int nDays = GetViewDuration ();

	if ((arQueryApps.GetSize () == 0 && arDragApps.GetSize () == 0) || 
		m_ViewRects.GetSize () != nDays)
	{
		ClearVisibleUpDownIcons ();		
		return;
	}

	BOOL bDragDrop        = IsDragDrop ();
	DROPEFFECT dragEffect = GetDragEffect ();
	BOOL bDragMatch       = IsCaptureMatched ();

	bDragDrop = !bDragDrop || 
		(bDragDrop && ((dragEffect & DROPEFFECT_COPY) == DROPEFFECT_COPY && bDragMatch) || 
		!bDragMatch);
	bDragDrop = bDragDrop && arDragApps.GetSize ();
	
	int nApp = 0;

	for (nApp = 0; nApp < 2; nApp++)
	{
		if (!bDragDrop && nApp == 0)
		{
			continue;
		}

		XBCGPAppointmentArray& arApps = nApp == 0 ? arDragApps : arQueryApps;

		for (int i = 0; i < arApps.GetSize (); i++)
		{
			CBCGPAppointment* pApp = arApps[i];
			ASSERT_VALID (pApp);

			pApp->ResetDraw ();
		}
	}

	COleDateTime date (m_DateStart);

	COleDateTimeSpan spanDay (1, 0, 0, 0);
	const int delta = m_nRowHeight + 2;

	XBCGPAppointmentArray arAllOrMulti;

	for (nApp = 0; nApp < 2; nApp++)
	{
		if (!bDragDrop && nApp == 1)
		{
			continue;
		}

		XBCGPAppointmentArray& arApps = nApp == 1 ? arDragApps : arQueryApps;

		for (int i = 0; i < arApps.GetSize (); i++)
		{
			CBCGPAppointment* pApp = arApps[i];
			ASSERT_VALID (pApp);

			if (pApp->IsAllDay () || pApp->IsMultiDay ())
			{
				arAllOrMulti.Add (pApp);
			}
		}
	}

	if (arAllOrMulti.GetSize () > 0)
	{
		SortAppointments (arAllOrMulti, (int) arAllOrMulti.GetSize ());
	}

	for (int nDay = 0; nDay < nDays; nDay ++)
	{
		CRect rect (m_ViewRects [nDay]);
		rect.top += m_nRowHeight + 1;
		rect.DeflateRect (1, 0);

		BOOL bTopEq    = TRUE;
		BOOL bBottomEq = TRUE;
		
		if (nDay > 0)
		{
			bTopEq    = m_ViewRects [nDay].top == m_ViewRects [nDay - 1].top;
			bBottomEq = m_ViewRects [nDay].bottom == m_ViewRects [nDay - 1].bottom;
		}

		CList<int, int> lsItems;

		// first draw all days or multi days
		for (int n = 0; n < 2; n++)
		{
			for (nApp = 0; nApp < 2; nApp++)
			{
				if (n == 0 && nApp == 1)
				{
					continue;
				}

				if (!bDragDrop)
				{
					if(n == 1 && nApp == 0)
					{
						continue;
					}
				}

				XBCGPAppointmentArray& arApps = 
					n == 0
					? arAllOrMulti
					: (nApp == 0 ? arDragApps : arQueryApps);

				for (int i = 0; i < arApps.GetSize (); i++)
				{
					CBCGPAppointment* pApp = arApps[i];
					ASSERT_VALID (pApp);

					BOOL bAllOrMulti = pApp->IsAllDay () || pApp->IsMultiDay ();

					if (CBCGPPlannerView::IsAppointmentInDate (*pApp, date) &&
						((n == 0 && bAllOrMulti) || (n == 1 && !bAllOrMulti)))
					{
						CRect rtApp (rect);

						int nItem = 0;

						while(TRUE)
						{
							POSITION pos = lsItems.Find (nItem);
							if (pos != NULL)
							{
								nItem++;
							}
							else
							{
								break;
							}
						}

						rtApp.top += nItem * delta;
						rtApp.bottom = rtApp.top + m_nRowHeight;

						// check for add new rect
						if (nDay > 0)
						{
							if (bTopEq && bAllOrMulti)
							{
								if (!pApp->GetDSDraw ().IsEmpty ())
								{
									CRect rt;
									rt = pApp->GetRectDraw (date - spanDay);

									if (!rt.IsRectEmpty () && rtApp.top != rt.top)
									{
										rtApp.top    = rt.top;
										rtApp.bottom = rt.bottom;
									}
								}
							}
						}

						if (((bTopEq && !bBottomEq) ||
							 (!bAllOrMulti && 
							  !IsOneDay (pApp->GetStart (), pApp->GetFinish ()))) &&
							!pApp->GetDSDraw ().IsEmpty ())
						{
							CRect rtInter;
							rtInter.IntersectRect (rtApp, m_ViewRects [nDay]);

							if (rtInter.Height () < rtApp.Height () || 
								rtInter.bottom >= rect.bottom || !bAllOrMulti)
							{
								// add impossible rect
								rtApp.OffsetRect (0, 1);
								pApp->SetRectDraw (rtApp, date);
								// return rect
								rtApp.OffsetRect (0, -1);
							}
						}

						pApp->SetRectDraw (rtApp, date);

						lsItems.AddTail ((rtApp.top - rect.top) / delta);
					}
				}
			}
		}

		CheckVisibleAppointments(date, rect, TRUE);

		date += spanDay;
	}

	CheckVisibleUpDownIcons (TRUE);

	m_bUpdateToolTipInfo = TRUE;
}

void CBCGPPlannerViewMonth::AddUpDownRect(BYTE nType, const CRect& rect)
{
	if (nType != 0)
	{
		CSize sz (GetPlanner ()->GetUpDownIconSize ());
		CRect rt (CPoint(rect.right - sz.cx, rect.bottom - sz.cy), sz);

		m_DownRects.Add (rt);
	}
}

void CBCGPPlannerViewMonth::DrawHeader (CDC* pDC, const CRect& rect, int dxColumn)
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
		rt.OffsetRect (dxColumn + 1, 0);
	}
}

void CBCGPPlannerViewMonth::OnPaint (CDC* pDC, const CRect& rectClient)
{
	ASSERT_VALID (pDC);

	if (m_nWeekBarWidth != 0)
	{
		CRect rtBar (rectClient);
		rtBar.right  = rtBar.left + m_nWeekBarWidth;

		OnDrawWeekBar (pDC, rtBar);
	}

	OnDrawClient (pDC, m_rectApps);

	if (m_nHeaderHeight != 0)
	{
		CRect rtHeader (rectClient);
		rtHeader.left   = m_rectApps.left;
		rtHeader.bottom = rtHeader.top + m_nHeaderHeight;

		OnDrawHeader (pDC, rtHeader);
	}

	OnDrawAppointments (pDC);

	OnDrawUpDownIcons (pDC);

	InitToolTipInfo ();
}

void CBCGPPlannerViewMonth::OnDrawClient (CDC* pDC, const CRect& rect)
{
	ASSERT_VALID (pDC);

	CRect rectFill (rect);

	int nMonth = m_Date.GetMonth ();

	BOOL bIsWorking = nMonth % 2;

	const int nRows = GetViewDuration () / 7;

	COleDateTime day (GetDateStart ());

	COleDateTime dayCurrent = COleDateTime::GetCurrentTime ();
	dayCurrent.SetDateTime (dayCurrent.GetYear (), dayCurrent.GetMonth (), 
		dayCurrent.GetDay (), 0, 0, 0);

	int iRow = 0;

	int nStartColumn = 1;
	if (m_nWeekBarWidth != 0)
	{
		nStartColumn = 0;
	}

	{
		CPen penBlack (PS_SOLID, 0, visualManager->GetPlannerSeparatorColor (this));
		CPen* pOldPen = pDC->SelectObject (&penBlack);

		const int nEnd = 7;

		int nCol = 0;
		int iColumn = 1;

		for (iColumn = 1; iColumn < 7; iColumn++)
		{
			if (m_ViewRects [iColumn - 1].right == m_ViewRects [iColumn].right)
			{
				nCol = iColumn - 1;
				break;
			}
		}

		for (iColumn = nStartColumn; iColumn < nEnd; iColumn++)
		{
			pDC->MoveTo (m_ViewRects [iColumn].left - 1, rect.top);
			pDC->LineTo (m_ViewRects [iColumn].left - 1, rect.bottom);
		}

		for (iRow = 0; iRow < nRows; iRow++)
		{
			int nIndex = iRow * 7 + 6;

			pDC->MoveTo (rect.left , m_ViewRects [nIndex].bottom);
			pDC->LineTo (rect.right, m_ViewRects [nIndex].bottom);

			if (m_bCompressWeekend)
			{
				nIndex -= (6 - nCol);
				pDC->MoveTo (m_ViewRects [nIndex].left, m_ViewRects [nIndex].bottom);
				pDC->LineTo (m_ViewRects [nIndex].right, m_ViewRects [nIndex].bottom);
			}
		}

		pDC->SelectObject (pOldPen);
	}

	DWORD dwFlags = GetPlanner ()->GetDrawFlags ();
	BOOL bBold = (dwFlags & BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_BOLD) ==
			BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_BOLD;
	BOOL bCompact = (dwFlags & BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_COMPACT) ==
			BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_COMPACT;

	HFONT hOldFont = NULL;
	if (bBold)
	{
		hOldFont = SetCurrFont (pDC, bBold);
	}

	const BOOL bDateBeforeMonth = CBCGPPlannerView::IsDateBeforeMonth ();

	for (iRow = 0; iRow < nRows; iRow++)
	{
		for (int iDay = 0; iDay < 7; iDay++)
		{
			int nDay = iRow * 7 + iDay;

			CRect rectDayCaption (m_ViewRects [nDay]);

			BOOL bToday = day == dayCurrent;
			BOOL bSelected = IsDateInSelection (day);

			bIsWorking = day.GetMonth () % 2;

			visualManager->PreparePlannerBackItem (bToday, bSelected);
			OnFillPlanner (pDC, rectDayCaption, bIsWorking);

			rectDayCaption.bottom = rectDayCaption.top + m_nRowHeight + 1;

			CString strFormat (_T("d"));
			CString strDate;

			BOOL bNewYear = FALSE;

			if (!bCompact)
			{
				if ((iRow == 0 && iDay == 0) || day.GetDay () == 1)
				{
					if (bDateBeforeMonth)
					{
						strFormat = _T("d MMMM");
					}
					else
					{
						strFormat = _T("MMMM d");
					}

					if (day.GetDay () == 1 && day.GetMonth () == 1)
					{
						bNewYear = TRUE;
						strFormat += _T(" yyyy");
					}			
				}

				if (!strFormat.IsEmpty ())
				{
					strDate.GetBuffer (_MAX_PATH);

					SYSTEMTIME st;
					day.GetAsSystemTime (st);

					::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st, 
						strFormat, (LPTSTR)(LPCTSTR)strDate, _MAX_PATH);

					strDate.ReleaseBuffer ();

					CSize szSize (pDC->GetTextExtent (strDate));

					if (rectDayCaption.Width () - 4 < szSize.cx)
					{
						if (bDateBeforeMonth)
						{
							strFormat = _T("d MMM");
						}
						else
						{
							strFormat = _T("MMM d");
						}

						if (bNewYear)
						{
							strFormat += _T(" yy");
						}

						strDate.GetBuffer (_MAX_PATH);

						::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st, 
							strFormat, (LPTSTR)(LPCTSTR)strDate, _MAX_PATH);

						strDate.ReleaseBuffer ();
					}
				}
			}
			else
			{
				if ((iRow == 0 && iDay == 0) || day.GetDay () == 1)
				{
					if (bDateBeforeMonth)
					{
						strFormat = _T("d MMM");
					}
					else
					{
						strFormat = _T("MMM d");
					}

					if (day.GetDay () == 1 && day.GetMonth () == 1)
					{
						bNewYear = TRUE;
						strFormat += _T(" yy");
					}
				}

				if (!strFormat.IsEmpty ())
				{
					strDate.GetBuffer (_MAX_PATH);

					SYSTEMTIME st;
					day.GetAsSystemTime (st);

					::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st, 
						strFormat, (LPTSTR)(LPCTSTR)strDate, _MAX_PATH);

					strDate.ReleaseBuffer ();

					CSize szSize (pDC->GetTextExtent (strDate));

					if (rectDayCaption.Width () - 4 < szSize.cx)
					{
						strFormat = _T("d");
						BOOL bNeedFormat = TRUE;

						if (bNewYear)
						{
							if (bDateBeforeMonth)
							{
								strFormat += _T(" MMM");
							}
							else
							{
								strFormat = _T("MMM ") + strFormat;
							}

							bNeedFormat = FALSE;

							strDate.GetBuffer (_MAX_PATH);

							::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st, 
								strFormat, (LPTSTR)(LPCTSTR)strDate, _MAX_PATH);

							strDate.ReleaseBuffer ();

							szSize = pDC->GetTextExtent (strDate);

							if (rectDayCaption.Width () - 4 < szSize.cx)
							{
								strFormat = _T("d");
								bNeedFormat = TRUE;
							}
						}

						if (bNeedFormat)
						{
							strDate.GetBuffer (_MAX_PATH);

							::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st, 
								strFormat, (LPTSTR)(LPCTSTR)strDate, _MAX_PATH);

							strDate.ReleaseBuffer ();
						}
					}
				}
			}

			if (bToday)
			{
				rectDayCaption.bottom--;
			}

			visualManager->PreparePlannerCaptionBackItem (FALSE);
			COLORREF clrText = OnFillPlannerCaption (
				pDC, rectDayCaption, bToday, bSelected, FALSE);

			DrawCaptionText (pDC, rectDayCaption, strDate, clrText, bCompact ? DT_LEFT : DT_RIGHT,
				bToday && bSelected);

			day += COleDateTimeSpan (1, 0, 0, 0);
		}
	}

	if (hOldFont != NULL)
	{
		::SelectObject (pDC->GetSafeHdc (), hOldFont);
	}
}

void CBCGPPlannerViewMonth::OnDrawHeader (CDC* pDC, const CRect& rectHeader)
{
	ASSERT_VALID (pDC);

	const int dxColumn = m_ViewRects [0].Width ();

	CRect rectDayCaption (rectHeader);
	
	DrawHeader (pDC, rectDayCaption, dxColumn);

	rectDayCaption.right = rectDayCaption.left + dxColumn;

	COleDateTime day 
		(
			GetFirstWeekDay2 (m_Date, CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () + 1)
		);

	const int nEnd = m_bCompressWeekend ? 6 : 7;

	CStringArray sa;
	sa.SetSize (nEnd);

	int iColumn = 0;

	for (iColumn = 0; iColumn < nEnd; iColumn++)
	{
		CString strDate;

		if (IsCompressWeekend () && day.GetDayOfWeek () == 7)
		{
			for (int i = 0; i < 2; i++)
			{
				CString strTemp;
				strTemp.GetBuffer (_MAX_PATH);

				SYSTEMTIME st;
				day.GetAsSystemTime (st);

				::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st, 
					_T("ddd"), (LPTSTR)(LPCTSTR)strTemp, _MAX_PATH);

				strTemp.ReleaseBuffer ();

				strDate += strTemp;

				if (i == 0)
				{
					strDate += _T("/");
				}

				day += COleDateTimeSpan (1, 0, 0, 0);
			}
		}
		else
		{
			strDate.GetBuffer (_MAX_PATH);

			SYSTEMTIME st;
			day.GetAsSystemTime (st);

			::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st, 
				m_strCaptionFormat, (LPTSTR)(LPCTSTR)strDate, _MAX_PATH);

			strDate.ReleaseBuffer ();

			day += COleDateTimeSpan (1, 0, 0, 0);
		}

		sa.SetAt (iColumn, strDate);
	}

	for (iColumn = 0; iColumn < nEnd; iColumn++)
	{
		// Draw caption bar (on top):
		visualManager->PreparePlannerCaptionBackItem (TRUE);
		COLORREF clrText = OnFillPlannerCaption (
			pDC, rectDayCaption, FALSE, FALSE, FALSE);
		DrawCaptionText (pDC, rectDayCaption, sa[iColumn], clrText);

		rectDayCaption.OffsetRect (dxColumn + 1, 0);
	}
}

void CBCGPPlannerViewMonth::OnDrawWeekBar (CDC* pDC, const CRect& rectBar)
{
	if (rectBar.IsRectEmpty ())
	{
		return;
	}

	ASSERT_VALID (pDC);

	visualManager->PreparePlannerCaptionBackItem (FALSE);
	visualManager->OnFillPlannerWeekBar (pDC, this, rectBar);

	COLORREF clrLine = visualManager->GetPlannerSeparatorColor (this);

	const int nRows = GetViewDuration () / 7;

	const BOOL bDateBeforeMonth = CBCGPPlannerView::IsDateBeforeMonth ();

	CStringArray saFormat;
	{
		CString strMonthFmt (_T("MMM"));
		CString strSep (_T(" "));

		HFONT hOldFont = (HFONT)::SelectObject (pDC->GetSafeHdc (), GetFont ());

		CSize szText (pDC->GetTextExtent (_T("AAA 00 - AAA 00")));

		CRect rect (m_WeekRects[0]);
		if (rect.Height() < (szText.cx * 1.5))
		{
			strMonthFmt = _T("M");
			strSep      = CBCGPPlannerView::GetDateSeparator ();
		}

		if (hOldFont != NULL)
		{
			::SelectObject (pDC->GetSafeHdc (), hOldFont);
		}

		CString strFormat;
		if (bDateBeforeMonth)
		{
			strFormat = _T("d") + strSep + strMonthFmt;
			saFormat.Add (_T("d"));
			saFormat.Add (strFormat);
			saFormat.Add (strFormat);
		}
		else
		{
			strFormat = strMonthFmt + strSep + _T("d");
			saFormat.Add (strFormat);
			saFormat.Add (_T("d"));
			saFormat.Add (strFormat);
		}
	}

	COLORREF clrTextOld = pDC->GetTextColor ();
	HFONT hOldFont = (HFONT)::SelectObject (pDC->GetSafeHdc (), GetFontVert ());

	COleDateTime day1 (GetDateStart ());
	COleDateTime day2 (day1);
	day2 += COleDateTimeSpan (6, 0, 0, 0);

	COleDateTimeSpan span (7, 0, 0, 0);

	for (int iRow = 0; iRow < nRows; iRow++)
	{
		CRect rect (m_WeekRects[iRow]);

		if (rect.top < rect.bottom)
		{
			COLORREF clrText = 
				visualManager->OnFillPlannerCaption (pDC, this, rect, FALSE, FALSE, TRUE, FALSE);

			pDC->SetTextColor (clrText);

			pDC->Draw3dRect (rect, clrLine, clrLine);

			rect.DeflateRect (1, 1);

			SYSTEMTIME st1;
			day1.GetAsSystemTime (st1);
			SYSTEMTIME st2;
			day2.GetAsSystemTime (st2);

			CString strDate1;
			CString strDate2;

			if (day1.GetMonth () == day2.GetMonth ())
			{
				strDate1.GetBuffer (_MAX_PATH);
				::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st1, 
					saFormat[0], (LPTSTR)(LPCTSTR)strDate1, _MAX_PATH);
				strDate1.ReleaseBuffer ();

				strDate2.GetBuffer (_MAX_PATH);
				::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st2, 
					saFormat[1], (LPTSTR)(LPCTSTR)strDate2, _MAX_PATH);
				strDate2.ReleaseBuffer ();
			}
			else
			{
				strDate1.GetBuffer (_MAX_PATH);
				::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st1, 
					saFormat[2], (LPTSTR)(LPCTSTR)strDate1, _MAX_PATH);
				strDate1.ReleaseBuffer ();

				strDate2.GetBuffer (_MAX_PATH);
				::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st2, 
					saFormat[2], (LPTSTR)(LPCTSTR)strDate2, _MAX_PATH);
				strDate2.ReleaseBuffer ();
			}

			CString strText = strDate1 + _T(" - ") + strDate2;

			CSize szText (pDC->GetTextExtent (strText));

			CRect rectText (rect);
			rectText.left = rectText.left + (rect.Width () - szText.cy + 1) / 2;

			rectText.top = rectText.bottom;
			if (szText.cx < rect.Height ())
			{
				rectText.top -= (rect.Height () - szText.cx + 1) / 2;
			}
			rectText.bottom = rect.top;

			pDC->DrawText (strText, rectText, DT_SINGLELINE);
		}

		day1 += span;
		day2 += span;
	}

	pDC->SetTextColor (clrTextOld);
	if (hOldFont != NULL)
	{
		::SelectObject (pDC->GetSafeHdc (), hOldFont);
	}
}

BOOL CBCGPPlannerViewMonth::OnScroll(UINT /*nScrollCode*/, UINT /*nPos*/, BOOL /*bDoScroll*/)
{
	return FALSE;
}

BOOL CBCGPPlannerViewMonth::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int nPrevOffset = m_nScrollOffset;

	switch (nSBCode)
	{
	case SB_LINEUP:
		m_nScrollOffset -= 1;
		break;

	case SB_LINEDOWN:
		m_nScrollOffset += 1;
		break;

	case SB_TOP:
		m_nScrollOffset = 0;
		break;

	case SB_BOTTOM:
		m_nScrollOffset = m_nScrollTotal;
		break;

	case SB_PAGEUP:
		m_nScrollOffset -= m_nScrollPage;
		break;

	case SB_PAGEDOWN:
		m_nScrollOffset += m_nScrollPage;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_nScrollOffset = nPos;
		break;

	default:
		return FALSE;
	}

	if (m_nScrollOffset == nPrevOffset)
	{
		return FALSE;
	}

	int nOffset = m_nScrollOffset - nPrevOffset;

	COleDateTimeSpan span = COleDateTimeSpan (nOffset * 7, 0, 0, 0);

	m_Date      += span;
	m_DateStart += span;
	m_DateEnd   += span;

	SetSelection (m_Selection [0] + span, m_Selection [1] + span);

	QueryAppointments ();

	return CBCGPPlannerView::OnVScroll (nSBCode, nPos, pScrollBar);
}

BOOL CBCGPPlannerViewMonth::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	if (m_pCapturedAppointment != NULL)
	{
		GetPlanner ()->SendMessage (WM_CANCELMODE, 0, 0);
		return TRUE;
	}

	BOOL isShiftPressed = (0x8000 & GetKeyState(VK_SHIFT)) != 0;

	BOOL bHandled = FALSE;

	if (nChar == VK_PRIOR || nChar == VK_NEXT) // Page Up, Page Down
	{
		OnVScroll (nChar == VK_PRIOR ? SB_PAGEUP : SB_PAGEDOWN, 0, NULL);

		SetSelection (m_Date, m_Date);

		bHandled = TRUE;
	}
	else if (nChar == VK_HOME || nChar == VK_END)
	{
		COleDateTime date
			(
				GetFirstWeekDay2 (m_Date, CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () + 1)
			);

		if (nChar == VK_END)
		{
			date += COleDateTimeSpan (6, 0, 0, 0);
		}

		if (!isShiftPressed)
		{
			m_Date = date;
		}
		else
		{
			if (nChar == VK_HOME)
			{
				COleDateTime oldDate (m_Date);

				m_Date = date;
				date   = oldDate;
			}
			else
			{
				m_Date = m_Selection[1];
			}
		}

		SetSelection (m_Date, date);

		bHandled = TRUE;
	}
	else if (nChar == VK_UP || nChar == VK_DOWN || nChar == VK_LEFT || nChar == VK_RIGHT)
	{
		COleDateTime date (m_Selection [1]);

		COleDateTimeSpan span (1, 0, 0 ,0);

		if (nChar == VK_LEFT)
		{
			date -= span;
		}
		else if (nChar == VK_RIGHT)
		{
			date += span;
		}
		else
		{
			span = COleDateTimeSpan (7, 0, 0 ,0);

			if (nChar == VK_UP)
			{
				date -= span;
			}
			else if (nChar == VK_DOWN)
			{
				date += span;
			}
		}

		BOOL bScroll = FALSE;

		if (date < m_DateStart)
		{
			OnVScroll (SB_LINEUP, 0, NULL);
			bScroll = TRUE;
		}
		else if (date >= (m_DateStart + COleDateTimeSpan (GetViewDuration (), 0, 0, 0)))
		{
			OnVScroll (SB_LINEDOWN, 0, NULL);
			bScroll = TRUE;
		}

		if (!isShiftPressed || bScroll)
		{
			m_Date = date;
			SetSelection (m_Date, m_Date);
		}
		else
		{
			SetSelection (m_Selection [0], date);

			if (date <= m_Date)
			{
				m_Date = date;
			}
		}

		bHandled = TRUE;
	}

	return bHandled;
}

COleDateTime CBCGPPlannerViewMonth::GetDateFromPoint (const CPoint& point) const
{
	return CBCGPPlannerView::GetDateFromPoint (point);
}

void CBCGPPlannerViewMonth::OnActivate(CBCGPPlannerManagerCtrl* pPlanner, const CBCGPPlannerView* pOldView)
{
	ASSERT_VALID(pPlanner);

	if (pOldView != NULL)
	{
		m_Date = pOldView->GetDate ();
	}

	m_nDuration = 35;
	m_DateStart = CalculateDateStart (
		COleDateTime(m_Date.GetYear (), m_Date.GetMonth (), m_Date.GetDay (), 0, 0, 0));
	m_DateEnd   = m_DateStart + COleDateTimeSpan (m_nDuration - 1, 23, 59, 59);

	if (m_Date > m_DateEnd || m_Date < m_DateStart)
	{
		m_Date = m_DateStart;
	}

	COleDateTime sel1 (m_Date);
	COleDateTime sel2 (m_Date);

	if (pOldView != NULL)
	{
		sel1 = pOldView->GetSelectionStart ();
		sel2 = pOldView->GetSelectionEnd ();
	}

	SetSelection (sel1, sel2, FALSE);

	CBCGPPlannerView::OnActivate (pPlanner, NULL);
}

void CBCGPPlannerViewMonth::SetSelection (const COleDateTime& sel1, const COleDateTime& sel2, BOOL bRedraw)
{
	COleDateTime s1 (sel1);
	COleDateTime s2 (sel2);

	if (s1 < s2 || IsOneDay (s1, s2))
	{
		s1 = COleDateTime (s1.GetYear (), s1.GetMonth (), s1.GetDay (), 0, 0, 0);
		s2 = COleDateTime (s2.GetYear (), s2.GetMonth (), s2.GetDay (), 23, 59, 59);
	}
	else
	{
		s2 = COleDateTime (s2.GetYear (), s2.GetMonth (), s2.GetDay (), 0, 0, 0);
		s1 = COleDateTime (s1.GetYear (), s1.GetMonth (), s1.GetDay (), 23, 59, 59);
	}

	CBCGPPlannerView::SetSelection (s1, s2, bRedraw);
}

void CBCGPPlannerViewMonth::SetDate (const COleDateTime& date)
{
	COleDateTime dt (date.GetYear (), date.GetMonth (), date.GetDay (), 0, 0, 0);

	if (m_DateEnd < dt || dt < m_DateStart)
	{
		m_DateStart = GetFirstWeekDay2 (dt, CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () + 1);
		m_DateEnd   = m_DateStart + COleDateTimeSpan (m_nDuration - 1, 23, 59, 59);
	}
	
	if (m_DateEnd < dt || dt < m_DateStart)
	{
		dt = m_DateStart;
	}

	CBCGPPlannerView::SetDate (dt);
}

void CBCGPPlannerViewMonth::SetDateInterval (const COleDateTime& date1, const COleDateTime& date2)
{
	ASSERT (date1 <= date2);

	COleDateTimeSpan duration (date2 - date1);

	m_nDuration = ((int)(duration.GetTotalDays () / 7.0)) * 7;
	if (m_nDuration < duration.GetTotalDays ())
	{
		m_nDuration += 7;
	}

	if (m_nDuration < 14)
	{
		m_nDuration = 14;
	}
	else if (m_nDuration > 42)
	{
		m_nDuration = 42;
	}

	COleDateTime _date1 (date1);

	if (IsCompressWeekend () &&
		CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () == 0)
	{
		_date1 += COleDateTimeSpan (1, 0, 0, 0);
	}

	m_DateStart = GetFirstWeekDay2 (_date1, CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () + 1);
	m_DateEnd   = m_DateStart + COleDateTimeSpan (m_nDuration - 1, 23, 59, 59);

	if (m_DateEnd < m_Date || m_Date < m_DateStart)
	{
		m_Date = m_DateStart;
	}

	SetDate (m_Date);
}

COleDateTime CBCGPPlannerViewMonth::GetFirstWeekDay2 (const COleDateTime& date, int nWeekStart) const
{
	return CBCGPPlannerView::GetFirstWeekDay (date, 
		(IsCompressWeekend () && nWeekStart == 1) ? 2 : nWeekStart);
}

COleDateTime CBCGPPlannerViewMonth::CalculateDateStart (const COleDateTime& date) const
{
	return GetFirstWeekDay2 (COleDateTime (date.GetYear (), 
				date.GetMonth (), 1, 0, 0, 0), CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () + 1);
}

void CBCGPPlannerViewMonth::SetDrawTimeFinish (BOOL bDraw)
{
	if (m_bDrawTimeFinish != bDraw)
	{
		m_bDrawTimeFinish = bDraw;

		CBCGPPlannerView::AdjustLayout ();
	}
}

BOOL CBCGPPlannerViewMonth::IsDrawTimeFinish () const
{
	return m_bDrawTimeFinish;
}

void CBCGPPlannerViewMonth::SetDrawTimeAsIcons (BOOL bDraw)
{
	if (m_bDrawTimeAsIcons != bDraw)
	{
		m_bDrawTimeAsIcons = bDraw;

		CBCGPPlannerView::AdjustLayout ();
	}
}

BOOL CBCGPPlannerViewMonth::IsDrawTimeAsIcons () const
{
	return m_bDrawTimeAsIcons;
}

void CBCGPPlannerViewMonth::SetCompressWeekend (BOOL bCompress)
{
	if (m_bCompressWeekend != bCompress)
	{
		m_bCompressWeekend = bCompress;

		if (CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () != 1)
		{
			m_DateStart = GetFirstWeekDay2 (m_DateStart, CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () + 1);
			m_DateEnd   = m_DateStart + COleDateTimeSpan (m_nDuration - 1, 23, 59, 59);
			SetDate (m_Date);
		}
		else
		{
			CBCGPPlannerView::AdjustLayout ();
		}
	}
}

BOOL CBCGPPlannerViewMonth::IsCompressWeekend () const
{
	return m_bCompressWeekend;
}

BOOL CBCGPPlannerViewMonth::CanCaptureAppointment (BCGP_PLANNER_HITTEST hitCapturedAppointment) const
{
	if (IsReadOnly ())
	{
		return FALSE;
	}

	return hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_LEFT ||
		   hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_RIGHT;
}

CBCGPPlannerView::BCGP_PLANNER_HITTEST
CBCGPPlannerViewMonth::HitTestArea (const CPoint& point) const
{
	BCGP_PLANNER_HITTEST hit = CBCGPPlannerView::HitTestArea (point);

	if (hit == BCGP_PLANNER_HITTEST_CLIENT)
	{
		const int nDuration = GetViewDuration ();

		for (int i = 0; i < nDuration; i++)
		{
			CRect rt (m_ViewRects[i]);
			rt.right++;
			rt.bottom++;

			if (rt.PtInRect (point))
			{
				rt.bottom = rt.top + min(rt.Height (), m_nRowHeight);

				if (rt.PtInRect (point))
				{
					hit = BCGP_PLANNER_HITTEST_DAY_CAPTION;
				}

				break;
			}
		}
	}
	else if (hit == BCGP_PLANNER_HITTEST_NOWHERE)
	{
		if (m_rectApps.left <= point.x && point.x <= m_rectApps.right)
		{
			if (0 <= point.y && point.y < m_nHeaderHeight)
			{
				hit = BCGP_PLANNER_HITTEST_HEADER;
			}
		}
		else if (0 <= point.x && point.x < m_nWeekBarWidth)
		{
			hit = BCGP_PLANNER_HITTEST_WEEKBAR;

			for (int iRow = 0; iRow < m_WeekRects.GetSize (); iRow++)
			{
				if (m_WeekRects[iRow].PtInRect (point))
				{
					hit = BCGP_PLANNER_HITTEST_WEEK_CAPTION;
					break;
				}
			}
		}
	}
		
	return hit;
}

DROPEFFECT CBCGPPlannerViewMonth::HitTestDrag (DWORD dwKeyState, const CPoint& point) const
{
	BCGP_PLANNER_HITTEST hit = HitTestArea (point);

	if (hit == BCGP_PLANNER_HITTEST_NOWHERE || 
		hit == BCGP_PLANNER_HITTEST_HEADER)
	{
		return DROPEFFECT_NONE;
	}

	return (dwKeyState & MK_CONTROL) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;	
}

int CBCGPPlannerViewMonth::GetWeekFromPoint (const CPoint& point) const
{
	int nWeek = -1;

	if (0 <= point.x && point.x < m_nWeekBarWidth)
	{
		for (int iRow = 0; iRow < m_WeekRects.GetSize (); iRow++)
		{
			if (m_WeekRects[iRow].PtInRect (point))
			{
				nWeek = iRow;
				break;
			}
		}
	}

	return nWeek;
}

CString CBCGPPlannerViewMonth::GetAccName() const
{
	return _T("Month View");
}

#endif // BCGP_EXCLUDE_PLANNER
