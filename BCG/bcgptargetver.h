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

#ifndef _BCGP_TARGET_VER
#define _BCGP_TARGET_VER

#pragma once

// You can include this file to your stdafx.h PRIOR to "#include <afxwin.h>" statement.

#ifndef WINVER
#if _MSC_VER < 1300
#elif _MSC_VER < 1400
	#define WINVER 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#elif _MSC_VER < 1500
	#define WINVER 0x0501		// Change this to the appropriate value to target other 
#elif _MSC_VER == 1500
	#define WINVER 0x0600       // Change this to the appropriate value to target other versions of Windows.
#else
	#include <SDKDDKVer.h>
#endif
#endif

#endif // _BCGP_TARGET_VER
