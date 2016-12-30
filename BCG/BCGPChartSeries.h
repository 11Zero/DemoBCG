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
// BCGPChartSeries.h: declaration of the CBCGPChartSeries and derived class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPCHARTSERIES_H__E7B32F7D_06C9_42FC_881D_543E11E643F1__INCLUDED_)
#define AFX_BCGPCHARTSERIES_H__E7B32F7D_06C9_42FC_881D_543E11E643F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPVisualContainer.h"
#include "BCGPChartImpl.h"
#include "BCGPChartShape3D.h"

class CBCGPChartAxis;
class CBCGPChartBaseFormula;
struct BCGPChartHitInfo;

//****************************************************************************************
// Basic chart data value
//****************************************************************************************

class BCGCBPRODLLEXPORT CBCGPChartValue
{
public:
	CBCGPChartValue()
	{
		m_bIsEmpty = TRUE;
		m_dblVal = 0.;
		m_pAxis = NULL;
	}

	CBCGPChartValue(const CBCGPChartValue& src)
	{
		CopyFrom(src);
	}

	CBCGPChartValue(double dblValue, CBCGPChartAxis* pAxis = NULL)
	{
		m_dblVal = dblValue;
		m_bIsEmpty = FALSE;
		m_pAxis = pAxis;
	}

	void SetEmpty(BOOL bEmpty = TRUE)
	{
		m_bIsEmpty = bEmpty;
		m_pAxis = NULL;
	}

	BOOL IsEmpty() const
	{
		return m_bIsEmpty;
	}

	void SetValue(double dblValue, CBCGPChartAxis* pAxis = NULL)
	{
		m_dblVal = dblValue;
		m_bIsEmpty = FALSE;
		m_pAxis = pAxis;
	}

	double GetValue() const
	{
		return m_dblVal;
	}

	CBCGPChartAxis* GetAxis () const
	{
		return m_pAxis;
	}

	operator double() const
	{
		return m_dblVal;
	}

	CBCGPChartValue& operator=(const CBCGPChartValue& src)
	{
		CopyFrom(src);
		return *this;
	}

	void CopyFrom(const CBCGPChartValue& src)
	{
		m_bIsEmpty = src.IsEmpty();
		m_dblVal = src.GetValue();
		m_pAxis = src.GetAxis();
	}
protected:
	CBCGPChartAxis*	m_pAxis;
	BOOL			m_bIsEmpty;
	double			m_dblVal;
};

//****************************************************************************************
// CBCGPChartData - a composition of several chart values that represent a data point
//****************************************************************************************

class BCGCBPRODLLEXPORT CBCGPChartData
{
	friend class CBCGPChartSurfaceSeries;
public:
	enum ComponentIndex
	{
		CI_Y,
		CI_X,
		CI_Z,
		CI_Y1,
		CI_Y2,
		CI_Y3,
		CI_PERCENTAGE,
		CI_CUSTOM
	};

	CBCGPChartData()
	{
	}

	CBCGPChartData(const CBCGPChartData& src)
	{
		CopyFrom(src);
	}

	CBCGPChartData(const CBCGPChartValue& dblX, const CBCGPChartValue& dblY, const CBCGPChartValue& dblY1)
	{
		m_arData.SetAtGrow(CBCGPChartData::CI_Y, dblY);
		m_arData.SetAtGrow(CBCGPChartData::CI_X, dblX);
		m_arData.SetAtGrow(CBCGPChartData::CI_Y1, dblY1);
	}

	CBCGPChartData(const CBCGPChartValue& dblVal, ComponentIndex ci = CBCGPChartData::CI_Y)
	{
		m_arData.SetAtGrow(ci, dblVal);
	}

	CBCGPChartData(const CBCGPChartValue& dblX, const CBCGPChartValue& dblY)
	{
		m_arData.SetAtGrow(CBCGPChartData::CI_Y, dblY);
		m_arData.SetAtGrow(CBCGPChartData::CI_X, dblX);
	}

	CBCGPChartData(const CArray<CBCGPChartValue, CBCGPChartValue>& arData)
	{
		SetValues(arData);
	}

	CBCGPChartData(const CArray<double, double>& arData)
	{
		SetValues(arData);
	}

	BOOL IsEmpty() const
	{
		for (int i = 0; i < m_arData.GetSize(); i++)
		{
			CBCGPChartValue val = m_arData[i];
			if (!val.IsEmpty())
			{
				return FALSE;
			}
		}
		return TRUE;
	}

	void SetEmpty(BOOL bEmpty = TRUE)
	{
		for (int i = 0; i < m_arData.GetSize(); i++)
		{
			CBCGPChartValue& val = m_arData[i];
			val.SetEmpty(bEmpty);
		}
	}

 	void SetValue(CBCGPChartValue val, ComponentIndex ci = CBCGPChartData::CI_Y)
 	{
 		m_arData.SetAtGrow(ci, val);
 	}

	void SetComponentCount(int nCount)
	{
		m_arData.SetSize(nCount);
		m_arData.FreeExtra();
	}

	void SetValues(const CArray<CBCGPChartValue, CBCGPChartValue>& arData)
	{
		m_arData.RemoveAll();
		m_arData.Append(arData);
	}

	void SetValues(const CArray<double, double>& arData)
	{
		m_arData.RemoveAll();
		for (int i = 0; i < arData.GetSize(); i++)
		{
			m_arData.Add(CBCGPChartValue(arData[i]));
		}
	}

	CBCGPChartValue GetValue(ComponentIndex ci = CBCGPChartData::CI_Y) const
	{
		if (ci >= m_arData.GetSize() || ci < 0)
		{
			return CBCGPChartValue();
		}

		return m_arData[ci];
	}

	int GetComponentCount() const
	{
		return (int) m_arData.GetSize();
	}

	CBCGPChartData& operator=(const CBCGPChartData& src)
	{
		CopyFrom(src);
		return *this;
	}

	void CopyFrom(const CBCGPChartData& src)
	{
		m_arData.RemoveAll();

		for (int i = 0; i < src.GetComponentCount(); i++)
		{
			m_arData.SetAtGrow(i, src.GetValue((CBCGPChartData::ComponentIndex)i));
		}
	}

protected:
	CArray<CBCGPChartValue, CBCGPChartValue>	m_arData;
};

//****************************************************************************************
// CBCGPChartDataPoint
//****************************************************************************************

class BCGCBPRODLLEXPORT CBCGPChartDataPoint
{
	friend class CBCGPBaseChartImpl;
	friend class CBCGPStockChartImpl;
	friend class CBCGPChartSeries;
	friend class CBCGPChartStockSeries;
	friend class CBCGPChartSurfaceSeries;
	friend class CBCGPSurfaceChart3DImpl;
	friend class CBCGPChartLegendCell;
	friend class CBCGPBaseChartStockSeries;
	friend class CBCGPChartVisualObject;

public:
	CBCGPChartDataPoint()
	{
		CommonInit();
	}

	CBCGPChartDataPoint (double dblVal, CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pFormatElement = NULL)
	{
		CommonInit();

		m_data.SetValue(dblVal, ci);
		m_dwUserData = dwUserData;

		if (pFormatElement != NULL)
		{
			m_pFormatElement = new BCGPChartFormatSeries ();
			*m_pFormatElement = *pFormatElement;
		}
	}

	CBCGPChartDataPoint (const CString& strCategoryName, double dblVal, CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pFormatElement = NULL)
	{
		CommonInit();

		m_data.SetValue(dblVal, ci);
		m_dwUserData = dwUserData;

		if (pFormatElement != NULL)
		{
			m_pFormatElement = new BCGPChartFormatSeries ();
			*m_pFormatElement = *pFormatElement;
		}

		m_strCategoryName = strCategoryName;
	}

	CBCGPChartDataPoint (const CString& strCategoryName, const CBCGPChartValue& valX, 
		const CBCGPChartValue& valY, const CBCGPChartValue& valY1, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pFormatElement = NULL)
	{
		CommonInit();

		m_data.SetValue(valY, CBCGPChartData::CI_Y);
		m_data.SetValue(valX, CBCGPChartData::CI_X);
		m_data.SetValue(valY1, CBCGPChartData::CI_Y1);

		m_dwUserData = dwUserData;

		if (pFormatElement != NULL)
		{
			m_pFormatElement = new BCGPChartFormatSeries ();
			*m_pFormatElement = *pFormatElement;
		}

		m_strCategoryName = strCategoryName;
	}

	CBCGPChartDataPoint (const CBCGPChartValue& valY, 
		const CBCGPChartValue& valY1, const CBCGPChartValue& valY2, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pFormatElement = NULL)
	{
		CommonInit();

		m_data.SetValue(valY, CBCGPChartData::CI_Y);
		m_data.SetValue(valY1, CBCGPChartData::CI_Y1);
		m_data.SetValue(valY2, CBCGPChartData::CI_Y2);

		m_dwUserData = dwUserData;

		if (pFormatElement != NULL)
		{
			m_pFormatElement = new BCGPChartFormatSeries ();
			*m_pFormatElement = *pFormatElement;
		}
	}

	CBCGPChartDataPoint (const CArray<double, double>& arData, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pFormatElement = NULL)
	{
		CommonInit();

		m_data.SetValues(arData);
		m_dwUserData = dwUserData;

		if (pFormatElement != NULL)
		{
			m_pFormatElement = new BCGPChartFormatSeries ();
			*m_pFormatElement = *pFormatElement;
		}
	}

	CBCGPChartDataPoint (const CBCGPChartDataPoint& src)
	{
		CommonInit();
		CopyFrom(src);
	}

	void CommonInit()
	{
		m_pFormatElement = NULL;
		m_dwUserData = 0;
		m_rectBounds.SetRectEmpty();
		m_rectDataLabel.SetRectEmpty();
		m_ptLabelLineStart.SetPoint(0, 0);
		m_ptLabelLineEnd.SetPoint(0, 0);
		m_bUseWordWrapForDataLabels = FALSE;
		m_bIncludeLabelToLegend = TRUE;
		m_nLegendID = 0;
		m_nColorIndex = -1;

		m_nSmartLabelAngle = -1;
		m_dblSmartLabelDistance = DBL_MAX;
		m_bShowSmartLabel = TRUE;

		m_pShape3D = NULL;
	}

	virtual ~CBCGPChartDataPoint ()
	{
		ClearFormat();
		ClearShape3D();
	}

	const BCGPChartFormatSeries* GetFormat() const {return m_pFormatElement;}
	void SetFormat(const BCGPChartFormatSeries& format)
	{
		if (m_pFormatElement == NULL)
		{
			m_pFormatElement = new BCGPChartFormatSeries ();
		}

		*m_pFormatElement = format;
	}

	void ClearFormat()
	{
		if (m_pFormatElement != NULL)
		{
			delete m_pFormatElement;
		}

		m_pFormatElement = NULL;
	}

	void ClearShape3D()
	{
		if (m_pShape3D != NULL)
		{
			delete m_pShape3D;
			m_pShape3D = NULL;
		}
	}

	CBCGPChartValue GetComponentValue(CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y) const
	{
		return m_data.GetValue(ci);
	}

	void SetComponentValue(CBCGPChartValue val, CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y)
	{
		m_data.SetValue(val, ci);
	}

	void SetComponentCount(int nCount)
	{
		m_data.SetComponentCount(nCount);
	}

	void SetValues(const CArray<double, double>& arValues)
	{
		m_data.SetValues(arValues);
	}

	void SetValues(const CArray<CBCGPChartValue, CBCGPChartValue>& arValues)
	{
		m_data.SetValues(arValues);
	}

	void SetData(const CBCGPChartData& data)
	{
		m_data = data;
	}

	const CBCGPChartData& GetData() const
	{
		return m_data;
	}

	BOOL IsEmpty() const
	{
		return m_data.IsEmpty();
	}

	int GetDataComponentCount() const
	{
		return m_data.GetComponentCount();
	}

	CBCGPChartDataPoint& operator=(const CBCGPChartDataPoint& src)
	{
		CopyFrom(src);
		return *this;
	}

	void CopyFrom(const CBCGPChartDataPoint& src)
	{
		m_data = src.GetData();
		m_dwUserData = src.m_dwUserData;

		m_strLegendLabel = src.m_strLegendLabel;

		m_arScreenPoints.RemoveAll();
		m_arScreenPoints.Append(src.m_arScreenPoints);

		m_strDataLabel = src.m_strDataLabel;

		m_rectDataLabel = src.m_rectDataLabel;
		m_ptLabelLineStart = src.m_ptLabelLineStart;
		m_ptLabelLineEnd = src.m_ptLabelLineEnd;

		m_bIncludeLabelToLegend = src.m_bIncludeLabelToLegend;
		m_rectLegendKey = src.m_rectLegendKey;
		m_rectLegendLabel = src.m_rectLegendLabel;
		m_nLegendID = src.m_nLegendID;

		m_nColorIndex = src.GetColorIndex();
		m_nSmartLabelAngle = src.m_nSmartLabelAngle;
		m_dblSmartLabelDistance = src.m_dblSmartLabelDistance;
		m_bShowSmartLabel = src.m_bShowSmartLabel;

		m_strCategoryName = src.GetCategoryName();

		const BCGPChartFormatSeries* pFormatSeries = src.GetFormat();
		if (pFormatSeries != NULL)
		{
			SetFormat(*pFormatSeries);
		}
		else
		{
			ClearFormat();
		}

		CBCGPChartShape3D* pShapeSrc = src.GetShape3D();
		if (pShapeSrc != NULL)
		{
			SetShape3D(*pShapeSrc);
		}
	}

	virtual CBCGPChartShape3D*	GetShape3D() const {return m_pShape3D;}

	virtual const CString& GetCategoryName() const {return m_strCategoryName;} 
	virtual void SetCategoryName(const CString& strName){m_strCategoryName = strName;}

	virtual int GetColorIndex() const {return m_nColorIndex;}
	virtual void SetColorIndex(int nIndex) {m_nColorIndex = nIndex;}

	virtual BOOL IsUseWordWrapForDatalabels() const {return m_bUseWordWrapForDataLabels;}

	int GetLegendID() const {return m_nLegendID;}

	// screen coordinates of data point in specific chart representation
	// for line chart, bar chart, area chart - only one point
	// stacked area - two points
	// can be more points for complex charts
	CBCGPRect				m_rectBounds; // bounding rect for calculation of data labels
	CString					m_strDataLabel;

	// allows to format legend labels for each series individually
	BOOL					m_bIncludeLabelToLegend;
	CString					m_strLegendLabel;
	CBCGPRect				m_rectLegendKey; // in coordinates of legend area
	CBCGPRect				m_rectLegendLabel; // in coordinates of legend area

	DWORD_PTR				m_dwUserData;

protected:
	void SetScreenPointAt(int nIndex, CBCGPPoint pt)
	{
		m_arScreenPoints.SetAtGrow(nIndex, pt);
	}

	void RemoveAllScreenPoints()
	{
		m_arScreenPoints.RemoveAll();
	}

	BOOL IsScreenPointsEmpty() const
	{
		return (m_arScreenPoints.GetSize() == 0);
	}

	int GetScreenPointCount() const
	{
		return (int)m_arScreenPoints.GetSize();
	}

	CBCGPPoint GetScreenPoint(int nIndex = 0) const
	{
		if (nIndex < 0 || nIndex >= m_arScreenPoints.GetSize())
		{
			return CBCGPPoint();
		}
		return m_arScreenPoints[nIndex];
	}

	void SetShape3D(const CBCGPChartShape3D& shape3D)
	{
		ClearShape3D();

		m_pShape3D = DYNAMIC_DOWNCAST(CBCGPChartShape3D, shape3D.GetRuntimeClass()->CreateObject());

		if (m_pShape3D != NULL)
		{
			m_pShape3D->CopyFrom(shape3D);
		}
	}

	void SetShape3D(CBCGPChartShape3D* pShape3D)
	{
		ClearShape3D();
		m_pShape3D = pShape3D;
	}

	int								m_nSmartLabelAngle;
	double							m_dblSmartLabelDistance;
	BOOL							m_bShowSmartLabel;

	CBCGPChartData					m_data;
	BCGPChartFormatSeries*			m_pFormatElement;
	CBCGPPointsArray				m_arScreenPoints; 
	CBCGPChartShape3D*				m_pShape3D;

	CBCGPRect						m_rectDataLabel;
	CBCGPPoint						m_ptLabelLineStart;
	CBCGPPoint						m_ptLabelLineEnd;
	CBCGPPoint						m_ptLabelUnderline;

	CString							m_strCategoryName;

	BOOL							m_bUseWordWrapForDataLabels;

	int								m_nLegendID;

	int								m_nColorIndex;
};

// params: dblXValue is a value in X axis coordinates; sent according to a value passed
// to SetCurveStep (by default 2) which corresponds to pixels and defines the curve resolution;
// a pointer to formula series
// return value: CBCGPChartValue with Y value calculated by the incoming X (like Y = sin(X)) - can be empty 
// for values that can't be calculated (log(x), 1/x etc)
typedef CBCGPChartValue (CALLBACK* BCGPCHART_VSERIES_CALLBACK)(double dblXValue, CBCGPChartSeries* pVirtualSeries, LPARAM lp);

//****************************************************************************************
// CBCGPChartSeries - the base series
//****************************************************************************************
class BCGCBPRODLLEXPORT CBCGPChartSeries : public CBCGPVisualDataObject
{
	friend class CBCGPBaseChartImpl;
	friend class CBCGPChartLegendCell;
	friend class CBCGPSurfaceChart3DImpl;
	friend class CBCGPChartVisualObject;

	DECLARE_DYNCREATE(CBCGPChartSeries)

	CBCGPChartSeries();

	CBCGPChartSeries(CBCGPChartVisualObject* pChartCtrl, 
		BCGPChartCategory chartCategory, 
		BCGPChartType chartType = BCGP_CT_DEFAULT);

	CBCGPChartSeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
		const CBCGPColor& seriesColor = CBCGPColor(), 
		BCGPChartCategory chartCategory = BCGPChartDefault, 
		BCGPChartType chartType = BCGP_CT_DEFAULT);
public:
	enum AxisIndex
	{
		AI_UNKNOWN = -1,
		AI_X = 0,
		AI_Y = 1,
		AI_Z = 2
	};

	enum TreatNulls
	{
		TN_VALUE,
		TN_SKIP,
		TN_NO_PAINT
	};

	void CommonInit(CBCGPChartVisualObject* pChartCtrl, 
		BCGPChartCategory chartCategory, 
		BCGPChartType chartType);
	void SetDefaultSeriesColor(const CBCGPColor& seriesColor);

	virtual ~CBCGPChartSeries();

	virtual void RecalcMinMaxValues();

	BOOL CanBeConvertedToCategory(BCGPChartCategory chartCategory) const;
	CBCGPChartSeries* SetChartType(BCGPChartCategory chartCategory, 
		BCGPChartType chartType = BCGP_CT_SIMPLE,
		BOOL bAdjustLayout = FALSE, BOOL bRedraw = FALSE);
	BCGPChartType GetChartType() const {return m_chartType;}
	BCGPChartCategory GetChartCategory() const {return m_chartCategory;}

	void SetChartCtrl(CBCGPChartVisualObject* pChartCtrl);
	CBCGPChartVisualObject* GetChartCtrl() const {return m_pChart;}

	void SetChartImpl(CBCGPBaseChartImpl* pImpl);
	CBCGPBaseChartImpl*	GetChartImpl(); 
	const CBCGPBaseChartImpl* GetChartImpl() const {return m_pChartImpl;}

	void SetVirtualMode(BOOL bSet, BCGPCHART_VSERIES_CALLBACK pCallBack, 
		LPARAM lParam = NULL, double dblMinRange = 0., double dblMaxRange = 0.);
	virtual const CBCGPPointsArray& GetVirtualSeriesScreenPoints() const;
	BOOL IsVirtualMode() const;
	
	virtual BOOL OnCalculateVirtualYValue(double dblXValue, CBCGPChartValue& dblYValue, LPARAM lp)
	{
		UNREFERENCED_PARAMETER(dblXValue);
		UNREFERENCED_PARAMETER(dblYValue);
		UNREFERENCED_PARAMETER(lp);

		return FALSE;
	}

	void SetFormula(const CBCGPChartBaseFormula& formula, BOOL bRedraw = FALSE);
	CBCGPChartBaseFormula* GetFormula() const;
	BOOL IsFormulaSeries() const;

	virtual void SetTreatNulls(CBCGPChartSeries::TreatNulls tn, BOOL bRecalcMinMax = TRUE);
	CBCGPChartSeries::TreatNulls GetTreatNulls() const {return m_treatNulls;} 

	virtual CBCGPChartValue GetMinValue(CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y);
	virtual CBCGPChartValue GetMaxValue(CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y);

	virtual void ClearMinMaxValues();
	void SetMinValue(double dblValue, CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y);
	void SetMaxValue(double dblValue, CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y);

	virtual void SetMinMaxValues(const CBCGPChartValue& val, CBCGPChartData::ComponentIndex ci, int nDataPointIndex);
	virtual void OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);
	virtual void OnGetDataPointLegendLabel(int nDataPointIndex, CString& strLabel);

	virtual void AdjustGradientAngels();
	
	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld);

	virtual int AddDataPoint(CBCGPChartDataPoint* pDataPoint);
	virtual int AddDataPoint(const CBCGPChartDataPoint& srcDataPoint);
	virtual int AddDataPoint(const CString& strCategoryName, double dblY, BCGPChartFormatSeries* pDataPointFormat = NULL, DWORD_PTR dwUserData = 0);
	virtual int AddDataPoint(double dblY, BCGPChartFormatSeries* pDataPointFormat = NULL, DWORD_PTR dwUserData = 0);
	virtual int AddDataPoint(double dblY, double dblX, BCGPChartFormatSeries* pDataPointFormat = NULL, DWORD_PTR dwUserData = 0);
	virtual int AddEmptyDataPoint(const CString& strCategoryName, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pDataPointFormat = NULL);
	virtual int AddEmptyDataPoint(DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pDataPointFormat = NULL);
	virtual int AddEmptyDataPoint(double dblX, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pDataPointFormat = NULL);

	virtual void AddDataPoints(const CBCGPDoubleArray& arYValues, CBCGPDoubleArray* pXValues = NULL, 
		CBCGPDoubleArray* pY1Values = NULL, BOOL bRecalcMinMaxValues = TRUE);
	virtual void AddDataPointsOptimized(const CBCGPDoubleArray& arYValues, CBCGPDoubleArray* pXValues = NULL, 
		CBCGPDoubleArray* pY1Values = NULL, BOOL bRecalcMinMaxValues = FALSE);

	virtual CBCGPChartData GetDataPointData(int nDataPointIndex) const;
	virtual CBCGPChartValue GetDataPointValue(int nDataPointIndex, CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y) const;
	virtual BOOL SetDataPointValue(int nDataPointIndex, double dblValue, CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y);
	virtual int GetDataPointCount() const;
	virtual const CBCGPChartDataPoint* GetDataPointAt(int nIndex) const;

	virtual int FindDataPointIndex(CBCGPChartDataPoint* pDataPoint);

	virtual void RemoveDataPoints(int nStartIdx, int nCount = 1, BOOL bFreeExtra = FALSE);
	virtual void RemoveAllDataPoints();

	virtual void MoveDataPoints(int nFromIdx, int nToIdx, int nCount = 1);
	virtual BOOL SetDataPointColorIndex(int nDataPointIndex, int nColorIndex);

	virtual int GetDataPointColorIndex(int nDataPointIndex);
	virtual BOOL IsDataPointScreenPointsEmpty(int nDataPointIndex) const;
	virtual int GetDataPointScreenPointCount(int nDataPointIndex) const;
	virtual CBCGPRect GetDataPointBoundingRect(int nDataPointIndex);
	virtual void SetDataPointBoundingRect(int nDataPointIndex, const CBCGPRect& rect);
	virtual CBCGPRect GetDataPointLabelRect(int nDataPointIndex) const;
	virtual BOOL SetDataPointLabelRect(int nDataPointIndex, const CBCGPRect& rect);
	virtual BOOL GetDataPointLabelDropLines(int nDataPointIndex, CBCGPPoint& ptLabelLineStart, 
		CBCGPPoint& ptLableLineEnd, CBCGPPoint& ptLabelUnderline) const;
	virtual BOOL SetUseWordWrapForDataLabels(BOOL bSet, int nDataPointIndex);
	virtual BOOL IsUseWordWrapForDataLabels(int nDataPointIndex);
	virtual double GetDataPointLabelAngle(int nDataPointIndex) const;
	virtual double GetDataPointLabelDistance(int nDataPointIndex) const;

	virtual LPCTSTR GetDataPointCategoryName(int nDataPointIndex);
	virtual BOOL SetDataPointCategoryName(const CString& strName, int nDataPointIndex);

	DWORD_PTR GetDataPointUserData(int nDataPointIndex) const;
	BOOL SetDataPointUserData(int nDataPointIndex, DWORD_PTR dwUserData);

	virtual BOOL GetDataPointLabelText(int nDataPointIndex, BCGPChartDataLabelOptions::LabelContent content, CString& strDPLabel);
	virtual BOOL GetDataPointTableText(int nDataPointIndex, CString& strDPLabel, int nPrecision = -1);
	virtual void GetDataTableName(CString& strName) { strName = m_strSeriesName; }

	virtual BOOL SetDataPointLabelDropLinePoints(int nDataPointIndex, const CBCGPPoint& ptLabelLineStart, 
		const CBCGPPoint& ptLabelLineEnd, const CBCGPPoint& ptLabelUnderline);
	virtual CBCGPPoint GetDataPointScreenPoint(int nDataPointIndex, int nScreenPointIndex = 0) const;
	virtual void SetDataPointScreenPoint(int nDataPointIndex, int nScreenPointIndex, CBCGPPoint pt);
	virtual void RemoveAllDataPointScreenPoints(int nDataPointIndex);
	virtual BOOL OnSetDataPointDataLabelText(int nDataPointIndex, const CString& strText);
	virtual BOOL OnGetDataLabelText(int nDataPointIndex, CString& strText) const;

	virtual BOOL OnGetDataPointTooltip(int nDataPointIndex, CString& strTooltip, CString& strTooltipDescr);

	virtual CBCGPChartShape3D* GetDataPointShape3D(int nDataPointIndex) const;
	virtual void SetDataPointShape3D(const CBCGPChartShape3D& shape3D, int nDataPointIndex);
	virtual void SetDataPointShape3D(CBCGPChartShape3D* pShape3D, int nDataPointIndex);
	
	virtual void CopyDataPoints(CArray<CBCGPChartDataPoint*, CBCGPChartDataPoint*>& dest) const;

	virtual BOOL IsAutoColorDataPoints() const {return m_bAutoColorDataPoints && !m_bHistoryMode;}
	virtual void EnableAutoColorDataPoints(BOOL bEnable = TRUE, BOOL bRedraw = FALSE);
	void SetColorIndex(int nIndex)
	{
		if (nIndex < 0)
		{
			nIndex = 0;
		}
		
		m_nColorIndex = nIndex;
	}
	int GetColorIndex() const {return m_nColorIndex;}

	virtual const BCGPChartFormatSeries* GetColors(BCGPSeriesColorsPtr& seriesColors, int nDataPointIndex);
	virtual BOOL IsColorEachLineEnabled() const {return FALSE;}
	
	CBCGPBrush::BCGP_GRADIENT_TYPE GetDefaultFillGradientType() const;

	void SetDefaultFillGradientType(CBCGPBrush::BCGP_GRADIENT_TYPE type)
	{
		if (type == (CBCGPBrush::BCGP_GRADIENT_TYPE) -1)
		{
			m_FillGradienType = GetSeriesFillGradientType();
		}
		else
		{
			m_FillGradienType = type;
		}
	}

	virtual BOOL IsDisplayShadow() const;

	int GetShadowAngle() const;
	virtual int GetDefaultShadowAngle() const
	{
		return 315;
	}

	int GetShadowTransparencyPercent() const;

	virtual void CopyFrom(const CBCGPChartSeries& src);
	void SetOrderIndex(int nIdx) {m_nOrderIndex = nIdx;}
	int GetOrderIndex() const {return m_nOrderIndex;}

	const BCGPChartFormatSeries* GetDataPointFormat(int nIndex, BOOL bAlloc);
	const BCGPChartFormatSeries* GetDataPointFormat(int nIndex) const;
	void ClearDataPointFormat(int nDataPointIndex = -1);

	//---- Visual Style wrappers
	virtual void ApplySeriesColorsToDataPointFormat(int nDataPointIndex);
	void SetMarkerFill(const CBCGPBrush& br, int nDataPointIndex = -1);
	void SetMarkerLineColor(const CBCGPBrush& br, int nDataPointIndex = -1);
	void SetMarkerLineWidth(double dblWidth, int nDataPointIndex = -1);
	void SetMarkerStrokeStyle(const CBCGPStrokeStyle& strokeStyle, int nDataPointIndex = -1);
	void SetMarkerSize (int nSize, int nDataPointIndex = -1);
	void SetMarkerShape(BCGPChartMarkerOptions::MarkerShape shape, int nDataPointIndex = -1);
	void SetMarkerFormat(const BCGPChartFormatMarker& format, int nDataPointIndex = -1);
	void SetMarkerOptions(const BCGPChartMarkerOptions& options, int nDataPointIndex = -1);
	void ShowMarker(BOOL bShow, int nDataPointIndex = -1);
	void SetSeriesFill(const CBCGPBrush& br, int nDataPointIndex = -1);
	void SetSeriesLineColor(const CBCGPBrush& br, int nDataPointIndex = -1);
	void SetSeriesLineWidth(double dblWidth, int nDataPointIndex = -1);
	void SetSeriesLineDashStyle(CBCGPStrokeStyle::BCGP_DASH_STYLE style, int nDataPointIndex = -1);
	void SetSeriesStrokeStyle(const CBCGPStrokeStyle& strokeStyle, int nDataPointIndex = -1);
	void SetSeriesElementFormat(const BCGPChartFormatArea& format, int nDataPointIndex = -1);
	void ShowDataLabel(BOOL bShow, int nDataPointIndex = -1);
	void SetDataLabelOptions(const BCGPChartDataLabelOptions& options, int nDataPointIndex = -1);
	void SetDataLabelFill(const CBCGPBrush& br, int nDataPointIndex = -1);
	void SetDataLabelLineColor(const CBCGPBrush& br, int nDataPointIndex = -1);
	void SetDataLabelLineWidth(double dblWidth, int nDataPointIndex = -1);
	void SetDataLabelStrokeStyle(const CBCGPStrokeStyle& strokeStyle, int nDataPointIndex = -1);
	void SetDataLabelTextFormat(const CBCGPTextFormat& textFormat, int nDataPointIndex = -1);
	void SetDataLabelDrawBorder(BOOL bDraw, int nDataPointIndex = -1);
	void SetDataLabelDropLineToMarker(BOOL bDrop, int nDataPointIndex = -1);
	void SetDataLabelUnderline(BOOL bUnderline, int nDataPointIndex = -1);
	void SetDataLabelFormat(const BCGPChartFormatDataLabel& format, int nDataPointIndex = -1);
	//---- Other Format Wrapper
	void SetDataLabelSeparator(const CString& strSeparator, int nDataPointIndex = -1);
	void SetDataLabelDisplayKey(BOOL bSet, int nDataPointIndex = -1);
	void SetDataLabelAngle(double dblAngle, int nDataPointIndex = -1);
	void SetDataLabelDistanceFromMarker(double dblOffset, int nDataPointIndex = -1);
	void SetDataLabelPosition(BCGPChartDataLabelOptions::LabelPosition position, int nDataPointIndex = -1);
	void SetDataLabelContent(BCGPChartDataLabelOptions::LabelContent content, int nDataPointIndex = -1);
	void SetDataLabelContentPadding(const CBCGPSize& sz, int nDataPointIndex = -1);
	void SetDataLabelDataFormat(const CString& strFormat, int nDataPointIndex = -1);
	void SetLegendLabelTextFormat(const CBCGPTextFormat& textFormat, int nDataPointIndex = -1);
	void SetLegendLabelTextColor(const CBCGPBrush& br, int nDataPointIndex = -1);
	void SetLegendLabelFillColor(const CBCGPBrush& br, int nDataPointIndex = -1);
	void SetLegendLabelFormat(const BCGPChartFormatLabel& format, int nDataPointIndex = -1);
	void SetLegendLabelContent(BCGPChartDataLabelOptions::LabelContent content, int nDataPointIndex = -1);
	void SetCurveType(BCGPChartFormatSeries::ChartCurveType type);
	BCGPChartFormatSeries::ChartCurveType GetCurveType() const;

	void SetDataPoint3DLineThickness(double dblLineThickness, int nDataPointIndex = -1);
	//----

	virtual BOOL IsComponentSet(CBCGPChartData::ComponentIndex ci) const 
	{
		return ci == CBCGPChartData::CI_X ? IsXComponentSet() : m_bZComponentSet;
	}

	virtual BOOL IsXComponentSet() const 
	{
		if (IsOptimizedLongDataMode())
		{
			return m_arLongDataX.GetSize() > 0;
		}
		return m_bXComponentSet || m_arLongDataX.GetSize() > 0;
	}
	virtual BOOL IsZComponentSet() const {return m_bZComponentSet;}

	virtual CBCGPSize GetMaxDataLabelSize() const {return m_szMaxDataLabelSize;}
	virtual void ResetMaxDataLabelSize() {m_szMaxDataLabelSize.SetSizeEmpty();}

	virtual void ShowOnPrimaryAxis(BOOL bPrimary = TRUE);
	virtual void SetRelatedAxes(CBCGPChartAxis* pXAxis, CBCGPChartAxis* pYAxis, CBCGPChartAxis* pZAxis = NULL);
	virtual void SetRelatedAxis(CBCGPChartAxis* pAxis, CBCGPChartSeries::AxisIndex axisIndex);
	virtual void ReplaceAxis(CBCGPChartAxis* pAxisOld, CBCGPChartAxis* pAxisNew);
	CBCGPChartAxis* GetRelatedAxis(CBCGPChartSeries::AxisIndex axisIndex) const;
	AxisIndex CBCGPChartSeries::GetRelatedAxisIndex(CBCGPChartAxis* pAxis);
	BOOL IsShownOnAxis(const CBCGPChartAxis* pAxis) const
	{
		ASSERT_VALID(this);

		return (pAxis == m_pXAxis || pAxis == m_pYAxis || pAxis == m_pZAxis);
	}
	BOOL IsShownOnCustomOrResizedAxis() const;

	void SetGroupID(int nGroupID = 0, BOOL bRecalcMinMaxValues = TRUE);
	int GetGroupID() const {return m_nGroupID;}

	BOOL IsRangeSeries() const {return (m_chartType == BCGP_CT_RANGE);}
	BOOL IsStakedSeries() const 
	{
		return (m_chartType == BCGP_CT_STACKED || 
				m_chartType == BCGP_CT_100STACKED);
	}

	BOOL IsBackgroundOrder() const {return m_bIsBackgroundOrder;}
	void SetBackroundOrder(BOOL bIsBackOrder) {m_bIsBackgroundOrder = bIsBackOrder;}

	virtual CBCGPRect GetAxesBoundingRect();

	virtual void EnableHistoryMode(BOOL bEnable, int nHistoryDepth = 1000, BOOL bReverseOrder = FALSE, 
		BOOL bSetDefaultValue = FALSE, double dblDefaultYValue = 0.);
	BOOL IsHistoryMode() const {return m_bHistoryMode;}
	virtual double GetFixedMajorXUnit() const {return 1.;}
	virtual void UpdateAxes();
	int GetHistoryDepth() {return m_nHistoryDepth;}
	
	void SetOffsetFromNullZone(int nOffset){m_nOffsetFromNullZone = nOffset;}
	int GetOffsetFromNullZone() const {return m_nOffsetFromNullZone;}

	int GetMinDataPointIndex() const;
	int GetMaxDataPointIndex() const;

	void EnableUpdateAxesOnNewData(BOOL bEnable = TRUE)
	{
		m_bUpdateAxesOnNewData = bEnable;
	}
	BOOL IsUpdateAxesOnNewDataEnabled() const {return m_bUpdateAxesOnNewData;}

	virtual void SetIndexMode(BOOL bSet);
	virtual BOOL IsIndexMode() const {return m_bIndexMode;}

	const BCGPChartFormatSeries& GetSeriesFormat()
	{
		return m_formatSeries;
	}

	void SetSeriesFormat(const BCGPChartFormatSeries& format)
	{
		m_formatSeries = format;
	}

	void ResetFormat()
	{
		m_formatSeries.Reset();
	}

	virtual void UpdateSeriesColors();

	virtual BCGPChartDataLabelOptions::LabelContent GetDefaultLegendContent() const {return BCGPChartDataLabelOptions::LC_X_VALUE;}

	virtual BOOL IsIgnoreNegativeValues() const {return FALSE;}
	virtual void SetIgnoreNegativeValues(BOOL bIgnore) {m_bIgnoreNegativeValues = bIgnore;}

	virtual BOOL CanIncludeDataPointToLegend(int nDataPointIndex);

	double GetMinDataPointDistance() const {return m_dblMinDataPointDistance;}

	virtual CBCGPPoint ScreenPointFromChartData(const CBCGPChartData& data, int nDataPointIndex);

	void SetFullStackedMinMax();
	BOOL IsFullStackedMinMaxSet() const {return m_bFullStackedMinMaxSet;}

	virtual int GetLegendElementCount() const;

	CBCGPDoubleArray* GetDataBuffer(int nBufferIndex);
	virtual void ResizeDataPointArray(int nNewSize);
	virtual void ResizeLongDataArray(int nNewSize, BOOL bGrowByCount = TRUE,  CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y);
	virtual BOOL IsOptimizedLongDataMode() const {return m_arLongDataY.GetSize() > 0;}
	virtual double GetLongDataEmptyValue() const {return m_dblEmptyValue;}
	void SetLongDataEmptyValue(double dblVal) {m_dblEmptyValue = dblVal;}
	virtual int GetLongDataOffset() const {return m_nLongDataOffset;}
	void SetLongDataOffset(int nOffset = 0) {m_nLongDataOffset = nOffset;}
	void ClearBoundingRectMap() 
	{
		m_mapDataPointBoundingRects.RemoveAll();
		m_mapDataPointLabelsRects.RemoveAll();
	}

	const CBCGPDoubleArray& GetLongDataY() const {return m_arLongDataY;}
	const CBCGPDoubleArray& GetLongDataX() const {return m_arLongDataX;}
	const CBCGPDoubleArray& GetLongDataY1() const {return m_arLongDataY1;}

	virtual BOOL IsFillClosedShape() const {return FALSE;}
	virtual BOOL HitTestDataPoint(const CBCGPPoint& pt, BCGPChartHitInfo* pHitInfo);

public:
	CString				m_strSeriesName;

	BOOL				m_bIncludeSeriesToLegend;
	BOOL				m_bIncludeSeriesToDataTable;
	BOOL				m_bIncludeInvisibleSeriesToLegend;
	BOOL				m_bIncludeInvisibleSeriesToDataTable;
	BOOL				m_bIncludeDataPointLabelsToLegend;

	int					m_nLegendKeyToLabelDistance;
	CBCGPRect			m_rectLegendKey; // in coordinates of legend area
	CBCGPRect			m_rectLegendLabel; // in coordinates of legend area

	BOOL				m_bVisible;

	DWORD_PTR GetUserData() const
	{
		return m_dwUserData;
	}

	void SetUserData(DWORD_PTR dwUserData)
	{
		m_dwUserData = dwUserData;
	}

	BOOL IsLegendEntryVisible() const
	{
		return (m_bVisible || m_bIncludeInvisibleSeriesToLegend) && m_bIncludeSeriesToLegend;
	}

	BOOL IsDataTableEntryVisible() const
	{
		return (m_bVisible || m_bIncludeInvisibleSeriesToDataTable) && m_bIncludeSeriesToDataTable;
	}

protected:
	CArray<CBCGPChartDataPoint*, CBCGPChartDataPoint*>	m_arDataPoints;
	CArray<CBCGPDoubleArray*, CBCGPDoubleArray*>		m_arDataBuffers;

	//--- support for long data
	CBCGPDoubleArray									m_arLongDataY;
	CBCGPDoubleArray									m_arLongDataY1;
	CBCGPDoubleArray									m_arLongDataX;
	CBCGPChartDataPoint									m_virtualPoint;
	int													m_nLongDataOffset;
	CMap<int, int, CBCGPRect, const CBCGPRect&>			m_mapDataPointBoundingRects;
	CMap<int, int, CBCGPRect, const CBCGPRect&>			m_mapDataPointLabelsRects;

	double												m_dblEmptyValue;
	//---
	
	BCGPChartFormatSeries			m_formatSeries;

	BCGPChartCategory				m_chartCategory;
	BCGPChartType					m_chartType;
	CBCGPChartData					m_minValues;
	CBCGPChartData					m_maxValues;

	CBCGPChartSeries::TreatNulls	m_treatNulls;

	CBCGPBaseChartImpl*				m_pChartImpl;
	CBCGPChartVisualObject*			m_pChart;
	int								m_nGroupID;
	int								m_nOrderIndex;
	int								m_nLegendID;

	BOOL							m_bGetDataPointLabelForLegend;

	BOOL							m_bAutoColorDataPoints;
	int								m_nColorIndex;
	CBCGPBrush::BCGP_GRADIENT_TYPE	m_FillGradienType;

	BOOL							m_bXComponentSet;
	BOOL							m_bZComponentSet;
	CBCGPSize						m_szMaxDataLabelSize;

	CBCGPChartAxis*					m_pXAxis;
	CBCGPChartAxis*					m_pYAxis;
	CBCGPChartAxis*					m_pZAxis;

	int								m_nMinDisplayedXIndex;
	int								m_nMaxDisplayedXIndex;

	BOOL							m_bHistoryMode;
	int								m_nHistoryDepth;
	int								m_nCurrentIndex;
	BOOL							m_bIndexMode;

	int								m_nFixedGridSizeX;
	int								m_nFixedGridSizeY;
	int								m_nValuesPerIntervalX;
	int								m_nValuesPerIntervalY;
	BOOL							m_bUpdateAxesOnNewData;
	BOOL							m_bReverseAdd;
	int								m_nOffsetFromNullZone;

	BOOL							m_bIgnoreNegativeValues;

	double							m_dblMinDataPointDistance;

	CBCGPChartBaseFormula*			m_pChartFormula;
	BOOL							m_bFullStackedMinMaxSet;

	BOOL							m_bIsBackgroundOrder;

	DWORD_PTR						m_dwUserData;

protected:
	virtual CBCGPBaseChartImpl* OnCreateChartImpl(BCGPChartCategory chartCategory);
	void SetMinMaxValuesSimple(double dblValue, CBCGPChartData::ComponentIndex ci, int nDataPointIndex);
	void SetMinMaxValuesRange(double dblValue, CBCGPChartData::ComponentIndex ci, int nDataPointIndex);
	void SetMinMaxValuesStacked(double dblValue, CBCGPChartData::ComponentIndex ci, int nDataPointIndex);
	void SetMinMaxValues100Stacked(double dblValue, CBCGPChartData::ComponentIndex ci, int nDataPointIndex);

	void OnCalcScreenPointsSimple(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);
	void OnCalcScreenPointsRange(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);
	void OnCalcScreenPointsStacked(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);
	void OnCalcScreenPoints100Stacked(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);

	virtual BOOL CanUseRangeForMinMax() const {return TRUE;}

	virtual CBCGPBrush::BCGP_GRADIENT_TYPE GetSeriesFillGradientType() const {return CBCGPBrush::BCGP_NO_GRADIENT;}

	void CalcFullStackedSums(int nDataPointIndex, double& dblStackedSum, double& dblPositiveStackedSum, 
		double& dblNegativeStackedSum, double& dblTotalSum);

	virtual void OnBeforeChangeType() {}
	virtual void UpdateAxisCategories();

private:
	CBCGPPointsArray		m_arDummyArray;
};

class BCGCBPRODLLEXPORT CBCGPChartLineSeries : public CBCGPChartSeries
{
	DECLARE_DYNCREATE(CBCGPChartLineSeries)

	CBCGPChartLineSeries();

public:
	CBCGPChartLineSeries(CBCGPChartVisualObject* pChartCtrl,  
		BCGPChartCategory chartCategory = BCGPChartLine,
		BCGPChartType chartType = BCGP_CT_DEFAULT);

	CBCGPChartLineSeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
		const CBCGPColor& seriesColor = CBCGPColor(), 
		BCGPChartCategory chartCategory = BCGPChartLine,
		BCGPChartType chartType = BCGP_CT_DEFAULT);

	void CommonInit();
	
	void EnableColorEachLine(BOOL bEnable, BOOL bRedraw = FALSE);
	virtual BOOL IsColorEachLineEnabled() const {return m_bColorEachLine;}

	virtual BOOL IsConnectFirstLastPoints() const {return m_bConnectFirstLastPoints;}
	virtual BOOL IsFillClosedShape() const {return m_bFillClosedShape;}

	void CloseShape(BOOL bClose, BOOL bFill)
	{
		m_bConnectFirstLastPoints = bClose;
		m_bFillClosedShape = bFill;
	}

protected:
	BOOL	m_bColorEachLine;
	BOOL	m_bConnectFirstLastPoints;
	BOOL	m_bFillClosedShape;

protected:
	virtual CBCGPBrush::BCGP_GRADIENT_TYPE GetSeriesFillGradientType() const 
	{
		if (m_chartCategory == BCGPChartLine) 
		{
			return CBCGPBrush::BCGP_NO_GRADIENT;
		}

		return CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT;
	}
};

class BCGCBPRODLLEXPORT CBCGPChartAreaSeries : public CBCGPChartSeries
{
	DECLARE_DYNCREATE(CBCGPChartAreaSeries)

	CBCGPChartAreaSeries();

public:
	CBCGPChartAreaSeries(CBCGPChartVisualObject* pChartCtrl,  
		BCGPChartCategory chartCategory = BCGPChartArea,
		BCGPChartType chartType = BCGP_CT_DEFAULT);

	CBCGPChartAreaSeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
		const CBCGPColor& seriesColor = CBCGPColor(), 
		BCGPChartCategory chartCategory = BCGPChartArea,
		BCGPChartType chartType = BCGP_CT_DEFAULT);

	void CommonInit();

	virtual void ClearMinMaxValues();
	virtual void RecalcMinMaxValues();

	void SetAreaOrigin(double dblValue, BOOL bRedraw = FALSE);
	double GetAreaOrigin() const {return m_valOrigin;}

	virtual int AddDataPoint(CBCGPChartDataPoint* pDataPoint);
	virtual int AddDataPoint(const CBCGPChartDataPoint& srcDataPoint);
	virtual int AddDataPoint(const CString& strCategoryName, double dblY, BCGPChartFormatSeries* pDataPointFormat = NULL, DWORD_PTR dwUserData = 0);
	virtual int AddDataPoint(double dblY, BCGPChartFormatSeries* pDataPointFormat = NULL, DWORD_PTR dwUserData = 0);
	virtual int AddDataPoint(double dblY, double dblX, BCGPChartFormatSeries* pDataPointFormat = NULL, DWORD_PTR dwUserData = 0);

	virtual BOOL SetDataPointValue(int nDataPointIndex, double dblValue, CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y);

	virtual int GetDefaultShadowAngle() const
	{
		return 45;
	}

protected:
	CBCGPChartValue	m_valOrigin;

protected:
	virtual CBCGPBrush::BCGP_GRADIENT_TYPE GetSeriesFillGradientType() const {return CBCGPBrush::BCGP_NO_GRADIENT;}
};


class BCGCBPRODLLEXPORT CBCGPChartBarSeries : public CBCGPChartSeries
{
	DECLARE_DYNCREATE(CBCGPChartBarSeries)

	CBCGPChartBarSeries();

public:
	CBCGPChartBarSeries(CBCGPChartVisualObject* pChartCtrl, BCGPChartCategory chartCategory = BCGPChartColumn, 
		BCGPChartType chartType = BCGP_CT_DEFAULT);

	CBCGPChartBarSeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
		const CBCGPColor& seriesColor, BCGPChartCategory chartCategory = BCGPChartColumn,
		BCGPChartType chartType = BCGP_CT_DEFAULT);

	void CommonInit();

	BOOL IsCustomSize() const {return m_bCustomSize;}
	int GetCustomSizePercent() const {return m_nCustomSizePercent;}
	void SetCustomSizePercent(int nSize, BOOL bSet = TRUE)
	{
		m_bCustomSize = bSet;
		if (bSet)
		{
			m_nCustomSizePercent = nSize;
		}
	}
	

	BOOL IsCustomOffset() const {return m_bCustomOffset;}
	int GetCustomOffsetPercent() const {return m_nCustomOffsetPercent;}
	void SetCustomOffsetPercent(int nOffset, BOOL bSet = TRUE)
	{
		m_bCustomOffset = bSet;
		if (bSet)
		{
			m_nCustomOffsetPercent = nOffset;
		}
	}

	virtual void CalcNumberOfSeriesOnAxis();
	int GetSeriesCountOnAxis() const {return m_nSeriesCountOnAxis;}
	

	int GetColumnOverlapPercent() const;
	int GetColumnDistancePercent() const;

	void SetColumnDistancePercent(int nPercent = 150);
	void SetColumnOverlapPercent(int nPercent = 0);

	virtual void SetRelatedAxis(CBCGPChartAxis* pAxis, CBCGPChartSeries::AxisIndex axisIndex);
	virtual void CopyFrom(const CBCGPChartSeries& src);

	virtual BCGPChartDataLabelOptions::LabelContent GetDefaultLegendContent() const {return BCGPChartDataLabelOptions::LC_CATEGORY_NAME;}

	virtual int GetDefaultShadowAngle() const;

protected:
	BOOL		m_bCustomSize;
	int			m_nCustomSizePercent; // size in percents from X Axis major unit width

	BOOL		m_bCustomOffset;
	int			m_nCustomOffsetPercent; // offset in percents from center

	int			m_nSeriesCountOnAxis;

protected:
	virtual CBCGPBrush::BCGP_GRADIENT_TYPE GetSeriesFillGradientType() const;
};

class BCGCBPRODLLEXPORT CBCGPChartHistogramSeries : public CBCGPChartSeries
{
	DECLARE_DYNCREATE(CBCGPChartHistogramSeries);
	CBCGPChartHistogramSeries();

public:
	CBCGPChartHistogramSeries(CBCGPChartVisualObject* pChartCtrl, BCGPChartType chartType = BCGP_CT_DEFAULT);

	CBCGPChartHistogramSeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
		const CBCGPColor& seriesColor = CBCGPColor(), BCGPChartType chartType = BCGP_CT_DEFAULT);

	void CommonInit();

	virtual void ClearMinMaxValues();

	void SetOrigin(double dblValue, BOOL bRedraw = FALSE);
	const CBCGPChartValue& GetOrigin() const {return m_valOrigin;}

	virtual BOOL IsDisplayShadow() const { return FALSE; }

protected:
	CBCGPChartValue	m_valOrigin;
};

class BCGCBPRODLLEXPORT CBCGPChartSurfaceSeries : public CBCGPChartSeries
{
	friend class CBCGPSurfaceChart3DImpl;

	DECLARE_DYNCREATE(CBCGPChartSurfaceSeries)

protected:
	CBCGPChartSurfaceSeries()
	{
		CommonInit();
	}

public:
	enum SurfaceType
	{
		ST_STANDARD,
		ST_LEVELS,
	};

	enum ColorMode
	{
		CM_SINGLE,
		CM_MULTIPLE,
		CM_PALETTE,
		CM_CUSTOM
	};

	enum FrameStyle
	{
		FS_NONE,
		FS_MESH,
		FS_CONTOUR,
		FS_CONTOUR_MESH
	};

	enum LevelRangeMode
	{
		LRM_MINMAX_SERIES,
		LRM_MINMAX_Y_AXIS,
		LRM_CUSTOM
	};

	CBCGPChartSurfaceSeries(CBCGPChartVisualObject* pChartCtrl);

	CBCGPChartSurfaceSeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
		const CBCGPColor& seriesColor = CBCGPColor());

	virtual ~CBCGPChartSurfaceSeries();

	void CommonInit();

	void SetSurfaceDimension(int nXSize);
	int  GetSurfaceDimension() const {return m_nXSize;}
	virtual void UpdateSeriesColors();

	void SetColorMapCount(int nCount = 16);
	int  GetColorMapCount() const {return m_nColorMapCount;}

	void SetColorMode(ColorMode colorMode);
	ColorMode GetColorMode() const	{ return m_colorMode; }
	
	void SetSurfaceType(SurfaceType type);
	SurfaceType GetSurfaceType() const {return m_surfaceType;}

	void SetFrameStyle(FrameStyle style);
	FrameStyle GetFrameStyle() const {return m_frameStyle;}

	void SetFrameColor(const CBCGPBrush& br);
	const CBCGPBrush& GetFrameColor() const {return m_brFrame;}

	void SetWireFrame(BOOL bSet);
	BOOL IsWireFrame() const {return m_bWireFrame;}

	void SetSurfaceOpacity(double dblOpacity);
	double GetSurfaceOpacity() const {return m_dblSurfaceOpacity;}

	void EnableFrameTransparency(BOOL bEnable = TRUE);
	BOOL IsFrameTransparencyEnabled() const {return m_bEnableFrameTransparency;}

	void SetLevelRangeMode(LevelRangeMode rangeMode, double dblMinCustomRangeValue = 0., double dblMaxCustomRangeValue = 0.);
	void GetLevelRange(double& dblMinValue, double& dblMaxValue);

	void SetDrawFlat(BOOL bSet = FALSE, double dblFlatLevel = -0.9);
	BOOL IsDrawFlat() const {return m_bDrawFlat;}
	double GetFlatLevel() const {return m_dblFlatLevel;}

	virtual const BCGPChartFormatSeries* GetColors(BCGPSeriesColorsPtr& seriesColors, int nDataPointIndex);
	virtual void OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);

	virtual int AddDataPoint(CBCGPChartDataPoint* pDataPoint);
	virtual int AddDataPoint(const CBCGPChartDataPoint& srcDataPoint);
	virtual int AddDataPoint(double dblY, double dblX, BCGPChartFormatSeries* pDataPointFormat = NULL, DWORD_PTR dwUserData = 0);
	
	virtual void AddDataPointsOptimized(const CBCGPDoubleArray& arYValues, CBCGPDoubleArray* pXValues = NULL, 
		CBCGPDoubleArray* pY1Values = NULL, BOOL bRecalcMinMaxValues = FALSE)
	{
		AddDataPoints(arYValues, pXValues, pY1Values, bRecalcMinMaxValues);
	}

	int GetLevelCount() const {return (int)m_arFillBrushes.GetSize() - 1;}

	virtual void LevelIndexToString(int nLevelIndex, CString& strValues);
	BOOL IsLevelRangeInLegendLabelEnabled() const {return m_bLevelRangleInLabel;}
	void EnableLevelRangeInLegendLabel(BOOL bEnable = TRUE);

	CBCGPBrush* GetLevelFillBrush(int nIndex);
	CBCGPBrush* GetLevelLineBrush(int nIndex);
	CBCGPBrush* GetLevelFillBackBrush(int nIndex);
	CBCGPBrush* GetLevelLineBackBrush(int nIndex);

	void InitLegendElements();
	void AddLegendElement(const CBCGPRect& rectKey, const CBCGPRect& rectLabel, const CString& strLabel);
	void GetLegendElementRects(int nIndex, CBCGPRect& rectKey, CBCGPRect& rectLabel);
	void SetLegendElementRects(int nIndex, const CBCGPRect& rectKey, const CBCGPRect& rectLabel);
	virtual void GetLegendElementLabel(int nIndex, CString& strLabel);

	const CBCGPChartSurfaceTriangleArray& GetSurfaceTriangles() const {return m_arSurfaceTriangles;}

	int GetLegendElementCount() const;

	void SetContinuousLegendKey(BOOL bSet = TRUE);
	BOOL IsContinuousLegendKey() const {return m_bContinuousLegendKey;}

public:
	CString m_strLegendLevelValueFormat;
	CArray<CBCGPColor, const CBCGPColor&> m_arCustomSurfaceColors;

protected:
	void CleanBrushes();
	void ClearSurfaceCells();
	int GetBrushIndex(int nDataPointIndex);
	void GenerateSurfaceColors(const CArray<CBCGPColor, const CBCGPColor&>& arColors, double dblOpacity, BOOL bFill);

	void CreateSurfaceTriangles(double dblLevelStep);
	virtual void UpdateSurfaceTriangleColors();

	void ProcessTriangle(CBCGPChartSurfaceTriangle& triangle, double dblLevelStep, 
		CBCGPChartDiagram3D* pDiagram3D, double dblDepthScalePercent, const CBCGPPoint& ptDiagramCenter,
		double dblFlatLevel);

	virtual BOOL IsDisplayShadow() const { return FALSE; }

protected:
	int			m_nXSize;
	int			m_nColorMapCount;
	ColorMode	m_colorMode;
	BOOL		m_bLevelRangleInLabel;

	CBCGPChartSurfaceTriangleArray	m_arSurfaceTriangles;	

	SurfaceType m_surfaceType;
	FrameStyle	m_frameStyle;
	CBCGPBrush	m_brFrame;
	BOOL		m_bWireFrame;

	double		m_dblFlatLevel;
	BOOL		m_bDrawFlat;

	double		m_dblSurfaceOpacity;
	BOOL		m_bEnableFrameTransparency;

	LevelRangeMode	m_levelRangeMode;
	double			m_dblMinCustomLevelRangeVal;
	double			m_dblMaxCustomLevelRangeVal;

	CArray<double, double>				m_arLevelYValues;

	CArray<CBCGPBrush*, CBCGPBrush*>	m_arFillBrushes;
	CArray<CBCGPBrush*, CBCGPBrush*>	m_arLineBrushes;

	CArray<CBCGPBrush*, CBCGPBrush*>	m_arFillBackBrushes;
	CArray<CBCGPBrush*, CBCGPBrush*>	m_arLineBackBrushes;

	CArray<CBCGPRect, const CBCGPRect&>	m_arLegendKeyRects;
	CArray<CBCGPRect, const CBCGPRect&>	m_arLegendLabelRects;
	CStringArray						m_arLegendLabels;
	BOOL								m_bContinuousLegendKey;

private:
	CBCGPPoint m_arTmpPoints[10];
};

class BCGCBPRODLLEXPORT CBCGPChartLongSeries : public CBCGPChartSeries
{
	friend class CBCGPLongDataChartImpl;

	DECLARE_DYNCREATE(CBCGPChartLongSeries)
	CBCGPChartLongSeries()
	{
		m_bFilterSimilarXValues = TRUE;
		m_bIsScatterMode = FALSE;
		m_dblScatterPointSize = 0.0;
	}

public:
	CBCGPChartLongSeries(CBCGPChartVisualObject* pChartCtrl, int nDataCount = 100000, int nGrowBy = 10000);

	void SetScatterMode(BOOL bSet = TRUE, double dblSize = 2.0);
	BOOL IsScatterMode() const
	{
		return m_bIsScatterMode;
	}

	double GetScatterPointSize() const
	{
		return m_dblScatterPointSize;
	}

	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld);

	virtual int FindDataPointIndex(CBCGPChartDataPoint* pDataPoint)
	{
		ASSERT_VALID(this);

		UNREFERENCED_PARAMETER(pDataPoint);

		ASSERT(FALSE);
		TRACE0("CBCGPChartLongSeries::FindDataPointIndex is not implemented for long data series.");
		return -1;
	}

	virtual void SetSeriesData(double dblY, double dblX, int nIndex);
	virtual int AddDataPoint(CBCGPChartDataPoint* pDataPoint);
	virtual int AddDataPoint(const CBCGPChartDataPoint& srcDataPoint)
	{
		ASSERT_VALID(this);
		return AddDataPoint((CBCGPChartDataPoint*) &srcDataPoint);
	}

	virtual int AddDataPoint(const CString& strCategoryName, double dblY, BCGPChartFormatSeries* pDataPointFormat = NULL, DWORD_PTR dwUserData = 0);
	virtual int AddDataPoint(double dblY, BCGPChartFormatSeries* pDataPointFormat = NULL, DWORD_PTR dwUserData = 0);
	virtual void AddDataPoints(const CBCGPDoubleArray& arYValues, CBCGPDoubleArray* pXValues = NULL, CBCGPDoubleArray* pY1Values = NULL, BOOL bRecalcMinMaxValues = TRUE);

	virtual void AddDataPointsOptimized(const CBCGPDoubleArray& arYValues, CBCGPDoubleArray* pXValues = NULL, 
		CBCGPDoubleArray* pY1Values = NULL, BOOL bRecalcMinMaxValues = FALSE)
	{
		AddDataPoints(arYValues, pXValues, pY1Values, bRecalcMinMaxValues);
	}

	virtual int AddEmptyDataPoint(const CString& strCategoryName, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pDataPointFormat = NULL)
	{
		UNREFERENCED_PARAMETER(strCategoryName);
		UNREFERENCED_PARAMETER(dwUserData);
		UNREFERENCED_PARAMETER(pDataPointFormat);
		TRACE0("Empty data point support is not implemented for long data chart.\n");

		return -1;
	}
	virtual int AddEmptyDataPoint(DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pDataPointFormat = NULL)
	{
		UNREFERENCED_PARAMETER(dwUserData);
		UNREFERENCED_PARAMETER(pDataPointFormat);
		TRACE0("Empty data point support is not implemented for long data chart.\n");
		
		return -1;
	}
	virtual int AddEmptyDataPoint(double dblX, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pDataPointFormat = NULL)
	{
		UNREFERENCED_PARAMETER(dblX);
		UNREFERENCED_PARAMETER(dwUserData);
		UNREFERENCED_PARAMETER(pDataPointFormat);
		TRACE0("Empty data point support is not implemented for long data chart.\n");
		
		return -1;
	}


	virtual BOOL SetDataPointValue(int nDataPointIndex, double dblValue, CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y);
	virtual void RemoveDataPoints(int nStartIdx, int nCount = 1, BOOL bFreeExtra = TRUE);
	virtual void MoveDataPoints(int nFromIdx, int nToIdx, int nCount = 1);
	virtual CBCGPChartValue GetDataPointValue(int nDataPointIndex, CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y) const;
	virtual int GetDataPointCount() const {return m_nCurrIndex;}
	virtual const CBCGPChartDataPoint* GetDataPointAt(int nIndex) const;
	virtual void OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);
	virtual void OnGetDataPointLegendLabel(int nDataPointIndex, CString& strLabel)
	{
		UNREFERENCED_PARAMETER(nDataPointIndex);
		UNREFERENCED_PARAMETER(strLabel);
	}

	virtual const CBCGPPointsArray& GetScreenPoints() const {return m_arScreenPoints;}
	
	BOOL IsFilterSimilarXValues() const {return m_bFilterSimilarXValues;}
	void SetFilterSimilarXValues(BOOL bSet = TRUE) {m_bFilterSimilarXValues = bSet;}

	void SetDataPointCount(int nCount);

	virtual BOOL IsAutoColorDataPoints() const {return FALSE;}
	virtual void EnableAutoColorDataPoints(BOOL bEnable = TRUE, BOOL bRedraw = FALSE) 
	{
		UNREFERENCED_PARAMETER(bEnable);
		UNREFERENCED_PARAMETER(bRedraw);
	}

	virtual BOOL GetNearestScreenPoint(const CBCGPPoint& ptClient, CBCGPPoint& ptScreen,
												 double& dblX, double& dblY) const;

protected:
	virtual BOOL IsFixedSize() const {return FALSE;}
	virtual void UpdateAxisCategories(){}
	virtual BOOL IsDisplayShadow() const { return FALSE; }

protected:
	int						m_nCurrIndex;
	CArray<double, double>	m_arXValues;
	CArray<double, double>	m_arYValues;
	CBCGPPointsArray		m_arScreenPoints;

	BOOL					m_bFilterSimilarXValues;
	
	BOOL					m_bIsScatterMode;
	double					m_dblScatterPointSize;

	CBCGPChartDataPoint m_dpCurrent;
};

class BCGCBPRODLLEXPORT CBCGPChartHistoricalLineSeries : public CBCGPChartLongSeries
{
	DECLARE_DYNCREATE(CBCGPChartHistoricalLineSeries)
protected:
	CBCGPChartHistoricalLineSeries()
	{
		CommonInit();
	}

	void CommonInit();

public:
	CBCGPChartHistoricalLineSeries(CBCGPChartVisualObject* pRelatedChart, int nHistoryDepth, double dblDefaultY = 0.);

	virtual int AddDataPoint(const CString& strCategoryName, double dblY, BCGPChartFormatSeries* pDataPointFormat = NULL, DWORD_PTR dwUserData = 0);
	virtual int AddDataPoint(double dblY, BCGPChartFormatSeries* pDataPointFormat = NULL, DWORD_PTR dwUserData = 0);
	virtual int AddDataPoint(double dblY, double dblX, BCGPChartFormatSeries* pDataPointFormat = NULL, DWORD_PTR dwUserData = 0);
	virtual int AddDataPoint(CBCGPChartDataPoint* pDataPoint);
	virtual void AddDataPoints(const CBCGPDoubleArray& arYValues, CBCGPDoubleArray* pXValues = NULL, CBCGPDoubleArray* pY1Values = NULL, BOOL bRecalcMinMaxValues = TRUE);

	virtual void AddDataPointsOptimized(const CBCGPDoubleArray& arYValues, CBCGPDoubleArray* pXValues = NULL, 
		CBCGPDoubleArray* pY1Values = NULL, BOOL bRecalcMinMaxValues = FALSE)
	{
		AddDataPoints(arYValues, pXValues, pY1Values, bRecalcMinMaxValues);
	}

	virtual void SetSeriesData(double dblY, double dblX, int nIndex);
	virtual void SetSeriesData(double dblY, BOOL bRedraw = TRUE);

	virtual void SetRelatedAxes(CBCGPChartAxis* pXAxis, CBCGPChartAxis* pYAxis, CBCGPChartAxis* pZAxis = NULL);
	virtual void SetRelatedAxis(CBCGPChartAxis* pAxis, CBCGPChartSeries::AxisIndex axisIndex);

	virtual void RemoveAllDataPoints();

	void SetHistoryDepth(int nDepth, BOOL bInitY = FALSE, double dblDefaultY = 0);
	
	void SetMajorXUnit(double dblUnit = 1.)
	{
		m_dblMajorXUnit = dblUnit;
	}

	virtual double GetFixedMajorXUnit() const {return m_dblMajorXUnit;}

	virtual void EnableHistoryMode(BOOL bEnable, int nHistoryDepth, BOOL bReverseOrder, 
		BOOL bSetDefaultValue = FALSE, double dblDefaultYValue = 0.);

protected:
	virtual BOOL IsFixedSize() const {return TRUE;}
	virtual void UpdateAxisCategories(){}


protected:
	double  m_dblMajorXUnit;
	double	m_dblLastInitY;
	BOOL	m_bLastIsInitY;
};

class BCGCBPRODLLEXPORT CBCGPChartBubbleSeries : public CBCGPChartSeries
{
	DECLARE_DYNCREATE(CBCGPChartBubbleSeries)

	CBCGPChartBubbleSeries();
public:
	CBCGPChartBubbleSeries(CBCGPChartVisualObject* pChartCtrl);

	void CommonInit();

	virtual void AddDataPointsOptimized(const CBCGPDoubleArray& arYValues, CBCGPDoubleArray* pXValues = NULL, 
		CBCGPDoubleArray* pY1Values = NULL, BOOL bRecalcMinMaxValues = FALSE)
	{
		AddDataPoints(arYValues, pXValues, pY1Values, bRecalcMinMaxValues);
	}


	virtual void SetMinMaxValues(const CBCGPChartValue& val, CBCGPChartData::ComponentIndex ci, int nDataPointIndex);
	virtual void OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);

	void SetBubbleScale(double dblScale);
	double GetBubbleScale() const {return m_dblScale;}

	virtual BCGPChartDataLabelOptions::LabelContent GetDefaultLegendContent() const {return BCGPChartDataLabelOptions::LC_CATEGORY_NAME;}
	virtual BOOL IsIgnoreNegativeValues() const {return m_bIgnoreNegativeValues;}

	virtual BOOL CanIncludeDataPointToLegend(int nDataPointIndex);

protected:
	virtual CBCGPBrush::BCGP_GRADIENT_TYPE GetSeriesFillGradientType() const;
	virtual BOOL CanUseRangeForMinMax() const {return FALSE;}

protected:
	double m_dblScale;
};

class BCGCBPRODLLEXPORT CBCGPChartPieSeries : public CBCGPChartSeries
{
	DECLARE_DYNCREATE(CBCGPChartPieSeries)
	CBCGPChartPieSeries()
	{
		CommonInit();
	}

public:
	CBCGPChartPieSeries(CBCGPChartVisualObject* pChartCtrl, BCGPChartCategory category);
	CBCGPChartPieSeries(CBCGPChartVisualObject* pChartCtrl, 
		BCGPChartCategory category, const CString& strSeriesName);

	void CommonInit();

	virtual void AddDataPointsOptimized(const CBCGPDoubleArray& arYValues, CBCGPDoubleArray* pXValues = NULL, 
		CBCGPDoubleArray* pY1Values = NULL, BOOL bRecalcMinMaxValues = FALSE)
	{
		AddDataPoints(arYValues, pXValues, pY1Values, bRecalcMinMaxValues);
	}


	virtual void OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);

	void SetPieExplosion(double dblExplosion);
	virtual double GetPieExplosion() const {return m_dblPieExplosion;}

	void SetDataPointPieExplosion(double dblExplosion, int nDataPontIndex);
	virtual double GetDataPointPieExplosion(int nDataPointIndex) const;
	virtual CBCGPChartValue GetDataPointValue(int nDataPointIndex, CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y) const;

	void SetPieRotation(int nAngle = 0) {m_nPieRotation = nAngle;}
	int  GetPieRotation() const {return m_nPieRotation;}

	void SetHeightPercent(int nPercent);
	int GetHeightPercent() const {return m_nHeightPercent;}

	void SetPieAngle(double dblAngle);
	double GetPieAngle() const {return m_dblPieAngle;}

	void EnableFitDiagramArea(BOOL bFit = TRUE) {m_bFitDiagramArea = bFit;}
	BOOL IsFitDiagramAreaEnabled() const {return m_bFitDiagramArea;}

	virtual BOOL IsIgnoreNegativeValues() const {return m_bIgnoreNegativeValues;}
	virtual BOOL CanIncludeDataPointToLegend(int nDataPointIndex);

	virtual BCGPChartDataLabelOptions::LabelContent GetDefaultLegendContent() const {return BCGPChartDataLabelOptions::LC_CATEGORY_NAME;}
	virtual BOOL GetDataPointLabelText(int nDataPointIndex, BCGPChartDataLabelOptions::LabelContent content, CString& strDPLabel);
	
	virtual BOOL HitTestDataPoint(const CBCGPPoint& pt, BCGPChartHitInfo* pHitInfo);
	virtual BOOL OnGetDataPointTooltip(int nDataPointIndex, CString& strTooltip, CString& strTooltipDescr);
	virtual int GetDoughnutPercent() const {return 0;}

	virtual BOOL IsDisplayShadow() const { return FALSE; }

	void SetGroupSmallerSlices(BOOL bSet = TRUE, double dblMinPercent = 5.0, BOOL bGroupInGegend = FALSE, const CString& strLabel = _T(""));

protected:
	double	m_dblPieExplosion;
	int		m_nPieRotation;

	double  m_dblPieAngle;
	int		m_nHeightPercent;	

	BOOL	m_bFitDiagramArea;

	BOOL	m_bGroupSmallerSlices;
	BOOL	m_bGroupSmallerSlicesInLegend;
	double	m_dblMinGroupPercent;
	CString	m_strSmallerSlicesGroupLabel;
	int		m_nSmallGroupDataPointIndex;

protected:
	virtual void OnBeforeChangeType();
	CBCGPBrush::BCGP_GRADIENT_TYPE GetSeriesFillGradientType() const;
};

class BCGCBPRODLLEXPORT CBCGPChartDoughnutSeries : public CBCGPChartPieSeries
{
	DECLARE_DYNCREATE(CBCGPChartDoughnutSeries)

	CBCGPChartDoughnutSeries()
	{
		m_nDoughnutPercent = 50;
	}

public:
	CBCGPChartDoughnutSeries(CBCGPChartVisualObject* pChartCtrl, BCGPChartCategory category) :
		CBCGPChartPieSeries(pChartCtrl, category)
	{
		m_nDoughnutPercent = 50;
	}

	CBCGPChartDoughnutSeries(CBCGPChartVisualObject* pChartCtrl, BCGPChartCategory category, const CString& strSeriesName) :
		CBCGPChartPieSeries(pChartCtrl, category, strSeriesName)
	{
		m_nDoughnutPercent = 50;
	}

	void SetDoughnutPercent(int nPercent);
	virtual int GetDoughnutPercent() const {return m_nDoughnutPercent;}
	
protected:
	int	m_nDoughnutPercent;
};

class BCGCBPRODLLEXPORT CBCGPChartPyramidSeries : public CBCGPChartSeries
{
	DECLARE_DYNCREATE(CBCGPChartPyramidSeries)
	CBCGPChartPyramidSeries()
	{
		CommonInit();
	}

public:
	CBCGPChartPyramidSeries(CBCGPChartVisualObject* pChartCtrl, BCGPChartCategory category);
	CBCGPChartPyramidSeries(CBCGPChartVisualObject* pChartCtrl, 
		BCGPChartCategory category, const CString& strSeriesName);

	void CommonInit();

	virtual void AddDataPointsOptimized(const CBCGPDoubleArray& arYValues, CBCGPDoubleArray* pXValues = NULL, 
		CBCGPDoubleArray* pY1Values = NULL, BOOL bRecalcMinMaxValues = FALSE)
	{
		AddDataPoints(arYValues, pXValues, pY1Values, bRecalcMinMaxValues);
	}


	void SetGap(int nGap);
	int GetGap() const {return m_nGap;}

	void SetDepthPercent(int nDepthPercent = 20);
	int GetDepthPercent() const {return m_nDepthPercent;}

	void SetRotation(double dblAngle);
	double GetRotation() const {return m_dblRotation;}

	void SetCircularBase(BOOL bIsCircularBase = FALSE);
	BOOL IsCircularBase() const {return m_bIsCircularBase;}

	void SortDataPoints(int nScreenPointIndex, CArray<CBCGPChartDataPoint*, CBCGPChartDataPoint*>& arDPSorted);

	virtual void OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);
	virtual BCGPChartDataLabelOptions::LabelContent GetDefaultLegendContent() const {return BCGPChartDataLabelOptions::LC_CATEGORY_NAME;}

	virtual CBCGPChartValue GetDataPointValue(int nDataPointIndex, CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y) const;

	virtual BOOL IsIgnoreNegativeValues() const {return m_bIgnoreNegativeValues;}
	virtual BOOL CanIncludeDataPointToLegend(int nDataPointIndex);

public:
	BOOL	m_bDataLabelsInColumns;

protected:
	virtual void OnBeforeChangeType();
	CBCGPBrush::BCGP_GRADIENT_TYPE GetSeriesFillGradientType() const;

	BOOL CalcSeriesParams(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, double& dblSum, 
		int& nNonEmptyCount, CBCGPRect& rectShape);

protected:
	int m_nGap;
	int m_nDepthPercent;
	double m_dblRotation;
	BOOL m_bIsCircularBase;

	int	m_nAllowedGap;
};

class BCGCBPRODLLEXPORT CBCGPChartFunnelSeries : public CBCGPChartPyramidSeries
{
	DECLARE_DYNCREATE(CBCGPChartFunnelSeries)
	CBCGPChartFunnelSeries()
	{
		CommonInit();
	}
public:
	CBCGPChartFunnelSeries(CBCGPChartVisualObject* pChartCtrl, BCGPChartCategory category);
	CBCGPChartFunnelSeries(CBCGPChartVisualObject* pChartCtrl, 
		BCGPChartCategory category, const CString& strSeriesName);
	void CommonInit();

	virtual void OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);

	void SetNeckHeightInPercents(double dblHeight);
	void SetNeckHeightInChartValues(double dblValue);

	BOOL IsNeckHeightChartValue() const {return m_bNeckHeightIsValue;}
	double GetNeckHeight() const {return m_dblNeckHeight;}

	void SetNeckWidth(int nWidth);
	int GetNeckWidth() const {return m_nNeckWidth;}

	CBCGPSize GetNeckSizeInPixels() const {return m_szNeckSize;}

protected:
	double	m_dblNeckHeight;
	BOOL	m_bNeckHeightIsValue;

	int		m_nNeckWidth;

	CBCGPSize	m_szNeckSize; 
};

struct CBCGPChartStockData
{
	CBCGPChartStockData()
	{
		SetEmpty();
	}

	CBCGPChartStockData(const COleDateTime& tm)
	{
		SetEmpty();
		m_dateTime = tm;
	}

	CBCGPChartStockData(double dblOpen, double dblHigh, double dblLow, double dblClose, 
		COleDateTime dateTime)
	{
		m_dblOpen = dblOpen;
		m_dblHigh = dblHigh;
		m_dblLow = dblLow;
		m_dblClose = dblClose;
		m_dateTime = dateTime;
	}

	void SetEmpty()
	{
		m_dblOpen = 0.;
		m_dblHigh = 0.;
		m_dblLow = 0.;
		m_dblClose = 0.;
	}

	BOOL IsEmpty() const 
	{
		return m_dblOpen == 0 && m_dblHigh == 0 && m_dblLow == 0 && m_dblClose == 0;
	}

	double m_dblOpen;
	double m_dblHigh;
	double m_dblLow;
	double m_dblClose;
	COleDateTime m_dateTime;
};

typedef CBCGPChartValue (CALLBACK* BCGPCHART_STOCK_CALLBACK)(int nDataPointIndex, CBCGPChartStockSeries* pStockSeries);
typedef CArray<CBCGPChartStockData, CBCGPChartStockData&> CBCGPStockDataArray;

class BCGCBPRODLLEXPORT CBCGPBaseChartStockSeries : public CBCGPChartSeries
{
	friend class CBCGPChartStockSeries;
	DECLARE_DYNCREATE(CBCGPBaseChartStockSeries)
public:
	enum StockSeriesKind
	{
		SSK_UNDEFINED,
		SSK_OPEN,
		SSK_HIGH,
		SSK_LOW,
		SSK_CLOSE
	};

	enum StockSeriesType
	{
		SST_BAR,
		SST_CANDLE,
		SST_LINE_OPEN,
		SST_LINE_HIGH,
		SST_LINE_LOW,
		SST_LINE_CLOSE,
		SST_LINE_CUSTOM
	};

protected:
	CBCGPBaseChartStockSeries();
	CBCGPBaseChartStockSeries(CBCGPChartVisualObject* pChartCtrl, StockSeriesType seriesType);

public:
	BOOL IsMainStockSeries() const {return m_bIsMainStockSeries;}
	virtual void RecalcMinMaxValues() {return;}

	// virtual methods to override in custom virtual mode
	virtual BOOL IsOptimizedLongDataMode() const // return TRUE in custom virtual mode
	{
		if (m_bIsMainStockSeries)
		{
			return m_stockData.GetSize() > 0;
		}
	
		if (m_pParentSeries == NULL)
		{
			return FALSE;
		}

		return m_pParentSeries->IsOptimizedLongDataMode();
	} 
	
	virtual int GetDataPointCount() const; // return number of stock data points in your data
	virtual CBCGPChartStockData GetStockDataAt(int nDataPointIndex) const; // return stock data at specified data point

	// virtual mode
	virtual CBCGPChartValue GetDataPointValue(int nDataPointIndex, CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y) const;
	const CBCGPChartDataPoint* GetDataPointAt(int nIndex) const;

	virtual const CBCGPStockDataArray& GetStockData() const;

protected:
	BOOL										m_bIsMainStockSeries;
	CBCGPBaseChartStockSeries::StockSeriesType	m_type;
	CBCGPBaseChartStockSeries::StockSeriesKind	m_kind;
	CBCGPBaseChartStockSeries*					m_pParentSeries;

	CBCGPStockDataArray m_stockData;
};

class BCGCBPRODLLEXPORT CBCGPChartStockSeries : public CBCGPBaseChartStockSeries
{
	DECLARE_DYNCREATE(CBCGPChartStockSeries)
	CBCGPChartStockSeries()
	{
		m_kind = CBCGPBaseChartStockSeries::SSK_OPEN;
		CommonInit();
	}
public:
	CBCGPChartStockSeries(CBCGPChartVisualObject* pChartCtrl, CBCGPBaseChartStockSeries::StockSeriesType seriesType);
	void CommonInit();

	virtual ~CBCGPChartStockSeries();
	
	void SetStockSeriesType(CBCGPBaseChartStockSeries::StockSeriesType type, BOOL bRedraw);
	CBCGPBaseChartStockSeries::StockSeriesType GetStockSeriesType() const {return m_type;}

	virtual void SetTreatNulls(CBCGPChartSeries::TreatNulls tn, BOOL bRecalcMinMax = TRUE);

	virtual CBCGPChartStockData GetStockDataAt(int nDataPointIndex) const;

	void AddData(double dblOpen, double dblHigh, double dblLow, double dblClose, 
		const COleDateTime& dateTime = COleDateTime());
	void AddData(const CBCGPChartStockData& data);
	void AddData(const CBCGPStockDataArray& arStockData);

	virtual int AddEmptyDataPoint(const CString& strCategoryName, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pDataPointFormat = NULL);
	virtual int AddEmptyDataPoint(DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pDataPointFormat = NULL);
	virtual int AddEmptyDataPoint(double dblX, DWORD_PTR dwUserData = 0, BCGPChartFormatSeries* pDataPointFormat = NULL);

	CBCGPBaseChartStockSeries* GetChildSeries(int nIdx) const;
	int GetChildSeriesCount() const {return (int)m_arChildSeries.GetSize();}
	virtual void OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);
	virtual void OnCalcScreenPointsSimple(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);
	virtual CBCGPPoint GetDataPointScreenPoint(int nDataPointIndex, int nScreenPointIndex) const;
	virtual void SetRelatedAxes(CBCGPChartAxis* pXAxis, CBCGPChartAxis* pYAxis, CBCGPChartAxis* pZAxis = NULL);
	virtual void SetRelatedAxis(CBCGPChartAxis* pAxis, CBCGPChartSeries::AxisIndex axisIndex);
	virtual void RecalcMinMaxValues();
	virtual CBCGPChartValue GetMinValue(CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y);
	virtual CBCGPChartValue GetMaxValue(CBCGPChartData::ComponentIndex ci = CBCGPChartData::CI_Y);
	virtual void ClearMinMaxValues();

	void SetCustomStockValueCallback(BCGPCHART_STOCK_CALLBACK pfn, BOOL bRedraw = FALSE);
	BCGPCHART_STOCK_CALLBACK GetCustomStockValueCallback() const {return m_pfnCustomValueCallback;}	
	virtual CBCGPChartValue GetCustomStockValue(int nDataPointIndex) const 
	{
		if (m_pfnCustomValueCallback != NULL)
		{
			return (*m_pfnCustomValueCallback)(nDataPointIndex, (CBCGPChartStockSeries*)this);
		}
		return CBCGPChartValue();
	}


	virtual void SetIndexMode(BOOL bSet);
	virtual BOOL IsXComponentSet() const {return TRUE;}

	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld);
	
	virtual void EnableHistoryMode(BOOL bEnable, int nHistoryDepth, BOOL bReverseOrder, 
		BOOL bSetDefaultValue = FALSE, double dblDefaultYValue = 0.);

	virtual BOOL OnGetDataPointTooltip(int nDataPointIndex, CString& strTooltip, CString& strTooltipDescr);

	// percent from major unit interval
	// for candles - specifies the width of candle
	// for bars - specifies full width of open-close thick marks
	int				m_nOpenCloseBarSizePercent; 

	// special down candle style; up style is taken from the main series
	BCGPChartFormatArea m_downCandleStyle;
	// special down bar style; up style is taken from the main series
	BCGPChartFormatLine  m_downBarStyle;

	static const int CHART_STOCK_SERIES_HIGH_IDX;
	static const int CHART_STOCK_SERIES_LOW_IDX;
	static const int CHART_STOCK_SERIES_CLOSE_IDX;

	static const int STOCK_ARRAY_OPEN_IDX;
	static const int STOCK_ARRAY_HIGH_IDX;
	static const int STOCK_ARRAY_LOW_IDX;
	static const int STOCK_ARRAY_CLOSE_IDX;

	virtual void ResizeDataPointArray(int nNewSize);

protected:
	CArray<CBCGPBaseChartStockSeries*, CBCGPBaseChartStockSeries*> m_arChildSeries;
	BCGPCHART_STOCK_CALLBACK m_pfnCustomValueCallback;
};

class BCGCBPRODLLEXPORT CBCGPChartPolarSeries : public CBCGPChartLineSeries
{
	DECLARE_DYNCREATE(CBCGPChartPolarSeries)
	CBCGPChartPolarSeries()
	{
		CommonInit();
	}

public:
	CBCGPChartPolarSeries(CBCGPChartVisualObject* pChartCtrl);
	CBCGPChartPolarSeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
		const CBCGPColor& seriesColor = CBCGPColor());

	~CBCGPChartPolarSeries();
	void CommonInit();

	virtual void AddDataPointsOptimized(const CBCGPDoubleArray& arYValues, CBCGPDoubleArray* pXValues = NULL, 
		CBCGPDoubleArray* pY1Values = NULL, BOOL bRecalcMinMaxValues = FALSE)
	{
		AddDataPoints(arYValues, pXValues, pY1Values, bRecalcMinMaxValues);
	}


	virtual void ShowOnPrimaryAxis(BOOL bPrimary);
	virtual void OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);

	void MakeRose(int nPoints = 1, BOOL bFillArea = TRUE);
	int  GetRosePointCount() const {return m_nPoints;}

	virtual BOOL IsIndexMode() const {return FALSE;}

	virtual CBCGPPoint ScreenPointFromChartData(const CBCGPChartData& data, int nDataPointIndex);

protected:
	int		m_nPoints;
};

class BCGCBPRODLLEXPORT CBCGPChartTernarySeries : public CBCGPChartLineSeries
{
	DECLARE_DYNCREATE(CBCGPChartTernarySeries)
	CBCGPChartTernarySeries();

public:
	CBCGPChartTernarySeries(CBCGPChartVisualObject* pChartCtrl);
	CBCGPChartTernarySeries(CBCGPChartVisualObject* pChartCtrl, const CString& strSeriesName, 
		const CBCGPColor& seriesColor = CBCGPColor());

	~CBCGPChartTernarySeries();
	void CommonInit();

	void SetDisplayComponentPercentInTooltip(BOOL bSet = TRUE) {m_bShowPercentsInTooltip = bSet;}
	BOOL IsDisplayComponentPercentInTooltip() const {return m_bShowPercentsInTooltip;}

	virtual void ShowOnPrimaryAxis(BOOL bPrimary);

	virtual void OnCalcScreenPoints(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea);
	virtual CBCGPPoint ScreenPointFromChartData(const CBCGPChartData& data, int nDataPointIndex);

	virtual void SetMinMaxValues(const CBCGPChartValue& val, CBCGPChartData::ComponentIndex ci, int nDataPointIndex);
	virtual BCGPChartDataLabelOptions::LabelContent GetDefaultLegendContent() const 
	{
		return (BCGPChartDataLabelOptions::LabelContent)
			(BCGPChartDataLabelOptions::LC_CATEGORY_NAME | BCGPChartDataLabelOptions::LC_ALL_TERNARY_PERCENTAGE);
	}
	virtual BOOL GetDataPointLabelText(int nDataPointIndex, BCGPChartDataLabelOptions::LabelContent content, CString& strDPLabel);

	double GetDataPointComponentPercent(int nDataPointIndex, CBCGPChartData::ComponentIndex ci) const;

	CBCGPChartValue GetMinPercentage(CBCGPChartData::ComponentIndex ci) const;
	CBCGPChartValue GetMaxPercentage(CBCGPChartData::ComponentIndex ci) const;

	void SetMinMaxPercentage(double dblValue, CBCGPChartData::ComponentIndex ci);

	virtual BOOL OnGetDataPointTooltip(int nDataPointIndex, CString& strTooltip, CString& strTooltipDescr);

protected:
	CBCGPChartData					m_minPercentage;
	CBCGPChartData					m_maxPercentage;
	BOOL							m_bShowPercentsInTooltip;
};

#endif // !defined(AFX_BCGPCHARTSERIES_H__E7B32F7D_06C9_42FC_881D_543E11E643F1__INCLUDED_)
