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
// BCGPRecurrence.cpp: implementation of the CBCGPRecurrence class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPRecurrence.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPAppointment.h"
#include "BCGPRecurrenceRule.h"
#include "BCGPPlannerView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CBCGPRecurrence, CObject, VERSIONABLE_SCHEMA | 1)

CBCGPRecurrence::CBCGPRecurrence()
	: m_pAppointment (NULL)
	, m_pRule        (NULL)
{

}

CBCGPRecurrence::~CBCGPRecurrence()
{
	if (m_pRule != NULL)
	{
		delete m_pRule;
		m_pRule = NULL;
	}

	RemoveExceptions ();
}

void CBCGPRecurrence::RemoveExceptions ()
{
	POSITION Pos = m_Exceptions.GetStartPosition ();
	COleDateTime Key;
	XBCGPRecurrenceException* Val = NULL;

	while (Pos != NULL)
	{
		m_Exceptions.GetNextAssoc (Pos, Key, Val);

		if (Val != NULL)
		{
			delete Val;
			Val = NULL;
		}
	}

	m_Exceptions.RemoveAll ();
}

BOOL CBCGPRecurrence::IsExceptionsCollectionEmpty () const
{
	return m_Exceptions.GetCount () == 0;
}

BOOL CBCGPRecurrence::ExceptionExists (const COleDateTime& dtDate) const
{
	XBCGPRecurrenceException* ecp = NULL;

	return m_Exceptions.Lookup (dtDate, ecp);
}

void CBCGPRecurrence::DoException (const COleDateTime& dtDate, 
								   const CBCGPAppointmentPropertyList& props,
								   BOOL bDeleted)
{
	ASSERT_VALID (m_pAppointment);

	CBCGPAppointmentPropertyList srcProps;
	m_pAppointment->GetProperties (srcProps);

	if (!bDeleted)
	{
		ASSERT (srcProps.GetCount () == props.GetCount ());
	}

	CBCGPRecurrenceBaseRule* pRule = GetRule ();
	ASSERT_VALID (pRule);

	COleDateTime dtStart;
	COleDateTime dtFinish;

	CBCGPAppointmentPropertyList propsEcp;

	XBCGPRecurrenceException* pEcp = GetException (dtDate);

	if (!bDeleted)
	{
		CBCGPAppointmentProperty* pProp = NULL;

		pProp = (CBCGPAppointmentProperty*)props.Get (BCGP_APPOINTMENT_PROPERTY_DATE_START);
		dtStart  = *pProp;

		pProp = (CBCGPAppointmentProperty*)props.Get (BCGP_APPOINTMENT_PROPERTY_DATE_FINISH);
		dtFinish = *pProp;

		COleDateTime dtStartNeed  (dtDate + pRule->GetTimeStart ());
		COleDateTime dtFinishNeed (dtDate + pRule->GetTimeFinish ());

		BOOL bTimeChanged = dtStart != dtStartNeed || dtFinish != dtFinishNeed;

		if (pEcp != NULL)
		{
			bTimeChanged |=
				pEcp->m_Properties.PropertyExists (BCGP_APPOINTMENT_PROPERTY_DATE_START) ||
				pEcp->m_Properties.PropertyExists (BCGP_APPOINTMENT_PROPERTY_DATE_FINISH);

			if (!bDeleted)
			{
				POSITION pos = pEcp->m_Properties.GetStart ();
				DWORD ID = 0;
				CBCGPAppointmentBaseProperty* pProp = NULL;

				while (pos != NULL)
				{
					pEcp->m_Properties.GetNext (pos, ID, pProp);

					if (ID == BCGP_APPOINTMENT_PROPERTY_DATE_START ||
						ID == BCGP_APPOINTMENT_PROPERTY_DATE_FINISH)
					{
						continue;
					}

					*(srcProps[ID]) = *pProp;
				}
			}
		}

		POSITION pos = srcProps.GetStart ();
		DWORD ID = 0;
		CBCGPAppointmentBaseProperty* pSrcProp = NULL;

		while (pos != NULL)
		{
			srcProps.GetNext (pos, ID, pSrcProp);

			if (!bTimeChanged && 
				(ID == BCGP_APPOINTMENT_PROPERTY_DATE_START ||
				 ID == BCGP_APPOINTMENT_PROPERTY_DATE_FINISH))
			{
				continue;
			}

			const CBCGPAppointmentBaseProperty* pDstProp = props.Get (ID);

			BOOL bEqual = TRUE;

			if (pDstProp == NULL)
			{
				continue;
			}
			else if (pSrcProp != NULL)
			{
				bEqual = *pSrcProp == *pDstProp;
			}

			if (!bEqual ||
				(ID == BCGP_APPOINTMENT_PROPERTY_DATE_START ||
				 ID == BCGP_APPOINTMENT_PROPERTY_DATE_FINISH))
			{
				CBCGPAppointmentBaseProperty* pNewProp = 
					(CBCGPAppointmentBaseProperty*)(pDstProp->GetRuntimeClass ()->CreateObject ());

				*pNewProp = *pDstProp;
					
				propsEcp[ID] = pNewProp;
			}
		}
	}

	if (bDeleted || propsEcp.GetCount () > 0)
	{
		if (pEcp == NULL)
		{
			pEcp = new XBCGPRecurrenceException;
		}

		if (!bDeleted)
		{
			pEcp->m_dtStart  = dtStart;
			pEcp->m_dtFinish = dtFinish;

			POSITION pos = propsEcp.GetStart ();

			DWORD ID = 0;
			CBCGPAppointmentBaseProperty* pProp = NULL;

			while (pos != NULL)
			{
				propsEcp.GetNext (pos, ID, pProp);
				ASSERT_VALID (pProp);

				if (!pEcp->m_Properties.PropertyExists (ID))
				{
					CBCGPAppointmentBaseProperty* pNewProp = 
						(CBCGPAppointmentBaseProperty*)(pProp->GetRuntimeClass ()->CreateObject ());

					*pNewProp = *pProp;

					pEcp->m_Properties[ID] = pNewProp;
				}
				else
				{
					*(pEcp->m_Properties[ID]) = *pProp;
				}
			}
		}
		else
		{
			pEcp->m_Deleted = bDeleted;
			pEcp->m_Properties.RemoveAll ();
		}

		m_Exceptions[dtDate] = pEcp;
	}
}

CBCGPRecurrence::XBCGPRecurrenceException*
CBCGPRecurrence::GetException (const COleDateTime& dtDate)
{
	XBCGPRecurrenceException* pEcp = NULL;

	if (m_Exceptions.Lookup (dtDate, pEcp))
	{
		return pEcp;
	}

	return NULL;
}

void CBCGPRecurrence::SetRule (const CBCGPRecurrenceBaseRule* pRule)
{
	ASSERT_VALID(pRule);

	if (pRule != NULL)
	{
		if (m_pRule != NULL)
		{
			delete m_pRule;
			m_pRule = NULL;
		}

		RemoveExceptions ();

		if (pRule != NULL)
		{
			ASSERT_VALID (pRule);
			
			m_pRule = pRule->CreateCopy ();
		}
	}
}

CBCGPAppointment* CBCGPRecurrence::CreateClone (const COleDateTime& dtDate) const
{
	CBCGPAppointment* pApp = NULL;
	
	ASSERT_VALID (m_pAppointment);
	
	if (m_pAppointment != NULL)
	{
		XBCGPRecurrenceException* ecp = NULL;
		if (m_Exceptions.Lookup (dtDate, ecp))
		{
			if (ecp->m_Deleted)
			{
				return NULL;
			}
		}

		pApp = CBCGPAppointment::CreateRecurrenceClone (m_pAppointment, dtDate);

		if (ecp != NULL)
		{
			pApp->SetProperties (ecp->m_Properties);
		}
	}

	return pApp;
}

void CBCGPRecurrence::Query (XBCGPAppointmentArray& ar, const COleDateTime& date1, const COleDateTime& date2) const
{
	ar.RemoveAll ();

	POSITION Pos = m_Exceptions.GetStartPosition ();
	COleDateTime Key;
	XBCGPRecurrenceException* Val = NULL;

	CList<COleDateTime, const COleDateTime&> exceptions;

	CBCGPAppointment* pApp = NULL;

	while (Pos != NULL)
	{
		m_Exceptions.GetNextAssoc (Pos, Key, Val);

		if (Val != NULL && !Val->m_Deleted)
		{
			if ((date1 <= Val->m_dtStart && Val->m_dtStart <= date2) || 
				(date1 <= Val->m_dtFinish && Val->m_dtFinish <= date2) ||
				(Val->m_dtStart < date1 && date2 < Val->m_dtFinish))
			{
				pApp = CreateClone (Key);

				if (pApp != NULL)
				{
					exceptions.AddTail (Key);
					ar.Add (pApp);
				}
			}
		}
	}

	CBCGPRecurrenceBaseRule* pRule = GetRule ();
	ASSERT_VALID(pRule);

	if (pRule == NULL)
	{
		return;
	}

	BOOL bNext = TRUE;
	COleDateTime dt1 (pRule->GetDateStart ());

	if (!CBCGPPlannerView::IsOneDay (m_pAppointment->GetStart (), 
			m_pAppointment->GetFinish ()))
	{
		if (dt1 < date1)
		{
			COleDateTimeSpan span ((double)((int)
				((double)m_pAppointment->GetDuration () + 0.5)));

			COleDateTime d (date1 - span);
			if (d > dt1)
			{
				dt1 = pRule->GetSiblingEventDay (d);
			}

			bNext = FALSE;
		}
	}

	if (bNext)
	{
		if (dt1 <= date2)
		{
			if (dt1 <= date1)
			{
				dt1 = pRule->GetSiblingEventDay (date1);
			}
		}
	}

	if (dt1 == COleDateTime ())
	{
		return;
	}

	while (dt1 <= date2)
	{
		if (exceptions.Find (dt1) == NULL 
			&& !ExceptionExists (dt1))
		{
			pApp = CreateClone (dt1);

			if (pApp != NULL)
			{
				if (pApp->GetFinish () != date1 ||
					(pApp->GetStart () == pApp->GetFinish () && pApp->GetFinish () == date1))
				{
					ar.Add (pApp);
				}
				else
				{
					delete pApp;
				}
			}
		}

		dt1 = pRule->GetNextEventDay (dt1);

		if (dt1 == COleDateTime ())
		{
			break;
		}
	}
}

void CBCGPRecurrence::SetAppointment (CBCGPAppointment* pApp)
{
	m_pAppointment = pApp;
}

void CBCGPRecurrence::Serialize (CArchive& ar)
{
	CObject::Serialize (ar);

	if (ar.IsStoring ())
	{
		ASSERT_VALID (m_pRule);

		ar << m_pRule;
		ar << (DWORD)m_Exceptions.GetCount ();

		POSITION Pos = m_Exceptions.GetStartPosition ();
		COleDateTime Key;
		XBCGPRecurrenceException* Val = NULL;

		while (Pos != NULL)
		{
			m_Exceptions.GetNextAssoc (Pos, Key, Val);

			ar << Key;
			ar << Val->m_dtStart;
			ar << Val->m_dtFinish;
			ar << Val->m_Deleted;

			if (!Val->m_Deleted)
			{
				ar << &(Val->m_Properties);
			}
		}
	}
	else
	{
		RemoveExceptions ();

		CBCGPRecurrenceBaseRule* pRule = NULL;
		ar >> pRule;

		ASSERT_VALID (pRule);

		SetRule (pRule);

		delete pRule;

		DWORD dwCount = 0;
		ar >> dwCount;

		for (DWORD i = 0; i < dwCount; i++)
		{
			COleDateTime Key;
			ar >> Key;

			XBCGPRecurrenceException* Val = new XBCGPRecurrenceException;
			ar >> Val->m_dtStart;
			ar >> Val->m_dtFinish;
			ar >> Val->m_Deleted;

			if (!Val->m_Deleted)
			{
				CBCGPAppointmentPropertyList* pList = NULL;
				ar >> pList;
				
				Val->m_Properties.CopyFrom (*pList);

				delete pList;
			}

			m_Exceptions[Key] = Val;
		}
	}
}

#endif // BCGP_EXCLUDE_PLANNER
