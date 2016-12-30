#if !defined(AFX_BCGPAPPOINTMENTSTORAGE_H__70804508_FCC2_4727_A8A6_C1B9B0F6DF54__INCLUDED_)
#define AFX_BCGPAPPOINTMENTSTORAGE_H__70804508_FCC2_4727_A8A6_C1B9B0F6DF54__INCLUDED_

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
// BCGPAppointmentStorage.h : header file
//

#ifndef __AFXTEMPL_H__
	#pragma warning (disable : 4786)
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPAppointment.h"
#include "BCGPRecurrence.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPAppointmentBaseStorage

class BCGCBPRODLLEXPORT CBCGPAppointmentBaseStorage : public CObject
{
	DECLARE_DYNAMIC(CBCGPAppointmentBaseStorage)

public:
	virtual ~CBCGPAppointmentBaseStorage ()
	{
	}

	virtual BOOL Add (CBCGPAppointment* pApp) = 0;
	virtual BOOL Update (CBCGPAppointment* pApp, const COleDateTime& dtOld, BOOL bForceAdd = FALSE) = 0;
	virtual BOOL Remove (CBCGPAppointment* pApp) = 0;
	virtual BOOL RemoveAll () = 0;
	virtual void Query (XBCGPAppointmentArray& ar, const COleDateTime& date1, const COleDateTime& date2) = 0;
	virtual void QueryAll (XBCGPAppointmentArray& ar) = 0;
	virtual BOOL IsEmpty () const = 0;

	virtual CBCGPAppointment* GetRecurrence (DWORD ID) const = 0;
	
protected:
	CBCGPAppointmentBaseStorage ()
	{
	}
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPAppointmentBaseResourceInfo

class BCGCBPRODLLEXPORT CBCGPAppointmentBaseResourceInfo : public CObject
{
	DECLARE_DYNAMIC(CBCGPAppointmentBaseResourceInfo)

public:
	virtual ~CBCGPAppointmentBaseResourceInfo ()
	{
	}

	virtual CString GetDescription () const = 0;
	virtual COleDateTime GetWorkStart () const = 0;
	virtual COleDateTime GetWorkEnd () const = 0;
	
protected:
	CBCGPAppointmentBaseResourceInfo ()
	{
	}
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPAppointmentBaseMultiStorage

class BCGCBPRODLLEXPORT CBCGPAppointmentBaseMultiStorage : public CBCGPAppointmentBaseStorage
{
	DECLARE_DYNAMIC(CBCGPAppointmentBaseMultiStorage)

public:
	enum
	{
		e_UnknownResourceID = 0
	};

	typedef CArray<UINT, UINT> XResourceIDArray;

public:
	virtual ~CBCGPAppointmentBaseMultiStorage ()
	{
	}

	virtual BOOL AddStorage (UINT nResourceID, CBCGPAppointmentBaseStorage* pStorage, 
		CBCGPAppointmentBaseResourceInfo* pInfo = NULL, BOOL bAutoDelete = TRUE) = 0;
	virtual BOOL RemoveStorage (UINT nResourceID) = 0;

	virtual CBCGPAppointmentBaseStorage* GetStorage (UINT nResourceID) const = 0;

	virtual int GetCount () const = 0;
	virtual void GetResourceIDs (XResourceIDArray& ar) const = 0;

	virtual const CBCGPAppointmentBaseResourceInfo* GetResourceInfo (UINT nResourceID) const = 0;

	virtual BOOL SetCurrentResourceID (UINT nResourceID) = 0;
	virtual UINT GetCurrentResourceID () const = 0;
	
protected:
	CBCGPAppointmentBaseMultiStorage ()
	{
	}
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPAppointmentStorage

class CBCGPAppointmentStorage: public CBCGPAppointmentBaseStorage
{
	DECLARE_DYNCREATE(CBCGPAppointmentStorage)

public:
	virtual ~CBCGPAppointmentStorage ();

	BOOL IsAutoDelete () const
	{
		return m_bAutoDelete;
	}

	void SetAutoDelete (BOOL bDelete)
	{
		m_bAutoDelete = bDelete;
	}

// Overrides
public:
	virtual BOOL Add (CBCGPAppointment* pApp);
	virtual BOOL Update (CBCGPAppointment* pApp, const COleDateTime& dtOld, BOOL bForceAdd = FALSE);
	virtual BOOL Remove (CBCGPAppointment* pApp);
	virtual BOOL RemoveAll ();

	virtual void Query (XBCGPAppointmentArray& ar, const COleDateTime& date1, const COleDateTime& date2);
	virtual void QueryAll (XBCGPAppointmentArray& ar);

	virtual BOOL IsEmpty () const;

	virtual CBCGPAppointment* GetRecurrence (DWORD ID) const;

protected:
	// Construction
	CBCGPAppointmentStorage ();

	virtual BOOL AddRecurrence (CBCGPAppointment* pApp);
	virtual BOOL UpdateRecurrence (CBCGPAppointment* pApp, const COleDateTime& dtOld);
	virtual BOOL RemoveRecurrence (CBCGPAppointment* pApp);
	virtual BOOL RemoveAllRecurrence ();

protected:
	typedef CMap<int, int, XBCGPAppointmentList*, XBCGPAppointmentList*> XDayCollection;
	typedef CMap<int, int, XDayCollection*, XDayCollection*> XYearCollection;

	typedef CMap<DWORD, DWORD, CBCGPAppointment*, CBCGPAppointment*> XRecurrenceCollection;

	XYearCollection& GetCollection ()
	{
		return m_Collection;
	}

private:
	BOOL			m_bAutoDelete;
	XYearCollection	m_Collection;
	XBCGPAppointmentArray
					m_CollectionMulti;

	DWORD			m_dwRecurrenceLastID;
	XRecurrenceCollection
					m_Recurrences;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPAppointmentMultiStorage

class CBCGPAppointmentMultiStorage: public CBCGPAppointmentBaseMultiStorage
{
	DECLARE_DYNCREATE(CBCGPAppointmentMultiStorage)

public:
	virtual ~CBCGPAppointmentMultiStorage ();

// Overrides
public:
	virtual BOOL Add (CBCGPAppointment* pApp);
	virtual BOOL Update (CBCGPAppointment* pApp, const COleDateTime& dtOld, BOOL bForceAdd = FALSE);
	virtual BOOL Remove (CBCGPAppointment* pApp);
	virtual BOOL RemoveAll ();
	virtual void Query (XBCGPAppointmentArray& ar, const COleDateTime& date1, const COleDateTime& date2);
	virtual void QueryAll (XBCGPAppointmentArray& ar);
	virtual BOOL IsEmpty () const;

	virtual CBCGPAppointment* GetRecurrence (DWORD ID) const;

	virtual BOOL AddStorage (UINT nResourceID, CBCGPAppointmentBaseStorage* pStorage, 
		CBCGPAppointmentBaseResourceInfo* pInfo = NULL, BOOL bAutoDelete = TRUE);
	virtual BOOL RemoveStorage (UINT nResourceID);

	virtual CBCGPAppointmentBaseStorage* GetStorage (UINT nResourceID) const;

	virtual int GetCount () const;

	virtual void GetResourceIDs (XResourceIDArray& ar) const;

	virtual const CBCGPAppointmentBaseResourceInfo* GetResourceInfo (UINT nResourceID) const;

	virtual BOOL SetCurrentResourceID (UINT nResourceID);
	virtual UINT GetCurrentResourceID () const;

protected:
	class XResource
	{
	public:
		XResource ()
			: m_pStorage     (NULL)
			, m_pResourceInfo(NULL)
			, m_bAutoDelete  (FALSE)
		{
		}

		XResource (CBCGPAppointmentBaseStorage* pStorage, 
			CBCGPAppointmentBaseResourceInfo* pInfo = NULL, BOOL bAutoDelete = TRUE)
			: m_pStorage     (pStorage)
			, m_pResourceInfo(pInfo)
			, m_bAutoDelete  (bAutoDelete)
		{
		}

		~XResource ()
		{
			if (m_bAutoDelete && m_pStorage != NULL)
			{
				delete m_pStorage;
				m_pStorage = NULL;

				if (m_pResourceInfo != NULL)
				{
					delete m_pResourceInfo;
					m_pResourceInfo = NULL;
				}
			}
		}

		CBCGPAppointmentBaseStorage* m_pStorage;
		CBCGPAppointmentBaseResourceInfo* m_pResourceInfo;
		BOOL m_bAutoDelete;
	};

	typedef CMap<UINT, UINT, XResource*, XResource*> XResourceCollection;

	XResource* LookupResource (UINT nResourceID) const;

protected:
	// Construction
	CBCGPAppointmentMultiStorage ();

private:
	XResourceCollection m_ResourceCollection;
	UINT	m_CurrentResourceID;
};

#endif // BCGP_EXCLUDE_PLANNER

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPAPPOINTMENTSTORAGE_H__70804508_FCC2_4727_A8A6_C1B9B0F6DF54__INCLUDED_)
