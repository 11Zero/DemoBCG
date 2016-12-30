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
// BCGPReportCtrl.cpp: implementation of the CBCGPReportCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPReportCtrl.h"

#ifndef BCGP_EXCLUDE_GRID_CTRL

#include "bcgglobals.h"

#ifndef _BCGPGRID_STANDALONE
	#include "BCGPVisualManager.h"
#endif

#ifndef _BCGPGRID_STANDALONE
#ifndef _BCGSUITE_
	extern CBCGPWorkspace* g_pWorkspace;
#endif
	#define visualManager	CBCGPVisualManager::GetInstance ()
#else
	#define visualManager	CBCGPGridVisualManager::GetInstance ()
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define TEXT_MARGIN		3

/////////////////////////////////////////////////////////////////////////////
// CBCGPReportRow object

IMPLEMENT_DYNAMIC(CBCGPReportRow, CBCGPGridRow)

CBCGPReportRow::CBCGPReportRow() : m_nPreviewHeight (-1)
{
	m_bGroup = FALSE;
}
//******************************************************************************************
CBCGPReportRow::CBCGPReportRow (const CString& strGroupName, DWORD dwData)
	: CBCGPGridRow (0, dwData), m_strName (strGroupName), m_nPreviewHeight (-1)
{
	m_bGroup = TRUE;
}
//******************************************************************************************
CBCGPReportRow::~CBCGPReportRow()
{
}
//******************************************************************************************
void CBCGPReportRow::OnDrawName (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);

	if (HasValueField ())
	{
		if (m_pWndList->IsPreviewRowEnabled ())
		{
			OnDrawPreview (pDC, rect);
		}
		return;
	}

	// Draw group row:
	COLORREF clrTextOld = pDC->GetTextColor ();

	COLORREF clrText = (COLORREF)-1;
	if (IsSelected ())
	{
		CRect rectFill = rect;
		rectFill.top++;
		if (IsGroup ())
		{
			rectFill.DeflateRect (0, 0, 0, 1);
		}

		if (m_pWndList->m_ColorData.m_GroupColors.m_clrText != (COLORREF)-1)
		{
			clrText = m_pWndList->m_ColorData.m_GroupColors.m_clrText;
		}
		else
		{
			clrText = visualManager->OnFillReportCtrlRowBackground (
				m_pWndList, pDC, rectFill, IsSelected (), IsGroup ());
		}
	}
	else
	{
		if (IsGroup () && !HasValueField ())
		{
			if (m_pWndList->m_ColorData.m_GroupColors.m_clrText != (COLORREF)-1)
			{
				clrText = m_pWndList->m_ColorData.m_GroupColors.m_clrText;
			}
			else
			{
				clrText = visualManager->OnFillReportCtrlRowBackground (
					m_pWndList, pDC, rect, IsSelected (), IsGroup () && !HasValueField ());
			}
		}
		else
		{
			clrText = visualManager->OnFillReportCtrlRowBackground (
				m_pWndList, pDC, rect, IsSelected (), IsGroup () && !HasValueField ());
		}
	}

	rect.DeflateRect (TEXT_MARGIN, 0);
	if (IsGroup ())
	{
		int nDeflate = (m_pWndList->m_nLargeRowHeight - m_pWndList->m_nRowHeight) / 2;
		if (nDeflate > 0)
		{
			rect.DeflateRect (0, nDeflate, 0, 0);
		}
	}

	if (clrText != (COLORREF)-1)
	{
		pDC->SetTextColor (clrText);
	}

	pDC->DrawText (m_strName, rect, 
		DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

	if (clrText != (COLORREF)-1)
	{
		pDC->SetTextColor (clrTextOld);
	}

	m_bNameIsTrancated = pDC->GetTextExtent (m_strName).cx > rect.Width ();
}
//******************************************************************************************
void CBCGPReportRow::OnDrawPreview (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);

	ASSERT (HasValueField ());

	int dx = m_pWndList->IsSortingMode () && !m_pWndList->IsGrouping () ? 0 : GetHierarchyLevel () * m_pWndList->GetHierarchyLevelOffset ();

	// Fill background for the preview area:
	CRect rectDescr = rect;
	rectDescr.top++;
	rectDescr.left = rect.left + m_pWndList->GetExtraHierarchyOffset () + dx;
	
	COLORREF clrText = m_pWndList->GetTextColor ();
	if (IsSelected ())
	{
		clrText = m_pWndList->OnFillSelItem (pDC, rectDescr, NULL);
	}
	else
	{
		if (m_pWndList->m_brBackground.GetSafeHandle () != NULL)
		{
			pDC->FillRect (rectDescr, &m_pWndList->m_brBackground);
		}
		else
		{
			COLORREF clr = visualManager->OnFillGridItem (
				m_pWndList, pDC, rectDescr, IsSelected (), FALSE/*bActiveItem*/, FALSE);
			if (clrText == (COLORREF)-1)
			{
				clrText = clr;
			}
		}
	}

	// Draw text:
	CRect rectMargins = m_pWndList->OnGetPreviewRowMargins (this);
	rectDescr.DeflateRect (rectMargins);

	COLORREF clrTextOld = (COLORREF)-1;
	COLORREF clrPreviewText = m_pWndList->GetPreviewTextColor (IsSelected ());
	if (clrPreviewText != (COLORREF)-1)
	{
		clrTextOld = pDC->SetTextColor (clrPreviewText);
	}
	else if (clrText != (COLORREF)-1)
	{
		clrTextOld = pDC->SetTextColor (clrText);
	}

	CString strDescription;
	GetPreviewText (strDescription);

	pDC->DrawText (strDescription, rectDescr, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);

	if (clrText != (COLORREF)-1)
	{
		pDC->SetTextColor (clrTextOld);
	}
}
//******************************************************************************************
void CBCGPReportRow::OnDrawExpandBox (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);
	ASSERT (IsGroup ());

	if (m_pWndList->m_bIsPrinting)
	{
		// map to printer metrics
		ASSERT_VALID (m_pWndList->m_pPrintDC);
		CSize szOne = m_pWndList->m_PrintParams.m_pageInfo.m_szOne;

		CRect rectFill = rect;
		rectFill.top += szOne.cx;
		if (IsGroup ())
		{
			rectFill.DeflateRect (0, 0, 0, szOne.cy);
		}

		CBrush brGroupBG (m_pWndList->m_clrPrintGroupBG);
		pDC->FillRect (rectFill, &brGroupBG);
	}
	else
	{
		if (IsSelected ())
		{
			CRect rectFill = rect;
			rectFill.top++;
			if (IsGroup ())
			{
				rectFill.DeflateRect (0, 0, 0, 1);
			}

			if (m_pWndList->m_ColorData.m_GroupColors.m_clrText == (COLORREF)-1)
			{
				visualManager->OnFillReportCtrlRowBackground (
					m_pWndList, pDC, rectFill, IsSelected (), IsGroup ());
			}
		}
	}

	if (IsGroup ())
	{
		int nDeflate = (m_pWndList->m_nLargeRowHeight - m_pWndList->m_nRowHeight) / 2;
		if (nDeflate > 0)
		{
			rect.DeflateRect (0, nDeflate, 0, 0);
		}
	}

	CBCGPGridRow::OnDrawExpandBox (pDC, rect);
}
//******************************************************************************************
void CBCGPReportRow::OnPrintName (CDC* pDC, CRect rect)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);
	ASSERT_VALID (m_pWndList);
	ASSERT (m_pWndList->m_bIsPrinting);

	if (HasValueField ())
	{
		return;
	}

	// map to printer metrics
	HDC hDCFrom = ::GetDC(NULL);
	int nXMul = pDC->GetDeviceCaps(LOGPIXELSX);			// pixels in print dc
	int nXDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSX);	// pixels in screen dc
	::ReleaseDC(NULL, hDCFrom);

	const int CALCULATED_TEXT_MARGIN = ::MulDiv (TEXT_MARGIN, nXMul, nXDiv);
	const CRect& rectClip = m_pWndList->m_PrintParams.m_pageInfo.m_rectPageItems;
	const CSize& szOne = m_pWndList->m_PrintParams.m_pageInfo.m_szOne;

	//----------------
	// Fill background
	//----------------
	CRect rectFill = rect;
	rectFill.top += szOne.cy;
	if (IsGroup ())
	{
		rectFill.DeflateRect (0, 0, 0, szOne.cy);
	}

	CRect rectClipFill = rectFill;
	rectClipFill.NormalizeRect ();
	rectClipFill.IntersectRect (&rectClip, &rectClipFill);

	if (rectClipFill.Width () > 0)
	{
		CBrush brGroupBG (m_pWndList->m_clrPrintGroupBG);
		pDC->FillRect (rectClipFill, &brGroupBG);
	}

	//----------
	// Draw text
	//----------
	CRect rectText = rect;
	rectText.DeflateRect (CALCULATED_TEXT_MARGIN, 0);
	if (IsGroup ())
	{
		int nDeflate = (m_pWndList->m_nLargeRowHeight - m_pWndList->m_nRowHeight) / 2;
		if (nDeflate > 0)
		{
			rectText.DeflateRect (0, nDeflate, 0, 0);
		}
	}

	CRect rectClipText = rectText;
	rectClipText.NormalizeRect ();
	if (rectClipText.IntersectRect (&rectClipText, &rectClip))
	{
		COLORREF clrTextOld = pDC->SetTextColor (m_pWndList->m_clrPrintText);

		// Draw text vertically centered
		ASSERT_VALID (m_pWndList->m_pPrintDC);

		TEXTMETRIC tm;
		m_pWndList->m_pPrintDC->GetTextMetrics (&tm);
		int nDescent = tm.tmDescent;
		int nVCenterOffset = (rectText.Height () - pDC->GetTextExtent (m_strName).cy + nDescent) / 2;

		pDC->SetTextAlign (TA_LEFT | TA_TOP);
		pDC->ExtTextOut (rectText.left, rectText.top + nVCenterOffset, ETO_CLIPPED, &rectClipText, m_strName, NULL);

		pDC->SetTextColor (clrTextOld);
	}
}
//******************************************************************************************
int CBCGPReportRow::GetHierarchyLevel () const
{
	ASSERT_VALID (this);

	int nLevel = CBCGPGridRow::GetHierarchyLevel ();

	if (!IsGroup () && nLevel > 0)
	{
		nLevel--;
	}

	return nLevel;
}
//******************************************************************************************
CString CBCGPReportRow::GetName ()
{
	return m_strName;
}
//******************************************************************************************
CRect CBCGPReportRow::GetNameTooltipRect ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pWndList);

	CRect rectName = CBCGPGridRow::GetNameTooltipRect ();

	if (IsGroup ())
	{
		rectName.DeflateRect (0, (m_pWndList->GetLargeRowHeight () - m_pWndList->GetRowHeight ()) / 2, 0, 0);
	}

	return rectName;
}
//******************************************************************************************
void CBCGPReportRow::GetPreviewText (CString& str) const
{
	str = GetDescription ();
}
//******************************************************************************************
void CBCGPReportRow::OnMeasureGridRowRect (CRect& rect)
{
	CBCGPGridRow::OnMeasureGridRowRect (rect);

	if (m_pWndList->m_bIsPrinting)
	{
		return;
	}

	if (m_nPreviewHeight < 0 && !rect.IsRectEmpty ())
	{
		m_nPreviewHeight = CalcPreview (rect);
	}

	if (m_nPreviewHeight > 0)
	{
		rect.bottom += m_nPreviewHeight;
	}
}
//******************************************************************************************
int CBCGPReportRow::CalcPreview (const CRect& rect)
{
	CBCGPReportCtrl* pReportWnd = DYNAMIC_DOWNCAST (CBCGPReportCtrl, m_pWndList);

	if (pReportWnd->GetSafeHwnd () == NULL)
	{
		return 0;
	}

	ASSERT_VALID (pReportWnd);

	if (pReportWnd->m_pPreviewDC == NULL)
	{
		pReportWnd->m_pPreviewDC = new CClientDC (pReportWnd);
		pReportWnd->SetCurrFont (pReportWnd->m_pPreviewDC);
	}

	ASSERT_VALID (pReportWnd->m_pPreviewDC);

	if (!IsGroup () &&
		pReportWnd->IsRowExtraHeightAllowed () && pReportWnd->m_bPreviewRow)
	{
		CString strDescription;
		GetPreviewText (strDescription);
		if (strDescription.IsEmpty ())
		{
			return 0;
		}

		CRect rectDescr = rect;
		if (!m_pWndList->m_bIsPrinting)
		{
			int dx = m_pWndList->IsSortingMode () && !m_pWndList->IsGrouping () ? 0 : GetHierarchyLevel () * m_pWndList->GetHierarchyLevelOffset ();

			rectDescr.top++;
			rectDescr.left = rect.left + m_pWndList->GetExtraHierarchyOffset () + dx;

			CRect rectMargins = m_pWndList->OnGetPreviewRowMargins (this);
			rectDescr.DeflateRect (rectMargins);
		}
		const int nMaxHeight = (m_pWndList->m_bIsPrinting ? pReportWnd->m_nPrintPreviewRowHeight : pReportWnd->m_nPreviewRowHeight);

		rectDescr.top = rect.bottom;
		rectDescr.bottom = rectDescr.bottom + nMaxHeight;

		int nHeight = pReportWnd->m_pPreviewDC->DrawText (strDescription, rectDescr, DT_CALCRECT | DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);

		if (nHeight > 0)
		{
			int nBottomMargin = (m_pWndList->m_bIsPrinting ? 2 * m_pWndList->m_PrintParams.m_pageInfo.m_szOne.cy : 2);
			return min (nHeight + nBottomMargin, nMaxHeight);
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPReportCtrl window

IMPLEMENT_DYNAMIC(CBCGPReportCtrl, CBCGPGridCtrl)

CBCGPReportCtrl::CBCGPReportCtrl()
{
	m_bDrawFocusRect = TRUE;
	m_bWholeRowSel = TRUE;

	m_bAllowRowExtraHeight = TRUE;

	m_bPreviewRow = FALSE;
	m_nPreviewRowMaxLines = 3;
	m_nPreviewRowHeight = 0;
	m_nPrintPreviewRowHeight = 0;
	m_nPreviewRowLeftMargin = -1;
	m_nPreviewRowRightMargin = -1;

	m_pPreviewDC = NULL;

	m_bGridItemBorders = FALSE;
	m_bGridLines = FALSE;
}

CBCGPReportCtrl::~CBCGPReportCtrl()
{
	if (m_pPreviewDC != NULL)
	{
		delete m_pPreviewDC;
		m_pPreviewDC = NULL;
	}
}

BEGIN_MESSAGE_MAP(CBCGPReportCtrl, CBCGPGridCtrl)
	//{{AFX_MSG_MAP(CBCGPReportCtrl)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CBCGPReportCtrl::SetRowHeight ()
{
	CBCGPGridCtrl::SetRowHeight ();

	if (m_bIsPrinting)
	{
		ASSERT_VALID (m_pPrintDC);

		// map to printer metrics
		HDC hDCFrom = ::GetDC(NULL);
		int nYMul = m_pPrintDC->GetDeviceCaps(LOGPIXELSY);	// pixels in print dc
		int nYDiv = ::GetDeviceCaps(hDCFrom, LOGPIXELSY);	// pixels in screen dc
		::ReleaseDC(NULL, hDCFrom);

		m_PrintParams.m_nLargeRowHeight = m_PrintParams.m_nRowHeight + ::MulDiv (14 + 2, nYMul, nYDiv);

		TEXTMETRIC tm;
		m_pPrintDC->GetTextMetrics (&tm);
		m_nPrintPreviewRowHeight = m_bPreviewRow ? m_nPreviewRowMaxLines * tm.tmHeight + ::MulDiv (2, nYMul, nYDiv) : 0;
	}
	else
	{
		m_nLargeRowHeight = m_nRowHeight + 14 + 2;

		CClientDC dc (this);
		HFONT hfontOld = SetCurrFont (&dc);

		TEXTMETRIC tm;
		dc.GetTextMetrics (&tm);
		m_nPreviewRowHeight = m_bPreviewRow ? m_nPreviewRowMaxLines * tm.tmHeight + 2 : 0;

		::SelectObject (dc.GetSafeHdc (), hfontOld);
	}
}
//******************************************************************************************
void CBCGPReportCtrl::EnablePreviewRow (BOOL bPreviewRow, BOOL bRedraw)
{
	ASSERT (IsRowExtraHeightAllowed () != NULL);

	m_bPreviewRow = bPreviewRow;

	if (bRedraw)
	{
		BOOL bRebuildTerminalItemsOld = m_bRebuildTerminalItems;
		m_bRebuildTerminalItems = FALSE;

		SetRecalcPreview ();
		AdjustLayout ();
		AdjustLayout ();

		m_bRebuildTerminalItems = bRebuildTerminalItemsOld;
	}
}
//******************************************************************************************
BOOL CBCGPReportCtrl::IsPreviewRowEnabled () const
{
	return m_bPreviewRow;
}
//******************************************************************************************
void CBCGPReportCtrl::SetPreviewRowAutoLeftMargin (BOOL bAuto)
{
	m_bPreviewRowAutoLeftMargin = bAuto;

	RecalcMargins ();
}
//******************************************************************************************
void CBCGPReportCtrl::RecalcMargins ()
{
	if (m_bPreviewRowAutoLeftMargin)
	{
		m_nPreviewRowLeftMargin = m_Columns.GetLeftTextOffset () + TEXT_MARGIN;
	}
}
//******************************************************************************************
CRect CBCGPReportCtrl::OnGetPreviewRowMargins (CBCGPGridRow* pRow) const
{
	ASSERT_VALID (pRow);

	int nDefaultMargin = GetRowHeight () / 2;

	CRect rect (
		(m_nPreviewRowLeftMargin >= 0) ? m_nPreviewRowLeftMargin : nDefaultMargin,
		(pRow->IsGroup () ? GetLargeRowHeight () : GetRowHeight ()),
		(m_nPreviewRowRightMargin >= 0) ? m_nPreviewRowRightMargin : nDefaultMargin,
		2);

	return rect;
}
//******************************************************************************************
int CBCGPReportCtrl::GetHierarchyOffset () const 
{
	int nLevel = m_Columns.GetGroupColumnCount ();
	if (nLevel > 0)
	{
		nLevel--;
	}

	return nLevel * GetHierarchyLevelOffset ();
}
//******************************************************************************************
void CBCGPReportCtrl::OnResizeColumns ()
{
	CBCGPGridCtrl::OnResizeColumns ();

	RecalcMargins ();
}
//******************************************************************************************
void CBCGPReportCtrl::SetRecalcPreview ()
{
	ASSERT_VALID (this);

	const CList<CBCGPGridRow*, CBCGPGridRow*>& lst =
		(!IsSortingMode () && !IsGrouping ()) ? m_lstItems : m_lstTerminalItems;

	for (POSITION pos = lst.GetHeadPosition (); pos != NULL;)
	{
		CBCGPReportRow* pItem = DYNAMIC_DOWNCAST (CBCGPReportRow, lst.GetNext (pos));
		if (pItem == NULL)
		{
			continue;
		}

		ASSERT_VALID (pItem);

		pItem->m_nPreviewHeight = -1;
	}
}
//******************************************************************************************
void CBCGPReportCtrl::OnPaint() 
{
#ifndef _BCGPGRID_STANDALONE
	m_ColorData.m_GroupColors.m_clrBackground = 
		visualManager->GetReportCtrlGroupBackgoundColor ();
#else
	m_ColorData.m_GroupColors.m_clrBackground = globalData.clrBtnLight;
#endif

	CBCGPGridCtrl::OnPaint();
}

#endif // BCGP_EXCLUDE_GRID_CTRL
