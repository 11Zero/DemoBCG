//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPGraphicsManager.cpp: implementation of the CBCGPGraphicsManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPMath.h"
#include "BCGPGraphicsManager.h"
#include "BCGPGraphicsManagerGDI.h"
#include "BCGPGraphicsManagerD2D.h"
#include "BCGPDrawManager.h"
#if (!defined _BCGSUITE_) && (!defined _BCGPCHART_STANDALONE)
#include "BCGPColorBar.h"
#endif
#include <float.h>
#include "BCGPImageProcessing.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CBCGPGraphicsResource, CObject)
IMPLEMENT_DYNAMIC(CBCGPGeometry, CBCGPGraphicsResource)
IMPLEMENT_DYNAMIC(CBCGPRectangleGeometry, CBCGPGeometry)
IMPLEMENT_DYNAMIC(CBCGPRoundedRectangleGeometry, CBCGPGeometry)
IMPLEMENT_DYNAMIC(CBCGPEllipseGeometry, CBCGPGeometry)
IMPLEMENT_DYNAMIC(CBCGPPolygonGeometry, CBCGPGeometry)
IMPLEMENT_DYNAMIC(CBCGPSplineGeometry, CBCGPGeometry)
IMPLEMENT_DYNAMIC(CBCGPComplexGeometry, CBCGPGeometry)
IMPLEMENT_DYNAMIC(CBCGPGraphicsManager, CObject)
IMPLEMENT_DYNAMIC(CBCGPLineSegment, CObject)
IMPLEMENT_DYNAMIC(CBCGPBezierSegment, CObject)
IMPLEMENT_DYNAMIC(CBCGPArcSegment, CObject)

static const double g_dblPiePrecision = 0.1;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPGraphicsManager::CBCGPGraphicsManager()
{
	m_Type = (BCGP_GRAPHICS_MANAGER)-1;
	m_pOriginal = NULL;
	m_pPrintInfo = NULL;
	m_pImageDest = NULL;
	m_nSupportedFeatures = 0;
	m_bIsTransparentGradient = TRUE;
	m_bIsDrawShadowMode = FALSE;
	m_ptShadowOffset = CBCGPPoint(2.0, 2.0);
	m_dblScale = 0.0;
}

CBCGPGraphicsManager::~CBCGPGraphicsManager()
{
}

CBCGPGraphicsManager::CBCGPGraphicsManager(const CBCGPRect& rectDest, CBCGPImage* pImageDest) :
	m_pImageDest(pImageDest),
	m_rectDest(rectDest)
{
	m_Type = (BCGP_GRAPHICS_MANAGER)-1;
	m_pOriginal = NULL;
	m_pPrintInfo = NULL;
	m_bIsTransparentGradient = TRUE;
	m_bIsDrawShadowMode = FALSE;
	m_ptShadowOffset = CBCGPPoint(2.0, 2.0);
	m_nSupportedFeatures = 0;
	m_dblScale = 0.0;
}

CBCGPGraphicsManager* CBCGPGraphicsManager::CreateInstance(BCGP_GRAPHICS_MANAGER manager, BOOL bFallback, CBCGPGraphicsManagerParams* pParams)
{
	if (manager == BCGP_GRAPHICS_MANAGER_DEFAULT)
	{
		manager = BCGP_GRAPHICS_MANAGER_D2D;
	}

	CBCGPGraphicsManager* pGM = NULL;

	switch (manager)
	{
	case BCGP_GRAPHICS_MANAGER_GDI:
		pGM = new CBCGPGraphicsManagerGDI(NULL, TRUE, pParams);
		break;

	case BCGP_GRAPHICS_MANAGER_D2D:
		pGM = new CBCGPGraphicsManagerD2D(NULL, TRUE, pParams);

		if (!pGM->IsValid())
		{
			delete pGM;
			pGM = NULL;

			if (bFallback)
			{
				pGM = new CBCGPGraphicsManagerGDI(NULL, TRUE, pParams);;
			}
		}
		break;
	}

	return pGM;
}

void CBCGPGraphicsManager::CleanResources(BOOL bDetach)
{
	for (POSITION pos = m_lstRes.GetHeadPosition(); pos != NULL;)
	{
		CBCGPGraphicsResource* pRes = m_lstRes.GetNext(pos);
		ASSERT_VALID(pRes);

		pRes->Destroy(bDetach);
	}

	m_lstRes.RemoveAll();
}

void CBCGPGraphicsManager::Detach(CBCGPGraphicsResource* pRes)
{
	ASSERT_VALID(pRes);

	if (pRes->IsTemporary())
	{
		return;
	}

	POSITION pos = m_lstRes.Find(pRes);
	if (pos != NULL)
	{
		m_lstRes.RemoveAt(pos);
	}
}

void CBCGPGraphicsManager::SetDrawShadowMode(BOOL bSet, const CBCGPPoint& ptOffset)
{
	m_bIsDrawShadowMode = bSet;
	
	if (bSet)
	{
		m_ptShadowOffset = ptOffset;

		if (m_brShadow.IsEmpty())
		{
			m_brShadow = CBCGPBrush(CBCGPColor::DarkGray, .3);
		}
	}
}

void CBCGPGraphicsManager::SetDrawShadowMode(BOOL bSet, const CBCGPColor& color, int nTransparencyPercent, int nAngle, int nDistance)
{
	m_bIsDrawShadowMode = bSet;
	
	if (!bSet)
	{
		return;
	}

	CBCGPBrush br(color, 0.01 * (100 - nTransparencyPercent));
	if (!m_brShadow.CompareWith(br))
	{
		m_brShadow = br;
	}

	m_ptShadowOffset.x = (double)nDistance * cos(bcg_deg2rad((double)nAngle));
	m_ptShadowOffset.y = -(double)nDistance * sin(bcg_deg2rad((double)nAngle));
}

double CBCGPGraphicsManager::PrepareBevelColors(const CBCGPColor& color, CBCGPColor& colorLight, CBCGPColor& colorDark)
{
	colorDark = color;
	colorDark.MakeDarker(.1);

	colorLight = color;
	colorLight.MakePale();

	return 3.;
}

BOOL CBCGPGraphicsManager::PrepareImage(const CBCGPImage& image, HBITMAP hBitmap)
{
	if (image.IsEmpty () || hBitmap == NULL)
	{
		return FALSE;
	}

	BOOL bIsChanged = FALSE;

	if (!image.GetColorTheme().IsNull())
	{
		CBCGPColorLookupTable lut;
		lut.SetColor(image.GetColorTheme(), (long)(image.GetColorTheme().a * 100));

		if (!BCGPAdjustmentBitmap(hBitmap, (COLORREF)-1, image.IsIgnoreAlphaBitmap(), lut, TRUE))
		{
			return FALSE;
		}

		bIsChanged = TRUE;
	}

	if (image.GetLightRatio() != 1.0)
	{
		if (image.GetLightRatio() < 2.0)
		{
			if (!BCGPBlendBitmap(hBitmap, (COLORREF)-1, image.IsIgnoreAlphaBitmap(), image.GetLightRatio()))
			{
				return FALSE;
			}

			bIsChanged = TRUE;
		}
	}

	return bIsChanged;
}

void CBCGPGraphicsManager::DrawPie(
	const CBCGPEllipse& ellipseSrc, 
	double dblStartAngle, double dblFinishAngle, 
	const CBCGPBrush& brFill, const CBCGPBrush& brLine,
	double dblOffsetFromCenter)
{
	DrawDoughnut(ellipseSrc, 0., dblStartAngle, dblFinishAngle, brFill, brLine, dblOffsetFromCenter);
}

void CBCGPGraphicsManager::DrawDoughnut(
	const CBCGPEllipse& ellipseSrc, 
	double dblHolePerc,	/* 0 - 1 */
	double dblStartAngle, double dblFinishAngle, 
	const CBCGPBrush& brFill, const CBCGPBrush& brLine,
	double dblOffsetFromCenter)
{
	if (ellipseSrc.radiusX < 1. || ellipseSrc.radiusY < 1.)
	{
		return;
	}

	const CBCGPSize sizeHole(ellipseSrc.radiusX * dblHolePerc, ellipseSrc.radiusY * dblHolePerc);
	const BOOL bIsFullEllipse = bcg_IsFullEllipse(dblStartAngle, dblFinishAngle, TRUE, g_dblPiePrecision);

	if (bIsFullEllipse)
	{
		if (!sizeHole.IsEmpty())
		{
			CBCGPEllipseGeometry g1(ellipseSrc);

			CBCGPEllipse ellipseHole = ellipseSrc;
			ellipseHole.radiusX = sizeHole.cx;
			ellipseHole.radiusY = sizeHole.cy;

			CBCGPEllipseGeometry g2(ellipseHole);

			CBCGPGeometry geometry;
			CombineGeometry(geometry, g1, g2, RGN_XOR);

			if (brFill.GetGradientType() == CBCGPBrush::BCGP_GRADIENT_BEVEL)
			{
				const double dblBevelRatio = .95;

				CBCGPEllipse ellipseBevel = ellipseSrc;
				ellipseBevel.radiusX *= dblBevelRatio;
				ellipseBevel.radiusY *= dblBevelRatio;

				CBCGPEllipseGeometry geometryBevel(ellipseBevel);

				CBCGPGeometry geometryInternal;
				CombineGeometry(geometryInternal, geometry, geometryBevel, RGN_AND);

				OnFillGeometryBevel(brFill, geometry, geometryInternal);
			}
			else
			{
				FillGeometry(geometry, brFill);
			}

			DrawEllipse(ellipseHole, brLine);
		}
		else
		{
			FillEllipse(ellipseSrc, brFill);
		}

		DrawEllipse(ellipseSrc, brLine);
		return;
	}
	else if (fabs(dblStartAngle - dblFinishAngle) < g_dblPiePrecision)
	{
		return;
	}

	double angleCos1 = cos(bcg_deg2rad(dblStartAngle));
	double angleSin1 = sin(bcg_deg2rad(dblStartAngle));
	
	double angleCos2 = cos(bcg_deg2rad(dblFinishAngle));
	double angleSin2 = sin(bcg_deg2rad(dblFinishAngle));

	BOOL bIsLargeArc = fabs(dblFinishAngle - dblStartAngle) >= 180.;
	BOOL bIsClockwise = dblStartAngle > dblFinishAngle;

	CBCGPEllipse ellipse = ellipseSrc;

	if (dblOffsetFromCenter != 0. && !bIsFullEllipse)
	{
		double angleMid = .5 * (bcg_deg2rad(dblStartAngle) + bcg_deg2rad(dblFinishAngle));

		ellipse.point.x += dblOffsetFromCenter * cos(angleMid);
		ellipse.point.y -= dblOffsetFromCenter * sin(angleMid);
	}

	CBCGPComplexGeometry shape;

	CBCGPPoint ptStart = CBCGPPoint(
		ellipse.point.x + angleCos1 * sizeHole.cx, 
		ellipse.point.y - angleSin1 * sizeHole.cy);

	shape.SetStart(ptStart);
	
	shape.AddLine(CBCGPPoint(
		ellipse.point.x + angleCos1 * ellipse.radiusX, 
		ellipse.point.y - angleSin1 * ellipse.radiusY));

	shape.AddArc(
		CBCGPPoint(ellipse.point.x + angleCos2 * ellipse.radiusX, ellipse.point.y - angleSin2 * ellipse.radiusY),
		CBCGPSize(ellipse.radiusX, ellipse.radiusY), bIsClockwise, bIsLargeArc);

	if (!sizeHole.IsEmpty())
	{
		shape.AddLine(CBCGPPoint(
			ellipse.point.x + angleCos2 * sizeHole.cx, 
			ellipse.point.y - angleSin2 * sizeHole.cy));

		shape.AddArc(ptStart, sizeHole, !bIsClockwise, bIsLargeArc);
	}

	if (!brFill.HasTextureImage())
	{
		m_rectCurrGradient = ellipseSrc;
	}

	if (brFill.GetGradientType() == CBCGPBrush::BCGP_GRADIENT_BEVEL)
	{
		CBCGPComplexGeometry shapeInternal;
		double nDepth = min(ellipse.radiusX, ellipse.radiusY) * .07;

		ptStart = CBCGPPoint(
			ellipse.point.x + angleCos1 * sizeHole.cx, 
			ellipse.point.y - angleSin1 * sizeHole.cy);

		shapeInternal.SetStart(ptStart);

		double rX = ellipse.radiusX - nDepth;
		double rY = ellipse.radiusY - nDepth;
		
		shapeInternal.AddLine(CBCGPPoint(
			ellipse.point.x + angleCos1 * rX, 
			ellipse.point.y - angleSin1 * rY));

		shapeInternal.AddArc(
			CBCGPPoint(ellipse.point.x + angleCos2 * rX, ellipse.point.y - angleSin2 * rY),
			CBCGPSize(rX, rY), bIsClockwise, bIsLargeArc);

		if (!sizeHole.IsEmpty())
		{
			shapeInternal.AddLine(CBCGPPoint(
				ellipse.point.x + angleCos2 * sizeHole.cx, 
				ellipse.point.y - angleSin2 * sizeHole.cy));

			shapeInternal.AddArc(ptStart, sizeHole, !bIsClockwise, bIsLargeArc);
		}

		OnFillGeometryBevel(brFill, shape, shapeInternal);
	}
	else
	{
		FillGeometry(shape, brFill);
	}

	DrawGeometry(shape, brLine);
	m_rectCurrGradient.SetRectEmpty();
}

void CBCGPGraphicsManager::OnFillGeometryBevel(const CBCGPBrush& brFill, const CBCGPGeometry& shape, const CBCGPGeometry& shapeInternal)
{
	CBCGPColor colorLight, colorDark;
	PrepareBevelColors(brFill.GetColor(), colorLight, colorDark);

	FillGeometry(shape, 
		CBCGPBrush(colorDark, colorLight, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT, brFill.GetOpacity()));

	FillGeometry(shapeInternal, 
		CBCGPBrush(brFill.GetColor(), colorLight, CBCGPBrush::BCGP_GRADIENT_HORIZONTAL, brFill.GetOpacity()));
}

static void Create3DDoughnutSideShape(CBCGPComplexGeometry& shape,
								 const CBCGPPoint& pt,
								 double dblStartAngle,
								 double dblFinishAngle,
								 const CBCGPSize& szRadiusIn,
								 const CBCGPPoint& ptCenter,
								 double dblHeight)
{
	if (pt.y < ptCenter.y)
	{
		return;
	}

	CBCGPSize szRadius = szRadiusIn;
	szRadius.cx = max(0, szRadius.cx);
	szRadius.cy = max(0, szRadius.cy);

	CBCGPPoint ptSide = ptCenter;

	BOOL bIsClockWise = FALSE;

	if (dblStartAngle > dblFinishAngle)
	{
		ptSide.x += szRadius.cx;
		bIsClockWise = TRUE;
	}
	else
	{
		ptSide.x -= szRadius.cx;
		bIsClockWise = FALSE;
	}

	shape.SetStart(CBCGPPoint(ptSide.x, ptSide.y + dblHeight));

	shape.AddArc(CBCGPPoint(pt.x, pt.y + dblHeight), szRadius, bIsClockWise, FALSE);
	shape.AddLine(pt);
	shape.AddArc(ptSide, szRadius, !bIsClockWise, FALSE);
}

void CBCGPGraphicsManager::DrawFillPolygon(const CBCGPPointsArray& pts, const CBCGPBrush& brFill, const CBCGPBrush& brLine)
{
	int nSize = (int)pts.GetSize();
	if (nSize < 3)
	{
		return;
	}

	CBCGPComplexGeometry shape;
	shape.SetStart(pts[0]);

	int i = 0;

	for (i = 1; i < nSize; i++)
	{
		shape.AddLine(pts[i]);
	}

	if (!brFill.IsEmpty())
	{
		FillGeometry(shape, brFill);
	}

	if (!brLine.IsEmpty())
	{
		for (i = 0; i < nSize; i++)
		{
			DrawLine(pts[i], pts[i == nSize - 1 ? 0 : i + 1], brLine);
		}
	}
}

void CBCGPGraphicsManager::DrawBeveledRectangle(const CBCGPRect& rect, const CBCGPBrush& brush, 
												int bevel_size, BOOL bDrawBorder)
{
	if (brush.IsEmpty())
	{
		return;
	}

	CBCGPRect rectShape((CRect)rect);
	int bevel = bevel_size;
	
	if (bDrawBorder)
	{
		rectShape.DeflateRect (1.0, 1.0);
		bevel--;
	}
	
	COLORREF color = brush.GetColor();
	double opacity = brush.GetOpacity();
	
	double percent = 0.23;

	CBCGPColor colorLight(color);
	colorLight.MakeLighter (percent);

	CBCGPColor colorDark(color);
	colorDark.MakeDarker (percent);
	
	// draw internal part
	{
		CBCGPRect rectInternal(rectShape);
		rectInternal.DeflateRect (bevel, bevel);
		
		CBCGPBrush brFill(colorDark, colorLight, CBCGPBrush::BCGP_GRADIENT_HORIZONTAL, opacity);
		FillRectangle (rectInternal, brFill);
	}

	// draw bevel
	{
		CBCGPRect rectBevel(rectShape);
		rectBevel.DeflateRect (bevel, bevel);
		
		CBCGPRect rectBevelHT(rectBevel);
		rectBevelHT.bottom = rectBevelHT.top + 1.0;
		
		CBCGPRect rectBevelHB(rectBevel);
		rectBevelHB.top = rectBevelHB.bottom - 1.0;
		
		CBCGPRect rectBevelVL(rectBevel);
		rectBevelVL.top++;
		rectBevelVL.bottom--;
		rectBevelVL.right = rectBevelVL.left + 1.0;
		
		CBCGPRect rectBevelVR(rectBevel);
		rectBevelVR.top++;
		rectBevelVR.bottom--;
		rectBevelVR.left = rectBevelVR.right - 1.0;
		
		CBCGPColor color1(colorLight);
		color1.MakeLighter (percent);
		CBCGPBrush br1(color1, opacity);

		CBCGPColor color2(colorDark);
		color2.MakeDarker (percent);
		CBCGPBrush br2(color2, opacity);

		double percent2 = percent / 3.0;

		color1 = colorLight;
		color1.MakeDarker (percent2);
		color2 = colorDark;
		color2.MakeDarker (percent2);

		CBCGPBrush br3(color2, color1, CBCGPBrush::BCGP_GRADIENT_HORIZONTAL, opacity);

		color1 = colorLight;
		color1.MakeDarker (percent);
		color2 = colorDark;
		color2.MakeDarker (percent);

		CBCGPBrush br4(color2, color1, CBCGPBrush::BCGP_GRADIENT_HORIZONTAL, opacity);

		for (int i = 0; i < bevel; i++)
		{
			rectBevelHT.InflateRect (1.0, 0.0);
			rectBevelHT.OffsetRect (0.0, -1.0);
			rectBevelHB.InflateRect (1.0, 0.0);
			rectBevelHB.OffsetRect (0.0, 1.0);

			FillRectangle (rectBevelHT, br1);
			FillRectangle (rectBevelHB, br2);
			
			rectBevelVL.InflateRect (0.0, 1.0);
			rectBevelVL.OffsetRect (-1.0, 0.0);

			FillRectangle (rectBevelVL, br3);
			
			rectBevelVR.InflateRect (0.0, 1.0);
			rectBevelVR.OffsetRect (1.0, 0.0);

			FillRectangle (rectBevelVR, br4);
		}
	}
	
	// draw border
	if (bDrawBorder)
	{
		rectShape = rect;
		if (GetType () == CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_D2D)
		{
			rectShape.right--;
			rectShape.bottom--;
		}
		
		colorDark.MakeDarker (0.23);
		DrawRectangle (rectShape, CBCGPBrush(colorDark, opacity));
	}
}

void CBCGPGraphicsManager::Draw3DPie(
	const CBCGPEllipse& ellipseSrc, 
	const double dblHeightSrc,
	double dblStartAngle, double dblFinishAngle, 
	const CBCGPBrush& brFill, const CBCGPBrush& brSideFill, const CBCGPBrush& brLine,
	double dblOffsetFromCenter,
	int nDrawingFlags)
{
	Draw3DDoughnut(ellipseSrc, 0., dblHeightSrc, dblStartAngle, dblFinishAngle, brFill, brSideFill, brLine, dblOffsetFromCenter, nDrawingFlags);
}

void CBCGPGraphicsManager::Draw3DDoughnut(
	const CBCGPEllipse& ellipseSrc, 
	const double dblHolePerc,
	const double dblHeightSrc,
	double dblStartAngle, double dblFinishAngle, 
	const CBCGPBrush& brFill, const CBCGPBrush& brSideFillSrc, const CBCGPBrush& brLine,
	double dblOffsetFromCenter,
	int nDrawingFlags)
{
	CBCGPBrush brBevel1;
	CBCGPBrush brBevel2;
	CBCGPBrush brSideFillBevel;

	const double dblBevelRatio = .95;

	const BOOL bIsBevel = (brFill.GetGradientType() == CBCGPBrush::BCGP_GRADIENT_BEVEL);

	if (bIsBevel)
	{
		CBCGPColor colorLight, colorDark;
		PrepareBevelColors(brFill.GetColor(), colorLight, colorDark);

		brBevel1.SetColors(colorDark, colorLight, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT, brFill.GetOpacity());
		brBevel2.SetColors(brFill.GetColor(), colorLight, CBCGPBrush::BCGP_GRADIENT_HORIZONTAL, brFill.GetOpacity());

		PrepareBevelColors(brSideFillSrc.GetColor(), colorLight, colorDark);
		brSideFillBevel.SetColors(colorDark, colorLight, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT, brFill.GetOpacity());
	}

	const CBCGPBrush& brSideFill = bIsBevel ? brSideFillBevel : brSideFillSrc;

	if (ellipseSrc.radiusX < 1. || ellipseSrc.radiusY < 1.)
	{
		return;
	}

	const CBCGPSize sizeHole(ellipseSrc.radiusX * dblHolePerc, ellipseSrc.radiusY * dblHolePerc);

	const BOOL bIsFullEllipse = bcg_IsFullEllipse(dblStartAngle, dblFinishAngle, TRUE, g_dblPiePrecision);

	double dblHeight = dblHeightSrc;
	dblHeight += .5;

	if (dblHeight <= 2.)
	{
		DrawDoughnut(ellipseSrc, dblHolePerc, dblStartAngle, dblFinishAngle, brFill, brLine, dblOffsetFromCenter);
		return;
	}

	CBCGPEllipse ellipse = ellipseSrc;

	if (dblOffsetFromCenter != 0.)
	{
		double angleMid = .5 * (bcg_deg2rad(dblStartAngle) + bcg_deg2rad(dblFinishAngle));

		ellipse.point.x += dblOffsetFromCenter * cos(angleMid);
		ellipse.point.y -= dblOffsetFromCenter * sin(angleMid);
	}

	double angleCos1 = cos(bcg_deg2rad(dblStartAngle));
	double angleSin1 = sin(bcg_deg2rad(dblStartAngle));

	double angleCos2 = cos(bcg_deg2rad(dblFinishAngle));
	double angleSin2 = sin(bcg_deg2rad(dblFinishAngle));

	double dblDelta = fabs(dblFinishAngle - dblStartAngle);

	BOOL bIsLargeArc = dblDelta >= 180.;

	CBCGPPoint ptCenter = ellipse.point;

	CBCGPPoint pt1(ptCenter.x + angleCos1 * ellipse.radiusX, ptCenter.y - angleSin1 * ellipse.radiusY);
	CBCGPPoint pt2(ptCenter.x + angleCos2 * ellipse.radiusX, ptCenter.y - angleSin2 * ellipse.radiusY);

	CBCGPSize szRadius(ellipse.radiusX, ellipse.radiusY);
	
	CBCGPComplexGeometry shapeTop;
	CBCGPComplexGeometry shapeTopInternal;

	if (!bIsFullEllipse)
	{
		if (fabs(dblStartAngle - dblFinishAngle) < g_dblPiePrecision)
		{
			return;
		}

		CBCGPPoint ptStart = CBCGPPoint(
			ptCenter.x + angleCos1 * sizeHole.cx, 
			ptCenter.y - angleSin1 * sizeHole.cy);

		shapeTop.SetStart(ptStart);

		shapeTop.AddLine(pt1);
		shapeTop.AddArc(pt2, szRadius, dblStartAngle > dblFinishAngle, bIsLargeArc);

		if (!sizeHole.IsEmpty())
		{
			shapeTop.AddLine(CBCGPPoint(
				ptCenter.x + angleCos2 * sizeHole.cx, 
				ptCenter.y - angleSin2 * sizeHole.cy));

			shapeTop.AddArc(ptStart, sizeHole, dblStartAngle <= dblFinishAngle, bIsLargeArc);
		}

		if (bIsBevel)
		{
			shapeTopInternal.SetStart(ptStart);
		
			double rX = ellipse.radiusX * dblBevelRatio;
			double rY = ellipse.radiusY * dblBevelRatio;
			
			shapeTopInternal.AddLine(CBCGPPoint(
				ellipse.point.x + angleCos1 * rX, 
				ellipse.point.y - angleSin1 * rY));

			shapeTopInternal.AddArc(
				CBCGPPoint(ellipse.point.x + angleCos2 * rX, ellipse.point.y - angleSin2 * rY),
				CBCGPSize(rX, rY), dblStartAngle > dblFinishAngle, bIsLargeArc);

			if (!sizeHole.IsEmpty())
			{
				shapeTopInternal.AddLine(CBCGPPoint(
					ellipse.point.x + angleCos2 * sizeHole.cx, 
					ellipse.point.y - angleSin2 * sizeHole.cy));

				shapeTopInternal.AddArc(ptStart, sizeHole, dblStartAngle <= dblFinishAngle, bIsLargeArc);
			}
		}
	}

	CBCGPPoint pt1Bottom = pt1;
	pt1Bottom.y += dblHeight;

	CBCGPPoint pt2Bottom = pt2;
	pt2Bottom.y += dblHeight;

	CBCGPPoint ptCenterBottom = ptCenter;
	ptCenterBottom.y += dblHeight;

	CBCGPComplexGeometry shapeSide;
	CBCGPComplexGeometry shapeSide2;

	if (nDrawingFlags & (BCGP_3D_DRAW_SIDE1 | BCGP_3D_DRAW_SIDE2))
	{
		if (pt1.y >= ptCenter.y && pt2.y >= ptCenter.y && !bIsLargeArc && !bIsFullEllipse)
		{
			if (nDrawingFlags & BCGP_3D_DRAW_SIDE1)
			{
				shapeSide.SetStart(pt1Bottom);

				shapeSide.AddArc(pt2Bottom, szRadius, dblStartAngle > dblFinishAngle, bIsLargeArc);
				shapeSide.AddLine(pt2);
				shapeSide.AddArc(pt1, szRadius, dblStartAngle < dblFinishAngle, bIsLargeArc);
			}
		}
		else if (bIsFullEllipse || (pt1.y < ptCenter.y && pt2.y < ptCenter.y && bIsLargeArc))
		{
			if (nDrawingFlags & BCGP_3D_DRAW_SIDE1)
			{
				CBCGPPoint ptLeft(ptCenter.x - szRadius.cx, ptCenter.y);
				CBCGPPoint ptRight(ptCenter.x + szRadius.cx, ptCenter.y);

				CBCGPPoint ptLeftBottom(ptCenter.x - szRadius.cx, ptCenter.y + dblHeight);
				CBCGPPoint ptRightBottom(ptCenter.x + szRadius.cx, ptCenter.y + dblHeight);

				shapeSide.SetStart(ptLeft);

				shapeSide.AddArc(ptRight, szRadius, FALSE, bIsLargeArc);
				shapeSide.AddLine(ptRightBottom);
				shapeSide.AddArc(ptLeftBottom, szRadius, TRUE, bIsLargeArc);
			}
		}
		else
		{
			if (nDrawingFlags & BCGP_3D_DRAW_SIDE1)
			{
				Create3DDoughnutSideShape(shapeSide, pt1, dblFinishAngle, dblStartAngle, szRadius, ptCenter, dblHeight);
			}

			if (nDrawingFlags & BCGP_3D_DRAW_SIDE2)
			{
				Create3DDoughnutSideShape(shapeSide2, pt2, dblStartAngle, dblFinishAngle, szRadius, ptCenter, dblHeight);
			}
		}
	}

	CBCGPPoint ptOffsetHole1(angleCos1 * sizeHole.cx, -angleSin1 * sizeHole.cy);
	CBCGPPoint ptOffsetHole2(angleCos2 * sizeHole.cx, -angleSin2 * sizeHole.cy);

	CBCGPPoint ptOffsetHole1Int = (ptOffsetHole1.y > 0 && ptOffsetHole2.y > 0) || bIsFullEllipse ? CBCGPPoint(sizeHole.cx, 0) : ptOffsetHole1;
	CBCGPPoint ptOffsetHole2Int = (ptOffsetHole1.y> 0  && ptOffsetHole2.y > 0) || bIsFullEllipse ? CBCGPPoint(-sizeHole.cx, 0) : ptOffsetHole2;

	CBCGPComplexGeometry shapeInternal;

	BOOL bDrawInternalShape = !sizeHole.IsEmpty() && (ptOffsetHole1.y < 0 || ptOffsetHole2.y < 0 || bIsLargeArc) &&
		(nDrawingFlags & BCGP_3D_DRAW_INTERNAL_SIDE);

	if (bDrawInternalShape)
	{
		shapeInternal.SetStart(ptCenter + ptOffsetHole1Int);

		shapeInternal.AddArc(ptCenter + ptOffsetHole2Int, sizeHole, FALSE, bIsLargeArc);
		shapeInternal.AddLine(ptCenterBottom + ptOffsetHole2Int);
		shapeInternal.AddArc(ptCenterBottom + ptOffsetHole1Int, sizeHole, TRUE, bIsLargeArc);
	}

	CBCGPPointsArray ptsEdge1;
	CBCGPPointsArray ptsEdge2;

	if (nDrawingFlags & (BCGP_3D_DRAW_EDGE1 | BCGP_3D_DRAW_EDGE2))
	{
		if (fabs(pt1.x - ptCenter.x) > 2.0 && (nDrawingFlags & BCGP_3D_DRAW_EDGE1))
		{
			ptsEdge1.Add(ptCenter + ptOffsetHole1);
			ptsEdge1.Add(pt1);
			ptsEdge1.Add(pt1Bottom);
			ptsEdge1.Add(ptCenterBottom + ptOffsetHole1);
		}

		if (fabs(pt2.x - ptCenter.x) > 2.0 && (nDrawingFlags & BCGP_3D_DRAW_EDGE2))
		{
			ptsEdge2.Add(ptCenter + ptOffsetHole2);
			ptsEdge2.Add(pt2);
			ptsEdge2.Add(pt2Bottom);
			ptsEdge2.Add(ptCenterBottom + ptOffsetHole2);
		}

		if (pt2.y > pt1.y)
		{
			DrawFillPolygon(ptsEdge1, brSideFill, brLine);
			DrawFillPolygon(ptsEdge2, brSideFill, brLine);
		}
		else
		{
			DrawFillPolygon(ptsEdge2, brSideFill, brLine);
			DrawFillPolygon(ptsEdge1, brSideFill, brLine);
		}
	}

	if (bDrawInternalShape)
	{
		if (!brSideFill.HasTextureImage())
		{
			m_rectCurrGradient = ellipseSrc;
		}

		FillGeometry(shapeInternal, brSideFill);

		DrawArc(ptCenterBottom + ptOffsetHole2Int, ptCenterBottom + ptOffsetHole1Int, 
			sizeHole, TRUE, bIsLargeArc, brLine);

		m_rectCurrGradient.SetRectEmpty();

		if (ptsEdge1.GetSize() > 0 && pt1.y >= ptCenter.y)
		{
			DrawFillPolygon(ptsEdge1, brSideFill, brLine);
		}

		if (ptsEdge2.GetSize() > 0 && pt2.y >= ptCenter.y)
		{
			DrawFillPolygon(ptsEdge2, brSideFill, brLine);
		}
	}

	if (!shapeSide.IsNull() || !shapeSide2.IsNull())
	{
		if (!brSideFill.HasTextureImage())
		{
			m_rectCurrGradient = ellipseSrc;

			m_rectCurrGradient.top = m_rectCurrGradient.CenterPoint().y;
			m_rectCurrGradient.bottom = m_rectCurrGradient.top + dblHeight;
		}

		if (!shapeSide.IsNull())
		{
			FillGeometry(shapeSide, brSideFill);
			DrawGeometry(shapeSide, brLine);
		}

		if (!shapeSide2.IsNull())
		{
			FillGeometry(shapeSide2, brSideFill);
			DrawGeometry(shapeSide2, brLine);
		}

		m_rectCurrGradient.SetRectEmpty();
	}

	if (nDrawingFlags & BCGP_3D_DRAW_TOP)
	{
		if (bIsFullEllipse)
		{
			if (!sizeHole.IsEmpty())
			{
				CBCGPEllipseGeometry g1(ellipseSrc);

				CBCGPEllipse ellipseHole = ellipseSrc;
				ellipseHole.radiusX = sizeHole.cx;
				ellipseHole.radiusY = sizeHole.cy;

				CBCGPEllipseGeometry g2(ellipseHole);

				CBCGPGeometry geometry;
				CombineGeometry(geometry, g1, g2, RGN_XOR);

				if (bIsBevel)
				{
					CBCGPEllipse ellipseBevel = ellipseSrc;
					ellipseBevel.radiusX *= dblBevelRatio;
					ellipseBevel.radiusY *= dblBevelRatio;

					CBCGPEllipseGeometry geometryBevel(ellipseBevel);

					CBCGPGeometry geometryInternal;
					CombineGeometry(geometryInternal, geometry, geometryBevel, RGN_AND);

					OnFillGeometryBevel(brFill, geometry, geometryInternal);
				}
				else
				{
					FillGeometry(geometry, brFill);
				}

				DrawEllipse(ellipseHole, brLine);
			}
			else
			{
				if (bIsBevel)
				{
					FillEllipse(ellipseSrc, brBevel1);

					CBCGPEllipse ellipseInternal = ellipseSrc;
					ellipseInternal.radiusX *= dblBevelRatio;
					ellipseInternal.radiusY *= dblBevelRatio;

					FillEllipse(ellipseInternal, brBevel2);
				}
				else
				{
					FillEllipse(ellipseSrc, brFill);
				}
			}

			DrawEllipse(ellipseSrc, brLine);
		}
		else
		{
			if (!brFill.HasTextureImage())
			{
				m_rectCurrGradient = ellipseSrc;
			}

			if (bIsBevel)
			{
				OnFillGeometryBevel(brFill, shapeTop, shapeTopInternal);
			}
			else
			{
				FillGeometry(shapeTop, brFill);
			}

			DrawGeometry(shapeTop, brLine);
		}
	}

	m_rectCurrGradient.SetRectEmpty();
}

struct CBCGPTorusArc
{
	CBCGPPoint ptFrom;
	CBCGPPoint ptTo;
	CBCGPSize sizeRadius;
	BOOL bIsClockwise;
	BOOL bIsLargeArc;

	CBCGPTorusArc()
	{
		bIsClockwise = bIsLargeArc = FALSE;
	}
};

static void PrepareTorusShapes(const CBCGPEllipse& ellipseExternal,
							   const CBCGPEllipse& ellipseInternal,
							   double dblHeight,
							   double dblStartAngle, double dblFinishAngle,
							   CBCGPEllipse& ellipseEdge1, CBCGPEllipse& ellipseEdge2,
							   CBCGPComplexGeometry& shape,
							   CBCGPTorusArc* pArc1 = NULL,
							   CBCGPTorusArc* pArc2 = NULL,
							   BOOL bBuildShape = TRUE)
{
	CBCGPPoint ptCenter = ellipseExternal.point;


	double angleCos1 = cos(bcg_deg2rad(dblStartAngle));
	double angleSin1 = sin(bcg_deg2rad(dblStartAngle));

	double angleCos2 = cos(bcg_deg2rad(dblFinishAngle));
	double angleSin2 = sin(bcg_deg2rad(dblFinishAngle));

	CBCGPPoint pt1External(ellipseExternal.point.x + angleCos1 * ellipseExternal.radiusX, ellipseExternal.point.y - angleSin1 * ellipseExternal.radiusY);
	CBCGPPoint pt2External(ellipseExternal.point.x + angleCos2 * ellipseExternal.radiusX, ellipseExternal.point.y - angleSin2 * ellipseExternal.radiusY);

	CBCGPPoint pt1Internal(ellipseInternal.point.x + angleCos1 * ellipseInternal.radiusX, ellipseInternal.point.y - angleSin1 * ellipseInternal.radiusY);
	CBCGPPoint pt2Internal(ellipseInternal.point.x + angleCos2 * ellipseInternal.radiusX, ellipseInternal.point.y - angleSin2 * ellipseInternal.radiusY);

	CBCGPPoint ptCenter1(pt1Internal.x + dblHeight * angleCos1 / 2, (pt1Internal.y + pt1External.y) / 2);
	CBCGPPoint ptCenter2(pt2Internal.x + dblHeight * angleCos2 / 2, (pt2Internal.y + pt2External.y) / 2);

	CBCGPSize szRadiusSide1(dblHeight * fabs(angleCos1), dblHeight - dblHeight * fabs(angleSin1) / 4);
	CBCGPSize szRadiusSide2(dblHeight * fabs(angleCos2), dblHeight - dblHeight * fabs(angleSin2) / 4);

	CBCGPPoint pt1Ext(
		ptCenter1.x + angleCos1 * (szRadiusSide1.cx / 2 + 1),
		ptCenter1.y - angleSin1 * (szRadiusSide1.cy / 2 + 1));

	CBCGPPoint pt1Int(
		ptCenter1.x - angleCos1 * (szRadiusSide1.cx / 2 + 1),
		ptCenter1.y + angleSin1 * (szRadiusSide1.cy / 2 + 1));

	CBCGPPoint pt2Ext(
		ptCenter2.x + angleCos2 * (szRadiusSide2.cx / 2 + 1),
		ptCenter2.y - angleSin2 * (szRadiusSide2.cy / 2 + 1));

	CBCGPPoint pt2Int(
		ptCenter2.x - angleCos2 * (szRadiusSide2.cx / 2 + 1),
		ptCenter2.y + angleSin2 * (szRadiusSide2.cy / 2 + 1));

	if (ptCenter1.x > ptCenter.x + 2.0)
	{
		ellipseEdge1 = CBCGPEllipse(ptCenter1, szRadiusSide1);
	}

	if (ptCenter2.x < ptCenter.x - 2.0)
	{
		ellipseEdge2 = CBCGPEllipse(ptCenter2, szRadiusSide2);
	}

	BOOL bIsClockwise1 = pt1Int.x > ptCenter.x;
	BOOL bIsClockwise2 = pt2Int.x > ptCenter.x;

	if (bBuildShape)
	{
		double angleStep = .01;

		double angleRad1 = min(bcg_deg2rad(dblStartAngle), bcg_deg2rad(dblFinishAngle));
		double angleRad2 = max(bcg_deg2rad(dblStartAngle), bcg_deg2rad(dblFinishAngle));

		double angle = 0.;

		for (angle = angleRad1; angle <= angleRad2; angle += angleStep)
		{
			double angleCos = cos(angle);
			double angleSin = sin(angle);

			CBCGPPoint ptExternal(ellipseExternal.point.x + angleCos * ellipseExternal.radiusX, ellipseExternal.point.y - angleSin * ellipseExternal.radiusY);
			CBCGPPoint ptInternal(ellipseInternal.point.x + angleCos * ellipseInternal.radiusX, ellipseInternal.point.y - angleSin * ellipseInternal.radiusY);

			CBCGPPoint ptCenterCurr(ptInternal.x + dblHeight * angleCos / 2, (ptInternal.y + ptExternal.y) / 2);
			CBCGPSize szRadiusSide(dblHeight * fabs(angleCos), dblHeight - dblHeight * fabs(angleSin) / 4);

			CBCGPPoint pt(
				ptCenterCurr.x + angleCos * (szRadiusSide.cx / 2 + 1),
				ptCenterCurr.y - angleSin * (szRadiusSide.cy / 2 + 1));

			if (angle == angleRad1)
			{
				shape.SetStart(pt);
			}
			else
			{
				shape.AddLine(pt);
			}
		}

		if (szRadiusSide2.cx > 2)
		{
			shape.AddArc(pt2Int, 
				CBCGPSize(szRadiusSide2.cx / 2, szRadiusSide2.cy / 2), !bIsClockwise2, FALSE);
		}

		for (angle = angleRad2; angle >= angleRad1; angle -= angleStep)
		{
			double angleCos = cos(angle);
			double angleSin = sin(angle);

			CBCGPPoint ptExternal(ellipseExternal.point.x + angleCos * ellipseExternal.radiusX, ellipseExternal.point.y - angleSin * ellipseExternal.radiusY);
			CBCGPPoint ptInternal(ellipseInternal.point.x + angleCos * ellipseInternal.radiusX, ellipseInternal.point.y - angleSin * ellipseInternal.radiusY);

			CBCGPPoint ptCenterCurr(ptInternal.x + dblHeight * angleCos / 2, (ptInternal.y + ptExternal.y) / 2);
			CBCGPSize szRadiusSide(dblHeight * fabs(angleCos), dblHeight - dblHeight * fabs(angleSin) / 4);

			CBCGPPoint pt(
				ptCenterCurr.x - angleCos * (szRadiusSide.cx / 2 + 1),
				ptCenterCurr.y + angleSin * (szRadiusSide.cy / 2 + 1));

			shape.AddLine(pt);
		}

		if (szRadiusSide1.cx > 2)
		{
			shape.AddArc(pt1Ext, 
				CBCGPSize(szRadiusSide1.cx / 2, szRadiusSide1.cy / 2), bIsClockwise1, FALSE);
		}
	}

	if (pArc1 != NULL)
	{
		pArc1->ptFrom = pt1Int;
		pArc1->ptTo = pt1Ext;
		pArc1->sizeRadius = CBCGPSize(szRadiusSide1.cx / 2, szRadiusSide1.cy / 2);
		pArc1->bIsClockwise = bIsClockwise1;
		pArc1->bIsLargeArc = FALSE;
	}

	if (pArc2 != NULL)
	{
		pArc2->ptFrom = pt2Ext;
		pArc2->ptTo = pt2Int;
		pArc2->sizeRadius = CBCGPSize(szRadiusSide2.cx / 2, szRadiusSide2.cy / 2);
		pArc2->bIsClockwise = !bIsClockwise2;
		pArc2->bIsLargeArc = FALSE;
	}
}

void CBCGPGraphicsManager::Draw3DTorus(
	const CBCGPEllipse& ellipseSrc, 
	const double dblHeightSrc,
	double dblStartAngle, double dblFinishAngle, 
	const CBCGPBrush& brFillSrc, const CBCGPBrush& brSideFillSrc, const CBCGPBrush& brLineSrc,
	double dblOffsetFromCenter,
	int nDrawingFlags)
{
	if (ellipseSrc.radiusX < 1. || ellipseSrc.radiusY < 1.)
	{
		return;
	}

	if (dblHeightSrc <= 2.)
	{
		DrawDoughnut(ellipseSrc, .5, dblStartAngle, dblFinishAngle, brFillSrc, brLineSrc, dblOffsetFromCenter);
		return;
	}

	double dblGradientStep = IsSupported(BCGP_GRAPHICS_MANAGER_ANTIALIAS) ? 1. : 20;

	CBCGPBrush brFill = brFillSrc;
	CBCGPBrush brSideFill = brSideFillSrc;
	CBCGPBrush brLine = brLineSrc;

	CBCGPColor clrDark = brFill.GetColor();
	clrDark.MakeDarker();

	CBCGPColor clrLight = brFill.GetGradientColor();
	clrLight.MakePale();

	if (!brFill.HasTextureImage())
	{
		brFill.SetColors(clrDark, clrLight, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
	}
	else
	{
		dblGradientStep = 360;
	}

	clrLight = brLine.GetColor();
	clrLight.MakePale(.7);

	brLine.SetColor(clrLight);

	clrDark = brSideFill.GetColor();
	clrDark.MakeDarker();

	clrLight = brSideFill.GetGradientColor();
	clrLight.MakePale();

	if (!brSideFill.HasTextureImage())
	{
		brSideFill.SetColors(clrDark, clrLight, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_RIGHT);
	}

	CBCGPEllipse ellipse = ellipseSrc;
	ellipse.radiusY = min(ellipse.radiusY, ellipse.radiusX * 3 / 4);

	double dblHeight = min(dblHeightSrc, min(ellipse.radiusX / 2, ellipse.radiusY * 3 / 4));

	const CBCGPSize sizeHole(ellipse.radiusX * (dblHeight / ellipse.radiusY), dblHeight);

	const BOOL bIsFullEllipse = bcg_IsFullEllipse(dblStartAngle, dblFinishAngle, TRUE, g_dblPiePrecision);

	ellipse.radiusX -= (ellipse.radiusX - sizeHole.cx) / 3;
	ellipse.radiusY -= (ellipse.radiusY - sizeHole.cy) / 2;

	if (dblOffsetFromCenter != 0.)
	{
		double angleMid = .5 * (bcg_deg2rad(dblStartAngle) + bcg_deg2rad(dblFinishAngle));

		ellipse.point.x += dblOffsetFromCenter * cos(angleMid);
		ellipse.point.y -= dblOffsetFromCenter * sin(angleMid);
	}

	CBCGPPoint ptCenter = ellipse.point;

	CBCGPEllipse ellipseInternal = ellipse;
	ellipseInternal.radiusX -= dblHeight * ellipse.radiusY / ellipse.radiusX;
	ellipseInternal.radiusY -= dblHeight / 2;

	CBCGPEllipse ellipseExternal = ellipse;
	ellipseExternal.radiusX += dblHeight * ellipse.radiusY / ellipse.radiusX;
	ellipseExternal.radiusY += dblHeight / 2;

	CBCGPEllipse ellipseEdge1;
	CBCGPEllipse ellipseEdge2;

	if (bIsFullEllipse)
	{
		if (!IsSupported(BCGP_GRAPHICS_MANAGER_ANTIALIAS))
		{
			CBCGPEllipseGeometry g1(ellipseExternal);
			CBCGPEllipseGeometry g2(ellipseInternal);

			CBCGPGeometry geometry;
			CombineGeometry(geometry, g1, g2, RGN_XOR);

			if (!brFill.HasTextureImage())
			{
				m_rectCurrGradient = ellipseSrc;
				m_rectCurrGradient.top -= m_rectCurrGradient.Height() / 2;
			}

			FillGeometry(geometry, brFill);

			DrawEllipse(ellipseExternal, brLine);
			DrawEllipse(ellipseInternal, brLine);

			m_rectCurrGradient.SetRectEmpty();
		}
		else
		{
			for (double dblAngle = 0; dblAngle <= 360; dblAngle += dblGradientStep)
			{
				CBCGPComplexGeometry shape;

				PrepareTorusShapes(ellipseExternal, ellipseInternal, dblHeight,
								   dblAngle, dblAngle + dblGradientStep + 2.,
								   ellipseEdge1, ellipseEdge2, shape);
				FillGeometry(shape, brFill);
			}
		}

		return;
	}

	CBCGPComplexGeometry shape;

	CBCGPTorusArc arc1;
	CBCGPTorusArc arc2;

	BOOL bSmouthFill = nDrawingFlags & (BCGP_3D_DRAW_SIDE1 | BCGP_3D_DRAW_SIDE2);

	PrepareTorusShapes(ellipseExternal, ellipseInternal, dblHeight,
					   dblStartAngle, dblFinishAngle,
					   ellipseEdge1, ellipseEdge2, shape, &arc1, &arc2, bSmouthFill);

	if (bSmouthFill)
	{
		if (IsSupported(BCGP_GRAPHICS_MANAGER_ANTIALIAS))
		{
			for (double dblAngle = dblStartAngle; dblAngle < dblFinishAngle; dblAngle += dblGradientStep)
			{
				double dblAngle2 = min(dblAngle + dblGradientStep + 1., dblFinishAngle);

				if (dblAngle < dblAngle2)
				{
					CBCGPEllipse ellipseEdge11;
					CBCGPEllipse ellipseEdge21;

					CBCGPComplexGeometry shape1;

					PrepareTorusShapes(ellipseExternal, ellipseInternal, dblHeight,
									   dblAngle, dblAngle2,
									   ellipseEdge11, ellipseEdge21, shape1);
					FillGeometry(shape1, brFill);
				}
			}
		}
		else
		{
			FillGeometry(shape, brFill);
		}
	}

	if (nDrawingFlags & BCGP_3D_DRAW_EDGE1)
	{
		FillEllipse(ellipseEdge1, brSideFill);
		DrawEllipse(ellipseEdge1, brLineSrc);
	}

	DrawArc(arc1.ptFrom, arc1.ptTo, arc1.sizeRadius, arc1.bIsClockwise, arc1.bIsLargeArc, brLine);

	if (nDrawingFlags & BCGP_3D_DRAW_EDGE2)
	{
		FillEllipse(ellipseEdge2, brSideFill);
		DrawEllipse(ellipseEdge2, brLineSrc);
	}

	DrawArc(arc2.ptFrom, arc2.ptTo, arc2.sizeRadius, arc2.bIsClockwise, arc2.bIsLargeArc, brLine);
}

void CBCGPGraphicsManager::DrawPyramid(const CBCGPRect& rect,
		const CBCGPBrush& brFill, const CBCGPBrush& brLine,
		double Y1, double Y2)
{
	Draw3DPyramid(rect, 0., brFill, CBCGPBrush(), brLine, 0., Y1, Y2);
}

void CBCGPGraphicsManager::Draw3DPyramid(const CBCGPRect& rectPyramid, double dblDepth,
		const CBCGPBrush& brSideFillLeft, const CBCGPBrush& brSideFillRight, const CBCGPBrush& brLine,
		double dblXCenterOffset, double Y1, double Y2, BOOL bIsCircularBase, const CBCGPBrush& brTopFill)
{
	if (rectPyramid.IsRectEmpty())
	{
		return;
	}

	const BOOL bIsTransparent = brSideFillLeft.GetOpacity() < 1. && dblDepth > 0.;

	CBCGPPointsArray		ptsLeft;
	CBCGPPointsArray		ptsRight;
	CBCGPPointsArray		ptsTop;
	CBCGPPointsArray		ptsHidden;

	CBCGPComplexGeometry	shapeSide;
	CBCGPEllipse			ellipseTop;
	CBCGPEllipse			ellipseBottom;

	dblDepth = min(dblDepth, rectPyramid.Height() / 2 - 1);
	dblDepth = min(dblDepth, rectPyramid.Width() / 2 - 1);

	if (dblDepth <= 4.)
	{
		dblDepth = 0.;
	}

	CBCGPRect rect = rectPyramid;
	rect.DeflateRect(dblDepth, 0, dblDepth, dblDepth);

	double dblMaxXOffset = rect.Width() / 2;

	if (fabs(dblXCenterOffset) > dblMaxXOffset)
	{
		if (dblXCenterOffset < 0)
		{
			dblXCenterOffset = -dblMaxXOffset; 
		}
		else
		{
			dblXCenterOffset = dblMaxXOffset;
		}
	}

	double xCenter = rect.CenterPoint().x;

	if (dblXCenterOffset != 0 && !bIsCircularBase)
	{
		double dblHalfWidth = rect.Width() / 2;
		dblDepth *= (dblHalfWidth - fabs(dblXCenterOffset)) / dblHalfWidth;
	}

	if (Y1 <= 0.)
	{
		double bottom = Y2 < 0. ? rect.bottom : rect.top + Y2;

		double dblRatio = (rect.bottom - bottom) / rect.Height();
		
		dblDepth *= (1. - dblRatio);
		dblXCenterOffset *= (1. - dblRatio);

		double dx = .5 * dblRatio * rect.Width();

		CBCGPPoint ptTop(xCenter, rect.top);
		CBCGPPoint ptBottom(xCenter + dblXCenterOffset, bottom);

		CBCGPPoint ptBottomLeft(rect.left + dx, bottom - .5 * dblDepth);
		CBCGPPoint ptBottomRight(rect.right - dx, bottom - .5 * dblDepth);

		if (bIsCircularBase)
		{
			shapeSide.SetStart(ptTop);
			shapeSide.AddLine(ptBottomLeft);

			if (dblDepth > 0.)
			{
				shapeSide.AddArc(ptBottomRight, CBCGPSize(fabs(xCenter - ptBottomLeft.x), dblDepth / 2), FALSE);
			}
			else
			{
				shapeSide.AddLine(ptBottomRight);
			}

			if (bIsTransparent)
			{
				ellipseBottom.point = CBCGPPoint(xCenter, ptBottomLeft.y + .5);
				ellipseBottom.radiusX = fabs(xCenter - ptBottomLeft.x) - .5;
				ellipseBottom.radiusY = dblDepth / 2 - .5;
			}
		}
		else if (brSideFillRight.IsEmpty())
		{
			ptsLeft.Add(ptTop);
			ptsLeft.Add(ptBottomLeft);
			ptsLeft.Add(ptBottomRight);
		}
		else
		{
			ptsLeft.Add(ptTop);
			ptsLeft.Add(ptBottomLeft);
			ptsLeft.Add(ptBottom);

			ptsRight.Add(ptTop);
			ptsRight.Add(ptBottomRight);
			ptsRight.Add(ptBottom);
		}

		if (bIsTransparent && !bIsCircularBase)
		{
			ptsHidden.Add(ptBottomLeft);
			ptsHidden.Add(CBCGPPoint(ptTop.x, bottom - dblDepth));
			ptsHidden.Add(ptTop);
			ptsHidden.Add(ptBottomRight);
		}
	}
	else
	{
		Y1 = rect.top + Y1;
		Y2 = (Y2 < 0.) ? rect.bottom : rect.top + Y2;

		double dblRatioTop = (rect.bottom - Y1) / rect.Height();
		double dblDepthTop = dblDepth * (1. - dblRatioTop);

		double dblRatioBottom = (rect.bottom - Y2) / rect.Height();
		double dblDepthBottom = dblDepth * (1. - dblRatioBottom);

		double dblXCenterTop = xCenter + dblXCenterOffset * (1. - dblRatioTop);
		double dblXCenterBottom = xCenter + dblXCenterOffset * (1. - dblRatioBottom);

		double dxTop = .5 * dblRatioTop * rect.Width();
		double dxBottom = .5 * dblRatioBottom * rect.Width();

		CBCGPPoint ptTopCenter1(xCenter - dblXCenterOffset * (1. - dblRatioTop), Y1 - dblDepthTop);
		CBCGPPoint ptTopCenter2(dblXCenterTop, Y1);

		CBCGPPoint ptTopLeft(rect.left + dxTop, Y1 - .5 * dblDepthTop);
		CBCGPPoint ptTopRight(rect.right - dxTop, Y1 - .5 * dblDepthTop);

		CBCGPPoint ptBottomCenter(dblXCenterBottom, Y2);

		CBCGPPoint ptBottomLeft(rect.left + dxBottom, Y2 - .5 * dblDepthBottom);
		CBCGPPoint ptBottomRight(rect.right - dxBottom, Y2 - .5 * dblDepthBottom);

		if (dblDepth > 0.)
		{
			if (bIsCircularBase)
			{
				ellipseTop.point = CBCGPPoint(xCenter, ptTopLeft.y + .5);
				ellipseTop.radiusX = fabs(xCenter - ptTopLeft.x) - .5;
				ellipseTop.radiusY = dblDepthTop / 2 - .5;

				if (bIsTransparent)
				{
					ellipseBottom.point = CBCGPPoint(xCenter, ptBottomLeft.y + .5);
					ellipseBottom.radiusX = fabs(xCenter - ptBottomLeft.x) - .5;
					ellipseBottom.radiusY = dblDepthBottom / 2 - .5;
				}
			}
			else
			{
				ptsTop.Add(ptTopCenter1);
				ptsTop.Add(ptTopLeft);
				ptsTop.Add(ptTopCenter2);
				ptsTop.Add(ptTopRight);
			}
		}

		if (bIsCircularBase)
		{
			double dblDelta = (m_Type == BCGP_GRAPHICS_MANAGER_GDI) ? 1. : 0.;

			shapeSide.SetStart(ptBottomLeft);

			if (dblDepth > 0.)
			{
				shapeSide.AddArc(ptBottomRight, CBCGPSize(fabs(xCenter - ptBottomLeft.x), dblDepthBottom / 2 + dblDelta), FALSE);
				shapeSide.AddLine(ptTopRight);
				shapeSide.AddArc(ptTopLeft, CBCGPSize(fabs(xCenter - ptTopLeft.x), dblDepthTop / 2 - dblDelta), TRUE);
			}
			else
			{
				shapeSide.AddLine(ptBottomRight);
				shapeSide.AddLine(ptTopRight);
				shapeSide.AddLine(ptTopLeft);
			}
		}
		else if (brSideFillRight.IsEmpty())
		{
			ptsLeft.Add(ptTopLeft);
			ptsLeft.Add(ptBottomLeft);
			ptsLeft.Add(ptBottomRight);
			ptsLeft.Add(ptTopRight);
		}
		else
		{
			ptsLeft.Add(ptTopCenter2);
			ptsLeft.Add(ptTopLeft);
			ptsLeft.Add(ptBottomLeft);
			ptsLeft.Add(ptBottomCenter);

			ptsRight.Add(ptTopCenter2);
			ptsRight.Add(ptTopRight);
			ptsRight.Add(ptBottomRight);
			ptsRight.Add(ptBottomCenter);
		}

		if (bIsTransparent && !bIsCircularBase)
		{
			ptsHidden.Add(ptBottomLeft);
			ptsHidden.Add(CBCGPPoint(xCenter - dblXCenterOffset * (1. - dblRatioBottom), Y2 - dblDepthBottom));
			ptsHidden.Add(ptTopCenter1);
			ptsHidden.Add(ptBottomRight);
		}
	}

	if (!shapeSide.IsEmpty())
	{
		FillGeometry(shapeSide, brSideFillLeft);
		DrawGeometry(shapeSide, brLine);

		if (!ellipseTop.IsNull())
		{
			FillEllipse(ellipseTop, brTopFill.IsEmpty() ? brSideFillLeft : brTopFill);
			DrawEllipse(ellipseTop, brLine);
		}

		if (!ellipseBottom.IsNull())
		{
			DrawEllipse(ellipseBottom, brLine);
		}
	}
	else
	{
		DrawFillPolygon(ptsLeft, brSideFillLeft, brLine);
		DrawFillPolygon(ptsRight, brSideFillRight, brLine);

		DrawFillPolygon(ptsTop, brTopFill.IsEmpty() ? brSideFillLeft : brTopFill, brLine);
	}

	if (ptsHidden.GetSize() == 4)
	{
		CBCGPBrush brHidden;
		brHidden.SetColor(brLine.GetColor(), brLine.GetOpacity() / 5);

		DrawLine(ptsHidden[0], ptsHidden[1], brHidden);
		DrawLine(ptsHidden[1], ptsHidden[2], brHidden);
		DrawLine(ptsHidden[1], ptsHidden[3], brHidden);
	}
}

void CBCGPGraphicsManager::DrawFunnel(const CBCGPRect& rectFunnel,
	double dblNeckHeight, double dblNeckWidth,
	const CBCGPBrush& brFill, const CBCGPBrush& brLine,
	double Y1, double Y2)
{
	Draw3DFunnel(rectFunnel, 0., dblNeckHeight, dblNeckWidth, brFill, brLine, Y1, Y2);
}

void CBCGPGraphicsManager::Draw3DFunnel(const CBCGPRect& rectFunnel, double dblDepth,
	double dblNeckHeight, double dblNeckWidth,
	const CBCGPBrush& brFill, const CBCGPBrush& brLine,
	double Y1, double Y2, BOOL /*bIsCircularBase*/, const CBCGPBrush& brTopFill)
{
	if (rectFunnel.IsRectEmpty())
	{
		return;
	}

	CBCGPComplexGeometry shapeSide;
	CBCGPEllipse ellipseTop;

	CBCGPRect rect = rectFunnel;

	dblNeckWidth = min(dblNeckWidth, rect.Width());
	dblNeckHeight = min(dblNeckHeight, rect.Height());

	double dblRadiusY1 = 5.0;
	double dblRadiusY2 = 5.0;

	if (dblDepth >= 1.)
	{
		double dblDepth2 = dblDepth / 2.0;
		double nHeight = rect.Height();

		rect.DeflateRect(0.0, dblDepth2);

		double dblRatio = rect.Height() / nHeight;
		dblNeckHeight *= dblRatio;
		Y1 *= dblRatio;
		Y2 *= dblRatio;

		nHeight = rect.Height();

		dblRadiusY1 = max(dblRadiusY1, dblDepth2 * Y1 / nHeight);
		dblRadiusY2 = max(dblRadiusY2, dblDepth2 * Y2 / nHeight);
	}

	Y1 += rect.top;
	Y2 += rect.top;

	double xCenter = rect.CenterPoint().x;

	rect.bottom -= dblNeckHeight;

	if (Y1 >= rect.bottom)
	{
		// Neck part:
		double dblXRadius = dblNeckWidth / 2;

		if (dblDepth < 1.)
		{
			shapeSide.SetStart(CBCGPPoint(xCenter - dblXRadius, Y1));
			shapeSide.AddLine(CBCGPPoint(xCenter + dblXRadius, Y1));
			shapeSide.AddLine(CBCGPPoint(xCenter + dblXRadius, Y2));
			shapeSide.AddLine(CBCGPPoint(xCenter - dblXRadius, Y2));
		}
		else
		{
			CBCGPSize szRadiusTop(dblXRadius, dblRadiusY1);
			CBCGPSize szRadiusBottom(dblXRadius, dblRadiusY2);

			double dblDelta = m_Type == BCGP_GRAPHICS_MANAGER_GDI ? 0 : 2;

			shapeSide.SetStart(CBCGPPoint(xCenter - dblXRadius, Y1 + dblDelta));
			shapeSide.AddArc(CBCGPPoint(xCenter + dblXRadius, Y1), szRadiusTop, FALSE);
			shapeSide.AddLine(CBCGPPoint(xCenter + dblXRadius, Y2));
			shapeSide.AddArc(CBCGPPoint(xCenter - dblXRadius, Y2), szRadiusBottom, TRUE);

			ellipseTop.point = CBCGPPoint(xCenter, Y1 + 1);
			ellipseTop.radiusX = szRadiusTop.cx;
			ellipseTop.radiusY = szRadiusTop.cy;
		}
	}
	else
	{
		double dblNeckWidth2 = dblNeckWidth / 2;
		double dblAngle = bcg_angle(rect.Width() / 2 - dblNeckWidth2, rect.Height());

		double yBottom = Y2 > rect.bottom ? rect.bottom : Y2;

		double dy       = (yBottom - Y1);
		double dx       = dy / tan(dblAngle);
		double dyBottom = Y2 > rect.bottom ? 0 : rect.bottom - Y2;
		double dxBottom = dblNeckWidth2 + (dx * dyBottom / dy);
		double dxTop    = dxBottom + dx;

		CBCGPPoint ptTopLeft(xCenter + dxTop, Y1);
		CBCGPPoint ptTopRight(xCenter - dxTop, Y1);

		CBCGPPoint ptBottomLeft(xCenter + dxBottom, yBottom);
		CBCGPPoint ptBottomRight(xCenter - dxBottom, yBottom);

		if (Y2 > rect.bottom)
		{
			CBCGPPoint ptBottomLeftNeck(xCenter + dxBottom, Y2);
			CBCGPPoint ptBottomRightNeck(xCenter - dxBottom, Y2);

			if (dblDepth < 1.)
			{
				shapeSide.SetStart(ptTopLeft);
				shapeSide.AddLine(ptBottomLeft);
				shapeSide.AddLine(ptBottomLeftNeck);
				shapeSide.AddLine(ptBottomRightNeck);
				shapeSide.AddLine(ptBottomRight);
				shapeSide.AddLine(ptTopRight);
			}
			else
			{
				shapeSide.SetStart(ptBottomLeftNeck);
				shapeSide.AddArc(ptBottomRightNeck, CBCGPSize(fabs(xCenter - ptBottomLeft.x), dblRadiusY2), TRUE);
				shapeSide.AddLine(ptBottomRight);
				shapeSide.AddLine(ptTopRight);
				shapeSide.AddArc(ptTopLeft, CBCGPSize(fabs(xCenter - ptTopLeft.x), dblRadiusY1), FALSE);
				shapeSide.AddLine(ptBottomLeft);

				ellipseTop.point = CBCGPPoint(xCenter, ptTopLeft.y + 1);
				ellipseTop.radiusX = fabs(xCenter - ptTopLeft.x);
				ellipseTop.radiusY = dblRadiusY1;
			}
		}
		else
		{
			if (dblDepth < 1.)
			{
				shapeSide.SetStart(ptTopLeft);
				shapeSide.AddLine(ptBottomLeft);
				shapeSide.AddLine(ptBottomRight);
				shapeSide.AddLine(ptTopRight);
			}
			else
			{
				shapeSide.SetStart(ptBottomLeft);
				shapeSide.AddArc(ptBottomRight, CBCGPSize(fabs(xCenter - ptBottomLeft.x), dblRadiusY2), TRUE);
				shapeSide.AddLine(ptTopRight);
				shapeSide.AddArc(ptTopLeft, CBCGPSize(fabs(xCenter - ptTopLeft.x), dblRadiusY1), FALSE);

				ellipseTop.point = CBCGPPoint(xCenter, ptTopLeft.y + 1);
				ellipseTop.radiusX = fabs(xCenter - ptTopLeft.x);
				ellipseTop.radiusY = dblRadiusY1;
			}
		}
	}

	FillGeometry(shapeSide, brFill);
	DrawGeometry(shapeSide, brLine);

	if (!ellipseTop.IsNull())
	{
		FillEllipse(ellipseTop, brTopFill.IsEmpty() ? brFill : brTopFill);
		DrawEllipse(ellipseTop, brLine);
	}
}

void CBCGPGraphicsManager::DrawArrow3(const CBCGPPoint& pt1, const CBCGPPoint& pt2, double length, 
	const CBCGPBrush& brFill, const CBCGPBrush& brLine, double arrowAngle, DWORD dwStyle)
{
    const double angle = bcg_angle (CBCGPPoint(pt1), CBCGPPoint(pt2));
    double delta = arrowAngle / 180.0;
    const double len2  = length / 2.0;

	CBCGPPoint ptStart = pt2;

    if (dwStyle == 1)
    {
		ptStart.x += cos(angle) * len2;
		ptStart.y += sin(angle) * len2;
    }
    else if (dwStyle == 2)
    {
		ptStart.x += cos(angle) * length;
		ptStart.y += sin(angle) * length;
    }

	CBCGPComplexGeometry shapeArrow;
	shapeArrow.SetStart(ptStart);

    length = length / sin(delta);
    delta *= M_PI;

    shapeArrow.AddLine(CBCGPPoint(
		cos (angle - delta) * length + ptStart.x,
		sin (angle - delta) * length + ptStart.y));

    shapeArrow.AddLine(CBCGPPoint(
		cos (angle + delta) * length + ptStart.x,
		sin (angle + delta) * length + ptStart.y));

	FillGeometry(shapeArrow, brFill);
	DrawGeometry(shapeArrow, brLine);
}

void CBCGPGraphicsManager::DrawArrow4(const CBCGPPoint& pt1, const CBCGPPoint& pt2, double length, 
	const CBCGPBrush& brFill, const CBCGPBrush& brLine, double arrowAngle, DWORD dwStyle)
{
    const double angle = bcg_angle(CBCGPPoint(pt1), CBCGPPoint(pt2));
    double delta = arrowAngle / 180.0;
    const double len2  = length / 2.0;

	CBCGPPoint ptStart = pt2;

    if (dwStyle == 1)
    {
        ptStart.x += cos (angle) * len2;
        ptStart.y += sin (angle) * len2;
    }
    else if (dwStyle == 2)
    {
        ptStart.x += cos (angle) * length;
        ptStart.y += sin (angle) * length;
    }

	CBCGPComplexGeometry shapeArrow;
	shapeArrow.SetStart(ptStart);

    length = length / sin(delta);
    delta *= M_PI;

	shapeArrow.AddLine(CBCGPPoint(
		cos (angle - delta) * length + ptStart.x,
		sin (angle - delta) * length + ptStart.y));

	shapeArrow.AddLine(CBCGPPoint(
		ptStart.x - cos (angle) * len2 * 1.5,
		ptStart.y - sin (angle) * len2 * 1.5));

	shapeArrow.AddLine(CBCGPPoint(
		cos (angle + delta) * length + ptStart.x,
		sin (angle + delta) * length + ptStart.y));

	FillGeometry(shapeArrow, brFill);
	DrawGeometry(shapeArrow, brLine);
}

void CBCGPGraphicsManager::DrawSide(
		const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3, const CBCGPPoint& pt4,
		const CBCGPBrush& brFill, const CBCGPBrush& brLine, double lineWidth)
{
	if (!brFill.IsEmpty())
	{
		CBCGPComplexGeometry shape;
		shape.SetStart(pt1);

		shape.AddLine(pt2);
		shape.AddLine(pt3);
		shape.AddLine(pt4);

		FillGeometry(shape, brFill);
	}

	if (!brLine.IsEmpty())
	{
		DrawLine(pt1, pt2, brLine, lineWidth);
		DrawLine(pt2, pt3, brLine, lineWidth);
		DrawLine(pt3, pt4, brLine, lineWidth);
		DrawLine(pt4, pt1, brLine, lineWidth);
	}
}

void CBCGPGraphicsManager::DrawCheckBox(const CBCGPRect& rect, int nState, const CBCGPColor& clrForeground, const CBCGPBrush& brBackground)
{
	CBCGPRect rectButton(rect);

	int nSize = (int)min(rectButton.Width(), rectButton.Height());
	if (nSize < 4)
	{
		return;
	}

	rectButton.left   = (int)(rectButton.left + (rectButton.Width () - nSize) * 0.5);
	rectButton.right  = rectButton.left + nSize;

	rectButton.top    = (int)(rectButton.top + (rectButton.Height () - nSize) * 0.5);
	rectButton.bottom = rectButton.top + nSize;

	if (!brBackground.IsEmpty())
	{
		FillRectangle(rectButton, brBackground);
	}

	double dblLineWidth = max(1.0, nSize / 10.0);

	CBCGPBrush brLine(clrForeground);

	if (nState == 1)
	{
		CBCGPRect rectInternal(rectButton);
		double dblOffset = bcg_round(2.5 * dblLineWidth);
		rectInternal.DeflateRect(dblOffset, dblOffset);

		CBCGPPointsArray pts;
		pts.Add(rectInternal.CenterLeft());
		pts.Add(rectInternal.BottomCenter());
		pts.Add(rectInternal.TopRight());

		CBCGPStrokeStyle style(CBCGPStrokeStyle::BCGP_CAP_STYLE_ROUND, CBCGPStrokeStyle::BCGP_CAP_STYLE_ROUND,
			CBCGPStrokeStyle::BCGP_CAP_STYLE_ROUND, CBCGPStrokeStyle::BCGP_LINE_JOIN_ROUND);

		DrawLines (pts, brLine, dblLineWidth, &style);
	}

	DrawRectangle(rectButton, brLine, dblLineWidth);
}

void CBCGPGraphicsManager::DrawRadioButton(const CBCGPRect& rect, BOOL bIsSelected, const CBCGPColor& clrForeground, const CBCGPBrush& brBackground)
{
	CBCGPRect rectButton(rect);

	int nSize = (int)min(rectButton.Width(), rectButton.Height());
	if (nSize < 4)
	{
		return;
	}

	rectButton.left   = (int)(rectButton.left + (rectButton.Width () - nSize) * 0.5);
	rectButton.right  = rectButton.left + nSize;

	rectButton.top    = (int)(rectButton.top + (rectButton.Height () - nSize) * 0.5);
	rectButton.bottom = rectButton.top + nSize;

	if (!brBackground.IsEmpty())
	{
		FillEllipse(rectButton, brBackground);
	}

	double dblLineWidth = max(1.0, nSize / 10.0);

	CBCGPBrush brLine(clrForeground);
	
	if (bIsSelected)
	{
		CBCGPRect rectInternal(rectButton);
		double dblOffset = bcg_round(2.0 * dblLineWidth);
		rectInternal.DeflateRect(dblOffset, dblOffset);

		FillEllipse(rectInternal, brLine);
	}
	
	DrawEllipse(rectButton, brLine, dblLineWidth);
}

void CBCGPGraphicsManager::SetClipRect(const CBCGPRect& rectClip, int nFlags)
{
	CBCGPRectangleGeometry geometry(rectClip);
	SetClipArea(geometry, nFlags);
}

void CBCGPGraphicsManager::SetClipEllipse(const CBCGPEllipse& ellipseClip, int nFlags)
{
	CBCGPEllipseGeometry geometry(ellipseClip);
	SetClipArea(geometry, nFlags);
}

void CBCGPGraphicsManager::SetClipRoundedRect(const CBCGPRoundedRect& rectRounded, int nFlags)
{
	CBCGPRoundedRectangleGeometry geometry(rectRounded);
	SetClipArea(geometry, nFlags);
}

////////////////////////////////////////////////////////////////////////////////////
// CBCGPGraphicsResource

void CBCGPGraphicsResource::Set(CBCGPGraphicsManager* pGM, LPVOID lpHandle, LPVOID lpHandle1, LPVOID lpHandle2)
{
	ASSERT(m_lpHandle == NULL || lpHandle == NULL);

	if (pGM != NULL && pGM->m_pOriginal != NULL)
	{
		pGM = pGM->m_pOriginal;
	}

	if (m_pGM != pGM && m_pGM != NULL)
	{
		ASSERT_VALID(m_pGM);

		POSITION pos = m_pGM->m_lstRes.Find(this);
		if (pos != NULL)
		{
			m_pGM->m_lstRes.RemoveAt(pos);
		}
	}

	m_pGM = pGM;
	m_lpHandle = lpHandle;
	m_lpHandle1 = lpHandle1;
	m_lpHandle2 = lpHandle2;

	if (m_pGM != NULL && !m_bIsTemporary)
	{
		ASSERT_VALID(m_pGM);

		if (m_pGM->m_lstRes.Find(this) == NULL)
		{
			m_pGM->m_lstRes.AddTail(this);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////
// CBCGPPointsArray

CBCGPRect CBCGPPointsArray::GetBoundsRect() const
{
	if (!m_rectBounds.IsRectNull())
	{
		return m_rectBounds;
	}

	CBCGPRect rt(0, 0, 0, 0);

	int nCount = (int)GetSize();

	if (nCount == 0)
	{
		return rt;
	}

	rt.left   = (*this)[0].x;
	rt.top    = (*this)[0].y;
	rt.right  = rt.left;
	rt.bottom = rt.top;

	for(int i = 1; i < nCount; i++)
	{
		const CBCGPPoint& pt = (*this)[i];

		if(rt.left > pt.x)
		{
			rt.left = pt.x;
		}

		if(rt.top > pt.y)
		{
			rt.top = pt.y;
		}

		if(rt.right < pt.x)
		{
			rt.right = pt.x;
		}

		if(rt.bottom < pt.y)
		{
			rt.bottom = pt.y;
		}
	}

	return rt;
}

void CBCGPPointsArray::Scale(const CBCGPPoint& ptOffset, double dblRatioX, double dblRatioY, double dblRatioZ)
{
	CBCGPPoint* pPoints = GetData ();
	for(int i = 0; i < (int)GetSize(); i++)
	{
		pPoints->Scale (dblRatioX, dblRatioY, dblRatioZ, ptOffset);
		pPoints++;
	}
}

void CBCGPPointsArray::Rotate(const CBCGPPoint& ptOffset, double angle)
{
	CBCGPPoint* pPoints = GetData ();
	for(int i = 0; i < (int)GetSize(); i++)
	{
		double pt_distance = bcg_distance(ptOffset, *pPoints);
		double pt_angle    = bcg_angle(ptOffset, *pPoints, TRUE);

		pPoints->x += pt_distance * cos(angle + pt_angle);
		pPoints->y -= pt_distance * sin(angle + pt_angle);

		pPoints++;
	}
}

////////////////////////////////////////////////////////////////////////////////////
// CBCGPColor

CArray<COLORREF, COLORREF> CBCGPColor::m_arColors;

CBCGPColor::operator COLORREF() const
{
	return RGB(bcg_clamp(bcg_round(r * 255.0), 0, 255), 
		bcg_clamp(bcg_round(g * 255.0), 0, 255), bcg_clamp(bcg_round(b * 255.0), 0, 255));
}

CString CBCGPColor::ToString() const
{
	if (IsNull())
	{
		return _T("");
	}

	if (a == 0.0)
	{
		return _T("Empty");
	}

	CString strA;
	if (a != 1.0)
	{
		strA = _T(" ,a: ");

		CString str1;
		str1.Format(_T("%d%%"), (int)(a * 100));

		strA += str1;
	}

	CString strName;
#ifndef _BCGPCHART_STANDALONE
#ifndef _BCGSUITE_
	if (!CBCGPColorBar::GetColorName((COLORREF)*this, strName))
#endif
#endif
	{
		strName.Format (_T("r:%02x, g:%02x, b:%02x"), (int)(r * 255), (int)(g * 255), (int)(b * 255));
	}

	strName += strA;
	return strName;
}

BOOL CBCGPColor::IsDark() const
{
	if (IsNull())
	{
		return FALSE;
	}
	
	return CBCGPDrawManager::IsDarkColor((COLORREF)*this);
}

BOOL CBCGPColor::IsLight() const
{
	if (IsNull())
	{
		return FALSE;
	}
	
	return CBCGPDrawManager::IsLightColor((COLORREF)*this);
}

BOOL CBCGPColor::IsPale() const
{
	if (IsNull())
	{
		return FALSE;
	}
	
	return CBCGPDrawManager::IsPaleColor((COLORREF)*this);
}

int CBCGPColor::CompareWith(const CBCGPColor& rOther) const
{
	if (*this == rOther)
	{
		return 0;
	}

	COLORREF clr1 = (COLORREF)*this;
	COLORREF clr2 = (COLORREF)rOther;
	
	double H;
	double S;
	double L1;
	double L2;
	
	CBCGPDrawManager::RGBtoHSL (clr1, &H, &S, &L1);
	CBCGPDrawManager::RGBtoHSL (clr2, &H, &S, &L2);

	return L1 < L2 ? -1 : 1;
}

void CBCGPColor::MakeLighter(double dblRatio)
{
	if (IsNull())
	{
		return;
	}

	if (dblRatio < 0)
	{
		MakeDarker(-dblRatio);
		return;
	}

	COLORREF clr = CBCGPDrawManager::ColorMakeLighter((COLORREF)*this, dblRatio);
	*this = CBCGPColor(clr);
}

void CBCGPColor::MakeDarker(double dblRatio)
{
	if (IsNull())
	{
		return;
	}

	if (dblRatio < 0)
	{
		MakeLighter(-dblRatio);
		return;
	}

	COLORREF clr = CBCGPDrawManager::ColorMakeDarker((COLORREF)*this, dblRatio);
	*this = CBCGPColor(clr);
}

void CBCGPColor::MakePale(double dblLum)
{
	if (IsNull())
	{
		return;
	}

	COLORREF clr = CBCGPDrawManager::ColorMakePale((COLORREF)*this, dblLum);
	*this = CBCGPColor(clr);
}

#ifndef _BCGPCHART_STANDALONE

void CBCGPColor::AddColorToArray(BCGP_COLOR colorName, const CString& strColorLabel)
{
	COLORREF color = (COLORREF)CBCGPColor(colorName);

	if (!strColorLabel.IsEmpty())
	{
		CString str = strColorLabel;

		for (int i = 1; i < str.GetLength(); i++)
		{
			TCHAR tc = str[i];

			if (tc == (TCHAR)_totupper(tc))
			{
				str.Insert(i, _T(" "));
				i++;
			}
		}

		CBCGPColorBar::SetColorName(color, str);
	}

	m_arColors.Add(color);
}

#define BCGP_AddColorToArray(color) AddColorToArray(color, _T(#color))

const CArray<COLORREF, COLORREF>& CBCGPColor::GetRGBArray()
{
	if (m_arColors.GetSize() > 0)
	{
		return m_arColors;
	}

	BCGP_AddColorToArray(Black);
	BCGP_AddColorToArray(MidnightBlue);
	BCGP_AddColorToArray(Indigo);
	BCGP_AddColorToArray(Purple);
	BCGP_AddColorToArray(Maroon);
	BCGP_AddColorToArray(Brown);
	BCGP_AddColorToArray(RosyBrown);
	BCGP_AddColorToArray(DarkKhaki);
	BCGP_AddColorToArray(SeaGreen);
	BCGP_AddColorToArray(DarkGreen);
	BCGP_AddColorToArray(SlateGray);
	BCGP_AddColorToArray(DarkCyan);
	BCGP_AddColorToArray(DeepPink);
	BCGP_AddColorToArray(MediumVioletRed);
	BCGP_AddColorToArray(DarkSlateGray);
	BCGP_AddColorToArray(Navy);
	BCGP_AddColorToArray(DarkSlateBlue);
	BCGP_AddColorToArray(DarkMagenta);
	BCGP_AddColorToArray(DarkRed);
	BCGP_AddColorToArray(Firebrick);
	BCGP_AddColorToArray(BurlyWood);
	BCGP_AddColorToArray(Khaki);
	BCGP_AddColorToArray(MediumSeaGreen);
	BCGP_AddColorToArray(Teal);
	BCGP_AddColorToArray(LightSlateGray);
	BCGP_AddColorToArray(LightSeaGreen);
	BCGP_AddColorToArray(HotPink);
	BCGP_AddColorToArray(PaleVioletRed);
	BCGP_AddColorToArray(DimGray);
	BCGP_AddColorToArray(DarkBlue);
	BCGP_AddColorToArray(SlateBlue);
	BCGP_AddColorToArray(BlueViolet);
	BCGP_AddColorToArray(SaddleBrown);
	BCGP_AddColorToArray(IndianRed);
	BCGP_AddColorToArray(Wheat);
	BCGP_AddColorToArray(PaleGoldenrod);
	BCGP_AddColorToArray(LimeGreen);
	BCGP_AddColorToArray(DarkOliveGreen);
	BCGP_AddColorToArray(CadetBlue);
	BCGP_AddColorToArray(DarkTurquoise);
	BCGP_AddColorToArray(Fuchsia);
	BCGP_AddColorToArray(LightCoral);
	BCGP_AddColorToArray(Gray);
	BCGP_AddColorToArray(MediumBlue);
	BCGP_AddColorToArray(MediumSlateBlue);
	BCGP_AddColorToArray(DarkViolet);
	BCGP_AddColorToArray(Sienna);
	BCGP_AddColorToArray(Crimson);
	BCGP_AddColorToArray(NavajoWhite);
	BCGP_AddColorToArray(LightYellow);
	BCGP_AddColorToArray(LightGreen);
	BCGP_AddColorToArray(Olive);
	BCGP_AddColorToArray(SteelBlue);
	BCGP_AddColorToArray(MediumAquamarine);
	BCGP_AddColorToArray(Magenta);
	BCGP_AddColorToArray(DarkSalmon);
	BCGP_AddColorToArray(DarkGray);
	BCGP_AddColorToArray(Blue);
	BCGP_AddColorToArray(MediumPurple);
	BCGP_AddColorToArray(DarkOrchid);
	BCGP_AddColorToArray(Chocolate);
	BCGP_AddColorToArray(Red);
	BCGP_AddColorToArray(PeachPuff);
	BCGP_AddColorToArray(LightGoldenrodYellow);
	BCGP_AddColorToArray(PaleGreen);
	BCGP_AddColorToArray(OliveDrab);
	BCGP_AddColorToArray(CornflowerBlue);
	BCGP_AddColorToArray(MediumTurquoise);
	BCGP_AddColorToArray(LightPink);
	BCGP_AddColorToArray(Tan);
	BCGP_AddColorToArray(Silver);
	BCGP_AddColorToArray(RoyalBlue);
	BCGP_AddColorToArray(Thistle);
	BCGP_AddColorToArray(MediumOrchid);
	BCGP_AddColorToArray(Peru);
	BCGP_AddColorToArray(OrangeRed);
	BCGP_AddColorToArray(Moccasin);
	BCGP_AddColorToArray(LemonChiffon);
	BCGP_AddColorToArray(SpringGreen);
	BCGP_AddColorToArray(Green);
	BCGP_AddColorToArray(LightSteelBlue);
	BCGP_AddColorToArray(Turquoise);
	BCGP_AddColorToArray(MistyRose);
	BCGP_AddColorToArray(Orange);
	BCGP_AddColorToArray(LightGray);
	BCGP_AddColorToArray(DodgerBlue);
	BCGP_AddColorToArray(Lavender);
	BCGP_AddColorToArray(Orchid);
	BCGP_AddColorToArray(DarkGoldenrod);
	BCGP_AddColorToArray(Tomato);
	BCGP_AddColorToArray(Bisque);
	BCGP_AddColorToArray(OldLace);
	BCGP_AddColorToArray(Lime);
	BCGP_AddColorToArray(ForestGreen);
	BCGP_AddColorToArray(LightBlue);
	BCGP_AddColorToArray(Aquamarine);
	BCGP_AddColorToArray(PapayaWhip);
	BCGP_AddColorToArray(Gold);
	BCGP_AddColorToArray(Gainsboro);
	BCGP_AddColorToArray(DeepSkyBlue);
	BCGP_AddColorToArray(AliceBlue);
	BCGP_AddColorToArray(Violet);
	BCGP_AddColorToArray(Goldenrod);
	BCGP_AddColorToArray(Coral);
	BCGP_AddColorToArray(BlanchedAlmond);
	BCGP_AddColorToArray(Linen);
	BCGP_AddColorToArray(LawnGreen);
	BCGP_AddColorToArray(DarkSeaGreen);
	BCGP_AddColorToArray(PowderBlue);
	BCGP_AddColorToArray(PaleTurquoise);
	BCGP_AddColorToArray(LavenderBlush);
	BCGP_AddColorToArray(Yellow);
	BCGP_AddColorToArray(WhiteSmoke);
	BCGP_AddColorToArray(LightSkyBlue);
	BCGP_AddColorToArray(Honeydew);
	BCGP_AddColorToArray(Plum);
	BCGP_AddColorToArray(DarkOrange);
	BCGP_AddColorToArray(Salmon);
	BCGP_AddColorToArray(Cornsilk);
	BCGP_AddColorToArray(Snow);
	BCGP_AddColorToArray(Chartreuse);
	BCGP_AddColorToArray(YellowGreen);
	BCGP_AddColorToArray(LightCyan);
	BCGP_AddColorToArray(Aqua);
	BCGP_AddColorToArray(FloralWhite);
	BCGP_AddColorToArray(AntiqueWhite);
	BCGP_AddColorToArray(White);
	BCGP_AddColorToArray(SkyBlue);
	BCGP_AddColorToArray(MintCream);
	BCGP_AddColorToArray(Pink);
	BCGP_AddColorToArray(SandyBrown);
	BCGP_AddColorToArray(LightSalmon);
	BCGP_AddColorToArray(Beige);
	BCGP_AddColorToArray(GhostWhite);
	BCGP_AddColorToArray(GreenYellow);
	BCGP_AddColorToArray(MediumSpringGreen);
	BCGP_AddColorToArray(Azure);
	BCGP_AddColorToArray(Cyan);
	BCGP_AddColorToArray(Ivory);
	BCGP_AddColorToArray(SeaShell);

	return m_arColors;
}

BOOL CBCGPColor::CreatePalette(CPalette& palette)
{
	const CArray<COLORREF, COLORREF>& arColors = GetRGBArray();

	if (palette.GetSafeHandle () != NULL)
	{
		::DeleteObject (palette.Detach ());
		ASSERT (palette.GetSafeHandle () == NULL);
	}

	#define MAX_COLOURS 200
	int nNumColours = (int) arColors.GetSize ();
	if (nNumColours == 0)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT (nNumColours <= MAX_COLOURS);
	if (nNumColours > MAX_COLOURS)
	{
		nNumColours = MAX_COLOURS;
	}

	// Create the palette
	struct 
	{
		LOGPALETTE    LogPalette;
		PALETTEENTRY  PalEntry [MAX_COLOURS];
	}
	pal;

	LOGPALETTE* pLogPalette = (LOGPALETTE*) &pal;
	pLogPalette->palVersion    = 0x300;
	pLogPalette->palNumEntries = (WORD) nNumColours; 

	for (int i = 0; i < nNumColours; i++)
	{
		pLogPalette->palPalEntry[i].peRed   = GetRValue (arColors[i]);
		pLogPalette->palPalEntry[i].peGreen = GetGValue (arColors[i]);
		pLogPalette->palPalEntry[i].peBlue  = GetBValue (arColors[i]);
		pLogPalette->palPalEntry[i].peFlags = 0;
	}

	palette.CreatePalette (pLogPalette);
	return TRUE;
}

#endif

////////////////////////////////////////////////////////////////////////////////////
// CBCGPBrush

CBCGPBrush::CBCGPBrush()
{
	CommonInit();
}

CBCGPBrush::CBCGPBrush(const CBCGPColor& color, double opacity)
{
	CommonInit();

	m_opacity = opacity;
	m_color = color;
}

CBCGPBrush::CBCGPBrush(const CBCGPColor& color, const CBCGPColor& colorGradient, 
	BCGP_GRADIENT_TYPE gradientType, double opacity)
{
	CommonInit();

	m_gradientType = gradientType;
	m_opacity = opacity;
	m_color = color;
	m_colorGradient = colorGradient;
}

CBCGPBrush::CBCGPBrush(const CBCGPColor& color, 
	BCGP_GRADIENT_TYPE gradientType, double opacity)
{
	CommonInit();

	m_gradientType = gradientType;
	m_opacity = opacity;
	m_color = color;
	m_colorGradient = CBCGPColor(1., 1., 1.);
}

CBCGPBrush::CBCGPBrush(const CBCGPImage& image, const CBCGPColor& clrFillAlt, BOOL bIsWaterMarkImage)
{
	CommonInit();

	m_TextureImage = image;
	m_TextureImage.Destroy();

	m_color = clrFillAlt;
	m_gradientType = BCGP_NO_GRADIENT;

	m_bIsWaterMarkImage = bIsWaterMarkImage;
}

CBCGPBrush::CBCGPBrush(const CBCGPBrush& brush)
{
	m_opacity			= brush.m_opacity;
	m_color				= brush.m_color;
	m_colorGradient		= brush.m_colorGradient;
	m_gradientType		= brush.m_gradientType;
	m_nPenWidth			= brush.m_nPenWidth;
	m_nPenStyle			= brush.m_nPenStyle;
	m_TextureImage		= brush.m_TextureImage;
	m_bIsWaterMarkImage = brush.m_bIsWaterMarkImage;

	m_TextureImage.Destroy();
}

void CBCGPBrush::CommonInit()
{
	m_gradientType = BCGP_NO_GRADIENT;
	m_opacity = 1.;
	m_nPenWidth = 0;
	m_nPenStyle = PS_SOLID;
	m_bIsWaterMarkImage = FALSE;
}

void CBCGPBrush::Destroy(BOOL bDetachFromGM)
{
	if (m_pGM != NULL)
	{
		if (bDetachFromGM)
		{
			m_pGM->Detach(this);
		}

		m_pGM->DestroyBrush(*this);
	}
}

void CBCGPBrush::SetColor(const CBCGPColor& color, double opacity)
{
	SetColors(color, CBCGPColor(), BCGP_NO_GRADIENT, opacity);
}

void CBCGPBrush::SetColors(const CBCGPColor& color, const CBCGPColor& colorGradient, 
		BCGP_GRADIENT_TYPE gradientType, double opacity)
{
	if (m_gradientType == gradientType && m_opacity == opacity && 
		m_color == color && m_colorGradient == colorGradient)
	{
		return;
	}

	if (!m_TextureImage.IsEmpty())
	{
		if (m_color == color && m_colorGradient == colorGradient)
		{
			SetOpacity(opacity);
			return;
		}
	
		m_TextureImage.Clear();
	}

	if (m_pGM != NULL)
	{
		ASSERT_VALID(m_pGM);

		CBCGPGraphicsManager* pGM = m_pGM;
		m_pGM->DestroyBrush(*this);

		m_pGM = pGM;
	}

	m_gradientType = gradientType;
	m_opacity = opacity;
	m_color = color;
	m_colorGradient = colorGradient;
}

void CBCGPBrush::SetOpacity(double opacity)
{
	if (m_opacity == opacity)
	{
		return;
	}

	m_opacity = opacity;

	if (m_pGM != NULL)
	{
		ASSERT_VALID(m_pGM);
		m_pGM->SetBrushOpacity(*this);
	}
}

void CBCGPBrush::SetTextureImage(const CBCGPImage& image, const CBCGPColor& clrFillAlt, BOOL bIsWaterMarkImage)
{
	if (m_TextureImage == image && m_color == clrFillAlt && m_bIsWaterMarkImage == bIsWaterMarkImage)
	{
		return;
	}

	if (m_pGM != NULL)
	{
		ASSERT_VALID(m_pGM);
		
		CBCGPGraphicsManager* pGM = m_pGM;
		m_pGM->DestroyBrush(*this);
		
		m_pGM = pGM;
	}

	m_color = clrFillAlt;
	m_colorGradient = CBCGPColor();
	m_gradientType = BCGP_NO_GRADIENT;

	m_TextureImage = image;
	m_TextureImage.Destroy();

	m_bIsWaterMarkImage = bIsWaterMarkImage;
}

void CBCGPBrush::ClearTextureImage()
{
	SetTextureImage(CBCGPImage(), CBCGPColor());
}

void CBCGPBrush::SetPenAttributes(int nWidth, int nStyle)
{
	m_nPenWidth = nWidth;
	m_nPenStyle = nStyle;
}

void CBCGPBrush::MakeLighter(double dblRatio)
{
	m_TextureImage.MakeLighter(dblRatio);
	m_color.MakeLighter(dblRatio);
	m_colorGradient.MakeLighter(dblRatio);
}

void CBCGPBrush::MakeDarker(double dblRatio)
{
	m_TextureImage.MakeDarker(dblRatio);
	m_color.MakeDarker(dblRatio);
	m_colorGradient.MakeDarker(dblRatio);
}

void CBCGPBrush::MakePale(double dblLum)
{
	m_TextureImage.MakePale(dblLum);
	m_color.MakePale(dblLum);
	m_colorGradient.MakePale(dblLum);
}

void CBCGPBrush::CopyFrom(const CBCGPBrush& src)
{
	if (CompareWith(src))
	{
		return;
	}

	Destroy();

	m_gradientType = src.m_gradientType;
	m_opacity = src.m_opacity;
	m_color = src.m_color;
	m_colorGradient = src.m_colorGradient;
	m_nPenWidth = src.m_nPenWidth;
	m_nPenStyle = src.m_nPenStyle;
	m_TextureImage = src.m_TextureImage;
	m_bIsWaterMarkImage = src.m_bIsWaterMarkImage;
	m_TextureImage.Destroy();
}

BOOL CBCGPBrush::CompareWith(const CBCGPBrush& src) const
{
	return (m_gradientType == src.m_gradientType && 
		m_opacity == src.m_opacity && 
		m_color == src.m_color && 
		m_colorGradient == src.m_colorGradient &&
		m_nPenWidth == src.m_nPenWidth &&
		m_nPenStyle == src.m_nPenStyle &&
		m_TextureImage == src.m_TextureImage &&
		m_bIsWaterMarkImage == src.m_bIsWaterMarkImage);
}

////////////////////////////////////////////////////////////////////////////////////
// CBCGPGeometry

void CBCGPGeometry::Destroy(BOOL bDetachFromGM)
{
	if (m_pGM != NULL)
	{
		if (bDetachFromGM)
		{
			m_pGM->Detach(this);
		}

		m_pGM->DestroyGeometry(*this);
	}
}

CBCGPRect CBCGPGeometry::GetBoundsRect()
{
	CBCGPRect bounds;

	if (m_pGM != NULL)
	{
		m_pGM->GetGeometryBoundingRect (*this, bounds);
	}

	return CBCGPRect();
}

void CBCGPGeometry::Scale(double, double, const CBCGPPoint&)
{
}

////////////////////////////////////////////////////////////////////////////////////
// CBCGPArcSegment

CBCGPPoint CBCGPArcSegment::GetArcCenter(const CBCGPPoint& ptFrom, BOOL& bIsLargeArc, double& rX, double& rY) const
{
	const double x1 = ptFrom.x;
	const double y1 = ptFrom.y;

	const double x2 = m_Point.x;
	const double y2 = fabs(m_Point.y - y1) < (double)FLT_EPSILON ? y1 : m_Point.y;

	rX = m_Radius.cx;
	rY = m_Radius.cy;

	CBCGPPoint ptCenter;

	if (rX <= 0.0 || rY <= 0.0)
	{
		rX = 0.0;
		rY = 0.0;
		return ptCenter;
	}

	const double x12  = x1 * x1;
	const double x22  = x2 * x2;
	const double y12  = y1 * y1;
	const double y22  = y2 * y2;

	const double rX2  = rX * rX;
	const double rY2  = rY * rY;
	const double rYX2 = rY2 / rX2;

	const double dY = y1 == y2 ? 0.01 : (y1 - y2);
	const double K = rYX2 * (x2 - x1) / dY;
	const double M = (rYX2 * (x12 - x22) + y12 - y22) / (2.0 * dY);
	const double A = rY2 + rX2 * K * K;
	const double B = 2.0 * (rX2 * K * (M - y1) - rY2 * x1);
	const double C = rY2 * x12 + rX2 * ((M - 2.0 * y1) * M + y12) - rX2 * rY2;
	double D = B * B - 4.0 * A * C;

	bIsLargeArc = m_bIsLargeArc;

	if (D >= 0)
	{
		if (D > 0)
		{
			D = sqrt(D);

			if (y2 <= y1)
			{
				D = -D;
			}

			if ((!m_bIsClockwise || !m_bIsLargeArc) && (m_bIsClockwise || m_bIsLargeArc))
			{
				D = -D;
			}
		}

		ptCenter.x = (-B + D) / (2.0 * A);
		ptCenter.y = K * ptCenter.x + M;
	}
	else
	{
		ptCenter.x = (x2 + x1) / 2.0;
		ptCenter.y = (y2 + y1) / 2.0;

		D = rY / rX;
		rX = bcg_distance(x1 - ptCenter.x, (y1 - ptCenter.y) / D);
		rY = D * rX;

		bIsLargeArc = FALSE;
	}

	return ptCenter;
}

////////////////////////////////////////////////////////////////////////////////////
// CBCGPStrokeStyle

CBCGPStrokeStyle::CBCGPStrokeStyle(
	BCGP_CAP_STYLE startCap,
	BCGP_CAP_STYLE endCap,
	BCGP_CAP_STYLE dashCap,
	BCGP_LINE_JOIN lineJoin,
	FLOAT miterLimit,
	BCGP_DASH_STYLE dashStyle,
	FLOAT dashOffset)
{
	m_startCap = startCap;
	m_endCap = endCap;
	m_dashCap = dashCap;
	m_lineJoin = lineJoin;
	m_miterLimit = miterLimit;
	m_dashStyle = dashStyle;
	m_dashOffset = dashOffset;

	CommonInit();
}

CBCGPStrokeStyle::CBCGPStrokeStyle(
	const CArray<FLOAT, FLOAT>& arDashes,
	BCGP_CAP_STYLE startCap,
	BCGP_CAP_STYLE endCap,
	BCGP_CAP_STYLE dashCap,
	BCGP_LINE_JOIN lineJoin,
	FLOAT miterLimit,
	FLOAT dashOffset)
{
	m_arDashes.Append(arDashes);

	m_startCap = startCap;
	m_endCap = endCap;
	m_dashCap = dashCap;
	m_lineJoin = lineJoin;
	m_miterLimit = miterLimit;
	m_dashStyle = BCGP_DASH_STYLE_CUSTOM;
	m_dashOffset = dashOffset;

	CommonInit();
}

CBCGPStrokeStyle::CBCGPStrokeStyle(const CBCGPStrokeStyle& strokeStyle)
{
	CopyFrom(strokeStyle);
}

void CBCGPStrokeStyle::CommonInit()
{
	if (m_dashStyle == BCGP_DASH_STYLE_DOT && m_dashCap == BCGP_CAP_STYLE_FLAT)
	{
		m_dashCap = BCGP_CAP_STYLE_SQUARE;
	}
}

void CBCGPStrokeStyle::CopyFrom(const CBCGPStrokeStyle& strokeStyle)
{
	if (CompareWith(strokeStyle))
	{
		return;
	}

	Destroy();

	m_arDashes.RemoveAll();
	m_arDashes.Append(strokeStyle.m_arDashes);

	m_startCap = strokeStyle.m_startCap;
	m_endCap = strokeStyle.m_endCap;
	m_dashCap = strokeStyle.m_dashCap;
	m_lineJoin = strokeStyle.m_lineJoin;
	m_miterLimit = strokeStyle.m_miterLimit;
	m_dashStyle = strokeStyle.m_dashStyle;
	m_dashOffset = strokeStyle.m_dashOffset;
}

BOOL CBCGPStrokeStyle::CompareWith(const CBCGPStrokeStyle& strokeStyle) const
{
	return	m_startCap == strokeStyle.m_startCap &&
			m_endCap == strokeStyle.m_endCap &&
			m_dashCap == strokeStyle.m_dashCap &&
			m_lineJoin == strokeStyle.m_lineJoin &&
			m_miterLimit == strokeStyle.m_miterLimit &&
			m_dashStyle == strokeStyle.m_dashStyle &&
			m_dashOffset == strokeStyle.m_dashOffset;
}

void CBCGPStrokeStyle::Destroy(BOOL bDetachFromGM)
{
	if (m_pGM != NULL)
	{
		if (bDetachFromGM)
		{
			m_pGM->Detach(this);
		}

		m_pGM->DestroyStrokeStyle(*this);
	}
}

void CBCGPStrokeStyle::SetDashStyle(BCGP_DASH_STYLE dashStyle, FLOAT dashOffset)
{
	if (dashStyle == m_dashStyle && dashOffset == m_dashOffset)
	{
		return;
	}

	Destroy();

	m_dashStyle = dashStyle;
	m_dashOffset = dashOffset;
}

////////////////////////////////////////////////////////////////////////////////////
// CBCGPTextFormat

CBCGPTextFormat::CBCGPTextFormat()
{
	CommonInit();
}

CBCGPTextFormat::CBCGPTextFormat(
		const CString& strFontFamily,
		float fFontSize,
		long lFontWeight,
		BCGP_FONT_STYLE fontStyle,
		LPCTSTR lpszFontLocale)
{
	CommonInit();
	Create(strFontFamily, fFontSize, lFontWeight, fontStyle, lpszFontLocale);

	m_fFontSizeOriginal = fFontSize;
}

CBCGPTextFormat::CBCGPTextFormat(
		BYTE bCharSet,
		const CString& strFontFamily,
		float fFontSize,
		long lFontWeight,
		BCGP_FONT_STYLE fontStyle)
{
	CommonInit();
	Create(strFontFamily, fFontSize, lFontWeight, fontStyle, CharSetToLocale(bCharSet));

	m_fFontSizeOriginal = fFontSize;
}

CBCGPTextFormat::CBCGPTextFormat(
		const LOGFONT& lf)
{
	CommonInit();
	CreateFromLogFont(lf);
}

CBCGPTextFormat::CBCGPTextFormat(
		const CBCGPTextFormat& tf)
{
	CopyFrom(tf);
	Create(m_strFontFamily, m_fFontSize, m_lFontWeight, m_fontStyle, m_strFontLocale);
}

BOOL CBCGPTextFormat::Create(
	const CString& strFontFamily,
	float fFontSize,
	long lFontWeight,
	BCGP_FONT_STYLE fontStyle,
	LPCTSTR lpszFontLocale)
{
	if (m_pGM != NULL && GetHandle() != NULL)
	{
		ASSERT_VALID(m_pGM);

		CBCGPGraphicsManager* pGM = m_pGM;
		m_pGM->DestroyTextFormat(*this);

		m_pGM = pGM;
	}

	m_strFontFamily = strFontFamily;
	m_fFontSizeOriginal = m_fFontSize = fFontSize;
	m_lFontWeight = lFontWeight;
	m_fontStyle = fontStyle;
	m_strFontLocale = lpszFontLocale == NULL ? _T("") : lpszFontLocale;

	return TRUE;
}

BOOL CBCGPTextFormat::CreateFromLogFont(const LOGFONT& lf)
{
	m_bFromLogFont = TRUE;
	m_fFontSizeOriginal = (float)lf.lfHeight;
	m_bIsUnderline = lf.lfUnderline;
	m_bIsStrikethrough = lf.lfStrikeOut;

	return Create(lf.lfFaceName, (float)lf.lfHeight, lf.lfWeight,
		lf.lfItalic ? BCGP_FONT_STYLE_ITALIC : BCGP_FONT_STYLE_NORMAL,
		CharSetToLocale(lf.lfCharSet));
}

void CBCGPTextFormat::ExportToLogFont(LOGFONT& lf) const
{
	memset(&lf, 0, sizeof(LOGFONT));

	lstrcpyn (lf.lfFaceName, m_strFontFamily, LF_FACESIZE);
	lf.lfWeight = m_lFontWeight;
	lf.lfHeight = (long)m_fFontSize;
	lf.lfEscapement = lf.lfOrientation = (int)(m_dblDrawingAngle * 10.);
	lf.lfUnderline = (BYTE)m_bIsUnderline;
	lf.lfStrikeOut = (BYTE)m_bIsStrikethrough;
	
	lf.lfCharSet = GetCharSet();
	
	if (lf.lfHeight > 0)
	{
		lf.lfHeight = -lf.lfHeight;
	}
}

void CBCGPTextFormat::CopyFrom(const CBCGPTextFormat& from)
{
	if (CompareWith(from))
	{
		return;
	}

	Destroy();

	m_strFontFamily =			from.m_strFontFamily;
	m_fFontSize =				from.m_fFontSize;       
	m_fFontSizeOriginal =		from.m_fFontSizeOriginal;
	m_lFontWeight =				from.m_lFontWeight;     
	m_fontStyle =				from.m_fontStyle;       
	m_strFontLocale =			from.m_strFontLocale;   
	m_textAlignment =			from.m_textAlignment;   
	m_textAlignmentVert =		from.m_textAlignmentVert;
	m_bWordWrap =				from.m_bWordWrap;       
	m_bClipText	=				from.m_bClipText;
	m_bFromLogFont =			from.m_bFromLogFont;
	m_dblDrawingAngle =			from.m_dblDrawingAngle;
	m_bDrawingAngleWasChanged = from.m_bDrawingAngleWasChanged; 
	m_bIsUnderline = 			from.m_bIsUnderline;
	m_bIsStrikethrough =		from.m_bIsStrikethrough;
}

BOOL CBCGPTextFormat::CompareWith(const CBCGPTextFormat& src) const
{
	return 
		m_strFontFamily ==			src.m_strFontFamily &&
		m_fFontSize ==				src.m_fFontSize &&
		m_fFontSizeOriginal ==		src.m_fFontSizeOriginal &&
		m_lFontWeight ==			src.m_lFontWeight &&     
		m_fontStyle ==				src.m_fontStyle &&       
		m_strFontLocale ==			src.m_strFontLocale &&   
		m_textAlignment ==			src.m_textAlignment &&   
		m_textAlignmentVert ==		src.m_textAlignmentVert &&
		m_bWordWrap ==				src.m_bWordWrap &&       
		m_bClipText	==				src.m_bClipText &&
		m_dblDrawingAngle ==		src.m_dblDrawingAngle &&
		m_bIsUnderline ==			src.m_bIsUnderline &&
		m_bIsStrikethrough ==		src.m_bIsStrikethrough;
}

void CBCGPTextFormat::Destroy(BOOL bDetachFromGM)
{
	if (m_pGM != NULL)
	{
		if (bDetachFromGM)
		{
			m_pGM->Detach(this);
		}

		m_pGM->DestroyTextFormat(*this);
	}
}

void CBCGPTextFormat::CommonInit()
{
	m_fFontSize = 0.;
	m_fFontSizeOriginal = 0.;
	m_lFontWeight = 0;
	m_fontStyle = BCGP_FONT_STYLE_NORMAL;
	m_textAlignment = BCGP_TEXT_ALIGNMENT_LEADING;
	m_textAlignmentVert = BCGP_TEXT_ALIGNMENT_LEADING;
	m_bWordWrap = FALSE;
	m_bClipText = FALSE;
	m_bFromLogFont = FALSE;
	m_dblDrawingAngle = 0.;
	m_bDrawingAngleWasChanged = FALSE;
	m_bIsUnderline = FALSE;
	m_bIsStrikethrough = FALSE;
}

void CBCGPTextFormat::SetFontSize(float fFontSize)
{
	if (m_fFontSize != fFontSize)
	{
		Destroy();
		m_fFontSizeOriginal = m_fFontSize = fFontSize;
		Create(m_strFontFamily, m_fFontSize, m_lFontWeight, m_fontStyle, m_strFontLocale);
	}
}

void CBCGPTextFormat::SetFontStyle(BCGP_FONT_STYLE style)
{
	if (m_fontStyle != style)
	{
		Destroy();
		m_fontStyle = style;
		Create(m_strFontFamily, m_fFontSize, m_lFontWeight, m_fontStyle, m_strFontLocale);
	}
}

void CBCGPTextFormat::SetFontFamily(const CString& strFontFamily)
{
	if (m_strFontFamily != strFontFamily)
	{
		Destroy();
		m_strFontFamily = strFontFamily;
		Create(m_strFontFamily, m_fFontSize, m_lFontWeight, m_fontStyle, m_strFontLocale);
	}
}

void CBCGPTextFormat::SetUnderline(BOOL bSet)
{
	if (m_bIsUnderline != bSet)
	{
		Destroy();
		m_bIsUnderline = bSet;
	}
}

void CBCGPTextFormat::SetStrikethrough(BOOL bSet)
{
	if (m_bIsStrikethrough != bSet)
	{
		Destroy();
		m_bIsStrikethrough = bSet;
	}
}

void CBCGPTextFormat::Scale(double dblRatio)
{
	ASSERT(dblRatio > 0.);

	if (m_fFontSizeOriginal == 0.)
	{
		return;
	}

	if (dblRatio == 1.)
	{
		ResetSize();
		return;
	}

	float fFontSize = (float)(m_fFontSizeOriginal * dblRatio);
	if (m_fFontSize != fFontSize)
	{
		Destroy();
		m_fFontSize = fFontSize;
		float fFontSizeOriginal = m_fFontSizeOriginal;

		Create(m_strFontFamily, m_fFontSize, m_lFontWeight, m_fontStyle, m_strFontLocale);

		m_fFontSizeOriginal = fFontSizeOriginal;
	}
}

void CBCGPTextFormat::ResetSize()
{
	SetFontSize(m_fFontSizeOriginal);
}

void CBCGPTextFormat::AdjustTextSize(double dblAngle, CBCGPSize& sz)
{
	if (dblAngle == 0.)
	{
		return;
	}

	dblAngle = bcg_deg2rad(dblAngle);

	CBCGPPoint pt1(sz.cx* cos(dblAngle), sz.cx* sin(dblAngle));
	CBCGPPoint pt2(sz.cy* cos(-M_PI_2 + dblAngle), sz.cy* sin(-M_PI_2 + dblAngle));

	double d = bcg_distance(sz.cx, sz.cy);
	double a = bcg_angle(sz.cx, -sz.cy) + dblAngle;
	CBCGPPoint pt3(d* cos(a), d* sin(a));

	sz.cx = fabs(max(max(max(pt1.x, pt2.x), pt3.x), 0) - min(0, min(min(pt1.x, pt2.x), pt3.x)));
	sz.cy = fabs(max(max(max(pt1.y, pt2.y), pt3.y), 0) - min(0, min(min(pt1.y, pt2.y), pt3.y)));
}

BYTE CBCGPTextFormat::GetCharSet() const
{
	BYTE charset = DEFAULT_CHARSET;

	CString ln = m_strFontLocale;
	ln.TrimLeft (TCHAR(' '));
	ln.TrimRight (TCHAR(' '));

	int length = ln.GetLength();
	if (length < 2)
	{
		return charset;
	}

	ln.MakeLower ();

	CString ln2(ln.Left(2));

	if (length >= 5)
	{
		if (length >= 7)
		{
			if (ln2.Compare (_T("az")) == 0 || ln2.Compare (_T("uz")) == 0)
			{
				if (ln.Find (_T("cyrl")) != -1)
				{
					return RUSSIAN_CHARSET;
				}
				else if (ln.Find (_T("latn")) != -1)
				{
					return TURKISH_CHARSET;
				}
			}
			else if (ln2.Compare (_T("bs")) == 0 || ln2.Compare (_T("sr")) == 0)
			{
				if (ln.Find (_T("cyrl")) != -1)
				{
					return RUSSIAN_CHARSET;
				}
				else if (ln.Find (_T("latn")) != -1)
				{
					return 0xEE; // EE_CHARSET
				}
			}
			else if ((ln2.Compare (_T("mn")) == 0 || ln2.Compare (_T("tg")) == 0) &&
					ln.Find (_T("cyrl")) != -1)
			{
				return RUSSIAN_CHARSET;
			}
			else if (ln.Find (_T("iu-latn")) != -1 || ln.Find (_T("tmz-latn")) != -1)
			{
				return ANSI_CHARSET;
			}
		}
		else if (ln2.Compare (_T("zh")) == 0)
		{
			if (ln.Find (_T("cn")) != -1 || ln.Find (_T("sg")) != -1)
			{
				return GB2312_CHARSET;
			}
			else if (ln.Find (_T("hk")) != -1 || ln.Find (_T("mo")) != -1 ||
					ln.Find (_T("tw")) != -1)
			{
				return CHINESEBIG5_CHARSET;
			}
		}
	}

	int i = 0;

	if (length >= 3)
	{
		CString ln3(ln.Left(3));

		if (ln3.Compare (_T("sah")) == 0)
		{
			return RUSSIAN_CHARSET;
		}
		else if (ln3.Compare (_T("gbz")) == 0)
		{
			return ARABIC_CHARSET;
		}
		else
		{
			LPCTSTR szANSI[] = {_T("arn"), _T("moh"), _T("wen"), _T("smn"), 
								_T("smj"), _T("quz"), _T("sms"), _T("dsb"), 
								_T("sma"), _T("fil"), _T("qut")};

			for (i = 0; i < _countof(szANSI); i++)
			{
				if (ln3.Compare (szANSI[i]) == 0)
				{
					return ANSI_CHARSET;
				}
			}
		}
	}

	if (ln2.Compare (_T("el")) == 0)
	{
		return GREEK_CHARSET;
	}
	else if (ln2.Compare (_T("he")) == 0)
	{
		return HEBREW_CHARSET;
	}
	else if (ln2.Compare (_T("ja")) == 0)
	{
		return SHIFTJIS_CHARSET;
	}
	else if (ln2.Compare (_T("th")) == 0)
	{
		return THAI_CHARSET;
	}
	else if (ln2.Compare (_T("tr")) == 0)
	{
		return TURKISH_CHARSET;
	}
	else if (ln2.Compare (_T("ko")) == 0)
	{
		return HANGUL_CHARSET;
	}


	// ANSI_CHARSET
	LPCTSTR sz00[] = {_T("af"), _T("sq"), _T("eu"), _T("br"), _T("ca"), _T("da"),
					_T("nl"), _T("en"), _T("fo"), _T("fi"), _T("fr"), _T("fy"),
					_T("gl"), _T("de"), _T("kl"), _T("ha"), _T("is"), _T("id"),
					_T("ga"), _T("it"), _T("rw"), _T("lb"), _T("ms"), _T("mt"),
					_T("mi"), _T("nb"), _T("nn"), _T("oc"), _T("pt"), _T("rm"),
					_T("se"), _T("ns"), _T("tn"), _T("es"), _T("cy"), _T("wo"),
					_T("xh"), _T("zu")};

	// ARABIC_CHARSET
	LPCTSTR szB2[] = {_T("ar"), _T("fa"), _T("ug"), _T("ur")};

	// RUSSIAN_CHARSET
	LPCTSTR szCC[] = {_T("be"), _T("bg"), _T("kk"), _T("ky"), _T("mk"), _T("ru"),
					_T("tt"), _T("tk"), _T("uk")};

	// BALTIC_CHARSET
	LPCTSTR szBA[] = {_T("et"), _T("lv"), _T("lt")};

	// EE_CHARSET - 0xEE (238)	
	LPCTSTR szEE[] = {_T("hr"), _T("cs"), _T("hu"), _T("pl"), _T("ro"), _T("sk"),
					_T("sl")};

	for (i = 0; i < _countof(sz00); i++)
	{
		if (ln2.Compare (sz00[i]) == 0)
		{
			return ANSI_CHARSET;
		}
	}

	for (i = 0; i < _countof(szB2); i++)
	{
		if (ln2.Compare (szB2[i]) == 0)
		{
			return ARABIC_CHARSET;
		}
	}

	for (i = 0; i < _countof(szBA); i++)
	{
		if (ln2.Compare (szBA[i]) == 0)
		{
			return BALTIC_CHARSET;
		}
	}

	for (i = 0; i < _countof(szCC); i++)
	{
		if (ln2.Compare (szCC[i]) == 0)
		{
			return RUSSIAN_CHARSET;
		}
	}

	for (i = 0; i < _countof(szEE); i++)
	{
		if (ln2.Compare (szEE[i]) == 0)
		{
			return 0xEE;
		}
	}

	return charset;
}

CString CBCGPTextFormat::CharSetToLocale(BYTE bCharSet)
{
	switch (bCharSet)
	{
	case ANSI_CHARSET:
		return _T("en-US");

	case SHIFTJIS_CHARSET:
		return _T("ja-JP");

	case HANGUL_CHARSET:
		return _T("ko-KR");

	case GB2312_CHARSET:
		return _T("zh-CN");

	case CHINESEBIG5_CHARSET:
		return _T("zh-TW");

	case HEBREW_CHARSET:
		return _T("he-IL");

	case ARABIC_CHARSET:
		return _T("ar-SA");

	case GREEK_CHARSET:
		return _T("el-GR");

	case TURKISH_CHARSET:
		return _T("tr-TR");

	case VIETNAMESE_CHARSET:
		return _T("vi-VN");

	case THAI_CHARSET:
		return _T("th-TH");

	case EASTEUROPE_CHARSET:
		return _T("cs-CZ");

	case RUSSIAN_CHARSET:
		return _T("ru-RU");

	case BALTIC_CHARSET:
		return _T("lt-LT");
	}

	return _T("");
}

////////////////////////////////////////////////////////////////////////////////////
// CBCGPImage

CBCGPImage::CBCGPImage(UINT uiResID, LPCTSTR lpszType)
{
	CommonInit();

	m_uiResID = uiResID;
	m_szType = lpszType;
}

CBCGPImage::CBCGPImage(const CString& strPath)
{
	CommonInit();
	m_strPath = strPath;
}

CBCGPImage::CBCGPImage(HICON hIcon, CSize sizeIcon, BOOL bIsAlpha)
{
	CommonInit();

	m_hIcon = hIcon;
	m_sizeIcon = sizeIcon;
	m_bIsAlphaIcon = bIsAlpha;
}

CBCGPImage::CBCGPImage(HBITMAP hbmp, BOOL bIsIgnoreAlpha)
{
	CommonInit();

	m_hBitmap = hbmp;
	m_bIsIgnoreAlphaBitmap = bIsIgnoreAlpha;
}

CBCGPImage::CBCGPImage(const CBCGPImage& srcImage)
{
	CopyFrom(srcImage);
}

void CBCGPImage::CommonInit()
{
	m_uiResID = 0;
	m_szType = NULL;
	m_hIcon = NULL;
	m_sizeIcon = CSize(0, 0);
	m_bIsAlphaIcon = FALSE;
	m_hBitmap = NULL;
	m_bIsIgnoreAlphaBitmap = FALSE;
	m_dblLightRatio = 1.0;
	m_bMap3dColorsInGDI = FALSE;
}

void CBCGPImage::Load(UINT uiResID, LPCTSTR lpszType)
{
	Destroy(FALSE);

	m_uiResID = uiResID;
	m_szType = lpszType;
}

void CBCGPImage::Load(const CString& strPath)
{
	m_strPath = strPath;
}

void CBCGPImage::Destroy(BOOL bDetachFromGM)
{
	if (m_pGM != NULL)
	{
		if (bDetachFromGM)
		{
			m_pGM->Detach(this);
		}

		m_pGM->DestroyImage(*this);
	}
}

void CBCGPImage::CopyFrom(const CBCGPImage& srcImage)
{
	if (m_pGM != NULL && m_lpHandle != NULL)
	{
		m_pGM->DestroyImage(*this);
	}

	m_uiResID = srcImage.m_uiResID;
	m_szType = srcImage.m_szType;
	m_strPath = srcImage.m_strPath;
	m_hIcon = srcImage.m_hIcon;
	m_sizeIcon = srcImage.m_sizeIcon;
	m_bIsAlphaIcon = srcImage.m_bIsAlphaIcon;
	m_hBitmap = srcImage.m_hBitmap;
	m_bIsIgnoreAlphaBitmap = srcImage.m_bIsIgnoreAlphaBitmap;
	m_dblLightRatio = srcImage.m_dblLightRatio;
	m_bMap3dColorsInGDI = srcImage.m_bMap3dColorsInGDI;
	m_clrTheme = srcImage.m_clrTheme;
	m_sizeDest = srcImage.m_sizeDest;

	if (srcImage.m_lpHandle != NULL)
	{
		if (m_pGM == NULL)
		{
			m_pGM = srcImage.m_pGM;
		}

		if (m_pGM == NULL)
		{
			return;
		}

		ASSERT_VALID(m_pGM);
		m_pGM->CopyImage((CBCGPImage&)srcImage, (CBCGPImage&)*this, CBCGPRect());
	}
	else
	{
		m_lpHandle = NULL;
	}
}

BOOL CBCGPImage::CompareWith(const CBCGPImage& srcImage) const
{
	return m_uiResID == srcImage.m_uiResID &&
		m_szType == srcImage.m_szType &&
		m_strPath == srcImage.m_strPath &&
		m_hIcon == srcImage.m_hIcon &&
		m_bIsAlphaIcon == srcImage.m_bIsAlphaIcon &&
		m_hBitmap == srcImage.m_hBitmap &&
		m_bIsIgnoreAlphaBitmap == srcImage.m_bIsIgnoreAlphaBitmap &&
		m_dblLightRatio == srcImage.m_dblLightRatio &&
		m_clrTheme == srcImage.m_clrTheme &&
		m_sizeDest == srcImage.m_sizeDest;
}

CBCGPImage& CBCGPImage::operator = (const CBCGPImage& image)
{
	CopyFrom (image);
	return *this;
}

bool CBCGPImage::operator == (const CBCGPImage& image) const
{
	return (CompareWith(image) == TRUE);
}

CBCGPSize CBCGPImage::GetSize(CBCGPGraphicsManager* pGM) const
{
	if (pGM == NULL)
	{
		pGM = m_pGM;
	}

	if (pGM == NULL)
	{
		return CBCGPSize();
	}

	return pGM->GetImageSize((CBCGPImage&)*this);
}

void CBCGPImage::Resize(const CBCGPSize& sizeDest)
{
	if (m_sizeDest == sizeDest)
	{
		return;
	}

	m_sizeDest = sizeDest;
	Destroy(FALSE);
}

BOOL CBCGPImage::CopyTo(CBCGPImage& imageDest, const CBCGPRect& rectSrc) const
{
	if (m_pGM == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(m_pGM);

	return m_pGM->CopyImage((CBCGPImage&)*this, imageDest, rectSrc);
}

HBITMAP CBCGPImage::ExportToBitmap()
{
	if (m_pGM == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(m_pGM);

	return m_pGM->ExportImageToBitmap((CBCGPImage&)*this);
}

void CBCGPImage::Clear()
{
	CopyFrom(CBCGPImage());
}

void CBCGPImage::MakeLighter(double dblRatio)
{
	double dblLightRatio = bcg_clamp(1.0 + dblRatio, 1.0, 2.0);
	if (m_dblLightRatio != dblLightRatio)
	{
		m_dblLightRatio = dblLightRatio;
	}
}

void CBCGPImage::MakeDarker(double dblRatio)
{
	double dblLightRatio = bcg_clamp(dblRatio, 0.0, 1.0);
	if (m_dblLightRatio != dblLightRatio)
	{
		m_dblLightRatio = dblLightRatio;
	}
}

void CBCGPImage::MakePale(double dblLum)
{
	double dblLightRatio = bcg_clamp(2.0 + dblLum, 2.0, 3.0);
	if (m_dblLightRatio != dblLightRatio)
	{
		m_dblLightRatio = dblLightRatio;
	}
}

void CBCGPImage::MakeNormal()
{
	m_dblLightRatio = 1.0;
	m_clrTheme = CBCGPColor();
}

void CBCGPImage::Colorize(const CBCGPColor& clr)
{
	m_clrTheme = clr;
}

////////////////////////////////////////////////////////////////////////////////////
// CBCGPSplineGeometry

void CBCGPSplineGeometry::SetPoints(const CBCGPPointsArray& arPoints, BCGP_SPLINE_TYPE type, 
									BOOL bIsClosed, double tension, double bias, double continuity)
{
	Clear();

	m_bIsClosed = bIsClosed;

	int nCount = (int)arPoints.GetSize ();

	if (nCount < 3)
	{
		return;
	}

	m_arPoints.SetSize((nCount - 1) * 3 + 1, 0);

	const CBCGPPoint* pSrcPoints = arPoints.GetData ();
	CBCGPPoint* pDstPoints = m_arPoints.GetData ();

	*pDstPoints++ = *pSrcPoints;

	if (type == BCGP_SPLINE_TYPE_KB)
	{
		tension    = bcg_clamp(tension, -1.0, 1.0);
		bias       = bcg_clamp(bias, -1.0, 1.0);
		continuity = bcg_clamp(continuity, -1.0, 1.0);

		for (int i = 0; i < nCount - 1; i++)
		{
			const CBCGPPoint& pt0 = pSrcPoints[i == 0 ? (m_bIsClosed ? nCount - 2 : 0) : i - 1];
			const CBCGPPoint& pt1 = pSrcPoints[i];
			const CBCGPPoint& pt2 = pSrcPoints[i + 1];
			const CBCGPPoint& pt3 = pSrcPoints[i == nCount - 2 ? (m_bIsClosed ? 1 : nCount - 1) : i + 2];

			*pDstPoints++ = pt1 + ((pt2 - pt1) * (1.0 - bias) * (1.0 - continuity) +
						(pt1 - pt0) * (1.0 + bias) * (1.0 + continuity)) * (1.0 - tension) / 6.0;
			*pDstPoints++ = pt2 - ((pt3 - pt2) * (1.0 - bias) * (1.0 + continuity) +
						(pt2 - pt1) * (1.0 + bias) * (1.0 - continuity)) * (1.0 - tension) / 6.0;
			*pDstPoints++ = pt2;
		}
	}
	else
	{
		for (int i = 0; i < nCount - 1; i++)
		{
			const CBCGPPoint& pt0 = pSrcPoints[i == 0 ? (m_bIsClosed ? nCount - 2 : 0) : i - 1];
			const CBCGPPoint& pt1 = pSrcPoints[i];
			const CBCGPPoint& pt2 = pSrcPoints[i + 1];
			const CBCGPPoint& pt3 = pSrcPoints[i == nCount - 2 ? (m_bIsClosed ? 1 : nCount - 1) : i + 2];
	
			double x   = bcg_distance(pt2, pt1);
			double x01 = bcg_distance(pt1, pt0);
			double x10 = bcg_distance(pt3, pt2);

			if (x <= FLT_EPSILON)
			{
				x = 1.0;
			}
			if (x01 <= FLT_EPSILON)
			{
				x01 = 1.0;
			}
			if (x10 <= FLT_EPSILON)
			{
				x10 = 1.0;
			}

			*pDstPoints++ = pt1 + ((pt2 - pt1) + (pt1 - pt0) * x / x01) / 6.0;
			*pDstPoints++ = pt2 - ((pt3 - pt2) * x / x10 + (pt2 - pt1)) / 6.0;
			*pDstPoints++ = pt2;
		}
	}
}

void CBCGPSplineGeometry::Clear()
{
	m_bIsClosed = TRUE;
	m_arPoints.RemoveAll();
	
	if (m_pGM != NULL)
	{
		ASSERT_VALID(m_pGM);
		
		CBCGPGraphicsManager* pGM = m_pGM;
		m_pGM->DestroyGeometry(*this);
		
		m_pGM = pGM;
	}
}

void CBCGPSplineGeometry::Scale(double dblRatioX, double dblRatioY, const CBCGPPoint& ptOffset)
{
	Destroy ();
	m_arPoints.Scale (ptOffset, dblRatioX, dblRatioY, 1.0);
}

////////////////////////////////////////////////////////////////////////////////////
// CBCGPComplexGeometry

void CBCGPComplexGeometry::AddLine(const CBCGPPoint& pt)
{
	if (m_ptStart == CBCGPPoint(-1., -1.))
	{
		return;
	}

	m_arSegments.Add(new CBCGPLineSegment(pt));
}

void CBCGPComplexGeometry::AddBezier(const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3)
{
	if (m_ptStart == CBCGPPoint(-1., -1.))
	{
		ASSERT(FALSE);
		return;
	}

	m_arSegments.Add(new CBCGPBezierSegment(pt1, pt2, pt3));
}

void CBCGPComplexGeometry::AddArc(const CBCGPPoint& pt, const CBCGPSize szRadiusIn, 
	BOOL bIsClockwise, BOOL bIsLargeArc, double dblRotationAngle)
{
	if (m_ptStart == CBCGPPoint(-1., -1.))
	{
		ASSERT(FALSE);
		return;
	}

	CBCGPSize szRadius = szRadiusIn;
	szRadius.cx = max(0, szRadius.cx);
	szRadius.cy = max(0, szRadius.cy);

	m_arSegments.Add(new CBCGPArcSegment(pt, szRadius, bIsClockwise, bIsLargeArc, dblRotationAngle));
}

void CBCGPComplexGeometry::AddPoints(const CBCGPPointsArray& arPoints, CBCGPPolygonGeometry::BCGP_CURVE_TYPE curveType)
{
	BOOL bSetStart = m_ptStart == CBCGPPoint(-1., -1.);

	int nCount = (int)arPoints.GetSize();
	if (nCount < 1)
	{
		return;
	}

	if (bSetStart)
	{
		SetStart (arPoints[0]);
	}

	if (curveType == CBCGPPolygonGeometry::BCGP_CURVE_TYPE_LINE)
	{
		for (int i = (bSetStart ? 1 : 0); i < nCount; i++)
		{
			m_arSegments.Add(new CBCGPLineSegment(arPoints[i]));
		}
	}
	else
	{
		if (!bSetStart)
		{
			m_arSegments.Add(new CBCGPLineSegment(arPoints[0]));
		}

		if (nCount == 2)
		{
			m_arSegments.Add(new CBCGPLineSegment(arPoints[1]));
			return;
		}

		for (int i = 1; i < nCount; i++)
		{
			const CBCGPPoint& pt = arPoints[i];
			CBCGPBezierSegment* pSegment = new CBCGPBezierSegment(pt, pt, pt);

			if (i < nCount - 1)
			{
				i++;
			}

			pSegment->m_Point2 = arPoints[i];
			if (i < nCount - 1)
			{
				i++;
			}

			pSegment->m_Point3 = arPoints[i];

			m_arSegments.Add(pSegment);
		}
	}
}

void CBCGPComplexGeometry::Clear()
{
	m_ptStart = CBCGPPoint(-1., -1.);
	m_bIsClosed = TRUE;

	for (int i = 0; i < m_arSegments.GetSize(); i++)
	{
		delete m_arSegments[i];
	}

	m_arSegments.RemoveAll();

	if (m_pGM != NULL)
	{
		ASSERT_VALID(m_pGM);

		CBCGPGraphicsManager* pGM = m_pGM;
		m_pGM->DestroyGeometry(*this);

		m_pGM = pGM;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBCGPRectangleGeometry

void CBCGPRectangleGeometry::Scale(double dblRatioX, double dblRatioY, const CBCGPPoint& ptOffset)
{
	Destroy ();
	m_rect.Scale(dblRatioX, dblRatioY, ptOffset);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBCGPRoundedRectangleGeometry

void CBCGPRoundedRectangleGeometry::Scale(double dblRatioX, double dblRatioY, const CBCGPPoint& ptOffset)
{
	Destroy ();
	m_rectRounded.Scale(dblRatioX, dblRatioY, ptOffset);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBCGPEllipseGeometry

void CBCGPEllipseGeometry::Scale(double dblRatioX, double dblRatioY, const CBCGPPoint& ptOffset)
{
	Destroy ();
	m_Ellipse.Scale(dblRatioX, dblRatioY, ptOffset);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBCGPPolygonGeometry

void CBCGPPolygonGeometry::Clear()
{
	m_bIsClosed = TRUE;
	m_curveType = BCGP_CURVE_TYPE_LINE;

	m_arPoints.RemoveAll();

	if (m_pGM != NULL)
	{
		ASSERT_VALID(m_pGM);

		CBCGPGraphicsManager* pGM = m_pGM;
		m_pGM->DestroyGeometry(*this);

		m_pGM = pGM;
	}
}

void CBCGPPolygonGeometry::Scale(double dblRatioX, double dblRatioY, const CBCGPPoint& ptOffset)
{
	Destroy ();
	m_arPoints.Scale (ptOffset, dblRatioX, dblRatioY, 1.0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// CBCGPEllipse

double CBCGPEllipse::A() const
{
    double a = fabs(radiusX);
    double b = fabs(radiusY);
    return b < a ? a : b;
}

double CBCGPEllipse::B() const
{
    double a = fabs(radiusX);
    double b = fabs(radiusY);
    return b < a ? b : a;
}

double CBCGPEllipse::E() const
{
    double a = A();
    double b = B();
    if (a == 0.0 || b == 0.0)
    {
        return 1.0;
    }

    return sqrt(1.0 - bcg_sqr(b / a));
}

double CBCGPEllipse::C() const
{
    return A() * E();
}

double CBCGPEllipse::P() const
{
    double a = A();
    if (a == 0.0)
    {
        return 0.0;
    }

    return bcg_sqr(B()) / a;
}

double CBCGPEllipse::Length() const
{
    if (radiusX == 0.0 && radiusY == 0.0)
    {
        return 0.0;
    }

	if (radiusX == radiusY)
	{
		return 2.0 * M_PI * radiusX;
	}

    double _x = M_LN2 / log(M_PI_2);
    return 4.0 * pow(pow((double)A(), _x) + pow((double)B(), _x), 1.0 / _x);
}

double CBCGPEllipse::Area() const
{
    if (IsNull())
    {
        return 0.0;
    }

    return M_PI * A() * B();
}

double CBCGPEllipse::GetRadius(double dblAngle) const
{
	if (radiusX == radiusY)
	{
		return radiusX;
	}

	double cosA = cos(dblAngle);
	double sinA = sin(dblAngle);
	double dist = bcg_distance(radiusY * cosA, radiusX * sinA);

	if (dist < DBL_EPSILON)
	{
		return 0.0;
	}

	return (radiusX * radiusY) / dist;
}

CBCGPPoint CBCGPEllipse::GetPoint(double dblAngle, BOOL bInvertY/* = TRUE*/) const
{
	CBCGPPoint pt(point);

	double cosA = cos(dblAngle);
	double sinA = sin(dblAngle);

	double r = radiusX;
	if (radiusX != radiusY)
	{
		r = 0.0;
		double dist = bcg_distance(radiusY * cosA, radiusX * sinA);
		if (dist > DBL_EPSILON)
		{
			r = (radiusX * radiusY) / dist;
		}
	}

	pt.x += r * cosA;

	if (bInvertY)
	{
		pt.y -= r * sinA;
	}
	else
	{
		pt.y += r * sinA;
	}

	return pt;
}

