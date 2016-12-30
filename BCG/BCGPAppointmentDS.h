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
// BCGPAppointmentDS.h: interface for the CBCGPAppointmentDrawStruct class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPAPPOINTMENTDS_H__32245881_A691_493F_90EB_571F85748BB2__INCLUDED_)
#define AFX_BCGPAPPOINTMENTDS_H__32245881_A691_493F_90EB_571F85748BB2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXTEMPL_H__
	#pragma warning (disable : 4786)
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

class CBCGPPlannerManagerCtrl;
class CBCGPPlannerView;
class CBCGPPlannerPrint;
class CBCGPAppointment;

class BCGCBPRODLLEXPORT CBCGPAppointmentDrawStruct
{
public:
	enum BCGP_APPOINTMENT_DS_BORDER
	{
		BCGP_APPOINTMENT_DS_BORDER_NONE  = 0x00,
		BCGP_APPOINTMENT_DS_BORDER_START = 0x01,
		BCGP_APPOINTMENT_DS_BORDER_FINISH  = 0x02,
		BCGP_APPOINTMENT_DS_BORDER_ALL   = 0x03
	};

public:
	CBCGPAppointmentDrawStruct();
	virtual ~CBCGPAppointmentDrawStruct();

public:
	virtual void Reset ();

	const CRect& GetRect () const
	{
		return m_Rect;
	}
	void SetRect (const CRect& rect);

	BCGP_APPOINTMENT_DS_BORDER GetBorder () const
	{
		return m_Border;
	}
	void SetBorder (BCGP_APPOINTMENT_DS_BORDER border);

	BOOL IsVisible () const
	{
		return m_Visible;
	}
	void SetVisible (BOOL visible);

	const CBCGPAppointmentDrawStruct& operator = (const CBCGPAppointmentDrawStruct& rDS);

protected:
	CRect						m_Rect;
	BCGP_APPOINTMENT_DS_BORDER	m_Border;
	BOOL						m_Visible;
};


class BCGCBPRODLLEXPORT CBCGPAppointmentDrawStructEx: public CBCGPAppointmentDrawStruct
{
public:
	CBCGPAppointmentDrawStructEx();
	virtual ~CBCGPAppointmentDrawStructEx();

public:
	virtual void Reset ();

	BOOL IsToolTipNeeded () const
	{
		return m_bToolTipNeeded;
	}
	void SetToolTipNeeded (BOOL bToolTipNeeded);

	const CRect& GetRectEdit () const
	{
		return m_rectEdit;
	}
	void SetRectEdit (const CRect& rect);

	const CRect& GetRectEditHitTest () const
	{
		return m_rectEditHitTest;
	}
	void SetRectEditHitTest (const CRect& rect);

	const CBCGPAppointmentDrawStructEx& operator = (const CBCGPAppointmentDrawStructEx& rDS);

protected:
	BOOL						m_bToolTipNeeded;
	CRect						m_rectEdit;
	CRect						m_rectEditHitTest;
};


class BCGCBPRODLLEXPORT CBCGPAppointmentDSMap
{
	friend class CBCGPPlannerManagerCtrl;
	friend class CBCGPPlannerView;
	friend class CBCGPPlannerPrint;
	friend class CBCGPAppointment;

public:
	class XDateInterval
	{
	public:
		COleDateTime m_date1;
		COleDateTime m_date2;

		CBCGPAppointmentDrawStruct* m_pDS;

		XDateInterval ()
			: m_pDS (NULL)
		{
		}

		XDateInterval (const COleDateTime& date, CBCGPAppointmentDrawStruct* pDS)
			: m_date1 (date)
			, m_date2 (date)
			, m_pDS   (pDS)
		{
		}

		XDateInterval (const COleDateTime& date1, const COleDateTime& date2, 
			CBCGPAppointmentDrawStruct* pDS)
			: m_date1 (date1)
			, m_date2 (date2)
			, m_pDS   (pDS)
		{
		}

		~XDateInterval ()
		{
		}
	};

	typedef CArray<XDateInterval, XDateInterval&> XDSMap;

public:
	CBCGPAppointmentDSMap ();
	virtual ~CBCGPAppointmentDSMap ();

	BOOL IsEmpty () const
	{
		return m_DSMap.GetSize () == 0;
	}
	int  GetCount () const
	{
		return (int) m_DSMap.GetSize ();
	}
	BOOL PointInRect (const CPoint& point) const;

	BOOL IsConcatenate () const
	{
		return m_bConcatenate;
	};
	void SetConcatenate (BOOL bConcatenate)
	{
		if (IsEmpty ())
		{
			m_bConcatenate = bConcatenate;
		}
	}

	virtual void Add (const COleDateTime& date, const CRect& rect);
	CBCGPAppointmentDrawStruct* Get (const COleDateTime& date) const;
	CBCGPAppointmentDrawStruct* GetFromPoint (const CPoint& point) const;
	CBCGPAppointmentDrawStruct* GetByIndex (UINT index) const
	{
		return m_DSMap[index].m_pDS;
	}
	XDateInterval& operator [] (UINT index)
	{
		return m_DSMap[index];
	}

	void RemoveAll ();
	
	CRect GetVisibleRect(BOOL bFirst = TRUE) const;

protected:
	virtual CBCGPAppointmentDrawStruct* CreateStruct () const;

protected:
	XDSMap m_DSMap;
	BOOL   m_bConcatenate;
};

class BCGCBPRODLLEXPORT CBCGPAppointmentDSExMap : public CBCGPAppointmentDSMap
{
public:
	CBCGPAppointmentDSExMap ();
	virtual ~CBCGPAppointmentDSExMap ();

	virtual CBCGPAppointmentDrawStruct* CreateStruct () const;

	BOOL PointInRectEdit (const CPoint& point) const;
	BOOL PointInRectEditHitTest (const CPoint& point) const;
};

#endif // BCGP_EXCLUDE_PLANNER

#endif // !defined(AFX_BCGPAPPOINTMENTDS_H__32245881_A691_493F_90EB_571F85748BB2__INCLUDED_)
