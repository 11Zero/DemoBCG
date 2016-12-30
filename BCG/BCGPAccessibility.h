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
#include "oleacc.h"

#ifndef __BCGPACCESSIBILITY_H
#define __BCGPACCESSIBILITY_H

// CBCGPAccessibilityData

struct CBCGPAccessibilityData
{
	CBCGPAccessibilityData()
	{
		Clear ();
	}

	void Clear ()
	{
		m_nAccRole = 0;
		m_bAccState = 0;
		m_nAccHit = 0;
		m_rectAccLocation = CRect (0, 0, 0, 0);
		m_ptAccHit = CPoint (0, 0);

		m_strAccName.Empty ();
		m_strAccValue.Empty ();
		m_strDescription.Empty ();
		m_strAccKeys.Empty ();
		m_strAccHelp.Empty ();
		m_strAccName.Empty ();
		m_strAccDefAction.Empty ();
	}

	CString				m_strAccName;
	CString				m_strAccValue;
	UINT				m_nAccHit;
	CString				m_strDescription;
	int					m_nAccRole;
	UINT				m_bAccState;
	CString				m_strAccKeys;
	CString				m_strAccHelp;
	CString				m_strAccDefAction;
	CRect				m_rectAccLocation;
	CPoint				m_ptAccHit;
};


#ifndef WM_GETOBJECT
#define WM_GETOBJECT	    0x003D
#endif  

BCGCBPRODLLEXPORT extern UINT BCGM_ACCGETOBGECT;
BCGCBPRODLLEXPORT extern UINT BCGM_ACCHITTEST;
BCGCBPRODLLEXPORT extern UINT BCGM_ACCGETSTATE;

#ifndef OBJID_CLIENT
#define OBJID_CLIENT					0xFFFFFFFC
#endif

#ifndef	OBJID_WINDOW
#define OBJID_WINDOW					0x00000000
#endif

#ifndef	OBJID_NATIVEOM
#define OBJID_NATIVEOM					0xFFFFFFF0
#endif

#ifndef CHILDID_SELF
#define CHILDID_SELF					0
#endif

#ifndef EVENT_SYSTEM_MENUEND
#define EVENT_SYSTEM_MENUEND            0x0005
#endif

#ifndef EVENT_SYSTEM_MENUPOPUPSTART
#define EVENT_SYSTEM_MENUPOPUPSTART     0x0006
#endif

#ifndef EVENT_SYSTEM_MENUPOPUPEND
#define EVENT_SYSTEM_MENUPOPUPEND       0x0007
#endif

#ifndef EVENT_OBJECT_FOCUS
#define EVENT_OBJECT_FOCUS              0x8005
#endif 

#ifndef EVENT_OBJECT_STATECHANGE 
#define EVENT_OBJECT_STATECHANGE 		0x800a
#endif 

#ifndef EVENT_SYSTEM_MENUSTART
#define EVENT_SYSTEM_MENUSTART			0x0004
#endif

#ifndef STATE_SYSTEM_HASPOPUP
#define	STATE_SYSTEM_HASPOPUP	        0x40000000
#endif

#ifndef ROLE_SYSTEM_SPLITBUTTON
#define	ROLE_SYSTEM_SPLITBUTTON	        0x3e
#endif

#ifndef STATE_SYSTEM_NORMAL
#define STATE_SYSTEM_NORMAL				0x0000
#endif 

#ifndef EVENT_OBJECT_SELECTION
#define EVENT_OBJECT_SELECTION          0x8006
#endif

#ifndef EVENT_OBJECT_VALUECHANGE
#define EVENT_OBJECT_VALUECHANGE        0x800E
#endif


#endif	// __BCGPACCESSIBILITY_H