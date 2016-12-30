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
// BCGPPlannerViewMonth.h: interface for the CBCGPPlannerViewMonth class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPPLANNERVIEWMONTH_H__BEAF25DC_41EC_4DA2_B94D_500C660AEFFF__INCLUDED_)
#define AFX_BCGPPLANNERVIEWMONTH_H__BEAF25DC_41EC_4DA2_B94D_500C660AEFFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerView.h"

class BCGCBPRODLLEXPORT CBCGPPlannerViewMonth : public CBCGPPlannerView  
{
	friend class CBCGPPlannerManagerCtrl;

	DECLARE_DYNCREATE(CBCGPPlannerViewMonth)

public:
	virtual ~CBCGPPlannerViewMonth();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual void SetDate (const COleDateTime& date);
	virtual void SetDateInterval (const COleDateTime& date1, const COleDateTime& date2);

	virtual COleDateTime CalculateDateStart (const COleDateTime& date) const;

	virtual void SetDrawTimeFinish (BOOL bDraw);
	virtual BOOL IsDrawTimeFinish () const;

	virtual void SetDrawTimeAsIcons (BOOL bDraw);
	virtual BOOL IsDrawTimeAsIcons () const;

	virtual void SetCompressWeekend (BOOL bCompress);
	virtual BOOL IsCompressWeekend () const;

	virtual COleDateTime GetDateFromPoint (const CPoint& point) const;
	virtual int GetWeekFromPoint (const CPoint& point) const;

	virtual void SetSelection (const COleDateTime& sel1, const COleDateTime& sel2, BOOL bRedraw = TRUE);
	
	virtual BOOL CanCaptureAppointment (BCGP_PLANNER_HITTEST hitCapturedAppointment) const;

protected:
	CBCGPPlannerViewMonth();

	virtual void GetCaptionFormatStrings (CStringArray& sa);
	virtual void AdjustScrollSizes ();
	virtual void AdjustLayout (CDC* pDC, const CRect& rectClient);
	virtual void AdjustRects ();
	virtual void AdjustAppointments ();

	virtual BCGP_PLANNER_HITTEST HitTestArea (const CPoint& point) const;
	virtual DROPEFFECT HitTestDrag (DWORD dwKeyState, const CPoint& point) const;

	virtual void DrawHeader (CDC* pDC, const CRect& rect, int dxColumn);

	virtual void OnPaint (CDC* pDC, const CRect& rectClient);
	virtual void OnDrawClient (CDC* pDC, const CRect& rect);
	virtual void OnDrawHeader (CDC* pDC, const CRect& rectHeader);
	virtual void OnDrawWeekBar (CDC* pDC, const CRect& rectBar);
	
	virtual BOOL OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll);

	virtual void OnActivate(CBCGPPlannerManagerCtrl* pPlanner, const CBCGPPlannerView* pOldView);

	COleDateTime GetFirstWeekDay2 (const COleDateTime& date, int nWeekStart) const;

	virtual void AddUpDownRect(BYTE nType, const CRect& rect);

	virtual CString GetAccName() const;

protected:	
	int     m_nHeaderHeight;
	int     m_nWeekBarWidth;
	int		m_nDuration;

	BOOL	m_bDrawTimeFinish;
	BOOL	m_bDrawTimeAsIcons;

	BOOL	m_bCompressWeekend;

	CArray<CRect, CRect&>
			m_WeekRects;
};

#endif // BCGP_EXCLUDE_PLANNER

#endif // !defined(AFX_BCGPPLANNERVIEWMONTH_H__BEAF25DC_41EC_4DA2_B94D_500C660AEFFF__INCLUDED_)
