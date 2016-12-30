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

// BCGPContextMenuManager.cpp: implementation of the CBCGPContextMenuManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPContextMenuManager.h"
#include "BCGPTearOffManager.h"
#include "BCGPPopupMenu.h"
#include "MenuHash.h"
#include "BCGGlobals.h"
#include "RegPath.h"
#include "BCGPDialog.h"
#include "BCGPPropertyPage.h"
#include "BCGPWorkspace.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CBCGPWorkspace* g_pWorkspace;

static const CString strMenusProfile = _T("BCGContextMenuManager");

BCGCBPRODLLEXPORT CBCGPContextMenuManager*	g_pContextMenuManager = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPContextMenuManager::CBCGPContextMenuManager()
{
	ASSERT (g_pContextMenuManager == NULL);
	g_pContextMenuManager = this;
	m_nLastCommandID = 0;
	m_bTrackMode = FALSE;
	m_bDontCloseActiveMenu = FALSE;
}
//***********************************************************************************************
CBCGPContextMenuManager::~CBCGPContextMenuManager()
{
	POSITION pos = NULL;

	for (pos = m_Menus.GetStartPosition (); pos != NULL;)
	{
		UINT uiResId;
		HMENU hMenu;

		m_Menus.GetNextAssoc (pos, uiResId, hMenu);
		::DestroyMenu (hMenu);
	}

	for (pos = m_MenuOriginalItems.GetStartPosition (); pos != NULL;)
	{
		UINT uiResId;
		CObList* pLstOrginItems = NULL;

		m_MenuOriginalItems.GetNextAssoc (pos, uiResId, pLstOrginItems);
		ASSERT_VALID (pLstOrginItems);

		while (!pLstOrginItems->IsEmpty ())
		{
			delete pLstOrginItems->RemoveHead ();
		}

		delete pLstOrginItems;
	}

	g_pContextMenuManager = NULL;
}
//**********************************************************************************
BOOL CBCGPContextMenuManager::AddMenu(UINT uiMenuNameResId, UINT uiMenuResId)
{
	CString strMenuName;
	strMenuName.LoadString (uiMenuNameResId);

	return AddMenu (strMenuName, uiMenuResId);
}
//***********************************************************************************************
BOOL CBCGPContextMenuManager::AddMenu(LPCTSTR lpszName, UINT uiMenuResId)
{
	ASSERT (lpszName != NULL);

	CMenu menu;
	if (!menu.LoadMenu (uiMenuResId))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	HMENU hExMenu;
	if (m_Menus.Lookup (uiMenuResId, hExMenu))
	{
		//------------------------------------------
		// Menu with the same name is already exist!
		//------------------------------------------
		return FALSE;
	}

	HMENU hMenu = menu.Detach ();

	if (g_pBCGPTearOffMenuManager != NULL)
	{
		g_pBCGPTearOffMenuManager->SetupTearOffMenus (hMenu);
	}

	m_Menus.SetAt (uiMenuResId, hMenu);
	m_MenuNames.SetAt (lpszName, hMenu);

	return TRUE;
}
//***********************************************************************************************
BOOL CBCGPContextMenuManager::ShowPopupMenu (UINT uiMenuResId, int x, int y, 
											CWnd* pWndOwner, BOOL bOwnMessage,
											BOOL bRightAlign)
{
	HMENU hMenu;
	if (!m_Menus.Lookup (uiMenuResId, hMenu) || hMenu == NULL)
	{
		return FALSE;
	}

	if (x == -1 && y == -1 &&	// Undefined position
		pWndOwner != NULL)
	{
		CRect rectParent;
		pWndOwner->GetClientRect (&rectParent);
		pWndOwner->ClientToScreen (&rectParent);

		x = rectParent.left + 5;
		y = rectParent.top + 5;
	}

	HMENU hmenuPopup = ::GetSubMenu (hMenu, 0);
	if (hmenuPopup == NULL)
	{
		#ifdef _DEBUG

		MENUITEMINFO info;
		memset (&info, 0, sizeof (MENUITEMINFO));

		if (!::GetMenuItemInfo (hMenu, 0, TRUE, &info))
		{
			TRACE (_T ("Invalid menu: %d\n"), uiMenuResId);
		}
		else
		{
			ASSERT (info.hSubMenu == NULL);
			TRACE (_T ("Menu %d, first option '%s' doesn't contain popup menu!\n"), 
					uiMenuResId, info.dwTypeData);
		}

		#endif // _DEBUG
		return FALSE;
	}

	return ShowPopupMenu (hmenuPopup, x, y, pWndOwner, bOwnMessage,
		TRUE, bRightAlign) != NULL;
}
//***********************************************************************************************
CBCGPPopupMenu* CBCGPContextMenuManager::ShowPopupMenu (HMENU hmenuPopup, int x, int y,
											CWnd* pWndOwner, BOOL bOwnMessage,
											BOOL bAutoDestroy,
											BOOL bRightAlign)
{
	if (pWndOwner != NULL &&
		pWndOwner->IsKindOf (RUNTIME_CLASS (CBCGPDialog)) && !bOwnMessage)
	{
		// BCGDialog should own menu messages
		ASSERT (FALSE);
		return NULL;
	}

	if (pWndOwner != NULL &&
		pWndOwner->IsKindOf (RUNTIME_CLASS (CBCGPPropertyPage)) && !bOwnMessage)
	{
		// CBCGPropertyPage should own menu messages
		ASSERT (FALSE);
		return NULL;
	}

	ASSERT (hmenuPopup != NULL);
	if (g_pBCGPTearOffMenuManager != NULL)
	{
		g_pBCGPTearOffMenuManager->SetupTearOffMenus (hmenuPopup);
	}

	if (m_bTrackMode)
	{
		bOwnMessage = TRUE;
	}

	if (!bOwnMessage)
	{
		while (pWndOwner != NULL && pWndOwner->GetStyle() & WS_CHILD)
		{
			pWndOwner = pWndOwner->GetParent ();
		}
	}

	CBCGPPopupMenu* pPopupMenu = new CBCGPPopupMenu;
	if (!globalData.bIsWindowsNT4 || 
		bAutoDestroy)
	{
		pPopupMenu->SetAutoDestroy (FALSE);
	}

	pPopupMenu->m_bTrackMode = m_bTrackMode;
	pPopupMenu->SetRightAlign (bRightAlign);

	CBCGPPopupMenu* pMenuActive = CBCGPPopupMenu::GetActiveMenu ();
	if (!m_bDontCloseActiveMenu && pMenuActive != NULL)
	{
		pMenuActive->SendMessage (WM_CLOSE);
	}

	if (!pPopupMenu->Create (pWndOwner, x, y, hmenuPopup, FALSE, bOwnMessage))
	{
		return NULL;
	}

	return pPopupMenu;
}
//***********************************************************************************************
UINT CBCGPContextMenuManager::TrackPopupMenu (HMENU hmenuPopup, int x, int y,
											 CWnd* pWndOwner, BOOL bRightAlign)
{
	m_nLastCommandID = 0;

	CWinThread* pCurrThread = ::AfxGetThread ();
	if (pCurrThread == NULL)
	{
		ASSERT (FALSE);
		return 0;
	}

	m_bTrackMode = TRUE;

	CBCGPPopupMenu* pMenu = ShowPopupMenu (hmenuPopup, x, y, pWndOwner,
		FALSE, TRUE, bRightAlign);

	if (pMenu != NULL)
	{
		CRect rect;
		pMenu->GetWindowRect (&rect);
		pMenu->UpdateShadow (&rect);
	}

	CBCGPDialog* pParentDlg = NULL;
	if (pWndOwner != NULL && pWndOwner->GetParent () != NULL)
	{
		pParentDlg = DYNAMIC_DOWNCAST (CBCGPDialog, pWndOwner->GetParent ());
		if (pParentDlg != NULL)
		{
			pParentDlg->SetActiveMenu (pMenu);
		}
	}

	CBCGPPropertyPage* pParentPropPage = NULL;
	if (pWndOwner != NULL && pWndOwner->GetParent () != NULL)
	{
		pParentPropPage = DYNAMIC_DOWNCAST (CBCGPPropertyPage, pWndOwner->GetParent ());
		if (pParentPropPage != NULL)
		{
			pParentPropPage->SetActiveMenu (pMenu);
		}
	}

	m_bTrackMode = FALSE;

	if (pMenu != NULL && pCurrThread != NULL)
	{
		ASSERT_VALID (pMenu);

		if (g_pWorkspace == NULL || !g_pWorkspace->OnBCGPIdle (pMenu))
		{
			LONG lIdleCount = 0;
			HWND hwndMenu = pMenu->GetSafeHwnd ();

			while (::IsWindow (hwndMenu))
			{
				MSG msg;
				while (::PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
					{
						PostThreadMessage (GetCurrentThreadId(), 
							msg.message, msg.wParam, msg.lParam);
						return 0;
					}

					if (!::IsWindow (hwndMenu))
					{
						break;
					}

					switch (msg.message)
					{
					case WM_NCLBUTTONDOWN:
						pMenu->DestroyWindow ();

						PostMessage (msg.hwnd,
							msg.message, msg.wParam, msg.lParam);

						if (pParentDlg != NULL)
						{
							pParentDlg->SetActiveMenu (NULL);
						}

						if (pParentPropPage != NULL)
						{
							pParentPropPage->SetActiveMenu (NULL);
						}

						return 0;
					}

					if (::IsWindow (hwndMenu) &&
						!pCurrThread->PreTranslateMessage (&msg))
					{
						::TranslateMessage (&msg);
						::DispatchMessage (&msg);
					}

					if (::IsWindow (hwndMenu) && pMenu->IsIdle ())
					{
						pCurrThread->OnIdle (lIdleCount++);
					}
				}

				// reset "no idle" state after pumping "normal" message
				if (pCurrThread->IsIdleMessage (&msg))
				{
					lIdleCount = 0;
				}

				if (!::IsWindow (hwndMenu))
				{
					break;
				}

				WaitMessage ();
			}
		}
	}

	if (pParentDlg != NULL)
	{
		pParentDlg->SetActiveMenu (NULL);
	}

	if (pParentPropPage != NULL)
	{
		pParentPropPage->SetActiveMenu (NULL);
	}

	return m_nLastCommandID;
}
//***********************************************************************************************
void CBCGPContextMenuManager::GetMenuNames (CStringList& listOfNames) const
{
	listOfNames.RemoveAll ();

	for (POSITION pos = m_MenuNames.GetStartPosition (); pos != NULL;)
	{
		CString strName;
		HMENU hMenu;

		m_MenuNames.GetNextAssoc (pos, strName, hMenu);
		listOfNames.AddTail (strName);
	}
}
//***********************************************************************************************
HMENU CBCGPContextMenuManager::GetMenuByName (LPCTSTR lpszName, UINT* puiOrigResID) const
{
	HMENU hMenu;
	if (!m_MenuNames.Lookup (lpszName, hMenu))
	{
		return NULL;
	}

	if (puiOrigResID != NULL)
	{
		*puiOrigResID = 0;

		for (POSITION pos = m_Menus.GetStartPosition (); pos != NULL;)
		{
			UINT uiResId;
			HMENU hMenuMap;

			m_Menus.GetNextAssoc (pos, uiResId, hMenuMap);
			if (hMenuMap == hMenu)
			{
				*puiOrigResID = uiResId;
				break;
			}
		}
	}

	return hMenu;
}
//***********************************************************************************************
BOOL CBCGPContextMenuManager::LoadState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGPGetRegPath (strMenusProfile, lpszProfileName);

	for (POSITION pos = m_Menus.GetStartPosition (); pos != NULL;)
	{
		UINT uiResId;
		HMENU hMenu;

		m_Menus.GetNextAssoc (pos, uiResId, hMenu);
		ASSERT (hMenu != NULL);

		HMENU hPopupMenu = ::GetSubMenu (hMenu, 0);
		ASSERT (hPopupMenu != NULL);

		CBCGPPopupMenuBar* pBar = new CBCGPPopupMenuBar;

		CWnd* pParentWnd = AfxGetMainWnd ();
		if (pParentWnd == NULL)
		{
			pParentWnd = CWnd::FromHandle (GetDesktopWindow ());
		}

		if (pBar->Create (pParentWnd))
		{
			if (!pBar->ImportFromMenu (hPopupMenu))
			{
				pBar->DestroyWindow ();
				delete pBar;
				return FALSE;
			}

			pBar->BuildOrigItems (uiResId);

			if (pBar->LoadState (strProfileName, 0, uiResId) &&
				!pBar->IsResourceChanged ())
			{
				g_menuHash.SaveMenuBar (hPopupMenu, pBar);
			}

			CopyOriginalMenuItemsFromMenu (uiResId, *pBar);
			pBar->DestroyWindow ();
		}

		delete pBar;
	}

	return TRUE;
}
//***********************************************************************************************
BOOL CBCGPContextMenuManager::SaveState (LPCTSTR lpszProfileName)
{
	CString strProfileName = ::BCGPGetRegPath (strMenusProfile, lpszProfileName);

	for (POSITION pos = m_Menus.GetStartPosition (); pos != NULL;)
	{
		UINT uiResId;
		HMENU hMenu;

		m_Menus.GetNextAssoc (pos, uiResId, hMenu);
		ASSERT (hMenu != NULL);

		HMENU hPopupMenu = ::GetSubMenu (hMenu, 0);
		ASSERT (hPopupMenu != NULL);

		CBCGPPopupMenuBar* pBar = new CBCGPPopupMenuBar;
		if (pBar->Create (CWnd::FromHandle (GetDesktopWindow ())))
		{
			if (g_menuHash.LoadMenuBar (hPopupMenu, pBar))
			{
				CopyOriginalMenuItemsToMenu (uiResId, *pBar);

				if (!pBar->SaveState (strProfileName, 0, uiResId))
				{
					pBar->DestroyWindow ();
					delete pBar;
					return FALSE;
				}
			}
		
			pBar->DestroyWindow ();
		}
		delete pBar;
	}

	return TRUE;
}
//***********************************************************************************************
BOOL CBCGPContextMenuManager::ResetState ()
{
	POSITION pos = NULL;

	for (pos = m_Menus.GetStartPosition (); pos != NULL;)
	{
		UINT uiResId;
		HMENU hMenu;

		m_Menus.GetNextAssoc (pos, uiResId, hMenu);
		ASSERT (hMenu != NULL);

		HMENU hPopupMenu = ::GetSubMenu (hMenu, 0);
		ASSERT (hPopupMenu != NULL);

		g_menuHash.RemoveMenu (hPopupMenu);
	}

	for (pos = m_MenuOriginalItems.GetStartPosition (); pos != NULL;)
	{
		UINT uiResId;
		CObList* pLstOrginItems = NULL;

		m_MenuOriginalItems.GetNextAssoc (pos, uiResId, pLstOrginItems);
		ASSERT_VALID (pLstOrginItems);

		while (!pLstOrginItems->IsEmpty ())
		{
			delete pLstOrginItems->RemoveHead ();
		}

		delete pLstOrginItems;
	}

	m_MenuOriginalItems.RemoveAll ();

	return TRUE;
}
//********************************************************************************
HMENU CBCGPContextMenuManager::GetMenuById (UINT nMenuResId) const
{
   HMENU hMenu = NULL ;
   return m_Menus.Lookup (nMenuResId, hMenu) ? hMenu : NULL;
}
//********************************************************************************
void CBCGPContextMenuManager::CopyOriginalMenuItemsToMenu (UINT uiResId, 
														   CBCGPPopupMenuBar& menuBar)
{
	CObList* pLstOrginItems = NULL;

	if (!m_MenuOriginalItems.Lookup (uiResId, pLstOrginItems))
	{
		return;
	}

	ASSERT_VALID (pLstOrginItems);

	if (pLstOrginItems->IsEmpty ())
	{
		return;
	}

	CObList lstMenuItems;

	for (POSITION pos = pLstOrginItems->GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pSrcButton = DYNAMIC_DOWNCAST (
			CBCGPToolbarButton, pLstOrginItems->GetNext (pos));
		ASSERT_VALID (pSrcButton);

		CRuntimeClass* pClass = pSrcButton->GetRuntimeClass ();
		ASSERT (pClass != NULL);

		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pClass->CreateObject ();
		ASSERT_VALID (pButton);

		pButton->CopyFrom (*pSrcButton);
		lstMenuItems.AddTail (pButton);
	}

	menuBar.SetOrigButtons (lstMenuItems);
}
//********************************************************************************
void CBCGPContextMenuManager::CopyOriginalMenuItemsFromMenu (UINT uiResId, 
															 CBCGPPopupMenuBar& menuBar)
{
	const CObList& lstMenuItems = menuBar.GetOrigButtons();

	CObList* pLstOrginItems = NULL;

	if (m_MenuOriginalItems.Lookup (uiResId, pLstOrginItems))
	{
		ASSERT_VALID (pLstOrginItems);

		while (!pLstOrginItems->IsEmpty ())
		{
			delete pLstOrginItems->RemoveHead ();
		}

		if (lstMenuItems.IsEmpty ())
		{
			m_MenuOriginalItems.RemoveKey (uiResId);
			delete pLstOrginItems;
			return;
		}
	}
	else
	{
		if (lstMenuItems.IsEmpty ())
		{
			return;
		}

		pLstOrginItems = new CObList;
		m_MenuOriginalItems.SetAt (uiResId, pLstOrginItems);
	}

	ASSERT_VALID (pLstOrginItems);

	for (POSITION pos = lstMenuItems.GetHeadPosition (); pos != NULL;)
	{
		CBCGPToolbarButton* pSrcButton = DYNAMIC_DOWNCAST (
			CBCGPToolbarButton, lstMenuItems.GetNext (pos));
		ASSERT_VALID (pSrcButton);

		CRuntimeClass* pClass = pSrcButton->GetRuntimeClass ();
		ASSERT (pClass != NULL);

		CBCGPToolbarButton* pButton = (CBCGPToolbarButton*) pClass->CreateObject ();
		ASSERT_VALID (pButton);

		pButton->CopyFrom (*pSrcButton);
		pLstOrginItems->AddTail (pButton);
	}
}
