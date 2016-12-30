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

#include "stdafx.h"
#include "RegPath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CString BCGPGetRegPath (LPCTSTR lpszPostFix, LPCTSTR lpszProfileName)
{
	ASSERT (lpszPostFix != NULL);

	CString strReg;

	if (lpszProfileName != NULL &&
		lpszProfileName [0] != 0)
	{
		strReg = lpszProfileName;
	}
	else
	{
		CWinApp* pApp = AfxGetApp ();
		ASSERT_VALID (pApp);

		if (pApp->m_pszRegistryKey == NULL ||
			pApp->m_pszProfileName == NULL)
		{
			TRACE (_T("BCGPGetRegPath error: please call SetRegistryKey in your application InitInstance\n"));
			return strReg;
		}

		strReg = _T("SOFTWARE\\");

		CString strRegKey = pApp->m_pszRegistryKey;
		if (!strRegKey.IsEmpty ())
		{
			strReg += strRegKey;
			strReg += _T("\\");
		}

		strReg += pApp->m_pszProfileName;
		strReg += _T("\\");
		strReg += lpszPostFix ;
		strReg += _T("\\");
	}
    
    return strReg;
}
