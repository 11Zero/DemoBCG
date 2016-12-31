// XTPChartLineStyle.cpp
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


#include "XTPChartLineStyle.h"
#include "../XTPChartElement.h"

#include "../Drawing/XTPChartLineDeviceCommand.h"



CXTPChartLineStyle::CXTPChartLineStyle(CXTPChartElement* pOwner)
{
	m_pOwner = pOwner;
	m_nDashStyle = xtpChartDashStyleSolid;
	m_nThickness = 1;

#ifdef _XTP_ACTIVEX
	EnableAutomation();
	EnableTypeLib();
#endif
}

void CXTPChartLineStyle::SetDashStyle(XTPChartDashStyle nDashStyle)
{
	m_nDashStyle = nDashStyle;
	m_pOwner->OnChartChanged();
}

void CXTPChartLineStyle::SetThickness(int nThickness)
{
	m_nThickness = nThickness;
	m_pOwner->OnChartChanged();
}

CXTPChartDeviceCommand* CXTPChartLineStyle::CreateDeviceCommand(const CXTPChartPointF& point1, const CXTPChartPointF& point2, const CXTPChartColor& color)
{
	if (m_nDashStyle == xtpChartDashStyleEmpty)
		return NULL;

	if (m_nDashStyle == xtpChartDashStyleSolid)
		return new CXTPChartSolidLineDeviceCommand(point1, point2, color, m_nThickness);

	return new CXTPChartDashedLineDeviceCommand(point1, point2, color, m_nThickness, m_nDashStyle);
}


CXTPChartDeviceCommand* CXTPChartLineStyle::CreateDeviceCommand(const CXTPChartPoints& points, const CXTPChartColor& color)
{
	if (m_nDashStyle == xtpChartDashStyleEmpty)
		return NULL;

	return new CXTPChartSolidPolylineDeviceCommand(points, color, m_nThickness);
}


CXTPChartDeviceCommand* CXTPChartLineStyle::CreateSplineDeviceCommand(const CXTPChartPoints& points, const CXTPChartColor& color)
{
	if (m_nDashStyle == xtpChartDashStyleEmpty)
		return NULL;

	if (m_nDashStyle == xtpChartDashStyleSolid)
		return new CXTPChartSolidSplineDeviceCommand(points, color, m_nThickness);

	return new CXTPChartDashedSplineDeviceCommand(points, color, m_nThickness, m_nDashStyle);
}

void CXTPChartLineStyle::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Int(pPX, _T("Thickness"), m_nThickness, 1);
	PX_Enum(pPX, _T("DashStyle"), m_nDashStyle, xtpChartDashStyleSolid);
}


#ifdef _XTP_ACTIVEX

BEGIN_DISPATCH_MAP(CXTPChartLineStyle, CCmdTarget)
	DISP_PROPERTY_EX_ID(CXTPChartLineStyle, "Thickness", 1, GetThickness, SetThickness, VT_I4)
	DISP_PROPERTY_EX_ID(CXTPChartLineStyle, "DashStyle", 2, GetDashStyle, SetDashStyle, VT_I4)
END_DISPATCH_MAP()


// {56DBCC77-27BF-4cb1-9ABF-4558D9835223}
static const GUID IID_IChartLineStyle =
{ 0x56dbcc77, 0x27bf, 0x4cb1, { 0x9a, 0xbf, 0x45, 0x58, 0xd9, 0x83, 0x52, 0x23 } };

BEGIN_INTERFACE_MAP(CXTPChartLineStyle, CCmdTarget)
	INTERFACE_PART(CXTPChartLineStyle, IID_IChartLineStyle, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPChartLineStyle, IID_IChartLineStyle)


#endif
