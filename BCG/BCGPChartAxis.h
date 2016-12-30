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

#if !defined(AFX_BCGPCHARTAXIS_H__A644E7E3_DFE7_4550_936D_7D786F991BE6__INCLUDED_)
#define AFX_BCGPCHARTAXIS_H__A644E7E3_DFE7_4550_936D_7D786F991BE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPChartFormat.h"
#include "BCGPChartSeries.h"
#include "bcgpmath.h"
#include <Float.h>

struct BCGPChartHitInfo;
class CBCGPChartVisualObject;

struct BCGCBPRODLLEXPORT CBCGPChartAxisScaleBreak
{
	enum AxisScaleBreakStyle
	{
		ASBS_CONTINUOUS,
		ASBS_EMPTY,
		ASBS_LINE,
		ASBS_BOX,
		ASBS_WAVE,
		ASBS_SAWTOOTH
	};

	CBCGPChartAxisScaleBreak()
	{
		m_dblStart = 0;
		m_dblEnd = 0;
		m_dblOffsetPercent = 60.0;
		m_nGap = 8;
		m_scaleBreakStyle = CBCGPChartAxisScaleBreak::ASBS_LINE;
		m_scaleBreakFormat.m_outlineFormat.m_dblWidth = -1.0;
	}

	double	m_dblStart;
	double	m_dblEnd;

	double	m_dblOffsetPercent;
	int		m_nGap;

	AxisScaleBreakStyle	m_scaleBreakStyle;
	BCGPChartFormatArea	m_scaleBreakFormat;
};

struct BCGCBPRODLLEXPORT CBCGPChartAxisScalePart
{
	CBCGPChartAxisScalePart()
	{
		m_dblMinimum = 0;
		m_dblMaximum = 0;
		m_dblMinimumDisplayed = DBL_MAX;
		m_dblMaximumDisplayed = DBL_MAX;
		m_dblMajorUnit = 0;
		m_dblScreenStart = 0;
		m_dblScreenEnd = 0;
		m_szMaxLabelSize.cx = 16;
		m_szMaxLabelSize.cy = 16;
		m_dblMinorUnit = 0;
		m_dblScreenStartPercent = 0;
		m_dblScreenEndPercent = 100;
		m_nGap = 8;
		m_bValid = FALSE;
		m_nValuesCount = 1;
	}

	BOOL	m_bValid;

	double	m_dblMinimum;
	double	m_dblMaximum;
	double	m_dblScreenStartPercent;
	double	m_dblScreenEndPercent;
	int		m_nGap;

	CBCGPChartAxisScaleBreak::AxisScaleBreakStyle	m_scaleBreakStyle;
	BCGPChartFormatArea	m_scaleBreakFormat;

	double m_dblMinimumDisplayed;
	double m_dblMaximumDisplayed;

	double	m_dblMajorUnit;
	double  m_dblMinorUnit;
	double	m_dblScreenStart;
	double	m_dblScreenEnd;

	int		m_nValuesCount;

	CBCGPSize	m_szMaxLabelSize;

	double GetSize() const {return fabs(m_dblScreenEnd - m_dblScreenStart);}
	double GetRange() const {return GetMaximumDisplayed() - GetMinimumDisplayed();}
	double GetMinimumDisplayed() const
	{
		if (m_dblMinimumDisplayed == DBL_MAX)
		{
			return m_dblMinimum;
		}
		
		return m_dblMinimumDisplayed;
	}
	double GetMaximumDisplayed() const
	{
		if (m_dblMaximumDisplayed == DBL_MAX)
		{
			return m_dblMaximum;
		}

		return m_dblMaximumDisplayed;
	}

	int GetNumberOfUnitsToDisplay(BOOL bVert) const
	{
		double dblMajorUnitSize = bVert ? m_szMaxLabelSize.cy * 1.1 : m_szMaxLabelSize.cx * 1.1;
		return (int)(GetSize() / dblMajorUnitSize);
	}
};

struct BCGCBPRODLLEXPORT CBCGPChartScaleBreakOptions
{
	CBCGPChartScaleBreakOptions()
	{
		m_bAutomatic = TRUE;
		m_nMaxCount = 2;
		m_nGap = 8;
		m_dblValueThresholdPercent = 25;
		m_dblFirstBreakOffsetPercent = 70;
		m_style = CBCGPChartAxisScaleBreak::ASBS_LINE;
		m_bStartFromZero = TRUE;
		m_bMergeBreaksFromTail = TRUE;
	}

	void Verify()
	{
		if (m_nMaxCount < 1)
		{
			m_nMaxCount = 1;
		}

		if (m_nGap < 0)
		{
			m_nGap = 0;
		}

		m_dblValueThresholdPercent = bcg_clamp(m_dblValueThresholdPercent, 1., 99.);
		m_dblFirstBreakOffsetPercent = bcg_clamp(m_dblFirstBreakOffsetPercent, 10., 90.);
	}

	BOOL	m_bStartFromZero;
	BOOL	m_bAutomatic;
	int		m_nMaxCount;
	BOOL	m_bMergeBreaksFromTail;
	int		m_nGap;
	double	m_dblValueThresholdPercent;
	double	m_dblFirstBreakOffsetPercent;

	CBCGPChartAxisScaleBreak::AxisScaleBreakStyle	m_style;
	BCGPChartFormatArea	m_format;
}; 

class BCGCBPRODLLEXPORT CBCGPChartAxis : public CObject
{
	friend class CBCGPChartSeries;
	friend class CBCGPChartStockSeries;
	friend class CBCGPChartBarSeries;
	friend class CBCGPChartVisualObject;	

	DECLARE_DYNCREATE(CBCGPChartAxis)

public:
	enum TickMarkType
	{
		TMT_NO_TICKS,
		TMT_INSIDE,
		TMT_OUTSIDE,
		TMT_CROSS
	};

	enum AxisLabelsType
	{
		ALT_NO_LABELS,
		ALT_NEXT_TO_AXIS,
		ALT_HIGH,
		ALT_LOW
	};

	enum CrossType
	{
		CT_AUTO,
		CT_MAXIMUM_AXIS_VALUE,
		CT_MINIMUM_AXIS_VALUE,
		CT_AXIS_VALUE,
		CT_IGNORE,
		CT_FIXED_DEFAULT_POS
	};

	enum AxisDefaultPosition
	{
		ADP_LEFT,
		ADP_BOTTOM,
		ADP_RIGHT,
		ADP_TOP,
		ADP_DEPTH_BOTTOM,
		ADP_DEPTH_TOP
	};

	enum RoundType
	{
		RT_EXACT,
		RT_FLOOR,
		RT_CEIL,
		RT_ROUND
	};

protected:
	CBCGPChartAxis();
	CBCGPChartAxis(int nAxisID, AxisDefaultPosition position, CBCGPChartVisualObject* pChartCtrl);

	virtual void CommonInit();

public:
	virtual ~CBCGPChartAxis(void);

	BOOL					m_bInitialized;
	int						m_nAxisID;
	AxisDefaultPosition		m_axisDefaultPosition;

// Attributes
	BCGPChartFormatLine		m_axisLineFormat;
	CBCGPBrush				m_brInterval;
	CBCGPBrush				m_brInterval3D;

	double					m_dblLabelDistance; // label distance from left and right (tick marks) bounds of axis
	double					m_dblAxisNameDistance3D;

	BOOL					m_bDisplayAxisName;
	CString					m_strAxisName;
	CBCGPRect				m_rectAxisName;
	BCGPChartFormatLabel	m_axisNameFormat;

	CString					m_strMaxDisplayedLabel;
	CBCGPSize				m_szMaxFixedLabelSize;
	CBCGPSize				m_szMaxLabelSize;
	CBCGPRect				m_rectAxisLabels;
	BCGPChartFormatLabel	m_axisLabelsFormat;	

	TickMarkType			m_majorTickMarkType;
	TickMarkType			m_minorTickMarkType;
	AxisLabelsType			m_axisLabelType;

	BCGPChartFormatLine		m_majorGridLineFormat;
	BCGPChartFormatLine		m_minorGridLineFormat;

	CrossType				m_crossType;
	double					m_dblCrossOtherAxisValue;

	CString					m_strDataFormat;

	BOOL					m_bReverseOrder;
	BOOL					m_bEnableRotateFont;

	BOOL					m_bVisible;
	BOOL					m_bAlwaysVisible;
	UINT					m_nMaxTotalLines;

	BOOL					m_bFormatAsDate;

	BOOL					m_bNewMajorUnitBehavior; 

	CBCGPPoint				m_ptPanStart;

public:
	double GetMinDisplayedValue(BOOL bEnsureExactForFixedInterval = FALSE) const 
	{
		if (IsFixedIntervalWidth() && bEnsureExactForFixedInterval)
		{
			return ValueFromPointInternal(m_ptAxisStart, CBCGPChartAxis::RT_EXACT);
		}
		return m_dblMinDisplayedValue;
	}
	double GetMaxDisplayedValue(BOOL bEnsureExactForFixedInterval = FALSE) const 
	{
		if (IsFixedIntervalWidth() && bEnsureExactForFixedInterval)
		{
			return ValueFromPointInternal(m_ptAxisEnd, CBCGPChartAxis::RT_EXACT);
		}
		return m_dblMaxDisplayedValue;
	}
	int	GetDisplayedLinesCount() const {return m_nDisplayedLines;}

	void SetMajorTickMarkLen(int nLen);
	void SetMinorTickMarkLen(int nLen) {m_nMinorTickMarkLen = nLen;}

	double	GetMajorTickMarkLen(BOOL bScaled = FALSE) const;
	double	GetMinorTickMarkLen(BOOL bScaled = FALSE) const;
	double  GetLabelDistance() const;
	
	virtual BOOL IsAxisVisible() const;
	virtual BOOL HasSeries() const;
	virtual void Reset();
	virtual void SwapDirection(BOOL bAdjustGradientAngles);
	virtual BOOL HitTest(const CBCGPPoint& pt, BCGPChartHitInfo* pHitInfo, DWORD dwHitInfoFlags);
	virtual BOOL HitGridLinesTest(const CBCGPPoint& pt, BCGPChartHitInfo* pHitInfo);
	virtual BOOL HitTestAxisShape(const CBCGPPoint& pt) const;
	virtual BOOL HitTestAxisLables(const CBCGPPoint& pt) const;
	virtual CBCGPRect GetAxisRect(BOOL bCombineWithLabels = TRUE, BOOL bCombineWithTickMarks = TRUE, 
		BOOL bCombineWithScrollBar = TRUE) const;

	void SetVertical(BOOL bVert = TRUE);
	virtual BOOL IsVertical() const {return m_bIsVertical;}

	void SetLogScale(BOOL bSet, double dblLogScaleBase);
	virtual BOOL IsLogScale() const {return m_bLogScale;}
	double GetLogScaleBase() const {return m_dblLogScaleBase;} 

	double GetDisplayUnits() const {return m_dblDisplayUnits;}
	void SetDisplayUnits(double dblUnits = 0.) {m_dblDisplayUnits = dblUnits;}

	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld);

	virtual void AdjustDiagramArea(CBCGPRect& rectDiagramArea, const CBCGPRect& rectPlotArea);
	virtual void CalcMajorMinorUnits(CBCGPGraphicsManager* pGM, UINT nUnitsToDisplay = -1);
	virtual void CalcMinMaxDisplayedValues(BOOL bCalcMinDisplayedValue, double dblApproximation = 1.06);
	virtual void CalcMaxLabelSize(CBCGPGraphicsManager* pGM);
	virtual void UpdateAxisPos(const CBCGPRect& rectDiagramArea);
	virtual void CalcAxisPos(const CBCGPRect& rectDiagram, BOOL bStoreAxisPos = TRUE);
	virtual void CalcLabelsRect(CBCGPRect& rectDiagramArea);
	virtual BOOL CalcMinMaxValues();
	virtual void CalcNameRect(CBCGPGraphicsManager* pGM, CBCGPRect& rectPlotArea, const CRect& rectChartArea, BOOL bReposOnly);

	virtual void GetFormatString(CString& strFormat, double dblValue);
	virtual void GetFormattedValue(double dblValue, CString& strValue);
	virtual void GetDisplayedLabel(double dblValue, CString& strLabel);
	virtual void OnCalcDefaultTextAlignment(double dblCurrValue, const CBCGPRect& rectLabel, const CBCGPRect& rectDiagram, CBCGPTextFormat& textFormat);
	
	virtual double GetCrossValuePos(BOOL bMapToDiagramArea = FALSE);
	double GetNonIgnoredCrossValue() const;
	virtual double PointFromValue(double dblValue, BOOL bForceValueOnThickMark, BOOL bLogValue = TRUE);
	double PointFromValueOptimized3D(double dblValue, const CBCGPPoint& ptAxisStart,
		const CBCGPPoint& ptAxisEnd, double dblAxisSize, BOOL bIsDisplayDataBetweenTickMarks, 
		CBCGPChartData::ComponentIndex ci, BOOL bIsLogScale);
	virtual double ValueFromPoint(const CBCGPPoint& pt, CBCGPChartAxis::RoundType roundType = CBCGPChartAxis::RT_EXACT);
	virtual double ValueFromPointByScrollRange(const CBCGPPoint& pt, CBCGPChartAxis::RoundType roundType = CBCGPChartAxis::RT_EXACT);
	virtual double ScaleValue(double dblValue);

	virtual CBCGPPoint PointFromValue3D(double dblValue, BOOL bForceValueOnThickMark, BOOL bForceValueOnWall, BOOL bLogValue = TRUE);

	virtual CBCGPChartAxis* GetOppositeAxis(){return NULL;}
	virtual CBCGPChartAxis* GetPerpendecularAxis() const {return NULL;}
	virtual void GetPerpendecularAxes(CBCGPChartAxis*& pPrimary, CBCGPChartAxis*& pSecondary)
	{
		pPrimary = NULL;
		pSecondary = NULL;
	}
	virtual CBCGPChartData::ComponentIndex GetComponentIndex() const {return CBCGPChartData::CI_Y;}

	virtual BOOL IsDisplayDataBetweenTickMarks() const {return FALSE;}

	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawMajorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawMinorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawAxisLabels(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnFillUnitIntervals(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawScrollBar(CBCGPGraphicsManager* pGM);

	virtual void OnDrawScaleBreaks(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);

	virtual void OnDrawAxisName3D(CBCGPGraphicsManager* pGM);
	virtual void OnDrawAxisOnThickWall(CBCGPEngine3D* pEngine, CBCGPChartSide3D* pSide);
	virtual void OnDrawInterlaceOnThickWall(CBCGPEngine3D* pEngine, CBCGPChartSide3D* pSide, BOOL bIsNearWall);
	virtual void OnDrawGridLinesOnThickWall(CBCGPEngine3D* pEngine, CBCGPChartSide3D* pSide, BOOL bIsNearWall);

	void ShowMajorGridLines(BOOL bShow = TRUE);
	void ShowMinorGridLines(BOOL bShow = TRUE);
	BOOL IsMajorGridLinesVisible() const {return m_bShowMajorGridLines;};
	BOOL IsMinorGridLinesVisible() const {return m_bShowMinorGridLines;}

	virtual CBCGPRect GetBoundingRect() const;
	virtual CBCGPRect GetBoundingRects(CArray<CBCGPRect, CBCGPRect>& arRects) const;

	void EnableMajorUnitIntervalInterlacing(BOOL bEnable = TRUE)
	{
		m_bFillMajorUnitInterval = bEnable;
	}

	void SetMinorUnitCount(int nCount);
	int GetMinorUnitCount() const {return m_nMinorUnitCount;}

	BOOL IsMajorUnitIntervalInterlacingEnabled() const {return m_bFillMajorUnitInterval;}
	void SetFirstInterlacedIntervalIndex(int nIndex = 1)
	{
		m_nFirstIntervalToFill = (nIndex < 0) ? 0 : nIndex;
	}
	int GetFirstInterlacedIntervalIndex() const {return m_nFirstIntervalToFill;}

	void SetInterlaceStep(int nStep = 2)
	{
		m_nUnitFillStep = (nStep < 1) ? 1 : nStep;
		
	}
	int GetInterlaceStep() const {return m_nUnitFillStep;}

	int GetMinDisplayedLines() const
	{
		return m_nMinDisplayedLines;
	}

	void SetMinDisplayedLines(int nMinDisplayedLines)
	{
		m_nMinDisplayedLines = nMinDisplayedLines;
	}

	void SetAutoDisplayRange();
	virtual void SetFixedDisplayRange(double dblMinimum, double dblMaximum, double dblMajorUnit = 0.);
 	virtual BOOL Zoom(int nMagnifier, const CBCGPPoint& ptZoomCenter = CBCGPPoint());
	virtual void UnZoom();
	BOOL IsZoomed() const {return m_bZoomed;}

	void EnableZoom(BOOL bEnable = TRUE) {m_bEnableZoom = bEnable;}
	virtual BOOL IsZoomEnabled() const {return m_bEnableZoom && !m_bLogScale;}
	void EnableScroll(BOOL bEnable = TRUE) {m_bEnableScroll = bEnable;} 
	virtual BOOL IsScrollEnabled() const {return m_bEnableScroll && !m_bLogScale;}
	virtual BOOL CanBeScrolled() const;
	void SetMaxZoomInFactor(double dblValue = 1000);
	double GetMaxZoomInFactor() const {return m_dblMaxZoomInFactor;}
	void EnableNegativeValuesForFixedInterval(BOOL bEnable = TRUE) {m_bEnableNegativeValuesForFixedInterval = bEnable;}
	virtual BOOL IsNegativeValuesForFixedIntervaleEnabled() const 
	{
		return m_bEnableNegativeValuesForFixedInterval && !IsIndexedSeries();
	}

	void EnableIndependentZoom(BOOL bEnable = TRUE) {m_bIndependentZoom = bEnable;}
	virtual BOOL IsIndependentZoomEnabled() const {return m_bIndependentZoom;}

	void SetScrollRange(double dblMinValue = -DBL_MAX, double dblMaxValue = DBL_MAX, BOOL bForce = TRUE);
	double GetMinScrollValue() const {return m_dblMinScrollValue;}
	double GetMaxScrollValue() const {return m_dblMaxScrollValue;}

	BOOL SetThumbRect(const CBCGPPoint& ptHit, BOOL bLeftGrip, double dblOffset);
	void EnableThumbGrip(BOOL bEnable = TRUE);
	BOOL IsThumbGripEnabled() const {return m_bIsThumbGripEnabled;}

	void ShowScrollBar(BOOL bShow = TRUE, BOOL bRedraw = TRUE);
	virtual BOOL CanShowScrollBar() const;
	CBCGPRect GetScrollBarRect() const {return m_rectScrollBar;}

	void SetAlwaysShowScrollBar(BOOL bShow = TRUE, BOOL bRedraw = TRUE);
	virtual BOOL IsAlwaysShowScrollBar() const {return m_bAlwaysShowScrollBar;} 

	void SetScrollBarSize(double dblSize);
	double GetScrollBarSize(BOOL bScaled = FALSE) const;

	void SetMinThumbSize(double dblSize);
	double GetMinThumbSize(BOOL bScaled = FALSE) const;

	virtual void SetFixedMinimumDisplayValue(double dblMinimum, BOOL bByZoom = FALSE);
	virtual void SetFixedMaximumDisplayValue(double dblMaximum, BOOL bByZoom = FALSE);
	void NormalizeDisplayRange();

	virtual BOOL Scroll(double dblUnitsToScroll, BOOL bUnitsInPixels);
	virtual BOOL ScrollTo(double dblValue);

	virtual BOOL IsFixedDisplayRange() const {return m_bFixedRange;}
	virtual BOOL IsFixedMinimumDisplayValue() const {return m_bFixedMinimum;}
	virtual BOOL IsFixedMaximumDisplayValue() const {return m_bFixedMaximum;}

	virtual BOOL CanDisplayDataBetweenTickMarks() const {return TRUE;}
	virtual BOOL SetDisplayDataBetweenTickMarks(BOOL bSet = TRUE, BOOL bRedraw = TRUE);

	void GetAxisPos(CBCGPPoint& ptStart, CBCGPPoint& ptEnd) const 
	{
		ptStart = m_ptAxisStart; 
		ptEnd = m_ptAxisEnd;
	}

	void GetAxisPos3D(CBCGPPoint& ptStart, CBCGPPoint& ptEnd) const 
	{
		ptStart = m_ptAxisStart3D; 
		ptEnd = m_ptAxisEnd3D;
	}

	virtual double GetAxisSize(BOOL bLogical = FALSE) const;

	double GetAxisUnitSize();
	virtual double GetBaseUnitSize() const 
	{
		if (m_bFormatAsDate)
		{
			return m_dblMajorUnit;
		}

		return 1.;
	}

	void SetMinimumValue(double dblVal)
	{
		if (!IsFixedMinimumDisplayValue())
		{
			m_dblMinimum = dblVal;
		}
	}

	void SetMaximumValue(double dblVal)
	{
		if (!IsFixedMaximumDisplayValue())
		{
			m_dblMaximum = dblVal;
		}
	}

	void SetCustomPerpendicularAxis(CBCGPChartAxis* pAxis)
	{
		if (pAxis != NULL)
		{
			ASSERT_VALID(pAxis);
			ASSERT_KINDOF(CBCGPChartAxis, pAxis);
		}

		m_pPerpendicularAxis = pAxis;
	}

	virtual void SetFixedIntervalWidth(int nWidth, int nValuesPerInterval = 1);
	void SetAutoIntervalWidth()
	{
		m_bIsFixedIntervalWidth = FALSE;
	}

	BOOL IsFixedIntervalWidth() const {return m_bIsFixedIntervalWidth;}
	int  GetValuesPerInterval() const {return m_nValuesPerInterval;}
	int  GetFixedIntervalWidth() const {return m_nFixedIntervalWidth;}
	double GetFixedIntervalWidthScaled() const;
	void EnableStretchFixedIntervals(BOOL bEnable = TRUE) {m_bStretchFixedIntervals = bEnable;}
	BOOL IsStretchFixedIntervalsEnabled() const {return m_bStretchFixedIntervals;}

	void SetRightOffsetInPixels(int nValuesCount = 0);
	int  GetRightOffsetInPixels() const {return m_nRightOffset;}
	int  GetRightOffsetAsNumberOfValues() const;

	void SetFixedUnitCount(int nCount, BOOL bSet);
	BOOL IsFixedUnitCount() const {return m_bIsFxedUnitCount;}
	int  GetFixedUnitCount() const {return m_nFixedUnitCount;}

	virtual void SetFixedMajorUnit(double dblMajorUnit, BOOL bSet = TRUE);
	BOOL IsFixedMajorUnit() const {return m_bFixedMajorUnit;}

	void SetMinAllowedMajorUnit(double dblMinAllowedMajorUnit);
	double GetMinAllowedMajorUnit() const {return m_dblMinAllowedMajorUnit;}

	double GetMajorUnit() const {return m_dblMajorUnit;}
	double GetMinorUnit() const {return m_dblMinorUnit;}

	BOOL IsAutoMajorUnitEnabled() const {return m_bIsAutoMajorUnit;}
	void EnableAutoMajorUnit(BOOL bEnable = TRUE) {m_bIsAutoMajorUnit = bEnable;}

	virtual int GetMaxTotalLines() const {return m_nMaxTotalLines;}
	int GetMajorUnitSpacing() const {return m_nMajorUnitSpacing;}
	double GetMajorUnitSpacingScaled() const;
	void SetMajorUnitSpacing(int nSpacing = 0) {m_nMajorUnitSpacing = abs(nSpacing);}

	CBCGPChartAxis* GetCustomPerpendicularAxis() const {return m_pPerpendicularAxis;}

	void UsePerpendicularAxisForZoning(BOOL bUse) {m_bUsePerpAxisForZoning = bUse;}
	BOOL IsUsePerpendicularAxisForZoning() const {return m_bUsePerpAxisForZoning;}

	virtual void SetIndexedSeries(CBCGPChartSeries* pSeries);
	CBCGPChartSeries* GetIndexedSeries() const {return m_pIndexedSeries;}
	virtual BOOL IsIndexedSeries() const {return m_pIndexedSeries != NULL && m_pIndexedSeries->IsIndexMode();}

	virtual BOOL IsComponentXSet() const {return IsAxisComponentSet();}
	
	virtual CBCGPChartAxis* Split(double dblPercent, int nGapInPixels = 10, CRuntimeClass* pNewAxisRTC = NULL,
		BOOL bCustomAxisAtTop = TRUE);
	void SetAxisOffsets(double dblBottomOffset, double dblTopOffset, int nGapInPixels = 10);
	double GetBottomOffset() const {return m_dblBottomOffset;}
	double GetTopOffset() const {return m_dblTopOffset;}
	int GetAxisGap() const {return m_nAxisGap;}
	double GetAxisGapScaled() const; 
	
	CBCGPChartAxis*	GetSplitTop() const {return	m_pSplitTop;}
	CBCGPChartAxis*	GetSplitBottom() const	{return m_pSplitBottom;}
	void RemoveCustomAxis();

	virtual BOOL CanUpdateAxisSizeByOffset(const CBCGPPoint& ptOffset, BOOL bTop) const;
	BOOL UpdateAxisSizeByOffset(const CBCGPPoint& pt, BOOL bTop, const CBCGPRect& rectDiagram);
	BOOL UpdateSplitAxisSizeByPercent(double dblPercent, BOOL bTop);

	BOOL IsCustomOrResized() const;

	void UseApproximation(BOOL bUse) {m_bUseApproximation = bUse;}

	BOOL IsLabelInterlacingEnabled() const {return m_bEnableInterlaceLabels;}
	void EnableLabelInterlacing(BOOL bEnable = TRUE);

	BOOL IsPreventLabelOverlapIn3D() const {return TRUE;}
	void SetPreventLabelOverlapIn3D(BOOL bSet = TRUE) {m_bIsPreventLabelOverlapIn3D = bSet;}

	// Scale breaks support
	void EnableScaleBreaks(BOOL bEnable, CBCGPChartScaleBreakOptions* pOptions = NULL, BOOL bGenerateBreaks = TRUE);
	CBCGPChartScaleBreakOptions GetScaleBreaksOptions() const {return m_scaleBreakOptions;}
	virtual BOOL IsScaleBreakEnabled() const;
	BOOL CanUseScaleBreaks() const 
	{
		return IsScaleBreakEnabled() && m_scaleParts.GetCount() > 0 && m_scaleParts.GetHead().m_dblMajorUnit != 0 && 
			GetValidPartCount() > 1;
	}
	BOOL AddScaleBreak(const CBCGPChartAxisScaleBreak& scaleBreak, BOOL bCreateScaleParts = TRUE);
	void UpdateScaleBreaks(const CBCGPChartValue& val, CBCGPChartSeries* pSeries);
	virtual void GenerateScaleBreaks();
	void RemoveAllBreaks();
	int  GetScaleBreakCount() const;
	BOOL IsValueWithinScaleBreak(double dblValue) const;
	virtual double PointFromValueOnAxisWithBreak(double dblValue, BOOL bForceValueOnThickMark);
	virtual double ValueFromPointOnAxisWithBreak (const CBCGPPoint& pt, CBCGPChartAxis::RoundType roundType) const;
	BOOL ScaleBreakFromValue(double dblValue, CBCGPChartAxisScaleBreak& sb, int& nIndex) const;
	BOOL ScaleBreakFromPoint(const CBCGPPoint& pt, CBCGPChartAxisScaleBreak& sb, int& nIndex) const;
	BOOL ScalePartFromValue(double dblValue, CBCGPChartAxisScalePart& part) const;
	BOOL ScalePartFromPoint(const CBCGPPoint& pt, CBCGPChartAxisScalePart& part, int& nIndex) const;
	int GetValidPartCount() const;

	// 3D interface
	virtual void CalcAxisPos3D(const CBCGPRect& rectDiagram, BOOL bUpdateCrossValue);

	void SetThumbnailMode(BOOL bSet = TRUE);
	BOOL IsThumbnailMode() const
	{
		return m_bThumbnailMode;
	}

	void SetNormalizeValuesInAxis(BOOL bSet = TRUE);
	BOOL IsNormalizeValuesInAxis() const
	{
		return m_bNormalizeValuesInAxis;
	}

	AxisLabelsType GetAxisLabelType() const
	{
		return m_bConnectedToDataTable ? CBCGPChartAxis::ALT_LOW : m_axisLabelType;
	}

protected:
	BOOL				m_bThumbnailMode;
	BOOL				m_bConnectedToDataTable;

	double				m_dblMaxDisplayedValue;
	double				m_dblMinDisplayedValue;
	int					m_nDisplayedLines;
	int					m_nMinDisplayedLines;

	BOOL				m_bShowMajorGridLines;
	BOOL				m_bShowMinorGridLines;
	
	int					m_nMinorUnitCount;

	BOOL				m_bLogScale;
	double				m_dblLogScaleBase;

	double				m_dblDisplayUnits;

	int					m_nMajorTickMarkLen;
	int					m_nMinorTickMarkLen;

	CBCGPPoint			m_ptAxisStart;
	CBCGPPoint			m_ptAxisEnd;

	CBCGPPoint			m_ptPrevAxisStart;
	CBCGPPoint			m_ptPrevAxisEnd;

	BOOL				m_bDisplayDataBetweenTickMarks;

	int					m_nColumnOverlapPercent;
	int					m_nColumnDistancePercent;

	BOOL				m_bEmptySeries;
	CBCGPChartSeries*	m_pIndexedSeries;

	BOOL				m_bIsVertical;
	BOOL				m_bFixedRange;
	BOOL				m_bFixedMinimum;
	BOOL				m_bFixedMaximum;
	BOOL				m_bNormalizeValuesInAxis;

	BOOL				m_bIsFixedIntervalWidth;
	int					m_nFixedIntervalWidth;
	int					m_nValuesPerInterval;
	int					m_nRightOffset;
	BOOL				m_bStretchFixedIntervals;

	BOOL				m_bIsSecondaryAxis;

	double				m_dblMinimum;
	double				m_dblMaximum;
	BOOL				m_bFixedMajorUnit;

	double				m_dblMajorUnit;
	double				m_dblMinorUnit;

	BOOL				m_bIsAutoMajorUnit;

	BOOL				m_bIsFxedUnitCount;
	int					m_nFixedUnitCount;

	double				m_dblMinAllowedMajorUnit;
	int					m_nMajorUnitSpacing;

	BOOL				m_bPrevFixedRange;
	BOOL				m_bPrevFixedMinimum;
	BOOL				m_bPrevFixedMaximum;
	double				m_dblPrevMinimum;
	double				m_dblPrevMaximum;
	double				m_dblPrevMinDisplayedValue;
	double				m_dblPrevMaxDisplayedValue;
	int					m_nPrevFixedIntervalWidth;
	int					m_nPrevValuesPerInterval;
	BOOL				m_bZoomed;
	BOOL				m_bIndependentZoom;
	BOOL				m_bUnzoomOnRangeBreak;

	BOOL				m_bRoundMinMax;

	BOOL				m_bFillMajorUnitInterval;
	int					m_nFirstIntervalToFill;
	int					m_nUnitFillStep;

	BOOL				m_bEnableZoom;
	BOOL				m_bEnableScroll;

	double				m_dblMinScrollValue;
	double				m_dblMaxScrollValue;
	double				m_dblMaxZoomInFactor;

	BOOL				m_bShowScrollBar;
	BOOL				m_bAlwaysShowScrollBar;
	CBCGPRect			m_rectScrollBar;
	double				m_dblScrollBarSize; // in pixels
	double				m_dblMinThumbSize;
	BOOL				m_bIsThumbGripEnabled;
	BOOL				m_bTickMarksTopRight;
	CBCGPRect			m_rectThumb;
	CBCGPRect			m_rectLeftGrip;
	CBCGPRect			m_rectRightGrip;
	BOOL				m_bEnableNegativeValuesForFixedInterval;
	
	BOOL				m_bUseApproximation;
	
	BOOL				m_bEnableInterlaceLabels;
	BOOL				m_bIsPreventLabelOverlapIn3D;

	// custom perpendicular axis 
	CBCGPChartAxis*		m_pPerpendicularAxis;
	BOOL				m_bUsePerpAxisForZoning;

	CBCGPChartAxis*		m_pSplitTop;
	CBCGPChartAxis*		m_pSplitBottom;

	int					m_nResizeBandSize;

	double				m_dblBottomOffset; // percent from bottom
	double				m_dblTopOffset; // percent from top
	int					m_nAxisGap; // gap between axes

	CArray<CRect, CRect> m_arMajorGridLines;
	CArray<CRect, CRect> m_arMinorGridLines;

	CBCGPChartVisualObject*		m_pChart;

	double				m_dblMinDataPointDistance;

	// scale break support
	BOOL																m_bScaleBreakEnabled;
	CList<CBCGPChartAxisScaleBreak, const CBCGPChartAxisScaleBreak&>	m_scaleBreaks;
	CList<CBCGPChartAxisScalePart, const CBCGPChartAxisScalePart&>		m_scaleParts;

	CBCGPChartScaleBreakOptions											m_scaleBreakOptions;
	BOOL																m_bSavedScrollOption;
	BOOL																m_bSavedZoomOption;

	virtual void CreateScaleParts();
	void UpdateScaleParts();
	BOOL MergeScaleBreaks(double dblThreshold);
	void SetScaleBreakRanges();
	void UpdateScaleBreakPercents();
	void MergeScaleBreaksInternal(double dblThreshold, BOOL bSecondTime);
	BOOL UpdateScaleBreaksInternal(const CBCGPChartValue& val, double dblThreshold, CBCGPChartSeries* pSeries);
	BOOL IsScaleBreakValid(const CBCGPChartAxisScaleBreak& sb) const;
	BOOL IsScalePartValid(const CBCGPChartAxisScalePart& part) const;
	void CalcMaxLabelSizeForScaleParts(CBCGPGraphicsManager* pGM);
	void CalcMajorMinorUnitsForScaleParts(CBCGPGraphicsManager* pGM);
	BOOL GetNextPart(CBCGPChartAxisScalePart& partCurrent, POSITION& posScalePart, 
		double& dblUnitsInInterval, double& dblCurrValue, double& dblMaxValue);

	double ValueFromPointInternal (const CBCGPPoint& pt, CBCGPChartAxis::RoundType roundType) const;

	// 3D properties
	double				m_dblAxisSize3D;
	CBCGPPoint			m_ptAxisStart3D;
	CBCGPPoint			m_ptAxisEnd3D;

	BOOL				m_bEnableInterlaceNearWall;
	BOOL				m_bEnableInterlaceFarWall;
	BOOL				m_bEnableGridLineNearWall;
	BOOL				m_bEnableGridLineFarWall;


protected:
	void AdjustDiagramLeftSide(CBCGPRect& rectDiagramArea, const CBCGPRect& rectPlotArea, double& xOffset, double dblWidth, double dblDataTableOffset);
	double ApproximateValue(double dblValue, double dblUnit, double dblApproximation);
	double CalcMajorUnit(int nMaxLines, double dblRange, double dblOffset = 1.05);
	virtual int GetNumberOfUnitsToDisplay(UINT nUnitsToDisplay);
	virtual double UnitsInIntervalFromMajorUnitSpacing();

	double GetDoubleCorrection() const;

	BOOL IsPointOnAxis(const CBCGPPoint& pt);
	BOOL IsValueOnAxis3D(double dblVal);

	virtual void OnDrawAxisLine(CBCGPGraphicsManager* pGM);
	virtual void OnDrawScrollBar(CBCGPGraphicsManager* pGM, const CBCGPRect& rectScrollBar);

	virtual void OnBeforeSetFixedDisplayRange(double& dblMin, double& dblMax)
	{
		UNREFERENCED_PARAMETER(dblMin);
		UNREFERENCED_PARAMETER(dblMax);
	}

	virtual BOOL IsVariableLabelAngle() const {return FALSE;}
	virtual double GetVariableLabelAngle (double dblValue)
	{
		UNREFERENCED_PARAMETER(dblValue);
		return 0.;
	}

	void SaveRanges();
	void RestoreRanges();

	//void UpdateScrollRange();
	virtual void OnCalcThumbRect();
	void CheckThumbMinSize(CBCGPRect& rectThumb, double dblMinThumbSize);

	void SetAxisDefaultPosition(CBCGPChartAxis::AxisDefaultPosition defaultPos);

	double CalcLog(double dblVal, BOOL bForward = TRUE) const;

	// 3D interface
	virtual CBCGPRect	OnCalcAxisLabelRect3D(const CBCGPPoint& ptValue, const CBCGPSize& szLabel);
	virtual CBCGPPoint	GetPointOnAxisPlane3D(double dblValue, BOOL bValueOnWall) const;
	virtual void OnCalcFloorAndWallGridLinePoints(double dblCurrValue, CBCGPPoint& ptFloor, CBCGPPoint& ptWall);

	BOOL IsDiagram3D() const;
	double CalcNormalValue(double dblValue) const;

	virtual BOOL IsAxisComponentSet() const {return TRUE;}
};

class BCGCBPRODLLEXPORT CBCGPChartAxisX : public CBCGPChartAxis
{
	friend class CBCGPChartVisualObject;

	DECLARE_DYNCREATE(CBCGPChartAxisX)

protected:
	CBCGPChartAxisX();
	virtual void CommonInit();
public:
	CBCGPChartAxisX (int nAxisID, CBCGPChartAxis::AxisDefaultPosition position, CBCGPChartVisualObject* pChartCtrl);
	virtual void Reset();

public:
	CStringArray		m_arCategories;

public:
	virtual BOOL CalcMinMaxValues();
	virtual void CalcMaxLabelSize(CBCGPGraphicsManager* pGM);

	virtual CBCGPChartAxis* GetOppositeAxis();
	virtual CBCGPChartAxis* GetPerpendecularAxis() const;
	virtual void GetPerpendecularAxes(CBCGPChartAxis*& pPrimary, CBCGPChartAxis*& pSecondary);
	
	virtual BOOL IsDisplayDataBetweenTickMarks() const 
	{
		return m_bConnectedToDataTable || 
			(m_bDisplayDataBetweenTickMarks && !m_bIsAxisComponentSet && !m_bAutoMajorUnit && !IsIndexedSeries()); 
	}

	virtual CBCGPChartData::ComponentIndex GetComponentIndex() const {return CBCGPChartData::CI_X;}
	virtual void CalcMajorMinorUnits(CBCGPGraphicsManager* pGM, UINT nUnitsToDisplay = -1);
	virtual void GetDisplayedLabel(double dblValue, CString& strLabel);

	virtual BOOL CanDisplayDataBetweenTickMarks() const {return !m_bIsAxisComponentSet && !m_bOnlyFormula;}
	virtual BOOL IsComponentXSet() const {return IsAxisComponentSet();}

	virtual BOOL Scroll(double dblUnitsToScroll, BOOL bUnitsInPixels);

	virtual BOOL IsLogScale() const {return m_bLogScale && IsComponentXSet() && !m_bFormatAsDate;}

protected:
	virtual int GetNumberOfUnitsToDisplay(UINT nUnitsToDisplay);
	virtual void OnBeforeSetFixedDisplayRange(double& dblMin, double& dblMax);
	virtual BOOL IsAxisComponentSet() const {return m_bIsAxisComponentSet;}

	void CheckAxisComponent();

protected:
	int m_nMaxDataPointCount;
	BOOL m_bIsAxisComponentSet;
	BOOL m_bOnlyFormula;
	BOOL m_bAutoMajorUnit;
};

class BCGCBPRODLLEXPORT CBCGPChartAxisZ : public CBCGPChartAxisX
{
	DECLARE_DYNCREATE(CBCGPChartAxisZ)

protected:
	CBCGPChartAxisZ();
	virtual void CommonInit();
public:
	CBCGPChartAxisZ (int nAxisID, CBCGPChartAxis::AxisDefaultPosition position, CBCGPChartVisualObject* pChartCtrl);
	virtual void Reset();
	virtual CBCGPChartData::ComponentIndex GetComponentIndex() const {return CBCGPChartData::CI_Z;}

	virtual BOOL IsAxisVisible() const;
	virtual void CalcMaxLabelSize(CBCGPGraphicsManager* pGM);

	virtual CBCGPChartAxis* GetOppositeAxis();
	virtual CBCGPChartAxis* GetPerpendecularAxis() const;
	virtual void GetPerpendecularAxes(CBCGPChartAxis*& pPrimary, CBCGPChartAxis*& pSecondary);

	virtual BOOL CalcMinMaxValues();
	virtual void GetDisplayedLabel(double dblValue, CString& strLabel);

	virtual BOOL IsScaleBreakEnabled() const {return FALSE;}

	BOOL	m_bDisplaySeriesNameAsLabel;
};

class BCGCBPRODLLEXPORT CBCGPChartAxisY : public CBCGPChartAxis
{
	DECLARE_DYNCREATE(CBCGPChartAxisY)
protected:
	CBCGPChartAxisY();
	virtual void CommonInit();

public:
	CBCGPChartAxisY(int nAxisID, CBCGPChartAxis::AxisDefaultPosition position, CBCGPChartVisualObject* pChartCtrl);

public:
	virtual CBCGPChartAxis* GetOppositeAxis();
	virtual CBCGPChartAxis* GetPerpendecularAxis() const;
	virtual void GetPerpendecularAxes(CBCGPChartAxis*& pPrimary, CBCGPChartAxis*& pSecondary);
	virtual CBCGPChartData::ComponentIndex GetComponentIndex() const {return CBCGPChartData::CI_Y;}
	
	virtual BOOL CalcMinMaxValues();

	virtual void GetDisplayedLabel(double dblValue, CString& strLabel);

	virtual BOOL CanDisplayDataBetweenTickMarks() const {return FALSE;}
	virtual void CalcMinMaxDisplayedValues(BOOL bCalcMinDisplayedValue, double dblApproximation = 1.06);

	virtual int GetMaxTotalLines() const; 

protected:
	virtual int GetNumberOfUnitsToDisplay(UINT nUnitsToDisplay);	
};

class BCGCBPRODLLEXPORT CBCGPChartAxisPolarY : public CBCGPChartAxisY
{
	DECLARE_DYNCREATE(CBCGPChartAxisPolarY)
protected:
	CBCGPChartAxisPolarY();
	virtual void CommonInit();

public:
	CBCGPChartAxisPolarY(int nAxisID, CBCGPChartAxis::AxisDefaultPosition position, CBCGPChartVisualObject* pChartCtrl);
	virtual void CalcNameRect(CBCGPGraphicsManager* pGM, CBCGPRect& rectPlotArea, const CRect& rectChartArea, BOOL bReposOnly);
	virtual void AdjustDiagramArea(CBCGPRect& rectDiagramArea, const CBCGPRect& rectPlotArea);
	virtual void UpdateAxisPos(const CBCGPRect& rectDiagramArea);
	virtual void CalcAxisPos(const CBCGPRect& rectDiagram, BOOL bStoreAxisPos = TRUE);
	virtual void CalcLabelsRect(CBCGPRect& rectDiagramArea);

	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawMajorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawMinorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnFillUnitIntervals(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);

	virtual CBCGPChartAxis* GetPerpendecularAxis() const;
	virtual CBCGPRect GetBoundingRect() const;
	virtual CBCGPRect GetBoundingRects(CArray<CBCGPRect, CBCGPRect>& arRects) const;

	virtual BOOL IsScaleBreakEnabled() const {return FALSE;}

	virtual void SetFixedIntervalWidth(int nWidth, int nValuesPerInterval = 1) 
	{
		UNREFERENCED_PARAMETER(nValuesPerInterval);
		UNREFERENCED_PARAMETER(nWidth);
	}

	virtual BOOL Zoom(int nMagnifier, const CBCGPPoint& ptZoomCenter = CBCGPPoint())
	{
		UNREFERENCED_PARAMETER(nMagnifier);
		UNREFERENCED_PARAMETER(ptZoomCenter);

		return FALSE;
	}

	virtual BOOL Scroll(double dblUnitsToScroll, BOOL bUnitsInPixels)
	{
		UNREFERENCED_PARAMETER(dblUnitsToScroll);
		UNREFERENCED_PARAMETER(bUnitsInPixels);
		
		return FALSE;
	}

	virtual BOOL IsZoomEnabled() const {return FALSE;}
	virtual BOOL CanBeScrolled() const {return FALSE;}

	virtual BOOL CanShowScrollBar() const {return FALSE;}
	
	virtual BOOL IsVertical() const {return TRUE;}

	virtual double ValueFromPoint(const CBCGPPoint& pt, CBCGPChartAxis::RoundType roundType = CBCGPChartAxis::RT_EXACT);

	BOOL m_bRadialGridLines;

protected:
	void DrawGridLines(CBCGPGraphicsManager* pGM, BOOL bMajor);
};

class BCGCBPRODLLEXPORT CBCGPChartAxisPolarX : public CBCGPChartAxisX
{
	DECLARE_DYNCREATE(CBCGPChartAxisPolarX)
protected:
	CBCGPChartAxisPolarX();
	virtual void CommonInit();

public:
	CBCGPChartAxisPolarX(int nAxisID, CBCGPChartAxis::AxisDefaultPosition position, CBCGPChartVisualObject* pChartCtrl);

	virtual CBCGPChartAxis* GetPerpendecularAxis() const;
		
	virtual void CalcMajorMinorUnits(CBCGPGraphicsManager* pGM, UINT nUnitsToDisplay = -1);
	virtual void CalcMinMaxDisplayedValues(BOOL bCalcMinDisplayedValue, double dblApproximation = 1.06);

	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawMajorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawMinorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawAxisLabels(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnFillUnitIntervals(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);

	virtual CBCGPRect GetBoundingRect() const;
	virtual CBCGPRect GetBoundingRects(CArray<CBCGPRect, CBCGPRect>& arRects) const;

	double GetRadius(CBCGPPoint& ptCenter) const;
	virtual double GetAxisSize() const;
	virtual double ValueFromPoint(const CBCGPPoint& pt, CBCGPChartAxis::RoundType roundType = CBCGPChartAxis::RT_EXACT);
	double GetAngleFromIndex(int nIndex) const;

	virtual void SwapDirection(BOOL bAdjustGradientAngles);

	virtual BOOL HitTest(const CBCGPPoint& pt, BCGPChartHitInfo* pHitInfo, DWORD dwHitInfoFlags);

	virtual BOOL Zoom(int nMagnifier, const CBCGPPoint& ptZoomCenter = CBCGPPoint())
	{
		UNREFERENCED_PARAMETER(nMagnifier);
		UNREFERENCED_PARAMETER(ptZoomCenter);

		return FALSE;
	}

	virtual BOOL Scroll(double dblUnitsToScroll, BOOL bUnitsInPixels)
	{
		UNREFERENCED_PARAMETER(dblUnitsToScroll);
		UNREFERENCED_PARAMETER(bUnitsInPixels);
		
		return FALSE;
	}

	virtual BOOL IsZoomEnabled() const {return FALSE;}
	virtual BOOL CanBeScrolled() const {return FALSE;}

	virtual BOOL CanShowScrollBar() const {return FALSE;}
	virtual BOOL IsLogScale() const {return FALSE;}

	virtual BOOL IsFixedDisplayRange() const {return FALSE;}
	virtual BOOL IsFixedMinimumDisplayValue() const {return FALSE;}
	virtual BOOL IsFixedMaximumDisplayValue() const {return FALSE;}

	virtual BOOL IsDisplayDataBetweenTickMarks() const {return !IsComponentXSet();}

	virtual void SetFixedIntervalWidth(int nWidth, int nValuesPerInterval = 1) 
	{
		UNREFERENCED_PARAMETER(nValuesPerInterval);
		UNREFERENCED_PARAMETER(nWidth);
	}

	virtual void SetFixedMajorUnit(double dblMajorUnit, BOOL bSet = TRUE);

	virtual void SetIndexedSeries(CBCGPChartSeries* pSeries)
	{
		UNREFERENCED_PARAMETER(pSeries);
	}

	virtual BOOL IsVertical() const {return FALSE;}
	virtual BOOL IsScaleBreakEnabled() const {return FALSE;}
	
	BOOL	m_bRotateLabels;
	BOOL	m_bUseGridLineForAxisLine;

protected:
	void DrawGridLines(CBCGPGraphicsManager* pGM, BOOL bMajor);
	void DrawTickMark(CBCGPGraphicsManager* pGM, double dblValue, CBCGPPoint ptPolarCenter, 
		double dblRadius, BOOL bMajor);
	virtual BOOL IsVariableLabelAngle() const {return m_bRotateLabels;}
	virtual double GetVariableLabelAngle (double dblValue);
};

class BCGCBPRODLLEXPORT CBCGPChartTernaryAxis : public CBCGPChartAxis
{
	DECLARE_DYNCREATE(CBCGPChartTernaryAxis);

protected:
	CBCGPChartTernaryAxis();
	virtual void CommonInit();

public:
	enum LabelMode
	{
		LM_NORMAL,
		LM_2080,
		LM_BASE_1
	};
public:
	CBCGPChartTernaryAxis(int nAxisID, CBCGPChartAxis::AxisDefaultPosition position, CBCGPChartVisualObject* pChartCtrl);

	virtual CBCGPChartAxis* GetPerpendecularAxis() const;
	virtual BOOL IsVertical() const {return FALSE;}

	virtual BOOL CalcMinMaxValues();
	virtual void CalcMinMaxDisplayedValues(BOOL bCalcMinDisplayedValue, double dblApproximation = 1.06);

	virtual void AdjustDiagramArea(CBCGPRect& rectDiagramArea, const CBCGPRect& rectPlotArea);
	virtual void UpdateAxisPos(const CBCGPRect& rectDiagramArea);
	virtual void CalcAxisPos(const CBCGPRect& rectDiagram, BOOL bStoreAxisPos = TRUE);
	virtual void CalcLabelsRect(CBCGPRect& rectDiagramArea);
	virtual void CalcNameRect(CBCGPGraphicsManager* pGM, CBCGPRect& rectPlotArea, const CRect& rectChartArea, BOOL bReposOnly);

	virtual void GetDisplayedLabel(double dblValue, CString& strLabel);

	virtual BOOL HitTestAxisShape(const CBCGPPoint& pt) const;
	virtual BOOL HitTestAxisLables(const CBCGPPoint& pt) const;

	virtual double GetAxisSize() const;
	virtual CBCGPRect GetBoundingRect() const {return m_rectBounds;}

	virtual double PointFromValue(double dblValue, BOOL bForceValueOnThickMark, BOOL bLogValue = TRUE);
	CBCGPPoint PointFromChartData(const CBCGPChartData& data);
	virtual double ValueFromPoint(const CBCGPPoint& pt, CBCGPChartAxis::RoundType roundType = CBCGPChartAxis::RT_EXACT);

	virtual void OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawMajorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawMinorGridLines(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnDrawAxisLabels(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);
	virtual void OnFillUnitIntervals(CBCGPGraphicsManager* pGM, const CBCGPRect& rectDiagram);

	void SetLabelMode(CBCGPChartTernaryAxis::LabelMode lm, BOOL bRedraw = FALSE);
	CBCGPChartTernaryAxis::LabelMode GetLabelMode() const {return m_labelMode;}

	virtual BOOL IsScaleBreakEnabled() const {return FALSE;}

protected:
	virtual void DrawGridLine(CBCGPGraphicsManager* pGM, double dblCurValue, BOOL bIsMajor);

protected:
	CBCGPRect	m_rectBounds;
	double		m_dblTickAngle;

	double		m_dblACoef;
	double		m_dblBCoef;

	CBCGPChartTernaryAxis::LabelMode	m_labelMode;
};

#endif // !defined(AFX_BCGPCHARTAXIS_H__A644E7E3_DFE7_4550_936D_7D786F991BE6__INCLUDED_)
