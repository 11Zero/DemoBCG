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
// BCGPControlRenderer.h: interface for the CBCGPControlRenderer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPCONTROLRENDERER_H__CE281511_C403_4979_87C2_0E9F36FDF67D__INCLUDED_)
#define AFX_BCGPCONTROLRENDERER_H__CE281511_C403_4979_87C2_0E9F36FDF67D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPToolBarImages.h"

class BCGCBPRODLLEXPORT CBCGPControlRendererParams
{
public:
	CBCGPControlRendererParams ();
	~CBCGPControlRendererParams ();

	CBCGPControlRendererParams (HBITMAP hBitmap,
					COLORREF clrTransparent,
					const CRect& rtImage, 
					const CRect& rtCorners,
					const CRect& rtSides = CRect (0, 0, 0, 0),
					const CRect& rtInner = CRect (0, 0, 0, 0),
					BOOL bPreMultiplyCheck = TRUE);

	CBCGPControlRendererParams (UINT uiBmpResID,
					const CRect& rtImage, 
					const CRect& rtCorners,
					const CRect& rtSides = CRect (0, 0, 0, 0),
                    const CRect& rtInner = CRect (0, 0, 0, 0));

	CBCGPControlRendererParams (LPCTSTR lpszBmpResID,
					const CRect& rtImage, 
					const CRect& rtCorners,
					const CRect& rtSides = CRect (0, 0, 0, 0),
                    const CRect& rtInner = CRect (0, 0, 0, 0));

	CBCGPControlRendererParams (UINT uiBmpResID,
					COLORREF clrTransparent,
					const CRect& rtImage, 
					const CRect& rtCorners,
					const CRect& rtSides = CRect (0, 0, 0, 0),
                    const CRect& rtInner = CRect (0, 0, 0, 0),
					BOOL bPreMultiplyCheck = TRUE);

	CBCGPControlRendererParams (LPCTSTR lpszBmpResID,
					COLORREF clrTransparent,
					const CRect& rtImage, 
					const CRect& rtCorners,
					const CRect& rtSides = CRect (0, 0, 0, 0),
                    const CRect& rtInner = CRect (0, 0, 0, 0),
					BOOL bPreMultiplyCheck = TRUE);

	CBCGPControlRendererParams (const CBCGPControlRendererParams& rSrc);

public:
	void CommonInit ();

	HBITMAP GetBitmap() const;
	
	LPCTSTR GetResourceID () const;
	void SetResourceID (LPCTSTR lpszBmpResID);
	void SetBaseColor (COLORREF clrBase, COLORREF clrTarget);

	void Rotate (BOOL bCW = TRUE);

	CBCGPControlRendererParams& operator = (const CBCGPControlRendererParams& rSrc);

public:
	UINT		m_uiBmpResID;
	CString     m_strBmpResID;
	HBITMAP		m_hBitmap;
	CRect		m_rectImage;
	CRect		m_rectCorners;
	CRect		m_rectSides;
	CRect		m_rectInter;
	COLORREF	m_clrTransparent;
	BOOL		m_bPreMultiplyCheck;
	BOOL		m_bMapTo3DColors;
	COLORREF	m_clrBase;
	COLORREF	m_clrTarget;
};

class BCGCBPRODLLEXPORT CBCGPControlRenderer : public CObject  
{
	DECLARE_DYNCREATE(CBCGPControlRenderer)

public:
	CBCGPControlRenderer();
	virtual ~CBCGPControlRenderer();

// Operations:
public:
	virtual BOOL Create (const CBCGPControlRendererParams& params, BOOL bFlipvert = FALSE);

	BOOL SmoothResize (double dblScale);

	virtual void Draw (CDC* pDC, CRect rect, UINT index = 0, BYTE alphaSrc = 255);
	virtual void DrawFrame (CDC* pDC, CRect rect, UINT index = 0, BYTE alphaSrc = 255);
	virtual void FillInterior (CDC* pDC, CRect rect, UINT index = 0, BYTE alphaSrc = 255);
	virtual void FillInterior (CDC* pDC, CRect rect, CBCGPToolBarImages::ImageAlignHorz horz,
		CBCGPToolBarImages::ImageAlignVert vert, UINT index = 0, BYTE alphaSrc = 255);

	virtual void OnSysColorChange ();
	const CBCGPControlRendererParams& GetParams () const
	{
		return m_Params;
	}

	BOOL IsValid () const
	{
		return m_Bitmap.IsValid ();
	}

	BOOL IsMirror () const
	{
		return m_bMirror;
	}	
	virtual void Mirror ();

	virtual void Rotate (BOOL bCW = TRUE);

	virtual void CleanUp ();
	
	int GetImageCount () const
	{
		CRect rectImage (m_Params.m_rectImage);

		if (m_Bitmap.GetCount () == 1)
		{
			HBITMAP hBmp = m_Bitmap.GetImageWell ();
			BITMAP bmp;

			if (::GetObject (hBmp, sizeof (BITMAP), &bmp) == sizeof(BITMAP))
			{
				return bmp.bmHeight / m_Params.m_rectImage.Height ();
			}

			return 0;
		}

		return m_Bitmap.GetCount ();
	}

	CBCGPToolBarImages& GetImages ()
	{
		return m_Bitmap;
	}

	virtual BOOL CopyTo (CBCGPControlRenderer& dest);

	BOOL IsScaled () const
	{
		return m_bIsScaled;
	}

// Attributes:
protected:
	CBCGPToolBarImages			m_Bitmap;
	CBCGPControlRendererParams	m_Params;
    BOOL						m_bMirror;
	BOOL						m_bIsScaled;
};

class BCGCBPRODLLEXPORT CBCGPShadowRenderer : public CBCGPControlRenderer  
{
	DECLARE_DYNCREATE(CBCGPShadowRenderer)

public:
	CBCGPShadowRenderer();
	virtual ~CBCGPShadowRenderer();

// Operations:
public:
	virtual BOOL Create (int nDepth,
						 COLORREF clrBase,
                         int iMinBrightness = 0,
						 int iMaxBrightness = 100,
						 BOOL bCanMirror = TRUE,
						 double dblSmooth = 1.0,
						 double dblDarkRatio = 0.25);

	virtual void OnSysColorChange ();

	virtual void Draw (CDC* pDC, CRect rect, UINT index = 0, BYTE alphaSrc = 255);
	virtual void DrawFrame (CDC* pDC, CRect rect, UINT index = 0, BYTE alphaSrc = 255);

	virtual BOOL CopyTo (CBCGPControlRenderer& dest);

protected:
	virtual BOOL Create (const CBCGPControlRendererParams& params, BOOL bFlipvert = FALSE);

private:
	int      m_nDepth;
	COLORREF m_clrBase;
	int      m_iMinBrightness;
	int      m_iMaxBrightness;
};

#endif // !defined(AFX_BCGPCONTROLRENDERER_H__CE281511_C403_4979_87C2_0E9F36FDF67D__INCLUDED_)
