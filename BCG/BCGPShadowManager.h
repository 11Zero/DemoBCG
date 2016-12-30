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
// BCGPShadowManager.h: interface for the CBCGPShadowManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPSHADOWMANAGER_H__F172C157_4F31_4EE5_9419_E7ED4D5A0855__INCLUDED_)
#define AFX_BCGPSHADOWMANAGER_H__F172C157_4F31_4EE5_9419_E7ED4D5A0855__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPControlRenderer.h"

class CBCGPShadowSideWnd;

class CBCGPShadowManager  
{
	friend class CBCGPShadowSideWnd;

public:
	CBCGPShadowManager(CWnd* pOwner, BOOL bInteraction = TRUE);
	virtual ~CBCGPShadowManager();

	BOOL Create(const CSize& size = CSize(0, 0), int nMinBrightness = -1, 
		int nMaxBrightness = -1, double dblSmooth = 0.0, double dblDarkRatio = 0.0);
	void SetVisible(BOOL bVisible);

	void Repos();
	void UpdateTransparency(BYTE nTransparency);
	void UpdateBaseColor(COLORREF clr = (COLORREF)-1);

	UINT HitTest(CPoint point);

protected:
	CSize GetBorderSize() const;
	void Show(BOOL bShow);

protected:
	CWnd*	m_pOwner;
	BYTE	m_nTransparency;
	BOOL	m_bInteracion;
	CBCGPShadowRenderer m_Shadow;
	BOOL	m_bVisible;
	CSize	m_Size;
	int		m_nMinBrightness;
	int		m_nMaxBrightness;
	double	m_dblSmooth;
	double	m_dblDarkRatio;

	CBCGPShadowSideWnd* m_arWnd[4];
};

#endif // !defined(AFX_BCGPSHADOWMANAGER_H__F172C157_4F31_4EE5_9419_E7ED4D5A0855__INCLUDED_)
