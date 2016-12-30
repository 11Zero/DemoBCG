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
// BCGPSwitchImpl.h: interface for the CBCGPSwitchImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPSWITCHIMPL_H__6B7B4B68_1774_43DC_A61D_0602C62CC4B0__INCLUDED_)
#define AFX_BCGPSWITCHIMPL_H__6B7B4B68_1774_43DC_A61D_0602C62CC4B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPStaticGaugeImpl.h"
#include "BCGPVisualCtrl.h"

struct BCGCBPRODLLEXPORT CBCGPSwitchColors
{
	CBCGPSwitchColors();
	CBCGPSwitchColors(const CBCGPSwitchColors& src)
	{
		CopyFrom(src);
	}
	
	void CopyFrom(const CBCGPSwitchColors& src)
	{
		m_brOutline			= src.m_brOutline;
		m_brFill			= src.m_brFill;
		
		m_brOutlineThumb	= src.m_brOutlineThumb;
		m_brFillThumb		= src.m_brFillThumb;
		
		m_brFillOn			= src.m_brFillOn;
		m_brFillOff			= src.m_brFillOff;

		m_brLabelOn			= src.m_brLabelOn;
		m_brLabelOff		= src.m_brLabelOff;

		m_brFocus			= src.m_brFocus;
	}
	
	CBCGPBrush	m_brOutline;
	CBCGPBrush	m_brFill;

	CBCGPBrush	m_brOutlineThumb;
	CBCGPBrush	m_brFillThumb;

	CBCGPBrush	m_brFillOn;
	CBCGPBrush	m_brFillOff;

	CBCGPBrush	m_brLabelOn;
	CBCGPBrush	m_brLabelOff;

	CBCGPBrush	m_brFocus;
};

class BCGCBPRODLLEXPORT CBCGPSwitchImpl : public CBCGPStaticGaugeImpl
{
	DECLARE_DYNCREATE(CBCGPSwitchImpl)

public:
	enum BCGP_SWITCH_STYLE
	{
		BCGP_SWITCH_STYLE_FIRST                     = 0,
		BCGP_SWITCH_RECTANGLE                       = BCGP_SWITCH_STYLE_FIRST,
		BCGP_SWITCH_RECTANGLE_NARROW_GUTTER         = 1,
		BCGP_SWITCH_ROUNDED_RECTANGLE               = 2,
		BCGP_SWITCH_ROUNDED_RECTANGLE_NARROW_GUTTER = 3,
		BCGP_SWITCH_CIRCLE                          = 4,
		BCGP_SWITCH_CIRCLE_NARROW_GUTTER            = 5,
		BCGP_SWITCH_STYLE_LAST                      = BCGP_SWITCH_CIRCLE_NARROW_GUTTER
	};

public:
	CBCGPSwitchImpl(CBCGPVisualContainer* pContainer = NULL);
	virtual ~CBCGPSwitchImpl();

	virtual void CopyFrom(const CBCGPBaseVisualObject& src);

// Attributes:
public:
	BOOL IsOn() const 
	{ 
		return m_bValue; 
	}

	void SetOn(BOOL bSet = TRUE, BOOL bRedraw = FALSE)
	{
		m_bValue = bSet;
		SetDirty(TRUE, bRedraw);
	}

	// Color theme support:
	const CBCGPSwitchColors& GetColors() const
	{
		return m_Colors;
	}	

	void SetColors(const CBCGPSwitchColors& colors, BOOL bRedraw = FALSE)
	{
		m_Colors = colors;
		SetDirty(TRUE, bRedraw);
	}

	// Custom images:
	void SetGutterImage(const CBCGPImage& image, BOOL bOn, BOOL bIncludesThumb = FALSE, BOOL bRedraw = FALSE)
	{
		if (bOn)
		{
			m_imageGutterOn = image;
			m_bDontDrawThumbOn = bIncludesThumb;
		}
		else
		{
			m_imageGutterOff = image;
			m_bDontDrawThumbOff = bIncludesThumb;
		}

		SetDirty(TRUE, bRedraw);
	}

	const CBCGPImage& GetGutterImage(BOOL bOn) const
	{
		return bOn ? m_imageGutterOn : m_imageGutterOff;
	}

	void SetThumbImage(const CBCGPImage& image, BOOL bRedraw = FALSE)
	{
		m_imageThumb = image;
		SetDirty(TRUE, bRedraw);
	}
	
	const CBCGPImage& GetThumbImage() const
	{
		return m_imageThumb;
	}

	void EnableImageScaling(BOOL bEnable = TRUE)
	{
		m_bImageScaling = bEnable;
		SetDirty();
	}

	BOOL IsImageScalingEnabled() const
	{
		return m_bImageScaling;
	}

	// Style support:
	BCGP_SWITCH_STYLE GetStyle() const
	{
		return m_Style;
	}

	void SetStyle(BCGP_SWITCH_STYLE style, BOOL bRedraw = FALSE)
	{
		m_Style = style;
		SetDirty(TRUE, bRedraw);
	}

	// Text labels support:
	void EnableOnOffLabels(BOOL bEnable = TRUE, BOOL bRedraw = FALSE)
	{
		m_bDrawTextLabels = bEnable;
		SetDirty(TRUE, bRedraw);
	}

	BOOL IsDrawTextLabels() const
	{
		return m_bDrawTextLabels;
	}

	const CString& GetLabel(BOOL bOn) const
	{
		return bOn ? m_strLabelOn : m_strLabelOff;
	}

	void SetLabel(const CString& strLabel, BOOL bOn, BOOL bRedraw = FALSE)
	{
		if (bOn)
		{
			m_strLabelOn = strLabel;
		}
		else
		{
			m_strLabelOff = strLabel;
		}

		SetDirty(TRUE, bRedraw);
	}

	const CBCGPTextFormat& GetLabelTextFormat() const
	{
		return m_textFormat;
	}
	
	void SetLabelTextFormat(const CBCGPTextFormat& textFormat, BOOL bRedraw = FALSE)
	{
		m_textFormat = textFormat;
		SetDirty(TRUE, bRedraw);
	}

	// Focus rectangle (for CBCGPSwitchCtrl only!)
	void SetFocusRect(BOOL bSet = TRUE, BOOL bRedraw = FALSE)
	{
		m_bDrawFocus = bSet;
		SetDirty(TRUE, bRedraw);
	}
	
	BOOL IsFocusRect() const
	{
		return m_bDrawFocus;
	}

// Overrides:
protected:
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	virtual void FireClickEvent(const CBCGPPoint& pt)
	{
		if (m_bIsInteractiveMode)
		{
			m_bValue = !m_bValue;
			SetDirty(TRUE, TRUE);
		}

		CBCGPStaticGaugeImpl::FireClickEvent(pt);
	}

	virtual CBCGPSize GetDefaultSize(CBCGPGraphicsManager* /*pGM*/, const CBCGPBaseVisualObject* /*pParentGauge*/);

	virtual BOOL OnKeyboardDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld);

	virtual void OnDrawGutter(CBCGPGraphicsManager* pGM, const CBCGPRect& rectGutter, double dblPad, double dblThumbsSize);
	virtual void OnDrawLabel(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLabel);
	virtual void OnDrawThumb(CBCGPGraphicsManager* pGM, const CBCGPRect& rectThumb, double dblRoundedRectCornerRadius);

	void DoDrawImage(CBCGPGraphicsManager* pGM, const CBCGPImage& image, const CBCGPRect& rect);

	// MSAA support:	
public:
	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName);
	virtual HRESULT get_accValue(VARIANT varChild, BSTR *pszValue);
	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole);
	virtual HRESULT get_accState(VARIANT varChild, VARIANT *pvarState);
	virtual HRESULT get_accDefaultAction(VARIANT varChild, BSTR *pszDefaultAction);
	
	virtual HRESULT accDoDefaultAction(VARIANT varChild);

protected:
	CBCGPSwitchColors	m_Colors;
	BOOL				m_bValue;
	BCGP_SWITCH_STYLE	m_Style;
	BOOL				m_bDrawTextLabels;
	BOOL				m_bDrawFocus;
	CString				m_strLabelOn;
	CString				m_strLabelOff;
	CBCGPTextFormat		m_textFormat;
	CBCGPStrokeStyle	m_strokeFocus;
	BOOL				m_bImageScaling;
	CBCGPImage			m_imageGutterOn;
	CBCGPImage			m_imageGutterOff;
	BOOL				m_bDontDrawThumbOn;
	BOOL				m_bDontDrawThumbOff;
	CBCGPImage			m_imageThumb;
};

class BCGCBPRODLLEXPORT CBCGPSwitchCtrl : public CBCGPVisualCtrl
{
	DECLARE_DYNAMIC(CBCGPSwitchCtrl)
		
public:
	CBCGPSwitchCtrl()
	{
		m_pSwitch = NULL;
		m_nDlgCode = DLGC_WANTARROWS | DLGC_WANTCHARS;

		EnableTooltip();
	}
	
	virtual ~CBCGPSwitchCtrl()
	{
		if (m_pSwitch != NULL)
		{
			delete m_pSwitch;
		}
	}
	
	virtual CBCGPSwitchImpl* GetSwitch()
	{
		if (m_pSwitch == NULL)
		{
			m_pSwitch = new CBCGPSwitchImpl();
			m_pSwitch->SetFocusRect();
		}
		
		return m_pSwitch;
	}
	
protected:
	virtual CBCGPBaseVisualObject* GetVisualObject()
	{
		return GetSwitch();
	}
	
protected:
	CBCGPSwitchImpl* m_pSwitch;
};

BCGCBPRODLLEXPORT void AFXAPI DDX_Switch(CDataExchange* pDX, int nIDC, int& value);

#endif // !defined(AFX_BCGPSWITCHIMPL_H__6B7B4B68_1774_43DC_A61D_0602C62CC4B0__INCLUDED_)
