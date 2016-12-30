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
// BCGPPlannerManagerCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPPlannerManagerCtrl.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerClockIcons.h"

#include "BCGPPlannerViewDay.h"
#include "BCGPPlannerViewWorkWeek.h"
#include "BCGPPlannerViewWeek.h"
#include "BCGPPlannerViewMonth.h"
#include "BCGPPlannerViewMulti.h"

#include "BCGPPlannerPrintDay.h"
#include "BCGPPlannerPrintWeek.h"
#include "BCGPPlannerPrintMonth.h"
#include "BCGPPlannerPrintDual.h"

#include "BCGPPlannerManagerView.h"

#include "BCGPCalendarBar.h"

#include "BCGPMath.h"

#ifndef _BCGPCALENDAR_STANDALONE
	#include "bcgprores.h"
#else
	#include "resource.h"
#endif

#include "BCGPLocalResource.h"

#include "BCGPAppointmentStorage.h"

#include "BCGPRecurrenceRules.h"

#ifndef _BCGSUITE_
#include "BCGPTooltipManager.h"
#endif

#ifndef _BCGPCALENDAR_STANDALONE
	#include "BCGPVisualManager.h"
	#define visualManager	CBCGPVisualManager::GetInstance ()
#else
	#include "BCGPCalendarVisualManager.h"
	#define visualManager	CBCGPCalendarVisualManager::GetInstance ()
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// uncomment this string if you want drag & drop on another planner
#define BCGP_PLANNER_DND_TO_ANOTHER_PLANNER

#ifdef _UNICODE
	#define _TCF_TEXT	CF_UNICODETEXT
#else
	#define _TCF_TEXT	CF_TEXT
#endif

// wParam - not used, lParam - points to the appointment
UINT BCGP_PLANNER_ADD_APPOINTMENT			= ::RegisterWindowMessage (_T("BCGP_PLANNER_ADD_APPOINTMENT"));
// wParam - not used, lParam - points to the appointment
// Return value:
// -1L - cancel update
//  0L - update
UINT BCGP_PLANNER_BEFORE_UPDATE_APPOINTMENT	= ::RegisterWindowMessage (_T("BCGP_PLANNER_BEFORE_UPDATE_APPOINTMENT"));
// wParam - not used, lParam - points to the appointment
UINT BCGP_PLANNER_UPDATE_APPOINTMENT		= ::RegisterWindowMessage (_T("BCGP_PLANNER_UPDATE_APPOINTMENT"));
// wParam - not used, lParam - not used
UINT BCGP_PLANNER_UPDATED_ALL_APPOINTMENTS	= ::RegisterWindowMessage (_T("BCGP_PLANNER_UPDATED_ALL_APPOINTMENTS"));
// wParam - not used, lParam - points to the appointment
// Return value:
// -1L - cancel delete
//	0L - delete single appointment or only occurrence of the series
//	1L - delete full series
UINT BCGP_PLANNER_REMOVE_APPOINTMENT		= ::RegisterWindowMessage (_T("BCGP_PLANNER_REMOVE_APPOINTMENT"));
// wParam, lParam - not used
UINT BCGP_PLANNER_REMOVE_ALL_APPOINTMENTS	= ::RegisterWindowMessage (_T("BCGP_PLANNER_REMOVE_ALL_APPOINTMENTS"));

// wParam - TRUE - select, otherwise - unselect, lParam - points to the appointment
// Return value:
// -1L - cancel operation
//  0L - allow operation
UINT BCGP_PLANNER_BEFORE_SELECT_APPOINTMENT = ::RegisterWindowMessage (_T("BCGP_PLANNER_BEFORE_SELECT_APPOINTMENT"));
// wParam - TRUE - selected, otherwise - not selected, lParam - points to the appointment
UINT BCGP_PLANNER_SELECT_APPOINTMENT = ::RegisterWindowMessage (_T("BCGP_PLANNER_SELECT_APPOINTMENT"));

// wParam - old type, lParam - new type
UINT BCGP_PLANNER_TYPE_CHANGED				= ::RegisterWindowMessage (_T("BCGP_PLANNER_TYPE_CHANGED"));
// wParam, lParam - not used
UINT BCGP_PLANNER_DATE_CHANGED				= ::RegisterWindowMessage (_T("BCGP_PLANNER_DATE_CHANGED"));
// wParam, lParam - not used
UINT BCGP_PLANNER_RESOURCEID_CHANGED		= ::RegisterWindowMessage (_T("BCGP_PLANNER_RESOURCEID_CHANGED"));

// wParam, lParam - see WM_LBUTTONDBLCLK
UINT BCGP_PLANNER_LBUTTONDBLCLK 			= ::RegisterWindowMessage (_T("BCGP_PLANNER_LBUTTONDBLCLK"));
// wParam, lParam - see WM_KEYDOWN
UINT BCGP_PLANNER_KEYDOWN					= ::RegisterWindowMessage (_T("BCGP_PLANNER_KEYDOWN"));
// click on "up-down" icon
// wParam - hit test code of icon, see CBCGPPlannerView
// Return value - return FALSE if default handler is needed
UINT BCGP_PLANNER_ICONUPDOWN_CLICK			= ::RegisterWindowMessage (_T("BCGP_PLANNER_ICONUPDOWN_CLICK"));

// click on day caption
// wParam, lParam - not used
// Return value - return FALSE if default handler is needed
UINT BCGP_PLANNER_DAYCAPTION_CLICK          = ::RegisterWindowMessage (_T("BCGP_PLANNER_DAYCAPTION_CLICK"));

// click on week caption
// wParam, lParam - not used
// Return value - return FALSE if default handler is needed
UINT BCGP_PLANNER_WEEKCAPTION_CLICK         = ::RegisterWindowMessage (_T("BCGP_PLANNER_WEEKCAPTION_CLICK"));

// wParam, lParam - not used
UINT BCGP_PLANNER_DROP_APPOINTMENTS         = ::RegisterWindowMessage (_T("BCGP_PLANNER_DROP_APPOINTMENTS"));
// wParam - drop effect code, lParam - not used
UINT BCGP_PLANNER_DRAG_APPOINTMENTS         = ::RegisterWindowMessage (_T("BCGP_PLANNER_DRAG_APPOINTMENTS"));

// wParam - operation code, lParam - not used
// Return value:
// -1L - cancel operation
//  0L - allow operation
UINT BCGP_PLANNER_BEGIN_CHANGE_OPERATION    = ::RegisterWindowMessage (_T("BCGP_PLANNER_BEGIN_CHANGE_OPERATION"));
// wParam - operation code, lParam - result of operation
UINT BCGP_PLANNER_END_CHANGE_OPERATION      = ::RegisterWindowMessage (_T("BCGP_PLANNER_END_CHANGE_OPERATION"));


IMPLEMENT_DYNCREATE(CBCGPPlannerManagerCtrl, CBCGPWnd)

CLIPFORMAT CBCGPPlannerManagerCtrl::s_ClpFormat = 0;
CString CBCGPPlannerManagerCtrl::s_ClpFormatName = _T("BCGPPlannerManagerClpFmt");

CSize CBCGPPlannerManagerCtrl::s_ImageSize (0, 0);
CImageList CBCGPPlannerManagerCtrl::s_ImageList;

CBCGPRecurrenceRuleRegistrator CBCGPPlannerManagerCtrl::s_RecurrenceRules;

int CBCGPPlannerManagerCtrl::m_nWeekStart = 1;

template <class TYPE>
void CopyList (const TYPE& lstSrc, TYPE& lstDst)
{
	lstDst.RemoveAll ();
	
	POSITION pos = lstSrc.GetHeadPosition ();

	while (pos != NULL)
	{
		lstDst.AddTail (lstSrc.GetNext (pos));
	}
}


int CBCGPPlannerManagerCtrl::GetSystemFirstDayOfWeek (BOOL bConvertDayOfWeek)
{
	const int c_Size = 10;
	TCHAR szLocaleData[c_Size];
	::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_IFIRSTDAYOFWEEK, szLocaleData, c_Size);
	szLocaleData[1] = 0;

	int nDay = _ttoi (szLocaleData);
	if (bConvertDayOfWeek)
	{
		nDay = (nDay + 8) % 7;
	}

	return nDay;
}

CString CBCGPPlannerManagerCtrl::GetSystemWeekDayName (int nDay)
{
	CString str;

	::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SDAYNAME1 + nDay, str.GetBuffer (100), 100);
	str.ReleaseBuffer ();
	
	return str;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerManagerCtrl

CBCGPPlannerManagerCtrl::CBCGPPlannerManagerCtrl
(
	CRuntimeClass* pStorageClass,
	CRuntimeClass* pClockIconsClass
)
	: m_pAppsStorage      (NULL)
	, m_pClockIcons       (NULL)
	, m_pPrinter          (NULL)
	, m_pCurrentView      (NULL)
	, m_Type              (BCGP_PLANNER_TYPE_DAY)
	, m_ChangeOperation   (BCGP_PLANNER_CHANGE_OPERATION_NONE)
	, m_TimeDelta         (CBCGPPlannerView::BCGP_PLANNER_TIME_DELTA_60)
	, m_nFirstWorkingHour ( 9.0)
	, m_nLastWorkingHour  (18.0)
	, m_nFirstViewHour    ( 0.0)
	, m_nLastViewHour     (23.0)
	, m_nFirstPrintHour   ( 0.0)
	, m_nLastPrintHour    (23.0)
	, m_szUpDown          (0, 0)
	, m_bCaptured         (FALSE)
	, m_pWndLastCapture   (NULL)
	, m_bDragDrop         (FALSE)
	, m_ptCaptureStart    (0, 0)
	, m_ptCaptureCurrent  (0, 0)
	, m_dragEffect        (DROPEFFECT_NONE)
	, m_clrBackground     (CLR_DEFAULT)
	, m_bNotifyParent     (TRUE)
	, m_pNotifyWnd        (NULL)
	, m_pWndCalendar      (NULL)
	, m_dateFirst         ()
	, m_dateLast          ()
	, m_bReadOnly         (FALSE)
	, m_ToolTipCount      (0)
	, m_bShowToolTip      (TRUE)
	, m_bToolTipShowAlways(FALSE)
	, m_pToolTip          (NULL)
	, m_bRedrawAfterDrop  (FALSE)
	, m_bDefaultDrawFlags (TRUE)
	, m_dwDrawFlags       (0)
	, m_bUseDayViewInsteadWeekView (FALSE)
	, m_bScrollVisible    (TRUE)
	, m_bHeaderScrollingEnabled(TRUE)
	, m_bMultiResourceStorage (FALSE)
	, m_bUseMultiResourceDefault (FALSE)
	, m_nWorkWeekInterval (5)
{
	if (GetClipboardFormat () == 0)
	{
		s_ClpFormat = (CLIPFORMAT)
			::RegisterClipboardFormat (GetClipboardFormatName ());
	}

	SetStorageRTC (pStorageClass);
	SetClockIconsRTC (pClockIconsClass);

	RegisterRule (RUNTIME_CLASS (CBCGPRecurrenceRuleDaily));
	RegisterRule (RUNTIME_CLASS (CBCGPRecurrenceRuleWeekly));
	RegisterRule (RUNTIME_CLASS (CBCGPRecurrenceRuleMonthly));
	RegisterRule (RUNTIME_CLASS (CBCGPRecurrenceRuleYearly));

	SetUpDownIcons ((UINT)0);

	//m_nWeekStart = GetSystemFirstDayOfWeek () + 1;
	m_nWeekStart = GetSystemFirstDayOfWeek (TRUE) + 1;

	for (int i = 0; i < sizeof (m_pViews) / sizeof (CBCGPPlannerView*); i++)
	{
		m_pViews [i] = NULL;
	}

	SetScrollBarsStyle (CBCGPScrollBar::BCGP_SBSTYLE_DEFAULT);
}

CBCGPPlannerManagerCtrl::~CBCGPPlannerManagerCtrl()
{
	if (m_pAppsStorage != NULL)
	{
		delete m_pAppsStorage;
		m_pAppsStorage = NULL;
	}

	if (m_pClockIcons != NULL)
	{
		delete m_pClockIcons;
		m_pClockIcons = NULL;
	}

	if (m_pPrinter != NULL)
	{
		delete m_pPrinter;
		m_pPrinter = NULL;
	}

	int i = 0;
	for (i = 0; i < sizeof (m_pViews) / sizeof (CBCGPPlannerView*); i++)
	{
		if (m_pViews [i] != NULL)
		{
			delete m_pViews [i];
			m_pViews [i] = NULL;
		}
	}
}

void CBCGPPlannerManagerCtrl::SetStorageRTC (CRuntimeClass* pStorageClass/* = NULL*/, BOOL bDelete/* = TRUE*/)
{
	if (m_pAppsStorage != NULL)
	{
		if (bDelete)
		{
			ASSERT_VALID (m_pAppsStorage);
			delete m_pAppsStorage;
		}

		m_pAppsStorage = NULL;
	}

	if (pStorageClass != NULL)
	{
		if (!pStorageClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPAppointmentBaseStorage)))
		{
			ASSERT (FALSE);
			pStorageClass = NULL;
		}
	}
	
	if (pStorageClass == NULL)
	{
		if (!m_bUseMultiResourceDefault)
		{
			pStorageClass = RUNTIME_CLASS (CBCGPAppointmentStorage);
		}
		else
		{
			pStorageClass = RUNTIME_CLASS (CBCGPAppointmentMultiStorage);
		}
	}

	m_pAppsStorage = (CBCGPAppointmentBaseStorage*)pStorageClass->CreateObject ();
	ASSERT_VALID (m_pAppsStorage);

	if (m_pAppsStorage != NULL)
	{
		m_bMultiResourceStorage = m_pAppsStorage->IsKindOf (RUNTIME_CLASS(CBCGPAppointmentBaseMultiStorage));
	}
}

void CBCGPPlannerManagerCtrl::SetClockIconsRTC (CRuntimeClass* pClockIconsClass/* = NULL*/, BOOL bDelete/* = TRUE*/)
{
	if (m_pClockIcons != NULL)
	{
		if (bDelete)
		{
			ASSERT_VALID (m_pClockIcons);
			delete m_pClockIcons;
		}

		m_pClockIcons = NULL;
	}

	if (pClockIconsClass != NULL)
	{
		if (!pClockIconsClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPPlannerClockIcons)))
		{
			ASSERT (FALSE);
			pClockIconsClass = NULL;
		}
	}

	if (pClockIconsClass == NULL)
	{
		pClockIconsClass = RUNTIME_CLASS (CBCGPPlannerClockIcons);
	}

	m_pClockIcons  = (CBCGPPlannerClockIcons*)pClockIconsClass->CreateObject ();
	ASSERT_VALID (m_pClockIcons);
}

BEGIN_MESSAGE_MAP(CBCGPPlannerManagerCtrl, CBCGPWnd)
	//{{AFX_MSG_MAP(CBCGPPlannerManagerCtrl)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
	ON_WM_TIMECHANGE()
	ON_WM_CANCELMODE()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_GETFONT, OnGetFont)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnNeedToolTipText)
	ON_REGISTERED_MESSAGE(BCGM_UPDATETOOLTIPS, OnBCGUpdateToolTips)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

CLIPFORMAT CBCGPPlannerManagerCtrl::GetClipboardFormat ()
{
	return s_ClpFormat;
}

LPCTSTR CBCGPPlannerManagerCtrl::GetClipboardFormatName ()
{
	return s_ClpFormatName;
}

void CBCGPPlannerManagerCtrl::SetImages (UINT nResID, int cxImage, COLORREF clrTransparent)
{
	SetImages (MAKEINTRESOURCE (nResID), cxImage, clrTransparent);
}

void CBCGPPlannerManagerCtrl::SetImages (LPCTSTR szResID, int cxImage, COLORREF clrTransparent)
{
	if (s_ImageList.GetSafeHandle () != NULL)
	{
		s_ImageList.DeleteImageList ();
	}

	s_ImageSize = CSize (0, 0);

	if (szResID == NULL)
	{
		return;
	}

	HBITMAP hBmp = (HBITMAP)::LoadImage (AfxFindResourceHandle (szResID, RT_BITMAP), 
		szResID, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);

	if (hBmp == NULL)
	{
		return;
	}

	CBitmap bmp;
	bmp.Attach (hBmp);

	if (bmp.GetSafeHandle () == NULL)
	{
		return;
	}

	BITMAP bm;
	ZeroMemory (&bm, sizeof (BITMAP));

	bmp.GetBitmap (&bm);

	UINT nFlags = clrTransparent == (COLORREF)-1 ? 0 : ILC_MASK;

	switch (bm.bmBitsPixel)
	{
	case 4 :
		nFlags |= ILC_COLOR4;
		break;
	case 8 :
		nFlags |= ILC_COLOR8;
		break;
	case 16:
		nFlags |= ILC_COLOR16;
		break;
	case 24:
		nFlags |= ILC_COLOR24;
		break;
	case 32:
		nFlags |= ILC_COLOR32;
		if (clrTransparent == (COLORREF)-1)
		{
			nFlags |= ILC_MASK;
		}
		break;
	default:
		ASSERT (FALSE);
	}

	s_ImageSize.cx = cxImage;
	s_ImageSize.cy = bm.bmHeight;

	int nCount = bm.bmWidth / cxImage;

	s_ImageList.Create (s_ImageSize.cx, s_ImageSize.cy, nFlags, nCount, 0);

	if ((nFlags & ILC_COLOR32) == ILC_COLOR32 &&
		clrTransparent == (COLORREF)-1)
	{
		s_ImageList.Add (&bmp, (CBitmap*) NULL);
	}
	else
	{
		s_ImageList.Add (&bmp, clrTransparent);
	}
}

void CBCGPPlannerManagerCtrl::InitImages ()
{
	if (s_ImageSize != CSize (0, 0))
	{
		return;
	}

	CBCGPLocalResource localRes;
	SetImages (IDB_BCGBARRES_CALENDAR_ICONS, 16);
}

void CBCGPPlannerManagerCtrl::DrawImageIcon (CDC* pDC, const CPoint& pt, int nIndex)
{
	ASSERT_VALID (pDC);
	ASSERT (nIndex != -1);

	InitImages ();

	if (pDC == NULL || pDC->GetSafeHdc () == NULL || 
		!(0 <= nIndex && nIndex < s_ImageList.GetImageCount ()))
	{
		return;
	}

	s_ImageList.Draw (pDC, nIndex, pt, ILD_NORMAL);
}


DWORD CBCGPPlannerManagerCtrl::RegisterRule (CRuntimeClass* pRuleClass)
{
	return s_RecurrenceRules.RegisterRule (pRuleClass);
}

CBCGPRecurrenceBaseRule* CBCGPPlannerManagerCtrl::CreateRule (DWORD ID)
{
	CBCGPRecurrenceBaseRule* pRule = s_RecurrenceRules.CreateRule (ID);

	return pRule;
}

void CBCGPPlannerManagerCtrl::GetRulesID (CBCGPRecurrenceRuleRegistrator::XBCGPRecurrenceRuleIDArray& arID)
{
	s_RecurrenceRules.GetRulesID (arID);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerManagerCtrl message handlers

BOOL CBCGPPlannerManagerCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID) 
{
	return CWnd::Create (	
		globalData.RegisterWindowClass (_T("BCGPPlannerManagerCtrl")),
		_T(""), dwStyle, rect, pParentWnd, nID, NULL);
}

void CBCGPPlannerManagerCtrl::OnDraw (CDC* pDC)
{
	CBCGPPlannerView* pView = GetCurrentView ();	

	CRect rectClient;
	GetClientRect (rectClient);
	rectClient.right = pView->m_rectApps.right;

	pView->OnPaint (pDC, rectClient);
}

void CBCGPPlannerManagerCtrl::OnPreparePrinting(CPrintInfo* pInfo, CRuntimeClass* pPrinterClass)
{
	ASSERT_VALID(this);

	if (pPrinterClass != NULL)
	{
		if (!pPrinterClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPPlannerPrint)))
		{
			ASSERT (FALSE);

			pPrinterClass = NULL;
		}
	}

	if (pPrinterClass == NULL)
	{
		switch (m_Type)
		{
		case BCGP_PLANNER_TYPE_DAY:
			if (GetViewDuration () == 1)
			{
				pPrinterClass = RUNTIME_CLASS (CBCGPPlannerPrintDay);
			}
			else
			{
				pPrinterClass = RUNTIME_CLASS (CBCGPPlannerPrintDual);
			}
			break;
		case BCGP_PLANNER_TYPE_WORK_WEEK:
			pPrinterClass = RUNTIME_CLASS (CBCGPPlannerPrintWeek);
			break;
		case BCGP_PLANNER_TYPE_WEEK:
			pPrinterClass = RUNTIME_CLASS (CBCGPPlannerPrintWeek);
			break;
		case BCGP_PLANNER_TYPE_MONTH:
			pPrinterClass = RUNTIME_CLASS (CBCGPPlannerPrintMonth);
			break;
		}
	}

	if (pPrinterClass == NULL)
	{
		return;
	}

	CBCGPPlannerPrint* pPrinter = (CBCGPPlannerPrint*)pPrinterClass->CreateObject ();

	ASSERT_VALID (pPrinter);

	if (m_pPrinter != NULL)
	{
		delete m_pPrinter;
		m_pPrinter = NULL;
	}

	m_pPrinter = pPrinter;

	m_pPrinter->PrepareInfo (pInfo);
}

void CBCGPPlannerManagerCtrl::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo, CRuntimeClass* pPrinterClass /*= NULL*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	
	if (m_pPrinter == NULL)
	{
		OnPreparePrinting(pInfo, pPrinterClass);
	}

	if (m_pPrinter != NULL)
	{
		ASSERT_VALID(m_pPrinter);
		m_pPrinter->PreparePrinting (pDC, pInfo, this);
	}	
}

void CBCGPPlannerManagerCtrl::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	ASSERT_VALID(this);

	if (m_pPrinter != NULL)
	{
		delete m_pPrinter;
		m_pPrinter = NULL;
	}

	AdjustLayout ();
}

void CBCGPPlannerManagerCtrl::OnPrint (CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT(pInfo != NULL);
	ASSERT(pInfo->m_bContinuePrinting);

	if (m_pPrinter != NULL)
	{
		ASSERT_VALID(m_pPrinter);

		m_pPrinter->OnPaint (pDC, pInfo);
	}
}

void CBCGPPlannerManagerCtrl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	DoPaint(&dc);
}

void CBCGPPlannerManagerCtrl::DoPaint(CDC* pDCPaint) 
{
	ASSERT_VALID(pDCPaint);

	CBCGPPlannerView* pView = GetCurrentView ();	

	CRect rectClient;
	GetClientRect (rectClient);
	rectClient.right = pView->m_rectApps.right;

	CDC memDC;
	if (memDC.CreateCompatibleDC (pDCPaint))
	{
		CBitmap memBitmap;
		if (memBitmap.CreateCompatibleBitmap (pDCPaint, rectClient.Width (), rectClient.Height ()))
		{
			memDC.SelectObject (&memBitmap);
		}
		else
		{
			memDC.DeleteDC ();
		}
	}

	BOOL bMemDC = memDC.GetSafeHdc () != NULL;

	CDC* pDC = bMemDC ? &memDC : pDCPaint;

	pDC->SetBkMode (TRANSPARENT);

	HFONT hfontOld = pView->SetCurrFont (pDC);

	OnDraw (pDC);

	if (bMemDC)
	{
		pDCPaint->BitBlt (rectClient.left, rectClient.top, rectClient.Width (), rectClient.Height (),
			&memDC, 0, 0, SRCCOPY);
	}

	::SelectObject (pDC->GetSafeHdc (), hfontOld);
}

BOOL CBCGPPlannerManagerCtrl::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

void CBCGPPlannerManagerCtrl::OnDestroy() 
{
	CBCGPTooltipManager::DeleteToolTip (m_pToolTip);

	RemoveAllAppointments (FALSE);

	int i = 0;

	for (i = 0; i < sizeof (m_pViews) / sizeof (CBCGPPlannerView*); i++)
	{
		ASSERT_VALID (m_pViews [i]);

		if (GetCurrentView () == m_pViews [i])
		{
			m_pViews [i]->OnDeactivate (this);
		}
	}

	for (i = 0; i < sizeof (m_pViews) / sizeof (CBCGPPlannerView*); i++)
	{
		if (m_pViews [i] != NULL)
		{
			delete m_pViews [i];
			m_pViews [i] = NULL;
		}
	}

	CBCGPWnd::OnDestroy();
}

int CBCGPPlannerManagerCtrl::GetViewDuration () const
{
	return GetCurrentView ()->GetViewDuration ();
}

const COleDateTime& CBCGPPlannerManagerCtrl::GetDate () const
{
	return GetCurrentView ()->GetDate ();
}

const COleDateTime& CBCGPPlannerManagerCtrl::GetDateStart () const
{
	return GetCurrentView ()->GetDateStart ();
}

const COleDateTime& CBCGPPlannerManagerCtrl::GetDateEnd () const
{
	return GetCurrentView ()->GetDateEnd ();
}

void CBCGPPlannerManagerCtrl::SetReadOnly (BOOL bReadOnly /*= TRUE*/)
{
	if (m_bReadOnly != bReadOnly)
	{
		m_bReadOnly = bReadOnly;

		if (GetSafeHwnd () != NULL)
		{
			GetCurrentView ()->StopEditAppointment ();
		}
	}
}

void CBCGPPlannerManagerCtrl::SetToday (BOOL bRedraw /*= TRUE*/)
{
	SetDate (COleDateTime::GetCurrentTime (), bRedraw);
}

void CBCGPPlannerManagerCtrl::SetDateInterval (const COleDateTime& date1, 
											   const COleDateTime& date2, 
											   BOOL bRedraw /*= TRUE*/)
{
	ASSERT (date2 >= date1);

	COleDateTimeSpan span (date2 - date1);

	SetRedraw (FALSE);

	CBCGPCalendar* pCalendar = m_pWndCalendar;
	m_pWndCalendar = NULL;

	if (span.GetTotalDays () == 6 && 
		date1 == CBCGPPlannerView::GetFirstWeekDay (date1, GetFirstDayOfWeek () + 1) &&
		!m_bUseDayViewInsteadWeekView)
	{
		SetType (BCGP_PLANNER_TYPE_WEEK, FALSE);

		COleDateTime dt (CBCGPPlannerView::GetFirstWeekDay (date1, 
			CBCGPPlannerManagerCtrl::GetFirstDayOfWeek () + 1));

		// finding monday
		while (dt.GetDayOfWeek () != 2)
		{
			dt += COleDateTimeSpan (1, 0, 0, 0);
		}

		GetCurrentView ()->m_DateStart = dt;
		GetCurrentView ()->m_DateEnd   = dt + COleDateTimeSpan (6, 23, 59, 59);
		GetCurrentView ()->SetDate (dt);
	}
	else if (span.GetTotalDays () > 6)
	{
		SetType (BCGP_PLANNER_TYPE_MONTH, FALSE);
		GetCurrentView ()->SetDateInterval (date1, date2);
	}
	else
	{
		BOOL bUpdate = m_Type == BCGP_PLANNER_TYPE_DAY ||
					   m_Type == BCGP_PLANNER_TYPE_WORK_WEEK ||
					   m_Type == BCGP_PLANNER_TYPE_MULTI;

		CBCGPPlannerView* pView = GetCurrentView ();

		COleDateTime dtSel1 (pView->GetSelectionStart ());
		COleDateTime dtSel2 (pView->GetSelectionEnd ());

		if (m_Type == BCGP_PLANNER_TYPE_MULTI)
		{
			SetType (BCGP_PLANNER_TYPE_MULTI, FALSE);
		}
		else
		{
			SetType (BCGP_PLANNER_TYPE_DAY, FALSE);
		}
		GetCurrentView ()->SetDateInterval (date1, date2);

		if (bUpdate)
		{
			COleDateTime dt 	 (pView->GetDate ());
			COleDateTime dtStart (pView->GetDateStart ());
			COleDateTime dtEnd	 (pView->GetDateEnd ());

			BOOL bAdd = dtSel1 < dtSel2;
			COleDateTimeSpan span (bAdd ? dtSel2 - dtSel1 : dtSel1 - dtSel2);

			dt = COleDateTime (dt.GetYear (), dt.GetMonth (), dt.GetDay (),
							   dtSel1.GetHour (), dtSel1.GetMinute (), dtSel1.GetSecond ()); 

			if (dt != dtSel1)
			{
				dtSel1 = dt;
			}

			if (bAdd)
			{
				dtSel2 = dtSel1 + span;
			}
			else
			{
				dtSel2 = dtSel1;
				dtSel1 -= span;
			}

			if (dtSel1 < dtStart ||
				dtSel1 > dtEnd ||
				dtSel2 < dtStart ||
				dtSel2 > dtEnd)
			{
				const int nMinuts = CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ());

				dtSel1 = COleDateTime (dtSel1.GetYear (), dtSel1.GetMonth (), dtSel1.GetDay (), 0, 0, 0);
				dtSel1 += COleDateTimeSpan (0, GetFirstSelectionHour (), 
					(int)(GetFirstSelectionMinute () / nMinuts) * nMinuts, 0);
				dtSel2 = dtSel1;
			}

			pView->SetSelection 
				(
					dtSel1, 
					dtSel2,
					FALSE
				);
		}
	}

	SetRedraw (TRUE);

	if (bRedraw)
	{
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	m_pWndCalendar = pCalendar;

	OnDateChanged ();
}

void CBCGPPlannerManagerCtrl::SetDate (const COleDateTime& date, BOOL bRedraw /*= TRUE*/)
{
	CBCGPPlannerView* pView = GetCurrentView ();
	ASSERT_VALID (pView);

	COleDateTime dt (date.GetYear (), date.GetMonth (), date.GetDay (), 0, 0, 0);

	if (dt == pView->GetDate ())
	{
		return;
	}

	COleDateTime dtStart (pView->GetDateStart ());
	COleDateTime dtEnd (pView->GetDateEnd ());

	if ((m_Type == BCGP_PLANNER_TYPE_WEEK ||
		 m_Type == BCGP_PLANNER_TYPE_MONTH))
	{
		COleDateTime dtS (pView->CalculateDateStart (dt));

		if (dtStart <= dt && dt <= dtEnd && dtS == dtStart)
		{
			ClearAppointmentSelection (FALSE);
			
			pView->m_Date = dt;
			pView->SetSelection (dt, dt, TRUE);
		}
		else
		{
			pView->SetDate (dt);
		}
	}
	else
	{
		SetRedraw (FALSE);

		COleDateTime dtSel1 (pView->GetSelectionStart ());
		COleDateTime dtSel2 (pView->GetSelectionEnd ());

		if (GetViewDuration () > 1 && dtStart <= dt && dt <= dtEnd)
		{
			pView->m_Date = dt;
		}
		else
		{
			BOOL bSetDate = TRUE;

			if (m_Type == BCGP_PLANNER_TYPE_WORK_WEEK)
			{
				if (dtStart <= dt && dt <= (dtStart + COleDateTimeSpan (GetWorkWeekInterval () - 1, 23, 59, 59)))
				{
					bSetDate = FALSE;
				}
			}
			
			if (bSetDate)
			{
				pView->SetDate (dt);

				if (m_Type == BCGP_PLANNER_TYPE_WORK_WEEK)
				{
					if (pView->GetDateEnd () < dt)
					{
						pView->m_Date = pView->GetDateStart ();
					}
				}
			}
			else
			{
				SetRedraw (TRUE);
				OnDateChanged ();
				return;
			}
		}

		dt = pView->GetDate ();

		dtStart = pView->GetDateStart ();
		dtEnd	= pView->GetDateEnd ();

		BOOL bAdd = dtSel1 < dtSel2;
		COleDateTimeSpan span (bAdd ? dtSel2 - dtSel1 : dtSel1 - dtSel2);

		dt = COleDateTime (dt.GetYear (), dt.GetMonth (), dt.GetDay (),
						   dtSel1.GetHour (), dtSel1.GetMinute (), dtSel1.GetSecond ()); 

		if (dt != dtSel1)
		{
			dtSel1 = dt;
		}

		if (bAdd)
		{
			dtSel2 = dtSel1 + span;
		}
		else
		{
			dtSel2 = dtSel1;
			dtSel1 -= span;
		}

		if (dtSel1 < dtStart ||
			dtSel1 > dtEnd ||
			dtSel2 < dtStart ||
			dtSel2 > dtEnd)
		{
			const int nMinuts = CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ());

			dtSel1 = COleDateTime (dtSel1.GetYear (), dtSel1.GetMonth (), dtSel1.GetDay (), 0, 0, 0);
			dtSel1 += COleDateTimeSpan (0, GetFirstSelectionHour (), 
				(int)(GetFirstSelectionMinute () / nMinuts) * nMinuts, 0);
			dtSel2 = dtSel1;
		}

		pView->SetSelection 
			(
				dtSel1, 
				dtSel2,
				FALSE
			);

		SetRedraw (TRUE);

		if (bRedraw)
		{
			RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}

	OnDateChanged ();
}

void CBCGPPlannerManagerCtrl::OnDateChanged ()
{
	UpdateCalendars ();

	if (m_bNotifyParent)
	{
		NotifyMessage (BCGP_PLANNER_DATE_CHANGED, 0, 0);
	}
}

void CBCGPPlannerManagerCtrl::SetCompressWeekend (BOOL bCompress)
{
	GetCurrentView ()->SetCompressWeekend (bCompress);
	
	if (GetFirstDayOfWeek () != 1)
	{
		OnDateChanged ();
	}
}

BOOL CBCGPPlannerManagerCtrl::IsCompressWeekend () const
{
	return GetCurrentView ()->IsCompressWeekend ();
}

void CBCGPPlannerManagerCtrl::SetDrawTimeFinish (BOOL bDraw)
{
	GetCurrentView ()->SetDrawTimeFinish (bDraw);
}

void CBCGPPlannerManagerCtrl::SetNotifyParent (BOOL bNotifyParent)
{
	m_bNotifyParent = bNotifyParent;
}

BOOL CBCGPPlannerManagerCtrl::IsNotifyParent () const
{
	return m_bNotifyParent;
}

BOOL CBCGPPlannerManagerCtrl::IsDrawTimeFinish () const
{
	return GetCurrentView ()->IsDrawTimeFinish ();
}

void CBCGPPlannerManagerCtrl::SetDrawTimeAsIcons (BOOL bDraw)
{
	GetCurrentView ()->SetDrawTimeAsIcons (bDraw);
}

BOOL CBCGPPlannerManagerCtrl::IsDrawTimeAsIcons () const
{
	return GetCurrentView ()->IsDrawTimeAsIcons ();
}

void CBCGPPlannerManagerCtrl::SetSelection (const COleDateTime& sel1, const COleDateTime& sel2, BOOL bRedraw)
{
	GetCurrentView ()->SetSelection (sel1, sel2, bRedraw);
}

COleDateTime CBCGPPlannerManagerCtrl::GetSelectionStart () const
{
	return GetCurrentView ()->GetSelectionStart ();
}

COleDateTime CBCGPPlannerManagerCtrl::GetSelectionEnd () const
{
	return GetCurrentView ()->GetSelectionEnd ();
}

void CBCGPPlannerManagerCtrl::SetType (BCGP_PLANNER_TYPE type, BOOL bRedraw /*= TRUE*/)
{
	ASSERT (BCGP_PLANNER_TYPE_FIRST <= type);
	ASSERT (type <= BCGP_PLANNER_TYPE_LAST);

	if (m_Type == type)
	{
		return;
	}

	if (!IsMultiResourceStorage () && type == BCGP_PLANNER_TYPE_MULTI)
	{
		return;
	}

	BCGP_PLANNER_TYPE oldType = m_Type;

	const BOOL bIsWndCreated = GetSafeHwnd () != NULL;

	if (bIsWndCreated)
	{
		SetRedraw (FALSE);
	}

	XBCGPAppointmentList lst;
	CopyList<XBCGPAppointmentList>(m_lsSelectedApps, lst);
	
	//COleDateTime date = GetCurrentView ()->GetDate ();
	CBCGPPlannerView* pOldView = m_pCurrentView;

	COleDateTime dtSel1 (pOldView->GetSelectionStart ());
	COleDateTime dtSel2 (pOldView->GetSelectionEnd ());

	UINT nResourceID = GetCurrentResourceID ();

	if (pOldView != NULL)
	{
		pOldView->OnDeactivate (this);
	}

	m_Type = type;

	m_pCurrentView = GetView (m_Type);

	SetCurrentResourceID (nResourceID, FALSE);

	m_pCurrentView->OnActivate (this, pOldView);
	

	if (m_Type == BCGP_PLANNER_TYPE_DAY ||
		m_Type == BCGP_PLANNER_TYPE_WORK_WEEK ||
		m_Type == BCGP_PLANNER_TYPE_MULTI)
	{
		if (oldType == BCGP_PLANNER_TYPE_WEEK || 
			dtSel1 < m_pCurrentView->GetDateStart () ||
			dtSel1 > m_pCurrentView->GetDateEnd () ||
			dtSel2 < m_pCurrentView->GetDateStart () ||
			dtSel2 > m_pCurrentView->GetDateEnd ())
		{
			const int nMinuts = CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ());

			dtSel1 = m_pCurrentView->GetDateStart () + 
				COleDateTimeSpan (0, GetFirstSelectionHour (), 
					(int)(GetFirstSelectionMinute () / nMinuts) * nMinuts, 0);
			dtSel2 = dtSel1;
		}

		if ((m_Type == BCGP_PLANNER_TYPE_DAY && oldType == BCGP_PLANNER_TYPE_MULTI) ||
			(m_Type == BCGP_PLANNER_TYPE_MULTI && oldType == BCGP_PLANNER_TYPE_DAY))
		{
			m_pCurrentView->SetDateInterval (pOldView->GetDateStart (), pOldView->GetDateEnd ());
		}

		m_pCurrentView->SetSelection 
			(
				dtSel1, 
				dtSel2,
				FALSE
			);
	}

	RestoreAppointmentSelection (lst, FALSE);

	if (bIsWndCreated)
	{
		SetRedraw (TRUE);

		if (bRedraw)
		{
			RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}

	UpdateCalendarsSelection ();
	UpdateCalendarsState ();

	if (m_bNotifyParent)
	{
		NotifyMessage (BCGP_PLANNER_TYPE_CHANGED, (WPARAM) oldType, (LPARAM) m_Type);
	}
}

BOOL CBCGPPlannerManagerCtrl::AddAppointment (CBCGPAppointment* pApp, BOOL bQuery, BOOL bRedraw)
{
	ASSERT_VALID (pApp);

	CBCGPAppointmentBaseStorage* pAppsStorage = GetStorage ();
	ASSERT_VALID (pAppsStorage);

	LRESULT lRes = 0L;

	if (m_bNotifyParent)
	{
		lRes = NotifyMessage (BCGP_PLANNER_ADD_APPOINTMENT, 0, (LPARAM) pApp);
	}

	if (lRes == LRESULT(-1L))
	{
		return FALSE;
	}

	if (pApp->GetResourceID () == CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID)
	{
		pApp->SetResourceID (GetCurrentResourceID ());
	}

	if (!pAppsStorage->Add (pApp))
	{
		return FALSE;
	}

	CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION action = 
		((pApp->IsAllDay () || pApp->IsMultiDay ()) &&
			(m_Type == BCGP_PLANNER_TYPE_DAY || m_Type == BCGP_PLANNER_TYPE_WORK_WEEK ||
			 m_Type == BCGP_PLANNER_TYPE_MULTI))
			? CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_LAYOUT
			: CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_APPOINTMENTS;

	if (bQuery)
	{
		QueryAppointments ();
	}

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		if (action == CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_LAYOUT)
		{
			GetCurrentView ()->AdjustLayout (FALSE);
		}
		else
		{
			GetCurrentView ()->AdjustAppointments ();
		}

		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	if (bQuery)
	{
		UpdateCalendarsState ();
	}

	return TRUE;
}

BOOL CBCGPPlannerManagerCtrl::UpdateAppointment (CBCGPAppointment* pApp, 
												 const COleDateTime& dtOld,
												 BOOL bQuery, BOOL bRedraw)
{
	ASSERT_VALID (pApp);

	CBCGPAppointmentBaseStorage* pAppsStorage = GetStorage ();
	ASSERT_VALID (pAppsStorage);

	BOOL bRecurrenceClone = pApp->IsRecurrence () && pApp->IsRecurrenceClone ();

	LRESULT lRes = 0L;

	if (m_bNotifyParent)
	{
		lRes = NotifyMessage (BCGP_PLANNER_BEFORE_UPDATE_APPOINTMENT, 0, (LPARAM) pApp);
	}

	if (lRes == LRESULT(-1L))
	{
		return FALSE;
	}

	if (!pAppsStorage->Update (pApp, dtOld))
	{
		return FALSE;
	}

	if (m_bNotifyParent)
	{
		NotifyMessage (BCGP_PLANNER_UPDATE_APPOINTMENT, 0, 
			(LPARAM)((bRecurrenceClone && bQuery) ? NULL : pApp));
	}

	if (bQuery)
	{
		QueryAppointments ();
	}
	else
	{
		SortAppointments (m_arQueryApps, (int) m_arQueryApps.GetSize ());
		GetCurrentView ()->AdjustAppointments ();
	}

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		GetCurrentView ()->AdjustAppointments ();
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	if (m_bNotifyParent)
	{
		NotifyMessage (BCGP_PLANNER_UPDATED_ALL_APPOINTMENTS, 0, 0, TRUE);
	}

	if (bQuery)
	{
		UpdateCalendarsState ();
	}

	return TRUE;
}

BOOL CBCGPPlannerManagerCtrl::RemoveAppointment (CBCGPAppointment* pApp, BOOL bQuery, BOOL bRedraw)
{
	ASSERT_VALID (pApp);

	CBCGPAppointmentBaseStorage* pAppsStorage = GetStorage ();
	ASSERT_VALID (pAppsStorage);

	if (pApp == NULL)
	{
		return FALSE;
	}

	LRESULT lRes = 0L;

	if (m_bNotifyParent)
	{
		lRes = NotifyMessage (BCGP_PLANNER_REMOVE_APPOINTMENT, 0, (LPARAM) pApp);
	}

	if (lRes == LRESULT(-1L))
	{
		return FALSE;
	}

	CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION action = 
		((pApp->IsAllDay () || pApp->IsMultiDay ()) &&
			(m_Type == BCGP_PLANNER_TYPE_DAY || m_Type == BCGP_PLANNER_TYPE_WORK_WEEK ||
			 m_Type == BCGP_PLANNER_TYPE_MULTI))
			? CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_LAYOUT
			: CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_APPOINTMENTS;

	DWORD ID = 0;
	if (pApp->IsRecurrenceClone () && lRes == 1)
	{
		ID = pApp->GetRecurrenceID ();
		pApp = GetRecurrence (ID);

		if (action == CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_APPOINTMENTS)
		{
			action = ((pApp->IsAllDay () || pApp->IsMultiDay ()) &&
				(m_Type == BCGP_PLANNER_TYPE_DAY || m_Type == BCGP_PLANNER_TYPE_WORK_WEEK ||
				 m_Type == BCGP_PLANNER_TYPE_MULTI))
				? CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_LAYOUT
				: CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_APPOINTMENTS;
		}
	}
	else if (pApp->IsRecurrence ())
	{
		ID = pApp->GetRecurrenceID ();
	}

	// remove series
	// remove all cloned appointments from query and selection
	if (ID != 0)
	{
		for (int i = 0; i < m_arQueryApps.GetSize (); i++)
		{
			CBCGPAppointment* p = m_arQueryApps[i];

			if (p != NULL && p != pApp && 
				p->IsRecurrenceClone () && p->GetRecurrenceID () == ID)
			{
				ASSERT_VALID(p);

				m_arQueryApps[i] = NULL;

				POSITION pos = m_lsSelectedApps.Find (p);
				if (pos != NULL)
				{
					m_lsSelectedApps.RemoveAt (pos);
				}

				delete p;
			}
		}
	}

	// remove appointment from query
	for (int i = 0; i < m_arQueryApps.GetSize (); i++)
	{
		if (m_arQueryApps[i] == pApp)
		{
			m_arQueryApps[i] = NULL;
			break;
		}
	}

	// remove appointment from selection
	POSITION pos = m_lsSelectedApps.Find (pApp);
	if (pos != NULL)
	{
		m_lsSelectedApps.RemoveAt (pos);
	}

	pAppsStorage->Remove (pApp);

	if (bQuery)
	{
		QueryAppointments ();
	}

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		if (action == CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_LAYOUT)
		{
			GetCurrentView ()->AdjustLayout (FALSE);
		}
		else
		{
			GetCurrentView ()->AdjustAppointments ();
		}

		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	if (bQuery)
	{
		UpdateCalendarsState ();
	}

	return TRUE;
}

void CBCGPPlannerManagerCtrl::RemoveSelectedAppointments (BOOL bQuery, BOOL bRedraw)
{
	CBCGPAppointmentBaseStorage* pAppsStorage = GetStorage ();
	ASSERT_VALID (pAppsStorage);

	CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION action = 
		CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_NONE;

#ifdef _DEBUG
	CBCGPPlannerView* pView = GetCurrentView ();
	ASSERT_VALID (pView);
#endif

	CList<DWORD, DWORD> lsID;

	POSITION pos = m_lsSelectedApps.GetHeadPosition ();
	while (pos != NULL)
	{
		CBCGPAppointment* pApp = m_lsSelectedApps.GetNext (pos);
		ASSERT_VALID (pApp);

		if (pApp == NULL)
		{
			continue;
		}

		if (action != CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_LAYOUT)
		{
			action = ((pApp->IsAllDay () || pApp->IsMultiDay ()) &&
				(m_Type == BCGP_PLANNER_TYPE_DAY || m_Type == BCGP_PLANNER_TYPE_WORK_WEEK ||
				 m_Type == BCGP_PLANNER_TYPE_MULTI))
				? CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_LAYOUT
				: CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_APPOINTMENTS;
		}

		LRESULT lRes = 0L;

		BOOL bNotify = TRUE;
		if (pApp->IsRecurrenceClone ())
		{
			bNotify = lsID.Find (pApp->GetRecurrenceID ()) == NULL;
		}

		if (m_bNotifyParent && bNotify)
		{
			lRes = NotifyMessage (BCGP_PLANNER_REMOVE_APPOINTMENT, 0, (LPARAM) pApp);
		}

		if (lRes != LRESULT(-1L))
		{
			if (pApp->IsRecurrenceClone () && lRes == 1)
			{
				DWORD ID = pApp->GetRecurrenceID ();
				pApp = GetRecurrence (ID);

				if (action != CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_LAYOUT)
				{
					action = ((pApp->IsAllDay () || pApp->IsMultiDay ()) &&
						(m_Type == BCGP_PLANNER_TYPE_DAY || m_Type == BCGP_PLANNER_TYPE_WORK_WEEK ||
						 m_Type == BCGP_PLANNER_TYPE_MULTI))
						? CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_LAYOUT
						: CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_APPOINTMENTS;
				}

				lsID.AddTail (ID);
			}

			pAppsStorage->Remove (pApp);

			for (int i = 0; i < m_arQueryApps.GetSize (); i++)
			{
				if (m_arQueryApps[i] == pApp)
				{
					m_arQueryApps[i] = NULL;
					break;
				}
			}
		}
	}

	if (action != CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_LAYOUT)
	{
		for (int i = 0; i < m_arQueryApps.GetSize (); i++)
		{
			CBCGPAppointment* pApp = m_arQueryApps[i];

			if (pApp == NULL)
			{
				continue;
			}

			if (pApp->IsRecurrenceClone ())
			{
				if (lsID.Find (pApp->GetRecurrenceID ()) != NULL)
				{
					action = ((pApp->IsAllDay () || pApp->IsMultiDay ()) &&
						(m_Type == BCGP_PLANNER_TYPE_DAY || m_Type == BCGP_PLANNER_TYPE_WORK_WEEK ||
						 m_Type == BCGP_PLANNER_TYPE_MULTI))
						? CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_LAYOUT
						: CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_APPOINTMENTS;

					if (action == CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_LAYOUT)
					{
						break;
					}
				}
			}
		}
	}

	m_lsSelectedApps.RemoveAll ();

	if (bQuery)
	{
		QueryAppointments ();
	}

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		if (action == CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_LAYOUT)
		{
			GetCurrentView ()->AdjustLayout (FALSE);
		}
		else
		{
			GetCurrentView ()->AdjustAppointments ();
		}
		
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	if (bQuery)
	{
		UpdateCalendarsState ();
	}
}

void CBCGPPlannerManagerCtrl::_InternalRemoveSelectedAppointments 
	(BOOL bQuery, BOOL bRedraw)
{
	BOOL bNotify = m_bNotifyParent;
	m_bNotifyParent = FALSE;

	RemoveSelectedAppointments (bQuery, bRedraw);

	m_bNotifyParent = bNotify;
}

void CBCGPPlannerManagerCtrl::ClearQuery ()
{
	for (int i = 0; i < m_arQueryApps.GetSize (); i++)
	{
		CBCGPAppointment* pApp = m_arQueryApps[i];

		if (pApp != NULL && pApp->IsRecurrenceClone ())
		{
			delete pApp;
		}
	}

	m_arQueryApps.RemoveAll ();
}

BOOL CBCGPPlannerManagerCtrl::RemoveAllAppointments (BOOL bRedraw)
{
	CBCGPAppointmentBaseStorage* pAppsStorage = GetStorage ();
	ASSERT_VALID (pAppsStorage);

	BOOL bIsEmpty = pAppsStorage->IsEmpty ();

	ClearQuery ();

	if (!pAppsStorage->RemoveAll ())
	{
		return FALSE;
	}

	m_lsSelectedApps.RemoveAll ();

	if (bRedraw && GetSafeHwnd () != NULL)
	{
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	if (m_bNotifyParent && !bIsEmpty)
	{
		NotifyMessage (BCGP_PLANNER_REMOVE_ALL_APPOINTMENTS, 0, 0);
	}

	UpdateCalendarsState ();
	return TRUE;
}
//******************************************************************************************
void CBCGPPlannerManagerCtrl::AdjustLayout (BOOL bRedraw)
{
	GetCurrentView ()->AdjustLayout (bRedraw);
}
//******************************************************************************************
CBCGPPlannerView* CBCGPPlannerManagerCtrl::OnCreateView(BCGP_PLANNER_TYPE type)
{
	CBCGPPlannerView* pView = NULL;

	switch(type)
	{
	case BCGP_PLANNER_TYPE_DAY:
		pView = new CBCGPPlannerViewDay;
		break;
	case BCGP_PLANNER_TYPE_WORK_WEEK:
		pView = new CBCGPPlannerViewWorkWeek;
		break;
	case BCGP_PLANNER_TYPE_WEEK:
		pView = new CBCGPPlannerViewWeek;
		break;
	case BCGP_PLANNER_TYPE_MONTH:
		pView = new CBCGPPlannerViewMonth;
		break;
	case BCGP_PLANNER_TYPE_MULTI:
		pView = new CBCGPPlannerViewMulti;
		break;
	default:
		ASSERT (FALSE);
		TRACE0("CBCGPPlannerManagerCtrl: can not create client view.");
	}

	return pView;
}
//******************************************************************************************
BOOL CBCGPPlannerManagerCtrl::PreCreateWindow (CREATESTRUCT& cs)
{
	CRuntimeClass* viewClass[] =
	{
		RUNTIME_CLASS(CBCGPPlannerViewDay),
		RUNTIME_CLASS(CBCGPPlannerViewWorkWeek),
		RUNTIME_CLASS(CBCGPPlannerViewWeek),
		RUNTIME_CLASS(CBCGPPlannerViewMonth),
		RUNTIME_CLASS(CBCGPPlannerViewMulti),
	};

	for (int i = 0; i < sizeof (m_pViews) / sizeof (CBCGPPlannerView*); i++)
	{
		m_pViews [i] = OnCreateView ((BCGP_PLANNER_TYPE)i);

		ASSERT_VALID (m_pViews [i]);
		if (m_pViews [i] == NULL)
		{
			return FALSE;
		}

		if (!m_pViews [i]->IsKindOf (viewClass[i]))
		{
			ASSERT (FALSE);
			return FALSE;
		}
	}

	m_pCurrentView = GetView (m_Type);
	m_pCurrentView->OnActivate (this, NULL);
	m_pCurrentView->SetDate (COleDateTime::GetCurrentTime ());

	const int nMinuts = CBCGPPlannerView::GetTimeDeltaInMinuts (GetTimeDelta ());

	COleDateTime dtSel (m_pCurrentView->GetDate ());
	dtSel += COleDateTimeSpan (0, GetFirstSelectionHour (), 
		(int)(GetFirstSelectionMinute () / nMinuts) * nMinuts, 0);

	m_pCurrentView->SetSelection (dtSel, dtSel);

	return CBCGPWnd::PreCreateWindow (cs);
}
//******************************************************************************************
int CBCGPPlannerManagerCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_wndScrollVert.Create (WS_CHILD | WS_VISIBLE | SBS_VERT, CRect (0, 0, 0, 0), this, 
		BCGP_PLANNER_ID_SCROLL);
	m_wndHeaderScrollVert.Create (WS_CHILD | WS_VISIBLE | SBS_VERT, CRect (0, 0, 0, 0), this, 
		BCGP_PLANNER_ID_SCROLL_HEADER);

	m_DropTarget.Register (this);

	if (CBCGPTooltipManager::CreateToolTip (m_pToolTip, this,
									BCGP_TOOLTIP_TYPE_PLANNER))
	{
		m_pToolTip->Activate (TRUE);
		m_pToolTip->SetMaxTipWidth (150);
		m_pToolTip->SetWindowPos (&wndTop, -1, -1, -1, -1,
								SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	AdjustLayout (FALSE);
	return 0;
}
//******************************************************************************************
HFONT CBCGPPlannerManagerCtrl::SetCurrFont (CDC* pDC)
{
	ASSERT_VALID (pDC);
	
	CFont* pFont = GetFont ();

	if (pFont == NULL)
	{
		return NULL;
	}

	return (HFONT) ::SelectObject (pDC->GetSafeHdc (), pFont->GetSafeHandle () );
}
//******************************************************************************************
LRESULT CBCGPPlannerManagerCtrl::OnSetFont (WPARAM wParam, LPARAM /*lParam*/)
{
	GetCurrentView ()->SetFont ((HFONT) wParam);

	AdjustLayout ();
	return 0;
}
//******************************************************************************************
LRESULT CBCGPPlannerManagerCtrl::OnGetFont (WPARAM, LPARAM)
{
	return (LRESULT)GetCurrentView ()->GetFont ();
}
//******************************************************************************************
void CBCGPPlannerManagerCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CBCGPWnd::OnSize(nType, cx, cy);
	AdjustLayout ();
}
//******************************************************************************************

CScrollBar* CBCGPPlannerManagerCtrl::GetScrollBarCtrl(int nBar) const
{
	if (nBar == SB_HORZ || m_wndScrollVert.GetSafeHwnd () == NULL)
	{
		return NULL;
	}

	return (CScrollBar* ) &m_wndScrollVert;
}

CScrollBar* CBCGPPlannerManagerCtrl::GetHeaderScrollBarCtrl(int nBar) const
{
	if (nBar == SB_HORZ || m_wndHeaderScrollVert.GetSafeHwnd () == NULL)
	{
		return NULL;
	}

	return (CScrollBar* ) &m_wndHeaderScrollVert;
}

BOOL CBCGPPlannerManagerCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	GetCurrentView ()->StopEditAppointment ();

	COleDateTime dtOld (GetCurrentView ()->GetDate ());

	BOOL bRes = GetCurrentView ()->OnMouseWheel (nFlags, zDelta, pt);

	if ((m_Type == BCGP_PLANNER_TYPE_WEEK ||
		m_Type == BCGP_PLANNER_TYPE_MONTH) &&
		GetCurrentView ()->GetDate () != dtOld && bRes)
	{
		OnDateChanged ();
	}

	return bRes;
}

void CBCGPPlannerManagerCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	COleDateTime dtOld (GetCurrentView ()->GetDate ());

	GetCurrentView ()->OnVScroll (nSBCode, nPos, pScrollBar);

	if ((m_Type == BCGP_PLANNER_TYPE_WEEK ||
		m_Type == BCGP_PLANNER_TYPE_MONTH) &&
		GetCurrentView ()->GetDate () != dtOld)
	{
		OnDateChanged ();
	}
}

void CBCGPPlannerManagerCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus ();

	CBCGPPlannerView::BCGP_PLANNER_HITTEST hit = GetCurrentView ()->HitTest (point);

	if (hit == CBCGPPlannerView::BCGP_PLANNER_HITTEST_ICON_UP ||
		hit == CBCGPPlannerView::BCGP_PLANNER_HITTEST_ICON_DOWN ||
		hit == CBCGPPlannerView::BCGP_PLANNER_HITTEST_DAY_CAPTION ||
		hit == CBCGPPlannerView::BCGP_PLANNER_HITTEST_WEEK_CAPTION)
	{
		if (m_bNotifyParent)
		{
			UINT message = 0;
			WPARAM wparam = 0;

			if (hit == CBCGPPlannerView::BCGP_PLANNER_HITTEST_DAY_CAPTION)
			{
				message = BCGP_PLANNER_DAYCAPTION_CLICK;
			}
			else if (hit == CBCGPPlannerView::BCGP_PLANNER_HITTEST_WEEK_CAPTION)
			{
				message = BCGP_PLANNER_WEEKCAPTION_CLICK;
			}
			else
			{
				message = BCGP_PLANNER_ICONUPDOWN_CLICK;
				wparam  = hit;
			}

			if (NotifyMessage (message, wparam, 0) != 0L)
			{
				return;
			}
		}

		if (m_Type == BCGP_PLANNER_TYPE_WEEK ||
			m_Type == BCGP_PLANNER_TYPE_MONTH)
		{
			SetRedraw (FALSE);

			CBCGPPlannerView* pCurrentView = GetCurrentView ();

			if (hit == CBCGPPlannerView::BCGP_PLANNER_HITTEST_WEEK_CAPTION &&
				m_Type == BCGP_PLANNER_TYPE_MONTH)
			{
				int week = pCurrentView->GetWeekFromPoint (point);
				if (week != -1)
				{
					COleDateTime dt1 (pCurrentView->GetDateStart ());
					dt1 += COleDateTimeSpan (week * 7, 0, 0, 0);
					COleDateTime dt2 (dt1);
					dt2 += COleDateTimeSpan (6, 0, 0, 0);

					SetDateInterval (dt1, dt2);
				}
			}
			else
			{
				COleDateTime dt (pCurrentView->GetDateFromPoint (point));

				BOOL bSetDate = dt != GetDate ();
				SetType (BCGP_PLANNER_TYPE_DAY, !bSetDate);

				if (bSetDate)
				{
					SetDate (dt);
				}
			}	

			SetRedraw (TRUE);
			UpdateWindow ();

			return;
		}
	}

	COleDateTime dtOld (GetCurrentView ()->GetDate ());

	if (!GetCurrentView ()->OnLButtonDown (nFlags, point))
	{
		CBCGPWnd::OnLButtonDown(nFlags, point);
	}
	else
	{
		StartCapture ();
	}

	if (GetCurrentView ()->GetDate () != dtOld)
	{
		OnDateChanged ();
	}
}

void CBCGPPlannerManagerCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	BOOL bWasCaptured = IsCaptured ();

	COleDateTime dateStart = GetCurrentView ()->m_dtCaptureStart;
	COleDateTime dateFinish = GetCurrentView ()->m_dtCaptureCurrent;

	StopDragDrop ();

	StopCapture ();

	if (!GetCurrentView ()->OnLButtonUp (nFlags, point))
	{
		CBCGPWnd::OnLButtonUp(nFlags, point);
	}

	if (bWasCaptured && dateStart != dateFinish && m_lsSelectedApps.IsEmpty ())
	{
		if (dateStart > dateFinish)
		{
			COleDateTime dateTmp = dateFinish;
			dateFinish = dateStart;
			dateStart = dateTmp;
		}

		if (m_Type == BCGP_PLANNER_TYPE_DAY ||
			m_Type == BCGP_PLANNER_TYPE_WORK_WEEK ||
			m_Type == BCGP_PLANNER_TYPE_MULTI)
		{
			dateFinish += GetCurrentView ()->GetMinimumSpan ();
		}

		OnSelectTimeInterval (dateStart, dateFinish);
	}
}

void CBCGPPlannerManagerCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (IsCaptured ())
	{
		m_ptCaptureCurrent = point;
	}

	if (IsCaptured () && CanStartDragDrop () && (nFlags & MK_LBUTTON) != 0)
	{
		StartDragDrop ();
	}

	BOOL bHandled = GetCurrentView ()->OnMouseMove (nFlags, point);

	if (!bHandled)
	{
		CBCGPWnd::OnMouseMove (nFlags, point);
	}
}

BOOL CBCGPPlannerManagerCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (GetCurrentView ()->OnSetCursor (pWnd, nHitTest, message))
	{
		return TRUE;
	}
	
	return CBCGPWnd::OnSetCursor (pWnd, nHitTest, message);
}

void CBCGPPlannerManagerCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus ();
	
	if (!GetCurrentView ()->OnRButtonDown (nFlags, point))
	{
		CBCGPWnd::OnRButtonDown(nFlags, point);
	}
}

void CBCGPPlannerManagerCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	COleDateTime dtOld (GetCurrentView ()->GetDate ());

	if (!GetCurrentView ()->OnKeyDown (nChar, nRepCnt, nFlags))
	{
		if (nChar == VK_TAB)
		{
			const int nCount = (int) m_arQueryApps.GetSize ();

			if (nCount > 0)
			{
				CBCGPAppointment *pApp = NULL;

				if (GetSelectedAppointmentsCount () > 1)
				{
					pApp = m_lsSelectedApps.GetTail ();
					ClearAppointmentSelection (FALSE);
				}
				else
				{
					if (GetSelectedAppointmentsCount () == 1)
					{
						pApp = m_lsSelectedApps.GetHead ();
					}

					if (pApp == NULL)
					{
						pApp = m_arQueryApps[0];
					}
					else
					{
						CBCGPAppointment *pApp2 = NULL;

						for (int i = 0; i < nCount; i++)
						{
							if (pApp == m_arQueryApps[i] && (i < nCount - 1))
							{
								pApp2 = m_arQueryApps[i + 1];
							}
						}

						if (pApp != pApp2)
						{
							pApp = pApp2;
							ClearAppointmentSelection (pApp == NULL);
						}
					}
				}

				if (pApp != NULL)
				{
					SelectAppointment (pApp, !pApp->IsSelected (), TRUE);
					EnsureVisible (pApp, TRUE);
				}
			}
		}

		CBCGPWnd::OnKeyDown(nChar, nRepCnt, nFlags);
	}

	if (GetCurrentView ()->GetDate () != dtOld)
	{
		OnDateChanged ();
	}

	if (m_bNotifyParent)
	{
		NotifyMessage (BCGP_PLANNER_KEYDOWN, nChar, MAKELPARAM(nRepCnt, nFlags));
	}
}

COleDateTime CBCGPPlannerManagerCtrl::GetDateFromPoint (const CPoint& point) const
{
	return GetCurrentView ()->GetDateFromPoint (point);
}

void CBCGPPlannerManagerCtrl::SetFirstDayOfWeek (int nDay)
{
	ASSERT_VALID (this);
	ASSERT (nDay >= 0 && nDay < 7);

	if (m_nWeekStart != nDay + 1)
	{
		m_nWeekStart = nDay + 1;

		CBCGPPlannerView* pView = GetCurrentView ();

		COleDateTimeSpan span (pView->GetDateEnd() - pView->GetDateStart ());
		pView->m_DateStart = pView->CalculateDateStart (pView->GetDate ());
		pView->m_DateEnd   = pView->GetDateStart () + span;
		SetDate (pView->GetDate (), FALSE);

		QueryAppointments ();

		AdjustLayout ();

		UpdateCalendars ();
	}
}

int CBCGPPlannerManagerCtrl::GetFirstDayOfWeek ()
{
	return m_nWeekStart - 1;
}

void CBCGPPlannerManagerCtrl::SetTimeDelta (CBCGPPlannerView::BCGP_PLANNER_TIME_DELTA delta,
											BOOL bRedraw /*= TRUE*/)
{
	ASSERT(CBCGPPlannerView::BCGP_PLANNER_TIME_DELTA_FIRST <= delta);
	ASSERT(delta <= CBCGPPlannerView::BCGP_PLANNER_TIME_DELTA_LAST);

	if (m_TimeDelta != delta)
	{
		m_TimeDelta = delta;

		AdjustLayout (bRedraw);
	}
}

CBCGPPlannerView::BCGP_PLANNER_HITTEST
CBCGPPlannerManagerCtrl::HitTest (const CPoint& point) const
{
	return GetCurrentView ()->HitTest (point);
}

CBCGPPlannerView::BCGP_PLANNER_HITTEST
CBCGPPlannerManagerCtrl::HitTestArea (const CPoint& point) const
{
	return GetCurrentView ()->HitTestArea (point);
}

CBCGPPlannerView::BCGP_PLANNER_HITTEST
CBCGPPlannerManagerCtrl::HitTestAppointment (const CPoint& point) const
{
	return GetCurrentView ()->HitTestAppointment (point);
}

void CBCGPPlannerManagerCtrl::OnTimer(UINT_PTR nIDEvent) 
{
	if (!GetCurrentView ()->OnTimer (nIDEvent))
	{
		CBCGPWnd::OnTimer(nIDEvent);
	}
}

void CBCGPPlannerManagerCtrl::OnTimeChange() 
{
	if (m_Type == BCGP_PLANNER_TYPE_DAY || 
		m_Type == BCGP_PLANNER_TYPE_WORK_WEEK ||
		m_Type == BCGP_PLANNER_TYPE_MULTI)
	{
		GetCurrentView ()->OnTimer (CBCGPPlannerViewDay::BCGP_PLANNER_TIMER_EVENT);
	}
}

void CBCGPPlannerManagerCtrl::SetWorkingHourInterval (int nFirstHour, int nLastHour, 
													  BOOL bRedraw /*= TRUE*/)
{
	ASSERT (nFirstHour < nLastHour);

	ASSERT (0 <= nFirstHour);
	ASSERT (nLastHour <= 24);

	SetWorkingHourMinuteInterval ((double)nFirstHour, (double)nLastHour, bRedraw);
}

void CBCGPPlannerManagerCtrl::SetWorkingHourMinuteInterval (double dStart, double dEnd, 
															BOOL bRedraw /*= TRUE*/)
{
	ASSERT (dStart < dEnd);

	ASSERT (0 <= (int)dStart);
	ASSERT ((int)dEnd <= 24);

	ASSERT (0 <= (int)((dStart - (int)(dStart)) * 100.0));
	ASSERT ((int)((dStart - (int)(dStart)) * 100.0) <= 59);

	ASSERT (0 <= (int)((dEnd - (int)(dEnd)) * 100.0));
	ASSERT (((int)dEnd < 24 && (int)((dEnd - (int)(dEnd)) * 100.0) <= 59) ||
			((int)dEnd == 24 && (int)((dEnd - (int)(dEnd)) * 100.0) == 0));

	if (m_nFirstWorkingHour != dStart || m_nLastWorkingHour != dEnd)
	{
		m_nFirstWorkingHour = dStart;
		m_nLastWorkingHour	= dEnd;

		AdjustLayout (bRedraw);
	}
}

void CBCGPPlannerManagerCtrl::SetViewHourInterval (int nFirstHour, int nLastHour, 
													  BOOL bRedraw /*= TRUE*/)
{
	ASSERT (nFirstHour < nLastHour);

	ASSERT (0 <= nFirstHour);
	ASSERT (nLastHour <= 23);

	if (m_nFirstViewHour != nFirstHour || m_nLastViewHour != nLastHour)
	{
		m_nFirstViewHour = nFirstHour;
		m_nLastViewHour  = nLastHour;

		AdjustLayout (bRedraw);
	}
}

void CBCGPPlannerManagerCtrl::SetPrintHourInterval (int nFirstHour, int nLastHour)
{
	ASSERT (nFirstHour < nLastHour);

	ASSERT (0 <= nFirstHour);
	ASSERT (nLastHour <= 23);

	if (m_nFirstPrintHour != nFirstHour || m_nLastPrintHour != nLastHour)
	{
		m_nFirstPrintHour = nFirstHour;
		m_nLastPrintHour  = nLastHour;
	}
}

void CBCGPPlannerManagerCtrl::QueryAppointments ()
{
	COleDateTime date1 (GetCurrentView()->GetDateStart ());
	COleDateTime date2 (GetCurrentView()->GetDateEnd ());

	ASSERT (date1 <= date2);

	ClearAppointmentSelection (FALSE);

	ClearQuery ();

	QueryAppointments (m_arQueryApps, date1, date2);
}

void CBCGPPlannerManagerCtrl::QueryAppointments (XBCGPAppointmentArray& ar, 
												 const COleDateTime& date1, 
												 const COleDateTime& date2) const
{
	CBCGPAppointmentBaseStorage* pAppsStorage = GetStorage ();
	ASSERT_VALID (pAppsStorage);

	pAppsStorage->Query (ar, date1, date2);
}

void CBCGPPlannerManagerCtrl::ClearAppointmentSelection (BOOL bRedraw)
{
	int nOldCount = GetSelectedAppointmentsCount ();

	if (nOldCount > 0)
	{
		BOOL bNotify = m_bNotifyParent;

		while (m_lsSelectedApps.GetTailPosition () != NULL)
		{
			CBCGPAppointment* pApp = m_lsSelectedApps.GetTail ();

			pApp->SetSelected (FALSE);
			m_lsSelectedApps.RemoveTail ();

			if (bNotify)
			{
				NotifyMessage (BCGP_PLANNER_SELECT_APPOINTMENT, (WPARAM) FALSE, (LPARAM) pApp);
			}
		}

		if (bRedraw)
		{
			RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}
}

void CBCGPPlannerManagerCtrl::SelectAppointment (CBCGPAppointment* pApp, 
												 BOOL bSelect, BOOL bRedraw)
{
	ASSERT_VALID (pApp);

#ifdef _DEBUG
	if (bSelect)
	{
		ASSERT (!pApp->IsSelected ());
	}
	else
	{
		ASSERT (pApp->IsSelected ());
	}
#endif

	int nOldCount = GetSelectedAppointmentsCount ();

	POSITION pos = m_lsSelectedApps.Find (pApp);
	BOOL bOperation = (bSelect && (pos == NULL)) || (!bSelect && (pos != NULL));

	if (m_bNotifyParent && bOperation)
	{
		bOperation = NotifyMessage (BCGP_PLANNER_BEFORE_SELECT_APPOINTMENT, (WPARAM) bSelect, (LPARAM) pApp) == 0L;
	}

	if (bOperation)
	{
		pApp->SetSelected (bSelect);

		if (bSelect)
		{
			m_lsSelectedApps.AddTail (pApp);
		}
		else
		{
			m_lsSelectedApps.RemoveAt (pos);
		}

		if (m_bNotifyParent)
		{
			NotifyMessage (BCGP_PLANNER_SELECT_APPOINTMENT, (WPARAM) bSelect, (LPARAM) pApp);
		}
	}

	if (GetSelectedAppointmentsCount () != nOldCount && bRedraw)
	{
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void CBCGPPlannerManagerCtrl::RestoreAppointmentSelection 
	(XBCGPAppointmentList& lst, BOOL bRedraw)
{
	ClearAppointmentSelection (FALSE);

	if (lst.GetCount () > 0)
	{
		for (int i = 0; i < m_arQueryApps.GetSize (); i++)
		{
			if (lst.Find (m_arQueryApps[i]) != NULL)
			{
				SelectAppointment (m_arQueryApps[i], TRUE, FALSE);
			}
		}
	}

	if (GetSelectedAppointmentsCount () > 0 && bRedraw)
	{
		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

BOOL CBCGPPlannerManagerCtrl::SetUpDownIcons (UINT nResID, COLORREF clrTransparent)
{
	if (nResID == 0)
	{
		CBCGPLocalResource localRes;
		return SetUpDownIcons (MAKEINTRESOURCE (IDB_BCGBARRES_PLANNER_APP_UPDOWN));
	}

	return SetUpDownIcons (MAKEINTRESOURCE (nResID), clrTransparent);
}

BOOL CBCGPPlannerManagerCtrl::SetUpDownIcons (LPCTSTR szResID, COLORREF clrTransparent)
{
	if (szResID == NULL)
	{
		return FALSE;
	}

	HBITMAP hBmp = (HBITMAP)::LoadImage (AfxFindResourceHandle (szResID, RT_BITMAP), 
		szResID, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);

	if (hBmp == NULL)
	{
		return FALSE;
	}

	CBitmap bmp;
	bmp.Attach (hBmp);

	if (bmp.GetSafeHandle () == NULL)
	{
		return FALSE;
	}

	BITMAP bm;
	ZeroMemory (&bm, sizeof (BITMAP));

	bmp.GetBitmap (&bm);

	UINT nFlags = clrTransparent == (COLORREF)-1 ? 0 : ILC_MASK;

	switch (bm.bmBitsPixel)
	{
	case 4 :
		nFlags |= ILC_COLOR4;
		break;
	case 8 :
		nFlags |= ILC_COLOR8;
		break;
	case 16:
		nFlags |= ILC_COLOR16;
		break;
	case 24:
		nFlags |= ILC_COLOR24;
		break;
	case 32:
		nFlags |= ILC_COLOR32;
		if (clrTransparent == (COLORREF)-1)
		{
			nFlags |= ILC_MASK;
		}
		break;
	default:
		ASSERT (FALSE);
	}

	if (m_ilUpDown.GetSafeHandle () != NULL)
	{
		m_ilUpDown.DeleteImageList ();
	}

	m_szUpDown.cx = bm.bmWidth / 2;
	m_szUpDown.cy = bm.bmHeight;

	m_ilUpDown.Create (m_szUpDown.cx, m_szUpDown.cy, nFlags, 2, 0);

	if ((nFlags & ILC_COLOR32) == ILC_COLOR32 &&
		clrTransparent == (COLORREF)-1)
	{
		m_ilUpDown.Add (&bmp, (CBitmap*) NULL);
	}
	else
	{
		m_ilUpDown.Add (&bmp, clrTransparent);
	}

	return TRUE;
}

HICON CBCGPPlannerManagerCtrl::GetUpDownIcon(int nType)
{
	ASSERT (0 <= nType && nType <= 1);

	HICON hIcon = m_ilUpDown.ExtractIcon (nType);
	return hIcon;
}

BOOL CBCGPPlannerManagerCtrl::UpdateChangeOperation 
	(BCGP_PLANNER_CHANGE_OPERATION operation, BOOL bResult /*= FALSE*/)
{
	BOOL bRes = FALSE;

	if (m_ChangeOperation != operation)
	{
		if (m_ChangeOperation == BCGP_PLANNER_CHANGE_OPERATION_NONE)
		{
			m_ChangeOperation = operation;
			if(m_bNotifyParent)
			{
				bRes = NotifyMessage (BCGP_PLANNER_BEGIN_CHANGE_OPERATION, 
					(WPARAM)m_ChangeOperation, (LPARAM)FALSE) == 0L;
			}

			if(!bRes)
			{
				m_ChangeOperation = BCGP_PLANNER_CHANGE_OPERATION_NONE;
			}
		}
		else if (operation == BCGP_PLANNER_CHANGE_OPERATION_NONE)
		{
			if(m_bNotifyParent)
			{
				NotifyMessage (BCGP_PLANNER_END_CHANGE_OPERATION, 
					(WPARAM)m_ChangeOperation, (LPARAM)bResult, TRUE);
			}
			m_ChangeOperation = operation;
		}
	}

	return bRes;
}

BOOL CBCGPPlannerManagerCtrl::EditCut ()
{
	CWnd* pWnd = GetDlgItem (BCGP_PLANNER_ID_INPLACE);

	if (pWnd != NULL)
	{
		pWnd->SendMessage (WM_CUT, 0, 0);
		return FALSE;
	}

	if (!IsEditCutEnabled ())
	{
		return FALSE;
	}

	if (!UpdateChangeOperation(BCGP_PLANNER_CHANGE_OPERATION_CUT))
	{
		return FALSE;
	}

	BOOL bRes = EditCopy ();

	if (bRes)
	{
		RemoveSelectedAppointments (TRUE, TRUE);
	}

	UpdateChangeOperation(BCGP_PLANNER_CHANGE_OPERATION_NONE, bRes);

	return bRes;
}

BOOL CBCGPPlannerManagerCtrl::IsEditCutEnabled () const
{
	return IsEditCopyEnabled () && !IsReadOnly ();
}

BOOL CBCGPPlannerManagerCtrl::SerializeTo (CFile& file)
{
	XBCGPAppointmentArray ar;

	POSITION pos = m_lsSelectedApps.GetHeadPosition ();
	while (pos != NULL)
	{
		CBCGPAppointment* pApp = m_lsSelectedApps.GetNext (pos);
		ASSERT_VALID (pApp);

		ar.Add (pApp);
	}

	return CBCGPPlannerManagerCtrl::SerializeTo (file, ar);
}

BOOL CBCGPPlannerManagerCtrl::SerializeTo (CFile& file, XBCGPAppointmentArray& ar)
{
	const int nCount = (int) ar.GetSize ();

	if (nCount == 0)
	{
		return FALSE;
	}

	CArchive Archive (&file, CArchive::store);

	BOOL bResult = CBCGPPlannerManagerCtrl::SerializeTo (Archive, ar);

	Archive.Close();

	return bResult;
}

BOOL CBCGPPlannerManagerCtrl::SerializeTo (CArchive& Archive, XBCGPAppointmentArray& ar)
{
	const int nCount = (int) ar.GetSize ();

	if (nCount == 0)
	{
		return FALSE;
	}

	int i = 0;
	int nCountStored = 0;
	for (i = 0; i < nCount; i++)
	{
		CBCGPAppointment* pApp = ar[i];
		ASSERT_VALID (pApp);

		if (pApp != NULL && pApp->CanBeStored ())
		{
			nCountStored++;
		}
	}

	if (nCountStored == 0)
	{
		return FALSE;
	}

	BOOL bStored = FALSE;

	try
	{
		Archive << nCountStored;

		for (i = 0; i < nCount; i++)
		{
			CBCGPAppointment* pApp = ar[i];
			ASSERT_VALID (pApp);

			if (pApp != NULL && pApp->CanBeStored ())
			{
				Archive << pApp;
				bStored = TRUE;
			}
		}
	}
	catch (CArchiveException* pEx)
	{
		TRACE(_T("CBCGPPlannerManagerCtrl::SerializeTo. Archive exception\r\n"));
		pEx->Delete ();
		return FALSE;
	}

	return bStored;
}

BOOL CBCGPPlannerManagerCtrl::SerializeFrom (CFile& file, const COleDateTime& dtTo)
{
	XBCGPAppointmentArray ar;

	BOOL bResult = CBCGPPlannerManagerCtrl::SerializeFrom (file, ar, m_Type, dtTo);

	if (!bResult)
	{
		return FALSE;
	}

	CBCGPPlannerView* pView = GetCurrentView ();
	ASSERT_VALID(pView);

	BOOL bAllDay = FALSE;
	BOOL bNeedAdjust = m_Type == BCGP_PLANNER_TYPE_DAY || 
					   m_Type == BCGP_PLANNER_TYPE_WORK_WEEK ||
					   m_Type == BCGP_PLANNER_TYPE_MULTI;

	for (int i = 0; i < ar.GetSize (); i++)
	{
		CBCGPAppointment* pApp = ar[i];

		if (pApp->IsRecurrenceClone ())
		{
			pApp->RemoveRecurrence ();
		}

		pApp->SetResourceID (CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID);
		AddAppointment (pApp, FALSE, FALSE);

		if (bNeedAdjust && !bAllDay)
		{
			bAllDay = pApp->IsAllDay () || pApp->IsMultiDay ();
		}
	}

	if (!IsDragDrop ())
	{
		ClearAppointmentSelection (FALSE);
		QueryAppointments ();

		if (bNeedAdjust && bAllDay)
		{
			pView->AdjustLayout (FALSE);
		}
		else
		{
			pView->AdjustAppointments ();
		}

		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	return bResult;
}

BOOL CBCGPPlannerManagerCtrl::SerializeFrom (CFile& file, 
	XBCGPAppointmentArray& ar, BCGP_PLANNER_TYPE type, const COleDateTime& dtTo)
{
	CArchive Archive (&file, CArchive::load);

	BOOL bResult = CBCGPPlannerManagerCtrl::SerializeFrom (Archive, ar, type, dtTo);

	Archive.Close();

	return bResult;
}

BOOL CBCGPPlannerManagerCtrl::SerializeFrom (CArchive& Archive, 
	XBCGPAppointmentArray& ar, BCGP_PLANNER_TYPE type, const COleDateTime& dtTo)
{
	try
	{
		int nCount;
		Archive >> nCount;

		if (nCount == 0)
		{
			return FALSE;
		}

		ar.RemoveAll ();
		int i = 0;

		for (i = 0; i < nCount; i++)
		{
			CBCGPAppointment* pApp = NULL;
			Archive >> pApp;

			ASSERT_VALID (pApp);

			if (pApp != NULL)
			{
				ar.Add (pApp);
			}
		}

		if (dtTo != COleDateTime ())
		{
			// correct interval by date TO
			nCount = (int) ar.GetSize ();

			COleDateTime dtMin;

			for (i = 0; i < nCount; i++)
			{
				COleDateTime dt (ar[i]->GetStart ());

				if (i == 0)
				{
					dtMin = dt;
				}
				else if (dtMin > dt)
				{
					dtMin = dt;
				}
			}

			BOOL bAdd = dtMin < dtTo;
			COleDateTimeSpan spanTo;

			if (type == BCGP_PLANNER_TYPE_DAY || type == BCGP_PLANNER_TYPE_WORK_WEEK ||
				type == BCGP_PLANNER_TYPE_MULTI)
			{
				dtMin = COleDateTime (dtMin.GetYear (), dtMin.GetMonth (), 
					dtMin.GetDay (), dtMin.GetHour (), 0, 0);
			}
			else if (type == BCGP_PLANNER_TYPE_WEEK || type == BCGP_PLANNER_TYPE_MONTH)
			{
				dtMin = COleDateTime (dtMin.GetYear (), dtMin.GetMonth (), 
					dtMin.GetDay (), 0, 0, 0);
			}

			if (bAdd)
			{
				spanTo = dtTo - dtMin;
			}
			else
			{
				spanTo = dtMin - dtTo;
			}

			MoveAppointments (ar, spanTo, bAdd);
		}
	}
	catch (CArchiveException* pEx)
	{
		TRACE(_T("CBCGPPlannerManagerCtrl::SerializeTo. Archive exception\r\n"));
		pEx->Delete ();

		if (ar.GetSize () > 0)
		{
			for (int i = 0; i < ar.GetSize (); i++)
			{
				if (ar[i] != NULL)
				{
					delete ar[i];
				}
			}

			ar.RemoveAll ();
		}
	}

	return ar.GetSize () > 0;
}

void CBCGPPlannerManagerCtrl::MoveAppointments (XBCGPAppointmentArray& ar, const COleDateTimeSpan& spanTo, BOOL bAdd)
{
	const int nCount = (int) ar.GetSize ();

	if (nCount == 0 || spanTo.GetStatus () != COleDateTimeSpan::valid)
	{
		return;
	}

	for (int i = 0; i < nCount; i++)
	{
		CBCGPAppointment* pApp = ar[i];

		COleDateTime dtS (pApp->GetStart ());
		COleDateTime dtF (pApp->GetFinish ());

		if (bAdd)
		{
			dtS += spanTo;
			dtF += spanTo;
		}
		else
		{
			dtS -= spanTo;
			dtF -= spanTo;
		}

		pApp->SetInterval (dtS, dtF);
	}
}

BOOL CBCGPPlannerManagerCtrl::EditCopy ()
{
	CWnd* pWnd = GetDlgItem (BCGP_PLANNER_ID_INPLACE);

	if (pWnd != NULL)
	{
		pWnd->SendMessage (WM_COPY, 0, 0);
		return FALSE;
	}

	if (!IsEditCopyEnabled ())
	{
		return FALSE;
	}

	try
	{
		if (!AfxGetMainWnd ()->OpenClipboard ())
		{
			return FALSE;
		}

		if (!::EmptyClipboard ())
		{
			::CloseClipboard ();
			return FALSE;
		}

		CString strText;

		HGLOBAL hClipbuffer = NULL;

		{
			CSharedFile globFile;

			SerializeTo (globFile);

			POSITION pos = m_lsSelectedApps.GetHeadPosition ();
			while (pos != NULL)
			{
				CBCGPAppointment* pApp = m_lsSelectedApps.GetNext (pos);
				ASSERT_VALID (pApp);

				if (pApp != NULL && pApp->CanBeStored ())
				{
					CString strClip = pApp->GetClipboardText ();
					if (!strClip.IsEmpty ())
					{
						if (!strText.IsEmpty ())
						{
							strText += _T("\r\n");
						}

						strText += strClip;
					}
				}
			}

			globFile.SeekToBegin ();

			hClipbuffer = ::GlobalAlloc (GMEM_DDESHARE, (SIZE_T)globFile.GetLength ());
			if (hClipbuffer != NULL)
			{
				LPBYTE lpData = (LPBYTE)::GlobalLock (hClipbuffer);

				globFile.Read (lpData, (UINT) globFile.GetLength ());

				::GlobalUnlock (hClipbuffer);
			}

			globFile.Close ();
		}

		HANDLE hclipData = ::SetClipboardData (GetClipboardFormat (), 
			hClipbuffer);

		if (hclipData != NULL)
		{
			::SetClipboardData (CF_OWNERDISPLAY, NULL);

			if (!strText.IsEmpty ())
			{
				hClipbuffer = ::GlobalAlloc (GMEM_DDESHARE, (strText.GetLength () + 1) * sizeof (TCHAR));
				LPTSTR lpszBuffer = (LPTSTR) ::GlobalLock (hClipbuffer);

				lstrcpy (lpszBuffer, (LPCTSTR) strText);

				::GlobalUnlock (hClipbuffer);
				::SetClipboardData (_TCF_TEXT, hClipbuffer);
			}
			else
			{
				::SetClipboardData (_TCF_TEXT, NULL);
			}
		}

		::CloseClipboard ();		

		return hclipData != NULL;
	}
	catch (...)
	{
		CBCGPLocalResource localRes;
		AfxMessageBox (IDP_BCGBARRES_INTERLAL_ERROR);
	}

	return FALSE;
}

BOOL CBCGPPlannerManagerCtrl::IsEditCopyEnabled () const
{
	CWnd* pWnd = GetDlgItem (BCGP_PLANNER_ID_INPLACE);

	if (pWnd != NULL)
	{
		return TRUE;
	}

	return GetClipboardFormat () != 0 && GetSelectedAppointmentsCount () > 0;
}

BOOL CBCGPPlannerManagerCtrl::EditPaste (const COleDateTime& dtTo)
{
	CWnd* pWnd = GetDlgItem (BCGP_PLANNER_ID_INPLACE);

	if (pWnd != NULL)
	{
		pWnd->SendMessage (WM_PASTE, 0, 0);
		return FALSE;
	}

	if (!IsEditPasteEnabled ())
	{
		return FALSE;
	}

	if(!UpdateChangeOperation(BCGP_PLANNER_CHANGE_OPERATION_PASTE))
	{
		return FALSE;
	}

	BOOL bRes = FALSE;

	try
	{
		if (!AfxGetMainWnd ()->OpenClipboard ())
		{
			return FALSE;
		}

		HGLOBAL hClipbuffer = ::GetClipboardData(GetClipboardFormat ());

		if (hClipbuffer != NULL) 
		{ 
			LPBYTE lpData = (LPBYTE)::GlobalLock(hClipbuffer);

			if (lpData != NULL) 
			{ 
				CSharedFile globFile;
				globFile.Attach (lpData, (UINT)::GlobalSize (hClipbuffer));

				COleDateTime dt (dtTo);
				if (dt == COleDateTime ())
				{
					dt = GetCurrentView ()->GetSelectionStart ();
				}

				bRes = SerializeFrom (globFile, dt);

				::GlobalUnlock(hClipbuffer);

				if (bRes)
				{
					UpdateCalendarsState ();
				}
			} 
		} 

		::CloseClipboard ();
	}
	catch (...)
	{
		AfxMessageBox (IDP_BCGBARRES_INTERLAL_ERROR);
	}

	UpdateChangeOperation(BCGP_PLANNER_CHANGE_OPERATION_NONE, bRes);

	return bRes;
}

BOOL CBCGPPlannerManagerCtrl::IsEditPasteEnabled () const
{
	if (IsReadOnly ())
	{
		return FALSE;
	}

	CWnd* pWnd = GetDlgItem (BCGP_PLANNER_ID_INPLACE);

	if (pWnd != NULL)
	{
		return ::IsClipboardFormatAvailable (CF_TEXT);
	}

	BOOL bRes = GetClipboardFormat () != 0;

	if (bRes)
	{
		bRes = ::IsClipboardFormatAvailable (GetClipboardFormat ());
	}

	return bRes;
}

BOOL CBCGPPlannerManagerCtrl::IsAppointmentInSelection (const CBCGPAppointment* pApp) const
{
	ASSERT_VALID (pApp);
	return m_lsSelectedApps.Find (const_cast<CBCGPAppointment*>(pApp)) != NULL;
}

CBCGPAppointment*
CBCGPPlannerManagerCtrl::GetAppointmentFromPoint (const CPoint& point)
{
	return GetCurrentView ()->GetAppointmentFromPoint (point);
}

CBCGPAppointment* CBCGPPlannerManagerCtrl::GetRecurrence (DWORD ID) const
{
	CBCGPAppointmentBaseStorage* pAppsStorage = GetStorage ();
	ASSERT_VALID (pAppsStorage);

	if (IsMultiResourceStorage ())
	{
		CBCGPAppointmentBaseMultiStorage* pMultiStorage = 
			(CBCGPAppointmentBaseMultiStorage*)pAppsStorage;

		if (pMultiStorage->GetCurrentResourceID () == 
			CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID)
		{
			UINT nResourceID = pMultiStorage->GetCurrentResourceID ();
			pMultiStorage->SetCurrentResourceID (GetCurrentResourceID ());

			CBCGPAppointment* pApp = pMultiStorage->GetRecurrence (ID);

			pMultiStorage->SetCurrentResourceID (nResourceID);

			return pApp;
		}
	}

	return pAppsStorage->GetRecurrence (ID);
}

int CBCGPPlannerManagerCtrl::GetSelectedAppointmentsCount () const
{
	return (int) m_lsSelectedApps.GetCount ();
}

POSITION CBCGPPlannerManagerCtrl::GetFirstSelectedAppointment () const
{
	return m_lsSelectedApps.GetHeadPosition ();
}

POSITION CBCGPPlannerManagerCtrl::GetLastSelectedAppointment () const
{
	return m_lsSelectedApps.GetTailPosition ();
}

const CBCGPAppointment* CBCGPPlannerManagerCtrl::GetNextSelectedAppointment (POSITION& pos) const
{
	return m_lsSelectedApps.GetNext (pos);
}

const CBCGPAppointment* CBCGPPlannerManagerCtrl::GetPrevSelectedAppointment (POSITION& pos) const
{
	return m_lsSelectedApps.GetPrev (pos);
}

int CBCGPPlannerManagerCtrl::GetQueryedAppointmentsCount () const
{
	return (int) m_arQueryApps.GetSize ();
}

int CBCGPPlannerManagerCtrl::GetQueryedAppointments (XBCGPAppointmentArray& ar) const
{
	ar.RemoveAll ();
	ar.Copy (m_arQueryApps);

	return (int) ar.GetSize ();
}

CBCGPPlannerView* CBCGPPlannerManagerCtrl::GetView (BCGP_PLANNER_TYPE type)
{
	ASSERT (BCGP_PLANNER_TYPE_FIRST <= type);
	ASSERT (type <= BCGP_PLANNER_TYPE_LAST);

	switch (type)
	{
	case BCGP_PLANNER_TYPE_DAY:
		return m_pViews [0];

	case BCGP_PLANNER_TYPE_WORK_WEEK:
		return m_pViews [1];

	case BCGP_PLANNER_TYPE_WEEK:
		return m_pViews [2];

	case BCGP_PLANNER_TYPE_MONTH:
		return m_pViews [3];

	case BCGP_PLANNER_TYPE_MULTI:
		return m_pViews [4];
	}

	return NULL;
}

const CBCGPPlannerView* CBCGPPlannerManagerCtrl::GetView (BCGP_PLANNER_TYPE type) const
{
	ASSERT (BCGP_PLANNER_TYPE_FIRST <= type);
	ASSERT (type <= BCGP_PLANNER_TYPE_LAST);

	switch (type)
	{
	case BCGP_PLANNER_TYPE_DAY:
		return m_pViews [0];

	case BCGP_PLANNER_TYPE_WORK_WEEK:
		return m_pViews [1];

	case BCGP_PLANNER_TYPE_WEEK:
		return m_pViews [2];

	case BCGP_PLANNER_TYPE_MONTH:
		return m_pViews [3];

	case BCGP_PLANNER_TYPE_MULTI:
		return m_pViews [4];
	}

	return NULL;
}

void CBCGPPlannerManagerCtrl::StartDragDrop ()
{
	if (GetCurrentView () == NULL)
	{
		return;
	}

	if (!m_bDragDrop && CanStartDragDrop ())
	{
		if (!UpdateChangeOperation(BCGP_PLANNER_CHANGE_OPERATION_DRAG))
		{
			return;
		}

		COleDataSource* pSrcItem = new COleDataSource;

		try
		{
			CSharedFile globFile;
			
			m_bDragDrop = SerializeTo (globFile);

			if (m_bDragDrop)
			{
				pSrcItem->CacheGlobalData (GetClipboardFormat (), 
					globFile.Detach());
			}
		}
		catch (COleException* pEx)
		{
			TRACE(_T("CBCGPPlannerManagerCtrl::StartDragDrop. OLE exception: %x\r\n"), pEx->m_sc);
			pEx->Delete ();

			m_bDragDrop = FALSE;
		}
		catch (CNotSupportedException *pEx)
		{
			TRACE(_T("CBCGPPlannerManagerCtrl::StartDragDrop. \"Not Supported\" exception\r\n"));
			pEx->Delete ();

			m_bDragDrop = FALSE;
		}

		if (!m_bDragDrop)
		{
			pSrcItem->InternalRelease();
			return;
		}

		StopCapture ();

		GetCurrentView ()->StartCapture ();

		m_dragEffect = DROPEFFECT_NONE;

		m_DropSource.Empty ();

		DWORD dwEffects = DROPEFFECT_COPY | DROPEFFECT_SCROLL;

		if (!IsReadOnly ())
		{
			dwEffects |= DROPEFFECT_MOVE;
		}

		DROPEFFECT dropEffect = pSrcItem->DoDragDrop (dwEffects, NULL, &m_DropSource);

		BOOL bRes = dropEffect == DROPEFFECT_MOVE || dropEffect == DROPEFFECT_COPY;

		GetCurrentView ()->StopCapture ();

		StopDragDrop ();

#ifdef BCGP_PLANNER_DND_TO_ANOTHER_PLANNER
		BOOL bRemove = dropEffect == DROPEFFECT_MOVE && 
			GetSelectedAppointmentsCount () > 0;

		if (bRemove)
		{
			 RemoveSelectedAppointments (FALSE, FALSE);
		}
#endif

		GetCurrentView ()->ClearDragedAppointments ();

#ifdef BCGP_PLANNER_DND_TO_ANOTHER_PLANNER
		if (bRemove)
		{
			ClearAppointmentSelection (FALSE);
			QueryAppointments ();

			AdjustLayout (TRUE);

			UpdateCalendarsState ();
		}
		else if (m_bRedrawAfterDrop && !bRes)
		{
			m_bRedrawAfterDrop = FALSE;
			
			AdjustLayout (TRUE);
		}
#endif

		if (m_bNotifyParent && bRes)
		{
			NotifyMessage (BCGP_PLANNER_DRAG_APPOINTMENTS, (WPARAM)dropEffect, 0);
		}

		UpdateChangeOperation(BCGP_PLANNER_CHANGE_OPERATION_NONE, bRes);

		if (m_bNotifyParent)
		{
			NotifyMessage (BCGP_PLANNER_UPDATED_ALL_APPOINTMENTS, 0, 0, TRUE);
		}

		pSrcItem->InternalRelease();
	}
}

void CBCGPPlannerManagerCtrl::StopDragDrop ()
{
	if (GetCurrentView () == NULL)
	{
		return;
	}

	if (m_bDragDrop)
	{
		m_DropSource.Empty ();

		m_bDragDrop = FALSE;

		if (m_dragEffect != DROPEFFECT_NONE)
		{
			m_dragEffect = DROPEFFECT_NONE;
		}
	}
}

BOOL CBCGPPlannerManagerCtrl::CanStartDragDrop () const
{
	if (!IsDragDrop () &&
		 GetCurrentView () != NULL && 
		 GetCurrentView ()->CanStartDragDrop () &&
		 GetSelectedAppointmentsCount () > 0)
	{
		CSize czScroll (::GetSystemMetrics (SM_CXDRAG),
						::GetSystemMetrics (SM_CYDRAG));
		CRect rt (m_ptCaptureStart, czScroll);
		rt.OffsetRect (CPoint (-czScroll.cx / 2, -czScroll.cy / 2));

		return !rt.PtInRect (m_ptCaptureCurrent);
	}

	return FALSE;
}

BOOL CBCGPPlannerManagerCtrl::OnDrop(COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
#ifndef BCGP_PLANNER_DND_TO_ANOTHER_PLANNER
	// can't be dropped from another planner
	if (!IsDragDrop ())
	{
		return FALSE;
	}
#endif

	if (dropEffect != DROPEFFECT_MOVE && dropEffect != DROPEFFECT_COPY)
	{
		return FALSE;
	}

	if (GetCurrentView ()->IsCaptureMatched () && dropEffect != DROPEFFECT_COPY)
	{
		return FALSE;
	}

	if (IsReadOnly ())
	{
		return FALSE;
	}

	ASSERT (pDataObject != NULL);
	ASSERT (pDataObject->IsDataAvailable (GetClipboardFormat ()));

	BOOL bRes = FALSE;

	m_dtDrop = GetCurrentView ()->m_dtCaptureCurrent;

	if(!IsDragDrop ())
	{
		if (!UpdateChangeOperation(BCGP_PLANNER_CHANGE_OPERATION_DROP))
		{
			m_dtDrop = COleDateTime ();
			AdjustLayout (TRUE);

			return FALSE;
		}
	}
	else if(m_bNotifyParent)
	{
		if (NotifyMessage (BCGP_PLANNER_BEGIN_CHANGE_OPERATION, 
			(WPARAM)BCGP_PLANNER_CHANGE_OPERATION_DROP, (LPARAM)FALSE) != 0L)
		{
			m_dtDrop = COleDateTime ();
			m_bRedrawAfterDrop = TRUE;

			return FALSE;
		}
	}

	try
	{
		GetCurrentView ()->ClearDragedAppointments ();

		CFile* pFile = pDataObject->GetFileData (GetClipboardFormat ());
		if (pFile == NULL)
		{
			m_dtDrop = COleDateTime ();

			return FALSE;
		}

		XBCGPAppointmentArray ar;

		bRes = CBCGPPlannerManagerCtrl::SerializeFrom (*pFile, ar, m_Type, 
			m_bDragDrop ? COleDateTime () : GetCurrentView ()->GetDateFromPoint (point));

		UINT nResourceID  = GetCurrentView ()->GetResourceFromPoint (point);
		UINT nResourceOld = GetCurrentView ()->GetCurrentResourceID ();

		delete pFile;

		if (bRes)
		{
			BOOL bAllDay = FALSE;
			BOOL bCanMakeAllDay = m_Type == BCGP_PLANNER_TYPE_DAY || 
								  m_Type == BCGP_PLANNER_TYPE_WORK_WEEK ||
								  m_Type == BCGP_PLANNER_TYPE_MULTI;

			if (bCanMakeAllDay)
			{
				CBCGPPlannerView::BCGP_PLANNER_HITTEST hit = 
					GetCurrentView ()->HitTestArea (point);

				bAllDay = hit == CBCGPPlannerView::BCGP_PLANNER_HITTEST_HEADER_ALLDAY ||
						  hit == CBCGPPlannerView::BCGP_PLANNER_HITTEST_HEADER ||
						  hit == CBCGPPlannerView::BCGP_PLANNER_HITTEST_HEADER_RESOURCE;
			}

			if (m_bDragDrop)
			{
				COleDateTime dtS (GetCurrentView ()->m_dtCaptureStart);

				BOOL bAdd =  m_dtDrop > dtS;

				COleDateTimeSpan spanTo;

				if (bAdd)
				{
					spanTo = m_dtDrop - dtS;
				}
				else
				{
					spanTo = dtS - m_dtDrop;
				}

				MoveAppointments (ar, spanTo, bAdd);
			}

			BOOL bNotify = m_bNotifyParent;
			BOOL bNotifyParent = FALSE;
			if (m_bDragDrop && dropEffect != DROPEFFECT_COPY)
			{
				// Drop in same view as drag, don't send add & remove
				// notifications - send update notification
				bNotifyParent = m_bNotifyParent;
				m_bNotifyParent = FALSE;
			}

			const INT_PTR c_Count = ar.GetSize ();

			if (c_Count > 0)
			{
				BOOL bRemoveRecurrence = !m_bDragDrop || dropEffect == DROPEFFECT_COPY;

				if (m_bDragDrop)
				{
					for (int i = 0; i < m_arQueryApps.GetSize (); i++)
					{
						CBCGPAppointment* pApp = m_arQueryApps[i];

						if (pApp->IsRecurrenceClone () && pApp->IsSelected () &&
							pApp->GetResourceID () == nResourceID)
						{
							m_lsSelectedApps.RemoveAt(m_lsSelectedApps.Find (pApp));
						}
					}

					if (dropEffect == DROPEFFECT_MOVE)
					{
						RemoveSelectedAppointments (FALSE, FALSE);
					}
				}

				COleDateTime dtSel1 = GetSelectionStart ();
				COleDateTime dtSel2 = GetSelectionEnd ();

				for (int i = 0; i < c_Count; i++)
				{
					CBCGPAppointment* pApp = ar[i];
					ASSERT_VALID(pApp);

					BOOL bRecurrenceClone = pApp->IsRecurrence () && pApp->IsRecurrenceClone ();

					if ((bRemoveRecurrence || pApp->GetResourceID () != nResourceID) && 
						bRecurrenceClone)
					{
						pApp->RemoveRecurrence ();
					}

					pApp->SetResourceID (nResourceID);

					if (bCanMakeAllDay)
					{
						BOOL bAllOrMulti = pApp->IsAllDay () || pApp->IsMultiDay ();

						if (bAllOrMulti)
						{
							if (!bAllDay)
							{
								pApp->SetAllDay (FALSE);
								pApp->SetInterval (m_dtDrop, m_dtDrop);
							}
						}
						else if (bAllDay)
						{
							pApp->SetAllDay (TRUE);
							pApp->SetInterval (m_dtDrop, m_dtDrop);
						}
					}


					LRESULT lRes = 0L;
					if (bNotifyParent)
					{
						lRes = NotifyMessage (BCGP_PLANNER_BEFORE_UPDATE_APPOINTMENT, 0, (LPARAM)pApp);
					}

					if (lRes == 0L)
					{
						dtSel1 = pApp->GetStart ();
						dtSel2 = pApp->GetFinish ();

						if (AddAppointment (pApp, FALSE, FALSE) && bNotifyParent)
						{
							NotifyMessage (BCGP_PLANNER_UPDATE_APPOINTMENT, 0, 
								(LPARAM)(bRecurrenceClone ? NULL : pApp));
						}
					}
					else
					{
						if (pApp->IsRecurrenceClone ())
						{
							delete pApp;
						}
						else
						{
							AddAppointment (pApp, FALSE, FALSE);
						}
					}
				}

				if (m_Type == BCGP_PLANNER_TYPE_DAY ||
					m_Type == BCGP_PLANNER_TYPE_WORK_WEEK ||
					m_Type == BCGP_PLANNER_TYPE_MULTI)
				{
					if (dtSel1 == dtSel2)
					{
						dtSel2 += GetCurrentView ()->GetMinimumSpan ();
					}

					dtSel2 -= COleDateTimeSpan (0, 0, 0, 1);
				}
				else
				{
					dtSel1 = COleDateTime (dtSel1.GetYear (), dtSel1.GetMonth (), dtSel1.GetDay (), 0, 0, 0);
					dtSel2 = COleDateTime (dtSel2.GetYear (), dtSel2.GetMonth (), dtSel2.GetDay (), 0, 0, 0);
				}

				if (!GetCurrentView ()->SetCurrentResourceID (nResourceID, FALSE, FALSE))
				{
					nResourceID = nResourceOld;
				}

				GetCurrentView ()->SetSelection (dtSel1, dtSel2, FALSE);
			}

			if (!m_bDragDrop ||
				(m_bDragDrop && (dropEffect == DROPEFFECT_MOVE || dropEffect == DROPEFFECT_COPY)))
			{
				ClearAppointmentSelection (FALSE);
				QueryAppointments ();

				AdjustLayout (TRUE);

				UpdateCalendarsState ();
			}

			m_bNotifyParent = bNotify;

			if (m_bNotifyParent)
			{
				NotifyMessage (BCGP_PLANNER_UPDATED_ALL_APPOINTMENTS, 0, 0, TRUE);

				NotifyMessage (BCGP_PLANNER_DROP_APPOINTMENTS, 0, 0);

				if (nResourceOld != nResourceID)
				{
					NotifyMessage (BCGP_PLANNER_RESOURCEID_CHANGED, 0, 0);
				}
			}
		}
	}
	catch (COleException* pEx)
	{
		TRACE(_T("CBCGPPlannerManagerCtrl::OnDrop. OLE exception: %x\r\n"),
			pEx->m_sc);
		pEx->Delete ();
	}

	if(!IsDragDrop ())
	{
		UpdateChangeOperation(BCGP_PLANNER_CHANGE_OPERATION_NONE, bRes);
	}
	else if(m_bNotifyParent)
	{
		NotifyMessage (BCGP_PLANNER_END_CHANGE_OPERATION, 
			(WPARAM)BCGP_PLANNER_CHANGE_OPERATION_DROP, bRes);
	}

	m_dtDrop = COleDateTime ();

	return bRes;
}

DROPEFFECT CBCGPPlannerManagerCtrl::OnDragEnter(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return OnDragOver(pDataObject, dwKeyState, point);
}

void CBCGPPlannerManagerCtrl::OnDragLeave()
{
	CBCGPPlannerView* pView = GetCurrentView ();

	m_dragEffect = DROPEFFECT_NONE;

	if (pView->GetDragedAppointments ().GetSize () > 0)
	{
		pView->ClearDragedAppointments ();
		pView->AdjustAppointments ();

		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

DROPEFFECT CBCGPPlannerManagerCtrl::OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
#ifndef BCGP_PLANNER_DND_TO_ANOTHER_PLANNER
	// can't be dropped from another planner
	if (!IsDragDrop ())
	{
		return DROPEFFECT_NONE;
	}
#endif

	if (IsReadOnly ())
	{
		return DROPEFFECT_NONE;
	}

	CBCGPPlannerView* pView = GetCurrentView ();

	DROPEFFECT dragEffect = pView->OnDragOver (pDataObject, dwKeyState, point);

	CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION action = pView->GetAdjustAction ();

	if (!IsDragDrop () || (IsDragDrop () && 
		action != CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_NONE))
	{
		m_dragEffect = dragEffect;

		if (action == CBCGPPlannerView::BCGP_PLANNER_ADJUST_ACTION_APPOINTMENTS)
		{
			pView->AdjustAppointments ();
		}
		else
		{
			pView->AdjustLayout (FALSE);
		}

		RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	return m_dragEffect;
}

BOOL CBCGPPlannerManagerCtrl::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
	return GetCurrentView ()->OnScroll (nScrollCode, nPos, bDoScroll);
}

void CBCGPPlannerManagerCtrl::GetDragScrollRect (CRect& rect)
{
	GetCurrentView ()->GetDragScrollRect (rect);
}

DROPEFFECT CBCGPPlannerManagerCtrl::OnDragScroll(DWORD dwKeyState, CPoint point)
{
#ifndef BCGP_PLANNER_DND_TO_ANOTHER_PLANNER
	// can't be dropped from another planner
	if (!IsDragDrop ())
	{
		return DROPEFFECT_NONE;
	}
#endif

	return GetCurrentView ()->OnDragScroll (dwKeyState, point);
}

void CBCGPPlannerManagerCtrl::StartCapture ()
{
	if (!m_bCaptured)
	{
		CPoint pt;
		::GetCursorPos (&pt);
		ScreenToClient (&pt);

		m_ptCaptureStart = pt;
		m_ptCaptureCurrent = m_ptCaptureStart;

		GetCurrentView ()->StartCapture ();

		m_pWndLastCapture = SetCapture ();

		m_bCaptured = TRUE;
	}
}

void CBCGPPlannerManagerCtrl::StopCapture ()
{
	if (m_bCaptured)
	{
		m_bCaptured = FALSE;

		::ReleaseCapture ();

		if (m_pWndLastCapture != NULL)
		{
			m_pWndLastCapture->SetCapture ();
			m_pWndLastCapture = NULL;
		}

		GetCurrentView ()->StopCapture ();

		m_ptCaptureStart   = CPoint (0, 0);
		m_ptCaptureCurrent = m_ptCaptureStart;
	}
}

void CBCGPPlannerManagerCtrl::OnCancelMode() 
{
	StopDragDrop ();

	StopCapture ();

	CBCGPWnd::OnCancelMode();
}

LRESULT CBCGPPlannerManagerCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (IsCaptured ())
	{
		if (WM_MOUSEFIRST <= message && message <= WM_MOUSELAST &&
			message != WM_LBUTTONDOWN && message != WM_LBUTTONUP && message != WM_MOUSEMOVE)
		{
			OnCancelMode ();
		}
		else if (message == WM_KEYDOWN && (wParam == VK_CANCEL || wParam == VK_ESCAPE))
		{
			if (!IsDragDrop ())
			{
				GetCurrentView ()->RestoreCapturedAppointment ();
			}

			OnCancelMode ();
		}
	}
	
	return CBCGPWnd::WindowProc(message, wParam, lParam);
}

BOOL CBCGPPlannerManagerCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (m_ToolTipCount > 0 && m_pToolTip->GetSafeHwnd () != NULL)
	{
		m_pToolTip->RelayEvent (pMsg);
	}

	return CBCGPWnd::PreTranslateMessage (pMsg);
}

void CBCGPPlannerManagerCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if (!GetCurrentView ()->OnLButtonDblClk (nFlags, point))
	{
		CBCGPWnd::OnLButtonDblClk(nFlags, point);
	}

	if (m_bNotifyParent)
	{
		NotifyMessage (BCGP_PLANNER_LBUTTONDBLCLK, nFlags, MAKELPARAM(point.x, point.y));
	}
}

void CBCGPPlannerManagerCtrl::OnSelectTimeInterval (COleDateTime dateStart, COleDateTime dateFinish)
{
	CBCGPPlannerManagerView* pView = DYNAMIC_DOWNCAST (CBCGPPlannerManagerView,
		GetParent ());
	if (pView == NULL)
	{
		return;
	}

	ASSERT_VALID (pView);
	pView->OnSelectTimeInterval (dateStart, dateFinish);
}

void CBCGPPlannerManagerCtrl::SerializeRaw (CArchive& ar)
{
	CBCGPAppointmentBaseStorage* pAppsStorage = GetStorage ();
	ASSERT_VALID (pAppsStorage);

	CObList lstApps;

	if (ar.IsLoading ())
	{
		RemoveAllAppointments (FALSE);
		lstApps.Serialize (ar);

		for (POSITION pos = lstApps.GetHeadPosition () ; pos != NULL;)
		{
			CBCGPAppointment* pApp = (CBCGPAppointment*) lstApps.GetNext (pos);
			ASSERT_VALID (pApp);

			pAppsStorage->Add (pApp);
		}

		QueryAppointments ();
		AdjustLayout ();

		UpdateCalendarsState ();
	}
	else
	{
		XBCGPAppointmentArray arApps;
		pAppsStorage->QueryAll (arApps);

		for (int i = 0; i < arApps.GetSize (); i++)
		{
			lstApps.AddTail (arApps [i]);
		}

		lstApps.Serialize (ar);
	}
}

void CBCGPPlannerManagerCtrl::SetCalendar (CBCGPCalendar* pWndCalendar)
{
	m_pWndCalendar = pWndCalendar;

	if (m_pWndCalendar != NULL)
	{
		ASSERT_VALID (m_pWndCalendar);
		m_pWndCalendar->SetPlanner (this);
	}
}

void CBCGPPlannerManagerCtrl::UpdateCalendars(CBCGPCalendar* pWndCalendar /*= NULL*/)
{
	if (pWndCalendar == NULL)
	{
		pWndCalendar = m_pWndCalendar;
	}

	if (pWndCalendar->GetSafeHwnd () == NULL)
	{
		return;
	}

	ASSERT_VALID (pWndCalendar);

	UpdateCalendarsSelection (pWndCalendar);

	if (m_dateFirst != pWndCalendar->GetFirstDate () ||
		m_dateLast != pWndCalendar->GetLastDate ())
	{
		m_dateFirst = pWndCalendar->GetFirstDate ();
		m_dateLast = pWndCalendar->GetLastDate ();

		UpdateCalendarsState (pWndCalendar);
	}
}

void CBCGPPlannerManagerCtrl::UpdateCalendarsSelection (CBCGPCalendar* pWndCalendar /*= NULL*/)
{
	if (pWndCalendar == NULL)
	{
		pWndCalendar = m_pWndCalendar;
	}

	if (pWndCalendar->GetSafeHwnd () == NULL)
	{
		return;
	}

	ASSERT_VALID (pWndCalendar);

	CList<DATE, DATE&> lstDates;
	pWndCalendar->GetSelectedDates (lstDates);

	COleDateTime date1;
	COleDateTime date2;

	POSITION pos = lstDates.GetHeadPosition ();
	while (pos != NULL)
	{
		COleDateTime dt (lstDates.GetNext (pos));

		if (date1 == COleDateTime () && 
			date2 == COleDateTime ())
		{
			date1 = dt;
			date2 = dt;
		}
		else
		{
			if (dt < date1)
			{
				date1 = dt;
			}
			else if (date2 < dt)
			{
				date2 = dt;
			}

		}
	}

	COleDateTime dt (GetDateStart ());
	int nDuration = GetViewDuration ();
	int nWeekStart = GetFirstDayOfWeek ();

	if (m_Type == BCGP_PLANNER_TYPE_WEEK)
	{
		nWeekStart += 1;
		dt = CBCGPPlannerView::GetFirstWeekDay (dt, nWeekStart == 1 ? 2 : nWeekStart);
	}
	else if (m_Type == BCGP_PLANNER_TYPE_MONTH)
	{
		nWeekStart += 1;
		dt = CBCGPPlannerView::GetFirstWeekDay (dt, (IsCompressWeekend () && nWeekStart == 1) ? 2 : nWeekStart);
	}

	if (date1 != dt || date2 != (dt + COleDateTimeSpan (nDuration - 1, 0, 0 ,0)))
	{
		COleDateTimeSpan span (1, 0, 0, 0);
		CList<DATE, DATE&> lstDates;

		for (int i = 0; i < nDuration; i++)
		{
			lstDates.AddTail (dt.m_dt);
			dt += span;
		}

		pWndCalendar->SetSelectedDates (lstDates, FALSE);
	}
}

void CBCGPPlannerManagerCtrl::UpdateCalendarsState (CBCGPCalendar* pWndCalendar /*= NULL*/)
{
	if (pWndCalendar == NULL)
	{
		pWndCalendar = m_pWndCalendar;
	}

	if (pWndCalendar->GetSafeHwnd () == NULL)
	{
		return;
	}

	ASSERT_VALID (pWndCalendar);

	COleDateTime date1 = pWndCalendar->GetFirstDate ();
	COleDateTime date2 = pWndCalendar->GetLastDate ();

	XBCGPAppointmentArray arApps;
	QueryAppointments (arApps, date1, date2);

	CArray<DATE, DATE&> arDates;
	arDates.SetSize (arApps.GetSize ());

	for (int i = 0; i < arApps.GetSize (); i++)
	{
		const CBCGPAppointment* pApp = arApps [i];
		ASSERT(pApp != NULL);

		if (!pApp->IsAllDay ())
		{
			COleDateTime dtStart  (pApp->GetStart  ());
			COleDateTime dtFinish (pApp->GetFinish ());

			arDates [i] = dtStart;

			if (!CBCGPPlannerView::IsOneDay (dtStart, dtFinish))
			{
				dtStart.SetDate (dtStart.GetYear (), dtStart.GetMonth (), dtStart.GetDay ());

				if (dtFinish.GetHour () == 0 && dtFinish.GetMinute () == 0)
				{
					dtFinish -= COleDateTimeSpan (1, 0, 0, 0);
				}

				dtFinish.SetDate (dtFinish.GetYear (), dtFinish.GetMonth (), dtFinish.GetDay ());

				const int nDays = (dtFinish - dtStart).GetDays ();

				for (int nDay = 1; nDay <= nDays; nDay++)
				{
					dtStart += COleDateTimeSpan (1, 0, 0, 0);

					DATE d = dtStart;
					arDates.Add (d);
				}
			}
		}

		if (pApp->IsRecurrenceClone ())
		{
			delete pApp;
		}
	}

	pWndCalendar->MarkDates (arDates);
}

BOOL CBCGPPlannerManagerCtrl::OnDropAppointmentToCalendar (COleDataObject* pDataObject,
												 DROPEFFECT dropEffect, COleDateTime dateTo)
{
	if (IsReadOnly ())
	{
		return FALSE;
	}

	if (dropEffect != DROPEFFECT_MOVE && dropEffect != DROPEFFECT_COPY)
	{
		return FALSE;
	}

	ASSERT (pDataObject != NULL);
	ASSERT (pDataObject->IsDataAvailable (GetClipboardFormat ()));

	BOOL bRes = FALSE;

	m_dtDrop = dateTo;

	if(!IsDragDrop ())
	{
		if (!UpdateChangeOperation(BCGP_PLANNER_CHANGE_OPERATION_DROP))
		{
			m_dtDrop = COleDateTime ();

			return FALSE;
		}
	}
	else if(m_bNotifyParent)
	{
		if (NotifyMessage (BCGP_PLANNER_BEGIN_CHANGE_OPERATION, 
			(WPARAM)BCGP_PLANNER_CHANGE_OPERATION_DROP, (LPARAM)FALSE) != 0L)
		{
			m_dtDrop = COleDateTime ();

			return FALSE;
		}
	}

	try
	{
		GetCurrentView ()->ClearDragedAppointments ();

		CFile* pFile = pDataObject->GetFileData (GetClipboardFormat ());
		if (pFile == NULL)
		{
			m_dtDrop = COleDateTime ();
			
			return FALSE;
		}

		XBCGPAppointmentArray ar;

		bRes = CBCGPPlannerManagerCtrl::SerializeFrom 
			(*pFile, ar, BCGP_PLANNER_TYPE_WEEK, m_dtDrop);
		// BCGP_PLANNER_TYPE_WEEK or BCGP_PLANNER_TYPE_MONTH move appointments with
		// storing hours and minutes

		delete pFile;

		if (bRes)
		{
			CBCGPPlannerView* pView = GetCurrentView ();
			ASSERT_VALID(pView);

			BOOL bNotify = m_bNotifyParent;
			BOOL bNotifyParent = FALSE;
			if (m_bDragDrop && dropEffect != DROPEFFECT_COPY)
			{
				// Drop in same view as drag, don't send add & remove
				// notifications - send update notification
				bNotifyParent = m_bNotifyParent;
				m_bNotifyParent = FALSE;
			}

			const INT_PTR c_Count = ar.GetSize ();

			if (c_Count > 0)
			{
				BOOL bRemoveRecurrence = !m_bDragDrop || dropEffect == DROPEFFECT_COPY;

				if (m_bDragDrop)
				{
					for (int i = 0; i < m_arQueryApps.GetSize (); i++)
					{
						CBCGPAppointment* pApp = m_arQueryApps[i];

						if (pApp->IsRecurrenceClone () && pApp->IsSelected ())
						{
							m_lsSelectedApps.RemoveAt(m_lsSelectedApps.Find (pApp));
						}
					}

					if (dropEffect == DROPEFFECT_MOVE)
					{
						RemoveSelectedAppointments (FALSE, FALSE);
					}
				}

				for (int i = 0; i < c_Count; i++)
				{
					CBCGPAppointment* pApp = ar[i];
					ASSERT_VALID (pApp);

					BOOL bRecurrenceClone = pApp->IsRecurrence () && pApp->IsRecurrenceClone ();

					if (bRemoveRecurrence && bRecurrenceClone)
					{
						pApp->RemoveRecurrence ();
					}

					LRESULT lRes = 0L;
					if (bNotifyParent)
					{
						lRes = NotifyMessage (BCGP_PLANNER_BEFORE_UPDATE_APPOINTMENT, 0, (LPARAM)pApp);
					}

					if (lRes == 0L)
					{
						if (AddAppointment (pApp, FALSE, FALSE) && bNotifyParent)
						{
							NotifyMessage (BCGP_PLANNER_UPDATE_APPOINTMENT, 0, 
								(LPARAM)(bRecurrenceClone ? NULL : pApp));
						}
					}
					else
					{
						if (pApp->IsRecurrenceClone ())
						{
							delete pApp;
						}
						else
						{
							AddAppointment (pApp, FALSE, FALSE);
						}
					}
				}
			}

			if (!m_bDragDrop ||
				(m_bDragDrop && (dropEffect == DROPEFFECT_MOVE || dropEffect == DROPEFFECT_COPY)))
			{
				ClearAppointmentSelection (FALSE);
				QueryAppointments ();

				SetDate (dateTo, FALSE);

				pView->AdjustLayout (TRUE);

				UpdateCalendarsState ();
			}

			m_bNotifyParent = bNotify;

			if (m_bNotifyParent)
			{
				NotifyMessage (BCGP_PLANNER_UPDATED_ALL_APPOINTMENTS, 0, 0);

				NotifyMessage (BCGP_PLANNER_DROP_APPOINTMENTS, 0, 0);
			}
		}
	}
	catch (COleException* pEx)
	{
		TRACE(_T("CBCGPPlannerManagerCtrl::OnDrop. OLE exception: %x\r\n"),
			pEx->m_sc);
		pEx->Delete ();
	}

	if(!IsDragDrop ())
	{
		UpdateChangeOperation(BCGP_PLANNER_CHANGE_OPERATION_NONE, bRes);
	}
	else if(m_bNotifyParent)
	{
		NotifyMessage (BCGP_PLANNER_END_CHANGE_OPERATION, 
			(WPARAM)BCGP_PLANNER_CHANGE_OPERATION_DROP, bRes);
	}

	m_dtDrop = COleDateTime ();

	return bRes;
}

BOOL CBCGPPlannerManagerCtrl::IsClipboardFormatAvailable (
	COleDataObject* pDataObject)
{
	ASSERT (pDataObject != NULL);
	return (pDataObject->IsDataAvailable (GetClipboardFormat ()));
}

void CBCGPPlannerManagerCtrl::SetBackgroundColor (COLORREF clr, BOOL bRedraw /*= TRUE*/)
{
	if (clr != m_clrBackground)
	{
		m_clrBackground = clr;

		if (bRedraw)
		{
			RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}
}

void CBCGPPlannerManagerCtrl::Print ()
{
	CPrintInfo printInfo;
	printInfo.m_bPreview = FALSE;

	OnPreparePrinting(&printInfo);

	printInfo.m_pPD->m_pd.nFromPage = (WORD)printInfo.GetMinPage();
	printInfo.m_pPD->m_pd.nToPage = (WORD)printInfo.GetMaxPage();
	printInfo.m_pPD->m_pd.nCopies = 1;

	if (printInfo.m_pPD->DoModal() == IDCANCEL)
	{
		OnEndPrinting(NULL, NULL);
		return;
	}
	
	CDC dc;
	dc.Attach(printInfo.m_pPD->GetPrinterDC());
	dc.m_bPrinting = TRUE;

	CString strTitle;
	strTitle.LoadString(AFX_IDS_APP_TITLE);

	DOCINFO di;
	::ZeroMemory (&di, sizeof (DOCINFO));
	di.cbSize = sizeof (DOCINFO);
	di.lpszDocName = strTitle;

	BOOL bPrintingOK = dc.StartDoc(&di);

	printInfo.m_rectDraw.SetRect(0, 0,
							dc.GetDeviceCaps(HORZRES), 
							dc.GetDeviceCaps(VERTRES));

	OnBeginPrinting(&dc, &printInfo);

	dc.StartPage();

	OnPrint(&dc, &printInfo);

	bPrintingOK = (dc.EndPage() > 0);

	OnEndPrinting(&dc, &printInfo);

	if (bPrintingOK)
	{
		dc.EndDoc();
	}
	else
	{
		dc.AbortDoc();
	}

	dc.DeleteDC();
}

void CBCGPPlannerManagerCtrl::SetShowToolTip (BOOL bShowToolTip)
{
	if (m_bShowToolTip != bShowToolTip)
	{
		m_bShowToolTip = bShowToolTip;

		if (GetSafeHwnd () != NULL)
		{
			InitToolTipInfo ();
		}
	}
}

void CBCGPPlannerManagerCtrl::SetToolTipShowAlways (BOOL bToolTipShowAlways)
{
	if (m_bToolTipShowAlways != bToolTipShowAlways)
	{
		m_bToolTipShowAlways = bToolTipShowAlways;

		if (GetSafeHwnd () != NULL)
		{
			InitToolTipInfo ();
		}
	}
}

void CBCGPPlannerManagerCtrl::InitToolTipInfo ()
{
	ClearToolTipInfo ();

	if (!m_bShowToolTip)
	{
		return;
	}

	if (GetCurrentView () != NULL)
	{
		GetCurrentView ()->InitViewToolTipInfo ();
	}	

	for(int i = 0; i < m_arQueryApps.GetSize (); i++)
	{
		const CBCGPAppointment* pApp = m_arQueryApps[i];

		if (pApp != NULL)
		{
			AddToolTipInfo (pApp);
		}
	}
}

void CBCGPPlannerManagerCtrl::AddToolTipInfo (const CRect& rect)
{
	if (m_pToolTip->GetSafeHwnd () == NULL || rect.IsRectEmpty ())
	{
		return;
	}

	m_ToolTipCount++;
	m_pToolTip->AddTool (this, LPSTR_TEXTCALLBACK, rect, m_ToolTipCount);
}

void CBCGPPlannerManagerCtrl::AddToolTipInfo (const CBCGPAppointment* pApp)
{
	ASSERT_VALID (pApp);

	if (pApp != NULL)
	{
		BOOL bResizeVert = m_Type == BCGP_PLANNER_TYPE_DAY ||
						   m_Type == BCGP_PLANNER_TYPE_WORK_WEEK ||
						   m_Type == BCGP_PLANNER_TYPE_MULTI;

		if (pApp->GetDSDraw ().IsEmpty ())
		{
			if (pApp->IsVisibleDraw () && 
				(m_bToolTipShowAlways || pApp->IsToolTipNeeded ()))
			{
				CRect rt (pApp->GetRectEditHitTest ());
				pApp->AdjustToolTipRect (rt, bResizeVert);

				AddToolTipInfo (rt);
			}
		}
		else
		{
			for (int i = 0; i < pApp->GetDSDraw ().GetCount (); i++)
			{
				const CBCGPAppointmentDrawStructEx* pDS = 
					(const CBCGPAppointmentDrawStructEx*)(pApp->GetDSDraw ().GetByIndex (i));

				if (pDS != NULL && 
					pDS->IsVisible () && 
					(m_bToolTipShowAlways || pDS->IsToolTipNeeded ()))
				{
					CRect rt (pDS->GetRectEditHitTest ());
					pApp->AdjustToolTipRect (rt, bResizeVert);

					AddToolTipInfo (rt);
				}
			}
		}
	}
}

void CBCGPPlannerManagerCtrl::ClearToolTipInfo ()
{
	if (m_ToolTipCount > 0 && m_pToolTip->GetSafeHwnd () != NULL)
	{
		for (int i = 0; i < m_ToolTipCount; i++)
		{
			m_pToolTip->DelTool (this, i + 1);
		}

		m_ToolTipCount = 0;
	}
}

BOOL CBCGPPlannerManagerCtrl::OnNeedToolTipText(UINT /*id*/, NMHDR* pNMH, LRESULT* /*pResult*/)
{
	if (!m_bShowToolTip)
	{
		return FALSE;
	}

	static CString strToolTip;

	if (m_ToolTipCount == 0 || m_pToolTip->GetSafeHwnd () == NULL || 
		pNMH->hwndFrom != m_pToolTip->GetSafeHwnd ())
	{
		return FALSE;
	}

	CPoint point;
	::GetCursorPos (&point);
	ScreenToClient (&point);

	strToolTip = GetCurrentView ()->GetToolTipText (point);

	if (strToolTip.IsEmpty ())
	{
		return FALSE;
	}

	LPNMTTDISPINFO	pTTDispInfo = (LPNMTTDISPINFO) pNMH;
	ASSERT((pTTDispInfo->uFlags & TTF_IDISHWND) == 0);

	pTTDispInfo->lpszText = (LPTSTR)((LPCTSTR) strToolTip);
	m_pToolTip->SetFont (GetFont (), FALSE);

	return TRUE;
}
//**************************************************************************
LRESULT CBCGPPlannerManagerCtrl::OnBCGUpdateToolTips (WPARAM wp, LPARAM)
{
	UINT nTypes = (UINT) wp;

	if (m_pToolTip->GetSafeHwnd () == NULL)
	{
		return 0;
	}

	if (nTypes & BCGP_TOOLTIP_TYPE_PLANNER)
	{
		ClearToolTipInfo ();

		CString str;
		m_pToolTip->GetText (str, this);

		CBCGPTooltipManager::CreateToolTip (m_pToolTip, this,
			BCGP_TOOLTIP_TYPE_PLANNER);

		if (m_pToolTip->GetSafeHwnd () != NULL)
		{
			InitToolTipInfo ();
		}
	}

	return 0;
}
//**************************************************************************
void CBCGPPlannerManagerCtrl::SetDrawFlags (DWORD dwFlags, BOOL bRedraw/* = TRUE*/)
{
	ASSERT_VALID (this);

	m_bDefaultDrawFlags = FALSE;

	if ((dwFlags & BCGP_PLANNER_DRAW_APP_NO_DURATION) == BCGP_PLANNER_DRAW_APP_NO_DURATION &&
		(dwFlags & BCGP_PLANNER_DRAW_APP_DURATION_SHAPE) == BCGP_PLANNER_DRAW_APP_DURATION_SHAPE)
	{
		ASSERT(FALSE);
		dwFlags &= ~BCGP_PLANNER_DRAW_APP_DURATION_SHAPE;
	}

	if ((dwFlags & BCGP_PLANNER_DRAW_APP_OVERRIDE_SELECTION) == 0 &&
		(dwFlags & BCGP_PLANNER_DRAW_APP_DURATION_SHAPE) == BCGP_PLANNER_DRAW_APP_DURATION_SHAPE)
	{
		ASSERT(FALSE);
		dwFlags |= BCGP_PLANNER_DRAW_APP_OVERRIDE_SELECTION;
	}

	if (m_dwDrawFlags != dwFlags)
	{
		m_dwDrawFlags = dwFlags;

		if (GetSafeHwnd () != NULL)
		{
			AdjustLayout (bRedraw);
		}
	}
}
//**************************************************************************
DWORD CBCGPPlannerManagerCtrl::GetDrawFlags () const
{
	ASSERT_VALID (this);

	if (m_bDefaultDrawFlags)
	{
		return visualManager->GetPlannerDrawFlags ();
	}

	return m_dwDrawFlags;
}
//**************************************************************************
void CBCGPPlannerManagerCtrl::SetUseDayViewInsteadWeekView (BOOL bUse)
{
	m_bUseDayViewInsteadWeekView = bUse;
}
//**************************************************************************
BOOL CBCGPPlannerManagerCtrl::EnsureVisible(const CBCGPAppointment* pApp, BOOL bPartialOK)
{
	ASSERT_VALID(pApp);
	if (pApp == NULL)
	{
		return FALSE;
	}

	int nCount = GetQueryedAppointmentsCount ();
	if (nCount == 0)
	{
		return FALSE;
	}

	BOOL bFound = FALSE;
	for (int i = 0; i < nCount; i++)
	{
		if (m_arQueryApps[i] == pApp)
		{
			bFound = TRUE;
			break;
		}
	}

	if (!bFound)
	{
		return FALSE;
	}

	return GetCurrentView ()->EnsureVisible(pApp, bPartialOK);
}
//**************************************************************************
void CBCGPPlannerManagerCtrl::SetScrollBarVisible (BOOL bVisible)
{
	if (m_bScrollVisible != bVisible)
	{
		m_bScrollVisible = bVisible;
		AdjustLayout (TRUE);
	}
}
//**************************************************************************
void CBCGPPlannerManagerCtrl::EnableHeaderScrolling (BOOL bEnable)
{
	if (m_bHeaderScrollingEnabled != bEnable)
	{
		m_bHeaderScrollingEnabled = bEnable;

		CBCGPPlannerView* pView = GetCurrentView();
		if (pView != NULL && pView->CanUseHeaderScrolling())
		{
			AdjustLayout (TRUE);
		}
	}
}
//**************************************************************************
UINT CBCGPPlannerManagerCtrl::GetCurrentResourceID () const
{
	if (!IsMultiResourceStorage ())
	{
		return CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID;
	}

	UINT nResourceID = GetCurrentView ()->GetCurrentResourceID ();
	if (nResourceID != CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID)
	{
		return nResourceID;
	}

	return ((CBCGPAppointmentBaseMultiStorage*)GetStorage())->GetCurrentResourceID ();
}
//**************************************************************************
BOOL CBCGPPlannerManagerCtrl::SetCurrentResourceID (UINT nResourceID, BOOL bRedraw/* = TRUE*/)
{
	if (!IsMultiResourceStorage () || nResourceID == CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID)
	{
		return FALSE;
	}

	CBCGPAppointmentBaseMultiStorage* pStorage = (CBCGPAppointmentBaseMultiStorage*)GetStorage();

	UINT nOldResourceID = pStorage->GetCurrentResourceID ();

	if (!pStorage->SetCurrentResourceID (nResourceID))
	{
		return FALSE;
	}

	if (GetCurrentView () != NULL)
	{
		if (GetCurrentView ()->SetCurrentResourceID (nResourceID, FALSE))
		{
			pStorage->SetCurrentResourceID (CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID);
		}

		BOOL bUpdate = GetCurrentView ()->OnUpdateStorage ();
		if (nOldResourceID != pStorage->GetCurrentResourceID () || bUpdate)
		{
			QueryAppointments ();
			AdjustLayout (bRedraw);

			UpdateCalendarsState ();
		}
		else if (bRedraw && GetSafeHwnd () != NULL)
		{
			RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}	

	return TRUE;
}
//**************************************************************************
void CBCGPPlannerManagerCtrl::UpdateStorage (BOOL bRedraw/* = TRUE*/)
{
	if (GetCurrentView () != NULL)
	{
		if (GetCurrentView ()->OnUpdateStorage ())
		{
			QueryAppointments ();
			AdjustLayout (bRedraw);
		}
	}
}
//**************************************************************************
BOOL CBCGPPlannerManagerCtrl::AddResourceRTC (UINT nResourceID, CRuntimeClass* pStorageClass, 
										      CBCGPAppointmentBaseResourceInfo* pInfo, 
											  BOOL bAutoDelete/* = TRUE*/, 
											  BOOL bRedraw/* = TRUE*/)
{
	if (!IsMultiResourceStorage () || pInfo == NULL)
	{
		return FALSE;
	}

	if (pStorageClass != NULL)
	{
		if (!pStorageClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPAppointmentBaseStorage)))
		{
			ASSERT (FALSE);
			pStorageClass = NULL;
		}
	}

	if (pStorageClass == NULL)
	{
		pStorageClass = RUNTIME_CLASS (CBCGPAppointmentStorage);
	}

	CBCGPAppointmentBaseStorage* pAppsStorage = 
		(CBCGPAppointmentBaseStorage*)pStorageClass->CreateObject ();
	ASSERT_VALID (pAppsStorage);

	if (!AddResource (nResourceID, pAppsStorage, pInfo, bAutoDelete, bRedraw))
	{
		delete pAppsStorage;
		pAppsStorage = NULL;
	}

	return pAppsStorage != NULL;
}
//**************************************************************************
BOOL CBCGPPlannerManagerCtrl::AddResource (UINT nResourceID, CBCGPAppointmentBaseStorage* pStorage, 
										   CBCGPAppointmentBaseResourceInfo* pInfo, 
										   BOOL bAutoDelete/* = TRUE*/, BOOL bRedraw/* = TRUE*/)
{
	if (!IsMultiResourceStorage () || pStorage == NULL || pInfo == NULL)
	{
		return FALSE;
	}

	CBCGPAppointmentBaseMultiStorage* pAppsStorage = 
		DYNAMIC_DOWNCAST(CBCGPAppointmentBaseMultiStorage, GetStorage ());
	ASSERT_VALID(pAppsStorage);

	if (pAppsStorage == NULL)
	{
		return FALSE;
	}

	if (nResourceID == CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID)
	{
		CBCGPAppointmentBaseMultiStorage::XResourceIDArray ar;
		pAppsStorage->GetResourceIDs (ar);

		nResourceID = ar[ar.GetSize () - 1] + 1;
	}

	if (!pAppsStorage->AddStorage (nResourceID, pStorage, pInfo, bAutoDelete))
	{
		return FALSE;
	}

	if (GetSafeHwnd () != NULL)
	{
		SetCurrentResourceID (nResourceID, FALSE);

		UpdateStorage (FALSE);
		AdjustLayout (bRedraw);
	}

	return TRUE;
}
//**************************************************************************
BOOL CBCGPPlannerManagerCtrl::RemoveResource (UINT nResourceID, BOOL bRedraw/* = TRUE*/)
{
	if (nResourceID == CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID)
	{
		nResourceID = GetCurrentResourceID ();
	}

	if (!IsMultiResourceStorage () || nResourceID == CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID)
	{
		return FALSE;
	}

	CBCGPAppointmentBaseMultiStorage* pAppsStorage = 
		DYNAMIC_DOWNCAST(CBCGPAppointmentBaseMultiStorage, GetStorage ());
	ASSERT_VALID(pAppsStorage);

	if (pAppsStorage == NULL)
	{
		return FALSE;
	}

	if (pAppsStorage->GetCount () <= 1)
	{
		return FALSE;
	}

	if (pAppsStorage->GetStorage (nResourceID) != NULL)
	{
		ClearAppointmentSelection (FALSE);
		ClearQuery ();

		BOOL bRes = pAppsStorage->RemoveStorage (nResourceID);

		if (GetSafeHwnd () != NULL)
		{
			if (bRes)
			{
				CBCGPAppointmentBaseMultiStorage::XResourceIDArray ar;
				pAppsStorage->GetResourceIDs (ar);

				UpdateStorage (FALSE);
				SetCurrentResourceID (ar[0], bRedraw);
			}
			else
			{
				QueryAppointments ();
				AdjustLayout (bRedraw);
			}
		}

		return TRUE;
	}	

	return FALSE;
}
//**************************************************************************
const CBCGPAppointmentBaseResourceInfo* CBCGPPlannerManagerCtrl::GetResourceInfo (UINT nResourceID) const
{
	if (nResourceID == CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID)
	{
		nResourceID = GetCurrentResourceID ();
	}

	if (!IsMultiResourceStorage () || nResourceID == CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID)
	{
		return NULL;
	}

	CBCGPAppointmentBaseMultiStorage* pAppsStorage = 
		DYNAMIC_DOWNCAST(CBCGPAppointmentBaseMultiStorage, GetStorage ());
	ASSERT_VALID(pAppsStorage);

	if (pAppsStorage == NULL)
	{
		return FALSE;
	}

	return pAppsStorage->GetResourceInfo (nResourceID);
}
//**************************************************************************
void CBCGPPlannerManagerCtrl::SetUseMultiResourceDefault (BOOL bUseMultiResourceDefault)
{
	m_bUseMultiResourceDefault = bUseMultiResourceDefault;
};
//**************************************************************************
LRESULT CBCGPPlannerManagerCtrl::OnPrintClient(WPARAM wp, LPARAM lp)
{
	DWORD dwFlags = (DWORD)lp;

	if (dwFlags & PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);

		DoPaint(pDC);
	}

	return 0;
}
//**************************************************************************
int CBCGPPlannerManagerCtrl::GetFirstWorkingHour (BOOL bDefault) const
{
	if (!bDefault && IsMultiResourceStorage ())
	{
		CBCGPAppointmentBaseMultiStorage* pStorage = 
			DYNAMIC_DOWNCAST (CBCGPAppointmentBaseMultiStorage, GetStorage ());
		if (pStorage != NULL)
		{
			const CBCGPAppointmentBaseResourceInfo* pInfo = 
				pStorage->GetResourceInfo (GetCurrentResourceID ());

			if (pInfo != NULL)
			{
				COleDateTime dtStart (pInfo->GetWorkStart ());
				COleDateTime dtEnd (pInfo->GetWorkEnd ());

				if (dtStart != COleDateTime () && dtEnd != COleDateTime () &&
					dtStart < dtEnd)
				{
					return dtStart.GetHour ();
				}
			}
		}
	}

	return (int)m_nFirstWorkingHour;
}
//**************************************************************************
int CBCGPPlannerManagerCtrl::GetFirstWorkingMinute (BOOL bDefault) const
{
	if (!bDefault && IsMultiResourceStorage ())
	{
		CBCGPAppointmentBaseMultiStorage* pStorage = 
			DYNAMIC_DOWNCAST (CBCGPAppointmentBaseMultiStorage, GetStorage ());
		if (pStorage != NULL)
		{
			const CBCGPAppointmentBaseResourceInfo* pInfo = 
				pStorage->GetResourceInfo (GetCurrentResourceID ());

			if (pInfo != NULL)
			{
				COleDateTime dtStart (pInfo->GetWorkStart ());
				COleDateTime dtEnd (pInfo->GetWorkEnd ());

				if (dtStart != COleDateTime () && dtEnd != COleDateTime () &&
					dtStart < dtEnd)
				{
					return dtStart.GetMinute ();
				}
			}
		}
	}

	return (int)((m_nFirstWorkingHour - (int)m_nFirstWorkingHour) * 100.0 + 0.05);
}
//**************************************************************************
int CBCGPPlannerManagerCtrl::GetLastWorkingHour (BOOL bDefault) const
{
	if (!bDefault && IsMultiResourceStorage ())
	{
		CBCGPAppointmentBaseMultiStorage* pStorage = 
			DYNAMIC_DOWNCAST (CBCGPAppointmentBaseMultiStorage, GetStorage ());
		if (pStorage != NULL)
		{
			const CBCGPAppointmentBaseResourceInfo* pInfo = 
				pStorage->GetResourceInfo (GetCurrentResourceID ());

			if (pInfo != NULL)
			{
				COleDateTime dtStart (pInfo->GetWorkStart ());
				COleDateTime dtEnd (pInfo->GetWorkEnd ());

				if (dtStart != COleDateTime () && dtEnd != COleDateTime () &&
					dtStart < dtEnd)
				{
					return dtEnd.GetHour ();
				}
			}
		}
	}

	return (int)m_nLastWorkingHour;
}
//**************************************************************************
int CBCGPPlannerManagerCtrl::GetLastWorkingMinute (BOOL bDefault) const
{
	if (!bDefault && IsMultiResourceStorage ())
	{
		CBCGPAppointmentBaseMultiStorage* pStorage = 
			DYNAMIC_DOWNCAST (CBCGPAppointmentBaseMultiStorage, GetStorage ());
		if (pStorage != NULL)
		{
			const CBCGPAppointmentBaseResourceInfo* pInfo = 
				pStorage->GetResourceInfo (GetCurrentResourceID ());

			if (pInfo != NULL)
			{
				COleDateTime dtStart (pInfo->GetWorkStart ());
				COleDateTime dtEnd (pInfo->GetWorkEnd ());

				if (dtStart != COleDateTime () && dtEnd != COleDateTime () &&
					dtStart < dtEnd)
				{
					return dtEnd.GetMinute ();
				}
			}
		}
	}

	return (int)((m_nLastWorkingHour - (int)m_nLastWorkingHour) * 100.0 + 0.05);
}
//**************************************************************************
int CBCGPPlannerManagerCtrl::GetFirstSelectionHour () const
{
	int hour = GetFirstWorkingHour (FALSE);
	return (int)(hour < m_nFirstViewHour ? m_nFirstViewHour : hour);
}
//**************************************************************************
int CBCGPPlannerManagerCtrl::GetFirstSelectionMinute () const
{
	int hour = GetFirstWorkingHour (FALSE);
	if (hour < m_nFirstViewHour)
	{
		return (int)((m_nFirstViewHour - (int)m_nFirstViewHour) * 100.0 + 0.05);
	}

	return GetFirstWorkingMinute (FALSE);
}
//**************************************************************************
int CBCGPPlannerManagerCtrl::GetLastSelectionHour () const
{
	int hour = GetLastWorkingHour (FALSE);
	return (int)(m_nLastViewHour < hour ? m_nLastViewHour + 1 : hour);
}
//**************************************************************************
int CBCGPPlannerManagerCtrl::GetLastSelectionMinute () const
{
	int hour = GetLastWorkingHour (FALSE);
	if (m_nLastViewHour < hour)
	{
		return (int)((m_nLastViewHour - (int)m_nLastViewHour) * 100.0 + 0.05);
	}

	return GetLastWorkingMinute (FALSE);
}
//**************************************************************************
void CBCGPPlannerManagerCtrl::SetWorkWeekInterval(int nWorkWeekInterval)
{
	nWorkWeekInterval = bcg_clamp (nWorkWeekInterval, 1, 7);
	if (m_nWorkWeekInterval == nWorkWeekInterval)
	{
		return;
	}

	m_nWorkWeekInterval = nWorkWeekInterval;

	CBCGPPlannerView* pView = GetView (BCGP_PLANNER_TYPE_WORK_WEEK);
	if (pView == NULL)
	{
		return;
	}

	if (pView->GetPlanner () == this)
	{
		pView->SetDate (pView->GetDate ());

		OnDateChanged ();
	}
}
//**************************************************************************
CBCGPPlannerView::BCGP_PLANNER_WORKING_STATUS
CBCGPPlannerManagerCtrl::GetWorkingPeriodParameters (int ResourceId, const COleDateTime &DateTimeStart, const COleDateTime& DateTimeEnd, CBCGPPlannerView::XBCGPPlannerWorkingParameters& parameters) const
{
	CBCGPPlannerManagerView* pView = DYNAMIC_DOWNCAST (CBCGPPlannerManagerView,
		GetParent ());
	if (pView != NULL)
	{
		return pView->GetWorkingPeriodParameters (ResourceId, DateTimeStart, DateTimeEnd, parameters);
	}

	return CBCGPPlannerView::BCGP_PLANNER_WORKING_STATUS_UNKNOWN;
}
//**************************************************************************
LRESULT CBCGPPlannerManagerCtrl::NotifyMessage(UINT message, WPARAM wParam, LPARAM lParam, BOOL bPost) const
{
	CWnd* pNotifyWnd = GetNotifyWnd ();

	if (GetSafeHwnd () == NULL || pNotifyWnd->GetSafeHwnd () == NULL)
	{
		return 0L;
	}

	if (bPost)
	{
		return (LRESULT)pNotifyWnd->PostMessage (message, wParam, lParam);
	}

	return pNotifyWnd->SendMessage (message, wParam, lParam);
}
//**************************************************************************
const CBCGPAppointment* CBCGPPlannerManagerCtrl::GetAccAppointment() const
{
	POSITION pos = GetFirstSelectedAppointment ();
	if (pos == NULL)
	{
		return NULL;
	}

	return GetNextSelectedAppointment (pos);
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::get_accChildCount(long *pcountChildren)
{
	if (!pcountChildren)
	{
		return E_INVALIDARG;
	}

	*pcountChildren = 0;

	return S_OK;
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::get_accChild(VARIANT /*varChild*/, IDispatch **ppdispChild)
{
	if (!(*ppdispChild))
	{
		return E_INVALIDARG;
	}

	if (m_pStdObject != NULL)
	{
		*ppdispChild = m_pStdObject;
	}
	else
	{
		*ppdispChild = NULL;
	}

	return S_OK;
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::get_accName(VARIANT varChild, BSTR *pszName)
{
	if (pszName == NULL)
	{
		return E_INVALIDARG;
	}

	CBCGPPlannerView* pView = GetCurrentView ();
	if (pView == NULL)
	{
		return S_FALSE;
	}

	CString strValue;

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		strValue = _T("Calendar");

		CString strViewName(pView->GetAccName ());
		if (!strViewName.IsEmpty ())
		{
			strValue += _T(" - ") + strViewName;
		}
	}
	else
	{
		const CBCGPAppointment* pApp = GetAccAppointment ();
		if (pApp != NULL)
		{
			strValue = pApp->GetAccName();
		}
	}

	if (!strValue.IsEmpty ())
	{
		*pszName = strValue.AllocSysString ();

		return S_OK;
	}

	return S_FALSE;
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::get_accValue(VARIANT varChild, BSTR *pszValue)
{
	if (pszValue == NULL)
	{
		return E_INVALIDARG;
	}

	CString strValue;

	CBCGPPlannerView* pView = GetCurrentView ();
	if (pView == NULL)
	{
		return S_FALSE;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		strValue = pView->GetAccValue ();
	}
	else
	{
		const CBCGPAppointment* pApp = GetAccAppointment ();
		if (pApp != NULL)
		{
			strValue = pApp->GetAccValue();
		}
	}

	if (!strValue.IsEmpty ())
	{
		*pszValue = strValue.AllocSysString ();

		return S_OK;
	}

	return S_FALSE;
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::get_accDescription(VARIANT varChild, BSTR *pszDescription)
{
	if (pszDescription == NULL)
	{
		return E_INVALIDARG;
	}

	CBCGPPlannerView* pView = GetCurrentView ();
	if (pView == NULL)
	{
		return S_FALSE;
	}

	CString strValue;

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		strValue = pView->GetAccDescription ();
	}
	else
	{
		const CBCGPAppointment* pApp = GetAccAppointment ();
		if (pApp != NULL)
		{
			strValue = pApp->GetAccDescription();
		}
	}

	if (!strValue.IsEmpty ())
	{
		*pszDescription = strValue.AllocSysString();

		return S_OK;
	}

	return S_FALSE;
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
	if (!pvarRole || ((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)))
	{
		return E_INVALIDARG;
	}

	pvarRole->vt = VT_I4;
	pvarRole->lVal = ROLE_SYSTEM_STATICTEXT;

	return S_OK;
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::get_accState(VARIANT varChild, VARIANT *pvarState)
{
	if (!pvarState || ((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)))
	{
		return E_INVALIDARG;
	}

	pvarState->vt = VT_I4;
	pvarState->lVal = STATE_SYSTEM_FOCUSED | STATE_SYSTEM_READONLY;

	return S_OK;
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::get_accHelp(VARIANT /*varChild*/, BSTR * /*pszHelp*/)
{
	return S_FALSE;
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::get_accHelpTopic(BSTR * /*pszHelpFile*/, VARIANT /*varChild*/, long * /*pidTopic*/)
{
	return S_FALSE;
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::get_accKeyboardShortcut(VARIANT /*varChild*/, BSTR* /*pszKeyboardShortcut*/)
{
	return S_FALSE;
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::get_accFocus(VARIANT *pvarChild)
{
	if (NULL == pvarChild)
	{
		return E_INVALIDARG;
	}

	return DISP_E_MEMBERNOTFOUND;
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::get_accSelection(VARIANT *pvarChildren)
{
	if (NULL == pvarChildren)
	{
		return E_INVALIDARG;
	}

	return DISP_E_MEMBERNOTFOUND;
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::get_accDefaultAction(VARIANT /*varChild*/, BSTR* /*pszDefaultAction*/)
{
	return DISP_E_MEMBERNOTFOUND; 
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::accSelect(long flagsSelect, VARIANT varChild)
{
	if (m_pStdObject != NULL)
	{
		return m_pStdObject->accSelect(flagsSelect, varChild);
	}

	return E_INVALIDARG;
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
	if (!pxLeft || !pyTop || !pcxWidth || !pcyHeight)
	{
		return E_INVALIDARG;
	}

	if (GetCurrentView () == NULL)
	{
		return S_FALSE;
	}

	CRect rect;

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		GetWindowRect(rect);
	}
	else
	{
		const CBCGPAppointment* pApp = GetAccAppointment ();
		if (pApp != NULL)
		{
			return E_NOTIMPL;
		}
	}

	*pxLeft = rect.left;
	*pyTop = rect.top;
	*pcxWidth = rect.Width();
	*pcyHeight = rect.Height();

	return S_OK;
}
//**************************************************************************
HRESULT CBCGPPlannerManagerCtrl::accHitTest(long  xLeft, long yTop, VARIANT *pvarChild)
{
	if (!pvarChild)
	{
		return E_INVALIDARG;
	}

	if (GetCurrentView () == NULL)
	{
		return S_FALSE;
	}

	CPoint pt(xLeft, yTop);
	ScreenToClient(&pt);

	const CBCGPAppointment* pApp = GetAccAppointment ();
	if (pApp != NULL)
	{
		LPARAM lParam = MAKELPARAM((WORD)xLeft, (WORD)yTop);
		pvarChild->vt = VT_I4;
		pvarChild->lVal = (LONG)lParam;
	}
	else
	{
		pvarChild->vt = VT_I4;
		pvarChild->lVal = CHILDID_SELF;
	}

	return S_OK;
}

void CBCGPPlannerManagerCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CBCGPPlannerManagerView* pView = DYNAMIC_DOWNCAST (CBCGPPlannerManagerView,
		GetParent ());
	if (pView == NULL)
	{
		return;
	}

	ASSERT_VALID (pView);

	if (point.x == -1 && point.y == -1)
	{
		BOOL bFirst = TRUE;
		CRect rect(0, 0, 0, 0);

		POSITION pos = GetFirstSelectedAppointment();
		if (pos != NULL)
		{
			const CBCGPAppointment* pApp = GetNextSelectedAppointment(pos);
			if (pApp != NULL)
			{
				CRect rectApp (pApp->GetVisibleRectDraw(bFirst));

				rect.IntersectRect(rectApp, GetCurrentView()->GetAppointmentsRect());
				if (rect.IsRectEmpty())
				{
					rect = rectApp;
				}
			}
		}
		
		if (rect.IsRectEmpty())
		{
			COleDateTime dtStart(GetSelectionStart());
			COleDateTime dtEnd(GetSelectionEnd());

			if (dtEnd < dtStart)
			{
				COleDateTime dtTemp (dtStart);
				dtStart = dtEnd;
				dtEnd = dtTemp;
			}

			CRect rectView(GetCurrentView()->GetRectFromDate(bFirst ? dtStart : dtEnd));

			rect.IntersectRect(rectView, GetCurrentView()->GetAppointmentsRect());
			if (rect.IsRectEmpty())
			{
				rect = GetCurrentView()->GetAppointmentsRect();
				rect.left = rectView.left;
				rect.right = rectView.right;
			}
		}

		if (!rect.IsRectNull())
		{
			ClientToScreen(rect);
			point = rect.TopLeft();
		}
	}

	pView->SendMessage(WM_CONTEXTMENU, (WPARAM)pWnd->GetSafeHwnd(), MAKELPARAM(point.x, point.y));
}

#endif // BCGP_EXCLUDE_PLANNER
