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
// BCGPChartCtrl.h: declaration of the CBCGPChartCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPCHARTCTRL_H__8E3DB7C8_BD56_4654_89A6_3F0B67AD7B4B__INCLUDED_)
#define AFX_BCGPCHARTCTRL_H__8E3DB7C8_BD56_4654_89A6_3F0B67AD7B4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPChartVisualObject.h"
#include "BCGPChartLegend.h"
#include "BCGPVisualCtrl.h"

// CBCGPChartCtrl

class CBCGPChartVisualObject;

class BCGCBPRODLLEXPORT CBCGPChartCtrl : public CBCGPVisualCtrl
{
	DECLARE_DYNAMIC(CBCGPChartCtrl)

public:
	CBCGPChartCtrl();
	virtual ~CBCGPChartCtrl();

	BOOL CreateCustomChart(CRuntimeClass* pChartRTC);

	virtual CBCGPChartVisualObject*	GetChart()
	{
		if (m_pChart == NULL)
		{
			m_pChart = new CBCGPChartVisualObject();
		}

		return m_pChart;
	}

	virtual BOOL DoPrint(CDC* pDC = NULL, CPrintInfo* pInfo = NULL);

protected:
	//{{AFX_MSG(CBCGPChartCtrl)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	virtual CBCGPBaseVisualObject* GetVisualObject()
	{
		return GetChart();
	}

protected:
	CBCGPChartVisualObject*	m_pChart;
};

class BCGCBPRODLLEXPORT CBCGPChartLegendCtrl : public CBCGPVisualCtrl
{
	DECLARE_DYNAMIC(CBCGPChartLegendCtrl)

public:
	CBCGPChartLegendCtrl();
	virtual ~CBCGPChartLegendCtrl();

	BOOL CreateCustomLegend(CRuntimeClass* pLegendRTC);

	virtual CBCGPChartLegendVisualObject*	GetLegend()
	{
		if (m_pLegend == NULL)
		{
			m_pLegend = new CBCGPChartLegendVisualObject();
		}
		
		return m_pLegend;
	}

protected:
	//{{AFX_MSG(CBCGPChartLegendCtrl)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	virtual CBCGPBaseVisualObject* GetVisualObject()
	{
		return GetLegend();
	}

protected:
	CBCGPChartLegendVisualObject*	m_pLegend;
};

#endif // !defined(AFX_BCGPCHARTCTRL_H__8E3DB7C8_BD56_4654_89A6_3F0B67AD7B4B__INCLUDED_)
