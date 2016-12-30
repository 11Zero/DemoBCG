//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPPlannerManagerCtrl.h : header file
//

#if !defined(AFX_BCGPPLANNERMANAGERCTRL_H__54C9C402_F5A3_4B0F_A71D_89E6AA0114D9__INCLUDED_)
#define AFX_BCGPPLANNERMANAGERCTRL_H__54C9C402_F5A3_4B0F_A71D_89E6AA0114D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPWnd.h"
#include "BCGPPlannerClockIcons.h"
#include "BCGPPlannerPrint.h"

#include "BCGPPlannerView.h"

#include "BCGPPlannerDropTarget.h"
#include "BCGPPlannerDropSource.h"

#include "BCGPRecurrenceRule.h"

#include "BCGPScrollBar.h"

class CBCGPCalendar;
class CBCGPAppointmentBaseStorage;
class CBCGPAppointmentBaseResourceInfo;

// wParam - not used, lParam - points to the appointment
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_ADD_APPOINTMENT;
// wParam - not used, lParam - points to the appointment
// Return value:
// -1L - cancel update
//  0L - update
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_BEFORE_UPDATE_APPOINTMENT;
// wParam - not used, lParam - points to the appointment
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_UPDATE_APPOINTMENT;
// wParam - not used, lParam - not used
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_UPDATED_ALL_APPOINTMENTS;
// wParam - not used, lParam - points to the appointment
// Return value:
// -1L - cancel delete
//  0L - delete single appointment or only occurrence of the series
//  1L - delete full series
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_REMOVE_APPOINTMENT;
// wParam, lParam - not used
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_REMOVE_ALL_APPOINTMENTS;

// wParam - TRUE - select, otherwise - unselect, lParam - points to the appointment
// Return value:
// -1L - cancel operation
//  0L - allow operation
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_BEFORE_SELECT_APPOINTMENT;
// wParam - TRUE - selected, otherwise - not selected, lParam - points to the appointment
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_SELECT_APPOINTMENT;

// wParam - old type, lParam - new type
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_TYPE_CHANGED;
// wParam, lParam - not used
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_DATE_CHANGED;
// wParam, lParam - not used
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_RESOURCEID_CHANGED;

// wParam, lParam - see WM_LBUTTONDBLCLK
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_LBUTTONDBLCLK;
// wParam, lParam - see WM_KEYDOWN
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_KEYDOWN;

// click on "up-down" icon
// wParam - hit test code of icon, see CBCGPPlannerView
// Return value - return FALSE if default handler is needed
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_ICONUPDOWN_CLICK;

// click on day caption
// wParam, lParam - not used
// Return value - return FALSE if default handler is needed
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_DAYCAPTION_CLICK;

// click on week caption
// wParam, lParam - not used
// Return value - return FALSE if default handler is needed
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_WEEKCAPTION_CLICK;

// wParam, lParam - not used
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_DROP_APPOINTMENTS;
// wParam - drop effect code, lParam - not used
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_DRAG_APPOINTMENTS;

// wParam - operation code, lParam - not used
// Return value:
// -1L - cancel operation
//  0L - allow operation
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_BEGIN_CHANGE_OPERATION;
// wParam - operation code, lParam - result of operation
extern BCGCBPRODLLEXPORT UINT BCGP_PLANNER_END_CHANGE_OPERATION;

#define BCGP_PLANNER_RULE_DAILY   1
#define BCGP_PLANNER_RULE_WEEKLY  2
#define BCGP_PLANNER_RULE_MONTHLY 3
#define BCGP_PLANNER_RULE_YEARLY  4

#define BCGP_PLANNER_DRAW_APP_GRADIENT_FILL			0x0001
#define BCGP_PLANNER_DRAW_APP_ROUNDED_CORNERS		0x0002
#define BCGP_PLANNER_DRAW_APP_OVERRIDE_SELECTION	0x0004
#define BCGP_PLANNER_DRAW_APP_NO_MULTIDAY_CLOCKS	0x0008
#define BCGP_PLANNER_DRAW_APP_DURATION_SHAPE		0x0010
#define BCGP_PLANNER_DRAW_APP_NO_DURATION			0x0020
#define BCGP_PLANNER_DRAW_APP_NO_IMAGES				0x0040
#define BCGP_PLANNER_DRAW_APP_NO_RECURRENCE_IMAGE	0x0080
#define BCGP_PLANNER_DRAW_VIEW_NO_DURATION			0x0100
#define BCGP_PLANNER_DRAW_VIEW_WEEK_BAR				0x0200
#define BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_BOLD		0x0400
#define BCGP_PLANNER_DRAW_VIEW_CAPTION_DAY_COMPACT	0x0800
#define BCGP_PLANNER_DRAW_VIEW_NO_UPDOWN			0x1000
#define BCGP_PLANNER_DRAW_VIEW_DAYS_UPDOWN			0x2000
#define BCGP_PLANNER_DRAW_VIEW_DAYS_UPDOWN_VCENTER	0x4000

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerManagerCtrl window

class BCGCBPRODLLEXPORT CBCGPPlannerManagerCtrl : public CBCGPWnd
{
	DECLARE_DYNCREATE(CBCGPPlannerManagerCtrl)

	friend class CBCGPPlannerDropTarget;
	friend class CBCGPPlannerView;
	friend class CBCGPPlannerXCtrl;

// Construction
public:
	CBCGPPlannerManagerCtrl(CRuntimeClass* pStorageClass = NULL, 
							CRuntimeClass* pClockIconsClass = NULL);
	virtual ~CBCGPPlannerManagerCtrl();
	
	void SetStorageRTC (CRuntimeClass* pStorageClass = NULL, BOOL bDelete = TRUE);
	void SetClockIconsRTC (CRuntimeClass* pClockIconsClass = NULL, BOOL bDelete = TRUE);

// Attributes
public:
	enum
	{
		BCGP_PLANNER_ID_SCROLL = 1
	};

	enum
	{
		BCGP_PLANNER_ID_INPLACE = 2
	};

	enum
	{
		BCGP_PLANNER_ID_SCROLL_HEADER = 3
	};

	enum BCGP_PLANNER_TYPE
	{
		BCGP_PLANNER_TYPE_FIRST     = 0,
		BCGP_PLANNER_TYPE_DAY       = BCGP_PLANNER_TYPE_FIRST,
		BCGP_PLANNER_TYPE_WORK_WEEK = 1,
		BCGP_PLANNER_TYPE_WEEK      = 2,
		BCGP_PLANNER_TYPE_MONTH     = 3,
		BCGP_PLANNER_TYPE_MULTI     = 4,
		BCGP_PLANNER_TYPE_LAST      = BCGP_PLANNER_TYPE_MULTI
	};

	enum BCGP_PLANNER_CHANGE_OPERATION
	{
		BCGP_PLANNER_CHANGE_OPERATION_FIRST = 0,
		BCGP_PLANNER_CHANGE_OPERATION_NONE  = BCGP_PLANNER_CHANGE_OPERATION_FIRST,
		BCGP_PLANNER_CHANGE_OPERATION_CUT   = 1,
		BCGP_PLANNER_CHANGE_OPERATION_PASTE = 2,
		BCGP_PLANNER_CHANGE_OPERATION_DRAG  = 3,
		BCGP_PLANNER_CHANGE_OPERATION_DROP  = 4,
		BCGP_PLANNER_CHANGE_OPERATION_LAST  = BCGP_PLANNER_CHANGE_OPERATION_DROP
	};

	void SetDate (const COleDateTime& date, BOOL bRedraw = TRUE);
	void SetDateInterval (const COleDateTime& date1, const COleDateTime& date2, BOOL bRedraw = TRUE);
	void SetToday (BOOL bRedraw = TRUE);

	void SetType (BCGP_PLANNER_TYPE	type, BOOL bRedraw = TRUE);

	BCGP_PLANNER_TYPE GetType () const
	{
		return m_Type;
	}

	BCGP_PLANNER_CHANGE_OPERATION GetChangeOperation() const
	{
		return m_ChangeOperation;
	}

	void SetTimeDelta (CBCGPPlannerView::BCGP_PLANNER_TIME_DELTA delta, 
					BOOL bRedraw = TRUE);

	CBCGPPlannerView::BCGP_PLANNER_TIME_DELTA GetTimeDelta () const
	{
		return m_TimeDelta;
	}

	void SetFirstDayOfWeek (int nDay);	// 0-6, where 0=Sunday, 1=Monday, and so on.
	static int GetFirstDayOfWeek ();

	static int GetSystemFirstDayOfWeek (BOOL bConvertDayOfWeek = FALSE); // 0-6. If bConvertDayOfWeek=TRUE then 0=Sunday, otherwise 0=Monday

	static CString GetSystemWeekDayName (int nDay);

	void SetWorkingHourInterval (int nFirstHour, int nLastHour, BOOL bRedraw = TRUE);
	void SetWorkingHourMinuteInterval (double dStart, double dEnd, BOOL bRedraw = TRUE);
	
	int GetFirstWorkingHour (BOOL bDefault = TRUE) const;
	int GetFirstWorkingMinute (BOOL bDefault = TRUE) const;

	int GetLastWorkingHour (BOOL bDefault = TRUE) const;
	int GetLastWorkingMinute (BOOL bDefault = TRUE) const;
	
	void SetViewHourInterval (int nFirstHour, int nLastHour, BOOL bRedraw = TRUE);
	int GetFirstViewHour () const
	{
		return (int)m_nFirstViewHour;
	}
	int GetLastViewHour () const
	{
		return (int)m_nLastViewHour;
	}

	void SetPrintHourInterval (int nFirstHour, int nLastHour);
	int GetFirstPrintHour () const
	{
		return (int)m_nFirstPrintHour;
	}
	int GetLastPrintHour () const
	{
		return (int)m_nLastPrintHour;
	}

	void SetWorkWeekInterval(int nViewWorkInterval);
	int GetWorkWeekInterval() const
	{
		return m_nWorkWeekInterval;
	}

	BOOL IsDateInSelection (const COleDateTime& date) const;

	void SetSelection (const COleDateTime& sel1, const COleDateTime& sel2, BOOL bRedraw = TRUE);
	
	COleDateTime GetSelectionStart () const;
	COleDateTime GetSelectionEnd () const;

	// appointments
	BOOL AddAppointment (CBCGPAppointment* pApp, BOOL bQuery = TRUE, BOOL bRedraw = TRUE);
	BOOL UpdateAppointment (CBCGPAppointment* pApp, const COleDateTime& dtOld, BOOL bQuery = TRUE, BOOL bRedraw = TRUE);
	BOOL RemoveAppointment (CBCGPAppointment* pApp, BOOL bQuery = TRUE, BOOL bRedraw = TRUE);

	void RemoveSelectedAppointments (BOOL bQuery = TRUE, BOOL bRedraw = TRUE);
	BOOL RemoveAllAppointments (BOOL bRedraw = TRUE);

	void SelectAppointment (CBCGPAppointment* pApp, BOOL bSelect, BOOL bRedraw = TRUE);

	BOOL IsAppointmentInSelection (const CBCGPAppointment* pApp) const;

	void ClearAppointmentSelection (BOOL bRedraw = TRUE);

	int GetSelectedAppointmentsCount () const;

	POSITION GetFirstSelectedAppointment () const;
	POSITION GetLastSelectedAppointment () const;
	const CBCGPAppointment* GetNextSelectedAppointment (POSITION& pos) const;
	const CBCGPAppointment* GetPrevSelectedAppointment (POSITION& pos) const;

	int GetQueryedAppointmentsCount () const;
	int GetQueryedAppointments (XBCGPAppointmentArray& ar) const;

	// based on type of view
	void SetDrawTimeFinish (BOOL bDraw);
	BOOL IsDrawTimeFinish () const;

	void SetDrawTimeAsIcons (BOOL bDraw);
	BOOL IsDrawTimeAsIcons () const;

	void SetCompressWeekend (BOOL bCompress);
	BOOL IsCompressWeekend () const;

	void SetBackgroundColor (COLORREF clr, BOOL bRedraw = TRUE);
	COLORREF GetBackgroundColor () const
	{
		return m_clrBackground;
	}

	void SetNotifyParent (BOOL bNotifyParent);
	BOOL IsNotifyParent () const;

	void SetCalendar (CBCGPCalendar* pWndCalendar);
	CBCGPCalendar* GetCalendar () const
	{
		return m_pWndCalendar;
	}

	BOOL IsClipboardFormatAvailable (COleDataObject* pDataObject);
	
	CBCGPAppointment* GetRecurrence (DWORD ID) const;

	void SetReadOnly (BOOL bReadOnly = TRUE);
	BOOL IsReadOnly () const
	{
		return m_bReadOnly;
	}

	void SetDrawFlags (DWORD dwFlags, BOOL bRedraw = TRUE);
	DWORD GetDrawFlags () const;

	void SetUseDayViewInsteadWeekView (BOOL bUse);
	BOOL IsUseDayViewInsteadWeekView () const
	{
		return m_bUseDayViewInsteadWeekView;
	}

// Operations
public:
	int GetViewDuration () const;	// Days

	const COleDateTime& GetDate () const;

	const COleDateTime& GetDateStart () const;

	const COleDateTime& GetDateEnd () const;

	virtual BOOL EditCut ();

	virtual BOOL IsEditCutEnabled () const;

	virtual BOOL EditCopy ();

	virtual BOOL IsEditCopyEnabled () const;

	virtual BOOL EditPaste (const COleDateTime& dtTo = COleDateTime ());

	virtual BOOL IsEditPasteEnabled () const;

	static CSize GetImageSize ()
	{
		InitImages ();
		return s_ImageSize;
	}

	static CImageList& GetImageList ()
	{
		InitImages ();
		return s_ImageList;
	}

	static void DrawImageIcon (CDC* pDC, const CPoint& pt, int nIndex);

	static void SetImages (UINT nResID, int cxImage, COLORREF clrTransparent = RGB (255, 0, 255));
	static void SetImages (LPCTSTR szResID, int cxImage, COLORREF clrTransparent = RGB (255, 0, 255));

	static DWORD RegisterRule (CRuntimeClass* pRuleClass);
	static CBCGPRecurrenceBaseRule* CreateRule (DWORD ID);
	static void GetRulesID (CBCGPRecurrenceRuleRegistrator::XBCGPRecurrenceRuleIDArray& arID);

	CBCGPAppointmentBaseStorage* GetStorage () const
	{
		return m_pAppsStorage;
	}

	void UpdateStorage (BOOL bRedraw = TRUE);

	BOOL IsMultiResourceStorage () const
	{
		return m_bMultiResourceStorage;
	}

	BOOL AddResourceRTC (UINT nResourceID, CRuntimeClass* pStorageClass, 
		CBCGPAppointmentBaseResourceInfo* pInfo, BOOL bAutoDelete = TRUE, BOOL bRedraw = TRUE);
	BOOL AddResource (UINT nResourceID, CBCGPAppointmentBaseStorage* pStorage, 
		CBCGPAppointmentBaseResourceInfo* pInfo, BOOL bAutoDelete = TRUE, BOOL bRedraw = TRUE);
	BOOL RemoveResource (UINT nResourceID, BOOL bRedraw = TRUE);

	const CBCGPAppointmentBaseResourceInfo* GetResourceInfo (UINT nResourceID) const;

	UINT GetCurrentResourceID () const;
	BOOL SetCurrentResourceID (UINT nResourceID, BOOL bRedraw = TRUE);

	void SetUseMultiResourceDefault (BOOL bUseMultiResourceDefault);

	void SerializeRaw (CArchive& ar);

	void UpdateCalendars (CBCGPCalendar* pWndCalendar = NULL);
	void UpdateCalendarsState (CBCGPCalendar* pWndCalendar = NULL);
	void UpdateCalendarsSelection (CBCGPCalendar* pWndCalendar = NULL);

	BOOL IsShowToolTip () const
	{
		return m_bShowToolTip;
	}
	void SetShowToolTip (BOOL bShowToolTip);
	BOOL IsToolTipShowAlways () const
	{
		return m_bToolTipShowAlways;
	}
	void SetToolTipShowAlways (BOOL bToolTipShowAlways);
	
	const COleDateTime& GetDroppedDate () const
	{
		return m_dtDrop;
	}

	void SetScrollBarsStyle (CBCGPScrollBar::BCGPSB_STYLE style)
	{
		ASSERT (CBCGPScrollBar::BCGP_SBSTYLE_FIRST <= style && style <= CBCGPScrollBar::BCGP_SBSTYLE_LAST);
		m_wndScrollVert.SetVisualStyle (style);
		m_wndHeaderScrollVert.SetVisualStyle (style);
	}

	CBCGPScrollBar::BCGPSB_STYLE GetScrollBarsStyle () const
	{
		return m_wndScrollVert.GetVisualStyle ();
	}

	void SetScrollBarVisible (BOOL bVisible);
	BOOL IsScrollBarVisible () const
	{
		return m_bScrollVisible;
	}

	int GetFirstSelectionHour () const;
	int GetFirstSelectionMinute () const;

	int GetLastSelectionHour () const;
	int GetLastSelectionMinute () const;

	void EnableHeaderScrolling (BOOL bEnable);
	BOOL IsHeaderScrollingEnabled () const
	{
		return m_bHeaderScrollingEnabled;
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPPlannerManagerCtrl)
public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
	virtual CScrollBar* GetHeaderScrollBarCtrl(int nBar) const;

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL PreCreateWindow (CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:
	virtual CBCGPPlannerView* OnCreateView(BCGP_PLANNER_TYPE type);
	virtual CBCGPPlannerView::BCGP_PLANNER_WORKING_STATUS GetWorkingPeriodParameters (int ResourceId, const COleDateTime& dtStart, const COleDateTime& dtEnd, CBCGPPlannerView::XBCGPPlannerWorkingParameters& parameters) const;

public:
	virtual void AdjustLayout (BOOL bRedraw = TRUE);
	virtual void QueryAppointments ();
	virtual void ClearQuery ();
	
	virtual CBCGPPlannerView::BCGP_PLANNER_HITTEST HitTest (const CPoint& point) const;
	virtual CBCGPPlannerView::BCGP_PLANNER_HITTEST HitTestArea (const CPoint& point) const;
	virtual CBCGPPlannerView::BCGP_PLANNER_HITTEST HitTestAppointment (const CPoint& point) const;

	BOOL EnsureVisible(const CBCGPAppointment* pApp, BOOL bPartialOK);

	COleDateTime GetDateFromPoint (const CPoint& point) const;

	CBCGPAppointment* GetAppointmentFromPoint (const CPoint& point);

	void DrawClockIcon(CDC* pDC, const CPoint& point, const COleDateTime& time) const
	{
		if (m_pClockIcons != NULL)
		{
			m_pClockIcons->DrawIcon (pDC, point, time);
		}
	}

	CSize GetClockIconSize () const
	{
		if (m_pClockIcons != NULL)
		{
			return m_pClockIcons->GetIconSize ();
		}

		return CSize (0, 0);
	}

	BOOL SetUpDownIcons(UINT nResID, COLORREF clrTransparent = RGB (255, 0, 255));
	BOOL SetUpDownIcons(LPCTSTR szResID, COLORREF clrTransparent = RGB (255, 0, 255));

	// 0 - up; 1 - down
	HICON GetUpDownIcon(int nType);

	const CSize& GetUpDownIconSize () const
	{
		return m_szUpDown;
	}

	virtual void DoPaint(CDC* pDCPaint);
	virtual void OnPrint (CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPreparePrinting(CPrintInfo* pInfo, CRuntimeClass* pPrinterClass = NULL);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo, CRuntimeClass* pPrinterClass = NULL);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	virtual void QueryAppointments (XBCGPAppointmentArray& ar, 
		const COleDateTime& date1, const COleDateTime& date2) const;

	virtual void OnSelectTimeInterval (COleDateTime dateStart, COleDateTime dateFinish);
	virtual BOOL OnDropAppointmentToCalendar (COleDataObject* pDataObject,
									DROPEFFECT dropEffect, COleDateTime dateTo);

	const CBCGPPlannerView* GetCurrentView () const
	{
		ASSERT_VALID(m_pCurrentView);
		return m_pCurrentView;
	}

	// Printing without the Document/View framework
	void Print ();

	// clipboard
	static CLIPFORMAT GetClipboardFormat ();
	static LPCTSTR GetClipboardFormatName ();

	// serializing
	static BOOL SerializeTo (CFile& file, XBCGPAppointmentArray& ar);
	static BOOL SerializeTo (CArchive& Archive, XBCGPAppointmentArray& ar);

	static BOOL SerializeFrom (CFile& file, XBCGPAppointmentArray& ar, 
		BCGP_PLANNER_TYPE type, const COleDateTime& dtTo);
	static BOOL SerializeFrom (CArchive& Archive, XBCGPAppointmentArray& ar, 
		BCGP_PLANNER_TYPE type, const COleDateTime& dtTo);
	static void MoveAppointments (XBCGPAppointmentArray& ar, const COleDateTimeSpan& spanTo, BOOL bAdd);

	CWnd* GetNotifyWnd () const
	{
		return m_pNotifyWnd == NULL ? GetParent () : m_pNotifyWnd;
	}
	void SetNotifyWnd(CWnd* pWnd)
	{
		m_pNotifyWnd = pWnd;
	}

protected:

	virtual void OnDateChanged ();

	XBCGPAppointmentArray& GetQueryedAppointments ()
	{
		return m_arQueryApps;
	}

	const XBCGPAppointmentArray& GetQueryedAppointments () const
	{
		return m_arQueryApps;
	}

	XBCGPAppointmentList& GetSelectedAppointments ()
	{
		return m_lsSelectedApps;
	}

	const XBCGPAppointmentList& GetSelectedAppointments () const
	{
		return m_lsSelectedApps;
	}

	void RestoreAppointmentSelection (XBCGPAppointmentList& lst, BOOL bRedraw);

	static void InitImages ();

	// Generated message map functions
protected:

	// clipboard
	static CLIPFORMAT s_ClpFormat;
	static CString s_ClpFormatName;
	
	// serializing
	BOOL SerializeTo (CFile& file);
	BOOL SerializeFrom (CFile& file, const COleDateTime& dtTo);

	virtual void GetDragScrollRect (CRect& rect);

	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll);

	virtual BOOL OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);

	virtual DROPEFFECT OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);

	virtual void OnDragLeave();

	virtual DROPEFFECT OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);

	virtual DROPEFFECT OnDragScroll(DWORD dwKeyState, CPoint point);

	virtual void OnDraw (CDC* pDC);
	
	//{{AFX_MSG(CBCGPPlannerManagerCtrl)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnTimeChange();
	afx_msg void OnCancelMode();
	//}}AFX_MSG
	afx_msg LRESULT OnSetFont (WPARAM, LPARAM);
	afx_msg LRESULT OnGetFont (WPARAM, LPARAM);
	afx_msg BOOL OnNeedToolTipText(UINT id, NMHDR* pNMH, LRESULT* pResult);
	afx_msg LRESULT OnBCGUpdateToolTips (WPARAM, LPARAM);
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	afx_msg void OnContextMenu(CWnd*, CPoint point);
	DECLARE_MESSAGE_MAP()

	HFONT SetCurrFont (CDC* pDC);

protected:

	CBCGPPlannerView* GetCurrentView ()
	{
		ASSERT_VALID (m_pCurrentView);
		return m_pCurrentView;
	}

	CBCGPPlannerView* GetView (BCGP_PLANNER_TYPE type);

	const CBCGPPlannerView* GetView (BCGP_PLANNER_TYPE type) const;

	void StartCapture ();

	void StopCapture ();

	BOOL IsCaptured () const
	{
		return m_bCaptured;
	}

	void StartDragDrop ();

	void StopDragDrop ();

	BOOL CanStartDragDrop () const;

	BOOL IsDragDrop () const
	{
		return m_bDragDrop;
	}

	DROPEFFECT GetDragEffect () const
	{
		return m_dragEffect;
	}

	void _InternalRemoveSelectedAppointments (BOOL bQuery = TRUE, BOOL bRedraw = TRUE);

	virtual void InitToolTipInfo ();
	virtual void AddToolTipInfo (const CRect& rect);
	virtual void AddToolTipInfo (const CBCGPAppointment* pApp);
	virtual void ClearToolTipInfo ();

	BOOL UpdateChangeOperation (BCGP_PLANNER_CHANGE_OPERATION operation, BOOL bResult = FALSE);

	virtual LRESULT NotifyMessage(UINT message, WPARAM wParam, LPARAM lParam, BOOL bPost = FALSE) const;

	//----------------------
	// Accessibility support:
	//----------------------

// IAccessible
public:
	virtual HRESULT get_accChildCount(long *pcountChildren);
	virtual HRESULT get_accChild(VARIANT varChild, IDispatch **ppdispChild);
	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName);
	virtual HRESULT get_accValue(VARIANT varChild, BSTR *pszValue);
	virtual HRESULT get_accDescription(VARIANT varChild, BSTR *pszDescription);
	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole);
	virtual HRESULT get_accState(VARIANT varChild, VARIANT *pvarState);
	virtual HRESULT get_accHelp(VARIANT varChild, BSTR *pszHelp);
	virtual HRESULT get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic);
	virtual HRESULT get_accKeyboardShortcut(VARIANT varChild, BSTR *pszKeyboardShortcut);
	virtual HRESULT get_accFocus(VARIANT *pvarChild);
	virtual HRESULT get_accSelection(VARIANT *pvarChildren);
	virtual HRESULT get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction);

	virtual HRESULT accSelect(long flagsSelect, VARIANT varChild);
	virtual HRESULT accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild);
	virtual HRESULT accHitTest(long xLeft, long yTop, VARIANT *pvarChild);

protected:
	const CBCGPAppointment* GetAccAppointment() const;

protected:
	BOOL						m_bReadOnly;

	CBCGPScrollBar				m_wndHeaderScrollVert;
	CBCGPScrollBar				m_wndScrollVert;
	BOOL						m_bScrollVisible;
	BOOL						m_bHeaderScrollingEnabled;

	COLORREF					m_clrBackground;

	// first & last - double representation
	// [0 - 23].[0 - 59]
	double						m_nFirstWorkingHour;
	double						m_nLastWorkingHour;

	double						m_nFirstViewHour;
	double						m_nLastViewHour;
	double						m_nFirstPrintHour;
	double						m_nLastPrintHour;

	int							m_nWorkWeekInterval;

	static int					m_nWeekStart;

	BCGP_PLANNER_TYPE			m_Type;			// Planner type
	BCGP_PLANNER_CHANGE_OPERATION
								m_ChangeOperation;

	XBCGPAppointmentArray		m_arQueryApps;
	XBCGPAppointmentList		m_lsSelectedApps;

	CBCGPPlannerView::BCGP_PLANNER_TIME_DELTA
								m_TimeDelta;

	CImageList					m_ilUpDown;
	CSize						m_szUpDown;

	CBCGPPlannerView*			m_pViews [5];
	CBCGPPlannerView*			m_pCurrentView;

	CBCGPPlannerDropTarget		m_DropTarget;
	CBCGPPlannerDropSource		m_DropSource;

	BOOL						m_bCaptured;
	CWnd*						m_pWndLastCapture;

	CPoint						m_ptCaptureStart;
	CPoint						m_ptCaptureCurrent;

	BOOL						m_bDragDrop;
	DROPEFFECT					m_dragEffect;
	COleDateTime				m_dtDrop;
	BOOL						m_bRedrawAfterDrop;

	BOOL						m_bNotifyParent;
	CWnd*						m_pNotifyWnd;

	COleDateTime				m_dateFirst;
	COleDateTime				m_dateLast;

	CBCGPAppointmentBaseStorage*	m_pAppsStorage;
	BOOL						m_bMultiResourceStorage;
	CBCGPPlannerClockIcons*		m_pClockIcons;
	CBCGPPlannerPrint*			m_pPrinter;

	static CSize                s_ImageSize;
	static CImageList           s_ImageList;

	CBCGPCalendar*				m_pWndCalendar;

	CToolTipCtrl*				m_pToolTip;
	int							m_ToolTipCount;
	BOOL						m_bShowToolTip;
	BOOL						m_bToolTipShowAlways;

	static CBCGPRecurrenceRuleRegistrator s_RecurrenceRules;

	BOOL						m_bDefaultDrawFlags;
	DWORD						m_dwDrawFlags;
	
	BOOL						m_bUseDayViewInsteadWeekView;
	BOOL						m_bUseMultiResourceDefault;
};

#endif // BCGP_EXCLUDE_PLANNER

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPLANNERMANAGERCTRL_H__54C9C402_F5A3_4B0F_A71D_89E6AA0114D9__INCLUDED_)

