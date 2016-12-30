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
// BCGPRibbonCustomizePage.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPFrameWnd.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPRibbonCustomizePage.h"
#include "BCGPRibbonCustomizeQATPage.h"
#include "BCGPRibbonToolsPage.h"
#include "BCGPRibbonBar.h"
#include "BCGPRibbonCategory.h"
#include "BCGPRibbonPanel.h"
#include "BCGPRibbonKeyboardCustomizeDlg.h"
#include "BCGPLocalResource.h"
#include "BCGPKeyboardManager.h"
#include "BCGPRenameDlg.h"
#include "BCGPRibbonItemDlg.h"
#include "BCGPContextMenuManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef BCGP_EXCLUDE_RIBBON

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCustomize

IMPLEMENT_DYNAMIC(CBCGPRibbonCustomize, CBCGPPropertySheet)

CBCGPRibbonCustomize::CBCGPRibbonCustomize (CWnd* pWndParent, CBCGPRibbonBar* pRibbon, BOOL bCustomizeQAT) :
	CBCGPPropertySheet (_T(""), pWndParent),
	m_bCustomizeQAT(bCustomizeQAT)
{
	m_psh.dwFlags |= PSH_NOAPPLYNOW;

	CBCGPLocalResource locaRes;

	m_strCaption = (AfxGetApp ()->m_pszAppName == NULL) ? _T("") : AfxGetApp ()->m_pszAppName;
	m_psh.pszCaption = m_strCaption;

	m_pQATPage = new CBCGPRibbonCustomizeQATPage (pRibbon);
	AddPage (m_pQATPage);

	if (pRibbon->IsCustomizationEnabled())
	{
		m_pPage = new CBCGPRibbonCustomizeRibbonPage (pRibbon);
		AddPage (m_pPage);

		SetActivePage(m_bCustomizeQAT ? 0 : 1);
	}
	else
	{
		m_pPage = NULL;
	}

	if (g_pUserToolsManager != NULL)
	{
		m_pToolsPage = new CBCGPRibbonToolsPage(pRibbon);
		AddPage (m_pToolsPage);
	}
	else
	{
		m_pToolsPage = NULL;
	}

	SetLook(CBCGPPropertySheet::PropSheetLook_List, 150 /* List width */);
}
//**********************************************************************
CBCGPRibbonCustomize::~CBCGPRibbonCustomize() 
{
	if (m_pPage != NULL)
	{
		delete m_pPage;
	}
	
	if (m_pQATPage != NULL)
	{
		delete m_pQATPage;
	}

	if (m_pToolsPage != NULL)
	{
		delete m_pToolsPage;
	}
}
//**********************************************************************
void CBCGPRibbonCustomize::EnableKeyboradCustomization (BOOL bEnable/* = TRUE*/)
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pPage);

	if (m_pPage != NULL)
	{
		m_pPage->EnableKeyboradCustomization (bEnable);
	}
	else if (m_pQATPage != NULL)
	{
		m_pQATPage->EnableKeyboradCustomization (bEnable);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCustomizeRibbonPage property page

IMPLEMENT_DYNCREATE(CBCGPRibbonCustomizeRibbonPage, CBCGPPropertyPage)

CBCGPRibbonCustomizeRibbonPage::CBCGPRibbonCustomizeRibbonPage(
	CBCGPRibbonBar*	pRibbonBar, BOOL bShowHiddenCategories) : 
		CBCGPPropertyPage(CBCGPRibbonCustomizeRibbonPage::IDD),
		m_wndRibbonTreeSrc(NULL, pRibbonBar, bShowHiddenCategories),
		m_wndRibbonTreeDest(&m_CustomizationData, pRibbonBar, bShowHiddenCategories)
{
	ASSERT_VALID (pRibbonBar);

	//{{AFX_DATA_INIT(CBCGPRibbonCustomizeRibbonPage)
	m_nCategory = -1;
	m_nRibbonTabsDest = 1;
	//}}AFX_DATA_INIT

	m_pRibbonBar = pRibbonBar;

	CBCGPLocalResource locaRes;
	m_psp.hInstance = AfxGetResourceHandle ();

	m_bIsCustomizeKeyboard = TRUE;
	m_bNoContextCategories = FALSE;
	m_bDontProcessSelChanged = FALSE;

	m_pSelCategorySrc = NULL;
	m_pSelPanelSrc = NULL;
	m_pSelElemSrc = NULL;
	m_pSelCategoryDest = NULL;
	m_pSelPanelDest = NULL;
	m_pSelElemDest = NULL;

	if (m_pRibbonBar->m_bPrintPreviewMode)
	{
		m_pRibbonBar->SetPrintPreviewMode(FALSE);

		CFrameWnd* pParentFrame = m_pRibbonBar->GetParentFrame ();
		if (pParentFrame != NULL)
		{
			CBCGPFrameWnd* pFrameWnd = DYNAMIC_DOWNCAST(CBCGPFrameWnd, pParentFrame);
			if (pFrameWnd != NULL)
			{
				pFrameWnd->ClosePrintPreview();
			}
			else
			{
				CBCGPMDIFrameWnd* pMDIFrameWnd = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, pParentFrame);
				if (pMDIFrameWnd != NULL)
				{
					pMDIFrameWnd->ClosePrintPreview();
				}
			}
		}
	}
}
//**********************************************************************
CBCGPRibbonCustomizeRibbonPage::~CBCGPRibbonCustomizeRibbonPage()
{
	while (!m_lstCustomCategories.IsEmpty ())
	{
		delete m_lstCustomCategories.RemoveHead ();
	}
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::DoDataExchange(CDataExchange* pDX)
{
	CBCGPPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPRibbonCustomizeRibbonPage)
	DDX_Control(pDX, IDC_BCGBARRES_NEW_TAB, m_wndNewTab);
	DDX_Control(pDX, IDC_BCGBARRES_RIBBON_TABS, m_wndRibbonTabsCombo);
	DDX_Control(pDX, IDC_BCGBARRES_COMMANDS_LIST, m_wndRibbonSrcTreePlaceholder);
	DDX_Control(pDX, IDC_BCGBARRES_RENAME, m_wndRename);
	DDX_Control(pDX, IDC_BCGBARRES_NEW_GROUP, m_wndNewGroup);
	DDX_Control(pDX, IDC_BCGBARRES_RIBBON_TREE_LOCATION, m_wndRibbonTreePlaceholder);
	DDX_Control(pDX, IDC_BCGBARRES_KEYBOARD, m_wndKbdCustomize);
	DDX_Control(pDX, IDS_BCGBARRES_ADD, m_wndAdd);
	DDX_Control(pDX, IDC_BCGBARRES_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_BCGBARRES_CATEGORY, m_wndCategoryCombo);
	DDX_Control(pDX, IDC_BCGBARRES_MOVEUP, m_wndUp);
	DDX_Control(pDX, IDC_BCGBARRES_MOVEDOWN, m_wndDown);
	DDX_CBIndex(pDX, IDC_BCGBARRES_CATEGORY, m_nCategory);
	DDX_CBIndex(pDX, IDC_BCGBARRES_RIBBON_TABS, m_nRibbonTabsDest);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBCGPRibbonCustomizeRibbonPage, CBCGPPropertyPage)
	//{{AFX_MSG_MAP(CBCGPRibbonCustomizeRibbonPage)
	ON_CBN_SELENDOK(IDC_BCGBARRES_CATEGORY, OnSelendokCategoryCombo)
	ON_BN_CLICKED(IDS_BCGBARRES_ADD, OnAdd)
	ON_BN_CLICKED(IDC_BCGBARRES_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_BCGBARRES_MOVEUP, OnUp)
	ON_BN_CLICKED(IDC_BCGBARRES_MOVEDOWN, OnDown)
	ON_BN_CLICKED(IDC_BCGBARRES_RESET, OnToolbarReset)
	ON_BN_CLICKED(IDC_BCGBARRES_KEYBOARD, OnCustomizeKeyboard)
	ON_BN_CLICKED(IDC_BCGBARRES_NEW_GROUP, OnNewGroup)
	ON_BN_CLICKED(IDC_BCGBARRES_NEW_TAB, OnNewTab)
	ON_BN_CLICKED(IDC_BCGBARRES_RENAME, OnRename)
	ON_CBN_SELENDOK(IDC_BCGBARRES_RIBBON_TABS, OnSelendokRibbonTabsDest)
	//}}AFX_MSG_MAP
#ifndef BCGP_EXCLUDE_GRID_CTRL
	ON_REGISTERED_MESSAGE(BCGM_GRID_SEL_CHANGED, OnSelChangeRibbonTree)
	ON_REGISTERED_MESSAGE(BCGM_GRID_ITEM_CHANGED, OnChangeRibbonTree)
#endif
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCustomizeRibbonPage message handlers

void CBCGPRibbonCustomizeRibbonPage::OnSelendokCategoryCombo() 
{
	ASSERT_VALID (m_pRibbonBar);

	CWaitCursor wait;

	UpdateData ();

	if (m_nCategory < 0)
	{
		return;
	}

	m_wndRibbonTreeSrc.SetRedraw(FALSE);

	DWORD_PTR dwData = m_wndCategoryCombo.GetItemData (m_nCategory);
	
	if (dwData == 0) // Tabs
	{
		int nTabs = m_bNoContextCategories ? 1 : m_nCategory - m_wndCategoryCombo.GetCount() + 3;
		m_wndRibbonTreeSrc.RebuildItems(nTabs, m_strMainTabs);
	}
	else
	{
		CArray<CBCGPBaseRibbonElement*, CBCGPBaseRibbonElement*> arElements;

		CBCGPRibbonCategory* pCategory = DYNAMIC_DOWNCAST(CBCGPRibbonCategory, (CObject*)dwData);
		if (pCategory != NULL)
		{
			ASSERT_VALID(pCategory);

			pCategory->GetElements(arElements);
			m_wndRibbonTreeSrc.RebuildItems(arElements);
		}
		else
		{
			CBCGPRibbonCustomGroup* pCustomCategory = DYNAMIC_DOWNCAST(CBCGPRibbonCustomGroup, (CObject*)dwData);
			if (pCustomCategory != NULL)
			{
				ASSERT_VALID(pCustomCategory);

				for (POSITION pos = pCustomCategory->m_lstIDs.GetHeadPosition(); pos != NULL;)
				{
					UINT uiCmd = pCustomCategory->m_lstIDs.GetNext(pos);
					if (uiCmd != 0 && uiCmd != (UINT)-1)
					{
						CBCGPBaseRibbonElement* pElem = m_pRibbonBar->FindByID (uiCmd, FALSE, FALSE);
						if (pElem != NULL)
						{
							arElements.Add(pElem);
						}
					}
				}

				m_wndRibbonTreeSrc.RebuildItems(arElements);
			}
		}
	}

	m_wndRibbonTreeSrc.SetRedraw();
	m_wndRibbonTreeSrc.RedrawWindow();
}
//**********************************************************************
int CBCGPRibbonCustomizeRibbonPage::GetNewTabIndex()
{
#ifndef BCGP_EXCLUDE_GRID_CTRL
	// Calculate destination index:
	CBCGPRibbonCategory* pCurrCategoryDest = m_pSelCategoryDest;
	if (pCurrCategoryDest == NULL)
	{
		if (m_pSelPanelDest != NULL)
		{
			pCurrCategoryDest = m_pSelPanelDest->GetParentCategory();
		}
		else if (m_pSelElemDest != NULL)
		{
			pCurrCategoryDest = m_pSelElemDest->GetParentCategory();
		}
	}

	if (pCurrCategoryDest != NULL)
	{
		int nIndex = 0;

		for (int i = 0; i < m_wndRibbonTreeDest.GetRowCount(); i++)
		{
			CBCGPGridRow* pRow = m_wndRibbonTreeDest.GetRow(i);
			ASSERT_VALID(pRow);

			if (pRow->GetParent() == NULL)
			{
				if (pRow->GetData() == (DWORD_PTR)pCurrCategoryDest)
				{
					return nIndex;
				}

				nIndex++;
			}
		}
	}
#endif

	return -1;
}
//**********************************************************************
UINT CBCGPRibbonCustomizeRibbonPage::GetNewTabContextID()
{
#ifndef BCGP_EXCLUDE_GRID_CTRL
	return m_wndRibbonTreeDest.GetContextID(m_wndRibbonTreeDest.GetCurSel());
#else
	return 0;
#endif
}
//**********************************************************************
int CBCGPRibbonCustomizeRibbonPage::GetNewPanelIndex()
{
#ifndef BCGP_EXCLUDE_GRID_CTRL
	// Calculate destination index:

	if (m_pSelCategoryDest != NULL)
	{
		int nVisiblePanels = 0;

		for (int i = 0; i < m_pSelCategoryDest->GetPanelCount(); i++)
		{
			CBCGPRibbonPanel* pPanel = m_pSelCategoryDest->GetPanel(i);
			if (pPanel != NULL && !pPanel->m_bToBeDeleted)
			{
				nVisiblePanels++;
			}
		}

		return nVisiblePanels - 1;
	}

	CBCGPRibbonPanel* pCurrPanelDest = m_pSelPanelDest;
	if (pCurrPanelDest == NULL && m_pSelElemDest != NULL)
	{
		pCurrPanelDest = m_pSelElemDest->GetParentPanel();
	}

	if (pCurrPanelDest != NULL)
	{
		for (int i = 0; i < m_wndRibbonTreeDest.GetRowCount(); i++)
		{
			CBCGPGridRow* pRow = m_wndRibbonTreeDest.GetRow(i);
			ASSERT_VALID(pRow);

			if (pRow->GetParent() != NULL && pRow->IsGroup())
			{
				if (pRow->GetData() == (DWORD_PTR)pCurrPanelDest)
				{
					int nCurIndex = m_CustomizationData.GetPanelIndex(pCurrPanelDest);
					ASSERT(nCurIndex >= 0);

					return nCurIndex + 1;
				}
			}
		}
	}
#endif
	return -1;
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::OnAdd() 
{
	ASSERT_VALID (m_pRibbonBar);

	if (m_pSelCategorySrc != NULL)
	{
		ASSERT_VALID(m_pSelCategorySrc);

		UINT nContextID = GetNewTabContextID();

		CBCGPRibbonCategory* pNewTab = m_pSelCategorySrc->CreateCustomCopy(m_pRibbonBar);
		ASSERT_VALID(pNewTab);

		pNewTab->m_uiContextID = nContextID;
		pNewTab->m_bIsVisible = nContextID == 0;

		if (nContextID != 0)
		{
			pNewTab->SetTabColor(m_pRibbonBar->GetContextColor(nContextID));
		}

		m_pRibbonBar->AddCustomCategory(pNewTab, TRUE);
		m_CustomizationData.AddCustomTab(*pNewTab, GetNewTabIndex(), nContextID);

		RebuildDestTree((DWORD_PTR)pNewTab, TRUE);
		m_wndRibbonTreeSrc.MoveSelection(TRUE);
		return;
	}

	if (m_pSelPanelSrc != NULL)
	{
		ASSERT_VALID(m_pSelPanelSrc);

		CBCGPRibbonCategory* pParentCategory = m_pSelCategoryDest;
		if (pParentCategory == NULL)
		{
			if (m_pSelPanelDest != NULL)
			{
				ASSERT_VALID(m_pSelPanelDest);
				pParentCategory = m_pSelPanelDest->GetParentCategory();
			}
			else if (m_pSelElemDest != NULL)
			{
				ASSERT_VALID(m_pSelElemDest);
				pParentCategory = m_pSelElemDest->GetParentCategory();
			}
		}

		if (pParentCategory == NULL)
		{
			ASSERT(FALSE);
			return;
		}

		ASSERT_VALID(pParentCategory);

		if (pParentCategory->HasPanel(m_pSelPanelSrc))
		{
			if (m_CustomizationData.IsPanelHidden(m_pSelPanelSrc))
			{
				m_CustomizationData.ShowPanel(m_pSelPanelSrc, TRUE);
				RebuildDestTree((DWORD_PTR)m_pSelPanelSrc, TRUE);
			}
		}
		else
		{
			CBCGPRibbonPanel* pNewPanel = m_pSelPanelSrc->CreateCustomCopy(pParentCategory);
			ASSERT_VALID(pNewPanel);

			pNewPanel->m_bIsNew = TRUE;
			int nIndex = GetNewPanelIndex();

			pParentCategory->AddCustomPanel(pNewPanel, nIndex);
			m_CustomizationData.AddCustomPanel(*pNewPanel, nIndex);

			RebuildDestTree((DWORD_PTR)pNewPanel, TRUE);

			m_wndRibbonTreeSrc.MoveSelection(TRUE);
		}
		return;
	}

	if (m_pSelElemSrc != NULL)
	{
		ASSERT_VALID(m_pSelElemSrc);
		
		CBCGPRibbonPanel* pPanel = m_pSelPanelDest;
		if (pPanel == NULL)
		{
			if (m_pSelElemDest != NULL)
			{
				pPanel = m_pSelElemDest->GetParentPanel();
			}
		}

		if (pPanel == NULL)
		{
			ASSERT(FALSE);
			return;
		}

		ASSERT_VALID(pPanel);

		CBCGPBaseRibbonElement* pElem = m_pSelElemSrc->CreateCustomCopy();
		ASSERT_VALID (pElem);

		pElem->SetParentCategory(pPanel->GetParentCategory());
		pElem->m_bIsNew = TRUE;

		pPanel->Add(pElem);
		m_CustomizationData.AddCustomElement(*pPanel, *pElem);

		m_wndRibbonTreeSrc.MoveSelection(TRUE);

		RebuildDestTree((DWORD_PTR)pElem);
		return;
	}
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::OnRemove() 
{
#ifndef BCGP_EXCLUDE_GRID_CTRL
	CBCGPGridRow* pRow = m_wndRibbonTreeDest.GetCurSel();
	if (pRow == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	m_bDontProcessSelChanged = TRUE;

	ASSERT_VALID(pRow);

	CBCGPGridRow* pParentRow = pRow->GetParent();

	if (m_pSelCategoryDest != NULL)
	{
		ASSERT_VALID(m_pSelCategoryDest);
		ASSERT(m_pSelCategoryDest->IsCustom());

		m_pSelCategoryDest->m_bToBeDeleted = TRUE;
		m_CustomizationData.RemoveCustomTab(*m_pSelCategoryDest);
	}
	else if (m_pSelPanelDest != NULL)
	{
		ASSERT_VALID(m_pSelPanelDest);

		if (m_pSelPanelDest->IsCustom())
		{
			m_pSelPanelDest->m_bToBeDeleted = TRUE;
			m_CustomizationData.RemoveCustomPanel(*m_pSelPanelDest);
		}
		else
		{
			m_CustomizationData.ShowPanel(m_pSelPanelDest, FALSE);
		}
	}
	else if (m_pSelElemDest != NULL)
	{
		ASSERT_VALID(m_pSelElemDest);

		m_CustomizationData.RemoveCustomElement(*m_pSelElemDest);

		if (m_pSelElemDest->m_bIsNew)
		{
			CBCGPRibbonPanel* pPanel = m_pSelElemDest->GetParentPanel();
			ASSERT_VALID(pPanel);

			int nIndex = pPanel->GetIndex(m_pSelElemDest);
			ASSERT(nIndex >= 0);

			pPanel->Remove(nIndex);
		}
	}
	else
	{
		ASSERT(FALSE);
		m_bDontProcessSelChanged = FALSE;
		return;
	}

	int id = pRow->GetRowId();
	
	BOOL bDisableDown = FALSE;
	BOOL bDisableUp = FALSE;

	m_wndRibbonTreeDest.SetRedraw(FALSE);

	if (!m_wndRibbonTreeDest.MoveSelection(TRUE))
	{
		bDisableDown = TRUE;
		bDisableUp = !m_wndRibbonTreeDest.MoveSelection(FALSE);
	}

	CBCGPGridRow* pSelRow = m_wndRibbonTreeDest.GetCurSel();
	m_wndRibbonTreeDest.SetCurSel(NULL, FALSE);

	m_wndRibbonTreeDest.RemoveRow(id);
	m_wndRibbonTreeDest.AdjustLayout ();

	m_bDontProcessSelChanged = FALSE;

	if (bDisableDown && bDisableUp)
	{
		m_wndRibbonTreeDest.SetCurSel(pParentRow);
	}
	else
	{
		m_wndRibbonTreeDest.SetCurSel(pSelRow);
	}
	
	m_wndRibbonTreeDest.SetRedraw();
	m_wndRibbonTreeDest.RedrawWindow();

#endif
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::OnUp() 
{
	MoveItem (TRUE);
	m_wndUp.SetFocus();
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::OnDown() 
{
	MoveItem (FALSE);
	m_wndDown.SetFocus();
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::OnToolbarReset() 
{
#ifndef BCGP_EXCLUDE_GRID_CTRL
	ASSERT_VALID (m_pRibbonBar);

	CBCGPLocalResource locaRes;
	if (AfxMessageBox (IDS_BCGBARRES_RESET_RIBBON, MB_YESNO | MB_ICONWARNING) != IDYES)
	{
		return;
	}

	// Mark all custom data as "to be deleted":
	for (int i = 0; i < m_wndRibbonTreeDest.GetRowCount(); i++)
	{
		CBCGPGridRow* pRow = m_wndRibbonTreeDest.GetRow(i);
		if (pRow == NULL)
		{
			continue;
		}

		CObject* lpData = (CObject*)pRow->GetData();
		if (lpData == NULL)
		{
			continue;
		}

		CBCGPRibbonCategory* pCategory = DYNAMIC_DOWNCAST(CBCGPRibbonCategory, lpData);
		if (pCategory != NULL && pCategory->IsCustom())
		{
			pCategory->m_bToBeDeleted = TRUE;
		}
		else
		{
			CBCGPRibbonPanel* pPanel = DYNAMIC_DOWNCAST(CBCGPRibbonPanel, lpData);
			if (pPanel != NULL && pPanel->IsCustom())
			{
				pPanel->m_bToBeDeleted = TRUE;
			}
		}
	}

	m_CustomizationData.ResetAll(m_pRibbonBar);
	RebuildDestTree();
#endif
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::RebuildDestTree(DWORD_PTR dwNewSel, BOOL bExpandSel)
{
#ifndef BCGP_EXCLUDE_GRID_CTRL
	if (dwNewSel == 0)
	{
		CBCGPGridRow* pRow = m_wndRibbonTreeDest.GetCurSel();
		if (pRow != NULL)
		{
			dwNewSel = pRow->GetData();
		}
	}
	
	m_wndRibbonTreeDest.SetRedraw(FALSE);

	m_wndRibbonTreeDest.RebuildItems(m_bNoContextCategories ? 1 : m_nRibbonTabsDest, m_strMainTabs);

	if (dwNewSel != 0)
	{
		CBCGPGridRow* pRow = m_wndRibbonTreeDest.FindRowByData(dwNewSel);
		if (pRow != NULL)
		{
			if (bExpandSel && pRow->IsGroup())
			{
				pRow->Expand();
			}

			m_wndRibbonTreeDest.SetCurSel(pRow, FALSE);
			m_wndRibbonTreeDest.EnsureVisible(pRow, TRUE);
		}
	}

	m_wndRibbonTreeDest.SetRedraw();
	m_wndRibbonTreeDest.RedrawWindow();
#endif
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::CreateRibbonTree(CStatic& wndPlaceHolder, CBCGPRibbonTreeCtrl& wndTree)
{
	CRect rectGrid;
	wndPlaceHolder.GetClientRect (&rectGrid);
	wndPlaceHolder.MapWindowPoints (this, &rectGrid);

#ifndef BCGP_EXCLUDE_GRID_CTRL
	wndTree.Create (WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rectGrid, this, (UINT)-1);

	wndTree.EnableColumnAutoSize (TRUE);
	wndTree.SetSingleSel ();
	wndTree.EnableGroupByBox (FALSE);
	wndTree.SetReadOnly ();
	wndTree.SetWholeRowSel ();
	wndTree.EnableHeader (FALSE, 0);
	wndTree.EnableGridLines(FALSE);
	wndTree.SetRowMarker(FALSE);

	// Set grid tab order (first):
	wndTree.SetWindowPos (&CWnd::wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// Set grid colors
	CBCGPGridColors colors;
	colors.m_LeftOffsetColors.m_clrBackground = globalData.clrWindow;
	colors.m_SelColors.m_clrBackground = ::GetSysColor(COLOR_HIGHLIGHT);
	colors.m_SelColors.m_clrText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	colors.m_SelColorsInactive.m_clrBackground = ::GetSysColor(COLOR_BTNFACE);
	colors.m_SelColorsInactive.m_clrText = ::GetSysColor(COLOR_BTNTEXT);

	wndTree.SetColorTheme (colors);
	
	wndTree.InsertColumn (0, _T("Name"), 80);
#else
	wndTree.Create(_T("Please un-comment BCGP_EXCLUDE_GRID_CTRL in BCGCBProConfig.h"), WS_CHILD | WS_VISIBLE | WS_BORDER, rectGrid, this);
	wndTree.SetFont(GetFont());
#endif
}
//**********************************************************************
BOOL CBCGPRibbonCustomizeRibbonPage::OnInitDialog() 
{
	{
		CBCGPLocalResource localRes;
		CBCGPPropertyPage::OnInitDialog();
	}
	
	ASSERT_VALID (m_pRibbonBar);

	if (!m_pRibbonBar->IsCustomizationEnabled())
	{
		TRACE0("The Ribbon customization is not enabled\n");
		ASSERT(FALSE);
	}

	m_wndRibbonTabsCombo.GetLBText(1, m_strMainTabs);

	m_CustomizationData.CopyFrom(m_pRibbonBar->m_CustomizationData);

	CreateRibbonTree(m_wndRibbonSrcTreePlaceholder, m_wndRibbonTreeSrc);
	CreateRibbonTree(m_wndRibbonTreePlaceholder, m_wndRibbonTreeDest);

	CBCGPStaticLayout* pLayout = (CBCGPStaticLayout*)GetLayout ();
	if (pLayout != NULL)
	{
		pLayout->AddAnchor (IDC_BCGBARRES_CATEGORY, CBCGPStaticLayout::e_MoveTypeNone, CBCGPStaticLayout::e_SizeTypeHorz, CSize(0, 0), CSize(50, 100));
		pLayout->AddAnchor (m_wndRibbonTreeSrc.GetSafeHwnd(), CBCGPStaticLayout::e_MoveTypeNone, CBCGPStaticLayout::e_SizeTypeBoth, CSize(0, 0), CSize(50, 100));
		pLayout->AddAnchor (IDC_BCGBARRES_KEYBOARD, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeNone);
		pLayout->AddAnchor (IDC_BCGBARRES_ACCEL_LABEL, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeNone);
		pLayout->AddAnchor (IDS_BCGBARRES_ADD, CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeNone, CSize(50, 100));
		pLayout->AddAnchor (IDC_BCGBARRES_REMOVE, CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeNone, CSize(50, 100));
		pLayout->AddAnchor (IDD_BCGBAR_RES_LABEL1, CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeNone, CSize(50, 100));
		pLayout->AddAnchor (IDC_BCGBARRES_RIBBON_TABS, CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeHorz, CSize(50, 100), CSize(50, 100));
		pLayout->AddAnchor (m_wndRibbonTreeDest.GetSafeHwnd(), CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeBoth, CSize(50, 0), CSize(50, 100));
		pLayout->AddAnchor (IDC_BCGBARRES_MOVEUP, CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeNone);
		pLayout->AddAnchor (IDC_BCGBARRES_MOVEDOWN, CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeNone);
		pLayout->AddAnchor (IDC_BCGBARRES_NEW_TAB, CBCGPStaticLayout::e_MoveTypeBoth, CBCGPStaticLayout::e_SizeTypeNone, CSize(50, 100));
		pLayout->AddAnchor (IDC_BCGBARRES_NEW_GROUP, CBCGPStaticLayout::e_MoveTypeBoth, CBCGPStaticLayout::e_SizeTypeNone, CSize(50, 100));
		pLayout->AddAnchor (IDC_BCGBARRES_RENAME, CBCGPStaticLayout::e_MoveTypeBoth, CBCGPStaticLayout::e_SizeTypeNone, CSize(50, 100));
		pLayout->AddAnchor (IDD_BCGBAR_RES_LABEL2, CBCGPStaticLayout::e_MoveTypeBoth, CBCGPStaticLayout::e_SizeTypeNone, CSize(50, 100));
		pLayout->AddAnchor (IDC_BCGBARRES_RESET, CBCGPStaticLayout::e_MoveTypeBoth, CBCGPStaticLayout::e_SizeTypeNone, CSize(50, 100));
	}

	//-----------------------------------
	// Setup Customize Keyboard controls:
	//-----------------------------------
	if (!m_bIsCustomizeKeyboard || g_pKeyboardManager == NULL)
	{
		if (GetDlgItem (IDC_BCGBARRES_ACCEL_LABEL) != NULL)
		{
			GetDlgItem (IDC_BCGBARRES_ACCEL_LABEL)->ShowWindow (SW_HIDE);
		}

		m_wndKbdCustomize.EnableWindow (FALSE);
		m_wndKbdCustomize.ShowWindow (SW_HIDE);
	}

	m_wndUp.SetStdImage (CBCGPMenuImages::IdArowUpLarge, CBCGPMenuImages::ImageBlack2, CBCGPMenuImages::IdArowUpLarge, CBCGPMenuImages::ImageLtGray);
	m_wndUp.SetWindowText(_T("Up"));
	m_wndUp.SetDrawText(FALSE, FALSE);
	
	m_wndDown.SetStdImage (CBCGPMenuImages::IdArowDownLarge, CBCGPMenuImages::ImageBlack2, CBCGPMenuImages::IdArowDownLarge, CBCGPMenuImages::ImageLtGray);
	m_wndDown.SetWindowText(_T("Down"));
	m_wndDown.SetDrawText(FALSE, FALSE);

	//-----------------------
	// Add custom categories:
	//-----------------------
	int nIndex = 0;

	for (POSITION pos = m_lstCustomCategories.GetHeadPosition (); pos != NULL;)
	{
		CBCGPRibbonCustomGroup* pCustCategory = m_lstCustomCategories.GetNext (pos);
		ASSERT_VALID (pCustCategory);

		m_wndCategoryCombo.InsertString(nIndex, pCustCategory->m_strName);
		m_wndCategoryCombo.SetItemData (nIndex++, (DWORD_PTR) pCustCategory);
	}

	//-------------------
	// Add main category:
	//-------------------
	CBCGPRibbonCategory* pMainCategory = m_pRibbonBar->GetMainCategory ();
	if (pMainCategory != NULL)
	{
		ASSERT_VALID (pMainCategory);
		
		m_wndCategoryCombo.InsertString(nIndex, pMainCategory->GetName ());
		m_wndCategoryCombo.SetItemData (nIndex++, (DWORD_PTR) pMainCategory);
	}

	if (m_wndCategoryCombo.GetCount () > 0)
	{
		m_nCategory = 0;
		UpdateData (FALSE);

		OnSelendokCategoryCombo ();
	}

	//----------------------------------------------------------------------------
	// If ribbon doesn't have context categories, remove them from the comboboxes:
	//----------------------------------------------------------------------------
	if (m_pRibbonBar->GetContextCategoriesCount() == 0)
	{
		m_bNoContextCategories = TRUE;

		m_wndCategoryCombo.DeleteString(m_wndCategoryCombo.GetCount() - 3);
		m_wndCategoryCombo.DeleteString(m_wndCategoryCombo.GetCount() - 1);

		m_wndRibbonTabsCombo.DeleteString(0);
		m_wndRibbonTabsCombo.DeleteString(m_wndRibbonTabsCombo.GetCount() - 1);
	}

	OnSelendokRibbonTabsDest();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::MoveItem (BOOL bMoveUp) 
{
#ifndef BCGP_EXCLUDE_GRID_CTRL
	int nOffset = bMoveUp ? - 1: 1;

	BOOL bIsSelExpanded = FALSE;
	CBCGPGridRow* pRow = m_wndRibbonTreeDest.GetCurSel();
	if (pRow != NULL && pRow->IsGroup())
	{
		bIsSelExpanded = pRow->IsExpanded();
	}

	if (m_pSelCategoryDest != NULL)
	{
		ASSERT_VALID(m_pSelCategoryDest);

		if (m_pSelCategoryDest->IsCustom())
		{
			UINT nContextID = m_wndRibbonTreeDest.GetContextID(pRow, nOffset);
			m_CustomizationData.SetTabContextID(m_pSelCategoryDest, nContextID);
		}

		m_CustomizationData.SetTabIndex(m_pSelCategoryDest, m_CustomizationData.GetTabIndex(m_pSelCategoryDest) + nOffset);
	}
	else if (m_pSelPanelDest != NULL)
	{
		ASSERT_VALID(m_pSelPanelDest);

		int nCurIndex = m_CustomizationData.GetPanelIndex(m_pSelPanelDest);
		ASSERT(nCurIndex >= 0);

		m_CustomizationData.SetPanelIndex(m_pSelPanelDest, nCurIndex + nOffset);
	}
	else if (m_pSelElemDest != NULL)
	{
		ASSERT_VALID(m_pSelElemDest);
		m_CustomizationData.MoveElement(m_pSelElemDest, nOffset);
	}

	RebuildDestTree();

	pRow = m_wndRibbonTreeDest.GetCurSel();
	if (bIsSelExpanded && pRow != NULL)
	{
		pRow->Expand();
	}
#endif
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::OnOK() 
{
	ASSERT_VALID (m_pRibbonBar);

	UpdateData ();

	m_pRibbonBar->OnCancelMode ();
	m_pRibbonBar->OnCloseCustomizePage(TRUE);

	m_CustomizationData.Apply(m_pRibbonBar);

	CBCGPPropertyPage::OnOK();
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::OnCancel() 
{
	ASSERT_VALID (m_pRibbonBar);

	m_pRibbonBar->OnCloseCustomizePage(FALSE);
	CBCGPPropertyPage::OnCancel();
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::OnCustomizeKeyboard()
{
	ASSERT_VALID (m_pRibbonBar);

	CBCGPRibbonKeyboardCustomizeDlg dlg (m_pRibbonBar, this);
	dlg.DoModal ();
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::AddCustomCategory (LPCTSTR lpszName, const CList<UINT, UINT>& lstIDS)
{
	ASSERT_VALID (this);
	ASSERT (lpszName != NULL);
	ASSERT (GetSafeHwnd () == NULL);

	CBCGPRibbonCustomGroup* pCategory = new CBCGPRibbonCustomGroup;
	pCategory->m_strName = lpszName;

	pCategory->m_lstIDs.AddTail ((CList<UINT,UINT>*)&lstIDS);

	m_lstCustomCategories.AddTail (pCategory);
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::EnableKeyboradCustomization (BOOL bEnable/* = TRUE*/)
{
	ASSERT_VALID (this);
	ASSERT (GetSafeHwnd () == NULL);

	m_bIsCustomizeKeyboard = bEnable;
}
//**********************************************************************
LRESULT CBCGPRibbonCustomizeRibbonPage::OnSelChangeRibbonTree(WPARAM, LPARAM lp)
{
#ifndef BCGP_EXCLUDE_GRID_CTRL
	if (m_bDontProcessSelChanged)
	{
		return 0;
	}

	m_wndAdd.EnableWindow(FALSE);

	CBCGPGridCtrl* pGrid = (CBCGPGridCtrl*)lp;

	if (pGrid->GetSafeHwnd() == m_wndRibbonTreeDest.GetSafeHwnd())
	{
		m_wndRename.EnableWindow(FALSE);
		m_wndNewTab.EnableWindow(FALSE);
		m_wndNewGroup.EnableWindow(FALSE);
		m_wndRemove.EnableWindow(FALSE);
		m_wndUp.EnableWindow(FALSE);
		m_wndDown.EnableWindow(FALSE);

		m_pSelCategoryDest = NULL;
		m_pSelPanelDest = NULL;
		m_pSelElemDest = NULL;

		CBCGPGridRow* pRow = m_wndRibbonTreeDest.GetCurSel();
		if (pRow == NULL)
		{
			return 0;
		}

		ASSERT_VALID(pRow);

		CObject* lpData = (CObject*)pRow->GetData();
		if (lpData != NULL)
		{
			m_wndNewTab.EnableWindow();

			m_pSelCategoryDest = DYNAMIC_DOWNCAST(CBCGPRibbonCategory, lpData);
			if (m_pSelCategoryDest != NULL)
			{
				m_wndRename.EnableWindow();
				m_wndNewGroup.EnableWindow();

				if (m_pSelCategoryDest->IsCustom())
				{
					m_wndRemove.EnableWindow();
				}

				m_wndUp.EnableWindow(!m_wndRibbonTreeDest.IsSelFirstInGroup());
				m_wndDown.EnableWindow(!m_wndRibbonTreeDest.IsSelLastInGroup());
			}
			else
			{
				m_pSelPanelDest = DYNAMIC_DOWNCAST(CBCGPRibbonPanel, lpData);
				if (m_pSelPanelDest != NULL)
				{
					m_wndRename.EnableWindow();
					m_wndNewGroup.EnableWindow();
					m_wndRemove.EnableWindow();

					m_wndUp.EnableWindow(!m_wndRibbonTreeDest.IsSelFirstInGroup());
					m_wndDown.EnableWindow(!m_wndRibbonTreeDest.IsSelLastInGroup());
				}
				else
				{
					m_pSelElemDest = DYNAMIC_DOWNCAST(CBCGPBaseRibbonElement, lpData);
					if (m_pSelElemDest != NULL)
					{
						m_wndNewGroup.EnableWindow();
						
						if (m_pSelElemDest->IsCustom() && m_pSelElemDest->GetParentPanel() != NULL)
						{
							m_wndRename.EnableWindow();
							m_wndRemove.EnableWindow();

							m_wndUp.EnableWindow(!m_wndRibbonTreeDest.IsSelFirstInGroup());
							m_wndDown.EnableWindow(!m_wndRibbonTreeDest.IsSelLastInGroup());
						}
					}
				}
			}
		}
	}
	else if (pGrid->GetSafeHwnd() == m_wndRibbonTreeSrc.GetSafeHwnd())
	{
		m_pSelCategorySrc = NULL;
		m_pSelPanelSrc = NULL;
		m_pSelElemSrc = NULL;

		CBCGPGridRow* pRow = m_wndRibbonTreeSrc.GetCurSel();
		if (pRow == NULL)
		{
			return 0;
		}

		ASSERT_VALID(pRow);

		CObject* lpData = (CObject*)pRow->GetData();
		if (lpData != NULL)
		{
			m_pSelCategorySrc = DYNAMIC_DOWNCAST(CBCGPRibbonCategory, lpData);
			m_pSelPanelSrc = DYNAMIC_DOWNCAST(CBCGPRibbonPanel, lpData);
			m_pSelElemSrc = DYNAMIC_DOWNCAST(CBCGPBaseRibbonElement, lpData);
		}
	}

	if (m_pSelCategorySrc != NULL)
	{
		m_wndAdd.EnableWindow();
	}
	else if (m_pSelPanelSrc != NULL)
	{
		ASSERT_VALID(m_pSelPanelSrc);

		CBCGPRibbonCategory* pCategory = m_pSelCategoryDest;
		if (pCategory == NULL)
		{
			if (m_pSelPanelDest != NULL)
			{
				pCategory = m_pSelPanelDest->GetParentCategory();
			}
			else if (m_pSelElemDest != NULL)
			{
				pCategory = m_pSelElemDest->GetParentCategory();
			}
		}

		if (pCategory != NULL)
		{
			m_wndAdd.EnableWindow(pCategory != NULL && pCategory->FindPanelByOriginal(m_pSelPanelSrc) == NULL);
		}
	}
	else if (m_pSelElemSrc != NULL)
	{
		ASSERT_VALID(m_pSelElemSrc);

		CBCGPRibbonPanel* pPanel = m_pSelPanelDest;
		if (pPanel == NULL && m_pSelElemDest != NULL)
		{
			pPanel = m_pSelElemDest->GetParentPanel();
		}

		BOOL bCanAdd = FALSE;

		if (pPanel != NULL && pPanel->IsCustom() && pPanel->m_pOriginal == NULL)
		{
			CBCGPRibbonCustomPanel* pCustomPanel = m_CustomizationData.FindCustomPanel(pPanel);
			if (pCustomPanel != NULL)
			{
				ASSERT_VALID(pCustomPanel);

				if (pCustomPanel->FindByID(m_pSelElemSrc->GetID()) == NULL &&
					m_wndRibbonTreeDest.FindElementInPanel(pPanel, m_pSelElemSrc->GetID()) == NULL)
				{
					bCanAdd = TRUE;
				}
			}
		}

		m_wndAdd.EnableWindow(bCanAdd);
	}
#endif
	return 0;
}
//**********************************************************************
LRESULT CBCGPRibbonCustomizeRibbonPage::OnChangeRibbonTree(WPARAM /*wp*/, LPARAM lp)
{
#ifndef BCGP_EXCLUDE_GRID_CTRL
	BCGPGRID_ITEM_INFO* pInfo = (BCGPGRID_ITEM_INFO*)lp;
	if (pInfo == NULL)
	{
		return 0;
	}

	CBCGPGridRow* pRow = m_wndRibbonTreeDest.GetCurSel();
	if (pRow != NULL)
	{
		ASSERT_VALID(pRow);

		CBCGPGridItem* pItem = pRow->GetItem(0);
		ASSERT_VALID(pItem);

		m_CustomizationData.ShowTab((CBCGPRibbonCategory*)pRow->GetData(), (bool)pItem->GetValue());
	}
#endif
	return 0;
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::OnNewGroup() 
{
	ASSERT_VALID (m_pRibbonBar);

	CBCGPRibbonCategory* pParentCategory = m_pSelCategoryDest;
	if (pParentCategory == NULL)
	{
		if (m_pSelPanelDest != NULL)
		{
			ASSERT_VALID(m_pSelPanelDest);
			pParentCategory = m_pSelPanelDest->GetParentCategory();
		}
		else if (m_pSelElemDest != NULL)
		{
			ASSERT_VALID(m_pSelElemDest);
			pParentCategory = m_pSelElemDest->GetParentCategory();
		}
	}

	if (pParentCategory == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	ASSERT_VALID(pParentCategory);

	CBCGPRibbonPanel* pNewPanel = pParentCategory->AddPanel(m_pRibbonBar->GetCustomizationOptions().m_strNewPanelLabel);
	pNewPanel->m_bIsNew = TRUE;
	pNewPanel->m_bIsCustom = TRUE;

	m_CustomizationData.AddCustomPanel(*pNewPanel, GetNewPanelIndex());

	RebuildDestTree((DWORD_PTR)pNewPanel, TRUE);
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::OnNewTab() 
{
	ASSERT_VALID (m_pRibbonBar);

	UINT nContextID = GetNewTabContextID();

	CBCGPRibbonCategory* pNewTab = new CBCGPRibbonCategory(m_pRibbonBar);
	pNewTab->m_bIsNew = TRUE;
	pNewTab->SetName(m_pRibbonBar->GetCustomizationOptions().m_strNewCategoryLabel);
	pNewTab->m_uiContextID = nContextID;
	pNewTab->m_bIsVisible = nContextID == 0;
	
	if (nContextID != 0)
	{
		pNewTab->SetTabColor(m_pRibbonBar->GetContextColor(nContextID));
	}

	CBCGPRibbonPanel* pNewPanel = pNewTab->AddPanel(m_pRibbonBar->GetCustomizationOptions().m_strNewPanelLabel);
	pNewPanel->m_bIsNew = TRUE;
	pNewPanel->m_bIsCustom = TRUE;

	m_pRibbonBar->AddCustomCategory(pNewTab, TRUE);
	m_CustomizationData.AddCustomTab(*pNewTab, GetNewTabIndex(), nContextID);

	RebuildDestTree((DWORD_PTR)pNewPanel, TRUE);
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::OnRename() 
{
#ifndef BCGP_EXCLUDE_GRID_CTRL
	CBCGPGridRow* pRow = m_wndRibbonTreeDest.GetCurSel();
	if (pRow == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	if (m_pSelCategoryDest != NULL)
	{
		RenameCategory(pRow);
	}
	else if (m_pSelPanelDest != NULL)
	{
		RenamePanel(pRow);
	}
	else if (m_pSelElemDest != NULL)
	{
		RenameElement(pRow);
	}
	else
	{
		ASSERT(FALSE);
	}
#endif
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::OnSelendokRibbonTabsDest() 
{
#ifndef BCGP_EXCLUDE_GRID_CTRL
	UpdateData();

	RebuildDestTree();

	CBCGPGridRow* pRow = m_wndRibbonTreeDest.FindRowByData((DWORD_PTR)m_pRibbonBar->GetActiveCategory());
	if (pRow != NULL)
	{
		m_wndRibbonTreeDest.SetCurSel(pRow);

		if (pRow->IsGroup())
		{
			pRow->Expand();
		}
	}
#endif
}

#ifndef BCGP_EXCLUDE_GRID_CTRL

void CBCGPRibbonCustomizeRibbonPage::RenameCategory(CBCGPGridRow* pRow)
{
	ASSERT_VALID(m_pRibbonBar);
	ASSERT_VALID(m_pSelCategoryDest);
	ASSERT_VALID(pRow);

	CBCGPRenameDlg dlg(this);

	if (!m_CustomizationData.GetTabName(m_pSelCategoryDest, dlg.m_strName))
	{
		dlg.m_strName = m_pSelCategoryDest->GetDisplayName();
	}

	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	CString strNewName = dlg.m_strName;

	m_CustomizationData.RenameTab(m_pSelCategoryDest, strNewName);

	CBCGPGridCheckItem* pCheckItem = (CBCGPGridCheckItem*)pRow->GetItem(0);
	ASSERT_VALID(pCheckItem);

	if (m_pSelCategoryDest->IsCustom())
	{
		strNewName += m_pRibbonBar->GetCustomizationOptions().m_strCustomLabel;
	}

	pCheckItem->SetLabel(strNewName.IsEmpty() ? _T(" ") : strNewName);

	pRow->Redraw();
}
//***********************************************************************
void CBCGPRibbonCustomizeRibbonPage::RenamePanel(CBCGPGridRow* pRow)
{
	ASSERT_VALID(m_pRibbonBar);
	ASSERT_VALID(m_pSelPanelDest);
	ASSERT_VALID(pRow);

	CBCGPRenameDlg dlg(this);

	if (!m_CustomizationData.GetPanelName(m_pSelPanelDest, dlg.m_strName))
	{
		dlg.m_strName = m_pSelPanelDest->GetName();
	}

	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	CString strNewName = dlg.m_strName;

	m_CustomizationData.RenamePanel(m_pSelPanelDest, strNewName);
	
	if (m_pSelPanelDest->IsCustom())
	{
		strNewName += m_pRibbonBar->GetCustomizationOptions().m_strCustomLabel;
	}

	pRow->GetItem(0)->SetValue((LPCTSTR)strNewName);
	pRow->Redraw();
}
//**********************************************************************
void CBCGPRibbonCustomizeRibbonPage::RenameElement(CBCGPGridRow* pRow)
{
	ASSERT_VALID(m_pRibbonBar);
	ASSERT_VALID(m_pSelElemDest);
	ASSERT_VALID(pRow);

	CString strNewName;

	if (m_pRibbonBar->m_CustomImages.GetCount() > 0 && m_pSelElemDest->IsCustomIconAllowed())
	{
		CBCGPRibbonItemDlg dlg(m_pRibbonBar->m_CustomImages, this);
		dlg.m_strName = m_pSelElemDest->GetText();

		int nImageIndex = m_CustomizationData.GetElementImage(m_pSelElemDest);
		if (nImageIndex < 0)
		{
			nImageIndex = m_pSelElemDest->m_nCustomImageIndex;
		}

		dlg.m_iSelImage = nImageIndex;

		if (dlg.DoModal() != IDOK)
		{
			return;
		}
	
		strNewName = dlg.m_strName;
		m_CustomizationData.SetElementImage(m_pSelElemDest, dlg.m_iSelImage);
	}
	else
	{
		CBCGPRenameDlg dlg(this);
		dlg.m_strName = m_pSelElemDest->GetText();

		if (dlg.DoModal() != IDOK)
		{
			return;
		}
	
		strNewName = dlg.m_strName;
	}

	m_CustomizationData.RenameElement(m_pSelElemDest, strNewName);

	pRow->GetItem(0)->SetValue((LPCTSTR)strNewName);
	pRow->Redraw();
}
//************************************************************************************************
void CBCGPRibbonCustomizeRibbonPage::OnShowTreeContextMenu(CBCGPRibbonTreeCtrl* pTreeCtrl, CPoint point)
{
	if (pTreeCtrl != &m_wndRibbonTreeDest)
	{
		return;
	}

	if (m_pSelCategoryDest == NULL && m_pSelPanelDest == NULL && m_pSelElemDest == NULL)
	{
		return;
	}

	const UINT idNewTab		= (UINT) -102;
	const UINT idNewGroup	= (UINT) -103;
	const UINT idRename		= (UINT) -104;
	const UINT idRemove		= (UINT) -105;
	const UINT idMoveUp		= (UINT) -106;
	const UINT idMoveDown	= (UINT) -107;

	CMenu menu;
	menu.CreatePopupMenu ();

	{
		CBCGPLocalResource locaRes;
		CString strItem;

		m_wndNewTab.GetWindowText(strItem);
		menu.AppendMenu(MF_STRING, idNewTab, strItem);

		if (!m_wndNewTab.IsWindowEnabled())
		{
			menu.EnableMenuItem(idNewTab, MF_GRAYED);
		}

		m_wndNewGroup.GetWindowText(strItem);
		menu.AppendMenu(MF_STRING, idNewGroup, strItem);

		if (!m_wndNewGroup.IsWindowEnabled())
		{
			menu.EnableMenuItem(idNewGroup, MF_GRAYED);
		}

		menu.AppendMenu (MF_SEPARATOR);

		m_wndRename.GetWindowText(strItem);
		menu.AppendMenu(MF_STRING, idRename, strItem);

		if (!m_wndRename.IsWindowEnabled())
		{
			menu.EnableMenuItem(idRename, MF_GRAYED);
		}
		
		strItem.LoadString(IDS_BCGBARRES_DELETE);
		menu.AppendMenu(MF_STRING, idRemove, strItem);
		
		if (!m_wndRemove.IsWindowEnabled())
		{
			menu.EnableMenuItem(idRemove, MF_GRAYED);
		}

		menu.AppendMenu (MF_SEPARATOR);

		strItem.LoadString(IDS_BCGBARRES_MOVEUP);
		menu.AppendMenu(MF_STRING, idMoveUp, strItem);

		if (!m_wndUp.IsWindowEnabled())
		{
			menu.EnableMenuItem(idMoveUp, MF_GRAYED);
		}

		strItem.LoadString(IDS_BCGBARRES_MOVEDN);
		menu.AppendMenu(MF_STRING, idMoveDown, strItem);

		if (!m_wndDown.IsWindowEnabled())
		{
			menu.EnableMenuItem(idMoveDown, MF_GRAYED);
		}
	}

	HWND hwndThis = GetSafeHwnd ();

	CPoint pt = point;
	if (pt == CPoint(-1, -1))
	{
		CBCGPGridRow* pRow = pTreeCtrl->GetCurSel();
		if (pRow != NULL)
		{
			CRect rectRow = pRow->GetRect();
			pt  = CPoint(rectRow.left, rectRow.bottom + 1);
		}
		else
		{
			pt = CPoint (0, 0);
		}
	}

	pTreeCtrl->ClientToScreen(&pt);

	int nMenuResult = 0;
	
	if (g_pContextMenuManager != NULL)
	{
		nMenuResult = g_pContextMenuManager->TrackPopupMenu(menu, pt.x, pt.y, pTreeCtrl);
	}
	else
	{
		nMenuResult = ::TrackPopupMenu (menu, 
			TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, 
			pt.x, pt.y, 0, pTreeCtrl->GetSafeHwnd (), NULL);
	}

	if (!::IsWindow (hwndThis))
	{
		return;
	}

	switch (nMenuResult)
	{
	case idNewTab:
		OnNewTab();
		break;

	case idNewGroup:
		OnNewGroup();
		break;

	case idRename:
		OnRename();
		break;

	case idMoveUp:
		OnUp();
		break;

	case idMoveDown:
		OnDown();
		break;

	case idRemove:
		OnRemove();
		break;
	}
}

#endif // BCGP_EXCLUDE_GRID_CTRL
#endif // BCGP_EXCLUDE_RIBBON
