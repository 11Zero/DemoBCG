//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPDiagramShape.cpp: implementation of the CBCGPDiagramShape class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPDiagramShape.h"
#include "BCGPMath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define BCGP_DIAGRAM_TEXT_PADDING	2

IMPLEMENT_DYNCREATE(CBCGPDiagramShape, CBCGPDiagramVisualObject)
IMPLEMENT_DYNCREATE(CBCGPDiagramCustomShape, CBCGPDiagramShape)
IMPLEMENT_DYNCREATE(CBCGPDiagramImageObject, CBCGPDiagramVisualObject)
IMPLEMENT_DYNCREATE(CBCGPDiagramTableShape, CBCGPDiagramShape)

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramShape

CBCGPDiagramShape::CBCGPDiagramShape ()
{
	m_shape = Box;
	m_pGeometry = NULL;
	m_pGeometryShadow = NULL;
}

CBCGPDiagramShape::CBCGPDiagramShape(const CBCGPRect& rect, const CBCGPBrush& brFill, const CBCGPBrush& brOutline, const Shape shape, const CBCGPBrush& brShadow)
{
	m_brOutline = brOutline;
	m_brFill = brFill;
	m_shape = shape;
	m_brShadow = brShadow;
	
	m_pGeometry = NULL;
	m_pGeometryShadow = NULL;
	
	SetRect(rect);
}

CBCGPDiagramShape::CBCGPDiagramShape(const CBCGPDiagramShape& src)
{
	m_pGeometry = NULL;
	m_pGeometryShadow = NULL;
	
	CopyFrom(src);
}

CBCGPDiagramShape::~CBCGPDiagramShape ()
{
	CleanUp ();
}
//*******************************************************************************
void CBCGPDiagramShape::CleanUp ()
{
	if (m_pGeometry != NULL)
	{
		m_pGeometry->Destroy ();

		delete m_pGeometry;
		m_pGeometry = NULL;
	}

	if (m_pGeometryShadow != NULL)
	{
		m_pGeometryShadow->Destroy ();

		delete m_pGeometryShadow;
		m_pGeometryShadow = NULL;
	}
}
//*******************************************************************************
void CBCGPDiagramShape::SetShape (Shape shape)
{
	m_shape = shape;

	SetDirty ();
}
//*******************************************************************************
void CBCGPDiagramShape::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags)
{
	if ((dwFlags & BCGP_DRAW_STATIC) != BCGP_DRAW_STATIC)
	{
		return;
	}
	
	ASSERT_VALID(pGM);

	double scaleRatio = GetScaleRatioMid();
	int nShadowDepth = pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY) ? 3 : 0;
	
	CBCGPRect rectShadow = m_rect;
	rectShadow.OffsetRect(nShadowDepth * m_sizeScaleRatio.cx, nShadowDepth * m_sizeScaleRatio.cy);
	
	if (IsDirty())
	{
		CleanUp ();
		
		m_pGeometry = CreateGeometry (pGM);
		
		if (nShadowDepth > 0)
		{
			m_pGeometryShadow = CreateGeometry (pGM, TRUE);
		}
		
		SetDirty(FALSE);
	}
	
	if (m_pGeometryShadow != NULL && nShadowDepth > 0)
	{
		ASSERT_VALID (m_pGeometryShadow);
		
		pGM->FillGeometry(*m_pGeometryShadow, m_brShadow);
	}
	
	if (m_pGeometry != NULL)
	{
		ASSERT_VALID (m_pGeometry);
		
		pGM->FillGeometry(*m_pGeometry, m_brFill);
		pGM->DrawGeometry(*m_pGeometry, m_brOutline, m_Thickness * scaleRatio, &m_StrokeStyle);
	}

	if (m_pGeometry == NULL)
	{
		switch (m_shape)
		{
		case Box:
			{
				CBCGPSize sizeRadius(5 * m_sizeScaleRatio.cx, 5 * m_sizeScaleRatio.cy);

				CBCGPRoundedRect rrShadow(rectShadow, sizeRadius.cx, sizeRadius.cy);
				CBCGPRoundedRect rr(m_rect, sizeRadius.cx, sizeRadius.cy);
				
				if (nShadowDepth > 0)
				{
					pGM->FillRoundedRectangle(rrShadow, m_brShadow);
				}
				
				pGM->FillRoundedRectangle(rr, m_brFill);
				pGM->DrawRoundedRectangle(rr, m_brOutline, m_Thickness * scaleRatio, &m_StrokeStyle);
			}
			break;
			
		case Ellipse:
			if (nShadowDepth > 0)
			{
				pGM->FillEllipse(rectShadow, m_brShadow);
			}
			
			pGM->FillEllipse(m_rect, m_brFill);
			pGM->DrawEllipse(m_rect, m_brOutline, scaleRatio);
			
			if (nShadowDepth == 0)
			{
				CBCGPRect r1 = m_rect;
				r1.DeflateRect(m_sizeScaleRatio.cx, m_sizeScaleRatio.cy);
				
				pGM->DrawEllipse(r1, m_brOutline, m_Thickness * scaleRatio, &m_StrokeStyle);
			}
			
			break;

		case RoundBox:
			{
				double dx = min (m_rect.Width () / 2, m_rect.Height () / 2);

				CBCGPRoundedRect rrShadow(rectShadow, dx, dx);
				CBCGPRoundedRect rr(m_rect, dx, dx);
				
				if (nShadowDepth > 0)
				{
					pGM->FillRoundedRectangle(rrShadow, m_brShadow);
				}
				
				pGM->FillRoundedRectangle(rr, m_brFill);
				pGM->DrawRoundedRectangle(rr, m_brOutline, m_Thickness * scaleRatio, &m_StrokeStyle);
			}
			break;

		case Rectangle:
			{
				CBCGPRect rrShadow(rectShadow);
				CBCGPRect rr(m_rect);
				
				if (nShadowDepth > 0)
				{
					pGM->FillRectangle(rrShadow, m_brShadow);
				}
				
				pGM->FillRectangle(rr, m_brFill);
				pGM->DrawRectangle(rr, m_brOutline, m_Thickness * scaleRatio, &m_StrokeStyle);
			}
			break;
			
		}
	}
	
	DrawTextData (pGM, rectClip);
	SetConnectionPorts();
	//DrawConnectionPorts(pGM, m_brOutline, m_brFill);
}
//*******************************************************************************
void CBCGPDiagramShape::DrawTextData(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pGM);

	CBCGPRect rect = GetRect();
	rect.DeflateRect (BCGP_DIAGRAM_TEXT_PADDING, BCGP_DIAGRAM_TEXT_PADDING);

	CBCGPRect rectTextClip = rect;
	if (!rectTextClip.IntersectRect (rectClip))
	{
		return;
	}

	for (int i = 0; i < m_arData.GetSize (); i++)
	{
		CBCGPDiagramTextDataObject* pData = DYNAMIC_DOWNCAST(CBCGPDiagramTextDataObject, m_arData[i]);
		if (pData != NULL)
		{
			pData->Draw (pGM, rect, rectTextClip);
		}
	}
}
//*******************************************************************************
CBCGPGeometry* CBCGPDiagramShape::CreateGeometry (CBCGPGraphicsManager* pGM, BOOL bShadow)
{
	int nShadowDepth = pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY) ? 3 : 0;
	CBCGPPoint ptShadowOffset(nShadowDepth * m_sizeScaleRatio.cx, nShadowDepth * m_sizeScaleRatio.cy);

	switch (m_shape)
	{
	case Star:
		{
			CBCGPRect rect = m_rect;
			rect.NormalizedRect ();
			
			if (rect.IsRectEmpty ())
			{
				return NULL;
			}

			CBCGPPointsArray arPoints;
			arPoints.SetSize (10);
			
			double rx = rect.Width () / 2.0;
			double ry = rect.Height () / 2.0;
			double da = M_PI * 2.0 / 5.0;
			double ang = M_PI_2;
			
			int i = 0;
			for (i = 0; i < 5; i++)
			{
				arPoints[i * 2] = CBCGPPoint(rx * cos(ang), -ry * sin(ang));
				ang += da;
			}
			
			double ratio = 2.0;
			rx /= ratio;
			ry /= ratio;
			ang = M_PI_2 + da / 2.0;
			
			for (i = 0; i < 5; i++)
			{
				arPoints[i * 2 + 1] = CBCGPPoint(rx * cos(ang), -ry * sin(ang));
				ang += da;
			}
			
			arPoints += rect.CenterPoint ();
			
			if (bShadow)
			{
				arPoints += ptShadowOffset;
			}

			return new CBCGPPolygonGeometry (arPoints);
		}

	case Rhombus:
		{	
			CBCGPRect rect = m_rect;
			rect.NormalizedRect ();
			
			if (rect.IsRectEmpty ())
			{
				return NULL;
			}

			CBCGPPointsArray arPoints;
			arPoints.SetSize (4);
			arPoints[0] = CBCGPPoint(rect.left, m_rect.CenterPoint ().y);
			arPoints[1] = CBCGPPoint(rect.CenterPoint ().x, rect.top);
			arPoints[2] = CBCGPPoint(rect.right, rect.CenterPoint ().y);
			arPoints[3] = CBCGPPoint(rect.CenterPoint ().x, rect.bottom);

			if (bShadow)
			{
				arPoints += ptShadowOffset;
			}

			return new CBCGPPolygonGeometry (arPoints);
		}

	case Parallelogram:
		{
			CBCGPRect rect = m_rect;
			rect.NormalizedRect ();
			
			if (rect.IsRectEmpty ())
			{
				return NULL;
			}

			double dx = min (rect.Width (), rect.Height () / 2);
			
			CBCGPPointsArray arPoints;
			arPoints.SetSize (4);
			arPoints[0] = CBCGPPoint(rect.left + dx, m_rect.top);
			arPoints[1] = CBCGPPoint(rect.right, rect.top);
			arPoints[2] = CBCGPPoint(rect.right - dx, rect.bottom);
			arPoints[3] = CBCGPPoint(rect.left, rect.bottom);
			
			if (bShadow)
			{
				arPoints += ptShadowOffset;
			}
			
			return new CBCGPPolygonGeometry (arPoints);
		}

	case Trapezoid:
		{
			CBCGPRect rect = m_rect;
			rect.NormalizedRect ();
			
			if (rect.IsRectEmpty ())
			{
				return NULL;
			}
			
			double dx = min (rect.Width () / 4., rect.Height () / 2.);
			
			CBCGPPointsArray arPoints;
			arPoints.SetSize (4);
			arPoints[0] = CBCGPPoint(rect.left + dx, m_rect.top);
			arPoints[1] = CBCGPPoint(rect.right - dx, rect.top);
			arPoints[2] = CBCGPPoint(rect.right, rect.bottom);
			arPoints[3] = CBCGPPoint(rect.left, rect.bottom);
			
			if (bShadow)
			{
				arPoints += ptShadowOffset;
			}
			
			return new CBCGPPolygonGeometry (arPoints);
		}

	case Cloud:
		{
			CBCGPRect rect = m_rect;
			rect.NormalizedRect ();
			
			if (rect.IsRectEmpty ())
			{
				return NULL;
			}
			
			CBCGPComplexGeometry* pGeometry = new CBCGPComplexGeometry;
	
			double rx = rect.Width () / 2.0;
			double ry = rect.Height () / 2.0;
			double da = M_PI * 2.0 / 8.0;
			double ang = M_PI_2 + da / 2.0;

			double ratio = 1.4;
			double rx_ = rx / ratio;
			double ry_ = ry / ratio;
			double da_ = da / 8.0;

			CBCGPPoint pointStart (rx_ * cos(ang), -ry_ * sin(ang));
			pointStart += rect.CenterPoint ();
			
			if (bShadow)
			{
				pointStart += ptShadowOffset;
			}

			pGeometry->SetStart (pointStart);

			CBCGPPointsArray arPoints;
			arPoints.SetSize (3);

			for (int i = 0; i < 8; i++)
			{
				arPoints[0] = CBCGPPoint(rx * cos(ang + da_), -ry * sin(ang + da_));

				ang += da;
				if (i % 4 == 1)
				{
					ang += da / 2.0;
				}
				else
				{
					ang -= da / 6.0;
				}

				arPoints[1] = CBCGPPoint(rx * cos(ang - da_), -ry * sin(ang - da_));
				arPoints[2] = CBCGPPoint(rx_ * cos(ang), -ry_ * sin(ang));

				arPoints += rect.CenterPoint ();

				if (bShadow)
				{
					arPoints += ptShadowOffset;
				}

				pGeometry->AddBezier (arPoints[0], arPoints[1], arPoints[2]);
			}

			return pGeometry;
		}
	}

	return NULL;
}
//*******************************************************************************
void CBCGPDiagramShape::SetConnectionPorts()
{
	if (m_shape == Rhombus)
	{
		const CBCGPRect& rect = GetRect();
		const CBCGPSize sizeMin = GetMinSize();
		
		if (!sizeMin.IsEmpty())
		{
			if (rect.Width() < sizeMin.cx || rect.Height() < sizeMin.cy)
			{
				return;
			}
		}
		
		m_mapConnectionPorts[CP_Center] = rect.CenterPoint();

		m_mapConnectionPorts[CP_Left] = CBCGPPoint (rect.left, rect.CenterPoint().y);
		m_mapConnectionPorts[CP_Right] = CBCGPPoint (rect.right, rect.CenterPoint().y);
		m_mapConnectionPorts[CP_Top] = CBCGPPoint (rect.CenterPoint().x, rect.top);
		m_mapConnectionPorts[CP_Bottom] = CBCGPPoint (rect.CenterPoint().x, rect.bottom);

		return;
	}

	CBCGPDiagramVisualObject::SetConnectionPorts ();
}
//*******************************************************************************
void CBCGPDiagramShape::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPDiagramVisualObject::CopyFrom(srcObj);

	const CBCGPDiagramShape& src = (const CBCGPDiagramShape&)srcObj;

	m_brOutline = src.m_brOutline;
	m_brFill = src.m_brFill;
	m_shape = src.m_shape;
	m_brShadow = src.m_brShadow;
}

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramCustomShape

CBCGPDiagramCustomShape::CBCGPDiagramCustomShape()
{
	SetShape (CBCGPDiagramShape::Custom);
	m_rectCanvas.SetRectEmpty ();
}

CBCGPDiagramCustomShape::CBCGPDiagramCustomShape(const CBCGPRect& rect, const CBCGPBrush& brFill, const CBCGPBrush& brOutline, const CBCGPBrush& brShadow)
	: CBCGPDiagramShape(rect, brFill, brOutline, CBCGPDiagramShape::Custom, brShadow)
{
	m_rectCanvas.SetRectEmpty ();
}

CBCGPDiagramCustomShape::CBCGPDiagramCustomShape(const CBCGPDiagramCustomShape& src)
{
	CopyFrom (src);
}

CBCGPDiagramCustomShape::~CBCGPDiagramCustomShape ()
{
	CleanUp ();
}
//*******************************************************************************
void CBCGPDiagramCustomShape::CleanUp (BOOL bAll)
{
	int i;
	if (bAll)
	{
		for (i = 0; i < (int)m_arrParts.GetSize (); i++)
		{
			if (m_arrParts[i] != NULL)
			{
				delete m_arrParts[i];
				m_arrParts[i] = NULL;
			}
		}
	}

	for (i = 0; i < (int)m_arrGeometries.GetSize (); i++)
	{
		if (m_arrGeometries[i] != NULL)
		{
			m_arrGeometries[i]->Destroy ();
			delete m_arrGeometries[i];
			m_arrGeometries[i] = NULL;
		}
	}

	for (i = 0; i < (int)m_arrGeometriesShadow.GetSize (); i++)
	{
		if (m_arrGeometriesShadow[i] != NULL)
		{
			m_arrGeometriesShadow[i]->Destroy ();
			delete m_arrGeometriesShadow[i];
			m_arrGeometriesShadow[i] = NULL;
		}
	}
}
//*******************************************************************************
void CBCGPDiagramCustomShape::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags)
{
	if ((dwFlags & BCGP_DRAW_STATIC) != BCGP_DRAW_STATIC)
	{
		return;
	}
	
	ASSERT_VALID(pGM);

	BOOL bShadowSupported = pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY);
	
	if (IsDirty())
	{
		CleanUp (FALSE);
		
		CreateGeometry (pGM);
		
		if (bShadowSupported)
		{
			CreateGeometry (pGM, TRUE);
		}
		
		SetDirty(FALSE);
	}

	double scaleRatio = GetScaleRatioMid();

	int i;
	for (i = 0; i < (int) m_arrGeometriesShadow.GetSize (); i++)
	{
		const CBCGPComplexGeometry* pGeometryShadow = m_arrGeometriesShadow[i];
		if (pGeometryShadow != NULL && bShadowSupported)
		{
			ASSERT_VALID (pGeometryShadow);
			
			pGM->FillGeometry(*pGeometryShadow, m_brShadow);
		}
	}

	for (i = 0; i < (int) m_arrGeometries.GetSize (); i++)
	{
		const CBCGPComplexGeometry* pGeometry = m_arrGeometries[i];
		if (pGeometry != NULL)
		{
			ASSERT_VALID (pGeometry);
			
			pGM->FillGeometry(*pGeometry, m_brFill);
			pGM->DrawGeometry(*pGeometry, m_brOutline, m_Thickness * scaleRatio, &m_StrokeStyle);
		}
	}
	
	DrawTextData (pGM, rectClip);
	SetConnectionPorts();
}
//*******************************************************************************
CBCGPGeometry* CBCGPDiagramCustomShape::CreateGeometry (CBCGPGraphicsManager* pGM, BOOL bShadow)
{
	BOOL bIsShadowSupported = pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY);

	if (bShadow && !bIsShadowSupported)
	{
		return NULL;
	}

	const int size = (int) m_arrParts.GetSize ();
	for (int i = 0; i < size; i++)
	{
		CustomShapePart* pPart = m_arrParts[i];
		if (pPart == NULL)
		{
			continue;
		}

		CBCGPPoint ptOffset = m_rect.TopLeft ();
		if (pPart->m_bShadow && bIsShadowSupported)
		{
			ptOffset.x += pPart->m_ptShadowOffset.x * m_sizeScaleRatio.cx;
			ptOffset.y += pPart->m_ptShadowOffset.y * m_sizeScaleRatio.cy;
		}

		CBCGPSize szScaleRatio (
			(m_rectCanvas.Width () == 0.0) ? 1.0: m_rect.Width () / m_rectCanvas.Width (),
			(m_rectCanvas.Height () == 0.0) ? 1.0: m_rect.Height () / m_rectCanvas.Height ());

		CBCGPComplexGeometry* pGeometry = pPart->CreateGeometry (pGM, ptOffset, szScaleRatio);
		if (pGeometry == NULL)
		{
			continue;
		}

		ASSERT_VALID(pGeometry);

		if (pPart->m_bShadow)
		{
			if (bShadow && bIsShadowSupported)
			{
				m_arrGeometriesShadow.Add (pGeometry);
			}
			else
			{
				pGeometry->Destroy ();
				delete pGeometry;
			}
		}
		else
		{
			m_arrGeometries.Add (pGeometry);
		}
	}

	return NULL;
}
//*******************************************************************************
void CBCGPDiagramCustomShape::CopyFrom (const CBCGPBaseVisualObject& srcObj)
{
	CleanUp ();

	CBCGPDiagramShape::CopyFrom(srcObj);

	const CBCGPDiagramCustomShape& src = (const CBCGPDiagramCustomShape&)srcObj;

	m_rectCanvas = src.m_rectCanvas;

	const int size = (int) src.m_arrParts.GetSize ();
	m_arrParts.SetSize (size);

	for (int i = 0; i < size; i++)
	{
		m_arrParts[i] = NULL;

		const CustomShapePart* pSrc = src.m_arrParts[i];
		if (pSrc != NULL)
		{
			m_arrParts[i] = new CustomShapePart (*pSrc);
		}
	}
}
//*******************************************************************************
CBCGPDiagramCustomShape::BaseSegment::BaseSegment(const BaseSegment& src)
{
	Copy(src);
}

CBCGPDiagramCustomShape::LineSegment::LineSegment(const CBCGPPoint& pt)
{
	Add(pt);
}

CBCGPDiagramCustomShape::LineSegment::LineSegment(const LineSegment& src) : BaseSegment (src)
{
}

BOOL CBCGPDiagramCustomShape::LineSegment::AddToGeometry (CBCGPComplexGeometry* pGeometry, const CBCGPPoint& ptOffset, const CBCGPSize& szScale)
{
	ASSERT_VALID (pGeometry);

	if (GetSize () < 1)
	{
		return FALSE;
	}

	CBCGPPoint pt = ElementAt(0);
	pt.Offset (ptOffset);
	pt.Scale (szScale.cx, szScale.cy, 0.0, ptOffset);

	pGeometry->AddLine (pt);
	return TRUE;
}

CBCGPDiagramCustomShape::BezierSegment::BezierSegment(const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3)
{
	Add(pt1);
	Add(pt2);
	Add(pt3);
}

CBCGPDiagramCustomShape::BezierSegment::BezierSegment(const BezierSegment& src) : BaseSegment (src)
{
}

BOOL CBCGPDiagramCustomShape::BezierSegment::AddToGeometry (CBCGPComplexGeometry* pGeometry, const CBCGPPoint& ptOffset, const CBCGPSize& szScale)
{
	ASSERT_VALID (pGeometry);

	CBCGPPointsArray ar;
	ar.Copy (*this);
	ar.Offset (ptOffset);
	ar.Scale (szScale, ptOffset);

	if (ar.GetSize () < 3)
	{
		return FALSE;
	}

	pGeometry->AddBezier (ar[0], ar[1], ar[2]);
	return TRUE;
}

CBCGPDiagramCustomShape::ArcSegment::ArcSegment()
{
	m_szRadius = CBCGPSize();
	m_bIsClockwise = TRUE;
	m_bIsLargeArc = FALSE;
	m_dblRotationAngle = 0.;
}

CBCGPDiagramCustomShape::ArcSegment::ArcSegment(const CBCGPPoint& pt, const CBCGPSize szRadius,
	BOOL bIsClockwise, BOOL bIsLargeArc, double dblRotationAngle)
{
	Add(pt);
	m_szRadius = szRadius;
	m_bIsClockwise = bIsClockwise;
	m_bIsLargeArc = bIsLargeArc;
	m_dblRotationAngle = dblRotationAngle;
}

CBCGPDiagramCustomShape::ArcSegment::ArcSegment(const ArcSegment& src) : BaseSegment (src)
{
	m_szRadius = src.m_szRadius;
	m_bIsClockwise = src.m_bIsClockwise;
	m_bIsLargeArc = src.m_bIsLargeArc;
	m_dblRotationAngle = src.m_dblRotationAngle;
}

BOOL CBCGPDiagramCustomShape::ArcSegment::AddToGeometry (CBCGPComplexGeometry* pGeometry, const CBCGPPoint& ptOffset, const CBCGPSize& szScale)
{
	ASSERT_VALID (pGeometry);

	if (GetSize () < 1)
	{
		return FALSE;
	}

	CBCGPPoint pt = ElementAt(0);
	pt.Offset (ptOffset);
	pt.Scale (szScale.cx, szScale.cy, 0.0, ptOffset);

	CBCGPSize szRadius (m_szRadius.cx * szScale.cx, m_szRadius.cy * szScale.cy);

	pGeometry->AddArc (pt, szRadius, m_bIsClockwise, m_bIsLargeArc, m_dblRotationAngle);
	return TRUE;
}

CBCGPDiagramCustomShape::PointsSegment::PointsSegment()
{
	m_curveType = CBCGPPolygonGeometry::BCGP_CURVE_TYPE_LINE;
}

CBCGPDiagramCustomShape::PointsSegment::PointsSegment(const CBCGPPointsArray& arPoints, CBCGPPolygonGeometry::BCGP_CURVE_TYPE curveType)
{
	Append (arPoints);
	m_curveType = curveType;
}

CBCGPDiagramCustomShape::PointsSegment::PointsSegment(const PointsSegment& src) : BaseSegment (src)
{
	m_curveType = src.m_curveType;
}

BOOL CBCGPDiagramCustomShape::PointsSegment::AddToGeometry (CBCGPComplexGeometry* pGeometry, const CBCGPPoint& ptOffset, const CBCGPSize& szScale)
{
	ASSERT_VALID (pGeometry);

	CBCGPPointsArray ar;
	ar.Copy (*this);
	ar.Offset (ptOffset);
	ar.Scale (szScale, ptOffset);

	if (ar.GetSize () > 0)
	{
		pGeometry->AddPoints (ar, m_curveType);
		return TRUE;
	}

	return FALSE;
}

CBCGPDiagramCustomShape::CustomShapePart::CustomShapePart(BOOL bShadow, const CBCGPPoint& ptShadowOffset, const CBCGPPoint& ptStart, BOOL bIsClosed)
{
	m_bIsClosed = bIsClosed;
	m_ptStart = ptStart;
	m_bShadow = bShadow;
	m_ptShadowOffset = ptShadowOffset;
}

CBCGPDiagramCustomShape::CustomShapePart::CustomShapePart(const CustomShapePart& src)
{
	m_bIsClosed = src.m_bIsClosed;
	m_ptStart = src.m_ptStart;
	m_bShadow = src.m_bShadow;
	m_ptShadowOffset = src.m_ptShadowOffset;

	int size = (int) src.m_arrSegments.GetSize ();
	for (int i = 0; i < size; i++)
	{
		m_arrSegments.Add (src.m_arrSegments[i]->CreateCopy ());
	}
}

CBCGPDiagramCustomShape::CustomShapePart::~CustomShapePart()
{
	int size = (int) m_arrSegments.GetSize ();
	for (int i = 0; i < size; i++)
	{
		delete m_arrSegments[i];
		m_arrSegments[i] = NULL;
	}
}

CBCGPComplexGeometry* CBCGPDiagramCustomShape::CustomShapePart::CreateGeometry (CBCGPGraphicsManager* pGM, const CBCGPPoint& ptOffset, const CBCGPSize& szScale)
{
	if (pGM == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(pGM);

	if (m_bShadow && !pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY))
	{
		return NULL;
	}

	CBCGPComplexGeometry* pGeometry = new CBCGPComplexGeometry;

	CBCGPPoint ptStart = m_ptStart;
	ptStart.Offset (ptOffset);
	ptStart.Scale (szScale.cx, szScale.cy, 0.0, ptOffset);

	pGeometry->SetStart (ptStart);
	pGeometry->SetClosed (m_bIsClosed);

	for (int i = 0; i < (int)m_arrSegments.GetSize (); i++)
	{
		BaseSegment* pSegment = m_arrSegments[i];
		if (pSegment != NULL)
		{
			ASSERT_VALID(pSegment);

			if (!pSegment->AddToGeometry (pGeometry, ptOffset, szScale))
			{
				break;
			}
		}
	}

	return pGeometry;
}

void CBCGPDiagramCustomShape::CustomShapePart::Offset (const CBCGPPoint& pt)
{
	m_ptStart.Offset (pt);

	const int size = (int) m_arrSegments.GetSize ();
	for (int i = 0; i < size; i++)
	{
		BaseSegment* pSegment = m_arrSegments[i];
		if (pSegment != NULL)
		{
			pSegment->Offset (pt);
		}
	}
}
//*******************************************************************************
void CBCGPDiagramCustomShape::StartPart (BOOL bShadow, const CBCGPPoint& ptShadowOffset, const CBCGPPoint& ptStart, BOOL bIsClosed)
{
	CBCGPPoint ptOffset = -m_rectCanvas.TopLeft ();

	m_arrParts.Add (new CustomShapePart(bShadow, ptShadowOffset, ptStart + ptOffset, bIsClosed));
}
//*******************************************************************************
void CBCGPDiagramCustomShape::SetClosed(BOOL bIsClosed)
{
	if (m_arrParts.GetSize () == 0)
	{
		return;
	}

	CustomShapePart* pPart = m_arrParts[m_arrParts.GetUpperBound ()];
	ASSERT(pPart != NULL);

	pPart->m_bIsClosed = bIsClosed;
}
//*******************************************************************************
void CBCGPDiagramCustomShape::SetStart(const CBCGPPoint& ptStart)
{
	if (m_arrParts.GetSize () == 0)
	{
		return;
	}

	CBCGPPoint ptOffset = -m_rectCanvas.TopLeft ();

	CustomShapePart* pPart = m_arrParts[m_arrParts.GetUpperBound ()];
	ASSERT(pPart != NULL);

	pPart->m_ptStart = ptStart + ptOffset;
}
//*******************************************************************************
void CBCGPDiagramCustomShape::AddLine(const CBCGPPoint& pt)
{
	if (m_arrParts.GetSize () == 0)
	{
		return;
	}

	CBCGPPoint ptOffset = -m_rectCanvas.TopLeft ();

	CustomShapePart* pPart = m_arrParts[m_arrParts.GetUpperBound ()];
	ASSERT(pPart != NULL);

	pPart->m_arrSegments.Add (new LineSegment(pt + ptOffset));
}
//*******************************************************************************
void CBCGPDiagramCustomShape::AddBezier(const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3)
{
	if (m_arrParts.GetSize () == 0)
	{
		return;
	}

	CBCGPPoint ptOffset = -m_rectCanvas.TopLeft ();

	CustomShapePart* pPart = m_arrParts[m_arrParts.GetUpperBound ()];
	ASSERT(pPart != NULL);

	pPart->m_arrSegments.Add (new BezierSegment(pt1 + ptOffset, pt2 + ptOffset, pt3 + ptOffset));
}
//*******************************************************************************
void CBCGPDiagramCustomShape::AddArc(const CBCGPPoint& pt, const CBCGPSize szRadius, 
	BOOL bIsClockwise, BOOL bIsLargeArc, double dblRotationAngle)
{
	if (m_arrParts.GetSize () == 0)
	{
		return;
	}

	CBCGPPoint ptOffset = -m_rectCanvas.TopLeft ();

	CustomShapePart* pPart = m_arrParts[m_arrParts.GetUpperBound ()];
	ASSERT(pPart != NULL);

	pPart->m_arrSegments.Add (
		new ArcSegment(pt + ptOffset, szRadius, bIsClockwise, bIsLargeArc, dblRotationAngle));
}
//*******************************************************************************
void CBCGPDiagramCustomShape::AddPoints(const CBCGPPointsArray& arPoints, CBCGPPolygonGeometry::BCGP_CURVE_TYPE curveType)
{
	if (m_arrParts.GetSize () == 0)
	{
		return;
	}

	CBCGPPoint ptOffset = -m_rectCanvas.TopLeft ();

	CustomShapePart* pPart = m_arrParts[m_arrParts.GetUpperBound ()];
	ASSERT(pPart != NULL);

	PointsSegment* pPointsSegment = new PointsSegment(arPoints, curveType);
	pPointsSegment->Offset (ptOffset);

	pPart->m_arrSegments.Add (pPointsSegment);
}
//*******************************************************************************
void CBCGPDiagramCustomShape::SetCanvasRect (const CBCGPRect& rect)
{
	if (m_rectCanvas == rect)
	{
		return;
	}

	CBCGPPoint ptOffset = m_rectCanvas.TopLeft () - rect.TopLeft ();

	const int size = (int) m_arrParts.GetSize ();
	for (int i = 0; i < size; i++)
	{
		CustomShapePart* pPart = m_arrParts[i];
		if (pPart != NULL)
		{
			pPart->Offset (ptOffset);
		}
	}

	m_rectCanvas = rect;
}

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramImageObject

CBCGPDiagramImageObject::CBCGPDiagramImageObject()
{
	Init ();
}

CBCGPDiagramImageObject::CBCGPDiagramImageObject(UINT uiImageResID)
{
	Init ();
	SetImage (uiImageResID, FALSE);
}

CBCGPDiagramImageObject::CBCGPDiagramImageObject(const CBCGPImage& image)
{
	Init ();
	SetImage (image, FALSE);
}

CBCGPDiagramImageObject::CBCGPDiagramImageObject(const CBCGPDiagramImageObject& src)
{
	Init ();

	CopyFrom(src);
}

CBCGPDiagramImageObject::CBCGPDiagramImageObject(const CBCGPRect& rect)
{
	Init ();
	SetRect(rect);
}

CBCGPDiagramImageObject::~CBCGPDiagramImageObject ()
{
}
//*******************************************************************************
void CBCGPDiagramImageObject::Init ()
{
	SetFillBrush (CBCGPBrush());
	SetOutlineBrush (CBCGPBrush());

	m_VerticalAlign = VA_Top;
	m_HorizontalAlign = HA_Left;
	m_bLockAspectRatio = TRUE;
}
//*******************************************************************************
void CBCGPDiagramImageObject::SetImage (UINT uiImageResID, BOOL bRedraw)
{
	m_Image.Destroy ();
	m_Image = CBCGPImage (uiImageResID);
	
	SetDirty();
	
	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPDiagramImageObject::SetImage (const CBCGPImage& image, BOOL bRedraw)
{
	m_Image.Destroy ();
	m_Image = image;
	
	SetDirty();
	
	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPDiagramImageObject::SetImageAlign (HorizontalAlign horzAlign, VerticalAlign vertAlign, BOOL bLockAspectRatio, BOOL bRedraw)
{
	m_VerticalAlign = vertAlign;
	m_HorizontalAlign = horzAlign;
	m_bLockAspectRatio = bLockAspectRatio;

	SetDirty();
	
	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPDiagramImageObject::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectClip*/, DWORD dwFlags)
{
	if (m_rect.IsRectEmpty() || !m_bIsVisible)
	{
		return;
	}

	if ((dwFlags & BCGP_DRAW_STATIC) != BCGP_DRAW_STATIC)
	{
		return;
	}

	pGM->FillRectangle (m_rect, GetFillBrush ());

	CBCGPRect rectSrc = CBCGPRect(CBCGPPoint(0.0, 0.0), m_Image.GetSize (pGM));
	if (!rectSrc.IsRectEmpty ())
	{
		if (m_sizeScaleRatio != CBCGPSize(1.0, 1.0))
		{
			rectSrc.Scale(m_sizeScaleRatio.cx, m_sizeScaleRatio.cy);
		}

		CBCGPRect rectDst = m_rect;
		CBCGPSize sizeSrc = rectSrc.Size ();

		double aspect = 1.0;
		if (m_bLockAspectRatio)
		{
			if (m_HorizontalAlign == HA_Stretch && m_VerticalAlign != VA_Stretch)
			{
				aspect = rectDst.Width() / sizeSrc.Width ();
			}
			else if (m_HorizontalAlign != HA_Stretch && m_VerticalAlign == VA_Stretch)
			{
				aspect = rectDst.Height() / sizeSrc.Height ();
			}

			sizeSrc *= aspect;
		}

		if (m_HorizontalAlign != HA_Stretch)
		{
			if (m_HorizontalAlign == HA_Left)
			{
				rectDst.right = rectDst.left + sizeSrc.Width ();
			}
			else if (m_HorizontalAlign == HA_Right)
			{
				rectDst.left = rectDst.right - sizeSrc.Width ();
			}
			else if (m_HorizontalAlign == HA_Center)
			{
				rectDst.left += (rectDst.Width() - sizeSrc.Width ()) / 2;
				rectDst.right = rectDst.left + sizeSrc.Width ();
			}

			CBCGPRect rt (rectDst);
			rectDst.IntersectRect (rectDst, m_rect);

			if (0 < rectDst.Width () && rectDst.Width () != rectSrc.Width ())
			{
				rectSrc.left += (rectDst.left - rt.left) / aspect;
				rectSrc.right = rectSrc.left + min(rectDst.Width () / aspect, rectSrc.Width ());
			}
		}

		if (m_VerticalAlign != VA_Stretch)
		{
			if (m_VerticalAlign == VA_Top)
			{
				rectDst.bottom = rectDst.top + sizeSrc.Height ();
			}
			else if (m_VerticalAlign == VA_Bottom)
			{
				rectDst.top = rectDst.bottom - sizeSrc.Height ();
			}
			else if (m_VerticalAlign == VA_Center)
			{
				rectDst.top += (rectDst.Height() - sizeSrc.Height ()) / 2;
				rectDst.bottom = rectDst.top + sizeSrc.Height ();
			}
		
			CBCGPRect rt (rectDst);
			rectDst.IntersectRect (rectDst, m_rect);

			if (0 < rectDst.Height () && rectDst.Height () != rectSrc.Height ())
			{
				rectSrc.top += (rectDst.top - rt.top) / aspect;
				rectSrc.bottom = rectSrc.top + min(rectDst.Height () / aspect, rectSrc.Height ());
			}
		}

		if (m_sizeScaleRatio != CBCGPSize(1.0, 1.0))
		{
			rectSrc.Scale(1.0 / m_sizeScaleRatio.cx, 1.0 / m_sizeScaleRatio.cy);
		}		

		if (!rectSrc.IsRectEmpty () && !rectDst.IsRectEmpty ())
		{
			pGM->DrawImage(m_Image, rectDst.TopLeft(), rectDst.Size(), 1.0, CBCGPImage::BCGP_IMAGE_INTERPOLATION_MODE_LINEAR, rectSrc);
		}
	}

	if (!GetOutlineBrush ().IsEmpty ())
	{
		pGM->DrawRectangle (m_rect, GetOutlineBrush (), m_Thickness * GetScaleRatioMid (), &m_StrokeStyle);
	}

	SetDirty(FALSE);
}
//*******************************************************************************
CBCGPSize CBCGPDiagramImageObject::GetImageSize(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(pGM);
	return m_Image.GetSize (pGM);
}
//*******************************************************************************
void CBCGPDiagramImageObject::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPDiagramVisualObject::CopyFrom(srcObj);

	const CBCGPDiagramImageObject& src = (const CBCGPDiagramImageObject&)srcObj;

	SetImage (src.m_Image, FALSE);

 	m_VerticalAlign = src.m_VerticalAlign;
 	m_HorizontalAlign = src.m_HorizontalAlign;
	m_bLockAspectRatio = src.m_bLockAspectRatio;
}

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramTableShape

CBCGPDiagramTableShape::CBCGPDiagramTableShape()
{
	m_bRepos = TRUE;
}

CBCGPDiagramTableShape::CBCGPDiagramTableShape (const CBCGPRect& rect, const CBCGPBrush& brFill, const CBCGPBrush& brOutline, const CBCGPBrush& brShadow)
	: CBCGPDiagramShape(rect, brFill, brOutline, CBCGPDiagramShape::Box, brShadow)
{
	m_bRepos = TRUE;

	m_bCaption = FALSE;
	m_brCaptionFill = brFill;
}
//*******************************************************************************
CBCGPDiagramTableShape::CBCGPDiagramTableShape(const CBCGPDiagramTableShape& src)
{
	CopyFrom (src);
}
//*******************************************************************************
void CBCGPDiagramTableShape::SetCaption (const CString& strText, const CBCGPColor& clrText)
{
	SetCaption (strText, clrText, m_brFill, TRUE);
}
//*******************************************************************************
void CBCGPDiagramTableShape::SetCaption (const CString& strText, const CBCGPColor& clrText, const CBCGPBrush& brCaptionFill, BOOL bShowCaption)
{
	m_CaptionData.SetText (strText, clrText);

	m_brCaptionFill = brCaptionFill;
	m_bCaption = bShowCaption;
}
//*******************************************************************************
CString CBCGPDiagramTableShape::GetCaptionText() const
{
	return m_CaptionData.GetText ();
}
//*******************************************************************************
CBCGPColor CBCGPDiagramTableShape::GetCaptionTextColor() const
{
	return m_CaptionData.GetTextColor ();
}
//*******************************************************************************
const CBCGPColor& CBCGPDiagramTableShape::GetCaptionFillColor() const
{
	return m_brCaptionFill.GetColor ();
}
//*******************************************************************************
void CBCGPDiagramTableShape::SetCaptionFillBrush(const CBCGPBrush& brush)
{
	m_brCaptionFill = brush;
}
//*******************************************************************************
const CBCGPBrush& CBCGPDiagramTableShape::GetCaptionFillBrush() const
{
	return m_brCaptionFill;
}
//*******************************************************************************
void CBCGPDiagramTableShape::EnableCaption (BOOL bEnable)
{
	m_bCaption = bEnable;
}
//*******************************************************************************
BOOL CBCGPDiagramTableShape::IsCaptionEnabled () const
{
	return m_bCaption;
}
//*******************************************************************************
UINT CBCGPDiagramTableShape::ConnectionPortID (int nRowIndex, BOOL bLeft) const
{
	if (nRowIndex > 0 && nRowIndex < m_arData.GetSize ())
	{
		return CP_CustomFirst + 2 * nRowIndex + (bLeft ? 0 : 1);
	}

	return CP_None;
}
//*******************************************************************************
void CBCGPDiagramTableShape::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags)
{
	if (!!m_brCaptionFill.IsEmpty ())
	{
		CBCGPDiagramShape::OnDraw(pGM, rectClip, dwFlags);
		//DrawConnectionPorts (pGM, m_brOutline, m_brOutline);
		return;
	}

	if ((dwFlags & BCGP_DRAW_STATIC) != BCGP_DRAW_STATIC)
	{
		return;
	}
	
	ASSERT_VALID(pGM);
	
	int nShadowDepth = pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY) ? 3 : 0;
	
	CBCGPRect rectShadow = m_rect;
	rectShadow.OffsetRect(nShadowDepth * m_sizeScaleRatio.cx, nShadowDepth * m_sizeScaleRatio.cy);
	
	switch (m_shape)
	{
	case Box:
		{
			CBCGPSize sizeRadius(5 * m_sizeScaleRatio.cx, 5 * m_sizeScaleRatio.cy);

			CBCGPRoundedRect rrShadow(rectShadow, sizeRadius.cx, sizeRadius.cy);
			CBCGPRoundedRect rr(m_rect, sizeRadius.cx, sizeRadius.cy);
			
			if (nShadowDepth > 0)
			{
				pGM->FillRoundedRectangle(rrShadow, m_brShadow);
			}

			pGM->FillRoundedRectangle(rr, m_brFill);
			
			if (m_bCaption && !m_brCaptionFill.IsEmpty ())
			{
// 				if (m_bRepos)
// 				{
// 					Repos ();
// 				}

				double cyCaption = m_CaptionData.GetSize ().cy;
				if (m_CaptionData.GetText ().IsEmpty ())
				{
					CBCGPDiagramTextDataObject textData;
					textData.CopyFrom (m_CaptionData);
					textData.SetText (_T("Wq"), FALSE);
					cyCaption = textData.GetSize ().cy;
				}

				CBCGPRect rectItemClip = GetRect ();
				rectItemClip.bottom = rectItemClip.top + cyCaption + 2 * BCGP_DIAGRAM_TEXT_PADDING * m_sizeScaleRatio.cy;
				rectItemClip.InflateRect (m_sizeScaleRatio.cx, m_sizeScaleRatio.cy);
				rectItemClip.IntersectRect (rectClip);
		
				CBCGPRoundedRect rrCaption(rectItemClip, sizeRadius.cx, sizeRadius.cy);
				CBCGPGeometry captionGeometry;
				rectItemClip.top += rrCaption.radiusY + m_sizeScaleRatio.cy;
				pGM->CombineGeometry(captionGeometry, CBCGPRoundedRectangleGeometry(rrCaption), CBCGPRectangleGeometry(rectItemClip), RGN_OR);
				pGM->FillGeometry (captionGeometry, m_brCaptionFill);
			}

			pGM->DrawRoundedRectangle(rr, m_brOutline, m_Thickness * GetScaleRatioMid (), &m_StrokeStyle);
		}
		break;

	default:
		ASSERT(FALSE);
		break;
	}
	
	DrawTextData (pGM, rectClip);
	SetConnectionPorts();
	//DrawConnectionPorts (pGM, m_brOutline, m_brOutline);
}
//*******************************************************************************
void CBCGPDiagramTableShape::DrawTextData(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pGM);

	CBCGPRect rect = GetRect();
	rect.DeflateRect (BCGP_DIAGRAM_TEXT_PADDING, BCGP_DIAGRAM_TEXT_PADDING);

	CBCGPRect rectTextClip = rect;
	if (!rectTextClip.IntersectRect (rectClip))
	{
		return;
	}

	pGM->SetClipRect (rectTextClip);

	CBCGPPoint ptOffset = rect.TopLeft ();

	//--------------
	// Draw caption:
	//--------------
	if (m_bCaption)
	{
		CBCGPSize size = m_CaptionData.GetDefaultSize(pGM, this);
		CBCGPRect rectItem(ptOffset, CBCGPSize(rect.Width (), size.cy));
		m_bRepos = FALSE;

		m_CaptionData.SetSize (size);
		m_CaptionData.Draw (pGM, rectItem, rectTextClip);

		ptOffset.y += size.cy + 2 * BCGP_DIAGRAM_TEXT_PADDING;

		if (!m_brOutline.IsEmpty ())
		{
			double dy = ptOffset.y - BCGP_DIAGRAM_TEXT_PADDING;
			pGM->DrawLine (rect.left, dy, rect.right, dy, m_brOutline, GetScaleRatioMid ());
		}
	}

	//-------------------
	// Draw data objects:
	//-------------------
	for (int i = 0; i < m_arData.GetSize (); i++)
	{
		CBCGPDiagramTextDataObject* pData = DYNAMIC_DOWNCAST(CBCGPDiagramTextDataObject, m_arData[i]);
		if (pData != NULL)
		{
			CBCGPSize size = pData->GetDefaultSize(pGM, this);
			CBCGPRect rectItem(ptOffset, CBCGPSize(rect.Width (), size.cy));
			m_bRepos = FALSE;

			pData->SetSize (size);
			pData->Draw (pGM, rectItem, rectTextClip);

			ptOffset.y += size.cy + 2 * BCGP_DIAGRAM_TEXT_PADDING;
		}

		if (ptOffset.y >= rectTextClip.bottom)
		{
			break;
		}
	}

	pGM->ReleaseClipArea ();
}
//*******************************************************************************
void CBCGPDiagramTableShape::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld)
{
	CBCGPDiagramShape::OnScaleRatioChanged(sizeScaleRatioOld);

	m_CaptionData.SetFontScale (m_sizeScaleRatio.cy);
	//m_bRepos = TRUE;
}
//*******************************************************************************
void CBCGPDiagramTableShape::SetConnectionPorts()
{
	if (m_bRepos)
	{
		Repos ();
	}

	CBCGPRect rect = GetRect();
	const CBCGPSize sizeMin = GetMinSize();
	
	if (!sizeMin.IsEmpty())
	{
		if (rect.Width() < sizeMin.cx || rect.Height() < sizeMin.cy)
		{
			return;
		}
	}
	
	rect.DeflateRect (4.0, 0);

	CBCGPPoint ptOffset = rect.TopLeft ();
	ptOffset.y += BCGP_DIAGRAM_TEXT_PADDING;

	// Caption:
	if (m_bCaption)
	{
		if (m_CaptionData.GetText ().IsEmpty ())
		{
			CBCGPDiagramTextDataObject textData;
			textData.CopyFrom (m_CaptionData);
			textData.SetText (_T("Wq"), FALSE);
			ptOffset.y += textData.GetSize ().cy + 2 * BCGP_DIAGRAM_TEXT_PADDING;
		}
		else
		{
			ptOffset.y += m_CaptionData.GetSize ().cy + 2 * BCGP_DIAGRAM_TEXT_PADDING;
		}
	}

	// Data items:
	for (int i = 0; i < m_arData.GetSize (); i++)
	{
		CBCGPDiagramTextDataObject* pData = DYNAMIC_DOWNCAST(CBCGPDiagramTextDataObject, m_arData[i]);
		if (pData != NULL)
		{
			const CBCGPSize& size = pData->GetSize ();
			CBCGPRect rectItem(ptOffset, CBCGPSize(rect.Width (), size.cy));

			double y = min (rect.bottom, rectItem.CenterPoint().y);
			m_mapConnectionPorts[CP_CustomFirst + 2 * i]	  = CBCGPPoint (rectItem.left, y);
			m_mapConnectionPorts[CP_CustomFirst + 2 * i  + 1] = CBCGPPoint (rectItem.right, y);

			ptOffset.y += size.cy + 2 * BCGP_DIAGRAM_TEXT_PADDING;
		}
	}
}
//*******************************************************************************
UINT CBCGPDiagramTableShape::HitTestConnectionPort(const CBCGPPoint& pt) const
{
	CBCGPRect rect = GetRect();
	rect.Normalize();
	
	if (!rect.PtInRect(pt))
	{
		return HTNOWHERE;
	}

	CBCGPPoint ptOffset = rect.TopLeft ();

	// Caption:
	if (m_CaptionData.GetText ().IsEmpty ())
	{
		CBCGPDiagramTextDataObject textData;
		textData.CopyFrom (m_CaptionData);
		textData.SetText (_T("Wq"), FALSE);
		ptOffset.y += textData.GetSize ().cy + 2 * BCGP_DIAGRAM_TEXT_PADDING;
	}
	else
	{
		ptOffset.y += m_CaptionData.GetSize ().cy + 2 * BCGP_DIAGRAM_TEXT_PADDING;
	}

	// Data items:
	for (int i = 0; i < m_arData.GetSize (); i++)
	{
		CBCGPDiagramTextDataObject* pData = DYNAMIC_DOWNCAST(CBCGPDiagramTextDataObject, m_arData[i]);
		if (pData != NULL)
		{
			CBCGPRect rectItem(ptOffset, CBCGPSize(rect.Width (), pData->GetSize ().cy + 2 * BCGP_DIAGRAM_TEXT_PADDING));
			if (rectItem.PtInRect (pt))
			{
				if (pt.x <= rectItem.CenterPoint ().x)
				{
					return CP_CustomFirst + 2 * i;
				}
				else
				{
					return CP_CustomFirst + 2 * i + 1;
				}
			}

			ptOffset.y += rectItem.Height ();
		}
	}

	return  HTCLIENT;
}
//*******************************************************************************
void CBCGPDiagramTableShape::Repos ()
{
	CWnd* pWnd = GetOwner ();

	if (GetParentContainer () == NULL || pWnd->GetSafeHwnd () == NULL)
	{
		return;
	}

	CWindowDC dc (pWnd); // device context for painting
	
	CBCGPMemDC memDC (dc, pWnd);
	CDC* pDC = &memDC.GetDC ();

	const CBCGPRect& rectParent = GetParentContainer ()->GetRect ();

	CBCGPGraphicsManager* pGM = CBCGPGraphicsManager::CreateInstance ();
	ASSERT_VALID(pGM);

	pGM->BindDC (pDC, rectParent);
	if (pGM->IsBindDCFailed ())
	{
		delete pGM;
		return;
	}

	CBCGPSize sizeCaption = m_CaptionData.GetDefaultSize (pGM, this);
	m_CaptionData.SetSize (sizeCaption);

	for (int i = 0; i < m_arData.GetSize (); i++)
	{
		CBCGPDiagramTextDataObject* pData = DYNAMIC_DOWNCAST(CBCGPDiagramTextDataObject, m_arData[i]);
		if (pData != NULL)
		{
			CBCGPSize size = pData->GetDefaultSize(pGM, this);
			pData->SetSize (size);
		}
	}

	pGM->CleanResources(TRUE);
	delete pGM;

	m_bRepos = FALSE;
}
//*******************************************************************************
void CBCGPDiagramTableShape::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPDiagramShape::CopyFrom(srcObj);

	const CBCGPDiagramTableShape& src = (const CBCGPDiagramTableShape&)srcObj;

	m_bRepos = src.m_bRepos;

	m_bCaption = src.m_bCaption;
	m_CaptionData.CopyFrom (src.m_CaptionData);
	m_brCaptionFill = src.m_brCaptionFill;
}
