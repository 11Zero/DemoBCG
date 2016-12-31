// XTPChartTorus3DDiagram.cpp
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

#include "../../Appearance/XTPChartAppearance.h"
#include "../../Types/XTPChartPie.h"

#include "XTPChartTorus3DDiagram.h"

//////////////////////////////////////////////////////////////////////////
// CXTPChartTorus3DDiagramSeriesView

CXTPChartTorus3DSeriesView::CXTPChartTorus3DSeriesView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView)
	: CXTPChartPie3DSeriesView(pSeries, pDiagramView)
{

}

CXTPChartTorus3DSeriesView::~CXTPChartTorus3DSeriesView()
{
}

CXTPChartSeriesPointView* CXTPChartTorus3DSeriesView::CreateSeriesPointView(CXTPChartDeviceContext* pDC, CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView)
{
	UNREFERENCED_PARAMETER(pDC);
	return new CXTPChartTorus3DSeriesPointView(pPoint, pParentView);
}

//////////////////////////////////////////////////////////////////////////
// CXTPChartTorus3DDiagramSeriesPointView

CXTPChartTorus3DSeriesPointView::CXTPChartTorus3DSeriesPointView(CXTPChartSeriesPoint* pPoint, CXTPChartElementView* pParentView)
	: CXTPChartPie3DSeriesPointView(pPoint, pParentView)
{
}

CXTPChartTorus3DSeriesPointView::~CXTPChartTorus3DSeriesPointView()
{
}



CXTPChartDeviceCommand* CXTPChartTorus3DSeriesPointView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);

	CXTPChartColor color1 = GetColor();
	CXTPChartColor color2 = GetColor2();

	return m_pPie->CreateTorusDeviceCommand(color1, color2);
}
