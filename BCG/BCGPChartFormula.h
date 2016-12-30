//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a sample for BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//

#if !defined(AFX_BCGPCHARTFORMULA_H__FE0BBF37_BD61_474E_B4BE_D6D7AFEC94AC__INCLUDED_)
#define AFX_BCGPCHARTFORMULA_H__FE0BBF37_BD61_474E_B4BE_D6D7AFEC94AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

class CBCGPChartSeries;
class CBCGPChartVisualObject;


// params: arData - array of chart data extracted from all data points located at the specified index, 
// nDataPointIndex - data point index, 
// pFormulaSeries - pointer to formula series
// return value: new (translated) data
typedef CBCGPChartData (CALLBACK* BCGPCHART_TRANSITION_CALLBACK)(const CArray<CBCGPChartData, CBCGPChartData>& arData, 
					int nDataPointIndex, CBCGPChartSeries* pFormulaSeries, LPARAM lp);


//****************************************************************************************
// CBCGPChartBaseFormula - base formula declaration
//****************************************************************************************

class BCGCBPRODLLEXPORT CBCGPChartBaseFormula : public CObject
{
	DECLARE_DYNAMIC(CBCGPChartBaseFormula)

protected:
	CBCGPChartBaseFormula();
	void CommonInit();

public:
	virtual void GeneratePoints() = 0;
	virtual BOOL IsNonDiscreteCurve() const {return FALSE;}

	void SetParentSeries(CBCGPChartSeries* pSeries);
	CBCGPChartSeries* GetParentSeries() const {return m_pParentSeries;}

	LPARAM GetLParam() const {return m_lParam;}
	void SetLParam(LPARAM lp = NULL) {m_lParam = lp;}

	virtual int GetInputSeriesCount() const {return (int)m_arInputSeries.GetSize();}
	virtual CBCGPChartSeries* GetInputSeriesAt(int nIdx) const;
	virtual int FindInputSeriesIndex(CBCGPChartSeries* pSeries) const;

	virtual void CopyFrom(const CBCGPChartBaseFormula& src);

	CBCGPChartBaseFormula& operator=(const CBCGPChartBaseFormula& src);
	
	static double CalcEMA(double dblCurrValue, double dblPrevValue, double dblEmaKoef);
	static double CalcEMAOnArray(double* pInput, int nOrder, double dblEMAKoef = -1.0);
	static double CalcSMAOnArray(double* pInput, int nPeriod);

protected:
	CBCGPChartSeries*								m_pParentSeries;
	CArray<CBCGPChartSeries*, CBCGPChartSeries*>	m_arInputSeries;

	LPARAM											m_lParam;
};
//****************************************************************************************
class BCGCBPRODLLEXPORT CBCGPChartAdvancedFormula : public CBCGPChartBaseFormula
{
	DECLARE_DYNAMIC(CBCGPChartAdvancedFormula)
protected:
	CBCGPChartAdvancedFormula(int nPeriod = 14)
	{
		m_nPeriod = 0;
		m_dblEMAKoef = 0;
		SetPeriod(nPeriod, FALSE);
		m_bUseLongData = TRUE;
	}
public:
	enum MovingAverageType
	{
		MA_SIMPLE,
		MA_EXPONENTIAL,
		MA_SMOOTHED,
		MA_LINEAR_WEIGHTED
	};
public:
	BOOL m_bUseLongData;

public:
	virtual CBCGPChartSeries* Create(CBCGPChartVisualObject* pChart, CString strFormulaName, 
										CBCGPChartSeries* pInputSeries,
										CBCGPChartAxis* pYAxis = NULL, CBCGPChartAxis* pXAxis = NULL);
	virtual void GeneratePoints();
	virtual void UpdateLastDataPoints(int nCount = -1);

	void SetPeriod(int nPeriod, BOOL bGeneratePoints = TRUE);
	int GetPeriod() const {return m_nPeriod;}

	virtual void CopyFrom(const CBCGPChartBaseFormula& src);
	virtual CBCGPChartValue CalculateDataPoint(int nIndex, CBCGPChartSeries* pInputSeries, 
		CBCGPChartValue& valY1) = 0;

	virtual BCGPChartCategory GetOutputSeriesCategory() const {return BCGPChartLine;}
	virtual BCGPChartType GetOutputSeriesType() const {return BCGP_CT_SIMPLE;}

protected:
	double CalcPrevSum(int nIndexStart, int nOrder, CBCGPChartSeries* pInputSeries, int& nActual);
	double CalcWeightedSum(int nIndexStart, int nOrder, CBCGPChartSeries* pInputSeries);

protected:
	int			m_nPeriod;
	double		m_dblEMAKoef;
};
//****************************************************************************************
class BCGCBPRODLLEXPORT CBCGPChartMAFormula : public CBCGPChartAdvancedFormula
{
	DECLARE_DYNCREATE(CBCGPChartMAFormula)

public:
	CBCGPChartMAFormula(MovingAverageType maType = CBCGPChartAdvancedFormula::MA_SIMPLE, int nPeriod = 14);
	
	virtual CBCGPChartValue CalculateDataPoint(int nIndex, CBCGPChartSeries* pInputSeries, CBCGPChartValue& valY1);
	virtual void CopyFrom(const CBCGPChartBaseFormula& src);

	void SetMAType(MovingAverageType maType);
	MovingAverageType GetMAType() const {return m_maType;}

protected:
	MovingAverageType	m_maType;
	
};
//****************************************************************************************
class BCGCBPRODLLEXPORT CBCGPChartRSIFormula : public CBCGPChartAdvancedFormula
{
	DECLARE_DYNCREATE(CBCGPChartRSIFormula)
public:
	enum RSIType
	{
		RSI_SIMPLE,
		RSI_EXPONENTIAL,
		RSI_SMOOTHED,
	};

	CBCGPChartRSIFormula(RSIType rsiType = RSI_SIMPLE, int nPeriod = 14) : CBCGPChartAdvancedFormula(nPeriod)
	{
		m_rsiType = rsiType;
	}

	virtual void CopyFrom(const CBCGPChartBaseFormula& src);

	void SetRSIType(RSIType rsiType);
	RSIType GetRSIType() const {return m_rsiType;}

	virtual CBCGPChartValue CalculateDataPoint(int nIndex, CBCGPChartSeries* pInputSeries, CBCGPChartValue& valY1);

protected:
	void CalcSimpleSums(int nIndex, CBCGPChartSeries* pInputSeries, double& dblPositive, double& dblNegative);
protected:
	RSIType	m_rsiType;

};
//****************************************************************************************
class BCGCBPRODLLEXPORT CBCGPChartStochasticFormula : public CBCGPChartAdvancedFormula
{
	DECLARE_DYNCREATE(CBCGPChartStochasticFormula)

public:
	CBCGPChartStochasticFormula(int nKPeriod = 5, int nSlowing = 3)
	{
		m_nKPeriod = nKPeriod;
		m_nSlowing = nSlowing;
	}

	int GetKPeriod() const {return m_nKPeriod;}
	int GetSlowing() const {return m_nSlowing;}

	void SetKPeriod(int nKPeriod, BOOL bGenerateDataPoints = FALSE);
	void SetSlowing(int nSlowing, BOOL bGenerateDataPoints = FALSE);
	
	virtual void CopyFrom(const CBCGPChartBaseFormula& src);

	virtual CBCGPChartValue CalculateDataPoint(int nIndex, CBCGPChartSeries* pInputSeries, CBCGPChartValue& valY1);

protected:
	int m_nKPeriod;
	int m_nSlowing;
};
//****************************************************************************************
class BCGCBPRODLLEXPORT CBCGPChartBollingerBandsFormula : public CBCGPChartAdvancedFormula
{
	DECLARE_DYNCREATE(CBCGPChartBollingerBandsFormula);

public:
	CBCGPChartBollingerBandsFormula(int nDeviation = 2, int nPeriod = 20, double dblBandOpacity = 0.1);

	virtual CBCGPChartSeries* Create(CBCGPChartVisualObject* pChart, CString strFormulaName, 
		CBCGPChartSeries* pInputSeries, CBCGPChartAxis* pYAxis = NULL, CBCGPChartAxis* pXAxis = NULL);

	virtual void CopyFrom(const CBCGPChartBaseFormula& src);
	virtual CBCGPChartValue CalculateDataPoint(int nIndex, CBCGPChartSeries* pInputSeries, CBCGPChartValue& valY1);

	virtual BCGPChartCategory GetOutputSeriesCategory() const {return BCGPChartArea;}
	virtual BCGPChartType GetOutputSeriesType() const {return BCGP_CT_RANGE;}

	int GetDeviation() const {return m_nDeviation;}
	void SetDeviation(int nDeviation, BOOL bGenerateDataPoints = FALSE);

protected:
	int m_nDeviation;
	double m_dblBandOpacity;
};
//****************************************************************************************
class BCGCBPRODLLEXPORT CBCGPChartMACDFormula : public CBCGPChartAdvancedFormula
{
	DECLARE_DYNCREATE(CBCGPChartMACDFormula);

public:
	CBCGPChartMACDFormula(int nFastEMAPeriod = 12, int nSlowEMAPeriod = 26);

	int GetFastEMAPeriod() const {return m_nFastEMAPeriod;}
	int GetSlowEMAPeriod() const {return m_nSlowEMAPeriod;}

	void SetPeriods(int nFastEMAPeriod = 12, int nSlowEMAPeriod = 26, BOOL bGeneratePoints = FALSE);

	virtual void CopyFrom(const CBCGPChartBaseFormula& src);
	virtual CBCGPChartValue CalculateDataPoint(int nIndex, CBCGPChartSeries* pInputSeries, CBCGPChartValue& valY1);
	virtual BCGPChartCategory GetOutputSeriesCategory() const {return BCGPChartHistogram;}
	
protected:
	int m_nFastEMAPeriod;
	int m_nSlowEMAPeriod;

	double	m_dblFastEMAKoef;
	double	m_dblSlowEMAKoef;
};
//****************************************************************************************
// CBCGPChartTransitionFormula - implements formulas that generate data points for a 
// "formula series" using mathematical operations like sum, diff etc. Supports
// custom formulas (transitions)
//****************************************************************************************

class BCGCBPRODLLEXPORT CBCGPChartTransitionFormula : public CBCGPChartBaseFormula
{
	DECLARE_DYNCREATE(CBCGPChartTransitionFormula)

protected:
	CBCGPChartTransitionFormula();
	void CommonInit();

public:
	enum TransitionType
	{
		TT_UNDEFINED = -1,
		TT_SUM,
		TT_DIFF,
		TT_HIGH,
		TT_LOW,
		TT_MULTIPLY,
		TT_DIVIDE,
		TT_AVERAGE,
		TT_DIFF_ABS
	};

public:
// Construction
	CBCGPChartTransitionFormula(TransitionType type, LPARAM lParam = NULL);
	CBCGPChartTransitionFormula(BCGPCHART_TRANSITION_CALLBACK pCallback, LPARAM lParam = NULL);

// Operations
	void AddInputSeries(CBCGPChartSeries* pSeries);
	void SetInputSeries(const CArray<CBCGPChartSeries*, CBCGPChartSeries*>& arSeries);
	void RemoveAllInputSeries();
	void RemoveInputSeries(CBCGPChartSeries* pSeries);

	void SetTransitionCallback(BCGPCHART_TRANSITION_CALLBACK pCallback, LPARAM lParam = NULL);
	BCGPCHART_TRANSITION_CALLBACK GetTransitionCallback() const
	{
		return m_pfnTransitionCallback;
	}

	void SetTransitionType(TransitionType type, LPARAM lParam = NULL);
	TransitionType GetTransitionType() const {return m_type;}

// Copy
	virtual void CopyFrom(const CBCGPChartBaseFormula& src);

// Overrides
public:
	virtual void GeneratePoints();

protected:
	// override in a derived class and return TRUE for custom data point processing.
	virtual BOOL OnCalculateDataPoint(const CArray<CBCGPChartData, CBCGPChartData>& data, int nDataPointIndex, 
										CBCGPChartData& dataNew)
	{
		UNREFERENCED_PARAMETER(data);
		UNREFERENCED_PARAMETER(nDataPointIndex);
		UNREFERENCED_PARAMETER(dataNew);

		return FALSE;
	}

protected:
	CBCGPChartDataPoint* CalculateDataPoint(const CArray<CBCGPChartData, CBCGPChartData>& data, int nDataPointIndex);


protected:
	BCGPCHART_TRANSITION_CALLBACK	m_pfnTransitionCallback;
	TransitionType					m_type;
};

//****************************************************************************************
// CBCGPChartTrendFormula - generates trend lines for a given series. A trend line is drawn
// as a non-discrete curve using specified step. 
// As an additional feature can draw exponential, logarithmic and other curves with custom
// coefficients. Supports custom formulas like y = sin(x).
//****************************************************************************************

class BCGCBPRODLLEXPORT CBCGPChartTrendFormula : public CBCGPChartBaseFormula
{
	DECLARE_DYNCREATE(CBCGPChartTrendFormula)

protected:
	CBCGPChartTrendFormula();
	void CommonInit();

public:
	enum TrendLineType
	{
		TLT_UNDEFINED = -1,
		TLT_LINEAR,
		TLT_EXPONENTIAL,
		TLT_LOGARITHMIC,
		TLT_POLYNOMIAL,
		TLT_POWER
	};

public:
// Construction
	CBCGPChartTrendFormula(TrendLineType type, CBCGPChartSeries* pSeries = NULL, LPARAM lParam = NULL);

// Operations
	void SetInputSeries(CBCGPChartSeries* pSeries);

	void SetTrendType(TrendLineType type, LPARAM lParam = NULL);
	TrendLineType GetTrendType() const {return m_type;}

	void SetCoefficients(const CArray<double, double>& arCoef);
	void AddCoefficient(double dblCoef);
	void RemoveAllCoefficients() {m_arCoef.RemoveAll();}
	const CArray<double, double>& GetCoefficients() const {return m_arCoef;} 
	virtual CString GetFormulaText() const;

	void SetTrendRange(double dblMinVal, double dblMaxVal);
	void GetTrendRange(double& dblMinVal, double& dblMaxVal) const;

	void SetPolynomialTrendOrder(int nOrder);
	int GetPolynomialTrendOrder() const {return m_nPolynomialTrendOrder;}

	const CBCGPPointsArray& GetScreenPoints() const {return m_arScreenPoints;}

// Copy
	virtual void CopyFrom(const CBCGPChartBaseFormula& src);

// Overrides
public:
	virtual void GeneratePoints();
	virtual BOOL IsNonDiscreteCurve() const {return TRUE;}

protected:
	// override in a derived class and return TRUE to implement custom trend lines'
	// dblXVal is X value in chart coordinates;
	// dblYVal should be calculated by this method like dblYVal = sin(dblXVal)
	virtual BOOL OnCalculateYValue(double dblXVal, CBCGPChartValue& dblYVal)
	{
		UNREFERENCED_PARAMETER(dblXVal);
		UNREFERENCED_PARAMETER(dblYVal);

		return FALSE;
	}

	virtual void CalculateTrendCoefficients();

protected:
	virtual void CalculateLinearTrend();
	virtual void CalculateLogarithmicTrend();
	virtual void CalculateExponentialTrend();
	virtual void CalculatePowerTrend();
	virtual void CalculatePolynomialTrend();

	virtual CBCGPChartValue CalculateYValue(double dblXVal);

protected:
	TrendLineType				m_type;

	CBCGPPointsArray			m_arScreenPoints; // trend line screen points
	CArray<double, double>		m_arCoef; // trend line coefficients 

	double						m_dblMinRange;
	double						m_dblMaxRange;

	int							m_nPolynomialTrendOrder;
};

//****************************************************************************************
// Helper to implement virtual series
//****************************************************************************************

class CBCGPChartVirtualFormula : public CBCGPChartTrendFormula
{
	DECLARE_DYNCREATE(CBCGPChartVirtualFormula)

protected:
	CBCGPChartVirtualFormula();
	void CommonInit();
public:
	CBCGPChartVirtualFormula(BCGPCHART_VSERIES_CALLBACK pCallback, LPARAM lParam = NULL);

	void SetVirtualCallback(BCGPCHART_VSERIES_CALLBACK pCallback, LPARAM lParam = NULL);
	BCGPCHART_VSERIES_CALLBACK GetVirtualCallback() const
	{
		return m_pfnVirtualCallback;
	}

	virtual void CopyFrom(const CBCGPChartBaseFormula& src);

protected:
	virtual CBCGPChartValue CalculateYValue(double dblXVal);
protected:
	BCGPCHART_VSERIES_CALLBACK	m_pfnVirtualCallback;
};

#endif // !defined(AFX_BCGPCHARTFORMULA_H__FE0BBF37_BD61_474E_B4BE_D6D7AFEC94AC__INCLUDED_)
