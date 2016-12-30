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
// BCGPPlannerDayView.cpp: implementation of the CBCGPPlannerViewWeek class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPPlannerViewWeek.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerManagerCtrl.h"

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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CBCGPPlannerViewWeek, CBCGPPlannerView)

CBCGPPlannerViewWeek::CBCGPPlannerViewWeek()
	: CBCGPPlannerView ()
	, m_bDrawTimeFinish (TRUE)
	, m_bDrawTimeAsIcons (FALSE)
{
	m_DateEnd = m_DateStart + COleDateTimeSpan (6, 23, 59, 59);
}

CBCGPPlannerViewWeek::~CBCGPPlannerViewWeek()
{
}

#ifdef _DEBUG
void CBCGPPlannerViewWeek::AssertValid() const
{
	CBCGPPlannerView::AssertValid();
}

void CBCGPPlannerViewWeek::Dump(CDumpContext& dc) const
{
	CBCGPPlannerView::Dump(dc);
}
#endif

void CBCGPPlannerViewWeek::GetCaptionFormatStrings (CStringArray& sa)
{
	sa.RemoveAll ();

	BOOL bCompact = (GetPlanner ()->GetDrawFlags () & BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_COMPACT) ==
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

void CBCGPPlannerViewWeek::AdjustScrollSizes ()
{
	if (m_nScrollPage != 1 && m_nScrollTotal != 50)
	{
		m_nScrollPage   = 1;
		m_nScrollTotal  = 50;
		m_nScrollOffset = m_nScrollTotal / 2;
	}

	CBCGPPlannerView::AdjustScrollSizes ();
}

void CBCGPPlannerViewWeek::AdjustLayout (CDC* /*pDC*/, const CRect& /*rectClient*/)
{
	AdjustScrollSizes ();
}

void CBCGPPlannerViewWeek::AdjustRects ()
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

		if (nDay == 2 || nDay == 5)
		{
			rect.bottom = m_rectApps.bottom;
		}
		else if (nDay == 3)
		{
			rect = m_rectApps;

			rect.left  += nWidth2 + 1;
			rect.bottom = rect.top + nHeight3;
		}

		if (nWeekDay == -1)
		{
			CRect rt (rect);

			rt.bottom = rect.top + rect.Height () / 2;
			m_ViewRects.Add (rt);

			rt.top = rt.bottom + 1;
			rt.bottom = rect.bottom;
			m_ViewRects.Add (rt);
		}
		else
		{
			m_ViewRects.Add (rect);
		}

		rect.OffsetRect (0, nHeight3 + 1);
	}
}

void CBCGPPlannerViewWeek::AdjustAppointments ()
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

	COleDateTime date (m_DateStart);

	for (int nApp = 0; nApp < 2; nApp++)
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

	COleDateTimeSpan spanDay (1, 0, 0, 0);

	for (int nDay = 0; nDay < nDays; nDay ++)
	{
		CRect rect (m_ViewRects [nDay]);
		rect.top += m_nRowHeight + 1;
		rect.DeflateRect (1, 0);

		int nItem = 0;

		for (int nApp = 0; nApp < 2; nApp++)
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

				if (CBCGPPlannerView::IsAppointmentInDate (*pApp, date))
				{
					CRect rtApp (rect);

					if (nItem > 0)
					{
						rtApp.top += nItem * (m_nRowHeight + 2);
					}

					nItem++;

					rtApp.bottom = rtApp.top + m_nRowHeight;

					pApp->SetRectDraw (rtApp, date);
				}
			}
		}

		CheckVisibleAppointments(date, rect, TRUE);

		date += spanDay;
	}

	CheckVisibleUpDownIcons (TRUE);

	m_bUpdateToolTipInfo = TRUE;
}

void CBCGPPlannerViewWeek::AddUpDownRect(BYTE nType, const CRect& rect)
{
	if (nType != 0)
	{
		CSize sz (GetPlanner ()->GetUpDownIconSize ());
		CRect rt (CPoint(rect.right - sz.cx, rect.bottom - sz.cy), sz);

		m_DownRects.Add (rt);
	}
}

void CBCGPPlannerViewWeek::OnPaint (CDC* pDC, const CRect& rectClient)
{
	ASSERT_VALID (pDC);

	OnDrawClient (pDC, rectClient);
	
	OnDrawAppointments (pDC);

	OnDrawUpDownIcons (pDC);

	InitToolTipInfo ();
}

void CBCGPPlannerViewWeek::OnDrawClient (CDC* pDC, const CRect& rect)
{
	ASSERT_VALID (pDC);

	const int nDays = GetViewDuration ();

	if (m_ViewRects.GetSize () != nDays)
	{
		return;
	}

	COleDateTime day (GetDateStart ());

	COleDateTime dayCurrent = COleDateTime::GetCurrentTime ();
	dayCurrent = COleDateTime (dayCurrent.GetYear (), dayCurrent.GetMonth (), 
		dayCurrent.GetDay (), 0, 0, 0);

	int nDay = 0;

	CPen penBlack (PS_SOLID, 0, visualManager->GetPlannerSeparatorColor (this));
	CPen* pOldPen = pDC->SelectObject (&penBlack);

	pDC->MoveTo (m_ViewRects[0].right, rect.top);
	pDC->LineTo (m_ViewRects[0].right, rect.bottom);

	for (nDay = 1; nDay < 7; nDay++)
	{
		if (m_ViewRects[nDay].top != m_rectApps.top)
		{
			pDC->MoveTo (m_ViewRects[nDay].left, m_ViewRects[nDay].top - 1);
			pDC->LineTo (m_ViewRects[nDay].right, m_ViewRects[nDay].top - 1);
		}
	}

	pDC->SelectObject (pOldPen);

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

	for (nDay = 0; nDay < nDays; nDay++)
	{
		CRect rectDayCaption (m_ViewRects [nDay]);
	
		BOOL bToday = day == dayCurrent;
		BOOL bSelected = IsDateInSelection (day);		

		visualManager->PreparePlannerBackItem (bToday, bSelected);
		OnFillPlanner (pDC, rectDayCaption, TRUE);

		rectDayCaption.bottom = rectDayCaption.top + m_nRowHeight;

		visualManager->PreparePlannerCaptionBackItem (FALSE);
		COLORREF clrText = OnFillPlannerCaption (
			pDC, rectDayCaption, bToday, bSelected, FALSE);

		if (!bCompact)
		{
			DrawCaptionText (pDC, rectDayCaption, day, clrText, DT_RIGHT,
				bToday && bSelected);
		}
		else
		{
			CString strText;
			ConstructCaptionText (day, strText, GetCaptionFormat ());

			if (strText.Find (TCHAR('\n')) != -1)
			{
				CString strDate;

				AfxExtractSubString (strDate, strText, 0, TCHAR('\n'));
				DrawCaptionText (pDC, rectDayCaption, strDate, clrText, DT_LEFT,
					bToday && bSelected);

				AfxExtractSubString (strDate, strText, 1, TCHAR('\n'));
				DrawCaptionText (pDC, rectDayCaption, strDate, clrText, DT_CENTER,
					bToday && bSelected);
			}
			else
			{
				DrawCaptionText (pDC, rectDayCaption, strText, clrText, DT_LEFT,
					bToday && bSelected);
			}
		}

		day += COleDateTimeSpan (1, 0, 0, 0);
	}

	if (hOldFont != NULL)
	{
		::SelectObject (pDC->GetSafeHdc (), hOldFont);
	}
}

BOOL CBCGPPlannerViewWeek::OnScroll(UINT /*nScrollCode*/, UINT /*nPos*/, BOOL /*bDoScroll*/)
{
	return FALSE;
}

BOOL CBCGPPlannerViewWeek::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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

BOOL CBCGPPlannerViewWeek::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
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
		COleDateTime date (GetFirstWeekDay2 
			(m_Date, CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () + 1));

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

		if (nChar == VK_UP)
		{
			date -= span;
		}
		else if (nChar == VK_DOWN)
		{
			date += span;
		}
		else
		{
			COleDateTime end (m_DateStart + COleDateTimeSpan (6, 0, 0, 0));

			if (date == end)
			{
				date -= span;
			}

			COleDateTimeSpan span3 (3, 0, 0 ,0);

			if (nChar == VK_LEFT)
			{
				date -= span3;
			}
			else if (nChar == VK_RIGHT)
			{
				date += span3;
			}

			if (date < m_DateStart)
			{
				date -= COleDateTimeSpan (1, 0, 0, 0);
			}
			else if (date > end)
			{
				date += COleDateTimeSpan (1, 0, 0, 0);
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

COleDateTime CBCGPPlannerViewWeek::GetDateFromPoint (const CPoint& point) const
{
	return CBCGPPlannerView::GetDateFromPoint (point);
}

void CBCGPPlannerViewWeek::OnActivate(CBCGPPlannerManagerCtrl* pPlanner, const CBCGPPlannerView* pOldView)
{
	ASSERT_VALID(pPlanner);

	if (pOldView != NULL)
	{
		m_Date = pOldView->GetDate ();
	}

	m_DateStart = CalculateDateStart (
		COleDateTime(m_Date.GetYear (), m_Date.GetMonth (), m_Date.GetDay (), 0, 0, 0));
	m_DateEnd   = m_DateStart + COleDateTimeSpan (6, 23, 59, 59);

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

void CBCGPPlannerViewWeek::SetDate (const COleDateTime& date)
{
	COleDateTime dt (date.GetYear (), date.GetMonth (), date.GetDay (), 0, 0, 0);

	//if (m_DateEnd < dt || dt < m_DateStart)
	{
		m_DateStart = CalculateDateStart (dt);
		m_DateEnd   = m_DateStart + COleDateTimeSpan (6, 23, 59, 59);
	}

	CBCGPPlannerView::SetDate (dt);
}

COleDateTime CBCGPPlannerViewWeek::CalculateDateStart (const COleDateTime& date) const
{
	return GetFirstWeekDay2 (date, CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () + 1);
}

void CBCGPPlannerViewWeek::SetSelection (const COleDateTime& sel1, const COleDateTime& sel2, BOOL bRedraw)
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

void CBCGPPlannerViewWeek::SetDrawTimeFinish (BOOL bDraw)
{
	if (m_bDrawTimeFinish != bDraw)
	{
		m_bDrawTimeFinish = bDraw;

		CBCGPPlannerView::AdjustLayout ();
	}
}

BOOL CBCGPPlannerViewWeek::IsDrawTimeFinish () const
{
	return m_bDrawTimeFinish;
}

void CBCGPPlannerViewWeek::SetDrawTimeAsIcons (BOOL bDraw)
{
	if (m_bDrawTimeAsIcons != bDraw)
	{
		m_bDrawTimeAsIcons = bDraw;

		CBCGPPlannerView::AdjustLayout ();
	}
}

BOOL CBCGPPlannerViewWeek::IsDrawTimeAsIcons () const
{
	return m_bDrawTimeAsIcons;
}

COleDateTime CBCGPPlannerViewWeek::GetFirstWeekDay2 (const COleDateTime& date, int nWeekStart) const
{
	return CBCGPPlannerView::GetFirstWeekDay (date, nWeekStart == 1 ? 2 : nWeekStart);
}

BOOL CBCGPPlannerViewWeek::CanCaptureAppointment (BCGP_PLANNER_HITTEST hitCapturedAppointment) const
{
	if (IsReadOnly ())
	{
		return FALSE;
	}

	return hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_LEFT ||
		   hitCapturedAppointment == BCGP_PLANNER_HITTEST_APPOINTMENT_RIGHT;
}

CBCGPPlannerView::BCGP_PLANNER_HITTEST
CBCGPPlannerViewWeek::HitTestArea (const CPoint& point) const
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
		
	return hit;
}

CString CBCGPPlannerViewWeek::GetAccName() const
{
	return _T("Week View");
}

#endif // BCGP_EXCLUDE_PLANNER
