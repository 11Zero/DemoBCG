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
// BCGPGaugeInfo.cpp: implementation of the CBCGPDashboard class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPGaugeInfo.h"
#include "BCGPTagManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const DWORD c_dwVersion = MAKELONG (1, 0);

static LPCTSTR s_szTag_Body                   = _T("BCGP_DASHBOARD");

static LPCTSTR s_szTag_Rect                   = _T("RECT");
static LPCTSTR s_szTag_BorderSize             = _T("BORDER_SIZE");
static LPCTSTR s_szTag_LabelFormat            = _T("LABEL_FORMAT");
static LPCTSTR s_szTag_Visible                = _T("VISIBLE");

static LPCTSTR s_szTag_Scales                 = _T("SCALES");
static LPCTSTR s_szTag_Scale                  = _T("SCALE");
static LPCTSTR s_szTag_Pointers               = _T("POINTERS");
static LPCTSTR s_szTag_Pointer                = _T("POINTER");
static LPCTSTR s_szTag_Ranges                 = _T("RANGES");
static LPCTSTR s_szTag_Range                  = _T("RANGE");
static LPCTSTR s_szTag_Start                  = _T("START");
static LPCTSTR s_szTag_Finish                 = _T("FINISH");
static LPCTSTR s_szTag_Start_Angle            = _T("START_ANGLE");
static LPCTSTR s_szTag_Finish_Angle           = _T("FINISH_ANGLE");
static LPCTSTR s_szTag_Start_Value            = _T("START_VALUE");
static LPCTSTR s_szTag_Finish_Value           = _T("FINISH_VALUE");
static LPCTSTR s_szTag_Start_Width            = _T("START_WIDTH");
static LPCTSTR s_szTag_Finish_Width           = _T("FINISH_WIDTH");
static LPCTSTR s_szTag_Step                   = _T("STEP");
static LPCTSTR s_szTag_Value                  = _T("VALUE");
static LPCTSTR s_szTag_Size                   = _T("SIZE");

static LPCTSTR s_szTag_Dashboard              = _T("DASHBOARD");
static LPCTSTR s_szTag_Element                = _T("ELEMENT");
static LPCTSTR s_szTag_Elements               = _T("ELEMENTS");

static LPCTSTR s_szTag_Colors                 = _T("COLORS");
static LPCTSTR s_szTag_Border_Outline         = _T("BORDER_OUTLINE");
static LPCTSTR s_szTag_Border_Brush           = _T("BORDER_BRUSH");
static LPCTSTR s_szTag_Fill_Outline           = _T("FILL_OUTLINE");
static LPCTSTR s_szTag_Fill_Brush             = _T("FILL_BRUSH");
static LPCTSTR s_szTag_Pointer_Outline        = _T("POINTER_OUTLINE");
static LPCTSTR s_szTag_Pointer_Brush          = _T("POINTER_BRUSH");
static LPCTSTR s_szTag_Cap_Outline            = _T("CAP_OUTLINE");
static LPCTSTR s_szTag_Cap_Brush              = _T("CAP_BRUSH");
static LPCTSTR s_szTag_Text_Brush             = _T("TEXT_BRUSH");
static LPCTSTR s_szTag_TickMark_Brush         = _T("TICKMARK_BRUSH");
static LPCTSTR s_szTag_TickMarkMinor_Brush    = _T("TICKMARK_MINOR_BRUSH");
static LPCTSTR s_szTag_TickMarkMajor_Brush    = _T("TICKMARK_MAJOR_BRUSH");
static LPCTSTR s_szTag_Separator_Brush        = _T("SEPARATOR_BRUSH");
static LPCTSTR s_szTag_Digit_Brush            = _T("DIGIT_BRUSH");
static LPCTSTR s_szTag_Decimal_Brush          = _T("DECIMAL_BRUSH");
static LPCTSTR s_szTag_Sign_Brush             = _T("SIGN_BRUSH");
static LPCTSTR s_szTag_Dot_Brush              = _T("DOT_BRUSH");
static LPCTSTR s_szTag_MajorTickMarkStep      = _T("MAJOR_TICKMARK_STEP");
static LPCTSTR s_szTag_MinorTickMarkSize      = _T("MINOR_TICKMARK_SIZE");
static LPCTSTR s_szTag_MajorTickMarkSize      = _T("MAJOR_TICKMARK_SIZE");
static LPCTSTR s_szTag_CapSize                = _T("CAP_SIZE");
static LPCTSTR s_szTag_Shape_Area             = _T("SHAPE_AREA");
static LPCTSTR s_szTag_Offset_Border          = _T("OFFSET_BORDER");

static LPCTSTR s_szTag_Style                  = _T("STYLE");
static LPCTSTR s_szTag_Digits                 = _T("DIGITS");
static LPCTSTR s_szTag_Decimal                = _T("DECIMAL");
static LPCTSTR s_szTag_SeparatorWidth         = _T("SEPARATOR_WIDTH");
static LPCTSTR s_szTag_DrawDecimalPoint       = _T("DECIMAL_POINT");
static LPCTSTR s_szTag_DrawLeadingZeros       = _T("LEADING_ZEROS");

LPCTSTR CBCGPGaugeInfo::s_szCircularGauge     = _T("CircularGauge");
LPCTSTR CBCGPGaugeInfo::s_szNumericInd        = _T("NumericIndicator");
LPCTSTR CBCGPGaugeInfo::s_szColorInd          = _T("ColorIndicator");
LPCTSTR CBCGPGaugeInfo::s_szDashboard         = _T("Dashboard");

CBCGPGaugeInfo::XScale::XScale ()
	: m_dblStart            (0.0)
	, m_dblFinish           (100.0)
	, m_dblStep             (5.0)
	, m_strLabelFormat      (_T("%.0f"))
	, m_dblMajorTickMarkStep(2)
	, m_dblMinorTickMarkSize(5)
	, m_dblMajorTickMarkSize(10)
{
}

CBCGPGaugeInfo::XScale::~XScale ()
{
}

BOOL CBCGPGaugeInfo::XScale::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadDouble (s_szTag_Start, m_dblStart);
	tm.ReadDouble (s_szTag_Finish, m_dblFinish);
	tm.ReadDouble (s_szTag_Step, m_dblStep);
	tm.ReadString (s_szTag_LabelFormat, m_strLabelFormat);
	CBCGPTagManager::Entity_FromTag (m_strLabelFormat);
	tm.ReadDouble (s_szTag_MajorTickMarkStep, m_dblMajorTickMarkStep);
	tm.ReadDouble (s_szTag_MinorTickMarkSize, m_dblMinorTickMarkSize);
	tm.ReadDouble (s_szTag_MajorTickMarkSize, m_dblMajorTickMarkSize);
	tm.ReadBrush (s_szTag_Fill_Brush, m_brFill);
	tm.ReadBrush (s_szTag_Fill_Outline, m_brOutline);
	tm.ReadBrush (s_szTag_Text_Brush, m_brText);
	tm.ReadBrush (s_szTag_TickMarkMinor_Brush, m_brTickMarkMinor);
	tm.ReadBrush (s_szTag_TickMarkMajor_Brush, m_brTickMarkMajor);

	return TRUE;
}

void CBCGPGaugeInfo::XScale::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Start, m_dblStart, m_dblStart + 1.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Finish, m_dblFinish, m_dblFinish + 1.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Step, m_dblStep, m_dblStep + 1.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteString (s_szTag_LabelFormat, CBCGPTagManager::Entity_ToTag (m_strLabelFormat), _T(""), TRUE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_MajorTickMarkStep, m_dblMajorTickMarkStep, 2));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_MinorTickMarkSize, m_dblMinorTickMarkSize, 5));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_MajorTickMarkSize, m_dblMajorTickMarkSize, 10));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_brFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_brOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Text_Brush, m_brText));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_TickMarkMinor_Brush, m_brTickMarkMinor));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_TickMarkMajor_Brush, m_brTickMarkMajor));
}


CBCGPGaugeInfo::XData::XData ()
	: m_nScale          (0)
	, m_dblValue        (0.0)
{
}

CBCGPGaugeInfo::XData::~XData ()
{
}

BOOL CBCGPGaugeInfo::XData::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadInt (s_szTag_Scale, m_nScale);
 	tm.ReadDouble (s_szTag_Value, m_dblValue);
 	tm.ReadBrush (s_szTag_Fill_Brush, m_brFill);
 	tm.ReadBrush (s_szTag_Fill_Outline, m_brOutline);

	return TRUE;
}

void CBCGPGaugeInfo::XData::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Scale, m_nScale, m_nScale + 1));
 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Value, m_dblValue, m_dblValue + 1.0));
 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_brFill));
 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_brOutline));
}


CBCGPGaugeInfo::XColoredRange::XColoredRange()
	: m_nScale        (0)
	, m_dblStartValue (0.0)
	, m_dblFinishValue(0.0)
{
}

CBCGPGaugeInfo::XColoredRange::~XColoredRange()
{
}

BOOL CBCGPGaugeInfo::XColoredRange::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadInt (s_szTag_Scale, m_nScale);
 	tm.ReadDouble (s_szTag_Start_Value, m_dblStartValue);
	tm.ReadDouble (s_szTag_Finish_Value, m_dblFinishValue);
 	tm.ReadBrush (s_szTag_Fill_Brush, m_brFill);
 	tm.ReadBrush (s_szTag_Fill_Outline, m_brOutline);
	tm.ReadBrush (s_szTag_TickMark_Brush, m_brTickMark);
	tm.ReadBrush (s_szTag_Text_Brush, m_brTextLabel);

	return TRUE;
}

void CBCGPGaugeInfo::XColoredRange::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Scale, m_nScale, m_nScale + 1));
 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Start_Value, m_dblStartValue, m_dblStartValue + 1.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Finish_Value, m_dblFinishValue, m_dblFinishValue + 1.0));
 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_brFill));
 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_brOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_TickMark_Brush, m_brTickMark));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Text_Brush, m_brTextLabel));
}


CBCGPGaugeInfo::XElement::XElement(const CString& strElementName)
	: CBCGPBaseInfo::XBase (strElementName)
	, m_Rect               (0, 0, 0, 0)
	, m_dblValue           (0.0)
	, m_nBorderSize        (6)
	, m_bIsVisible         (TRUE)
{
}

CBCGPGaugeInfo::XElement::~XElement()
{
}

CBCGPGaugeInfo::XCircularScale::XCircularScale()
	: CBCGPGaugeInfo::XScale()
	, m_dblStartAngle       (225.0)
	, m_dblFinishAngle      (-45.0)
	, m_dblOffsetFromBorder (0.0)
{
}

CBCGPGaugeInfo::XCircularScale::~XCircularScale()
{
}

BOOL CBCGPGaugeInfo::XCircularScale::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadDouble (s_szTag_Start_Angle, m_dblStartAngle);
	tm.ReadDouble (s_szTag_Finish_Angle, m_dblFinishAngle);
	tm.ReadDouble (s_szTag_Offset_Border, m_dblOffsetFromBorder);

	return XScale::FromTag (tm.GetBuffer ());
}

void CBCGPGaugeInfo::XCircularScale::ToTag (CString& strTag) const
{
	XScale::ToTag(strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Start_Angle, m_dblStartAngle, 225.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Finish_Angle, m_dblFinishAngle, -45.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Offset_Border, m_dblOffsetFromBorder, 0.0));
}

CBCGPGaugeInfo::XCircularPointer::XCircularPointer()
	: CBCGPGaugeInfo::XData()
	, m_dblSize            (1.0)
{
}

CBCGPGaugeInfo::XCircularPointer::~XCircularPointer()
{
}

BOOL CBCGPGaugeInfo::XCircularPointer::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadDouble (s_szTag_Size, m_dblSize);

	return XData::FromTag (tm.GetBuffer ());
}

void CBCGPGaugeInfo::XCircularPointer::ToTag (CString& strTag) const
{
	XData::ToTag(strTag);

 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Size, m_dblSize, 1.0));
}

CBCGPGaugeInfo::XCircularColoredRange::XCircularColoredRange()
	: CBCGPGaugeInfo::XColoredRange ()
	, m_dblStartWidth               (0.0)
	, m_dblFinishWidth              (0.0)
	, m_dblOffsetFromBorder         (0.0)
{
}

CBCGPGaugeInfo::XCircularColoredRange::~XCircularColoredRange()
{
}

BOOL CBCGPGaugeInfo::XCircularColoredRange::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

 	tm.ReadDouble (s_szTag_Start_Width, m_dblStartWidth);
	tm.ReadDouble (s_szTag_Finish_Width, m_dblFinishWidth);
	tm.ReadDouble (s_szTag_Offset_Border, m_dblOffsetFromBorder);

	return XColoredRange::FromTag (tm.GetBuffer ());
}

void CBCGPGaugeInfo::XCircularColoredRange::ToTag (CString& strTag) const
{
	XColoredRange::ToTag(strTag);

 	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Start_Width, m_dblStartWidth, 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Finish_Width, m_dblFinishWidth, 0.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Offset_Border, m_dblOffsetFromBorder, 0.0));
}

CBCGPGaugeInfo::XElementCircular::XElementCircular()
	: CBCGPGaugeInfo::XElement (CBCGPGaugeInfo::s_szCircularGauge)
	, m_dblCapSize               (20)
	, m_bShapeByTicksArea      (FALSE)
{
}

CBCGPGaugeInfo::XElementCircular::~XElementCircular()
{
	int i = 0;
	for (i = 0; i < m_arScales.GetSize (); i++)
	{
		if (m_arScales[i] != NULL)
		{
			delete m_arScales[i];
		}
	}
	m_arScales.RemoveAll ();

	for (i = 0; i < m_arPointers.GetSize (); i++)
	{
		if (m_arPointers[i] != NULL)
		{
			delete m_arPointers[i];
		}
	}
	m_arPointers.RemoveAll ();

	for (i = 0; i < m_arRanges.GetSize (); i++)
	{
		if (m_arRanges[i] != NULL)
		{
			delete m_arRanges[i];
		}
	}
	m_arRanges.RemoveAll ();
}

CBCGPGaugeInfo::XElementNumeric::XElementNumeric()
	: CBCGPGaugeInfo::XElement (CBCGPGaugeInfo::s_szNumericInd)
	, m_Style                  (CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_CLASSIC)
	, m_nDigits                (6)
	, m_nDecimals              (1)
	, m_nSeparatorWidth        (1)
	, m_bDrawDecimalPoint      (FALSE)
	, m_bDrawLeadingZeros      (TRUE)
{
	m_nBorderSize = 1;
}

CBCGPGaugeInfo::XElementNumeric::~XElementNumeric()
{
}

CBCGPGaugeInfo::XElementColor::XElementColor()
	: CBCGPGaugeInfo::XElement (CBCGPGaugeInfo::s_szColorInd)
	, m_Style                  (CBCGPColorIndicatorImpl::BCGP_COLOR_INDICATOR_ELLIPSE)
{
}

CBCGPGaugeInfo::XElementColor::~XElementColor()
{
}

CBCGPGaugeInfo::XDashboard::XDashboard()
	: CBCGPBaseInfo::XBase (CBCGPGaugeInfo::s_szDashboard)
	, m_Rect               (0, 0, 0, 0)
{
}

CBCGPGaugeInfo::XDashboard::~XDashboard()
{
	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		if (m_arElements[i] != NULL)
		{
			delete m_arElements[i];
		}
	}
	m_arElements.RemoveAll ();
}

CBCGPBaseInfo::XBase* CBCGPGaugeInfo::CreateBaseFromName (const CString& name)
{
	CBCGPBaseInfo::XBase* base = NULL;

	if (name.Compare (CBCGPGaugeInfo::s_szDashboard) == 0)
	{
		base = new CBCGPGaugeInfo::XDashboard;
	}
	else
	{
		base = CBCGPGaugeInfo::XElement::CreateFromName (name);
	}

	return base;
}

CBCGPBaseInfo::XBase* CBCGPGaugeInfo::CreateBaseFromTag (const CString& tag)
{
	CBCGPBaseInfo::XBase* base = NULL;

	CString strElementName;

	{
		CBCGPTagManager tm (tag);
		tm.ReadString (CBCGPBaseInfo::s_szTag_ElementName, strElementName);
	}

	if (!strElementName.IsEmpty ())
	{
		base = CBCGPGaugeInfo::CreateBaseFromName (strElementName);
		if (base != NULL)
		{
			base->FromTag (tag);
		}
	}

	return base;
}

CBCGPGaugeInfo::XElement* CBCGPGaugeInfo::XElement::CreateFromName (const CString& name)
{
	CBCGPGaugeInfo::XElement* element = NULL;

	if (name.Compare (CBCGPGaugeInfo::s_szCircularGauge) == 0)
	{
		element = new CBCGPGaugeInfo::XElementCircular;
	}
	else if (name.Compare (CBCGPGaugeInfo::s_szNumericInd) == 0)
	{
		element = new CBCGPGaugeInfo::XElementNumeric;
	}
	else if (name.Compare (CBCGPGaugeInfo::s_szColorInd) == 0)
	{
		element = new CBCGPGaugeInfo::XElementColor;
	}

	return element;
}

BOOL CBCGPGaugeInfo::XElement::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strID;
	if (tm.ExcludeTag (s_szTag_ID, strID))
	{
		m_ID.FromTag (strID);
	}

	tm.ReadRect (s_szTag_Rect, m_Rect);

	CString strColors;
	if (tm.ExcludeTag (s_szTag_Colors, strColors))
	{
		ColorsFromTag (strColors);
	}

	tm.ReadDouble (s_szTag_Value, m_dblValue);
	tm.ReadInt (s_szTag_BorderSize, m_nBorderSize);
	tm.ReadBool (s_szTag_Visible, m_bIsVisible);

	return TRUE;
}

void CBCGPGaugeInfo::XElement::ToTag (CString& strTag) const
{
	XBase::ToTag (strTag);

	CString strID;
	m_ID.ToTag (strID);
	CBCGPTagManager::WriteItem (strTag, s_szTag_ID, strID);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteRect (s_szTag_Rect, m_Rect, CRect (0, 0, 0, 0)));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_Value, m_dblValue, m_dblValue + 1.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_BorderSize, m_nBorderSize, 6));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Visible, m_bIsVisible, TRUE));

	CString strColors;
	ColorsToTag (strColors);
	CBCGPTagManager::WriteItem (strTag, s_szTag_Colors, strColors);
}

BOOL CBCGPGaugeInfo::XElementCircular::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strScales;
	if (tm.ExcludeTag (s_szTag_Scales, strScales))
	{
		CBCGPTagManager tmScales (strScales);

		CString strScale;
		while (tmScales.ExcludeTag (s_szTag_Scale, strScale))
		{
			XCircularScale* pScale = new XCircularScale;
			if (pScale->FromTag (strScale))
			{
				m_arScales.Add (pScale);
			}
			else
			{
				delete pScale;
			}
		}
	}

	CString strPointers;
	if (tm.ExcludeTag (s_szTag_Pointers, strPointers))
	{
		CBCGPTagManager tmPointers (strPointers);

		CString strPointer;
		while (tmPointers.ExcludeTag (s_szTag_Pointer, strPointer))
		{
			XCircularPointer* pPointer = new XCircularPointer;
			if (pPointer->FromTag (strPointer))
			{
				m_arPointers.Add (pPointer);
			}
			else
			{
				delete pPointer;
			}
		}
	}

	CString strRanges;
	if (tm.ExcludeTag (s_szTag_Ranges, strRanges))
	{
		CBCGPTagManager tmRanges (strRanges);

		CString strRange;
		while (tmRanges.ExcludeTag (s_szTag_Range, strRange))
		{
			XCircularColoredRange* pRange = new XCircularColoredRange;
			if (pRange->FromTag (strRange))
			{
				m_arRanges.Add (pRange);
			}
			else
			{
				delete pRange;
			}
		}
	}

	tm.ReadDouble (s_szTag_CapSize, m_dblCapSize);
	tm.ReadBool (s_szTag_Shape_Area, m_bShapeByTicksArea);

	return XElement::FromTag (tm.GetBuffer ());
}

void CBCGPGaugeInfo::XElementCircular::ToTag (CString& strTag) const
{
	XElement::ToTag (strTag);

	int i = 0;

	CString strScales;
	for (i = 0; i < m_arScales.GetSize (); i++)
	{
		CString strScale;
		m_arScales[i]->ToTag (strScale);
		CBCGPTagManager::WriteItem (strScales, s_szTag_Scale, strScale);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Scales, strScales);

	CString strPointers;
	for (i = 0; i < m_arPointers.GetSize (); i++)
	{
		CString strPointer;
		m_arPointers[i]->ToTag (strPointer);
		CBCGPTagManager::WriteItem (strPointers, s_szTag_Pointer, strPointer);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Pointers, strPointers);

	CString strRanges;
	for (i = 0; i < m_arRanges.GetSize (); i++)
	{
		CString strRange;
		m_arRanges[i]->ToTag (strRange);
		CBCGPTagManager::WriteItem (strRanges, s_szTag_Range, strRange);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Ranges, strRanges);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteDouble (s_szTag_CapSize, m_dblCapSize, 20.0));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_Shape_Area, m_bShapeByTicksArea, FALSE));
}

BOOL CBCGPGaugeInfo::XElementCircular::ColorsFromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadBrush (s_szTag_Pointer_Brush, m_Colors.m_brPointerFill);
	tm.ReadBrush (s_szTag_Pointer_Outline, m_Colors.m_brPointerOutline);
	tm.ReadBrush (s_szTag_Border_Brush, m_Colors.m_brBorderFill);
	tm.ReadBrush (s_szTag_Border_Outline, m_Colors.m_brBorderOutline);
	tm.ReadBrush (s_szTag_Fill_Brush, m_Colors.m_brFill);
	tm.ReadBrush (s_szTag_Fill_Outline, m_Colors.m_brOutline);
	tm.ReadBrush (s_szTag_Cap_Brush, m_Colors.m_brCapFill);
	tm.ReadBrush (s_szTag_Cap_Outline, m_Colors.m_brCapOutline);
	tm.ReadBrush (s_szTag_Text_Brush, m_Colors.m_brText);
	tm.ReadBrush (s_szTag_TickMark_Brush, m_Colors.m_brTickMark);

	return TRUE;
}

void CBCGPGaugeInfo::XElementCircular::ColorsToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Pointer_Brush, m_Colors.m_brPointerFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Pointer_Outline, m_Colors.m_brPointerOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Border_Brush, m_Colors.m_brBorderFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Border_Outline, m_Colors.m_brBorderOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_Colors.m_brFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_Colors.m_brOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Cap_Brush, m_Colors.m_brCapFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Cap_Outline, m_Colors.m_brCapOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Text_Brush, m_Colors.m_brText));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_TickMark_Brush, m_Colors.m_brTickMark));
}

BOOL CBCGPGaugeInfo::XElementNumeric::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	int style = (int)CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_CLASSIC;
	tm.ReadInt (s_szTag_Style, style);
	m_Style = (CBCGPNumericIndicatorImpl::BCGPNumericIndicatorStyle)
		min(max(style, (int)CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_CLASSIC), (int)CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_DIGITAL);
	tm.ReadInt (s_szTag_Digits, m_nDigits);
	tm.ReadInt (s_szTag_Decimal, m_nDecimals);
	tm.ReadInt (s_szTag_SeparatorWidth, m_nSeparatorWidth);
	tm.ReadBool (s_szTag_DrawDecimalPoint, m_bDrawDecimalPoint);
	tm.ReadBool (s_szTag_DrawLeadingZeros, m_bDrawLeadingZeros);

	return XElement::FromTag (tm.GetBuffer ());
}

void CBCGPGaugeInfo::XElementNumeric::ToTag (CString& strTag) const
{
	XElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Style, (int)m_Style, (int)CBCGPNumericIndicatorImpl::BCGP_NUMERIC_INDICATOR_CLASSIC));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Digits, m_nDigits, 6));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Decimal, m_nDecimals, 1));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_SeparatorWidth, m_nSeparatorWidth, 1));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_DrawDecimalPoint, m_bDrawDecimalPoint, FALSE));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBool (s_szTag_DrawLeadingZeros, m_bDrawLeadingZeros, TRUE));
}

BOOL CBCGPGaugeInfo::XElementNumeric::ColorsFromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadBrush (s_szTag_Fill_Brush, m_Colors.m_brFill);
	tm.ReadBrush (s_szTag_Fill_Outline, m_Colors.m_brOutline);
	tm.ReadBrush (s_szTag_Separator_Brush, m_Colors.m_brSeparator);
	tm.ReadBrush (s_szTag_Digit_Brush, m_Colors.m_brDigit);
	tm.ReadBrush (s_szTag_Decimal_Brush, m_Colors.m_brDecimal);
	tm.ReadBrush (s_szTag_Sign_Brush, m_Colors.m_brDecimal);
	tm.ReadBrush (s_szTag_Dot_Brush, m_Colors.m_brDecimal);

	return TRUE;
}

void CBCGPGaugeInfo::XElementNumeric::ColorsToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_Colors.m_brFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_Colors.m_brOutline));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Separator_Brush, m_Colors.m_brSeparator));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Digit_Brush, m_Colors.m_brDigit));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Decimal_Brush, m_Colors.m_brDecimal));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Sign_Brush, m_Colors.m_brDecimal));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Dot_Brush, m_Colors.m_brDecimal));
}

BOOL CBCGPGaugeInfo::XElementColor::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	int style = (int)CBCGPColorIndicatorImpl::BCGP_COLOR_INDICATOR_ELLIPSE;
	tm.ReadInt (s_szTag_Style, style);
	m_Style = (CBCGPColorIndicatorImpl::BCGPColorIndicatorStyle)
		min(max(style, (int)CBCGPColorIndicatorImpl::BCGP_COLOR_INDICATOR_ELLIPSE), (int)CBCGPColorIndicatorImpl::BCGP_COLOR_INDICATOR_RECTANGLE);

	return XElement::FromTag (tm.GetBuffer ());
}

void CBCGPGaugeInfo::XElementColor::ToTag (CString& strTag) const
{
	XElement::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteInt (s_szTag_Style, (int)m_Style, (int)CBCGPColorIndicatorImpl::BCGP_COLOR_INDICATOR_ELLIPSE));
}

BOOL CBCGPGaugeInfo::XElementColor::ColorsFromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	tm.ReadBrush (s_szTag_Fill_Brush, m_Colors.m_brFill);
	tm.ReadBrush (s_szTag_Fill_Outline, m_Colors.m_brOutline);

	return TRUE;
}

void CBCGPGaugeInfo::XElementColor::ColorsToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Brush, m_Colors.m_brFill));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteBrush (s_szTag_Fill_Outline, m_Colors.m_brOutline));
}

BOOL CBCGPGaugeInfo::XDashboard::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	CString strElements;
	if (tm.ExcludeTag (s_szTag_Elements, strElements))
	{
		CBCGPTagManager tmElements (strElements);

		CString strElement;
		while (tmElements.ExcludeTag (s_szTag_Element, strElement))
		{
			XElement* pElement = (XElement*)CBCGPGaugeInfo::CreateBaseFromTag (strElement);
			if (pElement != NULL)
			{
				m_arElements.Add (pElement);
			}
		}
	}

	tm.ReadRect (s_szTag_Rect, m_Rect);

	return TRUE;
}

void CBCGPGaugeInfo::XDashboard::ToTag (CString& strTag) const
{
	XBase::ToTag (strTag);

	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteRect (s_szTag_Rect, m_Rect, CRect (0, 0, 0, 0)));

	CString strElements;
	for (int i = 0; i < m_arElements.GetSize (); i++)
	{
		CString strElement;
		m_arElements[i]->ToTag (strElement);
		CBCGPTagManager::WriteItem (strElements, s_szTag_Element, strElement);
	}
	CBCGPTagManager::WriteItem (strTag, s_szTag_Elements, strElements);
}


CBCGPGaugeInfo::CBCGPGaugeInfo()
	: CBCGPBaseInfo(c_dwVersion, 0)
{
}

CBCGPGaugeInfo::~CBCGPGaugeInfo()
{
}

BOOL CBCGPGaugeInfo::FromTag (const CString& strTag)
{
	CString strXML;
	{
		CBCGPTagManager tmDashboard (strTag);

		if (!tmDashboard.ExcludeTag (s_szTag_Body, strXML))
		{
			return FALSE;
		}
	}

	CBCGPTagManager tm (strXML);

	CString strHeader;
	if (tm.ExcludeTag (CBCGPBaseInfo::s_szTag_Header, strHeader))
	{
		CBCGPTagManager tmHeader (strHeader);

		int nValue = (int)GetVersion ();
		tmHeader.CBCGPTagManager::ReadInt (CBCGPBaseInfo::s_szTag_Version, nValue);
		SetVersion ((DWORD)nValue);
	}
	else
	{
		return FALSE;
	}

	CString strDashboard;
	if (tm.ExcludeTag (s_szTag_Dashboard, strDashboard))
	{
		m_Dashboard.FromTag (strDashboard);
	}

	return TRUE;
}

void CBCGPGaugeInfo::ToTag (CString& strTag) const
{
	CString strData;

	CString strHeader;
	CBCGPTagManager::WriteTag (strHeader, CBCGPTagManager::WriteUInt (CBCGPBaseInfo::s_szTag_Version, GetVersion (), 0));
	CBCGPTagManager::WriteItem (strData, CBCGPBaseInfo::s_szTag_Header, strHeader);

	CString strDashboard;
	m_Dashboard.ToTag (strDashboard);
	CBCGPTagManager::WriteItem (strData, s_szTag_Dashboard, strDashboard);

	CBCGPTagManager::WriteItem (strTag, s_szTag_Body, strData);
}


CBCGPGaugeInfoLoader::CBCGPGaugeInfoLoader (CBCGPGaugeInfo& info)
	: CBCGPBaseInfoLoader(info, _T("BCGP_GAUGE_XML"), 0)
{
}

CBCGPGaugeInfoLoader::~CBCGPGaugeInfoLoader()
{
}

CBCGPGaugeInfoWriter::CBCGPGaugeInfoWriter(CBCGPGaugeInfo& info)
	: CBCGPBaseInfoWriter(info)
{
}

CBCGPGaugeInfoWriter::~CBCGPGaugeInfoWriter()
{
}

BOOL CBCGPGaugeInfoWriter::Save (const CString& strFileName)
{
	if (strFileName.IsEmpty ())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CStringArray sa;
	sa.Add (strFileName);

	if (!CheckFiles (sa))
	{
		return FALSE;
	}

	return SaveInfo (strFileName);
}
