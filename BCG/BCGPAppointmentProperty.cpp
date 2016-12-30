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
// BCGPAppointmentProperty.cpp: implementation of the CBCGPAppointmentProperty class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPAppointmentProperty.h"

#ifndef BCGP_EXCLUDE_PLANNER

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CBCGPAppointmentBaseProperty, CObject, VERSIONABLE_SCHEMA | 1)

CBCGPAppointmentBaseProperty::CBCGPAppointmentBaseProperty()
{
}

CBCGPAppointmentBaseProperty::~CBCGPAppointmentBaseProperty()
{
}

BOOL CBCGPAppointmentBaseProperty::IsValid () const
{
	return FALSE;
}

BOOL CBCGPAppointmentBaseProperty::operator == (const CBCGPAppointmentBaseProperty& rProp) const
{
	return &rProp == this;
}

BOOL CBCGPAppointmentBaseProperty::operator != (const CBCGPAppointmentBaseProperty& rProp) const
{
	return !(*this == rProp);
}

const CBCGPAppointmentBaseProperty&
CBCGPAppointmentBaseProperty::operator = (const CBCGPAppointmentBaseProperty& /*rProp*/)
{
	return *this;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CBCGPAppointmentProperty, CBCGPAppointmentBaseProperty, VERSIONABLE_SCHEMA | 1)

CBCGPAppointmentProperty::CBCGPAppointmentProperty()
{
}

CBCGPAppointmentProperty::CBCGPAppointmentProperty(const CBCGPAppointmentProperty& value)
	: m_Value (value.m_Value)
{
}

CBCGPAppointmentProperty::CBCGPAppointmentProperty(const COleDateTime& value)
	: m_Value ((DATE) value, VT_DATE)
{
}

CBCGPAppointmentProperty::CBCGPAppointmentProperty(const COleDateTimeSpan& value)
	: m_Value ((double) value, VT_R8)
{
}

CBCGPAppointmentProperty::CBCGPAppointmentProperty(long value)
	: m_Value ((long) value, VT_I4)
{
}

CBCGPAppointmentProperty::CBCGPAppointmentProperty(BOOL value)
	: m_Value (value == 0 ? false : true)
{
}

CBCGPAppointmentProperty::CBCGPAppointmentProperty(const CString& value)
	: m_Value (value)
{
}

CBCGPAppointmentProperty::CBCGPAppointmentProperty(COLORREF value)
	: m_Value ((long) value, VT_I4)
{
}

CBCGPAppointmentProperty::~CBCGPAppointmentProperty()
{
}

COleDateTime CBCGPAppointmentProperty::GetDateTime () const
{
	ASSERT(IsValid ());
	ASSERT(GetType () == VT_DATE);

	return (DATE) m_Value;
}

void CBCGPAppointmentProperty::SetDateTime(const COleDateTime& value)
{
	ASSERT(IsValid ());
	ASSERT(GetType () == VT_DATE);

	m_Value = _variant_t((DATE) value, VT_DATE);
}

COleDateTimeSpan CBCGPAppointmentProperty::GetDateTimeSpan () const
{
	ASSERT(IsValid ());
	ASSERT(GetType () == VT_R8);

	return COleDateTimeSpan ((double) m_Value);
}

void CBCGPAppointmentProperty::SetDateTimeSpan(const COleDateTimeSpan& value)
{
	ASSERT(IsValid ());
	ASSERT(GetType () == VT_R8);

	m_Value = (double) value;
}

long CBCGPAppointmentProperty::GetLong () const
{
	ASSERT(IsValid ());
	ASSERT(GetType () == VT_I4);

	return (long) m_Value;
}

void CBCGPAppointmentProperty::SetLong(long value)
{
	ASSERT(IsValid ());
	ASSERT(GetType () == VT_I4);

	m_Value = value;
}

BOOL CBCGPAppointmentProperty::GetBOOL () const
{
	ASSERT(IsValid ());
	ASSERT(GetType () == VT_BOOL);

	return (BOOL)(bool) m_Value;
}

void CBCGPAppointmentProperty::SetBOOL(BOOL value)
{
	ASSERT(IsValid ());
	ASSERT(GetType () == VT_BOOL);

	m_Value = value == 0 ? false : true;
}

CString CBCGPAppointmentProperty::GetString () const
{
	ASSERT(IsValid ());
	ASSERT(GetType () == VT_BSTR);

	return (LPCTSTR) (_bstr_t) m_Value;
}

void CBCGPAppointmentProperty::SetString(const CString& value)
{
	ASSERT(IsValid ());
	ASSERT(GetType () == VT_BSTR);

	m_Value = (LPCTSTR) value;
}

COLORREF CBCGPAppointmentProperty::GetCOLORREF () const
{
	ASSERT(IsValid ());
	ASSERT(GetType () == VT_I4);

	return (COLORREF)(long) m_Value;
}

void CBCGPAppointmentProperty::SetCOLORREF(COLORREF value)
{
	ASSERT(IsValid ());
	ASSERT(GetType () == VT_I4);

	m_Value = (long)value;
}

const CBCGPAppointmentProperty& CBCGPAppointmentProperty::operator = (const COleDateTime& value)
{
	SetDateTime (value);
	return *this;
}

const CBCGPAppointmentProperty& CBCGPAppointmentProperty::operator = (const COleDateTimeSpan& value)
{
	SetDateTimeSpan (value);
	return *this;
}

const CBCGPAppointmentProperty& CBCGPAppointmentProperty::operator = (long value)
{
	SetLong (value);
	return *this;
}

const CBCGPAppointmentProperty& CBCGPAppointmentProperty::operator = (BOOL value)
{
	SetBOOL (value);
	return *this;
}

const CBCGPAppointmentProperty& CBCGPAppointmentProperty::operator = (const CString& value)
{
	SetString (value);
	return *this;
}

const CBCGPAppointmentProperty& CBCGPAppointmentProperty::operator = (COLORREF value)
{
	SetCOLORREF (value);
	return *this;
}

CBCGPAppointmentProperty::operator COleDateTime () const
{
	return GetDateTime ();
}

CBCGPAppointmentProperty::operator COleDateTimeSpan () const
{
	return GetDateTimeSpan ();
}

CBCGPAppointmentProperty::operator BOOL () const
{
	return GetBOOL ();
}

CBCGPAppointmentProperty::operator CString () const
{
	return GetString ();
}

CBCGPAppointmentProperty::operator COLORREF () const
{
	return GetCOLORREF ();
}

BOOL CBCGPAppointmentProperty::IsValid () const
{
	return m_Value.vt != VT_EMPTY;
}

VARTYPE CBCGPAppointmentProperty::GetType () const
{
	ASSERT (IsValid ());

	return m_Value.vt;
}

BOOL CBCGPAppointmentProperty::operator == (const CBCGPAppointmentBaseProperty& rProp) const
{
	if (CBCGPAppointmentBaseProperty::operator == (rProp))
	{
		return TRUE;
	}

	CBCGPAppointmentProperty* pProp = 
		DYNAMIC_DOWNCAST(CBCGPAppointmentProperty, &rProp);

	if (pProp == NULL)
	{
		return FALSE;
	}

	if (pProp->GetType () != GetType ())
	{
		return FALSE;
	}

	return pProp->m_Value == m_Value;
}

void CBCGPAppointmentProperty::Serialize (CArchive& ar)
{
	CBCGPAppointmentBaseProperty::Serialize (ar);

	if (ar.IsStoring ())
	{
		ASSERT (IsValid ());

		COleVariant var = m_Value;
		ar << var;
	}
	else
	{
		COleVariant var;
		ar >> var;
		
		m_Value = var;
	}
}

const CBCGPAppointmentBaseProperty&
CBCGPAppointmentProperty::operator = (const CBCGPAppointmentBaseProperty& rProp)
{
	const CBCGPAppointmentProperty* pProp = 
		DYNAMIC_DOWNCAST(CBCGPAppointmentProperty, &rProp);

	if (pProp != NULL)
	{
		CBCGPAppointmentBaseProperty::operator = (rProp);

		m_Value = pProp->m_Value;
	}
	else
	{
		ASSERT(FALSE);
	}

	return *this;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CBCGPAppointmentPropertyImages, CBCGPAppointmentBaseProperty, VERSIONABLE_SCHEMA | 1)

CBCGPAppointmentPropertyImages::CBCGPAppointmentPropertyImages()
{
}

CBCGPAppointmentPropertyImages::CBCGPAppointmentPropertyImages
	(const CBCGPAppointmentPropertyImages& value)
{
	CopyList (value.m_List, m_List);
}

CBCGPAppointmentPropertyImages::CBCGPAppointmentPropertyImages
	(const CBCGPAppointment::XImageList& value)
{
	CopyList (value, m_List);
}

CBCGPAppointmentPropertyImages::~CBCGPAppointmentPropertyImages()
{
}

BOOL CBCGPAppointmentPropertyImages::IsValid () const
{
	return TRUE;
}

void CBCGPAppointmentPropertyImages::Serialize (CArchive& ar)
{
	ASSERT (IsValid ());

	CBCGPAppointmentBaseProperty::Serialize (ar);

	if (ar.IsStoring ())
	{
		ar << (DWORD)(m_List.GetCount ());

		POSITION pos = m_List.GetHeadPosition ();
		while (pos != NULL)
		{
			ar << m_List.GetNext (pos);
		}
	}
	else
	{
		DWORD cnt = 0;
		ar >> cnt;

		for (DWORD i = 0; i < cnt; i++)
		{
			int value;
			ar >> value;

			m_List.AddTail (value);
		}
	}
}

void CBCGPAppointmentPropertyImages::SetImages (const CBCGPAppointment::XImageList& value)
{
	CopyList<CBCGPAppointment::XImageList>(value, m_List);
}

void CBCGPAppointmentPropertyImages::GetImages (CBCGPAppointment::XImageList& value) const
{
	CopyList<CBCGPAppointment::XImageList>(m_List, value);
}

const CBCGPAppointmentPropertyImages& CBCGPAppointmentPropertyImages::operator = 
	(const CBCGPAppointment::XImageList& value)
{
	SetImages (value);

	return *this;
}

BOOL CBCGPAppointmentPropertyImages::operator == (const CBCGPAppointmentBaseProperty& rProp) const
{
	if (CBCGPAppointmentBaseProperty::operator == (rProp))
	{
		return TRUE;
	}

	CBCGPAppointmentPropertyImages* pProp = 
		DYNAMIC_DOWNCAST(CBCGPAppointmentPropertyImages, &rProp);

	if (pProp == NULL)
	{
		return FALSE;
	}

	if (pProp->m_List.GetCount () != m_List.GetCount ())
	{
		return FALSE;
	}

	BOOL bEqual = TRUE;

	POSITION pos1 = m_List.GetHeadPosition ();
	POSITION pos2 = pProp->m_List.GetHeadPosition ();
	while (pos1 != NULL)
	{
		if (m_List.GetNext (pos1) != pProp->m_List.GetNext (pos2))
		{
			bEqual = FALSE;
			break;
		}
	}

	return bEqual;
}

const CBCGPAppointmentBaseProperty& CBCGPAppointmentPropertyImages::operator = 
	(const CBCGPAppointmentBaseProperty& rSrc)
{
	const CBCGPAppointmentPropertyImages* pProp = 
		DYNAMIC_DOWNCAST(CBCGPAppointmentPropertyImages, &rSrc);

	if (pProp != NULL)
	{
		CBCGPAppointmentBaseProperty::operator = (rSrc);

		CopyList (pProp->m_List, m_List);
	}
	else
	{
		ASSERT(FALSE);
	}

	return *this;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CBCGPAppointmentPropertyList, CObject, VERSIONABLE_SCHEMA | 1)

CBCGPAppointmentPropertyList::CBCGPAppointmentPropertyList()
{
}

CBCGPAppointmentPropertyList::CBCGPAppointmentPropertyList
	(const CBCGPAppointmentPropertyList& rProps)
{
	CopyFrom (rProps);
}

CBCGPAppointmentPropertyList::~CBCGPAppointmentPropertyList()
{
	RemoveAll ();
}

BOOL CBCGPAppointmentPropertyList::Add (DWORD ID, CBCGPAppointmentBaseProperty* pProperty)
{
	if (!PropertyExists (ID))
	{
		m_Properties.SetAt (ID, pProperty);
		return TRUE;
	}

	return FALSE;
}

BOOL CBCGPAppointmentPropertyList::Set (DWORD ID, CBCGPAppointmentBaseProperty* pProperty)
{
	if (PropertyExists (ID))
	{
		m_Properties.SetAt (ID, pProperty);
		return TRUE;
	}

	return FALSE;
}

BOOL CBCGPAppointmentPropertyList::Remove (DWORD ID)
{
	CBCGPAppointmentBaseProperty* prop = NULL;
	m_Properties.Lookup (ID, prop);

	if (prop != NULL)
	{
		delete prop;
		return m_Properties.RemoveKey (ID);
	}

	return FALSE;
}

void CBCGPAppointmentPropertyList::RemoveAll ()
{
	POSITION Pos = m_Properties.GetStartPosition ();
	DWORD Key;
	CBCGPAppointmentBaseProperty* Val = NULL;

	while (Pos != NULL)
	{
		m_Properties.GetNextAssoc (Pos, Key, Val);

		if (Val != NULL)
		{
			delete Val;
		}
	}

	m_Properties.RemoveAll ();
}

const CBCGPAppointmentBaseProperty* CBCGPAppointmentPropertyList::Get (DWORD ID) const
{
	CBCGPAppointmentBaseProperty* prop = NULL;

	m_Properties.Lookup (ID, prop);

	return prop;
}

int CBCGPAppointmentPropertyList::GetCount () const
{
	return (int) m_Properties.GetCount ();
}

POSITION CBCGPAppointmentPropertyList::GetStart () const
{
	return m_Properties.GetStartPosition ();
}

void CBCGPAppointmentPropertyList::GetNext (POSITION& rNextPosition, 
											DWORD& ID, 
											CBCGPAppointmentBaseProperty*& rProp) const
{
	m_Properties.GetNextAssoc (rNextPosition, ID, rProp);
}

BOOL CBCGPAppointmentPropertyList::PropertyExists (DWORD ID) const
{
	CBCGPAppointmentBaseProperty* prop = NULL;
	return m_Properties.Lookup (ID, prop);
}

void CBCGPAppointmentPropertyList::Serialize (CArchive& ar)
{
	CObject::Serialize (ar);
	
	if (ar.IsStoring ())
	{
		ar << (DWORD)m_Properties.GetCount ();

		POSITION Pos = m_Properties.GetStartPosition ();
		DWORD Key;
		CBCGPAppointmentBaseProperty* Val = NULL;

		while (Pos != NULL)
		{
			m_Properties.GetNextAssoc (Pos, Key, Val);
			ASSERT_VALID (Val);

			ar << Key;
			ar << Val;
		}
	}
	else
	{
		RemoveAll ();

		DWORD dwCount = 0;
		ar >> dwCount;

		for (DWORD i = 0; i < dwCount; i++)
		{
			DWORD Key;
			ar >> Key;

			CBCGPAppointmentBaseProperty* Val = NULL;
			ar >> Val;

			m_Properties[Key] = Val;
		}
	}
}

void CBCGPAppointmentPropertyList::CopyFrom (const CBCGPAppointmentPropertyList& rSrc)
{
	RemoveAll ();

	POSITION Pos = rSrc.m_Properties.GetStartPosition ();
	DWORD Key;
	CBCGPAppointmentBaseProperty* Val = NULL;

	while (Pos != NULL)
	{
		rSrc.m_Properties.GetNextAssoc (Pos, Key, Val);

		if (Val != NULL)
		{
			CBCGPAppointmentBaseProperty* newProp = 
				(CBCGPAppointmentBaseProperty*)(Val->GetRuntimeClass ()->CreateObject ());

			*newProp = *Val;

			m_Properties[Key] = newProp;
		}
	}	
}

CBCGPAppointmentBaseProperty*& CBCGPAppointmentPropertyList::operator [] (DWORD ID)
{
	return m_Properties[ID];
}

BOOL CBCGPAppointmentPropertyList::operator == (const CBCGPAppointmentPropertyList& rProps) const
{
	if (GetCount () != rProps.GetCount ())
	{
		return FALSE;
	}

	BOOL bEqual = TRUE;

	POSITION Pos = m_Properties.GetStartPosition ();
	DWORD Key;
	CBCGPAppointmentBaseProperty* Val = NULL;

	while (Pos != NULL)
	{
		m_Properties.GetNextAssoc (Pos, Key, Val);

		if (Val != NULL)
		{
			const CBCGPAppointmentBaseProperty* pProp = rProps.Get(Key);

			if (pProp == NULL)
			{
				bEqual = FALSE;
				break;
			}

			if (*pProp != *Val)
			{
				bEqual = FALSE;
				break;
			}
		}
	}

	return bEqual;
}

BOOL CBCGPAppointmentPropertyList::operator != (const CBCGPAppointmentPropertyList& rProps) const
{
	return !(*this == rProps);
}

#endif // BCGP_EXCLUDE_PLANNER
