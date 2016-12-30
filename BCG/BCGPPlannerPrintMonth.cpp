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
// BCGPPlannerPrintMonth.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPPlannerPrintMonth.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerManagerCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerPrintMonth

IMPLEMENT_DYNCREATE(CBCGPPlannerPrintMonth, CBCGPPlannerPrint)

CBCGPPlannerPrintMonth::CBCGPPlannerPrintMonth()
	: CBCGPPlannerPrint  ()
	, m_bDrawTimeFinish  (TRUE)
	, m_bCompressWeekend (TRUE)
	, m_nWeekBarWidth    (0)
{
}

CBCGPPlannerPrintMonth::~CBCGPPlannerPrintMonth()
{
}

#ifdef _DEBUG
void CBCGPPlannerPrintMonth::AssertValid() const
{
	CObject::AssertValid();
}

void CBCGPPlannerPrintMonth::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif

CString CBCGPPlannerPrintMonth::GetPageHeaderText () const
{
	CString strText;

	SYSTEMTIME st;
	CString str;

	COleDateTime dtStart (GetDateStart ());
	COleDateTime dtEnd (GetDateEnd ());

	if ((dtStart + COleDateTimeSpan (7, 0, 0, 0)).GetMonth () != 
		dtStart.GetMonth ())
	{
		dtStart += COleDateTimeSpan (7, 0, 0, 0);
	}

	if ((dtEnd - COleDateTimeSpan (7, 0, 0, 0)).GetMonth () != 
		 dtEnd.GetMonth ())
	{
		dtEnd -= COleDateTimeSpan (7, 0, 0, 0);
	}

	dtStart.GetAsSystemTime (st);
	::GetDateFormat
		(
			LOCALE_USER_DEFAULT,
			0,
			&st,
			_T("MMMM yyyy"),
			str.GetBuffer (100),
			100
		);
	str.ReleaseBuffer ();

	strText = str;

	if (dtStart.GetMonth () != dtEnd.GetMonth ())
	{
		dtEnd.GetAsSystemTime (st);
		::GetDateFormat
			(
				LOCALE_USER_DEFAULT,
				0,
				&st,
				_T("MMMM yyyy"),
				str.GetBuffer (100),
				100
			);
		str.ReleaseBuffer ();

		strText += _T(" -\r");
		strText += str;
	}


	return strText;
}

void CBCGPPlannerPrintMonth::GetCaptionFormatStrings (CStringArray& sa)
{
	sa.RemoveAll ();

	sa.Add (_T("dddd"));
	sa.Add (_T("ddd"));
}

void CBCGPPlannerPrintMonth::AdjustLayout (CDC* /*pDC*/, const CRect& /*rectClient*/)
{
	m_nHeaderHeight = m_nRowHeight;

	m_nWeekBarWidth = 0;

	if ((GetDrawFlags () & BCGP_PLANNER_DRAW_VIEW_WEEK_BAR) ==
			BCGP_PLANNER_DRAW_VIEW_WEEK_BAR)
	{
		m_nWeekBarWidth = m_nHeaderHeight;
	}

	m_rectApps.left += m_nWeekBarWidth;
	m_rectApps.top += m_nHeaderHeight;
}

void CBCGPPlannerPrintMonth::AdjustRects ()
{
	const int nDays = GetViewDuration ();

	const int nRows    = nDays / 7;
	const int dxColumn = CBCGPPlannerView::round (m_rectApps.Width () / (m_bCompressWeekend ? 6.0 : 7.0)) - m_OnePoint.cx;
	const int dxRow    = CBCGPPlannerView::round (m_rectApps.Height () / (double)nRows) - m_OnePoint.cy;

	const int nDayStart = (IsCompressWeekend () && CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () == 0)
		? 1
		: CBCGPPlannerManagerCtrl::GetFirstDayOfWeek ();

	for (int iRow = 0; iRow < nRows; iRow++)
	{
		CRect rect (m_rectApps);
		rect.right   = rect.left + dxColumn;
		rect.top    += iRow * (dxRow + m_OnePoint.cy);
		rect.bottom  = iRow == (nRows - 1) ? rect.bottom : rect.top + dxRow;

		int nTop    = rect.top;
		int nBottom = rect.bottom;

		for (int iDay = 0; iDay < 7; iDay++)
		{
			int nWeekDay = nDayStart + iDay - 7;

			if (m_bCompressWeekend)
			{
				if (nWeekDay == -1)
				{
					if (iDay == 5)
					{
						rect.right = m_rectApps.right;
					}
					
					rect.bottom = rect.top + dxRow / 2;
				}
				else if (nWeekDay == 0)
				{
					rect.top = rect.bottom + m_OnePoint.cy;
					rect.bottom = nBottom;
				}
				else
				{
					rect.top = nTop;
				}
			}
			else
			{
				if (iDay == 6)
				{
					rect.right = m_rectApps.right;
				}
			}

			m_ViewRects.Add (rect);

			if ((m_bCompressWeekend && nWeekDay != -1) || !m_bCompressWeekend)
			{
				rect.OffsetRect (dxColumn + m_OnePoint.cx, 0);
			}
		}
	}
}

void CBCGPPlannerPrintMonth::AdjustAppointments ()
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

	COleDateTimeSpan spanDay (1, 0, 0, 0);
	const int delta = m_nRowHeight + 2 * m_OnePoint.cy;

	for (int nDay = 0; nDay < nDays; nDay ++)
	{
		CRect rect (m_ViewRects [nDay]);
		rect.top += m_nRowHeight + 2 * m_OnePoint.cy;
		rect.DeflateRect (m_OnePoint.cx, 0);

		BOOL bTopEq    = TRUE;
		BOOL bBottomEq = TRUE;
		
		if (nDay > 0)
		{
			bTopEq    = m_ViewRects [nDay].top == m_ViewRects [nDay - 1].top;
			bBottomEq = m_ViewRects [nDay].bottom == m_ViewRects [nDay - 1].bottom;
		}

		CList<int, int> lsItems;

		for (int i = 0; i < arQueryApps.GetSize (); i++)
		{
			CBCGPAppointment* pApp = arQueryApps[i];
			ASSERT_VALID (pApp);

			if (CBCGPPlannerView::IsAppointmentInDate (*pApp, date))
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

				BOOL bAllOrMulti = pApp->IsAllDay () || pApp->IsMultiDay ();

				// check for add new rect
				if (nDay > 0)
				{
					if (bTopEq && bAllOrMulti)
					{
						if (!pApp->GetDSPrint ().IsEmpty ())
						{
							CRect rt;
							rt = pApp->GetRectPrint (date - spanDay);

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
					  !CBCGPPlannerView::IsOneDay (pApp->GetStart (), pApp->GetFinish ()))) &&
					!pApp->GetDSDraw ().IsEmpty ())
				{
					CRect rtInter;
					rtInter.IntersectRect (rtApp, m_ViewRects [nDay]);

					if (rtInter.Height () < rtApp.Height () || 
						rtInter.bottom >= rect.bottom || !bAllOrMulti)
					{
						// add impossible rect
						rtApp.OffsetRect (0, m_OnePoint.cy);
						pApp->SetRectPrint (rtApp, date);
						// return rect
						rtApp.OffsetRect (0, -m_OnePoint.cy);
					}
				}

				pApp->SetRectPrint (rtApp, date);

				lsItems.AddTail ((rtApp.top - rect.top) / delta);
			}
		}

		CheckVisibleAppointments(date, rect, TRUE);

		date += spanDay;
	}
}

void CBCGPPlannerPrintMonth::OnPaint (CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID (pDC);
	ASSERT (pInfo != NULL);

	CBCGPPlannerPrint::OnPaint (pDC, pInfo);

	int nOldBk = pDC->SetBkMode (TRANSPARENT);

	if (m_nWeekBarWidth != 0)
	{
		CRect rtBar (m_rectApps);
		rtBar.left -= m_nWeekBarWidth;
		rtBar.right = rtBar.left + m_nWeekBarWidth;

		OnDrawWeekBar (pDC, rtBar);
	}

	CFont* pOldFont = pDC->SelectObject (&m_Font);

	OnDrawClient (pDC);

	CRect rtFrame (m_rectApps);

	if (m_nHeaderHeight != 0)
	{
		CRect rtHeader (m_rectApps);
		rtHeader.top    -= m_nHeaderHeight;
		rtHeader.bottom = rtHeader.top + m_nHeaderHeight;

		OnDrawHeader (pDC, rtHeader);

		rtFrame.top = rtHeader.top;
	}

	OnDrawAppointments (pDC);

	pDC->SelectObject (pOldFont);
	pDC->SetBkMode (nOldBk);

	{
		CRect rect (rtFrame);
		rect.InflateRect (m_OnePoint.cx, m_OnePoint.cy);

		HBRUSH hOldBrush = (HBRUSH)::SelectObject (pDC->GetSafeHdc (), ::GetStockObject (NULL_BRUSH));
		CPen* pOldPen = (CPen*)pDC->SelectObject (&m_penBlack);

		pDC->Rectangle (rect);

		::SelectObject (pDC->GetSafeHdc (), hOldBrush);
		pDC->SelectObject (pOldPen);
	}
}

void CBCGPPlannerPrintMonth::OnDrawWeekBar (CDC* pDC, const CRect& rectBar)
{
	if (rectBar.IsRectEmpty ())
	{
		return;
	}

	ASSERT_VALID (pDC);

	const int nRows = GetViewDuration () / 7;

	const BOOL bDateBeforeMonth = CBCGPPlannerView::IsDateBeforeMonth ();

	CStringArray saFormat;
	{
		CString strMonthFmt (_T("MMM"));
		CString strSep (_T(" "));

		CFont* pOldFont = pDC->SelectObject (&m_Font);

		CSize szText (pDC->GetTextExtent (_T("AAA 00 - AAA 00")));

		CRect rect (m_ViewRects[0]);
		rect.right = rectBar.right;
		rect.left  = rectBar.left - m_OnePoint.cy;
		rect.top   += m_nRowHeight + m_OnePoint.cy;
		if (rect.Height() < (szText.cx * 1.5))
		{
			strMonthFmt = _T("M");
			strSep      = CBCGPPlannerView::GetDateSeparator ();
		}

		pDC->SelectObject (pOldFont);

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

	COLORREF clrTextOld = pDC->SetTextColor (m_clrText);
	CFont* pOldFont = pDC->SelectObject (&m_FontVert);
	CPen* pOldPen = pDC->SelectObject (&m_penBlack);

	COleDateTime day1 (GetDateStart ());
	COleDateTime day2 (day1);
	day2 += COleDateTimeSpan (6, 0, 0, 0);

	COleDateTimeSpan span (7, 0, 0, 0);

	int nIndex = 0;

	if (IsCompressWeekend () && m_ViewRects[0].top != m_ViewRects[1].top)
	{
		nIndex = 2;
	}

	for (int iRow = 0; iRow < nRows; iRow++)
	{
		CRect rect (m_ViewRects[nIndex]);
		rect.right = rectBar.right;
		rect.left  = rectBar.left - m_OnePoint.cy;
		rect.top    += m_nRowHeight + m_OnePoint.cy;

		if (rect.top < rect.bottom)
		{
			pDC->FillRect (rect, &m_brGray);
			pDC->MoveTo (rect.right, rect.top);
			pDC->LineTo (rect.left, rect.top);
			pDC->LineTo (rect.left, rect.bottom);
			pDC->LineTo (rect.right, rect.bottom);

			rect.DeflateRect (m_OnePoint.cx, m_OnePoint.cy);

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
			rectText.left = rectText.left + (rect.Width () - szText.cy + m_OnePoint.cx) / 2;

			rectText.top = rectText.bottom;
			if (szText.cx < rect.Height ())
			{
				rectText.top -= (rect.Height () - szText.cx + m_OnePoint.cy) / 2;
			}
			rectText.bottom = rect.top;

			pDC->DrawText (strText, rectText, DT_SINGLELINE);
		}

		nIndex += 7;
		day1 += span;
		day2 += span;
	}

	pDC->SelectObject (pOldPen);
	pDC->SelectObject (pOldFont);
	pDC->SetTextColor (clrTextOld);
}

void CBCGPPlannerPrintMonth::OnDrawClient (CDC* pDC)
{
	ASSERT_VALID (pDC);

	CRect rectFill (m_rectApps);

	int nMonth = m_Date.GetMonth ();

	const int nRows = GetViewDuration () / 7;

	COleDateTime day (GetDateStart ());

	int nIndex = 0;
	int iRow = 0;

	{
		CPen* pOldPen = pDC->SelectObject (&m_penBlack);

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

		for (iColumn = 1; iColumn < nEnd; iColumn++)
		{
			pDC->MoveTo (m_ViewRects [iColumn].left - m_OnePoint.cx, m_rectApps.top);
			pDC->LineTo (m_ViewRects [iColumn].left - m_OnePoint.cx, m_rectApps.bottom);
		}

		for (iRow = 0; iRow < nRows; iRow++)
		{
			int nIndex = iRow * 7 + 6;

			pDC->MoveTo (m_rectApps.left , m_ViewRects [nIndex].bottom);
			pDC->LineTo (m_rectApps.right, m_ViewRects [nIndex].bottom);

			if (m_bCompressWeekend)
			{
				nIndex -= (6 - nCol);
				pDC->MoveTo (m_ViewRects [nIndex].left, m_ViewRects [nIndex].bottom);
				pDC->LineTo (m_ViewRects [nIndex].right, m_ViewRects [nIndex].bottom);
			}
		}

		pDC->SelectObject (pOldPen);
	}

	DWORD dwFlags = GetDrawFlags ();
	BOOL bBold = (dwFlags & BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_BOLD) ==
			BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_BOLD;
	BOOL bCompact = (dwFlags & BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_COMPACT) ==
			BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_COMPACT;

	CFont* pOldFont = NULL;
	if (bBold)
	{
		pOldFont = pDC->SelectObject (&m_FontBold);
	}

	const BOOL bDateBeforeMonth = CBCGPPlannerView::IsDateBeforeMonth ();

	for (iRow = 0; iRow < nRows; iRow++)
	{
		for (int iDay = 0; iDay < 7; iDay++)
		{
			BOOL bAnotherMonth = nMonth != day.GetMonth ();

			int nDay = iRow * 7 + iDay;

			CRect rectDayCaption (m_ViewRects [nDay]);

			if (m_nRowHeight <= rectDayCaption.Height () + 2 * m_OnePoint.cy)
			{
				rectDayCaption.bottom = rectDayCaption.top + m_nRowHeight + m_OnePoint.cy;

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

				DrawCaption (pDC, rectDayCaption, strDate, FALSE, TRUE, 
					bAnotherMonth ? m_brDarkGray : m_brGray);
			}

			day += COleDateTimeSpan (1, 0, 0, 0);

			nIndex++;
		}
	}

	if (bBold && pOldFont != NULL)
	{
		pDC->SelectObject (pOldFont);
	}
}

void CBCGPPlannerPrintMonth::OnDrawHeader (CDC* pDC, const CRect& rectHeader)
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
		DrawCaption (pDC, rectDayCaption, sa[iColumn], TRUE, TRUE, m_brGray);
		rectDayCaption.OffsetRect (dxColumn + m_OnePoint.cx, 0);
	}
}

COleDateTime CBCGPPlannerPrintMonth::GetFirstWeekDay2 (const COleDateTime& date, int nWeekStart) const
{
	return CBCGPPlannerView::GetFirstWeekDay (date, 
		(IsCompressWeekend () && nWeekStart == 1) ? 2 : nWeekStart);
}

void CBCGPPlannerPrintMonth::CalculateDates (const COleDateTime& date)
{
	BOOL bNeedUpdate = TRUE;

	if (m_DateStart != COleDateTime () && m_DateEnd != COleDateTime ())
	{
		bNeedUpdate = ((m_DateEnd - m_DateStart).GetTotalDays () + 1) < 14;
	}

	if (bNeedUpdate)
	{
		m_DateStart = GetFirstWeekDay2 (COleDateTime (date.GetYear (), 
			date.GetMonth (), 1, 0, 0, 0), CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () + 1);

		m_DateEnd   = m_DateStart + COleDateTimeSpan (34, 23, 59, 59);
	}
}

void CBCGPPlannerPrintMonth::SetDrawTimeFinish (BOOL bDraw)
{
	m_bDrawTimeFinish = bDraw;
}

BOOL CBCGPPlannerPrintMonth::IsDrawTimeFinish () const
{
	return m_bDrawTimeFinish;
}

void CBCGPPlannerPrintMonth::SetCompressWeekend (BOOL bCompress)
{
	m_bCompressWeekend = bCompress;
}

BOOL CBCGPPlannerPrintMonth::IsCompressWeekend () const
{
	return m_bCompressWeekend;
}

#endif // BCGP_EXCLUDE_PLANNER
