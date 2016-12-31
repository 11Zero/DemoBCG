// XTPChartRangeBarSeriesStyle.h
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
#if !defined(__XTPCHARTRANGEBARSERIESSTYLE_H__)
#define __XTPCHARTRANGEBARSERIESSTYLE_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CXTPChartSeriesView;

#include "../Bar/XTPChartBarSeriesStyle.h"

//===========================================================================
// Summary:
//     This class represents a RangeBar series style for the chart,which is a kind of
//     CXTPChartBarSeriesStyle.
// Remarks:
//    A RangeBar chart or RangeBar graph is a chart with rectangular RangeBars with lengths
//    proportional to the values that they represent. RangeBar charts are used for
//    comparing two or more values that were taken over time or on different
//    conditions, usually on small data sets.
//===========================================================================
class _XTP_EXT_CLASS CXTPChartRangeBarSeriesStyle : public CXTPChartBarSeriesStyle
{
	DECLARE_SERIAL(CXTPChartRangeBarSeriesStyle)
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartRangeBarSeriesStyle object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartRangeBarSeriesStyle();

	//-------------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPChartRangeBarSeriesStyle object, handles cleanup.
	//-------------------------------------------------------------------------
	virtual ~CXTPChartRangeBarSeriesStyle();

public:
	CXTPChartMarker* GetMinValueMarker() const;
	CXTPChartMarker* GetMaxValueMarker() const;


protected:

	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to create the view of the RangeBar series.
	// Parameters:
	//     pSeries      - A pointer to the chart series object, to which this object
	//                    is associated with.
	//     pDiagramView - A pointer to the chart diagram view object, to which this object
	//                    is associated with.
	// Returns:
	//     A pointer to CXTPChartSeriesView, refers a newly created CXTPChartRangeBarSeriesView
	//     object.
	//-------------------------------------------------------------------------
	virtual CXTPChartSeriesView* CreateView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView);

public:
	void DoPropExchange(CXTPPropExchange* pPX);


protected:
	CXTPChartMarker* m_pMinValueMarker;
	CXTPChartMarker* m_pMaxValueMarker;

protected:

#ifdef _XTP_ACTIVEX
public:
//{{AFX_CODEJOCK_PRIVATE
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
	DECLARE_OLETYPELIB_EX(CXTPChartRangeBarSeriesStyle);
	DECLARE_OLECREATE_EX(CXTPChartRangeBarSeriesStyle);

	afx_msg LPDISPATCH OleGetMinValueMarker();
	afx_msg LPDISPATCH OleGetMaxValueMarker();

//}}AFX_CODEJOCK_PRIVATE
#endif
};
//===========================================================================
// Summary:
//     This class represents the view of a RangeBar series ,which is a kind of
//     CXTPChartSeriesView.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartRangeBarSeriesView : public CXTPChartBarSeriesView
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartRangeBarSeriesView object.
	// Parameters:
	//     pSeries      - A pointer to the chart series object.
	//     pDiagramView - A pointer to the diagram view object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartRangeBarSeriesView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView);

protected:
	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to create view of the RangeBar series point.
	// Parameters:
	//     pDC    - A pointer to chart device context.
	//     pPoint - A pointer to the chart series point object.
	// Returns:
	//     A pointer to CXTPChartSeriesPointView, which refers to a newly created
	//     CXTPChartRangeBarSeriesPointView object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartSeriesPointView* CreateSeriesPointView(CXTPChartDeviceContext* pDC, CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView);

protected:

	friend class CXTPChartRangeBarSeriesPointView;
};

//===========================================================================
// Summary:
//     This class represents the view of a RangeBar series point,which is a kind of
//     CXTPChartPointSeriesPointView.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartRangeBarSeriesPointView : public CXTPChartBarSeriesPointView
{
public:

	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartRangeBarSeriesPointView object.
	// Parameters:
	//     pPoint - A pointer to the chart series point object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartRangeBarSeriesPointView(CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView);

public:
	virtual CXTPChartRectF GetScreenRect() const;

	virtual CXTPChartDeviceCommand* CreateDeviceCommand(CXTPChartDeviceContext* pDC);

public:
	virtual void UpdateMinMaxRange(double& nMinValue, double& nMaxValue) const;

	CXTPChartPointF GetScreenPoint(BOOL bMinValue) const;


public:

};

AFX_INLINE CXTPChartMarker* CXTPChartRangeBarSeriesStyle::GetMinValueMarker() const {
	return m_pMinValueMarker;
}
AFX_INLINE CXTPChartMarker* CXTPChartRangeBarSeriesStyle::GetMaxValueMarker() const {
	return m_pMaxValueMarker;
}

#endif //#if !defined(__XTPCHARTRANGEBARSERIESSTYLE_H__)
