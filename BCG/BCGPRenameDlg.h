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
//
// BCGPRenameDlg.h : header file
//

#if !defined(AFX_BCGPRENAMEDLG_H__45DB8612_40F2_40AC_B521_A56EA313122F__INCLUDED_)
#define AFX_BCGPRENAMEDLG_H__45DB8612_40F2_40AC_B521_A56EA313122F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "bcgprores.h"
#include "BCGPDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CBCGPRenameDlg dialog

class BCGCBPRODLLEXPORT CBCGPRenameDlg : public CBCGPDialog
{
// Construction
public:
	CBCGPRenameDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBCGPRenameDlg)
	enum { IDD = IDD_BCGBARRES_RENAME_DLG };
	CString	m_strName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPRenameDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBCGPRenameDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPRENAMEDLG_H__45DB8612_40F2_40AC_B521_A56EA313122F__INCLUDED_)
