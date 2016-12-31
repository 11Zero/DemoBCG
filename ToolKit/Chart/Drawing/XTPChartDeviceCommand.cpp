// XTPChartDeviceCommand.cpp
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
#include "GraphicLibrary/OpenGL/GL.h"
#include "GraphicLibrary/OpenGL/GLU.h"


#include "XTPChartDeviceCommand.h"
#include "XTPChartDeviceContext.h"
#include "XTPChartOpenGLDeviceContext.h"
#include "XTPChartOpenGLDeviceContext.h"
#include "XTPChartOpenGLHelpers.h"

using namespace Gdiplus;
using namespace Gdiplus::DllExports;

//////////////////////////////////////////////////////////////////////////
// CXTPChartDeviceCommand

CXTPChartDeviceCommand::CXTPChartDeviceCommand()
{

}

CXTPChartDeviceCommand::~CXTPChartDeviceCommand()
{
	for (int i = 0; i < m_arrChildren.GetSize(); i++)
	{
		delete m_arrChildren[i];
	}

	m_arrChildren.RemoveAll();
}

CXTPChartDeviceCommand* CXTPChartDeviceCommand::AddChildCommand(CXTPChartDeviceCommand* pCommand)
{
	if (pCommand)
	{
		m_arrChildren.Add(pCommand);
	}
	return pCommand;
}

void CXTPChartDeviceCommand::Execute(CXTPChartDeviceContext* pDC)
{
	if (!pDC->GetNativeDrawing())
	{
		Execute((CXTPChartOpenGLDeviceContext*)pDC);
		return;
	}

	BeforeExecute(pDC);
	ExecuteOverride(pDC);

	for (int i = 0; i < m_arrChildren.GetSize(); i++)
	{
		m_arrChildren[i]->Execute(pDC);
	}

	AfterExecute(pDC);
}

void CXTPChartDeviceCommand::Execute(CXTPChartOpenGLDeviceContext* pDC)
{
	if (pDC->GetNativeDrawing())
	{
		Execute((CXTPChartDeviceContext*)pDC);
		return;
	}
	BeforeExecute(pDC);
	ExecuteOverride(pDC);

	for (int i = 0; i < m_arrChildren.GetSize(); i++)
	{
		m_arrChildren[i]->Execute(pDC);
	}

	AfterExecute(pDC);
}

void CXTPChartDeviceCommand::BeforeExecute(CXTPChartDeviceContext* /*pDC*/)
{

}

void CXTPChartDeviceCommand::AfterExecute(CXTPChartDeviceContext* /*pDC*/)
{

}

void CXTPChartDeviceCommand::ExecuteOverride(CXTPChartDeviceContext* /*pDC*/)
{

}


void CXTPChartDeviceCommand::BeforeExecute(CXTPChartOpenGLDeviceContext* /*pDC*/)
{

}

void CXTPChartDeviceCommand::AfterExecute(CXTPChartOpenGLDeviceContext* /*pDC*/)
{

}

void CXTPChartDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* /*pDC*/)
{

}

void CXTPChartDeviceCommand::TransformMatrix(CXTPChartMatrix* matrix)
{
	for (int i = 0; i < m_arrChildren.GetSize(); i++)
	{
		m_arrChildren[i]->TransformMatrix(matrix);
	}
}

CXTPChartElement* CXTPChartDeviceCommand::HitTest(CPoint point, CXTPChartElement* pParent) const
{
	for (int i = (int)m_arrChildren.GetSize() - 1; i >= 0; i--)
	{
		CXTPChartElement* pElement = m_arrChildren[i]->HitTest(point, pParent);
		if (pElement)
			return pElement;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartLightingDeviceCommand

CXTPChartLightingDeviceCommand::CXTPChartLightingDeviceCommand(CXTPChartColor ambientColor, CXTPChartColor materialSpecularColor, CXTPChartColor materialEmissionColor, float materialShininess)
{
	m_ambientColor = ambientColor;
	m_materialSpecularColor = materialSpecularColor;
	m_materialEmissionColor = materialEmissionColor;
	m_materialShininess = materialShininess;
}

void CXTPChartLightingDeviceCommand::AfterExecute(CXTPChartOpenGLDeviceContext* /*pDC*/)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_NORMALIZE);
}
void CXTPChartLightingDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC)
{
	float color[4];
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	pDC->CalculateColorComponents(m_ambientColor, color);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	pDC->CalculateColorComponents(m_materialSpecularColor, color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
	pDC->CalculateColorComponents(m_materialEmissionColor, color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_materialShininess);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
}



//////////////////////////////////////////////////////////////////////////
// CXTPChartLightDeviceCommand

CXTPChartLightDeviceCommand::CXTPChartLightDeviceCommand(int index, CXTPChartColor ambientColor, CXTPChartColor diffuseColor, CXTPChartColor specularColor, CXTPChartDiagramPoint position, CXTPChartDiagramPoint spotDirection, float spotExponent, float spotCutoff, float constantAttenuation, float linearAttenuation, float quadraticAttenuation)
{
	m_index = index;
	m_ambientColor = ambientColor;
	m_diffuseColor = diffuseColor;
	m_specularColor = specularColor;
	m_position = position;
	m_spotDirection = spotDirection;
	m_spotExponent = spotExponent;
	m_spotCutoff = spotCutoff;
	m_constantAttenuation = constantAttenuation;
	m_linearAttenuation = linearAttenuation;
	m_quadraticAttenuation = quadraticAttenuation;
	m_directional = false;
}

void CXTPChartLightDeviceCommand::AfterExecute(CXTPChartOpenGLDeviceContext* /*pDC*/)
{
	glDisable(GL_LIGHT0 + m_index);
}

void CXTPChartLightDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC)
{
	int actualIndex = GL_LIGHT0 + m_index;

	float color[4];
	pDC->CalculateColorComponents(m_ambientColor, color);
	glLightfv(actualIndex, GL_AMBIENT, color);
	pDC->CalculateColorComponents(m_diffuseColor, color);
	glLightfv(actualIndex, GL_DIFFUSE, color);
	pDC->CalculateColorComponents(m_specularColor, color);
	glLightfv(actualIndex, GL_SPECULAR, color);

	float fPosition[4] = {(float)m_position.X, (float)m_position.Y, (float)m_position.Z, m_directional ? (float)0 : (float)1};

	glLightfv(actualIndex, GL_POSITION, fPosition);

	float fSpot[4] = {(float)m_spotDirection.X, (float)m_spotDirection.Y, (float)m_spotDirection.Z, (float)1};

	glLightfv(actualIndex, GL_SPOT_DIRECTION, fSpot);

	glLightf(actualIndex, GL_SPOT_EXPONENT, m_spotExponent);
	glLightf(actualIndex, GL_SPOT_CUTOFF, m_spotCutoff);
	glLightf(actualIndex, GL_CONSTANT_ATTENUATION, m_constantAttenuation);
	glLightf(actualIndex, GL_LINEAR_ATTENUATION, m_linearAttenuation);
	glLightf(actualIndex, GL_QUADRATIC_ATTENUATION, m_quadraticAttenuation);

	glEnable(actualIndex);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartPolygonAntialiasingDeviceCommand

CXTPChartPolygonAntialiasingDeviceCommand::CXTPChartPolygonAntialiasingDeviceCommand(BOOL bAntiAlias)
{
	shiftCount = 4;
	memset(shifts, 0, sizeof(shifts));
	pixelData = 0;

	m_bAntiAlias = bAntiAlias;
}

CXTPChartPolygonAntialiasingDeviceCommand::~CXTPChartPolygonAntialiasingDeviceCommand()
{
}

void CXTPChartPolygonAntialiasingDeviceCommand::BeforeExecute(CXTPChartOpenGLDeviceContext* pDC)
{
	if (!m_bAntiAlias || pDC->m_bNativeDrawing || pDC->m_bWindowDC)
		return;

	glClearAccum(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_ACCUM_BUFFER_BIT);
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
	for (int i = 0; i < shiftCount; i++)
		shifts[i] = ((double)i) / shiftCount;

	CSize sz = pDC->GetSize();

	pixelData = new BYTE[sz.cx * sz.cy * 4];
	glFinish();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glViewport(0, 0, sz.cx, sz.cy);
	glReadPixels(0, 0, sz.cx, sz.cy, GL_RGBA, GL_BYTE, pixelData);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);


}

void CXTPChartPolygonAntialiasingDeviceCommand::Execute(CXTPChartOpenGLDeviceContext* pDC)
{
	if (!m_bAntiAlias || pDC->m_bNativeDrawing || pDC->m_bWindowDC)
	{
		CXTPChartDeviceCommand::Execute(pDC);
		return;
	}
	BeforeExecute(pDC);
	ExecuteOverride(pDC);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	float val = 0.5f / shiftCount;

	int i;
	int k;

	for (i = 0; i < shiftCount; i++)
	{
		RestoreImage(pDC->GetSize());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glTranslated(2.0 / (viewport[2]) * shifts[i], 2.0 / (viewport[3]) * shifts[i], 0);
		glMultMatrixd(projectionMatrix);

		for (k = 0; k < m_arrChildren.GetSize(); k++)
		{
			m_arrChildren[k]->Execute(pDC);
		}

		glAccum(GL_ACCUM, val);
	}

	for (i = 0; i < shiftCount; i++)
	{
		RestoreImage(pDC->GetSize());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glTranslated(2.0 / (viewport[2]) * shifts[i], 2.0 / (viewport[3]) * (1 - shifts[i]), 0);
		glMultMatrixd(projectionMatrix);

		for (k = 0; k < m_arrChildren.GetSize(); k++)
		{
			m_arrChildren[k]->Execute(pDC);
		}

		glAccum(GL_ACCUM, val);
	}

	AfterExecute(pDC);
}

void CXTPChartPolygonAntialiasingDeviceCommand::RestoreImage(CSize size)
{
	glViewport(0, 0, size.cx, size.cy);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, viewport[2], 0, viewport[3], -1, 1);
	glRasterPos2i(0, 0);
	glDrawPixels(size.cx, size.cy, GL_RGBA, GL_BYTE, pixelData);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT |GL_COLOR_BUFFER_BIT );
}


void CXTPChartPolygonAntialiasingDeviceCommand::AfterExecute(CXTPChartOpenGLDeviceContext* pDC)
{
	if (!m_bAntiAlias || pDC->m_bNativeDrawing || pDC->m_bWindowDC)
		return;

	glAccum(GL_RETURN, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(projectionMatrix);

	delete[] pixelData;
	pixelData = NULL;
}


void CXTPChartPolygonAntialiasingDeviceCommand::BeforeExecute(CXTPChartDeviceContext* pDC)
{
	GdipGetSmoothingMode(pDC->GetGraphics(), (SmoothingMode*)&m_bOldAntiAlias);
	GdipSetSmoothingMode(pDC->GetGraphics(), m_bAntiAlias ? SmoothingModeHighQuality : SmoothingModeDefault);
}

void CXTPChartPolygonAntialiasingDeviceCommand::AfterExecute(CXTPChartDeviceContext* pDC)
{
	GdipSetSmoothingMode(pDC->GetGraphics(), (SmoothingMode)m_bOldAntiAlias);

}


//////////////////////////////////////////////////////////////////////////
// CXTPChartDrawingTypeDeviceCommand

CXTPChartDrawingTypeDeviceCommand::CXTPChartDrawingTypeDeviceCommand(BOOL bNativeDrawing)
{
	m_bDrawingType = bNativeDrawing;
	m_bOldDrawingType = FALSE;
}

void CXTPChartDrawingTypeDeviceCommand::BeforeExecute(CXTPChartOpenGLDeviceContext* pDC)
{
	m_bOldDrawingType = pDC->GetNativeDrawing();

	pDC->SetNativeDrawing(m_bDrawingType);
}

void CXTPChartDrawingTypeDeviceCommand::AfterExecute(CXTPChartOpenGLDeviceContext* pDC)
{
	pDC->SetNativeDrawing(m_bOldDrawingType);
}

void CXTPChartDrawingTypeDeviceCommand::BeforeExecute(CXTPChartDeviceContext* pDC)
{
	m_bOldDrawingType = pDC->GetNativeDrawing();

	pDC->SetNativeDrawing(m_bDrawingType);
}

void CXTPChartDrawingTypeDeviceCommand::AfterExecute(CXTPChartDeviceContext* pDC)
{
	pDC->SetNativeDrawing(m_bOldDrawingType);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartHitTestElementCommand

CXTPChartHitTestElementCommand::CXTPChartHitTestElementCommand(CXTPChartElement* pElement)
{
	m_pElement = pElement;
	m_rcBounds.SetRectEmpty();
}

CXTPChartHitTestElementCommand::CXTPChartHitTestElementCommand(CXTPChartElement* pElement, const CRect& rcBounds)
{
	m_pElement = pElement;
	m_rcBounds = rcBounds;
}

CXTPChartHitTestElementCommand::CXTPChartHitTestElementCommand(CXTPChartElement* pElement, const CXTPChartRectF& rcBounds)
{
	m_pElement = pElement;

	m_rcBounds.left = (int)rcBounds.GetLeft();
	m_rcBounds.right = (int)rcBounds.GetRight();
	m_rcBounds.top = (int)rcBounds.GetTop();
	m_rcBounds.bottom = (int)rcBounds.GetBottom();
}

CXTPChartElement* CXTPChartHitTestElementCommand::HitTest(CPoint point, CXTPChartElement* /*pParent*/) const
{
	if (!m_rcBounds.IsRectEmpty())
	{
		if (!m_rcBounds.PtInRect(point))
			return NULL;
	}

	for (int i = (int)m_arrChildren.GetSize() - 1; i >= 0; i--)
	{
		CXTPChartElement* pElement = m_arrChildren[i]->HitTest(point, m_pElement);
		if (pElement)
			return pElement;
	}
	return NULL;
}
