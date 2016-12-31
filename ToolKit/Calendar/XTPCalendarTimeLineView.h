// XTPCalendarTimeLineView.h: interface for the CXTPCalendarTimeLineView class.
//
// This file is a part of the XTREME CALENDAR MFC class library.
// (c)1998-2011 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME TOOLKIT PRO LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////

//{{AFX_CODEJOCK_PRIVATE
#if !defined(_XTPCALENDARTIMELINEVIEW_H__)
#define _XTPCALENDARTIMELINEVIEW_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (disable: 4100)

#include "Common/XTPDrawHelpers.h"

#include "XTPCalendarView.h"
#include "XTPCalendarViewEvent.h"
#include "XTPCalendarTimeLineViewTimeScale.h"
#include "XTPCalendarTimeLineViewGroup.h"

class CXTPCalendarTimeLineView;

//============================================================================
// Summary:
//     predefine set of different scales based on time intervals
//============================================================================
enum XTPEnumCalendarTimeScaleParamsID
{
	xtpTSPID_Unknown    = 0,
	xtpTSPID_Day        = 1, //Day scale
	xtpTSPID_Week       = 2, //Week scale
	xtpTSPID_Month      = 3, //Month scale
	xtpTSPID_WorkWeek   = 4, //WorkWeek scale
};

//===========================================================================
// Summary: Set of parameters transferred from calendar items to
//          mouse handlers.
// Remarks: This structure groups together all essential parameters used in
//          items processing activities. It is a basic structure for all
//          particular calendar's structures.
// See also: XTP_CALENDAR_HITTESTINFO
//===========================================================================
struct XTP_CALENDAR_HITTESTINFO_TIMELINE_VIEW : public XTP_CALENDAR_HITTESTINFO
{
	POINT pt;   // Point where the mouse was clicked in client coordinates.
	COleDateTime dt; // datetime object to fill and return

	//-----------------------------------------------------------------------
	// Summary:
	//     Default constructor.
	// Remarks:
	//     Initializes a XTP_CALENDAR_HITTESTINFO_TIMELINE_VIEW object to the
	//     empty values.
	//-----------------------------------------------------------------------
	XTP_CALENDAR_HITTESTINFO_TIMELINE_VIEW()
	{
		pt.x = pt.y = -1;
		dt = COleDateTime::GetCurrentTime();
	}
};

//===========================================================================
// Summary:
//     This class implements a specific view portion of the Calendar
//     control - called Timeline view.
//===========================================================================
class _XTP_EXT_CLASS CXTPCalendarTimeLineView : public CXTPCalendarView
{
	friend class CXTPCalendarControl;
	friend class CXTPDatePickerControl;

	DECLARE_DYNAMIC(CXTPCalendarTimeLineView)

public:
	//------------------------------------------------------------------------
	// Remarks:
	//     Base class type definition
	//------------------------------------------------------------------------
	typedef CXTPCalendarView TBase;

	//-----------------------------------------------------------------------
	// Summary:
	//     Default object constructor.
	// Parameters:
	//     pCalendarControl - Pointer to CXTPCalendarControl object.
	//-----------------------------------------------------------------------
	CXTPCalendarTimeLineView(CXTPCalendarControl* pCalendarControl);

	//-----------------------------------------------------------------------
	// Summary:
	//     Default class destructor.
	// Remarks:
	//     Handles class members deallocation.
	//-----------------------------------------------------------------------
	virtual ~CXTPCalendarTimeLineView();

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this member function to obtain the view type flag.
	// Returns:
	//     An XTPCalendarViewType object that contains the view type flag.
	// See Also: XTPCalendarViewType
	//-----------------------------------------------------------------------
	virtual XTPCalendarViewType GetViewType() const;

	// -----------------
	// Summary:
	//     This member function is used to populate the view with data
	//     for all items contained in the view.
	// -----------------
	virtual void Populate();

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function is used to adjust the view's layout
	//     depending on the window's client dimensions and calls.
	// Parameters:
	//     pDC      - Pointer to a valid device context.
	//     rcView   - A CRect that contains the bounding rectangle dimensions
	//                of the view.
	//     bCallPostAdjustLayout - TRUE for calling additional post-adjustments
	// Remarks:
	//     Call Populate() prior adjusting layout. Adjust layout of all sub-items.
	//-----------------------------------------------------------------------
	virtual void AdjustLayout(CDC* pDC, const CRect& rcView, BOOL bCallPostAdjustLayout = TRUE);

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function performs all of the drawing logic of the
	//     calendar view.
	// Parameters:
	//     pDC - Pointer to a valid device context.
	//-----------------------------------------------------------------------
	virtual void Draw(CDC* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function performs all of the drawing logic of the
	//     calendar view.
	//     Draw2 is called by calendar control instead of Draw when theme is set.
	// Parameters:
	//     pDC - Pointer to a valid device context.
	//-----------------------------------------------------------------------
	virtual void Draw2(CDC* pDC);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this member function to obtain the view type flag.
	// Returns:
	//     An XTPCalendarViewType object that contains the view type flag.
	// See Also: XTPCalendarViewType
	//-----------------------------------------------------------------------
	virtual XTPCalendarViewType GetViewType()
	{
		return xtpCalendarTimeLineView;
	}

	// -----------------------
	// Summary:
	//  This member function do nothing in Timeline case but we need to have it due
	//  int CXTPCalendarView::GetViewDayCount(void) is abstract
	// Returns:
	//     number of days in view
	// -----------------------
	virtual int GetViewDayCount()
	{
		return 0;
	}

	// ---------------------------
	// Summary:
	//  This member function do nothing in Timeline case but we need to have it due
	//  int CXTPCalendarView::GetViewDay_(int) is abstract
	// Parameters:
	//     nIndex :  index of day
	// Returns:
	//     pointer to CXTPCalendarViewDay
	// ---------------------------
	virtual CXTPCalendarViewDay* GetViewDay_(int nIndex)
	{
		return 0;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this member function to obtain the date of the day view by
	//     the day's index.
	// Parameters:
	//     nIndex  - An int that contains the day view index in the view collection.
	// Remarks:
	//     Indexed number starts from 0 and cannot be negative.
	// Returns:
	//     A COleDateTime object that contains the day view date and time.
	//-----------------------------------------------------------------------
	virtual COleDateTime GetViewDayDate(int nIndex)
	{
		return CXTPCalendarUtils::ResetTime(GetStartViewDate());
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this member function to obtain the start view date
	// Returns:
	//     A COleDateTime object that contains the day view date and time.
	//-----------------------------------------------------------------------
	virtual COleDateTime GetStartViewDate();

	// ---------------------------
	// Summary:
	//     Call this member function to set the start view date
	// Parameters:
	//     dt :  start view date as COleDateTime
	// ---------------------------
	virtual void SetStartViewDate(COleDateTime dt);

	// ---------------------------
	// Summary:
	//     Call this member function to get the current time scale ID
	// Returns:
	//     int as scale ID
	// ---------------------------
	virtual int GetTimeScaleID();

	// ---------------------------
	// Summary:
	//     Call this member function to set the current time scale
	// Parameters:
	//     eTimeScaleParamsID :  as int or XTPEnumCalendarTimeScaleParamsID
	// ---------------------------
	virtual void SetTimeScaleID(int eTimeScaleParamsID);

	// -----------------------------
	// Summary:
	//     Call this member function to set the current time scale as object
	// Returns:
	//     pointer to CXTPCalendarTimeLineViewTimeScale
	// -----------------------------
	virtual CXTPCalendarTimeLineViewTimeScale* GetTimeScale();

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function is used to determine which view item,
	//     if any, is at a specified position index, and returns
	//     additional info in a XTP_CALENDAR_HITTESTINFO structure.
	// Parameters:
	//     pt       - Coordinates of point to test.
	//     pHitTest - Pointer to a XTP_CALENDAR_HITTESTINFO structure.
	// Returns:
	//     TRUE if item found. FALSE otherwise.
	//-----------------------------------------------------------------------
	virtual BOOL HitTest(CPoint pt, XTP_CALENDAR_HITTESTINFO* pHitTest);

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function is used to determine which view item,
	//     if any, is at a specified position index, and returns
	//     additional info in a XTP_CALENDAR_HITTESTINFO_TIMELINE_VIEW structure.
	// Parameters:
	//     pt       - Coordinates of point to test.
	//     pHitTest - Pointer to a XTP_CALENDAR_HITTESTINFO_TIMELINE_VIEW structure.
	// Returns:
	//     TRUE if item found. FALSE otherwise.
	// See Also: XTP_CALENDAR_HITTESTINFO_TIMELINE_VIEW
	//-----------------------------------------------------------------------
	virtual BOOL HitTestEx(CPoint pt, XTP_CALENDAR_HITTESTINFO_TIMELINE_VIEW* pHitTest)
	{
		return 0;
	}

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function is used to retrieve information about a
	//     scroll bar's state.
	// Parameters:
	//     pSI          - A pointer to scrollbar information structure.
	// Returns: A BOOL.
	//          TRUE if Vertical Scrollbar enabled
	//          FALSE otherwise.
	//-----------------------------------------------------------------------
	virtual BOOL GetScrollBarInfoV(SCROLLINFO* pSI);

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function is used to retrieve information about a
	//     scroll bar's state.
	// Parameters:
	//     pSI          - A pointer to scrollbar information structure.
	//     pnScrollStep - A pointer to int to receive scroll step. Can be NULL.
	// Returns: A BOOL.
	//          TRUE if the Horizontal Scrollbar enabled
	//          FALSE otherwise.
	//-----------------------------------------------------------------------
	virtual BOOL GetScrollBarInfoH(SCROLLINFO* pSI, int* pnScrollStep = NULL);

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function is used to scroll the content of a view.
	// Parameters:
	//     nPos     - Position value.
	//     nPos_raw - Raw position value.
	//-----------------------------------------------------------------------
	virtual void ScrollH(int nPos, int nPos_raw);

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function is used to scroll the content of a view.
	// Parameters:
	//     nPos     - Position value.
	//     nPos_raw - Raw position value.
	//-----------------------------------------------------------------------
	virtual void ScrollV(int nPos, int nPos_raw);

	// ----------------------------
	// Summary:
	//     This member function scroll timeline to passed date
	// Parameters:
	//     date :     datetime moment to ensure visible
	//     bSelect :  flag to select or not
	// ----------------------------
	virtual void ShowDay(const COleDateTime& date, BOOL bSelect = TRUE)
	{
		SetStartViewDate(date);
	}

	// -----------------------------
	// Summary:
	//     This member function popup DatePicker control
	// Parameters:
	//     dt :     initial popup datetime selection
	//     point :  point to show popup
	// -----------------------------
	void ShowCalendarPopup(COleDateTime dt, CPoint point);

protected:

	//-----------------------------------------------------------------------
	// Summary:
	//     This member function is called by the framework before the
	//     view is destroyed.
	//-----------------------------------------------------------------------
	virtual void OnBeforeDestroy();

	// -------------------------
	// Summary:
	//     This member function is used to force scroll to the end of scroll range
	// Parameters:
	//     nBar :  standard 0 - 1 values (for vertical or horizontal scrollbar)
	// -------------------------
	virtual void OnScrollEnd(int nBar);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this member function to obtain the bounding rectangle of
	//     the tooltip space
	//     Tooltip rectangle is calculated basing on the current mouse position
	// Parameters:
	//      ptHit - CPoint& of mouse position
	//      hitInfo - XTP_CALENDAR_HITTESTINFO
	// Returns:
	//     A CRect object with relative coordinates of the tooltip.
	// Remarks:
	//     This member function returns a rectangle calculated as a event
	//     rect which user currently holds a mouse on.
	//-----------------------------------------------------------------------
	CRect GetTooltipRect(const CPoint& ptHit, const XTP_CALENDAR_HITTESTINFO& hitInfo);

	// -------------------------
	// Summary:
	//     Call this member function to get all events in given range
	// Parameters:
	//     dtStartDay :  start of range
	//     dtEndDay :    end of range
	// Returns:
	//     CXTPCalendarEventsPtr object
	// -------------------------
	virtual CXTPCalendarEventsPtr _GetEvents(COleDateTime dtStartDay, COleDateTime dtEndDay);

	CXTPCalendarTimeLineViewTimeScale*  m_pTimeScale; // pointer to attached Time Scale object

	int m_nHScrollPos; // keep HScroll position
	int m_nVScrollPos; // keep VScroll position

private:
	CRect m_rcLastPopulate;// last populated view rectangle - used for refresh by demand

public:
	CXTPCalendarTimeLineViewGroups m_arGroups; // array of events organized in groups

	BOOL m_bTimeLineSeparateGroup; // flag to use separatore line between groups or not
	int m_nTimelineMaxPixelsForEvent_ScaleDay; // width in pixels for 1 event for day scale
	int m_nTimelineMaxPixelsForEvent_ScaleWeek; //  width in pixels for 1 event for week scale
	int m_nTimelineMaxPixelsForEvent; //  width in pixels for 1 event for month scale

	COLORREF m_clrEventBar; // Event Bar Color
	COLORREF m_clrEventBarLine; // Event Bar Line Color

	COLORREF m_clrTimeScaleBackground; // Time Scale Background Color
	COLORREF m_clrTimeScaleBorder; // Time Scale Border Color
	COLORREF m_clrTimeScaleHighlight; // Time Scale Highlight Color
	COLORREF m_clrTimeScaleText; // Time Scale Text Color

	COLORREF m_clrSelectedBackground; // Selected Background Color
	COLORREF m_clrSelectedText; // Selected Text Color

	COLORREF m_clrBackground; // Background Color
	COLORREF m_clrText; // Text Color


};

//===========================================================================
AFX_INLINE XTPCalendarViewType CXTPCalendarTimeLineView::GetViewType() const
{
	return xtpCalendarTimeLineView;
}

AFX_INLINE COleDateTime CXTPCalendarTimeLineView::GetStartViewDate()
{
	ASSERT(m_pTimeScale);
	return m_pTimeScale ? m_pTimeScale->GetStartViewDate() : COleDateTime::GetCurrentTime();
}

AFX_INLINE void CXTPCalendarTimeLineView::SetStartViewDate(COleDateTime dt)
{
	ASSERT(m_pTimeScale);
	if (m_pTimeScale)
		m_pTimeScale->SetStartViewDate(dt);

	Populate();
}

AFX_INLINE CXTPCalendarTimeLineViewTimeScale* CXTPCalendarTimeLineView::GetTimeScale()
{
	return m_pTimeScale;
}

#endif // !defined(_XTPCALENDARTIMELINEVIEW_H__)
