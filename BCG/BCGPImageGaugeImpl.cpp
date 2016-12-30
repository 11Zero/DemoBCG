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
// BCGPImageGaugeImpl.cpp: implementation of the CBCGPImageGaugeImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPImageGaugeImpl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CBCGPImageGaugeImpl, CBCGPStaticGaugeImpl)
IMPLEMENT_DYNAMIC(CBCGPImageGaugeCtrl, CBCGPVisualCtrl)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPImageGaugeImpl::CBCGPImageGaugeImpl(UINT uiImageResID, CBCGPVisualContainer* pContainer) :
	CBCGPStaticGaugeImpl(pContainer)
{
	m_VerticalAlign = VA_Top;
	m_HorizontalAlign = HA_Left;
	m_bLockAspectRatio = TRUE;

	m_DefaultDrawFlags = BCGP_DRAW_DYNAMIC;

	SetImage(uiImageResID);
}
//*******************************************************************************
CBCGPImageGaugeImpl::~CBCGPImageGaugeImpl()
{
}
//*******************************************************************************
void CBCGPImageGaugeImpl::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectClip*/, DWORD dwFlags)
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

	CBCGPRect rect = m_rect;
	if (m_bIsSubGauge)
	{
		rect.OffsetRect(-m_ptScrollOffset);
	}

	pGM->FillRectangle(rect, GetFillBrush ());

	CBCGPRect rectSrc = CBCGPRect(CBCGPPoint(0.0, 0.0), m_Image.GetSize (pGM));
	if (!m_bOff && !rectSrc.IsRectEmpty ())
	{
		if (m_sizeScaleRatio != CBCGPSize(1.0, 1.0))
		{
			rectSrc.Scale(m_sizeScaleRatio.cx, m_sizeScaleRatio.cy);
		}

		CBCGPRect rectDst = rect;
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
			rectDst.IntersectRect (rectDst, rect);

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
			rectDst.IntersectRect (rectDst, rect);

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
			pGM->DrawImage(m_Image, rectDst.TopLeft(), rectDst.Size(), GetOpacity(), CBCGPImage::BCGP_IMAGE_INTERPOLATION_MODE_LINEAR, rectSrc);
		}
		else
		{
			pGM->DrawImage(m_Image, rectDst.TopLeft(), CBCGPSize(), GetOpacity());
		}
	}

	pGM->DrawRectangle(rect, GetOutlineBrush (), GetScaleRatioMid());

	SetDirty(FALSE);
}
//*******************************************************************************
void CBCGPImageGaugeImpl::SetImage(UINT uiImageResID, BOOL bRedraw)
{
	m_Image.Destroy();
	m_Image = CBCGPImage(uiImageResID);

	SetDirty();

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPImageGaugeImpl::SetImage (const CBCGPImage& image, BOOL bRedraw)
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
void CBCGPImageGaugeImpl::SetImageAlign (HorizontalAlign horzAlign, VerticalAlign vertAlign, BOOL bLockAspectRatio, BOOL bRedraw)
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
CBCGPSize CBCGPImageGaugeImpl::GetDefaultSize(CBCGPGraphicsManager* pGM, const CBCGPBaseVisualObject* /*pParentGauge*/)
{
	ASSERT_VALID(pGM);

	CBCGPSize size(m_Image.GetSize(pGM));
	size.cx *= m_sizeScaleRatio.cx;
	size.cy *= m_sizeScaleRatio.cy;

	return size;
}
//*******************************************************************************
void CBCGPImageGaugeImpl::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPStaticGaugeImpl::CopyFrom(srcObj);

	const CBCGPImageGaugeImpl& src = (const CBCGPImageGaugeImpl&)srcObj;

	m_Image = src.m_Image;
	m_VerticalAlign = src.m_VerticalAlign;
	m_HorizontalAlign = src.m_HorizontalAlign;
	m_bLockAspectRatio = src.m_bLockAspectRatio;
}
