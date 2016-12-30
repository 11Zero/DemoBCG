//** *****************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a sample for BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPChartImpl.cpp : implementation of specific chart types
//

#include "stdafx.h"
#include "BCGPChartImpl.h"
#include "BCGPChartShape3D.h"
#include "BCGPChartSeries.h"
#include "BCGPChartFormula.h"
#include "BCGPChartVisualObject.h"
#include "BCGPGraphicsManagerGDI.h"
#include "BCGPGraphicsManagerD2D.h"

#include "BCGPEngine3D.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CBCGPBaseChartImpl, CObject)
IMPLEMENT_DYNCREATE(CBCGPLineChartImpl, CBCGPBaseChartImpl)
IMPLEMENT_DYNCREATE(CBCGPBarChartImpl, CBCGPBaseChartImpl)
IMPLEMENT_DYNCREATE(CBCGPHistogramChartImpl, CBCGPBarChartImpl)
IMPLEMENT_DYNCREATE(CBCGPAreaChartImpl, CBCGPBaseChartImpl)
IMPLEMENT_DYNCREATE(CBCGPBubbleChartImpl, CBCGPBaseChartImpl)
IMPLEMENT_DYNCREATE(CBCGPPieChartImpl, CBCGPBaseChartImpl)
IMPLEMENT_DYNCREATE(CBCGPPyramidChartImpl, CBCGPBaseChartImpl)
IMPLEMENT_DYNCREATE(CBCGPFunnelChartImpl, CBCGPPyramidChartImpl)
IMPLEMENT_DYNCREATE(CBCGPStockChartImpl, CBCGPLineChartImpl)
IMPLEMENT_DYNCREATE(CBCGPLongDataChartImpl, CBCGPBaseChartImpl)
IMPLEMENT_DYNCREATE(CBCGPPolarChartImpl, CBCGPLineChartImpl)
IMPLEMENT_DYNCREATE(CBCGPDoughnutChartImpl, CBCGPPieChartImpl)
IMPLEMENT_DYNCREATE(CBCGPTorusChartImpl, CBCGPPieChartImpl)

IMPLEMENT_DYNCREATE(CBCGPLineChart3DImpl, CBCGPLineChartImpl)
IMPLEMENT_DYNCREATE(CBCGPAreaChart3DImpl, CBCGPAreaChartImpl)
IMPLEMENT_DYNCREATE(CBCGPBarChart3DImpl, CBCGPBarChartImpl)
IMPLEMENT_DYNCREATE(CBCGPSurfaceChart3DImpl, CBCGPBaseChartImpl)

const int CBCGPPieChartImpl::PIE_SP_START = 0;
const int CBCGPPieChartImpl::PIE_SP_END = 1;
const int CBCGPPieChartImpl::PIE_SP_ANGLES = 2;
const int CBCGPPieChartImpl::PIE_SP_RADIUS = 3;
const int CBCGPPieChartImpl::PIE_SP_CENTER = 4;
const int CBCGPPieChartImpl::PIE_SP_HEIGHT = 5;
const int CBCGPPieChartImpl::PIE_SP_DIAGRAM_CENTER = 6; 

const int CBCGPBubbleChartImpl::BUBBLE_SP_SIZE = 1;

const int CBCGPPyramidChartImpl::PYRAMID_SP_HEIGHT = 0;
const int CBCGPPyramidChartImpl::PYRAMID_SP_OFFSETS = 1;
const int CBCGPPyramidChartImpl::PYRAMID_SP_LEFT_TOP = 2;
const int CBCGPPyramidChartImpl::PYRAMID_SP_RIGHT_BOTTOM = 3;

static const double nDefaultLegendKeyWidth = 28.0;
static const double nDefaultLegendKeyHeight = 9.0;

//****************************************************************************************
// Base chart implementation
//****************************************************************************************
CBCGPSize CBCGPBaseChartImpl::OnCalcLegendKeySize(CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	UNREFERENCED_PARAMETER(pSeries);
	UNREFERENCED_PARAMETER(nDataPointIndex);
	if (m_pRelatedChart == NULL)
	{
		return CBCGPSize(7, 8);
	}
	
	CBCGPSize szScale = m_pRelatedChart->GetScaleRatio();
	return CBCGPSize(szScale.cx * 7, szScale.cy * 8);
}
//****************************************************************************************
CBCGPSize CBCGPBaseChartImpl::OnCalcLegendKeySizeEx(const BCGPChartCellParams& params, CBCGPChartSeries* /*pSeries*/)
{
	ASSERT_VALID(this);

	CBCGPSize szScale = params.GetScaleRatio();
	return CBCGPSize(szScale.cx * 7, szScale.cy * 8);
}
//****************************************************************************************
void CBCGPBaseChartImpl::OnCalcScreenPositions(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (pSeries == NULL || m_pRelatedChart == NULL)
	{
		return;
	}

	ASSERT_VALID(pSeries);

	BOOL bIs3D = m_pRelatedChart->IsChart3D();

	pSeries->OnCalcScreenPoints(pGM, rectDiagramArea);

	for (int j = pSeries->GetMinDataPointIndex(); j <= pSeries->GetMaxDataPointIndex(); j++)
	{
		CBCGPChartDataPoint* pDataPoint = (CBCGPChartDataPoint*)pSeries->GetDataPointAt(j);
		const BCGPChartFormatSeries* pFormat = pSeries->GetDataPointFormat(j, FALSE);
		if (pDataPoint != NULL)
		{
			OnCalcBoundingRect(pDataPoint, rectDiagramArea, pSeries, j);

			pDataPoint->m_rectDataLabel.SetRectEmpty();
			pDataPoint->m_nSmartLabelAngle = -1;
			pDataPoint->m_bShowSmartLabel = TRUE;

			if (pFormat->m_dataLabelFormat.m_options.m_bShowDataLabel)
			{
				OnCalcDataPointLabelRect(pGM, pDataPoint, rectDiagramArea, pSeries, j);
			}

			if (bIs3D)
			{
				CBCGPChartShape3D* pShape = pDataPoint->GetShape3D();

				if (pShape != NULL)
				{
					pShape->PrepareSidePoints();
				}
			}
		}
	}
}
//****************************************************************************************
void CBCGPBaseChartImpl::OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& /*rectDiagramArea*/, 
											 CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	
	if (pDataPoint != NULL && !pSeries->IsDataPointScreenPointsEmpty(nDataPointIndex))
	{
		CBCGPPoint pt = pSeries->GetDataPointScreenPoint(nDataPointIndex, 0);
		pSeries->SetDataPointBoundingRect(nDataPointIndex, CBCGPRect(pt.x, pt.y, pt.x , pt.y));
	}
}
//****************************************************************************************
BOOL CBCGPBaseChartImpl::OnPrepareDataToCalcDataPointLabelRect(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram,
											CBCGPChartDataPoint* pDataPoint, CBCGPChartSeries* pSeries, int nDataPointIndex, 
											BCGPChartFormatSeries** ppFormatSeries, CBCGPSize& szDataLabel, 
											CBCGPPoint& ptMarkerPos, CBCGPRect& rectBounds)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);
	ASSERT(ppFormatSeries != NULL);

	if (pDataPoint == NULL || pSeries == NULL || pSeries->IsDataPointScreenPointsEmpty(nDataPointIndex) ||
		pDataPoint->IsEmpty())
	{
		return FALSE;
	}

	*ppFormatSeries = (BCGPChartFormatSeries*) pDataPoint->GetFormat();

	if (*ppFormatSeries == NULL)
	{
		*ppFormatSeries = (BCGPChartFormatSeries*) &pSeries->GetSeriesFormat();
	}

	if (!(*ppFormatSeries)->m_dataLabelFormat.m_options.m_bShowDataLabel || !pDataPoint->m_bShowSmartLabel)
	{
		return FALSE;
	}

	ptMarkerPos = OnGetMarkerPoint(pDataPoint, pSeries, nDataPointIndex);

	CBCGPRect rectAxisBounds = pSeries->GetAxesBoundingRect();
	CBCGPRect rectDataPoint = pSeries->GetDataPointBoundingRect(nDataPointIndex);

	if (!m_pRelatedChart->IsChart3D())
	{
		if (!rectDiagram.PtInRect(ptMarkerPos) || !rectAxisBounds.PtInRect(rectDataPoint.TopLeft()))
		{
			return FALSE;
		}
	}
	
	CString strDPLabel;
	if (!pSeries->OnGetDataLabelText(nDataPointIndex, strDPLabel))
	{
		pSeries->GetDataPointLabelText(nDataPointIndex, 
			(*ppFormatSeries)->m_dataLabelFormat.m_options.m_content, strDPLabel);
	}

	if (m_pRelatedChart == NULL || !m_pRelatedChart->OnCalcDataLabelSize(pGM, strDPLabel, pSeries, nDataPointIndex, szDataLabel))
	{
		szDataLabel = OnGetDataLabelSize(pGM, strDPLabel, pDataPoint, pSeries, nDataPointIndex);
	}

	rectBounds = rectDataPoint;

	return TRUE;
}
//****************************************************************************************
void CBCGPBaseChartImpl::AlignRectToArea(const CBCGPRect& rectArea,  CBCGPRect& rectAlign, 
										 BOOL bForceVert, BOOL bForceHorz)
{
	if (rectAlign.left < rectArea.left && (rectAlign.Width() <= rectArea.Width() || bForceHorz))
	{
		rectAlign.OffsetRect(rectArea.left - rectAlign.left, 0);
	}

	if (rectAlign.right > rectArea.right && (rectAlign.Width() <= rectArea.Width() || bForceHorz))
	{
		rectAlign.OffsetRect(rectArea.right - rectAlign.right, 0);
	}

	if (rectAlign.top < rectArea.top && (rectAlign.Height() <= rectArea.Height() || bForceVert))
	{
		rectAlign.OffsetRect(0, rectArea.top - rectAlign.top);
	}

	if (rectAlign.bottom > rectArea.bottom && (rectAlign.Height() <= rectArea.Height() || bForceVert))
	{
		rectAlign.OffsetRect(0, rectArea.bottom - rectAlign.bottom);
	}
}
//****************************************************************************************
BOOL CBCGPBaseChartImpl::CalcDataPointLabelDropLine(const CBCGPRect& rectLabel, const CBCGPPoint& ptDropEnd, 
													CBCGPPoint& ptStart, CBCGPPoint& ptLabelUnderline, 
													BOOL bCalcUnderlinePosition, BOOL bDataLabelHasBorder, 
													BOOL bDropLineToMarker)
{
	CBCGPPoint ptLabelCenter = rectLabel.CenterPoint();

	if (ptDropEnd.x == 0 && ptDropEnd.y == 0 ||  rectLabel.PtInRect(ptDropEnd) || m_pRelatedChart->IsChart3D())
	{
		return FALSE;
	}

	if (m_pRelatedChart->IsChart3D())
	{
		bDropLineToMarker = FALSE;
	}

	if (!bCalcUnderlinePosition && !bDataLabelHasBorder && !bDropLineToMarker)
	{
		return FALSE;
	}

	if (bDataLabelHasBorder || bDropLineToMarker)
	{
		if (rectLabel.right < ptDropEnd.x)
		{
			if (rectLabel.bottom < ptDropEnd.y)
			{
				ptStart.SetPoint(rectLabel.right, rectLabel.bottom);
			}
			else if (rectLabel.top > ptDropEnd.y)
			{
				ptStart.SetPoint(rectLabel.right, rectLabel.top);
			}
			else
			{
				ptStart.SetPoint(rectLabel.right, ptLabelCenter.y);
			}
		}
		else if (rectLabel.left > ptDropEnd.x)
		{
			if (rectLabel.bottom < ptDropEnd.y)
			{
				ptStart.SetPoint(rectLabel.left, rectLabel.bottom);
			}
			else if (rectLabel.top > ptDropEnd.y)
			{
				ptStart.SetPoint(rectLabel.left, rectLabel.top);
			}
			else
			{
				ptStart.SetPoint(rectLabel.left, ptLabelCenter.y);
			}
		}
		else
		{
			if (rectLabel.bottom < ptDropEnd.y)
			{
				ptStart.SetPoint(ptLabelCenter.x, rectLabel.bottom);
			}
			else if (rectLabel.top > ptDropEnd.y)
			{
				ptStart.SetPoint(ptLabelCenter.x, rectLabel.top);
			}
			else
			{
				return FALSE;
			}
		}
	}

	if (bCalcUnderlinePosition && !bDataLabelHasBorder)
	{
		if (rectLabel.right <= ptDropEnd.x)
		{
			ptStart.SetPoint(rectLabel.right, rectLabel.bottom);
			ptLabelUnderline.SetPoint(rectLabel.left, rectLabel.bottom);
			
		}
		else if (rectLabel.left >= ptDropEnd.x)
		{
			ptStart.SetPoint(rectLabel.left, rectLabel.bottom);
			ptLabelUnderline.SetPoint(rectLabel.right, rectLabel.bottom);
		}
		else
		{
			if (rectLabel.bottom < ptDropEnd.y)
			{
				if (ptDropEnd.x - rectLabel.left <= rectLabel.right - ptDropEnd.x)
				{
					ptStart.SetPoint(rectLabel.left, rectLabel.bottom);
					ptLabelUnderline.SetPoint(rectLabel.right, rectLabel.bottom);
				}
				else
				{
					ptStart.SetPoint(rectLabel.right, rectLabel.bottom);
					ptLabelUnderline.SetPoint(rectLabel.left, rectLabel.bottom);
				}

			}
			else if (rectLabel.top > ptDropEnd.y)
			{
				if (ptDropEnd.x == rectLabel.left)
				{
					ptLabelUnderline.SetPoint(rectLabel.right, rectLabel.bottom);
					ptStart.SetPoint(rectLabel.left, rectLabel.bottom);
				}
				else if (fabs(ptDropEnd.x - rectLabel.left) <= fabs(rectLabel.right - ptDropEnd.x))
				{

					double dblACoef = (rectLabel.top - ptDropEnd.y) / (rectLabel.left - ptDropEnd.x);
					double dblBCoef = rectLabel.top - dblACoef * rectLabel.left;

					double dblUnderlineX = (rectLabel.bottom - dblBCoef) / dblACoef;

					ptStart.SetPoint(dblUnderlineX, rectLabel.bottom);
					ptLabelUnderline.SetPoint(rectLabel.right, rectLabel.bottom);
				}
				else
				{
					double dblACoef = (rectLabel.top - ptDropEnd.y) / (rectLabel.right - ptDropEnd.x);
					double dblBCoef = rectLabel.top - dblACoef * rectLabel.right;

					double dblUnderlineX = (rectLabel.bottom - dblBCoef) / dblACoef;

					ptStart.SetPoint(dblUnderlineX, rectLabel.bottom);
					ptLabelUnderline.SetPoint(rectLabel.left, rectLabel.bottom);
				}
			}
			else
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}
//****************************************************************************************
void CBCGPBaseChartImpl::SetDataPointLabelRectAndDropLine(CBCGPChartSeries* pSeries, int nDataPointIndex, 
									BCGPChartFormatSeries* pFormatSeries, CBCGPRect& rectLabel, 
									const CBCGPPoint& ptDropPoint)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);

	CBCGPRect rectBounds = pSeries->GetAxesBoundingRect();
	rectLabel.Normalize();

	rectBounds.InflateRect(rectLabel.Width() + 1, rectLabel.Height() / 2);

	if (!m_pRelatedChart->IsChart3D())
	{
		AlignRectToArea(rectBounds, rectLabel);
	}
	
	pSeries->SetDataPointLabelRect(nDataPointIndex, rectLabel);

	if (rectLabel.PtInRect(ptDropPoint))
	{
		CBCGPPoint ptEmpty;
		pSeries->SetDataPointLabelDropLinePoints(nDataPointIndex, ptEmpty, ptEmpty, ptEmpty);
		return;
	}

	CBCGPPoint ptLabelCenter = rectLabel.CenterPoint();
	CBCGPPoint ptLabelLineStart;
	CBCGPPoint ptLabelUnderline;
	if (CalcDataPointLabelDropLine(rectLabel, ptDropPoint, ptLabelLineStart, ptLabelUnderline, 
		pFormatSeries->m_dataLabelFormat.m_options.m_bUnderlineDataLabel, 
		pFormatSeries->m_dataLabelFormat.m_options.m_bDrawDataLabelBorder,
		pFormatSeries->m_dataLabelFormat.m_options.m_bDropLineToMarker))
	{
		pSeries->SetDataPointLabelDropLinePoints(nDataPointIndex, ptLabelLineStart, ptDropPoint, ptLabelUnderline);
	}
}
//****************************************************************************************
void CBCGPBaseChartImpl::OnCalcDataPointLabelRect(CBCGPGraphicsManager* pGM, CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
											  CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);

	if (pDataPoint == NULL || pSeries == NULL || pSeries->IsDataPointScreenPointsEmpty(nDataPointIndex))
	{
		return;
	}

	BCGPChartFormatSeries* pFormatSeries = NULL;
	CBCGPSize szDataLabelSize;
	CBCGPPoint ptMarker;
	CBCGPRect rectBounds;

	if (!OnPrepareDataToCalcDataPointLabelRect(pGM, rectDiagramArea, pDataPoint, pSeries, nDataPointIndex, 
		&pFormatSeries, szDataLabelSize, ptMarker, rectBounds))
	{
		return;
	}

	BOOL bIsChart3D = m_pRelatedChart->IsChart3D ();

	BCGPChartCategory category = pSeries->GetChartCategory();

	BOOL bCenter = pFormatSeries->m_dataLabelFormat.m_options.m_position == BCGPChartDataLabelOptions::LP_CENTER || 
					pSeries->IsStakedSeries() && bIsChart3D && 
					(category == BCGPChartColumn3D || category == BCGPChartBar3D || category == BCGPChartArea3D);

	CBCGPRect rectLabel = bIsChart3D ? rectBounds : pSeries->GetDataPointLabelRect(nDataPointIndex);

	if (bIsChart3D)
	{
		ptMarker = rectLabel.TopLeft();
	}

	if (bCenter)
	{
		rectLabel.SetRect(ptMarker.x - szDataLabelSize.cx / 2, 
			ptMarker.y - szDataLabelSize.cy / 2, 
			ptMarker.x + szDataLabelSize.cx / 2, 
			ptMarker.y + szDataLabelSize.cy / 2);
	}
	else
	{
		double dblLabelAngle = bcg_deg2rad(pSeries->GetDataPointLabelAngle(nDataPointIndex));
		double dblCos = cos(dblLabelAngle);
		double dblSin = sin(dblLabelAngle);

		CBCGPPoint ptOffset (szDataLabelSize.cx / 2 * dblSin, -szDataLabelSize.cy / 2 * dblCos);

		CBCGPSize szMarker = pFormatSeries->m_markerFormat.GetMarkerSize();

		double dblDistance = pSeries->GetDataPointLabelDistance(nDataPointIndex) +
							 max(szMarker.cx, szMarker.cy) / 2.;

		CBCGPPoint ptLabelCenter(ptMarker.x + dblSin * dblDistance, 
								 ptMarker.y - dblCos * dblDistance);

		rectLabel.SetRect(ptLabelCenter.x - szDataLabelSize.cx / 2, 
							ptLabelCenter.y - szDataLabelSize.cy / 2, 
							ptLabelCenter.x + szDataLabelSize.cx / 2, 
							ptLabelCenter.y + szDataLabelSize.cy / 2);
		rectLabel.OffsetRect(ptOffset);
	}

	SetDataPointLabelRectAndDropLine(pSeries, nDataPointIndex, pFormatSeries, rectLabel, ptMarker);
}
//****************************************************************************************
CBCGPSize CBCGPBaseChartImpl::OnGetDataLabelSize(CBCGPGraphicsManager* pGM, const CString& strDPLabel, 
										const CBCGPChartDataPoint* pDataPoint, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(pGM);
	ASSERT_VALID(pSeries);

	CBCGPSize szDataLabelSize(0, 0);

	if (pDataPoint == NULL)
	{
		return szDataLabelSize;
	}

	const BCGPChartFormatSeries* pFormatSeries = pSeries->GetDataPointFormat(nDataPointIndex, FALSE);

	
	CBCGPSize szKeyLabel(0, 0);
	if (pFormatSeries->m_dataLabelFormat.m_options.m_bIncludeLegendKeyInLabel)
	{
		szKeyLabel = OnCalcLegendKeySize(pSeries, nDataPointIndex);
	}

	CBCGPSize szDataLabelTextSize(0, 0);
	szDataLabelTextSize = pGM->GetTextSize(strDPLabel, pFormatSeries->m_dataLabelFormat.m_textFormat);

	if (szDataLabelTextSize.cy != 0 && pFormatSeries->m_dataLabelFormat.m_options.m_bAutoWordWrap &&
		pFormatSeries->m_dataLabelFormat.m_options.m_dblMaxWidthToHeightRatio > 0 &&
		szDataLabelTextSize.cx / szDataLabelTextSize.cy > pFormatSeries->m_dataLabelFormat.m_options.m_dblMaxWidthToHeightRatio)
	{
		CBCGPTextFormat tf = pFormatSeries->m_dataLabelFormat.m_textFormat;
		tf.SetWordWrap(TRUE);
		szDataLabelTextSize = pGM->GetTextSize(strDPLabel, tf, 
				szDataLabelTextSize.cx / pFormatSeries->m_dataLabelFormat.m_options.m_dblLineCount);
		pSeries->SetUseWordWrapForDataLabels(TRUE, nDataPointIndex);
	}
	else
	{
		pSeries->SetUseWordWrapForDataLabels(FALSE, nDataPointIndex);
	}

	CBCGPSize szContentPadding = pFormatSeries->m_dataLabelFormat.GetContentPadding(TRUE);

	szDataLabelSize.cx = szKeyLabel.cx + szDataLabelTextSize.cx + szContentPadding.cx * 2;
	szDataLabelSize.cy = max(szKeyLabel.cy, szDataLabelTextSize.cy) + szContentPadding.cy * 2;

	if (szKeyLabel.cx > 0)
	{
		szDataLabelSize.cx += pFormatSeries->m_dataLabelFormat.m_options.GetKeyToLabelDistance();
	}

	return szDataLabelSize;
}
//****************************************************************************************
void CBCGPBaseChartImpl::OnDrawDiagramMarkers(CBCGPGraphicsManager* pGM, CBCGPChartSeries* pSeries, const CBCGPRect& rectDiagram)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (pSeries == NULL)
	{
		return;
	}

	ASSERT_VALID(pSeries);

	for (int i = pSeries->GetMinDataPointIndex(); i <= pSeries->GetMaxDataPointIndex(); i++)
	{
		const CBCGPChartDataPoint* pDataPoint = pSeries->GetDataPointAt(i);
		const BCGPChartFormatSeries* pFormat = pSeries->GetDataPointFormat(i, FALSE);
		if (pDataPoint != NULL && !pSeries->IsDataPointScreenPointsEmpty(i) && 
			pFormat->m_markerFormat.m_options.m_bShowMarker)
		{
			OnDrawDiagramMarker(pGM, pSeries, i, pDataPoint, pFormat, rectDiagram);
		}
	}
}
//****************************************************************************************
void CBCGPBaseChartImpl::OnDrawDiagramDataLabels(CBCGPGraphicsManager* pGM, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (pSeries == NULL)
	{
		return;
	}

	ASSERT_VALID(pSeries);

	for (int i = pSeries->GetMinDataPointIndex(); i <= pSeries->GetMaxDataPointIndex(); i++)
	{
		const CBCGPChartDataPoint* pDataPoint = pSeries->GetDataPointAt(i);

		if (pDataPoint != NULL)
		{
			const BCGPChartFormatSeries* pFormat = pSeries->GetDataPointFormat(i, FALSE);

			if (!pFormat->m_dataLabelFormat.m_options.m_bShowDataLabel || !pDataPoint->m_bShowSmartLabel)
			{
				continue;
			}

			CString strDataLabel;

			if (!pSeries->OnGetDataLabelText(i, strDataLabel))
			{
				pSeries->GetDataPointLabelText(i, pFormat->m_dataLabelFormat.m_options.m_content, strDataLabel);
			}

			OnDrawDiagramDataLabel(pGM, strDataLabel, pSeries, i, pDataPoint, pFormat);
		}
	}
}
//****************************************************************************************
void CBCGPBaseChartImpl::OnDrawDiagramDataLabel(CBCGPGraphicsManager* pGM, const CString& strDataLabel, CBCGPChartSeries* pSeries, int nDataPointIndex, 
											const CBCGPChartDataPoint* pDataPoint, const BCGPChartFormatSeries* pFormatSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT(pFormatSeries != NULL);
	ASSERT_VALID(pSeries);

	if (pDataPoint == NULL || !pFormatSeries->m_dataLabelFormat.m_options.m_bShowDataLabel || strDataLabel.IsEmpty() ||
		!pDataPoint->m_bShowSmartLabel)
	{
		return;
	}

	CBCGPRect rectDataLabel = pSeries->GetDataPointLabelRect(nDataPointIndex);

	if (!m_pRelatedChart->IsChart3D())
	{
		CBCGPRect rectAxisBounds = pSeries->GetAxesBoundingRect();

		rectAxisBounds.InflateRect(rectDataLabel.Width() + 2, rectDataLabel.Height() / 2);
		if (rectDataLabel.IsRectEmpty() || !rectAxisBounds.IntersectRect(rectDataLabel))
		{
			return;
		}
	}

	m_pRelatedChart->OnDrawChartSeriesDataLabel(pGM, rectDataLabel, strDataLabel, pSeries, nDataPointIndex);
}
//****************************************************************************************
void CBCGPBaseChartImpl::OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
										  const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, 
										  int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);
	ASSERT(pSeriesStyle != NULL);

	UNREFERENCED_PARAMETER(pSeries);
	UNREFERENCED_PARAMETER(nDataPointIndex);

	pGM->FillRectangle(rectLegendKey, pSeriesStyle->m_markerFormat.m_brFillColor);
}
//****************************************************************************************
void CBCGPBaseChartImpl::OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
												const BCGPChartCellParams& params, CBCGPChartSeries* /*pSeries*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	pGM->FillRectangle(rectLegendKey, params.m_brFill);
}
//****************************************************************************************
void CBCGPBaseChartImpl::OnDrawDiagramMarker(CBCGPGraphicsManager* pGM, CBCGPChartSeries* pSeries, int nDataPointIndex, 
										 const CBCGPChartDataPoint* pDataPoint, const BCGPChartFormatSeries* pFormatSeries, 
										 const CBCGPRect& rectDiagram)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT(pFormatSeries != NULL);
	ASSERT_VALID(pSeries);

	if (pDataPoint == NULL)
	{
		return;
	}

	if (pFormatSeries->m_markerFormat.m_options.m_bShowMarker && !pSeries->IsDataPointScreenPointsEmpty(nDataPointIndex))
	{
		CBCGPPoint ptMarker = OnGetMarkerPoint(pDataPoint, pSeries, nDataPointIndex);

		if (rectDiagram.PtInRect(ptMarker))
		{
			m_pRelatedChart->OnDrawChartSeriesMarker(pGM, ptMarker, pFormatSeries->m_markerFormat, pSeries, nDataPointIndex);
		}
	}
}
//****************************************************************************************
CBCGPPoint CBCGPBaseChartImpl::OnGetMarkerPoint(const CBCGPChartDataPoint* pDataPoint, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	UNREFERENCED_PARAMETER(pDataPoint);

	if (pSeries == NULL)
	{
		return CBCGPPoint();
	}

	ASSERT_VALID(pSeries);

	return pSeries->GetDataPointScreenPoint(nDataPointIndex, 0);
}
//****************************************************************************************
// 3D base support
//****************************************************************************************
void CBCGPBaseChartImpl::CollectSides3D(CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lstInvisibleSides, CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lstVisibleSides)
{
	ASSERT_VALID(this);

	if (m_pRelatedChart == NULL)
	{
		return;
	}

	CBCGPChartDiagram3D* pDiagram3D = m_pRelatedChart->GetDiagram3D();

	if (pDiagram3D != NULL && pDiagram3D->IsThickWallsAndFloor())
	{
		pDiagram3D->CollectWallAndFloorSides(lstInvisibleSides, lstVisibleSides);
	}

	for (int i = 0; i < m_pRelatedChart->GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = m_pRelatedChart->GetSeries(i);

		if (pSeries == NULL || !pSeries->m_bVisible)
		{
			continue;
		}

		for (int j = 0; j < pSeries->GetDataPointCount(); j++)
		{
			CBCGPChartShape3D* pShape = pSeries->GetDataPointShape3D(j);

			if (pShape == NULL)
			{
				continue;
			}

			pShape->AddSidesToList(lstInvisibleSides, FALSE);
			pShape->AddSidesToList(lstVisibleSides, TRUE);
		}
	}
}
//****************************************************************************************
void CBCGPBaseChartImpl::ArrangeSides3D(CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lstSides)
{
	ASSERT_VALID(this);

	if (m_pRelatedChart == NULL)
	{
		return;
	}

	CBCGPChartDiagram3D* pDiagram3D = m_pRelatedChart->GetDiagram3D();

	if (pDiagram3D == NULL)
	{
		return;
	}

	double dblYRotation = pDiagram3D->GetYRotation();
	BOOL bReverseCheck = dblYRotation < -90. || dblYRotation > 90.;

	for (POSITION pos = lstSides.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartSide3D* pSide = lstSides.GetNext(pos);

		pSide->m_bDrawn = FALSE;

		if (pos == NULL)
		{
			break;
		}

		for (POSITION posCheck = pos; posCheck != NULL;)
		{
			CBCGPChartSide3D* pSideCheck = lstSides.GetNext(posCheck);

			CBCGPPoint ptCollision;
			float zSide = 0;
			float zSideCheck = 0;

			if (pSide->HasCollision(*pSideCheck, ptCollision))
			{
				zSide = (float)pSide->CalcZ(ptCollision);
				zSideCheck = (float)pSideCheck->CalcZ(ptCollision);

				if (fabs(zSide - zSideCheck) < 2* FLT_EPSILON)
				{
					continue;
				}

				if (zSide != zSideCheck)
				{
					if (zSide < zSideCheck && !bReverseCheck || zSide > zSideCheck && bReverseCheck)
					{
						pSide->m_lstSidesBefore.AddTail(pSideCheck);
					}
					else
					{
						pSideCheck->m_lstSidesBefore.AddTail(pSide);
					}
				}
			}
		}
	}
}
//****************************************************************************************
double CBCGPBaseChartImpl::CalcDepth3D(double dblUnitSize)
{
	if (m_pRelatedChart == NULL)
	{
		return 0.;
	}

	CBCGPChartDiagram3D* pDiagram3D = m_pRelatedChart->GetDiagram3D();

	if (pDiagram3D == NULL)
	{
		return 0.;
	}

	double dblFrontDistance = pDiagram3D->GetFrontDistancePercent();
	
	if (dblFrontDistance >= 1.)
	{
		dblUnitSize /= dblFrontDistance;
	}
	else
	{
		dblUnitSize = 2 * dblUnitSize * (1. - dblFrontDistance);
	}

	return dblUnitSize;
}
//****************************************************************************************
void CBCGPBaseChartImpl::OnDrawDiagram3D()
{
	ASSERT_VALID(this);

	if (m_pRelatedChart == NULL)
	{
		return;
	}

	CBCGPChartDiagram3D* pDiagram3D = m_pRelatedChart->GetDiagram3D();

	if (pDiagram3D == NULL)
	{
		return;
	}

	CBCGPEngine3D* pEngine3D = pDiagram3D->GetEngine3D();

	if (pEngine3D == NULL)
	{
		return;
	}

	const CBCGPChartTheme& theme = m_pRelatedChart->GetColors();
	double dblThemeOpacity = theme.GetOpacity();

	m_lstInvisibleSides.RemoveAll();
	m_lstVisibleSides.RemoveAll();

	CollectSides3D(m_lstInvisibleSides, m_lstVisibleSides);


	if (dblThemeOpacity < 1.0 && pDiagram3D->IsShowBackfaces() || ForceAddBackfaces())
	{	
		m_lstVisibleSides.AddHead(&m_lstInvisibleSides);
	}
	
	POSITION pos = NULL;

	if (pEngine3D->IsSoftwareRendering())
	{
		ArrangeSides3D(m_lstVisibleSides);

		for (pos = m_lstVisibleSides.GetHeadPosition(); pos != NULL;)
		{
			CBCGPChartSide3D* pSide = m_lstVisibleSides.GetNext(pos);
			pSide->OnDraw(pEngine3D, TRUE, TRUE);
		}
	}
	else
	{
		if (dblThemeOpacity < 1.0 || pDiagram3D->IsThickWallsAndFloor())
		{
			ArrangeSides3D(m_lstVisibleSides);

			for (pos = m_lstVisibleSides.GetHeadPosition(); pos != NULL;)
			{
				CBCGPChartSide3D* pSide = m_lstVisibleSides.GetNext(pos);
				pSide->OnDraw(pEngine3D, TRUE, TRUE);
			}
 		}
 		else
 		{
			for (pos = m_lstVisibleSides.GetHeadPosition(); pos != NULL;)
			{
				CBCGPChartSide3D* pSide = m_lstVisibleSides.GetNext(pos);
				pSide->OnDraw(pEngine3D, TRUE, FALSE);
			}

			for (pos = m_lstVisibleSides.GetHeadPosition(); pos != NULL;)
			{
				CBCGPChartSide3D* pSide = m_lstVisibleSides.GetNext(pos);
				pSide->ResetState(TRUE);
			}
 
			for (pos = m_lstVisibleSides.GetHeadPosition(); pos != NULL;)
			{
				CBCGPChartSide3D* pSide = m_lstVisibleSides.GetNext(pos);
				pSide->OnDraw(pEngine3D, FALSE, TRUE);
			}

			for (pos = m_lstVisibleSides.GetHeadPosition(); pos != NULL;)
			{
				CBCGPChartSide3D* pSide = m_lstVisibleSides.GetNext(pos);
				pSide->ResetState();
 			}
		}
	}
}
//****************************************************************************************
void CBCGPBaseChartImpl::CalcIndexes(CBCGPChartSeries* pSeries, 
									 CArray<int, int>& arStartIndexes, CArray<int, int>& arEndIndexes)

{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);

	arStartIndexes.RemoveAll();
	arEndIndexes.RemoveAll();

	int nMinDataPointIndexTotal = pSeries->GetMinDataPointIndex();
	int nMaxDataPointIndexTotal = pSeries->GetMaxDataPointIndex();

	if (pSeries->GetTreatNulls() == CBCGPChartSeries::TN_NO_PAINT)
	{
		for (int i = nMinDataPointIndexTotal; i <= nMaxDataPointIndexTotal; i++)
		{
			while (pSeries->IsDataPointScreenPointsEmpty(i) && i <= nMaxDataPointIndexTotal)
			{
				i++;
			}
			
			if (i > nMaxDataPointIndexTotal)
			{
				break;
			}
			
			arStartIndexes.Add(i);
			
			while (!pSeries->IsDataPointScreenPointsEmpty(i) && i <= nMaxDataPointIndexTotal)
			{
				i++;
			}
			
			if (i > nMaxDataPointIndexTotal)
			{
				arEndIndexes.Add(nMaxDataPointIndexTotal);
			}
			else
			{
				arEndIndexes.Add(i - 1);
			}
		}
	}
	else
	{
		arStartIndexes.Add(nMinDataPointIndexTotal);
		arEndIndexes.Add(nMaxDataPointIndexTotal);
	}
}
//****************************************************************************************
// Line chart specific implementation
//****************************************************************************************
void CBCGPLineChartImpl::OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);
	UNREFERENCED_PARAMETER(rectDiagramArea);

	if (m_pRelatedChart == NULL || pSeries == NULL)
	{
		return;
	}

	CBCGPChartLineSeries* pLineSeries = DYNAMIC_DOWNCAST(CBCGPChartLineSeries, pSeries);
	CBCGPChartBaseFormula* pBaseFormula = pSeries->GetFormula();

	if (pBaseFormula != NULL && pBaseFormula->IsNonDiscreteCurve() || pSeries->IsVirtualMode())
	{
		CBCGPChartTrendFormula* pFormula = DYNAMIC_DOWNCAST(CBCGPChartTrendFormula, pBaseFormula);

		if (pFormula != NULL)
		{
			ASSERT_VALID(pFormula);
		}

		const CBCGPPointsArray& arScreenPoints = pFormula != NULL ? pFormula->GetScreenPoints() : 
												pSeries->GetVirtualSeriesScreenPoints();

		CBCGPPolygonGeometry geometry(arScreenPoints, pLineSeries->IsConnectFirstLastPoints());
		m_pRelatedChart->OnDrawChartSeriesItem(pGM, geometry, pSeries, -1, 
					CBCGPChartVisualObject::CE_MAIN_ELEMENT, !pLineSeries->IsFillClosedShape(), FALSE);
		return;
	}

	BOOL bCheckLineOffsets = CBCGPGraphicsManagerD2D::m_bCheckLineOffsets;
	CBCGPGraphicsManagerD2D::m_bCheckLineOffsets = FALSE;

	BCGPChartFormatSeries::ChartCurveType curveType = pSeries->GetCurveType();

	CArray<int, int> arStartIndexes;
	CArray<int, int> arEndIndexes;
	
	CalcIndexes(pSeries, arStartIndexes, arEndIndexes);
	for (int j = 0; j < arStartIndexes.GetSize(); j++)
	{
		int nFirstIndex = arStartIndexes[j];
		int nLastIndex = arEndIndexes[j];

		m_linePoints.RemoveAll();

		if (curveType == BCGPChartFormatSeries::CCT_LINE || 
			curveType == BCGPChartFormatSeries::CCT_STEP ||
			curveType == BCGPChartFormatSeries::CCT_REVERSED_STEP)
		{
			const CBCGPChartDataPoint* pDataPointEnd = NULL;
			const CBCGPChartDataPoint* pDataPointStart = pSeries->GetDataPointAt(nFirstIndex);

			int nFirstNonEmptyIndex = -1;
			int nLastNonEmptyIndex = -1;

			int nLastNonEmptyPoint = -1;
			for (int i = nFirstIndex; i <= nLastIndex; i++)
			{
				pDataPointStart = pSeries->GetDataPointAt(i);

				if (i + 1 <= pSeries->GetMaxDataPointIndex())
				{
					pDataPointEnd = pSeries->GetDataPointAt(i + 1);

					if (pDataPointStart != NULL && pDataPointEnd != NULL)
					{
						BOOL bIsStartEmpty = pSeries->IsDataPointScreenPointsEmpty(i);
						BOOL bIsEndEmpty = pSeries->IsDataPointScreenPointsEmpty(i + 1);

						if (pSeries->GetTreatNulls() == CBCGPChartSeries::TN_SKIP)
						{
							if (nLastNonEmptyPoint != -1 && nLastNonEmptyPoint != i && !bIsStartEmpty)
							{
								OnDrawDiagramLine(pGM, pSeries, nLastNonEmptyPoint, i, 
									pSeries->GetDataPointAt(nLastNonEmptyPoint), pDataPointStart, curveType);
							}

							if (!bIsStartEmpty)
							{
								nLastNonEmptyPoint = i;
							}
						}

						if (!bIsStartEmpty && !bIsEndEmpty)
						{
							OnDrawDiagramLine(pGM, pSeries, i, i + 1, pDataPointStart, pDataPointEnd, curveType);
						}

						if (nFirstNonEmptyIndex == -1 && !bIsStartEmpty)
						{
							nFirstNonEmptyIndex = i;
						}

						if (!bIsEndEmpty)
						{
							nLastNonEmptyIndex = i + 1;
						}
					}
				}
				else if (pSeries->GetTreatNulls() == CBCGPChartSeries::TN_SKIP)
				{
					if (!pSeries->IsDataPointScreenPointsEmpty(i) && 
						nLastNonEmptyPoint != -1 && nLastNonEmptyPoint != i)
					{
						OnDrawDiagramLine(pGM, pSeries, nLastNonEmptyPoint, i, 
									pSeries->GetDataPointAt(nLastNonEmptyPoint), pDataPointStart, curveType);
						nLastNonEmptyPoint = i;
					}
				}
			}

			if (pLineSeries != NULL && pLineSeries->IsConnectFirstLastPoints())
			{
				pDataPointStart = pSeries->GetDataPointAt(nFirstIndex);
				OnDrawDiagramLine(pGM, pSeries, nLastNonEmptyIndex, nFirstNonEmptyIndex, 
					pSeries->GetDataPointAt(nLastNonEmptyIndex), pSeries->GetDataPointAt(nFirstNonEmptyIndex), curveType);

				if (pLineSeries->IsFillClosedShape() && nLastIndex > 1)
				{
					CBCGPPolygonGeometry geometry(m_linePoints, TRUE);
					m_pRelatedChart->OnDrawChartSeriesItem(pGM, geometry, pSeries, -1, 
							CBCGPChartVisualObject::CE_MAIN_ELEMENT, FALSE, TRUE);
				}
			}
		}
		else if (curveType == BCGPChartFormatSeries::CCT_SPLINE || 
				 curveType == BCGPChartFormatSeries::CCT_SPLINE_HERMITE)
		{
			CBCGPSplineGeometry::BCGP_SPLINE_TYPE splineType = CBCGPSplineGeometry::BCGP_SPLINE_TYPE_KB;

			if (curveType == BCGPChartFormatSeries::CCT_SPLINE_HERMITE)
			{
				splineType = CBCGPSplineGeometry::BCGP_SPLINE_TYPE_HERMITE;
			}

			CBCGPPointsArray linePoints;

			int i = 0;
			for (i = nFirstIndex; i <= nLastIndex; i++)
			{
				if (!pSeries->IsDataPointScreenPointsEmpty(i))
				{
					linePoints.Add(pSeries->GetDataPointScreenPoint(i, 0));
				}
			}

			int nCount = (int)linePoints.GetSize();
			if (nCount > 0)
			{
				BOOL bIsConnectPoints = pLineSeries != NULL && pLineSeries->IsConnectFirstLastPoints();
				BOOL bIsFillClosedShape = pLineSeries != NULL && pLineSeries->IsFillClosedShape();

				if (nCount >= 3)
				{
					if (bIsConnectPoints && linePoints[0] != linePoints[nCount - 1])
					{
						CBCGPPoint pt = linePoints.GetAt(0); 
						linePoints.Add(pt);
					}

					CBCGPSplineGeometry splineGeometry(linePoints, splineType, bIsConnectPoints);
					m_pRelatedChart->OnDrawChartSeriesItem(pGM, splineGeometry, pSeries, -1, 
						CBCGPChartVisualObject::CE_MAIN_ELEMENT, !bIsFillClosedShape, FALSE);
				}
				else
				{
					CBCGPPolygonGeometry geometry(linePoints, bIsConnectPoints);
					m_pRelatedChart->OnDrawChartSeriesItem(pGM, geometry, pSeries, -1, 
						CBCGPChartVisualObject::CE_MAIN_ELEMENT, !bIsFillClosedShape, FALSE);
				}
			}
		}
	}

	CBCGPGraphicsManagerD2D::m_bCheckLineOffsets = bCheckLineOffsets;
}
//****************************************************************************************
void CBCGPLineChartImpl::OnGetStepPoint(CBCGPPoint ptBottomStart, CBCGPPoint ptBottomEnd, BOOL bXAxisVert, 
										BCGPChartFormatSeries::ChartCurveType curveType, CBCGPPoint& ptStep)
{
	if (bXAxisVert)
	{
		if (curveType == BCGPChartFormatSeries::CCT_STEP)
		{
			ptStep.x = ptBottomStart.x;
			ptStep.y = ptBottomEnd.y;
		}
		else
		{
			ptStep.x = ptBottomEnd.x;
			ptStep.y = ptBottomStart.y;
		}
	}
	else
	{
		if (curveType == BCGPChartFormatSeries::CCT_STEP)
		{
			ptStep.x = ptBottomEnd.x; 
			ptStep.y = ptBottomStart.y;
		}
		else
		{
			ptStep.x = ptBottomStart.x;
			ptStep.y = ptBottomEnd.y;
		}
	}

}
//****************************************************************************************
void CBCGPLineChartImpl::OnDrawDiagramLine(CBCGPGraphicsManager* pGM, CBCGPChartSeries* pSeries, 
										    int nStartDataPointIndex, int nEndDataPointIndex,
											const CBCGPChartDataPoint* pDataPointStart, 
											const CBCGPChartDataPoint* pDataPointEnd, 
											BCGPChartFormatSeries::ChartCurveType curveType)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(pSeries);

	UNREFERENCED_PARAMETER(pDataPointStart);
	UNREFERENCED_PARAMETER(pDataPointEnd);

	if (pSeries->IsDataPointScreenPointsEmpty(nStartDataPointIndex) || 
		pSeries->IsDataPointScreenPointsEmpty(nEndDataPointIndex))
	{
		return;
	}

	CBCGPPoint ptBottomStart = pSeries->GetDataPointScreenPoint(nStartDataPointIndex, 0);
	CBCGPPoint ptBottomEnd = pSeries->GetDataPointScreenPoint(nEndDataPointIndex, 0);

	if (curveType == BCGPChartFormatSeries::CCT_STEP || curveType == BCGPChartFormatSeries::CCT_REVERSED_STEP)
	{
		CBCGPChartAxis* pXAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X);
		CBCGPPoint ptStep;

		if (pXAxis == NULL)
		{
			return;
		}

		ASSERT_VALID(pXAxis);

		OnGetStepPoint(ptBottomStart, ptBottomEnd, pXAxis->IsVertical(), curveType, ptStep);

		m_pRelatedChart->OnDrawChartSeriesLine(pGM, ptBottomStart, ptStep, pSeries, nEndDataPointIndex);
		m_pRelatedChart->OnDrawChartSeriesLine(pGM, ptStep, ptBottomEnd, pSeries, nEndDataPointIndex);

		if (pSeries->IsFillClosedShape())
		{
			m_linePoints.Add(ptBottomStart);
			m_linePoints.Add(ptStep);
			m_linePoints.Add(ptBottomEnd);
		}
		
		return;
	}

	if (pSeries->GetChartType() == BCGP_CT_RANGE)
	{
		CBCGPPoint ptTopStart = pSeries->GetDataPointScreenPoint(nStartDataPointIndex, 1);
		CBCGPPoint ptTopEnd = pSeries->GetDataPointScreenPoint(nEndDataPointIndex, 1);

		if (ptTopStart.x == 0 && ptTopStart.y == 0)
		{
			ptTopStart = ptBottomStart;
		}

		if (ptTopEnd.x == 0 && ptTopEnd.y == 0)
		{
			ptTopEnd = ptBottomEnd;
		}

		CBCGPPointsArray arPoints(4);

		arPoints[0] = ptBottomStart;
		arPoints[1] = ptTopStart;
		arPoints[2] = ptTopEnd;
		arPoints[3] = ptBottomEnd;

		CBCGPPolygonGeometry geometry(arPoints);
		
		m_pRelatedChart->OnDrawChartSeriesItem(pGM, geometry, pSeries, nEndDataPointIndex, 
			CBCGPChartVisualObject::CE_MAIN_ELEMENT, FALSE, FALSE);
		m_pRelatedChart->OnDrawChartSeriesLine(pGM, ptTopStart, ptTopEnd, pSeries, nEndDataPointIndex);
	}

	m_pRelatedChart->OnDrawChartSeriesLine(pGM, ptBottomStart, ptBottomEnd, pSeries, nEndDataPointIndex);

	if (pSeries->IsFillClosedShape())
	{
		m_linePoints.Add(ptBottomStart);
		m_linePoints.Add(ptBottomEnd);
	}
}
//****************************************************************************************
void CBCGPLineChartImpl::OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
							  const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);
	ASSERT(pSeriesStyle != NULL);

	UNREFERENCED_PARAMETER(nDataPointIndex);

	CBCGPPoint ptLineStart (rectLegendKey.left, rectLegendKey.CenterPoint().y);
	CBCGPPoint ptLineEnd (rectLegendKey.right, rectLegendKey.CenterPoint().y);

	BCGPChartFormatLine& lineStyle = ((BCGPChartFormatSeries*) pSeriesStyle)->m_seriesElementFormat.m_outlineFormat;

	double dblLineWidth = pSeriesStyle->m_seriesElementFormat.m_outlineFormat.GetLineWidth(TRUE);
	double dblLineWidthSaved = pSeriesStyle->m_seriesElementFormat.m_outlineFormat.GetLineWidth(FALSE);

	if (dblLineWidth >= rectLegendKey.Height())
	{
		dblLineWidth = rectLegendKey.Height() / max(lineStyle.GetScaleRatio().cx, lineStyle.GetScaleRatio().cy) - 2;
		lineStyle.m_dblWidth = dblLineWidth;
	}

	CBCGPChartLineSeries* pLineSeries = DYNAMIC_DOWNCAST(CBCGPChartLineSeries, pSeries);
	if (pLineSeries != NULL && pLineSeries->IsFillClosedShape() && pLineSeries->GetDataPointCount() > 2)
	{
		m_pRelatedChart->OnDrawChartSeriesItem(pGM, rectLegendKey, pSeries, nDataPointIndex, CBCGPChartVisualObject::CE_MAIN_ELEMENT, FALSE, FALSE);
	}
	else
	{
		m_pRelatedChart->OnDrawChartSeriesLine(pGM, ptLineStart, ptLineEnd, pSeries, nDataPointIndex);

		CBCGPChartBaseFormula* pFormula = pLineSeries == NULL ? NULL : pLineSeries->GetFormula();

		if (pSeriesStyle->m_markerFormat.m_options.m_bShowMarker && 
			(pFormula == NULL || pFormula != NULL && !pFormula->IsNonDiscreteCurve()) && !m_pRelatedChart->IsChart3D())
		{
			CBCGPRect rectMarker = rectLegendKey;
			rectMarker.left = rectMarker.CenterPoint().x - rectLegendKey.Height() / 2;
			rectMarker.right = rectMarker.left + rectLegendKey.Height();

			BCGPChartFormatMarker markerFormat = pSeriesStyle->m_markerFormat;
			CBCGPSize szMarker = markerFormat.GetMarkerSize();
			double dblMarkerSize = max(szMarker.cx, szMarker.cy);
			CBCGPSize szScaleRatio = markerFormat.GetScaleRatio();

			if (dblMarkerSize + markerFormat.m_outlineFormat.GetLineWidth(TRUE) >= rectLegendKey.Height())
			{
				markerFormat.m_options.SetMarkerSize((int)(rectLegendKey.Height() / szScaleRatio.cy));
				markerFormat.m_outlineFormat.m_dblWidth = 1.;
			}

			m_pRelatedChart->OnDrawChartSeriesMarker(pGM, rectLegendKey.CenterPoint(), markerFormat, pSeries, nDataPointIndex);
		}
	}

	lineStyle.m_dblWidth = dblLineWidthSaved;
}
//****************************************************************************************
void CBCGPLineChartImpl::OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
												const BCGPChartCellParams& params, CBCGPChartSeries* pSeries)
{
	CBCGPPoint ptLineStart (rectLegendKey.left, rectLegendKey.CenterPoint().y);
	CBCGPPoint ptLineEnd (rectLegendKey.right, rectLegendKey.CenterPoint().y);
	
	BCGPChartFormatLine& lineStyle = (BCGPChartFormatLine&)params.m_lineStyle;
	
	double dblLineWidth = lineStyle.GetLineWidth(TRUE);

	if (dblLineWidth >= rectLegendKey.Height())
	{
		dblLineWidth = rectLegendKey.Height() / max(lineStyle.GetScaleRatio().cx, lineStyle.GetScaleRatio().cy) - 2;
	}

	CBCGPChartLineSeries* pLineSeries = DYNAMIC_DOWNCAST(CBCGPChartLineSeries, pSeries);

	if (pLineSeries != NULL && pLineSeries->IsFillClosedShape() && pLineSeries->GetDataPointCount() > 2)
	{
		m_pRelatedChart->OnDrawChartRect(pGM, rectLegendKey, &params.m_brFill, 
			&params.m_lineStyle.m_brLineColor, dblLineWidth, 
			&params.m_lineStyle.m_strokeStyle, FALSE, FALSE);
	}
	else
	{
		pGM->DrawLine(ptLineStart, ptLineEnd, params.m_lineStyle.m_brLineColor, dblLineWidth,
			&params.m_lineStyle.m_strokeStyle);
		
		CBCGPChartBaseFormula* pFormula = pLineSeries == NULL ? NULL : pLineSeries->GetFormula();
		
		if (params.m_markerOptions.m_bShowMarker && 
			(pFormula == NULL || pFormula != NULL && !pFormula->IsNonDiscreteCurve()) && 
			!m_pRelatedChart->IsChart3D())
		{
			CBCGPRect rectMarker = rectLegendKey;
			rectMarker.left = rectMarker.CenterPoint().x - rectLegendKey.Height() / 2;
			rectMarker.right = rectMarker.left + rectLegendKey.Height();
			
			CBCGPSize szMarker = params.GetMarkerSize();
			double dblMarkerSize = max(szMarker.cx, szMarker.cy);
			double dblLineWidth = params.m_markerLineStyle.GetLineWidth(TRUE);
			
			if (dblMarkerSize + dblLineWidth >= rectLegendKey.Height())
			{
				szMarker.SetSize(rectLegendKey.Height() - dblLineWidth, rectLegendKey.Height() - dblLineWidth);
			}
			
			m_pRelatedChart->OnDrawChartSeriesMarkerEx(pGM, rectLegendKey.CenterPoint(), szMarker, 
				params.m_brMarkerFill, params.m_markerLineStyle, params.m_markerOptions.m_markerShape);
		}
	}
}
//****************************************************************************************
CBCGPSize CBCGPLineChartImpl::OnCalcLegendKeySize(CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(pSeries);
	UNREFERENCED_PARAMETER(nDataPointIndex);

	if (m_pRelatedChart == NULL)
	{
		return CBCGPSize();
	}

	ASSERT_VALID(m_pRelatedChart);

	double nMarkerSize = (double)pSeries->GetSeriesFormat().m_markerFormat.m_options.GetMarkerSize();
	nMarkerSize = bcg_clamp(nMarkerSize, -nDefaultLegendKeyHeight, nDefaultLegendKeyHeight);

	return CBCGPSize(nDefaultLegendKeyWidth * m_pRelatedChart->GetScaleRatio().cx, nMarkerSize * m_pRelatedChart->GetScaleRatio().cy);
}
//****************************************************************************************
CBCGPSize CBCGPLineChartImpl::OnCalcLegendKeySizeEx(const BCGPChartCellParams& params, CBCGPChartSeries* /*pSeries*/)
{
	double dblMarkerSize = params.GetMarkerSize(FALSE).cy;
	dblMarkerSize = bcg_clamp(dblMarkerSize, -nDefaultLegendKeyHeight, nDefaultLegendKeyHeight);
	
	return CBCGPSize(nDefaultLegendKeyWidth * params.GetScaleRatio().cx, dblMarkerSize * params.GetScaleRatio().cy);
}
//****************************************************************************************
// 3D Line Chart specific implementation
//****************************************************************************************
void CBCGPLineChart3DImpl::OnDrawDiagram(CBCGPGraphicsManager* /*pGM*/, const CBCGPRect& /*rectDiagramArea*/, CBCGPChartSeries* /*pSeries*/)
{
	ASSERT_VALID(this);
	OnDrawDiagram3D();
}
//****************************************************************************************
void CBCGPLineChart3DImpl::OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& /*rectDiagramArea*/, 
		CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	CBCGPChartAxis* pXAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X);
	CBCGPChartAxis* pZAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_Z);

	if (pXAxis == NULL || pZAxis == NULL)
	{
		return;
	}

	CBCGPChartDiagram3D* pDiagram3D = m_pRelatedChart->GetDiagram3D();

	if (pDiagram3D == NULL)
	{
		return;
	}

	double dblXRotation = pDiagram3D->GetXRotation();
	int nDPCount = pSeries->GetDataPointCount();

	pSeries->SetDataPointShape3D(NULL, nDataPointIndex);

	BOOL bCreateCube = TRUE;

	if (pSeries->IsDataPointScreenPointsEmpty(nDataPointIndex))
	{
		bCreateCube = FALSE;
	}

	ASSERT_VALID(pXAxis);

	double dblXUnitSize = pXAxis->GetAxisUnitSize() / 2;

	CBCGPPoint pt;
	CBCGPPoint ptNext;
	int nNextIndex = nDataPointIndex + 1;

	if (pSeries->IsDataPointScreenPointsEmpty(nNextIndex))
	{
		if (pSeries->GetTreatNulls() == CBCGPChartSeries::TN_NO_PAINT)
		{
			bCreateCube = FALSE;
		}
		else
		{
			while(pSeries->IsDataPointScreenPointsEmpty(nNextIndex) && nNextIndex < nDPCount)
			{
				nNextIndex++;
			}
		}
	}

	if (nNextIndex == nDPCount)
	{
		bCreateCube = FALSE;
	}

	pt = pSeries->GetDataPointScreenPoint(nDataPointIndex, 0);
	ptNext = pSeries->GetDataPointScreenPoint(nNextIndex, 0);

	double dblZ = m_pRelatedChart->IsChart3DGrouped() ? 0 : pt.z;
	double dblDepth = CalcDepth3D(pZAxis->GetAxisUnitSize() / 2);
	m_pRelatedChart->SetBaseDepthPercent3D(dblXUnitSize);

	CBCGPChartShape3DCube cube(pt, ptNext, -0.01 * pSeries->GetDataPointFormat(nDataPointIndex, FALSE)->m_dbl3DLineThickness, dblZ, dblDepth);
	pDiagram3D->TransformShape(cube);

	cube.m_pSeries = pSeries;
	cube.m_nDataPointIndex = nDataPointIndex;

	if (bCreateCube)
	{
		pSeries->SetDataPointShape3D(cube, nDataPointIndex);
	}

	int nSide = CBCGPChartShape3DCube::CUBE_SIDE_LEFT;

	CBCGPRect rectBounds = cube.GetSides().GetAt(nSide).m_rectMinMaxPoints; 

	if (dblXRotation <= 90 || dblXRotation >= 270)
	{
		rectBounds.right = pDataPoint->m_rectBounds.left;
		rectBounds.bottom = pDataPoint->m_rectBounds.top;
	}
	else
	{
		rectBounds.left = pDataPoint->m_rectBounds.right;
		rectBounds.top = pDataPoint->m_rectBounds.bottom;
	}

	pSeries->SetDataPointBoundingRect(nDataPointIndex, rectBounds);
}
//****************************************************************************************
// Area chart specific implementation
//****************************************************************************************
void CBCGPAreaChartImpl::OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);

	UNREFERENCED_PARAMETER(rectDiagramArea);

	CBCGPChartAreaSeries* pAreaSeries = DYNAMIC_DOWNCAST(CBCGPChartAreaSeries, pSeries);

	if (m_pRelatedChart == NULL || pAreaSeries == NULL)
	{
		return;
	}

	CBCGPPoint ptAxisStartHorz;
	CBCGPPoint ptAxisEndHorz;

	CBCGPChartAxis* pAxisHorz = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X);
	CBCGPChartAxis* pAxisVert = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_Y);

	if (pAxisHorz == NULL || pAxisVert == NULL)
	{
		return;
	}

	BCGPChartFormatSeries::ChartCurveType curveType = pSeries->GetCurveType();

	BOOL bIsEmpty = FALSE;
	CBCGPPoint ptOrigin = m_pRelatedChart->ScreenPointFromChartData(CBCGPChartData(pAreaSeries->GetAreaOrigin()), 0, bIsEmpty, pSeries);

	pAxisHorz->GetAxisPos(ptAxisStartHorz, ptAxisEndHorz);

	ASSERT_VALID(pAxisHorz);
	ASSERT_VALID(pAxisVert);

	CBCGPPoint ptAxisStartVert;
	CBCGPPoint ptAxisEndVert;

	pAxisVert->GetAxisPos(ptAxisStartVert, ptAxisEndVert);
	
	BOOL bIsStacked = pSeries->IsStakedSeries() || pSeries->IsRangeSeries();

	CArray<int, int> arStartIndexes;
	CArray<int, int> arEndIndexes;

	CalcIndexes(pSeries, arStartIndexes, arEndIndexes);

	for (int j = 0; j < arStartIndexes.GetSize(); j++)
	{
		int nMinDataPointIndex = arStartIndexes[j];
		int nMaxDataPointIndex = arEndIndexes[j];

		CBCGPPointsArray pointsTopLine;
		CBCGPPoint ptFirstScreenPoint;
		CBCGPPoint ptLastScreenPoint;
		BOOL bFirstScreenPointFound = FALSE;

		for (int i = nMinDataPointIndex; i <= nMaxDataPointIndex; i++)
		{
			if (pSeries->IsDataPointScreenPointsEmpty(i))
			{
				continue;
			}

			CBCGPPoint ptScreenPoint = pSeries->GetDataPointScreenPoint(i, 0);

			if (!bFirstScreenPointFound && !bIsStacked)
			{
				if (!pAxisHorz->IsVertical())
				{
					if (ptAxisStartHorz.y < ptOrigin.y)
					{
						ptOrigin.y = ptAxisStartHorz.y;
					}

					ptFirstScreenPoint.SetPoint(ptScreenPoint.x, ptOrigin.y);
				}
				else
				{
					if (ptAxisStartHorz.x > ptOrigin.x)
					{
						ptOrigin.x = ptAxisStartHorz.x;
					}

					ptFirstScreenPoint.SetPoint(ptOrigin.x, ptScreenPoint.y);
				}

				bFirstScreenPointFound = TRUE;
			}

			
			pointsTopLine.Add(ptScreenPoint);

			if ((curveType == BCGPChartFormatSeries::CCT_STEP || 
				curveType == BCGPChartFormatSeries::CCT_REVERSED_STEP) && i != nMaxDataPointIndex)
			{
				BOOL bNonEmptyPointFound = FALSE;
				int j = i + 1;
				while (j <= nMaxDataPointIndex)
				{
					if (!pSeries->IsDataPointScreenPointsEmpty(j))
					{
						bNonEmptyPointFound = TRUE;
						i = j - 1;
						break;
					}

					j++;
				}

				if (!bNonEmptyPointFound)
				{
					ptLastScreenPoint = ptScreenPoint;
					break;
				}

				CBCGPPoint ptNextPoint = pSeries->GetDataPointScreenPoint(j, 0);
				if (curveType == BCGPChartFormatSeries::CCT_STEP)
				{
					pAxisHorz->IsVertical() ? ptNextPoint.x = ptScreenPoint.x : ptNextPoint.y = ptScreenPoint.y;
				}
				else 
				{
					pAxisHorz->IsVertical() ? ptNextPoint.y = ptScreenPoint.y : ptNextPoint.x = ptScreenPoint.x;
				}

				pointsTopLine.Add(ptNextPoint);
			}

			ptLastScreenPoint = ptScreenPoint;
		}

		if (!pAxisHorz->IsVertical())
		{
			ptLastScreenPoint.SetPoint(ptLastScreenPoint.x, ptOrigin.y);
		}
		else
		{
			ptLastScreenPoint.SetPoint(ptOrigin.x, ptLastScreenPoint.y);
		}

		CBCGPPointsArray pointsBottomLine;

		if (bIsStacked)
		{
			for (int i = nMaxDataPointIndex; i >= nMinDataPointIndex ; i--)
			{
				if (pSeries->IsDataPointScreenPointsEmpty(i))
				{
					continue;
				}

				CBCGPPoint ptScreenPoint = pSeries->GetDataPointScreenPoint(i, 1);
				pointsBottomLine.Add(ptScreenPoint);

				if ((curveType == BCGPChartFormatSeries::CCT_STEP || 
					curveType == BCGPChartFormatSeries::CCT_REVERSED_STEP) && i != nMinDataPointIndex )
				{
					CBCGPPoint ptNextPoint = pSeries->GetDataPointScreenPoint(i - 1, 1);
					if (curveType == BCGPChartFormatSeries::CCT_STEP)
					{
						pAxisHorz->IsVertical() ? ptNextPoint.y = ptScreenPoint.y : ptNextPoint.x = ptScreenPoint.x;
					}
					else 
					{
						pAxisHorz->IsVertical() ? ptNextPoint.x = ptScreenPoint.x : ptNextPoint.y = ptScreenPoint.y;
					}

					pointsBottomLine.Add(ptNextPoint);
				}
				
			}
		}

		if (curveType == BCGPChartFormatSeries::CCT_LINE || curveType == BCGPChartFormatSeries::CCT_STEP ||
			curveType == BCGPChartFormatSeries::CCT_REVERSED_STEP || pointsTopLine.GetSize() < 3)
		{
			CBCGPComplexGeometry complex;
			CBCGPPolygonGeometry geometryTop(pointsTopLine, FALSE, CBCGPPolygonGeometry::BCGP_CURVE_TYPE_LINE);
			CBCGPPolygonGeometry geometryBottom;

			complex.AddPoints(geometryTop.GetPoints(), CBCGPPolygonGeometry::BCGP_CURVE_TYPE_LINE);
			if (bIsStacked)
			{
				geometryBottom.SetPoints(pointsBottomLine, FALSE, CBCGPPolygonGeometry::BCGP_CURVE_TYPE_LINE);
				complex.AddPoints(pointsBottomLine, CBCGPPolygonGeometry::BCGP_CURVE_TYPE_LINE);
			}
			else
			{
				complex.AddLine(ptLastScreenPoint);
				complex.AddLine(ptFirstScreenPoint);
			}
			complex.SetClosed(TRUE);

			m_pRelatedChart->OnDrawChartSeriesItem(pGM, complex, pSeries, -1, CBCGPChartVisualObject::CE_MAIN_ELEMENT, FALSE, TRUE);
			m_pRelatedChart->OnDrawChartSeriesItem(pGM, geometryTop, pSeries, -1, CBCGPChartVisualObject::CE_MAIN_ELEMENT, TRUE, FALSE);

			if (pSeries->IsRangeSeries())
			{
				m_pRelatedChart->OnDrawChartSeriesItem(pGM, geometryBottom, pSeries, -1, CBCGPChartVisualObject::CE_MAIN_ELEMENT, TRUE, FALSE);
			}
		}
		else if (curveType == BCGPChartFormatSeries::CCT_SPLINE || curveType == BCGPChartFormatSeries::CCT_SPLINE_HERMITE)
		{
			CBCGPSplineGeometry::BCGP_SPLINE_TYPE splineType = CBCGPSplineGeometry::BCGP_SPLINE_TYPE_KB;

			if (curveType == BCGPChartFormatSeries::CCT_SPLINE_HERMITE)
			{
				splineType = CBCGPSplineGeometry::BCGP_SPLINE_TYPE_HERMITE;
			}
		
			CBCGPComplexGeometry complex;
			CBCGPSplineGeometry geometryTop(pointsTopLine, splineType, FALSE);
			CBCGPSplineGeometry geometryBottom;

			complex.AddPoints(geometryTop.GetPoints(), CBCGPPolygonGeometry::BCGP_CURVE_TYPE_BEZIER);
			if (bIsStacked)
			{
				geometryBottom.SetPoints(pointsBottomLine, splineType, FALSE);
				complex.AddPoints(geometryBottom.GetPoints(), CBCGPPolygonGeometry::BCGP_CURVE_TYPE_BEZIER);
			}
			else
			{
				complex.AddLine(ptLastScreenPoint);
				complex.AddLine(ptFirstScreenPoint);
			}
			complex.SetClosed(TRUE);

			m_pRelatedChart->OnDrawChartSeriesItem(pGM, complex, pSeries, -1, CBCGPChartVisualObject::CE_MAIN_ELEMENT, FALSE, TRUE);
			m_pRelatedChart->OnDrawChartSeriesItem(pGM, geometryTop, pSeries, -1, CBCGPChartVisualObject::CE_MAIN_ELEMENT, TRUE, FALSE);

			if (pSeries->IsRangeSeries())
			{
				m_pRelatedChart->OnDrawChartSeriesItem(pGM, geometryBottom, pSeries, -1, CBCGPChartVisualObject::CE_MAIN_ELEMENT, TRUE, FALSE);
			}
		}
	}
}
//****************************************************************************************
CBCGPSize CBCGPAreaChartImpl::OnCalcLegendKeySize(CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	UNREFERENCED_PARAMETER(pSeries);
	UNREFERENCED_PARAMETER(nDataPointIndex);
	
	if (m_pRelatedChart == NULL)
	{
		return CBCGPSize();
	}
	
	return CBCGPSize(nDefaultLegendKeyWidth * m_pRelatedChart->GetScaleRatio().cx, nDefaultLegendKeyHeight * m_pRelatedChart->GetScaleRatio().cy);
}
//****************************************************************************************
CBCGPSize CBCGPAreaChartImpl::OnCalcLegendKeySizeEx(const BCGPChartCellParams& params, CBCGPChartSeries* /*pSeries*/)
{
	return CBCGPSize(nDefaultLegendKeyWidth * params.GetScaleRatio().cx, nDefaultLegendKeyHeight * params.GetScaleRatio().cy);
}
//****************************************************************************************s
void CBCGPAreaChartImpl::OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
					  const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);
	ASSERT(pSeriesStyle != NULL);

	UNREFERENCED_PARAMETER(nDataPointIndex);
	UNREFERENCED_PARAMETER(pSeriesStyle);

	m_pRelatedChart->OnDrawChartSeriesItem(pGM, rectLegendKey, pSeries, nDataPointIndex, CBCGPChartVisualObject::CE_MAIN_ELEMENT, FALSE, FALSE);
}
//****************************************************************************************
void CBCGPAreaChartImpl::OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
												const BCGPChartCellParams& params, CBCGPChartSeries* /*pSeries*/)
{
	m_pRelatedChart->OnDrawChartRect(pGM, rectLegendKey, &params.m_brFill, &params.m_lineStyle.m_brLineColor, 
		params.m_lineStyle.GetLineWidth(TRUE), &params.m_lineStyle.m_strokeStyle, FALSE, FALSE);
}
//****************************************************************************************
void CBCGPAreaChartImpl::OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
										  CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);

	if (pDataPoint == NULL)
	{
		return;
	}

	CBCGPRect rectBounds;
	pSeries->SetDataPointBoundingRect(nDataPointIndex, rectBounds);

	CBCGPBaseChartImpl::OnCalcBoundingRect(pDataPoint, rectDiagramArea, pSeries, nDataPointIndex);
	if (pSeries->GetDataPointCount() == 2)
	{
		CBCGPPoint pt = pSeries->GetDataPointScreenPoint(nDataPointIndex, 1);
		rectBounds = pSeries->GetDataPointBoundingRect(nDataPointIndex);

		rectBounds.right = pt.x;
		rectBounds.bottom = pt.y;
		pSeries->SetDataPointBoundingRect(nDataPointIndex, rectBounds);
	}
}

//****************************************************************************************
// 3D Area chart specific implementation
//****************************************************************************************
void CBCGPAreaChart3DImpl::OnDrawDiagram(CBCGPGraphicsManager* /*pGM*/, const CBCGPRect& /*rectDiagramArea*/, CBCGPChartSeries* /*pSeries*/)
{
	ASSERT_VALID(this);
	OnDrawDiagram3D();
}
//****************************************************************************************
void CBCGPAreaChart3DImpl::OnCalcScreenPositions(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	
	if (pSeries == NULL || m_pRelatedChart == NULL)
	{
		return;
	}
	
	ASSERT_VALID(pSeries);

	m_nFirstNonEmptyIndex = -1;
	m_nBeforeLastNonEmptyIndex = -1;
	m_nLastNonEmptyIndex = -1;
	
	for (int j = pSeries->GetMinDataPointIndex(); j <= pSeries->GetMaxDataPointIndex(); j++)
	{
		if (m_nFirstNonEmptyIndex == -1 && !pSeries->IsDataPointScreenPointsEmpty(j))
		{
			m_nFirstNonEmptyIndex = j;
			break;
		}
	}

	// also we need to find a non empty data point index that comes right before last non empty index
	// to properly identify the last cube
	for (int k = pSeries->GetMaxDataPointIndex(); k >= pSeries->GetMinDataPointIndex(); k--)
	{
		if (pSeries->IsDataPointScreenPointsEmpty(k))
		{
			continue;
		}

		if (m_nLastNonEmptyIndex != -1)
		{
			m_nBeforeLastNonEmptyIndex = k;
			break;
		}

		if (m_nLastNonEmptyIndex == -1)
		{
			m_nLastNonEmptyIndex = k;
		}
	}

	CBCGPBaseChartImpl::OnCalcScreenPositions(pGM, rectDiagramArea, pSeries);
}
//****************************************************************************************
void CBCGPAreaChart3DImpl::OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& /*rectDiagramArea*/, 
		CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);

	if (m_pRelatedChart == NULL || pSeries == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pRelatedChart);
	ASSERT_VALID(pSeries);

	pSeries->SetDataPointShape3D(NULL, nDataPointIndex);

	CBCGPChartDiagram3D* pDiagram3D = m_pRelatedChart->GetDiagram3D();
	if (pDiagram3D == NULL)
	{
		return;
	}

	CBCGPChartAxis* pXAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X);
	CBCGPChartAxis* pYAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_Y);
	CBCGPChartAxis* pZAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_Z);
	CBCGPChartAreaSeries* pAreaSeries = DYNAMIC_DOWNCAST(CBCGPChartAreaSeries, pSeries);

	int nDataPointCount = pSeries->GetDataPointCount();

	if (nDataPointIndex == m_nLastNonEmptyIndex)
	{
		CBCGPChartShape3D* pShapePrev = pSeries->GetDataPointShape3D(m_nBeforeLastNonEmptyIndex);

		if (pShapePrev != NULL)
		{
			pDataPoint->m_rectBounds = pShapePrev->GetSides().GetAt(CBCGPChartShape3DCube::CUBE_SIDE_RIGHT).m_rectMinMaxPoints; 

			if (pSeries->IsStakedSeries())
			{
				pDataPoint->m_rectBounds.top = pDataPoint->m_rectBounds.bottom = 
					pDataPoint->m_rectBounds.top + pDataPoint->m_rectBounds.Height() / 2;
			}
		}

		return;
	}

	if (pXAxis == NULL || pAreaSeries == NULL || 
		nDataPointIndex == pSeries->GetDataPointCount() - 1 && m_dblPrevIntersectX == DBL_MAX)
	{
		return;
	}

	BOOL bSetCube = TRUE;

	if (pSeries->IsDataPointScreenPointsEmpty(nDataPointIndex))
	{
		bSetCube = FALSE;
	}

	if (nDataPointIndex == 0)
	{
		m_dblPrevIntersectX = DBL_MAX;
	}

	ASSERT_VALID(pXAxis);

	double dblXUnitSize = pXAxis->GetAxisUnitSize() / 2;

	CBCGPPoint pt = pSeries->GetDataPointScreenPoint(nDataPointIndex, 0);
	CBCGPPoint ptNext; 

	if (nDataPointIndex == nDataPointCount - 1)
	{
		ptNext = pt;
	}
	else
	{
		int nNextIndex = nDataPointIndex + 1;
		if (pSeries->GetTreatNulls() != CBCGPChartSeries::TN_VALUE)
		{
			while (pSeries->IsDataPointScreenPointsEmpty(nNextIndex) && nNextIndex < nDataPointCount)
			{
				nNextIndex++;
			}

			if (nNextIndex == nDataPointCount)
			{
				nNextIndex--;
				bSetCube = FALSE;
			}
		}

		ptNext = pSeries->GetDataPointScreenPoint(nNextIndex, 0);
	}

	BOOL bIsEmpty = FALSE;
	CBCGPPoint ptOrigin = m_pRelatedChart->ScreenPointFromChartData(CBCGPChartData(pAreaSeries->GetAreaOrigin()), 0, bIsEmpty, pSeries);

	ASSERT_VALID(pZAxis);

	double dblZ = m_pRelatedChart->IsChart3DGrouped() ? 0 : pt.z;
	double dblDepth = CalcDepth3D(pZAxis->GetAxisUnitSize() / 2);

	m_pRelatedChart->SetBaseDepthPercent3D(dblXUnitSize);

	CBCGPPoint ptStackedLeft = pSeries->GetDataPointScreenPoint(nDataPointIndex, 1);
	CBCGPPoint ptStackedRight = nDataPointIndex < nDataPointCount - 1 ? 
		pSeries->GetDataPointScreenPoint(nDataPointIndex + 1, 1) : ptStackedLeft;

	CBCGPPoint ptLeftBottom = pSeries->IsStakedSeries() || pSeries->IsRangeSeries() ? ptStackedLeft : ptOrigin;
	CBCGPPoint ptRightBottom = pSeries->IsStakedSeries() || pSeries->IsRangeSeries() ? ptStackedRight : ptOrigin;

	CBCGPPoint ptCorners[4];

	if (!pXAxis->IsVertical())
	{
		ptLeftBottom.x = pt.x;
		ptRightBottom.x = ptNext.x;

		ptCorners[0] = ptLeftBottom;
		ptCorners[1] = ptRightBottom;
		ptCorners[2] = pt;
		ptCorners[3] = ptNext;
	}
	else
	{
		ptLeftBottom.y = pt.y;
		ptRightBottom.y = ptNext.y;

		ptCorners[0] = ptLeftBottom;
		ptCorners[1] = pt;
		ptCorners[2] = ptRightBottom;
		ptCorners[3] = ptNext;
	}
	
	if (ptCorners[0].y == ptCorners[3].y)
	{
		ptCorners[3].y += FLT_EPSILON;
	}

	if (ptCorners[0].y == ptCorners[2].y)
	{
		ptCorners[2].y += FLT_EPSILON;
	}

	CBCGPChartShape3DCube cube(ptCorners[0], ptCorners[1], ptCorners[2], ptCorners[3], dblZ, dblDepth);
	BOOL bReverseOrder = (pXAxis->m_bReverseOrder || pYAxis->m_bReverseOrder) && 
						 !(pXAxis->m_bReverseOrder && pYAxis->m_bReverseOrder);

	if (m_nFirstNonEmptyIndex != m_nBeforeLastNonEmptyIndex)
	{
		if (nDataPointIndex == m_nFirstNonEmptyIndex)
		{
			if (pXAxis->IsVertical())
			{
				if (bReverseOrder)
				{
					cube.SetSideIgnored(CBCGPChartShape3DCube::CUBE_SIDE_TOP, TRUE);
				}
				else
				{
					cube.SetSideIgnored(CBCGPChartShape3DCube::CUBE_SIDE_BOTTOM, TRUE);
				}
			}
			else
			{
				cube.SetSideIgnored(CBCGPChartShape3DCube::CUBE_SIDE_RIGHT, TRUE);
			}
			
		}
		else if (nDataPointIndex == m_nBeforeLastNonEmptyIndex)
		{
			if (pXAxis->IsVertical())
			{
				if (bReverseOrder)
				{
					cube.SetSideIgnored(CBCGPChartShape3DCube::CUBE_SIDE_BOTTOM, TRUE);
				}
				else
				{
					cube.SetSideIgnored(CBCGPChartShape3DCube::CUBE_SIDE_TOP, TRUE);
				}
			}
			else
			{
				cube.SetSideIgnored(CBCGPChartShape3DCube::CUBE_SIDE_LEFT, TRUE);
			}
			
		}
		else 
		{
			if (pXAxis->IsVertical())
			{
				cube.SetSideIgnored(CBCGPChartShape3DCube::CUBE_SIDE_TOP, TRUE);
				cube.SetSideIgnored(CBCGPChartShape3DCube::CUBE_SIDE_BOTTOM, TRUE);
			}
			else
			{
				cube.SetSideIgnored(CBCGPChartShape3DCube::CUBE_SIDE_LEFT, TRUE);
				cube.SetSideIgnored(CBCGPChartShape3DCube::CUBE_SIDE_RIGHT, TRUE);
			}
		}
	}

	if (pSeries->IsRangeSeries() || bReverseOrder)
	{
		cube.Flip(TRUE);
	}
	
 	pDiagram3D->TransformShape(cube);

	int nSide = CBCGPChartShape3DCube::CUBE_SIDE_LEFT;
	pDataPoint->m_rectBounds = cube.GetSides().GetAt(nSide).m_rectMinMaxPoints; 

	if (pSeries->IsStakedSeries())
	{
		pDataPoint->m_rectBounds.top = pDataPoint->m_rectBounds.bottom = 
			pDataPoint->m_rectBounds.top + pDataPoint->m_rectBounds.Height() / 2;
	}

	if (bSetCube)
	{
		cube.m_pSeries = pSeries;
		cube.m_nDataPointIndex = nDataPointIndex;
		pSeries->SetDataPointShape3D(cube, nDataPointIndex);
	}
}
//****************************************************************************************
// SURFACE chart specific implementation
//****************************************************************************************
void CBCGPSurfaceChart3DImpl::OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagramArea*/, CBCGPChartSeries* /*pSeries*/)
{
	ASSERT_VALID(this);

	if (m_pRelatedChart == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pRelatedChart);

	CBCGPChartDiagram3D* pDiagram3D = m_pRelatedChart->GetDiagram3D(); 
	CBCGPEngine3D* pEngine3D = m_pRelatedChart->GetEngine3D();

	if (pDiagram3D == NULL || pEngine3D == NULL)
	{
		return;
	}

	BOOL bUseTransparency = FALSE;
	int  nTriangleCount = 0;
	int i;

	for (i = 0; i < m_pRelatedChart->GetSeriesCount(); i++)
	{
		CBCGPChartSurfaceSeries* pSurfaceSeries = DYNAMIC_DOWNCAST(CBCGPChartSurfaceSeries, m_pRelatedChart->GetSeries(i));

		if (pSurfaceSeries == NULL || !pSurfaceSeries->m_bVisible)
		{
			continue;
		}

		if (pSurfaceSeries->GetSurfaceOpacity() < 1.)
		{
			bUseTransparency = TRUE;
		}

		nTriangleCount += (int)pSurfaceSeries->GetSurfaceTriangles().GetSize();
	}

	SortSurfaceTriangles(bUseTransparency, nTriangleCount);

	BOOL bIsGDI = pGM->GetType() == CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_GDI;
	BOOL bIsAAEnabled = pGM->IsAntialiasingEnabled();

	if (bIsAAEnabled && bIsGDI || !m_pRelatedChart->IsAntialiasingEnabled())
	{
		pGM->EnableAntialiasing(FALSE);
	}

	if (m_pRelatedChart->GetDiagram3D()->IsThickWallsAndFloor())
	{
		CBCGPChartDiagram3D::DrawWallOptions dwo = pDiagram3D->GetDrawWallOptions();

		if (pEngine3D->IsSoftwareRendering() && pDiagram3D->GetYRotation() < 0)
		{
			pDiagram3D->SetDrawWallOptions((CBCGPChartDiagram3D::DrawWallOptions)(dwo & ~CBCGPChartDiagram3D::DWO_FILL_FLOOR));
		}

		OnDrawDiagram3D();

		pDiagram3D->SetDrawWallOptions(dwo);
	}

	pEngine3D->m_bForceEnableDepthTest = TRUE;

	for (i = nTriangleCount - 1; i >= 0; i--)
	{
		CBCGPChartSurfaceTriangleID& triangleID = m_arTriangleID[i];
		OnDrawSurfaceTriangle(pEngine3D, triangleID, bIsGDI, TRUE);
	}

	if (!pEngine3D->IsSoftwareRendering())
	{
		for (i = nTriangleCount - 1; i >= 0; i--)
		{
			CBCGPChartSurfaceTriangleID& triangleID = m_arTriangleID[i];
			OnDrawSurfaceTriangle(pEngine3D, triangleID, bIsGDI, FALSE);
		}
	}

	pEngine3D->m_bForceEnableDepthTest = FALSE;

	if (bIsAAEnabled && m_pRelatedChart->IsAntialiasingEnabled())
	{
		pGM->EnableAntialiasing(TRUE);
	}
}
//****************************************************************************************
void CBCGPSurfaceChart3DImpl::CollectSides3D(CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lstInvisibleSides, CList<CBCGPChartSide3D*, CBCGPChartSide3D*>& lstVisibleSides)
{
	CBCGPChartDiagram3D* pDiagram3D = m_pRelatedChart->GetDiagram3D();

	if (pDiagram3D != NULL && pDiagram3D->IsThickWallsAndFloor())
	{
		pDiagram3D->CollectWallAndFloorSides(lstInvisibleSides, lstVisibleSides);
	}
}
//****************************************************************************************
void CBCGPSurfaceChart3DImpl::OnDrawSurfaceTriangle(CBCGPEngine3D* pGM, 
						CBCGPChartSurfaceTriangleID& triangleID, BOOL bIsGDI, BOOL bFill)
{
	ASSERT_VALID(this);

	if (triangleID.m_pSurfaceSeries == NULL)
	{
		return;
	}

	CBCGPChartSurfaceTriangleArray& arTriangles = (CBCGPChartSurfaceTriangleArray&) triangleID.m_pSurfaceSeries->GetSurfaceTriangles();
	CBCGPChartSurfaceTriangle& triangle = arTriangles[triangleID.m_nTriangleIndex];

	if (triangle.m_bIsEmpty)
	{
		return;
	}

	if (pGM->IsSoftwareRendering())
	{
		if (triangle.m_bDrawn)
		{
			return;
		}

		triangle.m_bDrawn = TRUE;

		if (triangleID.m_arIDIndexesBefore.GetSize() > 0) 
		{
			for (int nBefore = 0; nBefore < triangleID.m_arIDIndexesBefore.GetSize(); nBefore++)
			{
				int nIdx = triangleID.m_arIDIndexesBefore[nBefore];
				CBCGPChartSurfaceTriangleID& triangleIDBefore = m_arTriangleID[nIdx];
				OnDrawSurfaceTriangle(pGM, triangleIDBefore, bIsGDI, bFill);
			}
		}
	}

	CBCGPChartSurfaceSeries::FrameStyle frameStyle = triangleID.m_pSurfaceSeries->GetFrameStyle();
	static const double dblLinePrecision = 4 * FLT_EPSILON;
	
	int i;

	pGM->SetPolygonNormal(triangle.m_vNormal[0], triangle.m_vNormal[1], triangle.m_vNormal[2]);

	// fill triangles
	if (!triangleID.m_pSurfaceSeries->IsWireFrame() && (bFill || pGM->IsSoftwareRendering()))
	{
		for (i = 0; i < triangle.m_arLevels.GetSize(); i++)
		{
			const CBCGPChartSurfaceLevel& level = triangle.m_arLevels[i];

			if (triangle.m_bBack)
			{
				if (level.m_levelAttribs.m_pBrFillBack == NULL)
				{
					continue;
				}

				pGM->FillPolygon(level.m_arLevelPoints, *level.m_levelAttribs.m_pBrFillBack);
			}
			else
			{
				if (level.m_levelAttribs.m_pBrFill == NULL)
				{
					continue;
				}

				pGM->FillPolygon(level.m_arLevelPoints, *level.m_levelAttribs.m_pBrFill);
			}
		}
	}

	if (bFill && !pGM->IsSoftwareRendering())
	{
		return;
	}

	const CBCGPBrush& brFrame = triangleID.m_pSurfaceSeries->GetFrameColor();

	// draw frame
	for (i = 0; i < triangle.m_arLevels.GetSize(); i++)
	{
		const CBCGPChartSurfaceLevel& level = triangle.m_arLevels[i];

		const int nNumPoints = (int)level.m_arLevelPoints.GetSize();

		if (nNumPoints == 0)
		{
			continue;
		}

		CBCGPBrush* pBrLine = NULL;

		if (brFrame.IsEmpty())
		{
			pBrLine = triangle.m_bBack ? level.m_levelAttribs.m_pBrLineBack : level.m_levelAttribs.m_pBrLine;  
		}
		else
		{
			pBrLine = (CBCGPBrush*)&brFrame;
		}

		if (pBrLine == NULL)
		{
			continue;
		}

		for (int j = 0; j < nNumPoints; j++)
		{
			const CBCGPPoint& pt1 = level.m_arLevelPoints[j];
			const CBCGPPoint& pt2 = level.m_arLevelPoints[(j + 1) % nNumPoints];

			if (frameStyle == CBCGPChartSurfaceSeries::FS_NONE)
			{
				continue;
			}

			// for left triangle edge with index 0 divides a surface cell; for right - 2
			if ((triangle.m_bLeft && triangle.CheckPointsOnEdge(pt1, pt2, 0, dblLinePrecision)) || 
				(!triangle.m_bLeft && triangle.CheckPointsOnEdge(pt1, pt2, 2, dblLinePrecision)))
			{
				continue;
			}

			BOOL bPtOnEdge = TRUE;

			if (frameStyle != CBCGPChartSurfaceSeries::FS_CONTOUR_MESH)
			{
				// "external" edges - all except 2
				int ind1 = 0;
				int ind2 = 1;

				if (triangle.m_bLeft)
				{
					// "external" edges - all except 0
					ind1 = 1;
					ind2 = 2;
				}

				bPtOnEdge = triangle.CheckPointsOnEdge(pt1, pt2, ind1, dblLinePrecision) || 
							triangle.CheckPointsOnEdge(pt1, pt2, ind2, dblLinePrecision);

				if (frameStyle == CBCGPChartSurfaceSeries::FS_CONTOUR)
				{
					bPtOnEdge = !bPtOnEdge;
				}
			}

			if (bPtOnEdge)
			{
				pGM->DrawLine(pt1, pt2, *pBrLine, 1.0);
			}
		}
	}

//#define _BCGP_PRINT_TRIANGLE_INDEXES
#ifdef _BCGP_PRINT_TRIANGLE_INDEXES
	CBCGPTextFormat tf(_T("Arial"), 7);
	tf.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
	tf.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);

	CString str;
	str.Format(_T("%d"), triangle.m_nTriangleIndex);
	
	CBCGPPointsArray ar(3);
	ar[0] = triangle.m_arPointsTransformed[0];
	ar[1] = triangle.m_arPointsTransformed[1];
	ar[2] = triangle.m_arPointsTransformed[2];

	CBCGPRect r = ar.GetBoundsRect();
	r.Normalize();

	pGM->DrawText(str, r, tf, CBCGPBrush(RGB(0, 0, 0)));
#endif
}

//****************************************************************************************
int __cdecl SortTriangles(const void* pT1, const void* pT2)
{
	CBCGPChartSurfaceTriangleID* t1 = (CBCGPChartSurfaceTriangleID*) pT1;
	CBCGPChartSurfaceTriangleID* t2 = (CBCGPChartSurfaceTriangleID*) pT2;

	double dblOpacity1 = t1->m_pSurfaceSeries->GetSurfaceOpacity();
	double dblOpacity2 = t2->m_pSurfaceSeries->GetSurfaceOpacity();

	if (dblOpacity1 == 1. && dblOpacity2 < 1.)
	{
		return 1;
	}

	if (dblOpacity2 == 1. && dblOpacity1 < 1.)
	{
		return -1;
	}

	CBCGPChartSurfaceTriangleArray& arTriangles1 = (CBCGPChartSurfaceTriangleArray&) t1->m_pSurfaceSeries->GetSurfaceTriangles();
	CBCGPChartSurfaceTriangle& triangle1 = arTriangles1[t1->m_nTriangleIndex];

	CBCGPChartSurfaceTriangleArray& arTriangles2 = (CBCGPChartSurfaceTriangleArray&) t2->m_pSurfaceSeries->GetSurfaceTriangles();
	CBCGPChartSurfaceTriangle& triangle2 = arTriangles2[t2->m_nTriangleIndex];

	if (triangle1.m_bIsEmpty)
	{
		return 1;
	}


	if (triangle2.m_bIsEmpty)
	{
		return -1;
	}
	
	return (int) (bcg_sign(triangle1.m_dblMinZRotated - triangle2.m_dblMinZRotated));
}

//****************************************************************************************
void CBCGPSurfaceChart3DImpl::SortSurfaceTriangles(BOOL bUseTransparency, int nTriangleCount)
{
	ASSERT_VALID(this);

	if (m_pRelatedChart == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pRelatedChart);

	CBCGPChartDiagram3D* pDiagram3D = m_pRelatedChart->GetDiagram3D();

	if (pDiagram3D == NULL)
	{
		return;
	}

	CBCGPEngine3D* pEngine3D = m_pRelatedChart->GetEngine3D();

	if (pEngine3D == NULL)
	{
		return;
	}

	double dblXRotation = pDiagram3D->GetXRotation();
	double dblYRotation = pDiagram3D->GetYRotation();

	BOOL bRotationChanged = m_dblLastXRotation != dblXRotation || m_dblLastYRotation != dblYRotation || 
		m_arTriangleID.GetSize() == 0 || pDiagram3D->IsSortNeeded();

	if (!bRotationChanged)
	{
		for (int s = 0; s < m_arTriangleID.GetSize(); s++)
		{
			CBCGPChartSurfaceTriangleID& triangleID = m_arTriangleID[s];
			CBCGPChartSurfaceTriangleArray& arTriangles = (CBCGPChartSurfaceTriangleArray&) triangleID.m_pSurfaceSeries->GetSurfaceTriangles();

			CBCGPChartSurfaceTriangle& triangle = arTriangles [triangleID.m_nTriangleIndex];
			triangle.m_bDrawn = FALSE;
		}

		return;
	}

	pDiagram3D->SetSortNeeded(FALSE);

	int nTrinagleIDIndex = 0;

	if (nTriangleCount != m_arTriangleID.GetSize())
	{
		m_arTriangleID.RemoveAll();
		m_arTriangleID.SetSize(nTriangleCount);
	}

	int i;
	for (i = 0; i < m_pRelatedChart->GetSeriesCount(); i++)
	{
		CBCGPChartSurfaceSeries* pSurfaceSeries = DYNAMIC_DOWNCAST(CBCGPChartSurfaceSeries, m_pRelatedChart->GetSeries(i));

		if (pSurfaceSeries == NULL || !pSurfaceSeries->m_bVisible)
		{
			continue;
		}

		const CBCGPChartSurfaceTriangleArray& triangles = pSurfaceSeries->GetSurfaceTriangles();
		
		for (int j = 0; j < triangles.GetSize(); j++, nTrinagleIDIndex++)
		{
			CBCGPChartSurfaceTriangleID& triangleID = m_arTriangleID[nTrinagleIDIndex];
			triangleID.m_nTriangleIndex = j;
			triangleID.m_pSurfaceSeries = pSurfaceSeries;
			triangleID.m_arIDIndexesBefore.RemoveAll();
		}
	}

	int nSize = nTriangleCount;
	BOOL bSoftRendering = pEngine3D->IsSoftwareRendering();

	if (!bSoftRendering)
	{
		if (bUseTransparency)
		{
			qsort(m_arTriangleID.GetData(), nSize, sizeof(CBCGPChartSurfaceTriangleID), SortTriangles);
		}
		return;
	}

	static const float fltPrecision = 2 * FLT_EPSILON;

	m_dblLastXRotation = dblXRotation;
	m_dblLastYRotation = dblYRotation;

	CBCGPPoint arRes[10]; // sure no more than 10 points
	int nResCount = 0;

	for (i = 0; i < nSize; i++)
	{
		CBCGPChartSurfaceTriangleID& triangleID = m_arTriangleID[i];

		CBCGPChartSurfaceTriangleArray& arTriangles = (CBCGPChartSurfaceTriangleArray&) triangleID.m_pSurfaceSeries->GetSurfaceTriangles();
		CBCGPChartSurfaceTriangle& triangle = arTriangles[triangleID.m_nTriangleIndex];

		if (triangle.m_bIsEmpty)
		{
			continue;
		}

		CBCGPRect rectRotated = triangle.m_rectBoundsRotated;

		for (int j = i + 1; j < nSize; j++)
		{
			CBCGPChartSurfaceTriangleID& triangleIDNext = m_arTriangleID[j];
			CBCGPChartSurfaceTriangleArray& arTrianglesNext = (CBCGPChartSurfaceTriangleArray&) triangleIDNext.m_pSurfaceSeries->GetSurfaceTriangles();
			CBCGPChartSurfaceTriangle& triangleNext = arTrianglesNext[triangleIDNext.m_nTriangleIndex];

			if (triangleNext.m_bIsEmpty)
			{
				continue;
			}

			if (triangleNext.m_rectBoundsRotated.left > rectRotated.right || 
				triangleNext.m_rectBoundsRotated.right < rectRotated.left || 
				triangleNext.m_rectBoundsRotated.bottom < rectRotated.top ||
				triangleNext.m_rectBoundsRotated.top > rectRotated.bottom)
			{
				continue;
			}

			if (triangle.m_dblMaxZRotated < triangleNext.m_dblMinZRotated)
			{
				triangleID.m_arIDIndexesBefore.Add(j);
				continue;
			}

			if (triangle.m_dblMinZRotated > triangleNext.m_dblMaxZRotated)
			{
				triangleIDNext.m_arIDIndexesBefore.Add(i);
				continue;
			}

			nResCount = 0;
			if (BCGPIntersectTriangle2D(triangle.m_arPointsRotated, triangleNext.m_arPointsRotated, arRes, nResCount))
			{
				CBCGPPoint ptCenterCollision;

				if (nResCount < 3)
				{
					continue;
				}

				ASSERT(nResCount < 10);

				for (int n = 0; n < nResCount; n++)
				{
					ptCenterCollision += arRes[n];
				}

				ptCenterCollision /= nResCount;
					
				float zTriangle = (float)triangle.CalcZ(ptCenterCollision);
				float zTriangleNext = (float)triangleNext.CalcZ(ptCenterCollision);

				if (fabs(zTriangle - zTriangleNext) < fltPrecision)
				{
					if (triangle.m_bBack == triangleNext.m_bBack)
					{
						continue;
					}
					else
					{
						if (triangleNext.m_bBack)
						{
							triangleID.m_arIDIndexesBefore.Add(j);
						}
						else
						{
							triangleIDNext.m_arIDIndexesBefore.Add(i);
						}
						continue;
					}
				}

				if (zTriangle < zTriangleNext)
				{
					triangleID.m_arIDIndexesBefore.Add(j);
				}
				else
				{
					triangleIDNext.m_arIDIndexesBefore.Add(i);
				}
			}
		}
	}
}
//****************************************************************************************
void CBCGPSurfaceChart3DImpl::OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	CBCGPChartSurfaceSeries* pSurfaceSeries = DYNAMIC_DOWNCAST(CBCGPChartSurfaceSeries, pSeries);

	if (pSurfaceSeries == NULL || pSeriesStyle == NULL)
	{
		return;
	}

	ASSERT_VALID(pSurfaceSeries);

	if (nDataPointIndex == -1)
	{
		// the case when legend is included in the legend as a series name and key, not as a map.
		nDataPointIndex = 0;
	}

	CBCGPBrush* pBrKeyFill = pSurfaceSeries->GetLevelFillBrush(nDataPointIndex);
	CBCGPBrush* pBrKeyLine = pSurfaceSeries->GetLevelLineBrush(nDataPointIndex);

	if (pBrKeyFill == NULL || pBrKeyLine == NULL)
	{
		return;
	}

	CBCGPRect rectKey = rectLegendKey;
	rectKey.InflateRect(0., 1.);

	pGM->FillRectangle(rectKey, *pBrKeyFill);

	if (!pSurfaceSeries->IsContinuousLegendKey())
	{
		pGM->DrawRectangle(rectKey, *pBrKeyLine, 
			pSeriesStyle->m_legendLabelFormat.m_outlineFormat.GetLineWidth(TRUE), 
			&pSeriesStyle->m_legendLabelFormat.m_outlineFormat.m_strokeStyle);
	}
}
//****************************************************************************************
void CBCGPSurfaceChart3DImpl::OnCalcScreenPositions(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	if (pSeries == NULL)
	{
		return;
	}

	ASSERT_VALID(pSeries);

	if (m_pRelatedChart == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pRelatedChart);

	CBCGPChartDiagram3D* pDiagram3D = m_pRelatedChart->GetDiagram3D();

	if (pDiagram3D == NULL)
	{
		return;
	}

	CBCGPChartSurfaceSeries* pSurfaceSeries = DYNAMIC_DOWNCAST(CBCGPChartSurfaceSeries, pSeries);

	if (pSurfaceSeries == NULL)
	{
		return;
	}

	CBCGPChartAxis* pXAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X);
	if (pXAxis == NULL)
	{
		return;
	}

	m_dblXAxisUnitSize = pXAxis->GetAxisUnitSize();

	pDiagram3D->SetBaseDepthPercent(1.);
	pSeries->OnCalcScreenPoints(pGM, rectDiagramArea);
}
//****************************************************************************************
void CBCGPSurfaceChart3DImpl::OnCalcBoundingRect(CBCGPChartDataPoint* /*pDataPoint*/, const CBCGPRect& /*rectDiagramArea*/, 
		CBCGPChartSeries* /*pSeries*/, int /*nDataPointIndex*/)
{
	ASSERT_VALID(this);
	ASSERT(FALSE);
}
//****************************************************************************************
// COLUMN/BAR chart specific implementation
//****************************************************************************************
void CBCGPBarChartImpl::OnCalcDataPointLabelRect(CBCGPGraphicsManager* pGM, CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
							  CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);

	if (pDataPoint == NULL || pSeries == NULL || pSeries->IsDataPointScreenPointsEmpty(nDataPointIndex))
	{
		return;
	}

	BCGPChartFormatSeries* pFormatSeries = NULL;
	CBCGPSize szDataLabelSize;
	CBCGPPoint ptMarker;
	CBCGPRect rectBounds;

	if (!OnPrepareDataToCalcDataPointLabelRect(pGM, rectDiagramArea, pDataPoint, pSeries, nDataPointIndex, 
		&pFormatSeries, szDataLabelSize, ptMarker, rectBounds))
	{
		return;
	}

	// Get X axis
	CBCGPChartAxis* pXAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X);

	if (pXAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(pXAxis);

	CBCGPRect rectDataLabel;
	CBCGPPoint ptCenter = rectBounds.CenterPoint();

	BOOL bBaseAtTop = pXAxis->IsVertical() ? ptMarker.x == rectBounds.left : ptMarker.y == rectBounds.bottom;

	if (pXAxis->IsVertical() && rectBounds.Width() == 0 || !pXAxis->IsVertical() && rectBounds.Height() == 0)
	{
		bBaseAtTop = FALSE;
	}

	double dblDistance = pSeries->GetDataPointLabelDistance(nDataPointIndex);

	if (pFormatSeries->m_dataLabelFormat.m_options.m_position == BCGPChartDataLabelOptions::LP_INSIDE_BASE ||
		pFormatSeries->m_dataLabelFormat.m_options.m_position == BCGPChartDataLabelOptions::LP_INSIDE_END)
	{
		dblDistance = pXAxis->IsVertical() ? dblDistance * rectBounds.Width() / 2. / 100. : 
											dblDistance = dblDistance * rectBounds.Height() / 2. / 100.;
	}

	BCGPChartDataLabelOptions::LabelPosition pos = pFormatSeries->m_dataLabelFormat.m_options.m_position;

	if (pos == BCGPChartDataLabelOptions::LP_DEFAULT_POS)
	{
		pos = pSeries->IsStakedSeries() ? (m_pRelatedChart->IsChart3D() ? BCGPChartDataLabelOptions::LP_CENTER : BCGPChartDataLabelOptions::LP_INSIDE_BASE) : 
					BCGPChartDataLabelOptions::LP_OUTSIDE_END;
	}

	if (pXAxis->IsVertical() && m_pRelatedChart->IsChart3D())
	{
		pos = BCGPChartDataLabelOptions::LP_CENTER;
	}

	switch(pos)
	{
	case BCGPChartDataLabelOptions::LP_CENTER:
		rectDataLabel.SetRect(ptCenter.x - szDataLabelSize.cx / 2, 
								ptCenter.y - szDataLabelSize.cy / 2, 
								ptCenter.x + szDataLabelSize.cx / 2, 
								ptCenter.y + szDataLabelSize.cy / 2);
		ptMarker = rectDataLabel.CenterPoint();
		break;
	case BCGPChartDataLabelOptions::LP_INSIDE_BASE:
		if (pXAxis->IsVertical())
		{
			rectDataLabel.top = ptCenter.y - szDataLabelSize.cy / 2;
			rectDataLabel.bottom = rectDataLabel.top + szDataLabelSize.cy;

			if (bBaseAtTop)
			{
				rectDataLabel.right = rectBounds.right - dblDistance;
				rectDataLabel.left = rectDataLabel.right - szDataLabelSize.cx;	
			}
			else
			{
				rectDataLabel.left = rectBounds.left + dblDistance;
				rectDataLabel.right = rectDataLabel.left + szDataLabelSize.cx;
			}
		}
		else
		{
			rectDataLabel.left = ptCenter.x - szDataLabelSize.cx / 2;
			rectDataLabel.right = rectDataLabel.left + szDataLabelSize.cx;

			if (bBaseAtTop)
			{
				rectDataLabel.top = rectBounds.top + dblDistance;
				rectDataLabel.bottom = rectDataLabel.top + szDataLabelSize.cy;
			}
			else
			{
				rectDataLabel.bottom = rectBounds.bottom - dblDistance;
				rectDataLabel.top = rectDataLabel.bottom - szDataLabelSize.cy;
			}
		}
		AlignRectToArea(rectBounds, rectDataLabel, !pXAxis->IsVertical(), pXAxis->IsVertical());
		ptMarker = rectDataLabel.CenterPoint();
		break;

	case BCGPChartDataLabelOptions::LP_INSIDE_END:
		if (pXAxis->IsVertical())
		{
			rectDataLabel.top = ptCenter.y - szDataLabelSize.cy / 2;
			rectDataLabel.bottom = rectDataLabel.top + szDataLabelSize.cy;

			if (bBaseAtTop)
			{
				rectDataLabel.left = rectBounds.left + dblDistance;
				rectDataLabel.right = rectDataLabel.left + szDataLabelSize.cx;
			}
			else
			{
				rectDataLabel.right = rectBounds.right - dblDistance;
				rectDataLabel.left = rectDataLabel.right - szDataLabelSize.cx;
			}
		}
		else
		{
			rectDataLabel.left = ptCenter.x - szDataLabelSize.cx / 2;
			rectDataLabel.right = rectDataLabel.left + szDataLabelSize.cx;

			if (bBaseAtTop)
			{
				rectDataLabel.bottom = rectBounds.bottom - dblDistance;
				rectDataLabel.top = rectDataLabel.bottom - szDataLabelSize.cy;
			}
			else
			{
				rectDataLabel.top = rectBounds.top + dblDistance;
				rectDataLabel.bottom = rectDataLabel.top + szDataLabelSize.cy;
			}
		}

		AlignRectToArea(rectBounds, rectDataLabel, !pXAxis->IsVertical(), pXAxis->IsVertical());
		ptMarker = rectDataLabel.CenterPoint();
		break;
	case BCGPChartDataLabelOptions::LP_OUTSIDE_END:
		if (pXAxis->IsVertical())
		{
			if (m_pRelatedChart->IsChart3D())
			{
				rectDataLabel.top = ptMarker.y - szDataLabelSize.cy / 2;
			}
			else
			{
				rectDataLabel.top = ptCenter.y - szDataLabelSize.cy / 2;
			}

			rectDataLabel.bottom = rectDataLabel.top + szDataLabelSize.cy;
			
			if (!m_pRelatedChart->IsChart3D())
			{
				if (bBaseAtTop)
				{
					rectDataLabel.right = rectBounds.left - dblDistance;
					rectDataLabel.left = rectDataLabel.right - szDataLabelSize.cx;
				}
				else
				{
					rectDataLabel.left = rectBounds.right + dblDistance;
					rectDataLabel.right = rectDataLabel.left + szDataLabelSize.cx;
				}
			}
			else
			{
				rectDataLabel.left = ptMarker.x + dblDistance;
				rectDataLabel.right = rectDataLabel.left + szDataLabelSize.cx;
			}
		}
		else
		{
			rectDataLabel.left = ptCenter.x - szDataLabelSize.cx / 2;
			rectDataLabel.right = rectDataLabel.left + szDataLabelSize.cx;

			if (bBaseAtTop)
			{
				rectDataLabel.top = rectBounds.bottom + dblDistance;
				rectDataLabel.bottom = rectDataLabel.top + szDataLabelSize.cy;
			}
			else
			{
				rectDataLabel.bottom = rectBounds.top - dblDistance;
				rectDataLabel.top = rectDataLabel.bottom - szDataLabelSize.cy;
			}
		}
		break;

	default:
		CBCGPBaseChartImpl::OnCalcDataPointLabelRect(pGM, pDataPoint, rectDiagramArea, pSeries, nDataPointIndex);
		return;
	}

	SetDataPointLabelRectAndDropLine(pSeries, nDataPointIndex, pFormatSeries, rectDataLabel, ptMarker);		
}
//****************************************************************************************
void CBCGPBarChartImpl::OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pRelatedChart);
	ASSERT_VALID(pSeries);

	UNREFERENCED_PARAMETER(rectDiagramArea);

	if (m_pRelatedChart == NULL)
	{
		return;
	}

	for (int i = pSeries->GetMinDataPointIndex(); i <= pSeries->GetMaxDataPointIndex(); i++)
	{
		CBCGPRect rectBounds = pSeries->GetDataPointBoundingRect(i);

		if (!rectBounds.IsRectNull())
		{
			m_pRelatedChart->OnDrawChartSeriesItem(pGM, rectBounds, pSeries, i, CBCGPChartVisualObject::CE_MAIN_ELEMENT, FALSE, FALSE);
		}
	}
}
//****************************************************************************************
void CBCGPBarChartImpl::OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
						 const BCGPChartFormatSeries* /*pSeriesStyle*/, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);

	UNREFERENCED_PARAMETER(nDataPointIndex);

	m_pRelatedChart->OnDrawChartSeriesItem(pGM, rectLegendKey, pSeries, nDataPointIndex, CBCGPChartVisualObject::CE_MAIN_ELEMENT, FALSE, FALSE);
}
//****************************************************************************************
void CBCGPBarChartImpl::OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
												const BCGPChartCellParams& params, CBCGPChartSeries* /*pSeries*/)
{
	m_pRelatedChart->OnDrawChartRect(pGM, rectLegendKey, &params.m_brFill, &params.m_lineStyle.m_brLineColor, 
		params.m_lineStyle.GetLineWidth(TRUE), &params.m_lineStyle.m_strokeStyle, FALSE, FALSE);
}
//****************************************************************************************
CBCGPPoint CBCGPBarChartImpl::OnGetMarkerPoint(const CBCGPChartDataPoint* pDataPoint, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pRelatedChart);
	ASSERT_VALID(pSeries);

	UNREFERENCED_PARAMETER(nDataPointIndex);

	if (pDataPoint == NULL)
	{
		return CBCGPPoint();
	}

	CBCGPRect rectBounds = pSeries->GetDataPointBoundingRect(nDataPointIndex);
	CBCGPChartAxis* pXAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X);

	if (pXAxis == NULL)
	{
		return CBCGPPoint();
	}

	ASSERT_VALID(pXAxis);

	if (pDataPoint->GetComponentValue(CBCGPChartData::CI_Y) < 0)
	{
		if (pXAxis->IsVertical())
		{
			return CBCGPPoint (rectBounds.left, rectBounds.CenterPoint().y);
		}
		return CBCGPPoint (rectBounds.CenterPoint().x, rectBounds.bottom);
	}

	if (pXAxis->IsVertical())
	{
		return CBCGPPoint (rectBounds.right, rectBounds.CenterPoint().y);
	}

	return CBCGPPoint (rectBounds.CenterPoint().x, rectBounds.top);
}
//****************************************************************************************
void CBCGPBarChartImpl::OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
									CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pRelatedChart);
	ASSERT_VALID(pSeries);

	UNREFERENCED_PARAMETER(rectDiagramArea);

	CBCGPChartBarSeries* pBarSeries = DYNAMIC_DOWNCAST(CBCGPChartBarSeries, pSeries);

	if (pDataPoint == NULL || pBarSeries == NULL)
	{
		return;
	}

	CBCGPRect rectBounds;
	pSeries->SetDataPointBoundingRect(nDataPointIndex, rectBounds);

	if (m_pRelatedChart == NULL || m_pRelatedChart->GetVisibleSeriesCount() == 0 ||
		pSeries->IsDataPointScreenPointsEmpty(nDataPointIndex))
	{
		return;
	}

	// Get X axis
	CBCGPChartAxis* pXAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X);

	if (pXAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(pXAxis);

	BOOL bIsStacked = pSeries->IsStakedSeries();
	BOOL bIsRange = pSeries->IsRangeSeries();

	double dblXUnitSize = pXAxis->GetAxisUnitSize() / pXAxis->GetMajorUnit();

	if (pXAxis->IsFixedIntervalWidth())
	{
		dblXUnitSize = pXAxis->GetFixedIntervalWidth() / pXAxis->GetValuesPerInterval();
	}

	double dblZeroLinePos = pXAxis->GetNonIgnoredCrossValue();

	if (pXAxis->GetPerpendecularAxis() != NULL)
	{
		dblZeroLinePos = pXAxis->GetPerpendecularAxis()->PointFromValue(dblZeroLinePos, TRUE);
	}


	CBCGPPoint ptBottom;
	
	if (pXAxis->IsVertical())
	{
		ptBottom.SetPoint(dblZeroLinePos, 0);
	}
	else
	{
		ptBottom.SetPoint(0, dblZeroLinePos);
	}
	
	if ((bIsStacked || bIsRange) && pSeries->GetDataPointScreenPointCount(nDataPointIndex) > 1)
	{
		CBCGPPoint pt = pSeries->GetDataPointScreenPoint(nDataPointIndex, 1);
		ptBottom = pt;
	}

	BOOL bIsGrouped = m_pRelatedChart->IsChart3DGrouped();

	int nSeriesIndex = pBarSeries->GetOrderIndex();
	int nVisibleSeriesCount = pBarSeries->GetSeriesCountOnAxis(); 

	if (pBarSeries->IsCustomSize() || pBarSeries->IsCustomOffset() || !bIsGrouped)
	{
		nVisibleSeriesCount = 1;
	}

	double dblBarWidth = dblXUnitSize * 100 / (nVisibleSeriesCount * 100 + pBarSeries->GetColumnDistancePercent());
	double dblOverlapDiff = dblBarWidth * (pBarSeries->GetColumnOverlapPercent() / 100.) / 2;
	
	if (pBarSeries->IsCustomSize())
	{
		nVisibleSeriesCount = 1;
		dblBarWidth = dblXUnitSize * pBarSeries->GetCustomSizePercent() / 100.;
		nSeriesIndex = 0;
		bIsStacked = FALSE;
		bIsRange = FALSE;
	}
	
	CBCGPPoint point = pSeries->GetDataPointScreenPoint(nDataPointIndex, 0);
	CBCGPPoint ptCenter = point;

	double nAllBarWidth = dblBarWidth * nVisibleSeriesCount;

	if (pBarSeries->IsCustomOffset())
	{
		if (pXAxis->IsVertical())
		{
			double dblBarOffset = dblXUnitSize / 2 * pBarSeries->GetCustomOffsetPercent() / 100. + dblBarWidth / 2;
			point.y -= dblBarOffset;
		}
		else
		{
			double dblBarOffset = dblXUnitSize / 2 * pBarSeries->GetCustomOffsetPercent() / 100. - dblBarWidth / 2;
			point.x += dblBarOffset;
		}
		
	}
	else if ((bIsStacked || bIsRange) && pBarSeries->GetColumnOverlapPercent() == 100 || !bIsGrouped)
	{
		nVisibleSeriesCount = 1;
		dblBarWidth = dblXUnitSize / (nVisibleSeriesCount + (pBarSeries->GetColumnDistancePercent() / 100.));
	}
	else 
	{
		double dblBarOffset = dblBarWidth * (nSeriesIndex) - nAllBarWidth / 2;
		pXAxis->IsVertical() ? point.y += dblBarOffset : point.x += dblBarOffset;
	}


	if (pXAxis->IsComponentXSet())
	{
		if (!pXAxis->IsVertical())
		{
			rectBounds.SetRect(point.x - dblBarWidth, 
				ptBottom.y > point.y ? ptBottom.y : point.y, 
				point.x, 
				ptBottom.y > point.y ? point.y : ptBottom.y);
		}
		else
		{
			rectBounds.SetRect(ptBottom.x < point.x ? point.x : ptBottom.x, 
				point.y - dblBarWidth, 
				ptBottom.x < point.x ? ptBottom.x : point.x,
				point.y);
		}
	}
	else
	{
		if (!pXAxis->IsVertical())
		{
			rectBounds.SetRect(point.x, 
				ptBottom.y > point.y ? ptBottom.y : point.y, 
				point.x + dblBarWidth, 
				ptBottom.y > point.y ? point.y : ptBottom.y);
		}
		else
		{
			rectBounds.SetRect(ptBottom.x < point.x ? point.x : ptBottom.x, 
				point.y, 
				ptBottom.x < point.x ? ptBottom.x : point.x,
				point.y + dblBarWidth);
		}
	}	
	rectBounds.Normalize();

	if (pBarSeries->IsRangeSeries() && !m_pRelatedChart->IsChart3D())
	{
		if (pXAxis->IsVertical() && rectBounds.left == rectBounds.right && 
			rectBounds.left != 0.)
		{
			rectBounds.right += 1.;
		}
		else if (!pXAxis->IsVertical() && rectBounds.top == rectBounds.bottom && 
			rectBounds.bottom != 0.)
		{
			rectBounds.top -= 1.;
		}
	}
	
	if ((bIsStacked || bIsRange) && pBarSeries->GetColumnOverlapPercent() == 100 && 
		!pBarSeries->IsCustomOffset() || !bIsGrouped)
	{
		if (pXAxis->IsVertical())
		{
			rectBounds.OffsetRect(0, -dblBarWidth / 2);
		}
		else
		{
			rectBounds.OffsetRect(-dblBarWidth / 2, 0);
		}
	}
	else if (!pBarSeries->IsCustomOffset() && nVisibleSeriesCount > 1)
	{
		if (!pXAxis->IsVertical())
		{
			if (nSeriesIndex == 0)
			{
				rectBounds.right += dblOverlapDiff;
			}
			else if (nSeriesIndex == nVisibleSeriesCount - 1)
			{
				rectBounds.left -= dblOverlapDiff;
			}
			else
			{
				rectBounds.InflateRect(dblOverlapDiff / 2, 0);
			}
		}
		else
		{
			if (nSeriesIndex == 0)
			{
				rectBounds.bottom += dblOverlapDiff;
			}
			else if (nSeriesIndex == nVisibleSeriesCount - 1)
			{
				rectBounds.top -= dblOverlapDiff;
			}
			else
			{
				rectBounds.InflateRect(0, dblOverlapDiff / 2);
			}
		}
	}

	if (dblOverlapDiff > 0 && !bIsStacked)
	{
		
	}

	if (rectBounds.Width() != 0 && rectBounds.Height() != 0 && !m_pRelatedChart->IsChart3D())
	{
		rectBounds.IntersectRect(rectDiagramArea);

		double dblValue = pDataPoint->GetComponentValue(CBCGPChartData::CI_Y);
		
		if (pXAxis->IsVertical())
		{
			if (rectBounds.right == dblZeroLinePos && dblValue < 0)
			{
				rectBounds.right -= 1.;
			}
			else if (rectBounds.left == dblZeroLinePos && dblValue > 0)
			{
				rectBounds.left += 1.;
			}
		}
		else
		{
			if (rectBounds.bottom == dblZeroLinePos && dblValue > 0)
			{
				rectBounds.bottom -= 1.;
			}
			else if (rectBounds.top == dblZeroLinePos && dblValue < 0)
			{
				rectBounds.top += 1.;
			}
		}
	}

	pSeries->SetDataPointBoundingRect(nDataPointIndex, rectBounds);
}
//****************************************************************************************
CBCGPRect CBCGPBarChartImpl::BoundingRectFromGapAndBarWidth(CBCGPPoint ptBarScreenPoint, int nGapPercent, 
					double dblBarWidth,	int nSeriesIndex, int nVisibleSeriesCount, double dblZeroLinePos, 
					BOOL bIsVertical)
{
	ASSERT_VALID(this);

	UNREFERENCED_PARAMETER(nGapPercent);

	CBCGPRect rectFinal;
	rectFinal.SetRectEmpty();

	double nAllBarWidth = dblBarWidth * nVisibleSeriesCount;

	if (bIsVertical)
	{
		double dblBarOffset =  dblBarWidth * (nSeriesIndex) - nAllBarWidth / 2;
		ptBarScreenPoint.y += dblBarOffset;
	}
	else
	{
		double dblBarOffset = dblBarWidth * (nSeriesIndex) - nAllBarWidth / 2;
		ptBarScreenPoint.x += dblBarOffset ;
	}

	if (bIsVertical)
	{
		rectFinal.SetRect(dblZeroLinePos < ptBarScreenPoint.x ? ptBarScreenPoint.x : dblZeroLinePos, 
			ptBarScreenPoint.y, 
			dblZeroLinePos < ptBarScreenPoint.x ? dblZeroLinePos : ptBarScreenPoint.x,
			ptBarScreenPoint.y + dblBarWidth);
	}
	else
	{
		rectFinal.SetRect(ptBarScreenPoint.x, 
			dblZeroLinePos > ptBarScreenPoint.y ? dblZeroLinePos : ptBarScreenPoint.y, 
			ptBarScreenPoint.x + dblBarWidth, 
			dblZeroLinePos > ptBarScreenPoint.y ? ptBarScreenPoint.y : dblZeroLinePos);		
	}

	rectFinal.Normalize();

	return rectFinal;
}

//****************************************************************************************
// 3D Bar Chart chart specific implementation
//****************************************************************************************
void CBCGPBarChart3DImpl::OnDrawDiagram(CBCGPGraphicsManager* /*pGM*/, const CBCGPRect& /*rectDiagramArea*/, CBCGPChartSeries* /*pSeries*/)
{
	ASSERT_VALID(this);
	OnDrawDiagram3D();
}
//****************************************************************************************
void CBCGPBarChart3DImpl::OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
		CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	CBCGPBarChartImpl::OnCalcBoundingRect(pDataPoint, rectDiagramArea, pSeries, nDataPointIndex);

	CBCGPChartDiagram3D* pDiagram3D = m_pRelatedChart->GetDiagram3D();

	if (pDiagram3D == NULL || pSeries == NULL)
	{
		return;
	}

	pSeries->SetDataPointShape3D(NULL, nDataPointIndex);

	if (pSeries->IsDataPointScreenPointsEmpty(nDataPointIndex))
	{
		return;
	}

	CBCGPChartAxis* pXAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X);

	double dblXUnitSize = pXAxis->GetAxisUnitSize() / 2;
	double dblFrontDistance = pDiagram3D->GetFrontDistancePercent();

	CBCGPPoint pt = pSeries->GetDataPointScreenPoint(nDataPointIndex, 0);

	double dblZ = 0;
	double dblBaseValue = pDiagram3D->IsXHorizontal() ? pDataPoint->m_rectBounds.Width() : pDataPoint->m_rectBounds.Height();

	if (pDiagram3D->GetBaseDepthPercent() == 0.)
	{
		if (!m_pRelatedChart->IsChart3DGrouped())
		{
			m_pRelatedChart->SetBaseDepthPercent3D(dblXUnitSize);
		}
		else
		{
			pDiagram3D->SetBaseDepthPercent(0.5 * (dblBaseValue + dblBaseValue * dblFrontDistance));
		}
	}

	if (!m_pRelatedChart->IsChart3DGrouped())
	{
		dblZ = pt.z;
		dblBaseValue = dblXUnitSize;

		if (dblFrontDistance >= 1.)
		{
			dblBaseValue /= dblFrontDistance;
		}
		else
		{
			dblBaseValue = 2 * dblBaseValue * (1. - dblFrontDistance);
		}
	}

	CBCGPChartShape3DCube cube(pDataPoint->m_rectBounds, dblZ, dblBaseValue);
	pDiagram3D->TransformShape(cube);
	cube.m_pSeries = pSeries;
	cube.m_nDataPointIndex = nDataPointIndex;
	pSeries->SetDataPointShape3D(cube, nDataPointIndex);

	double dblXRotation = pDiagram3D->GetXRotation();

	int nSide = (dblXRotation <= 90 || dblXRotation >= 270)  ? CBCGPChartShape3DCube::CUBE_SIDE_FRONT : 
																CBCGPChartShape3DCube::CUBE_SIDE_BACK;

	pDataPoint->m_rectBounds = cube.GetSides().GetAt(nSide).m_rectMinMaxPoints; 
	pDataPoint->m_rectBounds.Normalize();
}
//****************************************************************************************
CBCGPPoint CBCGPBarChart3DImpl::OnGetMarkerPoint(const CBCGPChartDataPoint* /*pDataPoint*/, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);

	if (pSeries == NULL || m_pRelatedChart == NULL)
	{
		return CBCGPPoint();
	}

	ASSERT_VALID(pSeries);
	ASSERT_VALID(m_pRelatedChart);

	CBCGPChartDiagram3D* pDiagram3D = m_pRelatedChart->GetDiagram3D();

	if (pDiagram3D == NULL)
	{
		return CBCGPPoint();
	}

	CBCGPPoint ptMarker = pSeries->GetDataPointScreenPoint(nDataPointIndex, 0);
	return pDiagram3D->TransformPoint(ptMarker);
}
//****************************************************************************************
// Histogram chart specific implementation
//****************************************************************************************
void CBCGPHistogramChartImpl::OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pRelatedChart);
	ASSERT_VALID(pSeries);
	
	UNREFERENCED_PARAMETER(rectDiagramArea);
	
	if (m_pRelatedChart == NULL)
	{
		return;
	}
	
	for (int i = pSeries->GetMinDataPointIndex(); i <= pSeries->GetMaxDataPointIndex(); i++)
	{
		CBCGPRect rectBounds = pSeries->GetDataPointBoundingRect(i);

		if (rectBounds.IsRectNull())
		{
			continue;
		}
		
		if (rectBounds.Width() == 0)
		{
			m_pRelatedChart->OnDrawChartSeriesLine(pGM, rectBounds.TopLeft(), 
				rectBounds.BottomLeft(), pSeries, i);
		}
		else
		{
			m_pRelatedChart->OnDrawChartSeriesLine(pGM, rectBounds.TopLeft(), 
				rectBounds.TopRight(), pSeries, i);
		}
	}
}
//****************************************************************************************
void CBCGPHistogramChartImpl::OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
		CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pRelatedChart);
	ASSERT_VALID(pSeries);
	
	UNREFERENCED_PARAMETER(rectDiagramArea);
	
	CBCGPChartHistogramSeries* pHistogramSeries = DYNAMIC_DOWNCAST(CBCGPChartHistogramSeries, pSeries);
	
	if (pDataPoint == NULL || pHistogramSeries == NULL)
	{
		return;
	}
	
	
	if (m_pRelatedChart == NULL || m_pRelatedChart->GetVisibleSeriesCount() == 0 ||
		pSeries->IsDataPointScreenPointsEmpty(nDataPointIndex))
	{
		return;
	}
	
	// Get X axis
	CBCGPChartAxis* pXAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X);
	CBCGPChartAxis* pYAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_Y);

	if (pXAxis == NULL || pYAxis == NULL)
	{
		return;
	}
	
	ASSERT_VALID(pXAxis);
	ASSERT_VALID(pYAxis);
	
	double dblZeroLinePos = pXAxis->GetNonIgnoredCrossValue();
	CBCGPChartAxis* pYPerpAxis = pXAxis->GetPerpendecularAxis();

	const CBCGPChartValue& origin = pHistogramSeries->GetOrigin();
	
	if (pYPerpAxis != NULL && pYPerpAxis == pYAxis && 
		pXAxis->m_crossType != CBCGPChartAxis::CT_FIXED_DEFAULT_POS && origin.IsEmpty())
	{
		dblZeroLinePos = pYPerpAxis->PointFromValue(dblZeroLinePos, TRUE);
	}
	else
	{
		dblZeroLinePos = pYAxis->PointFromValue(origin.GetValue(), FALSE);
	}
	
	BOOL bIsStacked = pSeries->IsStakedSeries();
	BOOL bIsRange = pSeries->IsRangeSeries();
	
	CBCGPPoint ptBottom;
	
	if (pXAxis->IsVertical())
	{
		ptBottom.SetPoint(dblZeroLinePos, 0);
	}
	else
	{
		ptBottom.SetPoint(0, dblZeroLinePos);
	}
	
	if ((bIsStacked || bIsRange) && pSeries->GetDataPointScreenPointCount(nDataPointIndex) > 1)
	{
		CBCGPPoint pt = pSeries->GetDataPointScreenPoint(nDataPointIndex, 1);
		ptBottom = pt;
	}

	CBCGPPoint ptTop = pSeries->GetDataPointScreenPoint(nDataPointIndex, 0);
	CBCGPRect rectBounds;

	if (pXAxis->IsVertical())
	{
		rectBounds.SetRect(ptBottom.x, ptTop.y, ptTop.x, ptTop.y);
	}
	else
	{
		rectBounds.SetRect(ptTop.x, ptBottom.y, ptTop.x, ptTop.y);
	}

	pSeries->SetDataPointBoundingRect(nDataPointIndex, rectBounds);
}
//****************************************************************************************
void CBCGPHistogramChartImpl::OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
			const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);
	ASSERT(pSeriesStyle != NULL);
	
	UNREFERENCED_PARAMETER(nDataPointIndex);
	
	CBCGPPoint ptLineStart (rectLegendKey.left, rectLegendKey.CenterPoint().y);
	CBCGPPoint ptLineEnd (rectLegendKey.right, rectLegendKey.CenterPoint().y);
	
	BCGPChartFormatLine& lineStyle = ((BCGPChartFormatSeries*) pSeriesStyle)->m_seriesElementFormat.m_outlineFormat;
	
	double dblLineWidth = pSeriesStyle->m_seriesElementFormat.m_outlineFormat.GetLineWidth(TRUE);
	double dblLineWidthSaved = pSeriesStyle->m_seriesElementFormat.m_outlineFormat.GetLineWidth(FALSE);
	
	if (dblLineWidth >= rectLegendKey.Height())
	{
		dblLineWidth = rectLegendKey.Height() / max(lineStyle.GetScaleRatio().cx, lineStyle.GetScaleRatio().cy) - 2;
		lineStyle.m_dblWidth = dblLineWidth;
	}
	
	m_pRelatedChart->OnDrawChartSeriesLine(pGM, ptLineStart, ptLineEnd, pSeries, nDataPointIndex);
	lineStyle.m_dblWidth = dblLineWidthSaved;
}
//****************************************************************************************
void CBCGPHistogramChartImpl::OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
											   const BCGPChartCellParams& params, CBCGPChartSeries* /*pSeries*/)
{
	ASSERT_VALID(this);

	CBCGPPoint ptLineStart (rectLegendKey.left, rectLegendKey.CenterPoint().y);
	CBCGPPoint ptLineEnd (rectLegendKey.right, rectLegendKey.CenterPoint().y);

	CBCGPSize szScaleRatio = params.m_lineStyle.GetScaleRatio();

	double dblLineWidth = bcg_clamp(params.m_lineStyle.m_dblWidth, 1., 7. * max(szScaleRatio.cx, szScaleRatio.cy));
	pGM->DrawLine(ptLineStart, ptLineEnd, params.m_lineStyle.m_brLineColor, dblLineWidth, &params.m_lineStyle.m_strokeStyle);
}
//****************************************************************************************
// Bubble chart specific implementation
//****************************************************************************************
void CBCGPBubbleChartImpl::OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pRelatedChart);
	ASSERT_VALID(pSeries);

	UNREFERENCED_PARAMETER(rectDiagramArea);

	if (m_pRelatedChart == NULL)
	{
		return;
	}

	int nMinValue = max(0, pSeries->GetMinDataPointIndex() - 1);
	int nMaxValue = min(pSeries->GetMaxDataPointIndex() + 1, pSeries->GetDataPointCount() - 1);

	for (int i = nMinValue; i <= nMaxValue; i++)
	{
		const CBCGPChartDataPoint* pDataPoint = pSeries->GetDataPointAt(i);
		CBCGPRect rectBounds = pSeries->GetDataPointBoundingRect(i);

		if (pDataPoint == NULL || rectBounds.IsRectEmpty())
		{
			continue;
		}

		BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) pSeries->GetDataPointFormat(i, FALSE);

		if (pFormat == NULL)
		{
			continue;
		}

		pFormat->m_markerFormat.m_options.SetMarkerSize(
			(int)(pSeries->GetDataPointScreenPoint(i, CBCGPBubbleChartImpl::BUBBLE_SP_SIZE).x / 
			 m_pRelatedChart->GetScaleRatio().cy));

		m_pRelatedChart->OnDrawChartSeriesMarker(pGM, rectBounds.CenterPoint(), 
			pFormat->m_markerFormat, pSeries, i, TRUE);
	}
}
//****************************************************************************************
void CBCGPBubbleChartImpl::OnDrawDiagramMarker(CBCGPGraphicsManager* /*pGM*/, CBCGPChartSeries* /*pSeries*/, int /*nDataPointIndex*/, 
						 const CBCGPChartDataPoint* /*pDataPoint*/, const BCGPChartFormatSeries* /*pFormatSeries*/, 
						 const CBCGPRect& /*rectDiagram*/)
{

}
//****************************************************************************************
void CBCGPBubbleChartImpl::OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
								const BCGPChartFormatSeries* /*pSeriesStyle*/, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(pSeries);

	UNREFERENCED_PARAMETER(nDataPointIndex);

	BCGPChartFormatSeries* pFormat = (BCGPChartFormatSeries*) pSeries->GetDataPointFormat(nDataPointIndex, FALSE);

	if (pFormat == NULL)
	{
		return;
	}

	pFormat->m_markerFormat.m_options.SetMarkerSize((int)(rectLegendKey.Height() / m_pRelatedChart->GetScaleRatio().cy));
	m_pRelatedChart->OnDrawChartSeriesMarker(pGM, rectLegendKey.CenterPoint(), 
		pFormat->m_markerFormat, pSeries, nDataPointIndex, TRUE);
}
//****************************************************************************************
void CBCGPBubbleChartImpl::OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
											   const BCGPChartCellParams& params, CBCGPChartSeries* /*pSeries*/)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	CBCGPSize szMarker(rectLegendKey.Height(), rectLegendKey.Height());

	m_pRelatedChart->OnDrawChartSeriesMarkerEx(pGM, rectLegendKey.CenterPoint(), 
		szMarker, params.m_brFill, params.m_lineStyle, params.m_markerOptions.m_markerShape);
}
//****************************************************************************************
void CBCGPBubbleChartImpl::OnCalcDataPointLabelRect(CBCGPGraphicsManager* pGM, CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
							  CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);

	if (pDataPoint == NULL || pSeries == NULL || pSeries->IsDataPointScreenPointsEmpty(nDataPointIndex))
	{
		return;
	}

	BCGPChartFormatSeries* pFormatSeries = NULL;
	CBCGPSize szDataLabelSize;
	CBCGPPoint ptMarker;
	CBCGPRect rectBounds;

	if (!OnPrepareDataToCalcDataPointLabelRect(pGM, rectDiagramArea, pDataPoint, pSeries, nDataPointIndex, 
		&pFormatSeries, szDataLabelSize, ptMarker, rectBounds))
	{
		return;
	}

	BOOL bCenter = pFormatSeries->m_dataLabelFormat.m_options.m_position == BCGPChartDataLabelOptions::LP_CENTER;

	CBCGPRect rectLabel = pSeries->GetDataPointLabelRect(nDataPointIndex);

	if (bCenter)
	{
		rectLabel.SetRect(ptMarker.x - szDataLabelSize.cx / 2, 
			ptMarker.y - szDataLabelSize.cy / 2, 
			ptMarker.x + szDataLabelSize.cx / 2, 
			ptMarker.y + szDataLabelSize.cy / 2);
	}
	else
	{
		double dblLabelAngle = bcg_deg2rad(pSeries->GetDataPointLabelAngle(nDataPointIndex));
		double dblCos = cos(dblLabelAngle);
		double dblSin = sin(dblLabelAngle);

		CBCGPPoint ptOffset (szDataLabelSize.cx / 2 * dblSin, -szDataLabelSize.cy / 2 * dblCos);

		double dblDistanceFromMarker = pSeries->GetDataPointScreenPoint(nDataPointIndex, CBCGPBubbleChartImpl::BUBBLE_SP_SIZE).x / 2;

		double dblLabelDistance = pSeries->GetDataPointLabelDistance(nDataPointIndex);
		if (m_pRelatedChart != NULL && m_pRelatedChart->GetScaleRatioMid() != 0.0)
		{
			dblLabelDistance /= m_pRelatedChart->GetScaleRatioMid();
		}

		double dblDistance = dblDistanceFromMarker + dblLabelDistance * dblDistanceFromMarker / 100.0;

		CBCGPPoint ptLabelCenter(ptMarker.x + dblSin * dblDistance, 
			ptMarker.y - dblCos * dblDistance);

		rectLabel.SetRect(ptLabelCenter.x - szDataLabelSize.cx / 2, 
			ptLabelCenter.y - szDataLabelSize.cy / 2, 
			ptLabelCenter.x + szDataLabelSize.cx / 2, 
			ptLabelCenter.y + szDataLabelSize.cy / 2);
		rectLabel.OffsetRect(ptOffset);
	}

	SetDataPointLabelRectAndDropLine(pSeries, nDataPointIndex, pFormatSeries, rectLabel, ptMarker);	
}
//****************************************************************************************
void CBCGPBubbleChartImpl::OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
								CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);

	UNREFERENCED_PARAMETER(rectDiagramArea);

	if (pDataPoint == NULL)
	{
		return;
	}

	CBCGPRect rectBounds;

	CBCGPPoint ptCenter = pSeries->GetDataPointScreenPoint(nDataPointIndex, 0);
	int nSize = (int)pSeries->GetDataPointScreenPoint(nDataPointIndex, CBCGPBubbleChartImpl::BUBBLE_SP_SIZE).x / 2;

	rectBounds.SetRect(ptCenter, ptCenter);
	rectBounds.InflateRect(nSize, nSize);

	pSeries->SetDataPointBoundingRect(nDataPointIndex, rectBounds);
}

//****************************************************************************************
// PIE chart specific implementation
//****************************************************************************************
void CBCGPPieChartImpl::OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagramArea*/, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(pSeries);

	if (!m_bIs3D)
	{
		for (int i = 0; i < pSeries->GetDataPointCount(); i++)
		{
			CBCGPChartDataPoint* pDP = (CBCGPChartDataPoint*)pSeries->GetDataPointAt(i);
			OnDrawPie(pGM, pDP, pSeries, i, FALSE);
		}
		return;
	}

	CList<CBCGPChartDataPoint*, CBCGPChartDataPoint*> lstSortedLeft;
	CList<CBCGPChartDataPoint*, CBCGPChartDataPoint*> lstSortedRight;
	CBCGPChartDataPoint* pDPTop = NULL;
	CBCGPChartDataPoint* pDPBottom = NULL;
	CBCGPChartDataPoint* pDPLast = NULL;
	CBCGPChartDataPoint* pLargeDP = NULL;

	CBCGPChartPieSeries* pPieSeries = DYNAMIC_DOWNCAST(CBCGPChartPieSeries, pSeries);

	if (pPieSeries == NULL)
	{
		return;
	}

	BOOL bIsExplosion = pPieSeries->GetPieExplosion() > 0.;
	BOOL bDiffSides = FALSE;

 	for (int i = 0; i < pSeries->GetDataPointCount(); i++)
 	{
		CBCGPChartDataPoint* pDP = (CBCGPChartDataPoint*)pSeries->GetDataPointAt(i);

		CBCGPPoint ptDiagramCenter = pSeries->GetDataPointScreenPoint(i, PIE_SP_CENTER);
 		CBCGPPoint ptStart = pSeries->GetDataPointScreenPoint(i, PIE_SP_START);
 		CBCGPPoint ptEnd = pSeries->GetDataPointScreenPoint(i, PIE_SP_END);
		CBCGPPoint ptAngles = pSeries->GetDataPointScreenPoint(i, PIE_SP_ANGLES);

		BOOL bIncluded = FALSE;

		if (pPieSeries->GetDataPointPieExplosion(i) > 0.)
		{
			bIsExplosion = TRUE;
		}

		double dblPercentage = pSeries->GetDataPointValue(i, CBCGPChartData::CI_PERCENTAGE).GetValue();
		if (dblPercentage > 50.)
		{
			pDPLast = pDP;

			if (dblPercentage < 100. && bIsExplosion)
			{
				pLargeDP = pDP;
			}
		}

		if (ptStart.x <= ptDiagramCenter.x && ptEnd.x >= ptDiagramCenter.x || 
			ptStart.x >= ptDiagramCenter.x && ptEnd.x <= ptDiagramCenter.x)
		{
			CBCGPPoint ptMarker = OnGetMarkerPoint(pDP, pSeries, i);
			if (ptMarker.y < ptDiagramCenter.y)
			{
				if (pDPTop == NULL)
				{
					pDPTop = pDP; 
					bIncluded = TRUE;
				}
			}
			else
			{
				if (pDPBottom == NULL)
				{
					pDPBottom = pDP;
					bIncluded = TRUE;
				}
			}

			if (dblPercentage > 50.)
			{
				bDiffSides = TRUE;
			}

			if (bIncluded)
			{
				continue;
			}
		}
		
		double dblAngle = bcg_rad2deg(ptAngles.x); 

		if (dblAngle < 0)
		{
			dblAngle = 360 + dblAngle;
		}

		if (dblAngle > 360)
		{
			dblAngle -= 360;
		}

		if (ptStart.x < ptDiagramCenter.x)
		{
			BOOL bInserted = FALSE;

			if (!lstSortedLeft.IsEmpty())
			{
				for (POSITION pos = lstSortedLeft.GetHeadPosition(); pos != NULL; lstSortedLeft.GetNext(pos))
				{
					CBCGPChartDataPoint* pNextDP = lstSortedLeft.GetAt(pos);
					int nIndex = pSeries->FindDataPointIndex(pNextDP);

					double dblAngleNext = bcg_rad2deg(pSeries->GetDataPointScreenPoint(nIndex, PIE_SP_ANGLES).x);

					if (dblAngleNext < 0)
					{
						dblAngleNext = 360 + dblAngleNext;
					}

					if (dblAngleNext > 360)
					{
						dblAngleNext -= 360;
					}

					if (dblAngle > dblAngleNext)
					{
						lstSortedLeft.InsertBefore(pos, pDP);
						bInserted = TRUE;
						break;
					}
				}
			}

			if (!bInserted)
			{
				lstSortedLeft.AddTail(pDP);
			}
		}
		else
		{
			BOOL bInserted = FALSE;

			if (!lstSortedRight.IsEmpty())
			{
				for (POSITION pos = lstSortedRight.GetHeadPosition(); pos != NULL; lstSortedRight.GetNext(pos))
				{
					CBCGPChartDataPoint* pNextDP = (CBCGPChartDataPoint*)lstSortedRight.GetAt(pos);
					int nIndex = pSeries->FindDataPointIndex(pNextDP);

					double dblAngleNext = bcg_rad2deg(pSeries->GetDataPointScreenPoint(nIndex, PIE_SP_ANGLES).x); 

					if (dblAngleNext < 0)
					{
						dblAngleNext = 360 + dblAngleNext;
					}

					if (dblAngleNext > 360)
					{
						dblAngleNext -= 360;
					}

					if (dblAngle < dblAngleNext)
					{
						lstSortedRight.InsertBefore(pos, pDP);
						bInserted = TRUE;
						break;
					}
				}
			}

			if (!bInserted)
			{
				lstSortedRight.AddTail(pDP);
			}
		}
 	}

	if (pLargeDP != NULL)
	{
		OnDrawPie(pGM, pLargeDP, pSeries, pSeries->FindDataPointIndex(pLargeDP), bIsExplosion);
	}

	if (pDPTop != NULL && pDPTop != pLargeDP)
	{
		OnDrawPie(pGM, pDPTop, pSeries, pSeries->FindDataPointIndex(pDPTop), bIsExplosion);
	}

	POSITION pos = NULL;

	for (pos = lstSortedLeft.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartDataPoint* pNextDP = (CBCGPChartDataPoint*)lstSortedLeft.GetNext(pos);
		if (pLargeDP != pNextDP)
		{
			OnDrawPie(pGM, pNextDP, pSeries, pSeries->FindDataPointIndex(pNextDP), bIsExplosion);
		}
	}

	for (pos = lstSortedRight.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartDataPoint* pNextDP = (CBCGPChartDataPoint*)lstSortedRight.GetNext(pos);
		if (pLargeDP != pNextDP)
		{
			OnDrawPie(pGM, pNextDP, pSeries, pSeries->FindDataPointIndex(pNextDP), bIsExplosion);
		}
	}

	if (pDPBottom != NULL && pDPBottom != pLargeDP)
	{
		OnDrawPie(pGM, pDPBottom, pSeries, pSeries->FindDataPointIndex(pDPBottom), bIsExplosion);
	}

 	if (pDPLast != NULL && pDPLast != pDPBottom && pDPLast != pLargeDP)
 	{
 		OnDrawPie(pGM, pDPLast, pSeries, pSeries->FindDataPointIndex(pDPLast), bIsExplosion);
 	}

	if (pLargeDP != NULL)
	{
		int nDPIndex = pSeries->FindDataPointIndex(pLargeDP);
		CBCGPPoint ptDiagramCenter = pSeries->GetDataPointScreenPoint(nDPIndex, PIE_SP_CENTER);
		CBCGPPoint ptStart = pSeries->GetDataPointScreenPoint(nDPIndex, PIE_SP_START);
 		CBCGPPoint ptEnd = pSeries->GetDataPointScreenPoint(nDPIndex, PIE_SP_END);

		int nExcludeFlag = BCGP_3D_DRAW_EDGE2 | BCGP_3D_DRAW_EDGE1;; 

		if (ptStart.y >= ptDiagramCenter.y && ptEnd.y >= ptDiagramCenter.y)
		{
			if (ptStart.y < ptEnd.y)
			{
				nExcludeFlag |= BCGP_3D_DRAW_SIDE2;
			}
			else
			{
				nExcludeFlag |= BCGP_3D_DRAW_SIDE1;
			}
		}

		if (!bDiffSides || ptStart.y <= ptDiagramCenter.y && ptEnd.y <= ptDiagramCenter.y || pLargeDP == pDPBottom)
		{
			OnDrawPie(pGM, pLargeDP, pSeries, nDPIndex, bIsExplosion, nExcludeFlag);
		}
	}

}
//****************************************************************************************
void CBCGPPieChartImpl::OnDrawPie(CBCGPGraphicsManager* pGM, CBCGPChartDataPoint* /*pDP*/, 
								  CBCGPChartSeries* pSeries, int nDataPointIndex, BOOL bIsExplosion, 
								  int nExcludeFlag)
{
	ASSERT_VALID(this);

	CBCGPPoint ptCenter = pSeries->GetDataPointScreenPoint(nDataPointIndex, PIE_SP_CENTER);

	double dblRadiusX = pSeries->GetDataPointScreenPoint(nDataPointIndex, PIE_SP_RADIUS).x;
	double dblRadiusY = pSeries->GetDataPointScreenPoint(nDataPointIndex, PIE_SP_RADIUS).y;
	double dblHeight = pSeries->GetDataPointScreenPoint(nDataPointIndex, PIE_SP_HEIGHT).x;
	CBCGPRect bounds (ptCenter.x - dblRadiusX, ptCenter.y - dblRadiusY, 
		ptCenter.x + dblRadiusX, ptCenter.y + dblRadiusY);

	CBCGPPoint ptAngles = pSeries->GetDataPointScreenPoint(nDataPointIndex, PIE_SP_ANGLES);
	CBCGPEllipse ellipse(bounds);
	double corr = bcg_deg2rad(90);

	BCGPSeriesColorsPtr colors;
	const BCGPChartFormatSeries* pFormat = pSeries->GetColors(colors, nDataPointIndex);

	if (pFormat == NULL || colors.m_pBrElementFillColor == NULL || colors.m_pBrElementLineColor == NULL)
	{
		return;
	}

	const double dblStartAngle = bcg_rad2deg(corr - ptAngles.y);
	const double dblFinishAngle = bcg_rad2deg(corr - ptAngles.x);

	const int nDoughnutPercent = GetDoughnutPercent(pSeries);

	if (!m_bIs3D)
	{
		if (nDoughnutPercent > 0)
		{
			pGM->DrawDoughnut(ellipse, .01 * nDoughnutPercent, dblStartAngle, dblFinishAngle, 
				*colors.m_pBrElementFillColor, *colors.m_pBrElementLineColor);
		}
		else
		{
			pGM->DrawPie(ellipse, dblStartAngle, dblFinishAngle, 
				*colors.m_pBrElementFillColor, *colors.m_pBrElementLineColor);
		}
	}
	else
	{
		CBCGPBrush brSide = *colors.m_pBrElementFillColor;
		brSide.MakeDarker(0.2);

		int nDrawingFlags = bIsExplosion ? BCGP_3D_DRAW_ALL : (BCGP_3D_DRAW_TOP | BCGP_3D_DRAW_SIDE1 | BCGP_3D_DRAW_SIDE2 | BCGP_3D_DRAW_INTERNAL_SIDE);
		nDrawingFlags &= ~nExcludeFlag;

		if (IsTorus())
		{
			pGM->Draw3DTorus(ellipse, ellipse.radiusY * 2 / 3, dblStartAngle, 
				dblFinishAngle, *colors.m_pBrElementFillColor, 
				brSide, *colors.m_pBrElementLineColor, 0, nDrawingFlags);
		}
		else if (nDoughnutPercent > 0)
		{
			pGM->Draw3DDoughnut(ellipse, .01 * nDoughnutPercent, dblHeight, dblStartAngle, 
				dblFinishAngle, *colors.m_pBrElementFillColor, 
				brSide, *colors.m_pBrElementLineColor, 0, nDrawingFlags);
		}
		else
		{
			pGM->Draw3DPie(ellipse, dblHeight, dblStartAngle, 
				dblFinishAngle, *colors.m_pBrElementFillColor, 
				brSide, *colors.m_pBrElementLineColor, 0, nDrawingFlags);
		}
	}

	if (pFormat->m_markerFormat.m_options.m_bShowMarker)
	{
		CBCGPPoint ptMarker = OnGetMarkerPoint(pSeries->GetDataPointAt(nDataPointIndex), pSeries, nDataPointIndex);
		m_pRelatedChart->OnDrawChartSeriesMarker(pGM, ptMarker, pFormat->m_markerFormat, pSeries, nDataPointIndex);
	}
}
//****************************************************************************************
void CBCGPPieChartImpl::OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
										   CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);

	if (pDataPoint != NULL)
	{
		pDataPoint->m_rectBounds.SetRectEmpty();
		CBCGPBaseChartImpl::OnCalcBoundingRect(pDataPoint, rectDiagramArea, pSeries, nDataPointIndex);	
	}
}
//****************************************************************************************
CBCGPSize CBCGPPieChartImpl::OnCalcLegendKeySize(CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	UNREFERENCED_PARAMETER(pSeries);
	UNREFERENCED_PARAMETER(nDataPointIndex);
	
	if (m_pRelatedChart == NULL)
	{
		return CBCGPSize(26, 26);
	}

	
	return m_pRelatedChart->GetScaleRatio() * 26.;
}
//****************************************************************************************
CBCGPSize CBCGPPieChartImpl::OnCalcLegendKeySizeEx(const BCGPChartCellParams& params, CBCGPChartSeries* /*pSeries*/)
{
	return params.GetScaleRatio() * 26.;
}
//****************************************************************************************
void CBCGPPieChartImpl::OnCalcDataPointLabelRect(CBCGPGraphicsManager* pGM, CBCGPChartDataPoint* pDataPoint, const CBCGPRect& rectDiagramArea, 
							  CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);

	if (pDataPoint == NULL || pSeries == NULL || pSeries->IsDataPointScreenPointsEmpty(nDataPointIndex))
	{
		return;
	}

	BCGPChartFormatSeries* pFormatSeries = NULL;
	CBCGPSize szDataLabelSize;
	CBCGPPoint ptMarker;
	CBCGPRect rectBounds;

	if (!OnPrepareDataToCalcDataPointLabelRect(pGM, rectDiagramArea, pDataPoint, pSeries, nDataPointIndex, 
		&pFormatSeries, szDataLabelSize, ptMarker, rectBounds))
	{
		return;
	}

	double dblRadiusX = pSeries->GetDataPointScreenPoint(nDataPointIndex, PIE_SP_RADIUS).x;
	double dblRadiusY = pSeries->GetDataPointScreenPoint(nDataPointIndex, PIE_SP_RADIUS).y;
	CBCGPPoint ptAngles = pSeries->GetDataPointScreenPoint(nDataPointIndex, PIE_SP_ANGLES);
	CBCGPPoint ptCenter = pSeries->GetDataPointScreenPoint(nDataPointIndex, PIE_SP_CENTER);

	CBCGPSize szPadding = pFormatSeries->m_dataLabelFormat.GetContentPadding(TRUE);

	double dblDistance = max(dblRadiusX, dblRadiusY) / 2 *  
		pSeries->GetDataPointLabelDistance(nDataPointIndex) / (100 * m_pRelatedChart->GetScaleRatio().cx);
	double dblXDistance = dblDistance + szPadding.cx;
	double dblYDistance = dblDistance + szPadding.cy;

	if (pFormatSeries->m_dataLabelFormat.m_options.m_position == BCGPChartDataLabelOptions::LP_INSIDE_END)
	{
		dblXDistance += pSeries->GetMaxDataLabelSize().cx / 2;
		dblYDistance += pSeries->GetMaxDataLabelSize().cy / 2;
	}

	CBCGPRect rectLabel;

	double dblDiff = ptAngles.y > ptAngles.x ? ptAngles.x + (ptAngles.y - ptAngles.x) / 2 : ptAngles.x + (ptAngles.x - ptAngles.y) / 2;
	CBCGPPoint ptPieCenter (ptCenter.x + dblRadiusX / 2 * sin(dblDiff), ptCenter.y - dblRadiusY / 2 * cos(dblDiff));
	CBCGPPoint ptOffset (dblXDistance * sin(dblDiff), -dblYDistance * cos(dblDiff));

	CBCGPPoint ptMarkerPos = ptMarker;

	double dblAngleDeg = bcg_normalize_deg(bcg_rad2deg(dblDiff));

	switch(pFormatSeries->m_dataLabelFormat.m_options.m_position)
	{
	case BCGPChartDataLabelOptions::LP_INSIDE_BASE:
	case BCGPChartDataLabelOptions::LP_CENTER:
		rectLabel.SetRect(ptPieCenter, CBCGPSize(0, 0));
		rectLabel.InflateRect(szDataLabelSize.cx / 2, szDataLabelSize.cy / 2);
		break;

	case BCGPChartDataLabelOptions::LP_INSIDE_END:
		ptMarkerPos -= ptOffset;
		rectLabel.SetRect(ptMarkerPos, CBCGPSize(0, 0));
		rectLabel.InflateRect(szDataLabelSize.cx / 2, szDataLabelSize.cy / 2);
		break;

	case BCGPChartDataLabelOptions::LP_DEFAULT_POS:
	case BCGPChartDataLabelOptions::LP_OUTSIDE_END:
		ptMarkerPos += ptOffset;
		if (dblAngleDeg > 0 && dblAngleDeg <= 90)
		{
			rectLabel.left = ptMarkerPos.x;
			rectLabel.right = ptMarkerPos.x + szDataLabelSize.cx;
			rectLabel.bottom = ptMarkerPos.y;
			rectLabel.top = ptMarkerPos.y - szDataLabelSize.cy;
		}
		else if (dblAngleDeg > 90 && dblAngleDeg <= 180)
		{
			double dblPieHeight = pSeries->GetDataPointScreenPoint(nDataPointIndex, PIE_SP_HEIGHT).x;
			rectLabel.SetRect(ptMarkerPos, szDataLabelSize);
			rectLabel.OffsetRect(0, dblPieHeight);
		}
		else if (dblAngleDeg > 180 && dblAngleDeg <= 270)
		{
			double dblPieHeight = pSeries->GetDataPointScreenPoint(nDataPointIndex, PIE_SP_HEIGHT).x;
			rectLabel.right = ptMarkerPos.x;
			rectLabel.left = ptMarkerPos.x - szDataLabelSize.cx; 
			rectLabel.top = ptMarkerPos.y;
			rectLabel.bottom = ptMarkerPos.y + szDataLabelSize.cy;
			rectLabel.OffsetRect(0, dblPieHeight);
		}
		else
		{
			rectLabel.right = ptMarkerPos.x;
			rectLabel.left = ptMarkerPos.x - szDataLabelSize.cx; 
			rectLabel.bottom = ptMarkerPos.y;
			rectLabel.top = ptMarkerPos.y - szDataLabelSize.cy;
		}
		break;
	}

	if (pFormatSeries->m_dataLabelFormat.m_options.m_position != BCGPChartDataLabelOptions::LP_OUTSIDE_END && 
		pFormatSeries->m_dataLabelFormat.m_options.m_position != BCGPChartDataLabelOptions::LP_DEFAULT_POS)
	{
		ptMarker = rectLabel.CenterPoint();
	}

	SetDataPointLabelRectAndDropLine(pSeries, nDataPointIndex, pFormatSeries, rectLabel, ptMarker);
}
//****************************************************************************************
CBCGPPoint CBCGPPieChartImpl::OnGetMarkerPoint(const CBCGPChartDataPoint* pDataPoint, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);

	UNREFERENCED_PARAMETER(pDataPoint);

	CBCGPChartPieSeries* pPieSeries = DYNAMIC_DOWNCAST(CBCGPChartPieSeries, pSeries);

	if (pPieSeries == NULL || pDataPoint == NULL)
	{
		return CBCGPPoint();
	}

	ASSERT_VALID(pPieSeries);

	double dblRadiusX = pSeries->GetDataPointScreenPoint(nDataPointIndex, PIE_SP_RADIUS).x;
	double dblRadiusY = pSeries->GetDataPointScreenPoint(nDataPointIndex, PIE_SP_RADIUS).y;
	CBCGPPoint ptAngles = pSeries->GetDataPointScreenPoint(nDataPointIndex, PIE_SP_ANGLES);
	CBCGPPoint ptCenter = pSeries->GetDataPointScreenPoint(nDataPointIndex, PIE_SP_CENTER);
	
	double dblDiff = ptAngles.y > ptAngles.x ? ptAngles.x + (ptAngles.y - ptAngles.x) / 2 : ptAngles.x + (ptAngles.x - ptAngles.y) / 2;
	return CBCGPPoint(ptCenter.x + dblRadiusX * sin(dblDiff), ptCenter.y - dblRadiusY * cos(dblDiff));
}
//****************************************************************************************
void CBCGPPieChartImpl::OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
						  const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT(pSeriesStyle != NULL);
	
	UNREFERENCED_PARAMETER(pSeries);
	UNREFERENCED_PARAMETER(nDataPointIndex);

	CBCGPRect rectKey = rectLegendKey;

	rectKey.DeflateRect(pSeriesStyle->m_seriesElementFormat.m_outlineFormat.GetLineWidth(TRUE), 
								pSeriesStyle->m_seriesElementFormat.m_outlineFormat.GetLineWidth(TRUE));

	CBCGPEllipse ellipse(rectKey);
	m_pRelatedChart->OnDrawChartSeriesItem(pGM, ellipse, pSeries, nDataPointIndex, 
		CBCGPChartVisualObject::CE_MAIN_ELEMENT, FALSE, FALSE);
}
//****************************************************************************************
void CBCGPPieChartImpl::OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
												  const BCGPChartCellParams& params, CBCGPChartSeries* /*pSeries*/)
{
	CBCGPRect rectKey = rectLegendKey;
	
	rectKey.DeflateRect(params.m_lineStyle.GetLineWidth(TRUE), params.m_lineStyle.GetLineWidth(TRUE));
	
	CBCGPEllipse ellipse(rectKey);
	m_pRelatedChart->OnDrawChartEllipse(pGM, ellipse, &params.m_brFill, &params.m_lineStyle.m_brLineColor, 
		params.m_lineStyle.GetLineWidth(TRUE), params.m_lineStyle.m_strokeStyle);
}
//****************************************************************************************
// Doughnut chart specific implementation
//****************************************************************************************
int CBCGPDoughnutChartImpl::GetDoughnutPercent(CBCGPChartSeries* pSeries) const
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);

	CBCGPChartDoughnutSeries* pDoughnutSeries = DYNAMIC_DOWNCAST(CBCGPChartDoughnutSeries, pSeries);
	if (pDoughnutSeries == NULL)
	{
		return 0;
	}

	ASSERT_VALID(pDoughnutSeries);
	return pDoughnutSeries->GetDoughnutPercent();
}

//****************************************************************************************
// Pyramid chart specific implementation
//****************************************************************************************
void CBCGPPyramidChartImpl::OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagramArea*/, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	CBCGPChartPyramidSeries* pPyramidSeries = DYNAMIC_DOWNCAST(CBCGPChartPyramidSeries, pSeries);

	if (pPyramidSeries == NULL || pGM == NULL)
	{
		return;
	}

	ASSERT_VALID(pPyramidSeries);	

	CArray<CBCGPChartDataPoint*, CBCGPChartDataPoint*> arSorted;
	pPyramidSeries->SortDataPoints(PYRAMID_SP_HEIGHT, arSorted);

	for (int i = (int)arSorted.GetSize() - 1; i >= 0; i--)
	{
		CBCGPChartDataPoint* pDPSorted = arSorted[i];
		OnDrawPyramidDataPoint(pGM, pPyramidSeries, pPyramidSeries->FindDataPointIndex(pDPSorted));
	}
}
//****************************************************************************************
void CBCGPPyramidChartImpl::OnDrawPyramidDataPoint(CBCGPGraphicsManager* pGM, CBCGPChartPyramidSeries* pSeries, 
												   int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(pSeries);

	if (pGM == NULL || pSeries == NULL || nDataPointIndex < 0 || nDataPointIndex >= pSeries->GetDataPointCount())
	{
		return;
	}

	CBCGPRect rectPyramid (pSeries->GetDataPointScreenPoint(nDataPointIndex, PYRAMID_SP_LEFT_TOP), 
								pSeries->GetDataPointScreenPoint(nDataPointIndex, PYRAMID_SP_RIGHT_BOTTOM));
	CBCGPPoint ptOffsets = pSeries->GetDataPointScreenPoint(nDataPointIndex, PYRAMID_SP_OFFSETS);

	BCGPSeriesColorsPtr colors;
	const BCGPChartFormatSeries* pFormat = pSeries->GetColors(colors, nDataPointIndex);

	if (pFormat == NULL || colors.m_pBrElementFillColor == NULL || colors.m_pBrElementLineColor == NULL)
	{
		return;
	}

	if (m_bIs3D)
	{
		CBCGPBrush brFillLeft = *colors.m_pBrElementFillColor;
		brFillLeft.MakeDarker(0.3);

		CBCGPBrush brFillTop = *colors.m_pBrElementFillColor;
		brFillTop.MakeDarker(0.1);

		double dblDepth = rectPyramid.Height() * pSeries->GetDepthPercent() / 100.; 
		double dblOffset = sin(bcg_deg2rad(pSeries->GetRotation())) * rectPyramid.Width() / 2;

		pGM->Draw3DPyramid(rectPyramid, dblDepth, brFillLeft, *colors.m_pBrElementFillColor, 
			*colors.m_pBrElementLineColor, dblOffset, ptOffsets.x, ptOffsets.y, pSeries->IsCircularBase(), 
			brFillTop);
	}
	else
	{
		pGM->DrawPyramid(rectPyramid, *colors.m_pBrElementFillColor, *colors.m_pBrElementLineColor, 
						ptOffsets.x, ptOffsets.y);
	}
}
//****************************************************************************************
void CBCGPPyramidChartImpl::OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
		const BCGPChartFormatSeries* /*pSeriesStyle*/, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);

	UNREFERENCED_PARAMETER(nDataPointIndex);

	m_pRelatedChart->OnDrawChartSeriesItem(pGM, rectLegendKey, pSeries, nDataPointIndex, CBCGPChartVisualObject::CE_MAIN_ELEMENT, FALSE, FALSE);
}
//****************************************************************************************
void CBCGPPyramidChartImpl::OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
											   const BCGPChartCellParams& params, CBCGPChartSeries* /*pSeries*/)
{
	m_pRelatedChart->OnDrawChartRect(pGM, rectLegendKey, &params.m_brFill, &params.m_lineStyle.m_brLineColor, 
		params.m_lineStyle.GetLineWidth(TRUE), &params.m_lineStyle.m_strokeStyle, FALSE, FALSE);
}
//****************************************************************************************
void CBCGPPyramidChartImpl::OnCalcBoundingRect(CBCGPChartDataPoint* pDataPoint, const CBCGPRect& /*rectDiagramArea*/, 
		CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);

	if (pDataPoint == NULL || pSeries == NULL)
	{
		return;
	}

	ASSERT_VALID(pSeries);

	CBCGPRect rectPyramid (pSeries->GetDataPointScreenPoint(nDataPointIndex, PYRAMID_SP_LEFT_TOP), 
								pSeries->GetDataPointScreenPoint(nDataPointIndex, PYRAMID_SP_RIGHT_BOTTOM));
	CBCGPPoint ptOffsets = pSeries->GetDataPointScreenPoint(nDataPointIndex, PYRAMID_SP_OFFSETS);

	pDataPoint->m_rectBounds = CBCGPRect(rectPyramid.left, rectPyramid.top + ptOffsets.x, 
										 rectPyramid.right, rectPyramid.top + ptOffsets.y);

}
//****************************************************************************************
CBCGPPoint CBCGPPyramidChartImpl::OnGetMarkerPoint(const CBCGPChartDataPoint* pDataPoint, CBCGPChartSeries* pSeries, 
												   int nDataPointIndex)
{
	ASSERT_VALID(this);

	CBCGPChartPyramidSeries* pPyramidSeries = DYNAMIC_DOWNCAST(CBCGPChartPyramidSeries, pSeries);

	if (pPyramidSeries == NULL)
	{
		return CBCGPPoint();
	}

	ASSERT_VALID(pPyramidSeries);

	const BCGPChartFormatSeries* pFormatSeries = pSeries->GetDataPointFormat(nDataPointIndex, FALSE);

	if (pFormatSeries == NULL)
	{
		return CBCGPPoint();
	}

	BCGPChartDataLabelOptions::LabelPosition pos = pFormatSeries->m_dataLabelFormat.m_options.m_position;

	if (pos == BCGPChartDataLabelOptions::LP_DEFAULT_POS)
	{
		pos = BCGPChartDataLabelOptions::LP_OUTSIDE_END;
	}

	if (pos == BCGPChartDataLabelOptions::LP_OUTSIDE_END)
	{
		CBCGPRect rectShape(pPyramidSeries->GetDataPointScreenPoint(nDataPointIndex, PYRAMID_SP_LEFT_TOP), 
									 pPyramidSeries->GetDataPointScreenPoint(nDataPointIndex, PYRAMID_SP_RIGHT_BOTTOM));
		CBCGPPoint ptOffsets = pSeries->GetDataPointScreenPoint(nDataPointIndex, PYRAMID_SP_OFFSETS);

		double dblDepth = m_bIs3D ? rectShape.Height() * pPyramidSeries->GetDepthPercent() / 100. : 0.; 
		double dblOffset = m_bIs3D && !pPyramidSeries->IsCircularBase() ? sin(bcg_deg2rad(pPyramidSeries->GetRotation())) * rectShape.Width() / 2 : 0.;

		CBCGPPoint ptLeft;
		CBCGPPoint ptRight;
		GetPyramidPoints(rectShape, dblDepth, dblOffset, ptOffsets.x, ptOffsets.y, ptLeft, ptRight);

		double dblLabelAngle = pSeries->GetDataPointLabelAngle(nDataPointIndex);
		if (dblLabelAngle >= 0 && dblLabelAngle <= 180)
		{
			return ptRight;
		}

		return ptLeft;
	}

	return pDataPoint->m_rectBounds.CenterPoint();
}
//****************************************************************************************
void CBCGPPyramidChartImpl::OnCalcDataPointLabelRect(CBCGPGraphicsManager* pGM, CBCGPChartDataPoint* pDataPoint, 
					const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);

	if (pSeries == NULL || pGM == NULL)
	{
		return;
	}

	ASSERT_VALID(pSeries);
	ASSERT_VALID(pGM);

	CBCGPChartPyramidSeries* pPyramidSeries = DYNAMIC_DOWNCAST(CBCGPChartPyramidSeries, pSeries);

	if (pPyramidSeries == NULL)
	{
		return;
	}

	BCGPChartFormatSeries* pFormatSeries = NULL;
	CBCGPSize szDataLabelSize;
	CBCGPPoint ptMarker;
	CBCGPRect rectBounds;

	if (!OnPrepareDataToCalcDataPointLabelRect(pGM, rectDiagramArea, pDataPoint, pSeries, nDataPointIndex, 
		&pFormatSeries, szDataLabelSize, ptMarker, rectBounds))
	{
		return;
	}

	BCGPChartDataLabelOptions::LabelPosition pos = pFormatSeries->m_dataLabelFormat.m_options.m_position;

	if (pos == BCGPChartDataLabelOptions::LP_DEFAULT_POS)
	{
		pos = BCGPChartDataLabelOptions::LP_OUTSIDE_END;
	}

	double dblDistance = pSeries->GetDataPointLabelDistance(nDataPointIndex);
	double dblAngle = pSeries->GetDataPointLabelAngle(nDataPointIndex);
	BOOL bDataLabelRight = dblAngle >= 0 && dblAngle < 180;

	CBCGPRect rectDataLabel;
	CBCGPSize szMaxDataLabelSize = pSeries->GetMaxDataLabelSize();

	switch(pos)
	{
	case BCGPChartDataLabelOptions::LP_INSIDE_BASE:
		dblDistance = dblDistance * rectBounds.Height() / 100.;
		rectDataLabel.bottom = rectBounds.bottom - dblDistance;
		rectDataLabel.top = rectDataLabel.bottom - szDataLabelSize.cy;
		rectDataLabel.left = rectBounds.CenterPoint().x - szDataLabelSize.cx / 2;
		rectDataLabel.right = rectDataLabel.left + szDataLabelSize.cx;

		AlignRectToArea(rectBounds, rectDataLabel, TRUE, TRUE);
		ptMarker = rectDataLabel.CenterPoint();
		break;

	case BCGPChartDataLabelOptions::LP_INSIDE_END:
		dblDistance = dblDistance * rectBounds.Height() / 100.;
		rectDataLabel.top = rectBounds.top + dblDistance;
		rectDataLabel.bottom = rectDataLabel.top + szDataLabelSize.cy;
		rectDataLabel.left = rectBounds.CenterPoint().x - szDataLabelSize.cx / 2;
		rectDataLabel.right = rectDataLabel.left + szDataLabelSize.cx;

		AlignRectToArea(rectBounds, rectDataLabel, TRUE, TRUE);
		ptMarker = rectDataLabel.CenterPoint();
		break;

	case BCGPChartDataLabelOptions::LP_OUTSIDE_END:
		if (pPyramidSeries->m_bDataLabelsInColumns)
		{
			if (bDataLabelRight)
			{
				rectDataLabel.left = rectDiagramArea.right - dblDistance - szMaxDataLabelSize.cx; 
			}
			else
			{
				rectDataLabel.left = rectDiagramArea.left + dblDistance;
			}
		}
		else
		{
			if (bDataLabelRight)
			{
				rectDataLabel.left = ptMarker.x + dblDistance;
			}
			else
			{
				rectDataLabel.left = ptMarker.x - dblDistance - szDataLabelSize.cx;;
			}
		}

		rectDataLabel.right = rectDataLabel.left + szDataLabelSize.cx;
		rectDataLabel.top = ptMarker.y - szDataLabelSize.cy / 2;
		rectDataLabel.bottom = rectDataLabel.top + szDataLabelSize.cy;
		break;

	case BCGPChartDataLabelOptions::LP_CENTER:
	default:
		CBCGPBaseChartImpl::OnCalcDataPointLabelRect(pGM, pDataPoint, rectDiagramArea, pSeries, nDataPointIndex);
		return;
	}

	SetDataPointLabelRectAndDropLine(pSeries, nDataPointIndex, pFormatSeries, rectDataLabel, ptMarker);		
}
//****************************************************************************************
void CBCGPPyramidChartImpl::GetPyramidPoints(const CBCGPRect& rectPyramid, double dblDepth, double dblXCenterOffset,
		double Y1, double Y2, CBCGPPoint& ptLeft, CBCGPPoint& ptRight)
{
	CBCGPRect rect = rectPyramid;
	rect.DeflateRect(dblDepth, 0, dblDepth, dblDepth);

	double dblMaxXOffset = rect.Width() / 2;

	if (fabs(dblXCenterOffset) > dblMaxXOffset)
	{
		if (dblXCenterOffset < 0)
		{
			dblXCenterOffset = -dblMaxXOffset; 
		}
		else
		{
			dblXCenterOffset = dblMaxXOffset;
		}
	}

	if (dblXCenterOffset != 0)
	{
		double dblHalfWidth = rect.Width() / 2;
		dblDepth *= (dblHalfWidth - fabs(dblXCenterOffset)) / dblHalfWidth;
	}

	Y1 = rect.top + max(0., Y1);
	Y2 = (Y2 < 0.) ? rect.bottom : rect.top + Y2;

	double dblAngle = bcg_angle(rect.Width() / 2, rect.Height());

	double dy       = (Y2 - Y1);
	double dx       = fabs(dy / tan(dblAngle));
	double dxTop = (dx * (Y1 - rect.top) / dy);
	double dxBottom    = dxTop + dx;
	double dx1 = (dxTop + dxBottom) / 2;

	double Y = (Y1 + Y2) / 2;

	if (Y1 > rect.top)
	{
		double dblRatioTop = (rect.bottom - Y1) / rect.Height();
		double dblDepthTop = dblDepth * (1. - dblRatioTop);

		double dblRatioBottom = (rect.bottom - Y2) / rect.Height();
		double dblDepthBottom = dblDepth * (1. - dblRatioBottom);

		Y = (Y1 - .5 * dblDepthTop + Y2 - .5 * dblDepthBottom) / 2;
	}

	ptLeft.x = rect.CenterPoint().x - dx1;
	ptLeft.y = Y;

	ptRight.x = rect.CenterPoint().x + dx1;
	ptRight.y = Y;
}
//****************************************************************************************
// Funnel chart specifics
//****************************************************************************************
void CBCGPFunnelChartImpl::OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagramArea*/, 
																		CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	CBCGPChartFunnelSeries* pFunnelSeries = DYNAMIC_DOWNCAST(CBCGPChartFunnelSeries, pSeries);

	if (pFunnelSeries == NULL || pGM == NULL)
	{
		return;
	}

	ASSERT_VALID(pFunnelSeries);	

	CArray<CBCGPChartDataPoint*, CBCGPChartDataPoint*> arSorted;
	pFunnelSeries->SortDataPoints(PYRAMID_SP_HEIGHT, arSorted);

	for (int i = 0; i < arSorted.GetSize(); i++)
	{
		CBCGPChartDataPoint* pDPSorted = arSorted[i];
		OnDrawFunnelDataPoint(pGM, pFunnelSeries, pFunnelSeries->FindDataPointIndex(pDPSorted));
	}
}
//****************************************************************************************
void CBCGPFunnelChartImpl::OnDrawFunnelDataPoint(CBCGPGraphicsManager* pGM, CBCGPChartFunnelSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(pSeries);

	if (pGM == NULL || pSeries == NULL || nDataPointIndex < 0 || nDataPointIndex >= pSeries->GetDataPointCount())
	{
		return;
	}

	CBCGPRect rectFunnel (pSeries->GetDataPointScreenPoint(nDataPointIndex, PYRAMID_SP_LEFT_TOP), 
								pSeries->GetDataPointScreenPoint(nDataPointIndex, PYRAMID_SP_RIGHT_BOTTOM));
	CBCGPPoint ptOffsets = pSeries->GetDataPointScreenPoint(nDataPointIndex, PYRAMID_SP_OFFSETS);
	CBCGPSize szNeck = pSeries->GetNeckSizeInPixels();

	BCGPSeriesColorsPtr colors;
	const BCGPChartFormatSeries* pFormat = pSeries->GetColors(colors, nDataPointIndex);

	if (pFormat == NULL || colors.m_pBrElementFillColor == NULL || colors.m_pBrElementLineColor == NULL)
	{
		return;
	}

	if (m_bIs3D)
	{
		CBCGPBrush brFillLeft = *colors.m_pBrElementFillColor;
		brFillLeft.MakeDarker(0.3);

		CBCGPBrush brFillTop = *colors.m_pBrElementFillColor;
		brFillTop.MakeDarker(0.2);

		double dblDepth = rectFunnel.Height() * pSeries->GetDepthPercent() / 100.; 
		//double dblOffset = sin(bcg_deg2rad(pSeries->GetRotation())) * rectFunnel.Width() / 2;

		pGM->Draw3DFunnel(rectFunnel, dblDepth, szNeck.cy, szNeck.cx, 
			*colors.m_pBrElementFillColor, *colors.m_pBrElementLineColor,
			ptOffsets.x, ptOffsets.y, pSeries->IsCircularBase(), brFillTop);
	}
	else
	{
 		pGM->DrawFunnel(rectFunnel, szNeck.cy, szNeck.cx, *colors.m_pBrElementFillColor, 
			*colors.m_pBrElementLineColor, ptOffsets.x, ptOffsets.y);
	}
}
//****************************************************************************************
CBCGPPoint CBCGPFunnelChartImpl::OnGetMarkerPoint(const CBCGPChartDataPoint* pDataPoint, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);

	CBCGPChartFunnelSeries* pFunnelSeries = DYNAMIC_DOWNCAST(CBCGPChartFunnelSeries, pSeries);

	if (pFunnelSeries == NULL)
	{
		return CBCGPPoint();
	}

	ASSERT_VALID(pFunnelSeries);

	const BCGPChartFormatSeries* pFormatSeries = pFunnelSeries->GetDataPointFormat(nDataPointIndex, FALSE);

	if (pFormatSeries == NULL)
	{
		return CBCGPPoint();
	}

	BCGPChartDataLabelOptions::LabelPosition pos = pFormatSeries->m_dataLabelFormat.m_options.m_position;

	if (pos == BCGPChartDataLabelOptions::LP_DEFAULT_POS)
	{
		pos = BCGPChartDataLabelOptions::LP_OUTSIDE_END;
	}

	if (pos == BCGPChartDataLabelOptions::LP_OUTSIDE_END)
	{
		CBCGPRect rectShape(pFunnelSeries->GetDataPointScreenPoint(nDataPointIndex, PYRAMID_SP_LEFT_TOP), 
									 pFunnelSeries->GetDataPointScreenPoint(nDataPointIndex, PYRAMID_SP_RIGHT_BOTTOM));
		CBCGPPoint ptOffsets = pFunnelSeries->GetDataPointScreenPoint(nDataPointIndex, PYRAMID_SP_OFFSETS);
		double dblDepth = m_bIs3D ? rectShape.Height() * pFunnelSeries->GetDepthPercent() / 100. : 0.; 

		CBCGPPoint ptLeft;
		CBCGPPoint ptRight;
		GetFunnelPoints(rectShape, dblDepth, pFunnelSeries->GetNeckSizeInPixels(), ptOffsets.x, ptOffsets.y, ptLeft, ptRight);

		if (pFormatSeries->m_dataLabelFormat.m_options.m_dblAngle >= 0 && 
			pFormatSeries->m_dataLabelFormat.m_options.m_dblAngle <= 180)
		{
			return ptRight;
		}

		return ptLeft;
	}

	return pDataPoint->m_rectBounds.CenterPoint();
}
//****************************************************************************************
void CBCGPFunnelChartImpl::GetFunnelPoints(const CBCGPRect& rectFunnel, double dblDepth, const CBCGPSize& szNeckSize,
		double Y1, double Y2, CBCGPPoint& ptLeft, CBCGPPoint& ptRight)
{
	CBCGPRect rect = rectFunnel;
	CBCGPSize szNeck = szNeckSize;

	if (dblDepth >= 1.)
	{
		double dblDepth2 = dblDepth / 2.0;
		double nHeight = rect.Height();

		rect.DeflateRect(0.0, dblDepth2);

		double dblRatio = rect.Height() / nHeight;
		szNeck.cy *= dblRatio;
		Y1 *= dblRatio;
		Y2 *= dblRatio;

		nHeight = rect.Height();
	}

	Y1 += rect.top;
	Y2 += rect.top;

	rect.bottom -= szNeck.cy;

	double Y = (Y1 + Y2) / 2;

	if (Y >= rect.bottom)
	{
		ptLeft.x = rect.CenterPoint().x - szNeck.cx / 2;
		ptLeft.y = Y;

		ptRight.x = rect.CenterPoint().x + szNeck.cx / 2;
		ptRight.y = Y;

		return;
	}

	if (Y2 > rect.bottom)
	{
		ptLeft.x = rect.CenterPoint().x - szNeck.cx / 2;
		ptLeft.y = rect.bottom;

		ptRight.x = rect.CenterPoint().x + szNeck.cx / 2;
		ptRight.y = rect.bottom;
		return;
	}

	double dblNeckWidth2 = szNeck.cx / 2;
	double dblAngle = bcg_angle(rect.Width() / 2 - dblNeckWidth2, rect.Height());

	double yBottom = Y2;

	double dy       = (yBottom - Y1);
	double dx       = dy / tan(dblAngle);
	double dyBottom = rect.bottom - Y2;
	double dxBottom = dblNeckWidth2 + (dx * dyBottom / dy);
	double dxTop    = dxBottom + dx;
	double dx1 = (dxTop + dxBottom) / 2;

	ptLeft.x = rect.CenterPoint().x - dx1;
	ptLeft.y = Y;

	ptRight.x = rect.CenterPoint().x + dx1;
	ptRight.y = Y;
}

//****************************************************************************************
// Stock chart specific implementation
//****************************************************************************************
void CBCGPStockChartImpl::OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	UNREFERENCED_PARAMETER(rectDiagramArea);

	CBCGPBaseChartStockSeries* pBaseStockSeries = DYNAMIC_DOWNCAST(CBCGPBaseChartStockSeries, pSeries);

	if (!pBaseStockSeries->IsMainStockSeries())
	{
		return;
	}

	CBCGPChartStockSeries* pMainStockSeries = DYNAMIC_DOWNCAST(CBCGPChartStockSeries, pSeries);

	if (pMainStockSeries == NULL || !pMainStockSeries->IsMainStockSeries())
	{
		return;
	}

	if (pMainStockSeries->GetStockSeriesType() != CBCGPBaseChartStockSeries::SST_BAR && 
		pMainStockSeries->GetStockSeriesType() != CBCGPBaseChartStockSeries::SST_CANDLE)
	{
		CBCGPLineChartImpl::OnDrawDiagram(pGM, rectDiagramArea, pSeries);
		return;
	}

	// Get X axis
	CBCGPChartAxis* pXAxis = pSeries->GetRelatedAxis(CBCGPChartSeries::AI_X);

	if (pXAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(pXAxis);

	double dblXUnitSize = pXAxis->GetAxisUnitSize();

	if (pXAxis->IsFixedIntervalWidth())
	{
		dblXUnitSize = pXAxis->GetFixedIntervalWidthScaled() / pXAxis->GetValuesPerInterval();
	}
	else if (pXAxis->m_bFormatAsDate && !pXAxis->IsIndexedSeries())
	{
		dblXUnitSize *= pXAxis->GetMajorUnit() / 2;
	}

	double dblCandleWidth = floor((dblXUnitSize * (double)pMainStockSeries->m_nOpenCloseBarSizePercent / 100.) / 2.); 

	BOOL bCheckLineOffsets = CBCGPGraphicsManagerD2D::m_bCheckLineOffsets;

	if (dblCandleWidth < 0)
	{
		dblCandleWidth = 0;
		CBCGPGraphicsManagerD2D::m_bCheckLineOffsets = FALSE;
	}

	BOOL bIsAAEnabled = pGM->IsAntialiasingEnabled();
	pGM->EnableAntialiasing(FALSE);

	CArray<CBCGPChartDataPoint*, CBCGPChartDataPoint*> arDataPoints;
	arDataPoints.SetSize(4);

	for (int i = pMainStockSeries->GetMinDataPointIndex(); i <= pMainStockSeries->GetMaxDataPointIndex(); i++)
	{
		CBCGPBaseChartStockSeries* pCloseSeries = pMainStockSeries->GetChildSeries(CBCGPChartStockSeries::CHART_STOCK_SERIES_CLOSE_IDX);
		ASSERT_VALID(pCloseSeries);

		CBCGPBaseChartStockSeries* pHighSeries = pMainStockSeries->GetChildSeries(CBCGPChartStockSeries::CHART_STOCK_SERIES_HIGH_IDX);
		ASSERT_VALID(pHighSeries);

		CBCGPBaseChartStockSeries* pLowSeries = pMainStockSeries->GetChildSeries(CBCGPChartStockSeries::CHART_STOCK_SERIES_LOW_IDX);
		ASSERT_VALID(pLowSeries);

		const CBCGPChartDataPoint* pDpOpen = pMainStockSeries->GetDataPointAt(i);
		const CBCGPChartDataPoint* pDpClose = pCloseSeries->GetDataPointAt(i);
		const CBCGPChartDataPoint* pDpHigh = pHighSeries->GetDataPointAt(i);
		const CBCGPChartDataPoint* pDpLow = pLowSeries->GetDataPointAt(i);

		if (pDpOpen == NULL || pDpClose == NULL || pDpHigh == NULL || pDpLow == NULL || 
			pDpOpen->IsScreenPointsEmpty() || pDpClose->IsScreenPointsEmpty() || pDpHigh->IsScreenPointsEmpty() || 
			pDpLow->IsScreenPointsEmpty())
		{
			continue;
		}

		arDataPoints.SetAt(CBCGPChartStockSeries::STOCK_ARRAY_OPEN_IDX, (CBCGPChartDataPoint*)pDpOpen);
		arDataPoints.SetAt(CBCGPChartStockSeries::STOCK_ARRAY_HIGH_IDX, (CBCGPChartDataPoint*)pDpHigh);
		arDataPoints.SetAt(CBCGPChartStockSeries::STOCK_ARRAY_LOW_IDX, (CBCGPChartDataPoint*)pDpLow);
		arDataPoints.SetAt(CBCGPChartStockSeries::STOCK_ARRAY_CLOSE_IDX, (CBCGPChartDataPoint*)pDpClose);

		OnDrawStockDataPoint(pGM, arDataPoints, pMainStockSeries, i, dblCandleWidth, pXAxis->IsVertical());
	}

	if (dblCandleWidth < 0)
	{
		CBCGPGraphicsManagerD2D::m_bCheckLineOffsets = bCheckLineOffsets;
	}

	pGM->EnableAntialiasing(bIsAAEnabled);
}
//****************************************************************************************
void CBCGPStockChartImpl::OnDrawStockDataPoint(CBCGPGraphicsManager* pGM, 
		const CArray<CBCGPChartDataPoint*, CBCGPChartDataPoint*>& arDataPoints, 
		CBCGPChartStockSeries* pMainStockSeries, int nDataPointIndex, double dblCandleWidth, BOOL bIsVertical)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(pMainStockSeries);

	CBCGPPoint ptOpen = arDataPoints[CBCGPChartStockSeries::STOCK_ARRAY_OPEN_IDX]->GetScreenPoint();
	CBCGPPoint ptHigh = arDataPoints[CBCGPChartStockSeries::STOCK_ARRAY_HIGH_IDX]->GetScreenPoint();
	CBCGPPoint ptLow = arDataPoints[CBCGPChartStockSeries::STOCK_ARRAY_LOW_IDX]->GetScreenPoint();
	CBCGPPoint ptClose = arDataPoints[CBCGPChartStockSeries::STOCK_ARRAY_CLOSE_IDX]->GetScreenPoint();

	double dblOpenVal = arDataPoints[CBCGPChartStockSeries::STOCK_ARRAY_OPEN_IDX]->GetComponentValue(CBCGPChartData::CI_Y);
	double dblCloseVal = arDataPoints[CBCGPChartStockSeries::STOCK_ARRAY_CLOSE_IDX]->GetComponentValue(CBCGPChartData::CI_Y);

	if (pMainStockSeries->GetStockSeriesType() == CBCGPBaseChartStockSeries::SST_CANDLE)
	{
		CBCGPRect rectCandle;

		if (bIsVertical)
		{
			rectCandle.SetRect(ptOpen.x, ptOpen.y - dblCandleWidth, ptClose.x, ptOpen.y + dblCandleWidth);
		}
		else
		{
			rectCandle.SetRect(ptOpen.x - dblCandleWidth, ptOpen.y, ptOpen.x + dblCandleWidth, ptClose.y);
		}

		rectCandle.Normalize();

		OnDrawStockCandle(pGM, ptHigh, ptLow, rectCandle, 
			dblOpenVal < dblCloseVal ? pMainStockSeries->GetSeriesFormat().m_seriesElementFormat.m_outlineFormat : pMainStockSeries->m_downCandleStyle.m_outlineFormat,
			dblOpenVal < dblCloseVal ? pMainStockSeries->GetSeriesFormat().m_seriesElementFormat : pMainStockSeries->m_downCandleStyle, 
			pMainStockSeries, nDataPointIndex, dblOpenVal < dblCloseVal);
	}
	else
	{
		CBCGPPoint ptOpenStart;
		CBCGPPoint ptCloseStart;

		if (bIsVertical)
		{
			ptOpenStart.SetPoint(ptOpen.x, ptOpen.y - dblCandleWidth);
			ptCloseStart.SetPoint(ptClose.x, ptClose.y + dblCandleWidth);
		}
		else
		{
			ptOpenStart.SetPoint(ptOpen.x - dblCandleWidth, ptOpen.y);
			ptCloseStart.SetPoint(ptClose.x + dblCandleWidth, ptClose.y);
		}

		OnDrawStockBar(pGM, ptOpen, ptHigh, ptLow, ptClose, ptOpenStart, ptCloseStart, 
			dblOpenVal < dblCloseVal ? pMainStockSeries->GetSeriesFormat().m_seriesElementFormat.m_outlineFormat 
										: pMainStockSeries->m_downBarStyle, 
			pMainStockSeries, nDataPointIndex, dblOpenVal < dblCloseVal);
	}
}
//****************************************************************************************
void CBCGPStockChartImpl::OnDrawStockCandle(CBCGPGraphicsManager* pGM, CBCGPPoint& ptHigh, CBCGPPoint& ptLow, 
										CBCGPRect& rectCandle, const BCGPChartFormatLine& lineStyle, 
										const BCGPChartFormatArea& candleStyle, CBCGPChartStockSeries* pMainStockSeries, 
										int nDataPointIndex, BOOL bUp)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	BCGPSeriesColorsPtr seriesColors;

	if (!bUp)
	{
		seriesColors.m_pBrElementAltFillColor = (CBCGPBrush*)&candleStyle.m_brFillColor;
		seriesColors.m_pBrElementAltLineColor = (CBCGPBrush*)&lineStyle.m_brLineColor;
	}

	pMainStockSeries->GetColors(seriesColors, nDataPointIndex);

	CBCGPBrush* pBrFill = bUp ? seriesColors.m_pBrElementFillColor : 
										seriesColors.m_pBrElementAltFillColor;
	CBCGPBrush* pBrLine = bUp ? seriesColors.m_pBrElementLineColor : 
										seriesColors.m_pBrElementAltLineColor;

	
	pGM->DrawLine(ptHigh, ptLow, *pBrLine, candleStyle.m_outlineFormat.GetLineWidth(TRUE), 
		&candleStyle.m_outlineFormat.m_strokeStyle);

	if (!rectCandle.IsRectNull())
	{
		if (rectCandle.Width() > 1)
		{
			m_pRelatedChart->OnDrawChartRect(pGM, rectCandle, pBrFill, pBrLine, 
				candleStyle.m_outlineFormat.GetLineWidth(TRUE), &candleStyle.m_outlineFormat.m_strokeStyle, FALSE, FALSE);
		}
	}
	else
	{
		return;
	}

	CBCGPRect rectBounds(rectCandle.left, ptHigh.y, rectCandle.right, ptLow.y);
	pMainStockSeries->SetDataPointBoundingRect(nDataPointIndex, rectBounds.NormalizedRect());
}
//****************************************************************************************
void CBCGPStockChartImpl::OnDrawStockCandleEx(CBCGPGraphicsManager* pGM, CBCGPPoint& ptHigh, CBCGPPoint& ptLow, 
											CBCGPRect& rectCandle, const CBCGPBrush& brFill, 
											const BCGPChartFormatLine& lineStyle)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	
	
	pGM->DrawLine(ptHigh, ptLow, lineStyle.m_brLineColor, lineStyle.GetLineWidth(TRUE), 
		&lineStyle.m_strokeStyle);
	
	if (!rectCandle.IsRectNull())
	{
		if (rectCandle.Width() > 1)
		{
			m_pRelatedChart->OnDrawChartRect(pGM, rectCandle, &brFill, &lineStyle.m_brLineColor, 
				lineStyle.GetLineWidth(TRUE), &lineStyle.m_strokeStyle, FALSE, FALSE);
		}
	}
}
//****************************************************************************************
void CBCGPStockChartImpl::OnDrawStockBarEx(CBCGPGraphicsManager* pGM, CBCGPPoint& ptOpen, CBCGPPoint& ptHigh, 
									 CBCGPPoint& ptLow, CBCGPPoint& ptClose, 
									 CBCGPPoint& ptOpenStart, CBCGPPoint& ptCloseStart,
									 const BCGPChartFormatLine& lineStyle)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	pGM->DrawLine(ptHigh, ptLow, lineStyle.m_brLineColor, lineStyle.GetLineWidth(TRUE), &lineStyle.m_strokeStyle);
	pGM->DrawLine(ptOpenStart, ptOpen, lineStyle.m_brLineColor, lineStyle.GetLineWidth(TRUE), &lineStyle.m_strokeStyle);
	pGM->DrawLine(ptCloseStart, ptClose, lineStyle.m_brLineColor, lineStyle.GetLineWidth(TRUE), &lineStyle.m_strokeStyle);
}
//****************************************************************************************
void CBCGPStockChartImpl::OnDrawStockBar(CBCGPGraphicsManager* pGM, CBCGPPoint& ptOpen, CBCGPPoint& ptHigh, 
									 CBCGPPoint& ptLow, CBCGPPoint& ptClose, 
									 CBCGPPoint& ptOpenStart, CBCGPPoint& ptCloseStart,
									 const BCGPChartFormatLine& barStyle,
									 CBCGPChartStockSeries* pMainStockSeries, int nDataPointIndex, 
									 BOOL bUp)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	BCGPSeriesColorsPtr seriesColors;

	if (!bUp)
	{
		seriesColors.m_pBrElementAltLineColor = (CBCGPBrush*)&barStyle.m_brLineColor;
	}

	pMainStockSeries->GetColors(seriesColors, nDataPointIndex);

	const CBCGPBrush& brLine = bUp ? *seriesColors.m_pBrElementLineColor : 
		*seriesColors.m_pBrElementAltLineColor;

	pGM->DrawLine(ptHigh, ptLow, brLine, barStyle.GetLineWidth(TRUE), &barStyle.m_strokeStyle);
	pGM->DrawLine(ptOpenStart, ptOpen, brLine, barStyle.GetLineWidth(TRUE), &barStyle.m_strokeStyle);
	pGM->DrawLine(ptCloseStart, ptClose, brLine, barStyle.GetLineWidth(TRUE), &barStyle.m_strokeStyle);

	CBCGPRect rectBounds(ptOpenStart.x, ptHigh.y, ptCloseStart.x, ptLow.y);
	pMainStockSeries->SetDataPointBoundingRect(nDataPointIndex, rectBounds.NormalizedRect());
}
//****************************************************************************************
void CBCGPStockChartImpl::OnCalcBoundingRect(CBCGPChartDataPoint* /*pDataPoint*/, const CBCGPRect& /*rectDiagramArea*/, 
		CBCGPChartSeries* /*pSeries*/, int /*nDataPointIndex*/)
{

}
//****************************************************************************************
CBCGPSize CBCGPStockChartImpl::OnCalcLegendKeySize(CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	UNREFERENCED_PARAMETER(pSeries);
	UNREFERENCED_PARAMETER(nDataPointIndex);
	
	if (m_pRelatedChart == NULL)
	{
		return CBCGPSize(20, 20);
	}

	CBCGPChartStockSeries* pStockSeries = DYNAMIC_DOWNCAST(CBCGPChartStockSeries, pSeries);
	
	if (pStockSeries == NULL)
	{
		return m_pRelatedChart->GetScaleRatio() * 20;
	}
	
	if (pStockSeries->GetStockSeriesType() != CBCGPBaseChartStockSeries::SST_BAR && 
		pStockSeries->GetStockSeriesType() != CBCGPBaseChartStockSeries::SST_CANDLE)
	{
		return CBCGPLineChartImpl::OnCalcLegendKeySize(pSeries, nDataPointIndex);
	}
	
	return m_pRelatedChart->GetScaleRatio() * 20.;
}
//****************************************************************************************
CBCGPSize CBCGPStockChartImpl::OnCalcLegendKeySizeEx(const BCGPChartCellParams& params, CBCGPChartSeries* pSeries)
{
	CBCGPChartStockSeries* pStockSeries = DYNAMIC_DOWNCAST(CBCGPChartStockSeries, pSeries);
	
	if (pStockSeries == NULL)
	{
		return params.GetScaleRatio() * 20.;
	}
	
	if (pStockSeries->GetStockSeriesType() != CBCGPBaseChartStockSeries::SST_BAR && 
		pStockSeries->GetStockSeriesType() != CBCGPBaseChartStockSeries::SST_CANDLE)
	{
		return CBCGPLineChartImpl::OnCalcLegendKeySizeEx(params, pSeries);
	}
	
	return params.GetScaleRatio() * 20.;
}
//****************************************************************************************
void CBCGPStockChartImpl::OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
						  const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(pSeries);

	CBCGPChartStockSeries* pStockSeries = DYNAMIC_DOWNCAST(CBCGPChartStockSeries, pSeries);

	if (pStockSeries == NULL)
	{
		return;
	}

	if (pStockSeries->GetStockSeriesType() != CBCGPBaseChartStockSeries::SST_BAR && 
		pStockSeries->GetStockSeriesType() != CBCGPBaseChartStockSeries::SST_CANDLE)
	{
		CBCGPLineChartImpl::OnDrawChartLegendKey(pGM, rectLegendKey, pSeriesStyle, pSeries, nDataPointIndex);
		return;
	}

	BOOL bIsAAEnabled = pGM->IsAntialiasingEnabled();
	pGM->EnableAntialiasing(FALSE);

	CBCGPPoint ptHigh(rectLegendKey.CenterPoint().x, rectLegendKey.top);
	CBCGPPoint ptLow(rectLegendKey.CenterPoint().x, rectLegendKey.bottom);
	CBCGPPoint ptOpen(rectLegendKey.CenterPoint().x, rectLegendKey.top + rectLegendKey.Height() / 4);
	CBCGPPoint ptClose (rectLegendKey.CenterPoint().x, rectLegendKey.bottom - rectLegendKey.Height() / 4);

	double dblCandleWidth = rectLegendKey.Width() / 4;

	switch(pStockSeries->GetStockSeriesType())
	{
	case CBCGPBaseChartStockSeries::SST_BAR:
		{
			CBCGPPoint ptOpenStart (ptOpen.x - dblCandleWidth, ptOpen.y);
			CBCGPPoint ptCloseStart (ptClose.x + dblCandleWidth, ptClose.y);
			OnDrawStockBar(pGM, ptOpen, ptHigh, ptLow, ptClose, ptOpenStart, ptCloseStart, 
				pStockSeries->GetSeriesFormat().m_seriesElementFormat.m_outlineFormat, 
				pStockSeries, nDataPointIndex, TRUE);
		}
		break;

	case CBCGPBaseChartStockSeries::SST_CANDLE:
		{
			CBCGPRect rectCandle;
			rectCandle.SetRect(ptOpen.x - dblCandleWidth, ptOpen.y, ptOpen.x + dblCandleWidth, ptClose.y);

			OnDrawStockCandle(pGM, ptHigh, ptLow, rectCandle, 
				pStockSeries->GetSeriesFormat().m_seriesElementFormat.m_outlineFormat,
				pStockSeries->GetSeriesFormat().m_seriesElementFormat, 
				pStockSeries, nDataPointIndex, TRUE);
		}
		break;
	}

	pGM->EnableAntialiasing(bIsAAEnabled);
}
//****************************************************************************************
void CBCGPStockChartImpl::OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
												   const BCGPChartCellParams& params, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT_VALID(pSeries);
	
	CBCGPChartStockSeries* pStockSeries = DYNAMIC_DOWNCAST(CBCGPChartStockSeries, pSeries);
	
	if (pStockSeries == NULL)
	{
		return;
	}
	
	if (pStockSeries->GetStockSeriesType() != CBCGPBaseChartStockSeries::SST_BAR && 
		pStockSeries->GetStockSeriesType() != CBCGPBaseChartStockSeries::SST_CANDLE)
	{
		CBCGPLineChartImpl::OnDrawChartLegendKeyEx(pGM, rectLegendKey, params, pSeries) ;
		return;
	}

	BOOL bIsAAEnabled = pGM->IsAntialiasingEnabled();
	pGM->EnableAntialiasing(FALSE);
	
	CBCGPPoint ptHigh(rectLegendKey.CenterPoint().x, rectLegendKey.top);
	CBCGPPoint ptLow(rectLegendKey.CenterPoint().x, rectLegendKey.bottom);
	CBCGPPoint ptOpen(rectLegendKey.CenterPoint().x, rectLegendKey.top + rectLegendKey.Height() / 4);
	CBCGPPoint ptClose (rectLegendKey.CenterPoint().x, rectLegendKey.bottom - rectLegendKey.Height() / 4);
	
	double dblCandleWidth = rectLegendKey.Width() / 4;
	
	switch(pStockSeries->GetStockSeriesType())
	{
	case CBCGPBaseChartStockSeries::SST_BAR:
		{
			CBCGPPoint ptOpenStart (ptOpen.x - dblCandleWidth, ptOpen.y);
			CBCGPPoint ptCloseStart (ptClose.x + dblCandleWidth, ptClose.y);
			OnDrawStockBarEx(pGM, ptOpen, ptHigh, ptLow, ptClose, ptOpenStart, ptCloseStart, params.m_lineStyle);
		}
		break;
		
	case CBCGPBaseChartStockSeries::SST_CANDLE:
		{
			CBCGPRect rectCandle;
			rectCandle.SetRect(ptOpen.x - dblCandleWidth, ptOpen.y, ptOpen.x + dblCandleWidth, ptClose.y);
			
			OnDrawStockCandleEx(pGM, ptHigh, ptLow, rectCandle, params.m_brFill, params.m_lineStyle);
		}
		break;
	}
	
	pGM->EnableAntialiasing(bIsAAEnabled);
}

//****************************************************************************************
// LARGE DATA chart specific implementation
//****************************************************************************************

void CBCGPLongDataChartImpl::OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagramArea*/, CBCGPChartSeries* pSeries)
{
	CBCGPChartLongSeries* pLongSeries = DYNAMIC_DOWNCAST(CBCGPChartLongSeries, pSeries);

	if (pLongSeries == NULL)
	{
		return;
	}

	ASSERT_VALID(pLongSeries);

	BCGPSeriesColorsPtr colors;
	const BCGPChartFormatSeries* pFormatSeries = pSeries->GetColors(colors, -1);

	if (pFormatSeries == NULL)
	{
		return;
	}

	if (pLongSeries->IsScatterMode())
	{
		if (colors.m_pBrElementFillColor == NULL)
		{
			return;
		}

		double dblSize = pLongSeries->GetScatterPointSize();
		if (m_pRelatedChart != NULL)
		{
			CBCGPSize szScale = m_pRelatedChart->GetScaleRatio();
			dblSize *= max(szScale.cx, szScale.cy);
		}

		pGM->DrawScatter(pLongSeries->GetScreenPoints(), *colors.m_pBrElementFillColor, dblSize);
	}
	else
	{
		if (colors.m_pBrElementLineColor == NULL)
		{
			return;
		}

		BOOL bOldCheck = CBCGPGraphicsManagerD2D::m_bCheckLineOffsets;
		CBCGPGraphicsManagerD2D::m_bCheckLineOffsets = FALSE;
		
		pGM->DrawLines(pLongSeries->GetScreenPoints(), *colors.m_pBrElementLineColor, 
			pFormatSeries->m_seriesElementFormat.m_outlineFormat.GetLineWidth(TRUE), 
			&pFormatSeries->m_seriesElementFormat.m_outlineFormat.m_strokeStyle);
		
		CBCGPGraphicsManagerD2D::m_bCheckLineOffsets = bOldCheck;
	}
}
//****************************************************************************************
void CBCGPLongDataChartImpl::OnCalcScreenPositions(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	pSeries->OnCalcScreenPoints(pGM, rectDiagramArea);
}
//****************************************************************************************
void CBCGPLongDataChartImpl::OnDrawDiagramMarkers(CBCGPGraphicsManager* /*pGM*/, CBCGPChartSeries* /*pSeries*/, const CBCGPRect& /*rectDiagram*/)
{

}
//****************************************************************************************
void CBCGPLongDataChartImpl::OnDrawDiagramDataLabels(CBCGPGraphicsManager* /*pGM*/, CBCGPChartSeries* /*pSeries*/)
{

}
//****************************************************************************************
CBCGPSize CBCGPLongDataChartImpl::OnCalcLegendKeySize(CBCGPChartSeries* /*pSeries*/, int /*nDataPointIndex*/)
{
	if (m_pRelatedChart == NULL)
	{
		return CBCGPSize(nDefaultLegendKeyWidth, nDefaultLegendKeyHeight);
	}

	CBCGPSize szScale = m_pRelatedChart->GetScaleRatio();
	return  CBCGPSize(szScale.cx * nDefaultLegendKeyWidth, szScale.cy * nDefaultLegendKeyHeight);
}
//****************************************************************************************
CBCGPSize CBCGPLongDataChartImpl::OnCalcLegendKeySizeEx(const BCGPChartCellParams& params, CBCGPChartSeries* /*pSeries*/)
{
	CBCGPSize szScale = params.GetScaleRatio();
	return  CBCGPSize(szScale.cx * nDefaultLegendKeyWidth, szScale.cy * nDefaultLegendKeyHeight);
}
//****************************************************************************************
void CBCGPLongDataChartImpl::OnDrawChartLegendKey(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey, 
								  const BCGPChartFormatSeries* pSeriesStyle, CBCGPChartSeries* pSeries, int nDataPointIndex)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);
	ASSERT(pSeriesStyle != NULL);

	UNREFERENCED_PARAMETER(nDataPointIndex);

	CBCGPPoint ptLineStart (rectLegendKey.left, rectLegendKey.CenterPoint().y);
	CBCGPPoint ptLineEnd (rectLegendKey.right, rectLegendKey.CenterPoint().y);

	BCGPChartFormatLine lineStyle = pSeriesStyle->m_seriesElementFormat.m_outlineFormat;

	if (lineStyle.GetLineWidth(TRUE) >= rectLegendKey.Height())
	{
		lineStyle.m_dblWidth = rectLegendKey.Height() / max(lineStyle.GetScaleRatio().cx, lineStyle.GetScaleRatio().cy) - 2;
	}

	BCGPSeriesColorsPtr colors;
	const BCGPChartFormatSeries* pFormatSeries = pSeries->GetColors(colors, -1);
	if (pFormatSeries == NULL || colors.m_pBrElementLineColor == NULL)
	{
		return;
	}

	pGM->DrawLine(ptLineStart, ptLineEnd, *colors.m_pBrElementLineColor, lineStyle.GetLineWidth(TRUE), &lineStyle.m_strokeStyle);
}
//****************************************************************************************
void CBCGPLongDataChartImpl::OnDrawChartLegendKeyEx(CBCGPGraphicsManager* pGM, const CBCGPRect& rectLegendKey,
		const BCGPChartCellParams& params, CBCGPChartSeries* /*pSeries*/)
{
	ASSERT_VALID(this);
	
	CBCGPPoint ptLineStart (rectLegendKey.left, rectLegendKey.CenterPoint().y);
	CBCGPPoint ptLineEnd (rectLegendKey.right, rectLegendKey.CenterPoint().y);
	
	CBCGPSize szScaleRatio = params.m_lineStyle.GetScaleRatio();
	
	double dblLineWidth = bcg_clamp(params.m_lineStyle.m_dblWidth, 1., 7. * max(szScaleRatio.cx, szScaleRatio.cy));
	pGM->DrawLine(ptLineStart, ptLineEnd, params.m_lineStyle.m_brLineColor, dblLineWidth, &params.m_lineStyle.m_strokeStyle);
}
//****************************************************************************************
// POLAR CHART specific implementation 
//****************************************************************************************
void CBCGPPolarChartImpl::OnDrawDiagram(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagramArea, CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pSeries);

	CBCGPChartPolarSeries* pPolarSeries = DYNAMIC_DOWNCAST(CBCGPChartPolarSeries, pSeries);

	if (pPolarSeries == NULL || m_pRelatedChart == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pRelatedChart);

	BCGPChartFormatSeries::ChartCurveType curveType = pPolarSeries->GetCurveType();

	BOOL bFillArea = pPolarSeries->IsFillClosedShape();
	int nPointCount = pPolarSeries->GetRosePointCount();
	int nDataPointCount = pPolarSeries->GetDataPointCount();

	if (curveType == BCGPChartFormatSeries::CCT_NO_LINE ||
		curveType == BCGPChartFormatSeries::CCT_STEP || 
		curveType == BCGPChartFormatSeries::CCT_REVERSED_STEP || 
		nPointCount == 1 || nDataPointCount <= nPointCount)
	{
		CBCGPLineChartImpl::OnDrawDiagram(pGM, rectDiagramArea, pSeries);
		return;
	}
	
	CBCGPChartAxisPolarY* pAxisY = DYNAMIC_DOWNCAST(CBCGPChartAxisPolarY, m_pRelatedChart->GetChartAxis(BCGP_CHART_Y_POLAR_AXIS));

	if (pAxisY == NULL)
	{
		return;
	}

	CBCGPSplineGeometry::BCGP_SPLINE_TYPE splineType = CBCGPSplineGeometry::BCGP_SPLINE_TYPE_KB;

	if (curveType == BCGPChartFormatSeries::CCT_SPLINE_HERMITE)
	{
		splineType = CBCGPSplineGeometry::BCGP_SPLINE_TYPE_HERMITE;
	}

	ASSERT_VALID(pAxisY);

	CBCGPPoint ptStartAxis;
	CBCGPPoint ptEndAxis;

	pAxisY->GetAxisPos(ptStartAxis, ptEndAxis);

	for (int i = 0; i < nDataPointCount; i += nPointCount)
	{
		CBCGPPointsArray arPoints;
		arPoints.Add(ptStartAxis);

		for (int j = i, nCount = 0; nCount < nPointCount; j++, nCount++)
		{
			if (j == nDataPointCount)
			{
				break;
			}

			if (pSeries->IsDataPointScreenPointsEmpty(j))
			{
				continue;
			}

			CBCGPPoint ptScreen = pPolarSeries->GetDataPointScreenPoint(j, 0);
			arPoints.Add(ptScreen);
		}
		
		if (arPoints.GetSize() >= 3 && 
			(curveType == BCGPChartFormatSeries::CCT_SPLINE_HERMITE || 
				curveType == BCGPChartFormatSeries::CCT_SPLINE))
		{
			CBCGPSplineGeometry splineGeometry(arPoints, splineType, TRUE);
			m_pRelatedChart->OnDrawChartSeriesItem(pGM, splineGeometry, pSeries, -1, 
				CBCGPChartVisualObject::CE_MAIN_ELEMENT, !bFillArea, FALSE);
		}
		else
		{
			CBCGPPolygonGeometry geometry(arPoints, TRUE);
			m_pRelatedChart->OnDrawChartSeriesItem(pGM, geometry, pSeries, -1, 
				CBCGPChartVisualObject::CE_MAIN_ELEMENT, !bFillArea, FALSE);
		}
	}

}
