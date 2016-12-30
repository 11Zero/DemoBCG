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
// BCGPPlannerViewWeek.h: interface for the CBCGPPlannerViewWeek class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPPLANNERVIEWWEEK_H__D39A55F3_4860_4551_8B09_A5940AB40CD7__INCLUDED_)
#define AFX_BCGPPLANNERVIEWWEEK_H__D39A55F3_4860_4551_8B09_A5940AB40CD7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerView.h"

class BCGCBPRODLLEXPORT CBCGPPlannerViewWeek : public CBCGPPlannerView  
{
	friend class CBCGPPlannerManagerCtrl;

	DECLARE_DYNCREATE(CBCGPPlannerViewWeek)

public:
	virtual ~CBCGPPlannerViewWeek();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual void SetDate (const COleDateTime& date);

	virtual COleDateTime CalculateDateStart (const COleDateTime& date) const;

	virtual void SetDrawTimeFinish (BOOL bDraw);
	virtual BOOL IsDrawTimeFinish () const;

	virtual void SetDrawTimeAsIcons (BOOL bDraw);
	virtual BOOL IsDrawTimeAsIcons () const;

	virtual COleDateTime GetDateFromPoint (const CPoint& point) const;

	virtual void SetSelection (const COleDateTime& sel1, const COleDateTime& sel2, BOOL bRedraw = TRUE);
	
	virtual BOOL CanCaptureAppointment (BCGP_PLANNER_HITTEST hitCapturedAppointment) const;

protected:
	CBCGPPlannerViewWeek();

	virtual void GetCaptionFormatStrings (CStringArray& sa);
	virtual void AdjustScrollSizes ();
	virtual void AdjustLayout (CDC* pDC, const CRect& rectClient);
	virtual void AdjustRects ();
	virtual void AdjustAppointments ();

	virtual BCGP_PLANNER_HITTEST HitTestArea (const CPoint& point) const;
	
	virtual void OnPaint (CDC* pDC, const CRect& rectClient);
	virtual void OnDrawClient (CDC* pDC, const CRect& rect);

	virtual BOOL OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll);

	virtual void OnActivate(CBCGPPlannerManagerCtrl* pPlanner, const CBCGPPlannerView* pOldView);

	COleDateTime GetFirstWeekDay2 (const COleDateTime& date, int nWeekStart) const;

	virtual void AddUpDownRect(BYTE nType, const CRect& rect);

	virtual CString GetAccName() const;

protected:	
	BOOL	m_bDrawTimeFinish;
	BOOL	m_bDrawTimeAsIcons;
};

#endif // BCGP_EXCLUDE_PLANNER

#endif // !defined(AFX_BCGPPLANNERVIEWWEEK_H__D39A55F3_4860_4551_8B09_A5940AB40CD7__INCLUDED_)
