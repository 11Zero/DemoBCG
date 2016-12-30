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
// BCGPRibbonCustomizeQATPage.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPRibbonCustomizeQATPage.h"
#include "BCGPRibbonCustomizePage.h"
#include "BCGPRibbonBar.h"
#include "BCGPRibbonCategory.h"
#include "BCGPRibbonKeyboardCustomizeDlg.h"
#include "BCGPLocalResource.h"
#include "BCGPKeyboardManager.h"
#include "BCGPRibbonCustomizePage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef BCGP_EXCLUDE_RIBBON

IMPLEMENT_DYNAMIC(CBCGPRibbonCustomGroup, CObject)

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCustomizeQATPage property page

IMPLEMENT_DYNCREATE(CBCGPRibbonCustomizeQATPage, CBCGPPropertyPage)

CBCGPRibbonCustomizeQATPage::CBCGPRibbonCustomizeQATPage(
	CBCGPRibbonBar*	pRibbonBar) : 
		CBCGPPropertyPage(CBCGPRibbonCustomizeQATPage::IDD),
		m_wndCommandsList (pRibbonBar),
		m_wndQATList (pRibbonBar, TRUE, TRUE)
{
	ASSERT_VALID (pRibbonBar);

	//{{AFX_DATA_INIT(CBCGPRibbonCustomizeQATPage)
	m_nCategory = -1;
	m_bQAToolbarOnBottom = FALSE;
	//}}AFX_DATA_INIT

	m_pRibbonBar = pRibbonBar;
	m_bQAToolbarOnBottom = !m_pRibbonBar->IsQuickAccessToolbarOnTop ();

	CBCGPLocalResource locaRes;
	m_psp.hInstance = AfxGetResourceHandle ();

	m_bIsCustomizeKeyboard = TRUE;
}
//**********************************************************************
CBCGPRibbonCustomizeQATPage::~CBCGPRibbonCustomizeQATPage()
{
	while (!m_lstCustomCategories.IsEmpty ())
	{
		delete m_lstCustomCategories.RemoveHead ();
	}
}
//**********************************************************************
void CBCGPRibbonCustomizeQATPage::DoDataExchange(CDataExchange* pDX)
{
	CBCGPPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPRibbonCustomizeQATPage)
	DDX_Control(pDX, IDC_BCGBARRES_KEYBOARD, m_wndKbdCustomize);
	DDX_Control(pDX, IDS_BCGBARRES_ADD, m_wndAdd);
	DDX_Control(pDX, IDC_BCGBARRES_REMOVE, m_wndRemove);
	DDX_Control(pDX, IDC_BCGBARRES_COMMANDS_LIST, m_wndCommandsList);
	DDX_Control(pDX, IDC_BCGBARRES_CATEGORY, m_wndCategoryCombo);
	DDX_Control(pDX, IDC_BCGBARRES_QAT_COMMANDS_LIST, m_wndQATList);
	DDX_Control(pDX, IDC_BCGBARRES_MOVEUP, m_wndUp);
	DDX_Control(pDX, IDC_BCGBARRES_MOVEDOWN, m_wndDown);
	DDX_CBIndex(pDX, IDC_BCGBARRES_CATEGORY, m_nCategory);
	DDX_Check(pDX, IDC_BCGBARRES_QAT_ON_BOTTOM, m_bQAToolbarOnBottom);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBCGPRibbonCustomizeQATPage, CBCGPPropertyPage)
	//{{AFX_MSG_MAP(CBCGPRibbonCustomizeQATPage)
	ON_CBN_SELENDOK(IDC_BCGBARRES_CATEGORY, OnSelendokCategoryCombo)
	ON_BN_CLICKED(IDS_BCGBARRES_ADD, OnAdd)
	ON_BN_CLICKED(IDC_BCGBARRES_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_BCGBARRES_MOVEUP, OnUp)
	ON_BN_CLICKED(IDC_BCGBARRES_MOVEDOWN, OnDown)
	ON_BN_CLICKED(IDC_BCGBARRES_RESET, OnToolbarReset)
	ON_LBN_SELCHANGE(IDC_BCGBARRES_QAT_COMMANDS_LIST, OnSelchangeQATCommands)
	ON_BN_CLICKED(IDC_BCGBARRES_KEYBOARD, OnCustomizeKeyboard)
	ON_LBN_DBLCLK(IDC_BCGBARRES_COMMANDS_LIST, OnAdd)
	ON_LBN_DBLCLK(IDC_BCGBARRES_QAT_COMMANDS_LIST, OnRemove)
	ON_LBN_SELCHANGE(IDC_BCGBARRES_COMMANDS_LIST, OnSelchangeCommandsList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonCustomizeQATPage message handlers

void CBCGPRibbonCustomizeQATPage::OnSelendokCategoryCombo() 
{
	ASSERT_VALID (m_pRibbonBar);

	UpdateData ();

	DWORD_PTR dwData = m_wndCategoryCombo.GetItemData (m_nCategory);
	
	if (dwData == 0)	// Separator, get next
	{
		if (m_nCategory == m_wndCategoryCombo.GetCount () - 1)
		{
			return;
		}

		m_nCategory++;
		UpdateData (FALSE);
	}

	CBCGPRibbonCustomGroup* pCustCategory = DYNAMIC_DOWNCAST(CBCGPRibbonCustomGroup, (CObject*)dwData);
	if (pCustCategory != NULL)
	{
		ASSERT_VALID (pCustCategory);

		m_wndCommandsList.FillFromIDs (pCustCategory->m_lstIDs, FALSE);
		OnSelchangeCommandsList();
	}
	else
	{
		CBCGPRibbonCategory* pCategory = DYNAMIC_DOWNCAST (
			CBCGPRibbonCategory, (CObject*)dwData);
		if (pCategory != NULL)
		{
			ASSERT_VALID (pCategory);

			m_wndCommandsList.FillFromCategory (pCategory);
			OnSelchangeCommandsList();
		}
	}
}
//**********************************************************************
void CBCGPRibbonCustomizeQATPage::OnAdd() 
{
	CBCGPBaseRibbonElement* pCmd = m_wndCommandsList.GetSelected ();
	if (pCmd == NULL)
	{
		return;
	}

	ASSERT_VALID (pCmd);

	if (pCmd->IsSeparator () && m_wndQATList.GetCount () == 0)
	{
		// Don't add separator to the empty QAT
		MessageBeep ((UINT)-1);
		return;
	}
	
	if (!m_wndQATList.AddCommand (pCmd, TRUE, FALSE))
	{
		return;
	}

	int nSel = m_wndCommandsList.GetCurSel ();
	if (nSel < m_wndCommandsList.GetCount () - 1)
	{
		m_wndCommandsList.SetCurSel (nSel + 1);
	}

	OnSelchangeQATCommands();
	OnSelchangeCommandsList();
}
//**********************************************************************
void CBCGPRibbonCustomizeQATPage::OnRemove() 
{
	int nIndex = m_wndQATList.GetCurSel ();
	if (nIndex >= 0)
	{
		m_wndQATList.DeleteString (nIndex);

		nIndex = min (nIndex, m_wndQATList.GetCount () - 1);

		if (nIndex >= 0)
		{
			m_wndQATList.SetCurSel (nIndex);
		}
	}

	OnSelchangeQATCommands();
	OnSelchangeCommandsList();
}
//**********************************************************************
void CBCGPRibbonCustomizeQATPage::OnUp() 
{
	MoveItem (TRUE);
}
//**********************************************************************
void CBCGPRibbonCustomizeQATPage::OnDown() 
{
	MoveItem (FALSE);
}
//**********************************************************************
void CBCGPRibbonCustomizeQATPage::OnToolbarReset() 
{
	CString strPrompt;

	{
		CBCGPLocalResource locaRes;

		CString strCaption;
		strPrompt.Format (IDS_BCGBARRES_RESET_TOOLBAR_FMT, strCaption);

		strPrompt.Remove (_T('\''));
		strPrompt.Remove (_T('\''));
	}

	if (MessageBox (strPrompt, NULL, MB_OKCANCEL | MB_ICONWARNING) != IDOK)
	{
		return;
	}

	CList<UINT,UINT> lstCmds;
	m_pRibbonBar->m_QAToolbar.GetDefaultCommands (lstCmds);

	m_wndQATList.FillFromIDs (lstCmds, FALSE);
}
//**********************************************************************
void CBCGPRibbonCustomizeQATPage::OnSelchangeQATCommands() 
{
	m_wndUp.EnableWindow (m_wndQATList.GetCurSel () > 0);
	m_wndDown.EnableWindow (m_wndQATList.GetCurSel () < m_wndQATList.GetCount () - 1);
	m_wndRemove.EnableWindow (m_wndQATList.GetCurSel () >= 0);
}
//**********************************************************************
BOOL CBCGPRibbonCustomizeQATPage::OnInitDialog() 
{
	CBCGPPropertyPage::OnInitDialog();
	
	ASSERT_VALID (m_pRibbonBar);

	CBCGPStaticLayout* pLayout = (CBCGPStaticLayout*)GetLayout ();
	if (pLayout != NULL)
	{
		pLayout->AddAnchor (IDC_BCGBARRES_CATEGORY, CBCGPStaticLayout::e_MoveTypeNone, CBCGPStaticLayout::e_SizeTypeHorz, CSize(0, 0), CSize(50, 100));
		pLayout->AddAnchor (IDC_BCGBARRES_COMMANDS_LIST, CBCGPStaticLayout::e_MoveTypeNone, CBCGPStaticLayout::e_SizeTypeBoth, CSize(0, 0), CSize(50, 100));
		pLayout->AddAnchor (IDC_BCGBARRES_QAT_ON_BOTTOM, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeNone);
		pLayout->AddAnchor (IDC_BCGBARRES_KEYBOARD, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeNone);
		pLayout->AddAnchor (IDC_BCGBARRES_ACCEL_LABEL, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeNone);
		pLayout->AddAnchor (IDS_BCGBARRES_ADD, CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeNone, CSize(50, 100));
		pLayout->AddAnchor (IDC_BCGBARRES_REMOVE, CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeNone, CSize(50, 100));
		pLayout->AddAnchor (IDC_BCGBARRES_QAT_COMMANDS_LIST, CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeBoth, CSize(50, 0), CSize(50, 100));
		pLayout->AddAnchor (IDC_BCGBARRES_MOVEUP, CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeNone);
		pLayout->AddAnchor (IDC_BCGBARRES_MOVEDOWN, CBCGPStaticLayout::e_MoveTypeHorz, CBCGPStaticLayout::e_SizeTypeNone);
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

	const CString strSeparator = _T("----------");

	m_wndUp.SetStdImage (CBCGPMenuImages::IdArowUpLarge, CBCGPMenuImages::ImageBlack2, CBCGPMenuImages::IdArowUpLarge, CBCGPMenuImages::ImageLtGray);
	m_wndUp.SetWindowText(_T("Up"));
	m_wndUp.SetDrawText(FALSE, FALSE);
	
	m_wndDown.SetStdImage (CBCGPMenuImages::IdArowDownLarge, CBCGPMenuImages::ImageBlack2, CBCGPMenuImages::IdArowDownLarge, CBCGPMenuImages::ImageLtGray);
	m_wndDown.SetWindowText(_T("Down"));
	m_wndDown.SetDrawText(FALSE, FALSE);

	//-----------------------
	// Add custom categories:
	//-----------------------
	for (POSITION pos = m_lstCustomCategories.GetHeadPosition (); pos != NULL;)
	{
		CBCGPRibbonCustomGroup* pCustCategory = m_lstCustomCategories.GetNext (pos);
		ASSERT_VALID (pCustCategory);

		int nIndex = m_wndCategoryCombo.AddString (pCustCategory->m_strName);
		m_wndCategoryCombo.SetItemData (nIndex, (DWORD_PTR) pCustCategory);
	}

	if (m_wndCategoryCombo.GetCount () > 0)
	{
		m_wndCategoryCombo.AddString (strSeparator);
	}

	//-------------------
	// Add main category:
	//-------------------
	CBCGPRibbonCategory* pMainCategory = m_pRibbonBar->GetMainCategory ();
	if (pMainCategory != NULL)
	{
		ASSERT_VALID (pMainCategory);
		
		int nIndex = m_wndCategoryCombo.AddString (pMainCategory->GetName ());
		m_wndCategoryCombo.SetItemData (nIndex, (DWORD_PTR) pMainCategory);
		m_wndCategoryCombo.AddString (strSeparator);
	}

	int i = 0;
	BOOL bHasContextCategories = FALSE;

	//----------------------------
	// Add non-context categories:
	//----------------------------
	for (i = 0; i < m_pRibbonBar->GetCategoryCount (); i++)
	{
		CBCGPRibbonCategory* pCategory = m_pRibbonBar->GetCategory (i);
		ASSERT_VALID (pCategory);

		CString strCategoryName;
		if (!m_pRibbonBar->m_CustomizationData.GetTabName(pCategory, strCategoryName))
		{
			strCategoryName = pCategory->GetName ();
		}

		if (pCategory->GetContextID () == 0)
		{
			int nIndex = m_wndCategoryCombo.AddString (strCategoryName);
			m_wndCategoryCombo.SetItemData (nIndex, (DWORD_PTR) pCategory);
		}
		else
		{
			bHasContextCategories = TRUE;
		}
	}

	if (bHasContextCategories)
	{
		//------------------------
		// Add context categories:
		//------------------------
		m_wndCategoryCombo.AddString (strSeparator);

		for (i = 0; i < m_pRibbonBar->GetCategoryCount (); i++)
		{
			CBCGPRibbonCategory* pCategory = m_pRibbonBar->GetCategory (i);
			ASSERT_VALID (pCategory);

			CString strCategoryName;
			if (!m_pRibbonBar->m_CustomizationData.GetTabName(pCategory, strCategoryName))
			{
				strCategoryName = pCategory->GetName ();
			}

			const UINT uiContextID = pCategory->GetContextID ();

			if (uiContextID != 0)
			{
				CString strName;
				CString strContext;

				if (m_pRibbonBar->GetContextName (uiContextID, strContext))
				{
					strName = strContext + _T(" | ") + strCategoryName;
				}
				else
				{
					strName = strCategoryName;
				}

				int nIndex = m_wndCategoryCombo.AddString (strName);
				m_wndCategoryCombo.SetItemData (nIndex, (DWORD_PTR) pCategory);
			}
		}
	}

	if (m_wndCategoryCombo.GetCount () > 0)
	{
		m_nCategory = 0;
		UpdateData (FALSE);

		OnSelendokCategoryCombo ();
	}

	CList<UINT,UINT> lstQACommands;
	m_pRibbonBar->GetQuickAccessCommands (lstQACommands);

	m_wndQATList.FillFromIDs (lstQACommands, FALSE);

	OnSelchangeQATCommands ();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//**********************************************************************
void CBCGPRibbonCustomizeQATPage::MoveItem (BOOL bMoveUp) 
{
	int nSel = m_wndQATList.GetCurSel ();

	CString str;
	m_wndQATList.GetText (nSel, str);

	DWORD_PTR dwData = m_wndQATList.GetItemData (nSel);

	m_wndQATList.DeleteString (nSel);

	int nNewIndex = bMoveUp ? nSel - 1 : nSel + 1;

	int nIndex = m_wndQATList.InsertString (nNewIndex, str);

	m_wndQATList.SetItemData (nIndex, dwData);

	m_wndQATList.SetCurSel (nIndex);
	OnSelchangeQATCommands();
}
//**********************************************************************
void CBCGPRibbonCustomizeQATPage::OnOK() 
{
	UpdateData ();

	ASSERT_VALID (m_pRibbonBar);

	CList<UINT,UINT> lstQACommands;

	for (int i = 0; i < m_wndQATList.GetCount (); i++)
	{
		lstQACommands.AddTail (m_wndQATList.GetCommand (i)->GetID ());
	}

	m_pRibbonBar->OnCancelMode ();
	m_pRibbonBar->m_QAToolbar.ReplaceCommands (lstQACommands);
	m_pRibbonBar->SetQuickAccessToolbarOnTop (!m_bQAToolbarOnBottom);

	m_pRibbonBar->RecalcLayout ();

	CFrameWnd* pParentFrame = m_pRibbonBar->GetParentFrame ();
	
	if (pParentFrame->GetSafeHwnd () != NULL)
	{
		pParentFrame->RecalcLayout ();
		pParentFrame->RedrawWindow ();
	}

	CBCGPPropertyPage::OnOK();
}
//**********************************************************************
void CBCGPRibbonCustomizeQATPage::OnCustomizeKeyboard()
{
	ASSERT_VALID (m_pRibbonBar);

	CBCGPRibbonKeyboardCustomizeDlg dlg (m_pRibbonBar, this);
	dlg.DoModal ();
}
//**********************************************************************
void CBCGPRibbonCustomizeQATPage::OnSelchangeCommandsList() 
{
	BOOL bEnableAddButton = TRUE;

	CBCGPBaseRibbonElement* pCmd = m_wndCommandsList.GetSelected ();
	if (pCmd == NULL)
	{
		bEnableAddButton = FALSE;
	}
	else
	{
		ASSERT_VALID (pCmd);
		bEnableAddButton = 
			pCmd->GetID () == 0 || m_wndQATList.GetCommandIndex (pCmd->GetID ()) < 0;
	}

	m_wndAdd.EnableWindow (bEnableAddButton);
}
//**********************************************************************
void CBCGPRibbonCustomizeQATPage::AddCustomCategory (LPCTSTR lpszName,
												  const CList<UINT, UINT>& lstIDS)
{
	ASSERT_VALID (this);
	ASSERT (lpszName != NULL);
	ASSERT (GetSafeHwnd () == NULL);

	CBCGPRibbonCustomGroup* pCategory = new CBCGPRibbonCustomGroup;
	pCategory->m_strName = lpszName;

	pCategory->m_lstIDs.AddHead ((UINT)0);	// Separator
	pCategory->m_lstIDs.AddTail ((CList<UINT,UINT>*)&lstIDS);

	m_lstCustomCategories.AddTail (pCategory);
}
//**********************************************************************
void CBCGPRibbonCustomizeQATPage::EnableKeyboradCustomization (BOOL bEnable/* = TRUE*/)
{
	ASSERT_VALID (this);
	ASSERT (GetSafeHwnd () == NULL);

	m_bIsCustomizeKeyboard = bEnable;
}

#endif // BCGP_EXCLUDE_RIBBON
