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
// BCGPGaugeImpl.h: interface for the CBCGPGaugeImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPGAUGEIMPL_H__3E99A0DC_6DEA_4C69_822D_D79E636EC4ED__INCLUDED_)
#define AFX_BCGPGAUGEIMPL_H__3E99A0DC_6DEA_4C69_822D_D79E636EC4ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXMT_H__
	#include <afxmt.h>
#endif

#include "BCGCBPro.h"
#include "BCGPGraphicsManager.h"
#include "BCGPVisualContainer.h"

// Notifications
extern BCGCBPRODLLEXPORT UINT BCGM_ON_GAUGE_START_TRACK;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_GAUGE_TRACK;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_GAUGE_FINISH_TRACK;
extern BCGCBPRODLLEXPORT UINT BCGM_ON_GAUGE_CANCEL_TRACK;

class CBCGPGaugeImpl;

struct BCGCBPRODLLEXPORT CBCGPGaugeTrackingData
{
	CBCGPGaugeTrackingData()
	{
		m_nMessage = 0;
		m_Value = 0.;
		m_nScale = 0;
		m_pGauge = NULL;
	}

	UINT			m_nMessage;
	CBCGPPoint		m_Point;
	double			m_Value;
	int				m_nScale;
	CBCGPGaugeImpl*	m_pGauge;
};

class BCGCBPRODLLEXPORT CBCGPGaugeScaleObject : public CObject
{
	DECLARE_DYNCREATE(CBCGPGaugeScaleObject)

	friend class CBCGPGaugeImpl;
	friend class CBCGPLinearGaugeImpl;

// Construction
public:
	enum BCGP_TICKMARK_STYLE
	{
		BCGP_TICKMARK_STYLE_FIRST  = 0,
		BCGP_TICKMARK_LINE         = BCGP_TICKMARK_STYLE_FIRST,
		BCGP_TICKMARK_TRIANGLE     = 1,
		BCGP_TICKMARK_BOX          = 2,
		BCGP_TICKMARK_CIRCLE       = 3,
		BCGP_TICKMARK_TRIANGLE_INV = 4,
		BCGP_TICKMARK_STYLE_LAST   = BCGP_TICKMARK_TRIANGLE_INV
	};

	enum BCGP_TICKMARK_POSITION
	{
		BCGP_TICKMARK_POSITION_FIRST  = 0,
		BCGP_TICKMARK_POSITION_NEAR   = BCGP_TICKMARK_POSITION_FIRST,
		BCGP_TICKMARK_POSITION_CENTER = 1,
		BCGP_TICKMARK_POSITION_FAR    = 2,
		BCGP_TICKMARK_POSITION_LAST   = BCGP_TICKMARK_POSITION_FAR
	};

	CBCGPGaugeScaleObject()
	{
		m_dblStart = 0.;
		m_dblFinish = 100.;
		m_dblStep = 5.;
		m_dblOffsetFromFrame = 5.;
		m_dblMinorTickMarkSize = 5.;
		m_dblMajorTickMarkSize = 10.;
		m_dblMajorTickMarkStep = 2;
		m_strLabelFormat = _T("%.0f");
		m_MinorTickMarkStyle = m_MajorTickMarkStyle = BCGP_TICKMARK_LINE;
		m_MinorTickMarkPosition = BCGP_TICKMARK_POSITION_NEAR;
		m_pParentGauge = NULL;
	}

	CBCGPGaugeScaleObject(const CBCGPGaugeScaleObject & src)
	{
		CopyFrom(src);
	}

	CBCGPGaugeScaleObject & operator = (const CBCGPGaugeScaleObject & src)
	{
		CopyFrom(src);
		return *this;
	}

	const CBCGPGaugeImpl* GetParentGauge() const
	{
		return m_pParentGauge;
	}

	virtual CBCGPGaugeScaleObject* CreateCopy() const
	{
		return new CBCGPGaugeScaleObject(*this);
	}

	virtual void CopyFrom(const CBCGPGaugeScaleObject & src)
	{
		m_dblStart = src.m_dblStart;
		m_dblFinish = src.m_dblFinish;
		m_dblStep = src.m_dblStep;
		m_dblOffsetFromFrame = src.m_dblOffsetFromFrame;
		m_strLabelFormat = src.m_strLabelFormat;
		m_dblMinorTickMarkSize = src.m_dblMinorTickMarkSize;
		m_dblMajorTickMarkSize = src.m_dblMajorTickMarkSize;
		m_dblMajorTickMarkStep = src.m_dblMajorTickMarkStep;
		m_brFill = src.m_brFill;
		m_brOutline = src.m_brOutline;
		m_brText = src.m_brText;
		m_brTickMarkMajor = src.m_brTickMarkMajor;
		m_brTickMarkMinor = src.m_brTickMarkMinor;
		m_brTickMarkMajorOutline = src.m_brTickMarkMajorOutline;
		m_brTickMarkMinorOutline = src.m_brTickMarkMinorOutline;
		m_MinorTickMarkStyle = src.m_MinorTickMarkStyle;
		m_MajorTickMarkStyle = src.m_MajorTickMarkStyle;
		m_MinorTickMarkPosition = src.m_MinorTickMarkPosition;
		m_pParentGauge = src.m_pParentGauge;
	}

	virtual void CleanUp()
	{
		m_Shape.Clear();
		m_Shape.Destroy();
	}

	double GetStart() const
	{
		return m_dblStart;
	}

	double GetFinish() const
	{
		return m_dblFinish;
	}

protected:
	void SetParentGauge(CBCGPGaugeImpl* pParentGauge)
	{
		m_pParentGauge = pParentGauge;
	}

// Attributes
protected:
	double					m_dblStart;
	double					m_dblFinish;
	double					m_dblStep;
	double					m_dblOffsetFromFrame;
	CString					m_strLabelFormat;
	double					m_dblMinorTickMarkSize;
	double					m_dblMajorTickMarkSize;
	double					m_dblMajorTickMarkStep;
	CBCGPBrush				m_brFill;
	CBCGPBrush				m_brOutline;
	CBCGPBrush				m_brText;
	CBCGPBrush				m_brTickMarkMajor;
	CBCGPBrush				m_brTickMarkMinor;
	CBCGPBrush				m_brTickMarkMajorOutline;
	CBCGPBrush				m_brTickMarkMinorOutline;
	CBCGPComplexGeometry	m_Shape;
	BCGP_TICKMARK_STYLE		m_MinorTickMarkStyle;
	BCGP_TICKMARK_STYLE		m_MajorTickMarkStyle;
	BCGP_TICKMARK_POSITION	m_MinorTickMarkPosition;
	
	CBCGPGaugeImpl*			m_pParentGauge;
};

class BCGCBPRODLLEXPORT CBCGPGaugeDataObject : public CBCGPVisualDataObject
{
	DECLARE_DYNCREATE(CBCGPGaugeDataObject)

// Construction
public:
	CBCGPGaugeDataObject()
	{
		m_nScale = 0;
	}

	CBCGPGaugeDataObject(const CBCGPGaugeDataObject& src)
	{
		CopyFrom(src);
	}

	virtual CBCGPVisualDataObject* CreateCopy () const
	{
		return new CBCGPGaugeDataObject(*this);
	}

	virtual void CopyFrom(const CBCGPVisualDataObject& srcIn)
	{
		CBCGPVisualDataObject::CopyFrom(srcIn);

		const CBCGPGaugeDataObject& src = (const CBCGPGaugeDataObject&)srcIn;

		m_nScale = src.m_nScale;
		m_brFill = src.m_brFill;
		m_brOutline = src.m_brOutline;
	}

	virtual BOOL IsCircleShape() const
	{
		return FALSE;
	}

// Attributes
public:
	int GetScale() const
	{
		return m_nScale;
	}

	double GetValue() const
	{
		return m_dblValue;
	}

	const CBCGPBrush& GetFillBrush() const
	{
		return m_brFill;
	}

	const CBCGPBrush& GetOutlineBrush() const
	{
		return m_brOutline;
	}

protected:
	int			m_nScale;
	CBCGPBrush	m_brFill;
	CBCGPBrush	m_brOutline;
};

class BCGCBPRODLLEXPORT CBCGPGaugeColoredRangeObject : public CObject
{
	friend class CBCGPGaugeImpl;
	friend class CBCGPCircularGaugeImpl;
	friend class CBCGPLinearGaugeImpl;

	DECLARE_DYNCREATE(CBCGPGaugeColoredRangeObject)

// Construction
public:
	CBCGPGaugeColoredRangeObject()
	{
		m_nScale = 0;
		m_dblStartValue = 0.;
		m_dblFinishValue = 0.;
		m_dblStartWidth = 0.;
		m_dblFinishWidth = 0.;
		m_dblOffsetFromFrame = 0.;
		m_pParentGauge = NULL;
	}

	CBCGPGaugeColoredRangeObject(const CBCGPGaugeColoredRangeObject& src)
	{
		CopyFrom(src);
	}

	CBCGPGaugeColoredRangeObject(
		double dblStartValue, double dblFinishValue,
		const CBCGPBrush& brFill, const CBCGPBrush& brFrame,
		int nScale = 0,
		double dblStartWidth = 0.,
		double dblFinishWidth = 0.,
		double dblOffsetFromFrame = 0.,
		const CBCGPBrush& brTextLabel = CBCGPBrush(), 
		const CBCGPBrush& brTickMarkOutline = CBCGPBrush(),
		const CBCGPBrush& brTickMarkFill = CBCGPBrush())
	{
		m_dblStartValue = dblStartValue;
		m_dblFinishValue = dblFinishValue;
		m_nScale = nScale;
		m_dblStartWidth = dblStartWidth;
		m_dblFinishWidth = dblFinishWidth;
		m_dblOffsetFromFrame = dblOffsetFromFrame;
		m_brFill = brFill;
		m_brOutline = brFrame;
		m_brTickMarkFill = brTickMarkFill;
		m_brTickMarkOutline = brTickMarkOutline;
		m_brTextLabel = brTextLabel;
		m_pParentGauge = NULL;
	}

	const CBCGPGaugeImpl* GetParentGauge() const
	{
		return m_pParentGauge;
	}

	CBCGPGaugeColoredRangeObject& operator = (const CBCGPGaugeColoredRangeObject& src)
	{
		CopyFrom(src);
		return *this;
	}

	virtual CBCGPGaugeColoredRangeObject* CreateCopy() const
	{
		return new CBCGPGaugeColoredRangeObject(*this);
	}
	
	virtual void CopyFrom(const CBCGPGaugeColoredRangeObject& src)
	{
		m_nScale = src.m_nScale;
		m_dblStartValue = src.m_dblStartValue;
		m_dblFinishValue = src.m_dblFinishValue;
		m_brFill = src.m_brFill;
		m_brOutline = src.m_brOutline;
		m_brTickMarkFill = src.m_brTickMarkFill;
		m_brTickMarkOutline = src.m_brTickMarkOutline;
		m_brTextLabel = src.m_brTextLabel;
		m_dblStartWidth = src.m_dblStartWidth;
		m_dblFinishWidth = src.m_dblFinishWidth;
		m_dblOffsetFromFrame = src.m_dblOffsetFromFrame;
		m_pParentGauge = src.m_pParentGauge;
	}

	int GetScale() const
	{
		return m_nScale;
	}

	double GetStartValue() const
	{
		return m_dblStartValue;
	}

	double GetFinishValue() const
	{
		return m_dblFinishValue;
	}

	void SetRange(double dblStart, double dblFinish)
	{
		m_dblStartValue = dblStart;
		m_dblFinishValue = dblFinish;
	}
	
	void SetFillBrush(const CBCGPBrush& brFill)
	{
		m_brFill = brFill;
	}

	const CBCGPBrush& GetFillBrush() const
	{
		return m_brFill;
	}

	void SetOutlineBrush(const CBCGPBrush& brOutline)
	{
		m_brOutline = brOutline;
	}

	const CBCGPBrush& GetOutlineBrush() const
	{
		return m_brOutline;
	}

	const CBCGPBrush& GetTickMarkFillBrush() const
	{
		return m_brTickMarkFill;
	}

	const CBCGPBrush& GetTickMarkOutlineBrush() const
	{
		return m_brTickMarkOutline;
	}

	const CBCGPBrush& GetTextLabelBrush() const
	{
		return m_brTextLabel;
	}

	double GetStartWidth() const
	{
		return m_dblStartWidth;
	}

	double GetFinishWidth() const
	{
		return m_dblFinishWidth;
	}

	void SetWidthRange(double dblStart, double dblFinish)
	{
		m_dblStartWidth = dblStart;
		m_dblFinishWidth = dblFinish;
	}

	double GetOffsetFromFrame () const
	{
		return m_dblOffsetFromFrame;
	}

	void SetOffsetFromFrame (double dblOffsetFromFrame)
	{
		m_dblOffsetFromFrame = dblOffsetFromFrame;
	}

protected:
	void SetParentGauge(CBCGPGaugeImpl* pParentGauge)
	{
		m_pParentGauge = pParentGauge;
	}

// Attributes
protected:
	int						m_nScale;
	double					m_dblStartValue;
	double					m_dblFinishValue;
	CBCGPBrush				m_brFill;
	CBCGPBrush				m_brOutline;
	CBCGPBrush				m_brTickMarkFill;
	CBCGPBrush				m_brTickMarkOutline;
	CBCGPBrush				m_brTextLabel;
	CBCGPComplexGeometry	m_Shape;
	double					m_dblStartWidth;
	double					m_dblFinishWidth;
	double					m_dblOffsetFromFrame;

	CBCGPGaugeImpl*			m_pParentGauge;
};

#define BCGP_GAUGE_RANGE_TEXT_COLOR			0x0001
#define BCGP_GAUGE_RANGE_TICKMARK_COLOR		0x0002

class BCGCBPRODLLEXPORT CBCGPGaugeImpl : public CBCGPBaseVisualObject  
{
	friend class CBCGPCircularGaugeImpl;
	friend class CBCGPLinearGaugeImpl;

	DECLARE_DYNAMIC(CBCGPGaugeImpl)

public:
	enum BCGP_SUB_GAUGE_POS
	{
		BCGP_SUB_GAUGE_POS_FIRST = 0,
		BCGP_SUB_GAUGE_NONE      = BCGP_SUB_GAUGE_POS_FIRST,
		BCGP_SUB_GAUGE_TOP       = 1,
		BCGP_SUB_GAUGE_BOTTOM    = 2,
		BCGP_SUB_GAUGE_LEFT      = 3,
		BCGP_SUB_GAUGE_RIGHT     = 4,
		BCGP_SUB_GAUGE_POS_LAST  = BCGP_SUB_GAUGE_RIGHT
	};

	CBCGPGaugeImpl(CBCGPVisualContainer* pContainer = NULL);
	virtual ~CBCGPGaugeImpl();

// Attributes:
public:
	// Frame attributes:
	int GetFrameSize() const
	{
		return m_nFrameSize;
	}

	void SetFrameSize(int nFrameSize)
	{
		m_nFrameSize = nFrameSize;
		SetDirty();
	}

	// Interactive mode:
	void SetInteractiveMode(BOOL bSet = TRUE)
	{
		m_bIsInteractiveMode = bSet;
	}

	BOOL IsInteractiveMode() const
	{
		return m_bIsInteractiveMode;
	}

	// Scale attributes:
	virtual void SetRange(double dblStart, double dblFinish, int nScale = 0);
	double GetStart(int nScale = 0) const;
	double GetFinish(int nScale = 0) const;

	void SetTickMarkStyle(CBCGPGaugeScaleObject::BCGP_TICKMARK_STYLE style, BOOL bIsMajor, double dblSize = -1., int nScale = 0);
	CBCGPGaugeScaleObject::BCGP_TICKMARK_STYLE GetTickMarkStyle(BOOL bIsMajor, int nScale = 0) const;

	void SetMinorTickMarkPosition(CBCGPGaugeScaleObject::BCGP_TICKMARK_POSITION pos, int nScale = 0);
	CBCGPGaugeScaleObject::BCGP_TICKMARK_POSITION GetMinorTickMarkPosition(int nScale = 0) const;

	void SetScaleOffsetFromFrame(double dblOffsetFromFrame, int nScale = 0);
	double GetScaleOffsetFromFrame(int nScale = 0) const;

	virtual BOOL SetValue(double dblVal, int nIndex = 0, UINT uiAnimationTime = 100 /* Ms, 0 - no animation */, BOOL bRedraw = FALSE);

	void SetStep(double dblStep, int nScale = 0);
	double GetStep(int nScale = 0) const;

	void SetTextLabelFormat(const CString& strFormat, int nScale = 0);
	CString GetTextLabelFormat(int nScale = 0) const;

	double GetTickMarkSize(BOOL bMajor, int nScale = 0) const;
	void SetTickMarkSize(double dblSize, BOOL bMajor, int nScale = 0);

	void SetMajorTickMarkStep(double dblMajorTickMarkStep, int nScale = 0);
	double GetMajorTickMarkStep(int nScale = 0) const;

	void SetScaleFillBrush(const CBCGPBrush& brFill, int nScale = 0);
	CBCGPBrush GetScaleFillBrush(int nScale = 0) const;

	void SetScaleOutlineBrush(const CBCGPBrush& brOutline, int nScale = 0);
	CBCGPBrush GetScaleOutlineBrush(int nScale = 0) const;

	void SetScaleTextBrush(const CBCGPBrush& brText, int nScale = 0);
	CBCGPBrush GetScaleTextBrush(int nScale = 0) const;

	void SetScaleTickMarkBrush(const CBCGPBrush& brTickMark, BOOL bMajor, int nScale = 0);
	CBCGPBrush GetScaleTickMarkBrush(BOOL bMajor, int nScale = 0) const;
	void SetScaleTickMarkOutlineBrush(const CBCGPBrush& brTickMark, BOOL bMajor, int nScale = 0);
	CBCGPBrush GetScaleTickMarkOutlineBrush(BOOL bMajor, int nScale = 0) const;

	BCGP_SUB_GAUGE_POS GetPos() const
	{
		return m_Pos;
	}
	void SetPos(BCGP_SUB_GAUGE_POS pos)
	{
		m_Pos = pos;
	}

	const CBCGPPoint& GetOffset() const
	{
		return m_ptOffset;
	}

	void SetOffset(const CBCGPPoint& ptOffset)
	{
		m_ptOffset = ptOffset;
	}

	void SetToolTip(LPCTSTR lpszToolTip, LPCTSTR lpszDescr = NULL);
	const CString& GetToolTip () const {	return m_strToolTip;	}
	const CString& GetDescription () const {	return m_strDescription;	}

// Operations:
public:
	// Scales:
	CBCGPGaugeScaleObject* GetScale(int nScale) const;

	virtual BOOL RemoveScale(int nIndex, BOOL bRedraw = FALSE);

	int GetScalesCount() const
	{
		return (int)m_arScales.GetSize();
	}

	// Colored ranges:
	int AddColoredRange(double dblStartValue, double dblFinishValue,
		const CBCGPBrush& brFill, const CBCGPBrush& brFrame,
		int nScale = 0,
		double dblStartWidth = 0.,
		double dblFinishWidth = 0.,
		double dblOffsetFromFrame = 0.,
		const CBCGPBrush& brTextLabel = CBCGPBrush(), 
		const CBCGPBrush& brTickMarkOutline = CBCGPBrush(), const CBCGPBrush& brTickMarkFill = CBCGPBrush(),
		BOOL bRedraw = FALSE);

	int AddColoredRange(CBCGPGaugeColoredRangeObject* pRange, BOOL bRedraw = FALSE);

	BOOL ModifyColoredRange(int nIndex, const CBCGPGaugeColoredRangeObject& range, BOOL bRedraw = FALSE);

	CBCGPGaugeColoredRangeObject* GetColoredRange(int nIndex)
	{
		if (nIndex < 0 || nIndex >= m_arRanges.GetSize())
		{
			ASSERT(FALSE);
			return NULL;
		}

		return m_arRanges[nIndex];
	}

	BOOL RemoveColoredRange(int nIndex, BOOL bRedraw = FALSE);

	int GetColoredRangesCount() const
	{
		return (int)m_arRanges.GetSize();
	}

	void RemoveAllColoredRanges(BOOL bRedraw = FALSE);
	void RemoveAllScales(BOOL bRedraw = FALSE);

	const CBCGPGaugeColoredRangeObject* GetColoredRangeByValue(double dblVal, int nScale = 0, DWORD dwFlags = 0) const;

public:
	virtual CBCGPSize GetDefaultSize(CBCGPGraphicsManager* pGM, const CBCGPBaseVisualObject* pParentGauge) = 0;
	virtual void CopyFrom(const CBCGPBaseVisualObject& src);

protected:
	virtual double GetAnimationRange(int nIndex);
	virtual BOOL ReposAllSubGauges(CBCGPGraphicsManager* /*pGM*/)
	{
		return FALSE;
	}

	BOOL FireTrackEvent(UINT nMessage, double& dblValue, const CBCGPPoint& pt);

// Overrides:
public:
	virtual void GetTickMarkLabel(CString& strLabel, const CString& strLabelFormat, double dblVal, int /*nScale*/)
	{
		strLabel.Format(strLabelFormat, dblVal);
	}

	virtual CBCGPSize GetTickMarkTextLabelSize(CBCGPGraphicsManager* pGM, const CString& strLabel, const CBCGPTextFormat& tf);
	virtual void OnDrawTickMarkTextLabel(CBCGPGraphicsManager* pGM, const CBCGPTextFormat& tf, const CBCGPRect& rectText, const CString& strLabel, double dblVal, int nScale, const CBCGPBrush& br);

	virtual BOOL HitTestValue(const CBCGPPoint& /*pt*/, double& /*dblValue*/, int /*nScale*/ = 0, BOOL /*bInsideGauge*/ = TRUE) const
	{
		return FALSE;
	}

	virtual CBCGPBaseVisualObject* CreateCopy () const;

protected:
	virtual BOOL OnMouseDown(int nButton, const CBCGPPoint& pt);
	virtual void OnMouseUp(int nButton, const CBCGPPoint& pt);
	virtual void OnMouseMove(const CBCGPPoint& pt);
	virtual void OnCancelMode();
	virtual BOOL OnKeyboardDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	virtual BOOL OnGetToolTip(const CBCGPPoint& pt, CString& strToolTip, CString& strDescr);

	virtual BOOL IsCaptured() const	
	{	
		return m_bIsInteractiveMode && m_nTrackingScale >= 0;
	}

	virtual HRESULT get_accName(VARIANT varChild, BSTR *pszName)
	{
		if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
		{
			CString strName = GetName();
			if (strName.IsEmpty())
			{
				strName = m_strToolTip;
			}
			
			if (!strName.IsEmpty())
			{
				*pszName = strName.AllocSysString();
				return S_OK;
			}
		}
		
		return S_FALSE;
	}

protected:
	int					m_nFrameSize;
	BCGP_SUB_GAUGE_POS	m_Pos;
	BOOL				m_bIsSubGauge;
	CBCGPPoint			m_ptOffset;
	int					m_nTrackingScale;
	CBCGPBrush			m_brShadow;
	CString				m_strToolTip;
	CString				m_strDescription;

	CArray<CBCGPGaugeScaleObject*, CBCGPGaugeScaleObject*>					m_arScales;
	CArray<CBCGPGaugeColoredRangeObject*, CBCGPGaugeColoredRangeObject*>	m_arRanges;
};

#endif // !defined(AFX_BCGPGAUGEIMPL_H__3E99A0DC_6DEA_4C69_822D_D79E636EC4ED__INCLUDED_)
