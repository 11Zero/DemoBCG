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

#if !defined(AFX_BCGPGRAPHICSMANAGERGDI_H__847B8B25_AE4A_44E1_B1E8_CFE353B89B48__INCLUDED_)
#define AFX_BCGPGRAPHICSMANAGERGDI_H__847B8B25_AE4A_44E1_B1E8_CFE353B89B48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPGraphicsManager.h"

class CBCGPDrawManager;

class BCGCBPRODLLEXPORT CBCGPGraphicsManagerGDI : public CBCGPGraphicsManager  
{
	DECLARE_DYNCREATE(CBCGPGraphicsManagerGDI)

public:
	CBCGPGraphicsManagerGDI(CDC* pDC = NULL, BOOL bDoubleBuffering = TRUE, CBCGPGraphicsManagerParams* pParams = NULL);
	virtual ~CBCGPGraphicsManagerGDI();

	virtual CBCGPGraphicsManager* CreateOffScreenManager(const CBCGPRect& rect, CBCGPImage* pImageDest);

protected:
	CBCGPGraphicsManagerGDI(const CBCGPRect& rectDest, CBCGPImage* pImageDest);

// Attributes:
public:
	static void EnableTransparency(BOOL bEnable = TRUE);
	static BOOL IsTransparencyEnabled()
	{
		return m_bTransparency;
	}

	CDC* GetDCPaint()
	{
		return m_pDCPaint;
	}

	static BOOL m_bCheckForAttribDC;

// overrides:
public:
	virtual BOOL IsValid() const
	{
		return TRUE;
	}

	virtual void BindDC(CDC* pDC, BOOL bDoubleBuffering = TRUE);
	virtual void BindDC(CDC* pDC, const CRect& /*rect*/)
	{
		BindDC(pDC);
	}

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

	virtual void SetClipArea(const CBCGPGeometry& geometry, int nFlags = RGN_COPY);
	virtual void ReleaseClipArea();

	virtual void CombineGeometry(CBCGPGeometry& geometryDest, const CBCGPGeometry& geometrySrc1, const CBCGPGeometry& geometrySrc2, int nFlags);
	virtual void GetGeometryBoundingRect(const CBCGPGeometry& geometry, CBCGPRect& rect);

	virtual void EnableAntialiasing(BOOL bEnable = TRUE);
	virtual BOOL IsAntialiasingEnabled() const;

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

	virtual LPVOID CreateStrokeStyle(CBCGPStrokeStyle& /*style*/) { return NULL; }
	virtual BOOL DestroyStrokeStyle(CBCGPStrokeStyle& /*style*/) { return FALSE; }

	void FillGradientByType (const CRect& rect, const CBCGPBrush& brush);
	BOOL FillTransparentGradient(const CRect& rect, CBCGPBrush& brush);

	HPEN CreateGDIPen(const CBCGPBrush& brush, const CBCGPStrokeStyle* pStrokeStyle, double lineWidth);
	void CreateBrushImage(CBCGPBrush& brush);

// Attributes:
protected:
	CDC*				m_pDC;
	CDC*				m_pDCPaint;
	CBCGPMemDC*			m_pMemDC;
	CBCGPDrawManager*	m_pDM;
	BOOL				m_bDoubleBuffering;
	CBitmap				m_bmpScreen;
	CBitmap*			m_pBmpScreenOld;
	CDC					m_dcScreen;
	static BOOL			m_bTransparency;
	BOOL				m_bIsAntialiasing;

private:
	BOOL				m_bCheckHVLine;
};

#endif // !defined(AFX_BCGPGRAPHICSMANAGERGDI_H__847B8B25_AE4A_44E1_B1E8_CFE353B89B48__INCLUDED_)
