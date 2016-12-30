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
// BCGPDashboardConstructor.cpp: implementation of the CBCGPRibbonConstrucor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPDashboardConstructor.h"

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

CBCGPDashboardConstructor::CBCGPDashboardConstructor(const CBCGPGaugeInfo& info)
	: m_Info(info)
{
}

CBCGPDashboardConstructor::~CBCGPDashboardConstructor()
{
}

void CBCGPDashboardConstructor::Construct (CBCGPDashboard& dashboard) const
{
	const CBCGPGaugeInfo::XDashboard& infoDashboard = GetInfo ().GetDashboard ();

	for (int i = 0; i < infoDashboard.m_arElements.GetSize (); i++)
	{
		CBCGPGaugeInfo::XElement& info = 
			(CBCGPGaugeInfo::XElement&)*infoDashboard.m_arElements[i];
		CBCGPGaugeImpl* pElement = CreateElement (info);
		if (pElement != NULL)
		{
			ASSERT_VALID (pElement);

//			dashboard.Add(pElement);
		}
	}	

	if (!infoDashboard.m_Rect.IsRectEmpty ())
	{
//		dashboard.SetRect (infoDashboard.m_Rect, TRUE, FALSE);
	}
}

CBCGPGaugeImpl* CBCGPDashboardConstructor::CreateElement (const CBCGPGaugeInfo::XElement& info) const
{
	CBCGPGaugeImpl* pElement = NULL;

	if (info.GetElementName ().Compare (CBCGPGaugeInfo::s_szCircularGauge) == 0)
	{
		const CBCGPGaugeInfo::XElementCircular& infoElement = 
			(const CBCGPGaugeInfo::XElementCircular&)info;

		CBCGPCircularGaugeImpl* pNewElement = new CBCGPCircularGaugeImpl;

		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);

		pNewElement->SetColors (infoElement.m_Colors);
		pNewElement->SetCapSize (infoElement.m_dblCapSize);
		pNewElement->EnableShapeByTicksArea (infoElement.m_bShapeByTicksArea);

		int i = 0;
		for (i = 0; i < infoElement.m_arScales.GetSize (); i++)
		{
			const CBCGPGaugeInfo::XCircularScale* pScaleInfo = 
				(const CBCGPGaugeInfo::XCircularScale*)infoElement.m_arScales[i];

			int index = 0;
			if (i != 0)
			{
				index = pNewElement->AddScale ();
			}

			pNewElement->SetRange (pScaleInfo->m_dblStart, pScaleInfo->m_dblFinish, index);
			pNewElement->SetStep (pScaleInfo->m_dblStep, index);
			pNewElement->SetTextLabelFormat (pScaleInfo->m_strLabelFormat, index);
			pNewElement->SetTickMarkSize (pScaleInfo->m_dblMinorTickMarkSize, FALSE, index);
			pNewElement->SetTickMarkSize (pScaleInfo->m_dblMajorTickMarkSize, TRUE, index);
			pNewElement->SetMajorTickMarkStep (pScaleInfo->m_dblMajorTickMarkStep, index);
			pNewElement->SetScaleFillBrush (pScaleInfo->m_brFill, index);
			pNewElement->SetScaleOutlineBrush (pScaleInfo->m_brOutline, index);
			pNewElement->SetScaleTextBrush (pScaleInfo->m_brText, index);
			pNewElement->SetScaleTickMarkBrush (pScaleInfo->m_brTickMarkMinor, FALSE, index);
			pNewElement->SetScaleTickMarkBrush (pScaleInfo->m_brTickMarkMajor, TRUE, index);
			pNewElement->SetTicksAreaAngles (pScaleInfo->m_dblStartAngle, pScaleInfo->m_dblFinishAngle, index);
			pNewElement->SetScaleOffsetFromBorder (pScaleInfo->m_dblOffsetFromBorder, index);
		}

		for (i = 0; i < infoElement.m_arPointers.GetSize (); i++)
		{
			const CBCGPGaugeInfo::XCircularPointer* pPointerInfo = 
				(const CBCGPGaugeInfo::XCircularPointer*)infoElement.m_arPointers[i];

			int index = 0;
			if (i != 0)
			{
				index = pNewElement->AddPointer (pPointerInfo->m_dblSize, pPointerInfo->m_nScale, pPointerInfo->m_brFill, 
					pPointerInfo->m_brOutline, FALSE);
			}

			pNewElement->SetValue (pPointerInfo->m_dblValue, index, 0, FALSE);
		}

		for (i = 0; i < infoElement.m_arRanges.GetSize (); i++)
		{
			const CBCGPGaugeInfo::XCircularColoredRange* pRangeInfo = 
				(const CBCGPGaugeInfo::XCircularColoredRange*)infoElement.m_arRanges[i];

			pNewElement->AddColoredRange (pRangeInfo->m_dblStartValue, pRangeInfo->m_dblFinishValue, pRangeInfo->m_brFill,
				pRangeInfo->m_brOutline, pRangeInfo->m_nScale, pRangeInfo->m_dblStartWidth, pRangeInfo->m_dblFinishWidth, 
				pRangeInfo->m_dblOffsetFromBorder, pRangeInfo->m_brTextLabel, pRangeInfo->m_brTickMark, FALSE);
		}
	}
	else if (info.GetElementName ().Compare (CBCGPGaugeInfo::s_szNumericInd) == 0)
	{
		const CBCGPGaugeInfo::XElementNumeric& infoElement = 
			(const CBCGPGaugeInfo::XElementNumeric&)info;

		CBCGPNumericIndicatorImpl* pNewElement = new CBCGPNumericIndicatorImpl;

		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);
		pNewElement->SetValue (info.m_dblValue);

		pNewElement->SetColors (infoElement.m_Colors);
		pNewElement->SetStyle (infoElement.m_Style);
		pNewElement->SetDigits (infoElement.m_nDigits);
		pNewElement->SetDecimals (infoElement.m_nDecimals);
		pNewElement->SetSeparatorWidth (infoElement.m_nSeparatorWidth);
		pNewElement->SetDrawDecimalPoint (infoElement.m_bDrawDecimalPoint);
		pNewElement->SetDrawLeadingZeros (infoElement.m_bDrawLeadingZeros);
	}
	else if (info.GetElementName ().Compare (CBCGPGaugeInfo::s_szColorInd) == 0)
	{
		const CBCGPGaugeInfo::XElementColor& infoElement = 
			(const CBCGPGaugeInfo::XElementColor&)info;

		CBCGPColorIndicatorImpl* pNewElement = new CBCGPColorIndicatorImpl;

		pElement = pNewElement;

		ConstructBaseElement (*pElement, info);

		pNewElement->SetColors (infoElement.m_Colors);
		pNewElement->SetStyle (infoElement.m_Style);
	}

	return pElement;
}

void CBCGPDashboardConstructor::ConstructBaseElement (CBCGPGaugeImpl& element, const CBCGPGaugeInfo::XElement& info) const
{
	element.SetName (info.m_ID.m_Name);
	element.SetID (info.m_ID.m_Value);
	element.SetRect (info.m_Rect);
	element.SetBorderSize (info.m_nBorderSize);
	element.SetVisible (info.m_bIsVisible);
}
