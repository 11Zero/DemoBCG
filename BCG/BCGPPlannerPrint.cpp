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
// BCGPPlannerPrint.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPPlannerPrint.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerManagerCtrl.h"

#include "BCGPMath.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerPrint

IMPLEMENT_DYNCREATE(CBCGPPlannerPrint, CObject)

CBCGPPlannerPrint::CBCGPPlannerPrint()
	: m_strCaptionFormat ()
	, m_rectApps         (0, 0, 0, 0)
	, m_rectPageHeader   (0, 0, 0, 0)
	, m_rectPageFooter   (0, 0, 0, 0)
	, m_nRowHeight       (0)
	, m_Font             ()
	, m_FontBold         ()
	, m_FontVert         ()
	, m_brWhite          (RGB (255, 255, 255))
	, m_brGray           (RGB (228, 228, 228))
	, m_brDarkGray       (RGB (192, 192, 192))
	, m_penBlack         (PS_SOLID, 0, RGB (0, 0, 0))
	, m_clrText          (RGB (0, 0, 0))
	, m_clrPen           (RGB (0, 0, 0))
	, m_bFirstDraw       (TRUE)
	, m_bDrawPageHeader  (TRUE)
	, m_bDrawPageFooter  (TRUE)
	, m_nCurrentPage     (0)
	, m_dwDrawFlags      (0)
	, m_nFirstViewHour   ( 0.0)
	, m_nLastViewHour    (23.0)
{
}

CBCGPPlannerPrint::~CBCGPPlannerPrint()
{
	for (int i = 0; i < m_arQuery.GetSize (); i++)
	{
		CBCGPAppointment* pApp = m_arQuery[i];

		if (pApp != NULL)
		{
			ASSERT_VALID (pApp);
			if (pApp->IsRecurrenceClone ())
			{
				delete pApp;
			}
			else
			{
				pApp->ResetPrint ();
			}
		}
	}
}

#ifdef _DEBUG
void CBCGPPlannerPrint::AssertValid() const
{
	CObject::AssertValid();
}

void CBCGPPlannerPrint::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif

void CBCGPPlannerPrint::AdjustCaptionFormat (CDC* pDC)
{
	ASSERT_VALID (pDC);

	const int nDays = GetViewDuration ();

	CStringArray sa;
	GetCaptionFormatStrings (sa);

	for (int n = 0; n < sa.GetSize (); n++)
	{
		COleDateTime day (GetDateStart ());

		m_strCaptionFormat = sa [n];

		BOOL bFlag = FALSE;

		for (int nDay = 0; nDay < nDays; nDay++)
		{
			CString strDate;
			strDate.GetBuffer (_MAX_PATH);

			SYSTEMTIME st;
			day.GetAsSystemTime (st);

			::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st, 
				m_strCaptionFormat, (LPTSTR)(LPCTSTR)strDate, _MAX_PATH);

			strDate.ReleaseBuffer ();

			CSize sz (pDC->GetTextExtent (strDate));

			day += COleDateTimeSpan (1, 0, 0, 0);

			if (m_ViewRects [nDay].Width () < sz.cx)
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

void CBCGPPlannerPrint::OnPaint (CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID (pDC);
	ASSERT (pInfo != NULL);

	if (m_bFirstDraw)
	{
		AdjustLayout (pDC, pInfo);
		m_bFirstDraw = FALSE;
	}

	m_nCurrentPage = pInfo->m_nCurPage;

	OnDrawPageHeader (pDC);

	OnDrawPageFooter (pDC);
}

void CBCGPPlannerPrint::AdjustPageHeader (CDC* pDC)
{
	LOGFONT lf;
	m_Font.GetLogFont (&lf);

	lf.lfHeight *= 3;
	lf.lfWeight = FW_BOLD;
	if (m_FontHeader.GetSafeHandle () != NULL)
	{
		m_FontHeader.DeleteObject ();
	}
	m_FontHeader.CreateFontIndirect (&lf);

	CFont* pOldFont = pDC->SelectObject (&m_FontHeader);

	TEXTMETRIC tm;
	pDC->GetTextMetrics (&tm);

	int nHeight = tm.tmHeight * 2 + 4 * m_OnePoint.cy;

	pDC->SelectObject (pOldFont);

	m_rectPageHeader = m_rectApps;
	m_rectPageHeader.bottom = m_rectPageHeader.top + nHeight;
}

void CBCGPPlannerPrint::AdjustPageFooter (CDC* pDC)
{
	LOGFONT lf;
	m_Font.GetLogFont (&lf);

	if (m_FontFooter.GetSafeHandle () != NULL)
	{
		m_FontFooter.DeleteObject ();
	}
	m_FontFooter.CreateFontIndirect (&lf);

	CFont* pOldFont = pDC->SelectObject (&m_FontFooter);

	TEXTMETRIC tm;
	pDC->GetTextMetrics (&tm);

	int nHeight = tm.tmHeight + 4 * m_OnePoint.cy;

	pDC->SelectObject (pOldFont);

	m_rectPageFooter = m_rectApps;
	m_rectPageFooter.top = m_rectPageFooter.bottom - nHeight;
}

void CBCGPPlannerPrint::AdjustLayout (CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID (pDC);
	ASSERT (pInfo != NULL);

	m_rectApps = pInfo->m_rectDraw;

	if (IsDrawPageHeader ())
	{
		AdjustPageHeader (pDC);

		if (!m_rectPageHeader.IsRectEmpty ())
		{
			m_rectApps.top = m_rectPageHeader.bottom + 6 * m_OnePoint.cy;
		}
	}

	if (IsDrawPageFooter ())
	{
		AdjustPageFooter (pDC);

		if (!m_rectPageFooter.IsRectEmpty ())
		{
			m_rectApps.bottom = m_rectPageFooter.top - 6 * m_OnePoint.cy;
		}
	}

	CFont* pOldFont = pDC->SelectObject (&m_FontBold);

	TEXTMETRIC tm;
	pDC->GetTextMetrics (&tm);
	m_nRowHeight    = tm.tmHeight + 4 * m_OnePoint.cy;//max (tm.tmHeight + 4, m_pPlanner->GetClockIconSize ().cy + 2);

	m_rectApps.DeflateRect (m_OnePoint.cx, m_OnePoint.cy);

	AdjustLayout (pDC, m_rectApps);

	m_ViewRects.RemoveAll ();

	AdjustRects ();

	AdjustCaptionFormat (pDC);

	AdjustAppointments ();

	pDC->SelectObject (pOldFont);
}

void CBCGPPlannerPrint::PrepareInfo (CPrintInfo* pInfo)
{
	if (pInfo != NULL)
	{
		pInfo->SetMinPage (1);
		pInfo->SetMaxPage (1);
		pInfo->m_pPD->m_pd.Flags |= PD_NOPAGENUMS | PD_NOSELECTION;
	}
}

void CBCGPPlannerPrint::PreparePrinting (CDC* pDC, CPrintInfo* pInfo, 
										CBCGPPlannerManagerCtrl* pPlanner)
{
	ASSERT_VALID (pPlanner);
	ASSERT_VALID (pDC);
	ASSERT (pInfo != NULL);

	if (pPlanner == NULL || pPlanner->GetSafeHwnd () == NULL || pInfo == NULL || 
		pDC == NULL || pDC->GetSafeHdc () == NULL)
	{
		return;
	}

	m_dwDrawFlags = pPlanner->GetDrawFlags ();

	m_bFirstDraw = TRUE;

	m_Date      = pPlanner->GetDate ();
	m_DateStart = pPlanner->GetDateStart ();
	m_DateEnd   = pPlanner->GetDateEnd ();

	m_nFirstViewHour = pPlanner->GetFirstPrintHour ();
	m_nLastViewHour  = pPlanner->GetLastPrintHour ();
	
	CalculateDates (m_Date);

	SetCompressWeekend (pPlanner->IsCompressWeekend ());

	m_arQuery.RemoveAll ();

	pPlanner->QueryAppointments (m_arQuery, GetDateStart (), GetDateEnd ());

	CFont* pFont = pPlanner->GetFont ();

	ASSERT (pFont != NULL);

	LOGFONT lf;
	pFont->GetLogFont (&lf);

	// map to printer font metrics
	HDC hDCFrom = ::GetDC(NULL);
	lf.lfHeight = ::MulDiv(lf.lfHeight, pDC->GetDeviceCaps(LOGPIXELSY),
		::GetDeviceCaps(hDCFrom, LOGPIXELSY));
	lf.lfWidth = ::MulDiv(lf.lfWidth, pDC->GetDeviceCaps(LOGPIXELSX),
		::GetDeviceCaps(hDCFrom, LOGPIXELSX));

	m_OnePoint.cx = CBCGPPlannerView::round (pDC->GetDeviceCaps(LOGPIXELSX) /
		(double)::GetDeviceCaps(hDCFrom, LOGPIXELSX));
	m_OnePoint.cy = CBCGPPlannerView::round (pDC->GetDeviceCaps(LOGPIXELSY) /
		(double)::GetDeviceCaps(hDCFrom, LOGPIXELSY));

	::ReleaseDC(NULL, hDCFrom);
	
	if (m_Font.GetSafeHandle () != NULL)
	{
		m_Font.DeleteObject ();
	}
	m_Font.CreateFontIndirect (&lf);

	LONG lfWeight = lf.lfWeight;
	lf.lfWeight = FW_BOLD;
	if (m_FontBold.GetSafeHandle () != NULL)
	{
		m_FontBold.DeleteObject ();
	}
	m_FontBold.CreateFontIndirect (&lf);
	lf.lfWeight      = lfWeight;

	if (m_FontVert.GetSafeHandle () != NULL)
	{
		m_FontVert.DeleteObject ();
	}
	lf.lfOrientation = 900;
	lf.lfEscapement  = lf.lfOrientation;
	m_FontVert.CreateFontIndirect (&lf);

	m_rectApps.SetRectEmpty ();
	m_rectPageHeader.SetRectEmpty ();
	m_rectPageFooter.SetRectEmpty ();

	m_dtBegin = COleDateTime::GetCurrentTime ();
	m_nCurrentPage = 0;
}

void CBCGPPlannerPrint::OnDrawPageHeader (CDC* pDC)
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

	CString str = GetPageHeaderText ();
	if (!str.IsEmpty ())
	{
		int nOldBk = pDC->SetBkMode (TRANSPARENT);
		CFont* pOldFont = pDC->SelectObject (&m_FontHeader);

		CRect rt (m_rectPageHeader);
		rt.DeflateRect (m_OnePoint.cx * 4, m_OnePoint.cy);

		pDC->DrawText (str, rt, DT_LEFT);

		pDC->SelectObject (pOldFont);
		pDC->SetBkMode (nOldBk);
	}

	pDC->Draw3dRect (m_rectPageHeader, m_clrPen, m_clrPen);
}

void CBCGPPlannerPrint::OnDrawPageFooter (CDC* pDC)
{
	if (m_rectPageFooter.IsRectEmpty ())
	{
		return;
	}

	int nOldBk = pDC->SetBkMode (TRANSPARENT);

	CFont* pOldFont = pDC->SelectObject (&m_FontFooter);

	CRect rt (m_rectPageFooter);
	rt.DeflateRect (0, m_OnePoint.cy);

	CString str;
	
	SYSTEMTIME st;
	m_dtBegin.GetAsSystemTime (st);

	TCHAR sz[100];
	::GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, sz, 100);

	str += sz;
	str += _T(" ");

	::GetTimeFormat (LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, sz, 100);

	str += sz;

	pDC->DrawText (str, rt, DT_VCENTER | DT_RIGHT | DT_SINGLELINE);

	str.Format (_T("%d"), m_nCurrentPage);

	pDC->DrawText (str, rt, DT_VCENTER | DT_CENTER | DT_SINGLELINE);

	pDC->SelectObject (pOldFont);
	pDC->SetBkMode (nOldBk);
}

void CBCGPPlannerPrint::CheckVisibleAppointments(const COleDateTime& date, const CRect& rect, 
	BOOL bFullVisible)
{
	XBCGPAppointmentArray& arQueryApps = GetQueryedAppointments ();

	BOOL bSelect = date != COleDateTime ();

	if (arQueryApps.GetSize () > 0)
	{
		for (int i = 0; i < arQueryApps.GetSize (); i++)
		{
			CBCGPAppointment* pApp = arQueryApps [i];

			if (bSelect)
			{
				if (!CBCGPPlannerView::IsAppointmentInDate (*pApp, date))
				{
					continue;
				}
			}

			CRect rtDraw (pApp->GetRectPrint (date));

			CRect rtInter;
			rtInter.IntersectRect (rtDraw, rect);

			pApp->SetVisiblePrint ((!bFullVisible && rtInter.Height () >= 2) || 
				(bFullVisible && rtInter.Height () == rtDraw.Height () && 
				 rtInter.bottom < rect.bottom), date);
		}
	}
}

BYTE CBCGPPlannerPrint::OnDrawAppointments (CDC* pDC, const CRect& rect, const COleDateTime& date)
{
	BYTE res = 0;

	XBCGPAppointmentArray& arQueryApps = GetQueryedAppointments ();

	BOOL bSelect = date != COleDateTime ();

	if (arQueryApps.GetSize () > 0)
	{
		for (int i = 0; i < arQueryApps.GetSize (); i++)
		{
			CBCGPAppointment* pApp = arQueryApps [i];

			if (bSelect)
			{
				if (!CBCGPPlannerView::IsAppointmentInDate (*pApp, date))
				{
					continue;
				}
			}

			if (!bSelect || (bSelect && pApp->IsVisiblePrint (date)))
			{
				pApp->SetPrinter (this);
				pApp->OnPrint (pDC, GetPenColor (), GetTextColor (), m_OnePoint);
				pApp->SetPrinter (NULL);
			}
			else
			{
				if (!rect.IsRectNull ())
				{
					if (pApp->GetRectPrint (date).bottom <= rect.top)
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

void CBCGPPlannerPrint::DrawHeader (CDC* pDC, const CRect& rect, int /*dxColumn*/)
{
	ASSERT_VALID (pDC);

	pDC->FillRect (rect, &m_brGray);
}

void CBCGPPlannerPrint::DrawCaption (CDC* pDC, const CRect& rect, 
	const CString& strText, BOOL bCenter, BOOL bExtended, const CBrush& br)
{
	ASSERT_VALID (pDC);

	CRect rt (rect);

	pDC->FillRect (rt, const_cast<CBrush*>(&br));

	rt.DeflateRect (2 * m_OnePoint.cx, 0);

	COLORREF clrOld = pDC->SetTextColor (m_clrText);

	BOOL bCompact = (GetDrawFlags () & BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_COMPACT) ==
		BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_COMPACT;

	DWORD dwFlags = DT_SINGLELINE | DT_VCENTER;
	if (bCompact)
	{
		if (strText.Find (TCHAR('\n')) != -1)
		{
			CString strDate;

			AfxExtractSubString (strDate, strText, 0, TCHAR('\n'));
			pDC->DrawText (strDate, rt, dwFlags | DT_LEFT);

			AfxExtractSubString (strDate, strText, 1, TCHAR('\n'));
			pDC->DrawText (strDate, rt, dwFlags | DT_CENTER);
		}
		else
		{
			dwFlags |= DT_LEFT;
			bCompact = FALSE;
		}
	}
	else
	{
		dwFlags |= (bCenter ? DT_CENTER : DT_RIGHT);
	}

	if (!bCompact)
	{
		pDC->DrawText (strText, rt, dwFlags);
	}	

	pDC->SetTextColor (clrOld);

	if (bExtended)
	{
		rt.InflateRect (2 * m_OnePoint.cx, 0);

		CPen* pOldPen = pDC->SelectObject (&m_penBlack);

		pDC->MoveTo (rt.left, rt.bottom);
		pDC->LineTo (rt.right, rt.bottom);

		pDC->SelectObject (pOldPen);
	}
}

void CBCGPPlannerPrint::DrawCaption (CDC* pDC, const CRect& rect, 
	const COleDateTime& day, BOOL bCenter, BOOL bExtended, const CBrush& br)
{
	ASSERT_VALID (pDC);

	CString strDate;
	strDate.GetBuffer (_MAX_PATH);

	SYSTEMTIME st;
	day.GetAsSystemTime (st);

	::GetDateFormat (LOCALE_USER_DEFAULT, 0, &st, 
		m_strCaptionFormat, (LPTSTR)(LPCTSTR)strDate, _MAX_PATH);

	strDate.ReleaseBuffer ();

	DrawCaption (pDC, rect, strDate, bCenter, bExtended, br);
}

void CBCGPPlannerPrint::DrawAppointment (CDC* pDC, CBCGPAppointment* pApp, CBCGPAppointmentDrawStruct* pDS)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pApp);

	if (pDC == NULL || pDC->GetSafeHdc () == NULL || pApp == NULL || pDS == NULL)
	{
		return;
	}

	CRect rect (pDS->GetRect ());

	COleDateTime dtStart  (pApp->GetStart ());
	COleDateTime dtFinish (pApp->GetFinish ());

	CString      strStart  (pApp->m_strStart);
	CString      strFinish (pApp->m_strFinish);

	BOOL bAlternative = pApp->IsAllDay () || pApp->IsMultiDay ();
	BOOL bEmpty       = (dtStart == dtFinish) && !bAlternative;

	if (!bAlternative && !CBCGPPlannerView::IsOneDay (dtStart, dtFinish))
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

	if (bAlternative)
	{
		rect.DeflateRect (3 * m_OnePoint.cx, 0);
	}
	else
	{
		rect.DeflateRect (5 * m_OnePoint.cx, 0);
	}

	if (pApp->m_clrBackgroung != CLR_DEFAULT || bAlternative)
	{
		CBrush br (pApp->m_clrBackgroung != CLR_DEFAULT 
			? pApp->m_clrBackgroung
			: RGB (255, 255, 255));
		pDC->FillRect (rect, &br);
	}

	COLORREF clrOld = pDC->SetTextColor (pApp->m_clrForeground != CLR_DEFAULT 
		? pApp->m_clrForeground
		: m_clrText);

	if (bAlternative)
	{
		CPen pen (PS_SOLID, 0, m_clrPen);
		CPen* pOldPen = (CPen*)pDC->SelectObject (&pen);

		if (pDS->GetBorder () == CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_ALL)
		{
			HBRUSH hOldBrush = (HBRUSH)::SelectObject (pDC->GetSafeHdc (), ::GetStockObject (NULL_BRUSH));

			pDC->Rectangle (rect);

			::SelectObject (pDC->GetSafeHdc (), hOldBrush);
		}
		else
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

		pDC->SelectObject (pOldPen);
	}

	if (!bAlternative)
	{
		CSize szSpace (pDC->GetTextExtent (_T(" ")));

		if (!strStart.IsEmpty ())
		{
			COleDateTime dt (2005, 1, 1, 23, 59, 0);

			CString strDT = CBCGPPlannerView::Is24Hours ()
				? dt.Format (_T("%H:%M"))
				: dt.Format (_T("%H:%M%p"));
			strDT.MakeLower ();

			CSize szDT (pDC->GetTextExtent (strDT));

			if (rect.Width () >= szDT.cx * 2)
			{
				CRect rectText (rect);
				rectText.DeflateRect (0, m_OnePoint.cy, m_OnePoint.cx, 0);

				rectText.right = rectText.left + szDT.cx;

				pDC->DrawText (strStart, rectText, DT_NOPREFIX | DT_RIGHT);

				rect.left = rectText.right + szSpace.cx;

				if (rect.Width () >= szDT.cx * 3)
				{
					CRect rectText (rect);
					rectText.DeflateRect (0, m_OnePoint.cy, m_OnePoint.cx, 0);

					rectText.right = rectText.left + szDT.cx;

					if (!bEmpty)
					{
						pDC->DrawText (strFinish, rectText, DT_NOPREFIX | DT_RIGHT);
					}

					rect.left = rectText.right + szSpace.cx;
				}
			}
		}
	}

	BOOL bDrawText = !pApp->m_strDescription.IsEmpty ();

	int nTextWidth = 0;
	if (bAlternative && bDrawText)
	{
		nTextWidth = pDC->GetTextExtent (pApp->m_strDescription).cx;
	}

	if (bDrawText)
	{
		CRect rectText (rect);

		rectText.DeflateRect (4 * m_OnePoint.cx, m_OnePoint.cy, m_OnePoint.cx, 0);

		if (bAlternative && nTextWidth < rectText.Width ())
		{
			rectText.left += (rectText.Width () - nTextWidth) / 2;
		}

		pDC->DrawText (pApp->m_strDescription, rectText, DT_NOPREFIX);
	}

	pDC->SetTextColor (clrOld);
}

void CBCGPPlannerPrint::SetDrawPageHeader (BOOL bDraw)
{
	m_bDrawPageHeader = bDraw;
}

void CBCGPPlannerPrint::SetDrawPageFooter (BOOL bDraw)
{
	m_bDrawPageFooter = bDraw;
}

#endif // BCGP_EXCLUDE_PLANNER
