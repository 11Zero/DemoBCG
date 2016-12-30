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
// BCGPLinearGaugeImpl.cpp: implementation of the CBCGPLinearGaugeImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPLinearGaugeImpl.h"

#include "BCGPMath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CBCGPLinearGaugePointer, CBCGPGaugeDataObject)
IMPLEMENT_DYNAMIC(CBCGPLinearGaugeCtrl, CBCGPVisualCtrl)

void CBCGPLinearGaugeColors::SetTheme(BCGP_LINEAR_GAUGE_COLOR_THEME theme)
{
	switch(theme)
	{
	case BCGP_LINEAR_GAUGE_SILVER:
		m_brPointerOutline.SetColor(CBCGPColor::DarkGray);
		m_brPointerFill.SetColor(CBCGPColor::Gray);
		m_brText.SetColor(CBCGPColor::Gray);
		m_brFrameOutline.SetColor(CBCGPColor::Gray);
		m_brFrameFill.SetColors(CBCGPColor::DimGray, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brFill.SetColors(CBCGPColor::Silver, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brTickMarkFill.SetColors(CBCGPColor::LightGray, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP_LEFT);
		m_brTickMarkOutline.SetColor(CBCGPColor::Gray);
		break;

	case BCGP_LINEAR_GAUGE_BLUE:
		m_brPointerOutline.SetColor(CBCGPColor::SlateGray);
		m_brPointerFill.SetColor(CBCGPColor::DeepSkyBlue);
		m_brText.SetColor(CBCGPColor::SlateGray);
		m_brFrameOutline.SetColor(CBCGPColor::LightSteelBlue);
		m_brFrameFill.SetColors(CBCGPColor::SlateGray, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brFill.SetColors(CBCGPColor::LightSteelBlue, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brTickMarkFill.SetColors(CBCGPColor::SlateGray, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP_LEFT);
		m_brTickMarkOutline.SetColor(CBCGPColor::SlateGray);
		break;

	case BCGP_LINEAR_GAUGE_GOLD:
		m_brPointerOutline.SetColor(CBCGPColor::Maroon);
		m_brPointerFill.SetColor(CBCGPColor::SaddleBrown);
		m_brText.SetColor(CBCGPColor::DarkRed);
		m_brFrameOutline.SetColor(CBCGPColor::DarkGoldenrod);
		m_brFrameFill.SetColors(CBCGPColor::Peru, CBCGPColor::LightGoldenrodYellow, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brFill.SetColors(CBCGPColor::Wheat, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brTickMarkFill.SetColors(CBCGPColor::DarkGoldenrod, CBCGPColor::Wheat, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP_LEFT);
		m_brTickMarkOutline.SetColor(CBCGPColor::DarkGoldenrod);
		break;

	case BCGP_LINEAR_GAUGE_BLACK:
		m_brPointerOutline.SetColor(CBCGPColor::DarkGray);
		m_brPointerFill.SetColor(CBCGPColor::AntiqueWhite);
		m_brText.SetColor(CBCGPColor::AntiqueWhite);
		m_brFrameOutline.SetColor(CBCGPColor::DarkGray);
		m_brFrameFill.SetColors(CBCGPColor::Black, CBCGPColor::LightGray, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brFill.SetColor(CBCGPColor::DarkSlateGray);
		m_brTickMarkFill.SetColor(CBCGPColor::DarkGray);
		m_brTickMarkOutline.SetColor(CBCGPColor::AntiqueWhite);
		break;

	case BCGP_LINEAR_GAUGE_WHITE:
		m_brPointerOutline.SetColor(CBCGPColor::DarkRed);
		m_brPointerFill.SetColor(CBCGPColor::IndianRed);
		m_brText.SetColor(CBCGPColor::Gray);
		m_brFrameOutline.SetColor(CBCGPColor::Gray);
		m_brFrameFill.SetColors(CBCGPColor::DimGray, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_brFill.SetColor(CBCGPColor::FloralWhite);
		m_brTickMarkFill.SetColors(CBCGPColor::LightGray, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP_LEFT);
		m_brTickMarkOutline.SetColor(CBCGPColor::Gray);
		break;
	}
}

IMPLEMENT_DYNCREATE(CBCGPLinearGaugeImpl, CBCGPGaugeImpl)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPLinearGaugeImpl::CBCGPLinearGaugeImpl(CBCGPVisualContainer* pContainer) :
	CBCGPGaugeImpl(pContainer)
{
	m_nFrameSize = 2;
	m_bCacheImage = TRUE;
	m_bInSetValue = FALSE;
	m_bIsVertical = FALSE;
	m_dblMaxRangeSize = 0.;

	LOGFONT lf;
	globalData.fontBold.GetLogFont(&lf);

	m_textFormat.CreateFromLogFont(lf);
	m_textFormat.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);

	// Add default scale:
	AddScale();
}
//*******************************************************************************
CBCGPLinearGaugeImpl::~CBCGPLinearGaugeImpl()
{
}
//*******************************************************************************
void CBCGPLinearGaugeImpl::OnDraw(CBCGPGraphicsManager* pGMSrc, const CBCGPRect& /*rectClip*/, DWORD dwFlags/* = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC*/)
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
		int i = 0;

		for (i = 0; i < m_arScales.GetSize(); i++)
		{
			CBCGPGaugeScaleObject* pScale = m_arScales[i];
			ASSERT_VALID(pScale);

			pScale->CleanUp();
		}

		SetDirty(FALSE);
	}

	CBCGPRect rect = m_rect;
	rect.DeflateRect(1., 1.);

	const double scaleRatio = GetScaleRatioMid();
	const double nFrameSize = m_nFrameSize * scaleRatio;

	if (dwFlags & BCGP_DRAW_STATIC)
	{
		const CBCGPBrush& brFill = m_nFrameSize <= 2 ? m_Colors.m_brFill : m_Colors.m_brFrameFill;

		pGM->FillRectangle(rect, brFill);
		pGM->DrawRectangle(rect, m_Colors.m_brFrameOutline, scaleRatio);

		if (scaleRatio == 1.0 && !pGM->IsSupported(BCGP_GRAPHICS_MANAGER_ANTIALIAS))
		{
			CBCGPRect rect1 = rect;
			rect1.DeflateRect(1, 1);

			pGM->DrawRectangle(rect1, m_Colors.m_brFrameOutline);
		}

		if (m_nFrameSize > 2)
		{
			CBCGPRect rectInternal = rect;
			rectInternal.DeflateRect(nFrameSize, nFrameSize);

			pGM->FillRectangle(rectInternal, m_Colors.m_brFill);
			pGM->DrawRectangle(rectInternal, m_Colors.m_brFrameOutline, scaleRatio);
		}

		m_sizeMaxLabel = GetTextLabelMaxSize(pGM);

		int i = 0;

		m_dblMaxRangeSize = 0.;

		// Draw colored ranges:
		for (i = 0; i < m_arRanges.GetSize(); i++)
		{
			CBCGPGaugeColoredRangeObject* pRange = m_arRanges[i];
			ASSERT_VALID(pRange);

			CBCGPRect rectRange;
			CBCGPPolygonGeometry shapeRange;

			if (GetRangeShape(rectRange, shapeRange, 
				pRange->m_dblStartValue, pRange->m_dblFinishValue, 
				pRange->m_dblStartWidth, pRange->m_dblFinishWidth,
				pRange->m_dblOffsetFromFrame, pRange->m_nScale))
			{
				if (!rectRange.IsRectEmpty())
				{
					pGM->FillRectangle(rectRange, pRange->m_brFill);
					pGM->DrawRectangle(rectRange, pRange->m_brOutline, scaleRatio);
				}
				else
				{
					pGM->FillGeometry(shapeRange, pRange->m_brFill);
					pGM->DrawGeometry(shapeRange, pRange->m_brOutline, scaleRatio);
				}

				m_dblMaxRangeSize = max(m_dblMaxRangeSize, max(pRange->m_dblStartWidth, pRange->m_dblFinishWidth) * scaleRatio);
			}
		}

		// Draw scales:
		for (i = 0; i < m_arScales.GetSize(); i++)
		{
			OnDrawScale(pGM, i);
		}
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
		CBCGPRect rectSaved = m_rect;
		m_rect.OffsetRect(-m_ptScrollOffset);

		int i = 0;

		for (i = 0; i < m_arData.GetSize(); i++)
		{
			CBCGPPointsArray pts;
			CBCGPPointsArray ptsShadow;

			CreatePointerPoints(pts, i, FALSE);

			if (pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY))
			{
				CreatePointerPoints(ptsShadow, i, TRUE);
			}

			OnDrawPointer(pGM, pts, ptsShadow, i);
		}

		m_rect = rectSaved;
	}
}
//*******************************************************************************
void CBCGPLinearGaugeImpl::OnDrawScale(CBCGPGraphicsManager* pGM, int nScale)
{
	CBCGPGaugeScaleObject* pScale = GetScale(nScale);
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
		CBCGPRect rectRange;
		CBCGPPolygonGeometry shapeRange;

		if (GetRangeShape(rectRange, shapeRange, 
			pScale->m_dblStart, pScale->m_dblFinish, 
			pScale->m_dblMajorTickMarkSize, pScale->m_dblMajorTickMarkSize,
			0, nScale))
		{
			if (!rectRange.IsRectEmpty())
			{
				pGM->FillRectangle(rectRange, brFill);
				pGM->DrawRectangle(rectRange, brOutline, scaleRatio);
			}
			else
			{
				pGM->FillGeometry(shapeRange, brFill);
				pGM->DrawGeometry(shapeRange, brOutline, scaleRatio);
			}
		}
	}

	int i = 0;
	double dblStep = (pScale->m_dblFinish > pScale->m_dblStart) ? pScale->m_dblStep : -pScale->m_dblStep;

	double dblMinorTickSize = pScale->m_dblMinorTickMarkSize * scaleRatio;
	double dblMajorTickSize = pScale->m_dblMajorTickMarkSize * scaleRatio;
	CBCGPGaugeScaleObject::BCGP_TICKMARK_POSITION position = pScale->m_MinorTickMarkPosition;
	
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

		double dblCurrTickSize = bIsMajorTick ? dblMajorTickSize : dblMinorTickSize;

		CBCGPPoint ptFrom;
		ValueToPoint(dblVal, ptFrom, nScale);

		if (!bIsMajorTick && position != CBCGPGaugeScaleObject::BCGP_TICKMARK_POSITION_NEAR && 
			dblCurrTickSize < dblMajorTickSize)
		{
			double dblDeltaTick = dblMajorTickSize - dblCurrTickSize;

			if (position == CBCGPGaugeScaleObject::BCGP_TICKMARK_POSITION_CENTER)
			{
				dblDeltaTick /= 2.0;
			}

			if (m_bIsVertical)
			{
				ptFrom.x += dblDeltaTick;
			}
			else
			{
				ptFrom.y += dblDeltaTick;
			}
		}

		CBCGPPoint ptTo = ptFrom;

		if (m_bIsVertical)
		{
			ptTo.x += dblCurrTickSize;
		}
		else
		{
			ptTo.y += dblCurrTickSize;
		}

		if (dblCurrTickSize > 0.)
		{
			OnDrawTickMark(pGM, ptFrom, ptTo, 
				bIsMajorTick ? pScale->m_MajorTickMarkStyle : pScale->m_MinorTickMarkStyle,
				bIsMajorTick, dblVal, nScale,
				bIsMajorTick ? pScale->m_brTickMarkMajor : pScale->m_brTickMarkMinor,
				bIsMajorTick ? pScale->m_brTickMarkMajorOutline : pScale->m_brTickMarkMinorOutline);
		}

		if (bIsMajorTick)
		{
			CString strLabel;
			GetTickMarkLabel(strLabel, pScale->m_strLabelFormat, dblVal, nScale);

			if (!strLabel.IsEmpty())
			{
				double offset = 0;

				if (pScale->m_dblMajorTickMarkSize == 0.)
				{
					offset = m_dblMaxRangeSize;
				}

				CBCGPRect rectText;

				double xLabel = ptTo.x;
				double yLabel = ptTo.y;

				CBCGPSize sizeText = GetTickMarkTextLabelSize(pGM, strLabel, m_textFormat);

				if (m_bIsVertical)
				{
					rectText.left = xLabel + 2 + offset;
					rectText.top = yLabel - sizeText.cy / 2;
				}
				else
				{
					rectText.left = xLabel - sizeText.cx / 2;
					rectText.top = yLabel + offset;
				}

				rectText.right = rectText.left + sizeText.cx;
				rectText.bottom = rectText.top + sizeText.cy;

				OnDrawTickMarkTextLabel(pGM, m_textFormat, rectText, strLabel, dblVal, nScale, 
					pScale->m_brText.IsEmpty() ? m_Colors.m_brText : pScale->m_brText);
			}
		}
	}
}
//**********************************************************************************************************
void CBCGPLinearGaugeImpl::OnDrawTickMark(CBCGPGraphicsManager* pGM, const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo, 
											CBCGPGaugeScaleObject::BCGP_TICKMARK_STYLE style, BOOL bMajor, 
											double dblVal, int nScale, 
											const CBCGPBrush& brFillIn, const CBCGPBrush& brOutlineIn)
{
	ASSERT_VALID(pGM);

	CBCGPGaugeScaleObject* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	const double scaleRatio = GetScaleRatioMid();
	const double dblSize = (bMajor ? pScale->m_dblMajorTickMarkSize : pScale->m_dblMinorTickMarkSize) * scaleRatio;

	const CBCGPGaugeColoredRangeObject* pRange = GetColoredRangeByValue(dblVal, nScale, BCGP_GAUGE_RANGE_TICKMARK_COLOR);

	const CBCGPBrush& brFill = (pRange != NULL && !pRange->m_brTickMarkFill.IsEmpty()) ?
		pRange->m_brTickMarkFill : (brFillIn.IsEmpty() ? m_Colors.m_brTickMarkFill : brFillIn);

	const CBCGPBrush& brOutline = (pRange != NULL && !pRange->m_brTickMarkOutline.IsEmpty()) ?
		pRange->m_brTickMarkOutline : (brOutlineIn.IsEmpty() ? m_Colors.m_brTickMarkOutline : brOutlineIn);

	switch (style)
	{
	case CBCGPGaugeScaleObject::BCGP_TICKMARK_LINE:
		pGM->DrawLine(ptFrom, ptTo, brOutline, scaleRatio);
		break;

	case CBCGPGaugeScaleObject::BCGP_TICKMARK_TRIANGLE:
	case CBCGPGaugeScaleObject::BCGP_TICKMARK_TRIANGLE_INV:
	case CBCGPGaugeScaleObject::BCGP_TICKMARK_BOX:
		{
			CBCGPPointsArray arPoints;
			
			const double dblWidth = (style == CBCGPGaugeScaleObject::BCGP_TICKMARK_BOX) ?
				(dblSize / 4.) : (bMajor ? (dblSize * 2. / 3.) : (dblSize / 2.));

			const double dx = m_bIsVertical ? dblSize : max(dblWidth * .5, 3);
			const double dy = m_bIsVertical ? max(dblWidth * .5, 3) : dblSize;

			double x = ptTo.x;
			double y = ptTo.y;

			switch (style)
			{
			case CBCGPGaugeScaleObject::BCGP_TICKMARK_TRIANGLE:
				if (m_bIsVertical)
				{
					arPoints.Add(CBCGPPoint(x - dx, y - dy));
					arPoints.Add(CBCGPPoint(x - dx, y + dy));
					arPoints.Add(CBCGPPoint(x, y));
				}
				else
				{
					arPoints.Add(CBCGPPoint(x + dx, y - dy));
					arPoints.Add(CBCGPPoint(x - dx, y - dy));
					arPoints.Add(CBCGPPoint(x, y));
				}
				break;

			case CBCGPGaugeScaleObject::BCGP_TICKMARK_TRIANGLE_INV:
				if (m_bIsVertical)
				{
					arPoints.Add(CBCGPPoint(x, y - dy));
					arPoints.Add(CBCGPPoint(x, y + dy));
					arPoints.Add(CBCGPPoint(x - dx, y));
				}
				else
				{
					arPoints.Add(CBCGPPoint(x + dx, y));
					arPoints.Add(CBCGPPoint(x - dx, y));
					arPoints.Add(CBCGPPoint(x, y - dy));
				}
				break;

			case CBCGPGaugeScaleObject::BCGP_TICKMARK_BOX:
				if (m_bIsVertical)
				{
					arPoints.Add(CBCGPPoint(x - dx, y - dy));
					arPoints.Add(CBCGPPoint(x - dx, y + dy));

					arPoints.Add(CBCGPPoint(x, y + dy));
					arPoints.Add(CBCGPPoint(x, y - dy));
				}
				else
				{
					arPoints.Add(CBCGPPoint(x + dx, y - dy));
					arPoints.Add(CBCGPPoint(x - dx, y - dy));

					arPoints.Add(CBCGPPoint(x - dx, y));
					arPoints.Add(CBCGPPoint(x + dx, y));
				}
				break;
			}

			CBCGPPolygonGeometry rgn(arPoints);

			pGM->FillGeometry(rgn, brFill);
			pGM->DrawGeometry(rgn, brOutline, scaleRatio);
		}
		break;

	case CBCGPGaugeScaleObject::BCGP_TICKMARK_CIRCLE:
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
static void CreateRectFromArray(const CBCGPPointsArray& arPoints, CBCGPRect& rect)
{
	rect.right = rect.left = arPoints[0].x;
	rect.bottom = rect.top = arPoints[0].y;

	for (int i = 1; i < 4; i++)
	{
		rect.left = min(rect.left, arPoints[i].x);
		rect.right = max(rect.right, arPoints[i].x);

		rect.top = min(rect.top, arPoints[i].y);
		rect.bottom = max(rect.bottom, arPoints[i].y);
	}
}
//*******************************************************************************
void CBCGPLinearGaugeImpl::OnDrawPointer(CBCGPGraphicsManager* pGM, 
	const CBCGPPointsArray& arPoints, 
	const CBCGPPointsArray& arShadowPoints,
	int nIndex)
{
	ASSERT_VALID(pGM);

	CBCGPLinearGaugePointer* pData = DYNAMIC_DOWNCAST(CBCGPLinearGaugePointer, m_arData[nIndex]);
	ASSERT_VALID(pData);
	
	const int nCount = (int)arPoints.GetSize ();

	const double scaleRatio = GetScaleRatioMid();
	const CBCGPBrush& brFill = pData->GetFillBrush().IsEmpty() ? m_Colors.m_brPointerFill : pData->GetFillBrush();
	const CBCGPBrush& brFrame = pData->GetOutlineBrush().IsEmpty() ? m_Colors.m_brPointerOutline : pData->GetOutlineBrush();

	if (pData->GetStyle() == CBCGPLinearGaugePointer::BCGP_GAUGE_NEEDLE_CIRCLE)
	{
		ASSERT(nCount == 4);

		if (pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY) &&
			arShadowPoints.GetSize() == 4)
		{
			CBCGPRect rectShadow;
			CreateRectFromArray(arShadowPoints, rectShadow);

			pGM->FillEllipse(rectShadow, m_brShadow);
		}

		CBCGPRect rect;
		CreateRectFromArray(arPoints, rect);

		pGM->FillEllipse(rect, brFill);
		pGM->DrawEllipse(rect, brFrame, scaleRatio);
	}
	else
	{
		if (pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY) &&
			arShadowPoints.GetSize() > 2)
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
}
//*******************************************************************************
void CBCGPLinearGaugeImpl::CreatePointerPoints(CBCGPPointsArray& arPoints, int nPointerIndex, BOOL bShadow)
{
	if (m_rect.IsRectEmpty())
	{
		return;
	}

	CBCGPLinearGaugePointer* pData = DYNAMIC_DOWNCAST(CBCGPLinearGaugePointer, m_arData[nPointerIndex]);
	if (pData == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	CBCGPGaugeScaleObject* pScale = GetScale(pData->GetScale());
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	const double scaleRatio = GetScaleRatioMid();

	double dblValue = pData->m_nAnimTimerID != 0 ? pData->m_dblAnimatedValue : pData->m_dblValue;

	double dblSizeMax = max(pScale->m_dblMajorTickMarkSize, 2. * pScale->m_dblMinorTickMarkSize) * scaleRatio;
	double dblSize = dblSizeMax;
	if (dblSize == 0)
	{
		dblSize = 8. * scaleRatio;
	}

	double dblSizePerc = bcg_clamp(pData->m_dblSize, 0.0, 1.0);
	if (dblSizePerc != 0.0)
	{
		dblSize = max(dblSize * dblSizePerc, 2.0);
	}

	CBCGPPoint point;

	if (!ValueToPoint(dblValue, point, pData->GetScale()))
	{
		return;
	}

	double offset = max(pScale->m_dblMajorTickMarkSize, pScale->m_dblMinorTickMarkSize) * scaleRatio;
	offset = max(offset, m_dblMaxRangeSize);

	double x = point.x;
	double y = point.y;

	if (dblSize < dblSizeMax && pData->GetPosition() != CBCGPLinearGaugePointer::BCGP_GAUGE_POSITION_NEAR)
	{
		double dblSizeDelta = dblSizeMax - dblSize;
		if (pData->GetPosition() == CBCGPLinearGaugePointer::BCGP_GAUGE_POSITION_CENTER)
		{
			dblSizeDelta /= 2.0;
		}

		if (m_bIsVertical)
		{
			x += dblSizeDelta;
		}
		else
		{
			y += dblSizeDelta;
		}
	}

	if (bShadow)
	{
		x += 2. * m_sizeScaleRatio.cx;
		y += 2. * m_sizeScaleRatio.cy;
	}

	const double delta = pData->m_dblWidth == 0 ? dblSize / 2 : (pData->m_dblWidth * scaleRatio) / 2;

	switch (pData->GetStyle())
	{
	case CBCGPLinearGaugePointer::BCGP_GAUGE_NEEDLE_TRIANGLE:
		if (m_bIsVertical)
		{
			arPoints.Add(CBCGPPoint(x, y - delta));
			arPoints.Add(CBCGPPoint(x, y + delta));
			arPoints.Add(CBCGPPoint(x + dblSize, y));
		}
		else
		{
			arPoints.Add(CBCGPPoint(x + delta, y));
			arPoints.Add(CBCGPPoint(x - delta, y));
			arPoints.Add(CBCGPPoint(x, y + dblSize));
		}
		break;

	case CBCGPLinearGaugePointer::BCGP_GAUGE_NEEDLE_TRIANGLE_INV:
		if (m_bIsVertical)
		{
			arPoints.Add(CBCGPPoint(x + dblSize, y - delta));
			arPoints.Add(CBCGPPoint(x + dblSize, y + delta));
			arPoints.Add(CBCGPPoint(x, y));
		}
		else
		{
			arPoints.Add(CBCGPPoint(x + delta, y + dblSize));
			arPoints.Add(CBCGPPoint(x - delta, y + dblSize));
			arPoints.Add(CBCGPPoint(x, y));
		}
		break;

	case CBCGPLinearGaugePointer::BCGP_GAUGE_NEEDLE_RECT:
	case CBCGPLinearGaugePointer::BCGP_GAUGE_NEEDLE_CIRCLE:
		if (m_bIsVertical)
		{
			arPoints.Add(CBCGPPoint(x, y + delta));
			arPoints.Add(CBCGPPoint(x, y - delta));

			arPoints.Add(CBCGPPoint(x + dblSize, y - delta));
			arPoints.Add(CBCGPPoint(x + dblSize, y + delta));
		}
		else
		{
			arPoints.Add(CBCGPPoint(x + delta, y + dblSize));
			arPoints.Add(CBCGPPoint(x - delta, y + dblSize));

			arPoints.Add(CBCGPPoint(x - delta, y));
			arPoints.Add(CBCGPPoint(x + delta, y));
		}
		break;

	case CBCGPLinearGaugePointer::BCGP_GAUGE_NEEDLE_DIAMOND:
		if (m_bIsVertical)
		{
			arPoints.Add(CBCGPPoint(x + .5 * dblSize, y + delta));
			arPoints.Add(CBCGPPoint(x, y));

			arPoints.Add(CBCGPPoint(x + .5 * dblSize, y - delta));
			arPoints.Add(CBCGPPoint(x + dblSize, y));
		}
		else
		{
			arPoints.Add(CBCGPPoint(x, y + dblSize));
			arPoints.Add(CBCGPPoint(x - delta, y + .5 * dblSize));

			arPoints.Add(CBCGPPoint(x, y));
			arPoints.Add(CBCGPPoint(x + delta, y + .5 * dblSize));
		}
	}
}
//*******************************************************************************
int CBCGPLinearGaugeImpl::AddPointer(const CBCGPLinearGaugePointer& pointer, int nScale, BOOL bRedraw)
{
	CBCGPGaugeScaleObject* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return -1;
	}

	CBCGPLinearGaugePointer* pData = new CBCGPLinearGaugePointer;
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
BOOL CBCGPLinearGaugeImpl::RemovePointer(int nIndex, BOOL bRedraw)
{
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
BOOL CBCGPLinearGaugeImpl::ModifyPointer(int nIndex, const CBCGPLinearGaugePointer& pointer, BOOL bRedraw)
{
	if (nIndex < 0 || nIndex >= m_arData.GetSize())
	{
		return FALSE;
	}

	CBCGPLinearGaugePointer* pPointer =
		DYNAMIC_DOWNCAST(CBCGPLinearGaugePointer, m_arData[nIndex]);
	ASSERT_VALID(pPointer);

	pPointer->m_Style = pointer.m_Style;
	pPointer->m_dblSize = pointer.m_dblSize;
	pPointer->m_dblWidth = pointer.m_dblWidth;

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
BOOL CBCGPLinearGaugeImpl::GetRangeShape(CBCGPRect& rect, CBCGPPolygonGeometry& shape, double dblStartValue, double dblFinishValue,
	double dblStartWidth, double dblFinishWidth,
	double dblOffsetFromFrame, int nScale)
{
	rect.SetRectEmpty();
	shape.Clear();

	CBCGPPoint pt1;
	if (!ValueToPoint(dblStartValue, pt1, nScale))
	{
		return FALSE;
	}

	CBCGPPoint pt2;
	if (!ValueToPoint(dblFinishValue, pt2, nScale))
	{
		return FALSE;
	}

	const double scaleRatio = GetScaleRatioMid();

	if (dblStartWidth == 0.)
	{
		CBCGPGaugeScaleObject* pScale = GetScale(nScale);
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
	dblOffsetFromFrame *= scaleRatio;

	if (dblFinishWidth == dblStartWidth)
	{
		if (m_bIsVertical)
		{
			rect.left = pt1.x + dblOffsetFromFrame;
			rect.top = pt1.y;
			rect.right = pt1.x + dblStartWidth + dblOffsetFromFrame;
			rect.bottom = pt2.y;
		}
		else
		{
			rect.left = pt1.x;
			rect.top = pt1.y + dblOffsetFromFrame;
			rect.right = pt2.x;
			rect.bottom = pt1.y + dblStartWidth + dblOffsetFromFrame;
		}
	}
	else
	{
		CBCGPPointsArray arPoints;

		if (m_bIsVertical)
		{
			arPoints.Add(CBCGPPoint(pt1.x + dblOffsetFromFrame, pt1.y));
			arPoints.Add(CBCGPPoint(pt2.x + dblOffsetFromFrame, pt2.y));
			arPoints.Add(CBCGPPoint(pt2.x + dblOffsetFromFrame + dblFinishWidth, pt2.y));
			arPoints.Add(CBCGPPoint(pt1.x + dblOffsetFromFrame + dblStartWidth, pt1.y));
		}
		else
		{
			arPoints.Add(CBCGPPoint(pt1.x, pt1.y + dblOffsetFromFrame));
			arPoints.Add(CBCGPPoint(pt2.x, pt2.y + dblOffsetFromFrame));
			arPoints.Add(CBCGPPoint(pt2.x, pt2.y + dblOffsetFromFrame + dblFinishWidth));
			arPoints.Add(CBCGPPoint(pt1.x, pt1.y + dblOffsetFromFrame + dblStartWidth));
		}

		shape.SetPoints(arPoints);
	}

	return TRUE;
}
//*******************************************************************************
int CBCGPLinearGaugeImpl::AddScale()
{
	CBCGPGaugeScaleObject* pScale = new CBCGPGaugeScaleObject;
	pScale->SetParentGauge (this);

	m_arScales.Add(pScale);
	return (int)m_arScales.GetSize() - 1;
}
//*******************************************************************************
BOOL CBCGPLinearGaugeImpl::SetValue(double dblVal, int nIndex, UINT uiAnimationTime, BOOL bRedraw)
{
	m_bInSetValue = TRUE;

	BOOL bIsDirty = m_bIsDirty;

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
void CBCGPLinearGaugeImpl::SetDirty(BOOL bSet, BOOL bRedraw)
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

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
CBCGPSize CBCGPLinearGaugeImpl::GetDefaultSize(CBCGPGraphicsManager* /*pGM*/, const CBCGPBaseVisualObject* /*pParentGauge*/)
{
	CBCGPSize size(100. * m_sizeScaleRatio.cx, 20. * m_sizeScaleRatio.cy);

	if (m_bIsVertical)
	{
		size.Swap();
	}

	return size;
}
//*******************************************************************************
BOOL CBCGPLinearGaugeImpl::ValueToPoint(double dblValue, CBCGPPoint& point, int nScale) const
{
	CBCGPGaugeScaleObject* pScale = GetScale(nScale);
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

	if (pScale->m_dblFinish == pScale->m_dblStart)
	{
		return FALSE;
	}

	CBCGPRect rect = m_rect;

	if (m_bIsVertical)
	{
		rect.DeflateRect((pScale->m_dblOffsetFromFrame + m_nFrameSize) * m_sizeScaleRatio.cx, m_sizeMaxLabel.cy / 2 + (m_nFrameSize + 1) * m_sizeScaleRatio.cy);
	}
	else
	{
		rect.DeflateRect(m_sizeMaxLabel.cx / 2 + (m_nFrameSize + 1) * m_sizeScaleRatio.cx, (pScale->m_dblOffsetFromFrame + m_nFrameSize) * m_sizeScaleRatio.cy);
	}

	const double dblTotalSize = m_bIsVertical ? rect.Height() : rect.Width();
	const double delta = dblTotalSize * fabs(dblValue - pScale->m_dblStart) / fabs(pScale->m_dblFinish - pScale->m_dblStart);

	point.x = m_bIsVertical ? rect.left : rect.left + delta;
	point.y = m_bIsVertical ? rect.bottom - delta : rect.top;

	return TRUE;
}
//*******************************************************************************
void CBCGPLinearGaugeImpl::SetTextFormat(const CBCGPTextFormat& textFormat)
{
	m_textFormat.Destroy();
	m_textFormat.CopyFrom(textFormat);
	SetDirty();
}
//*******************************************************************************
CBCGPSize CBCGPLinearGaugeImpl::GetTextLabelMaxSize(CBCGPGraphicsManager* pGM)
{
	CBCGPSize sizeMaxLabel (5., 5.);

	for (int iScale = 0; iScale < m_arScales.GetSize(); iScale++)
	{
		CBCGPGaugeScaleObject* pScale = m_arScales[iScale];
		ASSERT_VALID(pScale);

		double dblStep = (pScale->m_dblFinish > pScale->m_dblStart) ? pScale->m_dblStep : -pScale->m_dblStep;

		int i = 0;

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

			if (bIsMajorTick)
			{
				CString strLabel;
				GetTickMarkLabel(strLabel, pScale->m_strLabelFormat, dblVal, i);

				if (!strLabel.IsEmpty())
				{
					CBCGPSize sizeText = GetTickMarkTextLabelSize(pGM, strLabel, m_textFormat);

					sizeMaxLabel.cx = max(sizeMaxLabel.cx, sizeText.cx);
					sizeMaxLabel.cy = max(sizeMaxLabel.cy, sizeText.cy);
				}
			}
		}
	}

	return sizeMaxLabel;
}
//*******************************************************************************
BOOL CBCGPLinearGaugeImpl::HitTestValue(const CBCGPPoint& pt, double& dblValue, int nScale, BOOL bInsideGauge) const
{
	CBCGPGaugeScaleObject* pScale = GetScale(nScale);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CBCGPRect rect = m_rect;
	rect.OffsetRect(-m_ptScrollOffset);

	if (m_bIsVertical)
	{
		rect.DeflateRect((pScale->m_dblOffsetFromFrame + m_nFrameSize) * m_sizeScaleRatio.cx, m_sizeMaxLabel.cy / 2 + (m_nFrameSize + 1) * m_sizeScaleRatio.cy);
	}
	else
	{
		rect.DeflateRect(m_sizeMaxLabel.cx / 2 + (m_nFrameSize + 1) * m_sizeScaleRatio.cx, (pScale->m_dblOffsetFromFrame + m_nFrameSize) * m_sizeScaleRatio.cy);
	}

	if (bInsideGauge && !rect.PtInRect(pt))
	{
		return FALSE;
	}

	if (m_bIsVertical)
	{
		const double dblTotalSize = rect.Height();
		dblValue = pScale->m_dblStart + (rect.bottom - pt.y) * fabs(pScale->m_dblFinish - pScale->m_dblStart) / dblTotalSize;
	}
	else
	{
		const double dblTotalSize = rect.Width();
		dblValue = pScale->m_dblStart + (pt.x - rect.left) * fabs(pScale->m_dblFinish - pScale->m_dblStart) / dblTotalSize;
	}

	return TRUE;
}
//*******************************************************************************
BOOL CBCGPLinearGaugeImpl::OnSetMouseCursor(const CBCGPPoint& pt)
{
	if (m_bIsInteractiveMode)
	{
		CBCGPRect rect = m_rect;
		rect.DeflateRect(m_nFrameSize, m_nFrameSize);

		if (rect.PtInRect(pt))
		{
			::SetCursor (globalData.GetHandCursor());
			return TRUE;
		}
	}

	return CBCGPGaugeImpl::OnSetMouseCursor(pt);
}
//*******************************************************************************
void CBCGPLinearGaugeImpl::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld)
{
	CBCGPGaugeImpl::OnScaleRatioChanged(sizeScaleRatioOld);

	m_textFormat.Scale(GetScaleRatioMid());
	SetDirty();
}
//*******************************************************************************
void CBCGPLinearGaugeImpl::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPGaugeImpl::CopyFrom(srcObj);

	const CBCGPLinearGaugeImpl& src = (const CBCGPLinearGaugeImpl&)srcObj;

	m_bIsVertical = src.m_bIsVertical;
	m_Colors.CopyFrom(src.m_Colors);
	m_textFormat = src.m_textFormat;
}
