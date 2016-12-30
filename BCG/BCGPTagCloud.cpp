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
// BCGPTagCloud.cpp: implementation of the CBCGPTagCloud class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <float.h>
#include "BCGPTagCloud.h"
#ifndef _BCGSUITE_
#include "BCGPPopupMenu.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

UINT BCGM_ON_CLICK_TAG_CLOUD = ::RegisterWindowMessage (_T("BCGM_ON_CLICK_TAG_CLOUD"));

/////////////////////////////////////////////////////////////////////////////
// CBCGPTagCloudElement

CBCGPTagCloudElement::CBCGPTagCloudElement()
{
	CommonInit();
}
//*****************************************************************************************
CBCGPTagCloudElement::CBCGPTagCloudElement(const CString& strLabel, double dblValue, 
										   const CBCGPColor& colorText,
										   const CBCGPBrush& brushBackground,
										   const CBCGPColor& colorBorder)
{
	CommonInit();
	m_strLabel = strLabel;
	m_dblValue = dblValue;
	m_colorText = colorText;
	m_colorBorder = colorBorder,
	m_brushBackground = brushBackground;
}
//*****************************************************************************************
void CBCGPTagCloudElement::CommonInit()
{
	m_pOwner = NULL;
	m_nWeight = -1;
	m_dblValue = 0.0;
	m_rect.SetRectEmpty();
	m_dblTextHeight = 0.0;
}
//*****************************************************************************************
BOOL CBCGPTagCloudElement::IsSelected() const
{
	ASSERT_VALID(this);

	if (m_pOwner != NULL)
	{
		ASSERT_VALID(m_pOwner);
		return m_pOwner->m_pSelected == this;
	}

	return FALSE;
}
//*****************************************************************************************
void CBCGPTagCloudElement::DoDraw(CBCGPTagCloud* pTagCloud, CBCGPGraphicsManager* pGM, CBCGPTextFormat& tf, BOOL bIsPressed, BOOL bIsHighlighted)
{
	ASSERT_VALID(this);

	if (!m_rect.IsRectEmpty() && !m_strLabel.IsEmpty())
	{
		CBCGPRect rect = m_rect;
		CBCGPRect rectShape = m_rect;

		if (!m_brushBackground.IsEmpty() || !m_colorBorder.IsNull())
		{
			rectShape.DeflateRect(pTagCloud->GetHorzMargin() / 2, 0);
			rectShape.top = rectShape.CenterPoint().y - (m_dblTextHeight + pTagCloud->GetVertMargin()) / 2;
			rectShape.bottom = rectShape.top + m_dblTextHeight + pTagCloud->GetVertMargin();
		}

		if (!m_brushBackground.IsEmpty())
		{
			pGM->FillRectangle(rectShape, m_brushBackground);
		}

		rect.DeflateRect(pTagCloud->GetHorzMargin(), pTagCloud->GetVertMargin());

		BOOL bIsSelected = IsSelected();

		CBCGPColor clrText = (bIsPressed || bIsHighlighted || bIsSelected) ? m_colorHighlightedText : m_colorText;

		if (clrText.IsNull())
		{
			clrText = (bIsPressed || bIsHighlighted || bIsSelected) ? pTagCloud->GetHighlightedTextColor() : pTagCloud->GetTextColor();
		}

		pGM->DrawText(m_strLabel, rect, tf, CBCGPBrush(clrText));

		if (!m_colorBorder.IsNull())
		{
			pGM->DrawRectangle(rectShape, CBCGPBrush(m_colorBorder));
		}
	}
}
//*****************************************************************************************
void CBCGPTagCloudElement::SetValue(double value) 
{ 
	ASSERT_VALID(this);

	m_dblValue = value;

	if (m_pOwner != NULL)
	{
		m_pOwner->m_bSortTags = TRUE;
		m_pOwner->m_bCalcWeights = TRUE;

		m_pOwner->m_dblMinVal = DBL_MAX;
		m_pOwner->m_dblMaxVal = DBL_MIN;

		m_pOwner->SetDirty();
	}
}
//*****************************************************************************************
void CBCGPTagCloudElement::SetLabel(const CString& label) 
{ 
	ASSERT_VALID(this);

	m_strLabel = label; 

	if (m_pOwner != NULL)
	{
		m_pOwner->m_bSortTags = TRUE;
		m_pOwner->SetDirty();
	}
}
//*****************************************************************************************
void CBCGPTagCloudElement::Redraw()
{
	ASSERT_VALID(this);

	if (m_pOwner != NULL)
	{
		m_pOwner->Redraw();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPTagCloud

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CBCGPTagCloud, CBCGPBaseVisualObject)

CBCGPTagCloud::CBCGPTagCloud()
{
	m_SortOrder = NoSort;
	m_bDescending = FALSE;

	m_bCalcWeights = TRUE;

	m_dblMinVal = DBL_MAX;
	m_dblMaxVal = DBL_MIN;

	m_nHorzMargin = 5.;
	m_nVertMargin = 5.;

	m_nHorzMarginOriginal = m_nHorzMargin;
	m_nVertMarginOriginal = m_nVertMargin;

	m_dblFontSizeStep = 0.0;

	LOGFONT lf;
	globalData.fontRegular.GetLogFont(&lf);

	lf.lfHeight = lf.lfHeight * 2;
	m_tfBase = CBCGPTextFormat(lf);

	m_tfBase.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
	m_tfBase.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);

	m_clrText = CBCGPColor::Black;
	m_clrHighlightedText = CBCGPColor::SteelBlue;
	m_brFill = CBCGPBrush(CBCGPColor::White);

	SetMaxWeight(5);

	m_pHighlighted = NULL;
	m_pPressed = NULL;
	m_pSelected = NULL;

	m_bSortTags = FALSE;
	m_bIsInteractiveMode = TRUE;
}
//*****************************************************************************************
CBCGPTagCloud::~CBCGPTagCloud()
{
	RemoveAll();
}
//*****************************************************************************************
void CBCGPTagCloud::SetSortOrder(SortOrder nOrder)
{
	if (m_SortOrder == nOrder)
	{
		return;
	}

	m_SortOrder = nOrder;
	m_bSortTags = TRUE;
	SetDirty();
}
//*****************************************************************************************
void CBCGPTagCloud::SetSortDescending(BOOL bSet)
{
	if (m_bDescending == bSet)
	{
		return;
	}

	m_bDescending = bSet;
	m_bSortTags = TRUE;
	SetDirty();
}
//*****************************************************************************************
void CBCGPTagCloud::SetMaxWeight(int nMaxWeight)
{
	ASSERT(nMaxWeight > 0);
	m_nMaxWeight = nMaxWeight;

	RebuildTextFormats();
	m_bCalcWeights = TRUE;
}
//*****************************************************************************************
void CBCGPTagCloud::SetTextFormat(const CBCGPTextFormat& tf)
{
	m_tfBase = tf;
	RebuildTextFormats();
}
//*****************************************************************************************
void CBCGPTagCloud::SetFontSizeStep(double dblStep)
{
	m_dblFontSizeStep = dblStep;
	RebuildTextFormats();
}
//*****************************************************************************************
void CBCGPTagCloud::SetHorzMargin(double nMargin)
{
	m_nHorzMargin = nMargin;
	m_nHorzMarginOriginal = m_nHorzMargin;

	SetDirty();
}
//*****************************************************************************************
void CBCGPTagCloud::SetVertMargin(double nMargin)
{
	m_nVertMargin = nMargin;
	m_nVertMarginOriginal = m_nVertMargin;

	SetDirty();
}
//*****************************************************************************************
void CBCGPTagCloud::RebuildTextFormats()
{
	m_Fonts.RemoveAll();
	m_Fonts.SetSize(m_nMaxWeight);

	float fBaseSize = m_tfBase.GetFontSize();
	float fFontSizeStep = m_dblFontSizeStep == 0.0 ? fBaseSize / m_nMaxWeight : (float)m_dblFontSizeStep;

	if (m_dblFontSizeStep != 0.0 && fBaseSize < 0.0)
	{
		fFontSizeStep = -fFontSizeStep;
	}

	for (int i = 0; i < m_nMaxWeight; i++)
	{
		const float fSize = fBaseSize + fFontSizeStep * (i - m_nMaxWeight / 2);

		m_Fonts[i] = CBCGPTextFormat(m_tfBase.GetFontFamily(), fSize, m_tfBase.GetFontWeight(), m_tfBase.GetFontStyle(), m_tfBase.GetFontLocale());

		m_Fonts[i].SetTextAlignment(m_tfBase.GetTextAlignment());
		m_Fonts[i].SetTextVerticalAlignment(m_tfBase.GetTextVerticalAlignment());

		m_Fonts[i].SetUnderline(m_tfBase.IsUnderline());
		m_Fonts[i].SetStrikethrough(m_tfBase.IsStrikethrough());
	}

	SetDirty();
}
//*****************************************************************************************
void CBCGPTagCloud::Add(CBCGPTagCloudElement* pTag)
{
	pTag->m_pOwner = this;

	m_lstTags.AddTail(pTag);

	AddSorted(pTag);

	m_dblMinVal = min(m_dblMinVal, pTag->m_dblValue);
	m_dblMaxVal = max(m_dblMaxVal, pTag->m_dblValue);

	SetDirty();

	m_bCalcWeights = TRUE;
}
//*****************************************************************************************
void CBCGPTagCloud::RemoveAll()
{
	while (!m_lstTags.IsEmpty())
	{
		delete m_lstTags.RemoveHead();
	}

	m_lstTagsSorted.RemoveAll();

	m_dblMinVal = DBL_MAX;
	m_dblMaxVal = DBL_MIN;

	m_pSelected = m_pHighlighted = m_pPressed = NULL;

	SetDirty();
}
//*****************************************************************************************
void CBCGPTagCloud::AdjustRowHeight(CArray<CBCGPTagCloudElement*, CBCGPTagCloudElement*>& arRow, double nRowHeight)
{
	for (int i = 0; i < arRow.GetSize(); i++)
	{
		CBCGPTagCloudElement* pTag = arRow[i];
		pTag->m_rect.bottom = pTag->m_rect.top + nRowHeight;
	}

	arRow.RemoveAll();
}
//*****************************************************************************************
void CBCGPTagCloud::RecalcWeights()
{
	if (m_lstTags.IsEmpty())
	{
		return;
	}

	POSITION pos = NULL;

	if (m_dblMinVal == DBL_MAX || m_dblMaxVal == DBL_MIN)
	{
		for (pos = m_lstTags.GetHeadPosition(); pos != NULL;)
		{
			CBCGPTagCloudElement* pTag = m_lstTags.GetNext(pos);
			ASSERT_VALID(pTag);

			m_dblMinVal = min(m_dblMinVal, pTag->m_dblValue);
			m_dblMaxVal = max(m_dblMaxVal, pTag->m_dblValue);
		}
	}

	double dblWeightStep = (m_dblMaxVal - m_dblMinVal) / m_nMaxWeight;

	for (pos = m_lstTags.GetHeadPosition(); pos != NULL;)
	{
		CBCGPTagCloudElement* pTag = m_lstTags.GetNext(pos);
		ASSERT_VALID(pTag);

		if (dblWeightStep == 0)
		{
			pTag->m_nWeight = 1;
		}
		else
		{
			pTag->m_nWeight = min(m_nMaxWeight - 1, (int)((pTag->m_dblValue - m_dblMinVal) / dblWeightStep));
		}

		ASSERT(pTag->m_nWeight >= 0);
		ASSERT(pTag->m_nWeight < m_nMaxWeight);
	}
}
//*****************************************************************************************
void CBCGPTagCloud::RecalcLayout(CBCGPGraphicsManager* pGM)
{
	if (m_lstTags.IsEmpty())
	{
		return;
	}

	POSITION pos = NULL;

	if (m_bSortTags)
	{
		m_lstTagsSorted.RemoveAll();

		for (pos = m_lstTags.GetHeadPosition(); pos != NULL;)
		{
			CBCGPTagCloudElement* pTag = m_lstTags.GetNext(pos);
			ASSERT_VALID(pTag);

			AddSorted(pTag);
		}

		m_bSortTags = FALSE;
	}

	if (m_bCalcWeights)
	{
		RecalcWeights();
		m_bCalcWeights = FALSE;
	}

	CBCGPRect rectClient = m_rect;
	rectClient.DeflateRect(m_nHorzMargin, m_nVertMargin);

	double x = rectClient.left;
	double y = rectClient.top;

	double nRowHeight = 0;

	CArray<CBCGPTagCloudElement*, CBCGPTagCloudElement*> arRow;

	for (pos = m_lstTagsSorted.GetHeadPosition(); pos != NULL;)
	{
		CBCGPTagCloudElement* pTag = m_lstTagsSorted.GetNext(pos);
		ASSERT_VALID(pTag);

		ASSERT(pTag->m_nWeight >= 0);
		ASSERT(pTag->m_nWeight < m_nMaxWeight);

		CBCGPSize size = pGM->GetTextSize(pTag->m_strLabel, m_Fonts[pTag->m_nWeight]);

		pTag->m_dblTextHeight = size.cy;

		size.cx += 2 * m_nHorzMargin;
		size.cy += 2 * m_nVertMargin;

		if (nRowHeight > 0 && x + size.cx > rectClient.right)
		{
			y += nRowHeight;
			x = rectClient.left;
			
			AdjustRowHeight(arRow, nRowHeight);

			nRowHeight = 0;
		}

		if (y + size.cy > m_rect.bottom)
		{
			pTag->m_rect.SetRectEmpty();
		}
		else
		{
			pTag->m_rect = CBCGPRect(CBCGPPoint(x, y), size);
		}

		nRowHeight = max(nRowHeight, size.cy);

		x += size.cx;
		arRow.Add(pTag);
	}

	AdjustRowHeight(arRow, nRowHeight);
}
//*****************************************************************************************
void CBCGPTagCloud::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip, DWORD dwFlags)
{
	if (IsDirty())
	{
		RecalcLayout(pGM);
		SetDirty(FALSE);
	}

	if ((dwFlags & BCGP_DRAW_DYNAMIC) == 0)
	{
		return;
	}

	pGM->FillRectangle(m_rect, m_brFill);

	for (POSITION pos = m_lstTagsSorted.GetHeadPosition(); pos != NULL;)
	{
		CBCGPTagCloudElement* pTag = m_lstTagsSorted.GetNext(pos);
		ASSERT_VALID(pTag);

		CBCGPRect rectInter;
		if (rectInter.IntersectRect(pTag->m_rect, rectClip))
		{
			ASSERT(pTag->m_nWeight >= 0);
			ASSERT(pTag->m_nWeight < m_nMaxWeight);

			pTag->DoDraw(this, pGM, m_Fonts[pTag->m_nWeight], pTag == m_pPressed, pTag == m_pHighlighted);
		}
	}
}
//*****************************************************************************************
BOOL CBCGPTagCloud::OnKeyboardDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_lstTagsSorted.IsEmpty())
	{
		return FALSE;
	}

	CBCGPTagCloudElement* pNewSelected = NULL;
	CBCGPTagCloudElement* pPrevSelected = m_pSelected;

	switch (nChar)
	{
	case VK_LEFT:
	case VK_UP:
		if (m_pSelected == NULL || m_pSelected == m_lstTagsSorted.GetHead())
		{
			pNewSelected = m_lstTagsSorted.GetTail();
		}
		else
		{
			POSITION pos = m_lstTagsSorted.Find(m_pSelected);
			ASSERT(pos != NULL);

			m_lstTagsSorted.GetPrev(pos);
			pNewSelected = m_lstTagsSorted.GetAt(pos);
		}
		break;

	case VK_RIGHT:
	case VK_DOWN:
		if (m_pSelected == NULL || m_pSelected == m_lstTagsSorted.GetTail())
		{
			pNewSelected = m_lstTagsSorted.GetHead();
		}
		else
		{
			POSITION pos = m_lstTagsSorted.Find(m_pSelected);
			ASSERT(pos != NULL);
			
			m_lstTagsSorted.GetNext(pos);
			pNewSelected = m_lstTagsSorted.GetAt(pos);
		}
		break;

	case VK_RETURN:
	case VK_SPACE:
		if (m_pSelected != NULL)
		{
			OnClickTag(m_pSelected);
			return TRUE;
		}
		break;

	case VK_HOME:
		pNewSelected = m_lstTagsSorted.GetHead();
		break;

	case VK_END:
		pNewSelected = m_lstTagsSorted.GetTail();
		break;
	}

	if (pNewSelected != m_pSelected && pNewSelected != NULL)
	{
		m_pSelected = pNewSelected;

		RedrawTag(m_pSelected);
		RedrawTag(pPrevSelected);

		return TRUE;
	}

	return CBCGPBaseVisualObject::OnKeyboardDown(nChar, nRepCnt, nFlags);
}
//*****************************************************************************************
BOOL CBCGPTagCloud::OnMouseDown(int nButton, const CBCGPPoint& pt)
{
	if (CBCGPPopupMenu::GetActiveMenu() == NULL && nButton == 0)
	{
		m_pPressed = HitTestTag(pt);
	}

	return CBCGPBaseVisualObject::OnMouseDown(nButton, pt);
}
//*****************************************************************************************
void CBCGPTagCloud::OnMouseUp(int nButton, const CBCGPPoint& pt)
{
	CBCGPTagCloudElement* pClickedTag = NULL;

	if (nButton == 0)
	{
		if (m_pPressed == m_pHighlighted && m_pHighlighted != NULL)
		{
			pClickedTag = m_pHighlighted;
		}

		m_pPressed = m_pHighlighted = NULL;
	}

	if (pClickedTag != NULL)
	{
		RedrawTag(pClickedTag);

		OnClickTag(pClickedTag);
	}

	CBCGPBaseVisualObject::OnMouseUp(nButton, pt);
}
//*****************************************************************************************
void CBCGPTagCloud::OnMouseMove(const CBCGPPoint& pt)
{
	CBCGPBaseVisualObject::OnMouseMove(pt);

	if (CBCGPPopupMenu::GetActiveMenu() != NULL)
	{
		return;
	}

	CBCGPTagCloudElement* pOldHighlighted = m_pHighlighted;
	m_pHighlighted = HitTestTag(pt);

	if (m_pHighlighted != pOldHighlighted)
	{
		RedrawTag(pOldHighlighted);
		RedrawTag(m_pHighlighted);
	}
}
//*****************************************************************************************
void CBCGPTagCloud::OnMouseLeave()
{
	CBCGPBaseVisualObject::OnMouseLeave();

	CBCGPTagCloudElement* pOldHighlighted = m_pHighlighted;
	m_pHighlighted = NULL;

	RedrawTag(pOldHighlighted);
}
//*****************************************************************************************
BOOL CBCGPTagCloud::OnSetMouseCursor(const CBCGPPoint& pt)
{
	if (CBCGPPopupMenu::GetActiveMenu() == NULL && HitTestTag(pt) != NULL)
	{
		::SetCursor (globalData.GetHandCursor ());
		return TRUE;
	}

	return CBCGPBaseVisualObject::OnSetMouseCursor(pt);
}
//*****************************************************************************************
void CBCGPTagCloud::OnCancelMode()
{
	CBCGPBaseVisualObject::OnCancelMode();

	CBCGPTagCloudElement* pOldHighlighted = m_pHighlighted;
	m_pHighlighted = NULL;

	RedrawTag(pOldHighlighted);
}
//*****************************************************************************************
BOOL CBCGPTagCloud::OnGetToolTip(const CBCGPPoint& pt, CString& strToolTip, CString& strDescr)
{
	if (CBCGPPopupMenu::GetActiveMenu() == NULL)
	{
		CBCGPTagCloudElement* pTag = HitTestTag(pt);
		if (pTag != NULL)
		{
			strToolTip.Format(_T("%s: %lf"), pTag->m_strLabel, pTag->m_dblValue);
			return TRUE;
		}
	}

	return CBCGPBaseVisualObject::OnGetToolTip(pt, strToolTip, strDescr);
}
//*****************************************************************************************
CBCGPTagCloudElement* CBCGPTagCloud::HitTestTag(const CBCGPPoint& point) const
{
	for (POSITION pos = m_lstTagsSorted.GetHeadPosition(); pos != NULL;)
	{
		CBCGPTagCloudElement* pTag = m_lstTagsSorted.GetNext(pos);
		if (pTag->m_rect.PtInRect(point))
		{
			return pTag;
		}
	}

	return NULL;
}
//*****************************************************************************************
void CBCGPTagCloud::RedrawTag(CBCGPTagCloudElement* pTag)
{
	if (pTag != NULL)
	{
		GetOwner()->RedrawWindow((CRect)pTag->m_rect);
	}
}
//*****************************************************************************************
void CBCGPTagCloud::SetFillBrush(const CBCGPBrush& brFill)
{
	m_brFill = brFill;
	Redraw();
}
//*****************************************************************************************
void CBCGPTagCloud::SetTextColor(const CBCGPColor& clrText)
{
	m_clrText = clrText;
	Redraw();
}
//*****************************************************************************************
void CBCGPTagCloud::SetHighlightedTextColor(const CBCGPColor& clrText)
{
	m_clrHighlightedText = clrText;
}
//*****************************************************************************************
void CBCGPTagCloud::AddSorted(CBCGPTagCloudElement* pTagNew)
{
	if (m_SortOrder == NoSort)
	{
		m_lstTagsSorted.AddTail(pTagNew);
		return;
	}

	if (!m_bDescending)
	{
		for (POSITION pos = m_lstTagsSorted.GetHeadPosition(); pos != NULL;)
		{
			POSITION posSaved = pos;

			CBCGPTagCloudElement* pTag = m_lstTagsSorted.GetNext(pos);
			ASSERT_VALID(pTag);

			switch (m_SortOrder)
			{
			case AlphabeticalSort:
				if (pTag->m_strLabel.CompareNoCase(pTagNew->m_strLabel) > 0)
				{
					m_lstTagsSorted.InsertBefore(posSaved, pTagNew);
					return;
				}
				break;

			case SortByValue:
				if (pTag->m_dblValue > pTagNew->m_dblValue)
				{
					m_lstTagsSorted.InsertBefore(posSaved, pTagNew);
					return;
				}
				break;
			}
		}

		m_lstTagsSorted.AddTail(pTagNew);
	}
	else
	{
		for (POSITION pos = m_lstTagsSorted.GetTailPosition(); pos != NULL;)
		{
			POSITION posSaved = pos;

			CBCGPTagCloudElement* pTag = m_lstTagsSorted.GetPrev(pos);
			ASSERT_VALID(pTag);

			switch (m_SortOrder)
			{
			case AlphabeticalSort:
				if (pTag->m_strLabel.CompareNoCase(pTagNew->m_strLabel) > 0)
				{
					m_lstTagsSorted.InsertAfter(posSaved, pTagNew);
					return;
				}
				break;

			case SortByValue:
				if (pTag->m_dblValue > pTagNew->m_dblValue)
				{
					m_lstTagsSorted.InsertAfter(posSaved, pTagNew);
					return;
				}
				break;
			}
		}

		m_lstTagsSorted.AddHead(pTagNew);
	}
}
//*****************************************************************************************
void CBCGPTagCloud::OnClickTag(CBCGPTagCloudElement* pClickedTag)
{
	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	CWnd* pOwner = m_pWndOwner->GetOwner();
	if (pOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	m_pWndOwner->GetOwner()->PostMessage(BCGM_ON_CLICK_TAG_CLOUD, (WPARAM)GetID(), (LPARAM)pClickedTag);
}
//*****************************************************************************************
void CBCGPTagCloud::OnScaleRatioChanged(const CBCGPSize& /*sizeScaleRatioOld*/)
{
	m_tfBase.Scale(m_sizeScaleRatio.cy);
	RebuildTextFormats();

	m_nHorzMargin = m_nHorzMarginOriginal * m_sizeScaleRatio.cx;
	m_nVertMargin = m_nVertMarginOriginal * m_sizeScaleRatio.cy;

	SetDirty();
}
//*****************************************************************************
CBCGPTagCloudElement* CBCGPTagCloud::GetAccChild(int nIndex)
{
	ASSERT_VALID(this);

	nIndex--;

	if (nIndex < 0 || nIndex >= (int)m_lstTagsSorted.GetCount())
	{
		return NULL;
	}

	POSITION pos = m_lstTagsSorted.FindIndex(nIndex);
	if (pos == NULL)
	{
		return NULL;
	}

	return m_lstTagsSorted.GetAt(pos);
}
//*****************************************************************************
long CBCGPTagCloud::GetAccChildIndex(CBCGPTagCloudElement* pTile)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pTile);

	long nIndex = 1;

	for (POSITION pos = m_lstTagsSorted.GetHeadPosition(); pos != NULL; nIndex++)
	{
		CBCGPTagCloudElement* pListTile = m_lstTagsSorted.GetNext(pos);
		ASSERT_VALID(pListTile);

		if (pListTile == pTile)
		{
			return nIndex;
		}
	}

	return 0;
}
//****************************************************************************
HRESULT CBCGPTagCloud::get_accChildCount(long *pcountChildren)
{
	if (!pcountChildren)
	{
		return E_INVALIDARG;
	}

	*pcountChildren = (long)m_lstTagsSorted.GetCount();
	return S_OK;
}
//****************************************************************************
HRESULT CBCGPTagCloud::get_accChild(VARIANT /*varChild*/, IDispatch** /*ppdispChild*/)
{
	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPTagCloud::get_accName(VARIANT varChild, BSTR *pszName)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CString strName = GetName();

		if (strName.IsEmpty())
		{
			m_pWndOwner->GetWindowText(strName);

			if (strName.IsEmpty())
			{
				strName = _T("Tag Cloud");
			}
		}

		*pszName = strName.AllocSysString();
		return S_OK;
	}

	if (varChild.vt == VT_I4)
	{
		CBCGPTagCloudElement* pAccTag = GetAccChild(varChild.lVal);
		if (pAccTag != NULL)
		{
			CString strName = pAccTag->GetLabel();
			*pszName = strName.AllocSysString();

			return S_OK;
		}
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPTagCloud::get_accValue(VARIANT varChild, BSTR *pszValue)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		return S_FALSE;
	}

	if (varChild.vt == VT_I4)
	{
		CBCGPTagCloudElement* pAccTag = GetAccChild(varChild.lVal);
		if (pAccTag != NULL)
		{
			CString strValue;
			strValue.Format(_T("%f"), pAccTag->GetValue());

			*pszValue = strValue.AllocSysString();
			return S_OK;
		}
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPTagCloud::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
	if (!pvarRole || ((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)))
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		pvarRole->vt = VT_I4;
		pvarRole->lVal = ROLE_SYSTEM_LIST;

		return S_OK;
	}

	CBCGPTagCloudElement* pAccTag = GetAccChild(varChild.lVal);
	if (pAccTag != NULL)
	{
		pvarRole->vt = VT_I4;
		pvarRole->lVal = ROLE_SYSTEM_LISTITEM;

		return S_OK;
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPTagCloud::get_accState(VARIANT varChild, VARIANT *pvarState)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		pvarState->vt = VT_I4;
		pvarState->lVal = STATE_SYSTEM_NORMAL;
		return S_OK;
	}

	pvarState->vt = VT_I4;
	pvarState->lVal = STATE_SYSTEM_FOCUSABLE;
	pvarState->lVal |= STATE_SYSTEM_SELECTABLE;

	if (IsSelected())
	{
		pvarState->lVal |= STATE_SYSTEM_SELECTED;
		pvarState->lVal |= STATE_SYSTEM_FOCUSED;
	}
	
	return S_OK;
}
//****************************************************************************
HRESULT CBCGPTagCloud::get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction)
{
	if (varChild.vt == VT_I4 && varChild.lVal > 0)
	{
		CBCGPTagCloudElement* pAccTag = GetAccChild(varChild.lVal);
		if (pAccTag != NULL)
		{
			CString strDefAction = _T("Open");
			*pszDefaultAction = strDefAction.AllocSysString();

			return S_OK;
		}
	}
	
	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPTagCloud::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
	if (!pxLeft || !pyTop || !pcxWidth || !pcyHeight)
	{
		return E_INVALIDARG;
	}

	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return S_FALSE;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CRect rc;
		m_pWndOwner->GetWindowRect(rc);

		*pxLeft = rc.left;
		*pyTop = rc.top;
		*pcxWidth = rc.Width();
		*pcyHeight = rc.Height();

		return S_OK;
	}
	else if (varChild.vt == VT_I4)
	{
		CBCGPTagCloudElement* pAccTag = GetAccChild(varChild.lVal);
		if (pAccTag != NULL)
		{
			CRect rcProp = pAccTag->GetRect();
			m_pWndOwner->ClientToScreen(&rcProp);

			*pxLeft = rcProp.left;
			*pyTop = rcProp.top;
			*pcxWidth = rcProp.Width();
			*pcyHeight = rcProp.Height();
		}
	}

	return S_OK;
}
//****************************************************************************
HRESULT CBCGPTagCloud::accHitTest(long  xLeft, long yTop, VARIANT *pvarChild)
{
	if (!pvarChild)
	{
		return E_INVALIDARG;
	}

	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
        pvarChild->vt = VT_EMPTY;
		return S_FALSE;
	}

	CPoint pt(xLeft, yTop);
	m_pWndOwner->ScreenToClient(&pt);

	pvarChild->vt = VT_I4;

	CBCGPTagCloudElement* pTile = HitTestTag(pt);
	if (pTile != NULL)
	{
		pvarChild->lVal = GetAccChildIndex(pTile);
	}
	else
	{
		pvarChild->lVal = CHILDID_SELF;
	}

	return S_OK;
}
//******************************************************************************
HRESULT CBCGPTagCloud::accDoDefaultAction(VARIANT varChild)
{
    if (varChild.vt != VT_I4)
    {
        return E_INVALIDARG;
    }

	CBCGPTagCloudElement* pAccTag = GetAccChild(varChild.lVal);
	if (pAccTag != NULL)
	{
		OnClickTag(pAccTag);
		return S_OK;
    }

    return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPTagCloudCtrl

CBCGPTagCloudCtrl::CBCGPTagCloudCtrl()
{
	m_pCloud = NULL;
}
//*****************************************************************************************
CBCGPTagCloudCtrl::~CBCGPTagCloudCtrl()
{
	if (m_pCloud != NULL)
	{
		delete m_pCloud;
	}
}

BEGIN_MESSAGE_MAP(CBCGPTagCloudCtrl, CBCGPVisualCtrl)
	//{{AFX_MSG_MAP(CBCGPTagCloudCtrl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPTagCloudCtrl message handlers
