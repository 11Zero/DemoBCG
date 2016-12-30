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
// BCGPVisualManager2010.h: interface for the CBCGPVisualManager2010 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPVISUALMANAGER2010_H__7FC8A403_A388_41A0_B802_37AB6525DBD8__INCLUDED_)
#define AFX_BCGPVISUALMANAGER2010_H__7FC8A403_A388_41A0_B802_37AB6525DBD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPVisualManager2007.h"

class BCGCBPRODLLEXPORT CBCGPVisualManager2010 : public CBCGPVisualManager2007
{
	DECLARE_DYNCREATE(CBCGPVisualManager2010)

public:
	enum Style
	{
		VS2010_Blue,
		VS2010_Silver,
		VS2010_Black
	};

	static BOOL SetStyle (CBCGPVisualManager2010::Style style, LPCTSTR lpszPath = NULL);
	static CBCGPVisualManager2010::Style GetStyle();
	static void SetResourceHandle (HINSTANCE hinstRes);

	static void CleanStyle ();

protected:
	CBCGPVisualManager2010();
	virtual ~CBCGPVisualManager2010();

	static CString GetStyleResourceID(CBCGPVisualManager2010::Style style);

public:

#ifndef BCGP_EXCLUDE_RIBBON
	virtual void SetMainButtonColor(COLORREF clr);	// -1 - default (blue)
#endif

	virtual void OnUpdateSystemColors ();

protected:
	virtual void CleanUp ();

	virtual BOOL IsSmallMenuSystemButton () const	{	return TRUE;	}
	virtual void DrawSeparator2 (CDC* pDC, const CRect& rect, BOOL bHorz);

	virtual BOOL DoesRibbonExtendCaptionToTabsArea()	{	return TRUE;	}
	virtual int GetMessageBarMargin()	{	return 0;	}

	virtual void OnFillBarBackground (CDC* pDC, CBCGPBaseControlBar* pBar,
									CRect rectClient, CRect rectClip,
									BOOL bNCArea = FALSE);

	virtual COLORREF GetCaptionBarTextColor (CBCGPCaptionBar* pBar);
	virtual void OnDrawCaptionBarInfoArea (CDC* pDC, CBCGPCaptionBar* pBar, CRect rect);

	virtual CBrush& GetDlgButtonsAreaBrush(CWnd* pDlg, COLORREF* pclrLine);

	virtual COLORREF OnFillCaptionBarButton (CDC* pDC, CBCGPCaptionBar* pBar,
											CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
											BOOL bIsDisabled, BOOL bHasDropDownArrow,
											BOOL bIsSysButton);
	virtual void OnDrawCaptionBarButtonBorder (CDC* pDC, CBCGPCaptionBar* pBar,
											CRect rect, BOOL bIsPressed, BOOL bIsHighlighted, 
											BOOL bIsDisabled, BOOL bHasDropDownArrow,
											BOOL bIsSysButton);
	virtual void OnDrawSeparator (CDC* pDC, CBCGPBaseControlBar* pBar, CRect rect, BOOL bIsHoriz);
	virtual COLORREF OnDrawMenuLabel (CDC* pDC, CRect rect);

#ifndef BCGP_EXCLUDE_RIBBON
	virtual BOOL IsRibbonScenicLook()	{ return TRUE; }
	virtual int GetRibbonPanelMarginTop() const;

	virtual int GetRibbonQATChevronOffset ()	{	return 13;	}
	virtual int GetRibbonQATRightMargin ()	{	return 0;	}

	virtual void OnDrawRibbonMainButton (
					CDC* pDC, 
					CBCGPRibbonButton* pButton);
	virtual void OnDrawRibbonCaption (
					CDC* pDC, CBCGPRibbonBar* pBar, CRect rectCaption,
					CRect rectText);

	virtual COLORREF OnDrawRibbonCategoryCaption (
					CDC* pDC, 
					CBCGPRibbonContextCaption* pContextCaption);
	virtual void OnDrawRibbonCategoryCaptionText(
					CDC* pDC, CBCGPRibbonContextCaption* pContextCaption, 
					CString& strText, CRect rectText, BOOL bIsOnGlass, BOOL bIsZoomed);

	virtual void OnDrawRibbonCategory (
					CDC* pDC, 
					CBCGPRibbonCategory* pCategory, 
					CRect rectCategory);
	virtual COLORREF OnFillRibbonButton(CDC* pDC, CBCGPRibbonButton* pButton);
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
	virtual int GetRibbonPanelMargin(CBCGPRibbonCategory* pCategory);

	virtual void OnDrawRibbonMinimizeButtonImage(CDC* pDC, CBCGPRibbonMinimizeButton* pButton, BOOL bRibbonIsMinimized);
	virtual CSize GetRibbonMinimizeButtonImageSize();

	virtual COLORREF GetRibbonEditBackgroundColor (
					CBCGPRibbonEditCtrl* pEdit,
					BOOL bIsHighlighted,
					BOOL bIsPaneHighlighted,
					BOOL bIsDisabled);

	virtual int GetRibbonButtonsGroupHorzMargin()		{	return 3;		}
	virtual BOOL IsDrawRibbonGroupSeparator()			{	return TRUE;	}
	virtual void OnDrawRibbonGroupSeparator (CDC* pDC, CRect rectSeparator);

	virtual int GetRibbonBackstageTopLineHeight()		{	return 3;		}
	virtual void OnDrawRibbonBackstageTopLine(CDC* pDC, CRect rectLine);
	virtual CFont* GetBackstageViewEntryFont();

	virtual BOOL IsRibbonTabsAreaDark()	{	return CBCGPVisualManager2010::m_Style == CBCGPVisualManager2010::VS2010_Black;	}

	virtual void OnPreDrawRibbon (CDC* pDC, CBCGPRibbonBar* pRibbonBar, CRect rectTabs);

	virtual void OnDrawRibbonPaletteButtonIcon(
		CDC* pDC,
		CBCGPRibbonPaletteIcon* pButton,
		int nID,
		CRect rect);

#endif

	// Push button:
	virtual BOOL OnDrawPushButton (CDC* pDC, CRect rect, CBCGPButton* pButton, COLORREF& clrText);
	virtual BOOL IsDrawFocusRectOnPushButton(CBCGPButton* /*pButton*/) 	{ return FALSE; }

	// Separator:
	virtual void OnDrawDlgSeparator(CDC* pDC, CBCGPStatic* pCtrl, CRect rect, BOOL bIsHorz);

	// Outlook bar:
	virtual int GetOutlookBarSplitterHeight() { return 6; }

	virtual void OnFillOutlookPageButton (CDC* pDC, const CRect& rect,
										BOOL bIsHighlighted, BOOL bIsPressed,
										COLORREF& clrText);
	virtual void OnDrawOutlookPageButtonBorder (CDC* pDC, CRect& rectBtn, BOOL bIsHighlighted, BOOL bIsPressed);

	virtual void OnDrawOutlookBarFrame(CDC* pDC, CRect rectFrame);

	virtual COLORREF OnFillListBoxItem (CDC* pDC, CBCGPListBox* pListBox, int nItem, CRect rect, BOOL bIsHighlihted, BOOL bIsSelected);
	virtual void GetCalendarColors (const CBCGPCalendar* pCalendar,
				   CBCGPCalendarColors& colors);

	virtual void OnFillButtonInterior (CDC* pDC,
		CBCGPToolbarButton* pButton, CRect rect, CBCGPVisualManager::BCGBUTTON_STATE state);

	// Tree control:
	virtual COLORREF GetTreeControlFillColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected = FALSE, BOOL bIsFocused = FALSE, BOOL bIsDisabled = FALSE);
	virtual COLORREF GetTreeControlTextColor(CBCGPTreeCtrl* pTreeCtrl, BOOL bIsSelected = FALSE, BOOL bIsFocused = FALSE, BOOL bIsDisabled = FALSE);

#ifndef BCGP_EXCLUDE_PLANNER

	//virtual COLORREF GetPlannerSeparatorColor (CBCGPPlannerView* pView);
	virtual COLORREF OnFillPlannerCaption (CDC* pDC, CBCGPPlannerView* pView,
		CRect rect, BOOL bIsToday, BOOL bIsSelected, BOOL bNoBorder = FALSE,
		BOOL bHorz = TRUE);
	virtual void OnDrawPlannerHeader (CDC* pDC, 
		CBCGPPlannerView* pView, CRect rect);

#endif

	// Grid control:
#ifndef BCGP_EXCLUDE_GRID_CTRL
	virtual COLORREF OnFillReportCtrlRowBackground (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill, BOOL bSelected, BOOL bGroup);
	virtual COLORREF OnFillGridItem (CBCGPGridCtrl* pCtrl, CDC* pDC, CRect rectFill, BOOL bSelected, BOOL bActiveItem, BOOL bSortedColumn);
	virtual BOOL OnSetGridColorTheme (CBCGPGridCtrl* pCtrl, BCGP_GRID_COLOR_DATA& theme);
#endif // BCGP_EXCLUDE_GRID_CTRL

protected:
	CBCGPToolBarImages		m_RibbonBtnMinimizeIcon;
	CBCGPControlRenderer	m_ctrlRibbonCategoryTabFrame;
	CBCGPControlRenderer	m_ctrlRibbonPanelBack;

	CBCGPControlRenderer	m_ctrlRibbonBtnGroupSeparator;
	CBCGPControlRenderer	m_ctrlRibbonBtnBsNormal;
	CBCGPControlRenderer	m_ctrlRibbonBtnBsView;
	CBCGPToolBarImages		m_RibbonBtnBsViewIcon;
	CBCGPToolBarImages		m_RibbonBtnBsListIcon;
	CBCGPToolBarImages		m_RibbonBsSeparatorHorz;
	CBCGPToolBarImages		m_RibbonBsSeparatorVert;
	CBCGPControlRenderer	m_ctrlRibbonBsPanelBack;
	COLORREF				m_clrRibbonBsTextNormal;
	COLORREF				m_clrRibbonBsTextHighlighted;
	COLORREF				m_clrRibbonBsTextActive;
	COLORREF				m_clrRibbonBsTextDisabled;
	CBCGPControlRenderer	m_ctrlRibbonBsBtnPush;

	CBCGPBitmapCache		m_cacheRibbonPanelBack;

	struct XRibbonContextCategory2
	{
		CBCGPControlRenderer	m_ctrlSeparator;
		CBCGPControlRenderer	m_ctrlPanelBack;
		CBCGPControlRenderer	m_ctrlTabGlow;
		CBCGPBitmapCache		m_cachePanelBack;

		void CleanUp ()
		{
			m_ctrlSeparator.CleanUp ();
			m_ctrlPanelBack.CleanUp ();
			m_ctrlTabGlow.CleanUp ();
			m_cachePanelBack.Clear ();
		}
	};

	XRibbonContextCategory2	m_ctrlRibbonContextCategory2[BCGPRibbonCategoryColorCount];

	CBCGPControlRenderer	m_ctrlRibbonBtnMainColorized;
	CBCGPControlRenderer	m_ctrlRibbonBtnBsNormalColorized;
	CBCGPControlRenderer	m_ctrlRibbonBtnBsViewColorized;

	static Style			m_Style;

	virtual BOOL ParseStyleXMLVersion (const CString& strItem);
	virtual BOOL ParseStyleXMLRibbon (const CString& strItem);
};

#endif // !defined(AFX_BCGPVISUALMANAGER2010_H__7FC8A403_A388_41A0_B802_37AB6525DBD8__INCLUDED_)
