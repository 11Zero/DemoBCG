// XTPChartPieDeviceCommand.h
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
#if !defined(__XTPCHARTPIEDEVICECOMMAND_H__)
#define __XTPCHARTPIEDEVICECOMMAND_H__
//}}AFX_CODEJOCK_PRIVATE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "../Types/XTPChartTypes.h"
#include "../Types/XTPChartDiagramPoint.h"
#include "XTPChartDeviceCommand.h"
#include "XTPChartOpenGLHelpers.h"

namespace Gdiplus
{
	class GpPath;
	class GpBrush;
};

//===========================================================================
// Summary:
//     This class represents a chart pie device command,which is a kind of
//     CXTPChartDeviceCommand.It specifically handles the rendering of a pie
//     in a chart.
// Remarks:
//===========================================================================
class _XTP_EXT_CLASS CXTPChartPieDeviceCommand : public CXTPChartDeviceCommand
{
public:
	//-----------------------------------------------------------------------
	// Summary:
	//     Constructs a CXTPChartPieDeviceCommand object.
	// Parameters:
	//     ptCenter        - The center point of the pie.
	//     dMajorSemiAxis - The the major semi axis, of the ellipse, which comes
	//                     into picture when a 3D pie diagram is drawn.
	//     dMinorSemiAxis - The the minor semi axis, of the ellipse, which comes
	//                     into picture when a 3D pie diagram is drawn.
	//     dStartAngle    - The start angle of the pie.
	//     nMode     - The linear gradient mode, it can be horizontal, vertical,
	//                 forward diagonal or backward diagonal.
	// Remarks:
	//-----------------------------------------------------------------------
	CXTPChartPieDeviceCommand(const CXTPChartDiagramPoint& ptCenter, double dMajorSemiAxis, double dMinorSemiAxis, double dStartAngle, double dSweepAngle,
		double dDepth, int nHolePercent);

protected:
	Gdiplus::GpPath* CreatePieGraphicsPath(const CXTPChartDiagramPoint& center, double majorSemiaxis, double minorSemiaxis, double holePercent, double startAngle, double sweepAngle) const;
	void CalculateStartFinishPoints(const CXTPChartDiagramPoint& center, double majorSemiaxis, double minorSemiaxis, double startAngle, double sweepAngle, CXTPChartPointF& startPoint, CXTPChartPointF& finishPoint) const;

	virtual CXTPChartElement* HitTest(CPoint point, CXTPChartElement* pParent) const;

protected:
	CXTPChartDiagramPoint m_ptCenter;
	double m_dMajorSemiAxis;
	double m_dMinorSemiAxis;
	double m_dStartAngle;
	double m_dSweepAngle;
	double m_dDepth;
	double m_dHolePercent;

	double m_dInnerMajorSemiAxis;
	double m_dInnerMinorSemiAxis;

	double m_dFacetSize;
	double m_dRadius;
	double m_dCorrectedDepth;
	double m_dInnerRadius;

	BOOL m_bDoughnut;

};

class _XTP_EXT_CLASS CXTPChartGradientPieDeviceCommand : public CXTPChartPieDeviceCommand
{
public:
	CXTPChartGradientPieDeviceCommand(const CXTPChartDiagramPoint& center, double majorSemiAxis, double minorSemiAxis, double startAngle, double sweepAngle,
		double depth, int holePercent, const CXTPChartRectF& gradientBounds, const CXTPChartColor& color, const CXTPChartColor& color2);

protected:
	void ExecuteOverride(CXTPChartDeviceContext* pDC);
	void ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC);

	Gdiplus::GpBrush* CreateBrush();

protected:
	void PerformPieDrawing(CXTPChartOpenGLDeviceContext* pDC);
	void PerformDoughnutDrawing(CXTPChartOpenGLDeviceContext* pDC);

	void ExecutePie(CXTPChartOpenGLDeviceContext* pDC);
	void ExecuteDoughnut(CXTPChartOpenGLDeviceContext* pDC);
	void ExecuteSections(CXTPChartOpenGLDeviceContext* pDC, float sInnerRadius, float dOuterRadius);
	void ExecuteCylinder(CXTPChartOpenGLDeviceContext* pDC, float dRadius);

protected:
	void PartialDisk(float dRadius, float dStartAngle, float dSweepAngle);
	void PartialDisk(float dInnerRadius, float dOuterRadius, float dStartAngle, float dSweepAngle);
	void PieSections(float dHeight, float dInnerRadius, float dOuterRadius, float dFacetSize, float dStartAngle, float dEndAngle);
	void SetColor(double x, double y, double z);
	void PartialCylinder(float dHeight, float dRadius, float dStartAngle, float dSweepAngle);

	void glColorVertex(double x, double y, double z);
protected:
	CXTPChartRectF m_rcGradientBounds;
	CXTPChartColor m_color;
	CXTPChartColor m_color2;
};

class _XTP_EXT_CLASS CXTPChartGradientTorusDeviceCommand : public CXTPChartGradientPieDeviceCommand
{
public:
	CXTPChartGradientTorusDeviceCommand(const CXTPChartDiagramPoint& center, double majorSemiAxis, double minorSemiAxis, double startAngle, double sweepAngle,
		double depth, int holePercent, const CXTPChartRectF& gradientBounds, const CXTPChartColor& color, const CXTPChartColor& color2);

protected:
	void ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC);
	void PerformTorusDrawing(CXTPChartOpenGLDeviceContext* pDC);
};

class _XTP_EXT_CLASS CXTPChartBoundedPieDeviceCommand : public CXTPChartPieDeviceCommand
{
public:
	CXTPChartBoundedPieDeviceCommand(const CXTPChartDiagramPoint& center, double majorSemiAxis, double minorSemiAxis, double startAngle, double sweepAngle,
		double depth, int holePercent, const CXTPChartColor& color, int nThickness);

protected:
	void ExecuteOverride(CXTPChartDeviceContext* pDC);

protected:
	CXTPChartColor m_color;
	int m_nThickness;
};

#endif //#if !defined(__XTPCHARTPIEDEVICECOMMAND_H__)
