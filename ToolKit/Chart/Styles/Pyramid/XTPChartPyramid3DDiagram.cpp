// XTPChartPyramid3DDiagram.cpp
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

#include "../../Appearance/XTPChartAppearance.h"
#include "../../XTPChartSeriesLabel.h"
#include "../../Drawing/XTPChartDeviceCommand.h"
#include "../../Drawing/XTPChartTransformationDeviceCommand.h"
#include "../../Utils/XTPChartMathUtils.h"
#include "../../XTPChartContent.h"

#include "XTPChartPyramid3DDiagram.h"
#include "XTPChartPyramid3DSeriesStyle.h"
#include "XTPChartPyramidSeriesLabel.h"

#include "GraphicLibrary/OpenGL/GL.h"

//////////////////////////////////////////////////////////////////////////
// CXTPChartPyramid3DDiagram

IMPLEMENT_SERIAL(CXTPChartPyramid3DDiagram, CXTPChartDiagram3D, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPChartPyramid3DDiagram::CXTPChartPyramid3DDiagram()
{
	m_rotationMatrix = CXTPChartMatrix();

	CXTPChartRotateDeviceCommand rotate(100, CXTPChartDiagramPoint(1, 0, 0));
	//
	rotate.TransformMatrix(&m_rotationMatrix);

	CXTPChartRotateDeviceCommand rotate2(60, CXTPChartDiagramPoint(0, 0, 1));
	//
	rotate2.TransformMatrix(&m_rotationMatrix);
}


CXTPChartDiagramView* CXTPChartPyramid3DDiagram::CreateView(CXTPChartDeviceContext* pDC, CXTPChartElementView* pParent)
{
	UNREFERENCED_PARAMETER(pDC);
	return new CXTPChartPyramid3DDiagramView(this, pParent);
}



void CXTPChartPyramid3DDiagram::CalculateSeriesLayout(CXTPChartDeviceContext* pDC, CXTPChartDiagramView* pView)
{
	if (pView->GetCount() == 0)
		return;

	for (int i = 0; i < pView->GetSeriesView()->GetCount(); i++)
	{
		CXTPChartPyramid3DSeriesView* pSeriesView = (CXTPChartPyramid3DSeriesView*)pView->GetSeriesView()->GetAt(i);

		pSeriesView->CalculatePointLayout(pDC, pView->GetBounds());
		pSeriesView->CalculateLabelLayout(pDC);
	}
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartPyramid3DDiagramView

CXTPChartPyramid3DDiagramView::CXTPChartPyramid3DDiagramView(CXTPChartDiagram* pDiagram, CXTPChartElementView* pParent)
	: CXTPChartPyramidDiagramView(pDiagram, pParent)
{
	m_ptOldPosition = CPoint(0, 0);
}

void CXTPChartPyramid3DDiagramView::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	m_pContainer->SetCapture(this);
	m_ptOldPosition = point;
}

void CXTPChartPyramid3DDiagramView::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	int dx = point.x - m_ptOldPosition.x;
	int dy = point.y - m_ptOldPosition.y;
	m_ptOldPosition = point;

	//dy = 0;

	PerformRotation(dx, dy);
}

BOOL CXTPChartPyramid3DDiagramView::PerformRotation(int dx, int dy)
{
	CXTPChartDiagram3D* pDiagram = (CXTPChartDiagram3D*)GetDiagram();
	if (!pDiagram->IsAllowRotate())
		return FALSE;

	CXTPChartMatrix matrix;

	CXTPChartRotateDeviceCommand rotate1((float)dx, CXTPChartDiagramPoint(0, 1, 0));
	rotate1.TransformMatrix(&matrix);

	CXTPChartRotateDeviceCommand rotate2((float)dy, CXTPChartDiagramPoint(1, 0, 0));
	rotate2.TransformMatrix(&matrix);

	pDiagram->SetRotationMatrix(matrix * pDiagram->GetRotationMatrix());

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartPyramid3DDiagramSeriesView

CXTPChartPyramid3DSeriesView::CXTPChartPyramid3DSeriesView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView)
	: CXTPChartPyramidSeriesView(pSeries, pDiagramView)
{
	m_pDomain = NULL;

}

CXTPChartPyramid3DSeriesView::~CXTPChartPyramid3DSeriesView()
{
	SAFE_DELETE(m_pDomain);

}
void CXTPChartPyramid3DSeriesView::CalculatePointLayout(CXTPChartDeviceContext* pDC, CRect rcBounds)
{
	SAFE_DELETE(m_pDomain);

	m_pDomain = CreateDiagramDomain(pDC, rcBounds);

	CXTPChartPyramidSeriesView::CalculatePointLayout(pDC, rcBounds);
}

CXTPChartRectF CXTPChartPyramid3DSeriesView::GetInnerBounds() const
{
	return m_pDomain->GetInnerBounds();
}


CXTPChartDiagramDomain* CXTPChartPyramid3DSeriesView::CreateDiagramDomain(CXTPChartDeviceContext* /*pDC*/, CRect rcBounds)
{
	CXTPChartDiagram3D* pDiagram = (CXTPChartDiagram3D*)GetDiagram();

	rcBounds.DeflateRect(5, 5, 5, 5);

	CRect rcInnerBounds = rcBounds;

	int nWidth = min(rcInnerBounds.Width(), rcInnerBounds.Height());

	nWidth = int(nWidth / sqrt(2.0));

	if (nWidth < 0)
	{
		nWidth = 1;
		rcInnerBounds = CRect(rcBounds.CenterPoint(), CSize(1, 1));
	}

	CXTPChartRectF rcInnerBoundsF((float)(rcInnerBounds.left + rcInnerBounds.right - nWidth / 2),
		(float)(rcInnerBounds.top + rcInnerBounds.bottom - nWidth / 2), (float)nWidth, (float)nWidth);


	return new CXTPChartPyramid3DDiagramDomain(pDiagram, rcBounds, rcBounds, rcInnerBoundsF, 10);

}

void CXTPChartPyramid3DSeriesView::CalculateLabelLayout(CXTPChartDeviceContext* pDC)
{
	for (int i = 0; i < m_pLabelsView->GetCount(); i++)
	{
		CXTPChartSeriesLabelView* pLabelView = (CXTPChartSeriesLabelView*)m_pLabelsView->GetAt(i);

		pLabelView->CalculateLayout(pDC);
	}
}

CXTPChartSeriesPointView* CXTPChartPyramid3DSeriesView::CreateSeriesPointView(CXTPChartDeviceContext* pDC, CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView)
{
	UNREFERENCED_PARAMETER(pDC);
	return new CXTPChartPyramid3DSeriesPointView(pPoint, pParentView);
}

CXTPChartDeviceCommand* CXTPChartPyramid3DSeriesView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{
	CXTPChartDeviceCommand* pDrawingType = new CXTPChartDrawingTypeDeviceCommand(FALSE);

	CXTPChartPyramid3DDiagramDomain* pDomain = (CXTPChartPyramid3DDiagramDomain*)m_pDomain;

	CXTPChartDeviceCommand* pDeviceCommand = pDomain->CreateViewPortGraphicsCommand();

	pDrawingType->AddChildCommand(pDeviceCommand);

	CXTPChartDeviceCommand* pInnerCommand;
	pDeviceCommand->AddChildCommand(pDomain->CreateGraphicsCommand(pInnerCommand));
	pInnerCommand->AddChildCommand(m_pPointsView->CreateDeviceCommand(pDC));

	return pDrawingType;
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartPyramid3DDiagramSeriesPointView

CXTPChartPyramid3DSeriesPointView::CXTPChartPyramid3DSeriesPointView(CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView)
	: CXTPChartPyramidSeriesPointView(pPoint, pParentView)
{
}

CXTPChartPyramid3DSeriesPointView::~CXTPChartPyramid3DSeriesPointView()
{
}


class CXTPChartGradientPyramid3DDeviceCommand : public CXTPChartDeviceCommand
{
public:
	CXTPChartGradientPyramid3DDeviceCommand(CXTPChartRectF rcInnerBounds, double dFrom, double dTo,
		CXTPChartColor color1, CXTPChartColor color2, int nGap)
	{
		m_rcInnerBounds = rcInnerBounds;
		m_dFrom = dFrom;
		m_dTo = dTo;

		m_color = color1;
		m_color2 = color2;

		m_nGap = nGap;
	}
	void PerformPyrammidDrawing(CXTPChartOpenGLDeviceContext* pDC);
	void ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC);

	CXTPChartRectF m_rcInnerBounds;
	double m_dFrom;
	double m_dTo;
	CXTPChartColor m_color;
	CXTPChartColor m_color2;
	int m_nGap;

	void SetColor(double x, double y, double z);
	void glColorVertex(double x, double y, double z);
};


void CXTPChartGradientPyramid3DDeviceCommand::SetColor(double x, double y, double z)
{
	UNREFERENCED_PARAMETER(y);
	UNREFERENCED_PARAMETER(z);

	float sz = m_rcInnerBounds.Width;

	float coord;

	CXTPChartRectF relativeBounds(-sz / 2, -sz / 2, sz, sz);

	coord = (float)(-(relativeBounds.X - x) / relativeBounds.Width);

	if (coord > 1) coord = 1; else if (coord < 0) coord = 0;
	coord /= 255.0f;

	glColor4f((m_color.GetR() * coord + m_color2.GetR() * (1.0f/255.0f - coord)),
		(m_color.GetG() * coord + m_color2.GetG() * (1.0f/255.0f - coord)),
		(m_color.GetB() * coord + m_color2.GetB() * (1.0f/255.0f - coord)),
		(m_color.GetA() * coord + m_color2.GetA() * (1.0f/255.0f - coord)));

}


void CXTPChartGradientPyramid3DDeviceCommand::glColorVertex(double x, double y, double z)
{
	SetColor(x, y, z);
	glVertex3d(x, y, z);
}

void CXTPChartGradientPyramid3DDeviceCommand::PerformPyrammidDrawing(CXTPChartOpenGLDeviceContext* /*pDC*/)
{

	glBegin(GL_QUADS);

	double dFrom = m_rcInnerBounds.Width * m_dFrom;
	double dTo = m_rcInnerBounds.Width * m_dTo;

	dTo -= m_nGap;
	if (dTo - dFrom < 1)
		dTo = dFrom + 1;

	float a = float(-sqrt(5.0) / 5);

	glNormal3f(0, 0, -1);

	glColorVertex(dTo / 2, dTo / 2, dTo);
	glColorVertex(dTo / 2, -dTo / 2, dTo);
	glColorVertex(-dTo / 2, -dTo / 2, dTo);
	glColorVertex(-dTo / 2, dTo / 2, dTo);

	glNormal3f(2 * a, 0, a);

	glColorVertex(dFrom / 2, dFrom / 2, dFrom);
	glColorVertex(dFrom / 2, -dFrom / 2, dFrom);
	glColorVertex(dTo / 2, -dTo / 2, dTo);
	glColorVertex(dTo / 2, dTo / 2, dTo);

	glNormal3f(0, -2 * a, a);

	glColorVertex(dFrom / 2, -dFrom / 2, dFrom);
	glColorVertex(-dFrom / 2, -dFrom / 2, dFrom);
	glColorVertex(-dTo / 2, -dTo / 2, dTo);
	glColorVertex(dTo / 2, -dTo / 2, dTo);


	glNormal3f(-2 * a, 0, a);

	glColorVertex(-dFrom / 2, -dFrom / 2, dFrom);
	glColorVertex(-dFrom / 2, dFrom / 2, dFrom);
	glColorVertex(-dTo / 2, dTo / 2, dTo);
	glColorVertex(-dTo / 2, -dTo / 2, dTo);

	glNormal3f(0, 2 * a, a);

	glColorVertex(-dFrom / 2, dFrom / 2, dFrom);
	glColorVertex(dFrom / 2, dFrom / 2, dFrom);
	glColorVertex(dTo / 2, dTo / 2, dTo);
	glColorVertex(-dTo / 2, dTo / 2, dTo);

	glNormal3f(0, 0, 1);

	glColorVertex(dFrom / 2, dFrom / 2, dFrom);
	glColorVertex(dFrom / 2, -dFrom / 2, dFrom);
	glColorVertex(-dFrom / 2, -dFrom / 2, dFrom);
	glColorVertex(-dFrom / 2, dFrom / 2, dFrom);

	glEnd();
}

void CXTPChartGradientPyramid3DDeviceCommand::ExecuteOverride(CXTPChartOpenGLDeviceContext* pDC)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	double dSize = m_rcInnerBounds.Width * 0.5;
	glTranslated(0, 0, -dSize);

	PerformPyrammidDrawing(pDC);

	glPopMatrix();
}


CXTPChartDeviceCommand* CXTPChartPyramid3DSeriesPointView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{
	CXTPChartPyramid3DSeriesStyle* pStyle = STATIC_DOWNCAST(CXTPChartPyramid3DSeriesStyle, GetSeriesView()->GetStyle());
	UNREFERENCED_PARAMETER(pDC);

	CXTPChartColor color1 = GetColor();
	CXTPChartColor color2 = GetColor2();

	CXTPChartRectF rcInnerBounds = ((CXTPChartPyramid3DSeriesView*)GetSeriesView())->GetInnerBounds();

	int nTransparency = pStyle->GetTransparency();

	CXTPChartColor color11((BYTE)nTransparency, color1.GetRed(), color1.GetGreen(), color1.GetBlue());
	CXTPChartColor color22((BYTE)nTransparency, color2.GetRed(), color2.GetGreen(), color2.GetBlue());


	int nGap = 0;
	if (m_dTo != 1)
		nGap = pStyle->GetPointDistance();

	return new CXTPChartGradientPyramid3DDeviceCommand(rcInnerBounds, m_dFrom, m_dTo, color22, color11, nGap);
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartPyramid3DDiagramDomain

CXTPChartPyramid3DDiagramDomain::CXTPChartPyramid3DDiagramDomain(CXTPChartDiagram3D* pDiagram, CRect& rcBounds, CRect& rcLabelBounds, CXTPChartRectF& rcInnerBounds, int nDepthPercent)
	:  CXTPChartDiagram3DDomain(pDiagram, rcBounds, rcLabelBounds, rcInnerBounds, nDepthPercent)
{
}


CXTPChartDeviceCommand* CXTPChartPyramid3DDiagramDomain::CreateGraphicsCommand(CXTPChartDeviceCommand*& pInnerCommand)
{
	CXTPChartDeviceCommand* depthCommand = new CXTPChartDepthTestDeviceCommand();
	CXTPChartDeviceCommand* saveStateCommand = new CXTPChartSaveStateDeviceCommand();
	depthCommand->AddChildCommand(saveStateCommand);
	saveStateCommand->AddChildCommand(new CXTPChartIdentityTransformDeviceCommand(xtpChartMatrixModelView));
	saveStateCommand->AddChildCommand(new CXTPChartIdentityTransformDeviceCommand(xtpChartMatrixProjection));
	CXTPChartDeviceCommand* innerLightingCommand;
	CXTPChartDeviceCommand* projectionCommand = ((CXTPChartPyramid3DDiagram*)m_pDiagram)->CreateProjectionGraphicsCommand(this);
	saveStateCommand->AddChildCommand(projectionCommand);
	projectionCommand->AddChildCommand(CreateLightingCommand(innerLightingCommand));
	innerLightingCommand->AddChildCommand(((CXTPChartPyramid3DDiagram*)m_pDiagram)->CreateModelViewGraphicsCommand(this));
	pInnerCommand = new CXTPChartPolygonAntialiasingDeviceCommand();
	innerLightingCommand->AddChildCommand(pInnerCommand);

	return depthCommand;
}


CXTPChartDeviceCommand* CXTPChartPyramid3DDiagramDomain::CreateAdditionalModelViewCommand()
{
	return new CXTPChartTranslateDeviceCommand(m_dWidth * 0.5, m_dHeight * 0.5, 0.0);
}

CXTPChartDeviceCommand* CXTPChartPyramid3DDiagramDomain::CreateLightingCommand(CXTPChartDeviceCommand*& innerLightingCommand)
{
#if 0
	CXTPChartDeviceCommand* command = new CXTPChartLightingDeviceCommand(CXTPChartColor(255, 130, 130, 130),
		CXTPChartColor::White, CXTPChartColor::Black, 10.0f);

	double scale = m_dViewRadius * 0.7;
	CXTPChartDiagramPoint location(-scale, 1.5 * scale, scale * 2);
	CXTPChartDiagramPoint direction = location;
	direction.Revert();
	direction.Normalize();
	CXTPChartDeviceCommand* light0Command = new CXTPChartLightDeviceCommand(0, CXTPChartColor(255, 0, 0, 0),
		CXTPChartColor(255, 130, 130, 130), CXTPChartColor(255, 50, 50, 50),
		location, direction, 0.0f, 180.0f, 1.0f, 0.0f, 0.0f);
	command->AddChildCommand(light0Command);

	location = CXTPChartDiagramPoint(-scale * 2, -scale * 2, scale * 4);
	direction = location;
	direction.Revert();
	direction.Normalize();
	CXTPChartDeviceCommand* light1Command = new CXTPChartLightDeviceCommand(1, CXTPChartColor(255, 0, 0, 0),
		CXTPChartColor(255, 80, 80, 80), CXTPChartColor(255, 100, 100, 100),
		location, direction, 0.0f, 180.0f, 1.0f, 0.0f, 0.0f);
	light0Command->AddChildCommand(light1Command);
	innerLightingCommand = light1Command;
	return command;

#else


	//CXTPChartDeviceCommand* command = new CXTPChartLightingDeviceCommand(CXTPChartColor(255, 130, 130, 130),
	CXTPChartDeviceCommand* command = new CXTPChartLightingDeviceCommand(CXTPChartColor::Gray,
		//CXTPChartColor::White, CXTPChartColor::Black, 10.0f);
		CXTPChartColor::White, CXTPChartColor::Black, 120.0f);

	double scale = m_dViewRadius * 0.7;
	CXTPChartDiagramPoint location(-scale, -1.5 * scale, scale * 2);
	CXTPChartDiagramPoint direction = location;
	direction.Revert();
	direction.Normalize();
	CXTPChartDeviceCommand* light0Command = new CXTPChartLightDeviceCommand(0, CXTPChartColor(255, 0, 0, 0),
		CXTPChartColor(255, 130, 130, 130), CXTPChartColor(255, 50, 50, 50),
		location, direction, 0.0f, 180.0f, 1.0f, 0.0f, 0.0f);
	command->AddChildCommand(light0Command);

#if 1
	location = CXTPChartDiagramPoint(-scale * 2, -scale * 2, scale * 4);
	direction = location;
	direction.Revert();
	direction.Normalize();
	CXTPChartDeviceCommand* light1Command = new CXTPChartLightDeviceCommand(1, CXTPChartColor(255, 0, 0, 0),
		CXTPChartColor(255, 80, 80, 80), CXTPChartColor(255, 100, 100, 100),
		location, direction, 0.0f, 180.0f, 1.0f, 0.0f, 0.0f);
	light0Command->AddChildCommand(light1Command);
	innerLightingCommand = light1Command;
#else
	innerLightingCommand = light0Command;
#endif

	return command;
#endif

}
