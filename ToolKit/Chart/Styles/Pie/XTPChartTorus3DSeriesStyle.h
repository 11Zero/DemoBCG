// XTPChartTorus3DSeriesStyle.h
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
#if !defined(__XTPCHARTTODUS3DSERIESSTYLE_H__)
#define __XTPCHARTTODUS3DSERIESSTYLE_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CXTPChartSeriesView;

#include "XTPChartPieSeriesStyle.h"

//===========================================================================
// Summary:
//     This class abstracts the style of a torus seris.This class is a kind of
//     CXTPChartPieSeriesStyleBase.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartTorus3DSeriesStyle: public CXTPChartPieSeriesStyleBase
{
	DECLARE_SERIAL(CXTPChartTorus3DSeriesStyle)

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartTorus3DSeriesStyle object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartTorus3DSeriesStyle();

	//-------------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPChartTorus3DSeriesStyle object, handles cleanup
	//-------------------------------------------------------------------------
	virtual ~CXTPChartTorus3DSeriesStyle();

public:
	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to get the depth of the torus.
	// Returns:
	//     An integer value specifying the depth of the torus.
	//-------------------------------------------------------------------------
	int GetDepth() const;

protected:
	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to create the diagram for the torus series.
	// Returns:
	//     A pointer to CXTPChartDiagram refers to a newly created CXTPChartPie3DDiagram
	//     object.
	//-------------------------------------------------------------------------
	virtual CXTPChartDiagram* CreateDiagram();

	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to create the view of the torus series.
	// Parameters:
	//     pSeries      - A pointer to chart series.
	//     pDiagramView - A pointer to the diagram view.
	// Returns:
	//     A pointer to CXTPChartSeriesView refers to a newly created
	//     CXTPChartTorus3DSeriesView object.
	//-------------------------------------------------------------------------
	virtual CXTPChartSeriesView* CreateView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to check whether chart diagram object passed
	//     is a kind of CXTPChartPie3DDiagram.
	// Parameters:
	//     pDiagram - A pointer to a chart diagram, whose type is to be identified.
	// Returns:
	//     TRUE if the chart diagram is a pie 3D diagram and FALSE if not.
	// Remarks:
	//-----------------------------------------------------------------------
	virtual BOOL IsStyleDiagram(CXTPChartDiagram* pDiagram) const;

#ifdef _XTP_ACTIVEX
public:
//{{AFX_CODEJOCK_PRIVATE
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
	DECLARE_OLETYPELIB_EX(CXTPChartTorus3DSeriesStyle);
	DECLARE_OLECREATE_EX(CXTPChartTorus3DSeriesStyle)
//}}AFX_CODEJOCK_PRIVATE
#endif
};



#endif //#if !defined(__XTPCHARTTODUS3DSERIESSTYLE_H__)
