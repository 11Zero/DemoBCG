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
#include "BCGPChartAxis.h"
#include "BCGPChartVisualObject.h"

IMPLEMENT_DYNCREATE(CBCGPChartAxis, CObject)
IMPLEMENT_DYNCREATE(CBCGPChartAxisX, CBCGPChartAxis)
IMPLEMENT_DYNCREATE(CBCGPChartAxisY, CBCGPChartAxis)
IMPLEMENT_DYNCREATE(CBCGPChartAxisZ, CBCGPChartAxis)
IMPLEMENT_DYNCREATE(CBCGPChartAxisPolarY, CBCGPChartAxisY)
IMPLEMENT_DYNCREATE(CBCGPChartAxisPolarX, CBCGPChartAxisX)
IMPLEMENT_DYNCREATE(CBCGPChartTernaryAxis, CBCGPChartAxis)

//----------------------------------------------------------------------------//
// Common axis implementation
//----------------------------------------------------------------------------//
CBCGPChartAxis::CBCGPChartAxis()
{
	m_pChart = NULL;
	m_nAxisID = -1;
	m_axisDefaultPosition = (CBCGPChartAxis::AxisDefaultPosition)-1;
}

CBCGPChartAxis::CBCGPChartAxis(int nAxisID, CBCGPChartAxis::AxisDefaultPosition position, CBCGPChartVisualObject* pChartCtrl)
{
	m_pChart = pChartCtrl;
	m_nAxisID = nAxisID;
	m_axisDefaultPosition = (CBCGPChartAxis::AxisDefaultPosition)-1;

	SetAxisDefaultPosition(position);
}

CBCGPChartAxis::~CBCGPChartAxis(void)
{
}

double CBCGPChartAxis::CalcLog(double dblVal, BOOL bForward/* = TRUE*/) const
{
	if (bForward)
	{
		if (dblVal <= 0)
		{
			dblVal = 1.;
		}
		
		return log(dblVal) / log(m_dblMajorUnit);
	}
	
	return pow(m_dblMajorUnit, dblVal);
}

double CBCGPChartAxis::GetDoubleCorrection() const
{
	return m_dblMajorUnit > DBL_EPSILON ? 3 * DBL_EPSILON : m_dblMinorUnit;
}

void CBCGPChartAxis::SetAxisDefaultPosition(CBCGPChartAxis::AxisDefaultPosition defaultPos)
{
	if (m_axisDefaultPosition == -1)
	{
		m_axisDefaultPosition = defaultPos;
		m_bIsSecondaryAxis = m_axisDefaultPosition == CBCGPChartAxis::ADP_RIGHT || 
							m_axisDefaultPosition == CBCGPChartAxis::ADP_TOP || 
							m_axisDefaultPosition == CBCGPChartAxis::ADP_DEPTH_TOP;

		if (m_bIsSecondaryAxis)
		{
			m_bShowMajorGridLines = FALSE;
			m_crossType = CBCGPChartAxis::CT_MAXIMUM_AXIS_VALUE;
		}
		else
		{
			m_bShowMajorGridLines = TRUE;
			m_crossType = CBCGPChartAxis::CT_AUTO;
		}

		if (m_axisDefaultPosition == CBCGPChartAxis::ADP_DEPTH_BOTTOM)
		{
			m_crossType = CBCGPChartAxis::CT_MINIMUM_AXIS_VALUE;
		}
	}
}

void CBCGPChartAxis::CommonInit()
{
	m_bThumbnailMode = FALSE;
	m_bConnectedToDataTable = FALSE;

	m_bNewMajorUnitBehavior = FALSE;

	m_bShowMinorGridLines = FALSE;

	m_bReverseOrder = FALSE;

	m_majorTickMarkType = CBCGPChartAxis::TMT_OUTSIDE;
	m_minorTickMarkType = CBCGPChartAxis::TMT_NO_TICKS;
	m_axisLabelType = CBCGPChartAxis::ALT_NEXT_TO_AXIS;

	m_dblCrossOtherAxisValue = 0.;

	m_dblMinimum = 0.;
	m_dblMaximum = 0.;
	m_dblMajorUnit = 0.;
	m_dblMinorUnit = 0.;
	m_bFixedRange = FALSE;
	m_bFixedMaximum = FALSE;
	m_bFixedMinimum = FALSE;
	m_bNormalizeValuesInAxis = FALSE;

	m_bIsAutoMajorUnit = TRUE;

	m_nMinorUnitCount = 5;

	m_strAxisName = _T("Axis Title");

	m_axisNameFormat.m_textFormat.Create(BCGPChartFormatLabel::m_strDefaultFontFamily, 20);

	m_dblMaxDisplayedValue = 0.;
	m_dblMinDisplayedValue = 0.;
	m_nDisplayedLines = 0;
	m_nMinDisplayedLines = 3;
	m_bEnableRotateFont = TRUE;

	m_bInitialized = FALSE;
	m_bVisible = TRUE;
	m_bAlwaysVisible = FALSE;

	m_bLogScale = FALSE;
	m_dblLogScaleBase = 10;

	m_dblDisplayUnits = 0.0;

	m_bDisplayDataBetweenTickMarks = FALSE;

	m_nMajorTickMarkLen = 6;
	m_nMinorTickMarkLen = 3;

	m_dblAxisNameDistance3D = 0;
	
	m_dblLabelDistance = 2.; 
	m_axisLabelsFormat.SetContentPadding(CBCGPSize(2, 2)); 

	m_szMaxLabelSize.SetSize(0, 0);
	m_rectAxisLabels.SetRectEmpty();
	m_rectAxisName.SetRectEmpty();
	m_bDisplayAxisName = FALSE;

	m_bEmptySeries = FALSE;

	m_ptAxisStart.SetPoint(0, 0);
	m_ptAxisEnd.SetPoint(0, 0);

	m_nColumnOverlapPercent = -1;
	m_nColumnDistancePercent = 150;

	m_bFillMajorUnitInterval = FALSE;
	m_bFormatAsDate = FALSE;

	m_nUnitFillStep = 2;
	m_nFirstIntervalToFill = 0;

	m_pPerpendicularAxis = NULL;
	m_bUsePerpAxisForZoning = TRUE;

	m_nResizeBandSize = 3;

	m_bRoundMinMax = TRUE;

	m_dblBottomOffset = 0;
	m_dblTopOffset = 0;
	m_nAxisGap = 10;

	m_pSplitTop = NULL;
	m_pSplitBottom = NULL;

	BOOL bVert = (m_axisDefaultPosition == CBCGPChartAxis::ADP_LEFT || m_axisDefaultPosition == CBCGPChartAxis::ADP_RIGHT);
	SetVertical(bVert);

	m_bFixedMajorUnit = FALSE;
	m_dblMinAllowedMajorUnit = 0;
	m_nMajorUnitSpacing = 0;

	m_bIsFixedIntervalWidth = FALSE;
	m_nFixedIntervalWidth = 10;
	m_nValuesPerInterval = 1;	
	m_bStretchFixedIntervals = TRUE;

	m_nRightOffset = 0;

	m_bIsFxedUnitCount = FALSE;
	m_nFixedUnitCount = 10;

	m_bPrevFixedRange = FALSE;
	m_bPrevFixedMinimum = FALSE;
	m_bPrevFixedMaximum = FALSE;
	m_dblPrevMinimum = 0.;
	m_dblPrevMaximum = 0.;
	m_dblPrevMinDisplayedValue = 0.;
	m_dblPrevMaxDisplayedValue = 0.;
	m_nPrevFixedIntervalWidth = m_nFixedIntervalWidth;
	m_nPrevValuesPerInterval = m_nValuesPerInterval;

	m_bEnableZoom = TRUE;
	m_bEnableScroll = TRUE;
	m_bZoomed = FALSE;
	m_dblMaxZoomInFactor = 1000;
	m_bSavedScrollOption = m_bEnableScroll;
	m_bSavedZoomOption = m_bEnableZoom;
	m_bIsThumbGripEnabled = FALSE;

	m_bEnableNegativeValuesForFixedInterval = TRUE;

	m_dblMinScrollValue = -DBL_MAX;
	m_dblMaxScrollValue = DBL_MAX;

	m_dblScrollBarSize = 8;
	m_dblMinThumbSize = 16;

	m_rectScrollBar.SetRectEmpty();
	m_bShowScrollBar = FALSE;
	m_bTickMarksTopRight = FALSE;

	m_dblMinDataPointDistance = DBL_MAX;

	m_pIndexedSeries = NULL;

	m_bUseApproximation = TRUE;
	m_bAlwaysShowScrollBar = FALSE;

	m_bEnableInterlaceLabels = TRUE;
	m_bIsPreventLabelOverlapIn3D = TRUE;
	
	m_bIndependentZoom = FALSE;
	m_bUnzoomOnRangeBreak = FALSE;

	m_bScaleBreakEnabled = FALSE;

	// 3D properties
	m_dblAxisSize3D = 0;

	m_bEnableInterlaceNearWall = TRUE;
	m_bEnableInterlaceFarWall = TRUE;
	m_bEnableGridLineNearWall = TRUE;
	m_bEnableGridLineFarWall = TRUE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::Reset()
{
	ASSERT_VALID(this);

	m_dblMinimum = 0.;
	m_dblMaximum = 0.;
	m_dblMajorUnit = 0.;
	m_dblMinorUnit = 0.;
	m_bFixedRange = FALSE;
	m_bFixedMinimum = FALSE;
	m_bFixedMaximum = FALSE;
	m_bNormalizeValuesInAxis = FALSE;
	m_bFormatAsDate = FALSE;
	m_bFixedMajorUnit = FALSE;
	
	m_dblMaxDisplayedValue = 0.;
	m_dblMinDisplayedValue = 0.;
	m_nDisplayedLines = 0;

	m_dblMinAllowedMajorUnit = 0;

	m_nColumnOverlapPercent = 0;
	m_nColumnDistancePercent = 150;

	m_strDataFormat.Empty();

	m_nFirstIntervalToFill = 0;
	m_nUnitFillStep = 2;

	m_bEmptySeries = FALSE;
	m_bInitialized = FALSE;
	m_bDisplayDataBetweenTickMarks = FALSE;

	m_bFillMajorUnitInterval = FALSE;

	if (m_bIsSecondaryAxis)
	{
		m_crossType = CBCGPChartAxis::CT_MAXIMUM_AXIS_VALUE;
	}
	else
	{
		m_crossType = CBCGPChartAxis::CT_AUTO;
	}

	m_bIsFixedIntervalWidth = FALSE;
	m_nFixedIntervalWidth = 10;
	m_nValuesPerInterval = 1;
	m_nRightOffset = 0;

	m_bIsFxedUnitCount = FALSE;
	m_nFixedUnitCount = 10;

	m_nMajorUnitSpacing = 0;
	
	m_pIndexedSeries = NULL;

	m_bPrevFixedRange = FALSE;
	m_bPrevFixedMinimum = FALSE;
	m_bPrevFixedMaximum = FALSE;
	m_dblPrevMinimum = 0.;
	m_dblPrevMaximum = 0.;
	m_dblPrevMinDisplayedValue = 0.;
	m_dblPrevMaxDisplayedValue = 0.;
	m_nPrevFixedIntervalWidth = m_nFixedIntervalWidth;
	m_nPrevValuesPerInterval = m_nValuesPerInterval;

	m_dblMinScrollValue = -DBL_MAX;
	m_dblMaxScrollValue = DBL_MAX;

	m_dblMinDataPointDistance = DBL_MAX;

	m_dblMaxZoomInFactor = 1000;

	m_bZoomed = FALSE;

	m_bUseApproximation = TRUE;
	m_bAlwaysShowScrollBar = FALSE;

	m_bEnableInterlaceLabels = TRUE;
	m_bIndependentZoom = FALSE;
	m_bUnzoomOnRangeBreak = FALSE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetThumbnailMode(BOOL bSet/* = TRUE*/)
{
	ASSERT_VALID(this);

	m_bThumbnailMode = bSet;

	m_axisLabelsFormat.m_bIsThumbnailMode = bSet;
	m_axisLabelsFormat.Reset();

	m_axisLabelsFormat.SetContentPadding(bSet ? CBCGPSize(2, 0) : CBCGPSize(2, 2)); 
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetNormalizeValuesInAxis(BOOL bSet/* = TRUE*/)
{
	ASSERT_VALID(this);
	m_bNormalizeValuesInAxis = bSet;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetFixedMajorUnit(double dblMajorUnit, BOOL bSet)
{
	if (bSet && IsLogScale())
	{
		TRACE0("Fixed major unit mode can't be enabled for logarithmic scales.");
		return;
	}

	if (dblMajorUnit > 0. && bSet)
	{
		m_dblMajorUnit = dblMajorUnit;
		m_dblMinorUnit = dblMajorUnit / m_nMinorUnitCount;
	}

	m_bFixedMajorUnit = bSet;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetMinAllowedMajorUnit(double dblMinAllowedMajorUnit)
{
	m_dblMinAllowedMajorUnit = dblMinAllowedMajorUnit;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetIndexedSeries(CBCGPChartSeries* pSeries)
{
	ASSERT_VALID(this);

	if (pSeries != NULL)
	{
		ASSERT_VALID(pSeries);
	}

	m_pIndexedSeries = pSeries;
	SetFixedMajorUnit(1.0, pSeries != NULL);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetFixedIntervalWidth(int nWidth, int nValuesPerInterval)
{
	if (nValuesPerInterval < 0)
	{
		m_bIsFixedIntervalWidth = FALSE;
		SetFixedMajorUnit(1.0, FALSE);
		return;
	}

	if (nValuesPerInterval < 1)
	{
		nValuesPerInterval = 1;
	}

	m_nFixedIntervalWidth = nWidth;

	if (m_nFixedIntervalWidth < 2)
	{
		m_nFixedIntervalWidth = 2;
	}

	m_nValuesPerInterval = nValuesPerInterval;

	if (m_nValuesPerInterval > m_nFixedIntervalWidth)
	{
		m_nValuesPerInterval = m_nFixedIntervalWidth;
	}

	m_bIsFixedIntervalWidth = TRUE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetRightOffsetInPixels(int nValuesCount)
{
	m_nRightOffset = bcg_clamp(nValuesCount, 0, 10000);

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE);
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetFixedUnitCount(int nCount, BOOL bSet)
{
	m_bIsFxedUnitCount = bSet;
	m_nFixedUnitCount = bcg_clamp(nCount, 2, 5000);
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::IsAxisVisible() const
{
	ASSERT_VALID(this);

	if (!m_bVisible || m_pChart == NULL)
	{
		return FALSE;
	}

	if (m_bAlwaysVisible)
	{
		return TRUE;
	}

	return HasSeries();
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::HasSeries() const
{
	for (int i = 0; i < m_pChart->GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = m_pChart->GetSeries (i, FALSE);
		if (pSeries != NULL && pSeries->m_bVisible && 
			(pSeries->GetDataPointCount() > 0 || pSeries->IsFormulaSeries() || pSeries->IsVirtualMode()))
		{
			if (pSeries->IsShownOnAxis(this) && pSeries->GetChartImpl() != NULL && 
				pSeries->GetChartImpl()->GetAxisType() != CBCGPBaseChartImpl::AT_NO_AXIS)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SwapDirection(BOOL bAdjustGradientAngles)
{
	ASSERT_VALID(this);

	UNREFERENCED_PARAMETER(bAdjustGradientAngles);
	SetVertical(!IsVertical());
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetVertical(BOOL bVert)
{
	ASSERT_VALID(this);

	if (GetComponentIndex() == CBCGPChartData::CI_Z)
	{
		return;
	}

	m_bIsVertical = bVert;

	if (IsVertical())
	{
		m_axisNameFormat.m_textFormat.SetDrawingAngle(90);
	}
	else
	{
		m_axisNameFormat.m_textFormat.SetDrawingAngle(0);
	}

	if (IsDiagram3D() && GetComponentIndex() == CBCGPChartData::CI_X)
	{
		if (m_pChart != NULL)
		{
			CBCGPChartDiagram3D* pDiagram3D = m_pChart->GetDiagram3D();

			if (pDiagram3D != NULL)
			{
				pDiagram3D->SetXHorizontal(!IsVertical());
			}
		}
		
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetLogScale(BOOL bSet, double dblLogScaleBase)
{
	if (m_bFixedMajorUnit && bSet)
	{
		TRACE0("Logarithmic scale can't be enabled for an axis in fixed major unit mode.");
		return;
	}

	if (m_bZoomed)
	{
		TRACE0("Logarithmic scale can't be enabled for zoomed axis.");
		return;
	}

	EnableScroll(!bSet && !m_bScaleBreakEnabled);
	EnableZoom(!bSet && !m_bScaleBreakEnabled);

	m_bLogScale = bSet;
	m_dblLogScaleBase = bcg_clamp(dblLogScaleBase, 2., 1000.);
}
//----------------------------------------------------------------------------//
CBCGPRect CBCGPChartAxis::GetBoundingRect() const
{
	ASSERT_VALID(this);

	CBCGPRect rect;

	CBCGPChartAxis* pPerpAxis = GetPerpendecularAxis();
	if (pPerpAxis == NULL)
	{
		return rect;
	}

	ASSERT_VALID(pPerpAxis);

	CBCGPPoint ptAxisStartThis;
	CBCGPPoint ptAxisEndThis;

	CBCGPPoint ptAxisStartPerp;
	CBCGPPoint ptAxisEndPerp;

	GetAxisPos(ptAxisStartThis, ptAxisEndThis);
	pPerpAxis->GetAxisPos(ptAxisStartPerp, ptAxisEndPerp);

	CBCGPRect rectDiagram;
	m_pChart->OnGetPlotAreaRect(rectDiagram);

	CBCGPRect rectThis;
	CBCGPRect rectPerp;

	if (IsVertical())
	{
		rectThis.SetRect(rectDiagram.left, ptAxisStartThis.y, rectDiagram.right, ptAxisEndThis.y);
		rectPerp.SetRect(ptAxisStartPerp.x, rectDiagram.top, ptAxisEndPerp.x, rectDiagram.bottom);
	}
	else
	{
		rectThis.SetRect(ptAxisStartThis.x, rectDiagram.top, ptAxisEndThis.x, rectDiagram.bottom);
		rectPerp.SetRect(rectDiagram.left, ptAxisStartPerp.y, rectDiagram.right, ptAxisEndPerp.y);
	}

	rectThis.Normalize();
	rectPerp.Normalize();

	rect.IntersectRect(rectThis, rectPerp);

	return rect;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::ShowMajorGridLines(BOOL bShow)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pChart);
	m_bShowMajorGridLines = bShow;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty();
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::ShowMinorGridLines(BOOL bShow)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pChart);
	m_bShowMinorGridLines = bShow;

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty();
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetMinorUnitCount(int nCount)
{
	if (nCount < 1)
	{
		nCount = 1;
	}

	if (nCount > 10)
	{
		nCount = 10;
	}

	m_nMinorUnitCount = nCount;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetAutoDisplayRange()
{
	m_bFixedRange = FALSE;
	m_bFixedMinimum = FALSE;
	m_bFixedMaximum = FALSE;
	SetScrollRange();

	RestoreRanges();
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetFixedDisplayRange(double dblMinimum, double dblMaximum, double dblMajorUnit)
{
	OnBeforeSetFixedDisplayRange(dblMinimum, dblMaximum);

	if (dblMinimum == dblMaximum)
	{
		dblMinimum = m_dblMinorUnit > 0. ? dblMaximum - m_dblMinorUnit : dblMaximum - 0.1;
	}

	m_dblMinimum = dblMinimum;
	m_dblMaximum = dblMaximum;

	NormalizeDisplayRange();

	if (IsScrollEnabled())
	{
		if (m_dblMinimum < m_dblMinScrollValue)
		{
			m_dblMinimum = m_dblMinScrollValue;
		}

		if (m_dblMaximum > m_dblMaxScrollValue)
		{
			m_dblMaximum = m_dblMaxScrollValue;
		}
	}

	if (IsIndexedSeries())
	{
		m_dblMinimum = max(0, floor(dblMinimum));
	}

	m_dblMinDisplayedValue = m_dblMinimum;
	m_dblMaxDisplayedValue = m_dblMaximum;

	m_bFixedRange = TRUE;
	m_bFixedMinimum = TRUE;
	m_bFixedMaximum = TRUE;

	if (dblMajorUnit > 0)
	{
		m_dblMajorUnit = dblMajorUnit;
		m_dblMinorUnit = m_dblMajorUnit / m_nMinorUnitCount;
	
		m_bFixedMajorUnit = TRUE;
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetFixedMinimumDisplayValue(double dblMinimum, BOOL bByZoom)
{
	if (bByZoom)
	{
		SaveRanges();
	}

	if (IsScrollEnabled() && dblMinimum < m_dblMinScrollValue)
	{
		if (bByZoom)
		{
			dblMinimum = m_dblMinScrollValue;
		}
		else
		{
			m_dblMinScrollValue = dblMinimum;		
		}
		
	}

	m_dblMinimum = dblMinimum;
	m_dblMinDisplayedValue = m_dblMinimum;
	m_bFixedMinimum = TRUE;

	if (IsFixedMaximumDisplayValue())
	{
		m_bFixedRange = TRUE;
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetFixedMaximumDisplayValue(double dblMaximum, BOOL bByZoom)
{
	if (bByZoom)
	{
		SaveRanges();
	}

	if (IsScrollEnabled() && dblMaximum > m_dblMaxScrollValue)
	{
		if (bByZoom)
		{
			dblMaximum = m_dblMaxScrollValue;
		}
		else
		{
			m_dblMaxScrollValue = dblMaximum;
		}
		
	}

	m_dblMaximum = dblMaximum;
	m_dblMaxDisplayedValue = m_dblMaximum;
	m_bFixedMaximum = TRUE;

	if (IsFixedMinimumDisplayValue())
	{
		m_bFixedRange = TRUE;
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::NormalizeDisplayRange()
{
	if (m_dblMaximum < m_dblMinimum)
	{
		m_dblMinDisplayedValue = m_dblMaximum;
		m_dblMaxDisplayedValue = m_dblMinimum;

		m_dblMaximum = m_dblMaxDisplayedValue;
		m_dblMinimum = m_dblMinDisplayedValue;
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::ShowScrollBar(BOOL bShow, BOOL bRedraw)
{
	if (CanUseScaleBreaks())
	{
		return;
	}

	m_bShowScrollBar = bShow;

	if (bShow)
	{
		EnableScroll();
	}

	if (m_pChart != NULL)
	{
		if (bShow)
		{
			m_pChart->EnableScroll();
		}
		m_pChart->SetDirty(bShow, bRedraw);
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetAlwaysShowScrollBar(BOOL bShow, BOOL bRedraw)
{
	if (CanUseScaleBreaks())
	{
		return;
	}

	m_bAlwaysShowScrollBar = bShow;

	if (bShow)
	{
		EnableScroll();
	}

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(bShow, bRedraw);
	}
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::Scroll(double dblUnitsToScroll, BOOL bUnitsInPixels)
{
	if (!IsScrollEnabled())
	{
		return FALSE;
	}

	if (IsIndexedSeries() || m_bIsFixedIntervalWidth)
	{
		if (fabs(dblUnitsToScroll) < 0.5)
		{
			dblUnitsToScroll = 0;
		}
		else
		{
			dblUnitsToScroll = dblUnitsToScroll > 0 ? ceil(dblUnitsToScroll) : floor(dblUnitsToScroll);
		}
	}
	
	SaveRanges();

	double dblScrollValue = bUnitsInPixels ? dblUnitsToScroll : m_dblMajorUnit * dblUnitsToScroll;

	double dblScrollLow = GetMinDisplayedValue() + dblScrollValue;
	double dblScrollHigh = GetMaxDisplayedValue() + dblScrollValue;
	
	double dblScrollLowNew = -DBL_MAX;
	double dblScrollHighNew = DBL_MAX;

	if (dblScrollLow < m_dblMinScrollValue)
	{
		dblScrollLowNew = m_dblMinScrollValue;
	}
	
	if (dblScrollHigh > m_dblMaxScrollValue)
	{
		dblScrollHighNew = m_dblMaxScrollValue;
	}
	
	double dblLowDiff = fabs(GetMinDisplayedValue() - m_dblMinScrollValue);
	double dblHighDiff = fabs(dblScrollHighNew - GetMaxDisplayedValue());

	double dblScrollValueNew = dblScrollValue < 0 ? dblLowDiff : dblHighDiff;

	if (dblScrollValueNew < fabs(dblScrollValue))
	{
		dblScrollValue = dblScrollValue < 0 ? -dblScrollValueNew : dblScrollValueNew;

		dblScrollLow = GetMinDisplayedValue() + dblScrollValue;
		dblScrollHigh = GetMaxDisplayedValue() + dblScrollValue;
	}

	// check if it was really scrolled
	if (dblScrollLow != GetMinDisplayedValue() || dblScrollHigh != GetMaxDisplayedValue())
	{
		SetFixedDisplayRange(dblScrollLow, dblScrollHigh);
		m_pChart->OnAxisScrolled(this);
		return TRUE;
	}
	
	return FALSE;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::ScrollTo(double dblValue)
{
	if (!IsScrollEnabled())
	{
		return FALSE;
	}
	
	SaveRanges();

	double dblRange = GetMaxDisplayedValue() - GetMinDisplayedValue();

	double dblMinDisplayedValue = dblValue - dblRange / 2;
	double dblMaxDisplayedValue = dblValue + dblRange / 2;

	if (IsIndexedSeries() || m_bIsFixedIntervalWidth)
	{
		dblMinDisplayedValue = floor(dblMinDisplayedValue);
		dblMaxDisplayedValue = floor(dblMaxDisplayedValue);
	}

	if (dblMinDisplayedValue < m_dblMinScrollValue)
	{
		dblMinDisplayedValue = m_dblMinScrollValue;
		dblMaxDisplayedValue = dblMinDisplayedValue + dblRange;
	}

	if (dblMaxDisplayedValue > m_dblMaxScrollValue)
	{
		dblMaxDisplayedValue = m_dblMaxScrollValue;
		dblMinDisplayedValue = dblMaxDisplayedValue - dblRange;

		if (dblMinDisplayedValue < m_dblMinScrollValue)
		{
			dblMinDisplayedValue = m_dblMinScrollValue;
		}
	}

	// check if it was really scrolled
	if (dblMinDisplayedValue != GetMinDisplayedValue() || dblMaxDisplayedValue != GetMaxDisplayedValue())
	{
		SetFixedDisplayRange(dblMinDisplayedValue, dblMaxDisplayedValue);
		m_pChart->OnAxisScrolled(this);
		return TRUE;
	}
	
	return FALSE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetScrollRange(double dblMinValue, double dblMaxValue, BOOL bForce)
{
	if (bForce)
	{
		if (dblMinValue < dblMaxValue)
		{
			m_dblMinScrollValue = dblMinValue;
			m_dblMaxScrollValue = dblMaxValue;
		}
		else
		{
			m_dblMaxScrollValue = dblMinValue;
			m_dblMinScrollValue = dblMaxValue;
		}

		return;
	}

	if (IsZoomed())
	{
		return;
	}

	if (dblMinValue < dblMaxValue)
	{
		m_dblMinScrollValue = m_dblMinScrollValue == -DBL_MAX ? dblMinValue : min(m_dblMinScrollValue, dblMinValue);
		m_dblMaxScrollValue = m_dblMaxScrollValue == DBL_MAX ?  dblMaxValue : max(m_dblMaxScrollValue, dblMaxValue);
	}
	else
	{
		m_dblMaxScrollValue = m_dblMaxScrollValue == DBL_MAX ? dblMinValue : max(m_dblMaxScrollValue, dblMinValue);
		m_dblMinScrollValue = m_dblMinScrollValue == -DBL_MAX ? dblMaxValue : min(m_dblMinScrollValue, dblMaxValue);
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SaveRanges()
{
	if (!m_bZoomed)
	{
		m_bPrevFixedRange = IsFixedDisplayRange();
		m_bPrevFixedMinimum = IsFixedMinimumDisplayValue();
		m_bPrevFixedMaximum = IsFixedMaximumDisplayValue();
		m_dblPrevMinimum = m_dblMinimum;
		m_dblPrevMaximum = m_dblMaximum;
		m_dblPrevMinDisplayedValue = m_dblMinDisplayedValue;
		m_dblPrevMaxDisplayedValue = m_dblMaxDisplayedValue;
		m_nPrevFixedIntervalWidth = m_nFixedIntervalWidth;
		m_nPrevValuesPerInterval = m_nValuesPerInterval;
		m_bZoomed = TRUE;
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::RestoreRanges()
{
	if (m_bZoomed)
	{
		m_bFixedRange = m_bPrevFixedRange;
		m_bFixedMinimum = m_bPrevFixedMinimum;
		m_bFixedMaximum = m_bPrevFixedMaximum;
		m_dblMinimum = m_dblPrevMinimum;
		m_dblMaximum = m_dblPrevMaximum;
		m_dblMinDisplayedValue = m_dblPrevMinDisplayedValue;
		m_dblMaxDisplayedValue = m_dblPrevMaxDisplayedValue;
		m_nFixedIntervalWidth = m_nPrevFixedIntervalWidth;
		m_nValuesPerInterval = m_nPrevValuesPerInterval;
		m_bZoomed = FALSE;
	}
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::SetDisplayDataBetweenTickMarks(BOOL bSet, BOOL bRedraw)
{
	ASSERT_VALID(this);

	if (IsZoomed())
	{
		return FALSE;
	}

	BOOL bIsDisplayDataBetweenTickMarks = IsDisplayDataBetweenTickMarks();
	BOOL bResult = FALSE;

	if (CanDisplayDataBetweenTickMarks() && bSet)
	{
		m_bDisplayDataBetweenTickMarks = bSet;
		bResult = TRUE;
	}
	else
	{
		m_bDisplayDataBetweenTickMarks = FALSE;
	}

	if (bIsDisplayDataBetweenTickMarks != CanDisplayDataBetweenTickMarks() && m_pChart != NULL)
	{
		if (!IsFixedIntervalWidth())
		{
			m_pChart->RecalcMinMaxValues();
		}
		
		m_pChart->SetDirty(TRUE, bRedraw);
	}

	return bResult;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::HitTest(const CBCGPPoint& pt, BCGPChartHitInfo* pHitInfo, DWORD dwHitInfoFlags)
{
	ASSERT_VALID(this);
	ASSERT(pHitInfo != NULL);

	if (CanUseScaleBreaks() && (dwHitInfoFlags & BCGPChartHitInfo::HIT_AXIS_SCALE_BREAK) != 0)
	{
		CBCGPRect rectBounds = GetBoundingRect();
		rectBounds.UnionRect(rectBounds, m_rectAxisLabels);
		if (rectBounds.PtInRect(pt))
		{
			CBCGPChartAxisScaleBreak sb;
			int nIndex = 0;
			if (ScaleBreakFromPoint(pt, sb, nIndex))
			{
				pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_AXIS_SCALE_BREAK;
				pHitInfo->m_nIndex1 = m_nAxisID;
				pHitInfo->m_dblVal1 = sb.m_dblStart;
				pHitInfo->m_dblVal2 = sb.m_dblEnd;
				return TRUE;
			}
		}
	}

	if (!IsAxisVisible())
	{
		return FALSE;
	}

	if (m_bDisplayAxisName && m_rectAxisName.NormalizedRect().PtInRect(pt) && 
		(dwHitInfoFlags & BCGPChartHitInfo::HIT_AXIS_NAME) != 0)
	{
		pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_AXIS_NAME;
		pHitInfo->m_nIndex1 = m_nAxisID;
		return TRUE;
	}

	if ((dwHitInfoFlags & BCGPChartHitInfo::HIT_AXIS) != 0 && HitTestAxisLables(pt))
	{
		pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_AXIS;
		pHitInfo->m_nIndex1 = m_nAxisID;
		return TRUE;
	}

	if (CanShowScrollBar())
	{
		if (m_rectThumb.PtInRect(pt) && (dwHitInfoFlags & BCGPChartHitInfo::HIT_AXIS_THUMB) != 0)
		{
			pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_AXIS_THUMB;
			pHitInfo->m_nIndex1 = m_nAxisID;

			if (m_bIsThumbGripEnabled)
			{
				if (m_rectLeftGrip.PtInRect(pt))
				{
					pHitInfo->m_nIndex2 = 0;
					if (IsVertical())
					{
						pHitInfo->m_dblVal1 = m_rectLeftGrip.bottom - pt.y;
					}
					else
					{
						pHitInfo->m_dblVal1 = pt.x - m_rectLeftGrip.left;
					}
				}
				else if (m_rectRightGrip.PtInRect(pt))
				{
					pHitInfo->m_nIndex2 = 1;

					if (IsVertical())
					{
						pHitInfo->m_dblVal1 = pt.y - m_rectRightGrip.top;
					}
					else
					{
						pHitInfo->m_dblVal1 = m_rectRightGrip.right - pt.x;
					}
				}
			}

			return TRUE;
		}

		if (m_rectScrollBar.PtInRect(pt) && (dwHitInfoFlags & BCGPChartHitInfo::HIT_AXIS_SCROLL_BAR) != 0)
		{
			pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_AXIS_SCROLL_BAR;
			pHitInfo->m_nIndex1 = m_nAxisID;
			return TRUE;
		}
	}

	if ((dwHitInfoFlags & BCGPChartHitInfo::HIT_AXIS) != 0 && HitTestAxisShape(pt))
	{
		pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_AXIS;
		pHitInfo->m_nIndex1 = m_nAxisID;
		return TRUE;
	}

	if ((m_dblTopOffset != 0 || m_dblBottomOffset != 0) && m_pChart->IsResizeAxesEnabled())
	{
		CBCGPRect rectBounds = GetBoundingRect();
		CBCGPRect rectBoundsSave = rectBounds;
		if (m_dblTopOffset != 0)
		{
			if (IsVertical())
			{
				rectBounds.bottom = rectBounds.top;
				rectBounds.top -= m_nResizeBandSize;  
			}
			else
			{
				rectBounds.left = rectBounds.right;
				rectBounds.right += m_nResizeBandSize;	
			}

			if (rectBounds.PtInRect(pt))
			{
				pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_AXIS_RESIZE_BAND;
				pHitInfo->m_nIndex1 = m_nAxisID;
				pHitInfo->m_nIndex2 = 1;
				return TRUE;
			}
		}

		if (m_dblBottomOffset != 0)
		{
			rectBounds = rectBoundsSave;

			if (IsVertical())  
			{
				rectBounds.top = rectBounds.bottom;
				rectBounds.bottom += m_nResizeBandSize;
			}
			else
			{
				rectBounds.right = rectBounds.left;
				rectBounds.left -= m_nResizeBandSize;	
			}

			if (rectBounds.PtInRect(pt))
			{
				pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_AXIS_RESIZE_BAND;
				pHitInfo->m_nIndex1 = m_nAxisID;
				pHitInfo->m_nIndex2 = 0;
				return TRUE;
			}
		}
	}

	return FALSE;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::HitGridLinesTest(const CBCGPPoint& pt, BCGPChartHitInfo* pHitInfo)
{
	ASSERT_VALID(this);
	if (!m_bShowMajorGridLines && !m_bShowMinorGridLines)
	{
		return FALSE;
	}

	int i = 0;
	for (; i < m_arMajorGridLines.GetSize(); i++)
	{
		CBCGPRect rect = m_arMajorGridLines[i];
		if (rect.PtInRect(pt))
		{
			pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_AXIS_GRIDLINE;
			pHitInfo->m_nIndex1 = m_nAxisID;
			pHitInfo->m_nIndex2 = 1;
			return TRUE;
		}
	}

	for (i = 0; i < m_arMinorGridLines.GetSize(); i++)
	{
		CBCGPRect rect = m_arMinorGridLines[i];
		if (rect.PtInRect(pt))
		{
			pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_AXIS_GRIDLINE;
			pHitInfo->m_nIndex1 = m_nAxisID;
			pHitInfo->m_nIndex2 = 0;
			return TRUE;
		}
	}

	return FALSE;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::HitTestAxisShape(const CBCGPPoint& pt) const
{
	if (IsDiagram3D() && (fabs(m_ptAxisEnd.x - m_ptAxisStart.x) > 4 * m_nMajorTickMarkLen))
	{
		double dblACoef = (m_ptAxisEnd.y - m_ptAxisStart.y) / (m_ptAxisEnd.x - m_ptAxisStart.x);
		double dblBCoef = m_ptAxisEnd.y - dblACoef * m_ptAxisEnd.x;

		double dblY = dblACoef * pt.x + dblBCoef;
		double dblX = (pt.y - dblBCoef) / dblACoef;
		return (pt.y > (dblY - m_nMajorTickMarkLen) && pt.y < (dblY + m_nMajorTickMarkLen)) ||
			(pt.x > (dblX - m_nMajorTickMarkLen) && pt.x < (dblX + m_nMajorTickMarkLen));
	}

	CBCGPRect rectAxis = GetAxisRect(FALSE, TRUE, FALSE);
	return rectAxis.PtInRect(pt);
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::HitTestAxisLables(const CBCGPPoint& pt) const
{
	return (GetAxisLabelType() != CBCGPChartAxis::ALT_NO_LABELS && m_rectAxisLabels.NormalizedRect().PtInRect(pt));
}
//----------------------------------------------------------------------------//
CBCGPRect CBCGPChartAxis::GetAxisRect(BOOL bCombineWithLabels, BOOL bCombineWithTickMarks, BOOL bCombineWithScrollBar) const
{
	CBCGPPoint ptStart;
	CBCGPPoint ptEnd;

	GetAxisPos(ptStart, ptEnd);

	CBCGPRect rectAxis;
	rectAxis.SetRect(ptStart, ptEnd);
	rectAxis.Normalize();

	if (bCombineWithTickMarks)
	{
		if (IsVertical())
		{
			rectAxis.InflateRect(GetMajorTickMarkLen(TRUE), 0);
		}
		else
		{
			rectAxis.InflateRect(0, GetMajorTickMarkLen(TRUE));
		}
	}

	if (bCombineWithLabels && GetAxisLabelType() != CBCGPChartAxis::ALT_NO_LABELS)
	{
		rectAxis.UnionRect(m_rectAxisLabels);
	}

	if (bCombineWithScrollBar && !m_rectScrollBar.IsRectEmpty())
	{
		rectAxis.UnionRect(m_rectScrollBar);
	}

	return rectAxis;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::CalcNameRect(CBCGPGraphicsManager* pGM, CBCGPRect& rectPlotArea, const CRect& rectChartArea, BOOL bReposOnly)
{
	ASSERT_VALID(this);
	ASSERT_VALID(m_pChart);

	if (!bReposOnly)
	{
		m_rectAxisName.SetRectEmpty();
	}

	CBCGPRect rectAxisName; // ignore in adjust layout for custom rects
	BOOL bCustomAxisNameRect = m_pChart->OnGetAxisNameAreaRect(this, rectAxisName);

	if (m_bDisplayAxisName && !bCustomAxisNameRect && IsAxisVisible())
	{
		CBCGPSize szAxisNameSize = m_pChart->OnCalcAxisNameSize(pGM, this);
		double dblResize = 0;

		if (IsDiagram3D())
		{
			if (IsVertical())
			{
 				rectPlotArea.left += szAxisNameSize.cx + m_dblLabelDistance + m_dblAxisNameDistance3D;
 				rectPlotArea.right -= szAxisNameSize.cx + m_dblLabelDistance + m_dblAxisNameDistance3D;
			}
			else
			{
				if (GetComponentIndex() == CBCGPChartData::CI_X ||
					GetComponentIndex() == CBCGPChartData::CI_Y)
				{
					CBCGPChartAxis* pZAzis = m_pChart->GetChartAxis(BCGP_CHART_Z_PRIMARY_AXIS);
					CBCGPSize szAxisNameZ = m_pChart->OnCalcAxisNameSize(pGM, pZAzis);

					rectPlotArea.bottom -= max(szAxisNameZ.cy, szAxisNameSize.cy) + m_dblAxisNameDistance3D + m_dblLabelDistance;
				}
			}
			m_rectAxisName.SetRect(0, 0, szAxisNameSize.cx, szAxisNameSize.cy);
			return;
		}

		if (IsVertical())				
		{	
			if (m_bIsSecondaryAxis && rectChartArea.right - rectPlotArea.right < szAxisNameSize.cx)
			{
				dblResize = szAxisNameSize.cx - (rectChartArea.right - rectPlotArea.right);
			}
			else if (!m_bIsSecondaryAxis && rectPlotArea.left - rectChartArea.left < szAxisNameSize.cx)
			{
				dblResize = szAxisNameSize.cx - (rectPlotArea.left - rectChartArea.left);
			}
		}
		else
		{
			if (m_bIsSecondaryAxis && rectPlotArea.top - rectChartArea.top < szAxisNameSize.cy)
			{
				dblResize = szAxisNameSize.cy - (rectPlotArea.top - rectChartArea.top);
			}
			else if (!m_bIsSecondaryAxis && rectChartArea.bottom - rectPlotArea.bottom < szAxisNameSize.cy)
			{
				dblResize = szAxisNameSize.cy - (rectChartArea.bottom - rectPlotArea.bottom);
			}
		}

		if (!bReposOnly)
		{
			if (m_bIsSecondaryAxis)
			{
				IsVertical() ? rectPlotArea.right -= dblResize : rectPlotArea.top += dblResize;
			}
			else
			{
				IsVertical() ? rectPlotArea.left += dblResize : rectPlotArea.bottom -= dblResize;
			}

			if (m_bIsSecondaryAxis)
			{
				IsVertical() ? 
					m_rectAxisName.SetRect(rectPlotArea.right + 1, rectPlotArea.CenterPoint().y - szAxisNameSize.cy / 2, 
							rectPlotArea.right + szAxisNameSize.cx + 1, rectPlotArea.CenterPoint().y + szAxisNameSize.cy / 2) :
					m_rectAxisName.SetRect(rectPlotArea.CenterPoint().x - szAxisNameSize.cx / 2, rectPlotArea.top - szAxisNameSize.cy + 1, 
							rectPlotArea.CenterPoint().x + szAxisNameSize.cx / 2, rectPlotArea.top + 1);
				
			}
			else
			{
				IsVertical() ? 
					m_rectAxisName.SetRect(rectPlotArea.left - szAxisNameSize.cx, rectPlotArea.CenterPoint().y - szAxisNameSize.cy / 2, 
							rectPlotArea.left - 1, rectPlotArea.CenterPoint().y + szAxisNameSize.cy / 2) :
					m_rectAxisName.SetRect(rectPlotArea.CenterPoint().x - szAxisNameSize.cx / 2, rectPlotArea.bottom + 1, 
							rectPlotArea.CenterPoint().x + szAxisNameSize.cx / 2, rectPlotArea.bottom + szAxisNameSize.cy + 1);
			}
		}
		else
		{
			double dblAxisSize = GetAxisSize();

			if (IsVertical())
			{
				m_rectAxisName.top = m_ptAxisStart.y - dblAxisSize / 2 - szAxisNameSize.cy / 2;
				m_rectAxisName.bottom = m_rectAxisName.top + szAxisNameSize.cy;
			}
			else
			{
				m_rectAxisName.left = m_ptAxisStart.x + dblAxisSize / 2 - szAxisNameSize.cx / 2;
				m_rectAxisName.right = m_rectAxisName.left + szAxisNameSize.cx;
			}
		}

		m_rectAxisName.Normalize();
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld)
{
	ASSERT_VALID(this);

	m_axisNameFormat.OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);
	m_axisLabelsFormat.OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);

	m_axisLineFormat.SetScaleRatio(sizeScaleRatioNew);
	m_majorGridLineFormat.SetScaleRatio(sizeScaleRatioNew);
	m_minorGridLineFormat.SetScaleRatio(sizeScaleRatioNew);
}	
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetMajorTickMarkLen(int nLen) 
{
	m_nMajorTickMarkLen = nLen;

	if (m_pChart != NULL && IsDiagram3D())
	{
		m_pChart->GetDiagram3D()->m_bLayoutChanged = TRUE;
		m_pChart->SetDirty(TRUE);
	}
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::GetMajorTickMarkLen(BOOL bScaled) const
{
	ASSERT_VALID(this);

	double dblLen = m_nMajorTickMarkLen;

	if (bScaled && m_pChart != NULL)
	{
		ASSERT_VALID(m_pChart);
		dblLen = IsVertical() ? m_pChart->GetScaleRatio().cx * m_nMajorTickMarkLen : m_pChart->GetScaleRatio().cy * m_nMajorTickMarkLen;
	}

	if (m_bThumbnailMode)
	{
		dblLen = 0.5 * dblLen;
	}

	return dblLen;
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::GetMinorTickMarkLen(BOOL bScaled) const
{
	ASSERT_VALID(this);

	double dblLen = m_nMinorTickMarkLen;
	
	if (bScaled && m_pChart != NULL)
	{
		ASSERT_VALID(m_pChart);
		dblLen = IsVertical() ? m_pChart->GetScaleRatio().cx * m_nMinorTickMarkLen : m_pChart->GetScaleRatio().cy * m_nMinorTickMarkLen;
	}
	
	if (m_bThumbnailMode)
	{
		dblLen = 0.5 * dblLen;
	}
	
	return dblLen;
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::GetLabelDistance() const
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return m_dblLabelDistance + m_nMajorTickMarkLen;
	}

	ASSERT_VALID(m_pChart);

	CBCGPSize szScaleRatio = m_pChart->GetScaleRatio();

	if (m_pChart->IsChart3D() && m_pChart->GetDiagram3D()->IsThickWallsAndFloor())
	{
		double dblPading = m_bThumbnailMode ? 0. : 6.;

		return (m_dblLabelDistance + dblPading) * (IsVertical() ? szScaleRatio.cx : szScaleRatio.cy);
	}

	return m_dblLabelDistance * (IsVertical() ? szScaleRatio.cx : szScaleRatio.cy) + GetMajorTickMarkLen(TRUE);
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::GetFixedIntervalWidthScaled() const
{
	ASSERT_VALID(this);
	
	if (m_pChart == NULL)
	{
		return m_nFixedIntervalWidth;
	}
	
	ASSERT_VALID(m_pChart);

	CBCGPSize szScaleRatio = m_pChart->GetScaleRatio();
	return IsVertical() ? m_nFixedIntervalWidth * szScaleRatio.cy : m_nFixedIntervalWidth * szScaleRatio.cx;
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::GetMajorUnitSpacingScaled() const
{
	ASSERT_VALID(this);
	
	if (m_pChart == NULL)
	{
		return m_nMajorUnitSpacing;
	}
	
	ASSERT_VALID(m_pChart);
	
	CBCGPSize szScaleRatio = m_pChart->GetScaleRatio();
	return IsVertical() ? m_nMajorUnitSpacing * szScaleRatio.cy : m_nMajorUnitSpacing * szScaleRatio.cx;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetScrollBarSize(double dblSize) 
{
	m_dblScrollBarSize = bcg_clamp(dblSize, 4., 100.);
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::CanShowScrollBar() const 
{
	if (m_pChart != NULL && m_pChart->IsThumbSizeMode() && m_pChart->GetThumbSizeAxis() == this)
	{
		return TRUE;
	}

	if (GetMinScrollValue() == -DBL_MAX && GetMaxScrollValue() == DBL_MAX && !IsAlwaysShowScrollBar())
	{
		return FALSE;
	}

	if (GetMinDisplayedValue() <= GetMinScrollValue() && GetMaxDisplayedValue() >= GetMaxScrollValue() && 
		!IsAlwaysShowScrollBar() || IsDiagram3D() || CanUseScaleBreaks())
	{
		return FALSE;
	}

	return (m_bShowScrollBar || IsAlwaysShowScrollBar()) && IsScrollEnabled() && !IsLogScale();
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::GetScrollBarSize(BOOL bScaled) const
{
	ASSERT_VALID(this);

	if (!bScaled || m_pChart == NULL)
	{
		return m_dblScrollBarSize;
	}

	ASSERT_VALID(m_pChart);

	return IsVertical() ? m_pChart->GetScaleRatio().cx * m_dblScrollBarSize : m_pChart->GetScaleRatio().cy * m_dblScrollBarSize;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetMinThumbSize(double dblSize)
{
	m_dblMinThumbSize = bcg_clamp(dblSize, 4., 1000.);
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::GetMinThumbSize(BOOL bScaled) const
{
	ASSERT_VALID(this);

	if (!bScaled || m_pChart == NULL)
	{
		return m_dblMinThumbSize;
	}

	ASSERT_VALID(m_pChart);

	return IsVertical() ? m_pChart->GetScaleRatio().cy * m_dblMinThumbSize : m_pChart->GetScaleRatio().cx * m_dblMinThumbSize;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::AdjustDiagramLeftSide(CBCGPRect& rectDiagramArea, const CBCGPRect& rectPlotArea, double& xOffset, double dblWidth, double dblDataTableOffset)
{
	double dx = dblWidth - (rectDiagramArea.left - rectPlotArea.left);
	xOffset += dx;

	if (xOffset > dblDataTableOffset)
	{
		rectDiagramArea.left += dx;
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::AdjustDiagramArea(CBCGPRect& rectDiagramArea, const CBCGPRect& rectPlotArea)
{
	ASSERT_VALID(this);

	BOOL bShowLabels = GetAxisLabelType() != CBCGPChartAxis::ALT_NO_LABELS;

	if (!IsAxisVisible() || !bShowLabels && !CanShowScrollBar() || GetPerpendecularAxis() == NULL)
	{
		return;
	}

	double dblDataTableOffset = 0.0;
	double xOffset = 0.0;

	if (m_pChart != NULL)
	{
		dblDataTableOffset = m_pChart->GetDataTableHeaderColumnWidth();
	}

	CBCGPSize szAxisLabels = m_szMaxLabelSize; 
	CBCGPChartAxis::CrossType crossType = IsDiagram3D () ? CBCGPChartAxis::CT_IGNORE : m_crossType;

	double dblWidth = bShowLabels ? szAxisLabels.cx + GetLabelDistance() : 0;
	double dblHeight = bShowLabels ? szAxisLabels.cy + GetLabelDistance() : 0;

	double dblScrollBarSize = GetScrollBarSize(TRUE);

	if (CanShowScrollBar())
	{
		IsVertical() ? dblWidth += dblScrollBarSize : dblHeight += dblScrollBarSize;
	}

	BOOL bReverseOrder = GetPerpendecularAxis()->m_bReverseOrder;

	switch (GetAxisLabelType())
	{
	case CBCGPChartAxis::ALT_NO_LABELS:
	case CBCGPChartAxis::ALT_NEXT_TO_AXIS:
		if (m_crossType == CBCGPChartAxis::CT_MAXIMUM_AXIS_VALUE && !bReverseOrder || 
				bReverseOrder && !m_bIsSecondaryAxis)
		{
			if (IsVertical() && rectPlotArea.right - rectDiagramArea.right < dblWidth)
			{
				rectDiagramArea.right -= dblWidth - (rectPlotArea.right - rectDiagramArea.right);
			}
			else if (!IsVertical() && rectDiagramArea.top - rectPlotArea.top < dblHeight)
			{
				rectDiagramArea.top += dblHeight - (rectDiagramArea.top - rectPlotArea.top);
			}
		}
		else if (crossType == CBCGPChartAxis::CT_MAXIMUM_AXIS_VALUE && bReverseOrder && 
				 m_bIsSecondaryAxis)
		{
			if (IsVertical() && rectDiagramArea.left - rectPlotArea.left < dblWidth)
			{
				AdjustDiagramLeftSide(rectDiagramArea, rectPlotArea, xOffset, dblWidth, dblDataTableOffset);
			}
			else if (!IsVertical() &&  rectPlotArea.bottom - rectDiagramArea.bottom < dblHeight)
			{
				rectDiagramArea.bottom -= dblHeight - (rectPlotArea.bottom - rectDiagramArea.bottom);
			}
		}
		else if (crossType == CBCGPChartAxis::CT_AXIS_VALUE || crossType == CBCGPChartAxis::CT_AUTO ||
				 crossType == CBCGPChartAxis::CT_MINIMUM_AXIS_VALUE || 
				 crossType == CBCGPChartAxis::CT_IGNORE)
		{
			if (m_axisDefaultPosition == CBCGPChartAxis::ADP_LEFT)
			{
				if (IsVertical() && rectDiagramArea.left - rectPlotArea.left < dblWidth)
				{
					AdjustDiagramLeftSide(rectDiagramArea, rectPlotArea, xOffset, dblWidth, dblDataTableOffset);
				}
				else if (!IsVertical() &&  rectPlotArea.bottom - rectDiagramArea.bottom < dblHeight)
				{
					rectDiagramArea.bottom -= dblHeight - (rectPlotArea.bottom - rectDiagramArea.bottom);
				}
			}
			else if (m_axisDefaultPosition == CBCGPChartAxis::ADP_RIGHT && 
				rectPlotArea.right - rectDiagramArea.right < dblWidth)
			{
				if (IsVertical() && rectPlotArea.right - rectDiagramArea.right < dblWidth)
				{
					rectDiagramArea.right -= dblWidth - (rectPlotArea.right - rectDiagramArea.right);
				}
				else if (!IsVertical() && rectDiagramArea.top - rectPlotArea.top < dblHeight)
				{
					rectDiagramArea.top += dblHeight - (rectDiagramArea.top - rectPlotArea.top);
				}
				
			}
			else if (m_axisDefaultPosition == CBCGPChartAxis::ADP_TOP)
			{
				if (!IsVertical() && rectDiagramArea.top - rectPlotArea.top < dblHeight)
				{
					rectDiagramArea.top += dblHeight - (rectDiagramArea.top - rectPlotArea.top);
				}
				else if (IsVertical() && rectPlotArea.right - rectDiagramArea.right < dblWidth)
				{
					rectDiagramArea.right -= dblWidth - (rectPlotArea.right - rectDiagramArea.right);
				}
				
			}
			else if (m_axisDefaultPosition == CBCGPChartAxis::ADP_BOTTOM)
			{
				if (!IsVertical() && rectPlotArea.bottom - rectDiagramArea.bottom < dblHeight)
				{
					rectDiagramArea.bottom -= dblHeight - (rectPlotArea.bottom - rectDiagramArea.bottom);
				}
				else if (IsVertical() && rectDiagramArea.left - rectPlotArea.left < dblWidth)
				{
					AdjustDiagramLeftSide(rectDiagramArea, rectPlotArea, xOffset, dblWidth, dblDataTableOffset);
				}
			}
		}
		break;

	case CBCGPChartAxis::ALT_LOW:
		if (IsVertical() && rectDiagramArea.left - rectPlotArea.left < dblWidth)
		{
			AdjustDiagramLeftSide(rectDiagramArea, rectPlotArea, xOffset, dblWidth, dblDataTableOffset);
		}
		else if (!IsVertical() && rectPlotArea.bottom - rectDiagramArea.bottom < dblHeight)
		{
			rectDiagramArea.bottom -= dblHeight - (rectPlotArea.bottom - rectDiagramArea.bottom);
		}
		break;

	case CBCGPChartAxis::ALT_HIGH:
		if (IsVertical() && rectPlotArea.right - rectDiagramArea.right < dblWidth)
		{
			rectDiagramArea.right -= dblWidth - (rectPlotArea.right - rectDiagramArea.right);
		}
		else if (!IsVertical() && rectDiagramArea.top - rectPlotArea.top < dblHeight)
		{
			rectDiagramArea.top += dblHeight - (rectDiagramArea.top - rectPlotArea.top);
		}
		break;
	}
}

void CBCGPChartAxis::UpdateAxisPos(const CBCGPRect& rectDiagramArea)
{
	ASSERT_VALID(this);

	if (m_bIsSecondaryAxis)
	{
		CBCGPPoint ptStart;
		CBCGPPoint ptEnd;

		CBCGPChartAxis* pOpposite = GetOppositeAxis();
		pOpposite->GetAxisPos(ptStart, ptEnd);

		if (IsVertical())
		{
			m_ptAxisStart.y = ptStart.y;
			m_ptAxisEnd.y = ptEnd.y;
		}
		else
		{
			m_ptAxisStart.x = ptStart.x;
			m_ptAxisEnd.x = ptEnd.x;
		}

		if (IsScaleBreakEnabled() && m_scaleBreaks.GetCount() > 0)
		{
			UpdateScaleParts();
		}

		return;
	}

	CBCGPRect rectUpdated = rectDiagramArea;

	int nTopGap = 0; 
	
	if (m_dblTopOffset != 0 && m_dblBottomOffset != 0 || 
		m_dblTopOffset != 0 && m_dblBottomOffset == 0)
	{
		nTopGap = (int)GetAxisGapScaled();
	}

	if (IsVertical())
	{
		rectUpdated.bottom -= rectDiagramArea.Height() * m_dblBottomOffset / 100;
		rectUpdated.top += rectDiagramArea.Height() * m_dblTopOffset / 100 + nTopGap;
		
		m_ptAxisStart.y = m_bReverseOrder ? rectUpdated.top : rectUpdated.bottom;
		m_ptAxisEnd.y = m_bReverseOrder ? rectUpdated.bottom : rectUpdated.top;
	}
	else
	{
		rectUpdated.left += rectDiagramArea.Width() * m_dblBottomOffset / 100;
		rectUpdated.right -= rectDiagramArea.Width() * m_dblTopOffset / 100 + nTopGap;

		m_ptAxisStart.x = m_bReverseOrder ? rectUpdated.right : rectUpdated.left;
		m_ptAxisEnd.x = m_bReverseOrder ? rectUpdated.left : rectUpdated.right;
	}

	if (m_crossType == CT_FIXED_DEFAULT_POS)
	{
		switch (m_axisDefaultPosition)
		{
		case ADP_LEFT:
			m_ptAxisStart.x = rectUpdated.left;
			m_ptAxisEnd.x = rectUpdated.left;
			break;

		case ADP_BOTTOM:
			m_ptAxisStart.y = rectUpdated.bottom;
			m_ptAxisEnd.y = rectUpdated.bottom;
			break;

		case ADP_RIGHT:
			m_ptAxisStart.x = rectUpdated.right;
			m_ptAxisEnd.x = rectUpdated.right;
			break;

		case ADP_TOP:
			m_ptAxisStart.y = rectUpdated.top;
			m_ptAxisEnd.y = rectUpdated.top;
			break;
		}
	}

	if (IsScaleBreakEnabled() && m_scaleParts.GetCount() > 0)
	{
		UpdateScaleParts();
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::CalcLabelsRect(CBCGPRect& rectDiagramArea)
{
	ASSERT_VALID(this);

	CalcAxisPos(rectDiagramArea);

	if (!IsAxisVisible() || GetAxisLabelType() == CBCGPChartAxis::ALT_NO_LABELS || GetPerpendecularAxis() == NULL)
	{
		m_rectAxisLabels.SetRectEmpty();
		return;
	}

	double nLabelRectSize = IsVertical() ? m_szMaxLabelSize.cx : m_szMaxLabelSize.cy; 

	double dblDistance = GetLabelDistance();

	CBCGPPoint ptAxisStart = m_ptAxisStart;
	CBCGPPoint ptAxisEnd = m_ptAxisEnd;

	m_rectAxisLabels.SetRect(m_ptAxisStart, m_ptAxisEnd);

	CBCGPChartAxis* pOppositeAxis = GetOppositeAxis();

	BOOL bOppositeAxisVisible = pOppositeAxis != NULL && 
								pOppositeAxis->IsAxisVisible() && 
								!pOppositeAxis->m_rectAxisLabels.IsRectEmpty();

	CBCGPChartAxis::AxisLabelsType labelStyle = GetAxisLabelType();

	BOOL bReverseOrder = GetPerpendecularAxis()->m_bReverseOrder;

	BOOL bRectSet = FALSE;

	switch(labelStyle)
	{
	case CBCGPChartAxis::ALT_NEXT_TO_AXIS:
		if (IsVertical())
		{
			if (m_crossType == CBCGPChartAxis::CT_MAXIMUM_AXIS_VALUE && !bReverseOrder || 
				bReverseOrder && !m_bIsSecondaryAxis)
			{
				m_rectAxisLabels.left = m_ptAxisStart.x + dblDistance;
			}
			else if (m_crossType != CBCGPChartAxis::CT_MAXIMUM_AXIS_VALUE ||
				m_crossType == CBCGPChartAxis::CT_MAXIMUM_AXIS_VALUE && bReverseOrder && m_bIsSecondaryAxis)
			{
				m_rectAxisLabels.left = m_ptAxisStart.x - dblDistance - nLabelRectSize;
			}

			m_rectAxisLabels.right = m_rectAxisLabels.left + nLabelRectSize;
		}
		else
		{
			if (m_crossType == CBCGPChartAxis::CT_MAXIMUM_AXIS_VALUE && !bReverseOrder || 
				bReverseOrder && !m_bIsSecondaryAxis)
			{
				m_rectAxisLabels.top = m_ptAxisStart.y - dblDistance - nLabelRectSize;		
			}
			else if (m_crossType != CBCGPChartAxis::CT_MAXIMUM_AXIS_VALUE ||
				m_crossType == CBCGPChartAxis::CT_MAXIMUM_AXIS_VALUE && bReverseOrder && m_bIsSecondaryAxis)
			{
				m_rectAxisLabels.top = m_ptAxisStart.y + dblDistance;
			}
			
			m_rectAxisLabels.bottom = m_rectAxisLabels.top + nLabelRectSize;
		}
		break;

	case CBCGPChartAxis::ALT_LOW:
		if (m_bIsSecondaryAxis && bOppositeAxisVisible)
		{
			if (IsVertical())
			{
				if (pOppositeAxis->m_rectAxisLabels.left < rectDiagramArea.left)
				{
					m_rectAxisLabels.left = pOppositeAxis->m_rectAxisLabels.left - dblDistance * 2 - nLabelRectSize;
					m_rectAxisLabels.right = m_rectAxisLabels.left + nLabelRectSize;
					bRectSet = TRUE;
				}
			}
			else
			{
				if (pOppositeAxis->m_rectAxisLabels.bottom > rectDiagramArea.bottom)
				{
					m_rectAxisLabels.top = pOppositeAxis->m_rectAxisLabels.bottom + dblDistance * 2;
					m_rectAxisLabels.bottom = m_rectAxisLabels.top + nLabelRectSize;
					bRectSet = TRUE;
				}
			}
		}

		if (!bRectSet)
		{
			if (IsVertical())
			{
				m_rectAxisLabels.left = rectDiagramArea.left - dblDistance - nLabelRectSize;
				m_rectAxisLabels.right = m_rectAxisLabels.left + nLabelRectSize;
			}
			else
			{
				m_rectAxisLabels.top = rectDiagramArea.bottom + dblDistance;
				m_rectAxisLabels.bottom = m_rectAxisLabels.top + nLabelRectSize;
			}
		}
		break;

	case CBCGPChartAxis::ALT_HIGH:			
		
		if (m_bIsSecondaryAxis && bOppositeAxisVisible)
		{
			if (IsVertical())
			{
				if (pOppositeAxis->m_rectAxisLabels.right > rectDiagramArea.right)
				{
					m_rectAxisLabels.left = pOppositeAxis->m_rectAxisLabels.right + dblDistance * 2;
					m_rectAxisLabels.right = m_rectAxisLabels.left + nLabelRectSize;
					bRectSet = TRUE;
				}
			}
			else
			{
				if (pOppositeAxis->m_rectAxisLabels.top < rectDiagramArea.top)
				{
					m_rectAxisLabels.top = pOppositeAxis->m_rectAxisLabels.top - dblDistance * 2 - nLabelRectSize;
					m_rectAxisLabels.bottom = m_rectAxisLabels.top + nLabelRectSize;
					bRectSet = TRUE;
				}
			}
		}

		if (!bRectSet)
		{
			if (IsVertical())
			{
				m_rectAxisLabels.left = rectDiagramArea.right + dblDistance;
				m_rectAxisLabels.right = m_rectAxisLabels.left + nLabelRectSize;
			}
			else
			{
				m_rectAxisLabels.top = rectDiagramArea.top - dblDistance - nLabelRectSize;
				m_rectAxisLabels.bottom = m_rectAxisLabels.top + nLabelRectSize;
			}
		}
		break;
	}

	m_rectAxisLabels.Normalize();

	if (CanShowScrollBar() && !IsDiagram3D())
	{
		double dblScrollBarSize = GetScrollBarSize(TRUE);
		// offset labels only if they are located at the same axis side as scroll bar
		if (IsVertical())
		{
			if (m_rectScrollBar.left >= m_ptAxisStart.x && m_rectScrollBar.right > m_ptAxisStart.x && 
				m_rectAxisLabels.left > m_ptAxisStart.x)
			{
				m_rectAxisLabels.OffsetRect(dblScrollBarSize, 0);
			}
			else if (m_rectScrollBar.right <= m_ptAxisStart.x && m_rectScrollBar.left < m_ptAxisStart.x && 
				m_rectAxisLabels.right < m_ptAxisStart.x)
			{
				m_rectAxisLabels.OffsetRect(-dblScrollBarSize, 0);
			}
		}
		else
		{
			if (m_rectScrollBar.top >= m_ptAxisStart.y && m_rectScrollBar.bottom > m_ptAxisStart.y && 
				m_rectAxisLabels.top > m_ptAxisStart.y)
			{
				m_rectAxisLabels.OffsetRect(0, dblScrollBarSize);
			}
			else if (m_rectScrollBar.bottom <= m_ptAxisStart.y && m_rectScrollBar.top < m_ptAxisStart.y && 
				m_rectAxisLabels.bottom < m_ptAxisStart.y)
			{
				m_rectAxisLabels.OffsetRect(0, -dblScrollBarSize);
			}
		}
	}

	OnCalcDefaultTextAlignment(0, m_rectAxisLabels, rectDiagramArea, m_axisLabelsFormat.m_textFormat);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::CalcAxisPos(const CBCGPRect& rectDiagram, BOOL bStoreAxisPos)
{
	ASSERT_VALID(this);

	if (IsDiagram3D())
	{
		return;
	}

	CBCGPPoint ptStart = CBCGPPoint(rectDiagram.left, rectDiagram.bottom);
	CBCGPPoint ptEnd = CBCGPPoint(IsVertical() ? rectDiagram.left : rectDiagram.right, 
		IsVertical() ? rectDiagram.top : rectDiagram.bottom);

	if (m_pChart == NULL)
	{
		m_ptAxisStart = ptStart;
		m_ptAxisEnd = ptEnd;
		return;
	}

	double dblCrossValue = GetCrossValuePos(TRUE);

	if (IsVertical())
	{
		if (dblCrossValue > rectDiagram.right)
		{
			dblCrossValue = rectDiagram.right;
		}
		else if (dblCrossValue < rectDiagram.left)
		{
			dblCrossValue = rectDiagram.left;
		}
		ptStart.x = ptEnd.x = dblCrossValue;
	}
	else
	{
		if (dblCrossValue < rectDiagram.top)
		{
			dblCrossValue = rectDiagram.top;
		}
		else if (dblCrossValue > rectDiagram.bottom)
		{
			dblCrossValue = rectDiagram.bottom;
		}
		ptStart.y = ptEnd.y = dblCrossValue;
	}

	if (m_bIsSecondaryAxis)
	{
		CBCGPChartAxis* pPrimaryAxis = GetOppositeAxis();
		if (GetAxisLabelType() == CBCGPChartAxis::ALT_HIGH && pPrimaryAxis->GetAxisLabelType() == 
			CBCGPChartAxis::ALT_HIGH && pPrimaryAxis->IsAxisVisible())  
		{
			if (IsVertical())
			{
				ptStart.x = ptEnd.x = pPrimaryAxis->m_rectAxisLabels.right + GetMajorTickMarkLen(TRUE);
			}
			else
			{
				ptStart.y = ptEnd.y = pPrimaryAxis->m_rectAxisLabels.top - GetMajorTickMarkLen(TRUE);
			}
		}
		else if (GetAxisLabelType() == CBCGPChartAxis::ALT_LOW && pPrimaryAxis->GetAxisLabelType() == 
			CBCGPChartAxis::ALT_LOW && pPrimaryAxis->IsAxisVisible())
		{
			if (IsVertical())
			{
				ptStart.x = ptEnd.x = pPrimaryAxis->m_rectAxisLabels.left - GetMajorTickMarkLen(TRUE);
			}
			else
			{
				ptStart.y = ptEnd.y = pPrimaryAxis->m_rectAxisLabels.bottom + GetMajorTickMarkLen(TRUE);
			}
		}
	}

	if (bStoreAxisPos)
	{
		m_ptPrevAxisStart = m_ptAxisStart;
		m_ptPrevAxisEnd = m_ptAxisEnd;
	}
	

	m_ptAxisStart = ptStart;
	m_ptAxisEnd = ptEnd;

	UpdateAxisPos(rectDiagram);

	if (!bStoreAxisPos)
	{
		if ((m_ptPrevAxisStart.y != m_ptAxisEnd.y && IsVertical() || m_ptPrevAxisEnd.x != m_ptAxisEnd.x && !IsVertical()) && m_bIsFixedIntervalWidth && m_bFixedRange && m_bZoomed)
		{
			m_bFixedRange = FALSE;
			m_bFixedMaximum = TRUE;
		}
	}

	m_rectScrollBar.SetRectEmpty();
	m_bTickMarksTopRight = FALSE;

	if (CanShowScrollBar())
	{
		if (IsVertical())
		{
			if (m_ptAxisStart.x >= rectDiagram.right)
			{
				m_rectScrollBar.SetRect(m_ptAxisEnd.x, m_ptAxisEnd.y, 
					m_ptAxisStart.x + GetScrollBarSize(TRUE), m_ptAxisStart.y);
				m_bTickMarksTopRight = TRUE;
			}
			else
			{
				m_rectScrollBar.SetRect(m_ptAxisEnd.x, m_ptAxisEnd.y, 
					m_ptAxisStart.x - GetScrollBarSize(TRUE), m_ptAxisStart.y);
			}
		}
		else
		{
			if (m_ptAxisStart.y <= rectDiagram.top)
			{
				m_rectScrollBar.SetRect(m_ptAxisStart.x, m_ptAxisStart.y - GetScrollBarSize(TRUE), 
					m_ptAxisEnd.x, m_ptAxisEnd.y);
				m_bTickMarksTopRight = TRUE;
			}
			else
			{
				m_rectScrollBar.SetRect(m_ptAxisStart.x, m_ptAxisStart.y, 
					m_ptAxisEnd.x, m_ptAxisEnd.y + GetScrollBarSize(TRUE));
			}
		}

		m_rectScrollBar.Normalize();
	}
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::GetCrossValuePos(BOOL bMapToDiagramArea)
{
	ASSERT_VALID(this);

	CBCGPChartAxis* pPerpendecularAxis = GetPerpendecularAxis();
	double dblValue = 0.;

	if (pPerpendecularAxis == NULL)
	{
		return dblValue;
	}

	if (m_crossType == CBCGPChartAxis::CT_IGNORE || IsDiagram3D())
	{
		if (m_axisDefaultPosition == CBCGPChartAxis::ADP_LEFT || m_axisDefaultPosition == CBCGPChartAxis::ADP_BOTTOM ||
			m_axisDefaultPosition == CBCGPChartAxis::ADP_DEPTH_BOTTOM)
		{
			dblValue = pPerpendecularAxis->GetMinDisplayedValue();
		}
		else if (m_axisDefaultPosition == CBCGPChartAxis::ADP_RIGHT || m_axisDefaultPosition == CBCGPChartAxis::ADP_TOP ||
			m_axisDefaultPosition == CBCGPChartAxis::ADP_DEPTH_TOP)
		{
 			CBCGPPoint ptAxisStart;
 			CBCGPPoint ptAxisEnd;
 			pPerpendecularAxis->GetAxisPos(ptAxisStart, ptAxisEnd);
			dblValue = pPerpendecularAxis->ValueFromPoint(ptAxisEnd);
		}
	}
	else
	{
		dblValue = GetNonIgnoredCrossValue();
	}

	if (bMapToDiagramArea)
	{
		dblValue = pPerpendecularAxis->PointFromValue(dblValue, TRUE);
	}

	return dblValue;
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::GetNonIgnoredCrossValue() const
{
	ASSERT_VALID(this);

	CBCGPChartAxis* pPerpendecularAxis = GetPerpendecularAxis();
	double dblValue = 0.;

	if (pPerpendecularAxis == NULL)
	{
		return dblValue;
	}

	switch (m_crossType)
	{
	case CBCGPChartAxis::CT_AUTO:
		if (pPerpendecularAxis->GetMinDisplayedValue() < 0. && pPerpendecularAxis->GetMaxDisplayedValue() > 0.)
		{
			dblValue = pPerpendecularAxis->IsComponentXSet() ? 0. : 1.;
		}
		else if (pPerpendecularAxis->GetMinDisplayedValue() >= 0.)
		{
			dblValue = pPerpendecularAxis->GetMinDisplayedValue(TRUE);
		}
		else if (pPerpendecularAxis->GetMaxDisplayedValue() <= 0.)
		{
			dblValue = pPerpendecularAxis->GetMaxDisplayedValue(TRUE);
		}
		break;

	case CBCGPChartAxis::CT_AXIS_VALUE:
		dblValue = m_dblCrossOtherAxisValue;
		break;

	case CBCGPChartAxis::CT_MAXIMUM_AXIS_VALUE:
		dblValue = pPerpendecularAxis->GetMaxDisplayedValue(TRUE);
		break;

	case CBCGPChartAxis::CT_MINIMUM_AXIS_VALUE:
		dblValue = pPerpendecularAxis->GetMinDisplayedValue(TRUE);
		break;
	}

	return dblValue;
}
//----------------------------------------------------------------------------//
int CBCGPChartAxis::GetNumberOfUnitsToDisplay(UINT nUnitsToDisplay)
{
	ASSERT_VALID(this);

	if (m_bEmptySeries)
	{
		CBCGPChartAxis* pOppositeAxis = GetOppositeAxis();
		if (pOppositeAxis != NULL && !pOppositeAxis->m_bEmptySeries)
		{
			return pOppositeAxis->GetNumberOfUnitsToDisplay(nUnitsToDisplay);
		}

		return 0;
	}

	if (m_bIsFxedUnitCount)
	{
		return m_nFixedUnitCount;
	}

	if (m_szMaxLabelSize.cx == 0)
	{
		m_szMaxLabelSize.cx = 16;
		m_szMaxLabelSize.cy = 16;
	}

	double dblDiagramSize = fabs(GetAxisSize());

	if (m_bIsFixedIntervalWidth)
	{
		if (m_bStretchFixedIntervals)
		{
			return (int) (dblDiagramSize / GetFixedIntervalWidthScaled()) * m_nValuesPerInterval;
		}
		
		double dblIntervalSize = GetFixedIntervalWidthScaled();
 		int nTotalFullIntervals = (int)(dblDiagramSize / dblIntervalSize);
		int nTotalValues = nTotalFullIntervals * m_nValuesPerInterval;

 		double dblDiffSize = dblDiagramSize - nTotalFullIntervals * dblIntervalSize;
 		int nValueSize = (int) (dblIntervalSize / m_nValuesPerInterval);

 		return nTotalValues + (int)(dblDiffSize / nValueSize) - 1;
	}

	double dblMajorUnitSize = IsVertical() ? m_szMaxLabelSize.cy * 1.1 : m_szMaxLabelSize.cx * 1.1;
	int nDisplayedUnits = nUnitsToDisplay == -1 ?((int)(dblDiagramSize / dblMajorUnitSize)) : nUnitsToDisplay;

	int nMaxTotalLines = GetMaxTotalLines();
	if (nDisplayedUnits > nMaxTotalLines || GetMajorUnitSpacing() != 0)
	{
		nDisplayedUnits = nMaxTotalLines;
	}

	if (nDisplayedUnits < 1)
	{
		nDisplayedUnits = 1;
	}

	return nDisplayedUnits;
}
//----------------------------------------------------------------------------//
int CBCGPChartAxis::GetRightOffsetAsNumberOfValues() const
{
	if (!IsFixedIntervalWidth())
	{
		return 0;
	}

	int nPixelsPerValue = max(1, GetFixedIntervalWidth() / GetValuesPerInterval());
	return GetRightOffsetInPixels() / nPixelsPerValue;
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::UnitsInIntervalFromMajorUnitSpacing()
{
	double dblUnits = m_dblMajorUnit;

	if (GetMajorUnitSpacing() != 0)
	{
		CBCGPPoint ptInterval(GetMajorUnitSpacingScaled(), GetMajorUnitSpacingScaled());
		
		if (IsVertical())
		{
			m_bReverseOrder ? ptInterval.y = m_ptAxisStart.y + ptInterval.y : ptInterval.y = m_ptAxisStart.y - ptInterval.y;
		}
		else
		{
			m_bReverseOrder ? ptInterval.x = m_ptAxisStart.x - ptInterval.x : ptInterval.x += m_ptAxisStart.x;
		}
		
		dblUnits = ValueFromPoint(ptInterval) - ValueFromPoint(m_ptAxisStart);
		
		if (dblUnits < m_dblMinAllowedMajorUnit)
		{
			dblUnits = m_dblMinAllowedMajorUnit;
		}

		if (m_dblMinAllowedMajorUnit != 0 && m_dblMinAllowedMajorUnit != dblUnits)
		{
			int nMul = (int)floor(dblUnits / m_dblMinAllowedMajorUnit) + 1;
			dblUnits = m_dblMinAllowedMajorUnit * nMul;
		}
	}

	return dblUnits;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::CalcMajorMinorUnits(CBCGPGraphicsManager* pGM, UINT nUnitsToDisplay)
{
	ASSERT_VALID(this);

	double dblAxisSize = fabs(GetAxisSize());

	if (dblAxisSize == 0 || m_dblMaximum == m_dblMinimum && m_dblMaximum == 0.)
	{
		return;
	}

	m_nDisplayedLines = GetNumberOfUnitsToDisplay(nUnitsToDisplay); 

	if (m_nDisplayedLines == 0)
	{
		m_nDisplayedLines = 1;
	}

	if (m_bFixedMajorUnit)
	{
		if (m_dblMajorUnit == 0)
		{
			m_dblMajorUnit = fabs(m_dblMaximum - m_dblMinimum) / m_nDisplayedLines;
		}

		CalcMinMaxDisplayedValues(TRUE);
		m_bInitialized = TRUE;

		if (!IsIndexedSeries())
		{
			SetScrollRange(GetMinDisplayedValue(), GetMaxDisplayedValue(), FALSE);
		}
		else
		{
			int nDPCount = m_pIndexedSeries->GetDataPointCount();
			if (nDPCount > 0)
			{
				SetScrollRange(0, m_pIndexedSeries->GetDataPointCount() + GetRightOffsetAsNumberOfValues() - 1, TRUE);
			}
		}
		return;
	}

	if (IsLogScale())
	{
		m_dblMajorUnit = m_dblLogScaleBase;
		m_dblMinorUnit = m_dblLogScaleBase;
		m_nMinorUnitCount = (int)(m_dblLogScaleBase - 1);

		CalcMinMaxDisplayedValues(TRUE);
		CalcMaxLabelSize(pGM);

		double dblLabelSpacing = IsVertical() ? m_szMaxLabelSize.cy * 1.2 : m_szMaxLabelSize.cx * 1.2;
		if (dblLabelSpacing > 0)
		{
			double dblRequiredDiagramSize = IsVertical() ? dblAxisSize - m_szMaxLabelSize.cy : dblAxisSize - m_szMaxLabelSize.cx;

			if (dblRequiredDiagramSize < 0)
			{
				dblRequiredDiagramSize = 1.;
			}

			double dblRange = CalcLog(GetMaxDisplayedValue()) - CalcLog(GetMinDisplayedValue());
			double dblTimes = (dblLabelSpacing * dblRange / dblRequiredDiagramSize);

			m_dblMinorUnit = m_dblMajorUnit = pow(m_dblMajorUnit, ceil(dblTimes));
		}
		
		m_bInitialized = TRUE;

		return;
	}

	double dblMajorUnit = 0.;
	double dblRange = 0.;
	BOOL bCalcMinDisplayedValue = FALSE;
	BOOL bMinMaxOnTheSameSide = (m_dblMaximum > 0 && m_dblMinimum >= 0 || m_dblMaximum <= 0 && m_dblMinimum < 0);
	double dblMinMaxRange = fabs(m_dblMaximum - m_dblMinimum);

	if (bMinMaxOnTheSameSide && !IsFixedDisplayRange())
	{
		// both maximums are located on the same side from the zero line
		dblRange = m_dblMaximum > 0. ? m_dblMaximum : fabs(m_dblMinimum);
		dblMajorUnit = CalcMajorUnit(10, dblRange);

		// minimum should be lower than 83% from max to count major unit based on range between min/max
		BOOL bBaseOnRange = FALSE;
		double dblPredefinedRangePercent = fabs(dblRange) * 0.83333; 

		if (m_dblMaximum > 0.)
		{
			bBaseOnRange = m_dblMinimum > dblPredefinedRangePercent;
		}
		else
		{
			bBaseOnRange = fabs(m_dblMaximum) > dblPredefinedRangePercent;
		}

		if ((bBaseOnRange ||  dblMajorUnit > dblMinMaxRange) && dblMinMaxRange > 0.)
		{
			if (!m_bNewMajorUnitBehavior)
			{
				dblRange = dblMinMaxRange;
			}

			bCalcMinDisplayedValue = TRUE;
		}

		if (m_bNewMajorUnitBehavior)
		{
			dblRange = dblMinMaxRange;
			bCalcMinDisplayedValue = TRUE;
		}
	}
	else
	{
		dblRange = dblMinMaxRange;
		bCalcMinDisplayedValue = TRUE;
	}

	m_dblMajorUnit = CalcMajorUnit(m_nDisplayedLines, dblRange);

	if (m_dblMajorUnit > dblRange && IsFixedDisplayRange() && m_nDisplayedLines <= 2)
	{
		m_dblMajorUnit = dblRange;
	}

	if (!m_bIsFixedIntervalWidth && GetMajorUnitSpacing() == 0)
	{
		int nLines = m_nDisplayedLines;

		if (m_nDisplayedLines > 2)
		{
			double dblUnit = m_dblMajorUnit;

			do 
			{
				nLines--;
				if (nLines < 0)
				{
					break;
				}
				dblUnit = CalcMajorUnit(nLines, dblRange);
			}
			while (dblUnit == m_dblMajorUnit);

			nLines++;
		}

		m_nDisplayedLines = nLines;
	}

	if (m_dblMajorUnit < m_dblMinAllowedMajorUnit)
	{
		m_dblMajorUnit = m_dblMinAllowedMajorUnit;
	}

	if (m_dblMinAllowedMajorUnit != 0 && m_dblMinAllowedMajorUnit != m_dblMajorUnit)
	{
		int nMul = (int)floor(m_dblMajorUnit / m_dblMinAllowedMajorUnit) + 1;
		m_dblMajorUnit = m_dblMinAllowedMajorUnit * nMul;
	}

	if (m_dblMajorUnit < DBL_EPSILON && !IsLogScale() && dblRange > DBL_EPSILON)
	{
		m_dblMajorUnit = DBL_EPSILON;
	}

	m_dblMinorUnit = m_dblMajorUnit / m_nMinorUnitCount;

	if (IsFixedMinimumDisplayValue())
	{
		bCalcMinDisplayedValue = FALSE;
	}

	CalcMinMaxDisplayedValues(bCalcMinDisplayedValue);
	CalcMaxLabelSize(pGM);

	// enter recursion only for the first time
	if (m_nDisplayedLines > 3 && nUnitsToDisplay == -1 && !m_bIsFixedIntervalWidth && GetMajorUnitSpacing() == 0)
	{
		double nLabelSpacing = IsVertical() ? m_szMaxLabelSize.cy * 1.2 : m_szMaxLabelSize.cx * 1.2;
		
		double nRequiredDiagramSize = IsVertical() ? dblAxisSize - m_szMaxLabelSize.cy : dblAxisSize - m_szMaxLabelSize.cx;
		
		if (nLabelSpacing * m_nDisplayedLines - 1 > nRequiredDiagramSize)
		{
			int nPrevDisplayedLines = m_nDisplayedLines - 1;
			do 
			{
				nPrevDisplayedLines = m_nDisplayedLines;
				CalcMajorMinorUnits(pGM, m_nDisplayedLines - 1);

				nLabelSpacing = IsVertical() ? m_szMaxLabelSize.cy * 1.2 : m_szMaxLabelSize.cx * 1.2;
				nRequiredDiagramSize = IsVertical() ? dblAxisSize - m_szMaxLabelSize.cy : dblAxisSize - m_szMaxLabelSize.cx;

				if (nLabelSpacing * m_nDisplayedLines < nRequiredDiagramSize)
					break;

			}while (nPrevDisplayedLines < m_nDisplayedLines);
		}
	}

	if (IsScaleBreakEnabled() && m_scaleBreaks.GetCount() > 0)
	{
		CalcMajorMinorUnitsForScaleParts(pGM);
		//return;
	}

	m_bInitialized = TRUE;
	
	SetScrollRange(GetMinDisplayedValue(), GetMaxDisplayedValue(), FALSE);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::CalcMinMaxDisplayedValues(BOOL bCalcMinDisplayedValue, double dblApproximation)
{
	ASSERT_VALID(this);

	// bCalcMinDisplayedValue is FALSE when the chart shows zero line, or min and max are displayed
	// on different sides of zero. Otherwise the chart displays interval [0, max], or [min, 0]

	if (IsFixedDisplayRange() && !IsFixedIntervalWidth())
	{
		m_dblMinDisplayedValue = m_dblMinimum;
		m_dblMaxDisplayedValue = m_dblMaximum;
		return;
	}

	if (IsFixedMaximumDisplayValue())
	{
		m_dblMaxDisplayedValue = m_dblMaximum;
		m_dblMinDisplayedValue = m_dblMaxDisplayedValue - m_nDisplayedLines * m_dblMajorUnit;

		if (!IsNegativeValuesForFixedIntervaleEnabled() && m_bIsFixedIntervalWidth && m_dblMinDisplayedValue < 0)
		{
			m_dblMinDisplayedValue = 0;
			m_dblMaxDisplayedValue = m_nDisplayedLines * m_dblMajorUnit;
		}
		return;
	}

	if (IsFixedMinimumDisplayValue())
	{
		m_dblMinDisplayedValue = m_dblMinimum;
		m_dblMaxDisplayedValue = m_dblMinDisplayedValue + m_nDisplayedLines * m_dblMajorUnit;
		return;
	}

	if (IsLogScale())
	{
		double dblGrade = ceil(CalcLog(m_dblMaximum));
		m_dblMaxDisplayedValue = pow(m_dblMajorUnit, dblGrade);

		if (m_dblMinimum <= 0)
		{
			m_dblMinDisplayedValue = 1.;
		}
		else
		{
			dblGrade = floor(CalcLog(m_dblMinimum));
			dblGrade = dblGrade - 1;
			m_dblMinDisplayedValue = pow(m_dblMajorUnit, dblGrade);
		}
		return;
	}

	if (m_nDisplayedLines < 4)
	{
		dblApproximation = 1.01;
	}

	if (bCalcMinDisplayedValue)
	{
		if (m_bFixedMajorUnit)
		{
			if (IsNegativeValuesForFixedIntervaleEnabled())
			{
				double dblMiddleValue = m_dblMinimum + (m_dblMaximum - m_dblMinimum) / 2;
				m_dblMinDisplayedValue = dblMiddleValue - (m_nDisplayedLines / 2) * m_dblMajorUnit;
				m_dblMaxDisplayedValue = dblMiddleValue + (m_nDisplayedLines / 2) * m_dblMajorUnit;
			}
			else
			{
				m_dblMinDisplayedValue = m_dblMinimum;
				m_dblMaxDisplayedValue = m_nDisplayedLines * m_dblMajorUnit;
			}
			return;
		}

		if (m_dblMaximum > 0 && m_dblMinimum < 0)
		{
			double dblMaxDisplayedVal = ApproximateValue(m_dblMaximum, m_dblMajorUnit, dblApproximation);

			if (!IsFixedMaximumDisplayValue())
			{
				m_dblMaxDisplayedValue = dblMaxDisplayedVal;
			}
			m_dblMinDisplayedValue = m_dblMaxDisplayedValue - (m_nDisplayedLines + 1) * m_dblMajorUnit;

			if (m_dblMinDisplayedValue + m_dblMajorUnit < m_dblMinimum * dblApproximation)
			{
				m_dblMinDisplayedValue = m_dblMinDisplayedValue + m_dblMajorUnit;
			}
		}
		else
		{
			double dblMax = m_dblMaximum > 0. ? m_dblMaximum : fabs(m_dblMinimum);
			double dblMaxDisplayedVal = ApproximateValue(dblMax, m_dblMajorUnit, dblApproximation); 
			int nNumLines = m_nDisplayedLines + 2;

			if (m_nDisplayedLines < 3)
			{
				nNumLines--;
			}

			if (m_dblMaximum > 0)
			{
				if (!IsFixedMaximumDisplayValue())
				{
					m_dblMaxDisplayedValue = dblMaxDisplayedVal;
				}
				m_dblMinDisplayedValue = max(0, m_dblMaxDisplayedValue - nNumLines * m_dblMajorUnit);
			}
			else
			{
				m_dblMinDisplayedValue = -dblMaxDisplayedVal;
				if (!IsFixedMaximumDisplayValue())
				{
					m_dblMaxDisplayedValue = min(0, m_dblMinDisplayedValue + nNumLines * m_dblMajorUnit);
				}
			}
		}
	}
	else
	{
		double dblMaxValue = m_dblMaximum > 0.  ? m_dblMaximum : fabs(m_dblMinimum);

		double dblMaxDisplayedVal = ApproximateValue(dblMaxValue, m_dblMajorUnit, dblApproximation);

		if (m_dblMaximum > 0)
		{
			if (!IsFixedMaximumDisplayValue())
			{
				m_dblMaxDisplayedValue = dblMaxDisplayedVal;
			}
			if (!IsFixedMinimumDisplayValue())
			{
				m_dblMinDisplayedValue = 0;
			}
		}
		else
		{	
			if (!IsFixedMaximumDisplayValue())
			{
				m_dblMaxDisplayedValue = 0;
			}
			if (!IsFixedMinimumDisplayValue())
			{
				m_dblMinDisplayedValue = -dblMaxDisplayedVal;
			}
		}

		if (m_bNewMajorUnitBehavior)
		{
			if (m_dblMinimum >= 0 && !IsFixedMinimumDisplayValue())
			{
				if (m_dblMinimum - m_dblMajorUnit > 2 * m_dblMajorUnit)
				{
					m_dblMinDisplayedValue = max(0, ApproximateValue(m_dblMinimum, m_dblMajorUnit, 1.04) - 2 * m_dblMajorUnit);
				}
				else
				{
					m_dblMinDisplayedValue = 0;
				}
			}
			
			if (m_dblMaximum < 0 && !IsFixedMaximumDisplayValue())
			{
				if (fabs(m_dblMaximum) - m_dblMajorUnit > 2 * m_dblMajorUnit)
				{
					m_dblMaxDisplayedValue = min(0, ApproximateValue(m_dblMaximum, m_dblMajorUnit, 1.04) + 2 * m_dblMajorUnit);
				}
				else
				{
					m_dblMaxDisplayedValue = 0;
				}
			}
		}
	}

	if (!IsNegativeValuesForFixedIntervaleEnabled() && m_bIsFixedIntervalWidth && m_dblMinDisplayedValue < 0)
	{
		m_dblMinDisplayedValue = 0;
	}
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::ApproximateValue(double dblValue, double dblUnit, double dblApproximation)
{
	ASSERT_VALID(this);

	if (IsLogScale() || dblUnit == 0)
	{
		return dblValue;
	}

	double dblDecimalPartUnit = dblUnit - floor(dblUnit);
	if (dblDecimalPartUnit != 0)
	{
		double dblDecimalDigits = fabs(log10(dblDecimalPartUnit)) + 1;
		double dblGrade = pow(10.0, (int) dblDecimalDigits);

		__int64 dblGradedUnit = (__int64) (dblUnit * dblGrade);
		__int64 dblGradedValue = (__int64) (dblValue * dblGrade);

		__int64 nStart = (__int64)((dblGradedValue / dblGradedUnit) * dblGradedUnit);

		while (nStart <= dblGradedValue)
		{
			nStart += dblGradedUnit;
		}

		if (nStart - dblGradedValue * dblApproximation  < 0)
		{
			nStart += dblGradedUnit;
		}

		return nStart / dblGrade;
	}

	if (log10(fabs(dblValue)) > 19 || log10(fabs(dblUnit)) > 19)
	{
		return dblValue >= 0 ? dblValue + dblUnit : dblValue - dblUnit;
	}
	
	__int64 nStart = (__int64)((__int64)(dblValue / dblUnit) * dblUnit);
	while (nStart <= (__int64)dblValue)
	{
		nStart += (__int64)dblUnit;
	}

	if (nStart - (__int64)(dblValue * dblApproximation + 0.5)  < 0)
	{
		nStart += (__int64)dblUnit;
	}

	return (double)nStart;
	
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::CanBeScrolled() const
{
	return IsScrollEnabled() && 
		(m_dblMinDisplayedValue > m_dblMinScrollValue || m_dblMaxDisplayedValue < m_dblMaxScrollValue);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetMaxZoomInFactor(double dblValue)
{
	m_dblMaxZoomInFactor = bcg_clamp(dblValue, 1., 100000.);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::EnableThumbGrip(BOOL bEnable) 
{
	m_bIsThumbGripEnabled = bEnable;

	if (bEnable)
	{
		SetAlwaysShowScrollBar(TRUE);
	}
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::SetThumbRect(const CBCGPPoint& ptHit, BOOL bLeftGrip, double dblOffset)
{
	if (m_rectScrollBar.IsRectEmpty() || !IsZoomEnabled() || !m_bIsThumbGripEnabled)
	{
		return FALSE;
	}

	double dblScrollRange = m_dblMaxScrollValue - m_dblMinScrollValue;

	if (dblScrollRange == 0)
	{
		return FALSE;
	}

	CBCGPPoint ptAxisStart;
	CBCGPPoint ptAxisEnd;

	GetAxisPos(ptAxisStart, ptAxisEnd);

	double dblMinThumbSize = GetMinThumbSize(TRUE);
	double dblAxisSize = fabs(GetAxisSize());

	CBCGPRect rectThumb = m_rectThumb;

	if (IsVertical())
	{
		bLeftGrip ? rectThumb.bottom = ptHit.y + dblOffset : rectThumb.top = ptHit.y  - dblOffset;
		
		if (rectThumb.Height() >= dblAxisSize)
		{
			rectThumb.top = ptAxisEnd.y;
			rectThumb.bottom = ptAxisStart.y;
		}
	}
	else
	{
		bLeftGrip ? rectThumb.left = ptHit.x  - dblOffset: rectThumb.right = ptHit.x + dblOffset;

		if (rectThumb.Width() >= dblAxisSize)
		{
			rectThumb.top = ptAxisEnd.y;
			rectThumb.bottom = ptAxisStart.y;
		}
	}

	CheckThumbMinSize(rectThumb, dblMinThumbSize);
	rectThumb.Normalize();

	if (IsVertical())
	{
		double dblAxisTop = min(ptAxisEnd.y, ptAxisStart.y);
		double dblAxisBottom = max(ptAxisEnd.y, ptAxisStart.y);

		if (rectThumb.top < dblAxisTop)
		{
			rectThumb.top = dblAxisTop;
		}

		if (rectThumb.bottom > dblAxisBottom)
		{
			rectThumb.bottom = dblAxisBottom;
		}
	}
	else
	{
		double dblAxisLeft = min(ptAxisEnd.x, ptAxisStart.x);
		double dblAxisRight = max(ptAxisEnd.x, ptAxisStart.x);

		if (rectThumb.left < dblAxisLeft)
		{
			rectThumb.left = dblAxisLeft;
		}
		
		if (rectThumb.right > dblAxisRight)
		{
			rectThumb.right = dblAxisRight;
		}
	}

	if (rectThumb.Size() == m_rectThumb.Size())
	{
		return FALSE;
	}

	double dblMinPercent = 0;
	double dblMaxPercent = 0;

	if (IsVertical())
	{
		dblMinPercent = (m_ptAxisStart.y - rectThumb.bottom) / dblAxisSize;
		dblMaxPercent = (rectThumb.top - m_ptAxisEnd.y) / dblAxisSize;
	}
	else
	{
		dblMinPercent = (rectThumb.left - m_ptAxisStart.x) / dblAxisSize;
		dblMaxPercent = (m_ptAxisEnd.x - rectThumb.right) / dblAxisSize;
	}

	double dblFixedMin = dblMinPercent * dblScrollRange + m_dblMinScrollValue;
	double dblFixedMax = m_dblMaxScrollValue - dblMaxPercent * dblScrollRange;

	m_rectThumb = rectThumb;

	SaveRanges();
	SetFixedDisplayRange(dblFixedMin, dblFixedMax);

	if (m_pChart != NULL)
	{
		m_pChart->OnAxisZoomed(this);
	}
	
	return TRUE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnCalcThumbRect()
{
	if (m_rectScrollBar.IsRectEmpty())
	{
		return;
	}

	double dblScrollRange = m_dblMaxScrollValue - m_dblMinScrollValue;

	if (dblScrollRange == 0)
	{
		m_rectThumb.SetRectEmpty();
		return;
	}

	if (m_pChart != NULL && m_pChart->IsThumbSizeMode() && 
		m_pChart->GetThumbSizeAxis() == this)
	{
		if (IsVertical())
		{
			m_rectThumb.left = m_rectScrollBar.left;
			m_rectThumb.right = m_rectScrollBar.right;
		}
		else
		{
			m_rectThumb.top = m_rectScrollBar.top;
			m_rectThumb.bottom = m_rectScrollBar.bottom;
		}
		return;
	}

	double dblMinPercent = fabs((GetMinDisplayedValue() - m_dblMinScrollValue) / dblScrollRange);
	double dblMaxPercent = fabs((m_dblMaxScrollValue - GetMaxDisplayedValue()) / dblScrollRange);

	m_rectThumb = m_rectScrollBar;

	double dblMinThumbSize = GetMinThumbSize(TRUE);
	double dblAxisSize = fabs(GetAxisSize());

	if (IsVertical())
	{
		if (!m_bReverseOrder)
		{
			m_rectThumb.bottom = m_ptAxisStart.y - dblAxisSize * dblMinPercent;
			m_rectThumb.top =  m_ptAxisEnd.y + dblAxisSize * dblMaxPercent;
		}
		else
		{
			m_rectThumb.top = m_ptAxisStart.y + dblAxisSize * dblMinPercent;
			m_rectThumb.bottom =  m_ptAxisEnd.y - dblAxisSize * dblMaxPercent;
		}
	}
	else
	{
		if (!m_bReverseOrder)
		{
			m_rectThumb.left = m_ptAxisStart.x + dblAxisSize * dblMinPercent;
			m_rectThumb.right = m_ptAxisEnd.x - dblAxisSize * dblMaxPercent;
		}
		else
		{
			m_rectThumb.right = m_ptAxisStart.x - dblAxisSize * dblMinPercent;
			m_rectThumb.left = m_ptAxisEnd.x + dblAxisSize * dblMaxPercent;
		}
	}

	CheckThumbMinSize(m_rectThumb, dblMinThumbSize);
	m_rectThumb.Normalize();

	if (m_bIsThumbGripEnabled)
	{
		m_rectLeftGrip = m_rectThumb;
		m_rectRightGrip = m_rectThumb;

		if (IsVertical())
		{
			m_rectLeftGrip.top = m_rectLeftGrip.bottom - 5;
			m_rectRightGrip.bottom = m_rectRightGrip.top + 5;
		}
		else
		{
			m_rectLeftGrip.right = m_rectLeftGrip.left + 5;
			m_rectRightGrip.left = m_rectRightGrip.right - 5;
		}
	}
	else
	{
		m_rectLeftGrip.SetRectEmpty();
		m_rectRightGrip.SetRectEmpty();
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::CheckThumbMinSize(CBCGPRect& rectThumb, double dblMinThumbSize)
{
	if (IsVertical())
	{
		if (rectThumb.Height() < dblMinThumbSize)
		{
			rectThumb.bottom = rectThumb.CenterPoint().y + dblMinThumbSize / 2;
			rectThumb.top = rectThumb.bottom - dblMinThumbSize;
			
			if (rectThumb.bottom > m_rectScrollBar.bottom)
			{
				rectThumb.bottom = m_rectScrollBar.bottom;
				rectThumb.top = rectThumb.bottom - dblMinThumbSize;
			}
			else if (rectThumb.top < m_rectScrollBar.top)
			{
				rectThumb.top = m_rectScrollBar.top;
				rectThumb.bottom = rectThumb.top + dblMinThumbSize;
			}
		}
	}
	else
	{
		if (rectThumb.Width() < dblMinThumbSize)
		{
			rectThumb.left = rectThumb.CenterPoint().x - dblMinThumbSize / 2;
			rectThumb.right = rectThumb.left + dblMinThumbSize;
			
			if (rectThumb.left < m_rectScrollBar.left)
			{
				rectThumb.left = m_rectScrollBar.left;
				rectThumb.right = rectThumb.left + dblMinThumbSize;
			}
			else if (rectThumb.right > m_rectScrollBar.right)
			{
				rectThumb.right = m_rectScrollBar.right;
				rectThumb.left = rectThumb.right - dblMinThumbSize;
			}
		}
	}
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::Zoom(int nMagnifier, const CBCGPPoint& ptZoomCenter)
{
	if (!IsZoomEnabled())
	{
		return FALSE;
	}

 	if (m_bIsFixedIntervalWidth)
 	{
		int nPrevValuesPerInterval = m_nValuesPerInterval;

		m_nValuesPerInterval -= nMagnifier;

		if (m_nValuesPerInterval <= 0)
		{
			m_nValuesPerInterval = 1;
		}

		if (m_nValuesPerInterval > m_nFixedIntervalWidth)
		{
			m_nValuesPerInterval = m_nFixedIntervalWidth;
		}

		if (IsFixedDisplayRange())
		{
			m_bFixedMinimum = FALSE;
			m_bFixedRange = FALSE;
		}

		if (nPrevValuesPerInterval != m_nValuesPerInterval)
		{
			m_pChart->OnAxisZoomed(this);
		}
	}
	else 
	{
		double dblZoomValue = nMagnifier > 0 ? nMagnifier : 1. / nMagnifier;

		if (dblZoomValue == 1)
		{
			dblZoomValue = 2;
		}

		if (dblZoomValue == -1)
		{
			dblZoomValue = 0.5;
		}

		if (dblZoomValue == 0)
		{
			return FALSE;
		}

		double dblRange = GetMaxDisplayedValue() - GetMinDisplayedValue();

		if (!IsComponentXSet() && dblRange <= 1 && nMagnifier > 0)
		{
			return FALSE;
		}

		double dblCenterValue = GetMinDisplayedValue() + dblRange / 2;
		double dblOffset = 0;

		double dblMin = dblCenterValue - dblRange / 2 / fabs(dblZoomValue);

		if (ptZoomCenter.x != 0 && ptZoomCenter.y != 0)
		{
			dblCenterValue = ValueFromPoint(ptZoomCenter);
			dblOffset = fabs(dblCenterValue - GetMinDisplayedValue()) / fabs(dblZoomValue);

			dblMin = dblCenterValue - dblOffset;
		}

		double dblMax = dblMin + dblRange / fabs(dblZoomValue);

		double dblNewRange = dblMax - dblMin;
		double dblScrollRange = m_dblMaxScrollValue - m_dblMinScrollValue;

		if (dblNewRange < dblScrollRange && dblScrollRange / dblNewRange > m_dblMaxZoomInFactor)
		{
			return FALSE;
		}

		if (m_bFixedMajorUnit || !IsComponentXSet())
		{
			double dblMaxAllowedRange = fabs(GetAxisSize()) * m_dblMajorUnit / m_nMinorUnitCount;

			if (dblNewRange > dblMaxAllowedRange)
			{
				dblMin = dblCenterValue - (dblMaxAllowedRange / 2);
				dblMax = dblCenterValue + (dblMaxAllowedRange / 2);
			}
		}

		if (!IsComponentXSet() && dblMin < 0 && m_dblMinScrollValue >= 0)
		{
			dblMax += fabs(dblMin);
			dblMin = 0.;
		}

		if (dblMin < m_dblMinScrollValue)
		{
			dblMin = m_dblMinScrollValue;
		}

		if (dblMax > m_dblMaxScrollValue)
		{
			dblMax = m_dblMaxScrollValue;
		}

		if (dblMax <= dblMin)
		{
			if (dblMin < m_dblMinScrollValue + m_dblMajorUnit)
			{
				dblMax = dblMin + m_dblMajorUnit;
			}
			else
			{
				dblMin = dblMax - m_dblMajorUnit;
			}
		}

		if (IsIndexedSeries())
		{
			dblMin = floor(dblMin);
			if (dblMin < 1)
			{
				dblMin = 1;
			}
		}

		if (dblMin > m_dblMinScrollValue || dblMax < m_dblMaxScrollValue)
		{
			SaveRanges();			
			SetFixedDisplayRange(dblMin, dblMax);
			m_pChart->OnAxisZoomed(this);
		}
		else
		{
			UnZoom();
		}
	}


	return TRUE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::UnZoom()
{
	RestoreRanges();
	m_pChart->OnAxisZoomed(this);
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::CalcMajorUnit(int nMaxLines, double dblRange, double dblOffset)
{
	ASSERT_VALID(this);

	if (dblOffset == 0. || dblRange <= 0)
	{
		return 1.;
	}

	if (GetMajorUnitSpacing() != 0)
	{
		return dblRange / nMaxLines;
	}

	double dblValue = dblRange;
	double dblNormal = log10(dblValue);
	double dblStep = 0.0;

	// calculate standard range for m_nLines
	// calculate values for step 1
	// the upper and bottom values should not exceed 1.05 part of the total range
	double dblTopRange_1 = nMaxLines / dblOffset;
	double dblBottomRange_1 = (nMaxLines / 2.) / dblOffset;

	// calculate bottom value for range with step 0.1
	double dblBottomRange_01 = (nMaxLines / 10.) / dblOffset;

	// calculate bottom value for range with step 0.5
	double dblBottomRange_05 = (nMaxLines / 5.) / dblOffset;

	// calculate upper value for range with step 2
	double dblTopRange_2 = (nMaxLines * 2.)/dblOffset;

	// calculate upper value for range with step 5
	double dblTopRange_5 = (nMaxLines * 5.)/dblOffset;

	// incoming values should be normalized with log10, so the basic ranges should be normalized too
	double dblLogBottom_01 = log10(dblBottomRange_01);
	double dblLogBottom_05 = log10(dblBottomRange_05);
	double dblLogBottom_1 = log10(dblBottomRange_1);
	double dblLogUpper_1 = log10(dblTopRange_1); 
	double dblLogUpper_2 = log10(dblTopRange_2); 
	double dblLogUpper_5 = log10(dblTopRange_5); 

	// find grade
	double dblDecimalPart = 0.;
	double dblGrade = 0.;

	dblDecimalPart = dblNormal - floor(dblNormal);

	double dblIntPart = dblNormal - dblDecimalPart;
	dblGrade = pow (10.0, dblIntPart);

	if (dblDecimalPart <= dblLogBottom_01)
	{
		dblStep = 0.1 * dblGrade;
	}
	else if (dblDecimalPart > dblLogBottom_01 && dblDecimalPart <= dblLogBottom_05)
	{
		dblStep = 0.2 * dblGrade;
	}
	else if (dblDecimalPart > dblLogBottom_05 && dblDecimalPart <= dblLogBottom_1)
	{
		dblStep = 0.5 * dblGrade;
	}
	else if (dblDecimalPart > dblLogBottom_1 && dblDecimalPart <= dblLogUpper_1)
	{
		dblStep = 1. * dblGrade; 
	}
	else if (dblDecimalPart > dblLogUpper_1 && dblDecimalPart <= dblLogUpper_2)
	{
		dblStep = 2. * dblGrade; 
	}
	else if (dblDecimalPart > dblLogUpper_2 && dblDecimalPart <= dblLogUpper_5)
	{
		dblStep = 5. * dblGrade; 
	}
	else
	{
		dblStep = 10. * dblGrade; 
	}

	return dblStep;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::CalcMaxLabelSize(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);
	ASSERT(m_pChart != NULL);

	m_szMaxLabelSize.SetSize(0, 0);

	if (m_pChart == NULL || !IsAxisVisible() || GetAxisLabelType() == CBCGPChartAxis::ALT_NO_LABELS ||
		m_pChart->OnGetMaxLabelSize(this, m_szMaxLabelSize) || m_dblMajorUnit == 0.)
	{
		return;
	}

	if (!m_szMaxFixedLabelSize.IsNull())
	{
		m_szMaxLabelSize = m_szMaxFixedLabelSize;
		return;
	}

	CBCGPSize sizeTextMax(0,0);

	if (!m_strMaxDisplayedLabel.IsEmpty())
	{
		sizeTextMax = m_pChart->OnGetTextSize(pGM, m_strMaxDisplayedLabel, m_axisLabelsFormat.m_textFormat);
	}
	else
	{
		CBCGPSize sizeText(0,0); 
		CString strLabel;

		double dblMinValue = GetMinDisplayedValue();
		double dblMaxValue = GetMaxDisplayedValue();

		if (IsLogScale())
		{
			double dblUnitStep = CalcLog(GetMinDisplayedValue());

			double dblCurrValue = dblMinValue;

			while (dblCurrValue <= dblMaxValue)
			{
				GetDisplayedLabel(dblCurrValue, strLabel);
				sizeText = m_pChart->OnCalcAxisLabelSize(pGM, this, dblCurrValue, strLabel, m_axisLabelsFormat.m_textFormat);

				sizeTextMax.cx = max(sizeText.cx, sizeTextMax.cx);
				sizeTextMax.cy = max(sizeText.cy, sizeTextMax.cy);

				dblUnitStep += 1.;
				dblCurrValue = pow(m_dblMajorUnit, dblUnitStep);
			}
		}
		else
		{
			BOOL bVariableAngle = IsVariableLabelAngle();
			double dblDrawingAngle = m_axisLabelsFormat.m_textFormat.GetDrawingAngle();

			if (m_bUseApproximation)
			{
				dblMinValue = ApproximateValue(dblMinValue, m_dblMajorUnit, 1.) - m_dblMajorUnit;
			}

			for (double dblVal = dblMinValue; dblVal <= dblMaxValue; dblVal += m_dblMajorUnit)
			{
				GetDisplayedLabel(dblVal, strLabel);
				if (!strLabel.IsEmpty())
				{
					if (bVariableAngle)
					{
						m_axisLabelsFormat.m_textFormat.SetDrawingAngle(GetVariableLabelAngle(dblVal));
					}

					sizeText = m_pChart->OnCalcAxisLabelSize(pGM, this, dblVal, strLabel, m_axisLabelsFormat.m_textFormat);
					sizeTextMax.cx = max(sizeText.cx, sizeTextMax.cx);
					sizeTextMax.cy = max(sizeText.cy, sizeTextMax.cy);
					
					if (bVariableAngle)
					{
						m_axisLabelsFormat.m_textFormat.SetDrawingAngle(dblDrawingAngle);
					}
				}
			}

			if (CanUseScaleBreaks())
			{
				CalcMaxLabelSizeForScaleParts(pGM);
			}
		}
	}

	if (!CanUseScaleBreaks())
	{
		CBCGPSize szPadding = m_axisLabelsFormat.GetContentPadding(TRUE);
		
		m_szMaxLabelSize.cx = sizeTextMax.cx + szPadding.cx * 2;
		m_szMaxLabelSize.cy = sizeTextMax.cy + szPadding.cy * 2;
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::GetFormattedValue(double dblValue, CString& strValue)
{
	ASSERT_VALID(this);

	CString strFormat;
	GetFormatString(strFormat, dblValue);
	
	strValue.Format(strFormat, dblValue);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::GetDisplayedLabel(double dblValue, CString& strLabel)
{
	ASSERT_VALID(this);

	if (m_pIndexedSeries != NULL && m_pIndexedSeries->IsIndexMode())
	{
		if (m_pIndexedSeries->IsXComponentSet())
		{
			dblValue = m_pIndexedSeries->GetDataPointValue((int)dblValue, CBCGPChartData::CI_X).GetValue();
			if (dblValue == 0)
			{
				strLabel = _T("");
				return;
			}
		}
	}

	if (m_bFormatAsDate)
	{
		if (dblValue == 0.)
		{
			strLabel.Empty();
			return;
		}

		COleDateTime dt(dblValue);

		if (m_strDataFormat.IsEmpty())
		{
			strLabel = dt.Format(LOCALE_NOUSEROVERRIDE, LANG_USER_DEFAULT);
		}
		else
		{	
			strLabel = dt.Format(m_strDataFormat);
		}
	}
	else
	{
		if (m_dblDisplayUnits != 0.0)
		{
			dblValue /= m_dblDisplayUnits;
		}

		if (fabs(dblValue) < GetDoubleCorrection() && m_dblMajorUnit > 0.)
		{
			if (IsLogScale())
			{
				strLabel.Format(_T("%.2G"), dblValue);
				return;
			}
			else 
			{
				double dblLogMajor = log10(m_dblMajorUnit);
				double dblLogValue = log10(fabs(dblValue));

				if (CanUseScaleBreaks())
				{
					CBCGPChartAxisScalePart part;
					ScalePartFromValue(dblValue, part);

					dblLogMajor = log10(part.m_dblMajorUnit);
					if (part.m_dblMajorUnit != 0 && dblLogMajor > dblLogValue)
					{
						dblValue = 0.;
					}
				}
				else if (dblLogMajor > dblLogValue)
				{
					dblValue = 0.;
				}
			}
		}
		GetFormattedValue(dblValue, strLabel);
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::GetFormatString(CString& strFormat, double dblValue)
{
	if (m_strDataFormat.IsEmpty())
	{
		if (!m_bInitialized || m_dblMajorUnit == 0)
		{
			strFormat = _T("%.f");
			return;
		}

		double dblNormalMajor = fabs(log10(m_dblMajorUnit));
		double dblNormal = dblValue != 0 ? fabs(log10(fabs(dblValue))) : 0;

		if (dblNormal >= 8 && fabs(dblNormal - dblNormalMajor) < 6 && dblNormal != 0)
		{
			strFormat = _T("%.3G");
		}
		else if (dblNormal == 0)
		{
			strFormat = _T("%.f");
		}
		else
		{
			strFormat = _T("%.9G");
		}
	}
	else
	{
		strFormat = m_strDataFormat;
	}
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::ValueFromPointByScrollRange(const CBCGPPoint& pt, CBCGPChartAxis::RoundType roundType)
{
	ASSERT_VALID(this);

	if (GetMinScrollValue() == -DBL_MAX)
	{
		return ValueFromPoint(pt, roundType);
	}

	double dblMinRange = GetMinScrollValue();
	double dblMaxRange = GetMaxScrollValue();

	double dblRange = dblMaxRange - dblMinRange;

	if (dblRange == 0)
	{
		return 0;
	}

	if (IsIndexedSeries())
	{
		roundType = CBCGPChartAxis::RT_FLOOR;
	}

	double dblAxisSize = fabs(GetAxisSize());
	double dblPointOffset = IsVertical()
								? (m_bReverseOrder ? pt.y - m_ptAxisStart.y : m_ptAxisStart.y - pt.y)
								: (m_bReverseOrder ? m_ptAxisStart.x - pt.x : pt.x - m_ptAxisStart.x);

	double dblVal = (dblRange * dblPointOffset) / dblAxisSize;

	if (IsDisplayDataBetweenTickMarks())
	{
		dblVal -= GetBaseUnitSize() / 2;
	}

	if (roundType == CBCGPChartAxis::RT_FLOOR)
	{
		dblVal = ((int)(dblVal / m_dblMajorUnit)) * m_dblMajorUnit;
	}
	else if (roundType != CBCGPChartAxis::RT_EXACT)
	{
		dblVal = ((int)(dblVal / m_dblMajorUnit) + 1) * m_dblMajorUnit;
	}

	return dblVal + dblMinRange;
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::ValueFromPointInternal(const CBCGPPoint& pt, CBCGPChartAxis::RoundType roundType) const
{
	if (CanUseScaleBreaks())
	{
		return ValueFromPointOnAxisWithBreak(pt, roundType);
	}

	double dblMinDisplayedValue = GetMinDisplayedValue();
	double dblMaxDisplayedValue = GetMaxDisplayedValue();
	double dblMajorUnit = m_dblMajorUnit;
	double dblMinAllowedMajorUnit = m_dblMinAllowedMajorUnit == 0.0 ? m_dblMajorUnit : m_dblMinAllowedMajorUnit;

	if (IsLogScale())
	{
		dblMinDisplayedValue = CalcLog(dblMinDisplayedValue);
		dblMaxDisplayedValue = CalcLog(dblMaxDisplayedValue);
		dblMajorUnit = CalcLog(dblMajorUnit);
		dblMinAllowedMajorUnit = CalcLog(dblMinAllowedMajorUnit);
	}

	double dblRange = dblMaxDisplayedValue - dblMinDisplayedValue;
	
	if (dblRange == 0)
	{
		return 0;
	}
	
	double dblAxisSize = fabs(GetAxisSize());
	double dblPointOffset = IsVertical()
								? (m_bReverseOrder ? pt.y - m_ptAxisStart.y : m_ptAxisStart.y - pt.y)
								: (m_bReverseOrder ? m_ptAxisStart.x - pt.x : pt.x - m_ptAxisStart.x);
	
	double dblVal = 0.0;

	if (IsFixedIntervalWidth() && !m_bStretchFixedIntervals)
	{
		double dblValueStep = GetFixedIntervalWidthScaled() / m_nValuesPerInterval;
		dblVal = dblPointOffset / dblValueStep;
	}
	else
	{
		dblVal = (dblRange * dblPointOffset) / dblAxisSize;
		
		if (IsDisplayDataBetweenTickMarks())
		{
			dblVal -= GetBaseUnitSize() / 2;
		}
		
		if (roundType == CBCGPChartAxis::RT_FLOOR)
		{
			dblVal = ((int)(dblVal / dblMajorUnit)) * dblMajorUnit;
		}
		else if (roundType == CBCGPChartAxis::RT_ROUND)
		{
			dblVal = ((int)((dblVal / dblMinAllowedMajorUnit) + 0.5)) * dblMinAllowedMajorUnit;
		}
		else if (roundType == CBCGPChartAxis::RT_CEIL)
		{
			dblVal = ((int)(dblVal / dblMajorUnit) + 1) * dblMajorUnit;
		}
	}

	dblVal += dblMinDisplayedValue;

	if (IsLogScale())
	{
		dblVal = CalcLog(dblVal, FALSE);
	}

	return dblVal;
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::ValueFromPoint(const CBCGPPoint& pt, CBCGPChartAxis::RoundType roundType)
{
	ASSERT_VALID(this);

	return ValueFromPointInternal(pt, roundType);
}
//----------------------------------------------------------------------------//
CBCGPPoint CBCGPChartAxis::PointFromValue3D(double dblValue, BOOL bForceValueOnThickMark, 
											BOOL bForceValueOnWall, BOOL bLogValue)
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return CBCGPPoint();
	}

	double dblVal = CBCGPChartAxis::PointFromValue(dblValue, bForceValueOnThickMark, bLogValue);

	CBCGPChartDiagram3D* pDiagram3D = m_pChart->GetDiagram3D();

	if (!IsDiagram3D() || pDiagram3D == NULL)
	{
		CBCGPPoint ptRet;
		
		if (IsVertical())
		{
			ptRet.x = m_ptAxisStart.x;
			ptRet.y = dblVal;
		}
		else
		{
			ptRet.y = m_ptAxisStart.y;
			ptRet.x = dblVal;
		}

		return ptRet;
	}

	CBCGPPoint pt = GetPointOnAxisPlane3D(dblVal, bForceValueOnWall);
	return pDiagram3D->TransformPoint(pt, pDiagram3D->GetDiagramRect().CenterPoint());
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::CalcNormalValue(double dblValue) const
{
	ASSERT_VALID(this);

	double dblMinValue = GetMinDisplayedValue();
	double dblMaxValue = GetMaxDisplayedValue();

	double dblRange = dblMaxValue - dblMinValue;

	if (dblMinValue == 0 && IsLogScale())
	{
		dblMinValue = 1.;
	}

	if (dblRange == 0)
	{
		return 0;
	}

	if (IsLogScale() && dblValue > 0)
	{
		dblValue = CalcLog(dblValue);
		dblRange = CalcLog(dblMaxValue) - CalcLog(dblMinValue);
		dblMinValue = CalcLog(dblMinValue);
	}

	double dblAxisSize = fabs(GetAxisSize(TRUE));

	return ((dblValue - dblMinValue) * dblAxisSize / dblRange);
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::PointFromValueOptimized3D(double dblValue, const CBCGPPoint& ptAxisStart,
											const CBCGPPoint& /*ptAxisEnd*/, double dblAxisSize, 
											BOOL bIsDisplayDataBetweenTickMarks, CBCGPChartData::ComponentIndex ci, 
											BOOL bIsLogScale)
{
	ASSERT_VALID(this);

	BOOL bLogValue = TRUE;

	double dblMinValue = GetMinDisplayedValue();
	double dblMaxValue = GetMaxDisplayedValue();

	if (dblMinValue == 0 && IsLogScale())
	{
		dblMinValue = 1.;
	}

	double dblRange = dblMaxValue - dblMinValue;

	if (dblRange == 0)
	{
		return IsVertical() ? ptAxisStart.y : ptAxisStart.x;
	}

	double dblPoint = 0.;

	if (bIsDisplayDataBetweenTickMarks)
	{
		dblValue += GetBaseUnitSize() / 2;
	}

	if (bIsLogScale && (dblValue > 0 || !bLogValue))
	{
		if (bLogValue)
		{
			dblValue = CalcLog(dblValue);
		}

		dblRange = CalcLog(dblMaxValue) - CalcLog(dblMinValue);
		dblMinValue = CalcLog(dblMinValue);
	}

	double dblStart = 0;

	switch (ci)
	{
	case CBCGPChartData::CI_X:
	case CBCGPChartData::CI_Y:
		dblStart = IsVertical() ? m_ptAxisStart3D.y : m_ptAxisStart3D.x;
		break;

	case CBCGPChartData::CI_Z:
		dblStart = m_ptAxisStart3D.z;
		break;
	}

	dblPoint = dblStart + ((dblValue - dblMinValue) * dblAxisSize / dblRange);
	
	return dblPoint;
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::PointFromValue(double dblValue, BOOL bForceValueOnThickMark, BOOL bLogValue)
{
	ASSERT_VALID(this);

	if (CanUseScaleBreaks())
	{
		return PointFromValueOnAxisWithBreak(dblValue, bForceValueOnThickMark);
	}

	double dblMinValue = GetMinDisplayedValue();
	double dblMaxValue = GetMaxDisplayedValue();

	if (dblMinValue == 0 && IsLogScale())
	{
		dblMinValue = 1.;
	}

	CBCGPPoint ptAxisStart;
	CBCGPPoint ptAxisEnd;

	GetAxisPos(ptAxisStart, ptAxisEnd);

	if (IsFixedIntervalWidth() && !m_bStretchFixedIntervals)
	{
		double dblValueStep = GetFixedIntervalWidthScaled() / m_nValuesPerInterval;
	
		if (IsVertical())		
		{
			return m_ptAxisStart.y  - (dblValue - dblMinValue) * dblValueStep;	
		}
		else
		{
			return m_ptAxisStart.x  + (dblValue - dblMinValue) * dblValueStep;
		}
	}

	double dblRange = dblMaxValue - dblMinValue;

	if (dblRange == 0)
	{
		return IsVertical() ? ptAxisStart.y : ptAxisStart.x;
	}

	double dblPoint = 0.;

 	if (IsDisplayDataBetweenTickMarks() && !bForceValueOnThickMark)
 	{
 		dblValue += GetBaseUnitSize() / 2;
 	}

	if (IsLogScale() && (dblValue > 0 || !bLogValue))
	{
		if (bLogValue)
		{
			dblValue = CalcLog(dblValue);
		}
		
		dblRange = CalcLog(dblMaxValue) - CalcLog(dblMinValue);
		dblMinValue = CalcLog(dblMinValue);
	}

	double dblAxisSize = GetAxisSize(TRUE);

	if (IsDiagram3D())
	{
		CBCGPChartData::ComponentIndex ci = GetComponentIndex();
		double dblStart = 0;
		
		switch (ci)
		{
		case CBCGPChartData::CI_X:
		case CBCGPChartData::CI_Y:
			if (m_bReverseOrder)
			{
				dblStart = IsVertical() ? m_ptAxisEnd3D.y : m_ptAxisEnd3D.x;
			}
			else
			{
				dblStart = IsVertical() ? m_ptAxisStart3D.y : m_ptAxisStart3D.x;
			}
			break;
		case CBCGPChartData::CI_Z:
			dblStart = m_bReverseOrder ? m_ptAxisEnd3D.z : m_ptAxisStart3D.z;
			break;
		}

		if (m_bReverseOrder)
			dblPoint = dblStart - ((dblValue - dblMinValue) * dblAxisSize / dblRange);
		else
			dblPoint = dblStart + ((dblValue - dblMinValue) * dblAxisSize / dblRange);
	}
	else
	{
		if (IsVertical())
		{
			dblPoint = m_ptAxisStart.y - ((dblValue - dblMinValue) * dblAxisSize / dblRange);
		}
		else
		{
			dblPoint = m_ptAxisStart.x  + ((dblValue - dblMinValue) * dblAxisSize / dblRange);
		}
	}

	return dblPoint;
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::ScaleValue(double dblValue)
{
	ASSERT_VALID(this);

	double dblMinValue = GetMinDisplayedValue();
	double dblMaxValue = GetMaxDisplayedValue();

	if (dblMinValue == 0 && IsLogScale())
	{
		dblMinValue = 1.;
	}

	double dblRange = dblMaxValue - dblMinValue;

	if (dblRange <= 0)
	{
		return 0;
	}

	if (IsLogScale() && dblValue > 0)
	{
		dblRange = CalcLog(dblMaxValue) - CalcLog(dblMinValue);
		dblMinValue = CalcLog(dblMinValue);
	}

	CBCGPPoint ptAxisStart;
	CBCGPPoint ptAxisEnd;

	GetAxisPos(ptAxisStart, ptAxisEnd);
	
	return IsVertical() ? ptAxisStart.y - PointFromValue3D(fabs(dblValue) + dblMinValue, TRUE, FALSE).y :
		PointFromValue3D(fabs(dblValue) + dblMinValue, TRUE, FALSE).x - ptAxisStart.x;
}
//----------------------------------------------------------------------------//
// Scale breaks support
//----------------------------------------------------------------------------//
void CBCGPChartAxis::EnableScaleBreaks(BOOL bEnable, CBCGPChartScaleBreakOptions* pOptions, BOOL bGenerateBreaks)
{
	m_bScaleBreakEnabled = bEnable;

	if (pOptions != NULL)
	{
		m_scaleBreakOptions = *pOptions;
		m_scaleBreakOptions.Verify();
	}

	if (bEnable)
	{
		m_bSavedScrollOption = m_bEnableScroll;
		m_bSavedZoomOption = m_bEnableZoom;

		m_bEnableScroll = FALSE;
		m_bEnableZoom = FALSE;

		if (bGenerateBreaks && m_scaleBreakOptions.m_bAutomatic)
		{
			GenerateScaleBreaks();
		}
	}
	else
	{
		m_bEnableScroll = m_bSavedScrollOption;
		m_bEnableZoom = m_bSavedZoomOption;
	}
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::IsScaleBreakEnabled() const
{
	if (!m_bScaleBreakEnabled || IsLogScale() || IsScrollEnabled() || IsDiagram3D())
	{
		return FALSE;
	}

	return TRUE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::RemoveAllBreaks()
{
	m_scaleBreaks.RemoveAll();
	m_scaleParts.RemoveAll();
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::AddScaleBreak(const CBCGPChartAxisScaleBreak& scaleBreak, BOOL bCreateScaleParts)
{
	if (m_scaleBreakOptions.m_bAutomatic)
	{
		return FALSE;
	}

	CBCGPChartAxisScaleBreak& sbCorrect = (CBCGPChartAxisScaleBreak&) scaleBreak;

	if (scaleBreak.m_dblStart > scaleBreak.m_dblEnd)
	{
		double dblTmp = sbCorrect.m_dblStart;
		sbCorrect.m_dblStart = sbCorrect.m_dblEnd;
		sbCorrect.m_dblEnd = dblTmp;
	}

	if (IsValueWithinScaleBreak(scaleBreak.m_dblStart) || IsValueWithinScaleBreak(scaleBreak.m_dblEnd))
	{
		return FALSE;
	}

	sbCorrect.m_dblOffsetPercent = bcg_clamp(sbCorrect.m_dblOffsetPercent, 10.0, 90.0);
	
	for (POSITION pos = m_scaleBreaks.GetHeadPosition(); pos != NULL;)
	{
		const CBCGPChartAxisScaleBreak& sb = m_scaleBreaks.GetAt(pos);

		if (scaleBreak.m_dblStart > sb.m_dblEnd)
		{
			POSITION posNext = pos;
			m_scaleBreaks.GetNext(posNext);

			if (posNext == NULL)
			{
				break;
			}

			const CBCGPChartAxisScaleBreak& sbNext = m_scaleBreaks.GetAt(posNext);

			if (scaleBreak.m_dblEnd < sbNext.m_dblStart)
			{
				m_scaleBreaks.InsertAfter(pos, scaleBreak);
				if (bCreateScaleParts)
				{
					CreateScaleParts();
				}
				return TRUE;
			}
		}

		m_scaleBreaks.GetNext(pos);
	}

	m_scaleBreaks.AddTail(scaleBreak);

	if (bCreateScaleParts)
	{
		CreateScaleParts();
	}

	if (m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, FALSE);
	}

	return TRUE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::GenerateScaleBreaks() 
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return;
	}

	m_scaleParts.RemoveAll();

	if (!m_scaleBreakOptions.m_bAutomatic)
	{
		CreateScaleParts();
		return;
	}

	double dblRange = m_dblMaximum - m_dblMinimum;

	if (dblRange == 0)
	{
		if (!CalcMinMaxValues())
		{
			return;
		}

		dblRange = m_dblMaximum - m_dblMinimum;

		if (dblRange == 0)
		{
			return;
		}
	}

	double dblThreshold = dblRange * m_scaleBreakOptions.m_dblValueThresholdPercent / 100.;

	for (int i = 0; i < m_pChart->GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = m_pChart->GetSeries(i);

		if (pSeries == NULL)
		{
			continue;
		}

		ASSERT_VALID(pSeries);

		if (!pSeries->IsShownOnAxis(this))
		{
			continue;
		}
		
		for (int j = 0; j < pSeries->GetDataPointCount(); j++)
		{
			CBCGPChartValue val = pSeries->GetDataPointValue(j);
			UpdateScaleBreaksInternal(val, dblThreshold, pSeries);
		}
	}

	MergeScaleBreaks(dblThreshold);

	m_pChart->SetDirty(TRUE, TRUE);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::UpdateScaleBreaks(const CBCGPChartValue& val, CBCGPChartSeries* pSeries)
{
	double dblRange = m_dblMaximum - m_dblMinimum;
	
	if (dblRange == 0)
	{
		if (!CalcMinMaxValues())
		{
			return;
		}
		
		dblRange = m_dblMaximum - m_dblMinimum;
		
		if (dblRange == 0)
		{
			return;
		}
	}
	
	double dblThreshold = dblRange * m_scaleBreakOptions.m_dblValueThresholdPercent / 100.0;

	BOOL bScalePartsChanged = UpdateScaleBreaksInternal(val, dblThreshold, pSeries);

	if ((bScalePartsChanged || MergeScaleBreaks(dblThreshold)) && m_pChart != NULL)
	{
		m_pChart->SetDirty(TRUE, TRUE);
	}
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::MergeScaleBreaks(double dblThreshold)
{
	int nPartCount = (int)m_scaleParts.GetCount();

	MergeScaleBreaksInternal(dblThreshold, FALSE);
	SetScaleBreakRanges();
	MergeScaleBreaksInternal(dblThreshold, TRUE);
	UpdateScaleBreakPercents();

	return nPartCount != (int)m_scaleParts.GetCount();
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetScaleBreakRanges()
{
	for (POSITION pos = m_scaleParts.GetHeadPosition(); pos != NULL; m_scaleParts.GetNext(pos))
	{
		CBCGPChartAxisScalePart& part = (CBCGPChartAxisScalePart&) m_scaleParts.GetAt(pos);
		
		double dblRange = fabs(part.m_dblMaximum - part.m_dblMinimum);
		double dblAdditionalValue = 0;

		BOOL bRoundToZero = part.m_dblMaximum >= 0 && part.m_dblMinimum >= 0 ||
							part.m_dblMaximum <= 0 && part.m_dblMinimum <= 0;
		
		dblAdditionalValue = dblRange == 0 ? CalcMajorUnit(10, fabs(part.m_dblMaximum) / 10) : CalcMajorUnit(10, dblRange);
		double dblNewMinimum = ApproximateValue(part.m_dblMinimum, dblAdditionalValue, 1.04);

		int i = 1;
		// approximate may return a value noticeable larger than dblAdditionalValue, so we need to loop to find the bottom
		while (dblNewMinimum >= part.m_dblMinimum)
		{
			dblNewMinimum -= i * dblAdditionalValue;
			i++;
		}

		if (bRoundToZero && part.m_dblMinimum > 0 && dblNewMinimum < 0)
		{
			part.m_dblMinimum = 0;
		}
		else
		{
			part.m_dblMinimum = dblNewMinimum;
		}

		double dblNewMaximum = ApproximateValue(part.m_dblMaximum, dblAdditionalValue, 1.04) + dblAdditionalValue;

		if (dblNewMaximum == part.m_dblMaximum)
		{
			dblNewMaximum += dblAdditionalValue;
		}
		
		if (bRoundToZero && part.m_dblMaximum < 0 && dblNewMaximum > 0)
		{
			part.m_dblMaximum = 0;
		}
		else
		{
			part.m_dblMaximum = dblNewMaximum;
		}

		if (pos == m_scaleParts.GetHeadPosition())
		{
			if (IsFixedMinimumDisplayValue())
			{
				part.m_dblMinimum = m_dblMinimum;
			}
			else
			{
				m_dblMinimum = part.m_dblMinimum;
			}
		}

		if (pos == m_scaleParts.GetTailPosition())
		{
			if (IsFixedMaximumDisplayValue())
			{
				part.m_dblMaximum = m_dblMaximum;
			}
			else
			{
				m_dblMaximum = part.m_dblMaximum;
			}
		}
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::UpdateScaleBreakPercents()
{
	int nPartCount = (int)m_scaleParts.GetCount();
	double dblPercentOffset = nPartCount > 1 ? 
		(100 - m_scaleBreakOptions.m_dblFirstBreakOffsetPercent) / (nPartCount - 1) : 
	(100 - m_scaleBreakOptions.m_dblFirstBreakOffsetPercent);
	
	double dblPercentStart = m_scaleBreakOptions.m_dblFirstBreakOffsetPercent;
	
	for (POSITION pos = m_scaleParts.GetHeadPosition(); pos != NULL; m_scaleParts.GetNext(pos))
	{
		CBCGPChartAxisScalePart& part = (CBCGPChartAxisScalePart&) m_scaleParts.GetAt(pos);
		
		part.m_nGap = m_scaleBreakOptions.m_nGap;
		part.m_scaleBreakStyle = m_scaleBreakOptions.m_style;
		
		if (pos == m_scaleParts.GetHeadPosition())
		{
			part.m_dblScreenStartPercent = 0;
			part.m_dblScreenEndPercent = dblPercentStart;
		}
		else
		{
			part.m_dblScreenStartPercent = dblPercentStart;
			dblPercentStart += dblPercentOffset;
			part.m_dblScreenEndPercent = dblPercentStart;
		}
		
		if (pos == m_scaleParts.GetTailPosition())
		{
			part.m_dblScreenEndPercent = 100;
		}
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::MergeScaleBreaksInternal(double dblThreshold, BOOL bSecondTime)
{
	if (m_scaleParts.GetCount() > 0)
	{
		CBCGPChartAxisScalePart& partFirst = (CBCGPChartAxisScalePart&) m_scaleParts.GetHead();
		
		if (partFirst.m_dblMinimum > 0 && m_scaleBreakOptions.m_bStartFromZero)
		{
			partFirst.m_dblMinimum = 0;
		}
		
		CBCGPChartAxisScalePart& partLast = (CBCGPChartAxisScalePart&) m_scaleParts.GetTail();
		
		if (partLast.m_dblMaximum < 0 && m_scaleBreakOptions.m_bStartFromZero)
		{
			partLast.m_dblMaximum = 0;
		}
		
		double dblNewThreshold = fabs(partLast.m_dblMaximum - partFirst.m_dblMinimum) * m_scaleBreakOptions.m_dblValueThresholdPercent / 100.0;
		dblThreshold = max(dblNewThreshold, dblThreshold);
	}

	if (m_scaleParts.GetCount() > 1)
	{
		for (POSITION pos = m_scaleParts.GetHeadPosition(); pos != NULL;)
		{
			CBCGPChartAxisScalePart& part = (CBCGPChartAxisScalePart&) m_scaleParts.GetNext(pos);
			
			if (pos != NULL)
			{
				CBCGPChartAxisScalePart& partNext = m_scaleParts.GetAt(pos);
				
				if (fabs(partNext.m_dblMinimum - part.m_dblMaximum) < dblThreshold || partNext.m_dblMinimum <= part.m_dblMaximum)
				{
					double dblSaveMax = partNext.m_dblMaximum;
					int nValuesCount = partNext.m_nValuesCount;
					POSITION posSave = pos;
					
					m_scaleParts.GetNext(pos);
					m_scaleParts.RemoveAt(posSave);
					
					if (pos != NULL)
					{
						m_scaleParts.GetPrev(pos);
						CBCGPChartAxisScalePart& partPrev = m_scaleParts.GetAt(pos);
						partPrev.m_dblMaximum = max (partPrev.m_dblMaximum, dblSaveMax);
						partPrev.m_nValuesCount += nValuesCount;
					}
					else
					{
						CBCGPChartAxisScalePart& partPrev = m_scaleParts.GetTail();
						partPrev.m_dblMaximum = max (partPrev.m_dblMaximum, dblSaveMax);
						partPrev.m_nValuesCount += nValuesCount;
					}
				}
			}
		}
	}

	if (!bSecondTime && m_scaleParts.GetCount() - 1 > m_scaleBreakOptions.m_nMaxCount)
	{
		if (m_scaleBreakOptions.m_bMergeBreaksFromTail)
		{
			for (POSITION pos = m_scaleParts.GetTailPosition(); m_scaleParts.GetCount() - 1 > m_scaleBreakOptions.m_nMaxCount;)
			{
				CBCGPChartAxisScalePart& part = (CBCGPChartAxisScalePart&) m_scaleParts.GetPrev(pos);
				
				if (pos != NULL)
				{
					CBCGPChartAxisScalePart& partPrev = m_scaleParts.GetAt(pos);
					part.m_dblMinimumDisplayed = part.m_dblMinimum = partPrev.GetMinimumDisplayed();
					part.m_nValuesCount += partPrev.m_nValuesCount;

					POSITION posRemove = pos;
					m_scaleParts.GetNext(pos);
					m_scaleParts.RemoveAt(posRemove);
				}
			}
		}
		else
		{
			for (POSITION pos = m_scaleParts.GetHeadPosition(); m_scaleParts.GetCount() - 1 > m_scaleBreakOptions.m_nMaxCount;)
			{
				CBCGPChartAxisScalePart& part = (CBCGPChartAxisScalePart&) m_scaleParts.GetNext(pos);

				if (pos != NULL)
				{
					CBCGPChartAxisScalePart& partNext = m_scaleParts.GetAt(pos);
					part.m_dblMaximumDisplayed = part.m_dblMaximum = partNext.GetMaximumDisplayed();
					part.m_nValuesCount += partNext.m_nValuesCount;
					
					POSITION posRemove = pos;
					m_scaleParts.GetPrev(pos);
					m_scaleParts.RemoveAt(posRemove);
				}
			}
		}
	}
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::UpdateScaleBreaksInternal(const CBCGPChartValue& val, double dblThreshold, CBCGPChartSeries* pSeries)
{
	if (val.IsEmpty() && (pSeries == NULL || pSeries->GetTreatNulls() != CBCGPChartSeries::TN_VALUE))
	{
		return FALSE;
	}
	
	double dblVal = val.GetValue();

	if (m_scaleParts.GetCount() == 0)
	{
		CBCGPChartAxisScalePart part;

		part.m_dblMinimum = dblVal;
		part.m_dblMaximum = dblVal;
		
		m_scaleParts.AddTail(part);
		return FALSE;
	}

	int nCount = (int)m_scaleParts.GetCount();
	
	for (POSITION pos = m_scaleParts.GetHeadPosition(); pos != NULL; m_scaleParts.GetNext(pos))
	{
		CBCGPChartAxisScalePart& part = (CBCGPChartAxisScalePart&) m_scaleParts.GetAt(pos);
		
		if (dblVal >= part.m_dblMinimum && dblVal <= part.m_dblMaximum)
		{
			part.m_nValuesCount++;
			break;
		}
		
		if (dblVal < part.m_dblMinimum)
		{
			if (fabs(part.m_dblMinimum - dblVal) < dblThreshold)
			{
				part.m_dblMinimum = val;
				part.m_nValuesCount++;
			}
			else
			{
				CBCGPChartAxisScalePart part;
				part.m_dblMinimum = part.m_dblMaximum = dblVal;
				m_scaleParts.InsertBefore(pos, part);
			}
			break;
		}
		
		if (dblVal > part.m_dblMaximum)
		{
			if (fabs(dblVal - part.m_dblMaximum) < dblThreshold)
			{
				part.m_dblMaximum = dblVal;
				part.m_nValuesCount++;
				break;
			}
			else if (pos == m_scaleParts.GetTailPosition())
			{
				CBCGPChartAxisScalePart part;
				part.m_dblMinimum = part.m_dblMaximum = dblVal;
				m_scaleParts.AddTail(part);
				break;
			}
		}
	}

	return nCount != m_scaleParts.GetCount();
}
//----------------------------------------------------------------------------//
int CBCGPChartAxis::GetScaleBreakCount() const
{
	if (!IsScaleBreakEnabled())
	{
		return 0;
	}

	return (int) m_scaleBreaks.GetCount();
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::IsValueWithinScaleBreak(double dblValue) const
{
	for (POSITION pos = m_scaleBreaks.GetHeadPosition(); pos != NULL;)
	{
		const CBCGPChartAxisScaleBreak& sb = m_scaleBreaks.GetNext(pos);

		if (dblValue >= sb.m_dblStart && dblValue <= sb.m_dblEnd)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::IsScaleBreakValid(const CBCGPChartAxisScaleBreak& sb) const
{
	if (sb.m_dblStart >= sb.m_dblEnd)
	{
		return FALSE;
	}

	double dblMinValue = GetMinDisplayedValue();
	double dblMaxValue = GetMaxDisplayedValue();

	return sb.m_dblStart > dblMinValue && sb.m_dblEnd < dblMaxValue;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::CreateScaleParts()
{
	m_scaleParts.RemoveAll();

	int nBreakCount = (int) m_scaleBreaks.GetCount();
	if (nBreakCount == 0)
	{
		return;
	}

	if (m_dblMinimum == 0 && m_dblMaximum == 0)
	{
		CalcMinMaxValues();
	}

	double dblMinValue = m_dblMinimum;
	double dblMaxValue = m_dblMaximum;
	double dblStartPercent = 0;


	POSITION pos = NULL;

	int nCount = 0;
	for (pos = m_scaleBreaks.GetHeadPosition(); pos != NULL; nCount++)
	{
		const CBCGPChartAxisScaleBreak& sb = (CBCGPChartAxisScaleBreak&) m_scaleBreaks.GetNext(pos);

		if (pos != NULL)
		{
			CBCGPChartAxisScaleBreak& sbNext = (CBCGPChartAxisScaleBreak&) m_scaleBreaks.GetAt(pos);

			if (sbNext.m_dblOffsetPercent <= sb.m_dblOffsetPercent)
			{
				sbNext.m_dblOffsetPercent = sb.m_dblOffsetPercent + 
					(100 - sb.m_dblOffsetPercent) / (nBreakCount - nCount);
			}
		}
	}

	for (pos = m_scaleBreaks.GetHeadPosition(); pos != NULL;)
	{
		const CBCGPChartAxisScaleBreak& sb = m_scaleBreaks.GetNext(pos);

		CBCGPChartAxisScalePart part;

		part.m_dblMinimum = dblMinValue;
		part.m_dblMaximum = sb.m_dblStart;
		part.m_dblScreenStartPercent = dblStartPercent;
		part.m_dblScreenEndPercent = sb.m_dblOffsetPercent;
		part.m_nGap = sb.m_nGap;
		part.m_scaleBreakStyle = sb.m_scaleBreakStyle;
		part.m_scaleBreakFormat = sb.m_scaleBreakFormat;

		m_scaleParts.AddTail(part);

		dblStartPercent = part.m_dblScreenEndPercent;
		dblMinValue = sb.m_dblEnd;

		if (pos == NULL)
		{
			CBCGPChartAxisScalePart partLast;
			partLast.m_dblMinimum = dblMinValue;
			partLast.m_dblMaximum = dblMaxValue;
			partLast.m_dblScreenStartPercent = dblStartPercent;
			partLast.m_dblScreenEndPercent = 100;
			partLast.m_nGap = 0;

			m_scaleParts.AddTail(partLast);
		}
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::UpdateScaleParts()
{
	if (m_pChart == NULL)
	{
		return;
	}

	CBCGPSize szScaleRatio = m_pChart->GetScaleRatio();

	CBCGPPoint ptAxisStart;
	CBCGPPoint ptAxisEnd;
	
	GetAxisPos(ptAxisStart, ptAxisEnd);
	
	double dblAxisSize = GetAxisSize(FALSE);
	double dblAxisStart = IsVertical() ? ptAxisStart.y : ptAxisStart.x;

	int nGap = 0;
	for (POSITION pos = m_scaleParts.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartAxisScalePart& part = (CBCGPChartAxisScalePart&) m_scaleParts.GetNext(pos);

		if (IsVertical())
		{
			part.m_dblScreenStart = dblAxisStart - dblAxisSize * part.m_dblScreenStartPercent / 100.0;
			part.m_dblScreenStart += m_bReverseOrder ? nGap : -nGap;
			part.m_dblScreenEnd = dblAxisStart - dblAxisSize * part.m_dblScreenEndPercent / 100.0;
			nGap = (int)(part.m_nGap * szScaleRatio.cy);
		}
		else
		{
			part.m_dblScreenStart = dblAxisStart + dblAxisSize * part.m_dblScreenStartPercent / 100.0;
			part.m_dblScreenStart += m_bReverseOrder ? -nGap : nGap;
			part.m_dblScreenEnd = dblAxisStart + dblAxisSize * part.m_dblScreenEndPercent / 100.0;
			nGap = (int)(part.m_nGap * szScaleRatio.cx);
		}
	}
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::ScaleBreakFromValue(double dblValue, CBCGPChartAxisScaleBreak& sb, int& nIndex) const
{
	nIndex = 0;
	for (POSITION pos = m_scaleBreaks.GetHeadPosition(); pos != NULL; nIndex++)
	{
		sb = m_scaleBreaks.GetNext(pos);
		if (sb.m_dblStart <= dblValue && dblValue <= sb.m_dblEnd)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::ScaleBreakFromPoint(const CBCGPPoint& pt, CBCGPChartAxisScaleBreak& sb, int& nIndex) const
{
	double dblPoint = IsVertical() ? pt.y : pt.x;

	nIndex = 0;

	for (POSITION pos = m_scaleParts.GetHeadPosition(); pos != NULL; nIndex++)
	{
		const CBCGPChartAxisScalePart& part = m_scaleParts.GetNext(pos);

		if (pos != NULL)
		{
			const CBCGPChartAxisScalePart& partNext = m_scaleParts.GetAt(pos);
			if (dblPoint < part.m_dblScreenEnd && dblPoint > partNext.m_dblScreenStart ||
				dblPoint > part.m_dblScreenEnd && dblPoint < partNext.m_dblScreenStart)
			{
				if (m_scaleBreakOptions.m_bAutomatic)
				{
					sb.m_dblStart = part.GetMaximumDisplayed();
					sb.m_dblEnd = partNext.GetMinimumDisplayed();
					sb.m_nGap = part.m_nGap;
					sb.m_dblOffsetPercent = partNext.m_dblScreenStartPercent;
					sb.m_scaleBreakStyle = partNext.m_scaleBreakStyle;
					sb.m_scaleBreakFormat = m_scaleBreakOptions.m_format;
				}
				else
				{
					POSITION posBreak = m_scaleBreaks.FindIndex(nIndex);

					if (posBreak == NULL)
					{
						return FALSE;
					}

					sb = m_scaleBreaks.GetAt(posBreak);
				}
				
				return TRUE;
			}
		}
	}

	return FALSE;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::ScalePartFromValue(double dblValue, CBCGPChartAxisScalePart& part) const
{
	for (POSITION pos = m_scaleParts.GetHeadPosition(); pos != NULL;)
	{
		part = m_scaleParts.GetNext(pos);
		if (dblValue >= part.GetMinimumDisplayed() && dblValue <= part.GetMaximumDisplayed())
		{
			return TRUE;
		}
	}

	return FALSE;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::ScalePartFromPoint(const CBCGPPoint& pt, CBCGPChartAxisScalePart& part, int& nIndex) const
{
	nIndex = 0;
	for (POSITION pos = m_scaleParts.GetHeadPosition(); pos != NULL;)
	{
		part = m_scaleParts.GetNext(pos);

		double dblStart = part.m_dblScreenStart;
		double dblEnd = part.m_dblScreenEnd;

		if (dblStart > dblEnd)
		{
			dblStart = part.m_dblScreenEnd;
			dblEnd = part.m_dblScreenStart;
		}

		if (IsVertical() && pt.y >= dblStart && pt.y <= dblEnd || 
			!IsVertical()  && pt.x >= dblStart && pt.x <= dblEnd)
		{
			return TRUE;
		}

		nIndex++;
	}
	
	return FALSE;
}
//----------------------------------------------------------------------------//
int CBCGPChartAxis::GetValidPartCount() const
{
	int nCount = 0;

	for (POSITION pos = m_scaleParts.GetHeadPosition(); pos != NULL; m_scaleParts.GetNext(pos))
	{
		const CBCGPChartAxisScalePart& part = (const CBCGPChartAxisScalePart&) m_scaleParts.GetAt(pos);

		if (part.m_bValid)
		{
			nCount++;
		}
	}

	return nCount;
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::PointFromValueOnAxisWithBreak(double dblValue, BOOL bForceValueOnThickMark)
{
	ASSERT_VALID(this);
	ASSERT(IsScaleBreakEnabled());
	ASSERT(m_scaleParts.GetCount() > 1);

	CBCGPPoint ptAxisStart;
	CBCGPPoint ptAxisEnd;
	
	GetAxisPos(ptAxisStart, ptAxisEnd);

	CBCGPChartAxisScalePart part;
	if (!ScalePartFromValue(dblValue, part))
	{
		if (!m_scaleBreakOptions.m_bAutomatic)
		{
			CBCGPChartAxisScaleBreak sb;
			int nIndex = 0;
			if (ScaleBreakFromValue(dblValue, sb, nIndex))
			{
				POSITION posPart = m_scaleParts.FindIndex(nIndex);
				POSITION posPartNext = m_scaleParts.FindIndex(nIndex + 1);
				if (posPart != NULL)
				{
					part = m_scaleParts.GetAt(posPart);

					if (fabs(sb.m_dblStart - dblValue) <= fabs(sb.m_dblEnd - dblValue) || posPartNext == NULL)
					{
						return part.m_dblScreenEnd;
					}
					
					part = m_scaleParts.GetAt(posPartNext);
					return part.m_dblScreenStart;
				}
			}
		}
		return IsVertical() ? ptAxisStart.y : ptAxisStart.x;
	}

	double dblMinValue = part.GetMinimumDisplayed();
	double dblMaxValue = part.GetMaximumDisplayed();

	double dblRange = dblMaxValue - dblMinValue;
	
	if (dblRange == 0)
	{
		return part.m_dblScreenStart;
	}
	
	double dblPoint = 0.;
	
	if (IsDisplayDataBetweenTickMarks() && !bForceValueOnThickMark)
	{
		dblValue += GetBaseUnitSize() / 2;
	}
	
	double dblAxisSize = fabs(part.m_dblScreenEnd - part.m_dblScreenStart);
	
	if (m_bReverseOrder)
	{
		if (IsVertical())
		{
			dblPoint = part.m_dblScreenStart + ((dblValue - dblMinValue) * dblAxisSize / dblRange);
		}
		else
		{
			dblPoint = part.m_dblScreenStart - ((dblValue - dblMinValue) * dblAxisSize / dblRange);
		}
	}
	else
	{
		if (IsVertical())
		{
			dblPoint = part.m_dblScreenStart - ((dblValue - dblMinValue) * dblAxisSize / dblRange);
		}
		else
		{
			dblPoint = part.m_dblScreenStart  + ((dblValue - dblMinValue) * dblAxisSize / dblRange);
		}
	}
	
	
	return dblPoint;
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::ValueFromPointOnAxisWithBreak(const CBCGPPoint& pt, CBCGPChartAxis::RoundType roundType) const
{
	ASSERT_VALID(this);
	ASSERT(IsScaleBreakEnabled());
	ASSERT(m_scaleParts.GetCount() > 1);

	CBCGPChartAxisScalePart part;
	int nIndex = 0;
	if (!ScalePartFromPoint(pt, part, nIndex))
	{
		return 0;
	}

	double dblRange = part.GetRange();
	
	if (dblRange == 0)
	{
		return 0;
	}

	double dblAxisSize = fabs(part.GetSize());
	double dblPointOffset = IsVertical()
								? (m_bReverseOrder ? pt.y - part.m_dblScreenStart : part.m_dblScreenStart - pt.y)
								: (m_bReverseOrder ? part.m_dblScreenStart - pt.x : pt.x - part.m_dblScreenStart);
	
	double dblMinimumDisplayed = part.GetMinimumDisplayed();
	double dblVal = (dblRange * dblPointOffset) / dblAxisSize;
	
	if (IsDisplayDataBetweenTickMarks())
	{
		double dblBaseUnitSize = m_bFormatAsDate && !IsIndexedSeries() ? part.m_dblMajorUnit : 1.0;
		dblVal -= dblBaseUnitSize / 2;
	}

	if (roundType == CBCGPChartAxis::RT_FLOOR)
	{
		dblVal = ((int)(dblVal / part.m_dblMajorUnit)) * part.m_dblMajorUnit;
	}
	else if (roundType != CBCGPChartAxis::RT_EXACT)
	{
		dblVal = ((int)(dblVal / part.m_dblMajorUnit) + 1) * part.m_dblMajorUnit;
	}

	return dblVal + dblMinimumDisplayed;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::IsScalePartValid(const CBCGPChartAxisScalePart& part) const
{
	return part.m_dblMaximum >= part.m_dblMinimum /*&& part.m_dblMinimum >= m_dblMinDisplayedValue*/ && 
			part.m_dblMinimum < m_dblMaxDisplayedValue;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::CalcMajorMinorUnitsForScaleParts(CBCGPGraphicsManager* /*pGM*/)
{
	if (!m_scaleBreakOptions.m_bAutomatic && m_scaleBreaks.GetCount() == 0 ||
		m_scaleBreakOptions.m_bAutomatic && m_scaleParts.GetCount() < 2)
	{
		return;
	}

	POSITION pos = NULL;
	for (pos = m_scaleParts.GetHeadPosition(); pos != NULL; )
	{
		CBCGPChartAxisScalePart& part = (CBCGPChartAxisScalePart&) m_scaleParts.GetNext(pos);
		part.m_bValid = FALSE;
		part.m_dblMinimumDisplayed = DBL_MAX;
		part.m_dblMaximumDisplayed = DBL_MAX;
	}

	double dblSavedMinDisplayedValue = m_dblMinDisplayedValue;
	double dblSavedMaxDisplayedValue = m_dblMaxDisplayedValue;

	BOOL bLastPart = FALSE;
	for (pos = m_scaleParts.GetHeadPosition(); pos != NULL; m_scaleParts.GetNext(pos))
	{
		CBCGPChartAxisScalePart part = m_scaleParts.GetAt(pos);

		UINT nMaxLines = GetMaxTotalLines();
		UINT nDisplayedLines = (UINT) part.GetNumberOfUnitsToDisplay(IsVertical());

		if (m_scaleBreakOptions.m_bAutomatic)
		{
			nMaxLines = min((UINT)part.m_nValuesCount * 2, nMaxLines);
		}

		if (nMaxLines < 4)
		{
			nMaxLines = 4;
		}

		if (nDisplayedLines > nMaxLines)
		{
			nDisplayedLines = nMaxLines;
		}

		part.m_bValid = IsScalePartValid(part);

		if (!part.m_bValid)
		{
			bLastPart = TRUE;

			if (pos == m_scaleParts.GetHeadPosition())
			{
				break;
			}
			
			m_scaleParts.GetPrev(pos);
			part = (CBCGPChartAxisScalePart&) m_scaleParts.GetAt(pos);
			part.m_dblScreenEnd = m_scaleParts.GetTail().m_dblScreenEnd;
		}

		double dblRange = part.GetRange();

		part.m_dblMajorUnit = CalcMajorUnit(nDisplayedLines, dblRange);
		part.m_dblMinorUnit = part.m_dblMajorUnit / m_nMinorUnitCount;

		if (nDisplayedLines > 4)
		{
			double dblUnit = part.m_dblMajorUnit;
			
			do 
			{
				nDisplayedLines--;
				if (nDisplayedLines < 3)
				{
					break;
				}
				dblUnit = CalcMajorUnit(nDisplayedLines, part.GetRange());
			}
			while (dblUnit == part.m_dblMajorUnit);
			
			nDisplayedLines++;
		}

		BOOL bFirstOrLastPart = FALSE;

		if (pos == m_scaleParts.GetHeadPosition())
		{
			if (IsFixedMinimumDisplayValue())
			{
				m_dblMinDisplayedValue = m_dblMinimum;
			}
			else if (m_dblMinimum >= 0)
			{
				if (m_dblMinimum - part.m_dblMajorUnit > 2 * part.m_dblMajorUnit && !m_scaleBreakOptions.m_bStartFromZero)
				{
					m_dblMinDisplayedValue = max(0, ApproximateValue(m_dblMinimum, part.m_dblMajorUnit, 1.04) - 2 * part.m_dblMajorUnit);
				}
				else
				{
					m_dblMinDisplayedValue = 0;
				}
			}
			else
			{
				m_dblMinDisplayedValue = ApproximateValue(part.m_dblMinimum, part.m_dblMajorUnit, 1.04) - part.m_dblMajorUnit;
				if (m_dblMinDisplayedValue > part.GetMinimumDisplayed())
				{
					m_dblMinDisplayedValue -= part.m_dblMajorUnit;
				}
			}

			part.m_dblMinimumDisplayed = m_dblMinDisplayedValue;

			part.m_dblMajorUnit = CalcMajorUnit(nDisplayedLines, part.GetRange());
			part.m_dblMinorUnit = part.m_dblMajorUnit / m_nMinorUnitCount;

			bFirstOrLastPart = TRUE;
		}

		if (pos == m_scaleParts.GetTailPosition() || bLastPart)
		{
			if (IsFixedMaximumDisplayValue())
			{
				m_dblMaxDisplayedValue = m_dblMaximum;
			}
			else if (m_dblMaximum < 0)
			{
				if (fabs(m_dblMaximum) - part.m_dblMajorUnit > 2 * part.m_dblMajorUnit && !m_scaleBreakOptions.m_bStartFromZero)
				{
					m_dblMaxDisplayedValue = min(0, ApproximateValue(m_dblMaximum, part.m_dblMajorUnit, 1.04) + 2 * part.m_dblMajorUnit);
				}
				else
				{
					m_dblMaxDisplayedValue = 0;
				}
			}
			else
			{
				m_dblMaxDisplayedValue = ApproximateValue(m_dblMaximum, part.m_dblMajorUnit, 1.04) + part.m_dblMajorUnit;
				if (part.m_dblMinimum < 0)
				{
					POSITION posPrev = pos;
					m_scaleParts.GetPrev(posPrev);

					if (posPrev != NULL)
					{
						const CBCGPChartAxisScalePart& partPrev = m_scaleParts.GetAt(posPrev);

						double dblMinDisplayed = m_dblMaxDisplayedValue - (nDisplayedLines + 1) * part.m_dblMajorUnit;;
						if (dblMinDisplayed > partPrev.GetMaximumDisplayed())
						{
							part.m_dblMinimumDisplayed = dblMinDisplayed;
						}
						else
						{
							dblMinDisplayed = m_dblMaxDisplayedValue - nDisplayedLines * part.m_dblMajorUnit;
							if (dblMinDisplayed > partPrev.GetMaximumDisplayed() && dblMinDisplayed < part.m_dblMinimum)
							{
								part.m_dblMinimumDisplayed = dblMinDisplayed;
							}
							else
							{
								dblMinDisplayed = m_dblMaxDisplayedValue - (nDisplayedLines - 1) * part.m_dblMajorUnit;
								if (dblMinDisplayed > partPrev.GetMaximumDisplayed() && dblMinDisplayed < part.m_dblMinimum)
								{
									part.m_dblMinimumDisplayed = dblMinDisplayed;
								}
							}
						}

						if (part.m_dblMinimumDisplayed > part.m_dblMinimum)
						{
							part.m_dblMinimumDisplayed = part.m_dblMinimum - part.m_dblMajorUnit;
						}
					}
				}
			}

			part.m_dblMaximumDisplayed = m_dblMaxDisplayedValue;

			part.m_dblMajorUnit = CalcMajorUnit(nDisplayedLines, part.GetRange());
			part.m_dblMinorUnit = part.m_dblMajorUnit / m_nMinorUnitCount;

			bFirstOrLastPart = TRUE;
		}

		if (!bFirstOrLastPart && part.m_dblMinimum < 0 && part.m_dblMaximum > 0)
		{
			POSITION posPrev = pos;
			m_scaleParts.GetPrev(posPrev);
			if (posPrev != NULL)
			{
				const CBCGPChartAxisScalePart& partPrev = m_scaleParts.GetAt(posPrev);

				double dblMaxDisplayed = ApproximateValue(part.m_dblMaximum, part.m_dblMajorUnit, 1.04) + part.m_dblMajorUnit;
				double dblMinDisplayed = dblMaxDisplayed - (nDisplayedLines + 1) * part.m_dblMajorUnit;

				if (dblMinDisplayed > partPrev.GetMaximumDisplayed() && dblMinDisplayed < part.m_dblMinimum)
				{
					part.m_dblMaximumDisplayed = dblMaxDisplayed;
					part.m_dblMinimumDisplayed = dblMinDisplayed;
				}
				else
				{
					dblMinDisplayed = m_dblMaxDisplayedValue - nDisplayedLines * part.m_dblMajorUnit;
					if (dblMinDisplayed > partPrev.GetMaximumDisplayed() && dblMinDisplayed < part.m_dblMinimum)
					{
						part.m_dblMaximumDisplayed = dblMaxDisplayed;
						part.m_dblMinimumDisplayed = dblMinDisplayed;
					}
					else
					{
						dblMinDisplayed = m_dblMaxDisplayedValue - (nDisplayedLines - 1) * part.m_dblMajorUnit;
						if (dblMinDisplayed > partPrev.GetMaximumDisplayed() && dblMinDisplayed < part.m_dblMinimum)
						{
							part.m_dblMaximumDisplayed = dblMaxDisplayed;
							part.m_dblMinimumDisplayed = dblMinDisplayed;
						}
					}
				}

				if (part.m_dblMinimumDisplayed > part.m_dblMinimum)
				{
					part.m_dblMinimumDisplayed = part.m_dblMinimum - part.m_dblMajorUnit;
				}
			}
		}

		m_scaleParts.SetAt(pos, part);

		if (bLastPart)
		{
			break;
		}
	}

	if (GetValidPartCount() < 2)
	{
		m_dblMinDisplayedValue = dblSavedMinDisplayedValue;
		m_dblMaxDisplayedValue = dblSavedMaxDisplayedValue;
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::CalcMaxLabelSizeForScaleParts(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);
	ASSERT(IsScaleBreakEnabled());

	if (m_scaleParts.GetCount() == 0 || GetAxisLabelType() == CBCGPChartAxis::ALT_NO_LABELS)
	{
		return;
	}

	CBCGPSize szPadding = m_axisLabelsFormat.GetContentPadding(TRUE);

	for (POSITION pos = m_scaleParts.GetHeadPosition(); pos != NULL;)
	{
		CBCGPChartAxisScalePart part = m_scaleParts.GetNext(pos);

		double dblMinValue = part.GetMinimumDisplayed();
		part.m_szMaxLabelSize.SetSize(16, 16);
		
		if (part.m_dblMajorUnit > 0)
		{
			part.m_szMaxLabelSize.SetSize(0, 0);

			for (double dblVal = dblMinValue; dblVal <= part.GetMaximumDisplayed(); dblVal += part.m_dblMajorUnit)
			{
				CString strLabel;
				GetDisplayedLabel(dblVal, strLabel);
				if (!strLabel.IsEmpty())
				{
					CBCGPSize sizeText = m_pChart->OnCalcAxisLabelSize(pGM, this, dblVal, strLabel, m_axisLabelsFormat.m_textFormat);
					part.m_szMaxLabelSize.cx = max(sizeText.cx, part.m_szMaxLabelSize.cx);
					part.m_szMaxLabelSize.cy = max(sizeText.cy, part.m_szMaxLabelSize.cy);
				}
			}
		}

		m_szMaxLabelSize.cx = max(m_szMaxLabelSize.cx, part.m_szMaxLabelSize.cx); 
		m_szMaxLabelSize.cy = max(m_szMaxLabelSize.cy, part.m_szMaxLabelSize.cy); 

		part.m_szMaxLabelSize.cx += szPadding.cx * 2;
		part.m_szMaxLabelSize.cy += szPadding.cy * 2;
	}
}
//----------------------------------------------------------------------------//
// End scale break support
//----------------------------------------------------------------------------//
double CBCGPChartAxis::GetAxisUnitSize()
{
	static BOOL bOldWay = TRUE;
	ASSERT_VALID(this);

	if (bOldWay)
	{
		if (m_nDisplayedLines == 0)
		{
			return fabs(PointFromValue(m_dblMajorUnit, FALSE) - PointFromValue(0, FALSE)) / m_dblMajorUnit;
		}
		
		double nLen = fabs(GetAxisSize(TRUE)); 
		if (m_nDisplayedLines == 1)
		{
			return nLen / m_dblMajorUnit;
		}
		
		int nDisplayedLines = m_nDisplayedLines;
		
		if (m_bFormatAsDate)
		{
			return (nLen / (nDisplayedLines)) / m_dblMajorUnit;
		}
		
		return (nLen / (nDisplayedLines));
	}

	double nLen = fabs(GetAxisSize(TRUE)); 

	if (m_bFormatAsDate && !IsIndexedSeries() && m_nDisplayedLines != 0)
	{
		if (m_nDisplayedLines == 1)
		{
			return nLen / m_dblMajorUnit;
		}

		return (nLen / (m_nDisplayedLines)) / m_dblMajorUnit;
	}

	return fabs(PointFromValue(m_dblMajorUnit, FALSE) - PointFromValue(0, FALSE)) / m_dblMajorUnit;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::IsDiagram3D() const
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(m_pChart);

	return m_pChart->IsChart3D();
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::GetAxisSize(BOOL bLogical) const
{
	ASSERT_VALID(this);

	if (IsDiagram3D())
	{
		if (bLogical)
		{
			CBCGPPoint ptDist = m_ptAxisEnd3D - m_ptAxisStart3D;
			CBCGPChartData::ComponentIndex ci = GetComponentIndex();

			switch (ci)
			{
			case CBCGPChartData::CI_X:
			case CBCGPChartData::CI_Y:
				return IsVertical() ? ptDist.y : ptDist.x;
			case CBCGPChartData::CI_Z:
				return ptDist.z;
			}
		}

		return m_dblAxisSize3D;
	}

	return IsVertical() ? m_ptAxisStart.y - m_ptAxisEnd.y : m_ptAxisEnd.x - m_ptAxisStart.x;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::IsPointOnAxis(const CBCGPPoint& pt)
{
	double dblCorrection = 0.01;

	CBCGPPoint ptStart;
	CBCGPPoint ptEnd;

	GetAxisPos(ptStart, ptEnd);

	if (IsVertical() && ptStart.y < ptEnd.y)
	{
		double tmp = ptStart.y;
		ptStart.y = ptEnd.y;
		ptEnd.y = tmp;
	}
	else if (!IsVertical() && ptStart.x > ptEnd.x)
	{
		double tmp = ptStart.x;
		ptStart.x = ptEnd.x;
		ptEnd.x = tmp;
	}

	return (IsVertical() && (pt.y <= ptStart.y + dblCorrection && pt.y >= ptEnd.y - dblCorrection) || 
 			!IsVertical() && (pt.x >= ptStart.x - dblCorrection && pt.x <= ptEnd.x + dblCorrection));
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::IsValueOnAxis3D(double dblVal)
{
	CBCGPPoint ptAxisStart;
	CBCGPPoint ptAxisEnd;

	GetAxisPos3D(ptAxisStart, ptAxisEnd);

	CBCGPChartData::ComponentIndex ci = GetComponentIndex();

	if (IsVertical())
	{
		if (ptAxisStart.y > ptAxisEnd.y)
		{
			double tmp = ptAxisEnd.y;
			ptAxisEnd.y = ptAxisStart.y;
			ptAxisStart.y = tmp;
		}

		return (dblVal >= ptAxisStart.y && dblVal <= ptAxisEnd.y);
	}

	if (!IsVertical() && ci != CBCGPChartData::CI_Z)
	{
		if (ptAxisStart.x > ptAxisEnd.x)
		{
			double tmp = ptAxisEnd.x;
			ptAxisEnd.x = ptAxisStart.x;
			ptAxisStart.x = tmp;
		}

		return (dblVal >= ptAxisStart.x && dblVal <= ptAxisEnd.x);
	}
	
	if (ptAxisStart.z > ptAxisEnd.z)
	{
		double tmp = ptAxisEnd.z;
		ptAxisEnd.z = ptAxisStart.z;
		ptAxisStart.z = tmp;
	}

	return dblVal >= ptAxisStart.z && dblVal <= ptAxisEnd.z;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnDrawAxisName3D(CBCGPGraphicsManager* pGM)
{
	if (m_pChart == NULL || !IsDiagram3D() || !IsAxisVisible() || m_bThumbnailMode)
	{
		return;
	}

	CBCGPChartDiagram3D* pDiagram3D = m_pChart->GetDiagram3D();
	if (pDiagram3D == NULL)
	{
		return;
	}

	CBCGPPoint ptAxisStart;
	CBCGPPoint ptAxisEnd;
	
	GetAxisPos(ptAxisStart, ptAxisEnd);

	double dblAngle = bcg_angle(ptAxisStart, ptAxisEnd, FALSE);
	double dblDistanceX = 1.2 * m_szMaxLabelSize.cx + 2 * GetLabelDistance() + m_dblAxisNameDistance3D;
	double dblDistanceY = 1.2 * m_szMaxLabelSize.cy + 2 * GetLabelDistance() + m_dblAxisNameDistance3D;

	double dblXRotation = pDiagram3D->GetXRotation();
	CBCGPPoint ptCenter ((ptAxisEnd.x + ptAxisStart.x) / 2, (ptAxisEnd.y + ptAxisStart.y) / 2);

	CBCGPChartData::ComponentIndex nComponentIndex = GetComponentIndex();
	if (nComponentIndex == CBCGPChartData::CI_X && IsVertical())
	{
		nComponentIndex = CBCGPChartData::CI_Y;
	}
	else if (nComponentIndex == CBCGPChartData::CI_Y && !IsVertical())
	{
		nComponentIndex = CBCGPChartData::CI_X;
	}

	if ((nComponentIndex == CBCGPChartData::CI_X && dblXRotation >= 90 && dblXRotation <= 270) ||
		(nComponentIndex == CBCGPChartData::CI_Y && (dblXRotation < 90 || dblXRotation > 270)) ||
		(nComponentIndex == CBCGPChartData::CI_Z && dblXRotation > 180))
	{
		ptCenter.x += dblDistanceX * sin(dblAngle); 
		ptCenter.y -= dblDistanceY * cos(dblAngle); 	
	}
	else
	{
		ptCenter.x -= dblDistanceX * sin(dblAngle); 
		ptCenter.y += dblDistanceY * cos(dblAngle); 
	}

	CBCGPRect rectName;
	rectName.SetRect(ptCenter, ptCenter);
	rectName.InflateRect(m_rectAxisName.Width() / 2, m_rectAxisName.Height() / 2);

	dblAngle = bcg_rad2deg(bcg_normalize_rad(dblAngle));

	double dblDrawAngle = 360 - dblAngle;

 	if (dblDrawAngle > 90)
 	{
 		dblDrawAngle -= 180;

		if (dblDrawAngle > 90)
		{
			dblDrawAngle -= 180;
		}
 	}

	const CBCGPBrush& brText = m_axisNameFormat.m_brTextColor.IsEmpty() ? m_pChart->GetColors().m_brAxisNameTextColor :
																			m_axisNameFormat.m_brTextColor;

	double dblOldDrawAngle = m_axisNameFormat.m_textFormat.GetDrawingAngle();
	m_axisNameFormat.m_textFormat.SetDrawingAngle(dblDrawAngle);
	pGM->DrawText(m_strAxisName, rectName, m_axisNameFormat.m_textFormat, brText);
	m_axisNameFormat.m_textFormat.SetDrawingAngle(dblOldDrawAngle);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnDrawAxisOnThickWall(CBCGPEngine3D* pEngine, CBCGPChartSide3D* pSide)
{
	if (m_pChart == NULL || !IsDiagram3D() || !IsAxisVisible() || pSide == NULL)
	{
		return;
	}

	CBCGPChartDiagram3D* pDiagram3D = m_pChart->GetDiagram3D();
	CBCGPChartShape3DWall* pWall = DYNAMIC_DOWNCAST(CBCGPChartShape3DWall, pSide->GetShape());

	if (pWall == NULL)
	{
		return;
	}

	CBCGPPoint ptAxisStart;
	CBCGPPoint ptAxisEnd;

	GetAxisPos3D(ptAxisStart, ptAxisEnd);

	OnDrawAxisLine(pEngine->GetDefaultManager());

	if (m_majorTickMarkType == CBCGPChartAxis::TMT_NO_TICKS && m_minorTickMarkType == CBCGPChartAxis::TMT_NO_TICKS)
	{
		return;
	}

	CBCGPPoint ptInterval(GetFixedIntervalWidthScaled(), GetFixedIntervalWidthScaled());

	if (IsVertical())
	{
		m_bReverseOrder ? ptInterval.y = ptAxisStart.y + ptInterval.y : ptInterval.y = m_ptAxisStart.y - ptInterval.y;
	}
	else
	{
		m_bReverseOrder ? ptInterval.x = ptAxisStart.x - ptInterval.x : ptInterval.x += ptAxisStart.x;
	}

	double dblUnitsInInterval = m_bIsFixedIntervalWidth ? m_nValuesPerInterval : UnitsInIntervalFromMajorUnitSpacing();

	if (dblUnitsInInterval <= 0)
	{
		dblUnitsInInterval = 1.;
	}

	double dblMinorUnit = dblUnitsInInterval / m_nMinorUnitCount;
	double dblCurrValue = GetMinDisplayedValue();

	if (m_bUseApproximation && !IsLogScale())
	{
		dblCurrValue = ApproximateValue(dblCurrValue, m_dblMajorUnit, 1.) - m_dblMajorUnit;
	}

	double dblMinorTickMarkLen = GetMinorTickMarkLen(TRUE);

	CBCGPPoint ptScale(dblMinorTickMarkLen, 0);
	ptScale = pDiagram3D->TranslateDistance(ptScale);
	dblMinorTickMarkLen = ptScale.x;

	double dblUnitStep = CalcLog(GetMinDisplayedValue());

	int nWallPos = pWall->GetWallPosition();
	int nSideIndex = pSide->GetSideIndex();
	BOOL bIsFrontOrBack = nSideIndex == CBCGPChartShape3DCube::CUBE_SIDE_FRONT || 
							nSideIndex == CBCGPChartShape3DCube::CUBE_SIDE_BACK;

	double dblMaxValue = GetMaxDisplayedValue();

	while (dblCurrValue <= dblMaxValue)
	{
		double dblVal = PointFromValue(dblCurrValue, TRUE, FALSE);
		CBCGPPoint ptStartTick = GetPointOnAxisPlane3D(dblVal, TRUE);
		CBCGPPoint ptEndTick = ptStartTick;

		BOOL bDrawMajorTicks = IsValueOnAxis3D(dblVal);

		if (IsVertical())
		{
			if (bIsFrontOrBack)
			{
				ptEndTick.x = ptAxisStart.x;
			}
			else
			{
				ptEndTick.z = ptAxisStart.z;
			}
		}
		else
		{
			ptEndTick.y = ptAxisStart.y;
		}

		ptStartTick = pDiagram3D->TransformPoint(ptStartTick);
		ptEndTick = pDiagram3D->TransformPoint(ptEndTick);

		if (m_majorTickMarkType != TMT_NO_TICKS && bDrawMajorTicks)
		{
			m_pChart->OnDrawTickMark(pEngine->GetDefaultManager(), ptStartTick, ptEndTick, m_axisLineFormat, dblCurrValue, TRUE);
		}

		if ((m_minorTickMarkType != CBCGPChartAxis::TMT_NO_TICKS) && 
			dblCurrValue < dblMaxValue && 
			dblCurrValue >= GetMinDisplayedValue())
		{
			double dblMinorValue = IsComponentXSet() ? dblCurrValue : dblCurrValue + dblMinorUnit;
			int nMaxCount = IsComponentXSet() ? m_nMinorUnitCount : m_nMinorUnitCount - 1;
			double dblLogStep = (pow(m_dblMajorUnit, dblUnitStep + 1) - dblCurrValue) / m_nMinorUnitCount;

			if (dblMinorValue > GetMaxDisplayedValue() + GetDoubleCorrection())
			{
				break;
			}

			for (int i = 0; i < nMaxCount; i++)
			{
				double dblVal = PointFromValue(dblMinorValue, TRUE, FALSE);
				CBCGPPoint ptStartTick = GetPointOnAxisPlane3D(dblVal, TRUE);
				CBCGPPoint ptEndTick = ptStartTick;

				if (!IsValueOnAxis3D(dblVal))
				{
					if (IsLogScale())
					{
						dblMinorValue += dblLogStep;
					}
					else
					{
						dblMinorValue += dblMinorUnit;
					}
					continue;
				}

				if (IsVertical())
				{
					if (bIsFrontOrBack)
					{
						ptEndTick.x = ptAxisStart.x;
						ptStartTick.x -= dblMinorTickMarkLen;
					}
					else
					{
						if (nSideIndex == CBCGPChartShape3DCube::CUBE_SIDE_LEFT && 
							nWallPos == CBCGPChartShape3DWall::WALL_POS_FRONT)
						{
							ptStartTick.z -= dblMinorTickMarkLen;
						}
						else
						{
							ptStartTick.z += dblMinorTickMarkLen;
						}
						ptEndTick.z = ptAxisStart.z;
					}
				}
				else
				{
					ptStartTick.y = ptAxisStart.y + dblMinorTickMarkLen;
					ptEndTick.y = ptAxisStart.y;
				}

				ptStartTick = pDiagram3D->TransformPoint(ptStartTick);
				ptEndTick = pDiagram3D->TransformPoint(ptEndTick);

				m_pChart->OnDrawTickMark(pEngine->GetDefaultManager(), ptStartTick, ptEndTick, m_axisLineFormat, dblMinorValue, FALSE);
				
				if (IsLogScale())
				{
					dblMinorValue += dblLogStep;
				}
				else
				{
					dblMinorValue += dblMinorUnit;
				}
			}
		}

		if (IsLogScale())
		{
			dblUnitStep += 1.;
			dblCurrValue = pow(m_dblMajorUnit, dblUnitStep);
		}
		else
		{
			dblCurrValue += dblUnitsInInterval;
		}
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnDrawInterlaceOnThickWall(CBCGPEngine3D* pEngine, CBCGPChartSide3D* /*pSide*/, BOOL bIsNearWall)
{
	if (pEngine == NULL)
	{
		return;
	}

	BOOL bSavedNear = m_bEnableInterlaceNearWall;
	BOOL bSavedFar = m_bEnableInterlaceFarWall;

	m_bEnableInterlaceNearWall = bIsNearWall && bSavedNear;
	m_bEnableInterlaceFarWall = !bIsNearWall && bSavedFar;

	if (m_bEnableInterlaceNearWall || m_bEnableInterlaceFarWall)
	{
		OnFillUnitIntervals(pEngine->GetDefaultManager(), CBCGPRect());
	}

	m_bEnableInterlaceNearWall = bSavedNear;
	m_bEnableInterlaceFarWall = bSavedFar;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnDrawGridLinesOnThickWall(CBCGPEngine3D* pEngine, CBCGPChartSide3D* /*pSide*/, BOOL bIsNearWall)
{
	if (pEngine == NULL)
	{
		return;
	}

	BOOL bSavedNear = m_bEnableGridLineNearWall;
	BOOL bSavedFar = m_bEnableGridLineFarWall;

	m_bEnableGridLineNearWall = bIsNearWall && bSavedNear;
	m_bEnableGridLineFarWall = !bIsNearWall && bSavedFar;

	if (m_bEnableGridLineNearWall || m_bEnableGridLineFarWall)
	{
		OnDrawMajorGridLines(pEngine->GetDefaultManager(), CBCGPRect());
		OnDrawMinorGridLines(pEngine->GetDefaultManager(), CBCGPRect());
	}

	m_bEnableGridLineNearWall = bSavedNear;
	m_bEnableGridLineFarWall = bSavedFar;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagram*/)
{
	ASSERT_VALID(this);

	BOOL bIsAxisVisible = IsAxisVisible();

	if (m_pChart == NULL || !bIsAxisVisible)
	{
		return;
	}

	if (m_nDisplayedLines == 0)
	{
		OnDrawScrollBar(pGM, m_rectScrollBar);
		OnDrawAxisLine(pGM);
	}

	if (m_dblMajorUnit == 0)
	{
		return;
	}

	CBCGPPoint ptAxisStart;
	CBCGPPoint ptAxisEnd;

	GetAxisPos(ptAxisStart, ptAxisEnd);

	CBCGPPoint ptPrevGridLineStart = ptAxisStart;

	CBCGPChartAxis* pPerpAxis = GetPerpendecularAxis();
	
	double dblUnitsInInterval = m_bIsFixedIntervalWidth ? m_nValuesPerInterval : UnitsInIntervalFromMajorUnitSpacing();

	if (dblUnitsInInterval <= 0)
	{
		dblUnitsInInterval = 1.;
	}

	double dblScrollBarSize = CanShowScrollBar() ? GetScrollBarSize(TRUE) - 1 : 0;

	double dblMinorUnit = dblUnitsInInterval / m_nMinorUnitCount;
	double dblCurrValue = GetMinDisplayedValue();

	if (m_bUseApproximation && !IsLogScale())
	{
		dblCurrValue = ApproximateValue(dblCurrValue, m_dblMajorUnit, 1.) - m_dblMajorUnit;
	}

	CBCGPChartAxis::TickMarkType tmtMajor = m_majorTickMarkType;
	CBCGPChartAxis::TickMarkType tmtMinor = m_minorTickMarkType;

	BOOL bReverseTicks = pPerpAxis->m_bReverseOrder && !IsDiagram3D();

	CBCGPChartDiagram3D* pDiagram3D = m_pChart->GetDiagram3D();

	if (IsDiagram3D() && IsVertical() && pDiagram3D != NULL)
	{
		double dblXRotation = pDiagram3D->GetXRotation();

		if (dblXRotation >= 90 && dblXRotation < 270)
		{
			bReverseTicks = TRUE;
		}
	}

	if (bReverseTicks)
	{
		if (tmtMajor == CBCGPChartAxis::TMT_INSIDE)
		{
			tmtMajor = CBCGPChartAxis::TMT_OUTSIDE;
		}
		else if (tmtMajor == CBCGPChartAxis::TMT_OUTSIDE)
		{
			tmtMajor = CBCGPChartAxis::TMT_INSIDE;
		}

		if (tmtMinor == CBCGPChartAxis::TMT_INSIDE)
		{
			tmtMinor = CBCGPChartAxis::TMT_OUTSIDE;
		}
		else if (tmtMinor == CBCGPChartAxis::TMT_OUTSIDE)
		{
			tmtMinor = CBCGPChartAxis::TMT_INSIDE;
		}
	}

	double dblMajorTickMarkLen = GetMajorTickMarkLen(TRUE);
	double dblMinorTickMarkLen = GetMinorTickMarkLen(TRUE);

	double dblUnitStep = CalcLog(GetMinDisplayedValue());
	double dblMaxValue = GetMaxDisplayedValue() + GetDoubleCorrection();

	BOOL bUseScaleBreaks = CanUseScaleBreaks();
	POSITION posScalePart = m_scaleParts.GetHeadPosition();
	CBCGPChartAxisScalePart partCurrent;
	if (bUseScaleBreaks)
	{
		partCurrent = m_scaleParts.GetNext(posScalePart);
		dblUnitsInInterval = partCurrent.m_dblMajorUnit;
		dblMinorUnit = partCurrent.m_dblMinorUnit;
		dblCurrValue = partCurrent.GetMinimumDisplayed();
		dblMaxValue = partCurrent.GetMaximumDisplayed() + GetDoubleCorrection();
	}

	if (IsFixedIntervalWidth())
	{
		dblMaxValue += 1.0;
	}

	while (dblCurrValue <= dblMaxValue)
	{
		ptAxisStart = PointFromValue3D(dblCurrValue, TRUE, FALSE);
		
		BOOL bDrawTick = TRUE;

		if (!IsPointOnAxis(ptAxisStart))
		{
			bDrawTick = FALSE;
		}
		
		CBCGPPoint ptStartTick = ptAxisStart;
		CBCGPPoint ptEndTick = ptAxisStart; 

		if (m_majorTickMarkType != CBCGPChartAxis::TMT_NO_TICKS && bDrawTick)
		{
			switch (tmtMajor)
			{
			case CBCGPChartAxis::TMT_INSIDE:
				if (IsVertical())
				{
					m_bIsSecondaryAxis ? ptStartTick.x -=  dblMajorTickMarkLen: 
						ptStartTick.x += dblMajorTickMarkLen;

					if (m_bTickMarksTopRight && !m_bIsSecondaryAxis)
					{
						ptStartTick.x += dblScrollBarSize;
					}
					else if (!m_bTickMarksTopRight && m_bIsSecondaryAxis)
					{
						ptStartTick.x -= dblScrollBarSize;
					}
				}
				else
				{
					m_bIsSecondaryAxis ? ptStartTick.y += dblMajorTickMarkLen :
						ptStartTick.y -= dblMajorTickMarkLen;					

					if (m_bTickMarksTopRight && !m_bIsSecondaryAxis)
					{
						ptStartTick.y -= dblScrollBarSize;
					}
					else if (!m_bTickMarksTopRight && m_bIsSecondaryAxis)
					{
						ptStartTick.y += dblScrollBarSize;
					}
				}
				break;
			case CBCGPChartAxis::TMT_OUTSIDE:
				if (IsVertical())
				{
					m_bIsSecondaryAxis ? ptStartTick.x += dblMajorTickMarkLen :
							ptStartTick.x -= dblMajorTickMarkLen;

					if (!m_bTickMarksTopRight && !m_bIsSecondaryAxis)
					{
						ptStartTick.x -= dblScrollBarSize;
					}
					else if (m_bTickMarksTopRight && m_bIsSecondaryAxis)
					{
						ptStartTick.x += dblScrollBarSize;
					}
				}
				else
				{
					m_bIsSecondaryAxis ? ptStartTick.y -= dblMajorTickMarkLen: 
						ptStartTick.y += dblMajorTickMarkLen;

					if (!m_bTickMarksTopRight && !m_bIsSecondaryAxis)
					{
						ptStartTick.y += dblScrollBarSize;
					}
					else if (m_bTickMarksTopRight && m_bIsSecondaryAxis)
					{
						ptStartTick.y -= dblScrollBarSize;
					}
				}
				break;
			case CBCGPChartAxis::TMT_CROSS:
				if (IsVertical())
				{
					ptStartTick.x -= dblMajorTickMarkLen;
					ptEndTick.x += dblMajorTickMarkLen;
				}
				else
				{
					ptStartTick.y -= dblMajorTickMarkLen;
					ptEndTick.y += dblMajorTickMarkLen;
				}
				break;
			}

			m_pChart->OnDrawTickMark(pGM, ptStartTick, ptEndTick, m_axisLineFormat, dblCurrValue, TRUE);
		}

		if ((m_minorTickMarkType != CBCGPChartAxis::TMT_NO_TICKS) && dblCurrValue < GetMaxDisplayedValue() + GetDoubleCorrection())
		{
			double dblMinorValue = IsComponentXSet() ? dblCurrValue : dblCurrValue + dblMinorUnit;
			int nMaxCount = IsComponentXSet() ? m_nMinorUnitCount : m_nMinorUnitCount - 1;
			double dblLogStep = (pow(m_dblMajorUnit, dblUnitStep + 1) - dblCurrValue) / m_nMinorUnitCount;

			for (int i = 0; i < nMaxCount; i++)
			{
				ptAxisStart= PointFromValue3D(dblMinorValue, TRUE, FALSE);

				bDrawTick = IsPointOnAxis(ptAxisStart);

				CBCGPPoint ptStartTick = ptAxisStart;
				CBCGPPoint ptEndTick = ptAxisStart; 

				switch (tmtMinor)
				{
				case CBCGPChartAxis::TMT_INSIDE:
					if (IsVertical())
					{
						m_bIsSecondaryAxis ? ptStartTick.x -= dblMinorTickMarkLen :
							ptStartTick.x += dblMinorTickMarkLen;

						if (m_bTickMarksTopRight && !m_bIsSecondaryAxis)
						{
							ptStartTick.x += dblScrollBarSize;
						}
						else if (!m_bTickMarksTopRight && m_bIsSecondaryAxis)
						{
							ptStartTick.x -= dblScrollBarSize;
						}
					}
					else
					{
						m_bIsSecondaryAxis ? ptStartTick.y += dblMinorTickMarkLen :
							ptStartTick.y -= dblMinorTickMarkLen;

						if (m_bTickMarksTopRight && !m_bIsSecondaryAxis)
						{
							ptStartTick.y -= dblScrollBarSize;
						}
						else if (!m_bTickMarksTopRight && m_bIsSecondaryAxis)
						{
							ptStartTick.y += dblScrollBarSize;
						}
					}
					break;
				case CBCGPChartAxis::TMT_OUTSIDE:
					if (IsVertical())
					{
						m_bIsSecondaryAxis ? ptStartTick.x += dblMinorTickMarkLen:
							ptStartTick.x -= dblMinorTickMarkLen;

						if (!m_bTickMarksTopRight && !m_bIsSecondaryAxis)
						{
							ptStartTick.x -= dblScrollBarSize;
						}
						else if (m_bTickMarksTopRight && m_bIsSecondaryAxis)
						{
							ptStartTick.x += dblScrollBarSize;
						}
					}
					else
					{
						m_bIsSecondaryAxis ? ptStartTick.y -= dblMinorTickMarkLen :
							ptStartTick.y += dblMinorTickMarkLen;

						if (!m_bTickMarksTopRight && !m_bIsSecondaryAxis)
						{
							ptStartTick.y += dblScrollBarSize;
						}
						else if (m_bTickMarksTopRight && m_bIsSecondaryAxis)
						{
							ptStartTick.y -= dblScrollBarSize;
						}
					}
					break;
				case CBCGPChartAxis::TMT_CROSS:
					if (IsVertical())
					{
						ptStartTick.x -= dblMinorTickMarkLen;
						ptEndTick.x += dblMinorTickMarkLen;
					}
					else
					{
						ptStartTick.y -= dblMinorTickMarkLen;
						ptEndTick.y += dblMinorTickMarkLen;
					}
					break;
				}

				if (bDrawTick)
				{
					m_pChart->OnDrawTickMark(pGM, ptStartTick, ptEndTick, m_axisLineFormat, dblMinorValue, FALSE);
				}

				if (IsLogScale())
				{
					dblMinorValue += dblLogStep;
				}
				else
				{
					dblMinorValue += dblMinorUnit;
				}
			}
		}

		if (IsLogScale())
		{
			dblUnitStep += 1.;
			dblCurrValue = pow(m_dblMajorUnit, dblUnitStep);
		}
		else
		{
			dblCurrValue += dblUnitsInInterval;

			if (bUseScaleBreaks && dblCurrValue > partCurrent.GetMaximumDisplayed())
			{
				if (posScalePart == NULL)
				{
					break;
				}

				partCurrent = m_scaleParts.GetNext(posScalePart);

				if (!partCurrent.m_bValid)
				{
					break;
				}

				dblUnitsInInterval = partCurrent.m_dblMajorUnit;
				dblMinorUnit = partCurrent.m_dblMinorUnit;
				dblCurrValue = partCurrent.GetMinimumDisplayed();
				dblMaxValue = partCurrent.GetMaximumDisplayed() + GetDoubleCorrection();
			}
		}
	}

	if (bIsAxisVisible)
	{
		OnDrawAxisLine(pGM);
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnDrawScrollBar(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);

	if (IsAxisVisible())
	{
		OnDrawScrollBar(pGM, m_rectScrollBar);
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnDrawAxisLine(CBCGPGraphicsManager* /*pGM*/)
{
	ASSERT_VALID(this);

	const CBCGPBrush& brLineColor = m_axisLineFormat.m_brLineColor.IsEmpty() ? 
			m_pChart->GetColors().m_brAxisLineColor : m_axisLineFormat.m_brLineColor;

	CBCGPPoint ptAxisStart;
	CBCGPPoint ptAxisEnd;
	
	GetAxisPos(ptAxisStart, ptAxisEnd);

	CBCGPChartDiagram3D* pDiagram3D = m_pChart->GetDiagram3D();
	if (pDiagram3D != NULL)
	{
		CBCGPEngine3D* pEngine3D = pDiagram3D->GetEngine3D();

		if (pEngine3D != NULL)
		{
			ASSERT_VALID(pEngine3D);

			pEngine3D->DrawLine(ptAxisStart, ptAxisEnd, brLineColor, m_axisLineFormat.GetLineWidth(TRUE), 
								&m_axisLineFormat.m_strokeStyle);
		}
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnDrawMajorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram)
{
	ASSERT_VALID(this);

	BOOL bIsAxisVisible = IsAxisVisible();

	if (m_nDisplayedLines == 0 || m_pChart == NULL || !bIsAxisVisible || !m_bShowMajorGridLines)
	{
		return;
	}

	CBCGPPoint ptStart = m_ptAxisStart;
	CBCGPPoint ptEnd = m_ptAxisEnd;
	CBCGPPoint ptPrevGridLineStart = ptStart;

	CBCGPChartAxis* pPerpAxis = GetPerpendecularAxis();
	CBCGPPoint ptGridStart;
	CBCGPPoint ptGridEnd;

	if (pPerpAxis == NULL)
	{
		return;
	}

	ASSERT_VALID(pPerpAxis);

	CArray<CBCGPRect, CBCGPRect> arBoundingRects;
	GetBoundingRects(arBoundingRects);

	if (m_bUsePerpAxisForZoning)
	{
		pPerpAxis->GetAxisPos(ptGridStart, ptGridEnd);
	}
	else
	{
		if (IsVertical())
		{
			ptGridStart.SetPoint(rectDiagram.left, m_ptAxisStart.y);
			ptGridEnd.SetPoint(rectDiagram.right, m_ptAxisEnd.y);
		}
		else
		{
			ptGridStart.SetPoint(m_ptAxisStart.x, rectDiagram.bottom);
			ptGridEnd.SetPoint(m_ptAxisEnd.x, rectDiagram.top);
		}
	}

	m_arMajorGridLines.RemoveAll();

	double dblUnitsInInterval = m_bIsFixedIntervalWidth ? m_nValuesPerInterval : UnitsInIntervalFromMajorUnitSpacing();

	if (dblUnitsInInterval <= 0)
	{
		dblUnitsInInterval = 1.;
	}

	double dblCurrValue = GetMinDisplayedValue();

	if (m_bUseApproximation && !IsLogScale())
	{
		dblCurrValue = ApproximateValue(dblCurrValue, m_dblMajorUnit, 1.) - m_dblMajorUnit;
	}

	double dblUnitStep = CalcLog(GetMinDisplayedValue());

	CBCGPPointsArray arAxesPos;

	CBCGPEngine3D* pEngine3D = m_pChart->GetEngine3D();

	BOOL bIsSoftRendering = pEngine3D != NULL && pEngine3D->IsSoftwareRendering();
	BOOL bIsDiagram3D = IsDiagram3D();
	BOOL bIsThickFloor = m_pChart->GetDiagram3D() == NULL || !IsDiagram3D() ? FALSE : m_pChart->GetDiagram3D()->IsThickWallsAndFloor();

	if (bIsDiagram3D && !bIsSoftRendering)
	{
		arAxesPos.SetSize(6);
		if (m_pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS)->IsAxisVisible())
			m_pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS)->GetAxisPos(arAxesPos[0], arAxesPos[1]);
		if (m_pChart->GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS)->IsAxisVisible())
			m_pChart->GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS)->GetAxisPos(arAxesPos[2], arAxesPos[3]);
		if (m_pChart->GetChartAxis(BCGP_CHART_Z_PRIMARY_AXIS)->IsAxisVisible())
			m_pChart->GetChartAxis(BCGP_CHART_Z_PRIMARY_AXIS)->GetAxisPos(arAxesPos[4], arAxesPos[5]);
	}

	double dblMaxValue = bIsThickFloor ? GetMaxDisplayedValue() : GetMaxDisplayedValue() + GetDoubleCorrection();

	BOOL bUseScaleBreaks = CanUseScaleBreaks();
	POSITION posScalePart = m_scaleParts.GetHeadPosition();
	CBCGPChartAxisScalePart partCurrent;
	if (bUseScaleBreaks)
	{
		partCurrent = m_scaleParts.GetNext(posScalePart);
		dblUnitsInInterval = partCurrent.m_dblMajorUnit;
		dblCurrValue = partCurrent.GetMinimumDisplayed();
		dblMaxValue = partCurrent.GetMaximumDisplayed() + GetDoubleCorrection();
	}

	if (IsFixedIntervalWidth())
	{
		dblMaxValue += 1.0;
	}

	while (dblCurrValue <= dblMaxValue)
	{
		ptStart = PointFromValue3D(dblCurrValue, TRUE, TRUE);
		BOOL bIsPointOnAxis = IsPointOnAxis(ptStart);

		if (bIsThickFloor && bIsDiagram3D)
		{
			double dblVal = PointFromValue(dblCurrValue, TRUE, FALSE);
			bIsPointOnAxis = IsValueOnAxis3D(dblVal);
		}

		if (!bIsPointOnAxis || 
			bIsDiagram3D && (dblCurrValue == GetMinDisplayedValue() || dblCurrValue == GetMaxDisplayedValue()))
		{
			if (IsLogScale())
			{
				dblUnitStep += 1.;
				dblCurrValue = pow(m_dblMajorUnit, dblUnitStep);
			}
			else
			{
				dblCurrValue += dblUnitsInInterval;
			}
			continue;
		}
		
		if ((IsVertical() && ptStart.y < m_ptAxisEnd.y || !IsVertical() && ptStart.x > m_ptAxisEnd.x) && 
			dblCurrValue > GetMaxDisplayedValue() + GetDoubleCorrection())
		{
			break;
		}

		if (bIsDiagram3D)
		{
			CBCGPPoint ptEndFloor;
			CBCGPPoint ptEndWall;

			OnCalcFloorAndWallGridLinePoints(dblCurrValue, ptEndFloor, ptEndWall);

			BOOL bDrawLine1 = TRUE && m_bEnableGridLineNearWall;
			BOOL bDrawLine2 = pPerpAxis != NULL && pPerpAxis->IsAxisVisible() && m_bEnableGridLineFarWall;

			if (!bIsSoftRendering)
			{
				static double dblPrecision = 4 * FLT_EPSILON;
				for (int i = 0; i < 6; i += 2)
				{
					if (arAxesPos[i].x != 0 && arAxesPos[i + 1].x != 0)
					{
						if (bDrawLine1)
						{
							if (bcg_pointInLine(arAxesPos[i], arAxesPos[i + 1], ptStart, dblPrecision) != 0 && 
								bcg_pointInLine(arAxesPos[i], arAxesPos[i + 1], ptEndFloor, dblPrecision) != 0)
							{
								bDrawLine1 = FALSE;
							}
						}

						if (bDrawLine2)
						{
							if (bcg_pointInLine(arAxesPos[i], arAxesPos[i + 1], ptEndFloor, dblPrecision) != 0 && 
								bcg_pointInLine(arAxesPos[i], arAxesPos[i + 1], ptEndWall, dblPrecision) != 0)
							{
								bDrawLine2 = FALSE;
							}
						}
					}
				}
			}
			
			if (bDrawLine1)
			{
				m_pChart->OnDrawGridLine(pGM, ptStart, ptEndFloor, this, dblCurrValue, 
														m_majorGridLineFormat, TRUE);
			}

			if (bDrawLine2)
			{
				m_pChart->OnDrawGridLine(pGM, ptEndFloor, ptEndWall, this, dblCurrValue, 
					m_majorGridLineFormat, TRUE);
			}
		}
		else
		{
			BOOL bDrawGridLine = !bUseScaleBreaks || 
				partCurrent.m_scaleBreakStyle == CBCGPChartAxisScaleBreak::ASBS_CONTINUOUS ||
				dblCurrValue < dblMaxValue - partCurrent.m_dblMinorUnit;

			if (bDrawGridLine)
			{
				CBCGPPoint ptMajorGridLineStart(0, 0);
				CBCGPPoint ptMajorGridLineEnd(0, 0);
				
				if (IsVertical())
				{
					ptMajorGridLineStart.SetPoint(ptGridStart.x, ptStart.y);
					ptMajorGridLineEnd.SetPoint(ptGridEnd.x, ptStart.y);
				}
				else
				{
					ptMajorGridLineStart.SetPoint(ptStart.x, ptGridStart.y);
					ptMajorGridLineEnd.SetPoint(ptStart.x, ptGridEnd.y);
				}

				for (int j = 0; j < arBoundingRects.GetSize(); j++)
				{
					CBCGPRect rectBounds = arBoundingRects[j];

					CBCGPPoint ptStartLine = ptMajorGridLineStart;
					CBCGPPoint ptEndLine = ptMajorGridLineEnd;
					
					if (IsVertical())
					{
						ptStartLine.x = rectBounds.left;
						ptEndLine.x = rectBounds.right;
					}
					else
					{
						ptStartLine.y = rectBounds.top;
						ptEndLine.y = rectBounds.bottom;
					}

					m_pChart->OnDrawGridLine(pGM, ptStartLine, ptEndLine, this, dblCurrValue, 
													m_majorGridLineFormat, TRUE);
				}

				CBCGPRect rectMajorGL(ptMajorGridLineStart, ptMajorGridLineEnd);
				IsVertical() ? rectMajorGL.InflateRect(0, 4) : rectMajorGL.InflateRect(4, 0);

				m_arMajorGridLines.Add(rectMajorGL);
			}
		}

		if (IsLogScale())
		{
			dblUnitStep += 1.;
			dblCurrValue = pow(m_dblMajorUnit, dblUnitStep);
		}
		else
		{
			dblCurrValue += dblUnitsInInterval;

			if (bUseScaleBreaks && dblCurrValue > partCurrent.GetMaximumDisplayed())
			{
				if (!GetNextPart(partCurrent, posScalePart, dblUnitsInInterval, dblCurrValue, dblMaxValue))
				{
					break;
				}

				if (dblCurrValue > partCurrent.GetMaximumDisplayed())
				{
					if (!GetNextPart(partCurrent, posScalePart, dblUnitsInInterval, dblCurrValue, dblMaxValue))
					{
						break;
					}
				}
			}
		}
	}
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::GetNextPart(CBCGPChartAxisScalePart& partCurrent, POSITION& posScalePart, 
								 double& dblUnitsInInterval, double& dblCurrValue, double& dblMaxValue)
{
	if (posScalePart == NULL)
	{
		return FALSE;
	}
	
	BOOL bDrawMinimum = partCurrent.m_scaleBreakStyle == CBCGPChartAxisScaleBreak::ASBS_CONTINUOUS;
	
	partCurrent = m_scaleParts.GetNext(posScalePart);
	
	if (!partCurrent.m_bValid)
	{
		return FALSE;
	}
	
	dblUnitsInInterval = partCurrent.m_dblMajorUnit;
	if (bDrawMinimum)
	{
		dblCurrValue = partCurrent.GetMinimumDisplayed();
	}
	else
	{
		dblCurrValue = partCurrent.GetMinimumDisplayed() + dblUnitsInInterval;
	}
	
	dblMaxValue = partCurrent.GetMaximumDisplayed() + GetDoubleCorrection();

	return TRUE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnDrawMinorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram)
{
	ASSERT_VALID(this);

	BOOL bIsAxisVisible = IsAxisVisible();

	if (m_nDisplayedLines == 0 || m_pChart == NULL || !bIsAxisVisible || !m_bShowMinorGridLines)
	{
		return;
	}

	CBCGPPoint ptStart = m_ptAxisStart;
	CBCGPPoint ptEnd = m_ptAxisEnd;
	CBCGPPoint ptPrevGridLineStart = ptStart;

	CBCGPChartAxis* pPerpAxis = GetPerpendecularAxis();
	CBCGPPoint ptGridStart;
	CBCGPPoint ptGridEnd;

	if (m_bUsePerpAxisForZoning)
	{
		pPerpAxis->GetAxisPos(ptGridStart, ptGridEnd);
	}
	else
	{
		if (IsVertical())
		{
			ptGridStart.SetPoint(rectDiagram.left, m_ptAxisStart.y);
			ptGridEnd.SetPoint(rectDiagram.right, m_ptAxisEnd.y);
		}
		else
		{
			ptGridStart.SetPoint(m_ptAxisStart.x, rectDiagram.bottom);
			ptGridEnd.SetPoint(m_ptAxisEnd.x, rectDiagram.top);
		}
	}
	
	m_arMinorGridLines.RemoveAll();

	CBCGPPoint ptInterval(GetFixedIntervalWidthScaled(), GetFixedIntervalWidthScaled());

	if (IsVertical())
	{
		m_bReverseOrder ? ptInterval.y = m_ptAxisStart.y + ptInterval.y : ptInterval.y = m_ptAxisStart.y - ptInterval.y;
	}
	else
	{
		m_bReverseOrder ? ptInterval.x = m_ptAxisStart.x - ptInterval.x : ptInterval.x += m_ptAxisStart.x;
	}

	double dblUnitsInInterval = m_bIsFixedIntervalWidth ? m_nValuesPerInterval : UnitsInIntervalFromMajorUnitSpacing();

	if (dblUnitsInInterval <= 0)
	{
		dblUnitsInInterval = 1.;
	}

	double dblMinorUnit = dblUnitsInInterval / m_nMinorUnitCount;
	double dblCurrValue = GetMinDisplayedValue();

	if (m_bUseApproximation && !IsLogScale())
	{
		dblCurrValue = ApproximateValue(dblCurrValue, m_dblMajorUnit, 1.) - m_dblMajorUnit;
	}

	double dblUnitStep = CalcLog(GetMinDisplayedValue());

	BOOL bIsDiagram3D = IsDiagram3D();
	BOOL bIsThickFloor = m_pChart->GetDiagram3D() == NULL || !bIsDiagram3D ? FALSE : 
																m_pChart->GetDiagram3D()->IsThickWallsAndFloor();

	double dblMaxValue = bIsThickFloor ? GetMaxDisplayedValue() : GetMaxDisplayedValue() + GetDoubleCorrection();

	BOOL bUseScaleBreaks = CanUseScaleBreaks();
	POSITION posScalePart = m_scaleParts.GetHeadPosition();
	CBCGPChartAxisScalePart partCurrent;
	if (bUseScaleBreaks)
	{
		partCurrent = m_scaleParts.GetNext(posScalePart);
		dblUnitsInInterval = partCurrent.m_dblMajorUnit;
		dblMinorUnit = partCurrent.m_dblMinorUnit;
		dblCurrValue = partCurrent.GetMinimumDisplayed();
		dblMaxValue = partCurrent.GetMaximumDisplayed() + GetDoubleCorrection();
	}

	while (dblCurrValue <= dblMaxValue)
	{
		ptStart = PointFromValue3D(dblCurrValue, TRUE, TRUE);
		BOOL bIsPointOnAxis = IsPointOnAxis(ptStart);

		if (bIsThickFloor && bIsDiagram3D)
		{
			double dblVal = PointFromValue(dblCurrValue, TRUE, FALSE);
			bIsPointOnAxis = IsValueOnAxis3D(dblVal);
		}

		if (!bIsPointOnAxis || 
			bIsDiagram3D && (dblCurrValue == GetMinDisplayedValue() || dblCurrValue == GetMaxDisplayedValue()))
		{
			if (IsLogScale())
			{
				dblUnitStep += 1.;
				dblCurrValue = pow(m_dblMajorUnit, dblUnitStep);
			}
			else
			{
				dblCurrValue += dblMinorUnit;
			}
			continue;
		}
		
		if ((IsVertical() && ptStart.y < m_ptAxisEnd.y || !IsVertical() && ptStart.x > m_ptAxisEnd.x) && 
			dblCurrValue > GetMaxDisplayedValue() + GetDoubleCorrection())
		{
			break;
		}

		double dblMinorValue = dblCurrValue;
		double dblLogStep = (pow(m_dblMajorUnit, dblUnitStep + 1) - dblCurrValue) / m_nMinorUnitCount;
		int nMaxCount = IsLogScale() ? m_nMinorUnitCount : 1;

		for (int i = 0; i < nMaxCount; i++)
		{
			if (i != 0)
			{
				ptStart = PointFromValue3D(dblMinorValue, TRUE, TRUE);
				
				if ((IsVertical() && ptStart.y < m_ptAxisEnd.y || !IsVertical() && ptStart.x > m_ptAxisEnd.x) && 
					dblMinorValue > dblMaxValue || !bIsPointOnAxis)
				{
					break;
				}
			}
			
			if (bIsDiagram3D)
			{
				CBCGPPoint ptEndFloor;
				CBCGPPoint ptEndWall;

				OnCalcFloorAndWallGridLinePoints(dblMinorValue, ptEndFloor, ptEndWall);

				if (m_bEnableGridLineNearWall)
				{
					m_pChart->OnDrawGridLine(pGM, ptStart, ptEndFloor, this, dblCurrValue, 
						m_minorGridLineFormat, TRUE);
				}
				

				if (pPerpAxis != NULL && pPerpAxis->IsAxisVisible() && m_bEnableGridLineFarWall)
				{
					m_pChart->OnDrawGridLine(pGM, ptEndFloor, ptEndWall, this, dblCurrValue, 
														m_minorGridLineFormat, TRUE);
				}
			}
			else
			{
				CBCGPPoint ptGridLineStart(0, 0);
				CBCGPPoint ptGridLineEnd(0, 0);

				if (IsVertical())
				{
					ptGridLineStart.SetPoint(ptGridStart.x, ptStart.y);
					ptGridLineEnd.SetPoint(ptGridEnd.x, ptStart.y);
				}
				else
				{
					ptGridLineStart.SetPoint(ptStart.x, ptGridStart.y);
					ptGridLineEnd.SetPoint(ptStart.x, ptGridEnd.y);
				}

				m_pChart->OnDrawGridLine(pGM, ptGridLineStart, ptGridLineEnd, this, dblCurrValue, 
					m_minorGridLineFormat, FALSE);

				CBCGPRect rectMinorGL(ptGridLineStart, ptGridLineEnd);
				if (IsVertical())
				{
					rectMinorGL.InflateRect(0, 4);
				}
				else
				{
					rectMinorGL.InflateRect(4, 0);
				}

				m_arMinorGridLines.Add(rectMinorGL);
			}

			dblMinorValue += dblLogStep;
		}

		if (IsLogScale())
		{
			dblUnitStep += 1.;
			dblCurrValue = pow(m_dblMajorUnit, dblUnitStep);
		}
		else
		{
			dblCurrValue += dblMinorUnit;			

			if (bUseScaleBreaks && dblCurrValue > partCurrent.GetMaximumDisplayed())
			{
				if (posScalePart == NULL)
				{
					break;
				}
				
				partCurrent = m_scaleParts.GetNext(posScalePart);

				if (!partCurrent.m_bValid)
				{
					break;
				}
				
				dblUnitsInInterval = partCurrent.m_dblMajorUnit;
				dblMinorUnit = partCurrent.m_dblMinorUnit;
				dblCurrValue = partCurrent.GetMinimumDisplayed();
				dblMaxValue = partCurrent.GetMaximumDisplayed() + GetDoubleCorrection();
			}
		}
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnFillUnitIntervals(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram)
{
	ASSERT_VALID(this);

	if (m_nDisplayedLines == 0 || m_pChart == NULL || !IsAxisVisible() || !m_bFillMajorUnitInterval)
	{
		return;
	}

	double dblCurrValue = GetMinDisplayedValue();
	int nCount = -1;
	int nStep = m_nUnitFillStep - 1;

	CBCGPPoint	ptStart(0 , 0);
	CBCGPPoint	ptPrevGridLineStart(0, 0);
	CBCGPPoint	ptGridLineStart(0, 0);
	CBCGPPoint	ptGridLineEnd(0, 0);

	CBCGPChartAxis* pPerpAxis = GetPerpendecularAxis();
	CBCGPPoint ptGridStart;
	CBCGPPoint ptGridEnd;

	CArray<CBCGPRect, CBCGPRect> arBoundingRects;
	GetBoundingRects(arBoundingRects);

	if (m_bUsePerpAxisForZoning)
	{
		pPerpAxis->GetAxisPos(ptGridStart, ptGridEnd);
	}
	else
	{
		if (IsVertical())
		{
			ptGridStart.SetPoint(rectDiagram.left, m_ptAxisStart.y);
			ptGridEnd.SetPoint(rectDiagram.right, m_ptAxisEnd.y);
		}
		else
		{
			ptGridStart.SetPoint(m_ptAxisStart.x, rectDiagram.bottom);
			ptGridEnd.SetPoint(m_ptAxisEnd.x, rectDiagram.top);
		}
	}

	double dblUnitsInInterval = m_bIsFixedIntervalWidth ? m_nValuesPerInterval : UnitsInIntervalFromMajorUnitSpacing();

	if (dblUnitsInInterval <= 0)
	{
		dblUnitsInInterval = 1.;
	}

	if (m_bUseApproximation && !IsLogScale())
	{
		dblCurrValue = ApproximateValue(dblCurrValue, m_dblMajorUnit, 1.) - m_dblMajorUnit;
	}

	double dblUnitStep = CalcLog(GetMinDisplayedValue());

	CBCGPPoint ptFloor3D;
	CBCGPPoint ptWall3D;

	CBCGPPoint ptPrevStart3D;
	CBCGPPoint ptPrevFloor3D;
	CBCGPPoint ptPrevWall3D;

	const CBCGPBrush& br3DChart = pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY) ?
		m_pChart->GetColors().m_brAxisInterlaceColor3D :
		m_pChart->GetColors().m_brAxisInterlaceColor3DGDI;

	const CBCGPBrush& brChart = pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY) ?
		m_pChart->GetColors().m_brAxisInterlaceColor :
		m_pChart->GetColors().m_brAxisInterlaceColorGDI;

	const CBCGPBrush& br3D = (m_brInterval.IsEmpty() ? br3DChart : m_brInterval3D); 
	const CBCGPBrush& br = (m_brInterval.IsEmpty() ? brChart : m_brInterval);

	CBCGPChartDiagram3D* pDiagram3D = m_pChart->GetDiagram3D();
	BOOL bIsDiagram3D = IsDiagram3D();
	BOOL bIsThickFloor = m_pChart->GetDiagram3D() == NULL ? FALSE : m_pChart->GetDiagram3D()->IsThickWallsAndFloor();

	double dblMaxValue = GetMaxDisplayedValue() + GetDoubleCorrection();
	BOOL bUseScaleBreaks = CanUseScaleBreaks();
	POSITION posScalePart = m_scaleParts.GetHeadPosition();
	CBCGPChartAxisScalePart partCurrent;
	if (bUseScaleBreaks)
	{
		partCurrent = m_scaleParts.GetNext(posScalePart);
		dblUnitsInInterval = partCurrent.m_dblMajorUnit;
		dblCurrValue = partCurrent.GetMinimumDisplayed();
		dblMaxValue = partCurrent.GetMaximumDisplayed() + GetDoubleCorrection();
	}

	if (IsFixedIntervalWidth())
	{
		dblMaxValue += dblUnitsInInterval + 1.;
	}

	while (dblCurrValue <= dblMaxValue)
	{
		ptStart = PointFromValue3D(dblCurrValue, TRUE, TRUE);
		BOOL bIsPointOnAxis = IsFixedIntervalWidth() || IsPointOnAxis(ptStart);

		if (bIsThickFloor && bIsDiagram3D)
		{
			double dblVal = PointFromValue(dblCurrValue, TRUE, FALSE);
			bIsPointOnAxis = IsValueOnAxis3D(dblVal);
		}

		if (!bIsPointOnAxis)
		{
			if (IsLogScale())
			{
				dblUnitStep += 1.;
				dblCurrValue = pow(m_dblMajorUnit, dblUnitStep);
			}
			else
			{
				dblCurrValue += dblUnitsInInterval;
			}
			continue;
		}
		
		if (!IsFixedIntervalWidth() &&
			((IsVertical() && ptStart.y < m_ptAxisEnd.y || !IsVertical() && ptStart.x > m_ptAxisEnd.x) && 
			 dblCurrValue > GetMaxDisplayedValue() + GetDoubleCorrection()))
		{
			break;
		}

		if (IsDiagram3D())
		{
			OnCalcFloorAndWallGridLinePoints(dblCurrValue, ptFloor3D, ptWall3D);

			if (ptPrevStart3D.x != 0 && nCount >= m_nFirstIntervalToFill)
			{
				if (nStep == (m_nUnitFillStep - 1))
				{
					CBCGPVector4 vNormal;

					if (m_bEnableInterlaceNearWall)
					{
						CBCGPPointsArray arPoints1;
						arPoints1.Add(ptPrevStart3D);
						arPoints1.Add(ptPrevFloor3D);
						arPoints1.Add(ptFloor3D);
						arPoints1.Add(ptStart);

						if (pDiagram3D->IsCalculateNormals())
						{
							vNormal.CalcNormal(ptPrevStart3D, ptPrevFloor3D, ptFloor3D);
							pDiagram3D->GetEngine3D()->SetPolygonNormal(vNormal[0], vNormal[1], vNormal[2]);
						}

						CBCGPPolygonGeometry g1(arPoints1);
						m_pChart->OnFillAxisUnitInterval(pGM, g1, br3D);
					}

					if (pPerpAxis != NULL && pPerpAxis->IsAxisVisible() && m_bEnableInterlaceFarWall)
					{
						CBCGPPointsArray arPoints2;
						arPoints2.Add(ptPrevFloor3D);
						arPoints2.Add(ptPrevWall3D);
						arPoints2.Add(ptWall3D);
						arPoints2.Add(ptFloor3D);

						if (pDiagram3D->IsCalculateNormals())
						{
							vNormal.CalcNormal(ptPrevFloor3D, ptPrevWall3D, ptWall3D);
							pDiagram3D->GetEngine3D()->SetPolygonNormal(vNormal[0], vNormal[1], vNormal[2]);
						}

						CBCGPPolygonGeometry g2(arPoints2);
						m_pChart->OnFillAxisUnitInterval(pGM, g2, br);
					}

					nStep = 0;
				}
				else
				{
					nStep++;
				}
			}
	
			ptPrevStart3D = ptStart;
			ptPrevFloor3D = ptFloor3D;
			ptPrevWall3D = ptWall3D;
		}
		else
		{
			if (IsVertical())
			{
				ptGridLineStart.SetPoint(ptGridStart.x, ptStart.y);
				ptGridLineEnd.SetPoint(ptGridEnd.x, ptStart.y);
			}
			else
			{
				ptGridLineStart.SetPoint(ptStart.x, ptGridStart.y);
				ptGridLineEnd.SetPoint(ptStart.x, ptGridEnd.y);
			}

			if (ptPrevGridLineStart.x != 0 && nCount >= m_nFirstIntervalToFill)
			{
				if (nStep == (m_nUnitFillStep - 1))
				{
					CBCGPRect rectInterval(ptPrevGridLineStart, ptGridLineEnd);
					rectInterval.Normalize();

					for (int j = 0; j < arBoundingRects.GetSize(); j++)
					{
						CBCGPRect rectFill = arBoundingRects [j];
						if (rectFill.IntersectRect(rectInterval))
						{
							m_pChart->OnFillAxisUnitInterval(pGM, rectFill, br);
						}
					}
					nStep = 0;
				}
				else
				{
					nStep++;
				}
			}

			ptPrevGridLineStart = ptGridLineStart;
		}

		nCount++;
		

		if (IsLogScale())
		{
			dblUnitStep += 1.;
			dblCurrValue = pow(m_dblMajorUnit, dblUnitStep);
		}
		else
		{
			dblCurrValue += dblUnitsInInterval;

			if (bUseScaleBreaks && dblCurrValue > partCurrent.GetMaximumDisplayed())
			{
				if (posScalePart == NULL)
				{
					break;
				}
				
				partCurrent = m_scaleParts.GetNext(posScalePart);

				if (!partCurrent.m_bValid)
				{
					break;
				}
				
				dblUnitsInInterval = partCurrent.m_dblMajorUnit;
				dblCurrValue = partCurrent.GetMinimumDisplayed();
				dblMaxValue = partCurrent.GetMaximumDisplayed() + GetDoubleCorrection();
			}
		}
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnDrawAxisLabels(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram)
{
	ASSERT_VALID(this);

	if (m_nDisplayedLines == 0 || m_pChart == NULL || !IsAxisVisible() || 
		GetAxisLabelType() == CBCGPChartAxis::ALT_NO_LABELS)
	{
		return;
	}

	CBCGPPoint ptStart;
	CBCGPPoint ptEnd;

	GetAxisPos(ptStart, ptEnd);

	CBCGPPoint ptAxisStart = ptStart;
	CBCGPPoint ptAxisEnd = ptEnd;

	double dblUnitsInInterval = m_bIsFixedIntervalWidth ? m_nValuesPerInterval : UnitsInIntervalFromMajorUnitSpacing();

	if (dblUnitsInInterval <= 0)
	{
		dblUnitsInInterval = 1.;
	}

	double dblCurrValue = GetMinDisplayedValue();
	double dblMajorUnitInterval = fabs(ScaleValue(dblUnitsInInterval));

	CBCGPRect rectPrevLabel;
	
	BOOL bInterlaceLabels = (IsVertical() && dblMajorUnitInterval < m_szMaxLabelSize.cy  || 
								!IsVertical() && dblMajorUnitInterval < m_szMaxLabelSize.cx ) && 
								IsLabelInterlacingEnabled();
	
	int nCount = 0;
	int nStep = (int)(IsVertical() ? (m_szMaxLabelSize.cy / dblMajorUnitInterval) : (m_szMaxLabelSize.cx / dblMajorUnitInterval)) + 1;

	if (m_bUseApproximation && !IsLogScale())
	{
		dblCurrValue = ApproximateValue(dblCurrValue, m_dblMajorUnit, 1.) - m_dblMajorUnit;
	}

	int nOffset = (int) (IsDisplayDataBetweenTickMarks() ? dblMajorUnitInterval / 2 : 0);

	if (m_bReverseOrder)
	{
		nOffset = -nOffset;
	}

	CBCGPPoint ptOffset;
	
	if (IsVertical())
	{
		ptOffset.y = -nOffset;
	}
	else
	{
		ptOffset.x = nOffset;
	}

	BOOL bIs3D = IsDiagram3D();
	double dblUnitStep = CalcLog(GetMinDisplayedValue());
	BOOL bIsDiagram3D = IsDiagram3D();
	BOOL bIsThickFloor = m_pChart->GetDiagram3D() == NULL ? FALSE : m_pChart->GetDiagram3D()->IsThickWallsAndFloor();

	double dblMaxValue = GetMaxDisplayedValue() + GetDoubleCorrection();
	BOOL bUseScaleBreaks = CanUseScaleBreaks();
	POSITION posScalePart = m_scaleParts.GetHeadPosition();
	CBCGPChartAxisScalePart partCurrent;
	if (bUseScaleBreaks)
	{
		partCurrent = m_scaleParts.GetNext(posScalePart);
		dblUnitsInInterval = partCurrent.m_dblMajorUnit;
		dblCurrValue = partCurrent.GetMinimumDisplayed();
		dblMaxValue = partCurrent.GetMaximumDisplayed() + GetDoubleCorrection();
	}

	if (IsFixedIntervalWidth())
	{
		dblMaxValue += dblUnitsInInterval;
	}

	if (bIs3D && IsPreventLabelOverlapIn3D())
	{
		bInterlaceLabels = FALSE;
	}

	while (dblCurrValue <= dblMaxValue)
	{
		if (bIs3D)
		{
			double dblVal = IsDisplayDataBetweenTickMarks() ? dblCurrValue + m_dblMajorUnit / 2 : dblCurrValue;
			ptStart = PointFromValue3D(dblVal, TRUE, FALSE);
		}
		else
		{
			ptStart = PointFromValue3D(dblCurrValue, TRUE, FALSE);
			ptStart += ptOffset;
		}

		BOOL bIsPointOnAxis = IsPointOnAxis(ptStart);

		if (bIsThickFloor && bIsDiagram3D)
		{
			double dblVal = PointFromValue(dblCurrValue, TRUE, FALSE);
			bIsPointOnAxis = IsValueOnAxis3D(dblVal);
		}

		if (!bIsPointOnAxis)
		{
			if (IsLogScale())
			{
				dblUnitStep += 1.;
				dblCurrValue = pow(m_dblMajorUnit, dblUnitStep);
			}
			else
			{
				dblCurrValue += dblUnitsInInterval;
			}
			continue;
		}

		if ((IsVertical() && ptStart.y < ptAxisEnd.y || !IsVertical() && ptStart.x > ptAxisEnd.x) && 
			dblCurrValue > GetMaxDisplayedValue() + GetDoubleCorrection() || 
			IsDisplayDataBetweenTickMarks() && dblCurrValue >= GetMaxDisplayedValue())
		{
			break;
		}

		CString strLabel; 
		GetDisplayedLabel(dblCurrValue, strLabel);

		CBCGPSize szLabel = m_pChart->OnCalcAxisLabelSize(pGM, this, dblCurrValue, strLabel, m_axisLabelsFormat.m_textFormat);

		CBCGPSize szPadding = m_axisLabelsFormat.GetContentPadding(TRUE);
		CBCGPRect rectLabel;

		if (bIs3D)
		{
			rectLabel = OnCalcAxisLabelRect3D(ptStart, szLabel); 
		}
		else if (IsVertical())
		{
			rectLabel.SetRect(m_rectAxisLabels.left, 
							ptStart.y - szLabel.cy / 2, 
							m_rectAxisLabels.right, 
							ptStart.y + szLabel.cy / 2);
			
			rectLabel.InflateRect(0, szPadding.cy);
		}
		else
		{
			rectLabel.SetRect(ptStart.x - szLabel.cx / 2, 
				m_rectAxisLabels.top, 
				ptStart.x + szLabel.cx / 2, 
				m_rectAxisLabels.bottom);

			rectLabel.InflateRect(szPadding.cx, 0);
		}

		BOOL bDrawNextLabel = TRUE;

		if (bUseScaleBreaks || bIs3D && IsPreventLabelOverlapIn3D())
		{
			if (!rectPrevLabel.IsRectEmpty())
			{
				CBCGPRect rectCheck = rectPrevLabel;

				if (rectCheck.IntersectRect(rectCheck, rectLabel))
				{
					bDrawNextLabel = FALSE;
				}
			}
		}

		if (bDrawNextLabel && 
			(bIs3D || bInterlaceLabels && nCount % nStep == 0 || !bInterlaceLabels || 
			 m_nDisplayedLines <= m_nMinDisplayedLines && !IsFixedMajorUnit()))
		{
			if (IsVertical() && m_dblTopOffset != 0)
			{
				if (m_bReverseOrder)
				{
					if (rectLabel.top < ptStart.y)
					{
						rectLabel.top += ptStart.y - rectLabel.top;
						rectLabel.bottom += ptStart.y - rectLabel.top;
					}
				}
				else
				{
					if (rectLabel.top < ptEnd.y)
					{
						rectLabel.top += ptEnd.y - rectLabel.top;
						rectLabel.bottom += ptEnd.y - rectLabel.top;
					}
				}
			}

			m_pChart->OnDrawAxisLabel(pGM, dblCurrValue, strLabel, this, rectLabel, m_rectAxisLabels, rectDiagram);
			
			rectPrevLabel = rectLabel;
		}

		nCount++;

		if (IsLogScale())
		{
			dblUnitStep += 1.;
			dblCurrValue = pow(m_dblMajorUnit, dblUnitStep);
		}
		else
		{
			dblCurrValue += dblUnitsInInterval;

			if (bUseScaleBreaks && dblCurrValue > partCurrent.GetMaximumDisplayed())
			{
				if (posScalePart == NULL)
				{
					break;
				}
				
				partCurrent = m_scaleParts.GetNext(posScalePart);

				if (!partCurrent.m_bValid)
				{
					break;
				}
				
				dblUnitsInInterval = partCurrent.m_dblMajorUnit;
				dblCurrValue = partCurrent.GetMinimumDisplayed();
				dblMaxValue = partCurrent.GetMaximumDisplayed() + GetDoubleCorrection();
			}
		}
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnDrawScaleBreaks(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram)
{
	if (!CanUseScaleBreaks() || m_pChart == NULL)
	{
		return;
	}

	const CBCGPChartTheme& theme = m_pChart->GetColors();

	CBCGPRect rectBounds = GetBoundingRect();

	for (POSITION pos = m_scaleParts.GetHeadPosition(); pos != NULL;)
	{
		const CBCGPChartAxisScalePart& part = m_scaleParts.GetNext(pos);

		if (pos == NULL || !part.m_bValid)
		{
			break;
		}

		const CBCGPChartAxisScalePart& partNext = m_scaleParts.GetAt(pos);

		if (!partNext.m_bValid)
		{
			break;
		}

		CBCGPRect rectBreak = rectBounds;

		if (IsVertical())
		{
			rectBreak.top = partNext.m_dblScreenStart;
			rectBreak.bottom = part.m_dblScreenEnd;
		}
		else
		{
			rectBreak.left = part.m_dblScreenEnd;
			rectBreak.right = partNext.m_dblScreenStart;
		}

		rectBreak.Normalize();
		
		CBCGPBrush* pBrFill = (CBCGPBrush*)&part.m_scaleBreakFormat.m_brFillColor;
		CBCGPBrush* pBrLine = (CBCGPBrush*)&part.m_scaleBreakFormat.m_outlineFormat.m_brLineColor;

		BOOL bResoreFillOpacity = FALSE;
		BOOL bRestoreLineOpacity = FALSE;
		double dblFillOpacity = 0;
		double dblLineOpacity = 0;

		if (pBrFill->IsNull())
		{
			if (m_scaleBreakOptions.m_format.m_brFillColor.IsNull())
			{
				bResoreFillOpacity = TRUE;

				BCGPChartFormatArea& format = m_pChart->GetChartAreaFormat();

				if (!format.m_brFillColor.IsNull())
				{
					pBrFill = (CBCGPBrush*) &format.m_brFillColor;
				}
				else
				{
					pBrFill = (CBCGPBrush*) &theme.m_brChartFillColor;
				}
				
				dblFillOpacity = pBrFill->GetOpacity();
			}
			else
			{
				pBrFill = &m_scaleBreakOptions.m_format.m_brFillColor;
			}
		}

		if (pBrLine->IsNull())
		{
			if (m_scaleBreakOptions.m_format.m_outlineFormat.m_brLineColor.IsNull())
			{
				bRestoreLineOpacity = TRUE;

				pBrLine = (CBCGPBrush*)&theme.m_brAxisLineColor;
				dblLineOpacity = pBrLine->GetOpacity();
			}
			else
			{
				pBrLine = &m_scaleBreakOptions.m_format.m_outlineFormat.m_brLineColor;
			}
		}

		double dblLineWidth = part.m_scaleBreakFormat.m_outlineFormat.GetLineWidth(FALSE);

		if (dblLineWidth < 0.0)
		{
			dblLineWidth = m_scaleBreakOptions.m_format.m_outlineFormat.GetLineWidth(TRUE);
		}

		m_pChart->OnDrawAxisScaleBreak(pGM, rectBreak, part.m_scaleBreakStyle, *pBrFill, *pBrLine, dblLineWidth, 
			part.m_scaleBreakFormat.m_outlineFormat.m_strokeStyle.IsEmpty() ? 
			m_scaleBreakOptions.m_format.m_outlineFormat.m_strokeStyle : part.m_scaleBreakFormat.m_outlineFormat.m_strokeStyle,
			part.GetMaximumDisplayed(), partNext.GetMinimumDisplayed(), this, rectDiagram);

		if (bResoreFillOpacity)
		{
			pBrFill->SetOpacity(dblFillOpacity);
		}

		if (bRestoreLineOpacity)
		{
			pBrLine->SetOpacity(dblLineOpacity);
		}
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnDrawScrollBar(CBCGPGraphicsManager* pGM, const CBCGPRect& rectScrollBar)
{
	if (!CanShowScrollBar() || m_pChart == NULL)
	{
		return;
	}

	OnCalcThumbRect();

	const CBCGPBrush& brLineColor = m_axisLineFormat.m_brLineColor.IsEmpty() ? 
			m_pChart->GetColors().m_brAxisLineColor : m_axisLineFormat.m_brLineColor;

	pGM->FillRectangle(rectScrollBar, m_pChart->GetColors().m_brChartFillColor);
	pGM->DrawRectangle(rectScrollBar, brLineColor);

	CBCGPRect rectThumb = m_rectThumb;

	pGM->FillRectangle(rectThumb, IsVertical() ? m_pChart->GetColors().m_brScrollBarVert : m_pChart->GetColors().m_brScrollBarHorz);
	pGM->DrawRectangle(rectThumb, brLineColor);
	
}
//----------------------------------------------------------------------------//
CBCGPRect CBCGPChartAxis::GetBoundingRects(CArray<CBCGPRect, CBCGPRect>& arRects) const
{
	ASSERT_VALID(this);

	CBCGPRect rectAxisTotal;

	if (m_pChart == NULL)
	{
		return rectAxisTotal;
	}

	ASSERT_VALID(m_pChart);

	for (int i = 0; i < m_pChart->GetAxisCount(); i++)
	{
		CBCGPChartAxis* pAxis = m_pChart->GetChartAxis(i);

		if (pAxis == NULL || pAxis == this || pAxis->IsVertical() && IsVertical() || 
			!pAxis->IsVertical() && !IsVertical())
		{
			continue;
		}

		CBCGPChartAxis* pPerpAxis = pAxis->GetPerpendecularAxis();

		if (pPerpAxis == NULL)
		{
			continue;
		}

		if (pPerpAxis == this)
		{
			CBCGPRect r = pAxis->GetBoundingRect();
			r.Normalize();

			arRects.Add(r);
		}
	}

	if (arRects.GetSize() == 0)
	{
		CBCGPRect r = GetBoundingRect();
		r.Normalize();

		arRects.Add(r);
	}

	for (int j = 0; j < arRects.GetSize(); j++)
	{
		rectAxisTotal.UnionRect(arRects [j], rectAxisTotal);
	}

	return rectAxisTotal;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnCalcDefaultTextAlignment(double /*dblCurrValue*/, const CBCGPRect& /*rectLabel*/, const CBCGPRect& /*rectDiagram*/, 
											CBCGPTextFormat& /*textFormat*/)
{
	ASSERT_VALID(this);
	return;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::CalcMinMaxValues()
{
	ASSERT_VALID(this);

	double dblMin = 0.;
	double dblMax = 0.;

	BOOL bMinMaxSet = FALSE;
	int nSeriesCount = 0;

	CBCGPChartAxis* pAxis = GetPerpendecularAxis();

	if (m_pChart == NULL)
	{
		return FALSE;
	}

	int nNumSeries = m_pChart->GetVisibleSeriesCount();

	if (nNumSeries == 0 || pAxis == NULL)
	{
		return FALSE;
	}

	BOOL bDynamicMinMax = FALSE;
	m_dblMinDataPointDistance = DBL_MAX;

	if (IsIndexedSeries())
	{
		dblMin = 0.0;
		dblMax = m_pIndexedSeries->GetDataPointCount() - 1;
		if (!m_pIndexedSeries->IsHistoryMode())
		{
			// in history mode offset is updated separately
			dblMax += m_pIndexedSeries->GetOffsetFromNullZone();
		}
		m_dblMinDataPointDistance = 1.0;
		bMinMaxSet = TRUE;
		nSeriesCount = 1;
	}
	else
	{
		CBCGPChartAxis* pPerpAxis = GetPerpendecularAxis();
		if (pPerpAxis->IsIndexedSeries() && pPerpAxis->IsFixedIntervalWidth())
		{
			bDynamicMinMax = TRUE;
		}

		for (int i = 0 ; i < m_pChart->GetSeriesCount(); i++)
		{
			CBCGPChartSeries* pSeries = m_pChart->GetSeries(i);
			if (pSeries == NULL || !pSeries->m_bVisible)
			{
				continue;
			}

			if (pSeries->IsShownOnAxis(this))
			{
				if (bDynamicMinMax)
				{
					pSeries->RecalcMinMaxValues();
				}

				CBCGPChartValue valCurrMin = pSeries->GetMinValue(GetComponentIndex());
				CBCGPChartValue valCurrMax = pSeries->GetMaxValue(GetComponentIndex());

				nSeriesCount++;

				if (valCurrMin.IsEmpty() || valCurrMax.IsEmpty())
				{
					continue;
				}

				if (!bMinMaxSet)
				{
					dblMin = valCurrMin;
					dblMax = valCurrMax;
					bMinMaxSet = TRUE;
				}
				else
				{
					dblMin = min(dblMin, valCurrMin);
					dblMax = max(dblMax, valCurrMax);
				}

				m_dblMinDataPointDistance = min(m_dblMinDataPointDistance, pSeries->GetMinDataPointDistance());
			}
		}
	}

	if (nSeriesCount > 0)
	{
		m_bEmptySeries = FALSE;
	}

	if (m_pChart->OnGetMinMaxValues(this, dblMin, dblMax))
	{
		m_dblMinimum = dblMin;
		m_dblMaximum = dblMax;
		bMinMaxSet = TRUE;
		return TRUE;
	}

	if (bMinMaxSet)
	{
		SetMinimumValue(dblMin);
		SetMaximumValue(dblMax);
	}
	else if (nSeriesCount == 0)
	{
		CBCGPChartAxis* pAxis = GetOppositeAxis();
		if (pAxis != NULL)
		{
			ASSERT_VALID(pAxis);

			m_dblMinimum = pAxis->m_dblMinimum;
			m_dblMaximum = pAxis->m_dblMaximum;
			m_dblMinDisplayedValue = pAxis->GetMinDisplayedValue();
			m_dblMaxDisplayedValue = pAxis->GetMaxDisplayedValue();
			m_dblMajorUnit = pAxis->m_dblMajorUnit;
			m_dblMinorUnit = pAxis->m_dblMinorUnit;
			m_dblMinAllowedMajorUnit = pAxis->GetMinAllowedMajorUnit();
			m_bNewMajorUnitBehavior = pAxis->m_bNewMajorUnitBehavior;
			m_nMaxTotalLines = pAxis->m_nMaxTotalLines;
			m_nMajorUnitSpacing = pAxis->GetMajorUnitSpacing();
			m_nDisplayedLines = pAxis->GetDisplayedLinesCount();
			m_bDisplayDataBetweenTickMarks = pAxis->IsDisplayDataBetweenTickMarks();
			m_bFixedRange = pAxis->IsFixedDisplayRange();
			m_bFixedMinimum = pAxis->IsFixedMinimumDisplayValue();
			m_bFixedMaximum = pAxis->IsFixedMaximumDisplayValue();
			m_bUseApproximation = pAxis->m_bUseApproximation;
			m_bFormatAsDate = pAxis->m_bFormatAsDate;
			m_strDataFormat = pAxis->m_strDataFormat;
			m_bEmptySeries = TRUE;
		}
	}

	return bMinMaxSet;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::SetAxisOffsets(double dblBottomOffset, double dblTopOffset, int nGapInPixels)
{
	m_dblBottomOffset = dblBottomOffset;
	m_dblTopOffset = dblTopOffset;
	m_nAxisGap = nGapInPixels;
}
//----------------------------------------------------------------------------//
CBCGPChartAxis* CBCGPChartAxis::Split(double dblPercent, int nGapInPixels, CRuntimeClass* pNewAxisRTC,
									  BOOL bCustomAxisAtTop)
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(m_pChart);

	dblPercent = bcg_clamp(dblPercent, 1., 99.);
	m_nAxisGap = nGapInPixels;

	int nNextCustomID = m_pChart->GetNextCustomAxisID();

	CBCGPChartAxis* pAxis = pNewAxisRTC == NULL ? 
		DYNAMIC_DOWNCAST(CBCGPChartAxis, GetRuntimeClass()->CreateObject()) :
		DYNAMIC_DOWNCAST(CBCGPChartAxis, pNewAxisRTC->CreateObject());

	if (pAxis == NULL)
	{
		TRACE0("CBCGPChartAxis::Split. Dynamic Axis creation failed.");
		return NULL;
	}

	pAxis->EnableZoom(FALSE);

	pAxis->m_pChart = m_pChart;
	pAxis->SetAxisDefaultPosition(m_axisDefaultPosition);
	pAxis->m_nAxisID = nNextCustomID;

	pAxis->CommonInit();
	m_pChart->AddAxis(pAxis);


	if (bCustomAxisAtTop)
	{
		pAxis->m_dblTopOffset = m_dblTopOffset;
		pAxis->m_nAxisGap = nGapInPixels;
		
		double dblCurrSize = 100. - (m_dblTopOffset + m_dblBottomOffset);
		double dblNewSize = dblCurrSize * (100. - dblPercent) / 100.;
		m_dblTopOffset = 100. - dblNewSize - m_dblBottomOffset;
		
		pAxis->m_dblBottomOffset = m_dblBottomOffset + dblNewSize;

		if (m_pSplitTop != NULL)
		{
			m_pSplitTop->m_pSplitBottom = pAxis;
			pAxis->m_pSplitTop = m_pSplitTop;
		}

		m_pSplitTop = pAxis;
		pAxis->m_pSplitBottom = this;
	}
	else
	{
		double dblCurrSize = 100. - (m_dblTopOffset + m_dblBottomOffset);
		double dblNewSize = dblCurrSize * (100. - dblPercent) / 100.;

		m_dblBottomOffset = m_dblBottomOffset + dblNewSize;
		pAxis->m_dblTopOffset = 100 - m_dblBottomOffset;
 		pAxis->m_nAxisGap = nGapInPixels;

		if (m_pSplitBottom != NULL)
		{
			m_pSplitBottom->m_pSplitTop = pAxis;
			pAxis->m_pSplitBottom = m_pSplitBottom;
		}

		m_pSplitBottom = pAxis;
		pAxis->m_pSplitTop = this;
	}
	

	return pAxis;
}
//----------------------------------------------------------------------------//
double CBCGPChartAxis::GetAxisGapScaled() const
{
	if (m_pChart == NULL)
	{
		return m_nAxisGap;
	}

	return IsVertical() ? m_pChart->GetScaleRatio().cy * m_nAxisGap : 
							m_pChart->GetScaleRatio().cx * m_nAxisGap;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::RemoveCustomAxis()
{
	BOOL bHasTop = FALSE;
	if (m_pSplitTop != NULL)
	{
		m_pSplitTop->m_dblBottomOffset = m_dblBottomOffset;
		m_pSplitTop->m_pSplitBottom = m_pSplitBottom;
		bHasTop = TRUE;
	}

	if (m_pSplitBottom != NULL)
	{
		if (!bHasTop)
		{
			m_pSplitBottom->m_dblTopOffset = m_dblTopOffset;
		}
		
		m_pSplitBottom->m_pSplitTop = m_pSplitTop;
	}

	m_pSplitTop = NULL;
	m_pSplitBottom = NULL;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::UpdateAxisSizeByOffset(const CBCGPPoint& pt, BOOL bTop, const CBCGPRect& rectDiagram)
{
	double dblDistance = 0;
	

	if (pt.x == 0 && !IsVertical() || pt.y == 0 && IsVertical())
	{
		return FALSE;
	}

	double dblDiagramSize = IsVertical() ? rectDiagram.Height() : rectDiagram.Width();
	CBCGPRect rectAxis = GetAxisRect(FALSE, TRUE);
	rectAxis.Normalize();

	int nTopGap = 0; 
	
	if (m_dblTopOffset != 0 && m_dblBottomOffset != 0 || 
		m_dblTopOffset != 0 && m_dblBottomOffset == 0)
	{
		nTopGap = (int)GetAxisGapScaled();
	}

	if (bTop)
	{
		if (IsVertical())
		{
			rectAxis.top += pt.y - nTopGap;
			dblDistance = rectAxis.top - rectDiagram.top;
		}
		else
		{
			rectAxis.right += pt.x + nTopGap;
			dblDistance = rectDiagram.right - rectAxis.right;
		}
	}
	else
	{
		if (IsVertical())
		{
			rectAxis.bottom += pt.y;
			dblDistance = rectDiagram.bottom - rectAxis.bottom;
		}
		else
		{
			rectAxis.left += pt.x;
			dblDistance = rectAxis.left  - rectDiagram.left;
		}
	}

	double dblPercent = (dblDistance * 100. / dblDiagramSize);
	
	bTop ? m_dblTopOffset = dblPercent : m_dblBottomOffset = dblPercent;

	return TRUE;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::UpdateSplitAxisSizeByPercent(double dblPercent, BOOL bTop)
{
	CBCGPChartAxis* pNextAxis = bTop ? GetSplitTop() : GetSplitBottom();

	if (pNextAxis == NULL || m_pChart == NULL)
	{
		return FALSE;
	}

	if (bTop)
	{
		m_dblTopOffset = bcg_clamp(m_dblTopOffset + dblPercent, 0., 100.);
		pNextAxis->m_dblBottomOffset -= dblPercent;
	}
	else
	{
		m_dblBottomOffset = bcg_clamp(m_dblBottomOffset + dblPercent, 0., 100.);
		pNextAxis->m_dblTopOffset -= dblPercent;
	}

	return TRUE;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::CanUpdateAxisSizeByOffset(const CBCGPPoint& ptOffset, BOOL bTop) const
{
	double dblAxisSize = fabs(GetAxisSize());

	if (bTop)
	{
		return IsVertical() && (dblAxisSize - ptOffset.y > 10.) || 
			!IsVertical() && (dblAxisSize + ptOffset.x > 10.);
	}

	return IsVertical() && (dblAxisSize + ptOffset.y > 10.) || 
			!IsVertical() && (dblAxisSize - ptOffset.x > 10.);
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxis::IsCustomOrResized() const 
{
	return m_nAxisID >= BCGP_CHART_FIRST_CUSTOM_ID || m_dblTopOffset != 0. || m_dblBottomOffset != 0;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::EnableLabelInterlacing(BOOL bEnable)
{
	ASSERT_VALID(this);

	m_bEnableInterlaceLabels = bEnable;

	if (m_pChart != NULL)
	{
		ASSERT_VALID(m_pChart);
		m_pChart->SetDirty(TRUE);
	}
}
//----------------------------------------------------------------------------//
// 3D interface
//----------------------------------------------------------------------------//
void CBCGPChartAxis::CalcAxisPos3D(const CBCGPRect& /*rectDiagram*/, BOOL bUpdateCrossValue)
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return;
	}

	CBCGPChartDiagram3D* pDiagram3D = m_pChart->GetDiagram3D();

	if (!IsDiagram3D() || pDiagram3D == NULL)
	{
		return;
	}

	if (bUpdateCrossValue)
	{
		double dblCrossValue = GetCrossValuePos(TRUE);

		if (IsVertical())
		{
			m_ptAxisStart3D.x = dblCrossValue;
			m_ptAxisEnd3D.x = dblCrossValue;
		}
		else
		{
			m_ptAxisStart3D.y = dblCrossValue;
			m_ptAxisEnd3D.y = dblCrossValue;
		}
	}
	else
	{
		pDiagram3D->GetNormalAxisCoordinates(m_nAxisID, m_ptAxisStart3D, m_ptAxisEnd3D);
	}

	CBCGPPoint ptCenter = pDiagram3D->GetDiagramRect().CenterPoint();

	m_ptAxisStart = pDiagram3D->TransformPoint(m_ptAxisStart3D, ptCenter);
	m_ptAxisEnd = pDiagram3D->TransformPoint(m_ptAxisEnd3D, ptCenter);

	m_dblAxisSize3D = bcg_distance(m_ptAxisStart, m_ptAxisEnd);
}
//----------------------------------------------------------------------------//
CBCGPRect CBCGPChartAxis::OnCalcAxisLabelRect3D(const CBCGPPoint& ptValue, const CBCGPSize& szLabel)
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return CBCGPRect();
	}

	CBCGPChartDiagram3D* pDiagram3D = m_pChart->GetDiagram3D();

	if (pDiagram3D == NULL)
	{
		return CBCGPRect();
	}

	CBCGPRect rectLabel;

	CBCGPSize szPadding = m_axisLabelsFormat.GetContentPadding(TRUE);
	double dblDistance = GetLabelDistance();
	double dblXRotation = pDiagram3D->GetXRotation();
	CBCGPPoint ptDistance;

	if (IsVertical())
	{
		if (dblXRotation >= 90 && dblXRotation < 270)
		{
			rectLabel.SetRect(ptValue.x, ptValue.y - szLabel.cy / 2, 
				ptValue.x + szLabel.cx, ptValue.y + szLabel.cy / 2); 
			ptDistance.x = dblDistance;
		}
		else
		{
			rectLabel.SetRect(ptValue.x - szLabel.cx, ptValue.y - szLabel.cy / 2, 
				ptValue.x, ptValue.y + szLabel.cy / 2); 
			ptDistance.x = -dblDistance;
		}
		
		rectLabel.OffsetRect(ptDistance);
		rectLabel.InflateRect(0, szPadding.cy);
	}
	else
	{
		double dblCatetX = m_ptAxisStart.x - m_ptAxisEnd.x;
		double dblCatetY = m_ptAxisStart.y - m_ptAxisEnd.y;

		double dblAngle = dblCatetX == 0 ? 90 : bcg_rad2deg(atan((dblCatetY / dblCatetX)));

		if (dblCatetX == 0 && dblCatetY < 0)
		{
			dblAngle = -90;
		}

		CBCGPPoint ptDiagramCenter = pDiagram3D->GetDiagramRect().CenterPoint() + pDiagram3D->GetScrollOffset();

		BOOL bIsAxisOnLeftSide = m_ptAxisStart.x < ptDiagramCenter.x && m_ptAxisEnd.x < ptDiagramCenter.x;
		BOOL bIsAxisOnRightSide = m_ptAxisStart.x > ptDiagramCenter.x && m_ptAxisEnd.x > ptDiagramCenter.x;

		if (dblAngle >= 55 && dblAngle <= 90 && !bIsAxisOnRightSide || bIsAxisOnLeftSide)
		{
			rectLabel.SetRect(ptValue.x - szLabel.cx, ptValue.y - szLabel.cy / 2, 
				ptValue.x, ptValue.y + szLabel.cy / 2); 
			ptDistance.x = -dblDistance;
			ptDistance.y = dblDistance * cos(bcg_deg2rad(dblAngle));
		}
		else if (dblAngle < -55 && dblAngle >= -90 && !bIsAxisOnLeftSide || bIsAxisOnRightSide)
		{
			rectLabel.SetRect(ptValue.x, ptValue.y - szLabel.cy / 2, 
				ptValue.x + szLabel.cx, ptValue.y + szLabel.cy / 2); 
			ptDistance.x = dblDistance;
			ptDistance.y = dblDistance * cos(fabs(bcg_deg2rad(dblAngle)));
		}
		else
		{
			rectLabel.SetRect(ptValue.x - szLabel.cx / 2, ptValue.y, 
					ptValue.x + szLabel.cx / 2, ptValue.y + szLabel.cy); 
			ptDistance.y = dblDistance;
		}

		rectLabel.OffsetRect(ptDistance);
		rectLabel.InflateRect(szPadding.cx, 0);
	}
	
	return rectLabel;
}
//----------------------------------------------------------------------------//
CBCGPPoint CBCGPChartAxis::GetPointOnAxisPlane3D(double dblValue, BOOL bValueOnWall) const
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return CBCGPPoint();
	}

	CBCGPChartDiagram3D* pDiagram3D = m_pChart->GetDiagram3D();

	if (pDiagram3D == NULL)
	{
		return CBCGPPoint();
	}

	CBCGPChartData::ComponentIndex ci = GetComponentIndex();

	if (bValueOnWall)
	{
		CBCGPPoint ptPlane = pDiagram3D->GetAxisPoint(m_nAxisID, CBCGPChartDiagram3D::APT_PLANE);

		switch (ci)
		{
		case CBCGPChartData::CI_X:
		case CBCGPChartData::CI_Y:
			if (IsVertical())
			{
				ptPlane.y = dblValue;
			}
			else
			{
				ptPlane.x = dblValue;
			}
			return ptPlane;

		case CBCGPChartData::CI_Z:
			ptPlane.z = dblValue;
			return ptPlane;
		}
	}
 	else
	{
		switch (ci)
		{
		case CBCGPChartData::CI_X:
		case CBCGPChartData::CI_Y:
			if (IsVertical())
			{
				return CBCGPPoint (m_ptAxisStart3D.x, dblValue, m_ptAxisStart3D.z);
			}

			return CBCGPPoint (dblValue, m_ptAxisStart3D.y, m_ptAxisStart3D.z);

		case CBCGPChartData::CI_Z:
			return CBCGPPoint (m_ptAxisStart3D.x, m_ptAxisStart3D.y, dblValue);
		}
	}

	return CBCGPPoint();
	
}
//----------------------------------------------------------------------------//
void CBCGPChartAxis::OnCalcFloorAndWallGridLinePoints(double dblCurrValue, CBCGPPoint& ptFloor, CBCGPPoint& ptWall)
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return;
	}

	CBCGPChartDiagram3D* pDiagram3D = m_pChart->GetDiagram3D();

	if (!IsDiagram3D() || pDiagram3D == NULL)
	{
		return;
	}

	double dblNormalValue = PointFromValue(dblCurrValue, TRUE);

	CBCGPPoint ptFloor3D = pDiagram3D->GetAxisPoint(m_nAxisID, CBCGPChartDiagram3D::APT_FLOOR);
	CBCGPPoint ptWall3D = pDiagram3D->GetAxisPoint(m_nAxisID, CBCGPChartDiagram3D::APT_WALL);

	CBCGPChartData::ComponentIndex ci = GetComponentIndex();

	switch (ci)
	{
	case CBCGPChartData::CI_X:
	case CBCGPChartData::CI_Y:
		if (IsVertical())
		{
			ptFloor3D.y = dblNormalValue;
			ptWall3D.y = dblNormalValue;
		}
		else
		{
			ptFloor3D.x = dblNormalValue;
			ptWall3D.x = dblNormalValue;
		}
		break;

	case CBCGPChartData::CI_Z:
		ptFloor3D.z = dblNormalValue;
		ptWall3D.z = dblNormalValue;
		break;
	}

	CBCGPPoint ptCenter = pDiagram3D->GetDiagramRect().CenterPoint();

	ptFloor = pDiagram3D->TransformPoint(ptFloor3D, ptCenter);
	ptWall = pDiagram3D->TransformPoint(ptWall3D, ptCenter);
}
//----------------------------------------------------------------------------//
// Y-axis implementation
//----------------------------------------------------------------------------//
CBCGPChartAxisY::CBCGPChartAxisY()
{
	
}

CBCGPChartAxisY::CBCGPChartAxisY(int nAxisID, CBCGPChartAxis::AxisDefaultPosition position, CBCGPChartVisualObject* pChartCtrl) : 
	  CBCGPChartAxis(nAxisID, position, pChartCtrl)
{
	CommonInit();
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisY::CommonInit()
{
	CBCGPChartAxis::CommonInit();
	m_nMinorUnitCount = 5;
	m_nMaxTotalLines = 9;
}
//----------------------------------------------------------------------------//
CBCGPChartAxis* CBCGPChartAxisY::GetOppositeAxis()
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(m_pChart);

	if (m_axisDefaultPosition == CBCGPChartAxis::ADP_LEFT)
	{
		return m_pChart->GetChartAxis(BCGP_CHART_Y_SECONDARY_AXIS);
	}

	return m_pChart->GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS);
}
//----------------------------------------------------------------------------//
CBCGPChartAxis* CBCGPChartAxisY::GetPerpendecularAxis() const
{
	ASSERT_VALID(this);

	if ( m_pPerpendicularAxis != NULL)
	{
		return m_pPerpendicularAxis;
	}

	if (m_pChart == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(m_pChart);

	if (m_axisDefaultPosition == CBCGPChartAxis::ADP_RIGHT)
	{
		return m_pChart->GetChartAxis(BCGP_CHART_X_SECONDARY_AXIS);
	}

	return m_pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisY::GetPerpendecularAxes(CBCGPChartAxis*& pPrimary, CBCGPChartAxis*& pSecondary)
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return;
	}

	ASSERT_VALID(m_pChart);

	pPrimary = m_pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS);
	pSecondary = m_pChart->GetChartAxis(BCGP_CHART_X_SECONDARY_AXIS);
}
//----------------------------------------------------------------------------//
int CBCGPChartAxisY::GetNumberOfUnitsToDisplay(UINT nUnitsToDisplay)
{
	if (!m_bFixedMajorUnit || m_dblMajorUnit == 0)
	{
		return CBCGPChartAxis::GetNumberOfUnitsToDisplay(nUnitsToDisplay);
	}
	
	return (int)(fabs(m_dblMaximum - m_dblMinimum) / m_dblMajorUnit) + 2;
}
//----------------------------------------------------------------------------//
int CBCGPChartAxisY::GetMaxTotalLines() const
{
	if (m_nMajorUnitSpacing != 0)
	{
		return (int)(GetAxisSize(FALSE) / m_nMajorUnitSpacing + 2.);
	}

	if (m_dblMinAllowedMajorUnit == 0)
	{
		return m_nMaxTotalLines;
	}

	return (int)((m_dblMaximum - m_dblMinimum) / m_dblMinAllowedMajorUnit + 2.);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisY::GetDisplayedLabel(double dblValue, CString& strLabel)
{
	ASSERT_VALID(this);

	strLabel.Empty();

	if (m_pChart != NULL && m_pChart->OnGetDisplayedLabel(this, dblValue, strLabel))
	{
		return;
	}

	CBCGPChartAxis::GetDisplayedLabel(dblValue, strLabel);
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxisY::CalcMinMaxValues()
{
	BOOL bHasFullStackedSeries = FALSE;

	for (int i = 0; i < m_pChart->GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = m_pChart->GetSeries(i);

		if (pSeries != NULL && pSeries->m_bVisible && pSeries->GetChartType() == BCGP_CT_100STACKED)
		{
			bHasFullStackedSeries = TRUE;
			break;
		}
	}
	
	if (!bHasFullStackedSeries)
	{
		return CBCGPChartAxis::CalcMinMaxValues();
	}

	SetAutoDisplayRange();

	BOOL bMinMaxSet = CBCGPChartAxis::CalcMinMaxValues();

	if (bMinMaxSet)
	{
		if (m_dblMaximum > 80.)
		{
			SetFixedMaximumDisplayValue(100., FALSE);
		}

		if (m_dblMinimum < -80.)
		{
			SetFixedMinimumDisplayValue(-100., FALSE);
		}
		
		if (m_dblMinimum == 0)
		{
			SetFixedMinimumDisplayValue(0., FALSE);
		}
	}

	return bMinMaxSet;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisY::CalcMinMaxDisplayedValues(BOOL bCalcMinDisplayedValue, double dblApproximation)
{
	CBCGPChartAxis::CalcMinMaxDisplayedValues(bCalcMinDisplayedValue, dblApproximation);

	if (IsFixedDisplayRange())
	{
		return;
	}

// 	for (int i = 0; i < m_pChart->GetSeriesCount(); i++)
// 	{
// 		CBCGPChartSeries* pSeries = m_pChart->GetSeries (i, FALSE);
// 
// 		if (pSeries != NULL && pSeries->m_bVisible && 
// 			(pSeries->GetChartCategory() == BCGPChartLine || 
// 			 pSeries->GetChartCategory() == BCGPChartArea) &&
// 			(pSeries->GetCurveType() == BCGPChartFormatSeries::CCT_SPLINE || 
// 			 pSeries->GetCurveType() == BCGPChartFormatSeries::CCT_SPLINE_HERMITE))
// 		{
// 			if (!IsFixedMinimumDisplayValue() && fabs(m_dblMinimum - m_dblMinDisplayedValue) < m_dblMajorUnit)
// 			{
// 				m_dblMinDisplayedValue -= m_dblMajorUnit;
// 			}
// 
// 			if (!IsFixedMaximumDisplayValue() && fabs(m_dblMaximum - m_dblMaxDisplayedValue) < m_dblMajorUnit)
// 			{
// 				m_dblMaxDisplayedValue += m_dblMajorUnit;
// 			}
// 		}
// 	}
}
//----------------------------------------------------------------------------//
// X-axis implementation
//----------------------------------------------------------------------------//
CBCGPChartAxisX::CBCGPChartAxisX()
{

}

CBCGPChartAxisX::CBCGPChartAxisX (int nAxisID, CBCGPChartAxis::AxisDefaultPosition position, CBCGPChartVisualObject* pChartCtrl) : 
	CBCGPChartAxis(nAxisID, position, pChartCtrl)
{
	CommonInit();
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisX::CommonInit()
{
	CBCGPChartAxis::CommonInit();

	m_nMinorUnitCount = 2;
	m_nMaxTotalLines = 1;
	m_bFixedRange = FALSE;
	m_nMaxDataPointCount = 0;
	m_bDisplayDataBetweenTickMarks = TRUE;
	m_bIsAxisComponentSet = FALSE; 
	m_bOnlyFormula = FALSE;
	m_bAutoMajorUnit = FALSE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisX::Reset()
{
	ASSERT_VALID(this);

	CBCGPChartAxis::Reset();
	m_bFixedRange = FALSE;
	m_arCategories.RemoveAll();
	m_nMaxTotalLines = 1;
	m_nMaxDataPointCount = 0;
	m_bIsAxisComponentSet = FALSE;
	m_bOnlyFormula = FALSE;
	m_bDisplayDataBetweenTickMarks = TRUE;
	m_bAutoMajorUnit = FALSE;
}
//----------------------------------------------------------------------------//
CBCGPChartAxis* CBCGPChartAxisX::GetOppositeAxis()
{
	ASSERT_VALID(this);

	if (m_axisDefaultPosition == CBCGPChartAxis::ADP_BOTTOM)
	{
		return m_pChart->GetChartAxis(BCGP_CHART_X_SECONDARY_AXIS);
	}

	return m_pChart->GetChartAxis(BCGP_CHART_X_PRIMARY_AXIS);
}
//----------------------------------------------------------------------------//
CBCGPChartAxis* CBCGPChartAxisX::GetPerpendecularAxis() const
{
	ASSERT_VALID(this);

	if (m_pPerpendicularAxis != NULL)
	{
		return m_pPerpendicularAxis;
	}

	if (m_axisDefaultPosition == CBCGPChartAxis::ADP_TOP)
	{
		return m_pChart->GetChartAxis(BCGP_CHART_Y_SECONDARY_AXIS);
	}

	return m_pChart->GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisX::GetPerpendecularAxes(CBCGPChartAxis*& pPrimary, CBCGPChartAxis*& pSecondary)
{
	ASSERT_VALID(this);

	pPrimary = m_pChart->GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS);
	pSecondary = m_pChart->GetChartAxis(BCGP_CHART_Y_SECONDARY_AXIS);
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxisX::Scroll(double dblUnitsToScroll, BOOL bUnitsInPixels)
{
	return CBCGPChartAxis::Scroll(dblUnitsToScroll, bUnitsInPixels);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisX::CheckAxisComponent()
{
	m_bIsAxisComponentSet = FALSE;

	if (IsIndexedSeries())
	{
		return;
	}

	m_bOnlyFormula = TRUE;

	for (int i = 0 ; i < m_pChart->GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = m_pChart->GetSeries(i);
		if (pSeries == NULL || !pSeries->m_bVisible || !pSeries->IsShownOnAxis(this))
		{
			continue;
		}

		if (!pSeries->IsFormulaSeries() && !pSeries->IsVirtualMode())
		{
			m_bOnlyFormula = FALSE;
		}

		if (pSeries->IsComponentSet(GetComponentIndex()) && !pSeries->IsFormulaSeries() && !pSeries->IsVirtualMode())
		{
			m_bIsAxisComponentSet = TRUE;
			break;
		}
	}
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxisX::CalcMinMaxValues()
{
	ASSERT_VALID(this);

	CheckAxisComponent();

	if (!IsFixedDisplayRange())
	{
		if (!IsComponentXSet())
		{
			SetMinimumValue(1);
			SetMaximumValue(1);
		}
		else
		{
			SetMinimumValue(0);
			SetMaximumValue(0);
		}
		
	}

	BOOL bSet = CBCGPChartAxis::CalcMinMaxValues();

	// CalcMinMaxValues values can take the minimum from a formula and override the 1.
	if (bSet && !IsFixedDisplayRange() && !IsComponentXSet() && !IsFixedMinimumDisplayValue())
	{
		m_dblMinimum = min(m_dblMinimum, 1.);
	}

	m_nMaxDataPointCount = 0;
	BOOL bSeriesCount = 0;

	for (int i = 0 ; i < m_pChart->GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = m_pChart->GetSeries(i);
		if (pSeries == NULL || !pSeries->m_bVisible || !pSeries->IsShownOnAxis(this))
		{
			continue;
		}

		bSeriesCount++;

		if (pSeries->IsVirtualMode())
		{
			continue;
		}

		if (pSeries->IsIndexMode())
		{
			m_nMaxTotalLines = 0xFFFE;
			break;
		}

		m_nMaxDataPointCount = max(m_nMaxDataPointCount, pSeries->GetDataPointCount());

		if (!IsFixedDisplayRange() && !m_bIsAxisComponentSet)
		{
			SetMaximumValue(max(m_dblMaximum, pSeries->GetDataPointCount()));				
			m_nMaxTotalLines = max(m_nMaxTotalLines, (UINT)m_nMaxDataPointCount);
		}
		else if (m_nMaxTotalLines <= 2)
		{
			m_nMaxTotalLines = 0xFFFE;
		}
	}

	if (m_bOnlyFormula && m_nMaxTotalLines <= 2)
	{
		m_nMaxTotalLines = 0xFFFE;
	}

	if (bSeriesCount == 0)
	{
		CBCGPChartAxisX* pOpposAxis = DYNAMIC_DOWNCAST(CBCGPChartAxisX, GetOppositeAxis());

		if (pOpposAxis != NULL)
		{
			m_bIsAxisComponentSet = pOpposAxis->IsComponentXSet();
		}
		
		return FALSE;
	}

	if (m_bIsAxisComponentSet && m_nMaxTotalLines  <= 2)
	{
		m_nMaxTotalLines = 0xFFFE;
	}

	if (IsDisplayDataBetweenTickMarks())
	{
		m_nMaxTotalLines++;
		SetMaximumValue(max(m_dblMaximum, m_nMaxDataPointCount + 1));
		
	}
	else if (!IsComponentXSet())
	{
		SetMaximumValue(max(m_dblMaximum, m_nMaxDataPointCount));
	}

	return TRUE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisX::OnBeforeSetFixedDisplayRange(double& /*dblMin*/, double& /*dblMax*/)
{

}
//----------------------------------------------------------------------------//
int CBCGPChartAxisX::GetNumberOfUnitsToDisplay(UINT nUnitsToDisplay)
{
	ASSERT_VALID(this);

	if (m_bIsFxedUnitCount)
	{
		return m_nFixedUnitCount;
	}

	if (m_bAutoMajorUnit && !m_bIsFixedIntervalWidth && IsAutoMajorUnitEnabled())
	{
		return CBCGPChartAxis::GetNumberOfUnitsToDisplay(nUnitsToDisplay);
	}

	int nUnits = CBCGPChartAxis::GetNumberOfUnitsToDisplay(nUnitsToDisplay);

	if (IsFixedDisplayRange() && m_dblMinDataPointDistance > 0 && m_bZoomed && m_bFormatAsDate &&
		nUnits < m_nMaxDataPointCount)
	{
		double dblRange = m_dblMaxDisplayedValue - m_dblMinDisplayedValue;
		return (int)(dblRange / m_dblMinDataPointDistance);
	}
	
	if ((nUnits < m_nMaxDataPointCount && m_bFormatAsDate || !m_bIsAxisComponentSet) && !m_bEmptySeries && !m_bFixedMajorUnit && !m_bZoomed && !m_bOnlyFormula)
	{
		nUnits = m_nMaxDataPointCount;
		if (IsDisplayDataBetweenTickMarks())
		{
			nUnits++;
		}
	}
	return nUnits;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisX::CalcMajorMinorUnits(CBCGPGraphicsManager* pGM, UINT nUnitsToDisplay)
{
	ASSERT_VALID(this);

	if (m_bFormatAsDate && !m_bFixedMajorUnit && !IsIndexedSeries())
	{
		CBCGPChartAxis::CalcMajorMinorUnits(pGM, nUnitsToDisplay);

		if (m_dblMinAllowedMajorUnit != 0)
		{
			return;
		}

		if (m_dblMinDataPointDistance != DBL_MAX)
		{
 			if (m_dblMinDataPointDistance != 0)
 			{
 				m_dblMajorUnit = m_dblMinDataPointDistance;
 			}
			
			m_nDisplayedLines = GetNumberOfUnitsToDisplay(nUnitsToDisplay);
			m_dblMinorUnit = m_dblMajorUnit / m_nMinorUnitCount;
		}
		
		if (!IsFixedMinimumDisplayValue())
		{
			m_dblMinDisplayedValue = m_dblMinimum - m_dblMajorUnit;
		}

		if (!IsFixedMaximumDisplayValue())
		{
			m_dblMaxDisplayedValue = m_dblMaximum + m_dblMajorUnit;
		}

		SetScrollRange(GetMinDisplayedValue(), GetMaxDisplayedValue(), FALSE);
		
		return;
	}
	

	if ((m_bIsAxisComponentSet || m_bOnlyFormula || m_bFormatAsDate || m_bFixedMajorUnit) && !IsIndexedSeries() ||
		IsFixedIntervalWidth())
	{
		CBCGPChartAxis::CalcMajorMinorUnits(pGM, nUnitsToDisplay);
		return;
	}

	CalcMaxLabelSize(pGM);
	m_dblMajorUnit = 1.;
	m_dblMinorUnit = m_dblMajorUnit / m_nMinorUnitCount;

	CBCGPChartAxis* pAxis = GetPerpendecularAxis();

	m_dblMinDisplayedValue = m_dblMinimum;
	m_dblMaxDisplayedValue = m_dblMaximum;
	
	double dblRange = m_dblMaximum - m_dblMinimum;
	m_nDisplayedLines = (int)dblRange;

	CBCGPRect rectBounds = GetBoundingRect();

	if (!rectBounds.IsRectEmpty() && !IsFixedIntervalWidth() && !IsDiagram3D() && IsAutoMajorUnitEnabled())
	{
		int nAllowedUnits = (int)(IsVertical() ? rectBounds.Height() / 2 : rectBounds.Width() / 2);
		m_bAutoMajorUnit = nAllowedUnits < dblRange;

		if (m_bAutoMajorUnit)
		{
			CBCGPChartAxis::CalcMajorMinorUnits(pGM, nUnitsToDisplay);
			return;
		}
	}

	if (pAxis->m_crossType == CBCGPChartAxis::CT_AXIS_VALUE && !IsDiagram3D())
	{
		m_dblMinDisplayedValue = min(pAxis->m_dblCrossOtherAxisValue, m_dblMinDisplayedValue);
		m_dblMaxDisplayedValue = max(pAxis->m_dblCrossOtherAxisValue, m_dblMaxDisplayedValue);
		m_nDisplayedLines = (int)(m_dblMaxDisplayedValue - m_dblMinDisplayedValue);
	}	

	SetScrollRange(GetMinDisplayedValue(), GetMaxDisplayedValue(), FALSE);

	m_bInitialized = TRUE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisX::GetDisplayedLabel(double dblValue, CString& strLabel)
{
	ASSERT_VALID(this);

	strLabel.Empty();

	if (m_pChart != NULL && m_pChart->OnGetDisplayedLabel(this, dblValue, strLabel))
	{
		return;
	}

	if (m_arCategories.GetSize() > 0 && !m_bIsAxisComponentSet)
	{
		int nCategoryIndex = bcg_round(dblValue) - 1;
		if (nCategoryIndex < m_arCategories.GetSize() && nCategoryIndex >= 0)
		{
			strLabel = m_arCategories[nCategoryIndex];
			if (strLabel.IsEmpty())
			{
				CBCGPChartAxis::GetDisplayedLabel(nCategoryIndex + 1, strLabel);
			}
			return;
		}
	}

	if (!m_bIsAxisComponentSet)
	{
		dblValue = bcg_round(dblValue);
	}

	CBCGPChartAxis::GetDisplayedLabel(dblValue, strLabel);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisX::CalcMaxLabelSize(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pGM);

	CBCGPSize sizeText(0,0);
	m_szMaxLabelSize.SetSize(0, 0);
	
	if (GetAxisLabelType() == CBCGPChartAxis::ALT_NO_LABELS)
	{
		return;
	}

	if (m_arCategories.GetSize() > 0 && !m_bIsAxisComponentSet)
	{
		for (int i = 0; i < m_arCategories.GetSize(); i++)
		{
			CString strCategory = m_arCategories[i];

			sizeText = m_pChart->OnCalcAxisLabelSize(pGM, this, i, strCategory, m_axisLabelsFormat.m_textFormat);
			m_szMaxLabelSize.cx = max(m_szMaxLabelSize.cx, sizeText.cx);
			m_szMaxLabelSize.cy = max(m_szMaxLabelSize.cy, sizeText.cy);
		}
	}
	else
	{
		CBCGPChartAxis::CalcMaxLabelSize(pGM);
	}
}
//----------------------------------------------------------------------------//
// Z-axis implementation
//----------------------------------------------------------------------------//
CBCGPChartAxisZ::CBCGPChartAxisZ()
{

}
//----------------------------------------------------------------------------//
CBCGPChartAxisZ::CBCGPChartAxisZ(int nAxisID, CBCGPChartAxis::AxisDefaultPosition position, CBCGPChartVisualObject* pChartCtrl) : 
	CBCGPChartAxisX(nAxisID, position, pChartCtrl)
{
	CommonInit();
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisZ::CommonInit()
{
	m_crossType = CBCGPChartAxis::CT_IGNORE;
	m_bDisplaySeriesNameAsLabel = TRUE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisZ::Reset()
{

}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxisZ::IsAxisVisible() const
{
	BOOL bVisible = CBCGPChartAxisX::IsAxisVisible();

	if (!bVisible || !IsDiagram3D())
	{
		return FALSE;
	}

	return !m_pChart->IsChart3DGrouped();
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisZ::CalcMaxLabelSize(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(this);

	m_szMaxLabelSize.SetSize(0, 0);

	if (m_pChart->IsChart3DGrouped())
	{
		return;
	}

	CBCGPChartAxis::CalcMaxLabelSize(pGM);
}
//----------------------------------------------------------------------------//
CBCGPChartAxis* CBCGPChartAxisZ::GetOppositeAxis()
{
	ASSERT_VALID(this);

	return NULL;
}
//----------------------------------------------------------------------------//
CBCGPChartAxis* CBCGPChartAxisZ::GetPerpendecularAxis() const
{
	ASSERT_VALID(this);

	if (m_pPerpendicularAxis != NULL)
	{
		return m_pPerpendicularAxis;
	}

	if (m_axisDefaultPosition == CBCGPChartAxis::ADP_DEPTH_TOP)
	{
		return m_pChart->GetChartAxis(BCGP_CHART_Y_SECONDARY_AXIS);
	}

	return m_pChart->GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisZ::GetPerpendecularAxes(CBCGPChartAxis*& pPrimary, CBCGPChartAxis*& pSecondary)
{
	ASSERT_VALID(this);

	pPrimary = m_pChart->GetChartAxis(BCGP_CHART_Y_PRIMARY_AXIS);
	pSecondary = m_pChart->GetChartAxis(BCGP_CHART_Y_SECONDARY_AXIS);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisZ::GetDisplayedLabel(double dblValue, CString& strLabel)
{
	ASSERT_VALID(this);

	strLabel.Empty();

	if (m_pChart != NULL && m_pChart->OnGetDisplayedLabel(this, dblValue, strLabel))
	{
		return;
	}

	int nCategoryIndex = (int)(dblValue - 1.);

	if (!m_bIsAxisComponentSet && m_bDisplaySeriesNameAsLabel)
	{
		CBCGPChartSeries* pSeries = m_pChart->GetSeries(nCategoryIndex, FALSE);
		
		if (pSeries != NULL)
		{
			if (!m_bThumbnailMode)
			{
				strLabel = pSeries->m_strSeriesName;
			}
			return;
		}
	}

	CBCGPChartAxis::GetDisplayedLabel(dblValue, strLabel);
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxisZ::CalcMinMaxValues()
{
	ASSERT_VALID(this);

	if (m_pChart->IsChart3DGrouped())
	{
		SetMinimumValue(0);
		SetMaximumValue(1);
		m_nMaxTotalLines = 1;
		return TRUE;
	}

	CheckAxisComponent();

	if (!IsFixedDisplayRange())
	{
		if (!IsComponentXSet())
		{
			SetMinimumValue(1);
			SetMaximumValue(1);
		}
		else
		{
			SetMinimumValue(0);
			SetMaximumValue(0);
		}
	}

	BOOL bSet = CBCGPChartAxis::CalcMinMaxValues();

	// CalcMinMaxValues values can take the minimum from a formula and override the 1.
	if (bSet && !IsFixedDisplayRange() && !IsAxisComponentSet())
	{
		m_dblMinimum = min(m_dblMinimum, 1.);
	}

	int nSeriesCount = 0;
	int nMaxGroupID = 0;
	BOOL bIsStacked = FALSE;
	for (int i = 0 ; i < m_pChart->GetSeriesCount(); i++)
	{
		CBCGPChartSeries* pSeries = m_pChart->GetSeries(i);
		if (pSeries == NULL || !pSeries->m_bVisible || !pSeries->IsShownOnAxis(this))
		{
			continue;
		}

		nSeriesCount++;
		nMaxGroupID = max(nMaxGroupID, pSeries->GetGroupID());

		if (!bIsStacked && pSeries->IsStakedSeries())
		{
			bIsStacked = TRUE;
		}
	}

	if (!m_pChart->IsChart3DGrouped() && bIsStacked && !IsAxisComponentSet())
	{
		nSeriesCount = nMaxGroupID + 1;
	}

	if (!IsFixedDisplayRange() && !IsAxisComponentSet())
	{
		SetMaximumValue(max(m_dblMaximum, nSeriesCount));				
		m_nMaxTotalLines = max(m_nMaxTotalLines, (UINT)nSeriesCount);
	}

	if (m_bIsAxisComponentSet && m_nMaxTotalLines <= 2)
	{
		m_nMaxTotalLines = 0xFFFE;
	}

	CBCGPChartAxis* pAxis = GetPerpendecularAxis();

	if (pAxis != NULL)
	{
		if (pAxis->m_crossType == CBCGPChartAxis::CT_AXIS_VALUE && !IsDiagram3D())
		{
			m_dblMinimum = min(pAxis->m_dblCrossOtherAxisValue, m_dblMinimum);
			m_dblMaximum = max(pAxis->m_dblCrossOtherAxisValue, m_dblMaximum);
		}
	}

	if (IsDisplayDataBetweenTickMarks())
	{
		m_nMaxTotalLines++;
		SetMaximumValue(max(m_dblMaximum, nSeriesCount + 1));
	}
	else if (!IsAxisComponentSet())
	{
		SetMaximumValue(max(m_dblMaximum, nSeriesCount));
	}

	return TRUE;
}
//----------------------------------------------------------------------------//
// POLAR Axis implementation - Y axis
//----------------------------------------------------------------------------//
CBCGPChartAxisPolarY::CBCGPChartAxisPolarY()
{

}

CBCGPChartAxisPolarY::CBCGPChartAxisPolarY(int nAxisID, CBCGPChartAxis::AxisDefaultPosition position, CBCGPChartVisualObject* pChartCtrl) :
	CBCGPChartAxisY(nAxisID, position, pChartCtrl)
{
	CommonInit();
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarY::CommonInit()
{
	CBCGPChartAxisY::CommonInit();
	m_bRadialGridLines = TRUE;
	m_bUseApproximation = FALSE;
}
//----------------------------------------------------------------------------//
CBCGPChartAxis* CBCGPChartAxisPolarY::GetPerpendecularAxis() const
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(m_pChart);

	return m_pChart->GetChartAxis(BCGP_CHART_X_POLAR_AXIS);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarY::CalcNameRect(CBCGPGraphicsManager* /*pGM*/, CBCGPRect& /*rectPlotArea*/, const CRect& /*rectChartArea*/, BOOL /*bReposOnly*/)
{
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarY::AdjustDiagramArea(CBCGPRect& /*rectDiagramArea*/, const CBCGPRect& /*rectPlotArea*/)
{
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarY::UpdateAxisPos(const CBCGPRect& rectDiagramArea)
{
	ASSERT_VALID(this);

	CalcAxisPos(rectDiagramArea, FALSE);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarY::CalcAxisPos(const CBCGPRect& rectDiagram, BOOL /*bStoreAxisPos*/)
{
	ASSERT_VALID(this);

	CBCGPChartAxis* pXAxis = GetPerpendecularAxis();

	if (pXAxis == NULL)
	{
		return;
	}
	
	double dblPadding = IsVertical() ? pXAxis->m_szMaxLabelSize.cy : pXAxis->m_szMaxLabelSize.cx;
	dblPadding += GetLabelDistance();

	if (dblPadding == 0)
	{
		dblPadding = (pXAxis->GetMajorTickMarkLen(TRUE) + 1) * 2;
	}

	double dblWidth = rectDiagram.Width();
	double dblHeight = rectDiagram.Height();
	double dblDiff = 2 * m_pChart->GetScaleRatio().cy;

	if (dblHeight <= dblWidth)
	{
		if (dblWidth - dblHeight < pXAxis->m_szMaxLabelSize.cx)
		{
			dblDiff += max(0, dblHeight - dblWidth + pXAxis->m_szMaxLabelSize.cx);
		}
	}
	else
	{
		dblDiff = pXAxis->m_szMaxLabelSize.cx;
	}

	m_ptAxisStart = rectDiagram.CenterPoint();
	if (IsVertical())
	{
		m_ptAxisEnd.x = m_ptAxisStart.x;
		m_ptAxisEnd.y = m_ptAxisStart.y - min(dblHeight / 2, dblWidth / 2);
		m_ptAxisEnd.y += dblPadding + dblDiff;
	}
	else
	{
		m_ptAxisEnd.y = m_ptAxisStart.y;
		m_ptAxisEnd.x = m_ptAxisStart.x + min(dblHeight / 2, dblWidth / 2);
		m_ptAxisEnd.x -= (dblPadding + dblDiff);
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarY::CalcLabelsRect(CBCGPRect& rectDiagramArea)
{
	ASSERT_VALID(this);

	CalcAxisPos(rectDiagramArea);

	if (!IsAxisVisible() || GetAxisLabelType() == CBCGPChartAxis::ALT_NO_LABELS)
	{
		m_rectAxisLabels.SetRectEmpty();
		return;
	}

	double nLabelRectSize = IsVertical() ? m_szMaxLabelSize.cx : m_szMaxLabelSize.cy; 
	double dblDistance = GetLabelDistance();

	CBCGPPoint ptAxisStart = m_ptAxisStart;
	CBCGPPoint ptAxisEnd = m_ptAxisEnd;

	m_rectAxisLabels.SetRect(m_ptAxisStart, m_ptAxisEnd);

	switch (GetAxisLabelType())
	{
	case CBCGPChartAxis::ALT_LOW:
	case CBCGPChartAxis::ALT_NEXT_TO_AXIS:
		if (IsVertical())
		{
			m_rectAxisLabels.left = m_ptAxisStart.x - dblDistance - nLabelRectSize;
			m_rectAxisLabels.right = m_rectAxisLabels.left + nLabelRectSize;
		}
		else
		{
			m_rectAxisLabels.top = m_ptAxisStart.y + dblDistance;
			m_rectAxisLabels.bottom = m_rectAxisLabels.top + nLabelRectSize;
		}
		break;
		
	case CBCGPChartAxis::ALT_HIGH:			
		if (IsVertical())
		{
			m_rectAxisLabels.left = rectDiagramArea.right + dblDistance;
			m_rectAxisLabels.right = m_rectAxisLabels.left + nLabelRectSize;
		}
		else
		{
			m_rectAxisLabels.top = rectDiagramArea.top - dblDistance - nLabelRectSize;
			m_rectAxisLabels.bottom = m_rectAxisLabels.top + nLabelRectSize;
		}
		break;
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarY::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram)
{
	ASSERT_VALID(this);

	if (!IsAxisVisible() || m_pChart == NULL)
	{
		return;
	}

	CBCGPChartAxisY::OnDraw(pGM, rectDiagram);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarY::OnFillUnitIntervals(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagram*/)
{
	ASSERT_VALID(this);

	BOOL bIsAxisVisible = IsAxisVisible();
	CBCGPChartAxisPolarX* pXAxis = DYNAMIC_DOWNCAST(CBCGPChartAxisPolarX, GetPerpendecularAxis());

	if (m_nDisplayedLines == 0 || !m_bFillMajorUnitInterval || m_pChart == NULL || pXAxis == NULL || !bIsAxisVisible || !m_bShowMajorGridLines || 
		IsLogScale())
	{
		return;
	}

	ASSERT_VALID(pXAxis);

	double dblCurrValue = GetMinDisplayedValue() + m_nFirstIntervalToFill * m_dblMajorUnit;

	const CBCGPBrush& brChart = pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY) ?
		m_pChart->GetColors().m_brAxisInterlaceColor :
		m_pChart->GetColors().m_brAxisInterlaceColorGDI;

	const CBCGPBrush& br = m_brInterval.IsEmpty() ? brChart : m_brInterval;

	while (dblCurrValue <= GetMaxDisplayedValue() + GetDoubleCorrection() - m_dblMajorUnit)
	{
		double dblRadiusCurr = IsVertical() ? m_ptAxisStart.y - PointFromValue(dblCurrValue, TRUE) : 
			PointFromValue(dblCurrValue, TRUE) - m_ptAxisStart.x;
		double dblRadiusNext = IsVertical() ? m_ptAxisStart.y - PointFromValue(dblCurrValue + m_dblMajorUnit, TRUE) : 
			PointFromValue(dblCurrValue + m_dblMajorUnit, TRUE) - m_ptAxisStart.x;

		if (m_bRadialGridLines)
		{
			CBCGPEllipseGeometry gCurrEllipse(CBCGPEllipse(m_ptAxisStart, dblRadiusCurr, dblRadiusCurr));
			CBCGPEllipseGeometry gNextEllipse(CBCGPEllipse(m_ptAxisStart, dblRadiusNext, dblRadiusNext));

			pGM->CombineGeometry(gNextEllipse, gNextEllipse, gCurrEllipse, RGN_DIFF);
			m_pChart->OnFillAxisUnitInterval(pGM, gNextEllipse, br);
		}
		else
		{
			CBCGPPointsArray arCurr;
			CBCGPPointsArray arNext;

			int nIndex = 0;
			int nMaxValue = (int) pXAxis->GetMaxDisplayedValue();

			if (!pXAxis->IsComponentXSet())
			{
				nMaxValue++;
			}

			for (int i = (int)pXAxis->GetMinDisplayedValue(); i < nMaxValue; i += (int)pXAxis->GetMajorUnit(), nIndex++)
			{
				double dblAnglePoint = bcg_deg2rad(pXAxis->GetAngleFromIndex(nIndex));

				CBCGPPoint ptCurr = m_ptAxisStart;
				CBCGPPoint ptNext = m_ptAxisStart;

				ptCurr.x += dblRadiusCurr * sin(dblAnglePoint);
				ptCurr.y -= dblRadiusCurr * cos(dblAnglePoint);

				ptNext.x += dblRadiusNext * sin(dblAnglePoint);
				ptNext.y -= dblRadiusNext * cos(dblAnglePoint);

				arCurr.Add(ptCurr);
				arNext.Add(ptNext);
			}

			CBCGPPolygonGeometry gCurrPolygon(arCurr);
			CBCGPPolygonGeometry gNextPolygon(arNext);

			pGM->CombineGeometry(gNextPolygon, gNextPolygon, gCurrPolygon, RGN_DIFF);
			m_pChart->OnFillAxisUnitInterval(pGM, gNextPolygon, br);
		}

		dblCurrValue += m_dblMajorUnit * m_nUnitFillStep;
	}

}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarY::OnDrawMajorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagram*/)
{
	ASSERT_VALID(this);
	DrawGridLines(pGM, TRUE);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarY::OnDrawMinorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagram*/)
{
	ASSERT_VALID(this);
	if (!IsLogScale())
	{
		DrawGridLines(pGM, FALSE);
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarY::DrawGridLines(CBCGPGraphicsManager* pGM, BOOL bMajor)
{
	ASSERT_VALID(this);

	BOOL bIsAxisVisible = IsAxisVisible();
	CBCGPChartAxisPolarX* pXAxis = DYNAMIC_DOWNCAST(CBCGPChartAxisPolarX, GetPerpendecularAxis());

	if (m_nDisplayedLines == 0 || m_pChart == NULL || pXAxis == NULL || !bIsAxisVisible || !m_bShowMajorGridLines && bMajor ||
		!m_bShowMinorGridLines && !bMajor)
	{
		return;
	}

	ASSERT_VALID(pXAxis);

	double dblUnit = bMajor ? m_dblMajorUnit : m_dblMinorUnit;
	const BCGPChartFormatLine& formatGridLine = bMajor ? m_majorGridLineFormat : m_minorGridLineFormat;

	BOOL bRadialLines = m_bRadialGridLines;

	double dblRadius = 0.;
	double dblUnitStep = CalcLog(GetMinDisplayedValue());

	double dblCurrValue = IsLogScale() ? GetMinDisplayedValue() : GetMinDisplayedValue() + dblUnit;

	while (dblCurrValue <= GetMaxDisplayedValue() + GetDoubleCorrection())
	{
		dblRadius = IsVertical() ? m_ptAxisStart.y - PointFromValue(dblCurrValue, TRUE) : 
			PointFromValue(dblCurrValue, TRUE) - m_ptAxisStart.x;

		CBCGPEllipse e(m_ptAxisStart, dblRadius, dblRadius);

		if (bRadialLines)
		{
			m_pChart->OnDrawGridEllipse(pGM, e, this, dblCurrValue, formatGridLine, bMajor);
		}
		else
		{
			int nIndex = 0;
			int nMaxValue = (int) pXAxis->GetMaxDisplayedValue();

			if (!pXAxis->IsComponentXSet())
			{
				nMaxValue++;
			}

			for (int i = (int)pXAxis->GetMinDisplayedValue(); i < nMaxValue; i += (int)pXAxis->GetMajorUnit(), nIndex++)
			{
				double dblAngleStart = bcg_deg2rad(pXAxis->GetAngleFromIndex(nIndex));
				double dblAngleEnd = bcg_deg2rad(pXAxis->GetAngleFromIndex(nIndex + 1));

				CBCGPPoint ptStart = m_ptAxisStart;
				CBCGPPoint ptEnd = m_ptAxisStart;

				ptStart.x += dblRadius * sin(dblAngleStart);
				ptStart.y -= dblRadius * cos(dblAngleStart);

				ptEnd.x += dblRadius * sin(dblAngleEnd);
				ptEnd.y -= dblRadius * cos(dblAngleEnd);

				m_pChart->OnDrawGridLine(pGM, ptStart, ptEnd, this, dblCurrValue, formatGridLine, bMajor);
			}
		}

		if (IsLogScale())
		{
			dblUnitStep += 1.;
			dblCurrValue = pow(m_dblMajorUnit, dblUnitStep);
		}
		else
		{
			dblCurrValue += dblUnit;
		}
	}
}
//----------------------------------------------------------------------------//
CBCGPRect CBCGPChartAxisPolarY::GetBoundingRect() const
{
	ASSERT_VALID(this);

	CBCGPRect rect(m_bReverseOrder ? m_ptAxisEnd : m_ptAxisStart, m_bReverseOrder ? m_ptAxisEnd : m_ptAxisStart);
	double dblSize = fabs(GetAxisSize());
	rect.InflateRect(dblSize, dblSize);

	return rect;
}
//----------------------------------------------------------------------------//
CBCGPRect CBCGPChartAxisPolarY::GetBoundingRects(CArray<CBCGPRect, CBCGPRect>& arRects) const
{
	ASSERT_VALID(this);

	CBCGPRect r = GetBoundingRect();
	arRects.Add(r);

	return r;
}
//----------------------------------------------------------------------------//
double CBCGPChartAxisPolarY::ValueFromPoint(const CBCGPPoint& pt, CBCGPChartAxis::RoundType roundType/* = CBCGPChartAxis::RT_EXACT*/)
{
	ASSERT_VALID(this);

	const double dblDistance = bcg_distance(pt, m_ptAxisStart);
	if (dblDistance > bcg_distance(m_ptAxisEnd, m_ptAxisStart))
	{
		return 0.;
	}

	return CBCGPChartAxisY::ValueFromPoint(CBCGPPoint(m_ptAxisStart.x, m_ptAxisStart.y - dblDistance), roundType);
}

//----------------------------------------------------------------------------//
// POLAR Axis implementation - X axis
//----------------------------------------------------------------------------//
CBCGPChartAxisPolarX::CBCGPChartAxisPolarX()
{

}
//----------------------------------------------------------------------------//
CBCGPChartAxisPolarX::CBCGPChartAxisPolarX(int nAxisID, CBCGPChartAxis::AxisDefaultPosition position, CBCGPChartVisualObject* pChartCtrl) :
		CBCGPChartAxisX(nAxisID, position, pChartCtrl)
{
	CommonInit();
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarX::CommonInit()
{
	CBCGPChartAxisX::CommonInit();

	m_nMinorUnitCount = 5;
	m_dblMajorUnit = 15.;
	m_dblMinorUnit = m_dblMajorUnit / m_nMinorUnitCount;
	m_dblMinDisplayedValue = 0.;
	m_dblMaxDisplayedValue = 360.;

	m_bRotateLabels = FALSE;
	m_bUseGridLineForAxisLine = FALSE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarX::CalcMajorMinorUnits(CBCGPGraphicsManager* /*pGM*/, UINT /*nUnitsToDisplay*/)
{
	CheckAxisComponent();

	if (!IsComponentXSet())
	{
		m_dblMajorUnit = 1.;
		m_dblMinorUnit = m_dblMajorUnit / m_nMinorUnitCount;
		m_dblMinDisplayedValue = 0;
		m_dblMaxDisplayedValue = m_nMaxDataPointCount - 1;
	}
	else
	{
		m_dblMajorUnit = 15.;
		m_dblMinorUnit = m_dblMajorUnit / m_nMinorUnitCount;
		m_dblMinDisplayedValue = 0.;
		m_dblMaxDisplayedValue = 360.;
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarX::SetFixedMajorUnit(double dblMajorUnit, BOOL bSet)
{
	CheckAxisComponent();

	if (IsComponentXSet())
	{
		if (bSet)
		{
			m_dblMajorUnit = bcg_clamp(dblMajorUnit, 1., 360.);
		}
		else
		{
			m_dblMajorUnit = 15;
		}

		m_dblMinorUnit = m_dblMajorUnit / m_nMinorUnitCount;
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarX::CalcMinMaxDisplayedValues(BOOL /*bCalcMinDisplayedValue*/, double /*dblApproximation*/)
{
	ASSERT_VALID(this);

	CheckAxisComponent();

	if (!IsComponentXSet())
	{
		m_dblMinDisplayedValue = 0.;
		m_dblMaxDisplayedValue = 360.;
	}
	else
	{
		m_dblMinDisplayedValue = 0;
		m_dblMaxDisplayedValue = m_nMaxDataPointCount - 1;
	}
}
//----------------------------------------------------------------------------//
CBCGPChartAxis* CBCGPChartAxisPolarX::GetPerpendecularAxis() const
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(m_pChart);

	return m_pChart->GetChartAxis(BCGP_CHART_Y_POLAR_AXIS);
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartAxisPolarX::HitTest(const CBCGPPoint& pt, BCGPChartHitInfo* pHitInfo, DWORD dwHitInfoFlags)
{
	ASSERT_VALID(this);

	if (pHitInfo == NULL || !IsAxisVisible() || (dwHitInfoFlags & BCGPChartHitInfo::HIT_AXIS) == 0)
	{
		return FALSE;
	}

	CBCGPPoint ptCenter;
	double dblRadius = GetRadius(ptCenter);

	double dblDistance = bcg_distance(pt, ptCenter);

	if (fabs(dblDistance - dblRadius) < GetMajorTickMarkLen(TRUE))
	{
		pHitInfo->m_hitInfo = BCGPChartHitInfo::HIT_AXIS;
		pHitInfo->m_nIndex1 = m_nAxisID;
		return TRUE;
	}

	return FALSE;
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarX::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagram*/)
{
	ASSERT_VALID(this);

	if (m_pChart == NULL || !IsAxisVisible() || !IsComponentXSet())
	{
		return;
	}

	CBCGPChartAxisPolarY* pAxisY = DYNAMIC_DOWNCAST(CBCGPChartAxisPolarY, GetPerpendecularAxis());
	if (pAxisY == NULL || !pAxisY->m_bRadialGridLines || m_dblMajorUnit == 0)  
	{
		return;
	}

	CBCGPPoint ptCenter;
	double dblRadius = GetRadius(ptCenter);

	double dblCurrValue = GetMinDisplayedValue();

	while (dblCurrValue < GetMaxDisplayedValue())
	{
		if (m_majorTickMarkType != CBCGPChartAxis::TMT_NO_TICKS ||
			m_majorTickMarkType == CBCGPChartAxis::TMT_NO_TICKS && 
			m_minorTickMarkType != CBCGPChartAxis::TMT_NO_TICKS)
		{
			// if major tick marks are not visible, draw minor in place of them 
			DrawTickMark(pGM, dblCurrValue, ptCenter, dblRadius, 
				m_majorTickMarkType != CBCGPChartAxis::TMT_NO_TICKS);
		}

		if ((m_minorTickMarkType != CBCGPChartAxis::TMT_NO_TICKS) && dblCurrValue + m_dblMinorUnit <= GetMaxDisplayedValue())
		{
			double dblMinorValue = dblCurrValue + m_dblMinorUnit;
			for (int i = 0; i < m_nMinorUnitCount - 1; i++)
			{
				DrawTickMark(pGM, dblMinorValue, ptCenter, dblRadius, FALSE);
				dblMinorValue += m_dblMinorUnit;
			}
		}

		dblCurrValue += m_dblMajorUnit;
	}


	CBCGPEllipse ellipse(ptCenter, dblRadius, dblRadius);


	if (m_bUseGridLineForAxisLine)
	{
		const CBCGPBrush& brLine = m_majorGridLineFormat.m_brLineColor.IsEmpty() ? 
			m_pChart->GetColors().m_brAxisMajorGridLineColor : m_majorGridLineFormat.m_brLineColor;

		pGM->DrawEllipse(ellipse, brLine, m_majorGridLineFormat.GetLineWidth(TRUE), 
							&m_majorGridLineFormat.m_strokeStyle);
	}
	else
	{
		const CBCGPBrush& brLineColor = m_axisLineFormat.m_brLineColor.IsEmpty() ? 
						m_pChart->GetColors().m_brAxisLineColor : m_axisLineFormat.m_brLineColor;

		pGM->DrawEllipse(ellipse, brLineColor, m_axisLineFormat.GetLineWidth(TRUE), &m_axisLineFormat.m_strokeStyle);
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarX::DrawTickMark(CBCGPGraphicsManager* pGM, double dblValue, CBCGPPoint ptPolarCenter, 
										double dblRadius, BOOL bMajor)
{
	ASSERT_VALID(this);

	CBCGPChartAxis::TickMarkType tickMarkType = bMajor ? m_majorTickMarkType : m_minorTickMarkType;
	double dblTickMarkeLen = bMajor ? GetMajorTickMarkLen(TRUE) : GetMinorTickMarkLen(TRUE);

	double dblAngle = bcg_deg2rad(dblValue);

	ptPolarCenter.x += dblRadius * sin(dblAngle); 
	ptPolarCenter.y -= dblRadius * cos(dblAngle);

	CBCGPPoint ptStartTick = ptPolarCenter; 
	CBCGPPoint ptEndTick = ptPolarCenter;

	switch (tickMarkType)
	{
	case CBCGPChartAxis::TMT_INSIDE:
		ptStartTick.x -= dblTickMarkeLen * sin(dblAngle); 
		ptStartTick.y += dblTickMarkeLen * cos(dblAngle); 
		break;

	case CBCGPChartAxis::TMT_OUTSIDE:
		ptStartTick.x += dblTickMarkeLen * sin(dblAngle); 
		ptStartTick.y -= dblTickMarkeLen * cos(dblAngle); 
		break;
	case CBCGPChartAxis::TMT_CROSS:
		ptStartTick.x -= dblTickMarkeLen * sin(dblAngle); 
		ptStartTick.y += dblTickMarkeLen * cos(dblAngle); 
		ptEndTick.x += dblTickMarkeLen * sin(dblAngle); 
		ptEndTick.y -= dblTickMarkeLen * cos(dblAngle); 
		break;
	}

	if (m_bUseGridLineForAxisLine)
	{
		BCGPChartFormatLine lineFormat = m_majorGridLineFormat;
		lineFormat.m_brLineColor = m_majorGridLineFormat.m_brLineColor.IsEmpty() ? 
			m_pChart->GetColors().m_brAxisMajorGridLineColor : m_majorGridLineFormat.m_brLineColor;

		m_pChart->OnDrawTickMark(pGM, ptStartTick, ptEndTick, lineFormat, dblValue, bMajor);
	}
	else
	{
		m_pChart->OnDrawTickMark(pGM, ptStartTick, ptEndTick, m_axisLineFormat, dblValue, bMajor);
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarX::OnDrawMajorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagram*/)
{
	ASSERT_VALID(this);
	DrawGridLines(pGM, TRUE);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarX::OnDrawMinorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagram*/)
{
	ASSERT_VALID(this);
	DrawGridLines(pGM, FALSE);
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarX::DrawGridLines(CBCGPGraphicsManager* pGM, BOOL bMajor)
{
	ASSERT_VALID(this);

	CBCGPPoint ptCenter;
	double dblRadius = GetRadius(ptCenter);

	if (m_pChart == NULL || dblRadius == 0 || !m_bShowMajorGridLines && bMajor ||
		!m_bShowMinorGridLines && !bMajor || !IsAxisVisible() ||
		!m_bIsAxisComponentSet && m_nMaxDataPointCount == 0  || IsComponentXSet() && m_dblMajorUnit == 0)
	{
		return;
	}

	if (!IsComponentXSet() && !bMajor)
	{
		return;
	}

	double dblUnit = bMajor ? m_dblMajorUnit : m_dblMinorUnit;
	const BCGPChartFormatLine& formatGridLine = bMajor ? m_majorGridLineFormat : m_minorGridLineFormat;

	ASSERT_VALID(m_pChart);
	double dblCurrValue = GetMinDisplayedValue();
	double dblMaxValue = IsComponentXSet() ? GetMaxDisplayedValue() : 360.;
	double dblAngleStep = IsComponentXSet() ? dblUnit : 360. / m_nMaxDataPointCount;

	while (dblCurrValue < dblMaxValue)
	{
		CBCGPPoint ptGridLineEnd = ptCenter;
		double dblAngle = bcg_deg2rad(dblCurrValue);

		ptGridLineEnd.x += dblRadius * sin(dblAngle); 
		ptGridLineEnd.y -= dblRadius * cos(dblAngle); 

		m_pChart->OnDrawGridLine(pGM, ptCenter, ptGridLineEnd, this, dblCurrValue, formatGridLine, bMajor);
		dblCurrValue += dblAngleStep;
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarX::OnFillUnitIntervals(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagram*/)
{
	CBCGPPoint ptCenter;
	double dblRadius = GetRadius(ptCenter);

	if (m_pChart == NULL || dblRadius == 0 || !m_bShowMajorGridLines || !IsAxisVisible())
	{
		return;
	}

	CBCGPChartAxisPolarY* pAxisY = DYNAMIC_DOWNCAST(CBCGPChartAxisPolarY, GetPerpendecularAxis());
	if (pAxisY == NULL  || !IsComponentXSet() && m_nMaxDataPointCount == 0  ||
		IsComponentXSet() && m_dblMajorUnit == 0)
	{
		return;
	}
	
	ASSERT_VALID(pAxisY);

	const CBCGPBrush& brChart = pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY) ?
		m_pChart->GetColors().m_brAxisInterlaceColor :
		m_pChart->GetColors().m_brAxisInterlaceColorGDI;

	const CBCGPBrush& br = m_brInterval.IsEmpty() ? brChart : m_brInterval;

	double dblMaxValue = m_bIsAxisComponentSet ? GetMaxDisplayedValue() : 360.;
	double dblAngleStep = m_bIsAxisComponentSet ? m_dblMajorUnit : 360. / m_nMaxDataPointCount;

	double dblCurrValue = GetMinDisplayedValue() + dblAngleStep * m_nFirstIntervalToFill;

	while (dblCurrValue < dblMaxValue)
	{
		CBCGPPoint ptGridLineCurr = ptCenter;
		CBCGPPoint ptGridLineNext = ptCenter;

		double dblAngleCurr = bcg_deg2rad(dblCurrValue);
		double dblAngleNext = bcg_deg2rad(dblCurrValue + dblAngleStep);

		ptGridLineCurr.x += dblRadius * sin(dblAngleCurr); 
		ptGridLineCurr.y -= dblRadius * cos(dblAngleCurr); 

		ptGridLineNext.x += dblRadius * sin(dblAngleNext); 
		ptGridLineNext.y -= dblRadius * cos(dblAngleNext); 

		dblCurrValue += dblAngleStep * m_nUnitFillStep;

		CBCGPComplexGeometry gFill(ptCenter);
		gFill.AddLine(ptGridLineCurr);

		if (pAxisY->m_bRadialGridLines)
		{
			gFill.AddArc(ptGridLineNext, CBCGPSize(dblRadius, dblRadius), TRUE, FALSE, 0);
		}
		else
		{
			gFill.AddLine(ptGridLineNext);
		}
		
		m_pChart->OnFillAxisUnitInterval(pGM, gFill, br);
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarX::OnDrawAxisLabels(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram)
{
	ASSERT_VALID(this);

	CBCGPPoint ptCenter;
	double dblRadius = GetRadius(ptCenter);

	if (m_pChart == NULL || !IsAxisVisible() || dblRadius == 0. || 
		GetAxisLabelType() == CBCGPChartAxis::ALT_NO_LABELS || !IsComponentXSet() && m_nMaxDataPointCount == 0 ||
		IsComponentXSet() && m_dblMajorUnit == 0)
	{
		return;
	}

	ASSERT_VALID(m_pChart);
	double dblCurrValue = GetMinDisplayedValue();

	double dblMaxValue = 360.;
	double dblAngleStep = IsComponentXSet() ? m_dblMajorUnit : 360. / m_nMaxDataPointCount;

	CBCGPSize szContentPadding = m_axisLabelsFormat.GetContentPadding(TRUE);
	double dblLabelDistance = GetLabelDistance();

	int nCount = 0;

	while (dblCurrValue < dblMaxValue)
	{
		double dblDisplayedValue = IsComponentXSet() ? dblCurrValue : nCount + 1;
		double dblDrawingAngle = m_axisLabelsFormat.m_textFormat.GetDrawingAngle();

		if (m_bRotateLabels)
		{
			m_axisLabelsFormat.m_textFormat.SetDrawingAngle(GetVariableLabelAngle(dblCurrValue));
		}

		CString strLabel;
		GetDisplayedLabel(dblDisplayedValue, strLabel);

		CBCGPSize szLabel = pGM->GetTextSize(strLabel, m_axisLabelsFormat.m_textFormat);

		double dblDistanceX = dblRadius + szLabel.cx / 2 + szContentPadding.cx + dblLabelDistance;
		double dblDistanceY = dblRadius + szLabel.cy / 2 + szContentPadding.cy + dblLabelDistance;

		CBCGPPoint ptLabelCenter = ptCenter;
		double dblAngle = bcg_deg2rad(dblCurrValue);

		ptLabelCenter.x += dblDistanceX * sin(dblAngle); 
		ptLabelCenter.y -= dblDistanceY * cos(dblAngle); 

		CBCGPRect rectLabel(ptLabelCenter, ptLabelCenter);
		rectLabel.InflateRect(szLabel.cx / 2, szLabel.cy / 2);

		m_pChart->OnDrawAxisLabel(pGM, dblDisplayedValue, strLabel, this, rectLabel, m_rectAxisLabels, rectDiagram);		

		if (m_bRotateLabels)
		{
			m_axisLabelsFormat.m_textFormat.SetDrawingAngle(dblDrawingAngle);
		}

		dblCurrValue += dblAngleStep;
		nCount++;
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartAxisPolarX::SwapDirection(BOOL /*bAdjustGradientAngles*/)
{
	ASSERT_VALID(this);
	m_bIsVertical = !IsVertical();
}
//----------------------------------------------------------------------------//
double CBCGPChartAxisPolarX::GetVariableLabelAngle (double dblValue)
{
	return 90 - dblValue;
}
//----------------------------------------------------------------------------//
double CBCGPChartAxisPolarX::GetAngleFromIndex(int nIndex) const
{
	if (IsComponentXSet())
	{
		return m_dblMajorUnit * nIndex;
	}

	if (m_nMaxDataPointCount == 0)
	{
		return 0;
	}

	return 360. / m_nMaxDataPointCount * nIndex;
}
//----------------------------------------------------------------------------//
CBCGPRect CBCGPChartAxisPolarX::GetBoundingRect() const
{
	ASSERT_VALID(this);

	CBCGPChartAxis* pAxis = GetPerpendecularAxis();
	if (pAxis != NULL)
	{
		return pAxis->GetBoundingRect();
	}

	if (m_pChart != NULL)
	{
		return m_pChart->GetDiagramArea();
	}

	return CBCGPRect();
}
//----------------------------------------------------------------------------//
CBCGPRect CBCGPChartAxisPolarX::GetBoundingRects(CArray<CBCGPRect, CBCGPRect>& arRects) const
{
	ASSERT_VALID(this);

	CBCGPRect r = GetBoundingRect();
	arRects.Add(r);

	return r;
}
//----------------------------------------------------------------------------//
double CBCGPChartAxisPolarX::GetRadius(CBCGPPoint& ptCenter) const
{
	ASSERT_VALID(this);

	CBCGPChartAxis* pAxisY = GetPerpendecularAxis();
	if (pAxisY == NULL)
	{
		return 0;
	}
	
	ASSERT_VALID(pAxisY);

	CBCGPPoint ptAxisStart;
	CBCGPPoint ptAxisEnd;

	pAxisY->GetAxisPos(ptAxisStart, ptAxisEnd);
	ptCenter = ptAxisStart;

	return fabs(pAxisY->GetAxisSize());
}
//----------------------------------------------------------------------------//
double CBCGPChartAxisPolarX::GetAxisSize() const
{
	CBCGPPoint ptCenter;
	return 2 * M_PI * GetRadius(ptCenter);
}
//----------------------------------------------------------------------------//
double CBCGPChartAxisPolarX::ValueFromPoint(const CBCGPPoint& pt, CBCGPChartAxis::RoundType /*roundType*/)
{
	CBCGPPoint ptCenter;
	GetRadius(ptCenter);

	double dblAngle = bcg_angle(pt, ptCenter, FALSE);
	dblAngle = bcg_rad2deg(bcg_normalize_rad(dblAngle - M_PI / 2));
	
	if (IsComponentXSet())
	{
		return dblAngle;
	}

	double dblAngleStep = 360. / m_nMaxDataPointCount;
	return (int)(dblAngle / dblAngleStep);
}
//----------------------------------------------------------------------------//
// Ternary axis implementation
//----------------------------------------------------------------------------//
CBCGPChartTernaryAxis::CBCGPChartTernaryAxis()
{

}
//----------------------------------------------------------------------------//
CBCGPChartTernaryAxis::CBCGPChartTernaryAxis(int nAxisID, CBCGPChartAxis::AxisDefaultPosition position, CBCGPChartVisualObject* pChartCtrl) :
	CBCGPChartAxis(nAxisID, position, pChartCtrl)
{
	CommonInit();
}
//----------------------------------------------------------------------------//
void CBCGPChartTernaryAxis::CommonInit()
{
	CBCGPChartAxis::CommonInit();
	m_nMaxTotalLines = 100;
	m_bUsePerpAxisForZoning = FALSE;

	switch (m_axisDefaultPosition)
	{
	case CBCGPChartAxis::ADP_RIGHT:
		m_strAxisName = _T("A Component");
		m_dblTickAngle = bcg_deg2rad(60);
		break;
	case CBCGPChartAxis::ADP_LEFT:
		m_strAxisName = _T("B Component");
		m_dblTickAngle = bcg_deg2rad(-60);
		break;
	case CBCGPChartAxis::ADP_BOTTOM:
		m_strAxisName = _T("C Component");
		m_dblTickAngle = bcg_deg2rad(180);
		break;
	}

	m_axisNameFormat.m_textFormat.SetDrawingAngle(0);

	m_bDisplayAxisName = TRUE;
	m_bShowMajorGridLines = TRUE;

	m_minorTickMarkType = CBCGPChartAxis::TMT_OUTSIDE;
	m_nMinorUnitCount = 5;

	SetFixedMajorUnit(25, TRUE);

	m_dblACoef = 0;
	m_dblBCoef = 0;

	m_labelMode = CBCGPChartTernaryAxis::LM_NORMAL;
}
//----------------------------------------------------------------------------//
CBCGPChartAxis* CBCGPChartTernaryAxis::GetPerpendecularAxis() const
{
	CBCGPChartAxis* pAxis = NULL;

	switch (m_axisDefaultPosition)
	{
	case CBCGPChartAxis::ADP_RIGHT:
		pAxis = m_pChart->GetChartAxis(BCGP_CHART_B_TERNARY_AXIS);
		break;

	case CBCGPChartAxis::ADP_LEFT:
		pAxis = m_pChart->GetChartAxis(BCGP_CHART_C_TERNARY_AXIS);
		break;

	case CBCGPChartAxis::ADP_BOTTOM:
		pAxis = m_pChart->GetChartAxis(BCGP_CHART_A_TERNARY_AXIS);
		break;
	}

	return pAxis;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartTernaryAxis::CalcMinMaxValues()
{
	ASSERT_VALID(this);

	m_dblMinimum = 0;
	m_dblMaximum = 100;

	return TRUE;
}
//----------------------------------------------------------------------------//
void CBCGPChartTernaryAxis::CalcMinMaxDisplayedValues(BOOL /*bCalcMinDisplayedValue*/, double /*dblApproximation*/)
{
	ASSERT_VALID(this);

	m_dblMinDisplayedValue = 0;
	m_dblMaxDisplayedValue = 100;
}
//----------------------------------------------------------------------------//
void CBCGPChartTernaryAxis::AdjustDiagramArea(CBCGPRect& /*rectDiagramArea*/, const CBCGPRect& /*rectPlotArea*/)
{

}
//----------------------------------------------------------------------------//
void CBCGPChartTernaryAxis::UpdateAxisPos(const CBCGPRect& /*rectDiagramArea*/)
{

}
//----------------------------------------------------------------------------//
void CBCGPChartTernaryAxis::CalcNameRect(CBCGPGraphicsManager* pGM, CBCGPRect& /*rectPlotArea*/, const CRect& /*rectChartArea*/, BOOL bReposOnly)
{
	ASSERT_VALID(this);

	if (!bReposOnly)
	{
		m_rectAxisName.SetRectEmpty();
	}

	if (!m_bDisplayAxisName || !IsAxisVisible())
	{
		return;
	}

	if (!bReposOnly)
	{
		// called for the first time, axis position has not been calculated, therefore
		// we need to set the size of name only;
		// this size will be used later to calculate axis position
		CBCGPSize szAxisNameSize = m_pChart->OnCalcAxisNameSize(pGM, this);
		m_rectAxisName.SetRect(CBCGPPoint(0, 0), szAxisNameSize);
	}
	else
	{
		// axis position has been calculated;
		// we can set the actual name rect
		CBCGPSize szName = m_rectAxisName.Size();

		switch (m_axisDefaultPosition)
		{
		case CBCGPChartAxis::ADP_RIGHT:
			m_rectAxisName.left = m_rectBounds.CenterPoint().x - szName.cx / 2;
			m_rectAxisName.top = m_rectBounds.top - szName.cy - m_szMaxLabelSize.cy;
			break;

		case CBCGPChartAxis::ADP_LEFT:
			m_rectAxisName.left = m_rectBounds.left - szName.cx / 2;
			m_rectAxisName.top = m_rectBounds.bottom + m_pChart->GetChartAxis(BCGP_CHART_C_TERNARY_AXIS)->m_szMaxLabelSize.cy +
				m_pChart->GetChartAxis(BCGP_CHART_C_TERNARY_AXIS)->GetLabelDistance();
			break;

		case CBCGPChartAxis::ADP_BOTTOM:
			m_rectAxisName.left = m_rectBounds.right - szName.cx / 2;
			m_rectAxisName.top = m_rectBounds.bottom + m_szMaxLabelSize.cy + GetLabelDistance();
			break;
		}

		m_rectAxisName.SetSize(szName);
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartTernaryAxis::CalcAxisPos(const CBCGPRect& rectDiagram, BOOL /*bStoreAxisPos*/)
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return;
	}
	CBCGPRect rcDiagram = rectDiagram;

	CBCGPChartAxis* pRightAxis = m_pChart->GetChartAxis(BCGP_CHART_A_TERNARY_AXIS);
	CBCGPChartAxis* pLeftAxis = m_pChart->GetChartAxis(BCGP_CHART_B_TERNARY_AXIS);
	CBCGPChartAxis* pBottomAxis = m_pChart->GetChartAxis(BCGP_CHART_C_TERNARY_AXIS);

	rcDiagram.top += pRightAxis->m_rectAxisName.Height();
	rcDiagram.top += max(pRightAxis->m_szMaxLabelSize.cy + pRightAxis->GetLabelDistance(), pLeftAxis->m_szMaxLabelSize.cy + pLeftAxis->GetLabelDistance());
	rcDiagram.bottom -= max(pBottomAxis->m_rectAxisName.Height(), pLeftAxis->m_rectAxisName.Height());
	rcDiagram.bottom -= pBottomAxis->m_szMaxLabelSize.cy + pBottomAxis->GetLabelDistance();
	rcDiagram.left += max(pBottomAxis->m_rectAxisName.Width(), pLeftAxis->m_rectAxisName.Width()) / 2;
	rcDiagram.right -= max(pBottomAxis->m_rectAxisName.Width(), pLeftAxis->m_rectAxisName.Width()) / 2;

	CBCGPPoint ptCenter = rcDiagram.CenterPoint();

	double dblSize = rcDiagram.Width() > rcDiagram.Height() ? rcDiagram.Height() : rcDiagram.Width();
	m_rectBounds.SetRect(ptCenter, ptCenter);

	m_rectBounds.InflateRect(dblSize / 2, dblSize / 2);
	double dblNewTop = m_rectBounds.bottom - m_rectBounds.Width() * sin(bcg_deg2rad(60));
	double dblOffset = (dblNewTop - m_rectBounds.top) / 2;
	m_rectBounds.top = dblNewTop;
	m_rectBounds.OffsetRect(0, -dblOffset);

	switch (m_axisDefaultPosition)
	{
	case CBCGPChartAxis::ADP_RIGHT:
		m_ptAxisStart = m_rectBounds.BottomRight();
		m_ptAxisEnd.x = m_rectBounds.CenterPoint().x;
		m_ptAxisEnd.y = m_rectBounds.top;
		break;

	case CBCGPChartAxis::ADP_LEFT:
		m_ptAxisStart.x = m_rectBounds.CenterPoint().x;
		m_ptAxisStart.y = m_rectBounds.top;
		m_ptAxisEnd = m_rectBounds.BottomLeft();
		break;

	case CBCGPChartAxis::ADP_BOTTOM:
		m_ptAxisStart = m_rectBounds.BottomLeft();
		m_ptAxisEnd = m_rectBounds.BottomRight();
		break;
	}

	m_dblACoef = (m_ptAxisEnd.y - m_ptAxisStart.y) / (m_ptAxisEnd.x - m_ptAxisStart.x);
	m_dblBCoef = m_ptAxisEnd.y - m_dblACoef * m_ptAxisEnd.x;
}
//----------------------------------------------------------------------------//
double CBCGPChartTernaryAxis::GetAxisSize() const
{
	return m_rectBounds.Width();
}
//----------------------------------------------------------------------------//
void CBCGPChartTernaryAxis::CalcLabelsRect(CBCGPRect& /*rectDiagramArea*/)
{
	
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartTernaryAxis::HitTestAxisShape(const CBCGPPoint& pt) const
{
	double dblY = m_dblACoef * pt.x + m_dblBCoef;

	return pt.y > dblY - m_nMajorTickMarkLen && pt.y < dblY + m_nMajorTickMarkLen;
}
//----------------------------------------------------------------------------//
BOOL CBCGPChartTernaryAxis::HitTestAxisLables(const CBCGPPoint& /*pt*/) const
{
	return FALSE;
}
//----------------------------------------------------------------------------//
void CBCGPChartTernaryAxis::GetDisplayedLabel(double dblValue, CString& strLabel)
{
	switch (m_labelMode)
	{
	case CBCGPChartTernaryAxis::LM_NORMAL:
		CBCGPChartAxis::GetDisplayedLabel(dblValue, strLabel);
		break;

	case CBCGPChartTernaryAxis::LM_2080:
		strLabel.Format(_T("%.3G/%.3G"), dblValue, 100. - dblValue);
		break;

	case CBCGPChartTernaryAxis::LM_BASE_1:
		strLabel.Format(_T("%.3G"), dblValue / 100);
		break;
	}	
}
//----------------------------------------------------------------------------//
void CBCGPChartTernaryAxis::SetLabelMode(CBCGPChartTernaryAxis::LabelMode lm, BOOL bRedraw)
{
	if (lm != m_labelMode)
	{
		m_labelMode = lm;

		if (m_pChart != NULL)
		{
			m_pChart->SetDirty(TRUE, bRedraw);
		}
	}
}
//----------------------------------------------------------------------------//
double CBCGPChartTernaryAxis::PointFromValue(double /*dblValue*/, BOOL /*bForceValueOnThickMark*/, BOOL /*bLogValue*/)
{
	ASSERT(FALSE);
	TRACE0("CBCGPChartTernaryAxis::PointFromValue is not implemented. Call PointFromChartData instead");

	return 0;
}
//----------------------------------------------------------------------------//
CBCGPPoint CBCGPChartTernaryAxis::PointFromChartData(const CBCGPChartData& data)
{
	ASSERT_VALID(this);

	if (m_pChart == NULL)
	{
		return CBCGPPoint();
	}

	double dblRange = 100.;

	double dblA = fabs(data.GetValue(CBCGPChartData::CI_Y));
	double dblB = fabs(data.GetValue(CBCGPChartData::CI_Y1));
	double dblC = fabs(data.GetValue(CBCGPChartData::CI_Y2));

	double dblSum = dblA + dblB + dblC;

	if (dblSum == 0)
	{
		return CBCGPPoint();
	}

	double dblAPerc = dblA / dblSum * dblRange;
	double dblBPerc = dblB / dblSum * dblRange;
	
	double dblY = m_rectBounds.bottom - dblAPerc * m_rectBounds.Height() / dblRange;
	double dblX_B = dblBPerc * m_rectBounds.Width() / dblRange;

	CBCGPChartTernaryAxis* pAxisA = DYNAMIC_DOWNCAST(CBCGPChartTernaryAxis, m_pChart->GetChartAxis(BCGP_CHART_A_TERNARY_AXIS));
	ASSERT_VALID(pAxisA);
	
	double dblX = (dblY - pAxisA->m_dblBCoef - dblX_B * tan(bcg_deg2rad(60))) / pAxisA->m_dblACoef;

	return CBCGPPoint(dblX, dblY);
}
//----------------------------------------------------------------------------//
void CBCGPChartTernaryAxis::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagram*/)
{
	ASSERT_VALID(this);

	if (m_pChart == NULL || !IsAxisVisible() || m_dblMajorUnit == 0.)
	{
		return;
	}
	
	if (m_majorTickMarkType != CBCGPChartAxis::TMT_NO_TICKS || m_minorTickMarkType != CBCGPChartAxis::TMT_NO_TICKS)
	{
		CBCGPChartData data;
		data.SetValue(0., CBCGPChartData::CI_Y);
		data.SetValue(0., CBCGPChartData::CI_Y1);
		data.SetValue(0., CBCGPChartData::CI_Y2);

		int nStep = 0;

		for (double dblCurValue = 0; dblCurValue <= m_dblMaximum;)
		{
			switch (m_axisDefaultPosition)
			{
			case CBCGPChartAxis::ADP_RIGHT:
				data.SetValue(dblCurValue, CBCGPChartData::CI_Y);
				data.SetValue(m_dblMaximum - dblCurValue, CBCGPChartData::CI_Y2);
				break;
			case CBCGPChartAxis::ADP_LEFT:
				data.SetValue(dblCurValue, CBCGPChartData::CI_Y1);
				data.SetValue(m_dblMaximum - dblCurValue, CBCGPChartData::CI_Y);
				break;
			case CBCGPChartAxis::ADP_BOTTOM:
				data.SetValue(dblCurValue, CBCGPChartData::CI_Y2);
				data.SetValue(m_dblMaximum - dblCurValue, CBCGPChartData::CI_Y1);
				break;
			}

			CBCGPPoint pt = PointFromChartData(data);

			int nTickMarkLen = (nStep == 0 && m_majorTickMarkType != CBCGPChartAxis::TMT_NO_TICKS) ? m_nMajorTickMarkLen : m_nMinorTickMarkLen;
			TickMarkType tmt = (nStep == 0 && m_majorTickMarkType != CBCGPChartAxis::TMT_NO_TICKS) ? m_majorTickMarkType : m_minorTickMarkType;
	
			CBCGPPoint ptStart = pt;
			CBCGPPoint ptEnd = pt;

			switch (tmt)
			{
			case CBCGPChartAxis::TMT_INSIDE:
				ptStart.x -= nTickMarkLen * sin(m_dblTickAngle); 
				ptStart.y += nTickMarkLen * cos(m_dblTickAngle); 
				break;
			case CBCGPChartAxis::TMT_OUTSIDE:
				ptStart.x += nTickMarkLen * sin(m_dblTickAngle); 
				ptStart.y -= nTickMarkLen * cos(m_dblTickAngle); 
				break;
			case CBCGPChartAxis::TMT_CROSS:
				ptStart.x -= nTickMarkLen * sin(m_dblTickAngle); 
				ptStart.y += nTickMarkLen * cos(m_dblTickAngle); 
				ptEnd.x += nTickMarkLen * sin(m_dblTickAngle); 
				ptEnd.y -= nTickMarkLen * cos(m_dblTickAngle); 
				break;
			}

			m_pChart->OnDrawTickMark(pGM, ptStart, ptEnd, m_axisLineFormat, dblCurValue, nTickMarkLen == m_nMajorTickMarkLen);

			nStep++;
			dblCurValue += m_minorTickMarkType != CBCGPChartAxis::TMT_NO_TICKS ? m_dblMinorUnit : m_dblMajorUnit;

			if (nStep == m_nMinorUnitCount || m_minorTickMarkType == CBCGPChartAxis::TMT_NO_TICKS)
			{
				nStep = 0;
			}
		}
	}

	OnDrawAxisLine(pGM);
}
//----------------------------------------------------------------------------//
void CBCGPChartTernaryAxis::OnDrawMajorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagram*/)
{
	ASSERT_VALID(this);

	if (m_pChart == NULL || !IsAxisVisible() || !m_bShowMajorGridLines || m_dblMajorUnit == 0.)
	{
		return;
	}

	for (double dblCurValue = 0; dblCurValue <= m_dblMaximum; dblCurValue += m_dblMajorUnit)
	{
		DrawGridLine(pGM, dblCurValue, TRUE);
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartTernaryAxis::OnDrawMinorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& /*rectDiagram*/)
{
	ASSERT_VALID(this);

	if (m_pChart == NULL || !IsAxisVisible() || !m_bShowMinorGridLines || m_dblMajorUnit == 0.)
	{
		return;
	}

	int nStep = 0;

	for (double dblCurValue = 0; dblCurValue <= m_dblMaximum; dblCurValue += m_dblMinorUnit)
	{
		if (nStep != 0 || !m_bShowMajorGridLines)
		{
			// always draw minor grid lines
			DrawGridLine(pGM, dblCurValue, FALSE);
		}

		nStep++;

		if (nStep == m_nMinorUnitCount || m_minorTickMarkType == CBCGPChartAxis::TMT_NO_TICKS)
		{
			nStep = 0;
		}
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartTernaryAxis::DrawGridLine(CBCGPGraphicsManager* pGM, double dblCurValue, BOOL bIsMajor)
{
	CBCGPChartData dataStart;
	dataStart.SetValue(0., CBCGPChartData::CI_Y);
	dataStart.SetValue(0., CBCGPChartData::CI_Y1);
	dataStart.SetValue(0., CBCGPChartData::CI_Y2);

	CBCGPChartData dataEnd;
	dataEnd.SetValue(0., CBCGPChartData::CI_Y);
	dataEnd.SetValue(0., CBCGPChartData::CI_Y1);
	dataEnd.SetValue(0., CBCGPChartData::CI_Y2);

	switch (m_axisDefaultPosition)
	{
	case CBCGPChartAxis::ADP_RIGHT:
		dataStart.SetValue(dblCurValue, CBCGPChartData::CI_Y);
		dataStart.SetValue(m_dblMaximum - dblCurValue, CBCGPChartData::CI_Y2);

		dataEnd.SetValue(dblCurValue, CBCGPChartData::CI_Y);
		dataEnd.SetValue(m_dblMaximum - dblCurValue, CBCGPChartData::CI_Y1);
		break;

	case CBCGPChartAxis::ADP_LEFT:
		dataStart.SetValue(dblCurValue, CBCGPChartData::CI_Y1);
		dataStart.SetValue(m_dblMaximum - dblCurValue, CBCGPChartData::CI_Y);

		dataEnd.SetValue(dblCurValue, CBCGPChartData::CI_Y1);
		dataEnd.SetValue(m_dblMaximum - dblCurValue, CBCGPChartData::CI_Y2);
		break;

	case CBCGPChartAxis::ADP_BOTTOM:
		dataStart.SetValue(dblCurValue, CBCGPChartData::CI_Y2);
		dataStart.SetValue(m_dblMaximum - dblCurValue, CBCGPChartData::CI_Y1);

		dataEnd.SetValue(dblCurValue, CBCGPChartData::CI_Y2);
		dataEnd.SetValue(m_dblMaximum - dblCurValue, CBCGPChartData::CI_Y);
		break;
	}

	CBCGPPoint ptStart = PointFromChartData(dataStart);
	CBCGPPoint ptEnd = PointFromChartData(dataEnd);

	m_pChart->OnDrawGridLine(pGM, ptStart, ptEnd, this, dblCurValue, bIsMajor ? m_majorGridLineFormat : m_minorGridLineFormat, bIsMajor);
}
//----------------------------------------------------------------------------//
void CBCGPChartTernaryAxis::OnDrawAxisLabels(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram)
{
	ASSERT_VALID(this);

	m_rectAxisLabels.SetRectEmpty();

	if (m_pChart == NULL || GetAxisLabelType() == CBCGPChartAxis::ALT_NO_LABELS || !IsAxisVisible() || m_dblMajorUnit == 0.)
	{
		return;
	}

	CBCGPChartData data;
	data.SetValue(0., CBCGPChartData::CI_Y);
	data.SetValue(0., CBCGPChartData::CI_Y1);
	data.SetValue(0., CBCGPChartData::CI_Y2);

	double dblDistance = GetLabelDistance();

	for (double dblCurValue = 0; dblCurValue <= m_dblMaximum; dblCurValue += m_dblMajorUnit)
	{
		switch (m_axisDefaultPosition)
		{
		case CBCGPChartAxis::ADP_RIGHT:
			data.SetValue(dblCurValue, CBCGPChartData::CI_Y);
			data.SetValue(m_dblMaximum - dblCurValue, CBCGPChartData::CI_Y2);
			break;
		case CBCGPChartAxis::ADP_LEFT:
			data.SetValue(dblCurValue, CBCGPChartData::CI_Y1);
			data.SetValue(m_dblMaximum - dblCurValue, CBCGPChartData::CI_Y);
			break;
		case CBCGPChartAxis::ADP_BOTTOM:
			data.SetValue(dblCurValue, CBCGPChartData::CI_Y2);
			data.SetValue(m_dblMaximum - dblCurValue, CBCGPChartData::CI_Y1);
			break;
		}

		CString strLabel;
		GetDisplayedLabel(dblCurValue, strLabel);

		CBCGPPoint pt = PointFromChartData(data);


		pt.x += (dblDistance + m_szMaxLabelSize.cx / 2) * sin(m_dblTickAngle);
		pt.y -= (dblDistance + m_szMaxLabelSize.cy / 2) * cos(m_dblTickAngle);

		CBCGPRect rectLabel(pt, pt);
		rectLabel.InflateRect(m_szMaxLabelSize.cx / 2, m_szMaxLabelSize.cy / 2);
		
		m_pChart->OnDrawAxisLabel(pGM, dblCurValue, strLabel, this, rectLabel, m_rectAxisLabels, rectDiagram);

		m_rectAxisLabels.UnionRect(m_rectAxisLabels, rectLabel);
	}
}
//----------------------------------------------------------------------------//
void CBCGPChartTernaryAxis::OnFillUnitIntervals(CBCGPGraphicsManager* /*pGM*/, const CBCGPRect& /*rectDiagram*/)
{

}
//----------------------------------------------------------------------------//
double CBCGPChartTernaryAxis::ValueFromPoint(const CBCGPPoint& pt, CBCGPChartAxis::RoundType roundType)
{
	return max(0, min(100, fabs(CBCGPChartAxis::ValueFromPoint(pt, roundType))));
}