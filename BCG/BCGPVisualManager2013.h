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
// BCGPVisualManager2013.h: interface for the CBCGPVisualManager2013 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPVISUALMANAGER2013_H__E0C4C3FF_13B6_4956_9E0C_4692963F5E15__INCLUDED_)
#define AFX_BCGPVISUALMANAGER2013_H__E0C4C3FF_13B6_4956_9E0C_4692963F5E15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPVisualManagerVS2012.h"

class BCGCBPRODLLEXPORT CBCGPVisualManager2013 : public CBCGPVisualManagerVS2012
{
	DECLARE_DYNCREATE(CBCGPVisualManager2013)

public:
	enum Style
	{
		Office2013_White,
		Office2013_Gray,
		Office2013_DarkGray,
	};
	
	static void SetStyle(Style style);
	static Style GetStyle() { return m_Style; }
	
	CBCGPVisualManager2013();
	virtual ~CBCGPVisualManager2013();

	virtual void ModifyGlobalColors ();
	virtual void SetupColors();

	virtual BOOL IsDarkTheme () const { return FALSE; }
	virtual BOOL IsAutoGrayscaleImages() { return FALSE; }

	virtual COLORREF OnDrawMenuLabel (CDC* pDC, CRect rect);
	virtual void OnDrawMenuImageRectBorder (CDC* pDC, CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);
	virtual void OnFillMenuImageRect (CDC* pDC, CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnScrollBarDrawThumb (CDC* pDC, CBCGPScrollBar* pScrollBar, CRect rect, 
		BOOL bHorz, BOOL bHighlighted, BOOL bPressed, BOOL bDisabled);
	
	virtual void OnScrollBarDrawButton (CDC* pDC, CBCGPScrollBar* pScrollBar, CRect rect, 
		BOOL bHorz, BOOL bHighlighted, BOOL bPressed, BOOL bFirst, BOOL bDisabled);
	
	virtual void OnScrollBarFillBackground (CDC* pDC, CBCGPScrollBar* pScrollBar, CRect rect, 
		BOOL bHorz, BOOL bHighlighted, BOOL bPressed, BOOL bFirst, BOOL bDisabled);

	virtual COLORREF OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
		BOOL bActive, CRect rectCaption, CRect rectButtons);

	virtual void OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CBCGPBaseControlBar* pBar);
	virtual void OnDrawControlBarCaptionText (CDC* pDC, CBCGPDockingControlBar* pBar, BOOL bActive, const CString& strTitle, CRect& rectCaption);

	virtual void OnDrawCaptionButton (CDC* pDC, CBCGPCaptionButton* pButton, BOOL bActive, BOOL bHorz,
		BOOL bMaximized, BOOL bDisabled, int nImageID = -1);
	
	virtual void OnDrawCaptionButtonIcon (CDC* pDC, 
		CBCGPCaptionButton* pButton,
		CBCGPMenuImages::IMAGES_IDS id,
		BOOL bActive, BOOL bDisabled,
		CPoint ptImage);

	virtual void OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed);
	virtual COLORREF GetHighlightedMenuItemTextColor (CBCGPToolbarMenuButton* pButton);	
	virtual COLORREF GetToolbarButtonTextColor (CBCGPToolbarButton* pButton, CBCGPVisualManager::BCGBUTTON_STATE state);
	virtual int GetMenuDownArrowState (CBCGPToolbarMenuButton* pButton, BOOL bHightlight, BOOL bPressed, BOOL bDisabled);

	virtual COLORREF OnFillMiniFrameCaption (CDC* pDC, CRect rectCaption, 
		CBCGPMiniFrameWnd* pFrameWnd,
		BOOL bActive);
	virtual void OnDrawMiniFrameBorder (CDC* pDC, CBCGPMiniFrameWnd* pFrameWnd,
		CRect rectBorder, CRect rectBorderSize);

	virtual void OnDrawMenuResizeBar (CDC* pDC, CRect rect, int nResizeFlags);

	virtual BOOL UseLargeCaptionFontInDockingCaptions();
	virtual BOOL IsRibbonBackstageWhiteBackground()		{	return TRUE;	}

	virtual COLORREF OnFillListBox(CDC* pDC, CBCGPListBox* pListBox, CRect rectClient);
	virtual COLORREF OnFillListBoxItem (CDC* pDC, CBCGPListBox* pListBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected);
	
	virtual COLORREF OnFillComboBoxItem(CDC* pDC, CBCGPComboBox* pComboBox, int nIndex, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected);

	virtual void OnDrawComboBorder (CDC* pDC, CRect rect,
		BOOL bDisabled,
		BOOL bIsDropped,
		BOOL bIsHighlighted,
		CBCGPToolbarComboBoxButton* pButton);

	virtual void OnDrawComboDropButton (CDC* pDC, CRect rect,
		BOOL bDisabled,
		BOOL bIsDropped,
		BOOL bIsHighlighted,
		CBCGPToolbarComboBoxButton* pButton);

	virtual void OnDrawEditBorder (CDC* pDC, CRect rect,
		BOOL bDisabled,
		BOOL bIsHighlighted,
		CBCGPToolbarEditBoxButton* pButton);

	virtual void OnFillHighlightedArea (CDC* pDC, CRect rect, CBrush* pBrush, CBCGPToolbarButton* pButton);
	
	virtual void OnDrawButtonBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	// Tabs support:
	virtual void OnDrawTab (CDC* pDC, CRect rectTab, int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd);

	virtual void OnDrawTabContent (CDC* pDC, CRect rectTab,
		int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd,
		COLORREF clrText);

	virtual void OnDrawTabBorder(CDC* pDC, CBCGPTabWnd* pTabWnd, CRect rectBorder, COLORREF clrBorder, CPen& penLine);

	virtual BOOL OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd);

	virtual void OnDrawTabButton (CDC* pDC, CRect rect, 
		const CBCGPBaseTabWnd* pTabWnd, 
		int nID,
		BOOL bIsHighlighted,
		BOOL bIsPressed,
		BOOL bIsDisabled);

	virtual void GetTabFrameColors (const CBCGPBaseTabWnd* pTabWnd,
		COLORREF& clrDark,
		COLORREF& clrBlack,
		COLORREF& clrHighlight,
		COLORREF& clrFace,
		COLORREF& clrDarkShadow,
		COLORREF& clrLight,
		CBrush*& pbrFace,
		CBrush*& pbrBlack);

	virtual COLORREF GetActiveTabBackColor(const CBCGPBaseTabWnd* pTabWnd) const;

	// Grid control:
#ifndef BCGP_EXCLUDE_GRID_CTRL
	virtual void OnFillGridHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual BOOL OnDrawGridHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnFillGridRowHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual void OnFillGridSelectAllAreaBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual COLORREF OnFillGridGroupByBoxBackground (CDC* pDC, CRect rect);
	virtual COLORREF OnFillGridGroupByBoxTitleBackground (CDC* pDC, CRect rect);
	virtual COLORREF GetGridGroupByBoxLineColor () const;
	virtual void OnDrawGridGroupByBoxItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual COLORREF GetGridLeftOffsetColor (CBCGPGridCtrl* pCtrl);
	virtual void OnFillGridGroupBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill);
	virtual BOOL IsGridGroupUnderline () const;
	virtual void OnDrawGridGroupUnderline (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill);
	virtual COLORREF OnFillGridRowBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill, BOOL bSelected);
	virtual COLORREF OnFillGridItem (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill,
		BOOL bSelected, BOOL bActiveItem, BOOL bSortedColumn);
	virtual BOOL OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme);

	virtual BOOL IsGridSparklineDefaultSelColor() const
	{
		return TRUE;
	}

	// Report control:
	virtual COLORREF GetReportCtrlGroupBackgoundColor ();
	virtual COLORREF OnFillReportCtrlRowBackground (CBCGPGridCtrl* pCtrl, CDC* pDC,
		CRect rectFill, BOOL bSelected, BOOL bGroup);
#endif // BCGP_EXCLUDE_GRID_CTRL

#ifndef BCGP_EXCLUDE_TOOLBOX
	virtual BOOL OnEraseToolBoxButton (CDC* pDC, CRect rect, CBCGPToolBoxButton* pButton);
	virtual BOOL OnDrawToolBoxButtonBorder (CDC* pDC, CRect& rect, CBCGPToolBoxButton* pButton, UINT uiState);
	virtual COLORREF GetToolBoxButtonTextColor (CBCGPToolBoxButton* pButton);
#endif

	// Planner
#ifndef BCGP_EXCLUDE_PLANNER
	virtual void OnFillPlanner (CDC* pDC, CBCGPPlannerView* pView, 
		CRect rect, BOOL bWorkingArea);

	virtual COLORREF OnFillPlannerCaption (CDC* pDC, CBCGPPlannerView* pView,
		CRect rect, BOOL bIsToday, BOOL bIsSelected, BOOL bNoBorder = FALSE,
		BOOL bHorz = TRUE);

	virtual COLORREF GetPlannerViewWorkingColor (CBCGPPlannerView* pView);
	virtual COLORREF GetPlannerViewNonWorkingColor (CBCGPPlannerView* pView);

	virtual COLORREF OnFillPlannerTimeBar (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect, COLORREF& clrLine);

	virtual void OnFillPlannerWeekBar (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect);

	virtual void OnDrawPlannerHeader (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect);

	virtual void OnDrawPlannerHeaderPane (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect);

	virtual void OnFillPlannerHeaderAllDay (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect);

	virtual void OnDrawPlannerHeaderAllDayItem (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect, BOOL bIsToday, BOOL bIsSelected);
#endif

	// ToolTip
	virtual BOOL GetToolTipParams (CBCGPToolTipParams& params, UINT nType = (UINT)(-1));

	// Auto-hide buttons:
	virtual void OnDrawAutohideBar(CDC* pDC, CRect rectBar, CBCGPAutoHideButton* pButton);

	// Outlook bar:
	virtual void OnFillOutlookPageButton (	CDC* pDC, const CRect& rect,
		BOOL bIsHighlighted, BOOL bIsPressed,
		COLORREF& clrText);
	virtual void OnDrawOutlookPageButtonBorder (CDC* pDC, CRect& rectBtn, 
		BOOL bIsHighlighted, BOOL bIsPressed);
	
	virtual void OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, COLORREF& clrText);
	
	// Tree control:
	virtual COLORREF GetTreeControlFillColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected = FALSE, BOOL bIsFocused = FALSE, BOOL bIsDisabled = FALSE);
	virtual COLORREF GetTreeControlTextColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected = FALSE, BOOL bIsFocused = FALSE, BOOL bIsDisabled = FALSE);

#ifndef BCGP_EXCLUDE_RIBBON
	// Ribbon control:
	virtual void OnDrawRibbonPaletteButton (
		CDC* pDC,
		CBCGPRibbonPaletteIcon* pButton);
	
	virtual void OnDrawRibbonPaletteButtonIcon(
		CDC* pDC,
		CBCGPRibbonPaletteIcon* pButton,
		int nID,
		CRect rect);
#endif

	static Style m_Style;
};

#endif // !defined(AFX_BCGPVISUALMANAGER2013_H__E0C4C3FF_13B6_4956_9E0C_4692963F5E15__INCLUDED_)
