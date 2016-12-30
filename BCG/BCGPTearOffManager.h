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

// BCGTearOffManager.h: interface for the CBCGPTearOffManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPTEAROFFMANAGER_H__511162B7_5BEC_4B75_8D09_2C36CBFB2C5E__INCLUDED_)
#define AFX_BCGPTEAROFFMANAGER_H__511162B7_5BEC_4B75_8D09_2C36CBFB2C5E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

class BCGCBPRODLLEXPORT CBCGPTearOffManager : public CObject  
{

public:
	CBCGPTearOffManager();
	virtual ~CBCGPTearOffManager();

// Operations:
	BOOL Initialize (LPCTSTR lpszRegEntry, 
					UINT uiTearOffMenuFirst, 
					UINT uiTearOffMenuLast);

	void Reset (HMENU hmenu);

	void SetupTearOffMenus (HMENU hMenu);
	void SetInUse (UINT uiCmdId, BOOL bUse = TRUE);

	void Build (UINT uiTearOffBarID, CString& strText);
	UINT Parse (CString& str);

protected:
	UINT GetFreeTearOffID ();

// Attributes:
public:
	LPCTSTR GetRegPath () const
	{
		return m_strTearOfBarRegEntry;
	}

	BOOL IsDynamicID (UINT uiID) const
	{
		return	uiID >= m_uiTearOffMenuFirst &&
				uiID <= m_uiTearOffMenuLast;
	}

protected:
	UINT			m_uiTearOffMenuFirst;
	UINT			m_uiTearOffMenuLast;
	CString			m_strTearOfBarRegEntry;

	CArray<int,int>	m_arTearOffIDsUsage;
};

extern CBCGPTearOffManager*	g_pBCGPTearOffMenuManager;

#endif // !defined(AFX_BCGPTEAROFFMANAGER_H__511162B7_5BEC_4B75_8D09_2C36CBFB2C5E__INCLUDED_)
