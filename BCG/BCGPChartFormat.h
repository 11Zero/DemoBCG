//*******************************************************************************
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

#if !defined(AFX_BCGPCHARTFORMAT_H__32FC7098_1F70_4941_BA09_C5662C5F2878__INCLUDED_)
#define AFX_BCGPCHARTFORMAT_H__32FC7098_1F70_4941_BA09_C5662C5F2878__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPGraphicsManager.h"

struct BCGCBPRODLLEXPORT BCGPChartShadowType
{
	BCGPChartShadowType()
	{
		Reset ();
	}

	void Reset ()
	{
		m_bDisplayShadow = FALSE;
		m_nTransparencyPercent = 35;
		m_nSizePercent = 1;
		m_nBlur = 0;
		m_nAngle = -1;
		m_nDistance = 3;
		m_color = CBCGPColor::DarkGray;
	}

	BOOL		m_bDisplayShadow;	

	CBCGPColor	m_color;
	int			m_nTransparencyPercent;	// 0 - 100
	int			m_nAngle;				// - 1- default series angle, 0 - 360 - custom angle
	int			m_nDistance;

	// **** RESERVED FOR THE FUTURE USE ****

	int			m_nSizePercent; // 1-200
	int			m_nBlur; // 0-100

};

struct BCGCBPRODLLEXPORT BCGPChartGlowAndSoftEdgesStyle
{
	// **** RESERVED FOR THE FUTURE USE ****

	BCGPChartGlowAndSoftEdgesStyle()
	{
		Reset ();
	}

	void Reset ()
	{
		m_bDisplayGlowAndSoftEdges = FALSE;
		m_color = RGB(0, 0, 0);
		m_nSize = 0;
		m_nTransparencyPercent = 100;
		m_nSoftEdgesSize = 0;
	}

	BOOL m_bDisplayGlowAndSoftEdges;

	COLORREF m_color;
	int m_nSize; // 0-150 pt
	int m_nTransparencyPercent; //0-100
	int m_nSoftEdgesSize; // 0-100
};

// defines line and border types (width, dash, cap, joint, compound)
struct BCGCBPRODLLEXPORT BCGPChartFormatLine
{
	BCGPChartFormatLine ()
	{
		Reset ();
		m_szScaleRatio.SetSize(1, 1);
	}

	void Reset ()
	{
		m_dblWidth = 1.;
		m_brLineColor.Empty();
		
		m_strokeStyle = CBCGPStrokeStyle(
			CBCGPStrokeStyle::BCGP_CAP_STYLE_ROUND,
			CBCGPStrokeStyle::BCGP_CAP_STYLE_ROUND,
			CBCGPStrokeStyle::BCGP_CAP_STYLE_ROUND,
			CBCGPStrokeStyle::BCGP_LINE_JOIN_ROUND);
	}

	void SetScaleRatio(const CBCGPSize& sz)
	{
		m_szScaleRatio = sz;
	}

	const BCGPChartFormatLine& operator=(const BCGPChartFormatLine& src)
	{
		m_brLineColor = src.m_brLineColor;
		m_strokeStyle = src.m_strokeStyle;
		m_szScaleRatio = src.GetScaleRatio();

		m_dblWidth = src.m_dblWidth;
		return *this;
	}

	const CBCGPSize& GetScaleRatio() const {return m_szScaleRatio;}

	double GetLineWidth(BOOL bScaled = FALSE) const 
	{
		if (!bScaled)
		{
			return m_dblWidth;
		}
		return max(m_szScaleRatio.cx, m_szScaleRatio.cy) * m_dblWidth;
	}

	void DetachFromGM()
	{
		m_strokeStyle.Destroy(TRUE);
		m_brLineColor.Destroy(TRUE);
	}

	double				m_dblWidth;
	CBCGPStrokeStyle	m_strokeStyle;
	CBCGPBrush			m_brLineColor;
	
protected:
	CBCGPSize			m_szScaleRatio;
};

struct BCGCBPRODLLEXPORT BCGPChartFormatArea
{
	// fill color of any area, front color of 3D bars, also used for right wall fill in 3D charts
	CBCGPBrush						m_brFillColor; 

	// side color of bars, also used for left wall fill in 3D charts
	CBCGPBrush						m_brSideFillColor; 
	// top color of bars
	CBCGPBrush						m_brTopFillColor;
	// bottom of 3D bars, also used for floor fill in 3D charts or front face of thick floor
	CBCGPBrush						m_brBottomFillColor; 

	CBCGPBrush						m_brFloorFillColorTop3D; // for thick floor top
	CBCGPBrush						m_brFloorFillColorSide3D; // for thick floor side
	CBCGPBrush						m_brFloorFillColorBottom3D; // for thick floor bottom

	CBCGPBrush						m_brLeftWallFillColorTop3D; // for thick left wall top
	CBCGPBrush						m_brLeftWallFillColorSide3D; // for thick left wall side
	CBCGPBrush						m_brLeftWallFillColorBottom3D; // for thick left wall bottom

	CBCGPBrush						m_brRightWallFillColorTop3D; // for thick right wall top
	CBCGPBrush						m_brRightWallFillColorSide3D; // for thick right wall side
	CBCGPBrush						m_brRightWallFillColorBottom3D; // for thick right wall bottom

	// outline format for any area; outline for any bars; also used for right wall in 3D charts
	BCGPChartFormatLine				m_outlineFormat;
	// outline format for floor in 3D charts
	BCGPChartFormatLine				m_outlineFormatFloor;
	// outline format for left wall in 3D charts 
	BCGPChartFormatLine				m_outlineFormatLeftWall;

	BCGPChartShadowType				m_shadowStyle;
	BCGPChartGlowAndSoftEdgesStyle	m_edgeStyle;

	BCGPChartFormatArea()
	{
		Reset();
	}

	void DetachFromGM()
	{
		m_brFillColor.Destroy();
		m_brSideFillColor.Destroy();
		m_brTopFillColor.Destroy();
		m_brBottomFillColor.Destroy();
		m_brFloorFillColorTop3D.Destroy();
		m_brFloorFillColorSide3D.Destroy();
		m_brFloorFillColorBottom3D.Destroy();
		m_brLeftWallFillColorTop3D.Destroy(); 
		m_brLeftWallFillColorSide3D.Destroy(); 
		m_brLeftWallFillColorBottom3D.Destroy(); 
		
		m_brRightWallFillColorTop3D.Destroy(); 
		m_brRightWallFillColorSide3D.Destroy(); 
		m_brRightWallFillColorBottom3D.Destroy(); 

		m_outlineFormat.DetachFromGM();
		m_outlineFormatFloor.DetachFromGM();
		m_outlineFormatLeftWall.DetachFromGM();
	}

	static void Generate3DColorsFromBrush(const CBCGPBrush& brIn, CBCGPBrush& brTop, CBCGPBrush& brSide, CBCGPBrush& brBottom);

	void Generate3DColors()
	{
		Generate3DColorsFromBrush(m_brFillColor, m_brTopFillColor, m_brSideFillColor, m_brBottomFillColor);
	}

	void Generate3DColorsForThickWalls()
	{
		Generate3DColorsFromBrush(m_brBottomFillColor, m_brFloorFillColorTop3D, m_brFloorFillColorSide3D, 
									m_brFloorFillColorBottom3D);
		Generate3DColorsFromBrush(m_brSideFillColor, m_brLeftWallFillColorTop3D, m_brLeftWallFillColorSide3D, 
									m_brLeftWallFillColorBottom3D);
		Generate3DColorsFromBrush(m_brFillColor, m_brRightWallFillColorTop3D, m_brRightWallFillColorSide3D, 
									m_brRightWallFillColorBottom3D);
	}

	void AdjustGradientAngles()
	{
		if (m_brFillColor.GetGradientColor().IsNull())
		{
			return;
		}

		CBCGPColor color = m_brFillColor.GetColor();
		CBCGPColor gradientColor = m_brFillColor.GetGradientColor();

		CBCGPBrush::BCGP_GRADIENT_TYPE gradientType = m_brFillColor.GetGradientType();
		if (gradientType == CBCGPBrush::BCGP_GRADIENT_HORIZONTAL)
		{
			m_brFillColor.SetColors(gradientColor, color, 
				CBCGPBrush::BCGP_GRADIENT_VERTICAL, m_brFillColor.GetOpacity());
		}
		else if (gradientType == CBCGPBrush::BCGP_GRADIENT_VERTICAL)
		{
			m_brFillColor.SetColors(gradientColor, color, 
				CBCGPBrush::BCGP_GRADIENT_HORIZONTAL, m_brFillColor.GetOpacity());
		}
		else if (gradientType == CBCGPBrush::BCGP_GRADIENT_CENTER_HORIZONTAL)
		{
			m_brFillColor.SetColors(gradientColor, color, 
				CBCGPBrush::BCGP_GRADIENT_CENTER_VERTICAL, m_brFillColor.GetOpacity());
		}
		else if (gradientType == CBCGPBrush::BCGP_GRADIENT_CENTER_VERTICAL)
		{
			m_brFillColor.SetColors(gradientColor, color, 
				CBCGPBrush::BCGP_GRADIENT_CENTER_HORIZONTAL, m_brFillColor.GetOpacity());
		}
		else if (gradientType == CBCGPBrush::BCGP_GRADIENT_PIPE_HORIZONTAL)
		{
			m_brFillColor.SetColors(color, gradientColor,
				CBCGPBrush::BCGP_GRADIENT_PIPE_VERTICAL, m_brFillColor.GetOpacity());
		}
		else if (gradientType == CBCGPBrush::BCGP_GRADIENT_PIPE_VERTICAL)
		{
			m_brFillColor.SetColors(color, gradientColor,
				CBCGPBrush::BCGP_GRADIENT_PIPE_HORIZONTAL, m_brFillColor.GetOpacity());
		}
	}

	const BCGPChartFormatArea& operator=(const BCGPChartFormatArea& src)
	{
		CopyFrom(src);
		return *this;
	}

	void CopyFrom(const BCGPChartFormatArea& src)
	{
		m_brFillColor = src.m_brFillColor;
		m_brSideFillColor = src.m_brSideFillColor;
		m_brTopFillColor = src.m_brTopFillColor;
		m_brBottomFillColor = src.m_brBottomFillColor;

		m_brFloorFillColorTop3D = src.m_brFloorFillColorTop3D;
		m_brFloorFillColorSide3D = src.m_brFloorFillColorSide3D;
		m_brFloorFillColorBottom3D = src.m_brFloorFillColorBottom3D;

		m_brLeftWallFillColorTop3D = src.m_brLeftWallFillColorTop3D;
		m_brLeftWallFillColorSide3D = src.m_brLeftWallFillColorSide3D;
		m_brLeftWallFillColorBottom3D = src.m_brLeftWallFillColorBottom3D;

		m_brRightWallFillColorTop3D = src.m_brRightWallFillColorTop3D;
		m_brRightWallFillColorSide3D = src.m_brRightWallFillColorSide3D;
		m_brRightWallFillColorBottom3D = src.m_brRightWallFillColorBottom3D;

		m_outlineFormat = src.m_outlineFormat;
		m_outlineFormatFloor = src.m_outlineFormatFloor;
		m_outlineFormatLeftWall = src.m_outlineFormatLeftWall;

		m_shadowStyle = src.m_shadowStyle;
		m_edgeStyle = src.m_edgeStyle;
		
		m_szContentPadding = src.GetContentPadding(FALSE);
		m_szScaleRatio = src.GetScaleRatio();
	}

	void Reset()
	{
		m_szScaleRatio.SetSize(1., 1.);
		m_szContentPadding.SetSize(3, 3);

		m_brFillColor.Empty();
		m_brSideFillColor.Empty();
		m_brTopFillColor.Empty();
		m_brBottomFillColor.Empty();

		m_brFloorFillColorTop3D.Empty();
		m_brFloorFillColorSide3D.Empty();
		m_brFloorFillColorBottom3D.Empty();

		m_brLeftWallFillColorTop3D.Empty();
		m_brLeftWallFillColorSide3D.Empty();
		m_brLeftWallFillColorBottom3D .Empty();

		m_brRightWallFillColorTop3D.Empty();
		m_brRightWallFillColorSide3D.Empty();
		m_brRightWallFillColorBottom3D.Empty();

		m_outlineFormat.Reset();
		m_outlineFormatFloor.Reset();
		m_outlineFormatLeftWall.Reset();
		m_shadowStyle.Reset();
		m_edgeStyle.Reset();
	}

	void SetContentPadding(const CBCGPSize& sz)
	{
		m_szContentPadding = sz;
	}

	CBCGPSize GetContentPadding(BOOL bScaled = FALSE) const
	{
		if (bScaled)
		{
			return CBCGPSize(m_szContentPadding.cx * m_szScaleRatio.cx, m_szContentPadding.cy * m_szScaleRatio.cy);
		}

		return m_szContentPadding;
	}

	CBCGPSize GetScaleRatio() const {return m_szScaleRatio;}

	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld)
	{
		UNREFERENCED_PARAMETER(sizeScaleRatioOld);

		m_outlineFormat.SetScaleRatio(sizeScaleRatioNew);
		m_szScaleRatio = sizeScaleRatioNew;
	}

protected:
	CBCGPSize	m_szContentPadding;
	CBCGPSize	m_szScaleRatio;
};

struct BCGCBPRODLLEXPORT BCGPChartFormatSelection : public BCGPChartFormatArea
{
	enum SelectionType
	{
		ST_HORZ_AXIS_ONLY,
		ST_VERT_AXIS_ONLY,
		ST_VERT_AND_HORZ_AXES
	};

	BCGPChartFormatSelection()
	{
		Reset();
	}

	void DetachFromGM()
	{
		BCGPChartFormatArea::DetachFromGM();
		m_brFillColor.Destroy();
		m_outlineFormat.DetachFromGM();
	}

	void Reset()
	{
		m_brFillColor.Empty();
		m_outlineFormat.Reset();
		m_selectionType = BCGPChartFormatSelection::ST_VERT_AND_HORZ_AXES;
	}

	const BCGPChartFormatSelection& operator=(const BCGPChartFormatSelection& src)
	{
		BCGPChartFormatArea::operator=(src);
		m_selectionType = src.m_selectionType;
		return *this;
	}

	void CopyFrom(const BCGPChartFormatArea& src)
	{
		BCGPChartFormatArea::CopyFrom(src);
	}

	SelectionType	m_selectionType;
};

struct BCGCBPRODLLEXPORT BCGPChartFormatPlotArea : public BCGPChartFormatArea
{
	BCGPChartFormatPlotArea()
	{
		Reset();
	}

	void Reset()
	{
		BCGPChartFormatArea::Reset();
		m_nXRotation = 120;
		m_nYRotation = 90;
		m_nDepth = 100;
		m_nHeight = 100;
		m_bEnable3DFormat = FALSE;
	}

	const BCGPChartFormatPlotArea& operator=(const BCGPChartFormatPlotArea& src)
	{
		BCGPChartFormatArea::operator=(src);

		m_nXRotation = src.m_nXRotation;
		m_nYRotation = src.m_nYRotation;
		m_nDepth = src.m_nDepth;
		m_nHeight = src.m_nDepth;
		m_bEnable3DFormat = src.m_bEnable3DFormat;

		return *this;
	}

	BOOL	m_bEnable3DFormat;

	int		m_nXRotation;
	int		m_nYRotation;

	int		m_nDepth; // percent from base
	int		m_nHeight; // percent from base
};

struct BCGCBPRODLLEXPORT BCGPChartFormatLabel : public BCGPChartFormatArea
{
	static CString m_strDefaultFontFamily;

	BCGPChartFormatLabel()
	{
		m_bIsThumbnailMode = FALSE;
		Reset();
	}

	~BCGPChartFormatLabel()
	{
		
	}

	void DetachFromGM()
	{
		BCGPChartFormatArea::DetachFromGM();
		m_brTextColor.Destroy();
		m_textFormat.Destroy();
	}

	void Reset()
	{
		BCGPChartFormatArea::Reset();

		m_brTextColor.Empty();
		m_brFillColor.Empty();

		m_textFormat.Create(m_strDefaultFontFamily, m_bIsThumbnailMode ? 8.0f : 12.0f);
		m_textFormat.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
		m_textFormat.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
	}

	const BCGPChartFormatLabel& operator=(const BCGPChartFormatLabel& src)
	{
		BCGPChartFormatArea::operator=(src);
		
		m_textFormat.CopyFrom(src.m_textFormat);
		m_brTextColor = src.m_brTextColor;
		m_bIsThumbnailMode = src.m_bIsThumbnailMode;
		return *this;
	}

	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld)
	{
		BCGPChartFormatArea::OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);

		m_textFormat.Scale((float)sizeScaleRatioNew.cy);
	}

	CBCGPTextFormat		m_textFormat;
	CBCGPBrush			m_brTextColor;
	BOOL				m_bIsThumbnailMode;
};

struct BCGCBPRODLLEXPORT BCGPChartDataLabelOptions
{
	enum LabelPosition
	{
		LP_DEFAULT_POS,
		LP_CENTER,
		LP_INSIDE_END,
		LP_INSIDE_BASE,
		LP_OUTSIDE_END
	};

	enum LabelContent
	{
		LC_DEFAULT_CONTENT = 0x0,
		LC_SERIES_NAME = 0x01,
		LC_CATEGORY_NAME = 0x02,
		LC_VALUE = 0x04,
		LC_PERCENTAGE = 0x08,
		LC_X_VALUE = 0x10,
		LC_BUBBLE_SIZE = 0x20,
		LC_Y1_VALUE = 0x40,
		LC_Y2_VALUE = 0x80,
		LC_A_TERNARY_PERCENTAGE = LC_PERCENTAGE,
		LC_B_TERNARY_PERCENTAGE = 0x100,
		LC_C_TERNARY_PERCENTAGE = 0x200,
		LC_DP_INDEX = 0x400,
		LC_A_TERNARY_VALUE = LC_VALUE,
		LC_B_TERNARY_VALUE = LC_Y1_VALUE,
		LC_C_TERNARY_VALUE = LC_Y2_VALUE,
		LC_BUBBLE_VALUES = LC_VALUE | LC_X_VALUE | LC_BUBBLE_SIZE,
		LC_ALL_BUBBLE_VALUES = LC_SERIES_NAME | LC_VALUE | LC_X_VALUE | LC_BUBBLE_SIZE,
		LC_VALUE_PERCENTAGE = LC_VALUE | LC_PERCENTAGE,
		LC_SERIES_VALUES = LC_SERIES_NAME | LC_VALUE | LC_X_VALUE,
		LC_SERIES_CATEGORY_VALUE = LC_SERIES_NAME | LC_CATEGORY_NAME | LC_VALUE,
		LC_PIE_NAME_PERCENTAGE = LC_CATEGORY_NAME | LC_PERCENTAGE,
		LC_ALL_TERNARY_VALUES = LC_A_TERNARY_VALUE | LC_B_TERNARY_VALUE | LC_C_TERNARY_VALUE,
		LC_ALL_TERNARY_PERCENTAGE = LC_A_TERNARY_PERCENTAGE | LC_B_TERNARY_PERCENTAGE | LC_C_TERNARY_PERCENTAGE
	};

	BCGPChartDataLabelOptions()
	{
		Reset();
		m_szScaleRatio.SetSize(1., 1.);
	}

	void Reset()
	{
		m_bShowDataLabel = FALSE;

		m_position = BCGPChartDataLabelOptions::LP_DEFAULT_POS;
		m_dblAngle = 45;

		m_content = BCGPChartDataLabelOptions::LC_DEFAULT_CONTENT;
		m_strLabelSeparator = _T(", ");

		m_nKeyToLabelDistance = 3; 
		
		m_dblDistanceFromMarker = 15;

		m_bUnderlineDataLabel = FALSE;
		m_bDrawDataLabelBorder = TRUE;

		m_dblMaxWidthToHeightRatio = 6.;
		m_bAutoWordWrap = FALSE;
		m_dblLineCount = 1.2;

		m_bIncludeLegendKeyInLabel = FALSE;
		m_bDropLineToMarker = TRUE;
	}

	const BCGPChartDataLabelOptions& operator=(const BCGPChartDataLabelOptions& src)
	{
		m_bShowDataLabel = src.m_bShowDataLabel;
		m_position = src.m_position;
		
		if (src.m_dblAngle != -1)
		{
			m_dblAngle = src.m_dblAngle;
		}
		if (src.m_content != LC_DEFAULT_CONTENT)
		{
			m_content = src.m_content;
		}
		m_bIncludeLegendKeyInLabel = src.m_bIncludeLegendKeyInLabel;
		m_dblDistanceFromMarker = src.m_dblDistanceFromMarker;
		m_bDropLineToMarker = src.m_bDropLineToMarker;
		m_bUnderlineDataLabel = src.m_bUnderlineDataLabel;
		m_bDrawDataLabelBorder = src.m_bDrawDataLabelBorder;
		m_nKeyToLabelDistance = src.m_nKeyToLabelDistance;
		m_strLabelSeparator = src.m_strLabelSeparator;
		m_bAutoWordWrap = src.m_bAutoWordWrap;
		m_dblMaxWidthToHeightRatio = src.m_dblMaxWidthToHeightRatio;
		m_dblLineCount = src.m_dblLineCount;
		m_strDataFormat = src.m_strDataFormat;

		m_szScaleRatio = src.m_szScaleRatio;

		return *this;
	}

	BOOL				m_bShowDataLabel;

	LabelPosition		m_position;
	double				m_dblAngle;
	LabelContent		m_content;
	double				m_dblDistanceFromMarker;

	BOOL				m_bIncludeLegendKeyInLabel;

	int					m_nKeyToLabelDistance; // pixels

	BOOL				m_bDropLineToMarker;
	BOOL				m_bUnderlineDataLabel;
	BOOL				m_bDrawDataLabelBorder;

	BOOL				m_bAutoWordWrap;
	double				m_dblMaxWidthToHeightRatio;
	double				m_dblLineCount;

	CString				m_strLabelSeparator;
	CString				m_strDataFormat;

	double	GetDistanceFromMarker() const 
	{
		return m_dblDistanceFromMarker * max(m_szScaleRatio.cx, m_szScaleRatio.cy);
	}

	double	GetKeyToLabelDistance() const
	{
		return m_nKeyToLabelDistance * max(m_szScaleRatio.cx, m_szScaleRatio.cy);
	}

	const CBCGPSize& GetScaleRatio() const {return m_szScaleRatio;}
	void SetScaleRatio(const CBCGPSize& sz) {m_szScaleRatio = sz;}

protected:
	CBCGPSize			m_szScaleRatio;
};

struct BCGCBPRODLLEXPORT BCGPChartDataTableOptions
{
	enum TableContent
	{
		TC_DEFAULT_CONTENT = 0x0,
		TC_VALUE = 0x01,
		TC_DP_INDEX = 0x02,
	};

	BCGPChartDataTableOptions()
	{
		Reset();
		m_szScaleRatio.SetSize(1., 1.);
	}

	void Reset()
	{
		m_content = BCGPChartDataTableOptions::TC_DEFAULT_CONTENT;
		m_strLabelSeparator = _T(", ");
	}

	const BCGPChartDataTableOptions& operator=(const BCGPChartDataTableOptions& src)
	{
		if (src.m_content != TC_DEFAULT_CONTENT)
		{
			m_content = src.m_content;
		}

		m_strLabelSeparator = src.m_strLabelSeparator;
		m_strDataFormat = src.m_strDataFormat;

		m_szScaleRatio = src.m_szScaleRatio;

		return *this;
	}

	TableContent		m_content;
	CString				m_strLabelSeparator;
	CString				m_strDataFormat;

	const CBCGPSize& GetScaleRatio() const {return m_szScaleRatio;}
	void SetScaleRatio(const CBCGPSize& sz) {m_szScaleRatio = sz;}

protected:
	CBCGPSize			m_szScaleRatio;
};

struct BCGCBPRODLLEXPORT BCGPChartFormatDataLabel : public BCGPChartFormatLabel
{
	friend class CBCGPBaseChartImpl;

	BCGPChartFormatDataLabel()
	{
		Reset();
	}

	void Reset()
	{
		BCGPChartFormatLabel::Reset();
		m_options.Reset();

		m_szContentPadding.SetSize(3, 1);
		m_brFillColor.Empty();
		m_outlineFormat.Reset();
		m_outlineFormat.m_dblWidth = 1.;
	}

	const BCGPChartFormatDataLabel& operator=(const BCGPChartFormatDataLabel& src)
	{
		BCGPChartFormatLabel::operator=(src);
		m_options = src.m_options;
		return *this;
	}

	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld)
	{
		BCGPChartFormatLabel::OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);
		m_options.SetScaleRatio(sizeScaleRatioNew);
	}

	BCGPChartDataLabelOptions m_options;
};

struct BCGCBPRODLLEXPORT BCGPChartMarkerOptions
{
	enum MarkerShape
	{
		MS_CIRCLE,
		MS_TRIANGLE,
		MS_RECTANGLE,
		MS_RHOMBUS
	};

	BCGPChartMarkerOptions()
	{
		Reset();
	}

	void Reset()
	{
		m_nMarkerSize = 7;
		m_markerShape = BCGPChartMarkerOptions::MS_CIRCLE;
		m_bShowMarker = FALSE;
	}

	MarkerShape		m_markerShape;
	BOOL			m_bShowMarker;

	void SetMarkerSize(int nMarkerSize) {m_nMarkerSize = nMarkerSize;}
	int  GetMarkerSize() const {return m_nMarkerSize;}

protected:
	int				m_nMarkerSize;
};

struct BCGCBPRODLLEXPORT BCGPChartFormatMarker : public BCGPChartFormatArea
{
	BCGPChartFormatMarker ()
	{
		Reset();
	}

	void Reset ()
	{
		BCGPChartFormatArea::Reset();
		m_brFillColor.Empty();
		m_outlineFormat.Reset();
	}

	BCGPChartMarkerOptions	m_options;

	CBCGPSize GetMarkerSize() const
	{
		return CBCGPSize(m_options.GetMarkerSize() * m_szScaleRatio.cx, m_options.GetMarkerSize() * m_szScaleRatio.cy);
	}
};

struct BCGCBPRODLLEXPORT BCGPChartLayout
{
	enum LegendPosition
	{
		LP_NONE,
		LP_TOP,
		LP_BOTTOM,
		LP_LEFT,
		LP_RIGHT,
		LP_TOPRIGHT
	};

	BCGPChartLayout()
	{
		m_bShowChartTitle = TRUE;
		m_bTitleOverlapsChart = FALSE;

		m_legendPosition = BCGPChartLayout::LP_TOPRIGHT;
		m_bLegendOverlapsChart = FALSE;
		m_bDrawLegendShape = TRUE;

		m_bShowDataTable = FALSE;
		m_bShowDropLines = FALSE;
		m_bShowHighLowLines = FALSE;
	}

	BOOL			m_bShowChartTitle;
	BOOL			m_bTitleOverlapsChart;

	LegendPosition	m_legendPosition;
	BOOL			m_bLegendOverlapsChart;
	BOOL			m_bDrawLegendShape;

	BOOL			m_bShowDataTable;
	BOOL			m_bShowDropLines;
	BOOL			m_bShowHighLowLines;
};

struct BCGCBPRODLLEXPORT BCGPChartFormatSeries
{
	friend class CBCGPChartVisualObject;

	enum ChartCurveType	
	{
		CCT_NO_LINE,
		CCT_LINE,
		CCT_SPLINE,			// Kochanek-Bartels spline
		CCT_SPLINE_HERMITE,	// Hermite spline
		CCT_STEP,
		CCT_REVERSED_STEP
	};

	BCGPChartFormatMarker						m_markerFormat;
	BCGPChartFormatArea							m_seriesElementFormat;
	
	BCGPChartShadowType							m_shadowType;
	BCGPChartGlowAndSoftEdgesStyle				m_softEdgesStyle;

	BCGPChartFormatLabel						m_legendLabelFormat;
	BCGPChartDataLabelOptions::LabelContent		m_legendLabelContent;

	BCGPChartFormatDataLabel					m_dataLabelFormat;
	BCGPChartDataTableOptions					m_dataTableOptions;

	ChartCurveType								m_curveType;
	double										m_dbl3DLineThickness;

	BCGPChartFormatSeries ()
	{
		Reset();
	}

	void DetachFromGM()
	{
		m_markerFormat.DetachFromGM();
		m_seriesElementFormat.DetachFromGM();
		m_legendLabelFormat.DetachFromGM();
		m_dataLabelFormat.DetachFromGM();
	}

	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld)
	{
		m_markerFormat.OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);
		m_seriesElementFormat.OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);
		m_legendLabelFormat.OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);
		m_dataLabelFormat.OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);
		
		m_dataTableOptions.SetScaleRatio(sizeScaleRatioNew);
	}

//---- Set SeriesElement properties
	void SetSeriesFill(const CBCGPBrush& br)
	{
		m_seriesElementFormat.m_brFillColor = br;
		m_seriesElementFormat.Generate3DColors();
	}

	void SetSeriesLineColor(const CBCGPBrush& br)
	{
		m_seriesElementFormat.m_outlineFormat.m_brLineColor = br;
	}

	void SetSeriesLineWidth(double dblWidth)
	{
		m_seriesElementFormat.m_outlineFormat.m_dblWidth = dblWidth;
	}

	void SetSeriesStrokeStyle(const CBCGPStrokeStyle& strokeStyle)
	{
		m_seriesElementFormat.m_outlineFormat.m_strokeStyle = strokeStyle;
	}

	void SetSeriesOutlineDashStyle(CBCGPStrokeStyle::BCGP_DASH_STYLE style)
	{
		m_seriesElementFormat.m_outlineFormat.m_strokeStyle.SetDashStyle(style);
	}

//---- Set Series Marker properties
	void SetMarkerFill(const CBCGPBrush& br)
	{
		m_markerFormat.m_brFillColor = br;
	}

	void SetMarkerLineColor(const CBCGPBrush& br)
	{
		m_markerFormat.m_outlineFormat.m_brLineColor = br;
	}

	void SetMarkerLineWidth(double dblWidth)
	{
		m_markerFormat.m_outlineFormat.m_dblWidth = dblWidth;
	}

	void SetMarkerStrokeStyle(const CBCGPStrokeStyle& strokeStyle)
	{
		m_markerFormat.m_outlineFormat.m_strokeStyle = strokeStyle;
	}

	void SetMarkerSize (int nSize)
	{
		m_markerFormat.m_options.SetMarkerSize(nSize);
	}

	void SetMarkerShape(BCGPChartMarkerOptions::MarkerShape shape)
	{
		m_markerFormat.m_options.m_markerShape = shape;
	}
//------- Set DataLabel properties
	void SetDataLabelFill(const CBCGPBrush& br)
	{
		m_dataLabelFormat.m_brFillColor = br;
	}

	void SetDataLabelLineColor(const CBCGPBrush& br)
	{
		m_dataLabelFormat.m_outlineFormat.m_brLineColor = br;
	}

	void SetDataLabelLineWidth(double dblWidth)
	{
		m_dataLabelFormat.m_outlineFormat.m_dblWidth = dblWidth;
	}

	void SetDataLabelStrokeStyle(const CBCGPStrokeStyle& strokeStyle)
	{
		m_dataLabelFormat.m_outlineFormat.m_strokeStyle = strokeStyle;
	}

	void SetDataLabelTextFormat(const CBCGPTextFormat& textFormat)
	{
		m_dataLabelFormat.m_textFormat = textFormat;
	}

	void SetDataLabelDrawBorder(BOOL bDraw)
	{
		m_dataLabelFormat.m_options.m_bDrawDataLabelBorder = bDraw;
	}

	void SetDataLabelUnderline(BOOL bUnderline) 
	{
		m_dataLabelFormat.m_options.m_bUnderlineDataLabel = bUnderline;
	}

//---------
	const BCGPChartFormatSeries& operator=(const BCGPChartFormatSeries& src)
	{
		CopyFrom(src);
		return *this;
	}

	void CopyFrom(const BCGPChartFormatSeries& src)
	{
		m_markerFormat = src.m_markerFormat;
		m_seriesElementFormat = src.m_seriesElementFormat;
		m_dataLabelFormat = src.m_dataLabelFormat;
		m_dataTableOptions = src.m_dataTableOptions;
		
		m_shadowType = src.m_shadowType;
		m_softEdgesStyle = src.m_softEdgesStyle;
		m_legendLabelFormat = src.m_legendLabelFormat;
		m_curveType = src.m_curveType;
		m_legendLabelContent = src.m_legendLabelContent;
		m_dbl3DLineThickness = src.m_dbl3DLineThickness;
		m_dblSeriesFillOpacity = src.GetSeriesFillOpacity();
	}

	void Reset ()
	{
		m_markerFormat.Reset();
		m_seriesElementFormat.Reset ();
		m_dataLabelFormat.Reset();
		m_dataTableOptions.Reset();
		m_legendLabelFormat.Reset();
		m_curveType = (ChartCurveType)-1;
		m_legendLabelContent = BCGPChartDataLabelOptions::LC_SERIES_NAME;
		m_dbl3DLineThickness = 1.;
		m_dblSeriesFillOpacity = -1.;
	}

	double GetSeriesFillOpacity() const 
	{
		return m_dblSeriesFillOpacity;
	}

	void SetSeriesFillOpacity(double dblOpacity = -1.)
	{
		m_dblSeriesFillOpacity = dblOpacity;
	}

	void AdjustGradientAngles()
	{
		m_seriesElementFormat.AdjustGradientAngles();
	}

protected:
	double m_dblSeriesFillOpacity;

};

struct BCGCBPRODLLEXPORT BCGPChartFormatLegendTable 
{
	BCGPChartFormatLegendTable()
	{
		m_szScaleRatio.SetSize(1, 1);
		Reset();
	}

	void Reset()
	{
		m_brParentControlFill.Empty();
		m_brFill.Empty();
		m_outlineFormat.Reset();
		m_titleFormat.Reset();

		m_bInterlaceRows = FALSE;
		m_bShowTitle = FALSE;

		m_bDrawLegendBorders = TRUE;
		m_bDrawHorizontalGridLines = TRUE;
		m_bDrawVerticalGridLines = TRUE;
		m_bDrawTitleGridLine = TRUE;
		m_bFixedRowHeight = FALSE;

		m_bShowChartName = FALSE;
		m_bAdjustLegendSizeByTitleSize = TRUE;

		SetContentPadding(CBCGPSize(10, 10));
	}

	void DetachFromGM()
	{
		m_brParentControlFill.Destroy();
		m_brFill.Destroy();
		m_outlineFormat.DetachFromGM();
		m_brInterlaceFill.Destroy();
		m_titleFormat.DetachFromGM();
	}

	CBCGPBrush				m_brParentControlFill;
	CBCGPBrush				m_brFill;
	BCGPChartFormatLine		m_outlineFormat;

	BOOL					m_bInterlaceRows;
	CBCGPBrush				m_brInterlaceFill;

	BCGPChartFormatLabel	m_titleFormat;
	BOOL					m_bShowTitle;

	BOOL					m_bDrawLegendBorders;
	BOOL					m_bDrawHorizontalGridLines;
	BOOL					m_bDrawVerticalGridLines;
	BOOL					m_bDrawTitleGridLine;
	BOOL					m_bFixedRowHeight;

	BOOL					m_bShowChartName;
	BOOL					m_bAdjustLegendSizeByTitleSize;

	void SetContentPadding(const CBCGPSize& sz)
	{
		m_szContentPadding = sz;
	}
	
	CBCGPSize GetContentPadding(BOOL bScaled = FALSE) const
	{
		if (bScaled)
		{
			return CBCGPSize(m_szContentPadding.cx * m_szScaleRatio.cx, m_szContentPadding.cy * m_szScaleRatio.cy);
		}
		
		return m_szContentPadding;
	}
	
	CBCGPSize GetScaleRatio() const {return m_szScaleRatio;}
	
	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld)
	{
		UNREFERENCED_PARAMETER(sizeScaleRatioOld);
		
		m_outlineFormat.SetScaleRatio(sizeScaleRatioNew);
		m_titleFormat.OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);
		m_szScaleRatio = sizeScaleRatioNew;
	}

protected:
	CBCGPSize				m_szContentPadding;
	CBCGPSize				m_szScaleRatio;
};

struct BCGCBPRODLLEXPORT BCGPChartFormatDataTable
{
	BCGPChartFormatDataTable()
	{
		m_szScaleRatio.SetSize(1, 1);
		Reset();
	}
	
	void Reset()
	{
		m_brFill.Empty();
		m_outlineFormat.Reset();
		m_verticalGridLinesFormat.Reset();
		m_horizontalGridLinesFormat.Reset();
		m_labelFormat.Reset();
		
		m_bInterlaceRows = FALSE;
		
		m_bDrawOutline = TRUE;
		m_bDrawHorizontalGridLines = TRUE;
		m_bDrawVerticalGridLines = TRUE;
		
		SetContentPadding(CBCGPSize(3.0, 2.0));

		m_bShowLegendKeys = FALSE;
		m_bCustomInterlaceColor = FALSE;

		m_bRoundValues = TRUE;
	}
	
	void DetachFromGM()
	{
		m_brFill.Destroy();
		m_outlineFormat.DetachFromGM();
		m_horizontalGridLinesFormat.DetachFromGM();
		m_verticalGridLinesFormat.DetachFromGM();
		m_brInterlaceFill.Destroy();
		m_labelFormat.DetachFromGM();
	}

	CBCGPBrush					m_brFill;
	BCGPChartFormatLine			m_outlineFormat;
	BCGPChartFormatLine			m_verticalGridLinesFormat;
	BCGPChartFormatLine			m_horizontalGridLinesFormat;
	
	BOOL						m_bInterlaceRows;
	CBCGPBrush					m_brInterlaceFill;
	
	BCGPChartFormatLabel		m_labelFormat;

	BOOL						m_bDrawHorizontalGridLines;
	BOOL						m_bDrawVerticalGridLines;
	BOOL						m_bDrawOutline;
	BOOL						m_bShowLegendKeys;
	BOOL						m_bCustomInterlaceColor;
	BOOL						m_bRoundValues;
	
	void SetContentPadding(const CBCGPSize& sz)
	{
		m_szContentPadding = sz;
	}
	
	CBCGPSize GetContentPadding(BOOL bScaled = FALSE) const
	{
		if (bScaled)
		{
			return CBCGPSize(m_szContentPadding.cx * m_szScaleRatio.cx, m_szContentPadding.cy * m_szScaleRatio.cy);
		}
		
		return m_szContentPadding;
	}
	
	CBCGPSize GetScaleRatio() const {return m_szScaleRatio;}
	
	virtual void OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioNew, const CBCGPSize& sizeScaleRatioOld)
	{
		UNREFERENCED_PARAMETER(sizeScaleRatioOld);
		
		m_outlineFormat.SetScaleRatio(sizeScaleRatioNew);
		m_verticalGridLinesFormat.SetScaleRatio(sizeScaleRatioNew);
		m_horizontalGridLinesFormat.SetScaleRatio(sizeScaleRatioNew);
		m_labelFormat.OnScaleRatioChanged(sizeScaleRatioNew, sizeScaleRatioOld);

		m_szScaleRatio = sizeScaleRatioNew;
	}
	
protected:
	CBCGPSize				m_szContentPadding;
	CBCGPSize				m_szScaleRatio;
};

#define BCGP_CHART_NUM_SERIES_COLORS_IN_THEME	5

struct BCGCBPRODLLEXPORT BCGPSeriesColors
{
	CBCGPBrush		m_brElementFillColor;
	CBCGPBrush		m_brElementLineColor;

	CBCGPBrush		m_brElementTopFillColor;
	CBCGPBrush		m_brElementSideFillColor;
	CBCGPBrush		m_brElementBottomFillColor;

	CBCGPBrush		m_brAlternativeFillColor;
	CBCGPBrush		m_brAlternativeLineColor;

	CBCGPBrush		m_brMarkerFillColor;
	CBCGPBrush		m_brMarkerLineColor;

	CBCGPBrush		m_brDataLabelFillColor;
	CBCGPBrush		m_brDataLabelLineColor;
	CBCGPBrush		m_brDataLabelTextColor;

	void Empty()
	{
		m_brElementFillColor.Empty();

		m_brElementTopFillColor.Empty();
		m_brElementSideFillColor.Empty();
		m_brElementBottomFillColor.Empty();

		m_brElementLineColor.Empty();
		m_brAlternativeFillColor.Empty();
		m_brAlternativeLineColor.Empty();
		m_brMarkerFillColor.Empty();
		m_brMarkerLineColor.Empty();
		m_brDataLabelFillColor.Empty();
		m_brDataLabelLineColor.Empty();
		m_brDataLabelTextColor.Empty();
	}

	BOOL IsEmpty() const
	{
		return m_brElementFillColor.IsEmpty() && m_brElementLineColor.IsEmpty() && 
			m_brMarkerFillColor.IsEmpty() && m_brMarkerLineColor.IsEmpty() && 
			m_brDataLabelTextColor.IsEmpty();
	}

};

struct BCGCBPRODLLEXPORT BCGPSeriesColorsPtr
{
	BCGPSeriesColorsPtr()
	{
		m_pBrElementFillColor = NULL;
		m_pBrElementLineColor = NULL;

		m_pBrElementTopFillColor = NULL;
		m_pBrElementSideFillColor = NULL;
		m_pBrElementBottomFillColor = NULL;

		m_pBrMarkerFillColor = NULL;
		m_pBrMarkerLineColor = NULL;
		m_pBrDataLabelFillColor = NULL;
		m_pBrDataLabelLineColor = NULL;
		m_pBrDataLabelTextColor = NULL;

		m_pBrElementAltFillColor = NULL;
		m_pBrElementAltLineColor = NULL;
	}

	BCGPSeriesColorsPtr(const BCGPChartFormatSeries& format)
	{
		Init(format);
	}

	void Init(const BCGPChartFormatSeries& format)
	{
		m_pBrElementFillColor = (CBCGPBrush*)&format.m_seriesElementFormat.m_brFillColor;

		m_pBrElementTopFillColor = (CBCGPBrush*)&format.m_seriesElementFormat.m_brTopFillColor;
		m_pBrElementSideFillColor = (CBCGPBrush*)&format.m_seriesElementFormat.m_brSideFillColor;
		m_pBrElementBottomFillColor = (CBCGPBrush*)&format.m_seriesElementFormat.m_brBottomFillColor;

		m_pBrElementLineColor = (CBCGPBrush*)&format.m_seriesElementFormat.m_outlineFormat.m_brLineColor;
		m_pBrMarkerFillColor = (CBCGPBrush*)&format.m_markerFormat.m_brFillColor;
		m_pBrMarkerLineColor = (CBCGPBrush*)&format.m_markerFormat.m_outlineFormat.m_brLineColor;
		m_pBrDataLabelFillColor = (CBCGPBrush*)&format.m_dataLabelFormat.m_brFillColor;
		m_pBrDataLabelLineColor = (CBCGPBrush*)&format.m_dataLabelFormat.m_outlineFormat.m_brLineColor;
		m_pBrDataLabelTextColor = (CBCGPBrush*)&format.m_dataLabelFormat.m_brTextColor;
	}

	CBCGPBrush*		m_pBrElementFillColor;
	CBCGPBrush*		m_pBrElementLineColor;

	CBCGPBrush*		m_pBrElementTopFillColor;
	CBCGPBrush*		m_pBrElementSideFillColor;
	CBCGPBrush*		m_pBrElementBottomFillColor;

	CBCGPBrush*		m_pBrElementAltFillColor;
	CBCGPBrush*		m_pBrElementAltLineColor;

	CBCGPBrush*		m_pBrMarkerFillColor;
	CBCGPBrush*		m_pBrMarkerLineColor;
	CBCGPBrush*		m_pBrDataLabelFillColor;
	CBCGPBrush*		m_pBrDataLabelLineColor;
	CBCGPBrush*		m_pBrDataLabelTextColor;
};

struct BCGCBPRODLLEXPORT CBCGPElementColors
{
	CBCGPElementColors()
	{
		pBrFill = NULL;
		pBrLine = NULL;
		pBrText = NULL;
	}
	CBCGPBrush* pBrFill;
	CBCGPBrush* pBrLine;
	CBCGPBrush* pBrText;
};

class BCGCBPRODLLEXPORT CBCGPChartSeriesColorTheme
{
public:
	CBCGPChartSeriesColorTheme();
	~CBCGPChartSeriesColorTheme();

	void CreateFrom(const BCGPSeriesColors seriesColors[], double dblColorChangeStep);
	void GetColors(BCGPSeriesColorsPtr& colors, int nColorIndex, CBCGPBrush::BCGP_GRADIENT_TYPE type);

	void SetOpacity(double dblOpacity);

protected:
	void ApplyColors(BCGPSeriesColorsPtr& colors, BCGPSeriesColors* pSeriesColors);
	void RemoveAllColors();
	int GenerateColors(int nColorIndex, CBCGPBrush::BCGP_GRADIENT_TYPE type);

protected:
	CArray<BCGPSeriesColors*, BCGPSeriesColors*> m_arSeriesColors;
	int	m_nInitializedColors;
	double m_dblColorChangeStep;
};

struct BCGCBPRODLLEXPORT CBCGPChartTheme
{
public:
	enum ChartTheme
	{
		CT_DEFAULT,
		CT_PASTEL,
		CT_SPRING,
		CT_GREEN,
		CT_BLUE,
		CT_GOLD,
		CT_DARK_ROSE,
		CT_BLACK_AND_GOLD,
		CT_RAINBOW,
		CT_GRAY,
		CT_ARCTIC,
		CT_BLACK_AND_RED,
		CT_PLUM,
		CT_SUNNY,
		CT_VIOLET,
		CT_FLOWER,
		CT_STEEL,
		CT_GRAY_AND_GREEN,
		CT_OLIVE,
		CT_AUTUMN,
		CT_BLACK_AND_GREEN,
		CT_BLACK_AND_BLUE,
		CT_FLAT_2014_1,
		CT_FLAT_2014_2,
		CT_FLAT_2014_3,
		CT_FLAT_2014_4,
	};

	// Custom (user-defined) theme helper:
	static void InitChartColors(CBCGPChartTheme& theme, const CBCGPColor& colorFill, 
							 const CBCGPColor& colorOutline,
							 const CBCGPColor& colorTextIn = CBCGPColor(),
							 const CBCGPColor& colorSelIn = CBCGPColor(),
							 const CBCGPColor& colorPlotterIn = CBCGPColor(),
							 double dblInterlaceOpacity = .04,
							 BOOL bIsDarkBackground = FALSE);

	static void PrepareWallBrushes(CBCGPChartTheme& theme, const CBCGPBrush& brBase, BOOL bIsDarkBackground = FALSE);

	CBCGPChartTheme(ChartTheme chartTheme = CT_DEFAULT);
	CBCGPChartTheme(const CBCGPChartTheme& src);

	CBCGPBrush					m_brChartFillColor;
	CBCGPBrush					m_brChartLineColor;

	CBCGPBrush					m_brPlotFillColor;
	CBCGPBrush					m_brPlotLineColor;
	
	CBCGPBrush					m_brTitleTextColor;
	CBCGPBrush					m_brTitleFillColor;
	CBCGPBrush					m_brTitleLineColor;

	CBCGPBrush					m_brChartObjectTextColor;

	CBCGPBrush					m_brLegendEntryFillColor;
	CBCGPBrush					m_brLegendEntryLineColor;

	CBCGPBrush					m_brLegendFillColor;
	CBCGPBrush					m_brLegendLineColor;
	CBCGPBrush					m_brLegendEntryTextColor;
	
	CBCGPBrush					m_brSelectionFillColor;
	CBCGPBrush					m_brSelectionLineColor;

	CBCGPBrush					m_brAxisLineColor;
	CBCGPBrush					m_brAxisMajorGridLineColor;
	CBCGPBrush					m_brAxisMinorGridLineColor;

	CBCGPBrush					m_brAxisInterlaceColor;
	CBCGPBrush					m_brAxisInterlaceColor3D;
	CBCGPBrush					m_brAxisInterlaceColorGDI;
	CBCGPBrush					m_brAxisInterlaceColor3DGDI;

	CBCGPBrush					m_brFloorColor3D;
	CBCGPBrush					m_brLeftWallColor3D;
	CBCGPBrush					m_brRightWallColor3D;

	CBCGPBrush					m_brFloorColor3DTop;
	CBCGPBrush					m_brFloorColor3DSide;
	CBCGPBrush					m_brFloorColor3DBottom;

	CBCGPBrush					m_brLeftWallColor3DTop;
	CBCGPBrush					m_brLeftWallColor3DSide;
	CBCGPBrush					m_brLeftWallColor3DBottom;

	CBCGPBrush					m_brRightWallColor3DTop;
	CBCGPBrush					m_brRightWallColor3DSide;
	CBCGPBrush					m_brRightWallColor3DBottom;

	CBCGPBrush					m_brAxisLabelFillColor;
	CBCGPBrush					m_brAxisLabelLineColor;
	CBCGPBrush					m_brAxisLabelTextColor;
	
	CBCGPBrush					m_brAxisNameFillColor;
	CBCGPBrush					m_brAxisNameLineColor;
	CBCGPBrush					m_brAxisNameTextColor;

	CBCGPBrush					m_brScrollBarVert;
	CBCGPBrush					m_brScrollBarHorz;
	
	double						m_dblColorChangeStep;	
	BCGPSeriesColors			m_seriesColors[BCGP_CHART_NUM_SERIES_COLORS_IN_THEME];

	void SetTheme(CBCGPChartTheme::ChartTheme chartTheme);
	void SetOpacity(double dblOpacity);
	double GetOpacity() const {return m_dblOpacity;}

	const CBCGPChartTheme& operator=(const CBCGPChartTheme& src)
	{
		CopyFrom(src);
		return *this;
	}

	void CopyFrom(const CBCGPChartTheme& src);
	CBCGPChartTheme::ChartTheme GetThemeType() const {return m_themeType;}

	void GetSeriesColors(BCGPSeriesColorsPtr& seriesColors, int nColorIndex, CBCGPBrush::BCGP_GRADIENT_TYPE type) const;

	BOOL IsFlatTheme() const
	{
		return m_bIsFlatTheme;
	}
	
	void SetFlatTheme(BOOL bSet = TRUE)
	{
		m_bIsFlatTheme = bSet;
	}

protected:
	CBCGPChartTheme::ChartTheme		m_themeType;
	CBCGPChartSeriesColorTheme		m_seriesThemeColors;

	double							m_dblOpacity;
	BOOL							m_bIsFlatTheme;
};

#endif // !defined(AFX_BCGPCHARTFORMAT_H__32FC7098_1F70_4941_BA09_C5662C5F2878__INCLUDED_)