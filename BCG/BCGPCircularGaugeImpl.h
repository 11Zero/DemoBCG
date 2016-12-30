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
// BCGPGaugeImpl.h: interface for the CBCGPCircularGaugeImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPCIRCULARGAUGEIMPL_H__3E99A0DC_6DEA_4C69_822D_D79E636EC4ED__INCLUDED_)
#define AFX_BCGPCIRCULARGAUGEIMPL_H__3E99A0DC_6DEA_4C69_822D_D79E636EC4ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPVisualCtrl.h"
#include "BCGPGaugeImpl.h"

struct BCGCBPRODLLEXPORT CBCGPCircularGaugeColors
{
	enum BCGP_CIRCULAR_GAUGE_COLOR_THEME
	{
		BCGP_CIRCULAR_GAUGE_COLOR_THEME_FIRST = 0,
		BCGP_CIRCULAR_GAUGE_BLUE              = BCGP_CIRCULAR_GAUGE_COLOR_THEME_FIRST,
		BCGP_CIRCULAR_GAUGE_SILVER            = 1,
		BCGP_CIRCULAR_GAUGE_GOLD              = 2,
		BCGP_CIRCULAR_GAUGE_BLACK             = 3,
		BCGP_CIRCULAR_GAUGE_WHITE             = 4,
		BCGP_CIRCULAR_GAUGE_COLOR_THEME_LAST  = BCGP_CIRCULAR_GAUGE_WHITE
	};

	CBCGPCircularGaugeColors(BCGP_CIRCULAR_GAUGE_COLOR_THEME theme = BCGP_CIRCULAR_GAUGE_BLUE)
	{
		SetTheme(theme);
	}

	CBCGPCircularGaugeColors(const CBCGPCircularGaugeColors& src)
	{
		CopyFrom(src);
	}

	void CopyFrom(const CBCGPCircularGaugeColors& src)
	{
		m_brPointerFill = src.m_brPointerFill;
		m_brPointerOutline = src.m_brPointerOutline;
		m_brScaleFill = src.m_brScaleFill;
		m_brScaleOutline = src.m_brScaleOutline;
		m_brText = src.m_brText;
		m_brFrameFill = src.m_brFrameFill;
		m_brFrameFillInv = src.m_brFrameFillInv;
		m_brFrameOutline = src.m_brFrameOutline;
		m_brFill = src.m_brFill;
		m_brCapFill = src.m_brCapFill;
		m_brCapOutline = src.m_brCapOutline;
		m_brTickMarkFill = src.m_brTickMarkFill;
		m_brTickMarkOutline = src.m_brTickMarkOutline;
	}

	void SetTheme(BCGP_CIRCULAR_GAUGE_COLOR_THEME theme);

	CBCGPBrush	m_brPointerFill;
	CBCGPBrush	m_brPointerOutline;

	CBCGPBrush	m_brScaleFill;
	CBCGPBrush	m_brScaleOutline;

	CBCGPBrush	m_brText;
	
	CBCGPBrush	m_brFrameFill;
	CBCGPBrush	m_brFrameFillInv;
	CBCGPBrush	m_brFrameOutline;

	CBCGPBrush	m_brFill;

	CBCGPBrush	m_brCapFill;
	CBCGPBrush	m_brCapOutline;

	CBCGPBrush	m_brTickMarkFill;
	CBCGPBrush	m_brTickMarkOutline;
};

class BCGCBPRODLLEXPORT CBCGPCircularGaugeScale : public CBCGPGaugeScaleObject
{
	DECLARE_DYNCREATE(CBCGPCircularGaugeScale)

	friend class CBCGPCircularGaugeImpl;

public:
	CBCGPCircularGaugeScale()
	{
		m_dblStartAngle = 225.;
		m_dblFinishAngle = -45.;
		m_bRotateLabels = FALSE;
		m_bIsClosed = FALSE;
		m_bDrawLastTickMark = TRUE;
		m_bAnimationThroughStart = TRUE;
	}

	CBCGPCircularGaugeScale(const CBCGPCircularGaugeScale& srcIn)
	{
		CopyFrom(srcIn);
	}

	virtual CBCGPGaugeScaleObject* CreateCopy() const
	{
		return new CBCGPCircularGaugeScale(*this);
	}

	virtual void CopyFrom(const CBCGPGaugeScaleObject& srcIn)
	{
		CBCGPGaugeScaleObject::CopyFrom (srcIn);

		const CBCGPCircularGaugeScale& src = (const CBCGPCircularGaugeScale&)srcIn;

		m_dblStartAngle = src.m_dblStartAngle;
		m_dblFinishAngle = src.m_dblFinishAngle;
		m_bRotateLabels = src.m_bRotateLabels;
		m_bIsClosed = src.m_bIsClosed;
		m_bDrawLastTickMark = src.m_bDrawLastTickMark;
		m_bAnimationThroughStart = src.m_bAnimationThroughStart;
	}

	virtual void CleanUp()
	{
		CBCGPGaugeScaleObject::CleanUp();

		m_arPointsTickStart.RemoveAll();
		m_arPointsTickFinish.RemoveAll();
		m_arRectTickLabel.RemoveAll();
	}

	double GetStartAngle() const
	{
		return m_dblStartAngle;
	}

	double GetFinishAngle() const
	{
		return m_dblFinishAngle;
	}

protected:
	double								m_dblStartAngle;
	double								m_dblFinishAngle;
	BOOL								m_bRotateLabels;
	BOOL								m_bIsClosed;
	BOOL								m_bDrawLastTickMark;
	BOOL								m_bAnimationThroughStart;

	CBCGPPointsArray					m_arPointsTickStart;
	CBCGPPointsArray					m_arPointsTickFinish;
	CArray<CBCGPRect, const CBCGPRect&>	m_arRectTickLabel;
};

class BCGCBPRODLLEXPORT CBCGPCircularGaugePointer : public CBCGPGaugeDataObject
{
	DECLARE_DYNCREATE(CBCGPCircularGaugePointer)

	friend class CBCGPCircularGaugeImpl;

public:
	enum BCGP_GAUGE_POINTER_STYLE
	{
		BCGP_GAUGE_POINTER_STYLE_FIRST = 0,
		BCGP_GAUGE_NEEDLE_TRIANGLE     = BCGP_GAUGE_POINTER_STYLE_FIRST,
		BCGP_GAUGE_NEEDLE_RECT         = 1,
		BCGP_GAUGE_NEEDLE_ARROW        = 2,
		BCGP_GAUGE_NEEDLE_ARROW_BASE   = 3,
		BCGP_GAUGE_MARKER_TRIANGLE     = 4,
		BCGP_GAUGE_MARKER_RECT         = 5,
		BCGP_GAUGE_MARKER_CIRCLE       = 6,
		BCGP_GAUGE_MARKER_DIAMOND      = 7,
		BCGP_GAUGE_POINTER_STYLE_LAST  = BCGP_GAUGE_MARKER_DIAMOND
	};

	CBCGPCircularGaugePointer(	BCGP_GAUGE_POINTER_STYLE style = BCGP_GAUGE_NEEDLE_TRIANGLE,
								double dblSize = 0.0, double dblWidth = 0.0, BOOL bExtraLen = FALSE)
	{
		m_nScale = -1;
		m_Style = style;
		m_dblSize = dblSize;
		m_dblWidth = dblWidth;
		m_bExtraLen = bExtraLen;
	}

	CBCGPCircularGaugePointer(
		const CBCGPBrush& brFill, const CBCGPBrush& brOutline, 
		BCGP_GAUGE_POINTER_STYLE style = BCGP_GAUGE_NEEDLE_TRIANGLE,
		double dblSize = 0.0, double dblWidth = 0.0, BOOL bExtraLen = FALSE)
	{
		m_nScale = -1;
		m_Style = style;
		m_dblSize = dblSize;
		m_dblWidth = dblWidth;
		m_bExtraLen = bExtraLen;
		m_brFill = brFill;
		m_brOutline = brOutline;
	}

	CBCGPCircularGaugePointer(const CBCGPCircularGaugePointer& srcIn)
	{
		CopyFrom(srcIn);
	}

	virtual CBCGPVisualDataObject* CreateCopy () const
	{
		return new CBCGPCircularGaugePointer(*this);
	}

	virtual void CopyFrom(const CBCGPVisualDataObject& srcIn)
	{
		CBCGPGaugeDataObject::CopyFrom (srcIn);

		const CBCGPCircularGaugePointer& src = (const CBCGPCircularGaugePointer&)srcIn;

		m_dblSize = src.m_dblSize;
		m_dblWidth = src.m_dblWidth;
		m_bExtraLen = src.m_bExtraLen;
		m_Style = src.m_Style;
	}

	double GetSize() const
	{
		return m_dblSize;
	}
	
	double GetWidth() const
	{
		return m_dblWidth;
	}

	BOOL IsExtraLen () const
	{
		return m_bExtraLen;
	}

	BCGP_GAUGE_POINTER_STYLE GetStyle() const
	{
		return m_Style;
	}

	virtual BOOL IsCircleShape() const
	{
		return m_Style == BCGP_GAUGE_MARKER_CIRCLE;
	}

protected:
	double						m_dblSize;
	double						m_dblWidth;
	BOOL						m_bExtraLen;
	BCGP_GAUGE_POINTER_STYLE	m_Style;
};

#define CBCGPCircularGaugeColoredRangeObject CBCGPGaugeColoredRangeObject

class BCGCBPRODLLEXPORT CBCGPCircularGaugeImpl : public CBCGPGaugeImpl
{
	DECLARE_DYNCREATE(CBCGPCircularGaugeImpl)

public:
	CBCGPCircularGaugeImpl(CBCGPVisualContainer* pContainer = NULL);
	virtual ~CBCGPCircularGaugeImpl();

	virtual void CopyFrom(const CBCGPBaseVisualObject& src);

// Attributes:
public:
	const CBCGPCircularGaugeColors& GetColors() const
	{
		return m_Colors;
	}

	void SetColors(CBCGPCircularGaugeColors::BCGP_CIRCULAR_GAUGE_COLOR_THEME theme)
	{
		m_Colors.SetTheme(theme);
		SetDirty();
	}

	void SetColors(const CBCGPCircularGaugeColors& colors)
	{
		m_Colors = colors;
		SetDirty();
	}
	
	const CBCGPTextFormat& GetTextFormat() const
	{
		return m_textFormat;
	}

	void SetTextFormat(const CBCGPTextFormat& textFormat);

	double GetCapSize() const
	{
		return m_CapSize;
	}

	void SetCapSize(double capSize)
	{
		m_CapSize = capSize;
		SetDirty();
	}

	virtual BOOL SetValue(double dblVal, int nScale = 0, UINT uiAnimationTime = 100 /* Ms, 0 - no animation */, BOOL bRedraw = FALSE);
	virtual void SetDirty(BOOL bSet = TRUE, BOOL bRedraw = FALSE);

	// Scale attributes:
	void SetTicksAreaAngles(double dblStartAngle, double dblFinishAngle, int nScale = 0);	// In degrees;
	double GetTicksAreaStartAngle(int nScale = 0) const;
	double GetTicksAreaFinishAngle(int nScale = 0) const;

	void EnableLabelsRotation(BOOL bEnable = TRUE, int nScale = 0);
	BOOL IsLabelRotation(int nScale = 0) const;

	BOOL IsClosed(int nScale = 0) const;
	BOOL IsDrawLastTickMark(int nScale = 0) const;
	BOOL IsAnimationThroughStart(int nScal = 0) const;

	void EnableShapeByTicksArea(BOOL bEnable = TRUE)
	{
		if (m_bDrawTicksOutsideFrame)
		{
			ASSERT(FALSE);
			return;
		}

		m_bShapeByTicksArea = bEnable;
		SetDirty();
	}

	BOOL IsShapeByTicksArea() const
	{
		return m_bShapeByTicksArea;
	}

	BOOL ValueToAngle(double dblValue, double& dblAngle, int nScale = 0) const;
	BOOL AngleToValue(double dblAngle, double& dblValue, int nScale = 0) const;

// Operations
public:

	// Scales:
	int AddScale();

	// Pointers:
	int AddPointer(const CBCGPCircularGaugePointer& pointer, int nScale = 0, BOOL bRedraw = FALSE);

	BOOL RemovePointer(int nIndex, BOOL bRedraw = FALSE);

	int GetPointersCount() const
	{
		return (int)m_arData.GetSize();
	}

	BOOL ModifyPointer(int nIndex, const CBCGPCircularGaugePointer& pointer, BOOL bRedraw = FALSE);

	// Sub-gauges:
	void AddSubGauge(CBCGPGaugeImpl* pGauge, BCGP_SUB_GAUGE_POS pos, const CBCGPSize& size = CBCGPSize(), const CBCGPPoint& ptOffset = CBCGPPoint(), BOOL bAutoDestroy = TRUE);
	BOOL RemoveSubGauge(CBCGPGaugeImpl* pGauge);
	void RemoveAllSubGauges();

	const CArray<CBCGPGaugeImpl*, CBCGPGaugeImpl*>& GetSubGauges() const
	{
		return m_arSubGauges;
	}

	CBCGPGaugeImpl* GetSubGaugeByID(UINT nID);

// Overrides:
public:
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);
	virtual void SetRect(const CBCGPRect& rect, BOOL bRedraw = FALSE);
	virtual CWnd* SetOwner(CWnd* pWndOwner, BOOL bRedraw = FALSE);

	virtual void OnDrawScale(CBCGPGraphicsManager* pGM, int nScale, const CBCGPPoint& center, double radius);
	virtual void OnDrawPointer(CBCGPGraphicsManager* pGM, 
		const CBCGPPointsArray& arPoints, 
		const CBCGPPointsArray& arShadowPoints,
		int nIndex);
	virtual void OnDrawPointerCap(CBCGPGraphicsManager* pGM, const CBCGPRect& rectCap);
	virtual void OnDrawTickMark(CBCGPGraphicsManager* pGM, const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo, 
		CBCGPCircularGaugeScale::BCGP_TICKMARK_STYLE style,
		BOOL bMajor, double dblVal, double dblAngle, int nScale, 
		const CBCGPBrush& brFill = CBCGPBrush(), const CBCGPBrush& brOutline = CBCGPBrush());

	virtual void SetRange(double dblStart, double dblFinish, int nScale = 0);

	virtual void SetClosedRange(double dblStart, double dblFinish, 
		double dblStartAngle = 90, BOOL bDrawLastTickMark = TRUE, BOOL bAnimationThroughStart = TRUE,
		int nScale = 0);

	virtual CBCGPSize GetMinSize() const
	{
		return CBCGPSize(150, 150);
	}

	virtual CBCGPSize GetDefaultSize(CBCGPGraphicsManager* pGM, const CBCGPBaseVisualObject* pParentGauge);

	virtual BOOL HitTestValue(const CBCGPPoint& pt, double& dblValue, int nScale = 0, BOOL bInsideGauge = TRUE) const;
	virtual BOOL OnSetMouseCursor(const CBCGPPoint& pt);

	virtual void SetScrollOffset(const CBCGPPoint& ptScrollOffset);
	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld);
	
	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole)
	{
		HRESULT hr = CBCGPGaugeImpl::get_accRole(varChild, pvarRole);
		if (SUCCEEDED(hr))
		{
			pvarRole->lVal = ROLE_SYSTEM_DIAL;
		}
		
		return hr;
	}

protected:
	virtual void CreatePointerPoints(double dblRadius, CBCGPPointsArray& arPoints, int nPointerIndex, BOOL bShadow);

	BOOL CreateRangeShape(	const CBCGPPoint& center, double radius,
							CBCGPComplexGeometry& shape, double dblStartValue, double dblFinishValue,
							double dblStartWidth, double dblFinishWidth,
							double dblOffsetFromFrame, int nScale);

	BOOL CreateFrameShape(	const CBCGPPoint& center, double radius,
							CBCGPComplexGeometry& shape, BOOL bInternal);

	CBCGPCircularGaugeScale* GetScale(int nScale) const;

	virtual BOOL ReposAllSubGauges(CBCGPGraphicsManager* pGM);
	void ReposSubGauge(CBCGPGraphicsManager* pGM, CBCGPGaugeImpl* pSubGauge, const CBCGPSize& size);

	virtual double GetMaxLabelWidth(CBCGPGraphicsManager* pGM, int nScale);

protected:
	BOOL										m_bShapeByTicksArea;
	BOOL										m_bDrawTextBeforeTicks;
	BOOL										m_bDrawTicksOutsideFrame;
	double										m_CapSize;
	double										m_ScaleSize;

	CBCGPImage									m_ImageCache;

	CBCGPCircularGaugeColors					m_Colors;
	CBCGPTextFormat								m_textFormat;
	int											m_nCurrLabelIndex;

	CBCGPComplexGeometry						m_GeometryShape1;
	CBCGPComplexGeometry						m_GeometryShape2;

	CArray<CBCGPGaugeImpl*, CBCGPGaugeImpl*>	m_arSubGauges;
	BOOL										m_bInSetValue;
};

class BCGCBPRODLLEXPORT CBCGPCircularGaugeCtrl : public CBCGPVisualCtrl
{
	DECLARE_DYNAMIC(CBCGPCircularGaugeCtrl)

public:
	CBCGPCircularGaugeCtrl()
	{
		m_pGauge = NULL;
	}

	virtual ~CBCGPCircularGaugeCtrl()
	{
		if (m_pGauge != NULL)
		{
			delete m_pGauge;
		}
	}

	BOOL CreateCustomGauge(CRuntimeClass* pGaugeRTC)
	{
		if (pGaugeRTC != NULL && !pGaugeRTC->IsDerivedFrom(RUNTIME_CLASS(CBCGPCircularGaugeImpl)))
		{
			ASSERT(FALSE);
			TRACE0("CBCGPCircularGaugeCtrl::CreateCustomGauge: custom Gauge must be derived from CBCGPCircularGaugeImpl.");
			return FALSE;
		}

		if (m_pGauge != NULL)
		{
			delete m_pGauge;
		}

		if (pGaugeRTC != NULL)
		{
			m_pGauge = DYNAMIC_DOWNCAST(CBCGPCircularGaugeImpl, pGaugeRTC->CreateObject());
		}
		else
		{
			m_pGauge = new CBCGPCircularGaugeImpl();	
		}	

		if (m_pGauge != NULL)
		{
			m_pGauge->SetOwner(this);
		}

		ASSERT_VALID(m_pGauge);
		return TRUE;
	}

	virtual CBCGPCircularGaugeImpl*	GetGauge()
	{
		if (m_pGauge == NULL)
		{
			m_pGauge = new CBCGPCircularGaugeImpl();
		}

		return m_pGauge;
	}

protected:
	virtual CBCGPBaseVisualObject* GetVisualObject()
	{
		return GetGauge();
	}

protected:
	CBCGPCircularGaugeImpl*	m_pGauge;
};

#endif // !defined(AFX_BCGPCIRCULARGAUGEIMPL_H__3E99A0DC_6DEA_4C69_822D_D79E636EC4ED__INCLUDED_)
