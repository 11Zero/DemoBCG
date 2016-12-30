#if !defined(AFX_BCGPPLANNERMANAGERVIEW_H__CF30104F_F699_40EA_80C5_133DDBA82928__INCLUDED_)
#define AFX_BCGPPLANNERMANAGERVIEW_H__CF30104F_F699_40EA_80C5_133DDBA82928__INCLUDED_

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
// BCGPPlannerManagerView.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerManagerCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerManagerView view

class BCGCBPRODLLEXPORT CBCGPPlannerManagerView : public CView
{
	friend class CBCGPPlannerManagerCtrl;

protected:
	CBCGPPlannerManagerView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBCGPPlannerManagerView)

// Attributes
public:
	CBCGPPlannerManagerCtrl& GetManagerCtrl ()
	{
		return (m_pwndManagerCtrl != NULL) ? *m_pwndManagerCtrl : m_wndManagerCtrl;
	}

	const CBCGPPlannerManagerCtrl& GetManagerCtrl () const
	{
		return (m_pwndManagerCtrl != NULL) ? *m_pwndManagerCtrl : m_wndManagerCtrl;
	}

// Operations
public:

	void SetType (CBCGPPlannerManagerCtrl::BCGP_PLANNER_TYPE type);

	CBCGPPlannerManagerCtrl::BCGP_PLANNER_TYPE GetType () const
	{
		ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

		return GetManagerCtrl ().GetType ();
	}

	void SetTimeDelta (CBCGPPlannerView::BCGP_PLANNER_TIME_DELTA delta);

	CBCGPPlannerView::BCGP_PLANNER_TIME_DELTA GetTimeDelta () const
	{
		ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

		return GetManagerCtrl ().GetTimeDelta ();
	}

	void SetCompressWeekend (BOOL bCompress);

	BOOL IsCompressWeekend () const
	{
		ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

		return GetManagerCtrl ().IsCompressWeekend ();
	}

	void SetDrawTimeFinish (BOOL bDraw);

	BOOL IsDrawTimeFinish () const
	{
		ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

		return GetManagerCtrl ().IsDrawTimeFinish ();
	}

	void SetDrawTimeAsIcons (BOOL bDraw);

	BOOL IsDrawTimeAsIcons () const
	{
		ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

		return GetManagerCtrl ().IsDrawTimeAsIcons ();
	}

	void SetFirstDayOfWeek (int nDay);	// 0-6

	void SetWorkingHourInterval (int nFirstHour, int nLastHour);

	int GetFirstWorkingHour () const
	{
		ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

		return GetManagerCtrl ().GetFirstWorkingHour ();
	}

	int GetLastWorkingHour () const
	{
		ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

		return GetManagerCtrl ().GetLastWorkingHour ();
	}

	const COleDateTime& GetDate () const
	{
		ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

		return GetManagerCtrl ().GetDate ();
	}

	const COleDateTime& GetDateStart () const
	{
		ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

		return GetManagerCtrl ().GetDateStart ();
	}

	const COleDateTime& GetDateEnd () const
	{
		ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

		return GetManagerCtrl ().GetDateEnd ();
	}

	void SetToday ();

	int GetViewDuration () const
	{
		ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

		return GetManagerCtrl ().GetViewDuration ();
	}

	//void SetDate (const COleDateTime& date);

	void SerializeRaw (CArchive& ar)
	{
		GetManagerCtrl ().SerializeRaw (ar);
	}

	void SetReadOnly (BOOL bReadOnly = TRUE);
	BOOL IsReadOnly () const;

	void SetManagerCtrlRTC (CRuntimeClass* pManagerCtrlClass);

// Implementation
protected:
	virtual ~CBCGPPlannerManagerView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPPlannerManagerView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);	
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

protected:
	virtual CBCGPPlannerView::BCGP_PLANNER_HITTEST HitTest (const CPoint& point) const;

	COleDateTime GetDateFromPoint (const CPoint& point) const;

	// on appointment added
	virtual void OnAppointmentAdded (CBCGPAppointment* /*pApp*/){}
	// on appointment added with result (by default, this method calls "OnAppointmentAdded" and returns 0L)
	// Return value:
	// -1L - cancel
	//  0L - add
	virtual UINT OnAppointmentAddedRet (CBCGPAppointment* pApp);
	// on before update appointment
	// Return value:
	// -1L - cancel update
	//  0L - update
	virtual UINT OnUpdateAppointment (CBCGPAppointment* /*pApp*/){return 0L;}	
	// on appointment updated
	virtual void OnAppointmentUpdated (CBCGPAppointment* /*pApp*/){}
	// on after appointment updated
	virtual void OnAllAppointmentsUpdated (){}
	// on removing appointment
	// Return value:
	// -1L - cancel delete
	//  0L - delete single appointment or only occurrence of the series
	//  1L - delete full series
	virtual UINT OnRemoveAppointment (CBCGPAppointment* /*pApp*/){return 0L;}
	// on all appointments removed
	virtual void OnAllAppointmentsRemoved (){}
	// on select appointment
	// Return value:
	// FALSE - cancel operation
	// TRUE  - allow operation
	virtual BOOL OnSelectAppointment (CBCGPAppointment* /*pApp*/, BOOL /*bSelect*/){return TRUE;}
	// on appointment selected
	virtual void OnAppointmentSelected (CBCGPAppointment* /*pApp*/, BOOL /*bSelected*/){}

	// on appointments dropped on control
	virtual void OnAppointmentsDropped () {}
	// on appointments dragged on control
	virtual void OnAppointmentsDragged (DROPEFFECT /*dropEffect*/) {}
	
	virtual BOOL OnBeginChangeOperation (CBCGPPlannerManagerCtrl::BCGP_PLANNER_CHANGE_OPERATION /*operation*/){	return TRUE;	}
	virtual void OnEndChangeOperation (CBCGPPlannerManagerCtrl::BCGP_PLANNER_CHANGE_OPERATION /*operation*/, BOOL /*bResult*/){}

	// on double click on control
	virtual void OnDblClkCtrl (UINT /*nFlags*/, CPoint /*point*/)	{}
	// on key down on control
	virtual void OnKeyDownCtrl(UINT /*nChar*/, UINT /*nRepCnt*/, UINT /*nFlags*/) {}
	// on time interval changed
	virtual void OnSelectTimeInterval (COleDateTime /*dateStart*/, COleDateTime /*dateFinish*/)	{}
	// on click on "up-down" icon
	// bIconUp - TRUE if click on icon "up"
	// Return value - return FALSE if default handler is needed
	virtual BOOL OnIconUpDownClick(BOOL /*bIconUp*/) {return FALSE;}
	// click on day caption
	// wParam, lParam - not used
	// Return value - return FALSE if default handler is needed
	virtual BOOL OnDayCaptionClick() {return FALSE;}
	// click on week caption
	// wParam, lParam - not used
	// Return value - return FALSE if default handler is needed
	virtual BOOL OnWeekCaptionClick() {return FALSE;}

	virtual void OnTypeChanged (CBCGPPlannerManagerCtrl::BCGP_PLANNER_TYPE /*oldType*/,
		CBCGPPlannerManagerCtrl::BCGP_PLANNER_TYPE /*newType*/) {}
	virtual void OnDateChanged () {}
	virtual void OnResourceIDChanged () {}

	virtual CBCGPPlannerView::BCGP_PLANNER_WORKING_STATUS GetWorkingPeriodParameters (int ResourceId, const COleDateTime& dtStart, const COleDateTime& dtEnd, CBCGPPlannerView::XBCGPPlannerWorkingParameters& parameters) const;

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPPlannerManagerView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnTimeChange();
	//}}AFX_MSG
	afx_msg LRESULT OnNotifyAddApp (WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyBeforeUpdateApp (WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyUpdateApp (WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyUpdateAllApp (WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyRemoveApp (WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyRemoveAllApp (WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyBeforeSelectApp(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifySelectApp(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyTypeChanged (WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyDateChanged (WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyResourceIDChanged (WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyLButtonDblClk(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyKeyDown(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyIconUpDownClick(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyDayCaptionClick(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyWeekCaptionClick(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyDropApp(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyDragApp(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyBeginChangeOperation(WPARAM, LPARAM);
	afx_msg LRESULT OnNotifyEndChangeOperation(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

protected:
	CBCGPPlannerManagerCtrl		m_wndManagerCtrl;
	CBCGPPlannerManagerCtrl*	m_pwndManagerCtrl;

	CRuntimeClass*				m_pManagerCtrlClass;
};

#endif // BCGP_EXCLUDE_PLANNER

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPLANNERMANAGERVIEW_H__CF30104F_F699_40EA_80C5_133DDBA82928__INCLUDED_)
