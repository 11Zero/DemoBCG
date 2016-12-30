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
// BCGPGraphicsManagerD2D.h: interface for the CBCGPGraphicsManagerD2D class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPGRAPHICSMANAGERD2D_H__847B8B25_AE4A_44E1_B1E8_CFE353B89B48__INCLUDED_)
#define AFX_BCGPGRAPHICSMANAGERD2D_H__847B8B25_AE4A_44E1_B1E8_CFE353B89B48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPGraphicsManager.h"

interface ID2D1RenderTarget;
interface ID2D1DCRenderTarget;
interface ID2D1BitmapRenderTarget;
interface ID2D1Factory;
interface IDWriteFactory;
interface IDWriteGdiInterop;
interface IWICImagingFactory;
interface ID2D1Layer;
interface ID2D1StrokeStyle;
interface ID2D1Brush;
interface ID2D1Bitmap;
interface IDWriteTextFormat;

typedef enum BCGP_D2D1_FACTORY_TYPE
{
	BCGP_D2D1_FACTORY_TYPE_SINGLE_THREADED = 0,
	BCGP_D2D1_FACTORY_TYPE_MULTI_THREADED = 1,
	BCGP_D2D1_FACTORY_TYPE_FORCE_DWORD = 0xffffffff

} BCGP_D2D1_FACTORY_TYPE;

enum BCGP_DWRITE_FACTORY_TYPE
{
    BCGP_DWRITE_FACTORY_TYPE_SHARED,
    BCGP_DWRITE_FACTORY_TYPE_ISOLATED
};

class BCGCBPRODLLEXPORT CBCGPGraphicsManagerD2D : public CBCGPGraphicsManager  
{
	friend struct BCGPGLOBAL_DATA;

	DECLARE_DYNCREATE(CBCGPGraphicsManagerD2D)

public:
	CBCGPGraphicsManagerD2D(CDC* pDC = NULL, BOOL bDoubleBuffering = TRUE, CBCGPGraphicsManagerParams* pParams = NULL);
	virtual ~CBCGPGraphicsManagerD2D();

	virtual CBCGPGraphicsManager* CreateOffScreenManager(const CBCGPRect& rect, CBCGPImage* pImageDest);

protected:
	CBCGPGraphicsManagerD2D(const CBCGPRect& rectDest, CBCGPImage* pImageDest);

// Attributes:
public:
	virtual BOOL IsValid() const;

	virtual void BindDC(CDC* pDC, BOOL bDoubleBuffering = TRUE);
	virtual void BindDC(CDC* pDC, const CRect& rect);

	static BOOL m_bCheckLineOffsets;
	static BOOL m_bForceSoftwareRendering;
	static BOOL m_bUseFontConversion;	// Convert font names using GDI interop

// overrides:
public:
	virtual BOOL BeginDraw();
	virtual void EndDraw();

	virtual void Clear(const CBCGPColor& color = CBCGPColor());

	virtual void DrawLine(	
		const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo, const CBCGPBrush& brush, 
		double lineWidth = 1., const CBCGPStrokeStyle* pStrokeStyle = NULL);

	virtual void DrawLines(	
		const CBCGPPointsArray& arPoints, const CBCGPBrush& brush, 
		double lineWidth = 1., const CBCGPStrokeStyle* pStrokeStyle = NULL);

	virtual void DrawScatter(
		const CBCGPPointsArray& arPoints, const CBCGPBrush& brush, double dblPointSize, UINT nStyle = 0);

	virtual void DrawRectangle(
		const CBCGPRect& rect, const CBCGPBrush& brush, 
		double lineWidth = 1., const CBCGPStrokeStyle* pStrokeStyle = NULL);

	virtual void FillRectangle(
		const CBCGPRect& rect, const CBCGPBrush& brush);

	virtual void DrawRoundedRectangle(
		const CBCGPRoundedRect& rect, const CBCGPBrush& brush, 
		double lineWidth = 1., const CBCGPStrokeStyle* pStrokeStyle = NULL);

	virtual void FillRoundedRectangle(
		const CBCGPRoundedRect& rect, const CBCGPBrush& brush);

	virtual void DrawEllipse(
		const CBCGPEllipse& ellipse, const CBCGPBrush& brush, 
		double lineWidth = 1., const CBCGPStrokeStyle* pStrokeStyle = NULL);

	virtual void FillEllipse(
		const CBCGPEllipse& ellipse, const CBCGPBrush& brush);

	virtual void DrawArc(
		const CBCGPEllipse& ellipse, double dblStartAngle, double dblFinishAngle, BOOL bIsClockwise,
		const CBCGPBrush& brush, double lineWidth = 1., const CBCGPStrokeStyle* pStrokeStyle = NULL);

	virtual void DrawArc(
		const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo, const CBCGPSize sizeRadius, 
		BOOL bIsClockwise, BOOL bIsLargeArc,
		const CBCGPBrush& brush, double lineWidth = 1., const CBCGPStrokeStyle* pStrokeStyle = NULL);

	virtual void DrawGeometry(
		const CBCGPGeometry& geometry, const CBCGPBrush& brush, 
		double lineWidth = 1., const CBCGPStrokeStyle* pStrokeStyle = NULL);

	virtual void FillGeometry(
		const CBCGPGeometry& geometry, const CBCGPBrush& brush);

	virtual void DrawImage(
		const CBCGPImage& image, const CBCGPPoint& ptDest, const CBCGPSize& sizeDest = CBCGPSize(),
		double opacity = 1., CBCGPImage::BCGP_IMAGE_INTERPOLATION_MODE interpolationMode = CBCGPImage::BCGP_IMAGE_INTERPOLATION_MODE_LINEAR, 
		const CBCGPRect& rectSrc = CBCGPRect());

	virtual void DrawText(
		const CString& strText, const CBCGPRect& rectText, const CBCGPTextFormat& textFormat,
		const CBCGPBrush& foregroundBrush);

	virtual CBCGPSize GetTextSize(const CString& strText, const CBCGPTextFormat& textFormat, double dblWidth = 0., BOOL bIgnoreTextRotation = FALSE);

	virtual void SetClipRect(const CBCGPRect& rectClip, int nFlags = RGN_COPY);
	virtual void SetClipArea(const CBCGPGeometry& geometry, int nFlags = RGN_COPY);
	virtual void ReleaseClipArea();

	virtual void CombineGeometry(CBCGPGeometry& geometryDest, const CBCGPGeometry& geometrySrc1, const CBCGPGeometry& geometrySrc2, int nFlags);
	virtual void GetGeometryBoundingRect(const CBCGPGeometry& geometry, CBCGPRect& rect);

	virtual void EnableAntialiasing(BOOL bEnable = TRUE);
	virtual BOOL IsAntialiasingEnabled() const;

	virtual BOOL IsBindDCFailed() const
	{
		return m_bIsBindError;
	}

protected:
	virtual LPVOID CreateGeometry(CBCGPGeometry& geometry);
	virtual BOOL DestroyGeometry(CBCGPGeometry& geometry);

	virtual LPVOID CreateTextFormat(CBCGPTextFormat& textFormat);
	virtual BOOL DestroyTextFormat(CBCGPTextFormat& textFormat);

	virtual LPVOID CreateBrush(CBCGPBrush& brush);
	virtual BOOL DestroyBrush(CBCGPBrush& brush);
	virtual BOOL SetBrushOpacity(CBCGPBrush& brush);

	virtual LPVOID CreateImage(CBCGPImage& image);
	virtual BOOL DestroyImage(CBCGPImage& image);
	virtual CBCGPSize GetImageSize(CBCGPImage& image);
	virtual BOOL CopyImage(CBCGPImage& imageSrc, CBCGPImage& imageDest, const CBCGPRect& rectSrc);
	virtual HBITMAP ExportImageToBitmap(CBCGPImage& image);

	virtual LPVOID CreateStrokeStyle(CBCGPStrokeStyle& style);
	virtual BOOL DestroyStrokeStyle(CBCGPStrokeStyle& style);

	ID2D1StrokeStyle* CreateStrokeStyle(const CBCGPStrokeStyle* pStyle, double dblWidth = 1.);

	ID2D1Brush* PrepareBrush(CBCGPBrush& brush, const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo,
		CBCGPBrush::BCGP_GRADIENT_TYPE gradientType = CBCGPBrush::BCGP_GRADIENT_HORIZONTAL);

	ID2D1Brush* PrepareBrush(CBCGPBrush& brush, const CBCGPRect& rectIn, 
		CBCGPBrush::BCGP_GRADIENT_TYPE gradientType = CBCGPBrush::BCGP_GRADIENT_HORIZONTAL);

	void PrepareGradientPoints(ID2D1Brush* pD2DBrush, const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo,
		CBCGPBrush::BCGP_GRADIENT_TYPE gradientType = CBCGPBrush::BCGP_GRADIENT_HORIZONTAL);
	void PrepareGradientPoints(ID2D1Brush* pD2DBrush, const CBCGPRect& rectIn, CBCGPBrush::BCGP_GRADIENT_TYPE gradientType = CBCGPBrush::BCGP_GRADIENT_HORIZONTAL);

	virtual void OnFillGeometryBevel(const CBCGPBrush& brFill, const CBCGPGeometry& shape, const CBCGPGeometry& shapeInternal);

	BOOL IsGraphicsManagerValid(CBCGPGraphicsManager* pGM);

	ID2D1Bitmap* CreateImageFromHBITMAP(HBITMAP hBitmap, BOOL bIsIgnoreAlphaBitmap = FALSE);

	IDWriteTextFormat* CreateTextFormatFromLOGFONT(const LOGFONT& logFont, const CString& strLocale);

// Attributes:
protected:
	CDC*						m_pDC;
	ID2D1DCRenderTarget*		m_dcRenderTarget;
	ID2D1BitmapRenderTarget*	m_pBitmapRenderTarget;
	ID2D1RenderTarget*			m_pRenderTarget;
	ID2D1Layer*					m_pCurrlayer;
	CBCGPRect					m_rectClip;
	CBCGPPolygonGeometry		m_CurrGeometry;
	BOOL						m_bIsBindError;
	CBitmap						m_bmpPrint;
	CDC							m_dcPrint;
	CBitmap*					m_pBmpPrintOld;

	static HINSTANCE m_hinstD2DDLL;
	static HINSTANCE m_hinstDWriteDLL;

	static ID2D1Factory* m_pDirect2dFactory;
	static IDWriteFactory* m_pWriteFactory;
	static IDWriteGdiInterop* m_pGDIInterop;
	static IWICImagingFactory* m_pWicFactory;

	static FLOAT m_DesktopDpiX;
	static FLOAT m_DesktopDpiY;

	static BOOL m_bD2DInitialized;

	static BOOL InitD2D(
		BCGP_D2D1_FACTORY_TYPE d2dFactoryType = BCGP_D2D1_FACTORY_TYPE_SINGLE_THREADED, 
		BCGP_DWRITE_FACTORY_TYPE writeFactoryType = BCGP_DWRITE_FACTORY_TYPE_ISOLATED);

	static void ReleaseD2DRefs();
};

#endif // !defined(AFX_BCGPGRAPHICSMANAGERD2D_H__847B8B25_AE4A_44E1_B1E8_CFE353B89B48__INCLUDED_)
