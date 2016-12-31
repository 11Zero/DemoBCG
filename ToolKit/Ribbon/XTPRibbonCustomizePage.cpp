// XTPRibbonCustomizePage.cpp: implementation of the CXTPRibbonCustomizePage class.
//
// This file is a part of the XTREME RIBBON MFC class library.
// (c)1998-2011 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME TOOLKIT PRO LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "Resource.h"

#include "Common/XTPResourceManager.h"

#include "CommandBars/XTPControls.h"
#include "CommandBars/XTPControl.h"
#include "CommandBars/XTPControlPopup.h"

#include "CommandBars/Resource.h"
#include "CommandBars/XTPCommandBars.h"
#include "CommandBars/XTPCustomizeSheet.h"
#include "CommandBars/XTPPaintManager.h"

#include "XTPRibbonCustomizePage.h"
#include "XTPRibbonQuickAccessControls.h"
#include "XTPRibbonBar.h"
#include "XTPRibbonTab.h"
#include "XTPRibbonGroups.h"
#include "XTPRibbonGroup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXTPRibbonCustomizePage property page

CXTPRibbonCustomizePage::CXTPRibbonCustomizePage(CXTPCustomizeSheet* pSheet)
	:   CXTPPropertyPage(XTP_IDD_RIBBONCUSTOMIZE_RIBBON)
{
	m_pSheet = pSheet;
	m_pCommandBars = pSheet->GetCommandBars();
}

CXTPRibbonCustomizePage::~CXTPRibbonCustomizePage()
{
	for (int i = 0; i < m_arrCategories.GetSize(); i++)
	{
		delete m_arrCategories[i];
	}

}

void CXTPRibbonCustomizePage::DoDataExchange(CDataExchange* pDX)
{
	CXTPPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CXTPRibbonCustomizePage)
	DDX_Control(pDX, XTP_IDC_RIBBONCOMBO_CATEGORIES, m_lstCategories);
	DDX_Control(pDX, XTP_IDC_RIBBONLIST_QUICKACCESS, m_treeRibbon);
	DDX_Control(pDX, XTP_IDC_RIBBONLIST_COMMANDS, m_lstCommands);
	//}}AFX_DATA_MAP
}

#define WM_TREE_CHECKMARKCLICKED (WM_USER + 1541)


BEGIN_MESSAGE_MAP(CXTPRibbonCustomizePage, CXTPPropertyPage)
	//{{AFX_MSG_MAP(CXTPRibbonCustomizePage)
	ON_LBN_SELCHANGE(XTP_IDC_RIBBONCOMBO_CATEGORIES, OnCategoriesSelectionChanged)
	ON_LBN_DBLCLK(XTP_IDC_RIBBONLIST_COMMANDS, OnDblclkListCommands)
	ON_BN_CLICKED(XTP_IDC_RIBBONBUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(XTP_IDC_RIBBONBUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(XTP_IDC_RIBBONBUTTON_RESET, OnButtonReset)
	ON_LBN_SELCHANGE(XTP_IDC_RIBBONLIST_COMMANDS, OnCommandsSelectionChanged)
	//}}AFX_MSG_MAP
	ON_NOTIFY(NM_CLICK, XTP_IDC_RIBBONLIST_QUICKACCESS, OnRibbonTreeClick)
	ON_NOTIFY(TVN_KEYDOWN, XTP_IDC_RIBBONLIST_QUICKACCESS, OnRibbonTreeKeydown)
	ON_NOTIFY(TVN_SELCHANGED, XTP_IDC_RIBBONLIST_QUICKACCESS, OnRibbonSelChanged)

	ON_MESSAGE(WM_TREE_CHECKMARKCLICKED, OnRibbonTreeCheckChanged)
END_MESSAGE_MAP()

CXTPRibbonBar* CXTPRibbonCustomizePage::GetRibbonBar()
{
	CXTPRibbonBar* pRibbonBar = DYNAMIC_DOWNCAST(CXTPRibbonBar, m_pCommandBars->GetMenuBar());
	ASSERT(pRibbonBar);

	return pRibbonBar;
}

/////////////////////////////////////////////////////////////////////////////
// CXTPRibbonCustomizePage message handlers

BOOL CXTPRibbonCustomizePage::OnInitDialog()
{
	CXTPPropertyPage::OnInitDialog();

	for (int i = 0; i < m_arrCategories.GetSize(); i++)
	{
		int nIndex = m_lstCategories.AddString(m_arrCategories[i]->strCategory);
		m_lstCategories.SetItemData(nIndex, i);

		if (i == 0) m_lstCategories.SetCurSel(0);
	}

	CSize sz = m_pCommandBars->GetPaintManager()->DrawListBoxControl(NULL, NULL, CRect(0, 0, 0, 0), FALSE, FALSE, m_pCommandBars);
	m_lstCommands.SetItemHeight(-1, sz.cy);

	m_lstCommands.m_pCommandBars = m_pCommandBars;



	OnCategoriesSelectionChanged();
	RefreshRibbonList();

	UpdateData(FALSE);

	if (m_pCommandBars->IsLayoutRTL())
	{
		m_lstCommands.ModifyStyleEx(0, WS_EX_LAYOUTRTL);
		m_treeRibbon.ModifyStyleEx(0, WS_EX_LAYOUTRTL);
		m_lstCategories.ModifyStyleEx(0, WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR | WS_EX_RTLREADING);
	}


	SetResize(XTP_IDC_RIBBONCOMBO_CATEGORIES, XTP_ANCHOR_TOPLEFT, XTP_ANCHOR_TOPCENTER);
	SetResize(XTP_IDC_RIBBONLIST_COMMANDS, XTP_ANCHOR_TOPLEFT, XTP_ANCHOR_BOTTOMCENTER);
	SetResize(XTP_IDC_RIBBONLIST_QUICKACCESS, XTP_ANCHOR_TOPCENTER, XTP_ANCHOR_BOTTOMRIGHT);

	SetResize(XTP_IDC_RIBBONBUTTON_RESET, XTP_ANCHOR_BOTTOMCENTER, XTP_ANCHOR_BOTTOMCENTER);

	SetResize(XTP_IDC_RIBBONBUTTON_ADD,  CXTPResizePoint(.5, 0.5), CXTPResizePoint(.5, 0.5));
	SetResize(XTP_IDC_RIBBONBUTTON_REMOVE, CXTPResizePoint(.5, 0.5), CXTPResizePoint(.5, 0.5));

	GetDlgItem(XTP_IDC_RIBBONBUTTON_ADD)->EnableWindow(FALSE);
	GetDlgItem(XTP_IDC_RIBBONBUTTON_REMOVE)->EnableWindow(FALSE);
	GetDlgItem(XTP_IDC_RIBBONBUTTON_RESET)->EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CXTPRibbonCustomizePage::RefreshRibbonList()
{
	m_treeRibbon.SetRedraw(FALSE);
	m_treeRibbon.DeleteAllItems();

	m_treeRibbon.ModifyStyle(TVS_CHECKBOXES, 0); // To refresh internal Imagelist.
	m_treeRibbon.ModifyStyle(0, TVS_CHECKBOXES);

	CXTPRibbonBar* pRibbonBar = GetRibbonBar();

	for (int i = 0; i < pRibbonBar->GetTabCount(); i++)
	{
		CXTPRibbonTab* pTab = pRibbonBar->GetTab(i);

		CString strCaption(pTab->GetCaption());
		XTPDrawHelpers()->StripMnemonics(strCaption);

		HTREEITEM hItemTab = m_treeRibbon.InsertItem(strCaption);
		m_treeRibbon.SetCheck(hItemTab, pTab->IsVisible());
		m_treeRibbon.SetItemData(hItemTab, (DWORD_PTR)pTab);

		for (int g = 0; g < pTab->GetGroups()->GetCount(); g++)
		{
			CXTPRibbonGroup* pGroup = pTab->GetGroups()->GetAt(g);

			CString strCaption(pGroup->GetCaption());
			XTPDrawHelpers()->StripMnemonics(strCaption);

			HTREEITEM hItemGroup = m_treeRibbon.InsertItem(strCaption, hItemTab);
			m_treeRibbon.SetCheck(hItemGroup, pGroup->IsVisible());
			m_treeRibbon.SetItemData(hItemGroup, (DWORD_PTR)pGroup);

			for(int c = 0; c < pGroup->GetCount(); c++)
			{
				CXTPControl* pControl = pGroup->GetAt(c);

				CString strCaption(pControl->GetCaption());
				XTPDrawHelpers()->StripMnemonics(strCaption);

				HTREEITEM hItemControl = m_treeRibbon.InsertItem(strCaption, hItemGroup);
				m_treeRibbon.SetCheck(hItemControl, pControl->IsVisible());
				m_treeRibbon.SetItemData(hItemControl, (DWORD_PTR)pControl);
			}
		}
	}


	m_treeRibbon.SetRedraw(TRUE);

	OnRibbonSelectionChanged();
	OnCommandsSelectionChanged();
}

void CXTPRibbonCustomizePage::OnCategoriesSelectionChanged()
{

	m_lstCommands.ResetContent();

	int nIndex = m_lstCategories.GetCurSel();

	if (nIndex == LB_ERR)
		return;

	XTP_COMMANDBARS_CATEGORYINFO* pInfo = GetCategoryInfo((int)m_lstCategories.GetItemData(nIndex));

	if (pInfo == NULL)
		return;

	for (int i = 0; i < pInfo->pControls->GetCount(); i++)
	{
		CXTPControl* pControl = pInfo->pControls->GetAt(i);
		m_lstCommands.SendMessage(LB_INSERTSTRING, m_lstCommands.GetCount(), (LPARAM)pControl);
	}
	OnCommandsSelectionChanged();
}


XTP_COMMANDBARS_CATEGORYINFO* CXTPRibbonCustomizePage::FindCategory(LPCTSTR strCategory) const
{
	for (int i = 0; i < m_arrCategories.GetSize(); i++)
	{
		if (m_arrCategories[i]->strCategory.Compare(strCategory) == 0)
			return m_arrCategories[i];
	}
	return NULL;
}

XTP_COMMANDBARS_CATEGORYINFO* CXTPRibbonCustomizePage::GetCategoryInfo(int nIndex)
{
	if (nIndex >= 0 && nIndex < m_arrCategories.GetSize())
		return m_arrCategories[nIndex];
	return NULL;
}

BOOL CXTPRibbonCustomizePage::AddCategory(LPCTSTR strCategory, CMenu* pMenu, BOOL bListSubItems)
{
	CXTPControls* pCategoryControls = InsertCategory(strCategory);

	int nCount = pMenu->GetMenuItemCount();

	for (int i = 0; i < nCount; i++)
	{
		if (pMenu->GetMenuItemID(i) > 0)
		{
			CXTPControlPopup* pControl = DYNAMIC_DOWNCAST(CXTPControlPopup,
				pCategoryControls->AddMenuItem(pMenu, i));

			if (pControl && bListSubItems)
			{
				CXTPControls* pControls = pControl->GetCommandBar()->GetControls();
				for (int j = 0; j < pControls->GetCount(); j++)
				{
					pCategoryControls->AddClone(pControls->GetAt(j));
				}
			}
		}
	}

	return TRUE;
}


BOOL CXTPRibbonCustomizePage::AddCategories(UINT nIDResource, BOOL bListSubItems)
{
	CMenu menu;
	if (!XTPResourceManager()->LoadMenu(&menu, nIDResource))
		return FALSE;

	int nCount = menu.GetMenuItemCount();

	for (int i = 0; i < nCount; i++)
	{
		CString strCategory;
		if (menu.GetMenuString(i, strCategory, MF_BYPOSITION) > 0)
		{
			CMenu* pMenu = menu.GetSubMenu(i);
			if (pMenu)
			{
				CXTPPaintManager::StripMnemonics(strCategory);

				if (!AddCategory(strCategory, pMenu, bListSubItems))
					return FALSE;
			}
		}

	}

	return TRUE;
}

BOOL CXTPRibbonCustomizePage::AddCategories(CXTPControls* pControls)
{
	for (int i = 0; i < pControls->GetCount(); i++)
	{
		CXTPControl* pControl = pControls->GetAt(i);
		CString strCategory = pControl->GetCategory();

		if (!strCategory.IsEmpty())
		{
			CXTPControls* pCategoryControls = InsertCategory(strCategory);
			pCategoryControls->AddClone(pControl);
		}
	}
	return TRUE;
}


CXTPControls* CXTPRibbonCustomizePage::InsertCategory(LPCTSTR strCategory, int nIndex)
{
	XTP_COMMANDBARS_CATEGORYINFO* pInfo = FindCategory(strCategory);
	if (!pInfo)
	{
		pInfo = new XTP_COMMANDBARS_CATEGORYINFO(strCategory, m_pSheet->GetCommandBars());
		m_arrCategories.InsertAt(nIndex == -1 ? m_arrCategories.GetSize() : nIndex, pInfo);
	}
	return pInfo->pControls;
}

CXTPControls* CXTPRibbonCustomizePage::GetControls(LPCTSTR strCategory)
{
	XTP_COMMANDBARS_CATEGORYINFO* pInfo = FindCategory(strCategory);
	return pInfo ? pInfo->pControls : NULL;
}


void CXTPRibbonCustomizePage::OnDblclkListCommands()
{
	OnButtonAdd();

}

void CXTPRibbonCustomizePage::OnButtonAdd()
{
}

void CXTPRibbonCustomizePage::OnButtonRemove()
{
}

void CXTPRibbonCustomizePage::OnButtonReset()
{
}

void CXTPRibbonCustomizePage::OnCommandsSelectionChanged()
{
}

void CXTPRibbonCustomizePage::OnRibbonSelectionChanged()
{
}

LRESULT CXTPRibbonCustomizePage::OnRibbonTreeCheckChanged(WPARAM, LPARAM lParam)
{
	HTREEITEM hItem = (HTREEITEM)lParam;

	CCmdTarget* pData = (CCmdTarget*)(DWORD_PTR)m_treeRibbon.GetItemData(hItem);
	if (!pData)
		return 0;

	BOOL bVisible = m_treeRibbon.GetCheck(hItem);

	CXTPRibbonTab* pTab = DYNAMIC_DOWNCAST(CXTPRibbonTab, pData);
	if (pTab)
	{
		pTab->SetVisible(bVisible);

		return 1;
	}

	CXTPRibbonGroup* pGroup = DYNAMIC_DOWNCAST(CXTPRibbonGroup, pData);
	if (pGroup)
	{
		pGroup->SetVisible(bVisible);

		return 1;
	}

	CXTPControl* pControl = DYNAMIC_DOWNCAST(CXTPControl, pData);
	if (pControl)
	{
		pControl->SetVisible(bVisible);
		return 1;
	}

	return 0;
}

void CXTPRibbonCustomizePage::OnRibbonTreeClick(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	DWORD pos = GetMessagePos();
	CPoint point(LOWORD(pos), HIWORD(pos));
	m_treeRibbon.ScreenToClient(&point);

	UINT flags;
	HTREEITEM hItem = m_treeRibbon.HitTest(point, &flags);

	if (hItem && (flags & TVHT_ONITEMSTATEICON))
	{
		PostMessage(WM_TREE_CHECKMARKCLICKED, 0, (LPARAM)hItem);
	}

	*pResult = 0;
}

void CXTPRibbonCustomizePage::OnRibbonTreeKeydown(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVKEYDOWN pTVKeyDown = (LPNMTVKEYDOWN)pNMHDR;

	if (pTVKeyDown->wVKey == VK_SPACE)
	{
		HTREEITEM hItem = m_treeRibbon.GetSelectedItem();
		if (hItem != NULL)
		{
			PostMessage(WM_TREE_CHECKMARKCLICKED, 0, (LPARAM)hItem);
		}
	}

	*pResult = 0;
}

void CXTPRibbonCustomizePage::OnRibbonSelChanged(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	OnRibbonSelectionChanged();
}
