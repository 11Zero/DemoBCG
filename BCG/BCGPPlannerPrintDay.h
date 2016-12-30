#if !defined(AFX_BCGPPLANNERPRINTDAY_H__5B346DCD_33F3_4E8B_AA24_5D2015EF78D1__INCLUDED_)
#define AFX_BCGPPLANNERPRINTDAY_H__5B346DCD_33F3_4E8B_AA24_5D2015EF78D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
// BCGPPlannerPrintDay.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerView.h"
#include "BCGPPlannerPrint.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerPrintDay

class BCGCBPRODLLEXPORT CBCGPPlannerPrintDay : public CBCGPPlannerPrint
{
	DECLARE_DYNCREATE(CBCGPPlannerPrintDay)

public:
	virtual ~CBCGPPlannerPrintDay();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CBCGPPlannerView::BCGP_PLANNER_TIME_DELTA GetTimeDelta () const
	{
		return m_TimeDelta;
	}

	int GetViewHours () const;
	int GetViewHourOffset () const;

	virtual void OnPaint (CDC* pDC, CPrintInfo* pInfo);

	virtual void AdjustAppointments ();

protected:
	CBCGPPlannerPrintDay ();

	virtual void CalculateDates (const COleDateTime& date);	

	virtual void GetCaptionFormatStrings (CStringArray& sa);
	virtual void AdjustLayout (CDC* pDC, const CRect& rectClient);
	virtual void AdjustRects ();

	virtual void OnDrawClient (CDC* pDC);
	virtual void OnDrawHeader (CDC* pDC, const CRect& rectHeader);
	virtual void OnDrawHeaderAllDay (CDC* pDC, const CRect& rectHeader);
	virtual void OnDrawTimeBar (CDC* pDC, const CRect& rectBar);
	virtual void OnDrawAppointmentsDuration (CDC* pDC);

	virtual void OnDrawPageHeader (CDC* pDC);
	
	virtual void DrawAppointment (CDC* pDC, CBCGPAppointment* pApp, CBCGPAppointmentDrawStruct* pDS);

	virtual void CheckVisibleAppointments(const COleDateTime& date, const CRect& rect, 
		BOOL bFullVisible);

protected:
	CRect				m_rectTimeBar;
	int                 m_nHeaderHeight;
	int					m_nHeaderAllDayHeight;

	CBCGPPlannerView::BCGP_PLANNER_TIME_DELTA
						m_TimeDelta;
};

#endif // BCGP_EXCLUDE_PLANNER

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPLANNERPRINTDAY_H__5B346DCD_33F3_4E8B_AA24_5D2015EF78D1__INCLUDED_)
