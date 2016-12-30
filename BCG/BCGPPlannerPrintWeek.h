#if !defined(AFX_BCGPPLANNERPRINTWEEK_H__5B346DCD_33F3_4E8B_AA24_5D2015EF78D1__INCLUDED_)
#define AFX_BCGPPLANNERPRINTWEEK_H__5B346DCD_33F3_4E8B_AA24_5D2015EF78D1__INCLUDED_

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
// BCGPPlannerPrintWeek.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerPrint.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerPrintWeek

class BCGCBPRODLLEXPORT CBCGPPlannerPrintWeek : public CBCGPPlannerPrint
{
	DECLARE_DYNCREATE(CBCGPPlannerPrintWeek)

public:
	virtual ~CBCGPPlannerPrintWeek();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual void OnPaint (CDC* pDC, CPrintInfo* pInfo);

	virtual void SetDrawTimeFinish (BOOL bDraw);
	virtual BOOL IsDrawTimeFinish () const;

	virtual void AdjustAppointments ();

protected:
	CBCGPPlannerPrintWeek ();

	virtual void CalculateDates (const COleDateTime& date);

	virtual CString GetPageHeaderText () const;
	virtual void GetCaptionFormatStrings (CStringArray& sa);
	virtual void AdjustLayout (CDC* pDC, const CRect& rectClient);
	virtual void AdjustRects ();

	virtual void OnDrawClient (CDC* pDC);
	
	COleDateTime GetFirstWeekDay2 (const COleDateTime& date, int nWeekStart) const;

	BOOL	m_bDrawTimeFinish;
};

#endif // BCGP_EXCLUDE_PLANNER

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPLANNERPRINTWEEK_H__5B346DCD_33F3_4E8B_AA24_5D2015EF78D1__INCLUDED_)
