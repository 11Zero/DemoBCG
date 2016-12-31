// XTPChartDiagram3D.cpp
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
#include "Common/XTPPropExchange.h"

#include "GraphicLibrary/OpenGL/GLU.h"

#include "XTPChartDiagram3D.h"
#include "Utils/XTPChartMathUtils.h"
#include "Drawing/XTPChartTransformationDeviceCommand.h"


IMPLEMENT_DYNAMIC(CXTPChartDiagram3D, CXTPChartDiagram)

CXTPChartDiagram3D::CXTPChartDiagram3D()
{
	m_nPerspectiveAngle = 20;
	m_bAllowRotate = TRUE;


	CXTPChartRotateDeviceCommand rotate(-40, CXTPChartDiagramPoint(1, 0, 0));
	rotate.TransformMatrix(&m_rotationMatrix);
}


void CXTPChartDiagram3D::DoPropExchange(CXTPPropExchange* pPX)
{
	CXTPChartDiagram::DoPropExchange(pPX);

	PX_Bool(pPX, _T("AllowRotate"), m_bAllowRotate, TRUE);
	PX_Int(pPX, _T("PerspectiveAngle"), m_nPerspectiveAngle, 20);

	double* dValues = m_rotationMatrix.GetMatrix();
	DWORD dwSize = sizeof(dValues[0]) * 16;

	PX_Blob(pPX, _T("RotationMatrix"), (BYTE*&)dValues, dwSize);
}

CXTPChartDeviceCommand* CXTPChartDiagram3D::CreateRotationGraphicsCommand()
{
	return new CXTPChartTransformDeviceCommand(m_rotationMatrix);
}


CXTPChartDeviceCommand* CXTPChartDiagram3D::CreateModelViewGraphicsCommand(CXTPChartDiagram3DDomain* pDomain)
{
	CXTPChartDeviceCommand* command = new CXTPChartDeviceCommand();
	command->AddChildCommand(CreateRotationGraphicsCommand());
	command->AddChildCommand(new CXTPChartTranslateDeviceCommand(-pDomain->m_dWidth * 0.5,
		-pDomain->m_dHeight * 0.5, -pDomain->m_dDepth * 0.5));
	command->AddChildCommand(pDomain->CreateAdditionalModelViewCommand());
	return command;
}

CXTPChartDeviceCommand* CXTPChartDiagram3D::CreatePerspectiveGraphicsCommand(CXTPChartDiagram3DDomain* pDomain)
{
	double dmin = min(pDomain->GetBounds().Width(), pDomain->GetBounds().Height());

	CXTPChartDeviceCommand* command = new CXTPChartDeviceCommand();
	double distance = 0.5 * dmin * pDomain->m_dZoomFactor / tan(GetPerspectiveAngle() / 360.0 * CXTPChartMathUtils::PI);
	command->AddChildCommand(new CXTPChartTranslateDeviceCommand(0.0, 0.0, -distance - pDomain->m_dViewRadius));
	CXTPChartDiagramPoint p1, p2;
	pDomain->CalcBoundsAccordingZoomingAndScrolling(distance, p1, p2);
	command->AddChildCommand(new CXTPChartFrustumProjectionDeviceCommand(p1, p2));
	return command;
}

CXTPChartDeviceCommand* CXTPChartDiagram3D::CreateProjectionGraphicsCommand(CXTPChartDiagram3DDomain* pDomain)
{
	return CreatePerspectiveGraphicsCommand(pDomain);
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartDiagram3DDomain


CXTPChartDiagram3DDomain::CXTPChartDiagram3DDomain(CXTPChartDiagram3D* pDiagram, const CRect& rcBounds, const CRect& rcLabelBounds, const CXTPChartRectF& rcInnerBounds, int nDepthPercent)
{
	m_pDiagram = pDiagram;
	m_rcBounds = rcBounds;
	m_rcLabelBounds = rcLabelBounds;
	m_rcInnerBounds = rcInnerBounds;
	m_dDepthFactor = nDepthPercent / 100.0;

	m_dWidth =  rcBounds.Width();
	m_dHeight = rcBounds.Height();
	m_dDepth = m_rcInnerBounds.Width * m_dDepthFactor;

	m_dViewRadius = sqrt(m_dWidth * m_dWidth + m_dHeight * m_dHeight + m_dDepth * m_dDepth) * 0.5;

	double maxDimension = max(m_dWidth, m_dHeight), minDimension = min(m_dWidth, m_dHeight);

	double dimensionFactor = sqrt(minDimension / maxDimension);
	double multipleFactor = sqrt((1.0 + m_dDepthFactor * m_dDepthFactor) / (2.0 + m_dDepthFactor * m_dDepthFactor));
	m_dViewRadius = m_dViewRadius * dimensionFactor * multipleFactor * 1.001;


	if (FALSE) // TODO Perspective
	{

	}
	else
	{
		double radianAngle = CXTPChartMathUtils::Degree2Radian(m_pDiagram->GetPerspectiveAngle()) / 2.0;
		m_dZoomFactor = 2.0 * m_dViewRadius * (1.0 - sin(radianAngle)) / cos(radianAngle);
		m_dViewOffsetX = -m_dZoomFactor / 2.0;
		m_dViewOffsetY = -m_dZoomFactor / 2.0;
	}

	if (m_rcBounds.Width() > m_rcBounds.Height())
	{
		m_dZoomFactor /= m_rcBounds.Height();
		m_dViewOffsetX -= (m_rcBounds.Width() - m_rcBounds.Height()) * m_dZoomFactor * 0.5;
	}
	else
	{
		m_dZoomFactor /= m_rcBounds.Width();
		m_dViewOffsetY -= (m_rcBounds.Height() - m_rcBounds.Width()) * m_dZoomFactor * 0.5;
	}
}


CXTPChartDeviceCommand* CXTPChartDiagram3DDomain::CreateViewPortGraphicsCommand()
{
	return new CXTPChartViewPortDeviceCommand(m_rcBounds);
}


void CXTPChartDiagram3DDomain::CalcBoundsAccordingZoomingAndScrolling(double distance, CXTPChartDiagramPoint& p1, CXTPChartDiagramPoint& p2)
{
	p1 = CXTPChartDiagramPoint(m_dViewOffsetX, m_dViewOffsetY, distance);

	p2 = p1;
	p2.Offset(m_dZoomFactor * m_rcBounds.Width(), m_dZoomFactor * m_rcBounds.Height(), m_dViewRadius * 2.0);
}

CXTPChartDiagramPoint CXTPChartDiagram3DDomain::Project(const CXTPChartDiagramPoint& point)
{
	CXTPChartDeviceCommand* command = new CXTPChartDeviceCommand();
	command->AddChildCommand(m_pDiagram->CreateProjectionGraphicsCommand(this));
	command->AddChildCommand(m_pDiagram->CreateModelViewGraphicsCommand(this));

	CXTPChartMatrix modelViewMatrix(xtpChartMatrixModelView);
	CXTPChartMatrix projectionMatrix(xtpChartMatrixProjection);

	command->TransformMatrix(&modelViewMatrix);
	command->TransformMatrix(&projectionMatrix);

	delete command;

	int viewport[4] = {m_rcBounds.left, m_rcBounds.top, m_rcBounds.Width(), m_rcBounds.Height()};

	double x, y, z;
	gluProject(point.X, point.Y, point.Z, modelViewMatrix.GetMatrix(), projectionMatrix.GetMatrix(), viewport, &x, &y, &z);

	return CXTPChartDiagramPoint(x, m_rcBounds.Height() - y + m_rcBounds.top + m_rcBounds.top, z);
}
