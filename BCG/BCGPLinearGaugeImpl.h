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
// BCGPLinearGaugeImpl.h: interface for the CBCGPLinearGaugeImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPLINEARGAUGEIMPL_H__FC4A1E16_B594_4EB1_AFE5_6BB3AF94F034__INCLUDED_)
#define AFX_BCGPLINEARGAUGEIMPL_H__FC4A1E16_B594_4EB1_AFE5_6BB3AF94F034__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPVisualCtrl.h"
#include "BCGPGaugeImpl.h"

struct BCGCBPRODLLEXPORT CBCGPLinearGaugeColors
{
	enum BCGP_LINEAR_GAUGE_COLOR_THEME
	{
		BCGP_LINEAR_GAUGE_COLOR_THEME_FIRST = 0,
		BCGP_LINEAR_GAUGE_BLUE              = BCGP_LINEAR_GAUGE_COLOR_THEME_FIRST,
		BCGP_LINEAR_GAUGE_SILVER            = 1,
		BCGP_LINEAR_GAUGE_GOLD              = 2,
		BCGP_LINEAR_GAUGE_BLACK             = 3,
		BCGP_LINEAR_GAUGE_WHITE             = 4,
		BCGP_LINEAR_GAUGE_COLOR_THEME_LAST  = BCGP_LINEAR_GAUGE_WHITE
	};

	CBCGPLinearGaugeColors(BCGP_LINEAR_GAUGE_COLOR_THEME theme = BCGP_LINEAR_GAUGE_BLUE)
	{
		SetTheme(theme);
	}

	CBCGPLinearGaugeColors(const CBCGPLinearGaugeColors& src)
	{
		CopyFrom(src);
	}

	void CopyFrom(const CBCGPLinearGaugeColors& src)
	{
		m_brPointerFill = src.m_brPointerFill;
		m_brPointerOutline = src.m_brPointerOutline;
		m_brScaleFill = src.m_brScaleFill;
		m_brScaleOutline = src.m_brScaleOutline;
		m_brText = src.m_brText;
		m_brFrameFill = src.m_brFrameFill;
		m_brFrameOutline = src.m_brFrameOutline;
		m_brFill = src.m_brFill;
		m_brTickMarkFill = src.m_brTickMarkFill;
		m_brTickMarkOutline = src.m_brTickMarkOutline;
	}

	void SetTheme(BCGP_LINEAR_GAUGE_COLOR_THEME theme);

	CBCGPBrush	m_brPointerFill;
	CBCGPBrush	m_brPointerOutline;

	CBCGPBrush	m_brScaleFill;
	CBCGPBrush	m_brScaleOutline;

	CBCGPBrush	m_brText;
	
	CBCGPBrush	m_brFrameFill;
	CBCGPBrush	m_brFrameOutline;

	CBCGPBrush	m_brFill;

	CBCGPBrush	m_brTickMarkFill;
	CBCGPBrush	m_brTickMarkOutline;
};

class BCGCBPRODLLEXPORT CBCGPLinearGaugePointer : public CBCGPGaugeDataObject
{
	DECLARE_DYNCREATE(CBCGPLinearGaugePointer)

	friend class CBCGPLinearGaugeImpl;

public:
	enum BCGP_GAUGE_POINTER_STYLE
	{
		BCGP_GAUGE_POINTER_STYLE_FIRST = 0,
		BCGP_GAUGE_NEEDLE_TRIANGLE     = BCGP_GAUGE_POINTER_STYLE_FIRST,
		BCGP_GAUGE_NEEDLE_RECT         = 1,
		BCGP_GAUGE_NEEDLE_CIRCLE       = 2,
		BCGP_GAUGE_NEEDLE_DIAMOND      = 3,
		BCGP_GAUGE_NEEDLE_TRIANGLE_INV = 4,
		BCGP_GAUGE_POINTER_STYLE_LAST  = BCGP_GAUGE_NEEDLE_TRIANGLE_INV
	};

	enum BCGP_GAUGE_POINTER_POSITION
	{
		BCGP_GAUGE_POINTER_POSITION_FIRST = 0,
		BCGP_GAUGE_POSITION_NEAR          = BCGP_GAUGE_POINTER_POSITION_FIRST,
		BCGP_GAUGE_POSITION_CENTER        = 1,
		BCGP_GAUGE_POSITION_FAR           = 2,
		BCGP_GAUGE_POINTER_POSITION_LAST  = BCGP_GAUGE_POSITION_FAR
	};

	CBCGPLinearGaugePointer(	BCGP_GAUGE_POINTER_STYLE style = BCGP_GAUGE_NEEDLE_TRIANGLE,
								double dblSize = 0.0, double dblWidth = 0.0,
								BCGP_GAUGE_POINTER_POSITION position = BCGP_GAUGE_POSITION_NEAR)
	{
		m_nScale = -1;
		m_Style = style;
		m_dblSize = dblSize;
		m_dblWidth = dblWidth;
		m_Position = position;
	}

	CBCGPLinearGaugePointer(
		const CBCGPBrush& brFill, const CBCGPBrush& brOutline, 
		BCGP_GAUGE_POINTER_STYLE style = BCGP_GAUGE_NEEDLE_TRIANGLE,
		double dblSize = 0.0, double dblWidth = 0.0, 
		BCGP_GAUGE_POINTER_POSITION position = BCGP_GAUGE_POSITION_NEAR)
	{
		m_nScale = -1;
		m_Style = style;
		m_dblSize = dblSize;
		m_dblWidth = dblWidth;
		m_Position = position;
		m_brFill = brFill;
		m_brOutline = brOutline;
	}

	CBCGPLinearGaugePointer(const CBCGPLinearGaugePointer& srcIn)
	{
		CopyFrom(srcIn);
	}

	virtual CBCGPVisualDataObject* CreateCopy () const
	{
		return new CBCGPLinearGaugePointer(*this);
	}

	virtual void CopyFrom(const CBCGPVisualDataObject& srcIn)
	{
		CBCGPGaugeDataObject::CopyFrom (srcIn);

		const CBCGPLinearGaugePointer& src = (const CBCGPLinearGaugePointer&)srcIn;

		m_dblSize = src.m_dblSize;
		m_dblWidth = src.m_dblWidth;
		m_Style = src.m_Style;
		m_Position = src.m_Position;
	}

	double GetSize() const
	{
		return m_dblSize;
	}
	
	double GetWidth() const
	{
		return m_dblWidth;
	}

	BCGP_GAUGE_POINTER_STYLE GetStyle() const
	{
		return m_Style;
	}

	BCGP_GAUGE_POINTER_POSITION GetPosition() const
	{
		return m_Position;
	}

protected:
	double						m_dblSize;
	double						m_dblWidth;
	BCGP_GAUGE_POINTER_STYLE	m_Style;
	BCGP_GAUGE_POINTER_POSITION	m_Position;
};

class BCGCBPRODLLEXPORT CBCGPLinearGaugeImpl : public CBCGPGaugeImpl
{
	DECLARE_DYNCREATE(CBCGPLinearGaugeImpl)

public:
	CBCGPLinearGaugeImpl(CBCGPVisualContainer* pContainer = NULL);
	virtual ~CBCGPLinearGaugeImpl();

	virtual void CopyFrom(const CBCGPBaseVisualObject& src);

// Attributes:
public:
	void SetVerticalOrientation(BOOL bIsVertical = TRUE)
	{
		m_bIsVertical = bIsVertical;
		SetDirty();
	}

	BOOL IsVerticalOrientation() const
	{
		return m_bIsVertical;
	}

	const CBCGPLinearGaugeColors& GetColors() const
	{
		return m_Colors;
	}

	void SetColors(CBCGPLinearGaugeColors::BCGP_LINEAR_GAUGE_COLOR_THEME theme)
	{
		m_Colors.SetTheme(theme);
	}

	void SetColors(const CBCGPLinearGaugeColors& colors)
	{
		m_Colors = colors;
	}
	
	const CBCGPTextFormat& GetTextFormat() const
	{
		return m_textFormat;
	}

	void SetTextFormat(const CBCGPTextFormat& textFormat);

	virtual BOOL SetValue(double dblVal, int nScale = 0, UINT uiAnimationTime = 100 /* Ms, 0 - no animation */, BOOL bRedraw = FALSE);
	virtual void SetDirty(BOOL bSet = TRUE, BOOL bRedraw = FALSE);

	BOOL ValueToPoint(double dblValue, CBCGPPoint& point, int nScale = 0) const;

// Operations
public:

	// Scales:
	int AddScale();

	// Pointers:
	int AddPointer(const CBCGPLinearGaugePointer& pointer, int nScale = 0, BOOL bRedraw = FALSE);

	BOOL RemovePointer(int nIndex, BOOL bRedraw = FALSE);

	int GetPointersCount() const
	{
		return (int)m_arData.GetSize();
	}

	BOOL ModifyPointer(int nIndex, const CBCGPLinearGaugePointer& pointer, BOOL bRedraw = FALSE);

// Overrides:
public:
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);

	virtual void OnDrawScale(CBCGPGraphicsManager* pGM, int nScale);
	virtual void OnDrawPointer(CBCGPGraphicsManager* pGM, 
		const CBCGPPointsArray& arPoints, 
		const CBCGPPointsArray& arShadowPoints,
		int nIndex);
	virtual void OnDrawTickMark(CBCGPGraphicsManager* pGM, const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo, 
		CBCGPGaugeScaleObject::BCGP_TICKMARK_STYLE style,
		BOOL bMajor, double dblVal, int nScale, 
		const CBCGPBrush& brFill = CBCGPBrush(), const CBCGPBrush& brOutline = CBCGPBrush());

	virtual CBCGPSize GetDefaultSize(CBCGPGraphicsManager* pGM, const CBCGPBaseVisualObject* pParentGauge);

	virtual BOOL HitTestValue(const CBCGPPoint& pt, double& dblValue, int nScale = 0, BOOL bInsideGauge = TRUE) const;

	virtual BOOL OnSetMouseCursor(const CBCGPPoint& pt);
	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld);

protected:
	void CreatePointerPoints(CBCGPPointsArray& arPoints, int nPointerIndex, BOOL bShadow);

	BOOL GetRangeShape(	CBCGPRect& rect, CBCGPPolygonGeometry& shape, double dblStartValue, double dblFinishValue,
						double dblStartWidth, double dblFinishWidth,
						double dblOffsetFromFrame, int nScale);

	CBCGPSize GetTextLabelMaxSize(CBCGPGraphicsManager* pGM);

	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole)
	{
		HRESULT hr = CBCGPGaugeImpl::get_accRole(varChild, pvarRole);
		if (SUCCEEDED(hr))
		{
			pvarRole->lVal = ROLE_SYSTEM_PANE;
		}
		
		return hr;
	}

protected:
	BOOL					m_bIsVertical;
	CBCGPImage				m_ImageCache;
	CBCGPLinearGaugeColors	m_Colors;
	CBCGPTextFormat			m_textFormat;
	BOOL					m_bInSetValue;
	CBCGPSize				m_sizeMaxLabel;
	double					m_dblMaxRangeSize;
};

class BCGCBPRODLLEXPORT CBCGPLinearGaugeCtrl : public CBCGPVisualCtrl
{
	DECLARE_DYNAMIC(CBCGPLinearGaugeCtrl)

public:
	CBCGPLinearGaugeCtrl()
	{
		m_pGauge = NULL;
	}

	virtual ~CBCGPLinearGaugeCtrl()
	{
		if (m_pGauge != NULL)
		{
			delete m_pGauge;
		}
	}

	BOOL CreateCustomGauge(CRuntimeClass* pGaugeRTC)
	{
		if (pGaugeRTC != NULL && !pGaugeRTC->IsDerivedFrom(RUNTIME_CLASS(CBCGPLinearGaugeImpl)))
		{
			ASSERT(FALSE);
			TRACE0("CBCGPLinearGaugeCtrl::CreateCustomGauge: custom Gauge must be derived from CBCGPLinearGaugeImpl.");
			return FALSE;
		}

		if (m_pGauge != NULL)
		{
			delete m_pGauge;
		}

		if (pGaugeRTC != NULL)
		{
			m_pGauge = DYNAMIC_DOWNCAST(CBCGPLinearGaugeImpl, pGaugeRTC->CreateObject());
		}
		else
		{
			m_pGauge = new CBCGPLinearGaugeImpl();	
		}	

		if (m_pGauge != NULL)
		{
			m_pGauge->SetOwner(this);
		}

		ASSERT_VALID(m_pGauge);
		return TRUE;
	}

	virtual CBCGPLinearGaugeImpl*	GetGauge()
	{
		if (m_pGauge == NULL)
		{
			m_pGauge = new CBCGPLinearGaugeImpl();
		}

		return m_pGauge;
	}

protected:
	virtual CBCGPBaseVisualObject* GetVisualObject()
	{
		return GetGauge();
	}

protected:
	CBCGPLinearGaugeImpl*	m_pGauge;
};

#endif // !defined(AFX_BCGPLINEARGAUGEIMPL_H__FC4A1E16_B594_4EB1_AFE5_6BB3AF94F034__INCLUDED_)
