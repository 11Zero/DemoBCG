#if !defined(AFX_BCGPPROPERTYSHEET_H__54B75502_E449_41DD_AEE7_1BB6FBE72205__INCLUDED_)
#define AFX_BCGPPROPERTYSHEET_H__54B75502_E449_41DD_AEE7_1BB6FBE72205__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BCGPPropertySheet.h : header file
//

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
// BCGPropertySheet.h : header file
//

#ifndef __AFXTEMPL_H__
	#include "afxtempl.h"
#endif

#include "BCGCBPro.h"

#if (!defined _BCGSUITE_) && (!defined _BCGSUITE_INC_)
#include "BCGPOutlookBar.h"
#include "BCGPOutlookBarPane.h"
#include "BCGPTabWnd.h"
#include "BCGPToolBarImages.h"
#endif

#include "BCGPDlgImpl.h"
#include "BCGPListBox.h"
#include "BCGPTreeCtrl.h"
#include "BCGPButton.h"
#include "BCGPPageTransitionManager.h"

class CBCGPPropertySheet;
class CBCGPPropertyPage;

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropSheetBtn

class CBCGPPropSheetBtn : public CBCGPButton
{
	friend class CBCGPPropertySheet;

	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropSheetPane

class CBCGPPropSheetPane : public CBCGPOutlookBarPane
{
	friend class CBCGPPropertySheet;

	virtual BOOL OnSendCommand (const CBCGPToolbarButton* pButton);
	void EnsureVisible (int iButton);

	CBCGPPropertySheet* m_pParent;
};

class CBCGPPropSheetTab : public CBCGPTabWnd
{
	friend class CBCGPPropertySheet;

	CBCGPPropSheetTab();

	virtual BOOL SetActiveTab (int iTab);
	virtual void OnCustomFontChanged();

	CBCGPPropertySheet* m_pParent;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropSheetCategory

class BCGCBPRODLLEXPORT CBCGPPropSheetCategory : public CObject
{
	friend class CBCGPPropertySheet;

	DECLARE_DYNAMIC(CBCGPPropSheetCategory)

	CBCGPPropSheetCategory(LPCTSTR lpszName, int nIcon, int nSelectedItem,
		const CBCGPPropSheetCategory* pParentCategory,
		CBCGPPropertySheet& propSheet);
	virtual ~CBCGPPropSheetCategory();

	const CString					m_strName;
	const int						m_nIcon;
	const int						m_nSelectedIcon;
	CBCGPPropSheetCategory*			m_pParentCategory;
	HTREEITEM						m_hTreeItem;

	CList<CBCGPPropSheetCategory*,CBCGPPropSheetCategory*>		m_lstSubCategories;
	CList<CBCGPPropertyPage*, CBCGPPropertyPage*>				m_lstPages;

	HTREEITEM						m_hLastSelectedItem;
	CBCGPPropertySheet&				m_propSheet;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropertySheet

class BCGCBPRODLLEXPORT CBCGPPropertySheet :	public CPropertySheet,
												public CBCGPPageTransitionManager
{
	friend class CBCGPPropSheetPane;
	friend class CBCGPPropSheetCategory;
	friend class CBCGPPropertyPage;
	friend class CBCGPPropSheetBtn;
	friend class CBCGPControlsLayout;
	friend class CBCGPRibbonBackstageViewItemPropertySheet;
	friend class CBCGPDlgImpl;
	friend class CBCGPPopupMenu;

	DECLARE_DYNAMIC(CBCGPPropertySheet)

// Construction
public:
	CBCGPPropertySheet();
	CBCGPPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CBCGPPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

	enum PropSheetLook
	{
		PropSheetLook_Tabs,
		PropSheetLook_OutlookBar,
		PropSheetLook_Tree,
		PropSheetLook_OneNoteTabs,
		PropSheetLook_List,
		PropSheetLook_Wizard,
		PropSheetLook_AeroWizard,
		PropSheetLook_Pointer,
	};

	// Should be called BEFORE DoModal or Create!
	void SetLook (PropSheetLook look, int nNavControlWidth = 100, BOOL bGlassEffect = TRUE /* PropSheetLook_AeroWizard */);
	PropSheetLook GetLook () const
	{
		return m_look;
	}

	static BOOL	m_bUseOldLookInTreeMode;

// Attributes
public:
	CBCGPTabWnd& GetTab () const;
	int GetHeaderHeight () const
	{
		return m_nHeaderHeight;
	}

	int GetAeroHeight() const
	{
		return m_nAeroHeight;
	}

	BOOL IsGlassHeader() const
	{
		return m_bGlassArea && m_bDrawHeaderOnAeroCaption;
	}

	int GetNavBarWidth() const;

public:
	void EnableVisualManagerStyle (BOOL bEnable = TRUE, BOOL bNCArea = FALSE);

	BOOL IsVisualManagerStyle () const
	{
		return m_Impl.m_bVisualManagerStyle;
	}

	BOOL IsVisualManagerNCArea () const
	{
		return m_Impl.m_bVisualManagerNCArea;
	}

	// Allow to drag the dialog box by clicking inside the client area
	void EnableDragClientArea(BOOL bEnable = TRUE);
	BOOL IsDragClientAreaEnabled() const
	{
		return m_Impl.IsDragClientAreaEnabled();
	}
	
	// Layout:
	void EnableLayout(BOOL bEnable = TRUE, CRuntimeClass* pRTC = NULL);
	BOOL IsLayoutEnabled() const
	{
		return m_Impl.IsLayoutEnabled();
	}

	CBCGPControlsLayout* GetLayout()
	{
		return m_Impl.m_pLayout;
	}

	virtual void AdjustControlsLayout();

	void EnableLoadWindowPlacement(BOOL bEnable = TRUE)
	{
		m_Impl.m_bLoadWindowPlacement = bEnable;
	}

	BOOL IsWindowPlacementEnabled() const
	{
		return m_Impl.m_bLoadWindowPlacement;
	}

	// Tabs scrolling:
	BOOL IsTabsScrollingEnabled() const
	{
		return m_bIsTabsScrolling;
	}

	// PropSheetLook_Tabs mode only:
	void EnableTabsScrolling(BOOL bEnable = TRUE);

	// Page transition effects:
	void EnablePageTransitionEffect(CBCGPPageTransitionManager::BCGPPageTransitionEffect effect, int nTime = 300 /* ms */);

	BOOL IsPageTransitionAvailable() const
	{
		return m_PageTransitionEffect != BCGPPageTransitionNone && IsVisualManagerStyle();
	}

	BOOL SetActivePageWithEffects(int nPage);

protected:
	PropSheetLook		m_look;
	CBCGPOutlookBar		m_wndOutlookBar;
	CBCGPPropSheetPane	m_wndPane1;
	CBCGPPropSheetTab	m_wndTab;
	CBCGPTreeCtrl		m_wndTree;
	CBCGPListBox		m_wndList;
	int					m_nBarWidth;
	int					m_nActivePage;
	CImageList			m_Icons;
	BOOL				m_bAlphaBlendIcons;
	int					m_nHeaderHeight;
	int					m_nAeroHeight;
	BOOL				m_bDrawHeaderOnAeroCaption;
	BOOL				m_bWasMaximized;
	BOOL				m_bDrawPageFrame;
	CBCGPPropSheetBtn	m_btnBack;
	CBCGPToolBarImages	m_NavImages;
	CBCGPToolBarImages	m_NavImages16;
	BOOL				m_bGlassArea;
	BOOL				m_bGlassEffect;
	BOOL				m_bIsReady;
	CSize				m_sizePrev;
	CSize				m_sizeOriginal;
	BOOL				m_bInAdjustLayout;
	BOOL				m_bBackstageMode;
	BOOL				m_bIsTabsScrolling;
	int					m_nNewActivePage;

	CList<CBCGPPropSheetCategory*,CBCGPPropSheetCategory*>	m_lstTreeCategories;

	BOOL				m_bIsInSelectTree;
	CBCGPDlgImpl		m_Impl;

	CStringArray		m_arGroupCaptions;
	CArray<int, int>	m_arGroupIndexes;

// Operations
public:
	BOOL SetIconsList (UINT uiImageListResID, int cx, COLORREF clrTransparent = RGB (255, 0, 255));
	void SetIconsList (HIMAGELIST hIcons);

	void AddPage(CPropertyPage* pPage);
	void AddGroup(LPCTSTR lpszGroup);	// For PropSheetLook_List style only!

	void RemovePage(CPropertyPage* pPage);
	void RemovePage(int nPage);
	void RemoveCategory (CBCGPPropSheetCategory* pCategory);

	void RenamePage(CPropertyPage* pPage, const CString& strPageName);
	void RenamePage(int nPage, const CString& strPageName);

	// PropSheetLook_Tree methods:
	CBCGPPropSheetCategory* AddTreeCategory (LPCTSTR lpszLabel, 
		int nIconNum = -1, int nSelectedIconNum = -1,
		const CBCGPPropSheetCategory* pParentCategory = NULL);
	void AddPageToTree (CBCGPPropSheetCategory* pCategory, 
		CBCGPPropertyPage* pPage, int nIconNum = -1, int nSelIconNum = -1);

	void EnablePageHeader (int nHeaderHeight, BOOL bDrawHeaderOnAeroCaption = FALSE /* PropSheetLook_AeroWizard mode only */);

	// Call this method to re-subclass buttons:
	void AdjustButtons ();

	void OnDrawListBoxBackground(CDC* pDC, CBCGPListBox* pListBox);

// Overrides
	virtual void OnActivatePage (CPropertyPage* pPage);
	virtual CWnd* InitNavigationControl ();
	virtual BOOL OnRemoveTreePage(CPropertyPage* pPage);

	virtual void OnDrawPageHeader (CDC* pDC, int nPage, CRect rectHeader);
	virtual BOOL OnWizardChangePageWidthTransitionEffect(CBCGPPropertyPage* pCurrPage, BOOL bNext);
	
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBCGPPropertySheet)
	public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBCGPPropertySheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBCGPPropertySheet)
	afx_msg void OnSysColorChange();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcPaint();
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BCGNcHitTestType OnNcHitTest(CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnPaint();
	//}}AFX_MSG
	afx_msg LRESULT OnAfterActivatePage(WPARAM,LPARAM);
	afx_msg LRESULT OnAdjustButtons(WPARAM,LPARAM);
	afx_msg void OnSelectTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectList();
	afx_msg LRESULT OnDWMCompositionChanged(WPARAM,LPARAM);
	afx_msg LRESULT OnChangeVisualManager (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGSetControlBackStageMode (WPARAM, LPARAM);
	afx_msg LRESULT OnSetText(WPARAM, LPARAM);
	afx_msg LRESULT OnPowerBroadcast(WPARAM wp, LPARAM);
	afx_msg LRESULT OnAdjustWizardPage(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

	void InternalAddPage (int nTab);
	void AddCategoryToTree (CBCGPPropSheetCategory* pCategory);
	void CommonInit ();
	int FindPageIndexInList (CPropertyPage* pPage);
	void OnApply ();
	virtual int ReposButtons (BOOL bRedraw = FALSE);
	void DrawWizardGutter(CDC* pDC);
	void DrawWizardAero(CDC* pDC);
	int CalcNavBarWidth();

	virtual BOOL OnSetPlacement(WINDOWPLACEMENT& wp);
	virtual void OnPageTransitionFinished();

	void SetActiveMenu (CBCGPPopupMenu* pMenu);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BCGPPROPERTYSHEET_H__54B75502_E449_41DD_AEE7_1BB6FBE72205__INCLUDED_)
