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
// BCGPVisualManager.h: interface for the CBCGPPVisualManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPVISUALMANAGER_H__8070C021_60CE_11D5_A304_8B7130518C10__INCLUDED_)
#define AFX_BCGPVISUALMANAGER_H__8070C021_60CE_11D5_A304_8B7130518C10__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#define CBCGPRibbonComboEdit CBCGPRibbonEditCtrl

class CBCGPBaseControlBar;
class CBCGPPopupMenu;
class CBCGPBaseToolBar;
class CBCGPToolBar;
class CBCGPToolbarButton;
class CBCGPToolbarMenuButton;
class CBCGPCaptionButton;
class CBCGPCaptionMenuButton;
class CBCGPBaseTabWnd;
class CBCGPToolbarComboBoxButton;
class CBCGPStatusBar;
class CBCGStatusBarPaneInfo;
class CBCGPButton;
class CBCGPMiniFrameWnd;
class CBCGPCaptionBar;
class CBCGPHeaderCtrl;
class CBCGPToolbarEditBoxButton;
class CBCGPDockingControlBar;
class CBCGPTask;
class CBCGPTasksGroup;
class CBCGPSlider;
class CBCGPSplitterWnd;
class CBCGPAutoHideButton;
class CBCGPCalculator;
class CBCGPEdit;
class CBCGPSpinButtonCtrl;
class CBCGPAppBarWnd;
class CBCGPPopupWindow;
class CBCGPPopupWndButton;
class CBCGPPlannerView;
class CBCGPGridCtrl;
class CBCGPCalendar;
class CBCGPCalendarColors;
class CBCGPToolBoxButton;
class CBCGPPropList;
class CBCGPRibbonBar;
class CBCGPRibbonCategory;
class CBCGPRibbonCategoryScroll;
class CBCGPRibbonPanel;
class CBCGPRibbonButton;
class CBCGPRibbonTab;
class CBCGPRibbonCaptionButton;
class CBCGPRibbonButtonsGroup;
class CBCGPRibbonLaunchButton;
class CBCGPRibbonEditCtrl;
class CBCGPRibbonMainPanel;
class CBCGPRibbonLabel;
class CBCGPRibbonPaletteButton;
class CBCGPRibbonPaletteIcon;
class CBCGPRibbonContextCaption;
class CBCGPToolTipParams;
class CBCGTabButton;
class CBCGPTabWnd;
enum BCGPRibbonCategoryColor;
class CBCGPRibbonStatusBar;
class CBCGPRibbonStatusBarPane;
class CBCGPRibbonSlider;
class CBCGPRibbonProgressBar;
class CBCGPRibbonPanelMenuBar;
class CBCGPRibbonPanelMenu;
class CBCGPRibbonSeparator;
class CBCGPBaseRibbonElement;
class CBCGPPropertySheet;
class CBCGPRibbonCheckBox;
class CBCGPRibbonHyperlink;
class CBCGPRibbonColorButton;
class CBCGPRibbonRadioButton;
class CBCGPScrollBar;
class CBCGPToolTipCtrl;
class CBCGPGroup;
class CBCGPSliderCtrl;
class CBCGPURLLinkButton;
class CBCGPListBox;
class CBCGPComboBox;
class CBCGPGanttChart;
class CBCGPGanttItem;
struct BCGP_GANTT_CHART_COLORS;
struct BCGP_GANTT_CHART_HEADER_CELL_INFO;
struct BCGP_GRID_COLOR_DATA;
class CBCGPBreadcrumb;
class CBCGPDateTimeCtrl;
class CBCGPEditCtrl;
class CBCGPDockingBarScrollButton;
class CBCGPRibbonMinimizeButton;
class CBCGPStatic;
class CBCGPProp;
class CBCGPListCtrl;
class CBCGPTreeCtrl;
class CBCGPControlRenderer;

struct tagBREADCRUMBITEMINFO;
typedef struct tagBREADCRUMBITEMINFO BREADCRUMBITEMINFO;

#define	SPIN_PRESSEDUP			0x0001
#define	SPIN_PRESSEDDOWN		0x0002
#define	SPIN_HIGHLIGHTEDUP		0x0004
#define	SPIN_HIGHLIGHTEDDOWN	0x0008
#define	SPIN_DISABLED			0x0010

#ifndef _UXTHEME_H_

enum THEMESIZE
{
    TS_MIN,             // minimum size
    TS_TRUE,            // size without stretching
    TS_DRAW             // size that theme mgr will use to draw part
};

// From uxtheme.h:
typedef HANDLE HTHEME;          // handle to a section of theme data for class

#endif // THEMEAPI

typedef HTHEME (__stdcall * OPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT (__stdcall * CLOSETHEMEDATA)(HTHEME hTheme);
typedef HRESULT (__stdcall * DRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, 
						int iPartId, int iStateId, const RECT *pRect, 
						OPTIONAL const RECT *pClipRect);
typedef HRESULT (__stdcall * GETTHEMECOLOR)(HTHEME hTheme, int iPartId, 
						int iStateId, int iPropId, OUT COLORREF *pColor);
typedef COLORREF (__stdcall * GETTHEMESYSCOLOR)(HTHEME hTheme, int iColorId);

typedef HRESULT (__stdcall * GETCURRENTTHEMENAME)(
    OUT LPWSTR pszThemeFileName, int cchMaxNameChars, 
    OUT OPTIONAL LPWSTR pszColorBuff, int cchMaxColorChars,
    OUT OPTIONAL LPWSTR pszSizeBuff, int cchMaxSizeChars);

typedef HTHEME (__stdcall * GETWINDOWTHEME)(HWND hWnd);

typedef HRESULT (__stdcall * GETTHEMEPARTSIZE)(HTHEME hTheme, HDC hdc, int iPartId, 
						int iStateId, LPCRECT prc, THEMESIZE eSize, SIZE* psz);

class BCGCBPRODLLEXPORT CBCGPWinXPThemeManager : public CObject
{
public:
	CBCGPWinXPThemeManager ();
	virtual ~CBCGPWinXPThemeManager ();

	enum WinXpTheme
	{
		WinXpTheme_None = -1,
		WinXpTheme_NonStandard,
		WinXpTheme_Blue,
		WinXpTheme_Olive,
		WinXpTheme_Silver
	};

	virtual BOOL DrawThemeCaptionButton (CDC* pDC, CRect rect, BOOL bDisabled, BOOL bIsPressed, BOOL bIsHighlighted, BOOL bIsActiveFrame);

	virtual BOOL DrawPushButton (CDC* pDC, CRect rect, CBCGPButton* pButton, UINT uiState);

	virtual BOOL DrawStatusBarProgress (CDC* pDC, CBCGPStatusBar* pStatusBar,
			CRect rectProgress, int nProgressTotal, int nProgressCurr,
			COLORREF clrBar, COLORREF clrProgressBarDest, COLORREF clrProgressText,
			BOOL bProgressText);

	virtual BOOL DrawComboDropButton (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted);
	virtual BOOL DrawComboBorder	(CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted);

	virtual void FillRebarPane		(CDC* pDC, 
									CBCGPBaseControlBar* pBar, 
									CRect rectClient);

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

	virtual BOOL DrawProgressBar (	CDC *pDC, CRect rectProgress, CRect rectChunk, 
									BOOL bInfiniteMode, BOOL bIsVertical,
									int nPos, int nMin, int nMax);
	
	virtual WinXpTheme GetStandardWinXPTheme ();

protected:
	void UpdateSystemColors ();
	void CleanUpThemes ();
	void CleanUpTheme (HTHEME& hTheme);

	HTHEME				m_hThemeWindow;
	HTHEME				m_hThemeToolBar;
	HTHEME				m_hThemeRebar;
	HTHEME				m_hThemeButton;
	HTHEME				m_hThemeStatusBar;
	HTHEME				m_hThemeComboBox;
	HTHEME				m_hThemeProgress;
	HTHEME				m_hThemeHeader;
	HTHEME				m_hThemeScrollBar;
	HTHEME				m_hThemeExplorerBar;
	HTHEME				m_hThemeTree;
	HTHEME				m_hThemeStartPanel;
	HTHEME				m_hThemeTaskBand;
	HTHEME				m_hThemeTaskBar;
	HTHEME				m_hThemeSpin;
	HTHEME				m_hThemeTab;
	HTHEME				m_hThemeToolTip;
	HTHEME				m_hThemeTrack;
	HTHEME				m_hThemeMenu;
	HTHEME				m_hThemeNavigation;

	HINSTANCE			m_hinstUXDLL;

	OPENTHEMEDATA		m_pfOpenThemeData;
	CLOSETHEMEDATA		m_pfCloseThemeData;
	DRAWTHEMEBACKGROUND	m_pfDrawThemeBackground;
	GETTHEMECOLOR		m_pfGetThemeColor;
	GETTHEMESYSCOLOR	m_pfGetThemeSysColor;
	GETCURRENTTHEMENAME	m_pfGetCurrentThemeName;
	GETWINDOWTHEME		m_pfGetWindowTheme;
	GETTHEMEPARTSIZE	m_pfGetThemePartSize;

	CBCGPControlRenderer* m_pRrenderSysCaptionButton;
};

class BCGCBPRODLLEXPORT CBCGPVisualManager : public CBCGPWinXPThemeManager
{
	friend class CBCGPSkinManager;
	friend struct BCGPGLOBAL_DATA;
	friend class CBCGPWinApp;

	DECLARE_DYNCREATE (CBCGPVisualManager)

public:
	CBCGPVisualManager(BOOL bTemporary = FALSE);
	virtual ~CBCGPVisualManager();

	static void SetDefaultManager (CRuntimeClass* pRTI);

	virtual BOOL IsWinXPThemeSupported () const	{	return FALSE;	}
	static void DestroyInstance (BOOL bAutoDestroyOnly = FALSE);

// Operations:
public:
	static void RedrawAll ();
	static void AdjustToolbars ();
	static void AdjustFrames ();

	BOOL IsWindowActive (CWnd* pWnd) const;

// Overrides:
public:
	virtual void OnUpdateSystemColors ();

	virtual BOOL IsDarkTheme () const { return FALSE; }
	virtual BOOL IsFlatTheme () const { return TRUE; }

	virtual BOOL IsDWMCaptionSupported() const;
	virtual BOOL IsSmallSystemBorders() const;

	virtual void OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);
	virtual BOOL OnFillParentBarBackground (CWnd* pWnd, CDC* pDC, LPRECT rectClip = NULL);

	virtual void OnDrawBarBorder (CDC* pDC, CBCGPBaseControlBar* pBar, CRect& rect);
	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPPopupMenu* pMenu, CRect rect);
	virtual void OnDrawMenuShadow (CDC* pDC, const CRect& rectClient, const CRect& rectExclude,
									int nDepth,  int iMinBrightness,  int iMaxBrightness,  
									CBitmap* pBmpSaveBottom,  CBitmap* pBmpSaveRight, BOOL bRTL);
	virtual void OnDrawBarGripper (CDC* pDC, CRect rectGripper, BOOL bHorz, CBCGPBaseControlBar* pBar);
	virtual void OnDrawSeparator (CDC* pDC, CBCGPBaseControlBar* pBar, CRect rect, BOOL bIsHoriz);
	virtual COLORREF GetSeparatorColor();
	virtual COLORREF OnDrawMenuLabel (CDC* pDC, CRect rect);
	virtual COLORREF OnDrawControlBarCaption (CDC* pDC, CBCGPDockingControlBar* pBar, 
		BOOL bActive, CRect rectCaption, CRect rectButtons);
	
	virtual void OnDrawControlBarCaptionText (CDC* pDC, CBCGPDockingControlBar* pBar, BOOL bActive, const CString& strTitle, CRect& rectCaption);
	virtual BOOL IsMinFrameCaptionBold(CBCGPMiniFrameWnd* /* pFrame */) { return TRUE; }

	virtual void OnDrawCaptionButton (CDC* pDC, CBCGPCaptionButton* pButton, BOOL bActive, BOOL bHorz,
										BOOL bMaximized, BOOL bDisabled, int nImageID = -1);
	virtual void OnDrawMenuSystemButton (CDC* pDC, CRect rect, UINT uiSystemCommand, 
										UINT nStyle, BOOL bHighlight);
	virtual void OnDrawStatusBarPaneBorder (CDC* pDC, CBCGPStatusBar* pBar,
					CRect rectPane, UINT uiID, UINT nStyle);
	virtual COLORREF GetStatusBarPaneTextColor (CBCGPStatusBar* pStatusBar, 
									CBCGStatusBarPaneInfo* pPane);

	virtual void OnDrawStatusBarProgress (CDC* pDC, CBCGPStatusBar* /*pStatusBar*/,
										CRect rectProgress, int nProgressTotal, 
										int nProgressCurr,
										COLORREF clrBar, 
										COLORREF clrProgressBarDest, 
										COLORREF clrProgressText,
										BOOL bProgressText);
	virtual void OnDrawStatusBarSizeBox (CDC* pDC, CBCGPStatusBar* pStatBar,
				CRect rectSizeBox);

	virtual BOOL IsStatusBarCoversFrame() { return FALSE; }

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
	virtual void OnFillCombo		(CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted,
										CBCGPToolbarComboBoxButton* pButton);

	virtual COLORREF GetComboboxTextColor(CBCGPToolbarComboBoxButton* pButton, 
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted);

	virtual void OnDrawEditBorder (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsHighlighted,
										CBCGPToolbarEditBoxButton* pButton);
	virtual void OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive);

	virtual void OnDrawMenuResizeBar (CDC* pDC, CRect rect, int nResizeFlags);
	virtual void OnDrawMenuResizeGipper (CDC* pDC, CRect rect, int nResizeFlags, COLORREF clrGripper);
	virtual void OnDrawMenuScrollButton (CDC* pDC, CRect rect, BOOL bIsScrollDown, 
										BOOL bIsHighlited, BOOL bIsPressed, BOOL bIsDisabled);

	virtual HBRUSH GetToolbarEditColors(CBCGPToolbarButton* pButton, COLORREF& clrText, COLORREF& clrBk);

	virtual BOOL AreCustomToolbarCtrlColors()	{	return FALSE;	}

	enum BCGBUTTON_STATE
	{
		ButtonsIsRegular,
		ButtonsIsPressed,
		ButtonsIsHighlighted,
	};

	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawButtonBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawButtonSeparator (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state,
		BOOL bHorz);

	virtual void OnHighlightMenuItem (CDC *pDC, CBCGPToolbarMenuButton* pButton,
		CRect rect, COLORREF& clrText);
	virtual COLORREF GetHighlightedMenuItemTextColor (CBCGPToolbarMenuButton* pButton);
	virtual BOOL IsHighlightWholeMenuItem ()	{	return FALSE;	}
	
	virtual COLORREF GetMenuItemTextColor (CBCGPToolbarMenuButton* pButton, BOOL bHighlighted, BOOL bDisabled);
	virtual int GetMenuDownArrowState (CBCGPToolbarMenuButton* pButton, BOOL bHightlight, BOOL bPressed, BOOL bDisabled);
	virtual void OnHighlightRarelyUsedMenuItems (CDC* pDC, CRect rectRarelyUsed);

	virtual void OnFillMenuImageRect (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state)
	{
		OnFillButtonInterior (pDC, pButton, rect, state);
	}
		
	virtual void OnDrawMenuImageRectBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state)
	{
		OnDrawButtonBorder (pDC, pButton, rect, state);
	}

	virtual void OnDrawMenuCheck (CDC* pDC, CBCGPToolbarMenuButton* pButton, 
		CRect rect, BOOL bHighlight, BOOL bIsRadio);

	virtual void OnDrawMenuItemButton (CDC* pDC, CBCGPToolbarMenuButton* pButton,
				CRect rectButton, BOOL bHighlight, BOOL bDisabled);

	virtual BOOL IsOwnerDrawMenuCheck ()	{	return FALSE;	}

	virtual COLORREF GetToolbarButtonTextColor (CBCGPToolbarButton* pButton,
												CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual BOOL IsToolBarButtonDefaultBackground (CBCGPToolbarButton* pButton,
												CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual COLORREF GetToolbarDisabledColor () const
	{
		return (COLORREF)-1;
	}

	virtual BOOL IsToolbarRoundShape (CBCGPToolBar* /*pToolBar*/)
	{
		return FALSE;
	}

	virtual BOOL IsAutoGrayscaleImages() { return FALSE; }
	virtual double GetGrayScaleLumRatio() { return m_dblGrayScaleLumRatio; }

	virtual COLORREF GetToolbarHighlightColor ();

	virtual COLORREF GetToolbarDisabledTextColor ();
	virtual COLORREF GetToolbarEditPromptColor ();

	virtual int GetDockingBarCaptionExtraHeight () const
	{
		return 0;
	}

	virtual COLORREF GetHighlightedColor(UINT nType);

	virtual void OnDrawDockingBarScrollButton(CDC* pDC, CBCGPDockingBarScrollButton* pButton);

	// Caption bar:
	virtual COLORREF GetCaptionBarTextColor (CBCGPCaptionBar* pBar);
	virtual void OnDrawCaptionBarInfoArea (CDC* pDC, CBCGPCaptionBar* pBar, CRect rect);
	virtual void OnDrawCaptionBarBorder (CDC* pDC, CBCGPCaptionBar* pBar, CRect rect, COLORREF clrBarBorder, BOOL bFlatBorder);
	virtual COLORREF OnFillCaptionBarButton (CDC* pDC, CBCGPCaptionBar* pBar,
											CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
											BOOL bIsDisabled, BOOL bHasDropDownArrow,
											BOOL bIsSysButton);
	virtual void OnDrawCaptionBarButtonBorder (CDC* pDC, CBCGPCaptionBar* pBar,
											CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
											BOOL bIsDisabled, BOOL bHasDropDownArrow,
											BOOL bIsSysButton);
	virtual void OnDrawCaptionBarCloseButton(CDC* pDC, CBCGPCaptionBar* pBar,
			CRect rectClose, BOOL bIsCloseBtnPressed, BOOL bIsCloseBtnHighlighted, COLORREF clrText);
	virtual BOOL UseLargeCaptionFontInDockingCaptions() { return FALSE; }

	virtual int GetMessageBarMargin()				{	return 4;	 }
	virtual BOOL IsDrawMessageBoxButtonsBanner()	{	return TRUE; }

	// Outlook bar:
	virtual int GetOutlookBarSplitterHeight() { return 8; }
	virtual void OnFillOutlookPageButton (CDC* pDC, const CRect& rect,
										BOOL bIsHighlighted, BOOL bIsPressed,
										COLORREF& clrText);
	virtual void OnDrawOutlookPageButtonBorder (CDC* pDC, CRect& rectBtn, BOOL bIsHighlighted, BOOL bIsPressed);
	virtual void OnDrawOutlookBarSplitter (CDC* pDC, CRect rectSplitter);
	virtual void OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, COLORREF& clrText);
	virtual void OnDrawOutlookBarFrame(CDC* pDC, CRect rectFrame);

	// Tab overrides:
	virtual void OnEraseTabsArea (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd);
	virtual void OnDrawTab (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd);
	virtual void OnFillTab (CDC* pDC, CRect rectFill, CBrush* pbrFill, int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd);
	virtual void OnDrawTabContent (CDC* pDC, CRect rectTab,
							int iTab, BOOL bIsActive, const CBCGPBaseTabWnd* pTabWnd,
							COLORREF clrText);

	virtual void OnDrawTabButton (CDC* pDC, CRect rect, 
		const CBCGPBaseTabWnd* pTabWnd, 
		int nID,
		BOOL bIsHighlighted,
		BOOL bIsPressed,
		BOOL bIsDisabled);

	virtual void OnDrawTabCloseButton (CDC* pDC, CRect rect, 
											   const CBCGPBaseTabWnd* pTabWnd, 
											   BOOL bIsHighlighted,
											   BOOL bIsPressed,
											   BOOL bIsDisabled);
	virtual void OnDrawTabPinButton (CDC* pDC, CRect rect, 
												const CBCGPBaseTabWnd* pTabWnd, 
												BOOL bIsPinned,
												BOOL bIsHighlighted,
												BOOL bIsPressed,
												BOOL bIsDisabled);
	virtual void OnEraseTabsButton (CDC* pDC, CRect rect, CBCGPButton* pButton,
									CBCGPBaseTabWnd* pWndTab);
	virtual void OnDrawTabsButtonBorder (CDC* pDC, CRect& rect, 
									CBCGPButton* pButton, UINT uiState, CBCGPBaseTabWnd* pWndTab);
	virtual void OnDrawTabResizeBar (CDC* pDC, CBCGPBaseTabWnd* pWndTab, 
									BOOL bIsVert, CRect rect,
									CBrush* pbrFace, CPen* pPen);
	virtual void OnDrawTabsPointer(CDC* pDC, CBCGPBaseTabWnd* pWndTab, const CRect& rectTabs, int nPointerAreaHeight, const CRect& rectActiveTab);
	virtual COLORREF GetTabPointerColor(CBCGPBaseTabWnd* pWndTab);

	virtual void GetTabFrameColors (const CBCGPBaseTabWnd* pTabWnd,
				   COLORREF& clrDark,
				   COLORREF& clrBlack,
				   COLORREF& clrHighlight,
				   COLORREF& clrFace,
				   COLORREF& clrDarkShadow,
				   COLORREF& clrLight,
				   CBrush*& pbrFace,
				   CBrush*& pbrBlack);
	virtual void OnDrawTabBorder(CDC* pDC, CBCGPTabWnd* pTabWnd, CRect rectBorder, COLORREF clrBorder, CPen& penLine);
	virtual BOOL OnEraseTabsFrame (CDC* pDC, CRect rect, const CBCGPBaseTabWnd* pTabWnd);
	virtual BOOL IsHighlightOneNoteTabs () const	{	return FALSE;	}
	virtual BOOL AlwaysHighlight3DTabs () const		{	return FALSE;	}
	virtual COLORREF GetTabTextColor (const CBCGPBaseTabWnd* /*pTabWnd*/, int /*iTab*/, BOOL /*bIsActive*/)	{	return (COLORREF)-1;	}
	virtual int GetTabHorzMargin (const CBCGPBaseTabWnd* /*pTabWnd*/) { return 0; }
	virtual int GetTabsMargin (const CBCGPTabWnd* /*pTabWnd*/) { return 2; }
	virtual COLORREF GetActiveTabBackColor(const CBCGPBaseTabWnd* /*pTabWnd*/) const { return (COLORREF)-1; }

	virtual int GetMDITabsBordersSize ()		{	return -1;	/* Default */	}
	virtual BOOL IsMDITabsTopEdge ()			{	return TRUE;				}
	virtual int GetDockingTabsBordersSize ()	{	return -1;	/* Default */	}
	virtual BOOL IsDockingTabHasBorder ()		{	return TRUE;				}
	virtual BOOL IsDockingTabUpperCase()		{	return FALSE;				}
	virtual BOOL IsFocusedTabSeparateLook()		{	return FALSE;				}
	virtual BOOL IsTopLevelMenuItemUpperCase()	{	return FALSE;				}
	
	virtual int GetTabExtraHeight(const CBCGPTabWnd* /*pTab*/)	{ return 0;	}

	virtual int GetTabButtonState (CBCGPTabWnd* /*pTab*/, CBCGTabButton* /*pButton*/)	{	return -1;	}

	virtual BOOL IsTabColorBar(CBCGPTabWnd* pTab, int iTab);

	// Customization dialog:
	virtual COLORREF OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected = FALSE);
	virtual void OnDrawMenuArrowOnCustomizeList (CDC* pDC, CRect rectCommand, BOOL bSelected);

	virtual CSize GetButtonExtraBorder () const
	{
		return CSize (0, 0);
	}

	virtual CSize GetCaptionButtonExtraBorder () const
	{
		return CSize (0, 0);
	}

	virtual COLORREF OnFillMiniFrameCaption (CDC* pDC, CRect rectCaption, 
											CBCGPMiniFrameWnd* pFrameWnd,
											BOOL bActive);
	virtual void OnDrawMiniFrameBorder (CDC* pDC, CBCGPMiniFrameWnd* pFrameWnd,
										CRect rectBorder, CRect rectBorderSize);
	virtual void OnDrawFloatingToolbarBorder (	CDC* pDC, CBCGPBaseToolBar* pToolBar, 
												CRect rectBorder, CRect rectBorderSize);
	// Header control:
	virtual COLORREF GetHeaderCtrlTextColor (CBCGPHeaderCtrl* /*pCtrl*/, BOOL /*bIsPressed*/, BOOL /*bIsHighlighted*/)	{	return (COLORREF)-1;	}
	virtual void OnFillHeaderCtrlBackground (CBCGPHeaderCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual void OnDrawHeaderCtrlBorder (CBCGPHeaderCtrl* pCtrl, CDC* pDC,
								CRect& rect, BOOL bIsPressed, BOOL bIsHighlighted);
	virtual void OnDrawHeaderCtrlSortArrow (CBCGPHeaderCtrl* pCtrl, CDC* pDC, CRect& rect, BOOL bIsUp);
	void DoDrawHeaderSortArrow (CDC* pDC, CRect rect, BOOL bIsUp, BOOL bDlgCtrl);

	// List box:
	virtual COLORREF OnFillListBox(CDC* pDC, CBCGPListBox* pListBox, CRect rectClient);
	virtual COLORREF OnFillListBoxItem (CDC* pDC, CBCGPListBox* pListBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected);

	// Combo box:
	virtual COLORREF OnFillComboBoxItem(CDC* pDC, CBCGPComboBox* pComboBox, int nIndex, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected);
	virtual BOOL OnDrawComboBoxText(CDC* /*pDC*/, CBCGPComboBox* /*pComboBox*/) { return FALSE; }

	// List control:
	virtual HBRUSH GetListControlFillBrush(CBCGPListCtrl* pListCtrl);
	virtual COLORREF GetListControlTextColor(CBCGPListCtrl* pListCtrl);
	virtual COLORREF GetListControlMarkedColor(CBCGPListCtrl* pListCtrl);

	// Tree control:
	virtual COLORREF GetTreeControlFillColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected = FALSE, BOOL bIsFocused = FALSE, BOOL bIsDisabled = FALSE);
	virtual COLORREF GetTreeControlLineColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected = FALSE, BOOL bIsFocused = FALSE, BOOL bIsDisabled = FALSE);
	virtual COLORREF GetTreeControlTextColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected = FALSE, BOOL bIsFocused = FALSE, BOOL bIsDisabled = FALSE);

	// Obsolete
	virtual COLORREF OnDrawPropSheetListItem (CDC*, CBCGPPropertySheet*, CRect, BOOL, BOOL) {	return (COLORREF)0;	}

	// Tasks pane:
#ifndef BCGP_EXCLUDE_TASK_PANE
	virtual void OnFillTasksPaneBackground(CDC* pDC, CRect rectWorkArea);

	virtual void OnDrawTasksGroupCaption(CDC* pDC, CBCGPTasksGroup* pGroup, BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE, BOOL bCanCollapse = FALSE);
	virtual void OnDrawTasksGroupIcon(CDC* pDC, CBCGPTasksGroup* pGroup, int nIconHOffset = 5,
		BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE, BOOL bCanCollapse = FALSE);

	virtual void OnFillTasksGroupInterior(CDC* pDC, CRect rect, BOOL bSpecial = FALSE);
	virtual void OnDrawTasksGroupAreaBorder(CDC* pDC, CRect rect, BOOL bSpecial = FALSE, BOOL bNoTitle = FALSE);
	virtual void OnDrawTask(CDC* pDC, CBCGPTask* pTask, CImageList* pIcons, BOOL bIsHighlighted = FALSE, BOOL bIsSelected = FALSE);

	virtual void OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited);
	virtual void OnDrawToolBoxFrame (CDC* pDC, const CRect& rect);
	virtual void OnDrawFocusRect (CDC* pDC, const CRect& rect);
#endif

	// Property Sheet:
	virtual void OnFillPropSheetHeaderArea(CDC* pDC, CBCGPPropertySheet* pPropSheet, CRect rect, BOOL& bDrawBottomLine);

	// Slider
	virtual void OnDrawSlider (CDC* pDC, CBCGPSlider* pSlider, CRect rect, BOOL bAutoHideMode);

	// Property list:
	virtual void OnDrawExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox);
	virtual BOOL IsExpandingBoxTransparent() { return m_bIsTransparentExpandingBox; }

#ifndef BCGP_EXCLUDE_PROP_LIST
	virtual COLORREF OnFillPropList(CDC* pDC, CBCGPPropList* pList, const CRect& rectClient, COLORREF& clrFill);
	virtual void OnFillPropListToolbarArea(CDC* pDC, CBCGPPropList* pList, const CRect& rectToolBar);
	virtual COLORREF OnFillPropertyListSelectedItem(CDC* pDC, CBCGPProp* pProp, CBCGPPropList* pWndList, const CRect& rectFill, BOOL bFocused);
	virtual COLORREF GetPropListGroupColor (CBCGPPropList* pPropList);
	virtual COLORREF GetPropListGroupTextColor (CBCGPPropList* pPropList);
	virtual COLORREF GetPropListDisabledTextColor(CBCGPPropList* pPropList);
	virtual COLORREF GetPropListDesciptionTextColor (CBCGPPropList* /*pPropList*/)	{	return (COLORREF)-1;	}
	virtual COLORREF GetPropListCommandTextColor (CBCGPPropList* pPropList);
	virtual void OnDrawPropListComboButton(CDC* pDC, CRect rect, CBCGPProp* pProp, BOOL bControlBarColors, BOOL bEnabled, BOOL bFocused, BOOL bButtonIsDown, BOOL bButtonIsHighlighted);
	virtual COLORREF OnDrawPropListPushButton(CDC* pDC, CRect rect, CBCGPProp* pProp, BOOL bControlBarColors, BOOL bEnabled, BOOL bFocused, BOOL bButtonIsDown, BOOL bButtonIsHighlighted);
	virtual COLORREF OnFillPropListDescriptionArea(CDC* pDC, CBCGPPropList* pList, const CRect& rect);
	virtual COLORREF OnFillPropListCommandsArea(CDC* pDC, CBCGPPropList* pList, const CRect& rect);
#endif

	// Splitter:
	virtual void OnDrawSplitterBorder (CDC* pDC, CBCGPSplitterWnd* pSplitterWnd, CRect rect);
	virtual void OnDrawSplitterBox (CDC* pDC, CBCGPSplitterWnd* pSplitterWnd, CRect& rect);
	virtual void OnFillSplitterBackground (CDC* pDC, CBCGPSplitterWnd* pSplitterWnd, CRect rect);

	// Date/time controls:
	virtual COLORREF OnFillCalendarBarNavigationRow (CDC* pDC, CRect rect);
	virtual void OnDrawDateTimeDropButton (CDC* pDC, CRect rect, BOOL bDisabled, BOOL bPressed, BOOL bHighlighted, CBCGPDateTimeCtrl* pCtrl);

	virtual void GetCalendarColors (const CBCGPCalendar* pCalendar,
				   CBCGPCalendarColors& colors);

	// Check boxes / radio buttons:
	virtual CSize GetCheckRadioDefaultSize ();

	virtual void OnDrawCheckBox (CDC *pDC, CRect rect, 
										 BOOL bHighlighted, 
										 BOOL bChecked,
										 BOOL bEnabled);

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

	virtual void OnDrawControlBorder (CWnd* pWndCtrl);
	virtual void OnDrawControlBorder (CDC* pDC, CRect rect, CWnd* pWndCtrl, BOOL bDrawOnGlass);

	virtual void OnDrawControlBorderNoTheme (CWnd* pWndCtrl);
	virtual void OnDrawControlBorderNoTheme (CDC* pDC, CRect rect, CWnd* pWndCtrl, BOOL bDrawOnGlass);

	virtual CSize GetPinSize(BOOL bIsPinned);
	virtual void OnDrawPin(CDC* pDC, const CRect& rect, BOOL bIsPinned, BOOL bIsDark, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled);

	// Auto-hide buttons:
	virtual BOOL HasOverlappedAutoHideButtons () const
	{
		return FALSE;
	}

	virtual void OnFillAutoHideButtonBackground (CDC* pDC, CRect rect, CBCGPAutoHideButton* pButton);
	virtual void OnDrawAutoHideButtonBorder (CDC* pDC, CRect rectBounds, CRect rectBorderSize, CBCGPAutoHideButton* pButton);
	virtual COLORREF GetAutoHideButtonTextColor (CBCGPAutoHideButton* pButton);
	virtual COLORREF GetAutoHideButtonBorderColor(CBCGPAutoHideButton* pButton);
	virtual int GetAutohideButtonExtraSpace() { return 0; }

	// Calculator:
	virtual BOOL OnDrawCalculatorButton (CDC* pDC, CRect rect, CBCGPToolbarButton* pButton, CBCGPVisualManager::BCGBUTTON_STATE state, int cmd /* CBCGPCalculator::CalculatorCommands */, CBCGPCalculator* pCalculator);
	virtual BOOL OnDrawCalculatorDisplay (CDC* pDC, CRect rect, const CString& strText, BOOL bMem, CBCGPCalculator* pCalculator);
	virtual COLORREF GetCalculatorButtonTextColor (BOOL bIsUserCommand, int cmd /* CBCGPCalculator::CalculatorCommands */);
	virtual COLORREF GetCalculatorDisplayTextColor() { return (COLORREF)-1; }

	// Edit box:
	virtual BOOL OnDrawBrowseButton (CDC* pDC, CRect rect, CBCGPEdit* pEdit, CBCGPVisualManager::BCGBUTTON_STATE state, COLORREF& clrText);
	virtual void OnDrawEditClearButton(CDC* pDC, CBCGPButton* pButton, CRect rect);
	virtual CBrush& GetEditCtrlBackgroundBrush(CBCGPEdit* pEdit);
	virtual COLORREF GetEditCtrlTextColor(CBCGPEdit* pEdit);

	// Spin control:
	virtual void OnDrawSpinButtons(CDC* pDC, CRect rectSpin, int nState, BOOL bOrientation, CBCGPSpinButtonCtrl* pSpinCtrl);

	// Appbar window:
	virtual void OnDrawAppBarBorder (CDC* pDC, CBCGPAppBarWnd* pAppBarWnd,
									CRect rectBorder, CRect rectBorderSize);

	virtual void OnDrawAppBarCaption (	CDC* pDC, CBCGPAppBarWnd* pAppBarWnd, 
										CRect rectCaption, CString strCaption);

	// Windows XP drawing methods:
	virtual BOOL DrawPushButtonWinXP (CDC* /*pDC*/, CRect /*rect*/, CBCGPButton* /*pButton*/, UINT /*uiState*/)	{	return FALSE;	}
	virtual BOOL DrawComboDropButtonWinXP (CDC* /*pDC*/, CRect /*rect*/,
										BOOL /*bDisabled*/,
										BOOL /*bIsDropped*/,
										BOOL /*bIsHighlighted*/)	{	return FALSE;	}
	virtual BOOL DrawComboBorderWinXP (CDC* /*pDC*/, CRect /*rect*/,
										BOOL /*bDisabled*/,
										BOOL /*bIsDropped*/,
										BOOL /*bIsHighlighted*/)	{	return FALSE;	}

	// Toolbox:
#ifndef BCGP_EXCLUDE_TOOLBOX
	virtual BOOL OnEraseToolBoxButton (CDC* pDC, CRect rect, CBCGPToolBoxButton* pButton);
	virtual BOOL OnDrawToolBoxButtonBorder (CDC* pDC, CRect& rect, CBCGPToolBoxButton* pButton, UINT uiState);
	virtual COLORREF GetToolBoxButtonTextColor (CBCGPToolBoxButton* pButton);
#endif

	// Smart docking marker colors:
	virtual void GetSmartDockingBaseMarkerColors (
		COLORREF& clrBaseGroupBackground,
		COLORREF& clrBaseGroupBorder);

	virtual COLORREF GetSmartDockingMarkerToneColor ();

	virtual BCGP_SMARTDOCK_THEME GetSmartDockingTheme ()
	{
		return BCGP_SDT_VS2005;
	}

	// Popup window:
#ifndef BCGP_EXCLUDE_POPUP_WINDOW
	virtual void OnFillPopupWindowBackground (CDC* pDC, CRect rect);
	virtual void OnDrawPopupWindowBorder (CDC* pDC, CRect rect);
	virtual void OnDrawPopupWindowRoundedBorder (CDC* pDC, CRect rect, CBCGPPopupWindow* pPopupWnd, int nCornerRadius);
	virtual COLORREF OnDrawPopupWindowCaption (CDC* pDC, CRect rectCaption, CBCGPPopupWindow* pPopupWnd);
	virtual COLORREF GetPopupWindowCaptionTextColor(CBCGPPopupWindow* pPopupWnd, BOOL bButton);
	virtual COLORREF GetPopupWindowLinkTextColor(CBCGPPopupWindow* pPopupWnd, BOOL bIsHover);
	virtual void OnErasePopupWindowButton (CDC* pDC, CRect rectClient, CBCGPPopupWndButton* pButton);
	virtual void OnDrawPopupWindowButtonBorder (CDC* pDC, CRect rectClient, CBCGPPopupWndButton* pButton);
	virtual BOOL IsDefaultWinXPPopupButton (CBCGPPopupWndButton* /*pButton*/) const	{	return FALSE;	}
#endif

	// Editor colors:
	virtual COLORREF GetEditCtrlSelectionBkColor(CBCGPEditCtrl* pEdit, BOOL bIsFocused);
	virtual COLORREF GetEditCtrlSelectionTextColor(CBCGPEditCtrl* pEdit, BOOL bIsFocused);
	
	virtual COLORREF GetEditBackSidebarColor(CBCGPEditCtrl* /*pEdit*/)
	{
		return (COLORREF)-1;
	}
	
	virtual HBRUSH GetEditBackgroundBrush(CBCGPEditCtrl* /*pEdit*/)
	{
		return ::GetSysColorBrush (COLOR_WINDOW);
	}

	virtual COLORREF GetEditTextColor(CBCGPEditCtrl* /*pEdit*/)
	{
		return ::GetSysColor(COLOR_WINDOWTEXT);
	}

	virtual COLORREF GetEditLineNumbersBarBackColor(CBCGPEditCtrl* /*pEdit*/)
	{
		return (COLORREF)-1;
	}

	virtual COLORREF GetEditLineNumbersBarTextColor(CBCGPEditCtrl* /*pEdit*/)
	{
		return (COLORREF)-1;
	}

	virtual COLORREF GetEditOutlineColor(CBCGPEditCtrl* /*pEdit*/)
	{
		return RGB(128, 128, 128);
	}

	virtual BOOL IsDrawEditLineNumbersRightSideLine(CBCGPEditCtrl* /*pEdit*/)
	{
		return TRUE;
	}

	virtual void OnDrawEditCtrlResizeBox (CDC* pDC, CBCGPEditCtrl* /*pEdit*/, CRect rect);

	// Day planner:
#ifndef BCGP_EXCLUDE_PLANNER
	virtual COLORREF OnFillPlannerCaption (CDC* pDC, CBCGPPlannerView* pView,
		CRect rect, BOOL bIsToday, BOOL bIsSelected, BOOL bNoBorder = FALSE, BOOL bHorz = TRUE);

	virtual void OnDrawPlannerCaptionText (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect, const CString& strText, 
		COLORREF clrText, int nAlign, BOOL bHighlight);

	virtual void OnFillPlanner (CDC* pDC, CBCGPPlannerView* pView, 
		CRect rect, BOOL bWorkingArea);

	virtual void GetPlannerAppointmentColors (CBCGPPlannerView* pView,
		BOOL bSelected, BOOL bSimple, DWORD dwDrawFlags, 
		COLORREF& clrBack1, COLORREF& clrBack2,
		COLORREF& clrFrame1, COLORREF& clrFrame2, COLORREF& clrText);

	virtual COLORREF GetPlannerAppointmentTimeColor (CBCGPPlannerView* pView,
		BOOL bSelected, BOOL bSimple, DWORD dwDrawFlags);

	virtual COLORREF GetPlannerHourLineColor (CBCGPPlannerView* pView,
		BOOL bWorkingHours, BOOL bHour);

	virtual COLORREF OnFillPlannerTimeBar (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect, COLORREF& clrLine);

	virtual void OnDrawPlannerTimeLine (CDC* pDC, CBCGPPlannerView* pView, CRect rect);

	virtual void OnFillPlannerWeekBar (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect);

	virtual DWORD GetPlannerDrawFlags () const
	{
		return 0;
	}

	virtual int GetPlannerRowExtraHeight () const
	{
		return 0;
	}

	virtual COLORREF GetPlannerWorkColor () const
	{
		return m_clrPlannerWork;
	}

	virtual void OnDrawPlannerHeader (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect);

	virtual void OnDrawPlannerHeaderPane (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect);

	virtual void OnFillPlannerHeaderAllDay (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect);
	virtual void OnDrawPlannerHeaderAllDayItem (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect, BOOL bIsToday, BOOL bIsSelected);

	virtual COLORREF GetPlannerSelectionColor (CBCGPPlannerView* pView);
	virtual COLORREF GetPlannerSeparatorColor (CBCGPPlannerView* pView);

	virtual void PreparePlannerBackItem (BOOL bIsToday, BOOL bIsSelected);
	virtual void PreparePlannerCaptionBackItem (BOOL bIsHeader);
	
#endif // BCGP_EXCLUDE_PLANNER

	// Grid control:
#ifndef BCGP_EXCLUDE_GRID_CTRL
	virtual void OnFillGridBackground (CDC* pDC, CRect rect);
	virtual void OnDrawGridExpandingBox (CDC* pDC, CRect rect, BOOL bIsOpened, COLORREF colorBox);
	virtual void OnFillGridHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual BOOL OnDrawGridHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnFillGridRowHeaderBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual BOOL OnDrawGridRowHeaderItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnFillGridSelectAllAreaBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnDrawGridSelectAllMarker(CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, int nPadding, BOOL bPressed);

	virtual BOOL OnDrawGridSelectAllAreaBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, BOOL bPressed);
	virtual void OnDrawGridSortArrow (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectArrow, BOOL bIsUp);
	virtual COLORREF OnFillGridGroupByBoxBackground (CDC* pDC, CRect rect);
	virtual COLORREF OnFillGridGroupByBoxTitleBackground (CDC* pDC, CRect rect);
	virtual COLORREF GetGridGroupByBoxLineColor () const;
	virtual void OnDrawGridGroupByBoxItemBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual COLORREF GetGridLeftOffsetColor (CBCGPGridCtrl* pCtrl);
	virtual COLORREF GetGridItemSortedColor (CBCGPGridCtrl* pCtrl);
	virtual void OnFillGridGroupBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill);
	virtual BOOL IsGridGroupUnderline () const { return TRUE; }
	virtual void OnDrawGridGroupUnderline (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill);
	virtual COLORREF OnFillGridRowBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill, BOOL bSelected);
	virtual COLORREF OnFillGridItem (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill,
		BOOL bSelected, BOOL bActiveItem, BOOL bSortedColumn);
	virtual void OnDrawGridHeaderMenuButton (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect, 
		BOOL bHighlighted, BOOL bPressed, BOOL bDisabled);
	virtual void OnDrawGridSelectionBorder (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual COLORREF OnFillGridCaptionRow (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill);
	virtual CBrush& GetGridCaptionBrush(CBCGPGridCtrl* pCtrl);
	virtual void OnDrawGridDataBar (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual void GetGridColorScaleBaseColors(CBCGPGridCtrl* pCtrl, COLORREF& clrLow, COLORREF& clrMid, COLORREF& clrHigh);

	virtual BOOL OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme);

	virtual COLORREF OnDrawGridColumnChooserItem(CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectItem, BOOL bIsSelected);
	virtual BOOL IsGridSparklineDefaultSelColor() const
	{
		return FALSE;
	}

	// Report control:
	virtual COLORREF GetReportCtrlGroupBackgoundColor ();
	virtual COLORREF OnFillReportCtrlRowBackground (CBCGPGridCtrl* pCtrl, CDC* pDC,
		CRect rectFill, BOOL bSelected, BOOL bGroup);
#endif // BCGP_EXCLUDE_GRID_CTRL

	// Gantt control:
#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)
	virtual void GetGanttColors (const CBCGPGanttChart* pChart, BCGP_GANTT_CHART_COLORS& colors, COLORREF clrBack = CLR_DEFAULT) const;
	virtual void DrawGanttChartBackground (const CBCGPGanttChart* pChart, CDC& dc, const CRect& rectChart, COLORREF clrFill);
	virtual void DrawGanttItemBackgroundCell (const CBCGPGanttChart* pChart, CDC& dc, const CRect& rectItem, const CRect& rectClip, const BCGP_GANTT_CHART_COLORS& colors, BOOL bDayOff);
	virtual void DrawGanttHeaderCell (const CBCGPGanttChart* pChart, CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, BOOL bHilite);
	virtual void DrawGanttHeaderText (const CBCGPGanttChart* pChart, CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, const CString& sCellText, BOOL bHilite) const;
	virtual COLORREF GetGanttHeaderTextColor (BOOL bHilite) const;

	virtual void FillGanttBar (const CBCGPGanttItem* pItem, CDC& dc, const CRect& rectFill, COLORREF color, double dGlowLine);

#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

	// Breadcrumb control:
	virtual COLORREF BreadcrumbFillBackground (CDC& dc, CBCGPBreadcrumb* pControl, CRect rectFill);
	virtual void BreadcrumbDrawItemBackground (CDC& dc, CBCGPBreadcrumb* pControl, BREADCRUMBITEMINFO* pItemInfo, CRect rectItem, UINT uState, COLORREF color);
	virtual void BreadcrumbDrawItem (CDC& dc, CBCGPBreadcrumb* pControl, BREADCRUMBITEMINFO* pItemInfo, CRect rectItem, UINT uState, COLORREF color);
	virtual void BreadcrumbDrawArrowBackground (CDC& dc, CBCGPBreadcrumb* pControl, BREADCRUMBITEMINFO* pItemInfo, CRect rectArrow, UINT uState, COLORREF color);
	virtual void BreadcrumbDrawArrow (CDC& dc, CBCGPBreadcrumb* pControl, BREADCRUMBITEMINFO* pItemInfo, CRect rect, UINT uState, COLORREF color);
	virtual void BreadcrumbDrawLeftArrowBackground (CDC& dc, CBCGPBreadcrumb* pControl, CRect rect, UINT uState, COLORREF color);
	virtual void BreadcrumbDrawLeftArrow (CDC& dc, CBCGPBreadcrumb* pControl, CRect rect, UINT uState, COLORREF color);


	// Window apperance:
	virtual BOOL IsOwnerDrawCaption ()	{	return FALSE;	}
	virtual BOOL OnSetWindowRegion (CWnd* pWnd, CSize sizeWindow);
	virtual void OnActivateApp(CWnd* pWnd, BOOL bActive);
	virtual BOOL OnNcPaint (CWnd* pWnd, const CObList& lstSysButtons, CRect rectRedraw);
	virtual BOOL OnNcActivate (CWnd* pWnd, BOOL bActive);
	virtual CSize GetNcBtnSize (BOOL bSmall) const;
    virtual BOOL OnUpdateNcButtons (CWnd* pWnd, const CObList& lstSysButtons, CRect rectCaption);

	// Dialog:
	virtual BOOL OnFillDialog (CDC* pDC, CWnd* pDlg, CRect rect);
	virtual CBrush& GetDlgBackBrush (CWnd* pDlg);
	virtual void OnDrawDlgSizeBox (CDC* pDC, CWnd* pDlg, CRect rectSizeBox);
	virtual void OnFillRibbonBackstageForm(CDC* pDC, CWnd* pDlg, CRect rect);
	virtual void OnDrawButtonsArea(CDC* pDC, CWnd* pDlg, CRect rect);
	virtual CBrush& GetDlgButtonsAreaBrush(CWnd* pDlg, COLORREF* pclrLine = NULL);

	virtual CFont& GetNcCaptionTextFont();
	virtual COLORREF GetNcCaptionTextColor(BOOL bActive, BOOL bTitle = TRUE) const;

#ifndef BCGP_EXCLUDE_RIBBON
	// Ribbon control:
	virtual BOOL IsRibbonScenicLook()		{	return FALSE;	}
	virtual BOOL IsRibbonBackgroundImage()	{	return FALSE;	}

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

	virtual COLORREF OnFillRibbonPanelCaption (
					CDC* pDC,
					CBCGPRibbonPanel* pPanel, 
					CRect rectCaption);

	virtual void OnDrawRibbonLaunchButton (
					CDC* pDC,
					CBCGPRibbonLaunchButton* pButton,
					CBCGPRibbonPanel* pPanel);

	virtual void OnDrawRibbonDefaultPaneButton (
					CDC* pDC,
					CBCGPRibbonButton* pButton);

	virtual void OnDrawRibbonDefaultPaneButtonContext (
					CDC* pDC,
					CBCGPRibbonButton* pButton);

	virtual void OnDrawRibbonDefaultPaneButtonIndicator (
					CDC* pDC, 
					CBCGPRibbonButton* pButton,
					CRect rect, 
					BOOL bIsSelected, 
					BOOL bHighlighted);

	virtual COLORREF OnFillRibbonButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);

	virtual COLORREF OnFillRibbonPinnedButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton,
					BOOL& bIsDarkPin);

	virtual void OnFillRibbonEdit (
					CDC*, 
					CBCGPRibbonEditCtrl*,
					CRect,
					BOOL,
					BOOL,
					BOOL,
					COLORREF&, COLORREF&, COLORREF&) { /* Obsolete */}

	virtual COLORREF GetRibbonEditBackgroundColor (
					CBCGPRibbonEditCtrl* pEdit,
					BOOL bIsHighlighted,
					BOOL bIsPaneHighlighted,
					BOOL bIsDisabled);

	virtual COLORREF GetRibbonEditTextColor (
					CBCGPRibbonEditCtrl* pEdit,
					BOOL bIsHighlighted,
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

	virtual COLORREF OnFillRibbonMainPanelButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);

	virtual void OnDrawRibbonMainPanelButtonBorder (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);

	virtual void OnDrawRibbonCaption (
					CDC* pDC, CBCGPRibbonBar* pBar, CRect rect,
					CRect rectText);

	virtual int GetRibbonQATChevronOffset ()	{	return 0;	}

	virtual int GetRibbonQATRightMargin ()		{	return 0;	}

	virtual COLORREF GetRibbonQATTextColor (BOOL bDisabled = FALSE)
	{
		return bDisabled ? GetToolbarDisabledTextColor () : (COLORREF)-1;
	}

	virtual COLORREF GetRibbonBarTextColor (BOOL bDisabled = FALSE)
	{
		return bDisabled ? GetToolbarDisabledTextColor () : (COLORREF)-1;
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

	virtual void OnDrawRibbonLabel (
					CDC* pDC, 
					CBCGPRibbonLabel* pLabel,
					CRect rect);

	virtual void OnDrawRibbonPaletteButton (
					CDC* pDC,
					CBCGPRibbonPaletteIcon* pButton);

	virtual void OnDrawRibbonPaletteButtonIcon(
					CDC* pDC,
					CBCGPRibbonPaletteIcon* pButton,
					int nID,
					CRect rect);

	virtual void OnDrawRibbonPaletteBorder (
					CDC* pDC, 
					CBCGPRibbonPaletteButton* pButton, 
					CRect rectBorder);

	virtual COLORREF RibbonCategoryColorToRGB (BCGPRibbonCategoryColor color);

	virtual COLORREF OnDrawRibbonCategoryCaption (
					CDC* pDC, 
					CBCGPRibbonContextCaption* pContextCaption);

	virtual void OnDrawRibbonCategoryCaptionText(
					CDC* pDC, CBCGPRibbonContextCaption* pContextCaption, 
					CString& strText, CRect rectText, BOOL bIsOnGlass, BOOL bIsZoomed);

	virtual COLORREF OnDrawRibbonStatusBarPane (CDC* pDC, CBCGPRibbonStatusBar* pBar,
					CBCGPRibbonStatusBarPane* pPane);

	virtual void GetRibbonSliderColors (CBCGPRibbonSlider* pSlider, 
					BOOL bIsHighlighted, 
					BOOL bIsPressed,
					BOOL bIsDisabled,
					COLORREF& clrLine,
					COLORREF& clrFill);

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

	virtual void OnFillRibbonPanelMenuBarInCtrlMode(CDC* pDC, CBCGPRibbonPanelMenuBar* pBar, CRect rectClient);

	virtual int GetRibbonPopupBorderSize (const CBCGPRibbonPanelMenu* /*pPopup*/) const
	{
		return m_nMenuBorderSize;
	}

	virtual void OnDrawRibbonQATSeparator (CDC* pDC, CBCGPRibbonSeparator* pSeparator, CRect rect);

	virtual BOOL IsLayeredRibbonKeyTip () const	{	return FALSE;	}
	virtual void OnDrawRibbonKeyTip (CDC* pDC, CBCGPBaseRibbonElement* pElement, CRect rect, CString str);

	virtual void OnDrawRibbonCheckBoxOnList (CDC* pDC, CBCGPRibbonButton* pButton, CRect rect, BOOL bIsSelected, BOOL bHighlighted);
	virtual void OnDrawRibbonRadioButtonOnList (CDC* pDC, CBCGPRibbonButton* pButton, CRect rect, BOOL bIsSelected, BOOL bHighlighted);

	virtual COLORREF GetRibbonHyperlinkTextColor (CBCGPRibbonHyperlink* pHyperLink);
	virtual COLORREF GetRibbonStatusBarTextColor (CBCGPRibbonStatusBar* pStatusBar);

	virtual void OnDrawRibbonColorPaletteBox (CDC* pDC, CBCGPRibbonColorButton* pColorButton,
		CBCGPRibbonPaletteIcon* pIcon,
		COLORREF color, CRect rect, BOOL bDrawTopEdge, BOOL bDrawBottomEdge,
		BOOL bIsHighlighted, BOOL bIsChecked, BOOL bIsDisabled);

	const CPoint& GetRibbonMainImageOffset () const	{	return m_ptRibbonMainImageOffset;	}

	virtual int GetRibbonPanelMargin(CBCGPRibbonCategory* pCategory);
	virtual int GetRibbonPanelMarginTop() const {	return 3;	}

	virtual BOOL DoesRibbonExtendCaptionToTabsArea()	{	return FALSE;	}

	virtual BOOL IsRibbonTabsAreaDark()					{	return FALSE;	}

	virtual void OnDrawRibbonMinimizeButtonImage(CDC* pDC, CBCGPRibbonMinimizeButton* pButton, BOOL bRibbonIsMinimized);
	virtual CSize GetRibbonMinimizeButtonImageSize();

	virtual int GetRibbonButtonsGroupHorzMargin()		{	return 0;		}
	virtual BOOL IsDrawRibbonGroupSeparator()			{	return FALSE;	}
	virtual void OnDrawRibbonGroupSeparator (CDC* /*pDC*/, CRect /*rectSeparator*/) {}

	virtual int GetRibbonBackstageTopLineHeight()		{	return 1;		}
	virtual void OnDrawRibbonBackstageTopLine(CDC* pDC, CRect rectLine);

	virtual BOOL IsRibbonBackstageHideTabs()			{	return FALSE;	}
	virtual COLORREF GetRibbonBackstageTextColor();
	virtual BOOL IsRibbonBackstageWhiteBackground()		{	return TRUE;	}
	virtual CFont* GetBackstageViewEntryFont()			{	return NULL;	}

	virtual void SetMainButtonColor(COLORREF clr);	// -1 - default (blue)
	COLORREF GetMainButtonColor() const	{	return m_clrMainButton;	}

	virtual void OnPreDrawRibbon (CDC* /*pDC*/, CBCGPRibbonBar* /*pRibbonBar*/, CRect /*rectTabs*/)	{}

	virtual void OnDrawRibbonMenuArrow(CDC* pDC, CBCGPRibbonButton* pButton, UINT id, const CRect& rectMenuArrow);

	virtual void OnDrawRibbonBackStageTopMenu(CDC* /*pDC*/, CRect /*rectBackstageTopArea*/) {}

#endif

	// Vista support:
	virtual BOOL DrawTextOnGlass (CDC* pDC, CString strText, CRect rect, DWORD dwFlags, int nGlowSize = 0,
		COLORREF clrText = (COLORREF)-1);

	// MDI Client area
	virtual BOOL OnEraseMDIClientArea (CDC* pDC, CRect rectClient);

	// ToolTip
	virtual BOOL GetToolTipParams (CBCGPToolTipParams& params, UINT nType = (UINT)(-1));
	virtual void OnFillToolTip (CDC* pDC, CBCGPToolTipCtrl* pToolTip, CRect rect, COLORREF& clrText, COLORREF& clrLine);
	virtual CSize GetSystemToolTipCornerRadius(CToolTipCtrl* pToolTip);

	// Scrollbar
	virtual BOOL IsOwnerDrawScrollBar () const
	{
		return FALSE;
	}

	virtual void OnScrollBarDrawThumb (CDC* /*pDC*/, CBCGPScrollBar* /*pScrollBar*/, CRect /*rect*/, 
		BOOL /*bHorz*/, BOOL /*bHighlighted*/, BOOL /*bPressed*/, BOOL /*bDisabled*/)
	{
		ASSERT (FALSE);
	}
					
	virtual void OnScrollBarDrawButton (CDC* /*pDC*/, CBCGPScrollBar* /*pScrollBar*/, CRect /*rect*/, 
		BOOL /*bHorz*/, BOOL /*bHighlighted*/, BOOL /*bPressed*/, BOOL /*bFirst*/, BOOL /*bDisabled*/)
	{
		ASSERT (FALSE);
	}

	virtual void OnScrollBarFillBackground (CDC* /*pDC*/, CBCGPScrollBar* /*pScrollBar*/, CRect /*rect*/, 
		BOOL /*bHorz*/, BOOL /*bHighlighted*/, BOOL /*bPressed*/, BOOL /*bFirst*/, BOOL /*bDisabled*/)
	{
		ASSERT (FALSE);
	}

	// Push button:
	virtual BOOL OnDrawPushButton (CDC* /*pDC*/, CRect /*rect*/, CBCGPButton* /*pButton*/, COLORREF& /*clrText*/)	{	return FALSE;	}
	virtual BOOL IsDrawFocusRectOnPushButton(CBCGPButton* /*pButton*/) 	{ return TRUE; }
	virtual COLORREF GetURLLinkColor (CBCGPURLLinkButton* pButton, BOOL bHover);

	// Group:
	virtual void OnDrawGroup (CDC* pDC, CBCGPGroup* pGroup, CRect rect, const CString& strName);

	// Separator:
	virtual BOOL IsOwnerDrawDlgSeparator(CBCGPStatic* /*pCtrl*/)	{	return FALSE;	}
	virtual void OnDrawDlgSeparator(CDC* /*pDC*/, CBCGPStatic* /*pCtrl*/, CRect /*rect*/, BOOL /*bIsHorz*/)	{}

	// Slider:
	virtual void OnDrawSliderChannel (CDC* pDC, CBCGPSliderCtrl* pSlider, BOOL bVert, CRect rect, BOOL bDrawOnGlass);

	virtual void OnDrawSliderThumb (CDC* pDC, CBCGPSliderCtrl* pSlider, 
			CRect rect, BOOL bIsHighlighted, BOOL bIsPressed, BOOL bIsDisabled,
			BOOL bVert, BOOL bLeftTop, BOOL bRightBottom,
			BOOL bDrawOnGlass);

	virtual void OnDrawSliderTic(CDC* pDC, CBCGPSliderCtrl* pSlider, CRect rectTic, BOOL bVert, BOOL bLeftTop, BOOL bDrawOnGlass);

	// Explorer bar:
	virtual UINT GetNavButtonsID(BOOL bIsLarge);
	virtual UINT GetNavFrameID(BOOL bIsLarge);

// Attributes:
public:

	void SetMenuShadowDepth (int nDepth)	// Pixels
	{
		m_nMenuShadowDepth = nDepth;
	}

	int GetMenuShadowDepth () const
	{
		return m_nMenuShadowDepth;
	}
	
	static CBCGPVisualManager* GetInstance ()
	{
		if (m_pVisManager != NULL)
		{
			ASSERT_VALID (m_pVisManager);
			return m_pVisManager;
		}

		if (m_pRTIDefault == NULL)
		{
			m_pVisManager = new CBCGPVisualManager;
		}
		else
		{
			m_pVisManager = (CBCGPVisualManager*) m_pRTIDefault->CreateObject ();
			ASSERT_VALID (m_pVisManager);
		}

		m_pVisManager->m_bAutoDestroy = TRUE;
		m_pVisManager->OnUpdateSystemColors ();

		return m_pVisManager;
	}

	BOOL IsLook2000 () const			{	return m_bLook2000; }
	void SetLook2000 (BOOL bLook2000 = TRUE);

	BOOL IsMenuFlatLook () const			{	return m_bMenuFlatLook; }
	void SetMenuFlatLook (BOOL bMenuFlatLook = TRUE)
	{
		m_bMenuFlatLook = bMenuFlatLook;
		RedrawAll ();
	}

	BOOL IsAutoDestroy () const
	{
		return m_bAutoDestroy;
	}

	void SetShadowHighlightedImage (BOOL bShadow = TRUE)
	{
		m_bShadowHighlightedImage = bShadow;
	}

	BOOL IsShadowHighlightedImage () const
	{
		return m_bShadowHighlightedImage;
	}

	void EnableToolbarButtonFill (BOOL bEnable = TRUE)
	{
		m_bEnableToolbarButtonFill = bEnable;
	}

	BOOL IsToolbarButtonFillEnabled () const
	{
		return m_bEnableToolbarButtonFill;
	}

	BOOL IsEmbossDisabledImage () const
	{
		return m_bEmbossDisabledImage;
	}

	void SetEmbossDisabledImage (BOOL bEmboss = TRUE)
	{
		m_bEmbossDisabledImage = bEmboss;
	}

	BOOL IsFadeInactiveImage () const
	{
		return m_bFadeInactiveImage;
	}

	void SetFadeInactiveImage (BOOL bFade = TRUE)
	{
		m_bFadeInactiveImage = bFade;
	}

	virtual int GetMenuImageMargin () const
	{
		return 2;
	}

	virtual COLORREF GetMenuShadowBaseColor ()
	{
		return m_clrMenuShadowBase;
	}

	virtual COLORREF GetFrameShadowColor(BOOL bActive)
	{
		return bActive ? m_clrFrameShadowBaseActive : m_clrFrameShadowBaseInactive;
	}

	virtual CRect GetMenuImageFrameOffset () const
	{
		return CRect (2, 1, 0, 2);
	}

	virtual int GetPopupMenuGap () const
	{
		return 1;
	}

	virtual BOOL IsLook2000Allowed () const
	// Allows choose "Look 2000" in the customization dialog
	{
		return TRUE;
	}

	// TasksPane:
	int GetTasksPaneVertMargin() const
	{
		return m_nVertMargin;
	}

	int GetTasksPaneHorzMargin() const
	{
		return m_nHorzMargin;
	}

	int GetTasksPaneGroupVertOffset() const
	{
		return m_nGroupVertOffset;
	}

	int GetTasksPaneGroupCaptionHeight() const
	{
		return m_nGroupCaptionHeight;
	}

	int GetTasksPaneGroupCaptionHorzOffset() const
	{
		return m_nGroupCaptionHorzOffset;
	}

	int GetTasksPaneGroupCaptionVertOffset() const
	{
		return m_nGroupCaptionVertOffset;
	}
	
	int GetTasksPaneTaskMinHeight() const
	{
		return m_nTasksMinHeight;
	}

	int GetTasksPaneTasksVertMargin() const
	{
		return m_nTasksVertMargin;
	}

	int GetTasksPaneTaskHorzOffset() const
	{
		return m_nTasksHorzOffset;
	}

	int GetTasksPaneIconHorzOffset() const
	{
		return m_nTasksIconHorzOffset;
	}

	int GetTasksPaneIconVertOffset() const
	{
		return m_nTasksIconVertOffset;
	}

	virtual int GetToolBarCustomizeButtonMargin () const
	{
		return 2;
	}

	virtual BOOL IsOffsetPressedButton () const
	{
		return TRUE;
	}

	virtual BOOL IsOfficeXPStyleMenus () const
	{
		return m_bOfficeXPStyleMenus;
	}

	virtual BOOL GetPopupMenuBorderSize () const
	{
		return m_nMenuBorderSize;
	}

	BOOL IsFrameMenuCheckedItems () const
	{
		return m_bFrameMenuCheckedItems;
	}

	// "Show all menu items" methods:
	virtual int GetShowAllMenuItemsHeight (CDC* pDC, const CSize& sizeDefault);
	virtual void OnDrawShowAllMenuItems (CDC* pDC, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual BOOL IsEmbossDisabledBtnText ()
	{
		return TRUE;
	}

#ifndef BCGP_EXCLUDE_RIBBON
	BOOL IsRibbonPresent (CWnd* pWnd) const;
	CBCGPRibbonBar*	GetRibbonBar (CWnd* pWnd) const;
#endif
	
protected:
	static CBCGPVisualManager* CreateVisualManager (CRuntimeClass* pVisualManager);

// Attributes:
protected:
	static CRuntimeClass*		m_pRTIDefault;
	static CBCGPVisualManager*	m_pVisManager;

	BOOL	m_bLook2000;				// Single grippers
	int		m_nMenuShadowDepth;
	BOOL	m_bMenuFlatLook;			// Menu item is always stil unpressed
	BOOL	m_bShadowHighlightedImage;
	BOOL	m_bEmbossDisabledImage;
	BOOL	m_bFadeInactiveImage;
	BOOL	m_bEnableToolbarButtonFill;
	BOOL	m_bFrameMenuCheckedItems;

	BOOL	m_bIsTemporary;

	int		m_nVertMargin;
	int		m_nHorzMargin;
	int		m_nGroupVertOffset;
	int		m_nGroupCaptionHeight;
	int		m_nGroupCaptionHorzOffset;
	int		m_nGroupCaptionVertOffset;
	int		m_nTasksMinHeight;
	int		m_nTasksVertMargin;
	int		m_nTasksHorzOffset;
	int		m_nTasksIconHorzOffset;
	int		m_nTasksIconVertOffset;
	BOOL	m_bActiveCaptions;

	COLORREF	m_clrReportGroupText;

	BOOL	m_bOfficeXPStyleMenus;
	int		m_nMenuBorderSize;

	BOOL	m_bAlwaysFillTab;
	BOOL	m_b3DTabWideBorder;
	BOOL	m_bIsTransparentExpandingBox;

	COLORREF	m_clrMenuShadowBase;
	COLORREF	m_clrFrameShadowBaseActive;
	COLORREF	m_clrFrameShadowBaseInactive;
	COLORREF	m_clrPlannerWork;
	COLORREF	m_clrPalennerLine;

	CBrush		m_brPlanner;
	CBrush		m_brDlgButtonsArea;

	BOOL		m_bPlannerBackItemToday;
	BOOL		m_bPlannerBackItemSelected;
	BOOL		m_bPlannerCaptionBackItemHeader;

	CPoint		m_ptRibbonMainImageOffset;

	COLORREF	m_clrEditCtrlSelectionBkActive;
	COLORREF	m_clrEditCtrlSelectionTextActive;
	COLORREF	m_clrEditCtrlSelectionBkInactive;
	COLORREF	m_clrEditCtrlSelectionTextInactive;

	COLORREF	m_clrMainButton;	// Ribbon application button color; used in some visual managers
	BOOL		m_bIsRectangulareRibbonTab;

	int			m_nCurrDrawedTab;

	double		m_dblGrayScaleLumRatio;

	CSize	GetSystemBorders (BOOL bRibbonPresent) const;
	
	CMap<HWND, HWND, BOOL, BOOL> m_ActivateFlag;

private:
	BOOL	m_bAutoDestroy;
};

BCGCBPRODLLEXPORT extern UINT BCGM_CHANGEVISUALMANAGER;

#endif // !defined(AFX_BCGPVISUALMANAGER_H__8070C021_60CE_11D5_A304_8B7130518C10__INCLUDED_)
