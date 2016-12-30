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
// BCGPPlannerViewWorkWeek.cpp: implementation of the CBCGPPlannerViewWorkWeek class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPPlannerViewWorkWeek.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerManagerCtrl.h"
#include "BCGPMath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CBCGPPlannerViewWorkWeek, CBCGPPlannerViewDay)

CBCGPPlannerViewWorkWeek::CBCGPPlannerViewWorkWeek()
	: CBCGPPlannerViewDay ()
{
	m_DateEnd = m_DateStart + COleDateTimeSpan (4, 23, 59, 59);
}

CBCGPPlannerViewWorkWeek::~CBCGPPlannerViewWorkWeek()
{

}

#ifdef _DEBUG
void CBCGPPlannerViewWorkWeek::AssertValid() const
{
	CBCGPPlannerViewDay::AssertValid();
}

void CBCGPPlannerViewWorkWeek::Dump(CDumpContext& dc) const
{
	CBCGPPlannerViewDay::Dump(dc);
}
#endif

void CBCGPPlannerViewWorkWeek::OnActivate(CBCGPPlannerManagerCtrl* pPlanner, const CBCGPPlannerView* pOldView)
{
	ASSERT_VALID(pPlanner);

	if (pOldView != NULL)
	{
		m_Date = pOldView->GetDate ();
	}

	m_DateStart = CalculateDateStart (
		COleDateTime(m_Date.GetYear (), m_Date.GetMonth (), m_Date.GetDay (), 0, 0, 0));
	m_DateEnd   = m_DateStart + COleDateTimeSpan (pPlanner->GetWorkWeekInterval () - 1, 23, 59, 59);

	COleDateTime sel1 (m_Date);
	COleDateTime sel2 (m_Date);

	if (pOldView != NULL)
	{
		sel1 = pOldView->GetSelectionStart ();
		sel2 = pOldView->GetSelectionEnd ();
	}

	const int nMinuts = CBCGPPlannerView::GetTimeDeltaInMinuts (pPlanner->GetTimeDelta ());

	sel1 = COleDateTime (m_DateStart.GetYear (), m_DateStart.GetMonth (), m_DateStart.GetDay (),
		pPlanner->GetFirstSelectionHour (), (int)(pPlanner->GetFirstSelectionMinute () / nMinuts) * nMinuts, 0);
	sel2 = sel1 + COleDateTimeSpan (0, 0, nMinuts - 1, 59);

	//SetSelection (sel1, sel2, FALSE);

	CBCGPPlannerView::OnActivate (pPlanner, NULL);

	SetSelection (sel1, sel2);
}

void CBCGPPlannerViewWorkWeek::SetDate (const COleDateTime& date)
{
	COleDateTime dt (date.GetYear (), date.GetMonth (), date.GetDay (), 0, 0, 0);

	m_DateStart = CalculateDateStart (dt);
	m_DateEnd   = m_DateStart + COleDateTimeSpan (GetWorkWeekInterval () - 1, 23, 59, 59);

	CBCGPPlannerView::SetDate (dt);
}

COleDateTime CBCGPPlannerViewWorkWeek::CalculateDateStart (const COleDateTime& date) const
{
	return CBCGPPlannerView::GetFirstWeekDay (date, 2);//CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () + 1);
}

COleDateTime CBCGPPlannerViewWorkWeek::GetNextDateStart () const
{
	return m_DateStart + COleDateTimeSpan (7, 0, 0, 0);
}

COleDateTime CBCGPPlannerViewWorkWeek::GetPrevDateStart () const
{
	return m_DateStart - COleDateTimeSpan (7, 0, 0, 0);
}

int CBCGPPlannerViewWorkWeek::GetWorkWeekInterval () const
{
	ASSERT_VALID(GetPlanner ());
	return GetPlanner ()->GetWorkWeekInterval ();
}

CString CBCGPPlannerViewWorkWeek::GetAccName() const
{
	return _T("Work Week View");
}

#endif // BCGP_EXCLUDE_PLANNER
