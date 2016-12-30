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

// CBCGPToolbarCustomizePages.cpp : implementation file
//

#include "stdafx.h"

#include <afxpriv.h>
#include "bcgprores.h"
#include "BCGPToolbarCustomize.h"
#include "BCGPToolbarCustomizePages.h"
#include "BCGPToolbar.h"
#include "BCGPToolbarButton.h"
#include "BCGPLocalResource.h"
#include "BCGPPopupMenuBar.h"
#include "ToolbarNameDlg.h"
#include "BCGPCommandManager.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPOleIPFrameWnd.h"
#include "BCGPFrameWnd.h"
#include "BCGPDropDown.h"
#include "BCGPMiniFrameWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CBCGPCustomizePage, CPropertyPage)
IMPLEMENT_DYNCREATE(CBCGPToolbarsPage, CPropertyPage)

extern CObList	gAllToolbars;

/////////////////////////////////////////////////////////////////////////////
// CBCGPCustomizePage property page

CBCGPCustomizePage::CBCGPCustomizePage() : 
	CPropertyPage(CBCGPCustomizePage::IDD)
{
	//{{AFX_DATA_INIT(CBCGPCustomizePage)
	m_strButtonDescription = _T("");
	//}}AFX_DATA_INIT
}

CBCGPCustomizePage::~CBCGPCustomizePage()
{
}

void CBCGPCustomizePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPCustomizePage)
	DDX_Control(pDX, IDC_BCGBARRES_CATEGORY, m_wndCategory);
	DDX_Control(pDX, IDC_BCGBARRES_USER_TOOLS, m_wndTools);
	DDX_Text(pDX, IDC_BCGBARRES_BUTTON_DESCR, m_strButtonDescription);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGPCustomizePage, CPropertyPage)
	//{{AFX_MSG_MAP(CBCGPCustomizePage)
	ON_LBN_SELCHANGE(IDC_BCGBARRES_USER_TOOLS, OnSelchangeUserTools)
	ON_LBN_SELCHANGE(IDC_BCGBARRES_CATEGORY, OnSelchangeCategory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CBCGPCustomizePage::OnSelchangeCategory() 
{
	UpdateData ();

	int iIndex = m_wndCategory.GetCurSel ();
	if (iIndex == LB_ERR)
	{
		ASSERT (FALSE);
		return;
	}

	CWaitCursor wait;
	m_wndTools.SetRedraw (FALSE);

	m_wndTools.ResetContent ();

	//------------------------------------------
	// Only "All commands" list shoud be sorted!
	//------------------------------------------
	CString strCategory;
	m_wndCategory.GetText (iIndex, strCategory);

	BOOL bAllCommands =  (strCategory == m_strAllCategory);

	OnChangeSelButton (NULL);

	CObList* pCategoryButtonsList = 
		(CObList*) m_wndCategory.GetItemData (iIndex);
	ASSERT_VALID (pCategoryButtonsList);

	CBCGPToolbarCustomize* pWndParent = DYNAMIC_DOWNCAST (CBCGPToolbarCustomize, GetParent ());
	ASSERT (pWndParent != NULL);

	for (POSITION pos = pCategoryButtonsList->GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pCategoryButtonsList->GetNext (pos);
		ASSERT (pButton != NULL);

		pButton->m_bUserButton = pButton->m_nID != (UINT) -1 &&
			BCGPCMD_MGR.GetCmdImage (pButton->m_nID, FALSE) == -1;

		CString strText = pButton->m_strText;

		if (!pButton->m_strTextCustom.IsEmpty () &&
			(bAllCommands || pWndParent->GetCountInCategory (strText, *pCategoryButtonsList) > 1))
		{
			strText = pButton->m_strTextCustom;
		}

		int iIndex = -1;
		
		if (bAllCommands)
		{
			// Insert sortable:
			for (int i = 0; iIndex == -1 && i < m_wndTools.GetCount (); i ++)
			{
				CString strCommand;
				m_wndTools.GetText (i, strCommand);

				if (strCommand > strText)
				{
					iIndex = m_wndTools.InsertString (i, strText);
				}
			}
		}

		if (iIndex == -1)	// Not inserted yet
		{
			iIndex = m_wndTools.AddString (strText);
		}

		m_wndTools.SetItemData (iIndex, (DWORD_PTR) pButton);
	}

	m_wndTools.SetRedraw (TRUE);
}
//**************************************************************************************
void CBCGPCustomizePage::OnSelchangeUserTools() 
{
	int iIndex = m_wndTools.GetCurSel ();
	if (iIndex == LB_ERR)
	{
		OnChangeSelButton (NULL);
	}
	else
	{
		OnChangeSelButton ((CBCGPToolbarButton*) m_wndTools.GetItemData (iIndex));
	}
}
//**************************************************************************************
BOOL CBCGPCustomizePage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	CBCGPToolbarCustomize* pWndParent = DYNAMIC_DOWNCAST (CBCGPToolbarCustomize, GetParent ());
	ASSERT (pWndParent != NULL);

	pWndParent->FillCategoriesListBox (m_wndCategory);
	
	m_wndCategory.SetCurSel (0);
	OnSelchangeCategory ();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//**********************************************************************************************
void CBCGPCustomizePage::OnChangeSelButton (CBCGPToolbarButton* pSelButton)
{
	m_strButtonDescription = _T("");

	if (pSelButton != NULL)
	{
		if (pSelButton->m_nID == 0)
		{
			m_strButtonDescription = pSelButton->m_strText;
		}
		else
		{
			CFrameWnd* pParent = GetParentFrame ();
			if (pParent != NULL && pParent->GetSafeHwnd () != NULL)
			{
				pParent->GetMessageString (pSelButton->m_nID,
							m_strButtonDescription);
			}
		}
	}

	m_pSelButton = pSelButton;
	UpdateData (FALSE);
}
//*************************************************************************************
void CBCGPCustomizePage::SetUserCategory (LPCTSTR lpszCategory)
{
	ASSERT (lpszCategory != NULL);
	m_strUserCategory = lpszCategory;
}
//*************************************************************************************
void CBCGPCustomizePage::SetAllCategory (LPCTSTR lpszCategory)
{
	ASSERT (lpszCategory != NULL);
	m_strAllCategory = lpszCategory;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolbarsPage property page

CBCGPToolbarsPage::CBCGPToolbarsPage(CFrameWnd* pParentFrame) : 
	CPropertyPage(CBCGPToolbarsPage::IDD),
	m_bUserDefinedToolbars (FALSE),
	m_pParentFrame (pParentFrame)
{
	//{{AFX_DATA_INIT(CBCGPToolbarsPage)
	m_bTextLabels = FALSE;
	//}}AFX_DATA_INIT

	m_pSelectedToolbar = NULL;
	ASSERT_VALID (m_pParentFrame);
}

CBCGPToolbarsPage::~CBCGPToolbarsPage()
{
}

void CBCGPToolbarsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGPToolbarsPage)
	DDX_Control(pDX, IDC_BCGBARRES_TEXT_LABELS, m_wndTextLabels);
	DDX_Control(pDX, IDC_BCGBARRES_RENAME_TOOLBAR, m_bntRenameToolbar);
	DDX_Control(pDX, IDC_BCGBARRES_NEW_TOOLBAR, m_btnNewToolbar);
	DDX_Control(pDX, IDC_BCGBARRES_DELETE_TOOLBAR, m_btnDelete);
	DDX_Control(pDX, IDC_BCGBARRES_RESET, m_btnReset);
	DDX_Control(pDX, IDC_BCGBARRES_TOOLBAR_LIST, m_wndToolbarList);
	DDX_Check(pDX, IDC_BCGBARRES_TEXT_LABELS, m_bTextLabels);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGPToolbarsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CBCGPToolbarsPage)
	ON_LBN_SELCHANGE(IDC_BCGBARRES_TOOLBAR_LIST, OnSelchangeToolbarList)
	ON_LBN_DBLCLK(IDC_BCGBARRES_TOOLBAR_LIST, OnDblclkToolbarList)
	ON_BN_CLICKED(IDC_BCGBARRES_RESET, OnResetToolbar)
	ON_BN_CLICKED(IDC_BCGBARRES_RESET_ALL, OnResetAllToolbars)
	ON_BN_CLICKED(IDC_BCGBARRES_DELETE_TOOLBAR, OnDeleteToolbar)
	ON_BN_CLICKED(IDC_BCGBARRES_NEW_TOOLBAR, OnNewToolbar)
	ON_BN_CLICKED(IDC_BCGBARRES_RENAME_TOOLBAR, OnRenameToolbar)
	ON_BN_CLICKED(IDC_BCGBARRES_TEXT_LABELS, OnBcgbarresTextLabels)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CBCGPToolbarsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	if (!m_bUserDefinedToolbars)
	{
		m_btnNewToolbar.EnableWindow (FALSE);

		m_btnNewToolbar.ShowWindow (SW_HIDE);
		m_btnDelete.ShowWindow (SW_HIDE);
		m_bntRenameToolbar.ShowWindow (SW_HIDE);
	}
	
	for (POSITION pos = gAllToolbars.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolBar* pToolBar = (CBCGPToolBar*) gAllToolbars.GetNext (pos);
		ASSERT (pToolBar != NULL);

		if (CWnd::FromHandlePermanent (pToolBar->m_hWnd) != NULL)
		{
			ASSERT_VALID(pToolBar);

			//------------------------------
			// Don't add dropdown toolbars!
			//------------------------------
			if (!pToolBar->IsKindOf (RUNTIME_CLASS (CBCGPDropDownToolBar)))
			{
				//----------------------------------------------------------------------
				 // Check, if toolbar belongs to this dialog's parent main frame window
				//----------------------------------------------------------------------
				if (m_pParentFrame->GetTopLevelFrame() == 
						pToolBar->GetTopLevelFrame () &&
					pToolBar->AllowShowOnList () &&
					!pToolBar->m_bMasked)
				{
					CString strName;
					pToolBar->GetWindowText (strName);

					if (strName.IsEmpty ())
					{
						CBCGPLocalResource locaRes;
						strName.LoadString (IDS_BCGBARRES_UNTITLED_TOOLBAR);
					}

					int iIndex = m_wndToolbarList.AddString (strName);
					m_wndToolbarList.SetItemData (iIndex, (DWORD_PTR) pToolBar);

					if (pToolBar->GetStyle () & WS_VISIBLE)
					{
						m_wndToolbarList.SetCheck (iIndex, 1);
					}

					m_wndToolbarList.EnableCheck (iIndex, pToolBar->CanBeClosed ());
				}
			}
		}
	}

	CBCGPToolbarCustomize* pWndParent = DYNAMIC_DOWNCAST (CBCGPToolbarCustomize, GetParent ());
	ASSERT (pWndParent != NULL);

	if ((pWndParent->GetFlags () & BCGCUSTOMIZE_TEXT_LABELS) == 0)
	{
		m_wndTextLabels.ShowWindow (SW_HIDE);
	}

	if (m_wndToolbarList.GetCount () > 0)
	{
		m_wndToolbarList.SetCurSel (0);
		OnSelchangeToolbarList();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//*************************************************************************************
void CBCGPToolbarsPage::OnSelchangeToolbarList() 
{
	int iIndex = m_wndToolbarList.GetCurSel ();
	if (iIndex == LB_ERR)
	{
		m_pSelectedToolbar = NULL;
		m_btnReset.EnableWindow (FALSE);
		m_btnDelete.EnableWindow (FALSE);
		m_bntRenameToolbar.EnableWindow (FALSE);
		m_wndTextLabels.EnableWindow (FALSE);
		return;
	}

	m_pSelectedToolbar = (CBCGPToolBar*) m_wndToolbarList.GetItemData (iIndex);
	ASSERT_VALID(m_pSelectedToolbar);

	m_btnReset.EnableWindow (m_pSelectedToolbar->CanBeRestored ());
	m_btnDelete.EnableWindow (m_pSelectedToolbar->IsUserDefined ());
	m_bntRenameToolbar.EnableWindow (m_pSelectedToolbar->IsUserDefined ());
	m_wndTextLabels.EnableWindow (m_pSelectedToolbar->AllowChangeTextLabels ());

	m_bTextLabels = m_pSelectedToolbar->AreTextLabels ();
	UpdateData (FALSE);
}
//*************************************************************************************
void CBCGPToolbarsPage::OnDblclkToolbarList() 
{
	int iIndex = m_wndToolbarList.GetCurSel ();
	if (iIndex != LB_ERR)
	{
		m_pSelectedToolbar = (CBCGPToolBar*) m_wndToolbarList.GetItemData (iIndex);
		ASSERT_VALID(m_pSelectedToolbar);

		if (m_pSelectedToolbar->CanBeClosed ())
		{
			m_wndToolbarList.SetCheck (iIndex, !m_wndToolbarList.GetCheck (iIndex));
		}
		else
		{
			MessageBeep ((UINT) -1);
		}
	}

	OnSelchangeToolbarList ();
}
//*************************************************************************************
void CBCGPToolbarsPage::ShowToolBar (CBCGPToolBar* pToolBar, BOOL bShow)
{
	if (m_wndToolbarList.GetSafeHwnd () == NULL)
	{
		return;
	}

	for (int i = 0; i < m_wndToolbarList.GetCount (); i ++)
	{
		CBCGPToolBar* pListToolBar = (CBCGPToolBar*) m_wndToolbarList.GetItemData (i);
		ASSERT_VALID(pListToolBar);

		if (pListToolBar == pToolBar)
		{
			m_wndToolbarList.SetCheck (i, bShow);
			break;
		}
	}
}
//**************************************************************************************
void CBCGPToolbarsPage::OnResetToolbar() 
{
	ASSERT (m_pSelectedToolbar != NULL);
	ASSERT (m_pSelectedToolbar->CanBeRestored ());

	{
		CBCGPLocalResource locaRes;

		CString strName;
		m_pSelectedToolbar->GetWindowText (strName);

		CString strPrompt;
		strPrompt.Format (IDS_BCGBARRES_RESET_TOOLBAR_FMT, strName);

		if (MessageBox (strPrompt, NULL, MB_YESNO | MB_ICONQUESTION) != IDYES)
		{
			return;
		}
	}

	m_pSelectedToolbar->RestoreOriginalstate ();
}
//**************************************************************************************
void CBCGPToolbarsPage::OnResetAllToolbars() 
{
	{
		CBCGPLocalResource locaRes;

		CString strPrompt;
		strPrompt.LoadString (IDS_BCGBARRES_RESET_ALL_TOOLBARS);

		if (MessageBox (strPrompt, NULL, MB_YESNO | MB_ICONQUESTION) != IDYES)
		{
			return;
		}
	}

	BCGPCMD_MGR.ClearAllCmdImages ();

	//------------------------------------------
	// Fill image hash by the default image ids:
	//------------------------------------------
	for (POSITION pos = CBCGPToolBar::m_DefaultImages.GetStartPosition (); pos != NULL;)
	{
		UINT uiCmdId;
		int iImage;

		CBCGPToolBar::m_DefaultImages.GetNextAssoc (pos, uiCmdId, iImage);
		BCGPCMD_MGR.SetCmdImage (uiCmdId, iImage, FALSE);
	}

	for (int i = 0; i < m_wndToolbarList.GetCount (); i ++)
	{
		CBCGPToolBar* pListToolBar = (CBCGPToolBar*) m_wndToolbarList.GetItemData (i);
		ASSERT_VALID(pListToolBar);

		if (pListToolBar->CanBeRestored ())
		{
			pListToolBar->RestoreOriginalstate ();
		}
	}
}
//**********************************************************************************
void CBCGPToolbarsPage::OnDeleteToolbar() 
{
	ASSERT (m_pSelectedToolbar != NULL);
	ASSERT (m_pSelectedToolbar->IsUserDefined ());

	CFrameWnd* pParentFrame = GetParentFrame ();
	if (pParentFrame == NULL)
	{
		MessageBeep (MB_ICONASTERISK);
		return;
	}

	{
		CBCGPLocalResource locaRes;

		CString strName;
		m_pSelectedToolbar->GetWindowText (strName);

		CString strPrompt;
		strPrompt.Format (IDS_BCGBARRES_DELETE_TOOLBAR_FMT, strName);

		if (MessageBox (strPrompt, NULL, MB_YESNO | MB_ICONQUESTION) != IDYES)
		{
			return;
		}
	}

	if (pParentFrame->SendMessage (BCGM_DELETETOOLBAR, 0, (LPARAM) m_pSelectedToolbar)
		== 0)
	{
		MessageBeep (MB_ICONASTERISK);
		return;
	}

	m_wndToolbarList.DeleteString (m_wndToolbarList.GetCurSel ());
	m_wndToolbarList.SetCurSel (0);
	OnSelchangeToolbarList ();
}
//**********************************************************************************
void CBCGPToolbarsPage::OnNewToolbar()
{
	CString strToolbarName;
	{
		CBCGPLocalResource locaRes;

		CToolbarNameDlg dlg (this);
		if (dlg.DoModal () != IDOK)
		{
			return;
		}

		strToolbarName = dlg.m_strToolbarName;
	}

	CFrameWnd* pParentFrame = GetParentFrame ();
	if (pParentFrame == NULL)
	{
		MessageBeep (MB_ICONASTERISK);
		return;
	}

	CBCGPToolBar* pNewToolbar = 
		(CBCGPToolBar*)pParentFrame->SendMessage (BCGM_CREATETOOLBAR, 0,
			(LPARAM) (LPCTSTR) strToolbarName);
	if (pNewToolbar == NULL)
	{
		return;
	}

	ASSERT_VALID (pNewToolbar);

	int iIndex = m_wndToolbarList.AddString (strToolbarName);
	m_wndToolbarList.SetItemData (iIndex, (DWORD_PTR) pNewToolbar);

	m_wndToolbarList.SetCheck (iIndex, 1);
	m_wndToolbarList.SetCurSel (iIndex);
	m_wndToolbarList.SetTopIndex (iIndex);

	OnSelchangeToolbarList ();
}
//**********************************************************************************
void CBCGPToolbarsPage::OnRenameToolbar() 
{
	ASSERT (m_pSelectedToolbar != NULL);
	ASSERT (m_pSelectedToolbar->IsUserDefined ());

	CString strToolbarName;
	{
		CBCGPLocalResource locaRes;

		CToolbarNameDlg dlg (this);
		m_pSelectedToolbar->GetWindowText (dlg.m_strToolbarName);

		if (dlg.DoModal () != IDOK)
		{
			return;
		}

		strToolbarName = dlg.m_strToolbarName;
	}

	m_pSelectedToolbar->SetWindowText (strToolbarName);
	if (m_pSelectedToolbar->IsFloating ())
	{
		//-----------------------------
		// Change floating frame title:
		//-----------------------------
		CBCGPMiniFrameWnd* pParentMiniFrame = m_pSelectedToolbar->GetParentMiniFrame ();
		if (pParentMiniFrame != NULL)
		{
			pParentMiniFrame->SetWindowText (strToolbarName);
			pParentMiniFrame->RedrawWindow (NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
		}
	}

	m_wndToolbarList.DeleteString (m_wndToolbarList.GetCurSel ());

	int iIndex = m_wndToolbarList.AddString (strToolbarName);
	m_wndToolbarList.SetItemData (iIndex, (DWORD_PTR) m_pSelectedToolbar);

	if (m_pSelectedToolbar->GetStyle () & WS_VISIBLE)
	{
		m_wndToolbarList.SetCheck (iIndex, 1);
	}

	m_wndToolbarList.SetCurSel (iIndex);
	m_wndToolbarList.SetTopIndex (iIndex);

	OnSelchangeToolbarList ();
}
//*************************************************************************************
BOOL CBCGPToolbarsPage::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	UINT uiCode = HIWORD (wParam);
	UINT uiID = LOWORD (wParam);

	if (uiCode == CLBN_CHKCHANGE && uiID == IDC_BCGBARRES_TOOLBAR_LIST)
	{
		int iIndex = m_wndToolbarList.GetCurSel ();
		if (iIndex != LB_ERR)
		{
			CBCGPToolBar* pToolbar = (CBCGPToolBar*) m_wndToolbarList.GetItemData (iIndex);
			ASSERT_VALID (pToolbar);

			if (pToolbar->CanBeClosed ())
			{
				//-------------------
				// Show/hide toolbar:
				//-------------------
				pToolbar->ShowControlBar (m_wndToolbarList.GetCheck (iIndex), FALSE, TRUE);
			}
			else if (m_wndToolbarList.GetCheck (iIndex) == 0)
			{
				//----------------------------------
				// Toolbar should be always visible!
				//----------------------------------
				if (!pToolbar->IsPopupMode())
				{
					m_wndToolbarList.SetCheck (iIndex, TRUE);
				}

				MessageBeep ((UINT) -1);
			}
		}
	}
	
	return CPropertyPage::OnCommand(wParam, lParam);
}
//***********************************************************************************
void CBCGPToolbarsPage::OnBcgbarresTextLabels() 
{
	UpdateData ();

	ASSERT_VALID (m_pSelectedToolbar);
	m_pSelectedToolbar->EnableTextLabels (m_bTextLabels);
}
