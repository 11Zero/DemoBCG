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
// BCGPVisualManagerScenic.h: interface for the CBCGPVisualManagerScenic class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPVISUALMANAGERSCENIC_H__AA17DB7C_5605_4267_8CED_E539BC7BFED6__INCLUDED_)
#define AFX_BCGPVISUALMANAGERSCENIC_H__AA17DB7C_5605_4267_8CED_E539BC7BFED6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPWinXPVisualManager.h"
#include "BCGPControlRenderer.h"
#include "BCGPVisualManager2007.h"
#include "BCGPToolTipCtrl.h"

class BCGCBPRODLLEXPORT CBCGPVisualManagerScenic : public CBCGPWinXPVisualManager  
{
	DECLARE_DYNCREATE (CBCGPVisualManagerScenic)

public:
	enum Style
	{
		VSScenic_Default,
		VSScenic_Win7,
		VSScenic_Win8,
	};

	virtual BOOL SetStyle (Style style, LPCTSTR lpszPath = NULL);
	Style GetStyle ();

	virtual BOOL SetStyle(LPCTSTR lpszPath = NULL);
	virtual void SetResourceHandle(HINSTANCE hinstRes);
	virtual void CleanStyle();

	Style GetCurrStyle ();	// Checks for OS if style is default

protected:
	virtual CString MakeResourceID(LPCTSTR lpszID) const;
	virtual CString GetStyleDllName() const;
	virtual CString GetStyleResourceID() const;

public:
	CBCGPVisualManagerScenic();
	virtual ~CBCGPVisualManagerScenic();

#ifndef BCGP_EXCLUDE_RIBBON
	virtual void SetMainButtonColor(COLORREF clr);	// -1 - default (blue)
#endif

	virtual BOOL IsOwnerDrawMenuCheck ();
	virtual BOOL IsHighlightWholeMenuItem ();

	virtual CFont& GetNcCaptionTextFont();
	virtual COLORREF GetNcCaptionTextColor(BOOL bActive, BOOL bTitle = TRUE) const;

	virtual BOOL OnNcActivate(CWnd* pWnd, BOOL bActive);
	virtual void OnActivateApp(CWnd* pWnd, BOOL bActive);
	virtual BOOL OnNcPaint (CWnd* pWnd, const CObList& lstSysButtons, CRect rectRedraw);
	
	virtual void OnDrawComboDropButton (CDC* pDC, CRect rect,
										BOOL bDisabled,
										BOOL bIsDropped,
										BOOL bIsHighlighted,
										CBCGPToolbarComboBoxButton* pButton);

	virtual void DrawSeparator (CDC* pDC, const CRect& rect, BOOL bHorz);
    virtual void DrawSeparator (CDC* pDC, const CRect& rect, CPen& pen1, CPen& pen2, BOOL bHorz);
	virtual void DrawNcBtn (CDC* pDC, const CRect& rect, UINT nButton, 
							BCGBUTTON_STATE state, BOOL bSmall, 
							BOOL bActive, BOOL bMDI = FALSE, BOOL bEnabled = TRUE);
	virtual void DrawNcText (CDC* pDC, CRect& rect, const CString& strTitle, 
							BOOL bActive, BOOL bIsRTL, BOOL bTextCenter,
							BOOL bGlass = FALSE, int nGlassGlowSize = 0, 
							COLORREF clrGlassText = (COLORREF)-1);

	virtual void OnUpdateSystemColors ();
	virtual void CleanUp ();

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

	virtual void OnHighlightMenuItem (CDC *pDC, CBCGPToolbarMenuButton* pButton,
		CRect rect, COLORREF& clrText);
	virtual void OnDrawMenuBorder (CDC* pDC, CBCGPPopupMenu* pMenu, CRect rect);
	virtual void OnDrawMenuCheck (CDC* pDC, CBCGPToolbarMenuButton* pButton, 
		CRect rect, BOOL bHighlight, BOOL bIsRadio);
	virtual void OnFillMenuImageRect (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);
	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);
	virtual void OnDrawButtonBorder (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	virtual void OnDrawSeparator (CDC* pDC, CBCGPBaseControlBar* pBar, CRect rect, BOOL bIsHoriz);
	virtual COLORREF OnDrawMenuLabel (CDC* pDC, CRect rect);

	virtual void OnDrawMenuResizeBar (CDC* pDC, CRect rect, int nResizeFlags);

	virtual void OnFillPopupMenuBackground (CDC* pDC, CRect rect);

	virtual COLORREF GetCaptionBarTextColor (CBCGPCaptionBar* pBar);
	virtual void OnDrawCaptionBarInfoArea (CDC* pDC, CBCGPCaptionBar* pBar, CRect rect);

	virtual COLORREF OnFillCaptionBarButton (CDC* pDC, CBCGPCaptionBar* pBar,
											CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
											BOOL bIsDisabled, BOOL bHasDropDownArrow,
											BOOL bIsSysButton);
	virtual void OnDrawCaptionBarButtonBorder (CDC* pDC, CBCGPCaptionBar* pBar,
											CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
											BOOL bIsDisabled, BOOL bHasDropDownArrow,
											BOOL bIsSysButton);

	virtual void OnFillOutlookBarCaption (CDC* pDC, CRect rectCaption, COLORREF& clrText);

	virtual COLORREF OnFillListBoxItem (CDC* pDC, CBCGPListBox* pListBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected);

	virtual void OnDrawEditClearButton(CDC* pDC, CBCGPButton* pButton, CRect rect);

	virtual void OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);

	// Calculator:
	virtual BOOL OnDrawCalculatorButton (CDC* pDC, CRect rect, CBCGPToolbarButton* pButton, CBCGPVisualManager::BCGBUTTON_STATE state, int cmd /* CBCGPCalculator::CalculatorCommands */, CBCGPCalculator* pCalculator);
	virtual BOOL OnDrawCalculatorDisplay (CDC* pDC, CRect rect, const CString& strText, BOOL bMem, CBCGPCalculator* pCalculator);
	virtual COLORREF GetCalculatorButtonTextColor (BOOL bIsUserCommand, int cmd /* CBCGPCalculator::CalculatorCommands */);

	virtual COLORREF GetMenuItemTextColor (CBCGPToolbarMenuButton* pButton, BOOL bHighlighted, BOOL bDisabled);
	virtual COLORREF GetHighlightedMenuItemTextColor (CBCGPToolbarMenuButton* pButton);	

	// ToolTip
	virtual BOOL GetToolTipParams (CBCGPToolTipParams& params, UINT nType = (UINT)(-1));

	// Edit contgrol:
	virtual COLORREF GetEditBackSidebarColor(CBCGPEditCtrl* pEdit);

	// Tab control:
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

	// Editor colors:
	virtual void OnDrawEditCtrlResizeBox (CDC* pDC, CBCGPEditCtrl* /*pEdit*/, CRect rect);

#ifndef BCGP_EXCLUDE_RIBBON
	virtual BOOL IsRibbonScenicLook()	{ return TRUE; }

	virtual COLORREF OnDrawRibbonTabsFrame (
					CDC* pDC, 
					CBCGPRibbonBar* pWndRibbonBar, 
					CRect rectTab);

	virtual COLORREF GetRibbonQATTextColor (BOOL bDisabled = FALSE)
	{
		if (!CanDrawImage ())
		{
			return CBCGPWinXPVisualManager::GetRibbonQATTextColor (bDisabled);
		}

		return bDisabled ? m_clrRibbonCategoryTextDisabled : m_clrRibbonCategoryText;
	}

	virtual COLORREF GetRibbonBarTextColor (BOOL bDisabled = FALSE)
	{
		if (!CanDrawImage ())
		{
			return CBCGPWinXPVisualManager::GetRibbonBarTextColor (bDisabled);
		}

		return bDisabled ? m_clrRibbonBarBtnTextDisabled : m_clrRibbonBarBtnText;
	}

	virtual void OnDrawRibbonPaletteBorder (
					CDC* pDC, 
					CBCGPRibbonPaletteButton* pButton, 
					CRect rectBorder);
	virtual void OnDrawRibbonCaption (
					CDC* pDC, CBCGPRibbonBar* pBar, CRect rectCaption,
					CRect rectText);
	virtual void OnDrawRibbonCaptionButton(CDC* pDC, CBCGPRibbonCaptionButton* pButton);
	virtual COLORREF OnDrawRibbonButtonsGroup (
					CDC* pDC,
					CBCGPRibbonButtonsGroup* pGroup,
					CRect rectGroup);
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
	virtual COLORREF OnFillRibbonPanelCaption (
					CDC* pDC,
					CBCGPRibbonPanel* pPanel, 
					CRect rectCaption);
	virtual void OnPreDrawRibbon (
					CDC* pDC, 
					CBCGPRibbonBar* pRibbonBar, 
					CRect rectTabs);
	virtual void OnDrawRibbonPaletteButton (
					CDC* pDC,
					CBCGPRibbonPaletteIcon* pButton);
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
	virtual void OnDrawDefaultRibbonImage (
					CDC* pDC, 
					CRect rectImage,
					BOOL bIsDisabled = FALSE,
					BOOL bIsPressed = FALSE,
					BOOL bIsHighlighted = FALSE);
	virtual void OnDrawRibbonMainButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);

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
	virtual void OnDrawRibbonButtonBorder (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);
	virtual void OnDrawRibbonPinnedButtonBorder (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);
	virtual void OnDrawRibbonLaunchButton (
					CDC* pDC,
					CBCGPRibbonLaunchButton* pButton,
					CBCGPRibbonPanel* pPanel);
	virtual COLORREF OnFillRibbonButton(CDC* pDC, CBCGPRibbonButton* pButton);
	virtual COLORREF OnFillRibbonPinnedButton(CDC* pDC, CBCGPRibbonButton* pButton, BOOL& bIsDarkPin);

	virtual void OnDrawRibbonDefaultPaneButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);

	virtual int GetRibbonQATChevronOffset ()	{	return 13;	}
	virtual int GetRibbonQATRightMargin ()	{	return 0;	}

	virtual COLORREF GetRibbonEditBackgroundColor (
					CBCGPRibbonEditCtrl* pEdit,
					BOOL bIsHighlighted,
					BOOL bIsPaneHighlighted,
					BOOL bIsDisabled);

	virtual void OnDrawRibbonMenuCheckFrame (
					CDC* pDC,
					CBCGPRibbonButton* pButton, 
					CRect rect);

	virtual void OnFillRibbonQATPopup (
				CDC* pDC, CBCGPRibbonPanelMenuBar* pMenuBar, CRect rect);

	virtual int GetRibbonPopupBorderSize (const CBCGPRibbonPanelMenu* pPopup) const;

	virtual int GetRibbonPanelMargin(CBCGPRibbonCategory* pCategory);

	virtual void OnDrawRibbonMainPanelFrame (
					CDC* pDC, 
					CBCGPRibbonMainPanel* pPanel, 
					CRect rect);

	virtual void OnDrawRibbonRecentFilesFrame (
					CDC* pDC, 
					CBCGPRibbonMainPanel* pPanel, 
					CRect rect);

	virtual BOOL IsLayeredRibbonKeyTip () const
	{
		return CanDrawImage ();
	}

	virtual void OnDrawRibbonKeyTip (CDC* pDC, CBCGPBaseRibbonElement* pElement, CRect rect, CString str);

#endif

#ifndef BCGP_EXCLUDE_PROP_LIST
	virtual COLORREF GetPropListGroupTextColor (CBCGPPropList* pPropList);
#endif

	virtual void FillRebarPane (CDC* pDC, CBCGPBaseControlBar* pBar, CRect rectClient);

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

protected:
	CFont	m_AppCaptionFont;
	CPen	m_penSeparatorDark;
	CPen	m_penSeparatorLight;
	CBrush	m_brTabFace;

	BOOL	m_bLoaded;

	BOOL	m_bNcTextCenter;

	CBCGPControlRenderer m_ctrlPopupBorder;
	CBCGPControlRenderer m_ctrlPopupResizeBar;
	CBCGPToolBarImages   m_PopupResizeBar_HV;
	CBCGPToolBarImages   m_PopupResizeBar_HVT;
	CBCGPToolBarImages   m_PopupResizeBar_V;

	COLORREF m_clrMenuText;
	COLORREF m_clrMenuTextHighlighted;
	COLORREF m_clrMenuTextDisabled;

	COLORREF m_clrBarGradientLight;
	COLORREF m_clrBarGradientDark;

	COLORREF m_clrRibbonPanelText;
	COLORREF m_clrRibbonPanelTextHighlighted;
	COLORREF m_clrRibbonPanelCaptionText;
	COLORREF m_clrRibbonPanelCaptionTextHighlighted;

	COLORREF m_clrRibbonEdit;
	COLORREF m_clrRibbonEditDisabled;
	COLORREF m_clrRibbonEditHighlighted;
	COLORREF m_clrRibbonEditPressed;
	COLORREF m_clrRibbonEditBorder;
	COLORREF m_clrRibbonEditBorderDisabled;
	COLORREF m_clrRibbonEditBorderHighlighted;
	COLORREF m_clrRibbonEditBorderPressed;
	COLORREF m_clrRibbonEditSelection;
	CBCGPControlRenderer m_ctrlRibbonComboBoxBtn;

	CBCGPControlRenderer m_ctrlMenuItemBack;
    CBCGPToolBarImages   m_MenuItemMarkerC;
    CBCGPToolBarImages   m_MenuItemMarkerR;
	CBCGPControlRenderer m_ctrlMenuHighlighted[2];

	CBCGPControlRenderer m_ctrlRibbonCaptionQA;

    COLORREF             m_clrRibbonCategoryText;
    COLORREF             m_clrRibbonCategoryTextHighlighted;
	COLORREF             m_clrRibbonCategoryTextDisabled;
	CBCGPControlRenderer m_ctrlRibbonCategoryBack;
	CBCGPControlRenderer m_ctrlRibbonCategoryTab;
	CBCGPControlRenderer m_ctrlRibbonCategoryTabSep;
	CBCGPControlRenderer m_ctrlRibbonCategoryTabFrame;
	CBCGPControlRenderer m_ctrlRibbonCategoryBtnPage[2];
	CBCGPControlRenderer m_ctrlRibbonPanelBack;
	CBCGPControlRenderer m_ctrlRibbonPanelBackSep;
	CBCGPToolBarImages   m_RibbonPanelSeparator;
	CBCGPControlRenderer m_ctrlRibbonPanelQAT;
	CBCGPControlRenderer m_ctrlRibbonMainPanel;
	CBCGPControlRenderer m_ctrlRibbonBtnMainPanel;
	CBCGPControlRenderer m_ctrlRibbonBtnGroup_S;
	CBCGPControlRenderer m_ctrlRibbonBtnGroup_F;
	CBCGPControlRenderer m_ctrlRibbonBtnGroup_M;
	CBCGPControlRenderer m_ctrlRibbonBtnGroup_L;
	CBCGPControlRenderer m_ctrlRibbonBtnGroupMenu_F[2];
	CBCGPControlRenderer m_ctrlRibbonBtnGroupMenu_M[2];
	CBCGPControlRenderer m_ctrlRibbonBtnGroupMenu_L[2];
	CBCGPControlRenderer m_ctrlRibbonBtn[2];
	CBCGPControlRenderer m_ctrlRibbonBtnMenuH[2];
	CBCGPControlRenderer m_ctrlRibbonBtnMenuV[2];
	CBCGPControlRenderer m_ctrlRibbonBtnLaunch;
	CBCGPToolBarImages m_RibbonBtnLaunchIcon;
	CBCGPControlRenderer m_ctrlRibbonBtnMain;
	CBCGPControlRenderer m_ctrlRibbonBtnMainColorized;
	CBCGPControlRenderer m_ctrlRibbonSliderBtnPlus;
	CBCGPControlRenderer m_ctrlRibbonSliderBtnMinus;
	CBCGPToolBarImages	 m_RibbonBtnDefaultImage;
	CBCGPControlRenderer m_ctrlRibbonBtnDefault;
	CBCGPControlRenderer m_ctrlRibbonBtnDefaultIcon;
	CBCGPControlRenderer m_ctrlRibbonBtnDefaultQAT;
	CBCGPControlRenderer m_ctrlRibbonBtnCheck;
	CBCGPControlRenderer m_ctrlRibbonBtnRadio;
	CBCGPControlRenderer m_ctrlRibbonBtnStatusPane;
	CBCGPControlRenderer m_ctrlRibbonBtnPalette[3];
    COLORREF             m_clrRibbonBarBtnText;
	COLORREF             m_clrRibbonBarBtnTextHighlighted;
    COLORREF             m_clrRibbonBarBtnTextDisabled;

	CBCGPControlRenderer m_ctrlRibbonBorder_QAT;
	CBCGPControlRenderer m_ctrlRibbonBorder_Floaty;
	CBCGPControlRenderer m_ctrlRibbonBorder_Panel;

	CBCGPControlRenderer	m_ctrlRibbonKeyTip;
	COLORREF				m_clrRibbonKeyTipTextNormal;
	COLORREF				m_clrRibbonKeyTipTextDisabled;

	struct XRibbonContextCategory
	{
		CBCGPControlRenderer	m_ctrlBtnDefault;
		CBCGPControlRenderer	m_ctrlCaption;
		CBCGPControlRenderer	m_ctrlBack;
		CBCGPControlRenderer	m_ctrlTab;
		CBCGPControlRenderer	m_ctrlSeparator;
		CBCGPControlRenderer	m_ctrlPanelBack;
		CBCGPBitmapCache		m_cacheBack;
		CBCGPBitmapCache		m_cacheBtnDefault;
		CBCGPBitmapCache		m_cachePanelBack;
		COLORREF				m_clrCaptionText;
		COLORREF				m_clrText;
		COLORREF				m_clrTextHighlighted;

		void CleanUp ()
		{
			m_ctrlCaption.CleanUp ();
			m_ctrlBack.CleanUp ();
			m_ctrlTab.CleanUp ();
			m_ctrlSeparator.CleanUp ();
			m_ctrlBtnDefault.CleanUp ();
			m_ctrlPanelBack.CleanUp ();
			m_cacheBack.Clear ();
			m_cacheBtnDefault.Clear ();
			m_cachePanelBack.Clear ();
		}
	};

	CBCGPControlRenderer	m_ctrlRibbonContextSeparator;
	XRibbonContextCategory  m_ctrlRibbonContextCategory[BCGPRibbonCategoryColorCount];

	CBCGPBitmapCache m_cacheRibbonCategoryBack;
	CBCGPBitmapCache m_cacheRibbonPanelBack;
	CBCGPBitmapCache m_cacheRibbonBtnGroup_S;
	CBCGPBitmapCache m_cacheRibbonBtnGroup_F;
	CBCGPBitmapCache m_cacheRibbonBtnGroup_M;
	CBCGPBitmapCache m_cacheRibbonBtnGroup_L;
	CBCGPBitmapCache m_cacheRibbonBtnGroupMenu_F[2];
	CBCGPBitmapCache m_cacheRibbonBtnGroupMenu_M[2];
	CBCGPBitmapCache m_cacheRibbonBtnGroupMenu_L[2];
	CBCGPBitmapCache m_cacheRibbonBtnDefault;

	static Style	m_Style;
	CString			m_strStylePrefix;
	HINSTANCE		m_hinstRes;
	BOOL			m_bAutoFreeRes;

	CBCGPToolTipParams	m_ToolTipParams;
	BOOL				m_bToolTipParams;
};

#endif // !defined(AFX_BCGPVISUALMANAGERSCENIC_H__AA17DB7C_5605_4267_8CED_E539BC7BFED6__INCLUDED_)
