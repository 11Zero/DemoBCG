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
// BCGPOutlookBarDockingPane.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPOutlookBar.h"
#include "BCGPOutlookBarDockingPane.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CBCGPOutlookBarDockingPane, CBCGPDockingCBWrapper, VERSIONABLE_SCHEMA | 2)

/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookBarDockingPane

CBCGPOutlookBarDockingPane::CBCGPOutlookBarDockingPane()
{
	m_pTabbedControlBarRTC = RUNTIME_CLASS (CBCGPOutlookBar);
}

CBCGPOutlookBarDockingPane::~CBCGPOutlookBarDockingPane()
{
}

BEGIN_MESSAGE_MAP(CBCGPOutlookBarDockingPane, CBCGPDockingCBWrapper)
	//{{AFX_MSG_MAP(CBCGPOutlookBarDockingPane)
	ON_WM_NCDESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPOutlookBarDockingPane message handlers

void CBCGPOutlookBarDockingPane::OnNcDestroy() 
{
	CBCGPDockingCBWrapper::OnNcDestroy();
	delete this;
}
