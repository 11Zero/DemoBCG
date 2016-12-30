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
// BCGPReportView.cpp : implementation file
//

#include "stdafx.h"

#include "BCGPReportCtrl.h"
#include "BCGPReportView.h"

#ifndef BCGP_EXCLUDE_GRID_CTRL

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPReportView

IMPLEMENT_DYNCREATE(CBCGPReportView, CBCGPGridView)

CBCGPReportView::CBCGPReportView()
{
}

CBCGPReportView::~CBCGPReportView()
{
}


BEGIN_MESSAGE_MAP(CBCGPReportView, CBCGPGridView)
	//{{AFX_MSG_MAP(CBCGPReportView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPReportView drawing

void CBCGPReportView::OnDraw(CDC* pDC)
{
	CBCGPGridView::OnDraw (pDC);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPReportView diagnostics

#ifdef _DEBUG
void CBCGPReportView::AssertValid() const
{
	CBCGPGridView::AssertValid();
}

void CBCGPReportView::Dump(CDumpContext& dc) const
{
	CBCGPGridView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBCGPReportView message handlers

CBCGPGridCtrl* CBCGPReportView::CreateGrid ()
{
	return new CBCGPReportCtrl;
}

#endif // BCGP_EXCLUDE_GRID_CTRL
