#if !defined(AFX_BCGPBASETOOLBAR_H__70C8E89D_B209_4FF7_895F_69B66706C18E__INCLUDED_)
#define AFX_BCGPBASETOOLBAR_H__70C8E89D_B209_4FF7_895F_69B66706C18E__INCLUDED_

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
// BCGPToolBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseToolBar window

#include "BCGCBPro.h"
#include "BCGPControlBar.h"

class BCGCBPRODLLEXPORT CBCGPBaseToolBar : public CBCGPControlBar
{
	DECLARE_DYNCREATE(CBCGPBaseToolBar)
// Construction
public:
	CBCGPBaseToolBar();

// Attributes
public:
	virtual BCGP_DOCK_TYPE GetDockMode () const 
	{
		return BCGP_DT_IMMEDIATE;
	}

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPBaseToolBar)
	//}}AFX_VIRTUAL

	virtual void OnAfterChangeParent	(CWnd* pWndOldParent);
	virtual void GetMinSize (CSize& size) const  
	{
		if (IsHorizontal ())
		{
			size.cx = 40; size.cy = 16;
		}
		else
		{
			size.cy = 40; size.cx = 16;
		}
	}

// Implementation
public:
	virtual ~CBCGPBaseToolBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPBaseToolBar)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual void OnAfterStretch (int nStretchSize);
	virtual BOOL OnShowControlBarMenu (CPoint /*point*/)	{	return FALSE;	}
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPBASETOOLBAR_H__70C8E89D_B209_4FF7_895F_69B66706C18E__INCLUDED_)
