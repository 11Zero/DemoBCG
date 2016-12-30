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
// BCGPKnob.cpp: implementation of the CBCGPKnob class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPKnob.h"
#include "BCGPMath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CBCGPKnob, CBCGPCircularGaugeImpl)
IMPLEMENT_DYNAMIC(CBCGPKnobCtrl, CBCGPVisualCtrl)
IMPLEMENT_DYNCREATE(CBCGPKnobPointer, CBCGPGaugeDataObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPKnob::CBCGPKnob(CBCGPVisualContainer* pContainer) :
	CBCGPCircularGaugeImpl(pContainer)
{
	m_bCacheImage = FALSE;
	m_bIsInteractiveMode = TRUE;
	m_nFrameSize = 2;
	m_bDrawTextBeforeTicks = TRUE;
	m_bDrawTicksOutsideFrame = TRUE;
	m_CapSize = 0.;

	SetColors(CBCGPCircularGaugeColors::BCGP_CIRCULAR_GAUGE_SILVER);

	SetInteractiveMode();
	SetPointer(CBCGPKnobPointer(), FALSE);
}
//*******************************************************************************
CBCGPKnob::~CBCGPKnob()
{
}
//*******************************************************************************
void CBCGPKnob::SetColors(CBCGPCircularGaugeColors::BCGP_CIRCULAR_GAUGE_COLOR_THEME theme)
{
	m_Colors.SetTheme(theme);

	m_Colors.m_brPointerFill.SetColors(
		m_Colors.m_brFill.GetGradientColor(),
		m_Colors.m_brFill.GetColor(),
		CBCGPBrush::BCGP_GRADIENT_VERTICAL,
		m_Colors.m_brFill.GetOpacity());

	switch (theme)
	{
	case CBCGPCircularGaugeColors::BCGP_CIRCULAR_GAUGE_SILVER:
	case CBCGPCircularGaugeColors::BCGP_CIRCULAR_GAUGE_BLUE:
		break;

	case CBCGPCircularGaugeColors::BCGP_CIRCULAR_GAUGE_GOLD:
		{
			CBCGPColor clrPointer = m_Colors.m_brFill.GetColor();
			clrPointer.MakeDarker(.05);

			m_Colors.m_brPointerFill.SetColors(
				CBCGPColor::White,
				clrPointer,
				CBCGPBrush::BCGP_GRADIENT_VERTICAL,
				m_Colors.m_brFill.GetOpacity());
			m_Colors.m_brPointerOutline = m_Colors.m_brFrameOutline;
		}
		break;

	case CBCGPCircularGaugeColors::BCGP_CIRCULAR_GAUGE_BLACK:
		m_Colors.m_brPointerFill.SetColors(
			CBCGPColor::Gray,
			m_Colors.m_brFill.GetColor(),
			CBCGPBrush::BCGP_GRADIENT_VERTICAL,
			m_Colors.m_brFill.GetOpacity());

		m_Colors.m_brFill.SetColors(CBCGPColor::DarkSlateGray, CBCGPColor::Gray, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
		m_Colors.m_brText.SetColor(CBCGPColor::Gray);
		m_Colors.m_brTickMarkOutline.SetColor(CBCGPColor::Gray);
		break;

	case CBCGPCircularGaugeColors::BCGP_CIRCULAR_GAUGE_WHITE:
		{
			m_Colors.m_brFill.SetColors(CBCGPColor::AntiqueWhite, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);

			CBCGPColor clrPointer = m_Colors.m_brFill.GetColor();
			clrPointer.MakeDarker(.05);

			m_Colors.m_brPointerOutline = m_Colors.m_brFrameOutline;
			m_Colors.m_brPointerFill.SetColors(
				CBCGPColor::White,
				clrPointer,
				CBCGPBrush::BCGP_GRADIENT_VERTICAL,
				m_Colors.m_brFill.GetOpacity());
		}
		break;
	}

	SetDirty();
}
//*******************************************************************************
void CBCGPKnob::CreatePointerPoints(double dblRadius, 
	CBCGPPointsArray& arPoints,
	int nPointerIndex, BOOL bShadow)
{
	if (m_rect.IsRectEmpty())
	{
		return;
	}

	CBCGPRect rect = m_rect;

	CBCGPKnobPointer* pData = DYNAMIC_DOWNCAST(CBCGPKnobPointer, m_arData[nPointerIndex]);
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

	const double scaleRatio = GetScaleRatioMid();

	double dblValue = pData->IsAnimated() ? pData->GetAnimatedValue() : pData->GetValue();

	double dblOffset = bcg_clamp(pData->GetOffsetFromCenter(), 0.0, 1.0);
	if (dblOffset == 0.0)
	{
		dblOffset = dblRadius * .5;
	}
	else
	{
		dblOffset = dblRadius * bcg_clamp(dblOffset, 0.5, 1.0);
	}

	double dblAngle = bcg_deg2rad(pScale->GetStartAngle()) - bcg_deg2rad(pScale->GetStartAngle() - pScale->GetFinishAngle()) * (dblValue - pScale->GetStart()) / (pScale->GetFinish() - pScale->GetStart());
	dblAngle = bcg_normalize_rad (dblAngle);

	if (bShadow)
	{
		rect.OffsetRect(2 * m_sizeScaleRatio.cx, 2 * m_sizeScaleRatio.cy);
	}

	CBCGPPoint center((rect.left + rect.right) / 2.0, (rect.top + rect.bottom) / 2.0);

	const double angleCos  = cos(dblAngle);
	const double angleSin  = sin(dblAngle);

	double dblWidth = bcg_clamp(pData->GetWidth(), 0.0, dblRadius / 10.0);
	const double dblPointerAngle = dblAngle - M_PI_2;

	switch (pData->GetStyle())
	{
	case CBCGPKnobPointer::BCGP_KNOB_POINTER_HANDLE:
		{
			dblRadius -= .2 * scaleRatio;
			double dblExtend = (dblRadius * .9);
			double dblSize = dblRadius + dblExtend;
			center.x -= angleCos * dblExtend;
			center.y += angleSin * dblExtend;

			if (dblWidth == 0.0)
			{
				dblWidth = dblRadius / 3.0;
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

				arPoints.Add(CBCGPPoint(
					center.x + angleCos * (dblSize + dblArrowLen), 
					center.y - angleSin * (dblSize + dblArrowLen)));

				arPoints.Add(pt2);
			}
		}
		break;

	case CBCGPKnobPointer::BCGP_KNOB_POINTER_LINE:
		{
			if (bShadow)
			{
				return;
			}

			if (dblWidth == 0.0)
			{
				dblWidth = 2. * scaleRatio;
			}

			const double dx = cos(dblPointerAngle) * dblWidth;
			const double dy = -sin(dblPointerAngle) * dblWidth;

			arPoints.Add(CBCGPPoint(center.x + angleCos * dblOffset - dx, center.y - angleSin * dblOffset - dy));
			arPoints.Add(CBCGPPoint(center.x + angleCos * dblOffset + dx, center.y - angleSin * dblOffset + dy));

			dblOffset = dblRadius - 4. * scaleRatio;

			arPoints.Add(CBCGPPoint(center.x + angleCos * dblOffset + dx, center.y - angleSin * dblOffset + dy));
			arPoints.Add(CBCGPPoint(center.x + angleCos * dblOffset - dx, center.y - angleSin * dblOffset - dy));
		}
		break;

	case CBCGPKnobPointer::BCGP_KNOB_POINTER_CIRCLE:
		{
			if (bShadow)
			{
				return;
			}

			if (dblWidth == 0.0)
			{
				dblWidth = max(2. * scaleRatio, dblRadius / 8);
			}

			dblOffset = dblRadius - 6. * scaleRatio - dblWidth;

			arPoints.Add(CBCGPPoint(center.x + angleCos * dblOffset, center.y - angleSin * dblOffset));
			arPoints.Add(CBCGPPoint(dblWidth, dblWidth));
		}
		break;
	}
}
//*******************************************************************************
void CBCGPKnob::SetPointer(const CBCGPKnobPointer& pointer, BOOL bRedraw)
{
	CBCGPCircularGaugeScale* pScale = GetScale(0);
	if (pScale == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	double dblVal = pScale->GetStart();

	if (m_arData.GetSize() > 0)
	{
		dblVal = GetValue();

		delete m_arData[0];
		m_arData.RemoveAt(0);
	}

	CBCGPKnobPointer* pData = new CBCGPKnobPointer;
	pData->CopyFrom(pointer);

	pData->SetValue(dblVal);

	AddData(pData);

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPKnob::SetImageList(UINT uiResID, int cx, const CBCGPSize& szMargin)
{
	m_Icons.Destroy();

	m_sizeIcon = CBCGPSize(0, 0);
	
	if (uiResID != 0)
	{
		m_Icons.Load(uiResID);
		m_sizeIcon.cx = cx;

		m_sizeIconMargin = szMargin;
	}

	SetDirty();
}
//*******************************************************************************
CBCGPSize CBCGPKnob::GetTickMarkTextLabelSize(CBCGPGraphicsManager* pGM, const CString& strLabel, const CBCGPTextFormat& tf)
{
	if (m_sizeIcon.cx == 0)
	{
		return CBCGPCircularGaugeImpl::GetTickMarkTextLabelSize(pGM, strLabel, tf);
	}

	ASSERT_VALID(pGM);

	const double scaleRatio = GetScaleRatioMid();

	if (m_sizeIcon.cy == 0)
	{
		m_sizeIcon.cy = pGM->GetImageSize(m_Icons).cy;
	}

	return (m_sizeIcon + m_sizeIconMargin * 2.) * scaleRatio;
}
//*******************************************************************************
void CBCGPKnob::OnDrawTickMarkTextLabel(CBCGPGraphicsManager* pGM, const CBCGPTextFormat& tf, const CBCGPRect& rectText, const CString& strLabel, double dblVal, int nScale, const CBCGPBrush& br)
{
	if (m_sizeIcon.cx == 0)
	{
		CBCGPCircularGaugeImpl::OnDrawTickMarkTextLabel(pGM, tf, rectText, strLabel, dblVal, nScale, br);
		return;
	}

	if (m_sizeIcon.cy == 0)
	{
		m_sizeIcon.cy = pGM->GetImageSize(m_Icons).cy;
	}

	CBCGPSize sizeIcon(m_sizeIcon.cx * m_sizeScaleRatio.cx, m_sizeIcon.cy * m_sizeScaleRatio.cy);

	double cx = 0.;
	double cy = 0.;

	double angle = 0.;

	if (ValueToAngle(dblVal, angle, nScale))
	{
		cx = sizeIcon.cx * cos(bcg_deg2rad(angle)) / 2;
	}

	CBCGPPoint ptImage(
		rectText.left + max(0., .5 * (rectText.Width() - sizeIcon.cx)) + cx, 
		rectText.top + max(0., .5 * (rectText.Height() - sizeIcon.cy)) + cy);

	pGM->DrawImage(m_Icons, ptImage, sizeIcon, 1., CBCGPImage::BCGP_IMAGE_INTERPOLATION_MODE_LINEAR,
		CBCGPRect(CBCGPPoint(m_sizeIcon.cx * m_nCurrLabelIndex, 0), m_sizeIcon));
}
//*******************************************************************************
BOOL CBCGPKnob::OnGestureEventRotate(const CBCGPPoint& /*ptCenter*/, double dblAngle)
{
	double dblVal = GetValue();
	double dblCurrAngle = 0.;
	
	if (!ValueToAngle(dblVal, dblCurrAngle))
	{
		return FALSE;
	}

	dblCurrAngle += dblAngle;

	double dblNewValue = 0.;

	if (!AngleToValue(dblCurrAngle, dblNewValue))
	{
		return FALSE;
	}

	if (dblVal == dblNewValue)
	{
		return FALSE;
	}

	SetValue(dblNewValue, 0, 0, TRUE);
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPKnob::GetGestureConfig(CBCGPGestureConfig& gestureConfig)
{
	gestureConfig.EnableRotate();
	gestureConfig.EnablePan(FALSE);
	gestureConfig.EnableZoom(FALSE);
	return TRUE;
}
//*******************************************************************************
void CBCGPKnob::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPCircularGaugeImpl::CopyFrom(srcObj);

	const CBCGPKnob& src = (const CBCGPKnob&)srcObj;

	m_Icons = src.m_Icons;
	m_Icons.Destroy();

	m_sizeIcon = src.m_sizeIcon;
	m_sizeIconMargin = src.m_sizeIconMargin;
}
