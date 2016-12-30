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
// BCGPRecurrenceRule.h: interface for the CBCGPRecurrenceRule class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRECURRENCERULE_H__BF708D77_D94C_415C_AC2C_6E5B0E339ABE__INCLUDED_)
#define AFX_BCGPRECURRENCERULE_H__BF708D77_D94C_415C_AC2C_6E5B0E339ABE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPAppointmentProperty.h"

#define BCGP_REC_RULE_PROPERTY_FIRST          100
#define BCGP_REC_RULE_PROPERTY_DATE_START     BCGP_REC_RULE_PROPERTY_FIRST
#define BCGP_REC_RULE_PROPERTY_TIME_START     BCGP_REC_RULE_PROPERTY_FIRST + 1
#define BCGP_REC_RULE_PROPERTY_TIME_FINISH    BCGP_REC_RULE_PROPERTY_FIRST + 2
#define BCGP_REC_RULE_PROPERTY_LIMIT_TYPE     BCGP_REC_RULE_PROPERTY_FIRST + 3
#define BCGP_REC_RULE_PROPERTY_LIMIT_COUNT    BCGP_REC_RULE_PROPERTY_FIRST + 4
#define BCGP_REC_RULE_PROPERTY_LIMIT_DATE     BCGP_REC_RULE_PROPERTY_FIRST + 5
#define BCGP_REC_RULE_PROPERTY_LAST           BCGP_REC_RULE_PROPERTY_FIRST + 20

class BCGCBPRODLLEXPORT CBCGPRecurrenceBaseRule : public CObject
{
	DECLARE_SERIAL(CBCGPRecurrenceBaseRule)

	friend class CBCGPRecurrenceRuleRegistrator;
public:
	enum BCGP_RECURRENCE_RULE_LIMIT
	{
		BCGP_RECURRENCE_RULE_LIMIT_FIRST = 0,
		BCGP_RECURRENCE_RULE_LIMIT_NONE  = BCGP_RECURRENCE_RULE_LIMIT_FIRST,
		BCGP_RECURRENCE_RULE_LIMIT_COUNT = 1,
		BCGP_RECURRENCE_RULE_LIMIT_DATE  = 2,
		BCGP_RECURRENCE_RULE_LIMIT_LAST  = BCGP_RECURRENCE_RULE_LIMIT_DATE
	};

	virtual ~CBCGPRecurrenceBaseRule();

	void SetDateStart (const COleDateTime& dtStart);
	const COleDateTime& GetDateStart () const
	{
		return m_dtStart;
	}

	void SetTimeInterval (const COleDateTimeSpan& tmStart, const COleDateTimeSpan& tmFinish);
	const COleDateTimeSpan& GetTimeStart () const
	{
		return m_tmStart;
	}
	const COleDateTimeSpan& GetTimeFinish () const
	{
		return m_tmFinish;
	}

	void SetLimitType (BCGP_RECURRENCE_RULE_LIMIT type);
	BCGP_RECURRENCE_RULE_LIMIT GetLimitType () const
	{
		return m_LimitType;
	}

	void SetLimitDate (const COleDateTime& dtLimit);
	const COleDateTime& GetLimitDate () const
	{
		return m_LimitDate;
	}

	void SetLimitCount (DWORD dwCount);
	DWORD GetLimitCount () const
	{
		return m_LimitCount;
	}

	virtual void GetProperties (CBCGPAppointmentPropertyList& props) const;
	virtual void SetProperties (const CBCGPAppointmentPropertyList& props);

	virtual void CorrectStart ();
	virtual COleDateTime GetSiblingEventDay (const COleDateTime& dtSibling) const;
	virtual COleDateTime GetNextEventDay (const COleDateTime& /*dtPrev*/) const
	{
		ASSERT (FALSE);
		return COleDateTime ();
	}
	virtual COleDateTime GetPrevEventDay (const COleDateTime& /*dtPrev*/) const
	{
		ASSERT (FALSE);
		return COleDateTime ();
	}	

	virtual void Serialize (CArchive& ar);

	DWORD GetID() const
	{
		return m_ID;
	}
	
	CBCGPRecurrenceBaseRule* CreateCopy () const;

	virtual BOOL operator == (const CBCGPRecurrenceBaseRule& rRule) const;
	virtual BOOL operator != (const CBCGPRecurrenceBaseRule& rRule) const;

protected:
	CBCGPRecurrenceBaseRule();

	void SetID (DWORD ID);

private:
	COleDateTime		m_dtStart;

	COleDateTimeSpan	m_tmStart;
	COleDateTimeSpan	m_tmFinish;

	BCGP_RECURRENCE_RULE_LIMIT
						m_LimitType;

	COleDateTime		m_LimitDate;

	DWORD				m_LimitCount;
	DWORD				m_ID;
};

class BCGCBPRODLLEXPORT CBCGPRecurrenceRuleRegistrator : public CObject
{
	DECLARE_DYNCREATE(CBCGPRecurrenceRuleRegistrator)
public:
	typedef CArray<DWORD, DWORD>
		XBCGPRecurrenceRuleIDArray;

	CBCGPRecurrenceRuleRegistrator ();
	virtual ~CBCGPRecurrenceRuleRegistrator ();

	// return ID;
	DWORD RegisterRule (CRuntimeClass* pRuleClass);

	CBCGPRecurrenceBaseRule* CreateRule (DWORD ID) const;

	int	GetRulesCount () const
	{
		return (int) m_Rules.GetSize ();
	}

	void GetRulesID (XBCGPRecurrenceRuleIDArray& arID) const;

protected:
	void UnregisterRule (DWORD ID);
//	virtual void Serialize (CArchive& ar);

private:
	typedef CArray<CRuntimeClass*, CRuntimeClass*>
		XBCGPRecurrenceRuleArray;

	XBCGPRecurrenceRuleArray m_Rules;
};

#endif // BCGP_EXCLUDE_PLANNER

#endif // !defined(AFX_BCGPRECURRENCERULE_H__BF708D77_D94C_415C_AC2C_6E5B0E339ABE__INCLUDED_)
