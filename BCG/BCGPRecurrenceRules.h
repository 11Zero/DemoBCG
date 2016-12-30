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
// BCGPRecurrenceRules.h: interface for the CBCGPRecurrenceRule class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRECURRENCERULES_H__BF708D77_D94C_415C_AC2C_6E5B0E339ABE__INCLUDED_)
#define AFX_BCGPRECURRENCERULES_H__BF708D77_D94C_415C_AC2C_6E5B0E339ABE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPRecurrenceRule.h"

#define BCGP_REC_RULE_PROPERTY_DAILY_TYPE		BCGP_REC_RULE_PROPERTY_FIRST + 6
#define BCGP_REC_RULE_PROPERTY_DAILY_SPAN		BCGP_REC_RULE_PROPERTY_FIRST + 7

#define BCGP_REC_RULE_PROPERTY_WEEKLY_SPAN		BCGP_REC_RULE_PROPERTY_FIRST + 6
#define BCGP_REC_RULE_PROPERTY_WEEKLY_DAYS		BCGP_REC_RULE_PROPERTY_FIRST + 7

#define BCGP_REC_RULE_PROPERTY_MONTHLY_TYPE		BCGP_REC_RULE_PROPERTY_FIRST + 6
#define BCGP_REC_RULE_PROPERTY_MONTHLY_DAY		BCGP_REC_RULE_PROPERTY_FIRST + 7
#define BCGP_REC_RULE_PROPERTY_MONTHLY_DAY_SPAN	BCGP_REC_RULE_PROPERTY_FIRST + 8
#define BCGP_REC_RULE_PROPERTY_MONTHLY_DAYTYPE_DAY	BCGP_REC_RULE_PROPERTY_FIRST + 9
#define BCGP_REC_RULE_PROPERTY_MONTHLY_DAYTYPE_WEEKDAY	BCGP_REC_RULE_PROPERTY_FIRST + 10
#define BCGP_REC_RULE_PROPERTY_MONTHLY_DAYTYPE_SPAN	BCGP_REC_RULE_PROPERTY_FIRST + 11

#define BCGP_REC_RULE_PROPERTY_YEARLY_TYPE		BCGP_REC_RULE_PROPERTY_FIRST + 6
#define BCGP_REC_RULE_PROPERTY_YEARLY_DAY_MONTH	BCGP_REC_RULE_PROPERTY_FIRST + 7
#define BCGP_REC_RULE_PROPERTY_YEARLY_DAY		BCGP_REC_RULE_PROPERTY_FIRST + 8
#define BCGP_REC_RULE_PROPERTY_YEARLY_DAYTYPE_DAY	BCGP_REC_RULE_PROPERTY_FIRST + 9
#define BCGP_REC_RULE_PROPERTY_YEARLY_DAYTYPE_WEEKDAY	BCGP_REC_RULE_PROPERTY_FIRST + 10
#define BCGP_REC_RULE_PROPERTY_YEARLY_DAYTYPE_MONTH	BCGP_REC_RULE_PROPERTY_FIRST + 11

class BCGCBPRODLLEXPORT CBCGPRecurrenceRuleDaily : public CBCGPRecurrenceBaseRule
{
	DECLARE_SERIAL(CBCGPRecurrenceRuleDaily)
public:
	enum BCGP_REC_RULE_DAILY_TYPE
	{
		BCGP_REC_RULE_DAILY_TYPE_FIRST    = 0,
		BCGP_REC_RULE_DAILY_TYPE_SPAN     = BCGP_REC_RULE_DAILY_TYPE_FIRST,
		BCGP_REC_RULE_DAILY_TYPE_WEEKDAYS = 1,
		BCGP_REC_RULE_DAILY_TYPE_LAST     = BCGP_REC_RULE_DAILY_TYPE_WEEKDAYS
	};

	virtual ~CBCGPRecurrenceRuleDaily ();

	virtual COleDateTime GetSiblingEventDay (const COleDateTime& dtSibling) const;
	virtual COleDateTime GetNextEventDay (const COleDateTime& dtPrev) const;

	virtual void Serialize (CArchive& ar);

	virtual void CorrectStart ();

	void SetSpan (DWORD dwSpan);
	DWORD GetSpan () const
	{
		return m_dwSpan;
	}

	void SetType (BCGP_REC_RULE_DAILY_TYPE type);
	BCGP_REC_RULE_DAILY_TYPE GetType () const
	{
		return m_Type;
	}

	virtual void GetProperties (CBCGPAppointmentPropertyList& props) const;
	virtual void SetProperties (const CBCGPAppointmentPropertyList& props);

protected:
	CBCGPRecurrenceRuleDaily ();

private:
	BCGP_REC_RULE_DAILY_TYPE	m_Type;
	DWORD						m_dwSpan;
};


class BCGCBPRODLLEXPORT CBCGPRecurrenceRuleWeekly : public CBCGPRecurrenceBaseRule
{
	DECLARE_SERIAL(CBCGPRecurrenceRuleWeekly)
public:
	enum BCGP_REC_RULE_WEEKLY
	{
		BCGP_REC_RULE_WEEKLY_FIRST = 0x01,
		BCGP_REC_RULE_WEEKLY_1     = BCGP_REC_RULE_WEEKLY_FIRST,
		BCGP_REC_RULE_WEEKLY_2     = 0x02,
		BCGP_REC_RULE_WEEKLY_3     = 0x04,
		BCGP_REC_RULE_WEEKLY_4     = 0x08,
		BCGP_REC_RULE_WEEKLY_5     = 0x10,
		BCGP_REC_RULE_WEEKLY_6     = 0x20,
		BCGP_REC_RULE_WEEKLY_7     = 0x40,
		BCGP_REC_RULE_WEEKLY_LAST  = BCGP_REC_RULE_WEEKLY_7
	};

	virtual ~CBCGPRecurrenceRuleWeekly ();

	virtual COleDateTime GetSiblingEventDay (const COleDateTime& dtSibling) const;
	virtual COleDateTime GetNextEventDay (const COleDateTime& dtPrev) const;

	virtual void Serialize (CArchive& ar);

	virtual void CorrectStart ();

	void SetSpan (DWORD dwSpan);
	DWORD GetSpan () const
	{
		return m_dwSpan;
	}

	void SetDays (DWORD dwDays);
	DWORD GetDays () const
	{
		return m_dwDays;
	}

	virtual void GetProperties (CBCGPAppointmentPropertyList& props) const;
	virtual void SetProperties (const CBCGPAppointmentPropertyList& props);

protected:
	CBCGPRecurrenceRuleWeekly ();

	int TypeToDayOfWeek (BCGP_REC_RULE_WEEKLY type) const;
	BCGP_REC_RULE_WEEKLY DayOfWeekToType (int day) const;
	
	BCGP_REC_RULE_WEEKLY GetFirstDay () const;
	BCGP_REC_RULE_WEEKLY GetNextDay (BCGP_REC_RULE_WEEKLY type) const;
	BCGP_REC_RULE_WEEKLY GetLastDay () const;
	BCGP_REC_RULE_WEEKLY GetStartDay () const;

	int GetDayCount () const;

private:
	DWORD					m_dwSpan;
	DWORD					m_dwDays;

	BCGP_REC_RULE_WEEKLY	m_FirstDay;
	BCGP_REC_RULE_WEEKLY	m_LastDay;
	
	int						m_nCount;
};

class BCGCBPRODLLEXPORT CBCGPRecurrenceRuleMonthly : public CBCGPRecurrenceBaseRule
{
	DECLARE_SERIAL(CBCGPRecurrenceRuleMonthly)
public:
	enum BCGP_REC_RULE_MONTHLY_TYPE
	{
		BCGP_REC_RULE_MONTHLY_TYPE_FIRST   = 0,
		BCGP_REC_RULE_MONTHLY_TYPE_DAY     = BCGP_REC_RULE_MONTHLY_TYPE_FIRST,
		BCGP_REC_RULE_MONTHLY_TYPE_DAYTYPE = 1,
		BCGP_REC_RULE_MONTHLY_TYPE_LAST    = BCGP_REC_RULE_MONTHLY_TYPE_DAYTYPE
	};

	enum BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY
	{
		BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_FIRST = 0,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_1     = BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_FIRST,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_2     = 1,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_3     = 2,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_4     = 3,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_L     = 4,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_LAST  = BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY_L
	};

	enum BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY
	{
		BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_FIRST   = 0,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_DAY     = BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_FIRST,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_WEEKDAY = 1,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_WEEKEND = 2,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_1       = 3,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_2       = 4,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_3       = 5,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_4       = 6,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_5       = 7,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_6       = 8,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_7       = 9,
		BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_LAST    = BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY_7
	};

	virtual ~CBCGPRecurrenceRuleMonthly ();

	virtual COleDateTime GetSiblingEventDay (const COleDateTime& dtSibling) const;
	virtual COleDateTime GetNextEventDay (const COleDateTime& dtPrev) const;

	virtual void Serialize (CArchive& ar);

	virtual void CorrectStart ();

	void SetType (BCGP_REC_RULE_MONTHLY_TYPE type);
	BCGP_REC_RULE_MONTHLY_TYPE GetType () const
	{
		return m_Type;
	}

	void SetDay (DWORD dwDay);
	DWORD GetDay () const
	{
		return m_dwDay;
	}

	void SetDaySpan (DWORD dwDaySpan);
	DWORD GetDaySpan () const
	{
		return m_dwDaySpan;
	}

	void SetDayTypeDay (BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY DayTypeDay);
	BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY GetDayTypeDay () const
	{
		return m_DayTypeDay;
	}

	void SetDayTypeWeekDay (BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY DayTypeWeekDay);
	BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY GetDayTypeWeekDay () const
	{
		return m_DayTypeWeekDay;
	}

	void SetDayTypeSpan (DWORD dwDayTypeSpan);
	DWORD GetDayTypeSpan () const
	{
		return m_dwDaySpan;
	}

	virtual void GetProperties (CBCGPAppointmentPropertyList& props) const;
	virtual void SetProperties (const CBCGPAppointmentPropertyList& props);

protected:
	CBCGPRecurrenceRuleMonthly ();

	int GetPossibleDay (int m, int y) const;

private:
	BCGP_REC_RULE_MONTHLY_TYPE	m_Type;
	DWORD						m_dwDay;
	DWORD						m_dwDaySpan;

	BCGP_REC_RULE_MONTHLY_DAYTYPE_DAY
								m_DayTypeDay;
	BCGP_REC_RULE_MONTHLY_DAYTYPE_WEEKDAY
								m_DayTypeWeekDay;
	DWORD						m_dwDayTypeSpan;
};


class BCGCBPRODLLEXPORT CBCGPRecurrenceRuleYearly : public CBCGPRecurrenceBaseRule
{
	DECLARE_SERIAL(CBCGPRecurrenceRuleYearly)
public:
	enum BCGP_REC_RULE_YEARLY_TYPE
	{
		BCGP_REC_RULE_YEARLY_TYPE_FIRST   = 0,
		BCGP_REC_RULE_YEARLY_TYPE_DAY     = BCGP_REC_RULE_YEARLY_TYPE_FIRST,
		BCGP_REC_RULE_YEARLY_TYPE_DAYTYPE = 1,
		BCGP_REC_RULE_YEARLY_TYPE_LAST    = BCGP_REC_RULE_YEARLY_TYPE_DAYTYPE
	};

	enum BCGP_REC_RULE_YEARLY_DAYTYPE_DAY
	{
		BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_FIRST = 0,
		BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_1     = BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_FIRST,
		BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_2     = 1,
		BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_3     = 2,
		BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_4     = 3,
		BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_L     = 4,
		BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_LAST  = BCGP_REC_RULE_YEARLY_DAYTYPE_DAY_L
	};

	enum BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY
	{
		BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_FIRST   = 0,
		BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_DAY     = BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_FIRST,
		BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_WEEKDAY = 1,
		BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_WEEKEND = 2,
		BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_1       = 3,
		BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_2       = 4,
		BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_3       = 5,
		BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_4       = 6,
		BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_5       = 7,
		BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_6       = 8,
		BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_7       = 9,
		BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_LAST    = BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY_7
	};

	virtual ~CBCGPRecurrenceRuleYearly ();

	virtual COleDateTime GetSiblingEventDay (const COleDateTime& dtSibling) const;
	virtual COleDateTime GetNextEventDay (const COleDateTime& dtPrev) const;

	virtual void Serialize (CArchive& ar);

	virtual void CorrectStart ();

	void SetType (BCGP_REC_RULE_YEARLY_TYPE type);
	BCGP_REC_RULE_YEARLY_TYPE GetType () const
	{
		return m_Type;
	}

	void SetDayMonth (DWORD dwDayMonth);
	DWORD GetDayMonth () const
	{
		return m_dwDayMonth;
	}

	void SetDay (DWORD dwDay);
	DWORD GetDay () const
	{
		return m_dwDay;
	}

	void SetDayTypeDay (BCGP_REC_RULE_YEARLY_DAYTYPE_DAY DayTypeDay);
	BCGP_REC_RULE_YEARLY_DAYTYPE_DAY GetDayTypeDay () const
	{
		return m_DayTypeDay;
	}

	void SetDayTypeWeekDay (BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY DayTypeWeekDay);
	BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY GetDayTypeWeekDay () const
	{
		return m_DayTypeWeekDay;
	}

	void SetDayTypeMonth (DWORD dwDayTypeMonth);
	DWORD GetDayTypeMonth () const
	{
		return m_dwDayTypeMonth;
	}

	virtual void GetProperties (CBCGPAppointmentPropertyList& props) const;
	virtual void SetProperties (const CBCGPAppointmentPropertyList& props);

protected:
	CBCGPRecurrenceRuleYearly ();

	int GetPossibleDay (int y) const;

private:
	BCGP_REC_RULE_YEARLY_TYPE	m_Type;
	DWORD						m_dwDayMonth;
	DWORD						m_dwDay;

	BCGP_REC_RULE_YEARLY_DAYTYPE_DAY
								m_DayTypeDay;
	BCGP_REC_RULE_YEARLY_DAYTYPE_WEEKDAY
								m_DayTypeWeekDay;
	DWORD						m_dwDayTypeMonth;
};

#endif // BCGP_EXCLUDE_PLANNER

#endif // !defined(AFX_BCGPRECURRENCERULES_H__BF708D77_D94C_415C_AC2C_6E5B0E339ABE__INCLUDED_)
