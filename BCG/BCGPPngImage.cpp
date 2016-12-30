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
// BCGPPngImage.cpp: implementation of the CBCGPPngImage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPPngImage.h"
#include "BCGPMath.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#if !defined BCGP_EXCLUDE_PNG_SUPPORT

#if _MSC_VER >= 1300 && !defined BCGP_EXCLUDE_GDI_PLUS

CImage CBCGPPngImage::m_image;
BOOL CBCGPPngImage::m_bCImageInitialized = FALSE;

BOOL CBCGPPngImage::InitCImage()
{
	if (m_bCImageInitialized)
	{
		return TRUE;
	}

	CString strExporters;
	CSimpleArray<GUID> aguidFileTypes;

	m_bCImageInitialized = 
		SUCCEEDED(CBCGPPngImage::m_image.GetExporterFilterString(strExporters, aguidFileTypes));

	return m_bCImageInitialized;
}

static HRESULT PngSaveImage(CImage& image, IStream* pStream )
{
	REFGUID guidFileType = Gdiplus::ImageFormatPNG;

#if _MSC_VER == 1300
	return image.Save (pStream, guidFileType);
#else
	if (image.GetBPP() < 32)
	{
		return image.Save (pStream, guidFileType);
	}

	UINT nEncoders;
	UINT nBytes;
	Gdiplus::Status status;

	status = Gdiplus::GetImageEncodersSize (&nEncoders, &nBytes);
	if( status != Gdiplus::Ok )
	{
		return( E_FAIL );
	}

	USES_ATL_SAFE_ALLOCA;
	Gdiplus::ImageCodecInfo* pEncoders = static_cast<Gdiplus::ImageCodecInfo*>(_ATL_SAFE_ALLOCA(nBytes, _ATL_SAFE_ALLOCA_DEF_THRESHOLD));

	if (pEncoders == NULL)
	{
		return E_OUTOFMEMORY;
	}

	status = Gdiplus::GetImageEncoders (nEncoders, nBytes, pEncoders);
	if (status != Gdiplus::Ok)
	{
		return E_FAIL;
	}

	CLSID clsidEncoder = CLSID_NULL;
	for (UINT iEncoder = 0; iEncoder < nEncoders; iEncoder++)
	{
		if (pEncoders[iEncoder].FormatID == guidFileType)
		{
			clsidEncoder = pEncoders[iEncoder].Clsid;
			break;
		}
	}

	if (clsidEncoder == CLSID_NULL)
	{
		return E_FAIL;
	}

	Gdiplus::Bitmap bm (image.GetWidth(), image.GetHeight(), image.GetPitch(), PixelFormat32bppPARGB, static_cast<BYTE*>(image.GetBits ()));
	status = bm.Save (pStream, &clsidEncoder, NULL);
	if (status != Gdiplus::Ok)
	{
		return E_FAIL;
	}

	return S_OK;
#endif
}

#else

extern "C" {
	#ifdef BCGP_USE_EXTERNAL_PNG_LIB
		#include <png.h>
	#else
		#include "Lpng/png.h"
	#endif
}

#if (PNG_LIBPNG_VER_MAJOR > 1) || \
    ((PNG_LIBPNG_VER_MAJOR == 1) && (PNG_LIBPNG_VER_MINOR >= 4))
    #define png_check_sig(sig, num) !png_sig_cmp(sig, 0, num)
#endif


static void _png_cexcept_error(png_structp /*png_ptr*/, png_const_charp /*msg*/)
{
	// TODO
}

#ifdef USE_FAR_KEYWORD
    /* this is the model-independent version. Since the standard I/O library
       can't handle far buffers in the medium and small models, we have to copy
       the data.
    */

    #define NEAR_BUF_SIZE 1024
    #define MIN(a,b) (a <= b ? a : b)
#endif

static void _png_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	CFile* pFile = (CFile*)CVT_PTR(png_ptr->io_ptr);
	ASSERT_VALID (pFile);

    png_size_t check = 0;

#ifndef USE_FAR_KEYWORD

    check = pFile->Read(data, length);

#else

    png_byte *near_data;

    /* Check if data really is near. If so, use usual code. */
    near_data = (png_byte *)CVT_PTR_NOCHECK(data);

    if ((png_bytep)near_data == data)
    {
        check = pFile->Read(near_data, length);
    }
    else
    {
        png_byte buf[NEAR_BUF_SIZE];
        png_size_t read, remaining, err;
        check = 0;
        remaining = length;
        do
        {
            read = MIN(NEAR_BUF_SIZE, remaining);
            err = pFile->Read(buf, read);
            png_memcpy(data, buf, read); /* copy far buffer to near buffer */
            if(err != read)
            {
                break;
            }
            else
            {
                check += err;
            }

            data += read;
            remaining -= read;
        }
        while (remaining != 0);
    }

#endif

    if (check != length)
    {
        png_error(png_ptr, "Read Error");
    }
}

static void _png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	CFile* pFile = (CFile*)CVT_PTR(png_ptr->io_ptr);
	ASSERT_VALID (pFile);

    png_size_t check = 0;

#ifndef USE_FAR_KEYWORD

    ULONGLONG position = pFile->GetPosition();
    pFile->Write(data, length);
    check = (png_size_t)(pFile->GetPosition() - position);

#else

    png_byte *near_data;  /* Needs to be "png_byte *" instead of "png_bytep" */

    /* Check if data really is near. If so, use usual code. */
    near_data = (png_byte *)CVT_PTR_NOCHECK(data);

    if ((png_bytep)near_data == data)
    {
        ULONGLONG position = pFile->GetPosition();
        pFile->Write(data, length);
        check = (png_size_t)(pFile->GetPosition() - position);
    }
    else
    {
        png_byte buf[NEAR_BUF_SIZE];
        png_size_t written, remaining, err;
        check = 0;
        remaining = length;
        do
        {
            written = MIN(NEAR_BUF_SIZE, remaining);
            png_memcpy(buf, data, written); /* copy far buffer to near buffer */

            ULONGLONG position = pFile->GetPosition();
            pFile->Write(buf, written);
            err = (png_size_t)(pFile->GetPosition() - position);

            if (err != written)
            {
                break;
            }
            else
            {
                check += err;
            }

            data += written;
            remaining -= written;
        }
        while (remaining != 0);
    }

#endif

    if (check != length)
    {
        png_error(png_ptr, "Write Error");
    }
}

static void _png_write_flush(png_structp png_ptr)
{
	CFile* pFile = (CFile*)CVT_PTR(png_ptr->io_ptr);
	ASSERT_VALID (pFile);

	pFile->Flush ();
}

static HBITMAP PngToBitmap (
        BYTE *pbImage, int cxImgSize, int cyImgSize, int cImgChannels)
{

	BITMAPINFO bi = {0};

	// Fill in the BITMAPINFOHEADER
	bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth       = cxImgSize;
	bi.bmiHeader.biHeight      = cyImgSize;
	bi.bmiHeader.biSizeImage   = cxImgSize * cyImgSize;
	bi.bmiHeader.biPlanes      = 1;
	bi.bmiHeader.biBitCount    = (USHORT) ((cImgChannels == 4) ? 32 : 24);
	bi.bmiHeader.biCompression = BI_RGB;

	LPBYTE pBits = NULL;
	HBITMAP hbmp = ::CreateDIBSection (NULL, &bi, DIB_RGB_COLORS, (LPVOID*)&pBits, NULL, 0);

    // calculate both row-bytes

    WORD wImgRowBytes = (WORD) (cImgChannels * cxImgSize);
    WORD wDIRowBytes = (WORD) ((cImgChannels == 4)
                                ? (cImgChannels * cxImgSize)
                                : (((3 * cxImgSize + 3L) >> 2) << 2));

    for (int yImg = 0; yImg < cyImgSize; yImg++)
    {
        BYTE* src = pbImage + yImg * wImgRowBytes;
        BYTE* dst = pBits + (cyImgSize - yImg - 1) * wDIRowBytes;

        for (int xImg = 0; xImg < cxImgSize; xImg++)
        {
            BYTE r = *src++;
            BYTE g = *src++;
            BYTE b = *src++;

            *dst++ = b; /* note the reverse order */
            *dst++ = g;
            *dst++ = r;

            if (cImgChannels == 4)
            {
                BYTE a = *src++;
				*dst++ = a;
            }
        }
    }

    return hbmp;
}

static png_bytep* BitmapToPng (HBITMAP bitmap, BITMAPINFOHEADER& bih)
{
	ZeroMemory (&bih, sizeof (BITMAPINFOHEADER));
	bih.biSize = sizeof (BITMAPINFOHEADER);

	BYTE* pBits   = NULL;
	DIBSECTION ds = {0};
	if (::GetObject (bitmap, sizeof (DIBSECTION), &ds) == 0)
	{
		BITMAP bmp = {0};
		if (::GetObject (bitmap, sizeof (BITMAP), &bmp) == 0)
		{
			ASSERT (FALSE);
			return NULL;
		}

		bih.biWidth     = bmp.bmWidth;
		bih.biHeight    = bmp.bmHeight;
		bih.biPlanes    = bmp.bmPlanes;
		bih.biBitCount  = bmp.bmBitsPixel;
		bih.biCompression = BI_RGB;

		pBits = (BYTE*) bmp.bmBits;
	}
	else
	{
		memcpy (&bih, &ds.dsBmih, sizeof (BITMAPINFOHEADER));
		bih.biSize = sizeof (BITMAPINFOHEADER);
		pBits = (BYTE*) ds.dsBm.bmBits;
	}

	if (bih.biBitCount < 24)
	{
		return NULL;
	}

	LONG height = abs (bih.biHeight);
	DWORD cImgChannels  = bih.biBitCount / 8;
	DWORD wImgRowBytes  = cImgChannels * bih.biWidth;
	DWORD wDIRowBytes   = (DWORD) (((wImgRowBytes + 3L) >> 2) << 2);
	bih.biSizeImage = wDIRowBytes * height;

	long offset = bih.biHeight > 0 ? -(long) wDIRowBytes : wDIRowBytes;
	if (offset < 0)
	{
		pBits += (height - 1) * wDIRowBytes;
	}

	png_bytep* pbImage = new png_bytep[height];

    for (LONG yImg = 0; yImg < height; yImg++)
    {
        BYTE* src = pBits + yImg * offset;
        png_byte* dst = new png_byte[wImgRowBytes];
		pbImage[yImg] = dst;

		if (cImgChannels == 3)
		{
			memcpy (dst, src, wImgRowBytes);
		}
		else
		{
			for (LONG xImg = 0; xImg < bih.biWidth; xImg++)
			{
				RGBQUAD* quad = (RGBQUAD*)(src);
				src += 4;

				double alpha = quad->rgbReserved == 0 ? 0. : 255.0 / (double)quad->rgbReserved;

				*dst++ = (BYTE) min (bcg_round(quad->rgbBlue  * alpha), 255); /* note the reverse order */
				*dst++ = (BYTE) min (bcg_round(quad->rgbGreen * alpha), 255);
				*dst++ = (BYTE) min (bcg_round(quad->rgbRed   * alpha), 255);
				*dst++ = quad->rgbReserved;
			}
		}
    }

	return pbImage;
}

#pragma warning (disable : 4702)

static BOOL PngLoadImage (CFile* pFile, png_structp& png_ptr, png_infop& info_ptr, png_byte **ppbImageData,
                   int *piWidth, int *piHeight, int *piChannels, png_color *pBkgColor)
{
	ASSERT_VALID (pFile);

    png_byte            pbSig[8];
    int                 iBitDepth;
    int                 iColorType;
    double              dGamma;
    png_color_16       *pBackground;
    png_uint_32         ulChannels;
    png_uint_32         ulRowBytes;
    png_byte           *pbImageData = *ppbImageData;
    static png_byte   **ppbRowPointers = NULL;
    int                 i;

	pFile->Read (pbSig, 8);

    if (!png_check_sig(pbSig, 8))
    {
        *ppbImageData = pbImageData = NULL;
        return FALSE;
    }

    // create the two png(-info) structures

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
      (png_error_ptr)_png_cexcept_error, (png_error_ptr)NULL);
    if (!png_ptr)
    {
        *ppbImageData = pbImageData = NULL;
        return FALSE;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, NULL, NULL);

		png_ptr = NULL;
        *ppbImageData = pbImageData = NULL;

        return FALSE;
    }

    BOOL bRes = FALSE;

    try
    {
        // initialize the png structure
		png_set_read_fn(png_ptr, (png_voidp)pFile, _png_read_data);
        png_set_sig_bytes(png_ptr, 8);
        
        // read all PNG info up to image data
        
        png_read_info(png_ptr, info_ptr);
        
        // get width, height, bit-depth and color-type
        
        png_get_IHDR(png_ptr, info_ptr, (png_uint_32*) piWidth, (png_uint_32*) piHeight, &iBitDepth,
            &iColorType, NULL, NULL, NULL);
        
        // expand images of all color-type and bit-depth to 3x8 bit RGB images
        // let the library process things like alpha, transparency, background
        
        if (iBitDepth == 16)
            png_set_strip_16(png_ptr);
        if (iColorType == PNG_COLOR_TYPE_PALETTE)
            png_set_expand(png_ptr);
        if (iBitDepth < 8)
            png_set_expand(png_ptr);
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
            png_set_expand(png_ptr);
        if (iColorType == PNG_COLOR_TYPE_GRAY ||
            iColorType == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png_ptr);
        
        // set the background color to draw transparent and alpha images over.
        if (png_get_bKGD(png_ptr, info_ptr, &pBackground))
        {
            png_set_background(png_ptr, pBackground, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
            pBkgColor->red   = (byte) pBackground->red;
            pBkgColor->green = (byte) pBackground->green;
            pBkgColor->blue  = (byte) pBackground->blue;
        }
        else
        {
            pBkgColor = NULL;
        }
        
        // if required set gamma conversion
        if (png_get_gAMA(png_ptr, info_ptr, &dGamma))
            png_set_gamma(png_ptr, (double) 2.2, dGamma);
        
        // after the transformations have been registered update info_ptr data
        
        png_read_update_info(png_ptr, info_ptr);
        
        // get again width, height and the new bit-depth and color-type
        
        png_get_IHDR(png_ptr, info_ptr, (png_uint_32*) piWidth, (png_uint_32*) piHeight, &iBitDepth,
            &iColorType, NULL, NULL, NULL);
        
        
        // row_bytes is the width x number of channels
        
        ulRowBytes = png_get_rowbytes(png_ptr, info_ptr);
        ulChannels = png_get_channels(png_ptr, info_ptr);
        
        *piChannels = ulChannels;
        
        // now we can allocate memory to store the image
        
        if (pbImageData)
        {
            delete [] pbImageData;
            pbImageData = NULL;
        }
        if ((pbImageData = (png_byte*) new png_byte[ulRowBytes * (*piHeight)]) == NULL)
        {
            png_error(png_ptr, "Visual PNG: out of memory");
        }
        *ppbImageData = pbImageData;
        
        // and allocate memory for an array of row-pointers
        
        if ((ppbRowPointers = (png_bytepp) new png_bytep[(*piHeight)]) == NULL)
        {
            png_error(png_ptr, "Visual PNG: out of memory");
        }
        
        // set the individual row-pointers to point at the correct offsets
        
        for (i = 0; i < (*piHeight); i++)
        {
            ppbRowPointers[i] = pbImageData + i * ulRowBytes;
        }
        
        // now we can go ahead and just read the whole image
        
        png_read_image(png_ptr, ppbRowPointers);
        
        // read the additional chunks in the PNG file (not really needed)
        
        png_read_end(png_ptr, NULL);
        
        // yepp, done
        bRes = TRUE;
    }
    catch(...)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

        png_ptr = NULL;
        info_ptr = NULL;
        *ppbImageData = pbImageData = NULL;
    }

    if (ppbRowPointers != NULL)
    {
        delete [] ppbRowPointers;
        ppbRowPointers = NULL;
    }

    return bRes;
}

static BOOL PngSaveImage (CFile* pFile, png_structp& png_ptr, png_infop& info_ptr,  png_byte **ppbImageData,
                   int iWidth, int iHeight, int iChannels, COLORREF BkgColor)
{
	int color_type = iChannels == 4 ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
                    (png_error_ptr)_png_cexcept_error, (png_error_ptr)NULL);
	if (!png_ptr)
	{
		return FALSE;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, NULL);

		png_ptr = NULL;

		return FALSE;
	}

    BOOL bRes = FALSE;

	try
	{
        png_set_write_fn(png_ptr, (png_voidp)pFile, _png_write_data, _png_write_flush);

		png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

        png_set_IHDR(png_ptr, info_ptr, iWidth, iHeight,
                     8, color_type, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        if (BkgColor != CLR_DEFAULT)
        {   /* we know it's RGBA, not gray+alpha */
            png_color_16  background;

            background.red   = GetRValue(BkgColor);
            background.green = GetGValue(BkgColor);
            background.blue  = GetBValue(BkgColor);
            png_set_bKGD(png_ptr, info_ptr, &background);
        }

        png_write_info(png_ptr, info_ptr);

        png_set_bgr(png_ptr);

        png_set_packing(png_ptr);

        png_write_image(png_ptr, ppbImageData);

		png_write_end(png_ptr, NULL);

        bRes = TRUE;
	}
    catch(...)
    {
		png_destroy_write_struct(&png_ptr, &info_ptr);

        png_ptr = NULL;
        info_ptr = NULL;
    }

	return bRes;
}

#pragma warning (default : 4702)

#endif // _MSC_VER < 1300
#endif // BCGP_EXCLUDE_PNG_SUPPORT

CString CBCGPPngImage::m_strPngResType = _T("PNG");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPPngImage::CBCGPPngImage()
{
}
//*******************************************************************************
CBCGPPngImage::~CBCGPPngImage()
{
}

//////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////

BOOL CBCGPPngImage::Load (UINT uiResID, HINSTANCE hinstRes)
{
	return Load (MAKEINTRESOURCE (uiResID), hinstRes);
}
//*******************************************************************************
BOOL CBCGPPngImage::Load (LPCTSTR lpszResourceName, HINSTANCE hinstRes)
{
#ifdef BCGP_EXCLUDE_PNG_SUPPORT
	UNREFERENCED_PARAMETER(lpszResourceName);
	UNREFERENCED_PARAMETER(hinstRes);
	return FALSE;
#else
	if (hinstRes == NULL)
	{
		hinstRes = AfxFindResourceHandle(lpszResourceName, m_strPngResType);
	}

	HRSRC hRsrc = ::FindResource(hinstRes, lpszResourceName, m_strPngResType);
	if (hRsrc == NULL)
	{
		return FALSE;
	}

	HGLOBAL hGlobal = LoadResource (hinstRes, hRsrc);
	if (hGlobal == NULL)
	{
		return FALSE;
	}

	LPVOID lpBuffer = ::LockResource(hGlobal);
	if (lpBuffer == NULL)
	{
		FreeResource(hGlobal);
		return FALSE;
	}

	BOOL bRes = LoadFromBuffer ((LPBYTE) lpBuffer, (UINT) ::SizeofResource (hinstRes, hRsrc));

	UnlockResource(hGlobal);
	FreeResource(hGlobal);

	return bRes;
#endif // BCGP_EXCLUDE_PNG_SUPPORT
}
//*******************************************************************************
BOOL CBCGPPngImage::LoadFromFile (LPCTSTR lpszPath)
{
#ifdef BCGP_EXCLUDE_PNG_SUPPORT
	UNREFERENCED_PARAMETER(lpszPath);
	return FALSE;
#else
#if _MSC_VER < 1300 || defined BCGP_EXCLUDE_GDI_PLUS
	CFile file;
	if (!file.Open (lpszPath, CFile::modeRead | CFile::shareDenyNone))
	{
		return FALSE;
	}

	return LoadFromFile (&file);
#else
	if (m_image.Load (lpszPath) != S_OK)
	{
		return FALSE;
	}

	return Attach (m_image.Detach ());
#endif
#endif // BCGP_EXCLUDE_PNG_SUPPORT
}
//*******************************************************************************
BOOL CBCGPPngImage::LoadFromFile (CFile* pFile)
{
#ifdef BCGP_EXCLUDE_PNG_SUPPORT
	UNREFERENCED_PARAMETER(pFile);
	return FALSE;
#else
#if _MSC_VER < 1300 || defined BCGP_EXCLUDE_GDI_PLUS
	ASSERT_VALID (pFile);

	try
	{
		int cxImgSize = 0;
		int cyImgSize = 0;
		png_byte* pbImageData = NULL;
		int nChannels = 0;
		png_color bkgColor;

		png_structp png_ptr = NULL;
		png_infop info_ptr = NULL; 

		if (!PngLoadImage (pFile, png_ptr, info_ptr, &pbImageData, &cxImgSize, &cyImgSize, &nChannels, &bkgColor))
		{
			if (pbImageData != NULL)
			{
				delete [] pbImageData;
			}

			return FALSE;
		}

		if (pbImageData == NULL)
		{
			return FALSE;
		}

		HBITMAP hBitmap = PngToBitmap (pbImageData, cxImgSize, cyImgSize, nChannels);

		delete [] pbImageData;

        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

        png_ptr = NULL;
        info_ptr = NULL;

		if (hBitmap == NULL)
		{
			return FALSE;
		}

		Attach (hBitmap);
	}
	catch (CFileException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("File exception in CBCGPPngImage::LoadFromFile!\n"));
		return FALSE;
	}

	return TRUE;
#else
	UNREFERENCED_PARAMETER(pFile);

	ASSERT (FALSE);
	return FALSE;
#endif
#endif // BCGP_EXCLUDE_PNG_SUPPORT
}
//*******************************************************************************
BOOL CBCGPPngImage::LoadFromBuffer (LPBYTE lpBuffer, UINT uiSize)
{
#ifdef BCGP_EXCLUDE_PNG_SUPPORT
	UNREFERENCED_PARAMETER(lpBuffer);
	UNREFERENCED_PARAMETER(uiSize);
	return FALSE;
#else
	ASSERT(lpBuffer != NULL);

#if _MSC_VER >= 1300 && !defined BCGP_EXCLUDE_GDI_PLUS

	HGLOBAL hGlobal = ::GlobalAlloc (GMEM_MOVEABLE, uiSize);
	if (hGlobal == NULL)
	{
		return FALSE;
	}

	IStream* pStream = NULL;
	LPVOID lpResBuffer = ::GlobalLock (hGlobal);
	ASSERT (lpResBuffer != NULL);

	memcpy (lpResBuffer, lpBuffer, uiSize);

	::GlobalUnlock (hGlobal);

	BOOL bRes = FALSE;
	if (SUCCEEDED(::CreateStreamOnHGlobal (hGlobal, FALSE, &pStream)))
	{
		m_image.Load (pStream);
		pStream->Release ();

		bRes = Attach (m_image.Detach ());

		m_bCImageInitialized = TRUE;
	}

	::GlobalFree (hGlobal);

	return bRes;
#else
	try
	{
		CMemFile file (lpBuffer, uiSize);
		return LoadFromFile (&file);
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPPngImage::LoadFromBuffer!\n"));
	}

	return FALSE;
#endif
#endif // BCGP_EXCLUDE_PNG_SUPPORT
}
//*******************************************************************************
BOOL CBCGPPngImage::SaveToFile (LPCTSTR lpszPath)
{
#ifdef BCGP_EXCLUDE_PNG_SUPPORT
	UNREFERENCED_PARAMETER(lpszPath);
	return FALSE;
#else
	CFile file;
	if (!file.Open (lpszPath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone))
	{
		return FALSE;
	}

	return SaveToFile (&file);
#endif // BCGP_EXCLUDE_PNG_SUPPORT
}
//*******************************************************************************
UINT CBCGPPngImage::SaveToBuffer (LPBYTE* lpBuffer)
{
#ifdef BCGP_EXCLUDE_PNG_SUPPORT
	UNREFERENCED_PARAMETER(lpBuffer);
	return 0;
#else
	ASSERT(lpBuffer != NULL);
	UINT size = 0;
    *lpBuffer = 0;

#if _MSC_VER < 1300 || defined BCGP_EXCLUDE_GDI_PLUS
	try
	{
		BITMAPINFOHEADER bih;
		png_bytep* pbImageData = BitmapToPng ((HBITMAP) GetSafeHandle (), bih);
		if (pbImageData == NULL)
		{
			return FALSE;
		}

		CMemFile file;

		png_structp png_ptr = NULL;
		png_infop info_ptr = NULL; 

		if (PngSaveImage (&file, png_ptr, info_ptr, pbImageData, bih.biWidth, abs (bih.biHeight), 
			bih.biBitCount / 8, CLR_DEFAULT))
		{
			png_destroy_write_struct(&png_ptr, &info_ptr);

            png_ptr = NULL;
            info_ptr = NULL;

			size = (UINT)file.GetLength ();
			*lpBuffer = file.Detach ();
		}

		if (pbImageData != NULL)
		{
			for (LONG i = 0; i < abs (bih.biHeight); i++)
			{
				delete [] pbImageData[i];
			}

			delete [] pbImageData;
		}
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPPngImage::LoadFromBuffer!\n"));
	}
#else

	if (!CBCGPPngImage::InitCImage())
	{
		return 0;
	}

	HGLOBAL hGlobal = ::GlobalAlloc (GHND, 0);
	if (hGlobal != NULL)
	{
		IStream* pStream = NULL;
		if (SUCCEEDED(::CreateStreamOnHGlobal (hGlobal, FALSE, &pStream)))
		{
			m_image.Attach ((HBITMAP)GetSafeHandle());

			if (SUCCEEDED(PngSaveImage(m_image, pStream)))
			{
				STATSTG stat = {0};
				pStream->Stat (&stat, STATFLAG_NONAME);
				size = (UINT)stat.cbSize.QuadPart;
				if (size > 0)
				{
					*lpBuffer = new BYTE[size];
					LARGE_INTEGER dlibMove = {0};
					pStream->Seek (dlibMove, STREAM_SEEK_SET, NULL);
					pStream->Read (*lpBuffer, size, NULL);
				}
			}

			m_image.Detach ();

			pStream->Release ();
		}

		::GlobalFree (hGlobal);
	}

#endif

	return size;
#endif // BCGP_EXCLUDE_PNG_SUPPORT
}
//*******************************************************************************
BOOL CBCGPPngImage::SaveToFile (CFile* pFile)
{
#ifdef BCGP_EXCLUDE_PNG_SUPPORT
	UNREFERENCED_PARAMETER(pFile);
	return FALSE;
#else
	ASSERT_VALID (pFile);

	try
	{
		LPBYTE lpBuffer = NULL;
		UINT nCount = SaveToBuffer (&lpBuffer);
		if (nCount == 0 || lpBuffer == NULL)
		{
			return FALSE;
		}

		pFile->Write(lpBuffer, nCount);

		delete [] lpBuffer;
	}
	catch (CFileException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("File exception in CBCGPPngImage::SaveToFile!\n"));
		return FALSE;
	}

	return TRUE;
#endif // BCGP_EXCLUDE_PNG_SUPPORT
}
