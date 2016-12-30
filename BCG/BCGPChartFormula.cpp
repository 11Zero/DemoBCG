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
// BCGPChartFormula.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPChartSeries.h"
#include "BCGPChartFormula.h"
#include "BCGPChartVisualObject.h"
#include "BCGPMath.h"

IMPLEMENT_DYNAMIC(CBCGPChartBaseFormula, CObject)
IMPLEMENT_DYNAMIC(CBCGPChartAdvancedFormula, CBCGPChartBaseFormula)
IMPLEMENT_DYNCREATE(CBCGPChartMAFormula, CBCGPChartAdvancedFormula)
IMPLEMENT_DYNCREATE(CBCGPChartRSIFormula, CBCGPChartAdvancedFormula)
IMPLEMENT_DYNCREATE(CBCGPChartStochasticFormula, CBCGPChartAdvancedFormula)
IMPLEMENT_DYNCREATE(CBCGPChartBollingerBandsFormula, CBCGPChartAdvancedFormula)
IMPLEMENT_DYNCREATE(CBCGPChartMACDFormula, CBCGPChartAdvancedFormula)
IMPLEMENT_DYNCREATE(CBCGPChartTransitionFormula, CBCGPChartBaseFormula)
IMPLEMENT_DYNCREATE(CBCGPChartTrendFormula, CBCGPChartBaseFormula)
IMPLEMENT_DYNCREATE(CBCGPChartVirtualFormula, CBCGPChartTrendFormula)

static BOOL CreatePolynomial(const double* X, const double* Y, int count, int order, CBCGPMatrix& matrix, CBCGPVector& vector)
{
	if (count < 2)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	order = bcg_clamp(bcg_clamp(order, 1, count - 1), 1, 6);
	const int size = order + 1;

	matrix.Create(size, size, 0.0);
	vector.Create(size, 0.0);

	CBCGPMatrix matrixX2(size, count, 1.0);

	int y = 0;
	int x = 0;
	for (y = 1; y < size; y++)
	{
		memcpy(matrixX2[y], matrixX2[y - 1], sizeof(double) * count);

		for (x = 0; x < count; x++)
		{
			matrixX2[y][x] *= X[x];
		}
	}

	for (y = 0; y < size; y++)
	{
		double* pRow = matrixX2[y];
		for (x = 0; x < count; x++)
		{
			vector[y] += pRow[x] * Y[x];
		}
	}

	double* pRow = matrix[0];
	*pRow++ = count;
	for (x = 1; x < size; x++)
	{
		*pRow++ = matrixX2.SumRow(x);
	}

	for (y = 1; y < size; y++)
	{
		double* pData = matrix[y];
		memcpy(pData, matrix[y - 1] + 1, sizeof(double) * order);
		pData += (size - 1);

		double* pRowFirst = matrixX2[1];
		double* pRowLast  = matrixX2[size - 1];
 		for (x = 0; x < count; x++)
 		{
			*pRowLast *= *pRowFirst++;
			*pData += *pRowLast++;
 		}
	}

	return TRUE;
}

static BOOL SolveGauss(CBCGPMatrix& matrix, CBCGPVector& vector, CBCGPVector& result)
{
	result.Destroy();

	int size = matrix.GetRows ();
	if (size != matrix.GetCols () || size != vector.GetSize ())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	int x = 0;
	int y = 0;
	int i = 0;
	BOOL bRes = TRUE;

	for(y = 0; y < size; y++)
	{
		if(matrix[y][y] != 0.0)
		{
			continue;
		}

		for(x = 0; x < size; x++)
		{
			if(y == x || matrix[x][y] == 0.0 || matrix[y][x] == 0.0)
			{
				continue;
			}

			double temp = 0.0;
			for(int k = 0; k < size; k++)
			{
				temp = matrix[x][k];
				matrix[x][k] = matrix[y][k];
				matrix[y][k] = temp;
			}

			temp = vector[x];
			vector[x] = vector[y];
			vector[y] = temp;

			break;
		}
	}

	// process
	for(i = 0; i < size; i++)
	{
		for(y = i + 1; y < size; y++)
		{
			if(matrix[i][i] == 0.0)
			{
				bRes = FALSE;
				break;
			}

			double M = matrix[y][i] / matrix[i][i];
			for(x = i; x < size; x++)
			{
				matrix[y][x] -= M * matrix[i][x];
			}

			vector[y] -= M * vector[i];
		}

		if (!bRes)
		{
			break;
		}
	}

	if (bRes)
	{
		result.Create(size, 0.0);

		for(y = size - 1; y >= 0; y--)
		{
			double sum = 0.0;
			for(x = y; x < size; x++)
			{
				sum += matrix[y][x] * result[x];
			}

			result[y] = (vector[y] - sum) / matrix[y][y];
		}
	}

	return bRes;
}

static BOOL PolynomialRegression(const double* X, const double* Y, int count, int order, CArray<double, double>& result)
{
	result.RemoveAll ();

	CBCGPMatrix matrix;
	CBCGPVector vector;
	if (!CreatePolynomial (X, Y, count, order, matrix, vector))
	{
		return FALSE;
	}

	CBCGPVector vector_result;
	if (!SolveGauss (matrix, vector, vector_result))
	{
		return FALSE;
	}

	int size = vector_result.GetSize ();
	if (size == 0)
	{
		return FALSE;
	}

	vector_result.Swap();
	result.SetSize(size, 0);
	memcpy (result.GetData (), vector_result.GetData (), sizeof(double) * size);

	return TRUE;
}
//*******************************************************************************
// CBCGPChartBaseFormula
//*******************************************************************************
CBCGPChartBaseFormula::CBCGPChartBaseFormula()
{
	CommonInit();
}
//*******************************************************************************
void CBCGPChartBaseFormula::CommonInit()
{
	m_pParentSeries = NULL;
	m_lParam = NULL;
}
//*******************************************************************************
void CBCGPChartBaseFormula::SetParentSeries(CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);

	m_pParentSeries = pSeries;

	if (m_pParentSeries != NULL)
	{
		ASSERT_VALID(m_pParentSeries);
	}
}
//*******************************************************************************
CBCGPChartSeries* CBCGPChartBaseFormula::GetInputSeriesAt(int nIdx) const
{
	ASSERT_VALID(this);

	if (nIdx < 0 || nIdx >= GetInputSeriesCount())
	{
		return NULL;
	}

	return m_arInputSeries [nIdx];
}
//****************************************************************************************
int CBCGPChartBaseFormula::FindInputSeriesIndex(CBCGPChartSeries* pSeries) const
{
	for (int i = 0; i < GetInputSeriesCount(); i++)
	{
		CBCGPChartSeries* p = GetInputSeriesAt(i);

		if (p == pSeries)
		{
			return i;
		}
	}

	return -1;
}
//*******************************************************************************
void CBCGPChartBaseFormula::CopyFrom(const CBCGPChartBaseFormula& src)
{
	m_pParentSeries = src.GetParentSeries();
	m_lParam = src.GetLParam();

	for (int i = 0; i < src.GetInputSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = src.GetInputSeriesAt(i);

		if (pSeries != NULL)
		{
			ASSERT_VALID(pSeries);
			m_arInputSeries.Add(pSeries);
		}
	}
}
//*******************************************************************************
CBCGPChartBaseFormula& CBCGPChartBaseFormula::operator=(const CBCGPChartBaseFormula& src)
{
	ASSERT_VALID(this);

	CopyFrom(src);
	return *this;

}
//*******************************************************************************
double CBCGPChartBaseFormula::CalcEMA(double dblCurrValue, double dblPrevValue, double dblEmaKoef)
{
	if (dblPrevValue == 0 || dblEmaKoef == 0)
	{
		return dblCurrValue;
	}

	return dblCurrValue * dblEmaKoef + dblPrevValue * (1 - dblEmaKoef);
} 
//*******************************************************************************
double CBCGPChartBaseFormula::CalcEMAOnArray(double* pInput, int nPeriod, double dblEMAKoef)
{
	ASSERT(pInput != NULL);

	if (pInput == NULL)
	{
		return 0;
	}

	if (nPeriod == 1)
	{
		return pInput[0];
	}

	if (dblEMAKoef == -1.0)
	{
		dblEMAKoef = 2.0 / (double)(nPeriod + 1.0);
	}

	double dblSum = 0;
	for (int i = 0; i < nPeriod - 1; i++)
	{
		dblSum += pInput[i];
	}

	dblSum /= (double)(nPeriod - 1);
	return CalcEMA(pInput[nPeriod - 1], dblSum, dblEMAKoef);
}
//*******************************************************************************
double CBCGPChartBaseFormula::CalcSMAOnArray(double* pInput, int nPeriod)
{
	ASSERT(pInput != NULL);
	
	if (pInput == NULL)
	{
		return 0;
	}

	double dblSum = 0;
	for (int i = 0; i < nPeriod; i++)
	{
		dblSum += pInput[i];
	}

	return dblSum / nPeriod;
}

//*******************************************************************************
// CBCGPChartAdvancedFormula
//*******************************************************************************
CBCGPChartSeries* CBCGPChartAdvancedFormula::Create(CBCGPChartVisualObject* pChart, CString strFormulaName, 
											   CBCGPChartSeries* pInputSeries, CBCGPChartAxis* pYAxis, CBCGPChartAxis* pXAxis)
{
	m_pParentSeries = pChart->CreateSeries(strFormulaName, CBCGPColor(), GetOutputSeriesType(), GetOutputSeriesCategory()); 
	
	if (pInputSeries != NULL)
	{
		m_arInputSeries.Add(pInputSeries);
	}
	
	m_pParentSeries->SetFormula(*this);

	if (pYAxis != NULL)
	{
		m_pParentSeries->SetRelatedAxis(pYAxis, CBCGPChartSeries::AI_Y);
	}

	if (pXAxis != NULL)
	{
		m_pParentSeries->SetRelatedAxis(pXAxis, CBCGPChartSeries::AI_X);
	}

	m_pParentSeries->SetTreatNulls(CBCGPChartSeries::TN_SKIP);
	
	return m_pParentSeries;
}
//*******************************************************************************
void CBCGPChartAdvancedFormula::SetPeriod(int nPeriod, BOOL bGeneratePoints)
{
	if (nPeriod < 1)
	{
		nPeriod = 1;
	}

	m_dblEMAKoef = 2.0 / (double)(nPeriod + 1.0);
	m_nPeriod = nPeriod;

	if (bGeneratePoints)
	{
		GeneratePoints();
	}
	
}
//*******************************************************************************
void CBCGPChartAdvancedFormula::CopyFrom(const CBCGPChartBaseFormula& src)
{
	CBCGPChartBaseFormula::CopyFrom(src);

	const CBCGPChartAdvancedFormula& adv = (const CBCGPChartAdvancedFormula&)src;
	SetPeriod(adv.GetPeriod(), FALSE);
	m_bUseLongData = adv.m_bUseLongData;
}
//*******************************************************************************
void CBCGPChartAdvancedFormula::GeneratePoints()
{
	if (m_pParentSeries == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	m_pParentSeries->RemoveAllDataPoints();
	UpdateLastDataPoints();
}

//*******************************************************************************
void CBCGPChartAdvancedFormula::UpdateLastDataPoints(int nCount)
{
	if (m_pParentSeries == NULL)
	{
		ASSERT(FALSE);
		return;
	}
	
	CBCGPChartSeries* pInputSeries = GetInputSeriesAt(0);
	
	if (pInputSeries == NULL)
	{
		return;
	}

	if (nCount == -1)
	{
		nCount = pInputSeries->GetDataPointCount() - m_pParentSeries->GetDataPointCount();
	}

	if (nCount <= 0)
	{
		return;
	}

	if (m_bUseLongData)
	{
		m_pParentSeries->ResizeLongDataArray(nCount);
		BOOL bXSet = FALSE;
		BOOL bY1Set = FALSE;

		for (int i = max(0, pInputSeries->GetDataPointCount() - nCount); i < pInputSeries->GetDataPointCount(); i++)
		{
			CBCGPChartValue valY1;
			CBCGPChartValue valY = CalculateDataPoint(i, pInputSeries, valY1);
			
			const CBCGPChartValue& valX = pInputSeries->GetDataPointValue(i, CBCGPChartData::CI_X);

			if (valY.IsEmpty())
			{
				m_pParentSeries->SetDataPointValue(i, m_pParentSeries->GetLongDataEmptyValue());
			}
			else
			{
				m_pParentSeries->SetDataPointValue(i, valY);
			}
			

			if (!valX.IsEmpty())
			{
				if (!bXSet)
				{
					m_pParentSeries->ResizeLongDataArray(nCount, TRUE, CBCGPChartData::CI_X);
					bXSet = TRUE;
				}
				
				m_pParentSeries->SetDataPointValue(i, valX, CBCGPChartData::CI_X);
			}

			if (!valY1.IsEmpty())
			{
				if (!bY1Set)
				{
					m_pParentSeries->ResizeLongDataArray(nCount, TRUE, CBCGPChartData::CI_Y1);
					bY1Set = TRUE;
				}

				m_pParentSeries->SetDataPointValue(i, valY1, CBCGPChartData::CI_Y1);
			}
		}
	}
	else
	{
		for (int i = max(0, pInputSeries->GetDataPointCount() - nCount); i < pInputSeries->GetDataPointCount(); i++)
		{
			CBCGPChartValue valY1;
			CBCGPChartValue valY = CalculateDataPoint(i, pInputSeries, valY1);

			const CBCGPChartValue& valX = pInputSeries->GetDataPointValue(i, CBCGPChartData::CI_X);

			int nIndex = i;

			if (m_pParentSeries->GetDataPointCount() - 1 < i)
			{
				CBCGPChartDataPoint dp(valY);
				dp.SetComponentValue(valX, CBCGPChartData::CI_X);
				nIndex = m_pParentSeries->AddDataPoint(dp);
			}
			else
			{
				if (!valX.IsEmpty())
				{
					m_pParentSeries->SetDataPointValue(i, valX.GetValue(), CBCGPChartData::CI_X);
				}
				if (!valY.IsEmpty())
				{
					m_pParentSeries->SetDataPointValue(i, valY.GetValue(), CBCGPChartData::CI_Y);
				}
			}

			if (!valY1.IsEmpty())
			{
				m_pParentSeries->SetDataPointValue(nIndex, valY1.GetValue(), CBCGPChartData::CI_Y1);
			}
		}
	}
}
//*******************************************************************************
double CBCGPChartAdvancedFormula::CalcPrevSum(int nIndexStart, int nOrder, CBCGPChartSeries* pInputSeries, int& nActual)
{
	double dblSum = 0;
	int nCount = 0;

	for (int i = nIndexStart; i >= 0; i--)
	{
		dblSum += pInputSeries->GetDataPointValue(i, CBCGPChartData::CI_Y);
		nCount++;

		if (nCount >= nOrder)
		{
			break;
		}
	}

	nActual = nCount;

	return dblSum;
}
//*******************************************************************************
double CBCGPChartAdvancedFormula::CalcWeightedSum(int nIndexStart, int nOrder, CBCGPChartSeries* pInputSeries)
{
	int nWeight = nOrder;
	int nTotalWeight = 0;
	int nSteps = 0;
	double dblWeightedSum = 0;
	for (int i = nIndexStart; i >= 0 && nSteps < nOrder; i--, nSteps++, nWeight--)
	{
		double dblVal = pInputSeries->GetDataPointValue(i, CBCGPChartData::CI_Y);
		dblWeightedSum += dblVal * nWeight;
		nTotalWeight += nWeight;
	}

	return dblWeightedSum / nTotalWeight;
}
//*******************************************************************************
// CBCGPChartMAFormula - moving average
//*******************************************************************************
CBCGPChartMAFormula::CBCGPChartMAFormula(CBCGPChartMAFormula::MovingAverageType maType, int nPeriod) : CBCGPChartAdvancedFormula(nPeriod)
{
	m_maType = maType;
}
//*******************************************************************************
void CBCGPChartMAFormula::SetMAType(CBCGPChartMAFormula::MovingAverageType maType)
{
	if (m_maType != maType)
	{
		m_maType = maType;
		GeneratePoints();
	}
}
//*******************************************************************************
void CBCGPChartMAFormula::CopyFrom(const CBCGPChartBaseFormula& src)
{
	CBCGPChartAdvancedFormula::CopyFrom(src);
	m_maType = ((const CBCGPChartMAFormula&)src).GetMAType();
}

//*******************************************************************************
CBCGPChartValue CBCGPChartMAFormula::CalculateDataPoint(int nIndex, CBCGPChartSeries* pInputSeries, CBCGPChartValue& /*valY1*/)
{
	double dblY = 0.;

	if (nIndex == 0 || m_nPeriod == 1)
	{
		dblY = pInputSeries->GetDataPointValue(nIndex, CBCGPChartData::CI_Y);
	}
	else
	{
		int nActual = 1;
		
		switch(m_maType)
		{
		case MA_SIMPLE:
			dblY = CalcPrevSum(nIndex - 1, m_nPeriod - 1, pInputSeries, nActual);
			dblY += pInputSeries->GetDataPointValue(nIndex, CBCGPChartData::CI_Y);
			dblY /= (nActual + 1);
			break;
		case MA_EXPONENTIAL:
			{
				double dblPrevValue = 0;
				if (nIndex < m_nPeriod)
				{
					dblPrevValue = CalcPrevSum(nIndex - 1, m_nPeriod, pInputSeries, nActual);
					dblPrevValue /= nActual;
				}
				else
				{
					dblPrevValue = m_pParentSeries->GetDataPointValue(nIndex - 1, CBCGPChartData::CI_Y);
				}
				
				dblY = pInputSeries->GetDataPointValue(nIndex, CBCGPChartData::CI_Y);
				dblY = CalcEMA(dblY, dblPrevValue, m_dblEMAKoef);
			}
			break;
			
		case MA_SMOOTHED:
			{
				double dblPrevValue = m_pParentSeries->GetDataPointValue(nIndex - 1, CBCGPChartData::CI_Y);
				dblY = pInputSeries->GetDataPointValue(nIndex, CBCGPChartData::CI_Y);

				nActual = (nIndex < m_nPeriod) ? nIndex : m_nPeriod;
				dblY = (dblPrevValue * (nActual - 1) + dblY) / nActual;
			}
			break;
			
		case MA_LINEAR_WEIGHTED:
			{
				dblY = CalcWeightedSum(nIndex, m_nPeriod, pInputSeries);
			}
			break;
		}
	}

	return dblY;
}
//*******************************************************************************
// CBCGPChartRSIFormula - RSI
//*******************************************************************************
void CBCGPChartRSIFormula::CopyFrom(const CBCGPChartBaseFormula& src)
{
	CBCGPChartAdvancedFormula::CopyFrom(src);
	m_rsiType = ((const CBCGPChartRSIFormula&)src).GetRSIType();
}
//*******************************************************************************
void CBCGPChartRSIFormula::SetRSIType(RSIType rsiType)
{
	if (m_rsiType != rsiType)
	{
		m_rsiType = rsiType;
		GeneratePoints();
	}
}
//*******************************************************************************
void CBCGPChartRSIFormula::CalcSimpleSums(int nIndex, CBCGPChartSeries* pInputSeries, 
											double& dblPositive, double& dblNegative)
{
	int nCounted = 0;
	dblPositive = 0.0;
	dblNegative = 0.0;
	
	for (int i = nIndex - m_nPeriod + 1; i <= nIndex; i++, nCounted++)
	{
		double dblDiff = pInputSeries->GetDataPointValue(i) - pInputSeries->GetDataPointValue(i - 1);
		
		if (dblDiff > 0)
		{
			dblPositive += dblDiff;
		}
		else if (dblDiff < 0)
		{
			dblNegative -= dblDiff;
		}
	}

	dblPositive /= nCounted;
	dblNegative /= nCounted;
}
//*******************************************************************************
CBCGPChartValue CBCGPChartRSIFormula::CalculateDataPoint(int nIndex, CBCGPChartSeries* pInputSeries, CBCGPChartValue& /*valY1*/)
{
	// use parent series to store previous values
	CBCGPDoubleArray* pUpSums = m_pParentSeries->GetDataBuffer(0);
	CBCGPDoubleArray* pDownSums = m_pParentSeries->GetDataBuffer(1);

	if (pUpSums->GetSize() == 0)
	{
		pUpSums->SetSize(m_pParentSeries->GetDataPointCount(), 1000);
		pDownSums->SetSize(m_pParentSeries->GetDataPointCount(), 1000);
	}

	if (nIndex < m_nPeriod + 1)
	{
		double dblSumPositive = 0.0;
		double dblSumNegative = 0.0;

		CalcSimpleSums(nIndex, pInputSeries, dblSumPositive, dblSumNegative);

		pUpSums->SetAtGrow(nIndex, dblSumPositive);
		pDownSums->SetAtGrow(nIndex, dblSumNegative);

		return CBCGPChartValue();
	}
	
	double dblDiffCurrent = pInputSeries->GetDataPointValue(nIndex) - pInputSeries->GetDataPointValue(nIndex - 1);
	double dblDiffPos = 0;
	double dblDiffNeg = 0;

	if (dblDiffCurrent > 0)
	{
		dblDiffPos = dblDiffCurrent;
	}
	else if (dblDiffCurrent < 0)
	{
		dblDiffNeg = fabs(dblDiffCurrent);
	}

	double dblUpPrev = pUpSums->GetAt(nIndex - 1);
	double dblDownPrev = pDownSums->GetAt(nIndex - 1);

	switch (m_rsiType)
	{
	case CBCGPChartRSIFormula::RSI_EXPONENTIAL:
		dblUpPrev = CalcEMA(dblDiffPos, dblUpPrev, m_dblEMAKoef);
		dblDownPrev = CalcEMA(dblDiffNeg, dblDownPrev, m_dblEMAKoef);
		break;

	case CBCGPChartRSIFormula::RSI_SMOOTHED:
		dblUpPrev = (dblUpPrev * (m_nPeriod - 1) + dblDiffPos) / m_nPeriod;
		dblDownPrev = (dblDownPrev * (m_nPeriod - 1) + dblDiffNeg) / m_nPeriod;
		break;

	case CBCGPChartRSIFormula::RSI_SIMPLE:
		CalcSimpleSums(nIndex, pInputSeries, dblUpPrev, dblDownPrev);
		break;
	}	

	pUpSums->SetAtGrow(nIndex, dblUpPrev);
	pDownSums->SetAtGrow(nIndex, dblDownPrev);

	if (dblDownPrev == 0)
	{
		return 100.0;
	}

	return 100.0 - 100.0/(1 + (dblUpPrev / dblDownPrev));
}
//*******************************************************************************
// CBCGPChartStohasticFormula - stochastic oscillator
//*******************************************************************************
void CBCGPChartStochasticFormula::CopyFrom(const CBCGPChartBaseFormula& src)
{
	const CBCGPChartStochasticFormula& stochastic = (const CBCGPChartStochasticFormula&)src;
	CBCGPChartAdvancedFormula::CopyFrom(src);
	m_nKPeriod = stochastic.GetKPeriod();
	m_nSlowing = stochastic.GetSlowing();
}
//*******************************************************************************
void CBCGPChartStochasticFormula::SetKPeriod(int nKPeriod, BOOL bGenerateDataPoints)
{
	if (nKPeriod < 1)
	{
		nKPeriod = 1;
	}

	m_nKPeriod = nKPeriod;

	if (bGenerateDataPoints)
	{
		GeneratePoints();
	}
}
//*******************************************************************************
void CBCGPChartStochasticFormula::SetSlowing(int nSlowing, BOOL bGenerateDataPoints)
{
	if (nSlowing < 1)
	{
		nSlowing = 1;
	}
	
	m_nSlowing = nSlowing;
	
	if (bGenerateDataPoints)
	{
		GeneratePoints();
	}
}
//*******************************************************************************
CBCGPChartValue CBCGPChartStochasticFormula::CalculateDataPoint(int nIndex, CBCGPChartSeries* pInputSeries, CBCGPChartValue& /*valY1*/)
{
	int nActualStart = m_nKPeriod + 1;

	if (nIndex < nActualStart)
	{
		return CBCGPChartValue();
	}

	CBCGPChartSeries* pSeriesLow = pInputSeries;
	CBCGPChartSeries* pSeriesHigh = pInputSeries;
	CBCGPChartSeries* pSeriesClose = pInputSeries;

	CBCGPChartStockSeries* pStockSeries = DYNAMIC_DOWNCAST(CBCGPChartStockSeries, pInputSeries);

	if (pStockSeries != NULL && pStockSeries->IsMainStockSeries())
	{
		pSeriesLow = pStockSeries->GetChildSeries(CBCGPChartStockSeries::CHART_STOCK_SERIES_LOW_IDX);
		pSeriesHigh = pStockSeries->GetChildSeries(CBCGPChartStockSeries::CHART_STOCK_SERIES_HIGH_IDX);
		pSeriesClose = pStockSeries->GetChildSeries(CBCGPChartStockSeries::CHART_STOCK_SERIES_CLOSE_IDX);
	}

	double dblMin = DBL_MAX;
	double dblMax = -DBL_MAX;

	CBCGPDoubleArray* pLowBuffer = m_pParentSeries->GetDataBuffer(0);
	CBCGPDoubleArray* pHighBuffer = m_pParentSeries->GetDataBuffer(1);

	for (int nStart = max(nActualStart, nIndex - m_nKPeriod + 1); nStart <= nIndex; nStart++)
	{
		double dblValLow = pSeriesLow->GetDataPointValue(nStart);
		double dblValHigh = pSeriesHigh->GetDataPointValue(nStart);

		dblMin = min(dblMin, dblValLow);
		dblMax = max(dblMax, dblValHigh);
	}

	pLowBuffer->SetAtGrow(nIndex, dblMin);
	pHighBuffer->SetAtGrow(nIndex, dblMax);
	
	double dblSumLow = 0;
	double dblSumHigh = 0;

	for (int i = max(nActualStart, nIndex - m_nKPeriod + 1); i <= nIndex; i++)
	{
		double dblCloseVal = pSeriesClose->GetDataPointValue(i);
		dblSumLow += dblCloseVal - pLowBuffer->GetAt(i);
		dblSumHigh += pHighBuffer->GetAt(i) - pLowBuffer->GetAt(i);
	}

	if (dblSumHigh == 0)
	{
		return 100;
	}

	return dblSumLow / dblSumHigh * 100.0;
}
//*******************************************************************************
// CBCGPChartBollingerBandsFormula - Bollinger bands
//*******************************************************************************
CBCGPChartBollingerBandsFormula::CBCGPChartBollingerBandsFormula(int nDeviation, int nPeriod, double dblBandOpacity) : 
	CBCGPChartAdvancedFormula(nPeriod)
{
	m_nDeviation = nDeviation;
	if (m_nDeviation < 1)
	{
		m_nDeviation = 1;
	}

	m_dblBandOpacity = dblBandOpacity;
}
//*******************************************************************************
CBCGPChartSeries* CBCGPChartBollingerBandsFormula::Create(CBCGPChartVisualObject* pChart, CString strFormulaName, 
		CBCGPChartSeries* pInputSeries, CBCGPChartAxis* pYAxis, CBCGPChartAxis* pXAxis)
{
	CBCGPChartSeries* pSeries = CBCGPChartAdvancedFormula::Create(pChart, strFormulaName, pInputSeries, pYAxis, pXAxis);
	pSeries->SetChartType(BCGPChartArea, BCGP_CT_RANGE);
	pSeries->SetBackroundOrder(TRUE);

	BCGPChartFormatSeries& format = (BCGPChartFormatSeries&)pSeries->GetSeriesFormat();
	format.SetSeriesFillOpacity(m_dblBandOpacity);

	return pSeries;
}
//*******************************************************************************
void CBCGPChartBollingerBandsFormula::CopyFrom(const CBCGPChartBaseFormula& src)
{
	const CBCGPChartBollingerBandsFormula& bollinger = (const CBCGPChartBollingerBandsFormula&)src;
	CBCGPChartAdvancedFormula::CopyFrom(src);
	m_nDeviation = bollinger.GetDeviation();
}
//*******************************************************************************
void CBCGPChartBollingerBandsFormula::SetDeviation(int nDeviation, BOOL bGenerateDataPoints)
{
	if (nDeviation < 1)
	{
		nDeviation = 1;
	}

	m_nDeviation = nDeviation;

	if (bGenerateDataPoints)
	{
		GeneratePoints();
	}
}
//*******************************************************************************
CBCGPChartValue CBCGPChartBollingerBandsFormula::CalculateDataPoint(int nIndex, CBCGPChartSeries* pInputSeries, CBCGPChartValue& valY1)
{
	double dblVal = pInputSeries->GetDataPointValue(nIndex);	

 	int nActual = 1;
	
	double dblMA = CalcPrevSum(nIndex - 1, m_nPeriod - 1, pInputSeries, nActual);
	dblMA += dblVal;
	dblMA /= (nActual + 1);

	double dblTotal = 0.0;

	for (int i = nIndex; i > max(0, nIndex - m_nPeriod); i--)
	{
		double dblClose = pInputSeries->GetDataPointValue(i);
		dblTotal += pow(dblClose - dblMA, 2.0);
	}

	double dblDev = m_nDeviation * sqrt(dblTotal/(nActual + 1));

	valY1.SetValue(dblDev * 2);

	return dblMA - dblDev;
}
//*******************************************************************************
// CBCGPChartMACDFormula
//*******************************************************************************
CBCGPChartMACDFormula::CBCGPChartMACDFormula(int nFastEMAPeriod, int nSlowEMAPeriod)
{
	SetPeriods(nFastEMAPeriod, nSlowEMAPeriod, FALSE);
}
//*******************************************************************************
void CBCGPChartMACDFormula::SetPeriods(int nFastEMAPeriod, int nSlowEMAPeriod, BOOL bGeneratePoints)
{
	m_nFastEMAPeriod = nFastEMAPeriod;
	m_nSlowEMAPeriod = nSlowEMAPeriod;

	m_dblFastEMAKoef = 2.0 / (double)(m_nFastEMAPeriod + 1.0);
	m_dblSlowEMAKoef = 2.0 / (double)(m_nSlowEMAPeriod + 1.0);

	if (bGeneratePoints)
	{
		GeneratePoints();
	}
}
//*******************************************************************************
void CBCGPChartMACDFormula::CopyFrom(const CBCGPChartBaseFormula& src)
{
	CBCGPChartAdvancedFormula::CopyFrom(src);
	const CBCGPChartMACDFormula& srcMADC = (const CBCGPChartMACDFormula&) src;
	SetPeriods(srcMADC.GetFastEMAPeriod(), srcMADC.GetSlowEMAPeriod(), FALSE);
}
//*******************************************************************************
CBCGPChartValue CBCGPChartMACDFormula::CalculateDataPoint(int nIndex, CBCGPChartSeries* pInputSeries, CBCGPChartValue& /*valY1*/)
{
	double dblVal = pInputSeries->GetDataPointValue(nIndex);
	CBCGPDoubleArray* pFastEMA = m_pParentSeries->GetDataBuffer(0);
	CBCGPDoubleArray* pSlowEMA = m_pParentSeries->GetDataBuffer(1);

	int nActual = 1;
	double dblPrevValueFast = 0;
	double dblPrevValueSlow = 0;

	if (nIndex < m_nPeriod)
	{
		dblPrevValueFast = CalcPrevSum(nIndex - 1, m_nFastEMAPeriod, pInputSeries, nActual);
		dblPrevValueFast /= nActual;

		dblPrevValueSlow = CalcPrevSum(nIndex - 1, m_nSlowEMAPeriod, pInputSeries, nActual);
		dblPrevValueSlow /= nActual;
	}
	else
	{
		dblPrevValueFast = pFastEMA->GetAt(nIndex - 1);
		dblPrevValueSlow = pSlowEMA->GetAt(nIndex - 1);
	}
	
	double dblFastEMA = CalcEMA(dblVal, dblPrevValueFast, m_dblFastEMAKoef);
	pFastEMA->SetAtGrow(nIndex, dblFastEMA);

	double dblSlowEMA = CalcEMA(dblVal, dblPrevValueSlow, m_dblSlowEMAKoef);
	pSlowEMA->SetAtGrow(nIndex, dblSlowEMA);

	return dblFastEMA - dblSlowEMA;
}
//*******************************************************************************
// CBCGPChartTransitionFormula
//*******************************************************************************
CBCGPChartTransitionFormula::CBCGPChartTransitionFormula()
{
	CommonInit();
}
//*******************************************************************************
CBCGPChartTransitionFormula::CBCGPChartTransitionFormula(TransitionType type, LPARAM lParam)
{
	CommonInit();
	
	m_type = type;
	m_lParam = lParam;
}
//*******************************************************************************
CBCGPChartTransitionFormula::CBCGPChartTransitionFormula(BCGPCHART_TRANSITION_CALLBACK pCallback, LPARAM lParam)
{
	CommonInit();

	m_pfnTransitionCallback = pCallback;
	m_lParam = lParam;
}
//*******************************************************************************
void CBCGPChartTransitionFormula::CommonInit()
{
	CBCGPChartBaseFormula::CommonInit();

	m_pfnTransitionCallback = NULL;
	m_type = CBCGPChartTransitionFormula::TT_UNDEFINED;
}
//*******************************************************************************
void CBCGPChartTransitionFormula::CopyFrom(const CBCGPChartBaseFormula& src)
{
	ASSERT_VALID(this);

	CBCGPChartTransitionFormula* pSrcTransition = DYNAMIC_DOWNCAST(CBCGPChartTransitionFormula, &src);

	if (pSrcTransition == NULL)
	{
		return;
	}

	ASSERT_VALID(pSrcTransition);

	CBCGPChartBaseFormula::CopyFrom(src);

	m_type = pSrcTransition->GetTransitionType();
	m_pfnTransitionCallback = pSrcTransition->GetTransitionCallback();
}
//*******************************************************************************
void CBCGPChartTransitionFormula::SetTransitionType(TransitionType type, LPARAM lParam)
{
	ASSERT_VALID(this);

	m_type = type;
	m_lParam = lParam;

	GeneratePoints();
}
//*******************************************************************************
void CBCGPChartTransitionFormula::AddInputSeries(CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);

	if (pSeries != NULL)
	{
		ASSERT_VALID(pSeries);

		m_arInputSeries.Add(pSeries);
	}
}
//*******************************************************************************
void CBCGPChartTransitionFormula::SetInputSeries(const CArray<CBCGPChartSeries*, CBCGPChartSeries*>& arSeries)
{
	m_arInputSeries.RemoveAll();
	m_arInputSeries.Append(arSeries);
}
//*******************************************************************************
void CBCGPChartTransitionFormula::RemoveAllInputSeries()
{
	m_arInputSeries.RemoveAll();
}
//*******************************************************************************
void CBCGPChartTransitionFormula::RemoveInputSeries(CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);

	int nIdx = FindInputSeriesIndex(pSeries);

	if (nIdx != -1)
	{
		m_arInputSeries.RemoveAt(nIdx, 1);
	}
}
//*******************************************************************************
void CBCGPChartTransitionFormula::SetTransitionCallback(BCGPCHART_TRANSITION_CALLBACK pCallback, LPARAM lParam )
{
	ASSERT_VALID(this);

	m_lParam = lParam;
	m_pfnTransitionCallback = pCallback;
	m_type = CBCGPChartTransitionFormula::TT_UNDEFINED;

	GeneratePoints();
}
//*******************************************************************************
void CBCGPChartTransitionFormula::GeneratePoints()
{
	ASSERT_VALID(this);

	if (m_pParentSeries == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pParentSeries);

	m_pParentSeries->RemoveAllDataPoints();

	int nMaxDP = 0;
	int nInputSeriesCount = GetInputSeriesCount();

	int i = 0;
	for (; i < nInputSeriesCount; i++)
	{
		CBCGPChartSeries* pSeries = GetInputSeriesAt(i);

		if (pSeries != NULL)
		{
			ASSERT_VALID(pSeries);
			nMaxDP = max(nMaxDP, pSeries->GetDataPointCount());
		}
	}

	for (i = 0; i < nMaxDP; i++)
	{
		CArray<CBCGPChartData, CBCGPChartData> arData;

		for (int j = 0; j < nInputSeriesCount; j++)
		{
			CBCGPChartSeries* pSeries = GetInputSeriesAt(j);

			if (pSeries != NULL)
			{
				CBCGPChartData data = pSeries->GetDataPointData(i);
				arData.Add(data);
			}
		}

		CBCGPChartDataPoint* pDP = CalculateDataPoint(arData, i);
		m_pParentSeries->AddDataPoint(pDP);
	}
	
	m_pParentSeries->RecalcMinMaxValues();

	return;
}
//*******************************************************************************
CBCGPChartDataPoint* CBCGPChartTransitionFormula::CalculateDataPoint(const CArray<CBCGPChartData, CBCGPChartData>& data, 
														  int nDataPointIndex)
{
	CBCGPChartDataPoint* pDP = new CBCGPChartDataPoint();
	CBCGPChartData dataNew;

	int i = 0;
	for (i = 0; i < data.GetSize(); i++)
	{
		CBCGPChartValue valX = data[i].GetValue(CBCGPChartData::CI_X);
		if (!valX.IsEmpty())
		{
			pDP->SetComponentValue(valX, CBCGPChartData::CI_X);
			dataNew.SetValue(valX, CBCGPChartData::CI_X);
		}
	}

	if (OnCalculateDataPoint(data, nDataPointIndex, dataNew))
	{
		pDP->SetData(dataNew);
		return pDP;
	}
	
	if (m_pfnTransitionCallback != NULL)
	{
		CBCGPChartData dataNew = (*m_pfnTransitionCallback)(data, nDataPointIndex, m_pParentSeries, m_lParam);
		pDP->SetData(dataNew);
		return pDP;
	}

	double dblY = 0;
	BOOL bPrevValueWasEmpty = FALSE;
	int nEmptyCount = 0;

	for (i = 0; i < data.GetSize(); i++)
	{
		CBCGPChartValue valY = data[i].GetValue(CBCGPChartData::CI_Y);

		CBCGPChartSeries::TreatNulls tn = CBCGPChartSeries::TN_VALUE;
		CBCGPChartSeries* pSeries = GetInputSeriesAt(i);		

		if (pSeries != NULL)
		{
			tn = pSeries->GetTreatNulls();
		}

		if (valY.IsEmpty() && tn != CBCGPChartSeries::TN_VALUE)
		{
			nEmptyCount++;

			if (i == 0)
			{
				switch(m_type)
				{
				case CBCGPChartTransitionFormula::TT_HIGH:
					dblY = -DBL_MAX;
					break;
					
				case CBCGPChartTransitionFormula::TT_LOW:
					dblY = DBL_MAX;
					break;
					
				case CBCGPChartTransitionFormula::TT_MULTIPLY:
					dblY = 1;
					break;

				case CBCGPChartTransitionFormula::TT_DIFF:
				case CBCGPChartTransitionFormula::TT_DIVIDE:
					bPrevValueWasEmpty = TRUE;
					break;
				}
			}
			continue;
		}
		

		if (i == 0)
		{
			dblY = valY.GetValue();
			continue;
		}		

		switch(m_type)
		{
		case CBCGPChartTransitionFormula::TT_SUM:
		case CBCGPChartTransitionFormula::TT_AVERAGE:
			dblY += valY.GetValue();
			break;
			
		case CBCGPChartTransitionFormula::TT_DIFF:
		case CBCGPChartTransitionFormula::TT_DIFF_ABS:
			if (bPrevValueWasEmpty)
			{
				dblY = valY.GetValue();
			}
			else
			{
				dblY -= valY.GetValue();
				if (m_type == CBCGPChartTransitionFormula::TT_DIFF_ABS)
				{
					dblY = fabs(dblY);
				}
			}
			break;

		case CBCGPChartTransitionFormula::TT_HIGH:
			dblY = max(dblY, valY.GetValue());
			break;

		case CBCGPChartTransitionFormula::TT_LOW:
			dblY = min(dblY, valY.GetValue());
			break;
			
		case CBCGPChartTransitionFormula::TT_MULTIPLY:
			dblY *= valY.GetValue();
			break;

		case CBCGPChartTransitionFormula::TT_DIVIDE:
			{
				double dblVal = valY.GetValue();
				if (bPrevValueWasEmpty)
				{
					dblY = dblVal;
				}
				else if (dblVal != 0)
				{
					dblY /= dblVal;
				}
			}
			break;
		}

		bPrevValueWasEmpty = FALSE;
	}

	if (dblY == DBL_MAX || dblY == -DBL_MAX)
	{
		dblY = 0;
	}

	if (m_type == CBCGPChartTransitionFormula::TT_AVERAGE)
	{
		dblY /= (data.GetSize() - nEmptyCount);
	}

	pDP->SetComponentValue(dblY);

	return pDP;
}

//*******************************************************************************
// CBCGPChartTrendFormula
//*******************************************************************************
CBCGPChartTrendFormula::CBCGPChartTrendFormula()
{
	CommonInit();
}
//*******************************************************************************
CBCGPChartTrendFormula::CBCGPChartTrendFormula(TrendLineType type, CBCGPChartSeries* pSeries, LPARAM lParam)
{
	CommonInit();

	m_type = type;
	m_arInputSeries.Add(pSeries);
	m_lParam = lParam;

	m_nPolynomialTrendOrder = 2;
}
//*******************************************************************************
void CBCGPChartTrendFormula::CommonInit()
{
	CBCGPChartBaseFormula::CommonInit();

	m_type = CBCGPChartTrendFormula::TLT_UNDEFINED;
	m_dblMinRange = m_dblMaxRange = 0;
}
//*******************************************************************************
void CBCGPChartTrendFormula::CopyFrom(const CBCGPChartBaseFormula& src)
{
	ASSERT_VALID(this);

	CBCGPChartTrendFormula* pSrcTrend = DYNAMIC_DOWNCAST(CBCGPChartTrendFormula, &src);

	if (pSrcTrend == NULL)
	{
		return;
	}

	ASSERT_VALID(pSrcTrend);

	CBCGPChartBaseFormula::CopyFrom(src);

	m_type = pSrcTrend->GetTrendType();

	pSrcTrend->GetTrendRange(m_dblMinRange, m_dblMaxRange);

	m_nPolynomialTrendOrder = pSrcTrend->GetPolynomialTrendOrder();

	SetCoefficients(pSrcTrend->GetCoefficients());
}
//*******************************************************************************
void CBCGPChartTrendFormula::SetInputSeries(CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);

	m_arInputSeries.RemoveAll();

	if (pSeries != NULL)
	{
		ASSERT_VALID(pSeries);

		m_arInputSeries.Add(pSeries)	;
	}
}
//*******************************************************************************
void CBCGPChartTrendFormula::SetTrendType(TrendLineType type, LPARAM lParam)
{
	ASSERT_VALID(this);

	m_type = type;
	m_lParam = lParam;

	GeneratePoints();

	if (m_pParentSeries != NULL && m_pParentSeries->GetChartCtrl() != 0)
	{
		m_pParentSeries->GetChartCtrl()->SetDirty(TRUE, TRUE);
	}
}
//*******************************************************************************
void CBCGPChartTrendFormula::SetPolynomialTrendOrder(int nOrder)
{
	m_nPolynomialTrendOrder = bcg_clamp(nOrder, 2, 6);

	if (m_pParentSeries != NULL && m_pParentSeries->GetChartCtrl() != NULL)
	{
		m_pParentSeries->GetChartCtrl()->SetDirty(TRUE, FALSE);
	}
}
//*******************************************************************************
void CBCGPChartTrendFormula::SetCoefficients(const CArray<double, double>& arCoef)
{
	ASSERT_VALID(this);

	m_arCoef.RemoveAll();
	m_arCoef.Append(arCoef);
}
//*******************************************************************************
void CBCGPChartTrendFormula::AddCoefficient(double dblCoef)
{
	ASSERT_VALID(this);

	m_arCoef.Add(dblCoef);
}
//****************************************************************************************
CString CBCGPChartTrendFormula::GetFormulaText() const
{
	ASSERT_VALID(this);

	CString strText;

	double dblCoef0 = m_arCoef.GetSize() > 0 ? m_arCoef[0] : 0;
	double dblCoef1 = m_arCoef.GetSize() > 1 ? m_arCoef[1] : 0;

	switch(m_type)
	{
	case CBCGPChartTrendFormula::TLT_LINEAR:
		// y = ax + b
		if (dblCoef1 > 0)
		{
			if (dblCoef0 != 1 && dblCoef0 != 0)
			{
				strText.Format(_T("y = %G x + %G"), dblCoef0, dblCoef1);
			}
			else if (dblCoef0 == 1)
			{
				strText.Format(_T("y = x + %G"), dblCoef1);
			}
			else
			{
				strText.Format(_T("y = %G"), dblCoef1);
			}
			
		}
		else if (dblCoef1 < 0)
		{
			if (dblCoef0 != 1 && dblCoef0 != 0)
			{
				strText.Format(_T("y = %G x - %G"), dblCoef0, fabs(dblCoef1));
			}
			else if (dblCoef0 == 1)
			{
				strText.Format(_T("y = x - %G"), fabs(dblCoef1));
			}
			else
			{
				strText.Format(_T("y = %G"), dblCoef1);
			}
		}
		else
		{
			if (dblCoef0 != 0)
			{
				strText.Format(_T("y = %G x"), dblCoef0);
			}
			else
			{
				strText = _T("y = 0");
			}
		}
		
		break;

	case CBCGPChartTrendFormula::TLT_POWER:
		// y = a x^b)
		if (dblCoef0 != 1)
		{
			if (dblCoef1 != 1 && dblCoef1 != 0)
			{
				strText.Format(_T("y = %G x^%G"), dblCoef0, dblCoef1);
			}
			else if (dblCoef1 == 1)
			{
				strText.Format(_T("y = %G x"), dblCoef0);
			}
			else if (dblCoef1 == 0)
			{
				strText.Format(_T("y = %G"), dblCoef0);
			}
		}
		else
		{
			if (dblCoef1 != 1 && dblCoef1 != 0)
			{
				strText.Format(_T("y = x^%G"), dblCoef1);
			}
			else if (dblCoef1 == 1)
			{
				strText = _T("y = x");
			}
			else 
			{
				strText = _T("y = 1");
			}
		}
		break;

	case CBCGPChartTrendFormula::TLT_EXPONENTIAL:
		// y = a e^(bx)
		if (dblCoef1 != 0)
		{
			if (dblCoef0 != 1)
			{
				strText.Format(_T("y = %G e^%Gx"), dblCoef0, dblCoef1);
			}
			else
			{
				strText.Format(_T("y = e^%Gx"), dblCoef1);
			}
			
		}
		else
		{
			strText.Format(_T("y = %G"), dblCoef0);
		}
		break;

	case CBCGPChartTrendFormula::TLT_LOGARITHMIC:
		// y = a log (x) + b
		if (dblCoef1 > 0)
		{
			if (dblCoef0 != 0 && dblCoef0 != 1)
			{
				strText.Format(_T("y = %G ln(x) + %G"), dblCoef0, dblCoef1);
			}
			else if (dblCoef0 == 1)
			{
				strText.Format(_T("y = ln(x) + %G"), dblCoef1);
			}
			else
			{
				strText.Format(_T("y = %G"), dblCoef1);
			}
			
		}
		else if (dblCoef1 < 0)
		{
			if (dblCoef0 != 0 && dblCoef0 != 1)
			{
				strText.Format(_T("y = %G ln(x) - %G"), dblCoef0, fabs(dblCoef1));
			}
			else if (dblCoef0 == 1)
			{
				strText.Format(_T("y = ln(x) - %G"), fabs(dblCoef1));
			}
			else
			{
				strText.Format(_T("y = -%G"), fabs(dblCoef1));
			}
		}
		else
		{
			if (dblCoef0 != 1)
			{
				strText.Format(_T("y = %G ln(x)"), dblCoef0);
			}
			else
			{
				strText = _T("y = ln(x)");
			}
		}

		break;

	case CBCGPChartTrendFormula::TLT_POLYNOMIAL:
		// y = k(0) x^n + k(1) x^(n-1) + ... + k(n-1) x + k(n)
		{
			strText = _T("y = ");

			int nPow = (int)m_arCoef.GetSize() - 1;
			
			for (int i = 0; i < m_arCoef.GetSize(); i++)
			{
				CString strPart;
				double dblCoef = m_arCoef[i];

				if (i < nPow - 1)
				{
					if (dblCoef > 0)
					{
						if (i > 0)
						{
							strPart.Format(_T(" + %.5G x^%d"), dblCoef, nPow - i);
						}
						else
						{
							strPart.Format(_T("%.5G x^%d"), dblCoef, nPow - i);
						}
					}
					else if (dblCoef < 0)
					{
						strPart.Format(_T(" - %.5G x^%d"), fabs(dblCoef), nPow - i);
					}
				}
				else if (i == nPow - 1)
				{
					if (dblCoef > 0)
					{
						if (i > 0)
						{
							strPart.Format(_T(" + %.5G x"), dblCoef);
						}
						else
						{
							strPart.Format(_T("%.5G x"), dblCoef);
						}
					}
					else if (dblCoef < 0)
					{
						strPart.Format(_T(" - %.5G x"), fabs(dblCoef));
					}
				}
				else
				{
					if (dblCoef > 0)
					{
						if (i > 0)
						{
							strPart.Format(_T(" + %.5G"), dblCoef);
						}
						else
						{
							strPart.Format(_T("%.5G"), dblCoef);
						}
					}
					else if (dblCoef < 0)
					{
						strPart.Format(_T(" - %.5G"), fabs(dblCoef));
					}
				}

				strText += strPart;
			}
		}
		break;
	}

	return strText;
}
//****************************************************************************************
void CBCGPChartTrendFormula::SetTrendRange(double dblMinVal, double dblMaxVal)
{
	if (dblMinVal <= dblMaxVal)
	{
		m_dblMinRange = dblMinVal;
		m_dblMaxRange = dblMaxVal;
	}
	else
	{
		m_dblMinRange = dblMaxVal;
		m_dblMaxRange = dblMinVal;
	}
}
//****************************************************************************************
void CBCGPChartTrendFormula::GetTrendRange(double& dblMinVal, double& dblMaxVal) const
{
	dblMinVal = m_dblMinRange;
	dblMaxVal = m_dblMaxRange;
}
//****************************************************************************************
void CBCGPChartTrendFormula::CalculateTrendCoefficients()
{
	ASSERT_VALID(this);

	switch(m_type)
	{
	case CBCGPChartTrendFormula::TLT_LINEAR:
		CalculateLinearTrend();
		break;

	case CBCGPChartTrendFormula::TLT_EXPONENTIAL:
		CalculateExponentialTrend();
		break;

	case CBCGPChartTrendFormula::TLT_LOGARITHMIC:
		CalculateLogarithmicTrend();
		break;

	case CBCGPChartTrendFormula::TLT_POLYNOMIAL:
		CalculatePolynomialTrend();
		break;

	case CBCGPChartTrendFormula::TLT_POWER:
		CalculatePowerTrend();
		break;
	}
}
//****************************************************************************************
void CBCGPChartTrendFormula::GeneratePoints()
{
	ASSERT_VALID(this);

	m_arScreenPoints.RemoveAll();

	if (m_pParentSeries == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pParentSeries);

	m_pParentSeries->ClearMinMaxValues();

	CBCGPChartAxis* pXAxis = m_pParentSeries->GetRelatedAxis(CBCGPChartSeries::AI_X);
	CBCGPChartAxis* pYAxis = m_pParentSeries->GetRelatedAxis(CBCGPChartSeries::AI_Y);

	if (pXAxis == NULL || pYAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(pXAxis);
	ASSERT_VALID(pYAxis);

	CalculateTrendCoefficients();
	
	BOOL bUseRange = (m_dblMinRange != m_dblMaxRange);

	double dblMinDispValue = pXAxis->GetMinDisplayedValue();
	double dblMaxDispValue = pXAxis->GetMaxDisplayedValue();

	if (pXAxis->IsDisplayDataBetweenTickMarks())
	{
		dblMinDispValue -= pXAxis->GetMajorUnit();
	}

	double dblAxisSize = pXAxis->GetAxisSize();
	double dblRange = dblMaxDispValue - dblMinDispValue;

	BOOL bPolar = FALSE;
	if (pXAxis->IsKindOf(RUNTIME_CLASS(CBCGPChartAxisPolarX)) && !pXAxis->IsComponentXSet())
	{
		dblRange = 360.;
		bPolar = TRUE;
	}

	CBCGPChartSeries* pSeries = GetInputSeriesAt(0);

	double dblMinValue = bUseRange ? m_dblMinRange : dblMinDispValue;
	double dblMaxValue = bUseRange ? m_dblMaxRange : dblMaxDispValue;

	if (pSeries != NULL && !bUseRange && !pSeries->IsXComponentSet())
	{
		dblMinValue = 1;
		dblMaxValue = pSeries->GetDataPointCount();
	}

	double dblMinMaxDiff = (dblMaxValue - dblMinValue);

	if (bUseRange && !bPolar)
	{
		dblRange = dblMinMaxDiff;
	}

	double dblPixelStep = dblAxisSize == 0 ? dblMinMaxDiff / 10. : dblRange / dblAxisSize;

	if (bUseRange)
	{
		m_pParentSeries->SetMinMaxValues(dblMinValue, CBCGPChartData::CI_X, 0);
		m_pParentSeries->SetMinMaxValues(dblMaxValue, CBCGPChartData::CI_X, 0);
	}

	if (m_type == CBCGPChartTrendFormula::TLT_LINEAR && !pYAxis->IsLogScale())
	{
		CBCGPChartValue YVal = CalculateYValue(dblMinValue);
		CBCGPChartData dataMin(dblMinValue, YVal);

		m_pParentSeries->SetMinMaxValues(YVal, CBCGPChartData::CI_Y, 0);

		CBCGPPoint pt = pSeries != NULL ? pSeries->ScreenPointFromChartData(dataMin, 0) : m_pParentSeries->ScreenPointFromChartData(dataMin, 0);

		m_arScreenPoints.Add(pt);

		YVal = CalculateYValue(dblMaxValue);

		m_pParentSeries->SetMinMaxValues(YVal, CBCGPChartData::CI_Y, 0);

		CBCGPChartData dataMax(dblMaxValue, YVal);
		pt = pSeries != NULL ? pSeries->ScreenPointFromChartData(dataMax, 0) : m_pParentSeries->ScreenPointFromChartData(dataMax, 0);

		m_arScreenPoints.Add(pt);
	}
	else
	{
		if (dblPixelStep == 0)
		{
			return;
		}

		double dblStep = fabs(dblPixelStep);

		for (double dblXVal = dblMinValue; dblXVal < dblMaxValue + dblStep; dblXVal += dblStep)
		{
			if (dblXVal > dblMaxValue)
			{
				dblXVal = dblMaxValue;
			}

			CBCGPChartValue YVal = CalculateYValue(dblXVal);

			m_pParentSeries->SetMinMaxValues(YVal, CBCGPChartData::CI_Y, 0);

			if (!YVal.IsEmpty())
			{
				CBCGPChartData data(dblXVal, YVal);

				CBCGPPoint pt = pSeries != NULL ? pSeries->ScreenPointFromChartData(data, 0) : 
													m_pParentSeries->ScreenPointFromChartData(data, 0);
			
				m_arScreenPoints.Add(pt);
			}

			if (dblXVal == dblMaxValue)
			{
				break;
			}
		}
	}
}
//****************************************************************************************
CBCGPChartValue CBCGPChartTrendFormula::CalculateYValue(double dblXVal)
{
	CBCGPChartValue dblYVal;

	if (OnCalculateYValue(dblXVal, dblYVal))
	{
		return dblYVal;
	}
	if (m_arCoef.GetSize() == 0)
	{
		return CBCGPChartValue();
	}

	double dblCoef0 = m_arCoef.GetSize() >= 1 ? m_arCoef[0] : 0;
	double dblCoef1 = m_arCoef.GetSize() >= 2 ? m_arCoef[1] : 0;

	switch(m_type)
	{
	case CBCGPChartTrendFormula::TLT_LINEAR:
		// y = a * x + b
		return dblCoef0 * dblXVal + dblCoef1;

	case CBCGPChartTrendFormula::TLT_EXPONENTIAL:
		// y = a * exp(b*x)
		{
			double dblExp = exp(dblCoef1*dblXVal);

			if (_isnan(dblExp))
			{
				return CBCGPChartValue();
			}

			return dblCoef0 * dblExp;
		}
		break;
	
	case CBCGPChartTrendFormula::TLT_LOGARITHMIC:
		if (dblXVal != 0)
		{
			// y = a * log(x) + b
			double dblLog = log(dblXVal);

			if (_isnan(dblLog))
			{
				return CBCGPChartValue();
			}

			return dblCoef0 * dblLog + dblCoef1;
		}
		break;	

	case CBCGPChartTrendFormula::TLT_POLYNOMIAL:
		{
			int nPow = (int)m_arCoef.GetSize() - 1;
			double dblYSum = 0;
			for (int i = 0; i < (int)m_arCoef.GetSize(); i++)
			{
				double dblPow = pow(dblXVal, nPow - i);

				if (_isnan (dblPow))
				{
					return CBCGPChartValue();
				}

				dblYSum += m_arCoef[i] * dblPow;
			}
			return dblYSum;
		}
		break;

	case CBCGPChartTrendFormula::TLT_POWER:
		// y = a * pow (x, b)
		{
			double dblPow = pow (dblXVal, dblCoef1);

			if (_isnan (dblPow))
			{
				return CBCGPChartValue();
			}

			return CBCGPChartValue(dblCoef0 * dblPow);
		}

		break;
		
	}

	return CBCGPChartValue();
}
//****************************************************************************************
void CBCGPChartTrendFormula::CalculateLinearTrend()
{
	// y = ax + b
	// a = (n * sum(Xi*Yi) - sum(Xi) * sum(Yi)) / (n * sum(pow(Xi, 2)) - pow(sum(Xi), 2)
	// b = 1/n * (sum(Yi) - a * sum(Xi))

	ASSERT_VALID(this);

	if (m_arInputSeries.GetSize() == 0)
	{
		return;
	}

	CBCGPChartSeries* pSeries = GetInputSeriesAt(0);

	if (pSeries == NULL)
	{
		return;
	}

	m_arCoef.RemoveAll();

	ASSERT_VALID(pSeries);

	int nDPCount = pSeries->GetDataPointCount();

	double dblSumX = 0;
	double dblSumY = 0;
	double dblSumXY = 0;
	double dblSumXSquare = 0;

	BOOL bXCompSet = pSeries->IsXComponentSet();

	for (int i = 0; i < nDPCount; i++)
	{
		double dblX = bXCompSet ? pSeries->GetDataPointValue(i, CBCGPChartData::CI_X).GetValue() : i + 1;
		double dblY = pSeries->GetDataPointValue(i, CBCGPChartData::CI_Y).GetValue();

		dblSumX += dblX;
		dblSumY += dblY;
		dblSumXY += dblX * dblY;
		dblSumXSquare += dblX * dblX;
	}

	double dblA = (nDPCount * dblSumXY - dblSumX * dblSumY) / (nDPCount * dblSumXSquare - dblSumX * dblSumX);
	double dblB = 1./nDPCount * (dblSumY - dblA * dblSumX);
		
	m_arCoef.Add(dblA);
	m_arCoef.Add(dblB);
}
//****************************************************************************************
void CBCGPChartTrendFormula::CalculatePowerTrend()
{
	// y = a * pow(x, b)
	// b = (n * sum(ln(Xi)*ln(Yi) - sum(ln(Xi)) * sum(ln(Yi))) / (n * sum(pow(ln(Xi), 2)) - pow(sum(ln(Xi)), 2))
	// a = exp(1/n * (sum(ln(Yi)) - b*sum(ln(Xi))))

	if (m_arInputSeries.GetSize() == 0)
	{
		return;
	}

	CBCGPChartSeries* pSeries = GetInputSeriesAt(0);

	if (pSeries == NULL)
	{
		return;
	}

	ASSERT_VALID(pSeries);

	m_arCoef.RemoveAll();

	int nDPCount = pSeries->GetDataPointCount();

	double dblSumLogX = 0;
	double dblSumLogY = 0;
	double dblSumLogXY = 0;
	double dblSumLogXSquare = 0;

	BOOL bXCompSet = pSeries->IsXComponentSet();

	int nDPTotal = nDPCount;
	for (int i = 0; i < nDPCount; i++)
	{
		double dblX = bXCompSet ? pSeries->GetDataPointValue(i, CBCGPChartData::CI_X).GetValue() : i + 1;
		double dblY = pSeries->GetDataPointValue(i, CBCGPChartData::CI_Y).GetValue();

		if (dblX <= 0. || dblY <= 0.)
		{
			nDPTotal --;
			continue;
		}
			
		double dblLogX = log(dblX);
		double dblLogY = log(dblY);

		dblSumLogX += dblLogX;
		dblSumLogY += dblLogY;

		dblSumLogXY += dblLogX * dblLogY;
		dblSumLogXSquare += dblLogX * dblLogX;
	}

	double dblB = (nDPTotal * dblSumLogXY - dblSumLogX * dblSumLogY) / (nDPTotal * dblSumLogXSquare - dblSumLogX * dblSumLogX);
	double dblA = exp(1./nDPTotal*(dblSumLogY - dblB*dblSumLogX));

	m_arCoef.Add(dblA);
	m_arCoef.Add(dblB);
}
//****************************************************************************************
void CBCGPChartTrendFormula::CalculateLogarithmicTrend()
{
	// y = a * log (x) + b
	// a = (n * sum(Yi * log(Xi)) - sum(log(Xi)) * sum(Yi)) / ((n * sum(pow(log(Xi), 2))) - pow(sum(log(Xi)), 2))
	// b = 1/n * (sum(Yi) - a * sum(log(x)))

	if (m_arInputSeries.GetSize() == 0)
	{
		return;
	}

	CBCGPChartSeries* pSeries = GetInputSeriesAt(0);

	if (pSeries == NULL)
	{
		return;
	}

	ASSERT_VALID(pSeries);

	m_arCoef.RemoveAll();

	int nDPCount = pSeries->GetDataPointCount();

	double dblLogX = 0;
	double dblSumLogX = 0;
	double dblSumY = 0;
	double dblSumYLogX = 0;
	double dblSumLogXSquare = 0;

	BOOL bXCompSet = pSeries->IsXComponentSet();

	int nDPTotal = nDPCount;
	for (int i = 0; i < nDPCount; i++)
	{
		double dblX = bXCompSet ? pSeries->GetDataPointValue(i, CBCGPChartData::CI_X).GetValue() : i + 1;
		double dblY = pSeries->GetDataPointValue(i, CBCGPChartData::CI_Y).GetValue();

		if (dblX <= 0.)
		{
			nDPTotal --;
			continue;
		}

		dblLogX = log(dblX);
		dblSumLogX += dblLogX;
		dblSumYLogX += dblY * dblLogX;
		dblSumY += dblY;
		dblSumLogXSquare += dblLogX * dblLogX;
	}

	double dblA = (nDPTotal * dblSumYLogX - dblSumLogX * dblSumY) / (nDPTotal * dblSumLogXSquare - dblSumLogX * dblSumLogX);
	double dblB = 1./nDPTotal * (dblSumY - dblA * dblSumLogX);

	m_arCoef.Add(dblA);
	m_arCoef.Add(dblB);
}
//****************************************************************************************
void CBCGPChartTrendFormula::CalculateExponentialTrend()
{
	// y = a * exp(b*x);
	// b = (n * sum(Xi * log(Yi)) - sum(Xi) * sum(log(Yi))) / (n * sum(pow(Xi, 2)) - pow(sum(Xi), 2))
	// a = exp(1./n * (sum(log(Yi)) - b * sum(Xi)))

	if (m_arInputSeries.GetSize() == 0)
	{
		return;
	}

	CBCGPChartSeries* pSeries = GetInputSeriesAt(0);

	if (pSeries == NULL)
	{
		return;
	}

	ASSERT_VALID(pSeries);

	m_arCoef.RemoveAll();

	int nDPCount = pSeries->GetDataPointCount();

	double dblLogY = 0;
	double dblSumX = 0;
	double dblSumLogY = 0;
	double dblSumXLogY = 0;
	double dblSumXSquare = 0;

	BOOL bXCompSet = pSeries->IsXComponentSet();

	int nDPTotal = nDPCount;
	for (int i = 0; i < nDPCount; i++)
	{
		double dblX = bXCompSet ? pSeries->GetDataPointValue(i, CBCGPChartData::CI_X).GetValue() : i + 1;
		double dblY = pSeries->GetDataPointValue(i, CBCGPChartData::CI_Y).GetValue();

		if (dblY <= 0.)
		{
			nDPTotal --;
			continue;
		}

		dblLogY = log(dblY);

		dblSumX += dblX;
		dblSumLogY += dblLogY;
		dblSumXLogY += dblX * dblLogY;
		dblSumXSquare += dblX * dblX;
	}

	double dblB = (nDPTotal * dblSumXLogY - dblSumX * dblSumLogY) / (nDPTotal * dblSumXSquare - dblSumX * dblSumX);
	double dblA = exp(1./nDPTotal * (dblSumLogY - dblB * dblSumX));

	m_arCoef.Add(dblA);
	m_arCoef.Add(dblB);
}
//****************************************************************************************
void CBCGPChartTrendFormula::CalculatePolynomialTrend()
{
	if (m_arInputSeries.GetSize() == 0)
	{
		return;
	}

	CBCGPChartSeries* pSeries = GetInputSeriesAt(0);

	if (pSeries == NULL)
	{
		return;
	}

	ASSERT_VALID(pSeries);

	m_arCoef.RemoveAll();

	int nDPCount = pSeries->GetDataPointCount();
	if (nDPCount == 0)
	{
		return;
	}

	BOOL bXCompSet = pSeries->IsXComponentSet();

	CArray<double, double> arX;
	CArray<double, double> arY;

	arX.SetSize (nDPCount);
	arY.SetSize (nDPCount);

	for (int i = 0; i < nDPCount; i++)
	{
		arX[i] = bXCompSet ? pSeries->GetDataPointValue(i, CBCGPChartData::CI_X).GetValue() : i + 1;
		arY[i] = pSeries->GetDataPointValue(i, CBCGPChartData::CI_Y).GetValue();
	}

	PolynomialRegression(arX.GetData(), arY.GetData(), nDPCount, m_nPolynomialTrendOrder, m_arCoef);
}

//*******************************************************************************
// CBCGPChartVirtualFormula
//*******************************************************************************
CBCGPChartVirtualFormula::CBCGPChartVirtualFormula()
{
	CommonInit();
}
//*******************************************************************************
CBCGPChartVirtualFormula::CBCGPChartVirtualFormula(BCGPCHART_VSERIES_CALLBACK pCallback, LPARAM lParam)
{
	CommonInit();

	m_pfnVirtualCallback = pCallback;
	m_lParam = lParam;
}
//*******************************************************************************
void CBCGPChartVirtualFormula::CommonInit()
{
	CBCGPChartTrendFormula::CommonInit();

	m_pfnVirtualCallback = NULL;
}
//*******************************************************************************
void CBCGPChartVirtualFormula::CopyFrom(const CBCGPChartBaseFormula& src)
{
	ASSERT_VALID(this);

	CBCGPChartVirtualFormula* pSrc = DYNAMIC_DOWNCAST(CBCGPChartVirtualFormula, &src);

	if (pSrc == NULL)
	{
		return;
	}

	ASSERT_VALID(pSrc);

	CBCGPChartTrendFormula::CopyFrom(src);

	m_pfnVirtualCallback = pSrc->GetVirtualCallback();
}
//*******************************************************************************
void CBCGPChartVirtualFormula::SetVirtualCallback(BCGPCHART_VSERIES_CALLBACK pCallback, LPARAM lParam)
{
	ASSERT_VALID(this);

	m_pfnVirtualCallback = pCallback;
	m_lParam = lParam;

	GeneratePoints();
}
//*******************************************************************************
CBCGPChartValue CBCGPChartVirtualFormula::CalculateYValue(double dblXVal)
{
	ASSERT_VALID(this);

	if (m_pParentSeries == NULL)
	{
		return CBCGPChartValue();
	}

	CBCGPChartValue YValue;

	if (m_pParentSeries->OnCalculateVirtualYValue(dblXVal, YValue, m_lParam))
	{
		return YValue;
	}

	if (m_pfnVirtualCallback != NULL)
	{
		return (*m_pfnVirtualCallback)(dblXVal, m_pParentSeries, m_lParam);
	}

	return CBCGPChartValue();
}
