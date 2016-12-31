// XTPChartPie3DDiagram.cpp
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

#include "Common/XTPPropExchange.h"

#include "../../Appearance/XTPChartAppearance.h"
#include "../../XTPChartSeriesLabel.h"
#include "../../Drawing/XTPChartDeviceCommand.h"
#include "../../Drawing/XTPChartTransformationDeviceCommand.h"
#include "../../Utils/XTPChartMathUtils.h"
#include "../../XTPChartContent.h"

#include "../../Types/XTPChartPie.h"
#include "XTPChartPie3DDiagram.h"
#include "XTPChartPie3DSeriesStyle.h"
#include "XTPChartPieSeriesLabel.h"

//////////////////////////////////////////////////////////////////////////
// CXTPChartPie3DDiagram

IMPLEMENT_SERIAL(CXTPChartPie3DDiagram, CXTPChartDiagram3D, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPChartPie3DDiagram::CXTPChartPie3DDiagram()
{

}


CXTPChartDiagramView* CXTPChartPie3DDiagram::CreateView(CXTPChartDeviceContext* pDC, CXTPChartElementView* pParent)
{
	UNREFERENCED_PARAMETER(pDC);
	return new CXTPChartPie3DDiagramView(this, pParent);
}



void CXTPChartPie3DDiagram::CalculateSeriesLayout(CXTPChartDeviceContext* pDC, CXTPChartDiagramView* pView)
{
	if (pView->GetCount() == 0)
		return;

	for (int i = 0; i < pView->GetSeriesView()->GetCount(); i++)
	{
		CXTPChartPie3DSeriesView* pSeriesView = (CXTPChartPie3DSeriesView*)pView->GetSeriesView()->GetAt(i);

		pSeriesView->CalculatePointLayout(pDC, pView->GetBounds());
		pSeriesView->CalculateLabelLayout(pDC);
	}
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartPie3DDiagramView

CXTPChartPie3DDiagramView::CXTPChartPie3DDiagramView(CXTPChartDiagram* pDiagram, CXTPChartElementView* pParent)
	: CXTPChartPieDiagramView(pDiagram, pParent)
{
	m_ptOldPosition = CPoint(0, 0);
}

void CXTPChartPie3DDiagramView::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	m_pContainer->SetCapture(this);
	m_ptOldPosition = point;
}

void CXTPChartPie3DDiagramView::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	int dx = point.x - m_ptOldPosition.x;
	int dy = point.y - m_ptOldPosition.y;
	m_ptOldPosition = point;

	PerformRotation(dx, dy);
}

BOOL CXTPChartPie3DDiagramView::PerformRotation(int dx, int dy)
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
// CXTPChartPie3DDiagramSeriesView

CXTPChartPie3DSeriesView::CXTPChartPie3DSeriesView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView)
	: CXTPChartPieSeriesView(pSeries, pDiagramView)
{

}

CXTPChartPie3DSeriesView::~CXTPChartPie3DSeriesView()
{
}



CXTPChartDiagramDomain* CXTPChartPie3DSeriesView::CreateDiagramDomain(CXTPChartDeviceContext* pDC, CRect rcBounds)
{
	CXTPChartDiagram3D* pDiagram = (CXTPChartDiagram3D*)GetDiagram();

	CRect rcInnerBounds = rcBounds;

	CXTPChartSeriesStyle* pStyle = m_pSeries->GetStyle();

	CXTPChartPieSeriesLabel* pLabel = (CXTPChartPieSeriesLabel*)pStyle->GetLabel();

	if (pLabel->IsVisible() && !pLabel->IsInside())
	{
		CSize sz(0, 0);
		for (int i = 0; i < m_pPointsView->GetCount(); i++)
		{
			CXTPChartPieSeriesPointView* pPointView = (CXTPChartPieSeriesPointView*)m_pPointsView->GetAt(i);

			CXTPChartString text(pLabel->GetPointLabel(pPointView->GetPoint()));
			CXTPChartTextPainter painter(pDC, text, pLabel);

			sz.cx = max(sz.cx, (INT)painter.GetSize().Width);
			sz.cy = max(sz.cy, (INT)painter.GetSize().Height);
		}


		int nLineLength = pStyle->GetLabel()->GetLineLength();
		rcInnerBounds.DeflateRect(6 + nLineLength + sz.cx, 6 + nLineLength + sz.cy);
	}
	else
	{
		rcInnerBounds.DeflateRect(10, 10);
	}


	int nWidth = min(rcInnerBounds.Width(), rcInnerBounds.Height());

	if (nWidth < 0)
	{
		nWidth = 1;
		rcInnerBounds = CRect(rcBounds.CenterPoint(), CSize(1, 1));
	}

	CXTPChartRectF rcInnerBoundsF((float)(rcInnerBounds.left + rcInnerBounds.right - nWidth / 2),
		(float)(rcInnerBounds.top + rcInnerBounds.bottom - nWidth / 2), (float)nWidth, (float)nWidth);

	return new CXTPChartPie3DDiagramDomain(pDiagram, rcBounds, rcBounds, rcInnerBoundsF, ((CXTPChartPieSeriesStyleBase*)m_pSeries->GetStyle())->GetDepth());

}

void CXTPChartPie3DSeriesView::CalculateLabelLayout(CXTPChartDeviceContext* pDC)
{
	for (int i = 0; i < m_pLabelsView->GetCount(); i++)
	{
		CXTPChartSeriesLabelView* pLabelView = (CXTPChartSeriesLabelView*)m_pLabelsView->GetAt(i);

		pLabelView->CalculateLayout(pDC);
	}
}

CXTPChartSeriesPointView* CXTPChartPie3DSeriesView::CreateSeriesPointView(CXTPChartDeviceContext* pDC, CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView)
{
	UNREFERENCED_PARAMETER(pDC);
	return new CXTPChartPie3DSeriesPointView(pPoint, pParentView);
}

CXTPChartDeviceCommand* CXTPChartPie3DSeriesView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{
	CXTPChartDeviceCommand* pDrawingType = new CXTPChartDrawingTypeDeviceCommand(FALSE);

	CXTPChartPie3DDiagramDomain* pDomain = (CXTPChartPie3DDiagramDomain*)m_pDomain;

	CXTPChartDeviceCommand* pDeviceCommand = pDomain->CreateViewPortGraphicsCommand();
	pDrawingType->AddChildCommand(pDeviceCommand);

	CXTPChartDeviceCommand* pInnerCommand;
	pDeviceCommand->AddChildCommand(pDomain->CreateGraphicsCommand(pInnerCommand));
	pInnerCommand->AddChildCommand(m_pPointsView->CreateDeviceCommand(pDC));

	return pDrawingType;
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartPie3DDiagramSeriesPointView

CXTPChartPie3DSeriesPointView::CXTPChartPie3DSeriesPointView(CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView)
	: CXTPChartPieSeriesPointView(pPoint, pParentView)
{
}

CXTPChartPie3DSeriesPointView::~CXTPChartPie3DSeriesPointView()
{
}



CXTPChartDeviceCommand* CXTPChartPie3DSeriesPointView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);

	CXTPChartColor color1 = GetColor();
	CXTPChartColor color2 = GetColor2();

	return m_pPie->CreatePieDeviceCommand(color1, color2, CXTPChartPointF(0, 0));
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartPie3DDiagramDomain

CXTPChartPie3DDiagramDomain::CXTPChartPie3DDiagramDomain(CXTPChartDiagram3D* pDiagram, CRect& rcBounds, CRect& rcLabelBounds, CXTPChartRectF& rcInnerBounds, int nDepthPercent)
	:  CXTPChartDiagram3DDomain(pDiagram, rcBounds, rcLabelBounds, rcInnerBounds, nDepthPercent)
{
}


CXTPChartDeviceCommand* CXTPChartPie3DDiagramDomain::CreateGraphicsCommand(CXTPChartDeviceCommand*& pInnerCommand)
{
	CXTPChartDeviceCommand* depthCommand = new CXTPChartDepthTestDeviceCommand();
	CXTPChartDeviceCommand* saveStateCommand = new CXTPChartSaveStateDeviceCommand();
	depthCommand->AddChildCommand(saveStateCommand);
	saveStateCommand->AddChildCommand(new CXTPChartIdentityTransformDeviceCommand(xtpChartMatrixModelView));
	saveStateCommand->AddChildCommand(new CXTPChartIdentityTransformDeviceCommand(xtpChartMatrixProjection));
	CXTPChartDeviceCommand* innerLightingCommand;
	CXTPChartDeviceCommand* projectionCommand = ((CXTPChartPie3DDiagram*)m_pDiagram)->CreateProjectionGraphicsCommand(this);
	saveStateCommand->AddChildCommand(projectionCommand);
	projectionCommand->AddChildCommand(CreateLightingCommand(innerLightingCommand));
	innerLightingCommand->AddChildCommand(((CXTPChartPie3DDiagram*)m_pDiagram)->CreateModelViewGraphicsCommand(this));
	pInnerCommand = new CXTPChartPolygonAntialiasingDeviceCommand();
	innerLightingCommand->AddChildCommand(pInnerCommand);

	return depthCommand;
}


CXTPChartDeviceCommand* CXTPChartPie3DDiagramDomain::CreateAdditionalModelViewCommand()
{
	return new CXTPChartTranslateDeviceCommand(m_dWidth * 0.5, m_dHeight * 0.5, 0.0);
}

CXTPChartDeviceCommand* CXTPChartPie3DDiagramDomain::CreateLightingCommand(CXTPChartDeviceCommand*& innerLightingCommand)
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
	CXTPChartDiagramPoint location(-scale, 1.5 * scale, scale * 2);
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
