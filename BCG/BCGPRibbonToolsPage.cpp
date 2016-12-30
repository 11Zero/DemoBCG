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
// BCGPRibbonToolsPage.cpp : implementation file
//

#include "stdafx.h"
#include "BCGCBPro.h"
#include "BCGPLocalResource.h"
#include "bcgprores.h"
#include "BCGPRibbonToolsPage.h"
#include "BCGPUserToolsManager.h"
#include "BCGPRibbonBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef BCGP_EXCLUDE_RIBBON

static const int iBtnNew = 0;
static const int iBtnDelete = 1;
static const int iBtnMoveUp = 2;
static const int iBtnMoveDn = 3;

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonToolsPage

#pragma warning (disable : 4355)

CBCGPRibbonToolsPage::CBCGPRibbonToolsPage(CBCGPRibbonBar* pParentRibbon) :
	CBCGPPropertyPage(CBCGPRibbonToolsPage::IDD),
	m_pParentRibbon(pParentRibbon),
	m_wndToolsList (this),
	m_ToolsManager(*g_pUserToolsManager)
{
	//{{AFX_DATA_INIT(CBCGPRibbonToolsPage)
	m_strCommand = _T("");
	m_strArguments = _T("");
	m_strInitialDirectory = _T("");
	//}}AFX_DATA_INIT
	
	m_pSelTool = NULL;

	CBCGPLocalResource locaRes;
	m_psp.hInstance = AfxGetResourceHandle ();

	EnableLayout();
}

CBCGPRibbonToolsPage::~CBCGPRibbonToolsPage()
{
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonToolsPage message handlers

void CBCGPRibbonToolsPage::DoDataExchange(CDataExchange* pDX)
{
	CBCGPPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPRibbonToolsPage)
	DDX_Control(pDX, IDD_BCGBARRES_MENU_INITIAL_DIRECTORY, m_wndInitialDirBtn);
	DDX_Control(pDX, IDD_BCGBARRES_MENU_ARGUMENTS, m_wndArgumentsBtn);
	DDX_Control(pDX, IDD_BCGBARRES_ARGUMENTS, m_wndArgumentsEdit);
	DDX_Control(pDX, IDD_BCGBARRES_INITIAL_DIRECTORY, m_wndInitialDirEdit);
	DDX_Control(pDX, IDD_BCGBARRES_COMMAND, m_wndCommandEdit);
	DDX_Control(pDX, IDD_BCGBARRES_BROWSE_COMMAND, m_wndBrowseBtn);
	DDX_Control(pDX, IDD_BCGBARRES_COMMANDS_LIST, m_wndToolsList);
	DDX_Text(pDX, IDD_BCGBARRES_COMMAND, m_strCommand);
	DDX_Text(pDX, IDD_BCGBARRES_ARGUMENTS, m_strArguments);
	DDX_Text(pDX, IDD_BCGBARRES_INITIAL_DIRECTORY, m_strInitialDirectory);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGPRibbonToolsPage, CBCGPPropertyPage)
	//{{AFX_MSG_MAP(CBCGPRibbonToolsPage)
	ON_BN_CLICKED(IDD_BCGBARRES_BROWSE_COMMAND, OnBcgbarresBrowseCommand)
	ON_EN_UPDATE(IDD_BCGBARRES_ARGUMENTS, OnUpdateTool)
	ON_BN_CLICKED(IDD_BCGBARRES_MENU_ARGUMENTS, OnArgumentsOptions)
	ON_BN_CLICKED(IDD_BCGBARRES_MENU_INITIAL_DIRECTORY, OnInitialDirectoryOptions)
	ON_EN_UPDATE(IDD_BCGBARRES_COMMAND, OnUpdateTool)
	ON_EN_UPDATE(IDD_BCGBARRES_INITIAL_DIRECTORY, OnUpdateTool)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPRibbonToolsPage message handlers

BOOL CBCGPRibbonToolsPage::OnInitDialog() 
{
	CBCGPPropertyPage::OnInitDialog();

	CBCGPStaticLayout* pLayout = (CBCGPStaticLayout*)GetLayout ();
	if (pLayout != NULL)
	{
		pLayout->AddAnchor (IDD_BCGBARRES_COMMANDS_LIST, CBCGPStaticLayout::e_MoveTypeNone, CBCGPStaticLayout::e_SizeTypeBoth);
		
		pLayout->AddAnchor (IDD_BCGBAR_RES_LABEL1, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeNone);
		pLayout->AddAnchor (IDD_BCGBARRES_COMMAND, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeHorz);
		pLayout->AddAnchor (IDD_BCGBARRES_BROWSE_COMMAND, CBCGPStaticLayout::e_MoveTypeBoth, CBCGPStaticLayout::e_SizeTypeNone);

		pLayout->AddAnchor (IDD_BCGBAR_RES_LABEL2, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeNone);
		pLayout->AddAnchor (IDD_BCGBARRES_ARGUMENTS, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeHorz);
		pLayout->AddAnchor (IDD_BCGBARRES_MENU_ARGUMENTS, CBCGPStaticLayout::e_MoveTypeBoth, CBCGPStaticLayout::e_SizeTypeNone);

		pLayout->AddAnchor (IDD_BCGBAR_RES_LABEL3, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeNone);
		pLayout->AddAnchor (IDD_BCGBARRES_INITIAL_DIRECTORY, CBCGPStaticLayout::e_MoveTypeVert, CBCGPStaticLayout::e_SizeTypeHorz);
		pLayout->AddAnchor (IDD_BCGBARRES_MENU_INITIAL_DIRECTORY, CBCGPStaticLayout::e_MoveTypeBoth, CBCGPStaticLayout::e_SizeTypeNone);
	}

	//-------------
	// Add buttons:
	//-------------
	m_wndToolsList.SetStandardButtons ();

	//------------
	// Fill tools:
	//------------
	const CObList& lstTools = m_ToolsManager.GetUserTools ();
	for (POSITION pos = lstTools.GetHeadPosition (); pos != NULL;)
	{
		CBCGPUserTool* pTool = (CBCGPUserTool*) lstTools.GetNext (pos);
		ASSERT_VALID (pTool);

		m_wndToolsList.AddItem (pTool->m_strLabel, (DWORD_PTR) pTool);
	}

	UINT uMenuID = 0;
	uMenuID = m_ToolsManager.GetInitialDirMenuID ();
	if (uMenuID)
	{
		m_wndInitialDirBtn.ShowWindow(SW_SHOW);
		m_menuInitialDir.LoadMenu (uMenuID);
		m_wndInitialDirBtn.m_hMenu = m_menuInitialDir.GetSubMenu (0)->GetSafeHmenu ();
	}

	uMenuID = m_ToolsManager.GetArgumentsMenuID ();
	if (uMenuID)
	{
		m_wndArgumentsBtn.ShowWindow(SW_SHOW);
		m_menuArguments.LoadMenu (uMenuID);
		m_wndArgumentsBtn.m_hMenu = m_menuArguments.GetSubMenu (0)->GetSafeHmenu ();
	}

	m_wndInitialDirBtn.m_bRightArrow = TRUE;
	m_wndArgumentsBtn.m_bRightArrow  = TRUE;

	EnableControls ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//***************************************************************************
BOOL CRibbonToolsList::OnBeforeRemoveItem (int iItem)
{
	CBCGPUserTool* pTool = (CBCGPUserTool*) GetItemData (iItem);
	ASSERT_VALID (pTool);

	m_pParent->m_ToolsManager.RemoveTool (pTool);
	m_pParent->m_pSelTool = NULL;

	return TRUE;
}
//***************************************************************************
void CRibbonToolsList::OnAfterAddItem (int iItem)
{
	CBCGPUserTool* pTool = m_pParent->CreateNewTool ();
	if (pTool == NULL)
	{
		RemoveItem (iItem);
		return;
	}

	ASSERT_VALID (pTool);

	pTool->m_strLabel = GetItemText (iItem);
	SetItemData (iItem, (DWORD_PTR) pTool);

	OnSelectionChanged ();
}
//***************************************************************************
void CRibbonToolsList::OnAfterRenameItem (int iItem)
{
	CBCGPUserTool* pTool = (CBCGPUserTool*) GetItemData (iItem);
	ASSERT_VALID (pTool);

	pTool->m_strLabel = GetItemText (iItem);
}
//***************************************************************************
void CRibbonToolsList::OnAfterMoveItemUp (int iItem)
{
	CBCGPUserTool* pTool = (CBCGPUserTool*) GetItemData (iItem);
	ASSERT_VALID (pTool);

	m_pParent->m_ToolsManager.MoveToolUp (pTool);
}
//***************************************************************************
void CRibbonToolsList::OnAfterMoveItemDown (int iItem)
{
	CBCGPUserTool* pTool = (CBCGPUserTool*) GetItemData (iItem);
	ASSERT_VALID (pTool);

	m_pParent->m_ToolsManager.MoveToolDown (pTool);
}
//**************************************************************************
void CRibbonToolsList::OnSelectionChanged ()
{
	int iSelItem = GetSelItem ();
	CBCGPUserTool* pSelTool = (iSelItem < 0) ? 
		NULL : (CBCGPUserTool*) GetItemData (iSelItem);

	if (pSelTool == NULL)
	{
		m_pParent->m_strCommand.Empty ();
		m_pParent->m_strArguments.Empty ();
		m_pParent->m_strInitialDirectory.Empty ();
	}
	else
	{
		ASSERT_VALID (pSelTool);

		m_pParent->m_strCommand          = pSelTool->GetCommand ();
		m_pParent->m_strArguments        = pSelTool->m_strArguments;
		m_pParent->m_strInitialDirectory = pSelTool->m_strInitialDirectory;
	}

	m_pParent->m_pSelTool = pSelTool;
	m_pParent->UpdateData (FALSE);

	m_pParent->EnableControls ();
}
//**************************************************************************
void CBCGPRibbonToolsPage::OnBcgbarresBrowseCommand() 
{
	CFileDialog dlg (TRUE, m_ToolsManager.GetDefExt (), NULL, 0,
		m_ToolsManager.GetFilter (), this);
	if (dlg.DoModal () == IDOK)
	{
		m_strCommand = dlg.GetPathName ();
		UpdateData (FALSE);
		OnUpdateTool();
	}
}
//**************************************************************************
void CBCGPRibbonToolsPage::OnUpdateTool() 
{
	UpdateData ();

	int iSelItem = m_wndToolsList.GetSelItem ();
	CBCGPUserTool* pSelTool = (iSelItem >= 0) ?
		(CBCGPUserTool*) m_wndToolsList.GetItemData (iSelItem) : NULL;

	if (pSelTool == NULL)
	{
		m_strCommand.Empty ();
		m_strArguments.Empty ();
		m_strInitialDirectory.Empty ();

		UpdateData (FALSE);
	}
	else
	{
		ASSERT_VALID (pSelTool);

		pSelTool->SetCommand (m_strCommand);
		pSelTool->m_strArguments = m_strArguments;
		pSelTool->m_strInitialDirectory = m_strInitialDirectory;
	}

	EnableControls ();
}
//******************************************************************************
CBCGPUserTool* CBCGPRibbonToolsPage::CreateNewTool ()
{
	const int nMaxTools = m_ToolsManager.GetMaxTools ();

	if (m_ToolsManager.GetUserTools ().GetCount () == nMaxTools)
	{
		CBCGPLocalResource locaRes;
		
		CString strError;
		strError.Format (IDS_BCGBARRES_TOO_MANY_TOOLS_FMT, nMaxTools);

		MessageBox (strError);
		return NULL;
	}

	CBCGPUserTool* pTool = m_ToolsManager.CreateNewTool ();
	ASSERT_VALID (pTool);

	return pTool;
}
//*******************************************************************************
void CBCGPRibbonToolsPage::OnOK() 
{
	OnUpdateTool();
	g_pUserToolsManager->ApplyChanges(m_ToolsManager);

	if (m_pParentRibbon != NULL)
	{
		m_pParentRibbon->ForceRecalcLayout();
		m_pParentRibbon->RedrawWindow();
	}

	CBCGPPropertyPage::OnOK();
}
//******************************************************************************
BOOL CBCGPRibbonToolsPage::OnKillActive() 
{
	ASSERT_VALID (this);
	return CBCGPPropertyPage::OnKillActive();
}
//******************************************************************************
void CBCGPRibbonToolsPage::EnableControls ()
{
	BOOL bEnableItemProps = (m_wndToolsList.GetSelItem () >= 0);

	m_wndCommandEdit.EnableWindow (bEnableItemProps);
	m_wndArgumentsEdit.EnableWindow (bEnableItemProps);
	m_wndInitialDirEdit.EnableWindow (bEnableItemProps);
	m_wndBrowseBtn.EnableWindow (bEnableItemProps);

	m_wndInitialDirBtn.EnableWindow (bEnableItemProps);
	m_wndArgumentsBtn.EnableWindow (bEnableItemProps);
}
//******************************************************************************
void CBCGPRibbonToolsPage::OnArgumentsOptions () 
{
	if (m_wndArgumentsBtn.m_nMenuResult != 0)
	{
		CString strItem;
		strItem.LoadString (m_wndArgumentsBtn.m_nMenuResult);

		//-----------------------------------------------
		// Insert text to the current arguments position:
		//-----------------------------------------------
		for (int i = 0; i < strItem.GetLength (); i++)
		{
			m_wndArgumentsEdit.SendMessage (WM_CHAR, (TCHAR) strItem [i]);
		}
	}
}
//******************************************************************************
void CBCGPRibbonToolsPage::OnInitialDirectoryOptions () 
{
	if (m_wndInitialDirBtn.m_nMenuResult != 0)
	{
		CString strItem;
		strItem.LoadString (m_wndInitialDirBtn.m_nMenuResult);

		//-----------------------------------------------
		// Insert text to the current directory position:
		//-----------------------------------------------
		for (int i = 0; i < strItem.GetLength (); i++)
		{
			m_wndInitialDirEdit.SendMessage (WM_CHAR, (TCHAR) strItem [i]);
		}
	}
}

#endif // BCGP_EXCLUDE_RIBBON
