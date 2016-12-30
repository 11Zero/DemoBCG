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

// BCGToolBarImages.h: interface for the CBCGPToolBarImages class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPTOOLBARIMAGES_H__6F1376A1_C668_11D1_B110_ABB329E56E43__INCLUDED_)
#define AFX_BCGPTOOLBARIMAGES_H__6F1376A1_C668_11D1_B110_ABB329E56E43__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#ifndef __AFXMT_H__
	#include <afxmt.h>
#endif

#include "BCGCBPro.h"

// for custom drawing
typedef struct tagDrawState
{
	tagDrawState(BOOL bAutoGrayScale = FALSE)
	{
		m_bAutoGrayScale = bAutoGrayScale;
	}

	HBITMAP hbmMono;
	HBITMAP hbmMonoOld;
	HBITMAP hbmOldGlyphs;
	BOOL m_bAutoGrayScale;
}
CBCGPDrawState;

typedef BOOL (__stdcall * ALPHABLEND)(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION);
typedef BOOL (__stdcall * TRANSPARENTBLT)(HDC hdcDest, int xDest, int yDest, int nDestWidth, int nDestHeight,
	  HDC hdcSrc, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, 
	  UINT clrTransparent);

class BCGCBPRODLLEXPORT CBCGPToolBarImages : public CObject
{
	friend class CBCGPDropDownFrame;
	friend class CBCGPImageEditDlg;
	friend class CBCGPMenuImages;
	friend class CBCGPDrawManager;
	friend struct BCGPGLOBAL_DATA;
	friend class CBCGPRibbonCollector;

public:
	CBCGPToolBarImages();
	virtual ~CBCGPToolBarImages();

	enum ImageAlignHorz
	{
		ImageAlignHorzLeft,
		ImageAlignHorzCenter,
		ImageAlignHorzRight,
		ImageAlignHorzStretch
	};

	enum ImageAlignVert
	{
		ImageAlignVertTop,
		ImageAlignVertCenter,
		ImageAlignVertBottom,
		ImageAlignVertStretch
	};

// Operations:
public:
	static BOOL Is32BitTransparencySupported ();
	static BOOL PreMultiplyAlpha (HBITMAP hbmp, BOOL bAutoCheckPremlt);

	BOOL IsValid () const
	{
		return (m_hbmImageWell != NULL);
	}

	BOOL IsReadOnly () const
	{
		return m_bReadOnly;
	}

	HBITMAP GetImageWell () const
	{
		return m_hbmImageWell;				  	
	}

	HBITMAP GetImageWellLight () const
	{
		return m_hbmImageLight;
	}

	BOOL IsUserImagesList () const
	{
		return m_bUserImagesList;
	}

	void SetImageSize (SIZE sizeImage, BOOL bUpdateCount = FALSE)
	{
		m_sizeImage = sizeImage;

		if (bUpdateCount)
		{
			UpdateCount ();
		}
	}

	SIZE GetImageSize (BOOL bDest = FALSE) const
	{
		return bDest ? m_sizeImageDest : m_sizeImage;
	}

	int GetCount () const
	{
		return m_iCount;
	}

	int GetResourceOffset (UINT uiResId) const
	{
		int iOffset = -1;
		if (m_mapOrigResOffsets.Lookup (uiResId, iOffset))
		{
			return iOffset;
		}

		return -1;
	}

	COLORREF SetTransparentColor (COLORREF clrTransparent)
	{
		COLORREF clrOld = m_clrTransparent;

		if (clrTransparent != clrOld)
		{
			m_clrTransparent = clrTransparent;
			UpdateInternalImage (0);
			UpdateInternalImage (1);
		}

		return clrOld;
	}

	COLORREF GetTransparentColor () const
	{
		return m_clrTransparent;
	}

	BOOL Load (UINT uiResID, HINSTANCE hinstRes = NULL, BOOL bAdd = FALSE);
	BOOL LoadStr (LPCTSTR lpszResourceName, HINSTANCE hinstRes = NULL, BOOL bAdd = FALSE);

	BOOL Load (LPCTSTR lpszBmpFileName);
	BOOL Save (LPCTSTR lpszBmpFileName = NULL);

	void Clear ();

	BOOL PrepareDrawImage (CBCGPDrawState& ds,
						CSize sizeImageDest = CSize (0, 0),
						BOOL bFadeInactive = FALSE);

	BOOL Draw (CDC* pDC, 
				int x, int y,
				int iImageIndex,
				BOOL bHilite = FALSE, 
				BOOL bDisabled = FALSE,
				BOOL bIndeterminate = FALSE,
				BOOL bShadow = FALSE,
				BOOL bInactive = FALSE,
				BYTE alphaSrc = 255,
				BOOL bIsIgnoreAlpha = FALSE);

	BOOL DrawEx (CDC* pDC,
				CRect rect,
				int iImageIndex,
				ImageAlignHorz horzAlign = ImageAlignHorzLeft,
				ImageAlignVert vertAlign = ImageAlignVertTop,
				CRect rectSrc = CRect (0, 0, 0, 0),
				BYTE alphaSrc = 255,
				BOOL bIsIgnoreAlpha = FALSE);
				
	void EndDrawImage (CBCGPDrawState& ds);

	int AddImage (HBITMAP hbmp, BOOL bSetBitPerPixel = FALSE);
	int AddImage (const CBCGPToolBarImages& imageList, int nIndex);
	int AddIcon (HICON hIcon, BOOL bAlphaBlend = FALSE);
	BOOL UpdateImage (int iImage, HBITMAP hbmp);
	BOOL DeleteImage (int iImage);

	int GetBitsPerPixel () const
	{
		return m_nBitsPerPixel;
	}

	HICON ExtractIcon (int nIndex);

	BOOL CreateFromImageList(const CImageList& imageList);
	BOOL ExportToImageList(CImageList& imageList);

	BOOL CopyTo (CBCGPToolBarImages& imageList);
	BOOL CopyImageToClipboard (int iImage);

	BOOL GrayImages (int nGrayPercentage);

	HBITMAP GetMask (int iImage);

	void OnSysColorChange();

	BOOL MapTo3dColors (BOOL bUseRGBQUAD = TRUE,
						COLORREF clrSrc = (COLORREF)-1,
						COLORREF clrDest = (COLORREF)-1);
	static COLORREF MapToSysColorAlpha (COLORREF color);
	static COLORREF MapToSysColor (COLORREF color, BOOL bUseRGBQUAD = TRUE);
	static COLORREF MapFromSysColor (COLORREF color, BOOL bUseRGBQUAD = TRUE);
	static void FillDitheredRect (CDC* pDC, const CRect& rect);

	static void CleanUp ();

	static void SetDisabledImageAlpha (BYTE nValue)
	{
		ASSERT (nValue > 0);
		ASSERT (nValue <= 255);

		m_nDisabledImageAlpha = nValue;
	}

	static BYTE GetDisabledImageAlpha ()
	{
		return m_nDisabledImageAlpha;
	}

	static void SetFadedImageAlpha (BYTE nValue)
	{
		ASSERT (nValue > 0);
		ASSERT (nValue <= 255);

		m_nFadedImageAlpha = nValue;
	}

	static BYTE GetFadedImageAlpha ()
	{
		return m_nFadedImageAlpha;
	}

	void SetLightPercentage (int nValue)
	{
		m_nLightPercentage = nValue;
	}

	int GetLightPercentage () const
	{
		return m_nLightPercentage;
	}

	void SetPreMultiplyAutoCheck (BOOL bAuto = TRUE)
	{
		m_bAutoCheckPremlt = bAuto;
	}

	BOOL IsPreMultiplyAutoCheck () const
	{
		return m_bAutoCheckPremlt;
	}

	// RTL support:
	static void EnableRTL (BOOL bIsRTL = TRUE);
	static BOOL IsRTL ()
	{
		return m_bIsRTL;
	}

	BOOL Mirror ();
	static BOOL MirrorBitmap (HBITMAP& hbmp, int cxImage);

	BOOL MirrorVert ();
	static BOOL MirrorBitmapVert (HBITMAP& hbmp, int cyImage);

	static HRGN CreateRegionFromImage (HBITMAP bmp, COLORREF clrTransparent);

	void AddaptColors (COLORREF clrBase, COLORREF clrTone, BOOL bClampHue = TRUE);
	void AddaptColors (COLORREF clrBase, COLORREF clrTone, double dOpacity); // dOpacity [0.0 ... 1.0]

	void ConvertToGrayScale(double dblLumRatio = 1.0);

	void SetSingleImage (BOOL bUpdateInternalImages = TRUE);

	BOOL GetMapTo3DColors () const
	{
		return m_bMapTo3DColors;
	}

	void SetMapTo3DColors (BOOL bMapTo3DColors)
	{
		m_bMapTo3DColors = bMapTo3DColors;
	}

	BOOL GetAlwaysLight () const
	{
		return m_bAlwaysLight;
	}

	void SetAlwaysLight (BOOL bAlwaysLight = TRUE)
	{
		m_bAlwaysLight = bAlwaysLight;
	}

	CRect GetLastImageRect () const
	{
		return m_rectLastDraw;
	}

	static HBITMAP Copy (HBITMAP hbmpSrc);

	BOOL SmoothResize (double dblImageScale);
	
	double GetScale () const
	{
		return m_dblScale;
	}

	BOOL IsScaled () const
	{
		return GetScale () != 1.0;
	}

	BOOL ConvertTo32Bits (COLORREF clrTransparent = (COLORREF)-1);
	BOOL ConvertTo24Bits ();

	BOOL ChangeBpp(HBITMAP hbmp);

protected:
	static void TransparentBlt (HDC hdcDest, int nXDest, int nYDest, int nWidth, 
								int nHeight, CDC* pDcSrc, int nXSrc, int nYSrc,
								COLORREF colorTransparent,
								int nWidthDest = -1, int nHeightDest = -1);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void CreateMask(int iImage,
				BOOL bHilite, 
				BOOL bHiliteShadow);
	static HBITMAP CreateDitherBitmap();
	void UpdateCount ();
	BOOL UpdateInternalImage (int nIndex);

	BOOL PreMultiplyAlpha (HBITMAP hbmp);
	void CopyTemp (CBCGPToolBarImages& imagesDest);

	static BOOL MapBmpTo3dColors (	HBITMAP& hBmp, BOOL bUseRGBQUAD = TRUE,
									COLORREF clrSrc = (COLORREF)-1,
									COLORREF clrDest = (COLORREF)-1);

	static void _ConvertToGrayScale(HBITMAP& hbmp, COLORREF clrTransparent, double dblLumRatio);

	void CleanUpInternalImages();

// Attributes:
public:
	static BOOL			m_bDisableTrueColorAlpha;
	static BOOL			m_bMultiThreaded;	// Set to TRUE if images are used in different threads
	static BOOL			m_bIsDrawOnGlass;	// Draw image on Vista Glass area
	static double		m_dblColorTolerance;// Used by 'AddaptColors' method

protected:
	CSize				m_sizeImage;        // size of glyph
	CSize				m_sizeImageOriginal;
	CSize				m_sizeImageDest;	// destination size glyph
	HBITMAP				m_hbmImageWell;		// glyphs only
	HBITMAP				m_hbmImageLight;	// "faded" version (hi-color only)
	HBITMAP				m_hbmImageShadow;	// "shadow" version
	HBITMAP				m_hbmImageGray;		// grayscaled version
	BOOL				m_bUserImagesList;	// is user-defined images list?
	CString				m_strUDLPath;		// user-defined images path
	BOOL				m_bModified;		// is image modified?
	int					m_iCount;			// image counter
	CList<UINT, UINT>	m_lstOrigResIds;	// original resource ids
	CList<HINSTANCE, HINSTANCE>	m_lstOrigResInstances;	// original resource instances
	CMap<UINT, UINT, int, int> m_mapOrigResOffsets;// original resource offsets
	BOOL				m_bStretch;			// stretch images
	CDC					m_dcMem;			// DC for the drawing in memory
	CBitmap				m_bmpMem;			// bitmap for the drawing in memory
	CBitmap*			m_pBmpOriginal;
	COLORREF			m_clrTransparent;	// Transparent color
	COLORREF			m_clrTransparentOriginal;
	BOOL				m_bReadOnly;		// Loaded from read-only file
	BOOL				m_bIsTemporary;		// Temporary copy of another image
	COLORREF			m_clrImageShadow;	// Color of the shadow
	BOOL				m_bFadeInactive;	// Inactive image will be drawn with the "fading" effect
	int					m_nBitsPerPixel;	// Bitmap color depth
	static HINSTANCE	m_hinstMSIMGDLL;	// MSIMG.DLL instance
	static ALPHABLEND	m_pfAlphaBlend;		// Pointer to AlphaBlend method from MSIMG.DLL
	static TRANSPARENTBLT m_pfTransparentBlt;// Pointer to TransparentBlt method
	static BYTE			m_nDisabledImageAlpha;	// Used in the alpha-blending only
	static BYTE			m_nFadedImageAlpha;	// Used in the alpha-blending only
	BOOL				m_bIsGray;			// Image is grayed
	int					m_nGrayPercentage;	// Grayed image brightness
	static BOOL			m_bIsRTL;			// RTL languages support
	int					m_nLightPercentage;	// Light image percentage value
	BOOL				m_bMapTo3DColors;	// Map image colors to system
	BOOL				m_bAlwaysLight;		// Always use light image in all color resolutions
	CRect				m_rectLastDraw;		// Last drawn image location
	CRect				m_rectSubImage;		// The part of drawn image
	BOOL				m_bAutoCheckPremlt;	// Auto-check for 32 bpp images
	BOOL				m_bCreateMonoDC;	// Create mono DC in CBCGPDrawState
	double				m_dblScale;
	static CCriticalSection g_cs;			// For multi-thread applications
};

class BCGCBPRODLLEXPORT CBCGPDrawOnGlass
{
public:
	CBCGPDrawOnGlass (BOOL bSet = TRUE)
	{
		m_bIsDrawOnGlassSaved = CBCGPToolBarImages::m_bIsDrawOnGlass;
		CBCGPToolBarImages::m_bIsDrawOnGlass = bSet;
	}

	virtual ~CBCGPDrawOnGlass ()
	{
		Restore ();
	}

	void Restore ()
	{
		CBCGPToolBarImages::m_bIsDrawOnGlass = m_bIsDrawOnGlassSaved;
	}

protected:
	BOOL	m_bIsDrawOnGlassSaved;
};

#endif // !defined(AFX_BCGPTOOLBARIMAGES_H__6F1376A1_C668_11D1_B110_ABB329E56E43__INCLUDED_)
