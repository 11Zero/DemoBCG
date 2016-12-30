#if !defined(AFX_BCGPPLANNERCLOCKICONS_H__B8C60AAF_318B_462A_A4E4_B8697060C209__INCLUDED_)
#define AFX_BCGPPLANNERCLOCKICONS_H__B8C60AAF_318B_462A_A4E4_B8697060C209__INCLUDED_

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
// BCGPPlannerClockIcons.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_PLANNER

/////////////////////////////////////////////////////////////////////////////
// CBCGPPlannerClockIcons

class BCGCBPRODLLEXPORT CBCGPPlannerClockIcons : public CObject
{
	DECLARE_DYNCREATE(CBCGPPlannerClockIcons)

public:
	virtual ~CBCGPPlannerClockIcons();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Overrides
public:
	virtual void DrawIcon (CDC* pDC, const CPoint& point, const COleDateTime& time);
	virtual CSize GetIconSize ();

	BOOL IsInitialized () const
	{
		return m_bInitialized;
	}

	
protected:

	CBCGPPlannerClockIcons();

protected:
	virtual void DrawIcon (CDC* pDC, const CPoint& point, int nHour, int nMin);

	virtual BOOL Initialize ();

	CImageList	m_ilClockIcons;

	CSize		m_szSize;
	BOOL		m_bInitialized;
};

#endif // BCGP_EXCLUDE_PLANNER

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPLANNERCLOCKICONS_H__B8C60AAF_318B_462A_A4E4_B8697060C209__INCLUDED_)
