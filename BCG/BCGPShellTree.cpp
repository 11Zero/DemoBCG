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
// BCGShellTree.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPShellTree.h"
#include "BCGGlobals.h"

#ifndef BCGP_EXCLUDE_SHELL

#include "BCGPShellList.h"
#include "BCGPShellManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPShellTree

IMPLEMENT_DYNAMIC(CBCGPShellTree, CBCGPTreeCtrl)

IContextMenu2* CBCGPShellTree::m_pContextMenu2 = NULL;

CBCGPShellTree::CBCGPShellTree()
{
	m_bContextMenu = TRUE;
	m_hwndRelatedList = NULL;
	m_bNoNotify = FALSE;
	m_dwFlags = SHCONTF_FOLDERS;
}

CBCGPShellTree::~CBCGPShellTree()
{
}

BEGIN_MESSAGE_MAP(CBCGPShellTree, CBCGPTreeCtrl)
	//{{AFX_MSG_MAP(CBCGPShellTree)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemexpanding)
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnDeleteitem)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONDOWN()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPShellTree message handlers

int CBCGPShellTree::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBCGPTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (g_pShellManager == NULL)
	{
		TRACE0("You need to initialize CBCGPShellManager first\n");
		return -1;
	}
	
	InitTree ();
	return 0;
}
//***************************************************************************************
void CBCGPShellTree::SetRelatedList (CBCGPShellList* pShellList)
{
	ASSERT_VALID (this);

	m_hwndRelatedList = (pShellList == NULL) ? NULL : pShellList->GetSafeHwnd ();
	if (pShellList != NULL)
	{
		pShellList->m_hwndRelatedTree = GetSafeHwnd ();
	}
}
//***************************************************************************************
CBCGPShellList* CBCGPShellTree::GetRelatedList () const
{
	ASSERT_VALID (this);

	if (m_hwndRelatedList == NULL || !::IsWindow (m_hwndRelatedList))
	{
		return NULL;
	}

	CBCGPShellList* pList = DYNAMIC_DOWNCAST (CBCGPShellList,
		CWnd::FromHandlePermanent (m_hwndRelatedList));

	return pList;
}
//****************************************************************************************
void CBCGPShellTree::Refresh ()
{
	ASSERT_VALID (this);

	DeleteAllItems ();

	GetRootItems ();
	TreeView_SetScrollTime (GetSafeHwnd (), 100);
}
//***************************************************************************************
BOOL CBCGPShellTree::GetRootItems ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (g_pShellManager);

	LPITEMIDLIST pidl;

	if (FAILED (SHGetSpecialFolderLocation (NULL, CSIDL_DESKTOP, &pidl)))
	{
		return FALSE;
	}

	//--------------------------------
	// Get the desktop's IShellFolder:
	//--------------------------------
	LPSHELLFOLDER pDesktop;
	if (FAILED (SHGetDesktopFolder (&pDesktop)))
	{
		return FALSE;
	}
	
	//--------------------------------------------
	// Fill in the TVITEM structure for this item:
	//--------------------------------------------
	TV_ITEM	tvItem;
	tvItem.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
	
	//-------------------------------------------
	// Put the private information in the lParam:
	//-------------------------------------------
	LPBCGCBITEMINFO pItem = (LPBCGCBITEMINFO) GlobalAlloc (GPTR, sizeof(BCGCBITEMINFO));
	ASSERT (pItem != NULL);

	pItem->pidlRel = pidl;
	pItem->pidlFQ = g_pShellManager->CopyItem (pidl);
	
	//-------------------------------------------------------------
	// The desktop doesn't have a parent folder, so make this NULL:
	//-------------------------------------------------------------
	pItem->pParentFolder = NULL;
	tvItem.lParam = (LPARAM)pItem;
	
	CString strItem = OnGetItemText (pItem);
	tvItem.pszText = strItem.GetBuffer (strItem.GetLength ());
	tvItem.iImage = OnGetItemIcon (pItem, FALSE);
	tvItem.iSelectedImage = OnGetItemIcon (pItem, TRUE);
	
	//---------------------------------
	// Assume the desktop has children:
	//---------------------------------
	tvItem.cChildren = TRUE;
	
	//-----------------------------------------------------
	// Fill in the TV_INSERTSTRUCT structure for this item:
	//-----------------------------------------------------
	TV_INSERTSTRUCT tvInsert;

	tvInsert.item = tvItem;
	tvInsert.hInsertAfter = TVI_LAST;
	tvInsert.hParent = TVI_ROOT;
	
	//--------------
	// Add the item:
	//--------------
	HTREEITEM hParentItem = InsertItem (&tvInsert);
	
	//-------------------------------
	// Go ahead and expand this item:
	//-------------------------------
	Expand (hParentItem, TVE_EXPAND);
	
	pDesktop->Release();
	return TRUE;
}
//***************************************************************************************
BOOL CBCGPShellTree::GetChildItems (HTREEITEM hParentItem)
{
	ASSERT_VALID (this);

	CWaitCursor wait;

	//----------------------------
	// Get the parent item's pidl:
	//----------------------------
	TVITEM tvItem;
	ZeroMemory (&tvItem, sizeof(tvItem));

	tvItem.mask = TVIF_PARAM;
	tvItem.hItem = hParentItem;

	if (!GetItem (&tvItem))
	{
		return FALSE;
	}
	
	SetRedraw (FALSE);
	
	LPBCGCBITEMINFO pItem = (LPBCGCBITEMINFO) tvItem.lParam;
	ASSERT (pItem != NULL);
	
	LPSHELLFOLDER pParentFolder = NULL;
	HRESULT	hr;

	//--------------------------------------------------------------------
	// If the parent folder is NULL, then we are at the root 
	// of the namespace, so the parent of this item is the desktop folder
	//--------------------------------------------------------------------
	if (pItem->pParentFolder == NULL)
	{
		hr = SHGetDesktopFolder (&pParentFolder);
	}
	else
	{
		//---------------------------------------------------------
		// Otherwise we need to get the IShellFolder for this item:
		//---------------------------------------------------------
		hr = pItem->pParentFolder->BindToObject (pItem->pidlRel, 
					NULL, IID_IShellFolder, (LPVOID*) &pParentFolder);
	}
	
	if (FAILED (hr))
	{
		SetRedraw ();
		return FALSE;
	}
	
	EnumObjects (hParentItem, pParentFolder, pItem->pidlFQ);
	
	//--------------------
	// Sort the new items:
	//--------------------
	TV_SORTCB tvSort;

	tvSort.hParent = hParentItem;
	tvSort.lpfnCompare = CompareProc;
	tvSort.lParam = 0;

	SortChildrenCB (&tvSort);
	
	SetRedraw ();
	RedrawWindow ();
	
	pParentFolder->Release ();
	return TRUE;
}
//***************************************************************************************
HRESULT CBCGPShellTree::EnumObjects (HTREEITEM		hParentItem,
									LPSHELLFOLDER	pParentFolder,
									LPITEMIDLIST	pidlParent)
{
	ASSERT_VALID (this);
	ASSERT_VALID (g_pShellManager);

	LPENUMIDLIST pEnum;
	
	HRESULT hr = pParentFolder->EnumObjects (NULL, m_dwFlags, &pEnum);
	if (FAILED (hr))
	{
		return hr;
	}

    if (pEnum == NULL)
    {
        return E_POINTER;
    }

	LPITEMIDLIST		pidlTemp;
	DWORD				dwFetched = 1;
	
	//----------------------------
	// Enumerate the item's PIDLs:
	//----------------------------
	while (SUCCEEDED (pEnum->Next(1, &pidlTemp, &dwFetched)) && dwFetched)
	{
		TVITEM tvItem;
		ZeroMemory (&tvItem, sizeof (tvItem));
		
		//---------------------------------------------
		// Fill in the TV_ITEM structure for this item:
		//---------------------------------------------
		tvItem.mask =	TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | 
						TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
		
		//------------------------------------------------------
		// AddRef the parent folder so it's pointer stays valid:
		//------------------------------------------------------
		pParentFolder->AddRef ();
		
		//-------------------------------------------
		// Put the private information in the lParam:
		//-------------------------------------------
		LPBCGCBITEMINFO pItem = (LPBCGCBITEMINFO)GlobalAlloc (GPTR, sizeof (BCGCBITEMINFO));
		ASSERT (pItem != NULL);
		
		pItem->pidlRel = pidlTemp;
		pItem->pidlFQ = g_pShellManager->ConcatenateItem (pidlParent, pidlTemp);
		
		pItem->pParentFolder = pParentFolder;
		tvItem.lParam = (LPARAM)pItem;
		
		CString strItem = OnGetItemText (pItem);
		tvItem.pszText = strItem.GetBuffer (strItem.GetLength ());
		tvItem.iImage = OnGetItemIcon (pItem, FALSE);
		tvItem.iSelectedImage = OnGetItemIcon (pItem, TRUE);
		
		//------------------------------------
		// Determine if the item has children:
		//------------------------------------
		DWORD dwAttribs =	SFGAO_HASSUBFOLDER | SFGAO_FOLDER | 
							SFGAO_DISPLAYATTRMASK | SFGAO_CANRENAME;

		pParentFolder->GetAttributesOf (1, (LPCITEMIDLIST*) &pidlTemp, &dwAttribs);
		tvItem.cChildren = (dwAttribs & SFGAO_HASSUBFOLDER);

        if (!globalData.bIsWindowsVista)
        {
            // Exclude Control Panel items for Windows XP and earlier
            if (g_pShellManager->IsControlPanel (pItem->pParentFolder, pItem->pidlRel))
            {
                tvItem.cChildren = 0;
            }
        }
		
		//---------------------------------
		// Determine if the item is shared:
		//---------------------------------
		if (dwAttribs & SFGAO_SHARE)
		{
			tvItem.mask |= TVIF_STATE;
			tvItem.stateMask |= TVIS_OVERLAYMASK;
			tvItem.state |= INDEXTOOVERLAYMASK(1); //1 is the index for the shared overlay image
		}
		
		//-----------------------------------------------------
		// Fill in the TV_INSERTSTRUCT structure for this item:
		//-----------------------------------------------------
		TVINSERTSTRUCT tvInsert;

		tvInsert.item = tvItem;
		tvInsert.hInsertAfter = TVI_LAST;
		tvInsert.hParent = hParentItem;
		
		InsertItem (&tvInsert);
		dwFetched = 0;
	}
	
	pEnum->Release ();
	return S_OK;
}
//***************************************************************************************
int CALLBACK CBCGPShellTree::CompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	UNREFERENCED_PARAMETER(lParamSort);
	
	LPBCGCBITEMINFO pItem1 = (LPBCGCBITEMINFO)lParam1;
	LPBCGCBITEMINFO pItem2 = (LPBCGCBITEMINFO)lParam2;
	
	HRESULT hr = pItem1->pParentFolder->CompareIDs (0,
		pItem1->pidlRel,
		pItem2->pidlRel);
	
	if (FAILED (hr))
	{
		return 0;
	}
	
	return (short)SCODE_CODE(GetScode(hr));
}
//****************************************************************************************
void CBCGPShellTree::OnShowContextMenu (CPoint point)
{
	if (m_pContextMenu2 != NULL)
	{
		return;
	}

	if (!m_bContextMenu)
	{
		Default ();
		return;
	}

	HTREEITEM hItem = NULL;
	if (point.x == -1 && point.y == -1)
	{
		CRect rectItem;

		if ((hItem = GetSelectedItem ()) != NULL &&
			GetItemRect (hItem, rectItem, FALSE))
		{
			point.x = rectItem.left;
			point.y = rectItem.bottom + 1;

			ClientToScreen (&point);
		}
	}
	else
	{
		CPoint ptClient = point;
		ScreenToClient (&ptClient);

		hItem = HitTest (ptClient);
	}

	if (hItem == NULL)
	{
		return;
	}
	
	TVITEM tvItem;
	
	ZeroMemory (&tvItem, sizeof (tvItem));
	tvItem.mask = TVIF_PARAM;
	tvItem.hItem = hItem;
	
	if (!GetItem (&tvItem))
	{
		return;
	}

	LPBCGCBITEMINFO pInfo = (LPBCGCBITEMINFO)tvItem.lParam;
	if (pInfo == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	IShellFolder* psfFolder = pInfo->pParentFolder;
	
	if (psfFolder == NULL)
	{
		SHGetDesktopFolder (&psfFolder);
	}
	else
	{
		psfFolder->AddRef();
	}
	
	if (psfFolder != NULL)
	{
		HWND hwndParent = GetParent()->GetSafeHwnd ();
		IContextMenu* pcm = NULL;

		HRESULT hr = psfFolder->GetUIObjectOf (hwndParent,
			1, 
			(LPCITEMIDLIST*)&pInfo->pidlRel, 
			IID_IContextMenu, 
			NULL, 
			(LPVOID*)&pcm);
		
		if (SUCCEEDED (hr))
		{
			HMENU hPopup = CreatePopupMenu ();
			if (hPopup != NULL)
			{
				hr = pcm->QueryContextMenu(hPopup, 0, 1, 0x7fff, CMF_NORMAL | CMF_EXPLORE);
				
				if (SUCCEEDED (hr))
				{
					pcm->QueryInterface(IID_IContextMenu2, (LPVOID*)&m_pContextMenu2);
					
					HWND hwndThis = GetSafeHwnd ();
					UINT idCmd = TrackPopupMenu (hPopup,
									TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, 
									point.x, 
									point.y,
									0,
									GetSafeHwnd (),
									NULL);
					
					if (::IsWindow (hwndThis))
					{
						if (m_pContextMenu2 != NULL)
						{
							m_pContextMenu2->Release();
							m_pContextMenu2 = NULL;
						}
						
						if (idCmd != 0)
						{
							CWaitCursor wait;

							CMINVOKECOMMANDINFO cmi;
							cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
							cmi.fMask = 0;
							cmi.hwnd = hwndParent;
							cmi.lpVerb = (LPCSTR)(INT_PTR)(idCmd - 1);
							cmi.lpParameters = NULL;
							cmi.lpDirectory = NULL;
							cmi.nShow = SW_SHOWNORMAL;
							cmi.dwHotKey = 0;
							cmi.hIcon = NULL;

							hr = pcm->InvokeCommand (&cmi);

							if (SUCCEEDED (hr) && GetParent () != NULL)
							{
								GetParent ()->SendMessage (BCGPM_ON_AFTER_SHELL_COMMAND,
									(WPARAM) idCmd);
							}

							SetFocus ();
						}
					}
				}
			}
			
			if (pcm != NULL)
			{
				pcm->Release();
				pcm = NULL;
			}
		}
		
		if (psfFolder != NULL)
		{
			psfFolder->Release ();
			psfFolder = NULL;
		}
	}
}
//***************************************************************************************
CString CBCGPShellTree::OnGetItemText (LPBCGCBITEMINFO pItem)
{
	ASSERT (pItem != NULL);
	SHFILEINFO sfi;
	
	if (SHGetFileInfo ((LPCTSTR) pItem->pidlFQ, 0, &sfi, sizeof (sfi),
		SHGFI_PIDL | SHGFI_DISPLAYNAME))
	{
		return sfi.szDisplayName;
	}

	return _T("???");
}
//***************************************************************************************
int CBCGPShellTree::OnGetItemIcon (LPBCGCBITEMINFO pItem, BOOL bSelected)
{
	ASSERT (pItem != NULL);
	SHFILEINFO sfi;

	UINT uiFlags = SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON;

	if (bSelected)
	{
		uiFlags |= SHGFI_OPENICON;
	}
	else
	{
		uiFlags |= SHGFI_LINKOVERLAY;
	}
	
	if (SHGetFileInfo ((LPCTSTR)pItem->pidlFQ, 0, &sfi, sizeof(sfi), uiFlags))
	{
		return sfi.iIcon;
	}

	return -1;
}
//****************************************************************************************
void CBCGPShellTree::OnItemexpanding (NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	ASSERT (hItem != NULL);
	
	switch (pNMTreeView->action)
	{
	case TVE_EXPAND:
		GetChildItems (hItem);

		if (GetChildItem (hItem) == NULL)
		{
			// Remove '+':
			TV_ITEM tvItem;
			ZeroMemory(&tvItem, sizeof(tvItem));

			tvItem.hItem = hItem;
			tvItem.mask = TVIF_CHILDREN;

			SetItem (&tvItem);
		}
		break;
		
	case TVE_COLLAPSE:
		{
			for (HTREEITEM hItemSel = GetSelectedItem (); hItemSel != NULL;)
			{
				HTREEITEM hParentItem = GetParentItem (hItemSel);
				
				if (hParentItem == hItem)
				{
					SelectItem (hItem);
					break;
				}

				hItemSel = hParentItem;
			}

			//remove all of the items from this node
			Expand (hItem, TVE_COLLAPSE | TVE_COLLAPSERESET);
		}
		break;
	}
	
	*pResult = 0;
}
//****************************************************************************************
void CBCGPShellTree::OnDeleteitem(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ASSERT_VALID (g_pShellManager);

	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	LPBCGCBITEMINFO pItem = (LPBCGCBITEMINFO) pNMTreeView->itemOld.lParam;
	
	//-------------------------------------
	// Free up the pidls that we allocated:
	//-------------------------------------
	g_pShellManager->FreeItem (pItem->pidlFQ);
	g_pShellManager->FreeItem (pItem->pidlRel);
	
	//-------------------------------------------
	// This may be NULL if this is the root item:
	//-------------------------------------------
	if (pItem->pParentFolder != NULL)
	{
		pItem->pParentFolder->Release ();
	}
	
	GlobalFree ((HGLOBAL) pItem);
	*pResult = 0;
}
//****************************************************************************************
void CBCGPShellTree::OnContextMenu(CWnd* /*pWnd*/, CPoint point) 
{
	if (m_bContextMenu)
	{
		OnShowContextMenu (point);
	}
	else
	{
		Default ();
	}
}
//****************************************************************************************
BOOL CBCGPShellTree::SelectPath (LPCTSTR lpszPath)
{
	ASSERT_VALID (this);
	ASSERT_VALID (g_pShellManager);
	ASSERT (lpszPath != NULL);

	LPITEMIDLIST pidl;
	if (FAILED (g_pShellManager->ItemFromPath (lpszPath, pidl)))
	{
		return FALSE;
	}

	BOOL bRes = SelectPath (pidl);
	g_pShellManager->FreeItem (pidl);

	return bRes;
}
//***************************************************************************************
BOOL CBCGPShellTree::SelectPath (LPCITEMIDLIST lpidl)
{
	BOOL bRes = FALSE;

	ASSERT_VALID (this);
	ASSERT_VALID (g_pShellManager);

	if (lpidl == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	HTREEITEM htreeItem = GetRootItem ();

	SetRedraw (FALSE);

	if (g_pShellManager->GetItemCount (lpidl) == 0)
	{
		// Desktop
	}
	else
	{
		LPCITEMIDLIST lpidlCurr = lpidl;
		LPITEMIDLIST lpidlParent;

		CList<LPITEMIDLIST,LPITEMIDLIST> lstItems;
		lstItems.AddHead (g_pShellManager->CopyItem (lpidl));

		while (g_pShellManager->GetParentItem (lpidlCurr, lpidlParent) > 0)
		{
			lstItems.AddHead (lpidlParent);
			lpidlCurr = lpidlParent;
		}

		for (POSITION pos = lstItems.GetHeadPosition (); pos != NULL;)
		{
			LPITEMIDLIST lpidlCurr = lstItems.GetNext (pos);

			if (htreeItem != NULL)
			{
				if (GetChildItem (htreeItem) == NULL)
				{
					Expand (htreeItem, TVE_EXPAND);
				}

				BOOL bFound = FALSE;

				for (HTREEITEM hTreeChild = GetChildItem (htreeItem); 
					!bFound && hTreeChild != NULL;
					hTreeChild = GetNextSiblingItem (hTreeChild))
				{
					LPBCGCBITEMINFO pItem = (LPBCGCBITEMINFO) GetItemData (hTreeChild);
					if (pItem == NULL)
					{
						continue;
					}

					SHFILEINFO sfi1;
					SHFILEINFO sfi2;

					if (SHGetFileInfo ((LPCTSTR) pItem->pidlFQ, 0, &sfi1, sizeof (sfi1), SHGFI_PIDL | SHGFI_DISPLAYNAME) &&
						SHGetFileInfo ((LPCTSTR) lpidlCurr, 0, &sfi2, sizeof (sfi2), SHGFI_PIDL | SHGFI_DISPLAYNAME) &&
						lstrcmpi (sfi1.szDisplayName, sfi2.szDisplayName) == 0)
					{
						bFound = TRUE;
						htreeItem = hTreeChild;
					}
				}

				if (!bFound)
				{
					htreeItem = NULL;
				}
			}

			g_pShellManager->FreeItem (lpidlCurr);
		}
	}

	if (htreeItem != NULL)
	{
		m_bNoNotify = TRUE;

		SelectItem (htreeItem);

		if (GetChildItem (htreeItem) == NULL)
		{
			Expand (htreeItem, TVE_EXPAND);
		}

		EnsureVisible (htreeItem);

		m_bNoNotify = FALSE;
		bRes = TRUE;
	}

	SetRedraw ();
	RedrawWindow ();

	return bRes;
}
//****************************************************************************************
BOOL CBCGPShellTree::GetItemPath (CString& strPath, HTREEITEM htreeItem) const
{
	ASSERT_VALID (this);

	BOOL bRes = FALSE;
	strPath.Empty ();

	if (htreeItem == NULL)
	{
		htreeItem = GetSelectedItem ();
	}

	if (htreeItem == NULL)
	{
		return FALSE;
	}

	LPBCGCBITEMINFO pItem = (LPBCGCBITEMINFO) GetItemData (htreeItem);
	if (pItem == NULL || pItem->pidlFQ == NULL || pItem->pidlRel == NULL)
	{
		return FALSE;
	}

	LPSHELLFOLDER lpShellFolder = NULL;
	HRESULT hRes;

	if (pItem->pParentFolder == NULL)
	{
		hRes = SHGetDesktopFolder (&lpShellFolder);
	}
	else
	{
		hRes = pItem->pParentFolder->BindToObject (pItem->pidlRel,
			0, IID_IShellFolder, (LPVOID*) &lpShellFolder);
	}

	if (FAILED (hRes))
	{
		return FALSE;
	}

	ULONG uAttribs = SFGAO_FILESYSTEM;
	if (pItem->pParentFolder != NULL)
	{
		pItem->pParentFolder->GetAttributesOf (1, 
			(const struct _ITEMIDLIST **)&pItem->pidlFQ, &uAttribs);
	}
	// Else - assume desktop

	if ((uAttribs & SFGAO_FILESYSTEM) != 0)
	{
		TCHAR szFolderName [MAX_PATH];
		if (SHGetPathFromIDList (pItem->pidlFQ, szFolderName))
		{
			strPath = szFolderName;
			bRes = TRUE;
		}
	}

	if (lpShellFolder != NULL)
	{
		lpShellFolder->Release ();
	}

	return bRes;
}
//***************************************************************************************
void CBCGPShellTree::OnRButtonDown(UINT /*nFlags*/, CPoint point) 
{
	SetFocus ();
	SelectItem (HitTest (point));
}
//***************************************************************************************
void CBCGPShellTree::EnableShellContextMenu (BOOL bEnable)
{
	m_bContextMenu = bEnable;
}
//******************************************************************************************
BOOL CBCGPShellTree::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult) 
{
	if (message == WM_NOTIFY && !m_bNoNotify)
	{
		LPNMHDR lpnmh = (LPNMHDR) lParam;
		ASSERT (lpnmh != NULL);

		if (lpnmh->code == TVN_SELCHANGED)
		{
			CBCGPShellList* pRelatedShellList = GetRelatedList ();

			if (pRelatedShellList != NULL && 
				GetSelectedItem () != NULL)
			{
				ASSERT_VALID (pRelatedShellList);
				LPBCGCBITEMINFO pItem = (LPBCGCBITEMINFO) GetItemData (GetSelectedItem ());

				pRelatedShellList->m_bNoNotify = TRUE;
				pRelatedShellList->DisplayFolder (pItem);
				pRelatedShellList->m_bNoNotify = FALSE;

				return TRUE;
			}
		}
	}
	
	return CBCGPTreeCtrl::OnChildNotify(message, wParam, lParam, pLResult);
}
//***************************************************************************************
void CBCGPShellTree::OnDestroy() 
{
	CBCGPShellList* pList = GetRelatedList ();
	if (pList != NULL)
	{
		pList->m_hwndRelatedTree = NULL;
	}

	CBCGPTreeCtrl::OnDestroy();
}
//*********************************************************************************
LRESULT CBCGPShellTree::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
	case WM_INITMENUPOPUP:
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
		if (m_pContextMenu2 != NULL)
		{
			m_pContextMenu2->HandleMenuMsg(message, wParam, lParam);
			return 0;
		}
		break;
	}
	
	return CBCGPTreeCtrl::WindowProc(message, wParam, lParam);
}
//*********************************************************************************
void CBCGPShellTree::PreSubclassWindow() 
{
	CBCGPTreeCtrl::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState ();
	if (pThreadState->m_pWndInit == NULL)
	{
		InitTree ();
	}
}
//*********************************************************************************
void CBCGPShellTree::InitTree ()
{
	TCHAR szWinDir [MAX_PATH + 1];
	if (GetWindowsDirectory (szWinDir, MAX_PATH) > 0)
	{
		SHFILEINFO sfi;
		SetImageList (CImageList::FromHandle (
			(HIMAGELIST) SHGetFileInfo( 
							szWinDir,
							0,
							&sfi, 
							sizeof (SHFILEINFO), 
							SHGFI_SYSICONINDEX | SHGFI_SMALLICON)), 0);
	}

	Refresh ();
}
//**********************************************************************************
void CBCGPShellTree::SetFlags (DWORD dwFlags, BOOL bRefresh)
{
	ASSERT_VALID (this);
	m_dwFlags = dwFlags;

	if (bRefresh && GetSafeHwnd () != NULL)
	{
		Refresh ();
	}
}

#endif // BCGP_EXCLUDE_SHELL
