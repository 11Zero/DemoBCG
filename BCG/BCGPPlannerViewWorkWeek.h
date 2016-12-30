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
// BCGPPlannerViewWorkWeek.h: interface for the CBCGPPlannerViewWorkWeek class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPPLANNERVIEWWORKWEEK_H__7D9C12DD_5B44_46BC_9547_F7ED77FA50C1__INCLUDED_)
#define AFX_BCGPPLANNERVIEWWORKWEEK_H__7D9C12DD_5B44_46BC_9547_F7ED77FA50C1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

#include "BCGPPlannerViewDay.h"

class BCGCBPRODLLEXPORT CBCGPPlannerViewWorkWeek : public CBCGPPlannerViewDay  
{
	friend class CBCGPPlannerManagerCtrl;

	DECLARE_DYNCREATE(CBCGPPlannerViewWorkWeek)

public:
	virtual ~CBCGPPlannerViewWorkWeek();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual void SetDate (const COleDateTime& date);
	virtual void SetDateInterval (const COleDateTime& /*date1*/, const COleDateTime& /*date2*/){}

	virtual COleDateTime CalculateDateStart (const COleDateTime& date) const;

protected:
	CBCGPPlannerViewWorkWeek();

	virtual void OnActivate(CBCGPPlannerManagerCtrl* pPlanner, const CBCGPPlannerView* pOldView);

	virtual COleDateTime GetNextDateStart () const;
	virtual COleDateTime GetPrevDateStart () const;

	int GetWorkWeekInterval () const;
	
	virtual CString GetAccName() const;
};

#endif // BCGP_EXCLUDE_PLANNER

#endif // !defined(AFX_BCGPPLANNERWORKWEEKVIEW_H__7D9C12DD_5B44_46BC_9547_F7ED77FA50C1__INCLUDED_)
