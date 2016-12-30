#if !defined(__BCGPSHELLBREADCRUMB_H)
#define __BCGPSHELLBREADCRUMB_H

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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPBreadcrumb.h"

class CBCGPShellList;

#ifndef BCGP_EXCLUDE_SHELL

class BCGCBPRODLLEXPORT CBCGPShellBreadcrumb : public CBCGPBreadcrumb
{
	DECLARE_DYNAMIC(CBCGPShellBreadcrumb)
		
public:
	CBCGPShellBreadcrumb();

	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID, 
		DWORD dwStyle = WS_CHILD | WS_TABSTOP | WS_VISIBLE | WS_BORDER |
		BCCS_WANTRETURN | BCCS_EXTERNALBORDER | BCCS_INPLACEEDIT,
		DWORD dwStyleEx = 0)
	{
		return CBCGPBreadcrumb::Create(rect, pParentWnd, nID, dwStyle, dwStyleEx);
	}

	void SetRelatedShellList(CBCGPShellList* pShellListControl);
	CBCGPShellList* GetRelatedShellList () const
	{ return m_pRelatedShellList; }

	// Flags are same as in IShellFolder::EnumObjects
	void SetShellFlags (DWORD dwFlags);
	DWORD GetShellFlags () const
	{
		return m_dwShellFlags;
	}

	virtual CString GetItemPath (HBREADCRUMBITEM hItem, TCHAR delimiter = '\\') const;

	virtual BOOL SelectPath(const CString& itemPath, TCHAR delimiter = '\\');
	BOOL SelectShellItem (LPCITEMIDLIST lpidl);

protected:
	DECLARE_MESSAGE_MAP();
	afx_msg void DeleteItem (LPDELETEITEMSTRUCT pDeleteItem);

	virtual void OnInitRoot ();
	virtual void OnSelectionChanged (HBREADCRUMBITEM hSelectedItem);
	virtual void GetItemChildrenDynamic (HBREADCRUMBITEM hParentItem);
	static void ShellObjectToItemInfo (BREADCRUMBITEMINFO* pItemInfo, LPITEMIDLIST pidlCurrent, LPSHELLFOLDER pParentFolder, LPITEMIDLIST pidlParent);

	virtual CEdit* CreateInplaceEdit(CWnd* pParent, const CRect& rect, UINT uiControlID);

private:
	CBCGPShellList* m_pRelatedShellList;
	DWORD           m_dwShellFlags; // SHCONTF_* flags
};

#endif

#endif // __BCGPSHELLBREADCRUMB_H
