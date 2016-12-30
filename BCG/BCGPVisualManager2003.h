//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGPVisualManager2003.h: interface for the CBCGPVisualManager2003 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPVISUALMANAGER2003_H__44F195EA_6269_4D0C_80D4_2E53D5D1D0DB__INCLUDED_)
#define AFX_BCGPVISUALMANAGER2003_H__44F195EA_6269_4D0C_80D4_2E53D5D1D0DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPVisualManagerXP.h"

#ifndef BCGP_EXCLUDE_PLANNER
class CBCGPPlannerManagerCtrl;
#endif

class BCGCBPRODLLEXPORT CBCGPVisualManager2003 : public CBCGPVisualManagerXP
{
	DECLARE_DYNCREATE(CBCGPVisualManager2003)

public:
	CBCGPVisualManager2003();
	virtual ~CBCGPVisualManager2003();

	static void SetUseGlobalTheme (BOOL bUseGlobalTheme = TRUE);
	static BOOL IsUseGlobalTheme ()
	{
		return m_bUseGlobalTheme;
	}

	virtual BOOL IsWinXPThemeSupported () const	{	return m_hThemeWindow != NULL;	}

	static void SetStatusBarOfficeXPLook (BOOL bStatusBarOfficeXPLook = TRUE);
	static BOOL IsStatusBarOfficeXPLook ()
	{
		return m_bStatusBarOfficeXPLook;
	}

	static void SetDefaultWinXPColors (BOOL bDefaultWinXPColors = TRUE);
	static BOOL IsDefaultWinXPColorsEnabled ()
	{
		return m_bDefaultWinXPColors;
	}

	virtual BOOL IsFlatTheme () const { return m_clrHighlightGradientLight == m_clrHighlightGradientDark && m_clrHighlightDnGradientLight == m_clrHighlightDnGradientDark; }
	virtual COLORREF GetBaseThemeColor ();

	virtual COLORREF GetHighlightedColor(UINT nType);

	virtual void OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);
	virtual void OnDrawBarBorder (CDC* pDC, CBCGPBaseControlBar* pBar, CRect& rect);
	virtual void OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CBCGPBaseControlBar* pBar);
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
	virtual void OnFillOutlookPageButton (	CDC* pDC, const CRect& rect,
										BOOL bIsHighlighted, BOOL bIsPressed,
										COLORREF& clrText);
	virtual void OnDrawOutlookPageButtonBorder (CDC* pDC, CRect& rectBtn, 
										BOOL bIsHighlighted, BOOL bIsPressed);
	virtual void OnDrawOutlookBarSplitter (CDC* pDC, CRect rectSplitter);
	virtual void OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, COLORREF& clrText);
	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawButtonBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);
	virtual void OnDrawSeparator (CDC* pDC, CBCGPBaseControlBar* pBar, CRect rect, BOOL bIsHoriz);

	virtual int GetToolBarCustomizeButtonMargin () const
	{
		return 1;
	}

	virtual COLORREF GetToolbarDisabledColor () const
	{
		return m_clrToolbarDisabled;
	}

	virtual BOOL IsToolbarRoundShape (CBCGPToolBar* pToolBar);

	virtual void OnHighlightQuickCustomizeMenuButton (CDC* pDC, CBCGPToolbarMenuButton* pButton, CRect rect);
	virtual COLORREF OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected = FALSE);

	virtual void DrawCustomizeButton (CDC* pDC, CRect rect, BOOL bIsHorz,
						  CBCGPVisualManager::BCGBUTTON_STATE state,
						  BOOL bIsCustomize, BOOL bIsMoreButtons);

	virtual void OnUpdateSystemColors ();
	virtual void OnFillHighlightedArea (CDC* pDC, CRect rect, CBrush* pBrush,
		CBCGPToolbarButton* pButton);

	virtual BOOL IsOffsetPressedButton () const
	{
		return FALSE;
	}

	virtual int GetShowAllMenuItemsHeight (CDC* pDC, const CSize& sizeDefault);
	virtual void OnDrawShowAllMenuItems (CDC* pDC, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawCaptionBarBorder (CDC* pDC, CBCGPCaptionBar* pBar, CRect rect, COLORREF clrBarBorder, BOOL bFlatBorder);
	virtual void OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive);

	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPPopupMenu* pMenu, CRect rect);
	virtual void OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed);

	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd);
	virtual BOOL OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd);
	virtual void OnEraseTabsButton (CDC* pDC, CRect rect, CBCGPButton* pButton,
									CBCGPBaseTabWnd* pWndTab);
	virtual void OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
										CBCGPButton* pButton, UINT uiState, CBCGPBaseTabWnd* pWndTab);
	virtual void OnDrawTab (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd);
	virtual void OnFillTab (CDC* pDC, CRect rectFill, CBrush* pbrFill, int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd);
	virtual void GetTabFrameColors (const CBCGPBaseTabWnd* pTabWnd,
				   COLORREF& clrDark,
				   COLORREF& clrBlack,
				   COLORREF& clrHighlight,
				   COLORREF& clrFace,
				   COLORREF& clrDarkShadow,
				   COLORREF& clrLight,
				   CBrush*& pbrFace,
				   CBrush*& pbrBlack);
	virtual BOOL IsHighlightOneNoteTabs () const	{	return TRUE;	}
	virtual BOOL IsDockingTabHasBorder ()			{	return FALSE;	}

#ifndef BCGP_EXCLUDE_PROP_LIST
	virtual COLORREF GetPropListGroupColor (CBCGPPropList* pPropList);
	virtual COLORREF GetPropListGroupTextColor (CBCGPPropList* pPropList);
#endif

	// Tasks pane:
#ifndef BCGP_EXCLUDE_TASK_PANE
	virtual void OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea);

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
#endif

	virtual void OnDrawStatusBarProgress (CDC* pDC, CBCGPStatusBar* pStatusBar,
				CRect rectProgress, int nProgressTotal, int nProgressCurr,
				COLORREF clrBar, COLORREF clrProgressBarDest, COLORREF clrProgressText,
				BOOL bProgressText);
	virtual void OnDrawStatusBarPaneBorder (CDC* pDC, CBCGPStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle);

	virtual COLORREF OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
		BOOL bActive, CRect rectCaption, CRect rectButtons);

	// Auto-hide buttons:
	virtual void OnFillAutoHideButtonBackground (CDC* pDC, CRect rect, CBCGPAutoHideButton* pButton);
	virtual void OnDrawAutoHideButtonBorder (CDC* pDC, CRect rectBounds, CRect rectBorderSize, CBCGPAutoHideButton* pButton);

	virtual BOOL DrawPushButtonWinXP (CDC* pDC, CRect rect, CBCGPButton* pButton, UINT uiState)	
	{	
		return DrawPushButton (pDC, rect, pButton, uiState);
	}

	virtual BOOL DrawComboDropButtonWinXP (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted)
	{
		return DrawComboDropButton (pDC, rect,
										bDisabled,
										bIsDropped,
										bIsHighlighted);
	}

	virtual BOOL DrawComboBorderWinXP (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted)
	{
		return DrawComboBorder (pDC, rect,
								bDisabled,
								bIsDropped,
								bIsHighlighted);
	}

	// Calculator:
	virtual BOOL OnDrawCalculatorButton (CDC* pDC, CRect rect, CBCGPToolbarButton* pButton, CBCGPVisualManager::BCGBUTTON_STATE state, int cmd /* CBCGPCalculator::CalculatorCommands */, CBCGPCalculator* pCalculator);
	virtual BOOL OnDrawCalculatorDisplay (CDC* pDC, CRect rect, const CString& strText, BOOL bMem, CBCGPCalculator* pCalculator);

	// Edit box:
	virtual BOOL OnDrawBrowseButton (CDC* pDC, CRect rect, CBCGPEdit* pEdit, CBCGPVisualManager::BCGBUTTON_STATE state, COLORREF& clrText);

	virtual void OnDrawControlBorder (CDC* pDC, CRect rect, CWnd* pWndCtrl, BOOL bDrawOnGlass);

	virtual void OnDrawExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox);

	virtual void GetSmartDockingBaseMarkerColors (
		COLORREF& clrBaseGroupBackground,
		COLORREF& clrBaseGroupBorder);

	virtual COLORREF GetSmartDockingMarkerToneColor ();

	virtual void OnDrawStatusBarSizeBox (CDC* pDC, CBCGPStatusBar* pStatBar,
				CRect rectSizeBox);

	virtual void OnDrawHeaderCtrlBorder (CBCGPHeaderCtrl* pCtrl, CDC* pDC,
		CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted);

	virtual void OnDrawHeaderCtrlSortArrow (CBCGPHeaderCtrl* pCtrl, CDC* pDC, CRect& rect, BOOL bIsUp);

	// Toolbox:
#ifndef BCGP_EXCLUDE_TOOLBOX
	virtual BOOL OnEraseToolBoxButton (CDC* pDC, CRect rect, CBCGPToolBoxButton* pButton);
	virtual BOOL OnDrawToolBoxButtonBorder (CDC* pDC, CRect& rect, CBCGPToolBoxButton* pButton, UINT uiState);
#endif

	// Popup window:
#ifndef BCGP_EXCLUDE_POPUP_WINDOW
	virtual void OnFillPopupWindowBackground (CDC* pDC, CRect rect);
	virtual void OnDrawPopupWindowBorder (CDC* pDC, CRect rect);
	virtual void OnDrawPopupWindowRoundedBorder (CDC* pDC, CRect rect, CBCGPPopupWindow* pPopupWnd, int nCornerRadius);
	virtual COLORREF OnDrawPopupWindowCaption (CDC* pDC, CRect rectCaption, CBCGPPopupWindow* pPopupWnd);
	virtual COLORREF GetPopupWindowCaptionTextColor(CBCGPPopupWindow* pPopupWnd, BOOL bButton);
	virtual void OnErasePopupWindowButton (CDC* pDC, CRect rectClient, CBCGPPopupWndButton* pButton);
	virtual void OnDrawPopupWindowButtonBorder (CDC* pDC, CRect rectClient, CBCGPPopupWndButton* pButton);
#endif

	virtual COLORREF GetHighlightMenuItemColor () const { return m_clrHighlightMenuItem; }

	// Day planner:
#ifndef BCGP_EXCLUDE_PLANNER
	virtual COLORREF OnFillPlannerCaption (CDC* pDC, CBCGPPlannerView* pView,
		CRect rect, BOOL bIsToday, BOOL bIsSelected, BOOL bNoBorder = FALSE,
		BOOL bHorz = TRUE);

	virtual void OnFillPlanner (CDC* pDC, CBCGPPlannerView* pView, 
		CRect rect, BOOL bWorkingArea);

	virtual COLORREF GetPlannerHourLineColor (CBCGPPlannerView* pView,
		BOOL bWorkingHours, BOOL bHour);

	virtual COLORREF GetPlannerNonWorkingColor (COLORREF clrWorking);

	virtual COLORREF GetPlannerViewBackgroundColor (CBCGPPlannerView* pView);
	virtual COLORREF GetPlannerViewWorkingColor (CBCGPPlannerView* pView);
	virtual COLORREF GetPlannerViewNonWorkingColor (CBCGPPlannerView* pView);

	virtual void OnDrawPlannerTimeLine (CDC* pDC, CBCGPPlannerView* pView, CRect rect);
#endif // BCGP_EXCLUDE_PLANNER

	virtual void GetCalendarColors (const CBCGPCalendar* pCalendar,
				   CBCGPCalendarColors& colors);

#ifndef BCGP_EXCLUDE_GRID_CTRL
	// Grid control:
	virtual void OnDrawGridExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox);
	virtual void OnFillGridHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual BOOL OnDrawGridHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnFillGridRowHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual BOOL OnDrawGridRowHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnFillGridSelectAllAreaBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnDrawGridSelectAllMarker(CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, int nPadding, BOOL bPressed);
	virtual BOOL OnDrawGridSelectAllAreaBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnDrawGridSelectionBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual BOOL OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme);

	// Report control:
	virtual COLORREF GetReportCtrlGroupBackgoundColor ();

	virtual CBrush& GetGridCaptionBrush(CBCGPGridCtrl* /*pCtrl*/)
	{
		return m_brFace.GetSafeHandle () != NULL ? m_brFace : m_brBarBkgnd;
	}
	
#endif // BCGP_EXCLUDE_GRID_CTRL

    // Gantt control:
#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)
	virtual void GetGanttColors (const CBCGPGanttChart* pChart, BCGP_GANTT_CHART_COLORS& colors, COLORREF clrBack = CLR_DEFAULT) const;		
    virtual void DrawGanttHeaderCell (const CBCGPGanttChart* pChart, CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, BOOL bHilite);
    virtual void FillGanttBar (const CBCGPGanttItem* pItem, CDC& dc, const CRect& rectFill, COLORREF color, double dGlowLine);
#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)


	virtual void OnDrawCheckBoxEx (CDC *pDC, CRect rect,
										 int nState,
										 BOOL bHighlighted, 
										 BOOL bPressed,
										 BOOL bEnabled);
	virtual void OnDrawRadioButton (CDC *pDC, CRect rect, 
											BOOL bOn,
											BOOL bHighlighted, 
											BOOL bPressed,
											BOOL bEnabled);
	virtual BOOL GetToolTipParams (CBCGPToolTipParams& params, UINT nType = (UINT)(-1));

	// Property list:
#ifndef BCGP_EXCLUDE_PROP_LIST
	virtual COLORREF OnFillPropertyListSelectedItem(CDC* pDC, CBCGPProp* pProp, CBCGPPropList* pWndList, const CRect& rectFill, BOOL bFocused);
#endif

#ifndef BCGP_EXCLUDE_RIBBON
	// Ribbon control:
	virtual COLORREF OnDrawRibbonCategoryTab (
					CDC* pDC, 
					CBCGPRibbonTab* pTab,
					BOOL bIsActive);

	virtual COLORREF OnDrawRibbonButtonsGroup (
					CDC* pDC,
					CBCGPRibbonButtonsGroup* pGroup,
					CRect rectGroup);

	virtual COLORREF OnDrawRibbonCategoryCaption (
					CDC* pDC, 
					CBCGPRibbonContextCaption* pContextCaption);

	virtual COLORREF OnDrawRibbonStatusBarPane (CDC* pDC, CBCGPRibbonStatusBar* pBar,
					CBCGPRibbonStatusBarPane* pPane);

	virtual void GetRibbonSliderColors (CBCGPRibbonSlider* pSlider, 
					BOOL bIsHighlighted, 
					BOOL bIsPressed,
					BOOL bIsDisabled,
					COLORREF& clrLine,
					COLORREF& clrFill);

	virtual void OnDrawRibbonProgressBar (
			CDC* pDC, CBCGPRibbonProgressBar* pProgress, 
			CRect rectProgress, CRect rectChunk, BOOL bInfiniteMode);

	virtual void OnDrawRibbonQATSeparator (CDC* pDC, CBCGPRibbonSeparator* pSeparator, CRect rect);

	virtual COLORREF OnFillRibbonPanelCaption (CDC* pDC, CBCGPRibbonPanel* pPanel, CRect rectCaption);

#endif

protected:
	WinXpTheme	m_WinXPTheme;

	COLORREF GetThemeColor (HTHEME hTheme, int nIndex) const;

	COLORREF	m_clrBarGradientDark;
	COLORREF	m_clrBarGradientLight;

	COLORREF	m_clrToolBarGradientDark;
	COLORREF	m_clrToolBarGradientLight;

	COLORREF	m_clrToolbarDisabled;

	COLORREF	m_clrToolBarGradientVertLight;
	COLORREF	m_clrToolBarGradientVertDark;

	COLORREF	m_clrCustomizeButtonGradientDark;
	COLORREF	m_clrCustomizeButtonGradientLight;

	COLORREF	m_clrToolBarBottomLine;
	CPen		m_penBottomLine;

	COLORREF	m_colorToolBarCornerTop;
	COLORREF	m_colorToolBarCornerBottom;

	COLORREF	m_clrHighlightMenuItem;

	COLORREF	m_clrHighlightGradientLight;
	COLORREF	m_clrHighlightGradientDark;

	COLORREF	m_clrHighlightDnGradientLight;
	COLORREF	m_clrHighlightDnGradientDark;

	COLORREF	m_clrHighlightCheckedGradientLight;
	COLORREF	m_clrHighlightCheckedGradientDark;

	CPen		m_penSeparatorLight;

	COLORREF	m_clrGripper;

	COLORREF	m_clrCaptionBarGradientLight;
	COLORREF	m_clrCaptionBarGradientDark;

	CBrush		m_brTearOffCaption;
	CBrush		m_brFace;

	COLORREF	m_clrTaskPaneGradientDark;
	COLORREF	m_clrTaskPaneGradientLight;
	COLORREF	m_clrTaskPaneGroupCaptionDark;
	COLORREF	m_clrTaskPaneGroupCaptionLight;
	COLORREF	m_clrTaskPaneGroupCaptionSpecDark;
	COLORREF	m_clrTaskPaneGroupCaptionSpecLight;
	COLORREF	m_clrTaskPaneGroupAreaLight;
	COLORREF	m_clrTaskPaneGroupAreaDark;
	COLORREF	m_clrTaskPaneGroupAreaSpecLight;
	COLORREF	m_clrTaskPaneGroupAreaSpecDark;
	COLORREF	m_clrTaskPaneGroupBorder;

	CPen		m_penTaskPaneGroupBorder;

	CPen		m_penGridExpandBoxLight;
	CPen		m_penGridExpandBoxDark;

	COLORREF	m_clrGridSelectAllActive;

	COLORREF	m_clrPlannerTodayFill;
	COLORREF	m_clrPlannerTodayLine;
	CPen		m_penPlannerTodayLine;
	
	BOOL		m_bIsStandardWinXPTheme;
	BOOL		m_bEmbossGripper;
	BOOL		m_bTabFillGradient;

	static BOOL	m_bUseGlobalTheme;
	static BOOL m_bStatusBarOfficeXPLook;
	static BOOL	m_bDefaultWinXPColors;

	virtual void ModifyGlobalColors ();
	virtual COLORREF GetWindowColor () const;
};

#endif // !defined(AFX_BCGPVISUALMANAGER2003_H__44F195EA_6269_4D0C_80D4_2E53D5D1D0DB__INCLUDED_)
