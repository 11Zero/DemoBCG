// XTPChartTransformationDeviceCommand.cpp
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
#include <math.h>

#include "GraphicLibrary/GdiPlus/GdiPlus.h"
#include "GraphicLibrary/OpenGL/GL.h"

#include "XTPChartTransformationDeviceCommand.h"
#include "XTPChartDeviceContext.h"
#include "XTPChartOpenGLDeviceContext.h"
#include "../Utils/XTPChartMathUtils.h"


using namespace Gdiplus;
using namespace Gdiplus::DllExports;

//////////////////////////////////////////////////////////////////////////
// CXTPChartRotateDeviceCommand

CXTPChartRotateDeviceCommand::CXTPChartRotateDeviceCommand(float fAngle)
{
	m_fAngle = fAngle;
	m_ptRotateVector = CXTPChartDiagramPoint(0, 0, 1);
}

CXTPChartRotateDeviceCommand::CXTPChartRotateDeviceCommand(float fAngle, const CXTPChartDiagramPoint& rotateVector)
{
	m_fAngle = fAngle;
	m_ptRotateVector = rotateVector;
}

void CXTPChartRotateDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GdipRotateWorldTransform(pDC->GetGraphics(), m_fAngle, MatrixOrderPrepend);
}

void CXTPChartRotateDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	glMatrixMode(GL_MODELVIEW);
	glRotated(m_fAngle, m_ptRotateVector.X, m_ptRotateVector.Y, m_ptRotateVector.Z);
}


CXTPChartElement* CXTPChartRotateDeviceCommand::HitTest(CPoint point, CXTPChartElement* pParent) const
{
	double angle = m_fAngle * CXTPChartMathUtils::PI / 180.0;

	REAL x = (REAL)(point.x * cos(angle) - point.y * sin(angle));
	REAL y =  (REAL)(point.x * sin(angle) + point.y * cos(angle));

	return CXTPChartDeviceCommand::HitTest(CPoint((int)x, (int)y), pParent);
}


void CXTPChartRotateDeviceCommand::TransformMatrix(CXTPChartMatrix* matrix)
{
	if (matrix->GetType() != xtpChartMatrixModelView)
		return;

	CXTPChartDiagramPoint  rotateVector = m_ptRotateVector;
	rotateVector.Normalize();
	double angle = m_fAngle * CXTPChartMathUtils::PI / 180.0;

	CXTPChartMatrix vvt = CXTPChartMatrix::CalculateVMultVt(rotateVector);
	CXTPChartMatrix s(matrix->GetType() );
	int firstElement = 0;
	s[0] = firstElement;
	s[1] = rotateVector.Z;
	s[2] = -rotateVector.Y;
	s[3] = 0.0;
	s[4] = -rotateVector.Z;
	s[5] = 0.0;
	s[6] = rotateVector.X;
	s[7] = 0.0;
	s[8] = rotateVector.Y;
	s[9] = -rotateVector.X;
	s[10] = 0.0;
	s[11] = 0.0;
	s[12] = 0.0;
	s[13] = 0.0;
	s[14] = 0.0;
	s[15] = 0.0;

	CXTPChartMatrix rotateMatrix = vvt + (CXTPChartMatrix(matrix->GetType()) - vvt) * cos(angle) + s * sin(angle);

	rotateMatrix[3] = 0.0;
	rotateMatrix[7] = 0.0;
	rotateMatrix[11] = 0.0;
	rotateMatrix[12] = 0.0;
	rotateMatrix[13] = 0.0;
	rotateMatrix[14] = 0.0;
	rotateMatrix[15] = 1.0;

	*matrix = *matrix * rotateMatrix;
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartSaveStateDeviceCommand

CXTPChartSaveStateDeviceCommand::CXTPChartSaveStateDeviceCommand()
{
	m_nState = 0;
}


void CXTPChartSaveStateDeviceCommand::BeforeExecute(CXTPChartDeviceContext* pDC)
{
	GdipSaveGraphics(pDC->GetGraphics(), &m_nState);
}

void CXTPChartSaveStateDeviceCommand::AfterExecute(CXTPChartDeviceContext* pDC)
{
	GdipRestoreGraphics(pDC->GetGraphics(), m_nState);
}

void CXTPChartSaveStateDeviceCommand::BeforeExecute(CXTPChartOpenGLDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

}

void CXTPChartSaveStateDeviceCommand::AfterExecute(CXTPChartOpenGLDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

}


//////////////////////////////////////////////////////////////////////////
// CXTPChartTranslateDeviceCommand

CXTPChartTranslateDeviceCommand::CXTPChartTranslateDeviceCommand(double dx, double dy, double dz)
{
	m_dx = dx;
	m_dz = dz;
	m_dy = dy;
}

void CXTPChartTranslateDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* pDC)
{
	GdipTranslateWorldTransform(pDC->GetGraphics(), (float)m_dx, (float)m_dy, MatrixOrderPrepend);
}

void CXTPChartTranslateDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	glMatrixMode(GL_MODELVIEW);
	glTranslated(m_dx, m_dy, m_dz);
}

CXTPChartElement* CXTPChartTranslateDeviceCommand::HitTest(CPoint point, CXTPChartElement* pParent) const
{
	return CXTPChartDeviceCommand::HitTest(CPoint(int(point.x - m_dx), int(point.y - m_dy)), pParent);
}

void CXTPChartTranslateDeviceCommand::TransformMatrix(CXTPChartMatrix* matrix)
{
	if (matrix->GetType() != xtpChartMatrixModelView)
		return;

	CXTPChartMatrix translateMatrix;
	translateMatrix[12] = m_dx;
	translateMatrix[13] = m_dy;
	translateMatrix[14] = m_dz;
	*matrix = *matrix * translateMatrix;
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartViewPortDeviceCommand

CXTPChartViewPortDeviceCommand::CXTPChartViewPortDeviceCommand(CRect rcViewPort)
{
	m_rcViewPort = rcViewPort;
}

void CXTPChartViewPortDeviceCommand::BeforeExecute(CXTPChartOpenGLDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	glGetIntegerv(GL_VIEWPORT, m_rcDefaultViewPort);
}

void CXTPChartViewPortDeviceCommand::AfterExecute(CXTPChartOpenGLDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	glViewport(m_rcDefaultViewPort[0], m_rcDefaultViewPort[1], m_rcDefaultViewPort[2], m_rcDefaultViewPort[3]);
}

void CXTPChartViewPortDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC)
{
	glViewport(m_rcViewPort.left, pDC->GetSize().cy - m_rcViewPort.bottom, m_rcViewPort.Width(), m_rcViewPort.Height());
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartDepthTestDeviceCommand

CXTPChartDepthTestDeviceCommand::CXTPChartDepthTestDeviceCommand()
{

}

void CXTPChartDepthTestDeviceCommand::BeforeExecute(CXTPChartOpenGLDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	glEnable(GL_DEPTH_TEST);
}

void CXTPChartDepthTestDeviceCommand::AfterExecute(CXTPChartOpenGLDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	glDisable(GL_DEPTH_TEST);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartIdentityTransformDeviceCommand

CXTPChartIdentityTransformDeviceCommand::CXTPChartIdentityTransformDeviceCommand(XTPChartMatrixType nMatrixType)
{
	m_nMatrixType = nMatrixType;
}

void CXTPChartIdentityTransformDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	glMatrixMode(m_nMatrixType == xtpChartMatrixProjection ? GL_PROJECTION : GL_MODELVIEW);
	glLoadIdentity();
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartFrustumProjectionDeviceCommand

CXTPChartFrustumProjectionDeviceCommand::CXTPChartFrustumProjectionDeviceCommand(CXTPChartDiagramPoint p1, CXTPChartDiagramPoint p2)
{
	m_p1 = p1;
	m_p2 = p2;
}

void CXTPChartFrustumProjectionDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	glMatrixMode(GL_PROJECTION);
	glFrustum(m_p1.X, m_p2.X, m_p1.Y, m_p2.Y, m_p1.Z, m_p2.Z);

}

void CXTPChartFrustumProjectionDeviceCommand::TransformMatrix(CXTPChartMatrix* matrix)
{
	if (matrix->GetType() != xtpChartMatrixProjection)
		return;

	double width = m_p2.X - m_p1.X;
	double height = m_p2.Y - m_p1.Y;
	double depth = m_p2.Z - m_p1.Z;
	double doubleDistance = 2.0 * m_p1.Z;

	CXTPChartMatrix projectionMatrix(matrix->GetType());
	projectionMatrix[0] = doubleDistance / width;
	projectionMatrix[5] = doubleDistance / height;
	projectionMatrix[8] = (m_p1.X + m_p2.X) / width;
	projectionMatrix[9] = (m_p1.Y + m_p2.Y) / height;
	projectionMatrix[10] = -(m_p1.Z + m_p2.Z) / depth;
	projectionMatrix[11] = -1.0;
	projectionMatrix[14] = -2.0 * m_p1.Z * m_p2.Z / depth;
	projectionMatrix[15] = 0.0;

	*matrix = *matrix * projectionMatrix;
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartTransformDeviceCommand

CXTPChartTransformDeviceCommand::CXTPChartTransformDeviceCommand(const CXTPChartMatrix& matrix)
{
	m_matrix = matrix;
}

void CXTPChartTransformDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	glMatrixMode(GL_MODELVIEW);
	glMultMatrixd(m_matrix.GetMatrix());
}

void CXTPChartTransformDeviceCommand::TransformMatrix(CXTPChartMatrix* matrix)
{
	if (matrix->GetType() != xtpChartMatrixModelView)
		return;

	*matrix = *matrix * m_matrix;
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartClipDeviceCommand

CXTPChartClipDeviceCommand::CXTPChartClipDeviceCommand(CRect rcClip)
{
	m_rcClip = rcClip;
}

void CXTPChartClipDeviceCommand::BeforeExecute(CXTPChartDeviceContext* pDC)
{
	GdipGetClipBoundsI(pDC->GetGraphics(), (GpRect*)&m_rcState);

	GdipSetClipRectI(pDC->GetGraphics(), m_rcClip.left, m_rcClip.top, m_rcClip.Width(), m_rcClip.Height(), CombineModeIntersect);
}

void CXTPChartClipDeviceCommand::AfterExecute(CXTPChartDeviceContext* pDC)
{
	GdipSetClipRectI(pDC->GetGraphics(), m_rcState.left, m_rcState.top, m_rcState.Width(), m_rcState.Width(), CombineModeReplace);

}
