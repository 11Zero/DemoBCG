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
// BCGPColorIndicatorImpl.h: interface for the CBCGPColorIndicatorImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPCOLORINDICATORIMPL_H__A5406E02_1202_4554_B5D7_B7588C5C61B7__INCLUDED_)
#define AFX_BCGPCOLORINDICATORIMPL_H__A5406E02_1202_4554_B5D7_B7588C5C61B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPStaticGaugeImpl.h"

struct BCGCBPRODLLEXPORT CBCGPColorIndicatorColors
{
	CBCGPColorIndicatorColors();
	CBCGPColorIndicatorColors(const CBCGPColorIndicatorColors& src)
	{
		CopyFrom(src);
	}

	void CopyFrom(const CBCGPColorIndicatorColors& src)
	{
		m_brFill = src.m_brFill;
		m_brOutline = src.m_brOutline;
	}

	CBCGPBrush	m_brFill;
	CBCGPBrush	m_brOutline;
};

class BCGCBPRODLLEXPORT CBCGPColorIndicatorImpl : public CBCGPStaticGaugeImpl  
{
	DECLARE_DYNCREATE(CBCGPColorIndicatorImpl)

public:
	CBCGPColorIndicatorImpl(CBCGPVisualContainer* pContainer = NULL);
	
	CBCGPColorIndicatorImpl(const CBCGPColor& color,
							const CBCGPColor& clrGradient = CBCGPColor(), 
							const CBCGPColor& clrBorder = CBCGPColor(),
							CBCGPVisualContainer* pContainer = NULL);

	CBCGPColorIndicatorImpl(const CBCGPBrush& brFill,
							const CBCGPBrush& brBorder = CBCGPBrush(),
							CBCGPVisualContainer* pContainer = NULL);

	virtual ~CBCGPColorIndicatorImpl();

	enum BCGPColorIndicatorStyle
	{
		BCGP_COLOR_INDICATOR_STYLE_FIRST = 0,
		BCGP_COLOR_INDICATOR_ELLIPSE     = BCGP_COLOR_INDICATOR_STYLE_FIRST,
		BCGP_COLOR_INDICATOR_RECTANGLE   = 1,
		BCGP_COLOR_INDICATOR_STYLE_LAST  = BCGP_COLOR_INDICATOR_RECTANGLE
	};

// Overrides:
public:
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	virtual CBCGPSize GetDefaultSize(CBCGPGraphicsManager* pGM, const CBCGPBaseVisualObject* pParentGauge);
	virtual void CopyFrom(const CBCGPBaseVisualObject& src);

protected:
	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole)
	{
		HRESULT hr = CBCGPStaticGaugeImpl::get_accRole(varChild, pvarRole);
		if (SUCCEEDED(hr))
		{
			pvarRole->lVal = ROLE_SYSTEM_INDICATOR;
		}
		
		return hr;
	}

	virtual HRESULT get_accValue(VARIANT varChild, BSTR *pszValue)
	{
		if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
		{
			CString strValue = m_Colors.m_brFill.GetColor().ToString();
			*pszValue = strValue.AllocSysString();
			return S_OK;
		}
		
		return S_FALSE;
	}

// Attributes:
public:
	BCGPColorIndicatorStyle GetStyle() const
	{
		return m_Style;
	}

	void SetStyle(BCGPColorIndicatorStyle style)
	{
		m_Style = style;
	}

	void SetColor(const CBCGPColor& color, 
		const CBCGPColor& clrGradient = CBCGPColor(), 
		const CBCGPColor& clrBorder = CBCGPColor(), 
		BOOL bRedraw = FALSE);

	void SetColor(const CBCGPColor& color, BOOL bRedraw = FALSE)
	{
		SetColor(color, CBCGPColor(), CBCGPColor(), bRedraw);
	}

	void SetColor(const CBCGPBrush& brFill, const CBCGPBrush& brBorder = CBCGPBrush(), BOOL bRedraw = FALSE);

	const CBCGPColorIndicatorColors& GetColors() const
	{
		return m_Colors;
	}

	void SetColors(const CBCGPColorIndicatorColors& colors)
	{
		m_Colors = colors;
		CreateResources();
	}

	BOOL IsStretched () const
	{
		return m_bStretched;
	}

	void SetStretched (BOOL bStretched)
	{
		m_bStretched = bStretched;
	}

	virtual void SetOpacity(double opacity, BOOL bRedraw = TRUE);

protected:
	void CreateResources();

	CBCGPColorIndicatorColors	m_Colors;
	CBCGPColorIndicatorColors	m_LightColors;
	BCGPColorIndicatorStyle		m_Style;
	BOOL						m_bStretched;
};

#endif // !defined(AFX_BCGPCOLORINDICATORIMPL_H__A5406E02_1202_4554_B5D7_B7588C5C61B7__INCLUDED_)
