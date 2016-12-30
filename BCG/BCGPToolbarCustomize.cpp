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

// CBCGPToolbarCustomize.cpp : implementation file
//

#include "stdafx.h"

#include "afxpriv.h"
#include "bcgprores.h"
#include "BCGPToolbarCustomize.h"
#include "BCGPToolBar.h"
#include "BCGPMenuBar.h"
#include "BCGPLocalResource.h"
#include "BCGPMouseManager.h"
#include "BCGPContextMenuManager.h"
#include "BCGPTearOffManager.h"
#include "ButtonsTextList.h"
#include "BCGPMDIFrameWnd.h"
#include "BCGPFrameWnd.h"
#include "BCGPKeyboardManager.h"
#include "BCGPUserToolsManager.h"
#include "BCGPToolbarMenuButton.h"
#include "BCGPOutlookBar.h"
#include "BCGPUserTool.h"
#include "BCGPWorkspace.h"
#include "BCGPHelpIds.h"
#include "BCGPVisualManager.h"
#include "BCGPSkinManager.h"
#include "BCGPDockBar.h"
#include "BCGPReBar.h"
#include "BCGPImageEditDlg.h"
#include "BCGPOleIPFrameWnd.h"
#include "BCGPOleDocIPFrameWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CBCGPToolbarCustomize*	g_pWndCustomize = NULL;
extern CBCGPWorkspace*	g_pWorkspace;

static const int iButtonMargin = 8;

/////////////////////////////////////////////////////////////////////////////
// CBCGPToolbarCustomize

IMPLEMENT_DYNAMIC(CBCGPToolbarCustomize, CPropertySheet)

CBCGPToolbarCustomize::CBCGPToolbarCustomize(
	CFrameWnd* pWndParentFrame,
	BOOL bAutoSetFromMenus /* = FALSE */,
	UINT uiFlags /* = 0xFFFF */,
	CList<CRuntimeClass*,CRuntimeClass*>*	plistCustomPages /* = NULL */)
	 : CPropertySheet(_T(""), pWndParentFrame),
	 m_bAutoSetFromMenus (bAutoSetFromMenus),
	 m_uiFlags (uiFlags)
{

	m_uiControlbarsMenuEntryID = 0;

	if((m_uiFlags & BCGCUSTOMIZE_MENUAMPERS) == 0)
	{
		m_bSaveMenuAmps = FALSE;
		
	}else
	{
		m_bSaveMenuAmps = TRUE;
	}

	//----------------------------
	// Add optional custom pages:
	//----------------------------
	if (plistCustomPages != NULL) 
	{
		// does not use lib local resources, so moved to front
		ASSERT_VALID(plistCustomPages);
		for(POSITION pos=plistCustomPages->GetHeadPosition(); pos; )
		{
			CRuntimeClass* pClass = plistCustomPages->GetNext(pos);
			m_listCustomPages.AddTail( (CPropertyPage*) pClass->CreateObject() );
		}
	}

	if (!CBCGPVisualManager::GetInstance ()->IsLook2000Allowed ())
	{
		m_uiFlags &= ~BCGCUSTOMIZE_LOOK_2000;
	}

#if defined _AFXDLL && !defined _BCGCBPRO_STATIC_	// Skins manager can not be used in the static version
	if (g_pSkinManager == NULL)
	{
		ASSERT ((m_uiFlags & BCGCUSTOMIZE_SELECT_SKINS) == 0);
		m_uiFlags &= ~BCGCUSTOMIZE_SELECT_SKINS;
	}
#else
	m_uiFlags &= ~BCGCUSTOMIZE_SELECT_SKINS;
#endif

	CBCGPLocalResource locaRes;

	ASSERT (pWndParentFrame != NULL);
	m_pParentFrame = pWndParentFrame;

	m_pCustomizePage = new CBCGPCustomizePage;
	m_pToolbarsPage = new CBCGPToolbarsPage (m_pParentFrame);
	m_pKeyboardPage = new CBCGPKeyboardPage (m_pParentFrame, m_bAutoSetFromMenus);
	m_pMenuPage = new CBCGPMenuPage (m_pParentFrame, m_bAutoSetFromMenus);
	m_pMousePage = new CBCGPMousePage;

	//---------------------------------------
	// Add two main pages (available always):
	//---------------------------------------
	AddPage (m_pCustomizePage);
	AddPage (m_pToolbarsPage);

	//----------------
	// Add tools page:
	//----------------
	if (m_uiFlags & BCGCUSTOMIZE_NOTOOLS)
	{
		m_pToolsPage = NULL;
	}
	else
	{
		m_pToolsPage = new CBCGPToolsPage ();
		if (g_pUserToolsManager != NULL)
		{
			AddPage (m_pToolsPage);
		}
	}

	//---------------------------------------------------------
	// Add keyboard customization page (available only if
	// the main application windows accelerator table is exist):
	//---------------------------------------------------------
	if (g_pKeyboardManager != NULL && pWndParentFrame->m_hAccelTable != NULL)
	{
		AddPage (m_pKeyboardPage);
	}

	//---------------------------------------------------------------
	// Add menu customization page (available only if the menu bar or
	// context menu manager are available):
	//---------------------------------------------------------------
	BOOL bMenuBarIsAvailable = FALSE;

	CBCGPMDIFrameWnd* pMainMDIFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, m_pParentFrame);
	if (pMainMDIFrame != NULL)
	{
		bMenuBarIsAvailable = (pMainMDIFrame->IsMenuBarAvailable ());
		m_uiControlbarsMenuEntryID = pMainMDIFrame->GetControlbarsMenuEntryID();
	}
	else
	{
		CBCGPFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, m_pParentFrame);
		if (pMainFrame != NULL)
		{
			bMenuBarIsAvailable = (pMainFrame->IsMenuBarAvailable ());
			m_uiControlbarsMenuEntryID = pMainFrame->GetControlbarsMenuEntryID();
		}
	}

	if (g_pContextMenuManager != NULL || bMenuBarIsAvailable)
	{	
		AddPage (m_pMenuPage);
	}

	if (g_pMouseManager != NULL)
	{
		AddPage (m_pMousePage);
	}

	//---------------------------
	// Add optional custom pages:
	//---------------------------
	for(POSITION pos=m_listCustomPages.GetHeadPosition(); pos; )
	{
		AddPage( m_listCustomPages.GetNext(pos) );
	}

	m_pOptionsPage = new CBCGPOptionsPage (bMenuBarIsAvailable);
	AddPage (m_pOptionsPage);

	//----------------------------
	// Set property sheet caption:
	//----------------------------
	CString strCaption;
	strCaption.LoadString (IDS_BCGBARRES_PROPSHT_CAPTION);

	m_strAllCommands.LoadString (IDS_BCGBARRES_ALL_COMMANDS);
	m_pCustomizePage->SetAllCategory (m_strAllCommands);

	if (m_pKeyboardPage != NULL)
	{
		m_pKeyboardPage->SetAllCategory (m_strAllCommands);
	}

	SetTitle (strCaption);

	if (m_bAutoSetFromMenus)
	{
		SetupFromMenus ();
	}

	//-------------------------
	// Add a "New menu" button:
	//-------------------------
	CString strNewMenu;
	strNewMenu.LoadString (IDS_BCGBARRES_NEW_MENU);

	AddButton (strNewMenu, CBCGPToolbarMenuButton (0, NULL, -1, strNewMenu));
}
//**************************************************************************************
CBCGPToolbarCustomize::~CBCGPToolbarCustomize()
{
	POSITION pos = m_ButtonsByCategory.GetStartPosition();
	while (pos != NULL)
	{
		CObList* pCategoryButtonsList;
		CString string;
		
		m_ButtonsByCategory.GetNextAssoc (pos, string, pCategoryButtonsList);
		ASSERT_VALID(pCategoryButtonsList);

		while (!pCategoryButtonsList->IsEmpty ())
		{
			delete pCategoryButtonsList->RemoveHead ();
		}

		delete pCategoryButtonsList;
	}

	m_ButtonsByCategory.RemoveAll();

	delete m_pCustomizePage;
	delete m_pToolbarsPage;
	delete m_pKeyboardPage;
	delete m_pMenuPage;
	delete m_pMousePage;
	delete m_pOptionsPage;

	if (m_pToolsPage != NULL)
	{
		delete m_pToolsPage;
	}

	//----------------------------------
	// delete all optional custom pages:
	//----------------------------------
	while(!m_listCustomPages.IsEmpty())
	{
		delete m_listCustomPages.RemoveHead();
	}
}

BEGIN_MESSAGE_MAP(CBCGPToolbarCustomize, CPropertySheet)
	//{{AFX_MSG_MAP(CBCGPToolbarCustomize)
	ON_WM_CREATE()
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBCGPToolbarCustomize message handlers

int CBCGPToolbarCustomize::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}
	
	if (m_uiFlags & BCGCUSTOMIZE_CONTEXT_HELP)
	{
		ModifyStyleEx (0, WS_EX_CONTEXTHELP);
	}

	g_pWndCustomize = this;
	return 0;
}
//**************************************************************************************
void CBCGPToolbarCustomize::PostNcDestroy()
{
	g_pWndCustomize = NULL;
	SetFrameCustMode (FALSE);

	CPropertySheet::PostNcDestroy();
	delete this;
}
//**************************************************************************************
void CBCGPToolbarCustomize::AddButton (UINT uiCategoryId, const CBCGPToolbarButton& button,
				int iInsertAfter)
{
	CString strCategory;
	strCategory.LoadString (uiCategoryId);

	AddButton (strCategory, button, iInsertAfter);
}
//**************************************************************************************
void CBCGPToolbarCustomize::AddButton (LPCTSTR lpszCategory, const CBCGPToolbarButton& button,
										int iInsertAfter)
{
	int iId = (int) button.m_nID;

	if (m_uiControlbarsMenuEntryID != 0 && iId == (int)m_uiControlbarsMenuEntryID)
	{
		return;
	}

	if (!button.IsEditable ())
	{
		//------------------------------------------------------
		// Don't add protected, MRU, system and Window commands:
		//------------------------------------------------------
		return;
	}

	if (!CBCGPToolBar::IsCommandPermitted (button.m_nID))
	{
		return;
	}

	CString strText = button.m_strText;
	strText.TrimLeft ();
	strText.TrimRight ();

	BOOL bToolBtn = FALSE;

	if (g_pUserToolsManager != NULL &&
		g_pUserToolsManager->IsUserToolCmd (iId))
	{
		CBCGPUserTool* pTool = g_pUserToolsManager->FindTool (iId);

		if (pTool == NULL)
		{
			//------------------------------
			// Undefined user tool, skip it
			//------------------------------
			return;
		}

		ASSERT_VALID (pTool);

		//------------------------
		// Use tool name as label:
		//------------------------
		strText = pTool->m_strLabel;
		bToolBtn = TRUE;
	}

	if (strText.IsEmpty ())
	{
		//-------------------------------------------
		// Try to find the command name in resources:
		//-------------------------------------------
		CString strMessage;
		int iOffset;
		if (strMessage.LoadString (button.m_nID) &&
			(iOffset = strMessage.Find (_T('\n'))) != -1)
		{
			strText = strMessage.Mid (iOffset + 1);
		}

		if (strText.IsEmpty () && lpszCategory == m_strAllCommands)
		{
			return;
		}
	}
	else
	{
		if(!m_bSaveMenuAmps)
		{
			strText.Remove (_T('&'));
		}

		//----------------------------------------
		// Remove trailing label (ex.:"\tCtrl+S"):
		//----------------------------------------
		int iOffset = strText.Find (_T('\t'));
		if (iOffset != -1)
		{
			strText = strText.Left (iOffset);
		}
	}

	//---------------------------------------------------------------
	// If text is still empty, assume dummy command and don't add it:
	//---------------------------------------------------------------
	if (strText.IsEmpty ())
	{
		return;
	}
	
	//--------------------------------------------------
	// Find a category entry or create new if not exist:
	//--------------------------------------------------
	CObList* pCategoryButtonsList;
	if (!m_ButtonsByCategory.Lookup (lpszCategory, pCategoryButtonsList))
	{
		//--------------------------------
		// Category not found! Create new:
		//--------------------------------
		pCategoryButtonsList = new CObList;
		m_ButtonsByCategory.SetAt (lpszCategory, pCategoryButtonsList);

		if (lpszCategory != m_strAllCommands)
		{
			m_strCategoriesList.AddTail (lpszCategory);
		}
	}
	else
	{
		//--------------------------------------------------------
		// Category is not a new. Maybe the button is exist also?
		//--------------------------------------------------------
		ASSERT (pCategoryButtonsList != NULL);

		for (POSITION pos = pCategoryButtonsList->GetHeadPosition (); pos != NULL;)
		{
			CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pCategoryButtonsList->GetNext (pos);
			ASSERT (pButton != NULL);

			if ((pButton->m_nID == button.m_nID && pButton->m_nID != (UINT) -1) 
				|| (pButton->m_nID == (UINT) -1 && pButton->m_strText == button.m_strText))
				// The same exist... 
			{
				if (pButton->m_strText.IsEmpty ())
				{
					pButton->m_strText = button.m_strText;
				}

				return;
			}
		}
	}

	//-------------------------------------------------------------------
	// Create a new CBCGPToolbarButton object (MFC class factory is used):
	//-------------------------------------------------------------------
	CRuntimeClass* pClass = button.GetRuntimeClass ();
	ASSERT (pClass != NULL);

	CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pClass->CreateObject ();
	ASSERT_VALID (pButton);

	pButton->CopyFrom (button);
	pButton->m_strText = strText;

	if (bToolBtn)
	{
		pButton->SetImage (0);
	}

	//-------------------------------------------
	// Add a new button to the specific category:
	//-------------------------------------------
	BOOL bInserted = FALSE;
	if (iInsertAfter != -1)
	{
		POSITION pos = pCategoryButtonsList->FindIndex (iInsertAfter);
		if (pos != NULL)
		{
			pCategoryButtonsList->InsertBefore (pos, pButton);
			bInserted = TRUE;
		}
	}

	if (!bInserted)
	{
		pCategoryButtonsList->AddTail (pButton);
	}

	if (lpszCategory != m_strAllCommands)
	{
		AddButton (m_strAllCommands, button);
	}

	pButton->OnAddToCustomizePage ();
}
//**************************************************************************************
int CBCGPToolbarCustomize::RemoveButton (UINT uiCategoryId, UINT uiCmdId)
{
	if (uiCategoryId == (UINT) -1)	// Remove from ALL caregories
	{
		BOOL bFinish = FALSE;
		for (POSITION posCategory = m_strCategoriesList.GetHeadPosition();
			!bFinish;)
		{
			CString strCategory;
			if (posCategory == NULL)
			{
				strCategory = m_strAllCommands;
				bFinish = TRUE;
			}
			else
			{
				strCategory = m_strCategoriesList.GetNext (posCategory);
			}

			RemoveButton (strCategory, uiCmdId);
		}

		return 0;
	}

	CString strCategory;
	strCategory.LoadString (uiCategoryId);

	return RemoveButton (strCategory, uiCmdId);
}
//**************************************************************************************
int CBCGPToolbarCustomize::RemoveButton (LPCTSTR lpszCategory, UINT uiCmdId)
{
	ASSERT (lpszCategory != NULL);

	CObList* pCategoryButtonsList;
	if (!m_ButtonsByCategory.Lookup (lpszCategory, pCategoryButtonsList))
	{
		// Category not found!
		return -1;
	}

	int i = 0;
	for (POSITION pos = pCategoryButtonsList->GetHeadPosition (); pos != NULL; i ++)
	{
		POSITION posSave = pos;

		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pCategoryButtonsList->GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->m_nID == uiCmdId)
		{
			pCategoryButtonsList->RemoveAt (posSave);
			delete pButton;
			return i;
		}
	}

	return -1;
}
//**************************************************************************************
BOOL CBCGPToolbarCustomize::AddToolBar (UINT uiCategory, UINT uiToolbarResId)
{
	CString strCategory;
	strCategory.LoadString (uiCategory);

	return AddToolBar (strCategory, uiToolbarResId);
}
//**************************************************************************************
BOOL CBCGPToolbarCustomize::AddToolBar (LPCTSTR lpszCategory, UINT uiToolbarResId)
{
	struct CToolBarData
	{
		WORD wVersion;
		WORD wWidth;
		WORD wHeight;
		WORD wItemCount;

		WORD* items()
			{ return (WORD*)(this+1); }
	};

	LPCTSTR lpszResourceName = MAKEINTRESOURCE (uiToolbarResId);
	ASSERT(lpszResourceName != NULL);

	//---------------------------------------------------
	// determine location of the bitmap in resource fork:
	//---------------------------------------------------
	HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_TOOLBAR);
	HRSRC hRsrc = ::FindResource(hInst, lpszResourceName, RT_TOOLBAR);
	if (hRsrc == NULL)
	{
		TRACE(_T("CBCGPToolbarCustomize::AddToolBar: Can't load toolbar %x\n"), uiToolbarResId);
		return FALSE;
	}

	HGLOBAL hGlobal = ::LoadResource(hInst, hRsrc);
	if (hGlobal == NULL)
	{
		TRACE(_T("CBCGPToolbarCustomize::AddToolBar: Can't load toolbar %x\n"), uiToolbarResId);
		return FALSE;
	}

	CToolBarData* pData = (CToolBarData*)LockResource(hGlobal);
	if (pData == NULL)
	{
		TRACE(_T("CBCGPToolbarCustomize::AddToolBar: Can't load toolbar %x\n"), uiToolbarResId);
		::FreeResource (hGlobal);
		return FALSE;
	}

	ASSERT (pData->wVersion == 1);

	for (int i = 0; i < pData->wItemCount; i++)
	{
		UINT uiCmd = pData->items() [i];
		if (uiCmd > 0 && uiCmd != (UINT) -1)
		{
			AddButton (lpszCategory, CBCGPToolbarButton (uiCmd, -1));
		}
	}

	::UnlockResource (hGlobal);
	::FreeResource (hGlobal);

	return TRUE;
}
//**************************************************************************************
BOOL CBCGPToolbarCustomize::AddMenu (UINT uiMenuResId)
{
	CMenu menu;
	if (!menu.LoadMenu (uiMenuResId))
	{
		TRACE(_T("CBCGPToolbarCustomize::AddMenu: Can't load menu %x\n"), uiMenuResId);
		return FALSE;
	}

	AddMenuCommands (&menu, FALSE);
	return TRUE;
}
//**************************************************************************************
// Rename automatically imported categories (e.g. "?"->"Help")
BOOL CBCGPToolbarCustomize::RenameCategory(LPCTSTR lpszCategoryOld, LPCTSTR lpszCategoryNew)
{
	// New Name must not be present
	POSITION pos = m_strCategoriesList.Find(lpszCategoryNew);
	if(pos)
		return FALSE;

	// ...but the old one must be
	pos = m_strCategoriesList.Find(lpszCategoryOld);
	if(!pos)
		return FALSE;

	// Change Name in Button-map too:
	CObList* pCategoryButtonsList;

	// new Category must not be present yet
	if (m_ButtonsByCategory.Lookup (lpszCategoryNew, pCategoryButtonsList))
		return FALSE;

	// ...but the old one must be
	if (!m_ButtonsByCategory.Lookup (lpszCategoryOld, pCategoryButtonsList))
		return FALSE;

	// change both or nothing
	m_strCategoriesList.SetAt(pos, lpszCategoryNew);
	m_ButtonsByCategory.RemoveKey(lpszCategoryOld);
	m_ButtonsByCategory.SetAt(lpszCategoryNew, pCategoryButtonsList);

	return TRUE;
}
//**************************************************************************************
void CBCGPToolbarCustomize::ReplaceButton (UINT uiCmd, const CBCGPToolbarButton& button)
{
	CRuntimeClass* pClass = button.GetRuntimeClass ();
	ASSERT (pClass != NULL);

	BOOL bFinish = FALSE;
	for (POSITION posCategory = m_strCategoriesList.GetHeadPosition();
		!bFinish;)
	{
		CString strCategory;
		if (posCategory == NULL)
		{
			strCategory = m_strAllCommands;
			bFinish = TRUE;
		}
		else
		{
			strCategory = m_strCategoriesList.GetNext (posCategory);
		}

		CObList* pCategoryButtonsList;
		if (!m_ButtonsByCategory.Lookup (strCategory, pCategoryButtonsList))
		{
			ASSERT (FALSE);
		}

		ASSERT_VALID (pCategoryButtonsList);

		for (POSITION pos = pCategoryButtonsList->GetHeadPosition (); pos != NULL;)
		{
			POSITION posSave = pos;
			CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pCategoryButtonsList->GetNext (pos);
			ASSERT (pButton != NULL);

			if (pButton->m_nID == uiCmd)	// Found!
			{
				CBCGPToolbarButton* pNewButton = (CBCGPToolbarButton*) pClass->CreateObject ();
				ASSERT_VALID (pNewButton);

				pNewButton->CopyFrom (button);
				if (pNewButton->m_strText.IsEmpty ())
				{
					pNewButton->m_strText = pButton->m_strText;
				}

				pCategoryButtonsList->SetAt (posSave, pNewButton);
				delete pButton;
			}
		}
	}
}
//**************************************************************************************
BOOL CBCGPToolbarCustomize::SetUserCategory (LPCTSTR lpszCategory)
{
	ASSERT (lpszCategory != NULL);

	CObList* pCategoryButtonsList;
	if (!m_ButtonsByCategory.Lookup (lpszCategory, pCategoryButtonsList))
	{
		TRACE(_T("CBCGPToolbarCustomize::SetUserCategory: Can't find category '%s'\n"), 
			lpszCategory);
		return FALSE;
	}

	m_pCustomizePage->SetUserCategory (lpszCategory);
	return TRUE;
}
//**************************************************************************************
void CBCGPToolbarCustomize::SetFrameCustMode (BOOL bCustMode)
{
	ASSERT_VALID (m_pParentFrame);

	CWaitCursor	wait;

	//-------------------------------------------------------------------
	// Enable/disable all parent frame child windows (except docking bars
	// and our toolbars):
	//-------------------------------------------------------------------
	CWnd* pWndChild = m_pParentFrame->GetWindow (GW_CHILD);
	while (pWndChild != NULL)
	{
		CRuntimeClass* pChildClass = pWndChild->GetRuntimeClass ();
		if (pChildClass == NULL ||
			(!pChildClass->IsDerivedFrom (RUNTIME_CLASS (CDockBar)) &&
			 !pChildClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPDockBar)) &&
			 !pChildClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPOutlookBar)) &&
			 !pChildClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPReBar)) &&
			 !pChildClass->IsDerivedFrom (RUNTIME_CLASS (CBCGPToolBar))))
		{
			pWndChild->EnableWindow (!bCustMode);
		}

		pWndChild = pWndChild->GetNextWindow ();
	}

	//--------------------------------
	// Enable/Disable floating frames:
	//--------------------------------
	CBCGPDockManager* pDockManager = NULL;

	CBCGPMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, m_pParentFrame);
	if (pMainFrame != NULL)
	{
		pDockManager = pMainFrame->GetDockManager ();
	}
	else	// Maybe, SDI frame...
	{
		CBCGPFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, m_pParentFrame);
		if (pFrame != NULL)
		{
			pDockManager = pFrame->GetDockManager ();

		}
		else	// Maybe, OLE frame
		{
			CBCGPOleIPFrameWnd* pOleFrame = 
					DYNAMIC_DOWNCAST (CBCGPOleIPFrameWnd, m_pParentFrame);
			if (pOleFrame != NULL)
			{
				pDockManager = pOleFrame->GetDockManager ();
			}
			else
			{
				CBCGPOleDocIPFrameWnd* pOleDocFrame = 
					DYNAMIC_DOWNCAST (CBCGPOleDocIPFrameWnd, m_pParentFrame);
				if (pOleDocFrame != NULL)
				{
					pDockManager = pOleDocFrame->GetDockManager ();
				}
			}
		}
	}

	if (pDockManager != NULL)
	{
		ASSERT_VALID (pDockManager);

		const CObList& lstMiniFrames = pDockManager->GetMiniFrames ();
		for (POSITION pos = lstMiniFrames.GetHeadPosition (); pos != NULL;)
		{
			CBCGPMiniFrameWnd* pMiniFrame = 
				DYNAMIC_DOWNCAST (CBCGPMiniFrameWnd, lstMiniFrames.GetNext (pos));
			if (pMiniFrame != NULL &&
				DYNAMIC_DOWNCAST (CBCGPBaseToolBar, pMiniFrame->GetControlBar ()) == NULL)
			{
				pMiniFrame->EnableWindow (!bCustMode);
			}
		}
	}

	BOOL bLocked = m_pParentFrame->LockWindowUpdate();


	//-----------------------------------------------
	// Set/reset costumize mode for ALL our toolbars:
	//-----------------------------------------------
	CBCGPToolBar::SetCustomizeMode (bCustMode);

	//-------------------------------------------------------------
	// Inform the parent frame about mode (for additional actions):
	//-------------------------------------------------------------
	m_pParentFrame->SendMessage (BCGM_CUSTOMIZETOOLBAR, (WPARAM) bCustMode);

	if (bLocked)
	{
		m_pParentFrame->UnlockWindowUpdate();
	}

	if (!bCustMode && m_pParentFrame->GetActiveFrame () != NULL)
	{
		//---------------------
		// Restore active view:
		//---------------------
		m_pParentFrame->GetActiveFrame ()->PostMessage (WM_SETFOCUS);
	}
}
//**************************************************************************************
BOOL CBCGPToolbarCustomize::Create () 
{
	{
		CBCGPLocalResource locaRes;

		DWORD dwExStyle = 0;

		if ((m_pParentFrame != NULL) && (m_pParentFrame->GetExStyle() & WS_EX_LAYOUTRTL))
		{
			dwExStyle |= WS_EX_LAYOUTRTL;
		}

		if (!CPropertySheet::Create (m_pParentFrame, (DWORD)-1, dwExStyle))
		{
			return FALSE;
		}
	}

	SetFrameCustMode (TRUE);
	return TRUE;
}
//*************************************************************************************
void CBCGPToolbarCustomize::ShowToolBar (CBCGPToolBar* pToolBar, BOOL bShow)
{
	m_pToolbarsPage->ShowToolBar (pToolBar, bShow);
}
//*************************************************************************************
BOOL CBCGPToolbarCustomize::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
	CRect rectClient;	// Client area rectangle
	GetClientRect (&rectClient);

	//----------------------
	// Show "Cancel" button:
	//----------------------
	CWnd *pWndCancel = GetDlgItem (IDCANCEL);
	if (pWndCancel == NULL)
	{
		return bResult;
	}

	pWndCancel->ShowWindow (SW_SHOW);
	pWndCancel->EnableWindow ();

	CRect rectClientCancel;
	pWndCancel->GetClientRect (&rectClientCancel);
	pWndCancel->MapWindowPoints (this, &rectClientCancel);

	//-------------------------------
	// Enlarge property sheet window:
	//-------------------------------
	CRect rectWnd;
	GetWindowRect(rectWnd);	

	SetWindowPos(NULL, 0, 0,
		rectWnd.Width (),
		rectWnd.Height () + rectClientCancel.Height () + 2 * iButtonMargin,
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	//-------------------------------------------------
	// Move "Cancel" button to the right bottom corner:
	//-------------------------------------------------
	pWndCancel->SetWindowPos (NULL, 
		rectClient.right - rectClientCancel.Width () - iButtonMargin,
		rectClientCancel.top + iButtonMargin / 2,
		0, 0,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	//---------------------------------------------------
	// Change "Cancel" button's style to "DEFPUSHBUTTON":
	//---------------------------------------------------
	CWnd *pWndOk = GetDlgItem (IDOK);
	if (pWndOk != NULL)
	{
		pWndOk->ModifyStyle (BS_DEFPUSHBUTTON, 0);
	}

	pWndCancel->ModifyStyle (0, BS_DEFPUSHBUTTON);

	//--------------------------------------------------------
	// Replace "Cancel" text to "Close" 
	// (CPropertyPage::CancelToClose method does nothing in a 
	// modeless property sheet):
	//--------------------------------------------------------
	CString strCloseText;
	
	{
		CBCGPLocalResource locaRes;
		strCloseText.LoadString (IDS_BCGBARRES_CLOSE);
	}

	pWndCancel->SetWindowText (strCloseText);

	//------------------------
	// Adjust the Help button:
	//------------------------
	CButton *pWndHelp = (CButton*) GetDlgItem (IDHELP);
	if (pWndHelp == NULL)
	{
		return bResult;
	}

	if (m_uiFlags & BCGCUSTOMIZE_NOHELP)
	{
		pWndHelp->ShowWindow (SW_HIDE);
		pWndHelp->EnableWindow(FALSE);
	}
	else
	{
		m_btnHelp.SubclassWindow (pWndHelp->GetSafeHwnd ());
		m_btnHelp.ShowWindow (SW_SHOW);
		m_btnHelp.EnableWindow ();

		//-----------------------
		// Set Help button image:
		//-----------------------
		CBCGPLocalResource locaRes;
		m_btnHelp.SetImage (globalData.Is32BitIcons () ? 
			IDB_BCGBARRES_HELP32 : IDB_BCGBARRES_HELP);

		m_btnHelp.SetWindowText (_T(""));

		//-------------------------------------------------
		// Move "Help" button to the left bottom corner and
		// adjust its size by the bitmap size:
		//-------------------------------------------------
		
		CSize sizeHelp = m_btnHelp.SizeToContent (TRUE);

		m_btnHelp.SetWindowPos (NULL, 
			rectClient.left + iButtonMargin,
			rectClientCancel.top,
			sizeHelp.cx, sizeHelp.cy,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}

	return bResult;
}
//************************************************************************************
BOOL CBCGPToolbarCustomize::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (LOWORD (wParam))
	{
	case IDCANCEL:
		if (g_pUserToolsManager != NULL && m_pToolsPage != NULL)
		{
			if (!CheckToolsValidity (g_pUserToolsManager->GetUserTools ()))
			{
				// Continue customization....
				if (GetActivePage () != m_pToolsPage)
				{
					SetActivePage (m_pToolsPage);
				}

				return TRUE;
			}
		}

		DestroyWindow ();
		return TRUE;

	case IDHELP:
		ASSERT_VALID (m_pParentFrame);
		m_pParentFrame->SendMessage (BCGM_CUSTOMIZEHELP, GetActiveIndex (), (LPARAM) this);
		return TRUE;
	}
		
	return CPropertySheet::OnCommand(wParam, lParam);
}
//************************************************************************************
void CBCGPToolbarCustomize::EnableUserDefinedToolbars (BOOL bEnable)
{
	m_pToolbarsPage->EnableUserDefinedToolbars (bEnable);
}
//******************************************************************************************
void CBCGPToolbarCustomize::AddMenuCommands (const CMenu* pMenu, BOOL bPopup, 
											LPCTSTR lpszCategory, LPCTSTR lpszMenuPath)
{
	ASSERT (pMenu != NULL);

	BOOL bIsWindowsMenu = FALSE;
	int iCount = (int) pMenu->GetMenuItemCount ();

	for (int i = 0; i < iCount; i ++)
	{
		UINT uiCmd = pMenu->GetMenuItemID (i);

		CString strText;
		pMenu->GetMenuString (i, strText, MF_BYPOSITION);

		if(!m_bSaveMenuAmps)
		{
			strText.Remove (_T('&'));
		}

		switch (uiCmd)
		{
		case 0:		// Separator, ignore it.
			break;

		case -1:	// Submenu
			{
				CMenu* pSubMenu = pMenu->GetSubMenu (i);

				UINT uiTearOffId = 0;
				if (g_pBCGPTearOffMenuManager != NULL)
				{
					uiTearOffId = g_pBCGPTearOffMenuManager->Parse (strText);
				}

				
				CString strCategory = strText;
				strCategory.Remove (_T('&'));

				if (lpszCategory != NULL)
				{
					strCategory = lpszCategory;
				}

				if (m_bAutoSetFromMenus)
				{
					if (bPopup)
					{
						CBCGPToolbarMenuButton menuButton ((UINT) -1, 
							pSubMenu->GetSafeHmenu (),
							-1,
							strText);

						menuButton.SetTearOff (uiTearOffId);
						AddButton (strCategory, menuButton);
					}

					CString strPath;
					if (lpszMenuPath != NULL)
					{
						strPath = lpszMenuPath;
					}

					strPath += strText;
					AddMenuCommands (pSubMenu, bPopup, strCategory, strPath);
				}
				else
				{
					AddMenuCommands (pSubMenu, bPopup);
				}

			}
			break;

		default:
			if (bPopup && uiCmd >= AFX_IDM_WINDOW_FIRST && uiCmd <= AFX_IDM_WINDOW_LAST)
			{
				bIsWindowsMenu = TRUE;
			}

			if (lpszCategory != NULL &&
				g_pUserToolsManager != NULL && 
				g_pUserToolsManager->GetToolsEntryCmd () == uiCmd)
			{
				//----------------------------------------------
				// Replace tools entry by the actual tools list:
				//----------------------------------------------
				AddUserTools (lpszCategory);
			}
			else
			{
				CBCGPToolbarButton button (uiCmd, -1, strText);
				
				if (lpszMenuPath != NULL)
				{
					CString strCustom = CString (lpszMenuPath) + button.m_strText;

					LPTSTR pszCustom = strCustom.GetBuffer (strCustom.GetLength () + 1);

					for (int iCount = 0; iCount < lstrlen (pszCustom) - 1; iCount++)
					{
						if (pszCustom [iCount] == _TCHAR(' '))
						{
							CharUpperBuff (&pszCustom [iCount + 1], 1);
						}
					}

					strCustom.ReleaseBuffer();

					strCustom.Remove (_T(' '));
					button.m_strTextCustom = strCustom.SpanExcluding (_T("\t"));
				}

				AddButton (lpszCategory == NULL ?
							m_strAllCommands : lpszCategory, 
							button);
			}
		}
	}

	//--------------------------
	// Add windows manager item:
	//--------------------------
	if (bIsWindowsMenu && lpszCategory != NULL)
	{
		CBCGPMDIFrameWnd* pMainMDIFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, m_pParentFrame);
		if (pMainMDIFrame != NULL && 
			pMainMDIFrame->m_uiWindowsDlgMenuId != 0 &&
			pMainMDIFrame->m_bShowWindowsDlgAlways)
		{
			AddButton (lpszCategory, 
				CBCGPToolbarButton (pMainMDIFrame->m_uiWindowsDlgMenuId, -1,
									pMainMDIFrame->m_strWindowsDlgMenuText));
		}
	}
}
//******************************************************************************************
void CBCGPToolbarCustomize::FillCategoriesComboBox (CComboBox& wndCategory,
												   BOOL bAddEmpty) const
{
	CObList* pCategoryButtonsList;

	for (POSITION pos = m_strCategoriesList.GetHeadPosition(); pos != NULL;)
	{
		CString strCategory = m_strCategoriesList.GetNext (pos);

		if (!m_ButtonsByCategory.Lookup (strCategory, pCategoryButtonsList))
		{
			ASSERT (FALSE);
		}

		ASSERT_VALID (pCategoryButtonsList);

		BOOL bIsEmpty = FALSE;

		if (!bAddEmpty)
		{
			bIsEmpty = TRUE;
			for (POSITION posCat = pCategoryButtonsList->GetHeadPosition (); posCat != NULL;)
			{
				CBCGPToolbarButton* pButton = 
					(CBCGPToolbarButton*) pCategoryButtonsList->GetNext (posCat);
				ASSERT_VALID (pButton);

				if (pButton->m_nID > 0 && pButton->m_nID != (UINT) -1)
				{
					bIsEmpty = FALSE;
					break;
				}
			}
		}

		if (!bIsEmpty)
		{
			int iIndex = wndCategory.AddString (strCategory);
			wndCategory.SetItemData (iIndex, (DWORD_PTR) pCategoryButtonsList);
		}
	}

	// "All" category should be last!
	if (!m_ButtonsByCategory.Lookup (m_strAllCommands, pCategoryButtonsList))
	{
		ASSERT (FALSE);
	}

	ASSERT_VALID (pCategoryButtonsList);

	int iIndex = wndCategory.AddString (m_strAllCommands);
	wndCategory.SetItemData (iIndex, (DWORD_PTR) pCategoryButtonsList);
}
//******************************************************************************************
void CBCGPToolbarCustomize::FillCategoriesListBox (CListBox& wndCategory,
												  BOOL bAddEmpty) const
{
	CObList* pCategoryButtonsList;

	for (POSITION pos = m_strCategoriesList.GetHeadPosition(); pos != NULL;)
	{
		CString strCategory = m_strCategoriesList.GetNext (pos);

		if (!m_ButtonsByCategory.Lookup (strCategory, pCategoryButtonsList))
		{
			ASSERT (FALSE);
		}

		ASSERT_VALID (pCategoryButtonsList);

		BOOL bIsEmpty = FALSE;

		if (!bAddEmpty)
		{
			bIsEmpty = TRUE;
			for (POSITION posCat = pCategoryButtonsList->GetHeadPosition (); posCat != NULL;)
			{
				CBCGPToolbarButton* pButton = 
					(CBCGPToolbarButton*) pCategoryButtonsList->GetNext (posCat);
				ASSERT_VALID (pButton);

				if (pButton->m_nID > 0 && pButton->m_nID != (UINT) -1)
				{
					bIsEmpty = FALSE;
					break;
				}
			}
		}

		if (!bIsEmpty)
		{
			int iIndex = wndCategory.AddString (strCategory);
			wndCategory.SetItemData (iIndex, (DWORD_PTR) pCategoryButtonsList);
		}
	}

	// "All" category should be last!
	if (!m_ButtonsByCategory.Lookup (m_strAllCommands, pCategoryButtonsList))
	{
		ASSERT (FALSE);
	}

	ASSERT_VALID (pCategoryButtonsList);

	int iIndex = wndCategory.AddString (m_strAllCommands);
	wndCategory.SetItemData (iIndex, (DWORD_PTR) pCategoryButtonsList);
}
//*******************************************************************************************
void CBCGPToolbarCustomize::FillAllCommandsList (CListBox& wndListOfCommands) const
{
	wndListOfCommands.ResetContent ();

	CObList* pAllButtonsList;
	if (!m_ButtonsByCategory.Lookup (m_strAllCommands, pAllButtonsList))
	{
		return;
	}

	ASSERT_VALID (pAllButtonsList);

	for (POSITION pos = pAllButtonsList->GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pAllButtonsList->GetNext (pos);
		ASSERT (pButton != NULL);

		int iIndex = wndListOfCommands.AddString (
			pButton->m_strTextCustom.IsEmpty () ? pButton->m_strText : pButton->m_strTextCustom);
		wndListOfCommands.SetItemData (iIndex, (DWORD) pButton->m_nID);
	}
}
//***************************************************************************************
BOOL CBCGPToolbarCustomize::OnHelpInfo(HELPINFO* pHelpInfo)
{
	if ((m_uiFlags & BCGCUSTOMIZE_CONTEXT_HELP) &&
		pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		pHelpInfo->dwContextId = BCGCBHELP_OFFSET + pHelpInfo->iCtrlId;
		AfxGetApp()->WinHelp(pHelpInfo->dwContextId, HELP_CONTEXTPOPUP);
	}
	else
	{
		ASSERT_VALID (m_pParentFrame);
		m_pParentFrame->SendMessage (BCGM_CUSTOMIZEHELP, GetActiveIndex (), 
									(LPARAM) this);
	}

	return TRUE;
}
//***************************************************************************************
LPCTSTR CBCGPToolbarCustomize::GetCommandName (UINT uiCmd) const
{
	CObList* pAllButtonsList;
	if (!m_ButtonsByCategory.Lookup (m_strAllCommands, pAllButtonsList))
	{
		return NULL;
	}

	ASSERT_VALID (pAllButtonsList);

	for (POSITION pos = pAllButtonsList->GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pAllButtonsList->GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->m_nID == uiCmd)
		{
			return pButton->m_strText;
		}
	}

	return NULL;
}
//***************************************************************************************
void CBCGPToolbarCustomize::SetupFromMenus ()
{
	//-------------------------------------------------------------------
	// Find all application document templates and add menue items to the
	// "All commands" category:
	//------------------------------------------------------------------
	CDocManager* pDocManager = AfxGetApp ()->m_pDocManager;
	if (pDocManager != NULL)
	{
		//---------------------------------------
		// Walk all templates in the application:
		//---------------------------------------
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition (); pos != NULL;)
		{
			CMultiDocTemplate* pTemplate = 
				DYNAMIC_DOWNCAST (	CMultiDocTemplate, 
									pDocManager->GetNextDocTemplate (pos));
			if (pTemplate != NULL)
			{
				CMenu* pDocMenu = CMenu::FromHandle (pTemplate->m_hMenuShared);
				if (pDocMenu != NULL)
				{
					AddMenuCommands (pDocMenu, FALSE);
				}
			}
		}
	}

	//------------------------------------
	// Add commands from the default menu:
	//------------------------------------
	CMenu* pFrameMenu = CMenu::FromHandle (m_pParentFrame->m_hMenuDefault);
	if (pFrameMenu == NULL)
	{
		CBCGPMDIFrameWnd* pMainMDIFrame = DYNAMIC_DOWNCAST (CBCGPMDIFrameWnd, m_pParentFrame);
		const CBCGPMenuBar* pMenuBar = NULL;

		if (pMainMDIFrame != NULL)
		{
			pMenuBar = pMainMDIFrame->GetMenuBar ();
		}
		else
		{
			CBCGPFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGPFrameWnd, m_pParentFrame);
			if (pMainFrame != NULL)
			{
				pMenuBar = pMainFrame->GetMenuBar ();
			}
		}

		if (pMenuBar != NULL)
		{
			pFrameMenu = CMenu::FromHandle (pMenuBar->GetDefaultMenu ());
		}
	}

	if (pFrameMenu != NULL)
	{
		AddMenuCommands (pFrameMenu, FALSE);
	}
}
//*****************************************************************************************
void CBCGPToolbarCustomize::AddUserTools (LPCTSTR lpszCategory)
{
	ASSERT (lpszCategory != NULL);
	ASSERT_VALID (g_pUserToolsManager);

	CBCGPLocalResource locaRes;

	const CObList& lstTools = g_pUserToolsManager->GetUserTools ();
	for (POSITION pos = lstTools.GetHeadPosition (); pos != NULL;)
	{
		CBCGPUserTool* pTool = (CBCGPUserTool*) lstTools.GetNext (pos);
		ASSERT_VALID (pTool);

		AddButton (lpszCategory,
			CBCGPToolbarButton (pTool->GetCommandId (), 0, pTool->m_strLabel));
	}
}
//*******************************************************************************
void CBCGPToolbarCustomize::OnContextMenu(CWnd* pWnd, CPoint /*point*/) 
{
	if (g_pWorkspace != NULL && (m_uiFlags & BCGCUSTOMIZE_CONTEXT_HELP))
	{
		g_pWorkspace->OnAppContextHelp (pWnd, dwBCGResHelpIDs);
	}
}
//*******************************************************************************
int CBCGPToolbarCustomize::GetCountInCategory (LPCTSTR lpszItemName,
											  const CObList& lstCommands) const
{
	int nCount = 0;

	for (POSITION pos = lstCommands.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) lstCommands.GetNext (pos);
		ASSERT (pButton != NULL);

		if (pButton->m_strText == lpszItemName)
		{
			nCount++;
		}
	}

	return nCount;
}
//********************************************************************************
BOOL CBCGPToolbarCustomize::OnEditToolbarMenuImage (CWnd* pWndParent, CBitmap& bitmap, int nBitsPerPixel)
{
	CBCGPImageEditDlg dlg (&bitmap, pWndParent, nBitsPerPixel);
	return (dlg.DoModal () == IDOK);
}
