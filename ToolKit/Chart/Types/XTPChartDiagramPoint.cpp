// XTPChartDiagramPoint.cpp
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

#include "stdafx.h"

#include "XTPChartDiagramPoint.h"
#include "../Utils/XTPChartMathUtils.h"

#include <math.h>

CXTPChartDiagramPoint::CXTPChartDiagramPoint()
{
	X = Y = Z = 0;
}

CXTPChartDiagramPoint::CXTPChartDiagramPoint(double x, double y, double z)
{
	X = x;
	Y = y;
	Z = z;
}

CXTPChartDiagramPoint::CXTPChartDiagramPoint(const CXTPChartPointF& point)
{
	X = point.X;
	Y = point.Y;
	Z = 0;
}

CXTPChartDiagramPoint:: operator CXTPChartPointF() const
{
	return CXTPChartPointF((float)X, (float)Y);

}

void CXTPChartDiagramPoint::Offset(double dx, double dy, double dz)
{
	X += dx;
	Y += dy;
	Z += dz;
}

void CXTPChartDiagramPoint::Revert()
{
	X = -X;
	Y = -Y;
	Z = -Z;
}

void CXTPChartDiagramPoint::Normalize()
{
	double length = sqrt(X * X + Y * Y + Z * Z);

	if (CXTPChartMathUtils::Compare(length, 0.0) != 0)
	{
		X /= length;
		Y /= length;
		Z /= length;
	}
	else
	{
		X = 0;
		Y = 0;
		Z = 0;
	}
}

CXTPChartDiagramPoint CXTPChartDiagramPoint::CalcNormal(const  CXTPChartDiagramPoint& p1, const  CXTPChartDiagramPoint& p2, const CXTPChartDiagramPoint& p3)
{

	CXTPChartDiagramPoint normal((p2.Y - p1.Y) * (p3.Z - p1.Z) - (p2.Z - p1.Z) * (p3.Y - p1.Y),
		(p2.Z - p1.Z) * (p3.X - p1.X) - (p2.X - p1.X) * (p3.Z - p1.Z),
		(p2.X - p1.X) * (p3.Y - p1.Y) - (p2.Y - p1.Y) * (p3.X - p1.X));

	normal.Normalize();
	return normal;
}
