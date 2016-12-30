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
// BCGPKnob.h: interface for the CBCGPKnob class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPKNOB_H__E0223345_9C90_461A_9C67_F97F1F3A39B4__INCLUDED_)
#define AFX_BCGPKNOB_H__E0223345_9C90_461A_9C67_F97F1F3A39B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPCircularGaugeImpl.h"

class BCGCBPRODLLEXPORT CBCGPKnobPointer : public CBCGPGaugeDataObject
{
	DECLARE_DYNCREATE(CBCGPKnobPointer)

	friend class CBCGPCircularGaugeImpl;

public:
	enum BCGP_KNOB_POINTER_STYLE
	{
		BCGP_KNOB_POINTER_STYLE_FIRST = 0,
		BCGP_KNOB_POINTER_CIRCLE      = BCGP_KNOB_POINTER_STYLE_FIRST,
		BCGP_KNOB_POINTER_LINE        = 1,
		BCGP_KNOB_POINTER_HANDLE      = 2,
		BCGP_KNOB_POINTER_STYLE_LAST  = BCGP_KNOB_POINTER_HANDLE
	};

	CBCGPKnobPointer(	BCGP_KNOB_POINTER_STYLE style = BCGP_KNOB_POINTER_CIRCLE,
								double dblOffsetFromCenter = 0.0, double dblWidth = 0.0)
	{
		m_nScale = 0;
		m_Style = style;
		m_dblOffsetFromCenter = dblOffsetFromCenter;
		m_dblWidth = dblWidth;
	}

	CBCGPKnobPointer(
		const CBCGPBrush& brFill, const CBCGPBrush& brOutline, 
		BCGP_KNOB_POINTER_STYLE style = BCGP_KNOB_POINTER_CIRCLE,
		double dblOffsetFromCenter = 0.0, double dblWidth = 0.0)
	{
		m_nScale = 0;
		m_Style = style;
		m_dblOffsetFromCenter = dblOffsetFromCenter;
		m_dblWidth = dblWidth;
		m_brFill = brFill;
		m_brOutline = brOutline;
	}

	CBCGPKnobPointer(const CBCGPKnobPointer& srcIn)
	{
		CopyFrom(srcIn);
	}

	virtual CBCGPVisualDataObject* CreateCopy () const
	{
		return new CBCGPKnobPointer(*this);
	}

	virtual void CopyFrom(const CBCGPVisualDataObject& srcIn)
	{
		CBCGPGaugeDataObject::CopyFrom (srcIn);

		const CBCGPKnobPointer& src = (const CBCGPKnobPointer&)srcIn;

		m_dblOffsetFromCenter = src.m_dblOffsetFromCenter;
		m_dblWidth = src.m_dblWidth;
		m_Style = src.m_Style;
	}

	double GetOffsetFromCenter() const
	{
		return m_dblOffsetFromCenter;
	}
	
	double GetWidth() const
	{
		return m_dblWidth;
	}

	BCGP_KNOB_POINTER_STYLE GetStyle() const
	{
		return m_Style;
	}

	virtual BOOL IsCircleShape() const
	{
		return m_Style == BCGP_KNOB_POINTER_CIRCLE;
	}

protected:
	double						m_dblOffsetFromCenter;
	double						m_dblWidth;
	BCGP_KNOB_POINTER_STYLE		m_Style;
};

class BCGCBPRODLLEXPORT CBCGPKnob : public CBCGPCircularGaugeImpl
{
	DECLARE_DYNCREATE(CBCGPKnob)

public:
	CBCGPKnob(CBCGPVisualContainer* pContainer = NULL);
	virtual ~CBCGPKnob();

	virtual void CopyFrom(const CBCGPBaseVisualObject& src);

	void SetPointer(const CBCGPKnobPointer& pointer, BOOL bRedraw = FALSE);
	void SetColors(CBCGPCircularGaugeColors::BCGP_CIRCULAR_GAUGE_COLOR_THEME theme);

	void SetColors(const CBCGPCircularGaugeColors& colors)
	{
		m_Colors = colors;
		SetDirty();
	}

	void SetImageList(UINT uiResID, int cx = 16, const CBCGPSize& szMargin = CBCGPSize(0, 0));

protected:
	virtual CBCGPSize GetMinSize() const
	{
		return CBCGPSize(20, 20);
	}

	virtual void CreatePointerPoints(double dblRadius, CBCGPPointsArray& arPoints, int nPointerIndex, BOOL bShadow);
	virtual CBCGPSize GetTickMarkTextLabelSize(CBCGPGraphicsManager* pGM, const CString& strLabel, const CBCGPTextFormat& tf);
	virtual void OnDrawTickMarkTextLabel(CBCGPGraphicsManager* pGM, const CBCGPTextFormat& tf, const CBCGPRect& rectText, const CString& strLabel, double dblVal, int nScale, const CBCGPBrush& br);

	virtual BOOL GetGestureConfig(CBCGPGestureConfig& gestureConfig);
	virtual BOOL OnGestureEventRotate(const CBCGPPoint& ptCenter, double dblAngle);

	CBCGPImage				m_Icons;
	CBCGPSize				m_sizeIcon;
	CBCGPSize				m_sizeIconMargin;
};

class BCGCBPRODLLEXPORT CBCGPKnobCtrl : public CBCGPVisualCtrl
{
	DECLARE_DYNAMIC(CBCGPKnobCtrl)

public:
	CBCGPKnobCtrl()
	{
		m_pKnob = NULL;
		m_nDlgCode = DLGC_WANTARROWS | DLGC_WANTCHARS;
		EnableTooltip();
	}

	virtual ~CBCGPKnobCtrl()
	{
		if (m_pKnob != NULL)
		{
			delete m_pKnob;
		}
	}

	virtual CBCGPKnob* GetKnob()
	{
		if (m_pKnob == NULL)
		{
			m_pKnob = new CBCGPKnob();
		}

		return m_pKnob;
	}

protected:
	virtual CBCGPBaseVisualObject* GetVisualObject()
	{
		return GetKnob();
	}

protected:
	CBCGPKnob*	m_pKnob;
};

#endif // !defined(AFX_BCGPKNOB_H__E0223345_9C90_461A_9C67_F97F1F3A39B4__INCLUDED_)
