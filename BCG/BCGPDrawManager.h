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
// BCGDrawManager.h: interface for the CBCGPDrawManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPDRAWMANAGER_H__56EC7162_B29E_11D4_95C8_00A0C9289F1B__INCLUDED_)
#define AFX_BCGPDRAWMANAGER_H__56EC7162_B29E_11D4_95C8_00A0C9289F1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

class BCGCBPRODLLEXPORT CBCGPDrawManager : public CObject
{
public:
	CBCGPDrawManager(CDC& dc);
	virtual ~CBCGPDrawManager();

	CDC& GetDC() const
	{
		return m_dc;
	}

// Operations:
	BOOL DrawShadow (CRect rect,					// Shadow will be draw around this rect
					int nDepth,						// Shadow depth (pixels)
					int iMinBrightness = 100,		// Min. brighttness
					int iMaxBrightness = 50,		// Max. beightness
					CBitmap* pBmpSaveBottom = NULL,	// Bitmap may be saved in 
					CBitmap* pBmpSaveRight = NULL,	// two bitmaps
					COLORREF clrBase =(COLORREF)-1,	// Base color
					BOOL bRightShadow = TRUE);		// Shadow on right side

	BOOL HighlightRect (CRect rect,					// Rectangle to be highlighted
						int nPercentage = -1,		// < 100 - darker, > 100 - lighter, -1 - default light
						COLORREF clrTransparent =	// Don't change this color
							(COLORREF)-1,
						int nTolerance = 0,
						COLORREF clrBlend = (COLORREF)-1);

	void FillGradient (	CRect rect, 
						COLORREF colorStart, COLORREF colorFinish,
						BOOL bHorz = TRUE,
						int nStartFlatPercentage = 0,
						int nEndFlatPercentage = 0);

	void FillGradient2 (CRect rect, COLORREF colorStart, COLORREF colorFinish, 
						int nAngle = 0 /* 0 - 360 */);

	void Fill4ColorsGradient (CRect rect, 
						COLORREF colorStart1, COLORREF colorFinish1, 
						COLORREF colorStart2, COLORREF colorFinish2,
						BOOL bHorz = TRUE,
						int nPercentage = 50);	/* 0 - 100 */

	BOOL GrayRect (	CRect rect,					// Rectangle to be grayed
					int nPercentage = -1,		// < 100 - darker, > 100 - lighter, -1 - default light
					COLORREF clrTransparent =	// Don't change this color
							(COLORREF)-1,
					COLORREF clrDisabled =		// Use this color for desaturation
							(COLORREF)-1);

	void MirrorRect (CRect rect, BOOL bHorz = TRUE);

	BOOL DrawGradientRing (	CRect rect,
							COLORREF colorStart, COLORREF colorFinish,
							COLORREF colorBorder,
							int nAngle /* 0 - 360 */,
							int nWidth,
							COLORREF clrFace = (COLORREF) -1);

	void DrawLine (int x1, int y1, int x2, int y2, COLORREF clrLine, int nPenStyle = PS_SOLID);
	void DrawLineA (double x1, double y1, double x2, double y2, COLORREF clrLine, int nPenStyle = PS_SOLID);
	void DrawEllipse   (const CRect& rect, COLORREF clrFill, COLORREF clrLine, int nPenStyle = PS_SOLID);
	void DrawRect (const CRect& rect, COLORREF clrFill, COLORREF clrLine);
	void DrawArc(const CRect& rect, double dblStartAngle, double dblFinishAngle, BOOL bIsClockwise, COLORREF clrLine, int nPenStyle = PS_SOLID);
	void DrawFocusRect(const CRect& rectFocus);
	
	static void DrawAlpha (CDC* pDstDC, const CRect& rectDst, CDC* pSrcDC, const CRect& rectSrc, BYTE nOpacity = 255);

	void FillAlpha (const CRect& rect, BYTE bValue = 255);
	void DrawRotated (CRect rectDest, CDC& dcSrc, BOOL bClockWise);

// Helpers:
	static COLORREF PixelAlpha (COLORREF srcPixel, int percent);
	static COLORREF PixelAlpha (COLORREF srcPixel, double percentR, double percentG, double percentB);
	static COLORREF PixelAlpha (COLORREF srcPixel, COLORREF dstPixel, int percent);
	static void SetAlphaPixel (COLORREF* pBits, CRect rect, int x, int y, int percent, int iShadowSize,
		COLORREF clrBase = (COLORREF)-1, BOOL bIsRight = TRUE);
	static void SetPixel (COLORREF* pBits, int cx, int cy, int x, int y, COLORREF color);

// Conversion between the HSL (Hue, Saturation, and Luminosity) and RGB:
	static double HuetoRGB (double m1, double m2, double h);
	static BYTE HueToRGB (float rm1, float rm2, float rh);
	static COLORREF HLStoRGB_ONE (double H, double L, double S);
	static COLORREF HLStoRGB_TWO (double H, double L, double S);
	static void RGBtoHSL (COLORREF rgb, double *H, double *S, double *L);
	static double MakeHue_ONE (double H);
	static double MakeHue_TWO (double H);

// Color darkness/lightness:
	static BOOL IsDarkColor(COLORREF clr);
	static BOOL IsLightColor(COLORREF clr);
	static BOOL IsPaleColor(COLORREF clr);
	static COLORREF ColorMakeLighter(COLORREF clr, double dblRatio = .1);
	static COLORREF ColorMakeDarker(COLORREF clr, double dblRatio = .1);
	static COLORREF ColorMakePale(COLORREF clr, double dblLum = .97);
	static COLORREF GetInterlaceColor(COLORREF clr);

// Conversion between the HSV (Hue/Saturation/Value) and RGB:
	static void RGBtoHSV (COLORREF rgb, double *H, double *S, double *V);
	static COLORREF HSVtoRGB (double H, double S, double V);

	static COLORREF SmartMixColors (COLORREF color1, COLORREF color2,
		double dblLumRatio = 1., int k1 = 1, int k2 = 1);

	static COLORREF MixColors (COLORREF clr1, COLORREF clr2, double dblRatio);

	static HBITMAP CreateBitmap_24 (const CSize& size, void** pBits);
	static HBITMAP CreateBitmap_24 (HBITMAP bitmap);

	static HBITMAP CreateBitmap_32 (const CSize& size, void** pBits);
	static HBITMAP CreateBitmap_32 (HBITMAP bitmap, COLORREF clrTransparent = -1);

	static HBITMAP PrepareShadowMask (int nDepth,
                                      COLORREF clrBase,
                                      int iMinBrightness = 0,
									  int iMaxBrightness = 100,
									  double dblSmooth = 1.0,
									  double dblDarkRatio = 0.25);

	static void DrawGlassButtonBorder (CDC& dc, CRect rect,
				COLORREF clrGlow1, COLORREF clrGlow2, COLORREF clrDark1, COLORREF clrDark2);

	static void DrawGlassButtonBackround (CDC& dc, CRect rect,
				COLORREF clrGlow1, COLORREF clrGlow2, COLORREF clrDark1, COLORREF clrDark2);

	static void FillAlpha(const CRect& rect, HBITMAP hBmp, BYTE bValue);

	static void FillTransparentAlpha(const CRect& rect, HBITMAP hBmp, COLORREF clrTransparent);

protected:
	void _FillGradient (CRect rect, 
						COLORREF colorStart, COLORREF colorFinish,
						BOOL bHorz = TRUE,
						int nStartFlatPercentage = 0,
						int nEndFlatPercentage = 0);

// Attributes:
protected:
	CDC&	m_dc;
};

class BCGCBPRODLLEXPORT CBCGPPenSelector
{
public:
    CBCGPPenSelector (CDC& dc, CPen* pPen);
    CBCGPPenSelector (CDC& dc, COLORREF color, UINT nWidth = 1, DWORD dwStyle = PS_SOLID);
    ~CBCGPPenSelector ();

protected:
    CDC&	m_dc;
    CPen*	m_pOldPen;
	CPen	m_pen;
};

class BCGCBPRODLLEXPORT CBCGPBrushSelector
{
public:
    CBCGPBrushSelector (CDC& dc, CBrush* pBrush);
    CBCGPBrushSelector (CDC& dc, COLORREF color);
    ~CBCGPBrushSelector ();

protected:
    CDC&    m_dc;
    CBrush* m_pOldBrush;
	CBrush	m_brush;
};

class BCGCBPRODLLEXPORT CBCGPFontSelector
{
public:
    CBCGPFontSelector (CDC& dc, CFont* pFont);
    CBCGPFontSelector (CDC& dc, int nPointSize, LPCTSTR lpszFaceName);
    CBCGPFontSelector (CDC& dc, const LOGFONT* pLogFont);
    ~CBCGPFontSelector ();

protected:
    CDC&	m_dc;
    CFont*	m_pOldFont;
	CFont	m_font;
};

class BCGCBPRODLLEXPORT CBCGPAlphaDC : public CDC
{
public:
    CBCGPAlphaDC (CDC& originalDC, double dblOpacity = 1.0f);
    CBCGPAlphaDC (CDC& originalDC, const CRect& area, double dblOpacity = 1.0f);

	void IgnoreSourceAlpha ();

    ~CBCGPAlphaDC ();

protected:
    HDC     m_hOriginalDC;
    CRect   m_rcClipBox;
    HBITMAP m_hBufferBitmap;
    HBITMAP m_hTempBitmap;
    BYTE    m_nOpacity;
	BOOL	m_bIgnoreAlpha;

    COLORREF* m_pImageBits;

    void Initialize ();
};

#endif // !defined(AFX_BCGPDRAWMANAGER_H__56EC7162_B29E_11D4_95C8_00A0C9289F1B__INCLUDED_)
