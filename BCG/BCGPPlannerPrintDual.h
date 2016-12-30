#if !defined(AFX_BCGPPLANNERPRINTDUAL_H__5B346DCD_33F3_4E8B_AA24_5D2015EF78D1__INCLUDED_)
#define AFX_BCGPPLANNERPRINTDUAL_H__5B346DCD_33F3_4E8B_AA24_5D2015EF78D1__INCLUDED_

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
// BCGPPlannerPrintDual.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerPrintDay.h"
#include "BCGPPlannerPrintWeek.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerPrintDual

class BCGCBPRODLLEXPORT CBCGPPlannerPrintDual : public CBCGPPlannerPrint
{
	DECLARE_DYNCREATE(CBCGPPlannerPrintDual)

public:
	virtual ~CBCGPPlannerPrintDual();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CBCGPPlannerView::BCGP_PLANNER_TIME_DELTA GetTimeDelta () const;

	virtual void SetDrawTimeFinish (BOOL bDraw);
	virtual BOOL IsDrawTimeFinish () const;

	virtual void SetDrawPageHeader (BOOL bDraw);

	virtual void PreparePrinting (CDC* pDC, CPrintInfo* pInfo,  
		CBCGPPlannerManagerCtrl* pPlanner);

	virtual void OnPaint (CDC* pDC, CPrintInfo* pInfo);

protected:
	CBCGPPlannerPrintDual ();

	virtual void AdjustPageHeader (CDC* pDC);

	CBCGPPlannerPrintDay* m_pPrintDay;
	CBCGPPlannerPrintWeek* m_pPrintWeek;
};

#endif // BCGP_EXCLUDE_PLANNER

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPLANNERPRINTDUAL_H__5B346DCD_33F3_4E8B_AA24_5D2015EF78D1__INCLUDED_)
