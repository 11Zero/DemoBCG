#if !defined(AFX_BCGPPLANNERVIEW_H__9A8EBBB8_3FCC_4D8A_BE23_C88D8D1E1A08__INCLUDED_)
#define AFX_BCGPPLANNERVIEW_H__9A8EBBB8_3FCC_4D8A_BE23_C88D8D1E1A08__INCLUDED_

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
// BCGPPlannerView.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPAppointment.h"
#include "BCGPAppointmentProperty.h"
#include "bcgglobals.h"

class CBCGPAppointmentBaseStorage;
class CBCGPPlannerManagerCtrl;

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerView window

class BCGCBPRODLLEXPORT CBCGPPlannerView : public CObject
{
	friend class CBCGPAppointment;
	friend class CBCGPPlannerManagerCtrl;

	DECLARE_DYNAMIC(CBCGPPlannerView)

public:
	virtual ~CBCGPPlannerView();

	enum BCGP_PLANNER_HITTEST
	{
		BCGP_PLANNER_HITTEST_FIRST              =  0,
		BCGP_PLANNER_HITTEST_NOWHERE            =  BCGP_PLANNER_HITTEST_FIRST, // 
		BCGP_PLANNER_HITTEST_TIMEBAR            =  1, // day view timebar area
		BCGP_PLANNER_HITTEST_WEEKBAR            =  2, // month view weekbar area
		BCGP_PLANNER_HITTEST_HEADER             =  3, // day view or month view header area
		BCGP_PLANNER_HITTEST_HEADER_ALLDAY      =  4, // day view all day header area
		BCGP_PLANNER_HITTEST_HEADER_RESOURCE    =  5, // day view all day header area
		BCGP_PLANNER_HITTEST_CLIENT             =  6, // client area
		BCGP_PLANNER_HITTEST_DAY_CAPTION        =  7, // caption of the day in week and month views
		BCGP_PLANNER_HITTEST_WEEK_CAPTION       =  8, // caption of the week in month view
		BCGP_PLANNER_HITTEST_ICON_UP            =  9, // up icons
		BCGP_PLANNER_HITTEST_ICON_DOWN          = 10, // down icons
		// this is appointment hit tests. include only new appointments enums
		BCGP_PLANNER_HITTEST_APPOINTMENT        = 11, // appointment area
		BCGP_PLANNER_HITTEST_APPOINTMENT_MOVE   = 12, // appointment move area (left side)
		BCGP_PLANNER_HITTEST_APPOINTMENT_TOP    = 13, // day view appointment top area (top side)
		BCGP_PLANNER_HITTEST_APPOINTMENT_BOTTOM = 14, // day view appointment bottom area (bottom side)
		BCGP_PLANNER_HITTEST_APPOINTMENT_LEFT   = 15, // day view appointment left area (left side)
		BCGP_PLANNER_HITTEST_APPOINTMENT_RIGHT  = 16, // day view appointment right area (right side)
		BCGP_PLANNER_HITTEST_LAST               = BCGP_PLANNER_HITTEST_APPOINTMENT_RIGHT
	};

	enum BCGP_PLANNER_TIME_DELTA
	{
		BCGP_PLANNER_TIME_DELTA_FIRST = 0,
		BCGP_PLANNER_TIME_DELTA_60    = BCGP_PLANNER_TIME_DELTA_FIRST,
		BCGP_PLANNER_TIME_DELTA_30    = 1,
		BCGP_PLANNER_TIME_DELTA_20    = 2,
		BCGP_PLANNER_TIME_DELTA_15    = 3,
		BCGP_PLANNER_TIME_DELTA_10    = 4,
		BCGP_PLANNER_TIME_DELTA_6     = 5,
		BCGP_PLANNER_TIME_DELTA_5     = 6,
		BCGP_PLANNER_TIME_DELTA_4     = 7,
		BCGP_PLANNER_TIME_DELTA_3     = 8,
		BCGP_PLANNER_TIME_DELTA_2     = 9,
		BCGP_PLANNER_TIME_DELTA_1     = 10,
		BCGP_PLANNER_TIME_DELTA_LAST  = BCGP_PLANNER_TIME_DELTA_1
	};

	enum BCGP_PLANNER_ADJUST_ACTION
	{
		BCGP_PLANNER_ADJUST_ACTION_FIRST        = 0,
		BCGP_PLANNER_ADJUST_ACTION_NONE         = BCGP_PLANNER_ADJUST_ACTION_FIRST,
		BCGP_PLANNER_ADJUST_ACTION_APPOINTMENTS = 1,
		BCGP_PLANNER_ADJUST_ACTION_LAYOUT       = 2,
		BCGP_PLANNER_ADJUST_ACTION_LAST         = BCGP_PLANNER_ADJUST_ACTION_LAYOUT
	};

	enum BCGP_PLANNER_WORKING_STATUS
	{
		BCGP_PLANNER_WORKING_STATUS_FIRST                    = 0,
		BCGP_PLANNER_WORKING_STATUS_UNKNOWN                  = BCGP_PLANNER_WORKING_STATUS_FIRST,
		BCGP_PLANNER_WORKING_STATUS_ISNOTWORKING             = 1,
		BCGP_PLANNER_WORKING_STATUS_ISWORKING                = 2,
		BCGP_PLANNER_WORKING_STATUS_ISNORMALWORKINGDAY       = 3,
		BCGP_PLANNER_WORKING_STATUS_ISNORMALWORKINGDAYINWEEK = 4,
		BCGP_PLANNER_WORKING_STATUS_LAST                     = BCGP_PLANNER_WORKING_STATUS_ISNORMALWORKINGDAYINWEEK
	};

	struct XBCGPPlannerWorkingParameters
	{
		const CBCGPPlannerView*	m_pView;
		COLORREF				m_clrWorking;
		COLORREF				m_clrNonWorking;

		XBCGPPlannerWorkingParameters(const CBCGPPlannerView* pView, COLORREF clrWorking = CLR_DEFAULT, COLORREF clrNonWorking = CLR_DEFAULT)
			: m_pView        (pView)
			, m_clrWorking   (clrWorking)
			, m_clrNonWorking(clrNonWorking)
		{
		}
	};

	static long GetTimeDeltaInMinuts (BCGP_PLANNER_TIME_DELTA delta);
	static COleDateTime GetFirstWeekDay (const COleDateTime& day, int nWeekStart);

	static BOOL Is24Hours ();
	static CString GetTimeDesignator (BOOL bAM);
	static CString GetTimeSeparator ();
	static BOOL IsDateBeforeMonth ();
	static CString GetDateSeparator ();
	static BOOL IsDayLZero ();
	static BOOL IsMonthLZero ();
	static int round(double val);
	static BOOL IsAppointmentInDate (const CBCGPAppointment& rApp, const COleDateTime& date);
	static BOOL IsOneDay (const COleDateTime& date1, const COleDateTime& date2);
	static CString GetAccIntervalFormattedString(const COleDateTime& date1, const COleDateTime& date2);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	const COleDateTime& GetDate () const
	{
		return m_Date;
	}

	const COleDateTime& GetDateStart () const
	{
		return m_DateStart;
	}

	const COleDateTime& GetDateEnd () const
	{
		return m_DateEnd;
	}

	int GetViewDuration () const	// Days
	{
		return (int)(m_DateEnd - m_DateStart).GetTotalDays () + 1;
	}

	virtual UINT GetCurrentResourceID () const;
	virtual BOOL SetCurrentResourceID (UINT nResourceID, BOOL bRedraw = TRUE, BOOL bNotify = FALSE);
	virtual BOOL OnUpdateStorage (){return FALSE;}

	virtual COleDateTime CalculateDateStart (const COleDateTime& /*date*/) const = 0;
	virtual BOOL IsDateInSelection (const COleDateTime& date) const;

	virtual void SetSelection (const COleDateTime& sel1, const COleDateTime& sel2, BOOL bRedraw = TRUE);

	virtual COleDateTime GetDateFromPoint (const CPoint& point) const;
	virtual int GetWeekFromPoint (const CPoint& point) const;
	virtual UINT GetResourceFromPoint (const CPoint& point) const;
	virtual CBCGPAppointment* GetAppointmentFromPoint (const CPoint& point);
	virtual CRect GetRectFromDate(const COleDateTime& date) const;

	virtual void SetDate (const COleDateTime& date);

	virtual void SetDateInterval (const COleDateTime& /*date1*/, const COleDateTime& /*date2*/) {}
	virtual void SetCompressWeekend (BOOL /*bCompress*/) {}
	virtual BOOL IsCompressWeekend () const	{	return FALSE;	}
	virtual void SetDrawTimeFinish (BOOL /*bDraw*/) {};
	virtual BOOL IsDrawTimeFinish () const	{ return FALSE;	};
	virtual void SetDrawTimeAsIcons (BOOL /*bDraw*/) {};
	virtual BOOL IsDrawTimeAsIcons () const	{	return FALSE;	};
	virtual BOOL IsDrawAppsShadow () const	{	return FALSE;	};

	const CString& GetCaptionFormat () const
	{
		return m_strCaptionFormat;
	}

	BOOL IsActive () const
	{
		return m_bActive;
	}

	CSize GetClockIconSize () const;
	
	void DrawClockIcon (CDC* pDC, const CPoint& point, const COleDateTime& time) const;

	virtual void QueryAppointments ();

	const COleDateTime& GetSelectionStart () const
	{
		return m_Selection[0];
	}

	const COleDateTime& GetSelectionEnd () const
	{
		return m_Selection[1];
	}

	CBCGPPlannerManagerCtrl* GetPlanner ()
	{
		return m_pPlanner;
	}

	const CRect& GetAppointmentsRect() const
	{
		return m_rectApps;
	}

	int GetRowHeight () const
	{
		return m_nRowHeight;
	}
	
	DWORD GetDrawFlags() const;

	virtual BCGP_PLANNER_WORKING_STATUS GetWorkingPeriodParameters (int ResourceId, const COleDateTime& dtStart, const COleDateTime& dtEnd, XBCGPPlannerWorkingParameters& parameters) const;

protected:
	CBCGPPlannerView ();

protected:
	void ScreenToClient (LPPOINT point) const;
	void ScreenToClient (LPRECT rect) const;

	void ClientToScreen (LPPOINT point) const;
	void ClientToScreen (LPRECT rect) const;

	virtual void GetCaptionFormatStrings (CStringArray& sa) = 0;
	virtual void AdjustLayout (CDC* /*pDC*/, const CRect& /*rectClient*/) = 0;
	virtual void AdjustRects () = 0;
	virtual void AdjustAppointments () = 0;

	virtual void AdjustCaptionFormat (CDC* pDC);
	virtual void AdjustScrollSizes ();
	virtual void AdjustLayout (BOOL bRedraw = TRUE);

	// full hittest with areas and appointments
	virtual BCGP_PLANNER_HITTEST HitTest (const CPoint& point) const;
	// only area hittest
	virtual BCGP_PLANNER_HITTEST HitTestArea (const CPoint& point) const;
	// only appointments hittest
	virtual BCGP_PLANNER_HITTEST HitTestAppointment (const CPoint& point) const;
	
	virtual DROPEFFECT HitTestDrag (DWORD dwKeyState, const CPoint& point) const;
	virtual DROPEFFECT CanDropAppointmets (DWORD dwKeyState, const CPoint& point) const;

	virtual void CheckVisibleAppointments(const COleDateTime& date, const CRect& rect, 
		BOOL bFullVisible);
	virtual void ClearVisibleUpDownIcons();
	virtual void CheckVisibleUpDownIcons(BOOL bFullVisible);

	virtual void OnPaint (CDC* /*pDC*/, const CRect& /*rectClient*/) = 0;
	virtual void OnDrawClient (CDC* /*pDC*/, const CRect& /*rect*/) = 0;

	// 0 - all appointments in view; 
	// 1 - upper appointments not in view; 
	// 2 - lower appointments not in view
	// 3 - upper and lower not in view
	// if date = COleDateTime () - draws all appointments 
	//    in this case rect ignored, return value - always 0
	virtual BYTE OnDrawAppointments (CDC* pDC, const CRect& rect = CRect (0, 0, 0, 0), 
		const COleDateTime& date = COleDateTime ());

	virtual void OnDrawUpDownIcons (CDC* pDC);

	virtual void DrawHeader (CDC* pDC, const CRect& rect, int dxColumn);

	void ConstructCaptionText (const COleDateTime& day, CString& strText, const CString& strFormat);

	virtual void DrawCaptionText (CDC* pDC, CRect rect, 
		const COleDateTime& day, COLORREF colorText, int nAlign = DT_CENTER, BOOL bHighlight = FALSE);

	virtual void DrawCaptionText (CDC* pDC, CRect rect, 
		const CString& strText, COLORREF colorText, int nAlign = DT_CENTER, BOOL bHighlight = FALSE);
	
	virtual void DrawAppointment (CDC* pDC, CBCGPAppointment* pApp, CBCGPAppointmentDrawStructEx* pDS);

	virtual BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	virtual BOOL OnMouseMove(UINT nFlags, CPoint point);

	virtual BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

	virtual BOOL OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	virtual BOOL OnLButtonDown(UINT nFlags, CPoint point);
	
	virtual BOOL OnLButtonUp(UINT nFlags, CPoint point);

	virtual BOOL OnRButtonDown(UINT nFlags, CPoint point);

	virtual BOOL OnLButtonDblClk(UINT nFlags, CPoint point);

	virtual BOOL OnKeyDown(UINT /*nChar*/, UINT /*nRepCnt*/, UINT /*nFlags*/) = 0;

	virtual BOOL OnTimer(UINT_PTR nIDEvent);
	
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll);

	virtual void GetDragScrollRect (CRect& rect);

	virtual DROPEFFECT OnDragScroll(DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);

	int GetHeaderScrollOffset () const
	{
		return m_nHeaderScrollOffset;
	}
	int GetHeaderScrollPage () const
	{
		return m_nHeaderScrollPage;
	}
	int GetHeaderScrollTotal () const
	{
		return m_nHeaderScrollTotal;
	}

	int GetScrollOffset () const
	{
		return m_nScrollOffset;
	}
	int GetScrollPage () const
	{
		return m_nScrollPage;
	}
	int GetScrollTotal () const
	{
		return m_nScrollTotal;
	}

	const CBCGPPlannerManagerCtrl* GetPlanner () const
	{
		return m_pPlanner;
	}

	virtual HFONT SetFont (HFONT hFont);

	HFONT GetFont (BOOL bBold = FALSE);
	HFONT GetFontVert ();
	HFONT SetCurrFont (CDC* pDC, BOOL bBold = FALSE);

	virtual void OnActivate(CBCGPPlannerManagerCtrl* m_pPlanner, const CBCGPPlannerView* pOldView);
	virtual void OnDeactivate(CBCGPPlannerManagerCtrl* m_pPlanner);

	BOOL IsReadOnly () const;

	XBCGPAppointmentArray& GetQueryedAppointments ();

	const XBCGPAppointmentArray& GetQueryedAppointments () const;

	XBCGPAppointmentList& GetSelectedAppointments ();

	const XBCGPAppointmentList& GetSelectedAppointments () const;

	void SelectAppointment (CBCGPAppointment* pApp, BOOL bSelect, BOOL bRedraw = TRUE);
	
	void ClearAppointmentSelection (BOOL bRedraw = TRUE);

	BOOL IsDragDrop () const;

	BOOL IsCaptured () const;

	virtual BOOL CanStartDragDrop () const;

	virtual BOOL CanCaptureAppointment (BCGP_PLANNER_HITTEST hitCapturedAppointment) const;

	virtual COleDateTimeSpan GetMinimumSpan () const;

	virtual void StartCapture ();

	virtual void StopCapture ();

	virtual void RestoreCapturedAppointment ();

	DROPEFFECT GetDragEffect () const;

	XBCGPAppointmentArray& GetDragedAppointments ()
	{
		return m_arDragAppointments;
	}

	const XBCGPAppointmentArray& GetDragedAppointments () const
	{
		return m_arDragAppointments;
	}

	void ClearDragedAppointments ();

	BOOL IsCaptureDatesMatched () const
	{
		return m_dtCaptureCurrent == m_dtCaptureStart;
	}

	BOOL IsCaptureAreasMatched () const
	{
		return m_htCaptureAreaCurrent == m_htCaptureAreaStart;
	}

	BOOL IsCaptureResourcesMatched () const
	{
		return m_htCaptureResourceCurrent == m_htCaptureResourceStart;
	}

	virtual BOOL IsCaptureMatched () const;

	const COleDateTime& GetCaptureDateStart () const
	{
		return m_dtCaptureStart;
	}

	const COleDateTime& GetCaptureDateCurrent () const
	{
		return m_dtCaptureCurrent;
	}
	
	virtual void StartEditAppointment (CBCGPAppointment* pApp);
	virtual void StopEditAppointment ();

	void OnDateChanged ();

	virtual COLORREF GetHourLineColor (BOOL bWorkingHours, BOOL bHour);
	virtual void OnFillPlanner (CDC* pDC, CRect rect, BOOL bWorkingArea);
	virtual COLORREF OnFillPlannerCaption (CDC* pDC, 
		CRect rect, BOOL bIsToday, BOOL bIsSelected, BOOL bNoBorder = FALSE);

	BCGP_PLANNER_ADJUST_ACTION GetAdjustAction () const
	{
		return m_AdjustAction;
	}

	virtual void AddUpDownRect(BYTE /*nType*/, const CRect& /*rect*/){};
	
	void InitToolTipInfo ();
	void ClearToolTipInfo ();
	void AddToolTipInfo (const CRect& rect);

	virtual void InitViewToolTipInfo ();

	virtual CString GetToolTipText (const CPoint& point);

	virtual BOOL EnsureVisible(const CBCGPAppointment* /*pApp*/, BOOL /*bPartialOK*/)
	{
		return FALSE;
	}

	virtual BOOL CanUseHeaderScrolling() const
	{
		return FALSE;
	}
	
	virtual CString GetAccName() const;
	virtual CString GetAccValue() const;
	virtual CString GetAccDescription() const;

protected:
	CRect				m_rectApps;
	int					m_nRowHeight;

	int					m_nHeaderScrollOffset;
	int					m_nHeaderScrollTotal;
	int					m_nHeaderScrollPage;

	int					m_nScrollOffset;
	int					m_nScrollTotal;
	int					m_nScrollPage;

	COleDateTime		m_Date;			// Planner base date
	COleDateTime		m_DateStart;	// Planner start date
	COleDateTime		m_DateEnd;		// Planner end date
	COleDateTime		m_Selection [2];

	CArray<CRect, CRect&>
						m_ViewRects;
	CArray<CRect, CRect&>
						m_UpRects;
	CArray<CRect, CRect&>
						m_DownRects;

	COleDateTime		m_dtCaptureStart;
	COleDateTime		m_dtCaptureCurrent;

	BCGP_PLANNER_ADJUST_ACTION	m_AdjustAction;

	CBCGPPlannerManagerCtrl*	m_pPlanner;

	CString				m_strCaptionFormat;

	CFont				m_Font;
	CFont				m_FontBold;
	CFont				m_FontVert;

	bool				m_bActive;

	CBCGPAppointmentBaseStorage*
						m_pDragAppointments;
	XBCGPAppointmentArray
						m_arDragAppointments;

	UINT_PTR			m_TimerEdit;

	HCURSOR				m_hcurAppHorz;
	HCURSOR				m_hcurAppVert;

	// need only for appointment resizing
	CBCGPAppointment*	m_pCapturedAppointment;
	BCGP_PLANNER_HITTEST m_hitCapturedAppointment;
	COleDateTime		m_dtCapturedAppointment;
	BOOL				m_bCapturedAppointmentChanged;
	CBCGPAppointmentPropertyList
						m_CapturedProperties;

	BCGP_PLANNER_HITTEST m_htCaptureAreaStart;
	BCGP_PLANNER_HITTEST m_htCaptureAreaCurrent;
	UINT                 m_htCaptureResourceStart;
	UINT                 m_htCaptureResourceCurrent;
	
	BOOL				m_bUpdateToolTipInfo;

	BOOL IsTimerEditStarted () const
	{
		return m_TimerEdit != NULL;
	}
	void StartTimerEdit ();
	void StopTimerEdit ();
};

#endif // BCGP_EXCLUDE_PLANNER

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPLANNERVIEW_H__9A8EBBB8_3FCC_4D8A_BE23_C88D8D1E1A08__INCLUDED_)
