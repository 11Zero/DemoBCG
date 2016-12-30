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
// BCGPRecurrenceRule.cpp: implementation of the CBCGPRecurrenceRule class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPRecurrenceRule.h"

#ifndef BCGP_EXCLUDE_PLANNER

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CBCGPRecurrenceBaseRule, CObject, VERSIONABLE_SCHEMA | 2)

CBCGPRecurrenceBaseRule::CBCGPRecurrenceBaseRule()
	: m_LimitType  (BCGP_RECURRENCE_RULE_LIMIT_FIRST)
	, m_LimitCount (10)
	, m_ID         (0)
{
}

CBCGPRecurrenceBaseRule::~CBCGPRecurrenceBaseRule()
{
}

void CBCGPRecurrenceBaseRule::Serialize (CArchive& ar)
{
	ASSERT_VALID (this);

	CObject::Serialize (ar);

	if (ar.IsStoring ())
	{
		ar << m_dtStart;
		ar << m_tmStart;
		ar << m_tmFinish;
		ar << (DWORD)m_LimitType;
		ar << m_LimitDate;
		ar << m_LimitCount;
		ar << m_ID;
	}
	else
	{
		UINT nSchema = ar.GetObjectSchema();

		ar >> m_dtStart;
		ar >> m_tmStart;
		ar >> m_tmFinish;

		DWORD dwType;
		ar >> dwType;
		m_LimitType = (BCGP_RECURRENCE_RULE_LIMIT)(dwType);

		ar >> m_LimitDate;
		ar >> m_LimitCount;

		if (nSchema > 1)
		{
			ar >> m_ID;
		}
	}
}

void CBCGPRecurrenceBaseRule::SetDateStart (const COleDateTime& dtStart)
{
	ASSERT (dtStart.GetStatus () == COleDateTime::valid);

	m_dtStart = COleDateTime(dtStart.GetYear (), dtStart.GetMonth (), dtStart.GetDay (), 
		0, 0, 0);
}

void CBCGPRecurrenceBaseRule::SetTimeInterval (const COleDateTimeSpan& tmStart, 
											   const COleDateTimeSpan& tmFinish)
{
	ASSERT(tmStart.GetStatus () == COleDateTimeSpan::valid);
	ASSERT(tmFinish.GetStatus () == COleDateTimeSpan::valid);

	m_tmStart  = COleDateTimeSpan (0, tmStart.GetHours (), tmStart.GetMinutes (), tmStart.GetSeconds ());
	m_tmFinish = tmFinish;
}

void CBCGPRecurrenceBaseRule::SetLimitType (BCGP_RECURRENCE_RULE_LIMIT type)
{
	ASSERT(BCGP_RECURRENCE_RULE_LIMIT_FIRST <= type && 
		   type <= BCGP_RECURRENCE_RULE_LIMIT_LAST);

	m_LimitType = type;
}

void CBCGPRecurrenceBaseRule::SetLimitDate (const COleDateTime& dtLimit)
{
	ASSERT(dtLimit.GetStatus () == COleDateTime::valid);

	m_LimitDate = COleDateTime(dtLimit.GetYear (), dtLimit.GetMonth (), dtLimit.GetDay (), 
		0, 0, 0);
}

void CBCGPRecurrenceBaseRule::SetLimitCount (DWORD nCount)
{
	m_LimitCount = nCount;
}

void CBCGPRecurrenceBaseRule::CorrectStart ()
{
}

// ordinary enumeration of possibilities
COleDateTime CBCGPRecurrenceBaseRule::GetSiblingEventDay (const COleDateTime& dtSibling) const
{
	COleDateTime dt (GetDateStart ());

	if (dt > dtSibling)
	{
		return COleDateTime ();
	}
	else if (dt < dtSibling)
	{
		while (dt != COleDateTime () && dt < dtSibling)
		{
			dt = GetNextEventDay (dt);
		}
	}

	return dt;
}

void CBCGPRecurrenceBaseRule::SetID (DWORD ID)
{
	m_ID = ID;
}

void CBCGPRecurrenceBaseRule::GetProperties (CBCGPAppointmentPropertyList& props) const
{
	props.RemoveAll ();

	props.Add (BCGP_REC_RULE_PROPERTY_DATE_START, new CBCGPAppointmentProperty (m_dtStart));
	props.Add (BCGP_REC_RULE_PROPERTY_TIME_START, new CBCGPAppointmentProperty (m_tmStart));
	props.Add (BCGP_REC_RULE_PROPERTY_TIME_FINISH, new CBCGPAppointmentProperty (m_tmFinish));
	props.Add (BCGP_REC_RULE_PROPERTY_LIMIT_TYPE, new CBCGPAppointmentProperty ((DWORD)m_LimitType));
	props.Add (BCGP_REC_RULE_PROPERTY_LIMIT_COUNT, new CBCGPAppointmentProperty (m_LimitCount));
	props.Add (BCGP_REC_RULE_PROPERTY_LIMIT_DATE, new CBCGPAppointmentProperty (m_LimitDate));
}

void CBCGPRecurrenceBaseRule::SetProperties (const CBCGPAppointmentPropertyList& props)
{
	if (props.GetCount () > 0)
	{
		CBCGPAppointmentProperty* prop = NULL;

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_DATE_START))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_DATE_START);
			m_dtStart = *prop;
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_TIME_START))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_TIME_START);
			m_tmStart = *prop;
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_TIME_FINISH))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_TIME_FINISH);
			m_tmFinish = *prop;
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_LIMIT_TYPE))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_LIMIT_TYPE);
			m_LimitType = (BCGP_RECURRENCE_RULE_LIMIT)((DWORD)(*prop));
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_LIMIT_COUNT))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_LIMIT_COUNT);
			m_LimitCount = *prop;
		}

		if (props.PropertyExists (BCGP_REC_RULE_PROPERTY_LIMIT_DATE))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_REC_RULE_PROPERTY_LIMIT_DATE);
			m_LimitDate = *prop;
		}
	}
}

CBCGPRecurrenceBaseRule* CBCGPRecurrenceBaseRule::CreateCopy () const
{
	CBCGPRecurrenceBaseRule* pRule = 
		(CBCGPRecurrenceBaseRule*)GetRuntimeClass ()->CreateObject ();

	ASSERT_VALID (pRule);

	CBCGPAppointmentPropertyList props;
	GetProperties (props);
	pRule->SetProperties (props);

	pRule->SetID (m_ID);

	return pRule;
}

BOOL CBCGPRecurrenceBaseRule::operator == (const CBCGPRecurrenceBaseRule& rRule) const
{
	if (m_ID != rRule.GetID ())
	{
		return FALSE;
	}

	CBCGPAppointmentPropertyList props;
	GetProperties (props);

	CBCGPAppointmentPropertyList propsRule;
	rRule.GetProperties (propsRule);

	return props == propsRule;
}

BOOL CBCGPRecurrenceBaseRule::operator != (const CBCGPRecurrenceBaseRule& rRule) const
{
	return !(*this == rRule);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CBCGPRecurrenceRuleRegistrator, CObject)

CBCGPRecurrenceRuleRegistrator::CBCGPRecurrenceRuleRegistrator ()
{
}

CBCGPRecurrenceRuleRegistrator::~CBCGPRecurrenceRuleRegistrator ()
{
}

DWORD CBCGPRecurrenceRuleRegistrator::RegisterRule (CRuntimeClass* pRuleClass)
{
	if (pRuleClass == NULL)
	{
		ASSERT (FALSE);
		return 0;
	}

	if (!pRuleClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPRecurrenceBaseRule)))
	{
		ASSERT (FALSE);
		return 0;
	}

	if (m_Rules.Add (pRuleClass))
	{
		return GetRulesCount ();
	}

	return 0;
}

void CBCGPRecurrenceRuleRegistrator::UnregisterRule (DWORD ID)
{
	ASSERT (0 < ID && ID <= (DWORD)GetRulesCount ());

	if (ID <= (DWORD)GetRulesCount ())
	{
		m_Rules.RemoveAt (ID - 1);
	}
}

CBCGPRecurrenceBaseRule* CBCGPRecurrenceRuleRegistrator::CreateRule (DWORD ID) const
{
	ASSERT (0 < ID && ID <= (DWORD)GetRulesCount ());

	CBCGPRecurrenceBaseRule* pRule = NULL;

	if (ID <= (DWORD)GetRulesCount ())
	{
		pRule = (CBCGPRecurrenceBaseRule*)(m_Rules[ID - 1]->CreateObject ());
		pRule->SetID (ID);
	}

	return pRule;
}

void CBCGPRecurrenceRuleRegistrator::GetRulesID (XBCGPRecurrenceRuleIDArray& arID) const
{
	arID.RemoveAll ();

	for (int i = 0; i < GetRulesCount (); i++)
	{
		arID.Add ((DWORD)(i + 1));
	}
}

#endif // BCGP_EXCLUDE_PLANNER
