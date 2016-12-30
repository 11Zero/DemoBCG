//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPRibbonCheckBox.cpp: implementation of the CBCGPRibbonCheckBox class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgglobals.h"
#include "BCGPRibbonCheckBox.h"
#include "BCGPVisualManager.h"
#include "BCGPDrawManager.h"

#ifndef BCGP_EXCLUDE_RIBBON

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CBCGPRibbonCheckBox, CBCGPRibbonButton)

const int nTextMarginLeft = 4;
const int nTextMarginRight = 6;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRibbonCheckBox::CBCGPRibbonCheckBox()
{
}
//*******************************************************************************
CBCGPRibbonCheckBox::CBCGPRibbonCheckBox(UINT nID, LPCTSTR lpszText) :
	CBCGPRibbonButton (nID, lpszText)
{
}
//*******************************************************************************
CBCGPRibbonCheckBox::~CBCGPRibbonCheckBox()
{
}

//////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////

CSize CBCGPRibbonCheckBox::GetIntermediateSize (CDC* /*pDC*/)
{
	ASSERT_VALID (this);

	m_szMargin = CSize (2, 3);

	const CSize sizeCheckBox = CBCGPVisualManager::GetInstance ()->GetCheckRadioDefaultSize ();

	int cx = sizeCheckBox.cx + m_sizeTextRight.cx + nTextMarginLeft + nTextMarginRight + m_szMargin.cx;
	int cy = max (sizeCheckBox.cy, m_sizeTextRight.cy) + 2 * m_szMargin.cy;

	return CSize (cx, cy) + m_sizePadding;
}
//*******************************************************************************
void CBCGPRibbonCheckBox::OnDraw (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	const CSize sizeCheckBox = CBCGPVisualManager::GetInstance ()->GetCheckRadioDefaultSize ();

	//----------------
	// Draw check box:
	//----------------
	CRect rectCheck = m_rect;
	rectCheck.DeflateRect (m_szMargin);
	rectCheck.left++;
	rectCheck.right = rectCheck.left + sizeCheckBox.cx;
	rectCheck.top = rectCheck.CenterPoint ().y - sizeCheckBox.cx / 2;

	rectCheck.bottom = rectCheck.top + sizeCheckBox.cy;

	OnDrawMark (pDC, rectCheck);

	//-----------
	// Draw text:
	//-----------
	COLORREF clrTextOld = (COLORREF)-1;

	if (m_bIsDisabled)
	{
		COLORREF clrText = (COLORREF)-1;
		if (m_bQuickAccessMode)
		{
			clrText = CBCGPVisualManager::GetInstance ()->GetRibbonQATTextColor (TRUE);
		}
		else
		{
			clrText = CBCGPVisualManager::GetInstance ()->GetRibbonBarTextColor(TRUE);
		}

		if (clrText != (COLORREF)-1)
		{
			clrTextOld = pDC->SetTextColor (clrText);
		}
	}

	CRect rectText = m_rect;
	rectText.left = rectCheck.right + nTextMarginLeft;

	DoDrawText (pDC, m_strText, rectText, DT_SINGLELINE | DT_VCENTER, 
		(m_bIsDisabled && CBCGPToolBarImages::m_bIsDrawOnGlass)
			? (m_bQuickAccessMode
				? CBCGPVisualManager::GetInstance ()->GetRibbonQATTextColor (TRUE)
				: CBCGPVisualManager::GetInstance ()->GetToolbarDisabledTextColor ())
			: -1);

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor (clrTextOld);
	}

	if (IsFocused ())
	{
		CRect rectFocus = rectText;
		rectFocus.OffsetRect (-nTextMarginLeft / 2, 0);
		rectFocus.DeflateRect (0, 2);

		if (CBCGPToolBarImages::m_bIsDrawOnGlass)
		{
			CBCGPDrawManager dm (*pDC);
			dm.DrawFocusRect(rectFocus);
		}
		else
		{
			pDC->DrawFocusRect (rectFocus);
		}
	}
}
//********************************************************************************
void CBCGPRibbonCheckBox::OnDrawOnList (CDC* pDC, CString strText,
									  int nTextOffset, CRect rect,
									  BOOL bIsSelected, BOOL bHighlighted)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	BOOL bIsDisabled = m_bIsDisabled;
	m_bIsDisabled = FALSE;

	CRect rectCheck = rect;
	rectCheck.right = rect.left + nTextOffset;

	if (rectCheck.Width () > rectCheck.Height ())
	{
		rectCheck.left = rectCheck.CenterPoint ().x - rectCheck.Height () / 2;
		rectCheck.right = rectCheck.left + rectCheck.Height ();
	}
	else
	{
		rectCheck.top = rectCheck.CenterPoint ().y - rectCheck.Width () / 2;
		rectCheck.bottom = rectCheck.top + rectCheck.Width ();
	}

	OnDrawMarkOnList (pDC, rectCheck, bIsSelected, bHighlighted);

	rect.left += nTextOffset;
	
	const int xMargin = 3;
	rect.DeflateRect (xMargin, 0);

	pDC->DrawText (strText, rect, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	m_bIsDisabled = bIsDisabled;
}
//********************************************************************************
void CBCGPRibbonCheckBox::OnDrawMark (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	const BOOL bIsHighlighted = (IsHighlighted () || IsFocused ()) && !IsDisabled ();

	int nState = 0;

	if (m_bIsChecked == 2)
	{
		nState = 2;
	}
	else if (IsChecked () || (IsPressed () && bIsHighlighted))
	{
		nState = 1;
	}

	CBCGPVisualManager::GetInstance ()->OnDrawCheckBoxEx (pDC,
		rect,  nState, bIsHighlighted, IsPressed () && bIsHighlighted, !IsDisabled ());
}
//********************************************************************************
void CBCGPRibbonCheckBox::OnDrawMarkOnList (CDC* pDC, CRect rect, BOOL bIsSelected, BOOL bHighlighted)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CBCGPVisualManager::GetInstance ()->OnDrawRibbonCheckBoxOnList (
		pDC, this, rect, bIsSelected, bHighlighted);
}
//*******************************************************************************
BOOL CBCGPRibbonCheckBox::SetACCData(CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pParent);

	if (!CBCGPRibbonButton::SetACCData(pParent, data))
	{
		return FALSE;
	}

	data.m_nAccRole = ROLE_SYSTEM_CHECKBUTTON;


	if (IsChecked ())
	{
		data.m_strAccDefAction = L"Uncheck";
	}
	else
	{
		data.m_strAccDefAction = L"Check";
	}
		 
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonRadioButton

IMPLEMENT_DYNCREATE(CBCGPRibbonRadioButton, CBCGPRibbonCheckBox)

CBCGPRibbonRadioButton::CBCGPRibbonRadioButton()
{
}
//*******************************************************************************
CBCGPRibbonRadioButton::CBCGPRibbonRadioButton(UINT nID, LPCTSTR lpszText) :
	CBCGPRibbonCheckBox (nID, lpszText)
{
}
//*******************************************************************************
CBCGPRibbonRadioButton::~CBCGPRibbonRadioButton()
{
}
//********************************************************************************
void CBCGPRibbonRadioButton::OnDrawMark (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	const BOOL bIsHighlighted = (IsHighlighted () || IsFocused ()) && !IsDisabled ();

	CBCGPVisualManager::GetInstance ()->OnDrawRadioButton (pDC,
		rect, (IsChecked () || (IsPressed () && bIsHighlighted)) ? 1 : 0,
		bIsHighlighted, IsPressed () && bIsHighlighted, !IsDisabled ());
}
//********************************************************************************
void CBCGPRibbonRadioButton::OnDrawMarkOnList (CDC* pDC, CRect rect, BOOL bIsSelected, BOOL bHighlighted)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	CBCGPVisualManager::GetInstance ()->OnDrawRibbonRadioButtonOnList (
		pDC, this, rect, bIsSelected, bHighlighted);
}
//*******************************************************************************
BOOL CBCGPRibbonRadioButton::SetACCData(CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pParent);

	if (!CBCGPRibbonCheckBox::SetACCData(pParent, data))
	{
		return FALSE;
	}

	data.m_nAccRole = ROLE_SYSTEM_RADIOBUTTON; 
	return TRUE;
}

#endif // BCGP_EXCLUDE_RIBBON
