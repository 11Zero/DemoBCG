#if !defined(AFX_BCGPPLANNERDROPSOURCE_H__62FAAE6C_9C50_432A_9AC3_57A29F774577__INCLUDED_)
#define AFX_BCGPPLANNERDROPSOURCE_H__62FAAE6C_9C50_432A_9AC3_57A29F774577__INCLUDED_

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
// BCGPPlannerDropSource.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#ifndef __AFXOLE_H__
#include <afxole.h>
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerDropSource

class CBCGPPlannerDropSource : public COleDropSource
{
public:
	CBCGPPlannerDropSource();

	virtual ~CBCGPPlannerDropSource();

	virtual void Empty ();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPPlannerDropSource)
	virtual SCODE GiveFeedback(DROPEFFECT dropEffect);
	virtual SCODE QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState);
	virtual BOOL OnBeginDrag(CWnd* pWnd);
	//}}AFX_VIRTUAL

protected:
	BOOL	m_bEscapePressed;
	BOOL	m_bDragStarted;
	HCURSOR	m_hcurMove;
	HCURSOR	m_hcurCopy;
};

#endif // BCGP_EXCLUDE_PLANNER

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPLANNERDROPSOURCE_H__62FAAE6C_9C50_432A_9AC3_57A29F774577__INCLUDED_)
