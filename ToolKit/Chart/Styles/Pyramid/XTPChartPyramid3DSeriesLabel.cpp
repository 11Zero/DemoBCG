// XTPChartPyramid3DSeriesLabel.cpp
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
#include "../../Appearance/XTPChartFillStyle.h"
#include "../../Appearance/XTPChartBorder.h"

#include "../../Drawing/XTPChartDeviceContext.h"
#include "../../Drawing/XTPChartDeviceCommand.h"
#include "../../Drawing/XTPChartRectangleDeviceCommand.h"
#include "../../Drawing/XTPChartTransformationDeviceCommand.h"

#include "../../Utils/XTPChartMathUtils.h"
#include "../../Utils/XTPChartTextPainter.h"

#include "../../XTPChartElementView.h"

#include "XTPChartPyramidDiagram.h"
#include "XTPChartPyramid3DSeriesLabel.h"
#include "XTPChartPyramid3DSeriesStyle.h"
#include "XTPChartPyramid3DDiagram.h"


//////////////////////////////////////////////////////////////////////////
// CXTPChartPyramid3DSeriesLabel

IMPLEMENT_SERIAL(CXTPChartPyramid3DSeriesLabel, CXTPChartPyramidSeriesLabel, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPChartPyramid3DSeriesLabel::CXTPChartPyramid3DSeriesLabel()
{
}

CXTPChartPyramid3DSeriesLabel::~CXTPChartPyramid3DSeriesLabel()
{
}

CXTPChartPyramid3DSeriesStyle* CXTPChartPyramid3DSeriesLabel::GetStyle() const
{
	return (CXTPChartPyramid3DSeriesStyle*)m_pOwner;
}


//////////////////////////////////////////////////////////////////////////
// CView;

class CXTPChartPyramid3DSeriesLabel::CView : public CXTPChartSeriesLabelView
{
public:
	CView(CXTPChartSeriesLabel* pLabel, CXTPChartSeriesPointView* pPointView, CXTPChartElementView* pParentView);

protected:
	virtual void CalculateLayout(CXTPChartDeviceContext* pDC);
	virtual CXTPChartDeviceCommand* CreateDeviceCommand(CXTPChartDeviceContext* pDC);

protected:

	XTPChartPyramidLabelPosition GetPosition() const;

};

CXTPChartPyramid3DSeriesLabel::CView::CView(CXTPChartSeriesLabel* pLabel, CXTPChartSeriesPointView* pPointView, CXTPChartElementView* pParentView)
	: CXTPChartSeriesLabelView(pLabel, pPointView, pParentView)
{

}

void CXTPChartPyramid3DSeriesLabel::CView::CalculateLayout(CXTPChartDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
}


XTPChartPyramidLabelPosition CXTPChartPyramid3DSeriesLabel::CView::GetPosition() const
{
	return ((CXTPChartPyramid3DSeriesLabel*)m_pLabel)->GetPosition();
}


CXTPChartDeviceCommand* CXTPChartPyramid3DSeriesLabel::CView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{

	CXTPChartDeviceCommand* pCommand = new CXTPChartHitTestElementCommand(m_pLabel);

	double lineAngle;

	int borderThickness = m_pLabel->GetBorder()->GetThickness();

	CXTPChartPointF anchorPoint = ((CXTPChartPyramid3DSeriesLabel*)m_pLabel)->CalculateAnchorPointAndAngles((CXTPChartPyramid3DSeriesPointView*)m_pPointView,
		lineAngle);

	XTPChartPyramidLabelPosition position = GetPosition();

	if (position != xtpChartPyramidLabelCenter)
	{
		CXTPChartString text(m_pLabel->GetPointLabel(m_pPointView->GetPoint()));

		CXTPChartColor clrTextColor = GetActualTextColor();

		CXTPChartTextPainter painter(pDC, text, m_pLabel);
		CXTPChartSizeF size = painter.GetSize();

		CXTPChartPointF startPoint(anchorPoint);
		CXTPChartPointF finishPoint(anchorPoint.X + (float)(cos(lineAngle) * m_pLabel->GetLineLength()),
			anchorPoint.Y - (float)(sin(lineAngle) * m_pLabel->GetLineLength()));

		CXTPChartRectF innerBounds;
		CXTPChartRectF bounds = CXTPChartSeriesLabelConnectorPainterBase::CalcBorderBoundsForTangentDrawing(finishPoint, lineAngle, size, borderThickness, innerBounds);
		bounds.Round();
		innerBounds.Round();

		CXTPChartPointF labelPoint = innerBounds.GetLocation();

		CXTPChartColor clrBackColor = m_pLabel->GetActualBackColor();
		pCommand->AddChildCommand(m_pLabel->GetFillStyle()->CreateDeviceCommand(bounds, clrBackColor, clrBackColor));

		painter.SetLocation(labelPoint);
		pCommand->AddChildCommand(painter.CreateDeviceCommand(pDC, clrTextColor));


		if (m_pLabel->GetShowLines())
		{
			CXTPChartColor clrConnectorColor = GetActualConnectorColor();

			CXTPChartSeriesLabelLineConnectorPainter linePainter(startPoint, finishPoint, lineAngle, bounds);
			pCommand->AddChildCommand(linePainter.CreateDeviceCommand(pDC, m_pLabel, clrConnectorColor));
		}


		if (borderThickness)
		{
			CXTPChartColor clrBorderColor = GetActualBorderColor();

			pCommand->AddChildCommand(m_pLabel->GetBorder()->CreateInnerBorderDeviceCommand(bounds, clrBorderColor));
		}
	}


	return pCommand;
}


CXTPChartElementView* CXTPChartPyramid3DSeriesLabel::CreateView(CXTPChartDeviceContext* pDC, CXTPChartSeriesPointView* pPointView, CXTPChartElementView* pParentView)
{
	UNREFERENCED_PARAMETER(pDC);
	return new CView(this, pPointView, pParentView);
}


CXTPChartPointF CXTPChartPyramid3DSeriesLabel::CalculateAnchorPointAndAngles(CXTPChartPyramidSeriesPointView* pPointView, double& lineAngle)
{
	CXTPChartPyramid3DSeriesView* pView =  (CXTPChartPyramid3DSeriesView*)pPointView->GetSeriesView();

	double dPos = (pPointView->m_dFrom + pPointView->m_dTo) / 2;

	CXTPChartRectF rc = pView->GetInnerBounds();

	return Project((CXTPChartPyramid3DDiagramDomain*)pView->m_pDomain, dPos, rc.Width, lineAngle);

}

CXTPChartPointF CXTPChartPyramid3DSeriesLabel::Project(CXTPChartPyramid3DDiagramDomain* pDomain, double dPos, double dSize, double& lineAngle)
{
	CXTPChartPointF anchorPoint, anchorPoint1;
	BOOL bRightMost = GetPosition() == xtpChartPyramidLabelLeft ? 1 : 0;
	lineAngle = bRightMost ? CXTPChartMathUtils::PI : 0;

	anchorPoint = (CXTPChartPointF)pDomain->Project(CXTPChartDiagramPoint(dSize / 2 * dPos, -dSize / 2 * dPos, -dSize / 2 + dSize * dPos));

	anchorPoint1 = (CXTPChartPointF)pDomain->Project(CXTPChartDiagramPoint(dSize / 2 * dPos, dSize / 2 * dPos, -dSize / 2 + dSize * dPos));
	if (bRightMost ^ (anchorPoint.X < anchorPoint1.X))
		anchorPoint = anchorPoint1;

	anchorPoint1 = (CXTPChartPointF)pDomain->Project(CXTPChartDiagramPoint(-dSize / 2 * dPos, -dSize / 2 * dPos, -dSize / 2 + dSize * dPos));
	if (bRightMost ^ (anchorPoint.X < anchorPoint1.X))
		anchorPoint = anchorPoint1;

	anchorPoint1 = (CXTPChartPointF)pDomain->Project(CXTPChartDiagramPoint(-dSize / 2 * dPos, dSize / 2 * dPos, -dSize / 2 + dSize * dPos));
	if (bRightMost ^ (anchorPoint.X < anchorPoint1.X))
		anchorPoint = anchorPoint1;

	anchorPoint = anchorPoint.Round();

	return anchorPoint;
}
