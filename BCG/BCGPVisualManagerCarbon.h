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
// BCGPVisualManagerCarbon.h: interface for the CBCGPVisualManagerCarbon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPVISUALMANAGERCARBON_H__187C5819_4006_48EB_8C60_3F5529F47B2F__INCLUDED_)
#define AFX_BCGPVISUALMANAGERCARBON_H__187C5819_4006_48EB_8C60_3F5529F47B2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPVisualManager2007.h"
#include "BCGPToolBarImages.h"
#include "BCGPControlRenderer.h"
#include "BCGPToolTipCtrl.h"
#include "bcgglobals.h"

class BCGCBPRODLLEXPORT CBCGPVisualManagerCarbon : public CBCGPVisualManager2007
{
	DECLARE_DYNCREATE(CBCGPVisualManagerCarbon)

public:
	struct XLinkImages
	{
		CBCGPToolBarImages* bmpSrc;
		CBCGPToolBarImages* bmpDst;

		XLinkImages (CBCGPToolBarImages* bmpS = NULL, CBCGPToolBarImages* bmpD = NULL)
			: bmpSrc (bmpS)
			, bmpDst (bmpD)
		{
		}
	};
	typedef CArray<XLinkImages, XLinkImages> XLinkImagesArray;

	struct XLinkColors
	{
		COLORREF* clrSrc;
		COLORREF* clrDst;

		XLinkColors (COLORREF* clrS = NULL, COLORREF* clrD = NULL)
			: clrSrc (clrS)
			, clrDst (clrD)
		{
		}
	};
	typedef CArray<XLinkColors, XLinkColors> XLinkColorsArray;

public:
	CBCGPVisualManagerCarbon();
	virtual ~CBCGPVisualManagerCarbon();

public:
	static BOOL SetStyle (LPCTSTR lpszPath = NULL);
	static CString GetStyleResourceID ();

	virtual void UpdateColors (double h, double s);
	virtual void UpdateLinked ();

	virtual void OnUpdateSystemColors ();
	virtual BOOL OnUpdateNcButtons (CWnd* pWnd, const CObList& lstSysButtons, CRect rectCaption);

public:

	virtual BOOL OnNcPaint (CWnd* pWnd, const CObList& lstSysButtons, CRect rectRedraw);
	virtual BOOL OnSetWindowRegion (CWnd* pWnd, CSize sizeWindow);

	virtual void OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);

	virtual COLORREF OnFillMiniFrameCaption (CDC* pDC, CRect rectCaption, 
											CBCGPMiniFrameWnd* pFrameWnd,
											BOOL bActive);
	virtual void OnDrawMiniFrameBorder (CDC* pDC, CBCGPMiniFrameWnd* pFrameWnd,
										CRect rectBorder, CRect rectBorderSize);
	virtual void OnDrawFloatingToolbarBorder (	CDC* pDC, CBCGPBaseToolBar* pToolBar, 
												CRect rectBorder, CRect rectBorderSize);
	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPPopupMenu* pMenu, CRect rect);
	virtual int GetMenuDownArrowState (CBCGPToolbarMenuButton* pButton, BOOL bHightlight, BOOL bPressed, BOOL bDisabled);
	virtual void OnHighlightMenuItem (CDC *pDC, CBCGPToolbarMenuButton* pButton, CRect rect, COLORREF& clrText);
	virtual void OnDrawCaptionButton (CDC* pDC, CBCGPCaptionButton* pButton, BOOL bActive, BOOL bHorz,
										BOOL bMaximized, BOOL bDisabled, int nImageID = -1);

	virtual COLORREF OnDrawMenuLabel (CDC* pDC, CRect rect);

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
	virtual BOOL AlwaysHighlight3DTabs () const		{	return CBCGPVisualManager2003::AlwaysHighlight3DTabs ();	}
	virtual COLORREF GetTabTextColor (const CBCGPBaseTabWnd* pTabWnd, int iTab, BOOL bIsActive);
	virtual int GetTabHorzMargin (const CBCGPBaseTabWnd* pTabWnd);
	virtual int GetTabButtonState (CBCGPTabWnd* pTab, CBCGTabButton* pButton);

	// Slider
	virtual void OnDrawSlider (CDC* pDC, CBCGPSlider* pSlider, CRect rect, BOOL bAutoHideMode);
	virtual void OnDrawSliderTic(CDC* pDC, CBCGPSliderCtrl* pSlider, CRect rectTic, BOOL bVert, BOOL bLeftTop, BOOL bDrawOnGlass);

	virtual void OnDrawOutlookPageButtonBorder (CDC* pDC, CRect& rectBtn, 
										BOOL bIsHighlighted, BOOL bIsPressed);

	virtual COLORREF GetURLLinkColor (CBCGPURLLinkButton* pButton, BOOL bHover);

	virtual COLORREF GetToolbarButtonTextColor (CBCGPToolbarButton* pButton,
												CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawShowAllMenuItems (CDC* pDC, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);
	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);
	virtual void OnDrawTearOffCaption (CDC* pDC, CRect rect, BOOL bIsActive);
	virtual void OnDrawMenuScrollButton (CDC* pDC, CRect rect, BOOL bIsScrollDown, 
										BOOL bIsHighlited, BOOL bIsPressed, BOOL bIsDisabled);
	virtual void OnDrawMenuSystemButton (CDC* pDC, CRect rect, UINT uiSystemCommand, 
										UINT nStyle, BOOL bHighlight);

	virtual void OnDrawControlBorder (CDC* pDC, CRect rect, CWnd* pWndCtrl, BOOL bDrawOnGlass);

#ifndef BCGP_EXCLUDE_PROP_LIST
	virtual COLORREF OnFillPropList(CDC* pDC, CBCGPPropList* pList, const CRect& rectClient, COLORREF& clrFill);
	virtual COLORREF OnFillPropListDescriptionArea(CDC* pDC, CBCGPPropList* pList, const CRect& rect);
	virtual COLORREF OnFillPropListCommandsArea(CDC* pDC, CBCGPPropList* pList, const CRect& rect);
	virtual COLORREF GetPropListGroupColor (CBCGPPropList* pPropList);
	virtual COLORREF GetPropListGroupTextColor (CBCGPPropList* pPropList);
	virtual COLORREF GetPropListDisabledTextColor(CBCGPPropList* pPropList);
	virtual COLORREF GetPropListDesciptionTextColor (CBCGPPropList* pPropList);
	virtual COLORREF GetPropListCommandTextColor (CBCGPPropList* pPropList);
#endif

	// Toolbox:
#ifndef BCGP_EXCLUDE_TOOLBOX
	virtual COLORREF GetToolBoxButtonTextColor (CBCGPToolBoxButton* pButton);
#endif

	// Editor colors:
	virtual void OnDrawEditCtrlResizeBox (CDC* pDC, CBCGPEditCtrl* /*pEdit*/, CRect rect);

	// Grid control:
#ifndef BCGP_EXCLUDE_GRID_CTRL
	virtual COLORREF OnFillGridItem (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill,
		BOOL bSelected, BOOL bActiveItem, BOOL bSortedColumn);

	virtual COLORREF OnFillGridGroupByBoxBackground (CDC* pDC, CRect rect);	
	virtual BOOL OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme);

	virtual CBrush& GetGridCaptionBrush(CBCGPGridCtrl* /*pCtrl*/)
	{
		return m_brBarBkgnd;
	}

	virtual void OnDrawGridDataBar (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rect);
	virtual void GetGridColorScaleBaseColors(CBCGPGridCtrl* pCtrl, COLORREF& clrLow, COLORREF& clrMid, COLORREF& clrHigh);

	// Report control:
	virtual COLORREF OnFillReportCtrlRowBackground (CBCGPGridCtrl* pCtrl, CDC* pDC,
		CRect rectFill, BOOL bSelected, BOOL bGroup);

#endif // BCGP_EXCLUDE_GRID_CTRL

    // Gantt control:
#if !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)
	virtual void GetGanttColors (const CBCGPGanttChart* pChart, BCGP_GANTT_CHART_COLORS& colors, COLORREF clrBack = CLR_DEFAULT) const;
    virtual void DrawGanttHeaderCell (const CBCGPGanttChart* pChart, CDC& dc, const BCGP_GANTT_CHART_HEADER_CELL_INFO& cellInfo, BOOL bHilite);
	virtual COLORREF GetGanttHeaderTextColor (BOOL bHilite) const;
#endif // !defined (BCGP_EXCLUDE_GRID_CTRL) && !defined (BCGP_EXCLUDE_GANTT)

#ifndef BCGP_EXCLUDE_TASK_PANE
	virtual void OnDrawScrollButtons(CDC* pDC, const CRect& rect, const int nBorderSize,
									int iImage, BOOL bHilited);
#endif

	virtual void OnFillSpinButton (CDC* pDC, CBCGPSpinButtonCtrl* pSpinCtrl, CRect rect, BOOL bDisabled);
	virtual COLORREF OnFillCommandsListBackground (CDC* pDC, CRect rect, BOOL bIsSelected = FALSE);

	virtual BOOL IsEmbossDisabledBtnText ()
	{
		return FALSE;
	}

// Push button:
	virtual BOOL OnDrawPushButton (CDC* pDC, CRect rect, CBCGPButton* pButton, COLORREF& clrText);
	virtual BOOL IsDrawFocusRectOnPushButton(CBCGPButton* /*pButton*/) 	{ return TRUE; }

#ifndef BCGP_EXCLUDE_RIBBON
	virtual COLORREF OnDrawRibbonPanel (CDC* pDC, CBCGPRibbonPanel* pPanel, CRect rectPanel, CRect rectCaption);
	virtual COLORREF OnFillRibbonButton (CDC* pDC, CBCGPRibbonButton* pButton);
	virtual COLORREF OnFillRibbonMainPanelButton (CDC* pDC, CBCGPRibbonButton* pButton);
#endif

	virtual void OnFillPropSheetHeaderArea(CDC* pDC, CBCGPPropertySheet* pPropSheet, CRect rect, BOOL& bDrawBottomLine);

	virtual void OnDrawCaptionBarInfoArea (CDC* pDC, CBCGPCaptionBar* pBar, CRect rect);

	virtual BOOL OnDrawBrowseButton (CDC* pDC, CRect rect, CBCGPEdit* pEdit, CBCGPVisualManager::BCGBUTTON_STATE state, COLORREF& clrText);

	virtual CBrush& GetEditCtrlBackgroundBrush(CBCGPEdit* pEdit);
	virtual COLORREF GetListControlMarkedColor(CBCGPListCtrl* pListCtrl);

	// Popup window:
#ifndef BCGP_EXCLUDE_POPUP_WINDOW
	virtual COLORREF GetPopupWindowLinkTextColor(CBCGPPopupWindow* pPopupWnd, BOOL bIsHover);
#endif

protected:

	virtual void CleanUp ();

	virtual void DrawNcBtn (CDC* pDC, const CRect& rect, UINT nButton, 
							BCGBUTTON_STATE state, BOOL bSmall, 
							BOOL bActive, BOOL bMDI = FALSE, BOOL bEnabled = TRUE);
	virtual void DrawNcCaption (CDC* pDC, CRect rectCaption, 
						   DWORD dwStyle, DWORD dwStyleEx,
						   const CString& strTitle, const CString& strDocument,
						   HICON hIcon, BOOL bPrefix, BOOL bActive, BOOL bTextCenter,
						   const CObList& lstSysButtons);

	void SetHS ();
	static COLORREF GetColor(COLORREF clr, double h, double s);

protected:
	CBCGPControlRenderer	m_ctrlBaseMainBorderCaption;
	CBCGPControlRenderer	m_ctrlBaseMainBorderL;
	CBCGPControlRenderer	m_ctrlBaseMainBorderR;
	CBCGPControlRenderer	m_ctrlBaseMainBorderTB;
	CBCGPControlRenderer	m_ctrlBaseMainBorder;
	CBCGPControlRenderer	m_ctrlBaseMDIChildBorder;
	CBCGPControlRenderer	m_ctrlBaseMiniBorderCaption;
	CBCGPControlRenderer	m_ctrlBaseMiniBorderTB;
	CBCGPControlRenderer	m_ctrlBaseMiniBorder;
	CBCGPControlRenderer	m_ctrlBaseDialogBorder;
	CBCGPControlRenderer    m_ctrlBaseSysBtnBack[2];
	CBCGPControlRenderer    m_ctrlBaseSysBtnBackC[2];
	CBCGPControlRenderer	m_ctrlBaseStatusBarBack;
	CBCGPControlRenderer	m_ctrlBaseStatusBarBack_Ext;
	CBCGPControlRenderer	m_ctrlBaseMenuBarBtn;
	CBCGPControlRenderer	m_ctrlBasePopupBorder;
	CBCGPToolBarImages		m_BaseToolBarGripper;
	CBCGPToolBarImages		m_BaseToolBarTear;
	CBCGPToolBarImages		m_BaseStatusBarPaneBorder;
	CBCGPToolBarImages		m_BaseStatusBarSizeBox;
	CBCGPControlRenderer	m_ctrlBaseToolBarBtn;
	CBCGPControlRenderer	m_ctrlBaseMenuItemBack;
	CBCGPToolBarImages		m_BaseMenuItemMarkerC;
	CBCGPToolBarImages		m_BaseMenuItemMarkerR;
	CBCGPControlRenderer	m_ctrlBaseRibbonBtnPush;

	COLORREF				m_clrBaseBarFace;
	COLORREF				m_clrBaseActiveCaption;
	COLORREF				m_clrBaseInactiveCaption;
	COLORREF				m_clrBaseBarShadow;
	COLORREF				m_clrBaseBarDkShadow;
	COLORREF				m_clrBaseBarLight;
	COLORREF				m_clrBaseFloatToolBarBorder;
	COLORREF				m_clrBaseHighlightGradientDark;
	COLORREF				m_clrBaseHighlightGradientLight;
	COLORREF				m_clrBaseHighlightDnGradientDark;
	COLORREF				m_clrBaseHighlightDnGradientLight;
	COLORREF				m_clrBaseHighlightCheckedGradientDark;
	COLORREF				m_clrBaseHighlightCheckedGradientLight;
	COLORREF				m_clrBasePressedButtonBorder;

	COLORREF				m_clrBaseDlgBackground;
	COLORREF				m_clrBaseDlgButtonsArea;
	CBCGPToolTipParams		m_BaseToolTipParams;
	COLORREF				m_clrBaseMainClientArea;
	COLORREF				m_clrBaseMenuLight;
	COLORREF				m_clrBaseMenuRarelyUsed;
	COLORREF				m_clrBaseMenuBorder;
	COLORREF				m_clrBaseMenuItemBorder;
	COLORREF				m_clrBaseMenuGutterLight;
	COLORREF				m_clrBaseMenuGutterDark;
	COLORREF				m_clrBaseSeparator1;
	COLORREF				m_clrBaseSeparator2;

	COLORREF				m_clrBaseBarBkgnd;
	COLORREF				m_clrBaseBarGradientLight;
	COLORREF				m_clrBaseBarGradientDark;
	COLORREF				m_clrBaseToolBarGradientLight;
	COLORREF				m_clrBaseToolBarGradientDark;
	COLORREF				m_clrBaseToolbarDisabled;
	COLORREF				m_clrBaseToolBarGradientVertLight;
	COLORREF				m_clrBaseToolBarGradientVertDark;
	COLORREF				m_clrBaseCustomizeButtonGradientLight;
	COLORREF				m_clrBaseCustomizeButtonGradientDark;
	COLORREF				m_clrBaseToolBarBottomLine;
	COLORREF				m_clrBaseSeparatorLight;
	COLORREF				m_clrBaseSeparatorDark;
	COLORREF				m_clrBaseMenuBarGradientLight;
	COLORREF				m_clrBaseMenuBarGradientDark;
	COLORREF				m_clrBaseMenuBarGradientVertLight;
	COLORREF				m_clrBaseMenuBarGradientVertDark;
	COLORREF				m_clrBaseCaptionBarGradientLight;
	COLORREF				m_clrBaseCaptionBarGradientDark;

	COLORREF				m_clrBaseTab3DFace;
	COLORREF				m_clrBaseTab3DBlack;
	COLORREF				m_clrBaseTab3DDark;
	COLORREF				m_clrBaseTab3DDarkShadow;
	COLORREF				m_clrBaseTab3DLight;
	COLORREF				m_clrBaseTab3DHighlight;
	COLORREF				m_clrBaseTabFlatFace;
	COLORREF				m_clrBaseTabFlatBlack;
	COLORREF				m_clrBaseTabFlatDark;
	COLORREF				m_clrBaseTabFlatDarkShadow;
	COLORREF				m_clrBaseTabFlatLight;
	COLORREF				m_clrBaseTabFlatHighlight;

	COLORREF				m_clrBasePopupGradientLight;
	COLORREF				m_clrBasePopupGradientDark;


	CBCGPControlRenderer	m_ctrlMainBorderL;
	CBCGPControlRenderer	m_ctrlMainBorderR;
	CBCGPControlRenderer	m_ctrlMainBorderTB;
	CBCGPControlRenderer	m_ctrlMiniBorderCaption;
	CBCGPControlRenderer	m_ctrlMiniBorderTB;
	CBCGPControlRenderer	m_ctrlMiniBorder;

	CBCGPControlRenderer    m_SysBtnBackC[2];
	CBCGPControlRenderer    m_SysBtnBackH[2];
	CBCGPControlRenderer    m_SysBtnBackCH[2];

	CBCGPControlRenderer	m_ctrlMiniSysBtn;

	COLORREF				m_clrMenuGutterLight;
	COLORREF				m_clrMenuGutterDark;

	COLORREF				m_clrSeparatorLight;
	COLORREF				m_clrSeparatorDark;

	COLORREF				m_clrFloatToolBarBorder;
	COLORREF				m_clrLinkText;
	COLORREF				m_clrLinkHotText;
	COLORREF				m_clrActionText;

	BOOL					m_bUpdateColors;

	XLinkImagesArray		m_arLinkImages;
	XLinkColorsArray		m_arLinkColors;

	double					m_dblHue;
	double					m_dblSaturation;
};

#endif // !defined(AFX_BCGPVISUALMANAGERCARBON_H__187C5819_4006_48EB_8C60_3F5529F47B2F__INCLUDED_)
