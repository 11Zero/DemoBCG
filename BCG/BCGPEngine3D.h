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
// BCGPEngine3D.h: interface for the CBCGPEngine3D class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPENGINE3D_H__67A80CEC_E3D1_4EB5_95A6_087D7F7CFBA0__INCLUDED_)
#define AFX_BCGPENGINE3D_H__67A80CEC_E3D1_4EB5_95A6_087D7F7CFBA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPGraphicsManager.h"

class BCGCBPRODLLEXPORT CBCGPEngine3D  : public CObject
{
	DECLARE_DYNAMIC(CBCGPEngine3D);
public:
	enum BCGP_3D_RENDERING_TYPE
	{
		BCGP_3D_RENDERING_TYPE_FIRST = 0,
		BCGP_RT_SOFTWARE             = BCGP_3D_RENDERING_TYPE_FIRST,
		BCGP_RT_OPENGL               = 1,
		BCGP_3D_RENDERING_TYPE_LAST  = BCGP_RT_OPENGL
	};

public:
	CBCGPEngine3D();
	virtual ~CBCGPEngine3D();

	virtual void ShutDown()
	{
		m_bInitialized = FALSE;
	}

	static CBCGPEngine3D* CreateInstance(BCGP_3D_RENDERING_TYPE rt, BOOL bRenderToWindow = FALSE);

	virtual BCGP_3D_RENDERING_TYPE GetRenderingType() const {return CBCGPEngine3D::BCGP_RT_SOFTWARE;}
	virtual BOOL IsSoftwareRendering() const {return TRUE;}
	virtual BOOL Initialize(BOOL bRenderToWindow);

	virtual void SetRenderToWindow(BOOL bSet) 
	{
		UNREFERENCED_PARAMETER(bSet);
	}
	virtual BOOL IsRenderToWindow() const {return FALSE;}

	CBCGPGraphicsManager* GetDefaultManager() const {return m_pDefaultGM;}

	virtual void SetSceneRectAndDepth(const CBCGPRect& rect, double dblZMin, double dblZMax)
	{
		UNREFERENCED_PARAMETER(dblZMin);
		UNREFERENCED_PARAMETER(dblZMax);

		m_rectScene = rect;
	}

	virtual void SetClearColor(const CBCGPColor& clrClear)
	{
		m_clrClear = clrClear;
	}

	const CBCGPRect& GetSceneRect() const {return m_rectScene;}

	virtual void BeginDraw(CBCGPGraphicsManager* pGMTarget = NULL) 
	{
		m_pDefaultGM = pGMTarget;
	}

	virtual void EndDraw(const CBCGPRect& rectTarget, CBCGPGraphicsManager* pGMTarget = NULL) 
	{
		UNREFERENCED_PARAMETER(pGMTarget);
		UNREFERENCED_PARAMETER(rectTarget);

		m_pDefaultGM = NULL;
	}

	virtual BOOL EnableAntialiasing(BOOL bEnable = TRUE);
	virtual void SetPolygonNormal(double nx, double ny, double nz);
	virtual void DrawLine(const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo, const CBCGPBrush& brush, 
		double lineWidth = 1., const CBCGPStrokeStyle* pStrokeStyle = NULL);
	virtual void FillGeometry(const CBCGPGeometry& geometry, const CBCGPBrush& brush);
	virtual void FillPolygon(const CBCGPPointsArray& arPoints, const CBCGPBrush& brush);
	virtual void DrawPolygon(const CBCGPPointsArray& arPoints, const CBCGPBrush& brush, double dblLineWidth = 1.);
	virtual void DrawSide(const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3, const CBCGPPoint& pt4,
		const CBCGPBrush& brFill, const CBCGPBrush& brLine, double dblLineWidth = 1.,
		BOOL bFill = TRUE, BOOL bDrawLine = TRUE);

public:
	BOOL		m_bForceDisableDepthTest;
	BOOL		m_bForceEnableDepthTest;
	BOOL		m_bForceNoTransparency;

protected:
	BOOL		m_bInitialized;
	CBCGPRect	m_rectScene;
	CBCGPColor	m_clrClear;

	

	CBCGPGraphicsManager* m_pDefaultGM;
};


#endif // !defined(AFX_BCGPENGINE3D_H__67A80CEC_E3D1_4EB5_95A6_087D7F7CFBA0__INCLUDED_)
