// XTPChartSeriesLabel.cpp
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

#include "XTPChartSeriesLabel.h"
#include "XTPChartSeriesView.h"
#include "XTPChartSeriesPoint.h"
#include "XTPChartSeries.h"
#include "XTPChartSeriesStyle.h"

#include "XTPChartContent.h"

#include "Utils/XTPChartMathUtils.h"
#include "Types/XTPChartDiagramPoint.h"
#include "Appearance/XTPChartBorder.h"
#include "Appearance/XTPChartFillStyle.h"
#include "Appearance/XTPChartAppearance.h"

#include "Drawing/XTPChartDeviceCommand.h"
#include "Drawing/XTPChartDeviceContext.h"
#include "Drawing/XTPChartLineDeviceCommand.h"


//////////////////////////////////////////////////////////////////////////
// CXTPChartSeriesLabel

IMPLEMENT_SERIAL(CXTPChartSeriesLabel, CXTPChartTextElement, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPChartSeriesLabel::CXTPChartSeriesLabel()
{
	m_pFont = CXTPChartFont::GetTahoma8();
	m_clrTextColor = CXTPChartColor::Empty;
	m_clrBackColor = CXTPChartColor::Empty;
	m_clrLineColor = CXTPChartColor::Empty;

	m_pBorder = new CXTPChartBorder(this);

	m_pFillStyle = new CXTPChartFillStyle(this);
	m_pFillStyle->SetFillMode(xtpChartFillSolid);

	m_bAntialiasing = FALSE;
	m_bVisible = TRUE;
	m_bShowLines = TRUE;
	m_nLineLength = 10;

	m_strFormat = _T("{V}");

}

CXTPChartSeriesLabel::~CXTPChartSeriesLabel()
{
	SAFE_RELEASE(m_pFont);
	SAFE_RELEASE(m_pBorder);
	SAFE_RELEASE(m_pFillStyle);
}

void CXTPChartSeriesLabel::SetFont(CXTPChartFont* pFont)
{
	SAFE_RELEASE(m_pFont);
	m_pFont = pFont;
	OnChartChanged();
}

CXTPChartString CXTPChartSeriesLabel::GetPointLabel(CXTPChartSeriesPoint* pPoint, int nValueIndex) const
{
	if (!pPoint->GetLabelText().IsEmpty())
		return pPoint->GetLabelText();

	double dValue = pPoint->GetValue(nValueIndex == - 1 ? pPoint->GetSeries()->GetStyle()->GetSeriesPointValueIndex() : nValueIndex);

	CString strValue;

	if (fabs(dValue) < 1e-6)
		strValue = _T("0");
	else
		strValue.Format(_T("%g"), dValue);

	CString strFormat = m_strFormat;
	strFormat.Replace(_T("{V}"), strValue);
	strFormat.Replace(_T("{A}"), pPoint->GetArgument());
	strFormat.Replace(_T("{S}"), pPoint->GetSeries()->GetName());

	CString str;
	str.Format(strFormat, dValue);

	return CXTPChartString(str);
}

CXTPChartColor CXTPChartSeriesLabel::GetActualBackColor() const
{
	if (!m_clrBackColor.IsEmpty())
		return m_clrBackColor;

	CXTPChartSeriesStyle* pStyle = DYNAMIC_DOWNCAST(CXTPChartSeriesStyle, GetOwner());
	ASSERT(pStyle);

	return pStyle->GetStyleAppearance()->GetLabelAppearance()->BackgroundColor;

}

void CXTPChartSeriesLabel::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Bool(pPX, _T("Visible"), m_bVisible, TRUE);
	PX_Bool(pPX, _T("Antialiasing"), m_bAntialiasing, FALSE);
	PX_Bool(pPX, _T("ShowLines"), m_bShowLines, TRUE);
	PX_Int(pPX, _T("LineLength"), m_nLineLength, 10);

	PX_String(pPX, _T("Format"), m_strFormat, _T("{V}"));

	PX_Font(pPX, _T("Font"), m_pFont);

	PX_Color(pPX, _T("BackColor"), m_clrBackColor);
	PX_Color(pPX, _T("TextColor"), m_clrTextColor);
	PX_Color(pPX, _T("LineColor"), m_clrLineColor);

	CXTPPropExchangeSection secFillStyle(pPX->GetSection(_T("FillStyle")));
	m_pFillStyle->DoPropExchange(&secFillStyle);

	CXTPPropExchangeSection secBorder(pPX->GetSection(_T("Border")));
	m_pBorder->DoPropExchange(&secBorder);

}

//////////////////////////////////////////////////////////////////////////
// CXTPChartSeriesLabelView

CXTPChartSeriesLabelView::CXTPChartSeriesLabelView(CXTPChartSeriesLabel* pLabel, CXTPChartSeriesPointView* pPointView, CXTPChartElementView* pParentView)
	: CXTPChartElementView(pParentView)
{
	m_pLabel = pLabel;
	m_pPointView = pPointView;
}

void CXTPChartSeriesLabelView::CalculateLayout(CXTPChartDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
}

CXTPChartColor CXTPChartSeriesLabelView::GetActualTextColor() const
{
	if (!m_pLabel->GetTextColor().IsEmpty())
		return m_pLabel->GetTextColor();

	CXTPChartSeriesStyle* pStyle = (CXTPChartSeriesStyle*)m_pLabel->GetOwner();

	CXTPChartColor clrTextColor = pStyle->GetStyleAppearance()->GetLabelAppearance()->TextColor;
	if (!clrTextColor.IsEmpty())
		return clrTextColor;

	return m_pPointView->GetColor().GetDarkColor();
}

CXTPChartColor CXTPChartSeriesLabelView::GetActualBorderColor() const
{
	CXTPChartColor clrBorderColor = m_pLabel->GetBorder()->GetColor();
	if (!clrBorderColor.IsEmpty())
		return clrBorderColor;

	CXTPChartSeriesStyle* pStyle = (CXTPChartSeriesStyle*)m_pLabel->GetOwner();

	clrBorderColor = pStyle->GetStyleAppearance()->GetLabelAppearance()->BorderColor;
	if (!clrBorderColor.IsEmpty())
		return clrBorderColor;

	return m_pPointView->GetColor().GetDarkColor();
}

CXTPChartColor CXTPChartSeriesLabelView::GetActualConnectorColor() const
{
	CXTPChartColor clrLineColor = m_pLabel->GetLineColor();
	if (!clrLineColor.IsEmpty())
		return clrLineColor;

	CXTPChartSeriesStyle* pStyle = (CXTPChartSeriesStyle*)m_pLabel->GetOwner();

	clrLineColor = pStyle->GetStyleAppearance()->GetLabelAppearance()->ConnectorColor;
	if (!clrLineColor.IsEmpty())
		return clrLineColor;

	return m_pPointView->GetColor().GetDarkColor();
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartSeriesLabelConnectorPainterBase

CXTPChartSeriesLabelConnectorPainterBase::CXTPChartSeriesLabelConnectorPainterBase(CXTPChartPointF startPoint, CXTPChartPointF finishPoint, CXTPChartRectF bounds)
{
	m_startPoint = startPoint;
	m_finishPoint = finishPoint;
	m_bounds = bounds;
}

enum XTPChartAngleOrientation
{
	xtpChartAngleQuadrant1,
	xtpChartAngleQuadrant2,
	xtpChartAngleQuadrant3,
	xtpChartAngleQuadrant4,
	xtpChartAngle0,
	xtpChartAngle90,
	xtpChartAngle180,
	xtpChartAngle270,
};

static XTPChartAngleOrientation GetAngleOrientation(double angle, double deadAngle)
{
	angle = CXTPChartMathUtils::NormalizeRadian(angle);
	angle = angle < 0 ? 2 * CXTPChartMathUtils::PI + angle : angle;
	XTPChartAngleOrientation disposition;
	if (CXTPChartMathUtils::Compare(angle, 0, deadAngle) == 0)
		disposition = xtpChartAngle0;
	else if (CXTPChartMathUtils::Compare(angle, CXTPChartMathUtils::PI / 2.0, deadAngle) == 0)
		disposition = xtpChartAngle90;
	else if (CXTPChartMathUtils::Compare(angle, CXTPChartMathUtils::PI, deadAngle) == 0)
		disposition = xtpChartAngle180;
	else if (CXTPChartMathUtils::Compare(angle, 3 * CXTPChartMathUtils::PI / 2.0, deadAngle) == 0)
		disposition = xtpChartAngle270;
	else if (angle > 0 && angle < CXTPChartMathUtils::PI / 2.0)
		disposition = xtpChartAngleQuadrant1;
	else if (angle > CXTPChartMathUtils::PI / 2.0 && angle < CXTPChartMathUtils::PI)
		disposition = xtpChartAngleQuadrant2;
	else if (angle > CXTPChartMathUtils::PI && angle < 3 * CXTPChartMathUtils::PI / 2.0)
		disposition = xtpChartAngleQuadrant3;
	else
		disposition = xtpChartAngleQuadrant4;
	return disposition;
}


CXTPChartRectF CXTPChartSeriesLabelConnectorPainterBase::CalcBorderBoundsForTangentDrawing(const CXTPChartDiagramPoint& point, double angle, const CXTPChartSizeF& textSize, int borderThickness, CXTPChartRectF& innerBounds)
{
	const double DeadAngle = CXTPChartMathUtils::PI / 180.0 * 10;

	CXTPChartRectF bounds(0, 0, (float)floor(textSize.Width + 2 * borderThickness), (float)floor(textSize.Height + 2 * borderThickness));
	CXTPChartDiagramPoint topLeft = point;

	XTPChartAngleOrientation disposition = GetAngleOrientation(angle, DeadAngle);
	switch (disposition)
	{
				case xtpChartAngle0:
					topLeft.Offset(0, -bounds.Height / 2.0);
					break;
				case xtpChartAngle90:
					topLeft.Offset(-bounds.Width / 2.0, -bounds.Height);
					break;
				case xtpChartAngle180:
					topLeft.Offset(-bounds.Width, -bounds.Height / 2.0);
					break;
				case xtpChartAngle270:
					topLeft.Offset(-bounds.Width / 2.0, 0);
					break;
				case xtpChartAngleQuadrant1:
					topLeft.Offset(0, -bounds.Height);
					break;
				case xtpChartAngleQuadrant2:
					topLeft.Offset(-bounds.Width, -bounds.Height);
					break;
				case xtpChartAngleQuadrant3:
					topLeft.Offset(-bounds.Width, 0);
					break;
				case xtpChartAngleQuadrant4:
					break;
	}
	bounds.Offset(topLeft);
	bounds.Round();

	innerBounds = bounds;
	innerBounds.Inflate((float)-borderThickness, (float)-borderThickness);
	return bounds;

}

//////////////////////////////////////////////////////////////////////////
// CXTPChartSeriesLabelLineConnectorPainter

CXTPChartSeriesLabelLineConnectorPainter::CXTPChartSeriesLabelLineConnectorPainter(CXTPChartPointF startPoint, CXTPChartPointF finishPoint, double angle, CXTPChartRectF bounds)
	: CXTPChartSeriesLabelConnectorPainterBase(startPoint, finishPoint, bounds)
{
	m_angle = angle;
}

CXTPChartDeviceCommand* CXTPChartSeriesLabelLineConnectorPainter::CreateDeviceCommand(CXTPChartDeviceContext* pDC, CXTPChartSeriesLabel* pLabel, const CXTPChartColor& color)
{
	UNREFERENCED_PARAMETER(pDC);
	UNREFERENCED_PARAMETER(pLabel);

	CXTPChartDeviceCommand* pContaier = new CXTPChartPolygonAntialiasingDeviceCommand();
	pContaier->AddChildCommand(new CXTPChartSolidLineDeviceCommand(m_startPoint, m_finishPoint, color, 1));

	return pContaier;
}


