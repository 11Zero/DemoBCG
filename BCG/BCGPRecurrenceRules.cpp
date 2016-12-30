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
// BCGPRecurrenceRules.cpp: implementation of the CBCGPRecurrenceRule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPRecurrenceRules.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerManagerCtrl.h"
#include "BCGPCalendarBar.h"
#include "BCGPPlannerView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CBCGPRecurrenceRuleDaily, CBCGPRecurrenceBaseRule, VERSIONABLE_SCHEMA | 2)

CBCGPRecurrenceRuleDaily::CBCGPRecurrenceRuleDaily ()
	: m_Type   (BCGP_REC_RULE_DAILY_TYPE_FIRST)
	, m_dwSpan (1)
{
}

CBCGPRecurrenceRuleDaily::~CBCGPRecurrenceRuleDaily ()
{
}

void CBCGPRecurrenceRuleDaily::CorrectStart ()
{
	if (m_Type == BCGP_REC_RULE_DAILY_TYPE_WEEKDAYS)
	{
		COleDateTime dt (GetDateStart ());
		int nWD = dt.GetDayOfWeek ();

		if (nWD == 1)
		{
			dt += COleDateTimeSpan (1, 0, 0, 0);
		}
		else if (nWD == 7)
		{
			dt += COleDateTimeSpan (2, 0, 0, 0);
		}

		if (dt != GetDateStart ())
		{
			SetDateStart (dt);
		}
	}
}

COleDateTime CBCGPRecurrenceRuleDaily::GetSiblingEventDay (const COleDateTime& dtSibling) const
{
	return CBCGPRecurrenceBaseRule::GetSiblingEventDay (dtSibling);
}

COleDateTime CBCGPRecurrenceRuleDaily::GetNextEventDay (const COleDateTime& dtPrev) const
{
	if (dtPrev < GetDateStart ())
	{
		return COleDateTime ();
	}

	COleDateTime dt (dtPrev);
	
	if (m_Type == BCGP_REC_RULE_DAILY_TYPE_WEEKDAYS)
	{
		dt += COleDateTimeSpan (1, 0, 0, 0);

		int nWD = dt.GetDayOfWeek ();

		if (nWD == 1)
		{
			dt += COleDateTimeSpan (1, 0, 0, 0);
		}
		else if (nWD == 7)
		{
			dt += COleDateTimeSpan (2, 0, 0, 0);
		}
	}
	else
	{
		dt += COleDateTimeSpan (m_dwSpan, 0, 0, 0);
	}

	BCGP_RECURRENCE_RULE_LIMIT limitType = GetLimitType ();

	if (limitType == BCGP_RECURRENCE_RULE_LIMIT_COUNT)
	{
		if (m_Type == BCGP_REC_RULE_DAILY_TYPE_WEEKDAYS)
		{
			COleDateTime dtF (
				CBCGPPlannerView::GetFirstWeekDay (GetDateStart (), 
					2));

			int nDaysCount = (dt - GetDateStart ()).GetDays () + 1;
			int nWeekCount = (dt - dtF).GetDays () / 7;

			if ((nDaysCount - nWeekCount * 2) > (int)GetLimitCount ())
			{
				dt = COleDateTime ();
			}
		}
		else
		{
			if ((GetDateStart () + 
				 COleDateTimeSpan (m_dwSpan * GetLimitCount (), 0, 0, 0)) <= dt)
			{
				dt = COleDateTime ();
			}
		}
	}
	else if (limitType == BCGP_RECURRENCE_RULE_LIMIT_DATE)
	{
		if (GetLimitDate () < dt)
		{
			dt = COleDateTime ();
		}
	}

	return dt;
}

void CBCGPRecurrenceRuleDaily::SetSpan (DWORD dwSpan)
{
	ASSERT (dwSpan > 0);

	if (m_dwSpan != dwSpan)
	{
		m_dwSpan = dwSpan;
	}
}

void CBCGPRecurrenceRuleDaily::SetType (BCGP_REC_RULE_DAILY_TYPE type)
{
	ASSERT (BCGP_REC_RULE_DAILY_TYPE_FIRST <= type && 
		type <= BCGP_REC_RULE_DAILY_TYPE_LAST);

	if (m_Type != type)
	{
		m_Type = type;
	}
}

void CBCGPRecurrenceRuleDaily::Serialize (CArchive& ar)
{
	CBCGPRecurrenceBaseRule::Serialize (ar);

	if (ar.IsStoring ())
	{
		ar << (DWORD)m_Type;
		ar << m_dwSpan;
	}
	else
	{
		DWORD type;
		ar >> type;
		m_Type = (BCGP_REC_RULE_DAILY_TYPE)type;

		ar >> m_dwSpan;

		// for scheme #1
		if (GetID() == 0)
		{
			SetID(BCGP_PLANNER_RULE_DAILY);
		}
	}
}

void CBCGPRecurrenceRuleDaily::GetProperties (CBCGPAppointmentPropertyList& props) const
{
	CBCGPRecurrenceBaseRule::GetProperties (props);

	props.Add (BCGP_REC_RULE_PROPERTY_DAILY_TYPE, new CBCGPAppointmentProperty ((DWORD)m_Type));
	props.Add (BCGP_REC_RULE_PROPERTY_DAILY_SPAN, new CBCGPAppointmentProperty (m_dwSpan));
}

void CBCGPRecurrenceRuleDaily::SetProperties (const CBCGPAppointmentPropertyList& props)
{
	CBCGPRecurrenceBaseRule::SetProperties (props);

	if (props.GetCount () > 0)
	{
		CBCGPAppointmentProperty* prop = NULL;

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_DAILY_TYPE))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_DAILY_TYPE);
			m_Type = (BCGP_REC_RULE_DAILY_TYPE)((DWORD)*prop);
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_DAILY_SPAN))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_DAILY_SPAN);
			m_dwSpan = *prop;
		}
	}
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CBCGPRecurrenceRuleWeekly, CBCGPRecurrenceBaseRule, VERSIONABLE_SCHEMA | 2)

CBCGPRecurrenceRuleWeekly::CBCGPRecurrenceRuleWeekly ()
	: m_dwSpan   (1)
	, m_dwDays   (BCGP_REC_RULE_WEEKLY_FIRST)
	, m_FirstDay (BCGP_REC_RULE_WEEKLY_FIRST)
	, m_LastDay  (BCGP_REC_RULE_WEEKLY_FIRST)
	, m_nCount   (1)
{
}

CBCGPRecurrenceRuleWeekly::~CBCGPRecurrenceRuleWeekly ()
{
}

int CBCGPRecurrenceRuleWeekly::TypeToDayOfWeek (BCGP_REC_RULE_WEEKLY type) const
{
	ASSERT(BCGP_REC_RULE_WEEKLY_FIRST <= type && type <= BCGP_REC_RULE_WEEKLY_LAST);
	
	int day = 0;

	switch(type)
	{
	case BCGP_REC_RULE_WEEKLY_1:
		day = 2;
		break;
	case BCGP_REC_RULE_WEEKLY_2:
		day = 3;
		break;
	case BCGP_REC_RULE_WEEKLY_3:
		day = 4;
		break;
	case BCGP_REC_RULE_WEEKLY_4:
		day = 5;
		break;
	case BCGP_REC_RULE_WEEKLY_5:
		day = 6;
		break;
	case BCGP_REC_RULE_WEEKLY_6:
		day = 7;
		break;
	case BCGP_REC_RULE_WEEKLY_7:
		day = 1;
		break;
	};

	return day;
}

CBCGPRecurrenceRuleWeekly::BCGP_REC_RULE_WEEKLY
CBCGPRecurrenceRuleWeekly::DayOfWeekToType (int day) const
{
	ASSERT(1 <= day && day <= 7);
	
	BCGP_REC_RULE_WEEKLY type = BCGP_REC_RULE_WEEKLY_FIRST;

	switch(day)
	{
	case 2:
		type = BCGP_REC_RULE_WEEKLY_1;
		break;
	case 3:
		type = BCGP_REC_RULE_WEEKLY_2;
		break;
	case 4:
		type = BCGP_REC_RULE_WEEKLY_3;
		break;
	case 5:
		type = BCGP_REC_RULE_WEEKLY_4;
		break;
	case 6:
		type = BCGP_REC_RULE_WEEKLY_5;
		break;
	case 7:
		type = BCGP_REC_RULE_WEEKLY_6;
		break;
	case 1:
		type = BCGP_REC_RULE_WEEKLY_7;
		break;
	};

	return type;
}

CBCGPRecurrenceRuleWeekly::BCGP_REC_RULE_WEEKLY
CBCGPRecurrenceRuleWeekly::GetFirstDay () const
{
	BCGP_REC_RULE_WEEKLY firstType = BCGP_REC_RULE_WEEKLY_FIRST;

	for (DWORD dwD = BCGP_REC_RULE_WEEKLY_FIRST; dwD <= BCGP_REC_RULE_WEEKLY_LAST; dwD = dwD << 1)
	{
		if ((m_dwDays & dwD) != 0)
		{
			firstType = (BCGP_REC_RULE_WEEKLY)dwD;
			break;
		}
	}

	return firstType;
}

CBCGPRecurrenceRuleWeekly::BCGP_REC_RULE_WEEKLY
CBCGPRecurrenceRuleWeekly::GetNextDay (BCGP_REC_RULE_WEEKLY type) const
{
	BCGP_REC_RULE_WEEKLY nextType = BCGP_REC_RULE_WEEKLY_FIRST;

	for (DWORD dwD = DWORD(type) << 1; dwD <= BCGP_REC_RULE_WEEKLY_LAST; dwD = dwD << 1)
	{
		if ((m_dwDays & dwD) != 0)
		{
			nextType = (BCGP_REC_RULE_WEEKLY)dwD;
			break;
		}
	}

	return nextType;
}

CBCGPRecurrenceRuleWeekly::BCGP_REC_RULE_WEEKLY
CBCGPRecurrenceRuleWeekly::GetLastDay () const
{
	BCGP_REC_RULE_WEEKLY firstType = GetFirstDay ();
	BCGP_REC_RULE_WEEKLY lastType  = firstType;

	for (DWORD dwD = DWORD(firstType) << 1; dwD <= BCGP_REC_RULE_WEEKLY_LAST; dwD = dwD << 1)
	{
		if ((m_dwDays & dwD) != 0)
		{
			lastType = (BCGP_REC_RULE_WEEKLY)dwD;
		}
	}

	return lastType;
}

CBCGPRecurrenceRuleWeekly::BCGP_REC_RULE_WEEKLY
CBCGPRecurrenceRuleWeekly::GetStartDay () const
{
	return GetFirstDay ();
}

int CBCGPRecurrenceRuleWeekly::GetDayCount () const
{
	int dwCount = 0;

	for (DWORD dwD = BCGP_REC_RULE_WEEKLY_FIRST; dwD <= BCGP_REC_RULE_WEEKLY_LAST; dwD = dwD << 1)
	{
		if ((m_dwDays & dwD) != 0)
		{
			dwCount++;
		}
	}	

	return dwCount;
}

void CBCGPRecurrenceRuleWeekly::CorrectStart ()
{
	COleDateTime dt (GetDateStart ());

	int nWD = dt.GetDayOfWeek () - 1;

	int nWDS = TypeToDayOfWeek (GetStartDay ()) - 1;

	if (nWD == 0)
	{
		nWD = 7;
	}

	if (nWDS == 0)
	{
		nWDS = 7;
	}

	if (nWD != nWDS)
	{
		int delta = nWDS - nWD;
		if (delta < 0)
		{
			delta += 7;
		}

		dt += COleDateTimeSpan (delta, 0, 0, 0);
	}

	if (dt != GetDateStart ())
	{
		SetDateStart (dt);
	}
}

COleDateTime CBCGPRecurrenceRuleWeekly::GetSiblingEventDay (const COleDateTime& dtSibling) const
{
	return CBCGPRecurrenceBaseRule::GetSiblingEventDay (dtSibling);
}

COleDateTime CBCGPRecurrenceRuleWeekly::GetNextEventDay (const COleDateTime& dtPrev) const
{
	if (dtPrev < GetDateStart ())
	{
		return COleDateTime ();
	}

	COleDateTime dt (dtPrev);

	int nWD  = dt.GetDayOfWeek ();
	BCGP_REC_RULE_WEEKLY typeWD = DayOfWeekToType (nWD);
	nWD--;

	int nWDL = TypeToDayOfWeek (m_LastDay) - 1;

	if (nWD == 0)
	{
		nWD = 7;
	}

	if (nWDL == 0)
	{
		nWDL = 7;
	}

	if (nWD == nWDL)
	{
		COleDateTime dtFirst (CBCGPPlannerView::GetFirstWeekDay (dtPrev, 
						2));//CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () + 1));

		int nWDF = TypeToDayOfWeek (m_FirstDay) - 1;
		if (nWDF == 0)
		{
			nWDF = 7;
		}

		dt = dtFirst + COleDateTimeSpan (m_dwSpan * 7 + nWDF - 1, 0, 0, 0);
	}
	else
	{
		int nWDN = TypeToDayOfWeek (GetNextDay (typeWD)) - 1;

		if (nWDN == 0)
		{
			nWDN = 7;
		}

		int delta = nWDN - nWD;
		if (delta < 0)
		{
			delta += 7;
		}

		dt += COleDateTimeSpan (delta, 0, 0, 0);
	}


	BCGP_RECURRENCE_RULE_LIMIT limitType = GetLimitType ();

	if (limitType == BCGP_RECURRENCE_RULE_LIMIT_COUNT)
	{
		int nCount = GetLimitCount () % m_nCount;

		int delta  = GetLimitCount () / m_nCount;
		delta = delta * 7 * m_dwSpan;

		if (nCount > 0)
		{
			int i = 0;
			int c = 0;
			for (DWORD dwD = BCGP_REC_RULE_WEEKLY_FIRST; dwD <= BCGP_REC_RULE_WEEKLY_LAST; dwD = dwD << 1)
			{
				c++;

				if ((m_dwDays & dwD) != 0)
				{
					i++;

					if (i == nCount)
					{
						break;
					}
				}	
			}

			delta += c;
		}

		if (delta <= (dt - CBCGPPlannerView::GetFirstWeekDay (GetDateStart (), 
						2)))
		{
			dt = COleDateTime ();
		}
	}
	else if (limitType == BCGP_RECURRENCE_RULE_LIMIT_DATE)
	{
		if (dt > GetLimitDate ())
		{
			dt = COleDateTime ();
		}
	}

	return dt;
}

void CBCGPRecurrenceRuleWeekly::SetSpan (DWORD dwSpan)
{
	ASSERT (dwSpan > 0);

	if (m_dwSpan != dwSpan)
	{
		m_dwSpan = dwSpan;
	}
}

void CBCGPRecurrenceRuleWeekly::SetDays (DWORD dwDays)
{
	ASSERT (dwDays > 0);

	if (m_dwDays != dwDays && dwDays != 0)
	{
		m_dwDays = dwDays;

		m_FirstDay = GetFirstDay ();
		m_LastDay  = GetLastDay ();

		m_nCount   = GetDayCount ();
	}
}

void CBCGPRecurrenceRuleWeekly::Serialize (CArchive& ar)
{
	CBCGPRecurrenceBaseRule::Serialize (ar);

	if (ar.IsStoring ())
	{
		ar << m_dwSpan;
		ar << m_dwDays;
	}
	else
	{
		ar >> m_dwSpan;

		DWORD dwDays = 0;
		ar >> dwDays;
		SetDays (dwDays);

		// for scheme #1
		if (GetID() == 0)
		{
			SetID(BCGP_PLANNER_RULE_WEEKLY);
		}
	}
}

void CBCGPRecurrenceRuleWeekly::GetProperties (CBCGPAppointmentPropertyList& props) const
{
	CBCGPRecurrenceBaseRule::GetProperties (props);

	props.Add (BCGP_REC_RULE_PROPERTY_WEEKLY_SPAN, new CBCGPAppointmentProperty (m_dwSpan));
	props.Add (BCGP_REC_RULE_PROPERTY_WEEKLY_DAYS, new CBCGPAppointmentProperty (m_dwDays));
}

void CBCGPRecurrenceRuleWeekly::SetProperties (const CBCGPAppointmentPropertyList& props)
{
	CBCGPRecurrenceBaseRule::SetProperties (props);

	if (props.GetCount () > 0)
	{
		CBCGPAppointmentProperty* prop = NULL;

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_WEEKLY_SPAN))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_WEEKLY_SPAN);
			m_dwSpan = *prop;
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_WEEKLY_DAYS))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_WEEKLY_DAYS);
			SetDays (*prop);
		}
	}
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CBCGPRecurrenceRuleMonthly, CBCGPRecurrenceBaseRule, VERSIONABLE_SCHEMA | 2)

CBCGPRecurrenceRuleMonthly::CBCGPRecurrenceRuleMonthly ()
	: m_Type           (BCGP_REC_RULE_MONTHLY_TYPE_FIRST)
	, m_dwDay          (1)
	, m_dwDaySpan      (1)
	, m_DayTypeDay     (BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_FIRST)
	, m_DayTypeWeekDay (BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_FIRST)
	, m_dwDayTypeSpan  (1)
{
}

CBCGPRecurrenceRuleMonthly::~CBCGPRecurrenceRuleMonthly ()
{
}

int CBCGPRecurrenceRuleMonthly::GetPossibleDay (int m, int y) const
{
	int d = 1;

	if (m_Type == BCGP_REC_RULE_MONTHLY_TYPE_DAY)
	{
		d = m_dwDay;
	}
	else
	{
		if (m_DayTypeWeekDay == BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_DAY)
		{
			if (m_DayTypeDay == BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_L)
			{
				d = 32;
			}
			else
			{
				d = (int)m_DayTypeDay + 1;
			}
		}
		else if (m_DayTypeWeekDay == BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_WEEKDAY)
		{
			d = m_DayTypeDay == BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_L
					? CBCGPCalendar::GetMaxMonthDay (m, y)
					: 1;
			COleDateTime dt (y, m, d, 0, 0, 0);

			int nWDC = dt.GetDayOfWeek () - 1;
			if (nWDC == 0)
			{
				nWDC = 7;
			}

			if (m_DayTypeDay == BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_L)
			{	
				if (nWDC > 5)
				{
					d -= nWDC - 5;
				}
			}
			else
			{
				if (nWDC > 5)
				{
					d += 8 - nWDC; // first weekday
					nWDC = 1;
				}

				int nSpan = (int)m_DayTypeDay;
				if ((nWDC + nSpan) > 5)
				{
					d += 7 - nWDC;
					nSpan += nWDC - 5;
				}

				d += nSpan;
			}
		}
		else if (m_DayTypeWeekDay == BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_WEEKEND)
		{
			d = m_DayTypeDay == BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_L
					? CBCGPCalendar::GetMaxMonthDay (m, y)
					: 1;
			COleDateTime dt (y, m, d, 0, 0, 0);

			int nWDC = dt.GetDayOfWeek () - 1;
			if (nWDC == 0)
			{
				nWDC = 7;
			}

			if (m_DayTypeDay == BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_L)
			{
				if (nWDC < 6)
				{
					d -= nWDC;
				}
			}
			else
			{
				if (nWDC < 6)
				{
					d += 6 - nWDC;
					nWDC = 6;
				}

				if (m_DayTypeDay > BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_1)
				{
					if (nWDC == 6)
					{
						if (m_DayTypeDay >= BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_3)
						{
							d += m_DayTypeDay == BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_4 ? 8 : 7;
						}
						else
						{
							d++;
						}
					}
					else
					{
						d += 6;

						if (m_DayTypeDay >= BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_3)
						{
							d += m_DayTypeDay == BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_4 ? 7 : 1;
						}
					}
				}
			}
		}
		else
		{
			d = m_DayTypeDay == BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_L
					? CBCGPCalendar::GetMaxMonthDay (m, y)
					: 1;
			COleDateTime dt (y, m, d, 0, 0, 0);

			int nWD = 0;

			switch(m_DayTypeWeekDay)
			{
			case BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_1:
				nWD = 1;
				break;
			case BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_2:
				nWD = 2;
				break;
			case BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_3:
				nWD = 3;
				break;
			case BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_4:
				nWD = 4;
				break;
			case BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_5:
				nWD = 5;
				break;
			case BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_6:
				nWD = 6;
				break;
			case BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_7:
				nWD = 7;
				break;
			};

			int nWDC = dt.GetDayOfWeek () - 1;
			if (nWDC == 0)
			{
				nWDC = 7;
			}

			if (m_DayTypeDay == BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_L)
			{
				if (nWD > nWDC)
				{
					d -= 7;
				}
			}
			else
			{
				if (nWD < nWDC)
				{
					d += 7;
				}

				d += 7 * (int)m_DayTypeDay;
			}

			d += nWD - nWDC;
		}
	}

	return d;
}

void CBCGPRecurrenceRuleMonthly::CorrectStart ()
{
	COleDateTime dt (GetDateStart ());

	int m = dt.GetMonth ();
	int y = dt.GetYear ();

	int d = GetPossibleDay (m, y);

	if (d < dt.GetDay ())
	{
		m++;

		if (m == 13)
		{
			m = 1;
			y++;
		}

		d = GetPossibleDay (m, y);
	}

	int nDays = CBCGPCalendar::GetMaxMonthDay (m, y);

	dt = COleDateTime (y, m, nDays < d ? nDays : d, 0, 0, 0);

	if (dt != GetDateStart ())
	{
		SetDateStart (dt);
	}
}

COleDateTime CBCGPRecurrenceRuleMonthly::GetSiblingEventDay (const COleDateTime& dtSibling) const
{
	return CBCGPRecurrenceBaseRule::GetSiblingEventDay (dtSibling);
}

COleDateTime CBCGPRecurrenceRuleMonthly::GetNextEventDay (const COleDateTime& dtPrev) const
{
	if (dtPrev < GetDateStart ())
	{
		return COleDateTime ();
	}

	COleDateTime dt (dtPrev);

	DWORD dwSpan = m_Type == BCGP_REC_RULE_MONTHLY_TYPE_DAY ? m_dwDaySpan : m_dwDayTypeSpan;

	{
		int m = dt.GetMonth ();
		int y = dt.GetYear ();
		int nDays = 0;

		dt = COleDateTime (y, m, 1, 0, 0, 0);

		for (DWORD i = 0; i < dwSpan; i++)
		{
			m++;

			if (m == 13)
			{
				m = 1;
				y++;
			}

			nDays = CBCGPCalendar::GetMaxMonthDay (m, y);
			dt += COleDateTimeSpan (nDays, 0, 0, 0);
		}

		int d = GetPossibleDay (m, y);

		dt = COleDateTime (y, m, nDays < d ? nDays : d, 0, 0, 0);
	}

	BCGP_RECURRENCE_RULE_LIMIT limitType = GetLimitType ();

	if (limitType == BCGP_RECURRENCE_RULE_LIMIT_COUNT)
	{
		if (m_Type == BCGP_REC_RULE_MONTHLY_TYPE_DAY)
		{
			int m = ((dt.GetYear () - GetDateStart ().GetYear ()) * 12 +
				(dt.GetMonth () - GetDateStart ().GetMonth () + dwSpan)) / dwSpan;
			
			if ((int)GetLimitCount () < m)
			{
				dt = COleDateTime ();
			}
		}
	}
	else if (limitType == BCGP_RECURRENCE_RULE_LIMIT_DATE)
	{
		if (GetLimitDate () < dt)
		{
			dt = COleDateTime ();
		}
	}

	return dt;
}

void CBCGPRecurrenceRuleMonthly::SetType (BCGP_REC_RULE_MONTHLY_TYPE type)
{
	ASSERT (BCGP_REC_RULE_MONTHLY_TYPE_FIRST <= type && 
		type <= BCGP_REC_RULE_MONTHLY_TYPE_LAST);

	if (m_Type != type)
	{
		m_Type = type;
	}
}

void CBCGPRecurrenceRuleMonthly::SetDay (DWORD dwDay)
{
	ASSERT (1 <= dwDay && dwDay <= 31);

	if (m_dwDay != dwDay)
	{
		m_dwDay = dwDay;
	}
}

void CBCGPRecurrenceRuleMonthly::SetDaySpan (DWORD dwDaySpan)
{
	ASSERT (dwDaySpan > 0);

	if (m_dwDaySpan != dwDaySpan)
	{
		m_dwDaySpan = dwDaySpan;
	}
}

void CBCGPRecurrenceRuleMonthly::SetDayTypeDay (BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY DayTypeDay)
{
	ASSERT (BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_FIRST <= DayTypeDay && 
		DayTypeDay <= BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_LAST);

	if (m_DayTypeDay != DayTypeDay)
	{
		m_DayTypeDay = DayTypeDay;
	}
}

void CBCGPRecurrenceRuleMonthly::SetDayTypeWeekDay (BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY DayTypeWeekDay)
{
	ASSERT (BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_FIRST <= DayTypeWeekDay && 
		DayTypeWeekDay <= BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_LAST);

	if (m_DayTypeWeekDay != DayTypeWeekDay)
	{
		m_DayTypeWeekDay = DayTypeWeekDay;
	}
}

void CBCGPRecurrenceRuleMonthly::SetDayTypeSpan (DWORD dwDayTypeSpan)
{
	ASSERT (dwDayTypeSpan > 0);

	if (m_dwDayTypeSpan != dwDayTypeSpan)
	{
		m_dwDayTypeSpan = dwDayTypeSpan;
	}
}

void CBCGPRecurrenceRuleMonthly::Serialize (CArchive& ar)
{
	CBCGPRecurrenceBaseRule::Serialize (ar);

	if (ar.IsStoring ())
	{
		ar << (DWORD)m_Type;
		
		ar << m_dwDay;
		ar << m_dwDaySpan;

		ar << (DWORD)m_DayTypeDay;
		ar << (DWORD)m_DayTypeWeekDay;
		ar << m_dwDayTypeSpan;
	}
	else
	{
		DWORD type;
		ar >> type;
		m_Type = (BCGP_REC_RULE_MONTHLY_TYPE)type;

		ar >> m_dwDay;
		ar >> m_dwDaySpan;

		ar >> type;
		m_DayTypeDay = (BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY)type;
		ar >> type;
		m_DayTypeWeekDay = (BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY)type;
		ar >> m_dwDayTypeSpan;

		// for scheme #1
		if (GetID() == 0)
		{
			SetID(BCGP_PLANNER_RULE_MONTHLY);
		}
	}
}

void CBCGPRecurrenceRuleMonthly::GetProperties (CBCGPAppointmentPropertyList& props) const
{
	CBCGPRecurrenceBaseRule::GetProperties (props);

	props.Add (BCGP_REC_RULE_PROPERTY_MONTHLY_TYPE, new CBCGPAppointmentProperty ((DWORD)m_Type));
	props.Add (BCGP_REC_RULE_PROPERTY_MONTHLY_DAY, new CBCGPAppointmentProperty (m_dwDay));
	props.Add (BCGP_REC_RULE_PROPERTY_MONTHLY_DAY_SPAN, new CBCGPAppointmentProperty (m_dwDaySpan));
	props.Add (BCGP_REC_RULE_PROPERTY_MONTHLY_DAYTYPE_DAY, new CBCGPAppointmentProperty ((DWORD)m_DayTypeDay));
	props.Add (BCGP_REC_RULE_PROPERTY_MONTHLY_DAYTYPE_WEEKDAY, new CBCGPAppointmentProperty ((DWORD)m_DayTypeWeekDay));
	props.Add (BCGP_REC_RULE_PROPERTY_MONTHLY_DAYTYPE_SPAN, new CBCGPAppointmentProperty (m_dwDayTypeSpan));
}

void CBCGPRecurrenceRuleMonthly::SetProperties (const CBCGPAppointmentPropertyList& props)
{
	CBCGPRecurrenceBaseRule::SetProperties (props);

	if (props.GetCount () > 0)
	{
		CBCGPAppointmentProperty* prop = NULL;

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_MONTHLY_TYPE))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_MONTHLY_TYPE);
			m_Type = (BCGP_REC_RULE_MONTHLY_TYPE)((DWORD)*prop);
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_MONTHLY_DAY))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_MONTHLY_DAY);
			m_dwDay = *prop;
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_MONTHLY_DAY_SPAN))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_MONTHLY_DAY_SPAN);
			m_dwDaySpan = *prop;
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_MONTHLY_DAYTYPE_DAY))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_MONTHLY_DAYTYPE_DAY);
			m_DayTypeDay = (BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY)((DWORD)*prop);
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_MONTHLY_DAYTYPE_WEEKDAY))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_MONTHLY_DAYTYPE_WEEKDAY);
			m_DayTypeWeekDay = (BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY)((DWORD)*prop);
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_MONTHLY_DAYTYPE_SPAN))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_MONTHLY_DAYTYPE_SPAN);
			m_dwDayTypeSpan = *prop;
		}
	}
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CBCGPRecurrenceRuleYearly, CBCGPRecurrenceBaseRule, VERSIONABLE_SCHEMA | 2)

CBCGPRecurrenceRuleYearly::CBCGPRecurrenceRuleYearly ()
	: m_Type           (BCGP_REC_RULE_YEARLY_TYPE_FIRST)
	, m_dwDayMonth     (1)
	, m_dwDay          (1)
	, m_DayTypeDay     (BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_FIRST)
	, m_DayTypeWeekDay (BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_FIRST)
	, m_dwDayTypeMonth (1)
{
}

CBCGPRecurrenceRuleYearly::~CBCGPRecurrenceRuleYearly ()
{
}

int CBCGPRecurrenceRuleYearly::GetPossibleDay (int y) const
{
	int d = 1;

	if (m_Type == BCGP_REC_RULE_YEARLY_TYPE_DAY)
	{
		d = m_dwDay;
	}
	else
	{
		int m = m_dwDayTypeMonth;

		if (m_DayTypeWeekDay == BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_DAY)
		{
			if (m_DayTypeDay == BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_L)
			{
				d = 32;
			}
			else
			{
				d = (int)m_DayTypeDay + 1;
			}
		}
		else if (m_DayTypeWeekDay == BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_WEEKDAY)
		{
			d = m_DayTypeDay == BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_L
					? CBCGPCalendar::GetMaxMonthDay (m, y)
					: 1;
			COleDateTime dt (y, m, d, 0, 0, 0);

			int nWDC = dt.GetDayOfWeek () - 1;
			if (nWDC == 0)
			{
				nWDC = 7;
			}

			if (m_DayTypeDay == BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_L)
			{	
				if (nWDC > 5)
				{
					d -= nWDC - 5;
				}
			}
			else
			{
				if (nWDC > 5)
				{
					d += 8 - nWDC; // first weekday
					nWDC = 1;
				}

				int nSpan = (int)m_DayTypeDay;
				if ((nWDC + nSpan) > 5)
				{
					d += 7 - nWDC;
					nSpan += nWDC - 5;
				}

				d += nSpan;
			}
		}
		else if (m_DayTypeWeekDay == BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_WEEKEND)
		{
			d = m_DayTypeDay == BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_L
					? CBCGPCalendar::GetMaxMonthDay (m, y)
					: 1;
			COleDateTime dt (y, m, d, 0, 0, 0);

			int nWDC = dt.GetDayOfWeek () - 1;
			if (nWDC == 0)
			{
				nWDC = 7;
			}

			if (m_DayTypeDay == BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_L)
			{
				if (nWDC < 6)
				{
					d -= nWDC;
				}
			}
			else
			{
				if (nWDC < 6)
				{
					d += 6 - nWDC;
					nWDC = 6;
				}

				if (m_DayTypeDay > BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_1)
				{
					if (nWDC == 6)
					{
						if (m_DayTypeDay >= BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_3)
						{
							d += m_DayTypeDay == BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_4 ? 8 : 7;
						}
						else
						{
							d++;
						}
					}
					else
					{
						d += 6;

						if (m_DayTypeDay >= BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_3)
						{
							d += m_DayTypeDay == BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_4 ? 7 : 1;
						}
					}
				}
			}
		}
		else
		{
			d = m_DayTypeDay == BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_L
					? CBCGPCalendar::GetMaxMonthDay (m, y)
					: 1;
			COleDateTime dt (y, m, d, 0, 0, 0);

			int nWD = 0;

			switch(m_DayTypeWeekDay)
			{
			case BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_1:
				nWD = 1;
				break;
			case BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_2:
				nWD = 2;
				break;
			case BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_3:
				nWD = 3;
				break;
			case BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_4:
				nWD = 4;
				break;
			case BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_5:
				nWD = 5;
				break;
			case BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_6:
				nWD = 6;
				break;
			case BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_7:
				nWD = 7;
				break;
			};

			int nWDC = dt.GetDayOfWeek () - 1;
			if (nWDC == 0)
			{
				nWDC = 7;
			}

			if (m_DayTypeDay == BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_L)
			{
				if (nWD > nWDC)
				{
					d -= 7;
				}
			}
			else
			{
				if (nWD < nWDC)
				{
					d += 7;
				}

				d += 7 * (int)m_DayTypeDay;
			}

			d += nWD - nWDC;
		}
	}

	return d;
}

void CBCGPRecurrenceRuleYearly::CorrectStart ()
{
	COleDateTime dt (GetDateStart ());

	int y = dt.GetYear ();
	int m = m_Type == BCGP_REC_RULE_YEARLY_TYPE_DAY ? m_dwDayMonth : m_dwDayTypeMonth;
	int d = GetPossibleDay (y);

	int nDays = CBCGPCalendar::GetMaxMonthDay (m, y);

	dt = COleDateTime (y, m, nDays < d ? nDays : d, 0, 0, 0);

	if (dt < GetDateStart ())
	{
		y++;
		
		d = GetPossibleDay (y);
		nDays = CBCGPCalendar::GetMaxMonthDay (m, y);

		dt = COleDateTime (y, m, nDays < d ? nDays : d, 0, 0, 0);
	}

	if (dt != GetDateStart ())
	{
		SetDateStart (dt);
	}
}

COleDateTime CBCGPRecurrenceRuleYearly::GetSiblingEventDay (const COleDateTime& dtSibling) const
{
	return CBCGPRecurrenceBaseRule::GetSiblingEventDay (dtSibling);
}

COleDateTime CBCGPRecurrenceRuleYearly::GetNextEventDay (const COleDateTime& dtPrev) const
{
	if (dtPrev < GetDateStart ())
	{
		return COleDateTime ();
	}

	COleDateTime dt (dtPrev);

	{
		int y = dt.GetYear () + 1;
		int m = m_Type == BCGP_REC_RULE_YEARLY_TYPE_DAY ? m_dwDayMonth : m_dwDayTypeMonth;
		int d = GetPossibleDay (y);

		int nDays = CBCGPCalendar::GetMaxMonthDay (m, y);

		dt = COleDateTime (y, m, nDays < d ? nDays : d, 0, 0, 0);
	}

	BCGP_RECURRENCE_RULE_LIMIT limitType = GetLimitType ();

	if (limitType == BCGP_RECURRENCE_RULE_LIMIT_COUNT)
	{
		if (m_Type == BCGP_REC_RULE_YEARLY_TYPE_DAY)
		{
			int y = dt.GetYear () - GetDateStart ().GetYear () + 1;
			
			if ((int)GetLimitCount () < y)
			{
				dt = COleDateTime ();
			}
		}
	}
	else if (limitType == BCGP_RECURRENCE_RULE_LIMIT_DATE)
	{
		if (GetLimitDate () < dt)
		{
			dt = COleDateTime ();
		}
	}

	return dt;
}

void CBCGPRecurrenceRuleYearly::SetType (BCGP_REC_RULE_YEARLY_TYPE type)
{
	ASSERT (BCGP_REC_RULE_YEARLY_TYPE_FIRST <= type && 
		type <= BCGP_REC_RULE_YEARLY_TYPE_LAST);

	if (m_Type != type)
	{
		m_Type = type;
	}
}

void CBCGPRecurrenceRuleYearly::SetDayMonth (DWORD dwDayMonth)
{
	ASSERT (1 <= dwDayMonth && dwDayMonth <= 12);

	if (m_dwDayMonth != dwDayMonth)
	{
		m_dwDayMonth = dwDayMonth;
	}
}

void CBCGPRecurrenceRuleYearly::SetDay (DWORD dwDay)
{
	ASSERT (1 <= dwDay && dwDay <= 31);

	if (m_dwDay != dwDay)
	{
		m_dwDay = dwDay;
	}
}

void CBCGPRecurrenceRuleYearly::SetDayTypeDay (BCGP_REC_RULE_YEARLY_DAYTYPE_DAY DayTypeDay)
{
	ASSERT (BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_FIRST <= DayTypeDay && 
		DayTypeDay <= BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_LAST);

	if (m_DayTypeDay != DayTypeDay)
	{
		m_DayTypeDay = DayTypeDay;
	}
}

void CBCGPRecurrenceRuleYearly::SetDayTypeWeekDay (BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY DayTypeWeekDay)
{
	ASSERT (BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_FIRST <= DayTypeWeekDay && 
		DayTypeWeekDay <= BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_LAST);

	if (m_DayTypeWeekDay != DayTypeWeekDay)
	{
		m_DayTypeWeekDay = DayTypeWeekDay;
	}
}

void CBCGPRecurrenceRuleYearly::SetDayTypeMonth (DWORD dwDayTypeMonth)
{
	ASSERT (1 <= dwDayTypeMonth && dwDayTypeMonth <= 12);

	if (m_dwDayTypeMonth != dwDayTypeMonth)
	{
		m_dwDayTypeMonth = dwDayTypeMonth;
	}
}

void CBCGPRecurrenceRuleYearly::Serialize (CArchive& ar)
{
	CBCGPRecurrenceBaseRule::Serialize (ar);

	if (ar.IsStoring ())
	{
		ar << (DWORD)m_Type;
		ar << m_dwDayMonth;
		ar << m_dwDay;

		ar << (DWORD)m_DayTypeDay;
		ar << (DWORD)m_DayTypeWeekDay;
		ar << m_dwDayTypeMonth;
	}
	else
	{
		DWORD type;
		ar >> type;
		m_Type = (BCGP_REC_RULE_YEARLY_TYPE)type;

		ar >> m_dwDayMonth;
		ar >> m_dwDay;

		ar >> type;
		m_DayTypeDay = (BCGP_REC_RULE_YEARLY_DAYTYPE_DAY)type;
		ar >> type;
		m_DayTypeWeekDay = (BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY)type;
		ar >> m_dwDayTypeMonth;

		// for scheme #1
		if (GetID() == 0)
		{
			SetID(BCGP_PLANNER_RULE_YEARLY);
		}
	}
}

void CBCGPRecurrenceRuleYearly::GetProperties (CBCGPAppointmentPropertyList& props) const
{
	CBCGPRecurrenceBaseRule::GetProperties (props);

	props.Add (BCGP_REC_RULE_PROPERTY_YEARLY_TYPE, new CBCGPAppointmentProperty ((DWORD)m_Type));
	props.Add (BCGP_REC_RULE_PROPERTY_YEARLY_DAY_MONTH, new CBCGPAppointmentProperty (m_dwDayMonth));
	props.Add (BCGP_REC_RULE_PROPERTY_YEARLY_DAY, new CBCGPAppointmentProperty (m_dwDay));
	props.Add (BCGP_REC_RULE_PROPERTY_YEARLY_DAYTYPE_DAY, new CBCGPAppointmentProperty ((DWORD)m_DayTypeDay));
	props.Add (BCGP_REC_RULE_PROPERTY_YEARLY_DAYTYPE_WEEKDAY, new CBCGPAppointmentProperty ((DWORD)m_DayTypeWeekDay));
	props.Add (BCGP_REC_RULE_PROPERTY_YEARLY_DAYTYPE_MONTH, new CBCGPAppointmentProperty (m_dwDayTypeMonth));
}

void CBCGPRecurrenceRuleYearly::SetProperties (const CBCGPAppointmentPropertyList& props)
{
	CBCGPRecurrenceBaseRule::SetProperties (props);

	if (props.GetCount () > 0)
	{
		CBCGPAppointmentProperty* prop = NULL;

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_YEARLY_TYPE))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_YEARLY_TYPE);
			m_Type = (BCGP_REC_RULE_YEARLY_TYPE)((DWORD)*prop);
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_YEARLY_DAY_MONTH))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_YEARLY_DAY_MONTH);
			m_dwDayMonth = *prop;
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_YEARLY_DAY))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_YEARLY_DAY);
			m_dwDay = *prop;
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_YEARLY_DAYTYPE_DAY))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_YEARLY_DAYTYPE_DAY);
			m_DayTypeDay = (BCGP_REC_RULE_YEARLY_DAYTYPE_DAY)((DWORD)*prop);
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_YEARLY_DAYTYPE_WEEKDAY))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_YEARLY_DAYTYPE_WEEKDAY);
			m_DayTypeWeekDay = (BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY)((DWORD)*prop);
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_YEARLY_DAYTYPE_MONTH))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_YEARLY_DAYTYPE_MONTH);
			m_dwDayTypeMonth = *prop;
		}
	}
}

#endif // BCGP_EXCLUDE_PLANNER
