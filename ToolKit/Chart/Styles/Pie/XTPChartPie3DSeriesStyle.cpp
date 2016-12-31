// XTPChartPie3DSeriesStyle.cpp
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

#include "XTPChartPie3DSeriesStyle.h"
#include "XTPChartPieSeriesLabel.h"
#include "XTPChartPie3DDiagram.h"

//////////////////////////////////////////////////////////////////////////
// CXTPChartPie3DSeriesStyle

IMPLEMENT_SERIAL(CXTPChartPie3DSeriesStyle, CXTPChartPieSeriesStyleBase, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)


CXTPChartPie3DSeriesStyle::CXTPChartPie3DSeriesStyle()
{
	m_nDepth = 20;

	SetLabel(new CXTPChartPie3DSeriesLabel());
}

CXTPChartPie3DSeriesStyle::~CXTPChartPie3DSeriesStyle()
{

}

CXTPChartDiagram* CXTPChartPie3DSeriesStyle::CreateDiagram()
{
	return new CXTPChartPie3DDiagram();
}

CXTPChartSeriesView* CXTPChartPie3DSeriesStyle::CreateView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView)
{
	return new CXTPChartPie3DSeriesView(pSeries, pDiagramView);
}


BOOL CXTPChartPie3DSeriesStyle::IsStyleDiagram(CXTPChartDiagram* pDiagram) const
{
	return DYNAMIC_DOWNCAST(CXTPChartPie3DDiagram, pDiagram) != NULL;

}

#ifdef _XTP_ACTIVEX


BEGIN_DISPATCH_MAP(CXTPChartPie3DSeriesStyle, CXTPChartPieSeriesStyleBase)
	DISP_PROPERTY_NOTIFY_ID(CXTPChartPie3DSeriesStyle, "HolePercent", 100, m_nHolePercent, OleChartChanged, VT_I4)
	DISP_PROPERTY_NOTIFY_ID(CXTPChartPie3DSeriesStyle, "ExplodedDistancePercent", 101, m_nExplodedDistancePercent, OleChartChanged, VT_I4)
	DISP_PROPERTY_NOTIFY_ID(CXTPChartPie3DSeriesStyle, "Rotation", 102, m_nRotation, OleChartChanged, VT_I4)
END_DISPATCH_MAP()


// {413BCC77-27BF-4cb1-9ABF-4558D9835223}
static const GUID IID_IChartPie3DSeriesStyle =
{ 0x413bcc77, 0x27bf, 0x4cb1, { 0x9a, 0xbf, 0x45, 0x58, 0xd9, 0x83, 0x52, 0x23 } };

BEGIN_INTERFACE_MAP(CXTPChartPie3DSeriesStyle, CXTPChartPieSeriesStyleBase)
INTERFACE_PART(CXTPChartPie3DSeriesStyle, IID_IChartPie3DSeriesStyle, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPChartPie3DSeriesStyle, IID_IChartPie3DSeriesStyle)

// {41490E6F-92B6-4671-9613-6B2A0FBF80A8}
IMPLEMENT_OLECREATE_EX2(CXTPChartPie3DSeriesStyle, "Codejock.ChartPie3DSeriesStyle." _XTP_AXLIB_VERSION,
						0x41490e6f, 0x92b6, 0x4671, 0x96, 0x13, 0x6b, 0x2a, 0xf, 0xbf, 0x80, 0xa8);


#endif
