//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2010 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
 //*******************************************************************************
//
// BCGPGaugeInfo.h: interface for the CBCGPCircularGaugeImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPGAUGEINFO_H__38DF9C07_6F92_433D_B7D0_2CBB09EDD9D6__INCLUDED_)
#define AFX_BCGPGAUGEINFO_H__38DF9C07_6F92_433D_B7D0_2CBB09EDD9D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bcgcbpro.h"
#include "BCGPBaseInfo.h"

#include "BCGPCircularGaugeImpl.h"
#include "BCGPNumericIndicatorImpl.h"
#include "BCGPColorIndicatorImpl.h"

class CBCGPGaugeInfo: public CBCGPBaseInfo  
{
public:
	static CBCGPBaseInfo::XBase* CreateBaseFromName (const CString& name);
	static CBCGPBaseInfo::XBase* CreateBaseFromTag (const CString& tag);

	class XScale
	{
	public:
		XScale ();
		virtual ~XScale ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		double			m_dblStart;
		double			m_dblFinish;
		double			m_dblStep;
		CString			m_strLabelFormat;
		double			m_dblMajorTickMarkStep;
		double			m_dblMinorTickMarkSize;
		double			m_dblMajorTickMarkSize;
		CBCGPBrush		m_brFill;
		CBCGPBrush		m_brOutline;
		CBCGPBrush		m_brText;
		CBCGPBrush		m_brTickMarkMajor;
		CBCGPBrush		m_brTickMarkMinor;
	};
	typedef CArray<XScale*, XScale*> XArrayScale;

	class XData
	{
	public:
		XData ();
		virtual ~XData ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		int			m_nScale;
		double		m_dblValue;
		CBCGPBrush	m_brFill;
		CBCGPBrush	m_brOutline;
	};
	typedef CArray<XData*, XData*> XArrayData;

	class XColoredRange
	{
	public:
		XColoredRange ();
		virtual ~XColoredRange ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		int			m_nScale;
		double		m_dblStartValue;
		double		m_dblFinishValue;
		CBCGPBrush	m_brFill;
		CBCGPBrush	m_brOutline;
		CBCGPBrush	m_brTickMark;
		CBCGPBrush	m_brTextLabel;
	};
	typedef CArray<XColoredRange*, XColoredRange*> XArrayColoredRange;

	class XElement: public CBCGPBaseInfo::XBase
	{
	protected:
		XElement(const CString& strElementName);

	public:
		virtual ~XElement();

		static XElement* CreateFromName (const CString& name);

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		virtual BOOL ColorsFromTag (const CString& strTag) = 0;
		virtual void ColorsToTag (CString& strTag) const = 0;

	public:
		XID				m_ID;
		CRect			m_Rect;
		double			m_dblValue;
		int				m_nBorderSize;
		BOOL			m_bIsVisible;
	};
	typedef CArray<XElement*, XElement*> XArrayElement;

	class XCircularScale: public XScale
	{
	public:
		XCircularScale ();
		virtual ~XCircularScale ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		double			m_dblStartAngle;
		double			m_dblFinishAngle;
		double			m_dblOffsetFromBorder;
	};

	class XCircularPointer: public XData
	{
	public:
		XCircularPointer ();
		virtual ~XCircularPointer ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		double		m_dblSize;
	};

	class XCircularColoredRange: public XColoredRange
	{
	public:
		XCircularColoredRange ();
		virtual ~XCircularColoredRange ();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		double		m_dblStartWidth;
		double		m_dblFinishWidth;
		double		m_dblOffsetFromBorder;
	};

	class XElementCircular: public XElement
	{
	public:
		XElementCircular();
		virtual ~XElementCircular();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		virtual BOOL ColorsFromTag (const CString& strTag);
		virtual void ColorsToTag (CString& strTag) const;

	public:
		CBCGPCircularGaugeColors	m_Colors;
		double						m_dblCapSize;
		BOOL						m_bShapeByTicksArea;
		XArrayScale					m_arScales;
		XArrayData					m_arPointers;
		XArrayColoredRange			m_arRanges;
	};

	class XElementNumeric: public XElement
	{
	public:
		XElementNumeric();
		virtual ~XElementNumeric();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		virtual BOOL ColorsFromTag (const CString& strTag);
		virtual void ColorsToTag (CString& strTag) const;

	public:
		CBCGPNumericIndicatorColors	m_Colors;
		CBCGPNumericIndicatorImpl::BCGPNumericIndicatorStyle
									m_Style;
		int							m_nDigits;
		int							m_nDecimals;
		int							m_nSeparatorWidth;
		BOOL						m_bDrawDecimalPoint;
		BOOL						m_bDrawLeadingZeros;
	};

	class XElementColor: public XElement
	{
	public:
		XElementColor();
		virtual ~XElementColor();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	protected:
		virtual BOOL ColorsFromTag (const CString& strTag);
		virtual void ColorsToTag (CString& strTag) const;

	public:
		CBCGPColorIndicatorColors	m_Colors;
		CBCGPColorIndicatorImpl::BCGPColorIndicatorStyle
									m_Style;
	};

	class XDashboard: public CBCGPBaseInfo::XBase
	{
	public:
		XDashboard();
		virtual ~XDashboard();

		virtual BOOL FromTag (const CString& strTag);
		virtual void ToTag (CString& strTag) const;

	public:
		CRect			m_Rect;
		XArrayElement	m_arElements;
	};

public:
	CBCGPGaugeInfo();
	virtual ~CBCGPGaugeInfo();

	virtual BOOL FromTag (const CString& strTag);
	virtual void ToTag (CString& strTag) const;

	inline XDashboard& GetDashboard ()
	{
		return m_Dashboard;
	}
	inline const XDashboard& GetDashboard () const
	{
		return m_Dashboard;
	}

public:
	static LPCTSTR s_szCircularGauge;
	static LPCTSTR s_szNumericInd;
	static LPCTSTR s_szColorInd;
	static LPCTSTR s_szDashboard;

private:
	XDashboard	m_Dashboard;
};

class CBCGPGaugeInfoLoader: public CBCGPBaseInfoLoader
{
public:
	CBCGPGaugeInfoLoader (CBCGPGaugeInfo& info);
	virtual ~CBCGPGaugeInfoLoader();

protected:
	CBCGPGaugeInfo& GetGaugeInfo () const
	{
		return (CBCGPGaugeInfo&)GetInfo ();
	}
};

class CBCGPGaugeInfoWriter: public CBCGPBaseInfoWriter
{
public:
	CBCGPGaugeInfoWriter(CBCGPGaugeInfo& info);
	virtual ~CBCGPGaugeInfoWriter();

	virtual BOOL Save (const CString& strFileName);

protected:
	CBCGPGaugeInfo& GetGaugeInfo ()
	{
		return (CBCGPGaugeInfo&)GetInfo ();
	}
};

#endif // !defined(AFX_BCGPGAUGEINFO_H__38DF9C07_6F92_433D_B7D0_2CBB09EDD9D6__INCLUDED_)
