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

// BCGVisualManagerXP.h: interface for the CBCGPVisualManagerXP class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPVISUALMANAGERXP_H__062013FA_7440_4CEC_AA78_67893D195FFA__INCLUDED_)
#define AFX_BCGPVISUALMANAGERXP_H__062013FA_7440_4CEC_AA78_67893D195FFA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPVisualManager.h"
#include "MenuImages.h"

class CBCGPStatusBar;
class CBCGPBaseControlBar;


class BCGCBPRODLLEXPORT CBCGPVisualManagerXP : public CBCGPVisualManager  
{
	DECLARE_DYNCREATE(CBCGPVisualManagerXP)

protected:
	CBCGPVisualManagerXP(BOOL bIsTemporary = FALSE);

public:
	virtual ~CBCGPVisualManagerXP();

protected:
	virtual void OnUpdateSystemColors ();

	virtual void OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CBCGPBaseControlBar* pBar);
	virtual void OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);
	virtual void OnDrawBarBorder (CDC* pDC, CBCGPBaseControlBar* pBar, CRect& rect);
	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPPopupMenu* pMenu, CRect rect);
	virtual void OnDrawMenuShadow (CDC* pDC, const CRect& rectClient, const CRect& rectExclude,
									int nDepth,  int iMinBrightness,  int iMaxBrightness,  
									CBitmap* pBmpSaveBottom,  CBitmap* pBmpSaveRight,
									BOOL bRTL);
	virtual void OnDrawSeparator (CDC* pDC, CBCGPBaseControlBar* pBar, CRect rect, BOOL bIsHoriz);
	virtual COLORREF OnDrawMenuLabel (CDC* pDC, CRect rect);
	
	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawButtonBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnHighlightMenuItem (CDC*pDC, CBCGPToolbarMenuButton* pButton,
		CRect rect, COLORREF& clrText);
	virtual COLORREF GetHighlightedMenuItemTextColor (CBCGPToolbarMenuButton* pButton);
	virtual void OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed);
	virtual void OnHighlightQuickCustomizeMenuButton (CDC* pDC, CBCGPToolbarMenuButton* pButton, CRect rect);
	virtual BOOL IsHighlightWholeMenuItem ()	{	return TRUE;	}

	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd);
	virtual void OnDrawTab (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd);
	virtual void OnFillTab (CDC* pDC, CRect rectFill, CBrush* pbrFill, int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd);
	virtual COLORREF GetTabPointerColor(CBCGPBaseTabWnd* /*pWndTab*/)
	{
		return m_clrSeparator;
	}

	virtual void OnDrawTabButton (CDC* pDC, CRect rect, 
											   const CBCGPBaseTabWnd* pTabWnd, 
											   int nID,
											   BOOL bIsHighlighted,
											   BOOL bIsPressed,
											   BOOL bIsDisabled);
	virtual void OnEraseTabsButton (CDC* pDC, CRect rect, CBCGPButton* pButton,
									CBCGPBaseTabWnd* pWndTab);
	virtual void OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
										CBCGPButton* pButton, UINT uiState, CBCGPBaseTabWnd* pWndTab);

	virtual COLORREF OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
		BOOL bActive, CRect rectCaption, CRect rectButtons);

	virtual void OnDrawCaptionButton (CDC* pDC, CBCGPCaptionButton* pButton,
									BOOL bActive, BOOL bHorz, BOOL bMaximized, BOOL bDisabled, 
									int nImageID = -1);
	virtual void OnDrawCaptionButtonIcon (CDC* pDC, 
											CBCGPCaptionButton* pButton,
											CBCGPMenuImages::IMAGES_IDS id,
											BOOL bActive, BOOL bDisabled,
											CPoint ptImage);

	virtual void OnDrawDockingBarScrollButton(CDC* pDC, CBCGPDockingBarScrollButton* pButton);

	virtual void OnDrawComboDropButton (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted,
										CBCGPToolbarComboBoxButton* pButton);
	virtual void OnDrawComboBorder (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted,
										CBCGPToolbarComboBoxButton* pButton);
	virtual void OnDrawEditBorder (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsHighlighted,
										CBCGPToolbarEditBoxButton* pButton);

	virtual COLORREF GetToolbarButtonTextColor (CBCGPToolbarButton* pButton,
												CBCGPVisualManager::BCGBUTTON_STATE state);
	virtual BOOL IsToolBarButtonDefaultBackground (CBCGPToolbarButton* pButton,
												CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive);
	virtual void OnDrawMenuResizeBar (CDC* pDC, CRect rect, int nResizeFlags);
	virtual void OnDrawMenuScrollButton (CDC* pDC, CRect rect, BOOL bIsScrollDown, 
										BOOL bIsHighlited, BOOL bIsPressed, BOOL bIsDisabled);

	virtual void GetCalendarColors (const CBCGPCalendar* pCalendar,
				   CBCGPCalendarColors& colors);
	virtual void OnDrawSpinButtons (CDC* pDC, CRect rectSpin, int nState, BOOL bOrientation, CBCGPSpinButtonCtrl* pSpinCtrl);
	virtual void OnFillSpinButton (CDC* pDC, CBCGPSpinButtonCtrl* pSpinCtrl, CRect rect, BOOL bDisabled);

	virtual COLORREF OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected = FALSE);
	virtual void OnDrawMenuArrowOnCustomizeList (CDC* pDC, CRect rectCommand, BOOL bSelected);

	virtual void OnDrawMenuSystemButton (CDC* pDC, CRect rect, UINT uiSystemCommand, 
										UINT nStyle, BOOL bHighlight);
	virtual void OnDrawStatusBarPaneBorder (CDC* pDC, CBCGPStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle);

	virtual int GetMenuImageMargin () const
	{
		return 3;
	}

	virtual int GetPopupMenuGap () const
	{
		return 0;
	}

	virtual void OnDrawButtonSeparator (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state,
		BOOL bHorz);

	virtual COLORREF OnFillMiniFrameCaption (CDC* pDC, CRect rectCaption, 
											CBCGPMiniFrameWnd* pFrameWnd, 
											BOOL bActive);
	virtual void OnDrawMiniFrameBorder (CDC* pDC, CBCGPMiniFrameWnd* pFrameWnd,
										CRect rectBorder, CRect rectBorderSize);
	virtual void OnDrawFloatingToolbarBorder (	CDC* pDC, CBCGPBaseToolBar* pToolBar, 
												CRect rectBorder, CRect rectBorderSize);

	virtual COLORREF GetToolbarHighlightColor ()
	{
		return m_clrHighlight;
	}

	virtual COLORREF GetHighlightedColor(UINT nType);

	virtual COLORREF GetSeparatorColor()
	{
		return m_clrSeparator;
	}

	virtual COLORREF OnFillCaptionBarButton (CDC* pDC, CBCGPCaptionBar* pBar,
											CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
											BOOL bIsDisabled, BOOL bHasDropDownArrow,
											BOOL bIsSysButton);
	virtual void OnDrawCaptionBarButtonBorder (CDC* pDC, CBCGPCaptionBar* pBar,
											CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
											BOOL bIsDisabled, BOOL bHasDropDownArrow,
											BOOL bIsSysButton);
	virtual void OnDrawCaptionBarInfoArea (CDC* pDC, CBCGPCaptionBar* pBar, CRect rect);

#ifndef BCGP_EXCLUDE_PROP_LIST
	virtual COLORREF GetPropListGroupColor (CBCGPPropList* pPropList);
	virtual COLORREF GetPropListGroupTextColor (CBCGPPropList* pPropList);
#endif

	// Tasks pane:
#ifndef BCGP_EXCLUDE_TASK_PANE
	virtual void OnDrawTasksGroupCaption(CDC* pDC, CBCGPTasksGroup* pGroup, 
										BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE, 
										BOOL bCanCollapse = FALSE);

	virtual void OnFillTasksGroupInterior(CDC* pDC, CRect rect, BOOL bSpecial = FALSE);
	virtual void OnDrawTasksGroupAreaBorder(CDC* pDC, CRect rect, BOOL bSpecial = FALSE, 
											BOOL bNoTitle = FALSE);
	virtual void OnDrawTask(CDC* pDC, CBCGPTask* pTask, CImageList* pIcons, 
							BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE);

	virtual void OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited);
#endif // BCGP_EXCLUDE_TASK_PANE

	BOOL IsLook2000Allowed () const
	{
		return FALSE;
	}

	virtual void OnDrawSplitterBorder (CDC* pDC, CBCGPSplitterWnd* pSplitterWnd, CRect rect);
	virtual void OnDrawSplitterBox (CDC* pDC, CBCGPSplitterWnd* pSplitterWnd, CRect& rect);

	// Calculator:
	virtual BOOL OnDrawCalculatorButton (CDC* pDC, CRect rect, CBCGPToolbarButton* pButton, CBCGPVisualManager::BCGBUTTON_STATE state, int cmd /* CBCGPCalculator::CalculatorCommands */, CBCGPCalculator* pCalculator);
	virtual BOOL OnDrawCalculatorDisplay (CDC* pDC, CRect rect, const CString& strText, BOOL bMem, CBCGPCalculator* pCalculator);

	// Edit box:
	virtual BOOL OnDrawBrowseButton (CDC* pDC, CRect rect, CBCGPEdit* pEdit, CBCGPVisualManager::BCGBUTTON_STATE state, COLORREF& clrText);
	virtual void OnDrawEditClearButton(CDC* pDC, CBCGPButton* pButton, CRect rect);

	virtual COLORREF GetAutoHideButtonTextColor (CBCGPAutoHideButton* pButton);

	// Appbar window:
	virtual void OnDrawAppBarBorder (CDC* pDC, CBCGPAppBarWnd* pAppBarWnd,
									CRect rectBorder, CRect rectBorderSize);
	
	virtual void GetSmartDockingBaseMarkerColors (
		COLORREF& clrBaseGroupBackground,
		COLORREF& clrBaseGroupBorder);

	virtual COLORREF OnFillListBoxItem (CDC* pDC, CBCGPListBox* pListBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected);

	virtual COLORREF OnFillComboBoxItem(CDC* pDC, CBCGPComboBox* pComboBox, int nIndex, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected);

	// Popup window:
#ifndef BCGP_EXCLUDE_POPUP_WINDOW
	virtual void OnDrawPopupWindowBorder (CDC* pDC, CRect rect);
	virtual COLORREF OnDrawPopupWindowCaption (CDC* pDC, CRect rectCaption, CBCGPPopupWindow* pPopupWnd);
	virtual void OnDrawPopupWindowRoundedBorder (CDC* pDC, CRect rect, CBCGPPopupWindow* pPopupWnd, int nCornerRadius);
	virtual void OnErasePopupWindowButton (CDC* pDC, CRect rectClient, CBCGPPopupWndButton* pButton);
	virtual void OnDrawPopupWindowButtonBorder (CDC* pDC, CRect rectClient, CBCGPPopupWndButton* pButton);
	virtual void OnFillPopupWindowBackground (CDC* pDC, CRect rect);
#endif

	// Day planner:
#ifndef BCGP_EXCLUDE_PLANNER
	virtual COLORREF OnFillPlannerCaption (CDC* pDC, CBCGPPlannerView* pView,
		CRect rect, BOOL bIsToday, BOOL bIsSelected, BOOL bNoBorder = FALSE, BOOL bHorz = TRUE);
	virtual void OnDrawPlannerTimeLine (CDC* pDC, CBCGPPlannerView* pView, CRect rect);
#endif // BCGP_EXCLUDE_PLANNER

	// Grid control:
#ifndef BCGP_EXCLUDE_GRID_CTRL
	virtual void OnDrawGridHeaderMenuButton (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, 
		BOOL bHighlighted, BOOL bPressed, BOOL bDisabled);
	virtual BOOL OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme);

	virtual COLORREF OnDrawGridColumnChooserItem(CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectItem, BOOL bIsSelected);

	virtual CBrush& GetGridCaptionBrush(CBCGPGridCtrl* /*pCtrl*/)
	{
		return m_brBarBkgnd;
	}

	virtual void OnDrawGridSelectAllMarker(CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, int nPadding, BOOL bPressed);

	virtual BOOL IsGridSparklineDefaultSelColor() const
	{
		return TRUE;
	}

#endif

	virtual void OnDrawDateTimeDropButton (CDC* pDC, CRect rect, BOOL bDisabled, BOOL bPressed, BOOL bHighlighted, CBCGPDateTimeCtrl* pCtrl);

	// Breadcrumb control:
	virtual void BreadcrumbDrawItem (CDC& dc, CBCGPBreadcrumb* pControl, BREADCRUMBITEMINFO* pItemInfo, CRect rectItem, UINT uState, COLORREF color);
	virtual void BreadcrumbDrawArrow (CDC& dc, CBCGPBreadcrumb* pControl, BREADCRUMBITEMINFO* pItemInfo, CRect rect, UINT uState, COLORREF color);
	virtual void BreadcrumbDrawLeftArrow (CDC& dc, CBCGPBreadcrumb* pControl, CRect rect, UINT uState, COLORREF color);
	virtual void BreadcrumbDrawItemBackground (CDC& dc, CBCGPBreadcrumb* pControl, BREADCRUMBITEMINFO* pItemInfo, CRect rectItem, UINT uState, COLORREF color);
	virtual void BreadcrumbDrawArrowBackground (CDC& dc, CBCGPBreadcrumb* pControl, BREADCRUMBITEMINFO* pItemInfo, CRect rectArrow, UINT uState, COLORREF color);
	virtual void BreadcrumbDrawLeftArrowBackground (CDC& dc, CBCGPBreadcrumb* pControl, CRect rect, UINT uState, COLORREF color);

#ifndef BCGP_EXCLUDE_RIBBON
	// Ribbon control:
	virtual COLORREF OnFillRibbonButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);

	virtual COLORREF OnFillRibbonPinnedButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton, BOOL& bIsDarkPin);

	virtual void OnDrawRibbonButtonBorder (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);

	virtual void OnDrawRibbonPinnedButtonBorder (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);

	virtual void OnDrawRibbonCategoryScroll (
					CDC* pDC, 
					CBCGPRibbonCategoryScroll* pScroll);

	virtual void OnDrawRibbonMenuCheckFrame (
					CDC* pDC,
					CBCGPRibbonButton* pButton, 
					CRect rect);

	virtual void OnFillRibbonMenuFrame (
					CDC* pDC, 
					CBCGPRibbonMainPanel* pPanel, 
					CRect rect);

	virtual void OnDrawRibbonRecentFilesFrame (
					CDC* pDC, 
					CBCGPRibbonMainPanel* pPanel, 
					CRect rect);

	virtual COLORREF OnDrawRibbonStatusBarPane (CDC* pDC, CBCGPRibbonStatusBar* pBar,
					CBCGPRibbonStatusBarPane* pPane);

	virtual void GetRibbonSliderColors (CBCGPRibbonSlider* pSlider, 
					BOOL bIsHighlighted, 
					BOOL bIsPressed,
					BOOL bIsDisabled,
					COLORREF& clrLine,
					COLORREF& clrFill);
	virtual void OnDrawRibbonQATSeparator (CDC* pDC, CBCGPRibbonSeparator* pSeparator, CRect rect);

	virtual void OnDrawRibbonColorPaletteBox (CDC* pDC, CBCGPRibbonColorButton* pColorButton,
		CBCGPRibbonPaletteIcon* pIcon,
		COLORREF color, CRect rect, BOOL bDrawTopEdge, BOOL bDrawBottomEdge, 
		BOOL bIsHighlighted, BOOL bIsChecked, BOOL bIsDisabled);

	virtual COLORREF OnDrawRibbonPanel (
					CDC* pDC,
					CBCGPRibbonPanel* pPanel, 
					CRect rectPanel,
					CRect rectCaption);

#endif

	// Push button:
	virtual BOOL OnDrawPushButton (CDC* pDC, CRect rect, CBCGPButton* pButton, COLORREF& clrText);
	virtual BOOL IsDrawFocusRectOnPushButton(CBCGPButton* /*pButton*/) 	{ return FALSE; }

	CBrush		m_brGripperHorz;
	CBrush		m_brGripperVert;

	COLORREF	m_clrBarBkgnd;			// Control bar background color (expect menu bar)
	CBrush		m_brBarBkgnd;

	COLORREF	m_clrMenuRarelyUsed;
	CBrush		m_brMenuRarelyUsed;

	COLORREF	m_clrMenuLight;			// Color of the light menu area
	CBrush		m_brMenuLight;

	CBrush		m_brMenuConnect;

	COLORREF	m_clrInactiveTabText;
	CBrush		m_brTabBack;

	COLORREF	m_clrHighlight;			// Highlighted toolbar/menu item color
	CBrush		m_brHighlight;

	COLORREF	m_clrHighlightDn;		// Highlighted and pressed toolbar item color
	CBrush		m_brHighlightDn;

	COLORREF	m_clrHighlightChecked;
	CBrush		m_brHighlightChecked;
	COLORREF	m_clrPressedButtonBorder;	// Used in derived classes

	COLORREF	m_clrGripper;

	COLORREF	m_clrSeparator;
	CPen		m_penSeparator;

	COLORREF	m_clrPaneBorder;		// Status bar pane border

	COLORREF	m_clrMenuBorder;		// Menu border
	COLORREF	m_clrMenuItemBorder;	// Highlighted menu item border
	CPen		m_penMenuItemBorder;

	CBrush		m_brFloatToolBarBorder;	// Floating toolbar border

	BOOL		m_bConnectMenuToParent;

	BOOL		m_bShdowDroppedDownMenuButton;

	BOOL		m_bDrawLastTabLine;

	COLORREF	m_clrTaskPaneHotText;
	COLORREF	m_clrMenuSeparator;

	COLORREF	m_clrDockingBarScrollButton;
	COLORREF	m_clrDockingBarScrollButtonBorder;

	virtual void CreateGripperBrush ();
	virtual void ExtendMenuButton (CBCGPToolbarMenuButton* pMenuButton, CRect& rect);

	virtual COLORREF GetWindowColor () const;

	virtual CBrush& GetDlgButtonsAreaBrush(CWnd* pDlg, COLORREF* pclrLine = NULL);

	virtual void OnFillHighlightedArea (CDC* pDC, CRect rect, CBrush* pBrush,
		CBCGPToolbarButton* pButton);
};

#endif // !defined(AFX_BCGPVISUALMANAGERXP_H__062013FA_7440_4CEC_AA78_67893D195FFA__INCLUDED_)
