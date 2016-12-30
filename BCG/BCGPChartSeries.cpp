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
// BCGPChartSeries.cpp : implementation file
//

#include "stdafx.h"
#include "bcgprores.h"
#include "BCGPChartSeries.h"
#include "BCGPChartVisualObject.h"
#include "BCGPChartLegend.h"
#include "BCGPChartFormula.h"
#include "BCGPMath.h"

#include "BCGPDrawManager.h"
#include "BCGPImageProcessing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CBCGPChartSeries, CBCGPVisualDataObject)
IMPLEMENT_DYNCREATE(CBCGPChartLineSeries, CBCGPChartSeries)
IMPLEMENT_DYNCREATE(CBCGPChartAreaSeries, CBCGPChartSeries)
IMPLEMENT_DYNCREATE(CBCGPChartSurfaceSeries, CBCGPChartSeries)
IMPLEMENT_DYNCREATE(CBCGPChartBarSeries, CBCGPChartSeries)
IMPLEMENT_DYNCREATE(CBCGPChartHistogramSeries, CBCGPChartSeries)
IMPLEMENT_DYNCREATE(CBCGPChartLongSeries, CBCGPChartSeries)
IMPLEMENT_DYNCREATE(CBCGPChartHistoricalLineSeries, CBCGPChartLongSeries)
IMPLEMENT_DYNCREATE(CBCGPChartBubbleSeries, CBCGPChartSeries)
IMPLEMENT_DYNCREATE(CBCGPChartPieSeries, CBCGPChartSeries)
IMPLEMENT_DYNCREATE(CBCGPChartDoughnutSeries, CBCGPChartPieSeries)
IMPLEMENT_DYNCREATE(CBCGPChartPyramidSeries, CBCGPChartSeries)
IMPLEMENT_DYNCREATE(CBCGPChartFunnelSeries, CBCGPChartPyramidSeries)
IMPLEMENT_DYNCREATE(CBCGPBaseChartStockSeries, CBCGPChartSeries)
IMPLEMENT_DYNCREATE(CBCGPChartStockSeries, CBCGPBaseChartStockSeries)
IMPLEMENT_DYNCREATE(CBCGPChartPolarSeries, CBCGPChartLineSeries)
IMPLEMENT_DYNCREATE(CBCGPChartTernarySeries, CBCGPChartLineSeries)

const int CBCGPChartStockSeries::CHART_STOCK_SERIES_HIGH_IDX = 0;
const int CBCGPChartStockSeries::CHART_STOCK_SERIES_LOW_IDX = 1;
const int CBCGPChartStockSeries::CHART_STOCK_SERIES_CLOSE_IDX = 2;

const int CBCGPChartStockSeries::STOCK_ARRAY_OPEN_IDX = 0;
const int CBCGPChartStockSeries::STOCK_ARRAY_HIGH_IDX = 1;
const int CBCGPChartStockSeries::STOCK_ARRAY_LOW_IDX = 2;
const int CBCGPChartStockSeries::STOCK_ARRAY_CLOSE_IDX = 3;

//****************************************************************************************
// CBCGPChartSeries - basic
//****************************************************************************************
CBCGPChartSeries::CBCGPChartSeries()
{
	CommonInit(NULL, BCGPChartDefault, BCGP_CT_DEFAULT);
}

CBCGPChartSeries::CBCGPChartSeries(CBCGPChartVisualObject* pChartCtrl, 
				 BCGPChartCategory chartCategory, 
				 BCGPChartType chartType)
{
	CommonInit(pChartCtrl, chartCategory, chartType);
}
//****************************************************************************************
CBCGPChartSeries::CBCGPChartSeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
				 const CBCGPColor& seriesColor, BCGPChartCategory chartCategory, 
				 BCGPChartType chartType)
{
	CommonInit(pChartCtrl, chartCategory, chartType);
	m_strSeriesName = strSeriesName;

	if (!seriesColor.IsNull())
	{
		SetDefaultSeriesColor(seriesColor);
	}
}
//****************************************************************************************
CBCGPChartSeries::~CBCGPChartSeries()
{
	RemoveAllDataPoints();

	if (m_pChartImpl != NULL)
	{
		delete m_pChartImpl;
		m_pChartImpl = NULL;
	}

	if (m_pChartFormula != NULL)
	{
		delete m_pChartFormula;
	}

	for (int i = 0; i < (int)m_arDataBuffers.GetSize(); i++)
	{
		delete m_arDataBuffers[i];
	}

	if (m_pXAxis != NULL && m_pXAxis->GetIndexedSeries() == this)
	{
		m_pXAxis->SetIndexedSeries(NULL);
	}

	if (m_pYAxis != NULL && m_pYAxis->GetIndexedSeries() == this)
	{
		m_pYAxis->SetIndexedSeries(NULL);
	}

	m_arDataBuffers.RemoveAll();

	m_pChart = NULL;
}
//****************************************************************************************
void CBCGPChartSeries::CommonInit(CBCGPChartVisualObject* pChartCtrl, 
								  BCGPChartCategory chartCategory, 
								  BCGPChartType chartType)
{
	m_bVisible = TRUE;
	m_bIncludeDataPointLabelsToLegend = FALSE;
	m_bIncludeInvisibleSeriesToLegend = FALSE;

	m_rectLegendKey.SetRectEmpty();
	m_rectLegendLabel.SetRectEmpty();
	m_nLegendKeyToLabelDistance = 5;

	m_bGetDataPointLabelForLegend = FALSE;

	m_pChartImpl = FALSE;

	m_nOrderIndex = 0;
	m_nGroupID = 0;
	m_bXComponentSet = FALSE;
	m_bZComponentSet = FALSE;

	m_pXAxis = NULL;
	m_pYAxis = NULL;
	m_pZAxis = NULL;

	m_treatNulls = CBCGPChartSeries::TN_VALUE;

	m_nMinDisplayedXIndex = 0;
	m_nMaxDisplayedXIndex = 0;

	m_nLegendID = -1;

	m_nCurrentIndex = 0;
	m_bHistoryMode = FALSE;
	m_bIndexMode = FALSE;
	m_nHistoryDepth = 0;

	m_nFixedGridSizeX = 32;
	m_nFixedGridSizeY = 32;
	m_nValuesPerIntervalX = 1;
	m_nValuesPerIntervalY = 1;

	m_bUpdateAxesOnNewData = TRUE;
	m_bReverseAdd = FALSE;
	m_nOffsetFromNullZone = 0;

	m_bAutoColorDataPoints = FALSE;
	m_nColorIndex = 0;

	m_pChart = pChartCtrl;
	m_chartCategory = chartCategory;

	m_FillGradienType = CBCGPBrush::BCGP_NO_GRADIENT;

	if (chartType != BCGP_CT_DEFAULT)
	{
		m_chartType = chartType;
	}
	else if (m_pChart != NULL)
	{
		m_chartType = pChartCtrl->GetChartType();
	}

	m_bIgnoreNegativeValues = TRUE;
	m_bIncludeSeriesToLegend = TRUE;
	m_bIncludeSeriesToDataTable = TRUE;
	m_bIncludeInvisibleSeriesToDataTable = FALSE;

	m_dblMinDataPointDistance = DBL_MAX;

	m_pChartFormula = NULL;
	m_bFullStackedMinMaxSet = FALSE;

	m_bIsBackgroundOrder = FALSE;
	m_nLongDataOffset = 0;

	m_dblEmptyValue = 0;

	m_dwUserData = 0;
}
//****************************************************************************************
void CBCGPChartSeries::SetDefaultSeriesColor(const CBCGPColor& seriesColor)
{
	BCGPSeriesColorsPtr colors;
	CBCGPBrush::BCGP_GRADIENT_TYPE gradientType = CBCGPBrush::BCGP_NO_GRADIENT;
	double dblOpacity = 1.;

	if (m_pChart != NULL)
	{
		const CBCGPChartTheme& theme = m_pChart->GetColors();
		theme.GetSeriesColors(colors, 0, GetDefaultFillGradientType());

		if (colors.m_pBrElementFillColor != NULL)
		{
			gradientType = colors.m_pBrElementFillColor->GetGradientType();
			dblOpacity = colors.m_pBrElementFillColor->GetOpacity();
		}
	}

	if (!seriesColor.IsNull())
	{
		CBCGPColor clrBorder = seriesColor;
		clrBorder.MakeDarker(.1);

		CBCGPColor clrGradient = seriesColor;
		clrGradient.MakeLighter(.5);

		m_formatSeries.m_seriesElementFormat.m_brFillColor.SetColors(seriesColor, clrGradient, 
			gradientType, dblOpacity);
		m_formatSeries.m_seriesElementFormat.m_outlineFormat.m_brLineColor.SetColor(clrBorder);

		m_formatSeries.m_markerFormat.m_brFillColor = m_formatSeries.m_seriesElementFormat.m_brFillColor;
		m_formatSeries.m_markerFormat.m_outlineFormat.m_brLineColor.SetColor(clrBorder);

		m_formatSeries.m_dataLabelFormat.m_outlineFormat.m_brLineColor.SetColor(clrBorder);
		m_formatSeries.m_dataLabelFormat.m_brFillColor.SetColor(clrGradient);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetIndexMode(BOOL bSet)
{
	ASSERT_VALID(this);

	m_bIndexMode = bSet;
	if (m_pXAxis != NULL)
	{
		ASSERT_VALID(m_pXAxis);
		if (bSet)
		{
			if (m_pXAxis->GetIndexedSeries() == NULL)
			{
				m_pXAxis->SetIndexedSeries(this);
			}
		}
		else if (m_pXAxis->GetIndexedSeries() == this)
		{
			m_pXAxis->SetIndexedSeries(NULL);
		}
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetVirtualMode(BOOL bSet, BCGPCHART_VSERIES_CALLBACK pCallBack, 
									  LPARAM lParam, double dblMinRange, double dblMaxRange)
{
	ASSERT_VALID(this);

	if (!bSet)
	{
		if (m_pChartFormula != NULL && m_pChartFormula->IsKindOf(RUNTIME_CLASS(CBCGPChartVirtualFormula)))
		{
			delete m_pChartFormula;
			m_pChartFormula = NULL;
		}

		return;
	}

	CBCGPChartVirtualFormula vf(pCallBack, lParam);
	vf.SetTrendRange(dblMinRange, dblMaxRange);

	SetFormula(vf, TRUE);
}
//****************************************************************************************
BOOL CBCGPChartSeries::IsVirtualMode() const
{
	ASSERT_VALID(this);

	return m_pChartFormula != NULL && m_pChartFormula->IsKindOf(RUNTIME_CLASS(CBCGPChartVirtualFormula));
}
//****************************************************************************************
const CBCGPPointsArray& CBCGPChartSeries::GetVirtualSeriesScreenPoints() const
{
	ASSERT_VALID(this);

	CBCGPChartVirtualFormula* pVF = DYNAMIC_DOWNCAST(CBCGPChartVirtualFormula, m_pChartFormula);

	if (pVF == NULL)
	{
		return m_arDummyArray;
	}

	ASSERT_VALID(pVF);

	return pVF->GetScreenPoints();
}
//****************************************************************************************
BOOL CBCGPChartSeries::IsFormulaSeries() const 
{
	return m_pChartFormula != NULL && !IsVirtualMode();
}
//****************************************************************************************
CBCGPChartBaseFormula* CBCGPChartSeries::GetFormula() const 
{
	if (IsVirtualMode())
	{
		return NULL;
	}

	return m_pChartFormula;
}
//****************************************************************************************
void CBCGPChartSeries::SetFormula(const CBCGPChartBaseFormula& formula, BOOL bRedraw)
{
	ASSERT_VALID(this);

	RemoveAllDataPoints();

	if (m_pChartFormula != NULL)
	{
		delete m_pChartFormula;
		m_pChartFormula = NULL;
	}

	m_pChartFormula = (CBCGPChartBaseFormula*) formula.GetRuntimeClass()->CreateObject();

	ASSERT_VALID(m_pChartFormula);

	m_pChartFormula->CopyFrom(formula);
	m_pChartFormula->SetParentSeries(this);

	m_pChartFormula->GeneratePoints();
	
	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, bRedraw);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetTreatNulls(CBCGPChartSeries::TreatNulls tn, BOOL bRecalcMinMax)
{
	if (m_treatNulls != tn)
	{
		m_treatNulls = tn;

		if (bRecalcMinMax)
		{
			RecalcMinMaxValues();
		}

		if (m_pXAxis != NULL)
		{
			m_pXAxis->GenerateScaleBreaks();
		}

		if (m_pYAxis != NULL)
		{
			m_pYAxis->GenerateScaleBreaks();
		}

		if (m_pChart != NULL)
		{
			m_pChart->SetDirty(TRUE, TRUE);
		}
	}
}
//****************************************************************************************
CBCGPPoint CBCGPChartSeries::ScreenPointFromChartData(const CBCGPChartData& data, int nDataPointIndex)
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return CBCGPPoint();
	}

	BOOL bIsEmpty = FALSE;
	return m_pChart->ScreenPointFromChartData(data, nDataPointIndex, bIsEmpty, this);
}
//****************************************************************************************
void CBCGPChartSeries::EnableHistoryMode(BOOL bEnable, int nHistoryDepth, BOOL bReverseOrder, 
										 BOOL bSetDefaultValue, double dblDefaultYValue)
{
	ASSERT_VALID(this);

	if (IsOptimizedLongDataMode())
	{
		TRACE0("History mode can't be enabled for optimized long adat mode.");
		return;
	}

	if (m_pXAxis == NULL)
	{
		ASSERT(FALSE);
		TRACE0("CBCGPChartSeries::EnableHistoryMode. X axis must have been set.");
		return;
	}

	ASSERT_VALID(m_pXAxis);

	if (m_bHistoryMode && bEnable)
	{
		return;
	}

	m_nHistoryDepth = nHistoryDepth;
	m_bHistoryMode = bEnable;
	m_bIndexMode = bEnable;

	if (bEnable)
	{
		m_nCurrentIndex = 0;

		m_bReverseAdd = bReverseOrder;

		int nPrevSize = (int)m_arDataPoints.GetSize();
		ResizeDataPointArray(nHistoryDepth);
		
		if (bSetDefaultValue)
		{
			for (int i = 0; i < nHistoryDepth; i++)
			{
				CBCGPChartDataPoint* pDataPoint = new CBCGPChartDataPoint(dblDefaultYValue);
				m_arDataPoints.SetAt(i, pDataPoint);
			}
		}
		else if (nPrevSize < nHistoryDepth)
		{
			m_nCurrentIndex = nPrevSize;
		}
		else if (m_nCurrentIndex >= nHistoryDepth)
		{
			m_nCurrentIndex = nHistoryDepth;
		}

		m_bAutoColorDataPoints = FALSE;

		m_pXAxis->m_crossType = CBCGPChartAxis::CT_IGNORE;
		m_pXAxis->SetIndexedSeries(this);
		RecalcMinMaxValues();
		m_pXAxis->SetScrollRange(0, m_nHistoryDepth - 1);
	}
}
//****************************************************************************************
void CBCGPChartSeries::ResizeDataPointArray(int nNewSize)
{
	ASSERT_VALID(this);

	m_arDataPoints.SetSize(nNewSize);
}
//****************************************************************************************
void CBCGPChartSeries::ResizeLongDataArray(int nNewSize, BOOL bGrowByCount, CBCGPChartData::ComponentIndex ci)
{
	switch (ci)
	{
	case CBCGPChartData::CI_Y:
		if (bGrowByCount)
		{
			nNewSize += (int)m_arLongDataY.GetSize();
		}
		m_arLongDataY.SetSize(nNewSize);
		break;

	case CBCGPChartData::CI_Y1:
		if (bGrowByCount)
		{
			nNewSize += (int)m_arLongDataY1.GetSize();
		}
		m_arLongDataY1.SetSize(nNewSize);
		break;

	case CBCGPChartData::CI_X:
		if (bGrowByCount)
		{
			nNewSize += (int)m_arLongDataX.GetSize();
		}
		m_arLongDataX.SetSize(nNewSize);
		break;
	}
}
//****************************************************************************************
void CBCGPChartSeries::UpdateAxes()
{
	ASSERT_VALID(this);

	if (m_pXAxis != NULL)
	{
		ASSERT_VALID(m_pXAxis);

		m_pXAxis->SetFixedMajorUnit(GetFixedMajorXUnit());
		m_pXAxis->SetAutoDisplayRange();

		int nDisplayedLines = m_pXAxis->GetNumberOfUnitsToDisplay((UINT)-1);
		int nOffset = min(nDisplayedLines, abs(m_nOffsetFromNullZone)) - 1;

		if (m_bReverseAdd)
		{
			if (m_nCurrentIndex + nOffset < nDisplayedLines)
			{
				m_pXAxis->SetFixedMinimumDisplayValue(0);
			}
			else if (m_nCurrentIndex < m_nHistoryDepth)
			{
				m_pXAxis->SetFixedMinimumDisplayValue(m_nCurrentIndex - nDisplayedLines + nOffset);
			}
			else
			{
				m_pXAxis->SetFixedMinimumDisplayValue(m_nHistoryDepth - nDisplayedLines + nOffset);
			}
		}
		else
		{
			m_pXAxis->SetFixedMaximumDisplayValue(m_nHistoryDepth + nOffset);
		}

		m_pXAxis->SetScrollRange(0, m_nHistoryDepth - 1);
	}

	if (m_pYAxis != NULL)
	{
		if (m_pYAxis->GetMinDisplayedValue() > GetMinValue(CBCGPChartData::CI_Y) ||
			m_pYAxis->GetMaxDisplayedValue() < GetMaxValue(CBCGPChartData::CI_Y))
		{
			m_pYAxis->SetAutoDisplayRange();
		}

		if (m_pYAxis->GetMinDisplayedValue() < GetMinValue(CBCGPChartData::CI_Y) ||
			 m_pYAxis->GetMaxDisplayedValue() > GetMaxValue(CBCGPChartData::CI_Y))
		{
			m_pYAxis->SetScrollRange(min(m_pYAxis->GetMinScrollValue(), m_pYAxis->GetMinDisplayedValue()), 
									 max(m_pYAxis->GetMaxScrollValue(), m_pYAxis->GetMaxDisplayedValue()), FALSE);
		}
	}
}
//****************************************************************************************
int CBCGPChartSeries::GetMinDataPointIndex() const
{
	ASSERT_VALID(this);

	if (m_pXAxis != NULL && m_pXAxis->IsFixedMajorUnit() && m_bIndexMode)
	{
		return (int)max(0, m_pXAxis->GetMinDisplayedValue() - 1);
	}
	
	if (m_pXAxis != NULL && !m_pXAxis->IsComponentXSet()) 
	{	
		return max(0, (int)m_pXAxis->GetMinDisplayedValue() - 2);
	}

	return 0;
}
//****************************************************************************************
int CBCGPChartSeries::GetMaxDataPointIndex() const
{
	ASSERT_VALID(this);

	int nIdx = 0;
	int nDPCount = GetDataPointCount();

	if (m_pXAxis != NULL && m_pXAxis->IsFixedMajorUnit() && m_bIndexMode)
	{
		nIdx = (int)min(nDPCount - 1, m_pXAxis->GetMaxDisplayedValue());
	}
	else if (m_pXAxis != NULL && !m_pXAxis->IsComponentXSet())
	{
		// need 2 extra points to "extend" series shape to invisible area
		// if X axis is scrolled and zoomed
		int nExtraPoints = 2; 

		// for advanced formula series (usually indicators) we need to display only points that are on the screen
		if (m_pChartFormula != NULL && m_pChartFormula->IsKindOf(RUNTIME_CLASS(CBCGPChartAdvancedFormula)))
		{
			nExtraPoints = 0;
		}

		nIdx = min((int)m_pXAxis->GetMaxDisplayedValue() + nExtraPoints, nDPCount - 1);
	}
	else
	{
		nIdx = nDPCount - 1;
	}
	
	return nIdx;
}
//****************************************************************************************
CBCGPBaseChartImpl* CBCGPChartSeries::OnCreateChartImpl(BCGPChartCategory chartCategory)
{
	ASSERT_VALID(this);

	if (m_pChartImpl != NULL)
	{
		delete m_pChartImpl;
	}

	switch(chartCategory)
	{
	case BCGPChartLine:
		m_pChartImpl = new CBCGPLineChartImpl(m_pChart);
		break;
	case BCGPChartColumn:
	case BCGPChartBar:
		m_pChartImpl = new CBCGPBarChartImpl(m_pChart);
		break;
	case BCGPChartColumn3D:
	case BCGPChartBar3D:
		m_pChartImpl = new CBCGPBarChart3DImpl(m_pChart);
		break;
	case BCGPChartHistogram:
		m_pChartImpl = new CBCGPHistogramChartImpl(m_pChart);
		break;
	case BCGPChartLine3D:
		m_pChartImpl = new CBCGPLineChart3DImpl(m_pChart);
		break;
	case BCGPChartArea:
		m_pChartImpl = new CBCGPAreaChartImpl(m_pChart);
		break;
	case BCGPChartArea3D:
		m_pChartImpl = new CBCGPAreaChart3DImpl(m_pChart);
		break;
	case BCGPChartSurface3D:
		m_pChartImpl = new CBCGPSurfaceChart3DImpl(m_pChart);
		break;
	case BCGPChartBubble:
		m_pChartImpl = new CBCGPBubbleChartImpl(m_pChart);
		break;
	case BCGPChartPie:
		m_pChartImpl = new CBCGPPieChartImpl(m_pChart, FALSE);
		break;
	case BCGPChartPie3D:
		m_pChartImpl = new CBCGPPieChartImpl(m_pChart, TRUE);
		break;
	case BCGPChartDoughnut:
		m_pChartImpl = new CBCGPDoughnutChartImpl(m_pChart, FALSE);
		break;
	case BCGPChartDoughnut3D:
		m_pChartImpl = new CBCGPDoughnutChartImpl(m_pChart, TRUE);
		break;
	case BCGPChartTorus3D:
		m_pChartImpl = new CBCGPTorusChartImpl(m_pChart);
		break;
	case BCGPChartPyramid:
		m_pChartImpl = new CBCGPPyramidChartImpl(m_pChart, FALSE);
		break;
	case BCGPChartPyramid3D:
		m_pChartImpl = new CBCGPPyramidChartImpl(m_pChart, TRUE);
		break;
	case BCGPChartFunnel:
		m_pChartImpl = new CBCGPFunnelChartImpl(m_pChart, FALSE);
		break;
	case BCGPChartFunnel3D:
		m_pChartImpl = new CBCGPFunnelChartImpl(m_pChart, TRUE);
		break;
	case BCGPChartLongData:
	case BCGPChartHistoricalLine:
		m_pChartImpl = new CBCGPLongDataChartImpl(m_pChart);
		break;
	case BCGPChartPolar:
		m_pChartImpl = new CBCGPPolarChartImpl(m_pChart);
		break;
	case BCGPChartTernary:
	default:
		m_pChartImpl = new CBCGPLineChartImpl(m_pChart);
		break;
	}

	return m_pChartImpl;
}
//****************************************************************************************
void CBCGPChartSeries::SetChartCtrl(CBCGPChartVisualObject* pChartCtrl)
{
	m_pChart = pChartCtrl;

	if (m_pChart != NULL)
	{
		ASSERT_KINDOF(CBCGPChartVisualObject, m_pChart);
	}
}
//****************************************************************************************
CBCGPBaseChartImpl*	CBCGPChartSeries::GetChartImpl() 
{
	ASSERT_VALID(this);

	if (m_pChartImpl == NULL)
	{
		m_pChartImpl = OnCreateChartImpl(m_chartCategory);
		ASSERT_VALID(m_pChartImpl);
	}

	return m_pChartImpl;
}
//****************************************************************************************
void CBCGPChartSeries::SetChartImpl(CBCGPBaseChartImpl* pImpl)
{
	ASSERT_VALID(this);

	if (pImpl == NULL)
	{
		return;
	}

	ASSERT_VALID(pImpl);
	ASSERT_KINDOF(CBCGPBaseChartImpl, pImpl);

	if (m_pChartImpl != pImpl && m_pChartImpl != NULL)
	{
		delete m_pChartImpl;
	}

	m_pChartImpl = pImpl;
}
//****************************************************************************************
BOOL CBCGPChartSeries::CanBeConvertedToCategory(BCGPChartCategory chartCategory) const
{
	if (IsOptimizedLongDataMode() && 
		(chartCategory != BCGPChartArea && chartCategory != BCGPChartColumn && 
		chartCategory != BCGPChartBar && chartCategory != BCGPChartHistogram && 
		chartCategory != BCGPChartLine))
	{
		return FALSE;
	}

	if (chartCategory == BCGPChartStock || chartCategory == BCGPChartTernary)
	{
		// can't change to stock or ternary category.
		// stock and ternary types must be created explicitly
		return FALSE;
	}

	if (m_chartCategory == BCGPChartLongData || m_chartCategory == BCGPChartPolar || m_chartCategory == BCGPChartTernary)
	{
		// can't change long data, polar and ternary chart to anything else
		return FALSE;
	}

	return TRUE;
}
//****************************************************************************************
CBCGPChartSeries* CBCGPChartSeries::SetChartType(BCGPChartCategory chartCategory, BCGPChartType chartType,
									BOOL bAdjustLayout, BOOL bRedraw)
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return NULL;
	}

	if (IsOptimizedLongDataMode() && 
			(chartCategory != BCGPChartArea && chartCategory != BCGPChartColumn && 
			 chartCategory != BCGPChartBar && chartCategory != BCGPChartHistogram && 
			 chartCategory != BCGPChartLine))
	{
		return NULL;
	}

	if (chartCategory == BCGPChartSurface3D && chartType != BCGP_CT_SIMPLE)
	{
		chartType = BCGP_CT_SIMPLE;
	}

	if (chartCategory == BCGPChartStock || chartCategory == BCGPChartTernary)
	{
		// can't change to stock or ternary category.
		// stock and ternary types must be created explicitly
		return this;
	}

	if (m_chartCategory == BCGPChartLongData || m_chartCategory == BCGPChartPolar || m_chartCategory == BCGPChartTernary)
	{
		// can't change long data, polar and ternary chart to anything else
		return this;
	}

	if ((m_chartCategory == BCGPChartPie && chartCategory == BCGPChartPie3D ||
		m_chartCategory == BCGPChartPie3D && chartCategory == BCGPChartPie) ||
		(m_chartCategory == BCGPChartPyramid && chartCategory == BCGPChartPyramid3D ||
		m_chartCategory == BCGPChartPyramid3D && chartCategory == BCGPChartPyramid) ||
		(m_chartCategory == BCGPChartFunnel && chartCategory == BCGPChartFunnel3D ||
		m_chartCategory == BCGPChartFunnel3D && chartCategory == BCGPChartFunnel))
	{
		// don't need to change category, pie and 3dpie or pyramid and pyramid 3d differ by implementation only
		m_chartCategory = chartCategory;
		OnCreateChartImpl(chartCategory);
		m_pChart->EnableMagnifier(FALSE);

		if (m_pChart != NULL)
		{
			m_pChart->SetDirty(TRUE, bRedraw);
		}

		return this;
	}

	if (m_chartCategory == BCGPChartBar && 
		chartCategory == BCGPChartColumn ||
		m_chartCategory == BCGPChartColumn &&
		chartCategory == BCGPChartBar ||
		m_chartCategory == BCGPChartBar3D && 
		chartCategory == BCGPChartColumn3D ||
		m_chartCategory == BCGPChartColumn3D &&
		chartCategory == BCGPChartBar3D)
	{
		if (m_chartType == BCGP_CT_100STACKED && 
			m_chartType != chartType)
		{
			if (m_pYAxis != NULL)
			{
				ASSERT_VALID(m_pYAxis);
				m_pYAxis->SetAutoDisplayRange();
			}
		}

		m_chartCategory = chartCategory;
		m_chartType = chartType;
		OnCreateChartImpl(chartCategory);

		if (m_pChart != NULL)
		{
			CBCGPChartAxis* pAxis = GetRelatedAxis(CBCGPChartSeries::AI_X);
			if (pAxis != NULL && (pAxis->IsVertical() && (chartCategory != BCGPChartBar && chartCategory != BCGPChartBar3D) ||
				!pAxis->IsVertical() && (chartCategory == BCGPChartBar || chartCategory == BCGPChartBar3D)))
			{
				m_pChart->SwapAxesDirections(FALSE);
			}
			
			m_pChart->SetDirty(bAdjustLayout, bRedraw);
		}

		m_FillGradienType = GetSeriesFillGradientType();
		return this;
	}

	if (chartCategory == BCGPChartBar || m_chartCategory == BCGPChartBar ||
		chartCategory == BCGPChartBar3D || m_chartCategory == BCGPChartBar3D)
	{
		if (m_pChart != NULL)
		{
			CBCGPChartAxis* pAxis = GetRelatedAxis(CBCGPChartSeries::AI_X);
			
			if (pAxis != NULL)
			{
				BOOL bSetVert = chartCategory == BCGPChartBar || chartCategory == BCGPChartBar3D;
				m_pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS)->SetVertical(bSetVert);
				m_pChart->GetChartAxis(BCGP_CHART_X_SECONDARY_AXIS)->SetVertical(bSetVert);
				m_pChart->GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS)->SetVertical(!bSetVert);
				m_pChart->GetChartAxis(BCGP_CHART_Y_SECONDARY_AXIS)->SetVertical(!bSetVert);
			}
		}
	}

	ASSERT_VALID(m_pChart);
	m_pChartImpl = GetChartImpl();

	if (m_chartType != chartType)
	{
		// reset fixed range flag in Y axis, which might be left from stacked series
		if (m_chartType == BCGP_CT_100STACKED)
		{
			if (m_pYAxis != NULL)
			{
				ASSERT_VALID(m_pYAxis);
				m_pYAxis->SetAutoDisplayRange();
			}
		}
	}

	if (chartCategory == BCGPChartArea ||
		chartCategory == BCGPChartHistoricalLine ||
		chartCategory == BCGPChartLongData ||
		chartCategory == BCGPChartStock)
	{
		m_pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS)->SetDisplayDataBetweenTickMarks(FALSE, FALSE);
	}
	else
	{
		m_pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS)->SetDisplayDataBetweenTickMarks(TRUE, FALSE);
	}

	m_chartType = chartType;

	BOOL bIs3D = m_pChart->IsChart3D();
	
	if (chartCategory != m_chartCategory)
	{
		m_chartCategory = chartCategory;
		int nIdx = m_pChart->FindSeriesIndex(this);

		CBCGPChartSeries* pNewSeries = m_pChart->OnCreateChartSeries(chartCategory, chartType);
		if (pNewSeries != NULL)
		{
			ASSERT_VALID(pNewSeries);

			OnBeforeChangeType();

			pNewSeries->CopyFrom(*this);

			if (bIs3D)
			{
				pNewSeries->SetRelatedAxis(m_pChart->GetChartAxis(BCGP_CHART_Z_PRIMARY_AXIS), CBCGPChartSeries::AI_Z);
			}
			else
			{
				pNewSeries->SetRelatedAxis(NULL, CBCGPChartSeries::AI_Z);
			}

			pNewSeries->OnCreateChartImpl(chartCategory);
			pNewSeries->m_chartType = chartType;
			if (!pNewSeries->IsOptimizedLongDataMode())
			{
				pNewSeries->RecalcMinMaxValues();
			}
			

			if (nIdx != -1)
			{
				m_pChart->OnAddSeries(pNewSeries, nIdx);
			}

			if (m_pYAxis != NULL && m_pYAxis->GetIndexedSeries() == this)
			{
				m_pYAxis->SetIndexedSeries(pNewSeries);
				pNewSeries->SetIndexMode(TRUE);
			}

			if (m_pXAxis != NULL && m_pXAxis->GetIndexedSeries() == this)
			{
				m_pXAxis->SetIndexedSeries(pNewSeries);
				pNewSeries->SetIndexMode(TRUE);
			}

			delete this;
			return pNewSeries;
		}
		else
		{
			return this;
		}
	}	

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, bRedraw);
	}

	return this;
}
//****************************************************************************************
void CBCGPChartSeries::EnableAutoColorDataPoints(BOOL bEnable, BOOL bRedraw)
{
	ASSERT_VALID(this);

	m_bAutoColorDataPoints = bEnable;
	m_bIncludeDataPointLabelsToLegend = bEnable;
	m_formatSeries.m_legendLabelContent = GetDefaultLegendContent();

	if (m_pChart != NULL)
	{
		ASSERT_VALID(this);
		m_pChart->UpdateSeriesColorIndexes(bRedraw);
	}
}
//****************************************************************************************
void CBCGPChartSeries::ShowOnPrimaryAxis(BOOL bPrimary)
{
	ASSERT_VALID(this);
	if (m_pChart == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pChart);

	if (bPrimary || m_pChart->IsChart3D())
	{
		SetRelatedAxes(m_pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS), 
						m_pChart->GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS));

		if (m_pChart->IsChart3D())
		{
			SetRelatedAxis(m_pChart->GetChartAxis(BCGP_CHART_Z_PRIMARY_AXIS), CBCGPChartSeries::AI_Z);
		}
	}
	else
	{
		SetRelatedAxes(m_pChart->GetChartAxis(BCGP_CHART_X_SECONDARY_AXIS), 
			m_pChart->GetChartAxis(BCGP_CHART_Y_SECONDARY_AXIS));

		if (m_pChart->IsChart3D())
		{
			SetRelatedAxis(m_pChart->GetChartAxis(BCGP_CHART_Z_SECONDARY_AXIS), CBCGPChartSeries::AI_Z);
		}
	}
}
//****************************************************************************************
CBCGPRect CBCGPChartSeries::GetAxesBoundingRect()
{
	ASSERT_VALID(this);

	if (m_pXAxis == NULL || m_pYAxis == NULL)
	{
		return CBCGPRect();
	}

	ASSERT_VALID(m_pXAxis);
	ASSERT_VALID(m_pYAxis);

	if (m_pYAxis->m_nAxisID >= BCGP_CHART_FIRST_CUSTOM_ID)
	{
		return m_pYAxis->GetBoundingRect();
	}

	return m_pXAxis->GetBoundingRect();
}
//****************************************************************************************
void CBCGPChartSeries::SetRelatedAxes(CBCGPChartAxis* pXAxis, CBCGPChartAxis* pYAxis, CBCGPChartAxis* pZAxis)
{
	ASSERT_VALID(this);

	if (pXAxis != NULL)
	{
		ASSERT_VALID(pXAxis);
		ASSERT_KINDOF(CBCGPChartAxis, pXAxis);
	}

	if (pYAxis != NULL)
	{
		ASSERT_VALID(pYAxis);
		ASSERT_KINDOF(CBCGPChartAxis, pYAxis);
	}

	if (pZAxis != NULL)
	{
		ASSERT_VALID(pZAxis);
		ASSERT_KINDOF(CBCGPChartAxis, pZAxis);
	}

	SetRelatedAxis(pXAxis, CBCGPChartSeries::AI_X);
	SetRelatedAxis(pYAxis, CBCGPChartSeries::AI_Y);
	SetRelatedAxis(pZAxis, CBCGPChartSeries::AI_Z);
}
//****************************************************************************************
void CBCGPChartSeries::SetRelatedAxis(CBCGPChartAxis* pAxis, CBCGPChartSeries::AxisIndex axisIndex)
{
	ASSERT_VALID(this);
	
	if (pAxis != NULL)
	{
		ASSERT_VALID(pAxis);
		ASSERT_KINDOF(CBCGPChartAxis, pAxis);
	}

	switch(axisIndex)
	{
	case CBCGPChartSeries::AI_X:
		m_pXAxis = pAxis;
		UpdateAxisCategories();
		if (m_chartCategory == BCGPChartArea ||
			m_chartCategory == BCGPChartHistoricalLine ||
			m_chartCategory == BCGPChartLongData ||
			m_chartCategory == BCGPChartStock)
		{
			m_pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS)->SetDisplayDataBetweenTickMarks(FALSE);
		}
		else
		{
			m_pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS)->SetDisplayDataBetweenTickMarks(TRUE);
		}

		if (m_bIndexMode && m_pXAxis != NULL && m_pXAxis->GetIndexedSeries() == NULL)
		{
			m_pXAxis->SetIndexedSeries(this);
		}

		if (IsOptimizedLongDataMode())
		{
			m_pXAxis->SetFixedIntervalWidth(m_nFixedGridSizeX, m_nValuesPerIntervalX);
		}

		break;
	case CBCGPChartSeries::AI_Y:
		m_pYAxis = pAxis;
		if (IsOptimizedLongDataMode())
		{
			m_pYAxis->EnableZoom(FALSE);
			m_pYAxis->EnableScroll(FALSE);
		}
		break;
	case CBCGPChartSeries::AI_Z:
		m_pZAxis = pAxis;
		break;
	}

	if (pAxis != NULL && pAxis->IsScaleBreakEnabled())
	{
		pAxis->GenerateScaleBreaks();
	}
}
//****************************************************************************************
void CBCGPChartSeries::ReplaceAxis(CBCGPChartAxis* pAxisOld, CBCGPChartAxis* pAxisNew)
{
	if (m_pXAxis == pAxisOld)
	{
		SetRelatedAxis(pAxisNew, CBCGPChartSeries::AI_X);
	}
	else if (m_pYAxis == pAxisOld)
	{
		SetRelatedAxis(pAxisNew, CBCGPChartSeries::AI_Y);
	}
	else if (m_pZAxis == pAxisOld)
	{
		SetRelatedAxis(pAxisNew, CBCGPChartSeries::AI_Z);
	}

}
//****************************************************************************************
void CBCGPChartSeries::UpdateAxisCategories()
{
	ASSERT_VALID(this);

	CBCGPChartAxisX* pXAxis = DYNAMIC_DOWNCAST(CBCGPChartAxisX, m_pXAxis);

	if (pXAxis == NULL || pXAxis->m_bFormatAsDate || GetDataPointCount() > 500)
	{
		return;
	}

	ASSERT_VALID(pXAxis);

	for (int i = 0; i < GetDataPointCount(); i++)
	{
		CString strCategory = GetDataPointCategoryName(i);
		if (!strCategory.IsEmpty())
		{
			pXAxis->m_arCategories.SetAtGrow(i, strCategory);
		}
	}
}
//****************************************************************************************
CBCGPChartAxis* CBCGPChartSeries::GetRelatedAxis(CBCGPChartSeries::AxisIndex axisIndex) const
{
	ASSERT_VALID(this);

	switch(axisIndex)
	{
	case CBCGPChartSeries::AI_X:
		return m_pXAxis;

	case CBCGPChartSeries::AI_Y:
		return m_pYAxis;

	case CBCGPChartSeries::AI_Z:
		return m_pZAxis;

	}

	return NULL;
}
//****************************************************************************************
CBCGPChartSeries::AxisIndex CBCGPChartSeries::GetRelatedAxisIndex(CBCGPChartAxis* pAxis)
{
	if (m_pXAxis == pAxis)
	{
		return CBCGPChartSeries::AI_X;
	}
	else if (m_pYAxis == pAxis)
	{
		return CBCGPChartSeries::AI_Y;
	}
	else if (m_pZAxis == pAxis)
	{
		return CBCGPChartSeries::AI_Z;
	}

	return CBCGPChartSeries::AI_UNKNOWN;
}
//****************************************************************************************
BOOL CBCGPChartSeries::IsShownOnCustomOrResizedAxis() const
{
	return m_pXAxis != NULL && m_pXAxis->IsCustomOrResized() ||
			m_pYAxis != NULL && m_pYAxis->IsCustomOrResized() ||
			m_pZAxis != NULL && m_pZAxis->IsCustomOrResized();
}
//****************************************************************************************
void CBCGPChartSeries::SetGroupID(int nGroupID, BOOL bRecalcMinMaxValues)
{
	if (nGroupID < 0)
	{
		ASSERT(FALSE);
		TRACE0("A Group ID can't be negative.");
		return;
	}

	m_nGroupID = nGroupID;

	if (bRecalcMinMaxValues)
	{
		m_pChart->RecalcMinMaxValues();
	}
}
//****************************************************************************************
void CBCGPChartSeries::ClearMinMaxValues()
{
	ASSERT_VALID(this);

	m_minValues.SetEmpty(TRUE);
	m_maxValues.SetEmpty(TRUE);
	m_nMinDisplayedXIndex = GetDataPointCount();
	m_nMaxDisplayedXIndex = 0;

	m_bXComponentSet = FALSE;
}
//****************************************************************************************
void CBCGPChartSeries::SetMinValue(double dblValue, CBCGPChartData::ComponentIndex ci)
{
	ASSERT_VALID(this);

	m_minValues.SetValue(dblValue, ci);
}
//****************************************************************************************
void CBCGPChartSeries::SetMaxValue(double dblValue, CBCGPChartData::ComponentIndex ci)
{
	ASSERT_VALID(this);

	m_maxValues.SetValue(dblValue, ci);
}
//****************************************************************************************
CBCGPChartValue CBCGPChartSeries::GetMinValue(CBCGPChartData::ComponentIndex ci)
{
	ASSERT_VALID(this);

	return m_minValues.GetValue(ci);
}
//****************************************************************************************
CBCGPChartValue CBCGPChartSeries::GetMaxValue(CBCGPChartData::ComponentIndex ci)
{
	ASSERT_VALID(this);

	return m_maxValues.GetValue(ci);
}
//****************************************************************************************
void CBCGPChartSeries::SetMinMaxValuesSimple(double dblValue, CBCGPChartData::ComponentIndex ci, int nDataPointIndex)
{
	ASSERT_VALID(this);

	CBCGPChartValue minValue = GetMinValue(ci);
	CBCGPChartValue maxValue = GetMaxValue(ci);

	if (m_bIndexMode && ci == CBCGPChartData::CI_X)
	{
		dblValue = nDataPointIndex;
	}

	minValue.IsEmpty() ? SetMinValue(dblValue, ci) : SetMinValue(min (minValue, dblValue), ci);
	maxValue.IsEmpty() ? SetMaxValue(dblValue, ci) : SetMaxValue(max (maxValue, dblValue), ci);

	if (ci == CBCGPChartData::CI_X)
	{
		m_bXComponentSet = TRUE;
	}

	if (ci == CBCGPChartData::CI_Z)
	{
		m_bZComponentSet = TRUE;
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetMinMaxValuesRange(double dblValue, CBCGPChartData::ComponentIndex ci, int nDataPointIndex)
{
	ASSERT_VALID(this);

	const CBCGPChartDataPoint* pDataPoint = GetDataPointAt(nDataPointIndex);

	if (ci == CBCGPChartData::CI_Y || nDataPointIndex < 0 || nDataPointIndex >= GetDataPointCount())
	{
		CBCGPChartValue chartValueY1 = pDataPoint->GetComponentValue(CBCGPChartData::CI_Y1);
		CBCGPChartValue chartValueX = pDataPoint->GetComponentValue(CBCGPChartData::CI_X);

		SetMinMaxValuesSimple(dblValue, ci, nDataPointIndex);
		return;
	}

	CBCGPChartValue chartValueY = pDataPoint->GetComponentValue(CBCGPChartData::CI_Y);

	if (ci == CBCGPChartData::CI_X)
	{
		CBCGPChartValue chartValueY1 = pDataPoint->GetComponentValue(CBCGPChartData::CI_Y1);

		if (chartValueY1.IsEmpty())
		{
			SetMinMaxValuesSimple(dblValue + chartValueY.GetValue(), CBCGPChartData::CI_Y, nDataPointIndex);
			m_bXComponentSet = FALSE;
		}
		else
		{
			SetMinMaxValuesSimple(dblValue, ci, nDataPointIndex);
		}
		return;
	}

	if (ci == CBCGPChartData::CI_Y1)
	{
		SetMinMaxValuesSimple(dblValue + chartValueY.GetValue(), CBCGPChartData::CI_Y, nDataPointIndex);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetMinMaxValuesStacked(double dblValue, CBCGPChartData::ComponentIndex ci, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pChart);

	if (m_pYAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pYAxis);

	if (ci != CBCGPChartData::CI_Y || 
		nDataPointIndex < 0 || nDataPointIndex >= GetDataPointCount() || m_pChart == NULL)
	{
		SetMinMaxValuesSimple(dblValue, ci, nDataPointIndex);
		return;
	}

	double dblSum = 0;
	for (int i = 0; i < m_pChart->GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = DYNAMIC_DOWNCAST(CBCGPChartSeries, m_pChart->GetSeries(i, FALSE));
		if (pSeries == NULL || !pSeries->IsStakedSeries() || !pSeries->m_bVisible || m_nGroupID != pSeries->m_nGroupID || 
			!pSeries->IsShownOnAxis(m_pYAxis))
		{
			continue;
		}

		CBCGPChartValue val = pSeries->GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y);

		if (!val.IsEmpty())
		{
			if (GetChartCategory() == BCGPChartArea3D && val.GetValue() < 0.)
			{
				val.SetValue(fabs(val.GetValue()));
			}

			dblSum += val;
		}
	}
	
	SetMinMaxValuesSimple(dblValue, CBCGPChartData::CI_Y, nDataPointIndex);
	SetMinMaxValuesSimple(dblSum, CBCGPChartData::CI_Y, nDataPointIndex);
}
//****************************************************************************************
void CBCGPChartSeries::SetMinMaxValues100Stacked(double dblValue, CBCGPChartData::ComponentIndex ci, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pChart);

	if (m_pYAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pYAxis);

	if (ci != CBCGPChartData::CI_Y || 
		nDataPointIndex < 0 || nDataPointIndex >= GetDataPointCount() || m_pChart == NULL)
	{
		SetMinMaxValuesSimple(dblValue, ci, nDataPointIndex);
		return;
	}

	BOOL bIsBarChart = GetChartImpl()->IsKindOf(RUNTIME_CLASS(CBCGPBarChartImpl));

	double dblStackedSum = 0;
	double dblTotalSum = 0;
	double dblPositiveStackedSum = 0;
	double dblNegativeStackedSum = 0;

	CalcFullStackedSums(nDataPointIndex, dblStackedSum, dblPositiveStackedSum, dblNegativeStackedSum, dblTotalSum);

	double dblCalculatedVal = 0;
	if (fabs(dblTotalSum) > 0.)
	{
		if (bIsBarChart)
		{
			dblCalculatedVal = dblValue < 0. ? (dblNegativeStackedSum + dblValue) * 100. / fabs(dblTotalSum) :
				(dblPositiveStackedSum + dblValue) * 100. / dblTotalSum;
		}
		else
		{
			if (GetChartCategory() == BCGPChartArea3D && dblValue < 0.)
			{
				dblValue = fabs(dblValue);
			}

			dblCalculatedVal = (dblStackedSum + dblValue) * 100. / dblTotalSum;
		}
	}


	SetMinMaxValuesSimple(0., CBCGPChartData::CI_Y, nDataPointIndex);
	SetMinMaxValuesSimple(dblCalculatedVal, CBCGPChartData::CI_Y, nDataPointIndex);
}
//****************************************************************************************
void CBCGPChartSeries::SetMinMaxValues(const CBCGPChartValue& val, CBCGPChartData::ComponentIndex ci, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pChart);

	UNREFERENCED_PARAMETER(nDataPointIndex);

	if (_isnan(val.GetValue()) || !_finite(val.GetValue()))
	{
		return;
	}

	CBCGPBaseChartImpl* pImpl = GetChartImpl();
	ASSERT_VALID(pImpl);

	if (pImpl == NULL)
	{
		return;
	}

	if (val.IsEmpty() && m_treatNulls != CBCGPChartSeries::TN_VALUE)
	{
		return;
	}

	switch(m_chartType)
	{
	case BCGP_CT_SIMPLE:
		SetMinMaxValuesSimple(val, ci, nDataPointIndex);
		break;
	case BCGP_CT_RANGE:
		SetMinMaxValuesRange(val, ci, nDataPointIndex);
		break;
	case BCGP_CT_STACKED:
		SetMinMaxValuesStacked(val, ci, nDataPointIndex);
		break;
	case BCGP_CT_100STACKED:
		SetMinMaxValues100Stacked(val, ci, nDataPointIndex);
		break;
	}
}
//****************************************************************************************
void CBCGPChartSeries::OnCalcScreenPointsSimple(CBCGPGraphicsManager* /*pGM*/, const CBCGPRect& /*rectDiagramArea*/)
{
	ASSERT_VALID(this);

	for (int i = GetMinDataPointIndex(); i <= GetMaxDataPointIndex(); i++)
	{
		CBCGPChartDataPoint* pDp = (CBCGPChartDataPoint*)GetDataPointAt(i);
		if (pDp != NULL)
		{
			RemoveAllDataPointScreenPoints(i);

			BOOL bIsEmpty = FALSE;
			CBCGPPoint point = m_pChart->ScreenPointFromChartData(pDp->GetData(), i, bIsEmpty, this);

			if (!bIsEmpty)
			{
				SetDataPointScreenPoint(i, 0, point);
			}
		}
	}
}
//****************************************************************************************
void CBCGPChartSeries::OnCalcScreenPointsRange(CBCGPGraphicsManager* /*pGM*/, const CBCGPRect& /*rectDiagramArea*/)
{
	ASSERT_VALID(this);

	for (int i = GetMinDataPointIndex(); i <= GetMaxDataPointIndex(); i++)
	{
		CBCGPChartDataPoint* pDp = (CBCGPChartDataPoint*)GetDataPointAt(i);
		RemoveAllDataPointScreenPoints(i);

		if (pDp == NULL || pDp->IsEmpty())
		{
			continue;
		}

		CBCGPChartValue valY = GetDataPointValue(i, CBCGPChartData::CI_Y);
		CBCGPChartValue valX = GetDataPointValue(i, CBCGPChartData::CI_X);
		CBCGPChartValue valY1 = GetDataPointValue(i, CBCGPChartData::CI_Y1);

		if (valY1.IsEmpty())
		{
			valY1 = valX;
		}
		else if (m_chartCategory == BCGPChartArea3D && valY1.GetValue() < 0.)
		{
			valY1.SetValue(fabs(valY1.GetValue()));
		}

		valY1.SetValue(valY1.GetValue() + valY.GetValue());

		BOOL bIsEmpty = FALSE;

		// calculate top range bound
		if (!valY1.IsEmpty())
		{
			CBCGPChartData dataTop(valX, valY1);
			CBCGPPoint pointTop = m_pChart->ScreenPointFromChartData(dataTop, i, bIsEmpty, this);

			SetDataPointScreenPoint(i, 1, pointTop);
		}

		// calculate bottom point
		CBCGPChartData dataBottom(valX, valY);
		CBCGPPoint pointBottom = m_pChart->ScreenPointFromChartData(dataBottom, i, bIsEmpty, this);

		SetDataPointScreenPoint(i, 0, pointBottom);
	}
}
//****************************************************************************************
void CBCGPChartSeries::OnCalcScreenPointsStacked(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagramArea*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pChart);

	UNREFERENCED_PARAMETER(pGM);

	BOOL bIsBarChart = GetChartImpl()->IsKindOf(RUNTIME_CLASS(CBCGPBarChartImpl));
	BOOL bIsArea3D = GetChartCategory() == BCGPChartArea3D;

	// loop over all data points in the series and calculate sum for all data points at index i for all series (100% stacked)
	// or up to current series (for staked)
	for (int i = GetMinDataPointIndex(); i <= GetMaxDataPointIndex(); i++)
	{
		CBCGPChartDataPoint* pDp = (CBCGPChartDataPoint*)GetDataPointAt(i);
		RemoveAllDataPointScreenPoints(i);

		if (pDp == NULL || 
			pDp->GetComponentValue().IsEmpty() && m_treatNulls != CBCGPChartSeries::TN_VALUE)
		{
			continue;
		}

		double dblStackedSum = 0;
		double dblPositiveStackedSum = 0;
		double dblNegativeStackedSum = 0;
		double dblPositiveTotalSum = 0;
		double dblNegativeTotalSum = 0;
		BOOL bThisSeriesFound = FALSE;

		for (int j = 0; j < m_pChart->GetSeriesCount(); j++)
		{
			CBCGPChartSeries* pSeries = DYNAMIC_DOWNCAST(CBCGPChartSeries, m_pChart->GetSeries(j, FALSE));
			if (pSeries == NULL || !pSeries->IsStakedSeries() || !pSeries->m_bVisible || 
				m_nGroupID != pSeries->m_nGroupID || !pSeries->IsShownOnAxis(m_pYAxis))
			{
				continue;
			}

			if (pSeries == this)
			{
				if (m_chartType == BCGP_CT_STACKED)
				{
					break;
				}
				else
				{
					bThisSeriesFound = TRUE;
				}
			}


			CBCGPChartValue val = pSeries->GetDataPointValue(i, CBCGPChartData::CI_Y);
			if (!val.IsEmpty())
			{
				if (bIsArea3D && val < 0.)
				{
					val.SetValue(fabs(val));
				}

				if (!bThisSeriesFound)
				{
					dblStackedSum += val;
					if (val >= 0)
					{
						dblPositiveStackedSum += val;
					}
					else
					{
						dblNegativeStackedSum += val;
					}
				}

				dblPositiveTotalSum += val;
				dblNegativeTotalSum += val;
			}
		}

		// obtain data from the current data point to pass it later to ScreenPointFromChartData
		// we'll preserve X component to receive correct X coordinate, while the Y coordinate
		// will be set according to STACKED rules (100% or simple)
		CBCGPChartData chartData = pDp->GetData();
		double currentVal = pDp->GetComponentValue(CBCGPChartData::CI_Y);

		if (bIsArea3D && currentVal < 0.)
		{
			currentVal = fabs(currentVal);
		}

		if (bIsBarChart)
		{
			if (currentVal < 0)
			{
				chartData.SetValue(dblNegativeStackedSum, CBCGPChartData::CI_Y);
			}
			else
			{
				chartData.SetValue(dblPositiveStackedSum, CBCGPChartData::CI_Y);
			}
		}
		else
		{
			chartData.SetValue(dblStackedSum, CBCGPChartData::CI_Y);
		}
		
		BOOL bIsEmpty = FALSE;
		CBCGPPoint point = m_pChart->ScreenPointFromChartData(chartData, i, bIsEmpty, this);

		// set screen coordinate to point at position 1. It won't affect line charts, but area and bar charts
		// will take this point for the top bound
		SetDataPointScreenPoint(i, 1, point);

		if (bIsBarChart)
		{
			if (currentVal < 0)
			{
				chartData.SetValue(dblNegativeStackedSum + currentVal, CBCGPChartData::CI_Y);
			}
			else
			{
				chartData.SetValue(dblPositiveStackedSum + currentVal, CBCGPChartData::CI_Y);
			}
		}
		else
		{
			chartData.SetValue(dblStackedSum + currentVal, CBCGPChartData::CI_Y);
		}
		
		point = m_pChart->ScreenPointFromChartData(chartData, i, bIsEmpty, this);

		SetDataPointScreenPoint(i, 0, point);
	}
}
//****************************************************************************************
void CBCGPChartSeries::CalcFullStackedSums(int nDataPointIndex, double& dblStackedSum, double& dblPositiveStackedSum, 
										   double& dblNegativeStackedSum, double& dblTotalSum)
{
	ASSERT_VALID(this);

	BOOL bThisSeriesFound = FALSE;

	for (int j = 0; j < m_pChart->GetSeriesCount(); j++)
	{
		CBCGPChartSeries* pSeries = DYNAMIC_DOWNCAST(CBCGPChartSeries, m_pChart->GetSeries(j, FALSE));
		if (pSeries == NULL || !pSeries->IsStakedSeries() || !pSeries->m_bVisible || m_nGroupID != pSeries->m_nGroupID || 
			!pSeries->IsShownOnAxis(m_pYAxis))
		{
			continue;
		}

		if (pSeries == this)
		{
			bThisSeriesFound = TRUE;
		}

		CBCGPChartValue val = pSeries->GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y);

		if (!val.IsEmpty())
		{
			if (pSeries->GetChartCategory() == BCGPChartArea3D && val.GetValue() < 0.)
			{
				val.SetValue(fabs(val));
			}

			if (!bThisSeriesFound)
			{
				dblStackedSum += val;
				if (val >= 0)
				{
					dblPositiveStackedSum += val;
				}
				else
				{
					dblNegativeStackedSum += val;
				}
			}

			dblTotalSum += fabs(val);
		}
	}
}
//****************************************************************************************
void CBCGPChartSeries::OnCalcScreenPoints100Stacked(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagramArea*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pChart);

	UNREFERENCED_PARAMETER(pGM);

	BOOL bIsBarChart = GetChartImpl()->IsKindOf(RUNTIME_CLASS(CBCGPBarChartImpl));
	BOOL bIsLineChart = GetChartImpl()->IsKindOf(RUNTIME_CLASS(CBCGPLineChartImpl));
	BOOL bIsArea3D = GetChartCategory() == BCGPChartArea3D;

	// loop over all data points in the series and calculate sum for all data points at index i for all series (100% stacked)
	// or up to current series (for staked)
	for (int i = 0; i < GetDataPointCount(); i++)
	{
		CBCGPChartDataPoint* pDp = (CBCGPChartDataPoint*)GetDataPointAt(i);
		RemoveAllDataPointScreenPoints(i);

		if (pDp == NULL || 
			pDp->GetComponentValue().IsEmpty() && m_treatNulls != CBCGPChartSeries::TN_VALUE)
		{
			continue;
		}

		double dblStackedSum = 0;
		double dblTotalSum = 0;
		double dblPositiveStackedSum = 0;
		double dblNegativeStackedSum = 0;

		CalcFullStackedSums(i, dblStackedSum, dblPositiveStackedSum, dblNegativeStackedSum, dblTotalSum);

		// obtain data from the current data point to pass it later to ScreenPointFromChartData
		// we'll preserve X component to receive correct X coordinate, while the Y coordinate
		// will be set according to STACKED rules (100% or simple)
		CBCGPChartData chartData = pDp->GetData();
		double currentVal = pDp->GetComponentValue(CBCGPChartData::CI_Y);

		if (bIsArea3D && currentVal < 0.)
		{
			currentVal = fabs(currentVal);
		}

		double dblCalculatedVal = 0;
		
		if (dblTotalSum > 0)
		{
			if (bIsBarChart)
			{
				dblCalculatedVal = currentVal < 0. ? dblNegativeStackedSum * 100. / fabs(dblTotalSum) :
														dblPositiveStackedSum * 100. / dblTotalSum;
			}
			else
			{
				dblCalculatedVal = bIsLineChart ? (dblStackedSum + currentVal) * 100. / dblTotalSum :
													dblStackedSum * 100. / dblTotalSum;
			}
		}

		chartData.SetValue(dblCalculatedVal, CBCGPChartData::CI_Y);

		BOOL bIsEmpty = FALSE;
		CBCGPPoint point = m_pChart->ScreenPointFromChartData(chartData, i, bIsEmpty, this);

		// set screen coordinate to point at position 1. It won't affect line charts, but area and bar charts
		// will take this point for the top bound
		SetDataPointScreenPoint(i, 1, point);

		dblCalculatedVal = 0;
		if (fabs(dblTotalSum) > 0.)
		{
			if (bIsBarChart)
			{
				dblCalculatedVal = currentVal < 0. ? (dblNegativeStackedSum + currentVal) * 100. / fabs(dblTotalSum) :
												(dblPositiveStackedSum + currentVal) * 100. / dblTotalSum;
			}
			else
			{
				dblCalculatedVal = (dblStackedSum + currentVal) * 100. / dblTotalSum;
			}
		}

		chartData.SetValue(dblCalculatedVal, CBCGPChartData::CI_Y);
		point = m_pChart->ScreenPointFromChartData(chartData, i, bIsEmpty, this);

		SetDataPointScreenPoint(i, 0, point);
	}
}
//****************************************************************************************
void CBCGPChartSeries::OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea)
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return;
	}

	if (m_pChartFormula != NULL && m_pChartFormula->IsNonDiscreteCurve())
	{
		m_pChartFormula->GeneratePoints();
		return;
	}

	switch(m_chartType)
	{
	case BCGP_CT_SIMPLE:
		OnCalcScreenPointsSimple(pGM, rectDiagramArea);
		break;
	case BCGP_CT_RANGE:
		OnCalcScreenPointsRange(pGM, rectDiagramArea);
		break;
	case BCGP_CT_STACKED:
		OnCalcScreenPointsStacked(pGM, rectDiagramArea);
		break;
	case BCGP_CT_100STACKED:
		OnCalcScreenPoints100Stacked(pGM, rectDiagramArea);
		break;
	}
}
//****************************************************************************************
int CBCGPChartSeries::GetDataPointCount() const 
{
	if (IsVirtualMode())
	{
		const CBCGPPointsArray& ar = GetVirtualSeriesScreenPoints();
		return (int)ar.GetSize();
	}

	if (IsOptimizedLongDataMode())
	{
		return (int)m_arLongDataY.GetSize();
	}

	return (int)m_arDataPoints.GetSize();
}
//***************************************************************************************s*
const CBCGPChartDataPoint* CBCGPChartSeries::GetDataPointAt(int nIndex) const
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= GetDataPointCount() || IsVirtualMode())
	{
		return NULL;
	}

	int nLongDataSize = (int)m_arLongDataY.GetSize();
	if (nLongDataSize > 0)
	{
		if (nIndex >= nLongDataSize)
		{
			return NULL;
		}

		CBCGPChartDataPoint* pDP = (CBCGPChartDataPoint*)&m_virtualPoint;

		if (m_arLongDataY[nIndex] == GetLongDataEmptyValue() && GetTreatNulls() != CBCGPChartSeries::TN_VALUE)
		{
			pDP->SetComponentValue(CBCGPChartValue(), CBCGPChartData::CI_Y);
		}
		else
		{
			pDP->SetComponentValue(m_arLongDataY[nIndex], CBCGPChartData::CI_Y);
		}

		if (m_arLongDataY1.GetSize() > 0)
		{
			pDP->SetComponentValue(m_arLongDataY1[nIndex], CBCGPChartData::CI_Y1);
		}
		
		if (m_arLongDataX.GetSize() > 0)
		{
			pDP->SetComponentValue(m_arLongDataX[nIndex], CBCGPChartData::CI_X);
		}

		return &m_virtualPoint;
	}

	return m_arDataPoints[nIndex];
}
//****************************************************************************************
int CBCGPChartSeries::FindDataPointIndex(CBCGPChartDataPoint* pDataPoint)
{
	ASSERT_VALID(this);

	if (IsVirtualMode())
	{
		return -1;
	}

	for (int i = 0; i < GetDataPointCount(); i++)
	{
		const CBCGPChartDataPoint* pDP = GetDataPointAt(i);

		if (pDP == pDataPoint)
		{
			return i;
		}
	}

	return -1;
}
//****************************************************************************************
void CBCGPChartSeries::RemoveDataPoints(int nStartIdx, int nCount, BOOL bFreeExtra)
{
	ASSERT_VALID(this);

	if (IsVirtualMode())
	{
		return;
	}

	if (nStartIdx < 0 || nCount < 0 || nStartIdx + nCount > GetDataPointCount())
	{
		ASSERT(FALSE);
		return;
	}

	for (int i = nStartIdx; i < nStartIdx + nCount; i++)
	{
		CBCGPChartDataPoint* pDP = m_arDataPoints[i];
		if (pDP != NULL)
		{
			delete pDP;
		}
	}

	m_arDataPoints.RemoveAt(nStartIdx, nCount);

	if (bFreeExtra)
	{
		m_arDataPoints.FreeExtra();
	}
}
//****************************************************************************************
void CBCGPChartSeries::RemoveAllDataPoints()
{
	ASSERT_VALID(this);

	if (IsVirtualMode())
	{
		return;
	}

	for (int i = 0; i < m_arDataPoints.GetSize(); i++)
	{
		CBCGPChartDataPoint* pDP = m_arDataPoints[i];
		if (pDP != NULL)
		{
			delete pDP;
		}
	}

	m_arDataPoints.RemoveAll();
	m_arLongDataX.RemoveAll();
	m_arLongDataY.RemoveAll();
	m_arLongDataY1.RemoveAll();

	ClearMinMaxValues();
}
//****************************************************************************************
void CBCGPChartSeries::MoveDataPoints(int nFromIdx, int nToIdx, int nCount)
{
	ASSERT_VALID(this);

	if (IsVirtualMode())
	{
		return;
	}

	if (nFromIdx < 0 || nFromIdx >= GetDataPointCount() || nToIdx < 0 || IsVirtualMode())
	{
		ASSERT(FALSE);
		return;
	}

	int n = 0;
	int nDPCount = GetDataPointCount();

	for (int i = nFromIdx; i < nDPCount && n < nCount; i++, n++)
	{
		CBCGPChartDataPoint* pDP = m_arDataPoints[i];
		if (nToIdx + n < nDPCount)
		{
			CBCGPChartDataPoint* pDPOld = m_arDataPoints[nToIdx + n];

			if (pDPOld != NULL)
			{
				delete pDPOld;
			}
		}
		m_arDataPoints.SetAtGrow (nToIdx + n, pDP);
	}
}
//****************************************************************************************
int CBCGPChartSeries::AddDataPoint(CBCGPChartDataPoint* pDataPoint)
{
	ASSERT_VALID(this);
	ASSERT(pDataPoint != NULL);

	if (IsVirtualMode())
	{
		return 0;
	}

	if (IsOptimizedLongDataMode())
	{
		int nIndex = (int)m_arLongDataY.GetSize();

		CBCGPChartValue valY = pDataPoint->GetComponentValue(CBCGPChartData::CI_Y);
		m_arLongDataY.Add(valY);
		SetMinMaxValues(valY, CBCGPChartData::CI_Y, nIndex);

		if (m_arLongDataY1.GetSize() > 0)
		{
			CBCGPChartValue valY1 = pDataPoint->GetComponentValue(CBCGPChartData::CI_Y1);
			m_arLongDataY1.Add(valY1);
			SetMinMaxValues(valY1, CBCGPChartData::CI_Y1, nIndex);
		}

		if (m_arLongDataX.GetSize() > 0)
		{
			m_arLongDataX.Add(pDataPoint->GetComponentValue(CBCGPChartData::CI_X));
		}

		return nIndex;
	}

	if (m_bHistoryMode)
	{
		BOOL bDeleteDP = TRUE;
		if (m_bReverseAdd)
		{
			if (m_nCurrentIndex <= m_nHistoryDepth - 1)
			{
				bDeleteDP = FALSE;
			}
			else
			{
				m_nCurrentIndex--;
			}
		}
		
		if (bDeleteDP)
		{
			CBCGPChartDataPoint* pDataPoint = m_arDataPoints.GetAt(0);
			if (pDataPoint != NULL)
			{
				delete pDataPoint;
			}

			m_arDataPoints.RemoveAt(0, 1);
		}
		
	}

	int nDataPointIndex = 0;

	if (m_bHistoryMode)
	{
		CBCGPChartDataPoint* pOldDP = NULL;
		if (m_bReverseAdd)
		{
			pOldDP = m_nCurrentIndex < m_arDataPoints.GetSize() ? m_arDataPoints.GetAt(m_nCurrentIndex) : NULL;
			m_arDataPoints.SetAtGrow(m_nCurrentIndex, pDataPoint);
			nDataPointIndex = m_nCurrentIndex;

			if (m_nCurrentIndex <= m_nHistoryDepth - 1)
			{
				m_nCurrentIndex++;
			}
		}
		else
		{
			pOldDP = m_nHistoryDepth - 1 < m_arDataPoints.GetSize() ? m_arDataPoints.GetAt(m_nHistoryDepth - 1) : NULL;
			m_arDataPoints.SetAtGrow(m_nHistoryDepth - 1, pDataPoint);
			nDataPointIndex = m_nHistoryDepth - 1;
		}

		if (pOldDP != NULL)
		{
			delete pOldDP;
		}
	}
	else
	{
		nDataPointIndex = (int)m_arDataPoints.Add(pDataPoint);
	}

	if (m_bUpdateAxesOnNewData && m_bHistoryMode)
	{
		UpdateAxes();
	}

	if (nDataPointIndex != -1 && !m_bHistoryMode)
	{
		CString strCategoryName = pDataPoint->GetCategoryName();
		if (!strCategoryName.IsEmpty())
		{
			CBCGPChartAxisX* pXAxis = DYNAMIC_DOWNCAST(CBCGPChartAxisX, m_pXAxis);
			if (pXAxis != NULL)
			{
				ASSERT_VALID(pXAxis);
				pXAxis->m_arCategories.SetAtGrow(nDataPointIndex, strCategoryName);
			}
		}
	}

	if (pDataPoint->IsEmpty())
	{
		if (GetTreatNulls() == CBCGPChartSeries::TN_VALUE)
		{
			SetMinMaxValues(0., CBCGPChartData::CI_Y, nDataPointIndex);
		}

		return nDataPointIndex;
	}

	int nComponentCount = pDataPoint->GetDataComponentCount();

	int nMinMaxIndex = GetDataPointCount() - 1;

	for (int i = 0; i < nComponentCount; i++)
	{
		CBCGPChartValue dblVal = pDataPoint->GetComponentValue((CBCGPChartData::ComponentIndex)i);

		if (i == CBCGPChartData::CI_X || i == CBCGPChartData::CI_Z)
		{
			if (!dblVal.IsEmpty())
			{
				if (i == CBCGPChartData::CI_X)
				{
					m_bXComponentSet = TRUE;
				}
				else if (i == CBCGPChartData::CI_Z)
				{
					m_bZComponentSet = TRUE;
				}
				SetMinMaxValues(dblVal, (CBCGPChartData::ComponentIndex)i, nMinMaxIndex);
			}
			continue;
		}

		if (!dblVal.IsEmpty() || GetTreatNulls() == CBCGPChartSeries::TN_VALUE)
		{
			SetMinMaxValues(dblVal, (CBCGPChartData::ComponentIndex)i, nMinMaxIndex);
		}
	}

	if (IsAutoColorDataPoints())
	{
		m_pChart->UpdateSeriesColorIndexes();
	}

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, FALSE);
	}

	int nPrevDPIndex = nDataPointIndex - 1;

	if (nPrevDPIndex >= 0)
	{
		CBCGPChartValue prevVal = GetDataPointValue(nPrevDPIndex, CBCGPChartData::CI_X);

		if (!prevVal.IsEmpty())
		{
			CBCGPChartValue thisVal = pDataPoint->GetComponentValue(CBCGPChartData::CI_X);
			if (!thisVal.IsEmpty())
			{
				double dblDistance = fabs(thisVal) - fabs(prevVal);
				if (dblDistance != 0)
				{
					m_dblMinDataPointDistance = min(m_dblMinDataPointDistance, fabs(thisVal) - fabs(prevVal));
				}
			}
		}
	}

	m_bFullStackedMinMaxSet = FALSE;

	return nDataPointIndex;
}
//****************************************************************************************
int CBCGPChartSeries::AddDataPoint(const CBCGPChartDataPoint& srcDataPoint)
{
	return AddDataPoint(new CBCGPChartDataPoint(srcDataPoint));
}
//****************************************************************************************
int CBCGPChartSeries::AddDataPoint(const CString& strCategoryName, double dblY, BCGPChartFormatSeries* pDataPointFormat, DWORD_PTR dwUserData)
{
	return AddDataPoint(new CBCGPChartDataPoint(strCategoryName, dblY, CBCGPChartData::CI_Y, dwUserData, pDataPointFormat));
}
//****************************************************************************************
int CBCGPChartSeries::AddDataPoint(double dblY, BCGPChartFormatSeries* pDataPointFormat, DWORD_PTR dwUserData)
{
	return AddDataPoint(new CBCGPChartDataPoint(dblY, CBCGPChartData::CI_Y, dwUserData, pDataPointFormat));
}
//****************************************************************************************
int CBCGPChartSeries::AddDataPoint(double dblY, double dblX, BCGPChartFormatSeries* pDataPointFormat, DWORD_PTR dwUserData)
{
	ASSERT_VALID(this);

	CBCGPChartDataPoint* pDP = new CBCGPChartDataPoint(dblY, CBCGPChartData::CI_Y, dwUserData, pDataPointFormat);
	pDP->SetComponentValue(dblX, CBCGPChartData::CI_X);
	return AddDataPoint(pDP);
}
//****************************************************************************************
int CBCGPChartSeries::AddEmptyDataPoint(const CString& strCategoryName, DWORD_PTR dwUserData, BCGPChartFormatSeries* pDataPointFormat)
{
	CBCGPChartDataPoint* pDP = new CBCGPChartDataPoint();
	pDP->m_dwUserData = dwUserData;
	pDP->SetCategoryName(strCategoryName);

	if (pDataPointFormat != NULL)
	{
		pDP->SetFormat(*pDataPointFormat);
	}

	return AddDataPoint(pDP);
}
//****************************************************************************************
int CBCGPChartSeries::AddEmptyDataPoint(DWORD_PTR dwUserData, BCGPChartFormatSeries* pDataPointFormat)
{
	CBCGPChartDataPoint* pDP = new CBCGPChartDataPoint();
	pDP->m_dwUserData = dwUserData;

	if (pDataPointFormat != NULL)
	{
		pDP->SetFormat(*pDataPointFormat);
	}

	return AddDataPoint(pDP);
}
//****************************************************************************************
int CBCGPChartSeries::AddEmptyDataPoint(double dblX, DWORD_PTR dwUserData, BCGPChartFormatSeries* pDataPointFormat)
{
	ASSERT_VALID(this);
	return AddDataPoint(new CBCGPChartDataPoint(dblX, CBCGPChartData::CI_X, dwUserData, pDataPointFormat));
}
//****************************************************************************************
void CBCGPChartSeries::AddDataPoints(const CBCGPDoubleArray& arYValues, CBCGPDoubleArray* pXValues, 
									 CBCGPDoubleArray* pY1Values, BOOL bRecalcMinMaxValues)
{
	ASSERT_VALID(this);
	int nYSize = (int)arYValues.GetSize();
	
	if (nYSize == 0)
	{
		return;
	}
	
	int nXSize = (int)(pXValues == NULL ? 0 : pXValues->GetSize());
	int nY1Size = (int)(pY1Values == NULL ? 0 : pY1Values->GetSize());
	
	int nOldSize = (int)m_arDataPoints.GetSize();
	m_arDataPoints.SetSize(nOldSize + nYSize);
	
	for (int i = 0; i < nYSize; i++)
	{
		CBCGPChartDataPoint* pDP = new CBCGPChartDataPoint(arYValues[i]);
		
		if (i < nXSize)
		{
			pDP->SetComponentValue(pXValues->GetAt(i), CBCGPChartData::CI_X);
		}
		
		if (i < nY1Size)
		{
			pDP->SetComponentValue(pY1Values->GetAt(i), CBCGPChartData::CI_Y1);
		}
		
		m_arDataPoints.SetAt(nOldSize + i, pDP);
	}
	
	if (bRecalcMinMaxValues)
	{
		RecalcMinMaxValues();
	}
}
//****************************************************************************************
void CBCGPChartSeries::AddDataPointsOptimized(const CBCGPDoubleArray& arYValues, CBCGPDoubleArray* pXValues, 
									 CBCGPDoubleArray* pY1Values, BOOL bRecalcMinMaxValues)
{
	ASSERT_VALID(this);

	if (CBCGPChartVisualObject::IsCategory3D(m_chartCategory) || IsHistoryMode())
	{
		AddDataPoints(arYValues, pXValues, pY1Values, bRecalcMinMaxValues);
		return; 
	}

	int nYSize = (int)arYValues.GetSize();

	if (nYSize == 0)
	{
		return;
	}

	m_arLongDataY.Append(arYValues);

	if (pXValues != NULL)
	{
		m_arLongDataX.Append(*pXValues);
		m_arLongDataX.SetSize(m_arLongDataY.GetSize());
	}

	if (pY1Values != NULL)
	{
		m_arLongDataY1.Append(*pY1Values);
		m_arLongDataY1.SetSize(m_arLongDataY.GetSize());
	}

	if (bRecalcMinMaxValues)
	{
		RecalcMinMaxValues();
	}

	if (m_pXAxis != NULL)
	{
		m_pXAxis->SetFixedIntervalWidth(m_nFixedGridSizeX, m_nValuesPerIntervalX);
		SetIndexMode(TRUE);
		m_pXAxis->ShowScrollBar(TRUE);
	}

	if (m_pYAxis != NULL)
	{
		m_pYAxis->EnableZoom(FALSE);
		m_pYAxis->EnableScroll(FALSE);
	}
}
//****************************************************************************************
const BCGPChartFormatSeries* CBCGPChartSeries::GetDataPointFormat(int nIndex, BOOL bAlloc)
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= GetDataPointCount() || IsVirtualMode())
	{
		return &m_formatSeries;
	}

	const CBCGPChartDataPoint* pDp = GetDataPointAt(nIndex);

	if (pDp != NULL)
	{
		const BCGPChartFormatSeries* pFormat = pDp->GetFormat();

		if (pFormat != NULL)
		{
			return pFormat;
		}
		else if (bAlloc)
		{
			((CBCGPChartDataPoint*)pDp)->SetFormat(m_formatSeries);
			return pDp->GetFormat();
		}
	}

	return &m_formatSeries;
}
//****************************************************************************************
const BCGPChartFormatSeries* CBCGPChartSeries::GetDataPointFormat(int nIndex) const
{
	ASSERT_VALID(this);
	
	if (nIndex < 0 || nIndex >= GetDataPointCount() || IsVirtualMode())
	{
		return &m_formatSeries;
	}

	const CBCGPChartDataPoint* pDp = GetDataPointAt(nIndex);
	
	if (pDp != NULL)
	{
		const BCGPChartFormatSeries* pFormat = pDp->GetFormat();
		
		if (pFormat != NULL)
		{
			return pFormat;
		}
	}
	
	return &m_formatSeries;
}
//****************************************************************************************
void CBCGPChartSeries::ClearDataPointFormat(int nDataPointIndex)
{
	if (IsVirtualMode())
	{
		return;
	}

	if (nDataPointIndex == -1)
	{
		for (int i = 0; i < GetDataPointCount(); i++)
		{
			CBCGPChartDataPoint* pDP = m_arDataPoints[i];

			if (pDP != NULL)
			{
				pDP->ClearFormat();
			}
		}

		return;
	}

	CBCGPChartDataPoint* pDP = (CBCGPChartDataPoint*) GetDataPointAt(nDataPointIndex);

	if (pDP != NULL)
	{
		pDP->ClearFormat();
	}
}
//****************************************************************************************
const BCGPChartFormatSeries* CBCGPChartSeries::GetColors(BCGPSeriesColorsPtr& seriesColors, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pChart);

	if (m_pChart == NULL)
	{
		return NULL;
	}

	BCGPChartFormatSeries* pFormatSeries = &m_formatSeries;
	
	int nColorIndex = -1;

	if (nDataPointIndex != -1)
	{
		pFormatSeries = (BCGPChartFormatSeries*)GetDataPointFormat(nDataPointIndex, FALSE);
	}

	if (IsAutoColorDataPoints())
	{
		nColorIndex = GetDataPointColorIndex(nDataPointIndex);
	}

	if (nColorIndex == -1)
	{
		nColorIndex = m_nColorIndex;
	}

	ASSERT(pFormatSeries != NULL);

	if (pFormatSeries == NULL)
	{
		return NULL;
	}

	seriesColors.Init(*pFormatSeries);
	m_pChart->GetColors().GetSeriesColors(seriesColors, nColorIndex, GetDefaultFillGradientType());

	double dblSeriesFillOpacity = pFormatSeries->GetSeriesFillOpacity();
	if (seriesColors.m_pBrElementFillColor != NULL)
	{
		if (dblSeriesFillOpacity >= 0.0 && dblSeriesFillOpacity <= 1.0)
		{
			seriesColors.m_pBrElementFillColor->SetOpacity(dblSeriesFillOpacity);
		}
		else if (m_pChart != NULL)
		{
			double dblThemeOpacity = m_pChart->GetColors().GetOpacity();
			seriesColors.m_pBrElementFillColor->SetOpacity(dblThemeOpacity);
		}
	}

	return pFormatSeries;
}
//****************************************************************************************
BOOL CBCGPChartSeries::SetDataPointColorIndex(int nDataPointIndex, int nColorIndex)
{
	ASSERT_VALID(this);

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*) GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return FALSE;
	}

	pDataPoint->SetColorIndex(nColorIndex);
	return TRUE;
}
//****************************************************************************************
int CBCGPChartSeries::GetDataPointColorIndex(int nDataPointIndex)
{
	ASSERT_VALID(this);

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*) GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return m_nColorIndex;
	}

	return pDataPoint->GetColorIndex();
}
//****************************************************************************************
CBCGPRect CBCGPChartSeries::GetDataPointBoundingRect(int nDataPointIndex)
{
	ASSERT_VALID(this);

	if (IsOptimizedLongDataMode())
	{
		CBCGPRect rect;
		m_mapDataPointBoundingRects.Lookup(nDataPointIndex, rect);
		return rect;
	}

	const CBCGPChartDataPoint* pDataPoint = GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return CBCGPRect();
	}

	return pDataPoint->m_rectBounds;
}
//****************************************************************************************
void CBCGPChartSeries::SetDataPointBoundingRect(int nDataPointIndex, const CBCGPRect& rect)
{
	ASSERT_VALID(this);

	if (IsOptimizedLongDataMode())
	{
		m_mapDataPointBoundingRects.SetAt(nDataPointIndex, rect);
		return;
	}
	
	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*) GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return;
	}
	
	pDataPoint->m_rectBounds = rect;
}

//****************************************************************************************
CBCGPRect CBCGPChartSeries::GetDataPointLabelRect(int nDataPointIndex) const
{
	ASSERT_VALID(this);

	if (IsOptimizedLongDataMode())
	{
		CBCGPRect rect;
		m_mapDataPointLabelsRects.Lookup(nDataPointIndex, rect);
		return rect;
	}

	const CBCGPChartDataPoint* pDataPoint = GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL || !pDataPoint->m_bShowSmartLabel)
	{
		return CBCGPRect();
	}

	return pDataPoint->m_rectDataLabel;
}
//****************************************************************************************
BOOL CBCGPChartSeries::SetDataPointLabelRect(int nDataPointIndex, const CBCGPRect& rect)
{
	ASSERT_VALID(this);

	if (IsOptimizedLongDataMode())
	{
		m_mapDataPointLabelsRects.SetAt(nDataPointIndex, rect);
		return TRUE;
	}

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*)GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return FALSE;
	}

	pDataPoint->m_rectDataLabel = rect;
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartSeries::GetDataPointLabelDropLines(int nDataPointIndex, CBCGPPoint& ptLabelLineStart, 
										CBCGPPoint& ptLableLineEnd, CBCGPPoint& ptLabelUnderline) const
{
	ASSERT_VALID(this);

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*)GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return FALSE;
	}

	ptLabelLineStart = pDataPoint->m_ptLabelLineStart;
	ptLableLineEnd = pDataPoint->m_ptLabelLineEnd;
	ptLabelUnderline = pDataPoint->m_ptLabelUnderline;

	return ptLabelLineStart != ptLableLineEnd;
}
//****************************************************************************************
BOOL CBCGPChartSeries::SetUseWordWrapForDataLabels(BOOL bSet, int nDataPointIndex)
{
	ASSERT_VALID(this);

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*)GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return FALSE;
	}

	pDataPoint->m_bUseWordWrapForDataLabels = bSet;
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartSeries::IsUseWordWrapForDataLabels(int nDataPointIndex)
{
	ASSERT_VALID(this);

	const CBCGPChartDataPoint* pDataPoint = GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return FALSE;
	}

	return pDataPoint->m_bUseWordWrapForDataLabels;
}
//****************************************************************************************
BOOL CBCGPChartSeries::SetDataPointLabelDropLinePoints(int nDataPointIndex, const CBCGPPoint& ptLabelLineStart, 
											 const CBCGPPoint& ptLabelLineEnd, const CBCGPPoint& ptLabelUnderline)
{
	ASSERT_VALID(this);

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*)GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return FALSE;
	}

	pDataPoint->m_ptLabelLineStart = ptLabelLineStart;
	pDataPoint->m_ptLabelLineEnd = ptLabelLineEnd;
	pDataPoint->m_ptLabelUnderline = ptLabelUnderline;

	return TRUE;
}
//****************************************************************************************
LPCTSTR CBCGPChartSeries::GetDataPointCategoryName(int nDataPointIndex)
{
	ASSERT_VALID(this);

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*)GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return NULL;
	}

	return pDataPoint->GetCategoryName();
}
//****************************************************************************************
BOOL CBCGPChartSeries::SetDataPointCategoryName(const CString& strName, int nDataPointIndex)
{
	ASSERT_VALID(this);

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*)GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return FALSE;
	}

	if (m_pXAxis != NULL)
	{
		ASSERT_VALID(m_pXAxis);

		CBCGPChartAxisX* pXAxis = DYNAMIC_DOWNCAST(CBCGPChartAxisX, m_pXAxis);

		if (pXAxis != NULL)
		{
			pXAxis->m_arCategories.SetAtGrow(nDataPointIndex, strName);
		}
	}

	pDataPoint->SetCategoryName(strName);
	return TRUE;
}
//****************************************************************************************
DWORD_PTR CBCGPChartSeries::GetDataPointUserData(int nDataPointIndex) const
{
	ASSERT_VALID(this);
	
	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*)GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return 0;
	}
	
	return pDataPoint->m_dwUserData;
}
//****************************************************************************************
BOOL CBCGPChartSeries::SetDataPointUserData(int nDataPointIndex, DWORD_PTR dwUserData)
{
	ASSERT_VALID(this);
	
	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*)GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return FALSE;
	}
	
	pDataPoint->m_dwUserData = dwUserData;
	return TRUE;
}
//****************************************************************************************
CBCGPPoint CBCGPChartSeries::GetDataPointScreenPoint(int nDataPointIndex, int nScreenPointIndex) const
{
	ASSERT_VALID(this);

	if (IsVirtualMode())
	{
		const CBCGPPointsArray& arPoints = GetVirtualSeriesScreenPoints();
		if (nDataPointIndex >= 0 && nDataPointIndex < arPoints.GetSize())
		{
			return arPoints.GetAt(nDataPointIndex);
		}
		
		return CBCGPPoint();
	}

	const CBCGPChartDataPoint* pDataPoint = GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL || IsOptimizedLongDataMode() && nDataPointIndex < GetLongDataOffset())
	{
		return CBCGPPoint();
	}

	if (m_arLongDataY.GetSize() > 0 && m_pChart != NULL)
	{
		BOOL bIsEmpty = FALSE;
		CBCGPPoint point = m_pChart->ScreenPointFromChartData(pDataPoint->GetData(), nDataPointIndex, 
			bIsEmpty, (CBCGPChartSeries*)this);
		
		if (!bIsEmpty && nScreenPointIndex == 1)
		{
			if (m_arLongDataY1.GetSize() > 0 && m_pYAxis != NULL)
			{
				double dblVal = m_pYAxis->PointFromValue(m_arLongDataY[nDataPointIndex] + m_arLongDataY1[nDataPointIndex], FALSE);
				m_pYAxis->IsVertical() ? point.y = dblVal : point.x = dblVal;
			}
		}

		return point;
	}

	return pDataPoint->GetScreenPoint(nScreenPointIndex);
}
//****************************************************************************************
void CBCGPChartSeries::SetDataPointScreenPoint(int nDataPointIndex, int nScreenPointIndex, CBCGPPoint pt)
{
	ASSERT_VALID(this);

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*)GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return;
	}

	pDataPoint->SetScreenPointAt(nScreenPointIndex, pt);
}
//****************************************************************************************
void CBCGPChartSeries::RemoveAllDataPointScreenPoints(int nDataPointIndex)
{
	ASSERT_VALID(this);

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*)GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return;
	}

	pDataPoint->RemoveAllScreenPoints();
}
//****************************************************************************************
BOOL CBCGPChartSeries::IsDataPointScreenPointsEmpty(int nDataPointIndex) const 
{
	ASSERT_VALID(this);

	if (m_arLongDataY.GetSize() > 0)
	{
		if (nDataPointIndex >= m_arLongDataY.GetSize() || nDataPointIndex < GetLongDataOffset())
		{
			return TRUE;
		}

		if (m_arLongDataY[nDataPointIndex] == GetLongDataEmptyValue() && GetTreatNulls() != CBCGPChartSeries::TN_VALUE)
		{
			return TRUE;
		}

		return FALSE;
	}

	const CBCGPChartDataPoint* pDataPoint = GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return TRUE;
	}

	return pDataPoint->IsScreenPointsEmpty();
}
//****************************************************************************************
int CBCGPChartSeries::GetDataPointScreenPointCount(int nDataPointIndex) const 
{
	ASSERT_VALID(this);

	if (m_arLongDataY.GetSize() > 0)
	{
		if (nDataPointIndex >= m_arLongDataY.GetSize() || nDataPointIndex < GetLongDataOffset())
		{
			return 0;
		}
		
		if (m_arLongDataY[nDataPointIndex] == GetLongDataEmptyValue() && GetTreatNulls() != CBCGPChartSeries::TN_VALUE)
		{
			return 0;
		}

		if (m_arLongDataY1.GetSize() > 0)
		{
			return 2;
		}
		
		return 1;
	}

	const CBCGPChartDataPoint* pDataPoint = GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return TRUE;
	}

	return pDataPoint->GetScreenPointCount();
}
//****************************************************************************************
CBCGPChartShape3D* CBCGPChartSeries::GetDataPointShape3D(int nDataPointIndex) const
{
	ASSERT_VALID(this);

	const CBCGPChartDataPoint* pDataPoint = GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return NULL;
	}

	return pDataPoint->GetShape3D();
}
//****************************************************************************************
void CBCGPChartSeries::SetDataPointShape3D(const CBCGPChartShape3D& shape3D, int nDataPointIndex)
{
	ASSERT_VALID(this);

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*) GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return;
	}

	pDataPoint->SetShape3D(shape3D);
}
//****************************************************************************************
void CBCGPChartSeries::SetDataPointShape3D(CBCGPChartShape3D* pShape3D, int nDataPointIndex)
{
	ASSERT_VALID(this);

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*) GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return;
	}

	pDataPoint->SetShape3D(pShape3D);
}
//****************************************************************************************
BOOL CBCGPChartSeries::OnSetDataPointDataLabelText(int nDataPointIndex, const CString& strText)
{
	ASSERT_VALID(this);

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*) GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return FALSE;
	}

	pDataPoint->m_strDataLabel = strText;
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartSeries::OnGetDataLabelText(int nDataPointIndex, CString& strText) const
{
	ASSERT_VALID(this);

	const CBCGPChartDataPoint* pDataPoint = GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return FALSE;
	}

	strText = pDataPoint->m_strDataLabel;
	return !strText.IsEmpty();
}
//****************************************************************************************
void CBCGPChartSeries::OnGetDataPointLegendLabel(int nDataPointIndex, CString& strLabel)
{
	ASSERT_VALID(this);
	strLabel = m_strSeriesName;

	if (nDataPointIndex < 0 || nDataPointIndex >= GetDataPointCount())
	{
		return;
	}

	const CBCGPChartDataPoint* pDp = GetDataPointAt(nDataPointIndex);

	if (pDp != NULL)
	{
		strLabel = pDp->m_strLegendLabel;
	}

	if (strLabel.IsEmpty())
	{
		BCGPChartDataLabelOptions::LabelContent content = 
			m_formatSeries.m_legendLabelContent == BCGPChartDataLabelOptions::LC_DEFAULT_CONTENT ? 
			GetDefaultLegendContent() : m_formatSeries.m_legendLabelContent;

		m_bGetDataPointLabelForLegend = TRUE;

		GetDataPointLabelText(nDataPointIndex, content, strLabel);

		m_bGetDataPointLabelForLegend = FALSE;
	}
}
//****************************************************************************************
BOOL CBCGPChartSeries::CanIncludeDataPointToLegend(int nDataPointIndex)
{
	ASSERT_VALID(this);

	if (m_pChartFormula != NULL && m_pChartFormula->IsNonDiscreteCurve() && 
		nDataPointIndex == 0)
	{
		return TRUE;
	}

	const CBCGPChartDataPoint* pDp = GetDataPointAt(nDataPointIndex);

	if (pDp == NULL)
	{
		return FALSE;
	}

	if (pDp->GetComponentValue().IsEmpty() && GetTreatNulls() != CBCGPChartSeries::TN_VALUE)
	{
		return FALSE;
	}

	return m_bIncludeDataPointLabelsToLegend && pDp->m_bIncludeLabelToLegend;
}
//****************************************************************************************
BOOL CBCGPChartSeries::GetDataPointLabelText(int nDataPointIndex, BCGPChartDataLabelOptions::LabelContent content, 
											 CString& strDPLabel)
{
	ASSERT_VALID(this);

	if (m_pXAxis == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(m_pXAxis);

	const BCGPChartFormatSeries* pFormatSeries = GetDataPointFormat(nDataPointIndex, FALSE);
	BOOL bNeedSeparator = FALSE;
	CString strFormat = pFormatSeries->m_dataLabelFormat.m_options.m_strDataFormat;

	if (strFormat.IsEmpty())
	{
		strFormat = _T("%.9G");
	}

	if ((content & BCGPChartDataLabelOptions::LC_DP_INDEX) == BCGPChartDataLabelOptions::LC_DP_INDEX)
	{
		CString strIndex;
		strIndex.Format(_T("%d"), nDataPointIndex + 1);

		if (content != BCGPChartDataLabelOptions::LC_DP_INDEX)
		{
			// Has other flags
			strIndex += _T(": ");
		}

		strDPLabel += strIndex;
	}

	if ((content & BCGPChartDataLabelOptions::LC_SERIES_NAME) == BCGPChartDataLabelOptions::LC_SERIES_NAME)
	{
		strDPLabel += m_strSeriesName;
		bNeedSeparator = !m_strSeriesName.IsEmpty();
	}

	if ((content & BCGPChartDataLabelOptions::LC_CATEGORY_NAME) == BCGPChartDataLabelOptions::LC_CATEGORY_NAME)
	{
		CString strValue = GetDataPointCategoryName(nDataPointIndex);

		if (strValue.IsEmpty())
		{
			m_pXAxis->GetDisplayedLabel(nDataPointIndex + 1, strValue);
		}

		if (!strValue.IsEmpty())
		{
			if (bNeedSeparator)
			{
				strDPLabel += pFormatSeries->m_dataLabelFormat.m_options.m_strLabelSeparator;
			}

			strDPLabel += strValue;

			bNeedSeparator = !strValue.IsEmpty();
		}
	}

	if ((content & BCGPChartDataLabelOptions::LC_X_VALUE) == BCGPChartDataLabelOptions::LC_X_VALUE)
	{
		CString strValue;
		CBCGPChartValue val = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_X);

		if (m_pXAxis->m_bFormatAsDate)
		{
			m_pXAxis->GetDisplayedLabel(val.GetValue(), strValue);
		}
		else
		{
			if (val.IsEmpty())
			{
				strValue.Format(_T("%d"), (nDataPointIndex + 1));
			}
			else
			{
				strValue.Format(strFormat, (double)val);
			}
		}

		if (bNeedSeparator)
		{
			strDPLabel += pFormatSeries->m_dataLabelFormat.m_options.m_strLabelSeparator;
		}

		strDPLabel += strValue;
		bNeedSeparator = !strValue.IsEmpty();
	}

	if (((content & BCGPChartDataLabelOptions::LC_VALUE) == BCGPChartDataLabelOptions::LC_VALUE) || 
			content == BCGPChartDataLabelOptions::LC_DEFAULT_CONTENT)
	{
		CString strValue;
		double dblValue = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y).GetValue();

		strValue.Format(strFormat, dblValue);
		
		if (bNeedSeparator)
		{
			strDPLabel += pFormatSeries->m_dataLabelFormat.m_options.m_strLabelSeparator;
		}

		strDPLabel += strValue;
		bNeedSeparator = !strValue.IsEmpty();
	}

	if ((content & BCGPChartDataLabelOptions::LC_PERCENTAGE) == BCGPChartDataLabelOptions::LC_PERCENTAGE)
	{
		CString strValue;
		CBCGPChartValue val = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_X);
		double dblVal = val;

		val.IsEmpty() ? strValue = _T("0%") : strValue.Format(_T("%.1f%%"), dblVal);
		
		if (bNeedSeparator)
		{
			strDPLabel += pFormatSeries->m_dataLabelFormat.m_options.m_strLabelSeparator;
		}

		strDPLabel += strValue;
		bNeedSeparator = !strValue.IsEmpty();
	}

	if ((content & BCGPChartDataLabelOptions::LC_BUBBLE_SIZE) == BCGPChartDataLabelOptions::LC_BUBBLE_SIZE)
	{
		CString strValue;
		CBCGPChartValue val = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y1);

		if (!val.IsEmpty())
		{
			m_pXAxis->GetDisplayedLabel(val, strValue);

			if (bNeedSeparator)
			{
				strDPLabel += pFormatSeries->m_dataLabelFormat.m_options.m_strLabelSeparator;
			}

			strDPLabel += strValue;
			bNeedSeparator = !strValue.IsEmpty();
		}
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartSeries::GetDataPointTableText(int nDataPointIndex, CString& strDPLabel, int nPrecision)
{
	ASSERT_VALID(this);

	const BCGPChartFormatSeries* pSeriesFormat = GetDataPointFormat(nDataPointIndex, FALSE);
	if (pSeriesFormat == NULL)
	{
		return FALSE;
	}

	const BCGPChartDataTableOptions& options = pSeriesFormat->m_dataTableOptions;

	CString strFormat = options.m_strDataFormat;
	if (strFormat.IsEmpty())
	{
		strFormat = _T("%.9G");
	}

	BOOL bNeedSeparator = FALSE;

	if ((options.m_content & BCGPChartDataTableOptions::TC_DP_INDEX) == BCGPChartDataTableOptions::TC_DP_INDEX)
	{
		CString strIndex;
		strIndex.Format(_T("%d"), nDataPointIndex + 1);

		if (options.m_content != BCGPChartDataTableOptions::TC_DP_INDEX)
		{
			// Has other flags
			strIndex += _T(": ");
		}

		strDPLabel += strIndex;
	}

	if (((options.m_content & BCGPChartDataTableOptions::TC_VALUE) == BCGPChartDataTableOptions::TC_VALUE) || 
			options.m_content == BCGPChartDataTableOptions::TC_DEFAULT_CONTENT)
	{
		CBCGPChartValue val = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y);
		if (!val.IsEmpty() || GetTreatNulls() == CBCGPChartSeries::TN_VALUE)
		{
			CString strValue;
			strValue.Format(strFormat, bcg_double_precision(val.GetValue(), nPrecision));
			
			if (bNeedSeparator)
			{
				strDPLabel += options.m_strLabelSeparator;
			}

			strDPLabel += strValue;
			bNeedSeparator = !strValue.IsEmpty();
		}
	}

	return TRUE;
}
//****************************************************************************************
CBCGPChartData CBCGPChartSeries::GetDataPointData(int nDataPointIndex) const
{
	ASSERT_VALID(this);

	if (nDataPointIndex < 0 || nDataPointIndex >= GetDataPointCount())
	{
		return CBCGPChartData();
	}

	const CBCGPChartDataPoint* pDataPoint = GetDataPointAt(nDataPointIndex);

	if (pDataPoint == NULL)
	{
		return CBCGPChartData();
	}

	return pDataPoint->GetData();
}
//****************************************************************************************
CBCGPChartValue CBCGPChartSeries::GetDataPointValue(int nDataPointIndex, CBCGPChartData::ComponentIndex ci) const
{
	ASSERT_VALID(this);

	if (nDataPointIndex < 0 || nDataPointIndex >= GetDataPointCount())
	{
		return CBCGPChartValue();
	}

	const CBCGPChartDataPoint* pDataPoint = GetDataPointAt(nDataPointIndex);

	if (pDataPoint == NULL)
	{
		return CBCGPChartValue();
	}

	return pDataPoint->GetComponentValue(ci);
}
//****************************************************************************************
BOOL CBCGPChartSeries::SetDataPointValue(int nDataPointIndex, double dblValue, CBCGPChartData::ComponentIndex ci)
{
	ASSERT_VALID(this);

	if (nDataPointIndex < 0 || nDataPointIndex >= GetDataPointCount())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (m_arLongDataY.GetSize() > 0)
	{
		if (ci == CBCGPChartData::CI_Y)
		{
			m_arLongDataY.SetAtGrow(nDataPointIndex, dblValue);
		}
		else if (ci == CBCGPChartData::CI_X)
		{
			m_arLongDataX.SetAtGrow(nDataPointIndex, dblValue);
		}
		else if (ci == CBCGPChartData::CI_Y1)
		{
			m_arLongDataY1.SetAtGrow(nDataPointIndex, dblValue);
		}
		else
		{
			return FALSE;
		}

		return TRUE;
	}

	CBCGPChartDataPoint* pDataPoint = m_arDataPoints[nDataPointIndex];

	if (pDataPoint != NULL)
	{
		pDataPoint->SetComponentValue(dblValue, ci);
		SetMinMaxValues(dblValue, ci, nDataPointIndex);

		if (ci == CBCGPChartData::CI_X)
		{
			m_bXComponentSet = TRUE;
		}

		if (ci == CBCGPChartData::CI_Z)
		{
			m_bZComponentSet = TRUE;
		}
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartSeries::OnGetDataPointTooltip(int nDataPointIndex, CString& strTooltip, CString& strTooltipDescr)
{
	CBCGPChartAxis* pAxisY = GetRelatedAxis(CBCGPChartSeries::AI_Y);
	CBCGPChartAxis* pAxisX = GetRelatedAxis(CBCGPChartSeries::AI_X);

	if (pAxisY == NULL || pAxisX == NULL)
	{
		return TRUE;
	}

	ASSERT_VALID(pAxisY);
	ASSERT_VALID(pAxisX);

	CBCGPChartValue valY = GetDataPointValue(nDataPointIndex);

	if (valY.IsEmpty())
	{
		return TRUE;
	}

	CBCGPChartValue valX = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_X);

	if (m_pXAxis->IsIndexedSeries())
	{
		valX.SetValue(nDataPointIndex);
	}
	else if (valX.IsEmpty())
	{
		valX.SetValue(nDataPointIndex + 1);
	}
	
	strTooltip = m_strSeriesName;

	if (m_strSeriesName.IsEmpty())
	{
		strTooltip = _T("Series");
	}

	CString strLabelY;
	pAxisY->GetDisplayedLabel(valY.GetValue(), strLabelY);

	CString strLabelX;
	pAxisX->GetDisplayedLabel(valX.GetValue(), strLabelX);

	CString strValueFormat;
	if (m_chartType == BCGP_CT_RANGE || m_chartCategory == BCGPChartBubble)
	{
		strValueFormat.LoadString(IDS_BCGBARRES_CHART_TOOLTIP_RANGE_VALUE_FORMAT);
		CBCGPChartValue valY1 = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y1);
		CString strLabelY1;

		if (valY1.IsEmpty())
		{
			// X used as Value,  Y - as Size (Y1)
			strLabelY1 = strLabelY;
			valX = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_X);
			pAxisY->GetDisplayedLabel(valX.GetValue(), strLabelY);
		}
		else
		{
			pAxisY->GetDisplayedLabel(valY1.GetValue(), strLabelY1);
		}
		
		strTooltipDescr.Format(strValueFormat, strLabelX, strLabelY, strLabelY1);
	}
	else
	{
		strValueFormat.LoadString(IDS_BCGBARRES_CHART_TOOLTIP_VALUE_FORMAT);
		strTooltipDescr.Format(strValueFormat, strLabelX, strLabelY);
	}
	
	return TRUE;
}
//****************************************************************************************
void CBCGPChartSeries::CopyDataPoints(CArray<CBCGPChartDataPoint*, CBCGPChartDataPoint*>& dest) const
{
	ASSERT_VALID(this);

	dest.RemoveAll();
	int nCount = GetDataPointCount();
	dest.SetSize(nCount);

	for (int i = 0; i < nCount; i++)
	{
		const CBCGPChartDataPoint* pDataPoint = GetDataPointAt(i);
		CBCGPChartDataPoint* pNewDataPoint = NULL;

		if (pDataPoint != NULL)
		{
			pNewDataPoint = new CBCGPChartDataPoint(*pDataPoint);
			pNewDataPoint->ClearShape3D();
		}
		else
		{
			pNewDataPoint = new CBCGPChartDataPoint();
		}

		dest.SetAt(i, pNewDataPoint);
	}
}
//****************************************************************************************
void CBCGPChartSeries::CopyFrom(const CBCGPChartSeries& src)
{
	ASSERT_VALID(this);

	m_chartCategory = src.m_chartCategory;
	m_chartType = src.m_chartType;
	
	m_arLongDataX.RemoveAll();
	m_arLongDataY.RemoveAll();
	m_arLongDataY1.RemoveAll();
	if (src.IsOptimizedLongDataMode())
	{
		m_arLongDataX.Append(src.GetLongDataX());
		m_arLongDataY.Append(src.GetLongDataY());
		m_arLongDataY1.Append(src.GetLongDataY1());
		SetLongDataOffset(src.GetLongDataOffset());
	}
	else
	{
		src.CopyDataPoints(m_arDataPoints);
	}

	m_nFixedGridSizeX = src.m_nFixedGridSizeX;
	m_nFixedGridSizeY = src.m_nFixedGridSizeY;
	m_nValuesPerIntervalX = src.m_nValuesPerIntervalX;
	m_nValuesPerIntervalY = src.m_nValuesPerIntervalY;

	m_formatSeries = src.m_formatSeries;
	m_strSeriesName = src.m_strSeriesName;

	m_dwUserData = src.m_dwUserData;

	m_nLegendKeyToLabelDistance = src.m_nLegendKeyToLabelDistance;
	m_rectLegendKey = src.m_rectLegendKey;
	m_rectLegendLabel = src.m_rectLegendLabel;

	m_bVisible = src.m_bVisible;
	m_bIncludeInvisibleSeriesToLegend = src.m_bIncludeInvisibleSeriesToLegend;

	m_bAutoColorDataPoints = src.IsAutoColorDataPoints();
	m_nColorIndex = src.GetColorIndex();
	m_nLegendID = src.m_nLegendID;

	m_bIncludeDataPointLabelsToLegend = src.m_bIncludeDataPointLabelsToLegend;
	m_bIgnoreNegativeValues = src.IsIgnoreNegativeValues();

	CBCGPBaseChartImpl* pNewImpl = NULL;

	if (src.GetChartImpl() != NULL)
	{
		pNewImpl = (CBCGPBaseChartImpl*)src.GetChartImpl()->GetRuntimeClass()->CreateObject();
		pNewImpl->SetRelatedChartControl(m_pChart);
	}

	if (m_pChartImpl != NULL)
	{
		delete m_pChartImpl;
	}

	m_pChartImpl = pNewImpl;
	m_pChart = src.m_pChart;

	SetRelatedAxes(src.GetRelatedAxis(CBCGPChartSeries::AI_X), 
					src.GetRelatedAxis(CBCGPChartSeries::AI_Y), 
					src.GetRelatedAxis(CBCGPChartSeries::AI_Z));

	m_treatNulls = src.GetTreatNulls();
	SetLongDataEmptyValue(src.GetLongDataEmptyValue());
	SetLongDataOffset(src.GetLongDataOffset());
}
//****************************************************************************************
void CBCGPChartSeries::RecalcMinMaxValues()
{
	ASSERT_VALID(this);

	BOOL bDynamixMinMax = m_pXAxis != NULL && m_pXAxis->IsFixedIntervalWidth();

	if (!bDynamixMinMax &&
		(m_pXAxis != NULL && m_pXAxis->IsFixedDisplayRange() && !IsXComponentSet() && 
		 m_pYAxis != NULL && m_pYAxis->IsFixedDisplayRange() &&
		 (m_pZAxis == NULL || m_pZAxis->IsFixedDisplayRange()) || 
		 IsFormulaSeries()))	
	{
		return;
	}

	ClearMinMaxValues();

	BOOL bCheckRangeMin = FALSE;
	BOOL bCheckRangeMax = FALSE;
	double dblDisplayedMin = 0;
	double dblDisplayedMax = 0;

	int nDataPointCount = GetDataPointCount();

	m_nMinDisplayedXIndex = nDataPointCount;
	m_nMaxDisplayedXIndex = 0;


	if (m_pXAxis != NULL)
	{
		bCheckRangeMin = m_pXAxis->IsFixedMinimumDisplayValue() && CanUseRangeForMinMax();
		bCheckRangeMax = m_pXAxis->IsFixedMaximumDisplayValue() && CanUseRangeForMinMax();

		if (bCheckRangeMin)
		{
			dblDisplayedMin = m_pXAxis->GetMinDisplayedValue();
		}

		if (bCheckRangeMax)
		{
			dblDisplayedMax = m_pXAxis->GetMaxDisplayedValue();
		}

		if (bCheckRangeMin && bCheckRangeMax && m_pXAxis->GetMajorUnit() > (dblDisplayedMax - dblDisplayedMin))
		{
			dblDisplayedMin -= m_pXAxis->GetMajorUnit();
			dblDisplayedMax += m_pXAxis->GetMajorUnit();
		}

	}

	int nComponentCount = 0;
	int nLastCheckedIndex = 0;

	int nStartIndex = 0; 
	int nEndIndex = nDataPointCount - 1;

	BOOL bSetDisplayedIndex = TRUE;

	if (m_pXAxis != NULL && m_pXAxis->IsIndexedSeries() && CanUseRangeForMinMax())
	{
		nStartIndex = (int)m_pXAxis->GetMinDisplayedValue();
		nEndIndex = (int)(min(m_pXAxis->GetMaxDisplayedValue() + 1, nDataPointCount - 1));

		if (nStartIndex == nEndIndex && nEndIndex == 0)
		{
			nEndIndex = nDataPointCount - 1;
		}

		m_nMinDisplayedXIndex = nStartIndex;
		m_nMaxDisplayedXIndex = nEndIndex;
		bCheckRangeMin = FALSE;
		bCheckRangeMax = FALSE;
		bSetDisplayedIndex = FALSE;
	}

	for (int i = nStartIndex; i <= nEndIndex; i++)
	{
		const CBCGPChartDataPoint* pDataPoint = GetDataPointAt(i);

		if (pDataPoint != NULL)
		{
			nComponentCount = pDataPoint->GetDataComponentCount();

			CBCGPChartValue dblXVal = pDataPoint->GetComponentValue(CBCGPChartData::CI_X);

			if (!dblXVal.IsEmpty())
			{
				m_bXComponentSet = TRUE;
			}

			if (m_bIndexMode)
			{
				dblXVal.SetValue(i);
			}

			if (bCheckRangeMin && dblXVal < dblDisplayedMin) 
			{
				continue;
			}

			if (bCheckRangeMax && dblXVal > dblDisplayedMax)
			{
				nLastCheckedIndex = i;
				break;
			}

			if (nComponentCount > 1 && !dblXVal.IsEmpty())
			{
				SetMinMaxValues(dblXVal, CBCGPChartData::CI_X, i);
			}

			if (bSetDisplayedIndex)
			{
				m_nMinDisplayedXIndex = min(m_nMinDisplayedXIndex, i);
				m_nMaxDisplayedXIndex = max(m_nMinDisplayedXIndex, i);
			}
			
			for (int j = 0; j < nComponentCount; j++)
			{
				if (j != CBCGPChartData::CI_X)
				{
					CBCGPChartValue val = pDataPoint->GetComponentValue((CBCGPChartData::ComponentIndex)j);

					if (!val.IsEmpty() || GetTreatNulls() == CBCGPChartSeries::TN_VALUE)
					{
						SetMinMaxValues(val, (CBCGPChartData::ComponentIndex)j, i);
					}
				}
			}

			if (nComponentCount == 0 && GetTreatNulls() == CBCGPChartSeries::TN_VALUE)
			{
				SetMinMaxValues(0, CBCGPChartData::CI_Y, i);
			}
		}
	}

	if (bCheckRangeMax)
	{
		if (m_nMaxDisplayedXIndex < m_nMinDisplayedXIndex)
		{
			m_nMaxDisplayedXIndex = nLastCheckedIndex;
			m_nMinDisplayedXIndex = nLastCheckedIndex;
		}
		else
		{
			m_nMinDisplayedXIndex = max(0, m_nMinDisplayedXIndex - 1);
			m_nMaxDisplayedXIndex = min(m_nMaxDisplayedXIndex + 1, nDataPointCount - 1);
		}
	}

	m_bFullStackedMinMaxSet = FALSE;
}
//****************************************************************************************
void CBCGPChartSeries::SetFullStackedMinMax()
{
	ASSERT_VALID(this);

	if (m_chartType != BCGP_CT_100STACKED || m_bFullStackedMinMaxSet)
	{
		return;
	}

	RecalcMinMaxValues();

	m_bFullStackedMinMaxSet = TRUE;
}
//****************************************************************************************
void CBCGPChartSeries::ApplySeriesColorsToDataPointFormat(int nDataPointIndex)
{
	ASSERT_VALID(this);

	CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*) GetDataPointAt(nDataPointIndex);
	if (pDataPoint == NULL)
	{
		return;
	}

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) pDataPoint->GetFormat();

	if (pFormat == NULL)
	{
		// uses defaults anyway
		return;
	}

	pFormat->m_markerFormat.m_brFillColor = m_formatSeries.m_markerFormat.m_brFillColor;
	pFormat->m_markerFormat.m_outlineFormat.m_brLineColor = m_formatSeries.m_markerFormat.m_outlineFormat.m_brLineColor;
	pFormat->m_seriesElementFormat.m_brFillColor = m_formatSeries.m_seriesElementFormat.m_brFillColor;
	pFormat->m_seriesElementFormat.m_outlineFormat.m_brLineColor = m_formatSeries.m_seriesElementFormat.m_outlineFormat.m_brLineColor;
	pFormat->m_dataLabelFormat.m_brFillColor = m_formatSeries.m_dataLabelFormat.m_brFillColor;
	pFormat->m_dataLabelFormat.m_outlineFormat.m_brLineColor = m_formatSeries.m_dataLabelFormat.m_outlineFormat.m_brLineColor;
	pFormat->m_dataLabelFormat.m_brTextColor = m_formatSeries.m_dataLabelFormat.m_brTextColor;


	pFormat->m_seriesElementFormat.m_brSideFillColor =	pFormat->m_seriesElementFormat.m_brFillColor;
	pFormat->m_seriesElementFormat.m_brSideFillColor.MakeDarker(0.2);

}
//****************************************************************************************
void CBCGPChartSeries::SetMarkerFill(const CBCGPBrush& br, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_markerFormat.m_brFillColor = br;
}
//****************************************************************************************
void CBCGPChartSeries::SetMarkerLineColor(const CBCGPBrush& br, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_markerFormat.m_outlineFormat.m_brLineColor = br;
}
//****************************************************************************************
void CBCGPChartSeries::SetMarkerLineWidth(double dblWidth, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_markerFormat.m_outlineFormat.m_dblWidth = dblWidth;
}
//****************************************************************************************
void CBCGPChartSeries::SetMarkerStrokeStyle(const CBCGPStrokeStyle& strokeStyle, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_markerFormat.m_outlineFormat.m_strokeStyle = strokeStyle;
}
//****************************************************************************************
void CBCGPChartSeries::SetMarkerSize (int nSize, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_markerFormat.m_options.SetMarkerSize(nSize);

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetMarkerShape(BCGPChartMarkerOptions::MarkerShape shape, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_markerFormat.m_options.m_markerShape = shape;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetMarkerFormat(const BCGPChartFormatMarker& format, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_markerFormat = format;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetMarkerOptions(const BCGPChartMarkerOptions& options, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_markerFormat.m_options = options;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::ShowMarker(BOOL bShow, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_markerFormat.m_options.m_bShowMarker = bShow;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetSeriesFill(const CBCGPBrush& br, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_seriesElementFormat.m_brFillColor = br;
	pFormat->m_seriesElementFormat.Generate3DColors();
}
//****************************************************************************************
void CBCGPChartSeries::SetSeriesLineColor(const CBCGPBrush& br, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_seriesElementFormat.m_outlineFormat.m_brLineColor = br;
}
//****************************************************************************************
void CBCGPChartSeries::SetSeriesLineWidth(double dblWidth, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_seriesElementFormat.m_outlineFormat.m_dblWidth = dblWidth;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetSeriesLineDashStyle(CBCGPStrokeStyle::BCGP_DASH_STYLE style, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_seriesElementFormat.m_outlineFormat.m_strokeStyle.SetDashStyle(style);

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetSeriesStrokeStyle(const CBCGPStrokeStyle& strokeStyle, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_seriesElementFormat.m_outlineFormat.m_strokeStyle = strokeStyle;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetSeriesElementFormat(const BCGPChartFormatArea& format, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_seriesElementFormat = format;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::ShowDataLabel(BOOL bShow, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_options.m_bShowDataLabel = bShow;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelOptions(const BCGPChartDataLabelOptions& options, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_options = options;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelFill(const CBCGPBrush& br, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_brFillColor = br;
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelLineColor(const CBCGPBrush& br, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_outlineFormat.m_brLineColor = br;
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelLineWidth(double dblWidth, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_outlineFormat.m_dblWidth = dblWidth;
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelStrokeStyle(const CBCGPStrokeStyle& strokeStyle, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_outlineFormat.m_strokeStyle = strokeStyle;
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelTextFormat(const CBCGPTextFormat& textFormat, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_textFormat = textFormat;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelDrawBorder(BOOL bDraw, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_options.m_bDrawDataLabelBorder = bDraw;
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelDropLineToMarker(BOOL bDrop, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_options.m_bDropLineToMarker = bDrop;
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelUnderline(BOOL bUnderline, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_options.m_bUnderlineDataLabel = bUnderline;
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelSeparator(const CString& strSeparator, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_options.m_strLabelSeparator = strSeparator;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelDisplayKey(BOOL bSet, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_options.m_bIncludeLegendKeyInLabel = bSet;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelAngle(double dblAngle, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_options.m_dblAngle = dblAngle;
	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
double CBCGPChartSeries::GetDataPointLabelAngle(int nDataPointIndex) const
{
	ASSERT_VALID(this);

	CBCGPChartDataPoint* pDP = (CBCGPChartDataPoint*)GetDataPointAt(nDataPointIndex);

	if (pDP == NULL)
	{
		return 0.;
	}

	if (pDP->m_nSmartLabelAngle != -1)
	{
		return pDP->m_nSmartLabelAngle;
	}

	const BCGPChartFormatSeries* pFormat = GetDataPointFormat(nDataPointIndex);
	return pFormat->m_dataLabelFormat.m_options.m_dblAngle;
}
//****************************************************************************************
double CBCGPChartSeries::GetDataPointLabelDistance(int nDataPointIndex) const
{
	ASSERT_VALID(this);
	
	CBCGPChartDataPoint* pDP = (CBCGPChartDataPoint*)GetDataPointAt(nDataPointIndex);
	
	if (pDP == NULL)
	{
		return 0.;
	}

	const BCGPChartFormatSeries* pFormat = GetDataPointFormat(nDataPointIndex);
	CBCGPSize szScaleRatio = pFormat->m_dataLabelFormat.m_options.GetScaleRatio();
	
	if (pDP->m_dblSmartLabelDistance != DBL_MAX)
	{
		return pDP->m_dblSmartLabelDistance * max(szScaleRatio.cx, szScaleRatio.cy);
	}
	
	return pFormat->m_dataLabelFormat.m_options.GetDistanceFromMarker();
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelDistanceFromMarker(double dblOffset, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_options.m_dblDistanceFromMarker = dblOffset;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelPosition(BCGPChartDataLabelOptions::LabelPosition position, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_options.m_position = position;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelContent(BCGPChartDataLabelOptions::LabelContent content, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_options.m_content = content;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelContentPadding(const CBCGPSize& sz, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.SetContentPadding(sz);

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelDataFormat(const CString& strFormat, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat.m_options.m_strDataFormat = strFormat;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetDataLabelFormat(const BCGPChartFormatDataLabel& format, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dataLabelFormat = format;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetLegendLabelTextFormat(const CBCGPTextFormat& textFormat, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_legendLabelFormat.m_textFormat = textFormat;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetLegendLabelTextColor(const CBCGPBrush& br, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_legendLabelFormat.m_brTextColor = br;
}
//****************************************************************************************
void CBCGPChartSeries::SetLegendLabelFillColor(const CBCGPBrush& br, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_legendLabelFormat.m_brFillColor = br;
}
//****************************************************************************************
void CBCGPChartSeries::SetLegendLabelFormat(const BCGPChartFormatLabel& format, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_legendLabelFormat = format;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSeries::SetLegendLabelContent(BCGPChartDataLabelOptions::LabelContent content, int nDataPointIndex)
{
	ASSERT_VALID(this);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_legendLabelContent = content;
}
//****************************************************************************************
void CBCGPChartSeries::SetDataPoint3DLineThickness(double dblLineThickness, int nDataPointIndex)
{
	ASSERT_VALID(this);

	dblLineThickness = bcg_clamp(dblLineThickness, 0.1, 30.);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(nDataPointIndex, TRUE);
	pFormat->m_dbl3DLineThickness = dblLineThickness;
}
//****************************************************************************************
void CBCGPChartSeries::SetCurveType(BCGPChartFormatSeries::ChartCurveType type)
{
	ASSERT_VALID(this);
	m_formatSeries.m_curveType = type;
}
//****************************************************************************************
BCGPChartFormatSeries::ChartCurveType CBCGPChartSeries::GetCurveType() const
{
	ASSERT_VALID(this);

	if (m_formatSeries.m_curveType == (BCGPChartFormatSeries::ChartCurveType) -1)
	{
		return BCGPChartFormatSeries::CCT_LINE;
	}

	return m_formatSeries.m_curveType;
}
//****************************************************************************************
void CBCGPChartSeries::AdjustGradientAngels()
{
	ASSERT_VALID(this);

	for (int i = 0; i < GetDataPointCount(); i++)
	{
		const CBCGPChartDataPoint* pDp = GetDataPointAt(i);

		if (pDp != NULL)
		{
			BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) pDp->GetFormat();

			if (pFormat != NULL)
			{
				pFormat->AdjustGradientAngles();
			}
		}
	}

	m_formatSeries.AdjustGradientAngles();
}
//****************************************************************************************
void CBCGPChartSeries::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld)
{
	for (int i = 0; i < GetDataPointCount(); i++)
	{
		BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) GetDataPointFormat(i, FALSE);
		if (pFormat != NULL && pFormat != &m_formatSeries)
		{
			pFormat->OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);
		}
	}

	m_formatSeries.OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);
	ResetMaxDataLabelSize();
}
//****************************************************************************************
int CBCGPChartSeries::GetLegendElementCount() const
{
	if (m_pChartFormula != NULL && m_pChartFormula->IsNonDiscreteCurve())
	{
		return 1;
	}

	return m_bIncludeDataPointLabelsToLegend ?  GetDataPointCount() : 1;
}
//****************************************************************************************
void CBCGPChartSeries::UpdateSeriesColors()
{
	if (!m_pChart->IsChart3D())
	{
		return;
	}

	for (int i = 0; i < GetDataPointCount(); i++)
	{
		CBCGPChartShape3D* pShape = GetDataPointShape3D(i);

		if (pShape != NULL)
		{
			pShape->ResetDrawingResources();
		}
	}
}
//****************************************************************************************
CBCGPDoubleArray* CBCGPChartSeries::GetDataBuffer(int nBufferIndex)
{
	if (nBufferIndex < 0)
	{
		ASSERT(FALSE);
		return NULL;
	}

	if (nBufferIndex >= (int)m_arDataBuffers.GetSize())
	{
		CBCGPDoubleArray* pNewBuffer = new CBCGPDoubleArray();
		m_arDataBuffers.SetAtGrow(nBufferIndex, pNewBuffer);
	}

	return m_arDataBuffers[nBufferIndex];
}
//****************************************************************************************
BOOL CBCGPChartSeries::HitTestDataPoint(const CBCGPPoint& pt, BCGPChartHitInfo* pHitInfo)
{
	if (!GetAxesBoundingRect().NormalizedRect().PtInRect(pt) || m_pChart == NULL)
	{
		return FALSE;
	}
	
	for (int j = GetMaxDataPointIndex(); j >= GetMinDataPointIndex(); j--)
	{
		const CBCGPChartDataPoint* pDataPoint = GetDataPointAt(j);
		const BCGPChartFormatSeries* pSeriesFormat = GetDataPointFormat(j, FALSE);
		
		if (pDataPoint == NULL || pDataPoint->IsEmpty() || pSeriesFormat == NULL)
		{
			continue;
		}
		
		CBCGPRect rectBounds = GetDataPointBoundingRect(j);
		
		if (pSeriesFormat->m_markerFormat.m_options.m_bShowMarker && !m_pChart->IsThumbnailMode())
		{
			rectBounds.InflateRect(pSeriesFormat->m_markerFormat.GetMarkerSize());
		}
		else
		{
			rectBounds.InflateRect(m_pChart->GetHitTestDataPointPrecision(TRUE));
		}
		
		if (rectBounds.NormalizedRect().PtInRect(pt))
		{
			pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_DATA_POINT;
			pHitInfo->m_nIndex2 = j;
			return TRUE;
		}
	}

	return FALSE;
}
//****************************************************************************************
int CBCGPChartSeries::GetShadowAngle() const
{
	return m_formatSeries.m_shadowType.m_nAngle < 0 ? GetDefaultShadowAngle() : m_formatSeries.m_shadowType.m_nAngle;
}
//****************************************************************************************
int CBCGPChartSeries::GetShadowTransparencyPercent() const
{
	double dblOpacity = 0.0;

	if (m_pChart != NULL)
	{
		dblOpacity = m_pChart->GetColors().GetOpacity();
	}
	else
	{
		dblOpacity = m_formatSeries.m_seriesElementFormat.m_brFillColor.GetOpacity();
	}

	double dblOpacityShadow = 0.01 * (100 - m_formatSeries.m_shadowType.m_nTransparencyPercent);

	return (int)(100.0 * (1.0 - dblOpacity * dblOpacityShadow));
}
//****************************************************************************************
BOOL CBCGPChartSeries::IsDisplayShadow() const
{
	return m_formatSeries.m_shadowType.m_bDisplayShadow;
}
//****************************************************************************************
CBCGPBrush::BCGP_GRADIENT_TYPE CBCGPChartSeries::GetDefaultFillGradientType() const
{
	return m_pChart != NULL && m_pChart->GetColors().IsFlatTheme() ? CBCGPBrush::BCGP_NO_GRADIENT : m_FillGradienType;
}

//****************************************************************************************
// CBCGPChartSeries - specific LINE
//****************************************************************************************
CBCGPChartLineSeries::CBCGPChartLineSeries()
{
	CommonInit();
}
//****************************************************************************************
CBCGPChartLineSeries::CBCGPChartLineSeries(CBCGPChartVisualObject* pChartCtrl,  
						BCGPChartCategory chartCategory, BCGPChartType chartType) :
						CBCGPChartSeries(pChartCtrl, chartCategory, chartType)
{
	CommonInit();
}
//****************************************************************************************
CBCGPChartLineSeries::CBCGPChartLineSeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
	const CBCGPColor& seriesColor, BCGPChartCategory chartCategory, BCGPChartType chartType) :
	CBCGPChartSeries(pChartCtrl, strSeriesName, seriesColor, chartCategory, chartType)
{
	CommonInit();
}
//****************************************************************************************
void CBCGPChartLineSeries::CommonInit()
{
	m_bColorEachLine = FALSE;
	m_bConnectFirstLastPoints = FALSE;
	m_bFillClosedShape = FALSE;
	m_FillGradienType = GetSeriesFillGradientType();
}
//****************************************************************************************
void CBCGPChartLineSeries::EnableColorEachLine(BOOL bEnable, BOOL bRedraw)
{
	ASSERT_VALID(this);

	m_bColorEachLine = bEnable;

	if (m_pChart != NULL && bRedraw)
	{
		ASSERT_VALID(m_pChart);
		m_pChart->Redraw();
	}
}
//****************************************************************************************
// CBCGPChartSeries - specific AREA
//****************************************************************************************
CBCGPChartAreaSeries::CBCGPChartAreaSeries()
{
	CommonInit();
}
//****************************************************************************************
CBCGPChartAreaSeries::CBCGPChartAreaSeries(CBCGPChartVisualObject* pChartCtrl,  BCGPChartCategory chartCategory, 
										   BCGPChartType chartType) :
						CBCGPChartSeries(pChartCtrl, chartCategory, chartType)
{
	CommonInit();
}
//****************************************************************************************
CBCGPChartAreaSeries::CBCGPChartAreaSeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
	const CBCGPColor& seriesColor, BCGPChartCategory chartCategory, BCGPChartType chartType)
	: CBCGPChartSeries(pChartCtrl, strSeriesName, seriesColor, chartCategory, chartType)
{
	CommonInit();
}
//****************************************************************************************
void CBCGPChartAreaSeries::CommonInit()
{
	m_FillGradienType = CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT;
}
//****************************************************************************************
void CBCGPChartAreaSeries::ClearMinMaxValues()
{
	ASSERT_VALID(this);

	CBCGPChartSeries::ClearMinMaxValues();
	
	if (!IsStakedSeries() && !IsRangeSeries() && !m_valOrigin.IsEmpty())
	{
		SetMinValue(m_valOrigin, CBCGPChartData::CI_Y);
		SetMaxValue(m_valOrigin, CBCGPChartData::CI_Y);
	}
}
//****************************************************************************************
void CBCGPChartAreaSeries::RecalcMinMaxValues()
{
	CBCGPChartSeries::RecalcMinMaxValues();

	if (m_chartCategory == BCGPChartArea3D && !IsStakedSeries())
	{
		double dblVal = GetMinValue(CBCGPChartData::CI_Y);

		if (dblVal < m_valOrigin.GetValue())
		{
			m_valOrigin.SetValue(dblVal);
		}
	}
}
//****************************************************************************************
void CBCGPChartAreaSeries::SetAreaOrigin(double dblValue, BOOL bRedraw)
{
	ASSERT_VALID(this);

	m_valOrigin.SetValue(dblValue);
	RecalcMinMaxValues();

	if (m_pChart != NULL && bRedraw)
	{
		ASSERT_VALID(m_pChart);
		m_pChart->Redraw();
	}
}
//****************************************************************************************
BOOL CBCGPChartAreaSeries::SetDataPointValue(int nDataPointIndex, double dblValue, CBCGPChartData::ComponentIndex ci)
{
	ASSERT_VALID(this);

	if (m_chartCategory == BCGPChartArea3D && ci == CBCGPChartData::CI_Y)
	{
		if (dblValue < 0. && !IsStakedSeries())
		{
			m_valOrigin.SetValue(min(m_valOrigin.GetValue(), dblValue));
		}
	}

	return CBCGPChartSeries::SetDataPointValue(nDataPointIndex, dblValue, ci);
}
//****************************************************************************************
int CBCGPChartAreaSeries::AddDataPoint(CBCGPChartDataPoint* pDataPoint)
{
	ASSERT_VALID(this);

	if (m_chartCategory == BCGPChartArea3D && pDataPoint != NULL)
	{
		double dblVal = pDataPoint->GetComponentValue(CBCGPChartData::CI_Y);

		if (dblVal < 0. && !IsStakedSeries())
		{
			m_valOrigin.SetValue(min(m_valOrigin.GetValue(), dblVal));
		}
	}

	return CBCGPChartSeries::AddDataPoint(pDataPoint);
}
//****************************************************************************************
int CBCGPChartAreaSeries::AddDataPoint(const CBCGPChartDataPoint& srcDataPoint)
{
	return CBCGPChartSeries::AddDataPoint(srcDataPoint);
}
//****************************************************************************************
int CBCGPChartAreaSeries::AddDataPoint(const CString& strCategoryName, double dblY, BCGPChartFormatSeries* pDataPointFormat, DWORD_PTR dwUserData)
{
	return CBCGPChartSeries::AddDataPoint(strCategoryName, dblY, pDataPointFormat, dwUserData);
}
//****************************************************************************************
int CBCGPChartAreaSeries::AddDataPoint(double dblY, BCGPChartFormatSeries* pDataPointFormat, DWORD_PTR dwUserData)
{
	return CBCGPChartSeries::AddDataPoint(dblY, pDataPointFormat, dwUserData);
}
//****************************************************************************************
int CBCGPChartAreaSeries::AddDataPoint(double dblY, double dblX, BCGPChartFormatSeries* pDataPointFormat, DWORD_PTR dwUserData)
{
	return CBCGPChartSeries::AddDataPoint(dblY, dblX, pDataPointFormat, dwUserData);
}
//****************************************************************************************
// CBCGPChartSurfaceSeries - specific SURFACE
//****************************************************************************************
CBCGPChartSurfaceSeries::CBCGPChartSurfaceSeries(CBCGPChartVisualObject* pChartCtrl) : 
						CBCGPChartSeries(pChartCtrl, BCGPChartSurface3D)
{
	CommonInit();
}
//****************************************************************************************
CBCGPChartSurfaceSeries::CBCGPChartSurfaceSeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
						const CBCGPColor& seriesColor) : 
						CBCGPChartSeries(pChartCtrl, strSeriesName, seriesColor, BCGPChartSurface3D, BCGP_CT_SIMPLE)
{
	CommonInit();
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::CommonInit()
{
	m_nXSize = -1;
	m_colorMode = CM_MULTIPLE;
	m_nColorMapCount = -1;
	m_dblSurfaceOpacity = 1.;

	SetColorMapCount(16);

	m_surfaceType = CBCGPChartSurfaceSeries::ST_LEVELS;
	m_bContinuousLegendKey = TRUE;
	m_bLevelRangleInLabel = FALSE;

	m_strLegendLevelValueFormat = _T("%.2G");

	m_bWireFrame = FALSE;

	m_frameStyle = CBCGPChartSurfaceSeries::FS_MESH;

	m_levelRangeMode = CBCGPChartSurfaceSeries::LRM_MINMAX_Y_AXIS;
	m_dblMinCustomLevelRangeVal = 0.;
	m_dblMaxCustomLevelRangeVal = 0.;

	m_bEnableFrameTransparency = FALSE;

	m_dblFlatLevel = 0.;
	m_bDrawFlat = FALSE;
}
//****************************************************************************************
CBCGPChartSurfaceSeries::~CBCGPChartSurfaceSeries()
{
	CleanBrushes();
	ClearSurfaceCells();
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::CleanBrushes()
{
	int i;
	for (i = 0; i < m_arFillBrushes.GetSize(); i++)
	{
		delete m_arFillBrushes[i];
	}

	for (i = 0; i < m_arLineBrushes.GetSize(); i++)
	{
		delete m_arLineBrushes[i];
	}

	for (i = 0; i < m_arFillBackBrushes.GetSize(); i++)
	{
		delete m_arFillBackBrushes[i];
	}

	for (i = 0; i < m_arLineBackBrushes.GetSize(); i++)
	{
		delete m_arLineBackBrushes[i];
	}


	m_arFillBrushes.RemoveAll();
	m_arLineBrushes.RemoveAll();

	m_arFillBackBrushes.RemoveAll();
	m_arLineBackBrushes.RemoveAll();
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::SetSurfaceType(SurfaceType type)
{
	if (type != m_surfaceType)
	{
		m_surfaceType = type;
		ClearSurfaceCells();

		if (m_pChart != NULL)
		{
			m_pChart->SetDirty(TRUE);
		}
	}
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::SetFrameStyle(FrameStyle style)
{
	m_frameStyle = style;
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::SetFrameColor(const CBCGPBrush& br)
{
	m_brFrame = br;
	m_brFrame.SetColor(m_brFrame.GetColor(), m_bEnableFrameTransparency ? m_dblSurfaceOpacity : 1.);
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::SetWireFrame(BOOL bSet)
{
	m_bWireFrame = bSet;
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::SetSurfaceDimension(int nXSize)
{
	nXSize = bcg_clamp(nXSize, 2, 10000);
	m_nXSize = nXSize;
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::SetColorMapCount(int nCount)
{
	if (m_nColorMapCount == nCount)
	{
		return;
	}

	m_nColorMapCount = bcg_clamp(nCount, 1, 256);
	UpdateSeriesColors();
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::SetColorMode(ColorMode colorMode)
{
	if (m_colorMode == colorMode)
	{
		return;
	}

	m_colorMode = colorMode;
	UpdateSeriesColors();
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::SetSurfaceOpacity(double dblOpacity)
{
	dblOpacity = bcg_clamp(dblOpacity, 0., 1.);

	if (dblOpacity == m_dblSurfaceOpacity)
	{
		return;
	}

	m_dblSurfaceOpacity = dblOpacity;

	UpdateSeriesColors();
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::EnableFrameTransparency(BOOL bEnable)
{
	if (m_bEnableFrameTransparency == bEnable)
	{
		return;
	}

	m_bEnableFrameTransparency = bEnable;

	double dblNewOpacity = m_bEnableFrameTransparency ? m_dblSurfaceOpacity : 1.;

	for (int i = 0; i < m_arLineBrushes.GetSize(); i++)
	{
		CBCGPBrush* pBrush = m_arLineBrushes[i];
		pBrush->SetColor(pBrush->GetColor(), dblNewOpacity);

		CBCGPBrush* pBrushBack = m_arLineBackBrushes[i];
		pBrushBack->SetColor(pBrushBack->GetColor(), dblNewOpacity);
	}

	m_brFrame.SetColor(m_brFrame.GetColor(), dblNewOpacity);
}
//****************************************************************************************
const BCGPChartFormatSeries* CBCGPChartSurfaceSeries::GetColors(BCGPSeriesColorsPtr& seriesColors, int nDataPointIndex)
{
	if (nDataPointIndex == -1)
	{
		return CBCGPChartSeries::GetColors(seriesColors, -1);
	}

	const CBCGPChartDataPoint* pDp = GetDataPointAt(nDataPointIndex);
	const BCGPChartFormatSeries* pFormatDP = pDp->GetFormat();

	CBCGPBrush* pBrFill = NULL;
	CBCGPBrush* pBrLine = NULL;

	CBCGPBrush* pBrFillBack = NULL;

	int nBrushIndex = -1;

	if (pFormatDP != NULL)
	{
		if (!pFormatDP->m_seriesElementFormat.m_brFillColor.IsEmpty())
		{
			pBrFill = (CBCGPBrush*) &pFormatDP->m_seriesElementFormat.m_brFillColor;
		}

		if (!pFormatDP->m_seriesElementFormat.m_outlineFormat.m_brLineColor.IsEmpty())
		{
			pBrLine = (CBCGPBrush*) &pFormatDP->m_seriesElementFormat.m_outlineFormat.m_brLineColor;
		}

		if (!pFormatDP->m_seriesElementFormat.m_brBottomFillColor.IsEmpty())
		{
			pBrFillBack = (CBCGPBrush*) &pFormatDP->m_seriesElementFormat.m_brBottomFillColor;
		}
	}

	if (pBrFill == NULL)
	{
		nBrushIndex = GetBrushIndex(nDataPointIndex);
		if (nBrushIndex < m_arFillBrushes.GetSize())
		{
			pBrFill = m_arFillBrushes[nBrushIndex];
		}
	}

	if (pBrFillBack == NULL)
	{
		if (nBrushIndex == - 1)
		{
			nBrushIndex = GetBrushIndex(nDataPointIndex);
		}

		if (nBrushIndex < m_arFillBackBrushes.GetSize())
		{
			pBrFillBack = m_arFillBackBrushes[nBrushIndex];
		}
	}

	if (pBrLine == NULL)
	{
		if (nBrushIndex == - 1)
		{
			nBrushIndex = GetBrushIndex(nDataPointIndex);
		}

		if (nBrushIndex < m_arFillBrushes.GetSize())
		{
			pBrLine = m_arLineBrushes[nBrushIndex];
		}
	}

	const BCGPChartFormatSeries* pFormatSeries = pFormatDP != NULL ? pFormatDP : &m_formatSeries;
	
	seriesColors.Init(*pFormatSeries);
	m_pChart->GetColors().GetSeriesColors(seriesColors, 0, GetDefaultFillGradientType());

	if (pBrFill != NULL)
	{
		seriesColors.m_pBrElementFillColor = pBrFill;
	}

	if (pBrFillBack != NULL)
	{
		seriesColors.m_pBrElementBottomFillColor = pBrFillBack;
	}

	if (pBrLine != NULL)
	{
		seriesColors.m_pBrElementLineColor = pBrLine;
	}
	
	return pFormatSeries;
}
//****************************************************************************************
int CBCGPChartSurfaceSeries::AddDataPoint(CBCGPChartDataPoint* pDataPoint)
{
	int nRetVal = CBCGPChartSeries::AddDataPoint(pDataPoint);

	if (m_nXSize != -1)
	{
		return nRetVal;
	}

	CBCGPChartValue minVal = GetMaxValue(CBCGPChartData::CI_X);

	if (!minVal.IsEmpty() && pDataPoint->GetComponentValue(CBCGPChartData::CI_X) < minVal.GetValue())
	{
		m_nXSize = GetDataPointCount() - 1;
	}

	return nRetVal;
}
//****************************************************************************************
int CBCGPChartSurfaceSeries::AddDataPoint(const CBCGPChartDataPoint& srcDataPoint)
{
	return CBCGPChartSeries::AddDataPoint(srcDataPoint);
}
//****************************************************************************************
int CBCGPChartSurfaceSeries::AddDataPoint(double dblY, double dblX, BCGPChartFormatSeries* pDataPointFormat , DWORD_PTR dwUserData)
{
	return CBCGPChartSeries::AddDataPoint(dblY, dblX, pDataPointFormat, dwUserData);
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::OnCalcScreenPoints(CBCGPGraphicsManager* /*pGM*/, const CBCGPRect& /*rectDiagramArea*/)
{
	ASSERT_VALID(this);

	if (m_pXAxis == NULL || m_pYAxis == NULL || m_pZAxis == NULL)
	{
		return;
	}

	CBCGPChartDiagram3D* pDiagram3D = m_pChart->GetDiagram3D();

	if (pDiagram3D == NULL)
	{
		return;
	}
	
	CBCGPPoint ptDiagramCenter = pDiagram3D->GetDiagramRect().CenterPoint();

	CBCGPPoint	ptXAxisStart;
	CBCGPPoint	ptXAxisEnd;
	double		dblXAxisSize = m_pXAxis->GetAxisSize(TRUE);
	BOOL		bIsDisplaydataBetweenTickMarksX = m_pXAxis->IsDisplayDataBetweenTickMarks();
	BOOL		bIsLogScaleX = m_pXAxis->IsLogScale();

	CBCGPPoint	ptYAxisStart;
	CBCGPPoint	ptYAxisEnd;
	double		dblYAxisSize = m_pYAxis->GetAxisSize(TRUE);
	BOOL		bIsDisplaydataBetweenTickMarksY = m_pYAxis->IsDisplayDataBetweenTickMarks();
	BOOL		bIsLogScaleY = m_pYAxis->IsLogScale();

	CBCGPPoint	ptZAxisStart;
	CBCGPPoint	ptZAxisEnd;
	double		dblZAxisSize = m_pZAxis->GetAxisSize(TRUE);
	BOOL		bIsDisplaydataBetweenTickMarksZ = m_pZAxis->IsDisplayDataBetweenTickMarks();
	BOOL		bIsLogScaleZ = m_pZAxis->IsLogScale();

	m_pXAxis->GetAxisPos(ptXAxisStart, ptXAxisEnd);
	m_pYAxis->GetAxisPos(ptYAxisStart, ptYAxisEnd);
	m_pZAxis->GetAxisPos(ptZAxisStart, ptZAxisEnd);


	if (m_surfaceType == CBCGPChartSurfaceSeries::ST_STANDARD || m_surfaceType == CBCGPChartSurfaceSeries::ST_LEVELS)
	{
		for (int i = 0; i < m_arDataPoints.GetSize(); i++)
		{
			CBCGPChartDataPoint* pDp = m_arDataPoints[i];
			if (pDp != NULL)
			{
				if (pDp->IsEmpty())
				{
					pDp->m_arScreenPoints.RemoveAll();
					continue;
				}

				if (pDp->m_arScreenPoints.GetSize() != 2)
				{
					pDp->m_arScreenPoints.SetSize(2);
				}

				BOOL bIsEmpty = FALSE;

				const CBCGPChartData& data = pDp->GetData();

				CBCGPPoint point;
				point.x = m_pXAxis->PointFromValueOptimized3D(data.m_arData[CBCGPChartData::CI_X], 
					ptXAxisStart, ptXAxisEnd, dblXAxisSize, 
					bIsDisplaydataBetweenTickMarksX, CBCGPChartData::CI_X, bIsLogScaleX);
				point.y = m_pYAxis->PointFromValueOptimized3D(data.m_arData[CBCGPChartData::CI_Y], 
					ptYAxisStart, ptYAxisEnd, dblYAxisSize, 
					bIsDisplaydataBetweenTickMarksY, CBCGPChartData::CI_Y, bIsLogScaleY);
				point.z = m_pZAxis->PointFromValueOptimized3D(data.m_arData[CBCGPChartData::CI_Z], 
					ptZAxisStart, ptZAxisEnd, dblZAxisSize, 
					bIsDisplaydataBetweenTickMarksZ, CBCGPChartData::CI_Z, bIsLogScaleZ);

				if (!bIsEmpty)
				{
					pDp->m_arScreenPoints[0] = point;
				}
				else
				{
					pDp->m_arScreenPoints.RemoveAll();
				}
			}
		}
	}

	m_arLevelYValues.RemoveAll();
	m_arLevelYValues.SetSize(m_nColorMapCount + 1);

	double dblMinValue;
	double dblMaxValue;

	GetLevelRange(dblMinValue, dblMaxValue);

	double dblLevelStep = (dblMaxValue - dblMinValue) / m_nColorMapCount;

	for (int nLevel = 0; nLevel <= m_nColorMapCount; nLevel++)
	{
		m_arLevelYValues[nLevel] = m_pYAxis->PointFromValueOptimized3D(dblMinValue + nLevel * dblLevelStep, 
					ptYAxisStart, ptYAxisEnd, dblYAxisSize, 
					bIsDisplaydataBetweenTickMarksY, CBCGPChartData::CI_Y, bIsLogScaleY);
	}


	CreateSurfaceTriangles((m_arLevelYValues[m_nColorMapCount] - m_arLevelYValues[0]) / m_nColorMapCount);
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::ClearSurfaceCells()
{
	m_arSurfaceTriangles.RemoveAll();
}
//****************************************************************************************
int __cdecl compare(const void* pPt1, const void* pPt2)
{
	CBCGPPoint* p1 = (CBCGPPoint*) pPt1;
	CBCGPPoint* p2 = (CBCGPPoint*) pPt2;

	return (int) (bcg_sign(p1->x * p2->y - p1->y * p2->x));
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::CreateSurfaceTriangles(double dblLevelStep)
{
	ASSERT_VALID(this);

	int nZCount = (int)m_arDataPoints.GetSize() / m_nXSize;
	int nSurfaceSize = (m_nXSize - 1) * (nZCount - 1);

	ClearSurfaceCells();

	if (nSurfaceSize <= 0)
	{
		return;
	}

	if (m_pChart == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pChart);

	CBCGPChartDiagram3D* pDiagram3D = m_pChart->GetDiagram3D();

	if (pDiagram3D == NULL)
	{
		return;
	}

	m_arSurfaceTriangles.SetSize(nSurfaceSize * 2);

	CBCGPPoint ptDiagramCenter = pDiagram3D->GetDiagramRect().CenterPoint();
	double dblDepthScalePercent = pDiagram3D->GetDepthScalePercent();

	int nCellCount = 0;
	int nTriangleCount = 0;

	double dblFlatLevel = m_pYAxis->PointFromValue(m_dblFlatLevel, FALSE);

	if (dblFlatLevel < -0.985)
	{
		dblFlatLevel = -0.985;
	}

	if (dblFlatLevel > 1.)
	{
		dblFlatLevel = 1.;
	}

	for (int i = 0; i < nZCount - 1; i++)
	{
		for (int j = 0; j < m_nXSize - 1; j++, nCellCount++, nTriangleCount += 2)
		{
			int nDataPointIndex = j + i * m_nXSize;
			int nNextDataPointIndex = j + (i + 1) * m_nXSize;

			CBCGPChartDataPoint* pDpLeftBottom = m_arDataPoints[nDataPointIndex];
			CBCGPChartDataPoint* pDpLeftTop = m_arDataPoints[nNextDataPointIndex];
			CBCGPChartDataPoint* pDpRightTop = m_arDataPoints[nNextDataPointIndex + 1];
			CBCGPChartDataPoint* pDpRightBottom = m_arDataPoints[nDataPointIndex + 1];

			CBCGPChartSurfaceTriangle& leftTriangle = m_arSurfaceTriangles[nTriangleCount];
			CBCGPChartSurfaceTriangle& rightTriangle = m_arSurfaceTriangles[nTriangleCount + 1];

			leftTriangle.m_nTriangleIndex = nTriangleCount;
			rightTriangle.m_nTriangleIndex = nTriangleCount + 1;

			if (pDpLeftBottom->m_arScreenPoints.GetSize() > 0 && pDpRightTop->m_arScreenPoints.GetSize() > 0 && 
				pDpLeftTop->m_arScreenPoints.GetSize() > 0)
			{
				leftTriangle.m_arPoints[0] = pDpLeftBottom->m_arScreenPoints[0];
				leftTriangle.m_arPoints[1] = pDpRightTop->m_arScreenPoints[0];
				leftTriangle.m_arPoints[2] = pDpLeftTop->m_arScreenPoints[0];
			}
			else
			{
				leftTriangle.m_bIsEmpty = TRUE;
			}

			if (pDpLeftBottom->m_arScreenPoints.GetSize() > 0 && pDpRightBottom->m_arScreenPoints.GetSize() > 0 && 
				pDpRightTop->m_arScreenPoints.GetSize() > 0)
			{
				rightTriangle.m_arPoints[0] = pDpLeftBottom->m_arScreenPoints[0];
				rightTriangle.m_arPoints[1] = pDpRightBottom->m_arScreenPoints[0];
				rightTriangle.m_arPoints[2] = pDpRightTop->m_arScreenPoints[0];
			}
			else
			{
				rightTriangle.m_bIsEmpty = TRUE;
			}

			leftTriangle.m_nDataPointIndex = nDataPointIndex;
			rightTriangle.m_nDataPointIndex = nDataPointIndex;
			rightTriangle.m_bLeft = FALSE;

			ProcessTriangle(leftTriangle, dblLevelStep, pDiagram3D, dblDepthScalePercent, ptDiagramCenter, dblFlatLevel);
			ProcessTriangle(rightTriangle, dblLevelStep, pDiagram3D, dblDepthScalePercent, ptDiagramCenter, dblFlatLevel);

			if (m_surfaceType == CBCGPChartSurfaceSeries::ST_STANDARD && 
				!leftTriangle.m_bIsEmpty && !rightTriangle.m_bIsEmpty)
			{
				rightTriangle.m_arLevels[0].m_levelAttribs = leftTriangle.m_arLevels[0].m_levelAttribs;
			}
		}
	}

	UpdateSurfaceTriangleColors();
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::ProcessTriangle(CBCGPChartSurfaceTriangle& triangle, double dblLevelStep, 
											  CBCGPChartDiagram3D* pDiagram3D, double dblDepthScalePercent,
											  const CBCGPPoint& ptDiagramCenter, double dblFlatLevel)
{
	if (triangle.m_bIsEmpty)
	{
		return;
	}

	double dblYMin = DBL_MAX;
	double dblYMax = -DBL_MAX;

	if (m_surfaceType == CBCGPChartSurfaceSeries::ST_STANDARD)
	{
		dblYMin = dblYMax = triangle.m_arPoints[0].y;
	}
	else
	{
		for (int k = 0; k < 3; k++)
		{
			dblYMin = min(dblYMin, triangle.m_arPoints[k].y);
			dblYMax = max(dblYMax, triangle.m_arPoints[k].y);
		}
	}
	

	int nMinLevel = (int)((dblYMin - m_arLevelYValues[0]) / dblLevelStep);
	int nMaxLevel = (int)((dblYMax - m_arLevelYValues[0]) / dblLevelStep);

	if (nMinLevel < 0)
	{
		nMinLevel = 0;
	}

	if (nMaxLevel < 0)
	{
		nMaxLevel = 0;
	}

	if (nMaxLevel > (int)m_arLevelYValues.GetSize() - 2)
	{
		nMaxLevel = (int)m_arLevelYValues.GetSize() - 2;
	}

	if (nMinLevel > (int)m_arLevelYValues.GetSize() - 2)
	{
		nMinLevel = (int)m_arLevelYValues.GetSize() - 2;
	}

	CBCGPRect rectBoundsRotated(DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX);
	triangle.m_dblMinZRotated = DBL_MAX;
	triangle.m_dblMaxZRotated = -DBL_MAX;

	for (int t = 0; t < 3; t++)
	{
		CBCGPPoint pt = pDiagram3D->TransformPoint(triangle.m_arPoints[t], ptDiagramCenter, FALSE);
		triangle.m_arPointsRotated[t] = pt;

		if (!m_bDrawFlat)
		{
 			triangle.m_arPointsTransformed[t] = pt;
 			pDiagram3D->ScalePoint(triangle.m_arPointsTransformed[t], ptDiagramCenter);
		
			rectBoundsRotated.left = min(rectBoundsRotated.left, pt.x);
			rectBoundsRotated.top = min(rectBoundsRotated.top, pt.y);
			rectBoundsRotated.right = max(rectBoundsRotated.right, pt.x);
			rectBoundsRotated.bottom = max(rectBoundsRotated.bottom, pt.y);

			triangle.m_dblMinZRotated = min(triangle.m_dblMinZRotated, pt.z);
			triangle.m_dblMaxZRotated = max(triangle.m_dblMaxZRotated, pt.z);
		}
	}

	if (nMinLevel == nMaxLevel || m_surfaceType == CBCGPChartSurfaceSeries::ST_STANDARD)
	{
		CBCGPChartSurfaceLevel level;
		level.m_levelAttribs.m_nLevelIndex = nMinLevel;
		level.m_arLevelPoints.SetSize(3);

		for (int i = 0; i < 3; i++)
		{
			level.m_arLevelPoints[i] = triangle.m_arPoints[i];
		}

		triangle.m_arLevels.Add(level);
	}
	else
	{
		for (int k = nMinLevel; k <= nMaxLevel; k++)
		{
			CBCGPChartSurfaceLevel level;
			level.m_levelAttribs.m_nLevelIndex = k;

			double dblLevelY = m_arLevelYValues[k] + dblLevelStep;
			double dblLevelPrev = k == 0 ? dblLevelY : m_arLevelYValues[k - 1] + dblLevelStep;

			if (k == nMinLevel)
			{
				for (int nPoint = 0; nPoint < 3; nPoint++)
				{
					CBCGPPoint ptNext = triangle.m_arPoints[nPoint];

					if (ptNext.y < dblLevelY)
					{
						level.m_arLevelPoints.Add(ptNext);
					}
				}
			}
			else
			{
				const CBCGPChartSurfaceLevel& levelPrev = triangle.m_arLevels[k - nMinLevel - 1];
				level.m_arLevelPoints.Append(levelPrev.m_arIntersect);
				
				for (int nPoint = 0; nPoint < 3; nPoint++)
				{
					CBCGPPoint ptNext = triangle.m_arPoints[nPoint];
					
					if (ptNext.y < dblLevelY && ptNext.y > dblLevelPrev)
					{
						level.m_arLevelPoints.Add(ptNext);
					}
				}
			}

			for (int n = 0; n < 3; n++)
			{
				CBCGPPoint pt1 = triangle.m_arPoints[n];
				CBCGPPoint pt2 = triangle.m_arPoints[(n + 1) % 3];

				if (pt1.y > dblLevelY && pt2.y > dblLevelY || 
					pt1.y < dblLevelY && pt2.y < dblLevelY)
				{
					continue;
				}

		 		if (pt2.y == pt1.y && pt1.y == dblLevelY)
		 		{
		 			level.m_arLevelPoints.Add(pt1);
		 			level.m_arLevelPoints.Add(pt2);

					level.m_arIntersect.Add(pt1);
					level.m_arIntersect.Add(pt2);
		 		}
		 		else
				{
					double t = (dblLevelY - pt1.y) / (pt2.y - pt1.y);
					double x = t * (pt2.x - pt1.x) + pt1.x;
					double z = t * (pt2.z - pt1.z) + pt1.z;

					CBCGPPoint ptIntersect = CBCGPPoint(x, dblLevelY, z);
					level.m_arIntersect.Add(ptIntersect);
					level.m_arLevelPoints.Add(ptIntersect);
				}
			}

			triangle.m_arLevels.Add(level);
		}
	}

	for (int nLevel = 0; nLevel < triangle.m_arLevels.GetSize(); nLevel++)
	{
		CBCGPChartSurfaceLevel& level = triangle.m_arLevels[nLevel];
		int nSize = (int)level.m_arLevelPoints.GetSize();

		for (int nPoint = 0; nPoint < nSize; nPoint++)
		{
			if (m_bDrawFlat)
			{
				level.m_arLevelPoints[nPoint].y = dblFlatLevel;
			}
			pDiagram3D->TransformPointOpt(level.m_arLevelPoints[nPoint], dblDepthScalePercent, ptDiagramCenter);
		}

		int i;
		int nTmpCount = 0;

		for (i = 0; i < nSize; i++)
		{
			CBCGPPoint ptNext = level.m_arLevelPoints[i];

			BOOL bFound = FALSE;
			static const double dblLinePrecision = 4 * FLT_EPSILON;
			for (int j = 0; j < nTmpCount; j++)
			{
				if (fabs(ptNext.x - m_arTmpPoints[j].x) < dblLinePrecision && 
					fabs(ptNext.y - m_arTmpPoints[j].y) < dblLinePrecision &&
					fabs(ptNext.z - m_arTmpPoints[j].z) < dblLinePrecision)
				{
					bFound = TRUE;
					break;
				}
			}

			if (!bFound)
			{
				m_arTmpPoints[nTmpCount] = ptNext;
				nTmpCount++;
			}
		}

		for (i = 0; i < nTmpCount; i++)
		{
			level.m_arLevelPoints[i] = m_arTmpPoints[i];
		}

		level.m_arLevelPoints.SetSize(nTmpCount);
		nSize = nTmpCount;

		if (nSize > 3)
		{
			int z;
			CBCGPPoint ptBegin;

			for (z = 0; z < nSize; z++)
			{
				ptBegin.x += level.m_arLevelPoints[z].x;
				ptBegin.y += level.m_arLevelPoints[z].y;
			}

			ptBegin.x /= nSize;
			ptBegin.y /= nSize;

			for (z = 0; z < nSize; z++)
			{
				level.m_arLevelPoints[z].x = level.m_arLevelPoints[z].x - ptBegin.x;
				level.m_arLevelPoints[z].y = level.m_arLevelPoints[z].y - ptBegin.y;	
			}

			qsort(level.m_arLevelPoints.GetData(), nSize, sizeof(CBCGPPoint), compare);

			for (z = 0; z < nSize; z++)
			{
				level.m_arLevelPoints[z].x = level.m_arLevelPoints[z].x + ptBegin.x;
				level.m_arLevelPoints[z].y = level.m_arLevelPoints[z].y + ptBegin.y;
			}
		}
	}

	if (m_bDrawFlat)
	{
		for (int n = 0; n < 3; n++)
		{
			triangle.m_arPointsRotated[n] = triangle.m_arPoints[n];
			triangle.m_arPointsRotated[n].y = dblFlatLevel;

			triangle.m_arPointsTransformed[n] = triangle.m_arPoints[n];
			triangle.m_arPointsTransformed[n].y = dblFlatLevel;

			pDiagram3D->TransformPointOpt(triangle.m_arPointsRotated[n], dblDepthScalePercent, ptDiagramCenter, FALSE);
			pDiagram3D->TransformPointOpt(triangle.m_arPointsTransformed[n], dblDepthScalePercent, ptDiagramCenter, TRUE);

			CBCGPPoint pt = triangle.m_arPointsRotated[n];

			rectBoundsRotated.left = min(rectBoundsRotated.left, pt.x);
			rectBoundsRotated.top = min(rectBoundsRotated.top, pt.y);
			rectBoundsRotated.right = max(rectBoundsRotated.right, pt.x);
			rectBoundsRotated.bottom = max(rectBoundsRotated.bottom, pt.y);

			triangle.m_dblMinZRotated = min(triangle.m_dblMinZRotated, pt.z);
			triangle.m_dblMaxZRotated = max(triangle.m_dblMaxZRotated, pt.z);
		}
	}

	rectBoundsRotated.Normalize();
	triangle.m_rectBoundsRotated = rectBoundsRotated;

	triangle.m_vPlane.CalcPlane(triangle.m_arPointsRotated);
	triangle.m_bBack = triangle.m_vPlane[2] < -DBL_EPSILON;

	if (pDiagram3D->IsCalculateNormals())
	{
		if (triangle.m_bBack)
		{
			triangle.m_vNormal.CalcNormal(triangle.m_arPointsTransformed[2], 
											triangle.m_arPointsTransformed[1], 
											triangle.m_arPointsTransformed[0]);
		}
		else
		{
			triangle.m_vNormal.CalcNormal(triangle.m_arPointsTransformed[0], 
											triangle.m_arPointsTransformed[1], 
											triangle.m_arPointsTransformed[2]);
		}
	}

	triangle.CalcEdgeCoefs();
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::UpdateSurfaceTriangleColors()
{
	for (int nTriangle = 0; nTriangle < m_arSurfaceTriangles.GetSize(); nTriangle++)
	{
		CBCGPChartSurfaceTriangle& triangle = m_arSurfaceTriangles[nTriangle];

		for (int j = 0; j < triangle.m_arLevels.GetSize(); j++)
		{
			CBCGPChartSurfaceLevel& level = triangle.m_arLevels[j];

			int nIndex = level.m_levelAttribs.m_nLevelIndex;

			level.m_levelAttribs.m_pBrFill = GetLevelFillBrush(nIndex);
			level.m_levelAttribs.m_pBrLine = GetLevelLineBrush(nIndex);

			level.m_levelAttribs.m_pBrFillBack = GetLevelFillBackBrush(nIndex);
			level.m_levelAttribs.m_pBrLineBack = GetLevelLineBackBrush(nIndex);
		}
	}
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::SetLevelRangeMode(LevelRangeMode rangeMode, double dblMinCustomRangeValue, double dblMaxCustomRangeValue)
{
	m_levelRangeMode = rangeMode;

	if (m_levelRangeMode == CBCGPChartSurfaceSeries::LRM_CUSTOM)
	{
		m_dblMinCustomLevelRangeVal = min (GetMinValue(CBCGPChartData::CI_Y), dblMinCustomRangeValue);
		m_dblMaxCustomLevelRangeVal = max (GetMaxValue(CBCGPChartData::CI_Y), dblMaxCustomRangeValue);
	}
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::SetDrawFlat(BOOL bSet, double dblFlatLevel)
{
	m_bDrawFlat = bSet;
	m_dblFlatLevel = dblFlatLevel;
	
	if (m_pChart != NULL)
	{
		m_pChart->GetDiagram3D()->SetSortNeeded(TRUE);
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::GetLevelRange(double& dblMinValue, double& dblMaxValue) 
{
	dblMinValue = GetMinValue(CBCGPChartData::CI_Y);
	dblMaxValue = GetMaxValue(CBCGPChartData::CI_Y);

	switch(m_levelRangeMode)
	{
	case CBCGPChartSurfaceSeries::LRM_MINMAX_SERIES:
		break;

	case CBCGPChartSurfaceSeries::LRM_MINMAX_Y_AXIS:
		if (m_pYAxis != NULL)
		{
			ASSERT_VALID(m_pYAxis);
			dblMinValue = m_pYAxis->GetMinDisplayedValue();
			dblMaxValue = m_pYAxis->GetMaxDisplayedValue();
		}
		break;

	case CBCGPChartSurfaceSeries::LRM_CUSTOM:
		dblMinValue = m_dblMinCustomLevelRangeVal;
		dblMaxValue = m_dblMaxCustomLevelRangeVal;
		break;
	}
}
//****************************************************************************************
int CBCGPChartSurfaceSeries::GetBrushIndex(int nDataPointIndex)
{
	ASSERT_VALID(this);

	CBCGPChartValue val = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y);

	double dblMinValue;
	double dblMaxValue;

	GetLevelRange(dblMinValue, dblMaxValue);

	return (int) ((val.GetValue() -  dblMinValue) / (dblMaxValue - dblMinValue) * m_nColorMapCount);
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::LevelIndexToString(int nLevelIndex, CString& strValues)
{
	if (GetLevelCount() == 0)
	{
		return;
	}

	double dblMinValue;
	double dblMaxValue;

	GetLevelRange(dblMinValue, dblMaxValue);

	double dblLevelSize = (dblMaxValue - dblMinValue) / GetLevelCount();

	double dblFloorValue = nLevelIndex * dblLevelSize + dblMinValue;
	double dblCeilValue = dblFloorValue + dblLevelSize;

	if (fabs(dblFloorValue) < 1. || fabs(dblCeilValue) < 1.)
	{
		double dblOrderFloor = log10(fabs(dblFloorValue));
		double dblOrderCeil = log10(fabs(dblCeilValue));
		double dblOrderSize = log10(fabs(dblLevelSize));

		if (fabs(fabs(dblOrderFloor) - fabs(dblOrderSize)) > 2.)
		{
			dblFloorValue = 0;
		}

		if (fabs(fabs(dblOrderCeil) - fabs(dblOrderSize)) > 2.)
		{
			dblCeilValue = 0;
		}
	}

	if (m_bLevelRangleInLabel)
	{
		CString strFormat;
		strFormat.Format(_T("%s - %s"), m_strLegendLevelValueFormat, m_strLegendLevelValueFormat);
		strValues.Format(strFormat, dblFloorValue, dblCeilValue);
	}
	else
	{
		strValues.Format(m_strLegendLevelValueFormat, dblFloorValue);
	}
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::EnableLevelRangeInLegendLabel(BOOL bEnable)
{
	m_bLevelRangleInLabel = bEnable;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//****************************************************************************************
CBCGPBrush* CBCGPChartSurfaceSeries::GetLevelFillBrush(int nIndex)
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= m_arFillBrushes.GetSize())
	{
		return NULL;
	}

	return m_arFillBrushes[nIndex];
}
//****************************************************************************************
CBCGPBrush* CBCGPChartSurfaceSeries::GetLevelFillBackBrush(int nIndex)
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= m_arFillBackBrushes.GetSize())
	{
		return NULL;
	}

	return m_arFillBackBrushes[nIndex];
}
//****************************************************************************************
CBCGPBrush* CBCGPChartSurfaceSeries::GetLevelLineBrush(int nIndex)
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= m_arLineBrushes.GetSize())
	{
		return NULL;
	}

	return m_arLineBrushes[nIndex];
}
//****************************************************************************************
CBCGPBrush* CBCGPChartSurfaceSeries::GetLevelLineBackBrush(int nIndex)
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= m_arLineBackBrushes.GetSize())
	{
		return NULL;
	}

	return m_arLineBackBrushes[nIndex];
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::InitLegendElements()
{
	ASSERT_VALID(this);

	m_arLegendKeyRects.RemoveAll();
	m_arLegendLabelRects.RemoveAll();
	m_arLegendLabels.RemoveAll();
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::AddLegendElement(const CBCGPRect& rectKey, const CBCGPRect& rectLabel, const CString& strLabel)
{
	ASSERT_VALID(this);

	m_arLegendKeyRects.Add(rectKey);
	m_arLegendLabelRects.Add(rectLabel);
	m_arLegendLabels.Add(strLabel);
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::GetLegendElementRects(int nIndex, CBCGPRect& rectKey, CBCGPRect& rectLabel)
{
	ASSERT_VALID(this);	

	if (nIndex < 0 || nIndex >= m_arLegendKeyRects.GetSize())
	{
		return;
	}

	rectKey = m_arLegendKeyRects[nIndex];
	rectLabel = m_arLegendLabelRects[nIndex];
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::GetLegendElementLabel(int nIndex, CString& strLabel)
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= m_arLegendKeyRects.GetSize())
	{
		return;
	}

	strLabel = m_arLegendLabels[nIndex];
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::SetLegendElementRects(int nIndex, const CBCGPRect& rectKey, const CBCGPRect& rectLabel)
{
	ASSERT_VALID(this);	

	if (nIndex < 0 || nIndex >= m_arLegendKeyRects.GetSize())
	{
		return;
	}

	m_arLegendKeyRects[nIndex] = rectKey;
	m_arLegendLabelRects[nIndex] = rectLabel;
}
//****************************************************************************************
int CBCGPChartSurfaceSeries::GetLegendElementCount() const
{
	ASSERT_VALID(this);

	if (m_pChart != NULL && m_pChart->IsShowSurfaceMapInLegend())
	{
		return (int)m_arLegendKeyRects.GetSize();
	}

	return 1;
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::SetContinuousLegendKey(BOOL bSet)
{
	ASSERT_VALID(this);

	m_bContinuousLegendKey = bSet;
	m_pChart->SetDirty(TRUE);
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::UpdateSeriesColors()
{
	ASSERT_VALID(this);

	BCGPSeriesColorsPtr colors;
	const BCGPChartFormatSeries* pFormatSeries = CBCGPChartSeries::GetColors(colors, -1);
	if (pFormatSeries == NULL || colors.m_pBrElementFillColor == NULL || colors.m_pBrElementLineColor == NULL)
	{
		return;
	}

	const CBCGPChartTheme& theme = m_pChart->GetColors();

	CleanBrushes();

	CArray<CBCGPColor, const CBCGPColor&> arClrFill;
	CArray<CBCGPColor, const CBCGPColor&> arClrLine;

	if (m_arCustomSurfaceColors.GetSize() == 0 || m_colorMode != CBCGPChartSurfaceSeries::CM_CUSTOM)
	{
		for (int j = 0; j < BCGP_CHART_NUM_SERIES_COLORS_IN_THEME; j++)
		{
			BCGPSeriesColorsPtr seriesColors;
 			theme.GetSeriesColors(seriesColors, j, CBCGPBrush::BCGP_NO_GRADIENT);

			arClrFill.Add(seriesColors.m_pBrElementFillColor->GetColor());
			arClrLine.Add(seriesColors.m_pBrElementLineColor->GetColor());
		}
	}
	else
	{
		arClrFill.Append(m_arCustomSurfaceColors);

		for (int i = 0; i < m_arCustomSurfaceColors.GetSize(); i++)
		{
			CBCGPColor clr = m_arCustomSurfaceColors[i];
			clr.MakeDarker();
			arClrLine.Add(clr);
		}
	}

	GenerateSurfaceColors(arClrFill, m_dblSurfaceOpacity, TRUE);
	GenerateSurfaceColors(arClrLine, m_bEnableFrameTransparency ? m_dblSurfaceOpacity : 1., FALSE);

	UpdateSurfaceTriangleColors();
}
//****************************************************************************************
void CBCGPChartSurfaceSeries::GenerateSurfaceColors(const CArray<CBCGPColor, const CBCGPColor&>& arColors, double dblOpacity,
													BOOL bFill)
{
	int nSize = (int)arColors.GetSize();

	double dblHue = 0;
	double dblSat = 0;
	double dblLum = 0;

	// for custom color mode use algorithm for multiple colors;
	CBCGPChartSurfaceSeries::ColorMode colorMode = m_colorMode == CBCGPChartSurfaceSeries::CM_CUSTOM ? 
		CBCGPChartSurfaceSeries::CM_MULTIPLE : m_colorMode;

	for (int j = 0; j < m_nColorMapCount + 1; j++)
	{
 		CBCGPBrush* pBr = NULL;

		switch (colorMode)
		{
		case CM_SINGLE:
			if (nSize > 0)
			{
				CBCGPDrawManager::RGBtoHSL (arColors[0], &dblHue, &dblSat, &dblLum);

				dblLum = (double) j / (double)m_nColorMapCount;
				COLORREF clr = CBCGPDrawManager::HLStoRGB_ONE(dblHue, bFill ? dblLum : dblLum * .5, dblSat);

 				pBr = new CBCGPBrush(CBCGPColor(clr), dblOpacity);
				pBr->MakeLighter();
			}
			break;

		case CM_MULTIPLE:
			if (nSize >= m_nColorMapCount)
			{
 				pBr = new CBCGPBrush(arColors[j], dblOpacity);
			}
			else if (nSize > 0)
			{
				int nGroupSize = m_nColorMapCount / nSize;
				int nIndex = min (nSize - 1, j / nGroupSize);

				if (nIndex == nSize - 1)
				{
					nGroupSize = max(nGroupSize, m_nColorMapCount - nIndex * nGroupSize);
				}

				int nLevel = j % nGroupSize;

				CBCGPColor color = arColors[nIndex];
				
				if (nLevel < nGroupSize / 2)
				{
					while (nLevel++ < nGroupSize / 2)
					{
						color.MakeDarker();
					}
				}
				else if (nLevel > nGroupSize / 2)
				{
					while (nLevel++ < nGroupSize)
					{
						color.MakeLighter();
					}
				}

 				pBr = new CBCGPBrush(color, dblOpacity);
			}
			break;;

		case CM_PALETTE:
			{
				dblHue = (double) (m_nColorMapCount - j - 1) / (double)(m_nColorMapCount - 1.);
				dblLum = .5;
				dblSat = .75;

				COLORREF clr = CBCGPDrawManager::HLStoRGB_ONE(dblHue, bFill ? dblLum : dblLum * .8, dblSat);

 				pBr = new CBCGPBrush(CBCGPColor(clr), dblOpacity);
			}
			break;
		}

		if (pBr != NULL)
		{
			CBCGPBrush* pBrBack = new CBCGPBrush(*pBr);
			pBrBack->MakeDarker(0.25);

			if (bFill)
			{
				m_arFillBrushes.Add(pBr);
				m_arFillBackBrushes.Add(pBrBack);
			}
			else
			{
				m_arLineBrushes.Add(pBr);
				m_arLineBackBrushes.Add(pBrBack);
			}
		}
	}
}
//****************************************************************************************
// CBCGPChartLongSeries - specific LARGE DATA
//****************************************************************************************
CBCGPChartLongSeries::CBCGPChartLongSeries(CBCGPChartVisualObject* pChartCtrl, int nDataCount, int nGrowBy) : 
					CBCGPChartSeries(pChartCtrl, BCGPChartLongData)
{
	m_arXValues.SetSize(nDataCount, nGrowBy);
	m_arYValues.SetSize(nDataCount, nGrowBy);
	m_nCurrIndex = 0;

	m_bFilterSimilarXValues = FALSE;
	m_bIsScatterMode = FALSE;
	m_dblScatterPointSize = 0.0;
}
//****************************************************************************************
void CBCGPChartLongSeries::SetScatterMode(BOOL bSet/* = TRUE*/, double dblSize/* = 2.0*/)
{
	m_bIsScatterMode = bSet;
	m_dblScatterPointSize = dblSize;
	m_bFilterSimilarXValues = TRUE;
}
//****************************************************************************************
void CBCGPChartLongSeries::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld)
{
	m_formatSeries.OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);
}
//****************************************************************************************
void CBCGPChartLongSeries::SetSeriesData(double dblY, double dblX, int nIndex)
{
	ASSERT_VALID(this);

	m_arXValues.SetAtGrow(nIndex, dblX);
	m_arYValues.SetAtGrow(nIndex, dblY);

	if (nIndex >= m_nCurrIndex)
	{
		m_nCurrIndex = nIndex + 1;
	}
	
	SetMinMaxValuesSimple(dblX, CBCGPChartData::CI_X, nIndex);
	SetMinMaxValuesSimple(dblY, CBCGPChartData::CI_Y, nIndex);
}
//****************************************************************************************
int CBCGPChartLongSeries::AddDataPoint(CBCGPChartDataPoint* pDataPoint)
{
	ASSERT_VALID(this);
	ASSERT(pDataPoint != NULL);

	CBCGPChartValue valX = pDataPoint->GetComponentValue(CBCGPChartData::CI_X);
	CBCGPChartValue valY = pDataPoint->GetComponentValue(CBCGPChartData::CI_Y);

	if (valX.IsEmpty())
	{
		m_arXValues.SetAtGrow(m_nCurrIndex, m_nCurrIndex);
	}
	else
	{
		m_arXValues.SetAtGrow(m_nCurrIndex, valX);
	}
	m_arYValues.SetAtGrow(m_nCurrIndex, valY);
	
	SetMinMaxValuesSimple(valX, CBCGPChartData::CI_X, m_nCurrIndex);
	SetMinMaxValuesSimple(valY, CBCGPChartData::CI_Y, m_nCurrIndex);

	m_nCurrIndex++;

	return m_nCurrIndex - 1;
}
//****************************************************************************************
int CBCGPChartLongSeries::AddDataPoint(const CString& /*strCategoryName*/, double dblY, BCGPChartFormatSeries* pDataPointFormat, DWORD_PTR dwUserData)
{
	return AddDataPoint(dblY, pDataPointFormat, dwUserData);
}
//****************************************************************************************
int CBCGPChartLongSeries::AddDataPoint(double dblY, BCGPChartFormatSeries* pDataPointFormat, DWORD_PTR dwUserData)
{
	ASSERT_VALID(this);

	UNREFERENCED_PARAMETER(pDataPointFormat);
	UNREFERENCED_PARAMETER(dwUserData);

	m_dpCurrent.SetComponentValue(dblY, CBCGPChartData::CI_Y);
	m_dpCurrent.SetComponentValue(m_nCurrIndex, CBCGPChartData::CI_X);

	return AddDataPoint(&m_dpCurrent);
}
//****************************************************************************************
void CBCGPChartLongSeries::AddDataPoints(const CBCGPDoubleArray& arYValues, CBCGPDoubleArray* pXValues, CBCGPDoubleArray* pY1Values, BOOL bRecalcMinMaxValues)
{
	ASSERT_VALID(this);
	UNREFERENCED_PARAMETER(pY1Values);
	UNREFERENCED_PARAMETER(bRecalcMinMaxValues);

	int nYSize = (int)arYValues.GetSize();
	
	if (nYSize == 0)
	{
		return;
	}
	
	int nXSize = (int)(pXValues == NULL ? 0 : pXValues->GetSize());
	
	for (int i = 0; i < nYSize; i++)
	{
		double dblX = m_nCurrIndex; 
		double dblY = arYValues[i];
		if (i < nXSize)
		{
			dblX = pXValues->GetAt(i);
		}
		
		m_arXValues.SetAtGrow(m_nCurrIndex, dblX);
		m_arYValues.SetAtGrow(m_nCurrIndex, dblY);

 		SetMinMaxValuesSimple(dblX, CBCGPChartData::CI_X, m_nCurrIndex);
 		SetMinMaxValuesSimple(dblY, CBCGPChartData::CI_Y, m_nCurrIndex);

		m_nCurrIndex++;
	}
}
//****************************************************************************************
BOOL CBCGPChartLongSeries::SetDataPointValue(int nDataPointIndex, double dblValue, CBCGPChartData::ComponentIndex ci)
{
	ASSERT_VALID(this);
	if (nDataPointIndex < 0)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (ci == CBCGPChartData::CI_X)
	{
		m_arXValues.SetAtGrow(nDataPointIndex, dblValue);
	}
	else if (ci == CBCGPChartData::CI_Y)
	{
		m_arYValues.SetAtGrow(nDataPointIndex, dblValue);
	}
	else
	{
		return FALSE;
	}

	if (nDataPointIndex >= m_nCurrIndex)
	{
		m_nCurrIndex = nDataPointIndex + 1;
	}

	SetMinMaxValuesSimple(dblValue, ci, nDataPointIndex);
	return TRUE;
}
//****************************************************************************************
void CBCGPChartLongSeries::RemoveDataPoints(int nStartIdx, int nCount, BOOL bFreeExtra)
{
	ASSERT_VALID(this);

	UNREFERENCED_PARAMETER(bFreeExtra);

	if (nStartIdx < 0 || nStartIdx >= m_nCurrIndex)
	{
		ASSERT(FALSE);
		return;
	}

	if (nStartIdx + nCount > m_nCurrIndex)
	{
		nCount = m_nCurrIndex - nStartIdx;
	}

	m_arXValues.RemoveAt(nStartIdx, nCount);
	m_arYValues.RemoveAt(nStartIdx, nCount);

	m_nCurrIndex -= nCount;
}
//****************************************************************************************
void CBCGPChartLongSeries::MoveDataPoints(int nFromIdx, int nToIdx, int nCount)
{
	ASSERT_VALID(this);

	int nDataPointCount = GetDataPointCount();

	if (nFromIdx < 0 || nFromIdx >= nDataPointCount || nToIdx < 0)
	{
		ASSERT(FALSE);
		return;
	}

	CArray<double, double> arXTmp;
	CArray<double, double> arYTmp;

	nCount = min(GetDataPointCount() - nFromIdx, nCount);

	arXTmp.SetSize(nCount);
	arYTmp.SetSize(nCount);

	int n = 0;
	int i = nFromIdx;
	for (; i < nDataPointCount && n < nCount; i++, n++)
	{
		arXTmp.SetAt(n, m_arXValues[i]);
		arYTmp.SetAt(n, m_arYValues[i]);
	}

	for (i = nToIdx, n = 0; n < nCount; i++, n++)
	{
		double dblX = arXTmp[n];
		double dblY = arYTmp[n];

		m_arXValues.SetAtGrow (i, dblX);
		m_arYValues.SetAtGrow (i, dblY);
	}

	if (i > m_nCurrIndex)
	{
		m_nCurrIndex = i;
	}
}
//****************************************************************************************
CBCGPChartValue CBCGPChartLongSeries::GetDataPointValue(int nDataPointIndex, CBCGPChartData::ComponentIndex ci) const
{
	ASSERT_VALID(this);

	CBCGPChartValue val;

	if (nDataPointIndex < 0 || nDataPointIndex >= m_nCurrIndex)
	{
		ASSERT(FALSE);
		return val;
	}

	if (ci == CBCGPChartData::CI_X)
	{
		val.SetValue(m_arXValues[nDataPointIndex]);
	}
	else if (ci == CBCGPChartData::CI_Y)
	{
		val.SetValue(m_arYValues[nDataPointIndex]);
	}

	return val;
}
//****************************************************************************************
const CBCGPChartDataPoint* CBCGPChartLongSeries::GetDataPointAt(int nIndex) const
{
	ASSERT_VALID(this);

	if (nIndex < 0 || nIndex >= m_arYValues.GetSize())
	{
		ASSERT(FALSE);
		return NULL;
	}

	CBCGPChartDataPoint& dp = (CBCGPChartDataPoint&) m_dpCurrent;
	dp.SetComponentValue(m_arYValues[nIndex], CBCGPChartData::CI_Y);
	dp.SetComponentValue(m_arXValues[nIndex], CBCGPChartData::CI_X);

	return &m_dpCurrent;
}
//****************************************************************************************
void CBCGPChartLongSeries::SetDataPointCount(int nCount)
{
	ASSERT_VALID(this);

	if (nCount < 1)
	{
		return;
	}

	m_nCurrIndex = nCount;

	if (m_nCurrIndex > m_arXValues.GetSize())
	{
		m_arXValues.SetSize(nCount);
		m_arYValues.SetSize(nCount);
	}
}
//****************************************************************************************
BOOL CBCGPChartLongSeries::GetNearestScreenPoint(const CBCGPPoint& ptClient, CBCGPPoint& ptScreen,
												 double& dblX, double& dblY) const
{
	ASSERT_VALID(this);

	if (m_pXAxis == NULL || m_pYAxis == NULL || m_arScreenPoints.GetSize() < 2)
	{
		return FALSE;
	}

	if (m_bIsScatterMode)
	{
		double dblDist = bcg_sqr(m_arScreenPoints[0].x - ptClient.x) + bcg_sqr(m_arScreenPoints[0].y - ptClient.y);
		ptScreen = m_arScreenPoints[0];

		for (int i = 1; i < m_arScreenPoints.GetSize(); i++)
		{
			double dblDistCurr = bcg_sqr(m_arScreenPoints[i].x - ptClient.x) + bcg_sqr(m_arScreenPoints[i].y - ptClient.y);
			if (dblDistCurr < dblDist)
			{
				dblDist = dblDistCurr;
				ptScreen = m_arScreenPoints[i];
			}
		}
	}
	else
	{
		BOOL bFound = FALSE;
		BOOL bIsVertical = m_pXAxis->IsVertical();

		double dblClient = bIsVertical ? ptClient.y : ptClient.x;

		int i = 0;
		for (i = 0; i < m_arScreenPoints.GetSize() - 1; i++)
		{
			const CBCGPPoint& ptCurr = m_arScreenPoints[i];
			const CBCGPPoint& ptNext = m_arScreenPoints[i + 1];

			double dblCurr = bIsVertical ? ptCurr.y : ptCurr.x;
			double dblNext = bIsVertical ? ptNext.y : ptNext.x;

			if (dblClient <= dblCurr)
			{
				ptScreen = ptCurr;
				bFound = TRUE;
				break;
			}

			if (dblClient > dblCurr && dblClient < dblNext)
			{
				if (fabs(dblClient - dblCurr) < fabs(dblClient - dblNext))
				{
					ptScreen = ptCurr;
					bFound = TRUE;
					break;
				}
				else
				{
					ptScreen = ptNext;
					bFound = TRUE;
					break;
				}
			}
		}

		if (!bFound)
		{
			ptScreen = m_arScreenPoints[i];
		}
	}

	dblX = m_pXAxis->ValueFromPoint(ptScreen);
	dblY = m_pYAxis->ValueFromPoint(ptScreen);

	return TRUE;
}
//****************************************************************************************
void CBCGPChartLongSeries::OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea)
{
	ASSERT_VALID(this);
	ASSERT(m_pXAxis != NULL && m_pYAxis != NULL);

	UNREFERENCED_PARAMETER(pGM);
	UNREFERENCED_PARAMETER(rectDiagramArea);

	int nDataPointCount = GetDataPointCount();

	m_arScreenPoints.RemoveAll();
	m_arScreenPoints.SetSize(nDataPointCount);

	CBCGPPoint pt;
	CBCGPPoint ptPrev;
	int nIdx = 0;

	CBCGPChartValue valMaxPrev;
	CBCGPChartValue valMinPrev;

	BOOL bIsVertical = m_pXAxis->IsVertical();

	if (m_nMaxDisplayedXIndex == 0)
	{
		m_nMaxDisplayedXIndex = nDataPointCount - 1;
	}

	if (m_pXAxis->IsFixedMajorUnit())
	{
		m_nMinDisplayedXIndex = (int)max(0, m_pXAxis->GetMinDisplayedValue() - 1);
		m_nMaxDisplayedXIndex = (int)min(nDataPointCount - 1, m_pXAxis->GetMaxDisplayedValue());
	}

	if ((m_nMaxDisplayedXIndex - m_nMinDisplayedXIndex) < 3 && nDataPointCount > 3)
	{
		m_nMinDisplayedXIndex = max(0, m_nMinDisplayedXIndex - 1);
		m_nMaxDisplayedXIndex = min (nDataPointCount - 1, m_nMaxDisplayedXIndex + 1);
	}

	for (int i = m_nMinDisplayedXIndex; i <= m_nMaxDisplayedXIndex; i++)
	{
		if (bIsVertical)
		{
			pt.y = (int)m_pXAxis->PointFromValue(m_arXValues[i], FALSE);
			pt.x = (int)m_pYAxis->PointFromValue(m_arYValues[i], FALSE);
		}
		else
		{
			pt.x = (int)m_pXAxis->PointFromValue(m_arXValues[i], FALSE);
			pt.y = (int)m_pYAxis->PointFromValue(m_arYValues[i], FALSE);
		}
		
		if ((bIsVertical && ptPrev.y != pt.y || !bIsVertical && ptPrev.x != pt.x) && i > m_nMinDisplayedXIndex)
		{
			if (m_bFilterSimilarXValues)
			{
				if (!valMaxPrev.IsEmpty())
				{
					double dblAverage = valMinPrev + (valMaxPrev - valMinPrev) / 2;
					bIsVertical ? ptPrev.x = dblAverage : ptPrev.y = dblAverage;
					
				}
				
				m_arScreenPoints[nIdx++] = ptPrev;
			}
			else
			{
				if (!valMaxPrev.IsEmpty() && valMaxPrev != valMinPrev)
				{
					if (nIdx < GetDataPointCount() - 2)
					{
						bIsVertical ? ptPrev.x = valMaxPrev : ptPrev.y = valMaxPrev;
						m_arScreenPoints[nIdx++] = ptPrev;
						bIsVertical ? ptPrev.x = valMinPrev : ptPrev.y = valMinPrev;
						m_arScreenPoints[nIdx++] = ptPrev;
					}
					else
					{
						break;
					}
				}
				else
				{
					m_arScreenPoints[nIdx++] = ptPrev;
				}
			}

			valMaxPrev.SetEmpty();
			valMinPrev.SetEmpty();
		}
		else
		{
			if (valMaxPrev.IsEmpty())
			{
				valMaxPrev = bIsVertical ? pt.x : pt.y;
				valMinPrev = bIsVertical ? pt.x : pt.y;
			}
			else
			{
				valMaxPrev = bIsVertical ? max(pt.x, valMaxPrev) : max(pt.y, valMaxPrev);
				valMinPrev = bIsVertical ? min(pt.x, valMinPrev) : min(pt.y, valMinPrev);			
			}
		}

		ptPrev = pt;
	}

	if ((bIsVertical && ptPrev.y != 0 || !bIsVertical && ptPrev.x != 0) && nIdx < nDataPointCount)
	{
		m_arScreenPoints[nIdx++] = ptPrev;
	}

	m_arScreenPoints.RemoveAt(nIdx, GetDataPointCount() - nIdx);
	m_arScreenPoints.FreeExtra();
}
//****************************************************************************************
// CBCGPChartSeries - specific HISTORICAL LINE
//****************************************************************************************
CBCGPChartHistoricalLineSeries::CBCGPChartHistoricalLineSeries(CBCGPChartVisualObject* pRelatedChart, int nHistoryDepth, double dblDefaultY) : 
		CBCGPChartLongSeries(pRelatedChart, nHistoryDepth, -1)
{
	SetHistoryDepth(nHistoryDepth, TRUE, dblDefaultY);
	CommonInit();
}
//****************************************************************************************
void CBCGPChartHistoricalLineSeries::CommonInit()
{
	m_bUpdateAxesOnNewData = TRUE;

	m_dblMajorXUnit = 1.;
	m_dblLastInitY = 0.;
	m_bLastIsInitY = TRUE;
}
//****************************************************************************************
void CBCGPChartHistoricalLineSeries::SetHistoryDepth(int nDepth, BOOL bInitY, double dblDefaultY)
{
	ASSERT_VALID(this);

	m_arYValues.SetSize(nDepth);
	m_arXValues.SetSize(nDepth);

	m_arYValues.FreeExtra();
	m_arXValues.FreeExtra();
	
	m_nHistoryDepth = nDepth;

	for (int i = 0; i < m_arXValues.GetSize(); i++)
	{
		m_arXValues[i] = i;
		if (bInitY)
		{
			m_arYValues[i] = dblDefaultY;
		}
	}

	m_nCurrIndex = nDepth;
	m_dblLastInitY = dblDefaultY;
	m_bLastIsInitY = bInitY;

	if (bInitY)
	{
		SetMinMaxValues(dblDefaultY, CBCGPChartData::CI_Y, m_nHistoryDepth - 1);
	}

	if (m_pXAxis != NULL)
	{
		m_pXAxis->SetScrollRange(0, m_nHistoryDepth - 1);
		UpdateAxes();
	}
}
//****************************************************************************************
void CBCGPChartHistoricalLineSeries::RemoveAllDataPoints()
{
	SetHistoryDepth(m_nHistoryDepth, TRUE, m_bLastIsInitY ? m_dblLastInitY : 0);
}
//****************************************************************************************
int CBCGPChartHistoricalLineSeries::AddDataPoint(const CString& /*strCategoryName*/, double dblY, BCGPChartFormatSeries* pDataPointFormat, DWORD_PTR dwUserData)
{
	return AddDataPoint(dblY, pDataPointFormat, dwUserData);
}
//****************************************************************************************
int CBCGPChartHistoricalLineSeries::AddDataPoint(double dblY, double /*dblX*/, BCGPChartFormatSeries* pDataPointFormat, DWORD_PTR dwUserData)
{
	return AddDataPoint(dblY, pDataPointFormat, dwUserData);
}
//****************************************************************************************
int CBCGPChartHistoricalLineSeries::AddDataPoint(double dblY, BCGPChartFormatSeries* /*pDataPointFormat*/, DWORD_PTR /*dwUserData*/)
{
	ASSERT_VALID(this);
	SetSeriesData(dblY, FALSE);

	return m_nHistoryDepth - 1;
}
//****************************************************************************************
int CBCGPChartHistoricalLineSeries::AddDataPoint(CBCGPChartDataPoint* pDataPoint)
{
	ASSERT_VALID(this);

	if (pDataPoint != NULL)
	{
		SetSeriesData(pDataPoint->GetComponentValue(CBCGPChartData::CI_Y), FALSE);
	}

	return m_nHistoryDepth - 1;
}
//****************************************************************************************
void CBCGPChartHistoricalLineSeries::AddDataPoints(const CBCGPDoubleArray& arYValues, CBCGPDoubleArray* /*pXValues*/, CBCGPDoubleArray* /*pY1Values*/, BOOL /*bRecalcMinMaxValues*/)
{
	for (int i = 0; i < (int)arYValues.GetSize(); i++)
	{
		SetSeriesData(arYValues[i], FALSE);
	}
}
//****************************************************************************************
void CBCGPChartHistoricalLineSeries::SetSeriesData(double dblY, double /*dblX*/, int /*nIndex*/)
{
	ASSERT_VALID(this);
	ASSERT(m_nHistoryDepth > 1);

	m_arYValues.RemoveAt(0, 1);

	m_arYValues.SetAtGrow(m_nHistoryDepth - 1, dblY);
	SetMinMaxValues(dblY, CBCGPChartData::CI_Y, m_nHistoryDepth - 1);
}
//****************************************************************************************
void CBCGPChartHistoricalLineSeries::SetSeriesData(double dblY, BOOL bRedraw)
{
	SetSeriesData(dblY, 0, 0);

	if (m_bUpdateAxesOnNewData)
	{
		UpdateAxes();
	}

	if (bRedraw && m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, TRUE);
	}
}
//****************************************************************************************
void CBCGPChartHistoricalLineSeries::SetRelatedAxes(CBCGPChartAxis* pXAxis, CBCGPChartAxis* pYAxis, CBCGPChartAxis* pZAxis)
{
	ASSERT_VALID(this);
	CBCGPChartSeries::SetRelatedAxes(pXAxis, pYAxis, pZAxis);

	if (m_pXAxis != NULL)
	{
		m_pXAxis->m_crossType = CBCGPChartAxis::CT_IGNORE;
		if (!m_pXAxis->IsFixedIntervalWidth() && m_nFixedGridSizeX != -1)
		{
			m_pXAxis->SetFixedIntervalWidth(m_nFixedGridSizeX, m_nValuesPerIntervalX);
		}

		m_pXAxis->SetScrollRange(0, m_nHistoryDepth - 1);
	}

	UpdateAxes();
}
//****************************************************************************************
void CBCGPChartHistoricalLineSeries::SetRelatedAxis(CBCGPChartAxis* pAxis, CBCGPChartSeries::AxisIndex axisIndex)
{
	ASSERT_VALID(this);
	CBCGPChartSeries::SetRelatedAxis(pAxis, axisIndex);

	if (pAxis != NULL && axisIndex == CBCGPChartSeries::AI_X)
	{
		m_pXAxis->m_crossType = CBCGPChartAxis::CT_IGNORE;
	}

	UpdateAxes();
}
//****************************************************************************************
void CBCGPChartHistoricalLineSeries::EnableHistoryMode(BOOL bEnable, int nHistoryDepth, BOOL bReverseOrder, BOOL bSetDefaultValue, double dblDefaultYValue)
{
	ASSERT_VALID(this);
	
	CBCGPChartSeries::EnableHistoryMode(bEnable, nHistoryDepth, bReverseOrder, bSetDefaultValue, dblDefaultYValue);

	if (bEnable)
	{
		SetHistoryDepth(nHistoryDepth, bSetDefaultValue, dblDefaultYValue);
	}
}

//****************************************************************************************
// CBCGPChartSeries - specific BUBBLE
//****************************************************************************************
CBCGPChartBubbleSeries::CBCGPChartBubbleSeries()
{
	CommonInit();
}
//****************************************************************************************
CBCGPChartBubbleSeries::CBCGPChartBubbleSeries(CBCGPChartVisualObject* pChartCtrl) : CBCGPChartSeries(pChartCtrl, BCGPChartBubble)
{
	CommonInit();
}
//****************************************************************************************
void CBCGPChartBubbleSeries::CommonInit()
{
	m_dblScale = 100.;
	m_FillGradienType = GetDefaultFillGradientType();
	m_formatSeries.m_legendLabelContent = BCGPChartDataLabelOptions::LC_CATEGORY_NAME;
	m_formatSeries.m_dataLabelFormat.m_options.m_position = BCGPChartDataLabelOptions::LP_DEFAULT_POS;
	m_formatSeries.m_dataLabelFormat.m_options.m_content = BCGPChartDataLabelOptions::LC_BUBBLE_SIZE;
	m_formatSeries.m_dataLabelFormat.m_options.m_dblDistanceFromMarker = 40.;
	m_formatSeries.m_dataLabelFormat.m_options.m_bDropLineToMarker = FALSE;
}
//****************************************************************************************
void CBCGPChartBubbleSeries::SetBubbleScale(double dblScale)
{
	m_dblScale = bcg_clamp(dblScale, 25., 300.);
}
//****************************************************************************************
void CBCGPChartBubbleSeries::SetMinMaxValues(const CBCGPChartValue& val, CBCGPChartData::ComponentIndex ci, int nDataPointIndex)
{
	ASSERT_VALID(this);

	if (ci != CBCGPChartData::CI_Y1 || nDataPointIndex < 0 || nDataPointIndex >= GetDataPointCount())
	{
		CBCGPChartSeries::SetMinMaxValues(val, ci, nDataPointIndex);
		return;
	}

	if (IsIgnoreNegativeValues() && val.GetValue() < 0)
	{
		return;
	}

	CBCGPChartSeries::SetMinMaxValues(fabs(val.GetValue()), CBCGPChartData::CI_Y1, nDataPointIndex);
}
//****************************************************************************************
void CBCGPChartBubbleSeries::OnCalcScreenPoints(CBCGPGraphicsManager* /*pGM*/, const CBCGPRect& /*rectDiagramArea*/)
{
	CBCGPChartValue valMaxY1 = GetMaxValue(CBCGPChartData::CI_Y1);
	CBCGPRect rectBounds = GetAxesBoundingRect();

	if (m_pXAxis == NULL || m_pYAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pXAxis);
	ASSERT_VALID(m_pYAxis);

	double dblMinUnitSize = min(m_pXAxis->m_bFormatAsDate ? 
		m_pXAxis->GetAxisUnitSize() * m_pXAxis->GetMajorUnit() : 
		m_pXAxis->GetAxisUnitSize() / m_pXAxis->GetMajorUnit(), 
		m_pYAxis->GetAxisUnitSize() / m_pYAxis->GetMajorUnit());

	double dblOrgBubbleSize = dblMinUnitSize * 2 * 0.95 * m_dblScale / 100.;

	double dblMaxSize = valMaxY1.IsEmpty() ? 1. : valMaxY1;

	for (int i = 0; i <= GetDataPointCount(); i++)
	{
		const CBCGPChartDataPoint* pDp = GetDataPointAt(i);

		RemoveAllDataPointScreenPoints(i);

		if (pDp == NULL || pDp->IsEmpty())
		{
			continue;
		}

		CBCGPChartValue valY1 = GetDataPointValue(i, CBCGPChartData::CI_Y1);

		BOOL bIsEmpty = FALSE;
		CBCGPPoint point = m_pChart->ScreenPointFromChartData(pDp->GetData(), i, bIsEmpty, this);

		double dblValPercent = 1.; 
		
		if (valY1.IsEmpty())
		{
			dblValPercent = valMaxY1.IsEmpty() ? 1 : 0.; 
		}
		else
		{
			if ((double)valY1 < 0 && IsIgnoreNegativeValues())
			{
				continue;
			}

			dblValPercent = fabs(valY1) / dblMaxSize;
		}

		double dblMarkerSize = (dblValPercent * dblOrgBubbleSize) / 2;

		// Set center of bubble
		SetDataPointScreenPoint(i, 0, point);

		// Set Size Of Bubble
		SetDataPointScreenPoint(i, CBCGPBubbleChartImpl::BUBBLE_SP_SIZE, CBCGPPoint(dblMarkerSize, dblMarkerSize));
	}
}
//****************************************************************************************
BOOL CBCGPChartBubbleSeries::CanIncludeDataPointToLegend(int nDataPointIndex)
{
	if (!CBCGPChartSeries::CanIncludeDataPointToLegend(nDataPointIndex))
	{
		return FALSE;
	}

	double dblVal = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y1).GetValue();

	if (dblVal < 0 && IsIgnoreNegativeValues())
	{
		return FALSE;
	}

	return TRUE;
}

//****************************************************************************************
CBCGPBrush::BCGP_GRADIENT_TYPE CBCGPChartBubbleSeries::GetSeriesFillGradientType() const
{
	return CBCGPBrush::BCGP_GRADIENT_RADIAL_CENTER;
}
//****************************************************************************************
// CBCGPChartSeries - specific PIE chart
//****************************************************************************************
CBCGPChartPieSeries::CBCGPChartPieSeries(CBCGPChartVisualObject* pChartCtrl, 
					BCGPChartCategory category) : CBCGPChartSeries(pChartCtrl, category)
{
	ASSERT(category == BCGPChartPie || category == BCGPChartPie3D || category == BCGPChartDoughnut || category == BCGPChartDoughnut3D || category == BCGPChartTorus3D);
	CommonInit();
}
//****************************************************************************************
CBCGPChartPieSeries::CBCGPChartPieSeries(CBCGPChartVisualObject* pChartCtrl, 
				BCGPChartCategory category, const CString& strSeriesName)
			: CBCGPChartSeries(pChartCtrl, strSeriesName, CBCGPColor(), category)
{
	ASSERT(category == BCGPChartPie || category == BCGPChartPie3D);
	CommonInit();
}
//****************************************************************************************
void CBCGPChartPieSeries::CommonInit()
{
	m_dblPieExplosion = 0;
	m_nPieRotation = 0;

	m_bIncludeDataPointLabelsToLegend = TRUE;

	m_formatSeries.m_dataLabelFormat.m_options.m_dblDistanceFromMarker = 20;
	m_formatSeries.m_dataLabelFormat.m_options.m_bDropLineToMarker = FALSE;
	m_formatSeries.m_dataLabelFormat.m_options.m_content = BCGPChartDataLabelOptions::LC_PIE_NAME_PERCENTAGE;
	m_formatSeries.m_dataLabelFormat.m_options.m_position = BCGPChartDataLabelOptions::LP_OUTSIDE_END;
	m_formatSeries.m_legendLabelContent = BCGPChartDataLabelOptions::LC_CATEGORY_NAME;

	m_bAutoColorDataPoints = TRUE;
	m_FillGradienType = GetSeriesFillGradientType();

	m_bFitDiagramArea = FALSE;
	m_nHeightPercent = 100;
	m_dblPieAngle = 45.;

	m_bGroupSmallerSlices = FALSE;
	m_bGroupSmallerSlicesInLegend = FALSE;
	m_dblMinGroupPercent = 0.0;
	m_nSmallGroupDataPointIndex = -1;
}
//****************************************************************************************
CBCGPBrush::BCGP_GRADIENT_TYPE CBCGPChartPieSeries::GetSeriesFillGradientType() const 
{
	return CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT;
}
//****************************************************************************************
void CBCGPChartPieSeries::SetGroupSmallerSlices(BOOL bSet, double dblMinPercent/* = 5.0*/, BOOL bGroupInGegend/* = FALSE*/, const CString& strLabel/* = _T("Others (< 1%)")*/)
{
	m_bGroupSmallerSlices = bSet;
	m_dblMinGroupPercent = bcg_clamp(dblMinPercent, 0.0, 100.0);
	m_bGroupSmallerSlicesInLegend = bGroupInGegend;
	m_strSmallerSlicesGroupLabel = strLabel;

	//-----------------------------------
	// Rebuild legend entries visibility:
	//-----------------------------------

	double dblSum = 0.0;
	BOOL bIsFirstSmallerSlice = TRUE;

	for (int nStep = 0; nStep < 2; nStep++)
	{
		double dblMinDisplayedValue = dblSum * m_dblMinGroupPercent / 100.0;

		for (int i = 0; i < GetDataPointCount(); i++)
		{
			CBCGPChartDataPoint* pDp = (CBCGPChartDataPoint*)GetDataPointAt(i);

			if (pDp == NULL || pDp->IsEmpty())
			{
				continue;
			}

			double dblValue = pDp->GetComponentValue(CBCGPChartData::CI_Y);

			if (dblValue < 0 && IsIgnoreNegativeValues())
			{
				continue;
			}

			if (nStep == 0)
			{
				dblSum += fabs(dblValue);
			}
			else
			{
				if (dblValue < dblMinDisplayedValue && m_bGroupSmallerSlices && m_bGroupSmallerSlicesInLegend)
				{
					if (bIsFirstSmallerSlice)
					{
						pDp->m_bIncludeLabelToLegend = m_bIncludeDataPointLabelsToLegend;
						bIsFirstSmallerSlice = FALSE;
					}
					else
					{
						pDp->m_bIncludeLabelToLegend = FALSE;
					}
				}
				else
				{
					pDp->m_bIncludeLabelToLegend = m_bIncludeDataPointLabelsToLegend;
				}
			}
		}
	}


	if (m_pChart != NULL)
	{
		m_pChart->SetDirty();
	}
}
//****************************************************************************************
void CBCGPChartPieSeries::OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagramArea*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pChart);
	ASSERT_VALID(pGM);

	double dblSum = 0;
	
	m_nSmallGroupDataPointIndex = -1;

	if (m_pChart == NULL)
	{
		return;
	}

	CBCGPPieChartImpl* pImpl = DYNAMIC_DOWNCAST(CBCGPPieChartImpl, GetChartImpl());

	if (pImpl == NULL)
	{
		return;
	}

	ASSERT_VALID(pImpl);

	int i = 0;

	BOOL bOutside = FALSE;
	double dblMaxDistance = 0.;
	double dblMaxExplosion = 0;

	for (i = 0; i < GetDataPointCount(); i++)
	{
		const CBCGPChartDataPoint* pDp = GetDataPointAt(i);

		if (pDp == NULL || pDp->IsEmpty())
		{
			continue;
		}

		const BCGPChartFormatSeries* pDataPointFormat = GetDataPointFormat(i, FALSE);

		double dblValue = pDp->GetComponentValue(CBCGPChartData::CI_Y);

		if (dblValue < 0 && IsIgnoreNegativeValues())
		{
			continue;
		}

		dblSum += fabs(dblValue);

		CString strDPLabel;
		if (!OnGetDataLabelText(i, strDPLabel))
		{
			GetDataPointLabelText(i, pDataPointFormat->m_dataLabelFormat.m_options.m_content, strDPLabel);
		}

		CBCGPSize szDataLabelSize(0, 0);
		if (m_pChart == NULL || !m_pChart->OnCalcDataLabelSize(pGM, strDPLabel, this, i, szDataLabelSize))
		{
			szDataLabelSize = pImpl->OnGetDataLabelSize(pGM, strDPLabel, pDp, this, i);
		}

		m_szMaxDataLabelSize.cx = max(m_szMaxDataLabelSize.cx, szDataLabelSize.cx);
		m_szMaxDataLabelSize.cy = max(m_szMaxDataLabelSize.cy, szDataLabelSize.cy);

		if ((pDataPointFormat->m_dataLabelFormat.m_options.m_position == BCGPChartDataLabelOptions::LP_OUTSIDE_END ||
			pDataPointFormat->m_dataLabelFormat.m_options.m_position == BCGPChartDataLabelOptions::LP_DEFAULT_POS) && 
			pDataPointFormat->m_dataLabelFormat.m_options.m_bShowDataLabel)
		{
			dblMaxDistance = max(dblMaxDistance, GetDataPointLabelDistance(i));
			bOutside = TRUE;
		}

		dblMaxExplosion = max(dblMaxExplosion, GetDataPointPieExplosion(i));
	}

	dblMaxExplosion += m_dblPieExplosion;

	CBCGPRect rectDiagram = GetAxesBoundingRect();

	if (bOutside)
	{
		rectDiagram.DeflateRect(m_szMaxDataLabelSize.cx + dblMaxDistance, m_szMaxDataLabelSize.cy + dblMaxDistance);
	}
	else
	{
		rectDiagram.DeflateRect(rectDiagram.Width() * 5. / 100., 
			rectDiagram.Height() * 5. / 100.);
	}

	BOOL bPreserveShape = !IsFitDiagramAreaEnabled();
	rectDiagram.DeflateRect(dblMaxExplosion / 2, dblMaxExplosion / 2);

	CBCGPPoint ptCenter = rectDiagram.CenterPoint();
	double dblK = m_nHeightPercent / 100. / 5.0;
	double dblAngle = pImpl->Is3DPie() ? bcg_deg2rad(m_dblPieAngle) : M_PI_2;
	
	double dblRadiusX = bPreserveShape ? min(rectDiagram.Width() / 2, rectDiagram.Height() / (dblK * cos(dblAngle) + sin(dblAngle) * 2)) :
							rectDiagram.Width() / 2;

	double dblRadiusY = bPreserveShape ? dblRadiusX * sin(dblAngle) : 
		(pImpl->Is3DPie() ? rectDiagram.Height() / 2 - rectDiagram.Height() / 2 * dblK : rectDiagram.Height() / 2);
	double dblHeight = bPreserveShape ? dblRadiusX * dblK * cos(dblAngle) : dblRadiusY * dblK;
	
	if (bPreserveShape)
	{
		ptCenter.y -= dblHeight / 2;
	}	

	if (pImpl->Is3DPie() && !bPreserveShape && dblHeight < 5.)
	{
		dblHeight = 5.;
	}

	if (dblRadiusY <= 0)
	{
		dblRadiusY = 0.1;
	}

	double dblPieArea = 2 * M_PI * dblRadiusX * dblRadiusY;
	double dblStartAngle = bcg_deg2rad(m_nPieRotation);

	double dblPieExplosion = GetPieExplosion();
	
	dblRadiusX -= dblPieExplosion;
	dblRadiusY -= dblPieExplosion;

	double dblSmallSum = 0.0;
	double dblMinDisplayedValue = 0.0;

	if (m_bGroupSmallerSlices && m_dblMinGroupPercent > 0.0)
	{
		dblMinDisplayedValue = dblSum * m_dblMinGroupPercent / 100.0;

		for (i = 0; i < GetDataPointCount(); i++)
		{
			CBCGPChartDataPoint* pDp = (CBCGPChartDataPoint*)GetDataPointAt(i);

			if (pDp == NULL || pDp->IsEmpty())
			{
				continue;
			}

			double dblVal = pDp->GetComponentValue(CBCGPChartData::CI_Y);

			if (dblVal < 0 && IsIgnoreNegativeValues())
			{
				continue;
			}

			if (dblVal == 0)
			{
				continue;
			}

			if (fabs(dblVal) < dblMinDisplayedValue)
			{
				dblSmallSum += fabs(dblVal);
			}
		}
	}

	for (i = 0; i < GetDataPointCount(); i++)
	{
		CBCGPChartDataPoint* pDp = (CBCGPChartDataPoint*)GetDataPointAt(i);

		RemoveAllDataPointScreenPoints(i);

		if (pDp == NULL || pDp->IsEmpty())
		{
			continue;
		}

		pDp->m_bIncludeLabelToLegend = m_bIncludeDataPointLabelsToLegend;

		double dblDataPointExplosion = GetDataPointPieExplosion(i);
		double dblTotalExplosion = dblDataPointExplosion + dblPieExplosion; 

		double dblPieRadiusX = dblRadiusX - dblPieExplosion;
		double dblPieRadiusY = dblRadiusY - dblPieExplosion;

		double dblVal = pDp->GetComponentValue(CBCGPChartData::CI_Y);

		if (dblVal < 0 && IsIgnoreNegativeValues())
		{
			continue;
		}

		if (dblVal == 0)
		{
			continue;
		}

		BOOL bIsVisible = TRUE;
		BOOL bIsSmallGroup = FALSE;

		if (m_bGroupSmallerSlices && fabs(dblVal) < dblMinDisplayedValue)
		{
			if (m_nSmallGroupDataPointIndex >= 0)
			{
				bIsVisible = FALSE;

				if (m_bGroupSmallerSlicesInLegend)
				{
					pDp->m_bIncludeLabelToLegend = FALSE;
					continue;
				}
			}
			else
			{
				m_nSmallGroupDataPointIndex = i;

				if (m_bGroupSmallerSlicesInLegend)
				{
					dblVal = dblSmallSum;
				}
			}

			bIsSmallGroup = TRUE;
		}

		double dblPercent = (dblSum < DBL_EPSILON) ? 0. : (fabs(dblVal) * 100. / dblSum);
		pDp->SetComponentValue(dblPercent, CBCGPChartData::CI_PERCENTAGE);

		if (bIsVisible)
		{
			if (bIsSmallGroup && !m_bGroupSmallerSlicesInLegend)
			{
				dblPercent = (dblSum < DBL_EPSILON) ? 0. : (fabs(dblSmallSum) * 100. / dblSum);
			}

			double dblDPArea = dblPercent * dblPieArea / 100.;

			double dblAngle = (fabs(dblPieArea) < DBL_EPSILON) ? 0. : (2. * M_PI * dblDPArea / dblPieArea);
			CBCGPPoint ptAngles(dblStartAngle, dblStartAngle + dblAngle);

			double dblDiff = ptAngles.y > ptAngles.x ? ptAngles.x + (ptAngles.y - ptAngles.x) / 2 : ptAngles.x + (ptAngles.x - ptAngles.y) / 2;
			CBCGPPoint ptExplodedCenter (ptCenter.x + dblTotalExplosion * sin(dblDiff), ptCenter.y - dblTotalExplosion * cos(dblDiff));

			CBCGPPoint ptStartPie(ptExplodedCenter.x + dblRadiusX * sin(dblStartAngle), ptExplodedCenter.y - dblRadiusY * cos(dblStartAngle));
			dblStartAngle += dblAngle;

			CBCGPPoint ptEndPie(ptExplodedCenter.x + dblRadiusX * sin(dblStartAngle), ptExplodedCenter.y - dblRadiusY * cos(dblStartAngle));

			SetDataPointScreenPoint(i, CBCGPPieChartImpl::PIE_SP_START, ptStartPie);
			SetDataPointScreenPoint(i, CBCGPPieChartImpl::PIE_SP_END, ptEndPie);
			SetDataPointScreenPoint(i, CBCGPPieChartImpl::PIE_SP_ANGLES, ptAngles);
			SetDataPointScreenPoint(i, CBCGPPieChartImpl::PIE_SP_RADIUS, CBCGPPoint(dblPieRadiusX, dblPieRadiusY));
			SetDataPointScreenPoint(i, CBCGPPieChartImpl::PIE_SP_CENTER, ptExplodedCenter);
			SetDataPointScreenPoint(i, CBCGPPieChartImpl::PIE_SP_HEIGHT, CBCGPPoint(dblHeight, dblHeight));
			SetDataPointScreenPoint(i, CBCGPPieChartImpl::PIE_SP_DIAGRAM_CENTER, CBCGPPoint(ptCenter.x, ptCenter.y));
		}
	}
}
//****************************************************************************************
BOOL CBCGPChartPieSeries::GetDataPointLabelText(int nDataPointIndex, BCGPChartDataLabelOptions::LabelContent content, CString& strDPLabel)
{
	if ((!m_bGetDataPointLabelForLegend || m_bGroupSmallerSlicesInLegend) && nDataPointIndex == m_nSmallGroupDataPointIndex)
	{
		strDPLabel = m_strSmallerSlicesGroupLabel;
		
		if (strDPLabel.IsEmpty())
		{
			strDPLabel.Format(_T("(< %.0f%%)"), m_dblMinGroupPercent);
		}

		return TRUE;
	}

	return CBCGPChartSeries::GetDataPointLabelText(nDataPointIndex, content, strDPLabel);
}
//****************************************************************************************
void CBCGPChartPieSeries::OnBeforeChangeType()
{
	for (int i = 0; i < GetDataPointCount(); i++)
	{
		CBCGPChartDataPoint* pDp = (CBCGPChartDataPoint*) GetDataPointAt(i);
		pDp->SetComponentCount(1);

		if (m_bGroupSmallerSlices && m_bGroupSmallerSlicesInLegend)
		{
			pDp->m_bIncludeLabelToLegend = m_bIncludeDataPointLabelsToLegend;
		}
	}
}
//****************************************************************************************
void CBCGPChartPieSeries::SetHeightPercent(int nPercent) 
{
	m_nHeightPercent = bcg_clamp(nPercent, 0, 200);
}
//****************************************************************************************
void CBCGPChartPieSeries::SetPieAngle(double dblAngle)
{
	m_dblPieAngle = bcg_clamp(dblAngle, 0., 90.);
}
//****************************************************************************************
void CBCGPChartPieSeries::SetPieExplosion(double dblExplosion)
{
	m_dblPieExplosion = bcg_clamp(dblExplosion, 0., 400.);
}
//****************************************************************************************
void CBCGPChartPieSeries::SetDataPointPieExplosion(double dblExplosion, int nDataPontIndex)
{
	dblExplosion = bcg_clamp(dblExplosion, 0., 400.);
	SetDataPointValue(nDataPontIndex, dblExplosion, CBCGPChartData::CI_Y1);
}
//****************************************************************************************
double CBCGPChartPieSeries::GetDataPointPieExplosion(int nDataPointIndex) const
{
	return GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y1).GetValue();
}
//****************************************************************************************
CBCGPChartValue CBCGPChartPieSeries::GetDataPointValue(int nDataPointIndex, CBCGPChartData::ComponentIndex ci) const
{
	if (ci == CBCGPChartData::CI_X)
	{
		return GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_PERCENTAGE);
	}
	return CBCGPChartSeries::GetDataPointValue(nDataPointIndex, ci);
}
//****************************************************************************************
BOOL CBCGPChartPieSeries::CanIncludeDataPointToLegend(int nDataPointIndex)
{
	if (!CBCGPChartSeries::CanIncludeDataPointToLegend(nDataPointIndex))
	{
		return FALSE;
	}

	double dblVal = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y).GetValue();

	if (dblVal < 0 && IsIgnoreNegativeValues())
	{
		return FALSE;
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartPieSeries::HitTestDataPoint(const CBCGPPoint& pt, BCGPChartHitInfo* pHitInfo)
{
	CBCGPPieChartImpl* pImpl = DYNAMIC_DOWNCAST(CBCGPPieChartImpl, GetChartImpl());
	if (pImpl == NULL)
	{
		return FALSE;
	}

	int nSteps = (pImpl->Is3DPie() && m_nHeightPercent > 0) ? 2 : 1;

	for (int nStep = 0; nStep < nSteps; nStep++)
	{
		for (int nDataPointIndex = 0; nDataPointIndex < GetDataPointCount(); nDataPointIndex++)
		{
			CBCGPPoint ptCenter = GetDataPointScreenPoint(nDataPointIndex, CBCGPPieChartImpl::PIE_SP_CENTER);

			double dblRadiusX = GetDataPointScreenPoint(nDataPointIndex, CBCGPPieChartImpl::PIE_SP_RADIUS).x;
			double dblRadiusY = GetDataPointScreenPoint(nDataPointIndex, CBCGPPieChartImpl::PIE_SP_RADIUS).y;

			if (nStep == 1)
			{
				ptCenter.y += dblRadiusY * m_nHeightPercent / 100. / 5.0;
			}
			
			CBCGPRect bounds(ptCenter.x - dblRadiusX, ptCenter.y - dblRadiusY, ptCenter.x + dblRadiusX, ptCenter.y + dblRadiusY);
			CBCGPPoint ptAngles = GetDataPointScreenPoint(nDataPointIndex, CBCGPPieChartImpl::PIE_SP_ANGLES);

			if (bcg_pointInPie(bounds, ptAngles.x - M_PI_2, ptAngles.y - M_PI_2, pt, 0.01 * GetDoughnutPercent()))
			{
				pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_DATA_POINT;
				pHitInfo->m_nIndex2 = nDataPointIndex;
				return TRUE;
			}
		}
	}

	return FALSE;
}
//****************************************************************************************
BOOL CBCGPChartPieSeries::OnGetDataPointTooltip(int nDataPointIndex, CString& strTooltip, CString& strTooltipDescr)
{
	CBCGPChartAxis* pAxisY = GetRelatedAxis(CBCGPChartSeries::AI_Y);
	CBCGPChartAxis* pAxisX = GetRelatedAxis(CBCGPChartSeries::AI_X);

	if (pAxisY == NULL || pAxisX == NULL)
	{
		return TRUE;
	}

	ASSERT_VALID(pAxisY);
	ASSERT_VALID(pAxisX);

	CBCGPChartValue valY = GetDataPointValue(nDataPointIndex);

	if (valY.IsEmpty())
	{
		return TRUE;
	}

	CBCGPChartValue valX;
	valX.SetValue(nDataPointIndex + 1);

	if (nDataPointIndex == m_nSmallGroupDataPointIndex)
	{
		strTooltip = m_strSmallerSlicesGroupLabel;
		
		if (strTooltip.IsEmpty())
		{
			strTooltip.Format(_T("(< %.0f%%)"), m_dblMinGroupPercent);
		}
	}
	else
	{
		pAxisX->GetDisplayedLabel(valX, strTooltip);
	}

	if (strTooltip.IsEmpty())
	{
		strTooltip = _T("Pie Series");
	}

	CString strLabelY;
	pAxisY->GetDisplayedLabel(valY.GetValue(), strLabelY);

	valX = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_X);

	strTooltipDescr.Format(_T("%d: %s (%3.1f%%)"), nDataPointIndex + 1, strLabelY, (double)valX);
	
	return TRUE;
}
//****************************************************************************************
void CBCGPChartDoughnutSeries::SetDoughnutPercent(int nPercent) 
{
	m_nDoughnutPercent = bcg_clamp(nPercent, 0, 100);
}
//****************************************************************************************
// CBCGPChartSeries - specific PYRAMID chart
//****************************************************************************************
CBCGPChartPyramidSeries::CBCGPChartPyramidSeries(CBCGPChartVisualObject* pChartCtrl, BCGPChartCategory category) :
						CBCGPChartSeries(pChartCtrl, category)
{
	CommonInit();
}
//****************************************************************************************
CBCGPChartPyramidSeries::CBCGPChartPyramidSeries(CBCGPChartVisualObject* pChartCtrl, 
		BCGPChartCategory category, const CString& strSeriesName)
		: CBCGPChartSeries(pChartCtrl, strSeriesName, CBCGPColor(), category)
{
	CommonInit();
}
//****************************************************************************************
void CBCGPChartPyramidSeries::CommonInit()
{
	m_bIncludeDataPointLabelsToLegend = TRUE;
	m_bAutoColorDataPoints = TRUE;
	m_bIsCircularBase = FALSE;

	m_FillGradienType = GetSeriesFillGradientType();

	m_formatSeries.m_dataLabelFormat.m_options.m_content = 
		(BCGPChartDataLabelOptions::LabelContent)
			(BCGPChartDataLabelOptions::LC_CATEGORY_NAME | BCGPChartDataLabelOptions::LC_PERCENTAGE);
	m_formatSeries.m_dataLabelFormat.m_options.m_position = BCGPChartDataLabelOptions::LP_OUTSIDE_END;
	m_formatSeries.m_legendLabelContent = BCGPChartDataLabelOptions::LC_CATEGORY_NAME;

	m_nGap = 0;
	m_nAllowedGap = 0;
	m_nDepthPercent = 10;
	m_dblRotation = 0;

	m_bDataLabelsInColumns = TRUE;
}
//****************************************************************************************
CBCGPBrush::BCGP_GRADIENT_TYPE CBCGPChartPyramidSeries::GetSeriesFillGradientType() const 
{
	return m_bIsCircularBase ? CBCGPBrush::BCGP_GRADIENT_PIPE_VERTICAL : CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT;
}
//****************************************************************************************
void CBCGPChartPyramidSeries::SetGap(int nGap)
{
	m_nGap = bcg_clamp(nGap, 0, 100);
}
//****************************************************************************************
void CBCGPChartPyramidSeries::SetDepthPercent(int nDepthPercent)
{
	m_nDepthPercent = bcg_clamp(nDepthPercent, 1, 45);
}
//****************************************************************************************
void CBCGPChartPyramidSeries::SetRotation(double dblRotation)
{
	m_dblRotation = dblRotation;

	if (m_dblRotation < -90.0)
	{
		m_dblRotation += 180.0;
	}
	else if (m_dblRotation > 90.0)
	{
		m_dblRotation -= 180.0;
	}
}
//****************************************************************************************
void CBCGPChartPyramidSeries::SetCircularBase(BOOL bIsCircularBase)
{
	if (m_bIsCircularBase == bIsCircularBase)
	{
		return;
	}

	m_bIsCircularBase = bIsCircularBase;
	m_FillGradienType = GetSeriesFillGradientType();

	for (int i = 0; i < GetDataPointCount(); i++)
	{
		BCGPSeriesColorsPtr seriesColors;
		GetColors(seriesColors, i);

		if (seriesColors.m_pBrElementFillColor != NULL)
		{
			seriesColors.m_pBrElementFillColor->SetColors(
				seriesColors.m_pBrElementFillColor->GetColor(),
				seriesColors.m_pBrElementFillColor->GetGradientColor(),
				m_FillGradienType,
				seriesColors.m_pBrElementFillColor->GetOpacity());
		}
	}
}
//****************************************************************************************
void CBCGPChartPyramidSeries::OnBeforeChangeType()
{
	for (int i = 0; i < GetDataPointCount(); i++)
	{
		CBCGPChartDataPoint* pDp = (CBCGPChartDataPoint*) GetDataPointAt(i);
		pDp->SetComponentCount(1);
	}
}
//****************************************************************************************
CBCGPChartValue CBCGPChartPyramidSeries::GetDataPointValue(int nDataPointIndex, CBCGPChartData::ComponentIndex ci) const
{
	if (ci == CBCGPChartData::CI_X)
	{
		return GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_PERCENTAGE);
	}
	return CBCGPChartSeries::GetDataPointValue(nDataPointIndex, ci);
}
//****************************************************************************************
BOOL CBCGPChartPyramidSeries::CanIncludeDataPointToLegend(int nDataPointIndex)
{
	if (!CBCGPChartSeries::CanIncludeDataPointToLegend(nDataPointIndex))
	{
		return FALSE;
	}

	double dblVal = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y).GetValue();

	if (dblVal < 0 && IsIgnoreNegativeValues())
	{
		return FALSE;
	}

	return TRUE;
}
//****************************************************************************************
void CBCGPChartPyramidSeries::SortDataPoints(int nScreenPointIndex, CArray<CBCGPChartDataPoint*, CBCGPChartDataPoint*>& arDPSorted)
{
	ASSERT_VALID(this);

	arDPSorted.RemoveAll();

	for (int i = 0; i < GetDataPointCount(); i++)
	{
		CBCGPChartDataPoint* pDP = (CBCGPChartDataPoint*)GetDataPointAt(i);

		if (IsDataPointScreenPointsEmpty(i))
		{
			continue;
		}

		double dblHeight = GetDataPointScreenPoint(i, nScreenPointIndex).x;
		BOOL bInserted = FALSE;

		for (int j = 0; j < arDPSorted.GetSize(); j++)
		{
			CBCGPChartDataPoint* pDPSorted = arDPSorted[j];
			int nDPIndex = FindDataPointIndex(pDPSorted);
			double dblSortedHeight = GetDataPointScreenPoint(nDPIndex, nScreenPointIndex).x;
			if (dblHeight <= dblSortedHeight)
			{
				arDPSorted.InsertAt(j, pDP);
				bInserted = TRUE;
				break;
			}
		}

		if (!bInserted)
		{
			arDPSorted.Add(pDP);
		}
	}
}
//****************************************************************************************
BOOL CBCGPChartPyramidSeries::CalcSeriesParams(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, 
											   double& dblSum, int& nNonEmptyCount, CBCGPRect& rectShape)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pChart);
	ASSERT_VALID(pGM);


	dblSum = 0;
	nNonEmptyCount = 0;
	rectShape.SetRectEmpty();
	
	CBCGPPyramidChartImpl* pImpl = DYNAMIC_DOWNCAST(CBCGPPyramidChartImpl, GetChartImpl());

	if (m_pChart == NULL || pGM == NULL || pImpl == NULL)
	{
		return FALSE;
	}

	BOOL bOutside = FALSE;
	double dblMaxDistance = 0.;

	BOOL bRightLabels = FALSE;
	BOOL bLeftLabels = FALSE;

	int i = 0;
	for (; i < GetDataPointCount(); i++)
	{
		const CBCGPChartDataPoint* pDp = GetDataPointAt(i);

		if (pDp == NULL || pDp->IsEmpty())
		{
			continue;
		}

		const BCGPChartFormatSeries* pDataPointFormat = GetDataPointFormat(i, FALSE);

		double dblVal = pDp->GetComponentValue(CBCGPChartData::CI_Y);

		if (dblVal < 0 && IsIgnoreNegativeValues())
		{
			continue;
		}

		nNonEmptyCount++;

		dblSum += fabs(dblVal);

		CString strDPLabel;
		if (!OnGetDataLabelText(i, strDPLabel))
		{
			GetDataPointLabelText(i, pDataPointFormat->m_dataLabelFormat.m_options.m_content, strDPLabel);
		}

		CBCGPSize szDataLabelSize(0, 0);
		if (m_pChart == NULL || !m_pChart->OnCalcDataLabelSize(pGM, strDPLabel, this, i, szDataLabelSize))
		{
			szDataLabelSize = pImpl->OnGetDataLabelSize(pGM, strDPLabel, pDp, this, i);
		}

		m_szMaxDataLabelSize.cx = max(m_szMaxDataLabelSize.cx, szDataLabelSize.cx);
		m_szMaxDataLabelSize.cy = max(m_szMaxDataLabelSize.cy, szDataLabelSize.cy);

		if ((pDataPointFormat->m_dataLabelFormat.m_options.m_position == BCGPChartDataLabelOptions::LP_OUTSIDE_END ||
			pDataPointFormat->m_dataLabelFormat.m_options.m_position == BCGPChartDataLabelOptions::LP_DEFAULT_POS) && 
			pDataPointFormat->m_dataLabelFormat.m_options.m_bShowDataLabel)
		{
			dblMaxDistance = max(dblMaxDistance, GetDataPointLabelDistance(i));
			bOutside = TRUE;

			if (pDataPointFormat->m_dataLabelFormat.m_options.m_dblAngle >= 0)
			{
				bRightLabels = TRUE;
			}
			else if (pDataPointFormat->m_dataLabelFormat.m_options.m_dblAngle <= 0)
			{
				bLeftLabels = TRUE;
			}
		}
	}

	rectShape = rectDiagramArea;
	double dblDefaultGapX = rectShape.Width() * 3. / 100.;
	double dblDefaultGapY = rectShape.Height() * 5. / 100.;

	if (bOutside)
	{
		if (bRightLabels && bLeftLabels)
		{
			rectShape.DeflateRect(m_szMaxDataLabelSize.cx + dblMaxDistance, dblDefaultGapY);
		}
		else if (bLeftLabels)
		{
			rectShape.DeflateRect(dblDefaultGapX, dblDefaultGapY);
			rectShape.left += m_szMaxDataLabelSize.cx + dblMaxDistance;
		}
		else if (bRightLabels)
		{
			rectShape.DeflateRect(dblDefaultGapX, dblDefaultGapY);
			rectShape.right -= m_szMaxDataLabelSize.cx + dblMaxDistance;
		}
	}
	else
	{
		rectShape.DeflateRect(dblDefaultGapX, dblDefaultGapY);
	}

	double dblHeight = rectShape.Height();

	int nMinHeight = (nNonEmptyCount - 1) * 2;
	int nGap = bcg_round(m_pChart->GetScaleRatioMid() * m_nGap);

	double dblPureHeight = dblHeight - (nNonEmptyCount - 1) * nGap;

	if (dblPureHeight < 0)
	{
		m_nAllowedGap = (int)(dblHeight - nMinHeight) / (nNonEmptyCount - 1);
		dblPureHeight = dblHeight - (nNonEmptyCount - 1) * m_nAllowedGap;
	}
	else
	{
		m_nAllowedGap = nGap;
	}

	for (i = 0; i < GetDataPointCount(); i++)
	{
		CBCGPChartDataPoint* pDp = (CBCGPChartDataPoint*)GetDataPointAt(i);

		RemoveAllDataPointScreenPoints(i);

		if (pDp == NULL || pDp->IsEmpty())
		{
			continue;
		}

		double dblVal = pDp->GetComponentValue(CBCGPChartData::CI_Y);

		if (dblVal < 0 && IsIgnoreNegativeValues())
		{
			continue;
		}

		double dblPercent = (fabs(dblSum) < DBL_EPSILON) ? 0 : (fabs(dblVal) * 100. / dblSum);

		pDp->SetComponentValue(dblPercent, CBCGPChartData::CI_PERCENTAGE); 
		double dblDPHeight = dblPercent * dblPureHeight / 100.;

		SetDataPointScreenPoint(i, CBCGPPyramidChartImpl::PYRAMID_SP_HEIGHT, CBCGPPoint(dblDPHeight, dblDPHeight));
	}

	return TRUE;
}
//****************************************************************************************
void CBCGPChartPyramidSeries::OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagramArea*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pChart);
	ASSERT_VALID(pGM);


	if (m_pChart == NULL || pGM == NULL)
	{
		return;
	}

	double dblSum = 0;
	CBCGPRect rectShape;
	int nNonEmptyCount = 0;
	

	CBCGPRect rectDiagram = GetAxesBoundingRect();

	if (!CalcSeriesParams(pGM, rectDiagram, dblSum, nNonEmptyCount, rectShape))
	{
		return;
	}

	CArray<CBCGPChartDataPoint*, CBCGPChartDataPoint*> arDPSorted;
	SortDataPoints(CBCGPPyramidChartImpl::PYRAMID_SP_HEIGHT, arDPSorted);
	
	double dblCurrOffset = 0;

	for (int nIndex = 0; nIndex < arDPSorted.GetSize(); nIndex++)
	{
		CBCGPChartDataPoint* pDpNext = arDPSorted[nIndex];
		int nDataPointIndex = FindDataPointIndex(pDpNext);
		double dblHeight = GetDataPointScreenPoint(nDataPointIndex, CBCGPPyramidChartImpl::PYRAMID_SP_HEIGHT).x;

		SetDataPointScreenPoint(nDataPointIndex, CBCGPPyramidChartImpl::PYRAMID_SP_OFFSETS, 
			CBCGPPoint(dblCurrOffset, dblCurrOffset + dblHeight));

		SetDataPointScreenPoint(nDataPointIndex, CBCGPPyramidChartImpl::PYRAMID_SP_LEFT_TOP, rectShape.TopLeft());
		SetDataPointScreenPoint(nDataPointIndex, CBCGPPyramidChartImpl::PYRAMID_SP_RIGHT_BOTTOM, rectShape.BottomRight());

		dblCurrOffset += dblHeight + m_nAllowedGap;
	}
}
//****************************************************************************************
// CBCGPChartSeries - specific FUNNEL chart
//****************************************************************************************
CBCGPChartFunnelSeries::CBCGPChartFunnelSeries(CBCGPChartVisualObject* pChartCtrl, BCGPChartCategory category) :
					CBCGPChartPyramidSeries(pChartCtrl, category)
{
	CommonInit();
}
//****************************************************************************************
CBCGPChartFunnelSeries::CBCGPChartFunnelSeries(CBCGPChartVisualObject* pChartCtrl, 
					   BCGPChartCategory category, const CString& strSeriesName) : 
					CBCGPChartPyramidSeries(pChartCtrl, category, strSeriesName)
{
	CommonInit();
}
//****************************************************************************************
void CBCGPChartFunnelSeries::CommonInit()
{
	CBCGPChartPyramidSeries::CommonInit();

	m_bIsCircularBase = TRUE;
	m_FillGradienType = GetSeriesFillGradientType();

	m_nNeckWidth = 50;
	m_dblNeckHeight = 50.;
	m_bNeckHeightIsValue = FALSE;
	
	m_nGap = 0;
	m_nDepthPercent = 10;
}
//****************************************************************************************
void CBCGPChartFunnelSeries::SetNeckHeightInPercents(double dblHeight)
{
	m_dblNeckHeight = bcg_clamp(dblHeight, 0., 100.);
	m_bNeckHeightIsValue = FALSE;
}
//****************************************************************************************
void CBCGPChartFunnelSeries::SetNeckHeightInChartValues(double dblValue)
{
	m_dblNeckHeight = dblValue;
	m_bNeckHeightIsValue = TRUE;
}
//****************************************************************************************
void CBCGPChartFunnelSeries::SetNeckWidth(int nWidth)
{
	m_nNeckWidth = bcg_clamp(nWidth, 10, 100);
}
//****************************************************************************************
void CBCGPChartFunnelSeries::OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagramArea*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pChart);
	ASSERT_VALID(pGM);


	if (m_pChart == NULL || pGM == NULL)
	{
		return;
	}

	double dblSum = 0;
	CBCGPRect rectShape;
	int nNonEmptyCount = 0;

	m_szNeckSize.SetSizeEmpty();

	CBCGPRect rectDiagram = GetAxesBoundingRect();

	if (!CalcSeriesParams(pGM, rectDiagram, dblSum, nNonEmptyCount, rectShape))
	{
		return;
	}

	m_szNeckSize.cy = m_bNeckHeightIsValue ? 0 : rectShape.Height() * m_dblNeckHeight / 100.;
	m_szNeckSize.cx = max(10., rectShape.Width() / 2. * (double)m_nNeckWidth / 100.);

	CArray<CBCGPChartDataPoint*, CBCGPChartDataPoint*> arDPSorted;
	SortDataPoints(CBCGPPyramidChartImpl::PYRAMID_SP_HEIGHT, arDPSorted);
	
	double dblCurrOffset = 0;

	for (int nIndex = (int)arDPSorted.GetSize() - 1; nIndex >= 0; nIndex--)
	{
		CBCGPChartDataPoint* pDpNext = arDPSorted[nIndex];
		int nDataPointIndex = FindDataPointIndex(pDpNext);
		double dblHeight = GetDataPointScreenPoint(nDataPointIndex, CBCGPPyramidChartImpl::PYRAMID_SP_HEIGHT).x;

		SetDataPointScreenPoint(nDataPointIndex, CBCGPPyramidChartImpl::PYRAMID_SP_OFFSETS, 
			CBCGPPoint(dblCurrOffset, dblCurrOffset + dblHeight));

		SetDataPointScreenPoint(nDataPointIndex, CBCGPPyramidChartImpl::PYRAMID_SP_LEFT_TOP, rectShape.TopLeft());
		SetDataPointScreenPoint(nDataPointIndex, CBCGPPyramidChartImpl::PYRAMID_SP_RIGHT_BOTTOM, rectShape.BottomRight());

		if (m_bNeckHeightIsValue)
		{
			double dblValue = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y).GetValue();
			if (fabs(dblValue) <= m_dblNeckHeight)
			{
				m_szNeckSize.cy += dblHeight;
				m_szNeckSize.cy += m_nAllowedGap;
			}
		}

		dblCurrOffset += dblHeight + m_nAllowedGap;
	}
}
//****************************************************************************************
// CBCGPChartSeries - specific STOCK chart
//****************************************************************************************
CBCGPBaseChartStockSeries::CBCGPBaseChartStockSeries()
{
	m_bIsMainStockSeries = FALSE;
	m_type = CBCGPBaseChartStockSeries::SST_BAR;
	m_kind = CBCGPBaseChartStockSeries::SSK_UNDEFINED;
	m_pChartImpl = new CBCGPStockChartImpl();
	m_pParentSeries = NULL;
}
//****************************************************************************************
CBCGPBaseChartStockSeries::CBCGPBaseChartStockSeries(CBCGPChartVisualObject* pChartCtrl, StockSeriesType seriesType) : 
CBCGPChartSeries(pChartCtrl, BCGPChartStock)
{
	m_bIsMainStockSeries = FALSE;
	m_type = seriesType;
	m_kind = CBCGPBaseChartStockSeries::SSK_UNDEFINED;
	m_pParentSeries = NULL;
	m_pChartImpl = new CBCGPStockChartImpl(pChartCtrl);
}
//****************************************************************************************
CBCGPChartStockData CBCGPBaseChartStockSeries::GetStockDataAt(int nDataPointIndex) const
{
	const CBCGPStockDataArray& stockData = GetStockData();
	return stockData[nDataPointIndex];
}
//****************************************************************************************
CBCGPChartValue CBCGPBaseChartStockSeries::GetDataPointValue(int nDataPointIndex, CBCGPChartData::ComponentIndex ci) const
{
	if (!IsOptimizedLongDataMode())
	{
		return CBCGPChartSeries::GetDataPointValue(nDataPointIndex, ci);
	}

	int nSize = GetDataPointCount();

	if (nDataPointIndex >= nSize || nDataPointIndex < 0)
	{
		return CBCGPChartValue();
	}

	CBCGPChartStockData data = (m_bIsMainStockSeries || m_pParentSeries == NULL) ? 
		GetStockDataAt(nDataPointIndex) : m_pParentSeries->GetStockDataAt(nDataPointIndex);

	if (ci == CBCGPChartData::CI_X)
	{
		return CBCGPChartValue(data.m_dateTime.m_dt);
	}

	if (data.IsEmpty())
	{
		return CBCGPChartValue();
	}

	switch(m_kind)
	{
	case CBCGPBaseChartStockSeries::SSK_OPEN:
		return data.m_dblOpen;
	case CBCGPBaseChartStockSeries::SSK_HIGH:
		return data.m_dblHigh;
	case CBCGPBaseChartStockSeries::SSK_LOW:
		return data.m_dblLow;
	case CBCGPBaseChartStockSeries::SSK_CLOSE:
		return data.m_dblClose;
	}

	return CBCGPChartValue();
}
//****************************************************************************************
int CBCGPBaseChartStockSeries::GetDataPointCount() const
{
	if (!IsOptimizedLongDataMode())
	{
		return CBCGPChartSeries::GetDataPointCount();
	}

	if (m_bIsMainStockSeries)
	{
		const CBCGPStockDataArray& stockData = GetStockData();
		return (int)stockData.GetSize();
	}

	if (m_pParentSeries == NULL)
	{
		return 0;
	}

	return m_pParentSeries->GetDataPointCount();
}
//****************************************************************************************
const CBCGPChartDataPoint* CBCGPBaseChartStockSeries::GetDataPointAt(int nIndex) const
{
	if (!IsOptimizedLongDataMode())
	{
		return CBCGPChartSeries::GetDataPointAt(nIndex);
	}

	CBCGPChartDataPoint* pDP = (CBCGPChartDataPoint*)&m_virtualPoint;
	CBCGPChartValue val = GetDataPointValue(nIndex);
	pDP->SetComponentValue(val);

	val = GetDataPointValue(nIndex, CBCGPChartData::CI_X);
	pDP->SetComponentValue(val, CBCGPChartData::CI_X);

 	pDP->m_arScreenPoints.RemoveAll();
 	
 	BOOL bIsEmpty = FALSE;
 	CBCGPPoint point = m_pChart->ScreenPointFromChartData(pDP->GetData(), nIndex, bIsEmpty, (CBCGPChartSeries*)this);
 	
 	if (!bIsEmpty)
 	{
 		pDP->m_arScreenPoints.Add(point);
 	}

	return &m_virtualPoint;
}
//****************************************************************************************
const CBCGPStockDataArray& CBCGPBaseChartStockSeries::GetStockData() const
{
	if (IsMainStockSeries() || m_pParentSeries == NULL)
	{
		return m_stockData;
	}

	return m_pParentSeries->GetStockData();
}
//****************************************************************************************
//****************************************************************************************
CBCGPChartStockSeries::CBCGPChartStockSeries(CBCGPChartVisualObject* pChartCtrl,  
											 CBCGPBaseChartStockSeries::StockSeriesType seriesType) : 
							CBCGPBaseChartStockSeries(pChartCtrl, seriesType)
{
	ASSERT_VALID(pChartCtrl);

	CommonInit();

	m_kind = CBCGPBaseChartStockSeries::SSK_OPEN;
	CBCGPBaseChartStockSeries* pSeriesHigh = new CBCGPBaseChartStockSeries(pChartCtrl, seriesType);
	pSeriesHigh->m_kind = CBCGPBaseChartStockSeries::SSK_HIGH;
	pSeriesHigh->m_pParentSeries = this;

	m_arChildSeries.Add(pSeriesHigh);

	CBCGPBaseChartStockSeries* pSeriesLow = new CBCGPBaseChartStockSeries(pChartCtrl, seriesType);
	pSeriesLow->m_kind = CBCGPBaseChartStockSeries::SSK_LOW;
	pSeriesLow->m_pParentSeries = this;

	m_arChildSeries.Add(pSeriesLow);

	CBCGPBaseChartStockSeries* pSeriesClose = new CBCGPBaseChartStockSeries(pChartCtrl, seriesType);
	pSeriesClose->m_kind = CBCGPBaseChartStockSeries::SSK_CLOSE;
	pSeriesClose->m_pParentSeries = this;

	m_arChildSeries.Add(pSeriesClose);

	SetTreatNulls(CBCGPChartSeries::TN_SKIP, FALSE);
}
//****************************************************************************************
void CBCGPChartStockSeries::CommonInit()
{
	m_nOpenCloseBarSizePercent = 50;
	m_bIsMainStockSeries = TRUE;
	m_nOffsetFromNullZone = 2;

	m_nFixedGridSizeX = 16;
	m_nValuesPerIntervalX = 1;

	m_pfnCustomValueCallback = NULL;
}
//****************************************************************************************
CBCGPChartStockSeries::~CBCGPChartStockSeries()
{
	for (int i = 0; i < m_arChildSeries.GetSize(); i++)
	{
		CBCGPBaseChartStockSeries* pSeriesChild = DYNAMIC_DOWNCAST(CBCGPBaseChartStockSeries, m_arChildSeries[i]);
		delete pSeriesChild;
	}

	m_arChildSeries.RemoveAll();
}
//****************************************************************************************
CBCGPChartStockData CBCGPChartStockSeries::GetStockDataAt(int nDataPointIndex) const
{
	if (IsOptimizedLongDataMode())
	{
		return CBCGPBaseChartStockSeries::GetStockDataAt(nDataPointIndex);
	}

	if (!IsMainStockSeries())
	{
		return CBCGPChartStockData();
	}
	
	CBCGPChartStockData data;
	data.m_dateTime = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_X); 
	data.m_dblOpen = GetDataPointValue(nDataPointIndex);
	data.m_dblHigh = GetChildSeries(CBCGPChartStockSeries::CHART_STOCK_SERIES_HIGH_IDX)->GetDataPointValue(nDataPointIndex);
	data.m_dblLow = GetChildSeries(CBCGPChartStockSeries::CHART_STOCK_SERIES_LOW_IDX)->GetDataPointValue(nDataPointIndex);
	data.m_dblClose = GetChildSeries(CBCGPChartStockSeries::CHART_STOCK_SERIES_CLOSE_IDX)->GetDataPointValue(nDataPointIndex);
			
	return data;	
}
//****************************************************************************************
BOOL CBCGPChartStockSeries::OnGetDataPointTooltip(int nDataPointIndex, CString& strTooltip, CString& strTooltipDescr)
{
	if (!IsMainStockSeries() || m_pXAxis == NULL)
	{
		return TRUE;
	}

	strTooltip = m_strSeriesName;

	CBCGPBaseChartStockSeries* pCloseSeries = GetChildSeries(CBCGPChartStockSeries::CHART_STOCK_SERIES_CLOSE_IDX);
	ASSERT_VALID(pCloseSeries);
	
	CBCGPBaseChartStockSeries* pHighSeries = GetChildSeries(CBCGPChartStockSeries::CHART_STOCK_SERIES_HIGH_IDX);
	ASSERT_VALID(pHighSeries);
	
	CBCGPBaseChartStockSeries* pLowSeries = GetChildSeries(CBCGPChartStockSeries::CHART_STOCK_SERIES_LOW_IDX);
	ASSERT_VALID(pLowSeries);

	double dblOpen = GetDataPointValue(nDataPointIndex);
	double dblClose = pCloseSeries->GetDataPointValue(nDataPointIndex);
	double dblHigh = pHighSeries->GetDataPointValue(nDataPointIndex);
	double dblLow = pLowSeries->GetDataPointValue(nDataPointIndex);

	CString strTime;
	if (IsIndexMode())
	{
		m_pXAxis->GetDisplayedLabel(nDataPointIndex, strTime);
	}	
	else
	{
		CBCGPChartValue valX = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_X);
		m_pXAxis->GetDisplayedLabel(valX, strTime);
	}

	CString strFormat;
	strFormat.LoadString(IDS_BCGBARRES_CHART_STOCK_VALUE_FORMAT);
	strTooltipDescr.Format(strFormat, strTime, dblOpen, dblHigh, dblLow, dblClose);

	return TRUE;
}
//****************************************************************************************
CBCGPBaseChartStockSeries* CBCGPChartStockSeries::GetChildSeries(int nIdx) const
{
	if (nIdx < 0 || nIdx >= m_arChildSeries.GetSize())
	{
		return NULL;
	}

	return m_arChildSeries[nIdx];
}
//****************************************************************************************
void CBCGPChartStockSeries::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld)
{
	CBCGPChartSeries::OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);

	for (int i = 0; i < m_arChildSeries.GetSize(); i++)
	{
		CBCGPBaseChartStockSeries* pSeriesChild = DYNAMIC_DOWNCAST(CBCGPBaseChartStockSeries, m_arChildSeries[i]);

		ASSERT_VALID(pSeriesChild);

		if (pSeriesChild != NULL)
		{
			pSeriesChild->OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);
		}
	}

	m_downCandleStyle.OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);
	m_downBarStyle.SetScaleRatio(sizeScaleRatioNew);
}
//****************************************************************************************
void CBCGPChartStockSeries::EnableHistoryMode(BOOL bEnable, int nHistoryDepth, BOOL bReverseOrder, BOOL /*bSetDefaultValue*/, double /*dblDefaultYValue*/)
{
	ASSERT_VALID(this);

	CBCGPChartSeries::EnableHistoryMode(bEnable, nHistoryDepth, bReverseOrder, FALSE);

	for (int i = 0; i < m_arChildSeries.GetSize(); i++)
	{
		CBCGPBaseChartStockSeries* pSeriesChild = DYNAMIC_DOWNCAST(CBCGPBaseChartStockSeries, m_arChildSeries[i]);

		ASSERT_VALID(pSeriesChild);

		if (pSeriesChild != NULL)
		{
			pSeriesChild->m_bHistoryMode = bEnable;
			pSeriesChild->m_nHistoryDepth = nHistoryDepth;
			pSeriesChild->m_bUpdateAxesOnNewData = FALSE;
			pSeriesChild->m_bReverseAdd = bReverseOrder;
			pSeriesChild->m_nCurrentIndex = m_nCurrentIndex;
			pSeriesChild->m_bIndexMode = bEnable;
		}
	}
}
//****************************************************************************************
void CBCGPChartStockSeries::ResizeDataPointArray(int nNewSize)
{
	ASSERT_VALID(this);

	if (GetStockData().GetSize() > 0)
	{
		m_stockData.SetSize(nNewSize);
	}
	else
	{
		for (int i = 0; i < m_arChildSeries.GetSize(); i++)
		{
			CBCGPBaseChartStockSeries* pSeriesChild = DYNAMIC_DOWNCAST(CBCGPBaseChartStockSeries, m_arChildSeries[i]);
			
			ASSERT_VALID(pSeriesChild);
			
			if (pSeriesChild != NULL)
			{
				pSeriesChild->m_arDataPoints.SetSize(nNewSize);
			}
		}
		
		m_arDataPoints.SetSize(nNewSize);
	}
}
//****************************************************************************************
void CBCGPChartStockSeries::RecalcMinMaxValues()
{
	ASSERT_VALID(this);

	if (!IsMainStockSeries())
	{
		return;
	}

	CBCGPChartSeries::RecalcMinMaxValues();

	for (int i = GetMinDataPointIndex(); i <= GetMaxDataPointIndex(); i++)
	{
		CBCGPChartStockData data = GetStockDataAt(i);
		if (data.m_dblLow != 0 || data.m_dblLow == 0 && GetTreatNulls() == CBCGPChartSeries::TN_VALUE)
		{
			SetMinMaxValuesSimple(data.m_dblLow, CBCGPChartData::CI_Y, i);
		}

		if (data.m_dblHigh != 0 || data.m_dblHigh == 0 && GetTreatNulls() == CBCGPChartSeries::TN_VALUE)
		{
			SetMinMaxValuesSimple(data.m_dblHigh, CBCGPChartData::CI_Y, i);
		}
	}

	if (GetDataPointCount() > 1)
	{
		m_dblMinDataPointDistance = GetDataPointValue(1, CBCGPChartData::CI_X) - 
			GetDataPointValue(0, CBCGPChartData::CI_X);
	}
}
//****************************************************************************************
CBCGPChartValue CBCGPChartStockSeries::GetMinValue(CBCGPChartData::ComponentIndex ci)
{
	ASSERT_VALID(this);

	CBCGPChartValue minValue = CBCGPChartSeries::GetMinValue(ci);

	if (ci == CBCGPChartData::CI_X)
	{
		return minValue;
	}

	for (int i = 0; i < m_arChildSeries.GetSize(); i++)
	{
		CBCGPBaseChartStockSeries* pSeriesChild = DYNAMIC_DOWNCAST(CBCGPBaseChartStockSeries, m_arChildSeries[i]);

		ASSERT_VALID(pSeriesChild);

		if (pSeriesChild != NULL)
		{
			CBCGPChartValue minChildVal = pSeriesChild->GetMinValue(ci);

			if (!minChildVal.IsEmpty() || GetTreatNulls() == CBCGPChartSeries::TN_VALUE)
			{
				if (minValue.IsEmpty())
				{
					minValue = minChildVal;
				}
				else
				{
					minValue = min(minValue, minChildVal);
				}
			}
		}
	}
	return minValue;
}
//****************************************************************************************
CBCGPChartValue CBCGPChartStockSeries::GetMaxValue(CBCGPChartData::ComponentIndex ci)
{
	ASSERT_VALID(this);

	CBCGPChartValue maxValue = CBCGPChartSeries::GetMaxValue(ci);

	if (ci == CBCGPChartData::CI_X)
	{
		return maxValue;
	}

	for (int i = 0; i < m_arChildSeries.GetSize(); i++)
	{
		CBCGPBaseChartStockSeries* pSeriesChild = DYNAMIC_DOWNCAST(CBCGPBaseChartStockSeries, m_arChildSeries[i]);

		ASSERT_VALID(pSeriesChild);

		if (pSeriesChild != NULL)
		{
			CBCGPChartValue maxChildVal = pSeriesChild->GetMaxValue(ci);

			if (!maxChildVal.IsEmpty())
			{
				if (maxValue.IsEmpty())
				{
					maxValue = maxChildVal;
				}
				else
				{
					maxValue = max(maxValue, maxChildVal);
				}
			}
		}
	}

	return maxValue;
}
//****************************************************************************************
void CBCGPChartStockSeries::ClearMinMaxValues()
{
	ASSERT_VALID(this);

	CBCGPChartSeries::ClearMinMaxValues();

	for (int i = 0; i < m_arChildSeries.GetSize(); i++)
	{
		CBCGPBaseChartStockSeries* pSeriesChild = DYNAMIC_DOWNCAST(CBCGPBaseChartStockSeries, m_arChildSeries[i]);

		ASSERT_VALID(pSeriesChild);

		if (pSeriesChild != NULL)
		{
			pSeriesChild->ClearMinMaxValues();
		}
	}
}
//****************************************************************************************
void CBCGPChartStockSeries::SetCustomStockValueCallback(BCGPCHART_STOCK_CALLBACK pfn, BOOL bRedraw)
{
	m_pfnCustomValueCallback = pfn;
	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, bRedraw);
	}
}
//****************************************************************************************
void CBCGPChartStockSeries::OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea)
{
	ASSERT_VALID(this);

	CBCGPChartSeries::OnCalcScreenPoints(pGM, rectDiagramArea);

	for (int i = 0; i < m_arChildSeries.GetSize(); i++)
	{
		CBCGPBaseChartStockSeries* pSeriesChild = DYNAMIC_DOWNCAST(CBCGPBaseChartStockSeries, m_arChildSeries[i]);

		ASSERT_VALID(pSeriesChild);

		if (pSeriesChild != NULL)
		{
			pSeriesChild->OnCalcScreenPoints(pGM, rectDiagramArea);
		}
	}
}
//****************************************************************************************
void CBCGPChartStockSeries::OnCalcScreenPointsSimple(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea)
{
	if (!IsOptimizedLongDataMode())
	{
		CBCGPChartSeries::OnCalcScreenPointsSimple(pGM, rectDiagramArea);
	}
}
//****************************************************************************************
CBCGPPoint CBCGPChartStockSeries::GetDataPointScreenPoint(int nDataPointIndex, int nScreenPointIndex) const
{
	if (m_type == CBCGPBaseChartStockSeries::SST_CANDLE || m_type == CBCGPBaseChartStockSeries::SST_BAR)
	{
		return CBCGPChartSeries::GetDataPointScreenPoint(nDataPointIndex, nScreenPointIndex);
	}

	CBCGPChartDataPoint* pDP = NULL;

	switch (m_type)
	{
	case CBCGPBaseChartStockSeries::SST_LINE_OPEN:
		pDP = (CBCGPChartDataPoint*) GetDataPointAt(nDataPointIndex);
		break;

	case CBCGPBaseChartStockSeries::SST_LINE_HIGH:
		pDP = (CBCGPChartDataPoint*) GetChildSeries(CBCGPChartStockSeries::CHART_STOCK_SERIES_HIGH_IDX)->GetDataPointAt(nDataPointIndex);
		break;

	case CBCGPBaseChartStockSeries::SST_LINE_LOW:
		pDP = (CBCGPChartDataPoint*) GetChildSeries(CBCGPChartStockSeries::CHART_STOCK_SERIES_LOW_IDX)->GetDataPointAt(nDataPointIndex);
		break;

	case CBCGPBaseChartStockSeries::SST_LINE_CLOSE:
		pDP = (CBCGPChartDataPoint*) GetChildSeries(CBCGPChartStockSeries::CHART_STOCK_SERIES_CLOSE_IDX)->GetDataPointAt(nDataPointIndex);
		break;

	case CBCGPBaseChartStockSeries::SST_LINE_CUSTOM:
		{
			CBCGPChartValue valCustom = GetCustomStockValue(nDataPointIndex);
			CBCGPChartValue valX = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_X);
			CBCGPChartData data(valX, valCustom);
			BOOL bIsEmpty = FALSE;
			return m_pChart->ScreenPointFromChartData(data, nDataPointIndex, bIsEmpty, (CBCGPChartSeries*)this);
		}
		
		break;
	}

	if (pDP != NULL)
	{
		return pDP->GetScreenPoint();
	}

	return CBCGPPoint();
}
//****************************************************************************************
void CBCGPChartStockSeries::SetRelatedAxes(CBCGPChartAxis* pXAxis, CBCGPChartAxis* pYAxis, CBCGPChartAxis* pZAxis)
{
	ASSERT_VALID(this);

	CBCGPChartSeries::SetRelatedAxes(pXAxis, pYAxis, pZAxis);

	for (int i = 0; i < m_arChildSeries.GetSize(); i++)
	{
		CBCGPBaseChartStockSeries* pSeriesChild = DYNAMIC_DOWNCAST(CBCGPBaseChartStockSeries, m_arChildSeries[i]);

		ASSERT_VALID(pSeriesChild);

		if (pSeriesChild != NULL)
		{
			pSeriesChild->SetRelatedAxes(pXAxis, pYAxis, pZAxis);
		}
	}
}
//****************************************************************************************
void CBCGPChartStockSeries::SetRelatedAxis(CBCGPChartAxis* pAxis, CBCGPChartSeries::AxisIndex axisIndex)
{
	ASSERT_VALID(this);

	CBCGPChartSeries::SetRelatedAxis(pAxis, axisIndex);

	if (axisIndex == CBCGPChartSeries::AI_X)
	{
		pAxis->UseApproximation(FALSE);
		pAxis->EnableNegativeValuesForFixedInterval(FALSE);
		SetIndexMode(TRUE);
		pAxis->SetFixedIntervalWidth(m_nFixedGridSizeX, m_nValuesPerIntervalX);
		pAxis->EnableStretchFixedIntervals(FALSE);
	}

	if (axisIndex == CBCGPChartSeries::AI_Y)
	{
		pAxis->EnableIndependentZoom(TRUE);
		pAxis->EnableScroll(FALSE);
		pAxis->m_bNewMajorUnitBehavior = TRUE;
		pAxis->m_bUnzoomOnRangeBreak = TRUE;
	}

	for (int i = 0; i < m_arChildSeries.GetSize(); i++)
	{
		CBCGPBaseChartStockSeries* pSeriesChild = DYNAMIC_DOWNCAST(CBCGPBaseChartStockSeries, m_arChildSeries[i]);

		ASSERT_VALID(pSeriesChild);

		if (pSeriesChild != NULL)
		{
			pSeriesChild->SetRelatedAxis(pAxis, axisIndex);
		}
	}
}
//****************************************************************************************
void CBCGPChartStockSeries::SetIndexMode(BOOL bSet)
{
	ASSERT_VALID(this);
	
	if (IsMainStockSeries())
	{
		CBCGPChartSeries::SetIndexMode(bSet);
	}
	else
	{
		m_bIndexMode = bSet;
	}

	for (int i = 0; i < m_arChildSeries.GetSize(); i++)
	{
		CBCGPBaseChartStockSeries* pSeriesChild = DYNAMIC_DOWNCAST(CBCGPBaseChartStockSeries, m_arChildSeries[i]);
		
		ASSERT_VALID(pSeriesChild);
		
		if (pSeriesChild != NULL)
		{
			pSeriesChild->m_bIndexMode = bSet;
		}
	}
}
//****************************************************************************************
void CBCGPChartStockSeries::SetStockSeriesType(CBCGPBaseChartStockSeries::StockSeriesType type, BOOL bRedraw)
{
	ASSERT_VALID(this);

	m_type = type;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, bRedraw);
	}
}
//****************************************************************************************
void CBCGPChartStockSeries::SetTreatNulls(CBCGPChartSeries::TreatNulls tn, BOOL /*bRecalcMinMax*/)
{
	if (!IsMainStockSeries())
	{
		return;
	}

	CBCGPChartSeries::SetTreatNulls(tn, FALSE);

	for (int i = 0; i < m_arChildSeries.GetSize(); i++)
	{
		CBCGPBaseChartStockSeries* pSeriesChild = DYNAMIC_DOWNCAST(CBCGPBaseChartStockSeries, m_arChildSeries[i]);
		
		ASSERT_VALID(pSeriesChild);
		
		if (pSeriesChild != NULL)
		{
			pSeriesChild->SetTreatNulls(tn, FALSE);
		}
	}
}
//****************************************************************************************
void CBCGPChartStockSeries::AddData(double dblOpen, double dblHigh, double dblLow, double dblClose, 
									const COleDateTime& dateTime)
{
	ASSERT_VALID(this);

	if (IsMainStockSeries() && m_stockData.GetSize() > 0)
	{
		CBCGPChartStockData newData(dblOpen, dblHigh, dblLow, dblClose, dateTime);
		m_stockData.Add(newData);
	}
	else
	{
		CBCGPChartDataPoint dpOpen(dblOpen);
		CBCGPChartDataPoint dpHigh(dblHigh);
		CBCGPChartDataPoint dpLow(dblLow);
		CBCGPChartDataPoint dpClose(dblClose);
		
		if ((double)dateTime != 0. && dateTime.GetStatus() == COleDateTime::valid)
		{
			dpOpen.SetComponentValue((double)dateTime, CBCGPChartData::CI_X);
			dpHigh.SetComponentValue((double)dateTime, CBCGPChartData::CI_X);
			dpLow.SetComponentValue((double)dateTime, CBCGPChartData::CI_X);
			dpClose.SetComponentValue((double)dateTime, CBCGPChartData::CI_X);
		}
		
		AddDataPoint(dpOpen);
		
		m_arChildSeries[CHART_STOCK_SERIES_HIGH_IDX]->AddDataPoint(dpHigh);
		m_arChildSeries[CHART_STOCK_SERIES_LOW_IDX]->AddDataPoint(dpLow);
		m_arChildSeries[CHART_STOCK_SERIES_CLOSE_IDX]->AddDataPoint(dpClose);
	}
}
//****************************************************************************************
void CBCGPChartStockSeries::AddData(const CBCGPChartStockData& data)
{
	if (m_stockData.GetSize() > 0)
	{
		CBCGPChartStockData newData = data;
		m_stockData.Add(newData);
	}
	else
	{
		AddData(data.m_dblOpen, data.m_dblHigh, data.m_dblLow, data.m_dblClose, data.m_dateTime);
	}
}
//****************************************************************************************
void CBCGPChartStockSeries::AddData(const CBCGPStockDataArray& arStockData)
{
	m_stockData.Append(arStockData);
}
//****************************************************************************************
int CBCGPChartStockSeries::AddEmptyDataPoint(const CString& /*strCategoryName*/, DWORD_PTR /*dwUserData*/, BCGPChartFormatSeries* /*pDataPointFormat*/)
{
	ASSERT(FALSE);
	TRACE0("This method is not supported for Stock series; use AddEmptyDataPoint(double dblX, ...) instead\n");
	return -1;
}
//****************************************************************************************
int CBCGPChartStockSeries::AddEmptyDataPoint(DWORD_PTR /*dwUserData*/, BCGPChartFormatSeries* /*pDataPointFormat*/)
{
	ASSERT(FALSE);
	TRACE0("This method is not supported for Stock series; use AddEmptyDataPoint(double dblX, ...) instead\n");
	return -1;
}
//****************************************************************************************
int CBCGPChartStockSeries::AddEmptyDataPoint(double dblX, DWORD_PTR /*dwUserData*/, BCGPChartFormatSeries* /*pDataPointFormat*/)
{
	ASSERT_VALID(this);
	
	if (IsMainStockSeries() && m_stockData.GetSize() > 0)
	{
		CBCGPChartStockData newData(dblX);
		m_stockData.Add(newData);
		return (int)m_stockData.GetSize() - 1;
	}
	
	int nIdx = CBCGPChartSeries::AddEmptyDataPoint(dblX);
	
	m_arChildSeries[CHART_STOCK_SERIES_HIGH_IDX]->AddEmptyDataPoint(dblX);
	m_arChildSeries[CHART_STOCK_SERIES_LOW_IDX]->AddEmptyDataPoint(dblX);
	m_arChildSeries[CHART_STOCK_SERIES_CLOSE_IDX]->AddEmptyDataPoint(dblX);

	return nIdx;
}
//****************************************************************************************
// BAR Series - specific implementation
//****************************************************************************************
CBCGPChartBarSeries::CBCGPChartBarSeries()
{
	CommonInit();
}
//****************************************************************************************
CBCGPChartBarSeries::CBCGPChartBarSeries(CBCGPChartVisualObject* pChartCtrl,  
					BCGPChartCategory chartCategory, BCGPChartType chartType) :
			CBCGPChartSeries(pChartCtrl, chartCategory, chartType)
{
	CommonInit();
}
//****************************************************************************************
CBCGPChartBarSeries::CBCGPChartBarSeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
	 const CBCGPColor& seriesColor, BCGPChartCategory chartCategory, BCGPChartType chartType)
		   : CBCGPChartSeries(pChartCtrl, strSeriesName, seriesColor, chartCategory, chartType)
{
	CommonInit();
}

void CBCGPChartBarSeries::CommonInit()
{
	m_bCustomSize = FALSE;
	m_nCustomSizePercent = 50;

	m_bCustomOffset = 0;
	m_nCustomOffsetPercent = 0;

	m_nSeriesCountOnAxis = 0;
	m_FillGradienType = GetSeriesFillGradientType();

	if (m_chartType == BCGP_CT_SIMPLE)
	{
		m_formatSeries.m_dataLabelFormat.m_options.m_position = BCGPChartDataLabelOptions::LP_OUTSIDE_END;
	}
	else
	{
		m_formatSeries.m_dataLabelFormat.m_options.m_position = BCGPChartDataLabelOptions::LP_INSIDE_BASE;
	}

	m_formatSeries.m_legendLabelContent = BCGPChartDataLabelOptions::LC_CATEGORY_NAME;
}
//****************************************************************************************
int CBCGPChartBarSeries::GetColumnOverlapPercent() const 
{
	ASSERT_VALID(this);

	if (m_pXAxis != NULL)
	{
		return m_pXAxis->m_nColumnOverlapPercent;
	}

	return 0;
}
//****************************************************************************************
int CBCGPChartBarSeries::GetColumnDistancePercent() const 
{
	ASSERT_VALID(this);

	if (m_pXAxis != NULL)
	{
		return m_pXAxis->m_nColumnDistancePercent;
	}

	return 150;
}
//****************************************************************************************
void CBCGPChartBarSeries::SetColumnDistancePercent(int nPercent)
{
	ASSERT_VALID(this);

	if (m_pXAxis == NULL)
	{
		return;
	}

	if (nPercent < 0)
	{
		m_pXAxis->m_nColumnDistancePercent = 0;
	}
	else if (nPercent > 500)
	{
		m_pXAxis->m_nColumnDistancePercent = 500;
	}
	else
	{
		m_pXAxis->m_nColumnDistancePercent = nPercent;
	}
}
//****************************************************************************************
void CBCGPChartBarSeries::SetColumnOverlapPercent(int nPercent)
{
	ASSERT_VALID(this);

	if (m_pXAxis == NULL)
	{
		return;
	}

	if (nPercent < -100)
	{
		m_pXAxis->m_nColumnOverlapPercent = -100;
	}
	else if (nPercent > 100)
	{
		m_pXAxis->m_nColumnOverlapPercent = 100;
	}
	else
	{
		if (nPercent == 0)
		{
			nPercent = -1;
		}

		m_pXAxis->m_nColumnOverlapPercent = nPercent;
	}
}
//****************************************************************************************
void CBCGPChartBarSeries::CalcNumberOfSeriesOnAxis()
{
	ASSERT_VALID(this);

	if (m_pXAxis == NULL || m_pChart == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pChart);

	m_nSeriesCountOnAxis = 0;

	if (IsStakedSeries())
	{
		CMap<int, int, int, int> mapByGroup;
		// calculate number of stacked series on axis
		for (int i = 0; i < m_pChart->GetSeriesCount(); i++)
		{
			CBCGPChartBarSeries* pBarSeries = DYNAMIC_DOWNCAST(CBCGPChartBarSeries, m_pChart->GetSeries(i));
			if (pBarSeries == NULL || !pBarSeries->m_bVisible || !pBarSeries->IsStakedSeries() || 
				!pBarSeries->IsShownOnAxis(m_pXAxis) || 
				pBarSeries->IsCustomSize() || pBarSeries->IsCustomOffset())
			{
				continue;
			}
			int n;
			if (!mapByGroup.Lookup(pBarSeries->GetGroupID(), n))
			{
				mapByGroup.SetAt(pBarSeries->GetGroupID(), 0);
				m_nSeriesCountOnAxis++;
			}
		}
	}
	else
	{
		for (int i = 0; i < m_pChart->GetSeriesCount(); i++)
		{
			CBCGPChartBarSeries* pBarSeries = DYNAMIC_DOWNCAST(CBCGPChartBarSeries, m_pChart->GetSeries(i));
			if (pBarSeries == NULL || !pBarSeries->m_bVisible || pBarSeries->IsStakedSeries() || 
				!pBarSeries->IsShownOnAxis(m_pXAxis) || 
				pBarSeries->IsCustomSize() || pBarSeries->IsCustomOffset())
			{
				continue;
			}

			m_nSeriesCountOnAxis++;
		}
	}
}
//****************************************************************************************
void CBCGPChartBarSeries::CopyFrom(const CBCGPChartSeries& src)
{
	ASSERT_VALID(this);

	CBCGPChartSeries::CopyFrom(src);
	CBCGPChartBarSeries* pSrcSeries = DYNAMIC_DOWNCAST(CBCGPChartBarSeries, &src);

	if (pSrcSeries != NULL)
	{
		m_bCustomSize = pSrcSeries->IsCustomSize();
		m_nCustomSizePercent = pSrcSeries->GetCustomSizePercent();

		m_bCustomOffset = pSrcSeries->IsCustomOffset();
		m_nCustomOffsetPercent = pSrcSeries->GetCustomOffsetPercent();
	}
}
//****************************************************************************************
void CBCGPChartBarSeries::SetRelatedAxis(CBCGPChartAxis* pAxis, CBCGPChartSeries::AxisIndex axisIndex)
{
	CBCGPChartSeries::SetRelatedAxis(pAxis, axisIndex);

	if (axisIndex == CBCGPChartSeries::AI_X)
	{
		pAxis->SetVertical(m_chartCategory == BCGPChartBar || m_chartCategory == BCGPChartBar3D);
	}
	else if (axisIndex == CBCGPChartSeries::AI_Y)
	{
		pAxis->SetVertical(m_chartCategory == BCGPChartColumn || m_chartCategory == BCGPChartColumn3D);
	}
}
//****************************************************************************************
CBCGPBrush::BCGP_GRADIENT_TYPE CBCGPChartBarSeries::GetSeriesFillGradientType() const
{
	switch (m_chartCategory)
	{
	case BCGPChartBar3D:
		return CBCGPBrush::BCGP_GRADIENT_HORIZONTAL;

	case BCGPChartColumn3D:
		return CBCGPBrush::BCGP_GRADIENT_VERTICAL;

	case BCGPChartBar:
		return CBCGPBrush::BCGP_GRADIENT_PIPE_HORIZONTAL;

	case BCGPChartColumn:
	default:
		return CBCGPBrush::BCGP_GRADIENT_PIPE_VERTICAL;
	}
}
//****************************************************************************************
int CBCGPChartBarSeries::GetDefaultShadowAngle() const
{
	return (m_pXAxis != NULL && m_pXAxis->IsVertical()) ? 280 : 350;
}

//****************************************************************************************
// Histogram Series
//****************************************************************************************
CBCGPChartHistogramSeries::CBCGPChartHistogramSeries()
{
	CommonInit();
}
//****************************************************************************************
CBCGPChartHistogramSeries::CBCGPChartHistogramSeries(CBCGPChartVisualObject* pChartCtrl, BCGPChartType chartType) :
	CBCGPChartSeries(pChartCtrl, BCGPChartHistogram, chartType)
{
	CommonInit();
}
//****************************************************************************************
CBCGPChartHistogramSeries::CBCGPChartHistogramSeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
		const CBCGPColor& seriesColor, BCGPChartType chartType) : 
		CBCGPChartSeries(pChartCtrl, strSeriesName, seriesColor, BCGPChartHistogram, chartType)
{
	CommonInit();
}
//****************************************************************************************
void CBCGPChartHistogramSeries::CommonInit()
{
	if (m_chartType == BCGP_CT_SIMPLE)
	{
		m_formatSeries.m_dataLabelFormat.m_options.m_position = BCGPChartDataLabelOptions::LP_OUTSIDE_END;
	}
	else
	{
		m_formatSeries.m_dataLabelFormat.m_options.m_position = BCGPChartDataLabelOptions::LP_INSIDE_BASE;
	}
}
//****************************************************************************************
void CBCGPChartHistogramSeries::ClearMinMaxValues()
{
	ASSERT_VALID(this);
	
	CBCGPChartSeries::ClearMinMaxValues();
	
	if (!IsStakedSeries() && !IsRangeSeries() && !m_valOrigin.IsEmpty())
	{
		SetMinValue(m_valOrigin, CBCGPChartData::CI_Y);
		SetMaxValue(m_valOrigin, CBCGPChartData::CI_Y);
	}
}
//****************************************************************************************
void CBCGPChartHistogramSeries::SetOrigin(double dblValue, BOOL bRedraw)
{
	ASSERT_VALID(this);
	
	m_valOrigin.SetValue(dblValue);
	RecalcMinMaxValues();
	
	if (m_pChart != NULL && bRedraw)
	{
		ASSERT_VALID(m_pChart);
		m_pChart->Redraw();
	}
}
//****************************************************************************************
// POLAR Series
//****************************************************************************************
CBCGPChartPolarSeries::CBCGPChartPolarSeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
											 const CBCGPColor& seriesColor) : 
			CBCGPChartLineSeries(pChartCtrl, strSeriesName, seriesColor, BCGPChartLine, BCGP_CT_SIMPLE)
{
	m_chartCategory = BCGPChartPolar;
	CommonInit();
}
//****************************************************************************************
CBCGPChartPolarSeries::CBCGPChartPolarSeries(CBCGPChartVisualObject* pChartCtrl) : 
			CBCGPChartLineSeries(pChartCtrl)
{
	m_chartCategory = BCGPChartPolar;
	CommonInit();
}
//****************************************************************************************
CBCGPChartPolarSeries::~CBCGPChartPolarSeries()
{

}
//****************************************************************************************
void CBCGPChartPolarSeries::CommonInit()
{
	m_nPoints = 1;

	m_bConnectFirstLastPoints = TRUE;
	m_bFillClosedShape = FALSE;
}
//****************************************************************************************
void CBCGPChartPolarSeries::ShowOnPrimaryAxis(BOOL /*bPrimary*/)
{
	SetRelatedAxes(m_pChart->GetChartAxis(BCGP_CHART_X_POLAR_AXIS), 
					m_pChart->GetChartAxis(BCGP_CHART_Y_POLAR_AXIS));
}
//****************************************************************************************
void CBCGPChartPolarSeries::MakeRose(int nPoints, BOOL bFillArea)
{
	m_nPoints = bcg_clamp(nPoints, 1, 100);
	m_bFillClosedShape = bFillArea;
	m_bConnectFirstLastPoints = TRUE;
}
//****************************************************************************************
CBCGPPoint CBCGPChartPolarSeries::ScreenPointFromChartData(const CBCGPChartData& data, int nDataPointIndex)
{
	ASSERT_VALID(this);

	CBCGPChartAxisPolarX* pXAxis = DYNAMIC_DOWNCAST(CBCGPChartAxisPolarX, m_pXAxis);

	if (pXAxis == NULL || m_pYAxis == NULL)
	{
		return CBCGPPoint();
	}

	ASSERT_VALID(pXAxis);
	ASSERT_VALID(m_pYAxis);


	CBCGPPoint ptAxisStart;
	CBCGPPoint ptAxisEnd;

	m_pYAxis->GetAxisPos(ptAxisStart, ptAxisEnd);


	CBCGPChartValue valY = data.GetValue(CBCGPChartData::CI_Y);
	CBCGPChartValue valAngle = data.GetValue(CBCGPChartData::CI_X);

	double dblYScreen = m_pYAxis->PointFromValue(valY, FALSE);
	double dblOffset = m_pYAxis->IsVertical() ? ptAxisStart.y - dblYScreen : dblYScreen - ptAxisStart.x;

	CBCGPPoint pt = ptAxisStart;

	double dblAngle = !valAngle.IsEmpty() ? bcg_deg2rad(valAngle) : bcg_deg2rad(pXAxis->GetAngleFromIndex(nDataPointIndex));

	pt.x += dblOffset * sin(dblAngle);
	pt.y -= dblOffset * cos(dblAngle);

	return pt;
}
//****************************************************************************************
void CBCGPChartPolarSeries::OnCalcScreenPoints(CBCGPGraphicsManager* /*pGM*/, const CBCGPRect& /*rectDiagramArea*/)
{
	ASSERT_VALID(this);

	if (m_pChartFormula != NULL && m_pChartFormula->IsNonDiscreteCurve())
	{
		m_pChartFormula->GeneratePoints();
		return;
	}

	CBCGPChartAxisPolarX* pXAxis = DYNAMIC_DOWNCAST(CBCGPChartAxisPolarX, m_pXAxis);

	if (pXAxis == NULL || m_pYAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(pXAxis);
	ASSERT_VALID(m_pYAxis);

	CBCGPPoint ptAxisStart;
	CBCGPPoint ptAxisEnd;

	m_pYAxis->GetAxisPos(ptAxisStart, ptAxisEnd);

	for (int i = GetMinDataPointIndex(); i <= GetMaxDataPointIndex(); i++)
	{
		CBCGPChartDataPoint* pDp = (CBCGPChartDataPoint*)GetDataPointAt(i);
		if (pDp != NULL)
		{
			RemoveAllDataPointScreenPoints(i);

			CBCGPChartValue valY = GetDataPointValue(i, CBCGPChartData::CI_Y);
			CBCGPChartValue valAngle = GetDataPointValue(i, CBCGPChartData::CI_X);
			
			if (valY.IsEmpty() && GetTreatNulls() != CBCGPChartSeries::TN_VALUE)
			{
				continue;
			}

			double dblYScreen = m_pYAxis->PointFromValue(valY, FALSE);
			double dblOffset = m_pYAxis->IsVertical() ? ptAxisStart.y - dblYScreen : dblYScreen - ptAxisStart.x;

			CBCGPPoint pt = ptAxisStart;

			double dblAngle = !valAngle.IsEmpty() ? bcg_deg2rad(valAngle) : bcg_deg2rad(pXAxis->GetAngleFromIndex(i));

			pt.x += dblOffset * sin(dblAngle);
			pt.y -= dblOffset * cos(dblAngle);

			SetDataPointScreenPoint(i, 0, pt);
		}
	}
}
//****************************************************************************************
// POLAR Series
//****************************************************************************************
CBCGPChartTernarySeries::CBCGPChartTernarySeries()
{
	m_chartCategory = BCGPChartTernary;
	CommonInit();
}
//****************************************************************************************
CBCGPChartTernarySeries::CBCGPChartTernarySeries(CBCGPChartVisualObject* pChartCtrl) : CBCGPChartLineSeries(pChartCtrl)
{
	m_chartCategory = BCGPChartTernary;
	CommonInit();
}
//****************************************************************************************
CBCGPChartTernarySeries::CBCGPChartTernarySeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
												 const CBCGPColor& seriesColor) : 
								CBCGPChartLineSeries(pChartCtrl, strSeriesName, seriesColor, BCGPChartLine, BCGP_CT_SIMPLE)
{
	m_chartCategory = BCGPChartTernary;
	CommonInit();
}
//****************************************************************************************
CBCGPChartTernarySeries::~CBCGPChartTernarySeries()
{
	
}
//****************************************************************************************
void CBCGPChartTernarySeries::CommonInit()
{
	m_bIncludeDataPointLabelsToLegend = TRUE;
	m_bAutoColorDataPoints = TRUE;

	m_bConnectFirstLastPoints = TRUE;
	m_bFillClosedShape = FALSE;
	m_formatSeries.m_curveType = BCGPChartFormatSeries::CCT_NO_LINE;
	m_formatSeries.m_markerFormat.m_options.m_bShowMarker = TRUE;
	m_formatSeries.m_legendLabelContent = BCGPChartDataLabelOptions::LC_DEFAULT_CONTENT;

	m_bShowPercentsInTooltip = TRUE;
}
//****************************************************************************************
void CBCGPChartTernarySeries::ShowOnPrimaryAxis(BOOL /*bPrimary*/)
{
	SetRelatedAxes(m_pChart->GetChartAxis(BCGP_CHART_A_TERNARY_AXIS), 
		m_pChart->GetChartAxis(BCGP_CHART_B_TERNARY_AXIS),
		m_pChart->GetChartAxis(BCGP_CHART_C_TERNARY_AXIS));
}
//****************************************************************************************
CBCGPPoint CBCGPChartTernarySeries::ScreenPointFromChartData(const CBCGPChartData& data, int /*nDataPointIndex*/)
{
	ASSERT_VALID(this);

	if (m_pXAxis == NULL)
	{
		return CBCGPPoint();
	}

	ASSERT_VALID(m_pXAxis);
	
	return ((CBCGPChartTernaryAxis*)m_pXAxis)->PointFromChartData(data);
}
//****************************************************************************************
void CBCGPChartTernarySeries::OnCalcScreenPoints(CBCGPGraphicsManager* /*pGM*/, const CBCGPRect& /*rectDiagramArea*/)
{
	for (int i = 0; i < GetDataPointCount(); i++)
	{
		CBCGPChartDataPoint* pDp = (CBCGPChartDataPoint*)GetDataPointAt(i);
		RemoveAllDataPointScreenPoints(i);

		if (pDp == NULL)
		{
			continue;
		}

		if ((pDp->IsEmpty() || 
			pDp->GetComponentValue(CBCGPChartData::CI_Y).IsEmpty() && 
			pDp->GetComponentValue(CBCGPChartData::CI_Y1).IsEmpty() && 
			pDp->GetComponentValue(CBCGPChartData::CI_Y2).IsEmpty()) && 
			GetTreatNulls() != CBCGPChartSeries::TN_VALUE)
		{
			pDp->m_bIncludeLabelToLegend = FALSE;
			continue;
		}

		CBCGPPoint pt = ((CBCGPChartTernaryAxis*)m_pXAxis)->PointFromChartData(pDp->GetData());
		SetDataPointScreenPoint(i, 0, pt);
	}
}
//****************************************************************************************
void CBCGPChartTernarySeries::SetMinMaxValues(const CBCGPChartValue& val, CBCGPChartData::ComponentIndex ci, int nDataPointIndex)
{
	CBCGPChartSeries::SetMinMaxValuesSimple(val, ci, nDataPointIndex);
	if (ci == CBCGPChartData::CI_Y2)
	{
		double dblAPerc = GetDataPointComponentPercent(nDataPointIndex, CBCGPChartData::CI_Y);
		double dblBPerc = GetDataPointComponentPercent(nDataPointIndex, CBCGPChartData::CI_Y1);
		double dblCPerc = GetDataPointComponentPercent(nDataPointIndex, CBCGPChartData::CI_Y2);

		SetMinMaxPercentage(dblAPerc, CBCGPChartData::CI_Y);
		SetMinMaxPercentage(dblBPerc, CBCGPChartData::CI_Y1);
		SetMinMaxPercentage(dblCPerc, CBCGPChartData::CI_Y2);
	}
}
//****************************************************************************************
CBCGPChartValue CBCGPChartTernarySeries::GetMinPercentage(CBCGPChartData::ComponentIndex ci) const
{
	return m_minPercentage.GetValue(ci);
}
//****************************************************************************************
CBCGPChartValue CBCGPChartTernarySeries::GetMaxPercentage(CBCGPChartData::ComponentIndex ci) const
{
	return m_maxPercentage.GetValue(ci);
}
//****************************************************************************************
void CBCGPChartTernarySeries::SetMinMaxPercentage(double dblValue, CBCGPChartData::ComponentIndex ci)
{
	CBCGPChartValue minValue = GetMinPercentage(ci);
	CBCGPChartValue maxValue = GetMaxPercentage(ci);

	minValue.IsEmpty() ? m_minPercentage.SetValue(dblValue, ci) : m_minPercentage.SetValue(min(minValue, dblValue), ci);
	maxValue.IsEmpty() ? m_maxPercentage.SetValue(dblValue, ci) : m_maxPercentage.SetValue(max(maxValue, dblValue), ci);
}
//****************************************************************************************
BOOL CBCGPChartTernarySeries::GetDataPointLabelText(int nDataPointIndex, BCGPChartDataLabelOptions::LabelContent content, 
											 CString& strDPLabel)
{
	const BCGPChartFormatSeries* pFormatSeries = GetDataPointFormat(nDataPointIndex, FALSE);
	BOOL bNeedSeparator = FALSE;
	CString strFormat = pFormatSeries->m_dataLabelFormat.m_options.m_strDataFormat;

	if (strFormat.IsEmpty())
	{
		strFormat = _T("%.9G");
	}

	CString strPercentFormat = _T("%.2f%%");

	if (content == BCGPChartDataLabelOptions::LC_DEFAULT_CONTENT)
	{
		content = (BCGPChartDataLabelOptions::LabelContent)(BCGPChartDataLabelOptions::LC_CATEGORY_NAME | BCGPChartDataLabelOptions::LC_ALL_TERNARY_PERCENTAGE);
	}

	if ((content & BCGPChartDataLabelOptions::LC_SERIES_NAME) == BCGPChartDataLabelOptions::LC_SERIES_NAME)
	{
		strDPLabel += m_strSeriesName;
		if (!m_strSeriesName.IsEmpty())
		{
			bNeedSeparator = TRUE;
		}
	}

	if ((content & BCGPChartDataLabelOptions::LC_CATEGORY_NAME) == BCGPChartDataLabelOptions::LC_CATEGORY_NAME)
	{
		CString strCategory = GetDataPointCategoryName(nDataPointIndex);
		strDPLabel += strCategory;
		
		if (!strCategory.IsEmpty())
		{
			bNeedSeparator = TRUE;
		}
	}
	
	if ((content & BCGPChartDataLabelOptions::LC_A_TERNARY_VALUE) == BCGPChartDataLabelOptions::LC_A_TERNARY_VALUE)
	{
		if (bNeedSeparator)
		{
			strDPLabel += pFormatSeries->m_dataLabelFormat.m_options.m_strLabelSeparator;
		}

		double dblVal = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y).GetValue();

		CString str;
		str.Format(strFormat, dblVal);

		strDPLabel += str;
		bNeedSeparator = TRUE;
	}

	if ((content & BCGPChartDataLabelOptions::LC_A_TERNARY_PERCENTAGE) == BCGPChartDataLabelOptions::LC_A_TERNARY_PERCENTAGE)
	{
		if (bNeedSeparator)
		{
			strDPLabel += pFormatSeries->m_dataLabelFormat.m_options.m_strLabelSeparator;
		}

		double dblVal = GetDataPointComponentPercent(nDataPointIndex, CBCGPChartData::CI_Y);

		CString str;
		str.Format(strPercentFormat, dblVal);

		strDPLabel += str;
		bNeedSeparator = TRUE;
	}

	if ((content & BCGPChartDataLabelOptions::LC_B_TERNARY_VALUE) == BCGPChartDataLabelOptions::LC_B_TERNARY_VALUE)
	{
		if (bNeedSeparator)
		{
			strDPLabel += pFormatSeries->m_dataLabelFormat.m_options.m_strLabelSeparator;
		}

		double dblVal = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y1).GetValue();

		CString str;
		str.Format(strFormat, dblVal);

		strDPLabel += str;
		bNeedSeparator = TRUE;
	}

	if ((content & BCGPChartDataLabelOptions::LC_B_TERNARY_PERCENTAGE) == BCGPChartDataLabelOptions::LC_B_TERNARY_PERCENTAGE)
	{
		if (bNeedSeparator)
		{
			strDPLabel += pFormatSeries->m_dataLabelFormat.m_options.m_strLabelSeparator;
		}

		double dblVal = GetDataPointComponentPercent(nDataPointIndex, CBCGPChartData::CI_Y1);

		CString str;
		str.Format(strPercentFormat, dblVal);

		strDPLabel += str;

		bNeedSeparator = TRUE;
	}

	if ((content & BCGPChartDataLabelOptions::LC_C_TERNARY_VALUE) == BCGPChartDataLabelOptions::LC_C_TERNARY_VALUE)
	{
		if (bNeedSeparator)
		{
			strDPLabel += pFormatSeries->m_dataLabelFormat.m_options.m_strLabelSeparator;
		}

		double dblVal = GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y2).GetValue();

		CString str;
		str.Format(strFormat, dblVal);

		strDPLabel += str;

		bNeedSeparator = TRUE;
	}

	if ((content & BCGPChartDataLabelOptions::LC_C_TERNARY_PERCENTAGE) == BCGPChartDataLabelOptions::LC_C_TERNARY_PERCENTAGE)
	{
		if (bNeedSeparator)
		{
			strDPLabel += pFormatSeries->m_dataLabelFormat.m_options.m_strLabelSeparator;
		}

		double dblVal = GetDataPointComponentPercent(nDataPointIndex, CBCGPChartData::CI_Y2);

		CString str;
		str.Format(strPercentFormat, dblVal);

		strDPLabel += str;
	}

	return TRUE;
}
//****************************************************************************************
double CBCGPChartTernarySeries::GetDataPointComponentPercent(int nDataPointIndex, CBCGPChartData::ComponentIndex ci) const
{
	double dblA = fabs(GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y));
	double dblB = fabs(GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y1));
	double dblC = fabs(GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y2));

	double dblSum = dblA + dblB + dblC;

	if (dblSum == 0)
	{
		return 0;
	}

	if (ci == CBCGPChartData::CI_Y)
	{
		return dblA / dblSum * 100.;
	}
	else if (ci == CBCGPChartData::CI_Y1)
	{
		return dblB / dblSum * 100.;
	}

	return dblC / dblSum * 100.;
}
//****************************************************************************************
BOOL CBCGPChartTernarySeries::OnGetDataPointTooltip(int nDataPointIndex, CString& strTooltip, CString& strTooltipDescr)
{
	CBCGPChartAxis* pAxisA = m_pChart->GetChartAxis(BCGP_CHART_A_TERNARY_AXIS);
	CBCGPChartAxis* pAxisB = m_pChart->GetChartAxis(BCGP_CHART_B_TERNARY_AXIS);
	CBCGPChartAxis* pAxisC = m_pChart->GetChartAxis(BCGP_CHART_C_TERNARY_AXIS);

	if (pAxisA == NULL || pAxisB == NULL || pAxisC == NULL)
	{
		return FALSE;
	}

	strTooltip = m_strSeriesName;
	
	if (m_strSeriesName.IsEmpty())
	{
		strTooltip = _T("Series");
	}

	double dblA = m_bShowPercentsInTooltip ? 
			GetDataPointComponentPercent(nDataPointIndex, CBCGPChartData::CI_Y) :
			GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y);
	
	double dblB = m_bShowPercentsInTooltip ? 
			GetDataPointComponentPercent(nDataPointIndex, CBCGPChartData::CI_Y1) :
			GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y1);

	double dblC = m_bShowPercentsInTooltip ? 
			GetDataPointComponentPercent(nDataPointIndex, CBCGPChartData::CI_Y2) :
			GetDataPointValue(nDataPointIndex, CBCGPChartData::CI_Y2);

	CString strFormat;
	
	if (m_bShowPercentsInTooltip)
	{
		strFormat = _T("%.2f%%");
	}
	else 
	{
		const BCGPChartFormatSeries* pFormatSeries = GetDataPointFormat(nDataPointIndex, FALSE);
		strFormat = pFormatSeries->m_dataLabelFormat.m_options.m_strDataFormat;
		if (strFormat.IsEmpty())
		{
			strFormat = _T("%.9G");
		}
	}	

	CString strA;
	CString strB;
	CString strC;

	CString strPercentFormat = _T("%.2f%%");
	strA.Format(strPercentFormat, dblA);
	strB.Format(strPercentFormat, dblB);
	strC.Format(strPercentFormat, dblC);

	CString strFormatDescr;
	strFormatDescr.LoadString(IDS_BCGBARRES_CHART_TOOLTIP_TERNARY_VALUE_FORMAT);

	strTooltipDescr.Format(strFormatDescr, pAxisA->m_strAxisName, strA,
											pAxisB->m_strAxisName, strB, 
											pAxisC->m_strAxisName, strC);

	return TRUE;
}
