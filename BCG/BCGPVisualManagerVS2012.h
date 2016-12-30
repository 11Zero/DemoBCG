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
// BCGPVisualManagerVS2012.h: interface for the CBCGPVisualManagerVS2012 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPVISUALMANAGERVS2012_H__A49EA5C5_FB20_4DDD_BC38_9AF16C691505__INCLUDED_)
#define AFX_BCGPVISUALMANAGERVS2012_H__A49EA5C5_FB20_4DDD_BC38_9AF16C691505__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPVisualManagerVS2010.h"

class BCGCBPRODLLEXPORT CBCGPVisualManagerVS2012 : public CBCGPVisualManagerVS2010
{
	DECLARE_DYNCREATE(CBCGPVisualManagerVS2012)

public:
	enum AccentColor
	{
		VS2012_Blue,
		VS2012_Brown,
		VS2012_Green,
		VS2012_Lime,
		VS2012_Magenta,
		VS2012_Orange,
		VS2012_Pink,
		VS2012_Purple,
		VS2012_Red,
		VS2012_Teal,
	};

	enum Style
	{
		VS2012_Light,
		VS2012_Dark,
		VS2012_LightBlue,
	};

	static void SetStyle(Style style);
	static Style GetStyle() { return m_Style; }

	CBCGPVisualManagerVS2012();
	virtual ~CBCGPVisualManagerVS2012();

	static void SetAccentColor(AccentColor color);
	static void SetAccentColorRGB(COLORREF color);
    static COLORREF GetAccentColorRGB();
    static AccentColor GetAccentColor()
	{
		return m_curAccentColor;
	}

	static void SetFrameColor(COLORREF colorFill, COLORREF clrTextActive, COLORREF clrTextInactive, BOOL bRedraw = TRUE);
	static COLORREF GetFrameColor() { return m_clrFrame; }
	static COLORREF GetFrameTextColor(BOOL bActive) { return bActive ? m_clrFrameTextActive : m_clrFrameTextInactive; }

	static COLORREF AccentColorToRGB(AccentColor color);

	virtual void OnUpdateSystemColors ();
	virtual void ModifyGlobalColors ();

	virtual BOOL IsDarkTheme () const { return m_Style == VS2012_Dark; }

	virtual BOOL IsAutoGrayscaleImages();

	virtual BOOL IsEmbossDisabledBtnText ()
	{
		return FALSE;
	}

	virtual BOOL IsOwnerDrawCaption ();
	virtual BOOL OnSetWindowRegion (CWnd* pWnd, CSize sizeWindow);

	virtual void OnActivateApp(CWnd* pWnd, BOOL bActive);
	virtual BOOL OnNcActivate(CWnd* pWnd, BOOL bActive);
	virtual BOOL OnNcPaint (CWnd* pWnd, const CObList& lstSysButtons, CRect rectRedraw);
	virtual CSize GetNcBtnSize (BOOL bSmall) const;

	virtual CFont& GetNcCaptionTextFont();
	virtual COLORREF GetNcCaptionTextColor(BOOL bActive, BOOL bTitle = TRUE) const;

	virtual void DrawNcBtn (CDC* pDC, const CRect& rect, UINT nButton, 
		BCGBUTTON_STATE state, BOOL bSmall, 
		BOOL bActive, BOOL bMDI = FALSE, BOOL bEnabled = TRUE);

	virtual void DrawNcText (CDC* pDC, CRect& rect, const CString& strTitle, BOOL bActive, BOOL bIsRTL);

	virtual void DrawNcCaption (CDC* pDC, CRect rectCaption, 
		DWORD dwStyle, DWORD dwStyleEx,
		const CString& strTitle,
		HICON hIcon, BOOL bActive,
		const CObList& lstSysButtons, const CSize& szSysBorder);

	virtual void OnFillBackground(CDC* pDC, CRect rect, CWnd* pWnd);

	virtual void OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);

	virtual void OnDrawControlBorder (CDC* pDC, CRect rect, CWnd* pWndCtrl, BOOL bDrawOnGlass);

	virtual void OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed);
	virtual void OnHighlightMenuItem (CDC*pDC, CBCGPToolbarMenuButton* pButton,CRect rect, COLORREF& clrText);
	virtual void OnDrawShowAllMenuItems (CDC* pDC, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);
	virtual int GetShowAllMenuItemsHeight (CDC* pDC, const CSize& sizeDefault);
	virtual void OnDrawSeparator (CDC* pDC, CBCGPBaseControlBar* pBar, CRect rect, BOOL bIsHoriz);

	virtual int GetMenuDownArrowState (CBCGPToolbarMenuButton* pButton, BOOL bHightlight, BOOL bPressed, BOOL bDisabled);
	virtual COLORREF GetMenuItemTextColor (CBCGPToolbarMenuButton* pButton, BOOL bHighlighted, BOOL bDisabled);

	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPPopupMenu* pMenu, CRect rect);
	virtual void OnDrawMenuScrollButton (CDC* pDC, CRect rect, BOOL bIsScrollDown, 
		BOOL bIsHighlited, BOOL bIsPressed, BOOL bIsDisabled);

	virtual COLORREF OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
		BOOL bActive, CRect rectCaption, CRect rectButtons);

	virtual void OnDrawCaptionButton (CDC* pDC, CBCGPCaptionButton* pButton, BOOL bActive, BOOL bHorz,
										BOOL bMaximized, BOOL bDisabled, int nImageID = -1);

	virtual void OnDrawCaptionButtonIcon (CDC* pDC, 
											CBCGPCaptionButton* pButton,
											CBCGPMenuImages::IMAGES_IDS id,
											BOOL bActive, BOOL bDisabled,
											CPoint ptImage);

	virtual void OnDrawControlBarCaptionText (CDC* pDC, CBCGPDockingControlBar* pBar, BOOL bActive, const CString& strTitle, CRect& rectCaption);
	virtual BOOL IsMinFrameCaptionBold(CBCGPMiniFrameWnd* /* pFrame */) { return FALSE; }

	virtual void OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CBCGPBaseControlBar* pBar);

	virtual CRect GetMenuImageFrameOffset () const
	{
		return CRect (4, 2, 0, 3);
	}

	virtual void OnDrawMenuCheck (CDC* pDC, CBCGPToolbarMenuButton* pButton, CRect rect, BOOL bHighlight, BOOL bIsRadio);

	virtual void OnFillMenuImageRect (CDC* pDC, CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);
	virtual void OnDrawMenuImageRectBorder (CDC* pDC, CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawSlider (CDC* pDC, CBCGPSlider* pSlider, CRect rect, BOOL bAutoHideMode);

	virtual COLORREF GetToolbarDisabledTextColor ();

	virtual COLORREF OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected = FALSE);

	virtual void OnDrawCaptionBarBorder (CDC* pDC, CBCGPCaptionBar* pBar, CRect rect, COLORREF clrBarBorder, BOOL bFlatBorder);

	// Tabs support:
	virtual void OnDrawTab (CDC* pDC, CRect rectTab, int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd);
	virtual void OnDrawTabContent (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd,
							COLORREF clrText);

	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, 
								const CBCGPBaseTabWnd* pTabWnd);

	virtual void OnEraseTabsButton (CDC* pDC, CRect rect, CBCGPButton* pButton,
									CBCGPBaseTabWnd* pWndTab);

	virtual int GetTabButtonState (CBCGPTabWnd* pTab, CBCGTabButton* pButton);
	virtual BOOL OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd);
	virtual void OnDrawTabBorder(CDC* pDC, CBCGPTabWnd* pTabWnd, CRect rectBorder, COLORREF clrBorder, CPen& penLine);

	virtual BOOL IsMDITabsTopEdge ()			{	return FALSE;	}
	virtual BOOL AlwaysHighlight3DTabs () const	{	return TRUE;	}
	virtual BOOL IsDockingTabUpperCase()		{	return FALSE;	}
	virtual BOOL IsFocusedTabSeparateLook()		{	return TRUE;	}
	virtual BOOL IsTopLevelMenuItemUpperCase()	{	return TRUE;	}

	virtual void GetTabFrameColors (const CBCGPBaseTabWnd* pTabWnd,
				   COLORREF& clrDark,
				   COLORREF& clrBlack,
				   COLORREF& clrHighlight,
				   COLORREF& clrFace,
				   COLORREF& clrDarkShadow,
				   COLORREF& clrLight,
				   CBrush*& pbrFace,
				   CBrush*& pbrBlack);

	virtual void OnDrawTabResizeBar (CDC* pDC, CBCGPBaseTabWnd* pWndTab, 
									BOOL bIsVert, CRect rect,
									CBrush* pbrFace, CPen* pPen);

	virtual int GetTabExtraHeight(const CBCGPTabWnd* pTab);

	virtual void OnDrawTabButton (CDC* pDC, CRect rect, 
											   const CBCGPBaseTabWnd* pTabWnd, 
											   int nID,
											   BOOL bIsHighlighted,
											   BOOL bIsPressed,
											   BOOL bIsDisabled);
	virtual BOOL IsTabColorBar(CBCGPTabWnd* pTab, int iTab);

	// Auto-hide buttons:
	virtual void OnDrawAutoHideButtonBorder (CDC* pDC, CRect rectBounds, CRect rectBorderSize, CBCGPAutoHideButton* pButton);
	virtual COLORREF GetAutoHideButtonTextColor (CBCGPAutoHideButton* pButton);
	virtual int GetAutohideButtonExtraSpace() { return 8; }
	virtual void OnDrawAutohideBar(CDC* pDC, CRect rectBar, CBCGPAutoHideButton* pButton);
	virtual int CreateAutoHideButtonRegion (CRect rect, DWORD dwAlignment, LPPOINT& points);

	virtual BCGP_SMARTDOCK_THEME GetSmartDockingTheme ();

	virtual COLORREF GetStatusBarPaneTextColor (CBCGPStatusBar* pStatusBar, CBCGStatusBarPaneInfo* pPane);
	virtual void OnDrawStatusBarPaneBorder (CDC* pDC, CBCGPStatusBar* pBar, CRect rectPane, UINT uiID, UINT nStyle);

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
	
	virtual HBRUSH GetToolbarEditColors(CBCGPToolbarButton* pButton, COLORREF& clrText, COLORREF& clrBk);
	virtual COLORREF GetToolbarEditPromptColor ();

	virtual void OnFillCombo		(CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted,
										CBCGPToolbarComboBoxButton* pButton);

	virtual COLORREF GetComboboxTextColor(CBCGPToolbarComboBoxButton* pButton, 
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted);

	virtual void OnDrawSpinButtons (CDC* pDC, CRect rectSpin, int nState, BOOL bOrientation, CBCGPSpinButtonCtrl* pSpinCtrl);
	virtual void OnFillSpinButton (CDC* pDC, CBCGPSpinButtonCtrl* pSpinCtrl, CRect rect, BOOL bDisabled);

	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnFillHighlightedArea (CDC* pDC, CRect rect, CBrush* pBrush, CBCGPToolbarButton* pButton);

	virtual void OnDrawButtonBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual int GetDockingTabsBordersSize ()	{	return 0;	}

	virtual BOOL OnEraseMDIClientArea (CDC* pDC, CRect rectClient);

	virtual BOOL AreCustomToolbarCtrlColors()	{	return TRUE;	}

	virtual CSize GetButtonExtraBorder () const	{	return CSize(0, 0);	}

	virtual int GetDockingBarCaptionExtraHeight () const	{	return 3;	}

	virtual COLORREF OnFillMiniFrameCaption (CDC* pDC, CRect rectCaption, 
											CBCGPMiniFrameWnd* pFrameWnd,
											BOOL bActive);
	virtual void OnDrawMiniFrameBorder (CDC* pDC, CBCGPMiniFrameWnd* pFrameWnd,
										CRect rectBorder, CRect rectBorderSize);
	virtual COLORREF GetToolbarButtonTextColor (CBCGPToolbarButton* pButton, CBCGPVisualManager::BCGBUTTON_STATE state);

	// Dialog:
	virtual CBrush& GetDlgBackBrush (CWnd* pDlg)
	{
		return m_brDlgBackground.GetSafeHandle () == NULL ? CBCGPVisualManagerVS2008::GetDlgBackBrush (pDlg) : m_brDlgBackground;
	}

	// Push button:
	virtual BOOL OnDrawPushButton (CDC* pDC, CRect rect, CBCGPButton* pButton, COLORREF& clrText);
	virtual BOOL IsDrawFocusRectOnPushButton(CBCGPButton* /*pButton*/) 	{ return FALSE; }

#ifndef BCGP_EXCLUDE_PROP_LIST
	virtual COLORREF OnFillPropList(CDC* pDC, CBCGPPropList* pList, const CRect& rectClient, COLORREF& clrFill);
	virtual void OnFillPropListToolbarArea(CDC* pDC, CBCGPPropList* pList, const CRect& rectToolBar);
	virtual COLORREF OnFillPropertyListSelectedItem(CDC* pDC, CBCGPProp* pProp, CBCGPPropList* pWndList, const CRect& rectFill, BOOL bFocused);
	virtual COLORREF GetPropListGroupTextColor(CBCGPPropList* pPropList);
	virtual COLORREF OnFillPropListDescriptionArea(CDC* pDC, CBCGPPropList* pList, const CRect& rect);
	virtual COLORREF OnFillPropListCommandsArea(CDC* pDC, CBCGPPropList* pList, const CRect& rect);
	virtual COLORREF GetPropListCommandTextColor (CBCGPPropList* pPropList);
	virtual COLORREF GetPropListDisabledTextColor(CBCGPPropList* pPropList);
	virtual COLORREF GetPropListGroupColor (CBCGPPropList* pPropList);
#endif

	// Tasks pane:
#ifndef BCGP_EXCLUDE_TASK_PANE
	virtual void OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea);
	virtual void OnDrawTasksGroupCaption(CDC* pDC, CBCGPTasksGroup* pGroup, 
										BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE, 
										BOOL bCanCollapse = FALSE);
	virtual void OnFillTasksGroupInterior(CDC* pDC, CRect rect, BOOL bSpecial = FALSE);
	virtual void OnDrawTasksGroupAreaBorder(CDC* pDC, CRect rect, BOOL bSpecial = FALSE, BOOL bNoTitle = FALSE);
	virtual void OnDrawTask(CDC* pDC, CBCGPTask* pTask, CImageList* pIcons, BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE);
#endif

	// Toolbox:
#ifndef BCGP_EXCLUDE_TOOLBOX
	virtual BOOL OnEraseToolBoxButton (CDC* pDC, CRect rect, CBCGPToolBoxButton* pButton);
	virtual BOOL OnDrawToolBoxButtonBorder (CDC* pDC, CRect& rect, CBCGPToolBoxButton* pButton, UINT uiState);
	virtual COLORREF GetToolBoxButtonTextColor (CBCGPToolBoxButton* pButton);
#endif

	// Outlook bar:
	virtual void OnFillOutlookPageButton (	CDC* pDC, const CRect& rect,
										BOOL bIsHighlighted, BOOL bIsPressed,
										COLORREF& clrText);
	virtual void OnDrawOutlookPageButtonBorder (CDC* pDC, CRect& rectBtn, 
										BOOL bIsHighlighted, BOOL bIsPressed);

	virtual void OnDrawOutlookBarFrame(CDC* pDC, CRect rectFrame);

	virtual void OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, COLORREF& clrText);

	virtual COLORREF GetCaptionBarTextColor (CBCGPCaptionBar* pBar);

	virtual COLORREF GetHeaderCtrlTextColor (CBCGPHeaderCtrl* pCtrl, BOOL bIsPressed, BOOL bIsHighlighted);
	virtual void OnDrawHeaderCtrlBorder (CBCGPHeaderCtrl* pCtrl, CDC* pDC, CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted);

	virtual COLORREF GetURLLinkColor (CBCGPURLLinkButton* pButton, BOOL bHover);

	// Popup window:
#ifndef BCGP_EXCLUDE_POPUP_WINDOW
	virtual COLORREF GetPopupWindowLinkTextColor(CBCGPPopupWindow* pPopupWnd, BOOL bIsHover);
	virtual void OnFillPopupWindowBackground (CDC* pDC, CRect rect);
	virtual void OnDrawPopupWindowBorder (CDC* pDC, CRect rect);
	virtual void OnDrawPopupWindowRoundedBorder (CDC* pDC, CRect rect, CBCGPPopupWindow* pPopupWnd, int nCornerRadius);
	virtual COLORREF OnDrawPopupWindowCaption (CDC* pDC, CRect rectCaption, CBCGPPopupWindow* pPopupWnd);
	virtual void OnErasePopupWindowButton (CDC* pDC, CRect rectClient, CBCGPPopupWndButton* pButton);
	virtual void OnDrawPopupWindowButtonBorder (CDC* pDC, CRect rectClient, CBCGPPopupWndButton* pButton);
#endif

	virtual void OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive);

	// List box:
	virtual COLORREF OnFillListBox(CDC* pDC, CBCGPListBox* pListBox, CRect rectClient);
	virtual COLORREF OnFillListBoxItem (CDC* pDC, CBCGPListBox* pListBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected);

	// Combo box:
	virtual COLORREF OnFillComboBoxItem(CDC* pDC, CBCGPComboBox* pComboBox, int nIndex, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected);
	virtual BOOL OnDrawComboBoxText(CDC* pDC, CBCGPComboBox* pComboBox);

	// List control:
	virtual HBRUSH GetListControlFillBrush(CBCGPListCtrl* pListCtrl);
	virtual COLORREF GetListControlMarkedColor(CBCGPListCtrl* pListCtrl);
	virtual COLORREF GetListControlTextColor(CBCGPListCtrl* pListCtrl);

	// Date/time controls:
	virtual void GetCalendarColors (const CBCGPCalendar* pCalendar, CBCGPCalendarColors& colors);
	virtual void OnDrawDateTimeDropButton (CDC* pDC, CRect rect, BOOL bDisabled, BOOL bPressed, BOOL bHighlighted, CBCGPDateTimeCtrl* pCtrl);

	// Caption bar:
	virtual void OnDrawCaptionBarInfoArea (CDC* pDC, CBCGPCaptionBar* pBar, CRect rect);
	virtual COLORREF OnFillCaptionBarButton (CDC* pDC, CBCGPCaptionBar* pBar,
		CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
		BOOL bIsDisabled, BOOL bHasDropDownArrow,
		BOOL bIsSysButton);
	virtual void OnDrawCaptionBarButtonBorder (CDC* pDC, CBCGPCaptionBar* pBar,
		CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
		BOOL bIsDisabled, BOOL bHasDropDownArrow,
		BOOL bIsSysButton);

    // Gantt control:
#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)
	virtual void GetGanttColors (const CBCGPGanttChart* pChart, BCGP_GANTT_CHART_COLORS& colors, COLORREF clrBack = CLR_DEFAULT) const;
    virtual void DrawGanttHeaderCell (const CBCGPGanttChart* pChart, CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, BOOL bHilite);
#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

public:
	static BOOL	m_bAutoGrayscaleImages;

	// Grid control:
#ifndef BCGP_EXCLUDE_GRID_CTRL
	virtual void OnFillGridBackground (CDC* pDC, CRect rect);
	virtual void OnFillGridHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual BOOL OnDrawGridHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnFillGridRowHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual BOOL OnDrawGridRowHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnFillGridSelectAllAreaBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual BOOL OnDrawGridSelectAllAreaBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual COLORREF OnFillGridGroupByBoxBackground (CDC* pDC, CRect rect);
	virtual COLORREF OnFillGridGroupByBoxTitleBackground (CDC* pDC, CRect rect);
	virtual COLORREF GetGridGroupByBoxLineColor () const;
	virtual void OnDrawGridGroupByBoxItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual COLORREF GetGridLeftOffsetColor (CBCGPGridCtrl* pCtrl);
	virtual COLORREF GetGridItemSortedColor (CBCGPGridCtrl* pCtrl);
	virtual void OnFillGridGroupBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill);
	virtual void OnDrawGridGroupUnderline (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill);
	virtual COLORREF OnFillGridRowBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill, BOOL bSelected);
	virtual COLORREF OnFillGridItem (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill,
		BOOL bSelected, BOOL bActiveItem, BOOL bSortedColumn);
	virtual void OnDrawGridHeaderMenuButton (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, 
		BOOL bHighlighted, BOOL bPressed, BOOL bDisabled);
	virtual void OnDrawGridSelectionBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual void OnDrawGridDataBar (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual void GetGridColorScaleBaseColors(CBCGPGridCtrl* pCtrl, COLORREF& clrLow, COLORREF& clrMid, COLORREF& clrHigh);

	virtual BOOL OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme);

	virtual BOOL IsGridSparklineDefaultSelColor() const
	{
		return m_Style != VS2012_Light;
	}

	// Report control:
	virtual COLORREF GetReportCtrlGroupBackgoundColor ();
	virtual COLORREF OnFillReportCtrlRowBackground (CBCGPGridCtrl* pCtrl, CDC* pDC,
		CRect rectFill, BOOL bSelected, BOOL bGroup);
#endif // BCGP_EXCLUDE_GRID_CTRL

	// Editor colors:
	virtual COLORREF GetEditBackSidebarColor(CBCGPEditCtrl* pEdit);
	virtual HBRUSH GetEditBackgroundBrush(CBCGPEditCtrl* pEdit);
	virtual COLORREF GetEditTextColor(CBCGPEditCtrl* pEdit);
	virtual COLORREF GetEditLineNumbersBarBackColor(CBCGPEditCtrl* pEdit);
	virtual COLORREF GetEditLineNumbersBarTextColor(CBCGPEditCtrl* pEdit);

	virtual COLORREF GetEditOutlineColor(CBCGPEditCtrl* pEdit);

	// Tree control:
	virtual COLORREF GetTreeControlFillColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected = FALSE, BOOL bIsFocused = FALSE, BOOL bIsDisabled = FALSE);
	virtual COLORREF GetTreeControlTextColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected = FALSE, BOOL bIsFocused = FALSE, BOOL bIsDisabled = FALSE);

	virtual CBrush& GetEditCtrlBackgroundBrush(CBCGPEdit* pEdit);
	virtual COLORREF GetEditCtrlTextColor(CBCGPEdit* pEdit);

	// Breadcrumb control:
	virtual COLORREF BreadcrumbFillBackground (CDC& dc, CBCGPBreadcrumb* pControl, CRect rectFill);

	// Scrollbar
	virtual BOOL IsOwnerDrawScrollBar () const;

	virtual void OnScrollBarDrawThumb (CDC* pDC, CBCGPScrollBar* pScrollBar, CRect rect, 
		BOOL bHorz, BOOL bHighlighted, BOOL bPressed, BOOL bDisabled);
					
	virtual void OnScrollBarDrawButton (CDC* pDC, CBCGPScrollBar* pScrollBar, CRect rect, 
		BOOL bHorz, BOOL bHighlighted, BOOL bPressed, BOOL bFirst, BOOL bDisabled);

	virtual void OnScrollBarFillBackground (CDC* pDC, CBCGPScrollBar* pScrollBar, CRect rect, 
		BOOL bHorz, BOOL bHighlighted, BOOL bPressed, BOOL bFirst, BOOL bDisabled);

#ifndef BCGP_EXCLUDE_PLANNER
	virtual void OnFillPlanner (CDC* pDC, CBCGPPlannerView* pView, 
		CRect rect, BOOL bWorkingArea);

	virtual COLORREF OnFillPlannerCaption (CDC* pDC, CBCGPPlannerView* pView,
		CRect rect, BOOL bIsToday, BOOL bIsSelected, BOOL bNoBorder = FALSE,
		BOOL bHorz = TRUE);

	virtual void OnDrawPlannerCaptionText (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect, const CString& strText, 
		COLORREF clrText, int nAlign, BOOL bHighlight);

	virtual COLORREF GetPlannerAppointmentTimeColor (CBCGPPlannerView* pView,
		BOOL bSelected, BOOL bSimple, DWORD dwDrawFlags);

	virtual COLORREF GetPlannerHourLineColor (CBCGPPlannerView* pView,
		BOOL bWorkingHours, BOOL bHour);

	virtual COLORREF GetPlannerViewWorkingColor (CBCGPPlannerView* pView);
	virtual COLORREF GetPlannerViewNonWorkingColor (CBCGPPlannerView* pView);

	virtual COLORREF OnFillPlannerTimeBar (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect, COLORREF& clrLine);

	virtual void OnDrawPlannerTimeLine (CDC* pDC, CBCGPPlannerView* pView, CRect rect);

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

	virtual DWORD GetPlannerDrawFlags () const;
	
	virtual int GetPlannerRowExtraHeight () const;

	virtual COLORREF GetPlannerSelectionColor (CBCGPPlannerView* pView);
	virtual COLORREF GetPlannerSeparatorColor (CBCGPPlannerView* pView);

	virtual void PreparePlannerBackItem (BOOL bIsToday, BOOL bIsSelected);

#endif // BCGP_EXCLUDE_PLANNER

#ifndef BCGP_EXCLUDE_RIBBON
	virtual BOOL DoesRibbonExtendCaptionToTabsArea()	{	return TRUE;	}
	virtual BOOL IsRibbonScenicLook()					{	return TRUE;	}
	virtual BOOL IsRibbonBackgroundImage()				{	return TRUE;	}

	virtual COLORREF OnFillRibbonButton (
		CDC* pDC, 
		CBCGPRibbonButton* pButton);

	virtual void OnDrawRibbonProgressBar (
		CDC* pDC, CBCGPRibbonProgressBar* pProgress, 
		CRect rectProgress, CRect rectChunk, BOOL bInfiniteMode);

	virtual void OnDrawRibbonCategory (
		CDC* pDC, 
		CBCGPRibbonCategory* pCategory, 
		CRect rectCategory);
	
	virtual COLORREF OnDrawRibbonPanel (
		CDC* pDC,
		CBCGPRibbonPanel* pPanel, 
		CRect rectPanel,
		CRect rectCaption);

	virtual COLORREF OnFillRibbonPanelCaption (
		CDC* pDC,
		CBCGPRibbonPanel* pPanel, 
		CRect rectCaption);

	virtual COLORREF OnDrawRibbonCategoryTab (
		CDC* pDC, 
		CBCGPRibbonTab* pTab, 
		BOOL bIsActive);

	virtual void OnDrawRibbonMainButton (
		CDC* pDC, 
		CBCGPRibbonButton* pButton);

	virtual COLORREF OnDrawRibbonCategoryCaption (
		CDC* pDC, 
		CBCGPRibbonContextCaption* pContextCaption);

	virtual COLORREF OnDrawRibbonTabsFrame (
		CDC* pDC, 
		CBCGPRibbonBar* pWndRibbonBar, 
		CRect rectTab);

	virtual BOOL IsRibbonBackstageHideTabs()			{	return TRUE;	}
	virtual BOOL IsRibbonBackstageWhiteBackground()		{	return FALSE;	}
	virtual COLORREF GetRibbonBackstageTextColor();

	virtual COLORREF OnDrawRibbonStatusBarPane(CDC* pDC, CBCGPRibbonStatusBar* pBar, CBCGPRibbonStatusBarPane* pPane);

	virtual void OnDrawRibbonButtonBorder (
		CDC* pDC, 
		CBCGPRibbonButton* pButton);

	virtual void OnDrawRibbonDefaultPaneButton (
		CDC* pDC, 
		CBCGPRibbonButton* pButton);

	virtual void OnDrawDefaultRibbonImage (
		CDC* pDC, 
		CRect rectImage,
		BOOL bIsDisabled = FALSE,
		BOOL bIsPressed = FALSE,
		BOOL bIsHighlighted = FALSE);

	virtual void OnDrawRibbonSliderZoomButton (
					CDC* pDC, CBCGPRibbonSlider* pSlider, 
					CRect rect, BOOL bIsZoomOut, 
					BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled);

	virtual void OnDrawRibbonSliderChannel (
					CDC* pDC, CBCGPRibbonSlider* pSlider, 
					CRect rect);

	virtual void OnDrawRibbonSliderThumb (
					CDC* pDC, CBCGPRibbonSlider* pSlider, 
					CRect rect, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled);

	virtual COLORREF OnDrawRibbonButtonsGroup (
		CDC* pDC,
		CBCGPRibbonButtonsGroup* pGroup,
		CRect rectGroup);
	
	virtual BOOL IsDrawRibbonGroupSeparator()			{	return TRUE;	}
	virtual int GetRibbonButtonsGroupHorzMargin()		{	return 1;		}
	virtual void OnDrawRibbonGroupSeparator (CDC* pDC, CRect rectSeparator);

	virtual COLORREF GetRibbonEditBackgroundColor (
		CBCGPRibbonEditCtrl* pEdit,
		BOOL bIsHighlighted,
		BOOL bIsPaneHighlighted,
		BOOL bIsDisabled);

	virtual COLORREF GetRibbonEditTextColor (
		CBCGPRibbonEditCtrl* pEdit,
		BOOL bIsHighlighted,
		BOOL bIsDisabled);

	virtual COLORREF GetRibbonQATTextColor (BOOL bDisabled = FALSE);

	virtual void OnFillRibbonPanelMenuBarInCtrlMode(CDC* pDC, CBCGPRibbonPanelMenuBar* pBar, CRect rectClient);

	virtual void OnDrawRibbonMinimizeButtonImage(CDC* pDC, CBCGPRibbonMinimizeButton* pButton, BOOL bRibbonIsMinimized);

	virtual void OnDrawRibbonMenuArrow(CDC* pDC, CBCGPRibbonButton* pButton, UINT id, const CRect& rectMenuArrow);

	virtual void OnDrawRibbonLaunchButton (
		CDC* pDC,
		CBCGPRibbonLaunchButton* pButton,
		CBCGPRibbonPanel* pPanel);

	virtual void OnDrawRibbonBackStageTopMenu(CDC* pDC, CRect rectBackstageTopArea);

	virtual COLORREF GetRibbonHyperlinkTextColor (CBCGPRibbonHyperlink* pHyperLink);

#endif

	virtual BOOL DrawCheckBox (CDC *pDC, CRect rect, 
		BOOL bHighlighted, 
		int nState,
		BOOL bEnabled,
		BOOL bPressed);
	
	virtual BOOL DrawRadioButton (CDC *pDC, CRect rect, 
		BOOL bHighlighted, 
		BOOL bChecked,
		BOOL bEnabled,
		BOOL bPressed);
	
	// Calculator:
	virtual BOOL OnDrawCalculatorDisplay (CDC* pDC, CRect rect, const CString& strText, BOOL bMem, CBCGPCalculator* pCalculator);
	virtual COLORREF GetCalculatorDisplayTextColor();

	// Slider:
	virtual void OnDrawSliderChannel (CDC* pDC, CBCGPSliderCtrl* pSlider, BOOL bVert, CRect rect, BOOL bDrawOnGlass);

	virtual void OnDrawSliderThumb (CDC* pDC, CBCGPSliderCtrl* pSlider, 
			CRect rect, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled,
			BOOL bVert, BOOL bLeftTop, BOOL bRightBottom,
			BOOL bDrawOnGlass);

	// Explorer bar:
	virtual UINT GetNavButtonsID(BOOL bIsLarge);
	virtual UINT GetNavFrameID(BOOL bIsLarge);

	virtual BOOL IsDWMCaptionSupported() const { return FALSE; }
	virtual BOOL IsSmallSystemBorders() const { return TRUE; }
	virtual BOOL IsStatusBarCoversFrame() { return TRUE; }
	virtual void SetupColors();

protected:
	static COLORREF		m_clrAccent;
	static COLORREF		m_clrFrame;
	static COLORREF		m_clrFrameTextActive;
	static COLORREF		m_clrFrameTextInactive;
	static AccentColor	m_curAccentColor;
	static Style		m_Style;
	CBrush				m_brAccent;
	CBrush				m_brAccentHighlight;
	CBrush				m_brFrame;
	COLORREF			m_clrAccentLight;
	COLORREF			m_clrAccentHilight;
	CBrush				m_brAccentLight;
	COLORREF			m_clrAccentText;
	COLORREF			m_clrTextDisabled;
	COLORREF			m_clrGripperBk;
	COLORREF			m_clrRibbonTabs;
	COLORREF			m_clrTabsBackground;
	COLORREF			m_clrHighlighDownText;
	BOOL				m_bCheckedRibbonButtonFrame;
	COLORREF			m_clrRibbonCategoryFill;
	CBrush				m_brRibbonCategoryFill;
	COLORREF			m_clrControl;
	CBrush				m_brControl;
	COLORREF			m_clrDlgBackground;
	COLORREF			m_clrEditPrompt;
	COLORREF			m_clrButtonsArea;

	COLORREF			m_clrNcTextActive;
	COLORREF			m_clrNcTextInactive;
	COLORREF			m_clrNcBorder;

	COLORREF			m_clrFace;
	COLORREF			m_clrComboHighlighted;

	COLORREF			m_clrAutohideBarInactive;
	COLORREF			m_clrAutohideBarActive;

	COLORREF			m_clrHighlightNC;
	CBrush				m_brHighlightNC;
};

class BCGCBPRODLLEXPORT CBCGPVisualManagerVS2013 : public CBCGPVisualManagerVS2012
{
	DECLARE_DYNCREATE(CBCGPVisualManagerVS2013)

public:
	virtual void ModifyGlobalColors();
	virtual void SetupColors();

	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd);
	virtual void OnDrawTab (CDC* pDC, CRect rectTab, int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd);

	virtual void OnDrawComboBorder (CDC* pDC, CRect rect,
		BOOL bDisabled,
		BOOL bIsDropped,
		BOOL bIsHighlighted,
		CBCGPToolbarComboBoxButton* pButton);

	virtual void OnFillMenuImageRect (CDC* pDC, CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);
	virtual void OnDrawMenuImageRectBorder (CDC* pDC, CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);
};

// Backward compatibility definitions:

class BCGCBPRODLLEXPORT CBCGPVisualManagerVS11 : public CBCGPVisualManagerVS2012
{
	DECLARE_DYNCREATE(CBCGPVisualManagerVS11)
};

#define VS11_Blue		VS2012_Blue
#define VS11_Brown		VS2012_Brown
#define VS11_Green		VS2012_Green
#define VS11_Lime		VS2012_Lime
#define VS11_Magenta	VS2012_Magenta
#define VS11_Orange		VS2012_Orange
#define VS11_Pink		VS2012_Pink
#define VS11_Purple		VS2012_Purple
#define VS11_Red		VS2012_Red
#define VS11_Teal		VS2012_Teal
#define VS11_Light		VS2012_Light
#define VS11_Dark		VS2012_Dark

#endif // !defined(AFX_BCGPVISUALMANAGERVS11_H__A49EA5C5_FB20_4DDD_BC38_9AF16C691505__INCLUDED_)
