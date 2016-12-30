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
// BCGPPlannerViewMulti.cpp: implementation of the CBCGPPlannerViewMulti class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPPlannerViewMulti.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerManagerCtrl.h"
#include "BCGPDrawManager.h"
#include "BCGPAppointmentStorage.h"

#ifndef _BCGPCALENDAR_STANDALONE
	#include "BCGPVisualManager.h"
	#define visualManager	CBCGPVisualManager::GetInstance ()
#else
	#include "BCGPCalendarVisualManager.h"
	#define visualManager	CBCGPCalendarVisualManager::GetInstance ()
#endif

#include "BCGPMath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const int s_HeaderAllDayPadding = 2;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CBCGPPlannerViewMulti, CBCGPPlannerViewDay)

CBCGPPlannerViewMulti::CBCGPPlannerViewMulti()
	: CBCGPPlannerViewDay()
	, m_nResourceID      (CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID)
{
	m_DateEnd = m_DateStart + COleDateTimeSpan (0, 23, 59, 59);
}

CBCGPPlannerViewMulti::~CBCGPPlannerViewMulti()
{
	StopTimer (FALSE);
}

#ifdef _DEBUG
void CBCGPPlannerViewMulti::AssertValid() const
{
	CBCGPPlannerViewDay::AssertValid();
}

void CBCGPPlannerViewMulti::Dump(CDumpContext& dc) const
{
	CBCGPPlannerViewDay::Dump(dc);
}
#endif

void CBCGPPlannerViewMulti::OnDrawAppointmentsDuration (CDC* pDC)
{
	if ((GetPlanner ()->GetDrawFlags () & BCGP_PLANNER_DRAW_VIEW_NO_DURATION) == 
			BCGP_PLANNER_DRAW_VIEW_NO_DURATION)
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

	COleDateTime dtS = GetDateStart ();
	COleDateTime dtE = GetDateEnd ();

	const int nMinuts = CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ());
	const int nCount = GetViewHours() * 60 / nMinuts;
	const int yOffset = GetViewHourOffset () * m_nRowHeight;

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
			if (pApp == NULL || !(pApp->IsAllDay () || pApp->IsMultiDay ()) || 
				pApp->GetDurationColor () == CLR_DEFAULT)
			{
				continue;
			}

			BOOL bDraw = FALSE;

			if (bDragDrop && dragEffect != DROPEFFECT_NONE && 
				pApp->IsSelected () && nApp == 0)
			{
				if ((dragEffect & DROPEFFECT_COPY) == DROPEFFECT_COPY || bDragMatch)
				{
					bDraw = TRUE;
				}
			}
			else
			{
				bDraw = TRUE;
			}

			if(!bDraw)
			{
				continue;
			}

			int nResourceIndex = FindResourceIndexByID (pApp->GetResourceID ());
			ASSERT(nResourceIndex != -1);
			XResource& resource = m_Resources[nResourceIndex];

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
				? globalData.clrWindow
				: pApp->GetDurationColor ());

			for(int i = nStart; i < nEnd; i++)
			{
				CRect rt (resource.m_Rects[i]);

				rt.right  = rt.left + 
					CBCGPPlannerViewMulti::BCGP_PLANNER_DURATION_BAR_WIDTH + 1;		
				rt.left  -= (i == 0) ? 1 : 0;
				rt.top   -= 1;
				rt.bottom = rt.top + nCount * m_nRowHeight;
				rt.DeflateRect (1, 0);
				
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

					rt.IntersectRect (rt, resource.m_Rects[i]);
				}

				pDC->FillRect (rt, &br);
			}
		}
	}
}

void CBCGPPlannerViewMulti::AdjustLayout (CDC* /*pDC*/, const CRect& rectClient)
{
	if (IsCurrentTimeVisible ())
	{
		StartTimer (FALSE);
	}
	else
	{
		StopTimer (FALSE);
	}

	m_nHeaderHeight       = 2;
	m_nHeaderAllDayHeight = 1;

	const int nMinuts = CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ());
	const int nCount = GetViewHours() * 60 / nMinuts;

	m_rectTimeBar = rectClient;
	m_rectTimeBar.right = m_rectTimeBar.left + (long)(m_nRowHeight * (nMinuts == 60 ? 2.5 : 3.0)) + 5;

	m_rectApps.left = m_rectTimeBar.right;

	{
		// finding allday or multiday events
		const int nDays = GetViewDuration ();

		CDWordArray arDays;
		arDays.SetSize (nDays);

		COleDateTime dtS = GetDateStart ();

		XBCGPAppointmentArray& arQueryApps = GetQueryedAppointments ();
		XBCGPAppointmentArray& arDragApps  = GetDragedAppointments ();

		BOOL bDragDrop        = IsDragDrop ();
		DROPEFFECT dragEffect = GetDragEffect ();
		BOOL bDragMatch       = IsCaptureMatched ();

		bDragDrop = !bDragDrop || 
			(bDragDrop && ((dragEffect & DROPEFFECT_COPY) == DROPEFFECT_COPY && bDragMatch) || 
			!bDragMatch);
		bDragDrop = bDragDrop && arDragApps.GetSize ();

		DWORD maxCount = 0;

		for (int nRes = 0; nRes < m_Resources.GetSize (); nRes++)
		{
			UINT nResourceID = m_Resources[nRes].m_ResourceID;

			for (int nApp = 0; nApp < 2; nApp++)
			{
				if (!bDragDrop && nApp == 0)
				{
					continue;
				}

				XBCGPAppointmentArray& arApps = nApp == 0 ? arDragApps : arQueryApps;

				int nStartIndex = 0;
				int i = 0;
				for (i = 0; i < arApps.GetSize (); i++)
				{
					const CBCGPAppointment* pApp = arApps[i];
					if (pApp != NULL && pApp->GetResourceID () == nResourceID)
					{
						nStartIndex = i;
						break;
					}
				}

				for (i = nStartIndex; i < (int)arApps.GetSize (); i++)
				{
					const CBCGPAppointment* pApp = arApps[i];
					if (pApp == NULL)
					{
						continue;
					}

					ASSERT_VALID (pApp);

					if (pApp->GetResourceID () != nResourceID)
					{
						break;
					}

					if (pApp->IsAllDay () || pApp->IsMultiDay ())
					{
						if (nDays > 1)
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

							if (nEnd > (int) nDays)
							{
								nEnd = (int) nDays;
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
			}

			for (int i = 0; i < nDays; i++)
			{
				if (maxCount < arDays[i])
				{
					maxCount = arDays[i];
				}

				arDays[i] = 0;
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

	const int nRowHeightPadding = m_nRowHeight + s_HeaderAllDayPadding;
	int nHeaderAllDayCount = m_nHeaderAllDayHeight;
	m_nHeaderHeight       *= m_nRowHeight;
	m_nHeaderAllDayHeight *= nRowHeightPadding;

	m_rectApps.top += m_nHeaderHeight;

	m_nHeaderScrollTotal = 0;
	m_nHeaderScrollPage  = 1;

	if (GetPlanner()->IsHeaderScrollingEnabled() && 
		nHeaderAllDayCount > 0 && m_nHeaderAllDayHeight > m_rectApps.Height () / 2)
	{
		m_nHeaderAllDayHeight = min(m_rectApps.Height () / (nRowHeightPadding * 2), nHeaderAllDayCount);
		if (m_nHeaderAllDayHeight == 0)
		{
			m_nHeaderAllDayHeight = 1;
		}

		if (m_nHeaderAllDayHeight != nHeaderAllDayCount)
		{
			m_nHeaderScrollTotal = nHeaderAllDayCount - 1;
			m_nHeaderScrollPage  = m_nHeaderAllDayHeight;
		}

		m_nHeaderAllDayHeight *= nRowHeightPadding;
	}

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

	AdjustScrollSizes ();
	
	// correct selection
	COleDateTime sel1 (GetSelectionStart ());
	COleDateTime sel2 (GetSelectionEnd ());

	SetSelection (sel1, sel2, FALSE);
}

void CBCGPPlannerViewMulti::AdjustCaptionFormat (CDC* pDC)
{
	CBCGPPlannerViewDay::AdjustCaptionFormat (pDC);

	ASSERT_VALID (pDC);

	for (int nRes = 0; nRes < m_Resources.GetSize (); nRes++)
	{
		XResource& res = m_Resources[nRes];
		if (res.m_Rects.GetSize () == 0)
		{
			continue;
		}

		BOOL bToolTipNeeded = res.m_Rects[0].Width () <= (pDC->GetTextExtent (res.m_Description).cx + 2);
		if (res.m_bToolTipNeeded != bToolTipNeeded)
		{
			res.m_bToolTipNeeded = bToolTipNeeded;
			m_bUpdateToolTipInfo = TRUE;
		}
	}
}

void CBCGPPlannerViewMulti::AdjustRects ()
{
	CBCGPPlannerViewDay::AdjustRects ();

	const int countView = (int) m_ViewRects.GetSize ();

	if (m_Resources.GetSize () == 0)
	{
		OnUpdateStorage ();
	}
	const int count = (int) m_Resources.GetSize ();
	ASSERT(count > 0);

	int i = 0;

	for (i = 0; i < count; i++)
	{
		XResource& resource = m_Resources[i];
		resource.m_bToolTipNeeded = FALSE;
		resource.m_Rects.SetSize (countView);
	}

	for (i = 0; i < countView; i++)
	{
		CRect rect (m_ViewRects[i]);
		int dxColumn = CBCGPPlannerView::round (rect.Width () / (double)count);

		rect.right = rect.left + dxColumn;

		for (int j = 0; j < count; j++)
		{
			m_Resources[j].m_Rects[i] = rect;

			rect.OffsetRect (dxColumn, 0);

			if (j == (count - 2))
			{
				rect.right = m_ViewRects[i].right;
			}
		}
	}
}

void CBCGPPlannerViewMulti::AdjustAppointments ()
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

	BOOL bDrawShadow = IsDrawAppsShadow ();

	BOOL bDragDrop        = IsDragDrop ();
	DROPEFFECT dragEffect = GetDragEffect ();
	BOOL bDragMatch       = IsCaptureMatched ();

	bDragDrop = !bDragDrop || 
		(bDragDrop && ((dragEffect & DROPEFFECT_COPY) == DROPEFFECT_COPY && bDragMatch) || 
		!bDragMatch);
	bDragDrop = bDragDrop && arDragApps.GetSize ();

	const int nTimeDelta = CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ());
	const int yOffset = GetViewHourOffset () * m_nRowHeight;
	const int yHeaderOffset = m_nHeaderScrollOffset * (m_nRowHeight + s_HeaderAllDayPadding);

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
			pApp->GetDSDraw ().SetConcatenate (FALSE);
		}
	}

	COleDateTimeSpan spanDay (1, 0, 0, 0);

	int nDay = 0;

	const BOOL bNoDuration = 
		(GetPlanner ()->GetDrawFlags () & BCGP_PLANNER_DRAW_VIEW_NO_DURATION) == 
		BCGP_PLANNER_DRAW_VIEW_NO_DURATION;

	for (int nRes = 0; nRes < m_Resources.GetSize (); nRes++)
	{
		XResource& resource = m_Resources[nRes];

		date = m_DateStart;

		for (nDay = 0; nDay < nDays; nDay ++)
		{
			XBCGPAppointmentArray arAllDays;

			for (int nApp = 0; nApp < 2; nApp++)
			{
				if (!bDragDrop && nApp == 1)
				{
					continue;
				}

				XBCGPAppointmentArray& arApps = nApp == 1 ? arDragApps : arQueryApps;

				XBCGPAppointmentArray arByDate;

				int i;

				CRect rectFill (resource.m_Rects[nDay]);
				rectFill.top -= yOffset;

				if (bNoDuration && nDay > 0)
				{
					rectFill.left++;
				}

				int nStartIndex = 0;
				for (i = 0; i < arApps.GetSize (); i++)
				{
					const CBCGPAppointment* pApp = arApps[i];
					if (pApp != NULL && pApp->GetResourceID () == resource.m_ResourceID)
					{
						nStartIndex = i;
						break;
					}
				}

				for (i = nStartIndex; i < arApps.GetSize (); i++)
				{
					CBCGPAppointment* pApp = arApps[i];
					ASSERT_VALID (pApp);

					if (pApp->GetResourceID () != resource.m_ResourceID)
					{
						break;
					}

					if (CBCGPPlannerView::IsAppointmentInDate (*pApp, date))
					{
						CRect rtApp (rectFill);

						// trimming top and bottom edge, starting from TimeDelta and Appointment interval
						rtApp.top = rectFill.top + m_nRowHeight * 
							long((pApp->GetStart ().GetHour () * 60 + 
								 pApp->GetStart ().GetMinute ()) / nTimeDelta) - 1;

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
							rtApp.right -= 2 * BCGP_PLANNER_APPOINTMENT_SPACE;

							if (pApp->GetStart () != pApp->GetFinish ())
							{
								COleDateTime dtAF (pApp->GetFinish ());
								int minutes = dtAF.GetHour () * 60 + dtAF.GetMinute ();

								if (!IsOneDay (pApp->GetStart (), dtAF))
								{
									BOOL bStartDay   = IsOneDay (date, pApp->GetStart ());
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
											if (pApp->GetDSDraw ().GetCount () == 1)
											{
												pApp->GetDSDraw ()[0].m_date2 = date;
												continue;
											}
										}
									}
									else
									{
										if (!bStartDay)
										{
											rtApp.top = rectFill.top - 1;
										}
									}
								}

								rtApp.bottom = rectFill.top + m_nRowHeight * 
									(long)ceil(minutes / (double)nTimeDelta) - 1;
							}
							else
							{
								rtApp.bottom = rtApp.top + m_nRowHeight;
							}

							pApp->SetRectDraw (rtApp, date);
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

						// remove appointments, that have collisions in rects, from previous column 
						// to the next column
						while (i < ar->GetSize ())
						{
							CBCGPAppointment* pApp2 = ar->GetAt (i);

							CRect rtInter;
							if (rtInter.IntersectRect (pApp1->GetRectDraw (date), pApp2->GetRectDraw (date)))
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
								ASSERT_VALID (pApp);

								CRect rtApp (pApp->GetRectDraw (date));
								rtApp.left = nL;

								pApp->SetRectDraw (rtApp, date);
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
								CRect rtApp (pApp->GetRectDraw (date));

								for (int k = i + 1; k < nCount; k++)
								{
									XBCGPAppointmentArray* arNext = arColumns[k];

									for (int m = 0; m < arNext->GetSize (); m++)
									{
										CBCGPAppointment* pAppNext = arNext->GetAt (m);
									
										CRect rtInter;
										if (rtInter.IntersectRect (rtApp, pAppNext->GetRectDraw (date)))
										{
											rtApp.right = rectFill.left + nWidth * k - 
												(bDrawShadow ? BCGP_PLANNER_APPOINTMENT_SPACE : 0);
											pApp->SetRectDraw (rtApp, date);
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
			}

			// adjust "all day" or "multi day" appointments
			if (arAllDays.GetSize () > 0)
			{
				const int c_Count = (int) arAllDays.GetSize ();

				CRect rectFill (0, 0, 0, 0);

				rectFill.top    = m_nHeaderHeight + 1 - yHeaderOffset;
				rectFill.bottom = rectFill.top + m_nRowHeight;

				int i = 0;

				for (i = 0; i < c_Count; i++)
				{
					CBCGPAppointment* pApp = arAllDays[i];

					rectFill.left  = resource.m_Rects [nDay].left;
					rectFill.right = resource.m_Rects [nDay].right;

					if (CBCGPPlannerView::IsAppointmentInDate (*pApp, date))
					{
						pApp->SetRectDraw (rectFill, date);
					}
				}

				for (i = 1; i < c_Count; i++)
				{
					CBCGPAppointment* pApp1 = arAllDays[i];

					CRect rtApp1;
					if (pApp1->GetDSDraw ().IsEmpty ())
					{
						rtApp1 = pApp1->GetRectDraw ();
					}
					else
					{
						rtApp1 = pApp1->GetDSDraw ().Get (date)->GetRect ();
					}

					for (int j = 0; j < i; j++)
					{
						CBCGPAppointment* pApp2 = arAllDays[j];

						CRect rtApp2;
						if (pApp2->GetDSDraw ().IsEmpty ())
						{
							rtApp2 = pApp2->GetRectDraw ();
						}
						else
						{
							rtApp2 = pApp2->GetDSDraw ().Get (date)->GetRect ();
						}

						CRect rtInter;
						if (rtInter.IntersectRect (rtApp1, rtApp2))
						{
							rtApp1.top    = rtApp2.top;
							rtApp1.bottom = rtApp2.bottom;
							rtApp1.OffsetRect (0, m_nRowHeight + 2);
							
							if (pApp1->GetDSDraw ().IsEmpty ())
							{
								pApp1->SetRectDraw (rtApp1);
							}
							else
							{
								pApp1->GetDSDraw ().Get (date)->SetRect (rtApp1);
							}

							j = 0;
						}
					}
				}
			}

			date += spanDay;
		}
	}	

	date = m_DateStart;

	for (nDay = 0; nDay < nDays; nDay ++)
	{
		CheckVisibleAppointments (date, m_rectApps, FALSE);

		date += spanDay;
	}

	CheckVisibleUpDownIcons(FALSE);

	m_bUpdateToolTipInfo = TRUE;
}

int CBCGPPlannerViewMulti::FindResourceIndexByID (UINT nResourceID) const
{
	int nIndex = -1;

	XResourceCollection& resources = const_cast<XResourceCollection&>(m_Resources);

	for (int i = 0; i < resources.GetSize (); i++)
	{
		if (nResourceID == resources[i].m_ResourceID)
		{
			nIndex = i;
			break;
		}
	}

	return nIndex;
}

void CBCGPPlannerViewMulti::OnPaint (CDC* pDC, const CRect& rectClient)
{
	ASSERT_VALID (pDC);

	OnDrawClient (pDC, m_rectApps);

	OnDrawAppointmentsDuration (pDC);

	if (m_nHeaderAllDayHeight != 0)
	{
		CRect rtHeader (m_rectApps);
		rtHeader.top    -= m_nHeaderAllDayHeight;
		rtHeader.bottom  = rtHeader.top + m_nHeaderAllDayHeight;

		OnDrawHeaderAllDay (pDC, rtHeader);

		CRgn rgn;
		rgn.CreateRectRgn (rtHeader.left, rtHeader.top, rtHeader.right, rtHeader.bottom);

		pDC->SelectClipRgn (&rgn);

		OnDrawAppointments (pDC, rtHeader);

		pDC->SelectClipRgn (NULL);
	}

	{
		CRgn rgn;
		rgn.CreateRectRgn (m_rectApps.left, m_rectApps.top, m_rectApps.right, m_rectApps.bottom);

		pDC->SelectClipRgn (&rgn);
		
		OnDrawAppointments (pDC, m_rectApps);

		pDC->SelectClipRgn (NULL);
	}

	if (!m_rectTimeBar.IsRectEmpty ())
	{
		OnDrawTimeBar (pDC, m_rectTimeBar, IsCurrentTimeVisible ());
	}

	OnDrawUpDownIcons (pDC);

	if (m_nHeaderHeight != 0)
	{
		CRect rtHeader (rectClient);
		rtHeader.left   = m_rectApps.left;
		rtHeader.bottom = rtHeader.top + m_nHeaderHeight / 2;

		OnDrawHeader (pDC, rtHeader);

		rtHeader.top = rtHeader.bottom;
		rtHeader.bottom = rtHeader.top + m_nHeaderHeight / 2;

		OnDrawHeaderResource (pDC, rtHeader);
	}

	InitToolTipInfo ();
}

void CBCGPPlannerViewMulti::OnDrawClient (CDC* pDC, const CRect& rect)
{
	ASSERT_VALID (pDC);

	CRect rectFill (rect);

//	const int nWeekStart = CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () + 1;
	const int yOffset = m_nScrollOffset * m_nRowHeight;

	int nFirstWorkingHour   = GetFirstWorkingHour ();
	int nFirstWorkingMinute = GetFirstWorkingMinute ();
	int nLastWorkingHour    = GetLastWorkingHour ();
	int nLastWorkingMinute  = GetLastWorkingMinute ();

	const int nDays = GetViewDuration ();
	const int nRes  = (int) m_Resources.GetSize ();

	rectFill.OffsetRect (0, -yOffset);

	const int nMinuts = CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ());
	const int nCount = 60 / nMinuts;

	CPen     penHour[2];
	CPen     penHalfHour[2];

	for (int i = 0; i < 2; i++)
	{
		penHour[i].CreatePen (PS_SOLID, 0,
			GetHourLineColor (i == 0 /* Working */, TRUE));

		penHalfHour[i].CreatePen (PS_SOLID, 0, 
			GetHourLineColor (i == 0 /* Working */, FALSE));
	}

	XBCGPPlannerWorkingParameters WorkingParameters(this);
	COLORREF DefaultWorkingColorFill = visualManager->GetPlannerWorkColor ();

	OnFillPlanner (pDC, rect, FALSE /* Non-working */);

	CBrush brHilite (visualManager->GetPlannerSelectionColor (this));

	CPen penBlack (PS_SOLID, 0, visualManager->GetPlannerSeparatorColor (this));
	CPen* pOldPen = pDC->SelectObject (&penBlack);

	const int iStart = GetViewHourOffset();
	const int iEnd   = min (iStart + rect.Height () / m_nRowHeight, nCount * 24);

	COleDateTime dtStart (GetDateStart ());

	BOOL bShowSelection = !((m_Selection[0].GetHour ()   == 0 &&
							 m_Selection[0].GetMinute () == 0 &&
							 m_Selection[0].GetSecond () == 0 &&
							 m_Selection[1].GetHour ()   == 23 &&
							 m_Selection[1].GetMinute () == 59 &&
							 m_Selection[1].GetSecond () == 59) ||
							(m_Selection[1].GetHour ()   == 0 &&
							 m_Selection[1].GetMinute () == 0 &&
							 m_Selection[1].GetSecond () == 0 &&
							 m_Selection[0].GetHour ()   == 23 &&
							 m_Selection[0].GetMinute () == 59 &&
							 m_Selection[0].GetSecond () == 59));

	BOOL bIsDrawDuration = 
		(GetPlanner ()->GetDrawFlags () & BCGP_PLANNER_DRAW_VIEW_NO_DURATION) == 0;
	const int nDurationWidth = bIsDrawDuration ? BCGP_PLANNER_DURATION_BAR_WIDTH + 1 : 0;

	visualManager->PreparePlannerBackItem (FALSE, FALSE);

	const UINT nResourceID = GetCurrentResourceID ();

	int nDay = 0;
	for (nDay = 0; nDay < nDays; nDay++)
	{
		int nWD = dtStart.GetDayOfWeek ();
		BOOL bWeekEnd = nWD == 1 || nWD == 7;

		for (int i = 0; i < nRes; i++)
		{
			XResource& res = m_Resources[i];

			int nFirstHour   = nFirstWorkingHour;
			int nFirstMinute = nFirstWorkingMinute;
			int nLastHour    = nLastWorkingHour;
			int nLastMinute  = nLastWorkingMinute;

			if (res.m_WorkStart < res.m_WorkEnd)
			{
				nFirstHour   = res.m_WorkStart.GetHour ();
				nFirstMinute = res.m_WorkStart.GetMinute ();
				nLastHour    = res.m_WorkEnd.GetHour ();
				nLastMinute  = res.m_WorkEnd.GetMinute ();
			}

			int iWorkStart = nFirstHour * nCount + (int)(nFirstMinute / nMinuts);
			int iWorkEnd   = nLastHour * nCount + (int)(nLastMinute / nMinuts);

			rectFill = res.m_Rects[nDay];

			rectFill.left   += nDurationWidth;
			rectFill.bottom = rectFill.top + m_nRowHeight - 1;

			BCGP_PLANNER_WORKING_STATUS AllPeriodWorkingStatus = 
				GetWorkingPeriodParameters (res.m_ResourceID, dtStart + COleDateTimeSpan (0, 0, iStart * nMinuts, 0), dtStart + COleDateTimeSpan (0, 0, (iEnd * nMinuts) - 1, 59), WorkingParameters); 
			BCGP_PLANNER_WORKING_STATUS SpecificPeriodWorkingStatus = AllPeriodWorkingStatus; 

			for (int iStep = iStart; iStep < iEnd; iStep++)
			{
				BOOL bIsWork = TRUE;
				if (AllPeriodWorkingStatus == BCGP_PLANNER_WORKING_STATUS_UNKNOWN)
				{ // We don't know for the day -> we should see for the period
					COleDateTime CurrentPeriodStart = dtStart + COleDateTimeSpan (0, 0, iStep * nMinuts, 0);
					COleDateTime CurrentPeriodEnd = CurrentPeriodStart + COleDateTimeSpan (0, 0, nMinuts - 1, 59);
					SpecificPeriodWorkingStatus = GetWorkingPeriodParameters (res.m_ResourceID, CurrentPeriodStart, CurrentPeriodEnd, WorkingParameters); 
				}

				switch (SpecificPeriodWorkingStatus)
				{
				case BCGP_PLANNER_WORKING_STATUS_ISNOTWORKING: // not a working period
					bIsWork = FALSE;
					break;
				case BCGP_PLANNER_WORKING_STATUS_ISWORKING: // forced to be a working period (we do not control working hours)
					bIsWork = TRUE;
					break;
				case BCGP_PLANNER_WORKING_STATUS_ISNORMALWORKINGDAY: // regular working day without control of week-end (so it may be a week-end day !)
					bIsWork = (iWorkStart <= iStep && iStep < iWorkEnd);
					break;
				case BCGP_PLANNER_WORKING_STATUS_ISNORMALWORKINGDAYINWEEK: // regular working day in a week (the week end is not a working day)
				default: // could not determine if period is working or not so we calculate as "standard"
					bIsWork = !bWeekEnd && (iWorkStart <= iStep && iStep < iWorkEnd);
				}

				if (!IsDateInSelection (dtStart + 
					COleDateTimeSpan (0, (iStep * nMinuts) / 60, (iStep * nMinuts) % 60, 0)) ||
					!bShowSelection || res.m_ResourceID != nResourceID)
				{
					if (bIsWork)
					{
						if (WorkingParameters.m_clrWorking != CLR_DEFAULT)
						{
							CBrush brush(WorkingParameters.m_clrWorking);
							pDC->FillRect (rectFill, &brush);
						}
						else
						{
							OnFillPlanner (pDC, rectFill, TRUE /* Working */);
						}
					}
					else
					{ // IF non working color is different from default non working color -> we should draw it with new color..
						if ((WorkingParameters.m_clrNonWorking != CLR_DEFAULT) && 
							(WorkingParameters.m_clrNonWorking != DefaultWorkingColorFill))
						{
							CBrush brush(WorkingParameters.m_clrNonWorking);
							pDC->FillRect (rectFill, &brush);
						}
					}
				}
				else
				{
					pDC->FillRect (rectFill, &brHilite);
				}

				int nPenIndex = bIsWork ? 0 : 1;

				pDC->SelectObject (((iStep + 1) % nCount == 0) ? 
					&penHour [nPenIndex] : &penHalfHour [nPenIndex]);

				pDC->MoveTo (rectFill.left, rectFill.bottom);
				pDC->LineTo (rectFill.right, rectFill.bottom);

				rectFill.OffsetRect (0, m_nRowHeight);
			}
		}	

		dtStart += COleDateTimeSpan (1, 0, 0, 0);
	}

	pDC->SelectObject (&penBlack);

	if (bIsDrawDuration)
	{
		for (nDay = 0; nDay < nDays; nDay++)
		{
			for (int i = 0; i < nRes; i++)
			{
				CRect rectDurBar (m_Resources[i].m_Rects[nDay]);
				rectDurBar.right = rectDurBar.left + BCGP_PLANNER_DURATION_BAR_WIDTH;

				// Draw duration bar (at left):
				pDC->FillRect (rectDurBar, &globalData.brWindow);

				if (nDay > 0 || i > 0)
				{
					pDC->MoveTo (rectDurBar.left, rectDurBar.top);
					pDC->LineTo (rectDurBar.left, rectDurBar.bottom);
				}

				pDC->MoveTo (rectDurBar.right, rectDurBar.top);
				pDC->LineTo (rectDurBar.right, rectDurBar.bottom);
			}	
		}
	}
	else
	{
		for (nDay = 0; nDay < nDays; nDay++)
		{
			for (int i = 0; i < nRes; i++)
			{
				CRect rectDurBar (m_Resources[i].m_Rects[nDay]);

				if (nDay > 0 || i > 0)
				{
					pDC->MoveTo (rectDurBar.left, rectDurBar.top);
					pDC->LineTo (rectDurBar.left, rectDurBar.bottom);
				}
			}
		}
	}

	pDC->SelectObject (pOldPen);
}

void CBCGPPlannerViewMulti::OnDrawHeaderResource (CDC* pDC, const CRect& rectHeader)
{
	ASSERT_VALID (pDC);

	const int nDays = GetViewDuration ();

	DWORD dwFlags = GetPlanner ()->GetDrawFlags ();
	BOOL bBold = (dwFlags & BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_BOLD) ==
			BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_BOLD;

	HFONT hOldFont = NULL;
	if (bBold)
	{
		hOldFont = SetCurrFont (pDC, bBold);
	}

	for (int nDay = 0; nDay < nDays; nDay ++)
	{
		CRect rectCaption (rectHeader);
		rectCaption.left = m_ViewRects[nDay].left;
		rectCaption.right = m_ViewRects[nDay].right;

		DrawHeader (pDC, rectCaption, m_Resources [0].m_Rects[nDay].Width ());

		CRect rectResCaption (rectCaption);

		for (int i = 0; i < m_Resources.GetSize (); i++)
		{
			XResource& res = m_Resources[i];

			rectResCaption.left = res.m_Rects[nDay].left;
			rectResCaption.right = res.m_Rects[nDay].right;

			visualManager->PreparePlannerCaptionBackItem (TRUE);
			COLORREF clrText = OnFillPlannerCaption (
				pDC, rectResCaption, FALSE, FALSE, FALSE);

			DrawCaptionText (pDC, rectResCaption, res.m_Description, clrText, 
				res.m_bToolTipNeeded ? DT_LEFT : DT_CENTER);
		}
	}

	if (hOldFont != NULL)
	{
		::SelectObject (pDC->GetSafeHdc (), hOldFont);
	}
}

void CBCGPPlannerViewMulti::OnDrawHeaderAllDay (CDC* pDC, const CRect& rectHeader)
{
	ASSERT_VALID (pDC);

	const int nDays = GetViewDuration ();
	const int nRes  = (int) m_Resources.GetSize ();
	const int nAll  = nDays * nRes;

	visualManager->OnFillPlannerHeaderAllDay (pDC, this, rectHeader);

	int nDay = 0;

	COleDateTime date (GetDateStart ());
	COleDateTimeSpan span (1, 0, 0, 0);

	CRect rt (rectHeader);
	rt.bottom--;

	COleDateTime dayCurrent = COleDateTime::GetCurrentTime ();
	dayCurrent.SetDateTime (dayCurrent.GetYear (), dayCurrent.GetMonth (), 
		dayCurrent.GetDay (), 0, 0, 0);

	if (nAll > 1)
	{
		CPen penBlack (PS_SOLID, 0, visualManager->GetPlannerSeparatorColor (this));
		CPen* pOldPen = pDC->SelectObject (&penBlack);

		for (nDay = 1; nDay < nDays; nDay++)
		{
			pDC->MoveTo (m_ViewRects [nDay].left, rectHeader.top);
			pDC->LineTo (m_ViewRects [nDay].left, rectHeader.bottom - 1);
		}

		for (int i = 1; i < nRes; i++)
		{
			for (nDay = 0; nDay < nDays; nDay++)
			{
				CRect& rectRes = m_Resources[i].m_Rects[nDay];
				pDC->MoveTo (rectRes.left, rectHeader.top);
				pDC->LineTo (rectRes.left, rectHeader.bottom - 1);
			}
		}

		pDC->SelectObject (pOldPen);
	}

	UINT nResourceID = GetCurrentResourceID ();

	for (nDay = 0; nDay < nDays; nDay++)
	{
		for (int i = 0; i < nRes; i++)
		{
			rt.left  = m_Resources [i].m_Rects [nDay].left;
			rt.right = m_Resources [i].m_Rects [nDay].right;

			visualManager->OnDrawPlannerHeaderAllDayItem (pDC, this, rt, 
				FALSE, 
				m_SelectionAllDay[nDay] && m_Resources [i].m_ResourceID == nResourceID);
		}

		if (date == dayCurrent)
		{
			rt.left  = m_ViewRects [nDay].left;
			rt.right = m_ViewRects [nDay].right;

			visualManager->OnDrawPlannerHeaderAllDayItem (pDC, this, rt, TRUE, FALSE);
		}

		date += span;
	}
}

CBCGPPlannerView::BCGP_PLANNER_HITTEST
CBCGPPlannerViewMulti::HitTestArea (const CPoint& point) const
{
	BCGP_PLANNER_HITTEST hit = CBCGPPlannerViewDay::HitTestArea (point);

	if (hit == BCGP_PLANNER_HITTEST_HEADER)
	{
		if (m_nHeaderHeight / 2 <= point.y && point.y < m_nHeaderHeight)
		{
			hit = BCGP_PLANNER_HITTEST_HEADER_RESOURCE;
		}
	}

	return hit;
}

UINT CBCGPPlannerViewMulti::GetCurrentResourceID () const
{
	return m_nResourceID;
}

BOOL CBCGPPlannerViewMulti::SetCurrentResourceID (UINT nResourceID, 
												  BOOL bRedraw/* = TRUE*/, BOOL bNotify/* = FALSE*/)
{
	if (m_nResourceID != nResourceID)
	{
		m_nResourceID = nResourceID;

		if (bRedraw && GetPlanner()->GetSafeHwnd () != NULL)
		{
			GetPlanner()->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}

		if (bNotify)
		{
			CBCGPPlannerManagerCtrl* pPlanner = GetPlanner ();

			if (pPlanner->IsNotifyParent () && pPlanner->GetSafeHwnd () != NULL)
			{
				pPlanner->GetParent ()->SendMessage (BCGP_PLANNER_RESOURCEID_CHANGED, 0, 0);
			}
		}
	}

	return TRUE;
}

UINT CBCGPPlannerViewMulti::GetResourceFromPoint (const CPoint& point) const
{
	UINT nResourceID = GetCurrentResourceID ();

	XResourceCollection& resources = const_cast<XResourceCollection&>(m_Resources);

	for (int i = 0; i < m_Resources.GetSize (); i++)
	{
		CArray<CRect, CRect&>& rects = resources[i].m_Rects;
		for (int j = 0; j < rects.GetSize (); j++)
		{
			if (rects[j].left <= point.x && point.x < rects[j].right &&
				m_nHeaderHeight / 2 <= point.y)
			{
				nResourceID = resources[i].m_ResourceID;
				break;
			}
		}
	}

	return nResourceID;
}

CRect CBCGPPlannerViewMulti::GetRectFromDate(const COleDateTime& date) const
{
	CRect rect(0, 0, 0, 0);

	if (date < GetDateStart() || GetDateEnd() < date)
	{
		return rect;
	}

	int nResourceIndex = FindResourceIndexByID (GetCurrentResourceID ());
	if (nResourceIndex == -1)
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
	if (nIndex < 0 || nDays <= nIndex)
	{
		return rect;
	}

	rect = const_cast<XResourceCollection&>(m_Resources)[nResourceIndex].m_Rects[nIndex];
	rect.top += ((date.GetHour() * 60 + date.GetMinute()) / CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ()) - m_nScrollOffset) * m_nRowHeight;
	rect.bottom = rect.top + m_nRowHeight;

	return rect;
}

BOOL CBCGPPlannerViewMulti::OnUpdateStorage ()
{
	if (GetPlanner () == NULL)
	{
		return FALSE;
	}

	CBCGPAppointmentBaseMultiStorage* pStorage = 
		(CBCGPAppointmentBaseMultiStorage*)GetPlanner ()->GetStorage ();

	CBCGPAppointmentBaseMultiStorage::XResourceIDArray ar;
	pStorage->GetResourceIDs (ar);

	const int count = (int) ar.GetSize ();
	ASSERT (count > 0);

	BOOL bRes = FALSE;

	if (count != m_Resources.GetSize ())
	{
		m_Resources.SetSize (count);
		bRes= TRUE;
	}

	for (int i = 0; i < count; i++)
	{
		XResource& resource = m_Resources[i];

		if (resource.m_ResourceID != ar[i])
		{
			resource.m_ResourceID = ar[i];
			bRes = TRUE;
		}

		const CBCGPAppointmentBaseResourceInfo* pInfo = pStorage->GetResourceInfo (ar[i]);

		if (pInfo != NULL)
		{
			if (resource.m_Description != pInfo->GetDescription ())
			{
				resource.m_Description = pInfo->GetDescription ();
				bRes = TRUE;
			}
			if (resource.m_WorkStart != pInfo->GetWorkStart ())
			{
				resource.m_WorkStart = pInfo->GetWorkStart ();
				bRes = TRUE;
			}
			if (resource.m_WorkEnd != pInfo->GetWorkEnd ())
			{
				resource.m_WorkEnd = pInfo->GetWorkEnd ();
				bRes = TRUE;
			}
		}
	}

	return bRes;
}

void CBCGPPlannerViewMulti::InitViewToolTipInfo ()
{
	CBCGPPlannerViewDay::InitViewToolTipInfo ();

	for (int nRes = 0; nRes < m_Resources.GetSize (); nRes++)
	{
		XResource& res = m_Resources[nRes];

		if (!res.m_bToolTipNeeded)
		{
			continue;
		}

		for (int i = 0; i < res.m_Rects.GetSize (); i++)
		{
			CRect rect (res.m_Rects[i]);
			rect.top    = m_nHeaderHeight / 2;
			rect.bottom = m_nHeaderHeight;
			AddToolTipInfo (rect);
		}
	}
}

CString CBCGPPlannerViewMulti::GetToolTipText (const CPoint& point)
{
	if (HitTestArea (point) == BCGP_PLANNER_HITTEST_HEADER_RESOURCE)
	{
		UINT nResourceID = GetResourceFromPoint (point);
		if (nResourceID != CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID)
		{
			return m_Resources[FindResourceIndexByID (nResourceID)].m_Description;
		}
	}

	return CBCGPPlannerViewDay::GetToolTipText (point);
}

CString CBCGPPlannerViewMulti::GetAccName() const
{
	CString strValue(CBCGPPlannerViewDay::GetAccName());

	const CBCGPAppointmentBaseMultiStorage* pStorage = 
		DYNAMIC_DOWNCAST(CBCGPAppointmentBaseMultiStorage, GetPlanner ()->GetStorage ());

	const CBCGPAppointmentBaseResourceInfo* pInfo = pStorage->GetResourceInfo (GetCurrentResourceID ());
	if (pInfo != NULL && !pInfo->GetDescription ().IsEmpty ())
	{
		strValue = pInfo->GetDescription () + _T(". ") + strValue;
	}

	return strValue;
}

#endif // BCGP_EXCLUDE_PLANNER
