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

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__442F0493_397F_4E45_A664_561A0D3A971C__INCLUDED_)
#define AFX_STDAFX_H__442F0493_397F_4E45_A664_561A0D3A971C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define _AFX_ALL_WARNINGS

#include "BCGCBProConfig.h"

#if _MSC_VER < 1500
	#if _MSC_VER >= 1400
		#ifdef BCGP_CLEAN_DLL_PATH
			#define WINVER			0x0502
			#define _WIN32_WINNT	0x0502
		#else
			#define WINVER			0x0400
			#define _WIN32_WINNT	0x0400
		#endif
	#else
		#define WINVER			0x0400
	#endif
#endif

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#pragma warning (disable : 4786)

#include <afxtempl.h>
#include <afxpriv.h>

#if _MSC_VER >= 1300
	#include <..\atlmfc\src\mfc\afximpl.h>
#else
	#include <..\src\afximpl.h>
#endif

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#include <AfxAdv.h>
#endif // _AFX_NO_OLE_SUPPORT


#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			// MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#pragma warning (disable : 4201)
	#include "mmsystem.h"
	#pragma comment(lib,"winmm.lib") 
	#pragma message("Automatically linking with winmm.lib")
#pragma warning (default : 4201)

#if _MSC_VER >= 1400
	#pragma warning (disable : 6011)
	#pragma warning (disable : 6031)
	#pragma warning (disable : 6246)
	#pragma warning (disable : 6284)
	#pragma warning (disable : 6387)
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__442F0493_397F_4E45_A664_561A0D3A971C__INCLUDED_)
