#if !defined(AFX_BCGPAPPOINTMENT_H__54C9C402_F5A3_4B0F_A71D_89E6AA0114D9__INCLUDED_)
#define AFX_BCGPAPPOINTMENT_H__54C9C402_F5A3_4B0F_A71D_89E6AA0114D9__INCLUDED_

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
// BCGPAppointment.h : header file
//

#ifndef __AFXTEMPL_H__
	#pragma warning (disable : 4786)
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPAppointmentDS.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPAppointment object

class CBCGPPlannerClockIcons;
class CBCGPAppointmentPropertyList;
class CBCGPRecurrence;
class CBCGPRecurrenceBaseRule;

class BCGCBPRODLLEXPORT CBCGPAppointment : public CObject
{
	friend class CBCGPPlannerManagerCtrl;
	friend class CBCGPPlannerView;
	friend class CBCGPPlannerViewDay;
	friend class CBCGPPlannerViewWeek;
	friend class CBCGPPlannerViewMonth;
	friend class CBCGPPlannerViewMulti;
	friend class CBCGPPlannerPrint;
	friend class CBCGPPlannerPrintDay;
	friend class CBCGPPlannerPrintWeek;
	friend class CBCGPPlannerPrintMonth;
	friend class CBCGPAppointmentX;

	DECLARE_SERIAL(CBCGPAppointment)

// Construction
public:
	typedef CList<int, int> XImageList;

	enum
	{
		BCGP_PLANNER_SHADOW_DEPTH = 5
	};

	enum BCGP_APPOINTMENT_HITTEST
	{
		BCGP_APPOINTMENT_HITTEST_FIRST   = 0,
		BCGP_APPOINTMENT_HITTEST_NOWHERE = BCGP_APPOINTMENT_HITTEST_FIRST,
		BCGP_APPOINTMENT_HITTEST_INSIDE  = 1, // appointment area
		BCGP_APPOINTMENT_HITTEST_MOVE    = 2, // appointment move area (left side)
		BCGP_APPOINTMENT_HITTEST_TOP     = 3, // appointment top area (top side)
		BCGP_APPOINTMENT_HITTEST_BOTTOM  = 4, // appointment bottom area (bottom side)
		BCGP_APPOINTMENT_HITTEST_LEFT    = 5, // appointment left area (left side)
		BCGP_APPOINTMENT_HITTEST_RIGHT   = 6, // appointment right area (right side)
		BCGP_APPOINTMENT_HITTEST_LAST    = BCGP_APPOINTMENT_HITTEST_RIGHT
	};

	static CString GetFormattedTimeString (const COleDateTime& time, BOOL dwFlags, BOOL bSystem);

	static CBCGPAppointment* CreateRecurrenceClone (const CBCGPAppointment* pSrc, 
		const COleDateTime& dtDate);

public:
	CBCGPAppointment();

	CBCGPAppointment
	(
		const COleDateTime& dtStart,
		const COleDateTime& dtFinish,
		const CString& strText,
		COLORREF clrBackground = CLR_DEFAULT,
		COLORREF clrForeground = CLR_DEFAULT,
		COLORREF clrDuration   = CLR_DEFAULT
	);

	~CBCGPAppointment();

// Attributes
public:

	void SetInterval (const COleDateTime& dtStart, const COleDateTime& dtFinish);
	void SetDescription (const CString& strText);
	void SetBackgroundColor (COLORREF clr);
	void SetForegroundColor (COLORREF clr);
	void SetDurationColor (COLORREF clr);
	void SetAllDay (BOOL bAllDay);
	
	void AddImage (int nImageIndex, BOOL bTail = TRUE);
	BOOL RemoveImage (int nImageIndex);
	void RemoveAllImages ();
	const XImageList& GetImages () const
	{
		return m_lstImages;
	}

	BOOL HasImage (int nImageIndex) const
	{
		return m_lstImages.Find (nImageIndex) != NULL;
	}

	const XImageList& GetImageList () const
	{
		return m_lstImages;
	}

	const COleDateTime& GetStart () const
	{
		return m_dtStart;
	}

	const COleDateTime& GetFinish () const
	{
		return m_dtFinish;
	}

	COleDateTimeSpan GetDuration () const;

	const CString& GetDescription () const
	{
		return m_strDescription;
	}

	COLORREF GetBackgroundColor () const
	{
		return m_clrBackgroung;
	}

	COLORREF GetForegroundColor () const
	{
		return m_clrForeground;
	}

	COLORREF GetDurationColor () const
	{
		return m_clrDuration;
	}

	BOOL IsAllDay () const
	{
		return m_bAllDay;
	}

	BOOL IsMultiDay () const
	{
		return COleDateTimeSpan (0, 24, 0, 0) <= (m_dtFinish - m_dtStart);
	}
	
	virtual CString GetClipboardText () const;
	virtual CString GetToolTipText () const;

	BOOL IsSelected () const
	{
		return m_bSelected;
	}
	void SetSelected (BOOL selected);

	BOOL IsVisibleDraw (const COleDateTime& date = COleDateTime ()) const;
	void SetVisibleDraw (BOOL visible, const COleDateTime& date = COleDateTime ());

	BOOL IsVisiblePrint (const COleDateTime& date = COleDateTime ()) const;
	void SetVisiblePrint (BOOL visible, const COleDateTime& date = COleDateTime ());

	BOOL IsToolTipNeeded (const COleDateTime& date = COleDateTime ()) const;
	void SetToolTipNeeded (BOOL bToolTipNeeded, const COleDateTime& date = COleDateTime ());

	BOOL PointInRectDraw (const CPoint& point) const;
	BOOL PointInRectEdit (const CPoint& point) const;
	BOOL PointInRectEditHitTest (const CPoint& point) const;

	const CRect& GetRectDraw (const COleDateTime& date = COleDateTime ()) const;
	void SetRectDraw (const CRect& rect, const COleDateTime& date = COleDateTime ());
	void ResetDraw ();
	CRect GetVisibleRectDraw (BOOL bFirst = TRUE) const;

	const CRect& GetRectPrint (const COleDateTime& date = COleDateTime ()) const;
	void SetRectPrint (const CRect& rect, const COleDateTime& date = COleDateTime ());
	void ResetPrint ();

	const CRect& GetRectEdit (const COleDateTime& date = COleDateTime ()) const;
	const CRect& GetRectEditHitTest (const COleDateTime& date = COleDateTime ()) const;

	virtual BOOL CanBeStored () const
	{
		return TRUE;
	}

	virtual BOOL OnEndEdit (BOOL bCancel);

	BOOL IsEditExists () const
	{
		return m_pWndInPlace != NULL;
	}

	virtual void GetProperties (CBCGPAppointmentPropertyList& props) const;
	virtual void SetProperties (const CBCGPAppointmentPropertyList& props);

	void SetRecurrenceRule (const CBCGPRecurrenceBaseRule* pRule);
	void UpdateRecurrence ();
	void RemoveRecurrence ();

	BOOL IsRecurrence () const
	{
		return m_Recurrence;
	}

	BOOL IsRecurrenceClone () const
	{
		return m_RecurrenceClone;
	}

	BOOL IsRecurrenceException () const
	{
		return m_RecurrenceEcp;
	}

	void SetRecurrenceID (DWORD ID);
	DWORD GetRecurrenceID () const
	{
		return m_RecurrenceID;
	}

	const COleDateTime& GetRecurrenceDate () const
	{
		return m_RecurrenceDate;
	}

	CBCGPRecurrence* GetRecurrence () const
	{
		ASSERT (m_Recurrence && !m_RecurrenceClone);
		return m_RecurrenceObject;
	}

	CBCGPAppointment* CreateCopy () const;

	virtual BCGP_APPOINTMENT_HITTEST HitTest (const CPoint& point) const;
	void AdjustToolTipRect (CRect& rect, BOOL bResizeVert = TRUE) const;

	const CBCGPAppointmentDSExMap& GetDSDraw() const
	{
		return m_DSDraw;
	}

	const CBCGPAppointmentDSMap& GetDSPrint() const
	{
		return m_DSPrint;
	}

	UINT GetResourceID () const
	{
		return m_ResourceID;
	}
	void SetResourceID (UINT nResourceID)
	{
		m_ResourceID = nResourceID;
	}

// Overrides
protected:
	virtual void OnDraw (CDC* pDC, CBCGPPlannerView* pView);
	virtual void OnDraw (CDC* pDC, CBCGPPlannerView* pView, const COleDateTime& date);

	virtual void OnPrint (CDC* pDC, COLORREF clrPen = RGB (0, 0, 0), 
		COLORREF clrText = RGB (0, 0, 0), const CSize& szOnePoint = CSize (1, 1),
		int nTimeDeltaInMinuts = 0);

	virtual BOOL OnEdit (CWnd* pWndOwner);

	void SetupRectEdit (const CPoint& point);
	virtual CWnd* CreateEdit (CWnd* pWndOwner);
	virtual void DestroyEdit ();

	virtual void Serialize (CArchive& ar);

	void SetRecurrence (CBCGPRecurrence* pRecurrence);

	void SetPrinter (CBCGPPlannerPrint* pPrinter);

	CBCGPAppointmentDSExMap& GetDSDraw()
	{
		return m_DSDraw;
	}

	CBCGPAppointmentDSMap& GetDSPrint()
	{
		return m_DSPrint;
	}
	
	virtual CString GetAccName() const;
	virtual CString GetAccValue() const;
	virtual CString GetAccDescription() const;

protected:
	COleDateTime	m_dtStart;
	COleDateTime	m_dtFinish;

	CString			m_strDescription;
	XImageList		m_lstImages;

	COLORREF		m_clrBackgroung;
	COLORREF		m_clrDuration;
	COLORREF		m_clrForeground;

	BOOL			m_bSelected;
	BOOL			m_bAllDay;

	CRect			m_rectDraw;
	CRect			m_rectPrint;
	CRect			m_rectEdit;
	CRect			m_rectEditHitTest;

	BOOL			m_bVisibleDraw;
	BOOL			m_bVisiblePrint;
	BOOL			m_bToolTipNeeded;

	CBCGPAppointmentDSExMap m_DSDraw;
	CBCGPAppointmentDSMap  m_DSPrint;

	CWnd*			m_pWndInPlace;

	BOOL					m_Recurrence;
	DWORD					m_RecurrenceID;
	BOOL					m_RecurrenceClone;
	COleDateTime			m_RecurrenceDate;
	BOOL					m_RecurrenceEcp;
	CBCGPRecurrence*		m_RecurrenceObject;

	CBCGPPlannerPrint*		m_pPrinter;
	
	UINT			m_ResourceID;

private:
	void _CommonConstruct();

	void _UpdateTimeStrings ();

	void _UpdateTime ();

	CString			m_strStart;
	CString			m_strFinish;
};

typedef CArray<CBCGPAppointment*, CBCGPAppointment*> XBCGPAppointmentArray;
typedef CList<CBCGPAppointment*, CBCGPAppointment*> XBCGPAppointmentList;

void BCGCBPRODLLEXPORT SortAppointments (XBCGPAppointmentArray& a, int size);

#endif // BCGP_EXCLUDE_PLANNER

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPAPPOINTMENT_H__54C9C402_F5A3_4B0F_A71D_89E6AA0114D9__INCLUDED_)
