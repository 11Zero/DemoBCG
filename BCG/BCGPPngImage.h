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
// BCGPPngImage.h: interface for the CBCGPPngImage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPPNGIMAGE_H__62C7100C_EBD7_4B01_AC51_C6D9EC472CAE__INCLUDED_)
#define AFX_BCGPPNGIMAGE_H__62C7100C_EBD7_4B01_AC51_C6D9EC472CAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcbpro.h"

#if _MSC_VER >= 1300 && !defined BCGP_EXCLUDE_GDI_PLUS && !defined BCGP_EXCLUDE_PNG_SUPPORT
	#include "atlimage.h"
#endif

class BCGCBPRODLLEXPORT CBCGPPngImage : public CBitmap  
{
// Construction/Destruction
public:
	CBCGPPngImage();
	virtual ~CBCGPPngImage();

// Attributes:
public:
	static CString	m_strPngResType;	// "PNG" by default

protected:

#if _MSC_VER >= 1300 && !defined BCGP_EXCLUDE_GDI_PLUS && !defined BCGP_EXCLUDE_PNG_SUPPORT
	static CImage m_image;
	static BOOL m_bCImageInitialized;
	static BOOL InitCImage();
#endif

// Operations:
public:
	BOOL Load (UINT uiResID, HINSTANCE hinstRes = NULL);
	BOOL Load (LPCTSTR lpszResourceName, HINSTANCE hinstRes = NULL);

	BOOL LoadFromFile (LPCTSTR lpszPath);
	BOOL LoadFromBuffer (LPBYTE lpBuffer, UINT uiSize);

	BOOL SaveToFile (LPCTSTR lpszPath);
	UINT SaveToBuffer (LPBYTE* lpBuffer);
protected:
	BOOL LoadFromFile (CFile* pFile);
	BOOL SaveToFile (CFile* pFile);
};

#endif // !defined(AFX_BCGPPNGIMAGE_H__62C7100C_EBD7_4B01_AC51_C6D9EC472CAE__INCLUDED_)
