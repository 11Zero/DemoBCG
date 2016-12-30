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
// BCGWinXPVisualManager.h: interface for the CBCGPWinXPVisualManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPWINXPVISUALMANAGER_H__0795BCE7_8E67_4145_A840_D9655AC0293D__INCLUDED_)
#define AFX_BCGPWINXPVISUALMANAGER_H__0795BCE7_8E67_4145_A840_D9655AC0293D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPVisualManagerXP.h"
#include "BCGGlobals.h"

class CBCGPButton;

class BCGCBPRODLLEXPORT CBCGPWinXPVisualManager : public CBCGPVisualManagerXP
{
	DECLARE_DYNCREATE(CBCGPWinXPVisualManager)

public:
	CBCGPWinXPVisualManager(BOOL bIsTemporary = FALSE);
	virtual ~CBCGPWinXPVisualManager();

	static BOOL IsWinXPThemeAvailible ();

	void SetOfficeStyleMenus (BOOL bOn = TRUE);
	BOOL IsOfficeStyleMenus () const
	{
		return m_bOfficeStyleMenus;
	}

	static BOOL	m_b3DTabsXPTheme;

	virtual BOOL IsWinXPThemeSupported () const	{	return m_hThemeWindow != NULL;	}

	virtual void OnUpdateSystemColors ();

	virtual int GetPopupMenuGap () const
	{
		return m_bOfficeStyleMenus ? 0 : 1;
	}

	virtual void OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);
	virtual void OnDrawBarBorder (CDC* pDC, CBCGPBaseControlBar* pBar, CRect& rect);
	virtual void OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CBCGPBaseControlBar* pBar);
	virtual void OnDrawSeparator (CDC* pDC, CBCGPBaseControlBar* pBar, CRect rect, BOOL bIsHoriz);
	virtual void OnDrawCaptionButton (CDC* pDC, CBCGPCaptionButton* pButton, BOOL bActive, BOOL bHorz, 
										BOOL bMaximized, BOOL bDisabled, int nImageID = -1);
	virtual COLORREF OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
		BOOL bActive, CRect rectCaption, CRect rectButtons);

	virtual void OnDrawCaptionButtonIcon (CDC* pDC, 
											CBCGPCaptionButton* pButton,
											CBCGPMenuImages::IMAGES_IDS id,
											BOOL bActive, BOOL bDisabled,
											CPoint ptImage);
	virtual void OnDrawMenuSystemButton (CDC* pDC, CRect rect, UINT uiSystemCommand, 
										UINT nStyle, BOOL bHighlight);
	virtual void OnDrawStatusBarPaneBorder (CDC* pDC, CBCGPStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle);

	virtual void OnDrawStatusBarProgress (CDC* pDC, CBCGPStatusBar* pStatusBar,
				CRect rectProgress, int nProgressTotal, int nProgressCurr,
				COLORREF clrBar, COLORREF clrProgressBarDest, COLORREF clrProgressText,
				BOOL bProgressText);
	virtual void OnDrawStatusBarSizeBox (CDC* pDC, CBCGPStatusBar* pStatBar,
				CRect rectSizeBox);

	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPPopupMenu* pMenu, CRect rect);
	virtual void OnDrawMenuResizeBar (CDC* pDC, CRect rect, int nResizeFlags);
	virtual void OnFillPopupMenuBackground (CDC* pDC, CRect rect);
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
	virtual void OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive);

	virtual COLORREF GetToolbarButtonTextColor (CBCGPToolbarButton* pButton,
												CBCGPVisualManager::BCGBUTTON_STATE state);
	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawButtonBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnFillMenuImageRect (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawButtonSeparator (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state,
		BOOL bHorz);

	virtual void OnHighlightMenuItem (CDC *pDC, CBCGPToolbarMenuButton* pButton,
		CRect rect, COLORREF& clrText);
	virtual COLORREF GetHighlightedMenuItemTextColor (CBCGPToolbarMenuButton* pButton);
	virtual void OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed);
	virtual BOOL IsHighlightWholeMenuItem ()	{	return m_bOfficeStyleMenus || m_hThemeMenu != NULL;	}

	// Tab control:
	virtual void OnDrawTab (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd);
	virtual void OnDrawTabCloseButton (CDC* pDC, CRect rect, 
											   const CBCGPBaseTabWnd* pTabWnd, 
											   BOOL bIsHighlighted,
											   BOOL bIsPressed,
											   BOOL bIsDisabled);
	virtual void OnEraseTabsButton (CDC* pDC, CRect rect,
											  CBCGPButton* pButton,
											  CBCGPBaseTabWnd* pWndTab);
	virtual void OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
												 CBCGPButton* pButton, UINT uiState,
												 CBCGPBaseTabWnd* pWndTab);

	virtual BOOL AlwaysHighlight3DTabs () const		{	return m_hThemeTab != NULL;	}

	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd);
	virtual BOOL OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd);

	// Miniframe
	virtual COLORREF OnFillMiniFrameCaption (CDC* pDC, CRect rectCaption, 
											CBCGPMiniFrameWnd* pFrameWnd, 
											BOOL bActive);
	virtual void OnDrawMiniFrameBorder (CDC* pDC, CBCGPMiniFrameWnd* pFrameWnd,
										CRect rectBorder, CRect rectBorderSize);

	virtual void OnDrawFloatingToolbarBorder (	CDC* pDC, CBCGPBaseToolBar* pToolBar, 
												CRect rectBorder, CRect rectBorderSize);

	virtual int GetDockingBarCaptionExtraHeight () const
	{
		return globalData.bIsWindowsVista ? 0 : 3;
	}

	// Outlook bar page buttons:
	virtual void OnFillOutlookPageButton (	CDC* pDC, const CRect& rect,
										BOOL bIsHighlighted, BOOL bIsPressed,
										COLORREF& clrText);
	virtual void OnDrawOutlookPageButtonBorder (CDC* pDC, 
							CRect& rectBtn, BOOL bIsHighlighted, BOOL bIsPressed);

	// Customization dialog:
	virtual COLORREF OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected = FALSE);

	virtual CSize GetButtonExtraBorder () const;
	virtual CSize GetCaptionButtonExtraBorder () const;

	virtual void OnDrawHeaderCtrlBorder (CBCGPHeaderCtrl* pCtrl, CDC* pDC,
		CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted);
	virtual void OnDrawHeaderCtrlSortArrow (CBCGPHeaderCtrl* pCtrl, CDC* pDC, CRect& rect, BOOL bIsUp);

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
	
	virtual void OnDrawExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox);

	virtual void OnDrawControlBorder (CDC* pDC, CRect rect, CWnd* pWndCtrl, BOOL bDrawOnGlass);

	// Date/time controls:
	virtual void GetCalendarColors (const CBCGPCalendar* pCalendar,
				   CBCGPCalendarColors& colors);

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

	virtual BOOL IsOfficeXPStyleMenus () const
	{
		return m_bOfficeStyleMenus;
	}

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

	// Edit box:
	virtual BOOL OnDrawBrowseButton (CDC* pDC, CRect rect, CBCGPEdit* pEdit, CBCGPVisualManager::BCGBUTTON_STATE state, COLORREF& clrText);
	virtual void OnDrawSpinButtons (CDC* pDC, CRect rectSpin, int nState, BOOL bOrientation, CBCGPSpinButtonCtrl* pSpinCtrl);

	virtual void OnDrawDateTimeDropButton (CDC* pDC, CRect rect, BOOL bDisabled, BOOL bPressed, BOOL bHighlighted, CBCGPDateTimeCtrl* pCtrl);

	// Popup window:
#ifndef BCGP_EXCLUDE_POPUP_WINDOW
	virtual void OnErasePopupWindowButton (CDC* pDC, CRect rectClient, CBCGPPopupWndButton* pButton);
	virtual void OnDrawPopupWindowButtonBorder (CDC* pDC, CRect rectClient, CBCGPPopupWndButton* pButton);
	virtual BOOL IsDefaultWinXPPopupButton (CBCGPPopupWndButton* pButton) const;
	virtual COLORREF GetPopupWindowCaptionTextColor(CBCGPPopupWindow* pPopupWnd, BOOL bButton);
#endif

	// Grid control:
#ifndef BCGP_EXCLUDE_GRID_CTRL
	virtual void OnFillGridHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual BOOL OnDrawGridHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnFillGridRowHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual BOOL OnDrawGridRowHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnDrawGridSelectAllMarker(CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, int nPadding, BOOL bPressed);
	virtual void OnFillGridSelectAllAreaBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual BOOL OnDrawGridSelectAllAreaBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnDrawGridHeaderMenuButton (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, 
		BOOL bHighlighted, BOOL bPressed, BOOL bDisabled);
	virtual BOOL OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme);

	virtual BOOL IsGridSparklineDefaultSelColor() const
	{
		return FALSE;
	}

#endif

    // Gantt control:
#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)
    virtual void DrawGanttHeaderCell (const CBCGPGanttChart* pChart, CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, BOOL bHilite);
    virtual void FillGanttBar (const CBCGPGanttItem* pItem, CDC& dc, const CRect& rectFill, COLORREF color, double dGlowLine);
#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

#ifndef BCGP_EXCLUDE_TOOLBOX
	virtual BOOL OnEraseToolBoxButton (CDC* pDC, CRect rect, CBCGPToolBoxButton* pButton);
	virtual BOOL OnDrawToolBoxButtonBorder (CDC* pDC, CRect& rect, CBCGPToolBoxButton* pButton, UINT uiState);
#endif

	// Push button:
	virtual BOOL OnDrawPushButton (CDC* pDC, CRect rect, CBCGPButton* pButton, COLORREF& clrText);
	virtual BOOL IsDrawFocusRectOnPushButton(CBCGPButton* /*pButton*/) 	{ return TRUE; }

#ifndef BCGP_EXCLUDE_RIBBON

	// Ribbon control:
	virtual void OnDrawRibbonProgressBar (
			CDC* pDC, CBCGPRibbonProgressBar* pProgress, 
			CRect rectProgress, CRect rectChunk, BOOL bInfiniteMode);

	virtual void OnDrawRibbonPaletteButton (
		CDC* pDC,
		CBCGPRibbonPaletteIcon* pButton);
#endif

#ifndef BCGP_EXCLUDE_PROP_LIST
	virtual void OnFillPropListToolbarArea(CDC* pDC, CBCGPPropList* pList, const CRect& rectToolBar);
	virtual COLORREF GetPropListGroupColor (CBCGPPropList* pPropList);
	virtual COLORREF GetPropListGroupTextColor (CBCGPPropList* pPropList);
	virtual COLORREF OnFillPropertyListSelectedItem(CDC* pDC, CBCGPProp* pProp, CBCGPPropList* pWndList, const CRect& rectFill, BOOL bFocused);
#endif

	virtual COLORREF OnFillListBoxItem (CDC* pDC, CBCGPListBox* pListBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected);
	virtual COLORREF OnFillComboBoxItem(CDC* pDC, CBCGPComboBox* pComboBox, int nIndex, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected);

protected:
	BOOL		m_bOfficeStyleMenus;
	COLORREF	m_clrCalendarCaption;
	COLORREF	m_clrCalendarCaptionText;
};

#endif // !defined(AFX_BCGPWINXPVISUALMANAGER_H__0795BCE7_8E67_4145_A840_D9655AC0293D__INCLUDED_)
