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

#if !defined(AFX_BCGPCHARTIMPL_H__14AAD790_6C8D_4160_A013_1C12C567C329__INCLUDED_)
#define AFX_BCGPCHARTIMPL_H__14AAD790_6C8D_4160_A013_1C12C567C329__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPChartFormat.h"
#include <float.h>
#include "BCGPMath.h"
#include "BCGPChartLegend.h"

class CBCGPChartDataPoint;
class CBCGPChartSeries;
class CBCGPChartStockSeries;
class CBCGPChartVisualObject;
class CBCGPChartPyramidSeries;
class CBCGPChartFunnelSeries;
class CBCGPChartSurfaceSeries;

class CBCGPChartSide3D;
class CBCGPChartShape3D;

class CBCGPEngine3D;

typedef enum BCGPChartCategory
{
	BCGPChartDefault,
	BCGPChartLine,
	BCGPChartPie,
	BCGPChartPie3D,
	BCGPChartPyramid,
	BCGPChartPyramid3D,
	BCGPChartFunnel,
	BCGPChartFunnel3D,
	BCGPChartColumn,
	BCGPChartBar,
	BCGPChartHistogram,
	BCGPChartArea,
	BCGPChartStock,
	BCGPChartBubble,
	BCGPChartLongData,
	BCGPChartHistoricalLine,
	BCGPChartPolar,
	BCGPChartDoughnut,
	BCGPChartDoughnut3D,
	BCGPChartTorus3D,
	BCGPChartTernary,
	BCGPChartColumn3D,
	BCGPChartBar3D,
	BCGPChartLine3D,
	BCGPChartArea3D,
	BCGPChartSurface3D
}
BCGPChartCategory;

typedef enum BCGPChartType
{
	BCGP_CT_DEFAULT,
	BCGP_CT_SIMPLE,
	BCGP_CT_STACKED,
	BCGP_CT_100STACKED,
	BCGP_CT_RANGE
}BCGPChartType;

struct BCGCBPRODLLEXPORT AFX_NOVTABLE CBCGPChartSurfaceLevelAttribs
{
	CBCGPChartSurfaceLevelAttribs()
	{
		m_pBrFill = NULL;
		m_pBrLine = NULL;
		m_pBrFillBack = NULL;
		m_pBrLineBack = NULL;
		m_nLevelIndex = 0;
	}

	int					m_nLevelIndex;
	CBCGPBrush*			m_pBrFill;
	CBCGPBrush*			m_pBrLine;

	CBCGPBrush*			m_pBrFillBack;
	CBCGPBrush*			m_pBrLineBack;
};



struct BCGCBPRODLLEXPORT AFX_NOVTABLE CBCGPChartSurfaceLevel
{
	CBCGPChartSurfaceLevel()
	{
		m_vNormal.SetOpenGLDefaultNormal();
	}

	CBCGPPointsArray	m_arLevelPoints;
	CBCGPPointsArray	m_arIntersect;

	CBCGPChartSurfaceLevelAttribs		m_levelAttribs;
	CBCGPVector4						m_vNormal;
};

typedef CArray<CBCGPChartSurfaceLevel, const CBCGPChartSurfaceLevel&> CBCGPChartSurfaceLevelArray;

struct BCGCBPRODLLEXPORT AFX_NOVTABLE CBCGPChartSurfaceTriangle
{
	CBCGPChartSurfaceTriangle()
	{
		m_bBack = FALSE;
		m_bDrawn = FALSE;
		m_nDataPointIndex = -1;
		m_nTriangleIndex = -1;
		m_bLeft = TRUE;

		m_bIsEmpty = FALSE;

		m_vNormal.SetOpenGLDefaultNormal();

		memset(m_dblACoef, 0, sizeof(double) * 3);
		memset(m_dblBCoef, 0, sizeof(double) * 3);
		memset(m_bVertical, 0, sizeof(BYTE) * 3);
	}

	_inline BOOL IsLevelEmpty () const {return m_arLevels.GetSize() == 0;}

	_inline double CalcZ(const CBCGPPoint& pt) const
	{
		return -(m_vPlane[0] * pt.x + m_vPlane[1] * pt.y + m_vPlane[3]) / m_vPlane[2];
	}

	void CalcEdgeCoefs()
	{
		if (m_bIsEmpty)
		{
			return;
		}

		for (int i = 0; i < 3; i++)
		{
			const CBCGPPoint& pt1 = m_arPointsTransformed[i];
			const CBCGPPoint& pt2 = m_arPointsTransformed[(i + 1) % 3];

			m_bVertical[i] = fabs(pt2.x - pt1.x) < FLT_EPSILON;

			if (m_bVertical[i])
			{
				m_dblACoef[i] = 0.0;
				m_dblBCoef[i] = pt2.x;

				continue;
			}

			m_dblACoef[i] = (pt2.y - pt1.y) / (pt2.x - pt1.x);
			m_dblBCoef[i] = pt2.y - m_dblACoef[i] * pt2.x;
		}
	}

	BOOL CheckPointsOnEdge(const CBCGPPoint& pt1, const CBCGPPoint& pt2, int nEdgeIndex, double dblPrecision)
	{
		if (m_bIsEmpty || nEdgeIndex > 2)
		{
			return FALSE;
		}

		if (m_bVertical[nEdgeIndex])
		{
			if (fabs (pt1.x - m_dblBCoef[nEdgeIndex]) > dblPrecision)
			{
				return FALSE;
			}

			if (fabs (pt2.x - m_dblBCoef[nEdgeIndex]) > dblPrecision)
			{
				return FALSE;
			}
		}
		else
		{
			if (fabs (pt1.y - (m_dblACoef[nEdgeIndex] * pt1.x + m_dblBCoef[nEdgeIndex])) > dblPrecision)
			{
				return FALSE;
			}

			if (fabs (pt2.y - (m_dblACoef[nEdgeIndex] * pt2.x + m_dblBCoef[nEdgeIndex])) > dblPrecision)
			{
				return FALSE;
			}
		}

		return TRUE;
	}

	CBCGPPoint						m_arPoints[3];
	CBCGPPoint						m_arPointsRotated[3];
	CBCGPPoint						m_arPointsTransformed[3];

	CBCGPRect						m_rectBoundsRotated;
	double							m_dblMinZRotated;
	double							m_dblMaxZRotated;

	CBCGPChartSurfaceLevelArray		m_arLevels;
	CBCGPVector4					m_vPlane;
	CBCGPVector4					m_vNormal;
	BOOL							m_bBack;
	BOOL							m_bDrawn;
	BOOL							m_bLeft;

	BOOL							m_bIsEmpty;

	int								m_nDataPointIndex; // index of bottom left data point
	int								m_nTriangleIndex;
	
	// coefficients of transformed triangle edges
	double							m_dblACoef[3];
	double							m_dblBCoef[3];
	BYTE							m_bVertical[3];
};

typedef CArray<CBCGPChartSurfaceTriangle, const CBCGPChartSurfaceTriangle&> CBCGPChartSurfaceTriangleArray;

class BCGCBPRODLLEXPORT CBCGPBaseChartImpl : public CObject
{
	DECLARE_DYNAMIC(CBCGPBaseChartImpl)

public:
	enum AxisType
	{
		AT_NO_AXIS,
		AT_XY,
		AT_RADIAL
	};

protected:
	CBCGPChartVisualObject*	m_pRelatedChart;

	CBCGPBaseChartImpl()
	{
		m_pRelatedChart = NULL;
	}
	CBCGPBaseChartImpl(CBCGPChartVisualObject* pRelatedChartControl)
	{
		SetRelatedChartControl(pRelatedChartControl);
	}

public:
	virtual ~CBCGPBaseChartImpl()
	{
		m_pRelatedChart = NULL;
	}

	void SetRelatedChartControl(CBCGPChartVisualObject* pRelatedChartControl)
	{
		m_pRelatedChart = pRelatedChartControl;	
	}

	virtual AxisType GetAxisType() const = 0;

	virtual CBCGPSize OnCalcLegendKeySize(CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual CBCGPSize OnCalcLegendKeySizeEx(const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);

	virtual void OnCalcScreenPositions(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnCalcDataPointLabelRect(CBCGPGraphicsManager* pGM, CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);

	virtual CBCGPSize OnGetDataLabelSize(CBCGPGraphicsManager* pGM, const CString& strDPLabel, const CBCGPChartDataPoint* pDataPoint, CBCGPChartSeries* pSeries, int nDataPointIndex);

	virtual void OnDrawDiagramDataLabel(CBCGPGraphicsManager* pGM, const CString& strDataLabel, CBCGPChartSeries* pSeries, int nDataPointIndex, 
		const CBCGPChartDataPoint* pDataPoint, const BCGPChartFormatSeries* pFormatSeries);
	virtual void OnDrawDiagramMarkers(CBCGPGraphicsManager* pGM, CBCGPChartSeries* pSeries, const CBCGPRect& rectDiagram);
	virtual void OnDrawDiagramMarker(CBCGPGraphicsManager* pGM, CBCGPChartSeries* pSeries, int nDataPointIndex, 
		const CBCGPChartDataPoint* pDataPoint, const BCGPChartFormatSeries* pFormatSeries, const CBCGPRect& rectDiagram);

	virtual void OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex);

	virtual void OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
		const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);


	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries) = 0;
	virtual void OnDrawDiagramDataLabels(CBCGPGraphicsManager* pGM, CBCGPChartSeries* pSeries);
	virtual void OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual CBCGPPoint OnGetMarkerPoint(const CBCGPChartDataPoint* pDataPoint, CBCGPChartSeries* pSeries, int nDataPointIndex);

	const CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& GetVisibleSides() const {return m_lstVisibleSides;}

protected:
	BOOL OnPrepareDataToCalcDataPointLabelRect(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram, 
		CBCGPChartDataPoint* pDataPoint, CBCGPChartSeries* pSeries, int nDataPointIndex, 
		BCGPChartFormatSeries** ppFormatSeries, CBCGPSize& szDataLabel, CBCGPPoint& ptMarkerPos, CBCGPRect& rectBounds);

	void AlignRectToArea(const CBCGPRect& rectArea,  CBCGPRect& rectAlign, BOOL bForceVert = FALSE, BOOL bForceHorz = FALSE);
	BOOL CalcDataPointLabelDropLine(const CBCGPRect& rectDataLabel, const CBCGPPoint& ptDropEnd, CBCGPPoint& ptStart,
		CBCGPPoint& ptLabelUnderline, BOOL bCalcUnderlinePosition, BOOL bDataLabelHasBorder, BOOL bDropLineToMarker);
	void SetDataPointLabelRectAndDropLine(CBCGPChartSeries* pSeries, int nDataPointIndex, BCGPChartFormatSeries* pFormatSeries,
		CBCGPRect& rectLabel, const CBCGPPoint& ptDropPoint);

	virtual void OnDrawDiagram3D();
	virtual void CollectSides3D(CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lstInvisibleSides, CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lstVisibleSides);
	virtual void ArrangeSides3D(CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lstSides);

	virtual double CalcDepth3D(double dblUnitSize);

	virtual BOOL ForceAddBackfaces() const {return FALSE;}
	// helper that builds intervals of non empty point indexes
	void CalcIndexes(CBCGPChartSeries* pSeries, CArray<int, int>& arStartIndexes, CArray<int, int>& arEndIndexes);

	CList<CBCGPChartSide3D*, CBCGPChartSide3D*> m_lstInvisibleSides;
	CList<CBCGPChartSide3D*, CBCGPChartSide3D*> m_lstVisibleSides;
};

class BCGCBPRODLLEXPORT CBCGPLineChartImpl : public CBCGPBaseChartImpl
{
	DECLARE_DYNCREATE(CBCGPLineChartImpl)
public:
	CBCGPLineChartImpl()
	{
	}
	CBCGPLineChartImpl(CBCGPChartVisualObject* pRelatedChartControl) : CBCGPBaseChartImpl(pRelatedChartControl)
	{
	}

	virtual AxisType GetAxisType() const
	{
		return AT_XY;
	}

	virtual CBCGPSize OnCalcLegendKeySize(CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual CBCGPSize OnCalcLegendKeySizeEx(const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);

	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnDrawDiagramLine(CBCGPGraphicsManager* pGM, CBCGPChartSeries* pSeries, int nStartDataPointIndex, int nEndDataPointIndex,
		const CBCGPChartDataPoint* pDataPointStart, const CBCGPChartDataPoint* pDataPointEnd,
		BCGPChartFormatSeries::ChartCurveType curveType);

	virtual void OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
		const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);


protected:
	void OnGetStepPoint(CBCGPPoint ptBottomStart, CBCGPPoint ptBottomEnd, BOOL bXAxisVert, 
		BCGPChartFormatSeries::ChartCurveType curveType, CBCGPPoint& ptStep);

	CBCGPPointsArray m_linePoints;

};

class BCGCBPRODLLEXPORT CBCGPLineChart3DImpl : public CBCGPLineChartImpl
{
	DECLARE_DYNCREATE(CBCGPLineChart3DImpl)

public:
	CBCGPLineChart3DImpl()
	{
	}

	CBCGPLineChart3DImpl(CBCGPChartVisualObject* pRelatedChartControl) : CBCGPLineChartImpl(pRelatedChartControl)
	{
	}

	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);
};

class BCGCBPRODLLEXPORT CBCGPAreaChartImpl : public CBCGPBaseChartImpl
{
	DECLARE_DYNCREATE(CBCGPAreaChartImpl)
public:
	CBCGPAreaChartImpl()
	{
	}
	CBCGPAreaChartImpl(CBCGPChartVisualObject* pRelatedChartControl) : CBCGPBaseChartImpl(pRelatedChartControl)
	{
	}

	virtual AxisType GetAxisType() const
	{
		return AT_XY;
	}

	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);

	virtual CBCGPSize OnCalcLegendKeySize(CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual CBCGPSize OnCalcLegendKeySizeEx(const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);
	virtual void OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
		const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);
};

class BCGCBPRODLLEXPORT CBCGPAreaChart3DImpl : public CBCGPAreaChartImpl
{
	DECLARE_DYNCREATE(CBCGPAreaChart3DImpl)

protected:
	CBCGPAreaChart3DImpl()
	{
		m_dblPrevIntersectX = DBL_MAX;
		m_nFirstNonEmptyIndex = 0;
		m_nBeforeLastNonEmptyIndex = 0;
		m_nLastNonEmptyIndex = 0;
	}

public:
	
	CBCGPAreaChart3DImpl(CBCGPChartVisualObject* pRelatedChartControl) : CBCGPAreaChartImpl(pRelatedChartControl)
	{
		m_nFirstNonEmptyIndex = 0;
		m_nBeforeLastNonEmptyIndex = 0;
		m_nLastNonEmptyIndex = 0;
		
		m_dblPrevIntersectX = DBL_MAX;
	}

	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnCalcScreenPositions(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);

protected:
	double m_dblPrevIntersectX;
	int m_nFirstNonEmptyIndex;
	int m_nBeforeLastNonEmptyIndex;
	int m_nLastNonEmptyIndex;

	virtual BOOL ForceAddBackfaces() const {return TRUE;}
};

struct BCGCBPRODLLEXPORT CBCGPChartSurfaceTriangleID
{
	CBCGPChartSurfaceTriangleID()
	{
		m_nTriangleIndex = -1;
		m_pSurfaceSeries = NULL;
	}

	int							m_nTriangleIndex;
	CBCGPChartSurfaceSeries*	m_pSurfaceSeries;

	CArray<int, int> m_arIDIndexesBefore;
};

typedef CArray<CBCGPChartSurfaceTriangleID, const CBCGPChartSurfaceTriangleID&> CBCGPChartSurfaceTriangleIDArray;

class BCGCBPRODLLEXPORT CBCGPSurfaceChart3DImpl : public CBCGPBaseChartImpl
{
	DECLARE_DYNCREATE(CBCGPSurfaceChart3DImpl)

protected:
	CBCGPSurfaceChart3DImpl()
	{
	}

public:
	CBCGPSurfaceChart3DImpl(CBCGPChartVisualObject* pRelatedChartControl) : CBCGPBaseChartImpl(pRelatedChartControl)
	{
		m_dblLastXRotation = -1000;
		m_dblLastYRotation = -1000;
		m_dblXAxisUnitSize = 0;
	}
	
	virtual AxisType GetAxisType() const
	{
		return AT_XY;
	}
	virtual void OnCalcScreenPositions(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);

	// nDataPointIndex in this case represents level
	virtual void OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex);

protected:
	void SortSurfaceTriangles(BOOL bUseTransparency, int nTriangleCount);
	virtual void OnDrawSurfaceTriangle(CBCGPEngine3D* pGM, CBCGPChartSurfaceTriangleID& triangle, BOOL bIsGDI, BOOL bFill);
	virtual void CollectSides3D(CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lstInvisibleSides, CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lstVisibleSides);
	virtual BOOL ForceAddBackfaces() const {return TRUE;}

protected:
	double					m_dblLastXRotation;
	double					m_dblLastYRotation;
	double					m_dblXAxisUnitSize;
	
	CBCGPChartSurfaceTriangleIDArray	m_arTriangleID;
	
};

class BCGCBPRODLLEXPORT CBCGPBarChartImpl : public CBCGPBaseChartImpl
{
	DECLARE_DYNCREATE(CBCGPBarChartImpl)
public:
	CBCGPBarChartImpl()
	{
	}
	CBCGPBarChartImpl(CBCGPChartVisualObject* pRelatedChartControl) : CBCGPBaseChartImpl(pRelatedChartControl)
	{
	}

	virtual AxisType GetAxisType() const
	{
		return AT_XY;
	}

	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
		const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);

	virtual void OnCalcDataPointLabelRect(CBCGPGraphicsManager* pGM, CBCGPChartDataPoint* pDataPoint, const CBCGPRect& /*rectDiagramArea*/, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual CBCGPPoint OnGetMarkerPoint(const CBCGPChartDataPoint* pDataPoint, CBCGPChartSeries* pSeries, int nDataPointIndex);

protected:
	virtual CBCGPRect BoundingRectFromGapAndBarWidth(CBCGPPoint ptBarScreenPoint, int nGapPercent, 
		double dblBarWidth,	int nSeriesIndex, int nVisibleSeriesCount, double dblZeroLinePos, 
		BOOL bIsVertical);

};

class BCGCBPRODLLEXPORT CBCGPBarChart3DImpl : public CBCGPBarChartImpl
{
	DECLARE_DYNCREATE(CBCGPBarChart3DImpl)
public:
	CBCGPBarChart3DImpl()
	{
	}
	CBCGPBarChart3DImpl(CBCGPChartVisualObject* pRelatedChartControl) : CBCGPBarChartImpl(pRelatedChartControl)
	{
	}

	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual CBCGPPoint OnGetMarkerPoint(const CBCGPChartDataPoint* pDataPoint, CBCGPChartSeries* pSeries, int nDataPointIndex);
};


class BCGCBPRODLLEXPORT CBCGPHistogramChartImpl : public CBCGPBarChartImpl
{
	DECLARE_DYNCREATE(CBCGPHistogramChartImpl)
public:
	CBCGPHistogramChartImpl()
	{
	}
	CBCGPHistogramChartImpl(CBCGPChartVisualObject* pRelatedChartControl) : CBCGPBarChartImpl(pRelatedChartControl)
	{
	}
	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
		const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);
};

class BCGCBPRODLLEXPORT CBCGPBubbleChartImpl : public CBCGPBaseChartImpl
{
	DECLARE_DYNCREATE(CBCGPBubbleChartImpl)
public:
	CBCGPBubbleChartImpl()
	{
	}
	CBCGPBubbleChartImpl(CBCGPChartVisualObject* pRelatedChartControl) : CBCGPBaseChartImpl(pRelatedChartControl)
	{
	}

	virtual AxisType GetAxisType() const
	{
		return AT_XY;
	}

	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
		const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);


 	virtual void OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
 		CBCGPChartSeries* pSeries, int nDataPointIndex);

	virtual void OnDrawDiagramMarker(CBCGPGraphicsManager* pGM, CBCGPChartSeries* pSeries, int nDataPointIndex, 
		const CBCGPChartDataPoint* pDataPoint, const BCGPChartFormatSeries* pFormatSeries, 
		const CBCGPRect& rectDiagram);
 	virtual void OnCalcDataPointLabelRect(CBCGPGraphicsManager* pGM, CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
 		CBCGPChartSeries* pSeries, int nDataPointIndex);

public:
	static const int BUBBLE_SP_SIZE;

};

class BCGCBPRODLLEXPORT CBCGPPieChartImpl : public CBCGPBaseChartImpl
{
	DECLARE_DYNCREATE(CBCGPPieChartImpl)
public:
	CBCGPPieChartImpl()
	{
		m_bIs3D = FALSE;
	}

	CBCGPPieChartImpl(CBCGPChartVisualObject* pRelatedChartControl, BOOL b3D = FALSE) : CBCGPBaseChartImpl(pRelatedChartControl)
	{
		m_bIs3D = b3D;
	}

	virtual AxisType GetAxisType() const
	{
		return AT_NO_AXIS;
	}

	virtual CBCGPSize OnCalcLegendKeySize(CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual CBCGPSize OnCalcLegendKeySizeEx(const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);

	virtual void OnCalcDataPointLabelRect(CBCGPGraphicsManager* pGM, CBCGPChartDataPoint* pDataPoint, const CBCGPRect& /*rectDiagramArea*/, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);

	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnDrawPie(CBCGPGraphicsManager* pGM, CBCGPChartDataPoint* pDP, 
							CBCGPChartSeries* pSeries, int nDataPointIndex, BOOL bIsExplosion, int nExcludeFlag = 0);
	virtual void OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual CBCGPPoint OnGetMarkerPoint(const CBCGPChartDataPoint* pDataPoint, CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
		const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);

	virtual BOOL Is3DPie() const { return m_bIs3D; }
	virtual int GetDoughnutPercent(CBCGPChartSeries* /*pSeries*/) const	{ return 0; }

	virtual BOOL IsTorus() const { return FALSE; }

public:
	static const int PIE_SP_START;
	static const int PIE_SP_END;
	static const int PIE_SP_ANGLES;
	static const int PIE_SP_RADIUS;
	static const int PIE_SP_CENTER;
	static const int PIE_SP_HEIGHT;
	static const int PIE_SP_DIAGRAM_CENTER;

protected:
	BOOL	m_bIs3D;
};

class BCGCBPRODLLEXPORT CBCGPDoughnutChartImpl : public CBCGPPieChartImpl
{
	DECLARE_DYNCREATE(CBCGPDoughnutChartImpl)

public:
	CBCGPDoughnutChartImpl()
	{
	}

	CBCGPDoughnutChartImpl(CBCGPChartVisualObject* pRelatedChartControl, BOOL b3D = FALSE) :
		CBCGPPieChartImpl(pRelatedChartControl, b3D)
	{
	}

	virtual int GetDoughnutPercent(CBCGPChartSeries* pSeries) const;
};

class BCGCBPRODLLEXPORT CBCGPTorusChartImpl : public CBCGPPieChartImpl
{
	DECLARE_DYNCREATE(CBCGPTorusChartImpl)

public:
	CBCGPTorusChartImpl()
	{
	}

	CBCGPTorusChartImpl(CBCGPChartVisualObject* pRelatedChartControl) :
		CBCGPPieChartImpl(pRelatedChartControl, TRUE)
	{
	}

	virtual BOOL IsTorus() const
	{
		return TRUE;
	}
};

class BCGCBPRODLLEXPORT CBCGPPyramidChartImpl : public CBCGPBaseChartImpl
{
	DECLARE_DYNCREATE(CBCGPPyramidChartImpl)

	CBCGPPyramidChartImpl()
	{
		m_bIs3D = FALSE;
	}

public:
	CBCGPPyramidChartImpl(CBCGPChartVisualObject* pRelatedChartControl, BOOL b3D = FALSE) : CBCGPBaseChartImpl(pRelatedChartControl)
	{
		m_bIs3D = b3D;
	}

	virtual AxisType GetAxisType() const
	{
		return AT_NO_AXIS;
	}

	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
											   const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);

	virtual void OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual CBCGPPoint OnGetMarkerPoint(const CBCGPChartDataPoint* pDataPoint, CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnCalcDataPointLabelRect(CBCGPGraphicsManager* pGM, CBCGPChartDataPoint* pDataPoint, const CBCGPRect& /*rectDiagramArea*/, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);
	
public:
	static const int PYRAMID_SP_HEIGHT;
	static const int PYRAMID_SP_OFFSETS;
	static const int PYRAMID_SP_LEFT_TOP;
	static const int PYRAMID_SP_RIGHT_BOTTOM;
	
protected:
	BOOL	m_bIs3D;

protected:
	virtual void OnDrawPyramidDataPoint(CBCGPGraphicsManager* pGM, CBCGPChartPyramidSeries* pSeries, int nDataPointIndex);
	void GetPyramidPoints(const CBCGPRect& rectPyramid, double dblDepth, double dblOffset, double Y1, double Y2, CBCGPPoint& ptLeft, CBCGPPoint& ptRight);
};

class BCGCBPRODLLEXPORT CBCGPFunnelChartImpl : public CBCGPPyramidChartImpl
{
	DECLARE_DYNCREATE(CBCGPFunnelChartImpl)

	CBCGPFunnelChartImpl()
	{
		m_bIs3D = FALSE;
	}

public:
	CBCGPFunnelChartImpl(CBCGPChartVisualObject* pRelatedChartControl, BOOL b3D = FALSE) : CBCGPPyramidChartImpl(pRelatedChartControl, b3D)
	{
		m_bIs3D = b3D;
	}

	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual CBCGPPoint OnGetMarkerPoint(const CBCGPChartDataPoint* pDataPoint, CBCGPChartSeries* pSeries, int nDataPointIndex);

protected:
	virtual void OnDrawFunnelDataPoint(CBCGPGraphicsManager* pGM, CBCGPChartFunnelSeries* pSeries, int nDataPointIndex);	
	void GetFunnelPoints(const CBCGPRect& rectFunnel, double dblDepth, const CBCGPSize& szNeckSize,
		double Y1, double Y2, CBCGPPoint& ptLeft, CBCGPPoint& ptRight);
};

class BCGCBPRODLLEXPORT CBCGPStockChartImpl : public CBCGPLineChartImpl
{
	DECLARE_DYNCREATE(CBCGPStockChartImpl)
public:
	CBCGPStockChartImpl()
	{
	}
	CBCGPStockChartImpl(CBCGPChartVisualObject* pRelatedChartControl) : CBCGPLineChartImpl(pRelatedChartControl)
	{
	}

	virtual AxisType GetAxisType() const
	{
		return AT_XY;
	}

	virtual CBCGPSize OnCalcLegendKeySize(CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual CBCGPSize OnCalcLegendKeySizeEx(const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);
	
	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnDrawStockDataPoint(CBCGPGraphicsManager* pGM, const CArray<CBCGPChartDataPoint*, CBCGPChartDataPoint*>& arDataPoints, 
		CBCGPChartStockSeries* pMainStockSeries, int nDataPointIndex, double dblCandleWidth, BOOL bIsVertical);
	
	virtual void OnDrawStockBar(CBCGPGraphicsManager* pGM, CBCGPPoint& ptOpen, CBCGPPoint& ptHigh, 
		CBCGPPoint& ptLow, CBCGPPoint& ptClose, CBCGPPoint& ptOpenStart, CBCGPPoint& ptCloseStart,
		const BCGPChartFormatLine& barStyle, CBCGPChartStockSeries* pMainStockSeries, int nDataPointIndex,
		BOOL bUp);
	virtual void OnDrawStockCandle(CBCGPGraphicsManager* pGM, CBCGPPoint& ptHigh, CBCGPPoint& ptLow, 
		CBCGPRect& rectCandle, const BCGPChartFormatLine& lineStyle, const BCGPChartFormatArea& candleStyle, 
		CBCGPChartStockSeries* pMainStockSeries, int nDataPointIndex, BOOL bUp);
	virtual void OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex);

	virtual void OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
		const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);
	virtual void OnDrawStockCandleEx(CBCGPGraphicsManager* pGM, CBCGPPoint& ptHigh, CBCGPPoint& ptLow, 
		CBCGPRect& rectCandle, const CBCGPBrush& brFill, const BCGPChartFormatLine& lineStyle);
	virtual void OnDrawStockBarEx(CBCGPGraphicsManager* pGM, CBCGPPoint& ptOpen, CBCGPPoint& ptHigh, 
		CBCGPPoint& ptLow, CBCGPPoint& ptClose, CBCGPPoint& ptOpenStart, CBCGPPoint& ptCloseStart,
		const BCGPChartFormatLine& lineStyle);

	virtual void OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
		CBCGPChartSeries* pSeries, int nDataPointIndex);

};

class BCGCBPRODLLEXPORT CBCGPLongDataChartImpl : public CBCGPBaseChartImpl
{
	DECLARE_DYNCREATE(CBCGPLongDataChartImpl)
public:
	CBCGPLongDataChartImpl()
	{
	}

	CBCGPLongDataChartImpl(CBCGPChartVisualObject* pRelatedChartControl) : CBCGPBaseChartImpl(pRelatedChartControl)
	{
	}

	virtual AxisType GetAxisType() const
	{
		return AT_XY;
	}

	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnCalcScreenPositions(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
	virtual void OnDrawDiagramMarkers(CBCGPGraphicsManager* pGM, CBCGPChartSeries* pSeries, const CBCGPRect& rectDiagram);
	virtual void OnDrawDiagramDataLabels(CBCGPGraphicsManager* pGM, CBCGPChartSeries* pSeries);

	virtual CBCGPSize OnCalcLegendKeySize(CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual CBCGPSize OnCalcLegendKeySizeEx(const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);

	virtual void OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex);
	virtual void OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
		const BCGPChartCellParams& params, CBCGPChartSeries* pSeries);
};

class BCGCBPRODLLEXPORT CBCGPPolarChartImpl : public CBCGPLineChartImpl
{
	DECLARE_DYNCREATE(CBCGPPolarChartImpl)
	CBCGPPolarChartImpl()
	{
	}

public:
	CBCGPPolarChartImpl(CBCGPChartVisualObject* pRelatedChartControl) : CBCGPLineChartImpl(pRelatedChartControl)
	{
	}

	virtual AxisType GetAxisType() const
	{
		return AT_RADIAL;
	}	

	virtual void OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries);
};

#endif // !defined(AFX_BCGPCHARTIMPL_H__14AAD790_6C8D_4160_A013_1C12C567C329__INCLUDED_)
