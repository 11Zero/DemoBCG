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
// BCGPImageGaugeImpl.h: interface for the CBCGPImageGaugeImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPIMAGEGAUGEIMPL_H__9F10A362_8B4D_499E_BC20_6FDECBC95676__INCLUDED_)
#define AFX_BCGPIMAGEGAUGEIMPL_H__9F10A362_8B4D_499E_BC20_6FDECBC95676__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPStaticGaugeImpl.h"
#include "BCGPGraphicsManager.h"
#include "BCGPVisualCtrl.h"

class BCGCBPRODLLEXPORT CBCGPImageGaugeImpl : public CBCGPStaticGaugeImpl  
{
	DECLARE_DYNCREATE(CBCGPImageGaugeImpl)
		
	friend class CBCGPVisualCollector;
	friend class CBCGPVisualConstructor;

public:
	enum VerticalAlign
	{
		VA_First   = 0,
		VA_Top     = VA_First,
		VA_Center  = 1,
		VA_Bottom  = 2,
		VA_Stretch = 3,
		VA_Last    = VA_Stretch
	};

	enum HorizontalAlign
	{
		HA_First   = 0,
		HA_Left    = HA_First,
		HA_Center  = 1,
		HA_Right   = 2,
		HA_Stretch = 3,
		HA_Last    = HA_Stretch
	};

public:
	CBCGPImageGaugeImpl(UINT uiImageResID = 0, CBCGPVisualContainer* pContainer = NULL);
	virtual ~CBCGPImageGaugeImpl();

	virtual void CopyFrom(const CBCGPBaseVisualObject& src);

// Overrides:
public:
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	virtual CBCGPSize GetDefaultSize(CBCGPGraphicsManager* pGM, const CBCGPBaseVisualObject* pParentGauge);

protected:
	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole)
	{
		HRESULT hr = CBCGPStaticGaugeImpl::get_accRole(varChild, pvarRole);
		if (SUCCEEDED(hr))
		{
			pvarRole->lVal = ROLE_SYSTEM_GRAPHIC;
		}
		
		return hr;
	}

// Attributes:
public:
	void SetImage(UINT uiImageResID, BOOL bRedraw = FALSE);
	void SetImage (const CBCGPImage& image, BOOL bRedraw = FALSE);

 	void SetImageAlign (HorizontalAlign horzAlign, VerticalAlign vertAlign, BOOL bLockAspectRatio = TRUE, BOOL bRedraw = FALSE);
	
	HorizontalAlign GetHorizontalAlign() const
	{
		return m_HorizontalAlign;
	}

	VerticalAlign GetVerticalAlign() const
	{
		return m_VerticalAlign;
	}

	BOOL IsAspectRatioLocked() const
	{
		return m_bLockAspectRatio;
	}

protected:
	CBCGPImage		m_Image;
 	VerticalAlign	m_VerticalAlign;
 	HorizontalAlign m_HorizontalAlign;
	BOOL			m_bLockAspectRatio;	
};

class BCGCBPRODLLEXPORT CBCGPImageGaugeCtrl : public CBCGPVisualCtrl
{
	DECLARE_DYNAMIC(CBCGPImageGaugeCtrl)
		
public:
	CBCGPImageGaugeCtrl()
	{
		m_pImage = NULL;
		EnableTooltip();
	}
	
	virtual ~CBCGPImageGaugeCtrl()
	{
		if (m_pImage != NULL)
		{
			delete m_pImage;
		}
	}
	
	virtual CBCGPImageGaugeImpl* GetImageImpl()
	{
		if (m_pImage == NULL)
		{
			m_pImage = new CBCGPImageGaugeImpl();
		}
		
		return m_pImage;
	}
	
protected:
	virtual CBCGPBaseVisualObject* GetVisualObject()
	{
		return GetImageImpl();
	}
	
protected:
	CBCGPImageGaugeImpl* m_pImage;
};

#endif // !defined(AFX_BCGPIMAGEGAUGEIMPL_H__9F10A362_8B4D_499E_BC20_6FDECBC95676__INCLUDED_)
