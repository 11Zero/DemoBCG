// XTPChartPieSeriesLabel.cpp
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

#include "../../Types/XTPChartPie.h"
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

#include "XTPChartPieDiagram.h"
#include "XTPChartPieSeriesLabel.h"
#include "XTPChartPieSeriesStyle.h"
#include "XTPChartPie3DDiagram.h"


//////////////////////////////////////////////////////////////////////////
// CXTPChartPieSeriesLabel

IMPLEMENT_SERIAL(CXTPChartPieSeriesLabel, CXTPChartSeriesLabel, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPChartPieSeriesLabel::CXTPChartPieSeriesLabel()
{
	m_nPosition = xtpChartPieLabelOutiside;
}

CXTPChartPieSeriesLabel::~CXTPChartPieSeriesLabel()
{
}

CXTPChartPieSeriesStyleBase* CXTPChartPieSeriesLabel::GetStyle() const
{
	return (CXTPChartPieSeriesStyleBase*)m_pOwner;
}

void CXTPChartPieSeriesLabel::DoPropExchange(CXTPPropExchange* pPX)
{
	CXTPChartSeriesLabel::DoPropExchange(pPX);

	PX_Enum(pPX, _T("Position"), m_nPosition, xtpChartPieLabelOutiside);
}

CXTPChartLabelsView::CXTPChartLabelsView(CXTPChartElementView* pParentView)
	: CXTPChartElementView(pParentView)
{

}

void CXTPChartLabelsView::AddLabelRectangle(CXTPChartRectF& rc)
{
	m_arrLabels.Add(rc);
}

CXTPChartPointF CXTPChartLabelsView::CalculateOffset(double /*lineAngle*/, const CXTPChartRectF& rc)
{
	UNREFERENCED_PARAMETER(rc);

	return CXTPChartPointF(0, 0);

	// Temporary disabled.
#if 0
	float top1 = rc.GetTop();
	float top2 = rc.GetTop();
	float height = rc.Height;

	while (true)
	{
		bool changed = false;

		for (int i = 0; i < m_arrLabels.GetSize(); i++)
		{
			if (top1 >= m_arrLabels[i].GetBottom() ||
				top1 + height <= m_arrLabels[i].GetTop())
				continue;

			if (rc.GetLeft() >= m_arrLabels[i].GetRight() || rc.GetRight() <= m_arrLabels[i].GetLeft())
				continue;

			changed = true;
			top1 = float(m_arrLabels[i].GetBottom() + 1e-6);
			break;
		}

		if (!changed)
			break;
	}

	if (fabs(top1 - rc.GetTop()) < 1e-6)
		return CXTPChartPointF(0, 0);

	while (true)
	{
		bool changed = false;

		for (int i = 0; i < m_arrLabels.GetSize(); i++)
		{
			if (top2 >= m_arrLabels[i].GetBottom() ||
				top2 + height <= m_arrLabels[i].GetTop())
				continue;

			if (rc.GetLeft() >= m_arrLabels[i].GetRight() || rc.GetRight() <= m_arrLabels[i].GetLeft())
				continue;

			changed = true;
			top2 = float(m_arrLabels[i].GetTop() - height - 1e-6);
			break;
		}

		if (!changed)
			break;
	}

	if (fabs(top1 - rc.GetTop()) < fabs(top2 - rc.GetTop()))
		return CXTPChartPointF(0, top1 - rc.GetTop());

	return CXTPChartPointF(0, top2 - rc.GetTop());
#endif
}

CXTPChartDeviceCommand* CXTPChartLabelsView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{
	CXTPChartDeviceCommand* pDrawingType = new CXTPChartDrawingTypeDeviceCommand(TRUE);

	for (int i = 0; i < m_arrChildren.GetSize(); i++)
	{
		pDrawingType->AddChildCommand(m_arrChildren[i]->CreateDeviceCommand(pDC));
	}

	return pDrawingType;
}

//////////////////////////////////////////////////////////////////////////
// CView;

class CXTPChartPieSeriesLabel::CView : public CXTPChartSeriesLabelView
{
public:
	CView(CXTPChartSeriesLabel* pLabel, CXTPChartSeriesPointView* pPointView, CXTPChartElementView* pParentView);

protected:
	virtual void CalculateLayout(CXTPChartDeviceContext* pDC);
	virtual CXTPChartDeviceCommand* CreateDeviceCommand(CXTPChartDeviceContext* pDC);

protected:

	XTPChartPieLabelPosition GetPosition() const;

};

CXTPChartPieSeriesLabel::CView::CView(CXTPChartSeriesLabel* pLabel, CXTPChartSeriesPointView* pPointView, CXTPChartElementView* pParentView)
	: CXTPChartSeriesLabelView(pLabel, pPointView, pParentView)
{

}

void CXTPChartPieSeriesLabel::CView::CalculateLayout(CXTPChartDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
}

BOOL CXTPChartPieSeriesLabel::IsInside() const
{
	XTPChartPieLabelPosition position = GetPosition();
	return position == xtpChartPieLabelInside || position == xtpChartPieLabelRadial;

}

XTPChartPieLabelPosition CXTPChartPieSeriesLabel::CView::GetPosition() const
{
	return ((CXTPChartPieSeriesLabel*)m_pLabel)->GetPosition();
}


CXTPChartPointF CXTPChartPieSeriesLabel::CalculateAnchorPointAndAngles(CXTPChartPieSeriesPointView* pPointView,
	int borderThickness, double& lineAngle)
{
	UNREFERENCED_PARAMETER(borderThickness);
	lineAngle = pPointView->GetPie()->GetHalfAngle();

	int nHolePercent = pPointView->GetPie()->GetHolePercent();

	if (nHolePercent > 0 && IsInside())
	{
		double fraction = nHolePercent / 100.0;
		double width = pPointView->GetPie()->GetBounds().Width / 2.0;
		double height = pPointView->GetPie()->GetBounds().Height / 2.0;
		double minWidth = width * fraction;
		double minHeight = height * fraction;
		if (minWidth <= 0.0 || minHeight <= 0.0)
			return CXTPChartPointF(0, 0);

		double startAngle = pPointView->GetPie()->GetStartAngle();
		double halfAngle = pPointView->GetPie()->GetHalfAngle();
		CXTPChartDiagramPoint center = pPointView->GetPie()->CalculateCenter(pPointView->GetBasePoint());
		CXTPChartEllipse maxEllipse(center, width, height);
		CXTPChartEllipse minEllipse(center, minWidth, minHeight);
		CXTPChartPie maxPie(startAngle, halfAngle, maxEllipse, 0, 0);
		CXTPChartPie minPie(startAngle, halfAngle, minEllipse, 0, 0);
		return CXTPChartPointF((maxPie.GetFinishPoint().X + minPie.GetFinishPoint().X) / 2.0f, (maxPie.GetFinishPoint().Y + minPie.GetFinishPoint().Y) / 2.0f);

	}

	CXTPChartRectF realBounds(pPointView->GetPie()->GetBounds());

	CXTPChartEllipse actualEllipse(pPointView->GetPie()->CalculateCenter(pPointView->GetBasePoint()), realBounds.Width / 2, realBounds.Height / 2);
	CXTPChartPie actualPie(pPointView->GetPie()->GetStartAngle(), pPointView->GetPie()->GetHalfAngle(), actualEllipse, 0, 0);

	if (!IsInside())
	{
		return actualPie.GetFinishPoint();
	}

	if (pPointView->m_dValue == 1)
	{
		lineAngle = 0;
		return actualPie.GetCenterPoint();
	}

	return CXTPChartPointF((actualPie.GetCenterPoint().X + actualPie.GetFinishPoint().X) / 2.0f, (actualPie.GetCenterPoint().Y + actualPie.GetFinishPoint().Y) / 2.0f);
}

CXTPChartDeviceCommand* CXTPChartPieSeriesLabel::CView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{

	CXTPChartDeviceCommand* pCommand = new CXTPChartHitTestElementCommand(m_pLabel);

	double lineAngle;

	int borderThickness = m_pLabel->GetBorder()->GetThickness();

	CXTPChartRectF realBounds(((CXTPChartPieSeriesPointView*)m_pPointView)->GetPie()->GetBounds());

	if (realBounds.Width < 1 || realBounds.Height < 1)
		return pCommand;

	CXTPChartPointF anchorPoint = ((CXTPChartPieSeriesLabel*)m_pLabel)->CalculateAnchorPointAndAngles((CXTPChartPieSeriesPointView*)m_pPointView,
		borderThickness, lineAngle);

	XTPChartPieLabelPosition position = GetPosition();

	CXTPChartString text(m_pLabel->GetPointLabel(m_pPointView->GetPoint()));

	CXTPChartColor clrTextColor = GetActualTextColor();

	if (position == xtpChartPieLabelInside)
	{
		CXTPChartTextPainter painter(pDC, text, m_pLabel);
		CXTPChartSizeF size = painter.GetSize();

		CXTPChartRectF bounds(0, 0, size.Width + 2 * borderThickness, size.Height + 2 * borderThickness);
		bounds.Offset(anchorPoint.X - bounds.Width / 2, anchorPoint.Y - bounds.Height / 2);
		bounds.Round();

		CXTPChartRectF innerBounds = bounds;
		innerBounds.Inflate((float)-borderThickness, (float)-borderThickness);

		CXTPChartPointF labelPoint = innerBounds.GetLocation();

		CXTPChartColor clrBackColor = m_pLabel->GetActualBackColor();
		pCommand->AddChildCommand(m_pLabel->GetFillStyle()->CreateDeviceCommand(bounds, clrBackColor, clrBackColor));

		painter.SetLocation(labelPoint);

		pCommand->AddChildCommand(painter.CreateDeviceCommand(pDC, clrTextColor));

		if (borderThickness)
		{
			CXTPChartColor clrBorderColor = GetActualBorderColor();

			pCommand->AddChildCommand(m_pLabel->GetBorder()->CreateDeviceCommand(bounds, clrBorderColor));
		}
	}
	else if (position == xtpChartPieLabelRadial)
	{
		CXTPChartTextPainter painter(pDC, text, m_pLabel);
		CXTPChartSizeF size = painter.GetSize();

		CXTPChartRectF bounds(0, 0, size.Width + 2 * borderThickness, size.Height + 2 * borderThickness);
		bounds.Offset(- bounds.Width / 2, - bounds.Height / 2);
		bounds.Round();

		CXTPChartRectF innerBounds = bounds;
		innerBounds.Inflate((float)-borderThickness, (float)-borderThickness);

		float fAngle = (float)(-(int)CXTPChartMathUtils::Radian2Degree(lineAngle) + 90);
		fAngle = (float)CXTPChartMathUtils::NormalizeDegree(fAngle);

		fAngle = fAngle <= 180 ? fAngle - 90 : fAngle + 90;

		CXTPChartDeviceCommand* pStateGraphicsCommand = pCommand->AddChildCommand(new CXTPChartSaveStateDeviceCommand());
		pStateGraphicsCommand = pStateGraphicsCommand->AddChildCommand(new CXTPChartTranslateDeviceCommand(anchorPoint.X, anchorPoint.Y, 0));
		pStateGraphicsCommand = pStateGraphicsCommand->AddChildCommand(new CXTPChartRotateDeviceCommand(fAngle));
		pStateGraphicsCommand = pStateGraphicsCommand->AddChildCommand(new CXTPChartPolygonAntialiasingDeviceCommand(TRUE));

		CXTPChartPointF labelPoint = innerBounds.GetLocation();

		CXTPChartColor clrBackColor = m_pLabel->GetActualBackColor();
		pStateGraphicsCommand->AddChildCommand(m_pLabel->GetFillStyle()->CreateDeviceCommand(bounds, clrBackColor, clrBackColor));

		painter.SetLocation(labelPoint);

		pStateGraphicsCommand->AddChildCommand(painter.CreateDeviceCommand(pDC, clrTextColor));

		if (borderThickness)
		{
			CXTPChartColor clrBorderColor = GetActualBorderColor();

			pStateGraphicsCommand->AddChildCommand(m_pLabel->GetBorder()->CreateDeviceCommand(bounds, clrBorderColor));
		}
	}

	else if (position == xtpChartPieLabelOutiside)
	{
		CXTPChartTextPainter painter(pDC, text, m_pLabel);
		CXTPChartSizeF size = painter.GetSize();

		CXTPChartPointF startPoint(anchorPoint);
		CXTPChartPointF finishPoint(anchorPoint.X + (float)(cos(lineAngle) * m_pLabel->GetLineLength()),
			anchorPoint.Y - (float)(sin(lineAngle) * m_pLabel->GetLineLength()));

		CXTPChartRectF innerBounds;
		CXTPChartRectF bounds = CXTPChartSeriesLabelConnectorPainterBase::CalcBorderBoundsForTangentDrawing(finishPoint, lineAngle, size, borderThickness, innerBounds);
		bounds.Round();
		innerBounds.Round();

		CXTPChartPointF offest = ((CXTPChartLabelsView*)m_pParentView)->CalculateOffset(lineAngle, bounds);

		bounds.Offset(offest);
		innerBounds.Offset(offest);
		finishPoint += offest;

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

		((CXTPChartLabelsView*)m_pParentView)->AddLabelRectangle(bounds);
	}


	return pCommand;
}


CXTPChartElementView* CXTPChartPieSeriesLabel::CreateView(CXTPChartDeviceContext* pDC, CXTPChartSeriesPointView* pPointView, CXTPChartElementView* pParentView)
{
	UNREFERENCED_PARAMETER(pDC);
	return new CView(this, pPointView, pParentView);
}



//////////////////////////////////////////////////////////////////////////
// CXTPChartPie3DSeriesLabel

IMPLEMENT_SERIAL(CXTPChartPie3DSeriesLabel, CXTPChartPieSeriesLabel, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPChartPie3DSeriesLabel::CXTPChartPie3DSeriesLabel()
{

};

CXTPChartPie3DSeriesLabel::~CXTPChartPie3DSeriesLabel()
{

}

CXTPChartPointF CXTPChartPie3DSeriesLabel::CalculateAnchorPointAndAngles(CXTPChartPieSeriesPointView* pPointView,
	int borderThickness, double& lineAngle)
{
	UNREFERENCED_PARAMETER(borderThickness);

	lineAngle = 0;
	CXTPChartPointF labelPoint;
	CXTPChartPie* pPie = pPointView->GetPie();

	int nHolePercent = pPie->GetHolePercent();
	CXTPChartRectF rect(pPie->GetBounds());

	if (nHolePercent > 0 && IsInside())
	{
		double fraction = nHolePercent / 100.0;
		double width = rect.Width / 2.0;
		double height = rect.Height / 2.0;
		double minWidth = width * fraction;
		double minHeight = height * fraction;

		double startAngle = pPie->GetStartAngle();
		double halfAngle = pPie->GetHalfAngle();
		CXTPChartDiagramPoint center = pPie->CalculateCenter(pPointView->GetBasePoint());
		CXTPChartEllipse maxEllipse(center, width, height);
		CXTPChartEllipse minEllipse(center, minWidth, minHeight);
		CXTPChartPie maxPie(startAngle, halfAngle, maxEllipse, 0, 0);
		CXTPChartPie minPie(startAngle, halfAngle, minEllipse, 0, 0);
		labelPoint = CXTPChartPointF((maxPie.GetFinishPoint().X + minPie.GetFinishPoint().X) / 2.0f, (maxPie.GetFinishPoint().Y + minPie.GetFinishPoint().Y) / 2.0f);

		labelPoint.X = labelPoint.X - pPointView->GetBasePoint().X;
		labelPoint.Y = pPointView->GetBasePoint().Y - labelPoint.Y;
	}
	else
	{
		CXTPChartEllipse actualEllipse(pPie->CalculateCenter(pPointView->GetBasePoint()), rect.Width / 2, rect.Height / 2);
		CXTPChartPie actualPie(pPie->GetStartAngle(), pPie->GetHalfAngle(), actualEllipse, GetStyle()->GetDepth(), GetStyle()->GetHolePercent());

		labelPoint = CXTPChartPointF((actualPie.GetFinishPoint().X - actualPie.GetCenterPoint().X), (actualPie.GetCenterPoint().Y - actualPie.GetFinishPoint().Y));
		if (IsInside())
		{
			labelPoint.X /= 2;
			labelPoint.Y /= 2;

		}

		labelPoint.X = labelPoint.X + (actualPie.GetCenterPoint().X - pPointView->GetBasePoint().X);
		labelPoint.Y = labelPoint.Y - (actualPie.GetCenterPoint().Y - pPointView->GetBasePoint().Y);
	}

	CXTPChartPie3DSeriesView* pView =  (CXTPChartPie3DSeriesView*)pPointView->GetSeriesView();


	return Project((CXTPChartPie3DDiagramDomain*)pView->GetDomain(), labelPoint, lineAngle);
}

CXTPChartPointF CXTPChartPie3DSeriesLabel::Project(CXTPChartPie3DDiagramDomain* pDomain, CXTPChartPointF labelPoint, double& lineAngle)
{
	CXTPChartDiagramPoint p1 = pDomain->Project(CXTPChartDiagramPoint(labelPoint.X, labelPoint.Y));
	CXTPChartDiagramPoint p2 = pDomain->Project(CXTPChartDiagramPoint(labelPoint.X, labelPoint.Y, pDomain->m_dDepth));

	double z = p2.Z < p1.Z ? pDomain->m_dDepth : 0.0;
	CXTPChartPointF anchorPoint = (CXTPChartPointF)pDomain->Project(CXTPChartDiagramPoint(labelPoint.X, labelPoint.Y, z));
	CXTPChartPointF crossPoint = (CXTPChartPointF)pDomain->Project(CXTPChartDiagramPoint(labelPoint.Y, -labelPoint.X, z));
	CXTPChartPointF zeroPoint = (CXTPChartPointF)pDomain->Project(CXTPChartDiagramPoint(0.0, 0.0, z));

	lineAngle = atan2((double)(zeroPoint.Y - anchorPoint.Y), (double)(anchorPoint.X - zeroPoint.X));

	return anchorPoint;
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartTorus3DSeriesLabel

IMPLEMENT_SERIAL(CXTPChartTorus3DSeriesLabel, CXTPChartPie3DSeriesLabel, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPChartTorus3DSeriesLabel::CXTPChartTorus3DSeriesLabel()
{
	m_nPosition = xtpChartPieLabelOutiside;
}

CXTPChartTorus3DSeriesLabel::~CXTPChartTorus3DSeriesLabel()
{

}

CXTPChartPointF CXTPChartTorus3DSeriesLabel::Project(CXTPChartPie3DDiagramDomain* pDomain, CXTPChartPointF labelPoint, double& lineAngle)
{
	double z = pDomain->m_dDepth / 2;
	CXTPChartPointF anchorPoint = (CXTPChartPointF)pDomain->Project(CXTPChartDiagramPoint(labelPoint.X, labelPoint.Y, z));
	CXTPChartPointF crossPoint = (CXTPChartPointF)pDomain->Project(CXTPChartDiagramPoint(labelPoint.Y, -labelPoint.X, z));
	CXTPChartPointF zeroPoint = (CXTPChartPointF)pDomain->Project(CXTPChartDiagramPoint(0.0, 0.0, z));

	lineAngle = atan2((double)(zeroPoint.Y - anchorPoint.Y), (double)(anchorPoint.X - zeroPoint.X));

	return anchorPoint;
}

CXTPChartPointF CXTPChartTorus3DSeriesLabel::CalculateAnchorPointAndAngles(CXTPChartPieSeriesPointView* pPointView,
	int borderThickness, double& lineAngle)
{
	UNREFERENCED_PARAMETER(borderThickness);

	lineAngle = 0;
	CXTPChartPointF labelPoint;
	CXTPChartPie* pPie = pPointView->GetPie();

	CXTPChartRectF rect(pPie->GetBounds());


	CXTPChartEllipse actualEllipse(pPie->CalculateCenter(pPointView->GetBasePoint()), rect.Width / 2, rect.Height / 2);
	CXTPChartPie actualPie(pPie->GetStartAngle(), pPie->GetHalfAngle(), actualEllipse, GetStyle()->GetDepth(), GetStyle()->GetHolePercent());

	labelPoint = CXTPChartPointF((actualPie.GetFinishPoint().X - actualPie.GetCenterPoint().X), (actualPie.GetCenterPoint().Y - actualPie.GetFinishPoint().Y));


	labelPoint.X = labelPoint.X + (actualPie.GetCenterPoint().X - pPointView->GetBasePoint().X);
	labelPoint.Y = labelPoint.Y - (actualPie.GetCenterPoint().Y - pPointView->GetBasePoint().Y);


	CXTPChartPie3DSeriesView* pView =  (CXTPChartPie3DSeriesView*)pPointView->GetSeriesView();


	return Project((CXTPChartPie3DDiagramDomain*)pView->GetDomain(), labelPoint, lineAngle);

}



#ifdef _XTP_ACTIVEX

BEGIN_DISPATCH_MAP(CXTPChartPieSeriesLabel, CXTPChartSeriesLabel)
	DISP_PROPERTY_NOTIFY_ID(CXTPChartPieSeriesLabel, "Position", 106, m_nPosition, OleChartChanged, VT_I4)
END_DISPATCH_MAP()


// {CDFBCC77-27BF-4cb1-9ABF-4558D9835223}
static const GUID IID_IChartPieSeriesLabel =
{ 0xcdfbcc77, 0x27bf, 0x4cb1, { 0x9a, 0xbf, 0x45, 0x58, 0xd9, 0x83, 0x52, 0x23 } };

BEGIN_INTERFACE_MAP(CXTPChartPieSeriesLabel, CXTPChartSeriesLabel)
INTERFACE_PART(CXTPChartPieSeriesLabel, IID_IChartPieSeriesLabel, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPChartPieSeriesLabel, IID_IChartPieSeriesLabel)


#endif
