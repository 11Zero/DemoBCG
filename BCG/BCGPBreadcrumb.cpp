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
// BCGPBreadcrumb.cpp : implementation file
//

#include "stdafx.h"
#include "BCGPBreadcrumb.h"
#include "BCGPDrawManager.h"
#include "BCGPGlobalUtils.h"
#include "Bcgglobals.h"
#include "BCGPEdit.h"
#include "BCGPDlgImpl.h"

extern BCGCBPRODLLEXPORT UINT BCGM_ONSETCONTROLAERO;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define  IDC_BREADCRUMB_INPLACE_EDITOR   0x1001


IMPLEMENT_DYNAMIC(CBCGPBreadcrumb, CEdit)

CBCGPBreadcrumb::CBCGPBreadcrumb()
	: m_pInplaceEdit(NULL)
	, m_pImpl(NULL)
	, m_bInCreate(FALSE)
	, m_bOnGlass(FALSE)
	, m_bVisualManagerStyle(FALSE)
{
}

BOOL CBCGPBreadcrumb::Create (const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwStyle, DWORD dwStyleEx)
{
	m_bInCreate = TRUE;
	BOOL bRes = CWnd::CreateEx (dwStyleEx, globalData.RegisterWindowClass (_T("BCGPBreadcrumb")), _T(""), dwStyle, rect, pParentWnd, nID);
	m_bInCreate = FALSE;

	return bRes;
}

CBCGPBreadcrumb::~CBCGPBreadcrumb ()
{
}

HBREADCRUMBITEM CBCGPBreadcrumb::GetRootItem () const
{
	return (HBREADCRUMBITEM)::SendMessage (m_hWnd, BCCM_GETROOTITEM, 0, 0);
}

HBREADCRUMBITEM CBCGPBreadcrumb::GetSelectedItem () const
{
	return (HBREADCRUMBITEM)::SendMessage (m_hWnd, BCCM_GETSELECTEDITEM, 0, 0);
}

CString CBCGPBreadcrumb::GetItemPath (HBREADCRUMBITEM hItem, TCHAR delimiter) const
{
	CString strPath;
	HBREADCRUMBITEM hRootItem = GetRootItem ();
	BOOL bRootItemIsEmpty = GetItemText (hRootItem).IsEmpty ();

	while (hItem != NULL)
	{
		CString strItem = GetItemText (hItem);
		if (hItem == hRootItem && bRootItemIsEmpty)
		{
			// Do not add the root item if it has no text.
			break;
		}

		globalUtils.StringAddItemQuoted (strPath, strItem, TRUE, delimiter, '\"');

		hItem = GetItemParent (hItem);
	}

	return strPath;
}

CString CBCGPBreadcrumb::GetSelectedPath (TCHAR delimiter) const
{
	return GetItemPath (GetSelectedItem (), delimiter);
}

void CBCGPBreadcrumb::SelectItem (HBREADCRUMBITEM hItem)
{
	SendMessage (BCCM_SELECTITEM, 0, (LPARAM)hItem);
}

BOOL CBCGPBreadcrumb::SelectPath (const CString& itemPath, TCHAR delimiter)
{
	HBREADCRUMBITEM hRootItem = GetRootItem ();
	HBREADCRUMBITEM hItem = hRootItem;
	TCHAR tokens[2] = {delimiter, 0};

	int iPos = 0;
	CString s;

	// The root item may be omitted.
	CString strRoot = GetItemText (hRootItem);
	if (GetSubItemByName(hRootItem, strRoot) == NULL) // If there's no "root_item_text\root_item_text" item, continue.
	{
		s = globalUtils.StringExtractItemQuoted (itemPath, tokens, iPos);
		if (s.IsEmpty ())
		{
			return FALSE;
		}
		if (s != strRoot)
		{
			hItem = GetSubItemByName (hRootItem, s);
			if (hItem == NULL)
			{
				return FALSE;
			}
		}
	}

	while (!s.IsEmpty ())
	{
		CString s = globalUtils.StringExtractItemQuoted (itemPath, tokens, iPos);
		if (s.IsEmpty ()) break;
		if (hItem == hRootItem && s == GetItemText (hRootItem))
		{
			SelectItem (hRootItem);
		}
		HBREADCRUMBITEM hSubItem = GetSubItemByName (hItem, s);
		if (hSubItem == NULL) break;
		hItem = hSubItem;
	}

	if (hItem != NULL)
	{
		SelectItem (hItem);
		return TRUE;
	}

	return FALSE;
}

HBREADCRUMBITEM CBCGPBreadcrumb::InsertItem (HBREADCRUMBITEM hParent, const CString& text, int iImage /*= -1*/, LPARAM lParam /*= 0*/)
{
	BREADCRUMBITEMINFO info;
	ZeroMemory (&info, sizeof (info));
	info.hParentItem = hParent;
	info.iImage = iImage;
	info.lParam = lParam;
	info.pszText = const_cast<LPTSTR>((LPCTSTR)text);
	return (HBREADCRUMBITEM)SendMessage (BCCM_INSERTITEM, 0, (LPARAM)&info);
}

BOOL CBCGPBreadcrumb::DeleteItem (HBREADCRUMBITEM hItem)
{
	return (BOOL)SendMessage (BCCM_DELETEITEM, 0, (LPARAM)hItem);
}

BOOL CBCGPBreadcrumb::SetItemText (HBREADCRUMBITEM hItem, const CString& text)
{
	BREADCRUMBITEMINFO info;
	ZeroMemory(&info, sizeof (info));
	info.mask = BCCIF_TEXT;
	info.pszText = const_cast<LPTSTR>((LPCTSTR)text);
	return (BOOL)SendMessage (BCCM_SETITEM, (WPARAM)hItem, (LPARAM)&info);
}

CString CBCGPBreadcrumb::GetItemText (HBREADCRUMBITEM hItem) const
{
	BREADCRUMBITEMINFO info;
	ZeroMemory (&info, sizeof (info));
	info.mask = BCCIF_TEXT;
	::SendMessage (m_hWnd, BCCM_GETITEM, (WPARAM)hItem, (LPARAM)&info); //query text length
	int len = info.cchTextMax - 1;
	ASSERT(len >= 0);
	if (len <= 0)
	{
		return CString ();
	}

	CString strText;
	info.pszText = strText.GetBufferSetLength (len);
	::SendMessage (m_hWnd, BCCM_GETITEM, (WPARAM)hItem, (LPARAM)&info);
	return strText;
}

BOOL CBCGPBreadcrumb::SetItemTooltipText (HBREADCRUMBITEM hItem, const CString& text)
{
	BREADCRUMBITEMINFO info;
	ZeroMemory(&info, sizeof(info));
	info.mask = BCCIF_TOOLTIP;
	info.pszTooltipText = const_cast<LPTSTR>((LPCTSTR)text);
	return (BOOL)SendMessage (BCCM_SETITEM, (WPARAM)hItem, (LPARAM)&info);
}

CString CBCGPBreadcrumb::GetItemTooltipText (HBREADCRUMBITEM hItem) const
{
	BREADCRUMBITEMINFO info;
	ZeroMemory(&info, sizeof(info));
	info.mask = BCCIF_TOOLTIP;
	::SendMessage (m_hWnd, BCCM_GETITEM, (WPARAM)hItem, (LPARAM)&info); //query text length
	int len = info.cchTooltipTextMax - 1;
	ASSERT(len >= 0);
	if (len <= 0)
	{
		return CString ();
	}

	CString strText;
	info.pszTooltipText = strText.GetBufferSetLength (len);
	::SendMessage (m_hWnd, BCCM_GETITEM, (WPARAM)hItem, (LPARAM)&info);
	return strText;
}

BOOL CBCGPBreadcrumb::SetItemImageIndex (HBREADCRUMBITEM hItem, int iImageIndex)
{
	BREADCRUMBITEMINFO info;
	ZeroMemory (&info, sizeof (info));
	info.mask = BCCIF_IMAGE;
	info.iImage = iImageIndex;
	return (BOOL)SendMessage (BCCM_SETITEM, (WPARAM)hItem, (LPARAM)&info);
}

int CBCGPBreadcrumb::GetItemImageIndex (HBREADCRUMBITEM hItem) const
{
	BREADCRUMBITEMINFO info;
	ZeroMemory (&info, sizeof (info));
	info.mask = BCCIF_IMAGE;
	::SendMessage (m_hWnd, BCCM_GETITEM, (WPARAM)hItem, (LPARAM)&info);
	return info.iImage;
}

BOOL CBCGPBreadcrumb::SetItemData (HBREADCRUMBITEM hItem, LPARAM lParamData)
{
	BREADCRUMBITEMINFO info;
	ZeroMemory(&info, sizeof(info));
	info.mask = BCCIF_PARAM;
	info.lParam = lParamData;
	return (BOOL)SendMessage (BCCM_SETITEM, (WPARAM)hItem, (LPARAM)&info);
}

LPARAM CBCGPBreadcrumb::GetItemData (HBREADCRUMBITEM hItem) const
{
	BREADCRUMBITEMINFO info;
	ZeroMemory (&info, sizeof (info));
	info.mask = BCCIF_PARAM;
	::SendMessage (m_hWnd, BCCM_GETITEM, (WPARAM)hItem, (LPARAM)&info);
	return info.lParam;
}

BOOL CBCGPBreadcrumb::SetItemDynamic(HBREADCRUMBITEM hItem, BOOL bDynamic)
{
	BREADCRUMBITEMINFO info;
	ZeroMemory(&info, sizeof(info));
	info.mask = BCCIF_DYNAMIC;
	info.bDynamic = bDynamic;
	return (BOOL)SendMessage (BCCM_SETITEM, (WPARAM)hItem, (LPARAM)&info);
}

BOOL CBCGPBreadcrumb::GetItemDynamic(HBREADCRUMBITEM hItem) const
{
	BREADCRUMBITEMINFO info;
	ZeroMemory (&info, sizeof (info));
	info.mask = BCCIF_DYNAMIC;
	::SendMessage (m_hWnd, BCCM_GETITEM, (WPARAM)hItem, (LPARAM)&info);
	return info.bDynamic;
}

HBREADCRUMBITEM CBCGPBreadcrumb::GetItemParent (HBREADCRUMBITEM hItem) const
{
	BREADCRUMBITEMINFO info;
	ZeroMemory (&info, sizeof(info));
	info.mask = BCCIF_PARENT;
	::SendMessage (m_hWnd, BCCM_GETITEM, (WPARAM)hItem, (LPARAM)&info);
	return info.hParentItem;
}

HBREADCRUMBITEM CBCGPBreadcrumb::GetSubItem (HBREADCRUMBITEM hParentItem, int iIndex) const
{
	return (HBREADCRUMBITEM)::SendMessage (m_hWnd, BCCM_GETSUBITEM, iIndex, (LPARAM)hParentItem);
}

HBREADCRUMBITEM CBCGPBreadcrumb::GetSubItemByName (HBREADCRUMBITEM hParentItem, const CString& itemName) const
{
	int n = GetSubItemsCount (hParentItem);
	for (int i = 0; i < n; ++i)
	{
		HBREADCRUMBITEM hSubItem = GetSubItem (hParentItem, i);
		if (GetItemText (hSubItem).CompareNoCase (itemName) == 0)
		{
			return hSubItem;
		}
	}
	return NULL;
}

int CBCGPBreadcrumb::GetSubItemsCount (HBREADCRUMBITEM hParentItem) const
{
	return (int)::SendMessage (m_hWnd, BCCM_GETSUBITEMCOUNT, 0, (LPARAM)hParentItem);
}

CImageList* CBCGPBreadcrumb::SetImageList (CImageList* pImageList)
{
	HIMAGELIST hImageList = (HIMAGELIST)SendMessage (BCCM_SETIMAGELIST, 0, (LPARAM)pImageList->GetSafeHandle());
	return CImageList::FromHandle (hImageList);
}

CImageList* CBCGPBreadcrumb::GetImageList () const
{
	return CImageList::FromHandle ((HIMAGELIST)::SendMessage (m_hWnd, BCCM_GETIMAGELIST, 0, 0));
}

COLORREF CBCGPBreadcrumb::SetDefaultTextColor (COLORREF color)
{
	return (COLORREF)SendMessage (BCCM_SETTEXTCOLOR, 0, (LPARAM)color);
}

COLORREF CBCGPBreadcrumb::SetBackColor (COLORREF color)
{
	return (COLORREF)SendMessage (BCCM_SETBKCOLOR, 0, (LPARAM)color);
}

COLORREF CBCGPBreadcrumb::SetDefaultHighlightedTextColor (COLORREF color)
{
	return (COLORREF)SendMessage (BCCM_SETHILITETEXTCOLOR, 0, (LPARAM)color);
}

COLORREF CBCGPBreadcrumb::SetDefaultHighlightColor (COLORREF color)
{
	return (COLORREF)SendMessage (BCCM_SETHILITECOLOR, 0, (LPARAM)color);
}

COLORREF CBCGPBreadcrumb::GetDefaultTextColor () const
{
	return (COLORREF)::SendMessage (m_hWnd, BCCM_GETTEXTCOLOR, 0, 0);
}

COLORREF CBCGPBreadcrumb::GetBackColor () const
{
	return (COLORREF)::SendMessage (m_hWnd, BCCM_GETBKCOLOR, 0, 0);
}

COLORREF CBCGPBreadcrumb::GetDefaultHighlightedTextColor () const
{
	return (COLORREF)::SendMessage (m_hWnd, BCCM_GETHILITETEXTCOLOR, 0, 0);
}

COLORREF CBCGPBreadcrumb::GetDefaultHighlightColor () const
{
	return (COLORREF)::SendMessage (m_hWnd, BCCM_GETHILITECOLOR, 0, 0);
}

UINT CBCGPBreadcrumb::SetRightMargin (UINT cxMargin)
{
	return (UINT)SendMessage (BCCM_SETRIGHTMARGIN, cxMargin);
}

UINT CBCGPBreadcrumb::GetRightMargin () const
{
	return (UINT)::SendMessage (m_hWnd, BCCM_GETRIGHTMARGIN, 0, 0);
}


BEGIN_MESSAGE_MAP (CBCGPBreadcrumb, CEdit)
	ON_NOTIFY_REFLECT(BCCN_INITROOT, OnInitRootReflect)
	ON_NOTIFY_REFLECT_EX(NM_KEYDOWN, OnKeyDownReflect)
	ON_NOTIFY_REFLECT(NM_RETURN, OnReturnKeyReflect)
	ON_NOTIFY_REFLECT(BCCN_SELECTIONCHANGED, OnSelectionChangeReflect)
	ON_NOTIFY_REFLECT(NM_CLICK, OnLClickReflect)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRClickReflect)
	ON_NOTIFY_REFLECT(BCCN_BEGIN_INPLACE_EDITING, OnBeginInplaceEditingReflect)
	ON_NOTIFY_REFLECT_EX(BCCN_DYNAMIC_GETSUBITEMS, OnGetChildrenReflect)
	ON_EN_KILLFOCUS(IDC_BREADCRUMB_INPLACE_EDITOR, CancelInplaceEditor)
	ON_WM_SETCURSOR()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLAERO, OnBCGSetControlAero)
	ON_REGISTERED_MESSAGE(BCGM_ONSETCONTROLVMMODE, OnBCGSetControlVMMode)
END_MESSAGE_MAP()

// Redirect reflected messages to the corresponding virtual functions

void CBCGPBreadcrumb::OnInitRootReflect (NMHDR*, LRESULT*)
{
	OnInitRoot ();
}

BOOL CBCGPBreadcrumb::OnKeyDownReflect (NMHDR* pNmhdr, LRESULT* pResult)
{
	NMKEY* pKey = (NMKEY*)pNmhdr;
	if (pKey != NULL && OnKeyDown (pKey->nVKey, pKey->uFlags))
	{
		*pResult = TRUE;
		return TRUE;
	}
	return FALSE;
}

void CBCGPBreadcrumb::OnReturnKeyReflect (NMHDR*, LRESULT*)
{
	OnReturnKey ();
}

void CBCGPBreadcrumb::OnSelectionChangeReflect (NMHDR*, LRESULT*)
{
	OnSelectionChanged (GetSelectedItem ());
}

void CBCGPBreadcrumb::OnLClickReflect (NMHDR* pNmhdr, LRESULT*)
{
	NMCLICK* pClick = (NMCLICK*)pNmhdr;
	if (pClick != NULL)
	{
		OnLeftClick (pClick->pt, (UINT)pClick->dwHitInfo, (HBREADCRUMBITEM)pClick->dwItemSpec);
	}
}

void CBCGPBreadcrumb::OnRClickReflect (NMHDR* pNmhdr, LRESULT*)
{
	NMCLICK* pClick = (NMCLICK*)pNmhdr;
	if (pClick != NULL)
	{
		OnRightClick (pClick->pt, (UINT)pClick->dwHitInfo, (HBREADCRUMBITEM)pClick->dwItemSpec);
	}
}

void CBCGPBreadcrumb::OnBeginInplaceEditingReflect (NMHDR*, LRESULT*)
{
	BeginInplaceEditing ();
}


BOOL CBCGPBreadcrumb::OnGetChildrenReflect (NMHDR* pNmhdr, LRESULT* pResult)
{
	BREADCRUMB_ITEM_NOTIFICATION* pInfo = (BREADCRUMB_ITEM_NOTIFICATION*)pNmhdr;
	if (pInfo != NULL)
	{
		GetItemChildrenDynamic (pInfo->hItem);
		*pResult = TRUE;
		return TRUE;
	}
	return FALSE;
}

// Default implementation for reflected messages

void CBCGPBreadcrumb::OnInitRoot ()
{
}

BOOL CBCGPBreadcrumb::OnKeyDown (UINT, UINT)
{
	return FALSE;
}

void CBCGPBreadcrumb::OnReturnKey ()
{
}

void CBCGPBreadcrumb::OnLeftClick (POINT, UINT, HBREADCRUMBITEM)
{
}

void CBCGPBreadcrumb::OnRightClick (POINT, UINT, HBREADCRUMBITEM)
{
}

void CBCGPBreadcrumb::OnSelectionChanged (HBREADCRUMBITEM)
{
}

void CBCGPBreadcrumb::GetItemChildrenDynamic (HBREADCRUMBITEM)
{
}

// Inplace editing support

void CBCGPBreadcrumb::BeginInplaceEditing ()
{
	ASSERT(m_pInplaceEdit == NULL);

	CRect rectEdit;
	SendMessage(BCCM_GETINPLACEEDITRECT, 0, (LPARAM)(LPRECT)rectEdit);

	CFont* pFont = GetFont (); // breadcrumb font
	if (pFont == NULL)
	{
		pFont = CFont::FromHandle ((HFONT)::GetStockObject (DEFAULT_GUI_FONT));
	}

	// Adjust editor vertically

	int height = rectEdit.Height ();
	{
		CWindowDC dc (NULL);
		CBCGPFontSelector font (dc, pFont);

		TEXTMETRIC tm;
		dc.GetTextMetrics (&tm);
		height = tm.tmHeight + tm.tmInternalLeading + 1;
	}

	if (height < rectEdit.Height ())
	{
		rectEdit.top += (rectEdit.Height () - height) / 2 + 1;
	}

	// Create an inplace edit

	m_pInplaceEdit = CreateInplaceEdit (this, rectEdit, IDC_BREADCRUMB_INPLACE_EDITOR);

	ASSERT_VALID (m_pInplaceEdit);
	ASSERT (m_pInplaceEdit->GetParent () == this);
	ASSERT (m_pInplaceEdit->GetDlgCtrlID () == IDC_BREADCRUMB_INPLACE_EDITOR);
	
	m_pInplaceEdit->SetFont (pFont);

	m_pInplaceEdit->SetWindowText (GetSelectedPath ());
	m_pInplaceEdit->SetSel (0, -1); // select all
	m_pInplaceEdit->ShowWindow (SW_SHOW);

	m_pInplaceEdit->SetFocus ();
}

CEdit* CBCGPBreadcrumb::CreateInplaceEdit (CWnd* pParent, const CRect& rect, UINT uiControlID)
{
	CBCGPEdit* pEdit = new CBCGPEdit;
	pEdit->Create (WS_CHILD | ES_AUTOHSCROLL | ES_WANTRETURN, rect, pParent, uiControlID);

	pEdit->m_bOnGlass = m_bOnGlass;
	pEdit->m_bVisualManagerStyle = m_bVisualManagerStyle;

	return pEdit;
}

void CBCGPBreadcrumb::DestroyInplaceEdit (CEdit* pEditor)
{
	if (pEditor != NULL)
	{
		delete pEditor;
	}
}

BOOL CBCGPBreadcrumb::ValidateInplaceEdit (const CString& strPath)
{
	return !strPath.IsEmpty ();
}

void CBCGPBreadcrumb::CancelInplaceEditor ()
{
	if (m_pInplaceEdit != NULL)
	{
		m_pInplaceEdit->DestroyWindow ();
		DestroyInplaceEdit (m_pInplaceEdit);
		m_pInplaceEdit = NULL;
	}
}

BOOL CBCGPBreadcrumb::PreTranslateMessage (MSG* pMsg)
{
	if (m_pInplaceEdit == NULL && pMsg->message == WM_KEYDOWN)
	{
		if (IsCharAlphaNumeric((TCHAR)pMsg->wParam) || pMsg->wParam == VK_SPACE)
		{
			WindowProc (pMsg->message, pMsg->wParam, pMsg->lParam);
			return TRUE;
		}
	}

	if (m_pInplaceEdit != NULL && pMsg->hwnd == m_pInplaceEdit->GetSafeHwnd () && pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_RETURN:
			{
				CString strPath;
				m_pInplaceEdit->GetWindowText (strPath);
				if (!ValidateInplaceEdit (strPath))
				{
					// Continue editing
					return TRUE;
				}
				CancelInplaceEditor ();
				SelectPath (strPath);
				return TRUE;
			}

		case VK_ESCAPE:
			CancelInplaceEditor ();
			return TRUE;
		}
	}

	return CEdit::PreTranslateMessage (pMsg);
}

BOOL CBCGPBreadcrumb::OnSetCursor (CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_pInplaceEdit == NULL)
	{
		AfxGetApp ()->LoadCursor (IDC_ARROW);
		return TRUE;
	}

	return CWnd::OnSetCursor (pWnd, nHitTest, message);
}

void CBCGPBreadcrumb::OnDestroy ()
{
	if (m_pInplaceEdit != NULL)
	{
		CancelInplaceEditor ();
	}
}

void CBCGPBreadcrumb::PreSubclassWindow ()
{
	if (m_bInCreate)
	{
		return;
	}

	// Translate EDIT control styles into Breadcrumb styles.
	DWORD dwEditStyle = GetStyle ();
	bool bWantReturn	 = (dwEditStyle & ES_WANTRETURN) != 0;
	bool bSupportEdit	 = (dwEditStyle & ES_READONLY) == 0;

	DWORD dwBreadcrumbStyle = dwEditStyle & (WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | WS_GROUP);
	if (bWantReturn) dwBreadcrumbStyle |= BCCS_WANTRETURN;
	if (bSupportEdit) dwBreadcrumbStyle |= BCCS_INPLACEEDIT;

	::SetWindowLong (m_hWnd, GWL_STYLE, dwBreadcrumbStyle);
	SetWindowPos (NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

LRESULT CBCGPBreadcrumb::WindowProc (UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_pImpl == NULL)
	{
		m_pImpl = AttachBreadcrumbImplementation (this);
		OnInitRoot ();

		return CWnd::WindowProc (message, wParam, lParam);
	}

	BOOL bHandled = FALSE;
	LRESULT result = BreadcrumbWindowProc (m_pImpl, message, wParam, lParam, &bHandled);

	if (!bHandled)
	{
		return CWnd::WindowProc (message, wParam, lParam);
	}

	return result;
}

LRESULT CBCGPBreadcrumb::OnBCGSetControlVMMode (WPARAM wp, LPARAM)
{
	m_bVisualManagerStyle = (BOOL) wp;
	return 0;
}

LRESULT CBCGPBreadcrumb::OnBCGSetControlAero (WPARAM wp, LPARAM)
{
	m_bOnGlass = (BOOL) wp;
	return 0;
}

void CBCGPBreadcrumb::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/) 
{
}
