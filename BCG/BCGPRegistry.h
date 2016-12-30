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
// BCGPRegistry.h: interface for the CBCGPRegistry class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPREGISTRY_H__00F1EBC7_61BF_11D5_A304_F156F535EA11__INCLUDED_)
#define AFX_BCGPREGISTRY_H__00F1EBC7_61BF_11D5_A304_F156F535EA11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <winreg.h>

#define REG_RECT	0x0001
#define REG_POINT	0x0002

#include "BCGCBPro.h"


class BCGCBPRODLLEXPORT CBCGPRegistry : public CObject  
{
	friend class CBCGPRegistrySP;
	DECLARE_DYNCREATE(CBCGPRegistry)

// Construction
public:
	CBCGPRegistry (BOOL bAdmin, BOOL bReadOnly);
	virtual ~CBCGPRegistry();

	struct REGINFO
	{
		LONG lMessage;
		DWORD dwType;
		DWORD dwSize;
	}
	m_Info;

protected:
	CBCGPRegistry ();

// Operations
public:
	virtual BOOL ReadKeyValues(CStringArray& Values);
	virtual BOOL VerifyKey (LPCTSTR pszPath);
	virtual BOOL VerifyValue (LPCTSTR pszValue);
	virtual BOOL CreateKey (LPCTSTR pszPath);
	virtual BOOL Open (LPCTSTR pszPath);
	virtual void Close();

	virtual BOOL DeleteValue (LPCTSTR pszValue);
	virtual BOOL DeleteKey (LPCTSTR pszPath, BOOL bAdmin = FALSE);
	virtual BOOL Write (LPCTSTR pszKey, int iVal);
	virtual BOOL Write (LPCTSTR pszKey, DWORD dwVal);
    virtual BOOL Write (LPCTSTR pszKey, double dVal);
	virtual BOOL Write (LPCTSTR pszKey, LPCTSTR pszVal);
	virtual BOOL Write (LPCTSTR pszKey, CStringList& scStringList);
	virtual BOOL Write (LPCTSTR pszKey, CByteArray& bcArray);
	virtual BOOL Write (LPCTSTR pszKey, CStringArray& scArray);
	virtual BOOL Write (LPCTSTR pszKey, CDWordArray& dwcArray);
	virtual BOOL Write (LPCTSTR pszKey, CWordArray& wcArray);
	virtual BOOL Write (LPCTSTR pszKey, const CRect& rect);
	virtual BOOL Write (LPCTSTR pszKey, LPPOINT& lpPoint);
	virtual BOOL Write (LPCTSTR pszKey, LPBYTE pData, UINT nBytes);
	virtual BOOL Write (LPCTSTR pszKey, CObList& list);
	virtual BOOL Write (LPCTSTR pszKey, CObject& obj);
	virtual BOOL Write (LPCTSTR pszKey, CObject* pObj);

	virtual BOOL Read (LPCTSTR pszKey, int& iVal);
	virtual BOOL Read (LPCTSTR pszKey, DWORD& dwVal);
    virtual BOOL Read (LPCTSTR pszKey, double& dVal);
    virtual BOOL Read (LPCTSTR pszKey, CString& sVal);
	virtual BOOL Read (LPCTSTR pszKey, CStringList& scStringList);
	virtual BOOL Read (LPCTSTR pszKey, CStringArray& scArray);
	virtual BOOL Read (LPCTSTR pszKey, CDWordArray& dwcArray);
	virtual BOOL Read (LPCTSTR pszKey, CWordArray& wcArray);
	virtual BOOL Read (LPCTSTR pszKey, CByteArray& bcArray);
	virtual BOOL Read (LPCTSTR pszKey, LPPOINT& lpPoint);
	virtual BOOL Read (LPCTSTR pszKey, CRect& rect);
	virtual BOOL Read (LPCTSTR pszKey, BYTE** ppData, UINT* pBytes);
	virtual BOOL Read (LPCTSTR pszKey, CObList& list);
	virtual BOOL Read (LPCTSTR pszKey, CObject& obj);
	virtual BOOL Read (LPCTSTR pszKey, CObject*& pObj);

	virtual BOOL ReadSubKeys(CStringArray& SubKeys);

protected:

	HKEY		m_hKey;
	BOOL		m_bAdmin;
	CString		m_sPath;
	BOOL		m_bReadOnly;
	DWORD		m_dwUserData;
};
//////////////////////////////////////////////////////////////////////////////
// CBCGRegistrySP - Helper class that manages "safe" CBCGRegistry pointer

class BCGCBPRODLLEXPORT CBCGPRegistrySP
{
public:
	static BOOL SetRuntimeClass (CRuntimeClass* pRTI);

	CBCGPRegistrySP(DWORD dwUserData = 0) :
		m_pRegistry (NULL),
		m_dwUserData (dwUserData)
	{
	}

	~CBCGPRegistrySP()
	{
		if (m_pRegistry != NULL)
		{
			ASSERT_VALID (m_pRegistry);
			delete m_pRegistry;
		}
	}

	CBCGPRegistry& Create (BOOL bAdmin, BOOL bReadOnly);

protected:
	CBCGPRegistry*			m_pRegistry;
	static CRuntimeClass*	m_pRTIDefault;

	DWORD					m_dwUserData;
};

#endif // !defined(AFX_BCGPREGISTRY_H__00F1EBC7_61BF_11D5_A304_F156F535EA11__INCLUDED_)
