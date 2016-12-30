#if !defined(AFX_BCGPREPORTVIEW_H__5E51E0E0_FDF2_4D68_AE9E_9FD70665EFD0__INCLUDED_)
#define AFX_BCGPREPORTVIEW_H__5E51E0E0_FDF2_4D68_AE9E_9FD70665EFD0__INCLUDED_

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
// BCGPReportView.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_GRID_CTRL

#include "BCGPGridView.h"

class CBCGPReportCtrl;

/////////////////////////////////////////////////////////////////////////////
// CBCGPReportView view

class BCGCBPRODLLEXPORT CBCGPReportView : public CBCGPGridView
{
protected:
	CBCGPReportView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBCGPReportView)

// Attributes
public:
	CBCGPReportCtrl* GetReportCtrl () const
	{
		return (CBCGPReportCtrl*) m_pWndGridCtrl;
	}

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPReportView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CBCGPReportView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	virtual CBCGPGridCtrl* CreateGrid ();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPReportView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // BCGP_EXCLUDE_GRID_CTRL

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPREPORTVIEW_H__5E51E0E0_FDF2_4D68_AE9E_9FD70665EFD0__INCLUDED_)
