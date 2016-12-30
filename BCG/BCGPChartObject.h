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
// BCGPChartObject.h : header file
//

#if !defined(AFX_BCGPCHARTOBJECT_H__99CF881E_6ED1_4FA5_8891_C6AC9320B450__INCLUDED_)
#define AFX_BCGPCHARTOBJECT_H__99CF881E_6ED1_4FA5_8891_C6AC9320B450__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPChartFormat.h"

class CBCGPChartAxis;
class CBCGPGraphicsManager;
class CBCGPChartVisualObject;
class CBCGPChartSeries;

class BCGCBPRODLLEXPORT CBCGPChartObject : public CObject
{
	DECLARE_DYNCREATE(CBCGPChartObject);

protected:
	void CommonInit();

public:
	enum CoordinateMode
	{
		CM_PERCENTS,
		CM_PIXELS, // left - distance from left diagram bound, top - top bound, right - right bound, bottom - bottom bound
		CM_PIXELS_FIXED_SIZE, // // left - distance from left diagram bound, top - top bound, right - width, bottom - height
		CM_CHART_VALUES,
		CM_CHART_VALUE_DIST_ANGLE, // left - x value, top - y value, bottom - distance, right - not used
		CM_AXIS_INSIDE_MARK, // left - x value, top - y value, right - horz distance, bottom - vert distance
		CM_AXIS_OUTSIDE_MARK // same as inside mark, but the distance points outside of diagram 
	};

	static const double _EmptyCoordinate;
	static const CBCGPPoint _EmptyPoint;
	static const CBCGPRect _EmptyRect;

public:
	CBCGPChartObject();
	CBCGPChartObject(CBCGPChartVisualObject* pParentChart, 
		const CBCGPRect& rcCoordinates, CoordinateMode mode = CM_PIXELS);

	void SetCoordinates(const CBCGPRect& rcCoordinates, CoordinateMode mode = CM_PIXELS);
	void SetCoordinates(const CBCGPPoint& ptLeftTop, const CBCGPPoint& ptRightBottom, CoordinateMode  mode = CM_PIXELS);
	void SetCoordinates(double dblLeft, double dblTop, double dblRight, double dblBottom, CoordinateMode  mode = CM_PIXELS);

	CBCGPRect GetCoordinates() const {return m_rectCoordinates;}

	void SetAxisMarkCoordinate(double dblVal, BOOL bIsVert = TRUE, BOOL bOutside = TRUE);

	void SetCoordinateMode(CoordinateMode  mode) {m_coordinateMode = mode;}
	CoordinateMode GetCoordinateMode() const {return m_coordinateMode;}

	void SetObjectSize(const CBCGPSize sz, BOOL bRedraw = FALSE);
	CBCGPSize GetObjectSize() const {return m_szObjectSize;}

	void SetForeground(BOOL bFore = TRUE, BOOL bRedraw = FALSE);
	BOOL IsForeground() const {return m_bIsForeground;}

	void SetParentChart(CBCGPChartVisualObject* pChart);
	void SetRelatedAxes(CBCGPChartAxis* pXAxis, CBCGPChartAxis* pYAxis);
	BOOL IsObjectShownOnAxis(CBCGPChartAxis* pAxis) const;

	virtual BOOL HitTest(const CBCGPPoint& pt) const;

	virtual void OnCalcScreenPoints(CBCGPGraphicsManager* pGM);
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);

	CBCGPRect GetBoundingRect() const {return m_rectScreenBounds;}
	CBCGPRect GetShapeRect() const {return m_rectScreen;}

	BOOL IsRectOffsetsValid() const;

	void SetVisible(BOOL bVisible = TRUE) {m_bVisible = bVisible;}
	virtual BOOL IsVisible() const {return m_bVisible;}

	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld);

	BCGPChartFormatLabel	m_format;
	CString					m_strText;

	CString					m_strObjectName;
	int						m_nObjectID;

protected:
	// returns bounding rectangle to calculate offsets from
	// if X/Y axes are specified - bounding rectangle of axes
	// otherwise bounding rectangle of chart
	virtual CBCGPRect OnCalcBoundingRect();
	virtual CBCGPSize OnCalcObjectSize(CBCGPGraphicsManager* pGM);
	
	virtual void OnDrawShape(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawText(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);	

protected:
	CBCGPRect				m_rectScreen;
	CBCGPRect				m_rectScreenBounds;
	CBCGPPoint				m_ptAnchor;
	CBCGPSize				m_szCornerRadius;
	CBCGPSize				m_szObjectSize; // can be set by user
	double					m_dblShadowDepth;

	CBCGPChartObject::CoordinateMode	m_coordinateMode;

	BOOL					m_bVisible;
	BOOL					m_bIsForeground;	

	// offsets from left, top,  right, bottom sides of bounding axes, 
	// or plot area if axes are set to NULL
	// or coordinates in specified axes
	CBCGPRect				m_rectCoordinates;	

	CBCGPChartAxis*			m_pXAxis;
	CBCGPChartAxis*			m_pYAxis;
	CBCGPChartVisualObject* m_pParentChart;

	CBCGPBrush				m_brShadow;
};

class BCGCBPRODLLEXPORT CBCGPChartLineObject : public CBCGPChartObject
{
	DECLARE_DYNCREATE(CBCGPChartLineObject);

public:
	CBCGPChartLineObject();
	CBCGPChartLineObject(CBCGPChartVisualObject* pParentChart, 
		double dblX1, double dblY1, double dblX2, double dblY2, 
		const CBCGPBrush& brLine = CBCGPBrush(), double dblLineWidth = 1., CBCGPStrokeStyle* pStrokeStyle = NULL);
	CBCGPChartLineObject(CBCGPChartVisualObject* pParentChart, double dblVal, BOOL bHorz = TRUE, 
		const CBCGPBrush& brLine = CBCGPBrush(), double dblLineWidth = 1., CBCGPStrokeStyle* pStrokeStyle = NULL);

	virtual BOOL HitTest(const CBCGPPoint& pt) const;

protected:
	virtual void OnDrawShape(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawText(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram)
	{
		UNREFERENCED_PARAMETER(pGM);
		UNREFERENCED_PARAMETER(rectDiagram);
	}	
};

class BCGCBPRODLLEXPORT CBCGPChartRangeObject : public CBCGPChartObject
{
	DECLARE_DYNCREATE(CBCGPChartRangeObject);

public:
	CBCGPChartRangeObject();

	CBCGPChartRangeObject(CBCGPChartVisualObject* pParentChart, 
		double dblVal1, double dblVal2, BOOL bHorz, const CBCGPBrush& brFill);
};

class BCGCBPRODLLEXPORT CBCGPChartTextObject : public CBCGPChartObject
{
	DECLARE_DYNCREATE(CBCGPChartTextObject);

public:
	CBCGPChartTextObject();

	CBCGPChartTextObject(CBCGPChartVisualObject* pParentChart, 
		const CString& strText,
		double dblValX, double dblValY,
		const CBCGPBrush& brTextColor, const CBCGPBrush& brFill, const CBCGPBrush& brOutline,
		double dblDistanceFromPoint = _EmptyCoordinate, double dblAngle = _EmptyCoordinate,
		BOOL bDrawConnector = FALSE);
	
	void SetDrawConnector(BOOL bDraw = TRUE) {m_bDrawConnector = bDraw;}
	virtual BOOL IsDrawConnector() const {return m_bDrawConnector;}

	void SetRoundedShape(const CBCGPSize& sizeRadius = CBCGPSize(5.,5.))	// Set 0, 0 to remove rounded corners
	{
		m_szCornerRadius = sizeRadius;
	}

	const CBCGPSize& GetCornerRadius() const
	{
		return m_szCornerRadius;
	}

	void SetShadow(double dblShadowDepth = 5.)
	{
		m_dblShadowDepth = dblShadowDepth;
	}

	double GetShadowDepth() const
	{
		return m_dblShadowDepth;
	}

protected:
	virtual void OnDrawShape(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);

protected:
	BOOL	m_bDrawConnector;
};

class BCGCBPRODLLEXPORT CBCGPChartAxisMarkObject : public CBCGPChartObject
{
	DECLARE_DYNCREATE(CBCGPChartAxisMarkObject);

public:
	CBCGPChartAxisMarkObject();

	CBCGPChartAxisMarkObject(CBCGPChartVisualObject* pParentChart, double dblVal, const CString& strText, BOOL bVertAxis, BOOL bOutside,
		const CBCGPBrush& brTextColor, const CBCGPBrush& brFill = CBCGPBrush(), const CBCGPBrush& brOutline = CBCGPBrush());
};

class BCGCBPRODLLEXPORT CBCGPChartBaseEffect : public CObject
{
protected:
	CBCGPChartBaseEffect()
	{
		m_pChart = NULL;
		m_bVisible = TRUE;
	}
public:
	virtual ~CBCGPChartBaseEffect(){}

	virtual void OnDraw(CBCGPGraphicsManager* pGM) = 0;
	virtual void OnCalcScreenPoints(CBCGPGraphicsManager* pGM) = 0;
	void SetParentChart(CBCGPChartVisualObject* pChart) {m_pChart = pChart;}
	CBCGPChartVisualObject* GetParentChart() const {return m_pChart;}

	virtual BOOL IsVisible() const {return m_bVisible;}
	void SetVisible(BOOL bVisible = TRUE) {m_bVisible = bVisible;}

	virtual BOOL IsEffectShownOnAxis(CBCGPChartAxis* pAxis) = 0;

protected:
	CBCGPChartVisualObject* m_pChart;
	BOOL					m_bVisible;
};

class BCGCBPRODLLEXPORT CBCGPChartInterLineColoringEffect : public CBCGPChartBaseEffect
{
public:
	CBCGPChartInterLineColoringEffect(CBCGPChartVisualObject* pChart, CBCGPChartSeries* pSeries1, CBCGPChartSeries* pSeries2);
	CBCGPChartInterLineColoringEffect(CBCGPChartVisualObject* pChart, CBCGPChartSeries* pSeries1, double dblOrigin);
	virtual ~CBCGPChartInterLineColoringEffect();

	virtual void OnCalcScreenPoints(CBCGPGraphicsManager* pGM);
	virtual void OnDraw(CBCGPGraphicsManager* pGM);

	virtual BOOL IsEffectShownOnAxis(CBCGPChartAxis* pAxis);

	double	m_dblOrigin;
	BOOL	m_bTopOnly;

	CBCGPBrush m_brTopBrush;
	CBCGPBrush m_brBottomBrush;

protected:
	CBCGPGeometry* CreateGeometry(const CBCGPPointsArray& arOrgPoints, const CBCGPRect& rectBounds, 
									BCGPChartFormatSeries::ChartCurveType curveType, BOOL bClip);
	CBCGPGeometry* CreateClipGeometry(double dblOrigin);

	void DrawEffect(CBCGPGraphicsManager* pGM, CBCGPGeometry* pDrawGeometry, CBCGPGeometry* pClipGeometry, 
					const CBCGPRect& rectBounds, const CBCGPBrush& brFill);

protected:
	CBCGPChartSeries*	m_pSeries1;
	CBCGPChartSeries*	m_pSeries2;

	CBCGPPointsArray	m_arPointsSeries1;
	CBCGPPointsArray	m_arPointsSeries2;
};

#endif // !defined(AFX_BCGPCHARTOBJECT_H__99CF881E_6ED1_4FA5_8891_C6AC9320B450__INCLUDED_)
