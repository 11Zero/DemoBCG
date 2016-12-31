// XTPChartHighLowSeriesStyle.cpp
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

#include "XTPChartHighLowSeriesStyle.h"

#include "../../XTPChartSeriesPoint.h"
#include "../../XTPChartContent.h"
#include "../../Appearance/XTPChartAppearance.h"

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

//////////////////////////////////////////////////////////////////////////
// CXTPChartHighLowSeriesStyle

IMPLEMENT_SERIAL(CXTPChartHighLowSeriesStyle, CXTPChartDiagram2DSeriesStyle, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPChartHighLowSeriesStyle::CXTPChartHighLowSeriesStyle()
{
	m_nLineThickness = 2;

	m_clrDownColor = CXTPChartColor::Empty;
	m_clrUpColor = CXTPChartColor::Empty;
}

CXTPChartHighLowSeriesStyle::~CXTPChartHighLowSeriesStyle()
{

}

CXTPChartSeriesView* CXTPChartHighLowSeriesStyle::CreateView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView)
{
	return new CXTPChartHighLowSeriesView(pSeries, pDiagramView);
}


CXTPChartColor CXTPChartHighLowSeriesStyle::GetUpColor() const
{
	if (!m_clrUpColor.IsEmpty())
		return m_clrUpColor;

	return GetContent()->GetAppearance()->GetFinanceStyleAppearance()->UpColor;

}
CXTPChartColor CXTPChartHighLowSeriesStyle::GetDownColor() const
{
	if (!m_clrDownColor.IsEmpty())
		return m_clrDownColor;

	return GetContent()->GetAppearance()->GetFinanceStyleAppearance()->DownColor;

}


//////////////////////////////////////////////////////////////////////////
// CXTPChartHighLowSeriesView

CXTPChartHighLowSeriesView::CXTPChartHighLowSeriesView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView)
	: CXTPChartDiagram2DSeriesView(pSeries,pDiagramView)
{

}
CXTPChartSeriesPointView* CXTPChartHighLowSeriesView::CreateSeriesPointView(CXTPChartDeviceContext* pDC, CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView)
{
	UNREFERENCED_PARAMETER(pDC);
	return new CXTPChartHighLowSeriesPointView(pPoint, pParentView);
}


CXTPChartDeviceCommand* CXTPChartHighLowSeriesView::CreateLegendDeviceCommand(CXTPChartDeviceContext* pDC, CRect rcBounds)
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

		pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
			CXTPChartPointF(pointHight), CXTPChartPointF(pointLow), color, 2));

		pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
			CXTPChartPointF(pointOpen), CXTPChartPointF((float)(pointOpen.x - nWidth / 2), (float)pointOpen.y), color, 2));

		pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
			CXTPChartPointF(pointClose), CXTPChartPointF((float)(pointClose.x + nWidth / 2), (float)pointClose.y), color, 2));
	}

	return pCommand;
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartHighLowSeriesPointView

CXTPChartHighLowSeriesPointView::CXTPChartHighLowSeriesPointView(CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView)
	: CXTPChartSeriesPointView(pPoint, pParentView)
{

}
CXTPChartPointF CXTPChartHighLowSeriesPointView::GetScreenPoint(int nIndex)
{
	CXTPChartHighLowSeriesView* pView = (CXTPChartHighLowSeriesView*)GetSeriesView();

	return pView->GetScreenPoint(m_pPoint->GetInternalArgumentValue(), m_pPoint->GetValue(nIndex));
}


CXTPChartDeviceCommand* CXTPChartHighLowSeriesPointView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);

	CPoint pointOpen = GetScreenPoint(chartOpen).Round();
	CPoint pointClose = GetScreenPoint(chartClose).Round();

	CPoint pointLow = GetScreenPoint(chartLow).Round();
	CPoint pointHight = GetScreenPoint(chartHigh).Round();

	BOOL bUpColor = TRUE;


	CXTPChartSeriesPoint* pPrevPoint = m_pPoint->GetPreviousPoint();
	if (pPrevPoint)
	{
		if (pPrevPoint->GetValue(chartClose) > m_pPoint->GetValue(chartClose))
		{
			bUpColor = FALSE;
		}
	}

	CXTPChartHighLowSeriesView* pView = (CXTPChartHighLowSeriesView*)GetSeriesView();

	CXTPChartAxisView* pAxisView = pView->GetAxisViewX();
	CXTPChartHighLowSeriesStyle* pStyle = (CXTPChartHighLowSeriesStyle*)GetSeriesView()->GetStyle();

	int nWidth = (int)(pAxisView->DistanceToPoint(1) * 0.5);
	if (nWidth < 5)
		nWidth = 5;

	int nLineThickness = pStyle->GetLineThickness();

	nWidth = (nWidth & ~1) + nLineThickness * 3;

	CXTPChartDeviceCommand* pCommand = new CXTPChartHitTestElementCommand(m_pPoint);

	CXTPChartColor color = bUpColor ? pStyle->GetUpColor() : pStyle->GetDownColor();


	pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
		CXTPChartPointF(pointHight), CXTPChartPointF(pointLow), color, nLineThickness));

	pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
		CXTPChartPointF(pointOpen), CXTPChartPointF((float)(pointOpen.x - nWidth / 2), (float)pointOpen.y), color, nLineThickness));

	pCommand->AddChildCommand(new CXTPChartSolidLineDeviceCommand(
		CXTPChartPointF(pointClose), CXTPChartPointF((float)(pointClose.x + nWidth / 2), (float)pointClose.y), color, nLineThickness));


	return pCommand;
}




#ifdef _XTP_ACTIVEX



BEGIN_DISPATCH_MAP(CXTPChartHighLowSeriesStyle, CXTPChartDiagram2DSeriesStyle)
	DISP_PROPERTY_EX_ID(CXTPChartHighLowSeriesStyle, "LineThickness", 100, GetLineThickness, SetLineThickness, VT_I4)
END_DISPATCH_MAP()


// {432BCC77-27BF-4cb1-9ABF-4558D9835223}
static const GUID IID_IChartHighLowSeriesStyle =
{ 0x432bcc77, 0x27bf, 0x4cb1, { 0x9a, 0xbf, 0x45, 0x58, 0xd9, 0x83, 0x52, 0x23 } };

BEGIN_INTERFACE_MAP(CXTPChartHighLowSeriesStyle, CXTPChartDiagram2DSeriesStyle)
	INTERFACE_PART(CXTPChartHighLowSeriesStyle, IID_IChartHighLowSeriesStyle, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPChartHighLowSeriesStyle, IID_IChartHighLowSeriesStyle)

// {43490E6F-92B6-4671-9613-6B2A0FBF80A8}
IMPLEMENT_OLECREATE_EX2(CXTPChartHighLowSeriesStyle, "Codejock.ChartHighLowSeriesStyle." _XTP_AXLIB_VERSION,
	0x43490e6f, 0x92b6, 0x4671, 0x96, 0x13, 0x6b, 0x2a, 0xf, 0xbf, 0x80, 0xa8);


#endif
