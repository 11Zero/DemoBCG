// XTPChartFunnelSeriesStyle.h
//
// This file is a part of the XTREME TOOLKIT PRO MFC class library.
// (c)1998-2011 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME TOOLKIT PRO LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////

//{{AFX_CODEJOCK_PRIVATE
#if !defined(__XTPCHARTFUNNELSERIESSTYLE_H__)
#define __XTPCHARTFUNNELSERIESSTYLE_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CXTPChartSeriesView;
class CXTPChartBorder;
class CXTPChartFillStyle;

#include "../../XTPChartSeriesStyle.h"
#include "../../XTPChartSeriesView.h"

//===========================================================================
// Summary:
//     This class abstracts the basic style of a pie serie.This class is a kind of
//     CXTPChartSeriesStyle.
// Remarks:
//     A pie chart (or a circle graph) is a circular chart divided into sectors,
//     illustrating percents. In a pie chart, the arc length of each sector (and
//     consequently its central angle and area), is proportional to the quantity
//     it represents. Together, the sectors create a full disk.
//===========================================================================
class _XTP_EXT_CLASS CXTPChartFunnelSeriesStyle : public CXTPChartSeriesStyle
{
	DECLARE_SERIAL(CXTPChartFunnelSeriesStyle)
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartFunnelSeriesStyleBase object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartFunnelSeriesStyle();

	//-------------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPChartFunnelSeriesStyleBase object, handle cleanup.
	//-------------------------------------------------------------------------
	virtual ~CXTPChartFunnelSeriesStyle();

public:
	int GetPointDistance() const;
	void SetPointDistance(int nPointDistance);

	double GetHeightToWidthRatio() const;
	void SetHeightToWidthRatio(double dRatio);

	void SetEqualHeight(BOOL bEqualHeight);
	BOOL IsEqualHeight() const;

public:
	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to get the background fill style.
	// Returns:
	//     Returns the pointer to CXTPChartFillStyle object, which abstracts
	//     various fill styles.
	// Remarks:
	// See Also:
	//-------------------------------------------------------------------------
	CXTPChartFillStyle* GetFillStyle() const;

	CXTPChartBorder* GetBorder() const;

public:
	void DoPropExchange(CXTPPropExchange* pPX);

protected:
	CXTPChartDiagram* CreateDiagram();
	CXTPChartSeriesView* CreateView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView);
	BOOL IsStyleDiagram(CXTPChartDiagram* pDiagram) const;


#ifdef _XTP_ACTIVEX
public:
//{{AFX_CODEJOCK_PRIVATE
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
	DECLARE_OLETYPELIB_EX(CXTPChartFunnelSeriesStyle);
	DECLARE_OLECREATE_EX(CXTPChartFunnelSeriesStyle)
//}}AFX_CODEJOCK_PRIVATE
#endif

protected:
	double m_dHeightToWidthRatio;

	int m_nPointDistance;
	CXTPChartBorder* m_pBorder;
	CXTPChartFillStyle* m_pFillStyle;
	BOOL m_bEqualHeight;
};


//===========================================================================
// Summary:
//     This class abstracts the view of a pie series.This class is a kind of
//     CXTPChartSeriesView.
// Remarks:
//     A pie chart (or a circle graph) is a circular chart divided into sectors,
//     illustrating percents. In a pie chart, the arc length of each sector (and
//     consequently its central angle and area), is proportional to the quantity
//     it represents. Together, the sectors create a full disk.
//===========================================================================
class _XTP_EXT_CLASS CXTPChartFunnelSeriesView : public CXTPChartSeriesView
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartFunnelSeriesView object.
	// Parameters:
	//     pSeries       - Pointer to pie series object.
	//     pDiagramView  - Pointer to the pie diagram view.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartFunnelSeriesView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView);

	//-------------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPChartFunnelSeriesView object, handle cleanup.
	//-------------------------------------------------------------------------
	virtual ~CXTPChartFunnelSeriesView();

public:
	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to calculate the pie series point layout.
	// Parameters:
	//     pDC      - Pointer to the chart device context.
	//     rcBounds - The rectangular boundary.
	//-------------------------------------------------------------------------
	void CalculatePointLayout(CXTPChartDeviceContext* pDC, CRect rcBounds);

	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to calculate the pie series label layout.
	// Parameters:
	//     pDC      - Pointer to the chart device context.
	//     rcBounds - The rectangular boundary.
	//-------------------------------------------------------------------------
	void CalculateLabelLayout(CXTPChartDeviceContext* pDC);

	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to get the inner bounds of the diagram.
	// Returns:
	//     A chart rect object denoting the rectangular boundary.
	//-------------------------------------------------------------------------
	CXTPChartRectF GetInnerBounds() const;

protected:

	//-------------------------------------------------------------------------
	// Summary:
	//     Use this function to create the pie series point view object.
	// Parameters:
	//     pDC    - Pointer to chart device context.
	//     pPoint - Pointer to chart series point.
	// Returns:
	//     A pointer to the chart series point view.
	//-------------------------------------------------------------------------
	virtual CXTPChartSeriesPointView* CreateSeriesPointView(CXTPChartDeviceContext* pDC, CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView);

	//-------------------------------------------------------------------------
	// Summary:
	//     This function create a CXTPChartDeviceCommand object, this object
	//     represents the rendering of the pie series.
	// Parameters:
	//     pDC      - Pointer to a CXTPChartDeviceContext object.
	//     rcBounds - The rectangular boundary of the legend.
	// Returns:
	//     Returns CXTPChartDeviceCommand object, this object handles
	//     the rendering of an element in the chart.Here it handles
	//     the drawing of the pie series.
	// Remarks:
	// See Also:
	//-------------------------------------------------------------------------
	CXTPChartDeviceCommand* CreateDeviceCommand(CXTPChartDeviceContext* pDC);

	CXTPChartDeviceCommand* CreateLegendDeviceCommand(CXTPChartDeviceContext* pDC, CRect rcBounds);

	void CalculateValues();

public:
	CXTPChartDeviceCommand* CreateLegendDeviceCommand(CXTPChartDeviceContext* pDC, CRect rcBounds,
		CXTPChartColor color1, CXTPChartColor color2, CXTPChartColor clrBorder);

protected:
	CXTPChartRectF m_rcInnterBounds;
};

//===========================================================================
// Summary:
//     This class abstracts the view of a pie series point.This class is a kind of
//     CXTPChartSeriesPointView.
// Remarks:
//     A pie chart (or a circle graph) is a circular chart divided into sectors,
//     illustrating percents. In a pie chart, the arc length of each sector (and
//     consequently its central angle and area), is proportional to the quantity
//     it represents. Together, the sectors create a full disk.
//===========================================================================
class _XTP_EXT_CLASS CXTPChartFunnelSeriesPointView : public CXTPChartSeriesPointView
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartFunnelSeriesPointView object.
	// Parameters:
	//     pPoint        - Pointer to chart series point.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartFunnelSeriesPointView(CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView);

	//-------------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPChartFunnelSeriesPointView object, handle cleanup.
	//-------------------------------------------------------------------------
	virtual ~CXTPChartFunnelSeriesPointView();

public:
	//-------------------------------------------------------------------------
	// Summary:
	//     Use this function to calculate the layout of the pie series point.
	// Parameters:
	//     dAngle - The swipe angle of the sector(pie).
	//-------------------------------------------------------------------------
	void CalculateLayout(int nIndex, int nCount, double dFrom, double dTo);

	CXTPChartColor GetBorderActualColor() const;

protected:

	//-------------------------------------------------------------------------
	// Summary:
	//     This function create a CXTPChartDeviceCommand object, this object
	//     represents the rendering of the pie series point.
	// Parameters:
	//     pDC      - Pointer to a CXTPChartDeviceContext object.
	// Returns:
	//     Returns CXTPChartDeviceCommand object, this object handles
	//     the rendering of an element in the chart.Here it handles
	//     the drawing of the pie series point.
	// Remarks:
	// See Also:
	//-------------------------------------------------------------------------
	virtual CXTPChartDeviceCommand* CreateDeviceCommand(CXTPChartDeviceContext* pDC);

	CXTPChartDeviceCommand* CreateLegendDeviceCommand(CXTPChartDeviceContext* pDC, CRect rcBounds);
protected:
	double m_dValue;            //The value of the point.

public:
	CXTPChartRectF m_rc;

	double m_dFrom;
	double m_dTo;

	friend class CXTPChartFunnelSeriesView;
};


AFX_INLINE int CXTPChartFunnelSeriesStyle::GetPointDistance() const {
	return m_nPointDistance;
}
AFX_INLINE void CXTPChartFunnelSeriesStyle::SetPointDistance(int nPointDistance) {
	m_nPointDistance = nPointDistance;
	OnChartChanged();
}
AFX_INLINE CXTPChartFillStyle* CXTPChartFunnelSeriesStyle::GetFillStyle() const {
	return m_pFillStyle;
}
AFX_INLINE CXTPChartBorder* CXTPChartFunnelSeriesStyle::GetBorder() const {
	return m_pBorder;
}
AFX_INLINE CXTPChartRectF CXTPChartFunnelSeriesView::GetInnerBounds() const {
	return m_rcInnterBounds;
}
AFX_INLINE void CXTPChartFunnelSeriesStyle::SetHeightToWidthRatio(double dRatio) {
	m_dHeightToWidthRatio = dRatio;
	OnChartChanged();
}
AFX_INLINE double CXTPChartFunnelSeriesStyle::GetHeightToWidthRatio() const {
	return m_dHeightToWidthRatio;
}
AFX_INLINE void CXTPChartFunnelSeriesStyle::SetEqualHeight(BOOL bEqualHeight) {
	m_bEqualHeight = bEqualHeight;
	OnChartChanged();
}
AFX_INLINE BOOL CXTPChartFunnelSeriesStyle::IsEqualHeight() const {
	return m_bEqualHeight;
}


#endif //#if !defined(__XTPCHARTFUNNELSERIESSTYLE_H__)
