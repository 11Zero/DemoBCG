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
// BCGPNumericIndicatorImpl.h: interface for the CBCGPNumericIndicatorImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPNUMERICINDICATORIMPL_H__992533BF_C07E_4275_B8A4_8CD7A181785D__INCLUDED_)
#define AFX_BCGPNUMERICINDICATORIMPL_H__992533BF_C07E_4275_B8A4_8CD7A181785D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPGaugeImpl.h"

class CBCGPDigit
{
protected:
	struct XSegment
	{
		XSegment()
		{
		}
		XSegment(const XSegment& segment)
		{
			m_Points.Copy(segment.m_Points);
			m_DrawPoints.Copy(segment.m_DrawPoints);
		}

		XSegment& operator = (const XSegment& segment)
		{
			m_Points.Copy(segment.m_Points);
			m_DrawPoints.Copy(segment.m_DrawPoints);
			return *this;
		}

		CBCGPPointsArray m_Points;
		CBCGPPointsArray m_DrawPoints;

		void CreateDrawPoints(double scale);
		void Draw(CBCGPGraphicsManager& gm, const CBCGPBrush& br, const CBCGPRect& rect);
		CBCGPRect GetBounds() const;
	};
	struct XDigit
	{
		XDigit()
			: m_Scale(0.0)
		{
		}
		XDigit(const XDigit& digit)
		{
			m_Segments.Copy(digit.m_Segments);
			m_Dot    = digit.m_Dot;
			m_Bounds = digit.m_Bounds;
			m_Scale  = digit.m_Scale;
		}

		CArray<XSegment, XSegment>	m_Segments;
		CBCGPRect					m_Dot;
		CBCGPRect					m_Bounds;
		double						m_Scale;

		void CreateDrawPoints(double scale);
		void DrawSegments(CBCGPGraphicsManager& gm, const CBCGPBrush& br, const CString& strSegments, const CBCGPRect& rect, BOOL bInvert = FALSE);
		void DrawDot(CBCGPGraphicsManager& gm, const CBCGPBrush& br, const CBCGPRect& rect);
		void Initialize(double skewAngle);
		CBCGPRect GetBounds(BOOL bDot) const;
	};

public:
	CBCGPDigit();
	virtual ~CBCGPDigit();

	virtual void Create (double skewAngle = 0.0) = 0;

	CBCGPRect CreateDigit(CBCGPGraphicsManager& gm, const CBCGPRect& rect, BOOL bDot);
	void DrawDigit(CBCGPGraphicsManager& gm, const CBCGPBrush& br, TCHAR ch, const CBCGPRect& rect, double opacityInv = 0.0);
	void DrawDot(CBCGPGraphicsManager& gm, const CBCGPBrush& br, const CBCGPRect& rect);

	CBCGPSize GetDefaultSize(CBCGPGraphicsManager& gm, BOOL bDot) const;

protected:
	void Initialize(double skewAngle);

protected:
	CArray<XDigit, XDigit>	m_Digits;
	CArray<XDigit, XDigit>	m_DigitsFix;
	int						m_Digit;
};

struct BCGCBPRODLLEXPORT CBCGPNumericIndicatorColors
{
	CBCGPNumericIndicatorColors(int nTheme = 0 /* Classic theme */);
	CBCGPNumericIndicatorColors(const CBCGPNumericIndicatorColors& src)
	{
		CopyFrom(src);
	}

	void CopyFrom(const CBCGPNumericIndicatorColors& src)
	{
		m_brFill = src.m_brFill;
		m_brOutline = src.m_brOutline;
		m_brSeparator = src.m_brSeparator;
		m_brDigit = src.m_brDigit;
		m_brDecimal = src.m_brDecimal;
		m_brSign = src.m_brSign;
		m_brDot = src.m_brDot;
	}

	CBCGPBrush	m_brFill;
	CBCGPBrush	m_brOutline;
	CBCGPBrush	m_brSeparator;
	CBCGPBrush	m_brDigit;
	CBCGPBrush	m_brDecimal;
	CBCGPBrush	m_brSign;
	CBCGPBrush	m_brDot;
};

class BCGCBPRODLLEXPORT CBCGPNumericIndicatorImpl : public CBCGPGaugeImpl  
{
	DECLARE_DYNCREATE(CBCGPNumericIndicatorImpl)

public:
	enum BCGPNumericIndicatorStyle
	{
		BCGP_NUMERIC_INDICATOR_STYLE_FIRST = 0,
		BCGP_NUMERIC_INDICATOR_CLASSIC     = BCGP_NUMERIC_INDICATOR_STYLE_FIRST,
		BCGP_NUMERIC_INDICATOR_DIGITAL     = 1,
		BCGP_NUMERIC_INDICATOR_STYLE_LAST  = BCGP_NUMERIC_INDICATOR_DIGITAL
	};

public:
	CBCGPNumericIndicatorImpl(BCGPNumericIndicatorStyle style = BCGP_NUMERIC_INDICATOR_CLASSIC,
		CBCGPVisualContainer* pContainer = NULL);
	virtual ~CBCGPNumericIndicatorImpl();

	virtual void CopyFrom(const CBCGPBaseVisualObject& src);

// Attributes:
public:
	enum BCGPNumericIndicatorCellType
	{
		BCGP_NUMERIC_INDICATOR_CELL_TYPE_FIRST = 0,
		BCGP_NUMERIC_INDICATOR_CELL_DIGIT      = BCGP_NUMERIC_INDICATOR_CELL_TYPE_FIRST,
		BCGP_NUMERIC_INDICATOR_CELL_DECIMAL    = 1,
		BCGP_NUMERIC_INDICATOR_CELL_DOT        = 2,
		BCGP_NUMERIC_INDICATOR_CELL_SIGN       = 3,
		BCGP_NUMERIC_INDICATOR_CELL_ERROR      = 4,
		BCGP_NUMERIC_INDICATOR_CELL_TYPE_LAST  = BCGP_NUMERIC_INDICATOR_CELL_ERROR
	};

	BOOL SetValue(double dblVal, BOOL bRedraw = FALSE);

	const CBCGPNumericIndicatorColors& GetColors() const
	{
		return m_Colors;
	}

	void SetColors(const CBCGPNumericIndicatorColors& colors)
	{
		m_Colors = colors;
		SetDirty();
	}

	void SetCells(int nCells)
	{
		m_nCells = nCells;
		SetDirty();
	}

	int GetCells() const
	{
		return m_nCells;
	}

	void SetDecimals(int nDecimals)
	{
		m_nDecimals = nDecimals;
		SetDirty();
	}

	int GetDecimals() const
	{
		return m_nDecimals;
	}

	void SetSeparatorWidth(int nSeparatorWidth)
	{
		m_nSeparatorWidth = nSeparatorWidth;
		SetDirty();
	}

	int GetSeparatorWidth() const
	{
		return m_nSeparatorWidth;
	}

	void SetDrawSign(BOOL bDrawSign = TRUE)
	{
		m_bDrawSign = bDrawSign;
		SetDirty();
	}

	BOOL IsDrawSign() const
	{
		return m_bDrawSign;
	}

	void SetDrawDecimalPoint(BOOL bDrawDecimalPoint = TRUE)
	{
		m_bDrawDecimalPoint = bDrawDecimalPoint;
		SetDirty();
	}

	BOOL IsDrawDecimalPoint() const
	{
		return m_bDrawDecimalPoint;
	}

	void SetDrawLeadingZeros(BOOL bDrawLeadingZeros = TRUE)
	{
		m_bDrawLeadingZeros = bDrawLeadingZeros;
		SetDirty();
	}

	BOOL IsDrawLeadingZeros() const
	{
		return m_bDrawLeadingZeros;
	}

	BCGPNumericIndicatorStyle GetStyle() const
	{
		return m_Style;
	}

	void SetStyle(BCGPNumericIndicatorStyle style);

	const CBCGPTextFormat& GetTextFormat() const
	{
		return m_textFormat;
	}

	void SetTextFormat(const CBCGPTextFormat& textFormat);

// Overrides:
public:
	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags = BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);

	virtual void OnDrawFrame(CBCGPGraphicsManager* pGM, CBCGPRect rectFrame);
	virtual void OnDrawSeparator(CBCGPGraphicsManager* pGM, CBCGPRect rectSeparator);
	virtual void OnDrawChar(CBCGPGraphicsManager* pGM, const CBCGPTextFormat& tf,
		CBCGPRect rectCell, TCHAR ch, int nCellIndex, BCGPNumericIndicatorCellType type);
	virtual CBCGPSize GetDefaultSize(CBCGPGraphicsManager* pGM, const CBCGPBaseVisualObject* pParentGauge);
	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld);

protected:
	void CreateResources();

	virtual HRESULT get_accRole(VARIANT varChild, VARIANT *pvarRole)
	{
		HRESULT hr = CBCGPGaugeImpl::get_accRole(varChild, pvarRole);
		if (SUCCEEDED(hr))
		{
			pvarRole->lVal = ROLE_SYSTEM_STATICTEXT;
		}
		
		return hr;
	}

// Atrtributes:
protected:
	int							m_nCells;
	int							m_nDecimals;
	int							m_nSeparatorWidth;
	BOOL						m_bDrawSign;
	BOOL						m_bDrawDecimalPoint;
	BOOL						m_bDrawLeadingZeros;
	CBCGPNumericIndicatorColors	m_Colors;
	BCGPNumericIndicatorStyle	m_Style;
	CBCGPTextFormat				m_textFormat;
	CBCGPDigit*					m_Digit;
	CBCGPSize					m_sizeMargin;
};

#endif // !defined(AFX_BCGPNUMERICINDICATORIMPL_H__992533BF_C07E_4275_B8A4_8CD7A181785D__INCLUDED_)
