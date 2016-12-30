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
// BCGPChartFormat.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPChartFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString BCGPChartFormatLabel::m_strDefaultFontFamily = _T("Calibri");

//*******************************************************************************
// Global method to generate 3D colors;
//*******************************************************************************
void BCGPChartFormatArea::Generate3DColorsFromBrush(const CBCGPBrush& brIn, CBCGPBrush& brTop, CBCGPBrush& brSide, CBCGPBrush& brBottom)
{
	brTop = brIn;
	brTop.MakeLighter(0.25);

	brSide = brIn;
	brSide.MakeDarker(0.15);

	brBottom = brIn;
	brBottom.MakeDarker(0.25);
}

//*******************************************************************************
// Series Colors
//*******************************************************************************
CBCGPChartSeriesColorTheme::CBCGPChartSeriesColorTheme()
{
	m_dblColorChangeStep = 0.2;
}
//*******************************************************************************
CBCGPChartSeriesColorTheme::~CBCGPChartSeriesColorTheme()
{
	RemoveAllColors();
}
//*******************************************************************************
void CBCGPChartSeriesColorTheme::RemoveAllColors()
{
	for (int i = 0; i < m_arSeriesColors.GetSize(); i++)
	{
		BCGPSeriesColors* pColors = m_arSeriesColors[i];
		if (pColors != NULL)
		{
			delete pColors;
		}
	}

	m_arSeriesColors.RemoveAll();
	m_nInitializedColors = 0;
}
//*******************************************************************************
void CBCGPChartSeriesColorTheme::SetOpacity(double dblOpacity)
{
	for (int i = 0; i < m_arSeriesColors.GetSize(); i++)
	{
		BCGPSeriesColors* pColors = m_arSeriesColors[i];
		if (pColors != NULL) 
		{
			pColors->m_brElementFillColor.SetColors(pColors->m_brElementFillColor.GetColor(), 
				pColors->m_brElementFillColor.GetGradientColor(), 
				pColors->m_brElementFillColor.GetGradientType(), dblOpacity);
			
			pColors->m_brElementTopFillColor.SetColors(pColors->m_brElementTopFillColor.GetColor(), 
				pColors->m_brElementTopFillColor.GetGradientColor(), 
				pColors->m_brElementTopFillColor.GetGradientType(), dblOpacity);
			
			pColors->m_brElementSideFillColor.SetColors(pColors->m_brElementSideFillColor.GetColor(), 
				pColors->m_brElementSideFillColor.GetGradientColor(), 
				pColors->m_brElementSideFillColor.GetGradientType(), dblOpacity);
			
			pColors->m_brElementBottomFillColor.SetColors(pColors->m_brElementBottomFillColor.GetColor(), 
				pColors->m_brElementBottomFillColor.GetGradientColor(), 
				pColors->m_brElementBottomFillColor.GetGradientType(), dblOpacity);
			
			pColors->m_brAlternativeFillColor.SetColors(pColors->m_brAlternativeFillColor.GetColor(), 
				pColors->m_brAlternativeFillColor.GetGradientColor(), 
				pColors->m_brAlternativeFillColor.GetGradientType(), dblOpacity);
		}
	}
}
//*******************************************************************************
void CBCGPChartSeriesColorTheme::CreateFrom(const BCGPSeriesColors seriesColors[], double dblColorChangeStop)
{
	RemoveAllColors();
	m_nInitializedColors = 0;
	m_dblColorChangeStep = dblColorChangeStop;

	for (int i = 0; i < BCGP_CHART_NUM_SERIES_COLORS_IN_THEME; i++)
	{
		if (seriesColors[i].IsEmpty())
		{
			return;
		}

		BCGPSeriesColors* pNewColors = new BCGPSeriesColors(seriesColors[i]);

		if (pNewColors->m_brElementLineColor.IsEmpty())
		{
			pNewColors->m_brElementLineColor = pNewColors->m_brElementFillColor;
		}

		BCGPChartFormatArea::Generate3DColorsFromBrush(pNewColors->m_brElementFillColor, 
			pNewColors->m_brElementTopFillColor, pNewColors->m_brElementSideFillColor, 
			pNewColors->m_brElementBottomFillColor);
		
		m_arSeriesColors.Add(pNewColors);
		m_nInitializedColors++;
	}
}
//*******************************************************************************
int CBCGPChartSeriesColorTheme::GenerateColors(int nColorIndex, CBCGPBrush::BCGP_GRADIENT_TYPE type)
{
	if (m_nInitializedColors == 0 || m_arSeriesColors.GetSize() == 0)
	{
		return -1;
	}

	if (nColorIndex < m_arSeriesColors.GetSize() && m_arSeriesColors[nColorIndex] != NULL)
	{
		BCGPSeriesColors* pBaseColors = m_arSeriesColors[nColorIndex];
		
		pBaseColors->m_brElementFillColor.SetColors(
			pBaseColors->m_brElementFillColor.GetColor(),
			pBaseColors->m_brElementFillColor.GetGradientColor(),
			type,
			pBaseColors->m_brElementFillColor.GetOpacity());
		
		pBaseColors->m_brElementTopFillColor.SetColors(pBaseColors->m_brElementTopFillColor.GetColor(),
			pBaseColors->m_brElementTopFillColor.GetGradientColor(),
			type,
			pBaseColors->m_brElementTopFillColor.GetOpacity());
		
		
		pBaseColors->m_brElementSideFillColor.SetColors(pBaseColors->m_brElementSideFillColor.GetColor(),
			pBaseColors->m_brElementSideFillColor.GetGradientColor(),
			type,
			pBaseColors->m_brElementSideFillColor.GetOpacity());
		
		pBaseColors->m_brElementBottomFillColor.SetColors(pBaseColors->m_brElementBottomFillColor.GetColor(),
			pBaseColors->m_brElementBottomFillColor.GetGradientColor(),
			type,
			pBaseColors->m_brElementBottomFillColor.GetOpacity());
		
		pBaseColors->m_brAlternativeFillColor.SetColors(
			pBaseColors->m_brAlternativeFillColor.GetColor(),
			pBaseColors->m_brAlternativeFillColor.GetGradientColor(),
			type,
			pBaseColors->m_brAlternativeFillColor.GetOpacity());

		return nColorIndex;
	}

	double dblChangeRatio = nColorIndex / m_nInitializedColors  * m_dblColorChangeStep;
	int nBaseIdx = nColorIndex % m_nInitializedColors;

	if (dblChangeRatio == 0)
	{
		dblChangeRatio = nColorIndex / m_arSeriesColors.GetSize() * m_dblColorChangeStep;
		nBaseIdx = nColorIndex % (int)m_arSeriesColors.GetSize() - 1;
	}

	if (nBaseIdx >= m_arSeriesColors.GetSize() || nBaseIdx < 0)
	{
		nBaseIdx = 0;
	}

	BCGPSeriesColors* pBaseColors = m_arSeriesColors[nBaseIdx]; 
	BCGPSeriesColors* pNewColors = new BCGPSeriesColors(*pBaseColors);

	pNewColors->m_brElementFillColor.SetColors(
		pBaseColors->m_brElementFillColor.GetColor(),
		pBaseColors->m_brElementFillColor.GetGradientColor(),
		type,
		pBaseColors->m_brElementFillColor.GetOpacity());

	if (pNewColors->m_brElementLineColor.IsEmpty())
	{
		pNewColors->m_brElementLineColor = pNewColors->m_brElementFillColor;
	}

	pNewColors->m_brElementFillColor.MakeDarker(dblChangeRatio);

	BCGPChartFormatArea::Generate3DColorsFromBrush(pNewColors->m_brElementFillColor, 
		pNewColors->m_brElementTopFillColor, pNewColors->m_brElementSideFillColor, 
		pNewColors->m_brElementBottomFillColor);
	
	pNewColors->m_brElementLineColor.MakeDarker(dblChangeRatio);

	pNewColors->m_brAlternativeFillColor.MakeDarker(dblChangeRatio);
	pNewColors->m_brAlternativeLineColor.MakeDarker(dblChangeRatio);

	pNewColors->m_brMarkerFillColor.MakeDarker(dblChangeRatio);
	pNewColors->m_brMarkerLineColor.MakeDarker(dblChangeRatio);

	m_arSeriesColors.SetAtGrow(nColorIndex, pNewColors);
	return nColorIndex;
}
//*******************************************************************************
void CBCGPChartSeriesColorTheme::GetColors(BCGPSeriesColorsPtr& colors, int nColorIndex, CBCGPBrush::BCGP_GRADIENT_TYPE type)
{
	int nIdx = GenerateColors(nColorIndex, type);
	if (nIdx >= 0)
	{
		ApplyColors(colors, m_arSeriesColors [nIdx]);
	}
}
//*******************************************************************************
void CBCGPChartSeriesColorTheme::ApplyColors(BCGPSeriesColorsPtr& colors, BCGPSeriesColors* pSeriesColors)
{
	if (colors.m_pBrElementFillColor == NULL || colors.m_pBrElementFillColor->IsEmpty())
	{
		colors.m_pBrElementFillColor = (CBCGPBrush*)&pSeriesColors->m_brElementFillColor;
	}

	if (colors.m_pBrElementTopFillColor == NULL || colors.m_pBrElementTopFillColor->IsEmpty())
	{
		colors.m_pBrElementTopFillColor = (CBCGPBrush*)&pSeriesColors->m_brElementTopFillColor;
	}

	if (colors.m_pBrElementSideFillColor == NULL || colors.m_pBrElementSideFillColor->IsEmpty())
	{
		colors.m_pBrElementSideFillColor = (CBCGPBrush*)&pSeriesColors->m_brElementSideFillColor;
	}

	if (colors.m_pBrElementBottomFillColor == NULL || colors.m_pBrElementBottomFillColor->IsEmpty())
	{
		colors.m_pBrElementBottomFillColor = (CBCGPBrush*)&pSeriesColors->m_brElementBottomFillColor;
	}

	if (colors.m_pBrElementLineColor == NULL || colors.m_pBrElementLineColor->IsEmpty())
	{
		colors.m_pBrElementLineColor = (CBCGPBrush*)&pSeriesColors->m_brElementLineColor;
	}

	if (colors.m_pBrMarkerFillColor == NULL || colors.m_pBrMarkerFillColor->IsEmpty())
	{
		colors.m_pBrMarkerFillColor = (CBCGPBrush*)&pSeriesColors->m_brMarkerFillColor;
	}

	if (colors.m_pBrMarkerLineColor == NULL || colors.m_pBrMarkerLineColor->IsEmpty())
	{
		colors.m_pBrMarkerLineColor = (CBCGPBrush*)&pSeriesColors->m_brMarkerLineColor;
	}

	if (colors.m_pBrDataLabelFillColor == NULL || colors.m_pBrDataLabelFillColor->IsEmpty())
	{
		colors.m_pBrDataLabelFillColor = (CBCGPBrush*)&pSeriesColors->m_brDataLabelFillColor;
	}

	if (colors.m_pBrDataLabelLineColor == NULL || colors.m_pBrDataLabelLineColor->IsEmpty())
	{
		colors.m_pBrDataLabelLineColor = (CBCGPBrush*)&pSeriesColors->m_brDataLabelLineColor;
	}

	if (colors.m_pBrDataLabelTextColor == NULL || colors.m_pBrDataLabelTextColor->IsEmpty())
	{
		colors.m_pBrDataLabelTextColor = (CBCGPBrush*)&pSeriesColors->m_brDataLabelTextColor;
	}

	if (colors.m_pBrElementAltFillColor == NULL || colors.m_pBrElementAltFillColor->IsEmpty())
	{
		colors.m_pBrElementAltFillColor = (CBCGPBrush*)&pSeriesColors->m_brAlternativeFillColor;
	}

	if (colors.m_pBrElementAltLineColor == NULL || colors.m_pBrElementAltLineColor->IsEmpty())
	{
		colors.m_pBrElementAltLineColor = (CBCGPBrush*)&pSeriesColors->m_brAlternativeLineColor;
	}
}
//*******************************************************************************
// Global Colors Colors
//*******************************************************************************
CBCGPChartTheme::CBCGPChartTheme(CBCGPChartTheme::ChartTheme chartTheme)
{
	SetTheme(chartTheme);
	m_dblOpacity = 1.;
	m_bIsFlatTheme = FALSE;
}
//*******************************************************************************
CBCGPChartTheme::CBCGPChartTheme(const CBCGPChartTheme& src)
{
	CopyFrom(src);
}
//*******************************************************************************

static CBCGPColor colorNull;

void CBCGPChartTheme::InitChartColors(CBCGPChartTheme& theme, const CBCGPColor& colorFill, 
							 const CBCGPColor& colorOutline,
							 const CBCGPColor& colorTextIn,
							 const CBCGPColor& colorSelIn,
							 const CBCGPColor& colorPlotterIn,
							 double dblInterlaceOpacity,
							 BOOL bIsDarkBackground)
{
	CBCGPColor colorText = colorTextIn;
	if (colorText.IsNull())
	{
		colorText = colorOutline;
	}

	theme.m_brChartFillColor.SetColor(colorFill);
	theme.m_brChartLineColor.Empty();

	theme.m_brAxisLineColor.SetColor(colorOutline);

	theme.m_brAxisMajorGridLineColor.SetColor(colorOutline);
	theme.m_brAxisMinorGridLineColor.SetColor(colorOutline);

	if (bIsDarkBackground)
	{
		theme.m_brAxisLineColor.MakeDarker(.2);
		theme.m_brAxisMajorGridLineColor.MakeDarker(.2);
		theme.m_brAxisMinorGridLineColor.MakeDarker(.2);
	}
	else
	{
		theme.m_brAxisLineColor.MakeLighter(.2);
		theme.m_brAxisMajorGridLineColor.MakePale(.85);
		theme.m_brAxisMinorGridLineColor.MakePale(.9);
	}

	if (bIsDarkBackground)
	{
		theme.m_brAxisInterlaceColor.SetColor(colorPlotterIn.IsNull() ? colorFill : colorPlotterIn);
		theme.m_brAxisInterlaceColor.MakeLighter(dblInterlaceOpacity);
	}
	else
	{
		theme.m_brAxisInterlaceColor.SetColor(colorOutline, dblInterlaceOpacity);
	}

	if (colorPlotterIn.IsNull())
	{
		theme.m_brPlotFillColor = theme.m_brAxisInterlaceColor;
		theme.m_brPlotFillColor.MakePale(.99);

		theme.m_brAxisInterlaceColorGDI.SetColor(colorOutline);
		theme.m_brAxisInterlaceColorGDI.MakePale(.97);
	}
	else
	{
		theme.m_brPlotFillColor.SetColor(colorPlotterIn);

		theme.m_brAxisInterlaceColorGDI.SetColor(colorPlotterIn);
		theme.m_brAxisInterlaceColorGDI.MakeLighter();
	}

	theme.m_brAxisInterlaceColor3D = theme.m_brAxisInterlaceColor;
	theme.m_brAxisInterlaceColor3DGDI = theme.m_brAxisInterlaceColorGDI;

	if (bIsDarkBackground)
	{
		theme.m_brAxisInterlaceColor3DGDI.MakeLighter(dblInterlaceOpacity / 2);
		theme.m_brAxisInterlaceColor3D.MakeLighter(dblInterlaceOpacity / 2);
	}
	else
	{
		theme.m_brAxisInterlaceColor3DGDI.MakeDarker(.04);
		theme.m_brAxisInterlaceColor3D.MakeDarker(.04);
	}

	PrepareWallBrushes(theme, theme.m_brPlotFillColor, bIsDarkBackground);

	theme.m_brPlotLineColor = theme.m_brAxisMajorGridLineColor;

	theme.m_brTitleFillColor.Empty();
	theme.m_brTitleLineColor.Empty();
	theme.m_brTitleTextColor.SetColor(colorText);

	theme.m_brChartObjectTextColor.SetColor(colorText);

	theme.m_brLegendEntryFillColor.SetColor(theme.m_brPlotFillColor.GetColor(), 0.75);;
	theme.m_brLegendEntryLineColor.Empty();

	theme.m_brLegendFillColor.SetColor(theme.m_brPlotFillColor.GetColor(), 0.75);
	theme.m_brLegendLineColor = theme.m_brAxisMajorGridLineColor;
	theme.m_brLegendEntryTextColor.SetColor(colorText);

	if (colorSelIn.IsNull())
	{
		theme.m_brSelectionFillColor.SetColor(colorOutline, 0.5);
		theme.m_brSelectionFillColor.MakePale(.8);
	}
	else
	{
		theme.m_brSelectionFillColor.SetColor(colorSelIn, 0.5);
	}

	theme.m_brSelectionLineColor = theme.m_brSelectionFillColor;
	theme.m_brSelectionLineColor.MakeDarker(.5);

	theme.m_brAxisLabelFillColor.Empty();
	theme.m_brAxisLabelLineColor.Empty();
	theme.m_brAxisLabelTextColor.SetColor(colorText);

	theme.m_brAxisNameFillColor.Empty();
	theme.m_brAxisNameLineColor.Empty();
	theme.m_brAxisNameTextColor.SetColor(colorText);

	CBCGPColor clrScrollBar = theme.m_brSelectionFillColor.GetColor();
	CBCGPColor clrScrollBarLight = clrScrollBar;
	clrScrollBarLight.MakePale();

	theme.m_brScrollBarVert.SetColors(clrScrollBarLight, clrScrollBar, CBCGPBrush::BCGP_GRADIENT_VERTICAL);
	theme.m_brScrollBarHorz.SetColors(clrScrollBar, clrScrollBarLight, CBCGPBrush::BCGP_GRADIENT_HORIZONTAL);
}
//********************************************************************************
void CBCGPChartTheme::PrepareWallBrushes(CBCGPChartTheme& theme, const CBCGPBrush& brBase, BOOL bIsDarkBackground)
{
	CBCGPColor clrWall1 = theme.m_brPlotFillColor.GetColor();
	clrWall1.MakeDarker();
	
	CBCGPColor clrWall2 = theme.m_brPlotFillColor.GetColor();
	clrWall2.MakeLighter();
	
	if (brBase.HasTextureImage())
	{
		theme.m_brLeftWallColor3D = brBase;
		theme.m_brLeftWallColor3D.MakeDarker();

		theme.m_brRightWallColor3D = brBase;
		theme.m_brRightWallColor3D.MakeLighter();
	}
	else
	{
		theme.m_brLeftWallColor3D.SetColors(clrWall1, clrWall2, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP_RIGHT);
		theme.m_brRightWallColor3D.SetColors(clrWall1, clrWall2, CBCGPBrush::BCGP_GRADIENT_RADIAL_TOP_RIGHT);
	}
	
	if (bIsDarkBackground)
	{
		theme.m_brLeftWallColor3D.MakeLighter(0.015);
		clrWall1.MakeLighter(.12);
	}
	else
	{
		theme.m_brLeftWallColor3D.MakeDarker(0.015);
		clrWall1.MakeDarker(.12);
	}
	
	if (brBase.HasTextureImage())
	{
		theme.m_brFloorColor3D = brBase;
	}
	else
	{
		theme.m_brFloorColor3D.SetColors(clrWall1, clrWall2, CBCGPBrush::BCGP_GRADIENT_RADIAL_BOTTOM_LEFT);
	}
	
	BCGPChartFormatArea::Generate3DColorsFromBrush(theme.m_brLeftWallColor3D, 
		theme.m_brLeftWallColor3DTop, theme.m_brLeftWallColor3DSide, theme.m_brLeftWallColor3DBottom);	
	BCGPChartFormatArea::Generate3DColorsFromBrush(theme.m_brRightWallColor3D, 
		theme.m_brRightWallColor3DTop, theme.m_brRightWallColor3DSide, theme.m_brRightWallColor3DBottom);	
	BCGPChartFormatArea::Generate3DColorsFromBrush(theme.m_brFloorColor3D, 
		theme.m_brFloorColor3DTop, theme.m_brFloorColor3DSide, theme.m_brFloorColor3DBottom);	
	
	if (brBase.HasTextureImage())
	{
		theme.m_brLeftWallColor3DTop = brBase;
		theme.m_brRightWallColor3DTop = brBase;
	}
	else
	{
		theme.m_brLeftWallColor3DTop = clrWall1;
		theme.m_brRightWallColor3DTop = clrWall2;
	}

	theme.m_brLeftWallColor3DTop.MakeLighter();
	theme.m_brRightWallColor3DTop.MakeLighter();
	theme.m_brFloorColor3DTop.MakeDarker(0.07);
}
//********************************************************************************
static void InitSeriesColors2(BCGPSeriesColors& sc, const CBCGPColor& color, 
							 const CBCGPColor& colorGradientIn = colorNull,
							 const CBCGPColor& colorTextIn = colorNull,
							 const CBCGPColor& colorOutlineIn = colorNull)
{
	CBCGPColor colorGradient = colorGradientIn;

	if (colorGradient.IsNull())
	{
		colorGradient = color;
		colorGradient.MakeLighter(.5);
	}

	sc.m_brElementFillColor.SetColors(color, colorGradient, (CBCGPBrush::BCGP_GRADIENT_TYPE)-1);

	BCGPChartFormatArea::Generate3DColorsFromBrush(sc.m_brElementFillColor, sc.m_brElementTopFillColor, 
		sc.m_brElementSideFillColor, sc.m_brElementBottomFillColor);

	CBCGPColor colorOutline = colorOutlineIn;
	if (colorOutline.IsNull())
	{
		colorOutline = color;
		colorOutline.MakeDarker(.1);
	}

	sc.m_brElementLineColor.SetColor(colorOutline);
	sc.m_brMarkerFillColor.SetColors(color, colorGradient, CBCGPBrush::BCGP_GRADIENT_DIAGONAL_LEFT);
	sc.m_brMarkerFillColor.MakeLighter(.15);

	sc.m_brMarkerLineColor.SetColor(colorOutline);
	sc.m_brDataLabelLineColor.SetColor(colorOutline);

	sc.m_brDataLabelFillColor.SetColor(color);
	sc.m_brDataLabelFillColor.MakePale();

	sc.m_brAlternativeFillColor = sc.m_brElementFillColor; 
	sc.m_brAlternativeFillColor.MakeDarker(.2);

	sc.m_brAlternativeLineColor = sc.m_brElementLineColor;
	sc.m_brAlternativeLineColor.MakeDarker(.2);

	CBCGPColor colorText = colorTextIn;
	if (colorText.IsNull())
	{
		colorText = color;
		colorText.MakeDarker(.2);
	}

	sc.m_brDataLabelTextColor.SetColor(colorText);
}
//*******************************************************************************
static void InitSeriesColors(BCGPSeriesColors& sc, const CBCGPColor& color, const CBCGPColor& colorAlt = colorNull)
{
	InitSeriesColors2(sc, color);

	if (colorAlt == colorNull)
	{
		return;
	}

	CBCGPColor colorGradient = colorAlt;
	colorGradient.MakeLighter(.5);

	sc.m_brAlternativeFillColor.SetColors(colorAlt, colorGradient, (CBCGPBrush::BCGP_GRADIENT_TYPE)-1);

	CBCGPColor colorOutline = colorAlt;
	colorOutline.MakeDarker(.1);

	sc.m_brAlternativeLineColor.SetColor(colorOutline);
}
//********************************************************************************
void CBCGPChartTheme::SetTheme(CBCGPChartTheme::ChartTheme chartTheme)
{
	m_themeType = chartTheme;

	for (int i = 0; i < BCGP_CHART_NUM_SERIES_COLORS_IN_THEME; i++)
	{
		m_seriesColors[i].Empty();
	}

	m_dblColorChangeStep = 0.15;
	m_bIsFlatTheme = FALSE;

	CBCGPColor colorPlotBlack(CBCGPColor::Black);
	colorPlotBlack.MakeLighter(.3);

	switch (chartTheme)
	{
	case CT_DEFAULT:
		InitChartColors(*this, CBCGPColor::White, CBCGPColor::Black, colorNull, CBCGPColor::LightBlue);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::RoyalBlue, CBCGPColor::LightSkyBlue);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::IndianRed);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::DarkOliveGreen);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::BlueViolet);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::Goldenrod);
		break;

	case CT_PASTEL:
		InitChartColors(*this, CBCGPColor::White, CBCGPColor::DarkKhaki, CBCGPColor::DarkSlateBlue, colorNull, colorNull, .06);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::CadetBlue, CBCGPColor::DarkTurquoise);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::RosyBrown);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::DarkSeaGreen);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::SlateBlue);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::Peru);
		break;

	case CT_SPRING:
		InitChartColors(*this, CBCGPColor::White, CBCGPColor::DarkSlateGray, CBCGPColor::Black);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::SlateGray, CBCGPColor::PowderBlue);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::CadetBlue);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::Goldenrod);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::DarkSeaGreen);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::DodgerBlue);
		break;

	case CT_GREEN:
		InitChartColors(*this, CBCGPColor::White, CBCGPColor::DarkOliveGreen);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::DarkGreen, CBCGPColor::MediumSeaGreen);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::DarkOliveGreen);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::DarkKhaki);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::DarkSlateGray);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::OliveDrab);
		break;

	case CT_BLUE:
		InitChartColors(*this, CBCGPColor::White, CBCGPColor::CadetBlue, CBCGPColor::DarkBlue);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::DodgerBlue, CBCGPColor::SkyBlue);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::CadetBlue);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::Teal);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::SteelBlue);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::SlateGray);
		break;

	case CT_GOLD:
		InitChartColors(*this, CBCGPColor::White, CBCGPColor::DarkGoldenrod, CBCGPColor::DarkRed, colorNull, colorNull, .06);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::DarkGoldenrod, CBCGPColor::SandyBrown);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::SandyBrown);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::BurlyWood);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::Goldenrod);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::DarkSalmon);
		break;

	case CT_DARK_ROSE:
		InitChartColors(*this, CBCGPColor::White, CBCGPColor::DarkGoldenrod, CBCGPColor::DarkRed, colorNull, colorNull, .06);
		InitSeriesColors(m_seriesColors[0], CBCGPColor::RosyBrown, CBCGPColor::PeachPuff);
		break;

	case CT_BLACK_AND_GOLD:
		InitChartColors(*this, CBCGPColor::Black, CBCGPColor::Sienna, CBCGPColor::Goldenrod, colorNull, colorPlotBlack, .2, TRUE);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::DarkGoldenrod, CBCGPColor::SandyBrown);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::SandyBrown);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::BurlyWood);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::Goldenrod);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::DarkSalmon);
		break;

	case CT_RAINBOW:
		InitChartColors(*this, CBCGPColor::White, CBCGPColor::Black);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::DarkRed, CBCGPColor::DarkOrange);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::Goldenrod);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::CadetBlue);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::DarkGreen);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::DarkViolet);
		break;

	case CT_GRAY:
		InitChartColors(*this, CBCGPColor::WhiteSmoke, CBCGPColor::Black);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::Gray);
		InitSeriesColors2(m_seriesColors[1], CBCGPColor::Silver, colorNull, CBCGPColor::DimGray, CBCGPColor::DarkGray);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::DimGray);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::DarkGray);
		InitSeriesColors2(m_seriesColors[4], CBCGPColor::LightGray, colorNull, CBCGPColor::DimGray, CBCGPColor::DarkGray);
		break;

	case CT_ARCTIC:
		InitChartColors(*this, CBCGPColor::White, CBCGPColor::SteelBlue, colorNull, colorNull, colorNull, .06);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::LightSlateGray, CBCGPColor::LightSteelBlue);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::BurlyWood);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::Teal);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::DarkSeaGreen);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::LightSteelBlue);
		break;

	case CT_BLACK_AND_RED:
		InitChartColors(*this, CBCGPColor::Black, CBCGPColor::Brown, CBCGPColor::LightCoral, colorNull, colorPlotBlack, .2, TRUE);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::IndianRed, CBCGPColor::LightSalmon);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::Black);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::Firebrick);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::DimGray);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::Red);
		break;

	case CT_PLUM:
		InitChartColors(*this, CBCGPColor::White, CBCGPColor::SeaGreen, CBCGPColor::DarkGreen, colorNull, colorNull, .06);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::RosyBrown, CBCGPColor::PeachPuff);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::CadetBlue);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::DarkSlateGray);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::Goldenrod);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::DarkSlateBlue);
		break;

	case CT_SUNNY:
		InitChartColors(*this, CBCGPColor::White, CBCGPColor::Goldenrod, CBCGPColor::DarkGoldenrod, colorNull, colorNull, .06);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::Red, CBCGPColor::Orange);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::DodgerBlue);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::Green);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::Goldenrod);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::PaleVioletRed);
		break;

	case CT_VIOLET:
		InitChartColors(*this, CBCGPColor::WhiteSmoke, CBCGPColor::Violet, CBCGPColor::Purple, colorNull, colorNull, .06);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::BlueViolet, CBCGPColor::Orchid);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::HotPink);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::MediumPurple);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::Violet);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::Thistle);
		break;

	case CT_FLOWER:
		InitChartColors(*this, CBCGPColor::WhiteSmoke, CBCGPColor::SteelBlue, CBCGPColor::Teal, colorNull, colorNull, .05);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::DarkViolet, CBCGPColor::Fuchsia);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::OliveDrab);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::Maroon);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::Blue);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::Orange);
		break;

	case CT_STEEL:
		InitChartColors(*this, CBCGPColor::White, CBCGPColor::SteelBlue, CBCGPColor::DimGray);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::LightSlateGray, CBCGPColor::LightSteelBlue);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::DimGray);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::SlateGray);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::SteelBlue);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::LightSteelBlue);
		break;

	case CT_GRAY_AND_GREEN:
		InitChartColors(*this, CBCGPColor::White, CBCGPColor::SteelBlue, CBCGPColor::DimGray);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::LightSlateGray, CBCGPColor::LightSteelBlue);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::Gray);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::DarkSeaGreen);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::DarkKhaki);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::LightSteelBlue);
		break;

	case CT_OLIVE:
		InitChartColors(*this, CBCGPColor::PaleGoldenrod, CBCGPColor::DarkOliveGreen, colorNull, colorNull, CBCGPColor::LemonChiffon, .05);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::Olive, CBCGPColor::Khaki);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::DarkOliveGreen);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::DarkSeaGreen);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::DarkKhaki);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::SlateGray);
		break;

	case CT_AUTUMN:
		InitChartColors(*this, CBCGPColor::White, CBCGPColor::DarkOliveGreen);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::Maroon, CBCGPColor::Peru);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::Goldenrod);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::LightSlateGray);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::Sienna);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::OliveDrab);
		break;

	case CT_BLACK_AND_GREEN:
		InitChartColors(*this, CBCGPColor::Black, CBCGPColor::DarkOliveGreen, CBCGPColor::YellowGreen, colorNull, CBCGPColor::Black, 0.1, TRUE);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::YellowGreen);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::MediumSeaGreen);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::Green);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::LimeGreen);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::DarkGreen);
		break;

	case CT_BLACK_AND_BLUE:
		InitChartColors(*this, CBCGPColor::Black, CBCGPColor::SlateGray, CBCGPColor::CadetBlue, colorNull, CBCGPColor::Black, 0.1, TRUE);

		InitSeriesColors(m_seriesColors[0], CBCGPColor::DeepSkyBlue);
		InitSeriesColors(m_seriesColors[1], CBCGPColor::DodgerBlue);
		InitSeriesColors(m_seriesColors[2], CBCGPColor::CornflowerBlue);
		InitSeriesColors(m_seriesColors[3], CBCGPColor::SteelBlue);
		InitSeriesColors(m_seriesColors[4], CBCGPColor::DarkSlateGray);
		break;

	case CT_FLAT_2014_1:
		m_bIsFlatTheme = TRUE;

		InitChartColors(*this, CBCGPColor::White, CBCGPColor::DimGray, colorNull, CBCGPColor::LightBlue);

		InitSeriesColors(m_seriesColors[0], RGB(91, 155, 213));
		InitSeriesColors(m_seriesColors[1], RGB(237, 125, 49));
		InitSeriesColors(m_seriesColors[2], RGB(112, 173, 71));
		InitSeriesColors(m_seriesColors[3], RGB(165, 165, 165));
		InitSeriesColors(m_seriesColors[4], RGB(255, 192, 0));
		break;

	case CT_FLAT_2014_2:
		m_bIsFlatTheme = TRUE;

		InitChartColors(*this, CBCGPColor::White, CBCGPColor::DimGray, colorNull, CBCGPColor::LightBlue);

		InitSeriesColors(m_seriesColors[0], RGB(237, 125, 49));
		InitSeriesColors(m_seriesColors[1], RGB(255, 192, 0));
		InitSeriesColors(m_seriesColors[2], RGB(112, 173, 71));
		InitSeriesColors(m_seriesColors[3], RGB(158, 72, 14));
		InitSeriesColors(m_seriesColors[4], RGB(99, 99, 99));
		break;

	case CT_FLAT_2014_3:
		m_bIsFlatTheme = TRUE;

		InitChartColors(*this, CBCGPColor::White, CBCGPColor::DimGray, colorNull, CBCGPColor::LightBlue);

		InitSeriesColors(m_seriesColors[0], RGB(112, 173, 71));
		InitSeriesColors(m_seriesColors[1], RGB(68, 114, 196));
		InitSeriesColors(m_seriesColors[2], RGB(255, 192, 0));
		InitSeriesColors(m_seriesColors[3], RGB(67, 104, 43));
		InitSeriesColors(m_seriesColors[4], RGB(38, 68, 120));
		break;

	case CT_FLAT_2014_4:
		m_bIsFlatTheme = TRUE;

		InitChartColors(*this, CBCGPColor::White, CBCGPColor::DimGray, colorNull, CBCGPColor::LightBlue);

		InitSeriesColors(m_seriesColors[0], RGB(0, 114, 198));
		InitSeriesColors(m_seriesColors[1], RGB(255, 128, 87));
		InitSeriesColors(m_seriesColors[2], RGB(60, 179, 113));
		InitSeriesColors(m_seriesColors[3], RGB(163, 0, 192));
		InitSeriesColors(m_seriesColors[4], RGB(165, 165, 165));
		break;
	}

	m_seriesThemeColors.CreateFrom(m_seriesColors, m_dblColorChangeStep);
}
//*******************************************************************************
void CBCGPChartTheme::SetOpacity(double dblOpacity)
{
	m_seriesThemeColors.SetOpacity(dblOpacity);
	m_dblOpacity = dblOpacity;
}
//*******************************************************************************
void CBCGPChartTheme::CopyFrom(const CBCGPChartTheme& src)
{
	m_brChartFillColor = src.m_brChartFillColor;
	m_brChartLineColor = src.m_brChartLineColor;

	m_brPlotFillColor = src.m_brPlotFillColor;
	m_brPlotLineColor = src.m_brPlotLineColor;

	m_brTitleTextColor = src.m_brTitleTextColor;
	m_brTitleFillColor = src.m_brTitleFillColor;
	m_brTitleLineColor = src.m_brTitleLineColor;

	m_brChartObjectTextColor = src.m_brChartObjectTextColor;	

	m_brLegendEntryFillColor = src.m_brLegendEntryFillColor;
	m_brLegendEntryLineColor = src.m_brLegendEntryLineColor;

	m_brLegendFillColor = src.m_brLegendFillColor;
	m_brLegendLineColor = src.m_brLegendLineColor;
	m_brLegendEntryTextColor = src.m_brLegendEntryTextColor;

	m_brSelectionFillColor = src.m_brSelectionFillColor;
	m_brSelectionLineColor = src.m_brSelectionLineColor;

	m_brAxisLineColor = src.m_brAxisLineColor;
	m_brAxisMajorGridLineColor = src.m_brAxisMajorGridLineColor;
	m_brAxisMinorGridLineColor = src.m_brAxisMinorGridLineColor;

	m_brAxisLabelFillColor = src.m_brAxisLabelFillColor;
	m_brAxisLabelLineColor = src.m_brAxisLabelLineColor;
	m_brAxisLabelTextColor = src.m_brAxisLabelTextColor;

	m_brAxisNameFillColor = src.m_brAxisNameFillColor;
	m_brAxisNameLineColor = src.m_brAxisNameLineColor;
	m_brAxisNameTextColor = src.m_brAxisNameTextColor;

	m_brScrollBarVert = src.m_brScrollBarVert;
	m_brScrollBarHorz = src.m_brScrollBarHorz;

	m_brAxisInterlaceColor = src.m_brAxisInterlaceColor;
	m_brAxisInterlaceColor3D = src.m_brAxisInterlaceColor3D;
	m_brAxisInterlaceColorGDI = src.m_brAxisInterlaceColorGDI;
	m_brAxisInterlaceColor3DGDI = src.m_brAxisInterlaceColor3DGDI;

	m_brFloorColor3D = src.m_brFloorColor3D;
	m_brLeftWallColor3D = src.m_brLeftWallColor3D;
	m_brRightWallColor3D = src.m_brRightWallColor3D;

	m_brFloorColor3DTop = src.m_brFloorColor3DTop;
	m_brFloorColor3DSide = src.m_brFloorColor3DSide;
	m_brFloorColor3DBottom = src.m_brFloorColor3DBottom;

	m_brLeftWallColor3DTop = src.m_brLeftWallColor3DTop;
	m_brLeftWallColor3DSide = src.m_brLeftWallColor3DSide;
	m_brLeftWallColor3DBottom = src.m_brLeftWallColor3DBottom;

	m_brRightWallColor3DTop = src.m_brRightWallColor3DTop;
	m_brRightWallColor3DSide = src.m_brRightWallColor3DSide;
	m_brRightWallColor3DBottom = src.m_brRightWallColor3DBottom;

	for (int i = 0; i < BCGP_CHART_NUM_SERIES_COLORS_IN_THEME; i++)
	{
		m_seriesColors[i] = src.m_seriesColors[i];
	}

	m_dblColorChangeStep = src.m_dblColorChangeStep;
	m_seriesThemeColors.CreateFrom(m_seriesColors, m_dblColorChangeStep);

	m_dblOpacity = src.GetOpacity();
	m_bIsFlatTheme = src.m_bIsFlatTheme;
}

void CBCGPChartTheme::GetSeriesColors(BCGPSeriesColorsPtr& seriesColors, int nColorIndex, CBCGPBrush::BCGP_GRADIENT_TYPE type) const
{
	CBCGPChartSeriesColorTheme& theme = (CBCGPChartSeriesColorTheme&) m_seriesThemeColors;
	theme.GetColors(seriesColors, nColorIndex, type);
}
