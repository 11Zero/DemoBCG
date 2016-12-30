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


#include "stdafx.h"
#include "BCGPShellBreadcrumb.h"

#ifndef _BCGSUITE_
#include "BCGPShellList.h"
#include "BCGGlobals.h"
#endif
#include "BCGPGlobalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef BCGP_EXCLUDE_SHELL

CBCGPShellManager* GetShellManager()
{
#ifndef _BCGSUITE_
	return g_pShellManager;
#else
	CShellManager* pShellManager = NULL;
	CWinAppEx* pApp = DYNAMIC_DOWNCAST(CWinAppEx, AfxGetApp ());
	if (pApp != NULL)
	{
		pShellManager = pApp->GetShellManager ();
	}

	return pShellManager;
#endif
}

//////////////////////////////////////////////////////////////////////////
// CBCGPShellBreadcrumb class implementation

IMPLEMENT_DYNAMIC (CBCGPShellBreadcrumb, CBCGPBreadcrumb)


BEGIN_MESSAGE_MAP (CBCGPShellBreadcrumb, CBCGPBreadcrumb)
	ON_WM_DELETEITEM_REFLECT()
END_MESSAGE_MAP()

CBCGPShellBreadcrumb::CBCGPShellBreadcrumb ()
	: m_dwShellFlags      (SHCONTF_FOLDERS)
	, m_pRelatedShellList (NULL)
{
}

void CBCGPShellBreadcrumb::SetShellFlags (DWORD dwFlags)
{
	if (m_dwShellFlags != dwFlags)
	{
		m_dwShellFlags = dwFlags;

		if (GetSafeHwnd () != NULL)
		{
			SendMessage (BCCM_RESETCONTENT);
		}
	}
}

void CBCGPShellBreadcrumb::OnInitRoot()
{
	if (GetShellManager() == NULL)
	{
		TRACE0 ("You need to initialize CBCGPShellManager first. Call InitShellManager() from the application InitInstance() method.\n");
		ASSERT (FALSE);
		return;
	}

	// Setup shell image list

	TCHAR szWinDir [MAX_PATH + 1];
	if (GetWindowsDirectory (szWinDir, MAX_PATH) > 0)
	{
		SHFILEINFO sfi;
		HIMAGELIST hImageList = (HIMAGELIST) SHGetFileInfo (szWinDir, 0, &sfi, sizeof (SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
		SendMessage (BCCM_SETIMAGELIST, 0, (LPARAM)hImageList);
	}

	// Setup root item

	LPITEMIDLIST pidlRoot;

	if (FAILED (SHGetSpecialFolderLocation (NULL, CSIDL_DESKTOP, &pidlRoot)))
	{
		return;
	}

	LPSHELLFOLDER pRootShellFolder;
	if (FAILED (SHGetDesktopFolder (&pRootShellFolder)))
	{
		return;
	}

	BREADCRUMBITEMINFO rootItemInfo;
	ZeroMemory (&rootItemInfo, sizeof (rootItemInfo));
	rootItemInfo.pszText = new TCHAR [MAX_PATH + 1];
	rootItemInfo.cchTextMax = MAX_PATH;

	ShellObjectToItemInfo (&rootItemInfo, pidlRoot, NULL, NULL);
	rootItemInfo.mask = BCCIF_TEXT | BCCIF_TOOLTIP | BCCIF_IMAGE | BCCIF_DYNAMIC | BCCIF_PARAM;
	HBREADCRUMBITEM hRoot = GetRootItem ();
	SendMessage (BCCM_SETITEM, (WPARAM)hRoot, (LPARAM)&rootItemInfo);

	delete rootItemInfo.pszText;
	pRootShellFolder->Release ();
}

BOOL CBCGPShellBreadcrumb::SelectPath (const CString& strPath, TCHAR delimiter)
{
	ASSERT_VALID (this);

	CBCGPShellManager* pShellManager = GetShellManager();
	ASSERT_VALID (pShellManager);

	if (delimiter != '\\')
	{
		TRACE(_T("The shell breadcrumb supports only back-slash delimiters.\n"));
		ASSERT(FALSE);
	}



	LPITEMIDLIST pidl;
	if (FAILED (pShellManager->ItemFromPath (strPath, pidl)))
	{
		return CBCGPBreadcrumb::SelectPath (strPath, delimiter);
	}

	BOOL bRes = SelectShellItem (pidl);
	pShellManager->FreeItem (pidl);

	return bRes;
}

BOOL CBCGPShellBreadcrumb::SelectShellItem (LPCITEMIDLIST lpidl)
{
	ASSERT_VALID (this);

	CBCGPShellManager* pShellManager = GetShellManager();
	ASSERT_VALID (pShellManager);

	if (lpidl == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	HBREADCRUMBITEM hItem = GetRootItem ();

	if (pShellManager->GetItemCount (lpidl) != 0)
	{
		LPCITEMIDLIST lpidlCurrent = lpidl;
		LPITEMIDLIST lpidlParent;

		// Building full PIDL path
		CList<LPITEMIDLIST,LPITEMIDLIST> lstItems;
		lstItems.AddHead (pShellManager->CopyItem (lpidl));

		while (pShellManager->GetParentItem (lpidlCurrent, lpidlParent) > 0)
		{
			lstItems.AddHead (lpidlParent);
			lpidlCurrent = lpidlParent;
		}

		// Searching for corresponding breadcrumb item
		for (POSITION pos = lstItems.GetHeadPosition (); pos != NULL && hItem != NULL;)
		{
			LPITEMIDLIST lpidlCurr = lstItems.GetNext (pos);

			BOOL bFound = FALSE;

			// Searching through breadcrumb items
			for (int i = GetSubItemsCount (hItem) - 1; i >= 0; --i)
			{
				HBREADCRUMBITEM hSubItem = GetSubItem (hItem, i);

				LPBCGCBITEMINFO pItem = (LPBCGCBITEMINFO) GetItemData (hSubItem);
				if (pItem == NULL)
				{
					continue;
				}

				SHFILEINFO sfi1;
				SHFILEINFO sfi2;
				SHGetFileInfo ((LPCTSTR) pItem->pidlFQ, 0, &sfi1, sizeof (sfi1), SHGFI_PIDL | SHGFI_DISPLAYNAME);
				SHGetFileInfo ((LPCTSTR) lpidlCurr, 0, &sfi2, sizeof (sfi2), SHGFI_PIDL | SHGFI_DISPLAYNAME);
				if (lstrcmp (sfi1.szDisplayName, sfi2.szDisplayName) == 0)
				{
					bFound = TRUE;
					hItem = hSubItem;
					break;
				}
			}

			if (!bFound)
			{
				hItem = NULL;
			}

			pShellManager->FreeItem (lpidlCurr);
		}
	}

	if (hItem == NULL)
	{
		return FALSE;
	}

	SelectItem (hItem);
	return TRUE;
}


void CBCGPShellBreadcrumb::OnSelectionChanged(HBREADCRUMBITEM hSelectedItem)
{
	if (m_pRelatedShellList != NULL)
	{
		LPBCGCBITEMINFO pItem = (LPBCGCBITEMINFO)GetItemData (hSelectedItem);
		if (pItem != NULL)
		{
			m_pRelatedShellList->DisplayFolder (pItem);
		}
	}
	else if (GetParent () != NULL)
	{
		GetParent ()->SendMessage (BCGPM_CHANGE_CURRENT_FOLDER);
	}
}

void CBCGPShellBreadcrumb::DeleteItem (LPDELETEITEMSTRUCT pDeleteItem)
{
	LPBCGCBITEMINFO	pItemParam = (LPBCGCBITEMINFO)pDeleteItem->itemData;
	
	CBCGPShellManager* pShellManager = GetShellManager();
	ASSERT_VALID(pShellManager);

	//free up the PIDLs that we allocated
	pShellManager->FreeItem (pItemParam->pidlFQ);
	pShellManager->FreeItem (pItemParam->pidlRel);
	
	//this may be NULL if this is the root item
	if (pItemParam->pParentFolder != NULL)
	{
		pItemParam->pParentFolder->Release ();
		pItemParam->pParentFolder = NULL;
	}
	
	GlobalFree ((HGLOBAL) pItemParam);
}

void CBCGPShellBreadcrumb::ShellObjectToItemInfo (BREADCRUMBITEMINFO* pItemInfo, LPITEMIDLIST pidlCurrent, LPSHELLFOLDER pParentFolder, LPITEMIDLIST pidlParent)
{
	ASSERT(pItemInfo != NULL);
	if (pItemInfo == NULL) return;

	pItemInfo->iImage = -1;
	if (pItemInfo->pszText != NULL)
	{
		*pItemInfo->pszText = 0;
	}

	if (pidlCurrent == NULL)
	{
		return;
	}

	CBCGPShellManager* pShellManager = GetShellManager();
	ASSERT_VALID(pShellManager);

	LPBCGCBITEMINFO pItemParam = (LPBCGCBITEMINFO)GlobalAlloc (GPTR, sizeof (BCGCBITEMINFO));
	ASSERT (pItemParam != NULL);
		
	pItemParam->pidlRel = pidlCurrent;
	if (pParentFolder == NULL || pidlParent == NULL)
	{
		pItemParam->pidlFQ = pShellManager->CopyItem (pidlCurrent);

		pItemParam->pParentFolder = NULL;
	}
	else
	{
		pItemParam->pidlFQ = pShellManager->ConcatenateItem (pidlParent, pidlCurrent);
			
		pParentFolder->AddRef (); // Make a strong reference to parent folder.
		pItemParam->pParentFolder = pParentFolder;
	}
	
	pItemInfo->lParam = (LPARAM)pItemParam;

	// Query icon and text
	SHFILEINFO sfi;
	UINT uiFlags = SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_OPENICON | SHGFI_DISPLAYNAME;
	if (SHGetFileInfo ((LPCTSTR)pItemParam->pidlFQ, 0, &sfi, sizeof(sfi), uiFlags))
	{
		pItemInfo->iImage = sfi.iIcon;
		if (pItemInfo->pszText != 0 && pItemInfo->cchTextMax > 0)
		{
			lstrcpyn (pItemInfo->pszText, sfi.szDisplayName, pItemInfo->cchTextMax);
		}
	}
	
	pItemInfo->bDynamic = TRUE;
}

int __cdecl CompareBreadcrumbItemInfo (const void* pLeft, const void* pRight)
{
	if (pLeft == NULL || pRight == NULL) return 0;
	CString left(((const BREADCRUMBITEMINFO*)pLeft)->pszText);
	return left.CollateNoCase(((const BREADCRUMBITEMINFO*)pRight)->pszText);
}

void CBCGPShellBreadcrumb::GetItemChildrenDynamic (HBREADCRUMBITEM hParentItem)
{
	ASSERT_VALID (this);

	CWaitCursor wait;
	
	LPBCGCBITEMINFO pParentItemInfo = (LPBCGCBITEMINFO)GetItemData (hParentItem);
	ASSERT (pParentItemInfo != NULL);
	if (pParentItemInfo == NULL) return;
	
	HRESULT	hr;
	LPSHELLFOLDER pParentFolder = NULL;

	//--------------------------------------------------------------------
	// If the parent folder is NULL, then we are at the root 
	// of the namespace, so the parent of this item is the desktop folder
	//--------------------------------------------------------------------
	if (pParentItemInfo->pParentFolder == NULL)
	{
		hr = SHGetDesktopFolder (&pParentFolder);
	}
	else
	{
		//---------------------------------------------------------
		// Otherwise we need to get the IShellFolder for this item:
		//---------------------------------------------------------
		hr = pParentItemInfo->pParentFolder->BindToObject (pParentItemInfo->pidlRel, 
					NULL, IID_IShellFolder, (LPVOID*) &pParentFolder);
	}
	
	if (FAILED (hr))
	{
		return;
	}

#ifndef _BCGSUITE_
    if (!globalData.bIsWindowsVista)
    {
		CBCGPShellManager* pShellManager = GetShellManager();
		ASSERT_VALID (pShellManager);

        // Exclude Control Panel items for Windows XP and earlier
        if (pShellManager->IsControlPanel (pParentItemInfo->pParentFolder, pParentItemInfo->pidlRel))
        {
            return;
        }
    }
#endif

	// Enumerate the items
	LPENUMIDLIST pEnum;
	
	hr = pParentFolder->EnumObjects (NULL, m_dwShellFlags, &pEnum);
	if (FAILED (hr) || pEnum == NULL)
	{
		return;
	}

	CArray<BREADCRUMBITEMINFO, const BREADCRUMBITEMINFO&> arrSubItems;

	LPITEMIDLIST		pidlCurrent;
	DWORD				dwFetched = 1;
	
	while (SUCCEEDED (pEnum->Next(1, &pidlCurrent, &dwFetched)) && dwFetched)
	{
		BREADCRUMBITEMINFO itemInfo;
		ZeroMemory (&itemInfo, sizeof(itemInfo));

		itemInfo.pszText = new TCHAR [MAX_PATH + 1];
		itemInfo.cchTextMax = MAX_PATH;

		ShellObjectToItemInfo (&itemInfo, pidlCurrent, pParentFolder, pParentItemInfo->pidlFQ);
		arrSubItems.Add (itemInfo);

		dwFetched = 0;
	}
	
	pEnum->Release ();
	pParentFolder->Release ();
	
	if (arrSubItems.GetSize () > 0)
	{
		// Sorting items
		qsort (&arrSubItems[0], (size_t)arrSubItems.GetSize (), sizeof(arrSubItems[0]), CompareBreadcrumbItemInfo);

		// Inserting items
		for (int i = 0; i < (int)arrSubItems.GetSize (); ++i)
		{
			if (::lstrcmp (arrSubItems[i].pszText, _T("...")) != 0)
			{
				arrSubItems[i].hParentItem = hParentItem;
				SendMessage (BCCM_INSERTITEM, 0, (LPARAM)&arrSubItems[i]);
			}

			delete arrSubItems[i].pszText;
		}
	}
}

void CBCGPShellBreadcrumb::SetRelatedShellList(CBCGPShellList* pShellListControl)
{
	if (m_pRelatedShellList != pShellListControl)
	{
		m_pRelatedShellList = pShellListControl;
		if (m_pRelatedShellList != NULL)
		{
			LPBCGCBITEMINFO pItem = (LPBCGCBITEMINFO)GetItemData (GetSelectedItem ());
			if (pItem != NULL)
			{
				m_pRelatedShellList->DisplayFolder (pItem);
			}
		}
	}
}

CString CBCGPShellBreadcrumb::GetItemPath (HBREADCRUMBITEM hItem, TCHAR delimiter) const
{
	CString strPath;

	LPBCGCBITEMINFO pItem = (LPBCGCBITEMINFO)GetItemData (hItem);
	if (pItem == NULL || pItem->pidlFQ == NULL)
	{
		return strPath;
	}

	TCHAR szPath [MAX_PATH];
	if (SHGetPathFromIDList (pItem->pidlFQ, szPath))
	{
		strPath = szPath;
		if (delimiter != _T('\\'))
		{
			strPath.Replace (_T('\\'), delimiter);
		}
	}
	else
	{
		strPath = CBCGPBreadcrumb::GetItemPath (hItem, delimiter);
	}

	return strPath;
}

CEdit* CBCGPShellBreadcrumb::CreateInplaceEdit (CWnd* pParent, const CRect& rect, UINT uiControlID)
{
	CEdit* pEdit = CBCGPBreadcrumb::CreateInplaceEdit (pParent, rect, uiControlID);
	
	if (pEdit->GetSafeHwnd() != NULL)
	{	
		globalUtils.EnableEditCtrlAutoComplete (pEdit->GetSafeHwnd (), TRUE);
	}

	return pEdit;
}

#endif // BCGP_EXCLUDE_SHELL
