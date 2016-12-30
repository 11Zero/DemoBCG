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
// BCGPColorIndicatorImpl.cpp: implementation of the CBCGPColorIndicatorImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPColorIndicatorImpl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CBCGPColorIndicatorColors::CBCGPColorIndicatorColors()
{
	m_brFill = CBCGPBrush(CBCGPColor::LimeGreen, CBCGPColor::White, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP_LEFT);
	m_brOutline.SetColor(m_brFill.GetColor());
	m_brOutline.MakeDarker(.1);
}

IMPLEMENT_DYNCREATE(CBCGPColorIndicatorImpl, CBCGPStaticGaugeImpl)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPColorIndicatorImpl::CBCGPColorIndicatorImpl(CBCGPVisualContainer* pContainer) :
	CBCGPStaticGaugeImpl(pContainer)
{
	m_Style = BCGP_COLOR_INDICATOR_ELLIPSE;
	m_bStretched = FALSE;

	CreateResources();
}
//*******************************************************************************
CBCGPColorIndicatorImpl::CBCGPColorIndicatorImpl(const CBCGPColor& color,
							const CBCGPColor& clrGradient, 
							const CBCGPColor& clrBorder,
							CBCGPVisualContainer* pContainer) :
	CBCGPStaticGaugeImpl(pContainer)
{
	m_Style = BCGP_COLOR_INDICATOR_ELLIPSE;
	m_bStretched = FALSE;

	SetColor(color, clrGradient, clrBorder);
}
//*******************************************************************************
CBCGPColorIndicatorImpl::CBCGPColorIndicatorImpl(const CBCGPBrush& brFill,
							const CBCGPBrush& brBorder,
							CBCGPVisualContainer* pContainer) :
	CBCGPStaticGaugeImpl(pContainer)
{
	m_Style = BCGP_COLOR_INDICATOR_ELLIPSE;
	m_bStretched = FALSE;

	SetColor(brFill, brBorder);
}
//*******************************************************************************
CBCGPColorIndicatorImpl::~CBCGPColorIndicatorImpl()
{
}
//*******************************************************************************
void CBCGPColorIndicatorImpl::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectClip*/, DWORD dwFlags)
{
	ASSERT_VALID(pGM);

	if (m_rect.IsRectEmpty() || !m_bIsVisible)
	{
		return;
	}

	if ((dwFlags & m_DefaultDrawFlags) == 0)
	{
		return;
	}

	CBCGPRect rectDraw = m_rect;
	if (m_bIsSubGauge)
	{
		rectDraw.OffsetRect(-m_ptScrollOffset);
	}
	
	pGM->FillRectangle (rectDraw, GetFillBrush ());

	CBCGPRect rect = rectDraw;

	if (!m_bStretched)
	{
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
	}

	const CBCGPColorIndicatorColors& colors = m_bOff ? m_LightColors : m_Colors;
	const double scaleRatio = GetScaleRatioMid();

	if (m_Style == BCGP_COLOR_INDICATOR_ELLIPSE)
	{
		CBCGPEllipse ellipse(rect);

		pGM->SetClipEllipse(ellipse);

		pGM->FillRectangle(rect, colors.m_brFill);

		pGM->ReleaseClipArea();

		pGM->DrawEllipse(ellipse, colors.m_brOutline, scaleRatio);

		if (pGM->GetType() == CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_GDI)
		{
			ellipse.radiusX = ellipse.radiusX - 1.;
			ellipse.radiusY = ellipse.radiusY - 1.;

			pGM->DrawEllipse(ellipse, colors.m_brOutline, scaleRatio);
		}
	}
	else
	{
		pGM->FillRectangle(rect, colors.m_brFill);

		pGM->DrawRectangle(rect, colors.m_brOutline, scaleRatio);

		if (pGM->GetType() == CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_GDI)
		{
			rect.DeflateRect(m_sizeScaleRatio.cx, m_sizeScaleRatio.cy);
			pGM->DrawRectangle(rect, colors.m_brOutline, scaleRatio);
		}
	}

	pGM->DrawRectangle (rectDraw, GetOutlineBrush (), scaleRatio);

	SetDirty (FALSE);
}
//*******************************************************************************
void CBCGPColorIndicatorImpl::SetColor(const CBCGPColor& color, const CBCGPColor& clrGradient, const CBCGPColor& clrBorder, BOOL bRedraw)
{
	if (m_Colors.m_brFill.GetColor() == color &&
		m_Colors.m_brFill.GetGradientColor() == clrGradient &&
		m_Colors.m_brOutline.GetColor() == clrBorder)
	{
		return;
	}

	m_Colors.m_brFill = CBCGPBrush(color, 
		clrGradient.IsNull() ? m_Colors.m_brFill.GetGradientColor() : clrGradient, 
		CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP_LEFT);

	if (!clrBorder.IsNull())
	{
		m_Colors.m_brOutline.SetColor(clrBorder);
	}
	else
	{
		m_Colors.m_brOutline.SetColor(color);
		m_Colors.m_brOutline.MakeDarker(.1);
	}

	CreateResources();
	SetDirty();

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPColorIndicatorImpl::SetColor(const CBCGPBrush& brFill, const CBCGPBrush& brBorder, BOOL bRedraw)
{
	if (m_Colors.m_brFill == brFill && m_Colors.m_brOutline == brBorder)
	{
		return;
	}

	m_Colors.m_brFill = brFill;
	m_Colors.m_brOutline = brBorder;

	CreateResources();
	SetDirty();

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPColorIndicatorImpl::CreateResources()
{
	m_LightColors = m_Colors;

	m_LightColors.m_brFill.MakeLighter(.3);
	m_LightColors.m_brOutline.MakeLighter(.3);
}
//*******************************************************************************
CBCGPSize CBCGPColorIndicatorImpl::GetDefaultSize(CBCGPGraphicsManager* /*pGM*/, const CBCGPBaseVisualObject* /*pParentGauge*/)
{
	return CBCGPSize(15. * m_sizeScaleRatio.cx, 15. * m_sizeScaleRatio.cy);
}
//*******************************************************************************
void CBCGPColorIndicatorImpl::SetOpacity(double opacity, BOOL bRedraw)
{
	if (GetOpacity () != opacity)
	{
		CBCGPStaticGaugeImpl::SetOpacity (opacity, FALSE);
	}

	m_Colors.m_brFill.SetOpacity(opacity);
	m_Colors.m_brOutline.SetOpacity(opacity);

	m_LightColors.m_brFill.SetOpacity(opacity);
	m_LightColors.m_brOutline.SetOpacity(opacity);

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPColorIndicatorImpl::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPStaticGaugeImpl::CopyFrom(srcObj);

	const CBCGPColorIndicatorImpl& src = (const CBCGPColorIndicatorImpl&)srcObj;

	m_Colors = src.m_Colors;
	m_Style = src.m_Style;

	CreateResources();
}
