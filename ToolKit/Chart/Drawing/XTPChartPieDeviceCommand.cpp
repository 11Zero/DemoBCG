// XTPChartPieDeviceCommand.cpp
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

#include "GraphicLibrary/GdiPlus/GdiPlus.h"
#include "GraphicLibrary/OpenGL/GL.h"

#include "../Utils/XTPChartMathUtils.h"
#include "XTPChartDeviceContext.h"
#include "XTPChartDeviceCommand.h"

#include "../Types/XTPChartPie.h"
#include "XTPChartPieDeviceCommand.h"

#include <math.h>

using namespace Gdiplus;
using namespace Gdiplus::DllExports;


//////////////////////////////////////////////////////////////////////////
// CXTPChartPieDeviceCommand

CXTPChartPieDeviceCommand::CXTPChartPieDeviceCommand(const CXTPChartDiagramPoint& center, double dMajorSemiAxis, double dMinorSemiAxis, double dStartAngle, double dSweepAngle, double depth, int nHolePercent)
{
	m_ptCenter = center;
	m_dMajorSemiAxis = dMajorSemiAxis;
	m_dMinorSemiAxis = dMinorSemiAxis;
	m_dStartAngle = dStartAngle;
	m_dSweepAngle = dSweepAngle;
	m_dDepth = depth;
	m_dHolePercent = nHolePercent / 100.0;

	m_dFacetSize = 0;

	m_dInnerMajorSemiAxis = 0;
	m_dInnerMinorSemiAxis = 0;

	m_dCorrectedDepth = 0;

	m_dInnerRadius = 0;

	m_bDoughnut = nHolePercent > 0;

	if (m_bDoughnut)
	{
		double doughnutThickness = dMinorSemiAxis * (1.0 - m_dHolePercent);
		m_dInnerMajorSemiAxis = dMajorSemiAxis - doughnutThickness;
		m_dInnerMinorSemiAxis = dMinorSemiAxis - doughnutThickness;
	}

	if (depth > 0.0f)
	{
		m_dFacetSize = dMajorSemiAxis * 0.02f;
		m_dFacetSize = 0;

		if (m_dFacetSize > depth / 2.0f)
			m_dFacetSize = depth / 2.0f;
		m_dRadius = dMajorSemiAxis - m_dFacetSize;
		float diff = float(m_dRadius - m_dInnerMajorSemiAxis);
		if (diff < 0.0f) {
			m_dRadius -= diff;
			m_dFacetSize += diff;
		}

		m_dCorrectedDepth = depth - m_dFacetSize * 2;

		if (m_bDoughnut) {
			m_dInnerRadius = m_dInnerMajorSemiAxis;
			if (m_dInnerRadius > m_dRadius)
				m_dInnerRadius = m_dRadius;
		}

	}


}

//////////////////////////////////////////////////////////////////////////
// CXTPChartGradientPieDeviceCommand

CXTPChartGradientPieDeviceCommand::CXTPChartGradientPieDeviceCommand(const CXTPChartDiagramPoint& center, double dMajorSemiAxis, double dMinorSemiAxis,
	double dStartAngle, double dSweepAngle, double depth, int holePercent, const CXTPChartRectF& gradientBounds, const CXTPChartColor& color, const CXTPChartColor& color2)
	: CXTPChartPieDeviceCommand(center, dMajorSemiAxis, dMinorSemiAxis, dStartAngle, dSweepAngle, depth, holePercent)
{
	m_rcGradientBounds = gradientBounds;
	m_color = color;
	m_color2 = color2;
}

void CXTPChartPieDeviceCommand::CalculateStartFinishPoints(const CXTPChartDiagramPoint& center, double majorSemiaxis, double minorSemiaxis, double dStartAngle, double dSweepAngle, CXTPChartPointF& startPoint, CXTPChartPointF& finishPoint) const
{
	CXTPChartEllipse ellipse(center, majorSemiaxis, minorSemiaxis);
	startPoint = (CXTPChartPointF)ellipse.CalcEllipsePoint(-CXTPChartMathUtils::Degree2Radian(dStartAngle));
	finishPoint = (CXTPChartPointF)ellipse.CalcEllipsePoint(-CXTPChartMathUtils::Degree2Radian(dStartAngle + dSweepAngle));
}

CXTPChartElement* CXTPChartPieDeviceCommand::HitTest(CPoint point, CXTPChartElement* pParent) const
{
	GpPath* pGpPath = CreatePieGraphicsPath(m_ptCenter, m_dMajorSemiAxis, m_dMinorSemiAxis, m_dHolePercent, m_dStartAngle, m_dSweepAngle);

	BOOL bResult = FALSE;

	GdipIsVisiblePathPointI(pGpPath, point.x, point.y, NULL, &bResult);

	GdipDeletePath(pGpPath);

	return bResult ? pParent : NULL;
}

GpPath* CXTPChartPieDeviceCommand::CreatePieGraphicsPath(const CXTPChartDiagramPoint& center, double majorSemiaxis, double minorSemiaxis, double holePercent, double dStartAngle, double dSweepAngle)  const
{
	GpPath* pGpPath = NULL;
	GdipCreatePath(FillModeAlternate, &pGpPath);

	CRect rect(int(center.X - majorSemiaxis), int(center.Y - minorSemiaxis), int(center.X + majorSemiaxis), int(center.Y + minorSemiaxis));

	BOOL bShouldAddLines = CXTPChartMathUtils::Compare(dSweepAngle, -360.0) != 0;
	float dInnerMajorSemiaxis = (float)(majorSemiaxis * holePercent);
	float dInnerMinorSemiaxis = (float)(minorSemiaxis * holePercent);
	if (dInnerMajorSemiaxis >= 1.0f && dInnerMinorSemiaxis >= 1.0f)
	{
		CRect innerRect(int(center.X - dInnerMajorSemiaxis),  int(center.Y - dInnerMinorSemiaxis), int(center.X + dInnerMajorSemiaxis), int(center.Y + dInnerMinorSemiaxis));

		CXTPChartPointF startPoint, finishPoint, innerStartPoint, innerFinishPoint;

		if (bShouldAddLines)
		{
			CalculateStartFinishPoints(center, majorSemiaxis, minorSemiaxis, dStartAngle, dSweepAngle, startPoint, finishPoint);
			CalculateStartFinishPoints(center, dInnerMajorSemiaxis, dInnerMinorSemiaxis, dStartAngle, dSweepAngle, innerStartPoint, innerFinishPoint);
		}

		GdipAddPathArc(pGpPath, (REAL)rect.left, (REAL)rect.top, (REAL)rect.Width(), (REAL)rect.Height(), (REAL)dStartAngle, (REAL)dSweepAngle);

		if (bShouldAddLines)
		{
			GdipAddPathLine(pGpPath, finishPoint.X, finishPoint.Y, innerFinishPoint.X, innerFinishPoint.Y);
		}
		GdipAddPathArc(pGpPath, (REAL)innerRect.left, (REAL)innerRect.top, (REAL)innerRect.Width(), (REAL)innerRect.Height(), (REAL)(dStartAngle + dSweepAngle), (REAL)(-dSweepAngle));

		if (bShouldAddLines)
			GdipAddPathLine(pGpPath, innerStartPoint.X, innerStartPoint.Y, startPoint.X, startPoint.Y);

	}
	else if (bShouldAddLines)
	{
		GdipAddPathPie(pGpPath, (REAL)rect.left, (REAL)rect.top, (REAL)rect.Width(), (REAL)rect.Height(), (REAL)dStartAngle, (REAL)dSweepAngle);

	}
	else
	{
		GdipAddPathEllipse(pGpPath, (REAL)rect.left, (REAL)rect.top, (REAL)rect.Width(), (REAL)rect.Height());

	}

	return pGpPath;
}

GpBrush* CXTPChartGradientPieDeviceCommand::CreateBrush()
{
	GpLineGradient* pGpBrush = NULL;

	PointF ptGradient[2];
	ptGradient[0] = PointF((REAL)m_rcGradientBounds.X, (REAL)m_ptCenter.Y);
	ptGradient[1] = PointF((REAL)(m_rcGradientBounds.X + m_rcGradientBounds.Width), (REAL)m_ptCenter.Y);


	GdipCreateLineBrush(&ptGradient[0], &ptGradient[1], m_color.GetValue(), m_color2.GetValue(), WrapModeTile, &pGpBrush);

	return pGpBrush;
}

void CXTPChartGradientPieDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{

	GpBrush* pGpBrush = CreateBrush();


	GpPath* pGpPath = CreatePieGraphicsPath(m_ptCenter, m_dMajorSemiAxis, m_dMinorSemiAxis, m_dHolePercent, m_dStartAngle, m_dSweepAngle);

	GdipFillPath(pDC->GetGraphics(), pGpBrush, pGpPath);

	GdipDeletePath(pGpPath);

	GdipDeleteBrush(pGpBrush);
}

void CXTPChartGradientPieDeviceCommand::PartialDisk(float dRadius, float dStartAngle, float dSweepAngle)
{
	float dRadianStartAngle = (float)(dStartAngle * CXTPChartMathUtils::PI) / 180.0f;
	int nSlicesCount = abs(int(dSweepAngle));
	float dRadianSweepAngle = (float)(dSweepAngle * CXTPChartMathUtils::PI) / 180.0f / nSlicesCount;

	glNormal3d(0.0, 0.0, 1.0);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < nSlicesCount; i++)
	{
		double angle = i * dRadianSweepAngle + dRadianStartAngle;
		double x1 = cos(angle) * dRadius;
		double y1 = sin(angle) * dRadius;
		angle = angle + dRadianSweepAngle;
		double x2 = cos(angle) * dRadius;
		double y2 = sin(angle) * dRadius;

		glColorVertex(0.0, 0.0, 0.0);

		glColorVertex(x1, y1, 0.0);

		glColorVertex(x2, y2, 0.0);
	}
	glEnd();

}

void CXTPChartGradientPieDeviceCommand::PartialDisk(float dInnerRadius, float dOuterRadius, float dStartAngle, float dSweepAngle)
{
	float radianStartAngle = (float)(dStartAngle * CXTPChartMathUtils::PI) / 180.0f;
	int slicesCount = abs(int(dSweepAngle));;
	float radianSweepAngle = (float)(dSweepAngle * CXTPChartMathUtils::PI) / 180.0f / slicesCount;
	glNormal3d(0.0, 0.0, 1.0);
	glBegin(GL_QUADS);
	for (int i = 0; i < slicesCount; i++)
	{
		double angle = i * radianSweepAngle + radianStartAngle;
		double cos1 = cos(angle);
		double sin1 = sin(angle);
		double ox1 = cos1 * dOuterRadius;
		double oy1 = sin1 * dOuterRadius;
		double ix1 = cos1 * dInnerRadius;
		double iy1 = sin1 * dInnerRadius;
		angle = angle + radianSweepAngle;
		cos1 = cos(angle);
		sin1 = sin(angle);
		double ox2 = cos1 * dOuterRadius;
		double oy2 = sin1 * dOuterRadius;
		double ix2 = cos1 * dInnerRadius;
		double iy2 = sin1 * dInnerRadius;

		glColorVertex(ix2, iy2, 0.0);

		glColorVertex(ix1, iy1, 0.0);

		glColorVertex(ox1, oy1, 0.0);

		glColorVertex(ox2, oy2, 0.0);
	}
	glEnd();
}


void CXTPChartGradientPieDeviceCommand::SetColor(double x, double y, double z)
{
	UNREFERENCED_PARAMETER(y);
	UNREFERENCED_PARAMETER(z);

	float coord;
	CXTPChartRectF  relativeBounds = m_rcGradientBounds;

	relativeBounds.Offset(-(float)m_ptCenter.X, -(float)m_ptCenter.Y);

	coord = (float)(-(relativeBounds.X - x) / relativeBounds.Width);

	if (coord > 1) coord = 1; else if (coord < 0) coord = 0;
	coord /= 255.0f;

	glColor4f((m_color.GetR() * coord + m_color2.GetR() * (1.0f/255.0f - coord)),
		(m_color.GetG() * coord + m_color2.GetG() * (1.0f/255.0f - coord)),
		(m_color.GetB() * coord + m_color2.GetB() * (1.0f/255.0f - coord)),
		(m_color.GetA() * coord + m_color2.GetA() * (1.0f/255.0f - coord)));

}

void CXTPChartGradientPieDeviceCommand::PieSections(float dHeight, float dInnerRadius, float dOuterRadius,
													float dFacetSize, float dStartAngle, float dEndAngle)
{
	UNREFERENCED_PARAMETER(dFacetSize);

	double dRadianStartAngle = dStartAngle * CXTPChartMathUtils::PI / 180.0;
	double dRadianEndAngle = dEndAngle * CXTPChartMathUtils::PI / 180.0;
	double dStartAngleCos = cos(dRadianStartAngle);
	double dStartAngleSin = sin(dRadianStartAngle);
	double dEndAngleCos = cos(dRadianEndAngle);
	double dEndAngleSin = sin(dRadianEndAngle);

	CXTPChartDiagramPoint innerStart(dStartAngleCos * dInnerRadius, dStartAngleSin * dInnerRadius);
	CXTPChartDiagramPoint innerEnd(dEndAngleCos * dInnerRadius, dEndAngleSin * dInnerRadius);
	CXTPChartDiagramPoint outerStart(dStartAngleCos * dOuterRadius, dStartAngleSin * dOuterRadius);
	CXTPChartDiagramPoint outerEnd(dEndAngleCos * dOuterRadius, dEndAngleSin * dOuterRadius);
	CXTPChartDiagramPoint bottomCenter;
	CXTPChartDiagramPoint topCenter(0.0, 0.0, dHeight);
	CXTPChartDiagramPoint startNormal = CXTPChartDiagramPoint::CalcNormal(bottomCenter, topCenter, outerStart);
	CXTPChartDiagramPoint endNormal = CXTPChartDiagramPoint::CalcNormal(bottomCenter, topCenter, outerEnd);

	glBegin(GL_QUADS);
	glNormal3d(startNormal.X, startNormal.Y, startNormal.Z);

	SetColor(innerStart.X, innerStart.Y, 0.0);
	glVertex3d(innerStart.X, innerStart.Y, 0.0);
	glVertex3d(innerStart.X, innerStart.Y, dHeight);

	SetColor(outerStart.X, outerStart.Y, 0.0);
	glVertex3d(outerStart.X, outerStart.Y, dHeight);
	glVertex3d(outerStart.X, outerStart.Y, 0.0);

	glNormal3d(endNormal.X, endNormal.Y, endNormal.Z);
	SetColor(innerEnd.X, innerEnd.Y, 0.0);
	glVertex3d(innerEnd.X, innerEnd.Y, 0.0);
	glVertex3d(innerEnd.X, innerEnd.Y, dHeight);

	SetColor(outerEnd.X, outerEnd.Y, 0.0);
	glVertex3d(outerEnd.X, outerEnd.Y, dHeight);
	glVertex3d(outerEnd.X, outerEnd.Y, 0.0);
	glEnd();
}

void CXTPChartGradientPieDeviceCommand::PartialCylinder(float height, float radius, float dStartAngle, float dSweepAngle)
{
	float radianStartAngle = (float)(dStartAngle * CXTPChartMathUtils::PI) / 180.0f;
	int slicesCount = abs((int)dSweepAngle);
	float radianSweepAngle = (float)(dSweepAngle * CXTPChartMathUtils::PI) / 180.0f / slicesCount;
	for (int i = 0; i < slicesCount; i++)
	{
		double angle = i * radianSweepAngle + radianStartAngle;
		float x = (float)cos(angle);
		float y = (float)sin(angle);

		glBegin(GL_QUADS);
		glNormal3d(x, y, 0);
		x *= radius;
		y *= radius;
		SetColor(x, y, 0.0);
		glVertex3d(x, y, height);
		glVertex3d(x, y, 0);

		angle += radianSweepAngle;
		x = (float)cos(angle);
		y = (float)sin(angle);

		glNormal3d(x, y, 0);
		x *= radius;
		y *= radius;
		SetColor(x, y, 0.0);
		glVertex3d(x, y, 0.0);
		glVertex3d(x, y, height);
		glEnd();
	}
}



void CXTPChartGradientPieDeviceCommand::ExecutePie(CXTPChartOpenGLDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	glColor4ub(m_color2.GetR(), m_color2.GetG(), m_color2.GetB(), m_color2.GetA());
	PartialDisk((float)m_dRadius, (float)-m_dStartAngle, (float)-m_dSweepAngle);
}

void CXTPChartGradientPieDeviceCommand::ExecuteDoughnut(CXTPChartOpenGLDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	glColor4ub(m_color2.GetR(), m_color2.GetG(), m_color2.GetB(), m_color2.GetA());
	PartialDisk((float)m_dInnerRadius, (float)m_dRadius, (float)-m_dStartAngle, (float)-m_dSweepAngle);
}


void CXTPChartGradientPieDeviceCommand::ExecuteSections(CXTPChartOpenGLDeviceContext* pDC, float innerRadius, float outerRadius)
{
	UNREFERENCED_PARAMETER(pDC);
	glColor4ub(m_color2.GetR(), m_color2.GetG(), m_color2.GetB(), m_color2.GetA());
	PieSections((float)m_dDepth, innerRadius, outerRadius, (float)m_dFacetSize, (float)-m_dStartAngle, (float)-(m_dStartAngle + m_dSweepAngle));
}

void CXTPChartGradientPieDeviceCommand::ExecuteCylinder(CXTPChartOpenGLDeviceContext* pDC, float radius)
{
	UNREFERENCED_PARAMETER(pDC);
	glColor4ub(m_color2.GetR(), m_color2.GetG(), m_color2.GetB(), m_color2.GetA());
	PartialCylinder((float)m_dCorrectedDepth, radius, (float)-m_dStartAngle, (float)-m_dSweepAngle);
}

void CXTPChartGradientPieDeviceCommand::PerformPieDrawing(CXTPChartOpenGLDeviceContext* pDC)
{

	ExecutePie(pDC);

	if (m_dDepth != 0)
	{
		glPushMatrix();

		glTranslated(0.0, 0.0, m_dDepth);
		ExecutePie(pDC);

		glPopMatrix();
	}

	ExecuteCylinder(pDC, (float)m_dMajorSemiAxis);

	ExecuteSections(pDC, 0.0f, (float)m_dMajorSemiAxis);
}


void CXTPChartGradientPieDeviceCommand::PerformDoughnutDrawing(CXTPChartOpenGLDeviceContext* pDC)
{
	ExecuteDoughnut(pDC);
	if (m_dDepth != 0)
	{
		glPushMatrix();

		glTranslated(0.0, 0.0, m_dDepth);
		ExecuteDoughnut(pDC);

		glPopMatrix();
	}

	ExecuteCylinder(pDC, (float)m_dMajorSemiAxis);
	ExecuteCylinder(pDC, (float)m_dInnerMajorSemiAxis);

	ExecuteSections(pDC, (float)m_dInnerMajorSemiAxis, (float)m_dMajorSemiAxis);
}

void CXTPChartGradientPieDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslated(m_ptCenter.X, -m_ptCenter.Y, 0);

	if (m_bDoughnut)
	{
		PerformDoughnutDrawing(pDC);
	}
	else
	{

		PerformPieDrawing(pDC);
	}


	glPopMatrix();
}

void CXTPChartGradientPieDeviceCommand::glColorVertex(double x, double y, double z)
{
	SetColor(x, y, z);
	glVertex3d(x, y, z);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartGradientTorusDeviceCommand

CXTPChartGradientTorusDeviceCommand::CXTPChartGradientTorusDeviceCommand(const CXTPChartDiagramPoint& center, double dMajorSemiAxis, double dMinorSemiAxis, double dStartAngle, double dSweepAngle, double depth, int holePercent, const CXTPChartRectF& gradientBounds, const CXTPChartColor& color, const CXTPChartColor& color2)
	: CXTPChartGradientPieDeviceCommand(center, dMajorSemiAxis, dMinorSemiAxis, dStartAngle, dSweepAngle, depth, holePercent, gradientBounds, color, color2)
{

}

void CXTPChartGradientTorusDeviceCommand::PerformTorusDrawing(CXTPChartOpenGLDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);

	glColor4ub(m_color2.GetR(), m_color2.GetG(), m_color2.GetB(), m_color2.GetA());

	double Radius = (m_dInnerMajorSemiAxis + m_dMajorSemiAxis) / 2.0;
	double TubeRadius = (m_dMajorSemiAxis - m_dInnerMajorSemiAxis) / 2.0;

	glTranslated(0.0, 0.0, m_dDepth /2);

	int Sides = 40;
	double PI = CXTPChartMathUtils::PI;

	float radianStartAngle = (float)(m_dStartAngle * CXTPChartMathUtils::PI) / 180.0f;
	int slicesCount = abs((int)m_dSweepAngle);
	float radianSweepAngle = (float)(m_dSweepAngle * CXTPChartMathUtils::PI) / 180.0f / slicesCount;

	double theta = radianStartAngle;
	double cosTheta = cos(theta);
	double sinTheta = sin(theta);

	double sideDelta = 2.0 * PI / Sides;

	for (int i = 0; i < slicesCount; i++)
	{
		double theta1 = theta + radianSweepAngle;

		double cosTheta1 = cos(theta1);
		double sinTheta1 = sin(theta1);

		glBegin(GL_QUAD_STRIP);

		double phi = 0.0;
		for (int j = Sides; j >= 0; j--)
		{

			phi = phi + sideDelta;
			double cosPhi = cos(phi);
			double sinPhi = sin(phi);
			double dist = Radius + (TubeRadius * cosPhi);

			glNormal3f((float)(-cosTheta1 * cosPhi), (float)(sinTheta1 * cosPhi), (float)-sinPhi);
			glColorVertex(cosTheta1 * dist, -sinTheta1 * dist, TubeRadius * sinPhi);

			glNormal3f((float)(-cosTheta * cosPhi), (float)(sinTheta * cosPhi), (float)-sinPhi);
			glColorVertex(cosTheta * dist, -sinTheta * dist, TubeRadius * sinPhi);
		}
		glEnd();

		theta = theta1;
		cosTheta = cosTheta1;
		sinTheta = sinTheta1;
	}

	{

		theta = (float)((m_dStartAngle + m_dSweepAngle) * CXTPChartMathUtils::PI) / 180.0f;;

		double cosTheta = cos(theta);
		double sinTheta = sin(theta);

		glNormal3f((float)(sinTheta), (float)(-cosTheta), 0);
		glBegin(GL_POLYGON);

		double phi = 0.0;

		for (int j = Sides; j >= 0; j--)
		{
			phi = phi + sideDelta;
			double cosPhi = cos(phi);
			double sinPhi = sin(phi);
			double dist = Radius + (TubeRadius * cosPhi);

			glColorVertex(cosTheta * dist, -sinTheta * dist, TubeRadius * sinPhi);
		}

		glEnd();
	}

	{

		theta = radianStartAngle;

		double cosTheta = cos(theta);
		double sinTheta = sin(theta);

		glNormal3f((float)(-sinTheta), (float)(cosTheta), 0);
		glBegin(GL_POLYGON);

		double phi = 0.0;

		for (int j = Sides; j >= 0; j--)
		{
			phi = phi + sideDelta;
			double cosPhi = cos(phi);
			double sinPhi = sin(phi);
			double dist = Radius + (TubeRadius * cosPhi);

			glColorVertex(cosTheta * dist, -sinTheta * dist, TubeRadius * sinPhi);
		}

		glEnd();
	}

}


void CXTPChartGradientTorusDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glTranslated(m_ptCenter.X, -m_ptCenter.Y, 0);

	PerformTorusDrawing(pDC);

	glPopMatrix();
}



//////////////////////////////////////////////////////////////////////////
// CXTPChartBoundedPieDeviceCommand


CXTPChartBoundedPieDeviceCommand::CXTPChartBoundedPieDeviceCommand(const CXTPChartDiagramPoint& center, double dMajorSemiAxis, double dMinorSemiAxis, double dStartAngle, double dSweepAngle, double depth, int holePercent, const CXTPChartColor& color, int nThickness)
	: CXTPChartPieDeviceCommand(center, dMajorSemiAxis, dMinorSemiAxis, dStartAngle, dSweepAngle, depth, holePercent)
{
	m_color = color;
	m_nThickness = nThickness;
}

void CXTPChartBoundedPieDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{

	GpPen* pGpPen = NULL;
	GdipCreatePen1(m_color.GetValue(), (REAL)m_nThickness, UnitWorld, &pGpPen);


	GpPath* pGpPath = CreatePieGraphicsPath(m_ptCenter, m_dMajorSemiAxis, m_dMinorSemiAxis, m_dHolePercent, m_dStartAngle, m_dSweepAngle);

	GdipDrawPath(pDC->GetGraphics(), pGpPen, pGpPath);

	GdipDeletePath(pGpPath);

	GdipDeletePen(pGpPen);
}
