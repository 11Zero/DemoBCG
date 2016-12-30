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
// BCGPEngine3DOpenGL.h: interface for the CBCGPEngine3DOpenGL class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPENGINE3DOPENGL_H__E9670488_59D2_418B_88F1_5E9AE644561F__INCLUDED_)
#define AFX_BCGPENGINE3DOPENGL_H__E9670488_59D2_418B_88F1_5E9AE644561F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPEngine3D.h"
#include "BCGPMath.h"

#ifndef BCGP_EXCLUDE_OPENGL

#include <gl/gl.h>
#include <gl/glu.h>

typedef PROC  (WINAPI * WGLGETPROCADDRESS)(LPCSTR);

typedef BOOL  (WINAPI * WGLMAKECURRENT)(HDC, HGLRC);
typedef HGLRC (WINAPI * WGLCREATECONTEXT)(HDC);
typedef BOOL  (WINAPI * WGLDELETECONTEXT)(HGLRC);

typedef void (APIENTRY * GLSHADEMODEL)(GLenum mode);
typedef void (APIENTRY * GLCLEARCOLOR) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (APIENTRY * GLCLEAR) (GLbitfield mask);
typedef void (APIENTRY * GLCLEARDEPTH) (GLclampd depth);
typedef void (APIENTRY * GLENABLE) (GLenum cap);
typedef void (APIENTRY * GLDISABLE) (GLenum cap);

typedef GLboolean (APIENTRY * GLISENABLED) (GLenum cap);

typedef void (APIENTRY * GLDEPTHFUNC) (GLenum func);
typedef void (APIENTRY * GLHINT) (GLenum target, GLenum mode);
typedef void (APIENTRY * GLALPHAFUNC) (GLenum func, GLclampf ref);
typedef void (APIENTRY * GLBLENDFUNC) (GLenum sfactor, GLenum dfactor);
typedef void (APIENTRY * GLVIEWPORT) (GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (APIENTRY * GLMATRIXMODE) (GLenum mode);
typedef void (APIENTRY * GLLOADIDENTITY) (void);
typedef void (APIENTRY * GLTRANSLATEF) (GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * GLTRANSLATED) (GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY * GLSCALED) (GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY * GLDRAWBUFFER) (GLenum mode);
typedef void (APIENTRY * GLREADBUFFER) (GLenum mode);
typedef void (APIENTRY * GLPIXELSTOREI) (GLenum pname, GLint param);
typedef void (APIENTRY * GLREADPIXELS) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);

typedef void (APIENTRY * GLLINEWIDTH) (GLfloat width);
typedef void (APIENTRY * GLBEGIN) (GLenum mode);
typedef void (APIENTRY * GLEND) (void);

typedef void (APIENTRY * GLCOLOR4D) (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
typedef void (APIENTRY * GLVERTEX3D) (GLdouble x, GLdouble y, GLdouble z);
typedef void (APIENTRY * GLNORMAL3D) (GLdouble nx, GLdouble ny, GLdouble nz);
typedef void (APIENTRY * GLNORMAL3DV) (const GLdouble *v);

typedef void (APIENTRY * GLPOLYGONMODE) (GLenum face, GLenum mode);
typedef void (APIENTRY * GLPOLYGONOFFSET) (GLfloat factor, GLfloat units);

typedef void (APIENTRY * GLFLUSH) (void);
typedef void (APIENTRY * GLFINISH) (void);

#endif //BCGP_EXCLUDE_OPENGL

class BCGCBPRODLLEXPORT CBCGPOpenGLWrapper
{
public:
	CBCGPOpenGLWrapper();
	~CBCGPOpenGLWrapper();
	
	static BOOL InitWGL();
	static BOOL InitGLCore();

	static BOOL IsInitialized() {return m_bInitialized;}

#ifndef BCGP_EXCLUDE_OPENGL
	static WGLGETPROCADDRESS	m_pwglGetProcAddress;

	static WGLMAKECURRENT	m_pwglMakeCurrent;
	static WGLCREATECONTEXT	m_pwglCreateContext;
	static WGLDELETECONTEXT	m_pwglDeleteContext;

	static GLSHADEMODEL		m_pglShadeModel;
	static GLCLEARCOLOR		m_pglClearColor;
	static GLCLEARDEPTH		m_pglClearDepth;
	static GLCLEAR			m_pglClear;
	static GLENABLE			m_pglEnable;
	static GLDISABLE		m_pglDisable;
	static GLISENABLED		m_pglIsEnabled;
	static GLDEPTHFUNC		m_pglDepthFunc;
	static GLHINT			m_pglHint;
	static GLALPHAFUNC		m_pglAlphaFunc;
	static GLBLENDFUNC		m_pglBlendFunc;
	static GLVIEWPORT		m_pglViewport;
	static GLMATRIXMODE		m_pglMatrixMode;
	static GLLOADIDENTITY	m_pglLoadIdentity;
	static GLTRANSLATEF		m_pglTranslatef;
	static GLTRANSLATED		m_pglTranslated;
	static GLSCALED			m_pglScaled;
	static GLDRAWBUFFER		m_pglDrawBuffer;
	static GLREADBUFFER		m_pglReadBuffer;
	static GLPIXELSTOREI	m_pglPixelStorei;
	static GLREADPIXELS		m_pglReadPixels;
	static GLLINEWIDTH		m_pglLineWidth;
	static GLBEGIN			m_pglBegin;	
	static GLEND			m_pglEnd;
	static GLCOLOR4D		m_pglColor4d;
	static GLVERTEX3D		m_pglVertex3d;
	static GLNORMAL3D		m_pglNormal3d;
	static GLNORMAL3DV		m_pglNormal3dv;
	static GLPOLYGONMODE	m_pglPolygonMode;
	static GLPOLYGONOFFSET	m_pglPolygonOffset;
	static GLFLUSH			m_pglFlush;
	static GLFINISH			m_pglFinish; 
#endif

protected:
	static BOOL m_bGLPresent;
	static BOOL m_bInitialized;

	static HMODULE m_hGLModule;
};

class BCGCBPRODLLEXPORT CBCGPEngine3DOpenGL : public CBCGPEngine3D  
{
	DECLARE_DYNAMIC(CBCGPEngine3DOpenGL);

public:
	CBCGPEngine3DOpenGL();
	virtual ~CBCGPEngine3DOpenGL();

	virtual BOOL Initialize(BOOL bRenderToWindow);

	virtual BCGP_3D_RENDERING_TYPE GetRenderingType() const {return CBCGPEngine3D::BCGP_RT_OPENGL;}
	virtual BOOL IsSoftwareRendering() const {return FALSE;}

#ifndef BCGP_EXCLUDE_OPENGL
	virtual void ShutDown();

	virtual void SetRenderToWindow(BOOL bSet);
	virtual BOOL IsRenderToWindow() const {return m_bRenderToWindow;}

	virtual void BeginDraw(CBCGPGraphicsManager* pGMTarget = NULL);
	virtual void EndDraw(const CBCGPRect& rectTarget, CBCGPGraphicsManager* pGMTarget = NULL);

	virtual BOOL EnableAntialiasing(BOOL bEnable = TRUE);
	virtual void SetPolygonNormal(double nx, double ny, double nz);

	virtual void SetClearColor(const CBCGPColor& clrClear);

	virtual void SetSceneRectAndDepth(const CBCGPRect& rect, double dblZMin, double dblZMax);

	virtual void DrawLine(const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo, const CBCGPBrush& brush, 
		double lineWidth = 1., const CBCGPStrokeStyle* pStrokeStyle = NULL);
	virtual void DrawPolygon(const CBCGPPointsArray& arPoints, const CBCGPBrush& brush, double dblLineWidth = 1.);
	virtual void FillPolygon(const CBCGPPointsArray& arPoints, const CBCGPBrush& brush);
	virtual void FillGeometry(const CBCGPGeometry& geometry, const CBCGPBrush& brush);
	virtual void DrawSide(const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3, const CBCGPPoint& pt4,
		const CBCGPBrush& brFill, const CBCGPBrush& brLine, double dblLineWidth = 1.,
		BOOL bFill = TRUE, BOOL bDrawLine = TRUE);

protected:
	virtual BOOL InitGLWindow();
	virtual BOOL InitGLContext(HDC hDC);
	virtual void InitGL();
	virtual BOOL SetUpPixelFormat(HDC hDC, DWORD dwFormatFlags);

	virtual BOOL CreateGLBitmap(int nWidth, int nHeight);

protected:
	CWnd*	m_pWndGL;
	HGLRC	m_hGLRC;
	HDC		m_hGLDC;

	BOOL	m_bRenderToWindow;
	HBITMAP m_hMemBmp;
	HBITMAP m_hMemBmpOld;

	double	m_vNormal[3];

#endif 
};


#endif // !defined(AFX_BCGPENGINE3DOPENGL_H__E9670488_59D2_418B_88F1_5E9AE644561F__INCLUDED_)
