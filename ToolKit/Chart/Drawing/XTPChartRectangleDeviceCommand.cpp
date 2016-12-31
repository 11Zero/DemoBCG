// XTPChartRectangleDeviceCommand.cpp
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
#include "Common/XTPVC80Helpers.h"

#include "XTPChartRectangleDeviceCommand.h"
#include "XTPChartDeviceContext.h"
#include "XTPChartOpenGLDeviceContext.h"


using namespace Gdiplus;
using namespace Gdiplus::DllExports;

//////////////////////////////////////////////////////////////////////////
// CXTPChartBoundedRectangleDeviceCommand

CXTPChartBoundedRectangleDeviceCommand::CXTPChartBoundedRectangleDeviceCommand(const CXTPChartRectF& rect, const CXTPChartColor& color, int thickness)
{
	m_rect = rect;
	m_color = color;
	m_thickness = thickness;
}


void CXTPChartBoundedRectangleDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GpPen* pGpPen = NULL;
	GdipCreatePen1(m_color.GetValue(), (REAL)m_thickness, UnitWorld, &pGpPen);

	GdipDrawRectangle(pDC->GetGraphics(), pGpPen, m_rect.X, m_rect.Y, m_rect.Width, m_rect.Height);


	GdipDeletePen(pGpPen);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartInsideBorderDeviceCommand

CXTPChartInnerBorderDeviceCommand::CXTPChartInnerBorderDeviceCommand(const CXTPChartRectF& rect, const CXTPChartColor& color, int thickness)
	: CXTPChartBoundedRectangleDeviceCommand(rect, color, thickness)
{
	m_rect.X += thickness / 2.0f - 0.5f;
	m_rect.Y += thickness / 2.0f - 0.5f;

	m_rect.Width -= thickness - 0.5f;
	m_rect.Height -= thickness - 0.5f;
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartSolidRectangleDeviceCommand

CXTPChartSolidRectangleDeviceCommand::CXTPChartSolidRectangleDeviceCommand(const CXTPChartRectF& rect, const CXTPChartColor& color)
{
	m_rect = rect;
	m_color = color;
}

void CXTPChartSolidRectangleDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GpSolidFill* pGpBrush = NULL;
	GdipCreateSolidFill(m_color.GetValue(), &pGpBrush);

	GdipFillRectangle(pDC->GetGraphics(), pGpBrush, m_rect.X, m_rect.Y, m_rect.Width, m_rect.Height);


	GdipDeleteBrush(pGpBrush);
}

CXTPChartElement* CXTPChartSolidRectangleDeviceCommand::HitTest(CPoint point, CXTPChartElement* pParent) const
{
	if (m_rect.Contains((REAL)point.x, (REAL)point.y))
		return pParent;
	return NULL;
}

#ifdef _XTP_DEMOMODE

//////////////////////////////////////////////////////////////////////////
// CXTPChartWatermarkBackgroundDeviceCommand

CXTPChartWatermarkBackgroundDeviceCommand::CXTPChartWatermarkBackgroundDeviceCommand(const CXTPChartRectF& rect)
	: m_rect(rect)
{

}

void CXTPChartWatermarkBackgroundDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GdipSetTextRenderingHint(pDC->GetGraphics(), TextRenderingHintAntiAlias);

	GpSolidFill* pGpBrush = NULL;
	CXTPChartColor clrText(100, 102, 102, 102);
	GdipCreateSolidFill(clrText.GetValue(), &pGpBrush);

	GpFont* pGpFont = NULL;

	LOGFONT lf = {0};
	lf.lfHeight = LONG(m_rect.Width * 80 / 1000);

	lf.lfWeight = FW_BOLD;
	STRCPY_S(lf.lfFaceName, LF_FACESIZE, _T("Myraid Pro"));

	GdipCreateFontFromLogfont(pDC->GetHDC(), &lf, &pGpFont);


	GpStringFormat* pGpStringFormat = NULL;
	GdipCreateStringFormat(0, 0, &pGpStringFormat);

	GdipSetStringFormatLineAlign(pGpStringFormat, StringAlignmentCenter);
	GdipSetStringFormatAlign(pGpStringFormat, StringAlignmentCenter);

	GdipDrawString(pDC->GetGraphics(), L"Codejock Chart Pro Trial", -1, pGpFont, (RectF*)&m_rect, pGpStringFormat, pGpBrush);

	GdipDeleteFont(pGpFont);
	GdipDeleteBrush(pGpBrush);
	GdipDeleteStringFormat(pGpStringFormat);

}
#endif


//////////////////////////////////////////////////////////////////////////
// CXTPChartContentBackgroundDeviceCommand


CXTPChartContentBackgroundDeviceCommand::CXTPChartContentBackgroundDeviceCommand(const CXTPChartRectF& rect, const CXTPChartColor& color)
	: CXTPChartSolidRectangleDeviceCommand(rect, color)
{

}

void CXTPChartContentBackgroundDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	CXTPChartSolidRectangleDeviceCommand::ExecuteOverride(pDC);
}

void CXTPChartContentBackgroundDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* /*pDC*/)
{
	glClearColor(m_color.GetRed()/255.0f, m_color.GetGreen()/255.0f, m_color.GetBlue()/255.0f, 1.0f);

	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_SMOOTH);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartGradientRectangleDeviceCommand

CXTPChartGradientRectangleDeviceCommand::CXTPChartGradientRectangleDeviceCommand(const CXTPChartRectF& rect, const CXTPChartColor& color, const CXTPChartColor& color2, XTPChartLinearGradientMode nMode)
{
	m_rect = rect;
	m_color = color;
	m_color2 = color2;
	m_nMode = nMode;
}

void CXTPChartGradientRectangleDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GpLineGradient* pGpBrush = NULL;
	if (m_nMode == xtpChartLinearGradientModeCenterHorizontal)
	{
		GdipCreateLineBrushFromRect((RectF*)&m_rect, m_color.GetValue(), m_color.GetValue(), LinearGradientModeHorizontal, WrapModeTileFlipXY, &pGpBrush);

		Gdiplus::ARGB blend[] = {m_color.GetValue(), m_color2.GetValue(), m_color2.GetValue(), m_color.GetValue()};
		REAL positions[] ={0, 0.45f, 0.55f, 1};
		GdipSetLinePresetBlend(pGpBrush, blend, positions, 4);
	}
	else if (m_nMode == xtpChartLinearGradientModeCenterVertical)
	{
		GdipCreateLineBrushFromRect((RectF*)&m_rect, m_color.GetValue(), m_color.GetValue(), LinearGradientModeVertical, WrapModeTileFlipXY, &pGpBrush);

		Gdiplus::ARGB blend[] = {m_color.GetValue(), m_color2.GetValue(), m_color2.GetValue(), m_color.GetValue()};
		REAL positions[] ={0, 0.45f, 0.55f, 1};
		GdipSetLinePresetBlend(pGpBrush, blend, positions, 4);
	}
	else
	{
		GdipCreateLineBrushFromRect((RectF*)&m_rect, m_color.GetValue(), m_color2.GetValue(), (LinearGradientMode)m_nMode, WrapModeTileFlipXY, &pGpBrush);
	}

	GdipFillRectangle(pDC->GetGraphics(), pGpBrush, m_rect.X, m_rect.Y, m_rect.Width, m_rect.Height);


	GdipDeleteBrush(pGpBrush);
}

CXTPChartElement* CXTPChartGradientRectangleDeviceCommand::HitTest(CPoint point, CXTPChartElement* pParent) const
{
	if (m_rect.Contains((REAL)point.x, (REAL)point.y))
		return pParent;
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartHatchRectangleDeviceCommand

CXTPChartHatchRectangleDeviceCommand::CXTPChartHatchRectangleDeviceCommand(const CXTPChartRectF& rect, const CXTPChartColor& color, const CXTPChartColor& color2, XTPChartHatchStyle nStyle)
{
	m_rect = rect;
	m_color = color;
	m_color2 = color2;
	m_nStyle = nStyle;
}

void CXTPChartHatchRectangleDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GpHatch* pGpBrush = NULL;
	GdipCreateHatchBrush((GpHatchStyle)m_nStyle, m_color.GetValue(), m_color2.GetValue(), &pGpBrush);

	GdipFillRectangle(pDC->GetGraphics(), pGpBrush, m_rect.X, m_rect.Y, m_rect.Width, m_rect.Height);

	GdipDeleteBrush(pGpBrush);
}

CXTPChartElement* CXTPChartHatchRectangleDeviceCommand::HitTest(CPoint point, CXTPChartElement* pParent) const
{
	if (m_rect.Contains((REAL)point.x, (REAL)point.y))
		return pParent;
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartCircleDeviceCommand

CXTPChartCircleDeviceCommand::CXTPChartCircleDeviceCommand(const CXTPChartPointF& center, double radius)
{
	m_center = center;
	m_radius = radius;
}


CXTPChartElement* CXTPChartCircleDeviceCommand::HitTest(CPoint point, CXTPChartElement* pParent) const
{
	if (point.x < m_center.X - m_radius || point.x > m_center.X + m_radius)
		return NULL;

	if (point.y < m_center.Y - m_radius || point.y > m_center.Y + m_radius)
		return NULL;

	if ((point.x - m_center.X) * (point.x - m_center.X) + (point.y - m_center.Y) * (point.y - m_center.Y) <= m_radius * m_radius)
		return pParent;

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartSolidCircleDeviceCommand

CXTPChartSolidCircleDeviceCommand::CXTPChartSolidCircleDeviceCommand(const CXTPChartPointF& center, double radius, const CXTPChartColor& color)
	: CXTPChartCircleDeviceCommand(center, radius)
{
	m_color = color;
}

void CXTPChartSolidCircleDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	RectF rect((REAL)(m_center.X - m_radius), (REAL)(m_center.Y - m_radius), (REAL)(m_radius * 2), (REAL)(m_radius * 2));

	GpSolidFill* pGpBrush = NULL;
	GdipCreateSolidFill(m_color.GetValue(), &pGpBrush);

	GdipFillEllipse(pDC->GetGraphics(), pGpBrush, (REAL)(m_center.X - m_radius), (REAL)(m_center.Y - m_radius), (REAL)(m_radius * 2), (REAL)(m_radius * 2));

	GdipDeleteBrush(pGpBrush);
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartGradientCircleDeviceCommand

CXTPChartGradientCircleDeviceCommand::CXTPChartGradientCircleDeviceCommand(const CXTPChartPointF& center, double radius, const CXTPChartColor& color, const CXTPChartColor& color2, XTPChartLinearGradientMode nMode)
	: CXTPChartCircleDeviceCommand(center, radius)
{
	m_color = color;
	m_color2 = color2;
	m_nMode = nMode;
}

void CXTPChartGradientCircleDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	RectF rect((REAL)(m_center.X - m_radius), (REAL)(m_center.Y - m_radius), (REAL)(m_radius * 2), (REAL)(m_radius * 2));

	GpLineGradient* pGpBrush = NULL;
	if (m_nMode == xtpChartLinearGradientModeCenterHorizontal)
	{
		GdipCreateLineBrushFromRect((RectF*)&rect, m_color.GetValue(), m_color.GetValue(), LinearGradientModeHorizontal, WrapModeTileFlipXY, &pGpBrush);

		Gdiplus::ARGB blend[] = {m_color.GetValue(), m_color2.GetValue(), m_color2.GetValue(), m_color.GetValue()};
		REAL positions[] ={0, 0.45f, 0.55f, 1};
		GdipSetLinePresetBlend(pGpBrush, blend, positions, 4);
	}
	else if (m_nMode == xtpChartLinearGradientModeCenterVertical)
	{
		GdipCreateLineBrushFromRect((RectF*)&rect, m_color.GetValue(), m_color.GetValue(), LinearGradientModeVertical, WrapModeTileFlipXY, &pGpBrush);

		Gdiplus::ARGB blend[] = {m_color.GetValue(), m_color2.GetValue(), m_color2.GetValue(), m_color.GetValue()};
		REAL positions[] ={0, 0.45f, 0.55f, 1};
		GdipSetLinePresetBlend(pGpBrush, blend, positions, 4);
	}
	else
	{
		GdipCreateLineBrushFromRect((RectF*)&rect, m_color.GetValue(), m_color2.GetValue(), (LinearGradientMode)m_nMode, WrapModeTileFlipXY, &pGpBrush);
	}

	GdipFillEllipse(pDC->GetGraphics(), pGpBrush, (REAL)(m_center.X - m_radius), (REAL)(m_center.Y - m_radius), (REAL)(m_radius * 2), (REAL)(m_radius * 2));

	GdipDeleteBrush(pGpBrush);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartHatchCircleDeviceCommand

CXTPChartHatchCircleDeviceCommand::CXTPChartHatchCircleDeviceCommand(const CXTPChartPointF& center, double radius, const CXTPChartColor& color, const CXTPChartColor& color2, XTPChartHatchStyle nStyle)
	: CXTPChartCircleDeviceCommand(center, radius)
{
	m_color = color;
	m_color2 = color2;
	m_nStyle = nStyle;
}

void CXTPChartHatchCircleDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	RectF rect((REAL)(m_center.X - m_radius), (REAL)(m_center.Y - m_radius), (REAL)(m_radius * 2), (REAL)(m_radius * 2));

	GpHatch* pGpBrush = NULL;
	GdipCreateHatchBrush((GpHatchStyle)m_nStyle, m_color.GetValue(), m_color2.GetValue(), &pGpBrush);

	GdipFillEllipse(pDC->GetGraphics(), pGpBrush, (REAL)(m_center.X - m_radius), (REAL)(m_center.Y - m_radius), (REAL)(m_radius * 2), (REAL)(m_radius * 2));

	GdipDeleteBrush(pGpBrush);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartBoundedCircleDeviceCommand

CXTPChartBoundedCircleDeviceCommand::CXTPChartBoundedCircleDeviceCommand(const CXTPChartPointF& center, double radius, const CXTPChartColor& color, int thickness)
{
	m_center = center;
	m_radius = radius;
	m_color = color;
	m_thickness = thickness;
}

void CXTPChartBoundedCircleDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GpPen* pGpPen = NULL;
	GdipCreatePen1(m_color.GetValue(), (REAL)m_thickness, UnitWorld, &pGpPen);

	GdipDrawEllipse(pDC->GetGraphics(), pGpPen, (REAL)(m_center.X - m_radius), (REAL)(m_center.Y - m_radius), (REAL)(m_radius * 2), (REAL)(m_radius * 2));


	GdipDeletePen(pGpPen);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartPolygonDeviceCommand

CXTPChartPolygonDeviceCommand::CXTPChartPolygonDeviceCommand(const CXTPChartPoints& points)
{
	m_points.Copy(points);

	if (points.GetSize() != 0)
	{
		double xMax = points[0].X, xMin = points[0].X;
		double yMax = points[0].Y, yMin = points[0].Y;

		for (int i = 1; i < points.GetSize(); i++)
		{
			xMax = max(xMax, points[i].X);
			yMax = max(yMax, points[i].Y);
			xMin = min(xMin, points[i].X);
			yMin = min(yMin, points[i].Y);
		}

		m_bounds.X = (REAL)xMin;
		m_bounds.Y = (REAL)yMin;
		m_bounds.Width = (REAL)(xMax - xMin);
		m_bounds.Height = (REAL)(yMax - yMin);
	}
}

CXTPChartElement* CXTPChartPolygonDeviceCommand::HitTest(CPoint point, CXTPChartElement* pParent) const
{
	if (point.x < m_bounds.GetLeft() || point.x > m_bounds.GetRight())
		return NULL;

	if (point.y < m_bounds.GetTop() || point.y > m_bounds.GetBottom())
		return NULL;

	GpPath* pGpPath;
	GdipCreatePath(FillModeAlternate, &pGpPath);
	GdipAddPathPolygon(pGpPath, (PointF*)m_points.GetData(), (int)m_points.GetSize());

	BOOL bResult = FALSE;
	GdipIsVisiblePathPointI(pGpPath, point.x, point.y, 0, &bResult);

	GdipDeletePath(pGpPath);

	if (bResult)
		return pParent;

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartSolidPolygonDeviceCommand

CXTPChartSolidPolygonDeviceCommand::CXTPChartSolidPolygonDeviceCommand(const CXTPChartPoints& points, const CXTPChartColor& color)
	: CXTPChartPolygonDeviceCommand(points)
{
	m_color = color;
}

void CXTPChartSolidPolygonDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GpSolidFill* pGpBrush = NULL;
	GdipCreateSolidFill(m_color.GetValue(), &pGpBrush);

	GdipFillPolygon(pDC->GetGraphics(), pGpBrush, (PointF*)m_points.GetData(), (int)m_points.GetSize(), FillModeAlternate);

	GdipDeleteBrush(pGpBrush);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartGradientPolygonDeviceCommand



CXTPChartGradientPolygonDeviceCommand::CXTPChartGradientPolygonDeviceCommand(const CXTPChartPoints& points, const CXTPChartColor& color, const CXTPChartColor& color2, XTPChartLinearGradientMode nMode)
	: CXTPChartPolygonDeviceCommand(points)
{
	m_color = color;
	m_color2 = color2;
	m_nMode = nMode;

}

void CXTPChartGradientPolygonDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GpLineGradient* pGpBrush = NULL;

	if (m_nMode == xtpChartLinearGradientModeCenterHorizontal)
	{
		GdipCreateLineBrushFromRect((RectF*)&m_bounds, m_color.GetValue(), m_color.GetValue(), LinearGradientModeHorizontal, WrapModeTileFlipXY, &pGpBrush);

		Gdiplus::ARGB blend[] = {m_color.GetValue(), m_color2.GetValue(), m_color2.GetValue(), m_color.GetValue()};
		REAL positions[] ={0, 0.45f, 0.55f, 1};
		GdipSetLinePresetBlend(pGpBrush, blend, positions, 4);
	}
	else if (m_nMode == xtpChartLinearGradientModeCenterVertical)
	{
		GdipCreateLineBrushFromRect((RectF*)&m_bounds, m_color.GetValue(), m_color.GetValue(), LinearGradientModeVertical, WrapModeTileFlipXY, &pGpBrush);

		Gdiplus::ARGB blend[] = {m_color.GetValue(), m_color2.GetValue(), m_color2.GetValue(), m_color.GetValue()};
		REAL positions[] ={0, 0.45f, 0.55f, 1};
		GdipSetLinePresetBlend(pGpBrush, blend, positions, 4);
	}
	else
	{
		GdipCreateLineBrushFromRect((RectF*)&m_bounds, m_color.GetValue(), m_color2.GetValue(), (LinearGradientMode)m_nMode, WrapModeTileFlipXY, &pGpBrush);
	}

	GdipFillPolygon(pDC->GetGraphics(), pGpBrush, (PointF*)m_points.GetData(), (int)m_points.GetSize(), FillModeAlternate);


	GdipDeleteBrush(pGpBrush);
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartHatchPolygonDeviceCommand

CXTPChartHatchPolygonDeviceCommand::CXTPChartHatchPolygonDeviceCommand(const CXTPChartPoints& points, const CXTPChartColor& color, const CXTPChartColor& color2, XTPChartHatchStyle nStyle)
	: CXTPChartPolygonDeviceCommand(points)
{
	m_color = color;
	m_color2 = color2;
	m_nStyle = nStyle;
}

void CXTPChartHatchPolygonDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GpHatch* pGpBrush = NULL;
	GdipCreateHatchBrush((GpHatchStyle)m_nStyle, m_color.GetValue(), m_color2.GetValue(), &pGpBrush);

	GdipFillPolygon(pDC->GetGraphics(), pGpBrush, (PointF*)m_points.GetData(), (int)m_points.GetSize(), FillModeAlternate);


	GdipDeleteBrush(pGpBrush);
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartSolidSplinePolygonDeviceCommand

CXTPChartSolidSplinePolygonDeviceCommand::CXTPChartSolidSplinePolygonDeviceCommand(const CXTPChartPoints& points, const CXTPChartColor& color, BOOL bTwoSides)
{
	m_color = color;

	if (!bTwoSides)
	{
		GpPath* pGpPath = 0;
		GdipCreatePath(FillModeAlternate, &pGpPath);
		GdipStartPathFigure(pGpPath);

		int nCount = (int)points.GetSize();

		GdipAddPathLine(pGpPath, points[0].X, points[0].Y, points[1].X, points[1].Y);
		GdipAddPathCurve(pGpPath, (PointF*)points.GetData() + 1, nCount - 2);
		GdipAddPathLine(pGpPath, points[nCount - 2].X, points[nCount - 2].Y, points[nCount - 1].X, points[nCount - 1].Y);

		GdipClosePathFigures(pGpPath);

		m_pGpPath = pGpPath;
	}
	else
	{

		GpPath* pGpPath = 0;
		GdipCreatePath(FillModeAlternate, &pGpPath);
		GdipStartPathFigure(pGpPath);

		int nCount = (int)points.GetSize();

		GdipAddPathCurve(pGpPath, (PointF*)points.GetData(), nCount / 2);
		GdipAddPathLine(pGpPath, points[nCount / 2].X, points[nCount / 2].Y, points[nCount / 2 + 1].X, points[nCount / 2 + 1].Y);

		GdipAddPathCurve(pGpPath, (PointF*)points.GetData() + nCount / 2, nCount / 2);

		GdipAddPathLine(pGpPath, points[nCount - 1].X, points[nCount - 1].Y, points[0].X, points[0].Y);

		GdipClosePathFigures(pGpPath);

		m_pGpPath = pGpPath;

	}
}

CXTPChartSolidSplinePolygonDeviceCommand::~CXTPChartSolidSplinePolygonDeviceCommand()
{
	GdipDeletePath(m_pGpPath);
}

CXTPChartElement* CXTPChartSolidSplinePolygonDeviceCommand::HitTest(CPoint point, CXTPChartElement* pParent) const
{
	BOOL bResult = FALSE;
	GdipIsVisiblePathPointI(m_pGpPath, point.x, point.y, 0, &bResult);

	if (bResult)
		return pParent;

	return NULL;
}


void CXTPChartSolidSplinePolygonDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GpSolidFill* pGpBrush = NULL;
	GdipCreateSolidFill(m_color.GetValue(), &pGpBrush);

	GdipFillPath(pDC->GetGraphics(), pGpBrush, m_pGpPath);

	GdipDeleteBrush(pGpBrush);
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartBoundedPoygonDeviceCommand

CXTPChartBoundedPolygonDeviceCommand::CXTPChartBoundedPolygonDeviceCommand(const CXTPChartPoints& points, const CXTPChartColor& color, int thickness)
{
	m_points.Copy(points);
	m_color = color;
	m_thickness = thickness;
}


void CXTPChartBoundedPolygonDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GpPen* pGpPen = NULL;
	GdipCreatePen1(m_color.GetValue(), (REAL)m_thickness, UnitWorld, &pGpPen);

	GdipDrawPolygon(pDC->GetGraphics(), pGpPen, (PointF*)m_points.GetData(), (int)m_points.GetSize());


	GdipDeletePen(pGpPen);
}
