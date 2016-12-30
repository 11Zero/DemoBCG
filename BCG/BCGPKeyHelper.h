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

// KeyHelper.h: interface for the CBCGPKeyHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPKEYHELPER_H__283E6045_54C6_11D2_B110_D085EB8D1B3C__INCLUDED_)
#define AFX_BCGPKEYHELPER_H__283E6045_54C6_11D2_B110_D085EB8D1B3C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "BCGCBPro.h"

class BCGCBPRODLLEXPORT CBCGPKeyHelper : public CObject
{
public:
	CBCGPKeyHelper();
	CBCGPKeyHelper(LPACCEL lpAccel);
	virtual ~CBCGPKeyHelper();

// Operations:
public:
	void Format (CString& str) const;

protected:
	void AddVirtKeyStr (CString& str, UINT uiVirtKey, BOOL bLast = FALSE) const;

// Atttributes:
public:
	void SetAccelerator (LPACCEL lpAccel)
	{
		m_lpAccel = lpAccel;
	}

protected:
	LPACCEL m_lpAccel;
};

#endif // !defined(AFX_BCGPKEYHELPER_H__283E6045_54C6_11D2_B110_D085EB8D1B3C__INCLUDED_)
