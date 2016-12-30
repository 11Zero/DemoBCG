#if !defined(AFX_BCGPINTELLISENSEWND_H__2D884D5A_B4C1_4EE0_A5D6_3D8703CA99D1__INCLUDED_)
#define AFX_BCGPINTELLISENSEWND_H__2D884D5A_B4C1_4EE0_A5D6_3D8703CA99D1__INCLUDED_

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
// BCGPIntelliSenseWnd.h : header file
//

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_EDIT_CTRL

#include "BCGPIntelliSenseLB.h"

class CBCGPEditCtrl;

/////////////////////////////////////////////////////////////////////////////
// CBCGPIntelliSenseWnd frame

class BCGCBPRODLLEXPORT CBCGPIntelliSenseWnd : public CMiniFrameWnd
{
	DECLARE_DYNCREATE(CBCGPIntelliSenseWnd)
public:
	CBCGPIntelliSenseWnd();           // protected constructor used by dynamic creation

	virtual BOOL Create (CObList& lstData, DWORD dwStyle, CPoint pt, 
						 CWnd* pParentWnd, CFont* pLBFont, CImageList* pImageList = NULL, 
						 CRuntimeClass* pLBDataRTC = NULL);
// Attributes
public:
	CBCGPEditCtrl* GetParentEditCtrl () {return m_pParentEditCtrl;}
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPIntelliSenseWnd)
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CBCGPIntelliSenseWnd();

	// Generated message map functions
	//{{AFX_MSG(CBCGPIntelliSenseWnd)
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg LRESULT DeleteItem (WPARAM, LPARAM);
	afx_msg BCGNcHitTestType OnNcHitTest(CPoint point);
	DECLARE_MESSAGE_MAP()

	CBCGPBaseIntelliSenseLB*	m_pLstBoxData;
	CFont						m_lbFont;
	CBCGPEditCtrl*				m_pParentEditCtrl;
};

#endif	// BCGP_EXCLUDE_EDIT_CTRL

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPINTELLISENSEWND_H__2D884D5A_B4C1_4EE0_A5D6_3D8703CA99D1__INCLUDED_)
