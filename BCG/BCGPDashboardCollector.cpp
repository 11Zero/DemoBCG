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
// BCGPDashboardCollector.cpp: implementation of the CBCGPRibbonCollector class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPDashboardCollector.h"

//#include "BCGPDashboard.h"
#include "BCGPVisualContainer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPDashboardCollector::CBCGPDashboardCollector(CBCGPGaugeInfo& info)
	: m_Info (info)
{
}

CBCGPDashboardCollector::~CBCGPDashboardCollector()
{
}

void CBCGPDashboardCollector::Collect(const CBCGPDashboard& dashboard)
{
	CollectDashboard (dashboard, GetInfo ().GetDashboard ());
}

void CBCGPDashboardCollector::CollectDashboard(const CBCGPDashboard& dashboard, CBCGPGaugeInfo::XDashboard& info)
{
/*	info.m_Rect = dashboard.GetRect ();
	CPoint ptOffset (0, 0);

	if (!info.m_Rect.IsRectEmpty ())
	{
		ptOffset = info.m_Rect.TopLeft ();
	}

	for (int i = 0; i < dashboard.GetCount (); i++)
	{
		const CBCGPGaugeImpl* pElement = dashboard[i];
		ASSERT_VALID (pElement);

		if (pElement != NULL)
		{
			CBCGPGaugeInfo::XElement* pElementInfo = CollectElement (*pElement);
			if (pElementInfo != NULL)
			{
				pElementInfo->m_Rect.OffsetRect (-ptOffset);
				info.m_arElements.Add (pElementInfo);
			}
		}
	}*/
}

CBCGPGaugeInfo::XElement* CBCGPDashboardCollector::CollectElement(const CBCGPGaugeImpl& element)
{
	CBCGPGaugeInfo::XElement* info = NULL;

	if (element.IsKindOf (RUNTIME_CLASS (CBCGPCircularGaugeImpl)))
	{
		CBCGPGaugeInfo::XElementCircular* pNewInfo = new CBCGPGaugeInfo::XElementCircular;
		info = pNewInfo;

		CollectBaseElement (element, *info);
		info->m_dblValue = element.GetValue ();

		const CBCGPCircularGaugeImpl* pElement = (const CBCGPCircularGaugeImpl*)&element;
		ASSERT_VALID (pElement);

		pNewInfo->m_Colors             = pElement->GetColors ();
		pNewInfo->m_dblCapSize         = pElement->GetCapSize ();
		pNewInfo->m_bShapeByTicksArea  = pElement->IsShapeByTicksArea ();

		int i = 0;
		for (i = 0; i < element.GetScalesCount (); i++)
		{
			CBCGPGaugeInfo::XCircularScale* pScaleInfo = new CBCGPGaugeInfo::XCircularScale;

			pScaleInfo->m_dblStart             = pElement->GetStart (i);
			pScaleInfo->m_dblFinish            = pElement->GetFinish (i);
			pScaleInfo->m_dblStep              = pElement->GetStep (i);
			pScaleInfo->m_strLabelFormat       = pElement->GetTextLabelFormat (i);
			pScaleInfo->m_dblMinorTickMarkSize = pElement->GetTickMarkSize (FALSE, i);
			pScaleInfo->m_dblMajorTickMarkSize = pElement->GetTickMarkSize (TRUE, i);
			pScaleInfo->m_dblMajorTickMarkStep = pElement->GetMajorTickMarkStep (i);
			pScaleInfo->m_brFill               = pElement->GetScaleFillBrush (i);
			pScaleInfo->m_brOutline            = pElement->GetScaleOutlineBrush (i);
			pScaleInfo->m_brText               = pElement->GetScaleTextBrush (i);
			pScaleInfo->m_brTickMarkMinor      = pElement->GetScaleTickMarkBrush (FALSE, i);
			pScaleInfo->m_brTickMarkMajor      = pElement->GetScaleTickMarkBrush (TRUE, i);
			pScaleInfo->m_dblStartAngle        = pElement->GetTicksAreaStartAngle (i);
			pScaleInfo->m_dblFinishAngle       = pElement->GetTicksAreaFinishAngle (i);
			pScaleInfo->m_dblOffsetFromBorder  = pElement->GetScaleOffsetFromBorder (i);
			pNewInfo->m_arScales.Add (pScaleInfo);
		}

		for (i = 0; i < pElement->GetDataCount (); i++)
		{
			const CBCGPCircularGaugePointer* pPointer = (const CBCGPCircularGaugePointer*)pElement->GetData (i);
			CBCGPGaugeInfo::XCircularPointer* pPointerInfo = new CBCGPGaugeInfo::XCircularPointer;

			pPointerInfo->m_nScale    = pPointer->GetScale ();
			pPointerInfo->m_dblValue  = pPointer->GetValue ();
			pPointerInfo->m_brFill    = pPointer->GetFillBrush ();
			pPointerInfo->m_brOutline = pPointer->GetOutlineBrush ();
			pPointerInfo->m_dblSize   = pPointer->GetSize ();
			pNewInfo->m_arPointers.Add(pPointerInfo);
		}

		for (i = 0; i < pElement->GetColoredRangesCount (); i++)
		{
			const CBCGPCircularGaugeColoredRangeObject* pRange = (const CBCGPCircularGaugeColoredRangeObject*)pElement->GetColoredRange (i);
			CBCGPGaugeInfo::XCircularColoredRange* pRangeInfo = new CBCGPGaugeInfo::XCircularColoredRange;

			pRangeInfo->m_nScale              = pRange->GetScale ();
			pRangeInfo->m_dblStartValue       = pRange->GetStartValue ();
			pRangeInfo->m_dblFinishValue      = pRange->GetFinishValue ();
			pRangeInfo->m_brFill              = pRange->GetFillBrush ();
			pRangeInfo->m_brOutline           = pRange->GetOutlineBrush ();
			pRangeInfo->m_brTickMark          = pRange->GetTickMarkBrush ();
			pRangeInfo->m_brTextLabel         = pRange->GetTextLabelBrush ();
			pRangeInfo->m_dblStartWidth       = pRange->GetStartWidth ();
			pRangeInfo->m_dblFinishWidth      = pRange->GetFinishWidth ();
			pRangeInfo->m_dblOffsetFromBorder = pRange->GetOffsetFromBorder ();
			pNewInfo->m_arRanges.Add(pRangeInfo);
		}
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPNumericIndicatorImpl)))
	{
		CBCGPGaugeInfo::XElementNumeric* pNewInfo = new CBCGPGaugeInfo::XElementNumeric;
		info = pNewInfo;
		
		CollectBaseElement (element, *info);
		info->m_dblValue = element.GetValue ();

		const CBCGPNumericIndicatorImpl* pElement = (const CBCGPNumericIndicatorImpl*)&element;
		ASSERT_VALID (pElement);

		pNewInfo->m_Colors            = pElement->GetColors ();
		pNewInfo->m_Style             = pElement->GetStyle ();
		pNewInfo->m_nDigits           = pElement->GetDigits ();
		pNewInfo->m_nDecimals         = pElement->GetDecimals ();
		pNewInfo->m_nSeparatorWidth   = pElement->GetSeparatorWidth ();
		pNewInfo->m_bDrawDecimalPoint = pElement->IsDrawDecimalPoint ();
		pNewInfo->m_bDrawLeadingZeros = pElement->IsDrawLeadingZeros ();
	}
	else if (element.IsKindOf (RUNTIME_CLASS (CBCGPColorIndicatorImpl)))
	{
		CBCGPGaugeInfo::XElementColor* pNewInfo = new CBCGPGaugeInfo::XElementColor;
		info = pNewInfo;
		
		CollectBaseElement (element, *info);

		const CBCGPColorIndicatorImpl* pElement = (const CBCGPColorIndicatorImpl*)&element;
		ASSERT_VALID (pElement);

		pNewInfo->m_Colors = pElement->GetColors ();
		pNewInfo->m_Style  = pElement->GetStyle ();
	}

	return info;
}

void CBCGPDashboardCollector::CollectBaseElement (const CBCGPGaugeImpl& element, CBCGPGaugeInfo::XElement& info)
{
	info.m_ID.m_Name = element.GetName ();
	info.m_ID.m_Value = element.GetID ();
	info.m_Rect = element.GetRect ();
	info.m_nBorderSize = element.GetBorderSize ();
	info.m_bIsVisible = element.IsVisible ();
}
