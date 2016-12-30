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
// BCGPControlRenderer.cpp: implementation of the CBCGPControlRenderer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPControlRenderer.h"
#include "BCGPDrawManager.h"
#include "BCGGlobals.h"
#include "BCGPImageProcessing.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CBCGPControlRendererParams::CBCGPControlRendererParams ()
{
	CommonInit ();
}

CBCGPControlRendererParams::~CBCGPControlRendererParams ()
{
}

CBCGPControlRendererParams::CBCGPControlRendererParams (UINT uiBmpResID,
				const CRect& rtImage, 
				const CRect& rtCorners,
				const CRect& rtSides/* = CRect (0, 0, 0, 0)*/,
                const CRect& rtInner/* = CRect (0, 0, 0, 0)*/)
{
	CommonInit ();

	SetResourceID (MAKEINTRESOURCE (uiBmpResID));

	m_rectImage      = rtImage;
	m_rectCorners	 = rtCorners;
	m_rectSides		 = rtSides;
	m_rectInter      = rtInner;
}

CBCGPControlRendererParams::CBCGPControlRendererParams (LPCTSTR lpszBmpResID,
				const CRect& rtImage, 
				const CRect& rtCorners,
				const CRect& rtSides/* = CRect (0, 0, 0, 0)*/,
                const CRect& rtInner/* = CRect (0, 0, 0, 0)*/)
{
	CommonInit ();

	SetResourceID (lpszBmpResID);

	m_rectImage      = rtImage;
	m_rectCorners	 = rtCorners;
	m_rectSides		 = rtSides;
	m_rectInter      = rtInner;
}

CBCGPControlRendererParams::CBCGPControlRendererParams (UINT uiBmpResID,
				COLORREF clrTransparent,
				const CRect& rtImage, 
				const CRect& rtCorners,
				const CRect& rtSides/* = CRect (0, 0, 0, 0)*/,
                const CRect& rtInner/* = CRect (0, 0, 0, 0)*/,
				BOOL bPreMultiplyCheck/* = TRUE*/)
{
	CommonInit ();

	SetResourceID (MAKEINTRESOURCE (uiBmpResID));

	m_rectImage         = rtImage;
	m_rectCorners	    = rtCorners;
	m_rectSides		    = rtSides;
	m_rectInter         = rtInner;
	m_clrTransparent    = clrTransparent;
	m_bPreMultiplyCheck = bPreMultiplyCheck;
}

CBCGPControlRendererParams::CBCGPControlRendererParams (HBITMAP hBitmap,
				COLORREF clrTransparent,
				const CRect& rtImage, 
				const CRect& rtCorners,
				const CRect& rtSides/* = CRect (0, 0, 0, 0)*/,
				const CRect& rtInner/* = CRect (0, 0, 0, 0)*/,
				BOOL bPreMultiplyCheck/* = TRUE*/)
{
	CommonInit ();
	
	m_hBitmap			= hBitmap;
	m_rectImage         = rtImage;
	m_rectCorners	    = rtCorners;
	m_rectSides		    = rtSides;
	m_rectInter         = rtInner;
	m_clrTransparent    = clrTransparent;
	m_bPreMultiplyCheck = bPreMultiplyCheck;
}

CBCGPControlRendererParams::CBCGPControlRendererParams (LPCTSTR lpszBmpResID,
				COLORREF clrTransparent,
				const CRect& rtImage, 
				const CRect& rtCorners,
				const CRect& rtSides/* = CRect (0, 0, 0, 0)*/,
                const CRect& rtInner/* = CRect (0, 0, 0, 0)*/,
				BOOL bPreMultiplyCheck/* = TRUE*/)
{
	CommonInit ();

	SetResourceID (lpszBmpResID);

	m_rectImage         = rtImage;
	m_rectCorners	    = rtCorners;
	m_rectSides		    = rtSides;
	m_rectInter         = rtInner;
	m_clrTransparent    = clrTransparent;
	m_bPreMultiplyCheck = bPreMultiplyCheck;
}

CBCGPControlRendererParams::CBCGPControlRendererParams (const CBCGPControlRendererParams& rSrc)
{
	CommonInit ();

	(*this) = rSrc;
}

void CBCGPControlRendererParams::CommonInit ()
{
	m_uiBmpResID   = 0;
	m_hBitmap = NULL;
    m_strBmpResID.Empty();
	m_rectImage.SetRectEmpty ();
	m_rectCorners.SetRectEmpty ();
	m_rectSides.SetRectEmpty ();
	m_rectInter.SetRectEmpty ();
	m_clrTransparent    = CLR_DEFAULT;
	m_bPreMultiplyCheck = TRUE;
	m_bMapTo3DColors    = FALSE;
	m_clrBase = (COLORREF)-1;
	m_clrTarget	= (COLORREF)-1;
}

HBITMAP CBCGPControlRendererParams::GetBitmap() const
{
	return m_hBitmap;
}

LPCTSTR CBCGPControlRendererParams::GetResourceID () const
{
	if (m_strBmpResID.IsEmpty ())
	{
		return MAKEINTRESOURCE (m_uiBmpResID);
	}
	
	return m_strBmpResID;
}

void CBCGPControlRendererParams::SetResourceID (LPCTSTR lpszBmpResID)
{
	if (IS_INTRESOURCE(lpszBmpResID))
	{
		m_uiBmpResID = (UINT)((UINT_PTR)(lpszBmpResID));
	}
	else
	{
		m_strBmpResID = lpszBmpResID;
	}
}

void CBCGPControlRendererParams::SetBaseColor (COLORREF clrBase, COLORREF clrTarget)
{
	m_clrBase = clrBase;
	m_clrTarget = clrTarget;
}

CBCGPControlRendererParams& CBCGPControlRendererParams::operator = (const CBCGPControlRendererParams& rSrc)
{
	m_uiBmpResID	    = rSrc.m_uiBmpResID;
	m_strBmpResID       = rSrc.m_strBmpResID;
	m_hBitmap			= rSrc.m_hBitmap;
	m_rectImage         = rSrc.m_rectImage;
	m_rectCorners	    = rSrc.m_rectCorners;
	m_rectSides		    = rSrc.m_rectSides;
	m_rectInter         = rSrc.m_rectInter;
	m_clrTransparent    = rSrc.m_clrTransparent;
	m_bPreMultiplyCheck = rSrc.m_bPreMultiplyCheck;
	m_clrBase			= rSrc.m_clrBase;
	m_clrTarget			= rSrc.m_clrTarget;

	return *this;
}

static void RotateRect(CRect& rect, BOOL bCW)
{
	if (bCW)
	{
		rect = CRect(rect.bottom, rect.left, rect.top, rect.right);
	}
	else
	{
		rect = CRect(rect.top, rect.right, rect.bottom, rect.left);
	}
}

void CBCGPControlRendererParams::Rotate (BOOL bCW)
{
	RotateRect(m_rectImage, bCW);
	m_rectImage.NormalizeRect();
	RotateRect(m_rectCorners, bCW);
	RotateRect(m_rectSides, bCW);
	RotateRect(m_rectInter, bCW);
	m_rectInter.NormalizeRect();
}


IMPLEMENT_DYNCREATE(CBCGPControlRenderer, CObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPControlRenderer::CBCGPControlRenderer()
{
	m_bMirror = FALSE;
	m_bIsScaled = FALSE;
}
//*********************************************************************************
CBCGPControlRenderer::~CBCGPControlRenderer()
{
	CleanUp ();
}

//////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////

BOOL CBCGPControlRenderer::Create (const CBCGPControlRendererParams& params, BOOL bFlipvert /*= FALSE*/)
{
	CleanUp ();

	m_Params = params;

	LPCTSTR lpszResID = m_Params.GetResourceID ();
	HBITMAP hBitmap = m_Params.GetBitmap();

	if (lpszResID != NULL || hBitmap != NULL)
	{
		m_Bitmap.SetImageSize (m_Params.m_rectImage.Size ());
		m_Bitmap.SetPreMultiplyAutoCheck (m_Params.m_bPreMultiplyCheck);
		m_Bitmap.SetMapTo3DColors (m_Params.m_bMapTo3DColors);

		if (hBitmap == NULL)
		{
			m_Bitmap.LoadStr (lpszResID);
		}
		else
		{
			m_Bitmap.AddImage(hBitmap, TRUE);
		}

		if (bFlipvert)
		{
			m_Bitmap.MirrorVert ();
		}

		if (m_Params.m_clrTransparent != CLR_DEFAULT)
		{
			m_Bitmap.SetTransparentColor (m_Params.m_clrTransparent);
		}

		if (m_Params.m_clrBase != (COLORREF)-1 &&
			m_Params.m_clrTarget != (COLORREF)-1)
		{
			m_Bitmap.AddaptColors (m_Params.m_clrBase, m_Params.m_clrTarget);
		}

        if (CBCGPToolBarImages::IsRTL () && m_Bitmap.GetImageWell () != NULL &&
			m_Params.m_clrTransparent == CLR_DEFAULT)
        {
            BITMAP bmp;
            if (::GetObject (m_Bitmap.GetImageWell (), sizeof (BITMAP), &bmp) != 0)
            {
                if (bmp.bmBitsPixel == 32)
                {
					Mirror ();
				}
			}
        }

		if (m_Params.m_rectSides.IsRectNull ())
		{
			m_Params.m_rectSides = m_Params.m_rectCorners;
		}

		if (m_Params.m_rectInter.IsRectNull ())
		{
			m_Params.m_rectInter = CRect (CPoint (0, 0), m_Params.m_rectImage.Size ());
			m_Params.m_rectInter.left   += m_Params.m_rectCorners.left;
			m_Params.m_rectInter.top    += m_Params.m_rectCorners.top;
			m_Params.m_rectInter.right  -= m_Params.m_rectCorners.right;
			m_Params.m_rectInter.bottom -= m_Params.m_rectCorners.bottom;
		}

		if (bFlipvert)
		{
			long temp;
			temp = m_Params.m_rectCorners.top;
			m_Params.m_rectCorners.top = m_Params.m_rectCorners.bottom;
			m_Params.m_rectCorners.bottom = temp;

			temp = m_Params.m_rectSides.top;
			m_Params.m_rectSides.top = m_Params.m_rectSides.bottom;
			m_Params.m_rectSides.bottom = temp;

			long height = m_Params.m_rectImage.Height ();
			temp = m_Params.m_rectInter.top;
			m_Params.m_rectInter.top = height - m_Params.m_rectInter.bottom;
			m_Params.m_rectInter.bottom = height - temp;
		}
	}

	return TRUE;
}
//*********************************************************************************
void CBCGPControlRenderer::Mirror ()
{
	if (m_Bitmap.Mirror ())
	{
		m_bMirror = !m_bMirror;
	}
}
//*********************************************************************************
void CBCGPControlRenderer::Rotate (BOOL bCW)
{
	HBITMAP hBitmap = m_Bitmap.GetImageWell();
	if (hBitmap == NULL)
	{
		return;
	}

	HBITMAP hBitmapR = BCGPRotateBitmap(hBitmap, bCW);
	if (hBitmapR == NULL)
	{
		return;
	}

	m_Params.Rotate (bCW);

	m_Bitmap.Clear();

	m_Bitmap.SetImageSize (m_Params.m_rectImage.Size ());
	m_Bitmap.SetPreMultiplyAutoCheck (m_Params.m_bPreMultiplyCheck);
	m_Bitmap.SetMapTo3DColors (m_Params.m_bMapTo3DColors);

	if (m_Params.m_clrTransparent != CLR_DEFAULT)
	{
		m_Bitmap.SetTransparentColor (m_Params.m_clrTransparent);
	}

	m_Bitmap.AddImage(hBitmapR, TRUE);
	::DeleteObject(hBitmapR);
}
//*********************************************************************************
void CBCGPControlRenderer::CleanUp ()
{
	m_Bitmap.Clear ();
	m_Bitmap.SetTransparentColor ((COLORREF)(-1));

	CBCGPControlRendererParams emptyParams;
	m_Params = emptyParams;
    m_bMirror = FALSE;
}
//*********************************************************************************
void CBCGPControlRenderer::Draw (CDC* pDC, CRect rect, UINT index, BYTE alphaSrc/* = 255*/)
{
	CRect rectInter (rect);
	rectInter.left   += m_Params.m_rectSides.left;
	rectInter.top    += m_Params.m_rectSides.top;
	rectInter.right  -= m_Params.m_rectSides.right;
	rectInter.bottom -= m_Params.m_rectSides.bottom;

	FillInterior (pDC, rectInter, index, alphaSrc);

	DrawFrame (pDC, rect, index, alphaSrc);
}
//*********************************************************************************
void CBCGPControlRenderer::DrawFrame (CDC* pDC, CRect rect, UINT index, BYTE alphaSrc/* = 255*/)
{
	struct XHVTypes
	{
		CBCGPToolBarImages::ImageAlignHorz horz;
		CBCGPToolBarImages::ImageAlignVert vert;
	};

	XHVTypes corners[4] = 
	{
		{CBCGPToolBarImages::ImageAlignHorzLeft , CBCGPToolBarImages::ImageAlignVertTop},
		{CBCGPToolBarImages::ImageAlignHorzRight, CBCGPToolBarImages::ImageAlignVertTop},
		{CBCGPToolBarImages::ImageAlignHorzLeft , CBCGPToolBarImages::ImageAlignVertBottom},
		{CBCGPToolBarImages::ImageAlignHorzRight, CBCGPToolBarImages::ImageAlignVertBottom}
	};

	XHVTypes sides[4] = 
	{
		{CBCGPToolBarImages::ImageAlignHorzLeft   , CBCGPToolBarImages::ImageAlignVertStretch},
		{CBCGPToolBarImages::ImageAlignHorzRight  , CBCGPToolBarImages::ImageAlignVertStretch},
		{CBCGPToolBarImages::ImageAlignHorzStretch, CBCGPToolBarImages::ImageAlignVertTop},
		{CBCGPToolBarImages::ImageAlignHorzStretch, CBCGPToolBarImages::ImageAlignVertBottom}
	};

	CRect rectImage (m_Params.m_rectImage);
	int ind = index;
	if (m_Bitmap.GetCount () == 1)
	{
		rectImage.OffsetRect (0, m_Params.m_rectImage.Size ().cy * ind);
		ind = 0;
	}

	CRect rt  (rect);
	CRect rectCorners (m_Params.m_rectCorners);
	CRect rectSides   (m_Params.m_rectSides);

	rt.left   += rectCorners.left;
	rt.top    += rectCorners.top;
	rt.right  -= rectCorners.right;
	rt.bottom -= rectCorners.bottom;

	if (rt.Width () > 0 || rt.Height () > 0)
	{
		if (rt.Height () > 0)
		{
			if (rectSides.left > 0)
			{
				CRect r (rt);
				r.left  = rect.left;
				r.right = r.left + rectSides.left;

                CRect rectPart;
                if (m_bMirror)
                {
				    rectPart = CRect (rectImage.right - rectSides.left, 
					    rectImage.top + rectCorners.top, rectImage.right, rectImage.bottom - rectCorners.bottom);
                }
                else
                {
				    rectPart = CRect (rectImage.left, 
					    rectImage.top + rectCorners.top, rectImage.left + rectSides.left, rectImage.bottom - rectCorners.bottom);
                }

				m_Bitmap.DrawEx (pDC, r, ind, sides[0].horz, sides[0].vert, rectPart, alphaSrc);
			}

			if (rectSides.right > 0)
			{
				CRect r (rt);
				r.right = rect.right;
				r.left  = r.right - rectSides.right;

				CRect rectPart;
                if (m_bMirror)
                {
                    rectPart = CRect (rectImage.left, 
					    rectImage.top + rectCorners.top, rectImage.left + rectSides.right, rectImage.bottom - rectCorners.bottom);
                }
                else
                {
                    rectPart = CRect (rectImage.right - rectSides.right, 
    					rectImage.top + rectCorners.top, rectImage.right, rectImage.bottom - rectCorners.bottom);
                }

				m_Bitmap.DrawEx (pDC, r, ind, sides[1].horz, sides[1].vert, rectPart, alphaSrc);
			}
		}

		if (rt.Width () > 0)
		{
			if (rectSides.top > 0)
			{
				CRect r (rt);
				r.top    = rect.top;
				r.bottom = r.top + rectSides.top;

				CRect rectPart;
                if (m_bMirror)
                {
                    rectPart = CRect (rectImage.left + rectCorners.right, 
					    rectImage.top, rectImage.right - rectCorners.left, rectImage.top + rectSides.top);
                }
                else
                {
                    rectPart = CRect (rectImage.left + rectCorners.left, 
					    rectImage.top, rectImage.right - rectCorners.right, rectImage.top + rectSides.top);
                }

				m_Bitmap.DrawEx (pDC, r, ind, sides[2].horz, sides[2].vert, rectPart, alphaSrc);
			}

			if (rectSides.bottom > 0)
			{
				CRect r (rt);
				r.bottom = rect.bottom;
				r.top    = r.bottom - rectSides.bottom;
				
				CRect rectPart;
                if (m_bMirror)
                {
                    rectPart = CRect (rectImage.left + rectCorners.right, 
    					rectImage.bottom - rectSides.bottom, rectImage.right - rectCorners.left, rectImage.bottom);
                }
                else
                {
                    rectPart = CRect (rectImage.left + rectCorners.left, 
    					rectImage.bottom - rectSides.bottom, rectImage.right - rectCorners.right, rectImage.bottom);
                }

				m_Bitmap.DrawEx (pDC, r, ind, sides[3].horz, sides[3].vert, rectPart, alphaSrc);
			}
		}

		if (rectCorners.left > 0 && rectCorners.top > 0)
		{
			CRect rectPart;
            if (m_bMirror)
            {
                rectPart = CRect (CPoint (rectImage.right - rectCorners.left, rectImage.top), 
					CSize (rectCorners.left, rectCorners.top));
            }
            else
            {
                rectPart = CRect (CPoint (rectImage.left, rectImage.top), 
					CSize (rectCorners.left, rectCorners.top));
            }

			m_Bitmap.DrawEx (pDC, rect, ind, corners[0].horz, corners[0].vert, rectPart, alphaSrc);
		}

		if (rectCorners.right > 0 && rectCorners.top > 0)
		{
			CRect rectPart;
            if (m_bMirror)
            {
                rectPart = CRect (CPoint (rectImage.left, rectImage.top), 
					CSize (rectCorners.right, rectCorners.top));
            }
            else
            {
                rectPart = CRect (CPoint (rectImage.right - rectCorners.right, rectImage.top), 
					CSize (rectCorners.right, rectCorners.top));
            }

			m_Bitmap.DrawEx (pDC, rect, ind, corners[1].horz, corners[1].vert, rectPart, alphaSrc);
		}

		if (rectCorners.left > 0 && rectCorners.bottom > 0)
		{
			CRect rectPart;
            if (m_bMirror)
            {
                rectPart = CRect (CPoint (rectImage.right - rectCorners.left, rectImage.bottom - rectCorners.bottom), 
					CSize (rectCorners.left, rectCorners.bottom));
            }
            else
            {
                rectPart = CRect (CPoint (rectImage.left, rectImage.bottom - rectCorners.bottom), 
					CSize (rectCorners.left, rectCorners.bottom));
            }

			m_Bitmap.DrawEx (pDC, rect, ind, corners[2].horz, corners[2].vert, rectPart, alphaSrc);
		}

		if (rectCorners.right > 0 && rectCorners.bottom > 0)
		{
			CRect rectPart;
            if (m_bMirror)
            {
                rectPart = CRect (CPoint (rectImage.left, rectImage.bottom - rectCorners.bottom), 
					CSize (rectCorners.right, rectCorners.bottom));
            }
            else
            {
                rectPart = CRect (CPoint (rectImage.right - rectCorners.right, rectImage.bottom - rectCorners.bottom), 
					CSize (rectCorners.right, rectCorners.bottom));
            }

			m_Bitmap.DrawEx (pDC, rect, ind, corners[3].horz, corners[3].vert, rectPart, alphaSrc);
		}
	}
}
//*********************************************************************************
void CBCGPControlRenderer::FillInterior (CDC* pDC, CRect rect, CBCGPToolBarImages::ImageAlignHorz horz,
		CBCGPToolBarImages::ImageAlignVert vert, UINT index, BYTE alphaSrc/* = 255*/)
{
	if (m_Params.m_rectInter.IsRectEmpty ())
	{
		return;
	}

	CRect rectImage (m_Params.m_rectInter);

	if (m_bMirror)
	{
		rectImage.left  = m_Params.m_rectImage.Size ().cx - m_Params.m_rectInter.right;
		rectImage.right = rectImage.left + m_Params.m_rectInter.Width ();
	}

	rectImage.OffsetRect (m_Params.m_rectImage.TopLeft ());

	int ind = index;
	if (m_Bitmap.GetCount () == 1)
	{
		rectImage.OffsetRect (0, m_Params.m_rectImage.Size ().cy * ind);
		ind = 0;
	}

	m_Bitmap.DrawEx (pDC, rect, ind, horz, vert, rectImage, alphaSrc);
}
//*********************************************************************************
void CBCGPControlRenderer::FillInterior (CDC* pDC, CRect rect, UINT index, BYTE alphaSrc/* = 255*/)
{
	FillInterior (pDC, rect, CBCGPToolBarImages::ImageAlignHorzStretch,
		CBCGPToolBarImages::ImageAlignVertStretch, index, alphaSrc);
}
//*********************************************************************************
void CBCGPControlRenderer::OnSysColorChange ()
{
	if (m_Bitmap.GetImageWell () != NULL)
	{
		m_Bitmap.OnSysColorChange ();
	}
}
//*********************************************************************************
BOOL CBCGPControlRenderer::CopyTo (CBCGPControlRenderer& dest)
{
	if (this == &dest)
	{
		return FALSE;
	}

	dest.CleanUp ();

	if (m_Bitmap.CopyTo (dest.m_Bitmap))
	{
		dest.m_Params  = m_Params;
		dest.m_bMirror = m_bMirror;

		return TRUE;
	}

	return FALSE;
}
//*********************************************************************************

static void ResizeRect (CRect& rect, double dblScale)
{
	int nWidth = rect.Width ();
	int nHeight = rect.Height ();

	rect.left = (int) (.5 + dblScale * rect.left);
	rect.top = (int) (.5 + dblScale * rect.top);

	rect.right = rect.left + (int) (.5 + dblScale * nWidth);
	rect.bottom = rect.top + (int) (.5 + dblScale * nHeight);
}

BOOL CBCGPControlRenderer::SmoothResize (double dblScale)
{
	if (dblScale <= 1. || !m_Bitmap.SmoothResize (dblScale))
	{
		return FALSE;
	}

	ResizeRect (m_Params.m_rectImage, dblScale);
	ResizeRect (m_Params.m_rectInter, dblScale);

	m_bIsScaled = TRUE;
	return TRUE;
}


IMPLEMENT_DYNCREATE(CBCGPShadowRenderer, CBCGPControlRenderer)

CBCGPShadowRenderer::CBCGPShadowRenderer()
	: m_nDepth        (0)
	, m_clrBase       (0)
	, m_iMinBrightness(0)
	, m_iMaxBrightness(100)
{
}
//*********************************************************************************
CBCGPShadowRenderer::~CBCGPShadowRenderer()
{
}
//*********************************************************************************
BOOL CBCGPShadowRenderer::Create (const CBCGPControlRendererParams& /*params*/, BOOL /*bFlipvert*/ /*= FALSE*/)
{
	return FALSE;
}
//*********************************************************************************
BOOL CBCGPShadowRenderer::Create (int nDepth,
								  COLORREF clrBase,
                                  int iMinBrightness/* = 0*/,
								  int iMaxBrightness/* = 100*/,
								  BOOL bCanMirror/* = TRUE*/,
								  double dblSmooth/* = 1.0*/,
								  double dblDarkRatio/* = 0.25*/)
{
	if (IsValid() &&
		m_nDepth == nDepth && m_clrBase == clrBase && 
		m_iMinBrightness == iMinBrightness && m_iMaxBrightness == iMaxBrightness)
	{
		return TRUE;
	}

	CleanUp ();

	if (nDepth == 0 || iMaxBrightness == 0)
	{
		return FALSE;
	}

	m_nDepth         = nDepth;
	m_clrBase        = clrBase;
	m_iMinBrightness = iMinBrightness;
	m_iMaxBrightness = iMaxBrightness;

	if (m_clrBase == (COLORREF)-1)
	{
		clrBase = globalData.clrBarFace;
	}

	HBITMAP hBitmap = CBCGPDrawManager::PrepareShadowMask (nDepth, clrBase, iMinBrightness, iMaxBrightness, dblSmooth, dblDarkRatio);
	if (hBitmap == NULL)
	{
		return FALSE;
	}

	int nSize     = nDepth < 2 ? 2 : nDepth;
	int nDestSize = nSize * 2 + 1;

	m_Params.m_rectImage   = CRect (0, 0, nDestSize, nDestSize);
	m_Params.m_rectCorners = CRect (nSize, nSize, nSize, nSize);
	m_Params.m_rectSides = m_Params.m_rectCorners;

	m_Params.m_rectInter = CRect (CPoint (0, 0), m_Params.m_rectImage.Size ());
	m_Params.m_rectInter.left   += m_Params.m_rectCorners.left;
	m_Params.m_rectInter.top    += m_Params.m_rectCorners.top;
	m_Params.m_rectInter.right  -= m_Params.m_rectCorners.right;
	m_Params.m_rectInter.bottom -= m_Params.m_rectCorners.bottom;

	m_Bitmap.SetImageSize (m_Params.m_rectImage.Size ());
	m_Bitmap.SetPreMultiplyAutoCheck (m_Params.m_bPreMultiplyCheck);
	m_Bitmap.SetMapTo3DColors (m_Params.m_bMapTo3DColors);

	m_Bitmap.AddImage (hBitmap, TRUE);

	::DeleteObject (hBitmap);

    if (bCanMirror && CBCGPToolBarImages::IsRTL () && m_Bitmap.GetImageWell () != NULL)
    {
		Mirror ();
    }

	return m_Bitmap.GetCount () == 1;
}
//*********************************************************************************
void CBCGPShadowRenderer::OnSysColorChange ()
{
}
//*********************************************************************************
void CBCGPShadowRenderer::Draw (CDC* pDC, CRect rect, UINT index/* = 0*/, BYTE alphaSrc/* = 255*/)
{
	if (128 <= alphaSrc)
	{
		CRect rectInter (rect);
		
		if (CBCGPToolBarImages::IsRTL ())
		{
			rectInter.left   += m_Params.m_rectSides.left;
			rectInter.right   = rectInter.left + m_Params.m_rectSides.left;
		}
		else
		{
			rectInter.right  -= m_Params.m_rectSides.right;
			rectInter.left    = rectInter.right - m_Params.m_rectSides.right;
		}

		rectInter.bottom -= m_Params.m_rectSides.bottom;
		rectInter.top     = rectInter.bottom - m_Params.m_rectSides.bottom;

		FillInterior (pDC, rectInter, index, alphaSrc);
	}

	DrawFrame (pDC, rect, index, alphaSrc);
}
//*********************************************************************************
void CBCGPShadowRenderer::DrawFrame (CDC* pDC, CRect rect, UINT index/* = 0*/, BYTE alphaSrc/* = 255*/)
{
	struct XHVTypes
	{
		CBCGPToolBarImages::ImageAlignHorz horz;
		CBCGPToolBarImages::ImageAlignVert vert;
	};

	XHVTypes corners[4] = 
	{
		{CBCGPToolBarImages::ImageAlignHorzLeft , CBCGPToolBarImages::ImageAlignVertTop},
		{CBCGPToolBarImages::ImageAlignHorzRight, CBCGPToolBarImages::ImageAlignVertTop},
		{CBCGPToolBarImages::ImageAlignHorzLeft , CBCGPToolBarImages::ImageAlignVertBottom},
		{CBCGPToolBarImages::ImageAlignHorzRight, CBCGPToolBarImages::ImageAlignVertBottom}
	};

	XHVTypes sides[4] = 
	{
		{CBCGPToolBarImages::ImageAlignHorzLeft   , CBCGPToolBarImages::ImageAlignVertStretch},
		{CBCGPToolBarImages::ImageAlignHorzRight  , CBCGPToolBarImages::ImageAlignVertStretch},
		{CBCGPToolBarImages::ImageAlignHorzStretch, CBCGPToolBarImages::ImageAlignVertTop},
		{CBCGPToolBarImages::ImageAlignHorzStretch, CBCGPToolBarImages::ImageAlignVertBottom}
	};

	CRect rectImage (m_Params.m_rectImage);
	int ind = index;
	if (m_Bitmap.GetCount () == 1)
	{
		rectImage.OffsetRect (0, m_Params.m_rectImage.Size ().cy * ind);
		ind = 0;
	}

	CRect rt  (rect);
	CRect rectCorners (m_Params.m_rectCorners);
	CRect rectSides   (m_Params.m_rectSides);

	rt.left   += rectCorners.left;
	rt.top    += rectCorners.top;
	rt.right  -= rectCorners.right;
	rt.bottom -= rectCorners.bottom;

	BOOL bRTL = CBCGPToolBarImages::IsRTL ();

	if (rt.Width () > 0 || rt.Height () > 0)
	{
		if (rt.Height () > 0)
		{
			if (bRTL && !m_bMirror)
			{
				if (rectSides.left > 0)
				{
					CRect r (rt);
					r.left  = rect.left;
					r.right = r.left + rectSides.left;

					CRect rectPart (rectImage.left, 
							rectImage.top + rectCorners.top, rectImage.left + rectSides.left, rectImage.bottom - rectCorners.bottom);

					m_Bitmap.DrawEx (pDC, r, ind, sides[0].horz, sides[0].vert, rectPart, alphaSrc);
				}
			}
			else
			{
				if (rectSides.right > 0)
				{
					CRect r (rt);
					r.right = rect.right;
					r.left  = r.right - rectSides.right;

					CRect rectPart;
					if (m_bMirror)
					{
						rectPart = CRect (rectImage.left, 
							rectImage.top + rectCorners.top, rectImage.left + rectSides.right, rectImage.bottom - rectCorners.bottom);
					}
					else
					{
						rectPart = CRect (rectImage.right - rectSides.right, 
    						rectImage.top + rectCorners.top, rectImage.right, rectImage.bottom - rectCorners.bottom);
					}

					m_Bitmap.DrawEx (pDC, r, ind, sides[1].horz, sides[1].vert, rectPart, alphaSrc);
				}
			}	
		}

		if (rt.Width () > 0)
		{
			if (rectSides.bottom > 0)
			{
				CRect r (rt);
				r.bottom = rect.bottom;
				r.top    = r.bottom - rectSides.bottom;
				
				CRect rectPart  (rectImage.left + rectCorners.left, 
    					rectImage.bottom - rectSides.bottom, rectImage.right - rectCorners.right, rectImage.bottom);

				m_Bitmap.DrawEx (pDC, r, ind, sides[3].horz, sides[3].vert, rectPart, alphaSrc);
			}
		}

		if (bRTL && !m_bMirror)
		{
			if (rectCorners.left > 0 && rectCorners.top > 0)
			{
				CRect rectPart (CPoint (rectImage.left, rectImage.top), 
						CSize (rectCorners.left, rectCorners.top));

				m_Bitmap.DrawEx (pDC, rect, ind, corners[0].horz, corners[0].vert, rectPart, alphaSrc);
			}

			if (rectCorners.left > 0 && rectCorners.bottom > 0)
			{
				CRect rectPart = CRect (CPoint (rectImage.left, rectImage.bottom - rectCorners.bottom), 
						CSize (rectCorners.left, rectCorners.bottom));
				
				m_Bitmap.DrawEx (pDC, rect, ind, corners[2].horz, corners[2].vert, rectPart, alphaSrc);
			}

			if (rectCorners.right > 0 && rectCorners.bottom > 0)
			{
				CRect rectPart (CPoint (rectImage.right - rectCorners.right, rectImage.bottom - rectCorners.bottom), 
						CSize (rectCorners.right, rectCorners.bottom));

				m_Bitmap.DrawEx (pDC, rect, ind, corners[3].horz, corners[3].vert, rectPart, alphaSrc);
			}
		}
		else
		{
			if (rectCorners.right > 0 && rectCorners.top > 0)
			{
				CRect rectPart;
				if (m_bMirror)
				{
					rectPart = CRect (CPoint (rectImage.left, rectImage.top), 
						CSize (rectCorners.right, rectCorners.top));
				}
				else
				{
					rectPart = CRect (CPoint (rectImage.right - rectCorners.right, rectImage.top), 
						CSize (rectCorners.right, rectCorners.top));
				}

				m_Bitmap.DrawEx (pDC, rect, ind, corners[1].horz, corners[1].vert, rectPart, alphaSrc);
			}

			if (rectCorners.left > 0 && rectCorners.bottom > 0)
			{
				CRect rectPart;
				if (m_bMirror)
				{
					rectPart = CRect (CPoint (rectImage.right - rectCorners.left, rectImage.bottom - rectCorners.bottom), 
						CSize (rectCorners.left, rectCorners.bottom));
				}
				else
				{
					rectPart = CRect (CPoint (rectImage.left, rectImage.bottom - rectCorners.bottom), 
						CSize (rectCorners.left, rectCorners.bottom));
				}

				m_Bitmap.DrawEx (pDC, rect, ind, corners[2].horz, corners[2].vert, rectPart, alphaSrc);
			}

			if (rectCorners.right > 0 && rectCorners.bottom > 0)
			{
				CRect rectPart;
				if (m_bMirror)
				{
					rectPart = CRect (CPoint (rectImage.left, rectImage.bottom - rectCorners.bottom), 
						CSize (rectCorners.right, rectCorners.bottom));
				}
				else
				{
					rectPart = CRect (CPoint (rectImage.right - rectCorners.right, rectImage.bottom - rectCorners.bottom), 
						CSize (rectCorners.right, rectCorners.bottom));
				}

				m_Bitmap.DrawEx (pDC, rect, ind, corners[3].horz, corners[3].vert, rectPart, alphaSrc);
			}
		}
	}
}
//*********************************************************************************
BOOL CBCGPShadowRenderer::CopyTo (CBCGPControlRenderer& dest)
{
	if (this == &dest)
	{
		return FALSE;
	}

	if (CBCGPControlRenderer::CopyTo (dest))
	{
		if (dest.IsKindOf (RUNTIME_CLASS (CBCGPShadowRenderer)))
		{
			CBCGPShadowRenderer& rDest = (CBCGPShadowRenderer&)dest;
			rDest.m_nDepth = m_nDepth;
			rDest.m_clrBase = m_clrBase;
			rDest.m_iMinBrightness = m_iMinBrightness;
			rDest.m_iMaxBrightness = m_iMaxBrightness;

			return TRUE;
		}
	}

	return FALSE;
}
