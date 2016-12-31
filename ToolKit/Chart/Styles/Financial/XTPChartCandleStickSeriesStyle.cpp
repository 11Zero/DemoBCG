// XTPChartCandleStickSeriesStyle.cpp
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

#include "XTPChartCandleStickSeriesStyle.h"

#include "../../XTPChartSeriesPoint.h"

#include "../../Diagram2D/XTPChartDiagram2D.h"
#include "../../Diagram2D/XTPChartDiagramPane.h"
#include "../../Diagram2D/XTPChartAxisView.h"
#include "../../Diagram2D/XTPChartDiagram2DView.h"

#include "../../Drawing/XTPChartDeviceCommand.h"
#include "../../Drawing/XTPChartDeviceContext.h"
#include "../../Drawing/XTPChartRectangleDeviceCommand.h"
#include "../../Drawing/XTPChartLineDeviceCommand.h"


enum ValueIndex
{
	chartLow,
	chartHigh,
	chartOpen,
	chartClose
};

IMPLEMENT_SERIAL(CXTPChartCandleStickSeriesStyle, CXTPChartHighLowSeriesStyle, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)


//////////////////////////////////////////////////////////////////////////
// CXTPChartCandleStickSeriesStyle

CXTPChartCandleStickSeriesStyle::CXTPChartCandleStickSeriesStyle()
{
}

CXTPChartCandleStickSeriesStyle::~CXTPChartCandleStickSeriesStyle()
{

}

CXTPChartSeriesView* CXTPChartCandleStickSeriesStyle::CreateView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView)
{
	return new CXTPChartCandleStickSeriesView(pSeries, pDiagramView);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartCandleStickSeriesView

CXTPChartCandleStickSeriesView::CXTPChartCandleStickSeriesView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView)
	: CXTPChartDiagram2DSeriesView(pSeries,pDiagramView)
{

}
CXTPChartSeriesPointView* CXTPChartCandleStickSeriesView::CreateSeriesPointView(CXTPChartDeviceContext* pDC, CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView)
{
	UNREFERENCED_PARAMETER(pDC);
	return new CXTPChartCandleStickSeriesPointView(pPoint, pParentView);
}


CXTPChartDeviceCommand* CXTPChartCandleStickSeriesView::CreateLegendDeviceCommand(CXTPChartDeviceContext* pDC, CRect rcBounds)
{
	UNREFERENCED_PARAMETER(pDC);
	rcBounds.DeflateRect(1, 1);

	CXTPChartDeviceCommand* pCommand = new CXTPChartDeviceCommand();

	CXTPChartHighLowSeriesStyle* pStyle = STATIC_DOWNCAST(CXTPChartHighLowSeriesStyle, m_pSeries->GetStyle());

	CPoint ptCenter(rcBounds.CenterPoint().x, rcBounds.CenterPoint().y) ;

	for (int i = 0; i < 2; i++)
	{
		CXTPChartColor color = i == 0 ? pStyle->GetUpColor() : pStyle->GetDownColor();

		int nHeight = rcBounds.Height() - 3;
		int x = i == 0 ? ptCenter.x - 4 : ptCenter.x + 4;
		int y = i == 0 ? rcBounds.top : rcBounds.top + 3;

		CPoint pointHight(x, y);
		CPoint pointLow(x, pointHight.y + nHeight);
		CPoint pointOpen(x, pointHight.y + nHeight / 3);
		CPoint pointClose(x, pointHight.y + nHeight * 2 / 3);
		int nWidth = 6;

		CXTPChartRectF rc((float)pointOpen.x - nWidth / 2.0f, (float)pointOpen.y, (float)nWidth, (float)pointClose.y  - pointOpen.y);
		rc.X += 2 / 2.0f;
		rc.Width -= 2;
		pCommand->AddChildCommand(new CXTPChartBoundedRectangleDeviceCommand(rc, color, 2));

		pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
			CXTPChartPointF(pointHight), CXTPChartPointF((float)pointHight.x, (float)min(pointOpen.y, pointClose.y)), color, 2));

		pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
			CXTPChartPointF(pointLow), CXTPChartPointF((float)pointHight.x, (float)max(pointOpen.y, pointClose.y)), color, 2));

	}

	return pCommand;
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartCandleStickSeriesPointView

CXTPChartCandleStickSeriesPointView::CXTPChartCandleStickSeriesPointView(CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView)
	: CXTPChartSeriesPointView(pPoint, pParentView)
{

}

CXTPChartPointF CXTPChartCandleStickSeriesPointView::GetScreenPoint(int nIndex)
{
	CXTPChartCandleStickSeriesView* pView = (CXTPChartCandleStickSeriesView*)GetSeriesView();

	return pView->GetScreenPoint(m_pPoint->GetInternalArgumentValue(), m_pPoint->GetValue(nIndex));
}


CXTPChartDeviceCommand* CXTPChartCandleStickSeriesPointView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);

	CPoint pointOpen = GetScreenPoint(chartOpen).Round();
	CPoint pointClose = GetScreenPoint(chartClose).Round();

	CPoint pointLow = GetScreenPoint(chartLow).Round();
	CPoint pointHight = GetScreenPoint(chartHigh).Round();

	BOOL bUpColor = TRUE;
	BOOL bFilled = TRUE;

	if (pointOpen.y > pointClose.y)
	{
		CPoint p = pointClose;
		pointClose = pointOpen;
		pointOpen = p;
		bFilled = FALSE;
	}

	CXTPChartSeriesPoint* pPrevPoint = m_pPoint->GetPreviousPoint();
	if (pPrevPoint)
	{
		if (pPrevPoint->GetValue(chartClose) > m_pPoint->GetValue(chartClose))
		{
			bUpColor = FALSE;
		}
	}
	CXTPChartCandleStickSeriesView* pView = (CXTPChartCandleStickSeriesView*)GetSeriesView();


	CXTPChartAxisView* pAxisView = pView->GetAxisViewX();
	CXTPChartCandleStickSeriesStyle* pStyle = (CXTPChartCandleStickSeriesStyle*)GetSeriesView()->GetStyle();


	int nWidth = (int)(pAxisView->DistanceToPoint(1) * 0.5);
	if (nWidth < 5)
		nWidth = 5;

	int nLineThickness = pStyle->GetLineThickness();

	nWidth = (nWidth & ~1) + nLineThickness;


	CXTPChartDeviceCommand* pCommand = new CXTPChartHitTestElementCommand(m_pPoint);

	CXTPChartRectF rc((float)pointOpen.x - nWidth / 2.0f, (float)pointOpen.y, (float)nWidth, (float)pointClose.y  - pointOpen.y);
	if (rc.Height < nLineThickness)
	{
		rc.Height = (float)nLineThickness;
		rc.Y -= (float)nLineThickness / 2;
	}

	CXTPChartColor color = bUpColor ? pStyle->GetUpColor() : pStyle->GetDownColor();

	if (bFilled)
	{
		pCommand->AddChildCommand(new CXTPChartSolidRectangleDeviceCommand(rc, color));
	}
	else
	{
		rc.X += nLineThickness / 2.0f;
		rc.Width -= nLineThickness;
		pCommand->AddChildCommand(new CXTPChartBoundedRectangleDeviceCommand(rc, color, nLineThickness));
	}


	pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
		CXTPChartPointF(pointHight), CXTPChartPointF((float)pointHight.x, (float)min(pointOpen.y, pointClose.y)), color, nLineThickness));

	pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
		CXTPChartPointF(pointLow), CXTPChartPointF((float)pointHight.x, (float)max(pointOpen.y, pointClose.y)), color, nLineThickness));

	return pCommand;
}




#ifdef _XTP_ACTIVEX



BEGIN_DISPATCH_MAP(CXTPChartCandleStickSeriesStyle, CXTPChartHighLowSeriesStyle)
END_DISPATCH_MAP()


// {442BCC77-27BF-4cb1-9ABF-4558D9835223}
static const GUID IID_IChartCandleStickSeriesStyle =
{ 0x442bcc77, 0x27bf, 0x4cb1, { 0x9a, 0xbf, 0x45, 0x58, 0xd9, 0x83, 0x52, 0x23 } };

BEGIN_INTERFACE_MAP(CXTPChartCandleStickSeriesStyle, CXTPChartHighLowSeriesStyle)
	INTERFACE_PART(CXTPChartCandleStickSeriesStyle, IID_IChartCandleStickSeriesStyle, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPChartCandleStickSeriesStyle, IID_IChartCandleStickSeriesStyle)

// {44490E6F-92B6-4671-9613-6B2A0FBF80A8}
IMPLEMENT_OLECREATE_EX2(CXTPChartCandleStickSeriesStyle, "Codejock.ChartCandleStickSeriesStyle." _XTP_AXLIB_VERSION,
						0x44490e6f, 0x92b6, 0x4671, 0x96, 0x13, 0x6b, 0x2a, 0xf, 0xbf, 0x80, 0xa8);


#endif
