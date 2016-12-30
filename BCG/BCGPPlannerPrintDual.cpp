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
// BCGPPlannerPrintDual.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPPlannerPrintDual.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerManagerCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerPrintDual

IMPLEMENT_DYNCREATE(CBCGPPlannerPrintDual, CBCGPPlannerPrint)

CBCGPPlannerPrintDual::CBCGPPlannerPrintDual()
	: CBCGPPlannerPrint  ()
	, m_pPrintDay        (NULL)
	, m_pPrintWeek       (NULL)
{
	m_pPrintDay = (CBCGPPlannerPrintDay*)RUNTIME_CLASS(CBCGPPlannerPrintDay)->CreateObject ();
	m_pPrintWeek = (CBCGPPlannerPrintWeek*)RUNTIME_CLASS(CBCGPPlannerPrintWeek)->CreateObject ();
}

CBCGPPlannerPrintDual::~CBCGPPlannerPrintDual()
{
	if (m_pPrintDay != NULL)
	{
		delete m_pPrintDay;
		m_pPrintDay = NULL;
	}

	if (m_pPrintWeek != NULL)
	{
		delete m_pPrintWeek;
		m_pPrintWeek = NULL;
	}
}

#ifdef _DEBUG
void CBCGPPlannerPrintDual::AssertValid() const
{
	CObject::AssertValid();
}

void CBCGPPlannerPrintDual::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif

void CBCGPPlannerPrintDual::PreparePrinting (CDC* pDC, CPrintInfo* pInfo, 
										 CBCGPPlannerManagerCtrl* pPlanner)
{
	ASSERT_VALID (pPlanner);
	ASSERT_VALID (pDC);
	ASSERT (pInfo != NULL);

	ASSERT_VALID(m_pPrintDay);
	ASSERT_VALID(m_pPrintWeek);

	if (pPlanner->GetSafeHwnd () == NULL)
	{
		return;
	}

	CBCGPPlannerPrint::PreparePrinting (pDC, pInfo, pPlanner);

	m_pPrintDay->SetDrawPageFooter (FALSE);
	m_pPrintDay->PreparePrinting (pDC, pInfo, pPlanner);
	m_pPrintWeek->SetDrawPageFooter (FALSE);
	m_pPrintWeek->PreparePrinting (pDC, pInfo, pPlanner);
}

void CBCGPPlannerPrintDual::AdjustPageHeader (CDC* /*pDC*/)
{
	m_rectPageHeader.SetRectEmpty ();
}

void CBCGPPlannerPrintDual::OnPaint (CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID (pDC);
	ASSERT (pInfo != NULL);

	ASSERT_VALID(m_pPrintDay);
	ASSERT_VALID(m_pPrintWeek);

	CRect rect (pInfo->m_rectDraw);

	CBCGPPlannerPrint::OnPaint (pDC, pInfo);

	pInfo->m_rectDraw = m_rectApps;

	pInfo->m_rectDraw.right = pInfo->m_rectDraw.left + pInfo->m_rectDraw.Width () / 2 -
		3 * m_OnePoint.cx;

	m_pPrintDay->AdjustAppointments ();
	m_pPrintDay->OnPaint (pDC, pInfo);

	pInfo->m_rectDraw.left = pInfo->m_rectDraw.right + 6 * m_OnePoint.cx;
	pInfo->m_rectDraw.right = m_rectApps.right;

	m_pPrintWeek->AdjustAppointments ();
	m_pPrintWeek->OnPaint (pDC, pInfo);

	pInfo->m_rectDraw = rect;
}

CBCGPPlannerView::BCGP_PLANNER_TIME_DELTA CBCGPPlannerPrintDual::GetTimeDelta () const
{
	ASSERT_VALID(m_pPrintDay);

	return m_pPrintDay->GetTimeDelta ();
}

void CBCGPPlannerPrintDual::SetDrawTimeFinish (BOOL bDraw)
{
	ASSERT_VALID(m_pPrintWeek);

	m_pPrintWeek->SetDrawTimeFinish (bDraw);
}

BOOL CBCGPPlannerPrintDual::IsDrawTimeFinish () const
{
	ASSERT_VALID(m_pPrintWeek);
	
	return m_pPrintWeek->IsDrawTimeFinish ();
}

void CBCGPPlannerPrintDual::SetDrawPageHeader (BOOL bDraw)
{
	ASSERT_VALID(m_pPrintDay);
	ASSERT_VALID(m_pPrintWeek);

	CBCGPPlannerPrint::SetDrawPageHeader (bDraw);

	m_pPrintDay->SetDrawPageHeader (bDraw);
	m_pPrintWeek->SetDrawPageHeader (bDraw);
}

#endif // BCGP_EXCLUDE_PLANNER
