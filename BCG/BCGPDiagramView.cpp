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
// BCGPDiagramView.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPDiagramView.h"
#include "BCGPDiagramCtrl.h"
#include "BCGPDrawManager.h"
#include "bcgpmath.h"

#ifndef _BCGSUITE_
#include "BCGPMDIChildWnd.h"
#include "BCGPPrintPreviewView.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPDiagramView

IMPLEMENT_DYNCREATE(CBCGPDiagramView, CView)

CBCGPDiagramView::CBCGPDiagramView()
{
	m_pWndDiagramCtrl = NULL;
}

CBCGPDiagramView::~CBCGPDiagramView()
{
}


BEGIN_MESSAGE_MAP(CBCGPDiagramView, CView)
	//{{AFX_MSG_MAP(CBCGPDiagramView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_PRINT, CBCGPDiagramView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CBCGPDiagramView::OnFilePrint)
#ifndef _BCGSUITE_
	ON_REGISTERED_MESSAGE(BCGM_ON_PREPARE_TASKBAR_PREVIEW, OnPrepareTaskBarPreview)
#endif
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPDiagramView drawing

void CBCGPDiagramView::OnDraw(CDC* /*pDC*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPDiagramView diagnostics

#ifdef _DEBUG
void CBCGPDiagramView::AssertValid() const
{
	CView::AssertValid();
}

void CBCGPDiagramView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBCGPDiagramView message handlers

#define ID_Diagram_CTRL 1

CBCGPDiagramVisualContainerCtrl* CBCGPDiagramView::CreateDiagram ()
{
	return new CBCGPDiagramVisualContainerCtrl;
}
//*******************************************************************************
int CBCGPDiagramView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_pWndDiagramCtrl = CreateDiagram ();
	if (m_pWndDiagramCtrl == NULL)
	{
		TRACE0("CBCGPDiagramView::OnCreate: Diagram control is not created\n");
		return -1;
	}

	ASSERT_VALID (m_pWndDiagramCtrl);
	ASSERT (m_pWndDiagramCtrl->IsKindOf (RUNTIME_CLASS (CBCGPDiagramVisualContainerCtrl)));
	
	if (!m_pWndDiagramCtrl->Create (CBCGPRect(), this, ID_Diagram_CTRL))
	{
		TRACE0("CBCGPDiagramView::OnCreate: cannot create Diagram control\n");
		return -1;
	}

	CBCGPDiagramVisualContainer* pDiagram = GetDiagram();
	if (pDiagram != NULL)
	{
		pDiagram->EnableScrollBars();
	}

	return 0;
}
//*******************************************************************************
void CBCGPDiagramView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	if (m_pWndDiagramCtrl->GetSafeHwnd () != NULL)
	{
		m_pWndDiagramCtrl->SetWindowPos (NULL, -1, -1, cx, cy,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
}
//*******************************************************************************
BOOL CBCGPDiagramView::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//*******************************************************************************
void CBCGPDiagramView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	
	if (m_pWndDiagramCtrl != NULL)
	{
		ASSERT_VALID (m_pWndDiagramCtrl);
		m_pWndDiagramCtrl->SetFocus();
	}
}
//*******************************************************************************
void CBCGPDiagramView::OnDestroy() 
{
	if (m_pWndDiagramCtrl != NULL)
	{
		ASSERT_VALID (m_pWndDiagramCtrl);

		m_pWndDiagramCtrl->DestroyWindow ();
		delete m_pWndDiagramCtrl;
		m_pWndDiagramCtrl = NULL;
	}

	CView::OnDestroy();
}
//*******************************************************************************
BOOL CBCGPDiagramView::OnPreparePrinting(CPrintInfo* pInfo) 
{
	return DoPreparePrinting(pInfo);
}
//*******************************************************************************
void CBCGPDiagramView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/) 
{
}
//*******************************************************************************
void CBCGPDiagramView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/) 
{
}
//*******************************************************************************
void CBCGPDiagramView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	if (m_pWndDiagramCtrl == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pWndDiagramCtrl);
	m_pWndDiagramCtrl->DoPrint(pDC, pInfo);
}
//*******************************************************************************
void CBCGPDiagramView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	CView::OnPrepareDC(pDC, pInfo);
}
//*******************************************************************************
void CBCGPDiagramView::OnFilePrintPreview()
{
	BCGPPrintPreview(this);
}
//*******************************************************************************
CBCGPDiagramVisualContainer* CBCGPDiagramView::GetDiagram() const
{
	ASSERT_VALID(this);

	if (m_pWndDiagramCtrl == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(m_pWndDiagramCtrl);
	return DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainer, m_pWndDiagramCtrl->GetVisualContainer());
}
//****************************************************************************
LRESULT CBCGPDiagramView::OnPrepareTaskBarPreview(WPARAM wp, LPARAM /*lp*/)
{
	if (m_pWndDiagramCtrl == NULL)
	{
		return 0;
	}

	ASSERT_VALID (m_pWndDiagramCtrl);

	CDC* pDC = CDC::FromHandle((HDC) wp);
	ASSERT_VALID(pDC);

	m_pWndDiagramCtrl->DoPaint(pDC);
	return 1;
}
