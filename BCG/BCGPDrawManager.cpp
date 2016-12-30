// *******************************************************************************
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
// BCGDrawManager.cpp: implementation of the CBCGPDrawManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGCBPro.h"
#include "BCGGlobals.h"
#include "BCGPDrawManager.h"
#include "BCGPMath.h"
#include "BCGPImageProcessing.h"

#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE) && !(defined _BCGSUITE_) && !(defined _BCGPCHART_STANDALONE)
	#include "BCGPControlRenderer.h"
	#include "BCGPToolBarImages.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define CLR_TO_RGBA(c) c | 0xFF000000
#define RGB_TO_RGBA(r, g, b) CLR_TO_RGBA(RGB(r, g, b))
#define _RGBA(r, g, b, a) RGB(r, g, b) | (a << 24)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HBITMAP CBCGPDrawManager::CreateBitmap_32 (const CSize& size, void** pBits)
{
	if (pBits != NULL)
	{
		*pBits = NULL;
	}

	if (size.cx <= 0 || size.cy == 0)
	{
		return NULL;
	}

	BITMAPINFO bi = {0};

	// Fill in the BITMAPINFOHEADER
	bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth       = size.cx;
	bi.bmiHeader.biHeight      = size.cy;
	bi.bmiHeader.biSizeImage   = size.cx * abs(size.cy);
	bi.bmiHeader.biPlanes      = 1;
	bi.bmiHeader.biBitCount    = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	LPVOID pData = NULL;
	HBITMAP hbmp = ::CreateDIBSection (NULL, &bi, DIB_RGB_COLORS, &pData, NULL, 0);

	if (pData != NULL && hbmp != NULL && pBits != NULL)
	{
		*pBits = pData;
	}

	return hbmp;
}

HBITMAP CBCGPDrawManager::CreateBitmap_24 (const CSize& size, void** pBits)
{
	if (pBits != NULL)
	{
		*pBits = NULL;
	}

	if (size.cx <= 0 || size.cy == 0)
	{
		return NULL;
	}

	BITMAPINFO bi = {0};

	// Fill in the BITMAPINFOHEADER
	bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth       = size.cx;
	bi.bmiHeader.biHeight      = size.cy;
	bi.bmiHeader.biSizeImage   = 0;
	bi.bmiHeader.biPlanes      = 1;
	bi.bmiHeader.biBitCount    = 24;
	bi.bmiHeader.biCompression = BI_RGB;

	LPVOID pData = NULL;
	HBITMAP hbmp = ::CreateDIBSection (NULL, &bi, DIB_RGB_COLORS, &pData, NULL, 0);

	if (pData != NULL && hbmp != NULL && pBits != NULL)
	{
		*pBits = pData;
	}

	return hbmp;
}

HBITMAP CBCGPDrawManager::CreateBitmap_32 (HBITMAP bitmap, COLORREF clrTransparent/* = -1*/)
{
	if (bitmap == NULL)
	{
		return NULL;
	}

	BITMAP bmp;
	if (::GetObject (bitmap, sizeof (BITMAP), &bmp) == 0)
	{
		return NULL;
	}

	if (bmp.bmBits == NULL)
	{
		return NULL;
	}

	int nHeight = bmp.bmHeight;
	LPVOID lpBits = NULL;
	HBITMAP hbmp = CreateBitmap_32 (CSize (bmp.bmWidth, nHeight), &lpBits);
	nHeight = abs(nHeight);

	if (hbmp != NULL)
	{
		DWORD nSizeImage = bmp.bmWidth * nHeight;

		if (bmp.bmBitsPixel == 32)
		{
			memcpy (lpBits, bmp.bmBits, nSizeImage * 4);
		}
		else
		{
			CDC dcSrc;
			dcSrc.CreateCompatibleDC (NULL);
			HBITMAP hbmpSrc = (HBITMAP) dcSrc.SelectObject (bitmap);

			if (hbmpSrc != NULL)
			{
				CDC dcDst;
				dcDst.CreateCompatibleDC (NULL);
				HBITMAP hbmpDst = (HBITMAP) dcDst.SelectObject (hbmp);

				dcDst.BitBlt (0, 0, bmp.bmWidth, nHeight, &dcSrc, 0, 0, SRCCOPY);

				dcDst.SelectObject (hbmpDst);
				dcSrc.SelectObject (hbmpSrc);

				COLORREF* pBits = (COLORREF*) lpBits;
				if (clrTransparent == -1)
				{
					for (DWORD i = 0; i < nSizeImage; i++)
					{
						*pBits |= 0xFF000000;
						pBits++;
					}
				}
				else
				{
					COLORREF clrTrans = RGB (GetBValue (clrTransparent),
											 GetGValue (clrTransparent),
											 GetRValue (clrTransparent));

					for (DWORD i = 0; i < nSizeImage; i++)
					{
						if (*pBits != clrTrans)
						{
							*pBits |= 0xFF000000;
						}
						else
						{
							*pBits = (COLORREF) 0;
						}

						pBits++;
					}
				}
			}
		}
	}

	return hbmp;
}

HBITMAP CBCGPDrawManager::CreateBitmap_24(HBITMAP bitmap)
{
	if (bitmap == NULL)
	{
		return NULL;
	}

	BITMAP bmp;
	if (::GetObject (bitmap, sizeof (BITMAP), &bmp) == 0)
	{
		return NULL;
	}

	if (bmp.bmBits == NULL)
	{
		return NULL;
	}

	int nHeight = bmp.bmHeight;
	LPVOID lpBits = NULL;
	HBITMAP hbmp = CreateBitmap_24 (CSize (bmp.bmWidth, nHeight), &lpBits);
	nHeight = abs(nHeight);

	if (hbmp != NULL)
	{
		CDC dcSrc;
		dcSrc.CreateCompatibleDC (NULL);
		HBITMAP hbmpSrc = (HBITMAP) dcSrc.SelectObject (bitmap);

		if (hbmpSrc != NULL)
		{
			CDC dcDst;
			dcDst.CreateCompatibleDC (NULL);
			HBITMAP hbmpDst = (HBITMAP) dcDst.SelectObject (hbmp);

			dcDst.BitBlt (0, 0, bmp.bmWidth, nHeight, &dcSrc, 0, 0, SRCCOPY);

			dcDst.SelectObject (hbmpDst);
			dcSrc.SelectObject (hbmpSrc);
		}
	}

	return hbmp;
}

CBCGPDrawManager::CBCGPDrawManager(CDC& m_dc) :
	m_dc (m_dc)
{
}
//*************************************************************************************
CBCGPDrawManager::~CBCGPDrawManager()
{
}
//*************************************************************************************
BOOL CBCGPDrawManager::HighlightRect (CRect rect, int nPercentage, COLORREF clrTransparent,
									  int nTolerance, COLORREF clrBlend)
{
	if (nPercentage == 100)
	{
		// Nothing to do
		return TRUE;
	}

	if (rect.Height () <= 0 || rect.Width () <= 0)
	{
		return TRUE;
	}

	if (globalData.m_nBitsPerPixel <= 8)
	{
#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE)
		CBCGPToolBarImages::FillDitheredRect (&m_dc, rect);
#else
		m_dc.FillRect (&rect, &globalData.brLight);
#endif
		return TRUE;
	}

	if (clrBlend != (COLORREF)-1 && nPercentage > 100)
	{
		return FALSE;
	}

	int cx = rect.Width ();
	int cy = rect.Height ();

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		return FALSE;
	}

	//--------------------------------------------
	// Gets the whole menu and changes the shadow.
	//--------------------------------------------
	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, cx, cy))
	{
		return FALSE;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	if (pOldBmp == NULL)
	{
		return FALSE;
	}

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32 (CSize (cx, cy), (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		return FALSE;
	}

	dcMem.SelectObject (hmbpDib);
	dcMem.BitBlt (0, 0, cx, cy, &m_dc, rect.left, rect.top, SRCCOPY);

	if (clrTransparent != -1)
	{
		clrTransparent = RGB (GetBValue(clrTransparent), GetGValue(clrTransparent), GetRValue(clrTransparent));
	}

	for (int pixel = 0; pixel < cx * cy; pixel++, *pBits++)
	{
		COLORREF color = (COLORREF) *pBits;

		BOOL bIgnore = FALSE;

		if (nTolerance > 0)
		{
			bIgnore = (	abs (GetRValue (color) - GetRValue (clrTransparent)) < nTolerance &&
						abs (GetGValue (color) - GetGValue (clrTransparent)) < nTolerance &&
						abs (GetBValue (color) - GetBValue (clrTransparent)) < nTolerance);
		}
		else
		{
			bIgnore = color == clrTransparent;
		}

		if (!bIgnore)
		{
			if (nPercentage == -1)
			{
				*pBits = RGB_TO_RGBA (
					min (255, (2 * GetRValue (color) + GetBValue (globalData.clrBtnHilite)) / 3),
					min (255, (2 * GetGValue (color) + GetGValue (globalData.clrBtnHilite)) / 3),
					min (255, (2 * GetBValue (color) + GetRValue (globalData.clrBtnHilite)) / 3));
			}
			else
			{
				if (clrBlend == (COLORREF)-1)
				{
					*pBits = CLR_TO_RGBA (PixelAlpha (color, 
						.01 * nPercentage, .01 * nPercentage, .01 * nPercentage));
				}
				else
				{
					long R = GetRValue (color);
					long G = GetGValue (color);
					long B = GetBValue (color);

					*pBits = RGB_TO_RGBA (
						min (255, R + ::MulDiv (GetBValue (clrBlend) - R, nPercentage, 100)),
						min (255, G + ::MulDiv (GetGValue (clrBlend) - G, nPercentage, 100)),
						min (255, B + ::MulDiv (GetRValue (clrBlend) - B, nPercentage, 100))
						);
				}
			}
		}
	}

	//-------------------------------------------
	// Copy highligted bitmap back to the screen:
	//-------------------------------------------
	m_dc.BitBlt (rect.left, rect.top, cx, cy, &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);

	return TRUE;
}
//*********************************************************************************
void CBCGPDrawManager::MirrorRect (CRect rect, BOOL bHorz/* = TRUE*/)
{
	if (rect.Height () <= 0 || rect.Width () <= 0)
	{
		return;
	}

	CRect rectClip;
	m_dc.GetClipBox (rectClip);

	CRect rectUnion;
	rectUnion.UnionRect (rectClip, rect);

	if (rectUnion != rectClip)
	{
		return;
	}

	int cx = rect.Width ();
	int cy = rect.Height ();

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		return;
	}

	//--------------------------------------------
	// Gets the whole menu and changes the shadow.
	//--------------------------------------------
	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, cx, cy))
	{
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	if (pOldBmp == NULL)
	{
		return;
	}

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32 (CSize (cx, cy), (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		return;
	}

	dcMem.SelectObject (hmbpDib);
	dcMem.BitBlt (0, 0, cx, cy, &m_dc, rect.left, rect.top, SRCCOPY);

	if (bHorz)
	{
		for (int y = 0; y <= cy; y++)
		{
			for (int x = 0; x <= cx / 2; x++)
			{
				int xRight = cx - x - 1;
				int y1 = cy - y;

				if (cx * y1 + x >= cx * cy ||  
					cx * y1 + xRight >= cx * cy)
				{
					continue;
				}

				COLORREF* pColorLeft = (COLORREF*) (pBits + cx * y1 + x);
				COLORREF colorSaved = *pColorLeft;

				COLORREF* pColorRight = (COLORREF*) (pBits + cx * y1 + xRight);

				*pColorLeft = *pColorRight;
				*pColorRight = colorSaved;
			}
		}
	}
	else
	{
		for (int y = 0; y <= cy / 2; y++)
		{
			for (int x = 0; x < cx; x++)
			{
				int yBottom = cy - y - 1;

				COLORREF* pColorTop = (COLORREF*) (pBits + cx * y + x);
				COLORREF colorSaved = *pColorTop;

				COLORREF* pColorBottom = (COLORREF*) (pBits + cx * yBottom + x);

				*pColorTop = *pColorBottom;
				*pColorBottom = colorSaved;
			}
		}
	}

	m_dc.BitBlt (rect.left, rect.top, cx, cy, &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);
}
//*************************************************************************************
BOOL CBCGPDrawManager::GrayRect (CRect rect, int nPercentage, COLORREF clrTransparent,
								 COLORREF clrDisabled)
{
	if (rect.Height () <= 0 || rect.Width () <= 0)
	{
		return TRUE;
	}

	if (globalData.m_nBitsPerPixel <= 8)
	{
#if (!defined _BCGPCALENDAR_STANDALONE) && !(defined _BCGPGRID_STANDALONE)
		CBCGPToolBarImages::FillDitheredRect (&m_dc, rect);
#else
		m_dc.FillRect (&rect, &globalData.brLight);
#endif
		return TRUE;
	}

	int cx = rect.Width ();
	int cy = rect.Height ();

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		return FALSE;
	}

	//--------------------------------------------
	// Gets the whole menu and changes the shadow.
	//--------------------------------------------
	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, cx, cy))
	{
		return FALSE;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	if (pOldBmp == NULL)
	{
		return FALSE;
	}

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32 (CSize (cx, cy), (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		return FALSE;
	}

	dcMem.SelectObject (hmbpDib);
	dcMem.BitBlt (0, 0, cx, cy, &m_dc, rect.left, rect.top, SRCCOPY);

	if (clrTransparent != (COLORREF)-1)
	{
		clrTransparent = RGB (GetBValue(clrTransparent), GetGValue(clrTransparent), GetRValue(clrTransparent));
	}

	if (clrDisabled == (COLORREF)-1)
	{
		clrDisabled = globalData.clrBtnHilite;
	}

	for (int pixel = 0; pixel < cx * cy; pixel++, *pBits++)
	{
		COLORREF color = (COLORREF) *pBits;
		if (color != clrTransparent)
		{
			double H,S,L;
			RGBtoHSL(color, &H, &S, &L);
			color = HLStoRGB_ONE(H,L,0);
			
			if (nPercentage == -1)
			{
				*pBits = RGB_TO_RGBA (
					min (255, GetRValue (color) + ((GetBValue (clrDisabled) -
					GetRValue (color)) / 2)),
					min (255, GetGValue (color) + ((GetGValue (clrDisabled) -
					GetGValue (color)) / 2)),
					min (255, GetBValue(color) + ((GetRValue (clrDisabled) -
					GetBValue (color)) / 2)));
			}
			else
			{
				*pBits = CLR_TO_RGBA (PixelAlpha (color, .01 * nPercentage, .01 * nPercentage, .01 * nPercentage));
			}
		}
	}
	
	//-------------------------------------------
	// Copy highligted bitmap back to the screen:
	//-------------------------------------------
	m_dc.BitBlt (rect.left, rect.top, cx, cy, &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);

	return TRUE;
} 
//*************************************************************************************
void CBCGPDrawManager::_FillGradient (CRect rect, 
									COLORREF colorStart, COLORREF colorFinish, 
									BOOL bHorz/* = TRUE*/,
									int nStartFlatPercentage/* = 0*/,
									int nEndFlatPercentage/* = 0*/)
{
	if (colorStart == colorFinish)
	{
		CBrush br (colorStart);
		m_dc.FillRect (rect, &br);
		return;
	}

	if (nStartFlatPercentage > 0)
	{
		nStartFlatPercentage = min(nStartFlatPercentage, 100);

		if (bHorz)
		{
			CRect rectTop = rect;
			rectTop.bottom = rectTop.top + 
				rectTop.Height () * nStartFlatPercentage / 100;
			rect.top = rectTop.bottom;

			CBrush br (colorFinish);
			m_dc.FillRect (rectTop, &br);
		}
		else
		{
			CRect rectLeft = rect;
			rectLeft.right = rectLeft.left + 
				rectLeft.Width () * nStartFlatPercentage / 100;
			rect.left = rectLeft.right;

			CBrush br (colorStart);
			m_dc.FillRect (rectLeft, &br);
		}
	}

	if (nEndFlatPercentage > 0)
	{
		nEndFlatPercentage = min(nEndFlatPercentage, 100);

		if (bHorz)
		{
			CRect rectBottom = rect;
			rectBottom.top = rectBottom.bottom - 
				rectBottom.Height () * nEndFlatPercentage / 100;
			rect.bottom = rectBottom.top;

			CBrush br (colorStart);
			m_dc.FillRect (rectBottom, &br);
		}
		else
		{
			CRect rectRight = rect;
			rectRight.left = rectRight.right - 
				rectRight.Width () * nEndFlatPercentage / 100;
			rect.right = rectRight.left;

			CBrush br (colorFinish);
			m_dc.FillRect (rectRight, &br);
		}
	}

	if (nEndFlatPercentage + nStartFlatPercentage > 100)
	{
		return;
	}

    // this will make 2^6 = 64 fountain steps
    int nShift = 6;
    int nSteps = 1 << nShift;

    for (int i = 0; i < nSteps; i++)
    {
        // do a little alpha blending
        BYTE bR = (BYTE) ((GetRValue(colorStart) * (nSteps - i) +
                   GetRValue(colorFinish) * i) >> nShift);
        BYTE bG = (BYTE) ((GetGValue(colorStart) * (nSteps - i) +
                   GetGValue(colorFinish) * i) >> nShift);
        BYTE bB = (BYTE) ((GetBValue(colorStart) * (nSteps - i) +
                   GetBValue(colorFinish) * i) >> nShift);

		CBrush br (RGB(bR, bG, bB));

        // then paint with the resulting color
        CRect r2 = rect;
        if (bHorz)
        {
            r2.bottom = rect.bottom - 
                ((i * rect.Height()) >> nShift);
            r2.top = rect.bottom - 
                (((i + 1) * rect.Height()) >> nShift);
            if (r2.Height() > 0)
                m_dc.FillRect(r2, &br);
        }
        else
        {
            r2.left = rect.left + 
                ((i * rect.Width()) >> nShift);
            r2.right = rect.left + 
                (((i + 1) * rect.Width()) >> nShift);
            if (r2.Width() > 0)
                m_dc.FillRect(r2, &br);
        }
    }
}
void CBCGPDrawManager::FillGradient (CRect rect, 
									COLORREF colorStart, COLORREF colorFinish, 
									BOOL bHorz/* = TRUE*/,
									int nStartFlatPercentage/* = 0*/,
									int nEndFlatPercentage/* = 0*/)
{
	if (!CBCGPToolBarImages::m_bIsDrawOnGlass)
	{
		_FillGradient (rect, colorStart, colorFinish, bHorz,
						 nStartFlatPercentage, nEndFlatPercentage);
	}
	else
	{
		CRect rt (rect);
		rt.NormalizeRect ();

		CSize size (rt.Size ());
		if (size.cx == 0 || size.cy == 0)
		{
			return;
		}

		//--------------------------------------------
		// Copy screen content into the memory bitmap:
		//--------------------------------------------
		CDC dcMem;
		if (!dcMem.CreateCompatibleDC (&m_dc))
		{
			return;
		}

		//--------------------------------------------
		// Gets the whole menu and changes the shadow.
		//--------------------------------------------
		CBitmap	bmpMem;
		if (!bmpMem.CreateCompatibleBitmap (&m_dc, size.cx, size.cy))
		{
			return;
		}

		CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
		if (pOldBmp == NULL)
		{
			return;
		}

		COLORREF* pBits;
		HBITMAP hmbpDib = CreateBitmap_32 (size, (LPVOID*)&pBits);

		if (hmbpDib == NULL || pBits == NULL)
		{
			return;
		}

		dcMem.SelectObject (hmbpDib);

		CBCGPDrawManager dm (dcMem);
		dm._FillGradient (CRect (CPoint (0, 0), size), colorStart, colorFinish, bHorz,
						 nStartFlatPercentage, nEndFlatPercentage);

		int sizeImage = size.cx * size.cy;
		for (int i = 0; i < sizeImage; i++)
		{
			*pBits |= 0xFF000000;
			pBits++;
		}

		//--------------------------------
		// Copy bitmap back to the screen:
		//--------------------------------

		m_dc.BitBlt (rt.left, rt.top, size.cx, size.cy, &dcMem, 0, 0, SRCCOPY);

		dcMem.SelectObject (pOldBmp);
		DeleteObject (hmbpDib);
	}
}
//************************************************************************************
void CBCGPDrawManager::FillGradient2 (CRect rect, COLORREF colorStart, COLORREF colorFinish, 
					int nAngle)
{
	if (rect.Width () <= 0 || rect.Height () <= 0)
	{
		return;
	}

	if (colorStart == colorFinish)
	{
		CBrush br (colorStart);
		m_dc.FillRect (rect, &br);
		return;
	}

	//----------------------
	// Process simple cases:
	//----------------------
	switch (nAngle)
	{
	case 0:
	case 360:
		FillGradient (rect, colorStart, colorFinish, FALSE);
		return;

	case 90:
		FillGradient (rect, colorStart, colorFinish, TRUE);
		return;

	case 180:
		FillGradient (rect, colorFinish, colorStart, FALSE);
		return;

	case 270:
		FillGradient (rect, colorFinish, colorStart, TRUE);
		return;
	}

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		return;
	}

	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, rect.Width (), rect.Height ()))
	{
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject (&bmpMem);
	if (pOldBmp == NULL)
	{
		return;
	}

	CPen* pOldPen = (CPen*) dcMem.SelectStockObject (NULL_PEN);

    int nShift = 6;
    int nSteps = 1 << nShift;

	const double fAngle = bcg_deg2rad(nAngle + 180);
	const int nOffset = (int) (cos (fAngle) * rect.Height ());
	const int nTotalWidth = rect.Width () + abs (nOffset);

	const int xStart = nOffset > 0 ? - nOffset : 0;

    for (int i = 0; i < nSteps; i++)
    {
        // do a little alpha blending
        BYTE bR = (BYTE) ((GetRValue(colorStart) * (nSteps - i) +
                   GetRValue(colorFinish) * i) >> nShift);
        BYTE bG = (BYTE) ((GetGValue(colorStart) * (nSteps - i) +
                   GetGValue(colorFinish) * i) >> nShift);
        BYTE bB = (BYTE) ((GetBValue(colorStart) * (nSteps - i) +
                   GetBValue(colorFinish) * i) >> nShift);

		CBrush br (RGB (bR, bG, bB));

        int x11 = xStart + ((i * nTotalWidth) >> nShift);
        int x12 = xStart + (((i + 1) * nTotalWidth) >> nShift);

		if (x11 == x12)
		{
			continue;
		}

		int x21 = x11 + nOffset;
		int x22 = x21 + (x12 - x11);

		POINT points [4];
		points [0].x = x11;
		points [0].y = 0;
		points [1].x = x12;
		points [1].y = 0;
		points [2].x = x22;
		points [2].y = rect.Height ();
		points [3].x = x21;
		points [3].y = rect.Height ();

		CBrush* pOldBrush = dcMem.SelectObject (&br);
		dcMem.Polygon (points, 4);
		dcMem.SelectObject (pOldBrush);
	}

	dcMem.SelectObject (pOldPen);

	//--------------------------------
	// Copy bitmap back to the screen:
	//--------------------------------
	m_dc.BitBlt (rect.left, rect.top, rect.Width (), rect.Height (), &dcMem, 0, 0, SRCCOPY);
	dcMem.SelectObject (pOldBmp);
}
//************************************************************************************
void CBCGPDrawManager::Fill4ColorsGradient (CRect rect, 
	COLORREF colorStart1, COLORREF colorFinish1, 
	COLORREF colorStart2, COLORREF colorFinish2,
	BOOL bHorz/* = TRUE*/,
	int nPercentage)	/* = 50, 0 - 100 */
{
	nPercentage = min(max(0, nPercentage), 100);

	CRect rectFirst = rect;
	CRect rectSecond = rect;

	if (!bHorz)
	{
		rectFirst.right = rectFirst.left + rectFirst.Width () * nPercentage / 100;
		rectSecond.left = rectFirst.right;
	}
	else
	{
		rectFirst.bottom = rectFirst.top + rectFirst.Height () * nPercentage / 100;
		rectSecond.top = rectFirst.bottom;
	}

	FillGradient (rectFirst, colorStart1, colorFinish1, bHorz);
	FillGradient (rectSecond, colorStart2, colorFinish2, bHorz);
}
//************************************************************************************
BOOL CBCGPDrawManager::DrawGradientRing (CRect rect,
					   COLORREF colorStart, COLORREF colorFinish,
					   COLORREF colorBorder,
					   int nAngle /* 0 - 360 */,
					   int nWidth,
					   COLORREF clrFace /* = -1 */)
{
	int cx = rect.Width ();
	int cy = rect.Height ();

	if (cx <= 4 || cy <= 4)
	{
		//--------------------
		// Rectangle too small
		//--------------------
		return FALSE;
	}

	int xOrig = rect.left;
	int yOrig = rect.top;

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		return FALSE;
	}

	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, cx, cy))
	{
		return FALSE;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	if (pOldBmp == NULL)
	{
		return FALSE;
	}

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32 (CSize (cx, cy), (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		return FALSE;
	}

	dcMem.SelectObject (hmbpDib);
	dcMem.BitBlt (0, 0, cx, cy, &m_dc, rect.left, rect.top, SRCCOPY);

	rect.OffsetRect (-xOrig, -yOrig);

	const int xCenter = (rect.left + rect.right) / 2;
	const int yCenter = (rect.top + rect.bottom) / 2;

	const int nSteps = 360;
	const double fDelta = 2. * M_PI / nSteps;
	const double fStart = bcg_deg2rad(nAngle);
	const double fFinish = fStart + 2. * M_PI;

	double rDelta = (double) (.5 + GetRValue (colorFinish) - GetRValue (colorStart)) / nSteps * 2;
	double gDelta = (double) (.5 + GetGValue (colorFinish) - GetGValue (colorStart)) / nSteps * 2;
	double bDelta = (double) (.5 + GetBValue (colorFinish) - GetBValue (colorStart)) / nSteps * 2;

	for (int nLevel = 0; nLevel < nWidth; nLevel++)
	{
		int i = 0;
		const int nRadius = min (rect.Width (), rect.Height ()) / 2;
		const int nRectDelta = rect.Width () - rect.Height ();

		if (clrFace != (COLORREF) -1 && nLevel == 0)
		{
			//---------------
			// Fill interior:
			//---------------
			CBrush brFill (clrFace);
			CBrush* pOldBrush = dcMem.SelectObject (&brFill);
			CPen* pOldPen = (CPen*) dcMem.SelectStockObject (NULL_PEN);

			if (nRectDelta == 0)	// Circle
			{
				dcMem.Ellipse (rect);
			}
			else if (nRectDelta > 0)	// Horizontal
			{
				dcMem.Ellipse (rect.left, rect.top, rect.left + rect.Height (), rect.bottom);
				dcMem.Ellipse (rect.right - rect.Height (), rect.top, rect.right, rect.bottom);
				dcMem.Rectangle (rect.left + rect.Height () / 2, rect.top, rect.right - rect.Height () / 2, rect.bottom);
			}
			else	// Vertical
			{
				dcMem.Ellipse (rect.left, rect.top, rect.right, rect.top + rect.Width ());
				dcMem.Ellipse (rect.left, rect.bottom - rect.Width (), rect.right, rect.bottom);
				dcMem.Rectangle (rect.left, rect.top + rect.Width () / 2, rect.right, rect.bottom - rect.Width () / 2);
			}

			dcMem.SelectObject (pOldBrush);
			dcMem.SelectObject (pOldPen);
		}

		int xPrev = -1;
		int yPrev = -1;

		for (double fAngle = fStart; fAngle < fFinish + fDelta; fAngle += fDelta, i ++)
		{
			const int nStep = fAngle <= (fFinish + fStart) / 2 ? i : nSteps - i;

			const BYTE bR = (BYTE) max (0, min (255, (.5 + rDelta * nStep + GetRValue (colorStart))));
			const BYTE bG = (BYTE) max (0, min (255, (.5 + gDelta * nStep + GetGValue (colorStart))));
			const BYTE bB = (BYTE) max (0, min (255, (.5 + bDelta * nStep + GetBValue (colorStart))));

			COLORREF color = nLevel == 0 && colorBorder != -1 ? 
				colorBorder : RGB (bR, bG, bB);

			double dx = /*(fAngle >= 0 && fAngle <= PI / 2) || (fAngle >= 3 * PI / 2) ?
				.5 : -.5*/0;
			double dy = /*(fAngle <= PI) ? .5 : -.5*/0;

			int x = xCenter + (int) (dx + cos (fAngle) * nRadius);
			int y = yCenter + (int) (dy + sin (fAngle) * nRadius);

			if (nRectDelta > 0)
			{
				if (x > xCenter)
				{
					x += (int) (.5 * nRectDelta);
				}
				else
				{
					x -= (int) (.5 * nRectDelta);
				}

				if (xPrev != -1 && (xPrev > xCenter) != (x > xCenter))
				{
					for (int x1 = min (x, xPrev); x1 < max (x, xPrev); x1++)
					{
						SetPixel (pBits, cx, cy, x1, y, color);
					}
				}
			}
			else if (nRectDelta < 0)
			{
				if (y > yCenter)
				{
					y -= (int) (.5 * nRectDelta);
				}
				else
				{
					y += (int) (.5 * nRectDelta);
				}

				if (yPrev != -1 && (yPrev > yCenter) != (y > yCenter))
				{
					for (int y1 = min (y, yPrev); y1 < max (y, yPrev); y1++)
					{
						SetPixel (pBits, cx, cy, x, y1, color);
					}
				}
			}

			SetPixel (pBits, cx, cy, x, y, color);

			xPrev = x;
			yPrev = y;
		}

		rect.DeflateRect (1, 1);
	}


	//--------------------------------
	// Copy bitmap back to the screen:
	//--------------------------------
	m_dc.BitBlt (xOrig, yOrig, cx, cy, &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);

	return TRUE;
}
//*************************************************************************************
BOOL CBCGPDrawManager::DrawShadow (CRect rect, int nDepth,
								  int iMinBrightness, int iMaxBrightness,
								  CBitmap* pBmpSaveBottom,
								  CBitmap* pBmpSaveRight,
								  COLORREF clrBase,
								  BOOL bRightShadow/* = TRUE*/)
{
	if (nDepth <= 0 || rect.IsRectEmpty ())
	{
		return TRUE;
	}

	if (clrBase == (COLORREF)-1)
	{
		clrBase = RGB(192, 192, 192);
	}

	int cx = rect.Width ();
	int cy = rect.Height ();

	const BOOL bIsLeft = !bRightShadow;

	if (pBmpSaveRight != NULL && pBmpSaveRight->GetSafeHandle () != NULL &&
		pBmpSaveBottom != NULL && pBmpSaveBottom->GetSafeHandle () != NULL)
	{
		//---------------------------------------------------
		// Shadows are already implemented, put them directly
		// to the DC:
		//---------------------------------------------------
		m_dc.DrawState (CPoint (
						bIsLeft ? rect.left - nDepth : rect.right, 
						rect.top),
					CSize (nDepth, cy + nDepth),
					pBmpSaveRight, DSS_NORMAL);

		m_dc.DrawState (CPoint (
					bIsLeft ? rect.left - nDepth : rect.left, 
					rect.bottom),
					CSize (cx + nDepth, nDepth),
					pBmpSaveBottom, DSS_NORMAL);
		return TRUE;
	}

	ASSERT (pBmpSaveRight == NULL || pBmpSaveRight->GetSafeHandle () == NULL);
	ASSERT (pBmpSaveBottom == NULL || pBmpSaveBottom->GetSafeHandle () == NULL);

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		return FALSE;
	}

	//--------------------------------------------
	// Gets the whole menu and changes the shadow.
	//--------------------------------------------
	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, cx + nDepth, cy + nDepth))
	{
		return FALSE;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	if (pOldBmp == NULL)
	{
		return FALSE;
	}

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32 (CSize (cx + nDepth, cy + nDepth), (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		return FALSE;
	}

	dcMem.SelectObject (hmbpDib);
	dcMem.BitBlt (0, 0, cx + nDepth, cy + nDepth, &m_dc, 
		bIsLeft ? rect.left - nDepth : rect.left, 
		rect.top, SRCCOPY);

	BOOL bOldAlgorithm = TRUE;

#if (!defined _BCGSUITE_) && (!defined _BCGPCHART_STANDALONE)
	if (globalData.bIsOSAlphaBlendingSupport && !globalData.bIsWindows9x)
	{
		CBCGPShadowRenderer shadow;
		if (shadow.Create (nDepth, clrBase, 100 - iMinBrightness, 100 - iMaxBrightness, CBCGPToolBarImages::IsRTL ()))
		{
			shadow.DrawFrame (&dcMem, CRect (nDepth, nDepth, cx + nDepth, cy + nDepth));
			bOldAlgorithm = FALSE;
		}
	}
#endif

	if (bOldAlgorithm)
	{
		//----------------------------------------------------------------------------
		// Process shadowing:
		// For having a very nice shadow effect, its actually hard work. Currently,
		// I'm using a more or less "hardcoded" way to set the shadows (by using a
		// hardcoded algorythm):
		//
		// This algorythm works as follows:
		// 
		// It always draws a few lines, from left to bottom, from bottom to right,
		// from right to up, and from up to left). It does this for the specified
		// shadow width and the color settings.
		//-----------------------------------------------------------------------------

		// For speeding up things, iShadowOffset is the
		// value which is needed to multiply for each shadow step
		int iShadowOffset = (iMaxBrightness - iMinBrightness) / nDepth;

		// Loop for drawing the shadow
		// Actually, this was simpler to implement than I thought
		for (int c = 0; c < nDepth; c++)
		{
			// Draw the shadow from left to bottom
			for (int y = cy; y < cy + (nDepth - c); y++)
			{
				SetAlphaPixel (pBits, rect, c + nDepth, y, 
					iMaxBrightness - ((nDepth  - c) * (iShadowOffset)), nDepth, clrBase, bIsLeft);
			}

			// Draw the shadow from left to right
			for (int x = nDepth + (nDepth - c); x < cx + c; x++)
			{
				SetAlphaPixel(pBits, rect,x, cy + c,
					iMaxBrightness - ((c) * (iShadowOffset)),nDepth, clrBase, bIsLeft);
			}

			// Draw the shadow from top to bottom
			for (int y1 = nDepth + (nDepth - c); y1 < cy + c + 1; y1++)
			{
				SetAlphaPixel(pBits, rect, cx+c, y1, 
					iMaxBrightness - ((c) * (iShadowOffset)),
					nDepth, clrBase, bIsLeft);
			}
			
			// Draw the shadow from top to left
			for (int x1 = cx; x1 < cx + (nDepth - c); x1++)
			{
				SetAlphaPixel (pBits, rect, x1, c + nDepth,
					iMaxBrightness - ((nDepth - c) * (iShadowOffset)),
					nDepth, clrBase, bIsLeft);
			}
		}
	}

	//-----------------------------------------
	// Copy shadowed bitmap back to the screen:
	//-----------------------------------------
	m_dc.BitBlt (bIsLeft ? rect.left - nDepth : rect.left, 
		rect.top, 
		cx + nDepth, cy + nDepth, &dcMem, 0, 0, SRCCOPY);

	//------------------------------------
	// Save shadows in the memory bitmaps:
	//------------------------------------
	if (pBmpSaveRight != NULL)
	{
		pBmpSaveRight->CreateCompatibleBitmap (&m_dc, nDepth + 1, cy + nDepth);
		
		dcMem.SelectObject (pBmpSaveRight);
		dcMem.BitBlt (0, 0, nDepth, cy + nDepth,
			&m_dc, bIsLeft ? 0 : rect.right, rect.top, SRCCOPY);
	}

	if (pBmpSaveBottom != NULL)
	{
		pBmpSaveBottom->CreateCompatibleBitmap (&m_dc, cx + nDepth, nDepth + 1);

		dcMem.SelectObject (pBmpSaveBottom);
		dcMem.BitBlt (0, 0, cx + nDepth, nDepth, &m_dc,
						bIsLeft ? rect.left - nDepth : rect.left, 
						rect.bottom, SRCCOPY);
	}

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);

	return TRUE;
}

static BOOL SkipLinePixel(int nPenStyle, int nStep)
{
	if (nPenStyle == PS_SOLID)
	{
		return FALSE;
	}

	int i = nStep % 24;

	switch (nPenStyle)
	{
	case PS_DASH:
		return i >= 18;

	case PS_DOT:
		return (nStep % 5) >= 2;

	case PS_DASHDOT:
		return (i >= 10 && i < 15) || i >= 18;

	case PS_DASHDOTDOT:
		return (i >= 10 && i < 13) || (i >= 16 && i < 19) || (i >= 21);
	}

	return FALSE;
}

void CBCGPDrawManager::DrawLine (int x1, int y1, int x2, int y2, COLORREF clrLine, int nPenStyle)
{
	if (clrLine == -1)
	{
		return;
	}

    int x  = x1;
    int y  = y1;
	int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = bcg_sign(x2 - x1);
    int sy = bcg_sign(y2 - y1);

    if(dx == 0 && dy == 0)
    {
        return;
    }

	CRect rect (x1, y1, x2, y2);
	rect.NormalizeRect ();
	rect.InflateRect (0, 0, 1, 1);

	CSize size (rect.Size ());
	if (size.cx == 0 || size.cy == 0)
	{
		return;
	}

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		return;
	}

	//--------------------------------------------
	// Gets the whole menu and changes the shadow.
	//--------------------------------------------
	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, size.cx, size.cy))
	{
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	if (pOldBmp == NULL)
	{
		return;
	}

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32 (size, (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		dcMem.SelectObject (pOldBmp);
		return;
	}

	dcMem.SelectObject (hmbpDib);

#ifndef _BCGSUITE_
	if (CBCGPToolBarImages::m_pfAlphaBlend == NULL || globalData.bIsWindows9x)
	{
		dcMem.BitBlt (0, 0, size.cx, size.cy, &m_dc, rect.left, rect.top, SRCCOPY);
	}
#endif

	bool exch = false;

    if(dy > dx)
    {
        long z = dx;
        dx = dy;
        dy = z;
        exch = true;
    }

    long e = 2 * dy - dx;
    long i = 1;

	clrLine = RGB (GetBValue (clrLine), GetGValue (clrLine), GetRValue (clrLine)) | 0xFF000000;

    do
    {
		if (!SkipLinePixel(nPenStyle, i - 1))
		{
			*(pBits + (size.cy - (y - rect.top) - 1) * size.cx + (x - rect.left)) = clrLine;
		}

        while (e >= 0)
        {
            if (exch)
            {
                x += sx;
            }
            else
            {
                y += sy;
            }

            e -= 2 * dx;
        }

        if (exch)
        {
            y += sy;
        }
        else
        {
            x += sx;
        }

        e += 2 * dy;

        i++;
    }
    while (i <= dx);

	if (!SkipLinePixel(nPenStyle, i - 1))
	{
		*(pBits + (size.cy - (y - rect.top) - 1) * size.cx + (x - rect.left)) = clrLine;
	}

	//--------------------------------
	// Copy bitmap back to the screen:
	//--------------------------------

	DrawAlpha (&m_dc, rect, &dcMem, CRect (CPoint (0, 0), size));

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);
}

void CBCGPDrawManager::DrawLineA (double x1, double y1, double x2, double y2, COLORREF clrLine, int nPenStyle)
{
	if (clrLine == -1)
	{
		return;
	}

	double xd = x2 - x1;
    double yd = y2 - y1;

    if(xd == 0 && yd == 0)
    {
        return;
    }

	bool exch = false;

    if (fabs(xd) <= fabs(yd))
    {
        exch = true;

        double tmpreal = x1;
        x1 = y1;
        y1 = tmpreal;

        tmpreal = x2;
        x2 = y2;
        y2 = tmpreal;

        tmpreal = xd;
        xd = yd;
        yd = tmpreal;
    }

    if (x1 > x2)
    {
        double tmpreal = x1;
        x1 = x2;
        x2 = tmpreal;

        tmpreal = y1;
        y1 = y2;
        y2 = tmpreal;

        xd = x2 - x1;
        yd = y2 - y1;
    }

	double f1 = 0.0;
	double f2 = 0.0;
	double f3 = 0.0;
	double f4 = 0.0;

	double gradient = yd / xd;
	double intery;

	int ix1, ix2, iy1, iy2;

	{
		double xend = bcg_round (x1);
		double yend = y1 + gradient * (xend - x1);
		double xgap = 1.0 - bcg_frac (x1 + 0.5);
		ix1         = (int)xend;
		iy1         = (int)yend;

		intery      = yend + gradient;

		f2 = bcg_frac (yend) * xgap;
		f1 = xgap - f2;

		xend        = bcg_round (x2);
		yend        = y2 + gradient * (xend - x2);
		xgap        = bcg_frac (x2 + 0.5);
		ix2         = (int)xend;
		iy2         = (int)yend;

		f4 = bcg_frac (yend) * xgap;
		f3 = xgap - f4;
	}

	CRect rect (ix1, iy1, ix2, iy2);

	if (exch)
	{
		rect = CRect (iy1, ix1, iy2, ix2);
	}

	rect.NormalizeRect ();
	rect.InflateRect (0, 0, 1, 1);

	if (exch)
	{
		rect.right++;
	}
	else
	{
		rect.bottom++;
	}

	CSize size (rect.Size ());
	if (size.cx == 0 || size.cy == 0)
	{
		return;
	}

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		return;
	}

	//--------------------------------------------
	// Gets the whole menu and changes the shadow.
	//--------------------------------------------
	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, size.cx, size.cy))
	{
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	if (pOldBmp == NULL)
	{
		return;
	}

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32 (size, (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		dcMem.SelectObject (pOldBmp);
		return;
	}

	dcMem.SelectObject (hmbpDib);

#ifndef _BCGSUITE_
	if (CBCGPToolBarImages::m_pfAlphaBlend == NULL || globalData.bIsWindows9x)
	{
		dcMem.BitBlt (0, 0, size.cx, size.cy, &m_dc, rect.left, rect.top, SRCCOPY);
	}
#endif

	int clrR = GetRValue (clrLine);
	int clrG = GetGValue (clrLine);
	int clrB = GetBValue (clrLine);
	int clrA = 255;

	if (nPenStyle == PS_SOLID)
	{
		if (exch)
		{
			COLORREF* pRow = pBits + (size.cy - (ix1 - rect.top) - 1) * size.cx + (iy1 - rect.left);

			pRow[0] = RGB (clrB * f1, clrG * f1, clrR * f1) | ((int)(clrA * f1) << 24);
			pRow[1] = RGB (clrB * f2, clrG * f2, clrR * f2) | ((int)(clrA * f2) << 24);

			pRow = pBits + (size.cy - (ix2 - rect.top) - 1) * size.cx + (iy2 - rect.left);

			pRow[0] = RGB (clrB * f3, clrG * f3, clrR * f3) | ((int)(clrA * f3) << 24);
			pRow[1] = RGB (clrB * f4, clrG * f4, clrR * f4) | ((int)(clrA * f4) << 24);
		}
		else
		{
			COLORREF* pRow = pBits + (size.cy - (iy1 - rect.top) - 2) * size.cx + (ix1 - rect.left);

			pRow[size.cx] = RGB (clrB * f1, clrG * f1, clrR * f1) | ((int)(clrA * f1) << 24);
			pRow[0]       = RGB (clrB * f2, clrG * f2, clrR * f2) | ((int)(clrA * f2) << 24);

			pRow = pBits + (size.cy - (iy2 - rect.top) - 2) * size.cx + (ix2 - rect.left);

			pRow[size.cx] = RGB (clrB * f3, clrG * f3, clrR * f3) | ((int)(clrA * f3) << 24);
			pRow[0]       = RGB (clrB * f4, clrG * f4, clrR * f4) | ((int)(clrA * f4) << 24);
		}
	}

	int nStep = 0;

    for(int x = ix1 + 1; x <= ix2 - 1; x++, nStep++)
    {
		if (!SkipLinePixel(nPenStyle, nStep))
		{
			double f = bcg_frac (intery);

			int y = (int)intery;

			int B = (int)(clrB * f);
			int G = (int)(clrG * f);
			int R = (int)(clrR * f);
			int A = (int)(clrA * f);

			if (exch)
			{
				COLORREF* pRow = pBits + (size.cy - (x - rect.top) - 1) * size.cx + (y - rect.left);
				
				pRow[0] = RGB ((clrB - B), (clrG - G), (clrR - R)) | ((clrA - A) << 24);
				pRow[1] = RGB (B, G, R) | (A << 24);
			}
			else
			{
				COLORREF* pRow = pBits + (size.cy - (y - rect.top) - 2) * size.cx + (x - rect.left);

				pRow[size.cx] = RGB ((clrB - B), (clrG - G), (clrR - R)) | ((clrA - A) << 24);
				pRow[0]       = RGB (B, G, R) | (A << 24);
			}
		}

        intery = intery + gradient;
    }

	//--------------------------------
	// Copy bitmap back to the screen:
	//--------------------------------

	DrawAlpha (&m_dc, rect, &dcMem, CRect (CPoint (0, 0), size));

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);
}

void CBCGPDrawManager::DrawEllipse (const CRect& rect, COLORREF clrFill, COLORREF clrLine, int nPenStyle)
{
	if (clrFill == -1 && clrLine == -1)
	{
		return;
	}

	CRect rt (rect);
	rt.NormalizeRect ();

	CSize size (rt.Size ());
	if (size.cx <= 2 || size.cy <= 2)
	{
		return;
	}

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		return;
	}

	//--------------------------------------------
	// Gets the whole menu and changes the shadow.
	//--------------------------------------------
	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, size.cx, size.cy))
	{
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	if (pOldBmp == NULL)
	{
		return;
	}

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32 (size, (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		dcMem.SelectObject (pOldBmp);
		return;
	}

	dcMem.SelectObject (hmbpDib);

#ifndef _BCGSUITE_
	if (CBCGPToolBarImages::m_pfAlphaBlend == NULL || globalData.bIsWindows9x)
	{
		dcMem.BitBlt (0, 0, size.cx, size.cy, &m_dc, rt.left, rt.top, SRCCOPY);
	}
#endif

	if (clrLine == -1)
	{
		clrLine = clrFill;
	}

	int brdR = GetRValue (clrLine);
	int brdG = GetGValue (clrLine);
	int brdB = GetBValue (clrLine);

	int filR = GetRValue (clrFill);
	int filG = GetGValue (clrFill);
	int filB = GetBValue (clrFill);

	BOOL bFill = clrFill != -1;
	if (bFill)
	{
		clrFill = RGB (filB, filG, filR) | 0xFF000000;
	}

	int R, G, B, A;
	COLORREF clrN, clrI;

	double a = (size.cx - 1) / 2.0;
	double b = (size.cy - 1) / 2.0;
	const double ab = bcg_distance(a, b);

	if (bFill)
	{
		LPBYTE pFillRow = new BYTE[size.cx * 4];
		COLORREF* pFillClr = (COLORREF*)pFillRow;
		for(int i = 0; i < size.cx; i++)
		{
			*pFillClr++ = clrFill;
		}

		int y1 = 0;
		int y2 = (int)floor(b + b);
		COLORREF* pRow = pBits;
		for(int iy = y1; iy <= y2; iy++)
		{
			double dist = 1.0 - bcg_sqr((iy - b) / b);
			if(dist < 0)
			{
				continue;
			}

			double x  = a * sqrt(dist);
			int x1 = (int)ceil(a - x);
			int x2 = (int)floor(a + x);

			if (x2 < x1)
			{
				continue;
			}

			memcpy(pRow + x1, pFillRow, (x2 - x1 + 1) * 4);
			pRow += size.cx;
		}

		delete [] pFillRow;
	}

	double t  = a * a / ab;
	int i1 = (int)floor (a - t);
	int i2 = (int)ceil (a + t);

	int nStep = 0;

	for(int ix = i1; ix <= i2; ix++, nStep++)
	{
		double dist = 1.0 - bcg_sqr((ix - a) / a);
		if(dist < 0)
		{
			continue;
		}

		double y  = b * sqrt(dist);
		int iy = (int)ceil(b + y);
		double f  = iy - b - y;

		B = (int)(brdB * f);
		G = (int)(brdG * f);
		R = (int)(brdR * f);
		A = (int)(255  * f);

		if (bFill)
		{
			double fi = 1.0 - f;
			clrN = RGB (filB * fi + B, filG * fi + G, filR * fi + R) | 0xFF000000;
		}
		else
		{
			clrN = RGB (B, G, R) | (A << 24);
		}

		clrI = RGB ((brdB - B), (brdG - G), (brdR - R)) | ((255 - A) << 24);

		if (!SkipLinePixel(nPenStyle, nStep))
		{
			*(pBits + iy * size.cx + ix) = clrI;
			*(pBits + (iy - 1) * size.cx + ix) = clrN;
		}

		iy = (int)floor (b - y);
		f  = b - y - iy;

		B = (int)(brdB * f);
		G = (int)(brdG * f);
		R = (int)(brdR * f);
		A = (int)(255  * f);

		if (bFill)
		{
			double fi = 1.0 - f;
			clrN = RGB (filB * fi + B, filG * fi + G, filR * fi + R) | 0xFF000000;
		}
		else
		{
			clrN = RGB (B, G, R) | (A << 24);
		}
		
		clrI = RGB ((brdB - B), (brdG - G), (brdR - R)) | ((255 - A) << 24);

		if (!SkipLinePixel(nPenStyle, nStep))
		{
			*(pBits + iy * size.cx + ix) = clrI;
			*(pBits + (iy + 1) * size.cx + ix) = clrN;
		}
	}

	t  = b * b / ab;
	i1 = (int)ceil (b - t);
	i2 = (int)floor (b + t);

	for(int iy = i1; iy <= i2; iy++, nStep++)
	{
		double dist = 1.0 - bcg_sqr((iy - b) / b);
		if(dist < 0)
		{
			continue;
		}

		double x  = a * sqrt(dist);
		int ix = (int)floor(a - x);
		double f  = a - x - ix;

		B = (int)(brdB * f);
		G = (int)(brdG * f);
		R = (int)(brdR * f);
		A = (int)(255  * f);

		if (bFill)
		{
			double fi = 1.0 - f;
			clrN = RGB (filB * fi + B, filG * fi + G, filR * fi + R) | 0xFF000000;
		}
		else
		{
			clrN = RGB (B, G, R) | (A << 24);
		}

		clrI = RGB ((brdB - B), (brdG - G), (brdR - R)) | ((255 - A) << 24);

		if (!SkipLinePixel(nPenStyle, nStep))
		{
			*(pBits + iy * size.cx + ix) = clrI;
			*(pBits + iy * size.cx + ix + 1) = clrN;
		}

		ix = (int)ceil(a + x);
		f  = ix - a - x;

		B = (int)(brdB * f);
		G = (int)(brdG * f);
		R = (int)(brdR * f);
		A = (int)(255  * f);

		if (bFill)
		{
			double fi = 1.0 - f;
			clrN = RGB (filB * fi + B, filG * fi + G, filR * fi + R) | 0xFF000000;
		}
		else
		{
			clrN = RGB (B, G, R) | (A << 24);
		}

		clrI = RGB ((brdB - B), (brdG - G), (brdR - R)) | ((255 - A) << 24);

		if (!SkipLinePixel(nPenStyle, nStep))
		{
			*(pBits + iy * size.cx + ix) = clrI;
			*(pBits + iy * size.cx + ix - 1) = clrN;
		}
	}

	//--------------------------------
	// Copy bitmap back to the screen:
	//--------------------------------

	DrawAlpha (&m_dc, rt, &dcMem, CRect (CPoint (0, 0), size));

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);
}

void CBCGPDrawManager::DrawArc(const CRect& rect, double dblStartAngle, double dblFinishAngle, BOOL bIsClockwise, COLORREF clrLine, int nPenStyle)
{
	if (clrLine == -1)
	{
		return;
	}

	CRect rt (rect);
	rt.NormalizeRect ();

	CSize size (rt.Size ());
	if (size.cx <= 2 || size.cy <= 2)
	{
		if (size.cx > 0 && size.cy > 0)
		{
			if (size.cx <= 2)
			{
				int x = (rect.left + rect.right) / 2;
				DrawLine(x, rect.top, x, rect.bottom, clrLine, nPenStyle);
			}
			else
			{
				int y = (rect.top + rect.bottom) / 2;
				DrawLine(rect.left, y, rect.right, y, clrLine, nPenStyle);
			}
		}

		return;
	}

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		return;
	}

	//--------------------------------------------
	// Gets the whole menu and changes the shadow.
	//--------------------------------------------
	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, size.cx, size.cy))
	{
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	if (pOldBmp == NULL)
	{
		return;
	}

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32 (size, (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		dcMem.SelectObject (pOldBmp);
		return;
	}

	dcMem.SelectObject (hmbpDib);

#ifndef _BCGSUITE_
	if (CBCGPToolBarImages::m_pfAlphaBlend == NULL || globalData.bIsWindows9x)
	{
		dcMem.BitBlt (0, 0, size.cx, size.cy, &m_dc, rt.left, rt.top, SRCCOPY);
	}
#endif

	int brdR = GetRValue (clrLine);
	int brdG = GetGValue (clrLine);
	int brdB = GetBValue (clrLine);

	int R, G, B, A;

	const double a = (size.cx - 1) / 2.0;
	const double b = (size.cy - 1) / 2.0;
	const double ab = bcg_distance(a, b);

	CPoint ptCenter(rect.CenterPoint ());

	double dblAngle1 = bcg_normalize_rad (bcg_deg2rad (dblStartAngle));
	double dblAngle2 = bcg_normalize_rad (bcg_deg2rad (dblFinishAngle));

	if (bIsClockwise)
	{
		if (dblAngle1 < dblAngle2)
		{
			dblAngle1 += 2.0 * M_PI;
		}
	}
	else
	{
		if (dblAngle2 < dblAngle1)
		{
			dblAngle2 += 2.0 * M_PI;
		}
	}

	if (dblAngle2 < dblAngle1)
	{
		double t = dblAngle1;
		dblAngle1 = dblAngle2;
		dblAngle2 = t;
	}

	double t = a * a / ab;
	int i1 = (int)floor (a - t);
	int i2 = (int)ceil (a + t);

	int nStep = 0;

	for(int ix = i1; ix <= i2; ix++, nStep++)
	{
		double dist = 1.0 - bcg_sqr((ix - a) / a);
		if(dist < 0)
		{
			continue;
		}

		double y = b * sqrt(dist);

		double dblA = bcg_normalize_rad (bcg_angle (ix - a, y));
		double dblA_P2 = dblA + M_PI * 2.0;

		if (!SkipLinePixel(nPenStyle, nStep))
		{
			if ((dblAngle1 <= dblA && dblA <= dblAngle2) ||
				(dblAngle1 <= dblA_P2 && dblA_P2 <= dblAngle2))
			{
				int iy = (int)ceil(b + y);
				double f = iy - b - y;

				B = (int)(brdB * f);
				G = (int)(brdG * f);
				R = (int)(brdR * f);
				A = (int)(255  * f);

				*(pBits + iy * size.cx + ix) = RGB ((brdB - B), (brdG - G), (brdR - R)) | ((255 - A) << 24);
				*(pBits + (iy - 1) * size.cx + ix) = RGB (B, G, R) | (A << 24);
			}
		}

		dblA = bcg_normalize_rad (bcg_angle (ix - a, -y));
		dblA_P2 = dblA + M_PI * 2.0;

		if (!SkipLinePixel(nPenStyle, nStep))
		{
			if ((dblAngle1 <= dblA && dblA <= dblAngle2) ||
				(dblAngle1 <= dblA_P2 && dblA_P2 <= dblAngle2))
			{
				int iy = (int)floor (b - y);
				double f = b - y - iy;

				B = (int)(brdB * f);
				G = (int)(brdG * f);
				R = (int)(brdR * f);
				A = (int)(255  * f);

				*(pBits + iy * size.cx + ix) = RGB ((brdB - B), (brdG - G), (brdR - R)) | ((255 - A) << 24);
				*(pBits + (iy + 1) * size.cx + ix) = RGB (B, G, R) | (A << 24);
			}
		}
	}

	t  = b * b / ab;
	i1 = (int)ceil (b - t);
	i2 = (int)floor (b + t);

	for(int iy = i1; iy <= i2; iy++, nStep++)
	{
		double dist = 1.0 - bcg_sqr((iy - b) / b);
		if(dist < 0)
		{
			continue;
		}

		double x = a * sqrt(dist);

		double dblA = bcg_normalize_rad (bcg_angle (-x, iy - b));
		double dblA_P2 = dblA + M_PI * 2.0;

		if (!SkipLinePixel(nPenStyle, nStep))
		{
			if ((dblAngle1 <= dblA && dblA <= dblAngle2) ||
				(dblAngle1 <= dblA_P2 && dblA_P2 <= dblAngle2))
			{
				int ix = (int)floor(a - x);
				double f  = a - x - ix;

				B = (int)(brdB * f);
				G = (int)(brdG * f);
				R = (int)(brdR * f);
				A = (int)(255  * f);

				*(pBits + iy * size.cx + ix) = RGB ((brdB - B), (brdG - G), (brdR - R)) | ((255 - A) << 24);
				*(pBits + iy * size.cx + ix + 1) = RGB (B, G, R) | (A << 24);
			}
		}

		dblA = bcg_normalize_rad (bcg_angle (x, iy - b));
		dblA_P2 = dblA + M_PI * 2.0;

		if (!SkipLinePixel(nPenStyle, nStep))
		{
			if ((dblAngle1 <= dblA && dblA <= dblAngle2) ||
				(dblAngle1 <= dblA_P2 && dblA_P2 <= dblAngle2))
			{
				int ix = (int)ceil(a + x);
				double f  = ix - a - x;

				B = (int)(brdB * f);
				G = (int)(brdG * f);
				R = (int)(brdR * f);
				A = (int)(255  * f);

				*(pBits + iy * size.cx + ix) = RGB ((brdB - B), (brdG - G), (brdR - R)) | ((255 - A) << 24);
				*(pBits + iy * size.cx + ix - 1) = RGB (B, G, R) | (A << 24);
			}
		}
	}

	//--------------------------------
	// Copy bitmap back to the screen:
	//--------------------------------

	DrawAlpha (&m_dc, rt, &dcMem, CRect (CPoint (0, 0), size));

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);
}

void CBCGPDrawManager::DrawRect (const CRect& rect, COLORREF clrFill, COLORREF clrLine)
{
	if (clrFill == -1 && clrLine == -1)
	{
		return;
	}

	CRect rt (rect);
	rt.NormalizeRect ();

	CSize size (rt.Size ());
	if (size.cx == 0 || size.cy == 0)
	{
		return;
	}

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		return;
	}

	//--------------------------------------------
	// Gets the whole menu and changes the shadow.
	//--------------------------------------------
	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, size.cx, size.cy))
	{
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	if (pOldBmp == NULL)
	{
		return;
	}

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32 (size, (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		dcMem.SelectObject (pOldBmp);
		return;
	}

	dcMem.SelectObject (hmbpDib);

#ifndef _BCGSUITE_
	if (CBCGPToolBarImages::m_pfAlphaBlend == NULL || globalData.bIsWindows9x)
	{
		dcMem.BitBlt (0, 0, size.cx, size.cy, &m_dc, rt.left, rt.top, SRCCOPY);
	}
#endif

	int xB = 0;
	int xE = size.cx;
	int yB = 1;
	int yE = size.cy;

	if (clrLine != -1)
	{
		COLORREF clr = RGB (GetBValue (clrLine),
							GetGValue (clrLine),
							GetRValue (clrLine)) | 0xFF000000;

		for (int x = 0; x < size.cx; x++)
		{
			*pBits = clr;
			pBits++;
		}

		if (1 < size.cy)
		{
			memcpy((LPVOID)(pBits + (size.cy - 2) * size.cx), (LPCVOID)(pBits - size.cx), size.cx * sizeof(COLORREF));

			if (2 < size.cy)
			{
				*pBits = clr;
				if (2 <= size.cx)
				{
					*(pBits + size.cx - 1) = clr;
				}
				pBits++;
			}
		}

		xB++;
		xE--;
		yB++;
		yE--;
	}

	COLORREF clr = clrFill == -1
			? 0
			: RGB (GetBValue (clrFill),
					GetGValue (clrFill),
					GetRValue (clrFill)) | 0xFF000000;

	if (yB <= yE)
	{
		for (int x = xB; x < xE; x++)
		{
			*pBits = clr;
			pBits++;
		}

		if (xB < xE && clrLine != -1)
		{
			pBits++;
		}
	}

	for (int y = yB; y < yE; y++)
	{
		memcpy((LPVOID)(pBits), (LPCVOID)(pBits - size.cx), size.cx * sizeof(COLORREF));
		pBits += size.cx;
	}

	//--------------------------------
	// Copy bitmap back to the screen:
	//--------------------------------

	if (clrFill != -1)
	{
		m_dc.BitBlt (rt.left, rt.top, size.cx, size.cy, &dcMem, 0, 0, SRCCOPY);
	}
	else
	{
		DrawAlpha (&m_dc, rt, &dcMem, CRect (CPoint (0, 0), size));
	}

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);
}

void CBCGPDrawManager::DrawFocusRect(const CRect& rect)
{
	CRect rt (rect);
	rt.NormalizeRect ();

	CSize size (rt.Size ());
	if (size.cx == 0 || size.cy == 0)
	{
		return;
	}

	//--------------------------------------------
	// Copy screen content into the memory bitmap:
	//--------------------------------------------
	CDC dcMem;
	if (!dcMem.CreateCompatibleDC (&m_dc))
	{
		return;
	}

	//--------------------------------------------
	// Gets the whole menu and changes the shadow.
	//--------------------------------------------
	CBitmap	bmpMem;
	if (!bmpMem.CreateCompatibleBitmap (&m_dc, size.cx, size.cy))
	{
		return;
	}

	CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);
	if (pOldBmp == NULL)
	{
		return;
	}

	COLORREF* pBits;
	HBITMAP hmbpDib = CreateBitmap_32 (size, (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		dcMem.SelectObject (pOldBmp);
		return;
	}

	dcMem.SelectObject (hmbpDib);

#ifndef _BCGSUITE_
	if (CBCGPToolBarImages::m_pfAlphaBlend == NULL || globalData.bIsWindows9x)
	{
		dcMem.BitBlt (0, 0, size.cx, size.cy, &m_dc, rt.left, rt.top, SRCCOPY);
	}
#endif

	int xB = 0;
	int xE = size.cx;
	int yB = 1;
	int yE = size.cy;

	COLORREF clrLine = RGB(127, 127, 127);

	COLORREF clr = RGB (GetBValue (clrLine),
						GetGValue (clrLine),
						GetRValue (clrLine)) | 0xFF000000;

	for (int x = 0; x < size.cx; x++)
	{
		*pBits = (x % 2) ? clr : 0;
		pBits++;
	}

	if (1 < size.cy)
	{
		memcpy((LPVOID)(pBits + (size.cy - 2) * size.cx), (LPCVOID)(pBits - size.cx), size.cx * sizeof(COLORREF));

		if (2 < size.cy)
		{
			*pBits = clr;
			if (2 <= size.cx)
			{
				*(pBits + size.cx - 1) = clr;
			}
			pBits++;
		}
	}

	xB++;
	xE--;
	yB++;
	yE--;

	if (yB <= yE)
	{
		for (int x = xB; x < xE; x++)
		{
			*pBits = 0;
			pBits++;
		}

		if (xB < xE)
		{
			pBits++;
		}
	}

	for (int y = yB; y < yE; y++)
	{
		if (y % 2)
		{
			memcpy((LPVOID)(pBits), (LPCVOID)(pBits - 2 * size.cx), size.cx * sizeof(COLORREF));
		}

		pBits += size.cx;
	}

	//--------------------------------
	// Copy bitmap back to the screen:
	//--------------------------------
	DrawAlpha (&m_dc, rt, &dcMem, CRect (CPoint (0, 0), size));

	dcMem.SelectObject (pOldBmp);
	DeleteObject (hmbpDib);
}

void CBCGPDrawManager::SetAlphaPixel (COLORREF* pBits, 
											 CRect rect, int x, int y, 
											 int percent, int m_iShadowSize,
											 COLORREF clrBase,
											 BOOL bIsRight)
{
	// Our direct bitmap access swapped the y coordinate...
	y = (rect.Height()+m_iShadowSize)- y;

	COLORREF* pColor = (COLORREF*) (bIsRight ? 
		(pBits + (rect.Width () + m_iShadowSize) * (y + 1) - x) : 
		(pBits + (rect.Width () + m_iShadowSize) * y + x));

	*pColor = PixelAlpha (*pColor, percent);

	if (clrBase == (COLORREF)-1)
	{
		return;
	}

	*pColor = RGB (	min (255, (3 * GetRValue (*pColor) + GetBValue (clrBase)) / 4),
					min (255, (3 * GetGValue (*pColor) + GetGValue (clrBase)) / 4),
					min (255, (3 * GetBValue (*pColor) + GetRValue (clrBase)) / 4));
}

COLORREF CBCGPDrawManager::PixelAlpha (COLORREF srcPixel, int percent)
{
	// My formula for calculating the transpareny is as
	// follows (for each single color):
	//
	//							   percent
	// destPixel = sourcePixel * ( ------- )
	//                               100
	//
	// This is not real alpha blending, as it only modifies the brightness,
	// but not the color (a real alpha blending had to mix the source and
	// destination pixels, e.g. mixing green and red makes yellow).
	// For our nice "menu" shadows its good enough.

	COLORREF clrFinal = RGB ( min (255, (GetRValue (srcPixel) * percent) / 100), 
							  min (255, (GetGValue (srcPixel) * percent) / 100), 
							  min (255, (GetBValue (srcPixel) * percent) / 100));

//	TRACE ("%d %d %d\n", GetRValue (clrFinal), GetGValue (clrFinal), GetBValue (clrFinal));
	return (clrFinal);

}

static inline int AdjustChannel (int nValue, double nPercent)
{
	int nNewValue = (int) (.5 + nPercent * nValue);
	if (nValue == 0 && nPercent > 1.)
	{
		nNewValue = (int) (.5 + (nPercent - 1.) * 255);
	}

	return min (nNewValue, 255);
}

COLORREF CBCGPDrawManager::PixelAlpha (COLORREF srcPixel, double percentR, double percentG, double percentB)
{
	COLORREF clrFinal = RGB ( AdjustChannel (GetRValue (srcPixel), percentR), 
							  AdjustChannel (GetGValue (srcPixel), percentG), 
							  AdjustChannel (GetBValue (srcPixel), percentB));

	return (clrFinal);

}

// ==================================================================
// 
// FUNCTION :  PixelAlpha ()
// 
// * Description : Shades a color value with a specified percentage
// 
// * Author : [Guillaume Nodet]
// 
// * Returns : [COLORREF] - The result pixel
// 
// * Function parameters : 
// [srcPixel] - The source pixel
// [dstPixel] - The destination pixel
// [percent] -  Percentage (amount of shadow)
//
// ==================================================================
COLORREF CBCGPDrawManager::PixelAlpha (COLORREF srcPixel, COLORREF dstPixel, int percent)
{
	int ipercent = 100 - percent;
	COLORREF clrFinal = RGB ( (GetRValue (srcPixel) * percent + GetRValue (dstPixel) * ipercent) / 100, 
							  (GetGValue (srcPixel) * percent + GetGValue (dstPixel) * ipercent) / 100, 
							  (GetBValue (srcPixel) * percent + GetBValue (dstPixel) * ipercent) / 100);

	return (clrFinal);

}

void CBCGPDrawManager::SetPixel (COLORREF* pBits, int cx, int cy, int x, int y,
								COLORREF color)
{
	// Our direct bitmap access swapped the y coordinate...
	y = cy - y;

	int nOffset = cx * y + x;
	if (nOffset < cx * cy)
	{
		COLORREF* pColor = (COLORREF*) (pBits + nOffset);
		*pColor = RGB (GetBValue(color), GetGValue(color), GetRValue(color));
	}
}

//----------------------------------------------------------------------
// Conversion between the HSL (Hue, Saturation, and Luminosity) 
// and RBG color model.
//----------------------------------------------------------------------
// The conversion algorithms presented here come from the book by 
// Fundamentals of Interactive Computer Graphics by Foley and van Dam. 
// In the example code, HSL values are represented as floating point 
// number in the range 0 to 1. RGB tridrants use the Windows convention 
// of 0 to 255 of each element. 
//----------------------------------------------------------------------

double CBCGPDrawManager::HuetoRGB(double m1, double m2, double h )
{
	if( h < 0 ) h += 1.0;
	if( h > 1 ) h -= 1.0;
	if( 6.0*h < 1 )
		return (m1+(m2-m1)*h*6.0);
	if( 2.0*h < 1 )
		return m2;
	if( 3.0*h < 2.0 )
		return (m1+(m2-m1)*((2.0/3.0)-h)*6.0);
	return m1;
}

BYTE CBCGPDrawManager::HueToRGB(float rm1, float rm2, float rh)
{
	if (rh > 360.0f)
		rh -= 360.0f;
	else if (rh < 0.0f)
		rh += 360.0f;
	
	if (rh <  60.0f)
		rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;   
	else if (rh < 180.0f)
		rm1 = rm2;
	else if (rh < 240.0f)
		rm1 = rm1 + (rm2 - rm1) * (240.0f - rh) / 60.0f;      
	
	return static_cast<BYTE>(rm1 * 255);
}

COLORREF CBCGPDrawManager::HLStoRGB_ONE( double H, double L, double S )
{
	double r, g, b;
	double m1, m2;
	
	if(S==0) {
		r=g=b=L;
	} else {
		if(L <=0.5)
			m2 = L*(1.0+S);
		else
			m2 = L+S-L*S;
		m1 = 2.0*L-m2;
		r = HuetoRGB(m1, m2, H+1.0/3.0);
		g = HuetoRGB(m1, m2, H);
		b = HuetoRGB(m1, m2, H-1.0/3.0);
	}
	return RGB((BYTE)(r*255), (BYTE)(g*255), (BYTE)(b*255));
}

COLORREF CBCGPDrawManager::HLStoRGB_TWO( double H, double L, double S)
{
	WORD R, G, B; // RGB component values
	
	if (S == 0.0)
		R = G = B = unsigned char(L * 255.0);
	else
	{
		float rm1, rm2;
		
		if (L <= 0.5f)
			rm2 = (float)(L + L * S);
		else
			rm2 = (float)(L + S - L * S);
		
		rm1 = (float)(2.0f * L - rm2);
		
		R = HueToRGB(rm1, rm2, (float)(H + 120.0f));
		G = HueToRGB(rm1, rm2, (float)(H));
		B = HueToRGB(rm1, rm2, (float)(H - 120.0f));
	}
	
	return RGB(R, G, B);
}

double CBCGPDrawManager::MakeHue_ONE (double H)
{
	H -= (int)(H);
	if (H < 0.0)
	{
		H += 1.0;
	}

	return H;
}

double CBCGPDrawManager::MakeHue_TWO (double H)
{
	return MakeHue_ONE (H / 360.0) * 360.0;
}

void CBCGPDrawManager::RGBtoHSL( COLORREF rgb, double *H, double *S, double *L )
{   
	double delta;
	double r = (double)GetRValue(rgb)/255;
	double g = (double)GetGValue(rgb)/255;
	double b = (double)GetBValue(rgb)/255;   
	double cmax = max(r, max(g, b));
	double cmin = min(r, min(g, b));   
	*L=(cmax+cmin)/2.0;   
	
	if(cmax==cmin) 
	{
		*S = 0;      
		*H = 0; // it's really undefined   
	} 
	else 
	{
		if(*L < 0.5) 
			*S = (cmax-cmin)/(cmax+cmin);      
		else
			*S = (cmax-cmin)/(2.0-cmax-cmin);      
		
		delta = cmax - cmin;
		if(r==cmax) 
			*H = (g-b)/delta;      
		else if(g==cmax)
			*H = 2.0 +(b-r)/delta;
		else          
			*H=4.0+(r-g)/delta;
		*H /= 6.0; 
		if(*H < 0.0)
			*H += 1;  
	}
}

void CBCGPDrawManager::RGBtoHSV (COLORREF rgb, double *H, double *S, double *V)
// Algoritm by A. R. Smith 
{
	double r = (double) GetRValue (rgb) / 255;
	double g = (double) GetGValue (rgb) / 255;
	double b = (double) GetBValue (rgb) / 255;

	double dblMin = min (r, min (g, b));
	double dblMax = max (r, max (g, b));

	*V = dblMax;				// v
	double delta = dblMax - dblMin;

	if( dblMax != 0 )
		*S = delta / dblMax;		// s
	else {
		// r = g = b = 0		// s = 0, v is undefined
		*S = 0;
		*H = -1;
		return;
	}

	if (delta == 0.)
	{
		*H = 1;
	}
	else
	{
		if (r == dblMax)
			*H = (g - b) / delta;		// between yellow & magenta
		else if( g == dblMax )
			*H = 2 + ( b - r ) / delta;	// between cyan & yellow
		else
			*H = 4 + ( r - g ) / delta;	// between magenta & cyan
	}

	*H *= 60;				// degrees

	if (*H < 0)
		*H += 360;
}

COLORREF CBCGPDrawManager::HSVtoRGB (double h, double s, double v)
// Algoritm by A. R. Smith
{
	int i;
	double f, p, q, t;
	double r, g, b;

	if( s == 0 ) 
	{
		// achromatic (grey)
		r = g = b = v;
	}
	else
	{
		h /= 60;			// sector 0 to 5
		i = (int) floor( h );
		f = h - i;			// factorial part of h
		p = v * ( 1 - s );
		q = v * ( 1 - s * f );
		t = v * ( 1 - s * ( 1 - f ) );

		switch( i ) 
		{
		case 0:
			r = v;
			g = t;
			b = p;
			break;
		case 1:
			r = q;
			g = v;
			b = p;
			break;
		case 2:
			r = p;
			g = v;
			b = t;
			break;
		case 3:
			r = p;
			g = q;
			b = v;
			break;
		case 4:
			r = t;
			g = p;
			b = v;
			break;
		default:		// case 5:
			r = v;
			g = p;
			b = q;
			break;
		}
	}

	return RGB (
		(int) (.5 + r * 255),
		(int) (.5 + g * 255),
		(int) (.5 + b * 255));
}

COLORREF CBCGPDrawManager::SmartMixColors (COLORREF color1, COLORREF color2,
		double dblLumRatio, int k1, int k2)
{
	ASSERT (k1 >= 0);
	ASSERT (k2 >= 0);

	if (k1 + k2 == 0)
	{
		ASSERT (FALSE);
		return RGB (0, 0, 0);
	}

	COLORREF color = RGB (
		min(255, (GetRValue (color1) * k1 + GetRValue (color2) * k2) / (k1 + k2)),
		min(255, (GetGValue (color1) * k1 + GetGValue (color2) * k2) / (k1 + k2)),
		min(255, (GetBValue (color1) * k1 + GetBValue (color2) * k2) / (k1 + k2)));

	double h1, s1, v1;
	RGBtoHSV (color, &h1, &s1, &v1);

	double h2, s2, v2;
	RGBtoHSV (color2, &h2, &s2, &v2);

	v1 = v2;
	s1 = (s1 *  k1 + s2 *  k2) /  (k1 + k2);

	color = HSVtoRGB (h1, s1, v1);

	if (dblLumRatio != 1.)
	{
		double H, S, L;
		RGBtoHSL (color, &H, &S, &L);

		color = HLStoRGB_ONE (H, min (1., L * dblLumRatio), S);
	}

	return color;
}

COLORREF CBCGPDrawManager::MixColors (COLORREF clr1, COLORREF clr2, double dblRatio)
{
    ASSERT (dblRatio >= 0.0f && dblRatio <= 1.0f);

    return RGB (
        min(255, GetRValue(clr1) + dblRatio * (GetRValue(clr2) - GetRValue(clr1))),
        min(255, GetGValue(clr1) + dblRatio * (GetGValue(clr2) - GetGValue(clr1))),
        min(255, GetBValue(clr1) + dblRatio * (GetBValue(clr2) - GetBValue(clr1)))
        );
} 

void CBCGPDrawManager::DrawAlpha (CDC* pDstDC, const CRect& rectDst, CDC* pSrcDC, const CRect& rectSrc,
								  BYTE nOpacity)
{
	BLENDFUNCTION pixelblend = { AC_SRC_OVER, 0, 
		nOpacity, 1 /*AC_SRC_ALPHA*/};

#ifndef _BCGSUITE_
	if (CBCGPToolBarImages::m_pfAlphaBlend != NULL && !globalData.bIsWindows9x)
	{
		(*CBCGPToolBarImages::m_pfAlphaBlend) (pDstDC->m_hDC, 
			rectDst.left, rectDst.top,
			rectDst.Width(), rectDst.Height(), 
			pSrcDC->m_hDC, rectSrc.left, rectSrc.top,
			rectSrc.Width(), rectSrc.Height(), 
			pixelblend);
	}
	else
	{
		if (rectSrc.Width () != rectDst.Width () ||
			rectSrc.Height () != rectDst.Height ())
		{
			pDstDC->StretchBlt(rectDst.left, rectDst.top,
				rectDst.Width(), rectDst.Height(), 
				pSrcDC, rectSrc.left, rectSrc.top,
				rectSrc.Width(), rectSrc.Height(), SRCCOPY);
		}
		else
		{
			pDstDC->BitBlt (rectDst.left, rectDst.top, rectDst.Width(), rectDst.Height(), 
				pSrcDC, rectSrc.left, rectSrc.top, SRCCOPY);
		}
	}
#else
	pDstDC->AlphaBlend (rectDst.left, rectDst.top,
			rectDst.Width(), rectDst.Height(), 
			pSrcDC, rectSrc.left, rectSrc.top,
			rectSrc.Width(), rectSrc.Height(), 
			pixelblend);
#endif
}
//***********************************************************************************************************
void CBCGPDrawManager::FillAlpha (const CRect& rect, BYTE bValue/* = 255*/)
{
	const int cx = rect.Width ();
	const int cy = rect.Height ();

	COLORREF* pBits = NULL;
	HBITMAP hmbpDib = CreateBitmap_32 (CSize (cx, cy), (LPVOID*)&pBits);

	if (hmbpDib == NULL || pBits == NULL)
	{
		return;
	}

	CDC dcMem;
	dcMem.CreateCompatibleDC (&m_dc);

	HBITMAP hbmpOld = (HBITMAP) dcMem.SelectObject (hmbpDib);

	dcMem.BitBlt (0, 0, cx, cy, &m_dc, rect.left, rect.top, SRCCOPY);

	const DWORD dwMask = (bValue << 24) & 0xFF000000;

	for (int i = 0; i < cx * cy; i++)
	{
		*pBits |= dwMask;
		pBits++;
	}

	m_dc.BitBlt (rect.left, rect.top, cx, cy, &dcMem, 0, 0, SRCCOPY);

	dcMem.SelectObject (hbmpOld);
	DeleteObject (hmbpDib);
}
//***********************************************************************************************************
void CBCGPDrawManager::FillAlpha(const CRect& rect, HBITMAP hBmp, BYTE bValue)
{
	if (hBmp == NULL || rect.IsRectEmpty ())
	{
		return;
	}

	CSize size(rect.Size());

	CBCGPScanlinerBitmap scan;
	scan.Attach(hBmp, rect.TopLeft ());

	for (int y = 0; y < size.cy; y++)
	{
		LPBYTE pRow = scan.Get() + 3;
		for (int x = 0; x < size.cx; x++)
		{
			*pRow = bValue;
			pRow += 4;
		}

		scan++;
	}
}
//***********************************************************************************************************
void CBCGPDrawManager::FillTransparentAlpha(const CRect& rect, HBITMAP hBmp, COLORREF clrSrc)
{
	if (hBmp == NULL || rect.IsRectEmpty ())
	{
		return;
	}

	CSize size(rect.Size());

	CBCGPScanlinerBitmap scan;
	scan.Attach(hBmp, rect.TopLeft ());

	COLORREF clr = RGB (GetBValue (clrSrc),
						GetGValue (clrSrc),
						GetRValue (clrSrc));

	for (int y = 0; y < size.cy; y++)
	{
		DWORD* pRow = (DWORD*)(LPBYTE)scan.Get();

		for (int x = 0; x < size.cx; x++)
		{
			if (clr == ((*pRow) & 0xFFFFFF))
			{
				*pRow = 0;
			}

			pRow++;
		}

		scan++;
	}
}
//***********************************************************************************************************
void CBCGPDrawManager::DrawRotated (CRect rect, CDC& dcSrc, BOOL bClockWise)
{
	const int cx = rect.Width ();
	const int cy = rect.Height ();

	if (cx <= 0 || cy <= 0)
	{
		return;
	}

	for (int y = 0; y <= cy; y++)
	{
		for (int x = 0; x <= cx; x++)
		{
			int xSrc = y;
			int ySrc = bClockWise ? (cx - x - 1) : x;

			m_dc.SetPixel (rect.left + x, rect.top + y, dcSrc.GetPixel (xSrc, ySrc));
		}
	}
}

HBITMAP CBCGPDrawManager::PrepareShadowMask (int nDepth,
											 COLORREF clrBase,
                                             int iMinBrightness/* = 0*/,
											 int iMaxBrightness/* = 100*/,
											 double dblSmooth/* = 1.0*/,
											 double dblDarkRatio/* = 0.25*/)
{
	if (nDepth == 0)
	{
		return NULL;
	}

	if (dblSmooth <= 0.0)
	{
		dblSmooth = 1.0;
	}

	if (clrBase == (COLORREF)-1)
	{
		clrBase = RGB(192, 192, 192);
	}

	int nSize     = nDepth < 3 ? 3 : nDepth;
	int nDestSize = nSize * 2 + 1;

	LPBYTE lpBits = NULL;
	HBITMAP hBitmap = CreateBitmap_32 (CSize (nDestSize, nDestSize), (void**)&lpBits);

	if (hBitmap == NULL || lpBits == NULL)
	{
		return NULL;
	}

	// Create mask

	const double minValue  = iMinBrightness / 100.0;
	const double maxValue  = iMaxBrightness / 100.0;

	const long size2       = nDestSize / 2;
	const double size2S    = bcg_sqr(nDestSize) / 4.0;
	const double tolerance = bcg_round(size2S);
	const double variance  = -2.0 / size2S * dblSmooth;
	const double delta     = (maxValue - minValue);

	const double r = dblDarkRatio * GetRValue (clrBase);
	const double g = dblDarkRatio * GetGValue (clrBase);
	const double b = dblDarkRatio * GetBValue (clrBase);

	LPRGBQUAD pQuad = (LPRGBQUAD)lpBits;

	for(long y = -size2; y <= size2; y++)
	{
		double y2 = y * y;

		for(long x = -size2; x <= size2; x++)
		{
			double d = y2 + x * x;
			if(d <= tolerance)
			{
				double e = bcg_clamp (exp (d * variance) * delta + minValue, minValue, maxValue);

				pQuad->rgbRed      = (BYTE)bcg_clamp_to_byte(e * r);
				pQuad->rgbGreen    = (BYTE)bcg_clamp_to_byte(e * g);
				pQuad->rgbBlue     = (BYTE)bcg_clamp_to_byte(e * b);
				pQuad->rgbReserved = (BYTE)bcg_clamp_to_byte(e * 255);
			}

			pQuad++;
		}
	}

	return hBitmap;
}

void CBCGPDrawManager::DrawGlassButtonBorder (CDC& dc, CRect rect,
				COLORREF clrGlow1, COLORREF clrGlow2, COLORREF clrDark1, COLORREF clrDark2)
{
	if (rect.Height () <= 0)
	{
		return;
	}

	dc.FillSolidRect (rect.left, rect.top, rect.Width (), 1, clrGlow1);

	LONG cyMid = (rect.bottom + rect.top) / 2;
	for (LONG cy = rect.top + 1; cy < rect.bottom - 1; ++cy)
	{
		double dRatio;
		COLORREF clr;
		if (cy < cyMid) // Glow part
		{
			dRatio = 2.0 * (cy - rect.top) / rect.Height ();
			clr = CBCGPDrawManager::MixColors (clrGlow1, clrGlow2, dRatio);
		}
		else // Dark part
		{
			dRatio = 2.0 * (cy - cyMid) / rect.Height ();
			clr = CBCGPDrawManager::MixColors (clrDark1, clrDark2, dRatio);
		}

		dc.SetPixel (rect.left, cy, clr);
		dc.SetPixel (rect.right - 1, cy, clr);
	}
	dc.FillSolidRect (rect.left, rect.bottom - 1, rect.Width (), 1, clrDark2);
}

void CBCGPDrawManager::DrawGlassButtonBackround (CDC& dc, CRect rect,
				COLORREF clrGlow1, COLORREF clrGlow2, COLORREF clrDark1, COLORREF clrDark2)
{
	if (rect.Height () <= 0)
	{
		return;
	}

	LONG cyMid = (rect.bottom + rect.top) / 2;

	for (LONG cy = rect.top; cy < rect.bottom; ++cy)
	{
		double dRatio;
		COLORREF clr;
		if (cy < cyMid) // Glow part
		{
			dRatio = 2.0 * (cy - rect.top) / rect.Height ();
			clr = CBCGPDrawManager::MixColors (clrGlow1, clrGlow2, dRatio);
		}
		else // Dark part
		{
			dRatio = 2.0 * (cy - cyMid) / rect.Height ();
			clr = CBCGPDrawManager::MixColors (clrDark1, clrDark2, dRatio);
		}

		dc.FillSolidRect (rect.left, cy, rect.Width (), 1, clr);
	}
}

BOOL CBCGPDrawManager::IsDarkColor(COLORREF clr)
{
	if (clr == 0)
	{
		return TRUE;
	}
	
	double H;
	double S;
	double L;
	
	RGBtoHSL (clr, &H, &S, &L);
	return L < 0.3;
}

BOOL CBCGPDrawManager::IsLightColor(COLORREF clr)
{
	if (clr == 0)
	{
		return FALSE;
	}
	
	double H;
	double S;
	double L;
	
	RGBtoHSL (clr, &H, &S, &L);
	return L >= 0.7;
}

BOOL CBCGPDrawManager::IsPaleColor(COLORREF clr)
{
	if (clr == 0)
	{
		return FALSE;
	}
	
	double H;
	double S;
	double L;
	
	RGBtoHSL (clr, &H, &S, &L);
	return L >= 0.9;
}

COLORREF CBCGPDrawManager::ColorMakeLighter(COLORREF clr, double dblRatio)
{
	double H;
	double S;
	double L;
	double dblMin = 0.01;
	
	RGBtoHSL (clr, &H, &S, &L);

	if (H < dblMin && S < dblMin && L < dblMin)
	{
		int nVal = bcg_round(dblRatio * 255.0);
		return RGB(nVal, nVal, nVal);
	}
	else
	{
		dblRatio += 1.;
		
		return HLStoRGB_ONE (
			H,
			min (1., L * dblRatio),
			min (1., S * dblRatio));
	}
}

COLORREF CBCGPDrawManager::ColorMakeDarker(COLORREF clr, double dblRatio)
{
	double H;
	double S;
	double L;
	
	dblRatio = max(0., 1. - dblRatio);
	
	RGBtoHSL (clr, &H, &S, &L);
	
	clr = HLStoRGB_ONE (
		H,
		min (1., L * dblRatio),
		min (1., S * dblRatio));

	return clr;
}

COLORREF CBCGPDrawManager::ColorMakePale(COLORREF clr, double dblLum)
{
	double H;
	double S;
	double L;
	
	RGBtoHSL (clr, &H, &S, &L);
	return HLStoRGB_ONE(H, dblLum, S);
}

COLORREF CBCGPDrawManager::GetInterlaceColor(COLORREF clr)
{
	if (IsPaleColor(clr))
	{
		return ColorMakeDarker(clr, .02);
	}
	else
	{
		return ColorMakeLighter(clr, .1);
	}
}

// CBCGPPenSelector class implementation

CBCGPPenSelector::CBCGPPenSelector (CDC& dc, CPen* pPen)
	: m_dc (dc), m_pOldPen (NULL)
{
	if (pPen != NULL)
	{
		m_pOldPen = m_dc.SelectObject (pPen);
	}
}

CBCGPPenSelector::CBCGPPenSelector (CDC& dc, COLORREF color, UINT nWidth /*= 1*/, DWORD dwStyle /*= PS_SOLID*/)
	: m_dc (dc), m_pOldPen (NULL)
{
	m_pen.CreatePen (dwStyle, nWidth, color);
	m_pOldPen = m_dc.SelectObject (&m_pen);
}

CBCGPPenSelector::~CBCGPPenSelector ()
{
	if (m_pOldPen != NULL)
	{
		m_dc.SelectObject (m_pOldPen);
	}
}

// CBCGPBrushSelector class implementation

CBCGPBrushSelector::CBCGPBrushSelector (CDC& dc, CBrush* pBrush)
	: m_dc (dc), m_pOldBrush (NULL)
{
	if (pBrush != NULL)
	{
		m_pOldBrush = m_dc.SelectObject (pBrush);
	}
}

CBCGPBrushSelector::CBCGPBrushSelector (CDC& dc, COLORREF color)
	: m_dc (dc)
{
	m_brush.CreateSolidBrush (color);
	m_pOldBrush = m_dc.SelectObject (&m_brush);
}

CBCGPBrushSelector::~CBCGPBrushSelector ()
{
	if (m_pOldBrush != NULL)
	{
		m_dc.SelectObject (m_pOldBrush);
	}
}

// CBCGPFontSelector class implementation

CBCGPFontSelector::CBCGPFontSelector (CDC& dc, CFont* pFont)
	: m_dc (dc), m_pOldFont (NULL)
{
	if (pFont == NULL)
	{
		pFont = &globalData.fontRegular;
	}
	if (pFont != NULL)
	{
		m_pOldFont = m_dc.SelectObject (pFont);
	}
}

CBCGPFontSelector::CBCGPFontSelector (CDC& dc, int nPointSize, LPCTSTR lpszFaceName)
	: m_dc (dc), m_pOldFont (NULL)
{
	m_font.CreatePointFont (nPointSize, lpszFaceName);
	m_pOldFont = m_dc.SelectObject (&m_font);
}

CBCGPFontSelector::CBCGPFontSelector (CDC& dc, const LOGFONT* pLogFont)
	: m_dc (dc), m_pOldFont (NULL)
{
	m_font.CreatePointFontIndirect (pLogFont);
	m_pOldFont = m_dc.SelectObject (&m_font);
}

CBCGPFontSelector::~CBCGPFontSelector ()
{
	if (m_pOldFont != NULL)
	{
		ASSERT_VALID (m_pOldFont);
		m_dc.SelectObject (m_pOldFont);
	}
}

// CBCGPAlphaDC class implementation

CBCGPAlphaDC::CBCGPAlphaDC (CDC& originalDC, double dblOpacity)
    : m_hOriginalDC (originalDC.m_hDC)
    , m_hBufferBitmap(NULL)
    , m_hTempBitmap(NULL)
    , m_nOpacity((BYTE)(dblOpacity * 255))
    , m_pImageBits(NULL)
	, m_bIgnoreAlpha(FALSE)
{
    ::GetClipBox(originalDC, m_rcClipBox);
    Initialize();
}

CBCGPAlphaDC::CBCGPAlphaDC (CDC& originalDC, const CRect& rcArea, double dblOpacity)
    : m_hOriginalDC(originalDC.m_hDC)
    , m_hBufferBitmap(NULL)
    , m_hTempBitmap(NULL)
    , m_rcClipBox(rcArea)
    , m_nOpacity((BYTE)(dblOpacity * 255))
    , m_pImageBits(NULL)
	, m_bIgnoreAlpha(FALSE)
{
    Initialize();
}

void CBCGPAlphaDC::Initialize ()
{
    Attach (::CreateCompatibleDC (m_hOriginalDC));

    CRect rcClip = m_rcClipBox;
    ::LPtoDP (m_hOriginalDC, (POINT*)&rcClip, 2);

	if (rcClip.IsRectEmpty())
	{
		rcClip.right = rcClip.left + 1;
		rcClip.bottom = rcClip.top + 1;
	}

    m_hBufferBitmap = CBCGPDrawManager::CreateBitmap_32 (rcClip.Size (), (void**)&m_pImageBits);
    if (m_pImageBits == NULL)
    {
        return;
    }

    for (int i = 0; i < rcClip.Width () * rcClip.Height (); ++i)
    {
        m_pImageBits[i] = (DWORD)(255 << 24);
    }

    m_hTempBitmap = (HBITMAP)::SelectObject (m_hDC, m_hBufferBitmap);
    ::SetMapMode (m_hDC, ::GetMapMode (m_hOriginalDC));

    SIZE szTemp;
    ::GetWindowExtEx (m_hOriginalDC, &szTemp);
    ::SetWindowExtEx (m_hDC, szTemp.cx, szTemp.cy, NULL);
    ::GetViewportExtEx (m_hOriginalDC, &szTemp);
    ::SetViewportExtEx (m_hDC, szTemp.cx, szTemp.cy, NULL);
    ::SetWindowOrgEx (m_hDC, m_rcClipBox.left, m_rcClipBox.top, NULL);
}

CBCGPAlphaDC::~CBCGPAlphaDC ()
{
    if (m_nOpacity > 255)
    {
        m_nOpacity = 255;
    }

	if (!m_rcClipBox.IsRectEmpty ())
	{
		
		if (m_nOpacity > 0)
		{
			const DWORD alphaMask = 0xFF000000;
			int nPixels = m_rcClipBox.Width () * m_rcClipBox.Height ();
			
			BOOL bIsOSAlphaBlendingSupport = TRUE;
			
#if _MSC_VER < 1600
			bIsOSAlphaBlendingSupport = globalData.bIsOSAlphaBlendingSupport;
#endif
			if (m_bIgnoreAlpha || !bIsOSAlphaBlendingSupport)
			{
				for (int i = 0; i < nPixels; ++i)
				{
					m_pImageBits[i] = alphaMask | m_pImageBits[i];
				}
			}
			else
			{
				// invert alpha
				for (int i = 0; i < nPixels; ++i)
				{
					DWORD rgb = (m_pImageBits[i] & ~alphaMask);
					m_pImageBits[i] = (((alphaMask - (m_pImageBits[i] & alphaMask))) & alphaMask) | rgb;
				}
			}
			
			CBCGPDrawManager::DrawAlpha (CDC::FromHandle (m_hOriginalDC), m_rcClipBox, 
				this, m_rcClipBox, m_nOpacity);
		}
	}

    ::SelectObject (m_hDC, m_hTempBitmap);
    ::DeleteObject (m_hBufferBitmap);
    ::DeleteDC (Detach ());
}

void CBCGPAlphaDC::IgnoreSourceAlpha ()
{
	m_bIgnoreAlpha = TRUE;
}

