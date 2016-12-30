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
// BCGPPropertySheet.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPPropertyPage.h"
#include "BCGPPropertySheet.h"
#include "BCGPVisualManager.h"
#include "TrackMouse.h"
#include "BCGPDrawManager.h"
#include "BCGPButton.h"
#include "BCGPLocalResource.h"
#include "BCGProRes.h"
#include "BCGPGestureManager.h"

#ifndef _BCGSUITE_
#include "BCGPPngImage.h"
#include "BCGPOutlookWnd.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int idTree = 101;
const int idTab = 102;
const int idList = 103;

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropSheetBtn

BEGIN_MESSAGE_MAP(CBCGPPropSheetBtn, CBCGPButton)
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

void CBCGPPropSheetBtn::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rectClient;
	GetClientRect (&rectClient);

	CBCGPMemDC memDC (dc, this);
	CDC* pDC = &memDC.GetDC ();

	globalData.DrawParentBackground (this, pDC);

	int nIndex = 1;

	if (!IsWindowEnabled())
	{
		nIndex = 0;
	}
	else if (IsPressed())
	{
		nIndex = 3;
	}
	else if (IsHighlighted() || (GetFocus() == this))
	{
		nIndex = 2;
	}


	CBCGPPropertySheet* pParent = DYNAMIC_DOWNCAST(CBCGPPropertySheet, GetParent());
	if (pParent == NULL)
	{
		return;
	}

	CBCGPToolBarImages& images = 
		(globalData.m_nBitsPerPixel <= 8 || globalData.IsHighContastMode ()) && pParent->m_NavImages16.GetCount () > 0 ?
		pParent->m_NavImages16 : pParent->m_NavImages;

	images.DrawEx (pDC, rectClient, nIndex, 
			CBCGPToolBarImages::ImageAlignHorzCenter,
			CBCGPToolBarImages::ImageAlignVertCenter);
}
//**************************************************************************************
void CBCGPPropSheetBtn::OnSetFocus(CWnd* pOldWnd) 
{
	CBCGPButton::OnSetFocus(pOldWnd);
	RedrawWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropSheetPane

BOOL CBCGPPropSheetPane::OnSendCommand (const CBCGPToolbarButton* pButton)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParent);

	int nPageIndex = ButtonToIndex(pButton);

	if (m_pParent->IsPageTransitionAvailable())
	{
		m_pParent->SetActivePageWithEffects(nPageIndex);
		OnCancelMode();
	}
	else
	{
		CWaitCursor wait;
		m_pParent->SetActivePage(nPageIndex);
	}

	return TRUE;
}
//****************************************************************************************
void CBCGPPropSheetPane::EnsureVisible (int iButton)
{
	ASSERT_VALID (this);

	CBCGPToolbarButton* pButton = GetButton (iButton);
	ASSERT_VALID (pButton);

	CRect rectButton = pButton->Rect ();

	CRect rectWork;
	GetClientRect (rectWork);

	if (rectButton.Height () >= rectWork.Height ())
	{
		// Work area is too small, nothing to do
		return;
	}

	if (rectButton.top >= rectWork.top && rectButton.bottom <= rectWork.bottom)
	{
		// Already visible
		return;
	}

	if (rectButton.top < rectWork.top)
	{
		while (pButton->Rect ().top < rectWork.top)
		{
			int iScrollOffset = m_iScrollOffset;

			ScrollUp ();

			if (iScrollOffset == m_iScrollOffset)
			{
				break;
			}
		}
	}
	else
	{
		while (pButton->Rect ().bottom > rectWork.bottom)
		{
			int iScrollOffset = m_iScrollOffset;

			ScrollDown ();

			if (iScrollOffset == m_iScrollOffset)
			{
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropSheetTab

CBCGPPropSheetTab::CBCGPPropSheetTab ()
{
	m_bIsDlgControl = TRUE;
}
//*********************************************************************************
BOOL CBCGPPropSheetTab::SetActiveTab (int iTab)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pParent);

	if (m_pParent->GetActiveIndex () != iTab)
	{
		if (m_pParent->IsPageTransitionAvailable())
		{
			return m_pParent->SetActivePageWithEffects(iTab);
		}
		else 
		{
			CWaitCursor wait;

			if (!m_pParent->SetActivePage (iTab))
			{
				return FALSE;
			}
		}
	}

	CBCGPTabWnd::SetActiveTab (iTab);

	CRect rectWndArea = m_rectWndArea;
	MapWindowPoints (m_pParent, rectWndArea);

	CPropertyPage* pPage = m_pParent->GetPage (iTab);
	if (pPage != NULL)
	{
		pPage->SetWindowPos (NULL, rectWndArea.left, rectWndArea.top,
			rectWndArea.Width (), rectWndArea.Height (),
			SWP_NOACTIVATE | SWP_NOZORDER);
	}

	return TRUE;
}
//*********************************************************************************
void CBCGPPropSheetTab::OnCustomFontChanged()
{
	ASSERT_VALID (this);
	SetActiveTab(GetActiveTab());
}

//////////////////////////////////////////////////////////////////////////////
// CBCGPPropSheetCategory

IMPLEMENT_DYNAMIC(CBCGPPropSheetCategory, CObject)

CBCGPPropSheetCategory::CBCGPPropSheetCategory (LPCTSTR lpszName, int nIcon, 
											  int nSelectedIcon,
											  const CBCGPPropSheetCategory* pParentCategory,
											  CBCGPPropertySheet& propSheet) :
	m_strName (lpszName),
	m_nIcon (nIcon),
	m_nSelectedIcon (nSelectedIcon),
	m_pParentCategory ((CBCGPPropSheetCategory*) pParentCategory),
	m_propSheet (propSheet)
{
	m_hTreeItem = NULL;
	m_hLastSelectedItem = NULL;

	if (m_pParentCategory != NULL)
	{
		ASSERT_VALID (m_pParentCategory);
		m_pParentCategory->m_lstSubCategories.AddTail (this);
	}
}

CBCGPPropSheetCategory::~CBCGPPropSheetCategory()
{
	while (!m_lstSubCategories.IsEmpty ())
	{
		delete m_lstSubCategories.RemoveHead ();
	}

	if (m_propSheet.GetSafeHwnd () != NULL)
	{
		for (POSITION pos = m_lstPages.GetHeadPosition (); pos != NULL;)
		{
			CBCGPPropertyPage* pPage = m_lstPages.GetNext (pos);
			ASSERT_VALID (pPage);

			m_propSheet.RemovePage (pPage);
		}

		if (m_propSheet.m_wndTree.GetSafeHwnd () != NULL && m_hTreeItem != NULL)
		{
			m_propSheet.m_wndTree.DeleteItem (m_hTreeItem);
		}
	}

	if (m_pParentCategory != NULL)
	{
		ASSERT_VALID (m_pParentCategory);

		POSITION pos = m_pParentCategory->m_lstSubCategories.Find (this);
		if (pos != NULL)
		{
			m_pParentCategory->m_lstSubCategories.RemoveAt (pos);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropertySheet

#define UM_AFTERACTIVATEPAGE	(WM_USER + 1001)
#define UM_ADJUSTBUTTONS		(WM_USER + 1002)
#define UM_ADJUSTWIZARDPAGE		(WM_USER + 1003)

IMPLEMENT_DYNAMIC(CBCGPPropertySheet, CPropertySheet)

BOOL CBCGPPropertySheet::m_bUseOldLookInTreeMode = FALSE;

#pragma warning (disable : 4355)

CBCGPPropertySheet::CBCGPPropertySheet() :
	m_Impl (*this)
{
	CommonInit ();
}

CBCGPPropertySheet::CBCGPPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage),
	m_Impl (*this)
{
	CommonInit ();
}

CBCGPPropertySheet::CBCGPPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage),
	m_Impl (*this)
{
	CommonInit ();
}

#pragma warning (default : 4355)

void CBCGPPropertySheet::SetLook (PropSheetLook look, int nNavBarWidth, BOOL bGlassEffect)
{
	ASSERT (GetSafeHwnd () == NULL);

	m_look = look;

	m_nBarWidth = nNavBarWidth;

	m_bGlassEffect = bGlassEffect;

	if (m_look == PropSheetLook_Wizard || m_look == PropSheetLook_AeroWizard)
	{
		m_psh.dwFlags |= PSH_WIZARD;
	}

	if (m_look != PropSheetLook_Tabs)
	{
		EnableStackedTabs (FALSE);
	}

	if (m_look == PropSheetLook_AeroWizard)
	{
		m_Impl.m_lstNonSubclassedItems.AddTail(ID_WIZBACK);

		CBCGPLocalResource locaRes;

		BOOL bIsHighDPI = (globalData.GetRibbonImageScale() >= 1.2) && (globalData.m_nBitsPerPixel > 8) && !globalData.IsHighContastMode ();

		const CSize sizeImage = bIsHighDPI ? CSize(32, 32) : CSize(25, 25);

		m_NavImages16.SetImageSize (sizeImage);
		m_NavImages16.SetTransparentColor (globalData.clrBtnFace);
		
		m_NavImages16.Load (IDB_BCGBARRES_NAV_BUTTONS_16);
		
		m_NavImages.SetImageSize (sizeImage);
		m_NavImages.Load (CBCGPVisualManager::GetInstance()->GetNavButtonsID(bIsHighDPI));

		m_nAeroHeight = m_NavImages.GetImageSize ().cy * 3 / 2;
	}
}

CBCGPPropertySheet::~CBCGPPropertySheet()
{
	while (!m_lstTreeCategories.IsEmpty ())
	{
		delete m_lstTreeCategories.RemoveHead ();
	}
}

void CBCGPPropertySheet::CommonInit ()
{
	m_nBarWidth = 100;
	m_nActivePage = -1;
	m_look = PropSheetLook_Tabs;
	m_bIsInSelectTree = FALSE;
	m_bAlphaBlendIcons = FALSE;
	m_nHeaderHeight = 0;
	m_bDrawHeaderOnAeroCaption = FALSE;
	m_nAeroHeight = 0;
	m_bWasMaximized = FALSE;
	m_bDrawPageFrame = FALSE;
	m_bGlassArea = FALSE;
	m_bGlassEffect = TRUE;
	m_bIsReady = FALSE;
	m_sizeOriginal = m_sizePrev = CSize(0, 0);
	m_bInAdjustLayout = FALSE;
	m_bBackstageMode = FALSE;
	m_bIsTabsScrolling = FALSE;
	m_nNewActivePage = -1;
}

BEGIN_MESSAGE_MAP(CBCGPPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CBCGPPropertySheet)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SETTINGCHANGE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_NCPAINT()
	ON_WM_NCMOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_NCHITTEST()
	ON_WM_NCCALCSIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_ERASEBKGND()
	ON_WM_GETMINMAXINFO()
	ON_WM_CTLCOLOR()
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	ON_WM_ACTIVATE()
	ON_WM_NCACTIVATE()
	ON_WM_ENABLE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_MESSAGE(UM_AFTERACTIVATEPAGE, OnAfterActivatePage)
	ON_MESSAGE(UM_ADJUSTBUTTONS, OnAdjustButtons)
	ON_MESSAGE(UM_ADJUSTWIZARDPAGE, OnAdjustWizardPage)
	ON_NOTIFY(TVN_SELCHANGEDA, idTree, OnSelectTree)
	ON_NOTIFY(TVN_SELCHANGEDW, idTree, OnSelectTree)
	ON_NOTIFY(TVN_GETDISPINFOA, idTree, OnGetDispInfo)
	ON_NOTIFY(TVN_GETDISPINFOW, idTree, OnGetDispInfo)
	ON_LBN_SELCHANGE(idList, OnSelectList)
	ON_MESSAGE(WM_DWMCOMPOSITIONCHANGED, OnDWMCompositionChanged)
	ON_REGISTERED_MESSAGE(BCGM_CHANGEVISUALMANAGER, OnChangeVisualManager)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLBACKSTAGEMODE, OnBCGSetControlBackStageMode)
	ON_MESSAGE(WM_SETTEXT, OnSetText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPPropertySheet message handlers

void CBCGPPropertySheet::AddPage(CPropertyPage* pPage)
{
	CPropertySheet::AddPage (pPage);

	CBCGPPropertyPage* pBCGPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage, pPage);
	if (pBCGPage != NULL)
	{
		ASSERT_VALID (pBCGPage);
		pBCGPage->EnableVisualManagerStyle (IsVisualManagerStyle ());

		if (m_bBackstageMode)
		{
			pBCGPage->EnableBackstageMode();
		}
	}

	if (GetSafeHwnd () == NULL || m_look == PropSheetLook_Tabs)
	{
		return;
	}

	CTabCtrl* pTab = GetTabControl ();
	ASSERT_VALID (pTab);

	InternalAddPage (pTab->GetItemCount () - 1);
}
//****************************************************************************************
void CBCGPPropertySheet::AddGroup(LPCTSTR lpszGroup)
{
	ASSERT(m_look == PropSheetLook_List);
	ASSERT(lpszGroup != NULL);

	m_arGroupCaptions.Add(lpszGroup);
	m_arGroupIndexes.Add(GetPageCount());
}
//****************************************************************************************
void CBCGPPropertySheet::InternalAddPage (int nTab)
{
	CTabCtrl* pTab = GetTabControl ();
	ASSERT_VALID (pTab);

	TCHAR szTab [256];

	TCITEM item;
	item.mask = TCIF_TEXT;
	item.cchTextMax = 255;
	item.pszText = szTab;

	pTab->GetItem (nTab, &item);

	if (m_wndPane1.GetSafeHwnd () != NULL)
	{
		HICON hIcon = m_Icons.ExtractIcon (nTab);
		m_wndPane1.AddButton (hIcon, szTab, 0, -1, m_bAlphaBlendIcons);
		if (hIcon != NULL)
		{
			::DestroyIcon (hIcon);
		}	
	}

	if (m_wndTree.GetSafeHwnd () != NULL)
	{
		CBCGPPropertyPage* pPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage, GetPage (nTab));
		if (pPage == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		HTREEITEM hParent = NULL;
		if (pPage->m_pCategory != NULL)
		{
			ASSERT_VALID (pPage->m_pCategory);
			hParent = pPage->m_pCategory->m_hTreeItem;
		}

		HTREEITEM hTreeItem = m_wndTree.InsertItem (szTab, 
			I_IMAGECALLBACK, I_IMAGECALLBACK, hParent);
		m_wndTree.SetItemData (hTreeItem, (DWORD_PTR) pPage);
		pPage->m_hTreeNode = hTreeItem;
	}

	if (m_wndList.GetSafeHwnd () != NULL)
	{
		CBCGPPropertyPage* pPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage, GetPage (nTab));
		if (pPage == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		int nCaptionsBefore = 0;

		for (int i = 0; i < m_arGroupIndexes.GetSize(); i++)
		{
			if (m_arGroupIndexes[i] == nTab)
			{
				m_wndList.AddCaption(m_arGroupCaptions[i]);
			}

			if (m_arGroupIndexes[i] < m_wndList.GetCount())
			{
				nCaptionsBefore++;
			}
		}
		
		int nIndex = m_wndList.AddString (szTab);
		m_wndList.SetItemData (nIndex, (DWORD_PTR) pPage);

		m_wndList.SetItemImage(nIndex, nIndex - nCaptionsBefore);
	}

	if (m_wndTab.GetSafeHwnd () != NULL)
	{
		CBCGPPropertyPage* pPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage, GetPage (nTab));
		if (pPage == NULL)
		{
			ASSERT (FALSE);
			return;
		}

		UINT uiImage = m_Icons.GetSafeHandle () == NULL ? (UINT)-1 : nTab;

		m_wndTab.AddTab (pPage, szTab, uiImage, FALSE);
	}
}
//****************************************************************************************
void CBCGPPropertySheet::RemovePage(CPropertyPage* pPage)
{
	int nPage = GetPageIndex (pPage);
	ASSERT (nPage >= 0);

	CPropertySheet::RemovePage (nPage);

	if (m_wndPane1.GetSafeHwnd () != NULL)
	{
#ifndef _BCGSUITE_
		m_wndPane1.RemoveButtonByIndex(nPage);
#else
		m_wndPane1.RemoveButton (nPage);
#endif
	}

	if (m_wndTree.GetSafeHwnd () != NULL)
	{
		if (!OnRemoveTreePage (pPage))
		{
			return;
		}
	}

	if (m_wndList.GetSafeHwnd () != NULL)
	{
		m_wndList.DeleteString (FindPageIndexInList (pPage));
	}
}
//****************************************************************************************
void CBCGPPropertySheet::RemovePage(int nPage)
{
	if (m_wndTree.GetSafeHwnd () != NULL)
	{
		if (!OnRemoveTreePage (GetPage(nPage)))
		{
			return;
		}
	}
	
	if (m_wndList.GetSafeHwnd () != NULL)
	{
		m_wndList.DeleteString (FindPageIndexInList (GetPage (nPage)));
	}
	
	CPropertySheet::RemovePage (nPage);

	if (m_wndPane1.GetSafeHwnd () != NULL)
	{
#ifndef _BCGSUITE_
		m_wndPane1.RemoveButtonByIndex(nPage);
#else
		m_wndPane1.RemoveButton (nPage);
#endif
	}
}
//********************************************************************************
void CBCGPPropertySheet::RenamePage(CPropertyPage* pPage, const CString& strPageName)
{
	ASSERT_VALID(pPage);

	int nPage = GetPageIndex(pPage);
	if (nPage < 0)
	{
		ASSERT(FALSE);
		return;
	}

	RenamePage(nPage, strPageName);
}
//********************************************************************************
void CBCGPPropertySheet::RenamePage(int nPage, const CString& strPageName)
{
	if (nPage < 0 || nPage >= GetPageCount())
	{
		ASSERT(FALSE);
		return;
	}

	CTabCtrl* pTab = GetTabControl();
	if (pTab != NULL)
	{
		ASSERT_VALID(pTab);

		TC_ITEM ti;
		memset(&ti, 0, sizeof(ti));

		ti.mask = TCIF_TEXT;
		
		CString str = strPageName;
		ti.pszText = str.GetBuffer (str.GetLength());
	
		pTab->SetItem(nPage, &ti);
	}

	if (m_wndTab.GetSafeHwnd() != NULL)
	{
		m_wndTab.SetTabLabel(nPage, strPageName);
		return;
	}

	if (m_wndList.GetSafeHwnd() != NULL)
	{
		BOOL bIsSelected = m_wndList.GetCurSel() == nPage;

		m_wndList.SetRedraw(FALSE);
		m_wndList.DeleteString(nPage);

		m_wndList.InsertString(nPage, strPageName);
		m_wndList.SetItemData(nPage, (DWORD_PTR) GetPage(nPage));

		if (bIsSelected)
		{
			m_wndList.SetCurSel(nPage);
		}
		
		m_wndList.SetRedraw(TRUE);
		m_wndList.RedrawWindow();

		return;
	}

	if (m_wndTree.GetSafeHwnd() != NULL)
	{
		CBCGPPropertyPage* pBCGPropPage = DYNAMIC_DOWNCAST(CBCGPPropertyPage, GetPage(nPage));
		if (pBCGPropPage != NULL)
		{
			m_wndTree.SetItemText(pBCGPropPage->m_hTreeNode, strPageName);
		}

		return;
	}

	if (m_wndPane1.GetSafeHwnd() != NULL)
	{
		m_wndPane1.SetButtonText(nPage, strPageName);
		m_wndPane1.AdjustLayout();
		m_wndPane1.RedrawWindow();
		return;
	}

	if (m_look == PropSheetLook_Wizard || m_look == PropSheetLook_AeroWizard)
	{
		if (GetActiveIndex() == nPage)
		{
			SetWindowText(strPageName);
		}
	}
}
//********************************************************************************
void CBCGPPropertySheet::RemoveCategory (CBCGPPropSheetCategory* pCategory)
{
	ASSERT_VALID (pCategory);

	POSITION pos = m_lstTreeCategories.Find (pCategory);
	if (pos != NULL)
	{
		m_lstTreeCategories.RemoveAt (pos);
	}

	delete pCategory;
}
//****************************************************************************************
CBCGPPropSheetCategory* CBCGPPropertySheet::AddTreeCategory (LPCTSTR lpszLabel, 
	int nIconNum, int nSelectedIconNum, const CBCGPPropSheetCategory* pParentCategory)
{
	ASSERT_VALID (this);
	ASSERT (m_look == PropSheetLook_Tree);

	if (nSelectedIconNum == -1)
	{
		nSelectedIconNum = nIconNum;
	}

	CBCGPPropSheetCategory* pCategory = new CBCGPPropSheetCategory (
		lpszLabel, nIconNum, nSelectedIconNum,
		pParentCategory, *this);

	if (m_wndTree.GetSafeHwnd () != NULL)
	{
		HTREEITEM hParent = NULL;
		if (pParentCategory != NULL)
		{
			hParent = pParentCategory->m_hTreeItem;
		}

		pCategory->m_hTreeItem = m_wndTree.InsertItem (
			lpszLabel, I_IMAGECALLBACK, I_IMAGECALLBACK, hParent);
		m_wndTree.SetItemData (pCategory->m_hTreeItem, (DWORD_PTR) pCategory);
	}

	if (pParentCategory == NULL)
	{
		m_lstTreeCategories.AddTail (pCategory);
	}

	return pCategory;
}
//***************************************************************************************
void CBCGPPropertySheet::AddPageToTree (CBCGPPropSheetCategory* pCategory, 
									   CBCGPPropertyPage* pPage, int nIconNum,
									   int nSelIconNum)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pPage);
	ASSERT (m_look == PropSheetLook_Tree);

	if (pCategory != NULL)
	{
		ASSERT_VALID (pCategory);
		pCategory->m_lstPages.AddTail (pPage);
	}

	pPage->m_pCategory = pCategory;
	pPage->m_nIcon = nIconNum;
	pPage->m_nSelIconNum = nSelIconNum;

	CPropertySheet::AddPage (pPage);

	CBCGPPropertyPage* pBCGPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage, pPage);
	if (pBCGPage != NULL)
	{
		ASSERT_VALID (pBCGPage);
		pBCGPage->EnableVisualManagerStyle (IsVisualManagerStyle ());
	}

	if (GetSafeHwnd () != NULL)
	{
		CTabCtrl* pTab = GetTabControl ();
		ASSERT_VALID (pTab);

		InternalAddPage (pTab->GetItemCount () - 1);
	}
}
//****************************************************************************************
BOOL CBCGPPropertySheet::OnInitDialog() 
{
	if (AfxGetMainWnd() == this)
	{
		globalData.m_bIsRTL = (GetExStyle() & WS_EX_LAYOUTRTL);
		CBCGPToolBarImages::EnableRTL(globalData.m_bIsRTL);
	}
	
	BOOL bResult = CPropertySheet::OnInitDialog();

	m_Impl.m_bHasBorder = (GetStyle () & WS_BORDER) != 0;
	m_Impl.m_bHasCaption = (GetStyle() & WS_CAPTION) != 0;

	BOOL bIsCtrl = (GetStyle() & WS_CHILD) != 0;

	CWnd* pWndNavigator = InitNavigationControl ();

	if (IsVisualManagerStyle ())
	{
		if (m_psh.dwFlags & PSH_WIZARD97)
		{
			m_Impl.m_lstNonSubclassedWndClasses.AddTail(_T("Static"));
		}

		m_Impl.EnableVisualManagerStyle (TRUE, IsVisualManagerNCArea ());
	}

	if (m_look != PropSheetLook_AeroWizard)
	{
		m_bDrawHeaderOnAeroCaption = FALSE;
	}

	if (m_nHeaderHeight == 0 && ((m_psh.dwFlags & PSH_WIZARD) != 0) && ((m_psh.dwFlags & PSH_WIZARD97) == 0) && IsVisualManagerStyle ())
	{
		m_nHeaderHeight = 1;
	}

	if (m_wndTab.GetSafeHwnd () != NULL)
	{
#ifdef _BCGSUITE_
		if (IsVisualManagerStyle ())
		{
			m_wndTab.m_bIsDlgControl = FALSE;
		}

		const int nExtraHeight = m_nHeaderHeight;
#else
		m_wndTab.m_bIsPropertySheetTab = TRUE;
		const int nExtraHeight = m_nHeaderHeight + m_wndTab.GetPointerAreaHeight();
#endif

		if (nExtraHeight > 0)
		{
			CRect rectWindow;
			GetWindowRect (rectWindow);

			SetWindowPos (NULL, -1, -1, rectWindow.Width (),
				rectWindow.Height () + nExtraHeight,
				SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}

		CTabCtrl* pTab = GetTabControl ();
		ASSERT_VALID (pTab);

		CRect rectTab;
		pTab->GetWindowRect (rectTab);
		ScreenToClient (rectTab);

		rectTab.InflateRect (2, 0);

		rectTab.bottom += nExtraHeight;

		m_wndTab.MoveWindow (rectTab);

		pTab->ModifyStyle (WS_TABSTOP, 0);
		pTab->ShowWindow (SW_HIDE);

		if (pTab->GetItemCount () > 0)
		{
			m_wndTab.SetActiveTab (GetActiveIndex ());
		}

		if (nExtraHeight > 0)
		{
			ReposButtons ();
		}
	}
	else
	{
		if (pWndNavigator != NULL)
		{
			CTabCtrl* pTab = GetTabControl ();
			ASSERT_VALID (pTab);

			pTab->ModifyStyle (WS_TABSTOP, 0);

			CRect rectTabItem;
			pTab->GetItemRect (0, rectTabItem);
			pTab->MapWindowPoints (this, &rectTabItem);

			const int nVertMargin = bIsCtrl ? 0 : 5;
			const int nTabsHeight = rectTabItem.Height () + nVertMargin;

			CRect rectClient;
			GetClientRect (rectClient);

			if (m_nBarWidth < 0)
			{
				// Calculate according to caption widths:
				m_nBarWidth = CalcNavBarWidth();
			}

			if (!bIsCtrl)
			{
				SetWindowPos (NULL, -1, -1, rectClient.Width () + m_nBarWidth,
					rectClient.Height () - nTabsHeight + 4 * nVertMargin + m_nHeaderHeight,
					SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
			}
			
			GetClientRect (rectClient);
			pTab->MoveWindow (m_nBarWidth, -nTabsHeight, rectClient.right, rectClient.bottom - 2 * nVertMargin);

			if (IsVisualManagerStyle ())
			{
				pTab->ShowWindow (SW_HIDE);

				if (!m_bBackstageMode)
				{
					m_bDrawPageFrame = TRUE;
				}
			}

			CRect rectTab;
			pTab->GetWindowRect (rectTab);
			ScreenToClient (rectTab);

			CRect rectNavigator = rectClient;
			rectNavigator.right = rectNavigator.left + m_nBarWidth;

			if (!bIsCtrl)
			{
				rectNavigator.bottom = rectTab.bottom;
			}

			rectNavigator.DeflateRect (1, 1);

			if (m_look == PropSheetLook_List && !bIsCtrl && IsVisualManagerStyle ())
			{
				rectNavigator.bottom--;
			}

			if (m_look == PropSheetLook_Tree)
			{
				if (IsVisualManagerStyle ())
				{
					rectNavigator.bottom--;
					rectNavigator.DeflateRect (1, 1);
				}
				else
				{
					rectNavigator.bottom++;
				}
			}
			
			if (m_look == PropSheetLook_OutlookBar && IsVisualManagerStyle ())
			{
				rectNavigator.bottom -= 2;
			}

			if (m_bBackstageMode)
			{
				rectNavigator.left += 10;
			}

			pWndNavigator->SetWindowPos (&wndTop, 
								rectNavigator.left, rectNavigator.top,
								rectNavigator.Width (), 
								rectNavigator.Height (),
								SWP_NOACTIVATE);

			SetActivePage (GetActivePage ());

			ReposButtons ();
		}
		else if (m_nHeaderHeight > 0 || m_look == PropSheetLook_AeroWizard)
		{
			int nExtraHeight = m_nHeaderHeight;

			if (m_look != PropSheetLook_AeroWizard)
			{
				m_bDrawHeaderOnAeroCaption = FALSE;
			}

			if (m_look == PropSheetLook_AeroWizard)
			{
				if (m_bDrawHeaderOnAeroCaption && m_nHeaderHeight > 0)
				{
					m_nAeroHeight = max(m_nAeroHeight, m_nHeaderHeight);
					nExtraHeight = m_nAeroHeight;
				}
				else
				{
					nExtraHeight += m_nAeroHeight;
				}

#ifndef _BCGSUITE_
				if (CBCGPVisualManager::GetInstance()->IsDWMCaptionSupported())
#endif
				{
#if _MSC_VER < 1700 || !defined(_BCGSUITE_)
					if (m_bGlassEffect && globalData.DwmIsCompositionEnabled ())
#else
					BOOL bIsDWMEnabled = FALSE;
					DwmIsCompositionEnabled(&bIsDWMEnabled);

					if (m_bGlassEffect && bIsDWMEnabled)
#endif
					{
						CRect rectWindow;
						GetWindowRect (rectWindow);

						BCGPMARGINS margins;
						margins.cxLeftWidth = 0;
						margins.cxRightWidth = 0;
						margins.cyTopHeight = m_nAeroHeight;
						margins.cyBottomHeight = 0;

#if _MSC_VER < 1700 || !defined(_BCGSUITE_)
						if (globalData.DwmExtendFrameIntoClientArea (GetSafeHwnd (), &margins))
#else
						if (SUCCEEDED(DwmExtendFrameIntoClientArea (GetSafeHwnd (), &margins)))
#endif
						{
							m_bGlassArea = TRUE;
						}
					}
				}
			}

			CRect rectWindow;
			GetWindowRect (rectWindow);

			const int nVertMargin = (m_look == PropSheetLook_AeroWizard) ? 5 : 0;

			if (!bIsCtrl)
			{
				SetWindowPos (NULL, -1, -1, rectWindow.Width (),
					rectWindow.Height () + nExtraHeight - 2 * nVertMargin,
					SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
			}

			CTabCtrl* pTab = GetTabControl ();
			ASSERT_VALID (pTab);

			if (m_look == PropSheetLook_AeroWizard)
			{
				pTab->ShowWindow(SW_HIDE);
			}
			else
			{
				CRect rectTab;
				pTab->GetWindowRect (rectTab);

				pTab->SetWindowPos (NULL, -1, -1, rectTab.Width (),
					rectTab.Height () + m_nHeaderHeight,
					SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
			}

			SetActivePage (GetActivePage ());

			ReposButtons ();
		}
	}

	CRect rectClient;
	GetClientRect (rectClient);

	m_sizeOriginal = m_sizePrev = rectClient.Size ();

	if (GetLayout () != NULL)
	{
		CRect rectWindow;
		GetWindowRect (rectWindow);

		m_Impl.m_LayoutMMI.ptMinTrackSize.x = rectWindow.Width ();
		m_Impl.m_LayoutMMI.ptMinTrackSize.y = rectWindow.Height ();

		GetLayout ()->SetMinSize (m_sizePrev);
	}

	m_bIsReady = TRUE;
	return bResult;
}
//***************************************************************************************
void CBCGPPropertySheet::AdjustControlsLayout()
{
	const int nHorzMargin = 5;
	const int nVertMargin = 5;

	m_Impl.AdjustControlsLayout();

	if (!IsLayoutEnabled() || !m_bIsReady)
	{
		return;
	}

	m_bInAdjustLayout = TRUE;

	CRect rectClient;
	GetClientRect(rectClient);

	ReposButtons (TRUE);

	CSize sizeNew = rectClient.Size ();

	const int dx = sizeNew.cx - m_sizePrev.cx;
	const int dy = sizeNew.cy - m_sizePrev.cy;

	CWnd* pWndNavigator = NULL;

	if (m_wndOutlookBar.GetSafeHwnd() != NULL)
	{
		pWndNavigator = &m_wndOutlookBar;
	}
	else if (m_wndTree.GetSafeHwnd() != NULL)
	{
		pWndNavigator = &m_wndTree;
	}
	else if (m_wndList.GetSafeHwnd() != NULL)
	{
		pWndNavigator = &m_wndList;
	}

	if (pWndNavigator->GetSafeHwnd() != NULL && dy != 0)
	{
		CRect rectNavigator;
		pWndNavigator->GetWindowRect (rectNavigator);

		pWndNavigator->SetWindowPos(NULL, -1, -1, 
			rectNavigator.Width(), 
			rectNavigator.Height() + dy, 
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	CWnd* pWndTab = GetTabControl ();

	if (m_wndTab.GetSafeHwnd() != NULL && m_wndTab.IsWindowVisible())
	{
		pWndTab = &m_wndTab;
	}

	LockWindowUpdate();

	CPropertyPage* pPage = GetActivePage ();

	if (pPage != NULL)
	{
		BOOL bIsCtrl = (GetStyle() & WS_CHILD) != 0;

		if (bIsCtrl)
		{
			m_bIsReady = FALSE;
		}

		SetActivePage(pPage);
	}

	BOOL bIsResizableWizard = (m_psh.dwFlags & PSH_WIZARD) && IsLayoutEnabled();

	if (bIsResizableWizard)
	{
		OnAdjustWizardPage(0, 0);
	}
	else if (pWndTab->GetSafeHwnd() != NULL)
	{
		ASSERT_VALID (pWndTab);
		
		CRect rectTab;
		pWndTab->GetWindowRect (rectTab);
		
		pWndTab->SetWindowPos(NULL, -1, -1, 
			rectTab.Width() + dx, 
			rectTab.Height() + dy, 
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		
		if (pPage->GetSafeHwnd () != NULL)
		{
			if (m_wndTab.GetSafeHwnd() != NULL)
			{
				m_wndTab.SetActiveTab (GetPageIndex (pPage));
			}
			else
			{
				SetActivePage(pPage);
			}
		}
	}

	UnlockWindowUpdate();

	m_sizePrev = sizeNew;

	if (m_bDrawPageFrame && pPage->GetSafeHwnd () != NULL)
	{
		CRect rectFrame;
		pPage->GetWindowRect (rectFrame);
		ScreenToClient (&rectFrame);

		CRect rectRedraw = rectClient;
		rectRedraw.top = rectFrame.bottom - dy;

		RedrawWindow (rectRedraw, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ERASENOW);

		rectFrame.InflateRect(nHorzMargin, nVertMargin);
		RedrawWindow (rectFrame, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ERASENOW);
	}

	m_bInAdjustLayout = FALSE;
}
//***************************************************************************************
int CBCGPPropertySheet::ReposButtons (BOOL bRedraw)
{
	const int nHorzMargin = 5;
	const int nVertMargin = 5;

	int nButtonsHeight = 0;

	CRect rectClient;
	GetClientRect (rectClient);

	int ids[] = { IDOK, ID_WIZBACK, ID_WIZNEXT, ID_WIZFINISH, IDCANCEL, ID_APPLY_NOW, IDHELP };

	int nTotalButtonsWidth = 0;

	for (int iStep = 0; iStep < 2; iStep++)
	{
		for (int i = 0; i < sizeof (ids) / sizeof (ids [0]); i++)
		{
			CWnd* pButton = GetDlgItem (ids[i]);

			if (pButton != NULL)
			{
				if (ids [i] == IDHELP && (m_psh.dwFlags & PSH_HASHELP) == 0)
				{
					continue;
				}

				if (ids [i] == ID_APPLY_NOW && (m_psh.dwFlags & PSH_NOAPPLYNOW))
				{
					continue;
				}

				if (m_nAeroHeight > 0 && ids [i] == ID_WIZBACK)
				{
					if (m_btnBack.GetSafeHwnd() == NULL)
					{
						m_btnBack.SubclassWindow(pButton->GetSafeHwnd());
		
						CBCGPLocalResource locaRes;

						CString strTooltip;
						strTooltip.LoadString(ID_BCGBARRES_TASKPANE_BACK);

						m_btnBack.SetTooltip(strTooltip);
						m_btnBack.SetWindowText(_T(""));
					}

					m_btnBack.m_bOnGlass = m_bGlassArea;
					CSize sizeImage = m_NavImages.GetImageSize ();

					m_btnBack.SetWindowPos (NULL, sizeImage.cx / 4, (m_nAeroHeight - sizeImage.cy) / 2,
						sizeImage.cx, sizeImage.cy, SWP_NOACTIVATE | SWP_NOZORDER);
					m_btnBack.ModifyStyle (BS_DEFPUSHBUTTON, BS_OWNERDRAW);
				}
				else
				{
					CRect rectButton;
					pButton->GetWindowRect (rectButton);
					ScreenToClient (rectButton);

					if (iStep == 0)
					{
						// Align buttons at the bottom
						pButton->SetWindowPos (&wndTop, rectButton.left, 
							rectClient.bottom - rectButton.Height () - nVertMargin, 
							-1, -1, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

						nTotalButtonsWidth = max(nTotalButtonsWidth, rectButton.right);
						nButtonsHeight = max(nButtonsHeight, rectButton.Height ());
					}
					else
					{
						// Right align the buttons
						pButton->SetWindowPos (&wndTop, 
							rectButton.left + rectClient.right - nTotalButtonsWidth - nHorzMargin,
							rectButton.top,
							-1, -1, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
					}
				}

				if (bRedraw)
				{
					pButton->RedrawWindow();
				}
			}
		}
	}

	return nButtonsHeight;
}
//********************************************************************************************************
CWnd* CBCGPPropertySheet::InitNavigationControl ()
{
	ASSERT_VALID (this);

	if (m_psh.dwFlags & PSH_WIZARD)
	{
		return NULL;
	}

	CTabCtrl* pTab = GetTabControl ();
	ASSERT_VALID (pTab);

	if (m_look == PropSheetLook_OutlookBar)
	{
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_ALIGN_LEFT;
		DWORD dwBCGStyle = 0;
		m_wndOutlookBar.Create (_T(""), this, CRect (0, 0, 100, 100), 
			AFX_IDW_TOOLBAR, dwStyle, dwBCGStyle);

#ifndef _BCGSUITE_
		CBCGPBaseTabWnd* pWndTab = m_wndOutlookBar.GetUnderlinedWindow ();
		
		CBCGPOutlookWnd* pOutlookWnd = DYNAMIC_DOWNCAST(CBCGPOutlookWnd, pWndTab);
		if (pOutlookWnd != NULL)
		{
			pOutlookWnd->EnableBottomLine(FALSE);
		}

#else
		CBCGPBaseTabWnd* pWndTab = m_wndOutlookBar.GetUnderlyingWindow ();
#endif

		ASSERT_VALID (pWndTab);

		pWndTab->HideSingleTab ();

		m_wndPane1.Create (&m_wndOutlookBar, dwDefaultToolbarStyle, 1);
		m_wndPane1.m_pParent = this;
		m_wndOutlookBar.AddTab (&m_wndPane1);

		m_wndPane1.EnableTextLabels (TRUE);
		m_wndPane1.SetOwner (this);

		ASSERT (m_Icons.GetSafeHandle () != NULL);
		ASSERT (m_Icons.GetImageCount () >= pTab->GetItemCount ());

		for (int nTab = 0; nTab < pTab->GetItemCount (); nTab++)
		{
			InternalAddPage (nTab);
		}

		return &m_wndOutlookBar;
	}

	if (m_look == PropSheetLook_Tree)
	{
		CRect rectDummy (0, 0, 0, 0);
		
		DWORD dwTreeStyle =	WS_CHILD | WS_VISIBLE;
		
		if (!m_bUseOldLookInTreeMode)
		{
			dwTreeStyle |= 0x0200 /* TVS_TRACKSELECT */ | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS;
		}

		m_wndTree.Create (dwTreeStyle, rectDummy, this, (UINT) idTree);

		if (!IsVisualManagerStyle())
		{
			m_wndTree.ModifyStyleEx (0, WS_EX_CLIENTEDGE);
		}
		else
		{
			m_wndTree.m_bVisualManagerStyle = TRUE;
		}

		if (m_Icons.GetSafeHandle () != NULL)
		{
			m_wndTree.SetImageList (&m_Icons, TVSIL_NORMAL);
			m_wndTree.SetImageList (&m_Icons, TVSIL_STATE);
		}

		//----------------
		// Add categories:
		//----------------
		for (POSITION pos = m_lstTreeCategories.GetHeadPosition (); pos != NULL;)
		{
			AddCategoryToTree (m_lstTreeCategories.GetNext (pos));
		}

		//-----------
		// Add pages:
		//-----------
		for (int nTab = 0; nTab < pTab->GetItemCount (); nTab++)
		{
			InternalAddPage (nTab);
		}

		return &m_wndTree;
	}

	if (m_look == PropSheetLook_List)
	{
		CRect rectDummy (0, 0, 0, 0);
		const DWORD dwListStyle = 
			LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VSCROLL | WS_VISIBLE | LBS_NOTIFY | WS_TABSTOP;

		m_wndList.m_bPropertySheetNavigator = TRUE;
		m_wndList.m_bBackstageMode = m_bBackstageMode;
		m_wndList.m_bVisualManagerStyle = TRUE;

		m_wndList.Create (dwListStyle, rectDummy, this, (UINT) idList);

		if ((GetStyle() & WS_CHILD) == 0)
		{
			m_wndList.ModifyStyleEx (0, WS_EX_CLIENTEDGE);
		}

		if (m_Icons.GetSafeHandle () != NULL)
		{
			m_wndList.SetImageList (m_Icons.GetSafeHandle ());
		}

		//-----------
		// Add pages:
		//-----------
		for (int nTab = 0; nTab < pTab->GetItemCount (); nTab++)
		{
			InternalAddPage (nTab);
		}

		return &m_wndList;
	}

	if (m_look == PropSheetLook_OneNoteTabs || m_look == PropSheetLook_Pointer || IsVisualManagerStyle ())
	{
		const int nActiveTab = GetActiveIndex ();

		CRect rectDummy (0, 0, 0, 0);

#ifndef _BCGSUITE_
		m_wndTab.Create (
			m_look == PropSheetLook_Pointer ? CBCGPTabWnd::STYLE_POINTER : m_look == PropSheetLook_OneNoteTabs ? CBCGPTabWnd::STYLE_3D_ONENOTE : (m_bIsTabsScrolling ? CBCGPTabWnd::STYLE_3D_SCROLLED : CBCGPTabWnd::STYLE_3D),
			rectDummy, this, (UINT) idTab, CBCGPTabWnd::LOCATION_TOP, FALSE);
#else
		m_wndTab.Create (
			m_look == PropSheetLook_OneNoteTabs ? CBCGPTabWnd::STYLE_3D_ONENOTE : (m_bIsTabsScrolling ? CBCGPTabWnd::STYLE_3D_SCROLLED : CBCGPTabWnd::STYLE_3D),
			rectDummy, this, (UINT) idTab, CBCGPTabWnd::LOCATION_TOP, FALSE);
#endif
		m_wndTab.m_pParent = this;
		m_wndTab.EnableTabSwap (FALSE);
		m_wndTab.AutoDestroyWindow (FALSE);
		m_wndTab.AutoSizeWindow(FALSE);

		if (m_Icons.GetSafeHandle () != NULL)
		{
			ASSERT (m_Icons.GetImageCount () >= pTab->GetItemCount ());
			m_wndTab.SetImageList (m_Icons.GetSafeHandle ());
		}

		for (int nTab = 0; nTab < pTab->GetItemCount (); nTab++)
		{
			InternalAddPage (nTab);
		}

		SetActivePage (nActiveTab);
		return &m_wndTab;
	}

	if (m_look == PropSheetLook_Tabs)
	{
		if (m_Icons.GetSafeHandle () != NULL)
		{
			ASSERT (m_Icons.GetImageCount () >= pTab->GetItemCount ());
			pTab->SetImageList(&m_Icons);

			TCITEM tci;
			::ZeroMemory(&tci, sizeof(tci));
			tci.mask = TCIF_IMAGE;

			for (int nTab = 0; nTab < pTab->GetItemCount (); nTab++)
			{
				tci.iImage = nTab;
				pTab->SetItem (nTab, &tci);
			}
		}
	}

	return NULL;
}
//****************************************************************************************
void CBCGPPropertySheet::SetIconsList (HIMAGELIST hIcons)
{
	ASSERT_VALID(this);
	ASSERT (hIcons != NULL);
	ASSERT (m_Icons.GetSafeHandle () == NULL);

	m_Icons.Create (CImageList::FromHandle (hIcons));
}
//******************************************************************************************
void CBCGPPropertySheet::AddCategoryToTree (CBCGPPropSheetCategory* pCategory)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pCategory);
	ASSERT (m_look == PropSheetLook_Tree);

	HTREEITEM hParent = NULL;
	if (pCategory->m_pParentCategory != NULL)
	{
		hParent = pCategory->m_pParentCategory->m_hTreeItem;
	}

	pCategory->m_hTreeItem = m_wndTree.InsertItem (pCategory->m_strName, 
		I_IMAGECALLBACK, I_IMAGECALLBACK, hParent);
	m_wndTree.SetItemData (pCategory->m_hTreeItem, (DWORD_PTR) pCategory);

	for (POSITION pos = pCategory->m_lstSubCategories.GetHeadPosition (); pos != NULL;)
	{
		AddCategoryToTree (pCategory->m_lstSubCategories.GetNext (pos));
	}
}
//***************************************************************************************
BOOL CBCGPPropertySheet::SetIconsList (UINT uiImageListResID, int cx,
							  COLORREF clrTransparent)
{
	ASSERT_VALID(this);

	LPCTSTR lpszResourceName = MAKEINTRESOURCE (uiImageListResID);
	ASSERT(lpszResourceName != NULL);

	HBITMAP hbmp = NULL;

	//-----------------------------
	// Try to load PNG image first:
	//-----------------------------
	CBCGPPngImage pngImage;
	if (pngImage.Load (lpszResourceName))
	{
		hbmp = (HBITMAP) pngImage.Detach ();
	}
	else
	{
		hbmp = (HBITMAP) ::LoadImage (
			AfxGetResourceHandle (),
			lpszResourceName,
			IMAGE_BITMAP,
			0, 0,
			LR_CREATEDIBSECTION);
	}

	if (hbmp == NULL)
	{
		TRACE(_T("Can't load image: %x\n"), uiImageListResID);
		return FALSE;
	}

	CImageList icons;
	m_bAlphaBlendIcons = FALSE;

	BITMAP bmpObj;
	::GetObject (hbmp, sizeof (BITMAP), &bmpObj);

	UINT nFlags = (clrTransparent == (COLORREF) -1) ? 0 : ILC_MASK;

	switch (bmpObj.bmBitsPixel)
	{
	case 4:
	default:
		nFlags |= ILC_COLOR4;
		break;

	case 8:
		nFlags |= ILC_COLOR8;
		break;

	case 16:
		nFlags |= ILC_COLOR16;
		break;

	case 24:
		nFlags |= ILC_COLOR24;
		break;

	case 32:
		nFlags |= ILC_COLOR32;
		m_bAlphaBlendIcons = TRUE;
		break;
	}

	icons.Create (cx, bmpObj.bmHeight, nFlags, 0, 0);
	icons.Add (CBitmap::FromHandle (hbmp), clrTransparent);

	SetIconsList (icons);

	::DeleteObject (hbmp);
	return TRUE;
}
//***************************************************************************************
void CBCGPPropertySheet::OnActivatePage (CPropertyPage* pPage)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pPage);

	if (m_wndPane1.GetSafeHwnd () != NULL && !m_bInAdjustLayout)
	{
		int nPage = GetPageIndex (pPage);
		ASSERT (nPage >= 0);

		if (m_nActivePage >= 0)
		{
			m_wndPane1.SetButtonStyle (m_nActivePage, 0);
		}

		m_nActivePage = nPage;

		PostMessage (UM_AFTERACTIVATEPAGE);
	}

	if (m_wndTree.GetSafeHwnd () != NULL && !m_bInAdjustLayout)
	{
		CBCGPPropertyPage* pBCGPropPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage, pPage);
		if (pBCGPropPage != NULL)
		{
			if (!m_bIsInSelectTree)
			{
				m_wndTree.SelectItem (pBCGPropPage->m_hTreeNode);
			}

			m_wndTree.EnsureVisible(pBCGPropPage->m_hTreeNode);
			m_wndTree.RedrawWindow();
		}
	}

	if (m_wndList.GetSafeHwnd () != NULL && !m_bInAdjustLayout)
	{
		int nIdex = FindPageIndexInList (pPage);

		m_wndList.SetCurSel (nIdex);
		PostMessage (UM_AFTERACTIVATEPAGE);
	}

	if (m_wndTab.GetSafeHwnd () != NULL && !m_bInAdjustLayout)
	{
		const int nTab = GetPageIndex (pPage);

		m_wndTab.SetActiveTab (nTab);
		m_wndTab.EnsureVisible (nTab);
	}

	BOOL bIsCtrl = (GetStyle() & WS_CHILD) != 0;

	if (m_psh.dwFlags & PSH_WIZARD)
	{
		if (m_nHeaderHeight > 0 || m_nAeroHeight > 0)
		{
			CRect rectPage;
			pPage->GetWindowRect (rectPage);
			ScreenToClient(rectPage);

			if (m_nAeroHeight > 0)
			{
				int nExtraHeight = m_bDrawHeaderOnAeroCaption ? 0 : m_nHeaderHeight;

				pPage->SetWindowPos (NULL, rectPage.left, rectPage.top + m_nAeroHeight, rectPage.Width (),
					rectPage.Height () + nExtraHeight,
					SWP_NOZORDER | SWP_NOACTIVATE);
			}
			else
			{
				pPage->SetWindowPos (NULL, -1, -1, rectPage.Width (),
					rectPage.Height () + m_nHeaderHeight,
					SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			}

			CWnd* pWndLine = GetDlgItem(12326);
			if (pWndLine != NULL)
			{
				pWndLine->ShowWindow(SW_HIDE);
			}
		}

		AdjustButtons ();
	}
	else if (bIsCtrl)
	{
		int nNavBarWidth = GetNavBarWidth();
		if (nNavBarWidth > 0)
		{
			CRect rectPage;
			GetClientRect(rectPage);

			rectPage.left += nNavBarWidth;

			m_bIsReady = TRUE;

			pPage->SetWindowPos (NULL, rectPage.left, rectPage.top, rectPage.Width (), rectPage.Height (),
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
}
//****************************************************************************************
LRESULT CBCGPPropertySheet::OnAfterActivatePage(WPARAM,LPARAM)
{
	ASSERT_VALID (this);

	if (m_nActivePage >= 0)
	{
		if (m_wndPane1.GetSafeHwnd () != NULL)
		{
			m_wndPane1.SetButtonStyle (m_nActivePage, TBBS_CHECKED);
			m_wndPane1.EnsureVisible (m_nActivePage);
		}
	}

	if (m_wndList.GetSafeHwnd () != NULL)
	{
		m_wndList.RedrawWindow ();
	}

	return 0;
}
//************************************************************************************
void CBCGPPropertySheet::OnSelectTree(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
	*pResult = 0;

	HTREEITEM hTreeItem = m_wndTree.GetSelectedItem ();
	if (hTreeItem == NULL)
	{
		return;
	}

	CBCGPPropSheetCategory* pNewCategory = NULL;
	CBCGPPropSheetCategory* pOldCategory = NULL;

	CBCGPPropertyPage* pCurrPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage,
		GetActivePage ());
	if (pCurrPage != NULL)
	{
		ASSERT_VALID (pCurrPage);
		pOldCategory = pCurrPage->m_pCategory;
	}

	m_bIsInSelectTree = TRUE;

	CBCGPPropertyPage* pPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage,
		(CObject*) m_wndTree.GetItemData (hTreeItem));

	if (pPage == pCurrPage)
	{
		m_bIsInSelectTree = FALSE;
		return;
	}

	if (pPage != NULL)
	{
		CBCGPPropertyPage* pPrevPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage, GetActivePage ());

		ASSERT_VALID (pPage);

		BOOL bIsFailed = FALSE;

		if (IsPageTransitionAvailable())
		{
			bIsFailed = !SetActivePageWithEffects(GetPageIndex(pPage));
		}
		else
		{
			bIsFailed = !SetActivePage(pPage);
		}

		if (bIsFailed)
		{
		   m_wndTree.SendMessage (TVM_SELECTITEM, (WPARAM)TVGN_CARET,  (LPARAM)pCurrPage->m_hTreeNode);
		   m_bIsInSelectTree = FALSE;
		   return;
		}

		pNewCategory = pPage->m_pCategory;
		if (pNewCategory != NULL)
		{
			HTREEITEM hLastSelectedItem = hTreeItem;

			for (CBCGPPropSheetCategory* pCategory = pNewCategory;
				pCategory != NULL; 
				pCategory = pCategory->m_pParentCategory)
			{
				pCategory->m_hLastSelectedItem = hLastSelectedItem;
				hLastSelectedItem = pCategory->m_hTreeItem;
			}
		}

		if (pPrevPage != NULL)
		{
			ASSERT_VALID (pPrevPage);

			CRect rectItem;
			m_wndTree.GetItemRect (pPrevPage->m_hTreeNode, rectItem, FALSE);
			m_wndTree.InvalidateRect (rectItem);
		}
	}
	else
	{
		CBCGPPropSheetCategory* pCategory = DYNAMIC_DOWNCAST (CBCGPPropSheetCategory,
			(CObject*) m_wndTree.GetItemData (hTreeItem));
		if (pCategory != NULL)
		{
			ASSERT_VALID (pCategory);

			BOOL bIsPageSelected = FALSE;

			while (pCategory->m_hLastSelectedItem != NULL && !bIsPageSelected)
			{
				CBCGPPropSheetCategory* pChildCategory = DYNAMIC_DOWNCAST (CBCGPPropSheetCategory,
					(CObject*) m_wndTree.GetItemData (pCategory->m_hLastSelectedItem));
				if (pChildCategory == NULL)
				{
					CBCGPPropertyPage* pPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage,
						(CObject*) m_wndTree.GetItemData (pCategory->m_hLastSelectedItem));
					if (pPage != NULL)
					{
						if (IsPageTransitionAvailable())
						{
							SetActivePageWithEffects(GetPageIndex(pPage));
						}
						else
						{
							SetActivePage (pPage);
						}

						CRect rectItem;
						m_wndTree.GetItemRect (pPage->m_hTreeNode, rectItem, FALSE);
						m_wndTree.InvalidateRect (rectItem);

						bIsPageSelected = TRUE;
					}
				}
				else
				{
					pCategory = pChildCategory;
				}
			}

			if (!bIsPageSelected)
			{
				while (!pCategory->m_lstSubCategories.IsEmpty ())
				{
					pCategory = pCategory->m_lstSubCategories.GetHead ();
					ASSERT_VALID (pCategory);
				}

				if (!pCategory->m_lstPages.IsEmpty ())
				{
					pPage = pCategory->m_lstPages.GetHead ();
					ASSERT_VALID (pPage);

					if (IsPageTransitionAvailable())
					{
						SetActivePageWithEffects(GetPageIndex(pPage));
					}
					else
					{
						SetActivePage (pPage);
					}

					CRect rectItem;
					m_wndTree.GetItemRect (pPage->m_hTreeNode, rectItem, FALSE);
					m_wndTree.InvalidateRect (rectItem);
				}
			}

			pNewCategory = pCategory;
		}
	}

	if (pNewCategory != pOldCategory)
	{
		if (pOldCategory != NULL && m_bUseOldLookInTreeMode)
		{
			ASSERT_VALID (pOldCategory);
			HTREEITEM hItem = pOldCategory->m_hTreeItem;

			do
			{
				m_wndTree.Expand (hItem, TVE_COLLAPSE);
				hItem = m_wndTree.GetParentItem (hItem);
			}
			while (hItem != NULL);
		}

		if (pNewCategory != NULL)
		{
			ASSERT_VALID (pNewCategory);
			HTREEITEM hItem = pNewCategory->m_hTreeItem;

			do
			{
				m_wndTree.Expand (hItem, TVE_EXPAND);
				hItem = m_wndTree.GetParentItem (hItem);
			}
			while (hItem != NULL);
		}
	}

	m_bIsInSelectTree = FALSE;
}
//***************************************************************************************
void CBCGPPropertySheet::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVDISPINFO lptvdi = (LPNMTVDISPINFO) pNMHDR;

	CBCGPPropertyPage* pPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage,
		(CObject*) m_wndTree.GetItemData (lptvdi->item.hItem));
	if (pPage != NULL)
	{
		ASSERT_VALID (pPage);

		if (pPage == GetActivePage ())
		{
			lptvdi->item.iImage = pPage->m_nSelIconNum;
			lptvdi->item.iSelectedImage = pPage->m_nSelIconNum;
		}
		else
		{
			lptvdi->item.iImage = pPage->m_nIcon;
			lptvdi->item.iSelectedImage = pPage->m_nIcon;
		}
	}

	CBCGPPropSheetCategory* pCategory = DYNAMIC_DOWNCAST (CBCGPPropSheetCategory,
		(CObject*) m_wndTree.GetItemData (lptvdi->item.hItem));
	if (pCategory != NULL)
	{
		ASSERT_VALID (pCategory);

		if (lptvdi->item.state & TVIS_EXPANDED)
		{
			lptvdi->item.iImage = pCategory->m_nSelectedIcon;
			lptvdi->item.iSelectedImage = pCategory->m_nSelectedIcon;
		}
		else
		{
			lptvdi->item.iImage = pCategory->m_nIcon;
			lptvdi->item.iSelectedImage = pCategory->m_nIcon;
		}
	}

	*pResult = 0;
}
//********************************************************************************
CBCGPTabWnd& CBCGPPropertySheet::GetTab () const
{
	ASSERT_VALID (this);
	return (CBCGPTabWnd&) m_wndTab;
}
//********************************************************************************
BOOL CBCGPPropertySheet::PreTranslateMessage(MSG* pMsg) 
{
	if (m_Impl.PreTranslateMessage (pMsg))
	{
		return TRUE;
	}

	switch (pMsg->message)
	{
	case WM_LBUTTONDOWN:
		if (IsDragClientAreaEnabled())
		{
			CPoint point(BCG_GET_X_LPARAM (pMsg->lParam), BCG_GET_Y_LPARAM (pMsg->lParam));
			BOOL bFireDragCaption = FALSE;

			if (pMsg->hwnd == m_wndTree.GetSafeHwnd())
			{
				bFireDragCaption = (m_wndTree.HitTest(point) == NULL);
			}
			else if (pMsg->hwnd == m_wndList.GetSafeHwnd())
			{
				bFireDragCaption = m_wndList.HitTest(point) < 0;
			}
			else if (pMsg->hwnd == m_wndTab.GetSafeHwnd())
			{
				bFireDragCaption = m_wndTab.GetTabFromPoint(point) < 0;
			}
			else if (pMsg->hwnd == m_wndPane1.GetSafeHwnd())
			{
				bFireDragCaption = m_wndPane1.HitTest(point) < 0;
			}

			if (bFireDragCaption)
			{
				CPoint ptScreen = point;
				ClientToScreen(&ptScreen);
				
				SendMessage (WM_NCLBUTTONDOWN, (WPARAM) HTCAPTION, MAKELPARAM (ptScreen.x, ptScreen.y));
				return TRUE;
			}
		}
		break;

	case WM_SYSKEYDOWN:
		if (m_btnBack.GetSafeHwnd() != NULL)
		{
			PostMessage (UM_ADJUSTBUTTONS, (WPARAM)TRUE);
		}
		break;

	case PSM_SETWIZBUTTONS:
	case PSM_SETFINISHTEXT:
		AdjustButtons ();
		break;
	}

	return CPropertySheet::PreTranslateMessage(pMsg);
}
//********************************************************************************
BOOL CBCGPPropertySheet::OnRemoveTreePage(CPropertyPage* pPage)
{
	ASSERT (m_look == PropSheetLook_Tree);

	if (pPage == NULL)
	{
		return FALSE;
	}

	CBCGPPropertyPage* pDelPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage, pPage);
	if (pDelPage == NULL)
	{
		ASSERT(!_T("DYNAMIC_DOWNCAST (CBCGPPropertyPage, pPage)"));
		return FALSE;
	}

	ASSERT (pDelPage->m_hTreeNode != NULL);

	BOOL bResult = m_wndTree.DeleteItem (pDelPage->m_hTreeNode);
	ASSERT (pDelPage->m_pCategory != NULL);

	POSITION pos = (pDelPage->m_pCategory->m_lstPages).Find(pDelPage);
	if (pos != NULL)
	{
		(pDelPage->m_pCategory->m_lstPages).RemoveAt(pos);
		bResult = TRUE;
	}

	return bResult;
}
//********************************************************************************
void CBCGPPropertySheet::OnSysColorChange() 
{
	CPropertySheet::OnSysColorChange();
	
	if (AfxGetMainWnd()->GetSafeHwnd() == GetSafeHwnd())
	{
		globalData.UpdateSysColors ();
	}
}
//********************************************************************************
void CBCGPPropertySheet::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	CPropertySheet::OnSettingChange(uFlags, lpszSection);
	
	if (AfxGetMainWnd()->GetSafeHwnd() == GetSafeHwnd())
	{
		globalData.OnSettingChange ();
	}
}
//********************************************************************************
int CBCGPPropertySheet::FindPageIndexInList (CPropertyPage* pPage)
{
	for (int i = 0; i < m_wndList.GetCount (); i++)
	{
		if ((CPropertyPage*) m_wndList.GetItemData (i) == pPage)
		{
			return i;
		}
	}

	return -1;
}
//********************************************************************************
void CBCGPPropertySheet::OnSelectList()
{
	int nCurSel = m_wndList.GetCurSel ();

	if (nCurSel < 0)
	{
		return;
	}

	CPropertyPage* pPage = (CPropertyPage*) m_wndList.GetItemData (nCurSel);
	if (pPage == NULL)
	{
		return;
	}

	ASSERT_VALID (pPage);

	BOOL bIsFailed = FALSE;
	
	if (IsPageTransitionAvailable())
	{
		bIsFailed = !SetActivePageWithEffects(GetPageIndex(pPage));
	}
	else
	{
		bIsFailed = !SetActivePage(pPage);
	}

	if (bIsFailed)
	{
		CPropertyPage* pPage = GetActivePage ();			
		if (pPage->GetSafeHwnd () != NULL)
		{
			m_wndList.SetCurSel(GetPageIndex (pPage));
		}
	}

	m_wndList.RedrawWindow ();
}
//********************************************************************************
void CBCGPPropertySheet::EnablePageHeader (int nHeaderHeight, BOOL bDrawHeaderOnAeroCaption)
{
	ASSERT (GetSafeHwnd () == NULL);

	m_nHeaderHeight = nHeaderHeight;
	m_bDrawHeaderOnAeroCaption = bDrawHeaderOnAeroCaption;
}
//********************************************************************************
void CBCGPPropertySheet::OnDrawPageHeader (CDC* /*pDC*/, int /*nPage*/, CRect /*rectHeader*/)
{
}
//********************************************************************************
void CBCGPPropertySheet::EnableVisualManagerStyle (BOOL bEnable, BOOL bNCArea)
{
	ASSERT_VALID (this);

	if (bEnable)
	{
		EnableStackedTabs (FALSE);
	}

	m_Impl.EnableVisualManagerStyle (bEnable, bEnable && bNCArea);

	for (int i = 0; i < GetPageCount (); i++)
	{
		CBCGPPropertyPage* pBCGPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage, GetPage (i));
		if (pBCGPage != NULL)
		{
			ASSERT_VALID (pBCGPage);
			pBCGPage->EnableVisualManagerStyle (bEnable);
		}
	}
}
//********************************************************************************
void CBCGPPropertySheet::OnDestroy() 
{
	m_Impl.OnDestroy ();
	CPropertySheet::OnDestroy();
}
//***************************************************************************
void CBCGPPropertySheet::OnSize(UINT nType, int cx, int cy) 
{
	if (!m_rectPageTransition.IsRectEmpty())
	{
		StopPageTransition();
		OnPageTransitionFinished();
	}

	BOOL bIsMinimized = (nType == SIZE_MINIMIZED);

	if (m_Impl.IsOwnerDrawCaption ())
	{
		CRect rectWindow;
		GetWindowRect (rectWindow);

		WINDOWPOS wndpos;
		wndpos.flags = SWP_FRAMECHANGED;
		wndpos.x     = rectWindow.left;
		wndpos.y     = rectWindow.top;
		wndpos.cx    = rectWindow.Width ();
		wndpos.cy    = rectWindow.Height ();

		m_Impl.OnWindowPosChanged (&wndpos);
	}

	m_Impl.UpdateCaption ();
	
	if (!bIsMinimized && nType != SIZE_MAXIMIZED && !m_bWasMaximized)
	{
		AdjustControlsLayout();
		return;
	}

	CPropertySheet::OnSize(nType, cx, cy);
	m_bWasMaximized = (nType == SIZE_MAXIMIZED);

	AdjustControlsLayout();
}
//**************************************************************************
void CBCGPPropertySheet::OnNcPaint() 
{
	if ((GetStyle () & WS_BORDER) && (GetStyle () & WS_CHILD) && IsVisualManagerStyle())
	{
		CBCGPVisualManager::GetInstance()->OnDrawControlBorder(this);
		return;
	}

	if (!m_Impl.OnNcPaint ())
	{
		Default ();
	}
}
//**************************************************************************
void CBCGPPropertySheet::OnNcMouseMove(UINT nHitTest, CPoint point) 
{
	m_Impl.OnNcMouseMove (nHitTest, point);
	CPropertySheet::OnNcMouseMove(nHitTest, point);
}
//**************************************************************************
void CBCGPPropertySheet::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_Impl.OnLButtonUp (point);
	CPropertySheet::OnLButtonUp(nFlags, point);
}
//**************************************************************************
void CBCGPPropertySheet::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_nAeroHeight > 0)
	{
		CRect rectClient;
		GetClientRect(rectClient);

		CRect rectAero = rectClient;
		rectAero.bottom = rectAero.top + m_nAeroHeight;

		if (rectAero.PtInRect(point))
		{
			CPoint ptScreen = point;
			ClientToScreen(&ptScreen);

			SendMessage (WM_NCLBUTTONDOWN, (WPARAM) HTCAPTION, MAKELPARAM (ptScreen.x, ptScreen.y));
			return;
		}
	}

	m_Impl.OnLButtonDown (point);
	CPropertySheet::OnLButtonDown(nFlags, point);
}
//**************************************************************************
BCGNcHitTestType CBCGPPropertySheet::OnNcHitTest(CPoint point) 
{
	BCGNcHitTestType nHit = m_Impl.OnNcHitTest (point);
	if (nHit != HTNOWHERE)
	{
		return nHit;
	}

	return CPropertySheet::OnNcHitTest(point);
}
//**************************************************************************
void CBCGPPropertySheet::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	if (!m_Impl.OnNcCalcSize (bCalcValidRects, lpncsp))
	{
		CPropertySheet::OnNcCalcSize(bCalcValidRects, lpncsp);
	}
}
//**************************************************************************
void CBCGPPropertySheet::OnMouseMove(UINT nFlags, CPoint point) 
{
	m_Impl.OnMouseMove (point);
	CPropertySheet::OnMouseMove(nFlags, point);
}
//**************************************************************************
void CBCGPPropertySheet::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	if ((lpwndpos->flags & SWP_FRAMECHANGED) == SWP_FRAMECHANGED)
	{
		m_Impl.OnWindowPosChanged (lpwndpos);
	}

	CPropertySheet::OnWindowPosChanged(lpwndpos);

#ifndef _BCGSUITE_
	if (m_Impl.m_pShadow != NULL)
	{
		m_Impl.m_pShadow->Repos();
	}
#endif
}
//**************************************************************************
LRESULT CBCGPPropertySheet::OnChangeVisualManager (WPARAM, LPARAM)
{
	m_Impl.OnChangeVisualManager ();

	if (m_wndPane1.GetSafeHwnd() != NULL)
	{
		m_wndPane1.m_iFirstVisibleButton = 0;
		m_wndPane1.m_iScrollOffset = 0;

		m_wndPane1.AdjustLocations();
		m_wndPane1.RedrawWindow();
	}

	return 0;
}
//**************************************************************************
void CBCGPPropertySheet::DrawWizardGutter(CDC* pDC)
{
	ASSERT_VALID(pDC);

	if ((m_psh.dwFlags & PSH_WIZARD) && m_nHeaderHeight + m_nAeroHeight > 0)
	{
		CWnd* pPage = GetActivePage ();
		if (pPage->GetSafeHwnd () != NULL)
		{
			CRect rectPage;
			pPage->GetWindowRect (rectPage);
			ScreenToClient (&rectPage);

			CBCGPPenSelector pen (*pDC, IsVisualManagerStyle () ? globalData.clrBarShadow : globalData.clrBtnShadow);

			int y = rectPage.bottom + rectPage.top - m_nAeroHeight;

			int x1 = rectPage.left;
			int x2 = rectPage.right;

			if (!IsVisualManagerStyle () && !globalData.IsHighContastMode() && m_nAeroHeight > 0)
			{
				CRect rectClient;
				GetClientRect(rectClient);

				x1 = rectClient.left;
				x2 = rectClient.right;

				CRect rectFill = rectClient;
				rectFill.bottom = y;

				pDC->FillSolidRect(rectFill, RGB(255, 255, 255));
			}

			pDC->MoveTo(x1, y);
			pDC->LineTo(x2, y);

			CBCGPPenSelector pen1 (*pDC, IsVisualManagerStyle () ? globalData.clrBarHilite : globalData.clrBtnHilite);

			pDC->MoveTo(x1, y + 1);
			pDC->LineTo(x2, y + 1);
		}
	}
}
//**************************************************************************
void CBCGPPropertySheet::DrawWizardAero(CDC* pDC)
{
	ASSERT_VALID(pDC);

	if (m_nAeroHeight <= 0)
	{
		return;
	}

	CRect rectClient;
	GetClientRect (rectClient);

	CRect rectAero = rectClient;
	rectAero.bottom = rectAero.top + m_nAeroHeight;

	if (m_bGlassArea)
	{
		pDC->FillSolidRect (rectAero, RGB (0, 0, 0));
	}
	else
	{
		BOOL bDrawLine = TRUE;

		if (IsVisualManagerStyle ())
		{
#ifndef _BCGSUITE_
			CBCGPVisualManager::GetInstance()->OnFillPropSheetHeaderArea(pDC, this, rectAero, bDrawLine);
#else
			pDC->FillRect(rectAero, &globalData.brBarFace);
#endif
		}
		else
		{
			pDC->FillRect(rectAero, &globalData.brBtnFace);
		}

		if (bDrawLine)
		{
			CBCGPPenSelector pen (*pDC, IsVisualManagerStyle () ? globalData.clrBarShadow : globalData.clrBtnShadow);

			int y = rectAero.bottom;

			int x1 = rectAero.left;
			int x2 = rectAero.right;

			pDC->MoveTo(x1, y);
			pDC->LineTo(x2, y);

			CBCGPPenSelector pen1 (*pDC, IsVisualManagerStyle () ? globalData.clrBarHilite : globalData.clrBtnHilite);

			pDC->MoveTo(x1, y + 1);
			pDC->LineTo(x2, y + 1);
		}
	}

	if (m_nHeaderHeight > 1 && m_bDrawHeaderOnAeroCaption)
	{
		CRect rectBtnBack;
		m_btnBack.GetWindowRect(rectBtnBack);
		ScreenToClient(&rectBtnBack);

		CRect rectHeader = rectAero;
		rectHeader.left = rectBtnBack.right + 4;

		if (GetExStyle() & WS_EX_LAYOUTRTL)
		{
			rectHeader.left += rectBtnBack.Width();
		}

		OnDrawPageHeader (pDC, GetPageIndex (GetActivePage()), rectHeader);
	}
}
//**************************************************************************
BOOL CBCGPPropertySheet::OnEraseBkgnd(CDC* pDC) 
{
	CRect rectClient;
	GetClientRect (rectClient);

#ifndef _BCGSUITE_
	if (m_bBackstageMode)
	{
		CBCGPVisualManager::GetInstance ()->OnFillRibbonBackstageForm(pDC, this, rectClient);

		CBCGPPropertyPage* pPage = DYNAMIC_DOWNCAST(CBCGPPropertyPage, GetActivePage ());
		if (pPage != NULL && !pPage->m_rectBackstageWatermark.IsRectEmpty() && !pPage->m_pBackstageWatermark == NULL)
		{
			CRect rectImage = pPage->m_rectBackstageWatermark;

			pPage->MapWindowPoints(this, rectImage);
			pPage->m_pBackstageWatermark->DrawEx(pDC, rectImage, 0);
		}
	}
	else
#endif
	{
		if (!IsVisualManagerStyle () || 
			!CBCGPVisualManager::GetInstance ()->OnFillDialog (pDC, this, rectClient))
		{
			BOOL bRes = CPropertySheet::OnEraseBkgnd(pDC);
			
			DrawWizardGutter(pDC);
			DrawWizardAero(pDC);

			return bRes;
		}
	}

	if (m_bDrawPageFrame)
	{
		COLORREF clrFrame = globalData.clrBarShadow;

		CWnd* pPage = GetActivePage ();
		if (pPage->GetSafeHwnd () != NULL)
		{
			CRect rectFrame;
			pPage->GetWindowRect (rectFrame);
			ScreenToClient (&rectFrame);

			CBCGPPenSelector pen (*pDC, clrFrame);

			pDC->MoveTo (rectClient.left, rectFrame.bottom + 1);
			pDC->LineTo (rectClient.right, rectFrame.bottom + 1);
		}

		if (m_look == PropSheetLook_Tree && m_wndTree.GetSafeHwnd () != NULL)
		{
			CRect rectTree;
			m_wndTree.GetWindowRect (rectTree);
			ScreenToClient (&rectTree);

			rectTree.InflateRect (1, 1);

			pDC->Draw3dRect (rectTree, clrFrame, clrFrame);
		}
	}

	DrawWizardGutter(pDC);
	DrawWizardAero(pDC);

	return TRUE;
}
//**************************************************************************
void CBCGPPropertySheet::OnApply ()
{
	AdjustButtons ();
}
//**************************************************************************
LRESULT CBCGPPropertySheet::OnAdjustButtons(WPARAM wp, LPARAM)
{
	static int arButtons [] = 
	{
		ID_APPLY_NOW,
		ID_WIZBACK,
		ID_WIZNEXT,
		ID_WIZFINISH,
		IDOK,
		IDCANCEL,
		IDHELP
	};

	BOOL bBackBtnOnly = (BOOL)wp;

	if (bBackBtnOnly)
	{
		m_btnBack.RedrawWindow ();
		return 0;
	}

	for (int i = 0; i < sizeof (arButtons) / sizeof (int); i++)
	{
		CBCGPButton* pBtn = DYNAMIC_DOWNCAST (CBCGPButton, GetDlgItem (arButtons [i]));
		if (pBtn->GetSafeHwnd () != NULL)
		{
			ASSERT_VALID (pBtn);

			pBtn->ModifyStyle (BS_DEFPUSHBUTTON, BS_OWNERDRAW);
			pBtn->RedrawWindow ();
		}
	}

	CRect rectAero;
	rectAero.SetRectEmpty();

	if (m_bDrawHeaderOnAeroCaption)
	{
		GetClientRect(&rectAero);
		rectAero.bottom = rectAero.top + m_nHeaderHeight;
	}

	RedrawWindow (m_bDrawHeaderOnAeroCaption ? &rectAero : NULL, NULL, 
		m_bDrawHeaderOnAeroCaption ? 
		RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ERASENOW :
		RDW_FRAME | RDW_UPDATENOW);
	return 0;
}
//**************************************************************************
LRESULT CBCGPPropertySheet::OnAdjustWizardPage(WPARAM, LPARAM)
{
	if (m_psh.dwFlags & PSH_WIZARD)
	{
		CPropertyPage* pPage = GetActivePage();
		if (pPage->GetSafeHwnd() != NULL)
		{
			CRect rectPage;
			pPage->GetWindowRect(rectPage);
			
			CRect rectWindow;
			GetClientRect(rectWindow);
			
			pPage->SetWindowPos(NULL, -1, -1, 
				rectPage.Width() + rectWindow.Width() - m_sizeOriginal.cx, 
				rectPage.Height() + rectWindow.Height() - m_sizeOriginal.cy, 
				SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
		}
	}

	return 0;
}
//**************************************************************************
void CBCGPPropertySheet::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	m_Impl.OnGetMinMaxInfo (lpMMI);
	CPropertySheet::OnGetMinMaxInfo(lpMMI);
}
//****************************************************************************
LRESULT CBCGPPropertySheet::OnSetText(WPARAM, LPARAM) 
{
	LRESULT	lRes = Default();

	if (lRes && IsVisualManagerStyle () && IsVisualManagerNCArea ())
	{
		RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
	}

	return lRes;
}
//****************************************************************************
void CBCGPPropertySheet::AdjustButtons ()
{
	if (IsVisualManagerStyle () || m_bDrawHeaderOnAeroCaption)
	{
		PostMessage (UM_ADJUSTBUTTONS);
	}
}
//****************************************************************************
HBRUSH CBCGPPropertySheet::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	if (m_bDrawHeaderOnAeroCaption)
	{
		HBRUSH hbr = m_Impl.OnCtlColor (pDC, pWnd, nCtlColor);
		if (hbr != NULL)
		{
			return hbr;
		}
	}	
	
	return CPropertySheet::OnCtlColor(pDC, pWnd, nCtlColor);
}
//*****************************************************************************************
void CBCGPPropertySheet::EnableLayout(BOOL bEnable, CRuntimeClass* pRTC)
{
	m_Impl.EnableLayout(bEnable, pRTC, FALSE);
}
//****************************************************************************
void CBCGPPropertySheet::EnableDragClientArea(BOOL bEnable)
{
	m_Impl.EnableDragClientArea(bEnable);
}
//*****************************************************************************************
void CBCGPPropertySheet::EnableTabsScrolling(BOOL bEnable)
{
	m_bIsTabsScrolling = bEnable;
}
//*****************************************************************************************
void CBCGPPropertySheet::EnablePageTransitionEffect(CBCGPPageTransitionManager::BCGPPageTransitionEffect effect, int nTime)
{
	SetPageTransitionEffect(effect, nTime);
}
//*****************************************************************************************
BOOL CBCGPPropertySheet::OnWizardChangePageWidthTransitionEffect(CBCGPPropertyPage* pCurrPage, BOOL bNext)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pCurrPage);

	int nCurrPageIndex = GetPageIndex(pCurrPage);
	ASSERT(nCurrPageIndex >= 0);

	return SetActivePageWithEffects(bNext ? nCurrPageIndex + 1 : nCurrPageIndex - 1);
}
//*****************************************************************************************
BOOL CBCGPPropertySheet::SetActivePageWithEffects(int nPage)
{
	if (GetSafeHwnd() == NULL || GetActivePage()->GetSafeHwnd() == NULL || !IsPageTransitionAvailable())
	{
		return SetActivePage(nPage);
	}

	int nCurrPage = GetActiveIndex();

	int nLeftPaneWidth = 0;

	CWnd* pWndNavigator = NULL;
	
	if (m_wndOutlookBar.GetSafeHwnd() != NULL)
	{
		pWndNavigator = &m_wndOutlookBar;
	}
	else if (m_wndTree.GetSafeHwnd() != NULL)
	{
		pWndNavigator = &m_wndTree;
	}
	else if (m_wndList.GetSafeHwnd() != NULL)
	{
		pWndNavigator = &m_wndList;
	}
	
	if (pWndNavigator->GetSafeHwnd() != NULL)
	{
		CRect rectNavigator;
		pWndNavigator->GetWindowRect (rectNavigator);

		nLeftPaneWidth = rectNavigator.Width();
	}

	SetRedraw(FALSE);

	BOOL bTwoPagesTransition = 
		m_PageTransitionEffect != BCGPPageTransitionSlide && 
		m_PageTransitionEffect != BCGPPageTransitionSlideVertical;

	CArray<HWND, HWND> arPages;

	int nPage1 = min(nCurrPage, nPage);
	int nPage2 = max(nCurrPage, nPage);

	for (int i = nPage1; i <= nPage2; i++)
	{
		if (bTwoPagesTransition && (i != nPage1 && i != nPage2))
		{
			continue;
		}

		CPropertyPage* pPage = GetPage(i);
		if (pPage->GetSafeHwnd() == NULL)
		{
			SetActivePage(pPage);
		}

		arPages.Add(pPage->GetSafeHwnd());
	}

	GetActivePage()->ShowWindow(SW_HIDE);

	if (!StartPageTransition(GetSafeHwnd(), arPages, nPage < nCurrPage, 
		CSize(m_PageTransitionEffect == BCGPPageTransitionPop ? 0 : nLeftPaneWidth, 0)))
	{
		SetRedraw(TRUE);

		return SetActivePage(nPage);
	}

	if (m_wndTab.GetSafeHwnd() != NULL)
	{
		m_wndTab.RedrawWindow();
	}

	m_nNewActivePage = nPage;
	return TRUE;
}
//*****************************************************************************************
int CBCGPPropertySheet::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return m_Impl.OnCreate();
}
//*****************************************************************************************
int CBCGPPropertySheet::GetNavBarWidth() const
{
	switch (m_look)
	{
	case PropSheetLook_OutlookBar:
	case PropSheetLook_Tree:
	case PropSheetLook_List:
		return m_nBarWidth;
	}

	return 0;
}
//**************************************************************************
LRESULT CBCGPPropertySheet::OnBCGSetControlBackStageMode (WPARAM, LPARAM)
{
	m_bBackstageMode = TRUE;
	m_bDrawPageFrame = FALSE;

	m_wndList.m_bBackstageMode = TRUE;

	for (int i = 0; i < GetPageCount (); i++)
	{
		CBCGPPropertyPage* pBCGPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage, GetPage (i));
		if (pBCGPage != NULL)
		{
			ASSERT_VALID (pBCGPage);
			pBCGPage->EnableBackstageMode();
		}
	}

	return 0;
}
//*****************************************************************************************
BOOL CBCGPPropertySheet::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int nNotifyCode = HIWORD (wParam);

	if ((HWND)lParam == m_wndList.GetSafeHwnd() && m_wndList.GetSafeHwnd() != NULL)
	{
		if (nNotifyCode == LBN_SELCHANGE)
		{
			OnSelectList();
		}

		return TRUE;
	}

	UINT nCmdID = LOWORD(wParam);

	if (GetPageTransitionEffect() != CBCGPPageTransitionManager::BCGPPageTransitionNone &&
		(nCmdID == ID_WIZNEXT || nCmdID == ID_WIZBACK))
	{
		BOOL bNext = (nCmdID == ID_WIZNEXT);

		CPropertyPage* pCurrPage = GetActivePage();
		ASSERT_VALID(pCurrPage);

		int nCurrPageIndex = GetActiveIndex();

		if (bNext)
		{
			if (pCurrPage->OnWizardNext() == 0)
			{
				SetActivePageWithEffects(nCurrPageIndex + 1);
			}
		}
		else
		{
			if (pCurrPage->OnWizardBack() == 0)
			{
				SetActivePageWithEffects(nCurrPageIndex - 1);
			}
		}

		return TRUE;
	}

	BOOL bRes = CPropertySheet::OnCommand(wParam, lParam);
	BOOL bIsResizableWizard = (m_psh.dwFlags & PSH_WIZARD) && IsLayoutEnabled();

	if ((nCmdID == ID_WIZNEXT || nCmdID == ID_WIZBACK) && bIsResizableWizard)
	{
		PostMessage(UM_ADJUSTWIZARDPAGE);
	}

	return bRes;
}
//*****************************************************************************************
void CBCGPPropertySheet::OnDrawListBoxBackground(CDC* pDC, CBCGPListBox* pListBox)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pListBox);

	CBCGPPropertyPage* pPage = DYNAMIC_DOWNCAST(CBCGPPropertyPage, GetActivePage ());
	if (pPage == NULL || pPage->m_rectBackstageWatermark.IsRectEmpty() || pPage->m_pBackstageWatermark == NULL)
	{
		return;
	}

	CRect rectImage = pPage->m_rectBackstageWatermark;

	pPage->MapWindowPoints(pListBox, rectImage);

	pPage->m_pBackstageWatermark->DrawEx(pDC, rectImage, 0);
}
//*****************************************************************************************
int CBCGPPropertySheet::CalcNavBarWidth()
{
	int nMaxWidth = 0;

	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&globalData.fontRegular);
	ASSERT_VALID(pOldFont);

	CTabCtrl* pTab = GetTabControl ();
	ASSERT_VALID (pTab);

	for (int iTab = 0; iTab < pTab->GetItemCount(); iTab++)
	{
		TCHAR szTab [256];

		TCITEM item;
		item.mask = TCIF_TEXT;
		item.cchTextMax = 255;
		item.pszText = szTab;

		pTab->GetItem (iTab, &item);

		CString strName = szTab;

		nMaxWidth = max(nMaxWidth, dc.GetTextExtent(strName).cx);
	}	

	if (m_Icons.GetSafeHandle () != NULL)
	{
		IMAGEINFO info;
		m_Icons.GetImageInfo (0, &info);

		CRect rectImage = info.rcImage;
		nMaxWidth += rectImage.Width();
	}

#ifndef _BCGSUITE_
	dc.SelectObject(&globalData.fontCaption);
#endif
	for (int i = 0; i < m_arGroupCaptions.GetSize(); i++)
	{
		nMaxWidth = max(nMaxWidth, dc.GetTextExtent(m_arGroupCaptions[i]).cx);
	}

	dc.SelectObject(pOldFont);

	nMaxWidth += m_bBackstageMode ? 82 : 10;

	return nMaxWidth;
}
//*****************************************************************************************
void CBCGPPropertySheet::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CSize sizePrev(m_sizePrev);

	if (m_Impl.LoadPlacement())
	{
		CRect rectClient;
		GetClientRect(rectClient);

		CSize sizeNew = rectClient.Size ();

		const int dx = sizeNew.cx - sizePrev.cx;
		const int dy = sizeNew.cy - sizePrev.cy;

		if (m_wndTab.GetSafeHwnd() != NULL)
		{
			ASSERT_VALID (&m_wndTab);

			CRect rectTab;
			m_wndTab.GetWindowRect (rectTab);

			m_wndTab.SetWindowPos(NULL, -1, -1, 
				rectTab.Width() + dx, 
				rectTab.Height() + dy, 
				SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

			CPropertyPage* pPage = GetActivePage ();			
			if (pPage->GetSafeHwnd () != NULL)
			{
				m_wndTab.SetActiveTab (GetPageIndex (pPage));
			}
		}
		else if (IsLayoutEnabled())
		{
			CPropertyPage* pPage = GetActivePage ();			
			if (pPage->GetSafeHwnd () != NULL)
			{
				switch (m_look)
				{
				case PropSheetLook_OutlookBar:
				case PropSheetLook_Tree:
				case PropSheetLook_List:
					SetActivePage(pPage);
					break;
				}
			}
		}
	}

	CPropertySheet::OnShowWindow(bShow, nStatus);
}
//*****************************************************************************************
BOOL CBCGPPropertySheet::OnSetPlacement(WINDOWPLACEMENT& wp)
{
	return m_Impl.SetPlacement(wp);
}
//*****************************************************************************************
void CBCGPPropertySheet::OnActivate(UINT nState, CWnd *pWndOther, BOOL /*bMinimized*/) 
{
	Default();
	m_Impl.OnActivate (nState, pWndOther);
}
//*****************************************************************************************
BOOL CBCGPPropertySheet::OnNcActivate(BOOL bActive) 
{
	//-----------------------------------------------------------
	// Do not call the base class because it will call Default()
	// and we may have changed bActive.
	//-----------------------------------------------------------
	BOOL bRes = (BOOL)DefWindowProc (WM_NCACTIVATE, bActive, 0L);

	if (bRes)
	{
		m_Impl.OnNcActivate (bActive);
	}

	return bRes;
}
//*****************************************************************************************
void CBCGPPropertySheet::OnEnable(BOOL bEnable) 
{
	CPropertySheet::OnEnable (bEnable);
	m_Impl.OnNcActivate (bEnable);
}
//*****************************************************************************************
LRESULT CBCGPPropertySheet::OnDWMCompositionChanged(WPARAM,LPARAM)
{
	m_Impl.OnDWMCompositionChanged ();
	return 0;
}
//*****************************************************************************************
LRESULT CBCGPPropertySheet::OnPowerBroadcast(WPARAM wp, LPARAM)
{
	LRESULT lres = Default ();
	
	if (wp == PBT_APMRESUMESUSPEND && AfxGetMainWnd()->GetSafeHwnd() == GetSafeHwnd())
	{
		globalData.Resume ();
#ifdef _BCGSUITE_
		bcgpGestureManager.Resume();
#endif
	}
	
	return lres;
}
//*****************************************************************************************
void CBCGPPropertySheet::OnPaint() 
{
	if (m_rectPageTransition.IsRectEmpty())
	{
		Default();
		return;
	}

	if (m_look == PropSheetLook_Wizard)
	{
		CTabCtrl* pTab = GetTabControl ();
		if (pTab->GetSafeHwnd() != NULL && pTab->IsWindowVisible())
		{
			pTab->ShowWindow(SW_HIDE);
		}
	}

	CPaintDC dc(this); // device context for painting
	CBCGPMemDC memDC(dc, m_rectPageTransition);

	CDC* pDC = &memDC.GetDC();

#ifndef _BCGSUITE_
	if (m_bBackstageMode)
	{
		CBCGPVisualManager::GetInstance()->OnFillRibbonBackstageForm(pDC, NULL, m_rectPageTransition);
	}
	else
#endif
	{
		if (!IsVisualManagerStyle() || !CBCGPVisualManager::GetInstance ()->OnFillDialog (pDC, this, m_rectPageTransition))
		{
			pDC->FillRect(m_rectPageTransition, &globalData.brBarFace);
		}
	}

	DoDrawTransition(pDC, TRUE);
}
//*****************************************************************************************
void CBCGPPropertySheet::OnPageTransitionFinished()
{
	if (m_nNewActivePage != -1)
	{
		SetActivePage(m_nNewActivePage);
		m_nNewActivePage = -1;
	}
	
	if (!GetActivePage()->IsWindowVisible())
	{
		GetActivePage()->ShowWindow(SWP_NOACTIVATE);
	}

	if (m_wndTab.GetSafeHwnd() != NULL)
	{
		m_wndTab.RedrawWindow();
	}
	else if (m_wndTree.GetSafeHwnd() != NULL)
	{
		m_wndTree.RedrawWindow();
	}
	else if (m_wndList.GetSafeHwnd() != NULL)
	{
		m_wndList.RedrawWindow();
	}

	BOOL bIsResizableWizard = (m_psh.dwFlags & PSH_WIZARD) && IsLayoutEnabled();
	if (bIsResizableWizard)
	{
		PostMessage(UM_ADJUSTWIZARDPAGE);
	}
}
//**************************************************************************************
void CBCGPPropertySheet::SetActiveMenu (CBCGPPopupMenu* pMenu)
{
	m_Impl.SetActiveMenu (pMenu);
}
