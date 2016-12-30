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
// BCGPCircularGaugeImpl.cpp: implementation of the CBCGPCircularGaugeImpl class.
//
//////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPCircularGaugeImpl.h"

#include "BCGPMath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CBCGPCircularGaugeScale, CBCGPGaugeScaleObject)
IMPLEMENT_DYNCREATE(CBCGPCircularGaugePointer, CBCGPGaugeDataObject)
IMPLEMENT_DYNAMIC(CBCGPCircularGaugeCtrl, CBCGPVisualCtrl)

void CBCGPCircularGaugeColors::SetTheme(BCGP_CIRCULAR_GAUGE_COLOR_THEME theme)
{
	m_brFrameFillInv.Empty();

	switch (theme)
	{
	case BCGP_CIRCULAR_GAUGE_SILVER:
		m_brPointerOutline.SetColor(CBCGPColor::DarkGray);
		m_brPointerFill.SetColor(CBCGPColor::Gray);
		m_brText.SetColor(CBCGPColor::Gray);
		m_brFrameOutline.SetColor(CBCGPColor::Gray);
		m_brFrameFill.SetColors(CBCGPColor::DimGray, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brFill.SetColors(CBCGPColor::Silver, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brCapOutline.SetColor(CBCGPColor::Silver);
		m_brCapFill.SetColors(CBCGPColor::Silver, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP);
		m_brTickMarkFill.SetColors(CBCGPColor::LightGray, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP_LEFT);
		m_brTickMarkOutline.SetColor(CBCGPColor::Gray);
		break;

	case BCGP_CIRCULAR_GAUGE_BLUE:
		m_brPointerOutline.SetColor(CBCGPColor::SlateGray);
		m_brPointerFill.SetColor(CBCGPColor::DeepSkyBlue);
		m_brText.SetColor(CBCGPColor::SlateGray);
		m_brFrameOutline.SetColor(CBCGPColor::LightSteelBlue);
		m_brFrameFill.SetColors(CBCGPColor::SlateGray, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brFill.SetColors(CBCGPColor::LightSteelBlue, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brCapOutline.SetColor(CBCGPColor::LightSteelBlue);
		m_brCapFill.SetColors(CBCGPColor::LightSteelBlue, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP);
		m_brTickMarkFill.SetColors(CBCGPColor::SlateGray, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP_LEFT);
		m_brTickMarkOutline.SetColor(CBCGPColor::SlateGray);
		break;

	case BCGP_CIRCULAR_GAUGE_GOLD:
		m_brPointerOutline.SetColor(CBCGPColor::Maroon);
		m_brPointerFill.SetColor(CBCGPColor::SaddleBrown);
		m_brText.SetColor(CBCGPColor::DarkRed);
		m_brFrameOutline.SetColor(CBCGPColor::DarkGoldenrod);
		m_brFrameFill.SetColors(CBCGPColor::Peru, CBCGPColor::LightGoldenrodYellow, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brFill.SetColors(CBCGPColor::Wheat, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brCapOutline.SetColor(CBCGPColor::BurlyWood);
		m_brCapFill.SetColors(CBCGPColor::BurlyWood, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP);
		m_brTickMarkFill.SetColors(CBCGPColor::DarkGoldenrod, CBCGPColor::Wheat, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP_LEFT);
		m_brTickMarkOutline.SetColor(CBCGPColor::DarkGoldenrod);
		break;

	case BCGP_CIRCULAR_GAUGE_BLACK:
		m_brPointerOutline.SetColor(CBCGPColor::DarkGray);
		m_brPointerFill.SetColor(CBCGPColor::AntiqueWhite);
		m_brText.SetColor(CBCGPColor::AntiqueWhite);
		m_brFrameOutline.SetColor(CBCGPColor::DarkGray);
		m_brFrameFill.SetColors(CBCGPColor::Black, CBCGPColor::LightGray, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brFill.SetColor(CBCGPColor::DarkSlateGray);
		m_brCapOutline.SetColor(CBCGPColor::DarkGray);
		m_brCapFill.SetColors(CBCGPColor::DarkGray, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP);
		m_brTickMarkFill.SetColor(CBCGPColor::DarkGray);
		m_brTickMarkOutline.SetColor(CBCGPColor::AntiqueWhite);
		break;

	case BCGP_CIRCULAR_GAUGE_WHITE:
		m_brPointerOutline.SetColor(CBCGPColor::DarkRed);
		m_brPointerFill.SetColor(CBCGPColor::IndianRed);
		m_brText.SetColor(CBCGPColor::Gray);
		m_brFrameOutline.SetColor(CBCGPColor::Gray);
		m_brFrameFill.SetColors(CBCGPColor::DimGray, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brFill.SetColor(CBCGPColor::FloralWhite);
		m_brCapOutline.SetColor(CBCGPColor::Gray);
		m_brCapFill.SetColors(CBCGPColor::LightGray, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP);
		m_brTickMarkFill.SetColors(CBCGPColor::LightGray, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP_LEFT);
		m_brTickMarkOutline.SetColor(CBCGPColor::Gray);
		break;
	}
}

IMPLEMENT_DYNCREATE(CBCGPCircularGaugeImpl, CBCGPGaugeImpl)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPCircularGaugeImpl::CBCGPCircularGaugeImpl(CBCGPVisualContainer* pContainer) :
	CBCGPGaugeImpl(pContainer)
{
	m_bShapeByTicksArea = FALSE;
	m_bDrawTextBeforeTicks = FALSE;
	m_bDrawTicksOutsideFrame = FALSE;
	m_nFrameSize = 5;
	m_CapSize = 15.;
	m_ScaleSize = 0.;
	m_bCacheImage = TRUE;
	m_bInSetValue = FALSE;
	m_nCurrLabelIndex = 0;

	m_uiEditFlags = BCGP_EDIT_SIZE_LOCK_ASPECT_RATIO;

	LOGFONT lf;
	globalData.fontBold.GetLogFont(&lf);

	m_textFormat.CreateFromLogFont(lf);
	m_textFormat.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);

	// Add default scale:
	AddScale();

	// Add default pointer:
	AddPointer(CBCGPCircularGaugePointer());
}
//*******************************************************************************
CBCGPCircularGaugeImpl::~CBCGPCircularGaugeImpl()
{
	RemoveAllSubGauges();
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::SetTicksAreaAngles(double dblStartAngle, double dblFinishAngle, int nScale)	// In degrees
{
	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (pScale->m_bIsClosed)
	{
		pScale->m_dblStartAngle = bcg_normalize_deg(dblStartAngle);
		pScale->m_dblFinishAngle = -360.0 + pScale->m_dblStartAngle;
	}
	else
	{
		pScale->m_dblStartAngle = dblStartAngle;
		pScale->m_dblFinishAngle = dblFinishAngle;
	}

	SetDirty();
}
//*******************************************************************************
double CBCGPCircularGaugeImpl::GetTicksAreaStartAngle(int nScale) const
{
	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return 0.;
	}

	return pScale->m_dblStartAngle;
}
//*******************************************************************************
double CBCGPCircularGaugeImpl::GetTicksAreaFinishAngle(int nScale) const
{
	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return 0.;
	}

	return pScale->m_dblFinishAngle;
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::EnableLabelsRotation(BOOL bEnable, int nScale)
{
	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
	}

	pScale->m_bRotateLabels = bEnable;
}
//*******************************************************************************
BOOL CBCGPCircularGaugeImpl::IsLabelRotation(int nScale) const
{
	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return pScale->m_bRotateLabels;
}
//*******************************************************************************
BOOL CBCGPCircularGaugeImpl::IsClosed(int nScale) const
{
	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return pScale->m_bIsClosed;
}
//*******************************************************************************
BOOL CBCGPCircularGaugeImpl::IsDrawLastTickMark(int nScale) const
{
	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return pScale->m_bDrawLastTickMark;
}
//*******************************************************************************
BOOL CBCGPCircularGaugeImpl::IsAnimationThroughStart(int nScale) const
{
	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return pScale->m_bAnimationThroughStart;
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::OnDraw(CBCGPGraphicsManager* pGMSrc, const CBCGPRect& /*rectClip*/, DWORD dwFlags/* = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGMSrc);

	if (m_rect.IsRectEmpty() || !m_bIsVisible)
	{
		return;
	}

	BOOL bCacheImage = m_bCacheImage;

	if (pGMSrc->IsOffscreen())
	{
		bCacheImage = FALSE;
	}

	if (ReposAllSubGauges(pGMSrc) && bCacheImage)
	{
		SetDirty();
	}

	if (m_Colors.m_brFrameFillInv.IsEmpty())
	{
		m_Colors.m_brFrameFillInv.SetColors(
			m_Colors.m_brFrameFill.GetGradientColor(), m_Colors.m_brFrameFill.GetColor(),
			m_Colors.m_brFrameFill.GetGradientType(), m_Colors.m_brFrameFill.GetOpacity());
	}

	CBCGPGraphicsManager* pGM = pGMSrc;
	CBCGPGraphicsManager* pGMMem = NULL;

	CBCGPRect rectSaved;

	if (bCacheImage)
	{	
		if (m_ImageCache.GetHandle() == NULL)
		{
			SetDirty();
		}

		if (m_ImageCache.GetHandle() != NULL && !IsDirty() && (dwFlags & BCGP_DRAW_STATIC))
		{
			pGMSrc->DrawImage(m_ImageCache, m_rect.TopLeft());
			dwFlags &= ~BCGP_DRAW_STATIC;

			if (dwFlags == 0)
			{
				return;
			}
		}

		if (dwFlags & BCGP_DRAW_STATIC)
		{
			pGMMem = pGM->CreateOffScreenManager(m_rect, &m_ImageCache);

			if (pGMMem != NULL)
			{
				pGM = pGMMem;

				rectSaved = m_rect;
				m_rect = m_rect - m_rect.TopLeft();
			}
		}
	}

	if (IsDirty())
	{
		m_GeometryShape1.Destroy();
		m_GeometryShape2.Destroy();

		m_GeometryShape1.Clear();
		m_GeometryShape2.Clear();

		int i = 0;

		for (i = 0; i < m_arRanges.GetSize(); i++)
		{
			CBCGPGaugeColoredRangeObject* pRange = m_arRanges[i];
			ASSERT_VALID(pRange);

			pRange->m_Shape.Clear();
			pRange->m_Shape.Destroy();
		}

		for (i = 0; i < m_arScales.GetSize(); i++)
		{
			CBCGPCircularGaugeScale* pScale = 
				DYNAMIC_DOWNCAST(CBCGPCircularGaugeScale, m_arScales[i]);
			ASSERT_VALID(pScale);

			pScale->CleanUp();
		}

		SetDirty(FALSE);
	}

	CBCGPRect rect = m_rect;
	if (rect.Width() < rect.Height())
	{
		rect.top += (rect.Height() - rect.Width()) / 2;
		rect.bottom = rect.top + rect.Width();
	}
	else if (rect.Height() < rect.Width())
	{
		rect.left += (rect.Width() - rect.Height()) / 2;
		rect.right = rect.left + rect.Height();
	}

	const double scaleRatio = GetScaleRatioMid();
	const double nFrameSize = m_nFrameSize * scaleRatio;

	rect.DeflateRect(m_sizeScaleRatio.cx, m_sizeScaleRatio.cx);

	if (m_bShapeByTicksArea)
	{
		rect.DeflateRect(nFrameSize, nFrameSize);
	}

	double radius = rect.Height() / 2.0;

	if (dwFlags & BCGP_DRAW_STATIC)
	{
		const CBCGPPoint center(rect.CenterPoint());

		const CBCGPBrush& brFill = m_nFrameSize <= 2 ? m_Colors.m_brFill : m_Colors.m_brFrameFill;
		const BOOL bIsBevelFrame = m_nFrameSize > 2 && !m_bShapeByTicksArea && pGM->IsSupported(BCGP_GRAPHICS_MANAGER_ANTIALIAS);

		CBCGPRect rectFill = rect;
		
		if (m_bIsSubGauge)
		{
			rectFill.OffsetRect(-m_ptScrollOffset);
		}

		if (m_bDrawTicksOutsideFrame)
		{
			double dblExtraWidth = GetScale(0)->m_dblMajorTickMarkSize;

			for (int i = 0; i < m_arRanges.GetSize(); i++)
			{
				CBCGPGaugeColoredRangeObject* pRange = m_arRanges[i];
				ASSERT_VALID(pRange);

				dblExtraWidth = max(dblExtraWidth, max(pRange->m_dblStartWidth, pRange->m_dblFinishWidth));
			}

			dblExtraWidth *= scaleRatio;
			m_ScaleSize = GetMaxLabelWidth(pGM, 0) + dblExtraWidth;

			rectFill.DeflateRect(m_ScaleSize + 10 * m_sizeScaleRatio.cx, m_ScaleSize + 10 * m_sizeScaleRatio.cy);
		}

		if (m_bShapeByTicksArea)
		{
			CreateFrameShape(center, radius, m_GeometryShape1, FALSE);

			pGM->FillGeometry(m_GeometryShape1, brFill);
			pGM->DrawGeometry(m_GeometryShape1, m_Colors.m_brFrameOutline, scaleRatio);

			if (m_nFrameSize > 2)
			{
				CreateFrameShape(center, radius, m_GeometryShape2, TRUE);
			}
		}
		else
		{
			pGM->FillEllipse(rectFill, brFill);
			pGM->DrawEllipse(rectFill, m_Colors.m_brFrameOutline, scaleRatio);

			if (scaleRatio == 1.0 && !pGM->IsSupported(BCGP_GRAPHICS_MANAGER_ANTIALIAS))
			{
				CBCGPRect rect1 = rectFill;
				rect1.DeflateRect(1, 1);

				pGM->DrawEllipse(rect1, m_Colors.m_brFrameOutline);
			}
		}

		if (m_nFrameSize > 2)
		{
			CBCGPRect rectInternal = rectFill;
			rectInternal.DeflateRect(nFrameSize, nFrameSize);

			if (!m_bShapeByTicksArea)
			{
				if (bIsBevelFrame)
				{
					pGM->FillEllipse(rectInternal, m_Colors.m_brFrameFillInv);

					rectInternal.DeflateRect(nFrameSize, nFrameSize);
					pGM->DrawEllipse(rectInternal, m_Colors.m_brFrameOutline);
				}

				pGM->FillEllipse(rectInternal, m_Colors.m_brFill);

				if (scaleRatio == 1.0 && !pGM->IsSupported(BCGP_GRAPHICS_MANAGER_ANTIALIAS))
				{
					pGM->DrawEllipse(rectInternal, m_Colors.m_brFill);
					rectInternal.DeflateRect(1, 1);
					pGM->DrawEllipse(rectInternal, m_Colors.m_brFrameOutline);
				}
			}
			else
			{
				pGM->FillGeometry(m_GeometryShape2, m_Colors.m_brFill);
				pGM->DrawGeometry(m_GeometryShape2, m_Colors.m_brFrameOutline, scaleRatio);
			}

			if (!m_bDrawTicksOutsideFrame)
			{
				radius -= nFrameSize;
			}
		}

		if (!m_bDrawTicksOutsideFrame)
		{
			radius -= nFrameSize;
		}

		int i = 0;

		// Draw colored ranges:
		for (i = 0; i < m_arRanges.GetSize(); i++)
		{
			CBCGPGaugeColoredRangeObject* pRange = m_arRanges[i];
			ASSERT_VALID(pRange);

			if (CreateRangeShape(center, radius, pRange->m_Shape, 
				pRange->m_dblStartValue, pRange->m_dblFinishValue, 
				pRange->m_dblStartWidth, pRange->m_dblFinishWidth,
				pRange->m_dblOffsetFromFrame, pRange->m_nScale))
			{
				pGM->FillGeometry(pRange->m_Shape, pRange->m_brFill);
				pGM->DrawGeometry(pRange->m_Shape, pRange->m_brOutline, scaleRatio);
			}
		}

		// Draw scales:
		for (i = 0; i < m_arScales.GetSize(); i++)
		{
			OnDrawScale(pGM, i, center, radius);
		}

		// Draw static part of sub-gauges:
		if (!bCacheImage || pGMMem != NULL)
		{
			for (i = 0; i < m_arSubGauges.GetSize(); i++)
			{
				CBCGPGaugeImpl* pGauge = m_arSubGauges[i];
				ASSERT_VALID(pGauge);

				if (pGauge->IsVisible() && !pGauge->GetRect().IsRectEmpty())
				{
					CBCGPRect rectSavedSubGauge;

					if (pGMMem != NULL)
					{
						rectSavedSubGauge = pGauge->GetRect();
						pGauge->m_rect.OffsetRect(-rectSaved.left, -rectSaved.top);
					}

					pGauge->OnDraw(pGM, pGauge->GetRect(), BCGP_DRAW_STATIC);

					if (pGMMem != NULL)
					{
						pGauge->m_rect = rectSavedSubGauge;
					}
				}
			}
		}
	}
	else
	{
		radius -= (nFrameSize + (GetScale(0)->m_dblMajorTickMarkSize + GetScale(0)->m_dblOffsetFromFrame) * scaleRatio);
	}

	if (pGMMem != NULL)
	{
		delete pGMMem;
		pGM = pGMSrc;

		m_rect = rectSaved;

		pGMSrc->DrawImage(m_ImageCache, m_rect.TopLeft());
	}

	if (dwFlags & BCGP_DRAW_DYNAMIC)
	{
		int i = 0;

		CBCGPRect rectSaved = m_rect;

		if (m_bIsSubGauge)
		{
			m_rect.OffsetRect(-m_ptScrollOffset);
		}

		for (i = 0; i < m_arSubGauges.GetSize(); i++)
		{
			CBCGPGaugeImpl* pGauge = m_arSubGauges[i];
			ASSERT_VALID(pGauge);

			if (pGauge->IsVisible() && !pGauge->GetRect().IsRectEmpty())
			{
				CBCGPRect rectSubGauge = pGauge->GetRect();
				rectSubGauge.OffsetRect(-m_ptScrollOffset);

				pGauge->OnDraw(pGM, rectSubGauge, BCGP_DRAW_DYNAMIC);
			}
		}

		if (m_bDrawTicksOutsideFrame)
		{
			radius -= m_ScaleSize + nFrameSize - GetScale(0)->m_dblMajorTickMarkSize * scaleRatio;
		}

		for (i = 0; i < m_arData.GetSize(); i++)
		{
			CBCGPPointsArray pts;
			CBCGPPointsArray ptsShadow;

			CreatePointerPoints(radius, pts, i, FALSE);

			if (pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY))
			{
				CreatePointerPoints(radius, ptsShadow, i, TRUE);
			}

			OnDrawPointer(pGM, pts, ptsShadow, i);
		}

		m_rect = rectSaved;

		if (m_CapSize > 2.)
		{
			const double nCapSize = m_CapSize * scaleRatio;

			CBCGPRect rectCap = rect;

			if (m_bIsSubGauge)
			{
				rectCap.OffsetRect(-m_ptScrollOffset);
			}

			rectCap.left = rectCap.CenterPoint().x - nCapSize;
			rectCap.right = rectCap.left + 2 * nCapSize;

			rectCap.top = rectCap.CenterPoint().y - nCapSize;
			rectCap.bottom = rectCap.top + 2 * nCapSize;

			OnDrawPointerCap(pGM, rectCap);
		}
	}
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::OnDrawScale(CBCGPGraphicsManager* pGM, int nScale, const CBCGPPoint& center, double radius)
{
	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	const double scaleRatio = GetScaleRatioMid();

	const CBCGPBrush& brFill = pScale->m_brFill.IsEmpty() ? m_Colors.m_brScaleFill : pScale->m_brFill;
	const CBCGPBrush& brOutline = pScale->m_brOutline.IsEmpty() ? m_Colors.m_brScaleOutline : pScale->m_brOutline;

	if (!brFill.IsEmpty() || !brOutline.IsEmpty())
	{
		if (CreateRangeShape(center, radius, pScale->m_Shape, 
			pScale->m_dblStart, pScale->m_dblFinish, 
			pScale->m_dblMajorTickMarkSize, pScale->m_dblMajorTickMarkSize,
			pScale->m_dblOffsetFromFrame, nScale))
		{
			if (!brFill.IsEmpty())
			{
				pGM->FillGeometry(pScale->m_Shape, brFill);
			}

			if (!brOutline.IsEmpty())
			{
				pGM->DrawGeometry(pScale->m_Shape, brOutline, scaleRatio);
			}
		}
	}

	radius -= (pScale->m_dblOffsetFromFrame + 1) * scaleRatio;

	m_nCurrLabelIndex = 0;

	BOOL bRecalcPositions = pScale->m_arPointsTickStart.GetSize() == 0;

	double angleCos = 0.;
	double angleSin = 0.;

	int i = 0;

	double dblStep = (pScale->m_dblFinish > pScale->m_dblStart) ? pScale->m_dblStep : -pScale->m_dblStep;
	double dblAngle = bcg_deg2rad(pScale->m_dblStartAngle);
	double dblAngleStep = bcg_deg2rad(pScale->m_dblStartAngle - pScale->m_dblFinishAngle) / ((pScale->m_dblFinish - pScale->m_dblStart) / dblStep);

	double radiusTicks = radius;

	if (m_bDrawTextBeforeTicks)
	{
		radiusTicks -= GetMaxLabelWidth(pGM, nScale) + (m_nFrameSize + 1) * scaleRatio;
	}

	double dblMinorTickSize = pScale->m_dblMinorTickMarkSize * scaleRatio;
	double dblMajorTickSize = pScale->m_dblMajorTickMarkSize * scaleRatio;
	CBCGPGaugeScaleObject::BCGP_TICKMARK_POSITION position = pScale->m_MinorTickMarkPosition;

	CBCGPTextFormat tf(m_textFormat);
	if (pScale->m_bRotateLabels || tf.GetDrawingAngle() != 0.0)
	{
		tf.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
		tf.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
	}

	for (double dblVal = pScale->m_dblStart; 
		(dblStep > 0. && dblVal <= pScale->m_dblFinish) || (dblStep < 0. && dblVal >= pScale->m_dblFinish); 
		dblVal += dblStep, i++)
	{
		const BOOL bIsLastTick = (pScale->m_dblFinish > pScale->m_dblStart && dblVal + dblStep > pScale->m_dblFinish);

		BOOL bIsMajorTick = pScale->m_dblMajorTickMarkStep != 0. && ((i % bcg_round(pScale->m_dblMajorTickMarkStep)) == 0);

		if (!bIsMajorTick && (i == 0 || bIsLastTick))
		{
			// Always draw first and last ticks
			bIsMajorTick = TRUE;
		}

		CBCGPPoint ptFrom;
		CBCGPPoint ptTo;

		double dblCurrTickSize = bIsMajorTick ? dblMajorTickSize : dblMinorTickSize;

		if (bRecalcPositions)
		{
			double r = radiusTicks;

			if (!bIsMajorTick)
			{
				double dblDeltaTick = dblMajorTickSize - dblCurrTickSize;

				if (m_bDrawTicksOutsideFrame)
				{
					r -= dblDeltaTick;
				}

				if (position != CBCGPGaugeScaleObject::BCGP_TICKMARK_POSITION_NEAR && 
					dblCurrTickSize < dblMajorTickSize)
				{
					if (m_bDrawTicksOutsideFrame)
					{
						dblDeltaTick = -dblDeltaTick;
					}

					if (position == CBCGPGaugeScaleObject::BCGP_TICKMARK_POSITION_CENTER)
					{
						dblDeltaTick /= 2.0;
					}

					r -= dblDeltaTick;
				}
			}

			angleCos = cos(dblAngle);
			angleSin = sin(dblAngle);

			ptFrom = CBCGPPoint(center.x + angleCos * r, center.y - angleSin * r);
			ptTo = CBCGPPoint(center.x + angleCos * (r - dblCurrTickSize), center.y - angleSin * (r - dblCurrTickSize));

			pScale->m_arPointsTickStart.Add(ptFrom);
			pScale->m_arPointsTickFinish.Add(ptTo);
		}
		else
		{
			ptFrom = pScale->m_arPointsTickStart[i];
			ptTo = pScale->m_arPointsTickFinish[i];
		}

		BOOL bSkipTickMark = FALSE;

		if (pScale->m_bIsClosed)
		{
			if ((i == 0 && pScale->m_bDrawLastTickMark) || ((bIsLastTick && !pScale->m_bDrawLastTickMark)))
			{
				bSkipTickMark = TRUE;
			}
		}

		if (dblCurrTickSize > 0. && !bSkipTickMark)
		{
			OnDrawTickMark(pGM, ptFrom, ptTo, 
				bIsMajorTick ? pScale->m_MajorTickMarkStyle : pScale->m_MinorTickMarkStyle,
				bIsMajorTick, dblVal, dblAngle, nScale,
				bIsMajorTick ? pScale->m_brTickMarkMajor : pScale->m_brTickMarkMinor,
				bIsMajorTick ? pScale->m_brTickMarkMajorOutline : pScale->m_brTickMarkMinorOutline);
		}

		if (bIsMajorTick)
		{
			CString strLabel;
			GetTickMarkLabel(strLabel, pScale->m_strLabelFormat, dblVal, nScale);

			if (!strLabel.IsEmpty())
			{
				CBCGPRect rectText;

				if (pScale->m_bRotateLabels)
				{
					tf.SetDrawingAngle(bcg_rad2deg(dblAngle) - 90.0);
				}

				if (bRecalcPositions)
				{
					CBCGPPoint ptText = ptTo;

					if (m_bDrawTextBeforeTicks)
					{
						ptText = CBCGPPoint(center.x + angleCos * radius, center.y - angleSin * radius);
					}

					double xLabel = ptText.x - angleCos * 5.0 * scaleRatio;
					double yLabel = ptText.y + angleSin * 10.0 * scaleRatio;

					CBCGPSize sizeText = GetTickMarkTextLabelSize(pGM, strLabel, tf);

					if (pScale->m_bRotateLabels)
					{
						double dx = .5 * sizeText.cx * angleCos;
						double dy = .5 * sizeText.cy * angleSin;

						rectText = CBCGPRect(xLabel - dx, yLabel - dy, 
											xLabel + dx, yLabel + dy);
					}
					else
					{
						if (fabs(ptTo.x - center.x) < sizeText.cx)
						{
							rectText.left = xLabel - sizeText.cx / 2;
							rectText.right = rectText.left + sizeText.cx;
						}
						else if (ptTo.x < center.x)
						{
							rectText.left = xLabel;
							rectText.right = rectText.left + sizeText.cx;
						}
						else
						{
							rectText.right = xLabel;
							rectText.left = rectText.right - sizeText.cx;
						}

						rectText.top = yLabel - sizeText.cy / 2;
						rectText.bottom = rectText.top + sizeText.cy;
					}

					pScale->m_arRectTickLabel.Add(rectText);
				}
				else
				{
					rectText = pScale->m_arRectTickLabel[m_nCurrLabelIndex];
				}

				if (!bSkipTickMark)
				{
					OnDrawTickMarkTextLabel(pGM, tf, rectText, strLabel, dblVal, nScale, 
						pScale->m_brText.IsEmpty() ? m_Colors.m_brText : pScale->m_brText);
				}
			}
			else if (bRecalcPositions)
			{
				pScale->m_arRectTickLabel.Add(CBCGPRect());
			}

			m_nCurrLabelIndex++;
		}

		dblAngle -= dblAngleStep;
	}
}
//**********************************************************************************************************
void CBCGPCircularGaugeImpl::OnDrawTickMark(CBCGPGraphicsManager* pGM, const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo, 
											CBCGPCircularGaugeScale::BCGP_TICKMARK_STYLE style, BOOL bMajor, 
											double dblVal, double dblAngle, int nScale, 
											const CBCGPBrush& brFillIn, const CBCGPBrush& brOutlineIn)
{
	ASSERT_VALID(pGM);

	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	const double scaleRatio = GetScaleRatioMid();
	const double dblSize = pScale->m_dblMajorTickMarkSize * scaleRatio;

	const CBCGPGaugeColoredRangeObject* pRange = GetColoredRangeByValue(dblVal, nScale, BCGP_GAUGE_RANGE_TICKMARK_COLOR);

	const CBCGPBrush& brFill = (pRange != NULL && !pRange->m_brTickMarkFill.IsEmpty()) ?
		pRange->m_brTickMarkFill : (brFillIn.IsEmpty() ? m_Colors.m_brTickMarkFill : brFillIn);

	const CBCGPBrush& brOutline = (pRange != NULL && !pRange->m_brTickMarkOutline.IsEmpty()) ?
		pRange->m_brTickMarkOutline : (brOutlineIn.IsEmpty() ? m_Colors.m_brTickMarkOutline : brOutlineIn);

	switch (style)
	{
	case CBCGPCircularGaugeScale::BCGP_TICKMARK_LINE:
		pGM->DrawLine(ptFrom, ptTo, brOutline, scaleRatio);
		break;

	case CBCGPCircularGaugeScale::BCGP_TICKMARK_TRIANGLE:
	case CBCGPCircularGaugeScale::BCGP_TICKMARK_TRIANGLE_INV:
	case CBCGPCircularGaugeScale::BCGP_TICKMARK_BOX:
		{
			CBCGPPointsArray arPoints;
			
			const double dblPointerAngle = dblAngle - M_PI_2;
			const double dblWidth = (style == CBCGPCircularGaugeScale::BCGP_TICKMARK_BOX) ?
				(dblSize / 4.) : (bMajor ? (dblSize * 2. / 3.) : (dblSize / 2.));

			const double dx = cos(dblPointerAngle) * dblWidth * .5;
			const double dy = -sin(dblPointerAngle) * dblWidth * .5;

			if (style != CBCGPCircularGaugeScale::BCGP_TICKMARK_TRIANGLE_INV)
			{
				if (style == CBCGPCircularGaugeScale::BCGP_TICKMARK_BOX)
				{
					arPoints.Add(CBCGPPoint(ptTo.x + dx, ptTo.y + dy));
					arPoints.Add(CBCGPPoint(ptTo.x - dx, ptTo.y - dy));
				}
				else
				{
					arPoints.Add(ptTo);
				}

				arPoints.Add(CBCGPPoint(ptFrom.x - dx, ptFrom.y - dy));
				arPoints.Add(CBCGPPoint(ptFrom.x + dx, ptFrom.y + dy));
			}
			else
			{
				arPoints.Add(CBCGPPoint(ptTo.x - dx, ptTo.y - dy));
				arPoints.Add(CBCGPPoint(ptTo.x + dx, ptTo.y + dy));
				arPoints.Add(ptFrom);
			}

			CBCGPPolygonGeometry rgn(arPoints);

			pGM->FillGeometry(rgn, brFill);
			pGM->DrawGeometry(rgn, brOutline, scaleRatio);
		}
		break;

	case CBCGPCircularGaugeScale::BCGP_TICKMARK_CIRCLE:
		{
			CBCGPPoint ptCenter(
				.5 * (ptFrom.x + ptTo.x),
				.5 * (ptFrom.y + ptTo.y));

			const double dblRadius = bMajor ? (dblSize / 2.) : (dblSize / 4.);

			CBCGPEllipse ellipse(ptCenter, dblRadius, dblRadius);

			pGM->FillEllipse(ellipse, brFill);
			pGM->DrawEllipse(ellipse, brOutline, scaleRatio);

			if (scaleRatio == 1.0 && !pGM->IsSupported(BCGP_GRAPHICS_MANAGER_ANTIALIAS))
			{
				CBCGPEllipse ellipse1(ptCenter, dblRadius - 1., dblRadius - 1.);
				pGM->DrawEllipse(ellipse1, brOutline);
			}
		}
		break;
	}
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::OnDrawPointer(CBCGPGraphicsManager* pGM, 
	const CBCGPPointsArray& arPoints, 
	const CBCGPPointsArray& arShadowPoints,
	int nIndex)
{
	ASSERT_VALID(pGM);

	const double scaleRatio = GetScaleRatioMid();
	const int nCount = (int)arPoints.GetSize ();

	CBCGPGaugeDataObject* pData = DYNAMIC_DOWNCAST(CBCGPGaugeDataObject, m_arData[nIndex]);
	ASSERT_VALID(pData);

	const CBCGPBrush& brFrame = pData->GetOutlineBrush().IsEmpty() ? m_Colors.m_brPointerOutline : pData->GetOutlineBrush();
	const CBCGPBrush& brFill = pData->GetFillBrush().IsEmpty() ? m_Colors.m_brPointerFill : pData->GetFillBrush();

	if (pData->IsCircleShape())
	{
		if (nCount < 2)
		{
			return;
		}

		if (pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY) &&
			arShadowPoints.GetSize() >= 2)
		{
			const CBCGPEllipse ellipseShadow(arShadowPoints[0], arShadowPoints[1].x, arShadowPoints[1].y);
			pGM->FillEllipse(ellipseShadow, m_brShadow);
		}

		const CBCGPEllipse ellipse(arPoints[0], arPoints[1].x, arPoints[1].y);
		
		pGM->FillEllipse(ellipse, brFill);
		pGM->DrawEllipse(ellipse, brFrame, scaleRatio);

		return;
	}
	
	if (pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY) &&
		arShadowPoints.GetSize() > 0)
	{
		pGM->FillGeometry(CBCGPPolygonGeometry(arShadowPoints), m_brShadow);
	}

	if (nCount > 2)
	{
		CBCGPPolygonGeometry rgn(arPoints);
		pGM->FillGeometry(rgn, brFill);
	}

	for (int i = 0; i < nCount - 1; i++)
	{
		pGM->DrawLine(arPoints[i], arPoints[i + 1], brFrame, scaleRatio);
	}

	if (nCount > 2)
	{
		pGM->DrawLine(arPoints[nCount - 1], arPoints[0], brFrame, scaleRatio);
	}
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::OnDrawPointerCap(CBCGPGraphicsManager* pGM, const CBCGPRect& rectCap)
{
	ASSERT_VALID(pGM);

	const double scaleRatio = GetScaleRatioMid();

	CBCGPEllipseGeometry rgn(rectCap);

	pGM->FillEllipse(rectCap, m_Colors.m_brCapFill);
	pGM->DrawEllipse(rectCap, m_Colors.m_brCapOutline, scaleRatio);

	if (scaleRatio == 1.0 && !pGM->IsSupported(BCGP_GRAPHICS_MANAGER_ANTIALIAS))
	{
		CBCGPRect rect = rectCap;
		rect.DeflateRect(1, 1);

		pGM->DrawEllipse(rect, m_Colors.m_brCapOutline);
	}
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::CreatePointerPoints(double dblRadius, 
	CBCGPPointsArray& arPoints,
	int nPointerIndex, BOOL bShadow)
{
	if (m_rect.IsRectEmpty())
	{
		return;
	}

	const double scaleRatio = GetScaleRatioMid();
	CBCGPRect rect = m_rect;

	CBCGPCircularGaugePointer* pData = DYNAMIC_DOWNCAST(CBCGPCircularGaugePointer, m_arData[nPointerIndex]);
	if (pData == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CBCGPCircularGaugeScale* pScale = GetScale(pData->GetScale());
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	double dblValue = pData->m_nAnimTimerID != 0 ? pData->m_dblAnimatedValue : pData->m_dblValue;

	double dblSize = bcg_clamp(pData->m_dblSize, 0.0, 1.0) * scaleRatio;
	if (dblSize == 0.0)
	{
		dblSize = dblRadius - (pScale->m_dblMinorTickMarkSize + pScale->m_dblOffsetFromFrame) * scaleRatio;
	}
	else
	{
		dblSize = dblRadius * bcg_clamp(dblSize, 0.5, 1.0);
	}

	double dblExtended = 0.0;
	if (pData->m_bExtraLen)
	{
		dblExtended = bcg_clamp(m_CapSize > 0.0 ? m_CapSize * 1.5 * scaleRatio : dblSize / 4.0, 0.0, dblSize / 2.0);
	}

	double dblAngle = bcg_deg2rad(pScale->m_dblStartAngle) - bcg_deg2rad(pScale->m_dblStartAngle - pScale->m_dblFinishAngle) * (dblValue - pScale->m_dblStart) / (pScale->m_dblFinish - pScale->m_dblStart);
	dblAngle = bcg_normalize_rad (dblAngle);

	if (bShadow && dblAngle != M_PI_2 && dblAngle != M_PI_2 * 3.0)
	{
		if (dblAngle < M_PI_2 || M_PI_2 * 3 < dblAngle)
		{
			dblAngle -= .05;
		}
		else
		{
			dblAngle += .05;
		}
	}

	CBCGPPoint center((rect.left + rect.right) / 2.0, (rect.top + rect.bottom) / 2.0);

	const double angleCos  = cos(dblAngle);
	const double angleSin  = sin(dblAngle);

	if (dblExtended > 0.0)
	{
		dblSize += dblExtended;
		center.x -= angleCos * dblExtended;
		center.y += angleSin * dblExtended;
	}

	double dblWidth = bcg_clamp(pData->m_dblWidth * scaleRatio, 0.0, dblRadius / 10.0);
	const double dblPointerAngle = dblAngle - M_PI_2;

	switch (pData->GetStyle())
	{
	case CBCGPCircularGaugePointer::BCGP_GAUGE_NEEDLE_TRIANGLE:
		{
			if (dblWidth == 0.0)
			{
				dblWidth = 9.0 * scaleRatio;
			}

			dblWidth *= 0.5;

			const double dx = cos(dblPointerAngle) * dblWidth;
			const double dy = -sin(dblPointerAngle) * dblWidth;

			arPoints.Add(CBCGPPoint(center.x + dx, center.y + dy));

			arPoints.Add(CBCGPPoint(center.x - dx, center.y - dy));

			arPoints.Add(CBCGPPoint(
				center.x + angleCos * dblSize, 
				center.y - angleSin * dblSize));
		}
		break;

	case CBCGPCircularGaugePointer::BCGP_GAUGE_NEEDLE_RECT:
		{
			if (dblWidth == 0.0)
			{
				dblWidth = 5.0 * scaleRatio;
			}

			dblWidth *= 0.5;

			if (dblWidth < 1.0)
			{
				arPoints.Add(center);

				arPoints.Add(CBCGPPoint(
					center.x + angleCos * dblSize, 
					center.y - angleSin * dblSize));
			}
			else
			{
				const double dx = cos(dblPointerAngle) * dblWidth;
				const double dy = -sin(dblPointerAngle) * dblWidth;

				arPoints.Add(CBCGPPoint(center.x + dx, center.y + dy));

				arPoints.Add(CBCGPPoint(center.x - dx, center.y - dy));

				arPoints.Add(CBCGPPoint(
					center.x + angleCos * dblSize - dx, 
					center.y - angleSin * dblSize - dy));

				arPoints.Add(CBCGPPoint(
					center.x + angleCos * dblSize + dx, 
					center.y - angleSin * dblSize + dy));
			}
		}
		break;

	case CBCGPCircularGaugePointer::BCGP_GAUGE_NEEDLE_ARROW:
	case CBCGPCircularGaugePointer::BCGP_GAUGE_NEEDLE_ARROW_BASE:
		{
			if (dblWidth == 0.0)
			{
				dblWidth = 5.0 * scaleRatio;
			}

			dblWidth *= 0.5;

			const BOOL bIsArrowBase = pData->GetStyle() == CBCGPCircularGaugePointer::BCGP_GAUGE_NEEDLE_ARROW_BASE;

			if (dblWidth < scaleRatio)
			{
				arPoints.Add(center);

				arPoints.Add(CBCGPPoint(
					center.x + angleCos * dblSize, 
					center.y - angleSin * dblSize));
			}
			else
			{
				double dblArrowLen = max(2.0 * dblWidth, 10.0 * scaleRatio);
				dblSize -= dblArrowLen;

				const double dx = cos(dblPointerAngle) * dblWidth;
				const double dy = -sin(dblPointerAngle) * dblWidth;

				arPoints.Add(CBCGPPoint(center.x + dx, center.y + dy));

				arPoints.Add(CBCGPPoint(center.x - dx, center.y - dy));

				const CBCGPPoint pt1(
					center.x + angleCos * dblSize - dx,
					center.y - angleSin * dblSize - dy);

				const CBCGPPoint pt2(
					center.x + angleCos * dblSize + dx, 
					center.y - angleSin * dblSize + dy);

				arPoints.Add(pt1);

				if (bIsArrowBase)
				{
					arPoints.Add(CBCGPPoint(
						center.x + angleCos * dblSize - dx * 2, 
						center.y - angleSin * dblSize - dy * 2));
				}

				arPoints.Add(CBCGPPoint(
					center.x + angleCos * (dblSize + dblArrowLen), 
					center.y - angleSin * (dblSize + dblArrowLen)));

				if (bIsArrowBase)
				{
					arPoints.Add(CBCGPPoint(
						center.x + angleCos * dblSize + dx * 2, 
						center.y - angleSin * dblSize + dy * 2));
				}

				arPoints.Add(pt2);
			}
		}
		break;

	case CBCGPCircularGaugePointer::BCGP_GAUGE_MARKER_TRIANGLE:
	case CBCGPCircularGaugePointer::BCGP_GAUGE_MARKER_RECT:
		{
			if (dblWidth == 0.0)
			{
				dblWidth = 5.0 * scaleRatio;
			}

			const double dx = cos(dblPointerAngle) * dblWidth;
			const double dy = -sin(dblPointerAngle) * dblWidth;

			dblSize -= dblWidth;

			if (pData->GetStyle() == CBCGPCircularGaugePointer::BCGP_GAUGE_MARKER_TRIANGLE)
			{
				arPoints.Add(CBCGPPoint(center.x + angleCos * dblSize, center.y - angleSin * dblSize));
			}
			else
			{
				arPoints.Add(CBCGPPoint(center.x + angleCos * dblSize - dx, center.y - angleSin * dblSize - dy));
				arPoints.Add(CBCGPPoint(center.x + angleCos * dblSize + dx, center.y - angleSin * dblSize + dy));
			}

			dblSize += 2. * dblWidth;

			arPoints.Add(CBCGPPoint(center.x + angleCos * dblSize + dx, center.y - angleSin * dblSize + dy));
			arPoints.Add(CBCGPPoint(center.x + angleCos * dblSize - dx, center.y - angleSin * dblSize - dy));
		}
		break;

	case CBCGPCircularGaugePointer::BCGP_GAUGE_MARKER_CIRCLE:
		{
			if (dblWidth == 0.0)
			{
				dblWidth = 5.0 * scaleRatio;
			}

			dblSize -= 2.0 * scaleRatio;

			arPoints.Add(CBCGPPoint(center.x + angleCos * dblSize, center.y - angleSin * dblSize));
			arPoints.Add(CBCGPPoint(dblWidth, dblWidth));
		}
		break;

	case CBCGPCircularGaugePointer::BCGP_GAUGE_MARKER_DIAMOND:
		{
			if (dblWidth == 0.0)
			{
				dblWidth = 5.0 * scaleRatio;
			}

			const double dx = cos(dblPointerAngle) * dblWidth;
			const double dy = -sin(dblPointerAngle) * dblWidth;

			dblSize -= dblWidth;

			arPoints.Add(CBCGPPoint(center.x + angleCos * dblSize, center.y - angleSin * dblSize));

			dblSize += dblWidth;

			arPoints.Add(CBCGPPoint(center.x + angleCos * dblSize - dx, center.y - angleSin * dblSize - dy));

			dblSize += dblWidth;

			arPoints.Add(CBCGPPoint(center.x + angleCos * dblSize, center.y - angleSin * dblSize));

			dblSize -= dblWidth;

			arPoints.Add(CBCGPPoint(center.x + angleCos * dblSize + dx, center.y - angleSin * dblSize + dy));
		}
		break;

	}
}
//*******************************************************************************
int CBCGPCircularGaugeImpl::AddPointer(const CBCGPCircularGaugePointer& pointer, int nScale, BOOL bRedraw)
{
	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return -1;
	}

	CBCGPCircularGaugePointer* pData = new CBCGPCircularGaugePointer;
	pData->CopyFrom(pointer);

	pData->m_nScale = nScale;
	pData->m_dblValue = pScale->m_dblStart;

	int nIndex = (int)AddData(pData);

	if (bRedraw)
	{
		Redraw();
	}

	return nIndex;
}
//*******************************************************************************
BOOL CBCGPCircularGaugeImpl::RemovePointer(int nIndex, BOOL bRedraw)
{
	if (nIndex == 0)
	{
		TRACE0("CBCGPCircularGaugeImpl: Can't remove first (primary) pointer\n");
		ASSERT(FALSE);
		return FALSE;
	}

	if (nIndex < 0 || nIndex >= m_arData.GetSize())
	{
		return FALSE;
	}

	StopAnimation(m_arData[nIndex]->GetAnimationID(), FALSE);

	delete m_arData[nIndex];
	m_arData.RemoveAt(nIndex);

	if (bRedraw)
	{
		Redraw();
	}

	return TRUE;
}
//*******************************************************************************
BOOL CBCGPCircularGaugeImpl::ModifyPointer(int nIndex, const CBCGPCircularGaugePointer& pointer, BOOL bRedraw)
{
	if (nIndex < 0 || nIndex >= m_arData.GetSize())
	{
		return FALSE;
	}

	CBCGPCircularGaugePointer* pPointer =
		DYNAMIC_DOWNCAST(CBCGPCircularGaugePointer, m_arData[nIndex]);
	ASSERT_VALID(pPointer);

	pPointer->m_Style = pointer.m_Style;
	pPointer->m_dblSize = pointer.m_dblSize;
	pPointer->m_dblWidth = pointer.m_dblWidth;
	pPointer->m_bExtraLen = pointer.m_bExtraLen;

	if (!pointer.m_brFill.IsEmpty())
	{
		pPointer->m_brFill = pointer.m_brFill;
	}

	if (!pointer.m_brOutline.IsEmpty())
	{
		pPointer->m_brOutline = pointer.m_brOutline;
	}

	if (bRedraw)
	{
		Redraw();
	}

	return TRUE;
}
//*******************************************************************************
BOOL CBCGPCircularGaugeImpl::CreateRangeShape(const CBCGPPoint& center, double radius,
	CBCGPComplexGeometry& shape, double dblStartValue, double dblFinishValue,
	double dblStartWidth, double dblFinishWidth,
	double dblOffsetFromFrame, int nScale)
{
	if (!shape.IsNull())
	{
		return TRUE;
	}

	double dblStartAngle = 0.;
	if (!ValueToAngle(dblStartValue, dblStartAngle, nScale))
	{
		return FALSE;
	}

	double dblFinishAngle = 0.;
	if (!ValueToAngle(dblFinishValue, dblFinishAngle, nScale))
	{
		return FALSE;
	}

	const double scaleRatio = GetScaleRatioMid();
	const double nCapSize = m_CapSize * scaleRatio;

	double angleCos1 = cos(bcg_deg2rad(dblStartAngle));
	double angleSin1 = sin(bcg_deg2rad(dblStartAngle));

	double angleCos2 = cos(bcg_deg2rad(dblFinishAngle));
	double angleSin2 = sin(bcg_deg2rad(dblFinishAngle));

	if (dblStartWidth == 0.)
	{
		CBCGPCircularGaugeScale* pScale = GetScale(nScale);
		if (pScale != NULL)
		{
			dblStartWidth = pScale->m_dblMajorTickMarkSize + pScale->m_dblOffsetFromFrame;
		}
	}

	if (dblFinishWidth == 0.)
	{
		dblFinishWidth = dblStartWidth;
	}

	dblStartWidth *= scaleRatio;
	dblFinishWidth *= scaleRatio;

	if (m_bDrawTicksOutsideFrame)
	{
		radius -= m_ScaleSize;
	}

	double r1 = radius - (dblOffsetFromFrame + 1.) * scaleRatio;
	double r2 = max(.5 + nCapSize, r1 - dblStartWidth);
	double r3 = max(.5 + nCapSize, r1 - dblFinishWidth);

	BOOL bIsLargeArc = fabs(dblFinishAngle - dblStartAngle) >= 175.;

	shape.SetStart(
		CBCGPPoint(center.x + angleCos1 * r1, center.y - angleSin1 * r1));
	shape.AddArc(
		CBCGPPoint(center.x + angleCos2 * r1, center.y - angleSin2 * r1),
		CBCGPSize(r1, r1), dblStartAngle > dblFinishAngle, bIsLargeArc);
	shape.AddLine(
		CBCGPPoint(center.x + angleCos2 * r3, center.y - angleSin2 * r3));
	shape.AddArc(
		CBCGPPoint(center.x + angleCos1 * r2, center.y - angleSin1 * r2),
		CBCGPSize(.5 * (r2 + r3), .5 * (r2 + r3)), dblStartAngle < dblFinishAngle, bIsLargeArc);

	return TRUE;
}
//*******************************************************************************
BOOL CBCGPCircularGaugeImpl::CreateFrameShape(const CBCGPPoint& center, double radius,
	CBCGPComplexGeometry& shape, BOOL bInternal)
{
	if (!shape.IsNull())
	{
		return TRUE;
	}

	double dblStartAngleMax = (double)-INT_MAX;
	double dblFinishAngleMin = (double)INT_MAX;

	for (int i = 0; i < m_arScales.GetSize(); i++)
	{
		CBCGPCircularGaugeScale* pScale = DYNAMIC_DOWNCAST(CBCGPCircularGaugeScale, m_arScales[i]);
		ASSERT_VALID(pScale);

		dblStartAngleMax = max(dblStartAngleMax, pScale->m_dblStartAngle);
		dblFinishAngleMin = min(dblFinishAngleMin, pScale->m_dblFinishAngle);
	}

	const double scaleRatio = GetScaleRatioMid();
	const double nFrameSize = m_nFrameSize * scaleRatio;
	const double nCapSize = m_CapSize * scaleRatio + nFrameSize / 2.0;

	const double a1  = bcg_rad2deg(nCapSize / radius);

	const double dblStartAngleN = min(dblStartAngleMax, dblFinishAngleMin);
	const double dblFinishAngleN = max(dblStartAngleMax, dblFinishAngleMin);
	const double dblStartAngle = dblStartAngleN - a1;
	const double dblFinishAngle = dblFinishAngleN + a1;
	const double dblStartAngleRad = bcg_deg2rad(dblStartAngle);
	const double dblFinishAngleRad = bcg_deg2rad(dblFinishAngle);
	double angleStartCos = cos(dblStartAngleRad);
	double angleStartSin = sin(dblStartAngleRad);
	double angleFinishCos = cos(dblFinishAngleRad);
	double angleFinishSin = sin(dblFinishAngleRad);

	const double dblMiddleAngleRad = bcg_deg2rad(dblFinishAngleN + dblStartAngleN) / 2.0;
	const double coef = 1.0 / sin(bcg_deg2rad(dblFinishAngleN) - dblMiddleAngleRad);
	const double angleMiddleCos = cos(dblMiddleAngleRad) * coef;
	const double angleMiddleSin = sin(dblMiddleAngleRad) * coef;

	CBCGPPoint centerM(center.x - angleMiddleCos * nCapSize, center.y + angleMiddleSin * nCapSize);

	if (!bInternal)
	{
		const double a2 = nFrameSize / radius;

		radius += nFrameSize;

		centerM.x -= angleMiddleCos * nFrameSize;
		centerM.y += angleMiddleSin * nFrameSize;

		angleStartCos = cos(dblStartAngleRad - a2);
		angleStartSin = sin(dblStartAngleRad - a2);
		angleFinishCos = cos(dblFinishAngleRad + a2);
		angleFinishSin = sin(dblFinishAngleRad + a2);
	}

	BOOL bIsLargeArc = fabs(dblFinishAngle - dblStartAngle) >= 175.;

	shape.SetStart(
		CBCGPPoint(center.x + angleStartCos * radius, center.y - angleStartSin * radius));
	shape.AddArc(
		CBCGPPoint(center.x + angleFinishCos * radius, center.y - angleFinishSin * radius),
		CBCGPSize(radius, radius), dblStartAngle > dblFinishAngle, bIsLargeArc);

	if (!bIsLargeArc)
	{
		shape.AddLine(centerM);
	}

	return TRUE;
}
//*******************************************************************************
BOOL CBCGPCircularGaugeImpl::ValueToAngle(double dblValue, double& dblAngle, int nScale) const
{
	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	double dblStart = min(pScale->m_dblStart, pScale->m_dblFinish);
	double dblFinish = max(pScale->m_dblStart, pScale->m_dblFinish);

	if (dblValue < dblStart || dblValue > dblFinish)
	{
		return FALSE;
	}

	dblAngle = pScale->m_dblStartAngle + 
		(pScale->m_dblFinishAngle - pScale->m_dblStartAngle) / (pScale->m_dblFinish - pScale->m_dblStart) * (dblValue - pScale->m_dblStart);
	
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPCircularGaugeImpl::AngleToValue(double dblAngle, double& dblValue, int nScale) const
{
	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	double dblStartAngle = min(pScale->m_dblStartAngle, pScale->m_dblFinishAngle);
	double dblFinishAngle = max(pScale->m_dblStartAngle, pScale->m_dblFinishAngle);

	dblAngle = bcg_normalize_rad(dblAngle);
	dblStartAngle = bcg_normalize_rad(dblStartAngle);
	dblFinishAngle = bcg_normalize_rad(dblFinishAngle);

	if (dblAngle < dblStartAngle || dblAngle > dblFinishAngle)
	{
		return FALSE;
	}

	dblValue = pScale->m_dblStart + 
		(pScale->m_dblFinish - pScale->m_dblStart) / (pScale->m_dblFinishAngle - pScale->m_dblStartAngle) * (dblAngle - pScale->m_dblStartAngle);
	
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPCircularGaugeImpl::HitTestValue(const CBCGPPoint& pt, double& dblValue, int nScale, BOOL bInsideGauge) const
{
	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CBCGPRect rect = m_rect;
	rect.OffsetRect(-m_ptScrollOffset);

	if (rect.Width() < rect.Height())
	{
		rect.top += (rect.Height() - rect.Width()) / 2;
		rect.bottom = rect.top + rect.Width();
	}
	else if (rect.Height() < rect.Width())
	{
		rect.left += (rect.Width() - rect.Height()) / 2;
		rect.right = rect.left + rect.Height();
	}

	const double radius = rect.Width() / 2;
	const CBCGPPoint center = rect.CenterPoint();

	double dblDistanceToCenter = bcg_distance(pt, center);

	if (bInsideGauge && dblDistanceToCenter > radius)
	{
		return FALSE;
	}

	double dblRange = fabs(pScale->m_dblFinishAngle - pScale->m_dblStartAngle);
	if (dblRange == 0.0)
	{
		return FALSE;
	}

	const BOOL bIsClockwise = pScale->m_dblFinishAngle < pScale->m_dblStartAngle; // clockwise

	double dblAngle = bcg_normalize_deg(bcg_rad2deg(bcg_angle(center, pt, TRUE)));
	double dblAngle1 = bcg_normalize_deg(pScale->m_dblStartAngle);
	double dblAngle2 = bcg_normalize_deg(pScale->m_dblFinishAngle);

	BOOL bIsOutOfRange = FALSE;

	if (bIsClockwise)
	{
		bIsOutOfRange = (dblAngle < dblAngle2 && dblAngle > dblAngle1);
	}
	else
	{
		bIsOutOfRange = (dblAngle < dblAngle1 && dblAngle > dblAngle2);
	}

	if (bIsOutOfRange)
	{
		dblAngle = (fabs(dblAngle2 - dblAngle) > fabs(dblAngle1 - dblAngle)) ? dblAngle1 : dblAngle2;
	}
	
	dblAngle = bcg_normalize_deg(dblAngle - (bIsClockwise ? pScale->m_dblFinishAngle : pScale->m_dblStartAngle));
	dblValue = dblAngle * (pScale->m_dblFinish - pScale->m_dblStart) / dblRange;

	if (bIsClockwise)
	{
		dblValue = pScale->m_dblFinish - dblValue;
	}
	else
	{
		dblValue = pScale->m_dblStart + dblValue;
	}

	dblValue = bcg_clamp(dblValue, pScale->m_dblStart, pScale->m_dblFinish);
	return TRUE;
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::SetRange(double dblStart, double dblFinish, int nScale)
{
	CBCGPGaugeImpl::SetRange(dblStart, dblFinish, nScale);

	for (int i = 0; i < m_arRanges.GetSize(); i++)
	{
		CBCGPGaugeColoredRangeObject* pRange = m_arRanges[i];
		ASSERT_VALID(pRange);

		pRange->m_Shape.Clear();
		pRange->m_Shape.Destroy();
	}
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::SetClosedRange(double dblStart, double dblFinish, double dblStartAngle, 
											BOOL bDrawLastTickMark, BOOL bAnimationThroughStart, int nScale)
{
	if (nScale < 0 || nScale >= m_arScales.GetSize())
	{
		ASSERT(FALSE);
		return;
	}

	SetRange(dblStart, dblFinish, nScale);

	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	ASSERT_VALID(pScale);

	pScale->m_bIsClosed = TRUE;
	pScale->m_bDrawLastTickMark = bDrawLastTickMark;
	pScale->m_bAnimationThroughStart = bAnimationThroughStart;

	pScale->m_dblStartAngle = bcg_normalize_deg(dblStartAngle);
	pScale->m_dblFinishAngle = -360.0 + pScale->m_dblStartAngle;

	SetDirty();
}
//******************************************************************************
CBCGPCircularGaugeScale* CBCGPCircularGaugeImpl::GetScale(int nScale) const
{
	if (nScale < 0 || nScale >= m_arScales.GetSize())
	{
		return NULL;
	}

	CBCGPCircularGaugeScale* pScale = DYNAMIC_DOWNCAST(CBCGPCircularGaugeScale, m_arScales[nScale]);
	ASSERT_VALID(pScale);

	return pScale;
}
//*******************************************************************************
int CBCGPCircularGaugeImpl::AddScale()
{
	CBCGPCircularGaugeScale* pScale = new CBCGPCircularGaugeScale;
	pScale->SetParentGauge (this);

	m_arScales.Add(pScale);
	return (int)m_arScales.GetSize() - 1;
}
//*******************************************************************************
BOOL CBCGPCircularGaugeImpl::SetValue(double dblVal, int nIndex, UINT uiAnimationTime, BOOL bRedraw)
{
	m_bInSetValue = TRUE;

	BOOL bIsDirty = m_bIsDirty;

	if (uiAnimationTime != 0)
	{
		CBCGPGaugeDataObject* pData = DYNAMIC_DOWNCAST(CBCGPGaugeDataObject, m_arData[nIndex]);
		ASSERT_VALID(pData);

		CBCGPCircularGaugeScale* pScale = GetScale(pData->GetScale());
		if (pScale == NULL)
		{
			ASSERT(FALSE);
			return FALSE;
		}

		ASSERT_VALID(pScale);

		if (pScale->m_bIsClosed && pScale->m_bAnimationThroughStart)
		{
			const double dblCurrVal = GetValue(nIndex);

			if (dblCurrVal > dblVal)
			{
				if (pScale->m_dblStart < pScale->m_dblFinish)
				{
					pData->SetValue(pScale->m_dblStart - (pScale->m_dblFinish - dblCurrVal));
				}
				else
				{
					pData->SetValue(pScale->m_dblFinish - (pScale->m_dblStart - dblCurrVal));
				}
			}
		}
	}

	BOOL bRes = CBCGPGaugeImpl::SetValue(dblVal, nIndex, uiAnimationTime, FALSE);

	m_bIsDirty = bIsDirty;

	if (bRedraw)
	{
		Redraw();
	}

	m_bInSetValue = FALSE;
	return bRes;
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::SetDirty(BOOL bSet, BOOL bRedraw)
{
	if (m_bInSetValue)
	{
		return;
	}

	CBCGPGaugeImpl::SetDirty(bSet, FALSE);

	if (bSet)
	{
		m_ImageCache.Destroy();
	}

	for (int i = 0; i < m_arSubGauges.GetSize(); i++)
	{
		CBCGPGaugeImpl* pGauge = m_arSubGauges[i];
		ASSERT_VALID(pGauge);

		pGauge->SetDirty(bSet, FALSE);
	}

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::AddSubGauge(CBCGPGaugeImpl* pGauge, BCGP_SUB_GAUGE_POS pos, const CBCGPSize& size, const CBCGPPoint& ptOffset, BOOL bAutoDestroy)
{
	ASSERT_VALID(pGauge);

	pGauge->SetPos(pos);
	pGauge->SetOffset(ptOffset);
	pGauge->SetAutoDestroy(bAutoDestroy);
	pGauge->EnableImageCache(FALSE);
	pGauge->SetOwner(GetOwner());
	pGauge->m_pParentContainer = m_pParentContainer;

	m_arSubGauges.Add(pGauge);

	pGauge->SetRect(CBCGPRect(CBCGPPoint(-1., -1.), size), FALSE);
	pGauge->SetScaleRatio(m_sizeScaleRatio);

	pGauge->m_bIsSubGauge = TRUE;
}
//*******************************************************************************
BOOL CBCGPCircularGaugeImpl::RemoveSubGauge(CBCGPGaugeImpl* pGaugeIn)
{
	ASSERT_VALID(pGaugeIn);

	for (int i = 0; i < m_arSubGauges.GetSize(); i++)
	{
		CBCGPGaugeImpl* pGauge = m_arSubGauges[i];
		ASSERT_VALID(pGauge);

		if (pGauge == pGaugeIn)
		{
			if (pGauge->IsAutoDestroy())
			{
				delete pGauge;
			}

			m_arSubGauges.RemoveAt(i);
			return TRUE;
		}
	}

	return FALSE;
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::RemoveAllSubGauges()
{
	for (int i = 0; i < m_arSubGauges.GetSize(); i++)
	{
		CBCGPGaugeImpl* pGauge = m_arSubGauges[i];
		ASSERT_VALID(pGauge);

		if (pGauge->IsAutoDestroy())
		{
			delete pGauge;
		}
	}

	m_arSubGauges.RemoveAll();
}
//*******************************************************************************
CBCGPGaugeImpl* CBCGPCircularGaugeImpl::GetSubGaugeByID(UINT nID)
{
	for (int i = 0; i < m_arSubGauges.GetSize(); i++)
	{
		CBCGPGaugeImpl* pGauge = m_arSubGauges[i];
		ASSERT_VALID(pGauge);

		if (pGauge->GetID() == nID)
		{
			return pGauge;
		}
	}
	
	return NULL;
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::SetRect(const CBCGPRect& rect, BOOL bRedraw)
{
	CBCGPGaugeImpl::SetRect(rect, FALSE);

	if (m_rect.IsRectEmpty())
	{
		return;
	}

	for (int i = 0; i < m_arSubGauges.GetSize(); i++)
	{
		CBCGPGaugeImpl* pGauge = m_arSubGauges[i];
		ASSERT_VALID(pGauge);

		const CBCGPSize size = pGauge->GetRect().Size();
		pGauge->SetRect(CBCGPRect(CBCGPPoint(-1., -1.), size), FALSE);
	}

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::ReposSubGauge(CBCGPGraphicsManager* pGM, CBCGPGaugeImpl* pGauge, const CBCGPSize& sizeIn)
{
	ASSERT_VALID(pGM);
	ASSERT_VALID(pGauge);

	CBCGPSize size = sizeIn;
	if (size.IsEmpty())
	{
		size = pGauge->GetDefaultSize(pGM, this);
		if (size.IsEmpty())
		{
			return;
		}
	}

	CBCGPRect rect = m_rect;
	if (rect.Width() < rect.Height())
	{
		rect.top += (rect.Height() - rect.Width()) / 2;
		rect.bottom = rect.top + rect.Width();
	}
	else if (rect.Height() < rect.Width())
	{
		rect.left += (rect.Width() - rect.Height()) / 2;
		rect.right = rect.left + rect.Height();
	}

	CBCGPPoint pt;

	if (pGauge->GetPos() == BCGP_SUB_GAUGE_NONE)
	{
		pt = rect.TopLeft();
	}
	else
	{
		const double scaleRatio = GetScaleRatioMid();
		const double nFrameSize = m_nFrameSize * scaleRatio;
		const double nCapSize = m_CapSize * scaleRatio;

		pt = rect.CenterPoint();
		rect.DeflateRect (nFrameSize, nFrameSize);

		switch (pGauge->GetPos())
		{
		case BCGP_SUB_GAUGE_TOP:
			pt.y = rect.top + (pt.y - rect.top - nCapSize / 2.) / 2.;
			break;

		case BCGP_SUB_GAUGE_BOTTOM:
			pt.y = rect.bottom - (rect.bottom - pt.y - nCapSize / 2.) / 2.;
			break;

		case BCGP_SUB_GAUGE_LEFT:
			pt.x = rect.left + (pt.x - rect.left - nCapSize / 2.) / 2.;
			break;

		case BCGP_SUB_GAUGE_RIGHT:
			pt.x = rect.right - (rect.right - pt.x - nCapSize / 2.) / 2.;
			break;
		}

		pt.x -= size.cx / 2.;
		pt.y -= size.cy / 2.;
	}

	pGauge->SetRect(CBCGPRect(pt + pGauge->GetOffset(), size), FALSE);
}
//*******************************************************************************
BOOL CBCGPCircularGaugeImpl::ReposAllSubGauges(CBCGPGraphicsManager* pGM)
{
	BOOL bIsDirty = FALSE;

	for (int i = 0; i < m_arSubGauges.GetSize(); i++)
	{
		CBCGPGaugeImpl* pGauge = m_arSubGauges[i];
		ASSERT_VALID(pGauge);

		if (pGauge->IsDirty())
		{
			bIsDirty = TRUE;
		}

		if (pGauge->GetRect().TopLeft() == CBCGPPoint(-1., -1.))
		{
			ReposSubGauge(pGM, pGauge, pGauge->GetRect().Size());

			if (pGauge->ReposAllSubGauges(pGM))
			{
				bIsDirty = TRUE;
			}
		}
	}

	return bIsDirty;
}
//*******************************************************************************
CWnd* CBCGPCircularGaugeImpl::SetOwner(CWnd* pWndOwner, BOOL bRedraw)
{
	CWnd* pWndOld = CBCGPGaugeImpl::SetOwner(pWndOwner, bRedraw);

	for (int i = 0; i < m_arSubGauges.GetSize(); i++)
	{
		CBCGPGaugeImpl* pGauge = m_arSubGauges[i];
		ASSERT_VALID(pGauge);

		pGauge->SetOwner(pWndOwner);
	}

	return pWndOld;
}
//*******************************************************************************
CBCGPSize CBCGPCircularGaugeImpl::GetDefaultSize(CBCGPGraphicsManager* /*pGM*/, const CBCGPBaseVisualObject* /*pParentGauge*/)
{
	CBCGPSize size = GetRect().Size();
	double dblRadius = min(size.cx, size.cy) / 3.5;

	return CBCGPSize(dblRadius, dblRadius);
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::SetTextFormat(const CBCGPTextFormat& textFormat)
{
	m_textFormat.Destroy();
	m_textFormat.CopyFrom(textFormat);
	SetDirty();
}
//*******************************************************************************
double CBCGPCircularGaugeImpl::GetMaxLabelWidth(CBCGPGraphicsManager* pGM, int nScale)
{
	ASSERT_VALID(pGM);

	double dblMaxTextLen = 0.;

	int i = 0;

	CBCGPCircularGaugeScale* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		return 0.;
	}

	if (pScale->m_dblMajorTickMarkStep == 0.)
	{
		return 0.;
	}

	double dblStep = (pScale->m_dblFinish > pScale->m_dblStart) ? pScale->m_dblStep : -pScale->m_dblStep;

	for (double dblVal = pScale->m_dblStart; 
		(dblStep > 0. && dblVal <= pScale->m_dblFinish) || (dblStep < 0. && dblVal >= pScale->m_dblFinish); 
		dblVal += dblStep, i++)
	{
		const BOOL bIsLastTick = (pScale->m_dblFinish > pScale->m_dblStart && dblVal + dblStep > pScale->m_dblFinish);

		BOOL bIsMajorTick = ((i % bcg_round(pScale->m_dblMajorTickMarkStep)) == 0);

		if (!bIsMajorTick && (i == 0 || bIsLastTick))
		{
			// Always draw first and last ticks
			bIsMajorTick = TRUE;
		}

		if (bIsMajorTick)
		{
			CString strLabel;
			GetTickMarkLabel(strLabel, pScale->m_strLabelFormat, dblVal, nScale);

			CBCGPSize sizeLabel = GetTickMarkTextLabelSize(pGM, strLabel, m_textFormat);

			dblMaxTextLen = max(max(dblMaxTextLen, sizeLabel.cx), sizeLabel.cy);
		}
	}

	return dblMaxTextLen;
}
//*******************************************************************************
BOOL CBCGPCircularGaugeImpl::OnSetMouseCursor(const CBCGPPoint& pt)
{
	if (m_bIsInteractiveMode)
	{
		CBCGPRect rect = m_rect;
		rect.OffsetRect(-m_ptScrollOffset);

		if (rect.Width() < rect.Height())
		{
			rect.top += (rect.Height() - rect.Width()) / 2;
			rect.bottom = rect.top + rect.Width();
		}
		else if (rect.Height() < rect.Width())
		{
			rect.left += (rect.Width() - rect.Height()) / 2;
			rect.right = rect.left + rect.Height();
		}

		rect.DeflateRect(1., 1.);

		if (m_bShapeByTicksArea)
		{
			const double nFrameSize = m_nFrameSize * GetScaleRatioMid();
			rect.DeflateRect(nFrameSize, nFrameSize);
		}

		const double radius = rect.Height() / 2.0;
		const CBCGPPoint center = rect.CenterPoint();

		const double dblDistanceToCenter = bcg_distance(pt, center);

		if (dblDistanceToCenter <= radius)
		{
			::SetCursor (globalData.GetHandCursor());
			return TRUE;
		}
	}

	return CBCGPGaugeImpl::OnSetMouseCursor(pt);
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::SetScrollOffset(const CBCGPPoint& ptScrollOffset)
{
	CBCGPGaugeImpl::SetScrollOffset(ptScrollOffset);

	for (int i = 0; i < m_arSubGauges.GetSize(); i++)
	{
		CBCGPGaugeImpl* pGauge = m_arSubGauges[i];
		ASSERT_VALID(pGauge);

		pGauge->SetScrollOffset(ptScrollOffset);
	}
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld)
{
	CBCGPGaugeImpl::OnScaleRatioChanged(sizeScaleRatioOld);

	m_textFormat.Scale(GetScaleRatioMid());

	const double dblAspectRatioX = m_sizeScaleRatio.cx / sizeScaleRatioOld.cx;
	const double dblAspectRatioY = m_sizeScaleRatio.cy / sizeScaleRatioOld.cy;

	for (int i = 0; i < m_arSubGauges.GetSize(); i++)
	{
		CBCGPGaugeImpl* pObject = m_arSubGauges[i];
		if (pObject == NULL)
		{
			continue;
		}

		ASSERT_VALID(pObject);

		pObject->SetScaleRatio(m_sizeScaleRatio);

		if (m_sizeScaleRatio != CBCGPSize(1., 1.) || !pObject->ResetRect())
		{
			BOOL bDontResetRectSaved = pObject->m_bDontResetRect;
			pObject->m_bDontResetRect = TRUE;

			CBCGPRect rect = pObject->GetRect();
			rect.Scale (dblAspectRatioX, dblAspectRatioY);

			pObject->SetRect(rect);
			
			pObject->m_bDontResetRect = bDontResetRectSaved;
		}
	}

	SetDirty();
}
//*******************************************************************************
void CBCGPCircularGaugeImpl::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPGaugeImpl::CopyFrom(srcObj);

	const CBCGPCircularGaugeImpl& src = (const CBCGPCircularGaugeImpl&)srcObj;

	m_bShapeByTicksArea = src.m_bShapeByTicksArea;
	m_bDrawTextBeforeTicks = src.m_bDrawTextBeforeTicks;
	m_bDrawTicksOutsideFrame = src.m_bDrawTicksOutsideFrame;
	m_CapSize = src.m_CapSize;
	m_ScaleSize = src.m_ScaleSize;

	m_Colors.CopyFrom(src.m_Colors);

	m_textFormat = src.m_textFormat;

	RemoveAllSubGauges();
}
