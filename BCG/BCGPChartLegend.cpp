//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
// BCGPChartLegend.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPChartLegend.h"
#include "BCGPChartSeries.h"
#include "BCGPChartVisualObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT BCGM_ON_CHART_LEGEND_AFTER_DRAW = ::RegisterWindowMessage (_T("BCGM_ON_CHART_LEGEND_AFTER_DRAW"));
UINT BCGM_ON_CHART_LEGEND_CONTENT_CREATED = ::RegisterWindowMessage (_T("BCGM_ON_CHART_LEGEND_CONTENT_CREATED"));
UINT BCGM_ON_CHART_LEGEND_MOUSE_DOWN = ::RegisterWindowMessage (_T("BCGM_ON_CHART_LEGEND_MOUSE_DOWN"));
UINT BCGM_ON_CHART_LEGEND_MOUSE_UP = ::RegisterWindowMessage (_T("BCGM_ON_CHART_LEGEND_MOUSE_UP"));

CBCGPSize CBCGPLegendScrollButton::m_szScrollButtonSize = CBCGPSize(16, 16);

IMPLEMENT_DYNCREATE(CBCGPChartLegendCell, CObject)
IMPLEMENT_DYNCREATE(CBCGPChartLegendEntry, CObject)
IMPLEMENT_DYNCREATE(CBCGPChartLegendVisualObject, CBCGPBaseVisualObject)

//****************************************************************************************
// Legend Scroll button
//****************************************************************************************
void CBCGPLegendScrollButton::OnDraw(CBCGPGraphicsManager* pGM, CBCGPChartLegendVisualObject* pLegend)
{
	if (pGM->GetPrintInfo() != NULL)
	{
		return;
	}

	if (!m_rectBounds.IsRectEmpty())
	{
		CBCGPBrush brFill = pLegend->m_legendStyle.m_titleFormat.m_brTextColor;
		CBCGPBrush brArrow = pLegend->m_legendStyle.m_brFill;

		if (brFill.GetColor().IsDark())
		{
			brFill.MakeLighter(0.4);
		}

		CBCGPBrush brBorder = brFill;
		brBorder.MakeDarker(0.15);

		pGM->FillRectangle(m_rectBounds, brFill);
		pGM->DrawRectangle(m_rectBounds, brBorder);

		CBCGPRect rectArrow = m_rectBounds;

		CBCGPPointsArray arPoints;
		
		if (pLegend->IsVerticalLayout())
		{
			if (m_bIsLeftTop)
			{
				rectArrow.DeflateRect(3, 5);
				
				arPoints.Add(CBCGPPoint(rectArrow.CenterPoint().x, rectArrow.top));
				arPoints.Add(CBCGPPoint(rectArrow.left, rectArrow.bottom));
				arPoints.Add(CBCGPPoint(rectArrow.right, rectArrow.bottom));				
			}
			else
			{
				rectArrow.DeflateRect(3, 5);
				
				arPoints.Add(CBCGPPoint(rectArrow.CenterPoint().x, rectArrow.bottom));
				arPoints.Add(CBCGPPoint(rectArrow.left, rectArrow.top));
				arPoints.Add(CBCGPPoint(rectArrow.right, rectArrow.top));
			}
		}
		else
		{
			if (m_bIsLeftTop)
			{
				rectArrow.DeflateRect(5, 3);
				
				arPoints.Add(CBCGPPoint(rectArrow.left, rectArrow.CenterPoint().y));
				arPoints.Add(CBCGPPoint(rectArrow.right, rectArrow.top));
				arPoints.Add(CBCGPPoint(rectArrow.right, rectArrow.bottom));				
			}
			else
			{
				rectArrow.DeflateRect(5, 3);
				
				arPoints.Add(CBCGPPoint(rectArrow.right, rectArrow.CenterPoint().y));
				arPoints.Add(CBCGPPoint(rectArrow.left, rectArrow.top));
				arPoints.Add(CBCGPPoint(rectArrow.left, rectArrow.bottom));
			}
		}

		pGM->FillGeometry(CBCGPPolygonGeometry(arPoints), brArrow);
	}
}
//****************************************************************************************
void CBCGPLegendScrollButton::AdjustPosition(CBCGPGraphicsManager* pGM, CBCGPChartLegendVisualObject* pLegend)
{
	CBCGPSize szLegendSize = pLegend->GetLegendSize(FALSE, pGM);
	const CBCGPRect& rectLegendBounds = pLegend->GetLegendBounds();
	const CBCGPSize& szTitleSize = pLegend->GetTitleSize();
	
	if (pLegend->IsVerticalLayout())
	{
		if (rectLegendBounds.Height() < szLegendSize.cy)
		{
			if (IsLeftTop())
			{
				double dblTopOffset = 0;

				if (szTitleSize.cy > 0 && pLegend->m_legendStyle.m_bDrawTitleGridLine || 
					szTitleSize.cy == 0 && pLegend->m_legendStyle.m_bDrawLegendBorders)
				{
					dblTopOffset = pLegend->m_legendStyle.m_outlineFormat.m_dblWidth / 2;
				}

				m_rectBounds.SetRect(CBCGPPoint(rectLegendBounds.right - m_szScrollButtonSize.cx, 
					rectLegendBounds.top + szTitleSize.cy + dblTopOffset),  m_szScrollButtonSize);
			}
			else
			{
				m_rectBounds.SetRect(CBCGPPoint(rectLegendBounds.right - m_szScrollButtonSize.cx, 
					rectLegendBounds.bottom - m_szScrollButtonSize.cy), 
				m_szScrollButtonSize);
			}
			
		}
		else
		{
			m_rectBounds.SetRectEmpty();
		}
	}
	else 
	{
		if (rectLegendBounds.Width() < szLegendSize.cx)
		{
			if (IsLeftTop())
			{
				m_rectBounds.SetRect(CBCGPPoint(rectLegendBounds.left, 
					rectLegendBounds.bottom - m_szScrollButtonSize.cy),  m_szScrollButtonSize);
			}
			else
			{
				m_rectBounds.SetRect(CBCGPPoint(rectLegendBounds.right - m_szScrollButtonSize.cx, 
					rectLegendBounds.bottom - m_szScrollButtonSize.cy), m_szScrollButtonSize);
			}
		}
		else
		{
			m_rectBounds.SetRectEmpty();
		}
	}
}

//****************************************************************************************
// Legend Cell
//****************************************************************************************
CBCGPChartLegendCell::CBCGPChartLegendCell()
{
	Init();
}
//****************************************************************************************
CBCGPChartLegendCell::CBCGPChartLegendCell(CBCGPChartVisualObject* pChart, CBCGPChartSeries* pSeries, 
					int	nDataPointIndex, CBCGPChartLegendCell::LegendCellType type)
{
	Init();
	m_pChart = pChart;
	m_pSeries = pSeries;
	m_nDataPointIndex = nDataPointIndex;
	m_cellType = type;
	UpdateCellParams();
}
//****************************************************************************************
CBCGPChartLegendCell::CBCGPChartLegendCell(const CString& strContent, CObject* pCustomData)
{
	Init();
	m_cellType = CBCGPChartLegendCell::LCT_CUSTOM;
	m_strLabel = strContent;
	m_pCustomData = pCustomData;
}
//****************************************************************************************
void CBCGPChartLegendCell::Init()
{
	m_pChart = NULL;
	m_pSeries = NULL;
	m_nDataPointIndex = -1;
	m_pParentEntry = NULL;
	m_cellType = CBCGPChartLegendCell::LCT_CUSTOM;
	m_pCustomData = NULL;

	m_cellParams.SetContentPadding(CBCGPSize(6, 3));
}
//****************************************************************************************
CBCGPChartLegendCell::~CBCGPChartLegendCell()
{

}
//****************************************************************************************
void CBCGPChartLegendCell::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& /*sizeScaleRatioOld*/)
{
	m_cellParams.SetScaleRatio(sizeScaleRatioNew);
}
//****************************************************************************************
CBCGPChartLegendVisualObject* CBCGPChartLegendCell::GetParentLegend() const
{
	if (m_pParentEntry != NULL)
	{
		return m_pParentEntry->GetParentLegend();
	}

	return NULL;
}
//****************************************************************************************
void CBCGPChartLegendCell::UpdateCellParams()
{
	if (m_pSeries == NULL || m_cellType == CBCGPChartLegendCell::LCT_CHART_NAME)
	{
		CBCGPChartVisualObject* pChart = m_pChart != NULL ? m_pChart : m_pParentEntry->GetParentLegend()->GetRelatedChart(0);

		if (pChart == NULL)
		{
			return;
		}

		if (m_cellType == CBCGPChartLegendCell::LCT_CHART_NAME)
		{
			m_strLabel = m_pChart->GetChartTitle();
		}

		if (m_cellParams.m_brTextColor.IsEmpty())
		{
			CBCGPBrush brTitleText = pChart->GetTitleLabelFormat().m_brTextColor;
			CBCGPBrush brTitleFill = pChart->GetTitleLabelFormat().m_brFillColor;

			m_cellParams.m_brTextColor = brTitleText.IsEmpty() ? pChart->GetColors().m_brTitleTextColor : brTitleText;
			m_cellParams.m_brContentFill = brTitleFill.IsEmpty() ? pChart->GetColors().m_brTitleFillColor : brTitleFill;
		}

		if (m_pSeries != NULL)
		{
			const BCGPChartFormatSeries& formatSeries = m_pSeries->GetSeriesFormat();
			m_cellParams.m_textFormat = formatSeries.m_legendLabelFormat.m_textFormat;
		}

		m_cellParams.DetachFromGM();
		return;
	}

	BCGPSeriesColorsPtr colors;
	const BCGPChartFormatSeries* pFormatSeries = m_pSeries->GetColors(colors, m_nDataPointIndex);
	if (pFormatSeries == NULL)
	{
		return;
	}

	m_cellParams.m_brFill = *colors.m_pBrElementFillColor;

	m_cellParams.m_lineStyle = pFormatSeries->m_seriesElementFormat.m_outlineFormat;
	m_cellParams.m_lineStyle.m_brLineColor = *colors.m_pBrElementLineColor;

	m_cellParams.m_markerLineStyle = pFormatSeries->m_markerFormat.m_outlineFormat;
	m_cellParams.m_brMarkerFill = *colors.m_pBrMarkerFillColor;
	m_cellParams.m_markerLineStyle.m_brLineColor = *colors.m_pBrMarkerLineColor;
	m_cellParams.m_markerOptions = pFormatSeries->m_markerFormat.m_options;

	m_cellParams.m_brTextColor = pFormatSeries->m_legendLabelFormat.m_brTextColor;

	if (m_cellParams.m_brTextColor.IsEmpty())
	{
		m_cellParams.m_brTextColor = m_pChart->GetColors().m_brLegendEntryTextColor;
	}

	if (m_cellParams.m_brContentFill.IsEmpty())
	{
		if (m_cellType == CBCGPChartLegendCell::LCT_LABEL)
		{
			m_cellParams.m_brContentFill = pFormatSeries->m_legendLabelFormat.m_brFillColor;
		}
	}

	m_cellParams.m_textFormat = pFormatSeries->m_legendLabelFormat.m_textFormat;

	if (m_cellType == CBCGPChartLegendCell::LCT_LABEL)
	{
		m_strLabel.Empty();
		if (m_nDataPointIndex != -1 && m_pSeries->m_bIncludeDataPointLabelsToLegend && 
			m_pSeries->CanIncludeDataPointToLegend(m_nDataPointIndex))
		{
			const CBCGPChartDataPoint* pDataPoint = m_pSeries->GetDataPointAt(m_nDataPointIndex);
			if (pDataPoint != NULL)
			{
				m_pSeries->OnGetDataPointLegendLabel(m_nDataPointIndex, m_strLabel);
			}
		}

		if (m_strLabel.IsEmpty())
		{
			m_strLabel = m_pSeries->m_strSeriesName;
		}
	}

	m_cellParams.DetachFromGM();
}
//****************************************************************************************
void CBCGPChartLegendCell::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rect)
{
	ASSERT_VALID(this);

	CBCGPRect rectCell = rect;

	OnFillCellBackground(pGM, rectCell);

	CBCGPSize szPadding = m_cellParams.GetContentPadding(TRUE);
	rectCell.DeflateRect(szPadding);

	CBCGPRect rectContent = rectCell;
	rectContent.top = rectCell.CenterPoint().y - m_szContentSize.cy / 2;
	rectContent.bottom = rectContent.top + m_szContentSize.cy;
	rectContent.left = rectCell.CenterPoint().x - m_szContentSize.cx / 2;
	rectContent.right = rectContent.left + m_szContentSize.cx;

	switch(m_cellType)
	{
	case CBCGPChartLegendCell::LCT_KEY:
		if (m_pChart != NULL)
		{
			m_pChart->OnDrawChartLegendKeyEx(pGM, rectContent, this);
		}
		break;

	case CBCGPChartLegendCell::LCT_LABEL:
	case CBCGPChartLegendCell::LCT_CHART_NAME:
		if (m_pChart != NULL)
		{
			m_pChart->OnDrawChartLegendLabelEx(pGM, m_strLabel, rectContent, this);
		}
		break;

	case CBCGPChartLegendCell::LCT_CUSTOM:
		OnDrawCustomCell(pGM, rectContent);
		break;
	}

	m_rectHit = rect;
}
//****************************************************************************************
void CBCGPChartLegendCell::OnFillCellBackground(CBCGPGraphicsManager* pGM, const CBCGPRect& rectCell)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (!m_cellParams.m_brContentFill.IsEmpty())
	{
		pGM->FillRectangle(rectCell, m_cellParams.m_brContentFill);
	}
}
//****************************************************************************************
CBCGPSize CBCGPChartLegendCell::CalcSize(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);

	CBCGPSize sz(0, 0);

	switch (m_cellType)
	{
	case CBCGPChartLegendCell::LCT_KEY:
		sz = m_pChart->OnCalcLegendKeySizeEx(m_cellParams, m_pSeries);
		break;

	case CBCGPChartLegendCell::LCT_LABEL:
	case CBCGPChartLegendCell::LCT_CHART_NAME:
		m_cellParams.m_textFormat.Destroy();
		sz = m_pChart->OnCalcLegendLabelSizeEx(pGM, m_strLabel, m_cellParams);
		break;

	case CBCGPChartLegendCell::LCT_CUSTOM:
		sz = OnCalcCustomCellSize(pGM);
		break;
	}

	m_szContentSize = sz;
	m_szCellSize = sz + m_cellParams.GetContentPadding(TRUE) * 2;
	return m_szCellSize;
}
//****************************************************************************************
CBCGPSize CBCGPChartLegendCell::OnCalcCustomCellSize(CBCGPGraphicsManager* pGM)
{
	m_cellParams.m_textFormat.Destroy();
	return pGM->GetTextSize(m_strLabel, m_cellParams.m_textFormat);
}
//****************************************************************************************
void CBCGPChartLegendCell::OnDrawCustomCell(CBCGPGraphicsManager* pGM, const CBCGPRect& rectContent)
{
	pGM->DrawText(m_strLabel, rectContent, m_cellParams.m_textFormat, m_cellParams.m_brTextColor);
}
//****************************************************************************************
void CBCGPChartLegendCell::SetCellParams(const BCGPChartCellParams& params, BOOL bRedraw)
{
	m_cellParams = params;
	CBCGPChartLegendVisualObject* pLegend = GetParentLegend();

	if (pLegend != NULL)
	{
		pLegend->InvalidateLegendSize(bRedraw);
	}
}
//****************************************************************************************
void CBCGPChartLegendCell::SetTextFormat(const CBCGPTextFormat& format, BOOL bRedraw)
{
	m_cellParams.m_textFormat = format;
	m_cellParams.m_textFormat.Destroy();

	CBCGPChartLegendVisualObject* pLegend = GetParentLegend();
	
	if (pLegend != NULL)
	{
		pLegend->InvalidateLegendSize(bRedraw);
	}
}
//****************************************************************************************
void CBCGPChartLegendCell::SetCellText(const CString& str, BOOL bRedraw)
{
	m_strLabel = str;

	CBCGPChartLegendVisualObject* pLegend = GetParentLegend();
	
	if (pLegend != NULL)
	{
		pLegend->InvalidateLegendSize(bRedraw);
	}
}
//****************************************************************************************
void CBCGPChartLegendCell::Redraw()
{
	CBCGPChartLegendVisualObject* pLegend = GetParentLegend();
	if (pLegend != NULL)
	{
		ASSERT_VALID(pLegend);
		pLegend->RedrawRect(m_rectHit);
	}
}

//****************************************************************************************
// LEGEND ENTRY
//****************************************************************************************
CBCGPChartLegendEntry::~CBCGPChartLegendEntry()
{
	while (!m_lstLegendCells.IsEmpty())
	{
		delete m_lstLegendCells.RemoveTail();
	}
}
//****************************************************************************************
void CBCGPChartLegendEntry::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld)
{
	for (POSITION pos = m_lstLegendCells.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartLegendCell* pCell = DYNAMIC_DOWNCAST(CBCGPChartLegendCell, m_lstLegendCells.GetNext(pos));
		ASSERT_VALID(pCell);

		if (pCell != NULL)
		{
			pCell->OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);
		}
	}
}
//****************************************************************************************
void CBCGPChartLegendEntry::AddLegendCell(CBCGPChartLegendCell* pCell)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pCell);
	m_lstLegendCells.AddTail(pCell);
	pCell->m_pParentEntry = this;
	// it's necessary to update cell params at this stage, because they are not updated
	// for custom cells after creation
	if (pCell->GetCellType() == CBCGPChartLegendCell::LCT_CUSTOM)
	{
		pCell->UpdateCellParams();
	}
}
//****************************************************************************************
BOOL CBCGPChartLegendEntry::ContainsCell(CBCGPChartSeries* pSeries, int nDataPointIndex) const
{
	ASSERT_VALID(this);

	for (POSITION pos = m_lstLegendCells.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartLegendCell* pCell = DYNAMIC_DOWNCAST(CBCGPChartLegendCell, m_lstLegendCells.GetNext(pos));
		ASSERT_VALID(pCell);

		if (pCell->ContainsSeries(pSeries, nDataPointIndex))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//****************************************************************************************
CBCGPSize CBCGPChartLegendEntry::CalcSize(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);
	CBCGPSize sz(0, 0);

	ASSERT_VALID(pGM);

	if (pGM == NULL)
	{
		return sz;
	}

	for (POSITION pos = m_lstLegendCells.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartLegendCell* pCell = DYNAMIC_DOWNCAST(CBCGPChartLegendCell, m_lstLegendCells.GetNext(pos));
		ASSERT_VALID(pCell);

		CBCGPSize sizeCell = pCell->CalcSize(pGM);
		sz.cy = max(sz.cy, sizeCell.cy);
		sz.cx += sizeCell.cx;
	}

	m_szEntrySize = sz;
	return sz;
}
//****************************************************************************************
void CBCGPChartLegendEntry::AdjustLayout()
{
	ASSERT_VALID(this);

	POSITION pos = NULL;

	int nColumn = 0;
	m_szEntrySize.cx = 0;
	for (pos = m_lstLegendCells.GetHeadPosition(); pos != NULL; nColumn++)
	{
		CBCGPChartLegendCell* pCell = DYNAMIC_DOWNCAST(CBCGPChartLegendCell, m_lstLegendCells.GetNext(pos));
		ASSERT_VALID(pCell);

 		CBCGPSize szCell = pCell->GetSize();
 		szCell.cy = GetSize().cy;
		if (m_pParentLegend->IsVerticalLayout())
		{
			szCell.cx = m_pParentLegend->GetColumnWidth(nColumn);
			if (pos == NULL && nColumn < m_pParentLegend->GetColumnCount() - 1)
			{
				for (int j = nColumn + 1; j < m_pParentLegend->GetColumnCount(); j++)
				{
					szCell.cx += m_pParentLegend->GetColumnWidth(j);
				}
			}
		}

		pCell->SetSize(szCell);
		m_szEntrySize.cx += szCell.cx;
	}
}
//****************************************************************************************
BOOL CBCGPChartLegendEntry::IsCustom() const
{
	for (POSITION pos = m_lstLegendCells.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartLegendCell* pCell = DYNAMIC_DOWNCAST(CBCGPChartLegendCell, m_lstLegendCells.GetNext(pos));
		ASSERT_VALID(pCell);

		if (pCell->GetCellType() == CBCGPChartLegendCell::LCT_CUSTOM)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//****************************************************************************************
CBCGPChartLegendCell* CBCGPChartLegendEntry::GetCell(int nIndex) const
{
	if (nIndex < 0 || nIndex >= m_lstLegendCells.GetCount())
	{
		return NULL;
	}

	POSITION pos = m_lstLegendCells.FindIndex(nIndex);
	return DYNAMIC_DOWNCAST(CBCGPChartLegendCell, m_lstLegendCells.GetAt(pos));
}
//****************************************************************************************
double CBCGPChartLegendEntry::GetCellWidth(int nCellIndex) const
{
	if (nCellIndex < 0 || nCellIndex >= m_lstLegendCells.GetCount())
	{
		return 0;
	}

	POSITION pos = m_lstLegendCells.FindIndex(nCellIndex);
	CBCGPChartLegendCell* pCell = DYNAMIC_DOWNCAST(CBCGPChartLegendCell, m_lstLegendCells.GetAt(pos));

	return pCell->GetSize().cx;
}
//****************************************************************************************
void CBCGPChartLegendEntry::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(m_pParentLegend);

	if (pGM == NULL || m_pParentLegend == NULL)
	{
		return;
	}

	BOOL bIsVertical = m_pParentLegend->IsVerticalLayout();

	int nColumnIndex = 0;
	CBCGPRect rectCell = rect;

	// draw cell content
	for (POSITION pos = m_lstLegendCells.GetHeadPosition(); pos != NULL; nColumnIndex++)
	{
		CBCGPChartLegendCell* pCell = DYNAMIC_DOWNCAST(CBCGPChartLegendCell, m_lstLegendCells.GetNext(pos));
		ASSERT_VALID(pCell);

		const CBCGPSize& szCell = pCell->GetSize();
		rectCell.SetSize(szCell);

		pCell->OnDraw(pGM, rectCell);

		bIsVertical ? rectCell.OffsetRect(m_pParentLegend->GetColumnWidth(nColumnIndex), 0) : 
						rectCell.OffsetRect(szCell.cx, 0);
	}
}
//****************************************************************************************
void CBCGPChartLegendEntry::OnDrawGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& rect)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(m_pParentLegend);
	
	if (pGM == NULL || m_pParentLegend == NULL)
	{
		return;
	}

	BOOL bIsVertical = m_pParentLegend->IsVerticalLayout();
	int nColumnIndex = 0;
	CBCGPRect rectCell = rect;

	// draw cell borders
	if (bIsVertical && m_pParentLegend->m_legendStyle.m_bDrawVerticalGridLines)
	{
		for (POSITION pos = m_lstLegendCells.GetHeadPosition(); pos != NULL; nColumnIndex++)
		{
			CBCGPChartLegendCell* pCell = DYNAMIC_DOWNCAST(CBCGPChartLegendCell, m_lstLegendCells.GetNext(pos));
			ASSERT_VALID(pCell);
			
			const CBCGPSize& szCell = pCell->GetSize();
			rectCell.SetSize(szCell);
			
			if (pos != NULL)
			{
				pGM->DrawLine(rectCell.right, rectCell.top, rectCell.right, rectCell.bottom, 
					m_pParentLegend->m_legendStyle.m_outlineFormat.m_brLineColor, 
					m_pParentLegend->m_legendStyle.m_outlineFormat.GetLineWidth(TRUE), 
					&m_pParentLegend->m_legendStyle.m_outlineFormat.m_strokeStyle);
			}
			
			bIsVertical ? rectCell.OffsetRect(m_pParentLegend->GetColumnWidth(nColumnIndex), 0) : 
			rectCell.OffsetRect(szCell.cx, 0);
		}
	}
	
	if (bIsVertical && m_pParentLegend->m_legendStyle.m_bDrawHorizontalGridLines)
	{
		pGM->DrawLine(rect.left, rect.bottom, rect.right, rect.bottom, 
			m_pParentLegend->m_legendStyle.m_outlineFormat.m_brLineColor, 
			m_pParentLegend->m_legendStyle.m_outlineFormat.GetLineWidth(TRUE), 
			&m_pParentLegend->m_legendStyle.m_outlineFormat.m_strokeStyle);
		
	}
	else if (!bIsVertical && m_pParentLegend->m_legendStyle.m_bDrawVerticalGridLines)
	{
		pGM->DrawLine(rect.right, rect.top, rect.right, rect.bottom, 
			m_pParentLegend->m_legendStyle.m_outlineFormat.m_brLineColor, 
			m_pParentLegend->m_legendStyle.m_outlineFormat.GetLineWidth(TRUE), 
			&m_pParentLegend->m_legendStyle.m_outlineFormat.m_strokeStyle);
	}
}
//****************************************************************************************
CBCGPChartLegendCell* CBCGPChartLegendEntry::GetLegendCellFromPoint(const CBCGPPoint& pt) const
{
	for (POSITION pos = m_lstLegendCells.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartLegendCell* pCell = DYNAMIC_DOWNCAST(CBCGPChartLegendCell, m_lstLegendCells.GetNext(pos));

		if (pCell != NULL)
		{
			ASSERT_VALID(pCell);
			if (pCell->GetHitRect().PtInRect(pt))
			{
				return pCell;
			}
		}
	}

	return NULL;
}
//****************************************************************************************
// CHART LEGEND
//****************************************************************************************
CBCGPChartLegendVisualObject::CBCGPChartLegendVisualObject(CBCGPVisualContainer* pContainer) : 
				CBCGPBaseVisualObject(pContainer), m_btnLeftTop(TRUE), m_btnRightBottom(FALSE)
{
	m_nLegendID = 0;
	m_szLegendSize.SetSizeEmpty();
	m_pLegendCellRTC = NULL;
	m_bIsVertical = TRUE;
	m_dblMaxRowHeight = 0;
	
	m_dblScrollOffset = 0;
	m_dblScrollStep = 0;

	m_horzAlignment = CBCGPChartLegendVisualObject::LA_CENTER;
	m_vertAlignment = CBCGPChartLegendVisualObject::LA_CENTER;

	m_bIsContentUpdated = FALSE;
	m_bIsSizeValid = FALSE;

	m_bInsideOnDraw = FALSE;
}
//****************************************************************************************
CBCGPChartLegendVisualObject::~CBCGPChartLegendVisualObject()
{
	for (POSITION pos = m_lstRelatedCharts.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartVisualObject* pChart = DYNAMIC_DOWNCAST(CBCGPChartVisualObject, m_lstRelatedCharts.GetNext(pos));

		if (pChart != NULL)
		{
			pChart->RemoveRelatedLegend(this, FALSE);
		}
	}

	m_lstRelatedCharts.RemoveAll();
	RemoveAllEntries();
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld)
{
	if (m_legendStyle.GetScaleRatio() != m_sizeScaleRatio)
	{
		m_legendStyle.OnScaleRatioChanged(m_sizeScaleRatio, sizeScaleRatioOld);
	}

	for (POSITION pos = m_lstLegendEntries.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartLegendEntry* pEntry = DYNAMIC_DOWNCAST(CBCGPChartLegendEntry, m_lstLegendEntries.GetNext(pos));

		if (pEntry != NULL)
		{
			ASSERT_VALID(pEntry);
			pEntry->OnScaleRatioChanged(m_sizeScaleRatio, sizeScaleRatioOld);
		}
	}

	InvalidateLegendSize(FALSE);
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::SetLegendTitleFormat(const BCGPChartFormatLabel& titleFormat, BOOL bRedraw)
{
	m_legendStyle.m_titleFormat = titleFormat;
	InvalidateLegendSize(bRedraw);
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::ShowLegendTitle(BOOL bShow, const CString& strTitle, BOOL bRedraw)
{
	m_legendStyle.m_bShowTitle = bShow;

	if (bShow)
	{
		m_strLegendTitle = strTitle;
	}

	InvalidateLegendSize(bRedraw);
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::SetAdjustLegendSizeByTitleSize(BOOL bSet, BOOL bRedraw)
{
	m_legendStyle.m_bAdjustLegendSizeByTitleSize = bSet;
	InvalidateLegendSize(bRedraw);
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::ShowChartNameInLegend(BOOL bShow, BOOL bRedraw)
{
	m_legendStyle.m_bShowChartName = bShow;
	InvalidateLegendContent(bRedraw);
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::SetVerticalLayout(BOOL bIsVertical, BOOL bRedraw)
{
	m_bIsVertical = bIsVertical;
	InvalidateLegendSize(bRedraw);
}
//****************************************************************************************
BOOL CBCGPChartLegendVisualObject::AddRelatedChart(CBCGPChartVisualObject* pChart, BOOL bRedraw)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pChart);

	if (pChart == NULL)
	{
		return FALSE;
	}

	if (m_lstRelatedCharts.Find(pChart) != NULL)
	{
		return FALSE;
	}

	m_lstRelatedCharts.AddTail(pChart);
	pChart->AddRelatedLegend(this, bRedraw);

	InvalidateLegendContent(bRedraw);
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartLegendVisualObject::RemoveRelatedChart(CBCGPChartVisualObject* pChart, BOOL bRedraw)
{
	POSITION pos = m_lstRelatedCharts.Find(pChart);

	if (pos == NULL)
	{
		return FALSE;
	}

	m_lstRelatedCharts.RemoveAt(pos);
	pChart->RemoveRelatedLegend(this, FALSE);

	InvalidateLegendContent(bRedraw);
	return TRUE;
}
//****************************************************************************************
CBCGPChartVisualObject* CBCGPChartLegendVisualObject::GetRelatedChart(int nIndex) const
{
	POSITION pos = m_lstRelatedCharts.FindIndex(nIndex);

	if (pos == NULL)
	{
		return NULL;
	}

	return DYNAMIC_DOWNCAST(CBCGPChartVisualObject, m_lstRelatedCharts.GetAt(pos));
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::UpdateLegendContent()
{
	while (!m_lstLegendEntries.IsEmpty())
	{
		RemoveAllEntries();
	}

	for (POSITION pos = m_lstRelatedCharts.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartVisualObject* pChart = DYNAMIC_DOWNCAST(CBCGPChartVisualObject, m_lstRelatedCharts.GetNext(pos));

		if (pChart == NULL)
		{
			continue;
		}

		for (int i = 0; i < pChart->GetSeriesCount(TRUE); i++)
		{
			CBCGPChartSeries* pSeries = pChart->GetSeries(i);

			if (pSeries == NULL || !pSeries->IsLegendEntryVisible())
			{
				continue;
			}

			int nElementCount = pSeries->GetLegendElementCount();

			if (pSeries->m_bIncludeDataPointLabelsToLegend)
			{
				for (int j = 0; j < nElementCount; j++)
				{
					if (pSeries->CanIncludeDataPointToLegend(j))
					{
						AddLegendEntry(pChart, pSeries, j);
					}
				}
			}
			else
			{
				AddLegendEntry(pChart, pSeries, -1);
			}
		}
	}

	OnLegendContentCreated();
	OnScaleRatioChanged(GetScaleRatio());
	
	m_bIsContentUpdated = TRUE;
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::SetRect(const CBCGPRect& rect, BOOL bRedraw)
{
	InvalidateLegendSize(FALSE);
	CBCGPBaseVisualObject::SetRect(rect, bRedraw);
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::InvalidateLegendContent(BOOL bRedraw)
{
	m_bIsContentUpdated = FALSE;
	m_bIsSizeValid = FALSE;
	SetDirty(TRUE, bRedraw && !m_bInsideOnDraw);
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::InvalidateLegendSize(BOOL bRedraw)
{
	m_bIsSizeValid = FALSE;
	SetDirty(TRUE, bRedraw && !m_bInsideOnDraw);
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::OnLegendContentCreated()
{
	if (m_pWndOwner != NULL && m_pWndOwner->GetOwner()->GetSafeHwnd() != NULL)
	{
		m_pWndOwner->GetOwner()->SendMessage(BCGM_ON_CHART_LEGEND_CONTENT_CREATED, 0, (LPARAM)(LPVOID)this);
	}
}
//****************************************************************************************
CBCGPSize CBCGPChartLegendVisualObject::GetLegendSize(BOOL bWithPadding, CBCGPGraphicsManager* pGM)
{
	if (GetRelatedChartCount() == 0)
	{
		m_szLegendSize.SetSize(0, 0);
		return m_szLegendSize;
	}

	if (!m_bIsContentUpdated || !m_bIsSizeValid)
	{
		m_szLegendSize = CalcSize(pGM);
	}

	if (bWithPadding)
	{
		return m_szLegendSize + m_legendStyle.GetContentPadding(TRUE) * 2;
	}

	return m_szLegendSize;
}
//****************************************************************************************
CBCGPSize CBCGPChartLegendVisualObject::CalcSize(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);

	m_bIsSizeValid = TRUE;

	CBCGPSize sz(0, 0);

	BOOL bIsLocalGM = FALSE;
	CDC dcMem;

	if (pGM == NULL)
	{
		pGM = CBCGPGraphicsManager::CreateInstance(CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_D2D);

		if (pGM == NULL)
		{
			return sz;
		}

		dcMem.CreateCompatibleDC(NULL);
		pGM->BindDC(&dcMem, CBCGPRect());

		bIsLocalGM = TRUE;
	}

	ASSERT_VALID(pGM);
	
	if (!m_bIsContentUpdated)
	{
		UpdateLegendContent();
	}

	m_szTitleSize.SetSizeEmpty();
	m_arColumnWidths.RemoveAll();

	double dblMaxEntryHeight = 0;

	m_dblMaxRowHeight = 0;
	POSITION pos = NULL;

	for (pos = m_lstLegendEntries.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartLegendEntry* pEntry = DYNAMIC_DOWNCAST(CBCGPChartLegendEntry, m_lstLegendEntries.GetNext(pos));

		if (pEntry == NULL)
		{
			continue;
		}

		CBCGPSize szEntry = pEntry->CalcSize(pGM);

		dblMaxEntryHeight = max(szEntry.cy, dblMaxEntryHeight);
		
		if (IsVerticalLayout())
		{
			for (int i = 0; i < pEntry->GetCellCount(); i++)
			{
				m_arColumnWidths.SetAtGrow(i, max(GetColumnWidth(i), pEntry->GetCellWidth(i)));
			}
		}
	}

	m_dblMaxRowHeight = dblMaxEntryHeight; 
	AdjustLayout(pGM);

	for (pos = m_lstLegendEntries.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartLegendEntry* pEntry = DYNAMIC_DOWNCAST(CBCGPChartLegendEntry, m_lstLegendEntries.GetNext(pos));
		
		if (IsVerticalLayout())
		{
			sz.cy += pEntry->GetSize().cy;
		}
		else
		{
			sz.cx += pEntry->GetSize().cx;
		}
	}

	double dblTotalWidth = 0;
	for (int i = 0; i < m_arColumnWidths.GetSize(); i++)
	{
		dblTotalWidth += m_arColumnWidths[i];
	}

	if (IsVerticalLayout())
	{
		sz.cx = dblTotalWidth;
	}
	else
	{
		sz.cy = dblMaxEntryHeight;
	}

	if (m_legendStyle.m_bShowTitle)
	{
		m_szTitleSize = OnCalcTitleSize(pGM);
		sz.cx = max(sz.cx, m_szTitleSize.cx);
		sz.cy += m_szTitleSize.cy;
	}

	if (IsVerticalLayout() && dblTotalWidth < m_szTitleSize.cx && m_arColumnWidths.GetSize() > 0)
	{
		CBCGPSize szPadding = m_legendStyle.GetContentPadding(TRUE);
		CBCGPRect rectCtrl = m_rect;	
		rectCtrl.DeflateRect(szPadding);
		double dblTitleWidth = m_szTitleSize.cx;

		if (rectCtrl.Width() < dblTitleWidth && !m_legendStyle.m_bAdjustLegendSizeByTitleSize)
		{
			dblTitleWidth -= dblTitleWidth - rectCtrl.Width();
		}

		if (dblTitleWidth > dblTotalWidth)
		{
			double dblDiff = dblTitleWidth - dblTotalWidth;
			double dblDistribute = dblDiff / m_arColumnWidths.GetSize();
			dblTotalWidth = 0;

			for (int i = 0; i < m_arColumnWidths.GetSize(); i++)
			{
				m_arColumnWidths[i] += dblDistribute;
				dblTotalWidth += m_arColumnWidths[i];
			}

			sz.cx = dblTotalWidth;
		}
	}

	if (bIsLocalGM)
	{
		pGM->BindDC(NULL);
		pGM->CleanResources(TRUE);
		delete pGM;
	}

	return sz;
}
//****************************************************************************************
CBCGPSize CBCGPChartLegendVisualObject::OnCalcTitleSize(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (pGM == NULL || m_strLegendTitle.IsEmpty() || !m_legendStyle.m_bShowTitle)
	{
		return CBCGPSize();
	}

	m_legendStyle.m_titleFormat.DetachFromGM();

	return pGM->GetTextSize(m_strLegendTitle, m_legendStyle.m_titleFormat.m_textFormat) + 
		m_legendStyle.m_titleFormat.GetContentPadding(TRUE) * 2;
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::AdjustLegendBounds(CBCGPGraphicsManager* pGM)
{
	CBCGPSize szLegendSize = GetLegendSize(FALSE, pGM);

	if (OnAdjustLegendBounds(szLegendSize, m_rect, m_rectLegendBounds))
	{
		return;
	}

	CBCGPSize szPadding = m_legendStyle.GetContentPadding(TRUE);
	m_rectLegendBounds = m_rect;
	m_rectLegendBounds.DeflateRect(szPadding);
	
	if (szLegendSize.Width() < m_rectLegendBounds.Width())
	{
		switch(m_horzAlignment)
		{
		case CBCGPChartLegendVisualObject::LA_CENTER:
			m_rectLegendBounds.left = m_rectLegendBounds.CenterPoint().x - szLegendSize.cx / 2;
			m_rectLegendBounds.right = m_rectLegendBounds.left + szLegendSize.cx;
			break;

		case CBCGPChartLegendVisualObject::LA_NEAR:
			m_rectLegendBounds.right = m_rectLegendBounds.left + szLegendSize.cx;
			break;

		case CBCGPChartLegendVisualObject::LA_FAR:
			m_rectLegendBounds.left = m_rectLegendBounds.right - szLegendSize.cx;
			break;
		}
	}

	if (szLegendSize.Height() < m_rectLegendBounds.Height())
	{
		switch(m_vertAlignment)
		{
		case CBCGPChartLegendVisualObject::LA_CENTER:
			m_rectLegendBounds.top = m_rectLegendBounds.CenterPoint().y - szLegendSize.cy / 2;
			m_rectLegendBounds.bottom = m_rectLegendBounds.top + szLegendSize.cy;
			break;
			
		case CBCGPChartLegendVisualObject::LA_NEAR:
			m_rectLegendBounds.bottom = m_rectLegendBounds.top + szLegendSize.cy;
			break;
			
		case CBCGPChartLegendVisualObject::LA_FAR:
			m_rectLegendBounds.top = m_rectLegendBounds.bottom - szLegendSize.cy;
			break;
		}
	}
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::SetVerticalAlignment(LegendAlignment vertAlign, BOOL bRedraw)
{
	m_vertAlignment = vertAlign;
	SetDirty(TRUE, bRedraw && !m_bInsideOnDraw);
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::SetHorizontalAlignment(LegendAlignment horzAlign, BOOL bRedraw)
{
	m_horzAlignment = horzAlign;
	SetDirty(TRUE, bRedraw && !m_bInsideOnDraw);
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::AdjustLayout(CBCGPGraphicsManager* pGM)
{
	AdjustLegendBounds(pGM);
	
	CBCGPPoint ptOffset = m_rectLegendBounds.TopLeft();
	ptOffset.y += m_szTitleSize.cy;

	for (POSITION pos = m_lstLegendEntries.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartLegendEntry* pEntry = DYNAMIC_DOWNCAST(CBCGPChartLegendEntry, m_lstLegendEntries.GetNext(pos));

		if (m_legendStyle.m_bFixedRowHeight || !IsVerticalLayout())
		{
			pEntry->m_szEntrySize.cy = m_dblMaxRowHeight;
		}

		pEntry->SetOffset(ptOffset);

		if (IsVerticalLayout())
		{
			ptOffset.y += pEntry->GetSize().cy;
		}
		else
		{
			ptOffset.x += pEntry->GetSize().cx;
		}

		pEntry->AdjustLayout();
	}
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::AdjustScrollButtonPos(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);

	m_btnLeftTop.AdjustPosition(pGM, this);
	m_btnRightBottom.AdjustPosition(pGM, this);

	if (m_btnLeftTop.GetRect().IsRectEmpty() && m_btnRightBottom.GetRect().IsRectEmpty())
	{
		m_dblScrollOffset = 0;
	}
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::OnAfterLegendDraw(CBCGPGraphicsManager* pGM, DWORD dwFlags)
{
	ASSERT_VALID(this);

	if (m_pWndOwner != NULL && m_pWndOwner->GetOwner()->GetSafeHwnd() != NULL)
	{
		WPARAM wParam = MAKEWPARAM(GetID(), dwFlags);
		m_pWndOwner->GetOwner()->SendMessage(BCGM_ON_CHART_LEGEND_AFTER_DRAW, wParam, (LPARAM)(LPVOID)pGM);
	}
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectClip*/, DWORD dwFlags)
{
	ASSERT_VALID(this);

	if (m_bInsideOnDraw)
	{
		return;
	}

	m_bInsideOnDraw = TRUE;

	if ((dwFlags & BCGP_DRAW_STATIC) == 0)
	{
		OnAfterLegendDraw(pGM, dwFlags);
		m_bInsideOnDraw = FALSE;
		return;
	}

	if (m_lstRelatedCharts.IsEmpty())
	{
		pGM->FillRectangle(m_rect, CBCGPBrush(CBCGPColor::White));
		m_bInsideOnDraw = FALSE;
		return;
	}

	CBCGPChartVisualObject* pFirstChart = DYNAMIC_DOWNCAST(CBCGPChartVisualObject, m_lstRelatedCharts.GetHead());
	ASSERT_VALID(pFirstChart);

	if (pFirstChart == NULL)
	{
		m_bInsideOnDraw = FALSE;
		return;
	}

	CBCGPSize szLegend = GetLegendSize(FALSE, pGM);

	if (szLegend.IsEmpty())
	{
		m_bInsideOnDraw = FALSE;
		return;
	}

	if (IsDirty())
	{
		AdjustLayout(pGM);
		AdjustScrollButtonPos(pGM);
		AdjustScrollPos(pGM);
	}

	SetDirty(FALSE, FALSE);

	UpdateLegendColors(FALSE, pFirstChart);

	OnFillLegendControlBackground(pGM, m_rect);

	if (m_rectLegendBounds.Width() <= 0 || m_rectLegendBounds.Height() <= 0)
	{
		OnAfterLegendDraw(pGM, dwFlags);
		m_bInsideOnDraw = FALSE;
		return;
	}

	OnFillLegendBackground(pGM, m_rectLegendBounds);

	CBCGPRect rectTitle = m_rectLegendBounds;
	rectTitle.bottom = rectTitle.top + m_szTitleSize.cy;

	OnDrawLegendTitle(pGM, rectTitle);

	CBCGPRect rectBounds = m_rectLegendBounds;
	rectBounds.top += m_szTitleSize.cy;

	pGM->SetClipRect(rectBounds);
	OnDrawLegendEntries(pGM);
	OnDrawLegendGridLines(pGM);

	pGM->ReleaseClipArea();

	OnDrawTitleGridLine(pGM, rectTitle);
	OnDrawLegendBorders(pGM, m_rectLegendBounds);

	m_btnLeftTop.OnDraw(pGM, this);
	m_btnRightBottom.OnDraw(pGM, this);

	OnAfterLegendDraw(pGM, dwFlags);
	m_bInsideOnDraw = FALSE;
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::OnFillLegendControlBackground(CBCGPGraphicsManager* pGM, const CBCGPRect& rectBounds)
{
	if (!m_legendStyle.m_brParentControlFill.IsEmpty())
	{
		pGM->FillRectangle(rectBounds, m_legendStyle.m_brParentControlFill);
	}
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::OnFillLegendBackground(CBCGPGraphicsManager* pGM, const CBCGPRect& rectBounds)
{
	if (!m_legendStyle.m_brFill.IsEmpty())
	{
		pGM->FillRectangle(rectBounds, m_legendStyle.m_brFill);
	}
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::OnDrawLegendTitle(CBCGPGraphicsManager* pGM, const CBCGPRect& rectBounds)
{
	if (!m_legendStyle.m_bShowTitle)
	{
		return;
	}

	CBCGPRect rectTitle = rectBounds;

	if (!m_legendStyle.m_titleFormat.m_brFillColor.IsEmpty())
	{
		pGM->FillRectangle(rectTitle, m_legendStyle.m_titleFormat.m_brFillColor);
	}

	rectTitle.DeflateRect(m_legendStyle.m_titleFormat.GetContentPadding(TRUE));

	pGM->SetClipRect(rectTitle);
	pGM->DrawText(m_strLegendTitle, rectBounds, m_legendStyle.m_titleFormat.m_textFormat, 
		m_legendStyle.m_titleFormat.m_brTextColor);
	pGM->ReleaseClipArea();
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::OnDrawLegendBorders(CBCGPGraphicsManager* pGM, const CBCGPRect& rectBorders)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (m_legendStyle.m_bDrawLegendBorders)
	{
		pGM->DrawRectangle(rectBorders, m_legendStyle.m_outlineFormat.m_brLineColor, 
			m_legendStyle.m_outlineFormat.GetLineWidth(TRUE), &m_legendStyle.m_outlineFormat.m_strokeStyle);
	}
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::OnDrawLegendEntries(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	for (POSITION pos = m_lstLegendEntries.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartLegendEntry* pEntry = DYNAMIC_DOWNCAST(CBCGPChartLegendEntry, m_lstLegendEntries.GetNext(pos));
		CBCGPRect rectEntry(pEntry->GetOffset(), pEntry->GetSize());
		
		if (IsVerticalLayout())
		{
			rectEntry.OffsetRect(0, m_dblScrollOffset);
		}
		else
		{
			rectEntry.OffsetRect(m_dblScrollOffset, 0);
		}
		
		pEntry->OnDraw(pGM, rectEntry);
	}
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::OnDrawTitleGridLine(CBCGPGraphicsManager* pGM, const CBCGPRect& rectTitle)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (m_legendStyle.m_bDrawTitleGridLine && m_legendStyle.m_bShowTitle)
	{
		pGM->DrawLine(rectTitle.BottomLeft(), rectTitle.BottomRight(), 
			m_legendStyle.m_outlineFormat.m_brLineColor, 
			m_legendStyle.m_outlineFormat.GetLineWidth(TRUE), &m_legendStyle.m_outlineFormat.m_strokeStyle);
	}
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::OnDrawLegendGridLines(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	
	for (POSITION pos = m_lstLegendEntries.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartLegendEntry* pEntry = DYNAMIC_DOWNCAST(CBCGPChartLegendEntry, m_lstLegendEntries.GetNext(pos));
		CBCGPRect rectEntry(pEntry->GetOffset(), pEntry->GetSize());
		
		if (IsVerticalLayout())
		{
			rectEntry.OffsetRect(0, m_dblScrollOffset);
		}
		else
		{
			rectEntry.OffsetRect(m_dblScrollOffset, 0);
		}
		
		pEntry->OnDrawGridLines(pGM, rectEntry);
	}
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::UpdateLegendColors(BOOL bForceUpdate, CBCGPChartVisualObject* pChartOrg)
{
	ASSERT_VALID(this);

	if (pChartOrg == NULL)
	{
		pChartOrg = GetRelatedChart(0);
	}

	if (pChartOrg == NULL)
	{
		return;
	}

	ASSERT_VALID(pChartOrg);
	
	CBCGPElementColors colorsChart;
	BCGPChartFormatArea& chartFormat = pChartOrg->GetChartAreaFormat();
	pChartOrg->GetElementColors(chartFormat, CBCGPChartVisualObject::CE_CHART_AREA, colorsChart);
	
	if (m_legendStyle.m_brParentControlFill.IsEmpty() || bForceUpdate)
	{
		m_legendStyle.m_brParentControlFill = *colorsChart.pBrFill;
	}
	
	BCGPChartFormatArea& legendFormat = pChartOrg->GetLegendFormat();
	CBCGPElementColors colorsLegend;
	pChartOrg->GetElementColors(legendFormat, CBCGPChartVisualObject::CE_LEGEND, colorsLegend);
	
	if (m_legendStyle.m_brFill.IsEmpty() || bForceUpdate)
	{
		m_legendStyle.m_brFill = (colorsLegend.pBrFill == NULL || colorsLegend.pBrFill->IsEmpty()) ? 
			legendFormat.m_brFillColor : *colorsLegend.pBrFill;
	}
	
	if (m_legendStyle.m_outlineFormat.m_brLineColor.IsEmpty() || bForceUpdate)
	{
		m_legendStyle.m_outlineFormat.m_brLineColor = (colorsLegend.pBrLine == NULL || colorsLegend.pBrLine->IsEmpty()) ? 
			legendFormat.m_outlineFormat.m_brLineColor : *colorsLegend.pBrLine;
	}
	
	BCGPChartFormatLabel& chartTitleFormat = pChartOrg->GetTitleLabelFormat();
	if (m_legendStyle.m_titleFormat.m_brTextColor.IsEmpty() || bForceUpdate)
	{
		m_legendStyle.m_titleFormat.m_brTextColor = (chartTitleFormat.m_brTextColor.IsEmpty()) ? 
			pChartOrg->GetColors().m_brTitleTextColor : chartTitleFormat.m_brTextColor;
	}

	m_legendStyle.DetachFromGM();
}
//****************************************************************************************
double CBCGPChartLegendVisualObject::GetColumnWidth(int nColumnIndex) const
{
	ASSERT_VALID(this);

	if (nColumnIndex < 0 || nColumnIndex >= m_arColumnWidths.GetSize())
	{
		return 0.;
	}

	return m_arColumnWidths[nColumnIndex];
}
//****************************************************************************************
BOOL CBCGPChartLegendVisualObject::AddLegendEntry(CBCGPChartLegendEntry* pEntry)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pEntry);

	if (m_lstLegendEntries.Find(pEntry) != NULL)
	{
		return FALSE;
	}

	if (pEntry != NULL)
	{
		m_lstLegendEntries.AddTail(pEntry);
	}

	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartLegendVisualObject::InsertLegendEntry(CBCGPChartLegendEntry* pEntry, int nIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pEntry);

	if (m_lstLegendEntries.Find(pEntry) != NULL)
	{
		return FALSE;
	}

	if (pEntry != NULL)
	{
		POSITION pos = m_lstLegendEntries.FindIndex(nIndex);
		if (pos != NULL)
		{
			m_lstLegendEntries.InsertBefore(pos, pEntry);
		}
		else
		{
			m_lstLegendEntries.AddTail(pEntry);
		}
	}

	return TRUE;
}
//****************************************************************************************
CBCGPChartLegendEntry* CBCGPChartLegendVisualObject::AddLegendEntry(CBCGPChartVisualObject* pChart, 
									CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);

	CBCGPChartLegendCell* pChartName = IsChartNameInLegendVisible() ? 
		OnCreateLegendCell(pChart, pSeries, -1, CBCGPChartLegendCell::LCT_CHART_NAME) : NULL;
	CBCGPChartLegendCell* pCellKey = OnCreateLegendCell(pChart, pSeries, nDataPointIndex, CBCGPChartLegendCell::LCT_KEY);
	CBCGPChartLegendCell* pCellLabel = OnCreateLegendCell(pChart, pSeries, nDataPointIndex, CBCGPChartLegendCell::LCT_LABEL);

	ASSERT_VALID(pCellKey);
	ASSERT_VALID(pCellLabel);

	CBCGPChartLegendEntry* pEntry = OnCreateLegendEntry();
	ASSERT_VALID(pEntry);
	if (pChartName != NULL)
	{
		pEntry->AddLegendCell(pChartName);
	}
	pEntry->AddLegendCell(pCellKey);
	pEntry->AddLegendCell(pCellLabel);
	m_lstLegendEntries.AddTail(pEntry);

	return pEntry;
}
//****************************************************************************************
CBCGPChartLegendEntry* CBCGPChartLegendVisualObject::FindLegendEntry(CBCGPChartSeries* pSeries, int nDataPointIndex) const
{
	ASSERT_VALID(this);

	for (POSITION pos = m_lstLegendEntries.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartLegendEntry* pEntry = DYNAMIC_DOWNCAST(CBCGPChartLegendEntry, m_lstLegendEntries.GetNext(pos));
		ASSERT_VALID(pEntry);

		if (pEntry->ContainsCell(pSeries, nDataPointIndex))
		{
			return pEntry;
		}
	}

	return NULL;
}
//****************************************************************************************
CBCGPChartLegendEntry* CBCGPChartLegendVisualObject::GetLegendEntry(int nIndex) const
{
	POSITION pos = m_lstLegendEntries.FindIndex(nIndex);

	if (pos != NULL)
	{
		return DYNAMIC_DOWNCAST(CBCGPChartLegendEntry, m_lstLegendEntries.GetAt(pos));
	}

	return NULL;
}
//****************************************************************************************
CBCGPChartLegendCell* CBCGPChartLegendVisualObject::GetLegendCell(int nEntryIndex, int nCellIndex) const
{
	CBCGPChartLegendEntry* pEntry = GetLegendEntry(nEntryIndex);

	if (pEntry != NULL)
	{
		return pEntry->GetCell(nCellIndex);
	}

	return NULL;
}
//****************************************************************************************
BOOL CBCGPChartLegendVisualObject::RemoveLegendEntry(CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);

	for (POSITION pos = m_lstLegendEntries.GetHeadPosition(); pos != NULL;)
	{
		POSITION posCurr = pos;
		CBCGPChartLegendEntry* pEntry = DYNAMIC_DOWNCAST(CBCGPChartLegendEntry, m_lstLegendEntries.GetNext(pos));

		if (pEntry->ContainsCell(pSeries, nDataPointIndex))
		{
			m_lstLegendEntries.RemoveAt(posCurr);
			delete pEntry;
			return TRUE;
		}
	}

	return FALSE;
}
//****************************************************************************************
BOOL CBCGPChartLegendVisualObject::RemoveLegendEntry(CBCGPChartLegendEntry* pEntry)
{
	ASSERT_VALID(this);

	POSITION pos = m_lstLegendEntries.Find(pEntry);

	if (pos == NULL)
	{
		return FALSE;
	}

	m_lstLegendEntries.RemoveAt(pos);
	return TRUE;
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::RemoveAllEntries()
{
	ASSERT_VALID(this);

	while (!m_lstLegendEntries.IsEmpty())
	{
		delete m_lstLegendEntries.RemoveTail();
	}
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::SetLegendCellRTC(CRuntimeClass* pRTC)
{
	ASSERT_VALID(this);

	if (pRTC != NULL && !pRTC->IsDerivedFrom(RUNTIME_CLASS(CBCGPChartLegendCell)))
	{
		ASSERT(FALSE);
		TRACE0("CBCGPChartLegend::SetLegendCellRTC: Invalid runtine class specified for CBCGPChartLegendCell.");
		return;
	}

	m_pLegendCellRTC = pRTC;
}
//****************************************************************************************
CBCGPChartLegendCell* CBCGPChartLegendVisualObject::OnCreateLegendCell(CBCGPChartVisualObject* pChart, 
					CBCGPChartSeries* pSeries, int nDataPointIndex, CBCGPChartLegendCell::LegendCellType type)
{
	ASSERT_VALID(this);

	CBCGPChartLegendCell* pLegendCell = NULL;

	if (m_pLegendCellRTC != NULL)
	{
		pLegendCell = DYNAMIC_DOWNCAST(CBCGPChartLegendCell, m_pLegendCellRTC->CreateObject());

		ASSERT_VALID(pLegendCell);

		pLegendCell->m_pChart = pChart;
		pLegendCell->m_pSeries = pSeries;
		pLegendCell->m_nDataPointIndex = nDataPointIndex;
		pLegendCell->m_cellType = type;
	}
	else
	{
		pLegendCell = new CBCGPChartLegendCell(pChart, pSeries, nDataPointIndex, type);
	}

	return pLegendCell;
}
//****************************************************************************************
CBCGPChartLegendEntry* CBCGPChartLegendVisualObject::OnCreateLegendEntry()
{
	ASSERT_VALID(this);
	return new CBCGPChartLegendEntry(this);
}
//****************************************************************************************
CBCGPChartLegendCell* CBCGPChartLegendVisualObject::GetLegendCellFromPoint(const CBCGPPoint& pt) const
{
	for (POSITION pos = m_lstLegendEntries.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartLegendEntry* pEntry = DYNAMIC_DOWNCAST(CBCGPChartLegendEntry, m_lstLegendEntries.GetNext(pos));

		if (pEntry != NULL)
		{
			ASSERT_VALID(pEntry);
			CBCGPChartLegendCell* pCell = pEntry->GetLegendCellFromPoint(pt);

			if (pCell != NULL)
			{
				return pCell;
			}
		}
	}

	return NULL;
}
//****************************************************************************************
BOOL  CBCGPChartLegendVisualObject::HitTest(const CBCGPPoint& pt, BCGPChartLegendHitInfo* pHitInfo) const
{
	if (pHitInfo == NULL)
	{
		return FALSE;
	}

	pHitInfo->m_ptHit = pt;
	pHitInfo->m_pCell = NULL;
	pHitInfo->m_hitTest = BCGPChartLegendHitInfo::LHT_NONE;

	if (!m_rectLegendBounds.PtInRect(pt))
	{
		return TRUE;
	}

	CBCGPRect rectTitle = m_rectLegendBounds;
	rectTitle.bottom = rectTitle.top + m_szTitleSize.cy;

	if (rectTitle.PtInRect(pt))
	{
		pHitInfo->m_hitTest = BCGPChartLegendHitInfo::LHT_TITLE;
		return TRUE;
	}

	if (m_btnLeftTop.GetRect().PtInRect(pt))
	{
		pHitInfo->m_hitTest = BCGPChartLegendHitInfo::LHT_SCROLL_UP;
	}

	if (m_btnRightBottom.GetRect().PtInRect(pt))
	{
		pHitInfo->m_hitTest = BCGPChartLegendHitInfo::LHT_SCROLL_DOWN;
	}

	CBCGPChartLegendCell* pCell = GetLegendCellFromPoint(pt);
	pHitInfo->m_pCell = pCell;

	if (pCell != NULL)
	{
		pHitInfo->m_hitTest = (BCGPChartLegendHitInfo::LegendHitTest)(pHitInfo->m_hitTest | BCGPChartLegendHitInfo::LHT_CELL);
		return TRUE;
	}
	
	return TRUE;
}
//****************************************************************************************
BOOL CBCGPChartLegendVisualObject::FireMouseMessage(UINT nMsg, int nButton, const CBCGPPoint& pt, 
											  BOOL bUseLastHit)
{
	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}
	
	CWnd* pOwner = m_pWndOwner->GetOwner();
	
	if (pOwner->GetSafeHwnd() == NULL)
	{
		return FALSE;
	}
	
	if (!bUseLastHit)
	{
		HitTest(pt, &m_lastHit);
	}

	m_lastHit.m_nMouseButton = nButton;
	m_lastHit.m_ptHit = pt;
	
	if (pOwner->SendMessage(nMsg, (WPARAM)GetID(), (LPARAM) (LPVOID)&m_lastHit))
	{
		return TRUE;
	}
	
	return FALSE;

}
//****************************************************************************************
BOOL CBCGPChartLegendVisualObject::OnMouseDown(int nButton, const CBCGPPoint& pt)
{
	BOOL bRet = CBCGPBaseVisualObject::OnMouseDown(nButton, pt);

	if (FireMouseMessage(BCGM_ON_CHART_LEGEND_MOUSE_DOWN, nButton, pt))
	{
		return FALSE;
	}

	if (nButton != 0)
	{
		return bRet;
	}

	m_btnRightBottom.SetPressed(FALSE);  
	m_btnLeftTop.SetPressed(FALSE);

	if (m_btnRightBottom.GetRect().PtInRect(pt))
	{
		m_btnRightBottom.SetPressed(TRUE);
	}
	else if (m_btnLeftTop.GetRect().PtInRect(pt))
	{
		m_btnLeftTop.SetPressed(TRUE);
	}

	BOOL bWasScrolled = OnScroll(pt);

	if (m_btnRightBottom.GetRect().PtInRect(pt))
	{
		SetClickAndHoldEvent(m_btnRightBottom.GetRect());
		return TRUE;
	}
	else if (m_btnLeftTop.GetRect().PtInRect(pt))
	{
		SetClickAndHoldEvent(m_btnLeftTop.GetRect());
		return TRUE;
	}

	if (!bWasScrolled)
	{
		CBCGPChartLegendCell* pCell = GetLegendCellFromPoint(pt);
		if (pCell != NULL)
		{
			ASSERT_VALID(pCell);
			pCell->OnMouseDown(pt);
		}
	}

	return bRet;
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::OnClickAndHoldEvent(UINT /*nID*/, const CBCGPPoint& point)
{
	OnScroll(point);
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::OnMouseUp(int nButton, const CBCGPPoint& pt)
{
	CBCGPBaseVisualObject::OnMouseUp(nButton, pt);

	if (FireMouseMessage(BCGM_ON_CHART_LEGEND_MOUSE_UP, nButton, pt))
	{
		return;
	}

	if (nButton == 0)
	{
		CBCGPChartLegendCell* pCell = GetLegendCellFromPoint(pt);
		if (pCell != NULL)
		{
			ASSERT_VALID(pCell);
			pCell->OnMouseUp(pt);
		}

		BOOL bNeedRedraw = m_btnRightBottom.SetPressed(FALSE);  
		bNeedRedraw = bNeedRedraw || m_btnLeftTop.SetPressed(FALSE);

		if (bNeedRedraw)
		{
			Redraw();
		}
	}
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::OnCancelMode()
{
	CBCGPBaseVisualObject::OnCancelMode();

	BOOL bNeedRedraw = m_btnRightBottom.SetPressed(FALSE);  
	bNeedRedraw = bNeedRedraw || m_btnLeftTop.SetPressed(FALSE);

	if (bNeedRedraw)
	{
		Redraw();
	}
}
//****************************************************************************************
BOOL CBCGPChartLegendVisualObject::OnScroll(const CBCGPPoint& pt)
{
	double dblOldOffset = m_dblScrollOffset;

	if (m_btnRightBottom.GetRect().PtInRect(pt))
	{
		m_dblScrollOffset -= OnCalcScrollStep();
	}
	else if (m_btnLeftTop.GetRect().PtInRect(pt))
	{
		m_dblScrollOffset += OnCalcScrollStep();
	}

	if (m_dblScrollOffset != dblOldOffset)
	{
		SetDirty(TRUE, !m_bInsideOnDraw);
		return TRUE;
	}

	return FALSE;
}
//****************************************************************************************
double CBCGPChartLegendVisualObject::OnCalcScrollStep()
{
	if (m_dblScrollStep != 0)
	{
		return m_dblScrollStep;
	}

	return IsVerticalLayout() ? m_rectLegendBounds.Height() / 2 : m_rectLegendBounds.Width() / 2;
}
//****************************************************************************************
void CBCGPChartLegendVisualObject::AdjustScrollPos(CBCGPGraphicsManager* pGM)
{
	if (m_dblScrollOffset > 0)
	{
		m_dblScrollOffset = 0;
	}
	
	BOOL bFullScrollDown = FALSE;
	CBCGPSize szLegendSize = GetLegendSize(FALSE, pGM);

	if (m_dblScrollOffset < 0)
	{
		if (IsVerticalLayout() && 
			m_dblScrollOffset <= m_rectLegendBounds.Height() - szLegendSize.cy)
		{
			m_dblScrollOffset = m_rectLegendBounds.Height() - szLegendSize.cy;
			bFullScrollDown = TRUE;
		}
		else if (!IsVerticalLayout() && 
			m_dblScrollOffset <= m_rectLegendBounds.Width() - szLegendSize.cx)
		{
			m_dblScrollOffset = m_rectLegendBounds.Width() - szLegendSize.cx;
			bFullScrollDown = TRUE;
		}
	}

	if (bFullScrollDown)
	{
		m_btnRightBottom.SetRect(CBCGPRect());
	}
	
	if (m_dblScrollOffset == 0)
	{
		m_btnLeftTop.SetRect(CBCGPRect());
	}
}
