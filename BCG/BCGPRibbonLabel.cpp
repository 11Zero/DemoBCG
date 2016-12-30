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
// BCGPRibbonLabel.cpp: implementation of the CBCGPRibbonLabel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPVisualManager.h"
#include "bcgglobals.h"
#include "BCGPRibbonLabel.h"
#include "BCGPRibbonCategory.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef BCGP_EXCLUDE_RIBBON

IMPLEMENT_DYNCREATE(CBCGPRibbonLabel, CBCGPRibbonButton)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRibbonLabel::CBCGPRibbonLabel()
{
	m_bIsPaletteGroup = FALSE;
}
//***********************************************************************************
CBCGPRibbonLabel::CBCGPRibbonLabel(LPCTSTR lpszText, BOOL bIsMultiLine)
{
	ASSERT (lpszText != NULL);
	m_strText = lpszText;

	m_bIsAlwaysLarge = bIsMultiLine;
	m_bIsPaletteGroup = FALSE;
}
//***********************************************************************************
CBCGPRibbonLabel::~CBCGPRibbonLabel()
{
}
//***********************************************************************************
void CBCGPRibbonLabel::OnDraw (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (m_rect.IsRectEmpty ())
	{
		return;
	}

	CRect rectText = m_rect;
	rectText.DeflateRect (m_szMargin.cx, 0);

	COLORREF cltTextOld = (COLORREF)-1;

	BOOL bIsMenuMode = IsMenuMode () || m_bIsPaletteGroup;

	if (bIsMenuMode)
	{
		rectText.bottom -= 2;

		COLORREF clrText = CBCGPVisualManager::GetInstance ()->OnDrawMenuLabel (pDC, m_rect);

		if (clrText != (COLORREF)-1)
		{
			cltTextOld = pDC->SetTextColor (clrText);
		}
	}
	else
	{
		CBCGPVisualManager::GetInstance ()->OnDrawRibbonLabel (pDC, 
			this, m_rect);
	}

	CFont* pOldFont = NULL;
	
	if (bIsMenuMode)
	{
		pOldFont = pDC->SelectObject (&globalData.fontBold);
		ASSERT_VALID (pOldFont);
	}

	UINT uiDTFlags = bIsMenuMode || !m_bIsAlwaysLarge ? 
		DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX :
		DT_WORDBREAK | DT_NOPREFIX;

	if (!bIsMenuMode && m_bIsAlwaysLarge)
	{
		int dy = max (0, (rectText.Height () - m_sizeTextRight.cy) / 2);
		rectText.DeflateRect (0, dy);
	}

	DoDrawText (pDC, m_strText, rectText, uiDTFlags);

	if (pOldFont != NULL)
	{
		pDC->SelectObject (pOldFont);
	}

	if (cltTextOld != (COLORREF)-1)
	{
		cltTextOld = pDC->SetTextColor (cltTextOld);
	}
}
//******************************************************************************
void CBCGPRibbonLabel::OnCalcTextSize (CDC* pDC)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	if (IsMenuMode () || m_bIsPaletteGroup || !m_bIsAlwaysLarge)
	{
		CFont* pOldFont = NULL;
		
		if (IsMenuMode () || m_bIsPaletteGroup)
		{
			pOldFont = pDC->SelectObject (&globalData.fontBold);
			ASSERT_VALID (pOldFont);
		}

		CBCGPRibbonButton::OnCalcTextSize (pDC);

		if (pOldFont != NULL)
		{
			pDC->SelectObject (pOldFont);
		}

		return;
	}

	//------------------
	// Multi-line label:
	//------------------

	ASSERT_VALID (m_pParent);

	const CSize sizeImageLarge = m_pParent->GetImageSize (TRUE);
	if (sizeImageLarge == CSize (0, 0))
	{
		ASSERT (FALSE);
		return;
	}

	const int nMaxHeight = 2 * sizeImageLarge.cy;

	int nTextHeight = 0;
	int nTextWidth = 0;

	CString strText = m_strText;

	for (int dx = 10; dx < 200; dx += 10)
	{
		CRect rectText (0, 0, dx, 10000);

		nTextHeight = pDC->DrawText (strText, rectText, 
									DT_WORDBREAK | DT_CALCRECT);

		nTextWidth = rectText.Width ();
		
		if (nTextHeight <= nMaxHeight && nTextWidth >= nTextHeight)
		{
			break;
		}
	}

	m_sizeTextRight.cx = nTextWidth;
	m_sizeTextRight.cy = nTextHeight;
}
//**************************************************************************
CSize CBCGPRibbonLabel::GetIntermediateSize (CDC* pDC)
{
	ASSERT_VALID (this);

	if (IsMenuMode () || m_bIsPaletteGroup)
	{
		m_szMargin = CSize (3, 3);
	}
	else
	{
		m_szMargin = CSize (2, 4);
	}
	
	OnCalcTextSize (pDC);
	return CSize (m_sizeTextRight.cx + 2 * m_szMargin.cx, m_sizeTextRight.cy + 2 * m_szMargin.cy + m_sizePadding.cy);
}
//********************************************************************************
BOOL CBCGPRibbonLabel::SetACCData(CWnd* pParent, CBCGPAccessibilityData& data)
{
	ASSERT_VALID(this);

	CBCGPBaseRibbonElement::SetACCData (pParent, data);

	data.m_nAccRole = ROLE_SYSTEM_STATICTEXT;
	data.m_bAccState = 0;
	data.m_strAccDefAction.Empty();
	return TRUE;
}
//********************************************************************************
void CBCGPRibbonLabel::CopyFrom (const CBCGPBaseRibbonElement& s)
{
	ASSERT_VALID (this);

	CBCGPBaseRibbonElement::CopyFrom(s);

	CBCGPRibbonLabel& src = (CBCGPRibbonLabel&) s;

	m_bIsPaletteGroup = src.m_bIsPaletteGroup;
}

#endif // BCGP_EXCLUDE_RIBBON
