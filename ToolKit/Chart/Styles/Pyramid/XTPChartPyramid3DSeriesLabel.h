// XTPChartPyramidSeriesLabel.h
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
#if !defined(__XTPCHARTPYRAMID3DSERIESLABEL_H__)
#define __XTPCHARTPYRAMID3DSERIESLABEL_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "XTPChartPyramidSeriesLabel.h"

class CXTPChartSeriesView;
class CXTPChartPyramidSeriesPointView;
class CXTPChartPyramidSeriesStyleBase;
class CXTPChartPyramid3DDiagramDomain;
class CXTPChartPyramid3DSeriesStyle;

//===========================================================================
// Summary:
//     CXTPChartPyramid3DSeriesLabel is a kind of CXTPChartPyramidSeriesLabel, this class
//     abstracts the label of a 3D pie series.
// Remarks:
//
//===========================================================================
class _XTP_EXT_CLASS CXTPChartPyramid3DSeriesLabel : public CXTPChartPyramidSeriesLabel
{
	DECLARE_SERIAL(CXTPChartPyramid3DSeriesLabel)

	class CView;
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartPyramid3DSeriesLabel object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartPyramid3DSeriesLabel();
	//-------------------------------------------------------------------------
	// Summary:
	//     Destroys a CXTPChartPyramid3DSeriesLabel object, handles cleanup
	//-------------------------------------------------------------------------
	virtual ~CXTPChartPyramid3DSeriesLabel();

public:
	CXTPChartPyramid3DSeriesStyle* GetStyle() const;

public:
	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to calculate the anchor points and angles of the
	//     3d pie label stem.
	// Parameters:
	//     pPointView       - The pie series point view.
	//     borderThickness  - The border thickness of the label.
	//     lineAngle        - The line(stem) angle, out param.
	// Returns:
	//     Returns CXTPChartPointF object denoting the anchor point.
	// See Also:
	//-------------------------------------------------------------------------
	CXTPChartPointF CalculateAnchorPointAndAngles(CXTPChartPyramidSeriesPointView* pPointView, double& lineAngle);

	//-------------------------------------------------------------------------
	// Summary:
	//     Call this function to project the 3D pie label on 2D window.
	// Parameters:
	//     pDomain      - A pointer to the 3D diagram domain object.
	//     labelPoint   - The start point of the label.
	//     lineAngle    - The line(stem) angle, out param.
	// Returns:
	//     Returns CXTPChartPointF object denoting the anchor point.
	// See Also:
	//-------------------------------------------------------------------------
	CXTPChartPointF Project(CXTPChartPyramid3DDiagramDomain* pDomain, double dPos, double dSize, double& lineAngle);

	CXTPChartElementView* CreateView(CXTPChartDeviceContext* pDC, CXTPChartSeriesPointView* pPointView, CXTPChartElementView* pParentView);
};



#endif //#if !defined(__XTPCHARTPYRAMID3DSERIESLABEL_H__)
