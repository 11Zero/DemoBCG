// XTPChartPyramid3DSeriesStyle.cpp
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

#include "XTPChartPyramid3DSeriesStyle.h"
#include "XTPChartPyramidSeriesLabel.h"
#include "XTPChartPyramid3DDiagram.h"
#include "XTPChartPyramid3DSeriesLabel.h"

//////////////////////////////////////////////////////////////////////////
// CXTPChartPyramid3DSeriesStyle

IMPLEMENT_SERIAL(CXTPChartPyramid3DSeriesStyle, CXTPChartPyramidSeriesStyle, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)


CXTPChartPyramid3DSeriesStyle::CXTPChartPyramid3DSeriesStyle()
{
	m_nTransparency = 235;

	SetLabel(new CXTPChartPyramid3DSeriesLabel());
}

CXTPChartPyramid3DSeriesStyle::~CXTPChartPyramid3DSeriesStyle()
{

}


CXTPChartDiagram* CXTPChartPyramid3DSeriesStyle::CreateDiagram()
{
	return new CXTPChartPyramid3DDiagram();
}

CXTPChartSeriesView* CXTPChartPyramid3DSeriesStyle::CreateView(CXTPChartSeries* pSeries, CXTPChartDiagramView* pDiagramView)
{
	return new CXTPChartPyramid3DSeriesView(pSeries, pDiagramView);
}


BOOL CXTPChartPyramid3DSeriesStyle::IsStyleDiagram(CXTPChartDiagram* pDiagram) const
{
	return DYNAMIC_DOWNCAST(CXTPChartPyramid3DDiagram, pDiagram) != NULL;

}

#ifdef _XTP_ACTIVEX


BEGIN_DISPATCH_MAP(CXTPChartPyramid3DSeriesStyle, CXTPChartPyramidSeriesStyle)
	DISP_PROPERTY_NOTIFY_ID(CXTPChartPyramid3DSeriesStyle, "Transparency", 108, m_nTransparency, OleChartChanged, VT_I4)
END_DISPATCH_MAP()


// {790BCC77-27BF-4cb1-9ABF-4558D9835223}
static const GUID IID_IChartPyramid3DSeriesStyle =
{ 0x790bcc77, 0x27bf, 0x4cb1, { 0x9a, 0xbf, 0x45, 0x58, 0xd9, 0x83, 0x52, 0x23 } };

BEGIN_INTERFACE_MAP(CXTPChartPyramid3DSeriesStyle, CXTPChartPyramidSeriesStyle)
INTERFACE_PART(CXTPChartPyramid3DSeriesStyle, IID_IChartPyramid3DSeriesStyle, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPChartPyramid3DSeriesStyle, IID_IChartPyramid3DSeriesStyle)

// {79090E6F-92B6-4671-9613-6B2A0FBF80A8}
IMPLEMENT_OLECREATE_EX2(CXTPChartPyramid3DSeriesStyle, "Codejock.ChartPyramid3DSeriesStyle." _XTP_AXLIB_VERSION,
0x79090e6f, 0x92b6, 0x4671, 0x96, 0x13, 0x6b, 0x2a, 0xf, 0xbf, 0x80, 0xa8);


#endif
