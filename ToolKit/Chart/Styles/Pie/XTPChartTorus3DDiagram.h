// XTPChartTorus3DDiagram.h
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
#if !defined(__XTPCHARTTORUS3DDIAGRAM_H__)
#define __XTPCHARTTORUS3DDIAGRAM_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "XTPChartPie3DDiagram.h"

//===========================================================================
// Summary:
//     This class abstracts the view of a torus seris.This class is a kind of
//     CXTPChartPie3DSeriesView.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartTorus3DSeriesView : public CXTPChartPie3DSeriesView
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartTorus3DSeriesView object.
	// Parameters:
	//     pSeries      - The chart series pointer.
	//     pDiagramView - Pointer to chart diagram view.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartTorus3DSeriesView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView);

	//-------------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPChartTorus3DSeriesView object, handles cleanup
	//-------------------------------------------------------------------------
	virtual ~CXTPChartTorus3DSeriesView();

public:
	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to create the point view of torus series.
	// Parameters:
	//     pDC    - The chart device command.
	//     pPoint - The chart series point.
	// Returns:
	//     A pointer to CXTPChartSeriesPointView which refers to a newly created
	//     CXTPChartTorus3DSeriesPointView object.
	//-------------------------------------------------------------------------
	virtual CXTPChartSeriesPointView* CreateSeriesPointView(CXTPChartDeviceContext* pDC, CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView);

};

//===========================================================================
// Summary:
//     This class abstracts the view of a torus series point.This class is a kind of
//     CXTPChartPie3DSeriesPointView.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartTorus3DSeriesPointView : public CXTPChartPie3DSeriesPointView
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartTorus3DSeriesPointView object.
	// Parameters:
	//     pSeries      - The chart series pointer.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartTorus3DSeriesPointView(CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView);

	//-------------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPChartTorus3DSeriesPointView object, handles cleanup
	//-------------------------------------------------------------------------
	virtual ~CXTPChartTorus3DSeriesPointView();

public:

protected:
	//-------------------------------------------------------------------------
	// Summary:
	//     This function create a CXTPChartDeviceCommand object, this object
	//     represents the rendering of the torus series point.
	// Parameters:
	//     pDC     - Pointer to a CXTPChartDeviceContext object.
	// Returns:
	//     Returns CXTPChartDeviceCommand object, this object handles
	//     the rendering of an element in the chart.Here it renders the torus
	//     series point.
	// Remarks:
	//-------------------------------------------------------------------------
	virtual CXTPChartDeviceCommand* CreateDeviceCommand(CXTPChartDeviceContext* pDC);

protected:
	};


#endif //#if !defined(__XTPCHARTTORUS3DDIAGRAM_H__)
