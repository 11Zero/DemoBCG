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
// BCGPAppointmentProperty.h: interface for the CBCGPAppointmentProperty class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPAPPOINTMENTPROPERTY_H__173F7DE3_FBE9_49C9_9731_DD028EF27720__INCLUDED_)
#define AFX_BCGPAPPOINTMENTPROPERTY_H__173F7DE3_FBE9_49C9_9731_DD028EF27720__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXTEMPL_H__
	#pragma warning (disable : 4786)
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "comdef.h"
#include "BCGPAppointment.h"

class BCGCBPRODLLEXPORT CBCGPAppointmentBaseProperty : public CObject
{
	DECLARE_SERIAL(CBCGPAppointmentBaseProperty)

public:
	CBCGPAppointmentBaseProperty();
	virtual ~CBCGPAppointmentBaseProperty();

	virtual BOOL IsValid () const;

	virtual const CBCGPAppointmentBaseProperty& operator = (const CBCGPAppointmentBaseProperty& rSrc);
	virtual BOOL operator == (const CBCGPAppointmentBaseProperty& rProp) const;
	virtual BOOL operator != (const CBCGPAppointmentBaseProperty& rProp) const;
};

class BCGCBPRODLLEXPORT CBCGPAppointmentProperty : public CBCGPAppointmentBaseProperty
{
	DECLARE_SERIAL(CBCGPAppointmentProperty)

public:
	CBCGPAppointmentProperty();
	CBCGPAppointmentProperty(const CBCGPAppointmentProperty& value);
	CBCGPAppointmentProperty(const COleDateTime& value);
	CBCGPAppointmentProperty(const COleDateTimeSpan& value);
	CBCGPAppointmentProperty(long value);
	CBCGPAppointmentProperty(BOOL value);
	CBCGPAppointmentProperty(const CString& value);
	CBCGPAppointmentProperty(COLORREF value);
	CBCGPAppointmentProperty(const CBCGPAppointment::XImageList& value);

	virtual ~CBCGPAppointmentProperty();

	virtual BOOL IsValid () const;

	virtual void Serialize (CArchive& ar);

	void SetDateTime (const COleDateTime& value);
	void SetDateTimeSpan (const COleDateTimeSpan& value);
	void SetLong (long value);
	void SetBOOL (BOOL value);
	void SetString (const CString& value);
	void SetCOLORREF (COLORREF value);

	COleDateTime     GetDateTime () const;
	COleDateTimeSpan GetDateTimeSpan () const;
	long             GetLong () const;
	BOOL             GetBOOL () const;
	CString          GetString () const;
	COLORREF         GetCOLORREF () const;

	VARTYPE GetType () const;

	const CBCGPAppointmentProperty& operator = (const COleDateTime& value);
	const CBCGPAppointmentProperty& operator = (const COleDateTimeSpan& value);
	const CBCGPAppointmentProperty& operator = (long value);
	const CBCGPAppointmentProperty& operator = (BOOL value);
	const CBCGPAppointmentProperty& operator = (const CString& value);
	const CBCGPAppointmentProperty& operator = (COLORREF value);

	virtual BOOL operator == (const CBCGPAppointmentBaseProperty& rProp) const;

	operator COleDateTime () const;
	operator COleDateTimeSpan () const;
	operator BOOL () const;
	operator CString () const;
	operator COLORREF () const;

	virtual const CBCGPAppointmentBaseProperty& operator = (const CBCGPAppointmentBaseProperty& rSrc);

protected:
	_variant_t m_Value;
};

class CBCGPAppointmentPropertyImages : public CBCGPAppointmentBaseProperty
{
	DECLARE_SERIAL(CBCGPAppointmentPropertyImages)

public:
	CBCGPAppointmentPropertyImages();
	CBCGPAppointmentPropertyImages(const CBCGPAppointmentPropertyImages& value);
	CBCGPAppointmentPropertyImages(const CBCGPAppointment::XImageList& value);

	virtual ~CBCGPAppointmentPropertyImages();

	virtual BOOL IsValid () const;

	virtual void Serialize (CArchive& ar);

	void SetImages (const CBCGPAppointment::XImageList& value);
	void GetImages (CBCGPAppointment::XImageList& value) const;

	const CBCGPAppointmentPropertyImages& operator = (const CBCGPAppointment::XImageList& value);

	virtual BOOL operator == (const CBCGPAppointmentBaseProperty& rProp) const;

	virtual const CBCGPAppointmentBaseProperty& operator = (const CBCGPAppointmentBaseProperty& rSrc);

private:
	CBCGPAppointment::XImageList m_List;
};

#define BCGP_APPOINTMENT_PROPERTY_FIRST          0
#define BCGP_APPOINTMENT_PROPERTY_DATE_START     BCGP_APPOINTMENT_PROPERTY_FIRST
#define BCGP_APPOINTMENT_PROPERTY_DATE_FINISH    BCGP_APPOINTMENT_PROPERTY_FIRST + 1
#define BCGP_APPOINTMENT_PROPERTY_DESCRIPTION    BCGP_APPOINTMENT_PROPERTY_FIRST + 2
#define BCGP_APPOINTMENT_PROPERTY_COLOR_BACK     BCGP_APPOINTMENT_PROPERTY_FIRST + 3
#define BCGP_APPOINTMENT_PROPERTY_COLOR_DURATION BCGP_APPOINTMENT_PROPERTY_FIRST + 4
#define BCGP_APPOINTMENT_PROPERTY_COLOR_FORE     BCGP_APPOINTMENT_PROPERTY_FIRST + 5
#define BCGP_APPOINTMENT_PROPERTY_IMAGES         BCGP_APPOINTMENT_PROPERTY_FIRST + 6
#define BCGP_APPOINTMENT_PROPERTY_ALLDAY         BCGP_APPOINTMENT_PROPERTY_FIRST + 7
#define BCGP_APPOINTMENT_PROPERTY_LAST           BCGP_APPOINTMENT_PROPERTY_FIRST + 20

class BCGCBPRODLLEXPORT CBCGPAppointmentPropertyList : public CObject
{
	DECLARE_SERIAL(CBCGPAppointmentPropertyList)

public:
	CBCGPAppointmentPropertyList();
	CBCGPAppointmentPropertyList(const CBCGPAppointmentPropertyList& rProps);
	virtual ~CBCGPAppointmentPropertyList();

	BOOL Add (DWORD ID, CBCGPAppointmentBaseProperty* pProperty);
	BOOL Set (DWORD ID, CBCGPAppointmentBaseProperty* pProperty);
	const CBCGPAppointmentBaseProperty* Get (DWORD ID) const;
	BOOL Remove (DWORD ID);
	void RemoveAll ();

	BOOL PropertyExists (DWORD ID) const;

	int GetCount () const;
	POSITION GetStart () const;
	void GetNext (POSITION& rNextPosition, DWORD& ID, CBCGPAppointmentBaseProperty*& rProp) const;


	void CopyFrom (const CBCGPAppointmentPropertyList& rSrc);

	virtual void Serialize (CArchive& ar);

	CBCGPAppointmentBaseProperty*& operator [] (DWORD ID);

	virtual BOOL operator == (const CBCGPAppointmentPropertyList& rProps) const;
	virtual BOOL operator != (const CBCGPAppointmentPropertyList& rProp) const;

private:
	typedef CMap<DWORD, DWORD, CBCGPAppointmentBaseProperty*, CBCGPAppointmentBaseProperty*>
		XBCGPAppointmentProperties;

	XBCGPAppointmentProperties	m_Properties;
};

#endif // BCGP_EXCLUDE_PLANNER

#endif // !defined(AFX_BCGPAPPOINTMENTPROPERTY_H__173F7DE3_FBE9_49C9_9731_DD028EF27720__INCLUDED_)
