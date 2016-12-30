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
// BCGPAppointment.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPAppointment.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerManagerCtrl.h"
#include "BCGPPlannerViewDay.h"

#include "BCGPPlannerClockIcons.h"

#include "BCGPDrawManager.h"

#include "BCGPAppointmentProperty.h"

#include "BCGPRecurrence.h"
#include "BCGPAppointmentStorage.h"

#include "BCGPMath.h"

#ifndef _BCGSUITE_
#include "BCGPWorkspace.h"
extern CBCGPWorkspace*	g_pWorkspace;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int c_MAXSTACK = 2048;
const int c_HT_PREC  = 4;

inline
int CompareAppointments (const CBCGPAppointment* pApp1, const CBCGPAppointment* pApp2)
{
	if (pApp1 == pApp2)
	{
		return 0;
	}

	if (pApp1->GetResourceID () < pApp2->GetResourceID ())
	{
		return -1;
	}
	else if (pApp2->GetResourceID () < pApp1->GetResourceID ())
	{
		return 1;
	}

	BOOL bAllOrMulti1 = pApp1->IsMultiDay () || pApp1->IsAllDay ();
	BOOL bAllOrMulti2 = pApp2->IsMultiDay () || pApp2->IsAllDay ();

	if (CBCGPPlannerView::IsOneDay (pApp1->GetStart (), pApp2->GetStart ()))
	{
		if (bAllOrMulti1 && !bAllOrMulti2)
		{
			return -1;
		}
		else if (!bAllOrMulti1 && bAllOrMulti2)
		{
			return 1;
		}
	}

	if (!(bAllOrMulti1 && bAllOrMulti2) || (bAllOrMulti1 && bAllOrMulti2))
	{
		if (pApp1->GetStart () < pApp2->GetStart ())
		{
			return -1;
		}
		else if (pApp2->GetStart () < pApp1->GetStart ())
		{
			return 1;
		}
		else
		{
			COleDateTimeSpan span1 (pApp1->GetDuration ());
			COleDateTimeSpan span2 (pApp2->GetDuration ());

			if (span1 < span2)
			{
				return 1;
			}
			else if (span2 < span1)
			{
				return -1;
			}
		}
	}

	const CString& str1 = pApp1->GetDescription ();
	const CString& str2 = pApp2->GetDescription ();

	if (!str1.IsEmpty () && !str2.IsEmpty ())
	{
		if (_totupper(str1[0]) == _totupper(str2[0]))
		{
			return str1.Compare (str2);
		}
	}

	return str1.CompareNoCase (str2);
}

void SortAppointments (XBCGPAppointmentArray& a, int size)
{
	int i, j;
	int lb, ub;
	int lbstack[c_MAXSTACK], ubstack[c_MAXSTACK];

	int stackpos = 1;
	int ppos;
	CBCGPAppointment* pivot = NULL;

	lbstack[1] = 0;
	ubstack[1] = size-1;

	do
	{
		lb = lbstack[stackpos];
		ub = ubstack[stackpos];
		stackpos--;

		do
		{
			ppos = (lb + ub) >> 1;
			i = lb;
			j = ub;
			pivot = a[ppos];

			do
			{
				while (CompareAppointments (a[i], pivot) == -1) i++;
				while (CompareAppointments (pivot, a[j]) == -1) j--;

				if (i <= j)
				{
					if (i < j)
					{
						CBCGPAppointment* temp = a[i]; 
						a[i] = a[j];
						a[j] = temp;
					}

					i++;
					j--;
				}
			}
			while (i <= j);

			if (i < ppos)
			{
				if (i < ub)
				{
					stackpos++;
					lbstack[ stackpos ] = i;
					ubstack[ stackpos ] = ub;
				}

				ub = j;
			}
			else
			{
				if (j > lb)
				{ 
					stackpos++;
					lbstack[ stackpos ] = lb;
					ubstack[ stackpos ] = j;
				}

				lb = i;
			}

		}
		while (lb < ub);

	}
	while ( stackpos != 0 );
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPAppointmentEdit window

class CBCGPAppointmentEdit : public CEdit
{
// Construction
public:
	CBCGPAppointmentEdit(CBCGPAppointment& rApp);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPAppointmentEdit)
	protected:
	virtual BOOL CBCGPAppointmentEdit::PreTranslateMessage(MSG* pMsg);
	//virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPAppointmentEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPAppointmentEdit)
	afx_msg void OnKillfocus();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
private:
	CBCGPAppointment&	m_rAppointment;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPAppointmentEdit

CBCGPAppointmentEdit::CBCGPAppointmentEdit(CBCGPAppointment& rApp)
	: CEdit          ()
	, m_rAppointment (rApp)
{
}

CBCGPAppointmentEdit::~CBCGPAppointmentEdit()
{
}

BOOL CBCGPAppointmentEdit::PreTranslateMessage(MSG* pMsg)
{
	BOOL bHandled = CEdit::PreTranslateMessage(pMsg);

	if (!bHandled)
	{
		if (pMsg->message >= WM_KEYFIRST &&
			pMsg->message <= WM_KEYLAST)
		{
			switch (pMsg->wParam)
			{
			case VK_RETURN:
			case VK_TAB:
				m_rAppointment.OnEndEdit (FALSE);
				return TRUE;

			case VK_ESCAPE:
				m_rAppointment.OnEndEdit (TRUE);
				return TRUE;
			}
		}
	}

	return bHandled;
}

BEGIN_MESSAGE_MAP(CBCGPAppointmentEdit, CEdit)
	//{{AFX_MSG_MAP(CBCGPAppointmentEdit)
	ON_CONTROL_REFLECT(EN_KILLFOCUS, OnKillfocus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPAppointmentEdit message handlers

void CBCGPAppointmentEdit::OnKillfocus() 
{
	m_rAppointment.OnEndEdit (FALSE);
}

// TIME_NOMINUTESORSECONDS	- Does not use minutes or seconds. 
// TIME_NOSECONDS			- Does not use seconds. 
// TIME_NOTIMEMARKER		- Does not use a time marker. 
// TIME_FORCE24HOURFORMAT	- Always uses a 24-hour time format.
CString CBCGPAppointment::GetFormattedTimeString (const COleDateTime& time, BOOL dwFlags, BOOL bSystem)
{
	CString strFormat;

	if (!bSystem)
	{
		const int c_Size = 10;
		TCHAR szLocaleData[c_Size];

		BOOL b24Hours = (dwFlags & TIME_FORCE24HOURFORMAT) == TIME_FORCE24HOURFORMAT;

		if (!b24Hours)
		{
			// Time format specifier.
			::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_ITIME, szLocaleData, c_Size);

			switch (szLocaleData [0])
			{
			case TCHAR('0'):	// AM / PM 12-hour format.
				b24Hours = FALSE;
				break;
			case TCHAR('1'):	// 24-hour format.
				b24Hours = TRUE;
				break;
			}
		}

		strFormat = b24Hours ? _T("H") : _T("h");

		// Specifier for leading zeros in time fields.
		::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_ITLZERO, szLocaleData, c_Size);
		switch (szLocaleData [0])
		{
		case TCHAR('0'):	// No leading zeros for hours.
			break;
		case TCHAR('1'):	// Leading zeros for hours.
			strFormat += strFormat[0];
			break;
		}

		CString strTemp;

		// Character(s) for the time separator.
		::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_STIME, szLocaleData, c_Size);
		strTemp = szLocaleData;
		strTemp = _T("'") + strTemp + _T("'");

		if ((dwFlags & TIME_NOMINUTESORSECONDS) == 0)
		{
			strFormat += strTemp + _T("mm");

			if ((dwFlags & TIME_NOSECONDS) == 0)
			{
				strFormat += strTemp + _T("ss");
			}
		}

	/*
		//not used in MSVC 6.0

		// Specifier indicating whether the time marker should be used for 12-hour, 24-hour, 
		// or both types of clock settings.
		::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_ITIMEMARKERUSE, szLocaleData, c_Size);
		switch (szLocaleData [0])
		{
		case '0':	// Use with 12-hour clock.
			break;
		case '1':	// Use with 24-hour clock.
			break;
		case '2':	// Use with both 12-hour and 24-hour clocks.
			break;
		case '3':	// Never use.
			break;
		}
	*/	

		if (!b24Hours || (dwFlags & TIME_NOTIMEMARKER) != 0)
		{
			szLocaleData[0] = (TCHAR)0;

			if (time.GetHour () < 12)
			{
				// String for the AM designator.
				::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_S1159, szLocaleData, c_Size);
			}
			else
			{
				// String for the PM designator.
				::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_S2359, szLocaleData, c_Size);
			}

			strTemp = szLocaleData;
			strTemp = _T("'") + strTemp + _T("'");

			if (!strTemp.IsEmpty ())
			{
				// Specifier indicating whether the time marker string (AM or PM) precedes or follows the time string.
				::GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_ITIMEMARKPOSN, szLocaleData, c_Size);
				switch (szLocaleData [0])
				{
				case TCHAR('0'):	// Use as suffix.
					strFormat += strTemp;
					break;
				case TCHAR('1'):	// Use as prefix.
					strFormat = strTemp + strFormat;
					break;
				}
			}
		}
	}

	CString strResult;

	SYSTEMTIME st;
	time.GetAsSystemTime (st);

	::GetTimeFormat (LOCALE_USER_DEFAULT, dwFlags, &st, bSystem ? NULL : (LPCTSTR)strFormat, 
		strResult.GetBuffer (_MAX_PATH), _MAX_PATH);

	strResult.ReleaseBuffer ();

	return strResult;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPAppointment object

IMPLEMENT_SERIAL(CBCGPAppointment, CObject, VERSIONABLE_SCHEMA | 1)

CBCGPAppointment::CBCGPAppointment ()
	: m_clrBackgroung (CLR_DEFAULT)
	, m_clrForeground (CLR_DEFAULT)
	, m_clrDuration   (CLR_DEFAULT)
{
	_CommonConstruct ();
}

CBCGPAppointment::CBCGPAppointment
(
	const COleDateTime& dtStart,
	const COleDateTime& dtFinish,
	const CString& strText,
	COLORREF clrBackground /*= CLR_DEFAULT*/,
	COLORREF clrForeground /*= CLR_DEFAULT*/,
	COLORREF clrDuration /*= CLR_DEFAULT*/
)
	: m_strDescription (strText)
	, m_clrBackgroung  (clrBackground)
	, m_clrForeground  (clrForeground)
	, m_clrDuration    (clrDuration)
{
	_CommonConstruct ();

	SetInterval (dtStart, dtFinish);
}

CBCGPAppointment::~CBCGPAppointment ()
{
	DestroyEdit ();

	if (m_RecurrenceObject != NULL)
	{
		delete m_RecurrenceObject;
		m_RecurrenceObject = NULL;
	}
}

void CBCGPAppointment::_CommonConstruct()
{
	m_bAllDay       = FALSE;

	m_pWndInPlace	= NULL;

	m_rectDraw.SetRectEmpty ();
	m_rectPrint.SetRectEmpty ();
	m_rectEdit.SetRectEmpty ();
	m_rectEditHitTest.SetRectEmpty ();

	m_bVisibleDraw   = TRUE;
	m_bVisiblePrint  = TRUE;

	m_bToolTipNeeded = FALSE;

	m_bSelected      = FALSE;

	m_lstImages.RemoveAll ();

	m_Recurrence       = FALSE;
	m_RecurrenceID     = 0;
	m_RecurrenceDate   = COleDateTime ();
	m_RecurrenceEcp    = FALSE;
	m_RecurrenceClone  = FALSE;
	m_RecurrenceObject = NULL;

	m_pPrinter         = NULL;
	m_ResourceID       = CBCGPAppointmentBaseMultiStorage::e_UnknownResourceID;
}

COleDateTimeSpan CBCGPAppointment::GetDuration () const
{
	COleDateTimeSpan span (m_dtFinish - m_dtStart);

	if (IsAllDay ())
	{
		span += 1.0;
	}

	return span;
}

CString CBCGPAppointment::GetClipboardText () const
{
	return m_strDescription;
}

CString CBCGPAppointment::GetToolTipText () const
{
	CString strToolTip;

	if (!IsAllDay ())
	{
		strToolTip = m_strStart;

		if (m_dtStart != m_dtFinish)
		{
			strToolTip += _T("-");
			strToolTip += m_strFinish;
		}
	}

	if (!m_strDescription.IsEmpty ())
	{
		if (!strToolTip.IsEmpty ())
		{
			strToolTip += _T("\r\n");
		}

		if (m_strDescription.GetLength () <= 256)
		{
			strToolTip += m_strDescription;
		}
		else
		{
			strToolTip += m_strDescription.Left (256);
		}
	}

	return strToolTip;
}

void CBCGPAppointment::OnDraw (CDC* pDC, CBCGPPlannerView* pView)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pView);

	if (pDC == NULL || pDC->GetSafeHdc () == NULL || pView == NULL)
	{
		return;
	}

	if (m_DSDraw.IsEmpty ())
	{
		if (!m_rectDraw.IsRectEmpty () && m_bVisibleDraw)
		{
			CBCGPAppointmentDrawStructEx ds;
			ds.SetRect (m_rectDraw);
			ds.SetBorder (CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_ALL);
			ds.SetToolTipNeeded (m_bToolTipNeeded);

			pView->DrawAppointment (pDC, this, &ds);

			m_bToolTipNeeded  = ds.IsToolTipNeeded ();
			m_rectEdit        = ds.GetRectEdit ();
			m_rectEditHitTest = ds.GetRectEditHitTest ();
		}
	}
	else
	{
		for (int i = 0; i < m_DSDraw.GetCount (); i++)
		{
			CBCGPAppointmentDrawStructEx* pDS = 
				(CBCGPAppointmentDrawStructEx*)m_DSDraw.GetByIndex (i);

			if (pDS != NULL && pDS->IsVisible ())
			{
				pView->DrawAppointment (pDC, this, pDS);
			}
		}
	}
}

void CBCGPAppointment::OnDraw (CDC* pDC, CBCGPPlannerView* pView, const COleDateTime& date)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (pView);

	if (pDC == NULL || pDC->GetSafeHdc () == NULL || pView == NULL)
	{
		return;
	}

	if (m_DSDraw.IsEmpty () || date == COleDateTime ())
	{
		OnDraw (pDC, pView);
	}
	else
	{
		CBCGPAppointmentDrawStructEx* pDS = 
			(CBCGPAppointmentDrawStructEx*)m_DSDraw.Get (date);

		if (pDS != NULL && pDS->IsVisible ())
		{
			pView->DrawAppointment (pDC, this, pDS);
		}
	}
}

void CBCGPAppointment::SetPrinter (CBCGPPlannerPrint* pPrinter)
{
	m_pPrinter = pPrinter;
}

void CBCGPAppointment::OnPrint (CDC* pDC, COLORREF /*clrPen*/, 
		COLORREF /*clrText*/, const CSize& /*szOnePoint*/, int /*nTimeDeltaInMinuts*/)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pPrinter);

	if (pDC == NULL || pDC->GetSafeHdc () == NULL || m_pPrinter == NULL)
	{
		return;
	}

	if (m_DSPrint.IsEmpty ())
	{
		if (!m_rectPrint.IsRectEmpty () && m_bVisiblePrint)
		{
			CBCGPAppointmentDrawStruct ds;
			ds.SetRect (m_rectPrint);
			ds.SetBorder (CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_ALL);

			m_pPrinter->DrawAppointment (pDC, this, &ds);
		}
	}
	else
	{
		for (int i = 0; i < m_DSPrint.GetCount (); i++)
		{
			CBCGPAppointmentDrawStruct* pDS = m_DSPrint.GetByIndex (i);

			if (pDS != NULL && pDS->IsVisible ())
			{
				m_pPrinter->DrawAppointment (pDC, this, pDS);
			}
		}
	}
}

void CBCGPAppointment::SetInterval (const COleDateTime& dtStart, const COleDateTime& dtFinish)
{
	ASSERT(dtStart <= dtFinish);

	if (m_dtStart != dtStart || m_dtFinish != dtFinish)
	{
		if (m_dtStart != dtStart)
		{
			m_dtStart = dtStart;
		}

		if (m_dtFinish != dtFinish)
		{
			m_dtFinish = dtFinish;
		}
		
		if (m_bAllDay)
		{
			m_dtStart.SetDate (m_dtStart.GetYear (), m_dtStart.GetMonth (), m_dtStart.GetDay ());
			m_dtFinish.SetDate(m_dtFinish.GetYear (), m_dtFinish.GetMonth (), m_dtFinish.GetDay ());
		}

		_UpdateTime ();
	}

	ASSERT(m_dtStart.GetStatus () == COleDateTime::valid);
	ASSERT(m_dtFinish.GetStatus () == COleDateTime::valid);
}

void CBCGPAppointment::SetDescription (const CString& strText)
{
	if (m_strDescription != strText)
	{
		m_strDescription = strText;
	}
}

void CBCGPAppointment::SetBackgroundColor (COLORREF clr)
{
	if (m_clrBackgroung != clr)
	{
		m_clrBackgroung = clr;
	}
}

void CBCGPAppointment::SetForegroundColor (COLORREF clr)
{
	if (m_clrForeground != clr)
	{
		m_clrForeground = clr;
	}
}

void CBCGPAppointment::SetDurationColor (COLORREF clr)
{
	if (m_clrDuration != clr)
	{
		m_clrDuration = clr;
	}
}

void CBCGPAppointment::SetAllDay (BOOL bAllDay)
{
	if (m_bAllDay != bAllDay)
	{
		m_bAllDay = bAllDay;

		if (m_bAllDay)
		{
			SetInterval 
				(COleDateTime (m_dtStart.GetYear (), m_dtStart.GetMonth (), m_dtStart.GetDay (),
					0, 0, 0),
				COleDateTime (m_dtFinish.GetYear (), m_dtFinish.GetMonth (), m_dtFinish.GetDay (),
					0, 0, 0));
		}
	}
}

void CBCGPAppointment::SetSelected (BOOL selected)
{
	m_bSelected = selected;
}

void CBCGPAppointment::AddImage (int nImageIndex, BOOL bTail)
{
	if (HasImage (nImageIndex))
	{
		// already exist
		return;
	}

	if (bTail)
	{
		m_lstImages.AddTail (nImageIndex);
	}
	else
	{
		m_lstImages.AddHead (nImageIndex);
	}
}

BOOL CBCGPAppointment::RemoveImage (int nImageIndex)
{
	POSITION pos = m_lstImages.Find (nImageIndex);
	if (pos == NULL)
	{
		return FALSE;
	}

	m_lstImages.RemoveAt (pos);
	return TRUE;
}

void CBCGPAppointment::RemoveAllImages ()
{
	m_lstImages.RemoveAll ();
}

void CBCGPAppointment::_UpdateTimeStrings ()
{
	m_strStart.Empty ();
	m_strFinish.Empty ();

	m_strStart  = GetFormattedTimeString (m_dtStart, TIME_NOSECONDS, FALSE);
	m_strStart.MakeLower ();
	m_strFinish = GetFormattedTimeString (m_dtFinish, TIME_NOSECONDS, FALSE);
	m_strFinish.MakeLower ();
}

void CBCGPAppointment::_UpdateTime ()
{
	_UpdateTimeStrings ();
}

void CBCGPAppointment::Serialize (CArchive& ar)
{
	CObject::Serialize (ar);

    if (ar.IsStoring ())
	{
		ar << m_dtStart;
		ar << m_dtFinish;
		ar << m_strDescription;
		ar << m_bAllDay;
		ar << m_clrBackgroung;
		ar << m_clrDuration;
		ar << m_clrForeground;

		ar << m_Recurrence;

		if (m_Recurrence)
		{
			ar << m_RecurrenceID;
			ar << m_RecurrenceDate;
			ar << m_RecurrenceClone;

			if (m_RecurrenceClone)
			{
				ar << m_RecurrenceEcp;
			}
			else
			{
				ar << m_RecurrenceObject;
			}
		}

		ar << m_ResourceID;
	}
    else
	{
		_CommonConstruct ();

		ar >> m_dtStart;
		ar >> m_dtFinish;
		ar >> m_strDescription;

		BOOL bAllDay;
		ar >> bAllDay;
		SetAllDay (bAllDay);

		ar >> m_clrBackgroung;
		ar >> m_clrDuration;
		ar >> m_clrForeground;

		ar >> m_Recurrence;

		if (m_Recurrence)
		{
			ar >> m_RecurrenceID;
			ar >> m_RecurrenceDate;
			ar >> m_RecurrenceClone;

			if (m_RecurrenceClone)
			{
				ar >> m_RecurrenceEcp;
			}
			else
			{
				CBCGPRecurrence* pRecurrence = NULL;
				ar >> pRecurrence;

				ASSERT_VALID (pRecurrence);

				SetRecurrence (pRecurrence);
			}
		}

#ifndef _BCGSUITE_
		if (g_pWorkspace == NULL || (g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0xa0000))
		{
			ar >> m_ResourceID;
		}
#else
		ar >> m_ResourceID;
#endif

		_UpdateTime ();
	}

	m_lstImages.Serialize (ar);
}

void CBCGPAppointment::GetProperties (CBCGPAppointmentPropertyList& props) const
{
	props.RemoveAll ();

	props.Add (BCGP_APPOINTMENT_PROPERTY_DATE_START, new CBCGPAppointmentProperty (m_dtStart));
	props.Add (BCGP_APPOINTMENT_PROPERTY_DATE_FINISH, new CBCGPAppointmentProperty (m_dtFinish));
	props.Add (BCGP_APPOINTMENT_PROPERTY_DESCRIPTION, new CBCGPAppointmentProperty (m_strDescription));
	props.Add (BCGP_APPOINTMENT_PROPERTY_COLOR_BACK, new CBCGPAppointmentProperty (m_clrBackgroung));
	props.Add (BCGP_APPOINTMENT_PROPERTY_COLOR_DURATION, new CBCGPAppointmentProperty (m_clrDuration));
	props.Add (BCGP_APPOINTMENT_PROPERTY_COLOR_FORE, new CBCGPAppointmentProperty (m_clrForeground));
	props.Add (BCGP_APPOINTMENT_PROPERTY_IMAGES, new CBCGPAppointmentPropertyImages (m_lstImages));
	props.Add (BCGP_APPOINTMENT_PROPERTY_ALLDAY, new CBCGPAppointmentProperty (m_bAllDay));
}

void CBCGPAppointment::SetProperties (const CBCGPAppointmentPropertyList& props)
{
	if (props.GetCount () > 0)
	{
		CBCGPAppointmentProperty* prop = NULL;

		BOOL bDate = FALSE;

		if (props.PropertyExists (BCGP_APPOINTMENT_PROPERTY_DATE_START))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_APPOINTMENT_PROPERTY_DATE_START);
			m_dtStart = *prop;
			bDate = TRUE;
		}

		if (props.PropertyExists (BCGP_APPOINTMENT_PROPERTY_DATE_FINISH))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_APPOINTMENT_PROPERTY_DATE_FINISH);
			m_dtFinish = *prop;
			bDate = TRUE;
		}

		if (props.PropertyExists (BCGP_APPOINTMENT_PROPERTY_ALLDAY))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_APPOINTMENT_PROPERTY_ALLDAY);
			SetAllDay ((BOOL) *prop);
		}

		if (bDate)
		{
			_UpdateTime ();
		}

		if (props.PropertyExists (BCGP_APPOINTMENT_PROPERTY_DESCRIPTION))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_APPOINTMENT_PROPERTY_DESCRIPTION);
			m_strDescription = *prop;
		}

		if (props.PropertyExists (BCGP_APPOINTMENT_PROPERTY_COLOR_BACK))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_APPOINTMENT_PROPERTY_COLOR_BACK);
			m_clrBackgroung = *prop;
		}

		if (props.PropertyExists (BCGP_APPOINTMENT_PROPERTY_COLOR_DURATION))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_APPOINTMENT_PROPERTY_COLOR_DURATION);
			m_clrDuration = *prop;
		}

		if (props.PropertyExists (BCGP_APPOINTMENT_PROPERTY_COLOR_FORE))
		{
			prop = (CBCGPAppointmentProperty*)props.Get (BCGP_APPOINTMENT_PROPERTY_COLOR_FORE);
			m_clrForeground = *prop;
		}

		if (props.PropertyExists (BCGP_APPOINTMENT_PROPERTY_IMAGES))
		{
			CBCGPAppointmentPropertyImages* propI = 
				(CBCGPAppointmentPropertyImages*)props.Get (BCGP_APPOINTMENT_PROPERTY_IMAGES);
			propI->GetImages (m_lstImages);
		}

		if (m_RecurrenceClone)
		{
			m_RecurrenceEcp = TRUE;
		}
	}
}

BOOL CBCGPAppointment::OnEdit (CWnd* pWndOwner)
{
	ASSERT_VALID (pWndOwner);

	if (pWndOwner == NULL)
	{
		return FALSE;
	}

	if (!IsSelected ())// || !IsVisibleDraw ())
	{
		return FALSE;
	}

	if (IsEditExists ())
	{
		return FALSE;
	}

	m_pWndInPlace = CreateEdit (pWndOwner);

	if (m_pWndInPlace == NULL)
	{
		return FALSE;
	}

	ASSERT (IsEditExists ());

	if (!IsEditExists ())
	{
		return FALSE;
	}

	m_pWndInPlace->SetWindowText (GetDescription ());

	return TRUE;
}

BOOL CBCGPAppointment::OnEndEdit (BOOL bCancel)
{
	if (IsEditExists ())
	{
		CString strText (GetDescription ());

		ASSERT_VALID (m_pWndInPlace);
		m_pWndInPlace->GetWindowText (strText);

		CWnd* pParent = m_pWndInPlace->GetParent ();

		CString strTextOld = GetDescription ();

		if (!bCancel && strText != strTextOld)
		{
			SetDescription (strText);

			BOOL RecurrenceEcpOld = FALSE;
			if (m_RecurrenceClone)
			{
				RecurrenceEcpOld = m_RecurrenceEcp;
				m_RecurrenceEcp  = TRUE;
			}

			CBCGPPlannerManagerCtrl* pCtrl = DYNAMIC_DOWNCAST (
				CBCGPPlannerManagerCtrl, pParent);
			if (pCtrl != NULL)
			{
				if (!pCtrl->UpdateAppointment (this, m_dtStart, FALSE, FALSE))
				{
					SetDescription (strTextOld);
					if (m_RecurrenceClone)
					{
						m_RecurrenceEcp = RecurrenceEcpOld;
					}
				}
			}
		}

		DestroyEdit ();

		if (pParent != NULL)
		{
			pParent->RedrawWindow (NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			pParent->SetFocus ();
		}

		return TRUE;
	}

	return FALSE;
}

void CBCGPAppointment::SetupRectEdit (const CPoint& point)
{
	if (!m_DSDraw.IsEmpty ())
	{
		CBCGPAppointmentDrawStructEx* pDS = 
			(CBCGPAppointmentDrawStructEx*)m_DSDraw.GetFromPoint (point);

		if (pDS != NULL)
		{
			m_rectEdit = pDS->GetRectEdit ();
		}
	}
}

CWnd* CBCGPAppointment::CreateEdit (CWnd* pWndOwner)
{
	ASSERT_VALID (pWndOwner);

	if (!IsEditExists () && pWndOwner != NULL)
	{
		DWORD nStyle = ES_AUTOHSCROLL;

		const CBCGPPlannerManagerCtrl* pCtrl = DYNAMIC_DOWNCAST (
				CBCGPPlannerManagerCtrl, pWndOwner);
		if (pCtrl != NULL)
		{
			int nRowHeight = pCtrl->GetCurrentView ()->GetRowHeight ();

			if (nRowHeight < m_rectEdit.Height ())
			{
				nStyle = ES_MULTILINE | ES_AUTOVSCROLL;
			}
		}

		CBCGPAppointmentEdit* pEdit = new CBCGPAppointmentEdit (*this);

		pEdit->Create (WS_VISIBLE | WS_CHILD | nStyle, m_rectEdit, pWndOwner, 
			CBCGPPlannerManagerCtrl::BCGP_PLANNER_ID_INPLACE);

		pEdit->SetFont (pWndOwner->GetFont ());
		pEdit->SetFocus ();

		return pEdit;
	}

	return NULL;
}

void CBCGPAppointment::DestroyEdit ()
{
	if (IsEditExists ())
	{
		CWnd* pWndInPlace = m_pWndInPlace;
		m_pWndInPlace = NULL;

		pWndInPlace->DestroyWindow ();
		delete pWndInPlace;
	}
}


CBCGPAppointment* CBCGPAppointment::CreateRecurrenceClone 
	(const CBCGPAppointment* pSrc, const COleDateTime& dtDate)
{
	CBCGPAppointment* pApp = NULL;

	ASSERT_VALID(pSrc);

	if (pSrc == NULL || pSrc->m_RecurrenceClone || pSrc->m_RecurrenceID == 0)
	{
		return NULL;
	}

	pApp = (CBCGPAppointment*)(pSrc->GetRuntimeClass ()->CreateObject ());

	ASSERT_VALID(pApp);

	if (pApp != NULL)
	{
		CBCGPAppointmentPropertyList props;
		pSrc->GetProperties (props);
		pApp->SetProperties (props);
		pApp->SetResourceID (pSrc->GetResourceID ());

		COleDateTime dtStart (pApp->m_dtStart);

		COleDateTimeSpan span (pApp->m_dtFinish - dtStart);
		COleDateTime dt (dtDate.GetYear (), dtDate.GetMonth (), dtDate.GetDay (),
						dtStart.GetHour (), dtStart.GetMinute (), dtStart.GetSecond ());

		pApp->SetInterval (dt, dt + span);

		pApp->m_Recurrence       = TRUE;
		pApp->m_RecurrenceID     = pSrc->m_RecurrenceID;
		pApp->m_RecurrenceDate   = dtDate;
		pApp->m_RecurrenceClone  = TRUE;
		pApp->m_RecurrenceEcp    = FALSE;
		pApp->m_RecurrenceObject = NULL;
	}

	return pApp;
}

void CBCGPAppointment::SetRecurrenceRule (const CBCGPRecurrenceBaseRule* pRule)
{
	ASSERT_VALID(pRule);

	if (m_RecurrenceObject != NULL)
	{
		ASSERT_VALID (m_RecurrenceObject);

		m_RecurrenceObject->SetRule (pRule);
		UpdateRecurrence ();
	}
	else
	{
		CBCGPRecurrence* pRecurrence = new CBCGPRecurrence;

		pRecurrence->SetRule (pRule);
		SetRecurrence (pRecurrence);
	}
}

void CBCGPAppointment::SetRecurrence (CBCGPRecurrence* pRecurrence)
{
	RemoveRecurrence ();

	if (pRecurrence != NULL)
	{
		ASSERT_VALID (pRecurrence);

		m_Recurrence       = TRUE;
		m_RecurrenceObject = pRecurrence;

		m_RecurrenceObject->SetAppointment (this);

		UpdateRecurrence ();
	}
}

void CBCGPAppointment::UpdateRecurrence ()
{
	if (m_Recurrence && !m_RecurrenceClone)
	{
		ASSERT_VALID (m_RecurrenceObject);

		CBCGPRecurrenceBaseRule* pRule = m_RecurrenceObject->GetRule ();
		pRule->CorrectStart ();

		SetAllDay (FALSE);
		SetInterval (pRule->GetDateStart () + pRule->GetTimeStart (), 
					 pRule->GetDateStart () + pRule->GetTimeFinish ());
	}
}

void CBCGPAppointment::SetRecurrenceID (DWORD ID)
{
	ASSERT(m_Recurrence);
	ASSERT(!m_RecurrenceClone);

	m_RecurrenceID = ID;
}

void CBCGPAppointment::RemoveRecurrence ()
{
	if (!m_RecurrenceClone && m_RecurrenceObject != NULL)
	{
		delete m_RecurrenceObject;
		m_RecurrenceObject = NULL;
	}

	m_Recurrence      = FALSE;
	m_RecurrenceID    = 0;
	m_RecurrenceDate  = COleDateTime ();
	m_RecurrenceClone = FALSE;
	m_RecurrenceEcp   = FALSE;
}

CBCGPAppointment* CBCGPAppointment::CreateCopy () const
{
	CBCGPAppointment* pApp = 
		(CBCGPAppointment*)GetRuntimeClass ()->CreateObject ();

	CBCGPAppointmentPropertyList props;
	GetProperties (props);
	pApp->SetProperties (props);
	pApp->SetResourceID (GetResourceID ());

	return pApp;
}

BOOL CBCGPAppointment::IsVisibleDraw (const COleDateTime& date /*= COleDateTime ()*/) const
{
	if (date != COleDateTime () && !m_DSDraw.IsEmpty ())
	{
		CBCGPAppointmentDrawStructEx* pDS = 
			(CBCGPAppointmentDrawStructEx*)(m_DSDraw.Get (date));

		if (pDS != NULL)
		{
			return pDS->IsVisible ();
		}
	}

	return m_bVisibleDraw;
}

void CBCGPAppointment::SetVisibleDraw (BOOL visible, const COleDateTime& date /*= COleDateTime ()*/)
{
	if (date != COleDateTime () && !m_DSDraw.IsEmpty ())
	{
		CBCGPAppointmentDrawStructEx* pDS = 
			(CBCGPAppointmentDrawStructEx*)(m_DSDraw.Get (date));

		ASSERT (pDS != NULL);
		if (pDS != NULL)
		{
			pDS->SetVisible (visible);
		}
	}
	else
	{
		m_bVisibleDraw = visible;
	}
}

BOOL CBCGPAppointment::IsVisiblePrint (const COleDateTime& date /*= COleDateTime ()*/) const
{
	if (date != COleDateTime () && !m_DSPrint.IsEmpty ())
	{
		CBCGPAppointmentDrawStruct* pDS = m_DSPrint.Get (date);

		if (pDS != NULL)
		{
			return pDS->IsVisible ();
		}
	}

	return m_bVisiblePrint;
}

void CBCGPAppointment::SetVisiblePrint (BOOL visible, const COleDateTime& date /*= COleDateTime ()*/)
{
	if (date != COleDateTime () && !m_DSPrint.IsEmpty ())
	{
		CBCGPAppointmentDrawStruct* pDS = (m_DSPrint.Get (date));

		ASSERT (pDS != NULL);
		if (pDS != NULL)
		{
			pDS->SetVisible (visible);
		}
	}
	else
	{
		m_bVisiblePrint = visible;
	}
}

BOOL CBCGPAppointment::IsToolTipNeeded (const COleDateTime& date /*= COleDateTime ()*/) const
{
	if (date != COleDateTime () && !m_DSDraw.IsEmpty ())
	{
		CBCGPAppointmentDrawStructEx* pDS = 
			(CBCGPAppointmentDrawStructEx*)(m_DSDraw.Get (date));

		if (pDS != NULL)
		{
			return pDS->IsToolTipNeeded ();
		}
	}

	return m_bToolTipNeeded;
}

void CBCGPAppointment::SetToolTipNeeded (BOOL bToolTipNeeded, const COleDateTime& date /*= COleDateTime ()*/)
{
	if (date != COleDateTime () && !m_DSPrint.IsEmpty ())
	{
		CBCGPAppointmentDrawStructEx* pDS = 
			(CBCGPAppointmentDrawStructEx*)(m_DSDraw.Get (date));

		ASSERT (pDS != NULL);
		if (pDS != NULL)
		{
			pDS->SetToolTipNeeded (bToolTipNeeded);
		}
	}
	else
	{
		m_bToolTipNeeded = bToolTipNeeded;
	}
}

BOOL CBCGPAppointment::PointInRectDraw (const CPoint& point) const
{
	if (!m_DSDraw.IsEmpty ())
	{
		return m_DSDraw.PointInRect (point);
	}

	return m_bVisibleDraw && m_rectDraw.PtInRect (point);
}

BOOL CBCGPAppointment::PointInRectEdit (const CPoint& point) const
{
	if (!m_DSDraw.IsEmpty ())
	{
		return m_DSDraw.PointInRectEdit (point);
	}

	return m_bVisibleDraw && m_rectEdit.PtInRect (point);
}

BOOL CBCGPAppointment::PointInRectEditHitTest (const CPoint& point) const
{
	if (!m_DSDraw.IsEmpty ())
	{
		return m_DSDraw.PointInRectEditHitTest (point);
	}

	return m_bVisibleDraw && m_rectEditHitTest.PtInRect (point);
}

const CRect& CBCGPAppointment::GetRectDraw (const COleDateTime& date /*= COleDateTime ()*/) const
{
	if (date != COleDateTime () && !m_DSDraw.IsEmpty ())
	{
		CBCGPAppointmentDrawStructEx* pDS = 
			(CBCGPAppointmentDrawStructEx*)(m_DSDraw.Get (date));

		if (pDS != NULL)
		{
			return pDS->GetRect ();
		}
	}

	return m_rectDraw;
}

void CBCGPAppointment::SetRectDraw (const CRect& rect, const COleDateTime& date /*= COleDateTime ()*/)
{
	if (date != COleDateTime () && !CBCGPPlannerView::IsOneDay (m_dtStart, m_dtFinish))
	{
		BOOL bEmpty = m_DSDraw.IsEmpty ();
		CBCGPAppointmentDrawStruct* pDS = NULL;

		BOOL bStart  = CBCGPPlannerView::IsOneDay (date, m_dtStart);
		BOOL bFinish = CBCGPPlannerView::IsOneDay (date, m_dtFinish);

		BOOL bClose = FALSE;

		if (!IsAllDay () && 
			m_dtFinish.GetHour () == 0 && m_dtFinish.GetMinute () == 0)
		{
			COleDateTime dt (date);

			if (bFinish || 
				((dt + COleDateTimeSpan (1, 0, 0, 0)) == m_dtFinish))
			{
				if (!bEmpty)
				{
					if (bFinish)
					{
						dt -= COleDateTimeSpan (1, 0, 0, 0);
					}
					else
					{
						m_DSDraw.Add (date, rect);
					}

					pDS = m_DSDraw.Get (dt);
					m_DSDraw.Add (dt + COleDateTimeSpan (1, 0, 0, 0), pDS->GetRect ());

					bClose  = TRUE;
				}

				bFinish = TRUE;
			}
		}

		if (!bClose)
		{
			m_DSDraw.Add (date, rect);
			pDS = m_DSDraw.Get (date);
		}

		if (pDS == NULL)
		{
			return;
		}

		if (bEmpty)
		{
			if (!bStart)
			{
				pDS->SetBorder ((CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER)
					(pDS->GetBorder () &
					~CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START));
			}

			if (bFinish)
			{
				pDS->SetBorder ((CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER)
					(pDS->GetBorder () |
					CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH));
			}
		}
		else
		{
			if (bFinish)
			{
				pDS->SetBorder ((CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER)
					(pDS->GetBorder () |
					CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH));
			}
			else
			{
				pDS->SetBorder ((CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER)
					(pDS->GetBorder () &
					~CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH));
			}
		}
	}
	else
	{
		m_rectDraw = rect;
	}
}

void CBCGPAppointment::ResetDraw ()
{
	m_rectDraw.SetRectEmpty ();
	m_rectEdit.SetRectEmpty ();
	m_rectEditHitTest.SetRectEmpty ();
	m_bVisibleDraw   = FALSE;

	m_bToolTipNeeded = FALSE;

	m_DSDraw.RemoveAll ();
	m_DSDraw.SetConcatenate (TRUE);
}

CRect CBCGPAppointment::GetVisibleRectDraw (BOOL bFirst) const
{
	CRect rect(0, 0, 0, 0);

	if (!m_DSDraw.IsEmpty ())
	{
		rect = m_DSDraw.GetVisibleRect(bFirst);
	}

	if (m_bVisibleDraw && rect.IsRectNull())
	{
		rect = m_rectDraw;
	}

	return rect;
}

const CRect& CBCGPAppointment::GetRectEdit (const COleDateTime& date /*= COleDateTime ()*/) const
{
	if (date != COleDateTime () && !m_DSDraw.IsEmpty ())
	{
		CBCGPAppointmentDrawStructEx* pDS = 
			(CBCGPAppointmentDrawStructEx*)(m_DSDraw.Get (date));

		if (pDS != NULL)
		{
			return pDS->GetRectEdit ();
		}
	}

	return m_rectEdit;
}

const CRect& CBCGPAppointment::GetRectEditHitTest (const COleDateTime& date /*= COleDateTime ()*/) const
{
	if (date != COleDateTime () && !m_DSDraw.IsEmpty ())
	{
		CBCGPAppointmentDrawStructEx* pDS = 
			(CBCGPAppointmentDrawStructEx*)(m_DSDraw.Get (date));

		if (pDS != NULL)
		{
			return pDS->GetRectEditHitTest ();
		}
	}

	return m_rectEditHitTest;
}

const CRect& CBCGPAppointment::GetRectPrint (const COleDateTime& date /*= COleDateTime ()*/) const
{
	if (date != COleDateTime () && !m_DSPrint.IsEmpty ())
	{
		CBCGPAppointmentDrawStruct* pDS = m_DSPrint.Get (date);

		if (pDS != NULL)
		{
			return pDS->GetRect ();
		}
	}

	return m_rectPrint;
}

void CBCGPAppointment::SetRectPrint (const CRect& rect, const COleDateTime& date /*= COleDateTime ()*/)
{
	if (date != COleDateTime () && !CBCGPPlannerView::IsOneDay (m_dtStart, m_dtFinish))
	{
		BOOL bEmpty = m_DSPrint.IsEmpty ();
		CBCGPAppointmentDrawStruct* pDS = NULL;

		BOOL bStart  = CBCGPPlannerView::IsOneDay (date, m_dtStart);
		BOOL bFinish = CBCGPPlannerView::IsOneDay (date, m_dtFinish);

		BOOL bClose = FALSE;

		if (!IsAllDay () && 
			m_dtFinish.GetHour () == 0 && m_dtFinish.GetMinute () == 0)
		{
			COleDateTime dt (date);

			if (bFinish || 
				((dt + COleDateTimeSpan (1, 0, 0, 0)) == m_dtFinish))
			{
				if (!bEmpty)
				{
					if (bFinish)
					{
						dt -= COleDateTimeSpan (1, 0, 0, 0);
					}
					else
					{
						m_DSPrint.Add (date, rect);
					}

					pDS = m_DSPrint.Get (dt);
					m_DSPrint.Add (dt + COleDateTimeSpan (1, 0, 0, 0), pDS->GetRect ());

					bClose  = TRUE;
				}

				bFinish = TRUE;
			}
		}

		if (!bClose)
		{
			m_DSPrint.Add (date, rect);
			pDS = m_DSPrint.Get (date);
		}

		if (pDS == NULL)
		{
			return;
		}

		if (bEmpty)
		{
			if (!bStart)
			{
				pDS->SetBorder ((CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER)
					(pDS->GetBorder () &
					~CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START));
			}

			if (bFinish)
			{
				pDS->SetBorder ((CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER)
					(pDS->GetBorder () |
					CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH));
			}
		}
		else
		{
			if (bFinish)
			{
				pDS->SetBorder ((CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER)
					(pDS->GetBorder () |
					CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH));
			}
			else
			{
				pDS->SetBorder ((CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER)
					(pDS->GetBorder () &
					~CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH));
			}
		}
	}
	else
	{
		m_rectPrint = rect;
	}
}

void CBCGPAppointment::ResetPrint ()
{
	m_rectPrint.SetRectEmpty ();
	m_bVisiblePrint = FALSE;

	m_DSPrint.RemoveAll ();
}

CBCGPAppointment::BCGP_APPOINTMENT_HITTEST 
CBCGPAppointment::HitTest (const CPoint& point) const
{
	BCGP_APPOINTMENT_HITTEST hit = BCGP_APPOINTMENT_HITTEST_NOWHERE;

	if (m_DSDraw.IsEmpty ())
	{
		if (m_bVisibleDraw && m_rectDraw.PtInRect (point))
		{
			CRect rt (m_rectDraw);

			hit = BCGP_APPOINTMENT_HITTEST_INSIDE;

			rt.left = m_rectEdit.left;

			BOOL bAllOrMulti = IsAllDay () || IsMultiDay ();

			if (!bAllOrMulti)
			{
				if (rt.top <= point.y && point.y <= (rt.top + c_HT_PREC))
				{
					hit = BCGP_APPOINTMENT_HITTEST_TOP;
				}
				else if (point.y <= rt.bottom && (rt.bottom - c_HT_PREC) <= point.y)
				{
					hit = BCGP_APPOINTMENT_HITTEST_BOTTOM;
				}

			}
			else
			{
				if (rt.left <= point.x && point.x <= (rt.left + c_HT_PREC))
				{
					hit = BCGP_APPOINTMENT_HITTEST_LEFT;
				}
				else if (point.x <= rt.right && (rt.right - c_HT_PREC) <= point.x)
				{
					hit = BCGP_APPOINTMENT_HITTEST_RIGHT;
				}
			}

			if (hit == BCGP_APPOINTMENT_HITTEST_INSIDE)
			{
				rt.left  = m_rectDraw.left;
				rt.right = m_rectEditHitTest.left;

				if (rt.PtInRect (point))
				{
					hit = BCGP_APPOINTMENT_HITTEST_MOVE;
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < m_DSDraw.GetCount (); i++)
		{
			CBCGPAppointmentDrawStructEx* pDS = 
				(CBCGPAppointmentDrawStructEx*)m_DSDraw.GetByIndex (i);

			if (pDS == NULL)
			{
				continue;
			}

			if (pDS->IsVisible () && pDS->GetRect ().PtInRect (point))
			{
				CRect rt (pDS->GetRect ());

				hit = BCGP_APPOINTMENT_HITTEST_INSIDE;

				rt.left = pDS->GetRectEdit ().left;

				BOOL bAllOrMulti = IsAllDay () || IsMultiDay ();

				if (!bAllOrMulti)
				{
					if (pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START)
					{
						if (rt.top <= point.y && point.y <= (rt.top + c_HT_PREC))
						{
							hit = BCGP_APPOINTMENT_HITTEST_TOP;
						}
					}
					
					if (pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH)
					{
						if (point.y <= rt.bottom && (rt.bottom - c_HT_PREC) <= point.y)
						{
							hit = BCGP_APPOINTMENT_HITTEST_BOTTOM;
						}
					}
				}
				else
				{
					if (pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_START)
					{
						if (rt.left <= point.x && point.x <= (rt.left + c_HT_PREC))
						{
							hit = BCGP_APPOINTMENT_HITTEST_LEFT;
						}
					}
					
					if (pDS->GetBorder () & CBCGPAppointmentDrawStruct::BCGP_APPOINTMENT_DS_BORDER_FINISH)
					{
						if (point.x <= rt.right && (rt.right - c_HT_PREC) <= point.x)
						{
							hit = BCGP_APPOINTMENT_HITTEST_RIGHT;
						}
					}
				}

				if (hit == BCGP_APPOINTMENT_HITTEST_INSIDE)
				{
					rt.left  = pDS->GetRect ().left;
					rt.right = pDS->GetRectEditHitTest ().left;

					if (rt.PtInRect (point))
					{
						hit = BCGP_APPOINTMENT_HITTEST_MOVE;
					}
				}

				break;
			}
		}
	}

	return hit;
}

void CBCGPAppointment::AdjustToolTipRect (CRect& rect, BOOL bResizeVert) const
{
	if (IsAllDay () || IsMultiDay ())
	{
		rect.DeflateRect (c_HT_PREC, 0);
	}
	else if (bResizeVert)
	{
		rect.DeflateRect (0, c_HT_PREC);
	}
}

CString CBCGPAppointment::GetAccName() const
{
	return _T("Appointment");
}

CString CBCGPAppointment::GetAccValue() const
{
	CString strValue(CBCGPPlannerView::GetAccIntervalFormattedString(GetStart (), GetFinish ()));
	strValue += _T(".");

	if (!GetDescription ().IsEmpty ())
	{
		strValue += _T(" Subject ") + GetDescription () + _T(".");
	}

	if (IsRecurrenceClone ())
	{
		strValue += _T(" Recurring appointment.");

		if (IsRecurrenceException ())
		{
			strValue += _T(" Exception to recurring event.");
		}
	}

	return strValue;
}

CString CBCGPAppointment::GetAccDescription() const
{
	return CString();
}

#endif // BCGP_EXCLUDE_PLANNER
