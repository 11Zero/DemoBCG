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
// BCGPVisualManager2007.h: interface for the CBCGPVisualManager2007 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPVISUALMANAGER2007_H__4950864A_0CAE_4A54_8717_59381C1B05B8__INCLUDED_)
#define AFX_BCGPVISUALMANAGER2007_H__4950864A_0CAE_4A54_8717_59381C1B05B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPVisualManager2003.h"
#include "BCGPToolBarImages.h"
#include "BCGPControlRenderer.h"
#include "BCGPToolTipCtrl.h"
#include "bcgglobals.h"

#ifndef BCGP_EXCLUDE_GRID_CTRL
	#include "BCGPGridCtrl.h"
#endif

#define BCGPRibbonCategoryColorCount 7

class CBCGPTagManager;

class CBCGPBitmapCache
{
public:
	class CBitmapCacheItem
	{
	public:

		CBitmapCacheItem();
		virtual ~CBitmapCacheItem();

		void AddImage (HBITMAP hBmp);
		void Cache (const CSize& size, CBCGPControlRenderer& renderer);

		void Draw (CDC* pDC, CRect rect, int iImageIndex = 0, BYTE alphaSrc = 255);
		void DrawY (CDC* pDC, CRect rect, CSize sides, int iImageIndex = 0, BYTE alphaSrc = 255);

	protected:
		CBCGPToolBarImages m_Images;
		BOOL m_bMirror;
	};

	typedef CArray<CBitmapCacheItem*, CBitmapCacheItem*> XCache;
	typedef CArray<CSize, CSize> XSizes;

public:

	CBCGPBitmapCache();
	virtual ~CBCGPBitmapCache();

	void Clear ();

	int Cache (const CSize& size, CBCGPControlRenderer& renderer);
	int CacheY (int height, CBCGPControlRenderer& renderer);
	BOOL IsCached (const CSize& size) const;

	int FindIndex (const CSize& size) const;

	CBitmapCacheItem* Get (const CSize& size);
	CBitmapCacheItem* Get (int nIndex);

protected:
	XCache	m_Cache;
	XSizes  m_Sizes;
};

class BCGCBPRODLLEXPORT CBCGPVisualManager2007 : public CBCGPVisualManager2003  
{
	DECLARE_DYNCREATE(CBCGPVisualManager2007)

protected:
	CBCGPVisualManager2007();
	virtual ~CBCGPVisualManager2007();

public:

	enum Style
	{
		VS2007_LunaBlue,
		VS2007_ObsidianBlack,
		VS2007_Aqua,
		VS2007_Silver
	};

	static BOOL SetStyle (Style style, LPCTSTR lpszPath = NULL);
	static Style GetStyle ();
	static void SetResourceHandle (HINSTANCE hinstRes);

	static void SetCustomColor (COLORREF clrTarget);	// -1 - clear

	static void CleanStyle ();

	virtual void OnUpdateSystemColors ();

	virtual BOOL IsOwnerDrawCaption ()
	{
		return CanDrawImage () && !globalData.DwmIsCompositionEnabled ();
	}
	
	virtual BOOL IsStatusBarCoversFrame() { return IsOwnerDrawCaption(); }

	virtual BOOL OnNcActivate (CWnd* pWnd, BOOL bActive);
	virtual void OnActivateApp(CWnd* pWnd, BOOL bActive);
	virtual BOOL OnNcPaint (CWnd* pWnd, const CObList& lstSysButtons, CRect rectRedraw);
	virtual BOOL OnSetWindowRegion (CWnd* pWnd, CSize sizeWindow);

	virtual CSize GetNcBtnSize (BOOL bSmall) const;

public:
	virtual BOOL IsOwnerDrawMenuCheck ();
	virtual BOOL IsHighlightWholeMenuItem ();

	virtual BCGP_SMARTDOCK_THEME GetSmartDockingTheme ();

	virtual COLORREF GetStatusBarPaneTextColor (CBCGPStatusBar* pStatusBar, 
									CBCGStatusBarPaneInfo* pPane);

	virtual COLORREF GetToolbarButtonTextColor (CBCGPToolbarButton* pButton,
												CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual COLORREF GetMenuItemTextColor (CBCGPToolbarMenuButton* pButton, BOOL bHighlighted, BOOL bDisabled);

	virtual COLORREF GetHighlightedMenuItemTextColor (CBCGPToolbarMenuButton* pButton);	

	virtual CBrush& GetDlgButtonsAreaBrush(CWnd* pDlg, COLORREF* pclrLine);

	virtual CFont& GetNcCaptionTextFont();
	virtual COLORREF GetNcCaptionTextColor(BOOL bActive, BOOL bTitle = TRUE) const;

public:
	virtual CSize GetCheckRadioDefaultSize ();

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

	virtual void OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);
	virtual void OnFillHighlightedArea (CDC* pDC, CRect rect, CBrush* pBrush,
									CBCGPToolbarButton* pButton);
	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPPopupMenu* pMenu, CRect rect);
	virtual void OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CBCGPBaseControlBar* pBar);
	virtual void OnDrawSeparator (CDC* pDC, CBCGPBaseControlBar* pBar, CRect rect, BOOL bIsHoriz);
	virtual COLORREF OnDrawMenuLabel (CDC* pDC, CRect rect);
	virtual COLORREF OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
		BOOL bActive, CRect rectCaption, CRect rectButtons);

	virtual COLORREF OnFillCaptionBarButton (CDC* pDC, CBCGPCaptionBar* pBar,
											CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
											BOOL bIsDisabled, BOOL bHasDropDownArrow,
											BOOL bIsSysButton);

	virtual int GetShowAllMenuItemsHeight (CDC* pDC, const CSize& sizeDefault);
	virtual void OnDrawShowAllMenuItems (CDC* pDC, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawStatusBarPaneBorder (CDC* pDC, CBCGPStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle);
	virtual void OnDrawStatusBarSizeBox (CDC* pDC, CBCGPStatusBar* pStatBar,
				CRect rectSizeBox);

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
	virtual void OnDrawEditClearButton(CDC* pDC, CBCGPButton* pButton, CRect rect);

	virtual BOOL OnDrawBrowseButton (CDC* pDC, CRect rect, CBCGPEdit* pEdit, CBCGPVisualManager::BCGBUTTON_STATE state, COLORREF& clrText);

	virtual void OnDrawControlBorder (CDC* pDC, CRect rect, CWnd* pWndCtrl, BOOL bDrawOnGlass);

	virtual void OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive);
	virtual void OnDrawMenuResizeBar (CDC* pDC, CRect rect, int nResizeFlags);
	virtual void OnDrawMenuScrollButton (CDC* pDC, CRect rect, BOOL bIsScrollDown, 
										BOOL bIsHighlited, BOOL bIsPressed, BOOL bIsDisabled);
	virtual void OnDrawMenuSystemButton (CDC* pDC, CRect rect, UINT uiSystemCommand, 
										UINT nStyle, BOOL bHighlight);

	virtual COLORREF OnFillListBoxItem (CDC* pDC, CBCGPListBox* pListBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected);

	virtual COLORREF OnFillComboBoxItem(CDC* pDC, CBCGPComboBox* pComboBox, int nIndex, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected);

	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);
	virtual void OnDrawButtonBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);
	virtual void OnDrawButtonSeparator (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state,
		BOOL bHorz);

	virtual void OnHighlightMenuItem (CDC *pDC, CBCGPToolbarMenuButton* pButton,
		CRect rect, COLORREF& clrText);
	virtual void OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed);
	virtual void OnDrawMenuCheck (CDC* pDC, CBCGPToolbarMenuButton* pButton, 
		CRect rect, BOOL bHighlight, BOOL bIsRadio);
	
	virtual void OnDrawMenuItemButton (CDC* pDC, CBCGPToolbarMenuButton* pButton,
				CRect rectButton, BOOL bHighlight, BOOL bDisabled);

	virtual CSize GetPinSize(BOOL bIsPinned);
	virtual void OnDrawPin(CDC* pDC, const CRect& rect, BOOL bIsPinned, BOOL bIsDark, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled);

	// MiniFrame:
	virtual COLORREF OnFillMiniFrameCaption (CDC* pDC, CRect rectCaption, 
											CBCGPMiniFrameWnd* pFrameWnd,
											BOOL bActive);
	virtual void OnDrawMiniFrameBorder (CDC* pDC, CBCGPMiniFrameWnd* pFrameWnd,
										CRect rectBorder, CRect rectBorderSize);
	virtual void OnDrawFloatingToolbarBorder (	CDC* pDC, CBCGPBaseToolBar* pToolBar, 
												CRect rectBorder, CRect rectBorderSize);

	// tabs
	virtual void GetTabFrameColors (const CBCGPBaseTabWnd* pTabWnd,
				   COLORREF& clrDark,
				   COLORREF& clrBlack,
				   COLORREF& clrHighlight,
				   COLORREF& clrFace,
				   COLORREF& clrDarkShadow,
				   COLORREF& clrLight,
				   CBrush*& pbrFace,
				   CBrush*& pbrBlack);	
	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd);
	virtual BOOL OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd);
	virtual void OnEraseTabsButton (CDC* pDC, CRect rect, CBCGPButton* pButton,
									CBCGPBaseTabWnd* pWndTab);
	virtual void OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
										CBCGPButton* pButton, UINT uiState, CBCGPBaseTabWnd* pWndTab);
	virtual void OnDrawTab (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd);
	virtual void OnFillTab (CDC* pDC, CRect rectFill, CBrush* pbrFill, int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd);

	virtual void OnDrawTabButton (CDC* pDC, CRect rect, 
											   const CBCGPBaseTabWnd* pTabWnd, 
											   int nID,
											   BOOL bIsHighlighted,
											   BOOL bIsPressed,
											   BOOL bIsDisabled);

	virtual BOOL AlwaysHighlight3DTabs () const		{	return CanDrawImage () ? TRUE : CBCGPVisualManager2003::AlwaysHighlight3DTabs ();	}
	virtual COLORREF GetTabTextColor (const CBCGPBaseTabWnd* pTabWnd, int iTab, BOOL bIsActive);
	virtual int GetTabHorzMargin (const CBCGPBaseTabWnd* pTabWnd);

	virtual void OnDrawCaptionBarInfoArea (CDC* pDC, CBCGPCaptionBar* pBar, CRect rect);
	virtual COLORREF GetCaptionBarTextColor (CBCGPCaptionBar* pBar);
	
	// Outlook bar:
	virtual void OnFillOutlookPageButton (CDC* pDC, const CRect& rect,
										BOOL bIsHighlighted, BOOL bIsPressed,
										COLORREF& clrText);
	virtual void OnDrawOutlookPageButtonBorder (CDC* pDC, CRect& rectBtn, BOOL bIsHighlighted, BOOL bIsPressed);
	virtual void OnDrawOutlookBarSplitter (CDC* pDC, CRect rectSplitter);
	virtual void OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, COLORREF& clrText);

	virtual void GetCalendarColors (const CBCGPCalendar* pCalendar,
				   CBCGPCalendarColors& colors);

	virtual COLORREF GetToolbarDisabledTextColor ()
	{
		return CanDrawImage () ? 
			m_clrToolBarBtnTextDisabled : 
			CBCGPVisualManager2003::GetToolbarDisabledTextColor ();
	}

#ifndef BCGP_EXCLUDE_TASK_PANE
	virtual void OnDrawTasksGroupCaption(CDC* pDC, CBCGPTasksGroup* pGroup, 
										BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE, 
										BOOL bCanCollapse = FALSE);
	virtual void OnDrawTask(CDC* pDC, CBCGPTask* pTask, CImageList* pIcons, 
							BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE);

	virtual void OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited);
#endif // BCGP_EXCLUDE_TASK_PANE

	virtual COLORREF GetHeaderCtrlTextColor (CBCGPHeaderCtrl* pCtrl, BOOL bIsPressed, BOOL bIsHighlighted);
	virtual void OnDrawHeaderCtrlBorder (CBCGPHeaderCtrl* pCtrl, CDC* pDC,
			CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted);

	// Grid control:
#ifndef BCGP_EXCLUDE_GRID_CTRL
	virtual void OnFillGridHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual BOOL OnDrawGridHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnFillGridRowHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual BOOL OnDrawGridRowHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnFillGridSelectAllAreaBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnDrawGridSelectAllMarker(CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, int nPadding, BOOL bPressed);
	virtual BOOL OnDrawGridSelectAllAreaBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual COLORREF OnFillGridGroupByBoxBackground (CDC* pDC, CRect rect);	
	virtual COLORREF OnFillGridGroupByBoxTitleBackground (CDC* pDC, CRect rect);
	virtual void OnDrawGridGroupByBoxItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual COLORREF GetGridLeftOffsetColor (CBCGPGridCtrl* pCtrl);
	virtual COLORREF OnFillGridRowBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill, BOOL bSelected);
	virtual BOOL OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme);
	virtual void OnDrawGridDataBar (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);

	virtual CBrush& GetGridCaptionBrush(CBCGPGridCtrl* /*pCtrl*/)
	{
		return m_brGroupBackground.GetSafeHandle () != NULL ? m_brGroupBackground : m_brBarBkgnd;
	}

#endif

    // Gantt control:
#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)
	virtual void GetGanttColors (const CBCGPGanttChart* pChart, BCGP_GANTT_CHART_COLORS& colors, COLORREF clrBack = CLR_DEFAULT) const;	
    virtual void DrawGanttHeaderCell (const CBCGPGanttChart* pChart, CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, BOOL bHilite);
	virtual COLORREF GetGanttHeaderTextColor (BOOL bHilite) const;
#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

	// Ribbon control:
#ifndef BCGP_EXCLUDE_RIBBON
	virtual COLORREF OnDrawRibbonTabsFrame (
					CDC* pDC, 
					CBCGPRibbonBar* pWndRibbonBar, 
					CRect rectTab);
	
	virtual void OnDrawRibbonMainButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);

	virtual void OnDrawRibbonCategory (
					CDC* pDC, 
					CBCGPRibbonCategory* pCategory, 
					CRect rectCategory);

	virtual void OnDrawRibbonCategoryScroll (
					CDC* pDC, 
					CBCGPRibbonCategoryScroll* pScroll);

	virtual COLORREF OnDrawRibbonCategoryTab (
					CDC* pDC, 
					CBCGPRibbonTab* pTab, 
					BOOL bIsActive);

	virtual COLORREF OnDrawRibbonPanel (
					CDC* pDC,
					CBCGPRibbonPanel* pPanel, 
					CRect rectPanel,
					CRect rectCaption);

	virtual void OnDrawRibbonPanelCaption (
					CDC* pDC,
					CBCGPRibbonPanel* pPanel, 
					CRect rectCaption);

	virtual void OnDrawRibbonLaunchButton (
					CDC* pDC,
					CBCGPRibbonLaunchButton* pButton,
					CBCGPRibbonPanel* pPanel);

	virtual COLORREF OnFillRibbonButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);

	virtual COLORREF OnFillRibbonPinnedButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton, BOOL& bIsDarkPin);

	virtual COLORREF GetRibbonEditBackgroundColor (
					CBCGPRibbonEditCtrl* pEdit,
					BOOL bIsHighlighted,
					BOOL bIsPaneHighlighted,
					BOOL bIsDisabled);

	virtual void OnDrawRibbonButtonBorder (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);
	
	virtual void OnDrawRibbonPinnedButtonBorder (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);

	virtual void OnDrawRibbonMenuCheckFrame (
					CDC* pDC,
					CBCGPRibbonButton* pButton, 
					CRect rect);

	virtual void OnDrawRibbonDefaultPaneButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);

	virtual void OnDrawRibbonDefaultPaneButtonIndicator (
					CDC* pDC, 
					CBCGPRibbonButton* pButton,
					CRect rect, 
					BOOL bIsSelected, 
					BOOL bHighlighted);

	virtual void OnDrawRibbonCaption (
					CDC* pDC, CBCGPRibbonBar* pBar, CRect rectCaption,
					CRect rectText);

	virtual int GetRibbonQATChevronOffset ()	{	return IsBeta1 () ? 0 : 13;	}

	virtual int GetRibbonQATRightMargin ();

	virtual COLORREF GetRibbonQATTextColor (BOOL bDisabled = FALSE)
	{
		if (!CanDrawImage ())
		{
			return CBCGPVisualManager2003::GetRibbonQATTextColor (bDisabled);
		}

		return bDisabled ? m_clrRibbonCategoryTextDisabled : m_clrRibbonCategoryText;
	}

	virtual COLORREF GetRibbonBarTextColor (BOOL bDisabled = FALSE)
	{
		if (!CanDrawImage ())
		{
			return CBCGPVisualManager2003::GetRibbonBarTextColor (bDisabled);
		}

		return bDisabled ? m_clrRibbonBarBtnTextDisabled : m_clrRibbonBarBtnText;
	}

	virtual void OnDrawRibbonCaptionButton (
					CDC* pDC, CBCGPRibbonCaptionButton* pButton);

	virtual COLORREF OnDrawRibbonButtonsGroup (
					CDC* pDC,
					CBCGPRibbonButtonsGroup* pGroup,
					CRect rectGroup);

	virtual void OnDrawDefaultRibbonImage (
					CDC* pDC, 
					CRect rectImage,
					BOOL bIsDisabled = FALSE,
					BOOL bIsPressed = FALSE,
					BOOL bIsHighlighted = FALSE);

	virtual void OnDrawRibbonMainPanelFrame (
					CDC* pDC, 
					CBCGPRibbonMainPanel* pPanel, 
					CRect rect);

	virtual void OnFillRibbonMenuFrame (
					CDC* pDC, 
					CBCGPRibbonMainPanel* pPanel, 
					CRect rect);

	virtual void OnDrawRibbonRecentFilesFrame (
					CDC* pDC, 
					CBCGPRibbonMainPanel* pPanel, 
					CRect rect);

	virtual COLORREF OnFillRibbonMainPanelButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);

	virtual void OnDrawRibbonMainPanelButtonBorder (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);
	
	virtual void OnDrawRibbonPaletteButton (
					CDC* pDC,
					CBCGPRibbonPaletteIcon* pButton);

	virtual void OnDrawRibbonPaletteBorder (
					CDC* pDC, 
					CBCGPRibbonPaletteButton* pButton, 
					CRect rectBorder);
	
	virtual COLORREF OnDrawRibbonCategoryCaption (
					CDC* pDC, 
					CBCGPRibbonContextCaption* pContextCaption);

	virtual COLORREF OnDrawRibbonStatusBarPane (
					CDC* pDC, 
					CBCGPRibbonStatusBar* pBar,
					CBCGPRibbonStatusBarPane* pPane);
	
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


	virtual void OnDrawRibbonProgressBar (
					CDC* pDC, CBCGPRibbonProgressBar* pProgress, 
					CRect rectProgress, CRect rectChunk, BOOL bInfiniteMode);
	
	virtual void OnFillRibbonQATPopup (
				CDC* pDC, CBCGPRibbonPanelMenuBar* pMenuBar, CRect rect);

	virtual int GetRibbonPopupBorderSize (const CBCGPRibbonPanelMenu* pPopup) const;

	virtual BOOL IsLayeredRibbonKeyTip () const
	{
		return CanDrawImage ();
	}

	virtual void OnDrawRibbonKeyTip (CDC* pDC, CBCGPBaseRibbonElement* pElement, CRect rect, CString str);

	virtual void OnDrawRibbonCheckBoxOnList (CDC* pDC, CBCGPRibbonButton* pButton, CRect rect, BOOL bIsSelected, BOOL bHighlighted);
	virtual void OnDrawRibbonRadioButtonOnList (CDC* pDC, CBCGPRibbonButton* pButton, CRect rect, BOOL bIsSelected, BOOL bHighlighted);

	virtual COLORREF GetRibbonHyperlinkTextColor (CBCGPRibbonHyperlink* pHyperLink);
	virtual COLORREF GetRibbonStatusBarTextColor (CBCGPRibbonStatusBar* pStatusBar);

	virtual BOOL IsRibbonTabsAreaDark()	{	return m_Style == VS2007_ObsidianBlack;	}

#endif

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

	virtual COLORREF GetPlannerViewBackgroundColor (CBCGPPlannerView* pView);	
	virtual COLORREF GetPlannerHourLineColor (CBCGPPlannerView* pView,
		BOOL bWorkingHours, BOOL bHour);

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

	virtual DWORD GetPlannerDrawFlags () const;
	
	virtual int GetPlannerRowExtraHeight () const
	{
		return 3;
	}

	virtual COLORREF GetPlannerSelectionColor (CBCGPPlannerView* pView);
	virtual COLORREF GetPlannerSeparatorColor (CBCGPPlannerView* pView);

	virtual void PreparePlannerBackItem (BOOL bIsToday, BOOL bIsSelected);

#endif // BCGP_EXCLUDE_PLANNER

#ifndef BCGP_EXCLUDE_POPUP_WINDOW
	virtual void OnFillPopupWindowBackground (CDC* pDC, CRect rect);
	virtual COLORREF OnDrawPopupWindowCaption (CDC* pDC, CRect rectCaption, CBCGPPopupWindow* pPopupWnd);
	virtual COLORREF GetPopupWindowCaptionTextColor(CBCGPPopupWindow* pPopupWnd, BOOL bButton);
#endif

	virtual BOOL OnEraseMDIClientArea (CDC* pDC, CRect rectClient);

	// ToolTip
	virtual BOOL GetToolTipParams (CBCGPToolTipParams& params, UINT nType = (UINT)(-1));

	// Scrollbar
	virtual BOOL IsOwnerDrawScrollBar () const
	{
		return CanDrawImage ();
	}

	virtual void OnScrollBarDrawThumb (CDC* pDC, CBCGPScrollBar* pScrollBar, CRect rect, 
		BOOL bHorz, BOOL bHighlighted, BOOL bPressed, BOOL bDisabled);
					
	virtual void OnScrollBarDrawButton (CDC* pDC, CBCGPScrollBar* pScrollBar, CRect rect, 
		BOOL bHorz, BOOL bHighlighted, BOOL bPressed, BOOL bFirst, BOOL bDisabled);

	virtual void OnScrollBarFillBackground (CDC* pDC, CBCGPScrollBar* pScrollBar, CRect rect, 
		BOOL bHorz, BOOL bHighlighted, BOOL bPressed, BOOL bFirst, BOOL bDisabled);

	// Push button:
	virtual BOOL OnDrawPushButton (CDC* pDC, CRect rect, CBCGPButton* pButton, COLORREF& clrText);
	virtual BOOL IsDrawFocusRectOnPushButton(CBCGPButton* /*pButton*/) 	{ return FALSE; }

	// Group:
	virtual void OnDrawGroup (CDC* pDC, CBCGPGroup* pGroup, CRect rect, const CString& strName);

	// Separator:
	virtual BOOL IsOwnerDrawDlgSeparator(CBCGPStatic* /*pCtrl*/)	{	return CanDrawImage ();	}
	virtual void OnDrawDlgSeparator(CDC* pDC, CBCGPStatic* pCtrl, CRect rect, BOOL bIsHorz);

	// Dialog:
	virtual CBrush& GetDlgBackBrush (CWnd* pDlg)
	{
		return m_brDlgBackground.GetSafeHandle () == NULL ?
			CBCGPVisualManager2003::GetDlgBackBrush (pDlg) : m_brDlgBackground;
	}
	virtual BOOL OnFillDialog (CDC* pDC, CWnd* pDlg, CRect rect);
	virtual void OnDrawDlgSizeBox (CDC* pDC, CWnd* pDlg, CRect rectSizeBox);

	// Slider:
	virtual void OnDrawSliderChannel (CDC* pDC, CBCGPSliderCtrl* pSlider, BOOL bVert, CRect rect, BOOL bDrawOnGlass);

	virtual void OnDrawSliderThumb (CDC* pDC, CBCGPSliderCtrl* pSlider, 
			CRect rect, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled,
			BOOL bVert, BOOL bLeftTop, BOOL bRightBottom,
			BOOL bDrawOnGlass);

	// Calculator:
	virtual BOOL OnDrawCalculatorButton (CDC* pDC, CRect rect, CBCGPToolbarButton* pButton, CBCGPVisualManager::BCGBUTTON_STATE state, int cmd /* CBCGPCalculator::CalculatorCommands */, CBCGPCalculator* pCalculator);
	virtual BOOL OnDrawCalculatorDisplay (CDC* pDC, CRect rect, const CString& strText, BOOL bMem, CBCGPCalculator* pCalculator);
	virtual COLORREF GetCalculatorButtonTextColor (BOOL bIsUserCommand, int cmd /* CBCGPCalculator::CalculatorCommands */);

	virtual void OnFillPropSheetHeaderArea(CDC* pDC, CBCGPPropertySheet* pPropSheet, CRect rect, BOOL& bDrawBottomLine);

	virtual void OnDrawSpinButtons (CDC* pDC, CRect rectSpin, int nState, BOOL bOrientation, CBCGPSpinButtonCtrl* pSpinCtrl);
	virtual void OnFillSpinButton (CDC* pDC, CBCGPSpinButtonCtrl* pSpinCtrl, CRect rect, BOOL bDisabled);
	virtual void OnDrawDateTimeDropButton (CDC* pDC, CRect rect, BOOL bDisabled, BOOL bPressed, BOOL bHighlighted, CBCGPDateTimeCtrl* pCtrl);

	// Tree control:
	virtual COLORREF GetTreeControlFillColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected = FALSE, BOOL bIsFocused = FALSE, BOOL bIsDisabled = FALSE);
	virtual COLORREF GetTreeControlTextColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected = FALSE, BOOL bIsFocused = FALSE, BOOL bIsDisabled = FALSE);

	// List control:
	virtual HBRUSH GetListControlFillBrush(CBCGPListCtrl* pListCtrl);
	virtual COLORREF GetListControlTextColor(CBCGPListCtrl* pListCtrl);

protected:
	BOOL CanDrawImage () const
	{
#ifdef BCGP_EXCLUDE_PNG_SUPPORT
		return FALSE;
#else
		return globalData.m_nBitsPerPixel > 8 && 
			!globalData.IsHighContastMode () &&
			m_bLoaded;
#endif
	}

	static CString MakeResourceID (LPCTSTR lpszID);
	static CString GetStyleResourceID (Style style);

	virtual void CleanUp ();

	virtual BOOL IsSmallMenuSystemButton () const	{	return FALSE;	}
    virtual void DrawSeparator (CDC* pDC, const CRect& rect, BOOL bHorz);
    virtual void DrawSeparator (CDC* pDC, const CRect& rect, CPen& pen1, CPen& pen2, BOOL bHorz);
	virtual void DrawNcBtn (CDC* pDC, const CRect& rect, UINT nButton, 
							BCGBUTTON_STATE state, BOOL bSmall, 
							BOOL bActive, BOOL bMDI = FALSE, BOOL bEnabled = TRUE);
	virtual void DrawNcText (CDC* pDC, CRect& rect, const CString& strTitle, 
							const CString& strDocument, BOOL bPrefix, 
							BOOL bActive, BOOL bIsRTL, BOOL bTextCenter,
							BOOL bGlass = FALSE, int nGlassGlowSize = 0, 
							COLORREF clrGlassText = (COLORREF)-1);
	virtual void DrawNcCaption (CDC* pDC, CRect rectCaption, 
						   DWORD dwStyle, DWORD dwStyleEx,
						   const CString& strTitle, const CString& strDocument,
						   HICON hIcon, BOOL bPrefix, BOOL bActive, BOOL bTextCenter,
						   const CObList& lstSysButtons);

	BOOL					m_bLoaded;
	
	CSize					m_szNcBtnSize[2];
	BOOL					m_bNcTextCenter;

	CBCGPToolBarImages		m_ToolBarGripper;
	CBCGPToolBarImages		m_ToolBarTear;
	CBCGPControlRenderer	m_ctrlToolBarBorder;

	CBCGPControlRenderer	m_ctrlStatusBarBack;
	CBCGPControlRenderer	m_ctrlStatusBarBack_Ext;
	CBCGPToolBarImages		m_StatusBarPaneBorder;
	CBCGPToolBarImages		m_StatusBarSizeBox;

	CBCGPControlRenderer	m_SysBtnBack[2];
	CBCGPControlRenderer	m_SysBtnBackC[2];
	CBCGPToolBarImages		m_SysBtnClose[2];
	CBCGPToolBarImages		m_SysBtnRestore[2];
	CBCGPToolBarImages		m_SysBtnMaximize[2];
	CBCGPToolBarImages		m_SysBtnMinimize[2];
	CBCGPToolBarImages		m_SysBtnHelp[2];

	CFont m_AppCaptionFont;

	COLORREF m_clrAppCaptionActiveStart;
	COLORREF m_clrAppCaptionActiveFinish;
	COLORREF m_clrAppCaptionInactiveStart;
	COLORREF m_clrAppCaptionInactiveFinish;
	COLORREF m_clrAppCaptionActiveText;
	COLORREF m_clrAppCaptionInactiveText;
	COLORREF m_clrAppCaptionActiveTitleText;
	COLORREF m_clrAppCaptionInactiveTitleText;

	COLORREF m_clrMainClientArea;
	CBrush	 m_brMainClientArea;

	COLORREF m_clrMenuBarGradientLight;
	COLORREF m_clrMenuBarGradientDark;
	COLORREF m_clrMenuBarGradientVertLight;
	COLORREF m_clrMenuBarGradientVertDark;

	COLORREF m_clrMenuBarBtnText;
	COLORREF m_clrMenuBarBtnTextHighlighted;
	COLORREF m_clrMenuBarBtnTextDisabled;
	COLORREF m_clrToolBarBtnText;
	COLORREF m_clrToolBarBtnTextHighlighted;
	COLORREF m_clrToolBarBtnTextDisabled;
	COLORREF m_clrMenuText;
	COLORREF m_clrMenuTextHighlighted;
	COLORREF m_clrMenuTextDisabled;

	COLORREF m_clrStatusBarText;
	COLORREF m_clrStatusBarTextDisabled;
	COLORREF m_clrExtenedStatusBarTextDisabled;

	COLORREF m_clrEditBorder;
	COLORREF m_clrEditBorderDisabled;
	COLORREF m_clrEditBorderHighlighted;
	COLORREF m_clrEditSelection;
	COLORREF m_clrComboBorder;
	COLORREF m_clrComboBorderDisabled;
	COLORREF m_clrComboBorderPressed;
	COLORREF m_clrComboBorderHighlighted;
	COLORREF m_clrComboBtnStart;
	COLORREF m_clrComboBtnFinish;
	COLORREF m_clrComboBtnBorder;
	COLORREF m_clrComboBtnDisabledStart;
	COLORREF m_clrComboBtnDisabledFinish;
	COLORREF m_clrComboBtnBorderDisabled;
	COLORREF m_clrComboBtnPressedStart;
	COLORREF m_clrComboBtnPressedFinish;
	COLORREF m_clrComboBtnBorderPressed;
	COLORREF m_clrComboBtnHighlightedStart;
	COLORREF m_clrComboBtnHighlightedFinish;
	COLORREF m_clrComboBtnBorderHighlighted;
	COLORREF m_clrComboSelection;
	CBCGPControlRenderer m_ctrlComboBoxBtn;

	COLORREF m_clrHeaderNormalStart;
	COLORREF m_clrHeaderNormalFinish;
	COLORREF m_clrHeaderNormalBorder;
	COLORREF m_clrHeaderNormalText;
	COLORREF m_clrHeaderHighlightedStart;
	COLORREF m_clrHeaderHighlightedFinish;
	COLORREF m_clrHeaderHighlightedBorder;
	COLORREF m_clrHeaderHighlightedText;
	COLORREF m_clrHeaderPressedStart;
	COLORREF m_clrHeaderPressedFinish;
	COLORREF m_clrHeaderPressedBorder;
	COLORREF m_clrHeaderPressedText;

	COLORREF m_clrBarCaption;
	COLORREF m_clrMiniFrameCaption;

	COLORREF m_clrSeparator1;
	COLORREF m_clrSeparator2;
	CPen     m_penSeparator2;
	CPen	 m_penSeparatorDark;

	COLORREF m_clrGroupText;
	CBrush	 m_brGroupBackground;

	COLORREF m_clrCaptionBarText;
	COLORREF m_clrCaptionBarTextMessage;

	CBCGPControlRenderer m_ctrlMainBorder;
	CBCGPControlRenderer m_ctrlMDIChildBorder;
	CBCGPControlRenderer m_ctrlDialogBorder;
	CBCGPControlRenderer m_ctrlMainBorderCaption;
	CBCGPControlRenderer m_ctrlAppBorderCaption;
	CBCGPControlRenderer m_ctrlPopupBorder;
	CBCGPControlRenderer m_ctrlPopupResizeBar;
	CBCGPToolBarImages   m_PopupResizeBar_HV;
	CBCGPToolBarImages   m_PopupResizeBar_HVT;
	CBCGPToolBarImages   m_PopupResizeBar_V;
	
	CBCGPControlRenderer m_ctrlMenuBarBtn;

	CBCGPControlRenderer m_ctrlMenuItemBack;
	CBCGPToolBarImages   m_MenuItemMarkerC;
	CBCGPToolBarImages   m_MenuItemMarkerR;
	CBCGPToolBarImages   m_MenuItemPin;
	CBCGPControlRenderer m_ctrlMenuItemShowAll;
	CBCGPControlRenderer m_ctrlMenuHighlighted[2];
	CBCGPControlRenderer m_ctrlMenuButtonBorder;
	CBCGPControlRenderer m_ctrlMenuScrollBtn[2];

	CBCGPControlRenderer m_ctrlToolBarBtn;

#ifndef BCGP_EXCLUDE_TASK_PANE
	COLORREF	m_clrTaskPaneGroupCaptionHighDark;
	COLORREF	m_clrTaskPaneGroupCaptionHighLight;
	COLORREF	m_clrTaskPaneGroupCaptionHighSpecDark;
	COLORREF	m_clrTaskPaneGroupCaptionHighSpecLight;

	COLORREF	m_clrTaskPaneGroupCaptionTextSpec;
	COLORREF	m_clrTaskPaneGroupCaptionTextHighSpec;
	COLORREF	m_clrTaskPaneGroupCaptionText;
	COLORREF	m_clrTaskPaneGroupCaptionTextHigh;

	CBCGPControlRenderer m_ctrlTaskScrollBtn;
#endif

	COLORREF				m_clrTabTextActive;
	COLORREF				m_clrTabTextInactive;
	COLORREF				m_clrTabTextHighlighted;
	CBCGPControlRenderer	m_ctrlTab3D[2];
	COLORREF				m_clrTab3DFace;
	COLORREF				m_clrTab3DBlack;
	COLORREF				m_clrTab3DDark;
	COLORREF				m_clrTab3DDarkShadow;
	COLORREF				m_clrTab3DLight;
	COLORREF				m_clrTab3DHighlight;
	COLORREF				m_clrTab3DTextActive;
	COLORREF				m_clrTab3DTextInactive;
	COLORREF				m_clrTab3DTextHighlighted;
	CBCGPControlRenderer	m_ctrlTabFlat[2];
	COLORREF				m_clrTabFlatFace;
	COLORREF				m_clrTabFlatBlack;
	COLORREF				m_clrTabFlatDark;
	COLORREF				m_clrTabFlatDarkShadow;
	COLORREF				m_clrTabFlatLight;
	COLORREF				m_clrTabFlatHighlight;
	CPen					m_penTabFlatInner[2];
	CPen					m_penTabFlatOuter[2];
	COLORREF				m_clrTabFlatTextActive;
	COLORREF				m_clrTabFlatTextInactive;
	COLORREF				m_clrTabFlatTextHighlighted;

	CBCGPControlRenderer	m_ctrlOutlookWndBar;
	CBCGPControlRenderer	m_ctrlOutlookWndPageBtn;
	COLORREF				m_clrOutlookPageTextNormal;
	COLORREF				m_clrOutlookPageTextHighlighted;
	COLORREF				m_clrOutlookPageTextPressed;
	COLORREF				m_clrOutlookCaptionTextNormal;
	CBCGPControlRenderer	m_ctrlOutlookSplitter;

	CBCGPControlRenderer	m_ctrlScrollBar_Back[2][2];
	CBCGPControlRenderer	m_ctrlScrollBar_Item[2][2];
	CBCGPControlRenderer	m_ctrlScrollBar_ThumbBack[2][2];
	CBCGPControlRenderer	m_ctrlScrollBar_ThumbIcon[2][2];

	CBCGPControlRenderer	m_ctrlSliderThumb[6];

	COLORREF				m_clrRibbonBarBtnText;
	COLORREF				m_clrRibbonBarBtnTextHighlighted;
	COLORREF				m_clrRibbonBarBtnTextDisabled;
	CBCGPControlRenderer	m_ctrlRibbonCaptionQA;
	CBCGPControlRenderer	m_ctrlRibbonCaptionQA_Glass;
	COLORREF				m_clrRibbonCategoryText;
	COLORREF				m_clrRibbonCategoryTextHighlighted;
	COLORREF				m_clrRibbonCategoryTextActive;
	COLORREF				m_clrRibbonCategoryTextDisabled;
	CBCGPControlRenderer	m_ctrlRibbonCategoryBack;
	CBCGPControlRenderer	m_ctrlRibbonCategoryTab;
	CBCGPControlRenderer	m_ctrlRibbonCategoryTabSep;
	CBCGPControlRenderer	m_ctrlRibbonCategoryBtnPage[2];
	COLORREF				m_clrRibbonPanelText;
	COLORREF				m_clrRibbonPanelTextHighlighted;
	COLORREF				m_clrRibbonPanelCaptionText;
	COLORREF				m_clrRibbonPanelCaptionTextHighlighted;
	CBCGPControlRenderer	m_ctrlRibbonPanelBack_T;
	CBCGPControlRenderer	m_ctrlRibbonPanelBack_B;
	CBCGPToolBarImages		m_RibbonPanelSeparator;
	CBCGPControlRenderer	m_ctrlRibbonPanelQAT;
	CBCGPControlRenderer	m_ctrlRibbonMainPanel;
	CBCGPControlRenderer	m_ctrlRibbonMainPanelBorder;
	CBCGPControlRenderer	m_ctrlRibbonBtnMainPanel;

	COLORREF				m_clrRibbonMainPanelBkgnd;
	CBrush					m_brRibbonMainPanelBkgnd;

	CBCGPControlRenderer	m_ctrlRibbonBtnGroup_S;
	CBCGPControlRenderer	m_ctrlRibbonBtnGroup_F;
	CBCGPControlRenderer	m_ctrlRibbonBtnGroup_M;
	CBCGPControlRenderer	m_ctrlRibbonBtnGroup_L;
	CBCGPControlRenderer	m_ctrlRibbonBtnGroupMenu_F[2];
	CBCGPControlRenderer	m_ctrlRibbonBtnGroupMenu_M[2];
	CBCGPControlRenderer	m_ctrlRibbonBtnGroupMenu_L[2];
	CBCGPControlRenderer	m_ctrlRibbonBtn[2];
	CBCGPControlRenderer	m_ctrlRibbonBtnMenuH[2];
	CBCGPControlRenderer	m_ctrlRibbonBtnMenuV[2];
	CBCGPControlRenderer	m_ctrlRibbonBtnLaunch;
	CBCGPToolBarImages		m_RibbonBtnLaunchIcon;
	CBCGPControlRenderer	m_RibbonBtnMain;
	CBCGPControlRenderer	m_ctrlRibbonBtnDefault;
	CBCGPControlRenderer	m_ctrlRibbonBtnDefaultIcon;
	CBCGPToolBarImages		m_RibbonBtnDefaultImage;
	CBCGPControlRenderer	m_ctrlRibbonBtnDefaultQATIcon;
	CBCGPControlRenderer	m_ctrlRibbonBtnDefaultQAT;
	CBCGPControlRenderer	m_ctrlRibbonBtnCheck;
	CBCGPControlRenderer	m_ctrlRibbonBtnRadio;
	CBCGPControlRenderer	m_ctrlRibbonBtnPush;
	CBCGPControlRenderer	m_ctrlRibbonBtnGroup;
	CBCGPControlRenderer	m_ctrlRibbonBtnPalette[3];
	CBCGPControlRenderer	m_ctrlRibbonBtnStatusPane;
	CBCGPControlRenderer	m_ctrlRibbonSliderThumb;
	CBCGPControlRenderer	m_ctrlRibbonSliderThumbA[5];
	CBCGPControlRenderer	m_ctrlRibbonSliderBtnPlus;
	CBCGPControlRenderer	m_ctrlRibbonSliderBtnMinus;

	CBCGPControlRenderer	m_ctrlRibbonProgressBack;
	CBCGPControlRenderer	m_ctrlRibbonProgressNormal;
	CBCGPControlRenderer	m_ctrlRibbonProgressNormalExt;
	CBCGPControlRenderer	m_ctrlRibbonProgressInfinity;
	CBCGPControlRenderer	m_ctrlRibbonProgressBackV;
	CBCGPControlRenderer	m_ctrlRibbonProgressNormalV;
	CBCGPControlRenderer	m_ctrlRibbonProgressNormalExtV;
	CBCGPControlRenderer	m_ctrlRibbonProgressInfinityV;

	CBCGPControlRenderer	m_ctrlRibbonBorder_QAT;
	CBCGPControlRenderer	m_ctrlRibbonBorder_Floaty;
	CBCGPControlRenderer	m_ctrlRibbonBorder_Panel;
	CBCGPControlRenderer	m_ctrlRibbonKeyTip;
	COLORREF				m_clrRibbonKeyTipTextNormal;
	COLORREF				m_clrRibbonKeyTipTextDisabled;

	COLORREF m_clrRibbonEdit;
	COLORREF m_clrRibbonEditDisabled;
	COLORREF m_clrRibbonEditHighlighted;
	COLORREF m_clrRibbonEditPressed;
	COLORREF m_clrRibbonEditBorder;
	COLORREF m_clrRibbonEditBorderDisabled;
	COLORREF m_clrRibbonEditBorderHighlighted;
	COLORREF m_clrRibbonEditBorderPressed;
	COLORREF m_clrRibbonEditSelection;
	COLORREF m_clrRibbonComboBtnStart;
	COLORREF m_clrRibbonComboBtnFinish;
	COLORREF m_clrRibbonComboBtnBorder;
	COLORREF m_clrRibbonComboBtnDisabledStart;
	COLORREF m_clrRibbonComboBtnDisabledFinish;
	COLORREF m_clrRibbonComboBtnBorderDisabled;
	COLORREF m_clrRibbonComboBtnPressedStart;
	COLORREF m_clrRibbonComboBtnPressedFinish;
	COLORREF m_clrRibbonComboBtnBorderPressed;
	COLORREF m_clrRibbonComboBtnHighlightedStart;
	COLORREF m_clrRibbonComboBtnHighlightedFinish;
	COLORREF m_clrRibbonComboBtnBorderHighlighted;
	CBCGPControlRenderer m_ctrlRibbonComboBoxBtn;

	struct XRibbonContextCategory
	{
		CBCGPControlRenderer	m_ctrlCaption;
		CBCGPControlRenderer	m_ctrlBack;
		CBCGPControlRenderer	m_ctrlTab;
		CBCGPControlRenderer	m_ctrlBtnDefault;
		COLORREF				m_clrCaptionText;
		COLORREF				m_clrText;
		COLORREF				m_clrTextHighlighted;
		COLORREF				m_clrTextActive;
		CBCGPBitmapCache		m_cacheBack;
		CBCGPBitmapCache		m_cacheBtnDefault;

		void CleanUp ()
		{
			m_ctrlCaption.CleanUp ();
			m_ctrlBack.CleanUp ();
			m_ctrlTab.CleanUp ();
			m_ctrlBtnDefault.CleanUp ();
			m_cacheBack.Clear ();
			m_cacheBtnDefault.Clear ();
		}
	};

	CBCGPControlRenderer	m_ctrlRibbonContextPanelBack_T;
	CBCGPControlRenderer	m_ctrlRibbonContextPanelBack_B;
	COLORREF				m_clrRibbonContextPanelText;
	COLORREF				m_clrRibbonContextPanelTextHighlighted;
	COLORREF				m_clrRibbonContextPanelCaptionText;
	COLORREF				m_clrRibbonContextPanelCaptionTextHighlighted;
	CBCGPBitmapCache		m_cacheRibbonContextPanelBack_T;
	CBCGPBitmapCache		m_cacheRibbonContextPanelBack_B;
	CBCGPControlRenderer	m_ctrlRibbonContextSeparator;
	XRibbonContextCategory	m_ctrlRibbonContextCategory[BCGPRibbonCategoryColorCount];

	static Style			m_Style;
	static CString			m_strStylePrefix;
	static HINSTANCE		m_hinstRes;
	static BOOL				m_bAutoFreeRes;
	static COLORREF			m_clrBase;
	static COLORREF			m_clrTarget;

	CBCGPBitmapCache		m_cacheRibbonCategoryBack;
	CBCGPBitmapCache		m_cacheRibbonPanelBack_T;
	CBCGPBitmapCache		m_cacheRibbonPanelBack_B;
	CBCGPBitmapCache		m_cacheRibbonBtnDefault;

	CBCGPBitmapCache		m_cacheRibbonBtnGroup_S;
	CBCGPBitmapCache		m_cacheRibbonBtnGroup_F;
	CBCGPBitmapCache		m_cacheRibbonBtnGroup_M;
	CBCGPBitmapCache		m_cacheRibbonBtnGroup_L;
	CBCGPBitmapCache		m_cacheRibbonBtnGroupMenu_F[2];
	CBCGPBitmapCache		m_cacheRibbonBtnGroupMenu_M[2];
	CBCGPBitmapCache		m_cacheRibbonBtnGroupMenu_L[2];

	int 					m_nType;
	BOOL					m_bPlannerBlack;

	COLORREF				m_clrPlannerTodayCaption[4];
	COLORREF				m_clrPlannerTodayBorder;
	COLORREF				m_clrPlannerNcArea;
	COLORREF				m_clrPlannerNcLine;
	COLORREF				m_clrPlannerNcText;

	COLORREF				m_clrGridHeaderNormalStart;
	COLORREF				m_clrGridHeaderNormalFinish;
	COLORREF				m_clrGridHeaderNormalBorder;
	COLORREF				m_clrGridHeaderPressedStart;
	COLORREF				m_clrGridHeaderPressedFinish;
	COLORREF				m_clrGridHeaderPressedBorder;
	COLORREF				m_clrGridHeaderAllNormalBackStart;
	COLORREF				m_clrGridHeaderAllNormalBackFinish;
	COLORREF				m_clrGridHeaderAllPressedBackStart;
	COLORREF				m_clrGridHeaderAllPressedBackFinish;
	COLORREF				m_clrGridHeaderAllNormalBorderHighlighted;
	COLORREF				m_clrGridHeaderAllNormalBorderShadow;
	COLORREF				m_clrGridHeaderAllPressedBorderHighlighted;
	COLORREF				m_clrGridHeaderAllPressedBorderShadow;
	COLORREF				m_clrGridHeaderAllNormalSignStart;
	COLORREF				m_clrGridHeaderAllNormalSignFinish;
	COLORREF				m_clrGridHeaderAllPressedSignStart;
	COLORREF				m_clrGridHeaderAllPressedSignFinish;
	COLORREF				m_clrGridSeparator;
	CPen					m_penGridSeparator;
	COLORREF				m_clrGridLeftOffset;
	COLORREF				m_clrGridGroupLine;
	COLORREF				m_clrGridGroupSubLine;

	COLORREF				m_clrPopupGradientLight;
	COLORREF				m_clrPopupGradientDark;

	CBCGPToolTipParams		m_ToolTipParams;
	BOOL					m_bToolTipParams;

	COLORREF				m_clrRibbonHyperlinkInactive;
	COLORREF				m_clrRibbonHyperlinkActive;
	COLORREF				m_clrRibbonStatusbarHyperlinkInactive;
	COLORREF				m_clrRibbonStatusbarHyperlinkActive;

	COLORREF				m_clrDlgBackground;
	CBrush					m_brDlgBackground;
	
	COLORREF				m_clrDlgButtonsArea;

	CBCGPControlRenderer	m_ctrlCalculatorBtn;
	COLORREF				m_clrCalculatorBtnText[3];

	CBrush					m_brCtrlBackground;

#ifndef BCGP_EXCLUDE_GRID_CTRL
	BCGP_GRID_COLOR_DATA	m_GridColors;
#endif

	BOOL					IsBeta1  () const {	return m_nType == 10;	}
	BOOL					IsBeta  () const {	return m_nType < 20;	}

	BOOL					m_bUseScenicColors;

	virtual BOOL ParseStyleXML (CBCGPTagManager& tm);
	virtual BOOL ParseStyleXMLVersion (const CString& strItem);
	virtual BOOL ParseStyleXMLGlobals (const CString& strItem);
	virtual BOOL ParseStyleXMLMainWnd (const CString& strItem);
	virtual BOOL ParseStyleXMLMenu (const CString& strItem);
	virtual BOOL ParseStyleXMLBars (const CString& strItem);
	virtual BOOL ParseStyleXMLEdit (const CString& strItem);
	virtual BOOL ParseStyleXMLCombo (const CString& strItem);
	virtual BOOL ParseStyleXMLTaskPane (const CString& strItem);
	virtual BOOL ParseStyleXMLTabs (const CString& strItem);
	virtual BOOL ParseStyleXMLHeader (const CString& strItem);
	virtual BOOL ParseStyleXMLRibbon (const CString& strItem);
	virtual BOOL ParseStyleXMLPlanner (const CString& strItem);
	virtual BOOL ParseStyleXMLGrid (const CString& strItem);
	virtual BOOL ParseStyleXMLOutlook (const CString& strItem);
	virtual BOOL ParseStyleXMLPopup (const CString& strItem);
	virtual BOOL ParseStyleXMLSlider (const CString& strItem);
	virtual BOOL ParseStyleXMLCalculator (const CString& strItem);
};

#endif // !defined(AFX_BCGPVISUALMANAGER2007_H__4950864A_0CAE_4A54_8717_59381C1B05B8__INCLUDED_)
