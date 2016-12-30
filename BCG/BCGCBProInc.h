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
//

#ifndef __BCGCBPROINC_H
#define __BCGCBPROINC_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXTEMPL_H__
#include <afxtempl.h>
#endif

#include "BCGCBPro.h"		// Library global definitions.
#include "BCGCBProVer.h"	// Library version info.

#ifdef _BCGCBPRO_IN_OTHER_DLL
BCGCBPRODLLEXPORT void BCGCBProDllInitialize ();
#endif // _BCGCBPRO_IN_OTHER_DLL

#define _BCGCB_LIBNAME1_	"BCGCBPRO2210"

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_
	#define _BCGCB_LIBNAME2_	_BCGCB_LIBNAME1_
#else
	#define _BCGCB_LIBNAME2_	_BCGCB_LIBNAME1_##"Static"
#endif

#ifdef _UNICODE
	#define _BCGCB_LIBNAME3_	_BCGCB_LIBNAME2_##"U"
#else
	#define _BCGCB_LIBNAME3_	_BCGCB_LIBNAME2_
#endif

#ifdef _DEBUG
	#define _BCGCB_LIBNAME4_	_BCGCB_LIBNAME3_##"D"
#else
	#define _BCGCB_LIBNAME4_	_BCGCB_LIBNAME3_
#endif

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_
	#define _BCGCB_LIBNAME5_	_BCGCB_LIBNAME4_
#elif defined _BCGCBPRO_STATIC_
	#define _BCGCB_LIBNAME5_	_BCGCB_LIBNAME4_##"s"
#else
	#define _BCGCB_LIBNAME5_	_BCGCB_LIBNAME4_
#endif

#if _MSC_VER < 1300
	#define _BCGCB_LIBNAME_	_BCGCB_LIBNAME5_##".lib"
#elif _MSC_VER == 1300
	#define _BCGCB_LIBNAME_	_BCGCB_LIBNAME5_##"70.lib"
#elif _MSC_VER < 1400
	#define _BCGCB_LIBNAME_	_BCGCB_LIBNAME5_##"71.lib"
#elif _MSC_VER < 1500
	#define _BCGCB_LIBNAME_	_BCGCB_LIBNAME5_##"80.lib"
#elif _MSC_VER == 1500
	#define _BCGCB_LIBNAME_	_BCGCB_LIBNAME5_##"90.lib"
#elif _MSC_VER == 1600
	#define _BCGCB_LIBNAME_	_BCGCB_LIBNAME5_##"100.lib"
#elif _MSC_VER == 1700
	#define _BCGCB_LIBNAME_	_BCGCB_LIBNAME5_##"110.lib"
#elif _MSC_VER == 1800
	#define _BCGCB_LIBNAME_	_BCGCB_LIBNAME5_##"120.lib"
#else
	#error "Unsupported VC++ version"
#endif

#ifndef _BCGCB_DISABLE_AUTOLINK_
	#pragma comment(lib, _BCGCB_LIBNAME_)

	#ifndef _BCGCB_HIDE_AUTOLINK_OUTPUT_
	#pragma message("Automatically linking with " _BCGCB_LIBNAME_)
	#endif	// _BCGCB_HIDE_AUTOLINK_OUTPUT_
#endif // _BCGCB_DISABLE_AUTOLINK_

//----------------------------
// BCGControlBar control bars:
//----------------------------
#include "BCGPToolBar.h"
#include "BCGPStatusBar.h"
#include "BCGPMenuBar.h"
#include "BCGPDialogBar.h"
#include "BCGPOutlookBar.h"
#include "BCGPOutlookWnd.h"
#include "BCGPColorBar.h"
#include "BCGPCaptionBar.h"
#include "BCGPOutlookBarPane.h"
#include "BCGPTabbedControlBar.h"
#include "BCGPTabbedToolbar.h"
#include "BCGPToolBox.h"
#include "BCGPTasksPane.h"
#include "BCGPCalendarBar.h"
#include "BCGPCalculator.h"
#include "BCGPExplorerToolBar.h"

//--------------------------
// BCGControlBar Ribbon bar:
//--------------------------
#include "BCGPRibbonBar.h"
#include "BCGPRibbonCustomizePage.h"
#include "BCGPRibbonCustomizeQATPage.h"
#include "BCGPRibbonToolsPage.h"
#include "BCGPRibbonKeyboardCustomizeDlg.h"
#include "BCGPRibbonPanel.h"
#include "BCGPRibbonMainPanel.h"
#include "BCGPRibbonBackstageViewPanel.h"
#include "BCGPRibbonPanelMenu.h"
#include "BCGPRibbonFloaty.h"
#include "BCGPRibbonCategory.h"
#include "BCGPRibbonButton.h"
#include "BCGPRibbonLabel.h"
#include "BCGPBaseRibbonElement.h"
#include "BCGPRibbonButtonsGroup.h"
#include "BCGPRibbonColorButton.h"
#include "BCGPRibbonEdit.h"
#include "BCGPRibbonComboBox.h"
#include "BCGPRibbonCheckBox.h"
#include "BCGPRibbonCommandsListBox.h"
#include "BCGPRibbonPaletteButton.h"
#include "BCGPRibbonStatusBar.h"
#include "BCGPRibbonStatusBarPane.h"
#include "BCGPRibbonSlider.h"
#include "BCGPRibbonProgressBar.h"
#include "BCGPRibbonHyperlink.h"
#include "BCGPRibbonUndoButton.h"
#include "BCGPRibbonDialogBar.h"
#include "BCGPRibbonBackstageView.h"
#include "BCGPRibbonBackstageViewPanel.h"
#include "BCGPRibbonBackstagePagePrint.h"
#include "BCGPRibbonBackstagePageRecent.h"
#include "BCGPRibbonQuickStepsButton.h"

//-----------------------------------
// BCGControlBar control bar buttons:
//-----------------------------------
#include "BCGPToolbarButton.h"
#include "BCGPToolbarComboBoxButton.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPToolbarEditBoxButton.h"
#include "BCGPToolbarSpinEditBoxButton.h"
#include "BCGPToolbarSystemMenuButton.h"
#include "BCGPDropDown.h"
#include "BCGPColorMenuButton.h"
#include "BCGPToolbarFontCombo.h"
#include "BCGPToolbarDateTimeCtrl.h"
#include "BCGPCalendarMenuButton.h"
#include "BCGPOutlookButton.h"
#include "BCGPDropDownList.h"

//-------------------------
// Autohide bars
//-------------------------
#include "BCGPAutoHideButton.h"
#include "BCGPAutoHideToolBar.h"

//-------------------------------------------------------------------
// BCGControlBar frame and miniframe windows (replaces CFrameWnd, 
// CMDIFrameWnd, CMDIChildWnd, COleIPFrameWnd and COleDocIPFrameWnd):
//-------------------------------------------------------------------
#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPMDIChildWnd.h"
#include "BCGPOleIPFrameWnd.h"
#include "BCGPOleDocIPFrameWnd.h"
#include "BCGPOleCntrFrameWnd.h"

#include "BCGPMiniFrameWnd.h"
#include "BCGPMultiMiniFrameWnd.h"

#include "BCGPAppBarWnd.h"

//-----------------------------------
// BCGControlBar customization stuff:
//-----------------------------------
#include "BCGPToolbarCustomize.h"
#include "BCGPCustomizeMenuButton.h"

#include "BCGPContextMenuManager.h"
#include "BCGPKeyboardManager.h"
#include "BCGPMouseManager.h"

#include "BCGPUserTool.h"
#include "BCGPKeyHelper.h"

//--------------------------------
// BCGControlBar workspace manager
//--------------------------------
#include "BCGPWorkspace.h"
#include "BCGPRegistry.h"
#include "BCGPRebarState.h"

//-----------------------
// BCGControlBar helpers:
//-----------------------
#include "BCGPTearOffManager.h"
#include "BCGPDrawManager.h"
#include "BCGPDockBarRow.h"
#include "BCGPTooltipManager.h"
#include "BCGPControlRenderer.h"

//---------------------------------
// BCGControlBar menu replacements:
//---------------------------------
#include "BCGPPopupMenu.h"
#include "BCGPPopupMenuBar.h"
#include "BCGPToolBarImages.h"

//----------------------------
// BCGControlBar docking bars:
//----------------------------
#include "BCGPDockingControlBar.h"
#include "BCGPDockBar.h"
#include "BCGPTabWnd.h"
#include "BCGPDockingCBWrapper.h"

//------------------------
// BCGControlBar controls:
//------------------------
#include "BCGPButton.h"
#include "BCGPEdit.h"
#include "BCGPColorButton.h"
#include "BCGPMenuButton.h"
#include "BCGPURLLinkButton.h"
#include "BCGPEditListBox.h"
#include "BCGPAnimCtrl.h"
#include "BCGPPowerColorPicker.h"
#include "BCGPFontComboBox.h"
#include "BCGPReBar.h"
#include "BCGPShellTree.h"
#include "BCGPShellList.h"
#include "BCGPShellBreadcrumb.h"
#include "BCGPPropList.h"
#include "BCGPEditCtrl.h"
#include "BCGPIntelliSenseWnd.h"
#include "BCGPIntelliSenseLB.h"
#include "BCGPMaskEdit.h"
#include "BCGPDateTimeCtrl.h"
#include "BCGPDurationCtrl.h"
#include "BCGPHotSpotImageCtrl.h"
#include "BCGPSplitterWnd.h"
#include "BCGPSpinButtonCtrl.h"
#include "BCGPPopupWindow.h"
#include "BCGPPopupDlg.h"
#include "BCGPExCheckList.h"
#include "BCGPToolTipCtrl.h"
#include "BCGPPngImage.h"
#include "BCGPScrollBar.h"
#include "BCGPSliderCtrl.h"
#include "BCGPProgressCtrl.h"
#include "BCGPGroup.h"
#include "BCGPStatic.h"
#include "BCGPComboBox.h"
#include "BCGPListBox.h"
#include "BCGPListCtrl.h"
#include "BCGPTreeCtrl.h"
#include "BCGPBreadcrumb.h"
#include "BCGPRibbonElementHostCtrl.h"
#include "BCGPLineStyleComboBox.h"
#include "BCGPBrushButton.h"
#include "BCGPTagCloud.h"
#include "BCGPTreeMap.h"
#include "BCGPWinUITiles.h"

//-----------------------
// BCGControlBar dialogs:
//-----------------------
#include "BCGPDialog.h"
#include "BCGPFileDialog.h"
#include "BCGPWindowsManagerDlg.h"
#include "BCGPPrintPreviewView.h"
#include "BCGPImageEditDlg.h"
#include "BCGPColorDialog.h"
#include "BCGPKeyMapDlg.h"
#include "BCGPPropertySheet.h"
#include "BCGPPropertyPage.h"
#include "PropertySheetCtrl.h"
#include "BCGPProgressDlg.h"
#include "BCGPMessageBox.h"
#include "BCGPEditBrushDlg.h"
#include "BCGPTextFormatDlg.h"
#include "BCGPPageTransitionManager.h"

//---------------------
// BCGControlBar views:
//---------------------
#include "BCGPTabView.h"
#include "BCGPEditView.h"
#include "BCGPFormView.h"

//--------------------
// Visualization stuf:
//--------------------
#include "BCGPVisualManager.h"
#include "BCGPVisualManagerXP.h"
#include "BCGPWinXPVisualManager.h"
#include "BCGPVisualManager2003.h"
#include "BCGPVisualManagerVS2005.h"
#include "BCGPVisualManager2007.h"
#include "BCGPVisualManagerVS2008.h"
#include "BCGPVisualManagerVS2010.h"
#include "BCGPVisualManagerVS2012.h"
#include "BCGPVisualManagerCarbon.h"
#include "BCGPVisualManagerScenic.h"
#include "BCGPVisualManager2010.h"
#include "BCGPVisualManager2013.h"
#include "BCGPSkinManager.h"

//-------------------------
// OLE Full Server Support:
//-------------------------
#include "BCGPOleServerDoc.h"

//----------
// Planners:
//----------
#include "BCGPPlannerManagerView.h"
#include "BCGPPlannerPrintWeek.h"
#include "BCGPPlannerPrintMonth.h"
#include "BCGPPlannerPrintDay.h"
#include "BCGPPlannerPrint.h"
#include "BCGPPlannerPrintDual.h"

#include "BCGPRecurrence.h"
#include "BCGPRecurrenceRules.h"
#include "BCGPAppointmentProperty.h"

//--------------
// Grid control:
//--------------
#include "BCGPGridCtrl.h"
#include "BCGPGridSerialize.h"
#include "BCGPGridFilter.h"
#include "BCGPGridFilterMenu.h"
#include "BCGPReportCtrl.h"
#include "BCGPDBGridCtrl.h"

#include "BCGPGridView.h"
#include "BCGPReportView.h"

//---------------
// Gantt control:
//---------------
#include "BCGPGanttControl.h"
#include "BCGPGanttChart.h"
#include "BCGPGanttGrid.h"

//---------------
// Chart control:
//---------------
#include "BCGPChartAxis.h"
#include "BCGPChartFormat.h"
#include "BCGPChartImpl.h"
#include "BCGPChartSeries.h"
#include "BCGPChartFormula.h"
#include "BCGPChartVisualObject.h"
#include "BCGPChartLegend.h"
#include "BCGPChartCtrl.h"
#include "BCGPChartView.h"

//----------------
// Gauge controls:
//----------------
#include "BCGPVisualContainer.h"
#include "BCGPCircularGaugeImpl.h"
#include "BCGPLinearGaugeImpl.h"
#include "BCGPNumericIndicatorImpl.h"
#include "BCGPColorIndicatorImpl.h"
#include "BCGPTextGaugeImpl.h"
#include "BCGPImageGaugeImpl.h"
#include "BCGPRadialMenu.h"
#include "BCGPRotationObject.h"
#include "BCGPAnalogClock.h"
#include "BCGPKnob.h"
#include "BCGPSwitchImpl.h"

//-----------------
// Diagram control:
//-----------------
#include "BCGPDiagramVisualContainer.h"
#include "BCGPDiagramVisualObject.h"
#include "BCGPDiagramShape.h"
#include "BCGPDiagramCtrl.h"
#include "BCGPDiagramView.h"

//-------------------
// Graphics managers:
//-------------------
#include "BCGPGraphicsManager.h"
#include "BCGPGraphicsManagerGDI.h"
#include "BCGPGraphicsManagerD2D.h"

//-------------------------
// Global Utils:
//-------------------------
#include "BCGPGlobalUtils.h"
#include "BCGPAccessibility.h"

#endif // __BCGCBPROINC_H
