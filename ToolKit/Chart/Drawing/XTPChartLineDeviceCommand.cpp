// XTPChartLineDeviceCommand.cpp
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
#include <math.h>

#include "GraphicLibrary/GdiPlus/GdiPlus.h"
#include "GraphicLibrary/OpenGL/GL.h"

#include "XTPChartLineDeviceCommand.h"
#include "XTPChartDeviceContext.h"
#include "XTPChartOpenGLDeviceContext.h"


using namespace Gdiplus;
using namespace Gdiplus::DllExports;


//////////////////////////////////////////////////////////////////////////
// CXTPChartLineDeviceCommand

CXTPChartLineDeviceCommand::CXTPChartLineDeviceCommand(const CXTPChartDiagramPoint& p1, const CXTPChartDiagramPoint& p2, int thickness)
{
	m_p1 = p1;
	m_p2 = p2;
	m_nThickness = thickness;
};

CXTPChartElement* CXTPChartLineDeviceCommand::HitTest(CPoint point, CXTPChartElement* pParent) const
{
	if (point.x < min(m_p1.X, m_p2.X) - m_nThickness)
		return NULL;
	if (point.x > max(m_p1.X, m_p2.X) + m_nThickness)
		return NULL;

	if (point.y < min(m_p1.Y, m_p2.Y) - m_nThickness)
		return NULL;
	if (point.y > max(m_p1.Y, m_p2.Y) + m_nThickness)
		return NULL;

	double d = sqrt((m_p1.X - m_p2.X) * (m_p1.X - m_p2.X) + (m_p1.Y - m_p2.Y) * (m_p1.Y - m_p2.Y));
	if (d < m_nThickness)
		return pParent;

	double fArea = fabs((m_p2.X - m_p1.X) * (point.y - m_p1.Y) -  (m_p2.Y - m_p1.Y) * (point.x - m_p1.X));
	if (fArea / d <= m_nThickness)
		return pParent;

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartSolidLineDeviceCommand

CXTPChartSolidLineDeviceCommand::CXTPChartSolidLineDeviceCommand(const CXTPChartDiagramPoint& p1, const CXTPChartDiagramPoint& p2, const CXTPChartColor& color, int thickness)
	: CXTPChartLineDeviceCommand (p1, p2, thickness)
{
	m_color = color;
};


void CXTPChartSolidLineDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GpPen* pGpPen = NULL;
	GdipCreatePen1(m_color.GetValue(), (REAL)m_nThickness, UnitWorld, &pGpPen);

	GdipDrawLine(pDC->GetGraphics(), pGpPen, (REAL)m_p1.X, (REAL)m_p1.Y, (REAL)m_p2.X, (REAL)m_p2.Y);


	GdipDeletePen(pGpPen);

}

void CXTPChartSolidLineDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* /*pDC*/)
{

}

//////////////////////////////////////////////////////////////////////////
// CXTPChartDashedLineGraphicsCommand

CXTPChartDashedLineDeviceCommand::CXTPChartDashedLineDeviceCommand(const CXTPChartDiagramPoint& p1, const CXTPChartDiagramPoint& p2, const CXTPChartColor& color, int thickness, XTPChartDashStyle nDashStyle)
	: CXTPChartLineDeviceCommand (p1, p2, thickness)
{
	m_color = color;
	m_nDashStyle = nDashStyle;
};


void CXTPChartDashedLineDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GpPen* pGpPen = NULL;
	GdipCreatePen1(m_color.GetValue(), (REAL)m_nThickness, UnitWorld, &pGpPen);

	GdipSetPenDashStyle(pGpPen,
		m_nDashStyle == xtpChartDashStyleDash ? DashStyleDash :
		m_nDashStyle == xtpChartDashStyleDashDot ? DashStyleDashDot :
		m_nDashStyle == xtpChartDashStyleDot ? DashStyleDot :
		m_nDashStyle == xtpChartDashStyleDashDotDot ? DashStyleDashDotDot : DashStyleSolid);

	GdipDrawLine(pDC->GetGraphics(), pGpPen, (REAL)m_p1.X, (REAL)m_p1.Y, (REAL)m_p2.X, (REAL)m_p2.Y);


	GdipDeletePen(pGpPen);

}


//////////////////////////////////////////////////////////////////////////
// CXTPChartSolidPolylineDeviceCommand

CXTPChartSolidPolylineDeviceCommand::CXTPChartSolidPolylineDeviceCommand(const CXTPChartPoints& p, const CXTPChartColor& color, int thickness)
{
	m_p.Copy(p);
	m_nThickness = thickness;
	m_color = color;
}

void CXTPChartSolidPolylineDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GpPen* pGpPen = NULL;
	GdipCreatePen1(m_color.GetValue(), (REAL)m_nThickness, UnitWorld, &pGpPen);

	GdipDrawLines(pDC->GetGraphics(), pGpPen, (PointF*)m_p.GetData(), (int)m_p.GetSize());

	GdipDeletePen(pGpPen);

}

//////////////////////////////////////////////////////////////////////////
// CXTPChartSplineDeviceCommand

CXTPChartSplineDeviceCommand::CXTPChartSplineDeviceCommand(const CXTPChartPoints& p, int thickness)
{
	m_p.Copy(p);
	m_nThickness = thickness;
};

CXTPChartElement* CXTPChartSplineDeviceCommand::HitTest(CPoint point, CXTPChartElement* pParent) const
{
	GpPath* pGpPath;
	GdipCreatePath(FillModeAlternate, &pGpPath);
	GdipAddPathCurve(pGpPath, (PointF*)m_p.GetData(), (int)m_p.GetSize());

	GpPen* pGpPen = NULL;
	GdipCreatePen1(CXTPChartColor::Black, (float)m_nThickness, UnitWorld, &pGpPen);

	BOOL bResult = FALSE;
	GdipIsOutlineVisiblePathPointI(pGpPath, point.x, point.y, pGpPen, 0, &bResult);

	GdipDeletePen(pGpPen);

	GdipDeletePath(pGpPath);

	if (bResult)
		return pParent;

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartSolidSplineDeviceCommand

CXTPChartSolidSplineDeviceCommand::CXTPChartSolidSplineDeviceCommand(const CXTPChartPoints& p, const CXTPChartColor& color, int thickness)
	: CXTPChartSplineDeviceCommand(p, thickness)
{
	m_color = color;
};


void CXTPChartSolidSplineDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GpPen* pGpPen = NULL;
	GdipCreatePen1(m_color.GetValue(), (float)m_nThickness, UnitWorld, &pGpPen);

	GdipDrawCurve(pDC->GetGraphics(), pGpPen, (PointF*)m_p.GetData(), (int)m_p.GetSize());


	GdipDeletePen(pGpPen);

}

void CXTPChartSolidSplineDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* /*pDC*/)
{

}

//////////////////////////////////////////////////////////////////////////
// CXTPChartDashedSplineDeviceCommand

CXTPChartDashedSplineDeviceCommand::CXTPChartDashedSplineDeviceCommand(const CXTPChartPoints& p, const CXTPChartColor& color, int thickness, XTPChartDashStyle nDashStyle)
	: CXTPChartSplineDeviceCommand(p, thickness)
{
	m_color = color;
	m_nDashStyle = nDashStyle;
};


void CXTPChartDashedSplineDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GpPen* pGpPen = NULL;
	GdipCreatePen1(m_color.GetValue(), (REAL)m_nThickness, UnitWorld, &pGpPen);

	GdipSetPenDashStyle(pGpPen,
		m_nDashStyle == xtpChartDashStyleDash ? DashStyleDash :
		m_nDashStyle == xtpChartDashStyleDashDot ? DashStyleDashDot :
		m_nDashStyle == xtpChartDashStyleDot ? DashStyleDot :
		m_nDashStyle == xtpChartDashStyleDashDotDot ? DashStyleDashDotDot : DashStyleSolid);


	GdipDrawCurve(pDC->GetGraphics(), pGpPen, (PointF*)m_p.GetData(), (int)m_p.GetSize());


	GdipDeletePen(pGpPen);

}

void CXTPChartDashedSplineDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* /*pDC*/)
{

}
