// XTPChartOpenGLDeviceContext.cpp
//
// This file is a part of the XTREME TOOLKIT PRO MFC class library.
// (c)1998-2011 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME TOOLKIT PRO LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "GraphicLibrary/GdiPlus/GdiPlus.h"
#include "GraphicLibrary/OpenGL/Gl.h"

#include "XTPChartOpenGLDeviceContext.h"
#include "XTPChartDeviceCommand.h"

#include "XTPChartOpenGLHelpers.h"

using namespace Gdiplus;
using namespace Gdiplus::DllExports;



CXTPChartOpenGLDeviceContext::CXTPChartOpenGLDeviceContext(CXTPChartContainer* pContainer, HDC hDC, CSize szBounds, BOOL bWindowDC)
	: CXTPChartDeviceContext(pContainer)
{
	m_hDC = hDC;

	m_szBounds = szBounds;
	m_hDC = hDC;
	m_bWindowDC = TRUE;
	m_bDoubleBuffered = FALSE;
	m_hglrc = NULL;
	m_bNativeDrawing = FALSE;


	m_bWindowDC = bWindowDC;

	CreateNativeObjects();

	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);

	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_GENERIC_ACCELERATED | PFD_STEREO_DONTCARE;
	if (m_bWindowDC)
		pfd.dwFlags |= PFD_DRAW_TO_WINDOW | PFD_GENERIC_ACCELERATED | PFD_DOUBLEBUFFER;
	else
		pfd.dwFlags |= PFD_DRAW_TO_BITMAP;

	pfd.cAccumBits = 64;
	pfd.cStencilBits = 32;
	int pixelFormat = ChoosePixelFormat(m_hDC, &pfd);
	int count = DescribePixelFormat(m_hDC, pixelFormat, sizeof(pfd), &pfd);
	if (count == 0)
		return;

	if (pixelFormat != 0 && CreateContext(m_hDC, pixelFormat, pfd))
		return;

	for (int i = 1; i <= count; i++)
	{
		if (DescribePixelFormat(m_hDC, i, sizeof(pfd), &pfd) == 0 ||
			pfd.iPixelType != PFD_TYPE_RGBA || pfd.cStencilBits < 8 || pfd.cAccumBits < 32 || (pfd.dwFlags & PFD_SUPPORT_OPENGL) != PFD_SUPPORT_OPENGL)
			continue;

		if (m_bWindowDC)
		{
			if ((pfd.dwFlags & (PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER)) != (PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER))
				continue;
		}
		else if ((pfd.dwFlags & PFD_DRAW_TO_BITMAP) != PFD_DRAW_TO_BITMAP)
		{
			continue;
		}

		if (CreateContext(m_hDC, i, pfd))
				return;
	}
}

void CXTPChartOpenGLDeviceContext::CreateNativeObjects()
{
	CSize sz = m_szBounds;

	m_szBitmap.cx = 1;
	while (m_szBitmap.cx < m_szBounds.cx)
		m_szBitmap.cx *= 2;

	m_szBitmap.cy = 1;
	while (m_szBitmap.cy < m_szBounds.cy)
		m_szBitmap.cy *= 2;

	m_pGpBitmap = NULL;
	GdipCreateBitmapFromScan0(m_szBitmap.cx, m_szBitmap.cy, 0, PixelFormat32bppARGB, 0, &m_pGpBitmap);

	GdipGetImageGraphicsContext(m_pGpBitmap, &m_pGpGraphics);

	GdipGraphicsClear(m_pGpGraphics, Color::Transparent);

	GdipSetPageUnit(m_pGpGraphics, UnitPixel);
	//GdipSetSmoothingMode(m_pGpGraphics, SmoothingModeHighQuality);
	//GdipSetPixelOffsetMode(m_pGpGraphics, PixelOffsetModeHalf);
	GdipSetSmoothingMode(m_pGpGraphics, SmoothingModeHighSpeed);
	GdipSetTextRenderingHint(m_pGpGraphics, TextRenderingHintSingleBitPerPixelGridFit);

}

CXTPChartOpenGLDeviceContext::~CXTPChartOpenGLDeviceContext()
{
	if (m_hglrc)
	{
		wglDeleteContext(m_hglrc);
		m_hglrc = NULL;
	}

	if (m_pGpGraphics)
	{
		GdipDeleteGraphics(m_pGpGraphics);
		m_pGpGraphics = NULL;
	}

	if (m_pGpBitmap)
	{
		GdipDisposeImage(m_pGpBitmap);
		m_pGpBitmap = NULL;
	}

}

BOOL CXTPChartOpenGLDeviceContext::CreateContext(HDC hDC, int pixelFormat, PIXELFORMATDESCRIPTOR& pfd)
{
	if (!SetPixelFormat(hDC, pixelFormat, &pfd))
		return FALSE;

	m_hglrc = ::wglCreateContext(hDC);
	if (m_hglrc == NULL)
		return FALSE;

	m_bDoubleBuffered = (pfd.dwFlags & PFD_DOUBLEBUFFER) == PFD_DOUBLEBUFFER;
	return TRUE;
}

void CXTPChartOpenGLDeviceContext::Execute(CXTPChartDeviceCommand* pCommand)
{
#if 0
	_int64 nPerfomanceEnd;
	_int64 nPerfomanceStart;

	QueryPerformanceCounter((LARGE_INTEGER*)&nPerfomanceStart);
#endif

	if (pCommand == NULL || m_hglrc == NULL)
		return;

	if (!wglMakeCurrent(m_hDC, m_hglrc))
		return;

	glViewport(0, 0, m_szBounds.cx, m_szBounds.cy);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_SMOOTH);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, m_szBounds.cx, m_szBounds.cy, 0.0, 0.0, 1.0);

	m_bNativeDrawing = FALSE;

	pCommand->Execute(this);

	DrawBitmap();

	glFinish();

	if (m_bDoubleBuffered)
		SwapBuffers(m_hDC);


	wglMakeCurrent(0, 0);

#if 0
	QueryPerformanceCounter((LARGE_INTEGER*)&nPerfomanceEnd);
	TRACE(_T("CXTPChartOpenGLDeviceContext(%x)::Execute: %i \n"), (DWORD)(DWORD_PTR)m_pContainer, int(nPerfomanceEnd - nPerfomanceStart));
#endif
}

void CXTPChartOpenGLDeviceContext::CalculateColorComponents(CXTPChartColor& color, float res[4])
{
	res[0] = color.GetR() / 255.0f;
	res[1] = color.GetG() / 255.0f;
	res[2] = color.GetB() / 255.0f;
	res[3] = color.GetA() / 255.0f;
}

void CXTPChartOpenGLDeviceContext::SetNativeDrawing(BOOL bNativeDrawing)
{
	m_bNativeDrawing = bNativeDrawing;
}

void CXTPChartOpenGLDeviceContext::DrawBitmap()
{
	if (m_pGpBitmap == NULL)
		return;

	int nMaxTextureSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &nMaxTextureSize);
	UINT names;
	glGenTextures(1, &names);
	glBindTexture(GL_TEXTURE_2D, names);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_2D);
	glViewport(0, 0, m_szBounds.cx, m_szBounds.cy);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, m_szBounds.cx, m_szBounds.cy, 0.0, -1.0, 1.0);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0f, 0.0f, 0.0f, 0.0f);


	BitmapData BitmapData;

	GpRect rc(0, 0, m_szBitmap.cx, m_szBitmap.cy);
	GdipBitmapLockBits(m_pGpBitmap, &rc, ImageLockModeRead, PixelFormat32bppARGB, &BitmapData);

	if (m_szBitmap.cx <= nMaxTextureSize && m_szBitmap.cy <= nMaxTextureSize)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_szBitmap.cx, m_szBitmap.cy, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, BitmapData.Scan0);
		glBegin(GL_QUADS);
		glTexCoord2d(0, 0);
		glVertex3d(0, 0, 0.0);
		glTexCoord2d(0, 1);
		glVertex3d(0, 0 + BitmapData.Height, 0.0);
		glTexCoord2d(1, 1);
		glVertex3d(BitmapData.Width, BitmapData.Height, 0.0);
		glTexCoord2d(1, 0);
		glVertex3d(BitmapData.Width, 0, 0.0);
		glEnd();
	}
	else
	{
		int dx = m_szBitmap.cx;
		while (dx > nMaxTextureSize) dx /= 2;

		int dy = m_szBitmap.cy;
		while (dy > nMaxTextureSize) dy /= 2;

		DWORD* lpBits = (DWORD*)malloc(dx * dy * sizeof(DWORD));

		for (int y = 0; y < m_szBitmap.cy; y += dy)
		{
			for (int x = 0; x < m_szBitmap.cx; x += dx)
			{
				DWORD* pDest = lpBits;
				DWORD* pSrc = ((DWORD*)BitmapData.Scan0) + y * m_szBitmap.cx + x;

				for (int k = 0; k < dy; k++)
				{
					memcpy(pDest, pSrc, sizeof(DWORD) * dx);
					pDest += dx;
					pSrc += m_szBitmap.cx;
				}

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dx, dy, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, lpBits);
				glBegin(GL_QUADS);
				glTexCoord2d(0, 0);
				glVertex3d(x, y, 0.0);
				glTexCoord2d(0, 1);
				glVertex3d(x, y + dy, 0.0);
				glTexCoord2d(1, 1);
				glVertex3d(x + dx, y + dy, 0.0);
				glTexCoord2d(1, 0);
				glVertex3d(x + dx, y, 0.0);
				glEnd();
			}
		}

		free(lpBits);
	}
	GdipBitmapUnlockBits(m_pGpBitmap, &BitmapData);
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &names);
}
