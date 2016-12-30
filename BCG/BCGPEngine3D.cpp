//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPEngine3D.cpp: implementation of the CBCGPEngine3D class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPEngine3D.h"
#include "bcgpmath.h"

#include "BCGPEngine3DOpenGL.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CBCGPEngine3D, CObject)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPEngine3D::CBCGPEngine3D()
{
	m_bInitialized = FALSE;
	m_pDefaultGM = NULL;
	m_bForceDisableDepthTest = FALSE;
	m_bForceEnableDepthTest = FALSE;
	m_bForceNoTransparency = FALSE;
}
//****************************************************************************************
CBCGPEngine3D::~CBCGPEngine3D()
{
	ShutDown();
}
//****************************************************************************************
CBCGPEngine3D* CBCGPEngine3D::CreateInstance(BCGP_3D_RENDERING_TYPE rt, BOOL bRenderToWindow)
{
#ifdef BCGP_EXCLUDE_OPENGL
	rt = CBCGPEngine3D::BCGP_RT_SOFTWARE;
#endif // BCGP_EXCLUDE_OPENGL

	CBCGPEngine3D* pEngine = NULL;

	switch (rt)
	{
	case CBCGPEngine3D::BCGP_RT_OPENGL:
		pEngine = new CBCGPEngine3DOpenGL();
		break;

	case CBCGPEngine3D::BCGP_RT_SOFTWARE:
	default:
		pEngine = new CBCGPEngine3D();
		break;
	}

	ASSERT_VALID(pEngine);

	if (!pEngine->Initialize(bRenderToWindow))
	{
		delete pEngine;

		// should work always 
		pEngine = new CBCGPEngine3D();
		pEngine->Initialize(bRenderToWindow);
	}

	return pEngine;
}
//****************************************************************************************
BOOL CBCGPEngine3D::Initialize(BOOL /*bRenderToWindow*/)
{
	m_bInitialized = TRUE;
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPEngine3D::EnableAntialiasing(BOOL bEnable)
{
	if (m_pDefaultGM != NULL)
	{
		BOOL bPrevValue = m_pDefaultGM->IsAntialiasingEnabled();
		m_pDefaultGM->EnableAntialiasing(bEnable);
		return bPrevValue;
	}

	return FALSE;
}
//****************************************************************************************
void CBCGPEngine3D::SetPolygonNormal(double /*nx*/, double /*ny*/, double /*nz*/)
{

}
//****************************************************************************************
void CBCGPEngine3D::DrawLine(const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo, const CBCGPBrush& brush, 
		double lineWidth, const CBCGPStrokeStyle* pStrokeStyle)
{
	if (m_pDefaultGM != NULL)
	{
		ASSERT_VALID(m_pDefaultGM);
		m_pDefaultGM->DrawLine(ptFrom, ptTo, brush, lineWidth, pStrokeStyle);
	}
}
//****************************************************************************************
void CBCGPEngine3D::DrawPolygon(const CBCGPPointsArray& arPoints, const CBCGPBrush& brush, double dblLineWidth)
{
	int nNumPoints = (int)arPoints.GetSize();

	if (m_pDefaultGM == NULL || nNumPoints == 0)
	{
		return;
	}

	m_pDefaultGM->DrawLines(arPoints, brush, dblLineWidth);
}
//****************************************************************************************
void CBCGPEngine3D::FillPolygon(const CBCGPPointsArray& arPoints, const CBCGPBrush& brush)
{
	int nNumPoints = (int)arPoints.GetSize();

	if (m_pDefaultGM == NULL || nNumPoints == 0)
	{
		return;
	}

	ASSERT_VALID(m_pDefaultGM);

	if (m_pDefaultGM->GetType() == CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_GDI)
	{
		CBCGPPolygonGeometry g(arPoints);
		g.SetTemporary();

		m_pDefaultGM->FillGeometry(g, brush);
	}
	else
	{
		LPPOINT arPointsGDI = new POINT[nNumPoints];

		for (int i = 0; i < nNumPoints; i++)
		{
			arPointsGDI[i] = CPoint(bcg_round(arPoints[i].x), bcg_round(arPoints[i].y));
		}

		HRGN hrgn = ::CreatePolygonRgn(arPointsGDI, nNumPoints, ALTERNATE);
		UINT nBytesCount = ::GetRegionData(hrgn, sizeof(RGNDATA), NULL);

		if (nBytesCount == 0)
		{
			delete [] arPointsGDI;
			return;
		}

		LPBYTE lpRgnData = new BYTE[nBytesCount];
		ZeroMemory( lpRgnData, nBytesCount);
		
		if (::GetRegionData(hrgn, nBytesCount, (LPRGNDATA)lpRgnData) != nBytesCount)
		{
			delete [] arPointsGDI;
			delete [] lpRgnData;

			return;
		}

		LPRGNDATA pData = (LPRGNDATA)lpRgnData;
		LPRECT points = (LPRECT)pData->Buffer;

		for (DWORD k = 0; k < pData->rdh.nCount; k++)
		{
			m_pDefaultGM->FillRectangle(CBCGPRect(points[k]), brush);
		}

		delete [] arPointsGDI;
		delete [] lpRgnData;

		DeleteObject(hrgn);
	}
}
//****************************************************************************************
void CBCGPEngine3D::FillGeometry(const CBCGPGeometry& geometry, const CBCGPBrush& brush)
{
	if (m_pDefaultGM != NULL)
	{
		ASSERT_VALID(m_pDefaultGM);
		m_pDefaultGM->FillGeometry(geometry, brush);
	}
}
//****************************************************************************************
void CBCGPEngine3D::DrawSide(const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3, const CBCGPPoint& pt4,
		const CBCGPBrush& brFill, const CBCGPBrush& brLine, double dblLineWidth,
		BOOL /*bFill*/, BOOL /*bDrawLine*/)
{
	if (m_pDefaultGM != NULL)
	{
		ASSERT_VALID(m_pDefaultGM);
		m_pDefaultGM->DrawSide(pt1, pt2, pt3, pt4, brFill, brLine, dblLineWidth);
	}
}
