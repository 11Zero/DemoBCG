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
// BCGPRibbonQuickStepsButton.cpp: implementation of the CBCGPRibbonQuickStepsButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPRibbonQuickStepsButton.h"
#include "BCGPRibbonCategory.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef BCGP_EXCLUDE_RIBBON

/////////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonQuickStep

IMPLEMENT_DYNCREATE(CBCGPRibbonQuickStep, CBCGPRibbonPaletteIcon)

CBCGPRibbonQuickStep::CBCGPRibbonQuickStep(
					 int		nImageIndex,
					 LPCTSTR	lpszLabel,
					 LPCTSTR	lpszDescription,
					 BOOL		bIsEnabled)
{
	m_nImageIndex = nImageIndex;
	m_strText = (lpszLabel == NULL) ? _T("") : lpszLabel;
	m_strDescription = (lpszDescription == NULL) ? _T("") : lpszDescription;
	m_bIsDisabled = !bIsEnabled;
}
//******************************************************************************
CBCGPRibbonQuickStep::CBCGPRibbonQuickStep()
{
}
//******************************************************************************
CBCGPRibbonQuickStep::~CBCGPRibbonQuickStep()
{
}
//******************************************************************************
void CBCGPRibbonQuickStep::Enable(BOOL bEnable)
{
	ASSERT_VALID(this);
	m_bIsDisabled = !bEnable;
}
//******************************************************************************
CSize CBCGPRibbonQuickStep::GetToolTipImageSize(int& nRibbonImageType) const
{
	ASSERT_VALID(this);

	nRibbonImageType = CBCGPRibbonButton::RibbonImageSmall;

	if (m_pOwner == NULL)
	{
		return CSize(0, 0);
	}

	ASSERT_VALID(m_pOwner);
	return m_pOwner->GetIconSize();
}
//******************************************************************************
void CBCGPRibbonQuickStep::OnDrawTooltipImage(CDC* pDC, RibbonImageType /*type*/, const CRect& rectImage)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	if (m_pOwner == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pOwner);
	m_pOwner->GetImages().DrawEx(pDC, rectImage, m_nImageIndex);
}
//******************************************************************************
void CBCGPRibbonQuickStep::CopyFrom(const CBCGPBaseRibbonElement& src)
{
	ASSERT_VALID(this);

	CBCGPRibbonPaletteIcon::CopyFrom(src);
	m_bIsDisabled = src.IsDisabled();
}

/////////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonQuickStepsButton

IMPLEMENT_DYNCREATE(CBCGPRibbonQuickStepsButton, CBCGPRibbonPaletteButton)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPRibbonQuickStepsButton::CBCGPRibbonQuickStepsButton()
{
	CommonInit();
}
//******************************************************************************
CBCGPRibbonQuickStepsButton::CBCGPRibbonQuickStepsButton(
		 UINT				nID,
		 LPCTSTR			lpszText, 
		 int				nSmallImageIndex,
		 int				nLargeImageIndex,
		 CBCGPToolBarImages& imagesPalette) :
	CBCGPRibbonPaletteButton(nID, lpszText, nSmallImageIndex, nLargeImageIndex, imagesPalette)
{
	CommonInit();
}
//******************************************************************************
CBCGPRibbonQuickStepsButton::CBCGPRibbonQuickStepsButton(
		UINT				nID,
		LPCTSTR				lpszText, 
		int					nSmallImageIndex,
		int					nLargeImageIndex,
		UINT				uiImagesPaletteResID,
		int					cxPaletteImage) :
	CBCGPRibbonPaletteButton(nID, lpszText, nSmallImageIndex, nLargeImageIndex, uiImagesPaletteResID, cxPaletteImage)
{
	CommonInit();
}
//******************************************************************************
void CBCGPRibbonQuickStepsButton::CommonInit()
{
	RemoveAll();

	m_nIcons = 0;
	m_bSmallIcons = TRUE;
	m_nIconsInRow = m_nPanelColumns = 2;
}
//******************************************************************************
CBCGPRibbonQuickStepsButton::~CBCGPRibbonQuickStepsButton()
{
}
//******************************************************************************
void CBCGPRibbonQuickStepsButton::AddQuickStep(CBCGPRibbonQuickStep* pQuickStep)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pQuickStep);

	int nIndex = 0;

	if (m_arIcons.GetSize() == 0)
	{
		AddScrollButtons();
	}
	else
	{
		// Insert before scroll/menu buttons:
		nIndex = (int)m_arIcons.GetSize() - 3;
	}

	pQuickStep->m_pOwner = this;
	pQuickStep->m_nIndex = nIndex;

	m_arIcons.InsertAt(nIndex, pQuickStep);

	m_nIcons++;
}
//******************************************************************************
void CBCGPRibbonQuickStepsButton::OnDrawPaletteIcon (CDC* pDC, CRect rectIcon, int nIconIndex, CBCGPRibbonPaletteIcon* pIcon, COLORREF clrText)
{
	ASSERT_VALID(this);
	ASSERT_VALID (pDC);
	ASSERT_VALID(pIcon);

	const int nIconPadding = 2;
	const int nTextPadding = 2;

	BOOL bIsDisabled = IsDisabled() || pIcon->IsDisabled();

	if (nIconIndex >= 0 && nIconIndex < m_imagesPalette.GetCount())
	{
		int dy = max(0, (rectIcon.Height() - GetIconSize().cy) / 2);
		m_imagesPalette.Draw (pDC, rectIcon.left + nIconPadding, rectIcon.top + dy, nIconIndex, FALSE, bIsDisabled);
	}

	CRect rectText = rectIcon;
	rectText.left += GetIconSize().cx + nIconPadding + nTextPadding;
	rectText.right -= nTextPadding;
	
	COLORREF clrTextOld = (COLORREF)-1;
	
	if (bIsDisabled)
	{
		clrTextOld = pDC->SetTextColor(globalData.clrGrayedText);
	}
	else if (clrText != (COLORREF)-1)
	{
		clrTextOld = pDC->SetTextColor(clrText);
	}
	
	pDC->DrawText(pIcon->GetText(), rectText, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

	if (clrTextOld != (COLORREF)-1)
	{
		pDC->SetTextColor(clrTextOld);
	}
}
//***************************************************************************
CSize CBCGPRibbonQuickStepsButton::GetItemSize () const
{
	CBCGPRibbonQuickStepsButton* pOrigButton = DYNAMIC_DOWNCAST(CBCGPRibbonQuickStepsButton, m_pOriginal);
	
	if (pOrigButton != NULL)
	{
		ASSERT_VALID (pOrigButton);
		return pOrigButton->GetItemSize();
	}

	CSize size = GetIconSize();

	CBCGPRibbonCategory* pParent = GetParentCategory();
	if (pParent != NULL)
	{
		ASSERT_VALID(pParent);

		size.cx += pParent->GetImageSize(TRUE).cx * 3;
		size.cy += 4;
	}

	return size;
}
//*****************************************************************************
void CBCGPRibbonQuickStepsButton::CopyFrom (const CBCGPBaseRibbonElement& s)
{
	ASSERT_VALID (this);

	CBCGPRibbonPaletteButton::CopyFrom (s);

	if (!s.IsKindOf (RUNTIME_CLASS (CBCGPRibbonQuickStepsButton)))
	{
		return;
	}

	CBCGPRibbonQuickStepsButton& src = (CBCGPRibbonQuickStepsButton&) s;

	RemoveAll ();

	for (int i = 0; i < src.m_arIcons.GetSize(); i++)
	{
		CBCGPRibbonQuickStep* pButton = new CBCGPRibbonQuickStep();

		pButton->CopyFrom(*src.m_arIcons[i]);
		pButton->m_pOwner = this;

		m_arIcons.Add(pButton);
	}
}

#endif // BCGP_EXCLUDE_RIBBON
