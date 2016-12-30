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
// BCGPFormView.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPFormView.h"
#include "BCGPVisualManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPFormView

IMPLEMENT_DYNAMIC(CBCGPFormView, CFormView)

#pragma warning (disable : 4355)

CBCGPFormView::CBCGPFormView(LPCTSTR lpszTemplateName)
	: CFormView(lpszTemplateName),
	m_Impl (*this)
{
	m_bInitDlgCompleted = FALSE;
}

CBCGPFormView::CBCGPFormView(UINT nIDTemplate)
	: CFormView(nIDTemplate)
	, m_Impl (*this)
{
	m_bInitDlgCompleted = FALSE;
}

#pragma warning (default : 4355)

CBCGPFormView::~CBCGPFormView()
{
}

BEGIN_MESSAGE_MAP(CBCGPFormView, CFormView)
	//{{AFX_MSG_MAP(CBCGPFormView)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(BCGM_CHANGEVISUALMANAGER, OnChangeVisualManager)	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPFormView diagnostics

#ifdef _DEBUG
void CBCGPFormView::AssertValid() const
{
	CFormView::AssertValid();
}

void CBCGPFormView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBCGPFormView message handlers

HBRUSH CBCGPFormView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	if (IsVisualManagerStyle () || (IsWhiteBackground() && !globalData.IsHighContastMode() && !CBCGPVisualManager::GetInstance ()->IsDarkTheme()))
	{
		HBRUSH hbr = m_Impl.OnCtlColor (pDC, pWnd, nCtlColor);
		if (hbr != NULL)
		{
			return hbr;
		}
	}	

	return CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
}
//************************************************************************
BOOL CBCGPFormView::OnEraseBkgnd(CDC* pDC) 
{
	CRect rectClient;
	GetClientRect (rectClient);

	if (IsWhiteBackground() && !globalData.IsHighContastMode() && !CBCGPVisualManager::GetInstance ()->IsDarkTheme())
	{
		pDC->FillSolidRect(rectClient, RGB(255, 255, 255));
		return TRUE;
	}

	if (IsVisualManagerStyle () &&
		CBCGPVisualManager::GetInstance ()->OnFillDialog (pDC, this, rectClient))
	{
		return TRUE;
	}

	return CFormView::OnEraseBkgnd(pDC);
}
//************************************************************************
void CBCGPFormView::EnableVisualManagerStyle (BOOL bEnable, const CList<UINT,UINT>* plstNonSubclassedItems)
{
	ASSERT_VALID (this);

	m_Impl.EnableVisualManagerStyle (bEnable, FALSE, plstNonSubclassedItems);
	m_Impl.m_bTransparentStaticCtrls = FALSE;
}
//************************************************************************
void CBCGPFormView::SetWhiteBackground(BOOL bSet)
{
	ASSERT_VALID (this);

	m_Impl.m_bIsWhiteBackground = bSet && !globalData.IsHighContastMode();
}
//************************************************************************
void CBCGPFormView::OnInitialUpdate() 
{
	m_bInitDlgCompleted = TRUE;

	CFormView::OnInitialUpdate();
	
	if (IsVisualManagerStyle ())
	{
		m_Impl.EnableVisualManagerStyle (TRUE);
	}
}
//************************************************************************
void CBCGPFormView::OnDestroy() 
{
	m_Impl.OnDestroy ();
	CFormView::OnDestroy();
}
//************************************************************************
void CBCGPFormView::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	AdjustControlsLayout();
}
//************************************************************************
int CBCGPFormView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    return m_Impl.OnCreate();
}
//************************************************************************
LRESULT CBCGPFormView::OnChangeVisualManager (WPARAM, LPARAM)
{
	if (IsVisualManagerStyle ())
	{
		m_Impl.OnChangeVisualManager ();
	}

	return 0L;
}
