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

// MenuHash.h: interface for the CBCGPMenuHash class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUHASH_H__6DC611B4_D93A_11D1_A64E_00A0C93A70EC__INCLUDED_)
#define AFX_MENUHASH_H__6DC611B4_D93A_11D1_A64E_00A0C93A70EC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

class CBCGPToolBar;

class CBCGPMenuHash
{
public:
	CBCGPMenuHash();
	virtual ~CBCGPMenuHash();

	BOOL SaveMenuBar (HMENU hMenu, CBCGPToolBar* pBar);
	BOOL LoadMenuBar (HMENU hMenu, CBCGPToolBar* pBar);

	BOOL RemoveMenu (HMENU hMenu);
	void CleanUp ();

	BOOL IsActive () const
	{
		return m_bIsActive;
	}

protected:
	CMap<HMENU, HMENU&, HANDLE, HANDLE&>	m_StoredMenues;
	BOOL									m_bIsActive;
};

extern CBCGPMenuHash	g_menuHash;

#endif // !defined(AFX_MENUHASH_H__6DC611B4_D93A_11D1_A64E_00A0C93A70EC__INCLUDED_)
