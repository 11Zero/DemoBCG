#if !defined(AFX_BCGPOUTLOOKBARDOCKINGPANE_H__3AD22F5A_0095_4294_8F58_6B0903806725__INCLUDED_)
#define AFX_BCGPOUTLOOKBARDOCKINGPANE_H__3AD22F5A_0095_4294_8F58_6B0903806725__INCLUDED_

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
// BCGPOutlookBarDockingPane.h : header file
//

#include "BCGCBPro.h"
#include "BCGPDockingCBWrapper.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookBarDockingPane window

class BCGCBPRODLLEXPORT CBCGPOutlookBarDockingPane : public CBCGPDockingCBWrapper
{
	DECLARE_SERIAL(CBCGPOutlookBarDockingPane)
// Construction
public:
	CBCGPOutlookBarDockingPane();

// Attributes
public:
	
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPOutlookBarDockingPane)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPOutlookBarDockingPane();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPOutlookBarDockingPane)
	afx_msg void OnNcDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPOUTLOOKBARDOCKINGPANE_H__3AD22F5A_0095_4294_8F58_6B0903806725__INCLUDED_)
