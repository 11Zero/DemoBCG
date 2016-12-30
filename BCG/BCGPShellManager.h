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
// BCGPShellManager.h: interface for the CBCGPShellManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPSHELLMANAGER_H__E19B2D53_BD01_4B0B_94B9_EB32C246CFA5__INCLUDED_)
#define AFX_BCGPSHELLMANAGER_H__E19B2D53_BD01_4B0B_94B9_EB32C246CFA5__INCLUDED_

#ifndef _SHLOBJ_H_
#include <shlobj.h>
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

#ifndef BCGP_EXCLUDE_SHELL

typedef struct _BCGCBITEMINFO
{
	LPSHELLFOLDER  pParentFolder;
	LPITEMIDLIST   pidlFQ;
	LPITEMIDLIST   pidlRel;

	_BCGCBITEMINFO()
	{
		pParentFolder = NULL;
		pidlFQ = NULL;
		pidlRel = NULL;
	}
}
BCGCBITEMINFO, FAR *LPBCGCBITEMINFO;

class BCGCBPRODLLEXPORT CBCGPShellManager : public CObject
{
	friend class CBCGPShellList;

// Construction
public:
	CBCGPShellManager();
	virtual ~CBCGPShellManager();

// Operations
public:
	BOOL BrowseForFolder (	CString& strOutFolder,
							CWnd* pWndParent = NULL,
							LPCTSTR lplszInitialFolder = NULL,
							LPCTSTR lpszTitle = NULL, 
							UINT ulFlags = BIF_RETURNONLYFSDIRS, // Same as flags in BROWSEINFO structure
							LPINT piFolderImage = NULL);

	LPITEMIDLIST CreateItem (UINT cbSize);
	void FreeItem (LPITEMIDLIST pidl);

	UINT GetItemCount (LPCITEMIDLIST pidl);
	UINT GetItemSize (LPCITEMIDLIST pidl);

	LPITEMIDLIST ConcatenateItem (LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
	LPITEMIDLIST CopyItem (LPCITEMIDLIST pidlSource);

	LPITEMIDLIST GetNextItem(LPCITEMIDLIST pidl);
	int GetParentItem (LPCITEMIDLIST lpidl, LPITEMIDLIST& lpidlParent);

	HRESULT ItemFromPath (LPCTSTR lpszPath, LPITEMIDLIST& pidl);
    BOOL IsControlPanel (IShellFolder* pParentFolder, LPITEMIDLIST& pidl);

protected:
	static int CALLBACK BrowseCallbackProc (HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

// Attributes
protected:
	LPMALLOC	m_pBCGMalloc;
	LPCTSTR		m_lpszInitialPath;	// Valid in BrowseForFolder only
};

extern BCGCBPRODLLEXPORT CBCGPShellManager* g_pShellManager;

extern BCGCBPRODLLEXPORT UINT BCGPM_ON_AFTER_SHELL_COMMAND;

#endif	// BCGP_EXCLUDE_SHELL

#endif // !defined(AFX_BCGPSHELLMANAGER_H__E19B2D53_BD01_4B0B_94B9_EB32C246CFA5__INCLUDED_)
