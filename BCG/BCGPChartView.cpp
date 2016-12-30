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
// BCGPChartView.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPChartView.h"
#include "BCGPChartCtrl.h"
#include "BCGPChartLegend.h"
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
// CBCGPChartView

IMPLEMENT_DYNCREATE(CBCGPChartView, CScrollView)

CBCGPChartView::CBCGPChartView()
{
	m_pWndChartCtrl = NULL;
	m_pWndLegendCtrl = NULL;
}

CBCGPChartView::~CBCGPChartView()
{
}


BEGIN_MESSAGE_MAP(CBCGPChartView, CScrollView)
	//{{AFX_MSG_MAP(CBCGPChartView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CBCGPChartView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CBCGPChartView::OnFilePrint)
#ifndef _BCGSUITE_
	ON_REGISTERED_MESSAGE(BCGM_ON_PREPARE_TASKBAR_PREVIEW, OnPrepareTaskBarPreview)
#endif
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPChartView drawing

void CBCGPChartView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	CSize szDefault(100, 100);
	SetScrollSizes(MM_TEXT, szDefault);
}

BOOL CBCGPChartView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
	BOOL bRes = CScrollView::OnScrollBy(sizeScroll, bDoScroll);

	if (bDoScroll)
	{
		CRect rectClient;
		GetClientRect(rectClient);
		AdjustLayout(rectClient.Width(), rectClient.Height());
	}

	return bRes;
}

void CBCGPChartView::OnDraw(CDC* /*pDC*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPChartView diagnostics

#ifdef _DEBUG
void CBCGPChartView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CBCGPChartView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBCGPChartView message handlers

#define ID_CHART_CTRL 1
#define ID_LEGEND_CTRL 2

CBCGPChartCtrl* CBCGPChartView::CreateChart ()
{
	return new CBCGPChartCtrl;
}
//*******************************************************************************
CBCGPChartLegendCtrl* CBCGPChartView::CreateLegend()
{
	return new CBCGPChartLegendCtrl;
}
//*******************************************************************************
CBCGPChartLegendCtrl* CBCGPChartView::EnableAdvancedLegend(BOOL bEnable, 
		BCGPChartLayout::LegendPosition position,
		BOOL bIsVerticalLayout,
		CBCGPChartLegendVisualObject::LegendAlignment horzAlign,
		CBCGPChartLegendVisualObject::LegendAlignment vertAlign,
		CRuntimeClass* pLegendRTC)
{
	SetRedraw(FALSE);

	if (m_pWndLegendCtrl->GetSafeHwnd() != NULL)
	{
		m_pWndLegendCtrl->DestroyWindow();
	}

	if (m_pWndLegendCtrl != NULL)
	{
		delete m_pWndLegendCtrl;
		m_pWndLegendCtrl = NULL;
	}

	if (bEnable)
	{
		m_pWndLegendCtrl = CreateLegend();

		if (pLegendRTC != NULL)
		{
			m_pWndLegendCtrl->CreateCustomLegend(pLegendRTC);
		}

		m_pWndLegendCtrl->Create(CBCGPRect(), this, ID_LEGEND_CTRL);

		CBCGPChartLegendVisualObject* pLegend = m_pWndLegendCtrl->GetLegend();
		pLegend->SetHorizontalAlignment(horzAlign, FALSE);
		pLegend->SetVerticalAlignment(vertAlign, FALSE);
		pLegend->SetVerticalLayout(bIsVerticalLayout, FALSE);

		pLegend->AddRelatedChart(GetChart());

		m_legendPosition = position;
	}

	CRect rect;
	GetClientRect(rect);
	AdjustLayout(rect.Width(), rect.Height());

	SetRedraw(TRUE);
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);

	return m_pWndLegendCtrl;

}
//*******************************************************************************
int CBCGPChartView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_pWndChartCtrl = CreateChart ();
	if (m_pWndChartCtrl == NULL)
	{
		TRACE0("CBCGPChartView::OnCreate: Chart control is not created\n");
		return -1;
	}

	ASSERT_VALID (m_pWndChartCtrl);
	ASSERT (m_pWndChartCtrl->IsKindOf (RUNTIME_CLASS (CBCGPChartCtrl)));
	
	if (!m_pWndChartCtrl->Create (CBCGPRect(), this, ID_CHART_CTRL))
	{
		TRACE0("CBCGPChartView::OnCreate: cannot create Chart control\n");
		return -1;
	}

	return 0;
}
//*******************************************************************************
void CBCGPChartView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);
	AdjustLayout(cx, cy);
}
//*******************************************************************************
void CBCGPChartView::AdjustLayout(int nWidth, int nHeight)
{
	if (m_pWndChartCtrl->GetSafeHwnd () != NULL)
	{
		CSize szTotal = GetTotalSize();
		nWidth = max(szTotal.cx, nWidth);
		nHeight = max(szTotal.cy, nHeight);

		int nHorzOffset = -GetScrollPos(SB_HORZ);
		int nVertOffset = -GetScrollPos(SB_VERT);

		if (m_pWndLegendCtrl->GetSafeHwnd () == NULL)
		{
			m_pWndChartCtrl->SetWindowPos (NULL, nHorzOffset, nVertOffset, nWidth, nHeight,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
		else
		{
			CBCGPSize szLegend = m_pWndLegendCtrl->GetLegend()->GetLegendSize();
			szLegend += CSize(1, 1);

			switch (m_legendPosition)
			{
			case BCGPChartLayout::LP_RIGHT:
			case BCGPChartLayout::LP_TOPRIGHT:
				m_pWndChartCtrl->SetWindowPos (NULL, nHorzOffset, nVertOffset, nWidth - (int)szLegend.cx, nHeight,
					SWP_NOZORDER | SWP_NOACTIVATE);
				m_pWndLegendCtrl->SetWindowPos (NULL, nWidth - (int)szLegend.cx + nHorzOffset, nVertOffset, 
					(int)szLegend.cx, nHeight,SWP_NOZORDER | SWP_NOACTIVATE);
				break;

			case BCGPChartLayout::LP_LEFT:
				m_pWndLegendCtrl->SetWindowPos (NULL, nHorzOffset, nVertOffset, (int)szLegend.cx, nHeight,
					SWP_NOZORDER | SWP_NOACTIVATE);
				m_pWndChartCtrl->SetWindowPos (NULL, (int)szLegend.cx + nHorzOffset, nVertOffset, nWidth - (int)szLegend.cx, nHeight,
					SWP_NOZORDER | SWP_NOACTIVATE);
				break;

			case BCGPChartLayout::LP_TOP:
				m_pWndLegendCtrl->SetWindowPos (NULL, nHorzOffset, nVertOffset, nWidth, (int)szLegend.cy, 
					SWP_NOZORDER | SWP_NOACTIVATE);
				m_pWndChartCtrl->SetWindowPos (NULL, nHorzOffset, (int)szLegend.cy + nVertOffset, nWidth, nHeight - (int)szLegend.cy,
					SWP_NOZORDER | SWP_NOACTIVATE);
				break;

			case BCGPChartLayout::LP_BOTTOM:
				m_pWndChartCtrl->SetWindowPos (NULL, nHorzOffset, nVertOffset, nWidth, nHeight - (int)szLegend.cy, 
					SWP_NOZORDER | SWP_NOACTIVATE);

				m_pWndLegendCtrl->SetWindowPos (NULL, nHorzOffset, nHeight - (int)szLegend.cy + nVertOffset, nWidth, (int)szLegend.cy, 
					SWP_NOZORDER | SWP_NOACTIVATE);
				break;
			}
		}
	}
}
//*******************************************************************************
BOOL CBCGPChartView::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}
//*******************************************************************************
void CBCGPChartView::OnSetFocus(CWnd* pOldWnd) 
{
	CScrollView::OnSetFocus(pOldWnd);
	
	if (m_pWndChartCtrl != NULL)
	{
		ASSERT_VALID (m_pWndChartCtrl);
		m_pWndChartCtrl->SetFocus();
	}
}
//*******************************************************************************
void CBCGPChartView::OnDestroy() 
{
	if (m_pWndChartCtrl != NULL)
	{
		ASSERT_VALID (m_pWndChartCtrl);

		m_pWndChartCtrl->DestroyWindow ();
		delete m_pWndChartCtrl;
		m_pWndChartCtrl = NULL;
	}

	if (m_pWndLegendCtrl != NULL)
	{
		ASSERT_VALID (m_pWndLegendCtrl);
		
		m_pWndLegendCtrl->DestroyWindow ();
		delete m_pWndLegendCtrl;
		m_pWndLegendCtrl = NULL;
	}

	CScrollView::OnDestroy();
}
//*******************************************************************************
BOOL CBCGPChartView::OnPreparePrinting(CPrintInfo* pInfo) 
{
	return DoPreparePrinting(pInfo);
}
//*******************************************************************************
void CBCGPChartView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/) 
{
}
//*******************************************************************************
void CBCGPChartView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/) 
{
}
//*******************************************************************************
void CBCGPChartView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	if (m_pWndChartCtrl == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pWndChartCtrl);
	m_pWndChartCtrl->DoPrint(pDC, pInfo);
}
//*******************************************************************************
void CBCGPChartView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	CScrollView::OnPrepareDC(pDC, pInfo);
}
//*******************************************************************************
void CBCGPChartView::OnFilePrintPreview()
{
	BCGPPrintPreview(this);
}
//*******************************************************************************
CBCGPChartVisualObject* CBCGPChartView::GetChart() const
{
	ASSERT_VALID(this);

	if (m_pWndChartCtrl == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(m_pWndChartCtrl);
	return m_pWndChartCtrl->GetChart();
}
//****************************************************************************
CBCGPChartLegendVisualObject* CBCGPChartView::GetLegend() const
{
	ASSERT_VALID(this);
	
	if (m_pWndLegendCtrl == NULL)
	{
		return NULL;
	}
	
	ASSERT_VALID(m_pWndLegendCtrl);
	return m_pWndLegendCtrl->GetLegend();
}
//****************************************************************************
LRESULT CBCGPChartView::OnPrepareTaskBarPreview(WPARAM wp, LPARAM /*lp*/)
{
	if (m_pWndChartCtrl == NULL)
	{
		return 0;
	}

	ASSERT_VALID (m_pWndChartCtrl);

	CDC* pDC = CDC::FromHandle((HDC) wp);
	ASSERT_VALID(pDC);

	m_pWndChartCtrl->DoPaint(pDC);
	return 1;
}
