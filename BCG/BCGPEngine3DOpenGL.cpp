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
// BCGPEngine3DOpenGL.cpp: implementation of the CBCGPEngine3DOpenGL class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPEngine3DOpenGL.h"
#include "BCGPDrawManager.h"
#include "BCGPGraphicsManagerGDI.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BOOL CBCGPOpenGLWrapper::m_bGLPresent = TRUE;
BOOL CBCGPOpenGLWrapper::m_bInitialized = FALSE;
HMODULE CBCGPOpenGLWrapper::m_hGLModule = NULL;

#ifndef BCGP_EXCLUDE_OPENGL

WGLGETPROCADDRESS	CBCGPOpenGLWrapper::m_pwglGetProcAddress = NULL;

WGLMAKECURRENT		CBCGPOpenGLWrapper::m_pwglMakeCurrent = NULL;
WGLCREATECONTEXT	CBCGPOpenGLWrapper::m_pwglCreateContext = NULL;
WGLDELETECONTEXT	CBCGPOpenGLWrapper::m_pwglDeleteContext = NULL;

GLSHADEMODEL	CBCGPOpenGLWrapper::m_pglShadeModel = NULL;
GLCLEARCOLOR	CBCGPOpenGLWrapper::m_pglClearColor = NULL;
GLCLEARDEPTH	CBCGPOpenGLWrapper::m_pglClearDepth = NULL;
GLCLEAR			CBCGPOpenGLWrapper::m_pglClear = NULL;
GLENABLE		CBCGPOpenGLWrapper::m_pglEnable = NULL;
GLDISABLE		CBCGPOpenGLWrapper::m_pglDisable = NULL;	
GLISENABLED		CBCGPOpenGLWrapper::m_pglIsEnabled = NULL;
GLDEPTHFUNC		CBCGPOpenGLWrapper::m_pglDepthFunc = NULL;
GLHINT			CBCGPOpenGLWrapper::m_pglHint = NULL;
GLALPHAFUNC		CBCGPOpenGLWrapper::m_pglAlphaFunc = NULL;
GLBLENDFUNC		CBCGPOpenGLWrapper::m_pglBlendFunc = NULL;
GLVIEWPORT		CBCGPOpenGLWrapper::m_pglViewport = NULL;
GLMATRIXMODE	CBCGPOpenGLWrapper::m_pglMatrixMode = NULL;
GLLOADIDENTITY	CBCGPOpenGLWrapper::m_pglLoadIdentity = NULL;
GLTRANSLATEF	CBCGPOpenGLWrapper::m_pglTranslatef = NULL;
GLTRANSLATED	CBCGPOpenGLWrapper::m_pglTranslated = NULL;
GLSCALED		CBCGPOpenGLWrapper::m_pglScaled = NULL;
GLDRAWBUFFER	CBCGPOpenGLWrapper::m_pglDrawBuffer = NULL;
GLREADBUFFER	CBCGPOpenGLWrapper::m_pglReadBuffer = NULL;
GLPIXELSTOREI	CBCGPOpenGLWrapper::m_pglPixelStorei = NULL;
GLREADPIXELS	CBCGPOpenGLWrapper::m_pglReadPixels = NULL;
GLLINEWIDTH		CBCGPOpenGLWrapper::m_pglLineWidth = NULL;
GLBEGIN			CBCGPOpenGLWrapper::m_pglBegin = NULL;	
GLEND			CBCGPOpenGLWrapper::m_pglEnd = NULL;
GLCOLOR4D		CBCGPOpenGLWrapper::m_pglColor4d = NULL;
GLVERTEX3D		CBCGPOpenGLWrapper::m_pglVertex3d = NULL;
GLNORMAL3D		CBCGPOpenGLWrapper::m_pglNormal3d = NULL;
GLNORMAL3DV		CBCGPOpenGLWrapper::m_pglNormal3dv = NULL;
GLPOLYGONMODE	CBCGPOpenGLWrapper::m_pglPolygonMode = NULL;
GLPOLYGONOFFSET	CBCGPOpenGLWrapper::m_pglPolygonOffset = NULL;
GLFLUSH			CBCGPOpenGLWrapper::m_pglFlush = NULL;
GLFINISH		CBCGPOpenGLWrapper::m_pglFinish = NULL; 

#endif //BCGP_EXCLUDE_OPENGL

IMPLEMENT_DYNAMIC(CBCGPEngine3DOpenGL, CBCGPEngine3D)

static  PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),      
		1,									// Version number
		PFD_DRAW_TO_WINDOW |				// PFD for window
 		PFD_SUPPORT_OPENGL |				// PFD for OpenGL
 		PFD_DOUBLEBUFFER,					// PFD for double buffering
		PFD_TYPE_RGBA,						// RGBA format
		32,									// color depth
		0,									// Bits for Red Channel
		0,									// ignored
		0,									// Bits for Green Channel
		0,									// ignored
		0,									// Bits for Blue Channel
		0,									// ignored
		0,									// Bits for Alpha Channel
		0,									// ignored
		0,									// 
		0, 0, 0, 0,							// 
		32,									// 32 bits z-buffer
		0,									// no trafaret buffer
		0,									// no additional buffers
		PFD_MAIN_PLANE,						// 
		0,									// reserved
		0, 0, 0								// ignore layer mask
	};

static DWORD dwFlagsWnd = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
static DWORD dwFlagsMem = PFD_SUPPORT_OPENGL | PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP;


static CBCGPOpenGLWrapper glWrapper;

//////////////////////////////////////////////////////////////////////
// CBCGPOpenGLWrapper Construction/Destruction
//////////////////////////////////////////////////////////////////////
CBCGPOpenGLWrapper::CBCGPOpenGLWrapper()
{

}

CBCGPOpenGLWrapper::~CBCGPOpenGLWrapper()
{
	if (m_hGLModule != NULL)
	{
		FreeLibrary(m_hGLModule);
	}
}

BOOL CBCGPOpenGLWrapper::InitWGL()
{
#ifdef BCGP_EXCLUDE_OPENGL
	return FALSE;
#else
	if (CBCGPOpenGLWrapper::m_bInitialized)
	{
		return TRUE;
	}

	if (!m_bGLPresent)
	{
		return FALSE;
	}

	m_hGLModule = LoadLibrary(_T("opengl32.dll"));

	if (m_hGLModule == NULL)
	{
		m_bGLPresent = FALSE;
		return FALSE;
	}

	CBCGPOpenGLWrapper::m_pwglGetProcAddress = (WGLGETPROCADDRESS)	GetProcAddress(m_hGLModule, "wglGetProcAddress");

	if (CBCGPOpenGLWrapper::m_pwglGetProcAddress == NULL)
	{
		m_bGLPresent = FALSE;
		return FALSE;
	}

	CBCGPOpenGLWrapper::m_pwglMakeCurrent	= (WGLMAKECURRENT)		GetProcAddress(m_hGLModule, "wglMakeCurrent");
	CBCGPOpenGLWrapper::m_pwglCreateContext = (WGLCREATECONTEXT)	GetProcAddress(m_hGLModule, "wglCreateContext");
	CBCGPOpenGLWrapper::m_pwglDeleteContext = (WGLDELETECONTEXT)	GetProcAddress(m_hGLModule, "wglDeleteContext");

	return TRUE;
#endif //BCGP_EXCLUDE_OPENGL
}

BOOL CBCGPOpenGLWrapper::InitGLCore()
{
#ifdef BCGP_EXCLUDE_OPENGL
	return FALSE;
#else
	if (CBCGPOpenGLWrapper::m_bInitialized)
	{
		return TRUE;
	}

	if (!m_bGLPresent)
	{
		return FALSE;
	}

	CBCGPOpenGLWrapper::m_pglShadeModel =	(GLSHADEMODEL)		GetProcAddress(m_hGLModule, "glShadeModel");
	CBCGPOpenGLWrapper::m_pglClearColor =	(GLCLEARCOLOR)		GetProcAddress(m_hGLModule, "glClearColor");
	CBCGPOpenGLWrapper::m_pglClearDepth =	(GLCLEARDEPTH)		GetProcAddress(m_hGLModule, "glClearDepth");
	CBCGPOpenGLWrapper::m_pglClear =		(GLCLEAR)			GetProcAddress(m_hGLModule, "glClear");
	CBCGPOpenGLWrapper::m_pglEnable =		(GLENABLE)			GetProcAddress(m_hGLModule, "glEnable");
	CBCGPOpenGLWrapper::m_pglDisable =		(GLDISABLE)			GetProcAddress(m_hGLModule, "glDisable");	
	CBCGPOpenGLWrapper::m_pglIsEnabled =	(GLISENABLED)		GetProcAddress(m_hGLModule, "glIsEnabled");	
	CBCGPOpenGLWrapper::m_pglDepthFunc =	(GLDEPTHFUNC)		GetProcAddress(m_hGLModule, "glDepthFunc");
	CBCGPOpenGLWrapper::m_pglHint =			(GLHINT)			GetProcAddress(m_hGLModule, "glHint");
	CBCGPOpenGLWrapper::m_pglAlphaFunc =	(GLALPHAFUNC)		GetProcAddress(m_hGLModule, "glAlphaFunc");
	CBCGPOpenGLWrapper::m_pglBlendFunc =	(GLBLENDFUNC)		GetProcAddress(m_hGLModule, "glBlendFunc");
	CBCGPOpenGLWrapper::m_pglViewport =		(GLVIEWPORT)		GetProcAddress(m_hGLModule, "glViewport");
	CBCGPOpenGLWrapper::m_pglMatrixMode =	(GLMATRIXMODE)		GetProcAddress(m_hGLModule, "glMatrixMode");
	CBCGPOpenGLWrapper::m_pglLoadIdentity = (GLLOADIDENTITY)	GetProcAddress(m_hGLModule, "glLoadIdentity");
	CBCGPOpenGLWrapper::m_pglTranslatef =	(GLTRANSLATEF)		GetProcAddress(m_hGLModule, "glTranslatef");
	CBCGPOpenGLWrapper::m_pglTranslated =	(GLTRANSLATED)		GetProcAddress(m_hGLModule, "glTranslated");
	CBCGPOpenGLWrapper::m_pglScaled =		(GLSCALED)			GetProcAddress(m_hGLModule, "glScaled");
	CBCGPOpenGLWrapper::m_pglDrawBuffer =	(GLDRAWBUFFER)		GetProcAddress(m_hGLModule, "glDrawBuffer");
	CBCGPOpenGLWrapper::m_pglReadBuffer =	(GLREADBUFFER)		GetProcAddress(m_hGLModule, "glReadBuffer");
	CBCGPOpenGLWrapper::m_pglPixelStorei =	(GLPIXELSTOREI)		GetProcAddress(m_hGLModule, "glPixelStorei");
	CBCGPOpenGLWrapper::m_pglReadPixels =	(GLREADPIXELS)		GetProcAddress(m_hGLModule, "glReadPixels");
	CBCGPOpenGLWrapper::m_pglLineWidth =	(GLLINEWIDTH)		GetProcAddress(m_hGLModule, "glLineWidth");
	CBCGPOpenGLWrapper::m_pglBegin =		(GLBEGIN)			GetProcAddress(m_hGLModule, "glBegin");	
	CBCGPOpenGLWrapper::m_pglEnd =			(GLEND)				GetProcAddress(m_hGLModule, "glEnd");
	CBCGPOpenGLWrapper::m_pglColor4d =		(GLCOLOR4D)			GetProcAddress(m_hGLModule, "glColor4d");
	CBCGPOpenGLWrapper::m_pglVertex3d =		(GLVERTEX3D)		GetProcAddress(m_hGLModule, "glVertex3d");
	CBCGPOpenGLWrapper::m_pglNormal3d =		(GLNORMAL3D)		GetProcAddress(m_hGLModule, "glNormal3d");
	CBCGPOpenGLWrapper::m_pglNormal3dv =	(GLNORMAL3DV)		GetProcAddress(m_hGLModule, "glNormal3dv");
	CBCGPOpenGLWrapper::m_pglPolygonMode =	(GLPOLYGONMODE)		GetProcAddress(m_hGLModule, "glPolygonMode");
	CBCGPOpenGLWrapper::m_pglPolygonOffset = (GLPOLYGONOFFSET)	GetProcAddress(m_hGLModule, "glPolygonOffset");
	CBCGPOpenGLWrapper::m_pglFlush =		(GLFLUSH)			GetProcAddress(m_hGLModule, "glFlush");
	CBCGPOpenGLWrapper::m_pglFinish =		(GLFINISH)			GetProcAddress(m_hGLModule, "glFinish"); 

	CBCGPOpenGLWrapper::m_bInitialized = TRUE;
	return TRUE;
#endif //BCGP_EXCLUDE_OPENGL
}

//////////////////////////////////////////////////////////////////////
// CBCGPEngine3DOpenGL Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPEngine3DOpenGL::CBCGPEngine3DOpenGL()
{
#ifndef BCGP_EXCLUDE_OPENGL
	m_pWndGL = NULL;
	m_hGLRC  = NULL;
	m_hGLDC = NULL;
	m_bRenderToWindow = TRUE;

	m_hMemBmp = NULL;
	m_hMemBmpOld= NULL;

	// default OpenGL normal
	m_vNormal[0] = 0.;
	m_vNormal[1] = 0.;
	m_vNormal[2] = 1.;
#endif //BCGP_EXCLUDE_OPENGL
}
//****************************************************************************************
CBCGPEngine3DOpenGL::~CBCGPEngine3DOpenGL()
{
#ifndef BCGP_EXCLUDE_OPENGL
	ShutDown();
#endif //BCGP_EXCLUDE_OPENGL
}
//****************************************************************************************
BOOL CBCGPEngine3DOpenGL::Initialize(BOOL bRenderToWindow)
{
#ifdef BCGP_EXCLUDE_OPENGL
	UNREFERENCED_PARAMETER(bRenderToWindow);
	m_bInitialized = FALSE;
	return FALSE;
#else

	if (bRenderToWindow != m_bRenderToWindow)
	{
		ShutDown();
		m_bRenderToWindow = bRenderToWindow;
	}

	if (m_bInitialized)
	{
		return TRUE;
	}

	if (!CBCGPOpenGLWrapper::InitWGL())
	{
		ShutDown();
		return FALSE;
	}

	if (m_bRenderToWindow)
	{
		if (!InitGLWindow())
		{
			ShutDown();
			return FALSE;
		}

		if (!SetUpPixelFormat(m_hGLDC, dwFlagsWnd))
		{
			ShutDown();
			return FALSE;
		}

		if (!InitGLContext(m_hGLDC))
		{
			ShutDown();
			return FALSE;
		}

		if (!CBCGPOpenGLWrapper::InitGLCore())
		{
			ShutDown();
			return FALSE;
		}
	}
	else
	{
		HWND h = GetDesktopWindow();
		HDC hdcScreen = GetDC(h);
		m_hGLDC = CreateCompatibleDC(hdcScreen);
		ReleaseDC(h, hdcScreen);
		DeleteDC(hdcScreen);

		if (!CreateGLBitmap(10, 10))
		{
			ShutDown();
			return FALSE;
		}
	}

	if (!CBCGPOpenGLWrapper::InitGLCore())
	{
		return FALSE;
	}

	InitGL();

	m_bInitialized = TRUE;
	return TRUE;

#endif //BCGP_EXCLUDE_OPENGL
}
//****************************************************************************************

#ifndef BCGP_EXCLUDE_OPENGL

BOOL CBCGPEngine3DOpenGL::InitGLWindow()
{
	if (m_pWndGL != NULL)
	{
		return TRUE;
	}

	CString strClassName = ::AfxRegisterWndClass (CS_OWNDC, ::LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_BTNFACE + 1), NULL);
	CRect r(0, 0, 1, 1);

	m_pWndGL = new CWnd();

	if (!m_pWndGL->CreateEx(0, strClassName, _T("BCGCBPro OpenGL Dummy Window"), WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, r, NULL, 0))
	{
		ShutDown();
		return FALSE;
	}

	m_hGLDC = m_pWndGL->GetDC()->GetSafeHdc();

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPEngine3DOpenGL::SetUpPixelFormat(HDC hDC, DWORD dwFormatFlags)
{
	pfd.dwFlags = dwFormatFlags;

	GLuint PixelFormat = ChoosePixelFormat(hDC, &pfd);

	if (PixelFormat == 0)
	{
		TRACE0("CBCGPEngine3DOpenGL::SetUpPixelFormat: ChoosePixelFormat failed.\n");
		return FALSE;
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))
	{
		TRACE0("CBCGPEngine3DOpenGL::SetUpPixelFormat: SetPixelFormat failed.\n");
		return FALSE;
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPEngine3DOpenGL::InitGLContext(HDC hDC)
{
	if (m_hGLRC == NULL)
	{
		m_hGLRC = (*CBCGPOpenGLWrapper::m_pwglCreateContext)(hDC);
	}
	
	if (m_hGLRC == NULL)
	{
		TRACE0("CBCGPEngine3DOpenGL::InitGLContext: wglCreateContext failed.\n");
		return FALSE;
	}

	if (!(*CBCGPOpenGLWrapper::m_pwglMakeCurrent)(hDC, m_hGLRC))
	{
		TRACE0("CBCGPEngine3DOpenGL::InitGLContext: wglMakeCurrent failed.\n");
		return FALSE;
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPEngine3DOpenGL::CreateGLBitmap(int nWidth, int nHeight)
{
	if (m_hGLDC != NULL && m_hMemBmp != NULL && m_hMemBmpOld != NULL)
	{
		SelectObject(m_hGLDC, m_hMemBmpOld);
		DeleteObject(m_hMemBmp);
	}

	LPVOID pBits = NULL;
	m_hMemBmp = CBCGPDrawManager::CreateBitmap_32(CSize(nWidth, nHeight), &pBits);
	m_hMemBmpOld = (HBITMAP)SelectObject(m_hGLDC, m_hMemBmp);

	if (m_hGLRC == NULL)
	{
		if (!SetUpPixelFormat(m_hGLDC, dwFlagsMem))
		{
			return FALSE;
		}

		if (!InitGLContext(m_hGLDC))
		{
			return FALSE;
		}
	}

	return TRUE;
}
//****************************************************************************************
void CBCGPEngine3DOpenGL::InitGL()
{
	(*CBCGPOpenGLWrapper::m_pglShadeModel)(GL_SMOOTH);
	(*CBCGPOpenGLWrapper::m_pglClearColor)(0.0f, 0.0f, 0.0f, 0.0f);
	(*CBCGPOpenGLWrapper::m_pglClearDepth)(1.0f);

	(*CBCGPOpenGLWrapper::m_pglEnable)(GL_DEPTH_TEST);
	(*CBCGPOpenGLWrapper::m_pglDepthFunc)(GL_LEQUAL);

	(*CBCGPOpenGLWrapper::m_pglEnable)(GL_ALPHA_TEST);
 	(*CBCGPOpenGLWrapper::m_pglAlphaFunc)(GL_GREATER, 0.);

	(*CBCGPOpenGLWrapper::m_pglEnable)(GL_BLEND);
	(*CBCGPOpenGLWrapper::m_pglBlendFunc)(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	(*CBCGPOpenGLWrapper::m_pglEnable)(GL_LINE_SMOOTH);  
  	(*CBCGPOpenGLWrapper::m_pglHint)(GL_LINE_SMOOTH_HINT, GL_NICEST);


// 	glEnable(GL_LIGHT0);
// 	glEnable(GL_LIGHTING);
// 	glEnable(GL_COLOR_MATERIAL);


//     GLfloat light_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
//     GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
//     GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
//     GLfloat light_position[] = { 1.0, 3.5, 0.8, 0.0 };
// 
//     GLfloat global_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
// 
//     glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
//     glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
//     glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);
//     glLightfv (GL_LIGHT0, GL_POSITION, light_position);
//     glLightModelfv (GL_LIGHT_MODEL_AMBIENT, global_ambient);

}
//****************************************************************************************
void CBCGPEngine3DOpenGL::ShutDown()
{
	if (CBCGPOpenGLWrapper::m_pwglMakeCurrent != NULL)
	{
		(*CBCGPOpenGLWrapper::m_pwglMakeCurrent)(NULL, NULL);
	}

	if (m_hGLRC != NULL)
	{
		(*CBCGPOpenGLWrapper::m_pwglDeleteContext)(m_hGLRC);
		m_hGLRC = NULL;
	}

	if (m_hMemBmp != NULL)
	{
		if (m_hMemBmpOld != NULL && m_hGLDC != NULL)
		{
			SelectObject(m_hGLDC, m_hMemBmpOld);
		}

		DeleteObject(m_hMemBmp);
		m_hMemBmp = NULL;
	}

	if (m_pWndGL != NULL && m_pWndGL->GetSafeHwnd() != NULL && m_hGLDC != NULL)
	{
		ReleaseDC(m_pWndGL->GetSafeHwnd(), m_hGLDC);
	}

	if (m_hGLDC != NULL)
	{
		DeleteDC(m_hGLDC);
		m_hGLDC = NULL;
	}

	if (m_pWndGL != NULL)
	{
		if (m_pWndGL->GetSafeHwnd() != NULL)
		{
			m_pWndGL->DestroyWindow();
		}

		delete m_pWndGL;
		m_pWndGL = NULL;
	}

	m_bInitialized = FALSE;
}
//****************************************************************************************
void CBCGPEngine3DOpenGL::SetRenderToWindow(BOOL bSet)
{
	if (m_bRenderToWindow == bSet)
	{
		return;
	}

	m_bRenderToWindow = bSet;

	if (!m_bInitialized)
	{
		return;
	}

	ShutDown();
	Initialize(bSet);
}
//****************************************************************************************
void CBCGPEngine3DOpenGL::SetClearColor(const CBCGPColor& clrClear)
{
	CBCGPEngine3D::SetClearColor(clrClear);

	if (m_bInitialized)
	{
		(*CBCGPOpenGLWrapper::m_pglClearColor)((float)clrClear.r, (float)clrClear.g, (float)clrClear.b, 1.0f);
	}
}
//****************************************************************************************
void CBCGPEngine3DOpenGL::SetSceneRectAndDepth(const CBCGPRect& rect, double dblZMin, double dblZMax)
{
	CBCGPEngine3D::SetSceneRectAndDepth(rect, dblZMin, dblZMax);

	if (m_bRenderToWindow && m_pWndGL == NULL)
	{
		return;
	}

	int nWidth = (int)rect.Width();

	if (nWidth < 1)
	{
		nWidth = 1;
	}

	int nHeight = (int)rect.Height();

	if (nHeight < 1)
	{
		nHeight = 1;
	}

	if (m_bRenderToWindow)
	{
		m_pWndGL->SetWindowPos(NULL, -1, -1, nWidth, nHeight, 
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOACTIVATE);
	}
	else
	{
		CreateGLBitmap(nWidth, nHeight);
	}

	if (m_hGLDC != NULL && m_hGLRC != NULL)
	{
		(*CBCGPOpenGLWrapper::m_pwglMakeCurrent)(m_hGLDC, m_hGLRC);
	}

	(*CBCGPOpenGLWrapper::m_pglViewport)(0, 0, nWidth, nHeight);

	(*CBCGPOpenGLWrapper::m_pglMatrixMode)(GL_PROJECTION);
    (*CBCGPOpenGLWrapper::m_pglLoadIdentity)();

	(*CBCGPOpenGLWrapper::m_pglMatrixMode)(GL_MODELVIEW);
    (*CBCGPOpenGLWrapper::m_pglLoadIdentity)();

 	(*CBCGPOpenGLWrapper::m_pglTranslatef)(-1.0f, 1.0f, 0.0f);
 	(*CBCGPOpenGLWrapper::m_pglScaled)(2. / rect.Width(), -2. / rect.Height(), .1 / ((dblZMax - dblZMin) + 0.1));

	(*CBCGPOpenGLWrapper::m_pglTranslated)(-rect.left, -rect.top, 0.);
}
//****************************************************************************************
void CBCGPEngine3DOpenGL::BeginDraw(CBCGPGraphicsManager* pGMTarget)
{
	if (m_hGLDC == NULL)
	{
		CBCGPEngine3D::BeginDraw(pGMTarget);
		return;
	}

	if (m_bRenderToWindow)
	{
		(*CBCGPOpenGLWrapper::m_pglDrawBuffer)(GL_BACK);
	}
	
 	if (!(*CBCGPOpenGLWrapper::m_pwglMakeCurrent)(m_hGLDC, m_hGLRC))
 	{
 		ASSERT(FALSE);
 		ShutDown();
 		return;
 	}

	m_pDefaultGM = pGMTarget;

	(*CBCGPOpenGLWrapper::m_pglClear)(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

#ifdef _BCGSUITE_
static BOOL DrawAlpha(CBCGPGraphicsManager* pGMTarget, HBITMAP hBmp, const CRect& rectTarget)
{
	if (pGMTarget != NULL && pGMTarget->GetType() == CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_GDI)
	{
		CBCGPGraphicsManagerGDI* pGMGDI = DYNAMIC_DOWNCAST(CBCGPGraphicsManagerGDI, pGMTarget);
		if (pGMGDI != NULL)
		{
			CDC* pDC = pGMGDI->GetDCPaint();
			ASSERT_VALID(pDC);

			CDC dcMem;
			if (dcMem.CreateCompatibleDC(pDC))
			{
				HBITMAP hBitmapOld = (HBITMAP)dcMem.SelectObject(hBmp);
				if (hBitmapOld != NULL)
				{
					BITMAP bmp;
					::GetObject(hBmp, sizeof(BITMAP), (LPVOID)&bmp);

					BLENDFUNCTION pixelblend = { AC_SRC_OVER, 0, 255, (BYTE)0 };
					pDC->AlphaBlend(rectTarget.left, rectTarget.top, rectTarget.Width(), rectTarget.Height(),
						&dcMem, 0, 0, bmp.bmWidth, abs(bmp.bmHeight), pixelblend);

					dcMem.SelectObject(hBitmapOld);
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}
#endif

void CBCGPEngine3DOpenGL::EndDraw(const CBCGPRect& rectTarget, CBCGPGraphicsManager* pGMTarget)
{
	if (m_hGLDC == NULL)
	{
		CBCGPEngine3D::EndDraw(rectTarget, pGMTarget);
		return;
	}

	if (pGMTarget == NULL && m_pDefaultGM == NULL)
	{
		TRACE0("Target Graphics manager is not specified.\n");
		return;
	}

	(*CBCGPOpenGLWrapper::m_pglFlush)();
	(*CBCGPOpenGLWrapper::m_pglFinish)();

	if (pGMTarget == NULL && m_pDefaultGM != NULL)
	{
		pGMTarget = m_pDefaultGM;
	}

	BOOL bIsReady = FALSE;

	if (m_bRenderToWindow)
	{
		LPVOID pBits = NULL;
		HBITMAP hbmp = CBCGPDrawManager::CreateBitmap_32(m_rectScene.Size(), &pBits);

		(*CBCGPOpenGLWrapper::m_pglReadBuffer)(GL_BACK);

		(*CBCGPOpenGLWrapper::m_pglPixelStorei)(GL_UNPACK_ALIGNMENT, 4);
		(*CBCGPOpenGLWrapper::m_pglReadPixels)(0, 0, (int)m_rectScene.Width(), (int)m_rectScene.Height(), GL_BGRA_EXT, GL_UNSIGNED_BYTE, pBits);

#ifdef _BCGSUITE_
		bIsReady = DrawAlpha(pGMTarget, hbmp, rectTarget);
#endif
		if (!bIsReady)
		{
			CBCGPImage image(hbmp, TRUE);
			pGMTarget->DrawImage(image, rectTarget.TopLeft(), rectTarget.Size());
		}
		
		DeleteObject(hbmp);
	}
	else
	{
		SelectObject(m_hGLDC, m_hMemBmpOld);

#ifdef _BCGSUITE_
		bIsReady = DrawAlpha(pGMTarget, m_hMemBmp, rectTarget);
#endif
		if (!bIsReady)
		{
			CBCGPImage image(m_hMemBmp, TRUE);
			pGMTarget->DrawImage(image, rectTarget.TopLeft(), rectTarget.Size());
		}

		m_hMemBmpOld = (HBITMAP)SelectObject(m_hGLDC, m_hMemBmp);
	}

	m_pDefaultGM = NULL;
}
//****************************************************************************************
BOOL CBCGPEngine3DOpenGL::EnableAntialiasing(BOOL bEnable)
{
	if (!m_bInitialized)
	{
		return CBCGPEngine3D::EnableAntialiasing(bEnable);
	}

	GLboolean bPrevValue = (*CBCGPOpenGLWrapper::m_pglIsEnabled)(GL_LINE_SMOOTH);

	if (bEnable)
	{
		(*CBCGPOpenGLWrapper::m_pglEnable)(GL_LINE_SMOOTH);
	}
	else
	{
		(*CBCGPOpenGLWrapper::m_pglDisable)(GL_LINE_SMOOTH);
	}

	return (BOOL)bPrevValue;
}
//****************************************************************************************
void CBCGPEngine3DOpenGL::SetPolygonNormal(double nx, double ny, double nz)
{
	if (!m_bInitialized)
	{
		CBCGPEngine3D::SetPolygonNormal(nx, ny, nz);
		return; 
	}

	m_vNormal[0] = nx;
	m_vNormal[1] = ny;
	m_vNormal[2] = nz;
}
//****************************************************************************************
void CBCGPEngine3DOpenGL::DrawLine(const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo, const CBCGPBrush& brush, 
		double lineWidth, const CBCGPStrokeStyle* pStrokeStyle)
{
	if (!m_bInitialized)
	{
		CBCGPEngine3D::DrawLine(ptFrom, ptTo, brush, lineWidth, pStrokeStyle);
		return;
	}

	CBCGPColor color = brush.GetColor();
	double dblOpacity = m_bForceNoTransparency ? 1. : brush.GetOpacity();

	(*CBCGPOpenGLWrapper::m_pglLineWidth)((float)lineWidth);
	(*CBCGPOpenGLWrapper::m_pglBegin)(GL_LINES);

		(*CBCGPOpenGLWrapper::m_pglNormal3dv)(m_vNormal);
		(*CBCGPOpenGLWrapper::m_pglColor4d)(color.r, color.g, color.b, dblOpacity);
		(*CBCGPOpenGLWrapper::m_pglVertex3d)(ptFrom.x, ptFrom.y, ptFrom.z);
		(*CBCGPOpenGLWrapper::m_pglVertex3d)(ptTo.x, ptTo.y, ptTo.z);

	(*CBCGPOpenGLWrapper::m_pglEnd)();
}
//****************************************************************************************
void CBCGPEngine3DOpenGL::DrawPolygon(const CBCGPPointsArray& arPoints, const CBCGPBrush& brush, double dblLineWidth)
{
	if (!m_bInitialized)
	{
		CBCGPEngine3D::DrawPolygon(arPoints, brush, dblLineWidth);
		return;
	}

	CBCGPColor color = brush.GetColor();
	double dblOpacity = m_bForceNoTransparency ? 1. : brush.GetOpacity();

	(*CBCGPOpenGLWrapper::m_pglLineWidth)((float)dblLineWidth);

	(*CBCGPOpenGLWrapper::m_pglBegin)(GL_LINE_LOOP);

		(*CBCGPOpenGLWrapper::m_pglColor4d)(color.r, color.g, color.b, dblOpacity);
		(*CBCGPOpenGLWrapper::m_pglNormal3dv)(m_vNormal);

		for (int i = 0; i < arPoints.GetSize(); i++)
		{
			(*CBCGPOpenGLWrapper::m_pglVertex3d)(arPoints[i].x, arPoints[i].y, arPoints[i].z);
		}

	(*CBCGPOpenGLWrapper::m_pglEnd)();
}
//****************************************************************************************
void CBCGPEngine3DOpenGL::FillPolygon(const CBCGPPointsArray& arPoints, const CBCGPBrush& brush)
{
	if (!m_bInitialized)
	{
		CBCGPEngine3D::FillPolygon(arPoints, brush);
		return;
	}

	CBCGPColor color = brush.GetColor();
	double dblOpacity = m_bForceNoTransparency ? 1. : brush.GetOpacity();

	if ((dblOpacity < 1. || m_bForceDisableDepthTest) && !m_bForceEnableDepthTest)
	{
		(*CBCGPOpenGLWrapper::m_pglDisable)(GL_DEPTH_TEST);
	}

	(*CBCGPOpenGLWrapper::m_pglPolygonMode)(GL_FRONT_AND_BACK, GL_FILL);

  	(*CBCGPOpenGLWrapper::m_pglEnable)(GL_POLYGON_OFFSET_FILL);
  	(*CBCGPOpenGLWrapper::m_pglPolygonOffset)(1.0, 1.0);

	{
		(*CBCGPOpenGLWrapper::m_pglBegin)(GL_POLYGON);
			(*CBCGPOpenGLWrapper::m_pglColor4d)(color.r, color.g, color.b, dblOpacity);
			(*CBCGPOpenGLWrapper::m_pglNormal3dv)(m_vNormal);

		for (int i = 0; i < arPoints.GetSize(); i++)
		{
			(*CBCGPOpenGLWrapper::m_pglVertex3d)(arPoints[i].x, arPoints[i].y, arPoints[i].z);
		}
		
		(*CBCGPOpenGLWrapper::m_pglEnd)();
	}

	if ((dblOpacity < 1. || m_bForceDisableDepthTest) && !m_bForceEnableDepthTest)
	{
		(*CBCGPOpenGLWrapper::m_pglEnable)(GL_DEPTH_TEST);
	}

	(*CBCGPOpenGLWrapper::m_pglDisable)(GL_POLYGON_OFFSET_FILL);
}
//****************************************************************************************
void CBCGPEngine3DOpenGL::FillGeometry(const CBCGPGeometry& geometry, const CBCGPBrush& brush)
{
	if (!m_bInitialized)
	{
		CBCGPEngine3D::FillGeometry(geometry, brush);
		return;
	}

	const CBCGPPolygonGeometry* g = DYNAMIC_DOWNCAST(CBCGPPolygonGeometry, &geometry);

	if (g == NULL)
	{
		return;
	}

	const CBCGPPointsArray& arPoints = g->GetPoints();
	FillPolygon(arPoints, brush);
}
//****************************************************************************************
void CBCGPEngine3DOpenGL::DrawSide(const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3, const CBCGPPoint& pt4,
										const CBCGPBrush& brFill, const CBCGPBrush& brLine, double dblLineWidth,
										BOOL bFill, BOOL bDrawLine)
{
	if (!m_bInitialized)
	{
		CBCGPEngine3D::DrawSide(pt1, pt2, pt3, pt4, brFill, brLine, dblLineWidth);
		return;
	}

	if (bFill && !brFill.IsEmpty())
	{
		CBCGPColor color = brFill.GetColor();
		double dblFillOpacity = m_bForceNoTransparency ? 1.0 : brFill.GetOpacity();

		if ((dblFillOpacity < 1. || m_bForceDisableDepthTest) && !m_bForceEnableDepthTest)
		{
			(*CBCGPOpenGLWrapper::m_pglDisable)(GL_DEPTH_TEST);
		}
		
		(*CBCGPOpenGLWrapper::m_pglPolygonMode)(GL_FRONT_AND_BACK, GL_FILL);

		(*CBCGPOpenGLWrapper::m_pglEnable)(GL_POLYGON_OFFSET_FILL);
		(*CBCGPOpenGLWrapper::m_pglPolygonOffset)(1.0, 1.0);

		(*CBCGPOpenGLWrapper::m_pglBegin)(GL_POLYGON);
			(*CBCGPOpenGLWrapper::m_pglColor4d)(color.r, color.g, color.b, dblFillOpacity);
			(*CBCGPOpenGLWrapper::m_pglNormal3dv)(m_vNormal);

			(*CBCGPOpenGLWrapper::m_pglVertex3d)(pt1.x, pt1.y, pt1.z);
			(*CBCGPOpenGLWrapper::m_pglVertex3d)(pt2.x, pt2.y, pt2.z);
			(*CBCGPOpenGLWrapper::m_pglVertex3d)(pt3.x, pt3.y, pt3.z);
			(*CBCGPOpenGLWrapper::m_pglVertex3d)(pt4.x, pt4.y, pt4.z);
		(*CBCGPOpenGLWrapper::m_pglEnd)();

		(*CBCGPOpenGLWrapper::m_pglDisable)(GL_POLYGON_OFFSET_FILL);

		if ((dblFillOpacity < 1. || m_bForceDisableDepthTest) && !m_bForceEnableDepthTest)
		{
			(*CBCGPOpenGLWrapper::m_pglEnable)(GL_DEPTH_TEST);
		}
	}

	if (bDrawLine && !brLine.IsEmpty())
	{
		(*CBCGPOpenGLWrapper::m_pglPolygonMode)(GL_FRONT_AND_BACK, GL_LINE);

		CBCGPColor color = brLine.GetColor();
		(*CBCGPOpenGLWrapper::m_pglLineWidth)((float)dblLineWidth);

		(*CBCGPOpenGLWrapper::m_pglBegin)(GL_LINE_LOOP);
			(*CBCGPOpenGLWrapper::m_pglColor4d)(color.r, color.g, color.b, brLine.GetOpacity());
			(*CBCGPOpenGLWrapper::m_pglNormal3dv)(m_vNormal);

			(*CBCGPOpenGLWrapper::m_pglVertex3d)(pt1.x, pt1.y, pt1.z);
			(*CBCGPOpenGLWrapper::m_pglVertex3d)(pt2.x, pt2.y, pt2.z);
			(*CBCGPOpenGLWrapper::m_pglVertex3d)(pt3.x, pt3.y, pt3.z);
			(*CBCGPOpenGLWrapper::m_pglVertex3d)(pt4.x, pt4.y, pt4.z);
		(*CBCGPOpenGLWrapper::m_pglEnd)();
	}
}

#endif // BCGP_EXCLUDE_OPENGL