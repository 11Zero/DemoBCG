// Includes.h
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

#if !defined(__XTPCHARTCONTROLINCLUDES_H__)
#define __XTPCHARTCONTROLINCLUDES_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "XTPChartTitle.h"
#include "XTPChartSeriesView.h"
#include "XTPChartSeriesStyle.h"
#include "XTPChartSeriesPoint.h"
#include "XTPChartSeriesLabel.h"
#include "XTPChartSeries.h"
#include "XTPChartPanel.h"
#include "XTPChartLegend.h"
#include "XTPChartElementView.h"
#include "XTPChartElement.h"
#include "XTPChartDiagram3D.h"
#include "XTPChartDiagram.h"
#include "XTPChartDefines.h"
#include "XTPChartControl.h"
#include "XTPChartContent.h"
#include "XTPChartDefines.h"

#include "Styles/Pie/XTPChartPieSeriesStyle.h"
#include "Styles/Pie/XTPChartPie3DSeriesStyle.h"
#include "Styles/Pie/XTPChartTorus3DSeriesStyle.h"
#include "Styles/Pie/XTPChartPie3DDiagram.h"
#include "Styles/Pie/XTPChartPieDiagram.h"
#include "Styles/Pie/XTPChartPieSeriesLabel.h"
#include "Styles/Pie/XTPChartTorus3DDiagram.h"

#include "Styles/Funnel/XTPChartFunnelSeriesStyle.h"
#include "Styles/Funnel/XTPChartFunnelSeriesLabel.h"
#include "Styles/Funnel/XTPChartFunnelDiagram.h"

#include "Styles/Pyramid/XTPChartPyramidSeriesStyle.h"
#include "Styles/Pyramid/XTPChartPyramidSeriesLabel.h"
#include "Styles/Pyramid/XTPChartPyramidDiagram.h"

#include "Styles/Pyramid/XTPChartPyramid3DSeriesStyle.h"
#include "Styles/Pyramid/XTPChartPyramid3DDiagram.h"



#include "Styles/Point/XTPChartPointSeriesStyle.h"
#include "Styles/Point/XTPChartPointSeriesLabel.h"
#include "Styles/Point/XTPChartBubbleSeriesStyle.h"
#include "Styles/Point/XTPChartBubbleSeriesLabel.h"
#include "Styles/Point/XTPChartMarker.h"

#include "Styles/Line/XTPChartLineSeriesStyle.h"
#include "Styles/Line/XTPChartScatterLineSeriesStyle.h"
#include "Styles/Line/XTPChartFastLineSeriesStyle.h"
#include "Styles/Line/XTPChartStepLineSeriesStyle.h"
#include "Styles/Line/XTPChartSplineSeriesStyle.h"

#include "Styles/Area/XTPChartAreaSeriesStyle.h"
#include "Styles/Area/XTPChartStackedAreaSeriesStyle.h"
#include "Styles/Area/XTPChartSplineAreaSeriesStyle.h"
#include "Styles/Area/XTPChartStackedSplineAreaSeriesStyle.h"

#include "Styles/Bar/XTPChartBarSeriesStyle.h"
#include "Styles/Bar/XTPChartStackedBarSeriesStyle.h"
#include "Styles/Bar/XTPChartBarSeriesLabel.h"

#include "Styles/Range/XTPChartRangeBarSeriesStyle.h"
#include "Styles/Range/XTPChartRangeBarSeriesLabel.h"
#include "Styles/Range/XTPChartGanttSeriesStyle.h"

#include "Styles/Financial/XTPChartCandleStickSeriesStyle.h"
#include "Styles/Financial/XTPChartHighLowSeriesStyle.h"

#include "Drawing/XTPChartDeviceContext.h"
#include "Drawing/XTPChartDeviceCommand.h"
#include "Drawing/XTPChartLineDeviceCommand.h"
#include "Drawing/XTPChartOpenGLDeviceContext.h"
#include "Drawing/XTPChartPieDeviceCommand.h"
#include "Drawing/XTPChartRectangleDeviceCommand.h"
#include "Drawing/XTPChartTextDeviceCommand.h"
#include "Drawing/XTPChartTransformationDeviceCommand.h"

#include "Diagram2D/XTPChartDiagram2D.h"
#include "Diagram2D/XTPChartAxis.h"
#include "Diagram2D/XTPChartAxisLabel.h"
#include "Diagram2D/XTPChartAxisRange.h"
#include "Diagram2D/XTPChartAxisGridLines.h"
#include "Diagram2D/XTPChartAxisTitle.h"
#include "Diagram2D/XTPChartAxisConstantLines.h"
#include "Diagram2D/XTPChartAxisCustomLabels.h"
#include "Diagram2D/XTPChartAxisStrips.h"
#include "Diagram2D/XTPChartAxisTickMarks.h"
#include "Diagram2D/XTPChartDiagramPane.h"
#include "Diagram2D/XTPChartAxisView.h"
#include "Diagram2D/XTPChartScaleTypeMap.h"

#include "Appearance/XTPChartAppearance.h"
#include "Appearance/XTPChartLineStyle.h"
#include "Appearance/XTPChartBorder.h"
#include "Appearance/XTPChartPalette.h"
#include "Appearance/XTPChartFillStyle.h"

#include "Types/XTPChartDiagramPoint.h"
#include "Types/XTPChartMatrix.h"
#include "Types/XTPChartPie.h"
#include "Types/XTPChartTypes.h"

#include "Utils/XTPChartMathUtils.h"
#include "Utils/XTPChartTextPainter.h"

#endif //#if !defined(__XTPCHARTCONTROLINCLUDES_H__)
