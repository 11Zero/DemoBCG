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
// BCGPChartCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPChartCtrl.h"

// CBCGPChartCtrl

IMPLEMENT_DYNAMIC(CBCGPChartCtrl, CBCGPVisualCtrl)
IMPLEMENT_DYNAMIC(CBCGPChartLegendCtrl, CBCGPVisualCtrl)

CBCGPChartCtrl::CBCGPChartCtrl()
{
	m_pChart = NULL;
	m_bTooltipTrackingMode = TRUE;
}

CBCGPChartCtrl::~CBCGPChartCtrl()
{
	if (m_pChart != NULL)
	{
		delete m_pChart;
	}
}

BEGIN_MESSAGE_MAP(CBCGPChartCtrl, CBCGPVisualCtrl)
	//{{AFX_MSG_MAP(CBCGPChartCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CBCGPChartCtrl::CreateCustomChart(CRuntimeClass* pChartRTC)
{
	if (pChartRTC != NULL && !pChartRTC->IsDerivedFrom(RUNTIME_CLASS(CBCGPChartVisualObject)))
	{
		ASSERT(FALSE);
		TRACE0("CBCGPChartCtrl::CreateCustomChart: custom chart container must be derived from CBCGPChartVisualContainer.");
		return FALSE;
	}

	if (m_pChart != NULL)
	{
		delete m_pChart;
	}

	if (pChartRTC != NULL)
	{
		m_pChart = DYNAMIC_DOWNCAST(CBCGPChartVisualObject, pChartRTC->CreateObject());
	}
	else
	{
		m_pChart = new CBCGPChartVisualObject();	
	}
	
	if (m_pChart != NULL)
	{
		m_pChart->SetOwner(this);
	}

	ASSERT_VALID(m_pChart);
	return TRUE;
}

BOOL CBCGPChartCtrl::DoPrint(CDC* pDC, CPrintInfo* pInfo)
{
	if (m_pChart == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(m_pChart);

	BOOL bIs3D = m_pChart->IsChart3D();
	CBCGPEngine3D* pEngine = m_pChart->GetEngine3D();
	BOOL bIsRendertoWindow = FALSE;

	if (bIs3D && pEngine != NULL)
	{
		ASSERT_VALID(pEngine);

		bIsRendertoWindow = pEngine->IsRenderToWindow();
		if (bIsRendertoWindow)
		{
			pEngine->SetRenderToWindow(FALSE);
		}
	}

	BOOL bRet = CBCGPVisualCtrl::DoPrint(pDC, pInfo);

	if (bIs3D && pEngine != NULL && bIsRendertoWindow)
	{
		pEngine->SetRenderToWindow(TRUE);
	}

	return bRet;
}
//*******************************************************************************
// Legend control implementation
//*******************************************************************************
CBCGPChartLegendCtrl::CBCGPChartLegendCtrl()
{
	m_pLegend = NULL;
	m_bTooltipTrackingMode = TRUE;
}

CBCGPChartLegendCtrl::~CBCGPChartLegendCtrl()
{
	if (m_pLegend != NULL)
	{
		delete m_pLegend;
	}
}

BEGIN_MESSAGE_MAP(CBCGPChartLegendCtrl, CBCGPVisualCtrl)
//{{AFX_MSG_MAP(CBCGPChartLegendCtrl)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CBCGPChartLegendCtrl::CreateCustomLegend(CRuntimeClass* pLegendRTC)
{
	if (pLegendRTC != NULL && !pLegendRTC->IsDerivedFrom(RUNTIME_CLASS(CBCGPChartLegendVisualObject)))
	{
		ASSERT(FALSE);
		TRACE0("CBCGPChartLegendCtrl::CreateCustomLegend: custom legend must be derived from CBCGPChartLegend.");
		return FALSE;
	}
	
	if (m_pLegend != NULL)
	{
		delete m_pLegend;
	}
	
	if (pLegendRTC != NULL)
	{
		m_pLegend = DYNAMIC_DOWNCAST(CBCGPChartLegendVisualObject, pLegendRTC->CreateObject());
	}
	else
	{
		m_pLegend = new CBCGPChartLegendVisualObject();	
	}
	
	if (m_pLegend != NULL)
	{
		m_pLegend->SetOwner(this);
	}
	
	ASSERT_VALID(m_pLegend);
	return TRUE;
}
