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
// BCGPRenameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPRenameDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPRenameDlg dialog

CBCGPRenameDlg::CBCGPRenameDlg(CWnd* pParent /*=NULL*/)
	: CBCGPDialog(CBCGPRenameDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBCGPRenameDlg)
	m_strName = _T("");
	//}}AFX_DATA_INIT

	m_bIsLocal = TRUE;

	EnableVisualManagerStyle (globalData.m_bUseVisualManagerInBuiltInDialogs, TRUE);
}
//************************************************************************************
void CBCGPRenameDlg::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPRenameDlg)
	DDX_Text(pDX, IDC_BCGBARRES_NAME, m_strName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBCGPRenameDlg, CBCGPDialog)
	//{{AFX_MSG_MAP(CBCGPRenameDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPRenameDlg message handlers
