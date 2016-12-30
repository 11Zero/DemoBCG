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

#include "stdafx.h"
#include "BCGPChartObject.h"
#include "BCGPChartVisualObject.h"
#include "BCGPChartAxis.h"
#include "BCGPGraphicsManagerGDI.h"
#include "BCGPMath.h"
#include "BCGPChartSeries.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const double CBCGPChartObject::_EmptyCoordinate = DBL_MAX;
const CBCGPPoint CBCGPChartObject::_EmptyPoint(CBCGPChartObject::_EmptyCoordinate, CBCGPChartObject::_EmptyCoordinate);
const CBCGPRect CBCGPChartObject::_EmptyRect(CBCGPChartObject::_EmptyPoint, CBCGPChartObject::_EmptyPoint);


IMPLEMENT_DYNCREATE(CBCGPChartObject, CObject)
IMPLEMENT_DYNCREATE(CBCGPChartLineObject, CBCGPChartObject)
IMPLEMENT_DYNCREATE(CBCGPChartRangeObject, CBCGPChartObject)
IMPLEMENT_DYNCREATE(CBCGPChartTextObject, CBCGPChartObject)
IMPLEMENT_DYNCREATE(CBCGPChartAxisMarkObject, CBCGPChartObject)

//*******************************************************************************
CBCGPChartObject::CBCGPChartObject()
{
	CommonInit();
}
//*******************************************************************************
CBCGPChartObject::CBCGPChartObject(CBCGPChartVisualObject* pParentChart, 
		const CBCGPRect& rcCoordinates, CoordinateMode mode)
{
	CommonInit();

	m_pParentChart = pParentChart;

	SetCoordinates(rcCoordinates, mode);
}
//*******************************************************************************
void CBCGPChartObject::CommonInit()
{
	m_pParentChart = NULL;

	m_pXAxis = NULL;
	m_pYAxis = NULL;

	m_rectScreen.SetRectEmpty();
	m_rectCoordinates = CBCGPChartObject::_EmptyRect;
	m_szObjectSize.SetSizeEmpty();

	m_nObjectID = -1;

	m_bVisible = TRUE;
	m_bIsForeground = TRUE;

	m_dblShadowDepth = 0.;

	m_brShadow.SetColor(CBCGPColor(.1, .1, .1, .1));
}
//*******************************************************************************
void CBCGPChartObject::SetCoordinates(double dblLeft, double dblTop, double dblRight, double dblBottom, 
									CBCGPChartObject::CoordinateMode mode)
{
	ASSERT_VALID(this);

	if (dblLeft != CBCGPChartObject::_EmptyCoordinate)
	{
		m_rectCoordinates.left = dblLeft;
	}

	if (dblTop != CBCGPChartObject::_EmptyCoordinate)
	{
		m_rectCoordinates.top = dblTop;
	}

	if (dblRight != CBCGPChartObject::_EmptyCoordinate)
	{
		m_rectCoordinates.right = dblRight;
	}

	if (dblBottom != CBCGPChartObject::_EmptyCoordinate)
	{
		m_rectCoordinates.bottom = dblBottom;
	}

	m_coordinateMode = mode;
}
//*******************************************************************************
void CBCGPChartObject::SetCoordinates(const CBCGPRect& rcCoordinates, CBCGPChartObject::CoordinateMode mode)
{
	ASSERT_VALID(this);

	SetCoordinates(rcCoordinates.left, rcCoordinates.top, rcCoordinates.right, rcCoordinates.bottom, mode);
}
//*******************************************************************************
void CBCGPChartObject::SetCoordinates(const CBCGPPoint& ptLeftTop, const CBCGPPoint& ptRightBottom, 
									  CBCGPChartObject::CoordinateMode mode)
{
	ASSERT_VALID(this);

	SetCoordinates(ptLeftTop.x, ptLeftTop.y, ptRightBottom.x, ptRightBottom.y, mode);
}
//*******************************************************************************
void CBCGPChartObject::SetAxisMarkCoordinate(double dblVal, BOOL bIsVert, BOOL bOutside)
{
	bIsVert ? m_rectCoordinates.top = dblVal : m_rectCoordinates.left = dblVal;
	m_coordinateMode = bOutside ? CM_AXIS_OUTSIDE_MARK : CM_AXIS_INSIDE_MARK;
}
//*******************************************************************************
void CBCGPChartObject::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld)
{
	m_format.OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);
}
//*******************************************************************************
void CBCGPChartObject::SetParentChart(CBCGPChartVisualObject* pChart)
{
	ASSERT_VALID(this);

	m_pParentChart = pChart;

	if (m_pParentChart != NULL)
	{
		ASSERT_VALID(m_pParentChart);
	}
}
//*******************************************************************************
void CBCGPChartObject::SetRelatedAxes(CBCGPChartAxis* pXAxis, CBCGPChartAxis* pYAxis)
{
	ASSERT_VALID(this);

	m_pXAxis = pXAxis;
	m_pYAxis = pYAxis;

	if (m_pXAxis != NULL)
	{
		ASSERT_VALID(m_pXAxis);
	}

	if (m_pYAxis != NULL)
	{
		ASSERT_VALID(m_pYAxis);
	}
}
//*******************************************************************************
BOOL CBCGPChartObject::IsObjectShownOnAxis(CBCGPChartAxis* pAxis) const
{
	return (m_pXAxis == pAxis) || (m_pYAxis == pAxis);
}
//*******************************************************************************
void CBCGPChartObject::SetObjectSize(const CBCGPSize sz, BOOL bRedraw)
{
	ASSERT_VALID(this);

	m_szObjectSize = sz;

	if (bRedraw && m_pParentChart != NULL)
	{
		ASSERT_VALID(m_pParentChart);

		m_pParentChart->SetDirty(TRUE, TRUE);
	}
}
//*******************************************************************************
void CBCGPChartObject::SetForeground(BOOL bFore, BOOL bRedraw)
{
	ASSERT_VALID(this);

	m_bIsForeground = bFore;

	if (bRedraw && m_pParentChart != NULL)
	{
		ASSERT_VALID(m_pParentChart);

		m_pParentChart->SetDirty(TRUE, TRUE);
	}
}
//*******************************************************************************
BOOL CBCGPChartObject::IsRectOffsetsValid() const
{
	return m_rectCoordinates.left != CBCGPChartObject::_EmptyCoordinate || 
			m_rectCoordinates.top != CBCGPChartObject::_EmptyCoordinate || 
			m_rectCoordinates.right != CBCGPChartObject::_EmptyCoordinate || 
			m_rectCoordinates.bottom != CBCGPChartObject::_EmptyCoordinate;
}
//*******************************************************************************
BOOL CBCGPChartObject::HitTest(const CBCGPPoint& pt) const
{
	return m_rectScreen.NormalizedRect().PtInRect(pt);
}
//*******************************************************************************
CBCGPRect CBCGPChartObject::OnCalcBoundingRect()
{
	if (m_pXAxis == NULL && m_pYAxis == NULL || m_coordinateMode == CM_AXIS_OUTSIDE_MARK || m_coordinateMode == CM_AXIS_INSIDE_MARK ||
		m_pXAxis != NULL && !m_pXAxis->m_bInitialized)
	{
		return m_pParentChart->GetRect();
	}

	CBCGPRect rectXAxis;
	CBCGPRect rectYAxis;

	if (m_pXAxis != NULL)
	{
		rectXAxis = m_pXAxis->GetBoundingRect();
	}

	if (m_pYAxis != NULL)
	{
		rectYAxis = m_pYAxis->GetBoundingRect();
	}

	if (!rectXAxis.IsRectEmpty() && !rectYAxis.IsRectEmpty())
	{
		rectXAxis.IntersectRect(rectYAxis);
		if (rectXAxis.IsRectEmpty())
		{
			return rectYAxis;
		}
	}
	else if (rectXAxis.IsRectEmpty())
	{
		return rectYAxis;
	}

	return rectXAxis;
}
//*******************************************************************************
CBCGPSize CBCGPChartObject::OnCalcObjectSize(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);

	if (m_pParentChart != NULL && !m_strText.IsEmpty())
	{
		ASSERT_VALID(m_pParentChart);

		if (m_format.m_textFormat.IsWordWrap())
		{
			if (m_szObjectSize.cx == 0)
			{
				return CBCGPSize();
			}

			return m_pParentChart->OnGetTextSize(pGM, m_strText, m_format.m_textFormat, m_szObjectSize.cx) + m_format.GetContentPadding(TRUE) * 2;
		}

		return m_pParentChart->OnGetTextSize(pGM, m_strText, m_format.m_textFormat) + m_format.GetContentPadding(TRUE) * 2;
	}
	return CBCGPSize();
}
//*******************************************************************************
void CBCGPChartObject::OnCalcScreenPoints(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);

	m_rectScreen.SetRectEmpty();
	m_rectScreenBounds.SetRectEmpty();
	m_ptAnchor.x = m_ptAnchor.y = 0.;

	if (m_pParentChart == NULL)
	{
		return;
	}

	CBCGPSize szScaleRatio = m_pParentChart->GetScaleRatio();

	CBCGPChartAxis* pXAxis = m_pXAxis;
	CBCGPChartAxis* pYAxis = m_pYAxis;

	if (m_coordinateMode == CBCGPChartObject::CM_CHART_VALUES || 
		m_coordinateMode == CBCGPChartObject::CM_CHART_VALUE_DIST_ANGLE ||
		m_coordinateMode == CBCGPChartObject::CM_AXIS_INSIDE_MARK ||
		m_coordinateMode == CBCGPChartObject::CM_AXIS_OUTSIDE_MARK)
	{
		if (pXAxis == NULL)
		{
			m_pXAxis = pXAxis = m_pParentChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS);
		}

		if (pYAxis == NULL)
		{
			m_pYAxis = pYAxis = m_pParentChart->GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS);
		}
	}

	CBCGPSize szObjectSize = m_szObjectSize.IsNull() ?  OnCalcObjectSize(pGM) : m_szObjectSize;

	m_rectScreenBounds = OnCalcBoundingRect();

	CBCGPRect rectBounds = m_rectScreenBounds;

	CBCGPPoint ptLeftTop(CBCGPChartObject::_EmptyPoint);
	CBCGPPoint ptRightBottom(CBCGPChartObject::_EmptyPoint);

	if (m_coordinateMode == CBCGPChartObject::CM_PERCENTS)
	{
		if (m_rectCoordinates.left != CBCGPChartObject::_EmptyCoordinate)
		{
			ptLeftTop.x = rectBounds.left + rectBounds.Width() * m_rectCoordinates.left / 100.;
		}

		if (m_rectCoordinates.top != CBCGPChartObject::_EmptyCoordinate)
		{
			ptLeftTop.y = rectBounds.top + rectBounds.Height() * m_rectCoordinates.top / 100;
		}

		if (m_rectCoordinates.right != CBCGPChartObject::_EmptyCoordinate)
		{
			ptRightBottom.x = rectBounds.right - rectBounds.Width() * m_rectCoordinates.right / 100.;
		}

		if (m_rectCoordinates.bottom != CBCGPChartObject::_EmptyCoordinate)
		{
			ptRightBottom.y = rectBounds.bottom - rectBounds.Height() * m_rectCoordinates.bottom / 100.;
		}
	}
	else if (m_coordinateMode == CBCGPChartObject::CM_PIXELS)
	{
		if (m_rectCoordinates.left != CBCGPChartObject::_EmptyCoordinate)
		{
			ptLeftTop.x = rectBounds.left + m_rectCoordinates.left * szScaleRatio.cx;
		}

		if (m_rectCoordinates.top != CBCGPChartObject::_EmptyCoordinate)
		{
			ptLeftTop.y = rectBounds.top + m_rectCoordinates.top * szScaleRatio.cy;
		}

		if (m_rectCoordinates.right != CBCGPChartObject::_EmptyCoordinate)
		{
			ptRightBottom.x = rectBounds.right - m_rectCoordinates.right * szScaleRatio.cx;
		}

		if (m_rectCoordinates.bottom != CBCGPChartObject::_EmptyCoordinate)
		{
			ptRightBottom.y = rectBounds.bottom - m_rectCoordinates.bottom * szScaleRatio.cy;
		}
	}
	else if (m_coordinateMode == CBCGPChartObject::CM_PIXELS_FIXED_SIZE)
	{
		if (m_rectCoordinates.left != CBCGPChartObject::_EmptyCoordinate)
		{
			ptLeftTop.x = rectBounds.left + m_rectCoordinates.left * szScaleRatio.cx;
		}
		else
		{
			ptLeftTop.x = rectBounds.left;
		}

		if (m_rectCoordinates.top != CBCGPChartObject::_EmptyCoordinate)
		{
			ptLeftTop.y = rectBounds.top + m_rectCoordinates.top * szScaleRatio.cy;
		}
		else
		{
			ptLeftTop.y = rectBounds.top;
		}

		if (m_rectCoordinates.right != CBCGPChartObject::_EmptyCoordinate)
		{
			ptRightBottom.x = ptLeftTop.x + m_rectCoordinates.right * szScaleRatio.cx;
		}
		else
		{
			ptRightBottom.x = ptLeftTop.x + rectBounds.Width();
		}

		if (m_rectCoordinates.bottom != CBCGPChartObject::_EmptyCoordinate)
		{
			ptRightBottom.y = ptLeftTop.y + m_rectCoordinates.bottom * szScaleRatio.cy;
		}
		else
		{
			ptRightBottom.y = ptLeftTop.y + rectBounds.Height();
		}
	}
	else if (m_coordinateMode == CBCGPChartObject::CM_CHART_VALUES)
	{
		ASSERT_VALID(pXAxis);
		ASSERT_VALID(pYAxis);

		CBCGPRect rectCoordinates = m_rectCoordinates;

		if (pXAxis != NULL && pXAxis->IsVertical())
		{
			rectCoordinates = CBCGPRect(m_rectCoordinates.bottom, m_rectCoordinates.right,
										m_rectCoordinates.top, m_rectCoordinates.left);
			pXAxis = m_pYAxis;
			pYAxis = m_pXAxis;
		}

 		if (pXAxis->m_bReverseOrder && !pXAxis->IsVertical())
 		{
			if ((rectCoordinates.left != CBCGPChartObject::_EmptyCoordinate && 
				 rectCoordinates.right == CBCGPChartObject::_EmptyCoordinate) || 
				 (rectCoordinates.left == CBCGPChartObject::_EmptyCoordinate && 
				 rectCoordinates.right != CBCGPChartObject::_EmptyCoordinate))
			{
				rectCoordinates.SwapLeftRight();
			}
 		}
 
 		if (pYAxis->m_bReverseOrder && pYAxis->IsVertical())
 		{
			if ((rectCoordinates.top != CBCGPChartObject::_EmptyCoordinate && 
				 rectCoordinates.bottom == CBCGPChartObject::_EmptyCoordinate) || 
				 (rectCoordinates.top == CBCGPChartObject::_EmptyCoordinate && 
				 rectCoordinates.bottom != CBCGPChartObject::_EmptyCoordinate))
			{
				rectCoordinates.SwapTopBottom();
			}
 		}

		if (rectCoordinates.left != CBCGPChartObject::_EmptyCoordinate)
		{
			ptLeftTop.x = pXAxis->PointFromValue(rectCoordinates.left, FALSE);
		}
		else if (szObjectSize.cx == 0)
		{
			ptLeftTop.x = pXAxis->PointFromValue(pXAxis->GetMinDisplayedValue(TRUE), TRUE);
		}

		if (rectCoordinates.top != CBCGPChartObject::_EmptyCoordinate)
		{
			ptLeftTop.y = pYAxis->PointFromValue(rectCoordinates.top, FALSE);
		}
		else if (szObjectSize.cy == 0)
		{
			ptLeftTop.y = pYAxis->PointFromValue(pYAxis->GetMaxDisplayedValue(TRUE), TRUE);
		}

		if (rectCoordinates.right != CBCGPChartObject::_EmptyCoordinate)
		{
			ptRightBottom.x = pXAxis->PointFromValue(rectCoordinates.right, FALSE);
		}
		else if (szObjectSize.cx == 0)
		{
			ptRightBottom.x = pXAxis->PointFromValue(pXAxis->GetMaxDisplayedValue(TRUE), TRUE);
		}

		if (rectCoordinates.bottom != CBCGPChartObject::_EmptyCoordinate)
		{
			ptRightBottom.y = pYAxis->PointFromValue(rectCoordinates.bottom, FALSE);
		}
		else if (szObjectSize.cy == 0)
		{
			ptRightBottom.y = pYAxis->PointFromValue(pYAxis->GetMinDisplayedValue(TRUE), TRUE);
		}
	}
	else if (m_coordinateMode == CM_CHART_VALUE_DIST_ANGLE)
	{
		ASSERT_VALID(pXAxis);
		ASSERT_VALID(pYAxis);

		if (!IsRectOffsetsValid())
		{
			return;
		}

		CBCGPRect rectCoordinates = m_rectCoordinates;

		if (pXAxis != NULL && pXAxis->IsVertical())
		{
			rectCoordinates = CBCGPRect(m_rectCoordinates.top, m_rectCoordinates.left,
										m_rectCoordinates.right, m_rectCoordinates.bottom);
			pXAxis = m_pYAxis;
			pYAxis = m_pXAxis;
		}

		m_ptAnchor.x = pXAxis->PointFromValue(rectCoordinates.left, FALSE);
		m_ptAnchor.y = pYAxis->PointFromValue(rectCoordinates.top, FALSE); 

		double dblDistanceX = rectCoordinates.right * szScaleRatio.cx;
		double dblDistanceY = rectCoordinates.right * szScaleRatio.cy;

		CBCGPPoint ptCenter(m_ptAnchor.x + dblDistanceX * sin(bcg_deg2rad(rectCoordinates.bottom)),
							m_ptAnchor.y - dblDistanceY * cos(bcg_deg2rad(rectCoordinates.bottom)));

		ptLeftTop.SetPoint(ptCenter.x - szObjectSize.cx / 2, ptCenter.y - szObjectSize.cy / 2);
		ptRightBottom.SetPoint(ptCenter.x + szObjectSize.cx / 2, ptCenter.y + szObjectSize.cy / 2);
	}
	else if (m_coordinateMode == CM_AXIS_INSIDE_MARK || m_coordinateMode == CM_AXIS_OUTSIDE_MARK)
	{
		ASSERT_VALID(pXAxis);
		ASSERT_VALID(pYAxis);

		if (m_rectCoordinates.top == _EmptyCoordinate && m_rectCoordinates.left == _EmptyCoordinate)
		{
			return;
		}

		double dblHorzOffset = m_rectCoordinates.right == _EmptyCoordinate ? 0 : m_rectCoordinates.right * szScaleRatio.cx;
		double dblVertOffset = m_rectCoordinates.bottom == _EmptyCoordinate ? 0 : m_rectCoordinates.bottom * szScaleRatio.cy;

		CBCGPChartAxis* pUsedAxis = m_rectCoordinates.left != _EmptyCoordinate ? pXAxis : pYAxis;
		double dblUsedValue = m_rectCoordinates.left != _EmptyCoordinate ? m_rectCoordinates.left : m_rectCoordinates.top;

		double dblScrVal = pUsedAxis->PointFromValue(dblUsedValue, FALSE);
		CBCGPRect rectAxis = pUsedAxis->GetAxisRect(FALSE, FALSE, TRUE);

		CBCGPPoint ptCenter;

		if (m_coordinateMode == CM_AXIS_INSIDE_MARK && 
			(pUsedAxis->m_axisDefaultPosition == CBCGPChartAxis::ADP_BOTTOM || pUsedAxis->m_axisDefaultPosition == CBCGPChartAxis::ADP_LEFT) ||
			m_coordinateMode == CM_AXIS_OUTSIDE_MARK && 
			(pUsedAxis->m_axisDefaultPosition == CBCGPChartAxis::ADP_TOP || pUsedAxis->m_axisDefaultPosition == CBCGPChartAxis::ADP_RIGHT))
		{
			if (pUsedAxis->IsVertical())
			{
				ptCenter.x = rectAxis.right + dblVertOffset + szObjectSize.cx / 2;
				ptCenter.y = dblScrVal - dblHorzOffset;
			}
			else
			{
				ptCenter.x = dblScrVal + dblHorzOffset;
				ptCenter.y = rectAxis.top - dblVertOffset - szObjectSize.cy /2;
			}
		}
		else
		{
			if (pUsedAxis->IsVertical())
			{
				ptCenter.x = rectAxis.left - dblVertOffset - szObjectSize.cx / 2;
				ptCenter.y = dblScrVal - dblHorzOffset;
			}
			else
			{
				ptCenter.x = dblScrVal + dblHorzOffset;
				ptCenter.y = rectAxis.bottom + dblVertOffset + szObjectSize.cy /2;
			}
		}

		if (pUsedAxis->IsVertical() && (ptCenter.y < rectAxis.top || ptCenter.y > rectAxis.bottom) || 
			!pUsedAxis->IsVertical() && (ptCenter.x < rectAxis.left || ptCenter.x > rectAxis.right))
		{
			return;
		}

		ptLeftTop.SetPoint(ptCenter.x - szObjectSize.cx / 2, ptCenter.y - szObjectSize.cy / 2);
		ptRightBottom.SetPoint(ptCenter.x + szObjectSize.cx / 2, ptCenter.y + szObjectSize.cy / 2);
	}
	else
	{
		return;
	}

	if (m_format.m_textFormat.IsWordWrap())
	{
		if (ptLeftTop.x == CBCGPChartObject::_EmptyCoordinate && 
			ptRightBottom.x == CBCGPChartObject::_EmptyCoordinate && 
			szObjectSize.cx == 0)
		{
			ASSERT(FALSE);
			TRACE0(" CBCGPChartObject::OnCalcScreenPoints: Left and right offsets must be specified in order to properly use wrapped text.\n");
			return;
		}

		szObjectSize = pGM->GetTextSize(m_strText, m_format.m_textFormat, ptRightBottom.x - ptLeftTop.x);
	}
	

	if (ptLeftTop.x == CBCGPChartObject::_EmptyCoordinate)
	{
		ptLeftTop.x = ptRightBottom.x - szObjectSize.cx;
	}

	if (ptLeftTop.y == CBCGPChartObject::_EmptyCoordinate)
	{
		ptLeftTop.y = ptRightBottom.y - szObjectSize.cy;
	}

	if (ptRightBottom.x == CBCGPChartObject::_EmptyCoordinate)
	{
		ptRightBottom.x = ptLeftTop.x + szObjectSize.cx;
	}

	if (ptRightBottom.y == CBCGPChartObject::_EmptyCoordinate)
	{
		ptRightBottom.y = ptLeftTop.y + szObjectSize.cy;
	}

	m_rectScreen.SetRect(ptLeftTop, ptRightBottom);
}
//*******************************************************************************
void CBCGPChartObject::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram)
{
	ASSERT_VALID(this);

	if (m_pParentChart == NULL || !IsVisible())
	{
		return;
	}

	BOOL bWasTransparency = CBCGPGraphicsManagerGDI::IsTransparencyEnabled();
	CBCGPGraphicsManagerGDI::EnableTransparency();

	ASSERT_VALID(m_pParentChart);

	if (!m_pParentChart->OnDrawChartObjectShape(pGM, rectDiagram, this))
	{
		OnDrawShape(pGM, rectDiagram);
	}

	if (!m_strText.IsEmpty() && !m_pParentChart->OnDrawChartObjectText(pGM, rectDiagram, this))
	{
		OnDrawText(pGM, rectDiagram);
	}

	CBCGPGraphicsManagerGDI::EnableTransparency(bWasTransparency);
}
//*******************************************************************************
void CBCGPChartObject::OnDrawShape(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagram*/)
{
	ASSERT_VALID(this);

	if (m_rectScreen.IsRectNull() || m_rectScreen.IsRectEmpty())
	{
		return;
	}

	CBCGPRect rectShape = m_rectScreen;
	rectShape.Normalize();

	if (!m_format.m_brFillColor.IsEmpty())
	{
		if (m_dblShadowDepth > 0. && pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY))
		{
			CBCGPRect rectShadow = rectShape;
			rectShadow.OffsetRect(m_dblShadowDepth, m_dblShadowDepth);

			CBCGPGeometry geometryShadow;

			if (!m_szCornerRadius.IsNull())
			{
				pGM->CombineGeometry(geometryShadow, 
					CBCGPRoundedRectangleGeometry(CBCGPRoundedRect(rectShadow, m_szCornerRadius.cx, m_szCornerRadius.cy)), 
					CBCGPRoundedRectangleGeometry(CBCGPRoundedRect(rectShape, m_szCornerRadius.cx, m_szCornerRadius.cy)), 
					RGN_DIFF);
			}
			else
			{
				pGM->CombineGeometry(geometryShadow, 
					CBCGPRectangleGeometry(rectShadow), 
					CBCGPRectangleGeometry(rectShape), 
					RGN_DIFF);
			}

			pGM->FillGeometry(geometryShadow, m_brShadow);
		}

		if (!m_szCornerRadius.IsNull())
		{
			pGM->FillRoundedRectangle(CBCGPRoundedRect(rectShape, m_szCornerRadius.cx, m_szCornerRadius.cy), m_format.m_brFillColor);
		}
		else
		{
			pGM->FillRectangle(rectShape, m_format.m_brFillColor);
		}
	}
	
	if (!m_format.m_outlineFormat.m_brLineColor.IsEmpty())
	{
		if (!m_szCornerRadius.IsNull())
		{
			pGM->DrawRoundedRectangle(CBCGPRoundedRect(rectShape, m_szCornerRadius.cx, m_szCornerRadius.cy), m_format.m_outlineFormat.m_brLineColor, 
							m_format.m_outlineFormat.GetLineWidth(TRUE), 
							&m_format.m_outlineFormat.m_strokeStyle);
		}
		else
		{
			pGM->DrawRectangle(rectShape, m_format.m_outlineFormat.m_brLineColor, 
							m_format.m_outlineFormat.GetLineWidth(TRUE), 
							&m_format.m_outlineFormat.m_strokeStyle);
		}
	}
}
//*******************************************************************************
void CBCGPChartObject::OnDrawText(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagram*/)
{
	ASSERT_VALID(this);

	if (m_strText.IsEmpty() || m_rectScreen.IsRectEmpty())
	{
		return;
	}

	CBCGPRect rectShape = m_rectScreen;
	rectShape.DeflateRect(m_format.GetContentPadding(TRUE));

	const CBCGPBrush& brText = m_format.m_brTextColor.IsEmpty() ? 
		m_pParentChart->GetColors().m_brChartObjectTextColor : m_format.m_brTextColor;

	pGM->DrawText(m_strText, rectShape, m_format.m_textFormat, brText);
}
//*******************************************************************************
// Chart Line Object
//*******************************************************************************
CBCGPChartLineObject::CBCGPChartLineObject()
{

}
//*******************************************************************************
CBCGPChartLineObject::CBCGPChartLineObject(CBCGPChartVisualObject* pParentChart, 
		double dblX1, double dblY1, double dblX2, double dblY2, const CBCGPBrush& brLine, double dblLineWidth, 
		CBCGPStrokeStyle* pStrokeStyle)
{
	m_pParentChart = pParentChart;
	SetCoordinates(dblX1, dblY1, dblX2, dblY2, CM_CHART_VALUES);	
	m_format.m_outlineFormat.m_brLineColor = brLine;
	m_format.m_outlineFormat.m_dblWidth = dblLineWidth;

	if (pStrokeStyle != NULL)
	{
		m_format.m_outlineFormat.m_strokeStyle.SetDashStyle(pStrokeStyle->GetDashStyle());
	}
	
}
//*******************************************************************************
CBCGPChartLineObject::CBCGPChartLineObject(CBCGPChartVisualObject* pParentChart, double dblVal, BOOL bHorz, 
										   const CBCGPBrush& brLine, double dblLineWidth, CBCGPStrokeStyle* pStrokeStyle)
{
	m_pParentChart = pParentChart;

	if (bHorz)
	{
		SetCoordinates(CBCGPPoint(CBCGPChartObject::_EmptyCoordinate, dblVal), 
						CBCGPPoint(CBCGPChartObject::_EmptyCoordinate, dblVal), CM_CHART_VALUES);		
	}
	else
	{
		SetCoordinates(CBCGPPoint(dblVal, CBCGPChartObject::_EmptyCoordinate), 
						CBCGPPoint(dblVal, CBCGPChartObject::_EmptyCoordinate), CM_CHART_VALUES);
	}

	m_format.m_outlineFormat.m_brLineColor = brLine;
	m_format.m_outlineFormat.m_dblWidth = dblLineWidth;

	if (pStrokeStyle != NULL)
	{
		m_format.m_outlineFormat.m_strokeStyle = *pStrokeStyle;
	}
}
//*******************************************************************************
void CBCGPChartLineObject::OnDrawShape(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagram*/)
{
	ASSERT_VALID(this);

	if (m_rectScreen.IsRectNull())
	{
		return;
	}
	
	BCGPSeriesColorsPtr colors;
	m_pParentChart->GetColors().GetSeriesColors(colors, 0, CBCGPBrush::BCGP_NO_GRADIENT);

	if (m_format.m_outlineFormat.m_brLineColor.IsEmpty() && colors.m_pBrElementLineColor == NULL)
	{
		return;
	}

	const CBCGPBrush& brLine = m_format.m_outlineFormat.m_brLineColor.IsEmpty() ? 
		 *colors.m_pBrElementLineColor : m_format.m_outlineFormat.m_brLineColor;

	pGM->DrawLine(m_rectScreen.TopLeft(), m_rectScreen.BottomRight(),
		brLine, m_format.m_outlineFormat.GetLineWidth(TRUE), 
		&m_format.m_outlineFormat.m_strokeStyle);
}
//*******************************************************************************
BOOL CBCGPChartLineObject::HitTest(const CBCGPPoint& pt) const
{
	double dblWidth = m_format.m_outlineFormat.GetLineWidth(TRUE) / 2;

	CBCGPPoint ptTopLeft = m_rectScreen.TopLeft();
	CBCGPPoint ptBottomRight = m_rectScreen.BottomRight();

	if (ptTopLeft.x == ptBottomRight.x)
	{
		CBCGPRect r = m_rectScreen.NormalizedRect();
		r.InflateRect(dblWidth, dblWidth);
		return r.PtInRect(pt);
	}
	
	double dblACoef = (ptBottomRight.y - ptTopLeft.y) / (ptBottomRight.x - ptTopLeft.x);
	double dblBCoef = ptBottomRight.y - dblACoef * ptBottomRight.x;
	
	double dblY = dblACoef * pt.x + dblBCoef;

	return pt.y > dblY - dblWidth && pt.y < dblY + dblWidth;
}

//*******************************************************************************
// Chart Range Object
//*******************************************************************************

CBCGPChartRangeObject::CBCGPChartRangeObject()
{
	m_bIsForeground = FALSE;
}
//*******************************************************************************
CBCGPChartRangeObject::CBCGPChartRangeObject(CBCGPChartVisualObject* pParentChart, double dblVal1, double dblVal2, BOOL bHorz, const CBCGPBrush& brFill)
{
	m_pParentChart = pParentChart;
	m_bIsForeground = FALSE;

	if (bHorz)
	{
		SetCoordinates(
			CBCGPPoint(CBCGPChartObject::_EmptyCoordinate, dblVal1), 
			CBCGPPoint(CBCGPChartObject::_EmptyCoordinate, dblVal2), CM_CHART_VALUES);		
	}
	else
	{
		SetCoordinates(
			CBCGPPoint(dblVal1, CBCGPChartObject::_EmptyCoordinate), 
			CBCGPPoint(dblVal2, CBCGPChartObject::_EmptyCoordinate), CM_CHART_VALUES);
	}

	m_format.m_brFillColor = brFill;
	m_format.m_outlineFormat.m_brLineColor.Empty();
}

//*******************************************************************************
// Chart Text Object
//*******************************************************************************

CBCGPChartTextObject::CBCGPChartTextObject()
{
	m_bDrawConnector = FALSE;
}
//*******************************************************************************
CBCGPChartTextObject::CBCGPChartTextObject(CBCGPChartVisualObject* pParentChart, const CString& strText,
		double dblValX, double dblValY,
		const CBCGPBrush& brTextColor, const CBCGPBrush& brFill, const CBCGPBrush& brOutline,
		double dblDistanceFromPoint, double dblAngle, BOOL bDrawConnector)
{
	m_pParentChart = pParentChart;
	m_strText = strText;

	if (dblDistanceFromPoint == _EmptyCoordinate || dblAngle == _EmptyCoordinate)
	{
		m_bDrawConnector = FALSE;

		SetCoordinates(CBCGPPoint(dblValX, dblValY), 
			CBCGPPoint(CBCGPChartObject::_EmptyCoordinate, CBCGPChartObject::_EmptyCoordinate), CM_CHART_VALUES);
	}
	else
	{
		m_bDrawConnector = bDrawConnector;

		if (m_bDrawConnector)
		{
			m_ptAnchor = CBCGPPoint(dblValX, dblValY);
		}

		SetCoordinates(dblValX, dblValY, dblDistanceFromPoint, dblAngle, CM_CHART_VALUE_DIST_ANGLE);
	}
	
	m_format.m_brTextColor = brTextColor;
	m_format.m_brFillColor = brFill;
	m_format.m_outlineFormat.m_brLineColor = brOutline;
}
//*******************************************************************************
void CBCGPChartTextObject::OnDrawShape(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram)
{
	if (m_bDrawConnector && !m_format.m_outlineFormat.m_brLineColor.IsEmpty())
	{
		CBCGPRect rectShape = m_rectScreen;
		rectShape.Normalize();

		if (!rectShape.PtInRect(m_ptAnchor))
		{
			CBCGPPoint pt;

			if (bcg_CS_intersect(rectShape, m_ptAnchor, rectShape.CenterPoint(), pt))
			{
				pGM->DrawLine(m_ptAnchor, pt, m_format.m_outlineFormat.m_brLineColor, 
					m_format.m_outlineFormat.GetLineWidth(TRUE), &m_format.m_outlineFormat.m_strokeStyle);
			}
			
			CBCGPEllipse ellipse(m_ptAnchor, 2 * m_format.GetScaleRatio().cx, 2 * m_format.GetScaleRatio().cy);

			pGM->FillEllipse(ellipse, m_format.m_brFillColor);
			pGM->DrawEllipse(ellipse, m_format.m_outlineFormat.m_brLineColor);
		}
	}

	CBCGPChartObject::OnDrawShape(pGM, rectDiagram);
}

//*******************************************************************************
// Chart Axis Mark Object
//*******************************************************************************

CBCGPChartAxisMarkObject::CBCGPChartAxisMarkObject()
{

}

CBCGPChartAxisMarkObject::CBCGPChartAxisMarkObject(CBCGPChartVisualObject* pParentChart, double dblVal, 
						const CString& strText, BOOL bVertAxis, BOOL bOutside,
						const CBCGPBrush& brTextColor, const CBCGPBrush& brFill, const CBCGPBrush& brOutline)
{
	m_pParentChart = pParentChart;

	m_strText = strText;

	SetAxisMarkCoordinate(dblVal, bVertAxis, bOutside);

	m_format.m_brTextColor = brTextColor;
	m_format.m_brFillColor = brFill;
	m_format.m_outlineFormat.m_brLineColor = brOutline;
}
//*******************************************************************************
// Chart Effects
//*******************************************************************************
CBCGPChartInterLineColoringEffect::CBCGPChartInterLineColoringEffect(CBCGPChartVisualObject* pChart, CBCGPChartSeries* pSeries1, CBCGPChartSeries* pSeries2)
{
	m_pChart = pChart;
	m_pSeries1 = pSeries1;
	m_pSeries2 = pSeries2;
	m_dblOrigin = 0;
	m_bTopOnly = TRUE;
}
//*******************************************************************************
CBCGPChartInterLineColoringEffect::CBCGPChartInterLineColoringEffect(CBCGPChartVisualObject* pChart, CBCGPChartSeries* pSeries1, double dblOrigin)
{
	m_pChart = pChart;
	m_pSeries1 = pSeries1;
	m_pSeries2 = NULL;
	m_dblOrigin = dblOrigin;
	m_bTopOnly = TRUE;
}
//*******************************************************************************
CBCGPChartInterLineColoringEffect::~CBCGPChartInterLineColoringEffect()
{
	
}
//*******************************************************************************
BOOL CBCGPChartInterLineColoringEffect::IsEffectShownOnAxis(CBCGPChartAxis* pAxis)
{
	return (m_pSeries1 != NULL && m_pSeries1->IsShownOnAxis(pAxis) ||
			m_pSeries2 != NULL && m_pSeries2->IsShownOnAxis(pAxis));
}
//*******************************************************************************
void CBCGPChartInterLineColoringEffect::OnCalcScreenPoints(CBCGPGraphicsManager* /*pGM*/)
{
	m_arPointsSeries1.RemoveAll();
	m_arPointsSeries2.RemoveAll();

	if (m_pSeries1 == NULL)
	{
		return;
	}

	CBCGPChartAxis* pXAxis = m_pSeries1->GetRelatedAxis(CBCGPChartSeries::AI_X);

	if (pXAxis == NULL || m_pSeries2 != NULL && m_pSeries2->GetRelatedAxis(CBCGPChartSeries::AI_X) != pXAxis)
	{
		return;
	}

	int i = 0;
	for (i = m_pSeries1->GetMinDataPointIndex(); i <= m_pSeries1->GetMaxDataPointIndex(); i++)
	{
		if (!m_pSeries1->IsDataPointScreenPointsEmpty(i))
		{
			m_arPointsSeries1.Add(m_pSeries1->GetDataPointScreenPoint(i, 0)); 
		}
	}

	if (m_pSeries2 != NULL)
	{
		for (i = m_pSeries2->GetMinDataPointIndex(); i <= m_pSeries2->GetMaxDataPointIndex(); i++)
		{
			if (!m_pSeries2->IsDataPointScreenPointsEmpty(i))
			{
				m_arPointsSeries2.Add(m_pSeries2->GetDataPointScreenPoint(i, 0)); 
			}
		}
	}
}
//*******************************************************************************
void CBCGPChartInterLineColoringEffect::OnDraw(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);

	if (m_pSeries1 == NULL || m_arPointsSeries1.GetSize() < 2 || !IsVisible())
	{
		return;
	}

	BOOL bWasTransparency = CBCGPGraphicsManagerGDI::IsTransparencyEnabled();
	CBCGPGraphicsManagerGDI::EnableTransparency();

	BCGPChartFormatSeries::ChartCurveType curveType = m_pSeries1->GetCurveType();

	if (curveType == BCGPChartFormatSeries::CCT_STEP || curveType == BCGPChartFormatSeries::CCT_REVERSED_STEP ||
		m_pSeries2 != NULL && (m_pSeries2->GetCurveType() == BCGPChartFormatSeries::CCT_STEP || 
								m_pSeries2->GetCurveType() == BCGPChartFormatSeries::CCT_REVERSED_STEP))
	{
		return;
	}

	CBCGPChartAxis* pXAxis = m_pSeries1->GetRelatedAxis(CBCGPChartSeries::AI_X);
	CBCGPRect rectBounds = pXAxis->GetBoundingRect();

	CBCGPChartAxis* pYAxis = m_pSeries1->GetRelatedAxis(CBCGPChartSeries::AI_Y);

	if (pYAxis->m_bReverseOrder)
	{
		pYAxis->IsVertical() ? rectBounds.SwapTopBottom() : rectBounds.SwapLeftRight();
	}

	if (pXAxis->m_bReverseOrder)
	{
		pXAxis->IsVertical() ?  rectBounds.SwapTopBottom() : rectBounds.SwapLeftRight();
	}

	BCGPSeriesColorsPtr colors;
	m_pSeries1->GetColors(colors, -1);

	CBCGPBrush& brFill = m_brTopBrush.IsEmpty() ? *colors.m_pBrElementFillColor : m_brTopBrush;
	
	CBCGPGeometry* pDrawGeometry = CreateGeometry(m_arPointsSeries1, rectBounds, curveType, FALSE);
	CBCGPGeometry* pClipGeometry = m_pSeries2 != NULL ? 
		CreateGeometry(m_arPointsSeries2, rectBounds, m_pSeries2->GetCurveType(), TRUE) : 
		CreateClipGeometry(m_dblOrigin);

	ASSERT_VALID(pDrawGeometry);
	ASSERT_VALID(pClipGeometry);

	DrawEffect(pGM, pDrawGeometry, pClipGeometry, rectBounds, brFill);

	delete pClipGeometry;
	delete pDrawGeometry;

	if (m_pSeries2 != NULL && m_arPointsSeries2.GetSize() > 2 && !m_bTopOnly)
	{
		BCGPSeriesColorsPtr colors;
		m_pSeries2->GetColors(colors, -1);

		pDrawGeometry = CreateGeometry(m_arPointsSeries2, rectBounds, m_pSeries2->GetCurveType(), FALSE);
		pClipGeometry = CreateGeometry(m_arPointsSeries1, rectBounds, curveType, TRUE);

		CBCGPBrush& brFill = m_brBottomBrush.IsEmpty() ? *colors.m_pBrElementFillColor : m_brBottomBrush;
		DrawEffect(pGM, pDrawGeometry, pClipGeometry, rectBounds, brFill);

		delete pClipGeometry;
		delete pDrawGeometry;
	}

	CBCGPGraphicsManagerGDI::EnableTransparency(bWasTransparency);
}
//*******************************************************************************
void CBCGPChartInterLineColoringEffect::DrawEffect(CBCGPGraphicsManager* pGM, CBCGPGeometry* pDrawGeometry, CBCGPGeometry* pClipGeometry, 
													const CBCGPRect& rectBounds, const CBCGPBrush& brFill)
{
    CBCGPRectangleGeometry g(rectBounds);

    CBCGPGeometry geometryDest;

    pGM->CombineGeometry(geometryDest, g, *pClipGeometry, RGN_AND);

    CBCGPGeometry geometryDraw;
    pGM->CombineGeometry(geometryDraw, *pDrawGeometry, geometryDest, RGN_DIFF);

    pGM->FillGeometry(geometryDraw, brFill);
}
//*******************************************************************************
CBCGPGeometry* CBCGPChartInterLineColoringEffect::CreateClipGeometry(double dblOrigin)
{
	CBCGPChartAxis* pYAxis = m_pSeries1->GetRelatedAxis(CBCGPChartSeries::AI_Y);

	ASSERT_VALID(pYAxis);

	CBCGPRect rectBounds = pYAxis->GetBoundingRect();
	BOOL bIsVertical = pYAxis->IsVertical();

	double dblVal = pYAxis->PointFromValue(dblOrigin, TRUE);

	rectBounds.Normalize();

	if (bIsVertical)
	{
		rectBounds.top = dblVal;
	}
	else
	{
		rectBounds.right = dblVal;
	}

	CBCGPRectangleGeometry* pGeometry = new CBCGPRectangleGeometry(rectBounds);
	return pGeometry;
}
//*******************************************************************************
CBCGPGeometry* CBCGPChartInterLineColoringEffect::CreateGeometry(const CBCGPPointsArray& arOrgPoints, const CBCGPRect& rectBounds, 
																 BCGPChartFormatSeries::ChartCurveType curveType, BOOL bClip)
{
	CBCGPChartAxis* pXAxis = m_pSeries1->GetRelatedAxis(CBCGPChartSeries::AI_X);

	ASSERT_VALID(pXAxis);

	BOOL bIsVertical = pXAxis->IsVertical();
	CBCGPGeometry* pGeometry = NULL;

	CBCGPPointsArray arPoints;
	arPoints.Append(arOrgPoints);

	CBCGPPoint ptStart = arPoints[0];
	CBCGPPoint ptEnd = arPoints[arPoints.GetSize() - 1];

	CBCGPPoint ptClipCorrectionStart = ptStart;
	CBCGPPoint ptClipCorrectionEnd = ptEnd;

	if (bIsVertical)
	{
		if (bClip)
		{
			if (pXAxis->m_bReverseOrder)
			{
				ptClipCorrectionStart.y -= 1.;
				ptClipCorrectionEnd.y += 1.;
			}
			else
			{
				ptClipCorrectionStart.y += 1.;
				ptClipCorrectionEnd.y -= 1.;
			}
			
			ptStart = CBCGPPoint(rectBounds.left, ptClipCorrectionStart.y);
			ptEnd = CBCGPPoint(rectBounds.left, ptClipCorrectionEnd.y);
		}
		else
		{
			ptStart = CBCGPPoint(rectBounds.left, ptStart.y);
			ptEnd = CBCGPPoint(rectBounds.left, ptEnd.y);
		}
	}
	else
	{
		if (bClip)
		{
			if (pXAxis->m_bReverseOrder)
			{
				ptClipCorrectionStart.x += 1.;
				ptClipCorrectionEnd.x -= 1.;
			}
			else
			{
				ptClipCorrectionStart.x -= 1.;
				ptClipCorrectionEnd.x += 1.;
			}
			ptStart = CBCGPPoint(ptClipCorrectionStart.x, rectBounds.bottom);
			ptEnd = CBCGPPoint(ptClipCorrectionEnd.x, rectBounds.bottom);
		}
		else
		{
			ptStart = CBCGPPoint(ptStart.x, rectBounds.bottom);
			ptEnd = CBCGPPoint(ptEnd.x, rectBounds.bottom);
		}
	}

	if (curveType == BCGPChartFormatSeries::CCT_SPLINE || curveType == BCGPChartFormatSeries::CCT_SPLINE_HERMITE)
	{
		CBCGPSplineGeometry::BCGP_SPLINE_TYPE splineType = CBCGPSplineGeometry::BCGP_SPLINE_TYPE_KB;

		if (curveType == BCGPChartFormatSeries::CCT_SPLINE_HERMITE)
		{
			splineType = CBCGPSplineGeometry::BCGP_SPLINE_TYPE_HERMITE;
		}

		CBCGPComplexGeometry* pComplex = new CBCGPComplexGeometry() ;

		CBCGPSplineGeometry geometryTop(arOrgPoints, splineType, FALSE);

		pComplex->AddPoints(geometryTop.GetPoints(), CBCGPPolygonGeometry::BCGP_CURVE_TYPE_BEZIER);

		if (bClip)
		{
			pComplex->AddLine(ptClipCorrectionEnd);
		}

		pComplex->AddLine(ptEnd);
		pComplex->AddLine(ptStart);

		if (bClip)
		{
			pComplex->AddLine(ptClipCorrectionStart);
		}
	
		pComplex->SetClosed(TRUE);

		pGeometry = pComplex;
	}
	else
	{
		if (bClip)
		{
			arPoints.Add(ptClipCorrectionEnd);
		}

		arPoints.Add(ptEnd);
		arPoints.Add(ptStart);

		if (bClip)
		{
			arPoints.Add(ptClipCorrectionStart);
		}
		
		pGeometry = new CBCGPPolygonGeometry(arPoints);
	}

	return pGeometry;
}