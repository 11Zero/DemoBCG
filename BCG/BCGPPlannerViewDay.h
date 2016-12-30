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
// BCGPPlannerViewDay.h: interface for the CBCGPPlannerViewDay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPPLANNERVIEWDAY_H__99CF9A8D_73D3_4D83_836D_0E35487383E9__INCLUDED_)
#define AFX_BCGPPLANNERVIEWDAY_H__99CF9A8D_73D3_4D83_836D_0E35487383E9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerView.h"

class BCGCBPRODLLEXPORT CBCGPPlannerViewDay : public CBCGPPlannerView  
{
	friend class CBCGPPlannerManagerCtrl;

	DECLARE_DYNCREATE(CBCGPPlannerViewDay)

public:
	virtual ~CBCGPPlannerViewDay();

	enum
	{
		BCGP_PLANNER_TIMER_EVENT = 1974
	};

	enum
	{
		BCGP_PLANNER_DURATION_BAR_WIDTH = 6
	};

	enum
	{
		BCGP_PLANNER_APPOINTMENT_SPACE = 3
	};	

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual void SetDate (const COleDateTime& date);

	virtual void SetDateInterval (const COleDateTime& date1, const COleDateTime& date2);

	virtual COleDateTime CalculateDateStart (const COleDateTime& date) const;

	virtual BOOL IsDateInSelection (const COleDateTime& date) const;

	virtual COleDateTime GetDateFromPoint (const CPoint& point) const;
	virtual CRect GetRectFromDate(const COleDateTime& date) const;

	virtual BOOL IsDrawAppsShadow () const
	{
		return TRUE;
	}

	BCGP_PLANNER_TIME_DELTA GetTimeDelta () const;

	int GetFirstWorkingHour (BOOL bDefault = TRUE) const;
	int GetFirstWorkingMinute (BOOL bDefault = TRUE) const;

	int GetLastWorkingHour (BOOL bDefault = TRUE) const;
	int GetLastWorkingMinute (BOOL bDefault = TRUE) const;

	int GetFirstViewHour () const;
	int GetLastViewHour () const;
	int GetViewHours () const;
	int GetViewHourOffset () const;

	virtual void SetSelection (const COleDateTime& sel1, const COleDateTime& sel2, BOOL bRedraw = TRUE);
	
protected:
	CBCGPPlannerViewDay();

protected:
			void StartTimer (BOOL bRedraw);
			void StopTimer (BOOL bRedraw);
			BOOL IsCurrentTimeVisible () const;

	virtual COleDateTimeSpan GetMinimumSpan () const;

	virtual void StartEditAppointment (CBCGPAppointment* pApp);

	virtual void GetCaptionFormatStrings (CStringArray& sa);
	virtual void AdjustScrollSizes ();
	virtual void AdjustLayout (CDC* pDC, const CRect& rectClient);
	virtual void AdjustRects ();
	virtual void AdjustAppointments ();

	virtual BCGP_PLANNER_HITTEST HitTest (const CPoint& point) const;
	virtual BCGP_PLANNER_HITTEST HitTestArea (const CPoint& point) const;

	virtual void CheckVisibleAppointments(const COleDateTime& date, const CRect& rect, 
		BOOL bFullVisible);
	virtual void ClearVisibleUpDownIcons();
	virtual void CheckVisibleUpDownIcons(BOOL bFullVisible);

	virtual void OnPaint (CDC* pDC, const CRect& rectClient);
	virtual void OnDrawClient (CDC* pDC, const CRect& rect);
	virtual void OnDrawHeader (CDC* pDC, const CRect& rectHeader);
	virtual void OnDrawHeaderAllDay (CDC* pDC, const CRect& rectHeader);
	virtual void OnDrawTimeLine (CDC* pDC, const CRect& rectTime);
	virtual void OnDrawTimeBar (CDC* pDC, const CRect& rectBar, BOOL bDrawTimeLine);
	virtual void OnDrawAppointmentsDuration (CDC* pDC);

	virtual BYTE OnDrawAppointments (CDC* pDC, const CRect& rect = CRect (0, 0, 0, 0), 
		const COleDateTime& date = COleDateTime ());

	virtual void OnDrawUpDownIcons (CDC* pDC);

	virtual void DrawAppointment (CDC* pDC, CBCGPAppointment* pApp, CBCGPAppointmentDrawStructEx* pDS);

	virtual void AddUpDownRect(BYTE nType, const CRect& rect);
	virtual void AddHeaderUpDownRect(BYTE nType, const CRect& rect);

	virtual BOOL OnLButtonDown(UINT nFlags, CPoint point);
	virtual BOOL OnMouseMove(UINT nFlags, CPoint point);
	virtual BOOL OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL OnTimer(UINT_PTR nIDEvent);

	virtual void GetDragScrollRect (CRect& rect);

	virtual DROPEFFECT OnDragScroll(DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);

	virtual void OnActivate(CBCGPPlannerManagerCtrl* pPlanner, const CBCGPPlannerView* pOldView);
	virtual void OnDeactivate(CBCGPPlannerManagerCtrl* pPlanner);

	virtual COleDateTime GetNextDateStart () const;
	virtual COleDateTime GetPrevDateStart () const;

	virtual BOOL EnsureVisible(const CBCGPAppointment* pApp, BOOL bPartialOK);

	int GetFirstSelectionHour () const;
	int GetFirstSelectionMinute () const;

	int GetLastSelectionHour () const;
	int GetLastSelectionMinute () const;

	virtual BOOL CanUseHeaderScrolling() const
	{
		return TRUE;
	}
	
	virtual CString GetAccName() const;

protected:
	CRect				m_rectTimeBar;
	int					m_nHeaderHeight;
	int					m_nHeaderAllDayHeight;

	CArray<CRect, CRect&>
						m_HeaderUpRects;
	CArray<CRect, CRect&>
						m_HeaderDownRects;

	COleDateTime		m_CurrentTime;
	UINT_PTR			m_Timer;
	
	CArray<BOOL, BOOL>	m_SelectionAllDay;
};

#endif // BCGP_EXCLUDE_PLANNER

#endif // !defined(AFX_BCGPPLANNERVIEWDAY_H__99CF9A8D_73D3_4D83_836D_0E35487383E9__INCLUDED_)
