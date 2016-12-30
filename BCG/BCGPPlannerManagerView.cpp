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
// BCGPPlannerManagerView.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPPlannerManagerView.h"

#ifndef BCGP_EXCLUDE_PLANNER

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerManagerView

IMPLEMENT_DYNCREATE(CBCGPPlannerManagerView, CView)

CBCGPPlannerManagerView::CBCGPPlannerManagerView()
	: m_pwndManagerCtrl   (NULL)
	, m_pManagerCtrlClass (NULL)
{
}

CBCGPPlannerManagerView::~CBCGPPlannerManagerView()
{
	if (m_pwndManagerCtrl != NULL)
	{
		delete m_pwndManagerCtrl;
		m_pwndManagerCtrl = NULL;
	}
}


BEGIN_MESSAGE_MAP(CBCGPPlannerManagerView, CView)
	//{{AFX_MSG_MAP(CBCGPPlannerManagerView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_WM_TIMECHANGE ()
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_ADD_APPOINTMENT, OnNotifyAddApp)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_BEFORE_UPDATE_APPOINTMENT, OnNotifyBeforeUpdateApp)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_UPDATE_APPOINTMENT, OnNotifyUpdateApp)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_UPDATED_ALL_APPOINTMENTS, OnNotifyUpdateAllApp)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_REMOVE_APPOINTMENT, OnNotifyRemoveApp)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_REMOVE_ALL_APPOINTMENTS, OnNotifyRemoveAllApp)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_BEFORE_SELECT_APPOINTMENT, OnNotifyBeforeSelectApp)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_SELECT_APPOINTMENT, OnNotifySelectApp)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_TYPE_CHANGED, OnNotifyTypeChanged)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_DATE_CHANGED, OnNotifyDateChanged)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_RESOURCEID_CHANGED, OnNotifyResourceIDChanged)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_LBUTTONDBLCLK, OnNotifyLButtonDblClk)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_KEYDOWN, OnNotifyKeyDown)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_ICONUPDOWN_CLICK, OnNotifyIconUpDownClick)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_DAYCAPTION_CLICK, OnNotifyDayCaptionClick)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_WEEKCAPTION_CLICK, OnNotifyWeekCaptionClick)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_DROP_APPOINTMENTS, OnNotifyDropApp)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_DRAG_APPOINTMENTS, OnNotifyDragApp)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_BEGIN_CHANGE_OPERATION, OnNotifyBeginChangeOperation)
	ON_REGISTERED_MESSAGE(BCGP_PLANNER_END_CHANGE_OPERATION, OnNotifyEndChangeOperation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CBCGPPlannerManagerView::SetManagerCtrlRTC (CRuntimeClass* pManagerCtrlClass)
{
	ASSERT (m_pManagerCtrlClass == NULL);

	m_pManagerCtrlClass = pManagerCtrlClass;
}

void CBCGPPlannerManagerView::SetType (CBCGPPlannerManagerCtrl::BCGP_PLANNER_TYPE type)
{
	if (GetManagerCtrl ().GetType () != type)
	{
		GetManagerCtrl ().SetType (type);
	}
}

void CBCGPPlannerManagerView::SetTimeDelta (CBCGPPlannerView::BCGP_PLANNER_TIME_DELTA delta)
{
	if (GetManagerCtrl ().GetTimeDelta () != delta)
	{
		GetManagerCtrl ().SetTimeDelta (delta);
	}
}

void CBCGPPlannerManagerView::SetToday ()
{
	if (GetManagerCtrl ().GetSafeHwnd () != NULL)
	{
		GetManagerCtrl ().SetToday ();
	}
}

void CBCGPPlannerManagerView::SetCompressWeekend (BOOL bCompress)
{
	if (GetManagerCtrl ().IsCompressWeekend () != bCompress)
	{
		GetManagerCtrl ().SetCompressWeekend (bCompress);
	}
}

void CBCGPPlannerManagerView::SetDrawTimeFinish (BOOL bDraw)
{
	if (GetManagerCtrl ().IsDrawTimeFinish () != bDraw)
	{
		GetManagerCtrl ().SetDrawTimeFinish (bDraw);
	}
}

void CBCGPPlannerManagerView::SetDrawTimeAsIcons (BOOL bDraw)
{
	if (GetManagerCtrl ().IsDrawTimeAsIcons () != bDraw)
	{
		GetManagerCtrl ().SetDrawTimeAsIcons (bDraw);
	}
}

void CBCGPPlannerManagerView::SetFirstDayOfWeek (int nDay)
{
	if (CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () != nDay)
	{
		GetManagerCtrl ().SetFirstDayOfWeek (nDay);
	}
}

void CBCGPPlannerManagerView::SetWorkingHourInterval (int nFirstHour, int nLastHour)
{
	GetManagerCtrl ().SetWorkingHourInterval (nFirstHour, nLastHour);
}

void CBCGPPlannerManagerView::SetReadOnly (BOOL bReadOnly /*= TRUE*/)
{
	GetManagerCtrl ().SetReadOnly (bReadOnly);
}

BOOL CBCGPPlannerManagerView::IsReadOnly () const
{
	return GetManagerCtrl ().IsReadOnly ();
}

CBCGPPlannerView::BCGP_PLANNER_HITTEST
CBCGPPlannerManagerView::HitTest (const CPoint& point) const
{
	ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

	return GetManagerCtrl ().HitTest (point);
}

COleDateTime CBCGPPlannerManagerView::GetDateFromPoint (const CPoint& point) const
{
	ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

	return GetManagerCtrl ().GetDateFromPoint (point);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerManagerView drawing

void CBCGPPlannerManagerView::OnDraw(CDC* /*pDC*/)
{
}

void CBCGPPlannerManagerView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

	if (GetManagerCtrl ().GetSafeHwnd () != NULL)
	{
		GetManagerCtrl ().OnPrint (pDC, pInfo);
	}
}

BOOL CBCGPPlannerManagerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

	if (GetManagerCtrl ().GetSafeHwnd () != NULL)
	{
		GetManagerCtrl ().OnPreparePrinting (pInfo);
	}

	return DoPreparePrinting(pInfo);
}

void CBCGPPlannerManagerView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

	if (GetManagerCtrl ().GetSafeHwnd () != NULL)
	{	
		GetManagerCtrl ().OnBeginPrinting (pDC, pInfo);
	}
}

void CBCGPPlannerManagerView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT(GetManagerCtrl ().GetSafeHwnd () != NULL);

	if (GetManagerCtrl ().GetSafeHwnd () != NULL)
	{
		GetManagerCtrl ().OnEndPrinting (pDC, pInfo);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerManagerView diagnostics

#ifdef _DEBUG
void CBCGPPlannerManagerView::AssertValid() const
{
	CView::AssertValid();
}

void CBCGPPlannerManagerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerManagerView message handlers

int CBCGPPlannerManagerView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CBCGPPlannerManagerCtrl* pCtrl = &m_wndManagerCtrl;

	if (m_pManagerCtrlClass != NULL)
	{
		if (m_pManagerCtrlClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPPlannerManagerCtrl)))
		{
			m_pwndManagerCtrl = 
				(CBCGPPlannerManagerCtrl*)m_pManagerCtrlClass->CreateObject ();

			ASSERT_VALID (m_pwndManagerCtrl);
			pCtrl = m_pwndManagerCtrl;
		}
	}

	if (pCtrl == NULL || 
		!pCtrl->Create (WS_CHILD | WS_VISIBLE, CRect (0, 0, 0, 0), this, 0))
	{
		TRACE0("CBCGPPlannerManagerView::OnCreate: cannot create manager control\n");
		return -1;
	}
	
	return 0;
}

BOOL CBCGPPlannerManagerView::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

void CBCGPPlannerManagerView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	GetManagerCtrl ().MoveWindow (0, 0, cx, cy);
}

void CBCGPPlannerManagerView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	
	if (GetManagerCtrl ().GetSafeHwnd () != NULL)
	{
		GetManagerCtrl ().SetFocus ();
	}
}

void CBCGPPlannerManagerView::OnEditCopy() 
{
	if (GetManagerCtrl ().GetSafeHwnd () != NULL && GetManagerCtrl ().IsEditCopyEnabled ())
	{
		GetManagerCtrl ().EditCopy ();
	}
}

void CBCGPPlannerManagerView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (GetManagerCtrl ().GetSafeHwnd () != NULL && 
		GetManagerCtrl ().IsEditCopyEnabled ());
}

void CBCGPPlannerManagerView::OnEditCut() 
{
	if (GetManagerCtrl ().GetSafeHwnd () != NULL && GetManagerCtrl ().IsEditCutEnabled ())
	{
		GetManagerCtrl ().EditCut ();
	}
}

void CBCGPPlannerManagerView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (GetManagerCtrl ().GetSafeHwnd () != NULL && 
		GetManagerCtrl ().IsEditCutEnabled ());	
}

void CBCGPPlannerManagerView::OnEditPaste() 
{
	if (GetManagerCtrl ().GetSafeHwnd () != NULL && GetManagerCtrl ().IsEditPasteEnabled ())
	{
		GetManagerCtrl ().EditPaste ();
	}
}

void CBCGPPlannerManagerView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable (GetManagerCtrl ().GetSafeHwnd () != NULL && 
		GetManagerCtrl ().IsEditPasteEnabled ());	
}

void CBCGPPlannerManagerView::OnTimeChange() 
{
	if (GetManagerCtrl ().GetSafeHwnd () != NULL)
	{
		GetManagerCtrl ().SendMessage (WM_TIMECHANGE, 0, 0);
	}
}

UINT CBCGPPlannerManagerView::OnAppointmentAddedRet (CBCGPAppointment* pApp)
{
	OnAppointmentAdded (pApp);
	return 0L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyAddApp (WPARAM /*wParam*/, LPARAM lParam)
{
	return OnAppointmentAddedRet ((CBCGPAppointment*) lParam);
}

LRESULT CBCGPPlannerManagerView::OnNotifyBeforeUpdateApp (WPARAM /*wParam*/, LPARAM lParam)
{
	return OnUpdateAppointment ((CBCGPAppointment*) lParam);
}

LRESULT CBCGPPlannerManagerView::OnNotifyUpdateApp (WPARAM /*wParam*/, LPARAM lParam)
{
	OnAppointmentUpdated ((CBCGPAppointment*) lParam);
	return 0L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyUpdateAllApp (WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	OnAllAppointmentsUpdated ();
	return 0L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyRemoveApp (WPARAM /*wParam*/, LPARAM lParam)
{
	return OnRemoveAppointment ((CBCGPAppointment*) lParam);
}

LRESULT CBCGPPlannerManagerView::OnNotifyRemoveAllApp (WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	OnAllAppointmentsRemoved ();
	return 0L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyBeforeSelectApp(WPARAM wParam, LPARAM lParam)
{
	return OnSelectAppointment ((CBCGPAppointment*) lParam, (BOOL) wParam)
		? 0L
		: -1L;
}

LRESULT CBCGPPlannerManagerView::OnNotifySelectApp(WPARAM wParam, LPARAM lParam)
{
	OnAppointmentSelected ((CBCGPAppointment*) lParam, (BOOL) wParam);
	return 0L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyTypeChanged (WPARAM wParam, LPARAM lParam)
{
	OnTypeChanged (CBCGPPlannerManagerCtrl::BCGP_PLANNER_TYPE (wParam),
		CBCGPPlannerManagerCtrl::BCGP_PLANNER_TYPE (lParam));
	return 0L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyDateChanged (WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	OnDateChanged ();
	return 0L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyResourceIDChanged (WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	OnResourceIDChanged ();
	return 0L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyLButtonDblClk (WPARAM wParam, LPARAM lParam)
{
	CPoint pt(BCG_GET_X_LPARAM(lParam), BCG_GET_Y_LPARAM(lParam));
	OnDblClkCtrl ((UINT) wParam, pt);
	return 0L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyKeyDown (WPARAM wParam, LPARAM lParam)
{
	OnKeyDownCtrl ((UINT) wParam, LOWORD(lParam), HIWORD(lParam));
	return 0L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyIconUpDownClick (WPARAM wParam, LPARAM /*lParam*/)
{
	return OnIconUpDownClick (wParam == (WPARAM) CBCGPPlannerView::BCGP_PLANNER_HITTEST_ICON_UP)
		? 1L
		: 0L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyDayCaptionClick (WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return OnDayCaptionClick () ? 1L : 0L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyWeekCaptionClick (WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return OnWeekCaptionClick () ? 1L : 0L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyDropApp(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	OnAppointmentsDropped ();
	return 0L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyDragApp(WPARAM wParam, LPARAM /*lParam*/)
{
	OnAppointmentsDragged ((DROPEFFECT)wParam);
	return 0L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyBeginChangeOperation(WPARAM wParam, LPARAM /*lParam*/)
{
	return OnBeginChangeOperation
		((CBCGPPlannerManagerCtrl::BCGP_PLANNER_CHANGE_OPERATION)wParam) ? 0L : -1L;
}

LRESULT CBCGPPlannerManagerView::OnNotifyEndChangeOperation(WPARAM wParam, LPARAM lParam)
{
	OnEndChangeOperation
		((CBCGPPlannerManagerCtrl::BCGP_PLANNER_CHANGE_OPERATION)wParam, (BOOL)lParam);
	return 0L;
}

CBCGPPlannerView::BCGP_PLANNER_WORKING_STATUS
CBCGPPlannerManagerView::GetWorkingPeriodParameters (int /*ResourceId*/, const COleDateTime& /*dtStart*/, const COleDateTime& /*dtEnd*/, CBCGPPlannerView::XBCGPPlannerWorkingParameters& /*parameters*/) const
{
	return CBCGPPlannerView::BCGP_PLANNER_WORKING_STATUS_UNKNOWN;
}

#endif // BCGP_EXCLUDE_PLANNER
