// XTPChartDiagramPoint.h
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
#if !defined(__XTPCHARTDIAGRAMPOINT_H__)
#define __XTPCHARTDIAGRAMPOINT_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "XTPChartTypes.h"
//===========================================================================
// Summary:
//     This class represents a diagram point.It represents a vertex for 3D
//     modeling
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartDiagramPoint
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartDiagramPoint object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDiagramPoint();

	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartDeviceContext object.
	// Parameters:
	//     x - The X coordinate.
	//     y - The Y coordinate.
	//     z - The Z coordinate.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDiagramPoint(double x, double y, double z = 0);

	//-----------------------------------------------------------------------
	// Summary:
	//     Copy constructor, constructs a CXTPChartDeviceContext object.
	// Parameters:
	//     point - A reference to an existing chart point object.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartDiagramPoint(const CXTPChartPointF& point);

public:
	//-----------------------------------------------------------------------
	// Summary:
	//     The CXTPChartPointF conversion operator function
	// Returns:
	//     A CXTPChartPointF object.
	// Remarks:
	//-----------------------------------------------------------------------
	operator CXTPChartPointF() const;

public:

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to offset the point to another location.
	// Parameters:
	//     dx - The change in X coordinate.
	//     dy - The change in Y coordinate.
	//     dz - The change in Z coordinate.
	// Remarks:
	//-----------------------------------------------------------------------
	void Offset(double dx, double dy, double dz = 0);

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to reverse the sign of point.
	// Remarks:
	//-----------------------------------------------------------------------
	void Revert();

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to normalize point.
	// Remarks:
	//-----------------------------------------------------------------------
	void Normalize();

	//-----------------------------------------------------------------------
	// Summary:
	//     Call this function to calculate the normal to this point.
	// Returns
	//     A normalized CXTPChartDiagramPoint point defining the normal to the
	//     current point.
	// Remarks:
	//-----------------------------------------------------------------------
	static CXTPChartDiagramPoint CalcNormal(const  CXTPChartDiagramPoint& p1, const  CXTPChartDiagramPoint& p2, const CXTPChartDiagramPoint& p3);



public:

	double X;       //The X coordinate.
	double Y;       //The Y coordinate.
	double Z;       //The Z coordinate.
};



#endif //#if !defined(__XTPCHARTDIAGRAMPOINT_H__)
