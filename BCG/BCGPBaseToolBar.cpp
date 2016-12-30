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
// BCGPBaseToolBar.cpp : implementation file
//

#include "stdafx.h"

#include "BCGPDockBar.h"

#include "BCGPBaseToolBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CBCGPBaseToolBar,CBCGPControlBar)

/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseToolBar

CBCGPBaseToolBar::CBCGPBaseToolBar()
{
}

CBCGPBaseToolBar::~CBCGPBaseToolBar()
{
}


BEGIN_MESSAGE_MAP(CBCGPBaseToolBar, CBCGPControlBar)
	//{{AFX_MSG_MAP(CBCGPBaseToolBar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CBCGPBaseToolBar message handlers

void CBCGPBaseToolBar::OnAfterChangeParent (CWnd* /*pWndOldParent*/)
{
}

void CBCGPBaseToolBar::OnAfterStretch (int /*nStretchSize*/)
{
	CRect rectClient;
	GetClientRect (&rectClient);

	if (rectClient.Width () != m_rectVirtual.Width ())
	{
		Invalidate ();
		UpdateWindow ();
	}
}

