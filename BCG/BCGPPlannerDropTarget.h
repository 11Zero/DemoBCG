#if !defined(AFX_BCGPPLANNERDROPTARGET_H__9B440796_A576_4EA3_A635_BF9A998E387E__INCLUDED_)
#define AFX_BCGPPLANNERDROPTARGET_H__9B440796_A576_4EA3_A635_BF9A998E387E__INCLUDED_

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
// BCGPPlannerDropTarget.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#ifndef __AFXOLE_H__
#include <afxole.h>
#endif

class CBCGPPlannerManagerCtrl;
class CBCGPCalendar;

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerDropTarget

class CBCGPPlannerDropTarget : public COleDropTarget
{
public:
	CBCGPPlannerDropTarget();
	virtual ~CBCGPPlannerDropTarget();

// Operations
public:

	BOOL Register (CBCGPPlannerManagerCtrl* pPlanner);
	BOOL Register (CBCGPCalendar* pCalendar);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPPlannerDropTarget)
	public:
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, DROPEFFECT dropList, CPoint point);
	//}}AFX_VIRTUAL

protected:

	CBCGPPlannerManagerCtrl*	m_pPlanner;
	CBCGPCalendar*				m_pCalendar;
};

#endif // BCGP_EXCLUDE_PLANNER

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPLANNERDROPTARGET_H__9B440796_A576_4EA3_A635_BF9A998E387E__INCLUDED_)
