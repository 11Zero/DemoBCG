// XTPChartFillStyle.cpp
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

#include "../Drawing/XTPChartRectangleDeviceCommand.h"

#include "XTPChartFillStyle.h"
#include "../XTPChartElement.h"

LPCTSTR lpszGradientDirection[] =
{
	_T("TopToBottom"),
	_T("BottomToTop"),
	_T("LeftToRight"),
	_T("RightToLeft"),
	_T("TopLeftToBottomRight"),
	_T("BottomRightToTopLeft"),
	_T("TopRightToBottomLeft"),
	_T("BottomLeftToTopRight"),
	_T("ToCenterHorizontal"),
	_T("FromCenterHorizontal"),
	_T("ToCenterVertical"),
	_T("FromCenterVertical")
};




CXTPChartFillStyle::CXTPChartFillStyle(CXTPChartElement* pOwner)
{
	m_pOwner = pOwner;

	m_nFillMode = xtpChartFillSolid;

	m_nHatchStyle = xtpChartHatchStyleSolidDiamond;

	m_nGradientDirection = xtpChartGradientLeftToRight;

#ifdef _XTP_ACTIVEX
	EnableAutomation();
	EnableTypeLib();
#endif
}

CXTPChartDeviceCommand* CXTPChartFillStyle::CreateDeviceCommand(const CXTPChartRectF& bounds, const CXTPChartColor& color, const CXTPChartColor& color2)
{
	if (m_nFillMode == xtpChartFillSolid)
	{
		return new CXTPChartSolidRectangleDeviceCommand(bounds, color);
	}

	if (m_nFillMode == xtpChartFillHatch)
	{
		return new CXTPChartHatchRectangleDeviceCommand(bounds, color, color2, m_nHatchStyle);
	}

	if (m_nFillMode == xtpChartFillGradient)
	{
		switch(m_nGradientDirection)
		{
			case xtpChartGradientTopToBottom:
				return new CXTPChartGradientRectangleDeviceCommand(bounds, color, color2, xtpChartLinearGradientModeVertical);

			case xtpChartGradientBottomToTop:
				return new CXTPChartGradientRectangleDeviceCommand(bounds, color2, color, xtpChartLinearGradientModeVertical);

			case xtpChartGradientLeftToRight:
				return new CXTPChartGradientRectangleDeviceCommand(bounds, color, color2, xtpChartLinearGradientModeHorizontal);

			case xtpChartGradientRightToLeft:
				return new CXTPChartGradientRectangleDeviceCommand(bounds, color2, color, xtpChartLinearGradientModeHorizontal);

			case xtpChartGradientTopLeftToBottomRight:
				return new CXTPChartGradientRectangleDeviceCommand(bounds, color, color2, xtpChartLinearGradientModeForwardDiagonal);

			case xtpChartGradientBottomRightToTopLeft:
				return new CXTPChartGradientRectangleDeviceCommand(bounds, color2, color, xtpChartLinearGradientModeForwardDiagonal);

			case xtpChartGradientTopRightToBottomLeft:
				return new CXTPChartGradientRectangleDeviceCommand(bounds, color2, color, xtpChartLinearGradientModeBackwardDiagonal);

			case xtpChartGradientBottomLeftToTopRight:
				return new CXTPChartGradientRectangleDeviceCommand(bounds, color, color2, xtpChartLinearGradientModeBackwardDiagonal);

			case xtpChartGradientToCenterHorizontal:
				return new CXTPChartGradientRectangleDeviceCommand(bounds, color, color2, xtpChartLinearGradientModeCenterHorizontal);

			case xtpChartGradientFromCenterHorizontal:
				return new CXTPChartGradientRectangleDeviceCommand(bounds, color2, color, xtpChartLinearGradientModeCenterHorizontal);

			case xtpChartGradientToCenterVertical:
				return new CXTPChartGradientRectangleDeviceCommand(bounds, color, color2, xtpChartLinearGradientModeCenterVertical);

			case xtpChartGradientFromCenterVertical:
				return new CXTPChartGradientRectangleDeviceCommand(bounds, color2, color, xtpChartLinearGradientModeCenterVertical);
		}
	}


	return NULL;
}


CXTPChartDeviceCommand* CXTPChartFillStyle::CreateCircleDeviceCommand(const CXTPChartPointF& center, double radius, const CXTPChartColor& color, const CXTPChartColor& color2)
{
	if (m_nFillMode == xtpChartFillSolid)
	{
		return new CXTPChartSolidCircleDeviceCommand(center, radius, color);
	}

	if (m_nFillMode == xtpChartFillHatch)
	{
		return new CXTPChartHatchCircleDeviceCommand(center, radius, color, color2, m_nHatchStyle);
	}

	if (m_nFillMode == xtpChartFillGradient)
	{
		switch(m_nGradientDirection)
		{
		case xtpChartGradientTopToBottom:
			return new CXTPChartGradientCircleDeviceCommand(center, radius, color, color2, xtpChartLinearGradientModeVertical);

		case xtpChartGradientBottomToTop:
			return new CXTPChartGradientCircleDeviceCommand(center, radius, color2, color, xtpChartLinearGradientModeVertical);

		case xtpChartGradientLeftToRight:
			return new CXTPChartGradientCircleDeviceCommand(center, radius, color, color2, xtpChartLinearGradientModeHorizontal);

		case xtpChartGradientRightToLeft:
			return new CXTPChartGradientCircleDeviceCommand(center, radius, color2, color, xtpChartLinearGradientModeHorizontal);

		case xtpChartGradientTopLeftToBottomRight:
			return new CXTPChartGradientCircleDeviceCommand(center, radius, color, color2, xtpChartLinearGradientModeForwardDiagonal);

		case xtpChartGradientBottomRightToTopLeft:
			return new CXTPChartGradientCircleDeviceCommand(center, radius, color2, color, xtpChartLinearGradientModeForwardDiagonal);

		case xtpChartGradientTopRightToBottomLeft:
			return new CXTPChartGradientCircleDeviceCommand(center, radius, color2, color, xtpChartLinearGradientModeBackwardDiagonal);

		case xtpChartGradientBottomLeftToTopRight:
			return new CXTPChartGradientCircleDeviceCommand(center, radius, color, color2, xtpChartLinearGradientModeBackwardDiagonal);

		case xtpChartGradientToCenterHorizontal:
			return new CXTPChartGradientCircleDeviceCommand(center, radius, color, color2, xtpChartLinearGradientModeCenterHorizontal);

		case xtpChartGradientFromCenterHorizontal:
			return new CXTPChartGradientCircleDeviceCommand(center, radius, color2, color, xtpChartLinearGradientModeCenterHorizontal);

		case xtpChartGradientToCenterVertical:
			return new CXTPChartGradientCircleDeviceCommand(center, radius, color, color2, xtpChartLinearGradientModeCenterVertical);

		case xtpChartGradientFromCenterVertical:
			return new CXTPChartGradientCircleDeviceCommand(center, radius, color2, color, xtpChartLinearGradientModeCenterVertical);
		}
	}


	return NULL;
}



CXTPChartDeviceCommand* CXTPChartFillStyle::CreateSplineDeviceCommand(const CXTPChartPoints& arrPoints, const CXTPChartColor& color, const CXTPChartColor& /*color2*/, BOOL bTwoSides)
{
	return new CXTPChartSolidSplinePolygonDeviceCommand(arrPoints, color, bTwoSides);

}

CXTPChartDeviceCommand* CXTPChartFillStyle::CreateDeviceCommand(const CXTPChartPoints& arrPoints, const CXTPChartColor& color, const CXTPChartColor& color2)
{
	if (m_nFillMode == xtpChartFillSolid)
	{
		return new CXTPChartSolidPolygonDeviceCommand(arrPoints, color);
	}

	if (m_nFillMode == xtpChartFillHatch)
	{
		return new CXTPChartHatchPolygonDeviceCommand(arrPoints, color, color2, m_nHatchStyle);
	}

	if (m_nFillMode == xtpChartFillGradient)
	{
		switch(m_nGradientDirection)
		{
		case xtpChartGradientTopToBottom:
			return new CXTPChartGradientPolygonDeviceCommand(arrPoints, color, color2, xtpChartLinearGradientModeVertical);

		case xtpChartGradientBottomToTop:
			return new CXTPChartGradientPolygonDeviceCommand(arrPoints, color2, color, xtpChartLinearGradientModeVertical);

		case xtpChartGradientLeftToRight:
			return new CXTPChartGradientPolygonDeviceCommand(arrPoints, color, color2, xtpChartLinearGradientModeHorizontal);

		case xtpChartGradientRightToLeft:
			return new CXTPChartGradientPolygonDeviceCommand(arrPoints, color2, color, xtpChartLinearGradientModeHorizontal);

		case xtpChartGradientTopLeftToBottomRight:
			return new CXTPChartGradientPolygonDeviceCommand(arrPoints, color, color2, xtpChartLinearGradientModeForwardDiagonal);

		case xtpChartGradientBottomRightToTopLeft:
			return new CXTPChartGradientPolygonDeviceCommand(arrPoints, color2, color, xtpChartLinearGradientModeForwardDiagonal);

		case xtpChartGradientTopRightToBottomLeft:
			return new CXTPChartGradientPolygonDeviceCommand(arrPoints, color2, color, xtpChartLinearGradientModeBackwardDiagonal);

		case xtpChartGradientBottomLeftToTopRight:
			return new CXTPChartGradientPolygonDeviceCommand(arrPoints, color, color2, xtpChartLinearGradientModeBackwardDiagonal);

		case xtpChartGradientToCenterHorizontal:
			return new CXTPChartGradientPolygonDeviceCommand(arrPoints, color, color2, xtpChartLinearGradientModeCenterHorizontal);

		case xtpChartGradientFromCenterHorizontal:
			return new CXTPChartGradientPolygonDeviceCommand(arrPoints, color2, color, xtpChartLinearGradientModeCenterHorizontal);

		case xtpChartGradientToCenterVertical:
			return new CXTPChartGradientPolygonDeviceCommand(arrPoints, color, color2, xtpChartLinearGradientModeCenterVertical);

		case xtpChartGradientFromCenterVertical:
			return new CXTPChartGradientPolygonDeviceCommand(arrPoints, color2, color, xtpChartLinearGradientModeCenterVertical);
		}
	}
	return NULL;
}

void CXTPChartFillStyle::DoPropExchange(CXTPPropExchange* pPX)
{
	if (pPX->IsLoading())
	{
		CString strFillMode, strGradientDirection;

		PX_String(pPX, _T("FillMode"), strFillMode);

		if (strFillMode.CompareNoCase(_T("Gradient")) == 0)
		{
			m_nFillMode = xtpChartFillGradient;
			PX_String(pPX, _T("GradientDirection"), strGradientDirection);

			for (int i = 0; i < _countof(lpszGradientDirection); i++)
			{
				if (strGradientDirection.CompareNoCase(lpszGradientDirection[i]) == 0)
				{
					m_nGradientDirection = (XTPChartGradientDirection)i;
					break;
				}
			}

		}
		else if (strFillMode.CompareNoCase(_T("Empty")) == 0)
		{
			m_nFillMode = xtpChartFillEmpty;
		}
		else if (strFillMode.CompareNoCase(_T("Hatch")) == 0)
		{
			m_nFillMode = xtpChartFillHatch;

			PX_Enum(pPX, _T("HatchStyle"), m_nHatchStyle, xtpChartHatchStyleSolidDiamond);
		}
		else
		{
			m_nFillMode = xtpChartFillSolid;
		}
	}
	else
	{
		CString strFillMode = m_nFillMode == xtpChartFillEmpty ? _T("Empty") : m_nFillMode == xtpChartFillGradient ? _T("Gradient") :
			m_nFillMode == xtpChartFillHatch ? _T("Hatch") : _T("");

		PX_String(pPX, _T("FillMode"), strFillMode, _T(""));

		if (m_nFillMode == xtpChartFillGradient)
		{
			CString  strGradientDirection = lpszGradientDirection[m_nGradientDirection];

			PX_String(pPX, _T("GradientDirection"), strGradientDirection);
		}

		if (m_nFillMode == xtpChartFillHatch)
		{
			PX_Enum(pPX, _T("HatchStyle"), m_nHatchStyle, xtpChartHatchStyleHorizontal);
		}
	}

}


void CXTPChartFillStyle::SetFillMode(XTPChartFillMode nFillMode)
{
	m_nFillMode = nFillMode;
	m_pOwner->OnChartChanged();
}

void CXTPChartFillStyle::SetGradientDirection(XTPChartGradientDirection nDirection)
{
	m_nGradientDirection = nDirection;
	m_pOwner->OnChartChanged();
}

void CXTPChartFillStyle::SetHatchStyle(XTPChartHatchStyle nStyle)
{
	m_nHatchStyle = nStyle;
	m_pOwner->OnChartChanged();
}

CXTPChartFillStyle* CXTPChartFillStyle::CreateRotatedStyle()
{
	if (m_nFillMode != xtpChartFillGradient)
	{
		CXTPChartFillStyle* pStyle = this;
		pStyle->InternalAddRef();
		return pStyle;
	}

	CXTPChartFillStyle* pStyle = new CXTPChartFillStyle(m_pOwner);
	pStyle->m_nFillMode = xtpChartFillGradient;

	switch(m_nGradientDirection)
	{
	case xtpChartGradientTopToBottom: pStyle->m_nGradientDirection = xtpChartGradientRightToLeft; break;
	case xtpChartGradientBottomToTop: pStyle->m_nGradientDirection = xtpChartGradientLeftToRight; break;
	case xtpChartGradientLeftToRight: pStyle->m_nGradientDirection = xtpChartGradientTopToBottom; break;
	case xtpChartGradientRightToLeft: pStyle->m_nGradientDirection = xtpChartGradientBottomToTop; break;
	case xtpChartGradientToCenterHorizontal: pStyle->m_nGradientDirection = xtpChartGradientToCenterVertical; break;
	case xtpChartGradientFromCenterHorizontal: pStyle->m_nGradientDirection = xtpChartGradientFromCenterVertical; break;
	case xtpChartGradientToCenterVertical: pStyle->m_nGradientDirection = xtpChartGradientToCenterHorizontal; break;
	case xtpChartGradientFromCenterVertical: pStyle->m_nGradientDirection = xtpChartGradientFromCenterHorizontal; break;
	default:
		pStyle->m_nGradientDirection = m_nGradientDirection;
	}

	return pStyle;
}

#ifdef _XTP_ACTIVEX

BEGIN_DISPATCH_MAP(CXTPChartFillStyle, CCmdTarget)
	DISP_PROPERTY_EX_ID(CXTPChartFillStyle, "FillMode", 1, OleGetFillMode, SetFillMode, VT_I4)
	DISP_PROPERTY_EX_ID(CXTPChartFillStyle, "HatchStyle", 2, OleGetHatchStyle, SetHatchStyle, VT_I4)
	DISP_PROPERTY_EX_ID(CXTPChartFillStyle, "GradientDirection", 3, OleGetGradientDirection, SetGradientDirection, VT_I4)
END_DISPATCH_MAP()


// {76DBCC77-27BF-4cb1-9ABF-4558D9835223}
static const GUID IID_IChartFillStyle =
{ 0x76dbcc77, 0x27bf, 0x4cb1, { 0x9a, 0xbf, 0x45, 0x58, 0xd9, 0x83, 0x52, 0x23 } };

BEGIN_INTERFACE_MAP(CXTPChartFillStyle, CCmdTarget)
	INTERFACE_PART(CXTPChartFillStyle, IID_IChartFillStyle, Dispatch)
END_INTERFACE_MAP()

IMPLEMENT_OLETYPELIB_EX(CXTPChartFillStyle, IID_IChartFillStyle)

int CXTPChartFillStyle::OleGetFillMode()
{
	return GetFillMode();
}

int CXTPChartFillStyle::OleGetHatchStyle()
{
	return GetHatchStyle();
}

int CXTPChartFillStyle::OleGetGradientDirection()
{
	return GetGradientDirection();
}

#endif
