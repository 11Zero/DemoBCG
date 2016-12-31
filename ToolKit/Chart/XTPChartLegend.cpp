// XTPChartLegend.cpp
//
// This file is a part of the XTREME TOOLKIT PRO MFC class library.
// (c)1998-2011 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME TOOLKIT PRO LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Common/XTPPropExchange.h"

#include "XTPChartLegend.h"
#include "XTPChartElementView.h"
#include "XTPChartContent.h"
#include "XTPChartSeries.h"
#include "XTPChartSeriesStyle.h"

#include "Drawing/XTPChartDeviceCommand.h"
#include "Drawing/XTPChartDeviceContext.h"
#include "Drawing/XTPChartRectangleDeviceCommand.h"
#include "Drawing/XTPChartTextDeviceCommand.h"
#include "Appearance/XTPChartBorder.h"
#include "Appearance/XTPChartFillStyle.h"
#include "Appearance/XTPChartAppearance.h"
#include "Utils/XTPChartTextPainter.h"

IMPLEMENT_DYNAMIC(CXTPChartLegend, CXTPChartTextElement)

CXTPChartLegend::CXTPChartLegend(CXTPChartContent* pOwner)
{
	m_pOwner = (CXTPChartElement*)pOwner;

	m_pFont =  CXTPChartFont::GetTahoma8();

	m_bVisible = FALSE;

	m_bAntialiasing = FALSE;

	m_pBorder = new CXTPChartBorder(this);

	m_pFillStyle = new CXTPChartFillStyle(this);
	m_pFillStyle->SetFillMode(xtpChartFillSolid);

	m_szMarkerSize = CSize(20, 16);
	m_bMarkerVisible = TRUE;

	m_nDirection = xtpChartLegendTopToBottom;
	m_nHorizontalAlignment = xtpChartLegendFarOutside;
	m_nVerticalAlignment = xtpChartLegendNear;

	m_nColumnCount = 0;


}

CXTPChartLegend::~CXTPChartLegend()
{
	SAFE_RELEASE(m_pFont);
	SAFE_RELEASE(m_pBorder);
	SAFE_RELEASE(m_pFillStyle);
}

void CXTPChartLegend::SetFont(CXTPChartFont* pFont)
{
	SAFE_RELEASE(m_pFont);
	m_pFont = pFont;
	OnChartChanged();
}

void CXTPChartLegend::SetTextColor(const CXTPChartColor& clrTextColor)
{
	m_clrTextColor = clrTextColor;
	OnChartChanged();
}

void CXTPChartLegend::SetBackgroundColor(const CXTPChartColor& clrBackgroundColor)
{
	m_clrBackgroundColor = clrBackgroundColor;
	OnChartChanged();
}

CXTPChartColor CXTPChartLegend::GetTextColor() const
{
	return m_clrTextColor;
}

CXTPChartColor CXTPChartLegend::GetActualTextColor() const
{
	if (!m_clrTextColor.IsEmpty())
		return m_clrTextColor;

	return GetContent()->GetAppearance()->GetContentAppearance()->GetLegendAppearance()->TextColor;
}

CXTPChartColor CXTPChartLegend::GetBackgroundColor() const
{
	return m_clrBackgroundColor;
}

CXTPChartColor CXTPChartLegend::GetActualBackgroundColor() const
{
	if (!m_clrBackgroundColor.IsEmpty())
		return m_clrBackgroundColor;

	return GetContent()->GetAppearance()->GetContentAppearance()->GetLegendAppearance()->BackgroundColor;
}

CXTPChartColor CXTPChartLegend::GetActualBorderColor() const
{
	if (!m_pBorder->GetColor().IsEmpty())
		return m_pBorder->GetColor();

	return GetContent()->GetAppearance()->GetContentAppearance()->GetLegendAppearance()->BorderColor;
}


CXTPChartLegendView* CXTPChartLegend::CreateView(CXTPChartDeviceContext* pDC, CXTPChartElementView* pParentView)
{
	UNREFERENCED_PARAMETER(pDC);

	if (!m_bVisible)
		return NULL;

	CXTPChartLegendView* pView = new CXTPChartLegendView(this, pParentView);


	return pView;
}

void CXTPChartLegend::DoPropExchange(CXTPPropExchange* pPX)
{
	PX_Bool(pPX, _T("Visible"), m_bVisible, FALSE);
	PX_Bool(pPX, _T("MarkerVisible"), m_bMarkerVisible, FALSE);
	PX_Size(pPX, _T("MarkerSize"), m_szMarkerSize, CSize(0, 0));
	PX_Bool(pPX, _T("Antialiasing"), m_bAntialiasing, FALSE);

	PX_Int(pPX, _T("ColumnCount"), m_nColumnCount, 0);

	PX_Enum(pPX, _T("HorizontalAlignment"), m_nHorizontalAlignment, xtpChartLegendFarOutside);
	PX_Enum(pPX, _T("VerticalAlignment"), m_nVerticalAlignment, xtpChartLegendNear);
	PX_Enum(pPX, _T("Direction"), m_nDirection, xtpChartLegendTopToBottom);

	PX_Color(pPX, _T("BackColor"), m_clrBackgroundColor);
	PX_Color(pPX, _T("TextColor"), m_clrTextColor);


	CXTPPropExchangeSection secFillStyle(pPX->GetSection(_T("FillStyle")));
	m_pFillStyle->DoPropExchange(&secFillStyle);

	CXTPPropExchangeSection secBorder(pPX->GetSection(_T("Border")));
	m_pBorder->DoPropExchange(&secBorder);
}


//////////////////////////////////////////////////////////////////////////
// CXTPChartLegendView

CXTPChartLegendView::CXTPChartLegendView(CXTPChartLegend* pLegend, CXTPChartElementView* pParentView)
	: CXTPChartElementView(pParentView)
{
	m_pLegend = pLegend;
	m_rcBounds.SetRectEmpty();
}

CXTPChartLegendView::~CXTPChartLegendView()
{

}


CXTPChartDeviceCommand* CXTPChartLegendView::CreateDeviceCommand(CXTPChartDeviceContext* pDC)
{
	int nCount = (int)m_arrItems.GetSize();
	if (nCount == 0)
		return 0;

	CXTPChartDeviceCommand* pDeviceCommand = new CXTPChartHitTestElementCommand(m_pLegend, m_rcBounds);

	int nBorderSize = m_pLegend->GetBorder()->IsVisible() ? m_pLegend->GetBorder()->GetThickness() : 0;
	nBorderSize += 2;

	pDeviceCommand->AddChildCommand(m_pLegend->GetFillStyle()->CreateDeviceCommand(m_rcBounds, m_pLegend->GetActualBackgroundColor(), m_pLegend->GetActualBackgroundColor()));

	if (m_pLegend->GetBorder()->IsVisible())
	{
		pDeviceCommand->AddChildCommand(m_pLegend->GetBorder()->CreateInnerBorderDeviceCommand(m_rcBounds, m_pLegend->GetActualBorderColor()));
	}


	BOOL bVertical = m_pLegend->GetDirection() == xtpChartLegendTopToBottom;
	int nColumnCount = m_pLegend->GetColumnCount();


	if (nColumnCount == 0)
		nColumnCount = bVertical ? 1 : nCount;

	if (nColumnCount > nCount)
		nColumnCount = nCount;

	int nRowCount = (nCount + nColumnCount - 1) / nColumnCount;

	int nLeft = m_rcBounds.left + nBorderSize;

	for (int nColumn = 0; nColumn < nColumnCount; nColumn++)
	{
		int nColumnWidth = 0;

		int nTop = m_rcBounds.top + nBorderSize;

		for (int nRow = 0; nRow < nRowCount; nRow++)
		{
			int i = bVertical ? nColumn * nRowCount + nRow : nRow * nColumnCount + nColumn;
			if (i >= nCount)
				continue;

			CXTPChartLegendItem* pItem = m_arrItems[i];

			CXTPChartString strName = pItem->GetLegendName();

			if (strName.GetLength() == 0)
				continue;

			CXTPChartTextPainter painter(pDC, strName, m_pLegend);

			CXTPChartSizeF szText = painter.GetSize();
			CXTPChartSizeF sz = szText;

			if (m_pLegend->IsMarkerVisible())
			{
				if (sz.Height < m_pLegend->GetMarkerSize().cy)
					sz.Height = (float)m_pLegend->GetMarkerSize().cy;
			}

			int nTextLeft = nLeft;

			if (m_pLegend->IsMarkerVisible())
			{
				CSize szMarker = m_pLegend->GetMarkerSize();

				CRect rcMarker(CPoint(nLeft, nTop  + (int)sz.Height / 2 - szMarker.cy / 2), szMarker);

				pDeviceCommand->AddChildCommand(pItem->CreateLegendDeviceCommand(pDC, rcMarker));

				nTextLeft += szMarker.cx + 2;
				sz.Width += szMarker.cx + 2;
			}


			painter.SetLocation(CXTPChartPointF((float)nTextLeft, (float)(nTop + sz.Height / 2 - szText.Height / 2)));
			pDeviceCommand->AddChildCommand(painter.CreateDeviceCommand(pDC, m_pLegend->GetActualTextColor()));

			nColumnWidth = max(nColumnWidth, (int)sz.Width + 2);

			nTop += (int)sz.Height + 2;
		}
		nLeft += nColumnWidth;
	}


	return pDeviceCommand;
}

CSize CXTPChartLegendView::CalcSize(CXTPChartDeviceContext* pDC)
{
	UNREFERENCED_PARAMETER(pDC);

	int nHeight = 0;
	int nWidth = 0;

	BOOL bVertical = m_pLegend->GetDirection() == xtpChartLegendTopToBottom;
	int nColumnCount = m_pLegend->GetColumnCount();
	int nBorderSize = m_pLegend->GetBorder()->IsVisible() ? m_pLegend->GetBorder()->GetThickness() : 0;
	nBorderSize += 2;

	int nCount = (int)m_arrItems.GetSize();
	if (nCount == 0)
		return CSize(0, 0);

	if (nColumnCount == 0)
		nColumnCount = bVertical ? 1 : nCount;

	if (nColumnCount > nCount)
		nColumnCount = nCount;

	int nRowCount = (nCount + nColumnCount - 1) / nColumnCount;

	for (int nColumn = 0; nColumn < nColumnCount; nColumn++)
	{
		int nColumnWidth = 0;
		int nColumnHeight = 0;

		for (int nRow = 0; nRow < nRowCount; nRow++)
		{
			int i = bVertical ? nColumn * nRowCount + nRow : nRow * nColumnCount + nColumn;
			if (i >= nCount)
				continue;

			CXTPChartLegendItem* pItem = m_arrItems[i];

			CXTPChartString strName = pItem->GetLegendName();

			if (strName.GetLength() == 0)
				continue;

			CXTPChartTextPainter painter(pDC, strName, m_pLegend);
			CXTPChartSizeF sz = painter.GetSize();

			if (m_pLegend->IsMarkerVisible())
			{
				if (sz.Height < m_pLegend->GetMarkerSize().cy)
					sz.Height = (float)m_pLegend->GetMarkerSize().cy;

				sz.Width += m_pLegend->GetMarkerSize().cx + 2;
			}

			nColumnWidth = max(nColumnWidth, (int)sz.Width + 2);

			if (nRow) nColumnHeight += 2;
			nColumnHeight += (int)sz.Height;
		}

		nWidth += nColumnWidth;
		nHeight = max(nHeight, nColumnHeight);
	}



	return CSize(nWidth + 2 * nBorderSize, nHeight + 2 * nBorderSize);
}


void CXTPChartLegendView::CalculateView(CXTPChartDeviceContext* pDC, CRect& rcChart)
{

	CSize sz = CalcSize(pDC);
	CPoint pt;

	int nHorizontalAlignment = m_pLegend->GetHorizontalAlignment();
	int nVerticalAlignment = m_pLegend->GetVerticalAlignment();

	BOOL bInside = nHorizontalAlignment != xtpChartLegendNearOutside && nHorizontalAlignment != xtpChartLegendFarOutside &&
		nVerticalAlignment != xtpChartLegendNearOutside && nVerticalAlignment != xtpChartLegendFarOutside;

	if (nHorizontalAlignment == xtpChartLegendNear || nHorizontalAlignment == xtpChartLegendNearOutside)
	{
		pt.x = rcChart.left + (bInside ? 10 : 0);

		if (nHorizontalAlignment == xtpChartLegendNearOutside)
			rcChart.left += sz.cx + 10;

	}
	else if (nHorizontalAlignment == xtpChartLegendCenter)
	{
		pt.x = (rcChart.left + rcChart.right - sz.cx) / 2;
	}
	else
	{
		pt.x = rcChart.right - sz.cx -  (bInside ? 10 : 0);

		if (nHorizontalAlignment == xtpChartLegendFarOutside)
			rcChart.right -= sz.cx + 10;
	}


	if (nVerticalAlignment == xtpChartLegendNear || nVerticalAlignment == xtpChartLegendNearOutside)
	{
		pt.y = rcChart.top + (bInside ? 10 : 0);

		if (nVerticalAlignment== xtpChartLegendNearOutside)
			rcChart.top += sz.cy + 10;

	}
	else if (nVerticalAlignment == xtpChartLegendCenter)
	{
		pt.y = (rcChart.top + rcChart.bottom - sz.cy) / 2;
	}
	else
	{
		pt.y = rcChart.bottom - sz.cy - (bInside ? 10 : 0);

		if (nVerticalAlignment == xtpChartLegendFarOutside)
			rcChart.bottom -= sz.cy + 10;
	}

	m_rcBounds = CRect(pt, sz);

}

void CXTPChartLegendView::AddItem(CXTPChartLegendItem* pItem)
{
	m_arrItems.Add(pItem);
}





