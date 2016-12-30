#if !defined(__BCGPBREADCRUMB_H)
#define __BCGPBREADCRUMB_H

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

#include "BCGPBreadcrumbControlImpl.h"

class CBCGPBreadcrumbImpl;

class BCGCBPRODLLEXPORT CBCGPBreadcrumb : public CEdit
{
	DECLARE_DYNAMIC(CBCGPBreadcrumb)

// Constructors
public:
	CBCGPBreadcrumb();

	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID, 
		DWORD dwStyle = WS_CHILD | WS_TABSTOP | WS_VISIBLE | WS_BORDER |
		BCCS_WANTRETURN | BCCS_EXTERNALBORDER,
		DWORD dwStyleEx = 0);

	virtual ~CBCGPBreadcrumb();

// Attributes
public:
	HBREADCRUMBITEM GetRootItem() const;

	HBREADCRUMBITEM GetSelectedItem() const;
	void SelectItem(HBREADCRUMBITEM hItem);
	virtual BOOL SelectPath(const CString& itemPath, TCHAR delimiter = '\\');

	virtual CString GetItemPath(HBREADCRUMBITEM hItem, TCHAR delimiter = '\\') const;
	CString GetSelectedPath(TCHAR delimiter = '\\') const;

	HBREADCRUMBITEM InsertItem(HBREADCRUMBITEM hParent, const CString& text, int iImage = -1, LPARAM lParam = 0);

	BOOL DeleteItem(HBREADCRUMBITEM hItem);
	BOOL SetItemText(HBREADCRUMBITEM hItem, const CString& text);
	virtual CString GetItemText(HBREADCRUMBITEM hItem) const;

	BOOL SetItemTooltipText(HBREADCRUMBITEM hItem, const CString& text);
	CString GetItemTooltipText(HBREADCRUMBITEM hItem) const;

	BOOL SetItemImageIndex(HBREADCRUMBITEM hItem, int iImageIndex);
	int GetItemImageIndex(HBREADCRUMBITEM hItem) const;

	BOOL SetItemDynamic(HBREADCRUMBITEM hItem, BOOL bDynamic = TRUE);
	BOOL GetItemDynamic(HBREADCRUMBITEM hItem) const;

	BOOL SetItemData(HBREADCRUMBITEM hItem, LPARAM lParamData);
	LPARAM GetItemData(HBREADCRUMBITEM hItem) const;

	HBREADCRUMBITEM GetItemParent(HBREADCRUMBITEM hItem) const;

	int	GetSubItemsCount(HBREADCRUMBITEM hParentItem) const;
	HBREADCRUMBITEM	GetSubItem(HBREADCRUMBITEM hParentItem, int iIndex) const;
	HBREADCRUMBITEM	GetSubItemByName(HBREADCRUMBITEM hParentItem, const CString& itemName) const;

	CImageList* SetImageList(CImageList* pImageList);
	CImageList* GetImageList() const;

	COLORREF SetDefaultTextColor(COLORREF color);
	COLORREF GetDefaultTextColor() const;

	COLORREF SetBackColor(COLORREF color);
	COLORREF GetBackColor() const;

	COLORREF SetDefaultHighlightedTextColor(COLORREF color);
	COLORREF GetDefaultHighlightedTextColor() const;

	COLORREF SetDefaultHighlightColor(COLORREF color);
	COLORREF GetDefaultHighlightColor() const;

	UINT SetRightMargin(UINT cxMargin);
	UINT GetRightMargin() const;

	BOOL IsOnGlass() const
	{
		return m_bOnGlass;
	}

// Overridables
protected:
	virtual void OnInitRoot();
	virtual BOOL OnKeyDown(UINT nVKey, UINT uFlags);
	virtual void OnReturnKey();
	virtual void OnLeftClick(POINT ptClick, UINT uHitTest, HBREADCRUMBITEM hItem);
	virtual void OnRightClick(POINT ptClick, UINT uHitTest, HBREADCRUMBITEM hItem);
	virtual void OnSelectionChanged(HBREADCRUMBITEM hSelectedItem);
	virtual void GetItemChildrenDynamic(HBREADCRUMBITEM hParentItem);

	virtual void BeginInplaceEditing ();
	virtual CEdit* CreateInplaceEdit(CWnd* pParent, const CRect& rect, UINT uiControlID);
	virtual void DestroyInplaceEdit(CEdit* pEditor);
	virtual BOOL ValidateInplaceEdit(const CString& strPath);

	virtual BOOL PreTranslateMessage (MSG* pMsg);
	virtual void PreSubclassWindow ();
	virtual LRESULT WindowProc (UINT message, WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

protected:
	afx_msg void OnInitRootReflect(NMHDR* pNmhdr, LRESULT* pResult);
	afx_msg BOOL OnKeyDownReflect(NMHDR* pNmhdr, LRESULT* pResult);
	afx_msg void OnReturnKeyReflect(NMHDR* pNmhdr, LRESULT* pResult);
	afx_msg void OnSelectionChangeReflect(NMHDR* pNmhdr, LRESULT* pResult);
	afx_msg void OnLClickReflect(NMHDR* pNmhdr, LRESULT* pResult);
	afx_msg void OnRClickReflect(NMHDR* pNmhdr, LRESULT* pResult);
	afx_msg void OnBeginInplaceEditingReflect(NMHDR* pNmhdr, LRESULT* pResult);
	afx_msg BOOL OnGetChildrenReflect(NMHDR* pNmhdr, LRESULT* pResult);
	afx_msg void CancelInplaceEditor ();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnBCGSetControlAero (WPARAM, LPARAM);
	afx_msg LRESULT OnBCGSetControlVMMode (WPARAM, LPARAM);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

public:
	BOOL					m_bVisualManagerStyle;

protected:
	CBCGPBreadcrumbImpl*	m_pImpl;
	CEdit*					m_pInplaceEdit;
	BOOL					m_bInCreate;
	BOOL					m_bOnGlass;
};

#endif // __BCGPBREADCRUMB_H
