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
// BCGPRecurrence.h: interface for the CBCGPRecurrence class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRECURRENCE_H__D3857941_1EF2_4EA7_908C_04BA533575D7__INCLUDED_)
#define AFX_BCGPRECURRENCE_H__D3857941_1EF2_4EA7_908C_04BA533575D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXTEMPL_H__
	#pragma warning (disable : 4786)
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

class CBCGPAppointment;
class CBCGPRecurrenceBaseRule;

#include "BCGPAppointmentProperty.h"

class BCGCBPRODLLEXPORT CBCGPRecurrence : public CObject  
{
	DECLARE_SERIAL(CBCGPRecurrence)

	friend class CBCGPAppointment;
public:
	struct XBCGPRecurrenceException
	{
		BOOL						 m_Deleted;
		COleDateTime				 m_dtStart;
		COleDateTime				 m_dtFinish;
		CBCGPAppointmentPropertyList m_Properties;

		XBCGPRecurrenceException ()
		{
			m_Deleted = FALSE;
		}
	};

	CBCGPRecurrence();
	virtual ~CBCGPRecurrence();

	BOOL IsExceptionsCollectionEmpty () const;
	BOOL ExceptionExists (const COleDateTime& dtDate) const;
	void DoException (const COleDateTime& dtDate, const CBCGPAppointmentPropertyList& props, BOOL bDeleted);
	XBCGPRecurrenceException* GetException (const COleDateTime& dtDate);

	void RemoveExceptions ();

	virtual void Query (XBCGPAppointmentArray& ar, const COleDateTime& date1, const COleDateTime& date2) const;

	virtual void Serialize (CArchive& ar);

	void SetRule (const CBCGPRecurrenceBaseRule* pRule);
	CBCGPRecurrenceBaseRule* GetRule () const
	{
		return m_pRule;
	}

protected:
	void SetAppointment (CBCGPAppointment* pApp);

	virtual CBCGPAppointment* CreateClone (const COleDateTime& dtDate) const;

private:
	typedef CMap<COleDateTime, const COleDateTime&, XBCGPRecurrenceException*, XBCGPRecurrenceException*>
		XBCGPRecurrenceExceptions;

	CBCGPAppointment*			m_pAppointment;

	CBCGPRecurrenceBaseRule*	m_pRule;
	XBCGPRecurrenceExceptions	m_Exceptions;
};

#endif // BCGP_EXCLUDE_PLANNER

#endif // !defined(AFX_BCGPRECURRENCE_H__D3857941_1EF2_4EA7_908C_04BA533575D7__INCLUDED_)
