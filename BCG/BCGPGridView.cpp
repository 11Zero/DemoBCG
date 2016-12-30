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
// BCGPGridView.cpp : implementation file
//

#include "stdafx.h"

#include "BCGPGridCtrl.h"
#include "BCGPGridView.h"

#ifndef BCGP_EXCLUDE_GRID_CTRL

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridView

IMPLEMENT_DYNCREATE(CBCGPGridView, CView)

CBCGPGridView::CBCGPGridView()
{
	m_pWndGridCtrl = NULL;
}

CBCGPGridView::~CBCGPGridView()
{
}


BEGIN_MESSAGE_MAP(CBCGPGridView, CView)
	//{{AFX_MSG_MAP(CBCGPGridView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridView drawing

void CBCGPGridView::OnDraw(CDC* /*pDC*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridView diagnostics

#ifdef _DEBUG
void CBCGPGridView::AssertValid() const
{
	CView::AssertValid();
}
//*******************************************************************************
void CBCGPGridView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBCGPGridView message handlers

#define ID_GRIDCTRL 1

CBCGPGridCtrl* CBCGPGridView::CreateGrid ()
{
	return new CBCGPGridCtrl;
}
//*******************************************************************************
int CBCGPGridView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_pWndGridCtrl = CreateGrid ();
	if (m_pWndGridCtrl == NULL)
	{
		TRACE0("CBCGPGridView::OnCreate: grid control is not created\n");
		return -1;
	}

	ASSERT_VALID (m_pWndGridCtrl);
	ASSERT (m_pWndGridCtrl->IsKindOf (RUNTIME_CLASS (CBCGPGridCtrl)));
	
	if (!m_pWndGridCtrl->Create (WS_CHILD | WS_VISIBLE, 
		CRect (0, 0, 0, 0), this, ID_GRIDCTRL))
	{
		TRACE0("CBCGPGridView::OnCreate: cannot create grid control\n");
		return -1;
	}

	return 0;
}
//*******************************************************************************
void CBCGPGridView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	if (m_pWndGridCtrl->GetSafeHwnd () != NULL)
	{
		m_pWndGridCtrl->SetWindowPos (NULL, -1, -1, cx, cy,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
}
//*******************************************************************************
BOOL CBCGPGridView::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//*******************************************************************************
void CBCGPGridView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	
	if (m_pWndGridCtrl != NULL)
	{
		ASSERT_VALID (m_pWndGridCtrl);
		m_pWndGridCtrl->SetFocus();
	}
}
//*******************************************************************************
void CBCGPGridView::OnDestroy() 
{
	if (m_pWndGridCtrl != NULL)
	{
		ASSERT_VALID (m_pWndGridCtrl);

		m_pWndGridCtrl->DestroyWindow ();
		delete m_pWndGridCtrl;
		m_pWndGridCtrl = NULL;
	}

	CView::OnDestroy();
}
//*******************************************************************************
BOOL CBCGPGridView::OnPreparePrinting(CPrintInfo* pInfo) 
{
	if (m_pWndGridCtrl != NULL)
	{
		ASSERT_VALID (m_pWndGridCtrl);

		int nFirstItem = 0;										// By default print all grid items
		int nLastItem  = max(m_pWndGridCtrl->GetTotalItems () - 1, 0);	// from first row to the last
		m_pWndGridCtrl->OnPreparePrintPages (pInfo, nFirstItem, nLastItem);
	}

	return DoPreparePrinting(pInfo);
}
//*******************************************************************************
void CBCGPGridView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	if (m_pWndGridCtrl != NULL)
	{
		ASSERT_VALID (m_pWndGridCtrl);

		ASSERT_VALID (pDC);
		ASSERT (pInfo != NULL);

		int nFirstItem = 0;										// By default print all grid items
		int nLastItem  = max(m_pWndGridCtrl->GetTotalItems () - 1, 0);	// from first row to the last
		m_pWndGridCtrl->OnPreparePrintPages (pInfo, nFirstItem, nLastItem);

		ASSERT (pInfo == m_pWndGridCtrl->m_PrintParams.m_pPrintInfo);
		ASSERT (pInfo->m_lpUserData != NULL);

		m_pWndGridCtrl->m_bIsPrinting = TRUE;
		m_pWndGridCtrl->m_pPrintDC = pDC;

		m_pWndGridCtrl->OnBeginPrinting (pDC, pInfo);

		// The printable area has not been initialized. Initialize it.
		pInfo->m_rectDraw.SetRect (0, 0,
								pDC->GetDeviceCaps(HORZRES), 
								pDC->GetDeviceCaps(VERTRES));

		// Page margins:
		CRect rectMargins = m_pWndGridCtrl->OnGetPageMargins (pDC, pInfo);
		pInfo->m_rectDraw.DeflateRect (&rectMargins);

		// Specify pages count:
		int nPagesCount = m_pWndGridCtrl->OnCalcPrintPages (pDC, pInfo);
		pInfo->SetMaxPage (nPagesCount);
	}
}
//*******************************************************************************
void CBCGPGridView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	if (m_pWndGridCtrl != NULL)
	{
		ASSERT_VALID (m_pWndGridCtrl);
		m_pWndGridCtrl->OnEndPrinting (pDC, pInfo);
		
		m_pWndGridCtrl->m_bIsPrinting = FALSE;
		m_pWndGridCtrl->m_pPrintDC = NULL;

		m_pWndGridCtrl->AdjustLayout ();
	}
}
//*******************************************************************************
void CBCGPGridView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	if (m_pWndGridCtrl != NULL)
	{
		ASSERT_VALID (pDC);
		ASSERT (pInfo != NULL);
		ASSERT_VALID (m_pWndGridCtrl);

		// don't do anything if not fully initialized
		if (!m_pWndGridCtrl->m_bIsPrinting || m_pWndGridCtrl->m_pPrintDC == NULL)
		{
			return;
		}

		// Page margins:
		CRect rectMargins = m_pWndGridCtrl->OnGetPageMargins (pDC, pInfo);
		pInfo->m_rectDraw.DeflateRect (&rectMargins);

		m_pWndGridCtrl->OnPrint (pDC, pInfo);
	}
}
//*******************************************************************************
void CBCGPGridView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	CView::OnPrepareDC(pDC, pInfo);
}

#endif // BCGP_EXCLUDE_GRID_CTRL
